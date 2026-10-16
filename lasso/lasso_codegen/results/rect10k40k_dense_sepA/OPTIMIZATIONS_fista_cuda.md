# Optimization report: fista / cuda (m10000_n40000_k260_sepA)

Fresh-build head-to-head on della-l08g3, mean per solve over 5 solves after 8 warm-ups:

| solver | mean_us |
|---|---:|
| Phase A | 67915.019800 |
| Phase B | 26665.590000 |

Kept: **phase_b** (Phase-A / Phase-B time = 2.547). Final verification:

```
worst max-abs error over 8 samples: 3.132e-08 (tol 1.000e-05)
```

## Diff (Phase A -> kept)

```diff
--- phase_a_solver.cu	2026-10-16 11:06:09.065273000 -0400
+++ src/fom_solver.cu	2026-10-16 11:32:14.591735000 -0400
@@ -2,12 +2,32 @@
  *
  * CUDA implementation of the generated first-order method.  The whole
  * iteration loop runs on the device: the state is uploaded once per solve,
- * the two working vectors ping-pong by pointer swap, and only the final
- * state (plus the scalar residual) travels back to the host.
+ * the working vectors rotate by pointer swap, and only the final state
+ * (plus the scalar residual) travels back to the host.
  *
- * All matrix-vector products go through the accumulating helpers in
- * fom_ops.cu; every accumulator is zeroed (or seeded) before its terms are
- * applied.  No matrix is ever formed, inverted or hardcoded here.
+ * Structure exploited.  Of the eight generated matrices only two carry
+ * real numeric data (both 2.6M nnz); the other six are signed identity /
+ * shift blocks over the (s0, s1) = (x_{k-1}, x_k) state split:
+ *
+ *     t0_state           = [ 0 | A ]        <- the only A-product needed
+ *     t0_state__d_gamma  = [-A | A ]        = A applied to (s1 - s0)
+ *     t0_p_p0            = -I               (10000 nnz, diagonal)
+ *     t1_state           = [ 0 | I ]
+ *     t1_state__d_gamma  = [-I | I ]
+ *     su_state, su_t_t1  = the shift (s0, s1, t1) -> (s1, t1)
+ *
+ * so the generated body collapses to plain FISTA on the half-state:
+ *
+ *     y   = x_k + (g*x_k - g*x_{k-1})       (all four identity terms)
+ *     t0  = A y - p0                        (one product, not three)
+ *     x+  = soft(y + t1_t_t0 @ t0)
+ *     state update = a pointer rotation of the (x_{k-1}, x_k) pair
+ *
+ * i.e. two sparse products per iteration instead of seven, and the
+ * gamma-weighted terms are folded into y instead of being applied as
+ * separate (and, for t0_state__d_gamma, twice as large) matrices.  Both
+ * numeric operators are still applied through the generated helpers in
+ * fom_ops.cu; no matrix is formed, inverted or hardcoded here.
  */
 
 #include <cuda_runtime.h>
@@ -22,69 +42,106 @@
 /* Launch geometry (ceil(n / 256) evaluated at code-generation time).   */
 /* ------------------------------------------------------------------ */
 #define FOM_TPB        256
-#define FOM_BLK_STATE  313            /* ceil(80000 / 256) */
-#define FOM_BLK_T1     157            /* ceil(40000 / 256) */
+#define FOM_BLK_N      157            /* ceil(40000 / 256) */
+#define FOM_N2         (FOM_T1_DIM / 2)        /* half-state in double2 */
+#define FOM_T02        (FOM_T0_DIM / 2)
+#define FOM_BLK_N2     79             /* ceil(20000 / 256): a single wave */
 #define FOM_RED_BLOCKS 128
 
+#define FOM_N           FOM_T1_DIM             /* 40000: half of the state */
 #define FOM_STATE_BYTES ((size_t)FOM_STATE_DIM * sizeof(fom_float))
+#define FOM_N_BYTES     ((size_t)FOM_N         * sizeof(fom_float))
 #define FOM_T0_BYTES    ((size_t)FOM_T0_DIM    * sizeof(fom_float))
-#define FOM_T1_BYTES    ((size_t)FOM_T1_DIM    * sizeof(fom_float))
 #define FOM_P0_BYTES    ((size_t)FOM_PARAM_P0_DIM * sizeof(fom_float))
 
+/* Region timers are host-side; make them meaningful in the profiling
+   build by draining the launch queue.  No-op in the shipped build. */
+#ifdef FOM_PROFILE
+#define FOM_SYNC() cudaDeviceSynchronize()
+#else
+#define FOM_SYNC() ((void)0)
+#endif
+
 /* ------------------------------------------------------------------ */
 /* Device state (allocated once in fom_setup, released in fom_cleanup). */
 /* ------------------------------------------------------------------ */
-static fom_float *d_pool      = 0;   /* single slab backing everything   */
-static fom_float *d_init      = 0;   /* FOM_STATE_INIT                   */
-static fom_float *d_state     = 0;   /* ping buffer                      */
-static fom_float *d_new_state = 0;   /* pong buffer                      */
-static fom_float *d_sx        = 0;   /* gamma * state                    */
-static fom_float *d_p0        = 0;   /* parameter p0                     */
-static fom_float *d_b0        = 0;   /* t0_p_p0 @ p0 (constant per solve)*/
-static fom_float *d_t0        = 0;   /* tmp:t0 accumulator               */
-static fom_float *d_t1        = 0;   /* tmp:t1 accumulator               */
-static fom_float *d_part      = 0;   /* residual partials                */
-static fom_float *d_res       = 0;   /* residual scalar                  */
-static int        g_ready     = 0;
+static fom_float *d_pool  = 0;   /* single slab backing everything      */
+static fom_float *d_init  = 0;   /* FOM_STATE_INIT                      */
+static fom_float *d_x[3]  = {0, 0, 0}; /* x_k / x_{k-1} / x_{k-2} ring  */
+static fom_float *d_p0    = 0;   /* parameter p0                        */
+static fom_float *d_b0    = 0;   /* t0_p_p0 @ p0 (constant per solve)   */
+static fom_float *d_t0    = 0;   /* tmp:t0 accumulator                  */
+static fom_float *d_part  = 0;   /* residual partials                   */
+static fom_float *d_res   = 0;   /* residual scalar                     */
+static int        g_ready = 0;
 
 /* ------------------------------------------------------------------ */
 /* Kernels                                                              */
 /* ------------------------------------------------------------------ */
 
-/* In-place soft threshold: v <- sign(v) * max(|v| - lam, 0). */
-__global__ void fom_k_soft(fom_float *__restrict__ v, fom_float lam, fom_int n) {
+/* soft(v) = sign(v) * max(|v| - lam, 0). */
+__device__ __forceinline__ fom_float fom_soft(fom_float a, fom_float lam) {
+    fom_float hi = a - lam;
+    fom_float lo = a + lam;
+    return (hi > (fom_float)0) ? hi : ((lo < (fom_float)0) ? lo : (fom_float)0);
+}
+
+/* Per-iteration prologue over the 40000-long half-state:
+     acc      <- y = x_k + (g*x_k - g*x_{k-1})   (the t0/t1 "state" terms)
+     t0[0:m]  <- b0 = t0_p_p0 @ p0               (hoisted out of the loop)
+   One vector does double duty: `acc` holds y for the A product (read
+   through the s1 window, see fom_solve) *and* seeds the t1 accumulator,
+   because the t1 "state" terms sum to exactly the same y.
+   The multiplies use __dmul_rn so the rounding matches the reference
+   two-kernel form (sx = g*state, then +/- sx) bit for bit. */
+__global__ void fom_k_prep(const double2 *__restrict__ x,
+                           const double2 *__restrict__ xm,
+                           fom_float g,
+                           double2 *__restrict__ acc,
+                           const double2 *__restrict__ b0,
+                           double2 *__restrict__ t0) {
     fom_int i = (fom_int)(blockIdx.x * blockDim.x + threadIdx.x);
-    if (i < n) {
-        fom_float a  = v[i];
-        fom_float hi = a - lam;
-        fom_float lo = a + lam;
-        v[i] = (hi > (fom_float)0) ? hi : ((lo < (fom_float)0) ? lo : (fom_float)0);
+    if (i < (fom_int)FOM_N2) {
+        double2 xi = x[i], xp = xm[i], r;
+        r.x = xi.x + (__dmul_rn(g, xi.x) - __dmul_rn(g, xp.x));
+        r.y = xi.y + (__dmul_rn(g, xi.y) - __dmul_rn(g, xp.y));
+        acc[i] = r;
+        if (i < (fom_int)FOM_T02) t0[i] = b0[i];
     }
 }
 
-/* Fused per-iteration prologue: pre-scale the state for the gamma-weighted
-   terms and prepare all three accumulators in a single launch.
-     sx  <- g * state          (shared source of both d_gamma terms)
-     nxt <- 0                  (state-update accumulator, written later)
-     t1  <- 0                  (t1 linear-op accumulator)
-     t0  <- b0 = t0_p_p0 @ p0  (state-independent term, hoisted out of loop)
-   `nxt` is never read before the su step, so zeroing it here is safe. */
-__global__ void fom_k_prolog(const fom_float *__restrict__ cur,
-                             fom_float g,
-                             fom_float *__restrict__ sx,
-                             fom_float *__restrict__ nxt,
-                             fom_float *__restrict__ t1,
-                             const fom_float *__restrict__ b0,
-                             fom_float *__restrict__ t0) {
+/* The activation of step t1 and the prologue of the next iteration touch
+   the same two vectors, so they run as a single kernel:
+     x_{k+1} <- soft(acc_raw)   (in place)
+     acc     <- y_{k+1}
+   This is the steady-state body kernel; the plain variants above/below
+   cover the first and last iteration. */
+__global__ void fom_k_soft_prep(double2 *__restrict__ x,
+                                const double2 *__restrict__ xm,
+                                fom_float lam,
+                                fom_float g,
+                                double2 *__restrict__ acc,
+                                const double2 *__restrict__ b0,
+                                double2 *__restrict__ t0) {
     fom_int i = (fom_int)(blockIdx.x * blockDim.x + threadIdx.x);
-    if (i < (fom_int)FOM_STATE_DIM) {
-        sx[i]  = g * cur[i];
-        nxt[i] = (fom_float)0;
-        if (i < (fom_int)FOM_T1_DIM) t1[i] = (fom_float)0;
-        if (i < (fom_int)FOM_T0_DIM) t0[i] = b0[i];
+    if (i < (fom_int)FOM_N2) {
+        double2 xi = x[i], xp = xm[i], r;
+        xi.x = fom_soft(xi.x, lam);
+        xi.y = fom_soft(xi.y, lam);
+        x[i] = xi;
+        r.x = xi.x + (__dmul_rn(g, xi.x) - __dmul_rn(g, xp.x));
+        r.y = xi.y + (__dmul_rn(g, xi.y) - __dmul_rn(g, xp.y));
+        acc[i] = r;
+        if (i < (fom_int)FOM_T02) t0[i] = b0[i];
     }
 }
 
+/* In-place soft threshold (last iteration / tol-checking path). */
+__global__ void fom_k_soft(fom_float *__restrict__ v, fom_float lam) {
+    fom_int i = (fom_int)(blockIdx.x * blockDim.x + threadIdx.x);
+    if (i < (fom_int)FOM_N) v[i] = fom_soft(v[i], lam);
+}
+
 /* Block-wide max reduction into *dst (blockDim.x == FOM_TPB == 256). */
 __device__ __forceinline__ void fom_block_max(fom_float m, fom_float *sm, fom_float *dst) {
     sm[threadIdx.x] = m;
@@ -108,18 +165,22 @@
     }
 }
 
-/* Stage 1: per-block max |a - b|. */
-__global__ void fom_k_resid_part(const fom_float *__restrict__ a,
-                                 const fom_float *__restrict__ b,
-                                 fom_float *__restrict__ part,
-                                 fom_int n) {
+/* Stage 1: per-block max over both halves of (new_state - state), which
+   for the pair layout is max(|q - p|, |r - q|) element-wise. */
+__global__ void fom_k_resid_part(const fom_float *__restrict__ p,
+                                 const fom_float *__restrict__ q,
+                                 const fom_float *__restrict__ r,
+                                 fom_float *__restrict__ part) {
     __shared__ fom_float sm[FOM_TPB];
     fom_float m = (fom_float)0;
     fom_int stride = (fom_int)(gridDim.x * blockDim.x);
-    for (fom_int i = (fom_int)(blockIdx.x * blockDim.x + threadIdx.x); i < n; i += stride) {
-        fom_float d = a[i] - b[i];
-        d = fabs(d);
-        if (d > m) m = d;
+    for (fom_int i = (fom_int)(blockIdx.x * blockDim.x + threadIdx.x);
+         i < (fom_int)FOM_N; i += stride) {
+        fom_float qi = q[i];
+        fom_float d0 = fabs(qi - p[i]);
+        fom_float d1 = fabs(r[i] - qi);
+        if (d1 > d0) d0 = d1;
+        if (d0 > m) m = d0;
     }
     fom_block_max(m, sm, part + blockIdx.x);
 }
@@ -135,8 +196,9 @@
     fom_block_max(m, sm, res);
 }
 
-static void fom_residual(const fom_float *a, const fom_float *b) {
-    fom_k_resid_part<<<FOM_RED_BLOCKS, FOM_TPB>>>(a, b, d_part, (fom_int)FOM_STATE_DIM);
+/* residual = max_abs(new_state - state) for state = (p, q), new = (q, r). */
+static void fom_residual(const fom_float *p, const fom_float *q, const fom_float *r) {
+    fom_k_resid_part<<<FOM_RED_BLOCKS, FOM_TPB>>>(p, q, r, d_part);
     fom_k_resid_final<<<1, FOM_TPB>>>(d_part, d_res, (fom_int)FOM_RED_BLOCKS);
 }
 
@@ -168,31 +230,31 @@
     fom_reset_state(work);
 
     if (!g_ready) {
-        /* One slab; every sub-buffer stays 256-byte aligned because each
-           offset below is a multiple of 32 doubles. */
-        const size_t n_state = (size_t)FOM_STATE_DIM;   /* 80000 */
-        const size_t n_t1    = (size_t)FOM_T1_DIM;      /* 40000 */
-        const size_t n_t0    = (size_t)FOM_T0_DIM;      /* 10000 */
-        const size_t n_p0    = (size_t)FOM_PARAM_P0_DIM;/* 10000 */
+        /* One slab; every sub-buffer is padded to a multiple of 32 doubles
+           so each stays 256-byte aligned. */
+        const size_t n_state = (size_t)FOM_STATE_DIM;        /* 80000 */
+        const size_t n_half  = (size_t)FOM_N;                /* 40000 */
+        const size_t n_t0    = 32u * (((size_t)FOM_T0_DIM + 31u) / 32u);
         const size_t n_small = 32u * ((size_t)FOM_RED_BLOCKS + 32u);
-        size_t total = 4u * n_state + n_t1 + 3u * n_t0 + n_p0 + n_small;
+        size_t total = n_state + 3u * n_half + 3u * n_t0 + n_small;
 
         if (cudaMalloc((void **)&d_pool, total * sizeof(fom_float)) != cudaSuccess) {
             d_pool = 0;
             return;
         }
         {
+            /* d_init comes first so that `x - FOM_N` (the s1 window used
+               by the A product, below) always lands inside the slab. */
             fom_float *p = d_pool;
-            d_init = p;      p += n_state;
-            d_state = p;     p += n_state;
-            d_new_state = p; p += n_state;
-            d_sx = p;        p += n_state;
-            d_t1 = p;        p += n_t1;
-            d_t0 = p;        p += n_t0;
-            d_b0 = p;        p += n_t0;
-            d_p0 = p;        p += n_p0;
-            d_part = p;      p += (size_t)FOM_RED_BLOCKS;
-            d_res = p;
+            d_init = p;   p += n_state;
+            d_x[0] = p;   p += n_half;
+            d_x[1] = p;   p += n_half;
+            d_x[2] = p;   p += n_half;
+            d_t0   = p;   p += n_t0;
+            d_b0   = p;   p += n_t0;
+            d_p0   = p;   p += n_t0;
+            d_part = p;   p += (size_t)FOM_RED_BLOCKS;
+            d_res  = p;
         }
         cudaMemcpy(d_init, FOM_STATE_INIT, FOM_STATE_BYTES, cudaMemcpyHostToDevice);
         g_ready = 1;
@@ -206,7 +268,11 @@
 }
 
 extern "C" fom_int fom_solve(FomWorkspace *work) {
-    fom_float *cur, *nxt, *tmp;
+    /* Ring of three half-state buffers: a = x_k, b = x_{k-1}, c = the
+       slot holding x_{k-2} (dead, so it receives x_{k+1}).  The generated
+       state update is state <- (s1, t1), i.e. a pure shift of the pair,
+       so the whole update is a pointer rotation. */
+    fom_float *a, *b, *c, *t;
     fom_int max_iter, it, done, status;
     fom_int check_every, countdown;
     fom_float tol, host_res;
@@ -222,85 +288,102 @@
     countdown = check_every;
 
     FOM_PROF_BEGIN(upload);
-    /* Every solve restarts from FOM_STATE_INIT. */
-    cudaMemcpyAsync(d_state, d_init, FOM_STATE_BYTES, cudaMemcpyDeviceToDevice, 0);
+    /* Every solve restarts from FOM_STATE_INIT = (x_{-1}, x_0). */
+    a = d_x[0]; b = d_x[1]; c = d_x[2];
+    cudaMemcpyAsync(a, d_init + FOM_N, FOM_N_BYTES, cudaMemcpyDeviceToDevice, 0);
+    cudaMemcpyAsync(b, d_init,         FOM_N_BYTES, cudaMemcpyDeviceToDevice, 0);
     cudaMemcpyAsync(d_p0, work->params.p0, FOM_P0_BYTES, cudaMemcpyHostToDevice, 0);
 
     /* t0_p_p0 @ p0 does not depend on the state: hoist it out of the loop
        and seed the t0 accumulator with it each iteration. */
     cudaMemsetAsync(d_b0, 0, FOM_T0_BYTES, 0);
     fom_apply_t0_p_p0(d_p0, d_b0);
+    FOM_SYNC();
     FOM_PROF_END(upload);
 
-    cur = d_state;
-    nxt = d_new_state;
     done = 0;
     status = FOM_MAX_ITER_REACHED;
     host_res = (fom_float)0;
 
-    for (it = 0; it < max_iter; ++it) {
-        const fom_float g = fom_sched_gamma(it);
+    /* Prologue of iteration 0 (x_0 comes from the initial state, so it is
+       not thresholded; every later one is, inside fom_k_soft_prep). */
+    FOM_PROF_BEGIN(prolog);
+    if (max_iter > 0)
+        fom_k_prep<<<FOM_BLK_N2, FOM_TPB>>>((const double2 *)a, (const double2 *)b,
+                                            fom_sched_gamma(0), (double2 *)c,
+                                            (const double2 *)d_b0, (double2 *)d_t0);
+    FOM_SYNC();
+    FOM_PROF_END(prolog);
 
-        FOM_PROF_BEGIN(prolog);
-        fom_k_prolog<<<FOM_BLK_STATE, FOM_TPB>>>(cur, g, d_sx, nxt, d_t1, d_b0, d_t0);
-        FOM_PROF_END(prolog);
-
-        /* t0 = soft_threshold(t0_state @ s + g * (t0_state__d_gamma @ s)
-                               + t0_p_p0 @ p0, lambda = 0).
-           FOM_T0_LAMBDA is 0, so that activation is the identity and the
-           accumulator doubles as tmp:t0. */
-        FOM_PROF_BEGIN(t0);
-        fom_apply_t0_state(cur, d_t0);
-        if (g != (fom_float)0) fom_apply_t0_state__d_gamma(d_sx, d_t0);
-        FOM_PROF_END(t0);
-
-        /* t1 = soft_threshold(t1_state @ s + g * (t1_state__d_gamma @ s)
-                               + t1_t_t0 @ t0, FOM_T1_LAMBDA) */
-        FOM_PROF_BEGIN(t1);
-        fom_apply_t1_state(cur, d_t1);
-        if (g != (fom_float)0) fom_apply_t1_state__d_gamma(d_sx, d_t1);
-        fom_apply_t1_t_t0(d_t0, d_t1);
-        fom_k_soft<<<FOM_BLK_T1, FOM_TPB>>>(d_t1, (fom_float)FOM_T1_LAMBDA,
-                                            (fom_int)FOM_T1_DIM);
-        FOM_PROF_END(t1);
-
-        /* state <- su_state @ state + su_t_t1 @ t1  (nxt zeroed in prolog) */
-        FOM_PROF_BEGIN(su);
-        fom_apply_su_state(cur, nxt);
-        fom_apply_su_t_t1(d_t1, nxt);
-        FOM_PROF_END(su);
+    for (it = 0; it < max_iter; ++it) {
+        /* t0 = A @ y - p0.  The columns of t0_state all live in the s1
+           block of the state, so handing it `c - FOM_N` (a pointer that
+           stays inside the slab) makes it gather exactly y from c -- no
+           state-shaped copy of y is ever materialised.  The lambda-0
+           soft threshold of step t0 is the identity, so the accumulator
+           doubles as tmp:t0. */
+        FOM_PROF_BEGIN(spmv_A);
+        fom_apply_t0_state(c - FOM_N, d_t0);
+        FOM_SYNC();
+        FOM_PROF_END(spmv_A);
+
+        /* c <- y + t1_t_t0 @ t0  (the raw t1 linear op). */
+        FOM_PROF_BEGIN(spmv_At);
+        fom_apply_t1_t_t0(d_t0, c);
+        FOM_SYNC();
+        FOM_PROF_END(spmv_At);
 
+        /* state <- new_state is a pure shift of the pair: rotate. */
+        t = c; c = b; b = a; a = t;
         done = it + 1;
 
-        /* Residual / early exit. The generated termination rule is pure
-           max_iters (tol == 0), so this never syncs in the default build. */
         if (tol > (fom_float)0) {
-            --countdown;
-            if (countdown == 0) {
+            /* Cold path: the residual needs x_{k-1}, which the fused
+               prologue would overwrite, so keep the two steps apart. */
+            FOM_PROF_BEGIN(soft);
+            fom_k_soft<<<FOM_BLK_N, FOM_TPB>>>(a, (fom_float)FOM_T1_LAMBDA);
+            FOM_SYNC();
+            FOM_PROF_END(soft);
+            if (--countdown == 0) {
                 countdown = check_every;
-                fom_residual(nxt, cur);
+                fom_residual(c, b, a);
                 cudaMemcpy(&host_res, d_res, sizeof(fom_float), cudaMemcpyDeviceToHost);
-                if (host_res <= tol) {
-                    tmp = cur; cur = nxt; nxt = tmp;
-                    status = FOM_SOLVED;
-                    break;
-                }
+                if (host_res <= tol) { status = FOM_SOLVED; break; }
             }
+            if (it + 1 < max_iter)
+                fom_k_prep<<<FOM_BLK_N2, FOM_TPB>>>((const double2 *)a, (const double2 *)b,
+                                                    fom_sched_gamma(it + 1), (double2 *)c,
+                                                    (const double2 *)d_b0, (double2 *)d_t0);
+        } else if (it + 1 < max_iter) {
+            FOM_PROF_BEGIN(prolog);
+            fom_k_soft_prep<<<FOM_BLK_N2, FOM_TPB>>>((double2 *)a, (const double2 *)b,
+                                                     (fom_float)FOM_T1_LAMBDA,
+                                                     fom_sched_gamma(it + 1), (double2 *)c,
+                                                     (const double2 *)d_b0, (double2 *)d_t0);
+            FOM_SYNC();
+            FOM_PROF_END(prolog);
+        } else {
+            FOM_PROF_BEGIN(soft);
+            fom_k_soft<<<FOM_BLK_N, FOM_TPB>>>(a, (fom_float)FOM_T1_LAMBDA);
+            FOM_SYNC();
+            FOM_PROF_END(soft);
         }
-
-        tmp = cur; cur = nxt; nxt = tmp;
     }
 
-    /* `cur` holds the newest state, `nxt` the one it replaced. */
+    /* a = x_N, b = x_{N-1}, c = x_{N-2}: state = (c, b), new = (b, a). */
     if (status != FOM_SOLVED && done > 0) {
-        fom_residual(cur, nxt);
+        fom_residual(c, b, a);
         cudaMemcpy(&host_res, d_res, sizeof(fom_float), cudaMemcpyDeviceToHost);
     }
 
     FOM_PROF_BEGIN(download);
-    cudaMemcpy(work->solution.x, cur, FOM_STATE_BYTES, cudaMemcpyDeviceToHost);
+    if (done > 0) {
+        cudaMemcpyAsync(work->solution.x, b, FOM_N_BYTES, cudaMemcpyDeviceToHost, 0);
+        cudaMemcpy(work->solution.x + FOM_N, a, FOM_N_BYTES, cudaMemcpyDeviceToHost);
+        memcpy(work->state, work->solution.x, FOM_STATE_BYTES);
+    }
+    FOM_SYNC();
     FOM_PROF_END(download);
-    memcpy(work->state, work->solution.x, FOM_STATE_BYTES);
 
     work->solution.iter     = done;
     work->solution.residual = host_res;
```

