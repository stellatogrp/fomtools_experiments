#include "fom_solver.h"
#include "fom_data.h"
#include "fom_ops.h"
#include "fom_linsolve.h"

static void reset_state(FomWorkspace *work) {
    fom_int i;
    for (i = 0; i < FOM_STATE_DIM; i++) work->state[i] = FOM_STATE_INIT[i];
}

void fom_set_default_settings(FomSettings *settings) {
    settings->max_iter = FOM_MAX_ITER;
    settings->tol = FOM_TOL;
    settings->check_termination = FOM_CHECK_TERMINATION;
}

void fom_setup(FomWorkspace *work, const FomSettings *settings) {
    fom_int i;
    work->settings = *settings;
    for (i = 0; i < FOM_PARAM_P0_DIM; i++) work->params.p0[i] = 0;
    for (i = 0; i < FOM_PARAM_P1_DIM; i++) work->params.p1[i] = 0;
    reset_state(work);
    work->solution.iter = 0;
    work->solution.residual = 0;
    work->solution.status = FOM_MAX_ITER_REACHED;
}

void fom_update_params(FomWorkspace *work, const FomParams *params) {
    work->params = *params;
    reset_state(work);
}

fom_int fom_solve(FomWorkspace *work) {
    fom_float *x = work->state, *xn = work->new_state, *tmp;
    fom_float *bias_t0 = work->acc, *bias_t1 = work->acc + FOM_T0_DIM;
    fom_int it, i, n = work->settings.max_iter;
    fom_float res = 0;

    for (it = 0; it < n; it++) {
        const fom_float tau = fom_sched_tau(it), eta = fom_sched_eta(it);
        /* bias_t0 = t0_p_p1 @ p1 + tau * (t0_p_p1__d_tau @ p1); same for t1 with eta. */
        for (i = 0; i < FOM_T0_DIM + FOM_T1_DIM; i++) work->acc[i] = 0;
        fom_apply_t0_p_p1__d_tau(work->params.p1, bias_t0);
        fom_apply_t1_p_p0__d_eta(work->params.p0, bias_t1);
        for (i = 0; i < FOM_T0_DIM; i++) bias_t0[i] *= tau;
        for (i = 0; i < FOM_T1_DIM; i++) bias_t1[i] *= eta;
        fom_apply_t0_p_p1(work->params.p1, bias_t0);
        fom_apply_t1_p_p0(work->params.p0, bias_t1);
        fom_step_t0(x, fom_sched_taux(it), fom_sched_tth(it), bias_t0, &FOM_ACT_t0, work->t0);
        fom_step_t1(x, work->t0, eta, bias_t1, &FOM_ACT_t1, work->t1);
        for (i = 0; i < FOM_STATE_DIM; i++) xn[i] = 0;
        fom_fused_su(x, work->t0, work->t1, fom_sched_d1(it), fom_sched_g2(it),
                     fom_sched_m1(it), fom_sched_d2(it), fom_sched_g1(it),
                     fom_sched_m2(it), xn);
        tmp = x; x = xn; xn = tmp;
    }
    if (n > 0) {
        /* x = last iterate, xn = previous one. */
        for (i = 0; i < FOM_STATE_DIM; i++) {
            fom_float d = x[i] - xn[i];
            if (d < 0) d = -d;
            if (d > res) res = d;
        }
    }
    for (i = 0; i < FOM_STATE_DIM; i++) work->solution.x[i] = x[i];
    if (x != work->state)
        for (i = 0; i < FOM_STATE_DIM; i++) work->state[i] = x[i];
    work->solution.iter = n;
    work->solution.residual = res;
    work->solution.status = FOM_MAX_ITER_REACHED;
    return FOM_MAX_ITER_REACHED;
}

void fom_cleanup(FomWorkspace *work) { (void)work; }
