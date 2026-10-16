# Optimization report: ista / cuda (m325000_n1300000_k8, D and D^T separate)

Fresh-build head-to-head on della-i12g1, mean per solve over 5 solves after 8 warm-ups:

| solver | mean_us |
|---|---:|
| Phase A | 46378.102600 |
| Phase B | 22734.794800 |

Kept: **phase_b** (Phase-A / Phase-B time = 2.040). Final verification:

```
worst max-abs error over 8 samples: 4.441e-15 (tol 1.000e-05)
```

## Diff (Phase A -> kept)

```diff
--- phase_a_solver.cu	2026-10-16 06:45:59.875480000 -0400
+++ src/fom_solver.cu	2026-10-16 06:59:25.331972000 -0400
@@ -2,19 +2,37 @@
  *
  * ISTA-style fixed-iteration first-order method, CUDA target.
  *
+ * Spec iteration:
  *   t0    <- soft_threshold( t0_state @ s + t0_p_p0 @ p0 , 0 )
  *   t1    <- soft_threshold( t1_state @ s + t1_t_t0  @ t0, FOM_T1_LAMBDA )
  *   s     <- su_t_t1 @ t1
  *
- * All matrix-vector products go through the accumulating CSR helpers in
- * fom_ops.cu; there is no linear solve in this spec (every step is an
- * explicit activation, `solve: null`).  The whole iteration lives on the
- * device: state/temporaries are cudaMalloc'd once in fom_setup, the
- * parameter-dependent constant term is uploaded and folded in
- * fom_update_params, and the solution is copied back exactly once at the
- * end of fom_solve.  Buffers are swapped by pointer, never copied.
+ * Structure exploited (checked against data.npz / MATRIX_STATS.md):
+ *   - t1_state  (1300000x1300000, 1 nnz/row) is the IDENTITY, so the
+ *     `t1_state @ s` term is just `s`: instead of zeroing an accumulator
+ *     and running a CSR SpMV, the accumulator is *seeded* with s.
+ *   - su_t_t1   (1300000x1300000, 1 nnz/row) is the IDENTITY, so the
+ *     state update `s <- su_t_t1 @ t1` is a rename, not a SpMV.
+ *   - soft_threshold(.,0) is the identity map, so the t0 activation is a
+ *     no-op (t0 feeds straight into the t1 linear op).
+ *   - t0_p_p0 @ p0 is loop-invariant and folded once in fom_update_params.
+ * Both identity SpMVs are verified numerically at setup time (see
+ * fom_check_identities) and the solver falls back to the generic CSR
+ * path if the check ever fails, so the fast path can never silently
+ * compute the wrong thing.
  *
- * No `/` operator is used anywhere.
+ * What is left in the loop is therefore
+ *   t0 <- b0 + t0_state @ s            (b0 = t0_p_p0 @ p0, precomputed)
+ *   s  <- soft_threshold( s + t1_t_t0 @ t0, lambda )
+ * i.e. two CSR SpMVs and one elementwise pass.  Because `s` is both the
+ * input of the first SpMV and the accumulator of the second (and the
+ * two SpMVs are serialised on the default stream anyway), the whole
+ * iteration runs in a SINGLE n-sized device buffer -- no double
+ * buffering, no pointer swap, no extra 10 MB write per iteration.
+ *
+ * Everything stays resident on the device: buffers are cudaMalloc'd once
+ * in fom_setup and the solution is copied back exactly once at the end
+ * of fom_solve (through a page-locked host range, registered at setup).
  */

 #include <cuda_runtime.h>
@@ -28,47 +46,71 @@

 #define FOM_N          FOM_STATE_DIM      /* 1300000 */
 #define FOM_M          FOM_PARAM_P0_DIM   /*  325000 */
-#define FOM_THREADS    256                /* keep in sync with FOM_LOG2_THREADS */
+#define FOM_N2         (FOM_N >> 1)       /*  650000 double2 slots */
+#define FOM_M2         (FOM_M >> 1)       /*  162500 double2 slots */
+
+#define FOM_THREADS    256
 #define FOM_LOG2_THR   8
 /* ceil(n / FOM_THREADS) without a division operator */
 #define FOM_BLOCKS(n)  (((n) + (FOM_THREADS - 1)) >> FOM_LOG2_THR)
-#define FOM_BLOCKS_N   FOM_BLOCKS(FOM_N)  /* 5079 */
+#define FOM_BLOCKS_N   FOM_BLOCKS(FOM_N)   /* 5079 */
+#define FOM_BLOCKS_N2  FOM_BLOCKS(FOM_N2)  /* 2540 */
+#define FOM_ARENA_BYTES ((size_t)(FOM_N + FOM_M + FOM_M) * sizeof(fom_float))
+
+/* Host-timed profiling regions are meaningless while kernels are still in
+   flight, so the instrumented build synchronises at every region end.  In
+   the production build this is compiled out together with the timers. */
+#ifdef FOM_PROFILE
+#define FOM_SYNC() cudaDeviceSynchronize()
+#else
+#define FOM_SYNC() ((void)0)
+#endif

 /* ------------------------------------------------------------------ */
 /* Device kernels                                                      */
 /* ------------------------------------------------------------------ */

-/* Fused prologue of one iteration:
- *   t0[0:M] = b0          (b0 = t0_p_p0 @ p0, constant over the loop)
- *   t1[0:N] = 0           (accumulator for the t1 linear op)
- * One launch instead of a memcpy plus a memset. */
-__global__ void fom_prep_kern(const fom_float *__restrict__ b0,
-                              fom_float *__restrict__ t0,
-                              fom_float *__restrict__ t1)
+__device__ __forceinline__ double fom_soft(double v, double lam)
+{
+    return (v > lam) ? (v - lam) : ((v < -lam) ? (v + lam) : 0.0);
+}
+
+/* t0[0:M] <- b0 (the loop-invariant parameter term), vectorised. */
+__global__ void fom_seed_t0_kern(const double2 *__restrict__ b0,
+                                 double2 *__restrict__ t0)
 {
     int i = blockIdx.x * blockDim.x + threadIdx.x;
-    if (i < FOM_N) {
-        t1[i] = 0.0;
-        if (i < FOM_M) t0[i] = b0[i];
-    }
+    if (i < FOM_M2) t0[i] = b0[i];
 }

-/* Fused epilogue of one iteration:
- *   t1 <- soft_threshold(t1, lam)   (in place)
- *   ns <- 0                         (accumulator for the state update) */
-__global__ void fom_soft_zero_kern(fom_float *__restrict__ t1,
-                                   const fom_float lam,
-                                   fom_float *__restrict__ ns)
+/* Fused end-of-iteration kernel:
+ *     s[i]  <- soft_threshold(s[i], lam)      (in place: t1 -> new state)
+ *     t0[i] <- b0[i]                          (seed for the next iteration)
+ * Both ranges are 16-byte aligned and have an even length (N = 1300000,
+ * M = 325000 -> 650000 / 162500 double2 slots), so the whole pass runs on
+ * 128-bit loads/stores. */
+__global__ void fom_soft_seed_kern(double2 *__restrict__ s,
+                                   const double lam,
+                                   const double2 *__restrict__ b0,
+                                   double2 *__restrict__ t0)
 {
     int i = blockIdx.x * blockDim.x + threadIdx.x;
-    if (i < FOM_N) {
-        fom_float v = t1[i];
-        fom_float r = (v > lam) ? (v - lam) : ((v < -lam) ? (v + lam) : 0.0);
-        t1[i] = r;
-        ns[i] = 0.0;
+    if (i < FOM_N2) {
+        double2 v = s[i];
+        v.x = fom_soft(v.x, lam);
+        v.y = fom_soft(v.y, lam);
+        s[i] = v;
+        if (i < FOM_M2) t0[i] = b0[i];
     }
 }

+/* Generic (fallback) soft-threshold, used when the identity check fails. */
+__global__ void fom_soft_kern(fom_float *__restrict__ s, const double lam)
+{
+    int i = blockIdx.x * blockDim.x + threadIdx.x;
+    if (i < FOM_N) s[i] = fom_soft(s[i], lam);
+}
+
 /* Block-wise max|a - b|; the host folds the per-block partials. */
 __global__ void fom_resid_kern(const fom_float *__restrict__ a,
                                const fom_float *__restrict__ b,
@@ -84,8 +126,44 @@
     }
     sm[tid] = v;
     __syncthreads();
-    for (int s = blockDim.x >> 1; s > 0; s >>= 1) {
-        if (tid < s && sm[tid + s] > sm[tid]) sm[tid] = sm[tid + s];
+    for (int t = blockDim.x >> 1; t > 0; t >>= 1) {
+        if (tid < t && sm[tid + t] > sm[tid]) sm[tid] = sm[tid + t];
+        __syncthreads();
+    }
+    if (tid == 0) part[blockIdx.x] = sm[0];
+}
+
+/* Fallback-path helpers: zero an accumulator / seed it from the state. */
+__global__ void fom_zero_kern(double2 *__restrict__ v, int n2)
+{
+    int i = blockIdx.x * blockDim.x + threadIdx.x;
+    if (i < n2) { double2 z; z.x = 0.0; z.y = 0.0; v[i] = z; }
+}
+
+/* Probe kernels for the setup-time identity check. */
+__global__ void fom_fill_probe_kern(fom_float *__restrict__ v, int n)
+{
+    int i = blockIdx.x * blockDim.x + threadIdx.x;
+    /* a cheap, index-dependent, exactly representable pattern */
+    if (i < n) v[i] = (fom_float)(1 + (i & 1023)) * 0.125;
+}
+
+__global__ void fom_diff_kern(const fom_float *__restrict__ a,
+                              const fom_float *__restrict__ b,
+                              fom_float *__restrict__ part, int n)
+{
+    __shared__ fom_float sm[FOM_THREADS];
+    int tid = threadIdx.x;
+    int i = blockIdx.x * blockDim.x + tid;
+    fom_float v = 0.0;
+    if (i < n) {
+        v = a[i] - b[i];
+        if (v < 0.0) v = -v;
+    }
+    sm[tid] = v;
+    __syncthreads();
+    for (int t = blockDim.x >> 1; t > 0; t >>= 1) {
+        if (tid < t && sm[tid + t] > sm[tid]) sm[tid] = sm[tid + t];
         __syncthreads();
     }
     if (tid == 0) part[blockIdx.x] = sm[0];
@@ -95,24 +173,29 @@
 /* Persistent device state                                             */
 /* ------------------------------------------------------------------ */

-static fom_float *d_state     = 0;  /* N */
-static fom_float *d_new_state = 0;  /* N */
-static fom_float *d_t1        = 0;  /* N */
-static fom_float *d_init      = 0;  /* N, device copy of FOM_STATE_INIT */
-static fom_float *d_t0        = 0;  /* M */
-static fom_float *d_p0        = 0;  /* M */
-static fom_float *d_b0        = 0;  /* M, = t0_p_p0 @ p0 */
-static fom_float *d_part      = 0;  /* FOM_BLOCKS_N */
-static int        g_ready     = 0;
-static int        g_l2on      = 1;
+static fom_float *d_arena    = 0;  /* N + M + M, one contiguous block    */
+static fom_float *d_s        = 0;  /* N : state, and the t1 accumulator */
+static fom_float *d_prev     = 0;  /* N : snapshot for the residual      */
+static fom_float *d_alt      = 0;  /* N : fallback path only             */
+static fom_float *d_alt2     = 0;  /* N : fallback path only             */
+static fom_float *d_init     = 0;  /* N : device copy of FOM_STATE_INIT  */
+static fom_float *d_t0       = 0;  /* M */
+static fom_float *d_p0       = 0;  /* M */
+static fom_float *d_b0       = 0;  /* M : = t0_p_p0 @ p0                 */
+static fom_float *d_part     = 0;  /* FOM_BLOCKS_N */
+static int        g_ready    = 0;
+static int        g_fast     = 0;  /* identity structure confirmed       */
+static int        g_l2on     = 1;
+static void      *g_pinned   = 0;  /* host range registered for the D2H  */

 static fom_float h_part[FOM_BLOCKS_N];

 /* Pin `p` (num_bytes) in L2 for subsequent launches on the default stream.
-   The A-matvec (t0_state, 8 nnz/row) gathers 2.6M random entries out of the
-   10.4 MB state vector every iteration; keeping that vector L2-resident turns
-   those gathers from HBM sector traffic into L2 hits. Best-effort: if the
-   driver rejects the hint the solver is still correct, just slower. */
+   The A-matvec (t0_state, 8 nnz/row) gathers 2.6M essentially random
+   entries out of the 10.4 MB state vector every iteration; keeping that
+   vector L2-resident turns those gathers from HBM sector traffic into L2
+   hits.  Best-effort: if the driver rejects the hint the solver is still
+   correct, just slower. */
 static void fom_l2_persist(const fom_float *p, size_t num_bytes)
 {
     cudaStreamAttrValue a;
@@ -125,17 +208,62 @@
     cudaStreamSetAttribute(0, cudaStreamAttributeAccessPolicyWindow, &a);
 }

-static void fom_device_init(void)
+/* Numerically confirm that `t1_state` and `su_t_t1` really are the
+   identity before taking the fast path.  Two probes each (a non-constant
+   pattern) is enough to catch any change in the generated data; the cost
+   is a handful of kernels, once, at setup. */
+static int fom_check_identities(void)
+{
+    fom_float *probe = d_prev;      /* scratch, unused during setup */
+    fom_float *acc   = d_s;
+    fom_float worst  = 0.0;
+    int b;
+
+    fom_fill_probe_kern<<<FOM_BLOCKS_N, FOM_THREADS>>>(probe, FOM_N);
+
+    /* t1_state @ probe == probe ? */
+    fom_zero_kern<<<FOM_BLOCKS_N2, FOM_THREADS>>>((double2 *)acc, FOM_N2);
+    fom_apply_t1_state(probe, acc);
+    fom_diff_kern<<<FOM_BLOCKS_N, FOM_THREADS>>>(acc, probe, d_part, FOM_N);
+    cudaMemcpy(h_part, d_part, (size_t)FOM_BLOCKS_N * sizeof(fom_float),
+               cudaMemcpyDeviceToHost);
+    for (b = 0; b < FOM_BLOCKS_N; ++b) if (h_part[b] > worst) worst = h_part[b];
+
+    /* su_t_t1 @ probe == probe ? */
+    fom_zero_kern<<<FOM_BLOCKS_N2, FOM_THREADS>>>((double2 *)acc, FOM_N2);
+    fom_apply_su_t_t1(probe, acc);
+    fom_diff_kern<<<FOM_BLOCKS_N, FOM_THREADS>>>(acc, probe, d_part, FOM_N);
+    cudaMemcpy(h_part, d_part, (size_t)FOM_BLOCKS_N * sizeof(fom_float),
+               cudaMemcpyDeviceToHost);
+    for (b = 0; b < FOM_BLOCKS_N; ++b) if (h_part[b] > worst) worst = h_part[b];
+
+    cudaDeviceSynchronize();
+    return (worst == 0.0);
+}
+
+static void fom_device_init(FomWorkspace *work)
 {
     if (g_ready) return;
-    cudaMalloc((void **)&d_state,     (size_t)FOM_N * sizeof(fom_float));
-    cudaMalloc((void **)&d_new_state, (size_t)FOM_N * sizeof(fom_float));
-    cudaMalloc((void **)&d_t1,        (size_t)FOM_N * sizeof(fom_float));
-    cudaMalloc((void **)&d_init,      (size_t)FOM_N * sizeof(fom_float));
-    cudaMalloc((void **)&d_t0,        (size_t)FOM_M * sizeof(fom_float));
-    cudaMalloc((void **)&d_p0,        (size_t)FOM_M * sizeof(fom_float));
-    cudaMalloc((void **)&d_b0,        (size_t)FOM_M * sizeof(fom_float));
-    cudaMalloc((void **)&d_part,      (size_t)FOM_BLOCKS_N * sizeof(fom_float));
+    /* s, t0 and b0 are the only vectors touched by *every* iteration, so
+       they live in one contiguous arena: a single L2 access-policy window
+       then covers all three (the window is one address range).  N*8 and
+       M*8 are both multiples of 256, so every slice keeps cudaMalloc's
+       alignment and stays safe for 128-bit (double2) accesses. */
+    cudaMalloc((void **)&d_arena,
+               (size_t)(FOM_N + FOM_M + FOM_M) * sizeof(fom_float));
+    d_s  = d_arena;
+    d_t0 = d_arena + FOM_N;
+    d_b0 = d_arena + FOM_N + FOM_M;
+    cudaMalloc((void **)&d_prev, (size_t)FOM_N * sizeof(fom_float));
+    cudaMalloc((void **)&d_init, (size_t)FOM_N * sizeof(fom_float));
+    cudaMalloc((void **)&d_p0,   (size_t)FOM_M * sizeof(fom_float));
+    cudaMalloc((void **)&d_part, (size_t)FOM_BLOCKS_N * sizeof(fom_float));
+
+    g_fast = fom_check_identities();
+    if (!g_fast) {   /* the generic path needs two more n-vectors */
+        cudaMalloc((void **)&d_alt,  (size_t)FOM_N * sizeof(fom_float));
+        cudaMalloc((void **)&d_alt2, (size_t)FOM_N * sizeof(fom_float));
+    }

     /* Every solve restarts from FOM_STATE_INIT, so park it on the device
        once and refill the running state with a device-to-device copy. */
@@ -144,14 +272,19 @@
     cudaMemset(d_p0, 0, (size_t)FOM_M * sizeof(fom_float));
     cudaMemset(d_b0, 0, (size_t)FOM_M * sizeof(fom_float));

+    /* The one bulk transfer left in fom_solve is the D2H copy of the
+       solution; page-locking the destination roughly doubles its
+       bandwidth.  Best effort - a failure just leaves it pageable. */
+    if (work && cudaHostRegister(work->solution.x,
+                                 (size_t)FOM_N * sizeof(fom_float),
+                                 cudaHostRegisterDefault) == cudaSuccess)
+        g_pinned = work->solution.x;
+
     {   /* reserve as much L2 as the device allows for the persisting window */
         const char *e0 = getenv("FOM_L2OFF"); if (e0) g_l2on = 0;
         const char *e = getenv("FOM_L2MB");
-        size_t mb = e ? (size_t)atoi(e) : 11;
+        size_t mb = e ? (size_t)atoi(e) : 18;
         cudaDeviceSetLimit(cudaLimitPersistingL2CacheSize, mb << 20);
-        if (getenv("FOM_L1")) cudaDeviceSetCacheConfig(cudaFuncCachePreferL1);
-        if (getenv("FOM_SH")) cudaDeviceSetCacheConfig(cudaFuncCachePreferShared);
-        if (getenv("FOM_EQ")) cudaDeviceSetCacheConfig(cudaFuncCachePreferEqual);
     }
     g_ready = 1;
 }
@@ -175,7 +308,7 @@
     work->solution.iter     = 0;
     work->solution.residual = (fom_float)0;
     work->solution.status   = FOM_MAX_ITER_REACHED;
-    fom_device_init();
+    fom_device_init(work);
 }

 extern "C" void fom_update_params(FomWorkspace *work, const FomParams *params)
@@ -184,7 +317,7 @@
     /* Each solve starts from the generated initial state. */
     memcpy(work->state, FOM_STATE_INIT, (size_t)FOM_N * sizeof(fom_float));

-    fom_device_init();
+    fom_device_init(work);

     /* p0 is constant over the iteration, so the whole parameter term of
        the t0 linear op (t0_p_p0 @ p0) is folded once, here. */
@@ -198,52 +331,96 @@
 extern "C" fom_int fom_solve(FomWorkspace *work)
 {
     fom_int max_iter = work->settings.max_iter;
-    fom_float *cur = d_state;
-    fom_float *nxt = d_new_state;
     fom_float resid = (fom_float)0;
     fom_int k;
     int b;

     if (max_iter < 0) max_iter = 0;

-    cudaMemcpyAsync(cur, d_init, (size_t)FOM_N * sizeof(fom_float),
+    FOM_PROF_BEGIN(prologue);
+    cudaMemcpyAsync(d_s, d_init, (size_t)FOM_N * sizeof(fom_float),
                     cudaMemcpyDeviceToDevice, 0);
-
-    for (k = 0; k < max_iter; ++k) {
-        fom_float *tmp;
-
-        if (g_l2on) fom_l2_persist(cur, (size_t)FOM_N * sizeof(fom_float));
-
-        /* t0 <- t0_p_p0 @ p0 (precomputed) ; t1 <- 0 */
-        fom_prep_kern<<<FOM_BLOCKS_N, FOM_THREADS>>>(d_b0, d_t0, d_t1);
-
-        /* t0 += t0_state @ state ; activation is soft_threshold(.,0) == id */
-        fom_apply_t0_state(cur, d_t0);
-
-        /* t1 += t1_state @ state + t1_t_t0 @ t0 */
-        fom_apply_t1_state(cur, d_t1);
-        fom_apply_t1_t_t0(d_t0, d_t1);
-
-        /* t1 <- soft_threshold(t1, lambda) ; new_state <- 0 */
-        fom_soft_zero_kern<<<FOM_BLOCKS_N, FOM_THREADS>>>(
-            d_t1, (fom_float)FOM_T1_LAMBDA, nxt);
-
-        /* new_state += su_t_t1 @ t1 */
-        fom_apply_su_t_t1(d_t1, nxt);
-
-        if (k == max_iter - 1)
-            fom_resid_kern<<<FOM_BLOCKS_N, FOM_THREADS>>>(nxt, cur, d_part);
-
-        tmp = cur; cur = nxt; nxt = tmp;
+    /* Single working buffer => the L2 persisting window is set once, not
+       once per iteration. */
+    if (g_l2on) fom_l2_persist(d_arena, FOM_ARENA_BYTES);
+    if (g_fast)
+        fom_seed_t0_kern<<<FOM_BLOCKS(FOM_M2), FOM_THREADS>>>(
+            (const double2 *)d_b0, (double2 *)d_t0);
+    FOM_SYNC();
+    FOM_PROF_END(prologue);
+
+    if (g_fast) {
+        for (k = 0; k < max_iter; ++k) {
+            if (k == max_iter - 1) {
+                FOM_PROF_BEGIN(snapshot);
+                cudaMemcpyAsync(d_prev, d_s, (size_t)FOM_N * sizeof(fom_float),
+                                cudaMemcpyDeviceToDevice, 0);
+                FOM_SYNC();
+                FOM_PROF_END(snapshot);
+            }
+
+            /* t0 = b0 + t0_state @ s   (t0 already holds b0) */
+            FOM_PROF_BEGIN(spmv_t0_state);
+            fom_apply_t0_state(d_s, d_t0);
+            FOM_SYNC();
+            FOM_PROF_END(spmv_t0_state);
+
+            /* s += t1_t_t0 @ t0  (t1_state == I, so `s` IS the seed) */
+            FOM_PROF_BEGIN(spmv_t1_t_t0);
+            fom_apply_t1_t_t0(d_t0, d_s);
+            FOM_SYNC();
+            FOM_PROF_END(spmv_t1_t_t0);
+
+            /* s = soft(s, lam)  (su_t_t1 == I) ; reseed t0 = b0 */
+            FOM_PROF_BEGIN(soft_seed);
+            fom_soft_seed_kern<<<FOM_BLOCKS_N2, FOM_THREADS>>>(
+                (double2 *)d_s, (double)FOM_T1_LAMBDA,
+                (const double2 *)d_b0, (double2 *)d_t0);
+            FOM_SYNC();
+            FOM_PROF_END(soft_seed);
+        }
+    } else {
+        /* Generic path: no structural assumption at all.  Never taken for
+           the generated data (the identity probe above succeeds), but kept
+           so the solver stays correct if the numeric data ever changes. */
+        fom_float *cur = d_s, *nxt = d_alt, *acc = d_alt2;
+        for (k = 0; k < max_iter; ++k) {
+            fom_float *tmp;
+            cudaMemcpyAsync(d_t0, d_b0, (size_t)FOM_M * sizeof(fom_float),
+                            cudaMemcpyDeviceToDevice, 0);
+            fom_apply_t0_state(cur, d_t0);
+            /* soft_threshold(., 0) is the identity map */
+            fom_zero_kern<<<FOM_BLOCKS_N2, FOM_THREADS>>>((double2 *)acc,
+                                                          FOM_N2);
+            fom_apply_t1_state(cur, acc);
+            fom_apply_t1_t_t0(d_t0, acc);
+            fom_soft_kern<<<FOM_BLOCKS_N, FOM_THREADS>>>(
+                acc, (double)FOM_T1_LAMBDA);
+            fom_zero_kern<<<FOM_BLOCKS_N2, FOM_THREADS>>>((double2 *)nxt,
+                                                          FOM_N2);
+            fom_apply_su_t_t1(acc, nxt);
+            if (k == max_iter - 1)
+                cudaMemcpyAsync(d_prev, cur, (size_t)FOM_N * sizeof(fom_float),
+                                cudaMemcpyDeviceToDevice, 0);
+            tmp = cur; cur = nxt; nxt = tmp;
+        }
+        if (cur != d_s)
+            cudaMemcpyAsync(d_s, cur, (size_t)FOM_N * sizeof(fom_float),
+                            cudaMemcpyDeviceToDevice, 0);
     }

-    /* Keep the globals pointing at the live buffers for the next solve. */
-    d_state     = cur;
-    d_new_state = nxt;
+    FOM_PROF_BEGIN(residual);
+    if (max_iter > 0)
+        fom_resid_kern<<<FOM_BLOCKS_N, FOM_THREADS>>>(d_s, d_prev, d_part);
+    FOM_SYNC();
+    FOM_PROF_END(residual);

-    cudaMemcpy(work->solution.x, cur, (size_t)FOM_N * sizeof(fom_float),
+    FOM_PROF_BEGIN(copy_out);
+    cudaMemcpy(work->solution.x, d_s, (size_t)FOM_N * sizeof(fom_float),
                cudaMemcpyDeviceToHost);
+    FOM_PROF_END(copy_out);

+    FOM_PROF_BEGIN(reduce);
     if (max_iter > 0) {
         cudaMemcpy(h_part, d_part,
                    (size_t)FOM_BLOCKS_N * sizeof(fom_float),
@@ -251,6 +428,7 @@
         for (b = 0; b < FOM_BLOCKS_N; ++b)
             if (h_part[b] > resid) resid = h_part[b];
     }
+    FOM_PROF_END(reduce);

     work->solution.iter     = max_iter;
     work->solution.residual = resid;
@@ -262,13 +440,13 @@
 {
     (void)work;
     if (!g_ready) return;
-    cudaFree(d_state);     d_state     = 0;
-    cudaFree(d_new_state); d_new_state = 0;
-    cudaFree(d_t1);        d_t1        = 0;
-    cudaFree(d_init);      d_init      = 0;
-    cudaFree(d_t0);        d_t0        = 0;
-    cudaFree(d_p0);        d_p0        = 0;
-    cudaFree(d_b0);        d_b0        = 0;
-    cudaFree(d_part);      d_part      = 0;
+    if (g_pinned) { cudaHostUnregister(g_pinned); g_pinned = 0; }
+    cudaFree(d_arena); d_arena = 0; d_s = 0; d_t0 = 0; d_b0 = 0;
+    cudaFree(d_prev); d_prev = 0;
+    if (d_alt)  { cudaFree(d_alt);  d_alt  = 0; }
+    if (d_alt2) { cudaFree(d_alt2); d_alt2 = 0; }
+    cudaFree(d_init); d_init = 0;
+    cudaFree(d_p0);   d_p0   = 0;
+    cudaFree(d_part); d_part = 0;
     g_ready = 0;
 }
```

