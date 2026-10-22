#include "fom_solver.h"
#include "fom_data.h"
#include "fom_ops.h"
#include "fom_profile.h"

/* acc layout: [0,48) q1 = t0_p_p1__d_tau @ p1, [48,64) q0 = t1_p_p0__d_eta @ p0,
   [64,112) bias_t0, [112,128) bias_t1. The plain t0_p_p1 and t1_p_p0 parameter
   matrices are structurally zero, so the per-iteration bias is tau*q1 and
   eta*q0 (checked at compile time below). */
#define Q1 0
#define Q0 48
#define BT0 64
#define BT1 112
#if FOM_MAT_t0_p_p1_NNZ > 0 || FOM_MAT_t1_p_p0_NNZ > 0
#error "nonzero constant parameter matrices: extend acc layout"
#endif

static const fom_float s_zero_su[FOM_STATE_DIM];

static void reset_state(FomWorkspace *work) {
    fom_int i;
    for (i = 0; i < FOM_STATE_DIM; ++i) work->state[i] = FOM_STATE_INIT[i];
}

static void hoist_params(FomWorkspace *work) {
    fom_int i;
    fom_float *a = work->acc;
    for (i = 0; i < BT0; ++i) a[i] = (fom_float)0;
    fom_apply_t0_p_p1__d_tau(work->params.p1, a + Q1);
    fom_apply_t1_p_p0__d_eta(work->params.p0, a + Q0);
}

void fom_set_default_settings(FomSettings *settings) {
    settings->max_iter = FOM_MAX_ITER;
    settings->tol = FOM_TOL;
    settings->check_termination = FOM_CHECK_TERMINATION;
}

void fom_setup(FomWorkspace *work, const FomSettings *settings) {
    fom_int i;
    work->settings = *settings;
    for (i = 0; i < FOM_PARAM_P0_DIM; ++i) work->params.p0[i] = (fom_float)0;
    for (i = 0; i < FOM_PARAM_P1_DIM; ++i) work->params.p1[i] = (fom_float)0;
    hoist_params(work);
    reset_state(work);
    work->solution.iter = 0;
    work->solution.residual = (fom_float)0;
    work->solution.status = FOM_MAX_ITER_REACHED;
}

void fom_update_params(FomWorkspace *work, const FomParams *params) {
    work->params = *params;
    hoist_params(work);
    reset_state(work);
}

fom_int fom_solve(FomWorkspace *work) {
    fom_float *x = work->state;
    fom_float *xn = work->new_state;
    fom_float *a = work->acc;
    fom_int it, i, n = work->settings.max_iter;
    fom_float res = (fom_float)0;

    for (it = 0; it < n; ++it) {
        fom_float tau = fom_sched_tau(it), eta = fom_sched_eta(it);
        fom_float *tmp;
        FOM_PROF_BEGIN(bias);
        for (i = 0; i < FOM_T0_DIM; ++i) a[BT0 + i] = tau * a[Q1 + i];
        for (i = 0; i < FOM_T1_DIM; ++i) a[BT1 + i] = eta * a[Q0 + i];
        FOM_PROF_END(bias);
        FOM_PROF_BEGIN(iterate);
        fom_iterate(x, fom_sched_taux(it), fom_sched_tth(it), eta,
                    fom_sched_d1(it), fom_sched_g2(it), fom_sched_m1(it),
                    fom_sched_d2(it), fom_sched_g1(it), fom_sched_m2(it),
                    a + BT0, a + BT1, s_zero_su, &FOM_ACT_t0, &FOM_ACT_t1,
                    work->t0, work->t1, xn);
        FOM_PROF_END(iterate);
        tmp = x; x = xn; xn = tmp;
    }
    if (n > 0) {
        for (i = 0; i < FOM_STATE_DIM; ++i) {
            fom_float d = x[i] - xn[i];
            if (d < 0) d = -d;
            if (d > res) res = d;
        }
    }
    for (i = 0; i < FOM_STATE_DIM; ++i) work->solution.x[i] = x[i];
    if (x != work->state)
        for (i = 0; i < FOM_STATE_DIM; ++i) work->state[i] = x[i];
    work->solution.iter = n;
    work->solution.residual = res;
    work->solution.status = FOM_MAX_ITER_REACHED;
    return work->solution.status;
}

void fom_cleanup(FomWorkspace *work) { (void)work; }
