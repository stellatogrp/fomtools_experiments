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

/* ---------------------------------------------------------------------------
   Structure of this instance (read off the CSR operators in fom_ops.c):
   the 51-dim state is laid out as
       x[0..19]  : primal block (solve variable t3 lives here)
       x[20]     : scalar multiplier of the budget constraint
       x[21..30] : z0 block   (t0 = clip(z0, lo, hi))
       x[31..40] : z1 block   (t1 = relu(p0 - z1))
       x[41..50] : z2 block   (t2 = relu(z2 - p0))
   Every operator except the linear solve is diagonal / identity-like on
   these 10-element blocks, so all the CSR products collapse into one fused
   elementwise pass over i = 0..9 (coefficients below). The t3 solve keeps
   using the provided LDL^T routine.
   ------------------------------------------------------------------------- */
#define FOM_N 10
#define FOM_RHO_X ((fom_float)0.0001)   /* t3_state diagonal on x[0..19] */

/* LDL^T factor of the t3 KKT matrix, copied from fom_linsolve.c.
   Structure: P = [reverse(0..9) | permutation of 10..19]; L is dense
   unit-lower on the leading 10x10 block and the identity elsewhere, so
   the trailing 10 unknowns decouple into a diagonal scaling. */
static const fom_float FOM_T3_L[9][9] = {
    {0.16602386285986637},
    {0.1341780839113472, 0.20010229092261247},
    {0.19035283657203825, 0.13961788722608143, 0.10607319372409005},
    {0.19731979604288968, 0.23156081052995087, 0.026196776411614279, 0.17481151880781065},
    {0.12428060484349022, 0.20732269499242095, 0.26728155587099989, 0.1920496053567449, 0.026330663837121408},
    {0.13881640214139926, 0.19649959588190527, 0.19301898666910555, 0.046672730010245522, 0.086531444185446241, 0.14541293585522147},
    {0.26772809482559118, 0.097658568538979973, 0.17615344937314389, 0.0018494703013880265, 0.069785667976103319, 0.065773614673741015, 0.14424866442650147},
    {0.17011953500487584, 0.23384534084520589, 0.10412453344239907, -0.019255719841898746, 0.12163494782190522, 0.067775281478193478, 0.18447607982944869, 0.13362647467187422},
    {0.24245841476321156, 0.18510867020838886, 0.060978969748262682, 0.24280365366585702, 0.17569516334309193, 0.086847146225085464, 0.014248553666431142, 0.041644092542608119, 0.010636217764220577},
};
static const fom_float FOM_T3_DINV_TOP[10] = {0.20182132481600745, 0.20462072486430363, 0.19841598641271913, 0.19157610727375632, 0.24292137690108545, 0.20003137001802496, 0.25223835315190285, 0.25567047906760942, 0.23240999548691052, 0.27036951599737441};
static const fom_float FOM_T3_DINV_BOT[10] = {0.49997500124993743, 0.49997500124993743, 0.49997500124993743, 0.49997500124993743, 0.49997500124993743, 0.49997500124993743, 0.49997500124993743, 0.49997500124993743, 0.49997500124993743, 0.49997500124993743};

/* t3 = M^{-1} acc via the LDL^T factor above (same arithmetic as
   fom_lin_solve_t3, specialized to its block structure and unrolled). */
static inline void fom_solve_t3(const fom_float *acc, fom_float *t3) {
    fom_float y[FOM_N];
    fom_int i, j;
    /* trailing block: L = I, so just the diagonal scaling */
    for (i = 0; i < FOM_N; ++i) t3[FOM_N + i] = acc[FOM_N + i] * FOM_T3_DINV_BOT[i];
    /* leading block: y = P b with P reversing 0..9. Column-oriented
       (right-looking) sweeps keep the dependency chain at one FMA per
       pivot instead of a serial dot product per row. */
    for (i = 0; i < FOM_N; ++i) y[i] = acc[FOM_N - 1 - i];
    for (j = 0; j < FOM_N - 1; ++j) {
        const fom_float yj = y[j];
        for (i = j + 1; i < FOM_N; ++i) y[i] -= FOM_T3_L[i - 1][j] * yj;
    }
    for (i = 0; i < FOM_N; ++i) y[i] *= FOM_T3_DINV_TOP[i];
    for (j = FOM_N - 1; j > 0; --j) {
        const fom_float yj = y[j];
        for (i = 0; i < j; ++i) y[i] -= FOM_T3_L[j - 1][i] * yj;
    }
    for (i = 0; i < FOM_N; ++i) t3[FOM_N - 1 - i] = y[i];
}

static void fom_compute_consts(const fom_float *p0, fom_float *c3, fom_float *cs) {
    fom_int i;
    /* Parameter-only contributions are loop-invariant: precompute once. */
    for (i = 0; i < FOM_T3_DIM; ++i) c3[i] = FOM_MAT_t3_c[i];
    for (i = 0; i < FOM_STATE_DIM; ++i) cs[i] = FOM_MAT_su_c[i];
    fom_apply_t3_p_p0(p0, c3);
    fom_apply_su_p_p0(p0, cs);
}

/* t0 = clip(z0), t1 = relu(p - z1), t2 = relu(z2 - p) and the t3
   right-hand side for block i, given the current state x. */
