# Optimization report: ista / cuda (m325000_n1300000_k8)

Fresh-build head-to-head on della-i12g2, mean per solve over 5 solves after 8 warm-ups:

| solver | mean_us |
|---|---:|
| Phase A | 159651.837800 |
| Phase B | 142241.541800 |

Kept: **phase_b** (Phase-A / Phase-B time = 1.122). Final verification:

```
worst max-abs error over 8 samples: 8.704e-14 (tol 1.000e-06)
```

## Diff (Phase A -> kept)

```diff
--- phase_a_solver.cu	2026-10-14 17:08:29.217594000 -0400
+++ src/fom_solver.cu	2026-10-14 17:16:34.067221000 -0400
@@ -36,23 +36,50 @@
 static fom_float *d_state_init = NULL; /* copy of FOM_STATE_INIT */
 static fom_float *d_state = NULL;
 static fom_float *d_new_state = NULL;
+static fom_float *d_state_base = NULL; /* allocation holding state + new_state */
 static fom_float *d_acc = NULL;        /* linear_op of t0 */
 static fom_float *d_pterm = NULL;      /* t0_p_p0 @ p0 (loop invariant) */
 static fom_float *d_t0 = NULL;
 static fom_float *d_p0 = NULL;
 static unsigned long long *d_resid = NULL;
+/* Workspace whose p0 / solution.x buffers are page-locked (fast DMA). */
+static FomWorkspace *pinned_work = NULL;
+
+/* Persisting L2 window (bytes; 0 = unavailable) kept on the SpMV input. */
+static size_t l2_win_bytes = 0;
+
+static void set_l2_window(const fom_float *base) {
+    if (l2_win_bytes == 0) return;
+    cudaStreamAttrValue attr;
+    memset(&attr, 0, sizeof(attr));
+    attr.accessPolicyWindow.base_ptr = (void *)base;
+    attr.accessPolicyWindow.num_bytes = l2_win_bytes;
+    attr.accessPolicyWindow.hitRatio = 1.0f;
+    attr.accessPolicyWindow.hitProp = cudaAccessPropertyPersisting;
+    attr.accessPolicyWindow.missProp = cudaAccessPropertyStreaming;
+    if (cudaStreamSetAttribute(0, cudaStreamAttributeAccessPolicyWindow, &attr) != cudaSuccess) {
+        l2_win_bytes = 0;
+        cudaGetLastError();
+    }
+}

 static int grid_for(int n) { return (n + FOM_BLOCK - 1) / FOM_BLOCK; }

-/* tmp = soft_threshold(acc, lambda) */
-__global__ void soft_threshold_kernel(const fom_float *__restrict__ in,
+/* tmp = soft_threshold(acc, lambda); also re-arm acc with the
+   loop-invariant parameter term for the next iteration and zero the
+   state-update accumulator (fuses two memset/copy passes). */
+__global__ void soft_threshold_kernel(fom_float *__restrict__ acc,
+                                      const fom_float *__restrict__ pterm,
                                       fom_float *__restrict__ out,
+                                      fom_float *__restrict__ zero_out,
                                       fom_float lambd, int n) {
     int i = blockIdx.x * blockDim.x + threadIdx.x;
     if (i < n) {
-        fom_float v = in[i];
+        fom_float v = acc[i];
         fom_float a = fabs(v) - lambd;
         out[i] = (a > 0.0) ? copysign(a, v) : 0.0;
+        acc[i] = pterm[i];
+        zero_out[i] = 0.0;
     }
 }

@@ -93,14 +120,46 @@
     if (d_state == NULL) {
         size_t sb = sizeof(fom_float) * FOM_STATE_DIM;
         FOM_CUDA_CHECK(cudaMalloc((void **)&d_state_init, sb));
-        FOM_CUDA_CHECK(cudaMalloc((void **)&d_state, sb));
-        FOM_CUDA_CHECK(cudaMalloc((void **)&d_new_state, sb));
+        /* state and new_state share one allocation. */
+        FOM_CUDA_CHECK(cudaMalloc((void **)&d_state, 2 * sb));
+        d_new_state = d_state + FOM_STATE_DIM;
+        d_state_base = d_state;
+        {
+            int dev = 0, max_persist = 0, max_win = 0;
+            cudaGetDevice(&dev);
+            cudaDeviceGetAttribute(&max_persist, cudaDevAttrMaxPersistingL2CacheSize, dev);
+            cudaDeviceGetAttribute(&max_win, cudaDevAttrMaxAccessPolicyWindowSize, dev);
+            l2_win_bytes = sb;
+            if (l2_win_bytes > (size_t)max_win) l2_win_bytes = (size_t)max_win;
+            if (l2_win_bytes > (size_t)max_persist) l2_win_bytes = (size_t)max_persist;
+            if (l2_win_bytes == 0 ||
+                cudaDeviceSetLimit(cudaLimitPersistingL2CacheSize, l2_win_bytes) != cudaSuccess)
+                l2_win_bytes = 0;
+            cudaGetLastError();
+        }
         FOM_CUDA_CHECK(cudaMalloc((void **)&d_acc, sizeof(fom_float) * FOM_T0_DIM));
         FOM_CUDA_CHECK(cudaMalloc((void **)&d_pterm, sizeof(fom_float) * FOM_T0_DIM));
         FOM_CUDA_CHECK(cudaMalloc((void **)&d_t0, sizeof(fom_float) * FOM_T0_DIM));
         FOM_CUDA_CHECK(cudaMalloc((void **)&d_p0, sizeof(fom_float) * FOM_PARAM_P0_DIM));
         FOM_CUDA_CHECK(cudaMalloc((void **)&d_resid, sizeof(unsigned long long)));
     }
+    /* Page-lock the host buffers copied every solve (once per workspace). */
+    if (pinned_work != work) {
+        if (pinned_work != NULL) {
+            cudaHostUnregister(pinned_work->params.p0);
+            cudaHostUnregister(pinned_work->solution.x);
+        }
+        pinned_work = NULL;
+        if (cudaHostRegister(work->params.p0, sizeof(fom_float) * FOM_PARAM_P0_DIM,
+                             cudaHostRegisterDefault) == cudaSuccess) {
+            if (cudaHostRegister(work->solution.x, sizeof(fom_float) * FOM_STATE_DIM,
+                                 cudaHostRegisterDefault) == cudaSuccess)
+                pinned_work = work;
+            else
+                cudaHostUnregister(work->params.p0);
+        }
+        cudaGetLastError();
+    }
     FOM_CUDA_CHECK(cudaMemcpy(d_state_init, FOM_STATE_INIT,
                               sizeof(fom_float) * FOM_STATE_DIM,
                               cudaMemcpyHostToDevice));
@@ -124,22 +183,27 @@
     /* Parameter term is constant across iterations: compute it once. */
     FOM_CUDA_CHECK(cudaMemset(d_pterm, 0, sizeof(fom_float) * FOM_T0_DIM));
     fom_apply_t0_p_p0(d_p0, d_pterm);
+    FOM_CUDA_CHECK(cudaMemcpyAsync(d_acc, d_pterm,
+                                   sizeof(fom_float) * FOM_T0_DIM,
+                                   cudaMemcpyDeviceToDevice, 0));
     FOM_PROF_SYNC();
     FOM_PROF_END(setup);

     FOM_PROF_BEGIN(iterate);
+    set_l2_window(d_state);
     for (k = 0; k < max_iter; ++k) {
-        /* t0: linear_op = t0_state @ state + t0_p_p0 @ p0 */
-        FOM_CUDA_CHECK(cudaMemcpyAsync(d_acc, d_pterm,
-                                       sizeof(fom_float) * FOM_T0_DIM,
-                                       cudaMemcpyDeviceToDevice, 0));
+        /* t0: linear_op = t0_state @ state + t0_p_p0 @ p0
+           (acc already holds the parameter term) */
         fom_apply_t0_state(d_state, d_acc);
+        /* The old state is dead after the SpMV, so t0 overwrites it.
+           The last step keeps the old state for the residual. */
+        fom_float *t0 = (k == max_iter - 1) ? d_t0 : d_state;
+        set_l2_window(d_new_state); /* next SpMV input lands in L2 */
         soft_threshold_kernel<<<grid_for(FOM_T0_DIM), FOM_BLOCK>>>(
-            d_acc, d_t0, FOM_T0_LAMBDA, FOM_T0_DIM);
+            d_acc, d_pterm, t0, d_new_state, FOM_T0_LAMBDA, FOM_T0_DIM);

         /* state update: new_state = su_t_t0 @ t0 */
-        FOM_CUDA_CHECK(cudaMemsetAsync(d_new_state, 0, sizeof(fom_float) * n, 0));
-        fom_apply_su_t_t0(d_t0, d_new_state);
+        fom_apply_su_t_t0(t0, d_new_state);

         /* No tolerance: the residual only needs to be reported for the
            final iteration. */
@@ -177,14 +241,18 @@

 void fom_cleanup(FomWorkspace *work) {
     (void)work;
+    if (pinned_work != NULL) {
+        cudaHostUnregister(pinned_work->params.p0);
+        cudaHostUnregister(pinned_work->solution.x);
+        pinned_work = NULL;
+    }
     cudaFree(d_state_init);
-    cudaFree(d_state);
-    cudaFree(d_new_state);
+    cudaFree(d_state_base);
     cudaFree(d_acc);
     cudaFree(d_pterm);
     cudaFree(d_t0);
     cudaFree(d_p0);
     cudaFree(d_resid);
-    d_state_init = d_state = d_new_state = d_acc = d_pterm = d_t0 = d_p0 = NULL;
+    d_state_init = d_state = d_new_state = d_state_base = d_acc = d_pterm = d_t0 = d_p0 = NULL;
     d_resid = NULL;
 }
```

