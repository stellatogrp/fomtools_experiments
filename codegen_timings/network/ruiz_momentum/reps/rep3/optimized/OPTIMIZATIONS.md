# Optimization report

## Timing

```
translated (emitted)      62.5 us/solve
optimized                 13.9 us/solve
speedup                  4.51x
```

Optimization outcome: **kept (re-verified correct)**.

## Rounds

| round | before (us) | after (us) | gain | kept | agent (s) | outcome |
|---:|---:|---:|---:|:---:|---:|---|
| 1 | 62.488 | 19.355 | 69.0% | yes | 60 | continued |
| 2 | 19.898 | 13.863 | 30.3% | yes | 48 | continued |
| 3 | - | - | - | no | 46 | the round left the sources unchanged |

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
| `step_t1` | 10x123 | 206 | yes | basis | 252 | 0.000e+00 |
| `step_su` | 74x133 | 270 | yes | basis | 268 | 0.000e+00 |
| `fused_su__rest` | 74x133 | 270 | yes | basis | 268 | 0.000e+00 |
| `step_su__rest` | 74x133 | 270 | yes | basis | 268 | 0.000e+00 |
| `iterate` | 74x74 | 623 | yes | basis | 228 | 1.776e-15 |

### Rule violations

_None: the collected rewrite passed the rules and was kept._

## Blocked by rules

Optimizations the agent considered but did not make because a rule forbade them, in its own words.

| round | idea | rule | expected gain |
|---|---|---|---|
| 1 | Precompute `t1_state + xi_k*t1_state__d_xi` (and the other `__d_xi` pairs) for each scheduled `xi` value, so each pair costs one product instead of two | src/fom_ops.c must not read the schedule; fom_setup/fom_update_params must not rebuild matrices; the fom_data.c schedule and matrix data are fixed | ~10-20% |
| 1 | Compile with `-march=native` / `-ffast-math` so the compiler can reassociate and vectorize the unrolled sums | Makefile and build flags are fixed | unknown, maybe 10-20% |
| 2 | Write the `*__d_xi` coefficients as 2x the base coefficients, or fold the matching state and t0 coefficient pairs together, to save multiplies | verify checks each fused routine at every schedule value and against each member's exact coefficients, and those pairs agree only to about 1e-15, so the result is not exact | unknown, probably about 10-20% of the floating-point work |
| 2 | Inline `fom_iterate` into the solve loop to save one call per iteration | `src/fom_solver.c` may not read matrix data or contain products | small, a few percent |
| 3 | Write each t1 / su row with paired coefficients as `c*(t_t0[j] - state[j])`, using one shared coefficient for the matching state and t_t0 entries (they are negatives of each other up to about 1e-10, and 2x apart in the non-xi terms) | verify requires literals to equal each matrix entry exactly, far below a perturbed coefficient | about 10–20% fewer multiplies in t1/su, maybe 5% of the solve |
| 3 | Build with `-ffast-math` or `-march=native` so the compiler can reassociate and vectorize the unrolled rows | Makefile and build flags are fixed | unknown, maybe 5–15% |

## What the agent changed

