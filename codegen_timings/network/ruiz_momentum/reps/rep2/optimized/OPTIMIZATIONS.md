# Optimization report

## Timing

```
translated (emitted)      45.7 us/solve
optimized                 18.4 us/solve
speedup                  2.49x
```

Optimization outcome: **kept (re-verified correct)**.

## Rounds

| round | before (us) | after (us) | gain | kept | agent (s) | outcome |
|---:|---:|---:|---:|:---:|---:|---|
| 1 | 45.685 | 18.380 | 59.8% | yes | 77 | continued |
| 2 | - | - | - | no | 59 | the round left the sources unchanged |

## Kernel checks

Each `fom_apply_<name>` in the shipped `fom_ops.c`, checked by `make verify` against the package's reference over the same `fom_data.c` matrix. *basis* probes every unit vector, which recovers the routine's matrix column by column (an exact check), plus random vectors; *random* uses random vectors only (the matrix is over the probe budget). A `fused_<block>` row is a `fom_fused_<block>`, checked against the sum of its members: *basis* there probes every unit vector of each source with every schedule value at 0 and with each alone at 1, which recovers every member exactly.

| matrix | shape | nnz | rewritten | check | probes | max error |
|---|---|---|---|---|---|---|
| `t0_state` | 49x74 | 147 | yes | basis | 76 | 0.000e+00 |
| `t1_state` | 10x74 | 59 | yes | basis | 76 | 0.000e+00 |
| `t1_state__d_xi` | 10x74 | 49 | yes | basis | 76 | 0.000e+00 |
| `t1_t_t0` | 10x49 | 49 | yes | basis | 51 | 0.000e+00 |
| `t1_t_t0__d_xi` | 10x49 | 49 | yes | basis | 51 | 0.000e+00 |
| `su_state` | 74x74 | 64 | yes | basis | 76 | 0.000e+00 |
| `su_state__d_xi` | 74x74 | 49 | yes | basis | 76 | 0.000e+00 |
| `su_t_t0` | 74x49 | 98 | yes | basis | 51 | 0.000e+00 |
| `su_t_t0__d_xi` | 74x49 | 49 | yes | basis | 51 | 0.000e+00 |
| `su_t_t1` | 74x10 | 10 | yes | basis | 12 | 0.000e+00 |
| `su_p_p0` | 74x15 | 15 | yes | basis | 17 | 0.000e+00 |
| `fused_t1` | 10x123 | 206 | yes | basis | 248 | 0.000e+00 |
| `fused_su` | 74x133 | 270 | yes | basis | 268 | 0.000e+00 |
| `step_t0` | 49x74 | 147 | yes | basis | 80 | 0.000e+00 |
| `step_t1` | 10x123 | 206 | yes | basis | 252 | 8.882e-16 |
| `step_su` | 74x133 | 270 | yes | basis | 268 | 1.776e-15 |
| `fused_su__rest` | 74x133 | 270 | yes | basis | 268 | 0.000e+00 |
| `step_su__rest` | 74x133 | 270 | yes | basis | 268 | 1.776e-15 |
| `iterate` | 74x74 | 623 | yes | basis | 228 | 1.776e-15 |

### Rule violations

_None: the collected rewrite passed the rules and was kept._

## Blocked by rules

Optimizations the agent considered but did not make because a rule forbade them, in its own words.

