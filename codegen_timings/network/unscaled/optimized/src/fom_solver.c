#include "fom_solver.h"
#include "fom_data.h"
#include "fom_ops.h"
#include "fom_linsolve.h"
#include "fom_profile.h"

/* Structure (verified at setup by probing the operators with unit vectors):
   state x = [s0 (NI x NJ grid, r = NJ*i + j), s1 (NI), s2 (NJ)].
     t0[r]      = clip(c0[r] + d0[r] x[r] + a0[r] s1[i] + w0[r] s2[j])
     t1[i]      = relu(c1[i] + sum_j e1[r] x[r] + f1[i] s1[i] + sum_j g1[r] t0[r])
     xn[r]      = h[r] t0[r]
     xn[NG+i]   = k1[i] t1[i]
     xn[NG+NI+j]= pc[j] + q[j] s2[j] + sum_i (u[r] x[r] + v[r] t0[r])
   If the probed pattern differs, the generic sparse path is used. */
#define NI FOM_T1_DIM
#define NJ FOM_PARAM_P0_DIM
#define NG FOM_T0_DIM
#define S1 NG
#define S2 (NG + NI)

#if defined(__clang__)
#define FOM_VEC _Pragma("clang loop vectorize(enable) interleave(disable)")
#else
#define FOM_VEC
#endif

static fom_float c_d0[NG], c_a0[NG], c_w0[NG];
static fom_float c_e1[NG], c_g1[NG], c_f1[NI];
static fom_float c_h[NG], c_k1[NI];
static fom_float c_u[NG], c_v[NG], c_q[NJ];
static int fast_ok = 0;
/* Specialized kernel: valid when d0 == 1, h == 1, lo == 0, u == w0 (elementwise)
   and a0, e1, g1 are uniform scalars (checked in fom_setup). */
static int spec_ok = 0;
static fom_float s_a0, s_e1, s_g1;

static fom_float probe_in[FOM_STATE_DIM];
static fom_float probe_out[FOM_STATE_DIM];

typedef void (*fom_op_fn)(const fom_float *, fom_float *);

/* Probe column `col` of op (in_dim inputs, out_dim outputs) into probe_out. */
static void probe_col(fom_op_fn op, fom_int in_dim, fom_int out_dim, fom_int col) {
    fom_int i;
    for (i = 0; i < in_dim; ++i) probe_in[i] = 0.0;
    for (i = 0; i < out_dim; ++i) probe_out[i] = 0.0;
    probe_in[col] = 1.0;
    op(probe_in, probe_out);
}

/* Grid row / column of flat index r = NJ*i + j (division-free). */
static fom_int grid_row(fom_int r) { fom_int i = 0; while (r >= NJ) { r -= NJ; ++i; } return i; }
static fom_int grid_col(fom_int r) { while (r >= NJ) r -= NJ; return r; }

static int fom_extract_structure(void) {
    fom_int i, j, c, k;
    int ok = 1;
    fom_float *dst;

    /* t0_state: col r -> row r (d0); col S1+i -> rows NJ*i+j (a0); col S2+j -> rows NJ*i+j (w0) */
    for (c = 0; c < FOM_STATE_DIM; ++c) {
        probe_col(fom_apply_t0_state, FOM_STATE_DIM, NG, c);
        for (k = 0; k < NG; ++k) {
            i = grid_row(k); j = grid_col(k);
            dst = 0;
            if (c < NG) { if (k == c) dst = &c_d0[k]; }
            else if (c < S2) { if (c - S1 == i) dst = &c_a0[k]; }
            else { if (c - S2 == j) dst = &c_w0[k]; }
            if (dst) *dst = probe_out[k];
            else if (probe_out[k] != 0.0) ok = 0;
        }
    }
    /* t1_state: col r -> row i (e1); col S1+i -> row i (f1); col S2+j -> nothing */
    for (c = 0; c < FOM_STATE_DIM; ++c) {
        probe_col(fom_apply_t1_state, FOM_STATE_DIM, NI, c);
        for (k = 0; k < NI; ++k) {
            dst = 0;
            if (c < NG) { if (grid_row(c) == k) dst = &c_e1[c]; }
            else if (c < S2) { if (c - S1 == k) dst = &c_f1[k]; }
            if (dst) *dst = probe_out[k];
            else if (probe_out[k] != 0.0) ok = 0;
        }
    }
    /* t1_t_t0: col r -> row i (g1) */
    for (c = 0; c < NG; ++c) {
        probe_col(fom_apply_t1_t_t0, NG, NI, c);
        for (k = 0; k < NI; ++k) {
            if (grid_row(c) == k) c_g1[c] = probe_out[k];
            else if (probe_out[k] != 0.0) ok = 0;
        }
    }
    /* su_state: col r -> row S2+j (u); col S2+j -> row S2+j (q); else zero */
    for (c = 0; c < FOM_STATE_DIM; ++c) {
        probe_col(fom_apply_su_state, FOM_STATE_DIM, FOM_STATE_DIM, c);
        for (k = 0; k < FOM_STATE_DIM; ++k) {
            dst = 0;
            if (c < NG) { if (k == S2 + grid_col(c)) dst = &c_u[c]; }
            else if (c >= S2) { if (k == c) dst = &c_q[c - S2]; }
            if (dst) *dst = probe_out[k];
            else if (probe_out[k] != 0.0) ok = 0;
        }
    }
    /* su_t_t0: col r -> row r (h), row S2+j (v) */
    for (c = 0; c < NG; ++c) {
        probe_col(fom_apply_su_t_t0, NG, FOM_STATE_DIM, c);
        for (k = 0; k < FOM_STATE_DIM; ++k) {
            if (k == c) c_h[c] = probe_out[k];
            else if (k == S2 + grid_col(c)) c_v[c] = probe_out[k];
            else if (probe_out[k] != 0.0) ok = 0;
        }
    }
    /* su_t_t1: col i -> row S1+i (k1) */
    for (c = 0; c < NI; ++c) {
        probe_col(fom_apply_su_t_t1, NI, FOM_STATE_DIM, c);
        for (k = 0; k < FOM_STATE_DIM; ++k) {
            if (k == S1 + c) c_k1[c] = probe_out[k];
            else if (probe_out[k] != 0.0) ok = 0;
        }
    }
    /* su_p_p0: only rows S2.. may be touched */
    for (c = 0; c < NJ; ++c) {
        probe_col(fom_apply_su_p_p0, NJ, FOM_STATE_DIM, c);
        for (k = 0; k < S2; ++k)
            if (probe_out[k] != 0.0) ok = 0;
    }
    return ok;
}

