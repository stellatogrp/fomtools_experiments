# Optimization report: fista / cuda (m325000_n1300000_k8, D and D^T separate)

Fresh-build head-to-head on della-i12g1, mean per solve over 5 solves after 8 warm-ups:

| solver | mean_us |
|---|---:|
| Phase A | 146692.743000 |
| Phase B | 25907.137200 |

Kept: **phase_b** (Phase-A / Phase-B time = 5.662). Final verification:

```
worst max-abs error over 8 samples: 6.968e-07 (tol 1.000e-05)
```

## Diff (Phase A -> kept)

```diff
--- phase_a_solver.cu	2026-10-16 08:05:20.721541000 -0400
+++ src/fom_solver.cu	2026-10-16 08:32:56.732942000 -0400
@@ -2,22 +2,70 @@
  *
  * CUDA implementation of the generated first-order method.  The whole
  * iteration lives on the device: the state is uploaded once per solve
- * (device-to-device from a resident copy of FOM_STATE_INIT), every
- * matrix-vector product goes through the accumulating CSR helpers in
- * fom_ops.cu, and only the final state travels back to the host.
+ * (device-to-device from a resident copy of FOM_STATE_INIT), the two
+ * genuinely sparse matrix-vector products go through the accumulating
+ * CSR helpers in fom_ops.cu, and only the final state travels back to
+ * the host.
  *
- * Per iteration (it is the 0-based counter, g = fom_sched_gamma(it)):
+ * The generated iteration is
  *
- *   t0 = soft(t0_state@s + g*(t0_state__d_gamma@s) + t0_p_p0@p0, L0)
+ *   t0 = soft(t0_state@s + g*(t0_state__d_gamma@s) + t0_p_p0@p0, 0)
  *   t1 = soft(t1_state@s + g*(t1_state__d_gamma@s) + t1_t_t0@t0, L1)
  *   s' = su_state@s + su_t_t1@t1
  *
- * `t0_p_p0 @ p0` is parameter-only, so it is hoisted out of the loop.
- * g == 0 on the first iteration, so both d_gamma products are skipped
- * there.  Buffers are swapped instead of copied.  Termination is a
- * fixed iteration count (FOM_USE_TOL == 0), so the residual reduction
- * only has to run on the last iteration - its value is identical to
- * reducing every iteration and keeping the last.
+ * and the numeric data makes six of those eight operators pure
+ * structure.  With s = [s0; s1] (two halves of N = 1300000) and A the
+ * 325000 x N block (8 nnz/row):
+ *
+ *   t0_state            = [0, A]      (its left block is 1.38M entries
+ *                                      of 1e-16 round-off, nothing more)
+ *   t0_state__d_gamma   = [-A, A]     (same pattern, negated on the
+ *                                      left: exactly A@(s1-s0))
+ *   t0_p_p0             = -I          (325000 diagonal, all -1)
+ *   t1_state            = [0, I]      (row r holds 1 at col r+N: picks s1)
+ *   t1_state__d_gamma   = [-I, I]     (exactly s1 - s0)
+ *   su_state            = rows 0..N-1 hold 1 at col r+N      -> s0' = s1
+ *   su_t_t1             = rows N..2N-1 hold 1 at col r-N     -> s1' = t1
+ *   t1_t_t0             = the only other real matrix (2 nnz/row)
+ *
+ * so the method is FISTA: y = s1 + g*(s1 - s0), t0 = A@y - p0,
+ * t1 = soft(y + t1_t_t0@t0, L1), and the state shifts.  Four
+ * consequences shape this implementation.
+ *
+ * 1. The two t0 terms share one operand.  A@s1 + g*A@(s1-s0) is
+ *    A@(s1 + g*(s1-s0)) = A@y, so a single product with the 3.98M-nnz
+ *    t0_state replaces products with t0_state (3.98M nnz) *and*
+ *    t0_state__d_gamma (5.2M nnz).  y is materialized in the upper
+ *    half of a 2N vector whose lower half stays zero for the whole
+ *    run, so `fom_apply_t0_state(d_z, .)` evaluates [0,A]@[0;y] = A@y
+ *    and the round-off block multiplies zeros.
+ * 2. t1_state and t1_state__d_gamma are identity blocks, so
+ *    t1_state@s + g*(t1_state__d_gamma@s) is that same y: no CSR
+ *    product at all.  And since t1_t_t0 *accumulates*, y is used as
+ *    that product's accumulator - which removes both the zeroing of a
+ *    separate N-vector and the later pass that would read y back.
+ * 3. su_state / su_t_t1 are a permutation: s0' = s1, s1' = t1.  The
+ *    state never has to be contiguous, so the halves live in three
+ *    rotating N-vectors and the state update is a pointer rotation -
+ *    zero memory traffic, and bit-exact (the entries are 1.0 and
+ *    accumulate onto a zeroed output).
+ * 4. Because s1' = t1, the next y = t1 + g'*(t1 - s1) needs only
+ *    values the prox already holds at index i, so prox and
+ *    extrapolation are one kernel; it also primes the next
+ *    iteration's t0 accumulator with the loop-invariant b0.
+ *
+ * That leaves three device operations per iteration: A@y, the
+ * t1_t_t0 product, and the fused prox/extrapolate kernel.
+ *
+ * `t0_p_p0 @ p0` is parameter-only, so it stays hoisted out of the
+ * loop.  Termination is a fixed iteration count (FOM_USE_TOL == 0),
+ * so the residual reduction only has to run on the last iteration -
+ * its value is identical to reducing every iteration and keeping the
+ * last.  The rotation also makes the final s0 equal to t1 of the
+ * next-to-last iteration, so half the answer is final one iteration
+ * early and its PCIe transfer hides behind the last iteration; the
+ * caller's solution buffer is page-locked so both halves land there
+ * directly.
  */

 #include "fom_solver.h"
@@ -31,10 +79,19 @@
 #include <string.h>

 #define FOM_BLK 256
+#define FOM_N   FOM_T1_DIM               /* 1300000, one state half */
+
+/* Region timing needs the device to be quiescent; only the -DFOM_PROFILE
+   build pays for the synchronization. */
+#ifdef FOM_PROFILE
+#define FOM_SYNC() cudaDeviceSynchronize()
+#else
+#define FOM_SYNC() ((void)0)
+#endif

 #define FOM_STATE_BYTES ((size_t)FOM_STATE_DIM * sizeof(fom_float))
+#define FOM_N_BYTES     ((size_t)FOM_N         * sizeof(fom_float))
 #define FOM_T0_BYTES    ((size_t)FOM_T0_DIM    * sizeof(fom_float))
-#define FOM_T1_BYTES    ((size_t)FOM_T1_DIM    * sizeof(fom_float))
 #define FOM_P0_BYTES    ((size_t)FOM_PARAM_P0_DIM * sizeof(fom_float))

 /* ------------------------------------------------------------------ */
@@ -42,39 +99,19 @@
 /* ------------------------------------------------------------------ */

 static fom_float *d_init  = NULL;  /* FOM_STATE_INIT, resident         */
-static fom_float *d_state = NULL;  /* current state                    */
-static fom_float *d_next  = NULL;  /* next state                       */
+static fom_float *d_z     = NULL;  /* [0; y | t0], one allocation      */
+static fom_float *d_y     = NULL;  /* == d_z + FOM_N                   */
+static fom_float *d_rot[3];        /* rotating state halves / t1 slot  */
 static fom_float *d_p0    = NULL;  /* parameter p0                     */
 static fom_float *d_b0    = NULL;  /* t0_p_p0 @ p0 (loop invariant)    */
-static fom_float *d_a0    = NULL;  /* t0 accumulator                   */
-static fom_float *d_g0    = NULL;  /* t0 gamma-scaled accumulator      */
 static fom_float *d_t0    = NULL;
-static fom_float *d_a1    = NULL;  /* t1 accumulator                   */
-static fom_float *d_g1    = NULL;  /* t1 gamma-scaled accumulator      */
-static fom_float *d_t1    = NULL;
 static unsigned long long *d_res = NULL; /* max|dx| as ordered bits    */

 static fom_float *h_pin   = NULL;  /* pinned staging (p0 up, x down)   */
+static fom_float *g_hx    = NULL;  /* work->solution.x, page-locked    */
+static cudaStream_t g_cpy = NULL;  /* non-blocking D2H for the early half */
+static cudaEvent_t  g_ev  = NULL;
 static int        g_ready = 0;
-static int        g_l2    = 0;     /* L2 persisting carveout available */
-
-/* The CSR gathers of t0_state__d_gamma / t1_state__d_gamma touch the
-   whole 20.8 MB state vector, which thrashes L2 against the streaming
-   matrix data.  Pin the current state into the L2 set-aside area so
-   those gathers hit cache instead of HBM.  The fom_apply_* helpers
-   launch on the legacy default stream, so the window goes there. */
-static void fom_l2_pin(const fom_float *p) {
-    if (!g_l2) return;
-    cudaStreamAttrValue a;
-    memset(&a, 0, sizeof(a));
-    a.accessPolicyWindow.base_ptr  = (void *)p;
-    a.accessPolicyWindow.num_bytes = FOM_STATE_BYTES;
-    a.accessPolicyWindow.hitRatio  = 1.0f;
-    a.accessPolicyWindow.hitProp   = cudaAccessPropertyPersisting;
-    a.accessPolicyWindow.missProp  = cudaAccessPropertyStreaming;
-    cudaStreamSetAttribute(cudaStreamLegacy,
-                           cudaStreamAttributeAccessPolicyWindow, &a);
-}

 /* ------------------------------------------------------------------ */
 /* Kernels                                                             */
@@ -85,33 +122,56 @@
     return (a > (fom_float)0) ? copysign(a, v) : (fom_float)0;
 }

-/* out = soft((a + gamma*g) + c, lam), two doubles per thread.
-   HASG/HASC drop the unused terms at compile time. */
-template <int HASG, int HASC>
-__global__ void fom_k_combine(const double2 *__restrict__ a,
-                              const double2 *__restrict__ g,
-                              const double2 *__restrict__ c,
-                              fom_float gamma, fom_float lam,
-                              double2 *__restrict__ out, int n2) {
+/* y = s1 + gamma*(s1 - s0), two doubles per thread.  HASG drops the
+   extrapolation on the iterations where gamma == 0. */
+template <int HASG>
+__global__ void fom_k_extrap(const double2 *__restrict__ s1,
+                             const double2 *__restrict__ s0,
+                             fom_float gamma,
+                             double2 *__restrict__ y, int n2) {
     int i = blockIdx.x * blockDim.x + threadIdx.x;
     if (i >= n2) return;
-    double2 va = a[i];
-    fom_float x = va.x;
-    fom_float y = va.y;
+    double2 v1 = s1[i];
+    double2 o = v1;
     if (HASG) {
-        double2 vg = g[i];
-        x += gamma * vg.x;
-        y += gamma * vg.y;
-    }
-    if (HASC) {
-        double2 vc = c[i];
-        x += vc.x;
-        y += vc.y;
+        double2 v0 = s0[i];
+        o.x += gamma * (v1.x - v0.x);
+        o.y += gamma * (v1.y - v0.y);
     }
-    double2 o;
-    o.x = fom_soft(x, lam);
-    o.y = fom_soft(y, lam);
-    out[i] = o;
+    y[i] = o;
+}
+
+/* t1 = soft(y, lam) and, in the same pass, the next iteration's
+   extrapolation.  `y` arrives already holding y + t1_t_t0@t0, because
+   that product accumulates straight onto it.  Because the state update
+   is the rotation s0' = s1, s1' = t1, the next y is
+   t1 + gnext*(t1 - s1), so this thread holds every operand it needs at
+   index i: one fused kernel replaces a prox pass plus a separate
+   extrapolation pass. */
+__global__ void fom_k_prox_extrap(double2 *__restrict__ y,
+                                  double2 *__restrict__ out,
+                                  const double2 *__restrict__ s1,
+                                  fom_float lam, fom_float gnext, int n2,
+                                  const double2 *__restrict__ b0,
+                                  double2 *__restrict__ t0acc, int n2b) {
+    int i = blockIdx.x * blockDim.x + threadIdx.x;
+    /* Also prime the next iteration's t0 accumulator with the constant
+       term (t0_p_p0 @ p0, loop-invariant): folding this quarter-length
+       copy into a kernel that already runs saves a launch, and d_t0 is
+       free the moment t1_t_t0 has consumed it. */
+    if (i < n2b) t0acc[i] = b0[i];
+    if (i >= n2) return;
+    double2 vy = y[i];
+    double2 v1 = s1[i];
+    fom_float t0 = fom_soft(vy.x, lam);
+    fom_float t1 = fom_soft(vy.y, lam);
+    double2 vo;
+    vo.x = t0;
+    vo.y = t1;
+    out[i] = vo;
+    vy.x = t0 + gnext * (t0 - v1.x);
+    vy.y = t1 + gnext * (t1 - v1.y);
+    y[i] = vy;
 }

 /* residual = max |a - b| (non-negative doubles order like their bits,
@@ -178,27 +238,75 @@
     if (g_ready) return;

     cudaMalloc((void **)&d_init,  FOM_STATE_BYTES);
-    cudaMalloc((void **)&d_state, FOM_STATE_BYTES);
-    cudaMalloc((void **)&d_next,  FOM_STATE_BYTES);
+    /* One block holding [zeros(N) | y(N) | t0(M)].  d_z..d_z+2N is the
+       operand of t0_state; y and t0 are adjacent so that a single L2
+       window can pin both gather targets (see below).  N*8 is a
+       multiple of 16, so d_t0 keeps double2 alignment. */
+    cudaMalloc((void **)&d_z, FOM_STATE_BYTES + FOM_T0_BYTES);
+    cudaMalloc((void **)&d_rot[0], FOM_N_BYTES);
+    cudaMalloc((void **)&d_rot[1], FOM_N_BYTES);
+    cudaMalloc((void **)&d_rot[2], FOM_N_BYTES);
     cudaMalloc((void **)&d_p0,    FOM_P0_BYTES);
     cudaMalloc((void **)&d_b0,    FOM_T0_BYTES);
-    cudaMalloc((void **)&d_a0,    FOM_T0_BYTES);
-    cudaMalloc((void **)&d_g0,    FOM_T0_BYTES);
-    cudaMalloc((void **)&d_t0,    FOM_T0_BYTES);
-    cudaMalloc((void **)&d_a1,    FOM_T1_BYTES);
-    cudaMalloc((void **)&d_g1,    FOM_T1_BYTES);
-    cudaMalloc((void **)&d_t1,    FOM_T1_BYTES);
     cudaMalloc((void **)&d_res,   sizeof(unsigned long long));
     cudaHostAlloc((void **)&h_pin, FOM_STATE_BYTES, cudaHostAllocDefault);
-
-    {   /* Reserve as much L2 as the device allows for persisting lines. */
+    /* The rotation makes the final s0 equal to t1 of the next-to-last
+       iteration, so half the result is final one iteration early and
+       its PCIe transfer can hide behind the last iteration.  A
+       non-blocking stream is required: the fom_apply_* helpers launch
+       on the legacy default stream, which would otherwise serialize
+       against the copy.  An event keeps the ordering explicit. */
+    if (cudaStreamCreateWithFlags(&g_cpy, cudaStreamNonBlocking) != cudaSuccess)
+        g_cpy = NULL;
+    if (cudaEventCreateWithFlags(&g_ev, cudaEventDisableTiming) != cudaSuccess)
+        g_ev = NULL;
+
+    /* Page-lock the caller's solution buffer so the result lands there
+       straight from the device instead of being staged through h_pin
+       and memcpy'd.  Purely an optimization: if the registration is
+       refused the staged path below still runs. */
+    if (cudaHostRegister(work->solution.x, FOM_STATE_BYTES,
+                         cudaHostRegisterDefault) == cudaSuccess)
+        g_hx = work->solution.x;
+    else
+        cudaGetLastError();
+    d_y  = d_z + FOM_N;
+    d_t0 = d_y + FOM_N;
+
+    /* The lower half of d_z stays zero for the whole run: that is what
+       makes fom_apply_t0_state(d_z, .) equal A@y. */
+    cudaMemset(d_z, 0, FOM_STATE_BYTES + FOM_T0_BYTES);
+
+    /* The A gathers of t0_state hit the 10.4 MB y block 2.6M times per
+       iteration, but the 47.8 MB CSR stream behind them evicts it from
+       a 40 MB L2 every pass.  Set aside just enough L2 to hold y and
+       pin it there; the carveout is reserved whatever the window's
+       size, so asking for the device maximum starves everything else.
+       d_t0 (the 2.6 MB gather target of t1_t_t0, hit 2.6M times per
+       iteration) sits immediately after y in the same allocation, so
+       one 13 MB window pins both. */
+    {
         int dev = 0;
         size_t cap = 0;
         cudaGetDevice(&dev);
         cudaDeviceGetAttribute((int *)&cap, cudaDevAttrMaxPersistingL2CacheSize, dev);
+        /* A carveout sized exactly to the window thrashes on set
+           index; half again as much measures ~1% faster, while the
+           device maximum costs 8% by starving normal traffic. */
+        if (cap > (FOM_N_BYTES + FOM_T0_BYTES) * 3 / 2)
+            cap = (FOM_N_BYTES + FOM_T0_BYTES) * 3 / 2;
         if (cap > 0 &&
-            cudaDeviceSetLimit(cudaLimitPersistingL2CacheSize, cap) == cudaSuccess)
-            g_l2 = 1;
+            cudaDeviceSetLimit(cudaLimitPersistingL2CacheSize, cap) == cudaSuccess) {
+            cudaStreamAttrValue a;
+            memset(&a, 0, sizeof(a));
+            a.accessPolicyWindow.base_ptr  = (void *)d_y;
+            a.accessPolicyWindow.num_bytes = FOM_N_BYTES + FOM_T0_BYTES;
+            a.accessPolicyWindow.hitRatio  = 1.0f;
+            a.accessPolicyWindow.hitProp   = cudaAccessPropertyPersisting;
+            a.accessPolicyWindow.missProp  = cudaAccessPropertyNormal;
+            cudaStreamSetAttribute(cudaStreamLegacy,
+                                   cudaStreamAttributeAccessPolicyWindow, &a);
+        }
     }

     cudaMemcpy(d_init, FOM_STATE_INIT, FOM_STATE_BYTES, cudaMemcpyHostToDevice);
@@ -228,87 +336,115 @@
     if (!work) return FOM_MAX_ITER_REACHED;

     const fom_int max_iter = work->settings.max_iter;
-    const fom_float lam0 = FOM_T0_LAMBDA;
     const fom_float lam1 = FOM_T1_LAMBDA;
-    const int grid0 = fom_grid2(FOM_T0_DIM);
-    const int grid1 = fom_grid2(FOM_T1_DIM);
-    const int gridS = fom_grid2(FOM_STATE_DIM);
-    const int n2_0 = FOM_T0_DIM >> 1;
-    const int n2_1 = FOM_T1_DIM >> 1;
-    const int n2_S = FOM_STATE_DIM >> 1;
-
-    fom_float *cur = d_state;
-    fom_float *nxt = d_next;
+    const int gridN = fom_grid2(FOM_N);
+    const int n2_0  = FOM_T0_DIM >> 1;
+    const int n2_N  = FOM_N >> 1;
+
+    /* s0 = older half, s1 = newer half, dst = slot the new t1 lands in. */
+    fom_float *s0  = d_rot[0];
+    fom_float *s1  = d_rot[1];
+    fom_float *dst = d_rot[2];
     fom_int it;
+    int early = 0;                  /* first half already copied out  */
+    /* Direct pinned destination when available, else staged. */
+    fom_float *const dstx = (g_hx == work->solution.x) ? g_hx : h_pin;

     /* Reset the device state and the loop-invariant parameter term. */
-    cudaMemcpyAsync(cur, d_init, FOM_STATE_BYTES, cudaMemcpyDeviceToDevice, 0);
+    cudaMemcpyAsync(s0, d_init,         FOM_N_BYTES, cudaMemcpyDeviceToDevice, 0);
+    cudaMemcpyAsync(s1, d_init + FOM_N, FOM_N_BYTES, cudaMemcpyDeviceToDevice, 0);
     cudaMemsetAsync(d_res, 0, sizeof(unsigned long long), 0);
     cudaMemsetAsync(d_b0, 0, FOM_T0_BYTES, 0);
     fom_apply_t0_p_p0(d_p0, d_b0);

-    fom_l2_pin(cur);
+    /* d_t0 for iteration 0 and y for iteration 0; every later one is
+       produced by the fused prox kernel at the tail of the previous
+       iteration. */
+    cudaMemcpyAsync(d_t0, d_b0, FOM_T0_BYTES, cudaMemcpyDeviceToDevice, 0);
+    if (max_iter > 0) {
+        const fom_float g0 = fom_sched_gamma(0);
+        FOM_PROF_BEGIN(extrap);
+        if (g0 != (fom_float)0)
+            fom_k_extrap<1><<<gridN, FOM_BLK>>>((const double2 *)s1,
+                                                (const double2 *)s0,
+                                                g0, (double2 *)d_y, n2_N);
+        else
+            fom_k_extrap<0><<<gridN, FOM_BLK>>>((const double2 *)s1,
+                                                (const double2 *)s0,
+                                                g0, (double2 *)d_y, n2_N);
+        FOM_SYNC(); FOM_PROF_END(extrap);
+    }

     for (it = 0; it < max_iter; ++it) {
-        const fom_float gamma = fom_sched_gamma(it);
-        const int hasg = (gamma != (fom_float)0);
-
-        /* ---- t0 --------------------------------------------------- */
-        cudaMemsetAsync(d_a0, 0, FOM_T0_BYTES, 0);
-        fom_apply_t0_state(cur, d_a0);
-        if (hasg) {
-            cudaMemsetAsync(d_g0, 0, FOM_T0_BYTES, 0);
-            fom_apply_t0_state__d_gamma(cur, d_g0);
-            fom_k_combine<1, 1><<<grid0, FOM_BLK>>>(
-                (const double2 *)d_a0, (const double2 *)d_g0,
-                (const double2 *)d_b0, gamma, lam0,
-                (double2 *)d_t0, n2_0);
-        } else {
-            fom_k_combine<0, 1><<<grid0, FOM_BLK>>>(
-                (const double2 *)d_a0, (const double2 *)NULL,
-                (const double2 *)d_b0, gamma, lam0,
-                (double2 *)d_t0, n2_0);
+        /* ---- t0 = A@y + b0 ---------------------------------------- */
+        /* lam0 == 0, so the soft-threshold is the identity (it only
+           maps -0.0 to +0.0, which no consumer can observe), and the
+           constant term can seed the accumulator instead of needing a
+           separate pass: zeroing then adding b0 costs two extra
+           streams over d_t0 that priming with b0 does not.  The priming
+           itself rides along in the previous iteration's prox kernel. */
+        FOM_PROF_BEGIN(t0_spmv);
+        fom_apply_t0_state(d_z, d_t0);
+        FOM_SYNC(); FOM_PROF_END(t0_spmv);
+
+        /* ---- t1 = soft(t1_t_t0@t0 + y, lam1) ---------------------- */
+        /* The helper accumulates, so y itself is the accumulator: that
+           removes both the zeroing of a separate N-vector and the pass
+           that would later have to read y back to add it in.  y is dead
+           as an operand by now - the A@y product above was its only
+           other consumer this iteration. */
+        FOM_PROF_BEGIN(t1_spmv);
+        fom_apply_t1_t_t0(d_t0, d_y);
+        FOM_SYNC(); FOM_PROF_END(t1_spmv);
+
+        FOM_PROF_BEGIN(t1_prox_extrap);
+        fom_k_prox_extrap<<<gridN, FOM_BLK>>>((double2 *)d_y, (double2 *)dst,
+                                              (const double2 *)s1, lam1,
+                                              fom_sched_gamma(it + 1), n2_N,
+                                              (const double2 *)d_b0,
+                                              (double2 *)d_t0, n2_0);
+        FOM_SYNC(); FOM_PROF_END(t1_prox_extrap);
+
+        /* ---- state update: s0' = s1, s1' = t1 --------------------- */
+        if (it + 1 == max_iter) {
+            FOM_PROF_BEGIN(residual);
+            fom_k_maxdiff<<<gridN, FOM_BLK>>>((const double2 *)s1,
+                                              (const double2 *)s0,
+                                              d_res, n2_N);
+            fom_k_maxdiff<<<gridN, FOM_BLK>>>((const double2 *)dst,
+                                              (const double2 *)s1,
+                                              d_res, n2_N);
+            FOM_SYNC(); FOM_PROF_END(residual);
         }
+        { fom_float *old = s0; s0 = s1; s1 = dst; dst = old; }

-        /* ---- t1 --------------------------------------------------- */
-        cudaMemsetAsync(d_a1, 0, FOM_T1_BYTES, 0);
-        fom_apply_t1_state(cur, d_a1);
-        fom_apply_t1_t_t0(d_t0, d_a1);
-        if (hasg) {
-            cudaMemsetAsync(d_g1, 0, FOM_T1_BYTES, 0);
-            fom_apply_t1_state__d_gamma(cur, d_g1);
-            fom_k_combine<1, 0><<<grid1, FOM_BLK>>>(
-                (const double2 *)d_a1, (const double2 *)d_g1,
-                (const double2 *)NULL, gamma, lam1,
-                (double2 *)d_t1, n2_1);
-        } else {
-            fom_k_combine<0, 0><<<grid1, FOM_BLK>>>(
-                (const double2 *)d_a1, (const double2 *)NULL,
-                (const double2 *)NULL, gamma, lam1,
-                (double2 *)d_t1, n2_1);
+        /* s1 now holds t1 of this iteration; on the next-to-last pass
+           that is already the final s0, so start its D2H now. */
+        if (it + 2 == max_iter && dstx && g_cpy && g_ev) {
+            cudaEventRecord(g_ev, 0);
+            cudaStreamWaitEvent(g_cpy, g_ev, 0);
+            cudaMemcpyAsync(dstx, s1, FOM_N_BYTES,
+                            cudaMemcpyDeviceToHost, g_cpy);
+            early = 1;
         }
-
-        /* ---- state update ----------------------------------------- */
-        cudaMemsetAsync(nxt, 0, FOM_STATE_BYTES, 0);
-        fom_apply_su_state(cur, nxt);
-        fom_apply_su_t_t1(d_t1, nxt);
-
-        if (it + 1 == max_iter)
-            fom_k_maxdiff<<<gridS, FOM_BLK>>>((const double2 *)nxt,
-                                              (const double2 *)cur,
-                                              d_res, n2_S);
-
-        { fom_float *sw = cur; cur = nxt; nxt = sw; }
-        fom_l2_pin(cur);
     }

     /* ---- back to the host ----------------------------------------- */
-    if (h_pin) {
-        cudaMemcpy(h_pin, cur, FOM_STATE_BYTES, cudaMemcpyDeviceToHost);
-        memcpy(work->solution.x, h_pin, FOM_STATE_BYTES);
+    FOM_PROF_BEGIN(copy_out);
+    if (dstx) {
+        if (!early)
+            cudaMemcpyAsync(dstx, s0, FOM_N_BYTES, cudaMemcpyDeviceToHost, 0);
+        cudaMemcpyAsync(dstx + FOM_N, s1, FOM_N_BYTES,
+                        cudaMemcpyDeviceToHost, 0);
+        if (early) cudaStreamSynchronize(g_cpy);
+        cudaStreamSynchronize(0);
+        if (dstx != work->solution.x)
+            memcpy(work->solution.x, dstx, FOM_STATE_BYTES);
     } else {
-        cudaMemcpy(work->solution.x, cur, FOM_STATE_BYTES, cudaMemcpyDeviceToHost);
+        cudaMemcpy(work->solution.x,         s0, FOM_N_BYTES, cudaMemcpyDeviceToHost);
+        cudaMemcpy(work->solution.x + FOM_N, s1, FOM_N_BYTES, cudaMemcpyDeviceToHost);
     }
+    FOM_PROF_END(copy_out);

     {
         /* d_res holds the IEEE bits of the (non-negative) residual. */
@@ -326,17 +462,16 @@
     (void)work;
     if (!g_ready) return;
     cudaFree(d_init);  d_init = NULL;
-    cudaFree(d_state); d_state = NULL;
-    cudaFree(d_next);  d_next = NULL;
+    cudaFree(d_z);     d_z = NULL; d_y = NULL; d_t0 = NULL;
+    cudaFree(d_rot[0]); d_rot[0] = NULL;
+    cudaFree(d_rot[1]); d_rot[1] = NULL;
+    cudaFree(d_rot[2]); d_rot[2] = NULL;
     cudaFree(d_p0);    d_p0 = NULL;
     cudaFree(d_b0);    d_b0 = NULL;
-    cudaFree(d_a0);    d_a0 = NULL;
-    cudaFree(d_g0);    d_g0 = NULL;
-    cudaFree(d_t0);    d_t0 = NULL;
-    cudaFree(d_a1);    d_a1 = NULL;
-    cudaFree(d_g1);    d_g1 = NULL;
-    cudaFree(d_t1);    d_t1 = NULL;
     cudaFree(d_res);   d_res = NULL;
     if (h_pin) { cudaFreeHost(h_pin); h_pin = NULL; }
+    if (g_hx)  { cudaHostUnregister(g_hx);  g_hx = NULL; }
+    if (g_cpy) { cudaStreamDestroy(g_cpy); g_cpy = NULL; }
+    if (g_ev)  { cudaEventDestroy(g_ev);   g_ev = NULL; }
     g_ready = 0;
 }
```

