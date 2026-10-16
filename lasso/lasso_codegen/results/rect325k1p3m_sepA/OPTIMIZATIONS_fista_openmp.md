# Optimization report: fista / openmp (m325000_n1300000_k8, D and D^T separate)

Fresh-build head-to-head on della-i12g1, mean per solve over 5 solves after 8 warm-ups:

| solver | mean_us |
|---|---:|
| Phase A | 1924511.963600 |
| Phase B | 549692.465000 |

Kept: **phase_b** (Phase-A / Phase-B time = 3.501). Final verification:

```
worst max-abs error over 8 samples: 7.913e-07 (tol 1.000e-05)
```

## Diff (Phase A -> kept)

```diff
--- phase_a_solver.c	2026-10-16 07:19:48.278236000 -0400
+++ src/fom_solver.c	2026-10-16 07:52:07.563003000 -0400
@@ -2,14 +2,46 @@
  *   paper_fista_sparse_m325000_n1300000_a8nnz_lambda002_sepA
  *
  * Fixed-iteration first-order method.  Allocation-free, division-free;
- * every matrix-vector product goes through the accumulating CSR helpers
- * in fom_ops.c and every scratch vector lives in the static workspace.
+ * every scratch vector lives in the static workspace.
  *
- * The method is memory-bound, so the iteration body is written to touch
- * as few vectors as possible: each `gamma`-scaled term is accumulated
- * into its own destination and scaled in place (rather than staged in a
- * second buffer), and the constant `t0_p_p0 @ p0` term is hoisted out of
- * the loop.
+ * ---------------------------------------------------------------------
+ * Block structure of the generated operators (verified numerically
+ * against fom_ops.c; see OPTIMIZATIONS.md).  With the state split as
+ * s = [s0; s1], each of dimension FOM_HALF = FOM_STATE_DIM/2:
+ *
+ *   t0_state            = [   0  | -A ]      (8 nnz per row, right block)
+ *   t0_state__d_gamma   = [   A  | -A ]
+ *   t0_p_p0             = -I                 (exactly -1 on the diagonal)
+ *   t1_state            = [   0  |  I ]      (diagonal, unit)
+ *   t1_state__d_gamma   = [  -I  |  I ]      (diagonal, unit)
+ *   su_state            = [[0, I], [0, 0]]   (exact unit shift)
+ *   su_t_t1             = [[0], [I]]         (exact unit injection)
+ *
+ * so one iteration collapses to the FISTA kernel
+ *
+ *   w  = (1+gamma) * s1 - gamma * s0          (extrapolation)
+ *   t0 = -A w - p0                            (one sparse matvec)
+ *   t1 = soft(w + B t0, lambda)               (one sparse matvec)
+ *   s0 <- s1,  s1 <- t1                       (pointer rotation)
+ *
+ * The three diagonal/shift operators (t1_state, t1_state__d_gamma,
+ * su_state, su_t_t1, t0_p_p0) therefore need no CSR traversal at all,
+ * and the two *_d_gamma matvecs on the t0/t1 rows fold into the single
+ * extrapolated vector w.  That removes 6 of the 8 sparse applies and
+ * all 2*FOM_STATE_DIM-sized vector passes from the loop; only
+ *
+ *   t0_state   (A,   2.6e6 nnz)   and   t1_t_t0   (B, 2.6e6 nnz)
+ *
+ * still carry real numeric data, and both are still applied through the
+ * accumulating CSR helpers of fom_ops.c -- no matrix data is duplicated
+ * here.  `A` acts on the *upper* half of a state-sized vector, so w is
+ * held in the upper half of a workspace buffer and the helper is handed
+ * the base of that buffer; the lower half is never referenced (the
+ * block is structurally empty).
+ *
+ * The iterates rotate through the upper halves of work->state,
+ * work->new_state and work->acc, so the state update is free.
+ * ---------------------------------------------------------------------
  */

 #include <string.h>
@@ -20,25 +52,7 @@
 #include "fom_linsolve.h"
 #include "fom_profile.h"

-/* work->acc (FOM_MAX_VEC_DIM doubles) holds the loop-invariant
-   parameter contribution t0_p_p0 @ param:p0 (FOM_T0_DIM doubles). */
-
-/* Parallel zero fill; memset lets the C library use non-temporal
-   stores, which skips the read-for-ownership of the destination. */
-static void fom_zero(fom_float *v, fom_int n) {
-#ifdef _OPENMP
-    const fom_int chunk = (n + 1023) >> 10; /* ~1024 blocks, division-free */
-    fom_int b;
-#pragma omp parallel for schedule(static)
-    for (b = 0; b < n; b += chunk) {
-        fom_int len = n - b;
-        if (len > chunk) len = chunk;
-        memset(v + b, 0, sizeof(fom_float) * (size_t)len);
-    }
-#else
-    memset(v, 0, sizeof(fom_float) * (size_t)n);
-#endif
-}
+#define FOM_HALF (FOM_STATE_DIM / 2)   /* 1300000 == FOM_T1_DIM */

 /* sign(x) * max(|x| - lambda, 0), branch-light and division-free. */
 static inline fom_float fom_soft(fom_float x, fom_float lambda) {
@@ -73,81 +87,107 @@
 }

 fom_int fom_solve(FomWorkspace *work) {
-    fom_float *const pb = work->acc;      /* t0_p_p0 @ p0, loop invariant */
+    /* Three rotating half-state buffers, each the upper half of a
+       state-sized workspace array so that fom_apply_t0_state (whose
+       only nonzero block is the right one) can read it directly. */
+    fom_float *prv = work->state + FOM_HALF;      /* s0 */
+    fom_float *cur = work->new_state + FOM_HALF;  /* s1 */
+    fom_float *nxt = work->acc + FOM_HALF;        /* w, then t1 */
     fom_float *const t0 = work->t0;
-    fom_float *const t1 = work->t1;
-    fom_float *cur = work->state;
-    fom_float *nxt = work->new_state;
+    const fom_float *const p0 = work->params.p0;
     const fom_float lam1 = FOM_T1_LAMBDA;
     const fom_int max_iter = work->settings.max_iter;
     fom_float residual = (fom_float)0;
+    /* Shrinkage of the t1 output is deferred to the next iteration's
+       extrapolation pass, which has to read the vector anyway; lam_in
+       is the threshold still owed on `cur` (0 == already applied, as it
+       is for the initial state, which must not be shrunk). */
+    fom_float lam_in = (fom_float)0;
     fom_int it, i;

     /* Each solve restarts from the generated initial state. */
-    memcpy(cur, FOM_STATE_INIT, sizeof(fom_float) * (size_t)FOM_STATE_DIM);
-
-    fom_zero(pb, FOM_T0_DIM);
-    fom_apply_t0_p_p0(work->params.p0, pb);
+#ifdef _OPENMP
+#pragma omp parallel for schedule(static)
+#endif
+    for (i = 0; i < FOM_HALF; ++i) {
+        prv[i] = FOM_STATE_INIT[i];
+        cur[i] = FOM_STATE_INIT[FOM_HALF + i];
+    }

     for (it = 0; it < max_iter; ++it) {
         const fom_float gamma = fom_sched_gamma(it);
+        const fom_float gp1 = (fom_float)1 + gamma;
         /* With no tolerance test the per-iteration residual is never
            read, so only the reported (final) one is computed. */
         const int want_resid = FOM_USE_TOL || (it + 1 >= max_iter);

-        /* ---- t0 = soft(t0_state @ s + gamma * (t0_state__d_gamma @ s)
-         *               + t0_p_p0 @ p0, lambda = 0)   [lambda 0 == identity] */
-        FOM_PROF_BEGIN(t0);
-        fom_zero(t0, FOM_T0_DIM);
-        fom_apply_t0_state__d_gamma(cur, t0);
+        /* ---- one fused parallel region, two independent streams:
+                  s1 <- soft(s1);  w = (1+gamma)*s1 - gamma*s0
+                  t0 <- -p0                      (t0_p_p0 == -I)
+                The shrinkage owed on s1 rides along with the read this
+                pass already has to do, and s0 was shrunk the same way
+                one iteration earlier, so no separate soft pass is
+                needed.  Folding the (much shorter) t0 right-hand side
+                into the same region saves a fork/join per iteration. */
+        FOM_PROF_BEGIN(extrap);
 #ifdef _OPENMP
-#pragma omp parallel for schedule(static)
+#pragma omp parallel
 #endif
-        for (i = 0; i < FOM_T0_DIM; ++i) t0[i] = gamma * t0[i] + pb[i];
-        fom_apply_t0_state(cur, t0);
-        FOM_PROF_END(t0);
-
-        /* ---- t1 = soft(t1_state @ s + gamma * (t1_state__d_gamma @ s)
-         *               + t1_t_t0 @ t0, lambda = FOM_T1_LAMBDA) */
-        FOM_PROF_BEGIN(t1);
-        fom_zero(t1, FOM_T1_DIM);
-        fom_apply_t1_state__d_gamma(cur, t1);
+        {
 #ifdef _OPENMP
-#pragma omp parallel for schedule(static)
+#pragma omp for schedule(static) nowait
 #endif
-        for (i = 0; i < FOM_T1_DIM; ++i) t1[i] *= gamma;
-        fom_apply_t1_state(cur, t1);
-        fom_apply_t1_t_t0(t0, t1);
+            for (i = 0; i < FOM_HALF; ++i) {
+                fom_float c = fom_soft(cur[i], lam_in);
+                cur[i] = c;
+                nxt[i] = gp1 * c - gamma * prv[i];
+            }
 #ifdef _OPENMP
-#pragma omp parallel for schedule(static)
+#pragma omp for schedule(static)
 #endif
-        for (i = 0; i < FOM_T1_DIM; ++i) t1[i] = fom_soft(t1[i], lam1);
-        FOM_PROF_END(t1);
+            for (i = 0; i < FOM_T0_DIM; ++i) t0[i] = -p0[i];
+        }
+        FOM_PROF_END(extrap);

-        /* ---- state <- su_state @ state + su_t_t1 @ t1 */
-        FOM_PROF_BEGIN(su);
-        fom_zero(nxt, FOM_STATE_DIM);
-        fom_apply_su_state(cur, nxt);
-        fom_apply_su_t_t1(t1, nxt);
-        FOM_PROF_END(su);
+        /* ---- t0 = -A w - p0.  A is the right block of t0_state, so the
+                helper is handed the base of the buffer whose upper half
+                holds w; the (structurally empty) left block means the
+                lower half is never read.  soft(., 0) == identity. */
+        FOM_PROF_BEGIN(t0_spmv);
+        fom_apply_t0_state(nxt - FOM_HALF, t0);
+        FOM_PROF_END(t0_spmv);
+
+        /* ---- t1 = w + B t0 (shrinkage deferred, see lam_in) */
+        FOM_PROF_BEGIN(t1_spmv);
+        fom_apply_t1_t_t0(t0, nxt);
+        FOM_PROF_END(t1_spmv);
+        lam_in = lam1;

-        /* ---- residual = max |new_state - state| */
+        /* ---- residual = max |new_state - state| over both halves,
+                i.e. max(|s1 - s0|, |t1 - s1|). */
         if (want_resid) {
             FOM_PROF_BEGIN(resid);
             residual = (fom_float)0;
 #ifdef _OPENMP
 #pragma omp parallel for schedule(static) reduction(max : residual)
 #endif
-            for (i = 0; i < FOM_STATE_DIM; ++i) {
-                fom_float d = nxt[i] - cur[i];
-                if (d < (fom_float)0) d = -d;
-                if (d > residual) residual = d;
+            for (i = 0; i < FOM_HALF; ++i) {
+                fom_float t = fom_soft(nxt[i], lam1);
+                fom_float d0 = cur[i] - prv[i];
+                fom_float d1 = t - cur[i];
+                nxt[i] = t;
+                if (d0 < (fom_float)0) d0 = -d0;
+                if (d1 < (fom_float)0) d1 = -d1;
+                if (d0 < d1) d0 = d1;
+                if (d0 > residual) residual = d0;
             }
+            lam_in = (fom_float)0;   /* shrinkage already applied above */
             FOM_PROF_END(resid);
         }

-        {   /* swap iterate buffers: the update needs no copy */
-            fom_float *tmp = cur;
+        {   /* state update is a pure rotation: s0 <- s1, s1 <- t1 */
+            fom_float *tmp = prv;
+            prv = cur;
             cur = nxt;
             nxt = tmp;
         }
@@ -160,10 +200,28 @@
 #endif
     }

-    if (cur != work->state)
-        memcpy(work->state, cur, sizeof(fom_float) * (size_t)FOM_STATE_DIM);
+    /* Reassemble the full state [s0; s1] = [prv; cur], applying the
+       shrinkage still owed on the last iterate.  One of prv/cur aliases
+       work->state + FOM_HALF; each lane reads its element before the
+       store to the same index, so the overlap is harmless. */
+    FOM_PROF_BEGIN(finish);
+    {
+        fom_float *const st = work->state;
+        fom_float *const sx = work->solution.x;
+#ifdef _OPENMP
+#pragma omp parallel for schedule(static)
+#endif
+        for (i = 0; i < FOM_HALF; ++i) {
+            fom_float a = prv[i];
+            fom_float b = fom_soft(cur[i], lam_in);
+            st[i] = a;
+            st[FOM_HALF + i] = b;
+            sx[i] = a;
+            sx[FOM_HALF + i] = b;
+        }
+    }
+    FOM_PROF_END(finish);

-    memcpy(work->solution.x, work->state, sizeof(fom_float) * (size_t)FOM_STATE_DIM);
     work->solution.iter = it;
     work->solution.residual = residual;
 #if FOM_USE_TOL
```

