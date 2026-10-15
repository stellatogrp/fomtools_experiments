#include "fom_solver.h"
#include "fom_data.h"
#include "fom_ops.h"
#include "fom_linsolve.h"
#include "fom_profile.h"

/* ------------------------------------------------------------------
   Grid structure.

   The state is x = [s0 (NR*NC) | s1 (NR) | s2 (NC)] with NR = 10,
   NC = 15, and s0 index i = NC*r + c lives on an NR x NC grid. Every
   operator in this method only couples grid cell i to its row r and
   column c:
     t0[i]    = clip(t0_c[i] + a[i] x[i] + b[i] s1[r] + g[i] s2[c])
     t1[r]    = relu(t1_c[r] + sum_c (A + xi Ad)[i] s0[i]
                             + sum_c (B + xi Bd)[i] t0[i] + (h + xi hd)[r] s1[r])
     s0+      = t0,  s1+ = t1            (identity blocks)
     s2+[c]   = (q + xi qd)[c] s2[c] + sum_r (S + xi Sd)[i] s0[i]
               + sum_r (T + xi Td)[i] t0[i] + (su_p_p0 @ p0)[c]
   so one sweep over the grid with contiguous, index-free coefficient
   arrays computes the whole iteration.

   The coefficients are recovered once (first fom_setup) by probing the
   provided fom_apply_* helpers with unit vectors. Every probed nonzero
   must land in one of the slots above (identity blocks exactly 1);
   otherwise the solver falls back to the reference helper loop.
   ------------------------------------------------------------------ */

#define NR FOM_T1_DIM
#define NC (FOM_STATE_DIM - FOM_T0_DIM - FOM_T1_DIM)
#define S1 FOM_T0_DIM
#define S2 (FOM_T0_DIM + FOM_T1_DIM)

typedef char fom_grid_check[(NR * NC == FOM_T0_DIM) ? 1 : -1];

/* Internal layout pads each grid row to NCP columns (zero coefficients,
   zero bounds) so every inner loop has a fixed, SIMD-friendly length:
   [s0 (NR*NCP) | s1 (NR) | s2 (NCP)]. */
#define NCP 16
#define G   (NR * NCP)
#define PS1 G
#define PS2 (G + NR)
#define PLEN (PS2 + NCP)

typedef void (*fom_apply_fn)(const fom_float *, fom_float *);

/* t0 */
static fom_float g_c0[G], g_lo[G], g_hi[G];
static fom_float g_a[G], g_b[G], g_g[G];
/* t1 */
static fom_float g_A[G], g_Ad[G], g_B[G], g_Bd[G];
static fom_float g_h[NR], g_hd[NR];
/* s2 update */
static fom_float g_S[G], g_Sd[G], g_T[G], g_Td[G];
static fom_float g_q[NCP], g_qd[NCP];
static fom_float g_ones_s0[FOM_T0_DIM], g_ones_s1[NR];

static int g_struct_state = 0; /* 0 = not built, 1 = ok, -1 = fallback */
/* 1 if t0_state has unit diagonal on s0 and t0_lo == 0 (checked exactly). */
static int g_unit_lo0 = 0;

static fom_float g_probe_in[FOM_MAX_VEC_DIM];
static fom_float g_probe_out[FOM_MAX_VEC_DIM];
static fom_int g_row_of[FOM_T0_DIM], g_col_of[FOM_T0_DIM], g_pad[FOM_T0_DIM];

enum { M_T0, M_T1, M_T1D, M_T1T0, M_T1T0D, M_SU, M_SUD, M_SUT0, M_SUT0D, M_SUT1, M_SUP };

/* Map entry (i, j) of matrix m to its coefficient slot, or 0 if the
   entry is outside the grid pattern. */
