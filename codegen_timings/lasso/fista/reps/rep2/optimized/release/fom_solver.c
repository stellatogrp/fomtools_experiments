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
    memcpy(work->state, FOM_STATE_INIT, sizeof(work->state));
    memset(&work->params, 0, sizeof(work->params));
    work->solution.iter = 0;
    work->solution.residual = 0.0;
    work->solution.status = FOM_MAX_ITER_REACHED;
}

void fom_update_params(FomWorkspace *work, const FomParams *params) {
    work->params = *params;
    memcpy(work->state, FOM_STATE_INIT, sizeof(work->state));
}

fom_int fom_solve(FomWorkspace *work) {
    fom_float *x = work->state;
    fom_float *t = work->t0;
    fom_float *bias = work->acc;
    const fom_int max_iter = work->settings.max_iter;
    fom_float res = 0.0;
    fom_int it, i;

    /* Parameter term is iteration-invariant: hoist it. */
    memset(bias, 0, FOM_T0_DIM * sizeof(fom_float));
    fom_apply_t0_p_p0(work->params.p0, bias);

    for (it = 0; it < max_iter; ++it) {
        const fom_float g = fom_sched_gamma(it);
        fom_step_t0(x, g, bias, &FOM_ACT_t0, t);
        /* State update is a pure shift: new = [x[50..99], t]. */
        res = 0.0;
        for (i = 0; i < FOM_T0_DIM; ++i) {
            const fom_float a = x[i + FOM_T0_DIM];
            const fom_float b = t[i];
            fom_float d0 = a - x[i];
            fom_float d1 = b - a;
            d0 = d0 < 0 ? -d0 : d0;
            d1 = d1 < 0 ? -d1 : d1;
            if (d0 > res) res = d0;
            if (d1 > res) res = d1;
            x[i] = a;
            x[i + FOM_T0_DIM] = b;
        }
    }

    memcpy(work->solution.x, x, sizeof(work->solution.x));
    memcpy(work->new_state, x, sizeof(work->new_state));
    work->solution.iter = it;
    work->solution.residual = res;
    work->solution.status = FOM_MAX_ITER_REACHED;
    return FOM_MAX_ITER_REACHED;
}

void fom_cleanup(FomWorkspace *work) {
    (void)work;
}
