# Optimization report: fista / cuda (m325000_n1300000_k8)

Fresh-build head-to-head on della-i12g1, mean per solve over 5 solves after 8 warm-ups:

| solver | mean_us |
|---|---:|
| Phase A | 721226.690000 |
| Phase B | 456135.150600 |

Kept: **phase_b** (Phase-A / Phase-B time = 1.581). Final verification:

```
worst max-abs error over 8 samples: 2.000e-07 (tol 1.000e-06)
```

## Diff (Phase A -> kept)

```diff
--- phase_a_solver.cu	2026-10-14 18:15:29.095750000 -0400
+++ src/fom_solver.cu	2026-10-14 18:25:52.520167000 -0400
@@ -27,6 +27,51 @@
 #define FOM_PROF_SYNC() ((void)0)
 #endif

+static size_t fom_l2_persist = 0; /* L2 set-aside bytes (0 = unsupported) */
+
+/* Keep the vector the SpMVs gather from resident in the L2 persisting
+   region while the (much larger) matrix data streams through. */
+static void fom_l2_window(const void *ptr, size_t bytes) {
+    if (!fom_l2_persist) return;
+    cudaStreamAttrValue attr;
+    memset(&attr, 0, sizeof(attr));
+    attr.accessPolicyWindow.base_ptr = (void *)ptr;
+    attr.accessPolicyWindow.num_bytes = bytes;
+    attr.accessPolicyWindow.hitRatio =
+        bytes <= fom_l2_persist ? 1.0f : (float)fom_l2_persist / (float)bytes;
+    attr.accessPolicyWindow.hitProp = cudaAccessPropertyPersisting;
+    attr.accessPolicyWindow.missProp = cudaAccessPropertyStreaming;
+    cudaStreamSetAttribute(0, cudaStreamAttributeAccessPolicyWindow, &attr);
+}
+
+/* Host output buffers page-locked once per workspace, so the final state
+   is DMA'd straight into work->state and work->solution.x. */
+static FomWorkspace *fom_pinned_work = NULL;
+
+static void fom_unpin_outputs(void) {
+    if (!fom_pinned_work) return;
+    cudaHostUnregister(fom_pinned_work->state);
+    cudaHostUnregister(fom_pinned_work->solution.x);
+    cudaGetLastError();
+    fom_pinned_work = NULL;
+}
+
+static void fom_pin_outputs(FomWorkspace *work) {
+    if (fom_pinned_work == work) return;
+    fom_unpin_outputs();
+    const size_t bytes = sizeof(fom_float) * FOM_STATE_DIM;
+    if (cudaHostRegister(work->state, bytes, cudaHostRegisterDefault) != cudaSuccess) {
+        cudaGetLastError();
+        return;
+    }
+    if (cudaHostRegister(work->solution.x, bytes, cudaHostRegisterDefault) != cudaSuccess) {
+        cudaHostUnregister(work->state);
+        cudaGetLastError();
+        return;
+    }
+    fom_pinned_work = work;
+}
+
 static inline int fom_blocks(int n) { return (n + FOM_THREADS - 1) / FOM_THREADS; }

 /* t0 = soft_threshold(a + gamma * g + p, lambda) */
@@ -87,6 +132,21 @@
         cudaMalloc((void **)&d_g, sizeof(fom_float) * FOM_T0_DIM);
         cudaMalloc((void **)&d_p, sizeof(fom_float) * FOM_T0_DIM);
         cudaMalloc((void **)&d_res, sizeof(unsigned long long));
+        int dev = 0, maxp = 0, maxw = 0;
+        cudaGetDevice(&dev);
+        cudaDeviceGetAttribute(&maxp, cudaDevAttrMaxPersistingL2CacheSize, dev);
+        cudaDeviceGetAttribute(&maxw, cudaDevAttrMaxAccessPolicyWindowSize, dev);
+        /* Set aside only what the state needs; the rest of L2 serves the
+           streamed matrix sectors. */
+        size_t want = sizeof(fom_float) * FOM_STATE_DIM;
+        if (want > (size_t)maxp) want = (size_t)maxp;
+        if (maxp > 0 && maxw > 0 &&
+            cudaDeviceSetLimit(cudaLimitPersistingL2CacheSize, want) == cudaSuccess)
+            fom_l2_persist = want;
+        /* Gathers and interleaved row streams waste wide fetches: fetch only
+           the 32-byte sectors requested. */
+        cudaDeviceSetLimit(cudaLimitMaxL2FetchGranularity, 32);
+        cudaGetLastError();
     }
     cudaMemcpy(d_state_init, FOM_STATE_INIT, sizeof(fom_float) * FOM_STATE_DIM,
                cudaMemcpyHostToDevice);
@@ -122,6 +182,7 @@

         /* t0 = soft_threshold(t0_state @ s + gamma * (d_gamma @ s) + t0_p_p0 @ p0) */
         FOM_PROF_BEGIN(t0);
+        fom_l2_window(d_state, sizeof(fom_float) * FOM_STATE_DIM);
         cudaMemset(d_t0, 0, sizeof(fom_float) * FOM_T0_DIM);
         cudaMemset(d_g, 0, sizeof(fom_float) * FOM_T0_DIM);
         fom_apply_t0_state(d_state, d_t0);
@@ -133,6 +194,9 @@

         /* new_state = su_state @ state + su_t_t0 @ t0 */
         FOM_PROF_BEGIN(state_update);
+        /* The new state is the next iteration's gather vector: write it
+           into the persisting region. */
+        fom_l2_window(d_new, sizeof(fom_float) * FOM_STATE_DIM);
         cudaMemset(d_new, 0, sizeof(fom_float) * FOM_STATE_DIM);
         fom_apply_su_state(d_state, d_new);
         fom_apply_su_t_t0(d_t0, d_new);
@@ -157,8 +221,13 @@
     }

     FOM_PROF_BEGIN(copy_out);
+    fom_pin_outputs(work);
     cudaMemcpy(work->state, d_state, sizeof(fom_float) * FOM_STATE_DIM, cudaMemcpyDeviceToHost);
-    memcpy(work->solution.x, work->state, sizeof(fom_float) * FOM_STATE_DIM);
+    if (fom_pinned_work == work)
+        cudaMemcpy(work->solution.x, d_state, sizeof(fom_float) * FOM_STATE_DIM,
+                   cudaMemcpyDeviceToHost);
+    else
+        memcpy(work->solution.x, work->state, sizeof(fom_float) * FOM_STATE_DIM);
     cudaDeviceSynchronize();
     FOM_PROF_END(copy_out);

@@ -170,6 +239,7 @@

 extern "C" void fom_cleanup(FomWorkspace *work) {
     (void)work;
+    fom_unpin_outputs();
     cudaFree(d_state_init);
     cudaFree(d_state);
     cudaFree(d_new);
```