static int fom_check_special(void) {
    fom_int r;
    s_a0 = c_a0[0]; s_e1 = c_e1[0]; s_g1 = c_g1[0];
    for (r = 0; r < NG; ++r) {
        if (c_d0[r] != 1.0 || c_h[r] != 1.0 || FOM_MAT_t0_lo[r] != 0.0) return 0;
        if (c_u[r] != c_w0[r]) return 0;
        if (c_a0[r] != s_a0 || c_e1[r] != s_e1 || c_g1[r] != s_g1) return 0;
    }
    return 1;
}

static void fom_reset_state(FomWorkspace *work) {
    fom_int i;
    for (i = 0; i < FOM_STATE_DIM; ++i) work->state[i] = FOM_STATE_INIT[i];
}

/* pc = su_p_p0 @ p0 (iteration-invariant), stored in work->acc */
static void fom_param_const(FomWorkspace *work) {
    fom_int i;
    for (i = 0; i < FOM_STATE_DIM; ++i) work->acc[i] = 0.0;
    fom_apply_su_p_p0(work->params.p0, work->acc);
}

void fom_set_default_settings(FomSettings *settings) {
    settings->max_iter = FOM_MAX_ITER;
    settings->tol = FOM_TOL;
    settings->check_termination = FOM_CHECK_TERMINATION;
}

void fom_setup(FomWorkspace *work, const FomSettings *settings) {
    fom_int i;
    work->settings = *settings;
    for (i = 0; i < FOM_PARAM_P0_DIM; ++i) work->params.p0[i] = 0.0;
    fom_reset_state(work);
    fom_param_const(work);
    fast_ok = fom_extract_structure();
    spec_ok = fast_ok && fom_check_special();
    work->solution.iter = 0;
    work->solution.residual = 0.0;
    work->solution.status = FOM_MAX_ITER_REACHED;
}

void fom_update_params(FomWorkspace *work, const FomParams *params) {
    work->params = *params;
    fom_reset_state(work);
    fom_param_const(work);
}

/* Per iteration: elementwise t0 pass over the grid, column (over i) accumulation
   into s2, and row (over j) reductions for t1. Phases kept separate so each is
   a simple vectorizable loop. */
static void fom_iter_fast(const fom_float *restrict x, fom_float *restrict xn,
                          const fom_float *restrict pc) {
    fom_int i, j;
    fom_float acc[NJ];
    const fom_float *restrict x2 = x + S2;

    FOM_PROF_BEGIN(t0_s2);
    for (j = 0; j < NJ; ++j) acc[j] = pc[S2 + j] + c_q[j] * x2[j];
    for (i = 0; i < NI; ++i) {
        const fom_int b = i * NJ;
        const fom_float xs1 = x[S1 + i];
        fom_float se = 0.0, t1;
        FOM_VEC
        for (j = 0; j < NJ; ++j) {
            const fom_int r = b + j;
            const fom_float xr = x[r];
            fom_float t = FOM_MAT_t0_c[r] + ((c_d0[r] * xr + c_a0[r] * xs1) + c_w0[r] * x2[j]);
            t = t < FOM_MAT_t0_lo[r] ? FOM_MAT_t0_lo[r] : t;
            t = t > FOM_MAT_t0_hi[r] ? FOM_MAT_t0_hi[r] : t;
            xn[r] = c_h[r] * t;
            acc[j] += c_u[r] * xr + c_v[r] * t;
            se += c_e1[r] * xr + c_g1[r] * t;
        }
        t1 = FOM_MAT_t1_c[i] + c_f1[i] * xs1 + se;
        t1 = t1 > 0 ? t1 : 0.0;
        xn[S1 + i] = c_k1[i] * t1;
    }
    for (j = 0; j < NJ; ++j) xn[S2 + j] = acc[j];
    FOM_PROF_END(t0_s2);

}

