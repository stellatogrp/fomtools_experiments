# Optimization report: ista / openmp (m10000_n40000_k260_sepA)

Fresh-build head-to-head on della-l07g5, mean per solve over 5 solves after 8 warm-ups:

| solver | mean_us |
|---|---:|
| Phase A | 63771.140800 |
| Phase B | 58916.582600 |

Kept: **phase_b** (Phase-A / Phase-B time = 1.082). Final verification:

```
worst max-abs error over 8 samples: 0.000e+00 (tol 1.000e-05)
```

## Diff (Phase A -> kept)

```diff
--- phase_a_solver.c	2026-10-16 09:26:07.739320000 -0400
+++ src/fom_solver.c	2026-10-16 09:43:54.721076000 -0400
@@ -2,9 +2,43 @@
  * `paper_ista_sparse_m10000_n40000_a260nnz_lambda002_sepA`.
  *
  * Embedded target: no dynamic allocation, no variable-length arrays and
- * no use of the `/` operator.  Every matrix-vector product goes through
- * the accumulating sparse helpers in fom_ops.c; all numeric constants
- * come from fom_data.h / fom_workspace.h.
+ * no use of the `/` operator.  Every *structurally* sparse matrix-vector
+ * product still goes through the accumulating CSR helpers in fom_ops.c;
+ * the three operators that the generated data shows to be exact signed
+ * identities are applied as the trivial operators they are.
+ *
+ * ---------------------------------------------------------------------
+ * Structure exploited (checked exactly against data.npz / MATRIX_STATS):
+ *
+ *   t0_p_p0   (10000x10000, 10000 nnz) : indptr = 0..n, indices = 0..n,
+ *                                        all values exactly -1  =>  -I
+ *   t1_state  (40000x40000, 40000 nnz) : indptr = 0..n, indices = 0..n,
+ *                                        all values exactly +1  =>   I
+ *   su_t_t1   (40000x40000, 40000 nnz) : indptr = 0..n, indices = 0..n,
+ *                                        all values exactly +1  =>   I
+ *
+ * so the specified iteration
+ *
+ *     t0    <- soft_{0}  ( t0_state @ s + t0_p_p0 @ p0 )
+ *     t1    <- soft_{l1} ( t1_state @ s + t1_t_t0 @ t0 )
+ *     s     <- su_t_t1 @ t1
+ *
+ * collapses to the two genuinely sparse products plus one in-place
+ * shrinkage:
+ *
+ *     t0    <- t0_state @ s - p0           (soft_{0} is the identity)
+ *     s     <- soft_{l1}( s + t1_t_t0 @ t0 )
+ *
+ * This is *bit-identical* to the general form, not merely close:
+ *   - a +-1 identity row contributes exactly (+-1.0)*x[i], which is an
+ *     exact flop, so seeding the accumulator with -p0[i] / with s[i]
+ *     reproduces the identity term exactly;
+ *   - the only reordering is `(-p0) + (A@s)` instead of `(A@s) + (-p0)`,
+ *     and IEEE-754 addition is commutative, so the sum is unchanged;
+ *   - `su_t_t1 @ t1 == t1` exactly, so the new state *is* t1 and the
+ *     copy/ping-pong of state buffers disappears entirely.
+ * `make verify` reports a worst-case error of exactly 0.000e+00.
+ * ---------------------------------------------------------------------
  */
 
 #include "fom_solver.h"
@@ -44,8 +78,10 @@
 
     work->settings = *settings;
 
-    for (i = 0; i < FOM_PARAM_P0_DIM; ++i)
+    for (i = 0; i < FOM_PARAM_P0_DIM; ++i) {
         work->params.p0[i] = (fom_float)0;
+        work->acc[i] = (fom_float)0;          /* == -p0, see below */
+    }
 
     fom_reset_state(work);
 }
@@ -53,8 +89,13 @@
 void fom_update_params(FomWorkspace *work, const FomParams *params) {
     fom_int i;
 
-    for (i = 0; i < FOM_PARAM_P0_DIM; ++i)
+    /* -p0 seeds the t0 accumulator on every sweep, so hoist the
+       negation out of the iteration and park it in the (now otherwise
+       unused) `acc` scratch slot. */
+    for (i = 0; i < FOM_PARAM_P0_DIM; ++i) {
         work->params.p0[i] = params->p0[i];
+        work->acc[i] = -params->p0[i];
+    }
 
     fom_reset_state(work);
 }
@@ -62,72 +103,65 @@
 /* ---------------------------------------------------------------- */
 
 fom_int fom_solve(FomWorkspace *work) {
-    const fom_float lam0 = FOM_T0_LAMBDA;
     const fom_float lam1 = FOM_T1_LAMBDA;
-    const fom_float *p0 = work->params.p0;
+    const fom_float *neg_p0 = work->acc;        /* == -p0, precomputed */
     const fom_int max_iter = work->settings.max_iter;
-    fom_float *state = work->state;
-    fom_float *new_state = work->new_state;
+    fom_float *state = work->state;             /* iterated in place */
+    fom_float *prev = work->new_state;          /* residual snapshot */
     fom_float *t0 = work->t0;
-    fom_float *t1 = work->t1;
-    fom_float *t1_next = work->acc;
     fom_float residual = (fom_float)0;
     fom_int iter, i;
 
-    /* The accumulating helpers require zeroed destinations.  The tmp
-       buffers are re-zeroed inside the loop as soon as they go dead, so
-       that the clearing work fuses with the element-wise passes. */
-#ifdef _OPENMP
-    #pragma omp parallel
-#endif
-    {
-#ifdef _OPENMP
-        #pragma omp for nowait
-#endif
-        for (i = 0; i < FOM_T0_DIM; ++i)
-            t0[i] = (fom_float)0;
-#ifdef _OPENMP
-        #pragma omp for nowait
-#endif
-        for (i = 0; i < FOM_T1_DIM; ++i)
-            t1[i] = (fom_float)0;
+    if (max_iter <= 0) {
+        FOM_PROF_BEGIN(state_update);
+        for (i = 0; i < FOM_STATE_DIM; ++i)
+            work->solution.x[i] = state[i];
+        FOM_PROF_END(state_update);
+        work->solution.iter = 0;
+        work->solution.residual = (fom_float)0;
+        work->solution.status = FOM_MAX_ITER_REACHED;
+        return work->solution.status;
+    }
+
+    /* Seed the t0 accumulator for the first sweep; every later sweep is
+       seeded at the tail of the preceding iteration, where the clearing
+       work fuses with the shrinkage pass. */
 #ifdef _OPENMP
-        #pragma omp for nowait
+    #pragma omp parallel for
 #endif
-        for (i = 0; i < FOM_T1_DIM; ++i)
-            t1_next[i] = (fom_float)0;
-    }
+    for (i = 0; i < FOM_T0_DIM; ++i)
+        t0[i] = neg_p0[i];
 
     for (iter = 0; iter < max_iter; ++iter) {
-        /* ---- t0 = soft_threshold(t0_state @ state + t0_p_p0 @ p0) ---- */
+        /* ---- t0 = t0_state @ state - p0   (soft_threshold, lam = 0,
+               i.e. the identity activation: nothing to apply) ---- */
         FOM_PROF_BEGIN(t0_linop);
         fom_apply_t0_state(state, t0);
-        fom_apply_t0_p_p0(p0, t0);
         FOM_PROF_END(t0_linop);
 
-        if (lam0 > (fom_float)0) {
-            FOM_PROF_BEGIN(t0_act);
+        /* The residual never drives termination here (tol is disabled),
+           so only the final iterate's residual has to be formed: take
+           the one snapshot of the previous state that needs. */
+        if (iter + 1 == max_iter) {
+            FOM_PROF_BEGIN(residual_snapshot);
 #ifdef _OPENMP
             #pragma omp parallel for
 #endif
-            for (i = 0; i < FOM_T0_DIM; ++i) {
-                fom_float v = t0[i];
-                if (v > lam0)       v -= lam0;
-                else if (v < -lam0) v += lam0;
-                else                v = (fom_float)0;
-                t0[i] = v;
-            }
-            FOM_PROF_END(t0_act);
+            for (i = 0; i < FOM_STATE_DIM; ++i)
+                prev[i] = state[i];
+            FOM_PROF_END(residual_snapshot);
         }
 
-        /* ---- t1 = soft_threshold(t1_state @ state + t1_t_t0 @ t0) ---- */
+        /* ---- state += t1_t_t0 @ t0   (t1_state == I, so the identity
+               term is already in place: accumulate straight into the
+               state, no second buffer and no copy) ---- */
         FOM_PROF_BEGIN(t1_linop);
-        fom_apply_t1_state(state, t1);
-        fom_apply_t1_t_t0(t0, t1);
+        fom_apply_t1_t_t0(t0, state);
         FOM_PROF_END(t1_linop);
 
-        /* t0 is dead now: threshold t1, clear the state-update target
-           and re-zero t0 for the next sweep in a single pass. */
+        /* t0 is dead now.  Shrink the new iterate (su_t_t1 == I, so it
+           *is* the next state) and re-seed t0 with -p0 for the next
+           sweep inside a single parallel region / single barrier. */
         FOM_PROF_BEGIN(t1_act);
 #ifdef _OPENMP
         #pragma omp parallel
@@ -137,68 +171,37 @@
             #pragma omp for nowait
 #endif
             for (i = 0; i < FOM_T1_DIM; ++i) {
-                fom_float v = t1[i];
+                fom_float v = state[i];
                 if (v > lam1)       v -= lam1;
                 else if (v < -lam1) v += lam1;
                 else                v = (fom_float)0;
-                t1[i] = v;
-                new_state[i] = (fom_float)0;
-                t1_next[i] = (fom_float)0;
+                state[i] = v;
             }
 #ifdef _OPENMP
             #pragma omp for nowait
 #endif
             for (i = 0; i < FOM_T0_DIM; ++i)
-                t0[i] = (fom_float)0;
+                t0[i] = neg_p0[i];
         }
         FOM_PROF_END(t1_act);
 
-        /* ---- state <- su_t_t1 @ tmp:t1 ---- */
-        FOM_PROF_BEGIN(state_update);
-        fom_apply_su_t_t1(t1, new_state);
-
-        /* t1 is dead now.  The residual never drives termination here
-           (tol is disabled), so only its final value has to be formed;
-           fold that pass into the re-zeroing of t1. */
-        if (iter + 1 == max_iter) {
-            residual = (fom_float)0;
-#ifdef _OPENMP
-            #pragma omp parallel for reduction(max:residual)
-#endif
-            for (i = 0; i < FOM_STATE_DIM; ++i) {
-                fom_float d = new_state[i] - state[i];
-                if (d < (fom_float)0) d = -d;
-                if (d > residual) residual = d;
-            }
-        }
-
-        /* Swap the buffers rather than copying. */
-        {
-            fom_float *tmp = state;
-            state = new_state;
-            new_state = tmp;
-            tmp = t1;
-            t1 = t1_next;
-            t1_next = tmp;
-        }
-        FOM_PROF_END(state_update);
+        /* ---- state <- su_t_t1 @ t1 : su_t_t1 == I, so the state
+               update is a no-op on the buffer we already hold. ---- */
     }
 
-#ifdef _OPENMP
-    #pragma omp parallel for
-#endif
-    for (i = 0; i < FOM_STATE_DIM; ++i)
+    /* Single closing sweep: residual reduction and the solution copy
+       share one pass over the state. */
+    FOM_PROF_BEGIN(state_update);
+#ifdef _OPENMP
+    #pragma omp parallel for reduction(max:residual)
+#endif
+    for (i = 0; i < FOM_STATE_DIM; ++i) {
+        fom_float d = state[i] - prev[i];
+        if (d < (fom_float)0) d = -d;
+        if (d > residual) residual = d;
         work->solution.x[i] = state[i];
-
-    /* Leave work->state holding the final iterate regardless of which
-       buffer the swapping ended on. */
-    if (state != work->state) {
-#ifdef _OPENMP
-        #pragma omp parallel for
-#endif
-        for (i = 0; i < FOM_STATE_DIM; ++i)
-            work->state[i] = state[i];
     }
+    FOM_PROF_END(state_update);
 
     work->solution.iter = max_iter;
     work->solution.residual = residual;
```

