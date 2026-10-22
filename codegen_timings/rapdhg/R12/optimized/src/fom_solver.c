#include <string.h>

#include "fom_solver.h"
#include "fom_data.h"
#include "fom_ops.h"
#include "fom_linsolve.h"
#include "fom_profile.h"

#define S_DIM FOM_STATE_DIM
#define T0_DIM FOM_T0_DIM
#define T1_DIM FOM_T1_DIM

/* The state update has no constant term. */
static const fom_float ZERO_SU[FOM_STATE_DIM] = {0.0};

void fom_set_default_settings(FomSettings *settings) {
    settings->max_iter = FOM_MAX_ITER;
    settings->tol = FOM_TOL;
    settings->check_termination = FOM_CHECK_TERMINATION;
}

void fom_setup(FomWorkspace *work, const FomSettings *settings) {
    work->settings = *settings;
    memcpy(work->state, FOM_STATE_INIT, sizeof(work->state));
    memset(work->params.p0, 0, sizeof(work->params.p0));
    memset(work->params.p1, 0, sizeof(work->params.p1));
    work->solution.iter = 0;
    work->solution.residual = 0.0;
    work->solution.status = FOM_MAX_ITER_REACHED;
}

void fom_update_params(FomWorkspace *work, const FomParams *params) {
    work->params = *params;
    memcpy(work->state, FOM_STATE_INIT, sizeof(work->state));
}

fom_int fom_solve(FomWorkspace *work) {
    fom_float *x = work->state;
    fom_float *xn = work->new_state;
    fom_float *restrict t0 = work->t0;
    fom_float *restrict t1 = work->t1;
    /* Hoisted parameter terms live in solution.x until the final copy. */
    fom_float *restrict b0 = work->solution.x;        /* t0_p_p1 @ p1 */
    fom_float *restrict q0 = b0 + T0_DIM;             /* t0_p_p1__d_tau @ p1 */
    fom_float *restrict b1 = q0 + T0_DIM;             /* t1_p_p0 @ p0 */
    fom_float *restrict q1 = b1 + T1_DIM;             /* t1_p_p0__d_eta @ p0 */
    fom_float *restrict bias0 = work->acc;
    fom_float *restrict bias1 = work->acc + T0_DIM;
    const fom_int max_iter = work->settings.max_iter;
    fom_float res = 0.0;
    fom_int it, i;

    memcpy(x, FOM_STATE_INIT, sizeof(work->state));
    memset(b0, 0, (size_t)(2 * T0_DIM + 2 * T1_DIM) * sizeof(fom_float));
    /* t0_p_p1 and t1_p_p0 are empty (nnz 0): their terms vanish. */
    fom_apply_t0_p_p1__d_tau(work->params.p1, q0);
    fom_apply_t1_p_p0__d_eta(work->params.p0, q1);

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
        fom_float *tmp;

        FOM_PROF_BEGIN(bias);
        for (i = 0; i < T0_DIM; ++i) bias0[i] = tau * q0[i];
        for (i = 0; i < T1_DIM; ++i) bias1[i] = eta * q1[i];
        FOM_PROF_END(bias);

        FOM_PROF_BEGIN(iterate);
        fom_iterate(x, taux, tth, eta, d1, g2, m1, d2, g1, m2, bias0, bias1, ZERO_SU,
                    &FOM_ACT_t0, &FOM_ACT_t1, t0, t1, xn);
        FOM_PROF_END(iterate);

        if (it == max_iter - 1) {
            res = 0.0;
            for (i = 0; i < S_DIM; ++i) {
                fom_float d = xn[i] - x[i];
                d = d < 0.0 ? -d : d;
                if (d > res) res = d;
            }
        }
        tmp = x; x = xn; xn = tmp;
    }

    if (x != work->state) memcpy(work->state, x, sizeof(work->state));
    memcpy(work->solution.x, work->state, sizeof(work->solution.x));
    work->solution.iter = it;
    work->solution.residual = res;
    work->solution.status = FOM_MAX_ITER_REACHED;
    return FOM_MAX_ITER_REACHED;
}

void fom_cleanup(FomWorkspace *work) {
    (void)work;
}
