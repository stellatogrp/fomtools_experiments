#include <string.h>
#include <math.h>
#include "fom_solver.h"
#include "fom_data.h"
#include "fom_ops.h"
#include "fom_linsolve.h"
#include "fom_profile.h"

/* out += c * (M @ x), via the accumulating helper into scratch `tmp`.
   Skipped entirely when the schedule coefficient is exactly zero. */
typedef void (*fom_apply_fn)(const fom_float *, fom_float *);

static void axpy_op(fom_apply_fn f, fom_float c, const fom_float *x,
                    fom_float *out, fom_float *tmp, fom_int n) {
    fom_int i;
    if (c == (fom_float)0) return;
    if (c == (fom_float)1) { f(x, out); return; }
    memset(tmp, 0, (size_t)n * sizeof(fom_float));
    f(x, tmp);
    for (i = 0; i < n; i++) out[i] += c * tmp[i];
}

/* ------------------------------------------------------------------
   Fused operators.

   Every linear_op is a fixed sum of sparse matrices weighted by the
   schedule coefficients. For a given coefficient tuple the sum is a
   single sparse matrix, so we assemble it once (by probing the
   fom_apply_* helpers with unit vectors) and cache it. The iteration
   operates on the concatenated vector z = [state(160); t0(48); t1(16)]:
     OP0 (48 x 160) : t0 = relu(OP0 z + b0),       b0 = B0(tau) p1
     OP1 (16 x 208) : t1 = clip(OP1 z + b1),       b1 = B1(eta) p0
     OP2 (160 x 224): state+ = OP2 z
   The cache depends only on constant data and the schedule tuple, so
   it is shared by all workspaces. It is rebuilt whenever the tuple
   changes; if a fused operator ever exceeds its static capacity the
   solver falls back to the term-by-term reference iteration.
   ------------------------------------------------------------------ */

#define NSCHED 10
#define Z_T0 FOM_STATE_DIM
#define Z_T1 (FOM_STATE_DIM + FOM_T0_DIM)
#define Z_DIM (FOM_STATE_DIM + FOM_T0_DIM + FOM_T1_DIM)
#define OP_CAP 1024

#define PB_SHIFT 4
#define PB (1 << PB_SHIFT)          /* panel column-block width */
#define EXT_CAP 64                  /* merged-column slots after z */
#define ZX_DIM (Z_DIM + EXT_CAP)
#define NPB ((ZX_DIM + PB - 1) >> PB_SHIFT) /* column blocks of z (+ext) */
#define PANEL_CAP 2048
#define DB 16                         /* DIA row-block height */
#define DIA_CAP 64                    /* max diagonals per operator */
#define NOFF (FOM_STATE_DIM + ZX_DIM) /* offsets d = col - row */

typedef struct {
    /* full CSR as assembled */
    fom_int rowptr[FOM_STATE_DIM + 1];
    fom_int col[OP_CAP];
    fom_float val[OP_CAP];
    /* proportional-column merges: z[m_dst] = z[m_a] + m_alpha * z[m_b] */
    fom_int nmerge;
    fom_int m_dst[EXT_CAP], m_a[EXT_CAP], m_b[EXT_CAP];
    fom_float m_alpha[EXT_CAP];
    /* the merges grouped into runs of consecutive indices, equal alpha */
    fom_int nmrun;
    fom_int mr_dst[EXT_CAP], mr_a[EXT_CAP], mr_b[EXT_CAP], mr_len[EXT_CAP];
    fom_float mr_alpha[EXT_CAP];
    /* execution form */
    int copy_mode;                  /* every row is 0 or a single 1.0 */
    fom_int nruns;                  /* copy mode: out[d..d+l) = z[s..s+l) */
    fom_int run_dst[FOM_STATE_DIM], run_src[FOM_STATE_DIM], run_len[FOM_STATE_DIM];
    /* live-row subset of the runs (see mark_live) */
    fom_int nlruns;
    fom_int lrun_dst[FOM_STATE_DIM], lrun_src[FOM_STATE_DIM], lrun_len[FOM_STATE_DIM];
    /* row blocks of DB rows; within each, full diagonals (entry at
       column row + d in every row of the block) are stored as
       contiguous vectors, giving unit-stride vectorizable loads */
    fom_int nblk;
    fom_int b_r0[FOM_STATE_DIM], b_len[FOM_STATE_DIM];
    fom_int b_d0[FOM_STATE_DIM + 1];     /* diag range of block */
    fom_int d_off[DIA_CAP];
    fom_float d_val[DIA_CAP * DB];
    /* sparse remainder (CSR) */
    fom_int srowptr[FOM_STATE_DIM + 1];
    fom_int scol[OP_CAP];
    fom_float sval[OP_CAP];
    /* dense column panels: block c0..c0+PB over `nact` active rows,
       stored column-major so the inner loop runs over rows */
    fom_int npanels;
    fom_int p_c0[NPB], p_nact[NPB], p_roff[NPB], p_voff[NPB];
    fom_int p_rows[NPB * FOM_STATE_DIM];
    fom_float p_val[PANEL_CAP];
} FusedOp;

