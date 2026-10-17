# Optimization report

## Timing

```
translated (emitted)     364.1 us/solve
optimized                 26.6 us/solve
speedup                 13.68x
```

Optimization outcome: **kept (re-verified correct)**.

## What the agent changed

```diff
--- translated (emitted)
+++ optimized
@@ -27,55 +27,90 @@
     fom_reset_state(work);
 }
 
+/* t0_state has nonzeros only in the s1 columns (50..99), and
+   t0_state__d_gamma == [-A1, A1] with A1 = t0_state[:, 50:100].  Hence
+   A x + gamma D x = A1 (s1 + gamma (s1 - s0)): one 50x50 matvec per iteration.
+   A1 is extracted once (unit-vector probing of the fixed operator) into a
+   dense column-major table so the inner product vectorizes. */
+#define N FOM_T0_DIM
+static fom_float A1[N * N];
+static int A1_ready = 0;
+
+static void fom_extract_a1(void) {
+    fom_float e[FOM_STATE_DIM], col[N];
+    fom_int i, j;
+    memset(e, 0, sizeof(e));
+    for (j = 0; j < N; ++j) {
+        e[N + j] = 1.0;
+        memset(col, 0, sizeof(col));
+        fom_apply_t0_state(e, col);
+        for (i = 0; i < N; ++i) A1[j * N + i] = col[i];  /* column-major */
+        e[N + j] = 0.0;
+    }
+    A1_ready = 1;
+}
+
 fom_int fom_solve(FomWorkspace *work) {
-    fom_float *x = work->state;
-    fom_float *xn = work->new_state;
-    fom_float *t0 = work->t0;
-    fom_float *dg = work->acc;                    /* D_gamma @ state (50) */
-    fom_float *cp = work->acc + FOM_T0_DIM;       /* t0_p_p0 @ p0, constant (50) */
+    fom_float *s0 = work->state;
+    fom_float *s1 = work->state + N;
+    fom_float y[N];
+    fom_float *cp = work->acc + N;       /* t0_p_p0 @ p0, constant (50) */
+    fom_float t[N];
     const fom_int max_iter = work->settings.max_iter;
     const fom_int use_tol = work->settings.check_termination && work->settings.tol > 0;
     const fom_float tol = work->settings.tol;
     const fom_float lam = FOM_T0_LAMBDA;
     fom_float res = 0.0;
     fom_int status = FOM_MAX_ITER_REACHED;
-    fom_int it, i;
+    fom_int it, i, j;
+
+    if (!A1_ready) fom_extract_a1();
 
     /* Parameter contribution is iteration-invariant: hoist it. */
-    memset(cp, 0, FOM_T0_DIM * sizeof(fom_float));
+    memset(cp, 0, N * sizeof(fom_float));
     fom_apply_t0_p_p0(work->params.p0, cp);
 
     for (it = 0; it < max_iter; ++it) {
         const fom_float gamma = fom_sched_gamma(it);
+        const fom_int need_res = use_tol || it == max_iter - 1;
 
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
-        }
+        FOM_PROF_BEGIN(extrapolate);
+        for (i = 0; i < N; ++i) y[i] = s1[i] + gamma * (s1[i] - s0[i]);
+        FOM_PROF_END(extrapolate);
 
-        /* state <- su_state @ state + su_t_t0 @ t0 */
-        memset(xn, 0, FOM_STATE_DIM * sizeof(fom_float));
-        fom_apply_su_state(x, xn);
-        fom_apply_su_t_t0(t0, xn);
-
-        if (use_tol || it == max_iter - 1) {
-            res = 0.0;
-            for (i = 0; i < FOM_STATE_DIM; ++i) {
-                fom_float d = xn[i] - x[i];
-                if (d < 0) d = -d;
-                if (d > res) res = d;
+        FOM_PROF_BEGIN(matvec_prox);
+        /* column-oriented axpy form: vectorizes across rows with no
+           reassociation of floating-point sums */
+        {
+            fom_float *restrict tt = t;
+            const fom_float *restrict yy = y;
+            memcpy(tt, cp, N * sizeof(fom_float));
+            for (j = 0; j < N; ++j) {
+                const fom_float *restrict a = A1 + j * N;
+                const fom_float yj = yy[j];
+                for (i = 0; i < N; ++i) tt[i] += a[i] * yj;
+            }
+            for (i = 0; i < N; ++i) {
+                const fom_float v = tt[i];
+                tt[i] = (v > lam) ? (v - lam) : ((v < -lam) ? (v + lam) : 0.0);
             }
         }
+        FOM_PROF_END(matvec_prox);
 
-        { fom_float *tmp = x; x = xn; xn = tmp; }
+        FOM_PROF_BEGIN(state_update);
+        if (need_res) {
+            res = 0.0;
+            for (i = 0; i < N; ++i) {
+                fom_float d0 = s1[i] - s0[i], d1 = t[i] - s1[i];
+                if (d0 < 0) d0 = -d0;
+                if (d1 < 0) d1 = -d1;
+                if (d0 > res) res = d0;
+                if (d1 > res) res = d1;
+            }
+        }
+        memcpy(s0, s1, N * sizeof(fom_float));
+        memcpy(s1, t, N * sizeof(fom_float));
+        FOM_PROF_END(state_update);
 
         if (use_tol && res <= tol) {
             ++it;
@@ -84,7 +119,6 @@
         }
     }
 
-    if (x != work->state) memcpy(work->state, x, sizeof(work->state));
     memcpy(work->solution.x, work->state, sizeof(work->solution.x));
     work->solution.iter = it;
     work->solution.residual = res;
```

