# Optimization report

## Timing

```
translated (emitted)      42.3 us/solve
optimized                  8.5 us/solve
speedup                  4.99x
```

Optimization outcome: **kept (re-verified correct)**.

## Rounds

| round | before (us) | after (us) | gain | kept | agent (s) | outcome |
|---:|---:|---:|---:|:---:|---:|---|
| 1 | 42.342 | 8.957 | 78.8% | yes | 85 | continued |
| 2 | 8.913 | 8.491 | 4.7% | yes | 99 | continued |
| 3 | - | - | - | no | 38 | the round left the sources unchanged |

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
| 1 | Fold the 200 iterations so `t_t0`/`t_t1` are stored only on the last one | `fom_iterate` must write every temporary each call, and only `fom_iterate`/`fom_step`/`fom_fused` may hold the products | small, about 3% |
| 1 | Hoist `bias_su[0..58]` into `bias_t0`/`bias_t1` so the copy rows need no add | `fom_iterate` must be exact for arbitrary independent biases, and reading the constant vectors in `src/fom_ops.c` is forbidden | small, about 2% |
| 2 | Skip writing `t_t0` and `t_t1` in non-final iterations, keeping them in registers | `fom_iterate` must write every temporary, and verify checks them | unknown, probably small (59 stores per iteration) |
| 2 | Fold `bias_su[0..58]` into the t0/t1 biases to drop one add per selection row | `fom_setup`/`fom_update_params` may not probe or rebuild matrices, and `bias_su` belongs to the update, not the step | small (under 2%) |
| 3 | Skip writing the `t_t0`/`t_t1` temporaries on every iteration except the last (about 59 stores per iteration) | `fom_iterate` must write every temporary each call, because verify checks them and the solver and objective need them | ~5% |
| 3 | Run the 200-iteration loop inside `src/fom_ops.c` so the state stays in registers across iterations instead of going through buffers | the interface is fixed: `fom_iterate` runs exactly one iteration, and `src/fom_solver.c` may not read matrix data | unknown, likely <5% |

## What the agent changed

