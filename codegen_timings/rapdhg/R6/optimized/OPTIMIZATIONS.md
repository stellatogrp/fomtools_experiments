# Optimization report

## Timing

```
translated (emitted)     153.5 us/solve
optimized                 15.4 us/solve
speedup                  9.97x
```

Optimization outcome: **kept (re-verified correct)**.

## Rounds

| round | before (us) | after (us) | gain | kept | agent (s) | outcome |
|---:|---:|---:|---:|:---:|---:|---|
| 1 | 153.502 | 29.599 | 80.7% | yes | 142 | continued |
| 2 | 30.339 | 15.402 | 49.2% | yes | 184 | continued |
| 3 | - | - | - | no | 56 | the round left the sources unchanged |

## Kernel checks

Each `fom_apply_<name>` in the shipped `fom_ops.c`, checked by `make verify` against the package's reference over the same `fom_data.c` matrix. *basis* probes every unit vector, which recovers the routine's matrix column by column (an exact check), plus random vectors; *random* uses random vectors only (the matrix is over the probe budget). A `fused_<block>` row is a `fom_fused_<block>`, checked against the sum of its members: *basis* there probes every unit vector of each source with every schedule value at 0 and with each alone at 1, which recovers every member exactly.

| matrix | shape | nnz | rewritten | check | probes | max error |
|---|---|---|---|---|---|---|
| `t0_state` | 48x160 | 48 | yes | basis | 162 | 0.000e+00 |
| `t0_state__d_taux` | 48x160 | 112 | yes | basis | 162 | 0.000e+00 |
| `t0_state__d_tth` | 48x160 | 112 | yes | basis | 162 | 0.000e+00 |
| `t0_p_p1` | 48x48 | 0 | yes | basis | 50 | 0.000e+00 |
| `t0_p_p1__d_tau` | 48x48 | 48 | yes | basis | 50 | 0.000e+00 |
| `t1_state` | 16x160 | 16 | yes | basis | 162 | 0.000e+00 |
| `t1_state__d_eta` | 16x160 | 256 | yes | basis | 162 | 0.000e+00 |
| `t1_p_p0` | 16x16 | 0 | yes | basis | 18 | 0.000e+00 |
| `t1_p_p0__d_eta` | 16x16 | 16 | yes | basis | 18 | 0.000e+00 |
| `t1_t_t0` | 16x48 | 0 | yes | basis | 50 | 0.000e+00 |
| `t1_t_t0__d_eta` | 16x48 | 112 | yes | basis | 50 | 0.000e+00 |
| `su_state` | 160x160 | 16 | yes | basis | 162 | 0.000e+00 |
| `su_state__d_d1` | 160x160 | 64 | yes | basis | 162 | 0.000e+00 |
| `su_state__d_g2` | 160x160 | 64 | yes | basis | 162 | 0.000e+00 |
| `su_state__d_m1` | 160x160 | 16 | yes | basis | 162 | 0.000e+00 |
| `su_t_t0` | 160x48 | 0 | yes | basis | 50 | 0.000e+00 |
| `su_t_t0__d_d2` | 160x48 | 48 | yes | basis | 50 | 0.000e+00 |
| `su_t_t0__d_g1` | 160x48 | 48 | yes | basis | 50 | 0.000e+00 |
| `su_t_t1` | 160x16 | 0 | yes | basis | 18 | 0.000e+00 |
| `su_t_t1__d_d2` | 160x16 | 16 | yes | basis | 18 | 0.000e+00 |
| `su_t_t1__d_g1` | 160x16 | 16 | yes | basis | 18 | 0.000e+00 |
| `su_t_t1__d_m2` | 160x16 | 16 | yes | basis | 18 | 0.000e+00 |
| `fused_t0` | 48x160 | 272 | yes | basis | 482 | 0.000e+00 |
| `fused_t1` | 16x208 | 384 | yes | basis | 418 | 0.000e+00 |
| `fused_su` | 160x224 | 304 | yes | basis | 1570 | 0.000e+00 |
| `step_t0` | 48x160 | 272 | yes | basis | 486 | 8.882e-16 |
| `step_t1` | 16x208 | 384 | yes | basis | 422 | 1.066e-14 |
| `step_su` | 160x224 | 304 | yes | basis | 1570 | 0.000e+00 |
| `fused_su__rest` | 160x224 | 304 | yes | basis | 1570 | 8.882e-16 |
| `step_su__rest` | 160x224 | 304 | yes | basis | 1570 | 0.000e+00 |
| `iterate` | 160x160 | 960 | yes | basis | 1766 | 5.684e-14 |

### Rule violations

_None: the collected rewrite passed the rules and was kept._

## Blocked by rules

Optimizations the agent considered but did not make because a rule forbade them, in its own words.

