# Optimization report

## Timing

```
Timing summary (per solve):
  Python reference (before codegen): 1096264.321 us
  Generated C (initial, phase A):     18844.389 us
  Generated C (optimized, phase B):    5507.766 us
  Speedup from codegen (Py -> C):        58.2x
  Speedup from optimization (A -> B):    3.42x
  Total speedup (Py -> optimized C):    199.0x
```

Optimization outcome: **kept (re-verified correct)**.

## What the agent changed

```diff
--- src/fom_solver.cu (phase A)
+++ src/fom_solver.cu (optimized)
@@ -21,25 +21,41 @@
 static fom_float *fom_d_state;
 static fom_float *fom_d_new_state;
 static fom_float *fom_d_acc;
-static fom_float *fom_d_gamma_acc;
-static fom_float *fom_d_t0;
 static fom_float *fom_d_param_term;
 static fom_float *fom_d_p0;
 static unsigned long long *fom_d_residual;

 __global__ static void fom_gamma_threshold_kernel(fom_float *linear,
-                                                   const fom_float *delta,
-                                                   fom_float *result,
-                                                   fom_float gamma,
+                                                   const fom_float *state,
+                                                   fom_float *next,
                                                    fom_float lambda)
 {
     fom_int i = (fom_int)(blockIdx.x * blockDim.x + threadIdx.x);
     if (i < FOM_T0_DIM) {
-        fom_float value = linear[i] + gamma * delta[i];
-        linear[i] = value;
-        result[i] = value > lambda ? value - lambda
-                  : value < -lambda ? value + lambda
-                  : (fom_float)0;
+        fom_float value = linear[i];
+        next[i] = state[FOM_T0_DIM + i];
+        next[FOM_T0_DIM + i] = value > lambda ? value - lambda
+                                    : value < -lambda ? value + lambda
+                                    : (fom_float)0;
+    }
+}
+
+/* The FISTA state is [x_{k-1}; x_k].  The generated recurrent matrices are
+   B*x_k and B*(x_k-x_{k-1}); form their scheduled argument directly so the
+   large sparse operator is traversed only once. */
+__global__ static void fom_extrapolate_kernel(const fom_float *state,
+                                               fom_float *extrapolated,
+                                               const fom_float *param_term,
+                                               fom_float *acc,
+                                               fom_float gamma)
+{
+    fom_int i = (fom_int)(blockIdx.x * blockDim.x + threadIdx.x);
+    if (i < FOM_T0_DIM) {
+        extrapolated[i] = (fom_float)0;
+        extrapolated[FOM_T0_DIM + i] =
+            state[FOM_T0_DIM + i] +
+            gamma * (state[FOM_T0_DIM + i] - state[i]);
+        acc[i] = param_term[i];
     }
 }

@@ -85,10 +101,6 @@
                    sizeof(fom_float) * FOM_STATE_DIM);
         cudaMalloc((void **)&fom_d_acc,
                    sizeof(fom_float) * FOM_T0_DIM);
-        cudaMalloc((void **)&fom_d_gamma_acc,
-                   sizeof(fom_float) * FOM_T0_DIM);
-        cudaMalloc((void **)&fom_d_t0,
-                   sizeof(fom_float) * FOM_T0_DIM);
         cudaMalloc((void **)&fom_d_param_term,
                    sizeof(fom_float) * FOM_T0_DIM);
         cudaMalloc((void **)&fom_d_p0,
@@ -128,26 +140,17 @@
         const fom_float gamma = fom_sched_gamma(iter);

         FOM_PROF_BEGIN(recurrent_linear);
-        cudaMemcpyAsync(fom_d_acc, fom_d_param_term, t0_bytes,
-                        cudaMemcpyDeviceToDevice);
-        fom_apply_t0_state(state, fom_d_acc);
-        cudaMemset(fom_d_gamma_acc, 0, t0_bytes);
-        fom_apply_t0_state__d_gamma(state, fom_d_gamma_acc);
+        fom_extrapolate_kernel<<<FOM_T0_BLOCKS, FOM_CUDA_THREADS>>>(
+            state, next, fom_d_param_term, fom_d_acc, gamma);
+        fom_apply_t0_state(next, fom_d_acc);
         FOM_PROF_SYNC();
         FOM_PROF_END(recurrent_linear);

-        FOM_PROF_BEGIN(activation);
+        FOM_PROF_BEGIN(activation_state_update);
         fom_gamma_threshold_kernel<<<FOM_T0_BLOCKS, FOM_CUDA_THREADS>>>(
-            fom_d_acc, fom_d_gamma_acc, fom_d_t0, gamma, FOM_T0_LAMBDA);
+            fom_d_acc, state, next, FOM_T0_LAMBDA);
         FOM_PROF_SYNC();
-        FOM_PROF_END(activation);
-
-        FOM_PROF_BEGIN(state_update);
-        cudaMemset(next, 0, state_bytes);
-        fom_apply_su_state(state, next);
-        fom_apply_su_t_t0(fom_d_t0, next);
-        FOM_PROF_SYNC();
-        FOM_PROF_END(state_update);
+        FOM_PROF_END(activation_state_update);

         if (iter + 1 == work->settings.max_iter) {
             FOM_PROF_BEGIN(residual);
@@ -187,16 +190,12 @@
     cudaFree(fom_d_state);
     cudaFree(fom_d_new_state);
     cudaFree(fom_d_acc);
-    cudaFree(fom_d_gamma_acc);
-    cudaFree(fom_d_t0);
     cudaFree(fom_d_param_term);
     cudaFree(fom_d_p0);
     cudaFree(fom_d_residual);
     fom_d_state = 0;
     fom_d_new_state = 0;
     fom_d_acc = 0;
-    fom_d_gamma_acc = 0;
-    fom_d_t0 = 0;
     fom_d_param_term = 0;
     fom_d_p0 = 0;
     fom_d_residual = 0;
```

## Agent notes

```
Initial mean_us: 18960.498520
Final mean_us: 5516.986960
Overall speedup: 3.44x

Final profiling breakdown, hottest first:
- recurrent_linear: 159264.5590 us, 73.89%, 5000 calls
- activation_state_update: 54339.3730 us, 25.21%, 5000 calls
- residual: 1215.7690 us, 0.56%, 25 calls
- parameter_precompute: 717.3690 us, 0.33%, 25 calls

Optimizations applied:
- Collapsed two sparse recurrent products into one by exploiting the FISTA block structure: the operators encode B*x_k and B*(x_k-x_{k-1}), allowing direct construction of x_k + gamma*(x_k-x_{k-1}).
- Replaced generic sparse state-update products with a fused kernel by exploiting the complementary identity blocks implementing [x_{k-1}; x_k] -> [x_k; x_{k+1}].
- Fused soft-threshold activation with the state shift, eliminating an intermediate vector and one kernel launch per iteration.
- Fused the invariant parameter-term accumulator reset with extrapolation because both share the same 20,000-element row domain, eliminating a device-to-device copy per iteration.
- Kept all iterative state resident on the GPU and retained profiling markers around every main computational region.
```
