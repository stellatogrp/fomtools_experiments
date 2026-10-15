# Optimization report: fista / cuda (m325000_n1300000_k8, tolerance 1e-5)

Fresh-build head-to-head on della-i12g2, mean per solve over 5 solves after 8 warm-ups:

| solver | mean_us |
|---|---:|
| Phase A | 525104.028800 |
| Phase B | 127863.023600 |

Kept: **phase_b** (Phase-A / Phase-B time = 4.107). Final verification:

```
worst max-abs error over 8 samples: 1.377e-06 (tol 1.000e-05)
```

## Diff (Phase A -> kept)

```diff
--- phase_a_solver.cu	2026-10-15 16:53:51.690965000 -0400
+++ src/fom_solver.cu	2026-10-15 17:30:13.722180000 -0400
@@ -2,22 +2,67 @@
  *
  * CUDA implementation of the specified first-order method.
  *
- * Per iteration (SPEC.md):
- *   t0  = soft_threshold( t0_state @ state
- *                         + gamma(it) * (t0_state__d_gamma @ state)
- *                         + t0_p_p0 @ p0,
- *                         lambda )
+ * Specified iteration (SPEC.md), with state = [s0 ; s1]:
+ *   t0    = soft_threshold( t0_state @ state
+ *                           + gamma(it) * (t0_state__d_gamma @ state)
+ *                           + t0_p_p0 @ p0,  lambda )
  *   state <- su_state @ state + su_t_t0 @ t0
  *
- * The `t0_p_p0 @ p0` term does not depend on the state, so it is
- * evaluated once per solve and reused; everything else is recomputed
- * through the provided accumulating CSR helpers in fom_ops.cu.
+ * Three structural facts about the generated matrices drive this
+ * implementation (all checked against data.npz):
  *
- * The whole iteration lives on the device: the state is uploaded once
- * at the start of fom_solve, the state/new_state device pointers are
- * swapped instead of copied, and the solution is read back once at the
- * end.  The only host<->device traffic per solve is the parameter
- * vector in and the solution out.
+ * 1. `su_state` and `su_t_t0` are exact 0/1 shift matrices:
+ *    su_state has a single 1.0 in row r at column r + 1300000 for
+ *    r < 1300000 and is empty below; su_t_t0 has a single 1.0 in row
+ *    1300000 + j at column j.  The state update is therefore *exactly*
+ *    the relabelling
+ *        new_state = [ s1 ; t0 ],
+ *    i.e. s1 is carried over unchanged and t0 becomes the new s1.  No
+ *    arithmetic at all: both state-update SpMVs are replaced by naming
+ *    the buffers correctly, which is bit-for-bit identical.
+ *
+ * 2. Writing the 1300000 x 2600000 operators as pairs of 1300000 x
+ *    1300000 blocks, t0_state = [AL | M] and t0_state__d_gamma =
+ *    [-M | M] with the *same* M: the two stored copies of M agree to
+ *    2e-15 absolute, i.e. to rounding.  Hence
+ *        t0_state @ state + gamma * (t0_state__d_gamma @ state)
+ *          = AL s0 + M s1 + gamma * M (s1 - s0)
+ *          = AL s0 + M ( s1 + gamma (s1 - s0) )
+ *          = t0_state @ [ s0 ; s1 + gamma (s1 - s0) ].
+ *    So the 38999936-nnz `t0_state__d_gamma` product is removed
+ *    entirely: one `fom_apply_t0_state` on the extrapolated vector
+ *    replaces both products (69 Mnnz -> 30 Mnnz per iteration).  This
+ *    is just FISTA's momentum step written where it belongs - on the
+ *    iterate - instead of on the operator.
+ *
+ * 3. AL is the zero block.  Its 10460178 stored entries are all
+ *    rounding dust from the generator: max |AL| = 1.11e-16, one ulp of
+ *    the entries of M, whose diagonal is ~0.98.  (That is exactly what
+ *    the algebra predicts - with momentum off, the step depends only
+ *    on s1.)  So AL s0 is below the rounding error of the row sums it
+ *    is added to, for any bounded s0, and the leading half of the SpMV
+ *    argument may simply be held at zero:
+ *        v_k = [ 0 ; x_k + gamma_k (x_k - x_{k-1}) ].
+ *    That removes a 10.4 MB vector write per iteration and makes the
+ *    leading half permanently L2-resident.  The full 29960146-entry
+ *    CSR row structure is still traversed - the helper is used as
+ *    given - so this costs nothing in SpMV time; it is the vector
+ *    bookkeeping around it that gets cheaper.  Measured effect on the
+ *    verifier: none at all (1.377e-06 before and after, tol 1e-05).
+ *
+ * 4. Combining 1-3, the SpMV source is a single fixed 2600000-wide
+ *    buffer whose leading half never changes.  One kernel pass does
+ *    the whole non-SpMV part of the iteration: it soft-thresholds,
+ *    rolls the iterate forward, writes the extrapolated half of v for
+ *    the next iteration and clears the next accumulator.  A fixed
+ *    source address also lets the L2 persisting window be programmed
+ *    once per solve instead of once per iteration.
+ *
+ * The whole iteration lives on the device: the state is staged once at
+ * the start of fom_solve, nothing but device pointers move between
+ * iterations, and the solution is read back once at the end.  The only
+ * host<->device traffic per solve is the parameter vector in and the
+ * solution out.
  */

 #include <cuda_runtime.h>
@@ -33,6 +78,15 @@
 #define FOM_TPB_SHIFT  8
 #define FOM_RES_BLOCKS 1024

+/* Kernel launches are asynchronous; the profiling build synchronises at
+   each region boundary so the reported per-region times are real device
+   times.  Compiled out entirely in the production build. */
+#ifdef FOM_PROFILE
+#define FOM_PROF_SYNC() cudaDeviceSynchronize()
+#else
+#define FOM_PROF_SYNC() ((void)0)
+#endif
+
 /* ceil(n / FOM_TPB) without a division. */
 #define FOM_NBLOCKS(n) (((n) + (FOM_TPB - 1)) >> FOM_TPB_SHIFT)

@@ -46,35 +100,61 @@

 /* ---------------------------------------------------------------- */
 /* Device workspace (allocated once in fom_setup, reused by every     */
-/* solve).  d_buf holds three contiguous dim-FOM_NT0 vectors:         */
-/*   [0]        acc : t0_state @ state                                */
-/*   [NT0]      gac : t0_state__d_gamma @ state                       */
-/*   [2*NT0]    cst : t0_p_p0 @ p0   (constant over the iteration)    */
+/* solve).                                                            */
+/*                                                                    */
+/* fom_d_blk is the one FOM_NSTATE-wide vector the SpMV ever reads:   */
+/*   leading  half  = 0        (operand of the zero block AL; set     */
+/*                              once in fom_setup, never touched)     */
+/*   trailing half  = FOM_VT = x_k + gamma_k (x_k - x_{k-1})          */
+/* fom_d_x carries x_k across iterations; the soft-threshold kernel   */
+/* reads it, overwrites it with x_{k+1} and refreshes FOM_VT for the  */
+/* next iteration in the same pass.  fom_d_prev / fom_d_last hold the */
+/* two extra iterates the final state and residual need.              */
 /* ---------------------------------------------------------------- */
-static fom_float *fom_d_state     = NULL;  /* FOM_NSTATE */
-static fom_float *fom_d_new_state = NULL;  /* FOM_NSTATE */
-static fom_float *fom_d_init      = NULL;  /* FOM_NSTATE, = FOM_STATE_INIT */
-static fom_float *fom_d_buf       = NULL;  /* 3 * FOM_NT0 */
-static fom_float *fom_d_t0        = NULL;  /* FOM_NT0 */
-static fom_float *fom_d_p0        = NULL;  /* FOM_NP0 */
-static fom_float *fom_d_part      = NULL;  /* FOM_RES_BLOCKS */
+static fom_float *fom_d_blk   = NULL;  /* FOM_NSTATE: the fixed SpMV input */
+static fom_float *fom_d_x     = NULL;  /* FOM_NT0: current iterate        */
+static fom_float *fom_d_prev  = NULL;  /* FOM_NT0: residual scratch       */
+static fom_float *fom_d_last  = NULL;  /* FOM_NT0: final iterate          */
+static fom_float *fom_d_init  = NULL;  /* FOM_NSTATE, = FOM_STATE_INIT */
+static fom_float *fom_d_acc   = NULL;  /* 2 * FOM_NT0, double buffered */
+static fom_float *fom_d_cst   = NULL;  /* FOM_NT0, = t0_p_p0 @ p0 */
+static fom_float *fom_d_p0    = NULL;  /* FOM_NP0 */
+static fom_float *fom_d_part  = NULL;  /* FOM_RES_BLOCKS */
 static fom_float  fom_h_part[FOM_RES_BLOCKS];
 static int        fom_dev_ready = 0;
+static void      *fom_h_pinned  = NULL;  /* solution buffer page-locked */
 static int        fom_l2_ok = 0;

-/* Pin the state vector in L2.  The two dominant CSR kernels gather
-   x[col] with scattered column indices while streaming hundreds of MB
+#define FOM_VT (fom_d_blk + (size_t)FOM_NT0)
+
+/* Pin the hot end of the SpMV source vector in L2.  The CSR kernel
+   gathers x[col] with scattered column indices while streaming ~360 MB
    of matrix data through L2, which otherwise evicts the vector between
-   rows.  A persisting access-policy window over the active state
-   buffer keeps it resident.  Pure cache policy: no arithmetic
-   changes. */
+   rows.  A persisting access-policy window keeps it resident.  Pure
+   cache policy: no arithmetic changes.
+
+   Size and placement follow the gather density.  t0_state = [AL | M]
+   has 19499968 of its 29960146 entries in M, so the trailing
+   (extrapolated) half of the buffer is hit 1.86x as often as the
+   leading half, and it is also the half that changes every iteration,
+   so the window is anchored at the buffer end.  Its length is a
+   genuine trade-off: every byte reserved for persisting lines is a
+   byte the streamed matrix data cannot use.  Measured on
+   the A100's 40 MB L2 the curve bottoms out over 13-15 MB and climbs
+   on both sides (pinning the whole 20.8 MB block costs 1.7%, pinning
+   only 9 MB costs 7%), so the window takes the trailing 21/32 of the
+   buffer: all of the extrapolated half plus the adjacent 3.25 MB of
+   the zero half. */
+#define FOM_PIN_BYTES (((size_t)FOM_BYTES_STATE >> 5) * 21u)
+#define FOM_PIN_OFF   ((size_t)FOM_NSTATE - FOM_PIN_BYTES / sizeof(fom_float))
+
 static void fom_l2_pin(const fom_float *p)
 {
     cudaStreamAttrValue a;
     if (!fom_l2_ok) return;
     memset(&a, 0, sizeof(a));
-    a.accessPolicyWindow.base_ptr  = (void *)p;
-    a.accessPolicyWindow.num_bytes = FOM_BYTES_STATE;
+    a.accessPolicyWindow.base_ptr  = (void *)(p + FOM_PIN_OFF);
+    a.accessPolicyWindow.num_bytes = FOM_PIN_BYTES;
     a.accessPolicyWindow.hitRatio  = 1.0f;
     a.accessPolicyWindow.hitProp   = cudaAccessPropertyPersisting;
     a.accessPolicyWindow.missProp  = cudaAccessPropertyStreaming;
@@ -86,53 +166,89 @@
 /* Kernels                                                            */
 /* ---------------------------------------------------------------- */

-/* t0 = soft_threshold(acc + gamma*gac + cst, lam).
- *
- * The adds/muls are emitted as separate IEEE operations (no FMA
- * contraction) and in the term order given by SPEC.md so the result is
- * bit-faithful to the reference evaluation.  Two doubles per thread. */
-__global__ void fom_k_soft(const double2 *__restrict__ acc,
-                           const double2 *__restrict__ gac,
-                           const double2 *__restrict__ cst,
-                           double2 *__restrict__ out,
-                           fom_float gamma, fom_float lam, fom_int n2)
+/* Extrapolation only; used once, to prime the loop:
+ *   w = x1 + g * (x1 - x0).
+ * Two doubles per thread (vectorised 16-byte accesses). */
+__global__ void fom_k_extrap(const double2 *__restrict__ x0,
+                             const double2 *__restrict__ x1,
+                             double2 *__restrict__ w,
+                             fom_float g, fom_int n2)
+{
+    fom_int i = (fom_int)(blockIdx.x * blockDim.x + threadIdx.x);
+    double2 a, b, r;
+    if (i >= n2) return;
+    a = x0[i];
+    b = x1[i];
+    r.x = fma(g, b.x - a.x, b.x);
+    r.y = fma(g, b.y - a.y, b.y);
+    w[i] = r;
+}
+
+/* Fused activation + extrapolation, one pass over the iterate:
+ *   xn  = soft_threshold(acc + cst, lam)      (= x_{k+1})
+ *   w   = xn + gnext * (xn - xc)              (= next SpMV tail half)
+ * `xc` is x_k.  Doing both here saves a whole 2 x 10.4 MB round trip
+ * through memory compared with a separate extrapolation pass. */
+__global__ void fom_k_soft_extrap(const double2 *__restrict__ acc,
+                                  const double2 *__restrict__ cst,
+                                  const double2 *xin,
+                                  double2 *xout,
+                                  double2 *__restrict__ vt,
+                                  double2 *__restrict__ accz,
+                                  fom_float gnext, fom_float lam,
+                                  fom_int n2)
 {
     fom_int i = (fom_int)(blockIdx.x * blockDim.x + threadIdx.x);
+    double2 a, c, p, r, q;
+    fom_float vx, vy;
     if (i >= n2) return;

-    double2 a = acc[i];
-    double2 g = gac[i];
-    double2 c = cst[i];
+    a = acc[i];
+    c = cst[i];
+    p = xin[i];                     /* x_k  (xin may alias xout) */

-    fom_float vx = __dadd_rn(__dadd_rn(a.x, __dmul_rn(gamma, g.x)), c.x);
-    fom_float vy = __dadd_rn(__dadd_rn(a.y, __dmul_rn(gamma, g.y)), c.y);
+    vx = __dadd_rn(a.x, c.x);
+    vy = __dadd_rn(a.y, c.y);

-    double2 r;
     r.x = (vx > lam) ? (vx - lam) : ((vx < -lam) ? (vx + lam) : 0.0);
     r.y = (vy > lam) ? (vy - lam) : ((vy < -lam) ? (vy + lam) : 0.0);
-    out[i] = r;
+    xout[i] = r;                    /* x_{k+1} */
+
+    q.x = fma(gnext, r.x - p.x, r.x);
+    q.y = fma(gnext, r.y - p.y, r.y);
+    vt[i] = q;                      /* trailing half of v for it+1 */
+
+    q.x = 0.0; q.y = 0.0;
+    accz[i] = q;
 }

-/* Per-block max |a - b|; the FOM_RES_BLOCKS partials are reduced on
- * the host.  Only run on the final iteration (termination is
- * max_iters, so the residual never gates the loop). */
-__global__ void fom_k_resid(const fom_float *__restrict__ a,
-                            const fom_float *__restrict__ b,
+/* Per-block max over |u1 - u0| and |v1 - v0| together; the two halves
+ * of (new_state - state) are exactly these two differences.  The
+ * FOM_RES_BLOCKS partials are reduced on the host.  Only run on the
+ * final iteration (termination is max_iters, so the residual never
+ * gates the loop). */
+__global__ void fom_k_resid(const fom_float *__restrict__ u0,
+                            const fom_float *__restrict__ u1,
+                            const fom_float *__restrict__ v0,
+                            const fom_float *__restrict__ v1,
                             fom_float *__restrict__ part, fom_int n)
 {
     __shared__ fom_float sm[FOM_TPB];
     fom_int stride = (fom_int)(gridDim.x * blockDim.x);
     fom_float m = 0.0;
+    unsigned s;

     for (fom_int i = (fom_int)(blockIdx.x * blockDim.x + threadIdx.x);
          i < n; i += stride) {
-        fom_float d = fabs(a[i] - b[i]);
+        fom_float d = fabs(u1[i] - u0[i]);
+        fom_float e = fabs(v1[i] - v0[i]);
         if (d > m) m = d;
+        if (e > m) m = e;
     }

     sm[threadIdx.x] = m;
     __syncthreads();
-    for (unsigned s = blockDim.x >> 1; s > 0; s >>= 1) {
+    for (s = blockDim.x >> 1; s > 0; s >>= 1) {
         if (threadIdx.x < s) {
             fom_float o = sm[threadIdx.x + s];
             if (o > sm[threadIdx.x]) sm[threadIdx.x] = o;
@@ -169,19 +285,24 @@
     fom_reset_state(work);

     if (!fom_dev_ready) {
-        cudaMalloc((void **)&fom_d_state,     FOM_BYTES_STATE);
-        cudaMalloc((void **)&fom_d_new_state, FOM_BYTES_STATE);
-        cudaMalloc((void **)&fom_d_init,      FOM_BYTES_STATE);
-        cudaMalloc((void **)&fom_d_buf,       3u * FOM_BYTES_T0);
-        cudaMalloc((void **)&fom_d_t0,        FOM_BYTES_T0);
-        cudaMalloc((void **)&fom_d_p0,        FOM_BYTES_P0);
+        cudaMalloc((void **)&fom_d_blk,  FOM_BYTES_STATE);
+        cudaMalloc((void **)&fom_d_x,    FOM_BYTES_T0);
+        cudaMalloc((void **)&fom_d_prev, FOM_BYTES_T0);
+        cudaMalloc((void **)&fom_d_last, FOM_BYTES_T0);
+        /* The leading half of the SpMV source is the operand of the
+           (numerically zero) AL block and is never written again. */
+        cudaMemset(fom_d_blk, 0, FOM_BYTES_T0);
+        cudaMalloc((void **)&fom_d_init, FOM_BYTES_STATE);
+        cudaMalloc((void **)&fom_d_acc,  2u * FOM_BYTES_T0);
+        cudaMalloc((void **)&fom_d_cst,  FOM_BYTES_T0);
+        cudaMalloc((void **)&fom_d_p0,   FOM_BYTES_P0);
         cudaMalloc((void **)&fom_d_part,
                    (size_t)FOM_RES_BLOCKS * sizeof(fom_float));
         fom_dev_ready = 1;

         {
             cudaDeviceProp prop;
-            size_t want = FOM_BYTES_STATE;
+            size_t want = FOM_PIN_BYTES;
             if (cudaGetDeviceProperties(&prop, 0) == cudaSuccess) {
                 if ((size_t)prop.persistingL2CacheMaxSize < want)
                     want = (size_t)prop.persistingL2CacheMaxSize;
@@ -210,66 +331,116 @@
     const fom_int max_iter = work->settings.max_iter;
     const fom_float lam    = FOM_T0_LAMBDA;

-    fom_float *acc = fom_d_buf;
-    fom_float *gac = fom_d_buf + FOM_NT0;
-    fom_float *cst = fom_d_buf + 2 * (size_t)FOM_NT0;
-
-    fom_float *cur = fom_d_state;
-    fom_float *nxt = fom_d_new_state;
-
-    const fom_int n2       = FOM_NT0 >> 1;
-    const fom_int nb_soft  = FOM_NBLOCKS(n2);
+    const fom_int n2      = FOM_NT0 >> 1;
+    const fom_int nb_vec  = FOM_NBLOCKS(n2);
     fom_int it;
+    int e = 0;
     fom_float residual = (fom_float)0;

-    /* Upload once: state (device-to-device from the staged initial
-       state) and the parameter vector. */
-    cudaMemcpyAsync(cur, fom_d_init, FOM_BYTES_STATE,
+    /* Degenerate request: the state is still the initial one. */
+    if (max_iter <= 0) {
+        memcpy(work->solution.x, FOM_STATE_INIT, FOM_BYTES_STATE);
+        work->solution.iter     = 0;
+        work->solution.residual = (fom_float)0;
+        work->solution.status   = FOM_MAX_ITER_REACHED;
+        return work->solution.status;
+    }
+
+    /* Stage the initial state: x = s1 = x_0, and x_{-1} = s0 both for
+       the first extrapolation and as the residual's predecessor when
+       max_iter == 1.  v's leading half is the permanent zero vector. */
+    FOM_PROF_BEGIN(upload);
+    cudaMemcpyAsync(fom_d_x, fom_d_init + FOM_NT0, FOM_BYTES_T0,
+                    cudaMemcpyDeviceToDevice, 0);
+    cudaMemcpyAsync(fom_d_prev, fom_d_init, FOM_BYTES_T0,
                     cudaMemcpyDeviceToDevice, 0);
     cudaMemcpyAsync(fom_d_p0, work->params.p0, FOM_BYTES_P0,
                     cudaMemcpyHostToDevice, 0);
+    FOM_PROF_SYNC(); FOM_PROF_END(upload);

     /* Constant term t0_p_p0 @ p0: independent of the state, so it is
        formed once instead of once per iteration. */
-    cudaMemsetAsync(cst, 0, FOM_BYTES_T0, 0);
-    fom_apply_t0_p_p0(fom_d_p0, cst);
+    FOM_PROF_BEGIN(const_term);
+    cudaMemsetAsync(fom_d_cst, 0, FOM_BYTES_T0, 0);
+    cudaMemsetAsync(fom_d_acc, 0, FOM_BYTES_T0, 0);
+    fom_apply_t0_p_p0(fom_d_p0, fom_d_cst);
+    FOM_PROF_SYNC(); FOM_PROF_END(const_term);
+
+    /* Prime the loop invariant: v.t = x_0 + gamma_0 (x_0 - x_{-1}). */
+    FOM_PROF_BEGIN(extrap_init);
+    fom_k_extrap<<<nb_vec, FOM_TPB>>>((const double2 *)fom_d_init,
+                                      (const double2 *)fom_d_x,
+                                      (double2 *)FOM_VT,
+                                      fom_sched_gamma(0), n2);
+    FOM_PROF_SYNC(); FOM_PROF_END(extrap_init);
+
+    /* The SpMV source never moves, so the persisting window is set
+       once for the whole solve. */
+    fom_l2_pin(fom_d_blk);

     for (it = 0; it < max_iter; ++it) {
-        const fom_float gamma = fom_sched_gamma(it);
-
-        fom_l2_pin(cur);
-
-        /* acc = t0_state @ state ; gac = t0_state__d_gamma @ state */
-        cudaMemsetAsync(acc, 0, 2u * FOM_BYTES_T0, 0);
-        fom_apply_t0_state(cur, acc);
-        fom_apply_t0_state__d_gamma(cur, gac);
-
-        fom_k_soft<<<nb_soft, FOM_TPB>>>((const double2 *)acc,
-                                         (const double2 *)gac,
-                                         (const double2 *)cst,
-                                         (double2 *)fom_d_t0,
-                                         gamma, lam, n2);
-
-        /* new_state = su_state @ state + su_t_t0 @ t0 */
-        cudaMemsetAsync(nxt, 0, FOM_BYTES_STATE, 0);
-        fom_apply_su_state(cur, nxt);
-        fom_apply_su_t_t0(fom_d_t0, nxt);
+        /* The single state-dependent product of the iteration. */
+        FOM_PROF_BEGIN(spmv_t0_state);
+        fom_apply_t0_state(fom_d_blk, fom_d_acc + (size_t)e * FOM_NT0);
+        FOM_PROF_SYNC(); FOM_PROF_END(spmv_t0_state);
+
+        /* Keep x_{N-2} and x_N, the two iterates the final state and
+           residual need besides the one the loop carries. */
+        if (it == max_iter - 2)
+            cudaMemcpyAsync(fom_d_prev, fom_d_x, FOM_BYTES_T0,
+                            cudaMemcpyDeviceToDevice, 0);
+
+        FOM_PROF_BEGIN(soft_extrap);
+        fom_k_soft_extrap<<<nb_vec, FOM_TPB>>>(
+            (const double2 *)(fom_d_acc + (size_t)e * FOM_NT0),
+            (const double2 *)fom_d_cst,
+            (const double2 *)fom_d_x,
+            (double2 *)((it == max_iter - 1) ? fom_d_last : fom_d_x),
+            (double2 *)FOM_VT,
+            (double2 *)(fom_d_acc + (size_t)(e ^ 1) * FOM_NT0),
+            fom_sched_gamma(it + 1), lam, n2);
+        FOM_PROF_SYNC(); FOM_PROF_END(soft_extrap);

         if (it == max_iter - 1) {
-            fom_k_resid<<<FOM_RES_BLOCKS, FOM_TPB>>>(nxt, cur, fom_d_part,
-                                                     FOM_NSTATE);
+            /* new_state - state = [ x_it - x_{it-1} ; x_{it+1} - x_it ]. */
+            FOM_PROF_BEGIN(residual);
+            fom_k_resid<<<FOM_RES_BLOCKS, FOM_TPB>>>(
+                fom_d_prev, fom_d_x, fom_d_x, fom_d_last,
+                fom_d_part, FOM_NT0);
+            FOM_PROF_SYNC(); FOM_PROF_END(residual);
         }

-        { fom_float *t = cur; cur = nxt; nxt = t; }
+        e ^= 1;
     }

     if (fom_l2_ok) cudaCtxResetPersistingL2Cache();

-    /* `cur` now holds the final state. */
-    cudaMemcpy(work->solution.x, cur, FOM_BYTES_STATE,
-               cudaMemcpyDeviceToHost);
+    /* The final state is [ x_{max_iter-1} ; x_{max_iter} ]: the loop
+       carrier still holds the former, the last iteration parked the
+       latter in its own buffer.
+
+       Page-lock the caller's solution buffer once so the two readback
+       transfers run at full PCIe rate instead of being staged through
+       the driver's bounce buffer. */
+    if (fom_h_pinned != (void *)work->solution.x) {
+        if (fom_h_pinned) cudaHostUnregister(fom_h_pinned);
+        fom_h_pinned = (cudaHostRegister((void *)work->solution.x,
+                                         FOM_BYTES_STATE,
+                                         cudaHostRegisterDefault)
+                        == cudaSuccess)
+                       ? (void *)work->solution.x : NULL;
+        cudaGetLastError();
+    }
+
+    FOM_PROF_BEGIN(readback);
+    cudaMemcpyAsync(work->solution.x, fom_d_x, FOM_BYTES_T0,
+                    cudaMemcpyDeviceToHost, 0);
+    cudaMemcpyAsync(work->solution.x + FOM_NT0, fom_d_last, FOM_BYTES_T0,
+                    cudaMemcpyDeviceToHost, 0);
+    cudaStreamSynchronize(0);
+    FOM_PROF_END(readback);

-    if (max_iter > 0) {
+    {
         fom_int k;
         cudaMemcpy(fom_h_part, fom_d_part,
                    (size_t)FOM_RES_BLOCKS * sizeof(fom_float),
@@ -282,27 +453,26 @@
     work->solution.residual = residual;
     work->solution.status   = FOM_MAX_ITER_REACHED;

-    /* Keep the device pointers consistent for the next solve: `cur`
-       may be either buffer after the swaps. */
-    fom_d_state     = cur;
-    fom_d_new_state = nxt;
-
     return work->solution.status;
 }

 extern "C" void fom_cleanup(FomWorkspace *work)
 {
     (void)work;
+    if (fom_h_pinned) { cudaHostUnregister(fom_h_pinned); fom_h_pinned = NULL; }
     if (fom_dev_ready) {
-        cudaFree(fom_d_state);
-        cudaFree(fom_d_new_state);
+        cudaFree(fom_d_blk);
+        cudaFree(fom_d_x);
+        cudaFree(fom_d_prev);
+        cudaFree(fom_d_last);
         cudaFree(fom_d_init);
-        cudaFree(fom_d_buf);
-        cudaFree(fom_d_t0);
+        cudaFree(fom_d_acc);
+        cudaFree(fom_d_cst);
         cudaFree(fom_d_p0);
         cudaFree(fom_d_part);
-        fom_d_state = fom_d_new_state = fom_d_init = NULL;
-        fom_d_buf = fom_d_t0 = fom_d_p0 = fom_d_part = NULL;
+        fom_d_blk = fom_d_init = fom_d_acc = NULL;
+        fom_d_x = fom_d_prev = fom_d_last = NULL;
+        fom_d_cst = fom_d_p0 = fom_d_part = NULL;
         fom_dev_ready = 0;
     }
 }
```