```diff
--- src/fom_solver.c (translated)
+++ src/fom_solver.c (optimized)
@@ -1,6 +1,7 @@
 #include "fom_solver.h"
 #include "fom_data.h"
 #include "fom_ops.h"
+#include "fom_profile.h"
 
 void fom_set_default_settings(FomSettings *settings) {
     settings->max_iter = FOM_MAX_ITER;
@@ -32,13 +33,20 @@
     fom_float res = 0.0;
 
     /* Hoisted parameter term of the state update. */
-    for (i = 0; i < FOM_STATE_DIM; ++i) bias_su[i] = 0.0;
-    fom_apply_su_p_p0(work->params.p0, bias_su);
-
-    for (k = 0; k < n; ++k) {
-        fom_iterate(cur, FOM_MAT_t0_c, FOM_MAT_t1_c, bias_su, &FOM_ACT_t0,
-                    &FOM_ACT_t1, work->t0, work->t1, nxt);
-        tmp = cur; cur = nxt; nxt = tmp;
+    {
+        FOM_PROF_BEGIN(param_bias);
+        for (i = 0; i < FOM_STATE_DIM; ++i) bias_su[i] = 0.0;
+        fom_apply_su_p_p0(work->params.p0, bias_su);
+        FOM_PROF_END(param_bias);
+    }
+    {
+        FOM_PROF_BEGIN(iterate);
+        for (k = 0; k < n; ++k) {
+            fom_iterate(cur, FOM_MAT_t0_c, FOM_MAT_t1_c, bias_su, &FOM_ACT_t0,
+                        &FOM_ACT_t1, work->t0, work->t1, nxt);
+            tmp = cur; cur = nxt; nxt = tmp;
+        }
+        FOM_PROF_END(iterate);
     }
     if (n > 0) {
         for (i = 0; i < FOM_STATE_DIM; ++i) {
--- src/fom_ops.c (translated)
+++ src/fom_ops.c (optimized)
@@ -1,3 +1,4 @@
+#include <math.h>
 #include "fom_ops.h"
 #include "fom_data.h"
 
@@ -180,8 +181,141 @@
     }
 }
 
-void fom_iterate(const fom_float *state, const fom_float *bias_t0, const fom_float *bias_t1, const fom_float *bias_su, const FomAct_t0 *act_t0, const FomAct_t1 *act_t1, fom_float *t_t0, fom_float *t_t1, fom_float *new_state) {
-    fom_step_t0(state, bias_t0, act_t0, t_t0);
-    fom_step_t1(state, t_t0, bias_t1, act_t1, t_t1);
-    fom_step_su(state, t_t0, t_t1, bias_su, new_state);
-}
+void fom_iterate(const fom_float *restrict state, const fom_float *restrict bias_t0, const fom_float *restrict bias_t1, const fom_float *restrict bias_su, const FomAct_t0 *act_t0, const FomAct_t1 *act_t1, fom_float *restrict t_t0, fom_float *restrict t_t1, fom_float *restrict new_state) {
+    const fom_float *restrict lo0 = act_t0->lo, *restrict hi0 = act_t0->hi, *restrict lo1 = act_t1->lo;
+    fom_float v;
+    fom_float w[49];
+    v = bias_t0[0] + state[0] - (0.08226303133234722) * state[49] - (0.069655061626067494) * state[59];
+    v = fmin(fmax(v, lo0[0]), hi0[0]); t_t0[0] = v; w[0] = (2.0 * v - state[0]); new_state[0] = bias_su[0] + v;
+    v = bias_t0[1] + state[1] - (0.08226303133234722) * state[49] - (0.25588244633413909) * state[60];
+    v = fmin(fmax(v, lo0[1]), hi0[1]); t_t0[1] = v; w[1] = (2.0 * v - state[1]); new_state[1] = bias_su[1] + v;
+    v = bias_t0[2] + state[2] - (0.08226303133234722) * state[49] - (0.13345673088354346) * state[62];
+    v = fmin(fmax(v, lo0[2]), hi0[2]); t_t0[2] = v; w[2] = (2.0 * v - state[2]); new_state[2] = bias_su[2] + v;
+    v = bias_t0[3] + state[3] - (0.08226303133234722) * state[49] - (0.024964136788999065) * state[65];
+    v = fmin(fmax(v, lo0[3]), hi0[3]); t_t0[3] = v; w[3] = (2.0 * v - state[3]); new_state[3] = bias_su[3] + v;
+    v = bias_t0[4] + state[4] - (0.08226303133234722) * state[49] - (0.021966725812901995) * state[71];
+    v = fmin(fmax(v, lo0[4]), hi0[4]); t_t0[4] = v; w[4] = (2.0 * v - state[4]); new_state[4] = bias_su[4] + v;
+    v = bias_t0[5] + state[5] - (0.08226303133234722) * state[50] - (0.037583500758161678) * state[60];
+    v = fmin(fmax(v, lo0[5]), hi0[5]); t_t0[5] = v; w[5] = (2.0 * v - state[5]); new_state[5] = bias_su[5] + v;
+    v = bias_t0[6] + state[6] - (0.08226303133234722) * state[50] - (0.61752386121652636) * state[61];
+    v = fmin(fmax(v, lo0[6]), hi0[6]); t_t0[6] = v; w[6] = (2.0 * v - state[6]); new_state[6] = bias_su[6] + v;
+    v = bias_t0[7] + state[7] - (0.08226303133234722) * state[50] - (0.64483412842709398) * state[64];
+    v = fmin(fmax(v, lo0[7]), hi0[7]); t_t0[7] = v; w[7] = (2.0 * v - state[7]); new_state[7] = bias_su[7] + v;
+    v = bias_t0[8] + state[8] - (0.08226303133234722) * state[50] - (0.037882339410371904) * state[65];
+    v = fmin(fmax(v, lo0[8]), hi0[8]); t_t0[8] = v; w[8] = (2.0 * v - state[8]); new_state[8] = bias_su[8] + v;
+    v = bias_t0[9] + state[9] - (0.08226303133234722) * state[50] - (0.79478587733064987) * state[66];
+    v = fmin(fmax(v, lo0[9]), hi0[9]); t_t0[9] = v; w[9] = (2.0 * v - state[9]); new_state[9] = bias_su[9] + v;
+    v = bias_t0[10] + state[10] - (0.08226303133234722) * state[50] - (0.017294599334807721) * state[69];
+    v = fmin(fmax(v, lo0[10]), hi0[10]); t_t0[10] = v; w[10] = (2.0 * v - state[10]); new_state[10] = bias_su[10] + v;
+    v = bias_t0[11] + state[11] - (0.08226303133234722) * state[50] - (0.089618922945269686) * state[73];
+    v = fmin(fmax(v, lo0[11]), hi0[11]); t_t0[11] = v; w[11] = (2.0 * v - state[11]); new_state[11] = bias_su[11] + v;
+    v = bias_t0[12] + state[12] - (0.08226303133234722) * state[51] - (0.33760783556686547) * state[59];
+    v = fmin(fmax(v, lo0[12]), hi0[12]); t_t0[12] = v; w[12] = (2.0 * v - state[12]); new_state[12] = bias_su[12] + v;
+    v = bias_t0[13] + state[13] - (0.08226303133234722) * state[51] - (0.040307047471874392) * state[63];
+    v = fmin(fmax(v, lo0[13]), hi0[13]); t_t0[13] = v; w[13] = (2.0 * v - state[13]); new_state[13] = bias_su[13] + v;
+    v = bias_t0[14] + state[14] - (0.08226303133234722) * state[51] - (0.00976037797556846) * state[65];
+    v = fmin(fmax(v, lo0[14]), hi0[14]); t_t0[14] = v; w[14] = (2.0 * v - state[14]); new_state[14] = bias_su[14] + v;
+    v = bias_t0[15] + state[15] - (0.08226303133234722) * state[51] - (0.46831584781193331) * state[73];
+    v = fmin(fmax(v, lo0[15]), hi0[15]); t_t0[15] = v; w[15] = (2.0 * v - state[15]); new_state[15] = bias_su[15] + v;
+    v = bias_t0[16] + state[16] - (0.08226303133234722) * state[52] - (0.26836501080727221) * state[59];
+    v = fmin(fmax(v, lo0[16]), hi0[16]); t_t0[16] = v; w[16] = (2.0 * v - state[16]); new_state[16] = bias_su[16] + v;
+    v = bias_t0[17] + state[17] - (0.08226303133234722) * state[52] - (0.042388435165245451) * state[61];
+    v = fmin(fmax(v, lo0[17]), hi0[17]); t_t0[17] = v; w[17] = (2.0 * v - state[17]); new_state[17] = bias_su[17] + v;
+    v = bias_t0[18] + state[18] - (0.08226303133234722) * state[52] - (0.029151569883073398) * state[63];
+    v = fmin(fmax(v, lo0[18]), hi0[18]); t_t0[18] = v; w[18] = (2.0 * v - state[18]); new_state[18] = bias_su[18] + v;
+    v = bias_t0[19] + state[19] - (0.08226303133234722) * state[52] - (0.36011436438990652) * state[64];
+    v = fmin(fmax(v, lo0[19]), hi0[19]); t_t0[19] = v; w[19] = (2.0 * v - state[19]); new_state[19] = bias_su[19] + v;
+    v = bias_t0[20] + state[20] - (0.08226303133234722) * state[52] - (0.27658369193168919) * state[65];
+    v = fmin(fmax(v, lo0[20]), hi0[20]); t_t0[20] = v; w[20] = (2.0 * v - state[20]); new_state[20] = bias_su[20] + v;
+    v = bias_t0[21] + state[21] - (0.08226303133234722) * state[52] - (0.034735970917354191) * state[70];
+    v = fmin(fmax(v, lo0[21]), hi0[21]); t_t0[21] = v; w[21] = (2.0 * v - state[21]); new_state[21] = bias_su[21] + v;
+    v = bias_t0[22] + state[22] - (0.08226303133234722) * state[52] - (0.022087418331416996) * state[73];
+    v = fmin(fmax(v, lo0[22]), hi0[22]); t_t0[22] = v; w[22] = (2.0 * v - state[22]); new_state[22] = bias_su[22] + v;
+    v = bias_t0[23] + state[23] - (0.08226303133234722) * state[53] - (0.61132408824693363) * state[62];
+    v = fmin(fmax(v, lo0[23]), hi0[23]); t_t0[23] = v; w[23] = (2.0 * v - state[23]); new_state[23] = bias_su[23] + v;
+    v = bias_t0[24] + state[24] - (0.08226303133234722) * state[53] - (0.013160484266981857) * state[68];
+    v = fmin(fmax(v, lo0[24]), hi0[24]); t_t0[24] = v; w[24] = (2.0 * v - state[24]); new_state[24] = bias_su[24] + v;
+    v = bias_t0[25] + state[25] - (0.08226303133234722) * state[53] - (0.15819624000738933) * state[70];
+    v = fmin(fmax(v, lo0[25]), hi0[25]); t_t0[25] = v; w[25] = (2.0 * v - state[25]); new_state[25] = bias_su[25] + v;
+    v = bias_t0[26] + state[26] - (0.08226303133234722) * state[54] - (0.037081661631488835) * state[59];
+    v = fmin(fmax(v, lo0[26]), hi0[26]); t_t0[26] = v; w[26] = (2.0 * v - state[26]); new_state[26] = bias_su[26] + v;
+    v = bias_t0[27] + state[27] - (0.08226303133234722) * state[54] - (0.083276300544647269) * state[60];
+    v = fmin(fmax(v, lo0[27]), hi0[27]); t_t0[27] = v; w[27] = (2.0 * v - state[27]); new_state[27] = bias_su[27] + v;
+    v = bias_t0[28] + state[28] - (0.08226303133234722) * state[54] - (0.58435958348428074) * state[63];
+    v = fmin(fmax(v, lo0[28]), hi0[28]); t_t0[28] = v; w[28] = (2.0 * v - state[28]); new_state[28] = bias_su[28] + v;
+    v = bias_t0[29] + state[29] - (0.08226303133234722) * state[54] - (0.051091074244933282) * state[64];
+    v = fmin(fmax(v, lo0[29]), hi0[29]); t_t0[29] = v; w[29] = (2.0 * v - state[29]); new_state[29] = bias_su[29] + v;
+    v = bias_t0[30] + state[30] - (0.08226303133234722) * state[54] - (0.014854938892497713) * state[66];
+    v = fmin(fmax(v, lo0[30]), hi0[30]); t_t0[30] = v; w[30] = (2.0 * v - state[30]); new_state[30] = bias_su[30] + v;
+    v = bias_t0[31] + state[31] - (0.08226303133234722) * state[54] - (0.60372695765338591) * state[69];
+    v = fmin(fmax(v, lo0[31]), hi0[31]); t_t0[31] = v; w[31] = (2.0 * v - state[31]); new_state[31] = bias_su[31] + v;
+    v = bias_t0[32] + state[32] - (0.08226303133234722) * state[54] - (0.021590409041307837) * state[71];
+    v = fmin(fmax(v, lo0[32]), hi0[32]); t_t0[32] = v; w[32] = (2.0 * v - state[32]); new_state[32] = bias_su[32] + v;
+    v = bias_t0[33] + state[33] - (0.08226303133234722) * state[54] - (0.011556395626565408) * state[72];
+    v = fmin(fmax(v, lo0[33]), hi0[33]); t_t0[33] = v; w[33] = (2.0 * v - state[33]); new_state[33] = bias_su[33] + v;
+    v = bias_t0[34] + state[34] - (0.08226303133234722) * state[55] - (0.38643208723267541) * state[68];
+    v = fmin(fmax(v, lo0[34]), hi0[34]); t_t0[34] = v; w[34] = (2.0 * v - state[34]); new_state[34] = bias_su[34] + v;
+    v = bias_t0[35] + state[35] - (0.08226303133234722) * state[55] - (0.25854119370877737) * state[69];
+    v = fmin(fmax(v, lo0[35]), hi0[35]); t_t0[35] = v; w[35] = (2.0 * v - state[35]); new_state[35] = bias_su[35] + v;
+    v = bias_t0[36] + state[36] - (0.08226303133234722) * state[55] - (0.41032977319887487) * state[73];
+    v = fmin(fmax(v, lo0[36]), hi0[36]); t_t0[36] = v; w[36] = (2.0 * v - state[36]); new_state[36] = bias_su[36] + v;
+    v = bias_t0[37] + state[37] - (0.08226303133234722) * state[56] - (0.023067940734351828) * state[60];
+    v = fmin(fmax(v, lo0[37]), hi0[37]); t_t0[37] = v; w[37] = (2.0 * v - state[37]); new_state[37] = bias_su[37] + v;
+    v = bias_t0[38] + state[38] - (0.08226303133234722) * state[56] - (0.018156664303604776) * state[66];
+    v = fmin(fmax(v, lo0[38]), hi0[38]); t_t0[38] = v; w[38] = (2.0 * v - state[38]); new_state[38] = bias_su[38] + v;
+    v = bias_t0[39] + state[39] - (0.08226303133234722) * state[57] - (0.37063781296154064) * state[59];
+    v = fmin(fmax(v, lo0[39]), hi0[39]); t_t0[39] = v; w[39] = (2.0 * v - state[39]); new_state[39] = bias_su[39] + v;
+    v = bias_t0[40] + state[40] - (0.08226303133234722) * state[57] - (0.13174930294003487) * state[67];
+    v = fmin(fmax(v, lo0[40]), hi0[40]); t_t0[40] = v; w[40] = (2.0 * v - state[40]); new_state[40] = bias_su[40] + v;
+    v = bias_t0[41] + state[41] - (0.08226303133234722) * state[57] - (0.14484430995648803) * state[69];
+    v = fmin(fmax(v, lo0[41]), hi0[41]); t_t0[41] = v; w[41] = (2.0 * v - state[41]); new_state[41] = bias_su[41] + v;
+    v = bias_t0[42] + state[42] - (0.08226303133234722) * state[57] - (0.034846066829944282) * state[71];
+    v = fmin(fmax(v, lo0[42]), hi0[42]); t_t0[42] = v; w[42] = (2.0 * v - state[42]); new_state[42] = bias_su[42] + v;
+    v = bias_t0[43] + state[43] - (0.08226303133234722) * state[57] - (0.098520275933536933) * state[72];
+    v = fmin(fmax(v, lo0[43]), hi0[43]); t_t0[43] = v; w[43] = (2.0 * v - state[43]); new_state[43] = bias_su[43] + v;
+    v = bias_t0[44] + state[44] - (0.08226303133234722) * state[58] - (0.03036587998612799) * state[65];
+    v = fmin(fmax(v, lo0[44]), hi0[44]); t_t0[44] = v; w[44] = (2.0 * v - state[44]); new_state[44] = bias_su[44] + v;
+    v = bias_t0[45] + state[45] - (0.08226303133234722) * state[58] - (0.14046939723165486) * state[67];
+    v = fmin(fmax(v, lo0[45]), hi0[45]); t_t0[45] = v; w[45] = (2.0 * v - state[45]); new_state[45] = bias_su[45] + v;
+    v = bias_t0[46] + state[46] - (0.08226303133234722) * state[58] - (0.32653085943273191) * state[71];
+    v = fmin(fmax(v, lo0[46]), hi0[46]); t_t0[46] = v; w[46] = (2.0 * v - state[46]); new_state[46] = bias_su[46] + v;
+    v = bias_t0[47] + state[47] - (0.08226303133234722) * state[58] - (0.17994004051645768) * state[72];
+    v = fmin(fmax(v, lo0[47]), hi0[47]); t_t0[47] = v; w[47] = (2.0 * v - state[47]); new_state[47] = bias_su[47] + v;
+    v = bias_t0[48] + state[48] - (0.08226303133234722) * state[58] - (0.22496888289546163) * state[73];
+    v = fmin(fmax(v, lo0[48]), hi0[48]); t_t0[48] = v; w[48] = (2.0 * v - state[48]); new_state[48] = bias_su[48] + v;
+    v = bias_t1[0] + state[49] + (0.08226303133234722) * (((w[0] + w[1]) + (w[2] + w[3])) + w[4]);
+    v = fmax(v, lo1[0]); t_t1[0] = v; new_state[49] = bias_su[49] + v;
+    v = bias_t1[1] + state[50] + (0.08226303133234722) * (((w[5] + w[6]) + (w[7] + w[8])) + ((w[9] + w[10]) + w[11]));
+    v = fmax(v, lo1[1]); t_t1[1] = v; new_state[50] = bias_su[50] + v;
+    v = bias_t1[2] + state[51] + (0.08226303133234722) * ((w[12] + w[13]) + (w[14] + w[15]));
+    v = fmax(v, lo1[2]); t_t1[2] = v; new_state[51] = bias_su[51] + v;
+    v = bias_t1[3] + state[52] + (0.08226303133234722) * (((w[16] + w[17]) + (w[18] + w[19])) + ((w[20] + w[21]) + w[22]));
+    v = fmax(v, lo1[3]); t_t1[3] = v; new_state[52] = bias_su[52] + v;
+    v = bias_t1[4] + state[53] + (0.08226303133234722) * ((w[23] + w[24]) + w[25]);
+    v = fmax(v, lo1[4]); t_t1[4] = v; new_state[53] = bias_su[53] + v;
+    v = bias_t1[5] + state[54] + (0.08226303133234722) * (((w[26] + w[27]) + (w[28] + w[29])) + ((w[30] + w[31]) + (w[32] + w[33])));
+    v = fmax(v, lo1[5]); t_t1[5] = v; new_state[54] = bias_su[54] + v;
+    v = bias_t1[6] + state[55] + (0.08226303133234722) * ((w[34] + w[35]) + w[36]);
+    v = fmax(v, lo1[6]); t_t1[6] = v; new_state[55] = bias_su[55] + v;
+    v = bias_t1[7] + state[56] + (0.08226303133234722) * (w[37] + w[38]);
+    v = fmax(v, lo1[7]); t_t1[7] = v; new_state[56] = bias_su[56] + v;
+    v = bias_t1[8] + state[57] + (0.08226303133234722) * (((w[39] + w[40]) + (w[41] + w[42])) + w[43]);
+    v = fmax(v, lo1[8]); t_t1[8] = v; new_state[57] = bias_su[57] + v;
+    v = bias_t1[9] + state[58] + (0.08226303133234722) * (((w[44] + w[45]) + (w[46] + w[47])) + w[48]);
+    v = fmax(v, lo1[9]); t_t1[9] = v; new_state[58] = bias_su[58] + v;
+    new_state[59] = (bias_su[59] + state[59]) + ((((0.069655061626067494) * w[0] + (0.33760783556686547) * w[12]) + ((0.26836501080727221) * w[16] + (0.037081661631488835) * w[26])) + (0.37063781296154064) * w[39]);
+    new_state[60] = (bias_su[60] + state[60]) + (((0.25588244633413909) * w[1] + (0.037583500758161678) * w[5]) + ((0.083276300544647269) * w[27] + (0.023067940734351828) * w[37]));
+    new_state[61] = (bias_su[61] + state[61]) + ((0.61752386121652636) * w[6] + (0.042388435165245451) * w[17]);
+    new_state[62] = (bias_su[62] + state[62]) + ((0.13345673088354346) * w[2] + (0.61132408824693363) * w[23]);
+    new_state[63] = (bias_su[63] + state[63]) + (((0.040307047471874392) * w[13] + (0.029151569883073398) * w[18]) + (0.58435958348428074) * w[28]);
+    new_state[64] = (bias_su[64] + state[64]) + (((0.64483412842709398) * w[7] + (0.36011436438990652) * w[19]) + (0.051091074244933282) * w[29]);
+    new_state[65] = (bias_su[65] + state[65]) + ((((0.024964136788999065) * w[3] + (0.037882339410371904) * w[8]) + ((0.00976037797556846) * w[14] + (0.27658369193168919) * w[20])) + (0.03036587998612799) * w[44]);
+    new_state[66] = (bias_su[66] + state[66]) + (((0.79478587733064987) * w[9] + (0.014854938892497713) * w[30]) + (0.018156664303604776) * w[38]);
+    new_state[67] = (bias_su[67] + state[67]) + ((0.13174930294003487) * w[40] + (0.14046939723165486) * w[45]);
+    new_state[68] = (bias_su[68] + state[68]) + ((0.013160484266981857) * w[24] + (0.38643208723267541) * w[34]);
+    new_state[69] = (bias_su[69] + state[69]) + (((0.017294599334807721) * w[10] + (0.60372695765338591) * w[31]) + ((0.25854119370877737) * w[35] + (0.14484430995648803) * w[41]));
+    new_state[70] = (bias_su[70] + state[70]) + ((0.034735970917354191) * w[21] + (0.15819624000738933) * w[25]);
+    new_state[71] = (bias_su[71] + state[71]) + (((0.021966725812901995) * w[4] + (0.021590409041307837) * w[32]) + ((0.034846066829944282) * w[42] + (0.32653085943273191) * w[46]));
+    new_state[72] = (bias_su[72] + state[72]) + (((0.011556395626565408) * w[33] + (0.098520275933536933) * w[43]) + (0.17994004051645768) * w[47]);
+    new_state[73] = (bias_su[73] + state[73]) + ((((0.089618922945269686) * w[11] + (0.46831584781193331) * w[15]) + ((0.022087418331416996) * w[22] + (0.41032977319887487) * w[36])) + (0.22496888289546163) * w[48]);
+}
```

