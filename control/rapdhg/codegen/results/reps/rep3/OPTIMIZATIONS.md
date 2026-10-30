# Optimization report

## Timing

```
translated (emitted)    1122.0 us/solve
optimized                 40.1 us/solve
speedup                 28.00x
```

Optimization outcome: **kept (re-verified correct)**.

## Rounds

| round | before (us) | after (us) | gain | kept | agent (s) | outcome |
|---:|---:|---:|---:|:---:|---:|---|
| 1 | 1122.036 | 44.101 | 96.1% | yes | 1162 | continued |
| 2 | 43.685 | 40.078 | 8.3% | yes | 1867 | continued |
| 3 | - | - | - | no | 573 | the round left the sources unchanged |

## Kernel checks

Each `fom_apply_<name>` in the shipped `fom_ops.c`, checked by `make verify` against the package's reference over the same `fom_data.c` matrix. *basis* probes every unit vector, which recovers the routine's matrix column by column (an exact check), plus random vectors; *random* uses random vectors only (the matrix is over the probe budget). A `fused_<block>` row is a `fom_fused_<block>`, checked against the sum of its members: *basis* there probes every unit vector of each source with every schedule value at 0 and with each alone at 1, which recovers every member exactly.

| matrix | shape | nnz | rewritten | check | probes | max error |
|---|---|---|---|---|---|---|
| `t0_state` | 120x400 | 120 | yes | basis | 402 | 0.000e+00 |
| `t0_state__d_taux` | 120x400 | 520 | yes | basis | 402 | 0.000e+00 |
| `t0_state__d_tth` | 120x400 | 520 | yes | basis | 402 | 0.000e+00 |
| `t0_p_p1` | 120x120 | 0 | yes | basis | 122 | 0.000e+00 |
| `t0_p_p1__d_tau` | 120x120 | 120 | yes | basis | 122 | 0.000e+00 |
| `t1_state` | 40x400 | 40 | yes | basis | 402 | 0.000e+00 |
| `t1_state__d_eta` | 40x400 | 1600 | yes | basis | 402 | 0.000e+00 |
| `t1_p_p0` | 40x40 | 0 | yes | basis | 42 | 0.000e+00 |
| `t1_p_p0__d_eta` | 40x40 | 40 | yes | basis | 42 | 0.000e+00 |
| `t1_t_t0` | 40x120 | 0 | yes | basis | 122 | 0.000e+00 |
| `t1_t_t0__d_eta` | 40x120 | 520 | yes | basis | 122 | 0.000e+00 |
| `su_state` | 400x400 | 40 | yes | basis | 402 | 0.000e+00 |
| `su_state__d_d1` | 400x400 | 160 | yes | basis | 402 | 0.000e+00 |
| `su_state__d_g2` | 400x400 | 160 | yes | basis | 402 | 0.000e+00 |
| `su_state__d_m1` | 400x400 | 40 | yes | basis | 402 | 0.000e+00 |
| `su_t_t0` | 400x120 | 0 | yes | basis | 122 | 0.000e+00 |
| `su_t_t0__d_d2` | 400x120 | 120 | yes | basis | 122 | 0.000e+00 |
| `su_t_t0__d_g1` | 400x120 | 120 | yes | basis | 122 | 0.000e+00 |
| `su_t_t1` | 400x40 | 0 | yes | basis | 42 | 0.000e+00 |
| `su_t_t1__d_d2` | 400x40 | 40 | yes | basis | 42 | 0.000e+00 |
| `su_t_t1__d_g1` | 400x40 | 40 | yes | basis | 42 | 0.000e+00 |
| `su_t_t1__d_m2` | 400x40 | 40 | yes | basis | 42 | 0.000e+00 |
| `fused_t0` | 120x400 | 1160 | yes | basis | 1202 | 0.000e+00 |
| `fused_t1` | 40x520 | 2160 | yes | basis | 1042 | 0.000e+00 |
| `fused_su` | 400x560 | 760 | yes | basis | 3922 | 0.000e+00 |
| `step_t0` | 120x400 | 1160 | yes | basis | 1206 | 1.066e-14 |
| `step_t1` | 40x520 | 2160 | yes | basis | 1046 | 1.455e-11 |
| `step_su` | 400x560 | 760 | yes | basis | 3922 | 0.000e+00 |
| `fused_su__rest` | 400x560 | 760 | yes | basis | 3922 | 0.000e+00 |
| `step_su__rest` | 400x560 | 760 | yes | basis | 3922 | 0.000e+00 |
| `iterate` | 400x400 | 4080 | yes | basis | 4406 | 1.091e-11 |

### Rule violations

_None: the collected rewrite passed the rules and was kept._

## Blocked by rules

Optimizations the agent considered but did not make because a rule forbade them, in its own words.

| round | idea | rule | expected gain |
|---|---|---|---|
| 1 | Store s5 (and the state) in the P/M/K sum/difference basis, to drop the per-iteration basis transforms | state layout and headers are fixed, and verify compares states in the spec layout | ~5% |
| 1 | Overlap the ring buffers so a state's s4 rows alias the previous state's s0 rows, removing the s4 copy | the fixed contiguous state layout of the step routines (the overlap also clobbers the t0 history) | ~3% |
| 1 | Compute the C product before the t0 step in the solver's fast path (memory reports ~5% from this ordering) | `fom_step_t1` must contain the C product, and the solver may not compute products | ~3–5% |
| 1 | Keep the 80 KB bias cache in the workspace instead of a file-static array | `fom_workspace.h` is fixed | 0 (embedded hygiene only) |
| 1 | Precompute the derived P/M/K tables and taps in `fom_setup` instead of lazily inside the routines | the solver may not read `FOM_MAT_*` data and `fom_ops.c` has no setup hook | negligible (one branch per call) |
| 2 | Pass scaled bias terms (τ·b0, η·b1) to the step routines instead of materialised bias vectors, so the 82 KB, 64-slot bias cache can go | the fixed `fom_step_*` signatures in `include/fom_ops.h` | unknown (bias costs about 3%, plus L1 pressure) |
| 2 | Let `fom_iterate` skip writing the s3 rows (or take a flag saying so), so the fused single-pass iteration can keep the deferred, grouped s3 fold. The fused path would save the recomputed differences and the T0 reloads, about 40 ops and 60 loads per iteration | `fom_iterate` must write the complete new state, and its signature is fixed | about 3 to 5% |
| 2 | Let `fom_step_t1` also return C·s5_next, so the next iteration's cprod is spread through the current iteration's tail | the fixed step signatures and outputs | unknown |
| 3 | Change the step/iterate interface so fom_iterate can skip the s3 rows (the fold keeps them) and the fused one-pass iteration can be used | the fixed signatures in include/ and the rule that fom_iterate must compute the whole new state | about 0 us (ties the separate step calls; would only enable later fusion) |
| 3 | Pass b0 and tau into the t0 step instead of a materialized bias vector, which removes the bias cache | the fixed fom_step_t0 signature (bias is a vector) | about 1 us |
| 3 | Run the C·s5 product in float32 or on the SME matrix unit | float64 data type and fixed build flags | unknown |

