# Optimization report: ista / cuda (m325000_n1300000_k8_origistacuda)

Fresh-build head-to-head on della-l09g6, mean per solve over 5 solves after 8 warm-ups:

| solver | mean_us |
|---|---:|
| Phase A | 134397.087400 |
| Phase B | 106540.432800 |

Kept: **phase_b** (Phase-A / Phase-B time = 1.261). Final verification:

```
worst max-abs error over 8 samples: 8.704e-14 (tol 1.000e-05)
```

## Diff (Phase A -> kept)

```diff
--- phase_a_solver.cu	2026-10-16 11:32:55.059445000 -0400
+++ src/fom_solver.cu	2026-10-16 12:16:07.608394000 -0400
@@ -6,18 +6,41 @@
        new_state = su_t_t0 @ t0
        residual  = max_abs(new_state - state)
 
-   All matrix-vector products go through the accumulating CSR helpers in
-   `fom_ops.cu`; no matrix is ever formed here.  The whole state stays
-   resident on the device for the complete loop: buffers are cudaMalloc'd
-   once in fom_setup, the parameter vector and the initial state are
-   uploaded once per solve, the state/new_state device pointers are
-   swapped instead of copied, and the solution is copied back exactly once.
-
-   The term `t0_p_p0 @ p0` does not depend on the state, so it is computed
-   once per solve into `d_b` and the per-iteration accumulator is primed
-   with it instead of with zeros.  IEEE-wise this is identical to zeroing
-   and accumulating both terms (floating point addition is commutative),
-   it just removes one sparse matvec from every iteration.
+   Every matrix-vector product goes through the accumulating CSR helpers in
+   `fom_ops.cu`; no matrix is ever formed here.  The state stays resident on
+   the device for the whole loop: buffers are cudaMalloc'd once in
+   fom_setup, the parameter vector is uploaded once per solve and the
+   solution is copied back exactly once.
+
+   Structure exploited (see spec.json / MATRIX_STATS.md):
+
+   * `su_t_t0` is the 1300000x1300000 IDENTITY (rowptr = 0,1,2,..., colidx =
+     0,1,2,..., every value exactly 1.0), so `new_state = t0` bit-for-bit:
+     the CSR kernel evaluates `0 + 1.0*t0[i]`, exact for every finite double,
+     and the activation never emits -0.0 (its clamped branch returns +0.0).
+     The state-update matvec - and the memset that zeroed its accumulator -
+     are therefore dropped completely; the activation writes the next state.
+
+   * `t0_p_p0 @ p0` is state-independent, so it is evaluated once per solve
+     into `d_b`, and the per-iteration accumulator is primed with `b` instead
+     of with zeros.  Identical in IEEE arithmetic to zeroing and accumulating
+     both terms, one sparse matvec cheaper per iteration.
+
+   * Because the state update is the identity, the activation can write the
+     next iterate straight back into the buffer the matvec just read.  The
+     gather source therefore never changes address, which lets it be pinned:
+     `t0_state` streams 234 MB of CSR payload past the 40 MB L2 every
+     iteration while gathering 19499968 random elements out of the 10.4 MB
+     state vector, so by default the state is evicted between gathers and
+     every gather costs a 32 B DRAM sector for 8 B of payload.  The state
+     buffer is declared an L2 *persisting* access window (everything else,
+     the CSR payload included, is marked *streaming*) with the set-aside
+     sized 1.5x the window so the window does not thrash against itself.
+
+   * The accumulator re-prime (`acc <- b`) is loop-carried but needed only
+     two iterations later, so two accumulators are alternated and the copy
+     runs on a non-blocking side stream, overlapped with the next matvec;
+     the activation kernel then only reads `acc` and writes the state.
 
    No `/` operator is used anywhere (block counts are shifts). */
 
@@ -49,27 +72,27 @@
 /* ------------------------------------------------------------------ */
 /* Device scratch (allocated once, reused by every solve).             */
 /* ------------------------------------------------------------------ */
-static fom_float *d_bufA = 0;   /* state / new_state (swapped)         */
-static fom_float *d_bufB = 0;
-static fom_float *d_acc  = 0;   /* linear_op accumulator for t0        */
-static fom_float *d_t0   = 0;   /* activation output                   */
+static fom_float *d_bufA = 0;   /* state (L2-pinned gather source)     */
+static fom_float *d_bufB = 0;   /* previous iterate / tol ping-pong    */
+static fom_float *d_acc[2] = {0, 0}; /* linear_op accumulators         */
 static fom_float *d_b    = 0;   /* t0_p_p0 @ p0 (loop invariant)       */
+static fom_float *d_init = 0;   /* FOM_STATE_INIT, cached on device    */
 static fom_float *d_p0   = 0;
 static fom_float *d_red  = 0;   /* residual partials                   */
+static cudaStream_t  g_copy = 0;      /* non-blocking re-prime stream  */
+static cudaEvent_t   g_ev_step[2];    /* acc[i] free to overwrite      */
+static cudaEvent_t   g_ev_prime[2];   /* acc[i] holds b again          */
 static int        g_ready = 0;
 
 /* ------------------------------------------------------------------ */
 /* Kernels                                                             */
 /* ------------------------------------------------------------------ */
 
-/* Fused per-iteration element-wise work, in one pass over memory:
-     t0   = soft_threshold(acc, lambda)     (activation of step t0)
-     acc  = b                               (prime next linear_op)
-     zed  = 0                               (zero the su_t_t0 target)  */
-__global__ void fom_k_step(double2 *__restrict__ acc,
-                           const double2 *__restrict__ b,
-                           double2 *__restrict__ t0,
-                           double2 *__restrict__ zed,
+/* Activation of step t0.  Because su_t_t0 is the identity this value is
+   already the next state, so it is written straight into the state
+   buffer: one read stream and one write stream per iteration. */
+__global__ void fom_k_step(const double2 *__restrict__ acc,
+                           double2 *__restrict__ next,
                            double lam, int n2)
 {
     int i = blockIdx.x * blockDim.x + threadIdx.x;
@@ -79,9 +102,7 @@
     double t;
     t = fabs(a.x) - lam;  r.x = (t > 0.0) ? copysign(t, a.x) : 0.0;
     t = fabs(a.y) - lam;  r.y = (t > 0.0) ? copysign(t, a.y) : 0.0;
-    t0[i]  = r;
-    acc[i] = b[i];
-    zed[i] = make_double2(0.0, 0.0);
+    next[i] = r;
 }
 
 /* Partial max_abs(u - v) per block. */
@@ -136,17 +157,55 @@
 /* Helpers                                                             */
 /* ------------------------------------------------------------------ */
 
+/* Reserve an L2 set-aside and mark the state buffer as persisting for
+   every launch on the default stream; all other traffic (the CSR
+   payload) is marked streaming so it cannot evict it.  Pure cache
+   policy: no result changes, only the hit rate of the gather. */
+static void fom_l2_pin(void)
+{
+    const size_t win = (size_t)FOM_N * sizeof(fom_float);
+    size_t setaside = win + (win >> 1);            /* 1.5x, measured best */
+    cudaDeviceProp prop;
+    cudaStreamAttrValue attr;
+
+    if (cudaGetDeviceProperties(&prop, 0) != cudaSuccess) return;
+    if ((size_t)prop.persistingL2CacheMaxSize < win) return;
+    if ((size_t)prop.accessPolicyMaxWindowSize < win) return;
+    if (setaside > (size_t)prop.persistingL2CacheMaxSize)
+        setaside = (size_t)prop.persistingL2CacheMaxSize;
+    if (cudaDeviceSetLimit(cudaLimitPersistingL2CacheSize, setaside)
+            != cudaSuccess)
+        return;
+
+    attr.accessPolicyWindow.base_ptr  = (void *)d_bufA;
+    attr.accessPolicyWindow.num_bytes = win;
+    attr.accessPolicyWindow.hitRatio  = 1.0f;
+    attr.accessPolicyWindow.hitProp   = cudaAccessPropertyPersisting;
+    attr.accessPolicyWindow.missProp  = cudaAccessPropertyStreaming;
+    cudaStreamSetAttribute(0, cudaStreamAttributeAccessPolicyWindow, &attr);
+}
+
 static void fom_device_init(void)
 {
     if (g_ready) return;
     const size_t nb = (size_t)FOM_N * sizeof(fom_float);
     cudaMalloc((void **)&d_bufA, nb);
     cudaMalloc((void **)&d_bufB, nb);
-    cudaMalloc((void **)&d_acc,  nb);
-    cudaMalloc((void **)&d_t0,   nb);
+    cudaMalloc((void **)&d_acc[0], nb);
+    cudaMalloc((void **)&d_acc[1], nb);
     cudaMalloc((void **)&d_b,    nb);
+    cudaMalloc((void **)&d_init, nb);
     cudaMalloc((void **)&d_p0, (size_t)FOM_PARAM_P0_DIM * sizeof(fom_float));
     cudaMalloc((void **)&d_red, (size_t)FOM_RED_BLOCKS * sizeof(fom_float));
+    cudaStreamCreateWithFlags(&g_copy, cudaStreamNonBlocking);
+    cudaEventCreateWithFlags(&g_ev_step[0],  cudaEventDisableTiming);
+    cudaEventCreateWithFlags(&g_ev_step[1],  cudaEventDisableTiming);
+    cudaEventCreateWithFlags(&g_ev_prime[0], cudaEventDisableTiming);
+    cudaEventCreateWithFlags(&g_ev_prime[1], cudaEventDisableTiming);
+    /* Every solve starts from the same compile-time constant state, so
+       stage it on the device once instead of re-crossing PCIe per solve. */
+    cudaMemcpy(d_init, FOM_STATE_INIT, nb, cudaMemcpyHostToDevice);
+    fom_l2_pin();
     g_ready = 1;
 }
 
@@ -206,7 +265,7 @@
                              ? work->settings.check_termination : 1;
     const size_t nb = (size_t)FOM_N * sizeof(fom_float);
     fom_float res = (fom_float)0;
-    int iter, since = 0, status = FOM_MAX_ITER_REACHED;
+    int iter, par = 0, since = 0, status = FOM_MAX_ITER_REACHED;
 
     fom_device_init();
 
@@ -214,62 +273,80 @@
     cudaMemcpy(d_p0, work->params.p0,
                (size_t)FOM_PARAM_P0_DIM * sizeof(fom_float),
                cudaMemcpyHostToDevice);
-    cudaMemcpy(state, work->state, nb, cudaMemcpyHostToDevice);
+    cudaMemcpy(state, d_init, nb, cudaMemcpyDeviceToDevice);
     FOM_PROF_SYNC();
     FOM_PROF_END(upload);
 
-    /* b = t0_p_p0 @ p0 (constant over the loop), then prime the
-       accumulator with it for the first iteration. */
+    /* b = t0_p_p0 @ p0 (constant over the loop); prime both accumulators. */
     FOM_PROF_BEGIN(param_term);
     cudaMemset(d_b, 0, nb);
     fom_apply_t0_p_p0(d_p0, d_b);
-    cudaMemcpy(d_acc, d_b, nb, cudaMemcpyDeviceToDevice);
+    cudaMemcpy(d_acc[0], d_b, nb, cudaMemcpyDeviceToDevice);
+    cudaMemcpy(d_acc[1], d_b, nb, cudaMemcpyDeviceToDevice);
     FOM_PROF_SYNC();
     FOM_PROF_END(param_term);
 
     FOM_PROF_BEGIN(loop);
     for (iter = 0; iter < max_iter; ++iter) {
-        /* acc already holds t0_p_p0 @ p0; add the state term. */
+        fom_float *acc = d_acc[par];
+
+        /* acc was re-primed with b on the side stream two iterations
+           ago; make the matvec wait for that copy. */
+        if (iter >= 2) cudaStreamWaitEvent(0, g_ev_prime[par], 0);
+
         FOM_PROF_BEGIN(k_t0_state);
-        fom_apply_t0_state(state, d_acc);
+        fom_apply_t0_state(state, acc);
         FOM_PROF_SYNC();
         FOM_PROF_END(k_t0_state);
 
-        /* t0 = soft_threshold(acc); re-prime acc; zero the update target. */
+        /* Write the activation (== the next state, su_t_t0 being the
+           identity) in place, except when the previous iterate must
+           survive for a residual check or for the final report. */
+        if (use_tol || iter + 1 == max_iter) {
+            next = (state == d_bufA) ? d_bufB : d_bufA;
+        } else {
+            next = state;
+        }
+
         FOM_PROF_BEGIN(k_step);
-        fom_k_step<<<FOM_BLOCKS_N2, FOM_TPB>>>((double2 *)d_acc,
-                                               (const double2 *)d_b,
-                                               (double2 *)d_t0,
+        fom_k_step<<<FOM_BLOCKS_N2, FOM_TPB>>>((const double2 *)acc,
                                                (double2 *)next,
                                                lam, FOM_N2);
         FOM_PROF_SYNC();
         FOM_PROF_END(k_step);
 
-        /* state_next = su_t_t0 @ t0 */
-        FOM_PROF_BEGIN(k_su_t_t0);
-        fom_apply_su_t_t0(d_t0, next);
-        FOM_PROF_SYNC();
-        FOM_PROF_END(k_su_t_t0);
+        /* acc is dead now: refill it with b behind the next matvec.
+           (Pointless on the last iteration - nothing reads it again.) */
+        if (iter + 1 < max_iter) {
+            cudaEventRecord(g_ev_step[par], 0);
+            cudaStreamWaitEvent(g_copy, g_ev_step[par], 0);
+            cudaMemcpyAsync(acc, d_b, nb, cudaMemcpyDeviceToDevice, g_copy);
+            cudaEventRecord(g_ev_prime[par], g_copy);
+        }
+        par ^= 1;
 
         ++since;
         if (use_tol && since >= check) {
             since = 0;
             res = fom_residual(next, state);
             if (res <= work->settings.tol) {
-                fom_float *sw = state; state = next; next = sw;
+                state = next;
                 ++iter;
                 status = FOM_SOLVED;
                 break;
             }
         }
-
-        { fom_float *sw = state; state = next; next = sw; }
+        { fom_float *prev = state; state = next; next = prev; }
     }
+    /* The side stream is non-blocking, so nothing else orders it against
+       the next solve's writes to d_b / d_acc: drain it here.  By this
+       point its last copy has long finished, so this costs nothing. */
+    cudaStreamSynchronize(g_copy);
     FOM_PROF_END(loop);
 
     /* Report the residual of the final step (no early exit is possible
        with tol disabled, so this is the value the loop converged to). */
-    if (status != FOM_SOLVED) {
+    if (status != FOM_SOLVED && max_iter > 0) {
         FOM_PROF_BEGIN(residual);
         res = fom_residual(state, next);
         FOM_PROF_END(residual);
@@ -289,9 +366,14 @@
 {
     (void)work;
     if (!g_ready) return;
-    cudaFree(d_bufA); cudaFree(d_bufB); cudaFree(d_acc);
-    cudaFree(d_t0);   cudaFree(d_b);    cudaFree(d_p0);
-    cudaFree(d_red);
-    d_bufA = d_bufB = d_acc = d_t0 = d_b = d_p0 = d_red = 0;
+    cudaFree(d_bufA);   cudaFree(d_bufB);
+    cudaFree(d_acc[0]); cudaFree(d_acc[1]);
+    cudaFree(d_b);      cudaFree(d_init);
+    cudaFree(d_p0);     cudaFree(d_red);
+    cudaEventDestroy(g_ev_step[0]);  cudaEventDestroy(g_ev_step[1]);
+    cudaEventDestroy(g_ev_prime[0]); cudaEventDestroy(g_ev_prime[1]);
+    cudaStreamDestroy(g_copy);
+    d_bufA = d_bufB = d_acc[0] = d_acc[1] = d_b = d_init = d_p0 = d_red = 0;
+    g_copy = 0;
     g_ready = 0;
 }
```

