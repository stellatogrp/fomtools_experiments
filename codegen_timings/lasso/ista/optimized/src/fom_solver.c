#include "fom_solver.h"
#include "fom_data.h"
#include "fom_ops.h"
#include "fom_linsolve.h"
#include "fom_profile.h"

#if defined(__aarch64__) && defined(__ARM_NEON)
#include <arm_neon.h>
#define FOM_USE_NEON 1
#else
#define FOM_USE_NEON 0
#endif

static void fom_reset_state(FomWorkspace *work) {
    fom_int i;
    for (i = 0; i < FOM_STATE_DIM; ++i) work->state[i] = FOM_STATE_INIT[i];
}

void fom_set_default_settings(FomSettings *settings) {
    settings->max_iter = FOM_MAX_ITER;
    settings->tol = FOM_TOL;
    settings->check_termination = FOM_CHECK_TERMINATION;
}

/* Zero-padded copy of t0_state (rows/cols 15 -> 16) so every inner loop is
   an exact multiple of the SIMD width; built once in fom_setup. */
#define FOM_PAD 16
static fom_float fom_Mpad[FOM_PAD * FOM_PAD];

static void fom_build_padded(void) {
    fom_int i, j;
    for (i = 0; i < FOM_PAD * FOM_PAD; ++i) fom_Mpad[i] = 0.0;
    for (i = 0; i < FOM_STATE_DIM; ++i)
        for (j = 0; j < FOM_STATE_DIM; ++j)
            fom_Mpad[i * FOM_PAD + j] = FOM_MAT_t0_state[i * FOM_STATE_DIM + j];
}

void fom_setup(FomWorkspace *work, const FomSettings *settings) {
    fom_int i;
    work->settings = *settings;
    fom_build_padded();
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
    const fom_float nlam = -FOM_T0_LAMBDA;
    const fom_int max_iter = work->settings.max_iter;
    const fom_float tol = work->settings.tol;
    const fom_int check = work->settings.check_termination;
    const fom_int use_tol = (tol > 0.0) && (check > 0);
    const fom_float *M = fom_Mpad;
    fom_float a[FOM_PAD];   /* loop-invariant t0_p_p0 @ p0 */
    fom_float x[FOM_PAD];   /* current state (= t0, since su_t_t0 = I) */
#if !FOM_USE_NEON
    fom_float y[FOM_PAD];   /* pre-activation */
#endif
    fom_float res = 0.0;
    fom_int status = FOM_MAX_ITER_REACHED;
    fom_int k, i, j, ctr = 0;

    /* Parameter contribution is loop-invariant: a = t0_p_p0 @ p0. */
    FOM_PROF_BEGIN(param_term);
    {
        const fom_float *P = FOM_MAT_t0_p_p0;
        const fom_float *p0 = work->params.p0;
        for (i = 0; i < FOM_T0_DIM; ++i) {
            fom_float s = 0.0;
            for (j = 0; j < FOM_PARAM_P0_DIM; ++j) s += P[i * FOM_PARAM_P0_DIM + j] * p0[j];
            a[i] = s;
        }
        a[FOM_PAD - 1] = 0.0;
        for (i = 0; i < FOM_STATE_DIM; ++i) x[i] = work->state[i];
        x[FOM_PAD - 1] = 0.0;
    }
    FOM_PROF_END(param_term);

    FOM_PROF_BEGIN(iterations);
#if FOM_USE_NEON
    {
        /* 2-lane f64 kernel: y,z,w are 8 vector registers each; column j of
           the (symmetric) matrix is multiplied by lane j&1 of xv[j>>1]. */
        const float64x2_t vlam = vdupq_n_f64(lam);
        const float64x2_t vnlam = vdupq_n_f64(nlam);
        float64x2_t av[FOM_PAD / 2], xv[FOM_PAD / 2];
        for (i = 0; i < FOM_PAD / 2; ++i) {
            av[i] = vld1q_f64(a + 2 * i);
            xv[i] = vld1q_f64(x + 2 * i);
        }
#define FOM_COL(dst, j) do { \
            const fom_float *r_ = M + (j) * FOM_PAD; \
            const float64x2_t xb_ = xv[(j) >> 1]; \
            for (i = 0; i < FOM_PAD / 2; ++i) \
                dst[i] = vfmaq_laneq_f64(dst[i], vld1q_f64(r_ + 2 * i), xb_, (j) & 1); \
        } while (0)
        for (k = 0; k < max_iter; ++k) {
            float64x2_t yv[FOM_PAD / 2], zv[FOM_PAD / 2], wv[FOM_PAD / 2];
            for (i = 0; i < FOM_PAD / 2; ++i) {
                yv[i] = av[i];
                zv[i] = vdupq_n_f64(0.0);
                wv[i] = vdupq_n_f64(0.0);
            }
            FOM_COL(yv, 0);  FOM_COL(zv, 1);  FOM_COL(wv, 2);
            FOM_COL(yv, 3);  FOM_COL(zv, 4);  FOM_COL(wv, 5);
            FOM_COL(yv, 6);  FOM_COL(zv, 7);  FOM_COL(wv, 8);
            FOM_COL(yv, 9);  FOM_COL(zv, 10); FOM_COL(wv, 11);
            FOM_COL(yv, 12); FOM_COL(zv, 13); FOM_COL(wv, 14);
            if (use_tol || k == max_iter - 1) {
                float64x2_t rv = vdupq_n_f64(0.0);
                for (i = 0; i < FOM_PAD / 2; ++i) {
                    float64x2_t v = vaddq_f64(yv[i], vaddq_f64(zv[i], wv[i]));
                    /* soft_threshold(v) = v - clamp(v, -lam, lam) */
                    v = vsubq_f64(v, vminq_f64(vmaxq_f64(v, vnlam), vlam));
                    rv = vmaxq_f64(rv, vabdq_f64(v, xv[i]));
                    xv[i] = v;
                }
                res = vmaxvq_f64(rv);
            } else {
                for (i = 0; i < FOM_PAD / 2; ++i) {
                    float64x2_t v = vaddq_f64(yv[i], vaddq_f64(zv[i], wv[i]));
                    xv[i] = vsubq_f64(v, vminq_f64(vmaxq_f64(v, vnlam), vlam));
                }
            }
            if (use_tol && ++ctr >= check) {
                ctr = 0;
                if (res > tol) continue;
                ++k;
                status = FOM_SOLVED;
                break;
            }
        }
#undef FOM_COL
        for (i = 0; i < FOM_PAD / 2; ++i) vst1q_f64(x + 2 * i, xv[i]);
    }
