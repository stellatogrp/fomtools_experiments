#include "fom_solver.h"
#include "fom_data.h"
#include "fom_ops.h"

static void fom_reset_state(FomWorkspace *work) {
    fom_int i;
    for (i = 0; i < FOM_STATE_DIM; ++i) work->state[i] = FOM_STATE_INIT[i];
}

/* Hoisted parameter bias of the state update: acc = su_p_p0 @ p0. */
static void fom_update_bias(FomWorkspace *work) {
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
    fom_update_bias(work);
    fom_reset_state(work);
    work->solution.iter = 0;
    work->solution.residual = 0.0;
    work->solution.status = FOM_MAX_ITER_REACHED;
}

void fom_update_params(FomWorkspace *work, const FomParams *params) {
    work->params = *params;
    fom_update_bias(work);
    fom_reset_state(work);
}

fom_int fom_solve(FomWorkspace *work) {
    fom_float *x = work->state;
    fom_float *xn = work->new_state;
    fom_float res = 0.0;
    const fom_int max_iter = work->settings.max_iter;
    fom_int it, i;

    for (it = 0; it < max_iter; ++it) {
        const fom_float xi = fom_sched_xi(it);
        fom_float *tmp;
        /* Selection rows of su: new state rows 0..48 are t0 and rows
           49..58 are t1, so compute the temporaries in place there; the
           __rest routine writes only rows 59..73. */
        fom_float *t0 = xn;
        fom_float *t1 = xn + FOM_T0_DIM;
        fom_step_t0(x, FOM_MAT_t0_c, &FOM_ACT_t0, t0);
        fom_step_t1(x, t0, xi, FOM_MAT_t1_c, &FOM_ACT_t1, t1);
        fom_step_su__rest(x, t0, t1, xi, work->acc, xn);
        tmp = x; x = xn; xn = tmp;
    }
    /* Residual of the last iteration (x = newest, xn = previous). */
    if (max_iter > 0) {
        for (i = 0; i < FOM_STATE_DIM; ++i) {
            fom_float d = x[i] - xn[i];
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
    return FOM_MAX_ITER_REACHED;
}

void fom_cleanup(FomWorkspace *work) { (void)work; }