```diff
--- src/fom_solver.c (translated)
+++ src/fom_solver.c (optimized)
@@ -1,6 +1,7 @@
 #include "fom_solver.h"
 #include "fom_data.h"
 #include "fom_ops.h"
+#include "fom_profile.h"
 #include <string.h>
 
 void fom_set_default_settings(FomSettings *settings) {
@@ -36,8 +37,10 @@
 
     for (it = 0; it < max_iter; ++it) {
         fom_float xi = fom_sched_xi(it);
+        FOM_PROF_BEGIN(iterate);
         fom_iterate(state, xi, FOM_MAT_t0_c, FOM_MAT_t1_c, bias,
                     &FOM_ACT_t0, &FOM_ACT_t1, work->t0, work->t1, ns);
+        FOM_PROF_END(iterate);
         if (it == max_iter - 1) {
             res = 0.0;
             for (i = 0; i < FOM_STATE_DIM; ++i) {
--- src/fom_ops.c (translated)
+++ src/fom_ops.c (optimized)
@@ -252,8 +252,199 @@
     }
 }
 
-void fom_iterate(const fom_float *state, fom_float sc_xi, const fom_float *bias_t0, const fom_float *bias_t1, const fom_float *bias_su, const FomAct_t0 *act_t0, const FomAct_t1 *act_t1, fom_float *t_t0, fom_float *t_t1, fom_float *new_state) {
-    fom_step_t0(state, bias_t0, act_t0, t_t0);
-    fom_step_t1(state, t_t0, sc_xi, bias_t1, act_t1, t_t1);
-    fom_step_su(state, t_t0, t_t1, sc_xi, bias_su, new_state);
-}
+void fom_iterate(const fom_float *restrict state, fom_float sc_xi, const fom_float *restrict bias_t0, const fom_float *restrict bias_t1, const fom_float *restrict bias_su, const FomAct_t0 *act_t0, const FomAct_t1 *act_t1, fom_float *restrict t_t0, fom_float *restrict t_t1, fom_float *restrict new_state) {
+    const fom_float *lo0 = act_t0->lo, *hi0 = act_t0->hi, *lo1 = act_t1->lo;
+    fom_float v;
+    v = bias_t0[0] + (state[0] - 0.3148342557928728*state[49] - 0.1255906893098709*state[59]);
+    v = v < lo0[0] ? lo0[0] : v; v = v > hi0[0] ? hi0[0] : v; t_t0[0] = v;
+    v = bias_t0[1] + (state[1] - 0.1785106319392791*state[49] - 0.31483425579287283*state[60]);
+    v = v < lo0[1] ? lo0[1] : v; v = v > hi0[1] ? hi0[1] : v; t_t0[1] = v;
+    v = bias_t0[2] + (state[2] - 0.3148342556510004*state[49] - 0.18736322506593048*state[62]);
+    v = v < lo0[2] ? lo0[2] : v; v = v > hi0[2] ? hi0[2] : v; t_t0[2] = v;
+    v = bias_t0[3] + (state[3] - 0.3148342557928728*state[49] - 0.05210541922399308*state[65]);
+    v = v < lo0[3] ? lo0[3] : v; v = v > hi0[3] ? hi0[3] : v; t_t0[3] = v;
+    v = bias_t0[4] + (state[4] - 0.3148342557928728*state[49] - 0.04219712274031408*state[71]);
+    v = v < lo0[4] ? lo0[4] : v; v = v > hi0[4] ? hi0[4] : v; t_t0[4] = v;
+    v = bias_t0[5] + (state[5] - 0.3148342557928728*state[50] - 0.08155612991849112*state[60]);
+    v = v < lo0[5] ? lo0[5] : v; v = v > hi0[5] ? hi0[5] : v; t_t0[5] = v;
+    v = bias_t0[6] + (state[6] - 0.1149099042416003*state[50] - 0.31483425579287283*state[61]);
+    v = v < lo0[6] ? lo0[6] : v; v = v > hi0[6] ? hi0[6] : v; t_t0[6] = v;
+    v = bias_t0[7] + (state[7] - 0.11245022456106747*state[50] - 0.3148342557928728*state[64]);
+    v = v < lo0[7] ? lo0[7] : v; v = v > hi0[7] ? hi0[7] : v; t_t0[7] = v;
+    v = bias_t0[8] + (state[8] - 0.3148342557928728*state[50] - 0.07906843296231456*state[65]);
+    v = v < lo0[8] ? lo0[8] : v; v = v > hi0[8] ? hi0[8] : v; t_t0[8] = v;
+    v = bias_t0[9] + (state[9] - 0.1012882957183817*state[50] - 0.3148342557928728*state[66]);
+    v = v < lo0[9] ? lo0[9] : v; v = v > hi0[9] ? hi0[9] : v; t_t0[9] = v;
+    v = bias_t0[10] + (state[10] - 0.3148342557928728*state[50] - 0.024432609086831557*state[69]);
+    v = v < lo0[10] ? lo0[10] : v; v = v > hi0[10] ? hi0[10] : v; t_t0[10] = v;
+    v = bias_t0[11] + (state[11] - 0.31483425576776075*state[50] - 0.14375068353499568*state[73]);
+    v = v < lo0[11] ? lo0[11] : v; v = v > hi0[11] ? hi0[11] : v; t_t0[11] = v;
+    v = bias_t0[12] + (state[12] - 0.16283459527310534*state[51] - 0.3148342557655042*state[59]);
+    v = v < lo0[12] ? lo0[12] : v; v = v > hi0[12] ? hi0[12] : v; t_t0[12] = v;
+    v = bias_t0[13] + (state[13] - 0.3148342557928728*state[51] - 0.057878934426039565*state[63]);
+    v = v < lo0[13] ? lo0[13] : v; v = v > hi0[13] ? hi0[13] : v; t_t0[13] = v;
+    v = bias_t0[14] + (state[14] - 0.3148342557928728*state[51] - 0.020371967614988162*state[65]);
+    v = v < lo0[14] ? lo0[14] : v; v = v > hi0[14] ? hi0[14] : v; t_t0[14] = v;
+    v = bias_t0[15] + (state[15] - 0.13195167489181564*state[51] - 0.31483425579287283*state[73]);
+    v = v < lo0[15] ? lo0[15] : v; v = v > hi0[15] ? hi0[15] : v; t_t0[15] = v;
+    v = bias_t0[16] + (state[16] - 0.20484874346487622*state[52] - 0.31483425569820117*state[59]);
+    v = v < lo0[16] ? lo0[16] : v; v = v > hi0[16] ? hi0[16] : v; t_t0[16] = v;
+    v = bias_t0[17] + (state[17] - 0.3148342557928728*state[52] - 0.059210692177281046*state[61]);
+    v = v < lo0[17] ? lo0[17] : v; v = v > hi0[17] ? hi0[17] : v; t_t0[17] = v;
+    v = bias_t0[18] + (state[18] - 0.3148342557928728*state[52] - 0.04186021818779605*state[63]);
+    v = v < lo0[18] ? lo0[18] : v; v = v > hi0[18] ? hi0[18] : v; t_t0[18] = v;
+    v = bias_t0[19] + (state[19] - 0.2013575399297713*state[52] - 0.3148342556220557*state[64]);
+    v = v < lo0[19] ? lo0[19] : v; v = v > hi0[19] ? hi0[19] : v; t_t0[19] = v;
+    v = bias_t0[20] + (state[20] - 0.1717002983294537*state[52] - 0.31483425579287283*state[65]);
+    v = v < lo0[20] ? lo0[20] : v; v = v > hi0[20] ? hi0[20] : v; t_t0[20] = v;
+    v = bias_t0[21] + (state[21] - 0.3148342557928728*state[52] - 0.09586520152267977*state[70]);
+    v = v < lo0[21] ? lo0[21] : v; v = v > hi0[21] ? hi0[21] : v; t_t0[21] = v;
+    v = bias_t0[22] + (state[22] - 0.3148342557928728*state[52] - 0.03542869495158811*state[73]);
+    v = v < lo0[22] ? lo0[22] : v; v = v > hi0[22] ? hi0[22] : v; t_t0[22] = v;
+    v = bias_t0[23] + (state[23] - 0.11549111655165208*state[53] - 0.3148342557928728*state[62]);
+    v = v < lo0[23] ? lo0[23] : v; v = v > hi0[23] ? hi0[23] : v; t_t0[23] = v;
+    v = bias_t0[24] + (state[24] - 0.3148342557928728*state[53] - 0.023238864598874573*state[68]);
+    v = v < lo0[24] ? lo0[24] : v; v = v > hi0[24] ? hi0[24] : v; t_t0[24] = v;
+    v = bias_t0[25] + (state[25] - 0.22703157183385472*state[53] - 0.31483425579287283*state[70]);
+    v = v < lo0[25] ? lo0[25] : v; v = v > hi0[25] ? hi0[25] : v; t_t0[25] = v;
+    v = bias_t0[26] + (state[26] - 0.3148342557928728*state[54] - 0.06685962708718948*state[59]);
+    v = v < lo0[26] ? lo0[26] : v; v = v > hi0[26] ? hi0[26] : v; t_t0[26] = v;
+    v = bias_t0[27] + (state[27] - 0.31483425578928326*state[54] - 0.1807094243289255*state[60]);
+    v = v < lo0[27] ? lo0[27] : v; v = v > hi0[27] ? hi0[27] : v; t_t0[27] = v;
+    v = bias_t0[28] + (state[28] - 0.11812566027056481*state[54] - 0.31483425579287283*state[63]);
+    v = v < lo0[28] ? lo0[28] : v; v = v > hi0[28] ? hi0[28] : v; t_t0[28] = v;
+    v = bias_t0[29] + (state[29] - 0.3148342557928728*state[54] - 0.06983942457332966*state[64]);
+    v = v < lo0[29] ? lo0[29] : v; v = v > hi0[29] ? hi0[29] : v; t_t0[29] = v;
+    v = bias_t0[30] + (state[30] - 0.3148342557928728*state[54] - 0.01829049354312028*state[66]);
+    v = v < lo0[30] ? lo0[30] : v; v = v > hi0[30] ? hi0[30] : v; t_t0[30] = v;
+    v = bias_t0[31] + (state[31] - 0.11621549871906076*state[54] - 0.3148342557928728*state[69]);
+    v = v < lo0[31] ? lo0[31] : v; v = v > hi0[31] ? hi0[31] : v; t_t0[31] = v;
+    v = bias_t0[32] + (state[32] - 0.3148342557928728*state[54] - 0.04147423462601573*state[71]);
+    v = v < lo0[32] ? lo0[32] : v; v = v > hi0[32] ? hi0[32] : v; t_t0[32] = v;
+    v = bias_t0[33] + (state[33] - 0.3148342557928728*state[54] - 0.029904608409446035*state[72]);
+    v = v < lo0[33] ? lo0[33] : v; v = v > hi0[33] ? hi0[33] : v; t_t0[33] = v;
+    v = bias_t0[34] + (state[34] - 0.2007269945571799*state[55] - 0.3148342557928728*state[68]);
+    v = v < lo0[34] ? lo0[34] : v; v = v > hi0[34] ? hi0[34] : v; t_t0[34] = v;
+    v = bias_t0[35] + (state[35] - 0.31483425579287283*state[55] - 0.26432062531400063*state[69]);
+    v = v < lo0[35] ? lo0[35] : v; v = v > hi0[35] ? hi0[35] : v; t_t0[35] = v;
+    v = bias_t0[36] + (state[36] - 0.20810320529794363*state[55] - 0.31483425575411544*state[73]);
+    v = v < lo0[36] ? lo0[36] : v; v = v > hi0[36] ? hi0[36] : v; t_t0[36] = v;
+    v = bias_t0[37] + (state[37] - 0.3148342557928728*state[56] - 0.050057390438124606*state[60]);
+    v = v < lo0[37] ? lo0[37] : v; v = v > hi0[37] ? hi0[37] : v; t_t0[37] = v;
+    v = bias_t0[38] + (state[38] - 0.3148342557928728*state[56] - 0.022355820755170213*state[66]);
+    v = v < lo0[38] ? lo0[38] : v; v = v > hi0[38] ? hi0[38] : v; t_t0[38] = v;
+    v = bias_t0[39] + (state[39] - 0.14832333169428388*state[57] - 0.3148342557928728*state[59]);
+    v = v < lo0[39] ? lo0[39] : v; v = v > hi0[39] ? hi0[39] : v; t_t0[39] = v;
+    v = bias_t0[40] + (state[40] - 0.25687797426285774*state[57] - 0.3148342557740812*state[67]);
+    v = v < lo0[40] ? lo0[40] : v; v = v > hi0[40] ? hi0[40] : v; t_t0[40] = v;
+    v = bias_t0[41] + (state[41] - 0.3148342556269916*state[57] - 0.2046259837099301*state[69]);
+    v = v < lo0[41] ? lo0[41] : v; v = v > hi0[41] ? hi0[41] : v; t_t0[41] = v;
+    v = bias_t0[42] + (state[42] - 0.3148342557928728*state[57] - 0.0669377754137904*state[71]);
+    v = v < lo0[42] ? lo0[42] : v; v = v > hi0[42] ? hi0[42] : v; t_t0[42] = v;
+    v = bias_t0[43] + (state[43] - 0.31483425573999474*state[57] - 0.2549419703939111*state[72]);
+    v = v < lo0[43] ? lo0[43] : v; v = v > hi0[43] ? hi0[43] : v; t_t0[43] = v;
+    v = bias_t0[44] + (state[44] - 0.3148342557928728*state[58] - 0.063379996679072*state[65]);
+    v = v < lo0[44] ? lo0[44] : v; v = v > hi0[44] ? hi0[44] : v; t_t0[44] = v;
+    v = bias_t0[45] + (state[45] - 0.2409314392941177*state[58] - 0.31483425579287283*state[67]);
+    v = v < lo0[45] ? lo0[45] : v; v = v > hi0[45] ? hi0[45] : v; t_t0[45] = v;
+    v = bias_t0[46] + (state[46] - 0.15802369728139173*state[58] - 0.3148342557928728*state[71]);
+    v = v < lo0[46] ? lo0[46] : v; v = v > hi0[46] ? hi0[46] : v; t_t0[46] = v;
+    v = bias_t0[47] + (state[47] - 0.21287292492070603*state[58] - 0.3148342557928728*state[72]);
+    v = v < lo0[47] ? lo0[47] : v; v = v > hi0[47] ? hi0[47] : v; t_t0[47] = v;
+    v = bias_t0[48] + (state[48] - 0.2746827012680003*state[58] - 0.3148342555778952*state[73]);
+    v = v < lo0[48] ? lo0[48] : v; v = v > hi0[48] ? hi0[48] : v; t_t0[48] = v;
+    v = bias_t1[0] + (-0.31483425579287283*state[0] - 0.17851063193927913*state[1] - 0.3148342556510004*state[2] - 0.31483425579287283*state[3] - 0.31483425579287283*state[4] + state[49]) + sc_xi * (-0.6296685115857453*state[0] - 0.3570212638785581*state[1] - 0.6296685113020006*state[2] - 0.6296685115857453*state[3] - 0.6296685115857453*state[4]) + (0.6296685115857455*t_t0[0] + 0.35702126387855826*t_t0[1] + 0.6296685113020011*t_t0[2] + 0.6296685115857455*t_t0[3] + 0.6296685115857455*t_t0[4]) + sc_xi * (0.6296685115857453*t_t0[0] + 0.3570212638785581*t_t0[1] + 0.6296685113020003*t_t0[2] + 0.6296685115857453*t_t0[3] + 0.6296685115857453*t_t0[4]);
+    t_t1[0] = v < lo1[0] ? lo1[0] : v;
+    v = bias_t1[1] + (-0.31483425579287283*state[5] - 0.11490990424160037*state[6] - 0.11245022456106753*state[7] - 0.31483425579287283*state[8] - 0.10128829571838176*state[9] - 0.31483425579287283*state[10] - 0.3148342557677609*state[11] + state[50]) + sc_xi * (-0.6296685115857453*state[5] - 0.2298198084832004*state[6] - 0.22490044912213478*state[7] - 0.6296685115857453*state[8] - 0.2025765914367633*state[9] - 0.6296685115857453*state[10] - 0.6296685115355208*state[11]) + (0.6296685115857455*t_t0[5] + 0.22981980848320072*t_t0[6] + 0.22490044912213497*t_t0[7] + 0.6296685115857455*t_t0[8] + 0.20257659143676343*t_t0[9] + 0.6296685115857455*t_t0[10] + 0.6296685115355216*t_t0[11]) + sc_xi * (0.6296685115857453*t_t0[5] + 0.22981980848320033*t_t0[6] + 0.22490044912213478*t_t0[7] + 0.6296685115857453*t_t0[8] + 0.2025765914367634*t_t0[9] + 0.6296685115857453*t_t0[10] + 0.6296685115355208*t_t0[11]);
+    t_t1[1] = v < lo1[1] ? lo1[1] : v;
+    v = bias_t1[2] + (-0.16283459527310534*state[12] - 0.31483425579287283*state[13] - 0.31483425579287283*state[14] - 0.1319516748918157*state[15] + state[51]) + sc_xi * (-0.32566919054621074*state[12] - 0.6296685115857453*state[13] - 0.6296685115857453*state[14] - 0.26390334978363106*state[15]) + (0.3256691905462108*t_t0[12] + 0.6296685115857455*t_t0[13] + 0.6296685115857455*t_t0[14] + 0.2639033497836314*t_t0[15]) + sc_xi * (0.3256691905462105*t_t0[12] + 0.6296685115857453*t_t0[13] + 0.6296685115857453*t_t0[14] + 0.26390334978363095*t_t0[15]);
+    t_t1[2] = v < lo1[2] ? lo1[2] : v;
+    v = bias_t1[3] + (-0.2048487434648763*state[16] - 0.31483425579287283*state[17] - 0.31483425579287283*state[18] - 0.20135753992977132*state[19] - 0.17170029832945377*state[20] - 0.31483425579287283*state[21] - 0.31483425579287283*state[22] + state[52]) + sc_xi * (-0.40969748692975216*state[16] - 0.6296685115857453*state[17] - 0.6296685115857453*state[18] - 0.40271507985954247*state[19] - 0.34340059665890715*state[20] - 0.6296685115857453*state[21] - 0.6296685115857453*state[22]) + (0.4096974869297525*t_t0[16] + 0.6296685115857455*t_t0[17] + 0.6296685115857455*t_t0[18] + 0.4027150798595428*t_t0[19] + 0.3434005966589075*t_t0[20] + 0.6296685115857455*t_t0[21] + 0.6296685115857455*t_t0[22]) + sc_xi * (0.4096974869297524*t_t0[16] + 0.6296685115857453*t_t0[17] + 0.6296685115857453*t_t0[18] + 0.402715079859542*t_t0[19] + 0.34340059665890715*t_t0[20] + 0.6296685115857453*t_t0[21] + 0.6296685115857453*t_t0[22]);
+    t_t1[3] = v < lo1[3] ? lo1[3] : v;
+    v = bias_t1[4] + (-0.11549111655165209*state[23] - 0.31483425579287283*state[24] - 0.2270315718338548*state[25] + state[53]) + sc_xi * (-0.23098223310330407*state[23] - 0.6296685115857453*state[24] - 0.45406314366770917*state[25]) + (0.2309822331033042*t_t0[23] + 0.6296685115857455*t_t0[24] + 0.4540631436677096*t_t0[25]) + sc_xi * (0.23098223310330407*t_t0[23] + 0.6296685115857453*t_t0[24] + 0.45406314366770895*t_t0[25]);
+    t_t1[4] = v < lo1[4] ? lo1[4] : v;
+    v = bias_t1[5] + (-0.31483425579287283*state[26] - 0.3148342557892835*state[27] - 0.11812566027056481*state[28] - 0.31483425579287283*state[29] - 0.31483425579287283*state[30] - 0.11621549871906077*state[31] - 0.31483425579287283*state[32] - 0.31483425579287283*state[33] + state[54]) + sc_xi * (-0.6296685115857453*state[26] - 0.6296685115785658*state[27] - 0.23625132054112963*state[28] - 0.6296685115857453*state[29] - 0.6296685115857453*state[30] - 0.2324309974381214*state[31] - 0.6296685115857453*state[32] - 0.6296685115857453*state[33]) + (0.6296685115857455*t_t0[26] + 0.6296685115785666*t_t0[27] + 0.23625132054112963*t_t0[28] + 0.6296685115857455*t_t0[29] + 0.6296685115857455*t_t0[30] + 0.23243099743812146*t_t0[31] + 0.6296685115857455*t_t0[32] + 0.6296685115857455*t_t0[33]) + sc_xi * (0.6296685115857453*t_t0[26] + 0.6296685115785663*t_t0[27] + 0.23625132054112963*t_t0[28] + 0.6296685115857453*t_t0[29] + 0.6296685115857453*t_t0[30] + 0.2324309974381216*t_t0[31] + 0.6296685115857453*t_t0[32] + 0.6296685115857453*t_t0[33]);
+    t_t1[5] = v < lo1[5] ? lo1[5] : v;
+    v = bias_t1[6] + (-0.2007269945571799*state[34] - 0.31483425579287294*state[35] - 0.20810320529794374*state[36] + state[55]) + sc_xi * (-0.40145398911435987*state[34] - 0.6296685115857453*state[35] - 0.41620641059588687*state[36]) + (0.4014539891143599*t_t0[34] + 0.6296685115857457*t_t0[35] + 0.41620641059588726*t_t0[36]) + sc_xi * (0.40145398911435964*t_t0[34] + 0.6296685115857458*t_t0[35] + 0.4162064105958871*t_t0[36]);
+    t_t1[6] = v < lo1[6] ? lo1[6] : v;
+    v = bias_t1[7] + (-0.31483425579287283*state[37] - 0.31483425579287283*state[38] + state[56]) + sc_xi * (-0.6296685115857453*state[37] - 0.6296685115857453*state[38]) + (0.6296685115857455*t_t0[37] + 0.6296685115857455*t_t0[38]) + sc_xi * (0.6296685115857453*t_t0[37] + 0.6296685115857453*t_t0[38]);
+    t_t1[7] = v < lo1[7] ? lo1[7] : v;
+    v = bias_t1[8] + (-0.14832333169428388*state[39] - 0.25687797426285774*state[40] - 0.31483425562699185*state[41] - 0.31483425579287283*state[42] - 0.3148342557399948*state[43] + state[57]) + sc_xi * (-0.2966466633885678*state[39] - 0.5137559485257154*state[40] - 0.6296685112539826*state[41] - 0.6296685115857453*state[42] - 0.629668511479989*state[43]) + (0.29664666338856793*t_t0[39] + 0.5137559485257156*t_t0[40] + 0.6296685112539833*t_t0[41] + 0.6296685115857455*t_t0[42] + 0.6296685114799897*t_t0[43]) + sc_xi * (0.2966466633885674*t_t0[39] + 0.5137559485257152*t_t0[40] + 0.629668511253983*t_t0[41] + 0.6296685115857453*t_t0[42] + 0.6296685114799887*t_t0[43]);
+    t_t1[8] = v < lo1[8] ? lo1[8] : v;
+    v = bias_t1[9] + (-0.31483425579287283*state[44] - 0.2409314392941178*state[45] - 0.15802369728139182*state[46] - 0.21287292492070606*state[47] - 0.2746827012680003*state[48] + state[58]) + sc_xi * (-0.6296685115857453*state[44] - 0.481862878588235*state[45] - 0.31604739456278325*state[46] - 0.425745849841412*state[47] - 0.5493654025360006*state[48]) + (0.6296685115857455*t_t0[44] + 0.48186287858823557*t_t0[45] + 0.3160473945627835*t_t0[46] + 0.42574584984141206*t_t0[47] + 0.5493654025360007*t_t0[48]) + sc_xi * (0.6296685115857453*t_t0[44] + 0.481862878588235*t_t0[45] + 0.31604739456278325*t_t0[46] + 0.425745849841412*t_t0[47] + 0.5493654025360004*t_t0[48]);
+    t_t1[9] = v < lo1[9] ? lo1[9] : v;
+    new_state[0] = bias_su[0] + (t_t0[0]);
+    new_state[1] = bias_su[1] + (t_t0[1]);
+    new_state[2] = bias_su[2] + (t_t0[2]);
+    new_state[3] = bias_su[3] + (t_t0[3]);
+    new_state[4] = bias_su[4] + (t_t0[4]);
+    new_state[5] = bias_su[5] + (t_t0[5]);
+    new_state[6] = bias_su[6] + (t_t0[6]);
+    new_state[7] = bias_su[7] + (t_t0[7]);
+    new_state[8] = bias_su[8] + (t_t0[8]);
+    new_state[9] = bias_su[9] + (t_t0[9]);
+    new_state[10] = bias_su[10] + (t_t0[10]);
+    new_state[11] = bias_su[11] + (t_t0[11]);
+    new_state[12] = bias_su[12] + (t_t0[12]);
+    new_state[13] = bias_su[13] + (t_t0[13]);
+    new_state[14] = bias_su[14] + (t_t0[14]);
+    new_state[15] = bias_su[15] + (t_t0[15]);
+    new_state[16] = bias_su[16] + (t_t0[16]);
+    new_state[17] = bias_su[17] + (t_t0[17]);
+    new_state[18] = bias_su[18] + (t_t0[18]);
+    new_state[19] = bias_su[19] + (t_t0[19]);
+    new_state[20] = bias_su[20] + (t_t0[20]);
+    new_state[21] = bias_su[21] + (t_t0[21]);
+    new_state[22] = bias_su[22] + (t_t0[22]);
+    new_state[23] = bias_su[23] + (t_t0[23]);
+    new_state[24] = bias_su[24] + (t_t0[24]);
+    new_state[25] = bias_su[25] + (t_t0[25]);
+    new_state[26] = bias_su[26] + (t_t0[26]);
+    new_state[27] = bias_su[27] + (t_t0[27]);
+    new_state[28] = bias_su[28] + (t_t0[28]);
+    new_state[29] = bias_su[29] + (t_t0[29]);
+    new_state[30] = bias_su[30] + (t_t0[30]);
+    new_state[31] = bias_su[31] + (t_t0[31]);
+    new_state[32] = bias_su[32] + (t_t0[32]);
+    new_state[33] = bias_su[33] + (t_t0[33]);
+    new_state[34] = bias_su[34] + (t_t0[34]);
+    new_state[35] = bias_su[35] + (t_t0[35]);
+    new_state[36] = bias_su[36] + (t_t0[36]);
+    new_state[37] = bias_su[37] + (t_t0[37]);
+    new_state[38] = bias_su[38] + (t_t0[38]);
+    new_state[39] = bias_su[39] + (t_t0[39]);
+    new_state[40] = bias_su[40] + (t_t0[40]);
+    new_state[41] = bias_su[41] + (t_t0[41]);
+    new_state[42] = bias_su[42] + (t_t0[42]);
+    new_state[43] = bias_su[43] + (t_t0[43]);
+    new_state[44] = bias_su[44] + (t_t0[44]);
+    new_state[45] = bias_su[45] + (t_t0[45]);
+    new_state[46] = bias_su[46] + (t_t0[46]);
+    new_state[47] = bias_su[47] + (t_t0[47]);
+    new_state[48] = bias_su[48] + (t_t0[48]);
+    new_state[49] = bias_su[49] + (t_t1[0]);
+    new_state[50] = bias_su[50] + (t_t1[1]);
+    new_state[51] = bias_su[51] + (t_t1[2]);
+    new_state[52] = bias_su[52] + (t_t1[3]);
+    new_state[53] = bias_su[53] + (t_t1[4]);
+    new_state[54] = bias_su[54] + (t_t1[5]);
+    new_state[55] = bias_su[55] + (t_t1[6]);
+    new_state[56] = bias_su[56] + (t_t1[7]);
+    new_state[57] = bias_su[57] + (t_t1[8]);
+    new_state[58] = bias_su[58] + (t_t1[9]);
+    new_state[59] = bias_su[59] + (-0.12559068930987088*state[0] - 0.31483425576550444*state[12] - 0.3148342556982012*state[16] - 0.0668596270871895*state[26] - 0.31483425579287283*state[39] + state[59]) + sc_xi * (-0.25118137861974177*state[0] - 0.6296685115310079*state[12] - 0.6296685113964021*state[16] - 0.1337192541743789*state[26] - 0.6296685115857453*state[39]) + (0.2511813786197419*t_t0[0] + 0.6296685115310086*t_t0[12] + 0.6296685113964026*t_t0[16] + 0.13371925417437902*t_t0[26] + 0.6296685115857455*t_t0[39]) + sc_xi * (0.25118137861974166*t_t0[0] + 0.6296685115310079*t_t0[12] + 0.6296685113964017*t_t0[16] + 0.13371925417437883*t_t0[26] + 0.6296685115857453*t_t0[39]);
+    new_state[60] = bias_su[60] + (-0.31483425579287294*state[1] - 0.08155612991849118*state[5] - 0.18070942432892556*state[27] - 0.05005739043812461*state[37] + state[60]) + sc_xi * (-0.6296685115857453*state[1] - 0.16311225983698213*state[5] - 0.361418848657851*state[27] - 0.10011478087624918*state[37]) + (0.6296685115857457*t_t0[1] + 0.16311225983698227*t_t0[5] + 0.36141884865785107*t_t0[27] + 0.1001147808762492*t_t0[37]) + sc_xi * (0.6296685115857458*t_t0[1] + 0.16311225983698224*t_t0[5] + 0.361418848657851*t_t0[27] + 0.10011478087624924*t_t0[37]);
+    new_state[61] = bias_su[61] + (-0.31483425579287294*state[6] - 0.05921069217728105*state[17] + state[61]) + sc_xi * (-0.6296685115857453*state[6] - 0.11842138435456206*state[17]) + (0.6296685115857457*t_t0[6] + 0.11842138435456212*t_t0[17]) + sc_xi * (0.6296685115857458*t_t0[6] + 0.11842138435456201*t_t0[17]);
+    new_state[62] = bias_su[62] + (-0.18736322506593045*state[2] - 0.31483425579287283*state[23] + state[62]) + sc_xi * (-0.37472645013186096*state[2] - 0.6296685115857453*state[23]) + (0.3747264501318611*t_t0[2] + 0.6296685115857455*t_t0[23]) + sc_xi * (0.3747264501318605*t_t0[2] + 0.6296685115857453*t_t0[23]);
+    new_state[63] = bias_su[63] + (-0.05787893442603958*state[13] - 0.04186021818779605*state[18] - 0.31483425579287294*state[28] + state[63]) + sc_xi * (-0.11575786885207907*state[13] - 0.08372043637559205*state[18] - 0.6296685115857453*state[28]) + (0.11575786885207912*t_t0[13] + 0.08372043637559212*t_t0[18] + 0.6296685115857457*t_t0[28]) + sc_xi * (0.11575786885207907*t_t0[13] + 0.083720436375592*t_t0[18] + 0.6296685115857458*t_t0[28]);
+    new_state[64] = bias_su[64] + (-0.31483425579287283*state[7] - 0.31483425562205586*state[19] - 0.06983942457332967*state[29] + state[64]) + sc_xi * (-0.6296685115857453*state[7] - 0.6296685112441109*state[19] - 0.13967884914665926*state[29]) + (0.6296685115857455*t_t0[7] + 0.6296685112441113*t_t0[19] + 0.13967884914665932*t_t0[29]) + sc_xi * (0.6296685115857453*t_t0[7] + 0.6296685112441114*t_t0[19] + 0.13967884914665926*t_t0[29]);
+    new_state[65] = bias_su[65] + (-0.05210541922399309*state[3] - 0.07906843296231458*state[8] - 0.02037196761498816*state[14] - 0.31483425579287294*state[20] - 0.06337999667907203*state[44] + state[65]) + sc_xi * (-0.10421083844798612*state[3] - 0.15813686592462906*state[8] - 0.040743935229976325*state[14] - 0.6296685115857453*state[20] - 0.12675999335814395*state[44]) + (0.10421083844798616*t_t0[3] + 0.1581368659246291*t_t0[8] + 0.04074393522997634*t_t0[14] + 0.6296685115857457*t_t0[20] + 0.12675999335814403*t_t0[44]) + sc_xi * (0.10421083844798612*t_t0[3] + 0.15813686592462906*t_t0[8] + 0.040743935229976304*t_t0[14] + 0.6296685115857458*t_t0[20] + 0.12675999335814397*t_t0[44]);
+    new_state[66] = bias_su[66] + (-0.31483425579287283*state[9] - 0.01829049354312029*state[30] - 0.022355820755170213*state[38] + state[66]) + sc_xi * (-0.6296685115857453*state[9] - 0.03658098708624052*state[30] - 0.04471164151034042*state[38]) + (0.6296685115857455*t_t0[9] + 0.03658098708624057*t_t0[30] + 0.04471164151034043*t_t0[38]) + sc_xi * (0.6296685115857453*t_t0[9] + 0.03658098708624052*t_t0[30] + 0.0447116415103404*t_t0[38]);
+    new_state[67] = bias_su[67] + (-0.3148342557740813*state[40] - 0.31483425579287294*state[45] + state[67]) + sc_xi * (-0.629668511548162*state[40] - 0.6296685115857453*state[45]) + (0.6296685115481624*t_t0[40] + 0.6296685115857457*t_t0[45]) + sc_xi * (0.6296685115481624*t_t0[40] + 0.6296685115857458*t_t0[45]);
+    new_state[68] = bias_su[68] + (-0.02323886459887458*state[24] - 0.31483425579287283*state[34] + state[68]) + sc_xi * (-0.04647772919774914*state[24] - 0.6296685115857453*state[34]) + (0.04647772919774916*t_t0[24] + 0.6296685115857455*t_t0[34]) + sc_xi * (0.04647772919774911*t_t0[24] + 0.6296685115857453*t_t0[34]);
+    new_state[69] = bias_su[69] + (-0.024432609086831564*state[10] - 0.31483425579287283*state[31] - 0.26432062531400075*state[35] - 0.20462598370993018*state[41] + state[69]) + sc_xi * (-0.0488652181736631*state[10] - 0.6296685115857453*state[31] - 0.5286412506280009*state[35] - 0.4092519674198599*state[41]) + (0.04886521817366314*t_t0[10] + 0.6296685115857455*t_t0[31] + 0.5286412506280014*t_t0[35] + 0.40925196741986025*t_t0[41]) + sc_xi * (0.04886521817366307*t_t0[10] + 0.6296685115857453*t_t0[31] + 0.5286412506280012*t_t0[35] + 0.40925196741986014*t_t0[41]);
+    new_state[70] = bias_su[70] + (-0.09586520152267985*state[21] - 0.31483425579287294*state[25] + state[70]) + sc_xi * (-0.19173040304535932*state[21] - 0.6296685115857453*state[25]) + (0.1917304030453596*t_t0[21] + 0.6296685115857457*t_t0[25]) + sc_xi * (0.19173040304535943*t_t0[21] + 0.6296685115857458*t_t0[25]);
+    new_state[71] = bias_su[71] + (-0.042197122740314105*state[4] - 0.04147423462601575*state[32] - 0.0669377754137904*state[42] - 0.31483425579287283*state[46] + state[71]) + sc_xi * (-0.08439424548062809*state[4] - 0.08294846925203142*state[32] - 0.1338755508275808*state[42] - 0.6296685115857453*state[46]) + (0.08439424548062816*t_t0[4] + 0.08294846925203149*t_t0[32] + 0.1338755508275808*t_t0[42] + 0.6296685115857455*t_t0[46]) + sc_xi * (0.08439424548062814*t_t0[4] + 0.08294846925203136*t_t0[32] + 0.1338755508275808*t_t0[42] + 0.6296685115857453*t_t0[46]);
+    new_state[72] = bias_su[72] + (-0.02990460840944605*state[33] - 0.25494197039391125*state[43] - 0.31483425579287283*state[47] + state[72]) + sc_xi * (-0.05980921681889204*state[33] - 0.5098839407878217*state[43] - 0.6296685115857453*state[47]) + (0.05980921681889208*t_t0[33] + 0.5098839407878222*t_t0[43] + 0.6296685115857455*t_t0[47]) + sc_xi * (0.05980921681889207*t_t0[33] + 0.509883940787822*t_t0[43] + 0.6296685115857453*t_t0[47]);
+    new_state[73] = bias_su[73] + (-0.14375068353499568*state[11] - 0.31483425579287294*state[15] - 0.03542869495158812*state[22] - 0.3148342557541156*state[36] - 0.31483425557789524*state[48] + state[73]) + sc_xi * (-0.2875013670699913*state[11] - 0.6296685115857453*state[15] - 0.0708573899031762*state[22] - 0.6296685115082304*state[36] - 0.6296685111557904*state[48]) + (0.28750136706999146*t_t0[11] + 0.6296685115857457*t_t0[15] + 0.07085738990317625*t_t0[22] + 0.6296685115082311*t_t0[36] + 0.6296685111557906*t_t0[48]) + sc_xi * (0.28750136706999097*t_t0[11] + 0.6296685115857458*t_t0[15] + 0.07085738990317617*t_t0[22] + 0.6296685115082304*t_t0[36] + 0.6296685111557899*t_t0[48]);
+}
```

