#include "fom_solver.h"
#include "fom_data.h"
#include "fom_ops.h"
#include "fom_linsolve.h"

void fom_set_default_settings(FomSettings *settings) {
    settings->max_iter = FOM_MAX_ITER;
    settings->tol = FOM_TOL;
    settings->check_termination = FOM_CHECK_TERMINATION;
}

static void fom_reset_state(FomWorkspace *work) {
    fom_int i;
    for (i = 0; i < FOM_STATE_DIM; ++i) work->state[i] = FOM_STATE_INIT[i];
}

/* Hoisted state-update bias: acc = su_p_p0 @ p0 (params fixed during a solve). */
static void fom_update_bias(FomWorkspace *work) {
    fom_int i;
    for (i = 0; i < FOM_STATE_DIM; ++i) work->acc[i] = 0.0;
    fom_apply_su_p_p0(work->params.p0, work->acc);
}

void fom_setup(FomWorkspace *work, const FomSettings *settings) {
    fom_int i;
    work->settings = *settings;
    for (i = 0; i < FOM_PARAM_P0_DIM; ++i) work->params.p0[i] = 0.0;
    fom_update_bias(work);
    fom_reset_state(work);
    work->solution.iter = 0;
    work->solution.residual = 0.0;
    work->solution.status = FOM_MAX_ITER_REACHED;
}

void fom_update_params(FomWorkspace *work, const FomParams *params) {
    work->params = *params;
    fom_update_bias(work);
    fom_reset_state(work);
}

fom_int fom_solve(FomWorkspace *work) {
    fom_int it, i;
    const fom_int max_iter = work->settings.max_iter;
    fom_float *x = work->state;
    fom_float *xn = work->new_state;
    fom_float res = 0.0;

    for (it = 0; it < max_iter; ++it) {
        fom_float *tmp;
        const fom_float xi = fom_sched_xi(it);
        fom_iterate(x, xi, FOM_MAT_t0_c, FOM_MAT_t1_c, work->acc,
                    &FOM_ACT_t0, &FOM_ACT_t1, work->t0, work->t1, xn);
        if (it == max_iter - 1) {
            res = 0.0;
            for (i = 0; i < FOM_STATE_DIM; ++i) {
                fom_float d = xn[i] - x[i];
                if (d < 0) d = -d;
                if (d > res) res = d;
            }
        }
        tmp = x; x = xn; xn = tmp;
    }
    if (x != work->state) {
        for (i = 0; i < FOM_STATE_DIM; ++i) work->state[i] = x[i];
    }
    for (i = 0; i < FOM_STATE_DIM; ++i) work->solution.x[i] = work->state[i];
    work->solution.iter = max_iter;
    work->solution.residual = res;
    work->solution.status = FOM_MAX_ITER_REACHED;
    return FOM_MAX_ITER_REACHED;
}

void fom_cleanup(FomWorkspace *work) {
    (void)work;
}
