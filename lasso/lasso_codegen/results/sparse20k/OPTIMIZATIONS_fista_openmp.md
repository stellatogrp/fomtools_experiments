# Optimization report

## Timing

```
Timing summary (per solve):
  Python reference (before codegen): 1113991.868 us
  Generated C (initial, phase A):    800941.143 us
  Generated C (optimized, phase B):  203134.990 us
  Speedup from codegen (Py -> C):         1.4x
  Speedup from optimization (A -> B):    3.94x
  Total speedup (Py -> optimized C):      5.5x
Profiling breakdown (hottest first):
  t0_state                  4877328.216 us   96.09%  (5000 calls)
  threshold                   88173.343 us    1.74%  (5000 calls)
  state_update                66344.953 us    1.31%  (5000 calls)
  extrapolate                 40540.340 us    0.80%  (5000 calls)
  parameter                    3546.591 us    0.07%  (25 calls)
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

 static void fom_reset_state(FomWorkspace *work)
 {
@@ -36,49 +37,63 @@
     fom_int it;
     fom_float residual = (fom_float)0;

+    /* The parameter contribution is constant for the whole solve. */
+    FOM_PROF_BEGIN(parameter);
+    memset(work->acc, 0, FOM_T0_DIM * sizeof(work->acc[0]));
+    fom_apply_t0_p_p0(work->params.p0, work->acc);
+    FOM_PROF_END(parameter);
+
     for (it = 0; it < work->settings.max_iter; ++it) {
         const fom_float gamma = fom_sched_gamma(it);
         fom_int i;

-        memset(work->t0, 0, sizeof(work->t0));
-        fom_apply_t0_state(work->state, work->t0);
+        /* t0_state uses only state[20000:40000].  The generated
+           d_gamma matrix is the same CSR matrix applied to
+           state[20000:40000] - state[0:20000].  Form the extrapolated
+           input and perform the sparse multiply only once. */
+        FOM_PROF_BEGIN(extrapolate);
+        for (i = 0; i < FOM_T0_DIM; ++i)
+            work->new_state[FOM_T0_DIM + i] = work->state[FOM_T0_DIM + i]
+                + gamma * (work->state[FOM_T0_DIM + i] - work->state[i]);
+        FOM_PROF_END(extrapolate);

-        memset(work->acc, 0, FOM_T0_DIM * sizeof(work->acc[0]));
-        fom_apply_t0_state__d_gamma(work->state, work->acc);
+        FOM_PROF_BEGIN(t0_state);
+        memcpy(work->t0, work->acc, sizeof(work->t0));
+        fom_apply_t0_state(work->new_state, work->t0);
+        FOM_PROF_END(t0_state);

-        fom_apply_t0_p_p0(work->params.p0, work->t0);
-
+        FOM_PROF_BEGIN(threshold);
         #ifdef _OPENMP
         #pragma omp parallel for
         #endif
         for (i = 0; i < FOM_T0_DIM; ++i) {
-            fom_float value = work->t0[i] + gamma * work->acc[i];
-            if (value > FOM_T0_LAMBDA)
-                value -= FOM_T0_LAMBDA;
-            else if (value < -FOM_T0_LAMBDA)
-                value += FOM_T0_LAMBDA;
-            else
-                value = (fom_float)0;
-            work->t0[i] = value;
+            const fom_float value = work->t0[i];
+            const fom_int side = (value > FOM_T0_LAMBDA)
+                - (value < -FOM_T0_LAMBDA);
+            work->t0[i] = (fom_float)(side != 0)
+                * (value - (fom_float)side * FOM_T0_LAMBDA);
         }
+        FOM_PROF_END(threshold);

-        memset(work->new_state, 0, sizeof(work->new_state));
-        fom_apply_su_state(work->state, work->new_state);
-        fom_apply_su_t_t0(work->t0, work->new_state);
-
-        residual = (fom_float)0;
-        #ifdef _OPENMP
-        #pragma omp parallel for reduction(max:residual)
-        #endif
-        for (i = 0; i < FOM_STATE_DIM; ++i) {
-            fom_float delta = work->new_state[i] - work->state[i];
-            if (delta < (fom_float)0)
-                delta = -delta;
-            if (delta > residual)
-                residual = delta;
+        /* su_state and su_t_t0 are exact block copies:
+           [old second half, t0]. */
+        FOM_PROF_BEGIN(state_update);
+        if (it + 1 == work->settings.max_iter) {
+            residual = (fom_float)0;
+            for (i = 0; i < FOM_T0_DIM; ++i) {
+                fom_float d0 = work->state[FOM_T0_DIM + i] - work->state[i];
+                fom_float d1 = work->t0[i] - work->state[FOM_T0_DIM + i];
+                if (d0 < (fom_float)0) d0 = -d0;
+                if (d1 < (fom_float)0) d1 = -d1;
+                if (d0 > residual) residual = d0;
+                if (d1 > residual) residual = d1;
+            }
         }
-
-        memcpy(work->state, work->new_state, sizeof(work->state));
+        for (i = 0; i < FOM_T0_DIM; ++i) {
+            work->state[i] = work->state[FOM_T0_DIM + i];
+            work->state[FOM_T0_DIM + i] = work->t0[i];
+        }
+        FOM_PROF_END(state_update);
         work->solution.iter = it + 1;

         if (FOM_USE_TOL && work->settings.check_termination &&
```

## Agent notes

```
Initial mean_us: 809302.376160
Final mean_us: 202704.496600
Overall speedup: 3.99x

Final profiling breakdown, hottest first:
- t0_state: 4891887.281 us, 96.09%
- threshold: 88180.172 us, 1.73%
- state_update: 66693.425 us, 1.31%
- extrapolate: 40814.972 us, 0.80%
- parameter: 3576.982 us, 0.07%

Optimizations applied:
- Fused the base and momentum matrix products into one CSR multiply. This exploits the exact signed two-block structure of `t0_state__d_gamma`: it applies the same sparse operator as `t0_state` to the difference between the two state blocks.
- Hoisted the parameter matrix-vector product outside the iteration loop. This exploits that `p0` is invariant throughout a solve.
- Replaced `su_state` and `su_t_t0` CSR operations with direct block updates. These matrices are identity/permutation blocks implementing `[old second state block, t0]`.
- Computed the residual only on the final iteration. This exploits the generated max-iteration-only termination configuration (`FOM_USE_TOL == 0`).
- Replaced branching soft-thresholding with comparison masks. This preserves the three threshold cases while enabling efficient vectorized, branch-free execution.
- Retained profiling markers around all principal computational regions.
```
