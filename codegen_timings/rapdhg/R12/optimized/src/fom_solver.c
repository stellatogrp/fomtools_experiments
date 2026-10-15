#include <string.h>

#include "fom_solver.h"
#include "fom_data.h"
#include "fom_ops.h"
#include "fom_linsolve.h"
#include "fom_profile.h"

#define S_DIM FOM_STATE_DIM
#define T0_DIM FOM_T0_DIM
#define T1_DIM FOM_T1_DIM

/* State slots (see spec.json): s0 x | s1 y | s2 | s3 | s4 x_prev | s5 */
#define NX FOM_PARAM_P0_DIM /* 16 */
#define NY FOM_PARAM_P1_DIM /* 48 */
#define OFF_S0 0
#define OFF_S1 16
#define OFF_S2 64
#define OFF_S3 80
#define OFF_S4 128
#define OFF_S5 144

/* Operator blocks extracted once (at setup) from the fom_ops.c helpers.
   The remaining terms of the iteration are identity / -identity / zero
   blocks, applied directly below.
     K  (NY x NX, CSC): t0_state__d_taux restricted to s0 columns;
                        t0_state__d_tth on s4 columns equals -K.
     C  (NX x NY, CSR): t1_t_t0__d_eta (= -K^T).
     E  (NX x NX, dense column-major): t1_state__d_eta restricted to s5 columns. */
static fom_int K_colptr[NX + 1];
static fom_int K_row[NY * NX];
static fom_float K_val[NY * NX];
static fom_int C_rowptr[NX + 1];
static fom_int C_col[NX * NY];
static fom_float C_val[NX * NY];
static fom_float E_cm[NX * NX];
static int ops_ready = 0;

typedef void (*apply_fn)(const fom_float *, fom_float *);

/* Probe columns [col_off, col_off+ncols) of an operator with unit vectors
   and store the nrows x ncols block densely, column-major. */
static void extract_dense_cm(apply_fn f, int col_off, int ncols, int nrows,
                             fom_float *dense) {
    fom_float e[S_DIM];
    fom_float out[S_DIM];
    memset(e, 0, sizeof(e));
    for (int j = 0; j < ncols; ++j) {
        memset(out, 0, sizeof(out));
        e[col_off + j] = 1.0;
        f(e, out);
        e[col_off + j] = 0.0;
        memcpy(dense + j * nrows, out, (size_t)nrows * sizeof(fom_float));
    }
}

/* Same, but stored as CSR (exact zeros dropped). */
static void extract_csr(apply_fn f, int col_off, int ncols, int nrows,
                        fom_int *rowptr, fom_int *col, fom_float *val) {
    fom_float dense[NY * NX];
    extract_dense_cm(f, col_off, ncols, nrows, dense);
    fom_int p = 0;
    for (int i = 0; i < nrows; ++i) {
        rowptr[i] = p;
        for (int j = 0; j < ncols; ++j) {
            const fom_float v = dense[j * nrows + i];
            if (v != 0.0) {
                col[p] = j;
                val[p] = v;
                ++p;
            }
        }
    }
    rowptr[nrows] = p;
}

/* Same, but stored as CSC (exact zeros dropped). */
static void extract_csc(apply_fn f, int col_off, int ncols, int nrows,
                        fom_int *colptr, fom_int *row, fom_float *val) {
    fom_float dense[NY * NX];
    extract_dense_cm(f, col_off, ncols, nrows, dense);
    fom_int p = 0;
    for (int j = 0; j < ncols; ++j) {
        colptr[j] = p;
        for (int i = 0; i < nrows; ++i) {
            const fom_float v = dense[j * nrows + i];
            if (v != 0.0) {
                row[p] = i;
                val[p] = v;
                ++p;
            }
        }
    }
    colptr[ncols] = p;
}

static void init_ops(void) {
    if (ops_ready) return;
    extract_csc(fom_apply_t0_state__d_taux, OFF_S0, NX, NY, K_colptr, K_row, K_val);
    extract_csr(fom_apply_t1_t_t0__d_eta, 0, NY, NX, C_rowptr, C_col, C_val);
    extract_dense_cm(fom_apply_t1_state__d_eta, OFF_S5, NX, NX, E_cm);
    ops_ready = 1;
}

