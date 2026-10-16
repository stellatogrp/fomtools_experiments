# Optimization report: ista / openmp (m325000_n1300000_k8, D and D^T separate)

Fresh-build head-to-head on della-i12g1, mean per solve over 5 solves after 8 warm-ups:

| solver | mean_us |
|---|---:|
| Phase A | 633075.170000 |
| Phase B | 433750.227000 |

Kept: **phase_b** (Phase-A / Phase-B time = 1.460). Final verification:

```
worst max-abs error over 8 samples: 0.000e+00 (tol 1.000e-05)
```

## Diff (Phase A -> kept)

```diff
--- phase_a_solver.c	2026-10-16 06:13:02.767388000 -0400
+++ src/fom_solver.c	2026-10-16 06:34:51.520526000 -0400
@@ -11,21 +11,36 @@
  * Every matrix-vector product goes through the accumulating CSR helpers
  * in fom_ops.c; this spec contains no affine solves.
  *
- * Workspace buffer roles:
- *   state      the iterate, updated in place (su_t_t1 writes back into it)
- *   acc        t1 linear-op accumulator, soft-thresholded in place
- *   t0         t0 linear-op accumulator, pre-seeded with the parameter term
+ * ---- Structure exploited -------------------------------------------------
+ * Two of the five operators of this spec are *identity* blocks:
+ *     t1_state  (1300000 x 1300000, 1 nnz/row, diag == 1)  ==  I
+ *     su_t_t1   (1300000 x 1300000, 1 nnz/row, diag == 1)  ==  I
+ * (and t0_p_p0 is -I, 1 nnz/row -- not assumed below, see `pre`).
+ * With those two identities the iteration collapses to
+ *     t0 <- t0_state @ s + (t0_p_p0 @ p0)      <- parameter term hoisted
+ *     s  <- soft(s + t1_t_t0 @ t0, lambda)     <- accumulate *into* s
+ * i.e. the t1_state matvec, the su_t_t1 matvec, the zero-fill of the t1
+ * accumulator and the separate t1 -> state copy all disappear: the only
+ * remaining O(nnz) kernels are the two genuinely sparse ones.
+ *
+ * Because that rewrite is only valid for *these* numeric blocks, it is not
+ * taken on faith: fom_setup probes both operators against two pseudo-random
+ * vectors (fom_probe_identity) and only arms the fast path on an exact,
+ * bit-for-bit match.  If the numeric data ever changes, the probe fails and
+ * fom_solve_generic -- the literal SPEC transcription, every term through its
+ * own fom_apply_* helper -- runs instead.  Correctness never depends on the
+ * assumption; only speed does.
+ *
+ * Workspace buffer roles (fast path):
+ *   state      the iterate; t1_t_t0 accumulates into it, then it is
+ *              soft-thresholded in place -- no ping-pong buffer, so the
+ *              hot t0_state gather sees one resident 1.3M-double vector
+ *   t0         t0 linear-op accumulator, re-seeded with the parameter term
+ *              at the tail of each sweep (fused with the threshold pass)
  *   t1         holds t0_p_p0 @ p0, loop-invariant and hoisted out of the loop
+ *   acc        generic path only
  *   new_state  only touched on the final sweep, to hold the previous iterate
  *              for the reported residual
- *
- * Shape of the loop: the only sweeps this file owns are folded into two
- * OpenMP regions per iteration.  `state` is updated in place so that the
- * zero-fill su_t_t1 needs is written immediately before the call that
- * consumes it, and so no ping-pong buffer enters the cache footprint --
- * the t0_state gather over `state` is the hot kernel and is sensitive to
- * how much of `state` is resident.  The matvecs are issued in dependency
- * order chosen for that same reason; see the comments in the loop.
  */

 #include <math.h>
@@ -39,6 +54,36 @@
 #define FOM_N FOM_STATE_DIM
 #define FOM_M FOM_T0_DIM

+/* Armed by fom_setup when the identity probe below succeeds. */
+static int fom_identity_blocks = 0;
+
+typedef void (*fom_apply_fn)(const fom_float *, fom_float *);
+
+/* Return 1 iff `apply` reproduces its input exactly on two pseudo-random
+   vectors, i.e. the operator is (to within an event of probability zero)
+   the identity.  Scratch: both FOM_N-sized buffers, setup time only. */
+static int fom_probe_identity(fom_apply_fn apply, fom_float *x, fom_float *y) {
+    unsigned long long rng;
+    fom_int trial, i;
+
+    for (trial = 0; trial < 2; ++trial) {
+        rng = 0x9E3779B97F4A7C15ULL + (unsigned long long)trial * 0xD1B54A32D192ED03ULL;
+        for (i = 0; i < FOM_N; ++i) {
+            rng ^= rng >> 30; rng *= 0xBF58476D1CE4E5B9ULL;
+            rng ^= rng >> 27; rng *= 0x94D049BB133111EBULL;
+            rng ^= rng >> 31;
+            /* nonzero, O(1), and distinct across lanes */
+            x[i] = (fom_float)(long long)(rng >> 12) * (fom_float)1e-15 + (fom_float)1.0;
+        }
+        for (i = 0; i < FOM_N; ++i) y[i] = (fom_float)0;
+        apply(x, y);
+        for (i = 0; i < FOM_N; ++i) {
+            if (y[i] != x[i]) return 0;
+        }
+    }
+    return 1;
+}
+
 /* Recompute the loop-invariant parameter term t0_p_p0 @ p0. */
 static void fom_prepare_param_term(FomWorkspace *work) {
     fom_float *pre = work->t1;
@@ -81,6 +126,11 @@
     /* Well-defined parameters even if fom_update_params is never called. */
     for (i = 0; i < FOM_PARAM_P0_DIM; ++i) work->params.p0[i] = (fom_float)0;

+    /* One-off structural probe (never inside a solve). */
+    fom_identity_blocks =
+        fom_probe_identity(fom_apply_t1_state, work->acc, work->new_state) &&
+        fom_probe_identity(fom_apply_su_t_t1, work->acc, work->new_state);
+
     fom_reset_state(work);
     fom_prepare_param_term(work);
 }
@@ -94,17 +144,83 @@
     fom_prepare_param_term(work);
 }

-fom_int fom_solve(FomWorkspace *work) {
+/* ---- Literal SPEC transcription; used when the probe does not arm. ------ */
+static fom_float fom_solve_generic(FomWorkspace *work, fom_int max_iter) {
     fom_float *const s = work->state;
     fom_float *const acc = work->acc;
     fom_float *const t0 = work->t0;
     fom_float *const old = work->new_state;
     const fom_float *const pre = work->t1;
     const fom_float lam = FOM_T1_LAMBDA;
+    fom_float residual = (fom_float)0;
+    fom_int iter, i;
+
+    for (iter = 0; iter < max_iter; ++iter) {
+        const int want_res = (iter + 1 >= max_iter);
+
+#ifdef _OPENMP
+        #pragma omp parallel for schedule(static)
+#endif
+        for (i = 0; i < FOM_N; ++i) acc[i] = (fom_float)0;
+        fom_apply_t1_state(s, acc);
+
+#ifdef _OPENMP
+        #pragma omp parallel for schedule(static)
+#endif
+        for (i = 0; i < FOM_M; ++i) t0[i] = pre[i];
+        fom_apply_t0_state(s, t0);
+        fom_apply_t1_t_t0(t0, acc);
+
+        if (want_res) {
+#ifdef _OPENMP
+            #pragma omp parallel for schedule(static)
+#endif
+            for (i = 0; i < FOM_N; ++i) old[i] = s[i];
+        }
+
+#ifdef _OPENMP
+        #pragma omp parallel for schedule(static)
+#endif
+        for (i = 0; i < FOM_N; ++i) {
+            fom_float v = acc[i];
+            fom_float a = fabs(v) - lam;
+            acc[i] = copysign(a > (fom_float)0 ? a : (fom_float)0, v);
+            s[i] = (fom_float)0;
+        }
+
+        fom_apply_su_t_t1(acc, s);
+        work->solution.iter = iter + 1;
+
+        if (want_res) {
+            fom_float r = (fom_float)0;
+#ifdef _OPENMP
+            #pragma omp parallel for schedule(static) reduction(max : r)
+#endif
+            for (i = 0; i < FOM_N; ++i) {
+                fom_float d = fabs(s[i] - old[i]);
+                if (d > r) r = d;
+            }
+            residual = r;
+        }
+    }
+    return residual;
+}
+
+fom_int fom_solve(FomWorkspace *work) {
+    fom_float *const s = work->state;
+    fom_float *const t0 = work->t0;
+    fom_float *const old = work->new_state;
+    const fom_float *const pre = work->t1;
+    const fom_float lam = FOM_T1_LAMBDA;
     const fom_int max_iter = work->settings.max_iter;
     fom_float residual = (fom_float)0;
     fom_int iter, i;

+    if (!fom_identity_blocks) {
+        residual = fom_solve_generic(work, max_iter);
+        goto finish;
+    }
+
     /* Seed the t0 accumulator for the first sweep; every later sweep is
        seeded at the tail of the preceding one. */
 #ifdef _OPENMP
@@ -115,41 +231,16 @@
     for (iter = 0; iter < max_iter; ++iter) {
         /* Residual is reported only; with FOM_USE_TOL == 0 nothing reads it
            before the loop ends, so only the final sweep pays for it. */
-#if FOM_USE_TOL
-        const int want_res = 1;
-#else
         const int want_res = (iter + 1 >= max_iter);
-#endif

-        /* ---- t1 = soft(t1_state @ state + t1_t_t0 @ t0, lam) ----------
-           t1_state is run before t0_state on purpose: it walks `state`
-           sequentially, which pulls the vector into cache right ahead of
-           the scattered t0_state gather over the same vector.  Measured
-           ~7% faster than the natural SPEC order on this target. */
-        FOM_PROF_BEGIN(acc_zero);
-#ifdef _OPENMP
-        #pragma omp parallel for schedule(static)
-#endif
-        for (i = 0; i < FOM_N; ++i) acc[i] = (fom_float)0;
-        FOM_PROF_END(acc_zero);
-
-        FOM_PROF_BEGIN(mv_t1_state);
-        fom_apply_t1_state(s, acc);
-        FOM_PROF_END(mv_t1_state);
-
-        /* ---- t0 = t0_state @ state + t0_p_p0 @ p0 ---------------------
-           The hot kernel: a scattered gather over `state`.  Its activation
-           has lambda exactly 0, i.e. the identity, so no extra sweep is
-           needed -- the accumulator already holds t0.  t1_t_t0, the only
-           consumer of t0, follows immediately below. */
+        /* ---- t0 = t0_state @ s + t0_p_p0 @ p0 -------------------------
+           The hot kernel: a scattered gather over `s`.  Its activation has
+           lambda exactly 0, i.e. the identity, so the accumulator already
+           holds t0 -- no extra sweep. */
         FOM_PROF_BEGIN(mv_t0_state);
         fom_apply_t0_state(s, t0);
         FOM_PROF_END(mv_t0_state);

-        FOM_PROF_BEGIN(mv_t1_t_t0);
-        fom_apply_t1_t_t0(t0, acc);
-        FOM_PROF_END(mv_t1_t_t0);
-
         if (want_res) {
             FOM_PROF_BEGIN(save_old);
 #ifdef _OPENMP
@@ -159,10 +250,19 @@
             FOM_PROF_END(save_old);
         }

-        /* One region for everything this file still owes: threshold acc in
-           place, clear `state` for the accumulating state update that comes
-           next, and re-seed t0 for the following sweep (t1_t_t0 has just
-           finished reading it). */
+        /* ---- s += t1_t_t0 @ t0 ----------------------------------------
+           t1_state == I, so the t1 accumulator *is* the old iterate: the
+           helper accumulates straight into `s` (no zero-fill, no second
+           pass over s, no separate t1 buffer).  su_t_t1 == I makes the
+           state update a no-op, so this result is already the new state
+           once thresholded. */
+        FOM_PROF_BEGIN(mv_t1_t_t0);
+        fom_apply_t1_t_t0(t0, s);
+        FOM_PROF_END(mv_t1_t_t0);
+
+        /* Everything this file still owes, in one parallel region:
+           threshold `s` in place and re-seed t0 for the next sweep
+           (t1_t_t0 has just finished reading it). */
         FOM_PROF_BEGIN(fused);
 #ifdef _OPENMP
         #pragma omp parallel
@@ -173,10 +273,9 @@
             #pragma omp for schedule(static) nowait
 #endif
             for (j = 0; j < FOM_N; ++j) {
-                fom_float v = acc[j];
+                fom_float v = s[j];
                 fom_float a = fabs(v) - lam;
-                acc[j] = copysign(a > (fom_float)0 ? a : (fom_float)0, v);
-                s[j] = (fom_float)0;
+                s[j] = copysign(a > (fom_float)0 ? a : (fom_float)0, v);
             }
 #ifdef _OPENMP
             #pragma omp for schedule(static) nowait
@@ -185,11 +284,6 @@
         }
         FOM_PROF_END(fused);

-        /* ---- state <- su_t_t1 @ t1, in place -------------------------- */
-        FOM_PROF_BEGIN(state_update);
-        fom_apply_su_t_t1(acc, s);
-        FOM_PROF_END(state_update);
-
         work->solution.iter = iter + 1;

         if (want_res) {
@@ -204,29 +298,20 @@
             }
             FOM_PROF_END(residual);
             residual = r;
-
-#if FOM_USE_TOL
-            if (work->settings.check_termination && r <= work->settings.tol) {
-                work->solution.residual = r;
-                work->solution.status = FOM_SOLVED;
-                goto finish;
-            }
-#endif
         }
     }

+finish:
     work->solution.residual = residual;
     work->solution.status = FOM_MAX_ITER_REACHED;
-
-#if FOM_USE_TOL
-finish:
-#endif
     {
         fom_float *const dst = work->solution.x;
+        FOM_PROF_BEGIN(export_x);
 #ifdef _OPENMP
         #pragma omp parallel for schedule(static)
 #endif
         for (i = 0; i < FOM_N; ++i) dst[i] = s[i];
+        FOM_PROF_END(export_x);
     }

     return work->solution.status;
```