/* Same pass as fom_iter_fast with the unit / zero / uniform / shared
   coefficients folded in: 5 streamed arrays per grid point instead of 11. */
static void fom_iter_special(const fom_float *restrict x, fom_float *restrict xn,
                             const fom_float *restrict pc) {
    fom_int i, j;
    fom_float acc[NJ];
    const fom_float *restrict x2 = x + S2;
    const fom_float a0 = s_a0, e1 = s_e1, g1 = s_g1;

    FOM_PROF_BEGIN(grid_special);
    for (j = 0; j < NJ; ++j) acc[j] = pc[S2 + j] + c_q[j] * x2[j];
    for (i = 0; i < NI; ++i) {
        const fom_int b = i * NJ;
        const fom_float xs1 = x[S1 + i];
        const fom_float as1 = a0 * xs1;
        fom_float sx = 0.0, st = 0.0, t1;
        FOM_VEC
        for (j = 0; j < NJ; ++j) {
            const fom_int r = b + j;
            const fom_float xr = x[r];
            const fom_float w = c_w0[r];
            fom_float t = FOM_MAT_t0_c[r] + ((xr + as1) + w * x2[j]);
            t = t < 0.0 ? 0.0 : t;
            t = t > FOM_MAT_t0_hi[r] ? FOM_MAT_t0_hi[r] : t;
            xn[r] = t;
            acc[j] += w * xr + c_v[r] * t;
            sx += xr;
            st += t;
        }
        t1 = FOM_MAT_t1_c[i] + c_f1[i] * xs1 + (e1 * sx + g1 * st);
        t1 = t1 > 0 ? t1 : 0.0;
        xn[S1 + i] = c_k1[i] * t1;
    }
    for (j = 0; j < NJ; ++j) xn[S2 + j] = acc[j];
    FOM_PROF_END(grid_special);
}

static void fom_iter_generic(FomWorkspace *work, const fom_float *x, fom_float *xn,
                             const fom_float *pc) {
    fom_float *t0 = work->t0;
    fom_float *t1 = work->t1;
    fom_int i;
    for (i = 0; i < FOM_T0_DIM; ++i) t0[i] = FOM_MAT_t0_c[i];
    fom_apply_t0_state(x, t0);
    for (i = 0; i < FOM_T0_DIM; ++i) {
        fom_float v = t0[i];
        v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v;
        v = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v;
        t0[i] = v;
    }
    for (i = 0; i < FOM_T1_DIM; ++i) t1[i] = FOM_MAT_t1_c[i];
    fom_apply_t1_state(x, t1);
    fom_apply_t1_t_t0(t0, t1);
    for (i = 0; i < FOM_T1_DIM; ++i) t1[i] = t1[i] > 0 ? t1[i] : 0.0;
    for (i = 0; i < FOM_STATE_DIM; ++i) xn[i] = pc[i];
    fom_apply_su_state(x, xn);
    fom_apply_su_t_t0(t0, xn);
    fom_apply_su_t_t1(t1, xn);
}

fom_int fom_solve(FomWorkspace *work) {
    fom_float *x = work->state;
    fom_float *xn = work->new_state;
    const fom_float *pc = work->acc; /* su_p_p0 @ p0, set in fom_update_params */
    const fom_int max_iter = work->settings.max_iter;
    const fom_float tol = work->settings.tol;
    const fom_int use_tol = work->settings.check_termination && tol > 0;
    fom_float res = 0.0;
    fom_int status = FOM_MAX_ITER_REACHED;
    fom_int k, i;

    for (k = 0; k < max_iter; ++k) {
        fom_float *sw;

        FOM_PROF_BEGIN(iterate);
        if (spec_ok) fom_iter_special(x, xn, pc);
        else if (fast_ok) fom_iter_fast(x, xn, pc);
        else fom_iter_generic(work, x, xn, pc);
        FOM_PROF_END(iterate);

        if (use_tol || k == max_iter - 1) {
            FOM_PROF_BEGIN(residual);
            res = 0.0;
            for (i = 0; i < FOM_STATE_DIM; ++i) {
                fom_float d = xn[i] - x[i];
                d = d < 0 ? -d : d;
                res = d > res ? d : res;
            }
            FOM_PROF_END(residual);
        }

        sw = x; x = xn; xn = sw;

        if (use_tol && res <= tol) {
            status = FOM_SOLVED;
            ++k;
            break;
        }
    }

    for (i = 0; i < FOM_STATE_DIM; ++i) work->solution.x[i] = x[i];
    if (x != work->state)
        for (i = 0; i < FOM_STATE_DIM; ++i) work->state[i] = x[i];
    work->solution.iter = k;
    work->solution.residual = res;
    work->solution.status = status;
    return status;
}

void fom_cleanup(FomWorkspace *work) {
    (void)work;
}
