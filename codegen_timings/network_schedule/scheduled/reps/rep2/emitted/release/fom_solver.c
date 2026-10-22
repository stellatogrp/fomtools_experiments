#include "fom_solver.h"
#include "fom_data.h"
#include "fom_ops.h"
#include "fom_linsolve.h"

void fom_set_default_settings(FomSettings *settings) {
    settings->max_iter = FOM_MAX_ITER;
    settings->tol = FOM_TOL;
    settings->check_termination = FOM_CHECK_TERMINATION;
}

static void reset_state(FomWorkspace *work) {
    fom_int i;
    for (i = 0; i < FOM_STATE_DIM; ++i) work->state[i] = FOM_STATE_INIT[i];
}

void fom_setup(FomWorkspace *work, const FomSettings *settings) {
    fom_int i;
    work->settings = *settings;
    for (i = 0; i < FOM_PARAM_P0_DIM; ++i) work->params.p0[i] = 0.0;
    for (i = 0; i < FOM_STATE_DIM; ++i) work->acc[i] = 0.0;
    reset_state(work);
    work->solution.iter = 0;
    work->solution.residual = 0.0;
    work->solution.status = FOM_MAX_ITER_REACHED;
}

void fom_update_params(FomWorkspace *work, const FomParams *params) {
    fom_int i;
    work->params = *params;
    /* Hoisted state-update bias: su_p_p0 @ p0. */
    for (i = 0; i < FOM_STATE_DIM; ++i) work->acc[i] = 0.0;
    fom_apply_su_p_p0(work->params.p0, work->acc);
    reset_state(work);
}

fom_int fom_solve(FomWorkspace *work) {
    fom_float *x = work->state;
    fom_float *xn = work->new_state;
    const fom_float *bias_su = work->acc;
    fom_int max_iter = work->settings.max_iter;
    fom_int it, i;
    fom_float res = 0.0;

    for (it = 0; it < max_iter; ++it) {
        const fom_float xi = fom_sched_xi(it);
        fom_float *t;
        /* Selection rows: new_state[0..599] = t0, new_state[600..619] = t1. */
        fom_step_t0(x, FOM_MAT_t0_c, &FOM_ACT_t0, xn);
        fom_step_t1(x, xn, xi, FOM_MAT_t1_c, &FOM_ACT_t1, xn + FOM_T0_DIM);
        fom_step_su__rest(x, xn, xn + FOM_T0_DIM, xi, bias_su, xn);
        res = 0.0;
        for (i = 0; i < FOM_STATE_DIM; ++i) {
            fom_float d = xn[i] - x[i];
            d = d < 0.0 ? -d : d;
            res = d > res ? d : res;
        }
        t = x; x = xn; xn = t;
    }

    for (i = 0; i < FOM_STATE_DIM; ++i) work->solution.x[i] = x[i];
    if (x != work->state)
        for (i = 0; i < FOM_STATE_DIM; ++i) work->state[i] = x[i];
    work->solution.iter = max_iter;
    work->solution.residual = res;
    work->solution.status = FOM_MAX_ITER_REACHED;
    return FOM_MAX_ITER_REACHED;
}

void fom_cleanup(FomWorkspace *work) {
    (void)work;
}
