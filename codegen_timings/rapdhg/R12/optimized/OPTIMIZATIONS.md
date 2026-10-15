# Optimization report

## Timing

```
translated (emitted)     208.4 us/solve
optimized                 22.3 us/solve
speedup                  9.36x
```

Optimization outcome: **kept (re-verified correct)**.

## What the agent changed

```diff
--- translated (emitted)
+++ optimized
@@ -10,14 +10,167 @@
 #define T0_DIM FOM_T0_DIM
 #define T1_DIM FOM_T1_DIM
 
-static inline void zero(fom_float *v, int n) {
-    memset(v, 0, (size_t)n * sizeof(fom_float));
-}
-
-/* out += c * v */
-static inline void axpy(fom_float *restrict out, fom_float c,
-                        const fom_float *restrict v, int n) {
-    for (int i = 0; i < n; ++i) out[i] += c * v[i];
+/* State slots (see spec.json): s0 x | s1 y | s2 | s3 | s4 x_prev | s5 */
+#define NX FOM_PARAM_P0_DIM /* 16 */
+#define NY FOM_PARAM_P1_DIM /* 48 */
+#define OFF_S0 0
+#define OFF_S1 16
+#define OFF_S2 64
+#define OFF_S3 80
+#define OFF_S4 128
+#define OFF_S5 144
+
+/* Operator blocks extracted once (at setup) from the fom_ops.c helpers.
+   The remaining terms of the iteration are identity / -identity / zero
+   blocks, applied directly below.
+     K  (NY x NX, CSC): t0_state__d_taux restricted to s0 columns;
+                        t0_state__d_tth on s4 columns equals -K.
+     C  (NX x NY, CSR): t1_t_t0__d_eta (= -K^T).
+     E  (NX x NX, dense column-major): t1_state__d_eta restricted to s5 columns. */
+static fom_int K_colptr[NX + 1];
+static fom_int K_row[NY * NX];
+static fom_float K_val[NY * NX];
+static fom_int C_rowptr[NX + 1];
+static fom_int C_col[NX * NY];
+static fom_float C_val[NX * NY];
+static fom_float E_cm[NX * NX];
+static int ops_ready = 0;
+
+typedef void (*apply_fn)(const fom_float *, fom_float *);
+
+/* Probe columns [col_off, col_off+ncols) of an operator with unit vectors
+   and store the nrows x ncols block densely, column-major. */
+static void extract_dense_cm(apply_fn f, int col_off, int ncols, int nrows,
+                             fom_float *dense) {
+    fom_float e[S_DIM];
+    fom_float out[S_DIM];
+    memset(e, 0, sizeof(e));
+    for (int j = 0; j < ncols; ++j) {
+        memset(out, 0, sizeof(out));
+        e[col_off + j] = 1.0;
+        f(e, out);
+        e[col_off + j] = 0.0;
+        memcpy(dense + j * nrows, out, (size_t)nrows * sizeof(fom_float));
+    }
+}
+
+/* Same, but stored as CSR (exact zeros dropped). */
+static void extract_csr(apply_fn f, int col_off, int ncols, int nrows,
+                        fom_int *rowptr, fom_int *col, fom_float *val) {
+    fom_float dense[NY * NX];
+    extract_dense_cm(f, col_off, ncols, nrows, dense);
+    fom_int p = 0;
+    for (int i = 0; i < nrows; ++i) {
+        rowptr[i] = p;
+        for (int j = 0; j < ncols; ++j) {
+            const fom_float v = dense[j * nrows + i];
+            if (v != 0.0) {
+                col[p] = j;
+                val[p] = v;
+                ++p;
+            }
+        }
+    }
+    rowptr[nrows] = p;
+}
+
+/* Same, but stored as CSC (exact zeros dropped). */
+static void extract_csc(apply_fn f, int col_off, int ncols, int nrows,
+                        fom_int *colptr, fom_int *row, fom_float *val) {
+    fom_float dense[NY * NX];
+    extract_dense_cm(f, col_off, ncols, nrows, dense);
+    fom_int p = 0;
+    for (int j = 0; j < ncols; ++j) {
+        colptr[j] = p;
+        for (int i = 0; i < nrows; ++i) {
+            const fom_float v = dense[j * nrows + i];
+            if (v != 0.0) {
+                row[p] = i;
+                val[p] = v;
+                ++p;
+            }
+        }
+    }
+    colptr[ncols] = p;
+}
+
+static void init_ops(void) {
+    if (ops_ready) return;
+    extract_csc(fom_apply_t0_state__d_taux, OFF_S0, NX, NY, K_colptr, K_row, K_val);
+    extract_csr(fom_apply_t1_t_t0__d_eta, 0, NY, NX, C_rowptr, C_col, C_val);
+    extract_dense_cm(fom_apply_t1_state__d_eta, OFF_S5, NX, NX, E_cm);
+    ops_ready = 1;
+}
+
+/* x <- su(x, t0, t1): identity-block state update, in place. */
+static inline void state_update(fom_float *restrict x,
+                                const fom_float *restrict t0,
+                                const fom_float *restrict t1,
+                                fom_float d1, fom_float d2, fom_float g1,
+                                fom_float g2, fom_float m1, fom_float m2) {
+    for (int i = 0; i < NX; ++i) {
+        const fom_float a0 = x[OFF_S0 + i];
+        const fom_float a2 = x[OFF_S2 + i];
+        const fom_float u = t1[i];
+        x[OFF_S0 + i] = g2 * a2 + g1 * u;
+        x[OFF_S2 + i] = d1 * a2 + d2 * u;
+        x[OFF_S4 + i] = a0;
+        x[OFF_S5 + i] = m1 * a2 + m2 * u;
+    }
+    for (int i = 0; i < NY; ++i) {
+        const fom_float a3 = x[OFF_S3 + i];
+        const fom_float v = t0[i];
+        x[OFF_S1 + i] = g2 * a3 + g1 * v;
+        x[OFF_S3 + i] = d1 * a3 + d2 * v;
+    }
+}
+
+/* Same update, also returning max_abs(x_new - x_old). */
+static inline fom_float state_update_res(fom_float *restrict x,
+                                         const fom_float *restrict t0,
+                                         const fom_float *restrict t1,
+                                         fom_float d1, fom_float d2,
+                                         fom_float g1, fom_float g2,
+                                         fom_float m1, fom_float m2) {
+    fom_float res = 0.0;
+    for (int i = 0; i < NX; ++i) {
+        const fom_float a0 = x[OFF_S0 + i];
+        const fom_float a2 = x[OFF_S2 + i];
+        const fom_float a4 = x[OFF_S4 + i];
+        const fom_float a5 = x[OFF_S5 + i];
+        const fom_float u = t1[i];
+        const fom_float n0 = g2 * a2 + g1 * u;
+        const fom_float n2 = d1 * a2 + d2 * u;
+        const fom_float n5 = m1 * a2 + m2 * u;
+        fom_float r0 = n0 - a0, r2 = n2 - a2, r4 = a0 - a4, r5 = n5 - a5;
+        r0 = r0 < 0.0 ? -r0 : r0;
+        r2 = r2 < 0.0 ? -r2 : r2;
+        r4 = r4 < 0.0 ? -r4 : r4;
+        r5 = r5 < 0.0 ? -r5 : r5;
+        res = r0 > res ? r0 : res;
+        res = r2 > res ? r2 : res;
+        res = r4 > res ? r4 : res;
+        res = r5 > res ? r5 : res;
+        x[OFF_S0 + i] = n0;
+        x[OFF_S2 + i] = n2;
+        x[OFF_S4 + i] = a0;
+        x[OFF_S5 + i] = n5;
+    }
+    for (int i = 0; i < NY; ++i) {
+        const fom_float a1 = x[OFF_S1 + i];
+        const fom_float a3 = x[OFF_S3 + i];
+        const fom_float v = t0[i];
+        const fom_float n1 = g2 * a3 + g1 * v;
+        const fom_float n3 = d1 * a3 + d2 * v;
+        fom_float r1 = n1 - a1, r3 = n3 - a3;
+        r1 = r1 < 0.0 ? -r1 : r1;
+        r3 = r3 < 0.0 ? -r3 : r3;
+        res = r1 > res ? r1 : res;
+        res = r3 > res ? r3 : res;
+        x[OFF_S1 + i] = n1;
+        x[OFF_S3 + i] = n3;
+    }
+    return res;
 }
 
 void fom_set_default_settings(FomSettings *settings) {
@@ -27,10 +180,11 @@
 }
 
 void fom_setup(FomWorkspace *work, const FomSettings *settings) {
+    init_ops();
     work->settings = *settings;
     memcpy(work->state, FOM_STATE_INIT, sizeof(work->state));
-    zero(work->params.p0, FOM_PARAM_P0_DIM);
-    zero(work->params.p1, FOM_PARAM_P1_DIM);
+    memset(work->params.p0, 0, sizeof(work->params.p0));
+    memset(work->params.p1, 0, sizeof(work->params.p1));
     work->solution.iter = 0;
     work->solution.residual = 0.0;
     work->solution.status = FOM_MAX_ITER_REACHED;
@@ -43,12 +197,11 @@
 
 fom_int fom_solve(FomWorkspace *work) {
     fom_float *restrict x = work->state;
-    fom_float *restrict xn = work->new_state;
-    fom_float *restrict acc = work->acc;
+    fom_float *restrict w = work->acc;
     fom_float *restrict t0 = work->t0;
     fom_float *restrict t1 = work->t1;
-    const fom_float *p0 = work->params.p0;
-    const fom_float *p1 = work->params.p1;
+    const fom_float *restrict p0 = work->params.p0;
+    const fom_float *restrict p1 = work->params.p1;
     const fom_int max_iter = work->settings.max_iter;
     const int use_tol = FOM_USE_TOL && work->settings.check_termination;
     const fom_float tol = work->settings.tol;
@@ -71,94 +224,65 @@
         const fom_float taux = fom_sched_taux(it);
         const fom_float tth = fom_sched_tth(it);
 
-        /* t0 = relu(...) */
-        zero(t0, T0_DIM);
-        fom_apply_t0_state(x, t0);
-        fom_apply_t0_p_p1(p1, t0);
-        if (taux != 0.0) {
-            zero(acc, T0_DIM);
-            fom_apply_t0_state__d_taux(x, acc);
-            axpy(t0, taux, acc, T0_DIM);
-        }
-        if (tth != 0.0) {
-            zero(acc, T0_DIM);
-            fom_apply_t0_state__d_tth(x, acc);
-            axpy(t0, tth, acc, T0_DIM);
-        }
-        if (tau != 0.0) {
-            zero(acc, T0_DIM);
-            fom_apply_t0_p_p1__d_tau(p1, acc);
-            axpy(t0, tau, acc, T0_DIM);
-        }
-        for (int i = 0; i < T0_DIM; ++i) t0[i] = t0[i] > 0.0 ? t0[i] : 0.0;
-
-        /* t1 = clip(...) */
-        zero(t1, T1_DIM);
-        fom_apply_t1_state(x, t1);
-        fom_apply_t1_p_p0(p0, t1);
-        fom_apply_t1_t_t0(t0, t1);
-        if (eta != 0.0) {
-            zero(acc, T1_DIM);
-            fom_apply_t1_state__d_eta(x, acc);
-            fom_apply_t1_p_p0__d_eta(p0, acc);
-            fom_apply_t1_t_t0__d_eta(t0, acc);
-            axpy(t1, eta, acc, T1_DIM);
-        }
-        for (int i = 0; i < T1_DIM; ++i) {
-            fom_float v = t1[i];
-            const fom_float lo = FOM_MAT_t1_lo[i];
-            const fom_float hi = FOM_MAT_t1_hi[i];
-            v = v < lo ? lo : v;
-            v = v > hi ? hi : v;
-            t1[i] = v;
-        }
-
-        /* state update */
-        zero(xn, S_DIM);
-        fom_apply_su_state(x, xn);
-        fom_apply_su_t_t0(t0, xn);
-        fom_apply_su_t_t1(t1, xn);
-        if (d1 != 0.0) {
-            zero(acc, S_DIM);
-            fom_apply_su_state__d_d1(x, acc);
-            axpy(xn, d1, acc, S_DIM);
-        }
-        if (g2 != 0.0) {
-            zero(acc, S_DIM);
-            fom_apply_su_state__d_g2(x, acc);
-            axpy(xn, g2, acc, S_DIM);
-        }
-        if (m1 != 0.0) {
-            zero(acc, S_DIM);
-            fom_apply_su_state__d_m1(x, acc);
-            axpy(xn, m1, acc, S_DIM);
-        }
-        if (d2 != 0.0) {
-            zero(acc, S_DIM);
-            fom_apply_su_t_t0__d_d2(t0, acc);
-            fom_apply_su_t_t1__d_d2(t1, acc);
-            axpy(xn, d2, acc, S_DIM);
-        }
-        if (g1 != 0.0) {
-            zero(acc, S_DIM);
-            fom_apply_su_t_t0__d_g1(t0, acc);
-            fom_apply_su_t_t1__d_g1(t1, acc);
-            axpy(xn, g1, acc, S_DIM);
-        }
-        if (m2 != 0.0) {
-            zero(acc, S_DIM);
-            fom_apply_su_t_t1__d_m2(t1, acc);
-            axpy(xn, m2, acc, S_DIM);
-        }
-
-        /* residual + swap-in */
-        res = 0.0;
-        for (int i = 0; i < S_DIM; ++i) {
-            fom_float d = xn[i] - x[i];
-            d = d < 0.0 ? -d : d;
-            res = d > res ? d : res;
-            x[i] = xn[i];
-        }
+        /* t0 = relu(s1 + K (taux*s0 - tth*s4) - tau*p1) */
+        FOM_PROF_BEGIN(t0);
+        for (int i = 0; i < NY; ++i) t0[i] = x[OFF_S1 + i] - tau * p1[i];
+        for (int j = 0; j < NX; ++j) {
+            const fom_float wj = taux * x[OFF_S0 + j] - tth * x[OFF_S4 + j];
+            for (fom_int p = K_colptr[j]; p < K_colptr[j + 1]; ++p)
+                t0[K_row[p]] += K_val[p] * wj;
+        }
+        for (int i = 0; i < NY; ++i) t0[i] = t0[i] > 0.0 ? t0[i] : 0.0;
+        FOM_PROF_END(t0);
+
+        /* t1 = clip(s0 + eta*(E s5 + C t0 - p0)) */
+        FOM_PROF_BEGIN(t1);
+        {
+            /* q = E s5 - p0, two interleaved accumulators over columns */
+            fom_float q[NX], qb[NX];
+            for (int i = 0; i < NX; ++i) {
+                q[i] = -p0[i];
+                qb[i] = 0.0;
+            }
+            for (int k = 0; k < NX; k += 2) {
+                const fom_float sa = x[OFF_S5 + k];
+                const fom_float sb = x[OFF_S5 + k + 1];
+                for (int i = 0; i < NX; ++i) {
+                    q[i] += E_cm[k * NX + i] * sa;
+                    qb[i] += E_cm[(k + 1) * NX + i] * sb;
+                }
+            }
+            for (int i = 0; i < NX; ++i) q[i] += qb[i];
+            for (int i = 0; i < NX; ++i) {
+                /* row of C t0, two accumulators to shorten the chain */
+                fom_float sa = 0.0, sb = 0.0;
+                const fom_int pe = C_rowptr[i + 1];
+                fom_int p = C_rowptr[i];
+                for (; p + 1 < pe; p += 2) {
+                    sa += C_val[p] * t0[C_col[p]];
+                    sb += C_val[p + 1] * t0[C_col[p + 1]];
+                }
+                if (p < pe) sa += C_val[p] * t0[C_col[p]];
+                const fom_float s = q[i] + (sa + sb);
+                fom_float v = x[OFF_S0 + i] + eta * s;
+                const fom_float lo = FOM_MAT_t1_lo[i];
+                const fom_float hi = FOM_MAT_t1_hi[i];
+                v = v < lo ? lo : v;
+                v = v > hi ? hi : v;
+                t1[i] = v;
+            }
+        }
+        FOM_PROF_END(t1);
+
+        /* state update (identity blocks only), in place. The residual is
+           only observable on the last iteration unless tol-checking is on. */
+        FOM_PROF_BEGIN(su);
+        if (use_tol || it + 1 >= max_iter) {
+            res = state_update_res(x, t0, t1, d1, d2, g1, g2, m1, m2);
+        } else {
+            state_update(x, t0, t1, d1, d2, g1, g2, m1, m2);
+        }
+        FOM_PROF_END(su);
 
         if (use_tol && res <= tol) {
             ++it;
```

