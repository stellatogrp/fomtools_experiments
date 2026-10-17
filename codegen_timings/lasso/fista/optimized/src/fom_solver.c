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

static void fom_reset_state(FomWorkspace *work) {
    memcpy(work->state, FOM_STATE_INIT, sizeof(work->state));
}

void fom_setup(FomWorkspace *work, const FomSettings *settings) {
    work->settings = *settings;
    memset(&work->params, 0, sizeof(work->params));
    memset(&work->solution, 0, sizeof(work->solution));
    fom_reset_state(work);
}

void fom_update_params(FomWorkspace *work, const FomParams *params) {
    work->params = *params;
    fom_reset_state(work);
}

/* t0_state has nonzeros only in the s1 columns (50..99), and
   t0_state__d_gamma == [-A1, A1] with A1 = t0_state[:, 50:100].  Hence
   A x + gamma D x = A1 (s1 + gamma (s1 - s0)): one 50x50 matvec per iteration.
   A1 is extracted once (unit-vector probing of the fixed operator) into a
   dense column-major table so the inner product vectorizes. */
#define N FOM_T0_DIM
static fom_float A1[N * N];
static int A1_ready = 0;

static void fom_extract_a1(void) {
    fom_float e[FOM_STATE_DIM], col[N];
    fom_int i, j;
    memset(e, 0, sizeof(e));
    for (j = 0; j < N; ++j) {
        e[N + j] = 1.0;
        memset(col, 0, sizeof(col));
        fom_apply_t0_state(e, col);
        for (i = 0; i < N; ++i) A1[j * N + i] = col[i];  /* column-major */
        e[N + j] = 0.0;
    }
    A1_ready = 1;
}

fom_int fom_solve(FomWorkspace *work) {
    fom_float *s0 = work->state;
    fom_float *s1 = work->state + N;
    fom_float y[N];
    fom_float *cp = work->acc + N;       /* t0_p_p0 @ p0, constant (50) */
    fom_float t[N];
    const fom_int max_iter = work->settings.max_iter;
    const fom_int use_tol = work->settings.check_termination && work->settings.tol > 0;
    const fom_float tol = work->settings.tol;
    const fom_float lam = FOM_T0_LAMBDA;
    fom_float res = 0.0;
    fom_int status = FOM_MAX_ITER_REACHED;
    fom_int it, i, j;

    if (!A1_ready) fom_extract_a1();

    /* Parameter contribution is iteration-invariant: hoist it. */
    memset(cp, 0, N * sizeof(fom_float));
    fom_apply_t0_p_p0(work->params.p0, cp);

    for (it = 0; it < max_iter; ++it) {
        const fom_float gamma = fom_sched_gamma(it);
        const fom_int need_res = use_tol || it == max_iter - 1;

        FOM_PROF_BEGIN(extrapolate);
        for (i = 0; i < N; ++i) y[i] = s1[i] + gamma * (s1[i] - s0[i]);
        FOM_PROF_END(extrapolate);

        FOM_PROF_BEGIN(matvec_prox);
        /* column-oriented axpy form: vectorizes across rows with no
           reassociation of floating-point sums */
        {
            fom_float *restrict tt = t;
            const fom_float *restrict yy = y;
            memcpy(tt, cp, N * sizeof(fom_float));
            for (j = 0; j < N; ++j) {
                const fom_float *restrict a = A1 + j * N;
                const fom_float yj = yy[j];
                for (i = 0; i < N; ++i) tt[i] += a[i] * yj;
            }
            for (i = 0; i < N; ++i) {
                const fom_float v = tt[i];
                tt[i] = (v > lam) ? (v - lam) : ((v < -lam) ? (v + lam) : 0.0);
            }
        }
        FOM_PROF_END(matvec_prox);

        FOM_PROF_BEGIN(state_update);
        if (need_res) {
            res = 0.0;
            for (i = 0; i < N; ++i) {
                fom_float d0 = s1[i] - s0[i], d1 = t[i] - s1[i];
                if (d0 < 0) d0 = -d0;
                if (d1 < 0) d1 = -d1;
                if (d0 > res) res = d0;
                if (d1 > res) res = d1;
            }
        }
        memcpy(s0, s1, N * sizeof(fom_float));
        memcpy(s1, t, N * sizeof(fom_float));
        FOM_PROF_END(state_update);

        if (use_tol && res <= tol) {
            ++it;
            status = FOM_SOLVED;
            break;
        }
    }

    memcpy(work->solution.x, work->state, sizeof(work->solution.x));
    work->solution.iter = it;
    work->solution.residual = res;
    work->solution.status = status;
    return status;
}

void fom_cleanup(FomWorkspace *work) {
    (void)work;
}
