# Optimization report

## Timing

```
translated (emitted)     138.2 us/solve
optimized                 19.0 us/solve
speedup                  7.27x
```

Optimization outcome: **kept (re-verified correct)**.

## Rounds

| round | before (us) | after (us) | gain | kept | agent (s) | outcome |
|---:|---:|---:|---:|:---:|---:|---|
| 1 | 138.173 | 20.272 | 85.3% | yes | 119 | continued |
| 2 | 21.764 | 19.004 | 12.7% | yes | 59 | continued |
| 3 | - | - | - | no | 64 | the round left the sources unchanged |

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
| `fused_t1` | 16x208 | 384 | yes | basis | 418 | 0.000e+00 |
| `fused_su` | 160x224 | 304 | no | basis | 1570 | 0.000e+00 |
| `step_t0` | 48x160 | 272 | yes | basis | 486 | 8.882e-16 |
| `step_t1` | 16x208 | 384 | yes | basis | 422 | 1.066e-14 |
| `step_su` | 160x224 | 304 | no | basis | 1570 | 0.000e+00 |
| `fused_su__rest` | 160x224 | 304 | no | basis | 1570 | 0.000e+00 |
| `step_su__rest` | 160x224 | 304 | no | basis | 1570 | 0.000e+00 |
| `iterate` | 160x160 | 960 | yes | basis | 1766 | 1.137e-13 |

### Rule violations

_None: the collected rewrite passed the rules and was kept._

## Blocked by rules

Optimizations the agent considered but did not make because a rule forbade them, in its own words.

| round | idea | rule | expected gain |
|---|---|---|---|
| 1 | Precompute the four parameter products in `fom_update_params` instead of at the top of every `fom_solve` | `fom_setup`/`fom_update_params` must not call `fom_apply_*` | under 0.3% (the params region is 0.24%) |
| 1 | Pass the parameter products and schedule values straight into `fom_step_t0`/`fom_step_t1` so each iteration skips the separate 64-element bias loop | step routine signatures in `include/fom_ops.h` are fixed | about 2–4% |
| 2 | Specializing the state update to the actual schedule (`g1` is 1 and `g2` is 0 in the early iterations, which would drop terms) | `src/fom_ops.c` must not read the schedule, and the routines must be exact for any schedule value | ~1–3% |
| 3 | Run the whole 200-iteration loop inside `fom_iterate` so the schedule and all buffers stay in registers across iterations | nothing in `src/fom_ops.c` may read the schedule (`fom_sched_*`), and `fom_iterate`'s per-iteration signature is fixed in the headers | ~5-10% (call overhead only) |
| 3 | Build a transposed copy of the dense `t1_state__d_eta` block once in `fom_setup` | `fom_solver.c` must not read `FOM_MAT_*` data of matrices it multiplies by, so only a lazily built static table inside `fom_ops.c` remains; that variant was tried and was slower | none measured |

## What the agent changed