## Agent notes

```
### Round 1

**Whole-solve mean_us:** 62.11 µs at the start, about 19.5 µs at the end (`make bench` runs gave 19.2 to 20.0 µs). That is a **~3.2× speedup**.

**Final profile breakdown (`make profile`, 5000 solves × 200 iterations):**
| region | total_us | calls | pct |
|---|---|---|---|
| iterate (`fom_iterate`: t0 clip step, t1 relu step, state update) | 99442 | 1000000 | 100.0% |

The residual is computed only on the last iteration, and the parameter term `su_p_p0 @ p0` is computed once per solve, so neither shows up in the profile. (The profiling build itself ran at 24.5 µs per solve.)

**Optimizations applied:**
1. **Fully unrolled `fom_iterate` with the coefficients written as literals** (allowed: `fom_iterate` is on the unit-vector-checked list). The structure exploited is a fixed, small sparsity pattern: 147 + 4×49 + 59 + 64 + 98 + 10 nonzeros in total. Every CSR index load, row-pointer loop and inner-loop branch is gone. Unit coefficients are dropped from the multiplications, so selection rows become plain adds. The products of each block are merged into one expression per row, with the `xi`-scaled members grouped under a single `sc_xi *`. The clip and relu activations are applied inline with the passed `act` bounds (never hardcoded), and the result is written straight to its destination with no zeroing or copy passes.
2. **Selection rows and the `su_t_t1` shift folded in.** Rows 0..48 of the state update are `bias_su + t0`, and rows 49..58 are `bias_su + t1`. The generator emits these as single adds from the generic unrolled form.
3. **t0 and t1 kept in local arrays (`T0`, `T1`) inside `fom_iterate`.** Without them, possible aliasing between the output pointers and `state` forces values to be reloaded from memory. The `t_t0`/`t_t1` outputs are still written for the solver and the objective. This gave about 5–10%.
4. **Already in place from the translation:** the loop-invariant parameter term `su_p_p0 @ p0` is computed once per solve as `bias_su`, there is no per-iteration copy (pointer swap instead), and the residual is computed only on the final iteration (termination is max_iters only).

**Blocked by rules:**
- Precompute `t1_state + xi_k*t1_state__d_xi` (and the other `__d_xi` pairs) for each scheduled `xi` value, so each pair costs one product instead of two | src/fom_ops.c must not read the schedule; fom_setup/fom_update_params must not rebuild matrices; the fom_data.c schedule and matrix data are fixed | ~10-20%
- Compile with `-march=native` / `-ffast-math` so the compiler can reassociate and vectorize the unrolled sums | Makefile and build flags are fixed | unknown, maybe 10-20%

### Round 2

**Whole-solve time (`make bench` mean_us):** 19.355 us at the start of this round. It is now 13.2 to 13.9 us over four runs, about 13.5 us typical, so about 1.4x faster this round. The best single solve went from 16 to 11 us. Against the translated solver (62.488 us) that is about 4.6x overall. The bench is noisy here: two baseline runs on this machine gave 17.8 and 20.0 us.

**Profile of the final solver (`make profile`), hottest first:**
- iterate: 67502 us total over 1,000,000 calls, 100.00%. This is the whole iteration in one call to `fom_iterate`. The only other per-solve work is computing `su_p_p0 @ p0` once and the residual on the last iteration, which are too small to show up.

**Optimizations applied this round:**
- **Dropped the local copies in `fom_iterate`.** It used to keep private arrays `T0[49]` and `T1[10]` and store every value twice, once there and once in `t_t0`/`t_t1`. It now writes only to `t_t0`/`t_t1` and reads them back, which removes 59 extra stores per iteration.
- **Marked the pointers `restrict` in the `fom_iterate` definition.** This is valid because the solver always passes separate buffers for state, the biases, `t0`, `t1` and the new state. With that guarantee the compiler no longer reloads values after every store, and it can keep `t_t0` values in registers as it chains `t0` into `t1` into the state update. The header prototype is unchanged; adding `restrict` only in the definition is allowed in C.

**Already in place from earlier rounds, unchanged:**
- One fused, fully unrolled pass per iteration, with the matrix coefficients written inline.
- The selection rows and the shift selection written as direct copies.
- The parameter term computed once per solve.
- The residual computed only on the last iteration.
- The state and new-state buffers swapped each iteration instead of copied.

Blocked by rules:
- Write the `*__d_xi` coefficients as 2x the base coefficients, or fold the matching state and t0 coefficient pairs together, to save multiplies | verify checks each fused routine at every schedule value and against each member's exact coefficients, and those pairs agree only to about 1e-15, so the result is not exact | unknown, probably about 10-20% of the floating-point work
- Inline `fom_iterate` into the solve loop to save one call per iteration | `src/fom_solver.c` may not read matrix data or contain products | small, a few percent

### Round 3

**Result:** this round left the sources unchanged because nothing I tried gave a measurable gain.
- Initial `mean_us` was 13.863 (reported); my local runs today gave about 11.5–11.7.
- Final `mean_us` is the same solver: about 11.5–11.7 locally, 13.863 as reported.
- Speedup this round: 1.00x. Overall speedup vs. the translated solver (62.488 us) stays at 4.51x on the reported numbers.

**Final profile (`make profile`), hottest first:**
- iterate: 58900 us over 1,000,000 calls, 100.00%. That is about 59 ns per iteration with profiling on; the profiled BENCH run gives mean_us=16.13.

**What the solver already has from earlier rounds:**
- `fom_iterate` computes the whole iteration in one pass (t0, then t1, then the state update) and writes the temporaries and the new state directly. Valid because each step has a fixed sparsity pattern.
- Each t0 row is fully unrolled over its 3 nonzeros (an identity entry plus two coupling entries), with the clip folded into the same pass.
- Each t1 row is unrolled over its small block-sparse support, with the ReLU folded in.
- su rows 0..58 are selection rows: a plain bias plus copy of t_t0 and t_t1.
- su rows 59..73 are unrolled sparse rows.
- The parameter term `su_p_p0 @ p0` is computed once per solve, outside the loop.
- The residual is computed only on the last iteration, and state buffers are swapped by pointer instead of copied.

**Tried this round and reverted:**
- Merged the two `sc_xi * (...)` groups in each t1 and su row into one multiply (25 rows).
- It passed verify (worst error 5.3e-15), but alternating old/new benchmarks showed no difference beyond noise (11.50–11.75 vs. 11.51–11.71 us).

The loop is now about 400 flops and one t0 → t1 → state dependency chain per iteration, with every product unrolled. The remaining cost is close to the arithmetic and latency floor.

Blocked by rules:
- Write each t1 / su row with paired coefficients as `c*(t_t0[j] - state[j])`, using one shared coefficient for the matching state and t_t0 entries (they are negatives of each other up to about 1e-10, and 2x apart in the non-xi terms) | verify requires literals to equal each matrix entry exactly, far below a perturbed coefficient | about 10–20% fewer multiplies in t1/su, maybe 5% of the solve
- Build with `-ffast-math` or `-march=native` so the compiler can reassociate and vectorize the unrolled rows | Makefile and build flags are fixed | unknown, maybe 5–15%
```
