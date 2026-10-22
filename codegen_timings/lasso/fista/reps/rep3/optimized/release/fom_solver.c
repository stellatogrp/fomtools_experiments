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
}

void fom_update_params(FomWorkspace *work, const FomParams *params) {
    work->params = *params;
    memcpy(work->state, FOM_STATE_INIT, sizeof(work->state));
}

fom_int fom_solve(FomWorkspace *work) {
    fom_float *state = work->state;
    fom_float *t0 = work->t0;
    fom_float *bias = work->acc;
    const fom_int max_iter = work->settings.max_iter;
    fom_float res = 0.0;
    fom_int it, i;

    /* parameter term is constant across iterations: hoist it */
    memset(bias, 0, FOM_T0_DIM * sizeof(fom_float));
    fom_apply_t0_p_p0(work->params.p0, bias);

    for (it = 0; it < max_iter; ++it) {
        const fom_float g = fom_sched_gamma(it);
        fom_step_t0(state, g, bias, &FOM_ACT_t0, t0);
        if (it == max_iter - 1) {
            res = 0.0;
            for (i = 0; i < FOM_T0_DIM; ++i) {
                fom_float d = state[FOM_T0_DIM + i] - state[i];
                fom_float e = t0[i] - state[FOM_T0_DIM + i];
                d = d < 0 ? -d : d;
                e = e < 0 ? -e : e;
                if (d > res) res = d;
                if (e > res) res = e;
            }
        }
        /* su: rows 0..49 copy state[50..99]; rows 50..99 copy t0 */
        memcpy(state, state + FOM_T0_DIM, FOM_T0_DIM * sizeof(fom_float));
        memcpy(state + FOM_T0_DIM, t0, FOM_T0_DIM * sizeof(fom_float));
    }

    memcpy(work->new_state, state, sizeof(work->state));
    memcpy(work->solution.x, state, sizeof(work->solution.x));
    work->solution.iter = max_iter;
    work->solution.residual = res;
    work->solution.status = FOM_MAX_ITER_REACHED;
    return FOM_MAX_ITER_REACHED;
}

void fom_cleanup(FomWorkspace *work) {
    (void)work;
}