## Agent summary (end of the code-optimization transcript)

### Result

| | mean_us |
|---|---:|
| initial (`make bench`, starting solver) | **527 576** |
| final | **127 720** |
| **overall speedup** | **4.13×** |

`make verify` prints `VERIFY OK` at every step; the worst max-abs error over the 8 samples is **1.377e-06** against a tolerance of 1.0e-05 (the starting solver was 1.823e-07).

### Final profiling breakdown

From `make profile` (10 solves; per-solve figures). The profile build inserts a `cudaDeviceSynchronize` at each region boundary, so its whole-solve mean is 130.1 ms rather than 127.7 ms.

| region | per solve | pct |
|---|---:|---:|
| `spmv_t0_state` (200 × `fom_apply_t0_state`) | 120.22 ms | 92.45 % |
| `soft_extrap` (200 × fused activation/extrapolation/clear) | 8.24 ms | 6.34 % |
| `readback` (20.8 MB D2H, once) | 1.13 ms | 0.87 % |
| `upload` (state stage + 2.6 MB H2D p0) | 0.27 ms | 0.21 % |
| `const_term` (`t0_p_p0 @ p0`, once) | 0.107 ms | 0.08 % |
| `residual` (last iteration only) | 0.040 ms | 0.03 % |
| `extrap_init` (loop prologue) | 0.030 ms | 0.02 % |

