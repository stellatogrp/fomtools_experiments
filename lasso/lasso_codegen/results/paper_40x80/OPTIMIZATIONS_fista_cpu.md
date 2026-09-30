# Optimization report

## Timing

```
Timing summary (per solve):
  Python reference (before codegen):   7710.843 us
  Generated C (initial, phase A):      4261.969 us
  Generated C (optimized, phase B):     267.021 us
  Speedup from codegen (Py -> C):         1.8x
  Speedup from optimization (A -> B):   15.96x
  Total speedup (Py -> optimized C):     28.9x
Profiling breakdown (hottest first):
  linear_map                1353077.983 us   90.62%  (1000000 calls)
  threshold                   79933.798 us    5.35%  (1000000 calls)
  state_update                51532.503 us    3.45%  (1000000 calls)
  param_map                    7901.708 us    0.53%  (5000 calls)
  final_residual                769.939 us    0.05%  (5000 calls)
```

Optimization outcome: **kept (re-verified correct)**.

## What the agent changed

```diff
--- src/fom_solver.c (phase A)
+++ src/fom_solver.c (optimized)
@@ -1,6 +1,7 @@
 #include "fom_solver.h"
 #include "fom_data.h"
 #include "fom_ops.h"
+#include "fom_profile.h"

 void fom_set_default_settings(FomSettings *settings)
 {
@@ -43,20 +44,61 @@
     fom_float residual = (fom_float)0;
     fom_int it;
     fom_int i;
+    fom_int j;
+
+    /* The parameter is constant for the whole solve.  Keep its dense product
+       in acc; this avoids repeating an 80x40 matrix-vector product. */
+    FOM_PROF_BEGIN(param_map);
+    for (i = 0; i < FOM_T0_DIM; ++i)
+        work->acc[i] = (fom_float)0;
+    fom_apply_t0_p_p0(work->params.p0, work->acc);
+    FOM_PROF_END(param_map);

     for (it = 0; it < work->settings.max_iter; ++it) {
         const fom_float gamma = fom_sched_gamma(it);

-        for (i = 0; i < FOM_T0_DIM; ++i) {
-            work->t0[i] = (fom_float)0;
-            work->acc[i] = (fom_float)0;
+        FOM_PROF_BEGIN(linear_map);
+        /* FISTA state is [x_old, x].  The generated dense maps are
+           [0,H] + gamma*[-H,H], so apply H once to x+gamma*(x-x_old). */
+        for (j = 0; j < FOM_T0_DIM; ++j)
+            next[j] = state[FOM_T0_DIM + j]
+                    + gamma * (state[FOM_T0_DIM + j] - state[j]);
+        for (i = 0; i < FOM_T0_DIM; i += 8) {
+            fom_float s0 = (fom_float)0, s1 = (fom_float)0;
+            fom_float s2 = (fom_float)0, s3 = (fom_float)0;
+            fom_float s4 = (fom_float)0, s5 = (fom_float)0;
+            fom_float s6 = (fom_float)0, s7 = (fom_float)0;
+            const fom_float *r0 = FOM_MAT_t0_state + i * FOM_STATE_DIM + FOM_T0_DIM;
+            const fom_float *r1 = r0 + FOM_STATE_DIM;
+            const fom_float *r2 = r1 + FOM_STATE_DIM;
+            const fom_float *r3 = r2 + FOM_STATE_DIM;
+            const fom_float *r4 = r3 + FOM_STATE_DIM;
+            const fom_float *r5 = r4 + FOM_STATE_DIM;
+            const fom_float *r6 = r5 + FOM_STATE_DIM;
+            const fom_float *r7 = r6 + FOM_STATE_DIM;
+            for (j = 0; j < FOM_T0_DIM; ++j) {
+                const fom_float y = next[j];
+                s0 += r0[j] * y;
+                s1 += r1[j] * y;
+                s2 += r2[j] * y;
+                s3 += r3[j] * y;
+                s4 += r4[j] * y;
+                s5 += r5[j] * y;
+                s6 += r6[j] * y;
+                s7 += r7[j] * y;
+            }
+            work->t0[i] = s0 + work->acc[i];
+            work->t0[i + 1] = s1 + work->acc[i + 1];
+            work->t0[i + 2] = s2 + work->acc[i + 2];
+            work->t0[i + 3] = s3 + work->acc[i + 3];
+            work->t0[i + 4] = s4 + work->acc[i + 4];
+            work->t0[i + 5] = s5 + work->acc[i + 5];
+            work->t0[i + 6] = s6 + work->acc[i + 6];
+            work->t0[i + 7] = s7 + work->acc[i + 7];
         }
-        fom_apply_t0_state(state, work->t0);
-        fom_apply_t0_state__d_gamma(state, work->acc);
-        for (i = 0; i < FOM_T0_DIM; ++i)
-            work->t0[i] += gamma * work->acc[i];
-        fom_apply_t0_p_p0(work->params.p0, work->t0);
+        FOM_PROF_END(linear_map);

+        FOM_PROF_BEGIN(threshold);
         for (i = 0; i < FOM_T0_DIM; ++i) {
             const fom_float value = work->t0[i];
             if (value > FOM_T0_LAMBDA)
@@ -66,20 +108,16 @@
             else
                 work->t0[i] = (fom_float)0;
         }
+        FOM_PROF_END(threshold);

-        for (i = 0; i < FOM_STATE_DIM; ++i)
-            next[i] = (fom_float)0;
-        fom_apply_su_state(state, next);
-        fom_apply_su_t_t0(work->t0, next);
-
-        residual = (fom_float)0;
-        for (i = 0; i < FOM_STATE_DIM; ++i) {
-            fom_float delta = next[i] - state[i];
-            if (delta < (fom_float)0)
-                delta = -delta;
-            if (delta > residual)
-                residual = delta;
+        FOM_PROF_BEGIN(state_update);
+        /* The sparse update is exactly [x, t0]: its only nonzeros are
+           unit diagonals from state[80:] and t0 into the two blocks. */
+        for (i = 0; i < FOM_T0_DIM; ++i) {
+            next[i] = state[FOM_T0_DIM + i];
+            next[FOM_T0_DIM + i] = work->t0[i];
         }
+        FOM_PROF_END(state_update);

         {
             fom_float *swap = state;
@@ -87,6 +125,20 @@
             next = swap;
         }
     }
+
+    /* Termination is max-iterations-only, so only the reported final
+       residual is observable.  'next' still holds the preceding state. */
+    FOM_PROF_BEGIN(final_residual);
+    if (it > 0) {
+        for (i = 0; i < FOM_STATE_DIM; ++i) {
+            fom_float delta = state[i] - next[i];
+            if (delta < (fom_float)0)
+                delta = -delta;
+            if (delta > residual)
+                residual = delta;
+        }
+    }
+    FOM_PROF_END(final_residual);

     if (state != work->state) {
         for (i = 0; i < FOM_STATE_DIM; ++i)
```

