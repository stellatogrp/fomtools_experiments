# Optimization report

## Timing

```
translated (emitted)      40.3 us/solve
optimized                  8.9 us/solve
speedup                  4.53x
```

Optimization outcome: **kept (re-verified correct)**.

## Rounds

| round | before (us) | after (us) | gain | kept | agent (s) | outcome |
|---:|---:|---:|---:|:---:|---:|---|
| 1 | 40.336 | 13.565 | 66.4% | yes | 113 | continued |
| 2 | 13.734 | 12.079 | 12.0% | yes | 49 | continued |
| 3 | 11.270 | 9.974 | 11.5% | yes | 44 | continued |
| 4 | 10.197 | 8.901 | 12.7% | yes | 52 | continued |
| 5 | - | - | - | no | 27 | the round left the sources unchanged |

## Kernel checks

Each `fom_apply_<name>` in the shipped `fom_ops.c`, checked by `make verify` against the package's reference over the same `fom_data.c` matrix. *basis* probes every unit vector, which recovers the routine's matrix column by column (an exact check), plus random vectors; *random* uses random vectors only (the matrix is over the probe budget). A `fused_<block>` row is a `fom_fused_<block>`, checked against the sum of its members: *basis* there probes every unit vector of each source with every schedule value at 0 and with each alone at 1, which recovers every member exactly.

| matrix | shape | nnz | rewritten | check | probes | max error |
|---|---|---|---|---|---|---|
| `t0_state` | 49x74 | 147 | yes | basis | 76 | 0.000e+00 |
| `t1_state` | 10x74 | 59 | yes | basis | 76 | 0.000e+00 |
| `t1_t_t0` | 10x49 | 49 | yes | basis | 51 | 0.000e+00 |
| `su_state` | 74x74 | 64 | yes | basis | 76 | 0.000e+00 |
| `su_t_t0` | 74x49 | 98 | yes | basis | 51 | 0.000e+00 |
| `su_t_t1` | 74x10 | 10 | yes | basis | 12 | 0.000e+00 |
| `su_p_p0` | 74x15 | 15 | yes | basis | 17 | 0.000e+00 |
| `fused_t1` | 10x123 | 108 | yes | basis | 125 | 0.000e+00 |
| `fused_su` | 74x133 | 172 | yes | basis | 135 | 0.000e+00 |
| `step_t0` | 49x74 | 147 | yes | basis | 80 | 0.000e+00 |
| `step_t1` | 10x123 | 108 | yes | basis | 129 | 0.000e+00 |
| `step_su` | 74x133 | 172 | yes | basis | 135 | 0.000e+00 |
| `fused_su__rest` | 74x133 | 172 | yes | basis | 135 | 0.000e+00 |
| `step_su__rest` | 74x133 | 172 | yes | basis | 135 | 0.000e+00 |
| `iterate` | 74x74 | 427 | yes | basis | 154 | 8.882e-16 |

### Rule violations

_None: the collected rewrite passed the rules and was kept._

## Blocked by rules

Optimizations the agent considered but did not make because a rule forbade them, in its own words.

| round | idea | rule | expected gain |
|---|---|---|---|
| 1 | Build `w` and the unrolled kernel from the data at setup time instead of with an offline generator script | none; I generated the code offline because time was short | none |
| 1 | Compile flags for wider SIMD (e.g. `-march=native`) to vectorize the 49-row t0 clip pass | Makefile/build flags are fixed | unknown, maybe 10–20% |
| 1 | Add `restrict` to the `fom_iterate` pointer arguments so the compiler can keep `t_t0` in registers instead of re-reading it after stores | headers and prototypes are fixed | small (<5%) |
| 3 | Skip adding `bias_su` on rows 0..58, where `su_p_p0` has no entries for this data | `fom_iterate` must handle any bias it is passed, and verify uses random biases | about 1-2% |
| 3 | Merge `t_t0` and `new_state[0..48]` into one write, since they differ only by the bias | the temporaries must be written separately because the solver and `fom_objective` read them, and the header signatures are fixed | about 2-4% |
| 4 | Merge several iterations into one call, or inline `fom_iterate` into the solver loop to keep the state in registers across iterations | fixed `fom_iterate` signature, and no matrix products allowed in `src/fom_solver.c` | about 5 to 10% |
| 4 | Build with `-ffast-math` or `-march=native` so the compiler regroups and vectorizes the sums itself | Makefile and build flags are fixed | unknown, small |
| 5 | Move the 200-iteration loop into `src/fom_ops.c` so the state stays in registers across iterations | `fom_iterate` signature, headers, and loop placement are fixed (the loop must live in `src/fom_solver.c`) | small, maybe 5-10%, though LTO already inlines the call |

## What the agent changed

