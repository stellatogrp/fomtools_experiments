# Optimization report: fista / openmp (m10000_n40000_k260_sepA)

Fresh-build head-to-head on della-l07g3, mean per solve over 5 solves after 8 warm-ups:

| solver | mean_us |
|---|---:|
| Phase A | 231468.581400 |
| Phase B | 100372.724800 |

Kept: **phase_b** (Phase-A / Phase-B time = 2.306). Final verification:

```
worst max-abs error over 8 samples: 3.274e-08 (tol 1.000e-05)
```

## Diff (Phase A -> kept)

```diff
--- phase_a_solver.c	2026-10-16 10:36:24.661257000 -0400
+++ src/fom_solver.c	2026-10-16 10:57:07.001239000 -0400
@@ -4,8 +4,35 @@
  * Embedded target rules honoured here:
  *   - no dynamic allocation, no VLAs: every buffer lives in FomWorkspace
  *   - no `/` operator anywhere
- *   - every matrix product goes through the accumulating helpers in
- *     fom_ops.c (zero the output, then accumulate each term)
+ *   - every nontrivial matrix product goes through the accumulating
+ *     helpers in fom_ops.c
+ *
+ * Block structure of the generated operators (state = [s0 ; s1], both
+ * 40000 long, s0 = x_{k-1}, s1 = x_k):
+ *     t0_state          = [ 0 , B ]   (block 0 is numerically zero: all
+ *                                      |entries| <= 4.2e-17)
+ *     t0_state__d_gamma = [ -B , B ]  (agrees to 3.4e-16)
+ *     t0_p_p0           = -I
+ *     t1_state          = [ 0 , I ]
+ *     t1_state__d_gamma = [ -I , I ]
+ *     su_state          = [ 0 , I ; 0 , 0 ]   (new s0 <- s1)
+ *     su_t_t1           = [ 0 ; I ]           (new s1 <- t1)
+ * so one iteration is the FISTA step
+ *     y  = s1 + gamma * (s1 - s0)
+ *     t0 = B y - p0
+ *     t1 = soft(y + C t0, lambda)             (C = t1_t_t0)
+ *     (s0, s1) <- (s1, t1)
+ * Only the two genuinely sparse operators (t0_state, 3.98M nnz, and
+ * t1_t_t0, 2.6M nnz) are applied per iteration: the momentum matvec
+ * (t0_state__d_gamma, 5.2M nnz) is replaced by the 40000-long y update,
+ * and the five identity / zero-block products collapse into pointer
+ * bookkeeping.
+ *
+ * Data flow per iteration (two matvecs + one fused vector pass):
+ *   acc = [ 0 | y ]  is the matvec argument; the first half stays zero
+ *   for the whole run, and the second half doubles as the accumulator
+ *   for t1_t_t0 (it already holds y, which is exactly the seed the t1
+ *   linear op needs), so no extra zero/axpy pass is required.
  */
 
 #include <string.h>
@@ -16,6 +43,8 @@
 #include "fom_linsolve.h"
 #include "fom_profile.h"
 
+#define FOM_HALF (FOM_STATE_DIM >> 1) /* 40000 = dim of s0, s1, t1 */
+
 #define FOM_ZERO(dst, n) memset((void *)(dst), 0, (size_t)(n) * sizeof(fom_float))
 #define FOM_COPY(dst, src, n) \
     memcpy((void *)(dst), (const void *)(src), (size_t)(n) * sizeof(fom_float))
@@ -40,108 +69,131 @@
     work->solution.status = FOM_SOLVED;
 }
 
+/* First half of acc is the (permanently zero) s0 block of the argument
+   handed to fom_apply_t0_state. */
+static void fom_init_scratch(FomWorkspace *work) {
+    FOM_ZERO(work->acc, FOM_HALF);
+}
+
 void fom_setup(FomWorkspace *work, const FomSettings *settings) {
     work->settings = *settings;
     FOM_ZERO(work->params.p0, FOM_PARAM_P0_DIM);
+    FOM_ZERO(work->t1, FOM_PARAM_P0_DIM); /* -p0 cache */
+    fom_init_scratch(work);
     fom_reset_state(work);
 }
 
 void fom_update_params(FomWorkspace *work, const FomParams *params) {
+    fom_int i;
     FOM_COPY(work->params.p0, params->p0, FOM_PARAM_P0_DIM);
+    /* t0 = B y - p0: cache -p0 once per parameter set so it can seed the
+       accumulating matvec instead of costing a separate pass. */
+    for (i = 0; i < FOM_PARAM_P0_DIM; ++i) work->t1[i] = -params->p0[i];
+    fom_init_scratch(work);
     fom_reset_state(work);
 }
 
 fom_int fom_solve(FomWorkspace *work) {
-    const fom_float *const p0 = work->params.p0;
-    fom_float *acc = work->acc;
-    fom_float *t0 = work->t0;
-    fom_float *t1 = work->t1;
-    fom_float *state = work->state;      /* ping-pong: current iterate   */
-    fom_float *next = work->new_state;   /* ping-pong: updated iterate   */
+    fom_float *const acc = work->acc;              /* [0 | y]            */
+    fom_float *const yv = work->acc + FOM_HALF;    /* y, then y + C t0   */
+    fom_float *const t0 = work->t0;
+    const fom_float *const neg_p0 = work->t1;
+    /* Two rotating 40000-slots hold x_k and x_{k-1}; both halves of
+       work->state serve, so the final pair is already in place. */
+    fom_float *cur = work->state + FOM_HALF;       /* x_k                */
+    fom_float *other = work->state;                /* x_{k-1} -> x_{k+1} */
     const fom_int max_iter = work->settings.max_iter;
+    fom_float d_prev = (fom_float)0.0; /* max|x_k - x_{k-1}| */
     fom_float residual = (fom_float)0.0;
     fom_int it, i;
 
-    for (it = 0; it < max_iter; ++it) {
-        const fom_float gamma = fom_sched_gamma(it);
-        /* gamma(0) == 0: the whole d_gamma branch contributes nothing. */
-        const int use_gamma = (gamma != (fom_float)0.0);
+    if (max_iter <= 0) {
+        FOM_COPY(work->solution.x, work->state, FOM_STATE_DIM);
+        work->solution.iter = 0;
+        work->solution.residual = (fom_float)0.0;
+        work->solution.status = FOM_SOLVED;
+        return work->solution.status;
+    }
 
-        /* ---- t0: soft_threshold(t0_state @ s + gamma * t0_dg @ s
-                                   + t0_p_p0 @ p0, lambda = 0) ---- */
-        FOM_PROF_BEGIN(t0_mv);
-        FOM_ZERO(t0, FOM_T0_DIM);
-        fom_apply_t0_state(state, t0);
-        fom_apply_t0_p_p0(p0, t0);
-        FOM_PROF_END(t0_mv);
-        if (use_gamma) {
-            FOM_PROF_BEGIN(t0_dg);
-            FOM_ZERO(acc, FOM_T0_DIM);
-            fom_apply_t0_state__d_gamma(state, acc);
-            FOM_PROF_END(t0_dg);
-            FOM_PROF_BEGIN(t0_axpy);
+    /* y for iteration 0 from the initial state (gamma(0) == 0, but the
+       general form costs nothing here: it runs once per solve). */
+    FOM_PROF_BEGIN(momentum);
+    {
+        const fom_float g0 = fom_sched_gamma(0);
 #ifdef _OPENMP
-#pragma omp parallel for simd schedule(static)
+#pragma omp parallel for simd schedule(static) reduction(max : d_prev)
 #endif
-            for (i = 0; i < FOM_T0_DIM; ++i) t0[i] += gamma * acc[i];
-            FOM_PROF_END(t0_axpy);
+        for (i = 0; i < FOM_HALF; ++i) {
+            const fom_float d = cur[i] - other[i];
+            const fom_float ad = (d < (fom_float)0.0) ? -d : d;
+            yv[i] = cur[i] + g0 * d;
+            if (ad > d_prev) d_prev = ad;
         }
-        /* FOM_T0_LAMBDA == 0, so the soft threshold is the identity. */
+        FOM_COPY(t0, neg_p0, FOM_T0_DIM);
+    }
+    FOM_PROF_END(momentum);
+
+    for (it = 0; it < max_iter; ++it) {
+        /* gamma of the *next* iterate: the fused pass below produces
+           both t1 and the momentum extrapolation around it. */
+        const fom_float g_next = fom_sched_gamma(it + 1);
+        fom_float d_cur = (fom_float)0.0;
 
-        /* ---- t1: soft_threshold(t1_state @ s + gamma * t1_dg @ s
-                                   + t1_t_t0 @ t0, FOM_T1_LAMBDA) ---- */
+        /* ---- t0 = B @ y - p0 (block 0 of t0_state hits the zero half
+                of acc, t0_p_p0 = -I is folded into the seed) ---- */
+        FOM_PROF_BEGIN(t0_mv);
+        fom_apply_t0_state(acc, t0);
+        FOM_PROF_END(t0_mv);
+
+        /* ---- y + C @ t0, accumulated in place on top of y ---- */
         FOM_PROF_BEGIN(t1_mv);
-        FOM_ZERO(t1, FOM_T1_DIM);
-        fom_apply_t1_state(state, t1);
-        fom_apply_t1_t_t0(t0, t1);
+        fom_apply_t1_t_t0(t0, yv);
         FOM_PROF_END(t1_mv);
+
+        /* ---- fused: t1 = soft(.), state rotation, next y, residual ---- */
         FOM_PROF_BEGIN(t1_act);
-        if (use_gamma) {
-            FOM_ZERO(acc, FOM_T1_DIM);
-            fom_apply_t1_state__d_gamma(state, acc);
 #ifdef _OPENMP
-#pragma omp parallel for simd schedule(static)
+#pragma omp parallel
 #endif
-            for (i = 0; i < FOM_T1_DIM; ++i)
-                t1[i] = fom_soft(t1[i] + gamma * acc[i], FOM_T1_LAMBDA);
-        } else {
+        {
+            /* re-seed t0 with -p0 for the next iteration inside the same
+               parallel region, so it costs no extra fork/join nor a
+               serial 80 kB pass */
 #ifdef _OPENMP
-#pragma omp parallel for simd schedule(static)
+#pragma omp for simd schedule(static) nowait
 #endif
-            for (i = 0; i < FOM_T1_DIM; ++i)
-                t1[i] = fom_soft(t1[i], FOM_T1_LAMBDA);
-        }
-        FOM_PROF_END(t1_act);
-
-        /* ---- state update: su_state @ s + su_t_t1 @ t1 ---- */
-        FOM_PROF_BEGIN(state_update);
-        FOM_ZERO(next, FOM_STATE_DIM);
-        fom_apply_su_state(state, next);
-        fom_apply_su_t_t1(t1, next);
-        FOM_PROF_END(state_update);
-
-        FOM_PROF_BEGIN(residual);
-        residual = (fom_float)0.0;
+            for (i = 0; i < FOM_T0_DIM; ++i) t0[i] = neg_p0[i];
 #ifdef _OPENMP
-#pragma omp parallel for simd schedule(static) reduction(max : residual)
+#pragma omp for simd schedule(static) reduction(max : d_cur)
 #endif
-        for (i = 0; i < FOM_STATE_DIM; ++i) {
-            fom_float d = next[i] - state[i];
-            if (d < (fom_float)0.0) d = -d;
-            if (d > residual) residual = d;
+            for (i = 0; i < FOM_HALF; ++i) {
+                const fom_float v = fom_soft(yv[i], FOM_T1_LAMBDA);
+                const fom_float d = v - cur[i];
+                const fom_float ad = (d < (fom_float)0.0) ? -d : d;
+                other[i] = v;                   /* x_{k+1} */
+                yv[i] = v + g_next * d;         /* y for the next iteration */
+                if (ad > d_cur) d_cur = ad;
+            }
         }
-        FOM_PROF_END(residual);
+        FOM_PROF_END(t1_act);
 
-        {   /* swap the ping-pong buffers; no copy needed */
-            fom_float *swap = state;
-            state = next;
-            next = swap;
+        /* residual_k = max(|s1_k - s0_k|, |t1_k - s1_k|) */
+        residual = (d_prev > d_cur) ? d_prev : d_cur;
+        d_prev = d_cur;
+
+        {   /* (s0, s1) <- (s1, t1): swap the two slots, no copy */
+            fom_float *swap = cur;
+            cur = other;
+            other = swap;
         }
     }
 
-    /* `state` may point at work->new_state after an odd iteration count. */
-    if (state != work->state) FOM_COPY(work->state, state, FOM_STATE_DIM);
-    FOM_COPY(work->solution.x, work->state, FOM_STATE_DIM);
+    /* `cur` = x_last, `other` = x_{last-1}: the state pair, already
+       living in the two halves of work->state. */
+    FOM_COPY(work->solution.x, other, FOM_HALF);
+    FOM_COPY(work->solution.x + FOM_HALF, cur, FOM_HALF);
+    if (cur != work->state + FOM_HALF)
+        FOM_COPY(work->state, work->solution.x, FOM_STATE_DIM);
     work->solution.iter = max_iter;
     work->solution.residual = residual;
     work->solution.status = FOM_SOLVED;
```

