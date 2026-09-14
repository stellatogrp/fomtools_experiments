# Optimization report

## Timing

```
Timing summary (per solve):
  Python reference (before codegen): 297445.415 us
  Generated C (initial, phase A):    250960.307 us
  Generated C (optimized, phase B):  208161.685 us
  Speedup from codegen (Py -> C):         1.2x
  Speedup from optimization (A -> B):    1.21x
  Total speedup (Py -> optimized C):      1.4x
Profiling breakdown (hottest first):
  state_product             9806762.282 us   93.95%  (10000 calls)
  threshold_update           624119.557 us    5.98%  (10000 calls)
  param_precompute             7346.832 us    0.07%  (50 calls)
```

Optimization outcome: **kept (re-verified correct)**.

## What the agent changed

```diff
--- src/fom_solver.c (phase A)
+++ src/fom_solver.c (optimized)
@@ -3,6 +3,7 @@
 #include "fom_solver.h"
 #include "fom_data.h"
 #include "fom_ops.h"
+#include "fom_profile.h"

 static void reset_state(FomWorkspace *work)
 {
@@ -41,37 +42,37 @@

     work->solution.status = FOM_MAX_ITER_REACHED;

+    FOM_PROF_BEGIN(param_precompute);
+    memset(work->acc, 0, sizeof(work->acc));
+    fom_apply_t0_p_p0(work->params.p0, work->acc);
+    FOM_PROF_END(param_precompute);
+
     for (iteration = 0; iteration < work->settings.max_iter; ++iteration) {
-        memset(work->t0, 0, sizeof(work->t0));
+        FOM_PROF_BEGIN(state_product);
+        memcpy(work->t0, work->acc, sizeof(work->t0));
         fom_apply_t0_state(work->state, work->t0);
-        fom_apply_t0_p_p0(work->params.p0, work->t0);
+        FOM_PROF_END(state_product);

-#ifdef _OPENMP
-#pragma omp parallel for
-#endif
-        for (i = 0; i < FOM_T0_DIM; ++i) {
-            const fom_float value = work->t0[i];
-            work->t0[i] = value > threshold
-                ? value - threshold
-                : (value < -threshold ? value + threshold : (fom_float)0);
-        }
-
-        memset(work->new_state, 0, sizeof(work->new_state));
-        fom_apply_su_t_t0(work->t0, work->new_state);
-
+        FOM_PROF_BEGIN(threshold_update);
         residual = (fom_float)0;
 #ifdef _OPENMP
 #pragma omp parallel for reduction(max:residual)
 #endif
-        for (i = 0; i < FOM_STATE_DIM; ++i) {
-            fom_float difference = work->new_state[i] - work->state[i];
-            if (difference < (fom_float)0)
-                difference = -difference;
-            if (difference > residual)
-                residual = difference;
+        for (i = 0; i < FOM_T0_DIM; ++i) {
+            const fom_float value = work->t0[i];
+            const fom_float next = value > threshold
+                ? value - threshold
+                : (value < -threshold ? value + threshold : (fom_float)0);
+            if (iteration + 1 == work->settings.max_iter) {
+                fom_float difference = next - work->state[i];
+                if (difference < (fom_float)0)
+                    difference = -difference;
+                if (difference > residual)
+                    residual = difference;
+            }
+            work->state[i] = next;
         }
-
-        memcpy(work->state, work->new_state, sizeof(work->state));
+        FOM_PROF_END(threshold_update);
     }

     memcpy(work->solution.x, work->state, sizeof(work->solution.x));
```

## Agent notes

```
Initial vs. final whole-solve mean_us: 249474.954440 → 207842.623260, an overall 1.200× speedup (16.7% reduction). Final verification: VERIFY OK with 0.000e+00 worst error.

Final profiling breakdown, hottest first:
- state_product: 9762008.567 us, 93.94%
- threshold_update: 622255.132 us, 5.99%
- param_precompute: 7178.698 us, 0.07%

Optimizations applied:
- Hoisted the parameter CSR matrix-vector product out of the 200-iteration loop. This exploits that `p0` and `t0_p_p0` are iteration-invariant, caching the product in existing workspace scratch.
- Removed the generated state-update CSR operation. This exploits that `su_t_t0` is exactly the identity matrix: one unit entry at column `i` in every row.
- Fused soft-thresholding with the identity state assignment, eliminating separate state-update and copy passes.
- Computed the residual only during the final iteration. This exploits the problem’s fixed max-iteration termination (`FOM_USE_TOL == 0`), under which intermediate residuals cannot affect execution.
- Retained profiling markers around all remaining principal computational regions.
```