## What the agent changed

```diff
--- src/fom_solver.c (translated)
+++ src/fom_solver.c (optimized)
@@ -1,9 +1,8 @@
 #include "fom_solver.h"
 #include "fom_data.h"
 #include "fom_ops.h"
+#include "fom_profile.h"
 
-/* su has no constant or parameter terms. */
-static const fom_float FOM_ZERO_SU[FOM_STATE_DIM] = {0};
 
 void fom_set_default_settings(FomSettings *settings) {
     settings->max_iter = FOM_MAX_ITER;
@@ -32,21 +31,67 @@
     reset_state(work);
 }
 
+/* Bias cache: bias_t0 = a0 + tau b0 and bias_t1 = a1 + eta b1 depend only
+   on (tau, eta) within a solve, and the schedules repeat with a short
+   period, so each slot (it mod FOM_BIAS_SLOTS) keeps the last bias built
+   for its (tau, eta) in the current solve. */
+#define FOM_BIAS_SLOTS 64
+static fom_float bias_cache[FOM_BIAS_SLOTS][160];
+static fom_float bias_tau[FOM_BIAS_SLOTS], bias_eta[FOM_BIAS_SLOTS];
+static unsigned bias_gen[FOM_BIAS_SLOTS], bias_cur = 0;
+
+/* Ring of state buffers. In the iterations where g2 = 0 and g1 = 1 (all
+   but the periodic restarts) the solver runs the steps and the su shift
+   rows itself, writing t0 straight into the s1 rows and t1 into the s0
+   rows of the next buffer. The s3 rows (the running average
+   s3 <- d1 s3 + d2 t0, read only by restarts and at the end) are left
+   stale and folded once per group of up to FOM_GROUP iterations from the
+   t0 history still held in the s1 rows of the ring. */
+#define FOM_GROUP 4
+/* Rows padded to 408 doubles (3264 bytes) and cache-line aligned, so the
+   buffers do not sit at a 4 KiB-aliasing stride from each other. */
+static fom_float ring[FOM_GROUP + 1][FOM_STATE_DIM + 8] __attribute__((aligned(64)));
+
+/* s3 of x = (prod d1) s3 of base + sum_i d2_i (prod_{j>i} d1_j) t0_i. */
+static void fold_s3(fom_float *x, const fom_float *base, fom_float *const *hist,
+                    const fom_float *pd1, const fom_float *pd2, fom_int np) {
+    fom_float c[FOM_GROUP], prod = 1.0;
+    fom_int i, e;
+    for (i = np - 1; i >= 0; --i) {
+        c[i] = pd2[i] * prod;
+        prod *= pd1[i];
+    }
+    for (e = 0; e < 120; ++e) {
+        fom_float v = prod * base[200 + e];
+        for (i = 0; i < np; ++i) v += c[i] * hist[i][40 + e];
+        x[200 + e] = v;
+    }
+}
+
 fom_int fom_solve(FomWorkspace *work) {
     fom_float *a0 = work->acc;        /* t0_p_p1 @ p1 */
     fom_float *b0 = work->acc + 120;  /* t0_p_p1__d_tau @ p1 */
     fom_float *a1 = work->acc + 240;  /* t1_p_p0 @ p0 */
     fom_float *b1 = work->acc + 280;  /* t1_p_p0__d_eta @ p0 */
-    fom_float *x = work->state, *xn = work->new_state, *tp;
-    fom_float *t0 = work->t0, *t1 = work->t1;
-    fom_int i, it, n = work->settings.max_iter;
+    fom_float *x, *xp, *base, *hist[FOM_GROUP];
+    fom_float pd1[FOM_GROUP], pd2[FOM_GROUP];
+    fom_int i, it, r = 0, np = 0, n = work->settings.max_iter;
     fom_float res = 0.0;
 
+    if (++bias_cur == 0) {
+        for (i = 0; i < FOM_BIAS_SLOTS; ++i) bias_gen[i] = 0;
+        bias_cur = 1;
+    }
     for (i = 0; i < 320; ++i) work->acc[i] = 0.0;
     fom_apply_t0_p_p1(work->params.p1, a0);
     fom_apply_t0_p_p1__d_tau(work->params.p1, b0);
     fom_apply_t1_p_p0(work->params.p0, a1);
     fom_apply_t1_p_p0__d_eta(work->params.p0, b1);
+
+    x = ring[0];
+    for (i = 0; i < FOM_STATE_DIM; ++i) x[i] = work->state[i];
+    xp = x;
+    base = x;
 
     for (it = 0; it < n; ++it) {
         const fom_float taux = fom_sched_taux(it), tth = fom_sched_tth(it);
@@ -54,19 +99,78 @@
         const fom_float d1 = fom_sched_d1(it), g2 = fom_sched_g2(it);
         const fom_float m1 = fom_sched_m1(it), d2 = fom_sched_d2(it);
         const fom_float g1 = fom_sched_g1(it), m2 = fom_sched_m2(it);
-        /* Biases built in the output buffers: step routines read bias[i]
-           before writing out[i], so in-place is safe. */
-        for (i = 0; i < 120; ++i) t0[i] = a0[i] + tau * b0[i];
-        fom_step_t0(x, taux, tth, t0, &FOM_ACT_t0, t0);
-        for (i = 0; i < 40; ++i) t1[i] = a1[i] + eta * b1[i];
-        fom_step_t1(x, t0, eta, t1, &FOM_ACT_t1, t1);
-        fom_step_su(x, t0, t1, d1, g2, m1, d2, g1, m2, FOM_ZERO_SU, xn);
-        tp = x; x = xn; xn = tp;
+        const fom_int slot = it & (FOM_BIAS_SLOTS - 1);
+        fom_float *bt0 = bias_cache[slot], *bt1 = bias_cache[slot] + 120;
+        fom_float *xn;
+        if (bias_gen[slot] != bias_cur || bias_tau[slot] != tau || bias_eta[slot] != eta) {
+            FOM_PROF_BEGIN(bias);
+            for (i = 0; i < 120; ++i) bt0[i] = a0[i] + tau * b0[i];
+            for (i = 0; i < 40; ++i) bt1[i] = a1[i] + eta * b1[i];
+            bias_gen[slot] = bias_cur;
+            bias_tau[slot] = tau;
+            bias_eta[slot] = eta;
+            FOM_PROF_END(bias);
+        }
+        /* The last iteration runs in full, so the residual sees whole states. */
+        if (g2 == 0.0 && g1 == 1.0 && it != n - 1) {
+            if (np == FOM_GROUP) {
+                FOM_PROF_BEGIN(fold);
+                fold_s3(x, base, hist, pd1, pd2, np);
+                FOM_PROF_END(fold);
+                base = x;
+                np = 0;
+            }
+            r = (r == FOM_GROUP) ? 0 : r + 1;
+            xn = ring[r];
+            {
+                FOM_PROF_BEGIN(step_t0);
+                fom_step_t0(x, taux, tth, bt0, &FOM_ACT_t0, xn + 40);
+                FOM_PROF_END(step_t0);
+            }
+            {
+                FOM_PROF_BEGIN(step_t1);
+                fom_step_t1(x, xn + 40, eta, bt1, &FOM_ACT_t1, xn);
+                FOM_PROF_END(step_t1);
+            }
+            {
+                /* su shift rows: s2 <- d1 s2 + d2 t1, s4 <- s0, s5 <- m1 s2 + m2 t1 */
+                FOM_PROF_BEGIN(su_rows);
+                for (i = 0; i < 40; ++i) {
+                    const fom_float p = x[160 + i], y = xn[i];
+                    xn[160 + i] = d1 * p + d2 * y;
+                    xn[320 + i] = x[i];
+                    xn[360 + i] = m1 * p + m2 * y;
+                }
+                FOM_PROF_END(su_rows);
+            }
+            hist[np] = xn;
+            pd1[np] = d1;
+            pd2[np] = d2;
+            ++np;
+        } else {
+            if (np > 0) {
+                FOM_PROF_BEGIN(fold);
+                fold_s3(x, base, hist, pd1, pd2, np);
+                FOM_PROF_END(fold);
+                np = 0;
+            }
+            r = (r == FOM_GROUP) ? 0 : r + 1;
+            xn = ring[r];
+            {
+                FOM_PROF_BEGIN(iterate);
+                fom_iterate(x, taux, tth, eta, d1, g2, m1, d2, g1, m2, bt0, bt1, 0, &FOM_ACT_t0, &FOM_ACT_t1, xn + 40, xn, xn);
+                FOM_PROF_END(iterate);
+            }
+            base = xn;
+        }
+        xp = x;
+        x = xn;
     }
+    if (np > 0) fold_s3(x, base, hist, pd1, pd2, np);
 
     if (n > 0) {
         for (i = 0; i < FOM_STATE_DIM; ++i) {
-            fom_float d = x[i] - xn[i];
+            fom_float d = x[i] - xp[i];
             if (d < 0) d = -d;
             if (d > res) res = d;
         }
--- src/fom_ops.c (translated)
+++ src/fom_ops.c (optimized)
@@ -262,47 +262,6 @@
         s = 0.0;
         for (p = FOM_MAT_su_t_t1__d_m2_ROWPTR[i]; p < FOM_MAT_su_t_t1__d_m2_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t1__d_m2_VALS[p] * t_t1[FOM_MAT_su_t_t1__d_m2_COLIDX[p]];
         out[i] += sc_m2 * s;
-    }
-}
-
-void fom_step_t0(const fom_float *state, fom_float sc_taux, fom_float sc_tth, const fom_float *bias, const FomAct_t0 *act, fom_float *out) {
-    fom_int i, r, p;
-    for (r = 0; r < 120; ++r) {
-        fom_float s, v;
-        i = r;
-        v = bias[i];
-        s = 0.0;
-        for (p = FOM_MAT_t0_state_ROWPTR[i]; p < FOM_MAT_t0_state_ROWPTR[i + 1]; ++p) s += FOM_MAT_t0_state_VALS[p] * state[FOM_MAT_t0_state_COLIDX[p]];
-        v += s;
-        s = 0.0;
-        for (p = FOM_MAT_t0_state__d_taux_ROWPTR[i]; p < FOM_MAT_t0_state__d_taux_ROWPTR[i + 1]; ++p) s += FOM_MAT_t0_state__d_taux_VALS[p] * state[FOM_MAT_t0_state__d_taux_COLIDX[p]];
-        v += sc_taux * s;
-        s = 0.0;
-        for (p = FOM_MAT_t0_state__d_tth_ROWPTR[i]; p < FOM_MAT_t0_state__d_tth_ROWPTR[i + 1]; ++p) s += FOM_MAT_t0_state__d_tth_VALS[p] * state[FOM_MAT_t0_state__d_tth_COLIDX[p]];
-        v += sc_tth * s;
-        if (v < act->lo[i]) v = act->lo[i];
-        out[i] = v;
-    }
-}
-
-void fom_step_t1(const fom_float *state, const fom_float *t_t0, fom_float sc_eta, const fom_float *bias, const FomAct_t1 *act, fom_float *out) {
-    fom_int i, r, p;
-    for (r = 0; r < 40; ++r) {
-        fom_float s, v;
-        i = r;
-        v = bias[i];
-        s = 0.0;
-        for (p = FOM_MAT_t1_state_ROWPTR[i]; p < FOM_MAT_t1_state_ROWPTR[i + 1]; ++p) s += FOM_MAT_t1_state_VALS[p] * state[FOM_MAT_t1_state_COLIDX[p]];
-        v += s;
-        s = 0.0;
-        for (p = FOM_MAT_t1_state__d_eta_ROWPTR[i]; p < FOM_MAT_t1_state__d_eta_ROWPTR[i + 1]; ++p) s += FOM_MAT_t1_state__d_eta_VALS[p] * state[FOM_MAT_t1_state__d_eta_COLIDX[p]];
-        v += sc_eta * s;
-        s = 0.0;
-        for (p = FOM_MAT_t1_t_t0__d_eta_ROWPTR[i]; p < FOM_MAT_t1_t_t0__d_eta_ROWPTR[i + 1]; ++p) s += FOM_MAT_t1_t_t0__d_eta_VALS[p] * t_t0[FOM_MAT_t1_t_t0__d_eta_COLIDX[p]];
-        v += sc_eta * s;
-        if (v < act->lo[i]) v = act->lo[i];
-        if (v > act->hi[i]) v = act->hi[i];
-        out[i] = v;
     }
 }
 
@@ -415,8 +374,307 @@
     }
 }
 
+/* Structure of the iteration (checked against the data by make verify):
+   - t0_state__d_tth = -t0_state__d_taux, both reading A0 (120x40) from s0 / s4,
+     so the t0 product is A0 w with w = taux s0 - tth s4;
+   - A0 = [I; -I; L; -L], L (20x40) a 10-tap block Toeplitz: row 2k+r is
+     sum_m a_{k-m} (w[4m+3-r] - w[4m+1-r]), a_j = A0[80+2j, col 3];
+   - t1_t_t0__d_eta = -A0^T, so D t0 = -(t0[0:40] - t0[40:80] + L^T v),
+     v = t0[80:100] - t0[100:120];
+   - t1_state__d_eta reads s5 densely (CSR rows = row-major 40x40 block C);
+   - every selection / shift product has unit coefficients. */
+typedef double v2 __attribute__((vector_size(16)));
+typedef double v2u __attribute__((vector_size(16), aligned(8)));
+#define LD(p) (*(const v2u *)(p))
+#define ST(p, v) (*(v2u *)(p) = (v))
+#define SWAP(v) __builtin_shufflevector((v), (v), 1, 0)
+#define VMAX(a, b) __builtin_elementwise_maximum((a), (b))
+#define VMIN(a, b) __builtin_elementwise_minimum((a), (b))
+
+/* C (index 4a + 2s + c) is symmetric and invariant under swapping s and
+   under swapping c, so with x_b = (s-sum c0, s-sum c1), sp = x0 + x1,
+   sm = x0 - x1 and d = (s-difference c0, c1) it splits into P (on sp),
+   M (on sm) and K (on both lanes of d), each 10x10. Derived once from
+   the data on first use; the 1/2 factors of the inverse transform are
+   folded in. Also caches the Toeplitz taps a_j of L. */
+static v2 ctab_pm[100], ctab_kk[100];
+static fom_float ctab_a[10], atab_a[10];
+/* The taps are affine past the first (a_j = a_1 + (j-1) dl for j >= 1, the
+   positions of a double integrator); when the data says so (to a few ulps
+   of the largest tap), L and L^T are applied by running prefix / suffix sums. */
+static fom_float ctab_dl, atab_dl;
+static int ctab_lin = 0, atab_lin = 0;
+
+static int taps_affine(const fom_float *a, fom_float *dl) {
+    fom_float big = 0.0, tol, d = a[2] - a[1];
+    fom_int j;
+    for (j = 0; j < 10; ++j) big = (a[j] > big) ? a[j] : (-a[j] > big ? -a[j] : big);
+    tol = big; /* a few ulps of the largest tap: big * 2^-46 */
+    for (j = 0; j < 46; ++j) tol *= 0.5;
+    for (j = 3; j < 10; ++j) {
+        fom_float r = a[j] - (a[j - 1] + d);
+        if (r < 0) r = -r;
+        if (r > tol) return 0;
+    }
+    *dl = d;
+    return 1;
+}
+static int ctab_ready = 0, atab_ready = 0;
+
+/* t1 side: C tables and the taps read from t1_t_t0__d_eta = -A0^T
+   (row 0 is -e_0 + e_40 + sum_k a_k e_{81+2k} - ...). */
+static void ctab_fill(void) {
+    const fom_float *C = FOM_MAT_t1_state__d_eta_VALS;
+    fom_int a, b;
+    for (a = 0; a < 10; ++a) {
+        for (b = 0; b < 10; ++b) {
+            const fom_float *r0 = C + 40 * (4 * a), *r2 = C + 40 * (4 * a + 2);
+            const fom_float s00 = (r0[4 * b] + r0[4 * b + 2]) + (r2[4 * b] + r2[4 * b + 2]);
+            const fom_float s01 = (r0[4 * b + 1] + r0[4 * b + 3]) + (r2[4 * b + 1] + r2[4 * b + 3]);
+            const fom_float kk = 0.5 * 0.5 * ((r2[4 * b + 2] - r2[4 * b]) - (r0[4 * b + 2] - r0[4 * b]));
+            v2 pm = {0.5 * 0.5 * 0.5 * (s00 + s01), 0.5 * 0.5 * 0.5 * (s00 - s01)};
+            v2 kv = {kk, kk};
+            ctab_pm[10 * b + a] = pm;
+            ctab_kk[10 * b + a] = kv;
+        }
+    }
+    for (a = 0; a < 10; ++a)
+        ctab_a[a] = FOM_MAT_t1_t_t0__d_eta_VALS[FOM_MAT_t1_t_t0__d_eta_ROWPTR[0] + 2 + a];
+    ctab_lin = taps_affine(ctab_a, &ctab_dl);
+    ctab_ready = 1;
+}
+
+/* t0 side: the taps read from t0_state__d_taux (row 80+2a has -a_a, a_a, ...). */
+static void atab_fill(void) {
+    fom_int a;
+    for (a = 0; a < 10; ++a)
+        atab_a[a] = FOM_MAT_t0_state__d_taux_VALS[FOM_MAT_t0_state__d_taux_ROWPTR[80 + 2 * a] + 1];
+    atab_lin = taps_affine(atab_a, &atab_dl);
+    atab_ready = 1;
+}
+
+#define UNROLL _Pragma("clang loop unroll(full)")
+
+/* C s5 in the sum/difference basis (see ctab_fill): rows 4m..4m+3 of C s5
+   are (h - z, h + z) with h = cs[2m], z = cs[2m+1]. */
+static inline __attribute__((always_inline)) void cprod(const fom_float *restrict s5, v2 *cs) {
+    const v2 pm1 = {1.0, -1.0};
+    v2 y[10], z[10];
+    fom_int k, m;
+    UNROLL for (k = 0; k < 10; ++k) {
+        const v2 u01 = LD(s5 + 4 * k), u23 = LD(s5 + 4 * k + 2);
+        const v2 x = u01 + u23, d = u23 - u01;
+        const v2 sx = x[0] + x[1] * pm1;
+        if (k == 0) {
+            UNROLL for (m = 0; m < 10; ++m) {
+                y[m] = ctab_pm[m] * sx;
+                z[m] = ctab_kk[m] * d;
+            }
+        } else {
+            UNROLL for (m = 0; m < 10; ++m) {
+                y[m] += ctab_pm[10 * k + m] * sx;
+                z[m] += ctab_kk[10 * k + m] * d;
+            }
+        }
+    }
+    UNROLL for (m = 0; m < 10; ++m) {
+        const v2 h = y[m][0] + y[m][1] * pm1;
+        cs[2 * m] = h;
+        cs[2 * m + 1] = z[m];
+    }
+}
+
+/* t0 = relu(s1 + A0 w + bias), w = taux s0 - tth s4, into T0; vv gets the
+   lane-swapped v = t0[80:100] - t0[100:120]. With `fast`, also the s3 rows
+   of xn (d1 s3 + d2 t0). */
+static inline __attribute__((always_inline)) void t0_part(const fom_float *restrict state, fom_float sc_taux, fom_float sc_tth, fom_float sc_d1, fom_float sc_d2, const fom_float *restrict bias_t0, const fom_float *restrict lo0, fom_float *T0, v2 *vv, fom_float *xn, const int fast, const int lin) {
+    const fom_float *restrict s0 = state, *restrict s1 = state + 40;
+    const fom_float *restrict s3 = state + 200, *restrict s4 = state + 320;
+    const fom_float *restrict ta = atab_a;
+    const fom_float ntth = -sc_tth;
+    const v2 d1 = {sc_d1, sc_d1}, d2 = {sc_d2, sc_d2};
+    v2 w[20], e[10], lw[10];
+    fom_int j, k, m;
+    UNROLL for (j = 0; j < 20; ++j) w[j] = sc_taux * LD(s0 + 2 * j) + ntth * LD(s4 + 2 * j);
+    UNROLL for (m = 0; m < 10; ++m) e[m] = SWAP(w[2 * m + 1] - w[2 * m]);
+    if (lin) {
+        /* lw_k = a0 e_k + U_k, U_k = sum_{m<k} a_{k-m} e_m, U_{k+1} = U_k + a1 e_k + dl S_k,
+           S_k = sum_{m<k} e_m */
+        const fom_float a0 = ta[0], a1 = ta[1], dl = atab_dl;
+        v2 S = e[0], U = a1 * e[0];
+        lw[0] = a0 * e[0];
+        UNROLL for (k = 1; k < 10; ++k) {
+            lw[k] = a0 * e[k] + U;
+            if (k < 9) {
+                U = U + (a1 * e[k] + dl * S);
+                if (k < 8) S = S + e[k];
+            }
+        }
+    } else {
+        /* two interleaved partial sums halve the dependent FMA chain */
+        UNROLL for (k = 0; k < 10; ++k) {
+            v2 acc = ta[k] * e[0], acc2;
+            if (k == 0) {
+                lw[k] = acc;
+                continue;
+            }
+            acc2 = ta[k - 1] * e[1];
+            UNROLL for (m = 2; m <= k; ++m) {
+                if (m & 1) acc2 += ta[k - m] * e[m];
+                else acc += ta[k - m] * e[m];
+            }
+            lw[k] = acc + acc2;
+        }
+    }
+    UNROLL for (j = 0; j < 20; ++j) {
+        const v2 u = VMAX(LD(s1 + 2 * j) + w[j] + LD(bias_t0 + 2 * j), LD(lo0 + 2 * j));
+        const v2 l = VMAX(LD(s1 + 40 + 2 * j) - w[j] + LD(bias_t0 + 40 + 2 * j), LD(lo0 + 40 + 2 * j));
+        ST(T0 + 2 * j, u);
+        ST(T0 + 40 + 2 * j, l);
+        if (fast) {
+            ST(xn + 200 + 2 * j, d1 * LD(s3 + 2 * j) + d2 * u);
+            ST(xn + 240 + 2 * j, d1 * LD(s3 + 40 + 2 * j) + d2 * l);
+        }
+    }
+    UNROLL for (k = 0; k < 10; ++k) {
+        const v2 u = VMAX(LD(s1 + 80 + 2 * k) + lw[k] + LD(bias_t0 + 80 + 2 * k), LD(lo0 + 80 + 2 * k));
+        const v2 l = VMAX(LD(s1 + 100 + 2 * k) - lw[k] + LD(bias_t0 + 100 + 2 * k), LD(lo0 + 100 + 2 * k));
+        ST(T0 + 80 + 2 * k, u);
+        ST(T0 + 100 + 2 * k, l);
+        if (fast) {
+            ST(xn + 280 + 2 * k, d1 * LD(s3 + 80 + 2 * k) + d2 * u);
+            ST(xn + 300 + 2 * k, d1 * LD(s3 + 100 + 2 * k) + d2 * l);
+        }
+        vv[k] = SWAP(u - l);
+    }
+}
+
+/* t1 = clip(s0 + eta (C s5 + D t0) + bias), D t0 = -(t0[0:40] - t0[40:80] + L^T v),
+   into T1. With `fast`, also the s2, s5 rows of xn. */
+static inline __attribute__((always_inline)) void t1_part(const fom_float *restrict state, const fom_float *T0, const v2 *vv, const v2 *cs, fom_float sc_eta, fom_float sc_d1, fom_float sc_m1, fom_float sc_d2, fom_float sc_m2, const fom_float *restrict bias_t1, const fom_float *restrict lo1, const fom_float *restrict hi1, fom_float *T1, fom_float *xn, const int fast, const int lin) {
+    const fom_float *restrict s0 = state, *restrict s2 = state + 160;
+    const fom_float *restrict ta = ctab_a;
+    const v2 eta = {sc_eta, sc_eta};
+    const v2 d1 = {sc_d1, sc_d1}, d2 = {sc_d2, sc_d2}, m1 = {sc_m1, sc_m1}, m2 = {sc_m2, sc_m2};
+    fom_int k, m;
+    /* rows 4m..4m+3: eta (C s5 + D t0) = eta ((h - g, h + g) + (l - u)) with
+       g = z - f, f = (L^T v)_m, so f is accumulated straight into g. */
+#define T1_ROWS(m, g)                                                                              \
+    do {                                                                                           \
+        const v2 h = cs[2 * (m)];                                                                  \
+        const v2 x01 = (h - (g)) + (LD(T0 + 40 + 4 * (m)) - LD(T0 + 4 * (m)));                     \
+        const v2 x23 = (h + (g)) + (LD(T0 + 42 + 4 * (m)) - LD(T0 + 2 + 4 * (m)));                 \
+        const v2 y01 = VMIN(VMAX(LD(s0 + 4 * (m)) + LD(bias_t1 + 4 * (m)) + eta * x01, LD(lo1 + 4 * (m))), LD(hi1 + 4 * (m))); \
+        const v2 y23 = VMIN(VMAX(LD(s0 + 4 * (m) + 2) + LD(bias_t1 + 4 * (m) + 2) + eta * x23, LD(lo1 + 4 * (m) + 2)), LD(hi1 + 4 * (m) + 2)); \
+        ST(T1 + 4 * (m), y01);                                                                     \
+        ST(T1 + 4 * (m) + 2, y23);                                                                 \
+        if (fast) {                                                                                \
+            const v2 p01 = LD(s2 + 4 * (m)), p23 = LD(s2 + 4 * (m) + 2);                           \
+            ST(xn + 160 + 4 * (m), d1 * p01 + d2 * y01);                                           \
+            ST(xn + 162 + 4 * (m), d1 * p23 + d2 * y23);                                           \
+            ST(xn + 360 + 4 * (m), m1 * p01 + m2 * y01);                                           \
+            ST(xn + 362 + 4 * (m), m1 * p23 + m2 * y23);                                           \
+        }                                                                                          \
+    } while (0)
+    if (lin) {
+        /* f_m = a0 v_m + U_m, U_m = sum_{k>m} a_{k-m} v_k, U_{m-1} = U_m + a1 v_m + dl S_m,
+           S_m = sum_{k>m} v_k; rows run from the last block back. */
+        const fom_float a0 = ta[0], a1 = ta[1], dl = ctab_dl;
+        v2 S = vv[9], U = a1 * vv[9];
+        {
+            const v2 g = cs[19] - a0 * vv[9];
+            T1_ROWS(9, g);
+        }
+        UNROLL for (m = 8; m >= 0; --m) {
+            const v2 g = (cs[2 * m + 1] - U) - a0 * vv[m];
+            T1_ROWS(m, g);
+            if (m > 0) {
+                U = U + (a1 * vv[m] + dl * S);
+                if (m > 1) S = S + vv[m];
+            }
+        }
+    } else {
+        UNROLL for (m = 0; m < 10; ++m) {
+            v2 g = cs[2 * m + 1] - ta[0] * vv[m];
+            if (m < 9) {
+                v2 g2 = ta[1] * vv[m + 1];
+                UNROLL for (k = m + 2; k < 10; ++k) {
+                    if ((k - m) & 1) g2 += ta[k - m] * vv[k];
+                    else g -= ta[k - m] * vv[k];
+                }
+                g -= g2;
+            }
+            T1_ROWS(m, g);
+        }
+    }
+#undef T1_ROWS
+}
+
+/* t0 and t1 into T0 / T1. With `fast` (sc_g2 = 0, sc_g1 = 1, no su bias,
+   T0 = xn + 40 and T1 = xn, where those rows of the new state are exactly
+   t0 and t1) it also writes every other row of xn as it goes. */
+static inline __attribute__((always_inline)) void iterate_body(const fom_float *restrict state, fom_float sc_taux, fom_float sc_tth, fom_float sc_eta, fom_float sc_d1, fom_float sc_m1, fom_float sc_d2, fom_float sc_m2, const fom_float *restrict bias_t0, const fom_float *restrict bias_t1, const fom_float *restrict lo0, const fom_float *restrict lo1, const fom_float *restrict hi1, fom_float *T0, fom_float *T1, fom_float *xn, const int fast, const int lin) {
+    v2 cs[20], vv[10];
+    fom_int j;
+    cprod(state + 360, cs);
+    t0_part(state, sc_taux, sc_tth, sc_d1, sc_d2, bias_t0, lo0, T0, vv, xn, fast, lin);
+    t1_part(state, T0, vv, cs, sc_eta, sc_d1, sc_m1, sc_d2, sc_m2, bias_t1, lo1, hi1, T1, xn, fast, lin);
+    if (fast) {
+        UNROLL for (j = 0; j < 40; j += 2) ST(xn + 320 + j, LD(state + j));
+    }
+}
+
+void fom_step_t0(const fom_float *state, fom_float sc_taux, fom_float sc_tth, const fom_float *bias, const FomAct_t0 *act, fom_float *out) {
+    v2 vv[10];
+    if (!atab_ready) atab_fill();
+    if (atab_lin) t0_part(state, sc_taux, sc_tth, 0.0, 0.0, bias, act->lo, out, vv, 0, 0, 1);
+    else t0_part(state, sc_taux, sc_tth, 0.0, 0.0, bias, act->lo, out, vv, 0, 0, 0);
+}
+
+void fom_step_t1(const fom_float *state, const fom_float *t_t0, fom_float sc_eta, const fom_float *bias, const FomAct_t1 *act, fom_float *out) {
+    v2 cs[20], vv[10];
+    fom_int k;
+    if (!ctab_ready) ctab_fill();
+    cprod(state + 360, cs);
+    UNROLL for (k = 0; k < 10; ++k) vv[k] = SWAP(LD(t_t0 + 80 + 2 * k) - LD(t_t0 + 100 + 2 * k));
+    if (ctab_lin) t1_part(state, t_t0, vv, cs, sc_eta, 0.0, 0.0, 0.0, 0.0, bias, act->lo, act->hi, out, 0, 0, 1);
+    else t1_part(state, t_t0, vv, cs, sc_eta, 0.0, 0.0, 0.0, 0.0, bias, act->lo, act->hi, out, 0, 0, 0);
+}
+
 void fom_iterate(const fom_float *state, fom_float sc_taux, fom_float sc_tth, fom_float sc_eta, fom_float sc_d1, fom_float sc_g2, fom_float sc_m1, fom_float sc_d2, fom_float sc_g1, fom_float sc_m2, const fom_float *bias_t0, const fom_float *bias_t1, const fom_float *bias_su, const FomAct_t0 *act_t0, const FomAct_t1 *act_t1, fom_float *t_t0, fom_float *t_t1, fom_float *new_state) {
-    fom_step_t0(state, sc_taux, sc_tth, bias_t0, act_t0, t_t0);
-    fom_step_t1(state, t_t0, sc_eta, bias_t1, act_t1, t_t1);
-    fom_step_su(state, t_t0, t_t1, sc_d1, sc_g2, sc_m1, sc_d2, sc_g1, sc_m2, bias_su, new_state);
-}
+    fom_int j;
+    if (!ctab_ready) ctab_fill();
+    if (!atab_ready) atab_fill();
+    /* bias_su == NULL means a zero su bias (su has no constant or
+       parameter terms). The fast path also needs the temporaries to be the
+       t0 / t1 rows of new_state, which the solver arranges. */
+    if (bias_su == 0 && sc_g2 == 0.0 && sc_g1 == 1.0 && t_t0 == new_state + 40 && t_t1 == new_state) {
+        if (atab_lin && ctab_lin)
+            iterate_body(state, sc_taux, sc_tth, sc_eta, sc_d1, sc_m1, sc_d2, sc_m2, bias_t0, bias_t1, act_t0->lo, act_t1->lo, act_t1->hi, t_t0, t_t1, new_state, 1, 1);
+        else
+            iterate_body(state, sc_taux, sc_tth, sc_eta, sc_d1, sc_m1, sc_d2, sc_m2, bias_t0, bias_t1, act_t0->lo, act_t1->lo, act_t1->hi, t_t0, t_t1, new_state, 1, 0);
+        return;
+    }
+    {
+        fom_float T0[120], T1[40];
+        const fom_float *s2 = state + 160, *s3 = state + 200;
+        fom_float *xn = new_state;
+        iterate_body(state, sc_taux, sc_tth, sc_eta, sc_d1, sc_m1, sc_d2, sc_m2, bias_t0, bias_t1, act_t0->lo, act_t1->lo, act_t1->hi, T0, T1, 0, 0, 0);
+        for (j = 0; j < 120; ++j) t_t0[j] = T0[j];
+        for (j = 0; j < 40; ++j) t_t1[j] = T1[j];
+        for (j = 0; j < 40; ++j) {
+            const fom_float y = T1[j], p = s2[j];
+            xn[j] = sc_g2 * p + sc_g1 * y;
+            xn[160 + j] = sc_d1 * p + sc_d2 * y;
+            xn[320 + j] = state[j];
+            xn[360 + j] = sc_m1 * p + sc_m2 * y;
+        }
+        for (j = 0; j < 120; ++j) {
+            const fom_float y = T0[j], p = s3[j];
+            xn[40 + j] = sc_g2 * p + sc_g1 * y;
+            xn[200 + j] = sc_d1 * p + sc_d2 * y;
+        }
+        if (bias_su)
+            for (j = 0; j < 400; ++j) xn[j] += bias_su[j];
+    }
+}
```

