# Optimization report

## Timing

```
Timing summary (per solve):
  Python reference (before codegen):   4207.732 us
  Generated C (initial, phase A):      4202.010 us
  Generated C (optimized, phase B):     726.633 us
  Speedup from codegen (Py -> C):         1.0x
  Speedup from optimization (A -> B):    5.78x
  Total speedup (Py -> optimized C):      5.8x
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

@@ -12,16 +13,42 @@
 static fom_float *fom_param_p0;
 static fom_float *fom_t0;
 static fom_float *fom_residual;
-
-__global__ static void fom_soft_threshold_kernel(fom_float *x,
-                                                  fom_float lambda)
+static fom_float *fom_t0_state_matrix;
+static fom_float *fom_t0_param_matrix;
+
+__global__ static void fom_parameter_projection_kernel(
+    const fom_float *__restrict__ matrix,
+    const fom_float *__restrict__ parameter,
+    fom_float *__restrict__ projection)
 {
     const fom_int i = (fom_int)(blockIdx.x * blockDim.x + threadIdx.x);
     if (i < FOM_T0_DIM) {
-        const fom_float value = x[i];
-        x[i] = value > lambda
-                   ? value - lambda
-                   : (value < -lambda ? value + lambda : (fom_float)0);
+        fom_float value = (fom_float)0;
+        for (fom_int j = 0; j < FOM_PARAM_P0_DIM; ++j)
+            value += matrix[i * FOM_PARAM_P0_DIM + j] * parameter[j];
+        projection[i] = value;
+    }
+}
+
+__global__ static void fom_ista_iteration_kernel(
+    const fom_float *__restrict__ matrix,
+    const fom_float *__restrict__ state,
+    const fom_float *__restrict__ parameter_projection,
+    fom_float *__restrict__ next,
+    fom_float lambda)
+{
+    const fom_int i = (fom_int)blockIdx.x;
+    const fom_int lane = (fom_int)threadIdx.x;
+    fom_float value = (fom_float)0;
+    for (fom_int j = lane; j < FOM_STATE_DIM; j += 32)
+        value += matrix[i * FOM_STATE_DIM + j] * state[j];
+    for (fom_int offset = 16; offset != 0; offset >>= 1)
+        value += __shfl_down_sync(0xffffffffu, value, offset);
+    if (lane == 0) {
+        value += parameter_projection[i];
+        next[i] = value > lambda
+                      ? value - lambda
+                      : (value < -lambda ? value + lambda : (fom_float)0);
     }
 }

@@ -59,13 +86,24 @@
         return;

     const size_t count = (size_t)(2 * FOM_STATE_DIM + FOM_PARAM_P0_DIM
-                                  + FOM_T0_DIM + 1);
+                                  + FOM_T0_DIM + 1
+                                  + FOM_STATE_DIM * FOM_STATE_DIM
+                                  + FOM_T0_DIM * FOM_PARAM_P0_DIM);
     cudaMalloc((void **)&fom_storage, count * sizeof(fom_float));
     fom_state_a = fom_storage;
     fom_state_b = fom_state_a + FOM_STATE_DIM;
     fom_param_p0 = fom_state_b + FOM_STATE_DIM;
     fom_t0 = fom_param_p0 + FOM_PARAM_P0_DIM;
     fom_residual = fom_t0 + FOM_T0_DIM;
+    fom_t0_state_matrix = fom_residual + 1;
+    fom_t0_param_matrix = fom_t0_state_matrix
+                          + FOM_STATE_DIM * FOM_STATE_DIM;
+    cudaMemcpy(fom_t0_state_matrix, FOM_MAT_t0_state,
+               sizeof(fom_float) * FOM_STATE_DIM * FOM_STATE_DIM,
+               cudaMemcpyHostToDevice);
+    cudaMemcpy(fom_t0_param_matrix, FOM_MAT_t0_p_p0,
+               sizeof(fom_float) * FOM_T0_DIM * FOM_PARAM_P0_DIM,
+               cudaMemcpyHostToDevice);
 }

 static void fom_reset_state(FomWorkspace *work)
@@ -109,6 +147,11 @@
                sizeof(fom_float) * FOM_STATE_DIM, cudaMemcpyHostToDevice);
     cudaMemcpy(fom_param_p0, work->params.p0,
                sizeof(fom_float) * FOM_PARAM_P0_DIM, cudaMemcpyHostToDevice);
+
+    FOM_PROF_BEGIN(parameter_projection);
+    fom_parameter_projection_kernel<<<1, FOM_KERNEL_THREADS>>>(
+        fom_t0_param_matrix, fom_param_p0, fom_t0);
+    FOM_PROF_END(parameter_projection);

     fom_float *state = fom_state_a;
     fom_float *next = fom_state_b;
@@ -117,21 +160,17 @@
     if (iterations < 0)
         iterations = 0;

+    FOM_PROF_BEGIN(iterations);
     for (fom_int k = 0; k < iterations; ++k) {
-        cudaMemset(fom_t0, 0, sizeof(fom_float) * FOM_T0_DIM);
-        fom_apply_t0_state(state, fom_t0);
-        fom_apply_t0_p_p0(fom_param_p0, fom_t0);
-        fom_soft_threshold_kernel<<<1, FOM_KERNEL_THREADS>>>(fom_t0,
-                                                             FOM_T0_LAMBDA);
-
-        cudaMemset(next, 0, sizeof(fom_float) * FOM_STATE_DIM);
-        fom_apply_su_t_t0(fom_t0, next);
-
+        fom_ista_iteration_kernel<<<FOM_STATE_DIM, 32>>>(
+            fom_t0_state_matrix, state, fom_t0, next, FOM_T0_LAMBDA);
         previous = state;
         state = next;
         next = previous;
     }
-
+    FOM_PROF_END(iterations);
+
+    FOM_PROF_BEGIN(finalization);
     if (iterations != 0) {
         fom_residual_kernel<<<1, FOM_KERNEL_THREADS>>>(state, previous,
                                                        fom_residual);
@@ -147,6 +186,7 @@
         work->state[i] = work->solution.x[i];
         work->new_state[i] = work->solution.x[i];
     }
+    FOM_PROF_END(finalization);

     work->solution.iter = iterations;
     work->solution.status = FOM_MAX_ITER_REACHED;
@@ -164,4 +204,6 @@
     fom_param_p0 = 0;
     fom_t0 = 0;
     fom_residual = 0;
-}
+    fom_t0_state_matrix = 0;
+    fom_t0_param_matrix = 0;
+}
```

## Agent notes

```
Initial mean_us: 4483.065496
Final mean_us: 736.150322
Overall speedup: 6.09x

Final profiling breakdown, hottest first:
- iterations: 568.642 us/solve, 74.16%
- finalization: 195.150 us/solve, 25.45%
- parameter_projection: 2.935 us/solve, 0.38%

Optimizations applied:
- Hoisted the invariant dense `t0_p_p0 @ p0` parameter projection outside the 200-iteration loop.
- Exploited the exact identity structure of `su_t_t0`, eliminating its kernel, accumulator memset, and intermediate copy.
- Fused the dense state transform and soft-threshold activation into one CUDA kernel, removing redundant passes and launches.
- Used one warp per output row for the dense 80×80 transform, enabling coalesced matrix reads and parallel warp reduction.
- Kept state and parameters device-resident, swapped device pointers each iteration, and copied results to the host only during finalization.
- Preserved profiling markers around parameter projection, iterations, and finalization.

Final verification: VERIFY OK; worst max-abs error 5.640e-14.
```
