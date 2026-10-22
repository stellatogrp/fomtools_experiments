#include "fom_solver.h"
#include "fom_data.h"
#include "fom_ops.h"
#include "fom_linsolve.h"
#include "fom_profile.h"

#define N FOM_STATE_DIM
#define NT0 FOM_T0_DIM
#define NT1 FOM_T1_DIM

/* acc layout (parameter terms, hoisted out of the loop):
   [0:48)   t0_p_p1 @ p1         [48:96)   t0_p_p1__d_tau @ p1
   [96:112) t1_p_p0 @ p0         [112:128) t1_p_p0__d_eta @ p0
   su has no constant or parameter terms, so its bias is a zero vector. */
#define A_T0 0
#define B_T0 NT0
#define A_T1 (2 * NT0)
#define B_T1 (2 * NT0 + NT1)

static void reset_state(FomWorkspace *work) {
    fom_int i;
    for (i = 0; i < N; ++i) work->state[i] = FOM_STATE_INIT[i];
}

static void hoist_params(FomWorkspace *work) {
    fom_int i;
    fom_float *acc = work->acc;
    for (i = 0; i < FOM_MAX_VEC_DIM; ++i) acc[i] = 0.0;
    fom_apply_t0_p_p1(work->params.p1, acc + A_T0);
    fom_apply_t0_p_p1__d_tau(work->params.p1, acc + B_T0);
    fom_apply_t1_p_p0(work->params.p0, acc + A_T1);
    fom_apply_t1_p_p0__d_eta(work->params.p0, acc + B_T1);
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
    for (i = 0; i < FOM_PARAM_P1_DIM; ++i) work->params.p1[i] = 0.0;
    reset_state(work);
    hoist_params(work);
    work->solution.iter = 0;
    work->solution.residual = 0.0;
    work->solution.status = FOM_MAX_ITER_REACHED;
}

void fom_update_params(FomWorkspace *work, const FomParams *params) {
    work->params = *params;
    reset_state(work);
    hoist_params(work);
}

fom_int fom_solve(FomWorkspace *work) {
    static const fom_float zero_su[N];
    fom_float *s = work->state;
    fom_float *ns = work->new_state;
    const fom_float *acc = work->acc;
    fom_float *t0 = work->t0;
    fom_float *t1 = work->t1;
    fom_float bias_t0[NT0];
    fom_float bias_t1[NT1];
    const fom_int max_iter = work->settings.max_iter;
    fom_float res = 0.0;
    fom_int it, i;
    fom_int status = FOM_MAX_ITER_REACHED;

    for (it = 0; it < max_iter; ++it) {
        const fom_float d1 = fom_sched_d1(it);
        const fom_float d2 = fom_sched_d2(it);
        const fom_float eta = fom_sched_eta(it);
        const fom_float g1 = fom_sched_g1(it);
        const fom_float g2 = fom_sched_g2(it);
        const fom_float m1 = fom_sched_m1(it);
        const fom_float m2 = fom_sched_m2(it);
        const fom_float tau = fom_sched_tau(it);
        const fom_float taux = fom_sched_taux(it);
        const fom_float tth = fom_sched_tth(it);
        fom_float *tmp;

        for (i = 0; i < NT0; ++i) bias_t0[i] = acc[A_T0 + i] + tau * acc[B_T0 + i];
        for (i = 0; i < NT1; ++i) bias_t1[i] = acc[A_T1 + i] + eta * acc[B_T1 + i];

        FOM_PROF_BEGIN(t0);
        fom_step_t0(s, taux, tth, bias_t0, &FOM_ACT_t0, t0);
        FOM_PROF_END(t0);
        FOM_PROF_BEGIN(t1);
        fom_step_t1(s, t0, eta, bias_t1, &FOM_ACT_t1, t1);
        FOM_PROF_END(t1);
        FOM_PROF_BEGIN(su);
        fom_step_su__rest(s, t0, t1, d1, g2, m1, d2, g1, m2, zero_su, ns);
        /* selection rows 128..143 copy state[0..15] */
        for (i = 0; i < 16; ++i) ns[128 + i] = s[i];
        FOM_PROF_END(su);

#if !FOM_USE_TOL
        /* only the final residual is reported when tolerance checks are compiled out */
        if (it + 1 == max_iter)
#endif
        {
            FOM_PROF_BEGIN(residual);
            res = 0.0;
            for (i = 0; i < N; ++i) {
                fom_float d = ns[i] - s[i];
                d = d < 0.0 ? -d : d;
                res = d > res ? d : res;
            }
            FOM_PROF_END(residual);
        }
        tmp = s; s = ns; ns = tmp;
#if FOM_USE_TOL
        if (work->settings.check_termination && res <= work->settings.tol) {
            ++it;
            status = FOM_SOLVED;
            break;
        }
#endif
    }

    if (s != work->state)
        for (i = 0; i < N; ++i) work->state[i] = s[i];
    for (i = 0; i < N; ++i) work->solution.x[i] = s[i];
    work->solution.iter = it;
    work->solution.residual = res;
    work->solution.status = status;
    return status;
}

void fom_cleanup(FomWorkspace *work) {
    (void)work;
}
