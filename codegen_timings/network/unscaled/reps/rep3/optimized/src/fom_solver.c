#include "fom_solver.h"
#include "fom_data.h"
#include "fom_ops.h"
#include "fom_profile.h"

void fom_set_default_settings(FomSettings *settings) {
    settings->max_iter = FOM_MAX_ITER;
    settings->tol = FOM_TOL;
    settings->check_termination = FOM_CHECK_TERMINATION;
}

static void reset_state(FomWorkspace *work) {
    fom_int i;
    for (i = 0; i < FOM_STATE_DIM; ++i) work->state[i] = FOM_STATE_INIT[i];
}

void fom_setup(FomWorkspace *work, const FomSettings *settings) {
    fom_int i;
    work->settings = *settings;
    for (i = 0; i < FOM_PARAM_P0_DIM; ++i) work->params.p0[i] = 0.0;
    reset_state(work);
}

void fom_update_params(FomWorkspace *work, const FomParams *params) {
    work->params = *params;
    reset_state(work);
}

fom_int fom_solve(FomWorkspace *work) {
    fom_float *cur = work->state, *nxt = work->new_state, *tmp;
    fom_float *bias_su = work->acc;
    fom_int i, k, n = work->settings.max_iter;
    fom_float res = 0.0;

    /* Hoisted parameter term of the state update. */
    {
        FOM_PROF_BEGIN(param_bias);
        for (i = 0; i < FOM_STATE_DIM; ++i) bias_su[i] = 0.0;
        fom_apply_su_p_p0(work->params.p0, bias_su);
        FOM_PROF_END(param_bias);
    }
    {
        FOM_PROF_BEGIN(iterate);
        for (k = 0; k < n; ++k) {
            fom_iterate(cur, FOM_MAT_t0_c, FOM_MAT_t1_c, bias_su, &FOM_ACT_t0,
                        &FOM_ACT_t1, work->t0, work->t1, nxt);
            tmp = cur; cur = nxt; nxt = tmp;
        }
        FOM_PROF_END(iterate);
    }
    if (n > 0) {
        for (i = 0; i < FOM_STATE_DIM; ++i) {
            fom_float d = cur[i] - nxt[i];
            if (d < 0) d = -d;
            if (d > res) res = d;
        }
    }
    for (i = 0; i < FOM_STATE_DIM; ++i) {
        work->solution.x[i] = cur[i];
        work->state[i] = FOM_STATE_INIT[i];
    }
    work->solution.iter = n;
    work->solution.residual = res;
    work->solution.status = FOM_MAX_ITER_REACHED;
    return FOM_MAX_ITER_REACHED;
}

void fom_cleanup(FomWorkspace *work) { (void)work; }