```diff
--- src/fom_ops.c (translated)
+++ src/fom_ops.c (optimized)
@@ -1,5 +1,6 @@
 #include "fom_ops.h"
 #include "fom_data.h"
+#include "fom_profile.h"
 
 void fom_apply_t0_state(const fom_float *x, fom_float *out) {
     fom_int i, p;
@@ -180,8 +181,89 @@
     }
 }
 
-void fom_iterate(const fom_float *state, const fom_float *bias_t0, const fom_float *bias_t1, const fom_float *bias_su, const FomAct_t0 *act_t0, const FomAct_t1 *act_t1, fom_float *t_t0, fom_float *t_t1, fom_float *new_state) {
-    fom_step_t0(state, bias_t0, act_t0, t_t0);
-    fom_step_t1(state, t_t0, bias_t1, act_t1, t_t1);
-    fom_step_su(state, t_t0, t_t1, bias_su, new_state);
-}
+void fom_iterate(const fom_float *restrict state, const fom_float *restrict bias_t0, const fom_float *restrict bias_t1, const fom_float *restrict bias_su, const FomAct_t0 *restrict act_t0, const FomAct_t1 *restrict act_t1, fom_float *restrict t_t0, fom_float *restrict t_t1, fom_float *restrict new_state) {
+    /* t1_t_t0 = -2 t1_state[:, :49] and su_t_t0 = -2 su_state[:, :49] on the
+       non-copy rows, so both blocks share w = state[:49] - 2 t_t0 */
+    const fom_float *restrict lo0 = act_t0->lo, *restrict hi0 = act_t0->hi, *restrict lo1 = act_t1->lo;
+    fom_float w[49];
+    FOM_PROF_BEGIN(it_t0);
+    { fom_float v = (bias_t0[0] + state[0]) + (-0.31483425579287277 * state[49] + -0.12559068930987091 * state[59]); if (v < lo0[0]) v = lo0[0]; if (v > hi0[0]) v = hi0[0]; t_t0[0] = v; new_state[0] = bias_su[0] + v; w[0] = state[0] - 2.0 * v; }
+    { fom_float v = (bias_t0[1] + state[1]) + (-0.1785106319392791 * state[49] + -0.31483425579287283 * state[60]); if (v < lo0[1]) v = lo0[1]; if (v > hi0[1]) v = hi0[1]; t_t0[1] = v; new_state[1] = bias_su[1] + v; w[1] = state[1] - 2.0 * v; }
+    { fom_float v = (bias_t0[2] + state[2]) + (-0.31483425565100043 * state[49] + -0.18736322506593048 * state[62]); if (v < lo0[2]) v = lo0[2]; if (v > hi0[2]) v = hi0[2]; t_t0[2] = v; new_state[2] = bias_su[2] + v; w[2] = state[2] - 2.0 * v; }
+    { fom_float v = (bias_t0[3] + state[3]) + (-0.31483425579287277 * state[49] + -0.052105419223993081 * state[65]); if (v < lo0[3]) v = lo0[3]; if (v > hi0[3]) v = hi0[3]; t_t0[3] = v; new_state[3] = bias_su[3] + v; w[3] = state[3] - 2.0 * v; }
+    { fom_float v = (bias_t0[4] + state[4]) + (-0.31483425579287277 * state[49] + -0.042197122740314078 * state[71]); if (v < lo0[4]) v = lo0[4]; if (v > hi0[4]) v = hi0[4]; t_t0[4] = v; new_state[4] = bias_su[4] + v; w[4] = state[4] - 2.0 * v; }
+    { fom_float v = (bias_t0[5] + state[5]) + (-0.31483425579287277 * state[50] + -0.081556129918491121 * state[60]); if (v < lo0[5]) v = lo0[5]; if (v > hi0[5]) v = hi0[5]; t_t0[5] = v; new_state[5] = bias_su[5] + v; w[5] = state[5] - 2.0 * v; }
+    { fom_float v = (bias_t0[6] + state[6]) + (-0.1149099042416003 * state[50] + -0.31483425579287283 * state[61]); if (v < lo0[6]) v = lo0[6]; if (v > hi0[6]) v = hi0[6]; t_t0[6] = v; new_state[6] = bias_su[6] + v; w[6] = state[6] - 2.0 * v; }
+    { fom_float v = (bias_t0[7] + state[7]) + (-0.11245022456106747 * state[50] + -0.31483425579287277 * state[64]); if (v < lo0[7]) v = lo0[7]; if (v > hi0[7]) v = hi0[7]; t_t0[7] = v; new_state[7] = bias_su[7] + v; w[7] = state[7] - 2.0 * v; }
+    { fom_float v = (bias_t0[8] + state[8]) + (-0.31483425579287277 * state[50] + -0.079068432962314555 * state[65]); if (v < lo0[8]) v = lo0[8]; if (v > hi0[8]) v = hi0[8]; t_t0[8] = v; new_state[8] = bias_su[8] + v; w[8] = state[8] - 2.0 * v; }
+    { fom_float v = (bias_t0[9] + state[9]) + (-0.1012882957183817 * state[50] + -0.31483425579287277 * state[66]); if (v < lo0[9]) v = lo0[9]; if (v > hi0[9]) v = hi0[9]; t_t0[9] = v; new_state[9] = bias_su[9] + v; w[9] = state[9] - 2.0 * v; }
+    { fom_float v = (bias_t0[10] + state[10]) + (-0.31483425579287277 * state[50] + -0.024432609086831557 * state[69]); if (v < lo0[10]) v = lo0[10]; if (v > hi0[10]) v = hi0[10]; t_t0[10] = v; new_state[10] = bias_su[10] + v; w[10] = state[10] - 2.0 * v; }
+    { fom_float v = (bias_t0[11] + state[11]) + (-0.31483425576776075 * state[50] + -0.14375068353499568 * state[73]); if (v < lo0[11]) v = lo0[11]; if (v > hi0[11]) v = hi0[11]; t_t0[11] = v; new_state[11] = bias_su[11] + v; w[11] = state[11] - 2.0 * v; }
+    { fom_float v = (bias_t0[12] + state[12]) + (-0.16283459527310534 * state[51] + -0.31483425576550422 * state[59]); if (v < lo0[12]) v = lo0[12]; if (v > hi0[12]) v = hi0[12]; t_t0[12] = v; new_state[12] = bias_su[12] + v; w[12] = state[12] - 2.0 * v; }
+    { fom_float v = (bias_t0[13] + state[13]) + (-0.31483425579287277 * state[51] + -0.057878934426039565 * state[63]); if (v < lo0[13]) v = lo0[13]; if (v > hi0[13]) v = hi0[13]; t_t0[13] = v; new_state[13] = bias_su[13] + v; w[13] = state[13] - 2.0 * v; }
+    { fom_float v = (bias_t0[14] + state[14]) + (-0.31483425579287277 * state[51] + -0.020371967614988162 * state[65]); if (v < lo0[14]) v = lo0[14]; if (v > hi0[14]) v = hi0[14]; t_t0[14] = v; new_state[14] = bias_su[14] + v; w[14] = state[14] - 2.0 * v; }
+    { fom_float v = (bias_t0[15] + state[15]) + (-0.13195167489181564 * state[51] + -0.31483425579287283 * state[73]); if (v < lo0[15]) v = lo0[15]; if (v > hi0[15]) v = hi0[15]; t_t0[15] = v; new_state[15] = bias_su[15] + v; w[15] = state[15] - 2.0 * v; }
+    { fom_float v = (bias_t0[16] + state[16]) + (-0.20484874346487622 * state[52] + -0.31483425569820117 * state[59]); if (v < lo0[16]) v = lo0[16]; if (v > hi0[16]) v = hi0[16]; t_t0[16] = v; new_state[16] = bias_su[16] + v; w[16] = state[16] - 2.0 * v; }
+    { fom_float v = (bias_t0[17] + state[17]) + (-0.31483425579287277 * state[52] + -0.059210692177281046 * state[61]); if (v < lo0[17]) v = lo0[17]; if (v > hi0[17]) v = hi0[17]; t_t0[17] = v; new_state[17] = bias_su[17] + v; w[17] = state[17] - 2.0 * v; }
+    { fom_float v = (bias_t0[18] + state[18]) + (-0.31483425579287277 * state[52] + -0.041860218187796047 * state[63]); if (v < lo0[18]) v = lo0[18]; if (v > hi0[18]) v = hi0[18]; t_t0[18] = v; new_state[18] = bias_su[18] + v; w[18] = state[18] - 2.0 * v; }
+    { fom_float v = (bias_t0[19] + state[19]) + (-0.20135753992977129 * state[52] + -0.31483425562205569 * state[64]); if (v < lo0[19]) v = lo0[19]; if (v > hi0[19]) v = hi0[19]; t_t0[19] = v; new_state[19] = bias_su[19] + v; w[19] = state[19] - 2.0 * v; }
+    { fom_float v = (bias_t0[20] + state[20]) + (-0.17170029832945369 * state[52] + -0.31483425579287283 * state[65]); if (v < lo0[20]) v = lo0[20]; if (v > hi0[20]) v = hi0[20]; t_t0[20] = v; new_state[20] = bias_su[20] + v; w[20] = state[20] - 2.0 * v; }
+    { fom_float v = (bias_t0[21] + state[21]) + (-0.31483425579287277 * state[52] + -0.095865201522679772 * state[70]); if (v < lo0[21]) v = lo0[21]; if (v > hi0[21]) v = hi0[21]; t_t0[21] = v; new_state[21] = bias_su[21] + v; w[21] = state[21] - 2.0 * v; }
+    { fom_float v = (bias_t0[22] + state[22]) + (-0.31483425579287277 * state[52] + -0.035428694951588111 * state[73]); if (v < lo0[22]) v = lo0[22]; if (v > hi0[22]) v = hi0[22]; t_t0[22] = v; new_state[22] = bias_su[22] + v; w[22] = state[22] - 2.0 * v; }
+    { fom_float v = (bias_t0[23] + state[23]) + (-0.11549111655165208 * state[53] + -0.31483425579287277 * state[62]); if (v < lo0[23]) v = lo0[23]; if (v > hi0[23]) v = hi0[23]; t_t0[23] = v; new_state[23] = bias_su[23] + v; w[23] = state[23] - 2.0 * v; }
+    { fom_float v = (bias_t0[24] + state[24]) + (-0.31483425579287277 * state[53] + -0.023238864598874573 * state[68]); if (v < lo0[24]) v = lo0[24]; if (v > hi0[24]) v = hi0[24]; t_t0[24] = v; new_state[24] = bias_su[24] + v; w[24] = state[24] - 2.0 * v; }
+    { fom_float v = (bias_t0[25] + state[25]) + (-0.22703157183385472 * state[53] + -0.31483425579287283 * state[70]); if (v < lo0[25]) v = lo0[25]; if (v > hi0[25]) v = hi0[25]; t_t0[25] = v; new_state[25] = bias_su[25] + v; w[25] = state[25] - 2.0 * v; }
+    { fom_float v = (bias_t0[26] + state[26]) + (-0.31483425579287277 * state[54] + -0.066859627087189483 * state[59]); if (v < lo0[26]) v = lo0[26]; if (v > hi0[26]) v = hi0[26]; t_t0[26] = v; new_state[26] = bias_su[26] + v; w[26] = state[26] - 2.0 * v; }
+    { fom_float v = (bias_t0[27] + state[27]) + (-0.31483425578928326 * state[54] + -0.18070942432892551 * state[60]); if (v < lo0[27]) v = lo0[27]; if (v > hi0[27]) v = hi0[27]; t_t0[27] = v; new_state[27] = bias_su[27] + v; w[27] = state[27] - 2.0 * v; }
+    { fom_float v = (bias_t0[28] + state[28]) + (-0.11812566027056481 * state[54] + -0.31483425579287283 * state[63]); if (v < lo0[28]) v = lo0[28]; if (v > hi0[28]) v = hi0[28]; t_t0[28] = v; new_state[28] = bias_su[28] + v; w[28] = state[28] - 2.0 * v; }
+    { fom_float v = (bias_t0[29] + state[29]) + (-0.31483425579287277 * state[54] + -0.069839424573329659 * state[64]); if (v < lo0[29]) v = lo0[29]; if (v > hi0[29]) v = hi0[29]; t_t0[29] = v; new_state[29] = bias_su[29] + v; w[29] = state[29] - 2.0 * v; }
+    { fom_float v = (bias_t0[30] + state[30]) + (-0.31483425579287277 * state[54] + -0.018290493543120279 * state[66]); if (v < lo0[30]) v = lo0[30]; if (v > hi0[30]) v = hi0[30]; t_t0[30] = v; new_state[30] = bias_su[30] + v; w[30] = state[30] - 2.0 * v; }
+    { fom_float v = (bias_t0[31] + state[31]) + (-0.11621549871906076 * state[54] + -0.31483425579287277 * state[69]); if (v < lo0[31]) v = lo0[31]; if (v > hi0[31]) v = hi0[31]; t_t0[31] = v; new_state[31] = bias_su[31] + v; w[31] = state[31] - 2.0 * v; }
+    { fom_float v = (bias_t0[32] + state[32]) + (-0.31483425579287277 * state[54] + -0.041474234626015731 * state[71]); if (v < lo0[32]) v = lo0[32]; if (v > hi0[32]) v = hi0[32]; t_t0[32] = v; new_state[32] = bias_su[32] + v; w[32] = state[32] - 2.0 * v; }
+    { fom_float v = (bias_t0[33] + state[33]) + (-0.31483425579287277 * state[54] + -0.029904608409446035 * state[72]); if (v < lo0[33]) v = lo0[33]; if (v > hi0[33]) v = hi0[33]; t_t0[33] = v; new_state[33] = bias_su[33] + v; w[33] = state[33] - 2.0 * v; }
+    { fom_float v = (bias_t0[34] + state[34]) + (-0.20072699455717991 * state[55] + -0.31483425579287277 * state[68]); if (v < lo0[34]) v = lo0[34]; if (v > hi0[34]) v = hi0[34]; t_t0[34] = v; new_state[34] = bias_su[34] + v; w[34] = state[34] - 2.0 * v; }
+    { fom_float v = (bias_t0[35] + state[35]) + (-0.31483425579287283 * state[55] + -0.26432062531400063 * state[69]); if (v < lo0[35]) v = lo0[35]; if (v > hi0[35]) v = hi0[35]; t_t0[35] = v; new_state[35] = bias_su[35] + v; w[35] = state[35] - 2.0 * v; }
+    { fom_float v = (bias_t0[36] + state[36]) + (-0.20810320529794363 * state[55] + -0.31483425575411544 * state[73]); if (v < lo0[36]) v = lo0[36]; if (v > hi0[36]) v = hi0[36]; t_t0[36] = v; new_state[36] = bias_su[36] + v; w[36] = state[36] - 2.0 * v; }
+    { fom_float v = (bias_t0[37] + state[37]) + (-0.31483425579287277 * state[56] + -0.050057390438124606 * state[60]); if (v < lo0[37]) v = lo0[37]; if (v > hi0[37]) v = hi0[37]; t_t0[37] = v; new_state[37] = bias_su[37] + v; w[37] = state[37] - 2.0 * v; }
+    { fom_float v = (bias_t0[38] + state[38]) + (-0.31483425579287277 * state[56] + -0.022355820755170213 * state[66]); if (v < lo0[38]) v = lo0[38]; if (v > hi0[38]) v = hi0[38]; t_t0[38] = v; new_state[38] = bias_su[38] + v; w[38] = state[38] - 2.0 * v; }
+    { fom_float v = (bias_t0[39] + state[39]) + (-0.14832333169428388 * state[57] + -0.31483425579287277 * state[59]); if (v < lo0[39]) v = lo0[39]; if (v > hi0[39]) v = hi0[39]; t_t0[39] = v; new_state[39] = bias_su[39] + v; w[39] = state[39] - 2.0 * v; }
+    { fom_float v = (bias_t0[40] + state[40]) + (-0.25687797426285774 * state[57] + -0.3148342557740812 * state[67]); if (v < lo0[40]) v = lo0[40]; if (v > hi0[40]) v = hi0[40]; t_t0[40] = v; new_state[40] = bias_su[40] + v; w[40] = state[40] - 2.0 * v; }
+    { fom_float v = (bias_t0[41] + state[41]) + (-0.31483425562699158 * state[57] + -0.2046259837099301 * state[69]); if (v < lo0[41]) v = lo0[41]; if (v > hi0[41]) v = hi0[41]; t_t0[41] = v; new_state[41] = bias_su[41] + v; w[41] = state[41] - 2.0 * v; }
+    { fom_float v = (bias_t0[42] + state[42]) + (-0.31483425579287277 * state[57] + -0.066937775413790396 * state[71]); if (v < lo0[42]) v = lo0[42]; if (v > hi0[42]) v = hi0[42]; t_t0[42] = v; new_state[42] = bias_su[42] + v; w[42] = state[42] - 2.0 * v; }
+    { fom_float v = (bias_t0[43] + state[43]) + (-0.31483425573999474 * state[57] + -0.25494197039391109 * state[72]); if (v < lo0[43]) v = lo0[43]; if (v > hi0[43]) v = hi0[43]; t_t0[43] = v; new_state[43] = bias_su[43] + v; w[43] = state[43] - 2.0 * v; }
+    { fom_float v = (bias_t0[44] + state[44]) + (-0.31483425579287277 * state[58] + -0.063379996679072001 * state[65]); if (v < lo0[44]) v = lo0[44]; if (v > hi0[44]) v = hi0[44]; t_t0[44] = v; new_state[44] = bias_su[44] + v; w[44] = state[44] - 2.0 * v; }
+    { fom_float v = (bias_t0[45] + state[45]) + (-0.2409314392941177 * state[58] + -0.31483425579287283 * state[67]); if (v < lo0[45]) v = lo0[45]; if (v > hi0[45]) v = hi0[45]; t_t0[45] = v; new_state[45] = bias_su[45] + v; w[45] = state[45] - 2.0 * v; }
+    { fom_float v = (bias_t0[46] + state[46]) + (-0.15802369728139173 * state[58] + -0.31483425579287277 * state[71]); if (v < lo0[46]) v = lo0[46]; if (v > hi0[46]) v = hi0[46]; t_t0[46] = v; new_state[46] = bias_su[46] + v; w[46] = state[46] - 2.0 * v; }
+    { fom_float v = (bias_t0[47] + state[47]) + (-0.21287292492070603 * state[58] + -0.31483425579287277 * state[72]); if (v < lo0[47]) v = lo0[47]; if (v > hi0[47]) v = hi0[47]; t_t0[47] = v; new_state[47] = bias_su[47] + v; w[47] = state[47] - 2.0 * v; }
+    { fom_float v = (bias_t0[48] + state[48]) + (-0.27468270126800032 * state[58] + -0.31483425557789518 * state[73]); if (v < lo0[48]) v = lo0[48]; if (v > hi0[48]) v = hi0[48]; t_t0[48] = v; new_state[48] = bias_su[48] + v; w[48] = state[48] - 2.0 * v; }
+    FOM_PROF_END(it_t0);
+    FOM_PROF_BEGIN(it_t1);
+    { fom_float v = ((bias_t1[0] + -0.31483425579287277 * w[0]) + (-0.1785106319392791 * w[1] + -0.31483425565100043 * w[2])) + ((-0.31483425579287277 * w[3] + -0.31483425579287277 * w[4]) + state[49]); if (v < lo1[0]) v = lo1[0]; t_t1[0] = v; new_state[49] = bias_su[49] + v; }
+    { fom_float v = (((bias_t1[1] + -0.31483425579287277 * w[5]) + -0.1149099042416003 * w[6]) + (-0.11245022456106747 * w[7] + -0.31483425579287277 * w[8])) + ((-0.1012882957183817 * w[9] + -0.31483425579287277 * w[10]) + (-0.31483425576776075 * w[11] + state[50])); if (v < lo1[1]) v = lo1[1]; t_t1[1] = v; new_state[50] = bias_su[50] + v; }
+    { fom_float v = ((bias_t1[2] + -0.16283459527310534 * w[12]) + -0.31483425579287277 * w[13]) + ((-0.31483425579287277 * w[14] + -0.13195167489181564 * w[15]) + state[51]); if (v < lo1[2]) v = lo1[2]; t_t1[2] = v; new_state[51] = bias_su[51] + v; }
+    { fom_float v = (((bias_t1[3] + -0.20484874346487622 * w[16]) + -0.31483425579287277 * w[17]) + (-0.31483425579287277 * w[18] + -0.20135753992977129 * w[19])) + ((-0.17170029832945369 * w[20] + -0.31483425579287277 * w[21]) + (-0.31483425579287277 * w[22] + state[52])); if (v < lo1[3]) v = lo1[3]; t_t1[3] = v; new_state[52] = bias_su[52] + v; }
+    { fom_float v = ((bias_t1[4] + -0.11549111655165208 * w[23]) + -0.31483425579287277 * w[24]) + (-0.22703157183385472 * w[25] + state[53]); if (v < lo1[4]) v = lo1[4]; t_t1[4] = v; new_state[53] = bias_su[53] + v; }
+    { fom_float v = (((bias_t1[5] + -0.31483425579287277 * w[26]) + -0.31483425578928326 * w[27]) + (-0.11812566027056481 * w[28] + -0.31483425579287277 * w[29])) + (((-0.31483425579287277 * w[30] + -0.11621549871906076 * w[31]) + -0.31483425579287277 * w[32]) + (-0.31483425579287277 * w[33] + state[54])); if (v < lo1[5]) v = lo1[5]; t_t1[5] = v; new_state[54] = bias_su[54] + v; }
+    { fom_float v = ((bias_t1[6] + -0.20072699455717991 * w[34]) + -0.31483425579287283 * w[35]) + (-0.20810320529794363 * w[36] + state[55]); if (v < lo1[6]) v = lo1[6]; t_t1[6] = v; new_state[55] = bias_su[55] + v; }
+    { fom_float v = (bias_t1[7] + -0.31483425579287277 * w[37]) + (-0.31483425579287277 * w[38] + state[56]); if (v < lo1[7]) v = lo1[7]; t_t1[7] = v; new_state[56] = bias_su[56] + v; }
+    { fom_float v = ((bias_t1[8] + -0.14832333169428388 * w[39]) + (-0.25687797426285774 * w[40] + -0.31483425562699158 * w[41])) + ((-0.31483425579287277 * w[42] + -0.31483425573999474 * w[43]) + state[57]); if (v < lo1[8]) v = lo1[8]; t_t1[8] = v; new_state[57] = bias_su[57] + v; }
+    { fom_float v = ((bias_t1[9] + -0.31483425579287277 * w[44]) + (-0.2409314392941177 * w[45] + -0.15802369728139173 * w[46])) + ((-0.21287292492070603 * w[47] + -0.27468270126800032 * w[48]) + state[58]); if (v < lo1[9]) v = lo1[9]; t_t1[9] = v; new_state[58] = bias_su[58] + v; }
+    FOM_PROF_END(it_t1);
+    FOM_PROF_BEGIN(it_su);
+    new_state[59] = ((bias_su[59] + -0.12559068930987091 * w[0]) + (-0.31483425576550422 * w[12] + -0.31483425569820117 * w[16])) + ((-0.066859627087189483 * w[26] + -0.31483425579287277 * w[39]) + state[59]);
+    new_state[60] = ((bias_su[60] + -0.31483425579287283 * w[1]) + -0.081556129918491121 * w[5]) + ((-0.18070942432892551 * w[27] + -0.050057390438124606 * w[37]) + state[60]);
+    new_state[61] = (bias_su[61] + -0.31483425579287283 * w[6]) + (-0.059210692177281046 * w[17] + state[61]);
+    new_state[62] = (bias_su[62] + -0.18736322506593048 * w[2]) + (-0.31483425579287277 * w[23] + state[62]);
+    new_state[63] = ((bias_su[63] + -0.057878934426039565 * w[13]) + -0.041860218187796047 * w[18]) + (-0.31483425579287283 * w[28] + state[63]);
+    new_state[64] = ((bias_su[64] + -0.31483425579287277 * w[7]) + -0.31483425562205569 * w[19]) + (-0.069839424573329659 * w[29] + state[64]);
+    new_state[65] = ((bias_su[65] + -0.052105419223993081 * w[3]) + (-0.079068432962314555 * w[8] + -0.020371967614988162 * w[14])) + ((-0.31483425579287283 * w[20] + -0.063379996679072001 * w[44]) + state[65]);
+    new_state[66] = ((bias_su[66] + -0.31483425579287277 * w[9]) + -0.018290493543120279 * w[30]) + (-0.022355820755170213 * w[38] + state[66]);
+    new_state[67] = (bias_su[67] + -0.3148342557740812 * w[40]) + (-0.31483425579287283 * w[45] + state[67]);
+    new_state[68] = (bias_su[68] + -0.023238864598874573 * w[24]) + (-0.31483425579287277 * w[34] + state[68]);
+    new_state[69] = ((bias_su[69] + -0.024432609086831557 * w[10]) + -0.31483425579287277 * w[31]) + ((-0.26432062531400063 * w[35] + -0.2046259837099301 * w[41]) + state[69]);
+    new_state[70] = (bias_su[70] + -0.095865201522679772 * w[21]) + (-0.31483425579287283 * w[25] + state[70]);
+    new_state[71] = ((bias_su[71] + -0.042197122740314078 * w[4]) + -0.041474234626015731 * w[32]) + ((-0.066937775413790396 * w[42] + -0.31483425579287277 * w[46]) + state[71]);
+    new_state[72] = ((bias_su[72] + -0.029904608409446035 * w[33]) + -0.25494197039391109 * w[43]) + (-0.31483425579287277 * w[47] + state[72]);
+    new_state[73] = ((bias_su[73] + -0.14375068353499568 * w[11]) + (-0.31483425579287283 * w[15] + -0.035428694951588111 * w[22])) + ((-0.31483425575411544 * w[36] + -0.31483425557789518 * w[48]) + state[73]);
+    FOM_PROF_END(it_su);
+}
```

