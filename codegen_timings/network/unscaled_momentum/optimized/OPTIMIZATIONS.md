# Optimization report

## Timing

```
translated (emitted)     169.5 us/solve
optimized                 18.8 us/solve
speedup                  9.03x
```

Optimization outcome: **kept (re-verified correct)**.

## What the agent changed

```diff
--- translated (emitted)
+++ optimized
@@ -2,6 +2,98 @@
 #include "fom_data.h"
 #include "fom_ops.h"
 #include "fom_linsolve.h"
+#include "fom_profile.h"
+
+/* Structure exploited (verified against data.npz):
+ *   state = [s0 (150) | s1 (10) | s2 (15)],  s0 is indexed by edge
+ *   e = 15*j + k joining node j of s1 (j<10) and node k of s2 (k<15).
+ *   t0[e]     = clip(c[e] + s0[e] + d1[j]*s1[j] + a[e]*s2[k], lo, hi)
+ *               (t0_state's s0 block is the identity, its s1 block is
+ *                a constant per node j)
+ *   t1[j]     = relu(t1_c[j] + w1[j]*s1[j]
+ *                    + (u[j]+xi*ud[j])*sum_k s0[e] + (v[j]+xi*vd[j])*sum_k t0[e])
+ *               (every t1 coefficient block is constant along its row:
+ *                t1 only sees the sum over the edges incident to node j)
+ *   s0'[e]    = t0[e]                (su_t_t0 rows 0..149 = identity)
+ *   s1'[j]    = h1[j]*t1[j]          (su_t_t1 is diagonal)
+ *   s2'[k]    = sg[k]*s2[k] + pk[k]*p0[k]
+ *               + sum_j (b[e]+xi*bd[e])*s0[e] + (g[e]+xi*gd[e])*t0[e]
+ * Every other block of every operator is zero. The per-edge/per-node
+ * coefficients are recovered once (in fom_setup) by probing the CSR
+ * helpers with unit vectors, so the iteration is a single fused,
+ * in-place O(150) pass instead of eleven CSR products. */
+
+#define NJ 10
+#define NK 15
+#define NE 150
+#define NKP 16          /* edge rows padded to 16 so the k loop has no tail */
+#define NEP (NJ * NKP)
+#define OFF1 150
+#define OFF2 160
+
+typedef struct {
+    /* Per-edge data in padded layout [j][k], k < NKP. Pad entries stay
+       zero (static storage), which makes them inert: t0 = clip(.,0,0)
+       = 0 and they add +0 to every sum. */
+    fom_float c[NEP], lo[NEP], hi[NEP], a[NEP];
+    fom_float b[NEP], bd[NEP], g[NEP], gd[NEP];
+    fom_float d1[NJ], u[NJ], ud[NJ], v[NJ], vd[NJ];
+    fom_float t1c[NJ], w1[NJ], h1[NJ];
+    fom_float sg[NK], pk[NK];
+} FomCoef;
+
+static FomCoef K;
+
+/* Column `col` of an operator with `n_in` inputs: out = M @ e_col. */
+static void probe(void (*op)(const fom_float *, fom_float *), fom_int n_in,
+                  fom_int n_out, fom_int col, fom_float *in, fom_float *out) {
+    fom_int i;
+    for (i = 0; i < n_in; ++i) in[i] = 0.0;
+    for (i = 0; i < n_out; ++i) out[i] = 0.0;
+    in[col] = 1.0;
+    op(in, out);
+}
+
+static void fom_extract_coefs(void) {
+    fom_float in[FOM_STATE_DIM], out[FOM_STATE_DIM];
+    fom_int j, k, e, q;
+    for (j = 0; j < NJ; ++j) for (k = 0; k < NK; ++k) {
+        e = NK * j + k; q = NKP * j + k;
+        K.c[q] = FOM_MAT_t0_c[e];
+        K.lo[q] = FOM_MAT_t0_lo[e];
+        K.hi[q] = FOM_MAT_t0_hi[e];
+    }
+    for (j = 0; j < NJ; ++j) K.t1c[j] = FOM_MAT_t1_c[j];
+
+    /* Columns of s0 / tmp:t0 (edge e). */
+    for (j = 0; j < NJ; ++j) for (k = 0; k < NK; ++k) {
+        e = NK * j + k; q = NKP * j + k;
+        if (k == 0) {
+            probe(fom_apply_t1_state, FOM_STATE_DIM, NJ, e, in, out);        K.u[j] = out[j];
+            probe(fom_apply_t1_state__d_xi, FOM_STATE_DIM, NJ, e, in, out);  K.ud[j] = out[j];
+            probe(fom_apply_t1_t_t0, NE, NJ, e, in, out);                    K.v[j] = out[j];
+            probe(fom_apply_t1_t_t0__d_xi, NE, NJ, e, in, out);              K.vd[j] = out[j];
+        }
+        probe(fom_apply_su_state, FOM_STATE_DIM, FOM_STATE_DIM, e, in, out);       K.b[q] = out[OFF2 + k];
+        probe(fom_apply_su_state__d_xi, FOM_STATE_DIM, FOM_STATE_DIM, e, in, out); K.bd[q] = out[OFF2 + k];
+        probe(fom_apply_su_t_t0, NE, FOM_STATE_DIM, e, in, out);          K.g[q] = out[OFF2 + k];
+        probe(fom_apply_su_t_t0__d_xi, NE, FOM_STATE_DIM, e, in, out);    K.gd[q] = out[OFF2 + k];
+    }
+    /* Columns of s1 (node j). */
+    for (j = 0; j < NJ; ++j) {
+        probe(fom_apply_t0_state, FOM_STATE_DIM, NE, OFF1 + j, in, out);
+        K.d1[j] = out[NK * j];
+        probe(fom_apply_t1_state, FOM_STATE_DIM, NJ, OFF1 + j, in, out); K.w1[j] = out[j];
+        probe(fom_apply_su_t_t1, NJ, FOM_STATE_DIM, j, in, out);         K.h1[j] = out[OFF1 + j];
+    }
+    /* Columns of s2 / p0 (node k). */
+    for (k = 0; k < NK; ++k) {
+        probe(fom_apply_t0_state, FOM_STATE_DIM, NE, OFF2 + k, in, out);
+        for (j = 0; j < NJ; ++j) K.a[NKP * j + k] = out[NK * j + k];
+        probe(fom_apply_su_state, FOM_STATE_DIM, FOM_STATE_DIM, OFF2 + k, in, out); K.sg[k] = out[OFF2 + k];
+        probe(fom_apply_su_p_p0, FOM_PARAM_P0_DIM, FOM_STATE_DIM, k, in, out);      K.pk[k] = out[OFF2 + k];
+    }
+}
 
 void fom_set_default_settings(FomSettings *settings) {
     settings->max_iter = FOM_MAX_ITER;
@@ -19,6 +111,7 @@
     work->settings = *settings;
     for (i = 0; i < FOM_PARAM_P0_DIM; ++i) work->params.p0[i] = 0.0;
     fom_reset_state(work);
+    fom_extract_coefs();
     work->solution.iter = 0;
     work->solution.residual = 0.0;
     work->solution.status = FOM_MAX_ITER_REACHED;
@@ -29,66 +122,104 @@
     fom_reset_state(work);
 }
 
+/* Sum of one node's NKP (padded) edges, as independent partial sums. */
+static inline fom_float row_sum(const fom_float *r) {
+    fom_float p0 = r[0] + r[1], p1 = r[2] + r[3];
+    fom_float p2 = r[4] + r[5], p3 = r[6] + r[7];
+    p0 += r[8] + r[9];
+    p1 += r[10] + r[11];
+    p2 += r[12] + r[13];
+    p3 += r[14] + r[15];
+    return (p0 + p1) + (p2 + p3);
+}
+
 fom_int fom_solve(FomWorkspace *work) {
-    fom_float *x = work->state;
-    fom_float *xn = work->new_state;
-    fom_float *acc = work->acc;
-    fom_float *t0 = work->t0;
-    fom_float *t1 = work->t1;
+    fom_float *restrict x = work->state;
+    fom_float *restrict prev = work->new_state; /* old state, last iter only */
+    fom_float *restrict t1 = work->t1;
+    /* Padded working copy of the state: s0 as [NJ][NKP], s1, s2[NKP]. */
+    fom_float z0[NEP], z1[NJ], z2[NKP];
+    fom_float s2p[NKP], pterm[NKP], ssum[NJ];
     fom_float res = 0.0;
     const fom_int max_iter = work->settings.max_iter;
-    fom_int it, i;
+    fom_int it, i, j, k;
+
+    FOM_PROF_BEGIN(pack);
+    for (j = 0; j < NJ; ++j) {
+        for (k = 0; k < NK; ++k) z0[NKP * j + k] = x[NK * j + k];
+        for (; k < NKP; ++k) z0[NKP * j + k] = 0.0;
+        z1[j] = x[OFF1 + j];
+    }
+    for (k = 0; k < NK; ++k) {
+        z2[k] = x[OFF2 + k];
+        /* Iteration-invariant parameter term of the s2 update. */
+        pterm[k] = K.pk[k] * work->params.p0[k];
+    }
+    for (; k < NKP; ++k) { z2[k] = 0.0; pterm[k] = 0.0; }
+    for (j = 0; j < NJ; ++j) ssum[j] = row_sum(z0 + NKP * j);
+    FOM_PROF_END(pack);
 
     for (it = 0; it < max_iter; ++it) {
         const fom_float xi = fom_sched_xi(it);
-
-        /* t0 = clip(t0_c + t0_state @ x, lo, hi) */
-        for (i = 0; i < FOM_T0_DIM; ++i) t0[i] = FOM_MAT_t0_c[i];
-        fom_apply_t0_state(x, t0);
-        for (i = 0; i < FOM_T0_DIM; ++i) {
-            fom_float v = t0[i];
-            const fom_float lo = FOM_MAT_t0_lo[i];
-            const fom_float hi = FOM_MAT_t0_hi[i];
-            v = v < lo ? lo : v;
-            v = v > hi ? hi : v;
-            t0[i] = v;
+        const int last = (it + 1 == max_iter);
+        if (last)
+            for (i = 0; i < FOM_STATE_DIM; ++i) prev[i] = x[i];
+
+        FOM_PROF_BEGIN(edge_pass);
+        for (k = 0; k < NKP; ++k) s2p[k] = 0.0;
+        for (j = 0; j < NJ; ++j) {
+            const fom_float s1j = z1[j];
+            const fom_float base = K.d1[j] * s1j;
+            fom_float *restrict x0 = z0 + NKP * j;
+            const fom_float *restrict c = K.c + NKP * j, *restrict a = K.a + NKP * j;
+            const fom_float *restrict lo = K.lo + NKP * j, *restrict hi = K.hi + NKP * j;
+            const fom_float *restrict b = K.b + NKP * j, *restrict bd = K.bd + NKP * j;
+            const fom_float *restrict g = K.g + NKP * j, *restrict gd = K.gd + NKP * j;
+            fom_float st;
+            /* No reductions here, so this loop vectorizes. */
+            for (k = 0; k < NKP; ++k) {
+                const fom_float s0e = x0[k];
+                fom_float v = c[k] + s0e + base + a[k] * z2[k];
+                v = v < lo[k] ? lo[k] : v;
+                v = v > hi[k] ? hi[k] : v;
+                x0[k] = v; /* s0' = t0 (su_t_t0 identity block), in place */
+                s2p[k] += (b[k] + xi * bd[k]) * s0e + (g[k] + xi * gd[k]) * v;
+            }
+            st = row_sum(x0);
+            {
+                const fom_float w = K.t1c[j] + K.w1[j] * s1j
+                                  + (K.u[j] + xi * K.ud[j]) * ssum[j]
+                                  + (K.v[j] + xi * K.vd[j]) * st;
+                t1[j] = w > 0.0 ? w : 0.0;
+            }
+            /* s0' = t0, so next iteration's sum over s0 is this st. */
+            ssum[j] = st;
         }
-
-        /* t1 = relu(t1_c + t1_state@x + xi*(d_xi@x) + t1_t_t0@t0 + xi*(d_xi@t0)) */
-        for (i = 0; i < FOM_T1_DIM; ++i) { t1[i] = FOM_MAT_t1_c[i]; acc[i] = 0.0; }
-        fom_apply_t1_state(x, t1);
-        fom_apply_t1_t_t0(t0, t1);
-        fom_apply_t1_state__d_xi(x, acc);
-        fom_apply_t1_t_t0__d_xi(t0, acc);
-        for (i = 0; i < FOM_T1_DIM; ++i) {
-            const fom_float v = t1[i] + xi * acc[i];
-            t1[i] = v > 0.0 ? v : 0.0;
-        }
-
-        /* State update */
-        for (i = 0; i < FOM_STATE_DIM; ++i) { xn[i] = 0.0; acc[i] = 0.0; }
-        fom_apply_su_state(x, xn);
-        fom_apply_su_t_t0(t0, xn);
-        fom_apply_su_t_t1(t1, xn);
-        fom_apply_su_p_p0(work->params.p0, xn);
-        fom_apply_su_state__d_xi(x, acc);
-        fom_apply_su_t_t0__d_xi(t0, acc);
-        for (i = 0; i < FOM_STATE_DIM; ++i) xn[i] += xi * acc[i];
-
-        if (it + 1 == max_iter) {
+        FOM_PROF_END(edge_pass);
+
+        FOM_PROF_BEGIN(node_update);
+        for (j = 0; j < NJ; ++j) z1[j] = K.h1[j] * t1[j];
+        for (k = 0; k < NKP; ++k)
+            z2[k] = K.sg[k] * z2[k] + pterm[k] + s2p[k];
+        FOM_PROF_END(node_update);
+
+        if (last) {
+            FOM_PROF_BEGIN(unpack);
+            for (j = 0; j < NJ; ++j) {
+                for (k = 0; k < NK; ++k) x[NK * j + k] = z0[NKP * j + k];
+                x[OFF1 + j] = z1[j];
+            }
+            for (k = 0; k < NK; ++k) x[OFF2 + k] = z2[k];
             res = 0.0;
             for (i = 0; i < FOM_STATE_DIM; ++i) {
-                fom_float d = xn[i] - x[i];
+                fom_float d = x[i] - prev[i];
                 d = d < 0.0 ? -d : d;
                 res = d > res ? d : res;
             }
+            FOM_PROF_END(unpack);
         }
-
-        { fom_float *tmp = x; x = xn; xn = tmp; }
-    }
-
-    if (x != work->state)
-        for (i = 0; i < FOM_STATE_DIM; ++i) work->state[i] = x[i];
+    }
+
     for (i = 0; i < FOM_STATE_DIM; ++i) work->solution.x[i] = x[i];
     work->solution.iter = max_iter;
     work->solution.residual = res;
```