## Agent notes

```
### Round 1

**Whole-solve time:** 1095.6 µs initially, about 43 µs final (A/B of prebuilt binaries, 42.5–43.2 µs). A plain `make bench` reads 44.9–46.3 µs. Overall speedup is about 25×.

**Final profile breakdown** (`make profile`; each region is timed every iteration, so timer overhead inflates the total to 110 µs and the small regions most):
| region | total_us | % |
|---|---|---|
| step_t1 (C product, Lᵀ Toeplitz, clip) | 141042 | 39.1 |
| step_t0 (A0 w, L Toeplitz, relu) | 92982 | 25.8 |
| su_rows (s2/s5 averages, s4 copy) | 83194 | 23.1 |
| fold (deferred s3 average) | 26558 | 7.4 |
| bias (cache refill) | 10738 | 3.0 |
| iterate (restart and last iterations) | 5801 | 1.6 |

**Optimizations applied:**
1. **Specialized `fom_iterate`, `fom_step_t0` and `fom_step_t1`** (1095 → 97 µs). These replace the generic CSR loops over 400 rows × 9 members. Structure used:
   - `t0_state__d_tth = -t0_state__d_taux`, so the t0 product is `A0 w` with `w = taux*s0 - tth*s4`.
   - `A0 = [I; -I; L; -L]`.
   - `t1_t_t0__d_eta = -A0ᵀ`, so `D t0 = -(t0[0:40] - t0[40:80] + Lᵀ v)`.
   - All selection products have unit coefficients.
2. **Toeplitz L.** L is a 10-tap block Toeplitz matrix: row 2k+r is `Σ a_{k-m}(w[4m+3-r] - w[4m+1-r])`. So `L w` and `Lᵀ v` cost 55 paired FMAs each instead of 220 sparse ones. The taps are read from each routine's own matrix data.
3. **Block-diagonal C** (97 → 90 µs). The dense 40×40 `t1_state__d_eta` block is symmetric and invariant under swapping s and swapping c (index 4a+2s+c). In the sum/difference basis it splits into three 10×10 blocks: P, M, and K, with K applied twice. That is 400 FMAs instead of 1600. The tables are derived from the matrix data on first call into static scratch in `fom_ops.c`.
4. **Explicit 2-wide vectors** (90 → 67 µs). `vector_size(16)` pairs, NaN-propagating `fmax`/`fmin` for relu and clip, and `restrict` on the inputs.
5. **Fast path writing t0/t1 into the new state** (67 → 57 µs). When g2 = 0 and g1 = 1 (63 of every 64 iterations), the s0 and s1 rows of the new state are exactly t1 and t0. The solver passes `t_t1 = xn` and `t_t0 = xn + 40`. The su rows are fused into the t0/t1 loops, and the loops are fully unrolled.
6. **Bias cache** (57 → 51 µs). All schedules repeat every 64 iterations, so `tau*b0` and `eta*b1` are cached per slot (it mod 64), keyed on the exact (tau, eta) and a per-solve generation counter.
7. **Deferred s3 average** (49 → 46 µs). The running average `s3 <- d1 s3 + d2 t0` is read only by restarts and at the end. The solver now runs the step routines plus the su shift rows itself, over a ring of state buffers. It folds s3 once per group, `s3 = (Π d1) s3 + Σ cᵢ t0ᵢ`, from the t0 history still in the ring's s1 rows. Restarts and the last iteration run the full `fom_iterate`, so the residual sees whole states.
8. **Fold group size G = 4** (46 → 43 µs). A sweep found G = 4 about 8% faster than G = 8; G = 5 and G = 16 were slower. Timing was sensitive to the ring layout.
9. **Smaller items:** C-transform lane tricks and an `fmul`-initialized accumulator (about 1–2%).

**Tried with no gain (under 1%):** splitting the Toeplitz FMA chains, a scalar K table, interleaved P/M/K tables with taps held in locals, `memcpy` for the s4 copy, an unrolled fold, and padding the ring for G = 4.

Blocked by rules:
- Store s5 (and the state) in the P/M/K sum/difference basis, to drop the per-iteration basis transforms | state layout and headers are fixed, and verify compares states in the spec layout | ~5%
- Overlap the ring buffers so a state's s4 rows alias the previous state's s0 rows, removing the s4 copy | the fixed contiguous state layout of the step routines (the overlap also clobbers the t0 history) | ~3%
- Compute the C product before the t0 step in the solver's fast path (memory reports ~5% from this ordering) | `fom_step_t1` must contain the C product, and the solver may not compute products | ~3–5%
- Keep the 80 KB bias cache in the workspace instead of a file-static array | `fom_workspace.h` is fixed | 0 (embedded hygiene only)
- Precompute the derived P/M/K tables and taps in `fom_setup` instead of lazily inside the routines | the solver may not read `FOM_MAT_*` data and `fom_ops.c` has no setup hook | negligible (one branch per call)

### Round 2

**Whole-solve mean_us:** 44.101 initial (listed; about 42.6 when re-measured this session) → about 37.6 final (alternating A/B of prebuilt binaries; a single `make bench` reads 38.6 to 40.2). Speedup for this round is about 1.13x against the listed figure (about 1.11x A/B), and about 29.8x overall against the 1122 us translation.

**Final profile (`make profile`, hottest first; the timer overhead of about 25 ns per region inflates the small regions):**
- step_t1: 115221 us, 39.2%
- step_t0: 72583 us, 24.7%
- su_rows: 65407 us, 22.2%
- fold: 26483 us, 9.0%
- bias: 9591 us, 3.3%
- iterate: 4940 us, 1.7%

**Optimizations applied:**
- **Affine Toeplitz taps, applied with running sums (about -5%).**
  - Structure: the 10 taps of L (in t0_state__d_taux and t1_t_t0__d_eta) satisfy a_j = a_1 + (j-1)Δ for j ≥ 1, to about 6e-16 relative. These are the positions of a double integrator.
  - L w (in t0) is now a prefix recurrence: lw_k = a0 e_k + U_k, with U_{k+1} = U_k + a1 e_k + Δ S_k.
  - L^T v (in t1) is the matching suffix recurrence. t1's rows are emitted in reverse block order, fused with it.
  - This cuts about 64 to about 42 FP ops per product. The taps and Δ are read from the matrix data at first use.
  - The check is done against a tolerance computed from the data (largest tap × 0.5^46), so no new float literal is needed. If the check fails, the dense taps are used.
  - The affine/dense choice is a compile-time template parameter with one runtime branch per step call. A single runtime branch inside the kernel caused spills and was about 20% slower.
- **L^T v folded into the K output of the C·s5 product (about -3%).**
  - Structure: rows 4m..4m+3 of C s5 + D t0 equal (h - g, h + g) + (l - u), with g = z - f.
  - So f = (L^T v)_m is subtracted straight from cprod's K-part z, and t1 no longer forms the ±z and ±f terms separately. That saves about 20 ops per iteration.
- **Padded state ring (about -2%).** The state buffers are padded to 408 doubles and 64-byte aligned, so they no longer sit at a 4 KiB-aliasing stride from each other.
- **Shorter FMA chains in the dense Toeplitz fallback** (even/odd partial sums, no zero-initialised accumulators): about -1%, and this path is still used by the restart iterations.

**Tried and rejected (each verified, each slower or neutral in A/B):**
- Fold changes: precomputed span weights (cached or not), group sizes 1, 2, 3, 5, 6, 8 and 16, running the fold after the steps, and an unrolled fold.
- Running every iteration through the fused `fom_iterate` (with or without reordering cprod).
- Computing s2/s5 as s2 + d2·(y - s2); the guard holds only 62% of the time, so the branch mispredicts.
- Explicit 2-wide su rows, a scalar K table using by-element FMAs (half the table loads; neutral), t0 rows fused with w, and putting the L-rows first.
- A 1-slot bias cache: +16%.

Blocked by rules:
- Pass scaled bias terms (τ·b0, η·b1) to the step routines instead of materialised bias vectors, so the 82 KB, 64-slot bias cache can go | the fixed `fom_step_*` signatures in `include/fom_ops.h` | unknown (bias costs about 3%, plus L1 pressure)
- Let `fom_iterate` skip writing the s3 rows (or take a flag saying so), so the fused single-pass iteration can keep the deferred, grouped s3 fold. The fused path would save the recomputed differences and the T0 reloads, about 40 ops and 60 loads per iteration | `fom_iterate` must write the complete new state, and its signature is fixed | about 3 to 5%
- Let `fom_step_t1` also return C·s5_next, so the next iteration's cprod is spread through the current iteration's tail | the fixed step signatures and outputs | unknown

### Round 3

**Result: no change this round.** The editable sources are restored byte-for-byte to the round-2 solver. Every change I tried was slower, the same speed, or unsafe.

- Initial `mean_us`: 40.078 (pipeline figure). Local reruns today give 38.1 to 38.7.
- Final `mean_us`: 38.3 to 38.7 locally, from the same unchanged code.
- Speedup this round: 1.00x. Overall against the translated solver (1122.036 us): 28.0x on the pipeline figure, about 29x on today's local runs.

Final profile (`make profile`; the profile build includes timer overhead, so only the percentages are meaningful):

| region | time | share |
|---|---|---|
| step_t1 | 114883 us | 39.2% |
| step_t0 | 72501 us | 24.7% |
| su_rows | 64717 us | 22.1% |
| fold | 27105 us | 9.2% |
| bias | 9351 us | 3.2% |
| iterate | 4804 us | 1.6% |

Where the time goes (found by timing-only ablations, all reverted):
- The dense C·s5 product inside step_t1 costs about 9.3 us. Removing it brings the solve to about 29 us.
- The s3 fold costs about 3.3 us.
- The su shift rows cost about 4 us.
- The rest is the t0 step and the t1 step without the dense product.
- The loop is mostly limited by how many FP instructions it can issue per cycle, not by the serial Toeplitz chains. Breaking the dependency in the L / Lᵀ chains saved only 0.6 us.

What I tried and dropped:
- **Structure of C.** Each 4×4 block of the 40×40 matrix C is a symmetric circulant with first row (α, β, γ, β). The existing P/M/K split (two 10×10 matrices, plus one applied to two vectors) is exactly its DFT diagonalization. Those 10×10 matrices are dense and full rank, with no linear relation between them. The pieces are only nearly semiseparable, and for N=10 that would not need fewer multiply-adds anyway. C is also not R + Lᵀ Q L for the Toeplitz L. So 200 vector FMAs per iteration is the floor for this product.
- **Column-major C tables with by-element FMAs.** This cuts table loads from 200 to 150 per iteration and still passes verify, but timing was unchanged. The product is limited by its FMAs, not its loads.
- **The fused `fom_iterate` fast path** (s3 updated every iteration): 40 us, slower. Without the s3 update it ties the separate step calls, so call order is not the issue.
- **Fold group sizes 6, 8, 12 and 16, an unrolled fold, and `restrict` versions of the fold and su loops:** no gain, and larger groups were slower.
- **Using the general Toeplitz path instead of the affine (prefix-sum) one,** for t0, t1 or both: 1 to 2 us slower.
- **Moving the t0 rows 0..79 ahead of the Toeplitz chain** to reduce register pressure: 1 us slower, because the critical chain needs to come first in program order.
- **Rewriting the su rows as p + d2·(y − p):** this relies on d1 + d2 = 1 and m1 + m2 = 1. The first holds exactly in the schedule data, but m1 + m2 differs from 1 by an ulp in 56 entries, so I didn't apply it.

Optimizations in the kept solver (from earlier rounds; it still has the `FOM_PROF_BEGIN/END` markers around step_t0, step_t1, su_rows, fold, bias and iterate):
- **Circulant / DFT split of C** (the matrix in `t1_state__d_eta`, applied to s5): the 40×40 product becomes four 10×10 products.
- **Shared Toeplitz block:** `t0_state__d_tth = -t0_state__d_taux`, and `t1_t_t0__d_eta = -A0ᵀ` with A0 = [I; −I; L; −L]. L is a 10-tap block Toeplitz whose taps are affine past the first, so L and Lᵀ are applied with running prefix and suffix sums.
- **Selection rows and shifts written straight into a ring of state buffers:** t0 goes into the s1 rows and t1 into the s0 rows, with no copies.
- **s3 (the running average of t0) folded once every 4 iterations** from the t0 history still held in the ring.
- **Bias cache:** bias_t0 = tau·b0 and bias_t1 = eta·b1, because t0_p_p1 and t1_p_p0 are zero matrices. Bias vectors are reused by schedule slot, since the schedule restarts every 64 iterations.

Blocked by rules:
- Change the step/iterate interface so fom_iterate can skip the s3 rows (the fold keeps them) and the fused one-pass iteration can be used | the fixed signatures in include/ and the rule that fom_iterate must compute the whole new state | about 0 us (ties the separate step calls; would only enable later fusion)
- Pass b0 and tau into the t0 step instead of a materialized bias vector, which removes the bias cache | the fixed fom_step_t0 signature (bias is a vector) | about 1 us
- Run the C·s5 product in float32 or on the SME matrix unit | float64 data type and fixed build flags | unknown
```
