# Optimization report: ista / cuda (m10000_n40000_k260_sepA_istacudaregen)

Fresh-build head-to-head on della-l09g2, mean per solve over 5 solves after 8 warm-ups:

| solver | mean_us |
|---|---:|
| Phase A | 28288.728600 |
| Phase B | 21454.564600 |

Kept: **phase_b** (Phase-A / Phase-B time = 1.319). Final verification:

```
worst max-abs error over 8 samples: 2.665e-15 (tol 1.000e-05)
```

## Diff (Phase A -> kept)

```diff
--- phase_a_solver.cu	2026-10-16 18:03:14.416619000 -0400
+++ src/fom_solver.cu	2026-10-16 18:33:48.073310000 -0400
@@ -10,11 +10,20 @@
  *   t1 = soft_threshold(t1_state @ state + t1_t_t0 @ t0, FOM_T1_LAMBDA)
  *   state <- su_t_t1 @ t1
  *
- * `t0_p_p0 @ p0` is constant over the loop, so it is evaluated once per
- * solve into `d_b0` and used to re-seed the t0 accumulator.  Accumulator
- * re-zeroing for the next iteration is folded into the single elementwise
- * kernel that already touches those buffers, which leaves five kernel
- * launches per iteration.  No `/` operator is used anywhere.
+ * Structure exploited (spec.json / MATRIX_STATS.md):
+ *   - FOM_T0_LAMBDA == 0, so the t0 activation is the identity and no
+ *     kernel is launched for it.
+ *   - `t0_p_p0 @ p0` is loop-invariant: it is evaluated once per solve
+ *     into `d_b0` and then only re-seeds the t0 accumulator each pass.
+ *   - Re-zeroing the accumulators for the next pass is folded into the
+ *     single elementwise kernel that already touches those buffers,
+ *     leaving five kernels per iteration.
+ *   - All matrix products use the CSR kernels of the fixed fom_ops.cu;
+ *     they are re-launched with a grid shaped for this GPU (below).
+ *   - The whole solve (parameter upload, all iterations, residual and
+ *     read-back) is captured once as a CUDA graph and replayed, so a
+ *     solve costs one launch instead of ~1000.
+ * No `/` operator is used anywhere.
  */
 
 #include "fom_solver.h"
@@ -30,14 +39,54 @@
 #define FOM_T0_BLOCKS    40   /* ceil(10000 / 256) */
 #define FOM_RED_THREADS  1024
 
+/* ----------------------------------------------------------------
+ * Launch geometry for the fixed CSR kernels of src/fom_ops.cu.
+ *
+ * The `fom_apply_<m>` wrappers launch their kernel with a hard-coded
+ * 256-thread block (grid = ceil(rows/256)) on the legacy stream.  The
+ * kernels index rows as `blockIdx.x * blockDim.x + threadIdx.x` and
+ * exit when the row is out of range, so *any* geometry that covers all
+ * rows executes exactly the same device code with exactly the same
+ * per-row work - one thread still owns one whole row, so results are
+ * bit-identical.  The kernel entry points are ordinary external
+ * symbols, so the solver re-launches them with a better shaped grid,
+ * and on its own stream (the wrappers are hard-wired to stream 0,
+ * which cannot be stream-captured).
+ *
+ * Shape matters most for t0_state (10000 rows x 260 nnz): with
+ * 256-thread blocks there are only 40 blocks, so at most 40 of the
+ * A100's 108 SMs ever receive work and this latency-bound gather runs
+ * at a third of the achievable rate.  Narrow blocks spread the same
+ * 10000 row-threads over many more SMs and give each SM several
+ * independent warps to hide the gather latency with; 23 threads per
+ * block (435 blocks, ~4 warps/SM) measured fastest here, and every
+ * value from 16 to 32 beats the wrapper's 256 by >6%.
+ * ---------------------------------------------------------------- */
+#define FOM_T0S_BLK   23
+#define FOM_T0S_GRID  435  /* ceil(10000 / 23)  */
+#define FOM_T1T_BLK   384
+#define FOM_T1T_GRID  105  /* ceil(40000 / 384) */
+#define FOM_T1S_BLK   256
+#define FOM_T1S_GRID  157  /* ceil(40000 / 256) */
+#define FOM_SU_BLK    512
+#define FOM_SU_GRID   79   /* ceil(40000 / 512) */
+#define FOM_P0_BLK    256
+#define FOM_P0_GRID   40   /* ceil(10000 / 256) */
+
+/* Kernel entry points defined in the (fixed) src/fom_ops.cu. */
+__global__ void fom_apply_t0_state_kern(const fom_float *, fom_float *);
+__global__ void fom_apply_t1_state_kern(const fom_float *, fom_float *);
+__global__ void fom_apply_t1_t_t0_kern(const fom_float *, fom_float *);
+__global__ void fom_apply_su_t_t1_kern(const fom_float *, fom_float *);
+__global__ void fom_apply_t0_p_p0_kern(const fom_float *, fom_float *);
+
 /* ---------------------------------------------------------------- */
 /* Device buffers (allocated once in fom_setup, freed in fom_cleanup) */
 /* ---------------------------------------------------------------- */
 
 /* Pool layout: [x | y | acc0 | acc1 | t1 | b0 | p0 | res].  The leading
    x/y/acc0 span is exactly the set of vectors the two big CSR kernels
-   gather from with irregular column indices; that span is what gets
-   pinned in L2. */
+   gather from with irregular column indices. */
 #define FOM_HOT_LEN  (2 * FOM_STATE_DIM + FOM_T0_DIM)
 #define FOM_POOL_LEN (FOM_HOT_LEN + 2 * FOM_STATE_DIM + FOM_T0_DIM + FOM_PARAM_P0_DIM + 1)
 
@@ -51,8 +100,30 @@
 static fom_float *d_p0       = 0;  /* uploaded parameter vector          */
 static fom_float *d_init     = 0;  /* FOM_STATE_INIT, uploaded once      */
 static fom_float *d_res      = 0;  /* scalar residual                    */
-static fom_float *h_stage    = 0;  /* pinned host staging buffer         */
-static int        fom_ready  = 0;
+static fom_float *h_p0       = 0;  /* pinned staging: parameter upload   */
+static fom_float *h_x        = 0;  /* pinned staging: solution read-back */
+static fom_float *h_res      = 0;  /* pinned staging: residual           */
+
+static cudaStream_t     s_main       = 0;
+static cudaStream_t     s_side       = 0;
+static cudaEvent_t      e_fork       = 0;
+static cudaEvent_t      e_join       = 0;
+static cudaGraphExec_t  s_graph      = 0;
+static fom_int          s_graph_iter = -1;
+static int              fom_ready    = 0;
+
+/* In the profiling build the solve body is issued eagerly (a captured
+   graph cannot be instrumented from the host, and stream syncs are
+   illegal during capture), with a stream sync closing every region so
+   the per-region totals attribute real device time.  Those syncs
+   serialise the launch pipeline, so the profiling build's BENCH line
+   is slower than the production one; the per-region split is what it
+   is there for. */
+#ifdef FOM_PROFILE
+#define FOM_PSYNC() cudaStreamSynchronize(s_main)
+#else
+#define FOM_PSYNC() ((void)0)
+#endif
 
 /* ---------------------------------------------------------------- */
 /* Kernels                                                           */
@@ -113,6 +184,180 @@
 }
 
 /* ---------------------------------------------------------------- */
+/* One iteration: five kernels, all issued into s_main               */
+/* ---------------------------------------------------------------- */
+
+static void fom_iter(fom_float *xp, fom_float *yp, int last) {
+    /* t0 = act(t0_state @ state + t0_p_p0 @ p0); acc0 already holds b0. */
+    FOM_PROF_BEGIN(t0_spmv);
+    cudaEventRecord(e_fork, s_main);
+    cudaStreamWaitEvent(s_side, e_fork, 0);
+    fom_apply_t1_state_kern<<<FOM_T1S_GRID, FOM_T1S_BLK, 0, s_side>>>(xp, d_acc1);
+    fom_apply_t0_state_kern<<<FOM_T0S_GRID, FOM_T0S_BLK, 0, s_main>>>(xp, d_acc0);
+    FOM_PSYNC();
+    FOM_PROF_END(t0_spmv);
+    if (FOM_T0_LAMBDA != (fom_float)0.0)
+        fom_k_soft<<<FOM_T0_BLOCKS, FOM_BLOCK, 0, s_main>>>(d_acc0, FOM_T0_DIM,
+                                                            FOM_T0_LAMBDA);
+
+    /* rhs(t1) = t1_state @ state + t1_t_t0 @ t0; acc1 is already zero. */
+    FOM_PROF_BEGIN(t1_spmv);
+    cudaEventRecord(e_join, s_side);
+    cudaStreamWaitEvent(s_main, e_join, 0);
+    fom_apply_t1_t_t0_kern<<<FOM_T1T_GRID, FOM_T1T_BLK, 0, s_main>>>(d_acc0, d_acc1);
+    FOM_PSYNC();
+    FOM_PROF_END(t1_spmv);
+
+    /* t1 = soft_threshold(rhs, lam1), plus buffer housekeeping. */
+    FOM_PROF_BEGIN(act_step);
+    fom_k_step<<<FOM_STATE_BLOCKS, FOM_BLOCK, 0, s_main>>>(
+        d_acc1, d_t1, last ? (fom_float *)0 : xp, d_b0, d_acc0, FOM_T1_LAMBDA);
+    FOM_PSYNC();
+    FOM_PROF_END(act_step);
+
+    /* state <- su_t_t1 @ t1 (yp is zero on entry). */
+    FOM_PROF_BEGIN(state_up);
+    fom_apply_su_t_t1_kern<<<FOM_SU_GRID, FOM_SU_BLK, 0, s_main>>>(d_t1, yp);
+    FOM_PSYNC();
+    FOM_PROF_END(state_up);
+}
+
+/* Whole solve: issued once into a graph, or eagerly when profiling. */
+static void fom_issue_solve(fom_int max_iter) {
+    fom_float *xp, *yp, *sw;
+    fom_int k;
+
+    FOM_PROF_BEGIN(upload);
+    /* Parameter upload + the loop-invariant b0 = t0_p_p0 @ p0. */
+    cudaMemcpyAsync(d_p0, h_p0, FOM_PARAM_P0_DIM * sizeof(fom_float),
+                    cudaMemcpyHostToDevice, s_main);
+    cudaMemsetAsync(d_b0, 0, FOM_T0_DIM * sizeof(fom_float), s_main);
+    fom_apply_t0_p_p0_kern<<<FOM_P0_GRID, FOM_P0_BLK, 0, s_main>>>(d_p0, d_b0);
+
+    /* Loop entry invariant: x = state, y = 0, acc1 = 0, acc0 = b0. */
+    cudaMemcpyAsync(d_x, d_init, FOM_STATE_DIM * sizeof(fom_float),
+                    cudaMemcpyDeviceToDevice, s_main);
+    cudaMemsetAsync(d_y, 0, FOM_STATE_DIM * sizeof(fom_float), s_main);
+    cudaMemsetAsync(d_acc1, 0, FOM_STATE_DIM * sizeof(fom_float), s_main);
+    cudaMemcpyAsync(d_acc0, d_b0, FOM_T0_DIM * sizeof(fom_float),
+                    cudaMemcpyDeviceToDevice, s_main);
+    FOM_PSYNC();
+    FOM_PROF_END(upload);
+
+    xp = d_x;
+    yp = d_y;
+    for (k = 0; k < max_iter; ++k) {
+        int last = (k + 1 == max_iter);
+        fom_iter(xp, yp, last);
+        if (last)
+            fom_k_resid<<<1, FOM_RED_THREADS, 0, s_main>>>(yp, xp, d_res);
+        sw = xp; xp = yp; yp = sw;
+    }
+
+    FOM_PROF_BEGIN(download);
+    cudaMemcpyAsync(h_res, d_res, sizeof(fom_float),
+                    cudaMemcpyDeviceToHost, s_main);
+    cudaMemcpyAsync(h_x, xp, FOM_STATE_DIM * sizeof(fom_float),
+                    cudaMemcpyDeviceToHost, s_main);
+    FOM_PSYNC();
+    FOM_PROF_END(download);
+}
+
+/* ---------------------------------------------------------------- */
+/* Setup-time L2 residency tuning                                    */
+/* ---------------------------------------------------------------- */
+
+/* Each iteration streams ~62 MB of CSR data (two 2.6M-nnz matrices)
+ * through a 40 MB L2, so every pass refetches all of it from HBM.
+ * Reserving part of L2 as a persisting window keeps a slice of that
+ * working set - plus the randomly gathered state vectors, which the
+ * streaming traffic would otherwise evict every pass - resident across
+ * iterations.  The best slice depends on where the driver places the
+ * (lazily loaded) matrix globals relative to our pool, so the window
+ * is chosen by measuring the real iteration on the real buffers at
+ * setup time rather than by guessing.  A window only changes cache
+ * replacement policy, never results, so every candidate is equally
+ * correct and the worst case is simply the untuned default.
+ */
+#define FOM_TUNE_WARM   2400  /* warm-up iterations (clocks + caches)   */
+#define FOM_TUNE_ITERS  24    /* iterations timed per trial             */
+#define FOM_TUNE_PASSES 3     /* round-robin passes over the candidates */
+#define FOM_TUNE_NCAND  7
+
+static void fom_set_l2_window(long long off, size_t bytes) {
+    cudaStreamAttrValue av;
+    cudaDeviceSetLimit(cudaLimitPersistingL2CacheSize, bytes);
+    memset(&av, 0, sizeof(av));
+    av.accessPolicyWindow.base_ptr = (void *)((char *)d_pool + off);
+    av.accessPolicyWindow.num_bytes = bytes;
+    av.accessPolicyWindow.hitRatio = 1.0f;
+    av.accessPolicyWindow.hitProp = cudaAccessPropertyPersisting;
+    av.accessPolicyWindow.missProp = cudaAccessPropertyStreaming;
+    cudaStreamSetAttribute(s_main, cudaStreamAttributeAccessPolicyWindow, &av);
+    if (s_side)
+        cudaStreamSetAttribute(s_side, cudaStreamAttributeAccessPolicyWindow, &av);
+}
+
+static float fom_trial(int iters) {
+    cudaEvent_t a, b;
+    float ms = 1.0e30f;
+    int k;
+    cudaEventCreate(&a);
+    cudaEventCreate(&b);
+    cudaEventRecord(a, s_main);
+    for (k = 0; k < iters; ++k) fom_iter(d_x, d_y, 1);
+    cudaEventRecord(b, s_main);
+    if (cudaEventSynchronize(b) == cudaSuccess)
+        cudaEventElapsedTime(&ms, a, b);
+    cudaEventDestroy(a);
+    cudaEventDestroy(b);
+    return ms;
+}
+
+static void fom_tune_l2(void) {
+    /* (offset from d_pool, window bytes); entry 0 - the hot vectors
+       only - is the conservative default.  The last entry covers the
+       case of a driver that places the matrix globals *below* the
+       pool instead of above it. */
+    static const long long cand_off[FOM_TUNE_NCAND] =
+        { 0, 0, 0, 0, 0, 0, -18000000 };
+    static const long long cand_len[FOM_TUNE_NCAND] =
+        { FOM_HOT_LEN * (long long)sizeof(fom_float),
+          8000000, 12000000, 16000000, 18000000, 21000000, 18000000 };
+    float ms[FOM_TUNE_NCAND];
+    int maxp = 0, i, pass, best = 0;
+
+    cudaDeviceGetAttribute(&maxp, cudaDevAttrMaxPersistingL2CacheSize, 0);
+    if (maxp <= 0) return;
+    for (i = 0; i < FOM_TUNE_NCAND; ++i) ms[i] = 1.0e30f;
+
+    /* Trials run on the live buffers; zero them so the arithmetic (and
+       hence the power draw, which gates the clock) matches a real
+       sparse ISTA iterate and the loop cannot produce infinities. */
+    cudaMemset(d_pool, 0, FOM_POOL_LEN * sizeof(fom_float));
+    fom_set_l2_window(0, (size_t)cand_len[0]);
+    for (i = 0; i < FOM_TUNE_WARM; i += FOM_TUNE_ITERS) fom_trial(FOM_TUNE_ITERS);
+
+    /* Round-robin so that any residual clock drift hits every
+       candidate equally; keep the best time seen per candidate. */
+    for (pass = 0; pass < FOM_TUNE_PASSES; ++pass) {
+        for (i = 0; i < FOM_TUNE_NCAND; ++i) {
+            float t;
+            if (cand_len[i] > (long long)maxp) continue;
+            fom_set_l2_window(cand_off[i], (size_t)cand_len[i]);
+            fom_trial(FOM_TUNE_ITERS);           /* let L2 settle */
+            t = fom_trial(FOM_TUNE_ITERS);
+            if (t < ms[i]) ms[i] = t;
+        }
+    }
+    /* Require a clear win over the conservative default. */
+    for (i = 1; i < FOM_TUNE_NCAND; ++i)
+        if (ms[i] < ms[best] * (best == 0 ? 0.995f : 1.0f)) best = i;
+    fom_set_l2_window(cand_off[best], (size_t)cand_len[best]);
+    cudaMemset(d_pool, 0, FOM_POOL_LEN * sizeof(fom_float));
+}
+
+/* ---------------------------------------------------------------- */
 /* ABI                                                               */
 /* ---------------------------------------------------------------- */
 
@@ -139,44 +384,66 @@
     work->solution.residual = (fom_float)0.0;
     work->solution.status = FOM_MAX_ITER_REACHED;
 
-    if (fom_ready) return;
-
-    /* One pooled allocation so the whole hot working set is a single
-       contiguous region that can be pinned in L2 (below).  The two big
-       CSR kernels stream tens of MB of matrix data per iteration, which
-       would otherwise evict the gathered vectors from L2 every pass. */
-    cudaMalloc((void **)&d_pool, FOM_POOL_LEN * sizeof(fom_float));
-    d_x    = d_pool;
-    d_y    = d_pool + FOM_STATE_DIM;
-    d_acc0 = d_pool + 2 * FOM_STATE_DIM;
-    d_acc1 = d_pool + FOM_HOT_LEN;
-    d_t1   = d_acc1 + FOM_STATE_DIM;
-    d_b0   = d_t1 + FOM_STATE_DIM;
-    d_p0   = d_b0 + FOM_T0_DIM;
-    d_res  = d_p0 + FOM_PARAM_P0_DIM;
-    cudaMalloc((void **)&d_init, FOM_STATE_DIM * sizeof(fom_float));
-    cudaHostAlloc((void **)&h_stage, FOM_STATE_DIM * sizeof(fom_float),
-                  cudaHostAllocDefault);
-
-    /* Keep the pool resident in L2 for every kernel on the default
-       stream (the fom_apply_* helpers launch there).  Best-effort: if
-       the device or driver refuses, the solver is unaffected. */
-    {
-        cudaStreamAttrValue av;
-        size_t bytes = FOM_HOT_LEN * sizeof(fom_float);
-        cudaDeviceSetLimit(cudaLimitPersistingL2CacheSize, bytes);
-        memset(&av, 0, sizeof(av));
-        av.accessPolicyWindow.base_ptr = (void *)d_pool;
-        av.accessPolicyWindow.num_bytes = bytes;
-        av.accessPolicyWindow.hitRatio = 1.0f;
-        av.accessPolicyWindow.hitProp = cudaAccessPropertyPersisting;
-        av.accessPolicyWindow.missProp = cudaAccessPropertyStreaming;
-        cudaStreamSetAttribute(0, cudaStreamAttributeAccessPolicyWindow, &av);
+    if (!fom_ready) {
+        /* One pooled allocation: the whole hot working set is then a
+           single contiguous region that can be pinned in L2. */
+        cudaMalloc((void **)&d_pool, FOM_POOL_LEN * sizeof(fom_float));
+        d_x    = d_pool;
+        d_y    = d_pool + FOM_STATE_DIM;
+        d_acc0 = d_pool + 2 * FOM_STATE_DIM;
+        d_acc1 = d_pool + FOM_HOT_LEN;
+        d_t1   = d_acc1 + FOM_STATE_DIM;
+        d_b0   = d_t1 + FOM_STATE_DIM;
+        d_p0   = d_b0 + FOM_T0_DIM;
+        d_res  = d_p0 + FOM_PARAM_P0_DIM;
+        cudaMalloc((void **)&d_init, FOM_STATE_DIM * sizeof(fom_float));
+        cudaMemset(d_pool, 0, FOM_POOL_LEN * sizeof(fom_float));
+        cudaHostAlloc((void **)&h_p0, FOM_PARAM_P0_DIM * sizeof(fom_float),
+                      cudaHostAllocDefault);
+        cudaHostAlloc((void **)&h_x, FOM_STATE_DIM * sizeof(fom_float),
+                      cudaHostAllocDefault);
+        cudaHostAlloc((void **)&h_res, sizeof(fom_float), cudaHostAllocDefault);
+
+        /* A private non-blocking stream: the whole solve is issued into
+           it (the fixed `fom_apply_*` wrappers are hard-wired to the
+           legacy stream, which cannot be stream-captured). */
+        cudaStreamCreateWithFlags(&s_main, cudaStreamNonBlocking);
+        cudaStreamCreateWithFlags(&s_side, cudaStreamNonBlocking);
+        cudaEventCreateWithFlags(&e_fork, cudaEventDisableTiming);
+        cudaEventCreateWithFlags(&e_join, cudaEventDisableTiming);
+
+        cudaMemcpy(d_init, FOM_STATE_INIT, FOM_STATE_DIM * sizeof(fom_float),
+                   cudaMemcpyHostToDevice);
+
+        /* Pick the L2 persisting window by measurement (best effort:
+           if the device refuses the attribute nothing changes). */
+        fom_tune_l2();
+        fom_ready = 1;
     }
 
-    cudaMemcpy(d_init, FOM_STATE_INIT, FOM_STATE_DIM * sizeof(fom_float),
-               cudaMemcpyHostToDevice);
-    fom_ready = 1;
+#ifndef FOM_PROFILE
+    /* Capture the entire solve once; only the *contents* of the
+       staging buffers change between solves, never the pointers, so a
+       single instantiation serves every solve with this iteration
+       count.  Any other count falls back to the eager path. */
+    if (work->settings.max_iter > 0 &&
+        work->settings.max_iter != s_graph_iter) {
+        cudaGraph_t g;
+        if (s_graph) { cudaGraphExecDestroy(s_graph); s_graph = 0; }
+        s_graph_iter = -1;
+        if (cudaStreamBeginCapture(s_main, cudaStreamCaptureModeThreadLocal)
+                == cudaSuccess) {
+            fom_issue_solve(work->settings.max_iter);
+            if (cudaStreamEndCapture(s_main, &g) == cudaSuccess) {
+                if (cudaGraphInstantiate(&s_graph, g, 0, 0, 0) == cudaSuccess) {
+                    s_graph_iter = work->settings.max_iter;
+                    cudaGraphUpload(s_graph, s_main);
+                }
+                cudaGraphDestroy(g);
+            }
+        }
+    }
+#endif
 }
 
 extern "C" void fom_update_params(FomWorkspace *work, const FomParams *params) {
@@ -190,87 +457,32 @@
 }
 
 extern "C" fom_int fom_solve(FomWorkspace *work) {
-    const fom_float lam0 = FOM_T0_LAMBDA;
-    const fom_float lam1 = FOM_T1_LAMBDA;
-    fom_int max_iter, k;
+    fom_int max_iter;
     fom_float residual = (fom_float)0.0;
-    fom_float *xp, *yp, *sw;
 
     if (!work) return FOM_MAX_ITER_REACHED;
     if (!fom_ready) fom_setup(work, &work->settings);
 
     max_iter = work->settings.max_iter;
 
-    FOM_PROF_BEGIN(upload);
-    /* Upload the parameter vector through pinned staging. */
-    memcpy(h_stage, work->params.p0, FOM_PARAM_P0_DIM * sizeof(fom_float));
-    cudaMemcpyAsync(d_p0, h_stage, FOM_PARAM_P0_DIM * sizeof(fom_float),
-                    cudaMemcpyHostToDevice, 0);
-
-    /* b0 = t0_p_p0 @ p0 is loop-invariant: evaluate it once. */
-    cudaMemsetAsync(d_b0, 0, FOM_T0_DIM * sizeof(fom_float), 0);
-    fom_apply_t0_p_p0(d_p0, d_b0);
-
-    /* Loop entry invariant: x = state, y = 0, acc1 = 0, acc0 = b0. */
-    cudaMemcpyAsync(d_x, d_init, FOM_STATE_DIM * sizeof(fom_float),
-                    cudaMemcpyDeviceToDevice, 0);
-    cudaMemsetAsync(d_y, 0, FOM_STATE_DIM * sizeof(fom_float), 0);
-    cudaMemsetAsync(d_acc1, 0, FOM_STATE_DIM * sizeof(fom_float), 0);
-    cudaMemcpyAsync(d_acc0, d_b0, FOM_T0_DIM * sizeof(fom_float),
-                    cudaMemcpyDeviceToDevice, 0);
-#ifdef FOM_PROFILE
-    cudaDeviceSynchronize();
-#endif
-    FOM_PROF_END(upload);
-
-    xp = d_x;
-    yp = d_y;
-
-    FOM_PROF_BEGIN(loop);
-    for (k = 0; k < max_iter; ++k) {
-        int last = (k + 1 == max_iter);
-
-        /* t0 = act(t0_state @ state + t0_p_p0 @ p0); acc0 already holds b0. */
-        fom_apply_t0_state(xp, d_acc0);
-        if (lam0 != (fom_float)0.0)
-            fom_k_soft<<<FOM_T0_BLOCKS, FOM_BLOCK>>>(d_acc0, FOM_T0_DIM, lam0);
-
-        /* rhs(t1) = t1_state @ state + t1_t_t0 @ t0; acc1 is already zero. */
-        fom_apply_t1_state(xp, d_acc1);
-        fom_apply_t1_t_t0(d_acc0, d_acc1);
-
-        /* t1 = soft_threshold(rhs, lam1), plus buffer housekeeping. */
-        fom_k_step<<<FOM_STATE_BLOCKS, FOM_BLOCK>>>(d_acc1, d_t1,
-                                                    last ? (fom_float *)0 : xp,
-                                                    d_b0, d_acc0, lam1);
-
-        /* state <- su_t_t1 @ t1 (yp is zero on entry). */
-        fom_apply_su_t_t1(d_t1, yp);
-
-        if (last)
-            fom_k_resid<<<1, FOM_RED_THREADS>>>(yp, xp, d_res);
-
-        sw = xp; xp = yp; yp = sw;
-    }
-#ifdef FOM_PROFILE
-    cudaDeviceSynchronize();
-#endif
-    FOM_PROF_END(loop);
-
-    FOM_PROF_BEGIN(download);
     if (max_iter > 0) {
-        cudaMemcpyAsync(&residual, d_res, sizeof(fom_float),
-                        cudaMemcpyDeviceToHost, 0);
-        cudaMemcpyAsync(h_stage, xp, FOM_STATE_DIM * sizeof(fom_float),
-                        cudaMemcpyDeviceToHost, 0);
-        cudaStreamSynchronize(0);
-        memcpy(work->state, h_stage, sizeof(work->state));
-        memcpy(work->solution.x, h_stage, sizeof(work->solution.x));
+        memcpy(h_p0, work->params.p0, FOM_PARAM_P0_DIM * sizeof(fom_float));
+
+        FOM_PROF_BEGIN(solve);
+        if (max_iter == s_graph_iter)
+            cudaGraphLaunch(s_graph, s_main);
+        else
+            fom_issue_solve(max_iter);
+        cudaStreamSynchronize(s_main);
+        FOM_PROF_END(solve);
+
+        residual = *h_res;
+        memcpy(work->state, h_x, sizeof(work->state));
+        memcpy(work->solution.x, h_x, sizeof(work->solution.x));
     } else {
         memcpy(work->state, FOM_STATE_INIT, sizeof(work->state));
         memcpy(work->solution.x, FOM_STATE_INIT, sizeof(work->solution.x));
     }
-    FOM_PROF_END(download);
 
     work->solution.iter = max_iter;
     work->solution.residual = residual;
@@ -285,9 +497,14 @@
 extern "C" void fom_cleanup(FomWorkspace *work) {
     (void)work;
     if (!fom_ready) return;
+    if (s_graph) { cudaGraphExecDestroy(s_graph); s_graph = 0; }
+    s_graph_iter = -1;
+    cudaStreamDestroy(s_main); s_main = 0;
     cudaFree(d_pool); d_pool = 0;
     cudaFree(d_init); d_init = 0;
     d_x = d_y = d_acc0 = d_acc1 = d_t1 = d_b0 = d_p0 = d_res = 0;
-    cudaFreeHost(h_stage); h_stage = 0;
+    cudaFreeHost(h_p0); h_p0 = 0;
+    cudaFreeHost(h_x); h_x = 0;
+    cudaFreeHost(h_res); h_res = 0;
     fom_ready = 0;
 }
```

