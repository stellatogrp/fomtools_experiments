# Optimization report

## Timing

```
translated (emitted)     125.3 us/solve
optimized                 30.2 us/solve
speedup                  4.15x
```

Optimization outcome: **kept (re-verified correct)**.

## Rounds

| round | before (us) | after (us) | gain | kept | agent (s) | outcome |
|---:|---:|---:|---:|:---:|---:|---|
| 1 | 125.343 | 59.167 | 52.8% | yes | 132 | continued |
| 2 | 59.113 | 37.704 | 36.2% | yes | 156 | continued |
| 3 | 37.814 | 30.204 | 20.1% | yes | 89 | continued |
| 4 | - | - | - | no | 56 | the round left the sources unchanged |

## Kernel checks

Each `fom_apply_<name>` in the shipped `fom_ops.c`, checked by `make verify` against the package's reference over the same `fom_data.c` matrix. *basis* probes every unit vector, which recovers the routine's matrix column by column (an exact check), plus random vectors; *random* uses random vectors only (the matrix is over the probe budget). A `fused_<block>` row is a `fom_fused_<block>`, checked against the sum of its members: *basis* there probes every unit vector of each source with every schedule value at 0 and with each alone at 1, which recovers every member exactly.

| matrix | shape | nnz | rewritten | check | probes | max error |
|---|---|---|---|---|---|---|
| `t0_state` | 600x650 | 1800 | no | basis | 652 | 0.000e+00 |
| `t1_state` | 20x650 | 620 | no | basis | 652 | 0.000e+00 |
| `t1_state__d_xi` | 20x650 | 600 | no | basis | 652 | 0.000e+00 |
| `t1_t_t0` | 20x600 | 600 | no | basis | 602 | 0.000e+00 |
| `t1_t_t0__d_xi` | 20x600 | 600 | no | basis | 602 | 0.000e+00 |
| `su_state` | 650x650 | 630 | no | basis | 652 | 0.000e+00 |
| `su_state__d_xi` | 650x650 | 600 | no | basis | 652 | 0.000e+00 |
| `su_t_t0` | 650x600 | 1200 | no | basis | 602 | 0.000e+00 |
| `su_t_t0__d_xi` | 650x600 | 600 | no | basis | 602 | 0.000e+00 |
| `su_t_t1` | 650x20 | 20 | no | basis | 22 | 0.000e+00 |
| `su_p_p0` | 650x30 | 30 | no | basis | 32 | 0.000e+00 |
| `fused_t1` | 20x1250 | 2420 | no | basis | 2502 | 0.000e+00 |
| `fused_su` | 650x1270 | 3050 | no | basis | 2542 | 0.000e+00 |
| `step_t0` | 600x650 | 1800 | yes | basis | 656 | 8.882e-16 |
| `step_t1` | 20x1250 | 2420 | yes | basis | 2506 | 3.553e-15 |
| `step_su` | 650x1270 | 3050 | no | basis | 2542 | 0.000e+00 |
| `fused_su__rest` | 650x1270 | 3050 | no | basis | 2542 | 0.000e+00 |
| `step_su__rest` | 650x1270 | 3050 | yes | basis | 2542 | 1.832e-15 |
| `iterate` | 650x650 | 7270 | yes | basis | 1956 | 2.665e-15 |

### Rule violations

_None: the collected rewrite passed the rules and was kept._

## Blocked by rules

Optimizations the agent considered but did not make because a rule forbade them, in its own words.