## Agent notes

```
### Round 1

**Whole solve (`make bench`, `mean_us`)**
- Before: 40.05 µs.
- After: 8.85 µs, which is about 4.5x faster.
- The final bench runs ranged from 8.9 to 11.3 µs. That is timer noise; the minimum per solve is 6 to 8 µs.
- `make verify` passes (`VERIFY OK`). The worst end-to-end error is about 8e-15.

**Final profile (`make profile`), hottest first**
- `iterate` (200 calls to `fom_iterate`): 51,770 µs total over 5,000 solves, 99.55%.
- `param_bias` (the `su_p_p0 @ p0` term, computed once per solve): 236 µs total, 0.45%.

**Optimizations applied**
1. **Selection rows of `su` become copies.** Rows 0..48 of the new state copy `t_t0[0..48]` and rows 49..58 copy `t_t1[0..9]`; `su_state` is empty on those rows. So `new_state[i] = bias_su[i] + t` is written as soon as each temporary is computed. Only rows 59..73 (15 rows) need the full `su_state + su_t_t0 + su_t_t1` product. This removed the 74-row, three-product pass. The `su_t_t1` shift folds into these copies. (35.4 µs)
2. **Fully unrolled `fom_iterate` with coefficients inlined as literals.** The sparsity is fixed and small (`t0_state` has 147 nonzeros, `t1` has 59+49, `su` rows 59..73 have 64+98). I generated straight-line code from the CSR data in `fom_data.c`, with values printed exactly (`%.17g`). This drops all index arrays and indirect loads, and ±1 coefficients become plain adds or subtracts. The rules allow literals here because verify probes `fom_iterate` on every unit vector. One pass computes `t0` with its clip, writes the copy rows, then `t1` with its ReLU, then the 15 remaining rows. (16.1 µs)
3. **`restrict` on the `fom_iterate` pointers.** `state` and `new_state` are different ping-pong buffers. Without `restrict` the compiler has to reload `state` after every store to `new_state` or a temporary. (about 9 µs)
4. **Profile markers.** `FOM_PROF_BEGIN/END` wrap the parameter-bias setup and the iteration loop. They are no-ops in the production build.

**Not done**
- The `su_t_t0` coefficients on rows 59..73 look like exactly -2 times the matching `su_state` coefficients. Each row could then become `state[i] + Σ a·(state[j] - 2·t0[j])`, saving about 64 multiplies per iteration. I did not check that the factor is exactly 2 for every coefficient, so I left it out (expected gain about 5 to 10%).

Blocked by rules:
- Fold the 200 iterations so `t_t0`/`t_t1` are stored only on the last one | `fom_iterate` must write every temporary each call, and only `fom_iterate`/`fom_step`/`fom_fused` may hold the products | small, about 3%
- Hoist `bias_su[0..58]` into `bias_t0`/`bias_t1` so the copy rows need no add | `fom_iterate` must be exact for arbitrary independent biases, and reading the constant vectors in `src/fom_ops.c` is forbidden | small, about 2%

### Round 2

**Whole-solve mean_us:** 8.957 at round start (42.342 for the translation) → about 6.9 in back-to-back runs, against about 7.25 for the round-1 binary under the same conditions. That is about 1.05x this round and about 6x against the translation, though that last figure mixes two measurement setups. Plain `make bench` is noisy on this machine (7.3 to 11.7 µs for the same code), so the numbers come from alternating runs of the old and new binaries.

**Final profile (`make profile`, hottest first):**
- iterate: 54393 us, 99.58%
- param_bias: 232 us, 0.42%

**Optimizations applied this round (all in `fom_iterate`):**
- **Shared intermediate `w_j = 2*t0_j - state_j`.** In every t1 row and every state-update row 59..73, the t_t0 coefficient is exactly -2 times the state coefficient on the same index: `t1_t_t0 = -2 * t1_state[:, 0:49]` and `su_t_t0[59:74] = -2 * su_state[59:74, 0:49]`. So each of those rows is `bias + diagonal state + sum_j c_j * w_j`. Each `w_j` is computed once, right after the t0 clamp, and used by both blocks. This removes about 150 multiplies per iteration. The sums are paired up (tree order) to shorten the dependency chain. On its own it timed within noise (about 0.5%), but it cuts the work.
- **Clamps as `fmin(fmax(.))` instead of compare-and-select pairs.** This applies to the clip on t0 (49 rows) and the relu on t1 (10 rows). It compiles to single ARM min/max instructions, with the same result for any non-NaN input. It gave about 4%.
- **Tried and reverted:** precomputing `u_k = a*state[49+k]` once per group of t0 rows. It made no difference.

**Blocked by rules:**
- Skip writing `t_t0` and `t_t1` in non-final iterations, keeping them in registers | `fom_iterate` must write every temporary, and verify checks them | unknown, probably small (59 stores per iteration)
- Fold `bias_su[0..58]` into the t0/t1 biases to drop one add per selection row | `fom_setup`/`fom_update_params` may not probe or rebuild matrices, and `bias_su` belongs to the update, not the step | small (under 2%)

### Round 3

**Initial vs. final:** this round started at 8.491 µs (`make bench` mean_us, the given baseline). The final solver is unchanged; three bench runs gave 8.23, 8.86 and 9.10 µs (run-to-run noise about 10%), and the profile build gave 8.13 µs. Overall speedup against the 42.342 µs translated solver is about 5.0x. This round added no gain: the sources are unchanged.

**Final profile (`make profile`), hottest first:**
| Region | Time (µs, 5000 solves) | % |
|---|---|---|
| iterate | 39897 | 99.53% |
| param_bias | 188 | 0.47% |

**Optimizations in place (from earlier rounds, kept):**
- **Fused one-pass `fom_iterate`.** It computes t0, then t1, then the state update in one pass, holding w = 2·t0 − s0 in registers. This is valid because of the network structure. `su_t_t0` and `su_state` combine into the reflection 2·t0 − s0 for rows 59..73, and `t1_t_t0` is a group-sum of the same w.
- **Unrolled over the fixed sparsity with coefficients written in.** Every t0 row is s0[i] minus two terms. The shared coefficient 0.0822… on the s1 block is factored out of each t1 group sum.
- **Selection rows written directly.** Rows 0..58 of the new state are just bias_su + t0 or t1, so there are no zeroing or copy passes.
- **Parameter term hoisted.** `su_p_p0 @ p0` is computed once per solve as `bias_su`.

**Ideas checked this round and dropped:**
- Explicit SIMD: rows gather from scattered entries of s2, so vector loads would need gathers that cost more than they save.
- Rewriting the clamp: `fmin`/`fmax` already compile to single `fminnm`/`fmaxnm` instructions on arm64.
- Loop restructuring: with `-O3 -flto`, `fom_iterate` is already inlined into the solve loop.

Each iteration is about 40 ns for roughly 500 flops plus about 160 stores. Remaining gains would be below the measurement noise.

Blocked by rules:
- Skip writing the `t_t0`/`t_t1` temporaries on every iteration except the last (about 59 stores per iteration) | `fom_iterate` must write every temporary each call, because verify checks them and the solver and objective need them | ~5%
- Run the 200-iteration loop inside `src/fom_ops.c` so the state stays in registers across iterations instead of going through buffers | the interface is fixed: `fom_iterate` runs exactly one iteration, and `src/fom_solver.c` may not read matrix data | unknown, likely <5%
```
