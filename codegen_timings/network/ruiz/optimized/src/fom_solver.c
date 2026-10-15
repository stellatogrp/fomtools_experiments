#include "fom_solver.h"
#include "fom_data.h"
#include "fom_ops.h"
#include "fom_linsolve.h"
#include "fom_profile.h"
#include <math.h>

/* Problem structure (verified against the CSR operators in fom_ops.c):
   state = [x (150) | y (10) | z (15)], x indexed i = NJ*g + j with
   g < NG = 10 (group / t1 row) and j < NJ = 15 (z / p0 index).
     t0_state : row i -> x_i (coef 1), y_g (ay_i), z_j (bz_i)
     t1_state : row g -> x_i for i in group g (P_i), y_g (ty_g)
     t1_t_t0  : row g -> t0_i (-2 P_i)
     su_state : row NX+NG+j -> x_i with i%NJ == j (R_i), z_j (sz_j); rows < NX+NG empty
     su_t_t0  : rows < NX identity; row NX+NG+j -> t0_i (-2 R_i)
     su_t_t1  : rows NX..NX+NG identity
     su_p_p0  : row NX+NG+j -> p_j (diagonal)
   So one iteration is a single fused pass over x:
     t_i  = clip(c_i + x_i + ay_i y_g + bz_i z_j)      -> new x_i
     w_i  = x_i - 2 t_i
     y_g' = relu(t1c_g + ty_g y_g + sum_j P_i w_i)
     z_j' = sz_j z_j + sum_g R_i w_i + (su_p_p0 p)_j
   The per-element coefficients are extracted once from the operators
   (by probing with unit vectors) into static read-only-after-setup tables. */

#define NX FOM_T0_DIM
#define NG FOM_T1_DIM
#define NJ FOM_PARAM_P0_DIM
#define OY NX
#define OZ (NX + NG)
#define NL 16              /* group width padded NJ -> NL for clean 2/4-wide SIMD */
#define NXP (NG * NL)

#if defined(__GNUC__) || defined(__clang__)
#define FOM_ALIGN __attribute__((aligned(64)))
#else
#define FOM_ALIGN
#endif

/* Padded per-element tables (lane j >= NJ is all zero, which keeps padded
   lanes of x and w identically zero: t = clip(0, 0, 0) = 0, w = 0 - 0). */
static FOM_ALIGN fom_float c_c[NXP], c_lo[NXP], c_hi[NXP];
static FOM_ALIGN fom_float c_ay[NXP], c_bz[NXP], c_P[NXP], c_R[NXP];
static fom_float c_t1c[NG], c_ty[NG], c_sz[NJ], c_pd[NJ];
/* Interleaved copy of the per-element tables: for each 4-lane chunk the
   fields are contiguous (F_* x 4 lanes) so a chunk's loads pair into ldp. */
#define CW 4
#define F_C 0
#define F_AY 1
#define F_BZ 2
#define F_LO 3
#define F_HI 4
#define F_P 5
#define F_R 6
#define NF 7
static FOM_ALIGN fom_float c_pack[NXP * NF];
static int c_ready = 0;
static int c_lo_zero = 0; /* t0_lo == 0 (box [0, hi]): specialised kernel */

