# Optimization report: fista / openmp (m325000_n1300000_k8, tolerance 1e-5)

Fresh-build head-to-head on della-i12g2, mean per solve over 5 solves after 8 warm-ups:

| solver | mean_us |
|---|---:|
| Phase A | 5827797.783200 |
| Phase B | 1254083.481000 |

Kept: **phase_b** (Phase-A / Phase-B time = 4.647). Final verification:

```
worst max-abs error over 8 samples: 1.403e-06 (tol 1.000e-05)
```

## Diff (Phase A -> kept)

```diff
--- phase_a_solver.c	2026-10-15 16:12:32.699543000 -0400
+++ src/fom_solver.c	2026-10-15 16:40:41.797991000 -0400
@@ -6,9 +6,42 @@
  *                            + t0_p_p0 @ p0,  lambda )
  *   x_new <- su_state @ x + su_t_t0 @ t0
  *
- * All matrix products go through the accumulating CSR helpers in fom_ops.c;
- * all constants come from fom_data.h.  No dynamic memory, no VLAs, and no
- * `/` operator anywhere.
+ * ---------------------------------------------------------------------------
+ * Structure exploited (all verified against data.npz / src/fom_ops.c):
+ *
+ *   state = [s0 ; s1], each of length N = FOM_T0_DIM = 1300000.
+ *
+ *   su_state  = [[0, I], [0, 0]]   (1300000 unit entries, row i -> col N+i)
+ *   su_t_t0   = [[0], [I]]         (1300000 unit entries, row N+i -> col i)
+ *       => the state update is a pure shift:  s0' = s1,  s1' = t0.
+ *          No matvec, no 2.6M-long zeroing, no copy: three 1300000-long
+ *          buffers rotated by pointer swap carry the whole recursion.
+ *
+ *   t0_state            = [0 | M]    (the emitted CSR holds only the 19499968
+ *                                     right-half entries; the left half is
+ *                                     structurally absent)
+ *   t0_state__d_gamma   = [-M | M]   (38999936 entries: the two halves share
+ *                                     the column pattern and differ only in
+ *                                     sign, to 1.3e-15 absolute)
+ *       => t0_state @ state + g * (t0_state__d_gamma @ state)
+ *            = M s1 + g (M s1 - M s0)
+ *            = M ( (1+g) s1 - g s0 )
+ *          i.e. the classic FISTA extrapolation.  One 19499968-nnz product
+ *          against a 1300000-long vector replaces two products totalling
+ *          58499904 nnz against 2600000-long vectors -- 3x less matrix
+ *          traffic and half the gather footprint (the gathered vector now
+ *          fits far better in cache).
+ *
+ *   The single remaining product is still issued through the generated
+ *   helper fom_apply_t0_state(); the extrapolated vector is simply placed in
+ *   the upper half of a 2600000-long buffer, which is exactly the half that
+ *   helper reads.
+ *
+ *   t0_p_p0 @ p0 depends only on the parameter, so it is computed once in
+ *   fom_update_params / fom_setup and cached in workspace scratch.
+ *
+ * No dynamic memory, no VLAs, and no `/` operator anywhere.
+ * ---------------------------------------------------------------------------
  */

 #include "fom_solver.h"
@@ -18,7 +51,25 @@
 #include "fom_profile.h"

 #define FOM_N_STATE  FOM_STATE_DIM
-#define FOM_N_T0     FOM_T0_DIM
+#define FOM_N        FOM_T0_DIM          /* half the state = t0 dimension */
+
+/* ------------------------------------------------------------------ */
+/* workspace scratch layout                                            */
+/*                                                                     */
+/*   acc[0 .. N)            pv   : t0_p_p0 @ p0, cached across solves  */
+/*   acc[N .. 2N)           yv   : extrapolated vector; the CSR helper */
+/*                                 is handed `acc` and only ever reads */
+/*                                 columns >= N, i.e. exactly yv       */
+/*   new_state[0 .. N)      b0   : rotation buffer  (x_{k-1})          */
+/*   new_state[N .. 2N)     b1   : rotation buffer  (x_k)              */
+/*   t0[0 .. N)             b2   : rotation buffer  (x_{k+1} / rhs)    */
+/*   state[0 .. 2N)         live-in / live-out state vector            */
+/* ------------------------------------------------------------------ */
+#define FOM_PV(w)   ((w)->acc)
+#define FOM_YBASE(w) ((w)->acc)              /* yv = FOM_YBASE + FOM_N */
+#define FOM_B0(w)   ((w)->new_state)
+#define FOM_B1(w)   ((w)->new_state + FOM_N)
+#define FOM_B2(w)   ((w)->t0)

 /* ------------------------------------------------------------------ */
 /* small parallel vector kernels                                       */
@@ -45,6 +96,15 @@
     work->solution.status = FOM_MAX_ITER_REACHED;
 }

+/* pv <- t0_p_p0 @ p0.  Iteration- and solve-invariant: only the parameter
+   feeds it, so it is refreshed exactly when the parameter changes. */
+static void fom_cache_param_term(FomWorkspace *work)
+{
+    fom_float *const restrict pv = FOM_PV(work);
+    fom_zero(pv, FOM_N);
+    fom_apply_t0_p_p0(work->params.p0, pv);
+}
+
 /* ------------------------------------------------------------------ */
 /* ABI                                                                 */
 /* ------------------------------------------------------------------ */
@@ -67,6 +127,7 @@

     fom_reset_state(work);
     fom_zero(work->solution.x, FOM_N_STATE);
+    fom_cache_param_term(work);          /* p0 == 0  ->  pv == 0 */
 }

 void fom_update_params(FomWorkspace *work, const FomParams *params)
@@ -79,6 +140,7 @@
     for (i = 0; i < FOM_PARAM_P0_DIM; ++i) dst[i] = src[i];

     fom_reset_state(work);
+    fom_cache_param_term(work);
 }

 fom_int fom_solve(FomWorkspace *work)
@@ -86,132 +148,137 @@
     const fom_int max_iter = work->settings.max_iter;
     const fom_float lam = FOM_T0_LAMBDA;

-    /* t0-sized scratch: acc is FOM_MAX_VEC_DIM (= 2 * FOM_T0_DIM), so its two
-       halves give us the gamma-term buffer and a cache of the (iteration
-       independent) parameter term.  The halves never overlap. */
-    fom_float *const restrict tv = work->t0;                   /* t0 step    */
-    fom_float *const restrict gv = work->acc;                  /* D @ x      */
-    fom_float *const restrict pv = work->acc + FOM_N_T0;       /* P @ p0     */
-
-    fom_float *cur = work->state;      /* ping-pong: avoids a 2.6M copy/iter */
-    fom_float *nxt = work->new_state;
+    const fom_float *const restrict pv = FOM_PV(work);
+    fom_float *const restrict ybase = FOM_YBASE(work);
+    fom_float *const restrict yv = ybase + FOM_N;
+
+    /* three rotating half-state buffers: x_{k-1}, x_k, x_{k+1} */
+    fom_float *xprev = FOM_B0(work);
+    fom_float *xcur  = FOM_B1(work);
+    fom_float *xnew  = FOM_B2(work);

     fom_float resid = (fom_float)0.0;
     fom_int it, i;

-    /* t0_p_p0 @ p0 is constant over the whole solve: hoist it out. */
-    fom_zero(pv, FOM_N_T0);
-    fom_apply_t0_p_p0(work->params.p0, pv);
+    /* load the live-in state: s0 -> x_{k-1}, s1 -> x_k */
+    {
+        const fom_float *const restrict st = work->state;
+        fom_float *const restrict p = xprev;
+        fom_float *const restrict c = xcur;
+#pragma omp parallel for schedule(static)
+        for (i = 0; i < FOM_N; ++i) {
+            p[i] = st[i];
+            c[i] = st[i + FOM_N];
+        }
+    }
+
+    /* y_0 = (1 + g_0) x_0 - g_0 x_{-1} */
+    {
+        const fom_float g = fom_sched_gamma(0);
+        const fom_float *const restrict p = xprev;
+        const fom_float *const restrict c = xcur;
+        fom_float *const restrict o = xnew;
+#pragma omp parallel for schedule(static)
+        for (i = 0; i < FOM_N; ++i) {
+            yv[i] = c[i] + g * (c[i] - p[i]);
+            o[i] = pv[i];                /* seed the first rhs */
+        }
+    }

     for (it = 0; it < max_iter; ++it) {
-        const fom_float g = fom_sched_gamma(it);
+        const fom_float gn = fom_sched_gamma(it + 1);  /* next extrapolation */
+        const fom_int last = (it + 1 >= max_iter);

-        /* ---- t0 = soft_threshold(linear_op, lambda) ------------------ */
+        /* ---- rhs = M @ y + pv --------------------------------------- *
+         * The helper accumulates into its output, so the buffer was
+         * pre-seeded with the cached parameter term pv (by the previous
+         * iteration's fused sweep, or by the prologue) rather than being
+         * zeroed and having pv added in a sweep of its own.             */
         FOM_PROF_BEGIN(mv_t0_state);
-        fom_zero(tv, FOM_N_T0);
-        fom_apply_t0_state(cur, tv);
+        fom_apply_t0_state(ybase, xnew);
         FOM_PROF_END(mv_t0_state);

-        if (g != (fom_float)0.0) {
-            FOM_PROF_BEGIN(mv_dgamma);
-            fom_zero(gv, FOM_N_T0);
-            fom_apply_t0_state__d_gamma(cur, gv);
-            FOM_PROF_END(mv_dgamma);
-            FOM_PROF_BEGIN(prox);
-
+        /* ---- prox + extrapolation, fused ---------------------------- *
+         * x_{k+1} = soft_threshold(rhs, lambda); in the same pass over
+         * the data, y_{k+1} = x_{k+1} + g' (x_{k+1} - x_k) (the stable
+         * form of (1+g') x_{k+1} - g' x_k), and pv is laid into the
+         * buffer that the rotation turns into the next rhs.  Fusing the
+         * three saves two extra sweeps of a 1300000-long vector per
+         * iteration.                                                     */
+        FOM_PROF_BEGIN(prox);
+        if (!last) {
+            fom_float *const restrict o = xnew;
+            const fom_float *const restrict c = xcur;
+            fom_float *const restrict y = yv;
+            fom_float *const restrict nxt = xprev;   /* becomes the next rhs */
 #pragma omp parallel for schedule(static)
-            for (i = 0; i < FOM_N_T0; ++i) {
-                fom_float v = tv[i] + g * gv[i] + pv[i];
+            for (i = 0; i < FOM_N; ++i) {
+                fom_float v = o[i];
                 fom_float a = (v < (fom_float)0.0) ? -v : v;
+                fom_float t;
                 a -= lam;
-                if (a <= (fom_float)0.0) tv[i] = (fom_float)0.0;
-                else tv[i] = (v < (fom_float)0.0) ? -a : a;
+                if (a <= (fom_float)0.0) t = (fom_float)0.0;
+                else t = (v < (fom_float)0.0) ? -a : a;
+                o[i] = t;
+                y[i] = t + gn * (t - c[i]);
+                nxt[i] = pv[i];          /* seed the next iteration's rhs */
             }
-            FOM_PROF_END(prox);
         } else {
-            /* gamma == 0: the scaled term contributes exactly zero. */
-#pragma omp parallel for schedule(static)
-            for (i = 0; i < FOM_N_T0; ++i) {
-                fom_float v = tv[i] + pv[i];
-                fom_float a = (v < (fom_float)0.0) ? -v : v;
-                a -= lam;
-                if (a <= (fom_float)0.0) tv[i] = (fom_float)0.0;
-                else tv[i] = (v < (fom_float)0.0) ? -a : a;
-            }
-        }
-
-        /* ---- state update -------------------------------------------- */
-        FOM_PROF_BEGIN(state_update);
-        fom_zero(nxt, FOM_N_STATE);
-        fom_apply_su_state(cur, nxt);
-        fom_apply_su_t_t0(tv, nxt);
-        FOM_PROF_END(state_update);
-
-        /* ---- residual ------------------------------------------------- */
-#if FOM_USE_TOL
-        {
-            fom_float m = (fom_float)0.0;
-#pragma omp parallel for schedule(static) reduction(max : m)
-            for (i = 0; i < FOM_N_STATE; ++i) {
-                fom_float d = nxt[i] - cur[i];
-                if (d < (fom_float)0.0) d = -d;
-                if (d > m) m = d;
-            }
-            resid = m;
-        }
-#else
-        /* No tolerance test is configured, so only the final iteration's
-           residual is ever observable; skip the sweep on the others. */
-        if (it + 1 >= max_iter) {
+            /* final iteration: no next y is needed, but the residual
+               max_abs(new_state - state) becomes observable. */
+            fom_float *const restrict o = xnew;
+            const fom_float *const restrict c = xcur;
+            const fom_float *const restrict p = xprev;
             fom_float m = (fom_float)0.0;
 #pragma omp parallel for schedule(static) reduction(max : m)
-            for (i = 0; i < FOM_N_STATE; ++i) {
-                fom_float d = nxt[i] - cur[i];
-                if (d < (fom_float)0.0) d = -d;
-                if (d > m) m = d;
+            for (i = 0; i < FOM_N; ++i) {
+                fom_float v = o[i];
+                fom_float a = (v < (fom_float)0.0) ? -v : v;
+                fom_float t, d0, d1;
+                a -= lam;
+                if (a <= (fom_float)0.0) t = (fom_float)0.0;
+                else t = (v < (fom_float)0.0) ? -a : a;
+                o[i] = t;
+                /* new_state = [x_k ; x_{k+1}], state = [x_{k-1} ; x_k] */
+                d0 = c[i] - p[i];
+                if (d0 < (fom_float)0.0) d0 = -d0;
+                d1 = t - c[i];
+                if (d1 < (fom_float)0.0) d1 = -d1;
+                if (d0 > m) m = d0;
+                if (d1 > m) m = d1;
             }
             resid = m;
         }
-#endif
+        FOM_PROF_END(prox);

+        /* ---- state update: pure shift, done by rotating pointers ----- */
         {
-            fom_float *swap = cur;
-            cur = nxt;
-            nxt = swap;
+            fom_float *swap = xprev;
+            xprev = xcur;
+            xcur = xnew;
+            xnew = swap;
         }
-
-#if FOM_USE_TOL
-        if (work->settings.check_termination && resid <= work->settings.tol) {
-            ++it;
-            work->solution.status = FOM_SOLVED;
-            break;
-        }
-#endif
     }

-    /* ---- publish ------------------------------------------------------ */
+    /* ---- publish: state = solution.x = [x_{k-1} ; x_k] ---------------- */
+    FOM_PROF_BEGIN(publish);
     {
         fom_float *const restrict x = work->solution.x;
         fom_float *const restrict st = work->state;
-        if (cur == st) {
+        const fom_float *const restrict p = xprev;
+        const fom_float *const restrict c = xcur;
 #pragma omp parallel for schedule(static)
-            for (i = 0; i < FOM_N_STATE; ++i) x[i] = st[i];
-        } else {
-            const fom_float *const restrict c = cur;
-#pragma omp parallel for schedule(static)
-            for (i = 0; i < FOM_N_STATE; ++i) {
-                fom_float v = c[i];
-                st[i] = v;
-                x[i] = v;
-            }
+        for (i = 0; i < FOM_N; ++i) {
+            fom_float a = p[i], b = c[i];
+            st[i] = a;          x[i] = a;
+            st[i + FOM_N] = b;  x[i + FOM_N] = b;
         }
     }
+    FOM_PROF_END(publish);

     work->solution.iter = it;
     work->solution.residual = resid;
-#if !FOM_USE_TOL
     work->solution.status = FOM_MAX_ITER_REACHED;
-#endif
     return work->solution.status;
 }

```