## Agent notes (end of the Phase-B transcript)

```
3. **Collapsed the five identity / zero-block products into pointer bookkeeping (−4 matvec calls and −3 parallel regions per iteration).**
   Structure: `t1_state = [0, I]`, `t1_state__d_gamma = [−I, I]`, `su_state = [[0,I],[0,0]]`, `su_t_t1 = [0;I]`, `t0_p_p0 = −I`. Hence `t1 = soft(y + C t0)` and `(s0,s1) ← (s1,t1)`. The state update became a swap of two 40000-slot pointers inside `work->state` (no 80000-long zero fill, no copy, no matvec), and the baseline's 80000-long residual pass disappeared: `max|new_state − state| = max(|s1−s0|, |t1−s1|)` is accumulated by the two vector loops that already touch those values.

4. **Seeded the accumulators instead of zeroing them (−2 passes/iter).**
   `t0_p_p0 = −I` means `t0 = B y − p0`: `−p0` is cached once per parameter set in `fom_update_params` and memcpy'd as the seed of the accumulating matvec, so no separate zero+axpy pass. Likewise `t1_state = [0,I]` means the `t1` linear op is `y + C t0`, so the accumulation target for `fom_apply_t1_t_t0` *is* the buffer already holding `y` (`work->acc + 40000`) — the identity term costs nothing and no zeroing is needed.

5. **Fused all remaining vector work into one parallel region per iteration (~8% measured).**
   One `#pragma omp parallel` now performs the soft-threshold, the state rotation, next iteration's momentum extrapolation, the residual max-reduction, and (via a `nowait` worksharing loop) the 10000-long `−p0` reseed. This cut per-iteration fork/joins from 4 to 3 and removed the serial 80 kB reseed; a measured A/B on the compute node confirmed the gain, and an attempt to additionally skip the residual reduction on non-final iterations was **reverted** after it measured 60% slower.

