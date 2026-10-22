#include "fom_solver.h"
#include "fom_data.h"
#include "fom_ops.h"

/* The state update copies t0 into rows 0..48 and t1 into rows 49..58
   (FOM_SELROWS_su), so both steps write straight into the next state and
   fom_step_su__rest fills rows 59..73. work->acc holds su_p_p0 @ p0. */

static void fom_reset(FomWorkspace *work) {
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
    for (i = 0; i < FOM_STATE_DIM; ++i) work->acc[i] = 0.0;
    work->solution.iter = 0;
    work->solution.residual = 0.0;
    work->solution.status = FOM_MAX_ITER_REACHED;
    fom_reset(work);
}

void fom_update_params(FomWorkspace *work, const FomParams *params) {
    fom_int i;
    work->params = *params;
    for (i = 0; i < FOM_STATE_DIM; ++i) work->acc[i] = 0.0;
    fom_apply_su_p_p0(work->params.p0, work->acc);
    fom_reset(work);
}

fom_int fom_solve(FomWorkspace *work) {
    fom_float *cur = work->state;
    fom_float *nxt = work->new_state;
    const fom_float *bias = work->acc;
    const fom_int max_iter = work->settings.max_iter;
    fom_float res = 0.0;
    fom_int it, i;

    for (it = 0; it < max_iter; ++it) {
        const fom_float xi = fom_sched_xi(it);
        fom_float *tmp;
        fom_step_t0(cur, FOM_MAT_t0_c, &FOM_ACT_t0, nxt);
        fom_step_t1(cur, nxt, xi, FOM_MAT_t1_c, &FOM_ACT_t1, nxt + FOM_T0_DIM);
        fom_step_su__rest(cur, nxt, nxt + FOM_T0_DIM, xi, bias, nxt);
        tmp = cur; cur = nxt; nxt = tmp;
    }
    if (max_iter > 0) {
        for (i = 0; i < FOM_STATE_DIM; ++i) {
            fom_float d = cur[i] - nxt[i];
            if (d < 0) d = -d;
            if (d > res) res = d;
        }
    }
    for (i = 0; i < FOM_STATE_DIM; ++i) {
        work->solution.x[i] = cur[i];
        work->state[i] = cur[i];
    }
    work->solution.iter = max_iter;
    work->solution.residual = res;
    work->solution.status = FOM_MAX_ITER_REACHED;
    return work->solution.status;
}

void fom_cleanup(FomWorkspace *work) {
    (void)work;
}
