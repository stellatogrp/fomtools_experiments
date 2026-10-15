# Optimization report: fista / openmp (m325000_n1300000_k8)

Fresh-build head-to-head on della-i14g1, mean per solve over 5 solves after 8 warm-ups:

| solver | mean_us |
|---|---:|
| Phase A | 10390281.649400 |
| Phase B | 9065550.431000 |

Kept: **phase_b** (Phase-A / Phase-B time = 1.146). Final verification:

```
worst max-abs error over 8 samples: 2.100e-07 (tol 1.000e-06)
```

## Diff (Phase A -> kept)

```diff
--- phase_a_solver.c	2026-10-14 17:18:56.069764216 -0400
+++ src/fom_solver.c	2026-10-14 18:12:32.888166000 -0400
@@ -1,3 +1,4 @@
+#include <stddef.h>
 #include <string.h>
 #include "fom_solver.h"
 #include "fom_data.h"
@@ -10,8 +11,50 @@
     settings->check_termination = FOM_CHECK_TERMINATION;
 }

+/* nnz of su_state and su_t_t0 (MATRIX_STATS.md). */
+#define FOM_SU_STATE_NNZ 1300000L
+#define FOM_SU_T_T0_NNZ 1300000L
+
+/* 1 when the state update is exactly new_state = [s1; t0], i.e. su_state
+   is the copy block s0 <- s1 and su_t_t0 the copy block s1 <- t0. */
+static int fom_su_is_shift = 0;
+
+/* Checks exactly that out = f(x) copies x[i + src_off] into out[i + dst_off]
+   for i < len and leaves every other row zero. With x = 1 every copied row
+   must give exactly 1, and with x[j] = j + 1 exactly its source index + 1;
+   since nnz == len and len rows are nonzero, each such row holds a single
+   stored entry, so the matrix is exactly that copy block. */
+static int fom_is_copy_block(void (*f)(const fom_float *, fom_float *),
+                             fom_int nin, fom_int nout, long nnz,
+                             fom_int dst_off, fom_int src_off, fom_int len,
+                             fom_float *x, fom_float *o1, fom_float *o2) {
+    fom_int i;
+    int ok = 1;
+    if (nnz != (long)len) return 0;
+    for (i = 0; i < nin; ++i) x[i] = 1.0;
+    for (i = 0; i < nout; ++i) o1[i] = 0.0;
+    f(x, o1);
+    for (i = 0; i < nin; ++i) x[i] = (fom_float)(i + 1);
+    for (i = 0; i < nout; ++i) o2[i] = 0.0;
+    f(x, o2);
+    for (i = 0; i < nout && ok; ++i) {
+        if (i >= dst_off && i < dst_off + len)
+            ok = (o1[i] == 1.0) && (o2[i] == (fom_float)(i - dst_off + src_off + 1));
+        else
+            ok = (o1[i] == 0.0) && (o2[i] == 0.0);
+    }
+    return ok;
+}
+
 void fom_setup(FomWorkspace *work, const FomSettings *settings) {
     work->settings = *settings;
+    fom_su_is_shift =
+        fom_is_copy_block(fom_apply_su_state, FOM_STATE_DIM, FOM_STATE_DIM,
+                          FOM_SU_STATE_NNZ, 0, FOM_T0_DIM, FOM_T0_DIM,
+                          work->acc, work->new_state, work->solution.x) &&
+        fom_is_copy_block(fom_apply_su_t_t0, FOM_T0_DIM, FOM_STATE_DIM,
+                          FOM_SU_T_T0_NNZ, FOM_T0_DIM, 0, FOM_T0_DIM,
+                          work->t0, work->new_state, work->solution.x);
     memcpy(work->state, FOM_STATE_INIT, sizeof(work->state));
     work->solution.iter = 0;
     work->solution.residual = 0.0;
@@ -27,55 +70,137 @@
     fom_float *new_state = work->new_state;
     fom_float *acc = work->acc;
     fom_float *t0 = work->t0;
+    /* Iteration-invariant term t0_p_p0@p0, kept in the unused upper half of acc. */
+    fom_float *cterm = work->acc + FOM_T0_DIM;
     const fom_float lam = FOM_T0_LAMBDA;
     fom_float residual = 0.0;
     fom_int it, i;
+    /* Copy-block path: state and new_state are adjacent in the workspace,
+       giving four contiguous slots of FOM_T0_DIM. The state [s0; s1] is a
+       window of two adjacent slots that slides by one slot per iteration
+       (new_state = [s1; t0] needs only t0 written after s1); s1 is copied
+       back to slot 0 only when the window reaches the end. */
+    const int slide = fom_su_is_shift &&
+        offsetof(FomWorkspace, new_state) ==
+            offsetof(FomWorkspace, state) + sizeof(work->state);
+    fom_float *slots = work->state;
+    int win = 0;

     /* Every solve starts from the initial state. */
     #pragma omp parallel for schedule(static)
     for (i = 0; i < FOM_STATE_DIM; ++i)
         state[i] = FOM_STATE_INIT[i];

+    #pragma omp parallel for schedule(static)
+    for (i = 0; i < FOM_T0_DIM; ++i)
+        cterm[i] = 0.0;
+    fom_apply_t0_p_p0(work->params.p0, cterm);
+    #pragma omp parallel for schedule(static)
+    for (i = 0; i < FOM_T0_DIM; ++i) {
+        t0[i] = cterm[i];
+        acc[i] = 0.0;
+    }
+
     for (it = 0; it < work->settings.max_iter; ++it) {
         const fom_float gamma = fom_sched_gamma(it);

         /* t0 = soft_threshold(t0_state@s + gamma*(d_gamma@s) + P@p0) */
         FOM_PROF_BEGIN(t0_linop);
-        #pragma omp parallel for schedule(static)
-        for (i = 0; i < FOM_T0_DIM; ++i) {
-            t0[i] = 0.0;
-            acc[i] = 0.0;
+        /* t0 = c, acc = 0: done in the previous iteration's update loop on
+           the copy-block path, and before the first iteration. */
+        if (!fom_su_is_shift && it > 0) {
+            #pragma omp parallel for schedule(static)
+            for (i = 0; i < FOM_T0_DIM; ++i) {
+                t0[i] = cterm[i];
+                acc[i] = 0.0;
+            }
         }
         fom_apply_t0_state(state, t0);
-        fom_apply_t0_state__d_gamma(state, acc);
-        fom_apply_t0_p_p0(work->params.p0, t0);
+        /* gamma*(d_gamma@s) vanishes when gamma == 0 (acc stays zero). */
+        if (gamma != 0.0)
+            fom_apply_t0_state__d_gamma(state, acc);
         FOM_PROF_END(t0_linop);

-        FOM_PROF_BEGIN(t0_prox);
-        #pragma omp parallel for schedule(static)
-        for (i = 0; i < FOM_T0_DIM; ++i) {
-            fom_float v = t0[i] + gamma * acc[i];
-            t0[i] = (v > lam) ? (v - lam) : ((v < -lam) ? (v + lam) : 0.0);
+        if (slide) {
+            FOM_PROF_BEGIN(t0_prox);
+            if (win < 2) {
+                /* new_state = window shifted by one slot; write t0 only. */
+                fom_float *out = state + 2 * FOM_T0_DIM;
+                new_state = state + FOM_T0_DIM;
+                #pragma omp parallel for schedule(static)
+                for (i = 0; i < FOM_T0_DIM; ++i) {
+                    fom_float v = t0[i] + gamma * acc[i];
+                    out[i] = (v > lam) ? (v - lam) : ((v < -lam) ? (v + lam) : 0.0);
+                    t0[i] = cterm[i];
+                    acc[i] = 0.0;
+                }
+                ++win;
+            } else {
+                /* Wrap: new_state = slots [0, 1] = [s1; t0]. */
+                new_state = slots;
+                #pragma omp parallel for schedule(static)
+                for (i = 0; i < FOM_T0_DIM; ++i) {
+                    fom_float v = t0[i] + gamma * acc[i];
+                    new_state[i] = state[FOM_T0_DIM + i];
+                    new_state[FOM_T0_DIM + i] =
+                        (v > lam) ? (v - lam) : ((v < -lam) ? (v + lam) : 0.0);
+                    t0[i] = cterm[i];
+                    acc[i] = 0.0;
+                }
+                win = 0;
+            }
+            FOM_PROF_END(t0_prox);
+        } else if (fom_su_is_shift) {
+            /* new_state = [s1; soft_threshold(.)], written in one pass. */
+            FOM_PROF_BEGIN(t0_prox);
+            #pragma omp parallel for schedule(static)
+            for (i = 0; i < FOM_T0_DIM; ++i) {
+                fom_float v = t0[i] + gamma * acc[i];
+                new_state[i] = state[FOM_T0_DIM + i];
+                new_state[FOM_T0_DIM + i] =
+                    (v > lam) ? (v - lam) : ((v < -lam) ? (v + lam) : 0.0);
+                t0[i] = cterm[i];
+                acc[i] = 0.0;
+            }
+            FOM_PROF_END(t0_prox);
+        } else {
+            FOM_PROF_BEGIN(t0_prox);
+            #pragma omp parallel for schedule(static)
+            for (i = 0; i < FOM_T0_DIM; ++i) {
+                fom_float v = t0[i] + gamma * acc[i];
+                t0[i] = (v > lam) ? (v - lam) : ((v < -lam) ? (v + lam) : 0.0);
+            }
+            FOM_PROF_END(t0_prox);
+
+            /* new_state = su_state@s + su_t_t0@t0 */
+            FOM_PROF_BEGIN(state_update);
+            #pragma omp parallel for schedule(static)
+            for (i = 0; i < FOM_STATE_DIM; ++i)
+                new_state[i] = 0.0;
+            fom_apply_su_state(state, new_state);
+            fom_apply_su_t_t0(t0, new_state);
+            FOM_PROF_END(state_update);
         }
-        FOM_PROF_END(t0_prox);
-
-        /* new_state = su_state@s + su_t_t0@t0 */
-        FOM_PROF_BEGIN(state_update);
-        #pragma omp parallel for schedule(static)
-        for (i = 0; i < FOM_STATE_DIM; ++i)
-            new_state[i] = 0.0;
-        fom_apply_su_state(state, new_state);
-        fom_apply_su_t_t0(t0, new_state);
-        FOM_PROF_END(state_update);

         FOM_PROF_BEGIN(residual);
-        residual = 0.0;
-        #pragma omp parallel for schedule(static) reduction(max:residual)
-        for (i = 0; i < FOM_STATE_DIM; ++i) {
-            fom_float d = new_state[i] - state[i];
-            if (d < 0) d = -d;
-            if (d > residual) residual = d;
-            state[i] = new_state[i];
+        /* Residual is only observable when checked for termination or
+           reported after the final iteration; the copy is a pointer swap. */
+        if ((FOM_USE_TOL && work->settings.check_termination) ||
+            it == work->settings.max_iter - 1) {
+            residual = 0.0;
+            #pragma omp parallel for schedule(static) reduction(max:residual)
+            for (i = 0; i < FOM_STATE_DIM; ++i) {
+                fom_float d = new_state[i] - state[i];
+                if (d < 0) d = -d;
+                if (d > residual) residual = d;
+            }
+        }
+        if (slide) {
+            state = new_state;
+        } else {
+            fom_float *tmp = state;
+            state = new_state;
+            new_state = tmp;
         }
         FOM_PROF_END(residual);

```

