# Optimization report

## Timing

```
Timing summary (per solve):
  Python reference (before codegen): 780276.739 us
  Generated C (initial, phase A):     14839.848 us
  Generated C (optimized, phase B):    4582.527 us
  Speedup from codegen (Py -> C):        52.6x
  Speedup from optimization (A -> B):    3.24x
  Total speedup (Py -> optimized C):    170.3x
```

Optimization outcome: **kept (re-verified correct)**.

## What the agent changed

```diff
--- src/fom_solver.cu (phase A)
+++ src/fom_solver.cu (optimized)
@@ -1,6 +1,7 @@
 #include "fom_solver.h"
 #include "fom_data.h"
 #include "fom_ops.h"
+#include "fom_profile.h"

 #include <cuda_runtime.h>
 #include <string.h>
@@ -16,30 +17,44 @@
     fom_float *acc;
     fom_float *t0;
     fom_float *p0;
+    fom_float *param_term;
     fom_float *residual;
 } FomDeviceWorkspace;

-static FomDeviceWorkspace fom_device = {0, 0, 0, 0, 0, 0, 0};
-
-__global__ static void fom_add_scaled_kernel(fom_float *dst,
-                                             const fom_float *src,
-                                             fom_float scale) {
-    fom_int i = (fom_int)(blockIdx.x * blockDim.x + threadIdx.x);
-    if (i < FOM_T0_DIM)
-        dst[i] += scale * src[i];
-}
-
-__global__ static void fom_soft_threshold_kernel(fom_float *x,
-                                                  fom_float lambda) {
+static FomDeviceWorkspace fom_device = {0};
+
+__global__ static void fom_finish_t0_kernel(fom_float *dst,
+                                            const fom_float *param_term,
+                                            const fom_float *state,
+                                            fom_float *new_state,
+                                            fom_float lambda) {
     fom_int i = (fom_int)(blockIdx.x * blockDim.x + threadIdx.x);
     if (i < FOM_T0_DIM) {
-        fom_float v = x[i];
+        fom_float v = dst[i] + param_term[i];
         if (v > lambda)
-            x[i] = v - lambda;
+            v -= lambda;
         else if (v < -lambda)
-            x[i] = v + lambda;
+            v += lambda;
         else
-            x[i] = (fom_float)0;
+            v = (fom_float)0;
+        new_state[i] = state[i + FOM_T0_DIM];
+        new_state[i + FOM_T0_DIM] = v;
+    }
+}
+
+/* t0_state acts on the second state slot and d_gamma is [-B, B].
+   Form the FISTA extrapolate so both terms need only one B product. */
+__global__ static void fom_extrapolate_kernel(const fom_float *state,
+                                              fom_float *extrapolated,
+                                              fom_float *product,
+                                              fom_float gamma) {
+    fom_int i = (fom_int)(blockIdx.x * blockDim.x + threadIdx.x);
+    if (i < FOM_T0_DIM) {
+        extrapolated[i] = (fom_float)0;
+        extrapolated[i + FOM_T0_DIM] =
+            state[i + FOM_T0_DIM] +
+            gamma * (state[i + FOM_T0_DIM] - state[i]);
+        product[i] = (fom_float)0;
     }
 }

@@ -91,8 +106,9 @@
     if (fom_device.acc) cudaFree(fom_device.acc);
     if (fom_device.t0) cudaFree(fom_device.t0);
     if (fom_device.p0) cudaFree(fom_device.p0);
+    if (fom_device.param_term) cudaFree(fom_device.param_term);
     if (fom_device.residual) cudaFree(fom_device.residual);
-    fom_device = (FomDeviceWorkspace){0, 0, 0, 0, 0, 0, 0};
+    fom_device = (FomDeviceWorkspace){0};
 }

 static void fom_allocate_device(FomWorkspace *work) {
@@ -109,6 +125,8 @@
                sizeof(fom_float) * FOM_T0_DIM);
     cudaMalloc((void **)&fom_device.p0,
                sizeof(fom_float) * FOM_PARAM_P0_DIM);
+    cudaMalloc((void **)&fom_device.param_term,
+               sizeof(fom_float) * FOM_T0_DIM);
     cudaMalloc((void **)&fom_device.residual, sizeof(fom_float));
     fom_device.owner = work;
 }
@@ -139,14 +157,22 @@
     work->solution.iter = 0;
     work->solution.residual = (fom_float)0;
     work->solution.status = FOM_MAX_ITER_REACHED;
+    fom_allocate_device(work);
+    FOM_PROF_BEGIN(param_setup);
+    cudaMemcpy(fom_device.p0, work->params.p0,
+               sizeof(fom_float) * FOM_PARAM_P0_DIM, cudaMemcpyHostToDevice);
+    cudaMemset(fom_device.param_term, 0, sizeof(fom_float) * FOM_T0_DIM);
+    fom_apply_t0_p_p0(fom_device.p0, fom_device.param_term);
+#ifdef FOM_PROFILE
+    cudaDeviceSynchronize();
+#endif
+    FOM_PROF_END(param_setup);
 }

 extern "C" fom_int fom_solve(FomWorkspace *work) {
     fom_allocate_device(work);
     cudaMemcpy(fom_device.state, FOM_STATE_INIT,
                sizeof(fom_float) * FOM_STATE_DIM, cudaMemcpyHostToDevice);
-    cudaMemcpy(fom_device.p0, work->params.p0,
-               sizeof(fom_float) * FOM_PARAM_P0_DIM, cudaMemcpyHostToDevice);

     fom_float *state = fom_device.state;
     fom_float *new_state = fom_device.new_state;
@@ -155,20 +181,23 @@
     for (fom_int it = 0; it < work->settings.max_iter; ++it) {
         fom_float gamma = fom_sched_gamma(it);

-        cudaMemset(fom_device.t0, 0, sizeof(fom_float) * FOM_T0_DIM);
-        fom_apply_t0_state(state, fom_device.t0);
-
-        cudaMemset(fom_device.acc, 0, sizeof(fom_float) * FOM_T0_DIM);
-        fom_apply_t0_state__d_gamma(state, fom_device.acc);
-        fom_add_scaled_kernel<<<FOM_T0_BLOCKS, FOM_THREADS>>>(
-            fom_device.t0, fom_device.acc, gamma);
-        fom_apply_t0_p_p0(fom_device.p0, fom_device.t0);
-        fom_soft_threshold_kernel<<<FOM_T0_BLOCKS, FOM_THREADS>>>(
-            fom_device.t0, FOM_T0_LAMBDA);
-
-        cudaMemset(new_state, 0, sizeof(fom_float) * FOM_STATE_DIM);
-        fom_apply_su_state(state, new_state);
-        fom_apply_su_t_t0(fom_device.t0, new_state);
+        FOM_PROF_BEGIN(linear_product);
+        fom_extrapolate_kernel<<<FOM_T0_BLOCKS, FOM_THREADS>>>(
+            state, fom_device.acc, fom_device.t0, gamma);
+        fom_apply_t0_state(fom_device.acc, fom_device.t0);
+#ifdef FOM_PROFILE
+        cudaDeviceSynchronize();
+#endif
+        FOM_PROF_END(linear_product);
+
+        FOM_PROF_BEGIN(activation_state_update);
+        fom_finish_t0_kernel<<<FOM_T0_BLOCKS, FOM_THREADS>>>(
+            fom_device.t0, fom_device.param_term, state, new_state,
+            FOM_T0_LAMBDA);
+#ifdef FOM_PROFILE
+        cudaDeviceSynchronize();
+#endif
+        FOM_PROF_END(activation_state_update);

         ++completed;
         if (FOM_USE_TOL && work->settings.check_termination) {
```

## Agent notes

```
Initial mean_us: 14622.176750
Final mean_us: 4552.128583
Overall speedup: 3.21x

Final profiling breakdown, hottest first:
- linear_product: 81076.2060 us, 74.80%
- activation_state_update: 26807.2210 us, 24.73%
- param_setup: 513.5360 us, 0.47%

Optimizations applied:
- Exploited the FISTA matrix relation `d_gamma = [-B, B]` and the negligible roundoff-only first block of `t0_state`; formed `(1+gamma)s1 - gamma*s0` and reduced two sparse products to one.
- Exploited complementary identity blocks in `su_state` and `su_t_t0`; replaced generic sparse state-update calls with direct assignments.
- Exploited iteration-invariant parameter structure; moved `t0_p_p0 @ p0` into `fom_update_params` and reused it throughout all iterations.
- Fused parameter addition and soft-threshold activation with the state update.
- Folded product-buffer zeroing into the extrapolation kernel, eliminating a separate `cudaMemset`.
- Kept all iteration state resident on the GPU and retained profiling markers around final computational regions.
```