## Agent notes

```
**Whole-solve time (`make bench`, 200 iterations, 160-dim state)**
- Initial: mean_us ≈ 199 (199.3 / 198.4 on two runs).
- Final: mean_us ≈ 20.6 on a quiet machine, 24–25 under load; min_us 19 in both cases.
- Overall speedup: about **8–9.5×**.

**Final profile (`make profile`; each region's time includes about 20 ns of timer overhead per call)**

| region | total_us | calls | % |
|---|---|---|---|
| t0 (K·w scatter, then relu) | 77,588 | 1,000,000 | 46.2 |
| t1 (E·s5 plus C·t0, then clip) | 55,769 | 1,000,000 | 33.2 |
| su (in-place state update) | 34,560 | 1,000,000 | 20.6 |

Initial profile for comparison: su 61.9%, t0 17.7%, t1 13.6%, residual 6.8%.

**Optimizations applied**
1. **Applied the identity, −identity and zero blocks directly** (largest gain: 199 → 42 µs).
   - The 22 operators break down as follows. `t0_p_p1`, `t1_p_p0`, `t1_t_t0`, `su_t_t0` and `su_t_t1` are all zero. `t0_state`, `t1_state` and all `su_*` operators are unit shifts between state slots. `t0_p_p1__d_tau` and `t1_p_p0__d_eta` are −I.
   - The state update collapses to:
     - s0 ← g2·s2 + g1·t1
     - s1 ← g2·s3 + g1·t0
     - s2 ← d1·s2 + d2·t1
     - s3 ← d1·s3 + d2·t0
     - s4 ← s0 (old value)
     - s5 ← m1·s2 + m2·t1
   - This runs as two fused in-place passes. They replace 7 CSR sweeps over 160 rows plus the zero/accumulate/axpy passes, and drop the `new_state` copy.
2. **Reduced t0 to one sparse product with K.** The `t0_state__d_tth` block on the s4 columns equals −K, where K is the `t0_state__d_taux` block on the s0 columns. So t0 = relu(s1 + K(taux·s0 − tth·s4) − tau·p1): one 112-nonzero product instead of two.
3. **Extracted the non-trivial blocks once at setup, no hand-copied data.** `fom_setup` probes the provided `fom_apply_*` helpers with unit vectors and stores:
   - K (48×16, 112 nonzeros) as CSC, scattered by column so there is no reduction chain;
   - C = `t1_t_t0__d_eta` (equal to −Kᵀ) as CSR;
   - E = `t1_state__d_eta` (16×16, dense) column-major.
   All three live in fixed-size static arrays.
4. **Residual computed only when it is observable** (42 → 26 µs). With `FOM_USE_TOL = 0` it only feeds `solution.residual` on the final iteration. Earlier iterations use an update with no residual; the version with the max-abs residual runs on the last iteration, or every iteration if tolerance checking is on.
5. **Column-major E with two accumulators** (24 → 21 µs). The dense 16×16 product becomes contiguous, vectorized axpy steps, and the two accumulators halve the floating-point dependency chain.
6. **Two accumulators in the C·t0 row dot products** (about 2–3%). This shortens the dependency chain on the serial x → t0 → t1 → x critical path.

**Tried and rejected**
- Dense K and C: about 2× slower, because without fast-math the reductions don't vectorize and dense column-major K is limited by the 768 multiply-adds.
- Splitting K's 32 single-entry rows from a CSR remainder: indistinguishable within noise, so I kept the simpler CSC form.

The last three kept changes each gained 3% or less, so I stopped there. The remaining time of about 100 ns per iteration is dominated by the serial dependency chain through two sparse products and one 16×16 dense product.
```