## Agent notes (end of the Phase-B transcript)

```

===== agent attempt 1 =====
I'll start by exploring the project structure and understanding the current solver.
Let me add fine-grained profiling regions to see the per-kernel breakdown.
The fixed kernels' host stubs are globally visible symbols. Let me inspect their launch configuration and indexing.
The kernel indexes rows as `ctaid.x*ntid.x+tid.x` with a bounds check, and its host stub is a global symbol — so I can relaunch it with my own grid/block config. Let me sweep configurations in a standalone harness.
Launch-config tuning looks promising. Let me build a full-loop harness with runtime-tunable configs to find the best combination.
Timing is power/clock sensitive to data values; the real ISTA iterate is sparse, so the zero-data mode matches the solver (28.5ms). Sweeping with that:
Now let me apply the launch-geometry tuning to the real solver.
Let me test CUDA graphs and cache-config tweaks in the harness before touching the solver.
CUDA graph gives another ~5%. Let me restructure the solver around a captured graph.
Marginal costs/iter: t0_state 88.6µs, t1_t_t0 40.5µs, t1_state 5.3µs, su 3.3µs, step 2.9µs. Let me test a wider-window overlap variant plus L2/geometry tuning.
Window size beyond the pool helps a lot — suggesting the matrix data lives in adjacent VA. Let me separate the carve-out limit from window coverage and map it out.
The gain comes from the window covering lazily-loaded matrix data placed after the pool. Let me map the landscape.
Let me test the wider L2 window directly in the real solver.
Offset 0 / hitRatio 1.0 / ~18MB is optimal. Now re-tuning block sizes under the new L2 config:
Now let me write the clean final version with a setup-time L2 window auto-tuner.
The tuner can be biased by clock ramp-up. Let me make it round-robin and better warmed.
Let me validate the non-graph fallback path produces identical results.

[fomtools: the Claude agent stopped early (ResultError: Claude Code returned an error result: Reached maximum number of turns (60) (exit code: 1)); any output above is partial. Raise CodegenConfig.agent_optimize_max_turns to give it more turns.]
./verify_bin
worst max-abs error over 8 samples: 2.665e-15 (tol 1.000e-05)
VERIFY OK
[fom-step in holder 15708098 on della-l09g2]
```