static void fom_extract_coeffs(FomWorkspace *work) {
    fom_float *e = work->new_state; /* probe input (175) */
    fom_float *o = work->acc;       /* probe output (175) */
    fom_int i, k, g, j;

    for (i = 0; i < NXP; ++i) {
        c_c[i] = 0.0; c_lo[i] = 0.0; c_hi[i] = 0.0;
        c_ay[i] = 0.0; c_bz[i] = 0.0; c_P[i] = 0.0; c_R[i] = 0.0;
    }
    for (i = 0; i < FOM_STATE_DIM; ++i) e[i] = 0.0;

    for (g = 0, i = 0; g < NG; ++g)
        for (j = 0; j < NJ; ++j, ++i) {
            c_c[g * NL + j] = FOM_MAT_t0_c[i];
            c_lo[g * NL + j] = FOM_MAT_t0_lo[i];
            c_hi[g * NL + j] = FOM_MAT_t0_hi[i];
        }
    for (g = 0; g < NG; ++g) c_t1c[g] = FOM_MAT_t1_c[g];
    c_lo_zero = 1;
    for (i = 0; i < NX; ++i) c_lo_zero &= (FOM_MAT_t0_lo[i] == 0.0);

    /* y columns of t0_state / t1_state (each row hits exactly one y_g) */
    for (k = 0; k < NG; ++k) e[OY + k] = 1.0;
    for (i = 0; i < NX; ++i) o[i] = 0.0;
    fom_apply_t0_state(e, o);
    for (g = 0, i = 0; g < NG; ++g)
        for (j = 0; j < NJ; ++j, ++i) c_ay[g * NL + j] = o[i];
    for (i = 0; i < NG; ++i) o[i] = 0.0;
    fom_apply_t1_state(e, o);
    for (i = 0; i < NG; ++i) c_ty[i] = o[i];
    for (k = 0; k < NG; ++k) e[OY + k] = 0.0;

    /* z columns of t0_state / su_state (each row hits exactly one z_j) */
    for (k = 0; k < NJ; ++k) e[OZ + k] = 1.0;
    for (i = 0; i < NX; ++i) o[i] = 0.0;
    fom_apply_t0_state(e, o);
    for (g = 0, i = 0; g < NG; ++g)
        for (j = 0; j < NJ; ++j, ++i) c_bz[g * NL + j] = o[i];
    for (i = 0; i < FOM_STATE_DIM; ++i) o[i] = 0.0;
    fom_apply_su_state(e, o);
    for (j = 0; j < NJ; ++j) c_sz[j] = o[OZ + j];
    for (k = 0; k < NJ; ++k) e[OZ + k] = 0.0;

    /* su_p_p0 is diagonal into the z rows: row OZ + j hits only p_j */
    for (j = 0; j < NJ; ++j) e[j] = 1.0;
    for (i = 0; i < FOM_STATE_DIM; ++i) o[i] = 0.0;
    fom_apply_su_p_p0(e, o);
    for (j = 0; j < NJ; ++j) c_pd[j] = o[OZ + j];
    for (j = 0; j < NJ; ++j) e[j] = 0.0;

    /* x columns of t1_state (row g) / su_state (row OZ + j) */
    for (g = 0, k = 0; g < NG; ++g)
        for (j = 0; j < NJ; ++j, ++k) {
            e[k] = 1.0;
            for (i = 0; i < FOM_STATE_DIM; ++i) o[i] = 0.0;
            fom_apply_t1_state(e, o);
            c_P[g * NL + j] = o[g];
            for (i = 0; i < FOM_STATE_DIM; ++i) o[i] = 0.0;
            fom_apply_su_state(e, o);
            c_R[g * NL + j] = o[OZ + j];
            e[k] = 0.0;
        }
    for (i = 0; i < NXP; i += CW) {
        fom_float *t = c_pack + i * NF;
        for (j = 0; j < CW; ++j) {
            t[F_C * CW + j] = c_c[i + j];
            t[F_AY * CW + j] = c_ay[i + j];
            t[F_BZ * CW + j] = c_bz[i + j];
            t[F_LO * CW + j] = c_lo[i + j];
            t[F_HI * CW + j] = c_hi[i + j];
            t[F_P * CW + j] = c_P[i + j];
            t[F_R * CW + j] = c_R[i + j];
        }
    }
    c_ready = 1;
}

/* One fused iteration over the padded x. Per group g (NL lanes):
     t = clip(c + x + ay*y_g + bz*z, lo, hi);  x <- t;  w = x_old - 2t
     y_g' = relu(t1c_g + ty_g*y_g + P.w)       (lane-wise partial sums)
     zacc += R*w                               (lane-wise, across groups) */