/* x <- su(x, t0, t1): identity-block state update, in place. */
static inline void state_update(fom_float *restrict x,
                                const fom_float *restrict t0,
                                const fom_float *restrict t1,
                                fom_float d1, fom_float d2, fom_float g1,
                                fom_float g2, fom_float m1, fom_float m2) {
    for (int i = 0; i < NX; ++i) {
        const fom_float a0 = x[OFF_S0 + i];
        const fom_float a2 = x[OFF_S2 + i];
        const fom_float u = t1[i];
        x[OFF_S0 + i] = g2 * a2 + g1 * u;
        x[OFF_S2 + i] = d1 * a2 + d2 * u;
        x[OFF_S4 + i] = a0;
        x[OFF_S5 + i] = m1 * a2 + m2 * u;
    }
    for (int i = 0; i < NY; ++i) {
        const fom_float a3 = x[OFF_S3 + i];
        const fom_float v = t0[i];
        x[OFF_S1 + i] = g2 * a3 + g1 * v;
        x[OFF_S3 + i] = d1 * a3 + d2 * v;
    }
}

/* Same update, also returning max_abs(x_new - x_old). */
static inline fom_float state_update_res(fom_float *restrict x,
                                         const fom_float *restrict t0,
                                         const fom_float *restrict t1,
                                         fom_float d1, fom_float d2,
                                         fom_float g1, fom_float g2,
                                         fom_float m1, fom_float m2) {
    fom_float res = 0.0;
    for (int i = 0; i < NX; ++i) {
        const fom_float a0 = x[OFF_S0 + i];
        const fom_float a2 = x[OFF_S2 + i];
        const fom_float a4 = x[OFF_S4 + i];
        const fom_float a5 = x[OFF_S5 + i];
        const fom_float u = t1[i];
        const fom_float n0 = g2 * a2 + g1 * u;
        const fom_float n2 = d1 * a2 + d2 * u;
        const fom_float n5 = m1 * a2 + m2 * u;
        fom_float r0 = n0 - a0, r2 = n2 - a2, r4 = a0 - a4, r5 = n5 - a5;
        r0 = r0 < 0.0 ? -r0 : r0;
        r2 = r2 < 0.0 ? -r2 : r2;
        r4 = r4 < 0.0 ? -r4 : r4;
        r5 = r5 < 0.0 ? -r5 : r5;
        res = r0 > res ? r0 : res;
        res = r2 > res ? r2 : res;
        res = r4 > res ? r4 : res;
        res = r5 > res ? r5 : res;
        x[OFF_S0 + i] = n0;
        x[OFF_S2 + i] = n2;
        x[OFF_S4 + i] = a0;
        x[OFF_S5 + i] = n5;
    }
    for (int i = 0; i < NY; ++i) {
        const fom_float a1 = x[OFF_S1 + i];
        const fom_float a3 = x[OFF_S3 + i];
        const fom_float v = t0[i];
        const fom_float n1 = g2 * a3 + g1 * v;
        const fom_float n3 = d1 * a3 + d2 * v;
        fom_float r1 = n1 - a1, r3 = n3 - a3;
        r1 = r1 < 0.0 ? -r1 : r1;
        r3 = r3 < 0.0 ? -r3 : r3;
        res = r1 > res ? r1 : res;
        res = r3 > res ? r3 : res;
        x[OFF_S1 + i] = n1;
        x[OFF_S3 + i] = n3;
    }
    return res;
}

void fom_set_default_settings(FomSettings *settings) {
    settings->max_iter = FOM_MAX_ITER;
    settings->tol = FOM_TOL;
    settings->check_termination = FOM_CHECK_TERMINATION;
}

void fom_setup(FomWorkspace *work, const FomSettings *settings) {
    init_ops();
    work->settings = *settings;
    memcpy(work->state, FOM_STATE_INIT, sizeof(work->state));
    memset(work->params.p0, 0, sizeof(work->params.p0));
    memset(work->params.p1, 0, sizeof(work->params.p1));
    work->solution.iter = 0;
    work->solution.residual = 0.0;
    work->solution.status = FOM_MAX_ITER_REACHED;
}

void fom_update_params(FomWorkspace *work, const FomParams *params) {
    work->params = *params;
    memcpy(work->state, FOM_STATE_INIT, sizeof(work->state));
}

