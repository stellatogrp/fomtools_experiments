# Optimization report

## Timing

```
Timing summary (per solve):
  Python reference (before codegen): 787519.386 us
  Generated C (initial, phase A):    420554.202 us
  Generated C (optimized, phase B):  146887.133 us
  Speedup from codegen (Py -> C):         1.9x
  Speedup from optimization (A -> B):    2.86x
  Total speedup (Py -> optimized C):      5.4x
Profiling breakdown (hottest first):
  t0_state                  1087540.817 us   65.09%  (2400 calls)
  threshold                  556380.491 us   33.30%  (2400 calls)
  state_update                20067.143 us    1.20%  (2400 calls)
  cache_init                   5769.696 us    0.35%  (12 calls)
  residual_copy                1170.474 us    0.07%  (12 calls)
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

 #include <string.h>

@@ -28,6 +29,8 @@
 void fom_update_params(FomWorkspace *work, const FomParams *params)
 {
     memcpy(work->params.p0, params->p0, sizeof(work->params.p0));
+    memset(work->new_state, 0, FOM_T0_DIM * sizeof(work->new_state[0]));
+    fom_apply_t0_p_p0(work->params.p0, work->new_state);
     fom_reset_state(work);
 }

@@ -38,46 +41,64 @@
     fom_int status = FOM_MAX_ITER_REACHED;
     fom_float residual = (fom_float)0;

+    /* Cache B*x(k-1).  The generated t0_state operator reads only the
+       second state slot, so acc's upper half can present x(k-1) to it. */
+    FOM_PROF_BEGIN(cache_init);
+    memset(work->acc, 0, sizeof(work->acc));
+    memcpy(work->acc + FOM_T0_DIM, work->state,
+           FOM_T0_DIM * sizeof(work->acc[0]));
+    fom_apply_t0_state(work->acc, work->acc);
+    FOM_PROF_END(cache_init);
+
     for (it = 0; it < work->settings.max_iter; ++it) {
         const fom_float gamma = fom_sched_gamma(it);

+        FOM_PROF_BEGIN(t0_state);
         memset(work->t0, 0, sizeof(work->t0));
         fom_apply_t0_state(work->state, work->t0);
-        fom_apply_t0_p_p0(work->params.p0, work->t0);
+        FOM_PROF_END(t0_state);

-        memset(work->acc, 0, FOM_T0_DIM * sizeof(work->acc[0]));
-        fom_apply_t0_state__d_gamma(work->state, work->acc);
-
+        FOM_PROF_BEGIN(threshold);
 #ifdef _OPENMP
 #pragma omp parallel for schedule(static)
 #endif
         for (i = 0; i < FOM_T0_DIM; ++i) {
-            fom_float value = work->t0[i] + gamma * work->acc[i];
-            if (value > FOM_T0_LAMBDA)
-                value -= FOM_T0_LAMBDA;
-            else if (value < -FOM_T0_LAMBDA)
-                value += FOM_T0_LAMBDA;
-            else
-                value = (fom_float)0;
+            const fom_float current = work->t0[i];
+            const fom_float delta = current - work->acc[i];
+            fom_float value = current + work->new_state[i] + gamma * delta;
+            work->acc[i] = current;
+            value = value > FOM_T0_LAMBDA ? value - FOM_T0_LAMBDA
+                  : value < -FOM_T0_LAMBDA ? value + FOM_T0_LAMBDA
+                  : (fom_float)0;
             work->t0[i] = value;
         }
+        FOM_PROF_END(threshold);

-        memset(work->new_state, 0, sizeof(work->new_state));
-        fom_apply_su_state(work->state, work->new_state);
-        fom_apply_su_t_t0(work->t0, work->new_state);
-
-        residual = (fom_float)0;
-#ifdef _OPENMP
-#pragma omp parallel for reduction(max:residual) schedule(static)
-#endif
-        for (i = 0; i < FOM_STATE_DIM; ++i) {
-            fom_float delta = work->new_state[i] - work->state[i];
-            if (delta < (fom_float)0)
-                delta = -delta;
-            if (delta > residual)
-                residual = delta;
-            work->state[i] = work->new_state[i];
+        /* su_state and su_t_t0 are unit copy blocks:
+           [x(k-1), x(k)] <- [x(k), t0]. */
+        FOM_PROF_BEGIN(state_update);
+        if (it + 1 < work->settings.max_iter) {
+            if (it + 2 == work->settings.max_iter)
+                memcpy(work->state, work->state + FOM_T0_DIM,
+                       FOM_T0_DIM * sizeof(work->state[0]));
+            memcpy(work->state + FOM_T0_DIM, work->t0,
+                   FOM_T0_DIM * sizeof(work->state[0]));
+        } else {
+            FOM_PROF_BEGIN(residual_copy);
+            residual = (fom_float)0;
+            for (i = 0; i < FOM_T0_DIM; ++i) {
+                fom_float delta0 = work->state[FOM_T0_DIM + i] - work->state[i];
+                fom_float delta1 = work->t0[i] - work->state[FOM_T0_DIM + i];
+                if (delta0 < (fom_float)0) delta0 = -delta0;
+                if (delta1 < (fom_float)0) delta1 = -delta1;
+                if (delta0 > residual) residual = delta0;
+                if (delta1 > residual) residual = delta1;
+                work->state[i] = work->state[FOM_T0_DIM + i];
+                work->state[FOM_T0_DIM + i] = work->t0[i];
+            }
+            FOM_PROF_END(residual_copy);
         }
+        FOM_PROF_END(state_update);

         if (FOM_USE_TOL && work->settings.check_termination &&
             residual <= work->settings.tol) {
```

## Agent notes

```
Initial mean_us: 424369.186083
Final mean_us: 145289.138083
Overall speedup: 2.92x
Verification: VERIFY OK (worst max-abs error 4.957e-07)

Final profiling breakdown, hottest first:
- t0_state: 1087720.755 us, 64.99%
- threshold: 558788.258 us, 33.39%
- state_update: 20171.047 us, 1.21%
- cache_init: 5824.247 us, 0.35%
- residual_copy: 1173.358 us, 0.07%

Optimizations:
- Exploited the FISTA operator identity `t0_state = B*x(k)` and `t0_state__d_gamma = B*x(k) - B*x(k-1)`. Cached the previous `B*x` result, eliminating the 1.2M-nonzero derivative CSR multiply from every iteration.
- Exploited parameter invariance: `t0_p_p0*p0` is constant throughout a solve. Precomputed it in `fom_update_params` using existing workspace scratch.
- Exploited the exact unit-copy block structure of `su_state` and `su_t_t0`. Replaced both CSR operations and output clearing with direct state-slot copies.
- Exploited max-iteration-only termination. Computed the observable residual only on the final iteration and retained the historical state slot only when needed.
- Fused extrapolation-difference formation, cached-product update, invariant parameter addition, and soft thresholding into one streaming pass.
- Kept profiling markers around all remaining main computational regions. The solver remains allocation-free and division-free.
```
