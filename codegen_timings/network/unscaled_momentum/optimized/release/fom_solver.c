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
    fom_float *acc = work->acc;
    fom_float *t0 = work->t0;
    fom_float *t1 = work->t1;
    fom_float res = 0.0;
    const fom_int max_iter = work->settings.max_iter;
    fom_int it, i;

    for (it = 0; it < max_iter; ++it) {
        const fom_float xi = fom_sched_xi(it);

        /* t0 = clip(t0_c + t0_state @ x, lo, hi) */
        for (i = 0; i < FOM_T0_DIM; ++i) t0[i] = FOM_MAT_t0_c[i];
        fom_apply_t0_state(x, t0);
        for (i = 0; i < FOM_T0_DIM; ++i) {
            fom_float v = t0[i];
            const fom_float lo = FOM_MAT_t0_lo[i];
            const fom_float hi = FOM_MAT_t0_hi[i];
            v = v < lo ? lo : v;
            v = v > hi ? hi : v;
            t0[i] = v;
        }

        /* t1 = relu(t1_c + t1_state@x + xi*(d_xi@x) + t1_t_t0@t0 + xi*(d_xi@t0)) */
        for (i = 0; i < FOM_T1_DIM; ++i) { t1[i] = FOM_MAT_t1_c[i]; acc[i] = 0.0; }
        fom_apply_t1_state(x, t1);
        fom_apply_t1_t_t0(t0, t1);
        fom_apply_t1_state__d_xi(x, acc);
        fom_apply_t1_t_t0__d_xi(t0, acc);
        for (i = 0; i < FOM_T1_DIM; ++i) {
            const fom_float v = t1[i] + xi * acc[i];
            t1[i] = v > 0.0 ? v : 0.0;
        }

        /* State update */
        for (i = 0; i < FOM_STATE_DIM; ++i) { xn[i] = 0.0; acc[i] = 0.0; }
        fom_apply_su_state(x, xn);
        fom_apply_su_t_t0(t0, xn);
        fom_apply_su_t_t1(t1, xn);
        fom_apply_su_p_p0(work->params.p0, xn);
        fom_apply_su_state__d_xi(x, acc);
        fom_apply_su_t_t0__d_xi(t0, acc);
        for (i = 0; i < FOM_STATE_DIM; ++i) xn[i] += xi * acc[i];

        if (it + 1 == max_iter) {
            res = 0.0;
            for (i = 0; i < FOM_STATE_DIM; ++i) {
                fom_float d = xn[i] - x[i];
                d = d < 0.0 ? -d : d;
                res = d > res ? d : res;
            }
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
