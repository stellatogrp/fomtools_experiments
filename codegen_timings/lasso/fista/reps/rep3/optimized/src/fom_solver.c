#include <string.h>
#include "fom_solver.h"
#include "fom_data.h"
#include "fom_ops.h"
#include "fom_profile.h"

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
    fom_float *cur = work->state;
    fom_float *nxt = work->new_state;
    fom_float *bias = work->acc;
    const fom_int max_iter = work->settings.max_iter;
    fom_float res = 0.0;
    fom_int it, i;

    /* parameter term is constant across iterations: hoist it */
    memset(bias, 0, FOM_T0_DIM * sizeof(fom_float));
    fom_apply_t0_p_p0(work->params.p0, bias);

    /* ping-pong between state and new_state: su copies x1 into the lower
       half and t0 is written straight into the upper half */
    for (it = 0; it < max_iter; ++it) {
        const fom_float g = fom_sched_gamma(it);
        fom_float *tmp;
        FOM_PROF_BEGIN(step_t0);
        fom_step_t0(cur, g, bias, &FOM_ACT_t0, nxt + FOM_T0_DIM);
        FOM_PROF_END(step_t0);
        FOM_PROF_BEGIN(state_update);
        memcpy(nxt, cur + FOM_T0_DIM, FOM_T0_DIM * sizeof(fom_float));
        FOM_PROF_END(state_update);
        if (it == max_iter - 1) {
            res = 0.0;
            for (i = 0; i < 2 * FOM_T0_DIM; ++i) {
                fom_float d = nxt[i] - cur[i];
                d = d < 0 ? -d : d;
                if (d > res) res = d;
            }
        }
        tmp = cur; cur = nxt; nxt = tmp;
    }

    if (cur != work->state) memcpy(work->state, cur, sizeof(work->state));
    memcpy(work->new_state, work->state, sizeof(work->state));
    memcpy(work->t0, work->state + FOM_T0_DIM, FOM_T0_DIM * sizeof(fom_float));
    memcpy(work->solution.x, work->state, sizeof(work->solution.x));
    work->solution.iter = max_iter;
    work->solution.residual = res;
    work->solution.status = FOM_MAX_ITER_REACHED;
    return FOM_MAX_ITER_REACHED;
}

void fom_cleanup(FomWorkspace *work) {
    (void)work;
}