## Agent summary (end of the code-optimization transcript)

### Whole-solve timing

| | mean_us | notes |
|---|---:|---|
| initial (phase-A solver) | **1,917,300** | 3 interleaved A/B runs, 4 solves each: 1,918,573 / 1,914,898 / 1,918,440 |
| final | **549,150** | 549,153 / 549,263 / 549,030 (same runs, interleaved) |
| **speedup** | **3.49×** | |

(`make bench BENCH_SOLVES=5` on the final solver: `mean_us=548786.578 min_us=546211.247`.)
Accuracy: `VERIFY OK`, worst max-abs error 7.913e-07 against a 1.000e-05 tolerance
(the phase-A solver scored 1.226e-08; the loss is pure floating-point noise —
the two operator copies of `A` differ in the last 1–2 ulp and the "identity"
blocks are `1±2ulp` rather than exactly 1, and 200 FISTA iterations amplify a
~1e-16 relative perturbation to ~1e-7. No term was dropped or approximated
beyond those last-ulp differences, and the margin is still 13×.)

### Final profiling breakdown (`make profile`, 3 solves)

| region | total_us | per solve | pct | what it is |
|---|---:|---:|---:|---|
| `t0_spmv` | 778,424 | 259.5 ms | 47.43% | `fom_apply_t0_state` — `A w`, 2.6e6 nnz |
| `t1_spmv` | 536,235 | 178.7 ms | 32.67% | `fom_apply_t1_t_t0` — `B t0`, 2.6e6 nnz |
| `extrap` | 322,548 | 107.5 ms | 19.65% | shrinkage + extrapolation + `t0` rhs, fused |
| `resid` | 2,353 | 0.78 ms | 0.14% | final-iteration residual (runs once) |
| `finish` | 1,589 | 0.53 ms | 0.10% | reassemble state / solution (runs once) |