## Agent notes (end of the Phase-B transcript)

```
I cut `fom_solve` from about 722 ms to 456 ms per solve, a 1.58× speedup. The final `make verify` prints VERIFY OK with a worst error of 2.0e-7 (tolerance 1e-6), the same as before.

All of the gain comes from GPU cache settings and host-side memory handling. The float64 arithmetic, the 200 iterations, and the order of every `fom_apply_*` call are unchanged, and the `FOM_PROF_BEGIN/END` markers are still in place. Over 94% of the time is still inside the five fixed `fom_apply_*` kernels, which I can't change. So what's left is making their memory traffic cheaper:

- **What helped most:** setting aside exactly enough L2 cache (persisting region) to hold the 20.8 MB state vector. That keeps the vector cached while about 70M matrix nonzeros stream through, without taking L2 away from the matrix data.
- **What was sensitive:** the set-aside size matters a lot. Half the state gave 704 ms, 0.9× gave 464 ms, and using the full 30 MB maximum gave 595 ms.

**Tried and reverted (no improvement):**
- Telling the GPU to prefer L1 cache: no change.
- 128-byte L2 fetches: 1002 ms, much worse.
- Removing the per-iteration memsets by re-zeroing the buffers inside the threshold kernel: 461.5 ms against 456.0.

**Not used:** the nonzero counts suggest `su_state` and `su_t_t0` are just copy blocks. Exploiting that would remove most of the state_update region (about 5% of the time). SPEC.md, spec.json and MATRIX_STATS.md don't document it, so I kept the general SpMV calls.

**Initial mean_us:** 721729.58

**Final mean_us:** 456172.18

**Final profile regions** (`make profile`, 3 solves; its timing includes extra syncs, so its total is 458236 µs):

| region | total_us | calls | pct |
|---|---:|---:|---:|
| t0 | 1294960.6 | 600 | 94.20 |
| state_update | 72619.9 | 600 | 5.28 |
| copy_out | 6543.2 | 3 | 0.48 |
| init | 408.9 | 3 | 0.03 |
| residual | 127.3 | 3 | 0.01 |

**Optimizations kept:**
- **L2 cache window on the state vector:** the current state is kept in L2 during the t0 SpMVs, via an access-policy window on the default stream (722 → 646 ms).
- **32-byte L2 fetch size:** the GPU fetches only the 32-byte pieces that are actually read, instead of wider lines (646 → 595 ms).
- **Smaller L2 set-aside:** the persisting region is sized to exactly the state vector, not the 30 MB maximum (595 → 460 ms).
- **Page-locked output buffers:** `work->state` and `work->solution.x` are page-locked once per workspace, and the final state is copied from the GPU straight into both, replacing the extra host `memcpy` (459.67 → 459.55 ms, marginal).
- **Window on the new state during state_update:** the vector the next iteration reads is written straight into the cached region (459.55 → 456.0 ms).
```
