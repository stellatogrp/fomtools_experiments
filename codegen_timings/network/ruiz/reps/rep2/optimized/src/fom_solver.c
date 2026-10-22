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

static void fom_reset_state(FomWorkspace *work) {
    memcpy(work->state, FOM_STATE_INIT, sizeof(work->state));
}

/* acc holds the hoisted state-update bias su_p_p0 @ p0 (rest rows only matter). */
static void fom_hoist_bias(FomWorkspace *work) {
    memset(work->acc, 0, sizeof(work->acc));
    fom_apply_su_p_p0(work->params.p0, work->acc);
}

void fom_setup(FomWorkspace *work, const FomSettings *settings) {
    work->settings = *settings;
    memset(&work->params, 0, sizeof(work->params));
    memset(&work->solution, 0, sizeof(work->solution));
    fom_reset_state(work);
    fom_hoist_bias(work);
}

void fom_update_params(FomWorkspace *work, const FomParams *params) {
    work->params = *params;
    fom_reset_state(work);
    fom_hoist_bias(work);
}

fom_int fom_solve(FomWorkspace *work) {
    fom_float *x = work->state, *xn = work->new_state, *t0 = work->t0, *t1 = work->t1;
    const fom_int max_iter = work->settings.max_iter;
    const fom_int use_tol = work->settings.check_termination && work->settings.tol > 0;
    const fom_float tol = work->settings.tol;
    fom_float res = 0;
    fom_int k, i, status = FOM_MAX_ITER_REACHED;

    for (k = 0; k < max_iter; ++k) {
        fom_float *tmp;
        /* selection rows of su: t0 and t1 are written straight into rows 0..58 of xn */
        FOM_PROF_BEGIN(step_t0);
        fom_step_t0(x, FOM_MAT_t0_c, &FOM_ACT_t0, xn);
        FOM_PROF_END(step_t0);
        FOM_PROF_BEGIN(step_t1);
        fom_step_t1(x, xn, FOM_MAT_t1_c, &FOM_ACT_t1, xn + FOM_T0_DIM);
        FOM_PROF_END(step_t1);
        FOM_PROF_BEGIN(step_su_rest);
        fom_step_su__rest(x, xn, xn + FOM_T0_DIM, work->acc, xn);
        FOM_PROF_END(step_su_rest);
        if (use_tol || k == max_iter - 1) {
            res = 0;
            for (i = 0; i < FOM_STATE_DIM; ++i) {
                fom_float d = xn[i] - x[i];
                d = d < 0 ? -d : d;
                res = d > res ? d : res;
            }
        }
        tmp = x; x = xn; xn = tmp;
        if (use_tol && res <= tol) { status = FOM_SOLVED; ++k; break; }
    }
    if (x != work->state) memcpy(work->state, x, sizeof(work->state));
    if (k > 0) {
        memcpy(t0, x, FOM_T0_DIM * sizeof(fom_float));
        memcpy(t1, x + FOM_T0_DIM, FOM_T1_DIM * sizeof(fom_float));
    }
    memcpy(work->solution.x, work->state, sizeof(work->solution.x));
    work->solution.iter = k;
    work->solution.residual = res;
    work->solution.status = status;
    return status;
}

void fom_cleanup(FomWorkspace *work) { (void)work; }