```diff
--- src/fom_solver.c (translated)
+++ src/fom_solver.c (optimized)
@@ -2,6 +2,7 @@
 #include "fom_data.h"
 #include "fom_ops.h"
 #include "fom_linsolve.h"
+#include "fom_profile.h"
 
 static void reset_state(FomWorkspace *work) {
     fom_int i;
@@ -32,26 +33,43 @@
 
 fom_int fom_solve(FomWorkspace *work) {
     fom_float *x = work->state, *xn = work->new_state, *tmp;
-    fom_float *bias_t0 = work->acc, *bias_t1 = work->acc + FOM_T0_DIM;
+    fom_float *t0 = work->t0, *t1 = work->t1;
+    fom_float p1a[FOM_T0_DIM], p1b[FOM_T0_DIM], p0a[FOM_T1_DIM], p0b[FOM_T1_DIM];
+    fom_float bias_t0[FOM_T0_DIM], bias_t1[FOM_T1_DIM];
     fom_int it, i, n = work->settings.max_iter;
     fom_float res = 0;
 
+    /* Parameter products are iteration-invariant: apply each once per solve. */
+    FOM_PROF_BEGIN(params);
+    for (i = 0; i < FOM_T0_DIM; i++) { p1a[i] = 0; p1b[i] = 0; }
+    for (i = 0; i < FOM_T1_DIM; i++) { p0a[i] = 0; p0b[i] = 0; }
+    fom_apply_t0_p_p1(work->params.p1, p1a);
+    fom_apply_t0_p_p1__d_tau(work->params.p1, p1b);
+    fom_apply_t1_p_p0(work->params.p0, p0a);
+    fom_apply_t1_p_p0__d_eta(work->params.p0, p0b);
+    FOM_PROF_END(params);
+
     for (it = 0; it < n; it++) {
         const fom_float tau = fom_sched_tau(it), eta = fom_sched_eta(it);
-        /* bias_t0 = t0_p_p1 @ p1 + tau * (t0_p_p1__d_tau @ p1); same for t1 with eta. */
-        for (i = 0; i < FOM_T0_DIM + FOM_T1_DIM; i++) work->acc[i] = 0;
-        fom_apply_t0_p_p1__d_tau(work->params.p1, bias_t0);
-        fom_apply_t1_p_p0__d_eta(work->params.p0, bias_t1);
-        for (i = 0; i < FOM_T0_DIM; i++) bias_t0[i] *= tau;
-        for (i = 0; i < FOM_T1_DIM; i++) bias_t1[i] *= eta;
-        fom_apply_t0_p_p1(work->params.p1, bias_t0);
-        fom_apply_t1_p_p0(work->params.p0, bias_t1);
-        fom_step_t0(x, fom_sched_taux(it), fom_sched_tth(it), bias_t0, &FOM_ACT_t0, work->t0);
-        fom_step_t1(x, work->t0, eta, bias_t1, &FOM_ACT_t1, work->t1);
-        for (i = 0; i < FOM_STATE_DIM; i++) xn[i] = 0;
-        fom_fused_su(x, work->t0, work->t1, fom_sched_d1(it), fom_sched_g2(it),
-                     fom_sched_m1(it), fom_sched_d2(it), fom_sched_g1(it),
-                     fom_sched_m2(it), xn);
+        const fom_float d1 = fom_sched_d1(it), g2 = fom_sched_g2(it), m1 = fom_sched_m1(it);
+        const fom_float d2 = fom_sched_d2(it), g1 = fom_sched_g1(it), m2 = fom_sched_m2(it);
+        FOM_PROF_BEGIN(t0);
+        for (i = 0; i < FOM_T0_DIM; i++) bias_t0[i] = p1a[i] + tau * p1b[i];
+        for (i = 0; i < FOM_T1_DIM; i++) bias_t1[i] = p0a[i] + eta * p0b[i];
+        fom_step_t0(x, fom_sched_taux(it), fom_sched_tth(it), bias_t0, &FOM_ACT_t0, t0);
+        FOM_PROF_END(t0);
+        FOM_PROF_BEGIN(t1);
+        fom_step_t1(x, t0, eta, bias_t1, &FOM_ACT_t1, t1);
+        FOM_PROF_END(t1);
+        /* State update: every su product is a 0/1 selection (see SPEC.md). */
+        FOM_PROF_BEGIN(su);
+        for (i = 0; i < 16; i++) xn[i] = g2 * x[i + 64] + g1 * t1[i];
+        for (i = 16; i < 64; i++) xn[i] = g2 * x[i + 64] + g1 * t0[i - 16];
+        for (i = 64; i < 80; i++) xn[i] = d1 * x[i] + d2 * t1[i - 64];
+        for (i = 80; i < 128; i++) xn[i] = d1 * x[i] + d2 * t0[i - 80];
+        for (i = 128; i < 144; i++) xn[i] = x[i - 128];
+        for (i = 144; i < 160; i++) xn[i] = m1 * x[i - 80] + m2 * t1[i - 144];
+        FOM_PROF_END(su);
         tmp = x; x = xn; xn = tmp;
     }
     if (n > 0) {
--- src/fom_ops.c (translated)
+++ src/fom_ops.c (optimized)
@@ -223,7 +223,10 @@
         for (p = FOM_MAT_t1_state_ROWPTR[i]; p < FOM_MAT_t1_state_ROWPTR[i + 1]; ++p) s += FOM_MAT_t1_state_VALS[p] * state[FOM_MAT_t1_state_COLIDX[p]];
         out[i] += s;
         s = 0.0;
-        for (p = FOM_MAT_t1_state__d_eta_ROWPTR[i]; p < FOM_MAT_t1_state__d_eta_ROWPTR[i + 1]; ++p) s += FOM_MAT_t1_state__d_eta_VALS[p] * state[FOM_MAT_t1_state__d_eta_COLIDX[p]];
+        /* t1_state__d_eta is a dense 16x16 block on state columns 144..159. */
+        const fom_float *w = FOM_MAT_t1_state__d_eta_VALS + 16 * i;
+        fom_int j;
+        for (j = 0; j < 16; ++j) s += w[j] * state[144 + j];
         out[i] += sc_eta * s;
         s = 0.0;
         for (p = FOM_MAT_t1_t_t0__d_eta_ROWPTR[i]; p < FOM_MAT_t1_t_t0__d_eta_ROWPTR[i + 1]; ++p) s += FOM_MAT_t1_t_t0__d_eta_VALS[p] * t_t0[FOM_MAT_t1_t_t0__d_eta_COLIDX[p]];
@@ -266,40 +269,70 @@
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
+    /* t0_state is the shift out[i] += state[i + 16]; the two scheduled
+       members share one product on y (below). */
+    fom_int i;
+    /* __d_tth equals -__d_taux shifted to columns 128..143 (checked per
+       member by verify), so both collapse to one product on
+       y = sc_taux * state[0..15] - sc_tth * state[128..143]. */
+    fom_float y[16];
+    for (i = 0; i < 16; ++i) y[i] = sc_taux * state[i] - sc_tth * state[i + 128];
+    /* Rows 0..31 of both scheduled members hold one entry each, in column
+       i mod 16 (__d_taux) and 128 + i mod 16 (__d_tth), stored at p = i. */
+    for (i = 0; i < 32; ++i) {
+        const fom_int c = i & 15;
+        fom_float v = bias[i] + state[i + 16] + FOM_MAT_t0_state__d_taux_VALS[i] * y[c];
         if (v < act->lo[i]) v = act->lo[i];
         out[i] = v;
     }
+    const fom_float *TA = FOM_MAT_t0_state__d_taux_VALS;
+    /* Rows 32..47: fixed sparsity of __d_taux unrolled. */
+    { fom_float v = bias[32] + state[48] + (TA[32] * y[1] + TA[33] * y[3]); if (v < act->lo[32]) v = act->lo[32]; out[32] = v; }
+    { fom_float v = bias[33] + state[49] + (TA[34] * y[0] + TA[35] * y[2]); if (v < act->lo[33]) v = act->lo[33]; out[33] = v; }
+    { fom_float v = bias[34] + state[50] + (TA[36] * y[1] + TA[37] * y[3] + TA[38] * y[5] + TA[39] * y[7]); if (v < act->lo[34]) v = act->lo[34]; out[34] = v; }
+    { fom_float v = bias[35] + state[51] + (TA[40] * y[0] + TA[41] * y[2] + TA[42] * y[4] + TA[43] * y[6]); if (v < act->lo[35]) v = act->lo[35]; out[35] = v; }
+    { fom_float v = bias[36] + state[52] + (TA[44] * y[1] + TA[45] * y[3] + TA[46] * y[5] + TA[47] * y[7] + TA[48] * y[9] + TA[49] * y[11]); if (v < act->lo[36]) v = act->lo[36]; out[36] = v; }
+    { fom_float v = bias[37] + state[53] + (TA[50] * y[0] + TA[51] * y[2] + TA[52] * y[4] + TA[53] * y[6] + TA[54] * y[8] + TA[55] * y[10]); if (v < act->lo[37]) v = act->lo[37]; out[37] = v; }
+    { fom_float v = bias[38] + state[54] + (TA[56] * y[1] + TA[57] * y[3] + TA[58] * y[5] + TA[59] * y[7] + TA[60] * y[9] + TA[61] * y[11] + TA[62] * y[13] + TA[63] * y[15]); if (v < act->lo[38]) v = act->lo[38]; out[38] = v; }
+    { fom_float v = bias[39] + state[55] + (TA[64] * y[0] + TA[65] * y[2] + TA[66] * y[4] + TA[67] * y[6] + TA[68] * y[8] + TA[69] * y[10] + TA[70] * y[12] + TA[71] * y[14]); if (v < act->lo[39]) v = act->lo[39]; out[39] = v; }
+    { fom_float v = bias[40] + state[56] + (TA[72] * y[1] + TA[73] * y[3]); if (v < act->lo[40]) v = act->lo[40]; out[40] = v; }
+    { fom_float v = bias[41] + state[57] + (TA[74] * y[0] + TA[75] * y[2]); if (v < act->lo[41]) v = act->lo[41]; out[41] = v; }
+    { fom_float v = bias[42] + state[58] + (TA[76] * y[1] + TA[77] * y[3] + TA[78] * y[5] + TA[79] * y[7]); if (v < act->lo[42]) v = act->lo[42]; out[42] = v; }
+    { fom_float v = bias[43] + state[59] + (TA[80] * y[0] + TA[81] * y[2] + TA[82] * y[4] + TA[83] * y[6]); if (v < act->lo[43]) v = act->lo[43]; out[43] = v; }
+    { fom_float v = bias[44] + state[60] + (TA[84] * y[1] + TA[85] * y[3] + TA[86] * y[5] + TA[87] * y[7] + TA[88] * y[9] + TA[89] * y[11]); if (v < act->lo[44]) v = act->lo[44]; out[44] = v; }
+    { fom_float v = bias[45] + state[61] + (TA[90] * y[0] + TA[91] * y[2] + TA[92] * y[4] + TA[93] * y[6] + TA[94] * y[8] + TA[95] * y[10]); if (v < act->lo[45]) v = act->lo[45]; out[45] = v; }
+    { fom_float v = bias[46] + state[62] + (TA[96] * y[1] + TA[97] * y[3] + TA[98] * y[5] + TA[99] * y[7] + TA[100] * y[9] + TA[101] * y[11] + TA[102] * y[13] + TA[103] * y[15]); if (v < act->lo[46]) v = act->lo[46]; out[46] = v; }
+    { fom_float v = bias[47] + state[63] + (TA[104] * y[0] + TA[105] * y[2] + TA[106] * y[4] + TA[107] * y[6] + TA[108] * y[8] + TA[109] * y[10] + TA[110] * y[12] + TA[111] * y[14]); if (v < act->lo[47]) v = act->lo[47]; out[47] = v; }
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
+    /* t1_state is the shift out[i] += state[i]; both eta members share sc_eta.
+       t1_state__d_eta is a dense 16x16 block on state columns 144..159. */
+    const fom_float *TV = FOM_MAT_t1_t_t0__d_eta_VALS;
+    fom_int i, j;
+    fom_float u[16];
+    /* t1_t_t0__d_eta: fixed sparsity unrolled. */
+    u[0] = TV[0] * t_t0[0] + TV[1] * t_t0[16] + TV[2] * t_t0[33] + TV[3] * t_t0[35] + TV[4] * t_t0[37] + TV[5] * t_t0[39] + TV[6] * t_t0[41] + TV[7] * t_t0[43] + TV[8] * t_t0[45] + TV[9] * t_t0[47];
+    u[1] = TV[10] * t_t0[1] + TV[11] * t_t0[17] + TV[12] * t_t0[32] + TV[13] * t_t0[34] + TV[14] * t_t0[36] + TV[15] * t_t0[38] + TV[16] * t_t0[40] + TV[17] * t_t0[42] + TV[18] * t_t0[44] + TV[19] * t_t0[46];
+    u[2] = TV[20] * t_t0[2] + TV[21] * t_t0[18] + TV[22] * t_t0[33] + TV[23] * t_t0[35] + TV[24] * t_t0[37] + TV[25] * t_t0[39] + TV[26] * t_t0[41] + TV[27] * t_t0[43] + TV[28] * t_t0[45] + TV[29] * t_t0[47];
+    u[3] = TV[30] * t_t0[3] + TV[31] * t_t0[19] + TV[32] * t_t0[32] + TV[33] * t_t0[34] + TV[34] * t_t0[36] + TV[35] * t_t0[38] + TV[36] * t_t0[40] + TV[37] * t_t0[42] + TV[38] * t_t0[44] + TV[39] * t_t0[46];
+    u[4] = TV[40] * t_t0[4] + TV[41] * t_t0[20] + TV[42] * t_t0[35] + TV[43] * t_t0[37] + TV[44] * t_t0[39] + TV[45] * t_t0[43] + TV[46] * t_t0[45] + TV[47] * t_t0[47];
+    u[5] = TV[48] * t_t0[5] + TV[49] * t_t0[21] + TV[50] * t_t0[34] + TV[51] * t_t0[36] + TV[52] * t_t0[38] + TV[53] * t_t0[42] + TV[54] * t_t0[44] + TV[55] * t_t0[46];
+    u[6] = TV[56] * t_t0[6] + TV[57] * t_t0[22] + TV[58] * t_t0[35] + TV[59] * t_t0[37] + TV[60] * t_t0[39] + TV[61] * t_t0[43] + TV[62] * t_t0[45] + TV[63] * t_t0[47];
+    u[7] = TV[64] * t_t0[7] + TV[65] * t_t0[23] + TV[66] * t_t0[34] + TV[67] * t_t0[36] + TV[68] * t_t0[38] + TV[69] * t_t0[42] + TV[70] * t_t0[44] + TV[71] * t_t0[46];
+    u[8] = TV[72] * t_t0[8] + TV[73] * t_t0[24] + TV[74] * t_t0[37] + TV[75] * t_t0[39] + TV[76] * t_t0[45] + TV[77] * t_t0[47];
+    u[9] = TV[78] * t_t0[9] + TV[79] * t_t0[25] + TV[80] * t_t0[36] + TV[81] * t_t0[38] + TV[82] * t_t0[44] + TV[83] * t_t0[46];
+    u[10] = TV[84] * t_t0[10] + TV[85] * t_t0[26] + TV[86] * t_t0[37] + TV[87] * t_t0[39] + TV[88] * t_t0[45] + TV[89] * t_t0[47];
+    u[11] = TV[90] * t_t0[11] + TV[91] * t_t0[27] + TV[92] * t_t0[36] + TV[93] * t_t0[38] + TV[94] * t_t0[44] + TV[95] * t_t0[46];
+    u[12] = TV[96] * t_t0[12] + TV[97] * t_t0[28] + TV[98] * t_t0[39] + TV[99] * t_t0[47];
+    u[13] = TV[100] * t_t0[13] + TV[101] * t_t0[29] + TV[102] * t_t0[38] + TV[103] * t_t0[46];
+    u[14] = TV[104] * t_t0[14] + TV[105] * t_t0[30] + TV[106] * t_t0[39] + TV[107] * t_t0[47];
+    u[15] = TV[108] * t_t0[15] + TV[109] * t_t0[31] + TV[110] * t_t0[38] + TV[111] * t_t0[46];
+    for (i = 0; i < 16; ++i) {
+        const fom_float *w = FOM_MAT_t1_state__d_eta_VALS + 16 * i;
+        fom_float s = u[i], v;
+        for (j = 0; j < 16; ++j) s += w[j] * state[144 + j];
+        v = bias[i] + state[i] + sc_eta * s;
         if (v < act->lo[i]) v = act->lo[i];
         if (v > act->hi[i]) v = act->hi[i];
         out[i] = v;
```