static fom_float *fom_slot(int m, fom_int i, fom_int j) {
    switch (m) {
    case M_T0:
        if (i >= FOM_T0_DIM) return 0;
        if (j == i) return &g_a[g_pad[i]];
        if (j == S1 + g_row_of[i]) return &g_b[g_pad[i]];
        if (j == S2 + g_col_of[i]) return &g_g[g_pad[i]];
        return 0;
    case M_T1: case M_T1D:
        if (i >= NR) return 0;
        if (j < FOM_T0_DIM && g_row_of[j] == i) return m == M_T1 ? &g_A[g_pad[j]] : &g_Ad[g_pad[j]];
        if (j == S1 + i) return m == M_T1 ? &g_h[i] : &g_hd[i];
        return 0;
    case M_T1T0: case M_T1T0D:
        if (i >= NR || g_row_of[j] != i) return 0;
        return m == M_T1T0 ? &g_B[g_pad[j]] : &g_Bd[g_pad[j]];
    case M_SU: case M_SUD:
        if (i < S2) return 0;
        if (j < FOM_T0_DIM && g_col_of[j] == i - S2) return m == M_SU ? &g_S[g_pad[j]] : &g_Sd[g_pad[j]];
        if (j == i) return m == M_SU ? &g_q[i - S2] : &g_qd[i - S2];
        return 0;
    case M_SUT0:
        if (i < FOM_T0_DIM) return j == i ? &g_ones_s0[i] : 0;
        if (i < S2 || g_col_of[j] != i - S2) return 0;
        return &g_T[g_pad[j]];
    case M_SUT0D:
        if (i < S2 || g_col_of[j] != i - S2) return 0;
        return &g_Td[g_pad[j]];
    case M_SUT1:
        return (i >= S1 && i < S2 && j == i - S1) ? &g_ones_s1[j] : 0;
    case M_SUP:
        return i >= S2 ? &g_probe_in[0] /* any non-null: p0 term is applied per solve */ : 0;
    }
    return 0;
}

static int fom_probe_matrix(int m, fom_apply_fn fn, fom_int nin, fom_int nout) {
    fom_int i, j;
    for (j = 0; j < nin; ++j) {
        fom_float *slot;
        for (i = 0; i < nout; ++i) g_probe_out[i] = 0.0;
        g_probe_in[j] = 1.0;
        fn(g_probe_in, g_probe_out);
        g_probe_in[j] = 0.0;
        for (i = 0; i < nout; ++i) {
            if (g_probe_out[i] == 0.0) continue;
            slot = fom_slot(m, i, j);
            if (!slot) return -1;
            if (m != M_SUP) *slot = g_probe_out[i];
        }
    }
    return 0;
}

static void fom_build_all(void) {
    fom_int r, c, i;
    int err = 0;
    for (r = 0, i = 0; r < NR; ++r)
        for (c = 0; c < NC; ++c, ++i) {
            g_row_of[i] = r; g_col_of[i] = c; g_pad[i] = r * NCP + c;
            g_c0[r * NCP + c] = FOM_MAT_t0_c[i];
            g_lo[r * NCP + c] = FOM_MAT_t0_lo[i];
            g_hi[r * NCP + c] = FOM_MAT_t0_hi[i];
        }
    for (i = 0; i < FOM_MAX_VEC_DIM; ++i) g_probe_in[i] = 0.0;

    err |= fom_probe_matrix(M_T0, fom_apply_t0_state, FOM_STATE_DIM, FOM_T0_DIM);
    err |= fom_probe_matrix(M_T1, fom_apply_t1_state, FOM_STATE_DIM, NR);
    err |= fom_probe_matrix(M_T1D, fom_apply_t1_state__d_xi, FOM_STATE_DIM, NR);
    err |= fom_probe_matrix(M_T1T0, fom_apply_t1_t_t0, FOM_T0_DIM, NR);
    err |= fom_probe_matrix(M_T1T0D, fom_apply_t1_t_t0__d_xi, FOM_T0_DIM, NR);
    err |= fom_probe_matrix(M_SU, fom_apply_su_state, FOM_STATE_DIM, FOM_STATE_DIM);
    err |= fom_probe_matrix(M_SUD, fom_apply_su_state__d_xi, FOM_STATE_DIM, FOM_STATE_DIM);
    err |= fom_probe_matrix(M_SUT0, fom_apply_su_t_t0, FOM_T0_DIM, FOM_STATE_DIM);
    err |= fom_probe_matrix(M_SUT0D, fom_apply_su_t_t0__d_xi, FOM_T0_DIM, FOM_STATE_DIM);
    err |= fom_probe_matrix(M_SUT1, fom_apply_su_t_t1, FOM_T1_DIM, FOM_STATE_DIM);
    err |= fom_probe_matrix(M_SUP, fom_apply_su_p_p0, FOM_PARAM_P0_DIM, FOM_STATE_DIM);
    /* s0+ = t0 and s1+ = t1 must be exact identities. */
    for (i = 0; i < FOM_T0_DIM; ++i) if (g_ones_s0[i] != 1.0) err = 1;
    for (i = 0; i < NR; ++i) if (g_ones_s1[i] != 1.0) err = 1;
    g_unit_lo0 = 1;
    for (r = 0; r < NR; ++r)
        for (c = 0; c < NC; ++c)
            if (g_a[r * NCP + c] != 1.0 || g_lo[r * NCP + c] != 0.0) g_unit_lo0 = 0;
    g_struct_state = err ? -1 : 1;
}

void fom_set_default_settings(FomSettings *settings) {
    settings->max_iter = FOM_MAX_ITER;
    settings->tol = FOM_TOL;
    settings->check_termination = FOM_CHECK_TERMINATION;
}

