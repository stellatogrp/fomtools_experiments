# Optimization report

## Timing

```
Timing summary (per solve):
  Python reference (before codegen): 294402.776 us
  Generated C (initial, phase A):      8124.147 us
  Generated C (optimized, phase B):    5414.754 us
  Speedup from codegen (Py -> C):        36.2x
  Speedup from optimization (A -> B):    1.50x
  Total speedup (Py -> optimized C):     54.4x
```

Optimization outcome: **kept (re-verified correct)**.

## What the agent changed

```diff
--- src/fom_solver.cu (phase A)
+++ src/fom_solver.cu (optimized)
@@ -1,12 +1,21 @@
 #include "fom_solver.h"
 #include "fom_data.h"
 #include "fom_ops.h"
+extern "C" {
+#include "fom_profile.h"
+}

 #include <cuda_runtime.h>
 #include <string.h>

-#define FOM_CUDA_THREADS 256
-#define FOM_CUDA_BLOCKS 79
+#define FOM_CUDA_THREADS 512
+#define FOM_CUDA_BLOCKS 40
+
+#ifdef FOM_PROFILE
+#define FOM_PROF_SYNC() cudaDeviceSynchronize()
+#else
+#define FOM_PROF_SYNC() ((void)0)
+#endif

 static fom_float *fom_d_state;
 static fom_float *fom_d_new_state;
@@ -88,20 +97,34 @@
     cudaMemcpy(fom_d_p0, work->params.p0,
                sizeof(fom_float) * FOM_PARAM_P0_DIM, cudaMemcpyHostToDevice);

+    /* t0_p_p0 is invariant across all ISTA iterations. */
+    FOM_PROF_BEGIN(parameter_precompute);
+    cudaMemset(fom_d_t0, 0, t0_bytes);
+    fom_apply_t0_p_p0(fom_d_p0, fom_d_t0);
+    FOM_PROF_SYNC();
+    FOM_PROF_END(parameter_precompute);
+
     for (iter = 0; iter < work->settings.max_iter; ++iter) {
-        cudaMemset(fom_d_acc, 0, t0_bytes);
+        FOM_PROF_BEGIN(recurrent_linear);
+        cudaMemcpyAsync(fom_d_acc, fom_d_t0, t0_bytes,
+                        cudaMemcpyDeviceToDevice);
         fom_apply_t0_state(state, fom_d_acc);
-        fom_apply_t0_p_p0(fom_d_p0, fom_d_acc);
+        FOM_PROF_SYNC();
+        FOM_PROF_END(recurrent_linear);
+
+        FOM_PROF_BEGIN(activation_identity_update);
         fom_soft_threshold_kernel<<<FOM_CUDA_BLOCKS, FOM_CUDA_THREADS>>>(
-            fom_d_acc, fom_d_t0, FOM_T0_LAMBDA);
-
-        cudaMemset(next, 0, state_bytes);
-        fom_apply_su_t_t0(fom_d_t0, next);
+            fom_d_acc, next, FOM_T0_LAMBDA);
+        FOM_PROF_SYNC();
+        FOM_PROF_END(activation_identity_update);

         if (iter + 1 == work->settings.max_iter) {
+            FOM_PROF_BEGIN(residual);
             cudaMemset(fom_d_residual, 0, sizeof(unsigned long long));
             fom_residual_kernel<<<FOM_CUDA_BLOCKS, FOM_CUDA_THREADS>>>(
                 next, state, fom_d_residual);
+            FOM_PROF_SYNC();
+            FOM_PROF_END(residual);
         }

         fom_float *swap = state;
```

## Agent notes

```
Initial mean_us: 8079.658540
Final mean_us: 5395.773020
Overall speedup: 1.497×
Correctness: VERIFY OK; worst max-abs error 4.263e-14

Final profiling breakdown, hottest first:
- recurrent_linear: 405624.4090 us, 70.16%, 10000 calls
- activation_identity_update: 165828.6920 us, 28.68%, 10000 calls
- residual: 4170.8580 us, 0.72%, 50 calls
- parameter_precompute: 2531.5580 us, 0.44%, 50 calls

Optimizations:
- Exploited the parameter term’s iteration invariance: computed sparse `t0_p_p0 @ p0` once per solve and reused it across all 200 iterations.
- Exploited `su_t_t0` being exactly the identity matrix: wrote soft-threshold results directly into the next state, eliminating 200 identity CSR launches and destination memsets.
- Tuned elementwise CUDA kernels to 512 threads and 40 blocks for the fixed 20,000-element vectors.
- Preserved device-resident state, pointer swapping, one final host download, and production-no-op profiling markers.
```