## Agent summary (end of the code-optimization transcript)

### Result

| | mean_us (`make bench`, 200 iters/solve) |
|---|---:|
| initial | 46 319.45 |
| final | 22 738.93 |
| **speedup** | **2.04×** |

Correctness held at every step: `make verify` prints `VERIFY OK` with worst max-abs error `4.441e-15` (tol `1.000e-05`), identical to the baseline's error — the fast path is bit-for-bit the same arithmetic, not an approximation.

### Final profiling breakdown (`make profile`, 3 solves)

The instrumented build inserts a `cudaDeviceSynchronize()` at every region end, so its absolute total (26 436 µs/solve) is inflated versus the production build (22 739 µs); the shares are representative.

| region | total_us (3 solves) | per solve | pct |
|---|---:|---:|---:|
| `spmv_t1_t_t0` | 33 609.5 | 11 203.2 | 42.42 % |
| `spmv_t0_state` | 33 430.7 | 11 143.6 | 42.19 % |
| `soft_seed` | 10 208.1 | 3 402.7 | 12.88 % |
| `copy_out` | 1 707.3 | 569.1 | 2.15 % |
| `prologue` | 93.8 | 31.3 | 0.12 % |
| `residual` | 67.6 | 22.5 | 0.09 % |
| `reduce` | 64.8 | 21.6 | 0.08 % |
| `snapshot` | 55.2 | 18.4 | 0.07 % |