/* Division-free reciprocal (setup only): range-reduce with frexp, then
   Newton iterations r <- r (2 - m r), which converge quadratically. */
static fom_float recip(fom_float v) {
    int e, k;
    fom_float m = frexp(fabs(v), &e), r;      /* m in [0.5, 1) */
    r = 2.8235294117647059 - 1.8823529411764706 * m;
    for (k = 0; k < 6; k++) r = r * (2 - m * r);
    r = ldexp(r, -e);
    return v < 0 ? -r : r;
}

/* Find column pairs (a, b) of the operator with identical sparsity and
   col_b = alpha * col_a (e.g. the extrapolation x + theta (x - x_old)
   puts G and -theta G on two state slots). Each pair is replaced by one
   column acting on a precombined input z_a + alpha z_b stored in an
   extension slot past Z_DIM, halving those entries. */
static void merge_columns(FusedOp *op, fom_int nrows, fom_int ncols) {
    static fom_int cptr[ZX_DIM + 1], crow[OP_CAP], cfill[ZX_DIM];
    static fom_float cval[OP_CAP];
    static fom_int remap[ZX_DIM];
    fom_int i, p, a, b, k, q, nnz = op->rowptr[nrows];
    op->nmerge = 0;
    memset(cptr, 0, sizeof(cptr));
    for (p = 0; p < nnz; p++) cptr[op->col[p] + 1]++;
    for (a = 0; a < ncols; a++) cptr[a + 1] += cptr[a];
    for (a = 0; a < ncols; a++) cfill[a] = cptr[a];
    for (i = 0; i < nrows; i++)
        for (p = op->rowptr[i]; p < op->rowptr[i + 1]; p++) {
            q = cfill[op->col[p]]++;
            crow[q] = i; cval[q] = op->val[p];
        }
    for (a = 0; a < ncols; a++) remap[a] = a;      /* -1 = dropped */
    for (b = 0; b < ncols && op->nmerge < EXT_CAP; b++) {
        fom_int nb = cptr[b + 1] - cptr[b];
        if (nb < 2 || remap[b] != b) continue;
        for (a = 0; a < b; a++) {
            fom_float alpha;
            int ok = 1;
            if (remap[a] != a || cptr[a + 1] - cptr[a] != nb) continue;
            alpha = cval[cptr[b]] * recip(cval[cptr[a]]);
            for (k = 0; k < nb && ok; k++) {
                fom_float va = cval[cptr[a] + k], vb = cval[cptr[b] + k];
                fom_float d = vb - alpha * va;
                if (crow[cptr[a] + k] != crow[cptr[b] + k]) ok = 0;
                else if (fabs(d) > 1e-14 * fabs(vb)) ok = 0;
            }
            if (!ok) continue;
            k = op->nmerge++;
            op->m_dst[k] = Z_DIM + k; op->m_a[k] = a; op->m_b[k] = b;
            op->m_alpha[k] = alpha;
            remap[a] = Z_DIM + k; remap[b] = -1;
            break;
        }
    }
    op->nmrun = 0;
    for (k = 0; k < op->nmerge; k++) {
        fom_int n = op->nmrun;
        if (n > 0 && op->mr_dst[n - 1] + op->mr_len[n - 1] == op->m_dst[k] &&
            op->mr_a[n - 1] + op->mr_len[n - 1] == op->m_a[k] &&
            op->mr_b[n - 1] + op->mr_len[n - 1] == op->m_b[k] &&
            op->mr_alpha[n - 1] == op->m_alpha[k]) {
            op->mr_len[n - 1]++;
        } else {
            op->mr_dst[n] = op->m_dst[k]; op->mr_a[n] = op->m_a[k];
            op->mr_b[n] = op->m_b[k]; op->mr_alpha[n] = op->m_alpha[k];
            op->mr_len[n] = 1; op->nmrun++;
        }
    }
    if (op->nmerge == 0) return;
    q = 0;
    for (i = 0; i < nrows; i++) {
        fom_int start = op->rowptr[i];
        op->rowptr[i] = q;
        for (p = start; p < op->rowptr[i + 1]; p++) {
            fom_int c = remap[op->col[p]];
            if (c < 0) continue;
            op->col[q] = c; op->val[q] = op->val[p]; q++;
        }
    }
    op->rowptr[nrows] = q;
}