## Agent notes (end of the Phase-B transcript)

```
Deleting the entire elementwise pass (an intentionally incorrect probe build) moved the solve only from 58.1 to 57.5 ms — i.e. **~99% of the remaining runtime is inside the two fixed CSR kernels in `fom_ops.c`**. Those run at ~2.8 cycles/nnz (1.04 G nnz per solve, 16 threads, ~3.1 GHz), which matches the latency of the serial `s += val*x[col]` accumulation chain in the CSR inner loop (addsd latency ≈ 3 cycles, and `-O3` without `-ffast-math` may not reassociate). They are therefore FP-latency-bound, not bandwidth-bound, and nothing a caller can do — buffer alignment, NUMA placement, access order — affects them. The solver-side headroom left is ~1%, so I stopped here.

FOMTOOLS-SUMMARY-END
Final state verified (`VERIFY OK`, worst error exactly `0.000e+00`) and benchmarked. Here's the wrap-up.

FOMTOOLS-SUMMARY-BEGIN

## Whole-solve timing (`make bench`, BENCH_SOLVES=10, 16 OpenMP threads)

| | `mean_us` | `min_us` |
|---|---:|---:|
| initial | 62,300 (4 runs: 62,051 / 62,079 / 61,283 / 63,752) | 60,800–61,400 |
| final | 58,000 (4 runs: 57,695 / 60,080 / 59,202 / 58,627; later runs 57,894 / 57,684) | 56,590–58,050 |
| **speedup** | **≈ 1.07×** | **≈ 1.07×** |

Measurement note: `make bench` dispatches through an `srun` "holder" onto a dedicated compute node; running `./bench_bin` directly from the shell gives ~125,000 µs on the shared node. All numbers above are from `make bench`. Run-to-run spread is ±2–3%, so every change was A/B'd in alternating rounds against the previous best rather than measured once. Correctness was re-checked after every accepted change; the final solver is **bit-identical** to the original (verify error exactly 0.0, not merely within the 1e-5 tolerance).

## Final profiling breakdown (`make profile`, 5 solves, 1000 iterations)

| region | total_us | pct | what it is |
|---|---:|---:|---|
| `t0_linop` | 173,574 | 54.12% | `fom_apply_t0_state` — CSR 10000×40000, 2,599,983 nnz (260/row) |
| `t1_linop` | 137,973 | 43.02% | `fom_apply_t1_t_t0` — CSR 40000×10000, 2,599,983 nnz (65/row) |
| `t1_act` | 9,125 | 2.85% | soft-threshold of the new iterate + re-seeding `t0` with `-p0` |
| `state_update` | 32 | 0.01% | closing pass: residual reduction + solution copy (once per solve) |
| `residual_snapshot` | 25 | 0.01% | previous-state snapshot, taken only on the last iteration |

## Optimizations applied

1. **`su_t_t1 == I` → the state update disappears entirely.** `data.npz` shows `su_t_t1` is 40000×40000 with exactly 40000 nnz, `indptr = indices = 0..n`, all values exactly `+1`. So `state <- su_t_t1 @ t1` *is* `t1`. Removed the 40000-row CSR call, the `new_state` zeroing pass and the state/`t1`/`t1_next` ping-pong buffers. (`state_update` went from 2.00% of profiled time to 0.01%.)

2. **`t1_state == I` → accumulate the `t1` linear op in place.** Same verified structure (40000 nnz, diagonal, values exactly `+1`), so `t1 = t1_state @ state + t1_t_t0 @ t0` is just `state += t1_t_t0 @ t0`. Since the helpers are *accumulating*, the identity term is already present in the destination — dropped the identity matvec, the 320 KB zero-fill of the destination and the 320 KB copy.

3. **`t0_p_p0 == -I` → seed the accumulator instead of multiplying.** Verified 10000 nnz, diagonal, all values exactly `-1`, so `t0_p_p0 @ p0 == -p0`. The accumulating helper needs its destination pre-filled anyway, so seeding `t0[i] = -p0[i]` replaces the zero-fill at identical cost and removes a whole 10000-row CSR call.

   All three are **exact** rewrites: a ±1 diagonal row contributes exactly `(±1.0)*x[i]` (an exact flop), and the only reordering introduced is `(-p0) + (A@s)` instead of `(A@s) + (-p0)`, which IEEE-754 addition commutes. Hence the 0.0 verify error.

4. **`soft_threshold(·, λ=0)` on `t0` is the identity** (spec `lambd: 0.0`) — the activation is gone rather than guarded by a runtime `if`.

5. **Loop-invariant hoisting into `fom_update_params`.** `-p0` is parameter-, not iteration-dependent, so the negation is computed once per parameter set into the now-free `acc` scratch slot (no new storage, no allocation).

6. **Pass fusion / barrier reduction.** The shrinkage of the new iterate and the re-seeding of `t0` for the next sweep live in one `#pragma omp parallel` with two `nowait` work-sharing loops → one barrier instead of two. OpenMP regions per iteration dropped from 6 to 3 (and the three remaining are the two fixed CSR kernels plus this one).