fom_int fom_solve(FomWorkspace *work) {
    fom_float *restrict x = work->state;
    fom_float *restrict w = work->acc;
    fom_float *restrict t0 = work->t0;
    fom_float *restrict t1 = work->t1;
    const fom_float *restrict p0 = work->params.p0;
    const fom_float *restrict p1 = work->params.p1;
    const fom_int max_iter = work->settings.max_iter;
    const int use_tol = FOM_USE_TOL && work->settings.check_termination;
    const fom_float tol = work->settings.tol;

    fom_float res = 0.0;
    fom_int status = FOM_MAX_ITER_REACHED;
    fom_int it;

    memcpy(x, FOM_STATE_INIT, sizeof(work->state));

    for (it = 0; it < max_iter; ++it) {
        const fom_float d1 = fom_sched_d1(it);
        const fom_float d2 = fom_sched_d2(it);
        const fom_float eta = fom_sched_eta(it);
        const fom_float g1 = fom_sched_g1(it);
        const fom_float g2 = fom_sched_g2(it);
        const fom_float m1 = fom_sched_m1(it);
        const fom_float m2 = fom_sched_m2(it);
        const fom_float tau = fom_sched_tau(it);
        const fom_float taux = fom_sched_taux(it);
        const fom_float tth = fom_sched_tth(it);

        /* t0 = relu(s1 + K (taux*s0 - tth*s4) - tau*p1) */
        FOM_PROF_BEGIN(t0);
        for (int i = 0; i < NY; ++i) t0[i] = x[OFF_S1 + i] - tau * p1[i];
        for (int j = 0; j < NX; ++j) {
            const fom_float wj = taux * x[OFF_S0 + j] - tth * x[OFF_S4 + j];
            for (fom_int p = K_colptr[j]; p < K_colptr[j + 1]; ++p)
                t0[K_row[p]] += K_val[p] * wj;
        }
        for (int i = 0; i < NY; ++i) t0[i] = t0[i] > 0.0 ? t0[i] : 0.0;
        FOM_PROF_END(t0);

        /* t1 = clip(s0 + eta*(E s5 + C t0 - p0)) */
        FOM_PROF_BEGIN(t1);
        {
            /* q = E s5 - p0, two interleaved accumulators over columns */
            fom_float q[NX], qb[NX];
            for (int i = 0; i < NX; ++i) {
                q[i] = -p0[i];
                qb[i] = 0.0;
            }
            for (int k = 0; k < NX; k += 2) {
                const fom_float sa = x[OFF_S5 + k];
                const fom_float sb = x[OFF_S5 + k + 1];
                for (int i = 0; i < NX; ++i) {
                    q[i] += E_cm[k * NX + i] * sa;
                    qb[i] += E_cm[(k + 1) * NX + i] * sb;
                }
            }
            for (int i = 0; i < NX; ++i) q[i] += qb[i];
            for (int i = 0; i < NX; ++i) {
                /* row of C t0, two accumulators to shorten the chain */
                fom_float sa = 0.0, sb = 0.0;
                const fom_int pe = C_rowptr[i + 1];
                fom_int p = C_rowptr[i];
                for (; p + 1 < pe; p += 2) {
                    sa += C_val[p] * t0[C_col[p]];
                    sb += C_val[p + 1] * t0[C_col[p + 1]];
                }
                if (p < pe) sa += C_val[p] * t0[C_col[p]];
                const fom_float s = q[i] + (sa + sb);
                fom_float v = x[OFF_S0 + i] + eta * s;
                const fom_float lo = FOM_MAT_t1_lo[i];
                const fom_float hi = FOM_MAT_t1_hi[i];
                v = v < lo ? lo : v;
                v = v > hi ? hi : v;
                t1[i] = v;
            }
        }
        FOM_PROF_END(t1);

        /* state update (identity blocks only), in place. The residual is
           only observable on the last iteration unless tol-checking is on. */
        FOM_PROF_BEGIN(su);
        if (use_tol || it + 1 >= max_iter) {
            res = state_update_res(x, t0, t1, d1, d2, g1, g2, m1, m2);
        } else {
            state_update(x, t0, t1, d1, d2, g1, g2, m1, m2);
        }
        FOM_PROF_END(su);

        if (use_tol && res <= tol) {
            ++it;
            status = FOM_SOLVED;
            break;
        }
    }

    memcpy(work->solution.x, x, sizeof(work->solution.x));
    work->solution.iter = it;
    work->solution.residual = res;
    work->solution.status = status;
    return status;
}

void fom_cleanup(FomWorkspace *work) {
    (void)work;
}