80% of what remains is the two sparse matvecs inside the fixed `fom_ops.c`
helpers, called on the smallest-nnz operator available for each product, once
per iteration each — that is the floor for this interface.

### Optimizations applied

1. **Collapsed the iteration onto its real operator structure (3.23× on its own).**
   Probed every operator numerically (`M@[u;0]` vs `M@[0;v]`) and cross-checked
   the CSR arrays: `su_state = [[0,I],[0,0]]` and `su_t_t1 = [[0],[I]]` are
   *exact* unit shift/injection, `t1_state = [0|I]` and `t1_state__d_gamma =
   [-I|I]` are diagonal, and `t0_p_p0 = -I` exactly. Structure exploited:
   identity / zero blocks and an exact unit shift. Consequences:
   - the state update `s <- su_state@s + su_t_t1@t1` becomes `s0 <- s1,
     s1 <- t1`, i.e. a **rotation of three buffer pointers** — both matvecs,
     the `FOM_STATE_DIM` zero-fill and the `FOM_STATE_DIM` output pass vanish;
   - `t1_state@s + gamma*(t1_state__d_gamma@s)` is just the FISTA
     extrapolation `w = (1+gamma)*s1 - gamma*s0`, one arithmetic pass instead
     of two CSR traversals plus a scaling pass;
   - `t0_p_p0 @ p0` is `-p0`, so the hoisted parameter matvec disappears too.
