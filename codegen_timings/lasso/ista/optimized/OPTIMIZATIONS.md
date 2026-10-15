# Optimization report

## Timing

```
translated (emitted)       8.1 us/solve
optimized                  3.3 us/solve
speedup                  2.43x
```

Optimization outcome: **kept (re-verified correct)**.

## What the agent changed

```diff
--- translated (emitted)
+++ optimized
@@ -2,6 +2,14 @@
 #include "fom_data.h"
 #include "fom_ops.h"
 #include "fom_linsolve.h"
+#include "fom_profile.h"
+
+#if defined(__aarch64__) && defined(__ARM_NEON)
+#include <arm_neon.h>
+#define FOM_USE_NEON 1
+#else
+#define FOM_USE_NEON 0
+#endif
 
 static void fom_reset_state(FomWorkspace *work) {
     fom_int i;
@@ -14,9 +22,23 @@
     settings->check_termination = FOM_CHECK_TERMINATION;
 }
 
+/* Zero-padded copy of t0_state (rows/cols 15 -> 16) so every inner loop is
+   an exact multiple of the SIMD width; built once in fom_setup. */
+#define FOM_PAD 16
+static fom_float fom_Mpad[FOM_PAD * FOM_PAD];
+
+static void fom_build_padded(void) {
+    fom_int i, j;
+    for (i = 0; i < FOM_PAD * FOM_PAD; ++i) fom_Mpad[i] = 0.0;
+    for (i = 0; i < FOM_STATE_DIM; ++i)
+        for (j = 0; j < FOM_STATE_DIM; ++j)
+            fom_Mpad[i * FOM_PAD + j] = FOM_MAT_t0_state[i * FOM_STATE_DIM + j];
+}
+
 void fom_setup(FomWorkspace *work, const FomSettings *settings) {
     fom_int i;
     work->settings = *settings;
+    fom_build_padded();
     for (i = 0; i < FOM_PARAM_P0_DIM; ++i) work->params.p0[i] = 0.0;
     fom_reset_state(work);
     work->solution.iter = 0;
@@ -31,45 +53,135 @@
 
 fom_int fom_solve(FomWorkspace *work) {
     const fom_float lam = FOM_T0_LAMBDA;
+    const fom_float nlam = -FOM_T0_LAMBDA;
     const fom_int max_iter = work->settings.max_iter;
     const fom_float tol = work->settings.tol;
     const fom_int check = work->settings.check_termination;
     const fom_int use_tol = (tol > 0.0) && (check > 0);
-    fom_float *state = work->state;
-    fom_float *ns = work->new_state;
-    fom_float *acc = work->acc;
-    fom_float *t0 = work->t0;
+    const fom_float *M = fom_Mpad;
+    fom_float a[FOM_PAD];   /* loop-invariant t0_p_p0 @ p0 */
+    fom_float x[FOM_PAD];   /* current state (= t0, since su_t_t0 = I) */
+#if !FOM_USE_NEON
+    fom_float y[FOM_PAD];   /* pre-activation */
+#endif
     fom_float res = 0.0;
     fom_int status = FOM_MAX_ITER_REACHED;
-    fom_int k, i, ctr = 0;
-
-    /* Parameter contribution is loop-invariant: acc = t0_p_p0 @ p0. */
-    for (i = 0; i < FOM_T0_DIM; ++i) acc[i] = 0.0;
-    fom_apply_t0_p_p0(work->params.p0, acc);
-
+    fom_int k, i, j, ctr = 0;
+
+    /* Parameter contribution is loop-invariant: a = t0_p_p0 @ p0. */
+    FOM_PROF_BEGIN(param_term);
+    {
+        const fom_float *P = FOM_MAT_t0_p_p0;
+        const fom_float *p0 = work->params.p0;
+        for (i = 0; i < FOM_T0_DIM; ++i) {
+            fom_float s = 0.0;
+            for (j = 0; j < FOM_PARAM_P0_DIM; ++j) s += P[i * FOM_PARAM_P0_DIM + j] * p0[j];
+            a[i] = s;
+        }
+        a[FOM_PAD - 1] = 0.0;
+        for (i = 0; i < FOM_STATE_DIM; ++i) x[i] = work->state[i];
+        x[FOM_PAD - 1] = 0.0;
+    }
+    FOM_PROF_END(param_term);
+
+    FOM_PROF_BEGIN(iterations);
+#if FOM_USE_NEON
+    {
+        /* 2-lane f64 kernel: y,z,w are 8 vector registers each; column j of
+           the (symmetric) matrix is multiplied by lane j&1 of xv[j>>1]. */
+        const float64x2_t vlam = vdupq_n_f64(lam);
+        const float64x2_t vnlam = vdupq_n_f64(nlam);
+        float64x2_t av[FOM_PAD / 2], xv[FOM_PAD / 2];
+        for (i = 0; i < FOM_PAD / 2; ++i) {
+            av[i] = vld1q_f64(a + 2 * i);
+            xv[i] = vld1q_f64(x + 2 * i);
+        }
+#define FOM_COL(dst, j) do { \
+            const fom_float *r_ = M + (j) * FOM_PAD; \
+            const float64x2_t xb_ = xv[(j) >> 1]; \
+            for (i = 0; i < FOM_PAD / 2; ++i) \
+                dst[i] = vfmaq_laneq_f64(dst[i], vld1q_f64(r_ + 2 * i), xb_, (j) & 1); \
+        } while (0)
+        for (k = 0; k < max_iter; ++k) {
+            float64x2_t yv[FOM_PAD / 2], zv[FOM_PAD / 2], wv[FOM_PAD / 2];
+            for (i = 0; i < FOM_PAD / 2; ++i) {
+                yv[i] = av[i];
+                zv[i] = vdupq_n_f64(0.0);
+                wv[i] = vdupq_n_f64(0.0);
+            }
+            FOM_COL(yv, 0);  FOM_COL(zv, 1);  FOM_COL(wv, 2);
+            FOM_COL(yv, 3);  FOM_COL(zv, 4);  FOM_COL(wv, 5);
+            FOM_COL(yv, 6);  FOM_COL(zv, 7);  FOM_COL(wv, 8);
+            FOM_COL(yv, 9);  FOM_COL(zv, 10); FOM_COL(wv, 11);
+            FOM_COL(yv, 12); FOM_COL(zv, 13); FOM_COL(wv, 14);
+            if (use_tol || k == max_iter - 1) {
+                float64x2_t rv = vdupq_n_f64(0.0);
+                for (i = 0; i < FOM_PAD / 2; ++i) {
+                    float64x2_t v = vaddq_f64(yv[i], vaddq_f64(zv[i], wv[i]));
+                    /* soft_threshold(v) = v - clamp(v, -lam, lam) */
+                    v = vsubq_f64(v, vminq_f64(vmaxq_f64(v, vnlam), vlam));
+                    rv = vmaxq_f64(rv, vabdq_f64(v, xv[i]));
+                    xv[i] = v;
+                }
+                res = vmaxvq_f64(rv);
+            } else {
+                for (i = 0; i < FOM_PAD / 2; ++i) {
+                    float64x2_t v = vaddq_f64(yv[i], vaddq_f64(zv[i], wv[i]));
+                    xv[i] = vsubq_f64(v, vminq_f64(vmaxq_f64(v, vnlam), vlam));
+                }
+            }
+            if (use_tol && ++ctr >= check) {
+                ctr = 0;
+                if (res > tol) continue;
+                ++k;
+                status = FOM_SOLVED;
+                break;
+            }
+        }
+#undef FOM_COL
+        for (i = 0; i < FOM_PAD / 2; ++i) vst1q_f64(x + 2 * i, xv[i]);
+    }
+#else
     for (k = 0; k < max_iter; ++k) {
-        /* t0 = soft_threshold(t0_state @ state + acc, lam) */
-        for (i = 0; i < FOM_T0_DIM; ++i) t0[i] = acc[i];
-        fom_apply_t0_state(state, t0);
-        for (i = 0; i < FOM_T0_DIM; ++i) {
-            fom_float v = t0[i];
-            t0[i] = v > lam ? v - lam : (v < -lam ? v + lam : 0.0);
-        }
-
-        /* state <- su_t_t0 @ t0 */
-        for (i = 0; i < FOM_STATE_DIM; ++i) ns[i] = 0.0;
-        fom_apply_su_t_t0(t0, ns);
-
+        /* y = a + t0_state @ x as column axpys; t0_state is symmetric, so
+           column j is the contiguous row j. Padding lane stays 0. */
+        {
+            /* Three independent partial sums (15 = 3 x 5 columns) cut the
+               serial FMA dependency chain per lane from 15 to 5. */
+            fom_float z[FOM_PAD], w[FOM_PAD];
+            for (i = 0; i < FOM_PAD; ++i) {
+                y[i] = a[i] + M[i] * x[0];
+                z[i] = M[FOM_PAD + i] * x[1];
+                w[i] = M[2 * FOM_PAD + i] * x[2];
+            }
+            for (j = 3; j < FOM_STATE_DIM; j += 3) {
+                const fom_float x0 = x[j], x1 = x[j + 1], x2 = x[j + 2];
+                const fom_float *r0 = M + j * FOM_PAD;
+                const fom_float *r1 = r0 + FOM_PAD;
+                const fom_float *r2 = r1 + FOM_PAD;
+                for (i = 0; i < FOM_PAD; ++i) {
+                    y[i] += r0[i] * x0;
+                    z[i] += r1[i] * x1;
+                    w[i] += r2[i] * x2;
+                }
+            }
+            for (i = 0; i < FOM_PAD; ++i) y[i] = y[i] + (z[i] + w[i]);
+        }
+
+        /* x_new = soft_threshold(y, lam); the state update is identity. */
         if (use_tol || k == max_iter - 1) {
             res = 0.0;
             for (i = 0; i < FOM_STATE_DIM; ++i) {
-                fom_float d = ns[i] - state[i];
+                fom_float v = y[i];
+                v = v > lam ? v - lam : (v < nlam ? v + lam : 0.0);
+                fom_float d = v - x[i];
                 if (d < 0) d = -d;
                 if (d > res) res = d;
             }
         }
-        {
-            fom_float *tmp = state; state = ns; ns = tmp;
+        for (i = 0; i < FOM_PAD; ++i) {
+            fom_float v = y[i];
+            x[i] = v > lam ? v - lam : (v < nlam ? v + lam : 0.0);
         }
         if (use_tol && ++ctr >= check) {
             ctr = 0;
@@ -79,10 +191,13 @@
             break;
         }
     }
-
-    if (state != work->state)
-        for (i = 0; i < FOM_STATE_DIM; ++i) work->state[i] = state[i];
-    for (i = 0; i < FOM_STATE_DIM; ++i) work->solution.x[i] = work->state[i];
+#endif
+    FOM_PROF_END(iterations);
+
+    for (i = 0; i < FOM_STATE_DIM; ++i) {
+        work->state[i] = x[i];
+        work->solution.x[i] = x[i];
+    }
     work->solution.iter = k;
     work->solution.residual = res;
     work->solution.status = status;
```

