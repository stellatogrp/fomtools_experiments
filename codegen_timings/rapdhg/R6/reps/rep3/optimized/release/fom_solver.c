#include <string.h>
#include "fom_solver.h"
#include "fom_data.h"
#include "fom_ops.h"

static const fom_float FOM_ZERO_SU[FOM_STATE_DIM] = {0};

void fom_set_default_settings(FomSettings *settings) {
    settings->max_iter = FOM_MAX_ITER;
    settings->tol = FOM_TOL;
    settings->check_termination = FOM_CHECK_TERMINATION;
}

static void reset_state(FomWorkspace *work) {
    memcpy(work->state, FOM_STATE_INIT, sizeof(work->state));
}

void fom_setup(FomWorkspace *work, const FomSettings *settings) {
    work->settings = *settings;
    memset(&work->params, 0, sizeof(work->params));
    reset_state(work);
}

void fom_update_params(FomWorkspace *work, const FomParams *params) {
    work->params = *params;
    reset_state(work);
}

fom_int fom_solve(FomWorkspace *work) {
    fom_float *x = work->state;
    fom_float *xn = work->new_state;
    /* Parameter terms, hoisted: acc[0..47] = t0_p_p1 @ p1, acc[48..95] = d_tau @ p1,
       acc[96..111] = t1_p_p0 @ p0, acc[112..127] = d_eta @ p0. */
    fom_float *a0 = work->acc, *b0 = work->acc + 48;
    fom_float *a1 = work->acc + 96, *b1 = work->acc + 112;
    fom_float bias_t0[FOM_T0_DIM], bias_t1[FOM_T1_DIM];
    fom_int max_iter = work->settings.max_iter;
    fom_int use_tol = FOM_USE_TOL && work->settings.check_termination;
    fom_float tol = work->settings.tol;
    fom_float res = 0;
    fom_int it, i, status = FOM_MAX_ITER_REACHED;

    memset(work->acc, 0, 128 * sizeof(fom_float));
    fom_apply_t0_p_p1(work->params.p1, a0);
    fom_apply_t0_p_p1__d_tau(work->params.p1, b0);
    fom_apply_t1_p_p0(work->params.p0, a1);
    fom_apply_t1_p_p0__d_eta(work->params.p0, b1);

    for (it = 0; it < max_iter; ++it) {
        const fom_float tau = fom_sched_tau(it), eta = fom_sched_eta(it);
        fom_float *tmp;
        for (i = 0; i < FOM_T0_DIM; ++i) bias_t0[i] = a0[i] + tau * b0[i];
        for (i = 0; i < FOM_T1_DIM; ++i) bias_t1[i] = a1[i] + eta * b1[i];
        fom_iterate(x, fom_sched_taux(it), fom_sched_tth(it), eta,
                    fom_sched_d1(it), fom_sched_g2(it), fom_sched_m1(it),
                    fom_sched_d2(it), fom_sched_g1(it), fom_sched_m2(it),
                    bias_t0, bias_t1, FOM_ZERO_SU, &FOM_ACT_t0, &FOM_ACT_t1,
                    work->t0, work->t1, xn);
        res = 0;
        for (i = 0; i < FOM_STATE_DIM; ++i) {
            fom_float d = xn[i] - x[i];
            if (d < 0) d = -d;
            if (d > res) res = d;
        }
        tmp = x; x = xn; xn = tmp;
        if (use_tol && res <= tol) { status = FOM_SOLVED; ++it; break; }
    }
    if (x != work->state) memcpy(work->state, x, sizeof(work->state));
    memcpy(work->solution.x, work->state, sizeof(work->state));
    work->solution.iter = it;
    work->solution.residual = res;
    work->solution.status = status;
    return status;
}

void fom_cleanup(FomWorkspace *work) { (void)work; }