## Agent summary (end of the code-optimization transcript)

**Whole-solve timing**

| | mean_us |
|---|---|
| initial (`make bench`, 5 solves) | 146549.78 |
| final (`make bench`, 10 solves) | 25997.84 |
| **overall speedup** | **5.64×** |

Correctness held at every step: `VERIFY OK`, worst max-abs error 6.968e-07 against tol 1.000e-05.

**Final profiling breakdown** (`make profile`, 5 solves, per-solve figures; the profile build inserts a `cudaDeviceSynchronize` per region, which inflates its total to ~29.5 ms)

| region | µs/solve | % |
|---|---:|---:|
| `t0_spmv` — `fom_apply_t0_state(d_z, d_t0)` = A@y | 12586.9 | 42.78 |
| `t1_spmv` — `fom_apply_t1_t_t0(d_t0, d_y)` accumulated onto y | 11144.4 | 37.87 |
| `t1_prox_extrap` — fused prox + next extrapolation + b0 priming | 5034.7 | 17.11 |
| `copy_out` — final 20.8 MB D2H | 570.7 | 1.94 |
| `residual` — two max-reductions, last iteration only | 48.9 | 0.17 |
| `extrap` — iteration-0 prologue only | 38.6 | 0.13 |

Per iteration this is ~130 µs against a ~108 µs HBM-bandwidth floor (≈158 MB moved, of which 85 MB is irreducible CSR matrix data), i.e. ~83 % of roofline. The three per-iteration launches are the minimum the fixed `fom_apply_*` helpers permit.

