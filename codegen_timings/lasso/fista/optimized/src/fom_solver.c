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

/* Structure exploited (see spec.json / fom_data.c):
 *   t0_state          = [ 0 | M ]   (left 15x15 block is zero)
 *   t0_state__d_gamma = [-M | M ]   (so A x + g D x = M (x2 + g (x2 - x1)))
 *   M is symmetric, so row j of M equals column j (axpy form, contiguous).
 *   su_state / su_t_t0 are 0/1 selection matrices: new state = [x2, t0].
 * Hence one 15x15 matvec per iteration and the state update is a pointer
 * rotation over three 15-vectors (no copies). */
#define FOM_N FOM_T0_DIM
#define FOM_M_ROW(j) (FOM_MAT_t0_state + (j) * FOM_MAT_t0_state_COLS + FOM_N)

fom_int fom_solve(FomWorkspace *work) {
    fom_float *x1 = work->state;            /* x_{k-1} */
    fom_float *x2 = work->state + FOM_N;    /* x_k     */
    fom_float *xn = work->new_state;        /* x_{k+1} */
    fom_float *cp = work->acc;              /* t0_p_p0 @ p0, constant */
    fom_float *y = work->acc + FOM_N;       /* extrapolated point */
    const fom_int max_iter = work->settings.max_iter;
    const fom_int use_tol = work->settings.check_termination && work->settings.tol > 0;
    const fom_float tol = work->settings.tol;
    const fom_float lam = FOM_T0_LAMBDA;
    fom_float res = 0.0;
    fom_int status = FOM_MAX_ITER_REACHED;
    fom_int it, i, j;

    FOM_PROF_BEGIN(param_const);
    memset(cp, 0, FOM_N * sizeof(fom_float));
    fom_apply_t0_p_p0(work->params.p0, cp);
    FOM_PROF_END(param_const);

    FOM_PROF_BEGIN(iterations);
    for (it = 0; it < max_iter; ++it) {
        const fom_float gamma = fom_sched_gamma(it);
        fom_float t[FOM_N];

        /* y = x2 + gamma (x2 - x1) */
        for (i = 0; i < FOM_N; ++i) y[i] = x2[i] + gamma * (x2[i] - x1[i]);

        /* t = cp + M y  (M symmetric: accumulate rows as columns) */
        /* Four independent partial sums shorten the FMA dependency chain
           from 15 to 4 (the loop is latency-bound, not throughput-bound). */
        {
            fom_float ta[FOM_N], tb[FOM_N], tc[FOM_N];
            for (i = 0; i < FOM_N; ++i) { t[i] = cp[i]; ta[i] = 0.0; tb[i] = 0.0; tc[i] = 0.0; }
            for (j = 0; j + 3 < FOM_N; j += 4) {
                const fom_float *m0 = FOM_M_ROW(j), *m1 = FOM_M_ROW(j + 1);
                const fom_float *m2 = FOM_M_ROW(j + 2), *m3 = FOM_M_ROW(j + 3);
                const fom_float y0 = y[j], y1 = y[j + 1], y2 = y[j + 2], y3 = y[j + 3];
                for (i = 0; i < FOM_N; ++i) {
                    t[i] += m0[i] * y0;
                    ta[i] += m1[i] * y1;
                    tb[i] += m2[i] * y2;
                    tc[i] += m3[i] * y3;
                }
            }
            for (; j < FOM_N; ++j) {
                const fom_float *mj = FOM_M_ROW(j);
                const fom_float yj = y[j];
                for (i = 0; i < FOM_N; ++i) ta[i] += mj[i] * yj;
            }
            for (i = 0; i < FOM_N; ++i) t[i] = (t[i] + ta[i]) + (tb[i] + tc[i]);
        }

        /* xn = soft_threshold(t, lam) */
        for (i = 0; i < FOM_N; ++i) {
            const fom_float v = t[i];
            xn[i] = (v > lam) ? (v - lam) : ((v < -lam) ? (v + lam) : 0.0);
        }

        /* residual on new state [x2, xn] vs old [x1, x2] */
        if (use_tol || it == max_iter - 1) {
            res = 0.0;
            for (i = 0; i < FOM_N; ++i) {
                fom_float d = x2[i] - x1[i];
                fom_float e = xn[i] - x2[i];
                if (d < 0) d = -d;
                if (e < 0) e = -e;
                if (d > res) res = d;
                if (e > res) res = e;
            }
        }

        /* state shift: (x1, x2) <- (x2, xn), recycle x1 buffer */
        { fom_float *tmp = x1; x1 = x2; x2 = xn; xn = tmp; }

        if (use_tol && res <= tol) {
            ++it;
            status = FOM_SOLVED;
            break;
        }
    }
    FOM_PROF_END(iterations);

    FOM_PROF_BEGIN(finalize);
    /* x1/x2 may alias any of the three buffers; stage through solution.x */
    memcpy(work->solution.x, x1, FOM_N * sizeof(fom_float));
    memcpy(work->solution.x + FOM_N, x2, FOM_N * sizeof(fom_float));
    memcpy(work->state, work->solution.x, sizeof(work->state));
    FOM_PROF_END(finalize);
    work->solution.iter = it;
    work->solution.residual = res;
    work->solution.status = status;
    return status;
}

void fom_cleanup(FomWorkspace *work) {
    (void)work;
}
