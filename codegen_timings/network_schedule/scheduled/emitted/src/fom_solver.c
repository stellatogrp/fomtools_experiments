#include <string.h>
#include "fom_solver.h"
#include "fom_data.h"
#include "fom_ops.h"
#include "fom_linsolve.h"

void fom_set_default_settings(FomSettings *settings) {
    settings->max_iter = FOM_MAX_ITER;
    settings->tol = FOM_TOL;
    settings->check_termination = FOM_CHECK_TERMINATION;
}

void fom_setup(FomWorkspace *work, const FomSettings *settings) {
    work->settings = *settings;
    memcpy(work->state, FOM_STATE_INIT, sizeof(work->state));
}

void fom_update_params(FomWorkspace *work, const FomParams *params) {
    work->params = *params;
    memcpy(work->state, FOM_STATE_INIT, sizeof(work->state));
}

fom_int fom_solve(FomWorkspace *work) {
    fom_float *cur = work->state;
    fom_float *nxt = work->new_state;
    fom_float *bias_su = work->acc;
    const fom_int max_iter = work->settings.max_iter;
    fom_float res = 0;
    fom_int it, i;

    /* Parameter term of the state update is loop-invariant. */
    memset(bias_su, 0, sizeof(fom_float) * FOM_STATE_DIM);
    fom_apply_su_p_p0(work->params.p0, bias_su);

    for (it = 0; it < max_iter; ++it) {
        const fom_float xi = fom_sched_xi(it);
        fom_float *tmp;
        /* Rows 0..599 of the update copy t0 and rows 600..619 copy t1
           (FOM_SELROWS_su), so the steps write straight into nxt. */
        fom_step_t0(cur, FOM_MAT_t0_c, &FOM_ACT_t0, nxt);
        fom_step_t1(cur, nxt, xi, FOM_MAT_t1_c, &FOM_ACT_t1, nxt + FOM_T0_DIM);
        fom_step_su__rest(cur, nxt, nxt + FOM_T0_DIM, xi, bias_su, nxt);
        tmp = cur; cur = nxt; nxt = tmp;
    }

    /* Residual of the last iteration only (no tolerance check). */
    if (max_iter > 0) {
        for (i = 0; i < FOM_STATE_DIM; ++i) {
            fom_float d = cur[i] - nxt[i];
            d = d < 0 ? -d : d;
            res = d > res ? d : res;
        }
    }
    memcpy(work->t0, cur, sizeof(work->t0));
    memcpy(work->t1, cur + FOM_T0_DIM, sizeof(work->t1));
    if (cur != work->state) memcpy(work->state, cur, sizeof(work->state));
    memcpy(work->solution.x, cur, sizeof(work->solution.x));
    work->solution.iter = max_iter > 0 ? max_iter : 0;
    work->solution.residual = res;
    work->solution.status = FOM_MAX_ITER_REACHED;
    return work->solution.status;
}

void fom_cleanup(FomWorkspace *work) { (void)work; }
