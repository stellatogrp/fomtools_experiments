# Optimization report

## Timing

```
translated (emitted)      63.3 us/solve
optimized                 13.1 us/solve
speedup                  4.83x
```

Optimization outcome: **kept (re-verified correct)**.

## Rounds

| round | before (us) | after (us) | gain | kept | agent (s) | outcome |
|---:|---:|---:|---:|:---:|---:|---|
| 1 | 63.290 | 13.104 | 79.3% | yes | 100 | continued |
| 2 | - | - | - | no | 62 | the round left the sources unchanged |

## Kernel checks

Each `fom_apply_<name>` in the shipped `fom_ops.c`, checked by `make verify` against the package's reference over the same `fom_data.c` matrix. *basis* probes every unit vector, which recovers the routine's matrix column by column (an exact check), plus random vectors; *random* uses random vectors only (the matrix is over the probe budget). A `fused_<block>` row is a `fom_fused_<block>`, checked against the sum of its members: *basis* there probes every unit vector of each source with every schedule value at 0 and with each alone at 1, which recovers every member exactly.

| matrix | shape | nnz | rewritten | check | probes | max error |
|---|---|---|---|---|---|---|
| `t0_state` | 49x74 | 147 | no | basis | 76 | 0.000e+00 |
| `t1_state` | 10x74 | 59 | no | basis | 76 | 0.000e+00 |
| `t1_state__d_xi` | 10x74 | 49 | no | basis | 76 | 0.000e+00 |
| `t1_t_t0` | 10x49 | 49 | no | basis | 51 | 0.000e+00 |
| `t1_t_t0__d_xi` | 10x49 | 49 | no | basis | 51 | 0.000e+00 |
| `su_state` | 74x74 | 64 | no | basis | 76 | 0.000e+00 |
| `su_state__d_xi` | 74x74 | 49 | no | basis | 76 | 0.000e+00 |
| `su_t_t0` | 74x49 | 98 | no | basis | 51 | 0.000e+00 |
| `su_t_t0__d_xi` | 74x49 | 49 | no | basis | 51 | 0.000e+00 |
| `su_t_t1` | 74x10 | 10 | no | basis | 12 | 0.000e+00 |
| `su_p_p0` | 74x15 | 15 | no | basis | 17 | 0.000e+00 |
| `fused_t1` | 10x123 | 206 | no | basis | 248 | 0.000e+00 |
| `fused_su` | 74x133 | 270 | no | basis | 268 | 0.000e+00 |
| `step_t0` | 49x74 | 147 | no | basis | 80 | 0.000e+00 |
| `step_t1` | 10x123 | 206 | no | basis | 252 | 0.000e+00 |
| `step_su` | 74x133 | 270 | no | basis | 268 | 0.000e+00 |
| `fused_su__rest` | 74x133 | 270 | no | basis | 268 | 0.000e+00 |
| `step_su__rest` | 74x133 | 270 | no | basis | 268 | 0.000e+00 |
| `iterate` | 74x74 | 623 | yes | basis | 228 | 1.776e-15 |

### Rule violations

_None: the collected rewrite passed the rules and was kept._

## Blocked by rules

Optimizations the agent considered but did not make because a rule forbade them, in its own words.

| round | idea | rule | expected gain |
|---|---|---|---|
| 1 | Fusing two consecutive iterations, or keeping state in registers across iterations by unrolling the outer loop | `fom_iterate` must compute exactly one iteration, and `src/fom_solver.c` may not contain matrix products or coefficient literals | unknown, likely around 10% (saves the copy-out and reload of 74+59 values per iteration) |
| 1 | Declaring `restrict` on the `fom_iterate` arguments so the compiler can drop the local copy-out buffers | headers and prototypes in `include/` are fixed | about 1 µs (removes about 133 stores per iteration) |
| 1 | Hardcoding the zero entries of `bias_su` on the selection rows, since `su_p_p0` only touches rows 59..73 | `bias` is generic and must be honoured for any input, and `fom_ops.c` may not read the constant or bias data | under 2% |
| 2 | Merge every t1/su row's 4 coefficient groups into one product on (a0 - x), since the a0 coefficients are about -2x the x coefficients and the xi coefficients about 2x apart, cutting flops about 2x | coefficients must exactly match the package data (they differ in the last digits), so the near-equal ones cannot be merged | ~20-30% of iterate |
| 2 | Precompute per-iteration combined coefficients (c + xi*e) for all 200 iterations at setup | `src/fom_ops.c` may not read the schedule, and `fom_setup` cannot probe matrices | ~30% |

## What the agent changed

