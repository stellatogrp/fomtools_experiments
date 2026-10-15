#include "fom_solver.h"
#include "fom_data.h"
#include "fom_ops.h"
#include "fom_linsolve.h"

#define N FOM_STATE_DIM
#define NT0 FOM_T0_DIM
#define NT1 FOM_T1_DIM

static inline void vzero(fom_float *x, fom_int n) {
    fom_int i;
    for (i = 0; i < n; ++i) x[i] = 0.0;
}

/* out += c * (M @ x), using `tmp` (length n) as scratch. Skipped when c == 0. */
static inline void scaled_apply(void (*apply)(const fom_float *, fom_float *),
                                fom_float c, const fom_float *x,
                                fom_float *tmp, fom_float *out, fom_int n) {
    fom_int i;
    if (c == 0.0) return;
    vzero(tmp, n);
    apply(x, tmp);
    for (i = 0; i < n; ++i) out[i] += c * tmp[i];
}

static void reset_state(FomWorkspace *work) {
    fom_int i;
    for (i = 0; i < N; ++i) work->state[i] = FOM_STATE_INIT[i];
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
    for (i = 0; i < FOM_PARAM_P1_DIM; ++i) work->params.p1[i] = 0.0;
    reset_state(work);
    work->solution.iter = 0;
    work->solution.residual = 0.0;
    work->solution.status = FOM_MAX_ITER_REACHED;
}

void fom_update_params(FomWorkspace *work, const FomParams *params) {
    work->params = *params;
    reset_state(work);
}

fom_int fom_solve(FomWorkspace *work) {
    fom_float *restrict s = work->state;
    fom_float *restrict ns = work->new_state;
    fom_float *restrict acc = work->acc;
    fom_float *restrict t0 = work->t0;
    fom_float *restrict t1 = work->t1;
    const fom_float *p0 = work->params.p0;
    const fom_float *p1 = work->params.p1;
    const fom_int max_iter = work->settings.max_iter;
    fom_float res = 0.0;
    fom_int it, i;
    fom_int status = FOM_MAX_ITER_REACHED;

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

        /* ---- t0 = relu(...) ; acc[0:48]=taux, [48:96]=tth, [96:144]=tau terms */
        vzero(t0, NT0);
        vzero(acc, 3 * NT0);
        fom_apply_t0_state(s, t0);
        fom_apply_t0_state__d_taux(s, acc);
        fom_apply_t0_state__d_tth(s, acc + NT0);
        fom_apply_t0_p_p1(p1, t0);
        fom_apply_t0_p_p1__d_tau(p1, acc + 2 * NT0);
        for (i = 0; i < NT0; ++i) {
            fom_float v = t0[i] + taux * acc[i] + tth * acc[NT0 + i] + tau * acc[2 * NT0 + i];
            t0[i] = v > 0.0 ? v : 0.0;
        }

        /* ---- t1 = clip(...) ; acc[0:16] = eta terms */
        vzero(t1, NT1);
        vzero(acc, NT1);
        fom_apply_t1_state(s, t1);
        fom_apply_t1_state__d_eta(s, acc);
        fom_apply_t1_p_p0(p0, t1);
        fom_apply_t1_p_p0__d_eta(p0, acc);
        fom_apply_t1_t_t0(t0, t1);
        fom_apply_t1_t_t0__d_eta(t0, acc);
        for (i = 0; i < NT1; ++i) {
            fom_float v = t1[i] + eta * acc[i];
            fom_float lo = FOM_MAT_t1_lo[i], hi = FOM_MAT_t1_hi[i];
            v = v < lo ? lo : v;
            t1[i] = v > hi ? hi : v;
        }

        /* ---- state update */
        vzero(ns, N);
        fom_apply_su_state(s, ns);
        scaled_apply(fom_apply_su_state__d_d1, d1, s, acc, ns, N);
        scaled_apply(fom_apply_su_state__d_g2, g2, s, acc, ns, N);
        scaled_apply(fom_apply_su_state__d_m1, m1, s, acc, ns, N);
        fom_apply_su_t_t0(t0, ns);
        fom_apply_su_t_t1(t1, ns);
        /* Scale the (short) sources instead of the 160-long outputs:
           c * (M @ x) == M @ (c * x). */
        {
            fom_float *d2t0 = acc;
            fom_float *g1t0 = acc + NT0;
            fom_float *d2t1 = acc + 2 * NT0;
            fom_float *g1t1 = d2t1 + NT1;
            fom_float *m2t1 = g1t1 + NT1;
            for (i = 0; i < NT0; ++i) {
                d2t0[i] = d2 * t0[i];
                g1t0[i] = g1 * t0[i];
            }
            for (i = 0; i < NT1; ++i) {
                d2t1[i] = d2 * t1[i];
                g1t1[i] = g1 * t1[i];
                m2t1[i] = m2 * t1[i];
            }
            fom_apply_su_t_t0__d_d2(d2t0, ns);
            fom_apply_su_t_t0__d_g1(g1t0, ns);
            fom_apply_su_t_t1__d_d2(d2t1, ns);
            fom_apply_su_t_t1__d_g1(g1t1, ns);
            fom_apply_su_t_t1__d_m2(m2t1, ns);
        }

        res = 0.0;
        for (i = 0; i < N; ++i) {
            fom_float d = ns[i] - s[i];
            d = d < 0.0 ? -d : d;
            if (d > res) res = d;
            s[i] = ns[i];
        }
#if FOM_USE_TOL
        if (work->settings.check_termination && res <= work->settings.tol) {
            ++it;
            status = FOM_SOLVED;
            break;
        }
#endif
    }

    for (i = 0; i < N; ++i) work->solution.x[i] = s[i];
    work->solution.iter = it;
    work->solution.residual = res;
    work->solution.status = status;
    return status;
}

void fom_cleanup(FomWorkspace *work) {
    (void)work;
}
