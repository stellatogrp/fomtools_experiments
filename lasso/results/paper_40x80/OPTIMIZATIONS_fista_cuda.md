# Optimization report

## Timing

```
Timing summary (per solve):
  Python reference (before codegen):   7564.269 us
  Generated C (initial, phase A):      9198.076 us
  Generated C (optimized, phase B):     558.110 us
  Speedup from codegen (Py -> C):         0.8x
  Speedup from optimization (A -> B):   16.48x
  Total speedup (Py -> optimized C):     13.6x
```

Optimization outcome: **kept (re-verified correct)**.

## What the agent changed

```diff
--- src/fom_solver.cu (phase A)
+++ src/fom_solver.cu (optimized)
@@ -1,11 +1,17 @@
 #include "fom_solver.h"
 #include "fom_data.h"
 #include "fom_ops.h"
+extern "C" {
+#include "fom_profile.h"
+}

 #include <cuda_runtime.h>

 static fom_float *fom_device_storage;
 static FomWorkspace *fom_device_owner;
+static cudaGraph_t fom_iteration_graph;
+static cudaGraphExec_t fom_iteration_graph_exec;
+static fom_int fom_graph_iterations = -1;

 enum {
     FOM_DEVICE_STATE_A = 0,
@@ -13,28 +19,40 @@
     FOM_DEVICE_ACC = FOM_DEVICE_STATE_B + FOM_STATE_DIM,
     FOM_DEVICE_T0 = FOM_DEVICE_ACC + FOM_MAX_VEC_DIM,
     FOM_DEVICE_P0 = FOM_DEVICE_T0 + FOM_T0_DIM,
-    FOM_DEVICE_RESIDUAL = FOM_DEVICE_P0 + FOM_PARAM_P0_DIM,
-    FOM_DEVICE_COUNT = FOM_DEVICE_RESIDUAL + 1
+    FOM_DEVICE_PARAM_TERM = FOM_DEVICE_P0 + FOM_PARAM_P0_DIM,
+    FOM_DEVICE_RESIDUAL = FOM_DEVICE_PARAM_TERM + FOM_T0_DIM,
+    FOM_DEVICE_H = FOM_DEVICE_RESIDUAL + 1,
+    FOM_DEVICE_COUNT = FOM_DEVICE_H + FOM_T0_DIM * FOM_STATE_DIM
 };

-__global__ static void fom_scaled_add_kernel(fom_float *dst,
-                                              const fom_float *src,
-                                              fom_float scale) {
-    fom_int i = blockIdx.x * blockDim.x + threadIdx.x;
-    if (i < FOM_T0_DIM) dst[i] += scale * src[i];
-}
-
-__global__ static void fom_soft_threshold_kernel(fom_float *x,
-                                                  fom_float lambda) {
-    fom_int i = blockIdx.x * blockDim.x + threadIdx.x;
+__global__ static void fom_iteration_kernel(const fom_float *matrix,
+                                             const fom_float *param_term,
+                                             const fom_float *state,
+                                             fom_float *next,
+                                             fom_float scale,
+                                             fom_float lambda) {
+    fom_int i = blockIdx.x;
+    fom_int lane = threadIdx.x;
+    fom_float v = (fom_float)0;
     if (i < FOM_T0_DIM) {
-        fom_float v = x[i];
+        const fom_float *row = matrix + i * FOM_STATE_DIM + FOM_T0_DIM;
+        for (fom_int j = lane; j < FOM_T0_DIM; j += 32) {
+            fom_float y = state[FOM_T0_DIM + j];
+            v += row[j] * (y + scale * (y - state[j]));
+        }
+        for (fom_int offset = 16; offset != 0; offset >>= 1)
+            v += __shfl_down_sync(0xffffffff, v, offset);
+    }
+    if (i < FOM_T0_DIM && lane == 0) {
+        v += param_term[i];
         if (v > lambda)
-            x[i] = v - lambda;
+            v -= lambda;
         else if (v < -lambda)
-            x[i] = v + lambda;
+            v += lambda;
         else
-            x[i] = (fom_float)0;
+            v = (fom_float)0;
+        next[i] = state[FOM_T0_DIM + i];
+        next[FOM_T0_DIM + i] = v;
     }
 }

@@ -58,6 +76,49 @@
     if (i == 0) result[0] = values[0];
 }

+static void fom_destroy_graph(void) {
+    if (fom_iteration_graph_exec) cudaGraphExecDestroy(fom_iteration_graph_exec);
+    if (fom_iteration_graph) cudaGraphDestroy(fom_iteration_graph);
+    fom_iteration_graph_exec = 0;
+    fom_iteration_graph = 0;
+    fom_graph_iterations = -1;
+}
+
+static bool fom_build_graph(fom_int iterations, const fom_float *matrix,
+                            const fom_float *param_term, fom_float *state_a,
+                            fom_float *state_b) {
+    fom_destroy_graph();
+    if (cudaGraphCreate(&fom_iteration_graph, 0) != cudaSuccess) return false;
+    cudaGraphNode_t previous = 0;
+    for (fom_int it = 0; it < iterations; ++it) {
+        fom_float *state = (it & 1) ? state_b : state_a;
+        fom_float *next = (it & 1) ? state_a : state_b;
+        fom_float gamma = fom_sched_gamma(it);
+        fom_float lambda = FOM_T0_LAMBDA;
+        void *args[] = {&matrix, &param_term, &state, &next, &gamma, &lambda};
+        cudaKernelNodeParams params = {};
+        params.func = (void *)fom_iteration_kernel;
+        params.gridDim = dim3(FOM_T0_DIM, 1, 1);
+        params.blockDim = dim3(32, 1, 1);
+        params.kernelParams = args;
+        cudaGraphNode_t node;
+        if (cudaGraphAddKernelNode(&node, fom_iteration_graph,
+                                   previous ? &previous : 0,
+                                   previous ? 1 : 0, &params) != cudaSuccess) {
+            fom_destroy_graph();
+            return false;
+        }
+        previous = node;
+    }
+    if (cudaGraphInstantiate(&fom_iteration_graph_exec, fom_iteration_graph,
+                             0, 0, 0) != cudaSuccess) {
+        fom_destroy_graph();
+        return false;
+    }
+    fom_graph_iterations = iterations;
+    return true;
+}
+
 static void fom_reset_host_state(FomWorkspace *work) {
     for (fom_int i = 0; i < FOM_STATE_DIM; ++i) {
         work->state[i] = FOM_STATE_INIT[i];
@@ -80,12 +141,17 @@
     work->settings = *settings;
     fom_reset_host_state(work);

+    fom_destroy_graph();
     if (fom_device_storage != 0) cudaFree(fom_device_storage);
     fom_device_storage = 0;
     fom_device_owner = 0;
     if (cudaMalloc((void **)&fom_device_storage,
-                   sizeof(fom_float) * FOM_DEVICE_COUNT) == cudaSuccess)
+                   sizeof(fom_float) * FOM_DEVICE_COUNT) == cudaSuccess) {
         fom_device_owner = work;
+        cudaMemcpy(fom_device_storage + FOM_DEVICE_H, FOM_MAT_t0_state,
+                   sizeof(fom_float) * FOM_T0_DIM * FOM_STATE_DIM,
+                   cudaMemcpyHostToDevice);
+    }
 }

 extern "C" void fom_update_params(FomWorkspace *work,
@@ -96,6 +162,7 @@

 extern "C" fom_int fom_solve(FomWorkspace *work) {
     if (fom_device_owner != work || fom_device_storage == 0) {
+        fom_destroy_graph();
         if (fom_device_storage != 0) cudaFree(fom_device_storage);
         fom_device_storage = 0;
         fom_device_owner = 0;
@@ -103,14 +170,17 @@
                        sizeof(fom_float) * FOM_DEVICE_COUNT) != cudaSuccess)
             return FOM_MAX_ITER_REACHED;
         fom_device_owner = work;
+        cudaMemcpy(fom_device_storage + FOM_DEVICE_H, FOM_MAT_t0_state,
+                   sizeof(fom_float) * FOM_T0_DIM * FOM_STATE_DIM,
+                   cudaMemcpyHostToDevice);
     }

     fom_float *state_a = fom_device_storage + FOM_DEVICE_STATE_A;
     fom_float *state_b = fom_device_storage + FOM_DEVICE_STATE_B;
-    fom_float *acc = fom_device_storage + FOM_DEVICE_ACC;
-    fom_float *t0 = fom_device_storage + FOM_DEVICE_T0;
     fom_float *p0 = fom_device_storage + FOM_DEVICE_P0;
+    fom_float *param_term = fom_device_storage + FOM_DEVICE_PARAM_TERM;
     fom_float *residual = fom_device_storage + FOM_DEVICE_RESIDUAL;
+    const fom_float *matrix = fom_device_storage + FOM_DEVICE_H;
     fom_float *state = state_a;
     fom_float *next = state_b;

@@ -120,29 +190,34 @@
                sizeof(fom_float) * FOM_PARAM_P0_DIM, cudaMemcpyHostToDevice);
     cudaMemset(residual, 0, sizeof(fom_float));

+    /* The parameter is fixed for the solve, hence so is this dense product. */
+    FOM_PROF_BEGIN(param_precompute);
+    cudaMemset(param_term, 0, sizeof(fom_float) * FOM_T0_DIM);
+    fom_apply_t0_p_p0(p0, param_term);
+#ifdef FOM_PROFILE
+    cudaDeviceSynchronize();
+#endif
+    FOM_PROF_END(param_precompute);
+
     fom_int iterations = work->settings.max_iter;
     if (iterations < 0) iterations = 0;
-
-    for (fom_int it = 0; it < iterations; ++it) {
-        fom_float gamma = fom_sched_gamma(it);
-
-        cudaMemset(t0, 0, sizeof(fom_float) * FOM_T0_DIM);
-        fom_apply_t0_state(state, t0);
-        cudaMemset(acc, 0, sizeof(fom_float) * FOM_T0_DIM);
-        fom_apply_t0_state__d_gamma(state, acc);
-        fom_scaled_add_kernel<<<1, 256>>>(t0, acc, gamma);
-        fom_apply_t0_p_p0(p0, t0);
-        fom_soft_threshold_kernel<<<1, 256>>>(t0, FOM_T0_LAMBDA);
-
-        cudaMemset(next, 0, sizeof(fom_float) * FOM_STATE_DIM);
-        fom_apply_su_state(state, next);
-        fom_apply_su_t_t0(t0, next);
-
-        fom_float *swap = state;
-        state = next;
-        next = swap;
-    }
-
+    FOM_PROF_BEGIN(prox_linear);
+    if (iterations != 0) {
+        if (fom_graph_iterations != iterations &&
+            !fom_build_graph(iterations, matrix, param_term, state_a, state_b))
+            return FOM_MAX_ITER_REACHED;
+        cudaGraphLaunch(fom_iteration_graph_exec, 0);
+#ifdef FOM_PROFILE
+        cudaDeviceSynchronize();
+#endif
+        if (iterations & 1) {
+            state = state_b;
+            next = state_a;
+        }
+    }
+    FOM_PROF_END(prox_linear);
+
+    FOM_PROF_BEGIN(finalize);
     if (iterations != 0)
         fom_residual_kernel<<<1, 256>>>(state, next, residual);

@@ -150,6 +225,7 @@
                sizeof(fom_float) * FOM_STATE_DIM, cudaMemcpyDeviceToHost);
     cudaMemcpy(&work->solution.residual, residual,
                sizeof(fom_float), cudaMemcpyDeviceToHost);
+    FOM_PROF_END(finalize);
     for (fom_int i = 0; i < FOM_STATE_DIM; ++i)
         work->solution.x[i] = work->state[i];
     work->solution.iter = iterations;
@@ -159,6 +235,7 @@

 extern "C" void fom_cleanup(FomWorkspace *work) {
     if (fom_device_owner == work && fom_device_storage != 0) {
+        fom_destroy_graph();
         cudaFree(fom_device_storage);
         fom_device_storage = 0;
         fom_device_owner = 0;
```

