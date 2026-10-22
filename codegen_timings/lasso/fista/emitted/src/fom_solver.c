#include <string.h>
#include "fom_solver.h"
#include "fom_data.h"
#include "fom_ops.h"
#include "fom_linsolve.h"
#include "fom_profile.h"

void fom_set_default_settings(FomSettings *settings) {
    settings->max_iter = FOM_MAX_ITER;
    settings->tol = FOM_TOL;
    settings->check_termination = FOM_CHECK_TERMINATION;
}

static void fom_reset_state(FomWorkspace *work) {
    memcpy(work->state, FOM_STATE_INIT, sizeof(work->state));
}

void fom_setup(FomWorkspace *work, const FomSettings *settings) {
    work->settings = *settings;
    memset(&work->params, 0, sizeof(work->params));
    memset(&work->solution, 0, sizeof(work->solution));
    fom_reset_state(work);
}

void fom_update_params(FomWorkspace *work, const FomParams *params) {
    work->params = *params;
    fom_reset_state(work);
}

fom_int fom_solve(FomWorkspace *work) {
    fom_float *x = work->state;
    fom_float *xn = work->new_state;
    fom_float *cp = work->acc + FOM_T0_DIM;       /* t0_p_p0 @ p0, constant (50) */
    const fom_int max_iter = work->settings.max_iter;
    const fom_int use_tol = work->settings.check_termination && work->settings.tol > 0;
    const fom_float tol = work->settings.tol;
    fom_float res = 0.0;
    fom_int status = FOM_MAX_ITER_REACHED;
    fom_int it, i;

    /* Parameter contribution is iteration-invariant: hoist it. */
    memset(cp, 0, FOM_T0_DIM * sizeof(fom_float));
    fom_apply_t0_p_p0(work->params.p0, cp);

    for (it = 0; it < max_iter; ++it) {
        const fom_float gamma = fom_sched_gamma(it);
        fom_float *t0 = xn + FOM_T0_DIM;          /* su: new_state[50..99] = t0 */

        /* t0 = soft_threshold(P p0 + A x + gamma * (D x)), fused */
        fom_step_t0(x, gamma, cp, &FOM_ACT_t0, t0);

        /* su: new_state[0..49] = state[50..99] (selection copy) */
        memcpy(xn, x + FOM_T0_DIM, FOM_T0_DIM * sizeof(fom_float));

        if (use_tol || it == max_iter - 1) {
            res = 0.0;
            for (i = 0; i < FOM_STATE_DIM; ++i) {
                fom_float d = xn[i] - x[i];
                if (d < 0) d = -d;
                if (d > res) res = d;
            }
        }

        { fom_float *tmp = x; x = xn; xn = tmp; }

        if (use_tol && res <= tol) {
            ++it;
            status = FOM_SOLVED;
            break;
        }
    }

    if (x != work->state) memcpy(work->state, x, sizeof(work->state));
    memcpy(work->t0, work->state + FOM_T0_DIM, sizeof(work->t0));
    memcpy(work->solution.x, work->state, sizeof(work->solution.x));
    work->solution.iter = it;
    work->solution.residual = res;
    work->solution.status = status;
    return status;
}

void fom_cleanup(FomWorkspace *work) {
    (void)work;
}
