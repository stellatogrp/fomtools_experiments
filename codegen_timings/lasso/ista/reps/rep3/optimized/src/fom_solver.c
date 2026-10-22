#include "fom_solver.h"
#include "fom_data.h"
#include "fom_ops.h"
#include "fom_profile.h"

/* su_t_t0 is the identity (SPEC), so the state update is new_state = t0 and
   fom_step_t0 writes straight into the next state buffer. The parameter term
   t0_p_p0 @ p0 is constant across iterations and hoisted into work->acc. */

static void fom_reset_state(FomWorkspace *work) {
    fom_int i;
    for (i = 0; i < FOM_STATE_DIM; ++i) work->state[i] = FOM_STATE_INIT[i];
}

static void fom_compute_bias(FomWorkspace *work) {
    fom_int i;
    for (i = 0; i < FOM_T0_DIM; ++i) work->acc[i] = 0.0;
    fom_apply_t0_p_p0(work->params.p0, work->acc);
}

void fom_set_default_settings(FomSettings *settings) {
    settings->max_iter = FOM_MAX_ITER;
    settings->tol = FOM_TOL;
    settings->check_termination = FOM_CHECK_TERMINATION;
}

void fom_setup(FomWorkspace *work, const FomSettings *settings) {
    fom_int i;
    work->settings = *settings;
    for (i = 0; i < FOM_PARAM_P0_DIM; ++i) work->params.p0[i] = 0.0;
    fom_compute_bias(work);
    fom_reset_state(work);
    work->solution.iter = 0;
    work->solution.residual = 0.0;
    work->solution.status = FOM_MAX_ITER_REACHED;
}

void fom_update_params(FomWorkspace *work, const FomParams *params) {
    work->params = *params;
    fom_compute_bias(work);
    fom_reset_state(work);
}

fom_int fom_solve(FomWorkspace *work) {
    fom_float *a = work->state;
    fom_float *b = work->new_state;
    fom_float *t;
    const fom_float *bias = work->acc;
    fom_int max_iter = work->settings.max_iter;
    fom_int k, i;
    fom_float res = 0.0;

    FOM_PROF_BEGIN(iterations);
    for (k = 0; k < max_iter; ++k) {
        fom_step_t0(a, bias, &FOM_ACT_t0, b);
        t = a; a = b; b = t;
    }
    FOM_PROF_END(iterations);
    FOM_PROF_BEGIN(residual_output);
    /* a holds the final iterate, b the previous one. */
    if (max_iter > 0) {
        for (i = 0; i < FOM_STATE_DIM; ++i) {
            fom_float d = a[i] - b[i];
            if (d < 0) d = -d;
            if (d > res) res = d;
        }
    }
    for (i = 0; i < FOM_STATE_DIM; ++i) {
        work->solution.x[i] = a[i];
        work->state[i] = FOM_STATE_INIT[i];
    }
    FOM_PROF_END(residual_output);
    work->solution.iter = max_iter;
    work->solution.residual = res;
    work->solution.status = FOM_MAX_ITER_REACHED;
    return FOM_MAX_ITER_REACHED;
}

void fom_cleanup(FomWorkspace *work) {
    (void)work;
}