## Agent notes

```
**Whole-solve time (`make bench`, 5000 solves):** 165.58 µs at the start, 19.80 µs at the end (18.4–20.0 µs over repeated runs; about 17 µs with 20,000 solves). That is about **8.4× faster**. `make verify` passes with a worst error of 5.3e-15 (tolerance 1e-6).

**Profile of the final solver (`make profile`).** Timer overhead roughly doubles the per-iteration numbers here, so read the percentages as relative shares:

| Region | Time (µs, total) | Share |
|---|---|---|
| edge_pass (t0, t1 and s2 sums, one fused pass) | 105,715 | 83.6% |
| node_update (s1, s2 writes) | 20,028 | 15.9% |
| unpack (last iteration only: copy back and residual) | 468 | 0.4% |
| pack (once per solve) | 183 | 0.1% |

**Optimizations applied:**

1. **One fused pass per iteration instead of eleven sparse matrix products (165.6 → 49.2 µs).** The state is a 10×15 edge network: s0 has one entry per edge e = 15j+k, s1 one per node j, s2 one per node k. Every operator only links an edge to its own two end nodes. So t0[e] depends only on s0[e], s1[j] and s2[k]; t1 and the s2 update are sums over each node's edges; and the new s0 equals t0 (that block of `su_t_t0` is the identity). The coefficients are read once in `fom_setup` by calling the fixed `fom_apply_*` helpers on unit vectors, so all numbers still come from `fom_ops.c`.
2. **Per-node constants and in-place update (49.2 → 34.0 µs).** All t1 coefficient blocks and the s1 column of `t0_state` have the same value along each row. So t1 needs only the sum of each node's edges times a per-node constant, not per-edge arrays. Because the new s0 is just t0, t0 is written straight into the state, with no extra buffer or copy.
3. **One accumulator for the s2 sums (about 10% faster).** xi is folded into each coefficient as (b + xi·bd) and (g + xi·gd), so there is one running sum instead of two.
4. **Reuse last iteration's edge sum.** Because the new s0 equals t0, the sum of s0 over a node's edges at the next iteration is exactly this iteration's t0 sum (bit-for-bit identical). That removes one sum and lets the main loop be vectorized.
5. **Pad each node's 15 edges to 16 (28 → ~17 µs).** The working copy of the state is kept in a [10][16] layout and copied back only at the end. The padding entries have all-zero coefficients and bounds, so they always come out as zero and change nothing. With 16 lanes the loop vectorizes cleanly with no leftover elements.
6. **Work hoisted out of the loop.** The p0 term of the s2 update is computed once per solve, and the old state is copied only on the last iteration, which is the only one that needs the residual.

**Tried and dropped (slower):** one contiguous block of coefficients per node (+10%), fully unrolling the node loop (+35%), and separate P/D accumulators after vectorizing (+5%). The s2 coefficients are related to `a` (b ≈ a, bd ≈ 2a, g ≈ gd ≈ −2a), but only to about 1e-16, not exactly, so I did not merge them.
```