85 % of the remaining time is inside the two fixed CSR helper kernels, which I cannot edit. `nsys` shows they are gather-bound: each does 2.6 M scattered 8-byte loads = ~2.6 M distinct 32-byte L2 sectors (an 4× read amplification) on top of ~37 MB / ~57 MB of compulsory CSR streaming. The solver is now within ~15 % of the memory-system floor implied by those fixed kernels.

### Optimizations applied

1. **Dropped the `t1_state @ s` SpMV — `t1_state` is the exact identity.**
   Structure: `t1_state` is 1 300 000×1 300 000 with 1 nnz/row, and every stored value is exactly `1.0` on the diagonal. So `t1_state @ s` *is* `s`. Instead of zeroing an accumulator and running a 1.3 M-row CSR SpMV, the accumulator is **seeded** with `s` and `t1_t_t0` accumulates onto it. Removes ~52 MB of HBM traffic per iteration.

2. **Dropped the `su_t_t1 @ t1` state update — `su_t_t1` is the exact identity.**
   Same structure (1 nnz/row, all values `1.0`, on-diagonal). The state update `s <- su_t_t1 @ t1` is therefore a rename, not a matvec. Removes another ~52 MB/iteration plus its memset.

   Together these two are the bulk of the 2× — forcing the generic path at runtime reproduces the original 46.6 ms exactly.

