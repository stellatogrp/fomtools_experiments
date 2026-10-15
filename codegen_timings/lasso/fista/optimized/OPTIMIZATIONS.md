# Optimization report

## Timing

```
translated (emitted)      25.0 us/solve
optimized                  4.4 us/solve
speedup                  5.63x
```

Optimization outcome: **kept (re-verified correct)**.

## What the agent changed

```diff
--- translated (emitted)
+++ optimized
@@ -27,55 +27,89 @@
     fom_reset_state(work);
 }
 
+/* Structure exploited (see spec.json / fom_data.c):
+ *   t0_state          = [ 0 | M ]   (left 15x15 block is zero)
+ *   t0_state__d_gamma = [-M | M ]   (so A x + g D x = M (x2 + g (x2 - x1)))
+ *   M is symmetric, so row j of M equals column j (axpy form, contiguous).
+ *   su_state / su_t_t0 are 0/1 selection matrices: new state = [x2, t0].
+ * Hence one 15x15 matvec per iteration and the state update is a pointer
+ * rotation over three 15-vectors (no copies). */
+#define FOM_N FOM_T0_DIM
+#define FOM_M_ROW(j) (FOM_MAT_t0_state + (j) * FOM_MAT_t0_state_COLS + FOM_N)
+
 fom_int fom_solve(FomWorkspace *work) {
-    fom_float *x = work->state;
-    fom_float *xn = work->new_state;
-    fom_float *t0 = work->t0;
-    fom_float *dg = work->acc;                    /* D_gamma @ state (15) */
-    fom_float *cp = work->acc + FOM_T0_DIM;       /* t0_p_p0 @ p0, constant (15) */
+    fom_float *x1 = work->state;            /* x_{k-1} */
+    fom_float *x2 = work->state + FOM_N;    /* x_k     */
+    fom_float *xn = work->new_state;        /* x_{k+1} */
+    fom_float *cp = work->acc;              /* t0_p_p0 @ p0, constant */
+    fom_float *y = work->acc + FOM_N;       /* extrapolated point */
     const fom_int max_iter = work->settings.max_iter;
     const fom_int use_tol = work->settings.check_termination && work->settings.tol > 0;
     const fom_float tol = work->settings.tol;
     const fom_float lam = FOM_T0_LAMBDA;
     fom_float res = 0.0;
     fom_int status = FOM_MAX_ITER_REACHED;
-    fom_int it, i;
+    fom_int it, i, j;
 
-    /* Parameter contribution is iteration-invariant: hoist it. */
-    memset(cp, 0, FOM_T0_DIM * sizeof(fom_float));
+    FOM_PROF_BEGIN(param_const);
+    memset(cp, 0, FOM_N * sizeof(fom_float));
     fom_apply_t0_p_p0(work->params.p0, cp);
+    FOM_PROF_END(param_const);
 
+    FOM_PROF_BEGIN(iterations);
     for (it = 0; it < max_iter; ++it) {
         const fom_float gamma = fom_sched_gamma(it);
+        fom_float t[FOM_N];
 
-        /* t0 = soft_threshold(A x + gamma * (D x) + P p0) */
-        memcpy(t0, cp, FOM_T0_DIM * sizeof(fom_float));
-        fom_apply_t0_state(x, t0);
-        if (gamma != 0.0) {
-            memset(dg, 0, FOM_T0_DIM * sizeof(fom_float));
-            fom_apply_t0_state__d_gamma(x, dg);
-            for (i = 0; i < FOM_T0_DIM; ++i) t0[i] += gamma * dg[i];
-        }
-        for (i = 0; i < FOM_T0_DIM; ++i) {
-            const fom_float v = t0[i];
-            t0[i] = (v > lam) ? (v - lam) : ((v < -lam) ? (v + lam) : 0.0);
+        /* y = x2 + gamma (x2 - x1) */
+        for (i = 0; i < FOM_N; ++i) y[i] = x2[i] + gamma * (x2[i] - x1[i]);
+
+        /* t = cp + M y  (M symmetric: accumulate rows as columns) */
+        /* Four independent partial sums shorten the FMA dependency chain
+           from 15 to 4 (the loop is latency-bound, not throughput-bound). */
+        {
+            fom_float ta[FOM_N], tb[FOM_N], tc[FOM_N];
+            for (i = 0; i < FOM_N; ++i) { t[i] = cp[i]; ta[i] = 0.0; tb[i] = 0.0; tc[i] = 0.0; }
+            for (j = 0; j + 3 < FOM_N; j += 4) {
+                const fom_float *m0 = FOM_M_ROW(j), *m1 = FOM_M_ROW(j + 1);
+                const fom_float *m2 = FOM_M_ROW(j + 2), *m3 = FOM_M_ROW(j + 3);
+                const fom_float y0 = y[j], y1 = y[j + 1], y2 = y[j + 2], y3 = y[j + 3];
+                for (i = 0; i < FOM_N; ++i) {
+                    t[i] += m0[i] * y0;
+                    ta[i] += m1[i] * y1;
+                    tb[i] += m2[i] * y2;
+                    tc[i] += m3[i] * y3;
+                }
+            }
+            for (; j < FOM_N; ++j) {
+                const fom_float *mj = FOM_M_ROW(j);
+                const fom_float yj = y[j];
+                for (i = 0; i < FOM_N; ++i) ta[i] += mj[i] * yj;
+            }
+            for (i = 0; i < FOM_N; ++i) t[i] = (t[i] + ta[i]) + (tb[i] + tc[i]);
         }
 
-        /* state <- su_state @ state + su_t_t0 @ t0 */
-        memset(xn, 0, FOM_STATE_DIM * sizeof(fom_float));
-        fom_apply_su_state(x, xn);
-        fom_apply_su_t_t0(t0, xn);
+        /* xn = soft_threshold(t, lam) */
+        for (i = 0; i < FOM_N; ++i) {
+            const fom_float v = t[i];
+            xn[i] = (v > lam) ? (v - lam) : ((v < -lam) ? (v + lam) : 0.0);
+        }
 
+        /* residual on new state [x2, xn] vs old [x1, x2] */
         if (use_tol || it == max_iter - 1) {
             res = 0.0;
-            for (i = 0; i < FOM_STATE_DIM; ++i) {
-                fom_float d = xn[i] - x[i];
+            for (i = 0; i < FOM_N; ++i) {
+                fom_float d = x2[i] - x1[i];
+                fom_float e = xn[i] - x2[i];
                 if (d < 0) d = -d;
+                if (e < 0) e = -e;
                 if (d > res) res = d;
+                if (e > res) res = e;
             }
         }
 
-        { fom_float *tmp = x; x = xn; xn = tmp; }
+        /* state shift: (x1, x2) <- (x2, xn), recycle x1 buffer */
+        { fom_float *tmp = x1; x1 = x2; x2 = xn; xn = tmp; }
 
         if (use_tol && res <= tol) {
             ++it;
@@ -83,9 +117,14 @@
             break;
         }
     }
+    FOM_PROF_END(iterations);
 
-    if (x != work->state) memcpy(work->state, x, sizeof(work->state));
-    memcpy(work->solution.x, work->state, sizeof(work->solution.x));
+    FOM_PROF_BEGIN(finalize);
+    /* x1/x2 may alias any of the three buffers; stage through solution.x */
+    memcpy(work->solution.x, x1, FOM_N * sizeof(fom_float));
+    memcpy(work->solution.x + FOM_N, x2, FOM_N * sizeof(fom_float));
+    memcpy(work->state, work->solution.x, sizeof(work->state));
+    FOM_PROF_END(finalize);
     work->solution.iter = it;
     work->solution.residual = res;
     work->solution.status = status;
```

