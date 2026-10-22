# Optimization report

## Timing

```
translated (emitted)     127.1 us/solve
optimized                 17.2 us/solve
speedup                  7.38x
```

Optimization outcome: **kept (re-verified correct)**.

## Rounds

| round | before (us) | after (us) | gain | kept | agent (s) | outcome |
|---:|---:|---:|---:|:---:|---:|---|
| 1 | 127.135 | 29.490 | 76.8% | yes | 105 | continued |
| 2 | 30.758 | 15.639 | 49.2% | yes | 97 | continued |
| 3 | 17.720 | 17.227 | 2.8% | yes | 42 | continued |
| 4 | - | - | - | no | 170 | the round left the sources unchanged |

## Kernel checks

Each `fom_apply_<name>` in the shipped `fom_ops.c`, checked by `make verify` against the package's reference over the same `fom_data.c` matrix. *basis* probes every unit vector, which recovers the routine's matrix column by column (an exact check), plus random vectors; *random* uses random vectors only (the matrix is over the probe budget). A `fused_<block>` row is a `fom_fused_<block>`, checked against the sum of its members: *basis* there probes every unit vector of each source with every schedule value at 0 and with each alone at 1, which recovers every member exactly.

| matrix | shape | nnz | rewritten | check | probes | max error |
|---|---|---|---|---|---|---|
| `t0_state` | 48x160 | 48 | no | basis | 162 | 0.000e+00 |
| `t0_state__d_taux` | 48x160 | 112 | no | basis | 162 | 0.000e+00 |
| `t0_state__d_tth` | 48x160 | 112 | no | basis | 162 | 0.000e+00 |
| `t0_p_p1` | 48x48 | 0 | no | basis | 50 | 0.000e+00 |
| `t0_p_p1__d_tau` | 48x48 | 48 | no | basis | 50 | 0.000e+00 |
| `t1_state` | 16x160 | 16 | no | basis | 162 | 0.000e+00 |
| `t1_state__d_eta` | 16x160 | 256 | no | basis | 162 | 0.000e+00 |
| `t1_p_p0` | 16x16 | 0 | no | basis | 18 | 0.000e+00 |
| `t1_p_p0__d_eta` | 16x16 | 16 | no | basis | 18 | 0.000e+00 |
| `t1_t_t0` | 16x48 | 0 | no | basis | 50 | 0.000e+00 |
| `t1_t_t0__d_eta` | 16x48 | 112 | no | basis | 50 | 0.000e+00 |
| `su_state` | 160x160 | 16 | no | basis | 162 | 0.000e+00 |
| `su_state__d_d1` | 160x160 | 64 | no | basis | 162 | 0.000e+00 |
| `su_state__d_g2` | 160x160 | 64 | no | basis | 162 | 0.000e+00 |
| `su_state__d_m1` | 160x160 | 16 | no | basis | 162 | 0.000e+00 |
| `su_t_t0` | 160x48 | 0 | no | basis | 50 | 0.000e+00 |
| `su_t_t0__d_d2` | 160x48 | 48 | no | basis | 50 | 0.000e+00 |
| `su_t_t0__d_g1` | 160x48 | 48 | no | basis | 50 | 0.000e+00 |
| `su_t_t1` | 160x16 | 0 | no | basis | 18 | 0.000e+00 |
| `su_t_t1__d_d2` | 160x16 | 16 | no | basis | 18 | 0.000e+00 |
| `su_t_t1__d_g1` | 160x16 | 16 | no | basis | 18 | 0.000e+00 |
| `su_t_t1__d_m2` | 160x16 | 16 | no | basis | 18 | 0.000e+00 |
| `fused_t0` | 48x160 | 272 | no | basis | 482 | 0.000e+00 |
| `fused_t1` | 16x208 | 384 | no | basis | 418 | 0.000e+00 |
| `fused_su` | 160x224 | 304 | no | basis | 1570 | 0.000e+00 |
| `step_t0` | 48x160 | 272 | no | basis | 486 | 0.000e+00 |
| `step_t1` | 16x208 | 384 | no | basis | 422 | 0.000e+00 |
| `step_su` | 160x224 | 304 | no | basis | 1570 | 0.000e+00 |
| `fused_su__rest` | 160x224 | 304 | no | basis | 1570 | 0.000e+00 |
| `step_su__rest` | 160x224 | 304 | no | basis | 1570 | 0.000e+00 |
| `iterate` | 160x160 | 960 | yes | basis | 1766 | 8.527e-14 |

### Rule violations

_None: the collected rewrite passed the rules and was kept._

## Blocked by rules

Optimizations the agent considered but did not make because a rule forbade them, in its own words.

