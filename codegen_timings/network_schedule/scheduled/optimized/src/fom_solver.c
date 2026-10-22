#include <string.h>
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
        /* One fused pass per node block: t0 block, t1 row and the su rest
           rows consume each t0 value while it is in registers. */
        FOM_PROF_BEGIN(iterate);
        fom_iterate(cur, xi, FOM_MAT_t0_c, FOM_MAT_t1_c, bias_su, &FOM_ACT_t0, &FOM_ACT_t1, work->t0, work->t1, nxt);
        FOM_PROF_END(iterate);
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
    if (cur != work->state) memcpy(work->state, cur, sizeof(work->state));
    memcpy(work->solution.x, cur, sizeof(work->solution.x));
    work->solution.iter = max_iter > 0 ? max_iter : 0;
    work->solution.residual = res;
    work->solution.status = FOM_MAX_ITER_REACHED;
    return work->solution.status;
}

void fom_cleanup(FomWorkspace *work) { (void)work; }
