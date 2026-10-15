# Optimization report

## Timing

```
translated (emitted)     108.3 us/solve
optimized                 13.7 us/solve
speedup                  7.88x
```

Optimization outcome: **kept (re-verified correct)**.

## What the agent changed

```diff
--- translated (emitted)
+++ optimized
@@ -3,6 +3,178 @@
 #include "fom_ops.h"
 #include "fom_linsolve.h"
 #include "fom_profile.h"
+#include <math.h>
+
+/* Problem structure (verified against the CSR operators in fom_ops.c):
+   state = [x (150) | y (10) | z (15)], x indexed i = NJ*g + j with
+   g < NG = 10 (group / t1 row) and j < NJ = 15 (z / p0 index).
+     t0_state : row i -> x_i (coef 1), y_g (ay_i), z_j (bz_i)
+     t1_state : row g -> x_i for i in group g (P_i), y_g (ty_g)
+     t1_t_t0  : row g -> t0_i (-2 P_i)
+     su_state : row NX+NG+j -> x_i with i%NJ == j (R_i), z_j (sz_j); rows < NX+NG empty
+     su_t_t0  : rows < NX identity; row NX+NG+j -> t0_i (-2 R_i)
+     su_t_t1  : rows NX..NX+NG identity
+     su_p_p0  : row NX+NG+j -> p_j (diagonal)
+   So one iteration is a single fused pass over x:
+     t_i  = clip(c_i + x_i + ay_i y_g + bz_i z_j)      -> new x_i
+     w_i  = x_i - 2 t_i
+     y_g' = relu(t1c_g + ty_g y_g + sum_j P_i w_i)
+     z_j' = sz_j z_j + sum_g R_i w_i + (su_p_p0 p)_j
+   The per-element coefficients are extracted once from the operators
+   (by probing with unit vectors) into static read-only-after-setup tables. */
+
+#define NX FOM_T0_DIM
+#define NG FOM_T1_DIM
+#define NJ FOM_PARAM_P0_DIM
+#define OY NX
+#define OZ (NX + NG)
+#define NL 16              /* group width padded NJ -> NL for clean 2/4-wide SIMD */
+#define NXP (NG * NL)
+
+#if defined(__GNUC__) || defined(__clang__)
+#define FOM_ALIGN __attribute__((aligned(64)))
+#else
+#define FOM_ALIGN
+#endif
+
+/* Padded per-element tables (lane j >= NJ is all zero, which keeps padded
+   lanes of x and w identically zero: t = clip(0, 0, 0) = 0, w = 0 - 0). */
+static FOM_ALIGN fom_float c_c[NXP], c_lo[NXP], c_hi[NXP];
+static FOM_ALIGN fom_float c_ay[NXP], c_bz[NXP], c_P[NXP], c_R[NXP];
+static fom_float c_t1c[NG], c_ty[NG], c_sz[NJ], c_pd[NJ];
+/* Interleaved copy of the per-element tables: for each 4-lane chunk the
+   fields are contiguous (F_* x 4 lanes) so a chunk's loads pair into ldp. */
+#define CW 4
+#define F_C 0
+#define F_AY 1
+#define F_BZ 2
+#define F_LO 3
+#define F_HI 4
+#define F_P 5
+#define F_R 6
+#define NF 7
+static FOM_ALIGN fom_float c_pack[NXP * NF];
+static int c_ready = 0;
+static int c_lo_zero = 0; /* t0_lo == 0 (box [0, hi]): specialised kernel */
+
+static void fom_extract_coeffs(FomWorkspace *work) {
+    fom_float *e = work->new_state; /* probe input (175) */
+    fom_float *o = work->acc;       /* probe output (175) */
+    fom_int i, k, g, j;
+
+    for (i = 0; i < NXP; ++i) {
+        c_c[i] = 0.0; c_lo[i] = 0.0; c_hi[i] = 0.0;
+        c_ay[i] = 0.0; c_bz[i] = 0.0; c_P[i] = 0.0; c_R[i] = 0.0;
+    }
+    for (i = 0; i < FOM_STATE_DIM; ++i) e[i] = 0.0;
+
+    for (g = 0, i = 0; g < NG; ++g)
+        for (j = 0; j < NJ; ++j, ++i) {
+            c_c[g * NL + j] = FOM_MAT_t0_c[i];
+            c_lo[g * NL + j] = FOM_MAT_t0_lo[i];
+            c_hi[g * NL + j] = FOM_MAT_t0_hi[i];
+        }
+    for (g = 0; g < NG; ++g) c_t1c[g] = FOM_MAT_t1_c[g];
+    c_lo_zero = 1;
+    for (i = 0; i < NX; ++i) c_lo_zero &= (FOM_MAT_t0_lo[i] == 0.0);
+
+    /* y columns of t0_state / t1_state (each row hits exactly one y_g) */
+    for (k = 0; k < NG; ++k) e[OY + k] = 1.0;
+    for (i = 0; i < NX; ++i) o[i] = 0.0;
+    fom_apply_t0_state(e, o);
+    for (g = 0, i = 0; g < NG; ++g)
+        for (j = 0; j < NJ; ++j, ++i) c_ay[g * NL + j] = o[i];
+    for (i = 0; i < NG; ++i) o[i] = 0.0;
+    fom_apply_t1_state(e, o);
+    for (i = 0; i < NG; ++i) c_ty[i] = o[i];
+    for (k = 0; k < NG; ++k) e[OY + k] = 0.0;
+
+    /* z columns of t0_state / su_state (each row hits exactly one z_j) */
+    for (k = 0; k < NJ; ++k) e[OZ + k] = 1.0;
+    for (i = 0; i < NX; ++i) o[i] = 0.0;
+    fom_apply_t0_state(e, o);
+    for (g = 0, i = 0; g < NG; ++g)
+        for (j = 0; j < NJ; ++j, ++i) c_bz[g * NL + j] = o[i];
+    for (i = 0; i < FOM_STATE_DIM; ++i) o[i] = 0.0;
+    fom_apply_su_state(e, o);
+    for (j = 0; j < NJ; ++j) c_sz[j] = o[OZ + j];
+    for (k = 0; k < NJ; ++k) e[OZ + k] = 0.0;
+
+    /* su_p_p0 is diagonal into the z rows: row OZ + j hits only p_j */
+    for (j = 0; j < NJ; ++j) e[j] = 1.0;
+    for (i = 0; i < FOM_STATE_DIM; ++i) o[i] = 0.0;
+    fom_apply_su_p_p0(e, o);
+    for (j = 0; j < NJ; ++j) c_pd[j] = o[OZ + j];
+    for (j = 0; j < NJ; ++j) e[j] = 0.0;
+
+    /* x columns of t1_state (row g) / su_state (row OZ + j) */
+    for (g = 0, k = 0; g < NG; ++g)
+        for (j = 0; j < NJ; ++j, ++k) {
+            e[k] = 1.0;
+            for (i = 0; i < FOM_STATE_DIM; ++i) o[i] = 0.0;
+            fom_apply_t1_state(e, o);
+            c_P[g * NL + j] = o[g];
+            for (i = 0; i < FOM_STATE_DIM; ++i) o[i] = 0.0;
+            fom_apply_su_state(e, o);
+            c_R[g * NL + j] = o[OZ + j];
+            e[k] = 0.0;
+        }
+    for (i = 0; i < NXP; i += CW) {
+        fom_float *t = c_pack + i * NF;
+        for (j = 0; j < CW; ++j) {
+            t[F_C * CW + j] = c_c[i + j];
+            t[F_AY * CW + j] = c_ay[i + j];
+            t[F_BZ * CW + j] = c_bz[i + j];
+            t[F_LO * CW + j] = c_lo[i + j];
+            t[F_HI * CW + j] = c_hi[i + j];
+            t[F_P * CW + j] = c_P[i + j];
+            t[F_R * CW + j] = c_R[i + j];
+        }
+    }
+    c_ready = 1;
+}
+
+/* One fused iteration over the padded x. Per group g (NL lanes):
+     t = clip(c + x + ay*y_g + bz*z, lo, hi);  x <- t;  w = x_old - 2t
+     y_g' = relu(t1c_g + ty_g*y_g + P.w)       (lane-wise partial sums)
+     zacc += R*w                               (lane-wise, across groups) */
+static inline __attribute__((always_inline)) void
+fom_iterate(fom_float *restrict X, fom_float *restrict yl, fom_float *restrict zl,
+            const fom_float *restrict pz, const int lo_zero) {
+    FOM_ALIGN fom_float zacc[NL];
+    fom_float yn[NG];
+    fom_int g, j, q, l;
+    for (j = 0; j < NL; ++j) zacc[j] = 0.0;
+    for (g = 0; g < NG; ++g) {
+        const fom_int b = g * NL;
+        const fom_float yg = yl[g];
+        fom_float *restrict xg = X + b;
+        FOM_ALIGN fom_float sp[NL];
+        fom_float s;
+        for (q = 0; q < NL; q += CW) {
+            const fom_float *restrict t = c_pack + (b + q) * NF;
+            for (l = 0; l < CW; ++l) {
+                const fom_int jj = q + l;
+                const fom_float xi = xg[jj];
+                fom_float v = t[F_C * CW + l] + xi + t[F_AY * CW + l] * yg + t[F_BZ * CW + l] * zl[jj];
+                fom_float w;
+                /* single fmaxnm/fminnm; lo == 0 drops the lower-bound load */
+                v = fmin(fmax(v, lo_zero ? 0.0 : t[F_LO * CW + l]), t[F_HI * CW + l]);
+                xg[jj] = v;
+                w = xi - 2.0 * v;
+                sp[jj] = t[F_P * CW + l] * w;
+                zacc[jj] += t[F_R * CW + l] * w;
+            }
+        }
+        for (j = 0; j < NL / 2; ++j) sp[j] += sp[j + NL / 2];
+        for (j = 0; j < NL / 4; ++j) sp[j] += sp[j + NL / 4];
+        s = (sp[0] + sp[2]) + (sp[1] + sp[3]);
+        s += c_t1c[g] + c_ty[g] * yg;
+        yn[g] = s > 0 ? s : 0.0;
+    }
+    for (g = 0; g < NG; ++g) yl[g] = yn[g];
+    for (j = 0; j < NJ; ++j) zl[j] = c_sz[j] * zl[j] + zacc[j] + pz[j];
+}
 
 static void fom_reset_state(FomWorkspace *work) {
     fom_int i;
@@ -18,6 +190,7 @@
 void fom_setup(FomWorkspace *work, const FomSettings *settings) {
     fom_int i;
     work->settings = *settings;
+    if (!c_ready) fom_extract_coeffs(work);
     for (i = 0; i < FOM_PARAM_P0_DIM; ++i) work->params.p0[i] = 0.0;
     fom_reset_state(work);
     work->solution.iter = 0;
@@ -30,68 +203,73 @@
     fom_reset_state(work);
 }
 
+static void fom_unpack(const fom_float *X, const fom_float *yl, const fom_float *zl,
+                       fom_float *out) {
+    fom_int g, j, i;
+    for (g = 0, i = 0; g < NG; ++g)
+        for (j = 0; j < NJ; ++j, ++i) out[i] = X[g * NL + j];
+    for (g = 0; g < NG; ++g) out[OY + g] = yl[g];
+    for (j = 0; j < NJ; ++j) out[OZ + j] = zl[j];
+}
+
 fom_int fom_solve(FomWorkspace *work) {
-    fom_float *x = work->state;
-    fom_float *xn = work->new_state;
-    fom_float *t0 = work->t0;
-    fom_float *t1 = work->t1;
-    fom_float *pc = work->acc; /* su_p_p0 @ p0: constant across iterations */
+    FOM_ALIGN fom_float X[NXP];   /* padded x (lanes >= NJ stay 0) */
+    FOM_ALIGN fom_float zl[NL];
+    fom_float yl[NG];
+    fom_float *xo = work->new_state; /* previous state (residual only) */
+    fom_float pz[NJ];                /* su_p_p0 @ p0 (z rows): constant across iterations */
     const fom_int max_iter = work->settings.max_iter;
     const fom_float tol = work->settings.tol;
     const fom_int use_tol = work->settings.check_termination && tol > 0;
     fom_float res = 0.0;
     fom_int status = FOM_MAX_ITER_REACHED;
-    fom_int k, i;
-
-    for (i = 0; i < FOM_STATE_DIM; ++i) pc[i] = 0.0;
-    fom_apply_su_p_p0(work->params.p0, pc);
+    fom_int k, i, g, j;
+
+    if (!c_ready) fom_extract_coeffs(work);
+
+    FOM_PROF_BEGIN(setup);
+    for (j = 0; j < NJ; ++j) pz[j] = c_pd[j] * work->params.p0[j];
+    for (g = 0, i = 0; g < NG; ++g) {
+        for (j = 0; j < NJ; ++j, ++i) X[g * NL + j] = work->state[i];
+        for (; j < NL; ++j) X[g * NL + j] = 0.0;
+    }
+    for (g = 0; g < NG; ++g) yl[g] = work->state[OY + g];
+    for (j = 0; j < NJ; ++j) zl[j] = work->state[OZ + j];
+    for (; j < NL; ++j) zl[j] = 0.0;
+    FOM_PROF_END(setup);
 
     for (k = 0; k < max_iter; ++k) {
-        fom_float *sw;
-
-        /* t0 = clip(t0_c + t0_state @ x, lo, hi) */
-        for (i = 0; i < FOM_T0_DIM; ++i) t0[i] = FOM_MAT_t0_c[i];
-        fom_apply_t0_state(x, t0);
-        for (i = 0; i < FOM_T0_DIM; ++i) {
-            fom_float v = t0[i];
-            v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v;
-            v = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v;
-            t0[i] = v;
-        }
-
-        /* t1 = relu(t1_c + t1_state @ x + t1_t_t0 @ t0) */
-        for (i = 0; i < FOM_T1_DIM; ++i) t1[i] = FOM_MAT_t1_c[i];
-        fom_apply_t1_state(x, t1);
-        fom_apply_t1_t_t0(t0, t1);
-        for (i = 0; i < FOM_T1_DIM; ++i) t1[i] = t1[i] > 0 ? t1[i] : 0.0;
-
-        /* xn = su_state @ x + su_t_t0 @ t0 + su_t_t1 @ t1 + su_p_p0 @ p0 */
-        for (i = 0; i < FOM_STATE_DIM; ++i) xn[i] = pc[i];
-        fom_apply_su_state(x, xn);
-        fom_apply_su_t_t0(t0, xn);
-        fom_apply_su_t_t1(t1, xn);
-
-        if (use_tol || k == max_iter - 1) {
+        const fom_int need_res = use_tol || k == max_iter - 1;
+        if (need_res) fom_unpack(X, yl, zl, xo);
+
+        FOM_PROF_BEGIN(iteration);
+        if (c_lo_zero) fom_iterate(X, yl, zl, pz, 1);
+        else fom_iterate(X, yl, zl, pz, 0);
+        FOM_PROF_END(iteration);
+
+        if (need_res) {
+            FOM_PROF_BEGIN(residual);
+            fom_float *xs = work->solution.x;
+            fom_unpack(X, yl, zl, xs);
             res = 0.0;
             for (i = 0; i < FOM_STATE_DIM; ++i) {
-                fom_float d = xn[i] - x[i];
+                fom_float d = xs[i] - xo[i];
                 d = d < 0 ? -d : d;
                 res = d > res ? d : res;
             }
-        }
-
-        sw = x; x = xn; xn = sw;
-
-        if (use_tol && res <= tol) {
-            status = FOM_SOLVED;
-            ++k;
-            break;
-        }
-    }
-
-    for (i = 0; i < FOM_STATE_DIM; ++i) work->solution.x[i] = x[i];
-    if (x != work->state)
-        for (i = 0; i < FOM_STATE_DIM; ++i) work->state[i] = x[i];
+            FOM_PROF_END(residual);
+            if (use_tol && res <= tol) {
+                status = FOM_SOLVED;
+                ++k;
+                break;
+            }
+        }
+    }
+
+    FOM_PROF_BEGIN(writeback);
+    fom_unpack(X, yl, zl, work->solution.x);
+    for (i = 0; i < FOM_STATE_DIM; ++i) work->state[i] = work->solution.x[i];
+    FOM_PROF_END(writeback);
     work->solution.iter = k;
     work->solution.residual = res;
     work->solution.status = status;
```

