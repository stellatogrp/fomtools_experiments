#include "fom_solver.h"
#include "fom_data.h"
#include "fom_ops.h"
#include "fom_linsolve.h"
#include "fom_profile.h"

static void fom_reset_state(FomWorkspace *work) {
    fom_int i;
    for (i = 0; i < FOM_STATE_DIM; ++i) work->state[i] = FOM_STATE_INIT[i];
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
    work->solution.iter = 0;
    work->solution.residual = 0.0;
    work->solution.status = FOM_MAX_ITER_REACHED;
}

void fom_update_params(FomWorkspace *work, const FomParams *params) {
    work->params = *params;
    fom_reset_state(work);
}

fom_int fom_solve(FomWorkspace *work) {
    fom_float *x = work->state;
    fom_float *xn = work->new_state;
    fom_float *t0 = work->t0;
    fom_float *t1 = work->t1;
    fom_float *pc = work->acc; /* su_p_p0 @ p0: constant across iterations */
    const fom_int max_iter = work->settings.max_iter;
    const fom_float tol = work->settings.tol;
    const fom_int use_tol = work->settings.check_termination && tol > 0;
    fom_float res = 0.0;
    fom_int status = FOM_MAX_ITER_REACHED;
    fom_int k, i;

    for (i = 0; i < FOM_STATE_DIM; ++i) pc[i] = 0.0;
    fom_apply_su_p_p0(work->params.p0, pc);

    for (k = 0; k < max_iter; ++k) {
        fom_float *sw;

        /* t0 = clip(t0_c + t0_state @ x, lo, hi) */
        for (i = 0; i < FOM_T0_DIM; ++i) t0[i] = FOM_MAT_t0_c[i];
        fom_apply_t0_state(x, t0);
        for (i = 0; i < FOM_T0_DIM; ++i) {
            fom_float v = t0[i];
            v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v;
            v = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v;
            t0[i] = v;
        }

        /* t1 = relu(t1_c + t1_state @ x + t1_t_t0 @ t0) */
        for (i = 0; i < FOM_T1_DIM; ++i) t1[i] = FOM_MAT_t1_c[i];
        fom_apply_t1_state(x, t1);
        fom_apply_t1_t_t0(t0, t1);
        for (i = 0; i < FOM_T1_DIM; ++i) t1[i] = t1[i] > 0 ? t1[i] : 0.0;

        /* xn = su_state @ x + su_t_t0 @ t0 + su_t_t1 @ t1 + su_p_p0 @ p0 */
        for (i = 0; i < FOM_STATE_DIM; ++i) xn[i] = pc[i];
        fom_apply_su_state(x, xn);
        fom_apply_su_t_t0(t0, xn);
        fom_apply_su_t_t1(t1, xn);

        if (use_tol || k == max_iter - 1) {
            res = 0.0;
            for (i = 0; i < FOM_STATE_DIM; ++i) {
                fom_float d = xn[i] - x[i];
                d = d < 0 ? -d : d;
                res = d > res ? d : res;
            }
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