## Agent notes (end of the Phase-B transcript)

```
`fom_solve` now takes 142.4 ms instead of 159.4 ms, about 10.7% faster. The final `make verify` printed VERIFY OK with a worst error of 8.704e-14, the same as the original version.

**Where the time goes.** Almost all of it (about 640–680 µs per iteration) is the `t0_state` matrix-vector product. That kernel lives in `fom_ops.cu`, which I'm not allowed to change, so I could only speed it up indirectly. The biggest gain came from telling the A100 to keep the vector this product reads in its fast on-chip L2 cache.

**What I tried and threw away** (each was verified, benchmarked and reverted):
- Running the zeroing and copy work on a second stream alongside the matrix product: slower, 152.2 ms.
- Making the cache-pinned region also cover `acc`, so only part of it could stay pinned: much slower, 208 ms.
- Doubling the cache space reserved for pinned data: slower, 147.7 ms.
- Processing two values per thread in the soft-threshold kernel: no gain.
- `cudaFuncCachePreferL1`: change was within noise.

**One gain I left on the table.** `su_t_t0` has exactly one nonzero per row, so it is probably the identity. If so, skipping its multiply and the zeroing step would save about 45 µs per iteration, roughly 9 ms per solve. SPEC.md and MATRIX_STATS.md don't say it's the identity, so under your rules I didn't use it.

I also saved a note to memory about which of these approaches helped, for similar runs later.

**Initial mean_us:** 159387.6 (`make bench`, 5 solves)

**Final mean_us:** 142352.0 (`make bench`, 5 solves; the profile build measured 142279.8)

**Final profile regions** (`make profile`, 5 solves):
| region | total_us | calls | pct |
|---|---:|---:|---:|
| iterate | 707231.8 | 5 | 99.41 |
| copy_out | 2863.6 | 5 | 0.40 |
| setup | 1300.9 | 5 | 0.18 |

**Optimizations kept:**
- **Fused soft-threshold kernel** (159.4 → 158.6 ms): one kernel now does the soft-threshold, resets `acc` to the parameter term for the next iteration, and zeroes `new_state`. This replaced a separate per-iteration copy and memset. The parameter term is still computed once per solve.
- **L2 cache pinning of the state vector** (158.6 → 149.8 ms): a persisting-L2 access-policy window on the default stream keeps the vector read by the `t0_state` product in cache. The two state buffers share one allocation.
- **Page-locked host buffers** (149.8 → 149.5 ms): `work->params.p0` and `work->solution.x` are pinned once per workspace in `fom_setup` and released in `fom_cleanup`, which speeds up the per-solve transfers.
- **`t0` written into the old state buffer** (149.5 → 148.0 ms): the previous state is no longer needed after the matrix product, so `t0` reuses its memory. The last iteration still uses a separate `d_t0`, because the final residual needs the old state.
- **Cache window that follows the state** (148.0 → 144.4 → 143.2 → 142.3 ms): the pinned region is exactly one state vector at full hit ratio. Each iteration it is moved onto `new_state` just before the soft-threshold kernel, so the zeroing and the `su_t_t0` writes go into pinned cache that the next matrix product reads.
```
