# Optimization report

## Timing

```
translated (emitted)      31.8 us/solve
optimized                  8.7 us/solve
speedup                  3.63x
```

Optimization outcome: **kept (re-verified correct)**.

## Rounds

| round | before (us) | after (us) | gain | kept | agent (s) | outcome |
|---:|---:|---:|---:|:---:|---:|---|
| 1 | 31.752 | 9.585 | 69.8% | yes | 80 | continued |
| 2 | 9.767 | 8.750 | 10.4% | yes | 108 | continued |
| 3 | - | - | - | no | 80 | the round left the sources unchanged |

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
| `step_t1` | 10x123 | 108 | yes | basis | 129 | 4.441e-16 |
| `step_su` | 74x133 | 172 | yes | basis | 135 | 0.000e+00 |
| `fused_su__rest` | 74x133 | 172 | yes | basis | 135 | 0.000e+00 |
| `step_su__rest` | 74x133 | 172 | yes | basis | 135 | 8.882e-16 |
| `iterate` | 74x74 | 427 | yes | basis | 154 | 1.776e-15 |

### Rule violations

_None: the collected rewrite passed the rules and was kept._

## Blocked by rules

Optimizations the agent considered but did not make because a rule forbade them, in its own words.

| round | idea | rule | expected gain |
|---|---|---|---|
| 1 | Write the fused iteration loop (all three blocks plus the residual) directly in `fom_solver.c`, keeping the new state in registers across iterations | `src/fom_solver.c` must not read matrix data, and the coefficients may only be written as literals inside the checked routines | unknown, small (calls are already cheap after inlining) |
| 1 | Hardcode the zero ReLU and clip bounds in the step kernels to drop the `lo`/`hi` loads | must apply whatever `act` is passed; never hardcode bounds | ~1-3% |
| 1 | Hoist `t0_c` and `t1_c` into the constant per row of the unrolled kernels | must not read the constant vectors in `src/fom_ops.c`; bias is passed in | ~1-2% |
| 2 | Skip adding `bias_su` on the selection rows inside `fom_iterate` (the solver's bias is zero on those rows), which would make the fused one-pass iteration with the shared `w` profitable | `fom_iterate` must be exact for any bias passed to it | ~5-10% |
| 3 | Have `fom_iterate` skip the bias add on the selection rows of `su`, relying on the solver's `su` bias being zero there | `fom_iterate` must be exact for any bias, so each row must add `bias_su[row]` | small, maybe 2-5% |

## What the agent changed

```diff
--- src/fom_solver.c (translated)
+++ src/fom_solver.c (optimized)
@@ -2,6 +2,7 @@
 #include "fom_solver.h"
 #include "fom_data.h"
 #include "fom_ops.h"
+#include "fom_profile.h"
 
 void fom_set_default_settings(FomSettings *settings) {
     settings->max_iter = FOM_MAX_ITER;
@@ -43,12 +44,16 @@
 
     for (k = 0; k < max_iter; ++k) {
         fom_float *tmp;
-        fom_step_t0(x, FOM_MAT_t0_c, &FOM_ACT_t0, t0);
-        fom_step_t1(x, t0, FOM_MAT_t1_c, &FOM_ACT_t1, t1);
-        /* selection rows: 0..48 <- t0, 49..58 <- t1 */
-        memcpy(xn, t0, FOM_T0_DIM * sizeof(fom_float));
-        memcpy(xn + FOM_T0_DIM, t1, FOM_T1_DIM * sizeof(fom_float));
-        fom_step_su__rest(x, t0, t1, work->acc, xn);
+        /* selection rows of su: t0 and t1 are written straight into rows 0..58 of xn */
+        FOM_PROF_BEGIN(step_t0);
+        fom_step_t0(x, FOM_MAT_t0_c, &FOM_ACT_t0, xn);
+        FOM_PROF_END(step_t0);
+        FOM_PROF_BEGIN(step_t1);
+        fom_step_t1(x, xn, FOM_MAT_t1_c, &FOM_ACT_t1, xn + FOM_T0_DIM);
+        FOM_PROF_END(step_t1);
+        FOM_PROF_BEGIN(step_su_rest);
+        fom_step_su__rest(x, xn, xn + FOM_T0_DIM, work->acc, xn);
+        FOM_PROF_END(step_su_rest);
         if (use_tol || k == max_iter - 1) {
             res = 0;
             for (i = 0; i < FOM_STATE_DIM; ++i) {
@@ -61,6 +66,10 @@
         if (use_tol && res <= tol) { status = FOM_SOLVED; ++k; break; }
     }
     if (x != work->state) memcpy(work->state, x, sizeof(work->state));
+    if (k > 0) {
+        memcpy(t0, x, FOM_T0_DIM * sizeof(fom_float));
+        memcpy(t1, x + FOM_T0_DIM, FOM_T1_DIM * sizeof(fom_float));
+    }
     memcpy(work->solution.x, work->state, sizeof(work->solution.x));
     work->solution.iter = k;
     work->solution.residual = res;
--- src/fom_ops.c (translated)
+++ src/fom_ops.c (optimized)
@@ -1,6 +1,92 @@
 #include "fom_ops.h"
 #include "fom_data.h"
 
+static inline void k_t0(const fom_float *restrict state, const fom_float *restrict bias, const fom_float *restrict lo, const fom_float *restrict hi, fom_float *restrict out) {
+    fom_float v;
+    v = bias[0] + (0.0 + state[0] + -0.3148342557928728 * state[49] + -0.1255906893098709 * state[59]); v = v < lo[0] ? lo[0] : v; out[0] = v > hi[0] ? hi[0] : v;
+    v = bias[1] + (0.0 + state[1] + -0.1785106319392791 * state[49] + -0.31483425579287283 * state[60]); v = v < lo[1] ? lo[1] : v; out[1] = v > hi[1] ? hi[1] : v;
+    v = bias[2] + (0.0 + state[2] + -0.3148342556510004 * state[49] + -0.18736322506593048 * state[62]); v = v < lo[2] ? lo[2] : v; out[2] = v > hi[2] ? hi[2] : v;
+    v = bias[3] + (0.0 + state[3] + -0.3148342557928728 * state[49] + -0.05210541922399308 * state[65]); v = v < lo[3] ? lo[3] : v; out[3] = v > hi[3] ? hi[3] : v;
+    v = bias[4] + (0.0 + state[4] + -0.3148342557928728 * state[49] + -0.04219712274031408 * state[71]); v = v < lo[4] ? lo[4] : v; out[4] = v > hi[4] ? hi[4] : v;
+    v = bias[5] + (0.0 + state[5] + -0.3148342557928728 * state[50] + -0.08155612991849112 * state[60]); v = v < lo[5] ? lo[5] : v; out[5] = v > hi[5] ? hi[5] : v;
+    v = bias[6] + (0.0 + state[6] + -0.1149099042416003 * state[50] + -0.31483425579287283 * state[61]); v = v < lo[6] ? lo[6] : v; out[6] = v > hi[6] ? hi[6] : v;
+    v = bias[7] + (0.0 + state[7] + -0.11245022456106747 * state[50] + -0.3148342557928728 * state[64]); v = v < lo[7] ? lo[7] : v; out[7] = v > hi[7] ? hi[7] : v;
+    v = bias[8] + (0.0 + state[8] + -0.3148342557928728 * state[50] + -0.07906843296231456 * state[65]); v = v < lo[8] ? lo[8] : v; out[8] = v > hi[8] ? hi[8] : v;
+    v = bias[9] + (0.0 + state[9] + -0.1012882957183817 * state[50] + -0.3148342557928728 * state[66]); v = v < lo[9] ? lo[9] : v; out[9] = v > hi[9] ? hi[9] : v;
+    v = bias[10] + (0.0 + state[10] + -0.3148342557928728 * state[50] + -0.024432609086831557 * state[69]); v = v < lo[10] ? lo[10] : v; out[10] = v > hi[10] ? hi[10] : v;
+    v = bias[11] + (0.0 + state[11] + -0.31483425576776075 * state[50] + -0.14375068353499568 * state[73]); v = v < lo[11] ? lo[11] : v; out[11] = v > hi[11] ? hi[11] : v;
+    v = bias[12] + (0.0 + state[12] + -0.16283459527310534 * state[51] + -0.3148342557655042 * state[59]); v = v < lo[12] ? lo[12] : v; out[12] = v > hi[12] ? hi[12] : v;
+    v = bias[13] + (0.0 + state[13] + -0.3148342557928728 * state[51] + -0.057878934426039565 * state[63]); v = v < lo[13] ? lo[13] : v; out[13] = v > hi[13] ? hi[13] : v;
+    v = bias[14] + (0.0 + state[14] + -0.3148342557928728 * state[51] + -0.020371967614988162 * state[65]); v = v < lo[14] ? lo[14] : v; out[14] = v > hi[14] ? hi[14] : v;
+    v = bias[15] + (0.0 + state[15] + -0.13195167489181564 * state[51] + -0.31483425579287283 * state[73]); v = v < lo[15] ? lo[15] : v; out[15] = v > hi[15] ? hi[15] : v;
+    v = bias[16] + (0.0 + state[16] + -0.20484874346487622 * state[52] + -0.31483425569820117 * state[59]); v = v < lo[16] ? lo[16] : v; out[16] = v > hi[16] ? hi[16] : v;
+    v = bias[17] + (0.0 + state[17] + -0.3148342557928728 * state[52] + -0.059210692177281046 * state[61]); v = v < lo[17] ? lo[17] : v; out[17] = v > hi[17] ? hi[17] : v;
+    v = bias[18] + (0.0 + state[18] + -0.3148342557928728 * state[52] + -0.04186021818779605 * state[63]); v = v < lo[18] ? lo[18] : v; out[18] = v > hi[18] ? hi[18] : v;
+    v = bias[19] + (0.0 + state[19] + -0.2013575399297713 * state[52] + -0.3148342556220557 * state[64]); v = v < lo[19] ? lo[19] : v; out[19] = v > hi[19] ? hi[19] : v;
+    v = bias[20] + (0.0 + state[20] + -0.1717002983294537 * state[52] + -0.31483425579287283 * state[65]); v = v < lo[20] ? lo[20] : v; out[20] = v > hi[20] ? hi[20] : v;
+    v = bias[21] + (0.0 + state[21] + -0.3148342557928728 * state[52] + -0.09586520152267977 * state[70]); v = v < lo[21] ? lo[21] : v; out[21] = v > hi[21] ? hi[21] : v;
+    v = bias[22] + (0.0 + state[22] + -0.3148342557928728 * state[52] + -0.03542869495158811 * state[73]); v = v < lo[22] ? lo[22] : v; out[22] = v > hi[22] ? hi[22] : v;
+    v = bias[23] + (0.0 + state[23] + -0.11549111655165208 * state[53] + -0.3148342557928728 * state[62]); v = v < lo[23] ? lo[23] : v; out[23] = v > hi[23] ? hi[23] : v;
+    v = bias[24] + (0.0 + state[24] + -0.3148342557928728 * state[53] + -0.023238864598874573 * state[68]); v = v < lo[24] ? lo[24] : v; out[24] = v > hi[24] ? hi[24] : v;
+    v = bias[25] + (0.0 + state[25] + -0.22703157183385472 * state[53] + -0.31483425579287283 * state[70]); v = v < lo[25] ? lo[25] : v; out[25] = v > hi[25] ? hi[25] : v;
+    v = bias[26] + (0.0 + state[26] + -0.3148342557928728 * state[54] + -0.06685962708718948 * state[59]); v = v < lo[26] ? lo[26] : v; out[26] = v > hi[26] ? hi[26] : v;
+    v = bias[27] + (0.0 + state[27] + -0.31483425578928326 * state[54] + -0.1807094243289255 * state[60]); v = v < lo[27] ? lo[27] : v; out[27] = v > hi[27] ? hi[27] : v;
+    v = bias[28] + (0.0 + state[28] + -0.11812566027056481 * state[54] + -0.31483425579287283 * state[63]); v = v < lo[28] ? lo[28] : v; out[28] = v > hi[28] ? hi[28] : v;
+    v = bias[29] + (0.0 + state[29] + -0.3148342557928728 * state[54] + -0.06983942457332966 * state[64]); v = v < lo[29] ? lo[29] : v; out[29] = v > hi[29] ? hi[29] : v;
+    v = bias[30] + (0.0 + state[30] + -0.3148342557928728 * state[54] + -0.01829049354312028 * state[66]); v = v < lo[30] ? lo[30] : v; out[30] = v > hi[30] ? hi[30] : v;
+    v = bias[31] + (0.0 + state[31] + -0.11621549871906076 * state[54] + -0.3148342557928728 * state[69]); v = v < lo[31] ? lo[31] : v; out[31] = v > hi[31] ? hi[31] : v;
+    v = bias[32] + (0.0 + state[32] + -0.3148342557928728 * state[54] + -0.04147423462601573 * state[71]); v = v < lo[32] ? lo[32] : v; out[32] = v > hi[32] ? hi[32] : v;
+    v = bias[33] + (0.0 + state[33] + -0.3148342557928728 * state[54] + -0.029904608409446035 * state[72]); v = v < lo[33] ? lo[33] : v; out[33] = v > hi[33] ? hi[33] : v;
+    v = bias[34] + (0.0 + state[34] + -0.2007269945571799 * state[55] + -0.3148342557928728 * state[68]); v = v < lo[34] ? lo[34] : v; out[34] = v > hi[34] ? hi[34] : v;
+    v = bias[35] + (0.0 + state[35] + -0.31483425579287283 * state[55] + -0.26432062531400063 * state[69]); v = v < lo[35] ? lo[35] : v; out[35] = v > hi[35] ? hi[35] : v;
+    v = bias[36] + (0.0 + state[36] + -0.20810320529794363 * state[55] + -0.31483425575411544 * state[73]); v = v < lo[36] ? lo[36] : v; out[36] = v > hi[36] ? hi[36] : v;
+    v = bias[37] + (0.0 + state[37] + -0.3148342557928728 * state[56] + -0.050057390438124606 * state[60]); v = v < lo[37] ? lo[37] : v; out[37] = v > hi[37] ? hi[37] : v;
+    v = bias[38] + (0.0 + state[38] + -0.3148342557928728 * state[56] + -0.022355820755170213 * state[66]); v = v < lo[38] ? lo[38] : v; out[38] = v > hi[38] ? hi[38] : v;
+    v = bias[39] + (0.0 + state[39] + -0.14832333169428388 * state[57] + -0.3148342557928728 * state[59]); v = v < lo[39] ? lo[39] : v; out[39] = v > hi[39] ? hi[39] : v;
+    v = bias[40] + (0.0 + state[40] + -0.25687797426285774 * state[57] + -0.3148342557740812 * state[67]); v = v < lo[40] ? lo[40] : v; out[40] = v > hi[40] ? hi[40] : v;
+    v = bias[41] + (0.0 + state[41] + -0.3148342556269916 * state[57] + -0.2046259837099301 * state[69]); v = v < lo[41] ? lo[41] : v; out[41] = v > hi[41] ? hi[41] : v;
+    v = bias[42] + (0.0 + state[42] + -0.3148342557928728 * state[57] + -0.0669377754137904 * state[71]); v = v < lo[42] ? lo[42] : v; out[42] = v > hi[42] ? hi[42] : v;
+    v = bias[43] + (0.0 + state[43] + -0.31483425573999474 * state[57] + -0.2549419703939111 * state[72]); v = v < lo[43] ? lo[43] : v; out[43] = v > hi[43] ? hi[43] : v;
+    v = bias[44] + (0.0 + state[44] + -0.3148342557928728 * state[58] + -0.063379996679072 * state[65]); v = v < lo[44] ? lo[44] : v; out[44] = v > hi[44] ? hi[44] : v;
+    v = bias[45] + (0.0 + state[45] + -0.2409314392941177 * state[58] + -0.31483425579287283 * state[67]); v = v < lo[45] ? lo[45] : v; out[45] = v > hi[45] ? hi[45] : v;
+    v = bias[46] + (0.0 + state[46] + -0.15802369728139173 * state[58] + -0.3148342557928728 * state[71]); v = v < lo[46] ? lo[46] : v; out[46] = v > hi[46] ? hi[46] : v;
+    v = bias[47] + (0.0 + state[47] + -0.21287292492070603 * state[58] + -0.3148342557928728 * state[72]); v = v < lo[47] ? lo[47] : v; out[47] = v > hi[47] ? hi[47] : v;
+    v = bias[48] + (0.0 + state[48] + -0.2746827012680003 * state[58] + -0.3148342555778952 * state[73]); v = v < lo[48] ? lo[48] : v; out[48] = v > hi[48] ? hi[48] : v;
+}
+
+static inline void k_t1(const fom_float *restrict state, const fom_float *restrict t_t0, const fom_float *restrict bias, const fom_float *restrict lo, fom_float *restrict out) {
+    fom_float v;
+    v = bias[0] + (0.0 + -0.3148342557928728 * (state[0] - 2.0 * t_t0[0]) + -0.1785106319392791 * (state[1] - 2.0 * t_t0[1]) + -0.3148342556510004 * (state[2] - 2.0 * t_t0[2]) + -0.3148342557928728 * (state[3] - 2.0 * t_t0[3]) + -0.3148342557928728 * (state[4] - 2.0 * t_t0[4]) + state[49]); out[0] = v < lo[0] ? lo[0] : v;
+    v = bias[1] + (0.0 + -0.3148342557928728 * (state[5] - 2.0 * t_t0[5]) + -0.1149099042416003 * (state[6] - 2.0 * t_t0[6]) + -0.11245022456106747 * (state[7] - 2.0 * t_t0[7]) + -0.3148342557928728 * (state[8] - 2.0 * t_t0[8]) + -0.1012882957183817 * (state[9] - 2.0 * t_t0[9]) + -0.3148342557928728 * (state[10] - 2.0 * t_t0[10]) + -0.31483425576776075 * (state[11] - 2.0 * t_t0[11]) + state[50]); out[1] = v < lo[1] ? lo[1] : v;
+    v = bias[2] + (0.0 + -0.16283459527310534 * (state[12] - 2.0 * t_t0[12]) + -0.3148342557928728 * (state[13] - 2.0 * t_t0[13]) + -0.3148342557928728 * (state[14] - 2.0 * t_t0[14]) + -0.13195167489181564 * (state[15] - 2.0 * t_t0[15]) + state[51]); out[2] = v < lo[2] ? lo[2] : v;
+    v = bias[3] + (0.0 + -0.20484874346487622 * (state[16] - 2.0 * t_t0[16]) + -0.3148342557928728 * (state[17] - 2.0 * t_t0[17]) + -0.3148342557928728 * (state[18] - 2.0 * t_t0[18]) + -0.2013575399297713 * (state[19] - 2.0 * t_t0[19]) + -0.1717002983294537 * (state[20] - 2.0 * t_t0[20]) + -0.3148342557928728 * (state[21] - 2.0 * t_t0[21]) + -0.3148342557928728 * (state[22] - 2.0 * t_t0[22]) + state[52]); out[3] = v < lo[3] ? lo[3] : v;
+    v = bias[4] + (0.0 + -0.11549111655165208 * (state[23] - 2.0 * t_t0[23]) + -0.3148342557928728 * (state[24] - 2.0 * t_t0[24]) + -0.22703157183385472 * (state[25] - 2.0 * t_t0[25]) + state[53]); out[4] = v < lo[4] ? lo[4] : v;
+    v = bias[5] + (0.0 + -0.3148342557928728 * (state[26] - 2.0 * t_t0[26]) + -0.31483425578928326 * (state[27] - 2.0 * t_t0[27]) + -0.11812566027056481 * (state[28] - 2.0 * t_t0[28]) + -0.3148342557928728 * (state[29] - 2.0 * t_t0[29]) + -0.3148342557928728 * (state[30] - 2.0 * t_t0[30]) + -0.11621549871906076 * (state[31] - 2.0 * t_t0[31]) + -0.3148342557928728 * (state[32] - 2.0 * t_t0[32]) + -0.3148342557928728 * (state[33] - 2.0 * t_t0[33]) + state[54]); out[5] = v < lo[5] ? lo[5] : v;
+    v = bias[6] + (0.0 + -0.2007269945571799 * (state[34] - 2.0 * t_t0[34]) + -0.31483425579287283 * (state[35] - 2.0 * t_t0[35]) + -0.20810320529794363 * (state[36] - 2.0 * t_t0[36]) + state[55]); out[6] = v < lo[6] ? lo[6] : v;
+    v = bias[7] + (0.0 + -0.3148342557928728 * (state[37] - 2.0 * t_t0[37]) + -0.3148342557928728 * (state[38] - 2.0 * t_t0[38]) + state[56]); out[7] = v < lo[7] ? lo[7] : v;
+    v = bias[8] + (0.0 + -0.14832333169428388 * (state[39] - 2.0 * t_t0[39]) + -0.25687797426285774 * (state[40] - 2.0 * t_t0[40]) + -0.3148342556269916 * (state[41] - 2.0 * t_t0[41]) + -0.3148342557928728 * (state[42] - 2.0 * t_t0[42]) + -0.31483425573999474 * (state[43] - 2.0 * t_t0[43]) + state[57]); out[8] = v < lo[8] ? lo[8] : v;
+    v = bias[9] + (0.0 + -0.3148342557928728 * (state[44] - 2.0 * t_t0[44]) + -0.2409314392941177 * (state[45] - 2.0 * t_t0[45]) + -0.15802369728139173 * (state[46] - 2.0 * t_t0[46]) + -0.21287292492070603 * (state[47] - 2.0 * t_t0[47]) + -0.2746827012680003 * (state[48] - 2.0 * t_t0[48]) + state[58]); out[9] = v < lo[9] ? lo[9] : v;
+}
+
+static inline void k_su_rest(const fom_float *restrict state, const fom_float *restrict t_t0, const fom_float *restrict t_t1, const fom_float *restrict bias, fom_float *restrict out) {
+    out[59] = bias[59] + (0.0 + -0.1255906893098709 * (state[0] - 2.0 * t_t0[0]) + -0.3148342557655042 * (state[12] - 2.0 * t_t0[12]) + -0.31483425569820117 * (state[16] - 2.0 * t_t0[16]) + -0.06685962708718948 * (state[26] - 2.0 * t_t0[26]) + -0.3148342557928728 * (state[39] - 2.0 * t_t0[39]) + state[59]);
+    out[60] = bias[60] + (0.0 + -0.31483425579287283 * (state[1] - 2.0 * t_t0[1]) + -0.08155612991849112 * (state[5] - 2.0 * t_t0[5]) + -0.1807094243289255 * (state[27] - 2.0 * t_t0[27]) + -0.050057390438124606 * (state[37] - 2.0 * t_t0[37]) + state[60]);
+    out[61] = bias[61] + (0.0 + -0.31483425579287283 * (state[6] - 2.0 * t_t0[6]) + -0.059210692177281046 * (state[17] - 2.0 * t_t0[17]) + state[61]);
+    out[62] = bias[62] + (0.0 + -0.18736322506593048 * (state[2] - 2.0 * t_t0[2]) + -0.3148342557928728 * (state[23] - 2.0 * t_t0[23]) + state[62]);
+    out[63] = bias[63] + (0.0 + -0.057878934426039565 * (state[13] - 2.0 * t_t0[13]) + -0.04186021818779605 * (state[18] - 2.0 * t_t0[18]) + -0.31483425579287283 * (state[28] - 2.0 * t_t0[28]) + state[63]);
+    out[64] = bias[64] + (0.0 + -0.3148342557928728 * (state[7] - 2.0 * t_t0[7]) + -0.3148342556220557 * (state[19] - 2.0 * t_t0[19]) + -0.06983942457332966 * (state[29] - 2.0 * t_t0[29]) + state[64]);
+    out[65] = bias[65] + (0.0 + -0.05210541922399308 * (state[3] - 2.0 * t_t0[3]) + -0.07906843296231456 * (state[8] - 2.0 * t_t0[8]) + -0.020371967614988162 * (state[14] - 2.0 * t_t0[14]) + -0.31483425579287283 * (state[20] - 2.0 * t_t0[20]) + -0.063379996679072 * (state[44] - 2.0 * t_t0[44]) + state[65]);
+    out[66] = bias[66] + (0.0 + -0.3148342557928728 * (state[9] - 2.0 * t_t0[9]) + -0.01829049354312028 * (state[30] - 2.0 * t_t0[30]) + -0.022355820755170213 * (state[38] - 2.0 * t_t0[38]) + state[66]);
+    out[67] = bias[67] + (0.0 + -0.3148342557740812 * (state[40] - 2.0 * t_t0[40]) + -0.31483425579287283 * (state[45] - 2.0 * t_t0[45]) + state[67]);
+    out[68] = bias[68] + (0.0 + -0.023238864598874573 * (state[24] - 2.0 * t_t0[24]) + -0.3148342557928728 * (state[34] - 2.0 * t_t0[34]) + state[68]);
+    out[69] = bias[69] + (0.0 + -0.024432609086831557 * (state[10] - 2.0 * t_t0[10]) + -0.3148342557928728 * (state[31] - 2.0 * t_t0[31]) + -0.26432062531400063 * (state[35] - 2.0 * t_t0[35]) + -0.2046259837099301 * (state[41] - 2.0 * t_t0[41]) + state[69]);
+    out[70] = bias[70] + (0.0 + -0.09586520152267977 * (state[21] - 2.0 * t_t0[21]) + -0.31483425579287283 * (state[25] - 2.0 * t_t0[25]) + state[70]);
+    out[71] = bias[71] + (0.0 + -0.04219712274031408 * (state[4] - 2.0 * t_t0[4]) + -0.04147423462601573 * (state[32] - 2.0 * t_t0[32]) + -0.0669377754137904 * (state[42] - 2.0 * t_t0[42]) + -0.3148342557928728 * (state[46] - 2.0 * t_t0[46]) + state[71]);
+    out[72] = bias[72] + (0.0 + -0.029904608409446035 * (state[33] - 2.0 * t_t0[33]) + -0.2549419703939111 * (state[43] - 2.0 * t_t0[43]) + -0.3148342557928728 * (state[47] - 2.0 * t_t0[47]) + state[72]);
+    out[73] = bias[73] + (0.0 + -0.14375068353499568 * (state[11] - 2.0 * t_t0[11]) + -0.31483425579287283 * (state[15] - 2.0 * t_t0[15]) + -0.03542869495158811 * (state[22] - 2.0 * t_t0[22]) + -0.31483425575411544 * (state[36] - 2.0 * t_t0[36]) + -0.3148342555778952 * (state[48] - 2.0 * t_t0[48]) + state[73]);
+}
+
+
 void fom_apply_t0_state(const fom_float *x, fom_float *out) {
     fom_int i, p;
     for (i = 0; i < 49; ++i) {
@@ -94,35 +180,11 @@
 }
 
 void fom_step_t0(const fom_float *state, const fom_float *bias, const FomAct_t0 *act, fom_float *out) {
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
+    k_t0(state, bias, act->lo, act->hi, out);
 }
 
 void fom_step_t1(const fom_float *state, const fom_float *t_t0, const fom_float *bias, const FomAct_t1 *act, fom_float *out) {
-    fom_int i, r, p;
-    for (r = 0; r < 10; ++r) {
-        fom_float s, v;
-        i = r;
-        v = bias[i];
-        s = 0.0;
-        for (p = FOM_MAT_t1_state_ROWPTR[i]; p < FOM_MAT_t1_state_ROWPTR[i + 1]; ++p) s += FOM_MAT_t1_state_VALS[p] * state[FOM_MAT_t1_state_COLIDX[p]];
-        v += s;
-        s = 0.0;
-        for (p = FOM_MAT_t1_t_t0_ROWPTR[i]; p < FOM_MAT_t1_t_t0_ROWPTR[i + 1]; ++p) s += FOM_MAT_t1_t_t0_VALS[p] * t_t0[FOM_MAT_t1_t_t0_COLIDX[p]];
-        v += s;
-        if (v < act->lo[i]) v = act->lo[i];
-        out[i] = v;
-    }
+    k_t1(state, t_t0, bias, act->lo, out);
 }
 
 void fom_step_su(const fom_float *state, const fom_float *t_t0, const fom_float *t_t1, const fom_float *bias, fom_float *out) {
@@ -162,22 +224,7 @@
 }
 
 void fom_step_su__rest(const fom_float *state, const fom_float *t_t0, const fom_float *t_t1, const fom_float *bias, fom_float *out) {
-    fom_int i, r, p;
-    for (r = 0; r < FOM_REST_su_N; ++r) {
-        fom_float s, v;
-        i = FOM_REST_su_ROWS[r];
-        v = bias[i];
-        s = 0.0;
-        for (p = FOM_MAT_su_state_ROWPTR[i]; p < FOM_MAT_su_state_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_state_VALS[p] * state[FOM_MAT_su_state_COLIDX[p]];
-        v += s;
-        s = 0.0;
-        for (p = FOM_MAT_su_t_t0_ROWPTR[i]; p < FOM_MAT_su_t_t0_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t0_VALS[p] * t_t0[FOM_MAT_su_t_t0_COLIDX[p]];
-        v += s;
-        s = 0.0;
-        for (p = FOM_MAT_su_t_t1_ROWPTR[i]; p < FOM_MAT_su_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t1_VALS[p] * t_t1[FOM_MAT_su_t_t1_COLIDX[p]];
-        v += s;
-        out[i] = v;
-    }
+    k_su_rest(state, t_t0, t_t1, bias, out);
 }
 
 void fom_iterate(const fom_float *state, const fom_float *bias_t0, const fom_float *bias_t1, const fom_float *bias_su, const FomAct_t0 *act_t0, const FomAct_t1 *act_t1, fom_float *t_t0, fom_float *t_t1, fom_float *new_state) {
```

## Agent notes

```
### Round 1

**Whole-solve mean_us:** 29.16 initially, 10.3 finally (runs ranged 10.0 to 11.7). Overall speedup is about 2.8×.

**Final profile breakdown** (`make profile`, 200 iterations × 5000 solves). The timer overhead per call is comparable to each region's own cost (profile build 26.1 µs vs 10.3 µs normal), so only the percentages are meaningful:
| region | total_us | pct |
|---|---|---|
| step_t0 (49-row clip block) | 23822 | 38.4% |
| step_su_rest (15 non-selection rows of su) | 19617 | 31.6% |
| step_t1 (10-row relu block) | 18581 | 30.0% |

**Optimizations applied:**
1. **Fully unrolled step kernels with the coefficients written in as literals** (`fom_step_t0`, `fom_step_t1`, `fom_step_su__rest`). The matrices' sparsity pattern never changes: t0_state has 147 nonzeros, t1_state + t1_t_t0 have 108, and the 15 rest rows of su_state + su_t_t0 + su_t_t1 are short. I generated one straight-line expression per row from `fom_data.c`, using exact shortest-round-trip float literals. Unit coefficients become plain adds or subtracts, and the CSR index and pointer loads are gone. This is allowed because these routines are checked on every unit vector. Bias and activation bounds still come from the arguments. 29.2 → 17.6 µs.
2. **Wrote t0 and t1 straight into the new state.** SU rows 0..48 copy t0 and rows 49..58 copy t1, so `fom_step_t0` writes into `xn`, `fom_step_t1` into `xn+49`, and `fom_step_su__rest` fills only rows 59..73. This removes two memcpys per iteration. `work->t0` and `work->t1` are filled once after the loop, from the final state. The hoisted `su_p_p0 @ p0` bias is still computed once per parameter update. 17.6 → about 15.4 µs.
3. **`restrict` on the internal static kernels.** The state is read-only, and the parts of `xn` that are read and written never overlap (reads of rows 0..58 versus writes to 49..58 and 59..73). Without `restrict` the compiler had to assume each output store could change an input and reload it. About 15.4 → 10.3 µs.

The `FOM_PROF_BEGIN/END` markers stay around step_t0, step_t1 and step_su_rest.

Blocked by rules:
- Write the fused iteration loop (all three blocks plus the residual) directly in `fom_solver.c`, keeping the new state in registers across iterations | `src/fom_solver.c` must not read matrix data, and the coefficients may only be written as literals inside the checked routines | unknown, small (calls are already cheap after inlining)
- Hardcode the zero ReLU and clip bounds in the step kernels to drop the `lo`/`hi` loads | must apply whatever `act` is passed; never hardcode bounds | ~1-3%
- Hoist `t0_c` and `t1_c` into the constant per row of the unrolled kernels | must not read the constant vectors in `src/fom_ops.c`; bias is passed in | ~1-2%

### Round 2

**Whole-solve time**
- Initial: 9.585 µs (pipeline figure at the start of this round).
- Final: about 8.3 µs. In 5 back-to-back runs the round-1 code averaged 9.46 µs and the new code 8.28 µs, about 1.14x faster.
- The new code was faster in every one of those 5 pairs.
- Against the translated solver (31.752 µs) that is roughly 3.8x overall.
- Run-to-run noise on this machine was ±15%. The last three plain benches read 8.5 to 8.9 µs.

**Final profile (`make profile`, instrumented build), hottest first**
- step_t0: 22722 µs, 37.3%
- step_t1: 19672 µs, 32.3%
- step_su_rest: 18451 µs, 30.3%

**Optimizations applied**
- **Factored the t1 and su updates through `state - 2*t0`.** Every coefficient on a `t_t0[j]` term in `t1_t_t0` and in the non-selection rows of `su_t_t0` equals -2 times the coefficient on `state[j]` in the matching `t1_state` / `su_state` row, to within 1e-15 relative. All 98 such pairs follow this pattern; this is the incidence-matrix structure of the network problem. So each pair `c*state[j] + d*t_t0[j]` becomes `c*(state[j] - 2.0*t_t0[j])`. That removes 98 multiplies and 98 coefficient loads per iteration, in `fom_step_t1` and `fom_step_su__rest`. Verify reports a worst error of about 3.6e-15.
- **Tried and dropped: a fused `fom_iterate`.** It computed `w = state - 2*t0` once and shared it between t1 and su. It verified, but it was consistently about 10% slower than calling the three step routines. The cause is that it must handle any bias, so it has to write `bias_su + t0/t1` into the 59 selection rows. The solver's direct-copy path avoids that work.
- Kept from round 1: fully unrolled kernels with coefficients inlined, t0 and t1 written straight into their selection rows of the new state, the bias `su_p_p0 @ p0` computed once per solve, and pointer swapping instead of copying the state.

Blocked by rules:
- Skip adding `bias_su` on the selection rows inside `fom_iterate` (the solver's bias is zero on those rows), which would make the fused one-pass iteration with the shared `w` profitable | `fom_iterate` must be exact for any bias passed to it | ~5-10%

### Round 3

**Result: no change this round.** The sources are the round-2 solver, unchanged.
- Initial `mean_us`: 8.750 (reported by the pipeline). The same code measures 6.77 to 7.09 us in back-to-back runs here.
- Final `mean_us`: 6.766. Against the translated solver's 31.752 us, that is about 4.7x on this run's timing (3.63x on the reported numbers).
- This round's gain is zero. The two candidate rewrites both lost to the unchanged code when benchmarked back to back.

Final profile (`make profile`), hottest first:
- step_t0: 21011 us, 37.35%
- step_t1: 18102 us, 32.18%
- step_su_rest: 17137 us, 30.47%

Optimizations already in place from earlier rounds (kept):
- **Unrolled sparse kernels with literal coefficients.** `t0` is identity plus two entries per row, with a clip folded in. `t1` and the 15 non-selection rows of `su` are unrolled over their fixed sparsity. They use the shared structure `t1_state` = -½·`t1_t_t0`, so both products collapse into one coefficient times `(state - 2 t0)`.
- **Selection rows written directly.** Rows 0..48 of `su` copy `t0` and rows 49..58 copy `t1`. The steps write `t0` and `t1` straight into the new state, with no copy pass.
- **Hoisted parameter term.** `su_p_p0 @ p0` is computed once per parameter update, in `fom_setup`/`fom_update_params`.
- **Residual only when needed.** It is computed only when tolerance checking is on, or at the last iteration.

Tried this round and reverted (both passed `make verify`):
- **Fused `fom_iterate`.** One pass that shares `w = state - 2 t0` between `t1` and `su`, with separate `t0`/`t1` buffers. It ran at 7.39 us against 6.77 us. The extra stores to the temporaries and the local `w` array cost more than the shared subtraction saved.
- **The same fused `fom_iterate` with `t_t0`/`t_t1` aliased into `new_state`** (no `restrict`). It ran at 12.2 us. The compiler can't schedule loads and stores well when the outputs may alias.

Blocked by rules:
- Have `fom_iterate` skip the bias add on the selection rows of `su`, relying on the solver's `su` bias being zero there | `fom_iterate` must be exact for any bias, so each row must add `bias_su[row]` | small, maybe 2-5%
```