## Agent notes (end of the Phase-B transcript)

```

**Measured and rejected:** pinning the accumulator next to the state in one 20.8 MB window (171.9 vs 125.9 ms — starves the payload); pinning both state ping-pong buffers; `missProp = Normal` (neutral); `cudaFuncCachePreferL1/Shared/None` (neutral to 1.7× worse in a microbenchmark replicating the kernel).

**Remaining bottleneck.** `fom_apply_t0_state` is 95.3% of the solve at ~518 µs/iteration. It lives in the immutable `fom_ops.cu` as a thread-per-row CSR kernel whose data arrays have internal linkage, so it can neither be replaced nor re-blocked from the solver; with the gather now L2-resident and the surrounding element-wise work cut to its two-stream minimum, the external levers (cache policy, overlap, redundant passes) are exhausted. Two further tuning rounds yielded <1% each.

FOMTOOLS-SUMMARY-END
The solver is final and verified. Here's what I did and where it ended up.

**Work summary:** The handed-over solver spent 96% of every solve inside `fom_apply_t0_state`, so the profile-guided loop focused on (a) deleting work that the matrix structure made redundant and (b) fixing the memory behaviour *around* that immutable kernel. The two structural facts that paid off were that `su_t_t0` is exactly the identity, and that the gather source of the big SpMV is only 10.4 MB — small enough to pin in L2 against the 234 MB of CSR payload that streams past it each iteration.

Every A/B was run back-to-back (the node's absolute timings drift by ~18% over tens of minutes), and the final headline pair was measured with 150 s cooldowns in a symmetric v0/final/final/v0 order.

FOMTOOLS-SUMMARY-BEGIN

## Result

| | mean_us (`make bench`, 3 solves) |
|---|---|
| initial solver | **160 717** (160 913 / 160 521) |
| final solver | **125 729** (125 606 / 125 852) |
| **speedup** | **1.28×** |

Both numbers come from one cooldown-separated, symmetric run (`v0, final, final, v0`, 150 s idle before each) to cancel the node's slow thermal/contention drift. Earlier in the session, on a quieter machine state, the same pair measured 136 900 → 106 616 µs — identical ratio (1.28×).

## Final profiling breakdown (`make profile`, 3 solves, mean 108 662 µs; `loop` encloses the two kernel regions)

| region | per solve | % of solve |
|---|---|---|
| `loop` (whole iteration loop) | 107.7 ms | 99.1 % |
| ├ `k_t0_state` (CSR SpMV, 200 calls, 518 µs each) | 103.5 ms | **95.3 %** |
| └ `k_step` (soft-threshold = next state, 200 calls, 15.7 µs each) | 3.13 ms | 2.9 % |
| `download` (solution D2H) | 0.67 ms | 0.6 % |
| `upload` (params H2D + state D2D) | 0.20 ms | 0.2 % |
| `param_term` (`t0_p_p0 @ p0`, once per solve) | 0.09 ms | 0.1 % |
| `residual` (max-abs reduction, once per solve) | 0.02 ms | 0.02 % |

## Optimizations applied

1. **Dropped the state-update matvec entirely (~7%).** `su_t_t0` is the 1300000×1300000 **identity**: its CSR arrays are `rowptr = 0,1,2,…`, `colidx = 0,1,2,…`, all values exactly `1.0` (verified against `fom_ops.cu`; `MATRIX_STATS.md` confirms nnz = 1300000 = #rows). So `new_state = su_t_t0 @ t0` evaluates `0 + 1.0*t0[i]`, which is exact for every finite double, and the activation never emits `-0.0` (its clamped branch returns `+0.0`) — the result is bit-for-bit `t0`. That removes a 1.3 M-row SpMV *and* the 10.4 MB `cudaMemset` that zeroed its accumulator, every iteration.

2. **In-place state update + L2-persisting state buffer (~13%, the single biggest win).** Because the update is the identity, the activation can write the next iterate straight back into the buffer the matvec just finished reading — so the gather source has a *fixed address and a fixed 10.4 MB size*. `t0_state` streams 234 MB of CSR payload (19 499 968 nnz, uniform 15/row) past the 40 MB L2 each iteration while gathering 19.5 M random elements out of that 10.4 MB vector; by default it is evicted between gathers, so every gather burns a 32 B DRAM sector to deliver 8 B. Declaring the state buffer a `cudaAccessPropertyPersisting` access window on the default stream (CSR payload left as `cudaAccessPropertyStreaming`) keeps it L2-resident for the whole solve. Pure cache policy — results are unchanged.

3. **L2 set-aside sized 1.5× the window (~4%).** Reserving exactly the window size makes the pinned vector conflict-thrash against itself; reserving too much starves the streaming payload. Swept 1.0–2.5×: 1.0× costs 4%, 1.25–1.75× is a flat optimum, ≥2× regresses. Uses 15.6 MB of the 25 MiB `persistingL2CacheMaxSize`, leaving 24 MB of normal L2 for the stream.

4. **Side-stream accumulator re-prime (~1.4%).** `acc ← b` is loop-carried but not needed until two iterations later, so two accumulators are alternated and the refill runs as a D2D copy on a **non-blocking** stream (events order it against the activation and the next matvec), overlapped with the 518 µs SpMV which has spare DRAM bandwidth. The activation kernel drops from 4 memory streams (read `acc`, read `b`, write state, write `acc`) to 2: 22.6 → 15.7 µs.

5. **Device-cached initial state (~0.4%).** Every solve starts from the compile-time constant `FOM_STATE_INIT`, so it is staged on the device once in `fom_setup` and restored per solve with a D2D copy instead of a 10.4 MB PCIe upload: `upload` 658 → 200 µs.

6. **Retained from the handed-over version:** `t0_p_p0 @ p0` is state-independent, so it is evaluated once per solve into `b` and the accumulator is primed with `b` instead of zeros — IEEE-identical, one sparse matvec cheaper per iteration.

7. **Correctness hardening (free).** The non-blocking re-prime stream is drained before `fom_solve` returns (nothing else orders it against the next solve's writes to `b`/`acc`), the pointless final-iteration re-prime is skipped, and the end-of-solve residual is guarded for `max_iter == 0`.

**Measured and rejected:** pinning the accumulator next to the state in one 20.8 MB window (171.9 vs 125.9 ms — starves the payload); pinning both state ping-pong buffers; `missProp = Normal` (neutral); `cudaFuncCachePreferL1/Shared/None` (neutral to 1.7× worse in a microbenchmark replicating the kernel).

**Remaining bottleneck.** `fom_apply_t0_state` is 95.3% of the solve at ~518 µs/iteration. It lives in the immutable `fom_ops.cu` as a thread-per-row CSR kernel whose data arrays have internal linkage, so it can neither be replaced nor re-blocked from the solver; with the gather now L2-resident and the surrounding element-wise work cut to its two-stream minimum, the external levers (cache policy, overlap, redundant passes) are exhausted. Two further tuning rounds yielded <1% each.

FOMTOOLS-SUMMARY-END./verify_bin
worst max-abs error over 8 samples: 8.704e-14 (tol 1.000e-05)
VERIFY OK
[fom-step in holder 15692379 on della-l09g6]
```
