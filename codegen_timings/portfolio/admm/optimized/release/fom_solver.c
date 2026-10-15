#include "fom_solver.h"
#include "fom_data.h"
#include "fom_ops.h"
#include "fom_linsolve.h"

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

fom_int fom_solve(FomWorkspace *work) {
    const fom_float *p0 = work->params.p0;
    fom_float *x = work->state;
    fom_float *xn = work->new_state;
    fom_float *t0 = work->t0, *t1 = work->t1, *t2 = work->t2, *t3 = work->t3;
    fom_float *acc = work->acc;
    const fom_int max_iter = work->settings.max_iter;
    const fom_float tol = work->settings.tol;
    const fom_int use_tol = work->settings.check_termination && tol > 0.0;
    fom_float res = 0.0;
    fom_int status = FOM_MAX_ITER_REACHED;
    fom_int k, i;

    /* Parameter-only contributions are loop-invariant: precompute once. */
    fom_float c1[FOM_T1_DIM], c2[FOM_T2_DIM], c3[FOM_T3_DIM], cs[FOM_STATE_DIM];
    for (i = 0; i < FOM_T1_DIM; ++i) c1[i] = 0.0;
    for (i = 0; i < FOM_T2_DIM; ++i) c2[i] = 0.0;
    for (i = 0; i < FOM_T3_DIM; ++i) c3[i] = FOM_MAT_t3_c[i];
    for (i = 0; i < FOM_STATE_DIM; ++i) cs[i] = FOM_MAT_su_c[i];
    fom_apply_t1_p_p0(p0, c1);
    fom_apply_t2_p_p0(p0, c2);
    fom_apply_t3_p_p0(p0, c3);
    fom_apply_su_p_p0(p0, cs);

    for (k = 0; k < max_iter; ++k) {
        /* t0 = clip(t0_state @ x, lo, hi) */
        for (i = 0; i < FOM_T0_DIM; ++i) t0[i] = 0.0;
        fom_apply_t0_state(x, t0);
        for (i = 0; i < FOM_T0_DIM; ++i) {
            fom_float v = t0[i];
            const fom_float lo = FOM_MAT_t0_lo[i], hi = FOM_MAT_t0_hi[i];
            v = v < lo ? lo : v;
            v = v > hi ? hi : v;
            t0[i] = v;
        }

        /* t1 = relu(t1_state @ x + t1_p_p0 @ p0) */
        for (i = 0; i < FOM_T1_DIM; ++i) t1[i] = c1[i];
        fom_apply_t1_state(x, t1);
        for (i = 0; i < FOM_T1_DIM; ++i) t1[i] = t1[i] > 0.0 ? t1[i] : 0.0;

        /* t2 = relu(t2_state @ x + t2_p_p0 @ p0) */
        for (i = 0; i < FOM_T2_DIM; ++i) t2[i] = c2[i];
        fom_apply_t2_state(x, t2);
        for (i = 0; i < FOM_T2_DIM; ++i) t2[i] = t2[i] > 0.0 ? t2[i] : 0.0;

        /* t3: solve M t3 = rhs */
        for (i = 0; i < FOM_T3_DIM; ++i) acc[i] = c3[i];
        fom_apply_t3_state(x, acc);
        fom_apply_t3_t_t0(t0, acc);
        fom_apply_t3_t_t1(t1, acc);
        fom_apply_t3_t_t2(t2, acc);
        fom_lin_solve_t3(acc, t3);

        /* state update */
        for (i = 0; i < FOM_STATE_DIM; ++i) xn[i] = cs[i];
        fom_apply_su_state(x, xn);
        fom_apply_su_t_t0(t0, xn);
        fom_apply_su_t_t1(t1, xn);
        fom_apply_su_t_t2(t2, xn);
        fom_apply_su_t_t3(t3, xn);

        if (use_tol || k == max_iter - 1) {
            res = 0.0;
            for (i = 0; i < FOM_STATE_DIM; ++i) {
                fom_float d = xn[i] - x[i];
                d = d < 0.0 ? -d : d;
                res = d > res ? d : res;
            }
        }

        { fom_float *tmp = x; x = xn; xn = tmp; }

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