| round | idea | rule | expected gain |
|---|---|---|---|
| 1 | Merge the two t1 products from t_t0 into `(1+xi)*(t1_t_t0 @ t0)` because `t1_t_t0 == t1_t_t0__d_xi` (likely the same for su_t_t0 on rest rows) | none (fused/step routines may hardcode literals; I ran out of time to check exact equality and inline the values) | ~10% of t1 |
| 1 | Precompute a solver-side layout of t0's 3-per-row structure (shared column 600 and its constant coefficient) at setup | solver must not read FOM_MAT data, and setup cannot probe matrices | small, unknown |
| 1 | none other |  |  |
| 2 | Precomputing transposed copies of the su rest and t1 value arrays at setup, for unit-stride vector loads | `fom_setup` and `src/fom_solver.c` may not read `FOM_MAT_*` data, and all `src/fom_ops.c` functions except the entry points must be `static` (`static const` tables inside the routine are allowed, but they mean regenerating the literal coefficients) | small, about 1 to 2 us |
| 2 | Letting the compiler reassociate and vectorize the reductions with fast-math flags | the Makefile and build flags are fixed | unknown, small |
| 3 | Hardware vector instructions (AVX2/NEON FMA) through build flags such as `-march=native` | Makefile and build flags are fixed | unknown, maybe 10–20% |
| 3 | Skip adding `bias_su` on selection rows 0..619, since it is zero there in the solver (p0 only reaches rows 620+) | `fom_iterate` must be right for any bias, and `fom_update_params` may not probe the matrix | under 2% |
| 3 | Do the repacking in `fom_setup` instead of a first-call check in `fom_iterate` | `src/fom_solver.c` may not read the `FOM_MAT_` data of matrices it multiplies | negligible |
| 4 | Fold the t0 bias into the clip bounds (`clip(b+y,l,h) = b + clip(y,l-b,h-b)`), precomputed once per solve | `fom_iterate` must be right for any bias and activation passed on each call, and `fom_solver.c` may not read matrix or activation data in a way that replaces the checked routines | ~3-5% |
| 4 | Repack the coefficient tables in `fom_setup` instead of behind a lazy first-call init flag | `fom_solver.c` must not read `FOM_MAT_*` data, and setup may not call apply routines | <1% |

## What the agent changed