/* Split the CSR into dense column panels (where the block is dense
   enough that a vectorizable column-major panel beats gathers) and a
   sparse CSR remainder; detect pure selection operators. */
static int build_exec(FusedOp *op, fom_int nrows, fom_int ncols) {
    fom_int i, p, b, r, j;
    fom_int voff = 0, roff = 0, q = 0;
    int dense_blk[NPB] = {0};
    op->copy_mode = 1;
    op->nmerge = 0;
    op->nmrun = 0;
    for (i = 0; i < nrows; i++) {
        fom_int len = op->rowptr[i + 1] - op->rowptr[i];
        if (len > 1 || (len == 1 && op->val[op->rowptr[i]] != 1)) op->copy_mode = 0;
    }
    if (op->copy_mode) {
        fom_int n = 0;
        for (i = 0; i < nrows; i++) {
            fom_int src = (op->rowptr[i + 1] > op->rowptr[i]) ? op->col[op->rowptr[i]] : -1;
            if (n > 0 && src >= 0 && op->run_src[n - 1] >= 0 &&
                op->run_dst[n - 1] + op->run_len[n - 1] == i &&
                op->run_src[n - 1] + op->run_len[n - 1] == src) {
                op->run_len[n - 1]++;
            } else if (n > 0 && src < 0 && op->run_src[n - 1] < 0 &&
                       op->run_dst[n - 1] + op->run_len[n - 1] == i) {
                op->run_len[n - 1]++;
            } else {
                op->run_dst[n] = i; op->run_src[n] = src; op->run_len[n] = 1; n++;
            }
        }
        op->nruns = n;
        return 1;
    }
    merge_columns(op, nrows, ncols);
    op->npanels = 0;
    for (b = 0; b * PB < ncols; b++) {
        fom_int c0 = b * PB, c1 = c0 + PB, nnz = 0, nact = 0;
        for (i = 0; i < nrows; i++) {
            fom_int rn = 0;
            for (p = op->rowptr[i]; p < op->rowptr[i + 1]; p++)
                if (op->col[p] >= c0 && op->col[p] < c1) rn++;
            nnz += rn;
            if (rn) nact++;
        }
        /* dense if at least ~40% of the active-row block is filled */
        dense_blk[b] = (c1 <= ncols && nact > 0 && 5 * nnz >= 2 * PB * nact &&
                        voff + PB * ((nact + 7) & ~7) <= PANEL_CAP);
        if (!dense_blk[b]) continue;
        {
            fom_int k = op->npanels++;
            fom_int ns = (nact + 7) & ~7;   /* row stride, padded with zeros */
            for (j = 0; j < PB * ns; j++) op->p_val[voff + j] = 0;
            op->p_c0[k] = c0; op->p_nact[k] = nact;
            op->p_roff[k] = roff; op->p_voff[k] = voff;
            r = 0;
            for (i = 0; i < nrows; i++) {
                int act = 0;
                for (p = op->rowptr[i]; p < op->rowptr[i + 1]; p++)
                    if (op->col[p] >= c0 && op->col[p] < c1) act = 1;
                if (!act) continue;
                for (p = op->rowptr[i]; p < op->rowptr[i + 1]; p++)
                    if (op->col[p] >= c0 && op->col[p] < c1)
                        op->p_val[voff + (op->col[p] - c0) * ns + r] = op->val[p];
                op->p_rows[roff + r] = i;
                r++;
            }
            roff += nact; voff += PB * ns;
        }
    }
    {
        static fom_int cnt[NOFF];
        static unsigned char taken[OP_CAP];
        fom_int r0, nd = 0, t, d;
        memset(taken, 0, sizeof(taken));
        for (p = 0; p < op->rowptr[nrows]; p++)
            if (dense_blk[op->col[p] >> PB_SHIFT]) taken[p] = 1;
        op->nblk = 0;
        for (r0 = 0; r0 < nrows; r0 += DB) {
            fom_int L = (nrows - r0 < DB) ? nrows - r0 : DB, kb = op->nblk++;
            op->b_r0[kb] = r0; op->b_len[kb] = L; op->b_d0[kb] = nd;
            memset(cnt, 0, sizeof(cnt));
            for (i = r0; i < r0 + L; i++)
                for (p = op->rowptr[i]; p < op->rowptr[i + 1]; p++)
                    if (!taken[p]) cnt[op->col[p] - i + FOM_STATE_DIM]++;
            for (d = 0; d < NOFF && nd < DIA_CAP; d++) {
                if (cnt[d] != L) continue;      /* full diagonals only */
                op->d_off[nd] = d - FOM_STATE_DIM;
                for (t = 0; t < L; t++) {
                    i = r0 + t;
                    for (p = op->rowptr[i]; p < op->rowptr[i + 1]; p++)
                        if (!taken[p] && op->col[p] - i + FOM_STATE_DIM == d) {
                            op->d_val[nd * DB + t] = op->val[p];
                            taken[p] = 1;
                        }
                }
                nd++;
            }
        }
        op->b_d0[op->nblk] = nd;
        for (i = 0; i < nrows; i++) {
            op->srowptr[i] = q;
            for (p = op->rowptr[i]; p < op->rowptr[i + 1]; p++) {
                if (taken[p]) continue;
                op->scol[q] = op->col[p]; op->sval[q] = op->val[p]; q++;
            }
        }
        op->srowptr[nrows] = q;
    }
    return 1;
}