2. **Folded the two `t0` state terms into a single matvec.** `t0_state =
   [0|-A]` and `t0_state__d_gamma = [A|-A]` share the one data-carrying block
   `A`, so `t0_state@s + gamma*(d_gamma@s) = -A*((1+gamma)s1 - gamma*s0) =
   -A w`. Structure exploited: the two `t0` operators are the *same* sparse
   block up to sign, and `A` sits in the right block only. That drops the
   5.2e6-nnz `d_gamma` traversal entirely and shrinks the surviving one from
   the 3.98e6-nnz pair to 2.6e6 nnz (8/row). Because `A` occupies columns
   `[n,2n)`, `w` is kept in the **upper half** of a state-sized workspace
   buffer and `fom_apply_t0_state` is handed that buffer's base; the left
   block is structurally empty, so the lower half is never dereferenced.
3. **Iterates rotate through the upper halves of `state` / `new_state` /
   `acc`.** Any of the three can therefore serve directly as the `A`-matvec
   input, so the state update needs no copy at all and no new storage is
   required (still allocation-free, still division-free).
4. **Deferred shrinkage, fused into the extrapolation pass (−6.5%).** The
   `t1` output is left un-shrunk; the next iteration's extrapolation — which
   must read that vector anyway — applies `soft()` on the fly and writes the
   shrunk value back, so a whole 10.4 MB read+write pass per iteration
   disappears. Structure exploited: `soft(·,0) == identity`, which lets the
   initial state (which must *not* be shrunk) flow through the same code with
   a zero threshold; a per-buffer "threshold still owed" variable keeps the
   final iterate, the residual and the reassembled state exact.
