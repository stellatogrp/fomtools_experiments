# Optimization report

## Timing

```
Timing summary (per solve):
  Python reference (before codegen):   4164.957 us
  Generated C (initial, phase A):       789.822 us
  Generated C (optimized, phase B):     271.245 us
  Speedup from codegen (Py -> C):         5.3x
  Speedup from optimization (A -> B):    2.91x
  Total speedup (Py -> optimized C):     15.4x
Profiling breakdown (hottest first):
  state_transform           1287955.279 us   90.85%  (1000000 calls)
  threshold_residual         129768.134 us    9.15%  (1000000 calls)
```

Optimization outcome: **kept (re-verified correct)**.

## What the agent changed

```diff
--- src/fom_solver.c (phase A)
+++ src/fom_solver.c (optimized)
@@ -1,6 +1,54 @@
 #include "fom_solver.h"
 #include "fom_data.h"
 #include "fom_ops.h"
+#include "fom_profile.h"
+
+/* Eight rows at a time keeps the reused state vector hot and exposes eight
+   independent reduction chains to the compiler. */
+static void apply_state(const fom_float *restrict x,
+                        fom_float *restrict out)
+{
+    fom_int i, j;
+
+    for (i = 0; i < FOM_STATE_DIM; i += 8) {
+        const fom_float *m0 = FOM_MAT_t0_state + i * FOM_STATE_DIM;
+        const fom_float *m1 = m0 + FOM_STATE_DIM;
+        const fom_float *m2 = m1 + FOM_STATE_DIM;
+        const fom_float *m3 = m2 + FOM_STATE_DIM;
+        const fom_float *m4 = m3 + FOM_STATE_DIM;
+        const fom_float *m5 = m4 + FOM_STATE_DIM;
+        const fom_float *m6 = m5 + FOM_STATE_DIM;
+        const fom_float *m7 = m6 + FOM_STATE_DIM;
+        fom_float s0 = (fom_float)0;
+        fom_float s1 = (fom_float)0;
+        fom_float s2 = (fom_float)0;
+        fom_float s3 = (fom_float)0;
+        fom_float s4 = (fom_float)0;
+        fom_float s5 = (fom_float)0;
+        fom_float s6 = (fom_float)0;
+        fom_float s7 = (fom_float)0;
+
+        for (j = 0; j < FOM_STATE_DIM; ++j) {
+            const fom_float v = x[j];
+            s0 += m0[j] * v;
+            s1 += m1[j] * v;
+            s2 += m2[j] * v;
+            s3 += m3[j] * v;
+            s4 += m4[j] * v;
+            s5 += m5[j] * v;
+            s6 += m6[j] * v;
+            s7 += m7[j] * v;
+        }
+        out[i] = s0;
+        out[i + 1] = s1;
+        out[i + 2] = s2;
+        out[i + 3] = s3;
+        out[i + 4] = s4;
+        out[i + 5] = s5;
+        out[i + 6] = s6;
+        out[i + 7] = s7;
+    }
+}

 static void reset_state(FomWorkspace *work)
 {
@@ -54,33 +102,29 @@
     fom_apply_t0_p_p0(work->params.p0, work->acc);

     for (k = 0; k < work->settings.max_iter; ++k) {
-        for (i = 0; i < FOM_T0_DIM; ++i)
-            work->t0[i] = (fom_float)0;
-        fom_apply_t0_state(work->state, work->t0);
+        FOM_PROF_BEGIN(state_transform);
+        apply_state(work->state, work->t0);
+        FOM_PROF_END(state_transform);

+        FOM_PROF_BEGIN(threshold_residual);
+        residual = (fom_float)0;
         for (i = 0; i < FOM_T0_DIM; ++i) {
             fom_float value = work->t0[i] + work->acc[i];
+            fom_float delta;
             if (value > FOM_T0_LAMBDA)
                 value -= FOM_T0_LAMBDA;
             else if (value < -FOM_T0_LAMBDA)
                 value += FOM_T0_LAMBDA;
             else
                 value = (fom_float)0;
-            work->t0[i] = value;
-            work->new_state[i] = (fom_float)0;
-        }
-
-        fom_apply_su_t_t0(work->t0, work->new_state);
-
-        residual = (fom_float)0;
-        for (i = 0; i < FOM_STATE_DIM; ++i) {
-            fom_float delta = work->new_state[i] - work->state[i];
+            delta = value - work->state[i];
             if (delta < (fom_float)0)
                 delta = -delta;
             if (delta > residual)
                 residual = delta;
-            work->state[i] = work->new_state[i];
+            work->state[i] = value;
         }
+        FOM_PROF_END(threshold_residual);

         if (work->settings.check_termination && FOM_USE_TOL &&
             residual <= work->settings.tol) {
```

## Agent notes

```
Initial mean_us: 794.030784
Final mean_us: 271.158332
Overall speedup: 2.928x

Final profiling breakdown, hottest first:
- state_transform: 1287363.351 us total, 90.84%
- threshold_residual: 129858.994 us total, 9.16%

Optimizations applied:
- Specialized the 80×80 dense row-major state transform into eight-row blocks. This exploits reuse of the common 80-element state vector and provides eight independent reduction chains for compiler vectorization and instruction-level parallelism while preserving each row’s arithmetic order.
- Eliminated the generic state-update sparse matrix operation because `su_t_t0` is exactly the identity matrix. Thresholded values can therefore become the next state directly.
- Fused soft-thresholding, residual calculation, and state copying into one pass. This exploits the identity state-update structure and reduces workspace traffic.
- Removed the dead thresholded-value write to `t0`, which is completely overwritten by the next dense transform.
- Retained profiling markers around the dense transform and fused threshold/residual region.

Final verification: VERIFY OK
```