static inline void copy_runs(fom_int n, const fom_int *dst, const fom_int *src,
                             const fom_int *len, const fom_float *z, fom_float *u) {
    fom_int k;
    for (k = 0; k < n; k++) {
        if (src[k] >= 0)
            memcpy(u + dst[k], z + src[k], (size_t)len[k] * sizeof(fom_float));
        else
            memset(u + dst[k], 0, (size_t)len[k] * sizeof(fom_float));
    }
}

/* u = b + op z  (b == NULL means zero). */
static inline void apply_fused(const FusedOp *op, fom_int nrows, const fom_float *b,
                               fom_float *z, fom_float *u) {
    fom_int i, p, k, j, r;
    for (k = 0; k < op->nmrun; k++) {
        fom_float *zd = z + op->mr_dst[k];
        const fom_float *za = z + op->mr_a[k], *zb = z + op->mr_b[k];
        const fom_float al = op->mr_alpha[k];
        const fom_int len = op->mr_len[k];
        for (i = 0; i < len; i++) zd[i] = za[i] + al * zb[i];
    }
    if (op->copy_mode) {
        copy_runs(op->nruns, op->run_dst, op->run_src, op->run_len, z, u);
        if (b) for (i = 0; i < nrows; i++) u[i] += b[i];
        return;
    }
    {
        const fom_int *rp = op->srowptr, *sc = op->scol;
        const fom_float *sv = op->sval;
        fom_int kb;
        for (kb = 0; kb < op->nblk; kb++) {
            const fom_int r0 = op->b_r0[kb], L = op->b_len[kb];
            fom_float acc[DB];
            fom_int t;
            if (b) for (t = 0; t < L; t++) acc[t] = b[r0 + t];
            else for (t = 0; t < L; t++) acc[t] = 0;
            if (L == DB) {
                for (k = op->b_d0[kb]; k < op->b_d0[kb + 1]; k++) {
                    const fom_float *zz = z + r0 + op->d_off[k], *dv = op->d_val + k * DB;
                    for (t = 0; t < DB; t++) acc[t] += dv[t] * zz[t];
                }
            } else {
                for (k = op->b_d0[kb]; k < op->b_d0[kb + 1]; k++) {
                    const fom_float *zz = z + r0 + op->d_off[k], *dv = op->d_val + k * DB;
                    for (t = 0; t < L; t++) acc[t] += dv[t] * zz[t];
                }
            }
            for (t = 0; t < L; t++) {
                fom_float v = acc[t], w = 0;
                const fom_int e = rp[r0 + t + 1];
                for (p = rp[r0 + t]; p + 1 < e; p += 2) {
                    v += sv[p] * z[sc[p]];
                    w += sv[p + 1] * z[sc[p + 1]];
                }
                if (p < e) v += sv[p] * z[sc[p]];
                u[r0 + t] = v + w;
            }
        }
    }
    for (k = 0; k < op->npanels; k++) {
        const fom_int nact = op->p_nact[k], ns = (nact + 7) & ~7;
        const fom_int *rows = op->p_rows + op->p_roff[k];
        const fom_float *pv = op->p_val + op->p_voff[k];
        const fom_float *zc = z + op->p_c0[k];
        fom_int r0 = 0;
        for (; r0 + 16 <= nact; r0 += 16) {
            fom_float a[16], c[16];
            const fom_float *q = pv + r0;
            for (r = 0; r < 16; r++) { a[r] = 0; c[r] = 0; }
            for (j = 0; j < PB; j += 2, q += 2 * ns) {
                const fom_float z0 = zc[j], z1 = zc[j + 1];
                for (r = 0; r < 16; r++) {
                    a[r] += q[r] * z0;
                    c[r] += q[ns + r] * z1;
                }
            }
            for (r = 0; r < 16; r++) u[rows[r0 + r]] += a[r] + c[r];
        }
        for (; r0 < nact; r0 += 8) {
            fom_float a[8], c[8];
            const fom_float *q = pv + r0;
            for (r = 0; r < 8; r++) { a[r] = 0; c[r] = 0; }
            for (j = 0; j < PB; j += 2, q += 2 * ns) {
                const fom_float z0 = zc[j], z1 = zc[j + 1];
                for (r = 0; r < 8; r++) {
                    a[r] += q[r] * z0;
                    c[r] += q[ns + r] * z1;
                }
            }
            for (r = 0; r < 8 && r0 + r < nact; r++) u[rows[r0 + r]] += a[r] + c[r];
        }
    }
}