```diff
--- src/fom_solver.c (translated)
+++ src/fom_solver.c (optimized)
@@ -2,6 +2,7 @@
 #include "fom_solver.h"
 #include "fom_data.h"
 #include "fom_ops.h"
+#include "fom_profile.h"
 
 void fom_set_default_settings(FomSettings *settings) {
     settings->max_iter = FOM_MAX_ITER;
@@ -27,11 +28,9 @@
     fom_float *restrict x = work->state;
     fom_float *restrict xn = work->new_state;
     fom_float *restrict bias = work->acc;
-    fom_float *restrict t0 = work->t0;
-    fom_float *restrict t1 = work->t1;
     const fom_int max_iter = work->settings.max_iter;
     fom_float res = 0;
-    fom_int it;
+    fom_int it, i;
 
     /* Hoisted parameter term of the state update. */
     memset(bias, 0, sizeof(fom_float) * FOM_STATE_DIM);
@@ -39,33 +38,31 @@
 
     for (it = 0; it < max_iter; ++it) {
         const fom_float xi = fom_sched_xi(it);
-        fom_int i;
-        fom_step_t0(x, FOM_MAT_t0_c, &FOM_ACT_t0, t0);
-        fom_step_t1(x, t0, xi, FOM_MAT_t1_c, &FOM_ACT_t1, t1);
-        fom_step_su__rest(x, t0, t1, xi, bias, xn);
-        /* Selection rows: 0..599 copy t0, 600..619 copy t1. */
-        res = 0;
-        for (i = 0; i < FOM_T0_DIM; ++i) {
-            fom_float v = t0[i], d = v - x[i];
-            d = d < 0 ? -d : d;
-            res = d > res ? d : res;
-            x[i] = v;
+        fom_float *tmp;
+        /* Selection rows of su: rows 0..599 are t0 and 600..619 are t1, so
+           both temporaries are written straight into the new state. */
+        FOM_PROF_BEGIN(iterate);
+        fom_iterate(x, xi, FOM_MAT_t0_c, FOM_MAT_t1_c, bias, &FOM_ACT_t0, &FOM_ACT_t1,
+                    xn, xn + FOM_T0_DIM, xn);
+        FOM_PROF_END(iterate);
+        /* Only the final residual is reported (no tolerance check). */
+        if (it == max_iter - 1) {
+            FOM_PROF_BEGIN(residual);
+            res = 0;
+            for (i = 0; i < FOM_STATE_DIM; ++i) {
+                fom_float d = xn[i] - x[i];
+                d = d < 0 ? -d : d;
+                res = d > res ? d : res;
+            }
+            FOM_PROF_END(residual);
         }
-        for (i = 0; i < FOM_T1_DIM; ++i) {
-            fom_float v = t1[i], d = v - x[FOM_T0_DIM + i];
-            d = d < 0 ? -d : d;
-            res = d > res ? d : res;
-            x[FOM_T0_DIM + i] = v;
-        }
-        for (i = FOM_T0_DIM + FOM_T1_DIM; i < FOM_STATE_DIM; ++i) {
-            fom_float v = xn[i], d = v - x[i];
-            d = d < 0 ? -d : d;
-            res = d > res ? d : res;
-            x[i] = v;
-        }
+        tmp = x; x = xn; xn = tmp;
     }
 
-    memcpy(work->solution.x, x, sizeof(work->solution.x));
+    if (x != work->state) memcpy(work->state, x, sizeof(work->state));
+    memcpy(work->t0, work->state, sizeof(work->t0));
+    memcpy(work->t1, work->state + FOM_T0_DIM, sizeof(work->t1));
+    memcpy(work->solution.x, work->state, sizeof(work->solution.x));
     work->solution.iter = it;
     work->solution.residual = res;
     work->solution.status = FOM_MAX_ITER_REACHED;
--- src/fom_ops.c (translated)
+++ src/fom_ops.c (optimized)
@@ -142,39 +142,61 @@
 }
 
 void fom_step_t0(const fom_float *state, const fom_float *bias, const FomAct_t0 *act, fom_float *out) {
-    fom_int i, r, p;
-    for (r = 0; r < 600; ++r) {
-        fom_float s, v;
-        i = r;
-        v = bias[i];
-        s = 0.0;
-        for (p = FOM_MAT_t0_state_ROWPTR[i]; p < FOM_MAT_t0_state_ROWPTR[i + 1]; ++p) s += FOM_MAT_t0_state_VALS[p] * state[FOM_MAT_t0_state_COLIDX[p]];
-        v += s;
-        if (v < act->lo[i]) v = act->lo[i];
-        if (v > act->hi[i]) v = act->hi[i];
-        out[i] = v;
-    }
+    /* t0_state has exactly 3 nonzeros per row (ROWPTR[i] = 3 i). */
+    const fom_int *restrict ci = FOM_MAT_t0_state_COLIDX;
+    const fom_float *restrict va = FOM_MAT_t0_state_VALS;
+    const fom_float *restrict lo = act->lo, *restrict hi = act->hi;
+    fom_int i;
+    /* Fixed pattern (checked on every unit vector): row 30 j + k reads
+       state[30 j + k] (coefficient 1), state[600 + j] and state[620 + k].
+       No gathers, so the inner loop vectorizes. */
+    fom_int j, k;
+    (void)ci;
+    for (j = 0; j < 20; ++j) {
+        const fom_float xj = state[600 + j];
+        const fom_float *restrict xr = state + 30 * j;
+        const fom_float *restrict xk = state + 620;
+        const fom_float *restrict b = bias + 30 * j;
+        const fom_float *restrict l = lo + 30 * j, *restrict h = hi + 30 * j;
+        const fom_float *restrict v3 = va + 90 * j;
+        fom_float *restrict o = out + 30 * j;
+        for (k = 0; k < 30; ++k) {
+            fom_float v = b[k] + xr[k] + v3[3 * k + 1] * xj + v3[3 * k + 2] * xk[k];
+            v = v < l[k] ? l[k] : v;
+            v = v > h[k] ? h[k] : v;
+            o[k] = v;
+        }
+    }
+    (void)i;
 }
 
 void fom_step_t1(const fom_float *state, const fom_float *t_t0, fom_float sc_xi, const fom_float *bias, const FomAct_t1 *act, fom_float *out) {
-    fom_int i, r, p;
-    for (r = 0; r < 20; ++r) {
-        fom_float s, v;
-        i = r;
-        v = bias[i];
-        s = 0.0;
-        for (p = FOM_MAT_t1_state_ROWPTR[i]; p < FOM_MAT_t1_state_ROWPTR[i + 1]; ++p) s += FOM_MAT_t1_state_VALS[p] * state[FOM_MAT_t1_state_COLIDX[p]];
-        v += s;
-        s = 0.0;
-        for (p = FOM_MAT_t1_state__d_xi_ROWPTR[i]; p < FOM_MAT_t1_state__d_xi_ROWPTR[i + 1]; ++p) s += FOM_MAT_t1_state__d_xi_VALS[p] * state[FOM_MAT_t1_state__d_xi_COLIDX[p]];
-        v += sc_xi * s;
-        s = 0.0;
-        for (p = FOM_MAT_t1_t_t0_ROWPTR[i]; p < FOM_MAT_t1_t_t0_ROWPTR[i + 1]; ++p) s += FOM_MAT_t1_t_t0_VALS[p] * t_t0[FOM_MAT_t1_t_t0_COLIDX[p]];
-        v += s;
-        s = 0.0;
-        for (p = FOM_MAT_t1_t_t0__d_xi_ROWPTR[i]; p < FOM_MAT_t1_t_t0__d_xi_ROWPTR[i + 1]; ++p) s += FOM_MAT_t1_t_t0__d_xi_VALS[p] * t_t0[FOM_MAT_t1_t_t0__d_xi_COLIDX[p]];
-        v += sc_xi * s;
-        if (v < act->lo[i]) v = act->lo[i];
+    /* Fixed pattern (checked on every unit vector): row i of t1_state reads
+       columns 30 i .. 30 i + 29 then 600 + i; t1_state__d_xi, t1_t_t0 and
+       t1_t_t0__d_xi read the same 30 columns, and
+       t1_t_t0__d_xi = -t1_state__d_xi, so xi (D x + E t) = xi E (t - x). */
+    const fom_float *restrict lo = act->lo;
+    fom_int i, k;
+    for (i = 0; i < 20; ++i) {
+        const fom_float *restrict xb = state + 30 * i;
+        const fom_float *restrict tb = t_t0 + 30 * i;
+        const fom_float *restrict va = FOM_MAT_t1_state_VALS + FOM_MAT_t1_state_ROWPTR[i];
+        const fom_float *restrict wa = FOM_MAT_t1_t_t0_VALS + FOM_MAT_t1_t_t0_ROWPTR[i];
+        const fom_float *restrict vd = FOM_MAT_t1_t_t0__d_xi_VALS + FOM_MAT_t1_t_t0__d_xi_ROWPTR[i];
+        fom_float sa[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0}, da[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
+        fom_float s, ud, v;
+        fom_int m;
+        /* Six independent partial sums break the serial FMA chain. */
+        for (k = 0; k < 30; k += 6) {
+            for (m = 0; m < 6; ++m) {
+                sa[m] += va[k + m] * xb[k + m] + wa[k + m] * tb[k + m];
+                da[m] += vd[k + m] * (tb[k + m] - xb[k + m]);
+            }
+        }
+        s = (sa[0] + sa[1]) + (sa[2] + sa[3]) + (sa[4] + sa[5]);
+        ud = (da[0] + da[1]) + (da[2] + da[3]) + (da[4] + da[5]);
+        v = bias[i] + va[30] * state[600 + i] + s + sc_xi * ud;
+        if (v < lo[i]) v = lo[i];
         out[i] = v;
     }
 }
@@ -228,32 +250,97 @@
 }
 
 void fom_step_su__rest(const fom_float *state, const fom_float *t_t0, const fom_float *t_t1, fom_float sc_xi, const fom_float *bias, fom_float *out) {
-    fom_int i, r, p;
-    for (r = 0; r < FOM_REST_su_N; ++r) {
-        fom_float s, v;
-        i = FOM_REST_su_ROWS[r];
-        v = bias[i];
-        s = 0.0;
-        for (p = FOM_MAT_su_state_ROWPTR[i]; p < FOM_MAT_su_state_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_state_VALS[p] * state[FOM_MAT_su_state_COLIDX[p]];
-        v += s;
-        s = 0.0;
-        for (p = FOM_MAT_su_state__d_xi_ROWPTR[i]; p < FOM_MAT_su_state__d_xi_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_state__d_xi_VALS[p] * state[FOM_MAT_su_state__d_xi_COLIDX[p]];
-        v += sc_xi * s;
-        s = 0.0;
-        for (p = FOM_MAT_su_t_t0_ROWPTR[i]; p < FOM_MAT_su_t_t0_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t0_VALS[p] * t_t0[FOM_MAT_su_t_t0_COLIDX[p]];
-        v += s;
-        s = 0.0;
-        for (p = FOM_MAT_su_t_t0__d_xi_ROWPTR[i]; p < FOM_MAT_su_t_t0__d_xi_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t0__d_xi_VALS[p] * t_t0[FOM_MAT_su_t_t0__d_xi_COLIDX[p]];
-        v += sc_xi * s;
-        s = 0.0;
-        for (p = FOM_MAT_su_t_t1_ROWPTR[i]; p < FOM_MAT_su_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t1_VALS[p] * t_t1[FOM_MAT_su_t_t1_COLIDX[p]];
-        v += s;
-        out[i] = v;
-    }
+    /* Fixed pattern (checked on every unit vector): rest rows are 620 + k,
+       k < 30. Row 620 + k of su_state reads columns 30 j + k (j < 20) then
+       620 + k; su_state__d_xi, su_t_t0, su_t_t0__d_xi read the same 30 j + k
+       and su_state__d_xi = -su_t_t0__d_xi there, so
+       xi (D x + E t) = xi E (t - x). su_t_t1 is empty on these rows.
+       Loop j outside so the 30 rows run as contiguous vectors. */
+    const fom_float *restrict S = FOM_MAT_su_state_VALS + FOM_MAT_su_state_ROWPTR[620];
+    const fom_float *restrict T = FOM_MAT_su_t_t0_VALS + FOM_MAT_su_t_t0_ROWPTR[620];
+    const fom_float *restrict E = FOM_MAT_su_t_t0__d_xi_VALS + FOM_MAT_su_t_t0__d_xi_ROWPTR[620];
+    fom_float acc[30], ud[30];
+    fom_int j, k;
+    (void)t_t1;
+    for (k = 0; k < 30; ++k) { acc[k] = bias[620 + k] + S[21 * k + 20] * state[620 + k]; ud[k] = 0.0; }
+    for (j = 0; j < 20; ++j) {
+        const fom_float *restrict xr = state + 30 * j;
+        const fom_float *restrict tr = t_t0 + 30 * j;
+        for (k = 0; k < 30; ++k) {
+            acc[k] += S[21 * k + j] * xr[k] + T[20 * k + j] * tr[k];
+            ud[k] += E[20 * k + j] * (tr[k] - xr[k]);
+        }
+    }
+    for (k = 0; k < 30; ++k) out[620 + k] = acc[k] + sc_xi * ud[k];
 }
 
 void fom_iterate(const fom_float *state, fom_float sc_xi, const fom_float *bias_t0, const fom_float *bias_t1, const fom_float *bias_su, const FomAct_t0 *act_t0, const FomAct_t1 *act_t1, fom_float *t_t0, fom_float *t_t1, fom_float *new_state) {
-    fom_step_t0(state, bias_t0, act_t0, t_t0);
-    fom_step_t1(state, t_t0, sc_xi, bias_t1, act_t1, t_t1);
-    fom_step_su(state, t_t0, t_t1, sc_xi, bias_su, new_state);
-}
+    /* One pass over the 20 blocks j (30 entries each): t0 block j, then
+       t1[j] (reads only state/t0 block j), then the su rest rows 620 + k
+       accumulate block j. Same fixed patterns as the step routines above;
+       the strided coefficients are repacked once into contiguous tables.
+       t_t0 / t_t1 may alias new_state (selection rows), so no restrict. */
+    static fom_float A1[600], A2[600], ST[600], TT[600], ET[600];
+    static int init = 0;
+    const fom_float *restrict lo0 = act_t0->lo, *restrict hi0 = act_t0->hi;
+    const fom_float *restrict lo1 = act_t1->lo;
+    const fom_float *restrict S = FOM_MAT_su_state_VALS + FOM_MAT_su_state_ROWPTR[620];
+    fom_float acc[30], ud[30];
+    fom_int j, k;
+    if (!init) {
+        const fom_float *T = FOM_MAT_su_t_t0_VALS + FOM_MAT_su_t_t0_ROWPTR[620];
+        const fom_float *E = FOM_MAT_su_t_t0__d_xi_VALS + FOM_MAT_su_t_t0__d_xi_ROWPTR[620];
+        for (j = 0; j < 20; ++j)
+            for (k = 0; k < 30; ++k) {
+                A1[30 * j + k] = FOM_MAT_t0_state_VALS[3 * (30 * j + k) + 1];
+                A2[30 * j + k] = FOM_MAT_t0_state_VALS[3 * (30 * j + k) + 2];
+                ST[30 * j + k] = S[21 * k + j];
+                TT[30 * j + k] = T[20 * k + j];
+                ET[30 * j + k] = E[20 * k + j];
+            }
+        init = 1;
+    }
+    for (k = 0; k < 30; ++k) { acc[k] = bias_su[620 + k] + S[21 * k + 20] * state[620 + k]; ud[k] = 0.0; }
+    for (j = 0; j < 20; ++j) {
+        const fom_float *restrict xr = state + 30 * j;
+        const fom_float *restrict xk = state + 620;
+        const fom_float xj = state[600 + j];
+        const fom_float *restrict b = bias_t0 + 30 * j;
+        const fom_float *restrict l = lo0 + 30 * j, *restrict h = hi0 + 30 * j;
+        const fom_float *restrict a1 = A1 + 30 * j, *restrict a2 = A2 + 30 * j;
+        const fom_float *restrict st = ST + 30 * j, *restrict tt = TT + 30 * j, *restrict et = ET + 30 * j;
+        const fom_float *restrict va = FOM_MAT_t1_state_VALS + FOM_MAT_t1_state_ROWPTR[j];
+        const fom_float *restrict wa = FOM_MAT_t1_t_t0_VALS + FOM_MAT_t1_t_t0_ROWPTR[j];
+        const fom_float *restrict vd = FOM_MAT_t1_t_t0__d_xi_VALS + FOM_MAT_t1_t_t0__d_xi_ROWPTR[j];
+        fom_float tb[30];
+        fom_float sa[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0}, da[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
+        fom_float s, d, v;
+        fom_int m;
+        for (k = 0; k < 30; ++k) {
+            fom_float t = b[k] + xr[k] + a1[k] * xj + a2[k] * xk[k];
+            t = t < l[k] ? l[k] : t;
+            t = t > h[k] ? h[k] : t;
+            tb[k] = t;
+            acc[k] += st[k] * xr[k] + tt[k] * t;
+            ud[k] += et[k] * (t - xr[k]);
+        }
+        /* Six independent partial sums break the serial FMA chain. */
+        for (k = 0; k < 30; k += 6)
+            for (m = 0; m < 6; ++m) {
+                sa[m] += va[k + m] * xr[k + m] + wa[k + m] * tb[k + m];
+                da[m] += vd[k + m] * (tb[k + m] - xr[k + m]);
+            }
+        s = (sa[0] + sa[1]) + (sa[2] + sa[3]) + (sa[4] + sa[5]);
+        d = (da[0] + da[1]) + (da[2] + da[3]) + (da[4] + da[5]);
+        /* When t_t0 aliases new_state (solver selection rows), skip the
+           redundant copy; new_state is written last either way. */
+        if (t_t0 != new_state)
+            for (k = 0; k < 30; ++k) t_t0[30 * j + k] = tb[k];
+        for (k = 0; k < 30; ++k) new_state[30 * j + k] = bias_su[30 * j + k] + tb[k];
+        v = bias_t1[j] + va[30] * xj + s + sc_xi * d;
+        if (v < lo1[j]) v = lo1[j];
+        t_t1[j] = v;
+        new_state[600 + j] = bias_su[600 + j] + v;
+    }
+    for (k = 0; k < 30; ++k) new_state[620 + k] = acc[k] + sc_xi * ud[k];
+}
```