#else
    for (k = 0; k < max_iter; ++k) {
        /* y = a + t0_state @ x as column axpys; t0_state is symmetric, so
           column j is the contiguous row j. Padding lane stays 0. */
        {
            /* Three independent partial sums (15 = 3 x 5 columns) cut the
               serial FMA dependency chain per lane from 15 to 5. */
            fom_float z[FOM_PAD], w[FOM_PAD];
            for (i = 0; i < FOM_PAD; ++i) {
                y[i] = a[i] + M[i] * x[0];
                z[i] = M[FOM_PAD + i] * x[1];
                w[i] = M[2 * FOM_PAD + i] * x[2];
            }
            for (j = 3; j < FOM_STATE_DIM; j += 3) {
                const fom_float x0 = x[j], x1 = x[j + 1], x2 = x[j + 2];
                const fom_float *r0 = M + j * FOM_PAD;
                const fom_float *r1 = r0 + FOM_PAD;
                const fom_float *r2 = r1 + FOM_PAD;
                for (i = 0; i < FOM_PAD; ++i) {
                    y[i] += r0[i] * x0;
                    z[i] += r1[i] * x1;
                    w[i] += r2[i] * x2;
                }
            }
            for (i = 0; i < FOM_PAD; ++i) y[i] = y[i] + (z[i] + w[i]);
        }

        /* x_new = soft_threshold(y, lam); the state update is identity. */
        if (use_tol || k == max_iter - 1) {
            res = 0.0;
            for (i = 0; i < FOM_STATE_DIM; ++i) {
                fom_float v = y[i];
                v = v > lam ? v - lam : (v < nlam ? v + lam : 0.0);
                fom_float d = v - x[i];
                if (d < 0) d = -d;
                if (d > res) res = d;
            }
        }
        for (i = 0; i < FOM_PAD; ++i) {
            fom_float v = y[i];
            x[i] = v > lam ? v - lam : (v < nlam ? v + lam : 0.0);
        }
        if (use_tol && ++ctr >= check) {
            ctr = 0;
            if (res > tol) continue;
            ++k;
            status = FOM_SOLVED;
            break;
        }
    }
#endif
    FOM_PROF_END(iterations);

    for (i = 0; i < FOM_STATE_DIM; ++i) {
        work->state[i] = x[i];
        work->solution.x[i] = x[i];
    }
    work->solution.iter = k;
    work->solution.residual = res;
    work->solution.status = status;
    return status;
}

void fom_cleanup(FomWorkspace *work) {
    (void)work;
}
