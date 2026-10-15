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
    fom_float *state = work->state;
    fom_float *ns = work->new_state;
    fom_float *acc = work->acc;
    fom_float *t0 = work->t0;
    fom_float *t1 = work->t1;
    const fom_int max_iter = work->settings.max_iter;
    const fom_int use_tol = FOM_USE_TOL && work->settings.check_termination;
    const fom_float tol = work->settings.tol;
    fom_float res = 0;
    fom_int it, i;

    /* Parameter contribution to the state update is loop-invariant; cache it
       in solution.x, which is only written with the result after the loop. */
    fom_float *pterm = work->solution.x;
    memset(pterm, 0, sizeof(fom_float) * FOM_STATE_DIM);
    fom_apply_su_p_p0(work->params.p0, pterm);

    for (it = 0; it < max_iter; ++it) {
        const fom_float xi = fom_sched_xi(it);

        /* t0 = clip(t0_c + t0_state @ state, lo, hi) */
        memcpy(t0, FOM_MAT_t0_c, sizeof(fom_float) * FOM_T0_DIM);
        fom_apply_t0_state(state, t0);
        for (i = 0; i < FOM_T0_DIM; ++i) {
            fom_float v = t0[i];
            v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v;
            v = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v;
            t0[i] = v;
        }

        /* t1 = relu(t1_c + t1_state@s + xi*(d@s) + t1_t_t0@t0 + xi*(d@t0)) */
        memcpy(t1, FOM_MAT_t1_c, sizeof(fom_float) * FOM_T1_DIM);
        fom_apply_t1_state(state, t1);
        fom_apply_t1_t_t0(t0, t1);
        if (xi != 0) {
            memset(acc, 0, sizeof(fom_float) * FOM_T1_DIM);
            fom_apply_t1_state__d_xi(state, acc);
            fom_apply_t1_t_t0__d_xi(t0, acc);
            for (i = 0; i < FOM_T1_DIM; ++i) t1[i] += xi * acc[i];
        }
        for (i = 0; i < FOM_T1_DIM; ++i) t1[i] = t1[i] > 0 ? t1[i] : 0;

        /* state update */
        memcpy(ns, pterm, sizeof(fom_float) * FOM_STATE_DIM);
        fom_apply_su_state(state, ns);
        fom_apply_su_t_t0(t0, ns);
        fom_apply_su_t_t1(t1, ns);
        if (xi != 0) {
            memset(acc, 0, sizeof(fom_float) * FOM_STATE_DIM);
            fom_apply_su_state__d_xi(state, acc);
            fom_apply_su_t_t0__d_xi(t0, acc);
            for (i = 0; i < FOM_STATE_DIM; ++i) ns[i] += xi * acc[i];
        }

        res = 0;
        for (i = 0; i < FOM_STATE_DIM; ++i) {
            fom_float d = ns[i] - state[i];
            d = d < 0 ? -d : d;
            res = d > res ? d : res;
        }
        memcpy(state, ns, sizeof(fom_float) * FOM_STATE_DIM);

        if (use_tol && res <= tol) {
            ++it;
            work->solution.iter = it;
            work->solution.residual = res;
            work->solution.status = FOM_SOLVED;
            memcpy(work->solution.x, state, sizeof(work->solution.x));
            return FOM_SOLVED;
        }
    }

    work->solution.iter = it;
    work->solution.residual = res;
    work->solution.status = FOM_MAX_ITER_REACHED;
    memcpy(work->solution.x, state, sizeof(work->solution.x));
    return FOM_MAX_ITER_REACHED;
}

void fom_cleanup(FomWorkspace *work) {
    (void)work;
}