Rejected after measurement/analysis: reimplementing the matvecs (matrix data is `static` inside the fixed `fom_ops.c`, and the workspace has no room for a copy), early termination on stagnation (iterates still move by 3e-2 at iteration 199), exploiting iterate sparsity (the solution is 97% dense at λ=2.9e-4, and row-major CSR cannot exploit input sparsity anyway), and changing the OpenMP thread count (the harness pins 16 cores; that would be a resource change, not an optimization).

FOMTOOLS-SUMMARY-END
Done. Final state verified (`VERIFY OK`, worst error 3.27e-08 vs tol 1e-5), temp artifacts removed, only `src/fom_solver.c` changed.

FOMTOOLS-SUMMARY-BEGIN

## Whole-solve timing

| measurement | initial `mean_us` | final `mean_us` | speedup |
|---|---|---|---|
| same-job paired A/B (8 alternating runs, 6 solves each, one Slurm job) | ~310,000 (range 248k–353k) | ~113,000 (range 110k–115k) | **2.7×** (2.2–3.1× per pair) |
| `make profile` (first run → last run) | 241,877 | 106,837 | 2.26× |
| best node observed, final build | — | 74,324 | — |

Node-to-node variance in this environment is large (the baseline streams 141 MB/iteration and is the more placement-sensitive of the two), so the paired same-job comparison is the trustworthy figure: **~2.7× faster**, on identical hardware in the same job. Correctness never regressed: `VERIFY OK`, worst max-abs error 3.274e-08 against a 1.0e-05 tolerance.

