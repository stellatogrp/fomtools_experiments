#include <string.h>

#include "fom_solver.h"
#include "fom_data.h"
#include "fom_ops.h"
#include "fom_linsolve.h"

#define S_DIM FOM_STATE_DIM
#define T0_DIM FOM_T0_DIM
#define T1_DIM FOM_T1_DIM

static inline void zero(fom_float *v, int n) {
    memset(v, 0, (size_t)n * sizeof(fom_float));
}

/* out += c * v */
static inline void axpy(fom_float *restrict out, fom_float c,
                        const fom_float *restrict v, int n) {
    for (int i = 0; i < n; ++i) out[i] += c * v[i];
}

void fom_set_default_settings(FomSettings *settings) {
    settings->max_iter = FOM_MAX_ITER;
    settings->tol = FOM_TOL;
    settings->check_termination = FOM_CHECK_TERMINATION;
}

void fom_setup(FomWorkspace *work, const FomSettings *settings) {
    work->settings = *settings;
    memcpy(work->state, FOM_STATE_INIT, sizeof(work->state));
    zero(work->params.p0, FOM_PARAM_P0_DIM);
    zero(work->params.p1, FOM_PARAM_P1_DIM);
    work->solution.iter = 0;
    work->solution.residual = 0.0;
    work->solution.status = FOM_MAX_ITER_REACHED;
}

void fom_update_params(FomWorkspace *work, const FomParams *params) {
    work->params = *params;
    memcpy(work->state, FOM_STATE_INIT, sizeof(work->state));
}

fom_int fom_solve(FomWorkspace *work) {
    fom_float *restrict x = work->state;
    fom_float *restrict xn = work->new_state;
    fom_float *restrict acc = work->acc;
    fom_float *restrict t0 = work->t0;
    fom_float *restrict t1 = work->t1;
    const fom_float *p0 = work->params.p0;
    const fom_float *p1 = work->params.p1;
    const fom_int max_iter = work->settings.max_iter;
    const int use_tol = FOM_USE_TOL && work->settings.check_termination;
    const fom_float tol = work->settings.tol;

    fom_float res = 0.0;
    fom_int status = FOM_MAX_ITER_REACHED;
    fom_int it;

    memcpy(x, FOM_STATE_INIT, sizeof(work->state));

    for (it = 0; it < max_iter; ++it) {
        const fom_float d1 = fom_sched_d1(it);
        const fom_float d2 = fom_sched_d2(it);
        const fom_float eta = fom_sched_eta(it);
        const fom_float g1 = fom_sched_g1(it);
        const fom_float g2 = fom_sched_g2(it);
        const fom_float m1 = fom_sched_m1(it);
        const fom_float m2 = fom_sched_m2(it);
        const fom_float tau = fom_sched_tau(it);
        const fom_float taux = fom_sched_taux(it);
        const fom_float tth = fom_sched_tth(it);

        /* t0 = relu(...) */
        zero(t0, T0_DIM);
        fom_apply_t0_state(x, t0);
        fom_apply_t0_p_p1(p1, t0);
        if (taux != 0.0) {
            zero(acc, T0_DIM);
            fom_apply_t0_state__d_taux(x, acc);
            axpy(t0, taux, acc, T0_DIM);
        }
        if (tth != 0.0) {
            zero(acc, T0_DIM);
            fom_apply_t0_state__d_tth(x, acc);
            axpy(t0, tth, acc, T0_DIM);
        }
        if (tau != 0.0) {
            zero(acc, T0_DIM);
            fom_apply_t0_p_p1__d_tau(p1, acc);
            axpy(t0, tau, acc, T0_DIM);
        }
        for (int i = 0; i < T0_DIM; ++i) t0[i] = t0[i] > 0.0 ? t0[i] : 0.0;

        /* t1 = clip(...) */
        zero(t1, T1_DIM);
        fom_apply_t1_state(x, t1);
        fom_apply_t1_p_p0(p0, t1);
        fom_apply_t1_t_t0(t0, t1);
        if (eta != 0.0) {
            zero(acc, T1_DIM);
            fom_apply_t1_state__d_eta(x, acc);
            fom_apply_t1_p_p0__d_eta(p0, acc);
            fom_apply_t1_t_t0__d_eta(t0, acc);
            axpy(t1, eta, acc, T1_DIM);
        }
        for (int i = 0; i < T1_DIM; ++i) {
            fom_float v = t1[i];
            const fom_float lo = FOM_MAT_t1_lo[i];
            const fom_float hi = FOM_MAT_t1_hi[i];
            v = v < lo ? lo : v;
            v = v > hi ? hi : v;
            t1[i] = v;
        }

        /* state update */
        zero(xn, S_DIM);
        fom_apply_su_state(x, xn);
        fom_apply_su_t_t0(t0, xn);
        fom_apply_su_t_t1(t1, xn);
        if (d1 != 0.0) {
            zero(acc, S_DIM);
            fom_apply_su_state__d_d1(x, acc);
            axpy(xn, d1, acc, S_DIM);
        }
        if (g2 != 0.0) {
            zero(acc, S_DIM);
            fom_apply_su_state__d_g2(x, acc);
            axpy(xn, g2, acc, S_DIM);
        }
        if (m1 != 0.0) {
            zero(acc, S_DIM);
            fom_apply_su_state__d_m1(x, acc);
            axpy(xn, m1, acc, S_DIM);
        }
        if (d2 != 0.0) {
            zero(acc, S_DIM);
            fom_apply_su_t_t0__d_d2(t0, acc);
            fom_apply_su_t_t1__d_d2(t1, acc);
            axpy(xn, d2, acc, S_DIM);
        }
        if (g1 != 0.0) {
            zero(acc, S_DIM);
            fom_apply_su_t_t0__d_g1(t0, acc);
            fom_apply_su_t_t1__d_g1(t1, acc);
            axpy(xn, g1, acc, S_DIM);
        }
        if (m2 != 0.0) {
            zero(acc, S_DIM);
            fom_apply_su_t_t1__d_m2(t1, acc);
            axpy(xn, m2, acc, S_DIM);
        }

        /* residual + swap-in */
        res = 0.0;
        for (int i = 0; i < S_DIM; ++i) {
            fom_float d = xn[i] - x[i];
            d = d < 0.0 ? -d : d;
            res = d > res ? d : res;
            x[i] = xn[i];
        }

        if (use_tol && res <= tol) {
            ++it;
            status = FOM_SOLVED;
            break;
        }
    }

    memcpy(work->solution.x, x, sizeof(work->solution.x));
    work->solution.iter = it;
    work->solution.residual = res;
    work->solution.status = status;
    return status;
}

void fom_cleanup(FomWorkspace *work) {
    (void)work;
}