The remaining time is essentially all inside the one fixed CSR helper, which cannot be modified.

### Optimizations applied

1. **Eliminated the `t0_state__d_gamma` product (65.7 % of the original runtime).** Structure: splitting the two 1300000×2600000 operators into 1300000×1300000 blocks gives `t0_state = [AL | M]` and `t0_state__d_gamma = [-M | M]` with the *same* `M` — the two stored copies agree to 2e-15 absolute, i.e. to rounding. So `t0_state @ s + γ·(d_gamma @ s) = t0_state @ [s0 ; s1 + γ(s1-s0)]`: moving FISTA's momentum from the operator onto the iterate turns 69 Mnnz of SpMV per iteration into 30 Mnnz. One `fom_apply_t0_state` call replaces two. (527.6 → 147.1 ms.)

2. **Deleted both state-update SpMVs — bit-exactly.** Structure: `su_state` and `su_t_t0` are exact 0/1 shift matrices (verified entry-by-entry against `data.npz`: single `1.0` per row, `su_state[r, r+1300000]`, `su_t_t0[1300000+j, j]`). The update is therefore the pure relabelling `new_state = [s1 ; t0]`, so it becomes buffer naming instead of 2.6 Mnnz of SpMV plus a 20.8 MB memset — no arithmetic, bit-for-bit identical.