```diff
--- src/fom_solver.c (translated)
+++ src/fom_solver.c (optimized)
@@ -2,6 +2,7 @@
 #include "fom_data.h"
 #include "fom_ops.h"
 #include "fom_linsolve.h"
+#include "fom_profile.h"
 
 void fom_set_default_settings(FomSettings *settings) {
     settings->max_iter = FOM_MAX_ITER;
@@ -48,15 +49,19 @@
     for (it = 0; it < max_iter; ++it) {
         fom_float *tmp;
         const fom_float xi = fom_sched_xi(it);
+        FOM_PROF_BEGIN(iterate);
         fom_iterate(x, xi, FOM_MAT_t0_c, FOM_MAT_t1_c, work->acc,
                     &FOM_ACT_t0, &FOM_ACT_t1, work->t0, work->t1, xn);
+        FOM_PROF_END(iterate);
         if (it == max_iter - 1) {
+            FOM_PROF_BEGIN(residual);
             res = 0.0;
             for (i = 0; i < FOM_STATE_DIM; ++i) {
                 fom_float d = xn[i] - x[i];
                 if (d < 0) d = -d;
                 if (d > res) res = d;
             }
+            FOM_PROF_END(residual);
         }
         tmp = x; x = xn; xn = tmp;
     }
--- src/fom_ops.c (translated)
+++ src/fom_ops.c (optimized)
@@ -253,7 +253,310 @@
 }
 
 void fom_iterate(const fom_float *state, fom_float sc_xi, const fom_float *bias_t0, const fom_float *bias_t1, const fom_float *bias_su, const FomAct_t0 *act_t0, const FomAct_t1 *act_t1, fom_float *t_t0, fom_float *t_t1, fom_float *new_state) {
-    fom_step_t0(state, bias_t0, act_t0, t_t0);
-    fom_step_t1(state, t_t0, sc_xi, bias_t1, act_t1, t_t1);
-    fom_step_su(state, t_t0, t_t1, sc_xi, bias_su, new_state);
-}
+    fom_float v, a0[49], a1[10], ns[74];
+    fom_int i;
+    const fom_float *x = state;
+    v = bias_t0[0] + x[0] + -0.3148342557928728 * x[49] + -0.1255906893098709 * x[59];
+    if (v < act_t0->lo[0]) v = act_t0->lo[0];
+    if (v > act_t0->hi[0]) v = act_t0->hi[0];
+    a0[0] = v;
+    v = bias_t0[1] + x[1] + -0.1785106319392791 * x[49] + -0.31483425579287283 * x[60];
+    if (v < act_t0->lo[1]) v = act_t0->lo[1];
+    if (v > act_t0->hi[1]) v = act_t0->hi[1];
+    a0[1] = v;
+    v = bias_t0[2] + x[2] + -0.3148342556510004 * x[49] + -0.18736322506593048 * x[62];
+    if (v < act_t0->lo[2]) v = act_t0->lo[2];
+    if (v > act_t0->hi[2]) v = act_t0->hi[2];
+    a0[2] = v;
+    v = bias_t0[3] + x[3] + -0.3148342557928728 * x[49] + -0.05210541922399308 * x[65];
+    if (v < act_t0->lo[3]) v = act_t0->lo[3];
+    if (v > act_t0->hi[3]) v = act_t0->hi[3];
+    a0[3] = v;
+    v = bias_t0[4] + x[4] + -0.3148342557928728 * x[49] + -0.04219712274031408 * x[71];
+    if (v < act_t0->lo[4]) v = act_t0->lo[4];
+    if (v > act_t0->hi[4]) v = act_t0->hi[4];
+    a0[4] = v;
+    v = bias_t0[5] + x[5] + -0.3148342557928728 * x[50] + -0.08155612991849112 * x[60];
+    if (v < act_t0->lo[5]) v = act_t0->lo[5];
+    if (v > act_t0->hi[5]) v = act_t0->hi[5];
+    a0[5] = v;
+    v = bias_t0[6] + x[6] + -0.1149099042416003 * x[50] + -0.31483425579287283 * x[61];
+    if (v < act_t0->lo[6]) v = act_t0->lo[6];
+    if (v > act_t0->hi[6]) v = act_t0->hi[6];
+    a0[6] = v;
+    v = bias_t0[7] + x[7] + -0.11245022456106747 * x[50] + -0.3148342557928728 * x[64];
+    if (v < act_t0->lo[7]) v = act_t0->lo[7];
+    if (v > act_t0->hi[7]) v = act_t0->hi[7];
+    a0[7] = v;
+    v = bias_t0[8] + x[8] + -0.3148342557928728 * x[50] + -0.07906843296231456 * x[65];
+    if (v < act_t0->lo[8]) v = act_t0->lo[8];
+    if (v > act_t0->hi[8]) v = act_t0->hi[8];
+    a0[8] = v;
+    v = bias_t0[9] + x[9] + -0.1012882957183817 * x[50] + -0.3148342557928728 * x[66];
+    if (v < act_t0->lo[9]) v = act_t0->lo[9];
+    if (v > act_t0->hi[9]) v = act_t0->hi[9];
+    a0[9] = v;
+    v = bias_t0[10] + x[10] + -0.3148342557928728 * x[50] + -0.024432609086831557 * x[69];
+    if (v < act_t0->lo[10]) v = act_t0->lo[10];
+    if (v > act_t0->hi[10]) v = act_t0->hi[10];
+    a0[10] = v;
+    v = bias_t0[11] + x[11] + -0.31483425576776075 * x[50] + -0.14375068353499568 * x[73];
+    if (v < act_t0->lo[11]) v = act_t0->lo[11];
+    if (v > act_t0->hi[11]) v = act_t0->hi[11];
+    a0[11] = v;
+    v = bias_t0[12] + x[12] + -0.16283459527310534 * x[51] + -0.3148342557655042 * x[59];
+    if (v < act_t0->lo[12]) v = act_t0->lo[12];
+    if (v > act_t0->hi[12]) v = act_t0->hi[12];
+    a0[12] = v;
+    v = bias_t0[13] + x[13] + -0.3148342557928728 * x[51] + -0.057878934426039565 * x[63];
+    if (v < act_t0->lo[13]) v = act_t0->lo[13];
+    if (v > act_t0->hi[13]) v = act_t0->hi[13];
+    a0[13] = v;
+    v = bias_t0[14] + x[14] + -0.3148342557928728 * x[51] + -0.020371967614988162 * x[65];
+    if (v < act_t0->lo[14]) v = act_t0->lo[14];
+    if (v > act_t0->hi[14]) v = act_t0->hi[14];
+    a0[14] = v;
+    v = bias_t0[15] + x[15] + -0.13195167489181564 * x[51] + -0.31483425579287283 * x[73];
+    if (v < act_t0->lo[15]) v = act_t0->lo[15];
+    if (v > act_t0->hi[15]) v = act_t0->hi[15];
+    a0[15] = v;
+    v = bias_t0[16] + x[16] + -0.20484874346487622 * x[52] + -0.31483425569820117 * x[59];
+    if (v < act_t0->lo[16]) v = act_t0->lo[16];
+    if (v > act_t0->hi[16]) v = act_t0->hi[16];
+    a0[16] = v;
+    v = bias_t0[17] + x[17] + -0.3148342557928728 * x[52] + -0.059210692177281046 * x[61];
+    if (v < act_t0->lo[17]) v = act_t0->lo[17];
+    if (v > act_t0->hi[17]) v = act_t0->hi[17];
+    a0[17] = v;
+    v = bias_t0[18] + x[18] + -0.3148342557928728 * x[52] + -0.04186021818779605 * x[63];
+    if (v < act_t0->lo[18]) v = act_t0->lo[18];
+    if (v > act_t0->hi[18]) v = act_t0->hi[18];
+    a0[18] = v;
+    v = bias_t0[19] + x[19] + -0.2013575399297713 * x[52] + -0.3148342556220557 * x[64];
+    if (v < act_t0->lo[19]) v = act_t0->lo[19];
+    if (v > act_t0->hi[19]) v = act_t0->hi[19];
+    a0[19] = v;
+    v = bias_t0[20] + x[20] + -0.1717002983294537 * x[52] + -0.31483425579287283 * x[65];
+    if (v < act_t0->lo[20]) v = act_t0->lo[20];
+    if (v > act_t0->hi[20]) v = act_t0->hi[20];
+    a0[20] = v;
+    v = bias_t0[21] + x[21] + -0.3148342557928728 * x[52] + -0.09586520152267977 * x[70];
+    if (v < act_t0->lo[21]) v = act_t0->lo[21];
+    if (v > act_t0->hi[21]) v = act_t0->hi[21];
+    a0[21] = v;
+    v = bias_t0[22] + x[22] + -0.3148342557928728 * x[52] + -0.03542869495158811 * x[73];
+    if (v < act_t0->lo[22]) v = act_t0->lo[22];
+    if (v > act_t0->hi[22]) v = act_t0->hi[22];
+    a0[22] = v;
+    v = bias_t0[23] + x[23] + -0.11549111655165208 * x[53] + -0.3148342557928728 * x[62];
+    if (v < act_t0->lo[23]) v = act_t0->lo[23];
+    if (v > act_t0->hi[23]) v = act_t0->hi[23];
+    a0[23] = v;
+    v = bias_t0[24] + x[24] + -0.3148342557928728 * x[53] + -0.023238864598874573 * x[68];
+    if (v < act_t0->lo[24]) v = act_t0->lo[24];
+    if (v > act_t0->hi[24]) v = act_t0->hi[24];
+    a0[24] = v;
+    v = bias_t0[25] + x[25] + -0.22703157183385472 * x[53] + -0.31483425579287283 * x[70];
+    if (v < act_t0->lo[25]) v = act_t0->lo[25];
+    if (v > act_t0->hi[25]) v = act_t0->hi[25];
+    a0[25] = v;
+    v = bias_t0[26] + x[26] + -0.3148342557928728 * x[54] + -0.06685962708718948 * x[59];
+    if (v < act_t0->lo[26]) v = act_t0->lo[26];
+    if (v > act_t0->hi[26]) v = act_t0->hi[26];
+    a0[26] = v;
+    v = bias_t0[27] + x[27] + -0.31483425578928326 * x[54] + -0.1807094243289255 * x[60];
+    if (v < act_t0->lo[27]) v = act_t0->lo[27];
+    if (v > act_t0->hi[27]) v = act_t0->hi[27];
+    a0[27] = v;
+    v = bias_t0[28] + x[28] + -0.11812566027056481 * x[54] + -0.31483425579287283 * x[63];
+    if (v < act_t0->lo[28]) v = act_t0->lo[28];
+    if (v > act_t0->hi[28]) v = act_t0->hi[28];
+    a0[28] = v;
+    v = bias_t0[29] + x[29] + -0.3148342557928728 * x[54] + -0.06983942457332966 * x[64];
+    if (v < act_t0->lo[29]) v = act_t0->lo[29];
+    if (v > act_t0->hi[29]) v = act_t0->hi[29];
+    a0[29] = v;
+    v = bias_t0[30] + x[30] + -0.3148342557928728 * x[54] + -0.01829049354312028 * x[66];
+    if (v < act_t0->lo[30]) v = act_t0->lo[30];
+    if (v > act_t0->hi[30]) v = act_t0->hi[30];
+    a0[30] = v;
+    v = bias_t0[31] + x[31] + -0.11621549871906076 * x[54] + -0.3148342557928728 * x[69];
+    if (v < act_t0->lo[31]) v = act_t0->lo[31];
+    if (v > act_t0->hi[31]) v = act_t0->hi[31];
+    a0[31] = v;
+    v = bias_t0[32] + x[32] + -0.3148342557928728 * x[54] + -0.04147423462601573 * x[71];
+    if (v < act_t0->lo[32]) v = act_t0->lo[32];
+    if (v > act_t0->hi[32]) v = act_t0->hi[32];
+    a0[32] = v;
+    v = bias_t0[33] + x[33] + -0.3148342557928728 * x[54] + -0.029904608409446035 * x[72];
+    if (v < act_t0->lo[33]) v = act_t0->lo[33];
+    if (v > act_t0->hi[33]) v = act_t0->hi[33];
+    a0[33] = v;
+    v = bias_t0[34] + x[34] + -0.2007269945571799 * x[55] + -0.3148342557928728 * x[68];
+    if (v < act_t0->lo[34]) v = act_t0->lo[34];
+    if (v > act_t0->hi[34]) v = act_t0->hi[34];
+    a0[34] = v;
+    v = bias_t0[35] + x[35] + -0.31483425579287283 * x[55] + -0.26432062531400063 * x[69];
+    if (v < act_t0->lo[35]) v = act_t0->lo[35];
+    if (v > act_t0->hi[35]) v = act_t0->hi[35];
+    a0[35] = v;
+    v = bias_t0[36] + x[36] + -0.20810320529794363 * x[55] + -0.31483425575411544 * x[73];
+    if (v < act_t0->lo[36]) v = act_t0->lo[36];
+    if (v > act_t0->hi[36]) v = act_t0->hi[36];
+    a0[36] = v;
+    v = bias_t0[37] + x[37] + -0.3148342557928728 * x[56] + -0.050057390438124606 * x[60];
+    if (v < act_t0->lo[37]) v = act_t0->lo[37];
+    if (v > act_t0->hi[37]) v = act_t0->hi[37];
+    a0[37] = v;
+    v = bias_t0[38] + x[38] + -0.3148342557928728 * x[56] + -0.022355820755170213 * x[66];
+    if (v < act_t0->lo[38]) v = act_t0->lo[38];
+    if (v > act_t0->hi[38]) v = act_t0->hi[38];
+    a0[38] = v;
+    v = bias_t0[39] + x[39] + -0.14832333169428388 * x[57] + -0.3148342557928728 * x[59];
+    if (v < act_t0->lo[39]) v = act_t0->lo[39];
+    if (v > act_t0->hi[39]) v = act_t0->hi[39];
+    a0[39] = v;
+    v = bias_t0[40] + x[40] + -0.25687797426285774 * x[57] + -0.3148342557740812 * x[67];
+    if (v < act_t0->lo[40]) v = act_t0->lo[40];
+    if (v > act_t0->hi[40]) v = act_t0->hi[40];
+    a0[40] = v;
+    v = bias_t0[41] + x[41] + -0.3148342556269916 * x[57] + -0.2046259837099301 * x[69];
+    if (v < act_t0->lo[41]) v = act_t0->lo[41];
+    if (v > act_t0->hi[41]) v = act_t0->hi[41];
+    a0[41] = v;
+    v = bias_t0[42] + x[42] + -0.3148342557928728 * x[57] + -0.0669377754137904 * x[71];
+    if (v < act_t0->lo[42]) v = act_t0->lo[42];
+    if (v > act_t0->hi[42]) v = act_t0->hi[42];
+    a0[42] = v;
+    v = bias_t0[43] + x[43] + -0.31483425573999474 * x[57] + -0.2549419703939111 * x[72];
+    if (v < act_t0->lo[43]) v = act_t0->lo[43];
+    if (v > act_t0->hi[43]) v = act_t0->hi[43];
+    a0[43] = v;
+    v = bias_t0[44] + x[44] + -0.3148342557928728 * x[58] + -0.063379996679072 * x[65];
+    if (v < act_t0->lo[44]) v = act_t0->lo[44];
+    if (v > act_t0->hi[44]) v = act_t0->hi[44];
+    a0[44] = v;
+    v = bias_t0[45] + x[45] + -0.2409314392941177 * x[58] + -0.31483425579287283 * x[67];
+    if (v < act_t0->lo[45]) v = act_t0->lo[45];
+    if (v > act_t0->hi[45]) v = act_t0->hi[45];
+    a0[45] = v;
+    v = bias_t0[46] + x[46] + -0.15802369728139173 * x[58] + -0.3148342557928728 * x[71];
+    if (v < act_t0->lo[46]) v = act_t0->lo[46];
+    if (v > act_t0->hi[46]) v = act_t0->hi[46];
+    a0[46] = v;
+    v = bias_t0[47] + x[47] + -0.21287292492070603 * x[58] + -0.3148342557928728 * x[72];
+    if (v < act_t0->lo[47]) v = act_t0->lo[47];
+    if (v > act_t0->hi[47]) v = act_t0->hi[47];
+    a0[47] = v;
+    v = bias_t0[48] + x[48] + -0.2746827012680003 * x[58] + -0.3148342555778952 * x[73];
+    if (v < act_t0->lo[48]) v = act_t0->lo[48];
+    if (v > act_t0->hi[48]) v = act_t0->hi[48];
+    a0[48] = v;
+    v = bias_t1[0] + -0.31483425579287283 * x[0] + -0.17851063193927913 * x[1] + -0.3148342556510004 * x[2] + -0.31483425579287283 * x[3] + -0.31483425579287283 * x[4] + x[49] + 0.6296685115857455 * a0[0] + 0.35702126387855826 * a0[1] + 0.6296685113020011 * a0[2] + 0.6296685115857455 * a0[3] + 0.6296685115857455 * a0[4] + sc_xi * (0.0 + -0.6296685115857453 * x[0] + -0.3570212638785581 * x[1] + -0.6296685113020006 * x[2] + -0.6296685115857453 * x[3] + -0.6296685115857453 * x[4] + 0.6296685115857453 * a0[0] + 0.3570212638785581 * a0[1] + 0.6296685113020003 * a0[2] + 0.6296685115857453 * a0[3] + 0.6296685115857453 * a0[4]);
+    if (v < act_t1->lo[0]) v = act_t1->lo[0];
+    a1[0] = v;
+    v = bias_t1[1] + -0.31483425579287283 * x[5] + -0.11490990424160037 * x[6] + -0.11245022456106753 * x[7] + -0.31483425579287283 * x[8] + -0.10128829571838176 * x[9] + -0.31483425579287283 * x[10] + -0.3148342557677609 * x[11] + x[50] + 0.6296685115857455 * a0[5] + 0.22981980848320072 * a0[6] + 0.22490044912213497 * a0[7] + 0.6296685115857455 * a0[8] + 0.20257659143676343 * a0[9] + 0.6296685115857455 * a0[10] + 0.6296685115355216 * a0[11] + sc_xi * (0.0 + -0.6296685115857453 * x[5] + -0.2298198084832004 * x[6] + -0.22490044912213478 * x[7] + -0.6296685115857453 * x[8] + -0.2025765914367633 * x[9] + -0.6296685115857453 * x[10] + -0.6296685115355208 * x[11] + 0.6296685115857453 * a0[5] + 0.22981980848320033 * a0[6] + 0.22490044912213478 * a0[7] + 0.6296685115857453 * a0[8] + 0.2025765914367634 * a0[9] + 0.6296685115857453 * a0[10] + 0.6296685115355208 * a0[11]);
+    if (v < act_t1->lo[1]) v = act_t1->lo[1];
+    a1[1] = v;
+    v = bias_t1[2] + -0.16283459527310534 * x[12] + -0.31483425579287283 * x[13] + -0.31483425579287283 * x[14] + -0.1319516748918157 * x[15] + x[51] + 0.3256691905462108 * a0[12] + 0.6296685115857455 * a0[13] + 0.6296685115857455 * a0[14] + 0.2639033497836314 * a0[15] + sc_xi * (0.0 + -0.32566919054621074 * x[12] + -0.6296685115857453 * x[13] + -0.6296685115857453 * x[14] + -0.26390334978363106 * x[15] + 0.3256691905462105 * a0[12] + 0.6296685115857453 * a0[13] + 0.6296685115857453 * a0[14] + 0.26390334978363095 * a0[15]);
+    if (v < act_t1->lo[2]) v = act_t1->lo[2];
+    a1[2] = v;
+    v = bias_t1[3] + -0.2048487434648763 * x[16] + -0.31483425579287283 * x[17] + -0.31483425579287283 * x[18] + -0.20135753992977132 * x[19] + -0.17170029832945377 * x[20] + -0.31483425579287283 * x[21] + -0.31483425579287283 * x[22] + x[52] + 0.4096974869297525 * a0[16] + 0.6296685115857455 * a0[17] + 0.6296685115857455 * a0[18] + 0.4027150798595428 * a0[19] + 0.3434005966589075 * a0[20] + 0.6296685115857455 * a0[21] + 0.6296685115857455 * a0[22] + sc_xi * (0.0 + -0.40969748692975216 * x[16] + -0.6296685115857453 * x[17] + -0.6296685115857453 * x[18] + -0.40271507985954247 * x[19] + -0.34340059665890715 * x[20] + -0.6296685115857453 * x[21] + -0.6296685115857453 * x[22] + 0.4096974869297524 * a0[16] + 0.6296685115857453 * a0[17] + 0.6296685115857453 * a0[18] + 0.402715079859542 * a0[19] + 0.34340059665890715 * a0[20] + 0.6296685115857453 * a0[21] + 0.6296685115857453 * a0[22]);
+    if (v < act_t1->lo[3]) v = act_t1->lo[3];
+    a1[3] = v;
+    v = bias_t1[4] + -0.11549111655165209 * x[23] + -0.31483425579287283 * x[24] + -0.2270315718338548 * x[25] + x[53] + 0.2309822331033042 * a0[23] + 0.6296685115857455 * a0[24] + 0.4540631436677096 * a0[25] + sc_xi * (0.0 + -0.23098223310330407 * x[23] + -0.6296685115857453 * x[24] + -0.45406314366770917 * x[25] + 0.23098223310330407 * a0[23] + 0.6296685115857453 * a0[24] + 0.45406314366770895 * a0[25]);
+    if (v < act_t1->lo[4]) v = act_t1->lo[4];
+    a1[4] = v;
+    v = bias_t1[5] + -0.31483425579287283 * x[26] + -0.3148342557892835 * x[27] + -0.11812566027056481 * x[28] + -0.31483425579287283 * x[29] + -0.31483425579287283 * x[30] + -0.11621549871906077 * x[31] + -0.31483425579287283 * x[32] + -0.31483425579287283 * x[33] + x[54] + 0.6296685115857455 * a0[26] + 0.6296685115785666 * a0[27] + 0.23625132054112963 * a0[28] + 0.6296685115857455 * a0[29] + 0.6296685115857455 * a0[30] + 0.23243099743812146 * a0[31] + 0.6296685115857455 * a0[32] + 0.6296685115857455 * a0[33] + sc_xi * (0.0 + -0.6296685115857453 * x[26] + -0.6296685115785658 * x[27] + -0.23625132054112963 * x[28] + -0.6296685115857453 * x[29] + -0.6296685115857453 * x[30] + -0.2324309974381214 * x[31] + -0.6296685115857453 * x[32] + -0.6296685115857453 * x[33] + 0.6296685115857453 * a0[26] + 0.6296685115785663 * a0[27] + 0.23625132054112963 * a0[28] + 0.6296685115857453 * a0[29] + 0.6296685115857453 * a0[30] + 0.2324309974381216 * a0[31] + 0.6296685115857453 * a0[32] + 0.6296685115857453 * a0[33]);
+    if (v < act_t1->lo[5]) v = act_t1->lo[5];
+    a1[5] = v;
+    v = bias_t1[6] + -0.2007269945571799 * x[34] + -0.31483425579287294 * x[35] + -0.20810320529794374 * x[36] + x[55] + 0.4014539891143599 * a0[34] + 0.6296685115857457 * a0[35] + 0.41620641059588726 * a0[36] + sc_xi * (0.0 + -0.40145398911435987 * x[34] + -0.6296685115857453 * x[35] + -0.41620641059588687 * x[36] + 0.40145398911435964 * a0[34] + 0.6296685115857458 * a0[35] + 0.4162064105958871 * a0[36]);
+    if (v < act_t1->lo[6]) v = act_t1->lo[6];
+    a1[6] = v;
+    v = bias_t1[7] + -0.31483425579287283 * x[37] + -0.31483425579287283 * x[38] + x[56] + 0.6296685115857455 * a0[37] + 0.6296685115857455 * a0[38] + sc_xi * (0.0 + -0.6296685115857453 * x[37] + -0.6296685115857453 * x[38] + 0.6296685115857453 * a0[37] + 0.6296685115857453 * a0[38]);
+    if (v < act_t1->lo[7]) v = act_t1->lo[7];
+    a1[7] = v;
+    v = bias_t1[8] + -0.14832333169428388 * x[39] + -0.25687797426285774 * x[40] + -0.31483425562699185 * x[41] + -0.31483425579287283 * x[42] + -0.3148342557399948 * x[43] + x[57] + 0.29664666338856793 * a0[39] + 0.5137559485257156 * a0[40] + 0.6296685112539833 * a0[41] + 0.6296685115857455 * a0[42] + 0.6296685114799897 * a0[43] + sc_xi * (0.0 + -0.2966466633885678 * x[39] + -0.5137559485257154 * x[40] + -0.6296685112539826 * x[41] + -0.6296685115857453 * x[42] + -0.629668511479989 * x[43] + 0.2966466633885674 * a0[39] + 0.5137559485257152 * a0[40] + 0.629668511253983 * a0[41] + 0.6296685115857453 * a0[42] + 0.6296685114799887 * a0[43]);
+    if (v < act_t1->lo[8]) v = act_t1->lo[8];
+    a1[8] = v;
+    v = bias_t1[9] + -0.31483425579287283 * x[44] + -0.2409314392941178 * x[45] + -0.15802369728139182 * x[46] + -0.21287292492070606 * x[47] + -0.2746827012680003 * x[48] + x[58] + 0.6296685115857455 * a0[44] + 0.48186287858823557 * a0[45] + 0.3160473945627835 * a0[46] + 0.42574584984141206 * a0[47] + 0.5493654025360007 * a0[48] + sc_xi * (0.0 + -0.6296685115857453 * x[44] + -0.481862878588235 * x[45] + -0.31604739456278325 * x[46] + -0.425745849841412 * x[47] + -0.5493654025360006 * x[48] + 0.6296685115857453 * a0[44] + 0.481862878588235 * a0[45] + 0.31604739456278325 * a0[46] + 0.425745849841412 * a0[47] + 0.5493654025360004 * a0[48]);
+    if (v < act_t1->lo[9]) v = act_t1->lo[9];
+    a1[9] = v;
+    ns[0] = bias_su[0] + a0[0];
+    ns[1] = bias_su[1] + a0[1];
+    ns[2] = bias_su[2] + a0[2];
+    ns[3] = bias_su[3] + a0[3];
+    ns[4] = bias_su[4] + a0[4];
+    ns[5] = bias_su[5] + a0[5];
+    ns[6] = bias_su[6] + a0[6];
+    ns[7] = bias_su[7] + a0[7];
+    ns[8] = bias_su[8] + a0[8];
+    ns[9] = bias_su[9] + a0[9];
+    ns[10] = bias_su[10] + a0[10];
+    ns[11] = bias_su[11] + a0[11];
+    ns[12] = bias_su[12] + a0[12];
+    ns[13] = bias_su[13] + a0[13];
+    ns[14] = bias_su[14] + a0[14];
+    ns[15] = bias_su[15] + a0[15];
+    ns[16] = bias_su[16] + a0[16];
+    ns[17] = bias_su[17] + a0[17];
+    ns[18] = bias_su[18] + a0[18];
+    ns[19] = bias_su[19] + a0[19];
+    ns[20] = bias_su[20] + a0[20];
+    ns[21] = bias_su[21] + a0[21];
+    ns[22] = bias_su[22] + a0[22];
+    ns[23] = bias_su[23] + a0[23];
+    ns[24] = bias_su[24] + a0[24];
+    ns[25] = bias_su[25] + a0[25];
+    ns[26] = bias_su[26] + a0[26];
+    ns[27] = bias_su[27] + a0[27];
+    ns[28] = bias_su[28] + a0[28];
+    ns[29] = bias_su[29] + a0[29];
+    ns[30] = bias_su[30] + a0[30];
+    ns[31] = bias_su[31] + a0[31];
+    ns[32] = bias_su[32] + a0[32];
+    ns[33] = bias_su[33] + a0[33];
+    ns[34] = bias_su[34] + a0[34];
+    ns[35] = bias_su[35] + a0[35];
+    ns[36] = bias_su[36] + a0[36];
+    ns[37] = bias_su[37] + a0[37];
+    ns[38] = bias_su[38] + a0[38];
+    ns[39] = bias_su[39] + a0[39];
+    ns[40] = bias_su[40] + a0[40];
+    ns[41] = bias_su[41] + a0[41];
+    ns[42] = bias_su[42] + a0[42];
+    ns[43] = bias_su[43] + a0[43];
+    ns[44] = bias_su[44] + a0[44];
+    ns[45] = bias_su[45] + a0[45];
+    ns[46] = bias_su[46] + a0[46];
+    ns[47] = bias_su[47] + a0[47];
+    ns[48] = bias_su[48] + a0[48];
+    ns[49] = bias_su[49] + a1[0];
+    ns[50] = bias_su[50] + a1[1];
+    ns[51] = bias_su[51] + a1[2];
+    ns[52] = bias_su[52] + a1[3];
+    ns[53] = bias_su[53] + a1[4];
+    ns[54] = bias_su[54] + a1[5];
+    ns[55] = bias_su[55] + a1[6];
+    ns[56] = bias_su[56] + a1[7];
+    ns[57] = bias_su[57] + a1[8];
+    ns[58] = bias_su[58] + a1[9];
+    ns[59] = bias_su[59] + -0.12559068930987088 * x[0] + -0.31483425576550444 * x[12] + -0.3148342556982012 * x[16] + -0.0668596270871895 * x[26] + -0.31483425579287283 * x[39] + x[59] + 0.2511813786197419 * a0[0] + 0.6296685115310086 * a0[12] + 0.6296685113964026 * a0[16] + 0.13371925417437902 * a0[26] + 0.6296685115857455 * a0[39] + sc_xi * (0.0 + -0.25118137861974177 * x[0] + -0.6296685115310079 * x[12] + -0.6296685113964021 * x[16] + -0.1337192541743789 * x[26] + -0.6296685115857453 * x[39] + 0.25118137861974166 * a0[0] + 0.6296685115310079 * a0[12] + 0.6296685113964017 * a0[16] + 0.13371925417437883 * a0[26] + 0.6296685115857453 * a0[39]);
+    ns[60] = bias_su[60] + -0.31483425579287294 * x[1] + -0.08155612991849118 * x[5] + -0.18070942432892556 * x[27] + -0.05005739043812461 * x[37] + x[60] + 0.6296685115857457 * a0[1] + 0.16311225983698227 * a0[5] + 0.36141884865785107 * a0[27] + 0.1001147808762492 * a0[37] + sc_xi * (0.0 + -0.6296685115857453 * x[1] + -0.16311225983698213 * x[5] + -0.361418848657851 * x[27] + -0.10011478087624918 * x[37] + 0.6296685115857458 * a0[1] + 0.16311225983698224 * a0[5] + 0.361418848657851 * a0[27] + 0.10011478087624924 * a0[37]);
+    ns[61] = bias_su[61] + -0.31483425579287294 * x[6] + -0.05921069217728105 * x[17] + x[61] + 0.6296685115857457 * a0[6] + 0.11842138435456212 * a0[17] + sc_xi * (0.0 + -0.6296685115857453 * x[6] + -0.11842138435456206 * x[17] + 0.6296685115857458 * a0[6] + 0.11842138435456201 * a0[17]);
+    ns[62] = bias_su[62] + -0.18736322506593045 * x[2] + -0.31483425579287283 * x[23] + x[62] + 0.3747264501318611 * a0[2] + 0.6296685115857455 * a0[23] + sc_xi * (0.0 + -0.37472645013186096 * x[2] + -0.6296685115857453 * x[23] + 0.3747264501318605 * a0[2] + 0.6296685115857453 * a0[23]);
+    ns[63] = bias_su[63] + -0.05787893442603958 * x[13] + -0.04186021818779605 * x[18] + -0.31483425579287294 * x[28] + x[63] + 0.11575786885207912 * a0[13] + 0.08372043637559212 * a0[18] + 0.6296685115857457 * a0[28] + sc_xi * (0.0 + -0.11575786885207907 * x[13] + -0.08372043637559205 * x[18] + -0.6296685115857453 * x[28] + 0.11575786885207907 * a0[13] + 0.083720436375592 * a0[18] + 0.6296685115857458 * a0[28]);
+    ns[64] = bias_su[64] + -0.31483425579287283 * x[7] + -0.31483425562205586 * x[19] + -0.06983942457332967 * x[29] + x[64] + 0.6296685115857455 * a0[7] + 0.6296685112441113 * a0[19] + 0.13967884914665932 * a0[29] + sc_xi * (0.0 + -0.6296685115857453 * x[7] + -0.6296685112441109 * x[19] + -0.13967884914665926 * x[29] + 0.6296685115857453 * a0[7] + 0.6296685112441114 * a0[19] + 0.13967884914665926 * a0[29]);
+    ns[65] = bias_su[65] + -0.05210541922399309 * x[3] + -0.07906843296231458 * x[8] + -0.02037196761498816 * x[14] + -0.31483425579287294 * x[20] + -0.06337999667907203 * x[44] + x[65] + 0.10421083844798616 * a0[3] + 0.1581368659246291 * a0[8] + 0.04074393522997634 * a0[14] + 0.6296685115857457 * a0[20] + 0.12675999335814403 * a0[44] + sc_xi * (0.0 + -0.10421083844798612 * x[3] + -0.15813686592462906 * x[8] + -0.040743935229976325 * x[14] + -0.6296685115857453 * x[20] + -0.12675999335814395 * x[44] + 0.10421083844798612 * a0[3] + 0.15813686592462906 * a0[8] + 0.040743935229976304 * a0[14] + 0.6296685115857458 * a0[20] + 0.12675999335814397 * a0[44]);
+    ns[66] = bias_su[66] + -0.31483425579287283 * x[9] + -0.01829049354312029 * x[30] + -0.022355820755170213 * x[38] + x[66] + 0.6296685115857455 * a0[9] + 0.03658098708624057 * a0[30] + 0.04471164151034043 * a0[38] + sc_xi * (0.0 + -0.6296685115857453 * x[9] + -0.03658098708624052 * x[30] + -0.04471164151034042 * x[38] + 0.6296685115857453 * a0[9] + 0.03658098708624052 * a0[30] + 0.0447116415103404 * a0[38]);
+    ns[67] = bias_su[67] + -0.3148342557740813 * x[40] + -0.31483425579287294 * x[45] + x[67] + 0.6296685115481624 * a0[40] + 0.6296685115857457 * a0[45] + sc_xi * (0.0 + -0.629668511548162 * x[40] + -0.6296685115857453 * x[45] + 0.6296685115481624 * a0[40] + 0.6296685115857458 * a0[45]);
+    ns[68] = bias_su[68] + -0.02323886459887458 * x[24] + -0.31483425579287283 * x[34] + x[68] + 0.04647772919774916 * a0[24] + 0.6296685115857455 * a0[34] + sc_xi * (0.0 + -0.04647772919774914 * x[24] + -0.6296685115857453 * x[34] + 0.04647772919774911 * a0[24] + 0.6296685115857453 * a0[34]);
+    ns[69] = bias_su[69] + -0.024432609086831564 * x[10] + -0.31483425579287283 * x[31] + -0.26432062531400075 * x[35] + -0.20462598370993018 * x[41] + x[69] + 0.04886521817366314 * a0[10] + 0.6296685115857455 * a0[31] + 0.5286412506280014 * a0[35] + 0.40925196741986025 * a0[41] + sc_xi * (0.0 + -0.0488652181736631 * x[10] + -0.6296685115857453 * x[31] + -0.5286412506280009 * x[35] + -0.4092519674198599 * x[41] + 0.04886521817366307 * a0[10] + 0.6296685115857453 * a0[31] + 0.5286412506280012 * a0[35] + 0.40925196741986014 * a0[41]);
+    ns[70] = bias_su[70] + -0.09586520152267985 * x[21] + -0.31483425579287294 * x[25] + x[70] + 0.1917304030453596 * a0[21] + 0.6296685115857457 * a0[25] + sc_xi * (0.0 + -0.19173040304535932 * x[21] + -0.6296685115857453 * x[25] + 0.19173040304535943 * a0[21] + 0.6296685115857458 * a0[25]);
+    ns[71] = bias_su[71] + -0.042197122740314105 * x[4] + -0.04147423462601575 * x[32] + -0.0669377754137904 * x[42] + -0.31483425579287283 * x[46] + x[71] + 0.08439424548062816 * a0[4] + 0.08294846925203149 * a0[32] + 0.1338755508275808 * a0[42] + 0.6296685115857455 * a0[46] + sc_xi * (0.0 + -0.08439424548062809 * x[4] + -0.08294846925203142 * x[32] + -0.1338755508275808 * x[42] + -0.6296685115857453 * x[46] + 0.08439424548062814 * a0[4] + 0.08294846925203136 * a0[32] + 0.1338755508275808 * a0[42] + 0.6296685115857453 * a0[46]);
+    ns[72] = bias_su[72] + -0.02990460840944605 * x[33] + -0.25494197039391125 * x[43] + -0.31483425579287283 * x[47] + x[72] + 0.05980921681889208 * a0[33] + 0.5098839407878222 * a0[43] + 0.6296685115857455 * a0[47] + sc_xi * (0.0 + -0.05980921681889204 * x[33] + -0.5098839407878217 * x[43] + -0.6296685115857453 * x[47] + 0.05980921681889207 * a0[33] + 0.509883940787822 * a0[43] + 0.6296685115857453 * a0[47]);
+    ns[73] = bias_su[73] + -0.14375068353499568 * x[11] + -0.31483425579287294 * x[15] + -0.03542869495158812 * x[22] + -0.3148342557541156 * x[36] + -0.31483425557789524 * x[48] + x[73] + 0.28750136706999146 * a0[11] + 0.6296685115857457 * a0[15] + 0.07085738990317625 * a0[22] + 0.6296685115082311 * a0[36] + 0.6296685111557906 * a0[48] + sc_xi * (0.0 + -0.2875013670699913 * x[11] + -0.6296685115857453 * x[15] + -0.0708573899031762 * x[22] + -0.6296685115082304 * x[36] + -0.6296685111557904 * x[48] + 0.28750136706999097 * a0[11] + 0.6296685115857458 * a0[15] + 0.07085738990317617 * a0[22] + 0.6296685115082304 * a0[36] + 0.6296685111557899 * a0[48]);
+    for (i = 0; i < 49; ++i) t_t0[i] = a0[i];
+    for (i = 0; i < 10; ++i) t_t1[i] = a1[i];
+    for (i = 0; i < 74; ++i) new_state[i] = ns[i];
+}
```