| round | idea | rule | expected gain |
|---|---|---|---|
| 1 | Precompute a dense, transposed (column-major) copy of the 16×16 `t1_state__d_eta` block at setup for unit-stride vector loads | `src/fom_ops.c` may not keep setup-time state, and `fom_setup` may not probe matrices; inlining the 256 literals is allowed but did not fit the time budget | ~5–10% of t1, unknown |
| 1 | none other |  |  |
| 2 | Pass the hoisted parameter accumulators and tau/eta into the step routines, so the per-iteration bias assembly fuses into the t0/t1 pass | `include/fom_ops.h` signatures are fixed | ~2-4% |
| 2 | Drop the all-zero su bias read in `fom_step_su__rest` (su has no constant or parameter terms) | the step routine must apply whatever `bias` it is passed, and the header is fixed | ~3-5% |
| 2 | Store the 16x16 `t1_state__d_eta` block transposed or blocked for SIMD | `src/fom_data.c` is fixed, and `fom_setup` may not rebuild matrix data | unknown, likely small |
| 3 | Store a transposed copy of the dense `t1_state__d_eta` block, built once in setup, so the column sweep reads contiguous memory | `fom_setup` and `fom_solver.c` may not read `FOM_MAT_*` data, and `fom_ops.c` may not have non-static or non-entry code reading matrix data | unknown, probably under 5% |
| 3 | Change the build flags (e.g. `-march=native` or `-ffast-math`) to get wider SIMD | the Makefile and build flags are fixed | unknown |

## What the agent changed