## Agent notes

```
**Whole-solve time** (Apple M5, `-O3`; both builds run alternately, 5 × 20,000 solves each):
- Initial: `mean_us` ≈ 105.0 (first `make bench` run: 109.2)
- Final: `mean_us` ≈ 10.7 (`make bench` at 5,000 solves is noisy and reads 10.6–15 µs)
- **Speedup: about 9.9×.** `make verify` still prints `VERIFY OK`, with a worst error of 6.7e-15.

**Profile of the final solver** (`make profile`, hottest first; profiling overhead included):

| Region | total_us | calls | pct |
|---|---|---|---|
| iteration (fused t0 / t1 / state-update pass) | 85,859 | 1,000,000 | 98.68% |
| residual (last iteration only) | 645 | 5,000 | 0.74% |
| writeback (unpack state into solution) | 343 | 5,000 | 0.39% |
| setup (`su_p_p0 @ p0`, pack state) | 158 | 5,000 | 0.18% |

**Optimizations applied** (each one is followed by the structure it relies on):
1. **Replaced 7 general CSR operator applies with one fused pass over the state (105 → 24.7 µs).** *Structure:* the state splits into x(150), y(10) and z(15), with element i = 15g + j. I checked every non-zero against the operators:
   - Each t0 row touches only x_i (coefficient 1), y_g and z_j.
   - `t1_state` and `t1_t_t0` are block-row reductions over group g, and `t1_t_t0` equals exactly −2 × the x-block of `t1_state`.
   - In `su_state` and `su_t_t0`, the rows for x are an identity on t0, and `su_t_t0`'s z rows equal exactly −2 × the x-columns of `su_state`.
   - `su_t_t1` is an identity on y, and `su_p_p0` is diagonal into z.

   So one iteration becomes: t = clip(c + x + ay·y_g + bz·z_j), then w = x − 2t, then y' = relu(t1c + ty·y + P·w) and z' = sz·z + R·w + d·p. There are no multiplies by identity blocks, the update happens in place, and nothing is swapped. The per-element coefficients are pulled out once by probing the existing operator functions with unit vectors.
2. **Split the pass into restrict-qualified block kernels (24.7 → 15.4 µs).** *Structure:* the state is updated in place and z/y are read-only within an iteration. Copying z into a local and adding `restrict` removed the possible aliasing that was blocking the compiler from vectorizing.
3. **Padded each 15-wide group to 16 lanes, with z-accumulation and the y dot-product fused into the same pass (15.4 → 14.1 µs).** *Structure:* the 10 × 15 block layout. Padding coefficients and bounds are zero, so the padded lanes stay exactly 0. Vector loops have no scalar remainder, and the 16 z accumulators stay in registers across groups.
4. **Clipped with `fmax`/`fmin` instead of compare-and-select (14.1 → 12.9 µs).** *Structure:* the box projection. Each bound becomes a single `fmaxnm`/`fminnm` instruction, and the result is the same for non-NaN data.
5. **Specialized the kernel for an all-zero lower bound (12.9 → 11.5 µs).** *Structure:* `t0_lo` is all zero here, so the clip is a projection onto [0, hi]. This is detected at setup, removes one load per element, and keeps a general fallback.
6. **Interleaved the coefficient tables in 4-lane chunks (11.5 → 10.75 µs).** *Structure:* every field is read with the same stride. Laying out c, ay, bz, lo, hi, P and R contiguously per chunk lets the loads pair into `ldp` and improves locality. Chunk widths of 2, 8 and 16 were tested and were not faster.
7. **Computed `su_p_p0 @ p0` as a 15-element diagonal product (about 1% faster).** *Structure:* `su_p_p0` is diagonal into the z rows, so this replaces a 175-row CSR apply in each solve.

Rejected: FMA-accumulating the y dot-product, because it was not faster. I stopped after two consecutive rounds gained under 2%.
```