## Agent notes

```
### Round 1

**Whole-solve mean_us:** 61.32 µs initially, 12.32 µs finally (repeated runs range from about 11.7 to 14.6 µs). Speedup is about 5.0x. `make verify` prints VERIFY OK, with a worst end-to-end error of 4.4e-15 against a tolerance of 1e-6.

**Final profile (`make profile`, instrumented build, 18.9 µs/solve):**
- iterate (`fom_iterate`, 200 calls/solve): 71084 µs total, 99.72%
- residual (last-iteration max-abs difference): 198 µs total, 0.28%

**Optimizations applied:**
1. **Fully unrolled `fom_iterate` with inlined coefficients (61 → ~20 µs).**
   - All products are small and fixed-sparsity: 49+10+74 rows and about 640 nonzeros in total. `fom_iterate` is checked on every unit vector, which allows literal coefficients there.
   - A generator read `data.npz` and emitted one straight-line expression per row, writing each value with exact round-trip `repr`.
   - Zero coefficients are dropped and ±1 coefficients are written as plain additions or subtractions.
   - The two schedule-scaled members of each block share a single `sc_xi * (...)` factor.
   - This removes all CSR index and pointer traffic, the per-matrix loop overhead, and the separate step calls.
   - The su selection rows (0..48 copy t0, 49..58 copy t1) and the `su_t_t1` shift become single additions of `bias_su[i]` plus a local.
   - Clip and ReLU bounds are still read from the `act` arguments, never hardcoded.
2. **All intermediates kept in local arrays, outputs stored at the end (~20 → ~12.5 µs).**
   - t0, t1 and the new state are computed into the locals `a0`, `a1` and `ns`.
   - They are copied to `t_t0`, `t_t1` and `new_state` only after every row is done.
   - Before this, each store to an output pointer forced the compiler to assume it could alias `state`, `bias_*` or `act->lo/hi`, and to reload them. With the stores at the end, the loads can stay in registers and be scheduled freely.
3. **Kept from the translation:** `su_p_p0 @ p0` is computed once per parameter update into `work->acc` and passed as `bias_su`. The residual is computed only on the last iteration. `FOM_PROF_BEGIN/END` markers now wrap the iterate and residual regions in `src/fom_solver.c`.

**Tried and reverted:** splitting the t0 clip into a separate loop the compiler could vectorize. It was slower (about 15 to 17 µs), because the straight-line clip interleaves better with the products.

**Considered but not done:**
- Merging state/t0 coefficient pairs in su rows 59..73 and the t1 rows, such as -0.3148·x[1] with +0.6297·t0[1]. The pairs differ in the last bits, so they are not exact multiples, and merging them would change the matrix being applied.
- Removing the 74/49/10-element copy-out loops by writing selection rows straight into their destinations. The expected gain was marginal, under the bench noise.

Blocked by rules:
- Fusing two consecutive iterations, or keeping state in registers across iterations by unrolling the outer loop | `fom_iterate` must compute exactly one iteration, and `src/fom_solver.c` may not contain matrix products or coefficient literals | unknown, likely around 10% (saves the copy-out and reload of 74+59 values per iteration)
- Declaring `restrict` on the `fom_iterate` arguments so the compiler can drop the local copy-out buffers | headers and prototypes in `include/` are fixed | about 1 µs (removes about 133 stores per iteration)
- Hardcoding the zero entries of `bias_su` on the selection rows, since `su_p_p0` only touches rows 59..73 | `bias` is generic and must be honoured for any input, and `fom_ops.c` may not read the constant or bias data | under 2%

### Round 2

Initial vs final: 63.290 us (translated) -> about 12.9 us (round-1 solver, unchanged this round; this round's runs ranged 10.6-13.0 us). That is about 4.9x overall, with no gain in round 2.

Final profile (`make profile`, instrumented build):
- iterate: 73060 us, 99.73%
- residual: 201 us, 0.27%

Optimizations (all from round 1, kept as is):
- One fused `fom_iterate` pass, fully unrolled over the fixed sparsity with coefficients inlined. All three blocks (t0 clip, t1 relu, state update) run in one routine with the values kept in locals, so there are no zeroing or extra copy passes. This is valid because those routines are checked on every unit vector.
- Selection rows of `su` (rows 0..48 copy t0, rows 49..58 copy t1 via the `su_t_t1` shift) become a bias plus a copy, with no product.
- The `su_p_p0 @ p0` bias is hoisted into `fom_update_params`, since parameters are fixed during a solve. The residual is computed only on the last iteration, because termination is max_iters only.

Tried this round and reverted (no measurable gain, within the ±15% bench noise):
- Factoring the `xi` terms as c*(a0[j]-x[j]) where the x and a0 coefficients are exact negatives. Only 48 of 148 pairs match exactly, which saves 48 multiplies per iteration.
- Writing the state update straight into `new_state` to drop the final 74-entry copy.

Blocked by rules:
- Merge every t1/su row's 4 coefficient groups into one product on (a0 - x), since the a0 coefficients are about -2x the x coefficients and the xi coefficients about 2x apart, cutting flops about 2x | coefficients must exactly match the package data (they differ in the last digits), so the near-equal ones cannot be merged | ~20-30% of iterate
- Precompute per-iteration combined coefficients (c + xi*e) for all 200 iterations at setup | `src/fom_ops.c` may not read the schedule, and `fom_setup` cannot probe matrices | ~30%
```