static FusedOp g_op0, g_op1, g_op2;
static fom_float g_sched[NSCHED];
static int g_valid = 0;   /* 0 = not built, 1 = built, -1 = overflow */

/* sched order: d1 d2 eta g1 g2 m1 m2 tau taux tth */
static void load_sched(fom_int it, fom_float *s) {
    s[0] = fom_sched_d1(it);  s[1] = fom_sched_d2(it);
    s[2] = fom_sched_eta(it); s[3] = fom_sched_g1(it);
    s[4] = fom_sched_g2(it);  s[5] = fom_sched_m1(it);
    s[6] = fom_sched_m2(it);  s[7] = fom_sched_tau(it);
    s[8] = fom_sched_taux(it); s[9] = fom_sched_tth(it);
}

/* Schedule run structure: g_next[it] is the first iteration after `it`
   whose coefficient tuple differs (SCHED_BIG if none; the accessors
   hold the last value past the table). Depends only on constant data,
   so computed once; the hot loop then reloads coefficients only at
   change points instead of every iteration. */
#define MAX2(a, b) ((a) > (b) ? (a) : (b))
#define SCHED_LEN MAX2(MAX2(MAX2(MAX2(FOM_SCHED_D1_LEN, FOM_SCHED_D2_LEN), \
    MAX2(FOM_SCHED_ETA_LEN, FOM_SCHED_G1_LEN)), MAX2(MAX2(FOM_SCHED_G2_LEN, \
    FOM_SCHED_M1_LEN), MAX2(FOM_SCHED_M2_LEN, FOM_SCHED_TAU_LEN))), \
    MAX2(FOM_SCHED_TAUX_LEN, FOM_SCHED_TTH_LEN))
#define SCHED_BIG 0x7fffffff
static fom_int g_next[SCHED_LEN];
static int g_runs_ready = 0;

static void init_sched_runs(void) {
    fom_float a[NSCHED], b[NSCHED];
    fom_int it;
    if (g_runs_ready) return;
    g_next[SCHED_LEN - 1] = SCHED_BIG;
    load_sched(SCHED_LEN - 1, b);
    for (it = SCHED_LEN - 2; it >= 0; it--) {
        load_sched(it, a);
        g_next[it] = memcmp(a, b, sizeof(a)) ? it + 1 : g_next[it + 1];
        memcpy(b, a, sizeof(a));
    }
    g_runs_ready = 1;
}

/* Column j of the fused operator `which` (0,1,2) for schedule s,
   written densely into colv (length nrows). */
static void probe_column(int which, const fom_float *s, fom_int j,
                         fom_float *colv, fom_float *tmp) {
    static fom_float e[FOM_STATE_DIM];
    fom_int n = (which == 0) ? FOM_T0_DIM : (which == 1) ? FOM_T1_DIM : FOM_STATE_DIM;
    fom_int k = j;
    memset(colv, 0, (size_t)n * sizeof(fom_float));
    memset(e, 0, sizeof(e));
    if (k < FOM_STATE_DIM) {
        e[k] = 1;
        if (which == 0) {
            fom_apply_t0_state(e, colv);
            axpy_op(fom_apply_t0_state__d_taux, s[8], e, colv, tmp, n);
            axpy_op(fom_apply_t0_state__d_tth, s[9], e, colv, tmp, n);
        } else if (which == 1) {
            fom_apply_t1_state(e, colv);
            axpy_op(fom_apply_t1_state__d_eta, s[2], e, colv, tmp, n);
        } else {
            fom_apply_su_state(e, colv);
            axpy_op(fom_apply_su_state__d_d1, s[0], e, colv, tmp, n);
            axpy_op(fom_apply_su_state__d_g2, s[4], e, colv, tmp, n);
            axpy_op(fom_apply_su_state__d_m1, s[5], e, colv, tmp, n);
        }
        return;
    }
    k -= FOM_STATE_DIM;
    if (k < FOM_T0_DIM) {
        e[k] = 1;
        if (which == 1) {
            fom_apply_t1_t_t0(e, colv);
            axpy_op(fom_apply_t1_t_t0__d_eta, s[2], e, colv, tmp, n);
        } else if (which == 2) {
            fom_apply_su_t_t0(e, colv);
            axpy_op(fom_apply_su_t_t0__d_d2, s[1], e, colv, tmp, n);
            axpy_op(fom_apply_su_t_t0__d_g1, s[3], e, colv, tmp, n);
        }
        return;
    }
    k -= FOM_T0_DIM;
    e[k] = 1;
    if (which == 2) {
        fom_apply_su_t_t1(e, colv);
        axpy_op(fom_apply_su_t_t1__d_d2, s[1], e, colv, tmp, n);
        axpy_op(fom_apply_su_t_t1__d_g1, s[3], e, colv, tmp, n);
        axpy_op(fom_apply_su_t_t1__d_m2, s[6], e, colv, tmp, n);
    }
}

