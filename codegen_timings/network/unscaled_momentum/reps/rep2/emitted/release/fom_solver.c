#include "fom_solver.h"
#include "fom_data.h"
#include "fom_ops.h"

static void reset_state(FomWorkspace *work) {
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
    for (i = 0; i < FOM_PARAM_P0_DIM; ++i) work->params.p0[i] = 0;
    reset_state(work);
    work->solution.iter = 0;
    work->solution.residual = 0;
    work->solution.status = FOM_MAX_ITER_REACHED;
}

void fom_update_params(FomWorkspace *work, const FomParams *params) {
    work->params = *params;
    reset_state(work);
}

fom_int fom_solve(FomWorkspace *work) {
    fom_float *x = work->state;
    fom_float *y = work->new_state;
    fom_float *bias_su = work->acc;
    const fom_int max_iter = work->settings.max_iter;
    fom_float res = 0;
    fom_int it, i;

    /* Parameter term of the state update is iteration-invariant: hoist it. */
    for (i = 0; i < FOM_STATE_DIM; ++i) bias_su[i] = 0;
    fom_apply_su_p_p0(work->params.p0, bias_su);

    for (it = 0; it < max_iter; ++it) {
        const fom_float xi = fom_sched_xi(it);
        fom_float *tmp;
        /* Selection rows of su: rows 0..48 are t0 and rows 49..58 are t1,
           so the temporaries live directly in the next state. */
        fom_float *t0 = y, *t1 = y + FOM_T0_DIM;
        fom_step_t0(x, FOM_MAT_t0_c, &FOM_ACT_t0, t0);
        fom_step_t1(x, t0, xi, FOM_MAT_t1_c, &FOM_ACT_t1, t1);
        fom_step_su__rest(x, t0, t1, xi, bias_su, y);
        tmp = x; x = y; y = tmp;
    }

    /* tol = 0: no early exit; residual reported from the last step. */
    if (max_iter > 0) {
        for (i = 0; i < FOM_STATE_DIM; ++i) {
            fom_float d = x[i] - y[i];
            if (d < 0) d = -d;
            if (d > res) res = d;
        }
    }
    for (i = 0; i < FOM_STATE_DIM; ++i) {
        work->solution.x[i] = x[i];
        work->state[i] = x[i];
    }
    work->solution.iter = max_iter;
    work->solution.residual = res;
    work->solution.status = FOM_MAX_ITER_REACHED;
    return work->solution.status;
}

void fom_cleanup(FomWorkspace *work) {
    (void)work;
}
