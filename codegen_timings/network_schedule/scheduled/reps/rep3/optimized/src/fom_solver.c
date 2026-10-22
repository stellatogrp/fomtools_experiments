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
    const fom_int max_iter = work->settings.max_iter;
    fom_float res = 0;
    fom_int it, i;

    /* Hoisted parameter term of the state update. */
    memset(bias, 0, sizeof(fom_float) * FOM_STATE_DIM);
    fom_apply_su_p_p0(work->params.p0, bias);

    for (it = 0; it < max_iter; ++it) {
        const fom_float xi = fom_sched_xi(it);
        fom_float *tmp;
        /* Selection rows of su: rows 0..599 are t0 and 600..619 are t1, so
           both temporaries are written straight into the new state. */
        FOM_PROF_BEGIN(iterate);
        fom_iterate(x, xi, FOM_MAT_t0_c, FOM_MAT_t1_c, bias, &FOM_ACT_t0, &FOM_ACT_t1,
                    xn, xn + FOM_T0_DIM, xn);
        FOM_PROF_END(iterate);
        /* Only the final residual is reported (no tolerance check). */
        if (it == max_iter - 1) {
            FOM_PROF_BEGIN(residual);
            res = 0;
            for (i = 0; i < FOM_STATE_DIM; ++i) {
                fom_float d = xn[i] - x[i];
                d = d < 0 ? -d : d;
                res = d > res ? d : res;
            }
            FOM_PROF_END(residual);
        }
        tmp = x; x = xn; xn = tmp;
    }

    if (x != work->state) memcpy(work->state, x, sizeof(work->state));
    memcpy(work->t0, work->state, sizeof(work->t0));
    memcpy(work->t1, work->state + FOM_T0_DIM, sizeof(work->t1));
    memcpy(work->solution.x, work->state, sizeof(work->solution.x));
    work->solution.iter = it;
    work->solution.residual = res;
    work->solution.status = FOM_MAX_ITER_REACHED;
    return work->solution.status;
}

void fom_cleanup(FomWorkspace *work) { (void)work; }
