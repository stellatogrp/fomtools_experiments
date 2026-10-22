#include "fom_solver.h"
#include "fom_data.h"
#include "fom_ops.h"
#include "fom_linsolve.h"
#include "fom_profile.h"

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
    fom_float *t0 = work->t0, *t1 = work->t1;
    fom_float p1a[FOM_T0_DIM], p1b[FOM_T0_DIM], p0a[FOM_T1_DIM], p0b[FOM_T1_DIM];
    fom_float bias_t0[FOM_T0_DIM], bias_t1[FOM_T1_DIM];
    fom_int it, i, n = work->settings.max_iter;
    fom_float res = 0;

    /* Parameter products are iteration-invariant: apply each once per solve. */
    FOM_PROF_BEGIN(params);
    for (i = 0; i < FOM_T0_DIM; i++) { p1a[i] = 0; p1b[i] = 0; }
    for (i = 0; i < FOM_T1_DIM; i++) { p0a[i] = 0; p0b[i] = 0; }
    fom_apply_t0_p_p1(work->params.p1, p1a);
    fom_apply_t0_p_p1__d_tau(work->params.p1, p1b);
    fom_apply_t1_p_p0(work->params.p0, p0a);
    fom_apply_t1_p_p0__d_eta(work->params.p0, p0b);
    FOM_PROF_END(params);

    for (it = 0; it < n; it++) {
        const fom_float tau = fom_sched_tau(it), eta = fom_sched_eta(it);
        const fom_float d1 = fom_sched_d1(it), g2 = fom_sched_g2(it), m1 = fom_sched_m1(it);
        const fom_float d2 = fom_sched_d2(it), g1 = fom_sched_g1(it), m2 = fom_sched_m2(it);
        FOM_PROF_BEGIN(t0);
        for (i = 0; i < FOM_T0_DIM; i++) bias_t0[i] = p1a[i] + tau * p1b[i];
        for (i = 0; i < FOM_T1_DIM; i++) bias_t1[i] = p0a[i] + eta * p0b[i];
        fom_step_t0(x, fom_sched_taux(it), fom_sched_tth(it), bias_t0, &FOM_ACT_t0, t0);
        FOM_PROF_END(t0);
        FOM_PROF_BEGIN(t1);
        fom_step_t1(x, t0, eta, bias_t1, &FOM_ACT_t1, t1);
        FOM_PROF_END(t1);
        /* State update: every su product is a 0/1 selection (see SPEC.md). */
        FOM_PROF_BEGIN(su);
        for (i = 0; i < 16; i++) xn[i] = g2 * x[i + 64] + g1 * t1[i];
        for (i = 16; i < 64; i++) xn[i] = g2 * x[i + 64] + g1 * t0[i - 16];
        for (i = 64; i < 80; i++) xn[i] = d1 * x[i] + d2 * t1[i - 64];
        for (i = 80; i < 128; i++) xn[i] = d1 * x[i] + d2 * t0[i - 80];
        for (i = 128; i < 144; i++) xn[i] = x[i - 128];
        for (i = 144; i < 160; i++) xn[i] = m1 * x[i - 80] + m2 * t1[i - 144];
        FOM_PROF_END(su);
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
