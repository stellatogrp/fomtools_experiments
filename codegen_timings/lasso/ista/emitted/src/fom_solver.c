#include "fom_solver.h"
#include "fom_data.h"
#include "fom_ops.h"
#include "fom_linsolve.h"

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
    const fom_int max_iter = work->settings.max_iter;
    const fom_float tol = work->settings.tol;
    const fom_int check = work->settings.check_termination;
    const fom_int use_tol = (tol > 0.0) && (check > 0);
    fom_float *state = work->state;
    fom_float *ns = work->new_state;
    fom_float *acc = work->acc;
    fom_float res = 0.0;
    fom_int status = FOM_MAX_ITER_REACHED;
    fom_int k, i, ctr = 0;

    /* Parameter contribution is loop-invariant: acc = t0_p_p0 @ p0. */
    for (i = 0; i < FOM_T0_DIM; ++i) acc[i] = 0.0;
    fom_apply_t0_p_p0(work->params.p0, acc);

    for (k = 0; k < max_iter; ++k) {
        /* su_t_t0 is the identity: new_state = soft_threshold(t0_state @ state + acc). */
        fom_step_t0(state, acc, &FOM_ACT_t0, ns);

        if (use_tol || k == max_iter - 1) {
            res = 0.0;
            for (i = 0; i < FOM_STATE_DIM; ++i) {
                fom_float d = ns[i] - state[i];
                if (d < 0) d = -d;
                if (d > res) res = d;
            }
        }
        {
            fom_float *tmp = state; state = ns; ns = tmp;
        }
        if (use_tol && ++ctr >= check) {
            ctr = 0;
            if (res > tol) continue;
            ++k;
            status = FOM_SOLVED;
            break;
        }
    }

    if (state != work->state)
        for (i = 0; i < FOM_STATE_DIM; ++i) work->state[i] = state[i];
    for (i = 0; i < FOM_STATE_DIM; ++i) work->solution.x[i] = work->state[i];
    work->solution.iter = k;
    work->solution.residual = res;
    work->solution.status = status;
    return status;
}

void fom_cleanup(FomWorkspace *work) {
    (void)work;
}