5. **Fused the `t0` right-hand side into the extrapolation's parallel region
   (−1.2%).** `t0 <- -p0` is independent of `w`, so it runs as a second
   `omp for` (`nowait` on the first) in the same region — one fork/join per
   iteration instead of two.
6. **Residual folded into the final shrinkage pass, and computed only once.**
   With `FOM_USE_TOL == 0` the per-iteration residual is unobservable;
   on the last iteration `max|new_state - state|` is `max(|s1-s0|, |t1-s1|)`
   over the two halves, which the pass that materialises `soft(t1)` computes
   for free.
7. **Parallel, single-pass state reassembly.** The closing
   `[s0;s1] -> state -> solution.x` copies are one `omp for` instead of three
   serial `memcpy`s (`finish`: 6.9 ms → 1.6 ms per 3 solves).

Rejected after measurement (kept the faster form): dropping the shrunk
write-back in favour of shrinking both iterates on the fly (3 vector streams
instead of 4, but two `soft()` evaluations per element — exactly a wash, the
extrapolation loop is partly ALU-bound); the fma-style `c + gamma*(c - s0)`
extrapolation (fewer flops, but a longer dependency chain: −0.9%); hoisting the
negation of `p0` out of the loop (−0.4%); replacing the serial `memcpy` of the
initial state with a parallel copy (libc's non-temporal `memcpy` is as good,
and all 16 threads share one NUMA domain, so first-touch placement buys
nothing).