## Agent summary (end of the code-optimization transcript)

**Whole-solve timing** (`make bench`, 3 solves × 200 iterations, measured via `srun` on the allocated cores, interleaved baseline/final pairs to cancel node-load drift):

| | mean_us |
|---|---|
| initial | 627,868 / 629,957 (two paired runs) |
| final | 431,952 / 431,201 |
| **speedup** | **1.46×** |

(Re-measured repeatedly; the original pre-optimization reading was 627,074 and the final sits in 429,721–433,152 across all runs, so the ratio is stable at ~1.45–1.46×. An independent interleaved A/B on the contended shared node gave the same conclusion: 1,024,689 → 656,750 µs, 1.56×.)

**Final profiling breakdown** (600 region calls = 3 solves × 200 iterations):

| region | total_us | pct | per call |
|---|---:|---:|---:|
| `mv_t0_state` (`t0_state @ s`, 2.6M nnz gather) | 710,996 | 54.73% | 1185 µs |
| `mv_t1_t_t0` (`t1_t_t0 @ t0`, 2.6M nnz) | 510,415 | 39.29% | 851 µs |
| `fused` (soft-threshold `s` in place + re-seed `t0`) | 76,446 | 5.88% | 127 µs |
| `save_old` (last sweep only) | 497 | 0.04% | — |
| `export_x` (last sweep only) | 463 | 0.04% | — |
| `residual` (last sweep only) | 243 | 0.02% | — |

