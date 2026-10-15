#include <string.h>
#include "fom_solver.h"
#include "fom_data.h"
#include "fom_ops.h"
#include "fom_linsolve.h"
#include "fom_profile.h"

/* out += c * (M @ x), via the accumulating helper into scratch `tmp`.
   Skipped entirely when the schedule coefficient is exactly zero. */
typedef void (*fom_apply_fn)(const fom_float *, fom_float *);

static void axpy_op(fom_apply_fn f, fom_float c, const fom_float *x,
                    fom_float *out, fom_float *tmp, fom_int n) {
    fom_int i;
    if (c == (fom_float)0) return;
    if (c == (fom_float)1) { f(x, out); return; }
    memset(tmp, 0, (size_t)n * sizeof(fom_float));
    f(x, tmp);
    for (i = 0; i < n; i++) out[i] += c * tmp[i];
}

void fom_set_default_settings(FomSettings *settings) {
    settings->max_iter = FOM_MAX_ITER;
    settings->tol = FOM_TOL;
    settings->check_termination = FOM_CHECK_TERMINATION;
}

static void reset_state(FomWorkspace *work) {
    memcpy(work->state, FOM_STATE_INIT, sizeof(work->state));
}

void fom_setup(FomWorkspace *work, const FomSettings *settings) {
    work->settings = *settings;
    memset(&work->params, 0, sizeof(work->params));
    reset_state(work);
    memcpy(work->solution.x, work->state, sizeof(work->state));
    work->solution.iter = 0;
    work->solution.residual = 0;
    work->solution.status = FOM_MAX_ITER_REACHED;
}

void fom_update_params(FomWorkspace *work, const FomParams *params) {
    work->params = *params;
    reset_state(work);
}

fom_int fom_solve(FomWorkspace *work) {
    fom_float *x = work->state;
    fom_float *xn = work->new_state;
    fom_float *acc = work->acc;
    fom_float *t0 = work->t0;
    fom_float *t1 = work->t1;
    const fom_float *p0 = work->params.p0;
    const fom_float *p1 = work->params.p1;
    const fom_int max_iter = work->settings.max_iter;
    fom_float res = 0;
    fom_int it, i;

    for (it = 0; it < max_iter; it++) {
        const fom_float d1 = fom_sched_d1(it), d2 = fom_sched_d2(it);
        const fom_float eta = fom_sched_eta(it);
        const fom_float g1 = fom_sched_g1(it), g2 = fom_sched_g2(it);
        const fom_float m1 = fom_sched_m1(it), m2 = fom_sched_m2(it);
        const fom_float tau = fom_sched_tau(it), taux = fom_sched_taux(it);
        const fom_float tth = fom_sched_tth(it);
        fom_float *tmp;

        /* t0 = relu(...) */
        memset(t0, 0, sizeof(work->t0));
        fom_apply_t0_state(x, t0);
        axpy_op(fom_apply_t0_state__d_taux, taux, x, t0, acc, FOM_T0_DIM);
        axpy_op(fom_apply_t0_state__d_tth, tth, x, t0, acc, FOM_T0_DIM);
        fom_apply_t0_p_p1(p1, t0);
        axpy_op(fom_apply_t0_p_p1__d_tau, tau, p1, t0, acc, FOM_T0_DIM);
        for (i = 0; i < FOM_T0_DIM; i++) t0[i] = t0[i] > 0 ? t0[i] : 0;

        /* t1 = clip(...) */
        memset(t1, 0, sizeof(work->t1));
        fom_apply_t1_state(x, t1);
        axpy_op(fom_apply_t1_state__d_eta, eta, x, t1, acc, FOM_T1_DIM);
        fom_apply_t1_p_p0(p0, t1);
        axpy_op(fom_apply_t1_p_p0__d_eta, eta, p0, t1, acc, FOM_T1_DIM);
        fom_apply_t1_t_t0(t0, t1);
        axpy_op(fom_apply_t1_t_t0__d_eta, eta, t0, t1, acc, FOM_T1_DIM);
        for (i = 0; i < FOM_T1_DIM; i++) {
            fom_float v = t1[i];
            if (v < FOM_MAT_t1_lo[i]) v = FOM_MAT_t1_lo[i];
            if (v > FOM_MAT_t1_hi[i]) v = FOM_MAT_t1_hi[i];
            t1[i] = v;
        }

        /* state update */
        memset(xn, 0, FOM_STATE_DIM * sizeof(fom_float));
        fom_apply_su_state(x, xn);
        axpy_op(fom_apply_su_state__d_d1, d1, x, xn, acc, FOM_STATE_DIM);
        axpy_op(fom_apply_su_state__d_g2, g2, x, xn, acc, FOM_STATE_DIM);
        axpy_op(fom_apply_su_state__d_m1, m1, x, xn, acc, FOM_STATE_DIM);
        fom_apply_su_t_t0(t0, xn);
        axpy_op(fom_apply_su_t_t0__d_d2, d2, t0, xn, acc, FOM_STATE_DIM);
        axpy_op(fom_apply_su_t_t0__d_g1, g1, t0, xn, acc, FOM_STATE_DIM);
        fom_apply_su_t_t1(t1, xn);
        axpy_op(fom_apply_su_t_t1__d_d2, d2, t1, xn, acc, FOM_STATE_DIM);
        axpy_op(fom_apply_su_t_t1__d_g1, g1, t1, xn, acc, FOM_STATE_DIM);
        axpy_op(fom_apply_su_t_t1__d_m2, m2, t1, xn, acc, FOM_STATE_DIM);

        if (it == max_iter - 1) {
            res = 0;
            for (i = 0; i < FOM_STATE_DIM; i++) {
                fom_float d = xn[i] - x[i];
                if (d < 0) d = -d;
                if (d > res) res = d;
            }
        }
        tmp = x; x = xn; xn = tmp;
    }

    if (x != work->state) memcpy(work->state, x, sizeof(work->state));
    memcpy(work->solution.x, work->state, sizeof(work->state));
    work->solution.iter = it;
    work->solution.residual = res;
    work->solution.status = FOM_MAX_ITER_REACHED;
    return work->solution.status;
}

void fom_cleanup(FomWorkspace *work) {
    (void)work;
}
