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
    fom_int i;
    for (i = 0; i < FOM_STATE_DIM; ++i) work->state[i] = FOM_STATE_INIT[i];
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
    fom_float *x = work->state;
    fom_float *xn = work->new_state;
    fom_float *t0 = work->t0;
    fom_float *t1 = work->t1;
    fom_float res = 0.0;
    const fom_int max_iter = work->settings.max_iter;
    fom_int it, i;
    fom_float pterm[FOM_STATE_DIM];
    fom_float bias_t0[FOM_T0_DIM];
    fom_float bias_t1[FOM_T1_DIM];

    /* su_p_p0 @ p0 is iteration-invariant: hoist it. */
    for (i = 0; i < FOM_STATE_DIM; ++i) pterm[i] = 0.0;
    fom_apply_su_p_p0(work->params.p0, pterm);

    for (i = 0; i < FOM_T0_DIM; ++i) bias_t0[i] = FOM_MAT_t0_c[i];
    for (i = 0; i < FOM_T1_DIM; ++i) bias_t1[i] = FOM_MAT_t1_c[i];

    for (it = 0; it < max_iter; ++it) {
        const fom_float xi = fom_sched_xi(it);

        /* t0, t1 and the state update in one fused pass. */
        FOM_PROF_BEGIN(iterate);
        fom_iterate(x, xi, bias_t0, bias_t1, pterm, &FOM_ACT_t0, &FOM_ACT_t1, t0, t1, xn);
        FOM_PROF_END(iterate);

        if (it + 1 == max_iter) {
            FOM_PROF_BEGIN(residual);
            res = 0.0;
            for (i = 0; i < FOM_STATE_DIM; ++i) {
                fom_float d = xn[i] - x[i];
                d = d < 0.0 ? -d : d;
                res = d > res ? d : res;
            }
            FOM_PROF_END(residual);
        }

        { fom_float *tmp = x; x = xn; xn = tmp; }
    }

    if (x != work->state)
        for (i = 0; i < FOM_STATE_DIM; ++i) work->state[i] = x[i];
    for (i = 0; i < FOM_STATE_DIM; ++i) work->solution.x[i] = x[i];
    work->solution.iter = max_iter;
    work->solution.residual = res;
    work->solution.status = FOM_MAX_ITER_REACHED;
    return FOM_MAX_ITER_REACHED;
}

void fom_cleanup(FomWorkspace *work) {
    (void)work;
}
