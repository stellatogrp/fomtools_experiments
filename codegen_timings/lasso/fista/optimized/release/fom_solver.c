#include <string.h>
#include "fom_solver.h"
#include "fom_data.h"
#include "fom_ops.h"
#include "fom_linsolve.h"

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
    fom_float *t0 = work->t0;
    fom_float *dg = work->acc;                    /* D_gamma @ state (15) */
    fom_float *cp = work->acc + FOM_T0_DIM;       /* t0_p_p0 @ p0, constant (15) */
    const fom_int max_iter = work->settings.max_iter;
    const fom_int use_tol = work->settings.check_termination && work->settings.tol > 0;
    const fom_float tol = work->settings.tol;
    const fom_float lam = FOM_T0_LAMBDA;
    fom_float res = 0.0;
    fom_int status = FOM_MAX_ITER_REACHED;
    fom_int it, i;

    /* Parameter contribution is iteration-invariant: hoist it. */
    memset(cp, 0, FOM_T0_DIM * sizeof(fom_float));
    fom_apply_t0_p_p0(work->params.p0, cp);

    for (it = 0; it < max_iter; ++it) {
        const fom_float gamma = fom_sched_gamma(it);

        /* t0 = soft_threshold(A x + gamma * (D x) + P p0) */
        memcpy(t0, cp, FOM_T0_DIM * sizeof(fom_float));
        fom_apply_t0_state(x, t0);
        if (gamma != 0.0) {
            memset(dg, 0, FOM_T0_DIM * sizeof(fom_float));
            fom_apply_t0_state__d_gamma(x, dg);
            for (i = 0; i < FOM_T0_DIM; ++i) t0[i] += gamma * dg[i];
        }
        for (i = 0; i < FOM_T0_DIM; ++i) {
            const fom_float v = t0[i];
            t0[i] = (v > lam) ? (v - lam) : ((v < -lam) ? (v + lam) : 0.0);
        }

        /* state <- su_state @ state + su_t_t0 @ t0 */
        memset(xn, 0, FOM_STATE_DIM * sizeof(fom_float));
        fom_apply_su_state(x, xn);
        fom_apply_su_t_t0(t0, xn);

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
    memcpy(work->solution.x, work->state, sizeof(work->solution.x));
    work->solution.iter = it;
    work->solution.residual = res;
    work->solution.status = status;
    return status;
}

void fom_cleanup(FomWorkspace *work) {
    (void)work;
}
