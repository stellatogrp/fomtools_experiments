#include "fom_solver.h"
#include "fom_data.h"
#include "fom_ops.h"
#include <string.h>

void fom_set_default_settings(FomSettings *settings) {
    settings->max_iter = FOM_MAX_ITER;
    settings->tol = FOM_TOL;
    settings->check_termination = FOM_CHECK_TERMINATION;
}

void fom_setup(FomWorkspace *work, const FomSettings *settings) {
    work->settings = *settings;
    memcpy(work->state, FOM_STATE_INIT, sizeof(work->state));
    work->solution.iter = 0;
    work->solution.residual = 0.0;
    work->solution.status = FOM_MAX_ITER_REACHED;
}

void fom_update_params(FomWorkspace *work, const FomParams *params) {
    work->params = *params;
    memcpy(work->state, FOM_STATE_INIT, sizeof(work->state));
}

fom_int fom_solve(FomWorkspace *work) {
    fom_float *state = work->state;
    fom_float *ns = work->new_state;
    fom_float *bias = work->acc;
    fom_float *tmp;
    fom_int it, i, max_iter = work->settings.max_iter;
    fom_float res = 0.0;

    /* Hoisted parameter term: bias_su = su_p_p0 @ p0. */
    memset(bias, 0, sizeof(work->acc));
    fom_apply_su_p_p0(work->params.p0, bias);

    for (it = 0; it < max_iter; ++it) {
        fom_float xi = fom_sched_xi(it);
        fom_iterate(state, xi, FOM_MAT_t0_c, FOM_MAT_t1_c, bias,
                    &FOM_ACT_t0, &FOM_ACT_t1, work->t0, work->t1, ns);
        if (it == max_iter - 1) {
            res = 0.0;
            for (i = 0; i < FOM_STATE_DIM; ++i) {
                fom_float d = ns[i] - state[i];
                if (d < 0) d = -d;
                if (d > res) res = d;
            }
        }
        tmp = state; state = ns; ns = tmp;
    }
    if (state != work->state) memcpy(work->state, state, sizeof(work->state));

    memcpy(work->solution.x, work->state, sizeof(work->solution.x));
    work->solution.iter = max_iter;
    work->solution.residual = res;
    work->solution.status = FOM_MAX_ITER_REACHED;
    return work->solution.status;
}

void fom_cleanup(FomWorkspace *work) { (void)work; }