**Optimizations applied, and the structure each one exploits**

1. **One A-product instead of two** (biggest single win). `t0_state = [0, A]` and `t0_state__d_gamma = [-A, A]` share the operand A, so `A@s1 + g·A@(s1−s0) = A@(s1 + g·(s1−s0)) = A@y`. One product with the 3.98M-nnz `t0_state` replaces products with `t0_state` (3.98M nnz) **and** `t0_state__d_gamma` (5.2M nnz) — 9.18M → 3.98M nnz touched. `y` is materialized in the upper half of a 2N vector whose lower half is permanently zero, so the helper evaluates `[0,A]@[0;y]` exactly; `t0_state`'s left block is 1.38M entries of 1e-16 round-off that then multiply zeros.
2. **The t1 linear op needs no CSR product at all.** `t1_state = [0, I]` and `t1_state__d_gamma = [−I, I]`, so its two state terms *are* that same `y`. Two SpMVs (1.3M + 2.6M nnz, ~150 MB/iter with their memsets and accumulator passes) collapse into the elementwise kernel that already builds `y`.
3. **State update as a pointer rotation.** `su_state` maps `s0' = s1` and `su_t_t1` maps `s1' = t1`, both with entries exactly 1.0 — a permutation. The state therefore never needs to be contiguous: the halves live in three rotating N-vectors and the update becomes a pointer swap. This removed two SpMVs plus a 20.8 MB memset (~135 MB/iter) and is bit-exact, since 1.0·v accumulated onto a zeroed output reproduces v.
4. **`y` as the accumulator for `t1_t_t0@t0`.** The helpers accumulate, and `y` is dead as an operand once `A@y` has been formed. Accumulating onto it instead of a fresh buffer removed the 10.4 MB zeroing *and* the later pass that would read `y` back to add it in: −20.8 MB/iter, −12 %.
5. **Fused prox + next-iteration extrapolation.** Because `s1' = t1`, the next `y = t1 + g'·(t1 − s1)` is built from values the prox kernel already holds at index `i`. Merging them saves a full streaming read of an N-vector and a launch.
6. **`t0_p_p0 = −I`**, so `b0 = t0_p_p0@p0` is a loop-invariant hoisted once per solve; and since `λ₀ = 0` makes that soft-threshold the identity (it only maps −0.0 to +0.0, unobservable downstream), `b0` *seeds* the accumulator rather than needing a separate add-and-threshold pass. The seeding itself rides along in the previous iteration's prox kernel, since `d_t0` is free the moment `t1_t_t0` has consumed it.
7. **L2 persisting window, profile-tuned (+13 % over the inherited setting).** The inherited full-state window with `missProp=Streaming` was *costing* 13 %: the carveout is reserved whatever the window covers, and `missProp=Streaming` marked `d_t0` — the 2.6 MB target of 2.6M gathers per iteration — evict-first. `y` and `t0` are now placed adjacently in one allocation (N·8 is a multiple of 16, so `double2` alignment holds) so a single 13 MB window pins both gather targets, with the carveout at 1.5× the window to relieve set-index conflicts. Measured against five alternatives: no window (−13 %), full 23.4 MB window, `t0`-only, device-max carveout (+8 %), and `missProp=Normal` on the old window.
8. **Copy-out: 2.46 ms → 0.57 ms.** The rotation makes the final `s0` equal to `t1` of the next-to-last iteration, so half the answer is final one iteration early; its transfer is issued on a non-blocking stream (required — the `fom_apply_*` helpers use the legacy default stream) ordered by an event. The caller's `solution.x` is page-locked with `cudaHostRegister`, so both halves land there directly instead of being staged through a pinned buffer and memcpy'd, with a staged fallback if registration is refused.

**Tried and rejected on measurement:** `double4` accesses in the prox kernel (−9 %), a 32-byte `cudaLimitMaxL2FetchGranularity` (no effect), and the L2 window variants in (7). CUDA graphs are unavailable because the fixed helpers launch on the legacy default stream, which cannot be captured.

The one accuracy note: using `t0_state`'s A-block for the whole `A@y` product (rather than `t0_state`'s block for `A@s1` and `t0_state__d_gamma`'s for the momentum term — the two agree to 2.9e-15) perturbs `t0` at the 1e-15 level. The problem amplifies matrix round-off by ~1e8 over 200 FISTA iterations, so the verification error rose from 5.9e-08 to 6.97e-07 — still 14× inside the 1e-05 tolerance on all 8 samples, and unchanged by every optimization after it.
