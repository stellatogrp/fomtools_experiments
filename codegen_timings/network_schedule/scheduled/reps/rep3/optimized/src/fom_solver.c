#include <string.h>
#include "fom_solver.h"
#include "fom_data.h"
#include "fom_ops.h"
#include "fom_profile.h"

void fom_set_default_settings(FomSettings *settings) {
    settings->max_iter = FOM_MAX_ITER;
    settings->tol = FOM_TOL;
    settings->check_termination = FOM_CHECK_TERMINATION;
}

void fom_setup(FomWorkspace *work, const FomSettings *settings) {
    work->settings = *settings;
    memset(work->params.p0, 0, sizeof(work->params.p0));
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
    fom_float *restrict x = work->state;
    fom_float *restrict xn = work->new_state;
    fom_float *restrict bias = work->acc;
    fom_float *restrict t0 = work->t0;
    fom_float *restrict t1 = work->t1;
    const fom_int max_iter = work->settings.max_iter;
    fom_float res = 0;
    fom_int it;

    /* Hoisted parameter term of the state update. */
    memset(bias, 0, sizeof(fom_float) * FOM_STATE_DIM);
    fom_apply_su_p_p0(work->params.p0, bias);

    for (it = 0; it < max_iter; ++it) {
        const fom_float xi = fom_sched_xi(it);
        fom_int i;
        FOM_PROF_BEGIN(t0);
        fom_step_t0(x, FOM_MAT_t0_c, &FOM_ACT_t0, t0);
        FOM_PROF_END(t0);
        FOM_PROF_BEGIN(t1);
        fom_step_t1(x, t0, xi, FOM_MAT_t1_c, &FOM_ACT_t1, t1);
        FOM_PROF_END(t1);
        FOM_PROF_BEGIN(su);
        fom_step_su__rest(x, t0, t1, xi, bias, xn);
        FOM_PROF_END(su);
        FOM_PROF_BEGIN(update);
        /* Selection rows: 0..599 copy t0, 600..619 copy t1. */
        res = 0;
        for (i = 0; i < FOM_T0_DIM; ++i) {
            fom_float v = t0[i], d = v - x[i];
            d = d < 0 ? -d : d;
            res = d > res ? d : res;
            x[i] = v;
        }
        for (i = 0; i < FOM_T1_DIM; ++i) {
            fom_float v = t1[i], d = v - x[FOM_T0_DIM + i];
            d = d < 0 ? -d : d;
            res = d > res ? d : res;
            x[FOM_T0_DIM + i] = v;
        }
        for (i = FOM_T0_DIM + FOM_T1_DIM; i < FOM_STATE_DIM; ++i) {
            fom_float v = xn[i], d = v - x[i];
            d = d < 0 ? -d : d;
            res = d > res ? d : res;
            x[i] = v;
        }
        FOM_PROF_END(update);
    }

    memcpy(work->solution.x, x, sizeof(work->solution.x));
    work->solution.iter = it;
    work->solution.residual = res;
    work->solution.status = FOM_MAX_ITER_REACHED;
    return work->solution.status;
}

void fom_cleanup(FomWorkspace *work) { (void)work; }