```diff
--- src/fom_solver.c (translated)
+++ src/fom_solver.c (optimized)
@@ -86,17 +86,31 @@
         for (i = 0; i < NT0; ++i) bias_t0[i] = acc[A_T0 + i] + tau * acc[B_T0 + i];
         for (i = 0; i < NT1; ++i) bias_t1[i] = acc[A_T1 + i] + eta * acc[B_T1 + i];
 
+        FOM_PROF_BEGIN(t0);
         fom_step_t0(s, taux, tth, bias_t0, &FOM_ACT_t0, t0);
+        FOM_PROF_END(t0);
+        FOM_PROF_BEGIN(t1);
         fom_step_t1(s, t0, eta, bias_t1, &FOM_ACT_t1, t1);
+        FOM_PROF_END(t1);
+        FOM_PROF_BEGIN(su);
         fom_step_su__rest(s, t0, t1, d1, g2, m1, d2, g1, m2, zero_su, ns);
-        for (i = 0; i < FOM_SELROWS_su_N; ++i)
-            ns[FOM_SELROWS_su_ROW[i]] = s[FOM_SELROWS_su_COL[i]];
+        /* selection rows 128..143 copy state[0..15] */
+        for (i = 0; i < 16; ++i) ns[128 + i] = s[i];
+        FOM_PROF_END(su);
 
-        res = 0.0;
-        for (i = 0; i < N; ++i) {
-            fom_float d = ns[i] - s[i];
-            d = d < 0.0 ? -d : d;
-            res = d > res ? d : res;
+#if !FOM_USE_TOL
+        /* only the final residual is reported when tolerance checks are compiled out */
+        if (it + 1 == max_iter)
+#endif
+        {
+            FOM_PROF_BEGIN(residual);
+            res = 0.0;
+            for (i = 0; i < N; ++i) {
+                fom_float d = ns[i] - s[i];
+                d = d < 0.0 ? -d : d;
+                res = d > res ? d : res;
+            }
+            FOM_PROF_END(residual);
         }
         tmp = s; s = ns; ns = tmp;
 #if FOM_USE_TOL
--- src/fom_ops.c (translated)
+++ src/fom_ops.c (optimized)
@@ -266,157 +266,132 @@
 }
 
 void fom_step_t0(const fom_float *state, fom_float sc_taux, fom_float sc_tth, const fom_float *bias, const FomAct_t0 *act, fom_float *out) {
-    fom_int i, r, p;
-    for (r = 0; r < 48; ++r) {
-        fom_float s, v;
-        i = r;
-        v = bias[i];
-        s = 0.0;
-        for (p = FOM_MAT_t0_state_ROWPTR[i]; p < FOM_MAT_t0_state_ROWPTR[i + 1]; ++p) s += FOM_MAT_t0_state_VALS[p] * state[FOM_MAT_t0_state_COLIDX[p]];
-        v += s;
-        s = 0.0;
-        for (p = FOM_MAT_t0_state__d_taux_ROWPTR[i]; p < FOM_MAT_t0_state__d_taux_ROWPTR[i + 1]; ++p) s += FOM_MAT_t0_state__d_taux_VALS[p] * state[FOM_MAT_t0_state__d_taux_COLIDX[p]];
-        v += sc_taux * s;
-        s = 0.0;
-        for (p = FOM_MAT_t0_state__d_tth_ROWPTR[i]; p < FOM_MAT_t0_state__d_tth_ROWPTR[i + 1]; ++p) s += FOM_MAT_t0_state__d_tth_VALS[p] * state[FOM_MAT_t0_state__d_tth_COLIDX[p]];
-        v += sc_tth * s;
-        if (v < act->lo[i]) v = act->lo[i];
+    /* t0_state: out[i] += state[i+16]; t0_state__d_taux = [K 0 ...] on state[0:16],
+       t0_state__d_tth = [... -K ...] on state[128:144]: one product with K on
+       w = taux*state[0:16] - tth*state[128:144]. */
+    fom_float w[16], we[8], wo[8];
+    fom_int i, h, k, m;
+    const fom_float *kv = FOM_MAT_t0_state__d_taux_VALS;
+    for (i = 0; i < 16; ++i) w[i] = sc_taux * state[i] - sc_tth * state[128 + i];
+    /* rows 0..31 of K hold one entry each, at column i mod 16 (entry p = i):
+       a straight vectorizable pass */
+    for (i = 0; i < 16; ++i) {
+        fom_float v = bias[i] + state[i + 16] + kv[i] * w[i];
+        fom_float v2 = bias[i + 16] + state[i + 32] + kv[i + 16] * w[i];
+        v = v < act->lo[i] ? act->lo[i] : v;
+        v2 = v2 < act->lo[i + 16] ? act->lo[i + 16] : v2;
         out[i] = v;
+        out[i + 16] = v2;
+    }
+    /* rows 32..47 of K: two blocks of 8 rows; in each, row pair k (k = 0..3)
+       reads 2(k+1) entries, the even row over odd columns 1,3,.. and the odd
+       row over even columns 0,2,.., stored contiguously from 32 + 40h + 2k(k+1). */
+    for (i = 0; i < 8; ++i) { we[i] = w[2 * i]; wo[i] = w[2 * i + 1]; }
+    for (h = 0; h < 2; ++h) {
+        for (k = 0; k < 4; ++k) {
+            const fom_float *ve = kv + 32 + 40 * h + 2 * k * (k + 1);
+            const fom_float *vo = ve + 2 * (k + 1);
+            const fom_int r = 32 + 8 * h + 2 * k;
+            fom_float se = 0.0, so = 0.0, v;
+            for (m = 0; m < 2 * (k + 1); ++m) { se += ve[m] * wo[m]; so += vo[m] * we[m]; }
+            v = bias[r] + state[r + 16] + se;
+            out[r] = v < act->lo[r] ? act->lo[r] : v;
+            v = bias[r + 1] + state[r + 17] + so;
+            out[r + 1] = v < act->lo[r + 1] ? act->lo[r + 1] : v;
+        }
     }
 }
 
 void fom_step_t1(const fom_float *state, const fom_float *t_t0, fom_float sc_eta, const fom_float *bias, const FomAct_t1 *act, fom_float *out) {
-    fom_int i, r, p;
-    for (r = 0; r < 16; ++r) {
-        fom_float s, v;
-        i = r;
-        v = bias[i];
-        s = 0.0;
-        for (p = FOM_MAT_t1_state_ROWPTR[i]; p < FOM_MAT_t1_state_ROWPTR[i + 1]; ++p) s += FOM_MAT_t1_state_VALS[p] * state[FOM_MAT_t1_state_COLIDX[p]];
-        v += s;
-        s = 0.0;
-        for (p = FOM_MAT_t1_state__d_eta_ROWPTR[i]; p < FOM_MAT_t1_state__d_eta_ROWPTR[i + 1]; ++p) s += FOM_MAT_t1_state__d_eta_VALS[p] * state[FOM_MAT_t1_state__d_eta_COLIDX[p]];
-        v += sc_eta * s;
-        s = 0.0;
-        for (p = FOM_MAT_t1_t_t0__d_eta_ROWPTR[i]; p < FOM_MAT_t1_t_t0__d_eta_ROWPTR[i + 1]; ++p) s += FOM_MAT_t1_t_t0__d_eta_VALS[p] * t_t0[FOM_MAT_t1_t_t0__d_eta_COLIDX[p]];
-        v += sc_eta * s;
+    /* t1_state: out[i] += state[i]; t1_state__d_eta is dense on cols 144..159 */
+    const fom_float *x = state + 144;
+    const fom_float *a = FOM_MAT_t1_state__d_eta_VALS;
+    fom_float acc[16];
+    fom_int i, j, p, g, r;
+    /* 16 independent accumulators instead of 16-long serial dot products */
+    for (i = 0; i < 16; ++i) {
+        const fom_float *r = a + 16 * i;
+        fom_float s0 = r[0] * x[0], s1 = r[1] * x[1], s2 = r[2] * x[2], s3 = r[3] * x[3];
+        for (j = 4; j < 16; j += 4) {
+            s0 += r[j] * x[j]; s1 += r[j + 1] * x[j + 1];
+            s2 += r[j + 2] * x[j + 2]; s3 += r[j + 3] * x[j + 3];
+        }
+        acc[i] = (s0 + s2) + (s1 + s3);
+    }
+    /* every row i of t1_t_t0__d_eta starts with columns i and 16 + i */
+    for (i = 0; i < 16; ++i) {
+        const fom_int q = FOM_MAT_t1_t_t0__d_eta_ROWPTR[i];
+        acc[i] += FOM_MAT_t1_t_t0__d_eta_VALS[q] * t_t0[i] + FOM_MAT_t1_t_t0__d_eta_VALS[q + 1] * t_t0[16 + i];
+    }
+    /* the rest of row 4g + r reads 8 - 2g entries of t_t0[32:48] */
+    for (g = 0; g < 4; ++g) {
+        for (r = 0; r < 4; ++r) {
+            const fom_int q = FOM_MAT_t1_t_t0__d_eta_ROWPTR[4 * g + r] + 2;
+            const fom_float *vv = FOM_MAT_t1_t_t0__d_eta_VALS + q;
+            const fom_int *cc = FOM_MAT_t1_t_t0__d_eta_COLIDX + q;
+            fom_float s = 0.0;
+            for (p = 0; p < 8 - 2 * g; ++p) s += vv[p] * t_t0[cc[p]];
+            acc[4 * g + r] += s;
+        }
+    }
+    for (i = 0; i < 16; ++i) {
+        fom_float v = bias[i] + state[i] + sc_eta * acc[i];
         if (v < act->lo[i]) v = act->lo[i];
         if (v > act->hi[i]) v = act->hi[i];
         out[i] = v;
     }
 }
 
+void fom_step_su__rest(const fom_float *state, const fom_float *t_t0, const fom_float *t_t1, fom_float sc_d1, fom_float sc_g2, fom_float sc_m1, fom_float sc_d2, fom_float sc_g1, fom_float sc_m2, const fom_float *bias, fom_float *out) {
+    /* every su member is a unit shift (see SPEC.md); row 128..143 are the selection rows */
+    fom_int i;
+    for (i = 0; i < 16; ++i) out[i] = bias[i] + sc_g2 * state[i + 64] + sc_g1 * t_t1[i];
+    for (i = 16; i < 64; ++i) out[i] = bias[i] + sc_g2 * state[i + 64] + sc_g1 * t_t0[i - 16];
+    for (i = 64; i < 80; ++i) out[i] = bias[i] + sc_d1 * state[i] + sc_d2 * t_t1[i - 64];
+    for (i = 80; i < 128; ++i) out[i] = bias[i] + sc_d1 * state[i] + sc_d2 * t_t0[i - 80];
+    for (i = 144; i < 160; ++i) out[i] = bias[i] + sc_m1 * state[i - 80] + sc_m2 * t_t1[i - 144];
+}
+
 void fom_step_su(const fom_float *state, const fom_float *t_t0, const fom_float *t_t1, fom_float sc_d1, fom_float sc_g2, fom_float sc_m1, fom_float sc_d2, fom_float sc_g1, fom_float sc_m2, const fom_float *bias, fom_float *out) {
-    fom_int i, r, p;
-    for (r = 0; r < 160; ++r) {
-        fom_float s, v;
-        i = r;
-        v = bias[i];
-        s = 0.0;
-        for (p = FOM_MAT_su_state_ROWPTR[i]; p < FOM_MAT_su_state_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_state_VALS[p] * state[FOM_MAT_su_state_COLIDX[p]];
-        v += s;
-        s = 0.0;
-        for (p = FOM_MAT_su_state__d_d1_ROWPTR[i]; p < FOM_MAT_su_state__d_d1_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_state__d_d1_VALS[p] * state[FOM_MAT_su_state__d_d1_COLIDX[p]];
-        v += sc_d1 * s;
-        s = 0.0;
-        for (p = FOM_MAT_su_state__d_g2_ROWPTR[i]; p < FOM_MAT_su_state__d_g2_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_state__d_g2_VALS[p] * state[FOM_MAT_su_state__d_g2_COLIDX[p]];
-        v += sc_g2 * s;
-        s = 0.0;
-        for (p = FOM_MAT_su_state__d_m1_ROWPTR[i]; p < FOM_MAT_su_state__d_m1_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_state__d_m1_VALS[p] * state[FOM_MAT_su_state__d_m1_COLIDX[p]];
-        v += sc_m1 * s;
-        s = 0.0;
-        for (p = FOM_MAT_su_t_t0__d_d2_ROWPTR[i]; p < FOM_MAT_su_t_t0__d_d2_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t0__d_d2_VALS[p] * t_t0[FOM_MAT_su_t_t0__d_d2_COLIDX[p]];
-        v += sc_d2 * s;
-        s = 0.0;
-        for (p = FOM_MAT_su_t_t0__d_g1_ROWPTR[i]; p < FOM_MAT_su_t_t0__d_g1_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t0__d_g1_VALS[p] * t_t0[FOM_MAT_su_t_t0__d_g1_COLIDX[p]];
-        v += sc_g1 * s;
-        s = 0.0;
-        for (p = FOM_MAT_su_t_t1__d_d2_ROWPTR[i]; p < FOM_MAT_su_t_t1__d_d2_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t1__d_d2_VALS[p] * t_t1[FOM_MAT_su_t_t1__d_d2_COLIDX[p]];
-        v += sc_d2 * s;
-        s = 0.0;
-        for (p = FOM_MAT_su_t_t1__d_g1_ROWPTR[i]; p < FOM_MAT_su_t_t1__d_g1_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t1__d_g1_VALS[p] * t_t1[FOM_MAT_su_t_t1__d_g1_COLIDX[p]];
-        v += sc_g1 * s;
-        s = 0.0;
-        for (p = FOM_MAT_su_t_t1__d_m2_ROWPTR[i]; p < FOM_MAT_su_t_t1__d_m2_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t1__d_m2_VALS[p] * t_t1[FOM_MAT_su_t_t1__d_m2_COLIDX[p]];
-        v += sc_m2 * s;
-        out[i] = v;
-    }
+    fom_int i;
+    fom_step_su__rest(state, t_t0, t_t1, sc_d1, sc_g2, sc_m1, sc_d2, sc_g1, sc_m2, bias, out);
+    for (i = 128; i < 144; ++i) out[i] = bias[i] + state[i - 128];
 }
 
 void fom_fused_su__rest(const fom_float *state, const fom_float *t_t0, const fom_float *t_t1, fom_float sc_d1, fom_float sc_g2, fom_float sc_m1, fom_float sc_d2, fom_float sc_g1, fom_float sc_m2, fom_float *out) {
-    fom_int i, r, p;
-    for (r = 0; r < FOM_REST_su_N; ++r) {
-        fom_float s;
-        i = FOM_REST_su_ROWS[r];
-        s = 0.0;
-        for (p = FOM_MAT_su_state_ROWPTR[i]; p < FOM_MAT_su_state_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_state_VALS[p] * state[FOM_MAT_su_state_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_su_state__d_d1_ROWPTR[i]; p < FOM_MAT_su_state__d_d1_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_state__d_d1_VALS[p] * state[FOM_MAT_su_state__d_d1_COLIDX[p]];
-        out[i] += sc_d1 * s;
-        s = 0.0;
-        for (p = FOM_MAT_su_state__d_g2_ROWPTR[i]; p < FOM_MAT_su_state__d_g2_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_state__d_g2_VALS[p] * state[FOM_MAT_su_state__d_g2_COLIDX[p]];
-        out[i] += sc_g2 * s;
-        s = 0.0;
-        for (p = FOM_MAT_su_state__d_m1_ROWPTR[i]; p < FOM_MAT_su_state__d_m1_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_state__d_m1_VALS[p] * state[FOM_MAT_su_state__d_m1_COLIDX[p]];
-        out[i] += sc_m1 * s;
-        s = 0.0;
-        for (p = FOM_MAT_su_t_t0__d_d2_ROWPTR[i]; p < FOM_MAT_su_t_t0__d_d2_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t0__d_d2_VALS[p] * t_t0[FOM_MAT_su_t_t0__d_d2_COLIDX[p]];
-        out[i] += sc_d2 * s;
-        s = 0.0;
-        for (p = FOM_MAT_su_t_t0__d_g1_ROWPTR[i]; p < FOM_MAT_su_t_t0__d_g1_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t0__d_g1_VALS[p] * t_t0[FOM_MAT_su_t_t0__d_g1_COLIDX[p]];
-        out[i] += sc_g1 * s;
-        s = 0.0;
-        for (p = FOM_MAT_su_t_t1__d_d2_ROWPTR[i]; p < FOM_MAT_su_t_t1__d_d2_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t1__d_d2_VALS[p] * t_t1[FOM_MAT_su_t_t1__d_d2_COLIDX[p]];
-        out[i] += sc_d2 * s;
-        s = 0.0;
-        for (p = FOM_MAT_su_t_t1__d_g1_ROWPTR[i]; p < FOM_MAT_su_t_t1__d_g1_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t1__d_g1_VALS[p] * t_t1[FOM_MAT_su_t_t1__d_g1_COLIDX[p]];
-        out[i] += sc_g1 * s;
-        s = 0.0;
-        for (p = FOM_MAT_su_t_t1__d_m2_ROWPTR[i]; p < FOM_MAT_su_t_t1__d_m2_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t1__d_m2_VALS[p] * t_t1[FOM_MAT_su_t_t1__d_m2_COLIDX[p]];
-        out[i] += sc_m2 * s;
-    }
-}
-
-void fom_step_su__rest(const fom_float *state, const fom_float *t_t0, const fom_float *t_t1, fom_float sc_d1, fom_float sc_g2, fom_float sc_m1, fom_float sc_d2, fom_float sc_g1, fom_float sc_m2, const fom_float *bias, fom_float *out) {
-    fom_int i, r, p;
-    for (r = 0; r < FOM_REST_su_N; ++r) {
-        fom_float s, v;
-        i = FOM_REST_su_ROWS[r];
-        v = bias[i];
-        s = 0.0;
-        for (p = FOM_MAT_su_state_ROWPTR[i]; p < FOM_MAT_su_state_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_state_VALS[p] * state[FOM_MAT_su_state_COLIDX[p]];
-        v += s;
-        s = 0.0;
-        for (p = FOM_MAT_su_state__d_d1_ROWPTR[i]; p < FOM_MAT_su_state__d_d1_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_state__d_d1_VALS[p] * state[FOM_MAT_su_state__d_d1_COLIDX[p]];
-        v += sc_d1 * s;
-        s = 0.0;
-        for (p = FOM_MAT_su_state__d_g2_ROWPTR[i]; p < FOM_MAT_su_state__d_g2_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_state__d_g2_VALS[p] * state[FOM_MAT_su_state__d_g2_COLIDX[p]];
-        v += sc_g2 * s;
-        s = 0.0;
-        for (p = FOM_MAT_su_state__d_m1_ROWPTR[i]; p < FOM_MAT_su_state__d_m1_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_state__d_m1_VALS[p] * state[FOM_MAT_su_state__d_m1_COLIDX[p]];
-        v += sc_m1 * s;
-        s = 0.0;
-        for (p = FOM_MAT_su_t_t0__d_d2_ROWPTR[i]; p < FOM_MAT_su_t_t0__d_d2_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t0__d_d2_VALS[p] * t_t0[FOM_MAT_su_t_t0__d_d2_COLIDX[p]];
-        v += sc_d2 * s;
-        s = 0.0;
-        for (p = FOM_MAT_su_t_t0__d_g1_ROWPTR[i]; p < FOM_MAT_su_t_t0__d_g1_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t0__d_g1_VALS[p] * t_t0[FOM_MAT_su_t_t0__d_g1_COLIDX[p]];
-        v += sc_g1 * s;
-        s = 0.0;
-        for (p = FOM_MAT_su_t_t1__d_d2_ROWPTR[i]; p < FOM_MAT_su_t_t1__d_d2_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t1__d_d2_VALS[p] * t_t1[FOM_MAT_su_t_t1__d_d2_COLIDX[p]];
-        v += sc_d2 * s;
-        s = 0.0;
-        for (p = FOM_MAT_su_t_t1__d_g1_ROWPTR[i]; p < FOM_MAT_su_t_t1__d_g1_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t1__d_g1_VALS[p] * t_t1[FOM_MAT_su_t_t1__d_g1_COLIDX[p]];
-        v += sc_g1 * s;
-        s = 0.0;
-        for (p = FOM_MAT_su_t_t1__d_m2_ROWPTR[i]; p < FOM_MAT_su_t_t1__d_m2_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t1__d_m2_VALS[p] * t_t1[FOM_MAT_su_t_t1__d_m2_COLIDX[p]];
-        v += sc_m2 * s;
-        out[i] = v;
-    }
+    fom_int i;
+    for (i = 0; i < 16; ++i) out[i] += sc_g2 * state[i + 64] + sc_g1 * t_t1[i];
+    for (i = 16; i < 64; ++i) out[i] += sc_g2 * state[i + 64] + sc_g1 * t_t0[i - 16];
+    for (i = 64; i < 80; ++i) out[i] += sc_d1 * state[i] + sc_d2 * t_t1[i - 64];
+    for (i = 80; i < 128; ++i) out[i] += sc_d1 * state[i] + sc_d2 * t_t0[i - 80];
+    for (i = 144; i < 160; ++i) out[i] += sc_m1 * state[i - 80] + sc_m2 * t_t1[i - 144];
 }
 
 void fom_iterate(const fom_float *state, fom_float sc_taux, fom_float sc_tth, fom_float sc_eta, fom_float sc_d1, fom_float sc_g2, fom_float sc_m1, fom_float sc_d2, fom_float sc_g1, fom_float sc_m2, const fom_float *bias_t0, const fom_float *bias_t1, const fom_float *bias_su, const FomAct_t0 *act_t0, const FomAct_t1 *act_t1, fom_float *t_t0, fom_float *t_t1, fom_float *new_state) {
-    fom_step_t0(state, sc_taux, sc_tth, bias_t0, act_t0, t_t0);
-    fom_step_t1(state, t_t0, sc_eta, bias_t1, act_t1, t_t1);
+    /* t1_t_t0__d_eta = -K^T with K the t0_state__d_taux block, so K^T t0 is
+       scattered while t0 is produced (same K rows), and t1 reuses it. */
+    const fom_float *x = state + 144;
+    const fom_float *a = FOM_MAT_t1_state__d_eta_VALS;
+    fom_float w[16], u[16], acc[16];
+    fom_int i, j, p;
+    for (i = 0; i < 16; ++i) { w[i] = sc_taux * state[i] - sc_tth * state[128 + i]; u[i] = 0.0; }
+    for (i = 0; i < 48; ++i) {
+        const fom_int p0 = FOM_MAT_t0_state__d_taux_ROWPTR[i], p1 = FOM_MAT_t0_state__d_taux_ROWPTR[i + 1];
+        fom_float s = 0.0, v;
+        for (p = p0; p < p1; ++p) s += FOM_MAT_t0_state__d_taux_VALS[p] * w[FOM_MAT_t0_state__d_taux_COLIDX[p]];
+        v = bias_t0[i] + state[i + 16] + s;
+        if (v < act_t0->lo[i]) v = act_t0->lo[i];
+        t_t0[i] = v;
+        for (p = p0; p < p1; ++p) u[FOM_MAT_t0_state__d_taux_COLIDX[p]] += FOM_MAT_t0_state__d_taux_VALS[p] * v;
+    }
+    for (i = 0; i < 16; ++i) acc[i] = a[16 * i] * x[0];
+    for (j = 1; j < 16; ++j)
+        for (i = 0; i < 16; ++i) acc[i] += a[16 * i + j] * x[j];
+    for (i = 0; i < 16; ++i) {
+        fom_float v = bias_t1[i] + state[i] + sc_eta * (acc[i] - u[i]);
+        if (v < act_t1->lo[i]) v = act_t1->lo[i];
+        if (v > act_t1->hi[i]) v = act_t1->hi[i];
+        t_t1[i] = v;
+    }
     fom_step_su(state, t_t0, t_t1, sc_d1, sc_g2, sc_m1, sc_d2, sc_g1, sc_m2, bias_su, new_state);
 }
```