/* Assemble a fused operator in CSR: count pass, then fill pass. */
static int build_op(FusedOp *op, int which, fom_int nrows, fom_int ncols,
                    const fom_float *s) {
    static fom_float colv[FOM_STATE_DIM], tmp[FOM_STATE_DIM];
    static fom_int fill[FOM_STATE_DIM];
    fom_int i, j;
    memset(op->rowptr, 0, sizeof(op->rowptr));
    for (j = 0; j < ncols; j++) {
        probe_column(which, s, j, colv, tmp);
        for (i = 0; i < nrows; i++) if (colv[i] != 0) op->rowptr[i + 1]++;
    }
    for (i = 0; i < nrows; i++) op->rowptr[i + 1] += op->rowptr[i];
    if (op->rowptr[nrows] > OP_CAP) return 0;
    for (i = 0; i < nrows; i++) fill[i] = op->rowptr[i];
    for (j = 0; j < ncols; j++) {
        probe_column(which, s, j, colv, tmp);
        for (i = 0; i < nrows; i++) {
            if (colv[i] != 0) {
                op->col[fill[i]] = j;
                op->val[fill[i]] = colv[i];
                fill[i]++;
            }
        }
    }
    return build_exec(op, nrows, ncols);
}

/* Mark the state columns of z read by an operator. */
static void mark_reads(const FusedOp *op, fom_int nrows, unsigned char *used) {
    fom_int i, p, k, c;
    if (op->copy_mode) {
        for (k = 0; k < op->nruns; k++)
            for (c = op->run_src[k]; c >= 0 && c < op->run_src[k] + op->run_len[k]; c++)
                if (c < FOM_STATE_DIM) used[c] = 1;
        return;
    }
    for (i = 0; i < nrows; i++)
        for (p = op->srowptr[i]; p < op->srowptr[i + 1]; p++)
            if (op->scol[p] < FOM_STATE_DIM) used[op->scol[p]] = 1;
    for (k = 0; k < op->npanels; k++)
        for (c = op->p_c0[k]; c < op->p_c0[k] + PB; c++)
            if (c < FOM_STATE_DIM) used[c] = 1;
    for (i = 0; i < op->nblk; i++)
        for (k = op->b_d0[i]; k < op->b_d0[i + 1]; k++)
            for (c = op->b_r0[i] + op->d_off[k];
                 c < op->b_r0[i] + op->d_off[k] + op->b_len[i]; c++)
                if (c < FOM_STATE_DIM) used[c] = 1;
    for (k = 0; k < op->nmerge; k++) {
        if (op->m_a[k] < FOM_STATE_DIM) used[op->m_a[k]] = 1;
        if (op->m_b[k] < FOM_STATE_DIM) used[op->m_b[k]] = 1;
    }
}

/* Liveness of the state update. When OP2 is a pure selection, an
   iteration only needs the state rows that some operator reads; the
   rest (pure copies nobody reads, e.g. duplicated slots) are only
   materialized in the final iterations. Fixed point:
   live = reads(OP0, OP1) U state-sources of live OP2 rows. */
static void mark_live(void) {
    static unsigned char live[FOM_STATE_DIM];
    FusedOp *op = &g_op2;
    fom_int i, k, n = 0;
    int changed = 1;
    memset(live, 0, sizeof(live));
    if (!op->copy_mode) {
        memset(live, 1, sizeof(live));
    } else {
        mark_reads(&g_op0, FOM_T0_DIM, live);
        mark_reads(&g_op1, FOM_T1_DIM, live);
        while (changed) {
            changed = 0;
            for (k = 0; k < op->nruns; k++)
                for (i = 0; i < op->run_len[k]; i++) {
                    fom_int sr = op->run_src[k] + i;
                    if (op->run_src[k] >= 0 && live[op->run_dst[k] + i] &&
                        sr < FOM_STATE_DIM && !live[sr]) { live[sr] = 1; changed = 1; }
                }
        }
    }
    op->nlruns = 0;
    if (!op->copy_mode) return;
    for (k = 0; k < op->nruns; k++)
        for (i = 0; i < op->run_len[k]; i++) {
            fom_int d = op->run_dst[k] + i;
            fom_int sr = op->run_src[k] < 0 ? -1 : op->run_src[k] + i;
            if (!live[d]) continue;
            if (n > 0 && op->lrun_dst[n - 1] + op->lrun_len[n - 1] == d &&
                ((sr < 0 && op->lrun_src[n - 1] < 0) ||
                 (sr >= 0 && op->lrun_src[n - 1] >= 0 &&
                  op->lrun_src[n - 1] + op->lrun_len[n - 1] == sr))) {
                op->lrun_len[n - 1]++;
            } else {
                op->lrun_dst[n] = d; op->lrun_src[n] = sr; op->lrun_len[n] = 1; n++;
            }
        }
    op->nlruns = n;
}