| round | idea | rule | expected gain |
|---|---|---|---|
| 1 | Fold `tau*q0` and `eta*q1` into `fom_iterate` to remove the separate `bias` pass (about 12% of profiled time) | the fixed `fom_iterate` signature takes only a precomputed bias, and `src/fom_ops.c` may not read the schedule | ~5–10% |
| 1 | Unroll `t1_t_t0__d_eta` and `t0_state__d_taux` with inlined literal coefficients, exploiting their repeated ±0.0726/0.2177/... banded pattern (they look like transposes of each other, a PDHG K / Kᵀ pair) | allowed in `fom_iterate`, but not done for lack of time within the 10-minute budget, not because a rule forbids it | unknown, likely ~10–20% |
| 2 | Add the `tau*q0` / `eta*q1` parameter terms to the bias inside `fom_iterate`, reading the hoisted terms and the schedule directly, which removes the separate bias pass | `fom_iterate` signature is fixed and `src/fom_ops.c` may not read the schedule | ~5% |
| 2 | Compile with `-ffast-math` / `-march=native` so the dense dot-product reductions vectorize | Makefile and build flags are fixed | unknown, likely <5% |
| 3 | Fold the `tau*q0` and `eta*q1` bias scaling into `fom_iterate` and remove the separate bias pass | fixed `fom_iterate` signature in `include/fom_ops.h` (headers not editable) | ~10-20% (the bias region's share) |
| 3 | Stop reading the all-zero state-update bias (`ZERO_SU`) inside `fom_iterate` | verify checks `fom_iterate` with random biases, so the routine must honour `bias_su` | ~1-3% |
| 4 | Pass `q0`, `q1` and `tau`/`eta` into `fom_iterate` so the per-iteration `bias = tau*q` scaling folds into the product pass (removes the separate 64-element bias loop) | fixed `fom_iterate` signature in `include/fom_ops.h` (headers are fixed) | about 2-5% |
| 4 | Precompute every scheduled bias `tau(it)*q0`, `eta(it)*q1` for all 200 iterations in `fom_update_params` | workspace layout in `include/fom_workspace.h` is fixed and has no scratch space for it | about 2-5% |

## What the agent changed

```diff
--- src/fom_solver.c (translated)
+++ src/fom_solver.c (optimized)
@@ -9,6 +9,9 @@
 #define S_DIM FOM_STATE_DIM
 #define T0_DIM FOM_T0_DIM
 #define T1_DIM FOM_T1_DIM
+
+/* The state update has no constant term. */
+static const fom_float ZERO_SU[FOM_STATE_DIM] = {0.0};
 
 void fom_set_default_settings(FomSettings *settings) {
     settings->max_iter = FOM_MAX_ITER;
@@ -49,9 +52,8 @@
 
     memcpy(x, FOM_STATE_INIT, sizeof(work->state));
     memset(b0, 0, (size_t)(2 * T0_DIM + 2 * T1_DIM) * sizeof(fom_float));
-    fom_apply_t0_p_p1(work->params.p1, b0);
+    /* t0_p_p1 and t1_p_p0 are empty (nnz 0): their terms vanish. */
     fom_apply_t0_p_p1__d_tau(work->params.p1, q0);
-    fom_apply_t1_p_p0(work->params.p0, b1);
     fom_apply_t1_p_p0__d_eta(work->params.p0, q1);
 
     for (it = 0; it < max_iter; ++it) {
@@ -67,16 +69,15 @@
         const fom_float tth = fom_sched_tth(it);
         fom_float *tmp;
 
-        for (i = 0; i < T0_DIM; ++i) bias0[i] = b0[i] + tau * q0[i];
-        for (i = 0; i < T1_DIM; ++i) bias1[i] = b1[i] + eta * q1[i];
+        FOM_PROF_BEGIN(bias);
+        for (i = 0; i < T0_DIM; ++i) bias0[i] = tau * q0[i];
+        for (i = 0; i < T1_DIM; ++i) bias1[i] = eta * q1[i];
+        FOM_PROF_END(bias);
 
-        fom_step_t0(x, taux, tth, bias0, &FOM_ACT_t0, t0);
-        fom_step_t1(x, t0, eta, bias1, &FOM_ACT_t1, t1);
-
-        memset(xn, 0, sizeof(work->new_state));
-        fom_fused_su__rest(x, t0, t1, d1, g2, m1, d2, g1, m2, xn);
-        for (i = 0; i < FOM_SELROWS_su_N; ++i)
-            xn[FOM_SELROWS_su_ROW[i]] = x[FOM_SELROWS_su_COL[i]];
+        FOM_PROF_BEGIN(iterate);
+        fom_iterate(x, taux, tth, eta, d1, g2, m1, d2, g1, m2, bias0, bias1, ZERO_SU,
+                    &FOM_ACT_t0, &FOM_ACT_t1, t0, t1, xn);
+        FOM_PROF_END(iterate);
 
         if (it == max_iter - 1) {
             res = 0.0;
--- src/fom_ops.c (translated)
+++ src/fom_ops.c (optimized)
@@ -416,7 +416,203 @@
 }
 
 void fom_iterate(const fom_float *state, fom_float sc_taux, fom_float sc_tth, fom_float sc_eta, fom_float sc_d1, fom_float sc_g2, fom_float sc_m1, fom_float sc_d2, fom_float sc_g1, fom_float sc_m2, const fom_float *bias_t0, const fom_float *bias_t1, const fom_float *bias_su, const FomAct_t0 *act_t0, const FomAct_t1 *act_t1, fom_float *t_t0, fom_float *t_t1, fom_float *new_state) {
-    fom_step_t0(state, sc_taux, sc_tth, bias_t0, act_t0, t_t0);
-    fom_step_t1(state, t_t0, sc_eta, bias_t1, act_t1, t_t1);
-    fom_step_su(state, t_t0, t_t1, sc_d1, sc_g2, sc_m1, sc_d2, sc_g1, sc_m2, bias_su, new_state);
-}
+    fom_int i;
+    /* t0: shift state[i+16] plus taux*A x[0:16] + tth*B x[128:144], where
+       B is -A with columns shifted by 128 (same pattern, negated values), so
+       both terms collapse to one CSR pass over w = taux*x[c] - tth*x[c+128]. */
+    {
+        fom_float w[16];
+        for (i = 0; i < 16; ++i) w[i] = sc_taux * state[i] - sc_tth * state[i + 128];
+        fom_float v;
+        /* Unrolled over the fixed sparsity of t0_state__d_taux. */
+        v = bias_t0[0] + state[16] + (w[0]);
+        if (v < act_t0->lo[0]) v = act_t0->lo[0];
+        t_t0[0] = v;
+        v = bias_t0[1] + state[17] + (w[1]);
+        if (v < act_t0->lo[1]) v = act_t0->lo[1];
+        t_t0[1] = v;
+        v = bias_t0[2] + state[18] + (w[2]);
+        if (v < act_t0->lo[2]) v = act_t0->lo[2];
+        t_t0[2] = v;
+        v = bias_t0[3] + state[19] + (w[3]);
+        if (v < act_t0->lo[3]) v = act_t0->lo[3];
+        t_t0[3] = v;
+        v = bias_t0[4] + state[20] + (w[4]);
+        if (v < act_t0->lo[4]) v = act_t0->lo[4];
+        t_t0[4] = v;
+        v = bias_t0[5] + state[21] + (w[5]);
+        if (v < act_t0->lo[5]) v = act_t0->lo[5];
+        t_t0[5] = v;
+        v = bias_t0[6] + state[22] + (w[6]);
+        if (v < act_t0->lo[6]) v = act_t0->lo[6];
+        t_t0[6] = v;
+        v = bias_t0[7] + state[23] + (w[7]);
+        if (v < act_t0->lo[7]) v = act_t0->lo[7];
+        t_t0[7] = v;
+        v = bias_t0[8] + state[24] + (w[8]);
+        if (v < act_t0->lo[8]) v = act_t0->lo[8];
+        t_t0[8] = v;
+        v = bias_t0[9] + state[25] + (w[9]);
+        if (v < act_t0->lo[9]) v = act_t0->lo[9];
+        t_t0[9] = v;
+        v = bias_t0[10] + state[26] + (w[10]);
+        if (v < act_t0->lo[10]) v = act_t0->lo[10];
+        t_t0[10] = v;
+        v = bias_t0[11] + state[27] + (w[11]);
+        if (v < act_t0->lo[11]) v = act_t0->lo[11];
+        t_t0[11] = v;
+        v = bias_t0[12] + state[28] + (w[12]);
+        if (v < act_t0->lo[12]) v = act_t0->lo[12];
+        t_t0[12] = v;
+        v = bias_t0[13] + state[29] + (w[13]);
+        if (v < act_t0->lo[13]) v = act_t0->lo[13];
+        t_t0[13] = v;
+        v = bias_t0[14] + state[30] + (w[14]);
+        if (v < act_t0->lo[14]) v = act_t0->lo[14];
+        t_t0[14] = v;
+        v = bias_t0[15] + state[31] + (w[15]);
+        if (v < act_t0->lo[15]) v = act_t0->lo[15];
+        t_t0[15] = v;
+        v = bias_t0[16] + state[32] + (-w[0]);
+        if (v < act_t0->lo[16]) v = act_t0->lo[16];
+        t_t0[16] = v;
+        v = bias_t0[17] + state[33] + (-w[1]);
+        if (v < act_t0->lo[17]) v = act_t0->lo[17];
+        t_t0[17] = v;
+        v = bias_t0[18] + state[34] + (-w[2]);
+        if (v < act_t0->lo[18]) v = act_t0->lo[18];
+        t_t0[18] = v;
+        v = bias_t0[19] + state[35] + (-w[3]);
+        if (v < act_t0->lo[19]) v = act_t0->lo[19];
+        t_t0[19] = v;
+        v = bias_t0[20] + state[36] + (-w[4]);
+        if (v < act_t0->lo[20]) v = act_t0->lo[20];
+        t_t0[20] = v;
+        v = bias_t0[21] + state[37] + (-w[5]);
+        if (v < act_t0->lo[21]) v = act_t0->lo[21];
+        t_t0[21] = v;
+        v = bias_t0[22] + state[38] + (-w[6]);
+        if (v < act_t0->lo[22]) v = act_t0->lo[22];
+        t_t0[22] = v;
+        v = bias_t0[23] + state[39] + (-w[7]);
+        if (v < act_t0->lo[23]) v = act_t0->lo[23];
+        t_t0[23] = v;
+        v = bias_t0[24] + state[40] + (-w[8]);
+        if (v < act_t0->lo[24]) v = act_t0->lo[24];
+        t_t0[24] = v;
+        v = bias_t0[25] + state[41] + (-w[9]);
+        if (v < act_t0->lo[25]) v = act_t0->lo[25];
+        t_t0[25] = v;
+        v = bias_t0[26] + state[42] + (-w[10]);
+        if (v < act_t0->lo[26]) v = act_t0->lo[26];
+        t_t0[26] = v;
+        v = bias_t0[27] + state[43] + (-w[11]);
+        if (v < act_t0->lo[27]) v = act_t0->lo[27];
+        t_t0[27] = v;
+        v = bias_t0[28] + state[44] + (-w[12]);
+        if (v < act_t0->lo[28]) v = act_t0->lo[28];
+        t_t0[28] = v;
+        v = bias_t0[29] + state[45] + (-w[13]);
+        if (v < act_t0->lo[29]) v = act_t0->lo[29];
+        t_t0[29] = v;
+        v = bias_t0[30] + state[46] + (-w[14]);
+        if (v < act_t0->lo[30]) v = act_t0->lo[30];
+        t_t0[30] = v;
+        v = bias_t0[31] + state[47] + (-w[15]);
+        if (v < act_t0->lo[31]) v = act_t0->lo[31];
+        t_t0[31] = v;
+        v = bias_t0[32] + state[48] + ((-0x1.295e9e1b0899fp-4) * w[1] + (0x1.295e9e1b0899fp-4) * w[3]);
+        if (v < act_t0->lo[32]) v = act_t0->lo[32];
+        t_t0[32] = v;
+        v = bias_t0[33] + state[49] + ((-0x1.295e9e1b0899fp-4) * w[0] + (0x1.295e9e1b0899fp-4) * w[2]);
+        if (v < act_t0->lo[33]) v = act_t0->lo[33];
+        t_t0[33] = v;
+        v = bias_t0[34] + state[50] + ((-0x1.bde3fbbd7b203p-3) * w[1] + (0x1.bde3fbbd7b203p-3) * w[3] + (-0x1.295e9e1b0899fp-4) * w[5] + (0x1.295e9e1b0899fp-4) * w[7]);
+        if (v < act_t0->lo[34]) v = act_t0->lo[34];
+        t_t0[34] = v;
+        v = bias_t0[35] + state[51] + ((-0x1.bde3fbbd7b203p-3) * w[0] + (0x1.bde3fbbd7b203p-3) * w[2] + (-0x1.295e9e1b0899fp-4) * w[4] + (0x1.295e9e1b0899fp-4) * w[6]);
+        if (v < act_t0->lo[35]) v = act_t0->lo[35];
+        t_t0[35] = v;
+        v = bias_t0[36] + state[52] + ((-0x1.738c5436b8f9bp-2) * w[1] + (0x1.738c5436b8f9bp-2) * w[3] + (-0x1.bde3fbbd7b203p-3) * w[5] + (0x1.bde3fbbd7b203p-3) * w[7] + (-0x1.295e9e1b0899fp-4) * w[9] + (0x1.295e9e1b0899fp-4) * w[11]);
+        if (v < act_t0->lo[36]) v = act_t0->lo[36];
+        t_t0[36] = v;
+        v = bias_t0[37] + state[53] + ((-0x1.738c5436b8f9bp-2) * w[0] + (0x1.738c5436b8f9bp-2) * w[2] + (-0x1.bde3fbbd7b203p-3) * w[4] + (0x1.bde3fbbd7b203p-3) * w[6] + (-0x1.295e9e1b0899fp-4) * w[8] + (0x1.295e9e1b0899fp-4) * w[10]);
+        if (v < act_t0->lo[37]) v = act_t0->lo[37];
+        t_t0[37] = v;
+        v = bias_t0[38] + state[54] + ((-0x1.041355475a31ap-1) * w[1] + (0x1.041355475a31ap-1) * w[3] + (-0x1.738c5436b8f9bp-2) * w[5] + (0x1.738c5436b8f9bp-2) * w[7] + (-0x1.bde3fbbd7b203p-3) * w[9] + (0x1.bde3fbbd7b203p-3) * w[11] + (-0x1.295e9e1b0899fp-4) * w[13] + (0x1.295e9e1b0899fp-4) * w[15]);
+        if (v < act_t0->lo[38]) v = act_t0->lo[38];
+        t_t0[38] = v;
+        v = bias_t0[39] + state[55] + ((-0x1.041355475a31ap-1) * w[0] + (0x1.041355475a31ap-1) * w[2] + (-0x1.738c5436b8f9bp-2) * w[4] + (0x1.738c5436b8f9bp-2) * w[6] + (-0x1.bde3fbbd7b203p-3) * w[8] + (0x1.bde3fbbd7b203p-3) * w[10] + (-0x1.295e9e1b0899fp-4) * w[12] + (0x1.295e9e1b0899fp-4) * w[14]);
+        if (v < act_t0->lo[39]) v = act_t0->lo[39];
+        t_t0[39] = v;
+        v = bias_t0[40] + state[56] + ((0x1.295e9e1b0899fp-4) * w[1] + (-0x1.295e9e1b0899fp-4) * w[3]);
+        if (v < act_t0->lo[40]) v = act_t0->lo[40];
+        t_t0[40] = v;
+        v = bias_t0[41] + state[57] + ((0x1.295e9e1b0899fp-4) * w[0] + (-0x1.295e9e1b0899fp-4) * w[2]);
+        if (v < act_t0->lo[41]) v = act_t0->lo[41];
+        t_t0[41] = v;
+        v = bias_t0[42] + state[58] + ((0x1.bde3fbbd7b203p-3) * w[1] + (-0x1.bde3fbbd7b203p-3) * w[3] + (0x1.295e9e1b0899fp-4) * w[5] + (-0x1.295e9e1b0899fp-4) * w[7]);
+        if (v < act_t0->lo[42]) v = act_t0->lo[42];
+        t_t0[42] = v;
+        v = bias_t0[43] + state[59] + ((0x1.bde3fbbd7b203p-3) * w[0] + (-0x1.bde3fbbd7b203p-3) * w[2] + (0x1.295e9e1b0899fp-4) * w[4] + (-0x1.295e9e1b0899fp-4) * w[6]);
+        if (v < act_t0->lo[43]) v = act_t0->lo[43];
+        t_t0[43] = v;
+        v = bias_t0[44] + state[60] + ((0x1.738c5436b8f9bp-2) * w[1] + (-0x1.738c5436b8f9bp-2) * w[3] + (0x1.bde3fbbd7b203p-3) * w[5] + (-0x1.bde3fbbd7b203p-3) * w[7] + (0x1.295e9e1b0899fp-4) * w[9] + (-0x1.295e9e1b0899fp-4) * w[11]);
+        if (v < act_t0->lo[44]) v = act_t0->lo[44];
+        t_t0[44] = v;
+        v = bias_t0[45] + state[61] + ((0x1.738c5436b8f9bp-2) * w[0] + (-0x1.738c5436b8f9bp-2) * w[2] + (0x1.bde3fbbd7b203p-3) * w[4] + (-0x1.bde3fbbd7b203p-3) * w[6] + (0x1.295e9e1b0899fp-4) * w[8] + (-0x1.295e9e1b0899fp-4) * w[10]);
+        if (v < act_t0->lo[45]) v = act_t0->lo[45];
+        t_t0[45] = v;
+        v = bias_t0[46] + state[62] + ((0x1.041355475a31ap-1) * w[1] + (-0x1.041355475a31ap-1) * w[3] + (0x1.738c5436b8f9bp-2) * w[5] + (-0x1.738c5436b8f9bp-2) * w[7] + (0x1.bde3fbbd7b203p-3) * w[9] + (-0x1.bde3fbbd7b203p-3) * w[11] + (0x1.295e9e1b0899fp-4) * w[13] + (-0x1.295e9e1b0899fp-4) * w[15]);
+        if (v < act_t0->lo[46]) v = act_t0->lo[46];
+        t_t0[46] = v;
+        v = bias_t0[47] + state[63] + ((0x1.041355475a31ap-1) * w[0] + (-0x1.041355475a31ap-1) * w[2] + (0x1.738c5436b8f9bp-2) * w[4] + (-0x1.738c5436b8f9bp-2) * w[6] + (0x1.bde3fbbd7b203p-3) * w[8] + (-0x1.bde3fbbd7b203p-3) * w[10] + (0x1.295e9e1b0899fp-4) * w[12] + (-0x1.295e9e1b0899fp-4) * w[14]);
+        if (v < act_t0->lo[47]) v = act_t0->lo[47];
+        t_t0[47] = v;
+    }
+    /* t1_t_t0__d_eta @ t0, unrolled over its fixed sparsity. */
+    fom_float ss[16];
+    ss[0] = -t_t0[0] + t_t0[16] + (0x1.295e9e1b0899fp-4) * t_t0[33] + (0x1.bde3fbbd7b203p-3) * t_t0[35] + (0x1.738c5436b8f9bp-2) * t_t0[37] + (0x1.041355475a31ap-1) * t_t0[39] + (-0x1.295e9e1b0899fp-4) * t_t0[41] + (-0x1.bde3fbbd7b203p-3) * t_t0[43] + (-0x1.738c5436b8f9bp-2) * t_t0[45] + (-0x1.041355475a31ap-1) * t_t0[47];
+    ss[1] = -t_t0[1] + t_t0[17] + (0x1.295e9e1b0899fp-4) * t_t0[32] + (0x1.bde3fbbd7b203p-3) * t_t0[34] + (0x1.738c5436b8f9bp-2) * t_t0[36] + (0x1.041355475a31ap-1) * t_t0[38] + (-0x1.295e9e1b0899fp-4) * t_t0[40] + (-0x1.bde3fbbd7b203p-3) * t_t0[42] + (-0x1.738c5436b8f9bp-2) * t_t0[44] + (-0x1.041355475a31ap-1) * t_t0[46];
+    ss[2] = -t_t0[2] + t_t0[18] + (-0x1.295e9e1b0899fp-4) * t_t0[33] + (-0x1.bde3fbbd7b203p-3) * t_t0[35] + (-0x1.738c5436b8f9bp-2) * t_t0[37] + (-0x1.041355475a31ap-1) * t_t0[39] + (0x1.295e9e1b0899fp-4) * t_t0[41] + (0x1.bde3fbbd7b203p-3) * t_t0[43] + (0x1.738c5436b8f9bp-2) * t_t0[45] + (0x1.041355475a31ap-1) * t_t0[47];
+    ss[3] = -t_t0[3] + t_t0[19] + (-0x1.295e9e1b0899fp-4) * t_t0[32] + (-0x1.bde3fbbd7b203p-3) * t_t0[34] + (-0x1.738c5436b8f9bp-2) * t_t0[36] + (-0x1.041355475a31ap-1) * t_t0[38] + (0x1.295e9e1b0899fp-4) * t_t0[40] + (0x1.bde3fbbd7b203p-3) * t_t0[42] + (0x1.738c5436b8f9bp-2) * t_t0[44] + (0x1.041355475a31ap-1) * t_t0[46];
+    ss[4] = -t_t0[4] + t_t0[20] + (0x1.295e9e1b0899fp-4) * t_t0[35] + (0x1.bde3fbbd7b203p-3) * t_t0[37] + (0x1.738c5436b8f9bp-2) * t_t0[39] + (-0x1.295e9e1b0899fp-4) * t_t0[43] + (-0x1.bde3fbbd7b203p-3) * t_t0[45] + (-0x1.738c5436b8f9bp-2) * t_t0[47];
+    ss[5] = -t_t0[5] + t_t0[21] + (0x1.295e9e1b0899fp-4) * t_t0[34] + (0x1.bde3fbbd7b203p-3) * t_t0[36] + (0x1.738c5436b8f9bp-2) * t_t0[38] + (-0x1.295e9e1b0899fp-4) * t_t0[42] + (-0x1.bde3fbbd7b203p-3) * t_t0[44] + (-0x1.738c5436b8f9bp-2) * t_t0[46];
+    ss[6] = -t_t0[6] + t_t0[22] + (-0x1.295e9e1b0899fp-4) * t_t0[35] + (-0x1.bde3fbbd7b203p-3) * t_t0[37] + (-0x1.738c5436b8f9bp-2) * t_t0[39] + (0x1.295e9e1b0899fp-4) * t_t0[43] + (0x1.bde3fbbd7b203p-3) * t_t0[45] + (0x1.738c5436b8f9bp-2) * t_t0[47];
+    ss[7] = -t_t0[7] + t_t0[23] + (-0x1.295e9e1b0899fp-4) * t_t0[34] + (-0x1.bde3fbbd7b203p-3) * t_t0[36] + (-0x1.738c5436b8f9bp-2) * t_t0[38] + (0x1.295e9e1b0899fp-4) * t_t0[42] + (0x1.bde3fbbd7b203p-3) * t_t0[44] + (0x1.738c5436b8f9bp-2) * t_t0[46];
+    ss[8] = -t_t0[8] + t_t0[24] + (0x1.295e9e1b0899fp-4) * t_t0[37] + (0x1.bde3fbbd7b203p-3) * t_t0[39] + (-0x1.295e9e1b0899fp-4) * t_t0[45] + (-0x1.bde3fbbd7b203p-3) * t_t0[47];
+    ss[9] = -t_t0[9] + t_t0[25] + (0x1.295e9e1b0899fp-4) * t_t0[36] + (0x1.bde3fbbd7b203p-3) * t_t0[38] + (-0x1.295e9e1b0899fp-4) * t_t0[44] + (-0x1.bde3fbbd7b203p-3) * t_t0[46];
+    ss[10] = -t_t0[10] + t_t0[26] + (-0x1.295e9e1b0899fp-4) * t_t0[37] + (-0x1.bde3fbbd7b203p-3) * t_t0[39] + (0x1.295e9e1b0899fp-4) * t_t0[45] + (0x1.bde3fbbd7b203p-3) * t_t0[47];
+    ss[11] = -t_t0[11] + t_t0[27] + (-0x1.295e9e1b0899fp-4) * t_t0[36] + (-0x1.bde3fbbd7b203p-3) * t_t0[38] + (0x1.295e9e1b0899fp-4) * t_t0[44] + (0x1.bde3fbbd7b203p-3) * t_t0[46];
+    ss[12] = -t_t0[12] + t_t0[28] + (0x1.295e9e1b0899fp-4) * t_t0[39] + (-0x1.295e9e1b0899fp-4) * t_t0[47];
+    ss[13] = -t_t0[13] + t_t0[29] + (0x1.295e9e1b0899fp-4) * t_t0[38] + (-0x1.295e9e1b0899fp-4) * t_t0[46];
+    ss[14] = -t_t0[14] + t_t0[30] + (-0x1.295e9e1b0899fp-4) * t_t0[39] + (0x1.295e9e1b0899fp-4) * t_t0[47];
+    ss[15] = -t_t0[15] + t_t0[31] + (-0x1.295e9e1b0899fp-4) * t_t0[38] + (0x1.295e9e1b0899fp-4) * t_t0[46];
+    /* t1: shift state[i] plus eta * (CSR state term + CSR t0 term), clip. */
+    for (i = 0; i < 16; ++i) {
+        fom_float s, v, s0 = 0.0, s1 = 0.0, s2 = 0.0, s3 = 0.0;
+        const fom_float *a = FOM_MAT_t1_state__d_eta_VALS + 16 * i;
+        const fom_float *xs = state + 144;
+        fom_int j;
+        /* t1_state__d_eta is a dense 16x16 block on state[144:160];
+           four independent partial sums break the FMA dependency chain. */
+        for (j = 0; j < 16; j += 4) {
+            s0 += a[j] * xs[j];
+            s1 += a[j + 1] * xs[j + 1];
+            s2 += a[j + 2] * xs[j + 2];
+            s3 += a[j + 3] * xs[j + 3];
+        }
+        s = (s0 + s1) + (s2 + s3) + ss[i];
+        v = bias_t1[i] + state[i] + sc_eta * s;
+        if (v < act_t1->lo[i]) v = act_t1->lo[i];
+        if (v > act_t1->hi[i]) v = act_t1->hi[i];
+        t_t1[i] = v;
+    }
+    /* su: every member is a 0/1 shift. */
+    for (i = 0; i < 16; ++i) new_state[i] = bias_su[i] + sc_g2 * state[i + 64] + sc_g1 * t_t1[i];
+    for (i = 16; i < 64; ++i) new_state[i] = bias_su[i] + sc_g2 * state[i + 64] + sc_g1 * t_t0[i - 16];
+    for (i = 64; i < 80; ++i) new_state[i] = bias_su[i] + sc_d1 * state[i] + sc_d2 * t_t1[i - 64];
+    for (i = 80; i < 128; ++i) new_state[i] = bias_su[i] + sc_d1 * state[i] + sc_d2 * t_t0[i - 80];
+    for (i = 128; i < 144; ++i) new_state[i] = bias_su[i] + state[i - 128];
+    for (i = 144; i < 160; ++i) new_state[i] = bias_su[i] + sc_m1 * state[i - 80] + sc_m2 * t_t1[i - 144];
+}
```

## Agent notes

```
### Round 1

**Whole-solve time:** initial `mean_us` = 125.5, final `mean_us` ≈ 29.0 to 30.7 across repeated `make bench` runs (min 25 µs). Overall speedup is about 4.2x. `make verify` prints `VERIFY OK` (worst end-to-end error 1.8e-12).

**Final profile breakdown** (from `make profile`, which has instrumentation overhead), hottest first:
- `iterate` (the fused `fom_iterate`: t0, t1 and the state update): 151.3 ms over 1,000,000 calls, 88.2%
- `bias` (per-iteration `b0 + tau*q0` and `b1 + eta*q1`): 20.2 ms over 1,000,000 calls, 11.8%

**Optimizations applied:**
1. **One fused `fom_iterate` pass replaces step_t0, step_t1, the zeroing of xn, and fused_su__rest plus the selection-row copies.** 125.5 → 50.3 µs. Structure exploited: every unscaled member is a 0/1 shift. `t0_state` is `x[i+16]` and `t1_state` is `x[i]`. All su members are shifts, so the whole state update becomes six contiguous scaled-copy loops, `new_state[i] = bias + s_a*x[...] + s_b*t[...]`, with no CSR traversal and no zero pass. The zero members (`t0_p_p1`, `t1_t_t0`, `su_t_t*`) are dropped.
2. **The two t0 CSR terms are merged into one.** 50.3 → 42.5 µs. Structure exploited: `t0_state__d_tth` has exactly the same row pointers as `t0_state__d_taux`, column indices shifted by +128, and exactly negated values (I checked this exactly in the data). So `taux*A x[0:16] + tth*B x[128:144]` equals `A (taux*x[0:16] - tth*x[128:144])`. That is one CSR pass over a 16-vector `w`, and the identity holds for any schedule values.
3. **Dense kernel for `t1_state__d_eta`.** 42.5 → 29.0 µs. Structure exploited: this 256-nonzero matrix is a dense 16×16 block on `state[144:160]` (row i, column 144+j). It now runs as a fixed-stride dense loop that the compiler can vectorize, with no index indirection.
4. **Earlier work kept:** the parameter products (`t0_p_p1 @ p1`, `t1_p_p0 @ p0` and their `__d_` variants) are still computed once per solve. The residual is still computed only on the last iteration.

**Tried and reverted:** a single-entry fast path for t0 rows 0..31 (each has one nonzero). The gain was within noise, so I kept the simpler code.

Blocked by rules:
- Fold `tau*q0` and `eta*q1` into `fom_iterate` to remove the separate `bias` pass (about 12% of profiled time) | the fixed `fom_iterate` signature takes only a precomputed bias, and `src/fom_ops.c` may not read the schedule | ~5–10%
- Unroll `t1_t_t0__d_eta` and `t0_state__d_taux` with inlined literal coefficients, exploiting their repeated ±0.0726/0.2177/... banded pattern (they look like transposes of each other, a PDHG K / Kᵀ pair) | allowed in `fom_iterate`, but not done for lack of time within the 10-minute budget, not because a rule forbids it | unknown, likely ~10–20%

### Round 2

**Whole solve (`make bench` mean_us):** 29.490 us at the start of this round, about 15.3 us now. Four runs gave 15.0 to 17.6 us; the machine is noisy. That is about 1.93x faster this round. Against the translated solver (127.135 us) it is about 8.3x overall.

**Final profile (`make profile`, -O3 with timers on, hottest first):**
- iterate: 72897 us, 77.5%
- bias: 21152 us, 22.5%

The bias share is mostly timer overhead. That loop is only 64 multiply-adds per iteration.

**Optimizations this round:**
- **Unrolled `t0_state__d_taux` with its values written in (inside `fom_iterate`).** It is a fixed 48x16 pattern with 112 nonzeros: rows 0..31 have one entry each, and rows 32..47 have 2 to 8. The ±1 coefficients became plain adds and subtracts. This removes the row-pointer and column-index loads and the short inner loops. The earlier `w = taux*x[0:16] - tth*x[128:144]` merge is kept. It is valid because `t0_state__d_tth` is the negated, column-shifted copy of `d_taux`. This change is the main win, roughly 29.5 to 15.5 us.
- **Unrolled `t1_t_t0__d_eta` the same way.** It is 16x48 with 112 nonzeros and is structurally `-(t0_state__d_taux[:, :16])^T`. It is now 16 fixed sums over `t_t0`, computed in `fom_iterate` right after `t0`.
- **Four independent partial sums for the dense 16x16 `t1_state__d_eta` block** (state[144:160]). This shortens the chain of dependent multiply-adds. The gain is small, within noise.
- **Tried and dropped:** a fully unrolled dense block with its values written in, and a column-stored dense block (16 axpy updates). Neither beat the 4-sum loop.

Blocked by rules:
- Add the `tau*q0` / `eta*q1` parameter terms to the bias inside `fom_iterate`, reading the hoisted terms and the schedule directly, which removes the separate bias pass | `fom_iterate` signature is fixed and `src/fom_ops.c` may not read the schedule | ~5%
- Compile with `-ffast-math` / `-march=native` so the dense dot-product reductions vectorize | Makefile and build flags are fixed | unknown, likely <5%

### Round 3

**Timing**
- Initial (translated) solver: 127.135 µs per solve. Start of this round: 15.639 µs.
- Final: about 14.6 µs (five runs gave 14.31, 14.37, 14.71, 14.82 and 14.93 µs).
- Overall speedup: about 8.7x over the translation. This round's gain is about 7%.

**Final profile (`make profile`), hottest first**
- iterate: 74254 µs, 77.4%
- bias: 21696 µs, 22.6%

The profile build adds timer overhead to every call, so its whole-solve figure (29.1 µs) is about twice the real one. The split between the two regions is therefore only rough.

**Optimizations**
- **This round: dropped the empty parameter products.** `t0_p_p1` and `t1_p_p0` have zero nonzeros (`FOM_MAT_*_NNZ 0`). Their products are always zero, whatever the parameters. The solver no longer calls them or adds them to the bias, so the bias is now just `tau*q0` and `eta*q1`, each computed once per solve and scaled per iteration. This removes 64 loads and 64 adds per iteration and one zeroing pass per solve.
- **Kept from earlier rounds: one-pass `fom_iterate`.**
  - In `t0`, the two scheduled terms are the same matrix with opposite signs on shifted columns. They collapse into one sparse pass over `w = taux*x[0:16] - tth*x[128:144]`, unrolled with the activation folded in.
  - `t1` uses an unrolled `t1_t_t0__d_eta` product plus a dense 16x16 block, accumulated with four partial sums.
  - The state update is all 0/1 shifts, so it is written as plain scaled copies.

**Nothing else to gain.** Each iteration now takes about 70 ns and is a single pass. The bias pass cannot be removed because `fom_iterate` takes the bias as an array, so the `tau`/`eta` scaling must happen outside it. I found no further changes worth making.

Blocked by rules:
- Fold the `tau*q0` and `eta*q1` bias scaling into `fom_iterate` and remove the separate bias pass | fixed `fom_iterate` signature in `include/fom_ops.h` (headers not editable) | ~10-20% (the bias region's share)
- Stop reading the all-zero state-update bias (`ZERO_SU`) inside `fom_iterate` | verify checks `fom_iterate` with random biases, so the routine must honour `bias_su` | ~1-3%

### Round 4

- Initial (round-4 start): 17.227 us per solve. Final: unchanged, about 16 to 17 us (three `make bench` runs: 16.00, 17.87, 16.24; run-to-run noise is about 15%). Speedup over the translated solver (127.135 us) stays at about 7.4x. This round gained nothing.
- Final profile (`make profile`), hottest first:
  - iterate: 74502 us, 77.6%
  - bias: 21564 us, 22.4% (mostly timer overhead around a 64-multiply loop)
- Tried and reverted this round:
  - Exploited the 4x4 block-circulant structure of the dense 16x16 block `t1_state__d_eta` (columns 144..159). Each 4x4 block is symmetric circulant `[a b c b]`, and the property holds exactly in the data. Adding the two `b` partners first saves one multiply in four (192 instead of 256). It passed `make verify` (error 1.8e-12) but was slower (17.5 to 19.2 us), because the per-row index arithmetic stopped the compiler from vectorizing the dense 4-way-split FMA loop.
- Kept from earlier rounds (unchanged):
  - One-pass `fom_iterate`. `t0_state__d_tth` is `-t0_state__d_taux` shifted by 128 columns, so the two scheduled products merge into one unrolled pass over `w = taux*x[0:16] - tth*x[128:144]`.
  - `t1_t_t0__d_eta` unrolled over its fixed sparsity with literal coefficients.
  - `t1_state__d_eta` as a dense block with 4 independent partial sums.
  - Every `su` member applied as a 0/1 shift (selection rows written as copies).
  - Empty parameter products (`t0_p_p1`, `t1_p_p0`) dropped, and the parameter terms hoisted out of the iteration loop.
- Profile markers `bias` and `iterate` are still in `src/fom_solver.c`.

Blocked by rules:
- Pass `q0`, `q1` and `tau`/`eta` into `fom_iterate` so the per-iteration `bias = tau*q` scaling folds into the product pass (removes the separate 64-element bias loop) | fixed `fom_iterate` signature in `include/fom_ops.h` (headers are fixed) | about 2-5%
- Precompute every scheduled bias `tau(it)*q0`, `eta(it)*q1` for all 200 iterations in `fom_update_params` | workspace layout in `include/fom_workspace.h` is fixed and has no scratch space for it | about 2-5%
```
