#include "fom_solver.h"
#include "fom_data.h"
#include "fom_ops.h"

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
    for (i = 0; i < FOM_PARAM_P0_DIM; ++i) work->params.p0[i] = 0;
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
    fom_float *s = work->state, *ns = work->new_state, *bias = work->acc;
    fom_int i, k, n = work->settings.max_iter;
    fom_float res = 0;

    for (i = 0; i < FOM_STATE_DIM; ++i) bias[i] = 0;
    fom_apply_su_p_p0(work->params.p0, bias);

    for (k = 0; k < n; ++k) {
        fom_float *t;
        fom_iterate(s, FOM_MAT_t0_c, FOM_MAT_t1_c, bias, &FOM_ACT_t0, &FOM_ACT_t1,
                    work->t0, work->t1, ns);
        if (k == n - 1) {
            res = 0;
            for (i = 0; i < FOM_STATE_DIM; ++i) {
                fom_float d = ns[i] - s[i];
                if (d < 0) d = -d;
                if (d > res) res = d;
            }
        }
        t = s; s = ns; ns = t;
    }
    if (s != work->state)
        for (i = 0; i < FOM_STATE_DIM; ++i) work->state[i] = s[i];

    for (i = 0; i < FOM_STATE_DIM; ++i) work->solution.x[i] = work->state[i];
    work->solution.iter = n;
    work->solution.residual = res;
    work->solution.status = FOM_MAX_ITER_REACHED;
    return work->solution.status;
}

void fom_cleanup(FomWorkspace *work) { (void)work; }