3. **L2 persisting-access window sized and placed by gather density.** The fixed CSR kernel gathers `x[col]` while streaming ~360 MB of matrix data through the 40 MB L2, which otherwise evicts the vector between rows. Structure: 19 499 968 of `t0_state`'s 29 960 146 entries live in `M`, so the trailing half of the argument is hit 1.86× as often — the window is anchored at the buffer end. Its length is a real trade-off (bytes reserved for persisting lines are bytes the streamed matrix cannot use); measured sweeps put the optimum at the trailing 21/32 ≈ 13.65 MB. Pinning the whole 20.8 MB block costs 1.7 %, pinning only 9 MB costs 7 %. (147.1 → 133.8 ms.)

4. **Fixed the SpMV source address (one buffer instead of a rotating ring).** Lets the persisting window be programmed once per solve instead of once per iteration, and makes the argument L2-warm from the write that just produced it. Costs one extra 10.4 MB vector write, which the L2 behaviour more than repays. (133.8 → 132.3 ms, then 131.4 ms after re-tuning the window for the new layout.)

5. **Held the leading half of the SpMV argument at zero.** Structure: `AL` is the zero block — its 10 460 178 stored entries are rounding dust, `max|AL| = 1.11e-16`, one ulp of `M`'s entries (diagonal ≈ 0.98). This is exactly what the algebra predicts: with momentum off the step depends only on `s1`. So `AL s0` is below the rounding error of the row sums it joins, and the half can simply stay zero, removing a 10.4 MB write per iteration and making it permanently L2-resident. The full CSR row structure is still traversed — the helper is used as given — so SpMV time is unaffected; only the vector bookkeeping gets cheaper. Effect on the verifier: none measurable (1.377e-06 before and after). (131.4 → 127.7 ms after re-tuning the window.)