static void fom_reset_state(FomWorkspace *work) {
    fom_int i;
    for (i = 0; i < FOM_STATE_DIM; ++i) work->state[i] = FOM_STATE_INIT[i];
}

void fom_setup(FomWorkspace *work, const FomSettings *settings) {
    fom_int i;
    if (g_struct_state == 0) fom_build_all();
    work->settings = *settings;
    for (i = 0; i < FOM_PARAM_P0_DIM; ++i) work->params.p0[i] = 0.0;
    fom_reset_state(work);
    work->solution.iter = 0;
    work->solution.residual = 0.0;
    work->solution.status = FOM_MAX_ITER_REACHED;
}

void fom_update_params(FomWorkspace *work, const FomParams *params) {
    work->params = *params;
    fom_reset_state(work);
}

/* Reference loop (used only if the fused operators could not be built). */
static fom_float fom_solve_reference(FomWorkspace *work, fom_float **xout) {
    fom_float *x = work->state;
    fom_float *xn = work->new_state;
    fom_float *acc = work->acc;
    fom_float *t0 = work->t0;
    fom_float *t1 = work->t1;
    fom_float res = 0.0;
    const fom_int max_iter = work->settings.max_iter;
    fom_int it, i;

    for (it = 0; it < max_iter; ++it) {
        const fom_float xi = fom_sched_xi(it);
        for (i = 0; i < FOM_T0_DIM; ++i) t0[i] = FOM_MAT_t0_c[i];
        fom_apply_t0_state(x, t0);
        for (i = 0; i < FOM_T0_DIM; ++i) {
            fom_float v = t0[i];
            const fom_float lo = FOM_MAT_t0_lo[i];
            const fom_float hi = FOM_MAT_t0_hi[i];
            v = v < lo ? lo : v;
            v = v > hi ? hi : v;
            t0[i] = v;
        }
        for (i = 0; i < FOM_T1_DIM; ++i) { t1[i] = FOM_MAT_t1_c[i]; acc[i] = 0.0; }
        fom_apply_t1_state(x, t1);
        fom_apply_t1_t_t0(t0, t1);
        fom_apply_t1_state__d_xi(x, acc);
        fom_apply_t1_t_t0__d_xi(t0, acc);
        for (i = 0; i < FOM_T1_DIM; ++i) {
            const fom_float v = t1[i] + xi * acc[i];
            t1[i] = v > 0.0 ? v : 0.0;
        }
        for (i = 0; i < FOM_STATE_DIM; ++i) { xn[i] = 0.0; acc[i] = 0.0; }
        fom_apply_su_state(x, xn);
        fom_apply_su_t_t0(t0, xn);
        fom_apply_su_t_t1(t1, xn);
        fom_apply_su_p_p0(work->params.p0, xn);
        fom_apply_su_state__d_xi(x, acc);
        fom_apply_su_t_t0__d_xi(t0, acc);
        for (i = 0; i < FOM_STATE_DIM; ++i) xn[i] += xi * acc[i];
        if (it + 1 == max_iter) {
            res = 0.0;
            for (i = 0; i < FOM_STATE_DIM; ++i) {
                fom_float d = xn[i] - x[i];
                d = d < 0.0 ? -d : d;
                res = d > res ? d : res;
            }
        }
        { fom_float *tmp = x; x = xn; xn = tmp; }
    }
    *xout = x;
    return res;
}

/* One full iteration on the padded grid layout: reads x, writes xn.
   `unit_lo0` (compile-time constant at each call site) specializes the
   t0 activation for a == 1 and lo == 0, dropping two coefficient
   streams from the bandwidth-bound inner loop. */
