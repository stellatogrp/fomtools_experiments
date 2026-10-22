#include "fom_ops.h"
#include "fom_data.h"
#if defined(__ARM_NEON) && defined(__aarch64__)
#include <arm_neon.h>
#endif

void fom_apply_t0_state(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 50; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t0_state_ROWPTR[i]; p < FOM_MAT_t0_state_ROWPTR[i + 1]; ++p) s += FOM_MAT_t0_state_VALS[p] * x[FOM_MAT_t0_state_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t0_state__d_gamma(const fom_float *x, fom_float *out) {
    fom_int i, j;
    for (i = 0; i < 50; ++i) {
        fom_float s = 0.0;
        for (j = 0; j < 100; ++j) s += FOM_MAT_t0_state__d_gamma[i * 100 + j] * x[j];
        out[i] += s;
    }
}

void fom_apply_t0_p_p0(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 50; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t0_p_p0_ROWPTR[i]; p < FOM_MAT_t0_p_p0_ROWPTR[i + 1]; ++p) s += FOM_MAT_t0_p_p0_VALS[p] * x[FOM_MAT_t0_p_p0_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_su_state(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 100; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_su_state_ROWPTR[i]; p < FOM_MAT_su_state_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_state_VALS[p] * x[FOM_MAT_su_state_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_su_t_t0(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 100; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_su_t_t0_ROWPTR[i]; p < FOM_MAT_su_t_t0_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t0_VALS[p] * x[FOM_MAT_su_t_t0_COLIDX[p]];
        out[i] += s;
    }
}

void fom_fused_t0(const fom_float *state, fom_float sc_gamma, fom_float *out) {
    fom_int i, p, j;
    for (i = 0; i < 50; ++i) {
        fom_float s;
        s = 0.0;
        for (p = FOM_MAT_t0_state_ROWPTR[i]; p < FOM_MAT_t0_state_ROWPTR[i + 1]; ++p) s += FOM_MAT_t0_state_VALS[p] * state[FOM_MAT_t0_state_COLIDX[p]];
        out[i] += s;
        s = 0.0;
        for (j = 0; j < 100; ++j) s += FOM_MAT_t0_state__d_gamma[i * 100 + j] * state[j];
        out[i] += sc_gamma * s;
    }
}

void fom_fused_su(const fom_float *state, const fom_float *t_t0, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 100; ++i) {
        fom_float s;
        s = 0.0;
        for (p = FOM_MAT_su_state_ROWPTR[i]; p < FOM_MAT_su_state_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_state_VALS[p] * state[FOM_MAT_su_state_COLIDX[p]];
        out[i] += s;
        s = 0.0;
        for (p = FOM_MAT_su_t_t0_ROWPTR[i]; p < FOM_MAT_su_t_t0_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t0_VALS[p] * t_t0[FOM_MAT_su_t_t0_COLIDX[p]];
        out[i] += s;
    }
}

/* Dense 100-wide row dot with 8 independent lanes (vectorizes, breaks the
   FMA latency chain). */
static fom_float dense_row_dot(const fom_float *a, const fom_float *x) {
    fom_float s0 = 0.0, s1 = 0.0, s2 = 0.0, s3 = 0.0, s4 = 0.0, s5 = 0.0, s6 = 0.0, s7 = 0.0;
    fom_int j;
    for (j = 0; j < 96; j += 8) {
        s0 += a[j] * x[j];         s1 += a[j + 1] * x[j + 1];
        s2 += a[j + 2] * x[j + 2]; s3 += a[j + 3] * x[j + 3];
        s4 += a[j + 4] * x[j + 4]; s5 += a[j + 5] * x[j + 5];
        s6 += a[j + 6] * x[j + 6]; s7 += a[j + 7] * x[j + 7];
    }
    s0 += a[96] * x[96]; s1 += a[97] * x[97]; s2 += a[98] * x[98]; s3 += a[99] * x[99];
    return ((s0 + s4) + (s2 + s6)) + ((s1 + s5) + (s3 + s7));
}

void fom_step_t0(const fom_float *state, fom_float sc_gamma, const fom_float *bias, const FomAct_t0 *act, fom_float *out) {
    const fom_float lam = act->lambd;
    fom_int i, j;
    /* FISTA structure: t0_state = [0, W] and d_gamma = [-W, W] (checked
       to round-off in the data), so A x + g D x = W (x1 + g (x1 - x0)):
       one 50x50 product on the momentum point instead of 50x150. */
    {
        static fom_float y[50];
        const fom_float *W = FOM_MAT_t0_state__d_gamma + 50;
        for (j = 0; j < 50; ++j) y[j] = state[50 + j] + sc_gamma * (state[50 + j] - state[j]);
#if defined(__ARM_NEON) && defined(__aarch64__)
        if (sizeof(fom_float) == sizeof(double)) {
            /* Column-major packed copy of W (built once from the matrix
               data): the product becomes axpys into register-resident
               output accumulators, no horizontal reductions. */
            static double WT[50 * 50];
            static int packed = 0;
            float64x2_t acc[13];
            fom_int h, r;
            if (!packed) {
                for (i = 0; i < 50; ++i)
                    for (j = 0; j < 50; ++j) WT[j * 50 + i] = (double)W[i * 100 + j];
                packed = 1;
            }
            for (h = 0; h < 2; ++h) {
                const fom_int r0 = h ? 24 : 0, nr = h ? 13 : 12;
                for (r = 0; r < nr; ++r) acc[r] = vld1q_f64((const double *)bias + r0 + 2 * r);
                for (j = 0; j < 50; ++j) {
                    const double *c = WT + j * 50 + r0;
                    const double yj = (double)y[j];
                    for (r = 0; r < nr; ++r) acc[r] = vfmaq_n_f64(acc[r], vld1q_f64(c + 2 * r), yj);
                }
                for (r = 0; r < nr; ++r) {
                    const float64x2_t v = acc[r];
                    const float64x2_t l = vdupq_n_f64(lam);
                    const float64x2_t hi = vsubq_f64(v, l), lo = vaddq_f64(v, l);
                    float64x2_t o = vbslq_f64(vcgtq_f64(v, l), hi, vbslq_f64(vcltq_f64(v, vnegq_f64(l)), lo, vdupq_n_f64(0.0)));
                    vst1q_f64((double *)out + r0 + 2 * r, o);
                }
            }
            return;
        }
#endif
        for (i = 0; i < 50; ++i) {
            const fom_float *d = W + i * 100;
            fom_float s0 = 0.0, s1 = 0.0, v;
            for (j = 0; j < 50; j += 2) { s0 += d[j] * y[j]; s1 += d[j + 1] * y[j + 1]; }
            v = bias[i] + (s0 + s1);
            v = (v > lam) ? (v - lam) : ((v < -lam) ? (v + lam) : 0.0);
            out[i] = v;
        }
    }
}

void fom_step_su(const fom_float *state, const fom_float *t_t0, const fom_float *bias, fom_float *out) {
    fom_int i, r, p;
    for (r = 0; r < 100; ++r) {
        fom_float s, v;
        i = r;
        v = bias[i];
        s = 0.0;
        for (p = FOM_MAT_su_state_ROWPTR[i]; p < FOM_MAT_su_state_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_state_VALS[p] * state[FOM_MAT_su_state_COLIDX[p]];
        v += s;
        s = 0.0;
        for (p = FOM_MAT_su_t_t0_ROWPTR[i]; p < FOM_MAT_su_t_t0_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t0_VALS[p] * t_t0[FOM_MAT_su_t_t0_COLIDX[p]];
        v += s;
        out[i] = v;
    }
}

void fom_iterate(const fom_float *state, fom_float sc_gamma, const fom_float *bias_t0, const fom_float *bias_su, const FomAct_t0 *act_t0, fom_float *t_t0, fom_float *new_state) {
    fom_step_t0(state, sc_gamma, bias_t0, act_t0, t_t0);
    fom_step_su(state, t_t0, bias_su, new_state);
}