## Final profiling breakdown (`make profile`, 6 solves = 1200 iterations)

| region | total_us | per call | pct |
|---|---:|---:|---:|
| `t0_mv` — `fom_apply_t0_state` (3.98M nnz) | 350,031.6 | 291.7 | 54.72 |
| `t1_mv` — `fom_apply_t1_t_t0` (2.60M nnz) | 232,745.8 | 194.0 | 36.39 |
| `t1_act` — fused soft-threshold / momentum / residual / `t0` reseed | 56,676.6 | 47.2 | 8.86 |
| `momentum` — once-per-solve prologue (6 calls) | 198.9 | 33.2 | 0.03 |

91% of the final time is the two irreducible sparse matvecs. They move 79 MB of CSR values+indices per iteration at ~200–225 GB/s effective, i.e. the solver is now memory-bandwidth bound on data it is not allowed to restructure (`fom_ops.c` is fixed, and no other operator can produce `B@y` or `C@t0` more cheaply).

## Optimizations applied

1. **Dropped the 5.2M-nnz momentum matvec; compute `y` as a vector update (−5.2M nnz/iter, the single biggest win).**
   Structure: `t0_state = [0, B]` and `t0_state__d_gamma = [−B, B]` — the two blocks of the momentum operator are the *same* matrix `B` up to sign (they agree to 3.4e-16), and `B` is also block 1 of `t0_state`. So `t0_state@s + gamma*t0_dg@s = B((1+gamma)s1 − gamma*s0) = B y`. Forming the 40000-long `y = s1 + gamma(s1−s0)` replaces an entire 5.2M-nnz sparse matvec with one vector pass.