#define FOM_PROX_RHS(i)                                                      \
    do {                                                                     \
        const fom_float z0 = x[21 + (i)], z1 = x[31 + (i)], z2 = x[41 + (i)]; \
        fom_float a = z0, b = p[i] - z1, c = z2 - p[i];                      \
        a = a < lo[i] ? lo[i] : a;                                           \
        a = a > hi[i] ? hi[i] : a;                                           \
        b = b > 0.0 ? b : 0.0;                                               \
        c = c > 0.0 ? c : 0.0;                                               \
        t0[i] = a; t1[i] = b; t2[i] = c;                                     \
        acc[i] = c3[i] + FOM_RHO_X * x[i] - x20 - z0 - z1 - z2               \
                 + 2.0 * (a - b + c);                                        \
        acc[FOM_N + (i)] = c3[FOM_N + (i)] + FOM_RHO_X * x[FOM_N + (i)]      \
                           + z1 - z2 + 2.0 * (b + c);                        \
    } while (0)

fom_int fom_solve(FomWorkspace *work) {
    fom_float p[FOM_N];
    fom_float x[FOM_STATE_DIM];
    fom_float c3[FOM_T3_DIM], cs[FOM_STATE_DIM];
    fom_float acc[FOM_T3_DIM], t3[FOM_T3_DIM];
    fom_float t0[FOM_N], t1[FOM_N], t2[FOM_N];
    const fom_float *lo = FOM_MAT_t0_lo, *hi = FOM_MAT_t0_hi;
    const fom_int max_iter = work->settings.max_iter;
    const fom_float tol = work->settings.tol;
    const fom_int use_tol = work->settings.check_termination && tol > 0.0;
    fom_float res = 0.0, x20;
    fom_int status = FOM_MAX_ITER_REACHED;
    fom_int k, i;

    for (i = 0; i < FOM_N; ++i) p[i] = work->params.p0[i];
    for (i = 0; i < FOM_STATE_DIM; ++i) x[i] = work->state[i];
    fom_compute_consts(p, c3, cs);

    if (max_iter > 0) {
        x20 = x[20];
        FOM_PROF_BEGIN(prox_rhs);
        for (i = 0; i < FOM_N; ++i) FOM_PROX_RHS(i);
        FOM_PROF_END(prox_rhs);
    }

    for (k = 0; k < max_iter; ++k) {
        const fom_int want_res = use_tol || k == max_iter - 1;
        fom_float s = 0.0;

        FOM_PROF_BEGIN(linsolve);
        fom_solve_t3(acc, t3);
        FOM_PROF_END(linsolve);

        FOM_PROF_BEGIN(update_prox_rhs);
        for (i = 0; i < FOM_N; ++i) s += t3[i];
        if (want_res) {
            /* State update with residual max|x_new - x|. */
            fom_float xn[FOM_STATE_DIM];
            for (i = 0; i < FOM_T3_DIM; ++i) xn[i] = cs[i] + t3[i];
            xn[20] = cs[20] + x[20] + s;
            for (i = 0; i < FOM_N; ++i) {
                xn[21 + i] = cs[21 + i] + x[21 + i] - t0[i] + t3[i];
                xn[31 + i] = cs[31 + i] + x[31 + i] + t1[i] + t3[i] - t3[FOM_N + i];
                xn[41 + i] = cs[41 + i] + x[41 + i] - t2[i] + t3[i] + t3[FOM_N + i];
            }
            res = 0.0;
            for (i = 0; i < FOM_STATE_DIM; ++i) {
                fom_float d = xn[i] - x[i];
                d = d < 0.0 ? -d : d;
                res = d > res ? d : res;
                x[i] = xn[i];
            }
            if ((use_tol && res <= tol) || k == max_iter - 1) {
                if (use_tol && res <= tol) status = FOM_SOLVED;
                ++k;
                FOM_PROF_END(update_prox_rhs);
                break;
            }
            x20 = x[20];
            for (i = 0; i < FOM_N; ++i) FOM_PROX_RHS(i);
        } else {
            /* State update fused with the next iteration's prox/RHS pass:
               block i of the new state only feeds block i of t0/t1/t2/acc. */
            x20 = cs[20] + x[20] + s;
            x[20] = x20;
            for (i = 0; i < FOM_N; ++i) {
                const fom_float ta = t3[i], tb = t3[FOM_N + i];
                x[i] = cs[i] + ta;
                x[FOM_N + i] = cs[FOM_N + i] + tb;
                x[21 + i] = cs[21 + i] + x[21 + i] - t0[i] + ta;
                x[31 + i] = cs[31 + i] + x[31 + i] + t1[i] + ta - tb;
                x[41 + i] = cs[41 + i] + x[41 + i] - t2[i] + ta + tb;
                FOM_PROX_RHS(i);
            }
        }
        FOM_PROF_END(update_prox_rhs);
    }

    for (i = 0; i < FOM_STATE_DIM; ++i) {
        work->solution.x[i] = x[i];
        work->state[i] = x[i];
    }
    work->solution.iter = k;
    work->solution.residual = res;
    work->solution.status = status;
    return status;
}

void fom_cleanup(FomWorkspace *work) {
    (void)work;
}