/* Ensure the cache matches schedule s. Returns 1 if fused ops usable. */
static int ensure_ops(const fom_float *s) {
    if (g_valid != 0 && memcmp(s, g_sched, sizeof(g_sched)) == 0)
        return g_valid > 0;
    memcpy(g_sched, s, sizeof(g_sched));
    g_valid = (build_op(&g_op0, 0, FOM_T0_DIM, FOM_STATE_DIM, s) &&
               build_op(&g_op1, 1, FOM_T1_DIM, Z_T1, s) &&
               build_op(&g_op2, 2, FOM_STATE_DIM, Z_DIM, s)) ? 1 : -1;
    if (g_valid > 0) mark_live();
    return g_valid > 0;
}

/* Parameter-only terms for schedule s. */
static void param_terms(const FomWorkspace *work, const fom_float *s,
                        fom_float *b0, fom_float *b1, fom_float *tmp) {
    memset(b0, 0, FOM_T0_DIM * sizeof(fom_float));
    fom_apply_t0_p_p1(work->params.p1, b0);
    axpy_op(fom_apply_t0_p_p1__d_tau, s[7], work->params.p1, b0, tmp, FOM_T0_DIM);
    memset(b1, 0, FOM_T1_DIM * sizeof(fom_float));
    fom_apply_t1_p_p0(work->params.p0, b1);
    axpy_op(fom_apply_t1_p_p0__d_eta, s[2], work->params.p0, b1, tmp, FOM_T1_DIM);
}

void fom_set_default_settings(FomSettings *settings) {
    settings->max_iter = FOM_MAX_ITER;
    settings->tol = FOM_TOL;
    settings->check_termination = FOM_CHECK_TERMINATION;
}

static void reset_state(FomWorkspace *work) {
    memcpy(work->state, FOM_STATE_INIT, sizeof(work->state));
}

void fom_setup(FomWorkspace *work, const FomSettings *settings) {
    fom_float s[NSCHED];
    work->settings = *settings;
    memset(&work->params, 0, sizeof(work->params));
    reset_state(work);
    memcpy(work->solution.x, work->state, sizeof(work->state));
    work->solution.iter = 0;
    work->solution.residual = 0;
    work->solution.status = FOM_MAX_ITER_REACHED;
    init_sched_runs();
    load_sched(0, s);
    ensure_ops(s);
}

void fom_update_params(FomWorkspace *work, const FomParams *params) {
    work->params = *params;
    reset_state(work);
}

/* Term-by-term reference iteration for one step (fallback only). */
static void ref_step(FomWorkspace *work, const fom_float *s,
                     const fom_float *x, fom_float *xn) {
    fom_float *acc = work->acc, *t0 = work->t0, *t1 = work->t1;
    const fom_float *p0 = work->params.p0, *p1 = work->params.p1;
    fom_int i;
    memset(t0, 0, sizeof(work->t0));
    fom_apply_t0_state(x, t0);
    axpy_op(fom_apply_t0_state__d_taux, s[8], x, t0, acc, FOM_T0_DIM);
    axpy_op(fom_apply_t0_state__d_tth, s[9], x, t0, acc, FOM_T0_DIM);
    fom_apply_t0_p_p1(p1, t0);
    axpy_op(fom_apply_t0_p_p1__d_tau, s[7], p1, t0, acc, FOM_T0_DIM);
    for (i = 0; i < FOM_T0_DIM; i++) t0[i] = t0[i] > 0 ? t0[i] : 0;
    memset(t1, 0, sizeof(work->t1));
    fom_apply_t1_state(x, t1);
    axpy_op(fom_apply_t1_state__d_eta, s[2], x, t1, acc, FOM_T1_DIM);
    fom_apply_t1_p_p0(p0, t1);
    axpy_op(fom_apply_t1_p_p0__d_eta, s[2], p0, t1, acc, FOM_T1_DIM);
    fom_apply_t1_t_t0(t0, t1);
    axpy_op(fom_apply_t1_t_t0__d_eta, s[2], t0, t1, acc, FOM_T1_DIM);
    for (i = 0; i < FOM_T1_DIM; i++) {
        fom_float v = t1[i];
        if (v < FOM_MAT_t1_lo[i]) v = FOM_MAT_t1_lo[i];
        if (v > FOM_MAT_t1_hi[i]) v = FOM_MAT_t1_hi[i];
        t1[i] = v;
    }
    memset(xn, 0, FOM_STATE_DIM * sizeof(fom_float));
    fom_apply_su_state(x, xn);
    axpy_op(fom_apply_su_state__d_d1, s[0], x, xn, acc, FOM_STATE_DIM);
    axpy_op(fom_apply_su_state__d_g2, s[4], x, xn, acc, FOM_STATE_DIM);
    axpy_op(fom_apply_su_state__d_m1, s[5], x, xn, acc, FOM_STATE_DIM);
    fom_apply_su_t_t0(t0, xn);
    axpy_op(fom_apply_su_t_t0__d_d2, s[1], t0, xn, acc, FOM_STATE_DIM);
    axpy_op(fom_apply_su_t_t0__d_g1, s[3], t0, xn, acc, FOM_STATE_DIM);
    fom_apply_su_t_t1(t1, xn);
    axpy_op(fom_apply_su_t_t1__d_d2, s[1], t1, xn, acc, FOM_STATE_DIM);
    axpy_op(fom_apply_su_t_t1__d_g1, s[3], t1, xn, acc, FOM_STATE_DIM);
    axpy_op(fom_apply_su_t_t1__d_m2, s[6], t1, xn, acc, FOM_STATE_DIM);
}

