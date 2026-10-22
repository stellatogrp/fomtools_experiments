#include <string.h>
#include "fom_solver.h"
#include "fom_data.h"
#include "fom_ops.h"

void fom_set_default_settings(FomSettings *settings) {
    settings->max_iter = FOM_MAX_ITER;
    settings->tol = FOM_TOL;
    settings->check_termination = FOM_CHECK_TERMINATION;
}

void fom_setup(FomWorkspace *work, const FomSettings *settings) {
    work->settings = *settings;
    memset(&work->params, 0, sizeof(work->params));
    memcpy(work->state, FOM_STATE_INIT, sizeof(work->state));
    work->solution.iter = 0;
    work->solution.residual = 0.0;
    work->solution.status = FOM_MAX_ITER_REACHED;
}

void fom_update_params(FomWorkspace *work, const FomParams *params) {
    work->params = *params;
    memcpy(work->state, FOM_STATE_INIT, sizeof(work->state));
}

/* ISTA: x <- soft_threshold(A x + B p0). The state update is the identity
   copy of t0, so iterate by ping-ponging between state and new_state. */
fom_int fom_solve(FomWorkspace *work) {
    fom_float *x = work->state, *y = work->new_state, *tswap;
    fom_float *bias = work->acc;
    fom_int max_iter = work->settings.max_iter, k, i;
    fom_float res = 0.0;

    memset(bias, 0, FOM_T0_DIM * sizeof(fom_float));
    fom_apply_t0_p_p0(work->params.p0, bias);

    for (k = 0; k < max_iter; ++k) {
        fom_step_t0(x, bias, &FOM_ACT_t0, y);
        tswap = x; x = y; y = tswap;
    }
    if (max_iter > 0) {
        for (i = 0; i < FOM_STATE_DIM; ++i) {
            fom_float d = x[i] - y[i];
            d = d < 0 ? -d : d;
            if (d > res) res = d;
        }
    }
    memcpy(work->solution.x, x, sizeof(work->solution.x));
    memcpy(work->t0, x, sizeof(work->t0));
    if (x != work->state) memcpy(work->state, x, sizeof(work->state));
    work->solution.iter = max_iter;
    work->solution.residual = res;
    work->solution.status = FOM_MAX_ITER_REACHED;
    return FOM_MAX_ITER_REACHED;
}

void fom_cleanup(FomWorkspace *work) { (void)work; }