## Agent notes

```
### Round 1

**Whole solve (`make bench`, mean_us):** 126.19 at the start, 59.4 at the end (a second run gave 58.96). That is about **2.1x faster**.

**Final profile** (`make profile`; the profiled build reports 65.0 µs because the timers add their own cost):

| Region | Time (µs) | Share |
|---|---|---|
| t0 (clip step, 600 rows) | 119867 | 39.2% |
| su (30 rest rows of the state update) | 107713 | 35.2% |
| t1 (relu step, 20 rows) | 76559 | 25.0% |
| residual (last iteration only) | 1703 | 0.6% |

**Optimizations applied:**
1. **Shared sparsity patterns in t1 and su:** one index pass serves several matrices. This gave 126 → 86.6 µs.
   - In t1, each `t1_state` row has the same pattern as `t1_state__d_xi` plus one extra entry at the end (column 600+i). `t1_t_t0` and `t1_t_t0__d_xi` have identical patterns.
   - On su's 30 rest rows, `su_state__d_xi`, `su_t_t0` and `su_t_t0__d_xi` share one column pattern, and `su_state` is that pattern plus one extra entry. Since `state` and `t_t0` are read at the same column, one index load gives four multiply-adds.
   - The coefficients are still read from the CSR data, and the bias is folded into the same pass.
2. **Fixed row length in t0:** `t0_state` has exactly 3 nonzeros per row, so the kernel is fully unrolled with the clip done inline.
3. **Selection rows written straight into the new state, plus a pointer swap:** this gave 86.6 → 74.8 µs.
   - Rows 0..599 of su are copies of t0 and rows 600..619 are copies of t1. So `fom_step_t0` and `fom_step_t1` write directly into `new_state[0..619]`, and `fom_step_su__rest` fills only rows 620..649.
   - Swapping the `state`/`new_state` pointers removes the per-iteration copy pass.
   - Because termination is `max_iters` with no tolerance, the residual is only computed on the last iteration. `work->t0` and `work->t1` are filled once at the end.
4. **Contiguous-column fast path in t1:** this gave 74.8 → 59 µs.
   - Each t1 row reads 30 consecutive columns (30i..30i+29) of both `state` and `t_t0`. When a row's sorted, unique column indices span exactly its length, it is read as a dense block with two accumulators, with no index loads.
   - The block check is computed from the data, and the general sparse loop is kept for any other pattern.

Blocked by rules:
- Merge the two t1 products from t_t0 into `(1+xi)*(t1_t_t0 @ t0)` because `t1_t_t0 == t1_t_t0__d_xi` (likely the same for su_t_t0 on rest rows) | none (fused/step routines may hardcode literals; I ran out of time to check exact equality and inline the values) | ~10% of t1
- Precompute a solver-side layout of t0's 3-per-row structure (shared column 600 and its constant coefficient) at setup | solver must not read FOM_MAT data, and setup cannot probe matrices | small, unknown
- none other

### Round 2

**Whole-solve `mean_us`:** 125.343 us for the translated solver. This round started at 59.167 us and ends at about 38.5 us (3 `make bench` runs: 38.4 to 39.8). That is 1.54x this round and 3.26x overall. `make verify` prints `VERIFY OK`.

**Final profile (`make profile`), hottest first:**
- su: 78888 us, 38.5%
- t0: 62957 us, 30.7%
- t1: 61503 us, 30.0%
- residual: 1791 us, 0.9%

Each region now costs well under 1 us per call. The per-region timer overhead is a large share of these numbers, which is why `su` (only 30 rows left to compute) still ranks first.

**Optimizations applied:**
- **t0 without gathers.** `t0_state` has a fixed 20×30 grid pattern: row 30j+k reads `state[30j+k]` with coefficient 1, plus `state[600+j]` and `state[620+k]`. The kernel loops over j outside and k inside, so it does contiguous loads with no `COLIDX` gather and vectorizes together with the clip. It drops the unit coefficient. This is allowed because `fom_step_t0` is checked on every unit vector. Gain: about 56 to 45 us.
- **su rest rows: one fewer product.** On rows 620..649, `su_state__d_xi = -su_t_t0__d_xi`, with the same pattern and all columns below 600. So `xi*(D x + E t)` becomes `xi * E (t - x)`, which removes one product per entry. Gain: 59.2 to 55.7 us.
- **su rest rows: loop order swapped.** Row 620+k reads columns 30j+k (j < 20), plus `state[620+k]`. With j outside and k inside, the 30 rows update as contiguous vectors instead of 30 stride-30 gathers. `su_t_t1` is empty on these rows, so that term is skipped. Gain: about 45 to 41 us.
- **t1 hardcoded and merged.** Row i reads the contiguous columns 30i..30i+29, plus `600+i`. `t1_t_t0__d_xi = -t1_state__d_xi` on those columns, so the same `xi * E (t - x)` merge applies. The four members now take one pass with six independent partial sums. Gain: about 41 to 38.5 us, which is near the run-to-run noise.

The minus-sign relations hold to about 3e-16 in the data, not bit for bit. Verify's worst error stays at 9.3e-15 against a tolerance of 1e-6.

**Tried and reverted:** the same t1 merge done through a generic runtime-checked branch was slower (about 59 us), so I replaced it with the hardcoded version above.

Blocked by rules:
- Precomputing transposed copies of the su rest and t1 value arrays at setup, for unit-stride vector loads | `fom_setup` and `src/fom_solver.c` may not read `FOM_MAT_*` data, and all `src/fom_ops.c` functions except the entry points must be `static` (`static const` tables inside the routine are allowed, but they mean regenerating the literal coefficients) | small, about 1 to 2 us
- Letting the compiler reassociate and vectorize the reductions with fast-math flags | the Makefile and build flags are fixed | unknown, small

### Round 3

**Whole-solve time (`make bench` mean_us):**
- Start of this round: 37.704 us (3.32x over the translated solver's 125.343 us).
- Final: about 29.5 us. Five runs gave 28.3, 29.5, 29.6, 29.8 and 31.8 us.
- That is about 1.27x faster this round and about 4.25x overall.
- `make verify` prints `VERIFY OK`.

**Final profile (`make profile`), hottest first:**
| Region | Time | Share |
|---|---|---|
| iterate (`fom_iterate`: t0, t1 and su together) | 140983 us | 98.8% |
| residual | 1680 us | 1.2% |

The profile build's own `BENCH` line read mean_us=30.07.

**Optimizations applied:**
1. **One pass per iteration (`fom_iterate`).** The solver now makes a single `fom_iterate` call per iteration, not three separate step calls.
   - Structure used: the iteration splits into 20 blocks of 30 entries. Block j of t0 needs only `state[30j..30j+29]`, `state[600+j]` and `state[620..649]`.
   - `t1[j]` reads only state block j and t0 block j.
   - The 30 rest rows of su (620+k) accumulate over the same blocks.
   - So each block of t0 is computed, used right away for `t1[j]` and the su accumulators while still in cache, and never re-read.
2. **Coefficient tables repacked once.** The t0 coefficients and the su rest-row coefficients (S, T, E) are stored with strides of 3, 21 and 20.
   - On the first call they are copied into contiguous static 600-entry tables, read from `fom_data.c`.
   - This makes the inner loops unit-stride, so they vectorize.
   - There is no dynamic allocation, and these values are not baked into the code.
3. **Selection rows written straight into the new state.** The solver passes `t_t0 = xn` and `t_t1 = xn + 600` to `fom_iterate`. This works because su rows 0..619 are copies of t0 and t1.
   - The redundant copy into `t_t0` is skipped when it aliases `new_state`. The value written is still correct for any bias.
4. **Kept from earlier rounds:**
   - The xi term pairs cancel, so `xi (D x + E t)` becomes `xi E (t - x)` in both t1 and the su rest rows.
   - The t1 dot products use six independent partial sums so the multiply-adds don't wait on each other.

**Blocked by rules:**
- Hardware vector instructions (AVX2/NEON FMA) through build flags such as `-march=native` | Makefile and build flags are fixed | unknown, maybe 10–20%
- Skip adding `bias_su` on selection rows 0..619, since it is zero there in the solver (p0 only reaches rows 620+) | `fom_iterate` must be right for any bias, and `fom_update_params` may not probe the matrix | under 2%
- Do the repacking in `fom_setup` instead of a first-call check in `fom_iterate` | `src/fom_solver.c` may not read the `FOM_MAT_` data of matrices it multiplies | negligible

### Round 4

**Timing.** The translated solver took 125.343 us. The solver from the earlier rounds took 30.204 us going in, and this round's runs measured 28.1 to 29.6 us on unchanged code; that spread is just run-to-run noise. Final: about 29 us, an overall speedup of about 4.2x to 4.3x. Round 4 gained nothing.

**Final profile (`make profile`, hottest first):**
| region | time | % |
|---|---|---|
| iterate | 143152 us | 98.82% |
| residual | 1704 us | 1.18% |

**Optimizations in the kept solver** (all from earlier rounds; this round added none):
- **One-pass `fom_iterate` over the 20 blocks of 30 entries.** `t1[j]` reads only state and t0 entries of block j. Each su "rest" row (620 to 649) has one entry per block. So t0, t1 and su can be computed in a single sweep, with values kept in registers or locals between them.
- **Selection rows written in place.** Su rows 0 to 619 are plain copies of t0 and t1. The iterate writes t0 and t1 straight into `new_state` (adding `bias_su`), so there are no separate copy or zeroing passes.
- **Fixed sparsity unrolled.** t0_state has 3 entries per row: one unit entry, one on `state[600+j]` and one on `state[620+k]`. The strided su and t1 coefficients are repacked once into contiguous 600-entry tables, which makes the inner loop vectorizable.
- **Shared difference term.** The xi-scaled terms act on `(t0 - state)` (the `et*(t - xr)` and `vd*(tb - xr)` terms in the code), so each xi product is a single FMA.
- **Six partial sums for the t1 dot products.** This breaks the serial FMA dependency chain.
- **Parameter term hoisted.** `su_p_p0 @ p0` is computed once per solve as the su bias.
- **Residual computed only at the last iteration.** There is no tolerance check, so it is only reported at the end.

**Tried and reverted this round:** packing the five coefficient tables into one interleaved per-block array, to reduce the number of memory streams. It measured about 30 to 32 us against about 28.5 us, so it was slower.

**Assessment:** the iterate already does close to the minimum arithmetic for these matrices, and all its data fits in L1 cache. What remains is within measurement noise.

Blocked by rules:
- Fold the t0 bias into the clip bounds (`clip(b+y,l,h) = b + clip(y,l-b,h-b)`), precomputed once per solve | `fom_iterate` must be right for any bias and activation passed on each call, and `fom_solver.c` may not read matrix or activation data in a way that replaces the checked routines | ~3-5%
- Repack the coefficient tables in `fom_setup` instead of behind a lazy first-call init flag | `fom_solver.c` must not read `FOM_MAT_*` data, and setup may not call apply routines | <1%
```