## Agent notes (end of the Phase-B transcript)

```
The optimized `src/fom_solver.c` passes: the final `make verify` printed **VERIFY OK** (worst error 2.1e-07, same as before), and `make bench` gives **9,064,380 µs** per solve. That's about 13% faster than the original on the same node. 97% of the time is now inside the two `t0` matrix–vector helpers, which I'm not allowed to change.

**Node change:** partway through, the Slurm job moved from `della-i12g2` to the much slower `della-i14g1`. The original measured 6,107,642 µs on the first node and 10,387,269 µs when I re-ran it on the second. All comparisons below are within one node.

**Exact structure I used:** the docs only give nnz counts, so I checked the two state-update matrices with an exact test. Two probe vectors (all ones, then each entry set to its own index), plus nnz = 1.3M, show the update is exactly `new_state = [s1; t0]`, pure copies. The solver repeats this check once in `fom_setup` and falls back to the original code if it ever fails.

**Rejected – the biggest potential win:** `t0_state` is `[0, G]` and `d_gamma` is `[-G, G]`. That would let one 30M-nnz product replace the current 30M + 39M (about 1.32 s per solve when tried). But the G stored in the two matrices differs by about 1e-15. Over 200 iterations that grows to 1.3e-6, which fails the 1e-6 check, so I reverted it both times.

**Also rejected:** parallel first-touch of the workspace pages (9.227 s vs 9.211 s) and pinning OpenMP threads to cores (9.176 s vs 9.173 s); neither improved the time.

I saved these findings to memory for future runs on this problem.

- **Initial mean_us:** 6,107,642 (`della-i12g2`); 10,387,269 for the original on the final node `della-i14g1`
- **Final mean_us:** 9,064,380 (`della-i14g1`)
- **Final profile regions:**
  - `t0_linop`: 26,429,692 µs, 97.12%
  - `t0_prox`: 780,204 µs, 2.87%
  - `residual`: 3,331 µs, 0.01%
  - `state_update` is gone: it is now done inside the `t0_prox` loop.
- **Optimizations kept:**
  - Compute the parameter term `t0_p_p0 @ p0` once per solve instead of every iteration (6.108 → 5.962 s).
  - Swap state pointers instead of copying the state back, and compute the residual only on the last iteration. With no tolerance check it is only reported there (5.962 → 5.763 s).
  - Replace the state-update products with the exactly verified copy `new_state = [s1; t0]`, written in the same loop as the soft-threshold (5.763 → 5.251 s).
  - Reset `t0 = c` and `acc = 0` for the next iteration inside that same loop (9.211 → 9.204 s; marginal but kept by the rule).
  - Skip the `d_gamma` product when `gamma == 0`; it is multiplied by exactly zero (9.204 → 9.173 s).
  - Slide the `[s0; s1]` window across the adjacent `state`/`new_state` arrays so `s1` is copied only every 3rd iteration (9.173 → 9.069 s).
```