## Agent summary (end of the code-optimization transcript)

### Result

| | whole-solve `mean_us` |
|---|---|
| initial | 5,757,653 |
| final | 1,251,609 |
| **speedup** | **4.60×** |

`make verify` → `VERIFY OK`, worst max-abs error 1.403e-06 against tol 1.0e-05 (the untouched baseline solver itself sits at 2.152e-07, so the reformulation costs ~6× of a 1e-9-scale margin and still clears tolerance by 7×). Still allocation-free, VLA-free, and the `/` operator appears nowhere outside comments.

### Final profile (`make profile`, 3 solves, 200 iterations each)

| region | total_us | pct | per call |
|---|---:|---:|---:|
| `mv_t0_state` | 3,274,653 | 86.68 % | 5,458 µs |
| `prox` | 501,119 | 13.26 % | 835 µs |
| `publish` | 2,258 | 0.06 % | 753 µs (3 calls) |

The one remaining matvec moves ~239 MB of CSR data plus 19.5 M random gathers per call at ~48 GB/s; the fused vector sweep runs at ~112 GB/s of actual DRAM traffic (counting read-for-ownership). Both are at the streaming limit for the 16 threads this job is given, so the loop is memory-bound in code I am not allowed to change.

### Optimizations applied

1. **Collapsed the two state matvecs into one (the dominant win, ~4.3× on its own).**
   Structure: `t0_state` is the block row `[0 | M]` — the emitted CSR in `fom_ops.c` holds only the 19,499,968 right-half entries (the IR's 10,460,178 left-half values are all ≤1.1e-16 and were pruned by codegen) — while `t0_state__d_gamma` is `[-M | M]`: I checked in `data.npz` that its two halves share the column pattern exactly and their values sum to ≤1.3e-15, and that its right half matches `t0_state` to 2.3e-15 relative. Therefore
   `t0_state@s + γ·(t0_state__d_gamma@s) = M·s1 + γ(M·s1 − M·s0) = M·((1+γ)s1 − γ·s0)`
   — plain FISTA extrapolation. One 19.5 M-nnz product against a 1.3 M-long vector replaces two products totalling 58.5 M nnz against 2.6 M-long vectors: 3× less matrix traffic and half the gather footprint. The product still goes through the generated helper — the extrapolated vector is placed in the upper half of a 2.6 M-long buffer, which is exactly the half `fom_apply_t0_state` reads.

2. **Deleted the state-update matvecs entirely.**
   Structure: `su_state` is `[[0,I],[0,0]]` and `su_t_t0` is `[[0],[I]]` (1.3 M unit entries each, verified as exact permutations), i.e. the update is the pure shift `s0' = s1, s1' = t0`. Replaced with a pointer rotation over three 1.3 M-long buffers — this removes two CSR calls, a 2.6 M-long zeroing, and a 2.6 M-long copy per iteration. This is what took `state_update` from 7.7 % of the old profile to zero.

3. **Hoisted `t0_p_p0 @ p0` out of the solve.**
   Structure: that term depends only on the parameter, not on the state or the iteration. It is now computed once in `fom_setup` / `fom_update_params` and cached in workspace scratch (`acc[0..N)`).

4. **Seeded the matvec output with the cached parameter term instead of zeroing it.**
   The generated helpers accumulate (`out += M@x`), so initialising the output to `pv` folds the `+pv` of the linear op into a pass that had to happen anyway, removing a separate add sweep.

5. **Fused prox + extrapolation + next-rhs seeding into one sweep** (~1.7 %).
   `x_{k+1} = soft(rhs)`, `y_{k+1} = x_{k+1} + γ'(x_{k+1} − x_k)`, and the `pv` seed for the buffer that the rotation turns into the next rhs all happen in a single parallel loop over the 1.3 M-long vectors, cutting two loop traversals and two OpenMP region launches per iteration. The extrapolation uses the differencing form `x + γ(x − x_prev)` rather than `(1+γ)x − γx_prev` to avoid cancellation when the iterates are close.

6. **Residual computed only on the final iteration** (kept from the baseline). `FOM_USE_TOL` is 0, so no intermediate residual is observable; the last-iteration sweep is fused into that iteration's prox loop and exploits `new_state − state = [x_k − x_{k-1}; x_{k+1} − x_k]`, which the three live buffers already hold.

### Things measured and rejected

- **Thread pinning** (`OMP_PROC_BIND=close|spread`): 1.65× *slower* (2.07 s). Unbound threads spread across this 8-NUMA-node EPYC 7H12 and draw on several memory controllers; confining them to one node's bandwidth is a large loss. Left unbound.
- **Huge pages for the gathered vector**: `/sys/kernel/mm/transparent_hugepage/enabled` is already `always`, so the `.bss`/stack workspace gets them for free; no `madvise` needed.
- **Buffer-layout shuffling** (separating the gathered vector and the matvec output into different arrays): within run-to-run noise (±0.4 %). Kept the clearer layout.
- **Factoring `M = I − AᵀA/L`** to do `y − (Aᵀ/L)(Ay)` (5.2 M nnz instead of 19.5 M): not possible — `fom_ops.c` exposes `Aᵀ/L` (as `t0_p_p0`) but no transpose-apply for `A`, and the CSR data is file-static.
- **Raising the OpenMP thread count** from the environment's 16: deliberately not done. The node has 128 cores with no cpuset restriction, so it would "win" by taking CPUs the job was not allocated rather than by improving the solver.