## Agent notes (end of the Phase-B transcript)

```
The solver is now at the floor set by the two fixed helper kernels: 96.7 + 42.0 = 138.7 µs of SpMV against 132 µs/iteration of total production wall time. Both helpers are one-thread-per-row scalar CSR kernels in the immutable `fom_ops.cu`; `t0_state` has only 10000 rows (40 blocks on 108 SMs), which is why it costs 2.3× as much as `t1_t_t0` for identical nnz. That cannot be fixed from `fom_solver.cu`, and the two products are strictly dependent, so they cannot be overlapped.

## Optimizations applied

1. **Fold the gamma term into the vector instead of applying a second matrix** (the big one, ~2.5×). Read off the generated CSR: `t0_state = [0 | A]` (2.6M nnz, all columns in the s1 block) and `t0_state__d_gamma = [-A | +A]` (5.2M nnz, the same A pattern mirrored across both state blocks, values ±A to 1 ulp). So `t0_state @ s + g*(t0_state__d_gamma @ s) = A·((1+g)x_k - g x_{k-1}) = A y`. Building `y` in a 40000-element kernel and doing **one** 2.6M-nnz product replaces 7.8M nnz of SpMV per iteration. Structure exploited: a zero block plus a mirrored-±A block sharing one sparsity pattern.
2. **Replace the four identity-block matrices with arithmetic.** `t1_state = [0 | I]`, `t1_state__d_gamma = [-I | +I]` (values exactly ±1), so the t1 linear op's state terms are exactly the same `y`; it seeds the t1 accumulator instead of costing two kernel launches. `t0_p_p0 = -I` is hoisted once per solve (it only depends on `p0`) and memcpy-seeded each iteration. Structure exploited: signed identity blocks.
3. **The state update is a shift, so make it a pointer rotation.** `su_state` maps s1→the new s0 and `su_t_t1` maps t1→the new s1, both with value 1.0 — a pure permutation. Keeping the state as a 3-buffer ring of 40000-vectors (`x_k`, `x_{k-1}`, `x_{k-2}`) turns the state update into pointer swaps, removing two SpMV launches and an 80000-element `cudaMemset` per iteration. The 3rd slot is exactly what the residual `max_abs(new_state - state)` needs, since that reduces to `max(|x_{k}-x_{k-1}|, |x_{k+1}-x_k|)`.
4. **Let the A-product gather `y` straight out of the t1 accumulator.** Because every column of `t0_state` lies in the s1 block, passing `c - 40000` (a pointer that stays inside the single allocated slab, which starts with the 80000-element init buffer) makes the helper gather `y` from the 40000-element buffer that already holds it. No state-shaped copy of `y` is materialised at all — one 320 KB write and one 320 KB read removed per iteration.
5. **Fuse the t1 activation into the next iteration's prologue.** `soft(·)` on `x_{k+1}` and the extrapolation that reads it touch the same two vectors, so they became one kernel (`fom_k_soft_prep`), cutting the per-iteration launch count from 4 to 3 (−2.6%). The first and last iterations use unfused variants, and the `tol > 0` path keeps them separate because the residual needs `x_{k-1}` before the prologue overwrites it.
6. **Keep everything device-resident and allocation-free in the loop**: one `cudaMalloc` slab in `fom_setup`, all sub-buffers 256-byte aligned, params uploaded once per solve, one D2H of the final state. The prologue kernels use `double2` accesses so the whole half-state is covered by a single 79-block wave.
7. **Rounding discipline:** the prologue uses `__dmul_rn` for `g*x` so the extrapolation reproduces the reference's `sx = g*state` then `±sx` rounding exactly, keeping the only numerical deviation the 1-ulp A/Â difference in opt #1.

Ideas evaluated and rejected: CUDA graphs (the fixed helpers launch on the legacy default stream, which cannot be captured); multi-stream overlap (the A and A' products are strictly dependent); using `t0_state__d_gamma` as the single A-operator (same 10000 rows but 2× the nnz); caching `A x_k` across iterations (impossible — `x_k` is produced by the soft threshold after the product); pinned-staging the final download (≈55 µs/solve, 0.2%, below run-to-run noise).

FOMTOOLS-SUMMARY-END
Done. Final state: `make verify` → VERIFY OK (worst error 3.1e-08 vs tol 1e-05), `make bench` → 26408 µs.

FOMTOOLS-SUMMARY-BEGIN

## Result

| | `mean_us` (`make bench`, 3 solves) |
|---|---|
| initial | 67711.6 |
| final | 26408.4 |
| **speedup** | **2.56×** |

Re-measured back-to-back in one session to rule out clock drift (5 solves each, alternating): baseline 68020 µs vs final 26926 µs → **2.53×**. Accuracy is unchanged in kind: worst max-abs error over the 8 verify samples went 2.1e-09 → 3.1e-08, still ~300× inside the 1e-05 tolerance (the growth is the ~1-ulp difference between the `A` stored in `t0_state` and the one stored in `t0_state__d_gamma`, see opt #1).

## Final profile (hottest first)

`make profile`, 200 iterations/solve. The profiling build now issues a `cudaDeviceSynchronize()` before each `FOM_PROF_END` (no-op in production), so the regions measure real device time instead of launch-queue backlog — this costs ~1–2 µs per region, which is why the profile build totals 29.7 ms against 26.4 ms in production.

| region | total_us (3 solves) | per call | pct |
|---|---:|---:|---:|
| `spmv_A` — `fom_apply_t0_state` (A @ y, 2.6M nnz) | 58034 | 96.7 µs/iter | 65.2% |
| `spmv_At` — `fom_apply_t1_t_t0` (A' @ t0, 2.6M nnz) | 25196 | 42.0 µs/iter | 28.3% |
| `prolog` — fused soft-threshold + extrapolation kernel | 5361 | 8.9 µs/iter | 6.0% |
| `download` — final state D2H | 342 | 114 µs/solve | 0.4% |
| `upload` — init/params + hoisted `t0_p_p0 @ p0` | 124 | 41 µs/solve | 0.14% |
| `soft` — last-iteration activation | 24 | 7.9 µs/solve | 0.03% |

The solver is now at the floor set by the two fixed helper kernels: 96.7 + 42.0 = 138.7 µs of SpMV against 132 µs/iteration of total production wall time. Both helpers are one-thread-per-row scalar CSR kernels in the immutable `fom_ops.cu`; `t0_state` has only 10000 rows (40 blocks on 108 SMs), which is why it costs 2.3× as much as `t1_t_t0` for identical nnz. That cannot be fixed from `fom_solver.cu`, and the two products are strictly dependent, so they cannot be overlapped.

## Optimizations applied

1. **Fold the gamma term into the vector instead of applying a second matrix** (the big one, ~2.5×). Read off the generated CSR: `t0_state = [0 | A]` (2.6M nnz, all columns in the s1 block) and `t0_state__d_gamma = [-A | +A]` (5.2M nnz, the same A pattern mirrored across both state blocks, values ±A to 1 ulp). So `t0_state @ s + g*(t0_state__d_gamma @ s) = A·((1+g)x_k - g x_{k-1}) = A y`. Building `y` in a 40000-element kernel and doing **one** 2.6M-nnz product replaces 7.8M nnz of SpMV per iteration. Structure exploited: a zero block plus a mirrored-±A block sharing one sparsity pattern.
2. **Replace the four identity-block matrices with arithmetic.** `t1_state = [0 | I]`, `t1_state__d_gamma = [-I | +I]` (values exactly ±1), so the t1 linear op's state terms are exactly the same `y`; it seeds the t1 accumulator instead of costing two kernel launches. `t0_p_p0 = -I` is hoisted once per solve (it only depends on `p0`) and memcpy-seeded each iteration. Structure exploited: signed identity blocks.
3. **The state update is a shift, so make it a pointer rotation.** `su_state` maps s1→the new s0 and `su_t_t1` maps t1→the new s1, both with value 1.0 — a pure permutation. Keeping the state as a 3-buffer ring of 40000-vectors (`x_k`, `x_{k-1}`, `x_{k-2}`) turns the state update into pointer swaps, removing two SpMV launches and an 80000-element `cudaMemset` per iteration. The 3rd slot is exactly what the residual `max_abs(new_state - state)` needs, since that reduces to `max(|x_{k}-x_{k-1}|, |x_{k+1}-x_k|)`.
4. **Let the A-product gather `y` straight out of the t1 accumulator.** Because every column of `t0_state` lies in the s1 block, passing `c - 40000` (a pointer that stays inside the single allocated slab, which starts with the 80000-element init buffer) makes the helper gather `y` from the 40000-element buffer that already holds it. No state-shaped copy of `y` is materialised at all — one 320 KB write and one 320 KB read removed per iteration.
5. **Fuse the t1 activation into the next iteration's prologue.** `soft(·)` on `x_{k+1}` and the extrapolation that reads it touch the same two vectors, so they became one kernel (`fom_k_soft_prep`), cutting the per-iteration launch count from 4 to 3 (−2.6%). The first and last iterations use unfused variants, and the `tol > 0` path keeps them separate because the residual needs `x_{k-1}` before the prologue overwrites it.
6. **Keep everything device-resident and allocation-free in the loop**: one `cudaMalloc` slab in `fom_setup`, all sub-buffers 256-byte aligned, params uploaded once per solve, one D2H of the final state. The prologue kernels use `double2` accesses so the whole half-state is covered by a single 79-block wave.
7. **Rounding discipline:** the prologue uses `__dmul_rn` for `g*x` so the extrapolation reproduces the reference's `sx = g*state` then `±sx` rounding exactly, keeping the only numerical deviation the 1-ulp A/Â difference in opt #1.

Ideas evaluated and rejected: CUDA graphs (the fixed helpers launch on the legacy default stream, which cannot be captured); multi-stream overlap (the A and A' products are strictly dependent); using `t0_state__d_gamma` as the single A-operator (same 10000 rows but 2× the nnz); caching `A x_k` across iterations (impossible — `x_k` is produced by the soft threshold after the product); pinned-staging the final download (≈55 µs/solve, 0.2%, below run-to-run noise).

FOMTOOLS-SUMMARY-END./verify_bin
worst max-abs error over 8 samples: 3.132e-08 (tol 1.000e-05)
VERIFY OK
[fom-step in holder 15690383 on della-l08g3]
```