7. **Residual computed only where it is observable.** `tol` is disabled (`FOM_USE_TOL 0`, termination is pure `max_iters`), so only the final iterate's residual is meaningful: the previous-state snapshot is taken on the last iteration only, and the residual reduction is fused with the `solution.x` copy into a single closing pass over the state.

## Things measured and rejected

- **Branchless shrinkage** (`copysign(max(|v|-λ,0), v)`, vectorizable under SSE2): consistently *slower* (59.1–61.0 vs 58.1–59.0 ms over 3 alternating rounds) — the three-way branch is perfectly predicted here because λ = 2.94e-4 never actually zeroes an entry. Reverted.
- **Static, page-aligned, parallel-first-touched scratch buffers** for `state`/`t0` instead of the stack-resident workspace: slower (min 59.1 vs 56.8 ms). Reverted.
- **Early exit on an exact fixed point**: instrumented the per-iteration `max|Δstate|`; it is still 6.0e-3 at iteration 199 (no stalling), and the iterate stays fully dense (nnz = 40000/40000 — λ never zeroes a coordinate), so there is neither an early-exit nor an iterate-sparsity structure to exploit.
- **Thread/affinity tuning**: probed from inside the holder — 16 threads pinned 1:1 onto 16 distinct cores, `omp_get_num_procs() == 16`, so the allocation is already saturated and `OMP_NUM_THREADS` was left as the harness set it.

## Where the remaining time goes

Deleting the entire elementwise pass (an intentionally incorrect probe build) moved the solve only from 58.1 to 57.5 ms — i.e. **~99% of the remaining runtime is inside the two fixed CSR kernels in `fom_ops.c`**. Those run at ~2.8 cycles/nnz (1.04 G nnz per solve, 16 threads, ~3.1 GHz), which matches the latency of the serial `s += val*x[col]` accumulation chain in the CSR inner loop (addsd latency ≈ 3 cycles, and `-O3` without `-ffast-math` may not reassociate). They are therefore FP-latency-bound, not bandwidth-bound, and nothing a caller can do — buffer alignment, NUMA placement, access order — affects them. The solver-side headroom left is ~1%, so I stopped here.

FOMTOOLS-SUMMARY-END./verify_bin
worst max-abs error over 8 samples: 0.000e+00 (tol 1.000e-05)
VERIFY OK
[fom-step in holder 15685512 on della-l07g5]
```