## Agent notes

```
### Round 1

**Whole-solve mean_us:** 136.89 at the start, about 21.1 at the end (two bench runs: 21.23 and 20.99). Overall speedup is about 6.5x.

**Final profile breakdown** (profile build; the timer calls add overhead, giving mean_us=33.78):
| region | total_us | pct |
|---|---|---|
| t0 (bias + `fom_step_t0`) | 39213 | 38.5% |
| su (state update as copies) | 38347 | 37.7% |
| t1 (`fom_step_t1`) | 23935 | 23.5% |
| params (once per solve) | 242 | 0.2% |

**Optimizations applied:**
1. **Parameter terms applied once per solve (137 → 57 µs, together with item 2).** The four parameter products (`t0_p_p1`, `t0_p_p1__d_tau`, `t1_p_p0`, `t1_p_p0__d_eta`) are computed once at the top of `fom_solve`. Each iteration then forms the bias as `A p + tau * B p`. This is valid because the parameters do not change during a solve.
2. **State update written as copies in `fom_solver.c`.** Every `su` member is a 0/1 selection (shift), and `su` has no constant term. So the new state is six contiguous blocks of the form `sched * x[shift] + sched * t[shift]`, plus the selection rows 128..143, which copy `x[0..15]`. This removes the zeroing pass and the 160-row × 9-member CSR walk.
3. **Unit shift members inlined in the step routines (→ 51.5 µs).** `t0_state` is the shift `state[i+16]` and `t1_state` is the shift `state[i]`, so they are added directly instead of walked as CSR. In t1, the two members scaled by `eta` are summed first and multiplied by `eta` once.
4. **Shared sparsity pattern in t0 (→ 34.8 µs).** `t0_state__d_tth` has exactly the sparsity of `t0_state__d_taux` with columns shifted by 128. One loop over the index list therefore drives both products.
5. **Dense block in t1.** `t1_state__d_eta` is a dense 16×16 block on state columns 144..159. It is now a fixed-length loop with no index array.
6. **Fixed sparsity unrolled (→ about 21 µs).** Rows 0..31 of the t0 members hold one entry each, in column `i mod 16`, so they are written in closed form. Rows 32..47 of t0 and all of `t1_t_t0__d_eta` are fully unrolled with their column indices as literals. Coefficient values are still read from `fom_data.c`.

Blocked by rules:
- Precompute the four parameter products in `fom_update_params` instead of at the top of every `fom_solve` | `fom_setup`/`fom_update_params` must not call `fom_apply_*` | under 0.3% (the params region is 0.24%)
- Pass the parameter products and schedule values straight into `fom_step_t0`/`fom_step_t1` so each iteration skips the separate 64-element bias loop | step routine signatures in `include/fom_ops.h` are fixed | about 2–4%

### Round 2

**Whole-solve `mean_us`:** 20.272 at the start of this round (138.173 for the translated solver). Final is about 18.6–19.8 over three `make bench` runs, call it ~19.1. That is about 1.06x this round and about 7.2x overall against the translation. Bench noise on this machine is ±10%: the unchanged baseline measured 24.1 and 21.2 on two reruns.

**Final profile (`make profile`, hottest first):**
| region | time | % |
|---|---|---|
| su | 37615 us | 41.5% |
| t0 | 29034 us | 32.0% |
| t1 | 23766 us | 26.2% |
| params | 267 us | 0.3% |

`t0` fell from 40754 us to 29034 us. Profiler overhead is about 30 ns per region call, and that dominates these tiny regions.

**Optimizations this round:**
- **Merged the two scheduled members of t0 into one product.** The structure: `t0_state__d_tth` has exactly the same sparsity and values as `t0_state__d_taux`, negated and shifted from state columns 0..15 to 128..143. So `fom_step_t0` forms `y = sc_taux*state[0..15] - sc_tth*state[128..143]` (16 FMAs) and applies the `__d_taux` sparsity once to `y`, with rows 0..31 one entry each and rows 32..47 unrolled. This halves the sparse multiply-adds in t0 (224 down to 112, plus 16 to form `y`). It is exact for any schedule values, and verify's per-member unit-vector probes confirm the relation.
- **Considered and skipped:** the dense symmetric 16x16 `t1_state__d_eta` block. Symmetry gives no flop saving for a matrix-vector product, and the block is full rank (rank 16), so a low-rank split is not possible.

Blocked by rules:
- Specializing the state update to the actual schedule (`g1` is 1 and `g2` is 0 in the early iterations, which would drop terms) | `src/fom_ops.c` must not read the schedule, and the routines must be exact for any schedule value | ~1–3%

### Round 3

**Whole-solve `mean_us`:** 138.173 µs for the translated solver vs. 19.004 µs now, a 7.27x speedup. Round 3 adds nothing: every change was reverted and the sources are the round-2 solver. Repeated `make bench` runs of that solver measure 17.0 to 19.5 µs, about ±10% noise.

**Final profile (`make profile`), hottest first:**
| region | time (µs) | % |
|---|---|---|
| su | 36734 | 41.35 |
| t0 | 28483 | 32.06 |
| t1 | 23410 | 26.35 |
| params | 215 | 0.24 |

The profiled run measures 30.9 µs per solve against about 18 µs unprofiled. Timer overhead, about 13 µs per solve out of 30.9, is therefore a large share of each region's time.

**Optimizations in the current solver, all from earlier rounds:**
- **Parameter products hoisted.** `t0_p_p1`, `t1_p_p0` and their `__d_` members are applied once per solve. Each iteration only forms `bias = a + s*b`. This is valid because the parameters are constant over a solve.
- **t0 step collapsed.** `t0_state__d_tth` is minus `__d_taux` shifted to columns 128..143. Both therefore reduce to one product on `y = taux*x[0..15] - tth*x[128..143]`. Rows 0..31 have a single entry each, and rows 32..47 are unrolled over their fixed sparsity. The `t0_state` shift and the relu are fused into the same pass.
- **t1 step fused.** The fixed sparsity of `t1_t_t0__d_eta` is unrolled. It is fused with the dense 16x16 block of `t1_state__d_eta` on columns 144..159 under a single `eta`, plus the identity shift and the clip.
- **State update as copies.** Every su member is a 0/1 shift, so the update is written directly in `fom_solver.c` as scaled copies. Rows 128..143 are a plain copy, and the zero members are dropped.
- **Pointer swap.** `x` and `xn` are swapped each iteration instead of copied, and the residual is computed only after the last iteration.

**Tried this round and reverted, no gain:**
- `restrict`-qualified pointers on the su copy loops: 17.3 to 19.3 µs, against 17.2 to 17.6 µs for the baseline.
- A column-oriented (transposed) layout for the dense 16x16 t1 block: 17.8 to 22.1 µs, slower.

Blocked by rules:
- Run the whole 200-iteration loop inside `fom_iterate` so the schedule and all buffers stay in registers across iterations | nothing in `src/fom_ops.c` may read the schedule (`fom_sched_*`), and `fom_iterate`'s per-iteration signature is fixed in the headers | ~5-10% (call overhead only)
- Build a transposed copy of the dense `t1_state__d_eta` block once in `fom_setup` | `fom_solver.c` must not read `FOM_MAT_*` data of matrices it multiplies by, so only a lazily built static table inside `fom_ops.c` remains; that variant was tried and was slower | none measured
```