| round | idea | rule | expected gain |
|---|---|---|---|
| 1 | Inlining the per-iteration products directly into the solver loop of `src/fom_solver.c`, removing the call boundaries | src/fom_solver.c must not read matrix data or write matrix-vector loops/literals | small (LTO already inlines most of it), ~2-5% |
| 1 | Factoring the near-proportional `state`/`t_t0`/`_d_xi` coefficient pairs into one product with a derived scalar | exactness of the coefficients each member applies (literal values must be exactly the matrix's), and no new float constants | ~30-40% of the step time |
| 2 | Exploit the near-relations between member coefficients (xi-matrices ~= 2x the base ones, t_t0 coefficient ~= -2x the state coefficient). Each row of t1 and su would then collapse to c_j*(2*t0_j - s_j) + xi*2c_j*(t0_j - s_j), cutting the work about in half | the relations hold only to the last few bits, so the result would not apply exactly the package's coefficients ("values must be exactly the matrix's") | ~30-45% |

## What the agent changed

```diff
--- src/fom_solver.c (translated)
+++ src/fom_solver.c (optimized)
@@ -1,6 +1,7 @@
 #include "fom_solver.h"
 #include "fom_data.h"
 #include "fom_ops.h"
+#include "fom_profile.h"
 
 /* The state update copies t0 into rows 0..48 and t1 into rows 49..58
    (FOM_SELROWS_su), so both steps write straight into the next state and
@@ -47,9 +48,15 @@
     for (it = 0; it < max_iter; ++it) {
         const fom_float xi = fom_sched_xi(it);
         fom_float *tmp;
+        FOM_PROF_BEGIN(step_t0);
         fom_step_t0(cur, FOM_MAT_t0_c, &FOM_ACT_t0, nxt);
+        FOM_PROF_END(step_t0);
+        FOM_PROF_BEGIN(step_t1);
         fom_step_t1(cur, nxt, xi, FOM_MAT_t1_c, &FOM_ACT_t1, nxt + FOM_T0_DIM);
+        FOM_PROF_END(step_t1);
+        FOM_PROF_BEGIN(step_su_rest);
         fom_step_su__rest(cur, nxt, nxt + FOM_T0_DIM, xi, bias, nxt);
+        FOM_PROF_END(step_su_rest);
         tmp = cur; cur = nxt; nxt = tmp;
     }
     if (max_iter > 0) {
--- src/fom_ops.c (translated)
+++ src/fom_ops.c (optimized)
@@ -138,69 +138,6 @@
         s = 0.0;
         for (p = FOM_MAT_su_t_t1_ROWPTR[i]; p < FOM_MAT_su_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t1_VALS[p] * t_t1[FOM_MAT_su_t_t1_COLIDX[p]];
         out[i] += s;
-    }
-}
-
-void fom_step_t0(const fom_float *state, const fom_float *bias, const FomAct_t0 *act, fom_float *out) {
-    fom_int i, r, p;
-    for (r = 0; r < 49; ++r) {
-        fom_float s, v;
-        i = r;
-        v = bias[i];
-        s = 0.0;
-        for (p = FOM_MAT_t0_state_ROWPTR[i]; p < FOM_MAT_t0_state_ROWPTR[i + 1]; ++p) s += FOM_MAT_t0_state_VALS[p] * state[FOM_MAT_t0_state_COLIDX[p]];
-        v += s;
-        if (v < act->lo[i]) v = act->lo[i];
-        if (v > act->hi[i]) v = act->hi[i];
-        out[i] = v;
-    }
-}
-
-void fom_step_t1(const fom_float *state, const fom_float *t_t0, fom_float sc_xi, const fom_float *bias, const FomAct_t1 *act, fom_float *out) {
-    fom_int i, r, p;
-    for (r = 0; r < 10; ++r) {
-        fom_float s, v;
-        i = r;
-        v = bias[i];
-        s = 0.0;
-        for (p = FOM_MAT_t1_state_ROWPTR[i]; p < FOM_MAT_t1_state_ROWPTR[i + 1]; ++p) s += FOM_MAT_t1_state_VALS[p] * state[FOM_MAT_t1_state_COLIDX[p]];
-        v += s;
-        s = 0.0;
-        for (p = FOM_MAT_t1_state__d_xi_ROWPTR[i]; p < FOM_MAT_t1_state__d_xi_ROWPTR[i + 1]; ++p) s += FOM_MAT_t1_state__d_xi_VALS[p] * state[FOM_MAT_t1_state__d_xi_COLIDX[p]];
-        v += sc_xi * s;
-        s = 0.0;
-        for (p = FOM_MAT_t1_t_t0_ROWPTR[i]; p < FOM_MAT_t1_t_t0_ROWPTR[i + 1]; ++p) s += FOM_MAT_t1_t_t0_VALS[p] * t_t0[FOM_MAT_t1_t_t0_COLIDX[p]];
-        v += s;
-        s = 0.0;
-        for (p = FOM_MAT_t1_t_t0__d_xi_ROWPTR[i]; p < FOM_MAT_t1_t_t0__d_xi_ROWPTR[i + 1]; ++p) s += FOM_MAT_t1_t_t0__d_xi_VALS[p] * t_t0[FOM_MAT_t1_t_t0__d_xi_COLIDX[p]];
-        v += sc_xi * s;
-        if (v < act->lo[i]) v = act->lo[i];
-        out[i] = v;
-    }
-}
-
-void fom_step_su(const fom_float *state, const fom_float *t_t0, const fom_float *t_t1, fom_float sc_xi, const fom_float *bias, fom_float *out) {
-    fom_int i, r, p;
-    for (r = 0; r < 74; ++r) {
-        fom_float s, v;
-        i = r;
-        v = bias[i];
-        s = 0.0;
-        for (p = FOM_MAT_su_state_ROWPTR[i]; p < FOM_MAT_su_state_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_state_VALS[p] * state[FOM_MAT_su_state_COLIDX[p]];
-        v += s;
-        s = 0.0;
-        for (p = FOM_MAT_su_state__d_xi_ROWPTR[i]; p < FOM_MAT_su_state__d_xi_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_state__d_xi_VALS[p] * state[FOM_MAT_su_state__d_xi_COLIDX[p]];
-        v += sc_xi * s;
-        s = 0.0;
-        for (p = FOM_MAT_su_t_t0_ROWPTR[i]; p < FOM_MAT_su_t_t0_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t0_VALS[p] * t_t0[FOM_MAT_su_t_t0_COLIDX[p]];
-        v += s;
-        s = 0.0;
-        for (p = FOM_MAT_su_t_t0__d_xi_ROWPTR[i]; p < FOM_MAT_su_t_t0__d_xi_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t0__d_xi_VALS[p] * t_t0[FOM_MAT_su_t_t0__d_xi_COLIDX[p]];
-        v += sc_xi * s;
-        s = 0.0;
-        for (p = FOM_MAT_su_t_t1_ROWPTR[i]; p < FOM_MAT_su_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t1_VALS[p] * t_t1[FOM_MAT_su_t_t1_COLIDX[p]];
-        v += s;
-        out[i] = v;
     }
 }
 
@@ -227,29 +164,227 @@
     }
 }
 
+void fom_step_t0(const fom_float *state, const fom_float *bias, const FomAct_t0 *act, fom_float *out) {
+    const fom_float *bias_t0 = bias; const fom_float *lo = act->lo, *hi = act->hi;
+    { fom_float v = bias_t0[0] + (state[0] - 0.3148342557928728 * state[49] - 0.1255906893098709 * state[59]);
+      if (v < lo[0]) v = lo[0]; if (v > hi[0]) v = hi[0]; out[0] = v; }
+    { fom_float v = bias_t0[1] + (state[1] - 0.1785106319392791 * state[49] - 0.31483425579287283 * state[60]);
+      if (v < lo[1]) v = lo[1]; if (v > hi[1]) v = hi[1]; out[1] = v; }
+    { fom_float v = bias_t0[2] + (state[2] - 0.3148342556510004 * state[49] - 0.18736322506593048 * state[62]);
+      if (v < lo[2]) v = lo[2]; if (v > hi[2]) v = hi[2]; out[2] = v; }
+    { fom_float v = bias_t0[3] + (state[3] - 0.3148342557928728 * state[49] - 0.05210541922399308 * state[65]);
+      if (v < lo[3]) v = lo[3]; if (v > hi[3]) v = hi[3]; out[3] = v; }
+    { fom_float v = bias_t0[4] + (state[4] - 0.3148342557928728 * state[49] - 0.04219712274031408 * state[71]);
+      if (v < lo[4]) v = lo[4]; if (v > hi[4]) v = hi[4]; out[4] = v; }
+    { fom_float v = bias_t0[5] + (state[5] - 0.3148342557928728 * state[50] - 0.08155612991849112 * state[60]);
+      if (v < lo[5]) v = lo[5]; if (v > hi[5]) v = hi[5]; out[5] = v; }
+    { fom_float v = bias_t0[6] + (state[6] - 0.1149099042416003 * state[50] - 0.31483425579287283 * state[61]);
+      if (v < lo[6]) v = lo[6]; if (v > hi[6]) v = hi[6]; out[6] = v; }
+    { fom_float v = bias_t0[7] + (state[7] - 0.11245022456106747 * state[50] - 0.3148342557928728 * state[64]);
+      if (v < lo[7]) v = lo[7]; if (v > hi[7]) v = hi[7]; out[7] = v; }
+    { fom_float v = bias_t0[8] + (state[8] - 0.3148342557928728 * state[50] - 0.07906843296231456 * state[65]);
+      if (v < lo[8]) v = lo[8]; if (v > hi[8]) v = hi[8]; out[8] = v; }
+    { fom_float v = bias_t0[9] + (state[9] - 0.1012882957183817 * state[50] - 0.3148342557928728 * state[66]);
+      if (v < lo[9]) v = lo[9]; if (v > hi[9]) v = hi[9]; out[9] = v; }
+    { fom_float v = bias_t0[10] + (state[10] - 0.3148342557928728 * state[50] - 0.024432609086831557 * state[69]);
+      if (v < lo[10]) v = lo[10]; if (v > hi[10]) v = hi[10]; out[10] = v; }
+    { fom_float v = bias_t0[11] + (state[11] - 0.31483425576776075 * state[50] - 0.14375068353499568 * state[73]);
+      if (v < lo[11]) v = lo[11]; if (v > hi[11]) v = hi[11]; out[11] = v; }
+    { fom_float v = bias_t0[12] + (state[12] - 0.16283459527310534 * state[51] - 0.3148342557655042 * state[59]);
+      if (v < lo[12]) v = lo[12]; if (v > hi[12]) v = hi[12]; out[12] = v; }
+    { fom_float v = bias_t0[13] + (state[13] - 0.3148342557928728 * state[51] - 0.057878934426039565 * state[63]);
+      if (v < lo[13]) v = lo[13]; if (v > hi[13]) v = hi[13]; out[13] = v; }
+    { fom_float v = bias_t0[14] + (state[14] - 0.3148342557928728 * state[51] - 0.020371967614988162 * state[65]);
+      if (v < lo[14]) v = lo[14]; if (v > hi[14]) v = hi[14]; out[14] = v; }
+    { fom_float v = bias_t0[15] + (state[15] - 0.13195167489181564 * state[51] - 0.31483425579287283 * state[73]);
+      if (v < lo[15]) v = lo[15]; if (v > hi[15]) v = hi[15]; out[15] = v; }
+    { fom_float v = bias_t0[16] + (state[16] - 0.20484874346487622 * state[52] - 0.31483425569820117 * state[59]);
+      if (v < lo[16]) v = lo[16]; if (v > hi[16]) v = hi[16]; out[16] = v; }
+    { fom_float v = bias_t0[17] + (state[17] - 0.3148342557928728 * state[52] - 0.059210692177281046 * state[61]);
+      if (v < lo[17]) v = lo[17]; if (v > hi[17]) v = hi[17]; out[17] = v; }
+    { fom_float v = bias_t0[18] + (state[18] - 0.3148342557928728 * state[52] - 0.04186021818779605 * state[63]);
+      if (v < lo[18]) v = lo[18]; if (v > hi[18]) v = hi[18]; out[18] = v; }
+    { fom_float v = bias_t0[19] + (state[19] - 0.2013575399297713 * state[52] - 0.3148342556220557 * state[64]);
+      if (v < lo[19]) v = lo[19]; if (v > hi[19]) v = hi[19]; out[19] = v; }
+    { fom_float v = bias_t0[20] + (state[20] - 0.1717002983294537 * state[52] - 0.31483425579287283 * state[65]);
+      if (v < lo[20]) v = lo[20]; if (v > hi[20]) v = hi[20]; out[20] = v; }
+    { fom_float v = bias_t0[21] + (state[21] - 0.3148342557928728 * state[52] - 0.09586520152267977 * state[70]);
+      if (v < lo[21]) v = lo[21]; if (v > hi[21]) v = hi[21]; out[21] = v; }
+    { fom_float v = bias_t0[22] + (state[22] - 0.3148342557928728 * state[52] - 0.03542869495158811 * state[73]);
+      if (v < lo[22]) v = lo[22]; if (v > hi[22]) v = hi[22]; out[22] = v; }
+    { fom_float v = bias_t0[23] + (state[23] - 0.11549111655165208 * state[53] - 0.3148342557928728 * state[62]);
+      if (v < lo[23]) v = lo[23]; if (v > hi[23]) v = hi[23]; out[23] = v; }
+    { fom_float v = bias_t0[24] + (state[24] - 0.3148342557928728 * state[53] - 0.023238864598874573 * state[68]);
+      if (v < lo[24]) v = lo[24]; if (v > hi[24]) v = hi[24]; out[24] = v; }
+    { fom_float v = bias_t0[25] + (state[25] - 0.22703157183385472 * state[53] - 0.31483425579287283 * state[70]);
+      if (v < lo[25]) v = lo[25]; if (v > hi[25]) v = hi[25]; out[25] = v; }
+    { fom_float v = bias_t0[26] + (state[26] - 0.3148342557928728 * state[54] - 0.06685962708718948 * state[59]);
+      if (v < lo[26]) v = lo[26]; if (v > hi[26]) v = hi[26]; out[26] = v; }
+    { fom_float v = bias_t0[27] + (state[27] - 0.31483425578928326 * state[54] - 0.1807094243289255 * state[60]);
+      if (v < lo[27]) v = lo[27]; if (v > hi[27]) v = hi[27]; out[27] = v; }
+    { fom_float v = bias_t0[28] + (state[28] - 0.11812566027056481 * state[54] - 0.31483425579287283 * state[63]);
+      if (v < lo[28]) v = lo[28]; if (v > hi[28]) v = hi[28]; out[28] = v; }
+    { fom_float v = bias_t0[29] + (state[29] - 0.3148342557928728 * state[54] - 0.06983942457332966 * state[64]);
+      if (v < lo[29]) v = lo[29]; if (v > hi[29]) v = hi[29]; out[29] = v; }
+    { fom_float v = bias_t0[30] + (state[30] - 0.3148342557928728 * state[54] - 0.01829049354312028 * state[66]);
+      if (v < lo[30]) v = lo[30]; if (v > hi[30]) v = hi[30]; out[30] = v; }
+    { fom_float v = bias_t0[31] + (state[31] - 0.11621549871906076 * state[54] - 0.3148342557928728 * state[69]);
+      if (v < lo[31]) v = lo[31]; if (v > hi[31]) v = hi[31]; out[31] = v; }
+    { fom_float v = bias_t0[32] + (state[32] - 0.3148342557928728 * state[54] - 0.04147423462601573 * state[71]);
+      if (v < lo[32]) v = lo[32]; if (v > hi[32]) v = hi[32]; out[32] = v; }
+    { fom_float v = bias_t0[33] + (state[33] - 0.3148342557928728 * state[54] - 0.029904608409446035 * state[72]);
+      if (v < lo[33]) v = lo[33]; if (v > hi[33]) v = hi[33]; out[33] = v; }
+    { fom_float v = bias_t0[34] + (state[34] - 0.2007269945571799 * state[55] - 0.3148342557928728 * state[68]);
+      if (v < lo[34]) v = lo[34]; if (v > hi[34]) v = hi[34]; out[34] = v; }
+    { fom_float v = bias_t0[35] + (state[35] - 0.31483425579287283 * state[55] - 0.26432062531400063 * state[69]);
+      if (v < lo[35]) v = lo[35]; if (v > hi[35]) v = hi[35]; out[35] = v; }
+    { fom_float v = bias_t0[36] + (state[36] - 0.20810320529794363 * state[55] - 0.31483425575411544 * state[73]);
+      if (v < lo[36]) v = lo[36]; if (v > hi[36]) v = hi[36]; out[36] = v; }
+    { fom_float v = bias_t0[37] + (state[37] - 0.3148342557928728 * state[56] - 0.050057390438124606 * state[60]);
+      if (v < lo[37]) v = lo[37]; if (v > hi[37]) v = hi[37]; out[37] = v; }
+    { fom_float v = bias_t0[38] + (state[38] - 0.3148342557928728 * state[56] - 0.022355820755170213 * state[66]);
+      if (v < lo[38]) v = lo[38]; if (v > hi[38]) v = hi[38]; out[38] = v; }
+    { fom_float v = bias_t0[39] + (state[39] - 0.14832333169428388 * state[57] - 0.3148342557928728 * state[59]);
+      if (v < lo[39]) v = lo[39]; if (v > hi[39]) v = hi[39]; out[39] = v; }
+    { fom_float v = bias_t0[40] + (state[40] - 0.25687797426285774 * state[57] - 0.3148342557740812 * state[67]);
+      if (v < lo[40]) v = lo[40]; if (v > hi[40]) v = hi[40]; out[40] = v; }
+    { fom_float v = bias_t0[41] + (state[41] - 0.3148342556269916 * state[57] - 0.2046259837099301 * state[69]);
+      if (v < lo[41]) v = lo[41]; if (v > hi[41]) v = hi[41]; out[41] = v; }
+    { fom_float v = bias_t0[42] + (state[42] - 0.3148342557928728 * state[57] - 0.0669377754137904 * state[71]);
+      if (v < lo[42]) v = lo[42]; if (v > hi[42]) v = hi[42]; out[42] = v; }
+    { fom_float v = bias_t0[43] + (state[43] - 0.31483425573999474 * state[57] - 0.2549419703939111 * state[72]);
+      if (v < lo[43]) v = lo[43]; if (v > hi[43]) v = hi[43]; out[43] = v; }
+    { fom_float v = bias_t0[44] + (state[44] - 0.3148342557928728 * state[58] - 0.063379996679072 * state[65]);
+      if (v < lo[44]) v = lo[44]; if (v > hi[44]) v = hi[44]; out[44] = v; }
+    { fom_float v = bias_t0[45] + (state[45] - 0.2409314392941177 * state[58] - 0.31483425579287283 * state[67]);
+      if (v < lo[45]) v = lo[45]; if (v > hi[45]) v = hi[45]; out[45] = v; }
+    { fom_float v = bias_t0[46] + (state[46] - 0.15802369728139173 * state[58] - 0.3148342557928728 * state[71]);
+      if (v < lo[46]) v = lo[46]; if (v > hi[46]) v = hi[46]; out[46] = v; }
+    { fom_float v = bias_t0[47] + (state[47] - 0.21287292492070603 * state[58] - 0.3148342557928728 * state[72]);
+      if (v < lo[47]) v = lo[47]; if (v > hi[47]) v = hi[47]; out[47] = v; }
+    { fom_float v = bias_t0[48] + (state[48] - 0.2746827012680003 * state[58] - 0.3148342555778952 * state[73]);
+      if (v < lo[48]) v = lo[48]; if (v > hi[48]) v = hi[48]; out[48] = v; }
+}
+
+void fom_step_t1(const fom_float *state, const fom_float *t_t0, fom_float sc_xi, const fom_float *bias, const FomAct_t1 *act, fom_float *out) {
+    const fom_float *bias_t1 = bias; const fom_float *lo = act->lo;
+    { fom_float v = bias_t1[0] + (-0.31483425579287283 * state[0] - 0.17851063193927913 * state[1] - 0.3148342556510004 * state[2] - 0.31483425579287283 * state[3] - 0.31483425579287283 * state[4] + state[49] + 0.6296685115857455 * t_t0[0] + 0.35702126387855826 * t_t0[1] + 0.6296685113020011 * t_t0[2] + 0.6296685115857455 * t_t0[3] + 0.6296685115857455 * t_t0[4]) + (sc_xi * (-0.6296685115857453 * state[0] - 0.3570212638785581 * state[1] - 0.6296685113020006 * state[2] - 0.6296685115857453 * state[3] - 0.6296685115857453 * state[4] + 0.6296685115857453 * t_t0[0] + 0.3570212638785581 * t_t0[1] + 0.6296685113020003 * t_t0[2] + 0.6296685115857453 * t_t0[3] + 0.6296685115857453 * t_t0[4]));
+      if (v < lo[0]) v = lo[0]; out[0] = v; }
+    { fom_float v = bias_t1[1] + (-0.31483425579287283 * state[5] - 0.11490990424160037 * state[6] - 0.11245022456106753 * state[7] - 0.31483425579287283 * state[8] - 0.10128829571838176 * state[9] - 0.31483425579287283 * state[10] - 0.3148342557677609 * state[11] + state[50] + 0.6296685115857455 * t_t0[5] + 0.22981980848320072 * t_t0[6] + 0.22490044912213497 * t_t0[7] + 0.6296685115857455 * t_t0[8] + 0.20257659143676343 * t_t0[9] + 0.6296685115857455 * t_t0[10] + 0.6296685115355216 * t_t0[11]) + (sc_xi * (-0.6296685115857453 * state[5] - 0.2298198084832004 * state[6] - 0.22490044912213478 * state[7] - 0.6296685115857453 * state[8] - 0.2025765914367633 * state[9] - 0.6296685115857453 * state[10] - 0.6296685115355208 * state[11] + 0.6296685115857453 * t_t0[5] + 0.22981980848320033 * t_t0[6] + 0.22490044912213478 * t_t0[7] + 0.6296685115857453 * t_t0[8] + 0.2025765914367634 * t_t0[9] + 0.6296685115857453 * t_t0[10] + 0.6296685115355208 * t_t0[11]));
+      if (v < lo[1]) v = lo[1]; out[1] = v; }
+    { fom_float v = bias_t1[2] + (-0.16283459527310534 * state[12] - 0.31483425579287283 * state[13] - 0.31483425579287283 * state[14] - 0.1319516748918157 * state[15] + state[51] + 0.3256691905462108 * t_t0[12] + 0.6296685115857455 * t_t0[13] + 0.6296685115857455 * t_t0[14] + 0.2639033497836314 * t_t0[15]) + (sc_xi * (-0.32566919054621074 * state[12] - 0.6296685115857453 * state[13] - 0.6296685115857453 * state[14] - 0.26390334978363106 * state[15] + 0.3256691905462105 * t_t0[12] + 0.6296685115857453 * t_t0[13] + 0.6296685115857453 * t_t0[14] + 0.26390334978363095 * t_t0[15]));
+      if (v < lo[2]) v = lo[2]; out[2] = v; }
+    { fom_float v = bias_t1[3] + (-0.2048487434648763 * state[16] - 0.31483425579287283 * state[17] - 0.31483425579287283 * state[18] - 0.20135753992977132 * state[19] - 0.17170029832945377 * state[20] - 0.31483425579287283 * state[21] - 0.31483425579287283 * state[22] + state[52] + 0.4096974869297525 * t_t0[16] + 0.6296685115857455 * t_t0[17] + 0.6296685115857455 * t_t0[18] + 0.4027150798595428 * t_t0[19] + 0.3434005966589075 * t_t0[20] + 0.6296685115857455 * t_t0[21] + 0.6296685115857455 * t_t0[22]) + (sc_xi * (-0.40969748692975216 * state[16] - 0.6296685115857453 * state[17] - 0.6296685115857453 * state[18] - 0.40271507985954247 * state[19] - 0.34340059665890715 * state[20] - 0.6296685115857453 * state[21] - 0.6296685115857453 * state[22] + 0.4096974869297524 * t_t0[16] + 0.6296685115857453 * t_t0[17] + 0.6296685115857453 * t_t0[18] + 0.402715079859542 * t_t0[19] + 0.34340059665890715 * t_t0[20] + 0.6296685115857453 * t_t0[21] + 0.6296685115857453 * t_t0[22]));
+      if (v < lo[3]) v = lo[3]; out[3] = v; }
+    { fom_float v = bias_t1[4] + (-0.11549111655165209 * state[23] - 0.31483425579287283 * state[24] - 0.2270315718338548 * state[25] + state[53] + 0.2309822331033042 * t_t0[23] + 0.6296685115857455 * t_t0[24] + 0.4540631436677096 * t_t0[25]) + (sc_xi * (-0.23098223310330407 * state[23] - 0.6296685115857453 * state[24] - 0.45406314366770917 * state[25] + 0.23098223310330407 * t_t0[23] + 0.6296685115857453 * t_t0[24] + 0.45406314366770895 * t_t0[25]));
+      if (v < lo[4]) v = lo[4]; out[4] = v; }
+    { fom_float v = bias_t1[5] + (-0.31483425579287283 * state[26] - 0.3148342557892835 * state[27] - 0.11812566027056481 * state[28] - 0.31483425579287283 * state[29] - 0.31483425579287283 * state[30] - 0.11621549871906077 * state[31] - 0.31483425579287283 * state[32] - 0.31483425579287283 * state[33] + state[54] + 0.6296685115857455 * t_t0[26] + 0.6296685115785666 * t_t0[27] + 0.23625132054112963 * t_t0[28] + 0.6296685115857455 * t_t0[29] + 0.6296685115857455 * t_t0[30] + 0.23243099743812146 * t_t0[31] + 0.6296685115857455 * t_t0[32] + 0.6296685115857455 * t_t0[33]) + (sc_xi * (-0.6296685115857453 * state[26] - 0.6296685115785658 * state[27] - 0.23625132054112963 * state[28] - 0.6296685115857453 * state[29] - 0.6296685115857453 * state[30] - 0.2324309974381214 * state[31] - 0.6296685115857453 * state[32] - 0.6296685115857453 * state[33] + 0.6296685115857453 * t_t0[26] + 0.6296685115785663 * t_t0[27] + 0.23625132054112963 * t_t0[28] + 0.6296685115857453 * t_t0[29] + 0.6296685115857453 * t_t0[30] + 0.2324309974381216 * t_t0[31] + 0.6296685115857453 * t_t0[32] + 0.6296685115857453 * t_t0[33]));
+      if (v < lo[5]) v = lo[5]; out[5] = v; }
+    { fom_float v = bias_t1[6] + (-0.2007269945571799 * state[34] - 0.31483425579287294 * state[35] - 0.20810320529794374 * state[36] + state[55] + 0.4014539891143599 * t_t0[34] + 0.6296685115857457 * t_t0[35] + 0.41620641059588726 * t_t0[36]) + (sc_xi * (-0.40145398911435987 * state[34] - 0.6296685115857453 * state[35] - 0.41620641059588687 * state[36] + 0.40145398911435964 * t_t0[34] + 0.6296685115857458 * t_t0[35] + 0.4162064105958871 * t_t0[36]));
+      if (v < lo[6]) v = lo[6]; out[6] = v; }
+    { fom_float v = bias_t1[7] + (-0.31483425579287283 * state[37] - 0.31483425579287283 * state[38] + state[56] + 0.6296685115857455 * t_t0[37] + 0.6296685115857455 * t_t0[38]) + (sc_xi * (-0.6296685115857453 * state[37] - 0.6296685115857453 * state[38] + 0.6296685115857453 * t_t0[37] + 0.6296685115857453 * t_t0[38]));
+      if (v < lo[7]) v = lo[7]; out[7] = v; }
+    { fom_float v = bias_t1[8] + (-0.14832333169428388 * state[39] - 0.25687797426285774 * state[40] - 0.31483425562699185 * state[41] - 0.31483425579287283 * state[42] - 0.3148342557399948 * state[43] + state[57] + 0.29664666338856793 * t_t0[39] + 0.5137559485257156 * t_t0[40] + 0.6296685112539833 * t_t0[41] + 0.6296685115857455 * t_t0[42] + 0.6296685114799897 * t_t0[43]) + (sc_xi * (-0.2966466633885678 * state[39] - 0.5137559485257154 * state[40] - 0.6296685112539826 * state[41] - 0.6296685115857453 * state[42] - 0.629668511479989 * state[43] + 0.2966466633885674 * t_t0[39] + 0.5137559485257152 * t_t0[40] + 0.629668511253983 * t_t0[41] + 0.6296685115857453 * t_t0[42] + 0.6296685114799887 * t_t0[43]));
+      if (v < lo[8]) v = lo[8]; out[8] = v; }
+    { fom_float v = bias_t1[9] + (-0.31483425579287283 * state[44] - 0.2409314392941178 * state[45] - 0.15802369728139182 * state[46] - 0.21287292492070606 * state[47] - 0.2746827012680003 * state[48] + state[58] + 0.6296685115857455 * t_t0[44] + 0.48186287858823557 * t_t0[45] + 0.3160473945627835 * t_t0[46] + 0.42574584984141206 * t_t0[47] + 0.5493654025360007 * t_t0[48]) + (sc_xi * (-0.6296685115857453 * state[44] - 0.481862878588235 * state[45] - 0.31604739456278325 * state[46] - 0.425745849841412 * state[47] - 0.5493654025360006 * state[48] + 0.6296685115857453 * t_t0[44] + 0.481862878588235 * t_t0[45] + 0.31604739456278325 * t_t0[46] + 0.425745849841412 * t_t0[47] + 0.5493654025360004 * t_t0[48]));
+      if (v < lo[9]) v = lo[9]; out[9] = v; }
+}
+
+void fom_step_su(const fom_float *state, const fom_float *t_t0, const fom_float *t_t1, fom_float sc_xi, const fom_float *bias, fom_float *out) {
+    const fom_float *bias_su = bias;
+    out[0] = bias_su[0] + (t_t0[0]);
+    out[1] = bias_su[1] + (t_t0[1]);
+    out[2] = bias_su[2] + (t_t0[2]);
+    out[3] = bias_su[3] + (t_t0[3]);
+    out[4] = bias_su[4] + (t_t0[4]);
+    out[5] = bias_su[5] + (t_t0[5]);
+    out[6] = bias_su[6] + (t_t0[6]);
+    out[7] = bias_su[7] + (t_t0[7]);
+    out[8] = bias_su[8] + (t_t0[8]);
+    out[9] = bias_su[9] + (t_t0[9]);
+    out[10] = bias_su[10] + (t_t0[10]);
+    out[11] = bias_su[11] + (t_t0[11]);
+    out[12] = bias_su[12] + (t_t0[12]);
+    out[13] = bias_su[13] + (t_t0[13]);
+    out[14] = bias_su[14] + (t_t0[14]);
+    out[15] = bias_su[15] + (t_t0[15]);
+    out[16] = bias_su[16] + (t_t0[16]);
+    out[17] = bias_su[17] + (t_t0[17]);
+    out[18] = bias_su[18] + (t_t0[18]);
+    out[19] = bias_su[19] + (t_t0[19]);
+    out[20] = bias_su[20] + (t_t0[20]);
+    out[21] = bias_su[21] + (t_t0[21]);
+    out[22] = bias_su[22] + (t_t0[22]);
+    out[23] = bias_su[23] + (t_t0[23]);
+    out[24] = bias_su[24] + (t_t0[24]);
+    out[25] = bias_su[25] + (t_t0[25]);
+    out[26] = bias_su[26] + (t_t0[26]);
+    out[27] = bias_su[27] + (t_t0[27]);
+    out[28] = bias_su[28] + (t_t0[28]);
+    out[29] = bias_su[29] + (t_t0[29]);
+    out[30] = bias_su[30] + (t_t0[30]);
+    out[31] = bias_su[31] + (t_t0[31]);
+    out[32] = bias_su[32] + (t_t0[32]);
+    out[33] = bias_su[33] + (t_t0[33]);
+    out[34] = bias_su[34] + (t_t0[34]);
+    out[35] = bias_su[35] + (t_t0[35]);
+    out[36] = bias_su[36] + (t_t0[36]);
+    out[37] = bias_su[37] + (t_t0[37]);
+    out[38] = bias_su[38] + (t_t0[38]);
+    out[39] = bias_su[39] + (t_t0[39]);
+    out[40] = bias_su[40] + (t_t0[40]);
+    out[41] = bias_su[41] + (t_t0[41]);
+    out[42] = bias_su[42] + (t_t0[42]);
+    out[43] = bias_su[43] + (t_t0[43]);
+    out[44] = bias_su[44] + (t_t0[44]);
+    out[45] = bias_su[45] + (t_t0[45]);
+    out[46] = bias_su[46] + (t_t0[46]);
+    out[47] = bias_su[47] + (t_t0[47]);
+    out[48] = bias_su[48] + (t_t0[48]);
+    out[49] = bias_su[49] + (t_t1[0]);
+    out[50] = bias_su[50] + (t_t1[1]);
+    out[51] = bias_su[51] + (t_t1[2]);
+    out[52] = bias_su[52] + (t_t1[3]);
+    out[53] = bias_su[53] + (t_t1[4]);
+    out[54] = bias_su[54] + (t_t1[5]);
+    out[55] = bias_su[55] + (t_t1[6]);
+    out[56] = bias_su[56] + (t_t1[7]);
+    out[57] = bias_su[57] + (t_t1[8]);
+    out[58] = bias_su[58] + (t_t1[9]);
+    out[59] = bias_su[59] + (-0.12559068930987088 * state[0] - 0.31483425576550444 * state[12] - 0.3148342556982012 * state[16] - 0.0668596270871895 * state[26] - 0.31483425579287283 * state[39] + state[59] + 0.2511813786197419 * t_t0[0] + 0.6296685115310086 * t_t0[12] + 0.6296685113964026 * t_t0[16] + 0.13371925417437902 * t_t0[26] + 0.6296685115857455 * t_t0[39]) + (sc_xi * (-0.25118137861974177 * state[0] - 0.6296685115310079 * state[12] - 0.6296685113964021 * state[16] - 0.1337192541743789 * state[26] - 0.6296685115857453 * state[39] + 0.25118137861974166 * t_t0[0] + 0.6296685115310079 * t_t0[12] + 0.6296685113964017 * t_t0[16] + 0.13371925417437883 * t_t0[26] + 0.6296685115857453 * t_t0[39]));
+    out[60] = bias_su[60] + (-0.31483425579287294 * state[1] - 0.08155612991849118 * state[5] - 0.18070942432892556 * state[27] - 0.05005739043812461 * state[37] + state[60] + 0.6296685115857457 * t_t0[1] + 0.16311225983698227 * t_t0[5] + 0.36141884865785107 * t_t0[27] + 0.1001147808762492 * t_t0[37]) + (sc_xi * (-0.6296685115857453 * state[1] - 0.16311225983698213 * state[5] - 0.361418848657851 * state[27] - 0.10011478087624918 * state[37] + 0.6296685115857458 * t_t0[1] + 0.16311225983698224 * t_t0[5] + 0.361418848657851 * t_t0[27] + 0.10011478087624924 * t_t0[37]));
+    out[61] = bias_su[61] + (-0.31483425579287294 * state[6] - 0.05921069217728105 * state[17] + state[61] + 0.6296685115857457 * t_t0[6] + 0.11842138435456212 * t_t0[17]) + (sc_xi * (-0.6296685115857453 * state[6] - 0.11842138435456206 * state[17] + 0.6296685115857458 * t_t0[6] + 0.11842138435456201 * t_t0[17]));
+    out[62] = bias_su[62] + (-0.18736322506593045 * state[2] - 0.31483425579287283 * state[23] + state[62] + 0.3747264501318611 * t_t0[2] + 0.6296685115857455 * t_t0[23]) + (sc_xi * (-0.37472645013186096 * state[2] - 0.6296685115857453 * state[23] + 0.3747264501318605 * t_t0[2] + 0.6296685115857453 * t_t0[23]));
+    out[63] = bias_su[63] + (-0.05787893442603958 * state[13] - 0.04186021818779605 * state[18] - 0.31483425579287294 * state[28] + state[63] + 0.11575786885207912 * t_t0[13] + 0.08372043637559212 * t_t0[18] + 0.6296685115857457 * t_t0[28]) + (sc_xi * (-0.11575786885207907 * state[13] - 0.08372043637559205 * state[18] - 0.6296685115857453 * state[28] + 0.11575786885207907 * t_t0[13] + 0.083720436375592 * t_t0[18] + 0.6296685115857458 * t_t0[28]));
+    out[64] = bias_su[64] + (-0.31483425579287283 * state[7] - 0.31483425562205586 * state[19] - 0.06983942457332967 * state[29] + state[64] + 0.6296685115857455 * t_t0[7] + 0.6296685112441113 * t_t0[19] + 0.13967884914665932 * t_t0[29]) + (sc_xi * (-0.6296685115857453 * state[7] - 0.6296685112441109 * state[19] - 0.13967884914665926 * state[29] + 0.6296685115857453 * t_t0[7] + 0.6296685112441114 * t_t0[19] + 0.13967884914665926 * t_t0[29]));
+    out[65] = bias_su[65] + (-0.05210541922399309 * state[3] - 0.07906843296231458 * state[8] - 0.02037196761498816 * state[14] - 0.31483425579287294 * state[20] - 0.06337999667907203 * state[44] + state[65] + 0.10421083844798616 * t_t0[3] + 0.1581368659246291 * t_t0[8] + 0.04074393522997634 * t_t0[14] + 0.6296685115857457 * t_t0[20] + 0.12675999335814403 * t_t0[44]) + (sc_xi * (-0.10421083844798612 * state[3] - 0.15813686592462906 * state[8] - 0.040743935229976325 * state[14] - 0.6296685115857453 * state[20] - 0.12675999335814395 * state[44] + 0.10421083844798612 * t_t0[3] + 0.15813686592462906 * t_t0[8] + 0.040743935229976304 * t_t0[14] + 0.6296685115857458 * t_t0[20] + 0.12675999335814397 * t_t0[44]));
+    out[66] = bias_su[66] + (-0.31483425579287283 * state[9] - 0.01829049354312029 * state[30] - 0.022355820755170213 * state[38] + state[66] + 0.6296685115857455 * t_t0[9] + 0.03658098708624057 * t_t0[30] + 0.04471164151034043 * t_t0[38]) + (sc_xi * (-0.6296685115857453 * state[9] - 0.03658098708624052 * state[30] - 0.04471164151034042 * state[38] + 0.6296685115857453 * t_t0[9] + 0.03658098708624052 * t_t0[30] + 0.0447116415103404 * t_t0[38]));
+    out[67] = bias_su[67] + (-0.3148342557740813 * state[40] - 0.31483425579287294 * state[45] + state[67] + 0.6296685115481624 * t_t0[40] + 0.6296685115857457 * t_t0[45]) + (sc_xi * (-0.629668511548162 * state[40] - 0.6296685115857453 * state[45] + 0.6296685115481624 * t_t0[40] + 0.6296685115857458 * t_t0[45]));
+    out[68] = bias_su[68] + (-0.02323886459887458 * state[24] - 0.31483425579287283 * state[34] + state[68] + 0.04647772919774916 * t_t0[24] + 0.6296685115857455 * t_t0[34]) + (sc_xi * (-0.04647772919774914 * state[24] - 0.6296685115857453 * state[34] + 0.04647772919774911 * t_t0[24] + 0.6296685115857453 * t_t0[34]));
+    out[69] = bias_su[69] + (-0.024432609086831564 * state[10] - 0.31483425579287283 * state[31] - 0.26432062531400075 * state[35] - 0.20462598370993018 * state[41] + state[69] + 0.04886521817366314 * t_t0[10] + 0.6296685115857455 * t_t0[31] + 0.5286412506280014 * t_t0[35] + 0.40925196741986025 * t_t0[41]) + (sc_xi * (-0.0488652181736631 * state[10] - 0.6296685115857453 * state[31] - 0.5286412506280009 * state[35] - 0.4092519674198599 * state[41] + 0.04886521817366307 * t_t0[10] + 0.6296685115857453 * t_t0[31] + 0.5286412506280012 * t_t0[35] + 0.40925196741986014 * t_t0[41]));
+    out[70] = bias_su[70] + (-0.09586520152267985 * state[21] - 0.31483425579287294 * state[25] + state[70] + 0.1917304030453596 * t_t0[21] + 0.6296685115857457 * t_t0[25]) + (sc_xi * (-0.19173040304535932 * state[21] - 0.6296685115857453 * state[25] + 0.19173040304535943 * t_t0[21] + 0.6296685115857458 * t_t0[25]));
+    out[71] = bias_su[71] + (-0.042197122740314105 * state[4] - 0.04147423462601575 * state[32] - 0.0669377754137904 * state[42] - 0.31483425579287283 * state[46] + state[71] + 0.08439424548062816 * t_t0[4] + 0.08294846925203149 * t_t0[32] + 0.1338755508275808 * t_t0[42] + 0.6296685115857455 * t_t0[46]) + (sc_xi * (-0.08439424548062809 * state[4] - 0.08294846925203142 * state[32] - 0.1338755508275808 * state[42] - 0.6296685115857453 * state[46] + 0.08439424548062814 * t_t0[4] + 0.08294846925203136 * t_t0[32] + 0.1338755508275808 * t_t0[42] + 0.6296685115857453 * t_t0[46]));
+    out[72] = bias_su[72] + (-0.02990460840944605 * state[33] - 0.25494197039391125 * state[43] - 0.31483425579287283 * state[47] + state[72] + 0.05980921681889208 * t_t0[33] + 0.5098839407878222 * t_t0[43] + 0.6296685115857455 * t_t0[47]) + (sc_xi * (-0.05980921681889204 * state[33] - 0.5098839407878217 * state[43] - 0.6296685115857453 * state[47] + 0.05980921681889207 * t_t0[33] + 0.509883940787822 * t_t0[43] + 0.6296685115857453 * t_t0[47]));
+    out[73] = bias_su[73] + (-0.14375068353499568 * state[11] - 0.31483425579287294 * state[15] - 0.03542869495158812 * state[22] - 0.3148342557541156 * state[36] - 0.31483425557789524 * state[48] + state[73] + 0.28750136706999146 * t_t0[11] + 0.6296685115857457 * t_t0[15] + 0.07085738990317625 * t_t0[22] + 0.6296685115082311 * t_t0[36] + 0.6296685111557906 * t_t0[48]) + (sc_xi * (-0.2875013670699913 * state[11] - 0.6296685115857453 * state[15] - 0.0708573899031762 * state[22] - 0.6296685115082304 * state[36] - 0.6296685111557904 * state[48] + 0.28750136706999097 * t_t0[11] + 0.6296685115857458 * t_t0[15] + 0.07085738990317617 * t_t0[22] + 0.6296685115082304 * t_t0[36] + 0.6296685111557899 * t_t0[48]));
+}
+
 void fom_step_su__rest(const fom_float *state, const fom_float *t_t0, const fom_float *t_t1, fom_float sc_xi, const fom_float *bias, fom_float *out) {
-    fom_int i, r, p;
-    for (r = 0; r < FOM_REST_su_N; ++r) {
-        fom_float s, v;
-        i = FOM_REST_su_ROWS[r];
-        v = bias[i];
-        s = 0.0;
-        for (p = FOM_MAT_su_state_ROWPTR[i]; p < FOM_MAT_su_state_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_state_VALS[p] * state[FOM_MAT_su_state_COLIDX[p]];
-        v += s;
-        s = 0.0;
-        for (p = FOM_MAT_su_state__d_xi_ROWPTR[i]; p < FOM_MAT_su_state__d_xi_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_state__d_xi_VALS[p] * state[FOM_MAT_su_state__d_xi_COLIDX[p]];
-        v += sc_xi * s;
-        s = 0.0;
-        for (p = FOM_MAT_su_t_t0_ROWPTR[i]; p < FOM_MAT_su_t_t0_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t0_VALS[p] * t_t0[FOM_MAT_su_t_t0_COLIDX[p]];
-        v += s;
-        s = 0.0;
-        for (p = FOM_MAT_su_t_t0__d_xi_ROWPTR[i]; p < FOM_MAT_su_t_t0__d_xi_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t0__d_xi_VALS[p] * t_t0[FOM_MAT_su_t_t0__d_xi_COLIDX[p]];
-        v += sc_xi * s;
-        s = 0.0;
-        for (p = FOM_MAT_su_t_t1_ROWPTR[i]; p < FOM_MAT_su_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t1_VALS[p] * t_t1[FOM_MAT_su_t_t1_COLIDX[p]];
-        v += s;
-        out[i] = v;
-    }
+    const fom_float *bias_su = bias;
+    out[59] = bias_su[59] + (-0.12559068930987088 * state[0] - 0.31483425576550444 * state[12] - 0.3148342556982012 * state[16] - 0.0668596270871895 * state[26] - 0.31483425579287283 * state[39] + state[59] + 0.2511813786197419 * t_t0[0] + 0.6296685115310086 * t_t0[12] + 0.6296685113964026 * t_t0[16] + 0.13371925417437902 * t_t0[26] + 0.6296685115857455 * t_t0[39]) + (sc_xi * (-0.25118137861974177 * state[0] - 0.6296685115310079 * state[12] - 0.6296685113964021 * state[16] - 0.1337192541743789 * state[26] - 0.6296685115857453 * state[39] + 0.25118137861974166 * t_t0[0] + 0.6296685115310079 * t_t0[12] + 0.6296685113964017 * t_t0[16] + 0.13371925417437883 * t_t0[26] + 0.6296685115857453 * t_t0[39]));
+    out[60] = bias_su[60] + (-0.31483425579287294 * state[1] - 0.08155612991849118 * state[5] - 0.18070942432892556 * state[27] - 0.05005739043812461 * state[37] + state[60] + 0.6296685115857457 * t_t0[1] + 0.16311225983698227 * t_t0[5] + 0.36141884865785107 * t_t0[27] + 0.1001147808762492 * t_t0[37]) + (sc_xi * (-0.6296685115857453 * state[1] - 0.16311225983698213 * state[5] - 0.361418848657851 * state[27] - 0.10011478087624918 * state[37] + 0.6296685115857458 * t_t0[1] + 0.16311225983698224 * t_t0[5] + 0.361418848657851 * t_t0[27] + 0.10011478087624924 * t_t0[37]));
+    out[61] = bias_su[61] + (-0.31483425579287294 * state[6] - 0.05921069217728105 * state[17] + state[61] + 0.6296685115857457 * t_t0[6] + 0.11842138435456212 * t_t0[17]) + (sc_xi * (-0.6296685115857453 * state[6] - 0.11842138435456206 * state[17] + 0.6296685115857458 * t_t0[6] + 0.11842138435456201 * t_t0[17]));
+    out[62] = bias_su[62] + (-0.18736322506593045 * state[2] - 0.31483425579287283 * state[23] + state[62] + 0.3747264501318611 * t_t0[2] + 0.6296685115857455 * t_t0[23]) + (sc_xi * (-0.37472645013186096 * state[2] - 0.6296685115857453 * state[23] + 0.3747264501318605 * t_t0[2] + 0.6296685115857453 * t_t0[23]));
+    out[63] = bias_su[63] + (-0.05787893442603958 * state[13] - 0.04186021818779605 * state[18] - 0.31483425579287294 * state[28] + state[63] + 0.11575786885207912 * t_t0[13] + 0.08372043637559212 * t_t0[18] + 0.6296685115857457 * t_t0[28]) + (sc_xi * (-0.11575786885207907 * state[13] - 0.08372043637559205 * state[18] - 0.6296685115857453 * state[28] + 0.11575786885207907 * t_t0[13] + 0.083720436375592 * t_t0[18] + 0.6296685115857458 * t_t0[28]));
+    out[64] = bias_su[64] + (-0.31483425579287283 * state[7] - 0.31483425562205586 * state[19] - 0.06983942457332967 * state[29] + state[64] + 0.6296685115857455 * t_t0[7] + 0.6296685112441113 * t_t0[19] + 0.13967884914665932 * t_t0[29]) + (sc_xi * (-0.6296685115857453 * state[7] - 0.6296685112441109 * state[19] - 0.13967884914665926 * state[29] + 0.6296685115857453 * t_t0[7] + 0.6296685112441114 * t_t0[19] + 0.13967884914665926 * t_t0[29]));
+    out[65] = bias_su[65] + (-0.05210541922399309 * state[3] - 0.07906843296231458 * state[8] - 0.02037196761498816 * state[14] - 0.31483425579287294 * state[20] - 0.06337999667907203 * state[44] + state[65] + 0.10421083844798616 * t_t0[3] + 0.1581368659246291 * t_t0[8] + 0.04074393522997634 * t_t0[14] + 0.6296685115857457 * t_t0[20] + 0.12675999335814403 * t_t0[44]) + (sc_xi * (-0.10421083844798612 * state[3] - 0.15813686592462906 * state[8] - 0.040743935229976325 * state[14] - 0.6296685115857453 * state[20] - 0.12675999335814395 * state[44] + 0.10421083844798612 * t_t0[3] + 0.15813686592462906 * t_t0[8] + 0.040743935229976304 * t_t0[14] + 0.6296685115857458 * t_t0[20] + 0.12675999335814397 * t_t0[44]));
+    out[66] = bias_su[66] + (-0.31483425579287283 * state[9] - 0.01829049354312029 * state[30] - 0.022355820755170213 * state[38] + state[66] + 0.6296685115857455 * t_t0[9] + 0.03658098708624057 * t_t0[30] + 0.04471164151034043 * t_t0[38]) + (sc_xi * (-0.6296685115857453 * state[9] - 0.03658098708624052 * state[30] - 0.04471164151034042 * state[38] + 0.6296685115857453 * t_t0[9] + 0.03658098708624052 * t_t0[30] + 0.0447116415103404 * t_t0[38]));
+    out[67] = bias_su[67] + (-0.3148342557740813 * state[40] - 0.31483425579287294 * state[45] + state[67] + 0.6296685115481624 * t_t0[40] + 0.6296685115857457 * t_t0[45]) + (sc_xi * (-0.629668511548162 * state[40] - 0.6296685115857453 * state[45] + 0.6296685115481624 * t_t0[40] + 0.6296685115857458 * t_t0[45]));
+    out[68] = bias_su[68] + (-0.02323886459887458 * state[24] - 0.31483425579287283 * state[34] + state[68] + 0.04647772919774916 * t_t0[24] + 0.6296685115857455 * t_t0[34]) + (sc_xi * (-0.04647772919774914 * state[24] - 0.6296685115857453 * state[34] + 0.04647772919774911 * t_t0[24] + 0.6296685115857453 * t_t0[34]));
+    out[69] = bias_su[69] + (-0.024432609086831564 * state[10] - 0.31483425579287283 * state[31] - 0.26432062531400075 * state[35] - 0.20462598370993018 * state[41] + state[69] + 0.04886521817366314 * t_t0[10] + 0.6296685115857455 * t_t0[31] + 0.5286412506280014 * t_t0[35] + 0.40925196741986025 * t_t0[41]) + (sc_xi * (-0.0488652181736631 * state[10] - 0.6296685115857453 * state[31] - 0.5286412506280009 * state[35] - 0.4092519674198599 * state[41] + 0.04886521817366307 * t_t0[10] + 0.6296685115857453 * t_t0[31] + 0.5286412506280012 * t_t0[35] + 0.40925196741986014 * t_t0[41]));
+    out[70] = bias_su[70] + (-0.09586520152267985 * state[21] - 0.31483425579287294 * state[25] + state[70] + 0.1917304030453596 * t_t0[21] + 0.6296685115857457 * t_t0[25]) + (sc_xi * (-0.19173040304535932 * state[21] - 0.6296685115857453 * state[25] + 0.19173040304535943 * t_t0[21] + 0.6296685115857458 * t_t0[25]));
+    out[71] = bias_su[71] + (-0.042197122740314105 * state[4] - 0.04147423462601575 * state[32] - 0.0669377754137904 * state[42] - 0.31483425579287283 * state[46] + state[71] + 0.08439424548062816 * t_t0[4] + 0.08294846925203149 * t_t0[32] + 0.1338755508275808 * t_t0[42] + 0.6296685115857455 * t_t0[46]) + (sc_xi * (-0.08439424548062809 * state[4] - 0.08294846925203142 * state[32] - 0.1338755508275808 * state[42] - 0.6296685115857453 * state[46] + 0.08439424548062814 * t_t0[4] + 0.08294846925203136 * t_t0[32] + 0.1338755508275808 * t_t0[42] + 0.6296685115857453 * t_t0[46]));
+    out[72] = bias_su[72] + (-0.02990460840944605 * state[33] - 0.25494197039391125 * state[43] - 0.31483425579287283 * state[47] + state[72] + 0.05980921681889208 * t_t0[33] + 0.5098839407878222 * t_t0[43] + 0.6296685115857455 * t_t0[47]) + (sc_xi * (-0.05980921681889204 * state[33] - 0.5098839407878217 * state[43] - 0.6296685115857453 * state[47] + 0.05980921681889207 * t_t0[33] + 0.509883940787822 * t_t0[43] + 0.6296685115857453 * t_t0[47]));
+    out[73] = bias_su[73] + (-0.14375068353499568 * state[11] - 0.31483425579287294 * state[15] - 0.03542869495158812 * state[22] - 0.3148342557541156 * state[36] - 0.31483425557789524 * state[48] + state[73] + 0.28750136706999146 * t_t0[11] + 0.6296685115857457 * t_t0[15] + 0.07085738990317625 * t_t0[22] + 0.6296685115082311 * t_t0[36] + 0.6296685111557906 * t_t0[48]) + (sc_xi * (-0.2875013670699913 * state[11] - 0.6296685115857453 * state[15] - 0.0708573899031762 * state[22] - 0.6296685115082304 * state[36] - 0.6296685111557904 * state[48] + 0.28750136706999097 * t_t0[11] + 0.6296685115857458 * t_t0[15] + 0.07085738990317617 * t_t0[22] + 0.6296685115082304 * t_t0[36] + 0.6296685111557899 * t_t0[48]));
 }
 
 void fom_iterate(const fom_float *state, fom_float sc_xi, const fom_float *bias_t0, const fom_float *bias_t1, const fom_float *bias_su, const FomAct_t0 *act_t0, const FomAct_t1 *act_t1, fom_float *t_t0, fom_float *t_t1, fom_float *new_state) {
@@ -257,3 +392,4 @@
     fom_step_t1(state, t_t0, sc_xi, bias_t1, act_t1, t_t1);
     fom_step_su(state, t_t0, t_t1, sc_xi, bias_su, new_state);
 }
+
```

## Agent notes

```
### Round 1

**Whole-solve `mean_us`:** initial 41.32 µs, final about 17.2 µs (three runs: 17.42, 16.57, 17.21; a fourth gave 17.45). Speedup is about 2.4x.

**Final profile breakdown** (`make profile`; the profiled build runs at about 35 µs because of timer overhead, so the shares matter more than the absolute times):
| Region | Time (µs, 5000 solves) | % |
|---|---|---|
| step_su_rest | 37329 | 35.1 |
| step_t0 | 37099 | 34.9 |
| step_t1 | 32025 | 30.1 |

**Optimizations applied:**
1. **Unrolled the step kernels with literal coefficients** (`fom_step_t0`, `fom_step_t1`, `fom_step_su`, `fom_step_su__rest`). These routines are checked on every unit vector, so the rules allow literal coefficients there. The routines are generated from the CSR data in `fom_data.c`, and each coefficient is written at full round-trip precision.
   - Structure exploited: the sparsity pattern is small and fixed (about 3 nonzeros per row in `t0_state`, about 10 per row in t1 and in the 15 remaining su rows).
   - Removed: all row-pointer and column-index loads, the inner-loop control, and the multiplies for the many unit (±1) coefficients, such as the identity part of `t0_state` and the `state[49+i]` term in t1.
2. **Fused the two products scaled by the schedule value.** The `state__d_xi` and `t_t0__d_xi` terms of each row are summed together and multiplied by `sc_xi` once per row. Likewise, the products of the plain matrices `state`, `t_t0` and `t_t1` form one expression per row.
   - Structure exploited: each output block's members share the same schedule value.
   - This removes a multiply per member and gives the compiler longer independent chains to schedule.
3. **Kept the existing solver structure.** It already writes the selection rows directly (rows 0..58 of the next state are `t_t0` and `t_t1` written in place), calls `__rest` for rows 59..73, and computes `su_p_p0 @ p0` once per parameter update. I added `FOM_PROF_BEGIN/END` markers around the three steps.

**Considered but not done:**
- Rewriting each row as A·(t_t0 − c·state). The `state`, `t_t0` and `_d_xi` coefficients look like near-exact multiples of each other (for example −0.3148… and 0.6296…), which would cut the multiply count by about half. They differ in the last digits, though, so this would change the effective coefficients by about 1e-16 relative. I rejected it as not exact; no rule blocked it.

Blocked by rules:
- Inlining the per-iteration products directly into the solver loop of `src/fom_solver.c`, removing the call boundaries | src/fom_solver.c must not read matrix data or write matrix-vector loops/literals | small (LTO already inlines most of it), ~2-5%
- Factoring the near-proportional `state`/`t_t0`/`_d_xi` coefficient pairs into one product with a derived scalar | exactness of the coefficients each member applies (literal values must be exactly the matrix's), and no new float constants | ~30-40% of the step time

### Round 2

Initial vs final: 18.380 us -> 18.960 us mean_us per solve (no change; the difference is run-to-run noise). The speedup over the translated solver stays 2.49x (45.685 us). The sources are unchanged this round.

Final profile (`make profile`, hottest first; profiling adds overhead, so the profiled solve is 31.9 us):
- step_su_rest   33921 us   35.0%
- step_t0        33721 us   34.8%
- step_t1        29184 us   30.1%

Optimizations applied this round: none. The previous rounds' kernels are already fully unrolled over the fixed sparsity, with the coefficients inlined. They skip zero and unit coefficients and fold the bias, activation and selection-row writes straight into the next state. The steps are inlined through LTO. One iteration costs about 95 ns for about 650 multiply-adds with no dependencies between rows, so it is close to the FMA throughput bound. Merging the steps into `fom_iterate` gains nothing, because LTO already inlines them. Splitting each row into more accumulators would shorten the dependency chains, but out-of-order execution across rows already hides that latency.

Blocked by rules:
- Exploit the near-relations between member coefficients (xi-matrices ~= 2x the base ones, t_t0 coefficient ~= -2x the state coefficient). Each row of t1 and su would then collapse to c_j*(2*t0_j - s_j) + xi*2c_j*(t0_j - s_j), cutting the work about in half | the relations hold only to the last few bits, so the result would not apply exactly the package's coefficients ("values must be exactly the matrix's") | ~30-45%
```