## Agent notes

```
**Whole-solve time (`mean_us`).** I measured both versions back to back with the same build flags, over 5×5000 and 3×200k solves. The 5000-solve `make bench` is noisy because the timer only resolves 1 µs: single runs of the original ranged from 6.1 to 10.2 µs.
- Initial: about 6.3 µs (steady state)
- Final: about 2.2 µs (steady state)
- Speedup: about 2.8×
- `make verify` still prints VERIFY OK. The worst max-abs error is 1.2e-14 against a tolerance of 1e-6.

**Profile of the final solver** (`profile_bin 200000`, mean 2.39 µs with profiling on):

| region | total_us | % |
|---|---|---|
| iterations (matvec + soft-threshold, 200 iterations) | 461113 | 99.14 |
| param_term (`t0_p_p0 @ p0`, done once, plus state load) | 3999 | 0.86 |

**Optimizations applied:**
1. **Removed the state-update step (identity structure).** `su_t_t0` is the 15×15 identity, stored in CSR form. The new state is just the soft-threshold output, so the CSR matvec, the `new_state` buffer and the pointer swap are all gone.
2. **Inlined the matvec at fixed size, written as column operations (symmetric structure).** `t0_state = I − tAᵀA` is symmetric, so column j is the same as row j and sits contiguously in memory. Each iteration becomes 15 vectorizable "add column times scalar" steps over the output instead of 15 separate row dot products. This also removes the call into another source file that the compiler could not inline. 6.3 → 3.8 µs.
3. **Padded the matrix to 16×16 (fixed sizes).** The matrix is copied once in `fom_setup` into a zero-padded static array (no dynamic allocation). Every vector loop is then an exact multiple of the 2-wide double SIMD width. The padding row and column are zero, so the extra 16th entry always stays exactly 0.
4. **Split the sum into 3 independent partial sums.** Profiling and cycle counts showed the loop was waiting on each multiply-add to finish before the next could start: a chain of 15 per entry, about 60 cycles per iteration. With three running sums over columns {0,3,…}, {1,4,…} and {2,5,…}, each chain is 5 long. This works because 15 = 3×5. It changes the rounding order only at the 1e-14 level. 3.8 → 2.41 µs.
5. **Hand-written NEON kernel** (guarded by `__aarch64__ && __ARM_NEON`; the portable scalar version above is kept as the fallback, and I checked separately that it also passes verification). The running sums stay in vector registers, and each column is multiplied directly by one lane of the state vector, which avoids the compiler's register spills. The soft-threshold is computed without branches as `v − clamp(v, −λ, λ)`, which gives exactly the same result. 2.41 → 2.2 µs. The kernel is now close to the hardware limit: about 120 multiply-adds per iteration, roughly 30 cycles of throughput against about 35 cycles of critical path.
6. **Residual only when it is needed.** The settings give `tol = 0`, so the tolerance check is off and the residual is computed only on the last iteration. The parameter term `t0_p_p0 @ p0` does not change across iterations, so it is computed once per solve.

**Tried and dropped:** a matvec that skips zero entries of the state, using the soft-threshold's support. It was slower (4.2 vs 3.8 µs) because on average about 10 of the 15 entries are nonzero, so the branching and index bookkeeping cost more than they saved.

No division and no dynamic allocation anywhere. The `FOM_PROF_BEGIN/END` markers remain around `param_term` and `iterations`.
```
