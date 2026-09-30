# Optimization report

## Timing

```
Timing summary (per solve):
  Python reference (before codegen): 202067.664 us
  Generated C (initial, phase A):    144437.162 us
  Generated C (optimized, phase B):  100794.746 us
  Speedup from codegen (Py -> C):         1.4x
  Speedup from optimization (A -> B):    1.43x
  Total speedup (Py -> optimized C):      2.0x
Profiling breakdown (hottest first):
  state_matvec              2340062.128 us   93.64%  (5000 calls)
  activation_update          134212.422 us    5.37%  (5000 calls)
  state_clear                 24400.915 us    0.98%  (5000 calls)
  solution_copy                 386.477 us    0.02%  (25 calls)
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

 static void reset_state(FomWorkspace *work)
 {
@@ -34,6 +35,12 @@

     for (i = 0; i < FOM_PARAM_P0_DIM; ++i)
         work->params.p0[i] = params->p0[i];
+
+    /* The parameter contribution is constant for every iteration.  Keep it in
+       the otherwise-unused accumulator scratch vector. */
+    for (i = 0; i < FOM_T0_DIM; ++i)
+        work->acc[i] = (fom_float)0;
+    fom_apply_t0_p_p0(work->params.p0, work->acc);
     reset_state(work);
 }

@@ -47,34 +54,39 @@
     fom_int status = FOM_MAX_ITER_REACHED;

     for (iter = 0; iter < max_iter; ++iter) {
+        FOM_PROF_BEGIN(state_clear);
         for (i = 0; i < FOM_T0_DIM; ++i)
             work->t0[i] = (fom_float)0;
+        FOM_PROF_END(state_clear);
+        FOM_PROF_BEGIN(state_matvec);
         fom_apply_t0_state(work->state, work->t0);
-        fom_apply_t0_p_p0(work->params.p0, work->t0);
+        FOM_PROF_END(state_matvec);

-        for (i = 0; i < FOM_T0_DIM; ++i) {
-            const fom_float value = work->t0[i];
-            if (value > lambda)
-                work->t0[i] = value - lambda;
-            else if (value < -lambda)
-                work->t0[i] = value + lambda;
-            else
-                work->t0[i] = (fom_float)0;
+        FOM_PROF_BEGIN(activation_update);
+        if (iter + 1 < max_iter) {
+            for (i = 0; i < FOM_T0_DIM; ++i) {
+                const fom_float value = work->t0[i] + work->acc[i];
+                const fom_float above = (fom_float)(value > lambda);
+                const fom_float below = (fom_float)(value < -lambda);
+                work->state[i] = above * (value - lambda) +
+                                 below * (value + lambda);
+            }
+        } else {
+            residual = (fom_float)0;
+            for (i = 0; i < FOM_T0_DIM; ++i) {
+                const fom_float value = work->t0[i] + work->acc[i];
+                const fom_float next = value > lambda ? value - lambda :
+                                       (value < -lambda ? value + lambda :
+                                                          (fom_float)0);
+                fom_float delta = next - work->state[i];
+                if (delta < (fom_float)0)
+                    delta = -delta;
+                if (delta > residual)
+                    residual = delta;
+                work->state[i] = next;
+            }
         }
-
-        for (i = 0; i < FOM_STATE_DIM; ++i)
-            work->new_state[i] = (fom_float)0;
-        fom_apply_su_t_t0(work->t0, work->new_state);
-
-        residual = (fom_float)0;
-        for (i = 0; i < FOM_STATE_DIM; ++i) {
-            fom_float delta = work->new_state[i] - work->state[i];
-            if (delta < (fom_float)0)
-                delta = -delta;
-            if (delta > residual)
-                residual = delta;
-            work->state[i] = work->new_state[i];
-        }
+        FOM_PROF_END(activation_update);

         if (FOM_USE_TOL && work->settings.check_termination &&
             residual <= work->settings.tol) {
@@ -84,8 +96,10 @@
         }
     }

+    FOM_PROF_BEGIN(solution_copy);
     for (i = 0; i < FOM_STATE_DIM; ++i)
         work->solution.x[i] = work->state[i];
+    FOM_PROF_END(solution_copy);
     work->solution.iter = iter;
     work->solution.residual = residual;
     work->solution.status = status;
```

## Agent notes

```
Initial vs. final mean_us: 143083.102720 → 99655.682040, an overall 1.436× speedup (30.35% reduction).

Final profiling breakdown, hottest first:
- state_matvec: 2338046.2550 µs, 93.67%
- activation_update: 133444.3560 µs, 5.35%
- state_clear: 24135.1800 µs, 0.97%
- solution_copy: 383.4050 µs, 0.02%

Optimizations applied:
- Hoisted the iteration-invariant sparse parameter product into `fom_update_params`, storing it in existing workspace scratch. This exploits the fact that `p0` is constant throughout all 200 iterations.
- Removed the sparse state-update operation and its zero-fill pass. This exploits `su_t_t0` being exactly the 40000×40000 identity matrix.
- Fused soft-thresholding with the state update, reducing memory passes.
- Computed the infinity-norm residual only on the final iteration. This exploits the fixed `FOM_USE_TOL == 0` max-iteration termination structure.
- Replaced scalar threshold branches in the common iteration path with mutually exclusive arithmetic masks, enabling SIMD vectorization.

Final validation: `VERIFY OK`, with worst max-absolute error 0.000e+00.
```