## Agent notes

```
Initial whole-solve mean_us: 4245.444
Final whole-solve mean_us: 268.280
Overall speedup: 15.82x

Final profiling breakdown, hottest first:
- linear_map: 1,356,234.502 us total, 90.46%
- threshold: 80,989.337 us total, 5.40%
- state_update: 53,429.280 us total, 3.56%
- param_map: 7,869.368 us total, 0.52%
- final_residual: 773.364 us total, 0.05%

Optimizations applied:
- Hoisted the parameter-only 80x40 dense map outside the iteration loop. This exploits that p0 is constant throughout a solve.
- Collapsed two 80x160 state maps into one 80x80 map. This exploits the FISTA block structure [0,H] + gamma*[-H,H] acting on state [x_old,x].
- Precomputed x + gamma*(x-x_old) once per iteration, avoiding repeated momentum arithmetic for every output row.
- Interleaved eight independent matrix rows. This exploits the common input vector and independent dot products to expose instruction-level parallelism while preserving each dot product’s accumulation order.
- Replaced generated CSR state-update calls with direct block copies. This exploits their exact unit-identity structure: next = [x,t0].
- Computed the residual only after the loop. This exploits max-iteration-only termination: intermediate residuals cannot affect control flow.
- Retained profiling markers around all principal computational regions.

Final verification: VERIFY OK; worst max-absolute error 1.120e-11.
```