fom_int fom_solve(FomWorkspace *work) {
    fom_float za[ZX_DIM], zb[ZX_DIM];
    fom_float *z = za, *zn = zb;
    fom_float b0[FOM_T0_DIM], b1[FOM_T1_DIM];
    fom_float s[NSCHED];
    const fom_int max_iter = work->settings.max_iter;
    fom_float res = 0;
    fom_int it, i;
    fom_int next = 0;
    int fused = 0;

    memcpy(z, work->state, sizeof(work->state));
    init_sched_runs();

    for (it = 0; it < max_iter; it++) {
        fom_float *xn = zn;
        const fom_float *x = z;

        FOM_PROF_BEGIN(sched);
        if (it >= next) {
            load_sched(it, s);
            next = (it < SCHED_LEN) ? g_next[it] : SCHED_BIG;
            fused = ensure_ops(s);
            param_terms(work, s, b0, b1, work->acc);
        }
        FOM_PROF_END(sched);

        if (!fused) {
            ref_step(work, s, x, xn);
        } else {
            fom_float *t0 = z + Z_T0, *t1 = z + Z_T1;

            FOM_PROF_BEGIN(t0);
            apply_fused(&g_op0, FOM_T0_DIM, b0, z, t0);
            for (i = 0; i < FOM_T0_DIM; i++) t0[i] = t0[i] > 0 ? t0[i] : 0;
            FOM_PROF_END(t0);

            FOM_PROF_BEGIN(t1);
            apply_fused(&g_op1, FOM_T1_DIM, b1, z, t1);
            for (i = 0; i < FOM_T1_DIM; i++) {
                fom_float v = t1[i];
                if (v < FOM_MAT_t1_lo[i]) v = FOM_MAT_t1_lo[i];
                if (v > FOM_MAT_t1_hi[i]) v = FOM_MAT_t1_hi[i];
                t1[i] = v;
            }
            FOM_PROF_END(t1);

            FOM_PROF_BEGIN(state_update);
            /* Rows nobody reads may be skipped unless the next
               iteration is the last, the residual/final state needs
               them, or the operators change at the next iteration. */
            if (g_op2.copy_mode && it + 2 < max_iter && it + 1 < next)
                copy_runs(g_op2.nlruns, g_op2.lrun_dst, g_op2.lrun_src,
                          g_op2.lrun_len, z, xn);
            else
                apply_fused(&g_op2, FOM_STATE_DIM, 0, z, xn);
            FOM_PROF_END(state_update);
        }

        if (it == max_iter - 1) {
            res = 0;
            for (i = 0; i < FOM_STATE_DIM; i++) {
                fom_float d = xn[i] - x[i];
                if (d < 0) d = -d;
                if (d > res) res = d;
            }
        }
        { fom_float *tmp = z; z = zn; zn = tmp; }
    }

    memcpy(work->state, z, sizeof(work->state));
    memcpy(work->solution.x, work->state, sizeof(work->state));
    work->solution.iter = it;
    work->solution.residual = res;
    work->solution.status = FOM_MAX_ITER_REACHED;
    return work->solution.status;
}

void fom_cleanup(FomWorkspace *work) {
    (void)work;
}
