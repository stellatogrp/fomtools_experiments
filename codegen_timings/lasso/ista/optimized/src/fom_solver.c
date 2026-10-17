#include "fom_solver.h"
#include "fom_data.h"
#include "fom_ops.h"
#include "fom_linsolve.h"
#include "fom_profile.h"

/* Column-major copy of the dense t0_state matrix, built once at setup, so
   the matvec becomes unit-stride column axpys that vectorize without
   reassociating a reduction. */
static fom_float T0_STATE_CM[FOM_T0_DIM * FOM_STATE_DIM];
static int t0_cm_ready = 0;

static void fom_build_cm(void) {
    fom_int i, j;
    if (t0_cm_ready) return;
    for (i = 0; i < FOM_T0_DIM; ++i)
        for (j = 0; j < FOM_STATE_DIM; ++j)
            T0_STATE_CM[j * FOM_T0_DIM + i] = FOM_MAT_t0_state[i * FOM_STATE_DIM + j];
    t0_cm_ready = 1;
}

static void fom_reset_state(FomWorkspace *work) {
    fom_int i;
    for (i = 0; i < FOM_STATE_DIM; ++i) work->state[i] = FOM_STATE_INIT[i];
}

void fom_set_default_settings(FomSettings *settings) {
    settings->max_iter = FOM_MAX_ITER;
    settings->tol = FOM_TOL;
    settings->check_termination = FOM_CHECK_TERMINATION;
}

void fom_setup(FomWorkspace *work, const FomSettings *settings) {
    fom_int i;
    work->settings = *settings;
    fom_build_cm();
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
    const fom_float lam = FOM_T0_LAMBDA;
    const fom_int max_iter = work->settings.max_iter;
    const fom_float tol = work->settings.tol;
    const fom_int check = work->settings.check_termination;
    const fom_int use_tol = (tol > 0.0) && (check > 0);
    fom_float *state = work->state;
    fom_float *ns = work->new_state;
    fom_float *acc = work->acc;
    fom_float res = 0.0;
    fom_int status = FOM_MAX_ITER_REACHED;
    fom_int k, i, j, ctr = 0;

    fom_build_cm();

    /* Parameter contribution is loop-invariant: acc = t0_p_p0 @ p0. */
    FOM_PROF_BEGIN(param_rhs);
    for (i = 0; i < FOM_T0_DIM; ++i) acc[i] = 0.0;
    fom_apply_t0_p_p0(work->params.p0, acc);
    FOM_PROF_END(param_rhs);

    FOM_PROF_BEGIN(iterations);
    for (k = 0; k < max_iter; ++k) {
        /* v = t0_state @ state + acc  (two fused column axpys per pass) */
        fom_float v[FOM_T0_DIM];
        for (i = 0; i < FOM_T0_DIM; ++i) v[i] = acc[i];
        for (j = 0; j < FOM_STATE_DIM; j += 2) {
            const fom_float xa = state[j], xb = state[j + 1];
            const fom_float *ca = T0_STATE_CM + j * FOM_T0_DIM;
            const fom_float *cb = ca + FOM_T0_DIM;
            for (i = 0; i < FOM_T0_DIM; ++i) {
                fom_float t = v[i] + ca[i] * xa;
                v[i] = t + cb[i] * xb;
            }
        }
        /* su_t_t0 is the identity: state <- soft_threshold(v, lam) directly. */
        for (i = 0; i < FOM_T0_DIM; ++i) {
            const fom_float t = v[i];
            ns[i] = t > lam ? t - lam : (t < -lam ? t + lam : 0.0);
        }

        if (use_tol || k == max_iter - 1) {
            res = 0.0;
            for (i = 0; i < FOM_STATE_DIM; ++i) {
                fom_float d = ns[i] - state[i];
                if (d < 0) d = -d;
                if (d > res) res = d;
            }
        }
        {
            fom_float *tmp = state; state = ns; ns = tmp;
        }
        if (use_tol && ++ctr >= check) {
            ctr = 0;
            if (res > tol) continue;
            ++k;
            status = FOM_SOLVED;
            break;
        }
    }
    FOM_PROF_END(iterations);

    if (state != work->state)
        for (i = 0; i < FOM_STATE_DIM; ++i) work->state[i] = state[i];
    for (i = 0; i < FOM_STATE_DIM; ++i) work->solution.x[i] = work->state[i];
    work->solution.iter = k;
    work->solution.residual = res;
    work->solution.status = status;
    return status;
}

void fom_cleanup(FomWorkspace *work) {
    (void)work;
}