## Agent notes

```
### Round 1

**Whole-solve mean_us:** initial 141.33 µs, final ≈ 29.0 µs (three runs: 28.8, 29.0, 30.9). Overall speedup ≈ **4.9×**.

**Final profile breakdown** (`make profile`; the per-region timer overhead inflates the totals, so the profiled mean is 43.9 µs):
| region | total_us | % |
|---|---|---|
| t1 (`fom_step_t1`) | 61982 | 41.6 |
| t0 (`fom_step_t0`) | 55036 | 36.9 |
| su (`fom_step_su__rest` + selection copy) | 31712 | 21.3 |
| residual (final iteration only) | 361 | 0.2 |

**Optimizations applied:**
1. **Shift-structured state update (141 → 67 µs).** Every `su` member is a 0/1 unit shift (see SPEC.md), so I replaced the nine CSR loops over 160 rows with five tight loops, one per row band. Rows 0..15 are `g2*s[i+64] + g1*t1[i]`; rows 16..63 are `g2*s[i+64] + g1*t0[i-16]`; rows 64..79 and 80..127 are `d1*s[i] + d2*t1/t0`; rows 144..159 are `m1*s[i-80] + m2*t1[i-144]`. This applies to `fom_step_su__rest`, `fom_fused_su__rest` and `fom_step_su`.
2. **Selection rows written in the solver.** Rows 128..143 are a plain copy `ns[128+i] = s[i]`, and `__rest` skips them.
3. **Shared block in t0 (67 → 45 µs).** `t0_state__d_tth` on `state[128:144]` is exactly `-K`, where `K` is the `t0_state__d_taux` block on `state[0:16]`. So the step forms `w = taux*s[0:16] - tth*s[128:144]` once and does a single sparse product `K w` (112 nnz) instead of two. `t0_state` is a shift, so it becomes the plain add `state[i+16]`.
4. **t1 structure.** `t1_state` is a shift (`+state[i]`). `t1_state__d_eta` is dense 16×16 on columns 144..159, and both eta terms share one scale. The dense product now uses 16 independent accumulators, ordered column-major, instead of 16 serial 16-long dot products. This breaks the FP add latency chain, since without fast-math the compiler won't reassociate.
5. **Residual only on the last iteration (45 → 29 µs, combined with the change above).** `FOM_USE_TOL = 0`, so only the final residual is reported. The 160-element max-abs pass now runs once instead of 200 times.
6. **Tried and reverted:** a fully fused `fom_iterate`. It used `t1_t_t0__d_eta = -Kᵀ` to scatter `Kᵀt0` during the t0 pass. It was slower (≈32 µs) because the scatter creates a store-to-load dependency chain.

Blocked by rules:
- Precompute a dense, transposed (column-major) copy of the 16×16 `t1_state__d_eta` block at setup for unit-stride vector loads | `src/fom_ops.c` may not keep setup-time state, and `fom_setup` may not probe matrices; inlining the 256 literals is allowed but did not fit the time budget | ~5–10% of t1, unknown
- none other

### Round 2

**Whole-solve time:** 29.6 us at the start of this round (153.5 us for the translated solver), 15.5 us at the end (`make bench` mean_us, three runs). That is 1.91x this round and 9.9x overall.

**Final profile (`make profile`), hottest first:**

| Region | Time | Share |
|---|---|---|
| su | 32145 us | 36.7% |
| t1 | 31469 us | 35.9% |
| t0 | 23657 us | 27.0% |
| residual | 402 us | 0.5% |

**Optimizations applied (all in `src/fom_ops.c`):**
- **t0, rows 0..31 of K.** K is the 48x16 `t0_state__d_taux` block. Each of these 32 rows holds one entry, at column `i mod 16`, stored at position p = i. The CSR loop over them became one straight pass that also adds the bias and applies the relu.
- **t0, rows 32..47 of K.** These form two identical 8-row groups. Rows 2k and 2k+1 of a group each hold 2(k+1) entries. The even row reads the odd columns of w and the odd row reads the even columns, stored contiguously from offset 32 + 40h + 2k(k+1). I split w into its even and odd entries once, so each row became a contiguous dot product with a fixed trip count, with no index gathers and no row-pointer loads.
- **t1, the dense 16x16 `t1_state__d_eta` block.** It was a column-sweep (each step jumps 16 entries ahead). It is now a row-contiguous dot product with 4 independent accumulators, which pairs well with SIMD (single instruction, multiple data) loads. This was the biggest single gain, about 25.1 to 21 us.
- **t1, `t1_t_t0__d_eta` structure.** Every row i starts with columns i and 16 + i, so those two terms are done in one vectorizable pass. The rest of row 4g + r always has 8 - 2g entries, so that loop now has a fixed trip count.

**Tried and reverted (slower or no gain):**
- Calling `fom_iterate`, which fuses everything and scatters Kᵀt0 for t1: 29.6 to 33.5 us. The scatter is a serial chain of memory updates.
- Writing the su update as plain copies in `src/fom_solver.c`: 15.5 to 17.3 us. The likely cause is that the compiler must assume the solver's arrays may overlap.
- Local `restrict` pointers in `fom_step_su__rest`: no change, so I left the code simpler.

Blocked by rules:
- Pass the hoisted parameter accumulators and tau/eta into the step routines, so the per-iteration bias assembly fuses into the t0/t1 pass | `include/fom_ops.h` signatures are fixed | ~2-4%
- Drop the all-zero su bias read in `fom_step_su__rest` (su has no constant or parameter terms) | the step routine must apply whatever `bias` it is passed, and the header is fixed | ~3-5%
- Store the 16x16 `t1_state__d_eta` block transposed or blocked for SIMD | `src/fom_data.c` is fixed, and `fom_setup` may not rebuild matrix data | unknown, likely small

### Round 3

No speedup this round, so I left the sources as they were after round 2. They still pass `make verify`.

**Timing**
- Translated solver: 153.502 us per solve.
- Start of this round: 15.402 us (re-measured at 15.56 us).
- End of this round: 15.56 us, the same code.
- Overall speedup: about 9.9x, unchanged by this round.

**Final profile** (`make profile`; the timer calls themselves dominate at about 77 ns per iteration):

| Region | Time (us) | Share |
|---|---|---|
| su | 32352 | 36.8% |
| t1 | 31338 | 35.7% |
| t0 | 23774 | 27.1% |
| residual | 391 | 0.45% |

**What I tried this round (both passed verify, both reverted)**
- **t1 loop order.** I swept the dense 16x16 block of `t1_state__d_eta` (columns 144..159) column by column, broadcasting one input entry at a time. It got slower, 19.3 us against 15.6 us. The stride-16 access defeats vectorization, so the version with 4 accumulators per row stays.
- **One call per iteration.** I chained `fom_step_t0`, `fom_step_t1` and `fom_step_su` inside `fom_iterate` and called it once per iteration so they could inline together. It ran at 15.57 us against 15.56 us, so no gain.

**Optimizations already in place from earlier rounds**
- **t0:** the three `t0` products collapse into one product with K, applied to w = taux·s[0:16] − tth·s[128:144]. This works because the `t0_state__d_tth` block is −K and `t0_state` is a unit shift. The fixed row pattern of K is unrolled.
- **t1:** the dense 16x16 block uses independent accumulators. The `t1_t_t0__d_eta` rows use their fixed column pattern.
- **su:** every member is a unit shift, so the update is five straight vector loops. The selection rows 128..143 are copied in the solver.
- **Hoisting and fusion:** parameter terms are computed once per solve. The bias and activation are folded into the step routines, and the residual is computed only on the last iteration when tolerance checks are off.

Blocked by rules:
- Store a transposed copy of the dense `t1_state__d_eta` block, built once in setup, so the column sweep reads contiguous memory | `fom_setup` and `fom_solver.c` may not read `FOM_MAT_*` data, and `fom_ops.c` may not have non-static or non-entry code reading matrix data | unknown, probably under 5%
- Change the build flags (e.g. `-march=native` or `-ffast-math`) to get wider SIMD | the Makefile and build flags are fixed | unknown
```