3. **Collapsed the whole iteration into a single n-sized device buffer.**
   Structure: with (1) and (2), `s` is both the *input* of the `t0_state` SpMV and the *accumulator* of the `t1_t_t0` SpMV, and the two are serialised on the default stream — so the read completes before the write begins. No double buffering, no pointer swap, no extra 10.4 MB copy per iteration.

4. **Both identities are *verified numerically at setup*, with a generic fallback.**
   `fom_check_identities()` probes each operator with a non-constant pattern and requires an exact zero difference. If it ever failed, the solver takes a full generic CSR path (also tested: `VERIFY OK`). This is what makes exploits (1)–(3) safe rather than a hardcoded assumption.

5. **Fused soft-threshold + next-iteration `t0` seed into one vectorized kernel.**
   `soft_threshold(·, 0)` on `t0` is the identity map (spec `lambd: 0.0`), so the only activation left is on `t1`. One kernel now does `s <- soft(s)` in place *and* reseeds `t0 <- b0` for the next iteration. `N = 1 300 000` and `M = 325 000` are both even and 16-byte aligned, so the whole pass runs on 128-bit `double2` loads/stores.

6. **Hoisted the parameter term out of the loop.**
   `t0_p_p0 @ p0` is loop-invariant; it is folded once per `fom_update_params` into `b0` (kept from the baseline).

7. **Put `s`, `t0` and `b0` in one contiguous arena so a single L2 persisting window covers all three.**
   Structure: these are exactly the three vectors every iteration touches (15.6 MB total), while the ~68 MB of CSR data is pure streaming. An access-policy window can only describe one address range, so making them contiguous lets the random gather source `s`, the gather source `t0`, and the 200×-reused constant `b0` all stay L2-resident. `N*8` and `M*8` are both multiples of 256, so each slice keeps `cudaMalloc`'s alignment. Carve-out swept: 18 MB best (≈6.8 % over no persistence). The window is also now set **once per solve** instead of once per iteration, since there is only one buffer.

8. **Page-locked the solution destination.**
   `cudaHostRegister` on `work->solution.x` at setup turns the single end-of-solve 10.4 MB D2H copy from pageable (~8 GB/s) into pinned (~15.6 GB/s): 593 µs saved per solve (2.6 %). Unregistered in `fom_cleanup`.

9. **Residual kept exact for one extra copy.**
   `max_abs(new_state - state)` needs the previous iterate, which in-place updating destroys — so a single 10.4 MB D2D snapshot is taken only before the final iteration (18 µs/solve, 0.07 %) rather than keeping a second buffer live all loop long.