94% now sits in the two genuinely sparse CSR kernels of the fixed `fom_ops.c`; everything this file still owns runs at ~205 GB/s, i.e. at the memory-bandwidth floor.

**Optimizations applied**

1. **Identity-block collapse — the main win.** Inspecting the numeric data showed `t1_state` and `su_t_t1` are both 1-nnz-per-row with diagonal exactly `1.0`, i.e. `I` (and `t0_p_p0` is `-I`). The iteration therefore collapses from five matvecs to two: the `t1_state` matvec (1.3M nnz), the `su_t_t1` state-update matvec (1.3M nnz), the 1.3M-element zero-fill of the `t1` accumulator and the separate `t1 → state` copy all vanish. That removed the `state_update` (10.1%), `mv_t1_state` (5.9%) and `acc_zero` (1.4%) regions outright and shrank `fused` by ~60%.
   *Safety:* the rewrite is not taken on faith — `fom_setup` probes both operators against two pseudo-random vectors and arms the fast path only on a bit-for-bit exact match; otherwise `fom_solve_generic`, a literal SPEC transcription routing every term through its own `fom_apply_*` helper, runs instead. The probe is setup-time only, outside any timed solve.
2. **In-place accumulation into the state.** Because `t1_state == I`, the t1 accumulator *is* the previous iterate, so `fom_apply_t1_t_t0(t0, s)` accumulates straight into `state`; because `su_t_t1 == I`, the thresholded result is already the new state. No ping-pong buffer enters the cache footprint, which also keeps the hot `t0_state` gather reading one resident 1.3M-double vector.
3. **Loop-invariant parameter term hoisted** (kept from the starting version): `t0_p_p0 @ p0` depends only on `p0`, so it is computed once in `fom_update_params` and used as the per-sweep seed of the `t0` accumulator. Note this makes *no* assumption that `t0_p_p0 == -I`; it goes through the helper.
4. **Zero-λ activation elided**: `t0`'s soft-threshold has `lambd == 0.0`, which is the identity, so the accumulator already holds `t0` — no extra pass over the 325k vector.
5. **One fused OpenMP region for the remaining sweeps**: the in-place soft-threshold of `s` and the re-seed of `t0` share a single `parallel` region with `nowait` on both loops, and both use `schedule(static)`, which matches the default static row partitioning of the CSR kernels — so each thread writes the slice it will later read/own, keeping NUMA first-touch aligned with the matvec output distribution.
6. **Report-only residual deferred**: `FOM_USE_TOL == 0`, so nothing reads the residual before the loop ends; the old-state copy and the max-abs reduction run only on the final sweep (0.06% of runtime instead of ~12 full extra passes).

**Tried and rejected** (kept the faster/safer variant): clamp-form soft threshold `v - clamp(v,-λ,λ)` — fewer ALU ops but measurably *slower* here (`fused` 85.6 ms vs 76.5 ms), and the `fabs`/`copysign` form is bit-exact; thread pinning via `OMP_PROC_BIND`/`OMP_PLACES` — catastrophically slower on this shared node. The thread count was deliberately left to the environment (`OMP_NUM_THREADS=16`) rather than overridden from inside the solver.

Constraints held throughout: no dynamic allocation, no VLAs, no `/` operator, all matrix products through the `fom_apply_*` helpers, and `FOM_PROF_BEGIN/END` markers retained around every main computational region.