## Agent notes

```
**Whole-solve time (`make bench`):** initial `mean_us` = 361.8, final `mean_us` ≈ 27.2 (range 25.3–28.3 over repeated runs, min_us 24). That is about **13.3× faster**. `make verify` prints `VERIFY OK`, with a worst max-abs error of 2.8e-10 against a tolerance of 1e-6.

**Final profile (`make profile`, 5000 solves × 200 iterations; the timing hooks themselves add overhead):**

| Region | Total time | Share |
|---|---|---|
| `matvec_prox` (dense 50×50 matrix-vector product plus soft-threshold) | 130.0 ms | 74.1% |
| `state_update` (residual check and state shift) | 24.7 ms | 14.1% |
| `extrapolate` (y = s1 + γ(s1 − s0)) | 20.7 ms | 11.8% |

**Optimizations applied:**
1. **Merged the two state operators into one 50×50 product.** I checked the operators numerically. `t0_state` only has entries in the s1 columns (50..99). `t0_state__d_gamma` equals `[-A1, A1]`, where A1 = `t0_state[:,50:]`, to within about 1e-15. So `A·x + γ·D·x` simplifies to `A1·(s1 + γ(s1 − s0))`. Each iteration now does one 50×50 product instead of a sparse 50×100 product, a dense 50×100 product and an extra pass to combine them (362 → 72 µs).
2. **Simplified the state update.** `su_state` and `su_t_t0` are just 0/1 selection matrices. The update is therefore only the shift s0 ← s1, s1 ← t, done with two copies instead of two sparse products. The residual is computed directly as max(|s1 − s0|, |t − s1|), and only when it is actually needed.
3. **Pulled fixed work out of the iteration loop.** A1 is read out of the fixed operator once, by applying it to unit vectors, into a static dense table (A1 is 69% nonzero, so a dense layout beats sparse). The parameter term `t0_p_p0·p0` is also computed once per solve.
4. **Rewrote the matrix-vector product so the compiler vectorizes it (72 → 28 µs).** A1 is stored column by column and the product is done as t += A1[:,j]·y[j]. This lets the compiler use vector instructions across rows without changing the order of floating-point additions, which matters because the build doesn't use `-ffast-math`. Working into local arrays keeps the compiler from assuming the buffers overlap.

**Tried and dropped because they were slower:** processing 5 columns per pass (about 35 µs); rotating three buffer pointers instead of copying the state, even with `restrict` (about 40 µs); skipping columns where y[j] is zero to exploit the sparse lasso iterates (about 42 µs, likely due to branch mispredictions). The last two rounds gave no improvement. The remaining product is about 2500 multiply-adds per iteration and is running close to the hardware's compute limit.
```