## Agent notes

```
### Round 1

**Timing:** `mean_us` went from 40.32 (initial) to about 12 (final; runs ranged 11.1 to 13.0). That is a speedup of about 3.3×. `make verify`: VERIFY OK (worst error 4.4e-15, tol 1e-6).

**Final profile** (`make profile`; it reports 32.6 µs per solve because of timer overhead, so the shares are inflated):
- it_t0 (clip block, 49 rows): 44977 µs total, 49.0%
- it_su (state update): 26767 µs total, 29.1%
- it_t1 (relu block, 10 rows): 20146 µs total, 21.9%

**Optimizations applied:**
1. **Fully unrolled `fom_iterate` with the coefficients written in.** The structure exploited is the fixed CSR sparsity of each matrix. The code is generated from the `fom_data.c` values, copied exactly as printed. Unit coefficients become plain adds or subtracts, and the clip, relu and bias are folded into each row. There are no zeroing or copy passes and no index lookups. The selection rows (su rows 0..58 copy t0/t1) and the su_t_t1 shift become direct assignments. This took the solve from 40.3 to about 16 µs.
2. **Temporaries written straight into `t_t0`/`t_t1`** and read back from there, with no extra local copies. Gain is small and within noise (about 16 µs).
3. **Shared intermediate across blocks.** The structure exploited: a check script found `t1_t_t0[i,k] == -2*t1_state[i,k]` exactly for all 49 entries. The same holds for `su_t_t0[i,k] == -2*su_state[i,k]` on every non-copy row of su (rows 59..73). So `c*s_k + (-2c)*t0_k` becomes `c*w_k`, with `w = state[:49] - 2*t_t0` computed once in the t0 pass and used by both t1 and su. This removes 98 multiply-adds per iteration and took the solve to about 12 µs. It holds for any schedule and activation values; the per-unit-vector checks recover `c` and `-2c` exactly.
4. `FOM_PROF_BEGIN/END` markers stay around the three blocks inside `fom_iterate` (it_t0, it_t1, it_su).

Blocked by rules:
- Build `w` and the unrolled kernel from the data at setup time instead of with an offline generator script | none; I generated the code offline because time was short | none
- Compile flags for wider SIMD (e.g. `-march=native`) to vectorize the 49-row t0 clip pass | Makefile/build flags are fixed | unknown, maybe 10–20%
- Add `restrict` to the `fom_iterate` pointer arguments so the compiler can keep `t_t0` in registers instead of re-reading it after stores | headers and prototypes are fixed | small (<5%)

### Round 2

**Timing (`make bench` mean_us, noisy machine)**
- Initial for this round: 13.565 us (my baseline reruns: 13.3–15.3 us).
- Final: about 10.6–13.6 us across reruns, roughly 12 us typical.
- Speedup this round: about 1.1x. Against the translated solver (40.336 us): about 3.3x overall.

**Final profile (`make profile`, hottest first; per-call timer overhead inflates the totals)**

| Region | Time | Share |
|---|---|---|
| it_t0 | 52660 us | 56.8% |
| it_t1 | 21241 us | 22.9% |
| it_su | 18827 us | 20.3% |

**Optimizations**
- **Copy rows written during the t0/t1 pass (this round).** Rows 0..48 of `su` copy `t_t0` and rows 49..58 copy `t_t1`. Each `new_state[i] = bias_su[i] + v` is now written while the clipped or relu'd value `v` is still in a register. This removes the separate 59-row pass that reloaded `t_t0` and `t_t1`. Only the 15 rows that are not copies remain in it_su.
- **Kept from round 1:** a fully unrolled `fom_iterate` with coefficients inlined over the fixed sparsity. Each `t0` row is `state[i]` plus two scaled state entries (an incidence structure). `t1_t_t0` equals −2 × `t1_state[:, :49]`, and `su_t_t0` equals −2 × `su_state[:, :49]` on the rows that are not copies. So `t1` and `su` share one intermediate, `w = state[:49] − 2 t_t0`. The `p0` term is applied once per solve into `bias_su`. The residual is computed only on the last iteration, and the state buffers are swapped by pointer.

**Blocked by rules:**
- none

### Round 3

**Whole-solve time (`make bench` mean_us)**
- At the start of this round: 11.63 us measured (12.08 us reported by the pipeline). The translated solver took 40.34 us.
- Final: 9.82 to 10.73 us over 3 runs, about 10.0 us typical.
- This round is about 1.16x faster. Against the translated solver it is about 4.0x.

**Final profile (`make profile`, instrumented build, hottest first)**
| Region | Time | Share |
|---|---|---|
| it_t0 | 42845 us | 52.1% |
| it_t1 | 20705 us | 25.2% |
| it_su | 18679 us | 22.7% |

**Optimizations applied**
- **`restrict` on every pointer argument of `fom_iterate`** (state, the biases, the activation bounds, `t_t0`, `t_t1`, `new_state`), plus local `restrict` pointers for the clip bounds.
  - Before this, the compiler had to assume that each store to `t_t0`, `new_state` or `w` could overwrite `state`, the biases or the bounds. So it reloaded shared entries after every store, for example `state[49..73]`, which several unrolled rows read.
  - The change is valid because the solver always passes separate buffers: state and new_state are ping-ponged, and the temporaries, biases and bounds live in their own workspace or constant arrays.
  - Gain: about 13%. `make verify` still prints `VERIFY OK`, with a worst error of 4.4e-15.
- **Kept from earlier rounds:**
  - One fully unrolled `fom_iterate` with the coefficients written as literals.
  - The relations `t1_t_t0 = -2 * t1_state[:, :49]` and `su_t_t0 = -2 * su_state[:, :49]` on the non-copy rows. Both blocks share `w = state[:49] - 2 * t_t0`.
  - The selection rows of `su` are written directly.
  - The parameter bias is computed once per solve.
  - The residual is computed only on the last iteration.
- Nothing else I found looked worth a further pass. Each iteration is already a single unrolled pass with no redundant work, so further gains would be marginal.

Blocked by rules:
- Skip adding `bias_su` on rows 0..58, where `su_p_p0` has no entries for this data | `fom_iterate` must handle any bias it is passed, and verify uses random biases | about 1-2%
- Merge `t_t0` and `new_state[0..48]` into one write, since they differ only by the bias | the temporaries must be written separately because the solver and `fom_objective` read them, and the header signatures are fixed | about 2-4%

### Round 4

**Whole-solve mean_us:** 9.974 at the start of this round, about 8.8 at the end (8 to 10% faster this round). Against the translated solver's 40.336 us, that is about 4.6x overall. `make bench` is noisy (roughly ±10% from run to run), so I compared the old and new versions in alternating runs. The old version had a median of about 9.6 us and the new one about 8.8 us.

**Final profile (`make profile`), hottest first.** The timers themselves add most of the time at this scale, so read these as proportions only.
- it_t0: 41688 us, 50.5%
- it_t1: 21250 us, 25.8%
- it_su: 19544 us, 23.7%

**Optimizations**
- Earlier rounds, kept as they were:
  - `fom_iterate` runs the whole iteration in one fully unrolled pass, with the coefficients written in.
  - The selection rows of `su` are written straight from t0 and t1.
  - Both `t1_t_t0` and `su_t_t0` equal -2 times the first 49 columns of `t1_state` and `su_state` on the rows that are not copies. So `t1` and the rows of `su` that are not copies share one vector, w = state[:49] - 2 t0.
  - The `p0` term is computed once per solve and folded into the `su` bias.
  - The residual is computed only at the last iteration, since there is no tolerance check and the solver always runs max_iters.
- New this round: the per-row sums are regrouped into a balanced tree.
  - This applies to the t0 rows, the t1 rows (up to 9 terms) and the `su` rows 59..73.
  - The loop-carried path is state[49..58] → t0 → w → t1 → new state. It is limited by latency, and the left-to-right sums made each row a serial chain of additions. The tree cuts that chain from n to about log2(n) adds.
  - It is valid because the rows are tiny, fixed-sparsity sums, and the regrouped values are the same matrix entries. Verify's worst error is 4.9e-15, against a tolerance of 1e-6.

There is little left to gain. The iteration is now about 45 ns, bounded by the latency of the t0 → t1 → state chain.

Blocked by rules:
- Merge several iterations into one call, or inline `fom_iterate` into the solver loop to keep the state in registers across iterations | fixed `fom_iterate` signature, and no matrix products allowed in `src/fom_solver.c` | about 5 to 10%
- Build with `-ffast-math` or `-march=native` so the compiler regroups and vectorizes the sums itself | Makefile and build flags are fixed | unknown, small

### Round 5

Initial vs. final whole-solve mean_us:
- Translated solver: 40.336 us. Start of this round: 8.901 us as reported, about 7.2 to 7.5 us on re-measurement (three `make bench` runs: 7.25, 7.55, 7.24). Final: unchanged, about 7.2 us. Overall speedup is 4.5x to 5.6x; the range comes from bench noise between runs.
- This round made no change to the sources.

Final profile (`make profile`, 200 iterations x 5000 solves; the timers inflate the totals):
- it_t0: 36794 us, 50.6%
- it_t1: 18736 us, 25.8%
- it_su: 17139 us, 23.6%

Optimizations in place, all from earlier rounds:
- Whole iteration in one `fom_iterate`, fully unrolled over the fixed sparsity with the coefficients inlined. Structure: each t0 row is the identity on state[:49] plus exactly two off-diagonal entries.
- Shared intermediate `w = state[:49] - 2 t0`. Structure: `t1_t_t0 = -2 t1_state[:, :49]` and `su_t_t0 = -2 su_state[:, :49]` on the non-copy rows. This turns two products into one.
- Selection rows written straight into new_state. Structure: su rows 0..48 copy t0 and rows 49..58 copy t1, so each value is written right after its activation.
- Parameter term `su_p_p0 @ p0` hoisted out of the loop as a per-solve bias. The residual is computed only on the last iteration, and the state is double-buffered by pointer swap.
- Balanced summation trees per row to shorten the dependency chain.

Why nothing more this round: each iteration depends on the previous one, so the solve is latency-bound at about 36 ns per iteration with about 300 flops. The remaining time is the t0 → w → t1 → state chain. Further reordering or SIMD gathers are unlikely to pay off at these sizes.

Blocked by rules:
- Move the 200-iteration loop into `src/fom_ops.c` so the state stays in registers across iterations | `fom_iterate` signature, headers, and loop placement are fixed (the loop must live in `src/fom_solver.c`) | small, maybe 5-10%, though LTO already inlines the call
```