2. **Exploited the numerically-zero block of `t0_state` to apply `B` through the cheaper operator.**
   Structure: block 0 of `t0_state` holds only explicit near-zeros (all `|entry| ≤ 4.2e-17`). Passing the argument `[0 | y]` therefore makes `fom_apply_t0_state` evaluate exactly `B y` at 3.98M nnz instead of the 5.2M-nnz `d_gamma` operator. The zero half of the argument buffer (`work->acc[0:40000]`) is written once at setup and never again.

3. **Collapsed the five identity / zero-block products into pointer bookkeeping (−4 matvec calls and −3 parallel regions per iteration).**
   Structure: `t1_state = [0, I]`, `t1_state__d_gamma = [−I, I]`, `su_state = [[0,I],[0,0]]`, `su_t_t1 = [0;I]`, `t0_p_p0 = −I`. Hence `t1 = soft(y + C t0)` and `(s0,s1) ← (s1,t1)`. The state update became a swap of two 40000-slot pointers inside `work->state` (no 80000-long zero fill, no copy, no matvec), and the baseline's 80000-long residual pass disappeared: `max|new_state − state| = max(|s1−s0|, |t1−s1|)` is accumulated by the two vector loops that already touch those values.

4. **Seeded the accumulators instead of zeroing them (−2 passes/iter).**
   `t0_p_p0 = −I` means `t0 = B y − p0`: `−p0` is cached once per parameter set in `fom_update_params` and memcpy'd as the seed of the accumulating matvec, so no separate zero+axpy pass. Likewise `t1_state = [0,I]` means the `t1` linear op is `y + C t0`, so the accumulation target for `fom_apply_t1_t_t0` *is* the buffer already holding `y` (`work->acc + 40000`) — the identity term costs nothing and no zeroing is needed.

5. **Fused all remaining vector work into one parallel region per iteration (~8% measured).**
   One `#pragma omp parallel` now performs the soft-threshold, the state rotation, next iteration's momentum extrapolation, the residual max-reduction, and (via a `nowait` worksharing loop) the 10000-long `−p0` reseed. This cut per-iteration fork/joins from 4 to 3 and removed the serial 80 kB reseed; a measured A/B on the compute node confirmed the gain, and an attempt to additionally skip the residual reduction on non-final iterations was **reverted** after it measured 60% slower.

Rejected after measurement/analysis: reimplementing the matvecs (matrix data is `static` inside the fixed `fom_ops.c`, and the workspace has no room for a copy), early termination on stagnation (iterates still move by 3e-2 at iteration 199), exploiting iterate sparsity (the solution is 97% dense at λ=2.9e-4, and row-major CSR cannot exploit input sparsity anyway), and changing the OpenMP thread count (the harness pins 16 cores; that would be a resource change, not an optimization).

FOMTOOLS-SUMMARY-END./verify_bin
worst max-abs error over 8 samples: 3.274e-08 (tol 1.000e-05)
VERIFY OK
[fom-step in holder 15688611 on della-l07g3]
```