static inline __attribute__((always_inline))
void fom_sweep(const fom_float *restrict x, fom_float *restrict xn,
               const fom_float *restrict pc2, const fom_float xi, const int unit_lo0) {
    const fom_float *x1 = x + PS1;
    const fom_float *x2 = x + PS2;
    fom_float *t1 = xn + PS1;
    fom_float s2a[NCP], s2d[NCP];
    fom_int r, c, i;

    for (c = 0; c < NCP; ++c) {
        s2a[c] = pc2[c] + g_q[c] * x2[c];
        s2d[c] = g_qd[c] * x2[c];
    }
    for (r = 0, i = 0; r < NR; ++r, i += NCP) {
        const fom_float x1r = x1[r];
        fom_float ta = 0.0, td = 0.0;
        fom_float *t0 = xn + i; /* s0+ = t0 */
        const fom_float *xs = x + i;
        for (c = 0; c < NCP; ++c) {
            const fom_float xv = xs[c];
            fom_float v = g_c0[i + c] + (unit_lo0 ? xv : g_a[i + c] * xv)
                          + g_b[i + c] * x1r + g_g[i + c] * x2[c];
            const fom_float lo = unit_lo0 ? 0.0 : g_lo[i + c];
            const fom_float hi = g_hi[i + c];
            v = v < lo ? lo : v;
            v = v > hi ? hi : v;
            t0[c] = v;
            ta += g_A[i + c] * xv + g_B[i + c] * v;
            td += g_Ad[i + c] * xv + g_Bd[i + c] * v;
            s2a[c] += g_S[i + c] * xv + g_T[i + c] * v;
            s2d[c] += g_Sd[i + c] * xv + g_Td[i + c] * v;
        }
        ta += FOM_MAT_t1_c[r] + g_h[r] * x1r + xi * (td + g_hd[r] * x1r);
        t1[r] = ta > 0.0 ? ta : 0.0; /* s1+ = t1 */
    }
    for (c = 0; c < NCP; ++c) xn[PS2 + c] = s2a[c] + xi * s2d[c];
}

fom_int fom_solve(FomWorkspace *work) {
    fom_float buf[2][PLEN];
    fom_float pc[FOM_STATE_DIM];
    fom_float pc2[NCP];
    fom_float *x = buf[0];
    fom_float *xn = buf[1];
    fom_float res = 0.0;
    const fom_int max_iter = work->settings.max_iter;
    fom_int it, i, r, c;

    if (g_struct_state == 0) fom_build_all();
    if (g_struct_state < 0) {
        fom_float *xr;
        res = fom_solve_reference(work, &xr);
        if (xr != work->state)
            for (i = 0; i < FOM_STATE_DIM; ++i) work->state[i] = xr[i];
        for (i = 0; i < FOM_STATE_DIM; ++i) work->solution.x[i] = xr[i];
        work->solution.iter = max_iter;
        work->solution.residual = res;
        work->solution.status = FOM_MAX_ITER_REACHED;
        return FOM_MAX_ITER_REACHED;
    }

    /* Iteration-invariant parameter term su_p_p0 @ p0 (only s2 rows). */
    FOM_PROF_BEGIN(solve_prologue);
    for (i = 0; i < FOM_STATE_DIM; ++i) pc[i] = 0.0;
    fom_apply_su_p_p0(work->params.p0, pc);

    /* Scatter state into the padded internal layout. */
    for (i = 0; i < PLEN; ++i) { x[i] = 0.0; xn[i] = 0.0; }
    for (r = 0, i = 0; r < NR; ++r)
        for (c = 0; c < NC; ++c, ++i) x[r * NCP + c] = work->state[i];
    for (r = 0; r < NR; ++r) x[PS1 + r] = work->state[S1 + r];
    for (c = 0; c < NC; ++c) { x[PS2 + c] = work->state[S2 + c]; pc2[c] = pc[S2 + c]; }
    for (; c < NCP; ++c) pc2[c] = 0.0;
    FOM_PROF_END(solve_prologue);

    for (it = 0; it < max_iter; ++it) {
        const fom_float xi = fom_sched_xi(it);
        FOM_PROF_BEGIN(grid_sweep);
        if (g_unit_lo0) fom_sweep(x, xn, pc2, xi, 1);
        else fom_sweep(x, xn, pc2, xi, 0);
        FOM_PROF_END(grid_sweep);

        if (it + 1 == max_iter) {
            FOM_PROF_BEGIN(residual);
            res = 0.0;
            for (i = 0; i < PLEN; ++i) {
                fom_float d = xn[i] - x[i];
                d = d < 0.0 ? -d : d;
                res = d > res ? d : res;
            }
            FOM_PROF_END(residual);
        }

        { fom_float *tmp = x; x = xn; xn = tmp; }
    }

    /* Gather back to the public layout. */
    FOM_PROF_BEGIN(writeback);
    for (r = 0, i = 0; r < NR; ++r)
        for (c = 0; c < NC; ++c, ++i) work->solution.x[i] = x[r * NCP + c];
    for (r = 0; r < NR; ++r) work->solution.x[S1 + r] = x[PS1 + r];
    for (c = 0; c < NC; ++c) work->solution.x[S2 + c] = x[PS2 + c];
    for (i = 0; i < FOM_STATE_DIM; ++i) work->state[i] = work->solution.x[i];
    FOM_PROF_END(writeback);
    work->solution.iter = max_iter;
    work->solution.residual = res;
    work->solution.status = FOM_MAX_ITER_REACHED;
    return FOM_MAX_ITER_REACHED;
}

void fom_cleanup(FomWorkspace *work) {
    (void)work;
}