static inline __attribute__((always_inline)) void
fom_iterate(fom_float *restrict X, fom_float *restrict yl, fom_float *restrict zl,
            const fom_float *restrict pz, const int lo_zero) {
    FOM_ALIGN fom_float zacc[NL];
    fom_float yn[NG];
    fom_int g, j, q, l;
    for (j = 0; j < NL; ++j) zacc[j] = 0.0;
    for (g = 0; g < NG; ++g) {
        const fom_int b = g * NL;
        const fom_float yg = yl[g];
        fom_float *restrict xg = X + b;
        FOM_ALIGN fom_float sp[NL];
        fom_float s;
        for (q = 0; q < NL; q += CW) {
            const fom_float *restrict t = c_pack + (b + q) * NF;
            for (l = 0; l < CW; ++l) {
                const fom_int jj = q + l;
                const fom_float xi = xg[jj];
                fom_float v = t[F_C * CW + l] + xi + t[F_AY * CW + l] * yg + t[F_BZ * CW + l] * zl[jj];
                fom_float w;
                /* single fmaxnm/fminnm; lo == 0 drops the lower-bound load */
                v = fmin(fmax(v, lo_zero ? 0.0 : t[F_LO * CW + l]), t[F_HI * CW + l]);
                xg[jj] = v;
                w = xi - 2.0 * v;
                sp[jj] = t[F_P * CW + l] * w;
                zacc[jj] += t[F_R * CW + l] * w;
            }
        }
        for (j = 0; j < NL / 2; ++j) sp[j] += sp[j + NL / 2];
        for (j = 0; j < NL / 4; ++j) sp[j] += sp[j + NL / 4];
        s = (sp[0] + sp[2]) + (sp[1] + sp[3]);
        s += c_t1c[g] + c_ty[g] * yg;
        yn[g] = s > 0 ? s : 0.0;
    }
    for (g = 0; g < NG; ++g) yl[g] = yn[g];
    for (j = 0; j < NJ; ++j) zl[j] = c_sz[j] * zl[j] + zacc[j] + pz[j];
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
    if (!c_ready) fom_extract_coeffs(work);
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

static void fom_unpack(const fom_float *X, const fom_float *yl, const fom_float *zl,
                       fom_float *out) {
    fom_int g, j, i;
    for (g = 0, i = 0; g < NG; ++g)
        for (j = 0; j < NJ; ++j, ++i) out[i] = X[g * NL + j];
    for (g = 0; g < NG; ++g) out[OY + g] = yl[g];
    for (j = 0; j < NJ; ++j) out[OZ + j] = zl[j];
}

fom_int fom_solve(FomWorkspace *work) {
    FOM_ALIGN fom_float X[NXP];   /* padded x (lanes >= NJ stay 0) */
    FOM_ALIGN fom_float zl[NL];
    fom_float yl[NG];
    fom_float *xo = work->new_state; /* previous state (residual only) */
    fom_float pz[NJ];                /* su_p_p0 @ p0 (z rows): constant across iterations */
    const fom_int max_iter = work->settings.max_iter;
    const fom_float tol = work->settings.tol;
    const fom_int use_tol = work->settings.check_termination && tol > 0;
    fom_float res = 0.0;
    fom_int status = FOM_MAX_ITER_REACHED;
    fom_int k, i, g, j;

    if (!c_ready) fom_extract_coeffs(work);

    FOM_PROF_BEGIN(setup);
    for (j = 0; j < NJ; ++j) pz[j] = c_pd[j] * work->params.p0[j];
    for (g = 0, i = 0; g < NG; ++g) {
        for (j = 0; j < NJ; ++j, ++i) X[g * NL + j] = work->state[i];
        for (; j < NL; ++j) X[g * NL + j] = 0.0;
    }
    for (g = 0; g < NG; ++g) yl[g] = work->state[OY + g];
    for (j = 0; j < NJ; ++j) zl[j] = work->state[OZ + j];
    for (; j < NL; ++j) zl[j] = 0.0;
    FOM_PROF_END(setup);

    for (k = 0; k < max_iter; ++k) {
        const fom_int need_res = use_tol || k == max_iter - 1;
        if (need_res) fom_unpack(X, yl, zl, xo);

        FOM_PROF_BEGIN(iteration);
        if (c_lo_zero) fom_iterate(X, yl, zl, pz, 1);
        else fom_iterate(X, yl, zl, pz, 0);
        FOM_PROF_END(iteration);

        if (need_res) {
            FOM_PROF_BEGIN(residual);
            fom_float *xs = work->solution.x;
            fom_unpack(X, yl, zl, xs);
            res = 0.0;
            for (i = 0; i < FOM_STATE_DIM; ++i) {
                fom_float d = xs[i] - xo[i];
                d = d < 0 ? -d : d;
                res = d > res ? d : res;
            }
            FOM_PROF_END(residual);
            if (use_tol && res <= tol) {
                status = FOM_SOLVED;
                ++k;
                break;
            }
        }
    }

    FOM_PROF_BEGIN(writeback);
    fom_unpack(X, yl, zl, work->solution.x);
    for (i = 0; i < FOM_STATE_DIM; ++i) work->state[i] = work->solution.x[i];
    FOM_PROF_END(writeback);
    work->solution.iter = k;
    work->solution.residual = res;
    work->solution.status = status;
    return status;
}

void fom_cleanup(FomWorkspace *work) {
    (void)work;
}