## Agent notes

```
**Whole-solve mean_us** (`./bench_bin 100000`; the default 5000-solve `make bench` is too noisy at this scale and read 5.3–6.0 µs for the final code)
- Initial: 23.6 µs (26–32 µs with the default `make bench`)
- Final: 3.32 µs
- Speedup: about 7.1×

**Final profile** (`make profile`, 5000 solves, hottest first)
| Region | total_us | pct |
|---|---|---|
| iterations (200-iteration FISTA loop: extrapolate, 15×15 matvec, soft-threshold, state rotation) | 28235 | 98.65% |
| finalize (write back state and solution) | 229 | 0.80% |
| param_const (t0_p_p0 @ p0, computed once per solve) | 158 | 0.55% |

**Optimizations applied**
1. **Halved the matvec work using the block structure of the linear operator.** `t0_state` = [0 | M] (left block ≤ 6e-17) and `t0_state__d_gamma` = [−M | M] (matches to about 1e-15). So `t0_state·x + γ·D·x` equals `M(x2 + γ(x2 − x1))`. Each iteration now computes the extrapolated point `y` (15 ops) and then does one 15×15 matvec, instead of two dense 15×30 matvecs. That is 225 multiply-adds instead of 900.
2. **Replaced the sparse state update with pointer rotation.** `su_state` and `su_t_t0` are 0/1 selection matrices, so the new state is just [x2, t0]. Three 15-vectors are rotated by pointer, so there are no CSR passes, no memset, and no copies. The state is written back once at the end.
3. **Contiguous, vectorizable matvec using the symmetry of M** (asymmetry ≤ 1.4e-16). Row j of M equals column j, so `t += y_j · M[j, :]` reads memory contiguously from `FOM_MAT_t0_state` with no transposed copy.
4. **Shortened the dependency chain with 4 independent partial sums.** Each iteration depends on the previous one, so the matvec is limited by multiply-add latency rather than throughput. Splitting the j-sum across 4 accumulators cuts the chain from 15 to 4 and took the solve from 4.35 µs to 3.30 µs.
5. **Kept the parameter term out of the loop.** `t0_p_p0 @ p0` does not change across iterations, so it is computed once per solve. The convergence residual is computed only on the last iteration, because tolerance checking is off (tol = 0).

**Tried and rejected:** a copy of M padded to 16 columns (no gain), a 5-way split (3.4× slower, likely register spills), a row dot-product form (4.3 µs, slower), and a 3-way split (same speed as 4-way).
```