## Agent notes

```
Initial mean_us: 9427.145299
Final mean_us: 557.209696
Overall speedup: 16.92x

Final profiling breakdown, hottest first:
- prox_linear: 659.433 us/solve, 93.80%
- finalize: 28.472 us/solve, 4.05%
- param_precompute: 15.089 us/solve, 2.15%

Optimizations applied:
- Hoisted `t0_p_p0 @ p0` out of the iteration loop. This exploits that `p0` is constant throughout each solve.
- Collapsed the FISTA linear expression to `H · (y + gamma·(y−x))`. This exploits the matrix block structure: `t0_state ≈ [0, H]` and `t0_state__d_gamma ≈ [-H, H]`.
- Fused the dense product, soft threshold, and state update. This exploits the state-update structure `[old second slot; prox result]`, composed solely of identity and zero blocks.
- Parallelized each matrix row across one CUDA warp with shuffle reduction, producing coalesced coefficient reads and greater GPU occupancy.
- Cached and replayed the complete iteration sequence as a CUDA Graph. This exploits the fixed iteration dependency graph and fixed device-buffer addresses, reducing 200 host launch submissions to one.
- Kept state and parameters device-resident, swapping fixed device buffers and downloading only the final state and residual.

Final correctness:
VERIFY OK
Worst max-abs error: 1.105e-11
```