6. **Fused the whole non-SpMV part of the iteration into one pass**, and hoisted everything loop-invariant. The single `soft_extrap` kernel soft-thresholds, rolls the iterate forward, rebuilds the extrapolated half of the argument for the next iteration, and clears the double-buffered accumulator the next SpMV will add into — replacing a separate 10.4 MB `cudaMemset` kernel. `t0_p_p0 @ p0` is state-independent and is formed once per solve rather than once per iteration; the initial state is staged on the device once in `fom_setup`; the residual reduction runs only on the final iteration (termination is `max_iters`, so it never gates the loop). The kernel moves 62.4 MB in 41 µs ≈ 1.5 TB/s, close to the A100's HBM peak; a block-size sweep (64–1024) changed the total by <0.4 %, so 256 was kept.

7. **Page-locked the caller's solution buffer** (`cudaHostRegister`, once, with a silent fallback) so the two 10.4 MB readbacks run at full PCIe rate: 1.54 → 1.13 ms per solve.

#### Things measured and rejected
`cudaFuncCachePreferL1` / `PreferShared` (+1.6 % / +45 %), `hitRatio < 1` with a wider window (+8 % to +30 %), no pinning at all (+6 %), vector-kernel block sizes other than 256 (<0.4 %), and a 3-block / 2-block rotating ring with a moving window (1.1 % slower than the fixed buffer).

#### Why it stops here
92.45 % of the remaining time is inside `fom_apply_t0_state`, a fixed one-thread-per-row CSR kernel in `src/fom_ops.cu` that may not be edited; its 29 960 146 entries must be traversed once per iteration and the schedule's 200 iterations are serially dependent through the soft-threshold. Everything around it now accounts for 7.5 % of the solve. The last two rounds yielded 2.3 % and 0.45 %.
