#include <string.h>
#include "fom_solver.h"
#include "fom_data.h"
#include "fom_ops.h"

/* su has no constant or parameter terms: its bias is zero. */
static const fom_float FOM_ZERO_SU[FOM_STATE_DIM];

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
    work->solution.residual = 0;
    work->solution.status = FOM_MAX_ITER_REACHED;
}

void fom_update_params(FomWorkspace *work, const FomParams *params) {
    work->params = *params;
    memcpy(work->state, FOM_STATE_INIT, sizeof(work->state));
}

fom_int fom_solve(FomWorkspace *work) {
    /* acc layout: [0,48) t0 constant param term, [48,96) t0 tau-scaled param
       term, [96,112) t1 constant, [112,128) t1 eta-scaled, [128,144) bias_t1.
       bias_t0 lives in a fixed-size local array. */
    fom_float *c0 = work->acc;
    fom_float *d0 = work->acc + 48;
    fom_float *c1 = work->acc + 96;
    fom_float *d1v = work->acc + 112;
    fom_float *b1 = work->acc + 128;
    fom_float b0[FOM_T0_DIM];
    fom_float *x = work->state, *xn = work->new_state, *tp;
    fom_float res = 0;
    fom_int it, i, n = work->settings.max_iter;

    memset(work->acc, 0, 128 * sizeof(fom_float));
    fom_apply_t0_p_p1(work->params.p1, c0);
    fom_apply_t0_p_p1__d_tau(work->params.p1, d0);
    fom_apply_t1_p_p0(work->params.p0, c1);
    fom_apply_t1_p_p0__d_eta(work->params.p0, d1v);

    for (it = 0; it < n; ++it) {
        const fom_float tau = fom_sched_tau(it), eta = fom_sched_eta(it);
        for (i = 0; i < FOM_T0_DIM; ++i) b0[i] = c0[i] + tau * d0[i];
        for (i = 0; i < FOM_T1_DIM; ++i) b1[i] = c1[i] + eta * d1v[i];
        fom_iterate(x, fom_sched_taux(it), fom_sched_tth(it), eta,
                    fom_sched_d1(it), fom_sched_g2(it), fom_sched_m1(it),
                    fom_sched_d2(it), fom_sched_g1(it), fom_sched_m2(it),
                    b0, b1, FOM_ZERO_SU, &FOM_ACT_t0, &FOM_ACT_t1,
                    work->t0, work->t1, xn);
        if (it == n - 1 || (FOM_USE_TOL && work->settings.check_termination)) {
            res = 0;
            for (i = 0; i < FOM_STATE_DIM; ++i) {
                fom_float d = xn[i] - x[i];
                if (d < 0) d = -d;
                if (d > res) res = d;
            }
        }
        tp = x; x = xn; xn = tp;
        if (FOM_USE_TOL && work->settings.check_termination && res <= work->settings.tol) {
            ++it;
            break;
        }
    }
    if (x != work->state) memcpy(work->state, x, sizeof(work->state));
    memcpy(work->solution.x, work->state, sizeof(work->state));
    work->solution.iter = it;
    work->solution.residual = res;
    work->solution.status = (FOM_USE_TOL && res <= work->settings.tol) ? FOM_SOLVED : FOM_MAX_ITER_REACHED;
    return work->solution.status;
}

void fom_cleanup(FomWorkspace *work) {
    (void)work;
}
