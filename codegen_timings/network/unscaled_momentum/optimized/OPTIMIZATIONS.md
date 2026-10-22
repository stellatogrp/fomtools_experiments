# Optimization report

## Timing

```
translated (emitted)      63.1 us/solve
optimized                 19.3 us/solve
speedup                  3.27x
```

Optimization outcome: **kept (re-verified correct)**.

## Rounds

| round | before (us) | after (us) | gain | kept | agent (s) | outcome |
|---:|---:|---:|---:|:---:|---:|---|
| 1 | 63.119 | 19.319 | 69.4% | yes | 81 | continued |
| 2 | - | - | - | no | 69 | the round left the sources unchanged |

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
| `iterate` | 74x74 | 623 | yes | basis | 228 | 8.882e-16 |

### Rule violations

_None: the collected rewrite passed the rules and was kept._

## Blocked by rules

Optimizations the agent considered but did not make because a rule forbade them, in its own words.

| round | idea | rule | expected gain |
|---|---|---|---|
| 1 | Merge each `su` row's `x[j]` and `t_t0[j]` terms into one `c * (t_t0[j] - x[j])`, since the coefficients are equal and opposite up to about 1e-16, and the same for the `__d_xi` pair | values must be exactly the matrix's; the tiny mismatches make the merged coefficient differ from the package data | about 5-10% |
| 1 | Build flags such as `-ffast-math` or `-march=native` to let the compiler reassociate and vectorize the unrolled sums | the Makefile and build flags are fixed | unknown, maybe 10-20% |
| 2 | Changing the compile flags so the compiler could vectorize the 49-row `t0` clip and the gather-heavy `su` rows | the Makefile and build flags are fixed | unknown, probably under 10% |

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
@@ -52,15 +53,19 @@
         const fom_float xi = fom_sched_xi(it);
 
         /* t0, t1 and the state update in one fused pass. */
+        FOM_PROF_BEGIN(iterate);
         fom_iterate(x, xi, bias_t0, bias_t1, pterm, &FOM_ACT_t0, &FOM_ACT_t1, t0, t1, xn);
+        FOM_PROF_END(iterate);
 
         if (it + 1 == max_iter) {
+            FOM_PROF_BEGIN(residual);
             res = 0.0;
             for (i = 0; i < FOM_STATE_DIM; ++i) {
                 fom_float d = xn[i] - x[i];
                 d = d < 0.0 ? -d : d;
                 res = d > res ? d : res;
             }
+            FOM_PROF_END(residual);
         }
 
         { fom_float *tmp = x; x = xn; xn = tmp; }
--- src/fom_ops.c (translated)
+++ src/fom_ops.c (optimized)
@@ -253,7 +253,197 @@
 }
 
 void fom_iterate(const fom_float *state, fom_float sc_xi, const fom_float *bias_t0, const fom_float *bias_t1, const fom_float *bias_su, const FomAct_t0 *act_t0, const FomAct_t1 *act_t1, fom_float *t_t0, fom_float *t_t1, fom_float *new_state) {
-    fom_step_t0(state, bias_t0, act_t0, t_t0);
-    fom_step_t1(state, t_t0, sc_xi, bias_t1, act_t1, t_t1);
-    fom_step_su(state, t_t0, t_t1, sc_xi, bias_su, new_state);
-}
+    const fom_float *x = state; fom_float v;
+    v = bias_t0[0] + (x[0] + -0.08226303133234722 * (x[49]) + -0.0696550616260675 * (x[59]));
+    v = v < act_t0->lo[0] ? act_t0->lo[0] : v; v = v > act_t0->hi[0] ? act_t0->hi[0] : v; t_t0[0] = v;
+    v = bias_t0[1] + (x[1] + -0.08226303133234722 * (x[49]) + -0.2558824463341391 * (x[60]));
+    v = v < act_t0->lo[1] ? act_t0->lo[1] : v; v = v > act_t0->hi[1] ? act_t0->hi[1] : v; t_t0[1] = v;
+    v = bias_t0[2] + (x[2] + -0.08226303133234722 * (x[49]) + -0.13345673088354346 * (x[62]));
+    v = v < act_t0->lo[2] ? act_t0->lo[2] : v; v = v > act_t0->hi[2] ? act_t0->hi[2] : v; t_t0[2] = v;
+    v = bias_t0[3] + (x[3] + -0.08226303133234722 * (x[49]) + -0.024964136788999065 * (x[65]));
+    v = v < act_t0->lo[3] ? act_t0->lo[3] : v; v = v > act_t0->hi[3] ? act_t0->hi[3] : v; t_t0[3] = v;
+    v = bias_t0[4] + (x[4] + -0.08226303133234722 * (x[49]) + -0.021966725812901995 * (x[71]));
+    v = v < act_t0->lo[4] ? act_t0->lo[4] : v; v = v > act_t0->hi[4] ? act_t0->hi[4] : v; t_t0[4] = v;
+    v = bias_t0[5] + (x[5] + -0.08226303133234722 * (x[50]) + -0.03758350075816168 * (x[60]));
+    v = v < act_t0->lo[5] ? act_t0->lo[5] : v; v = v > act_t0->hi[5] ? act_t0->hi[5] : v; t_t0[5] = v;
+    v = bias_t0[6] + (x[6] + -0.08226303133234722 * (x[50]) + -0.6175238612165264 * (x[61]));
+    v = v < act_t0->lo[6] ? act_t0->lo[6] : v; v = v > act_t0->hi[6] ? act_t0->hi[6] : v; t_t0[6] = v;
+    v = bias_t0[7] + (x[7] + -0.08226303133234722 * (x[50]) + -0.644834128427094 * (x[64]));
+    v = v < act_t0->lo[7] ? act_t0->lo[7] : v; v = v > act_t0->hi[7] ? act_t0->hi[7] : v; t_t0[7] = v;
+    v = bias_t0[8] + (x[8] + -0.08226303133234722 * (x[50]) + -0.037882339410371904 * (x[65]));
+    v = v < act_t0->lo[8] ? act_t0->lo[8] : v; v = v > act_t0->hi[8] ? act_t0->hi[8] : v; t_t0[8] = v;
+    v = bias_t0[9] + (x[9] + -0.08226303133234722 * (x[50]) + -0.7947858773306499 * (x[66]));
+    v = v < act_t0->lo[9] ? act_t0->lo[9] : v; v = v > act_t0->hi[9] ? act_t0->hi[9] : v; t_t0[9] = v;
+    v = bias_t0[10] + (x[10] + -0.08226303133234722 * (x[50]) + -0.01729459933480772 * (x[69]));
+    v = v < act_t0->lo[10] ? act_t0->lo[10] : v; v = v > act_t0->hi[10] ? act_t0->hi[10] : v; t_t0[10] = v;
+    v = bias_t0[11] + (x[11] + -0.08226303133234722 * (x[50]) + -0.08961892294526969 * (x[73]));
+    v = v < act_t0->lo[11] ? act_t0->lo[11] : v; v = v > act_t0->hi[11] ? act_t0->hi[11] : v; t_t0[11] = v;
+    v = bias_t0[12] + (x[12] + -0.08226303133234722 * (x[51]) + -0.33760783556686547 * (x[59]));
+    v = v < act_t0->lo[12] ? act_t0->lo[12] : v; v = v > act_t0->hi[12] ? act_t0->hi[12] : v; t_t0[12] = v;
+    v = bias_t0[13] + (x[13] + -0.08226303133234722 * (x[51]) + -0.04030704747187439 * (x[63]));
+    v = v < act_t0->lo[13] ? act_t0->lo[13] : v; v = v > act_t0->hi[13] ? act_t0->hi[13] : v; t_t0[13] = v;
+    v = bias_t0[14] + (x[14] + -0.08226303133234722 * (x[51]) + -0.00976037797556846 * (x[65]));
+    v = v < act_t0->lo[14] ? act_t0->lo[14] : v; v = v > act_t0->hi[14] ? act_t0->hi[14] : v; t_t0[14] = v;
+    v = bias_t0[15] + (x[15] + -0.08226303133234722 * (x[51]) + -0.4683158478119333 * (x[73]));
+    v = v < act_t0->lo[15] ? act_t0->lo[15] : v; v = v > act_t0->hi[15] ? act_t0->hi[15] : v; t_t0[15] = v;
+    v = bias_t0[16] + (x[16] + -0.08226303133234722 * (x[52]) + -0.2683650108072722 * (x[59]));
+    v = v < act_t0->lo[16] ? act_t0->lo[16] : v; v = v > act_t0->hi[16] ? act_t0->hi[16] : v; t_t0[16] = v;
+    v = bias_t0[17] + (x[17] + -0.08226303133234722 * (x[52]) + -0.04238843516524545 * (x[61]));
+    v = v < act_t0->lo[17] ? act_t0->lo[17] : v; v = v > act_t0->hi[17] ? act_t0->hi[17] : v; t_t0[17] = v;
+    v = bias_t0[18] + (x[18] + -0.08226303133234722 * (x[52]) + -0.029151569883073398 * (x[63]));
+    v = v < act_t0->lo[18] ? act_t0->lo[18] : v; v = v > act_t0->hi[18] ? act_t0->hi[18] : v; t_t0[18] = v;
+    v = bias_t0[19] + (x[19] + -0.08226303133234722 * (x[52]) + -0.3601143643899065 * (x[64]));
+    v = v < act_t0->lo[19] ? act_t0->lo[19] : v; v = v > act_t0->hi[19] ? act_t0->hi[19] : v; t_t0[19] = v;
+    v = bias_t0[20] + (x[20] + -0.08226303133234722 * (x[52]) + -0.2765836919316892 * (x[65]));
+    v = v < act_t0->lo[20] ? act_t0->lo[20] : v; v = v > act_t0->hi[20] ? act_t0->hi[20] : v; t_t0[20] = v;
+    v = bias_t0[21] + (x[21] + -0.08226303133234722 * (x[52]) + -0.03473597091735419 * (x[70]));
+    v = v < act_t0->lo[21] ? act_t0->lo[21] : v; v = v > act_t0->hi[21] ? act_t0->hi[21] : v; t_t0[21] = v;
+    v = bias_t0[22] + (x[22] + -0.08226303133234722 * (x[52]) + -0.022087418331416996 * (x[73]));
+    v = v < act_t0->lo[22] ? act_t0->lo[22] : v; v = v > act_t0->hi[22] ? act_t0->hi[22] : v; t_t0[22] = v;
+    v = bias_t0[23] + (x[23] + -0.08226303133234722 * (x[53]) + -0.6113240882469336 * (x[62]));
+    v = v < act_t0->lo[23] ? act_t0->lo[23] : v; v = v > act_t0->hi[23] ? act_t0->hi[23] : v; t_t0[23] = v;
+    v = bias_t0[24] + (x[24] + -0.08226303133234722 * (x[53]) + -0.013160484266981857 * (x[68]));
+    v = v < act_t0->lo[24] ? act_t0->lo[24] : v; v = v > act_t0->hi[24] ? act_t0->hi[24] : v; t_t0[24] = v;
+    v = bias_t0[25] + (x[25] + -0.08226303133234722 * (x[53]) + -0.15819624000738933 * (x[70]));
+    v = v < act_t0->lo[25] ? act_t0->lo[25] : v; v = v > act_t0->hi[25] ? act_t0->hi[25] : v; t_t0[25] = v;
+    v = bias_t0[26] + (x[26] + -0.08226303133234722 * (x[54]) + -0.037081661631488835 * (x[59]));
+    v = v < act_t0->lo[26] ? act_t0->lo[26] : v; v = v > act_t0->hi[26] ? act_t0->hi[26] : v; t_t0[26] = v;
+    v = bias_t0[27] + (x[27] + -0.08226303133234722 * (x[54]) + -0.08327630054464727 * (x[60]));
+    v = v < act_t0->lo[27] ? act_t0->lo[27] : v; v = v > act_t0->hi[27] ? act_t0->hi[27] : v; t_t0[27] = v;
+    v = bias_t0[28] + (x[28] + -0.08226303133234722 * (x[54]) + -0.5843595834842807 * (x[63]));
+    v = v < act_t0->lo[28] ? act_t0->lo[28] : v; v = v > act_t0->hi[28] ? act_t0->hi[28] : v; t_t0[28] = v;
+    v = bias_t0[29] + (x[29] + -0.08226303133234722 * (x[54]) + -0.05109107424493328 * (x[64]));
+    v = v < act_t0->lo[29] ? act_t0->lo[29] : v; v = v > act_t0->hi[29] ? act_t0->hi[29] : v; t_t0[29] = v;
+    v = bias_t0[30] + (x[30] + -0.08226303133234722 * (x[54]) + -0.014854938892497713 * (x[66]));
+    v = v < act_t0->lo[30] ? act_t0->lo[30] : v; v = v > act_t0->hi[30] ? act_t0->hi[30] : v; t_t0[30] = v;
+    v = bias_t0[31] + (x[31] + -0.08226303133234722 * (x[54]) + -0.6037269576533859 * (x[69]));
+    v = v < act_t0->lo[31] ? act_t0->lo[31] : v; v = v > act_t0->hi[31] ? act_t0->hi[31] : v; t_t0[31] = v;
+    v = bias_t0[32] + (x[32] + -0.08226303133234722 * (x[54]) + -0.021590409041307837 * (x[71]));
+    v = v < act_t0->lo[32] ? act_t0->lo[32] : v; v = v > act_t0->hi[32] ? act_t0->hi[32] : v; t_t0[32] = v;
+    v = bias_t0[33] + (x[33] + -0.08226303133234722 * (x[54]) + -0.011556395626565408 * (x[72]));
+    v = v < act_t0->lo[33] ? act_t0->lo[33] : v; v = v > act_t0->hi[33] ? act_t0->hi[33] : v; t_t0[33] = v;
+    v = bias_t0[34] + (x[34] + -0.08226303133234722 * (x[55]) + -0.3864320872326754 * (x[68]));
+    v = v < act_t0->lo[34] ? act_t0->lo[34] : v; v = v > act_t0->hi[34] ? act_t0->hi[34] : v; t_t0[34] = v;
+    v = bias_t0[35] + (x[35] + -0.08226303133234722 * (x[55]) + -0.2585411937087774 * (x[69]));
+    v = v < act_t0->lo[35] ? act_t0->lo[35] : v; v = v > act_t0->hi[35] ? act_t0->hi[35] : v; t_t0[35] = v;
+    v = bias_t0[36] + (x[36] + -0.08226303133234722 * (x[55]) + -0.41032977319887487 * (x[73]));
+    v = v < act_t0->lo[36] ? act_t0->lo[36] : v; v = v > act_t0->hi[36] ? act_t0->hi[36] : v; t_t0[36] = v;
+    v = bias_t0[37] + (x[37] + -0.08226303133234722 * (x[56]) + -0.023067940734351828 * (x[60]));
+    v = v < act_t0->lo[37] ? act_t0->lo[37] : v; v = v > act_t0->hi[37] ? act_t0->hi[37] : v; t_t0[37] = v;
+    v = bias_t0[38] + (x[38] + -0.08226303133234722 * (x[56]) + -0.018156664303604776 * (x[66]));
+    v = v < act_t0->lo[38] ? act_t0->lo[38] : v; v = v > act_t0->hi[38] ? act_t0->hi[38] : v; t_t0[38] = v;
+    v = bias_t0[39] + (x[39] + -0.08226303133234722 * (x[57]) + -0.37063781296154064 * (x[59]));
+    v = v < act_t0->lo[39] ? act_t0->lo[39] : v; v = v > act_t0->hi[39] ? act_t0->hi[39] : v; t_t0[39] = v;
+    v = bias_t0[40] + (x[40] + -0.08226303133234722 * (x[57]) + -0.13174930294003487 * (x[67]));
+    v = v < act_t0->lo[40] ? act_t0->lo[40] : v; v = v > act_t0->hi[40] ? act_t0->hi[40] : v; t_t0[40] = v;
+    v = bias_t0[41] + (x[41] + -0.08226303133234722 * (x[57]) + -0.14484430995648803 * (x[69]));
+    v = v < act_t0->lo[41] ? act_t0->lo[41] : v; v = v > act_t0->hi[41] ? act_t0->hi[41] : v; t_t0[41] = v;
+    v = bias_t0[42] + (x[42] + -0.08226303133234722 * (x[57]) + -0.03484606682994428 * (x[71]));
+    v = v < act_t0->lo[42] ? act_t0->lo[42] : v; v = v > act_t0->hi[42] ? act_t0->hi[42] : v; t_t0[42] = v;
+    v = bias_t0[43] + (x[43] + -0.08226303133234722 * (x[57]) + -0.09852027593353693 * (x[72]));
+    v = v < act_t0->lo[43] ? act_t0->lo[43] : v; v = v > act_t0->hi[43] ? act_t0->hi[43] : v; t_t0[43] = v;
+    v = bias_t0[44] + (x[44] + -0.08226303133234722 * (x[58]) + -0.03036587998612799 * (x[65]));
+    v = v < act_t0->lo[44] ? act_t0->lo[44] : v; v = v > act_t0->hi[44] ? act_t0->hi[44] : v; t_t0[44] = v;
+    v = bias_t0[45] + (x[45] + -0.08226303133234722 * (x[58]) + -0.14046939723165486 * (x[67]));
+    v = v < act_t0->lo[45] ? act_t0->lo[45] : v; v = v > act_t0->hi[45] ? act_t0->hi[45] : v; t_t0[45] = v;
+    v = bias_t0[46] + (x[46] + -0.08226303133234722 * (x[58]) + -0.3265308594327319 * (x[71]));
+    v = v < act_t0->lo[46] ? act_t0->lo[46] : v; v = v > act_t0->hi[46] ? act_t0->hi[46] : v; t_t0[46] = v;
+    v = bias_t0[47] + (x[47] + -0.08226303133234722 * (x[58]) + -0.17994004051645768 * (x[72]));
+    v = v < act_t0->lo[47] ? act_t0->lo[47] : v; v = v > act_t0->hi[47] ? act_t0->hi[47] : v; t_t0[47] = v;
+    v = bias_t0[48] + (x[48] + -0.08226303133234722 * (x[58]) + -0.22496888289546163 * (x[73]));
+    v = v < act_t0->lo[48] ? act_t0->lo[48] : v; v = v > act_t0->hi[48] ? act_t0->hi[48] : v; t_t0[48] = v;
+    v = bias_t1[0] + (-0.08226303133234725 * (x[0] + x[1] + x[2] + x[3] + x[4]) + x[49] + 0.16452606266469444 * (t_t0[0] + t_t0[1] + t_t0[2] + t_t0[3] + t_t0[4])) + sc_xi * (-0.16452606266469433 * (x[0] + x[1] + x[2] + x[3] + x[4]) + 0.16452606266469444 * (t_t0[0] + t_t0[1] + t_t0[2] + t_t0[3] + t_t0[4]));
+    t_t1[0] = v < act_t1->lo[0] ? act_t1->lo[0] : v;
+    v = bias_t1[1] + (-0.08226303133234725 * (x[5] + x[6] + x[7] + x[8] + x[9] + x[10] + x[11]) + x[50] + 0.16452606266469444 * (t_t0[5] + t_t0[6] + t_t0[7] + t_t0[8] + t_t0[9] + t_t0[10] + t_t0[11])) + sc_xi * (-0.16452606266469433 * (x[5] + x[6] + x[7] + x[8] + x[9] + x[10] + x[11]) + 0.16452606266469444 * (t_t0[5] + t_t0[6] + t_t0[7] + t_t0[8] + t_t0[9] + t_t0[10] + t_t0[11]));
+    t_t1[1] = v < act_t1->lo[1] ? act_t1->lo[1] : v;
+    v = bias_t1[2] + (-0.08226303133234725 * (x[12] + x[13] + x[14] + x[15]) + x[51] + 0.16452606266469444 * (t_t0[12] + t_t0[13] + t_t0[14] + t_t0[15])) + sc_xi * (-0.16452606266469433 * (x[12] + x[13] + x[14] + x[15]) + 0.16452606266469444 * (t_t0[12] + t_t0[13] + t_t0[14] + t_t0[15]));
+    t_t1[2] = v < act_t1->lo[2] ? act_t1->lo[2] : v;
+    v = bias_t1[3] + (-0.08226303133234725 * (x[16] + x[17] + x[18] + x[19] + x[20] + x[21] + x[22]) + x[52] + 0.16452606266469444 * (t_t0[16] + t_t0[17] + t_t0[18] + t_t0[19] + t_t0[20] + t_t0[21] + t_t0[22])) + sc_xi * (-0.16452606266469433 * (x[16] + x[17] + x[18] + x[19] + x[20] + x[21] + x[22]) + 0.16452606266469444 * (t_t0[16] + t_t0[17] + t_t0[18] + t_t0[19] + t_t0[20] + t_t0[21] + t_t0[22]));
+    t_t1[3] = v < act_t1->lo[3] ? act_t1->lo[3] : v;
+    v = bias_t1[4] + (-0.08226303133234725 * (x[23] + x[24] + x[25]) + x[53] + 0.16452606266469444 * (t_t0[23] + t_t0[24] + t_t0[25])) + sc_xi * (-0.16452606266469433 * (x[23] + x[24] + x[25]) + 0.16452606266469444 * (t_t0[23] + t_t0[24] + t_t0[25]));
+    t_t1[4] = v < act_t1->lo[4] ? act_t1->lo[4] : v;
+    v = bias_t1[5] + (-0.08226303133234725 * (x[26] + x[27] + x[28] + x[29] + x[30] + x[31] + x[32] + x[33]) + x[54] + 0.16452606266469444 * (t_t0[26] + t_t0[27] + t_t0[28] + t_t0[29] + t_t0[30] + t_t0[31] + t_t0[32] + t_t0[33])) + sc_xi * (-0.16452606266469433 * (x[26] + x[27] + x[28] + x[29] + x[30] + x[31] + x[32] + x[33]) + 0.16452606266469444 * (t_t0[26] + t_t0[27] + t_t0[28] + t_t0[29] + t_t0[30] + t_t0[31] + t_t0[32] + t_t0[33]));
+    t_t1[5] = v < act_t1->lo[5] ? act_t1->lo[5] : v;
+    v = bias_t1[6] + (-0.08226303133234725 * (x[34] + x[35] + x[36]) + x[55] + 0.16452606266469444 * (t_t0[34] + t_t0[35] + t_t0[36])) + sc_xi * (-0.16452606266469433 * (x[34] + x[35] + x[36]) + 0.16452606266469444 * (t_t0[34] + t_t0[35] + t_t0[36]));
+    t_t1[6] = v < act_t1->lo[6] ? act_t1->lo[6] : v;
+    v = bias_t1[7] + (-0.08226303133234725 * (x[37] + x[38]) + x[56] + 0.16452606266469444 * (t_t0[37] + t_t0[38])) + sc_xi * (-0.16452606266469433 * (x[37] + x[38]) + 0.16452606266469444 * (t_t0[37] + t_t0[38]));
+    t_t1[7] = v < act_t1->lo[7] ? act_t1->lo[7] : v;
+    v = bias_t1[8] + (-0.08226303133234725 * (x[39] + x[40] + x[41] + x[42] + x[43]) + x[57] + 0.16452606266469444 * (t_t0[39] + t_t0[40] + t_t0[41] + t_t0[42] + t_t0[43])) + sc_xi * (-0.16452606266469433 * (x[39] + x[40] + x[41] + x[42] + x[43]) + 0.16452606266469444 * (t_t0[39] + t_t0[40] + t_t0[41] + t_t0[42] + t_t0[43]));
+    t_t1[8] = v < act_t1->lo[8] ? act_t1->lo[8] : v;
+    v = bias_t1[9] + (-0.08226303133234725 * (x[44] + x[45] + x[46] + x[47] + x[48]) + x[58] + 0.16452606266469444 * (t_t0[44] + t_t0[45] + t_t0[46] + t_t0[47] + t_t0[48])) + sc_xi * (-0.16452606266469433 * (x[44] + x[45] + x[46] + x[47] + x[48]) + 0.16452606266469444 * (t_t0[44] + t_t0[45] + t_t0[46] + t_t0[47] + t_t0[48]));
+    t_t1[9] = v < act_t1->lo[9] ? act_t1->lo[9] : v;
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
+    new_state[59] = bias_su[59] + (-0.06965506162606752 * (x[0]) + -0.3376078355668656 * (x[12]) + -0.2683650108072723 * (x[16]) + -0.03708166163148883 * (x[26]) + -0.37063781296154064 * (x[39]) + x[59] + 0.13931012325213496 * (t_t0[0]) + 0.6752156711337309 * (t_t0[12]) + 0.5367300216145444 * (t_t0[16]) + 0.0741633232629777 * (t_t0[26]) + 0.7412756259230815 * (t_t0[39])) + sc_xi * (-0.1393101232521349 * (x[0]) + -0.6752156711337306 * (x[12]) + -0.536730021614544 * (x[16]) + -0.07416332326297768 * (x[26]) + -0.7412756259230812 * (x[39]) + 0.13931012325213504 * (t_t0[0]) + 0.6752156711337306 * (t_t0[12]) + 0.5367300216145442 * (t_t0[16]) + 0.07416332326297763 * (t_t0[26]) + 0.7412756259230803 * (t_t0[39]));
+    new_state[60] = bias_su[60] + (-0.25588244633413904 * (x[1]) + -0.03758350075816168 * (x[5]) + -0.08327630054464728 * (x[27]) + -0.023067940734351824 * (x[37]) + x[60] + 0.5117648926682782 * (t_t0[1]) + 0.07516700151632338 * (t_t0[5]) + 0.1665526010892946 * (t_t0[27]) + 0.04613588146870367 * (t_t0[37])) + sc_xi * (-0.5117648926682782 * (x[1]) + -0.07516700151632334 * (x[5]) + -0.1665526010892945 * (x[27]) + -0.046135881468703656 * (x[37]) + 0.511764892668278 * (t_t0[1]) + 0.07516700151632327 * (t_t0[5]) + 0.1665526010892944 * (t_t0[27]) + 0.0461358814687036 * (t_t0[37]));
+    new_state[61] = bias_su[61] + (-0.6175238612165267 * (x[6]) + -0.04238843516524547 * (x[17]) + x[61] + 1.2350477224330532 * (t_t0[6]) + 0.08477687033049092 * (t_t0[17])) + sc_xi * (-1.2350477224330518 * (x[6]) + -0.08477687033049083 * (x[17]) + 1.2350477224330518 * (t_t0[6]) + 0.08477687033049087 * (t_t0[17]));
+    new_state[62] = bias_su[62] + (-0.13345673088354354 * (x[2]) + -0.611324088246934 * (x[23]) + x[62] + 0.266913461767087 * (t_t0[2]) + 1.2226481764938675 * (t_t0[23])) + sc_xi * (-0.2669134617670867 * (x[2]) + -1.2226481764938668 * (x[23]) + 0.2669134617670866 * (t_t0[2]) + 1.2226481764938673 * (t_t0[23]));
+    new_state[63] = bias_su[63] + (-0.040307047471874385 * (x[13]) + -0.029151569883073405 * (x[18]) + -0.5843595834842807 * (x[28]) + x[63] + 0.0806140949437488 * (t_t0[13]) + 0.05830313976614679 * (t_t0[18]) + 1.168719166968562 * (t_t0[28])) + sc_xi * (-0.0806140949437488 * (x[13]) + -0.05830313976614678 * (x[18]) + -1.1687191669685617 * (x[28]) + 0.08061409494374874 * (t_t0[13]) + 0.05830313976614681 * (t_t0[18]) + 1.1687191669685606 * (t_t0[28]));
+    new_state[64] = bias_su[64] + (-0.6448341284270941 * (x[7]) + -0.36011436438990646 * (x[19]) + -0.05109107424493329 * (x[29]) + x[64] + 1.289668256854188 * (t_t0[7]) + 0.7202287287798131 * (t_t0[19]) + 0.10218214848986659 * (t_t0[29])) + sc_xi * (-1.2896682568541873 * (x[7]) + -0.7202287287798131 * (x[19]) + -0.10218214848986652 * (x[29]) + 1.2896682568541873 * (t_t0[7]) + 0.7202287287798127 * (t_t0[19]) + 0.10218214848986652 * (t_t0[29]));
+    new_state[65] = bias_su[65] + (-0.02496413678899907 * (x[3]) + -0.03788233941037191 * (x[8]) + -0.00976037797556846 * (x[14]) + -0.2765836919316893 * (x[20]) + -0.030365879986127997 * (x[44]) + x[65] + 0.04992827357799813 * (t_t0[3]) + 0.07576467882074381 * (t_t0[8]) + 0.01952075595113692 * (t_t0[14]) + 0.5531673838633784 * (t_t0[20]) + 0.060731759972255994 * (t_t0[44])) + sc_xi * (-0.04992827357799812 * (x[3]) + -0.0757646788207438 * (x[8]) + -0.019520755951136917 * (x[14]) + -0.553167383863378 * (x[20]) + -0.06073175997225594 * (x[44]) + 0.04992827357799812 * (t_t0[3]) + 0.07576467882074377 * (t_t0[8]) + 0.019520755951136917 * (t_t0[14]) + 0.5531673838633783 * (t_t0[20]) + 0.06073175997225592 * (t_t0[44]));
+    new_state[66] = bias_su[66] + (-0.7947858773306501 * (x[9]) + -0.014854938892497714 * (x[30]) + -0.018156664303604783 * (x[38]) + x[66] + 1.5895717546613 * (t_t0[9]) + 0.029709877784995432 * (t_t0[30]) + 0.036313328607209566 * (t_t0[38])) + sc_xi * (-1.5895717546612993 * (x[9]) + -0.02970987778499542 * (x[30]) + -0.036313328607209545 * (x[38]) + 1.5895717546612993 * (t_t0[9]) + 0.029709877784995405 * (t_t0[30]) + 0.03631332860720952 * (t_t0[38]));
+    new_state[67] = bias_su[67] + (-0.131749302940035 * (x[40]) + -0.14046939723165494 * (x[45]) + x[67] + 0.2634986058800698 * (t_t0[40]) + 0.28093879446330966 * (t_t0[45])) + sc_xi * (-0.2634986058800694 * (x[40]) + -0.2809387944633095 * (x[45]) + 0.2634986058800696 * (t_t0[40]) + 0.2809387944633098 * (t_t0[45]));
+    new_state[68] = bias_su[68] + (-0.013160484266981862 * (x[24]) + -0.38643208723267564 * (x[34]) + x[68] + 0.02632096853396372 * (t_t0[24]) + 0.7728641744653508 * (t_t0[34])) + sc_xi * (-0.026320968533963696 * (x[24]) + -0.7728641744653503 * (x[34]) + 0.026320968533963696 * (t_t0[24]) + 0.7728641744653507 * (t_t0[34]));
+    new_state[69] = bias_su[69] + (-0.017294599334807734 * (x[10]) + -0.6037269576533858 * (x[31]) + -0.2585411937087774 * (x[35]) + -0.14484430995648817 * (x[41]) + x[69] + 0.03458919866961544 * (t_t0[10]) + 1.2074539153067723 * (t_t0[31]) + 0.5170823874175549 * (t_t0[35]) + 0.2896886199129762 * (t_t0[41])) + sc_xi * (-0.03458919866961541 * (x[10]) + -1.207453915306772 * (x[31]) + -0.5170823874175547 * (x[35]) + -0.2896886199129758 * (x[41]) + 0.03458919866961544 * (t_t0[10]) + 1.2074539153067712 * (t_t0[31]) + 0.5170823874175545 * (t_t0[35]) + 0.28968861991297584 * (t_t0[41]));
+    new_state[70] = bias_su[70] + (-0.034735970917354184 * (x[21]) + -0.15819624000738947 * (x[25]) + x[70] + 0.06947194183470837 * (t_t0[21]) + 0.3163924800147787 * (t_t0[25])) + sc_xi * (-0.06947194183470837 * (x[21]) + -0.3163924800147784 * (x[25]) + 0.06947194183470838 * (t_t0[21]) + 0.3163924800147786 * (t_t0[25]));
+    new_state[71] = bias_su[71] + (-0.021966725812902 * (x[4]) + -0.02159040904130785 * (x[32]) + -0.03484606682994429 * (x[42]) + -0.3265308594327321 * (x[46]) + x[71] + 0.04393345162580401 * (t_t0[4]) + 0.04318081808261569 * (t_t0[32]) + 0.06969213365988856 * (t_t0[42]) + 0.6530617188654638 * (t_t0[46])) + sc_xi * (-0.043933451625803976 * (x[4]) + -0.043180818082615646 * (x[32]) + -0.06969213365988852 * (x[42]) + -0.6530617188654634 * (x[46]) + 0.04393345162580395 * (t_t0[4]) + 0.04318081808261562 * (t_t0[32]) + 0.06969213365988855 * (t_t0[42]) + 0.6530617188654637 * (t_t0[46]));
+    new_state[72] = bias_su[72] + (-0.01155639562656541 * (x[33]) + -0.09852027593353693 * (x[43]) + -0.17994004051645768 * (x[47]) + x[72] + 0.023112791253130816 * (t_t0[33]) + 0.19704055186707398 * (t_t0[43]) + 0.35988008103291547 * (t_t0[47])) + sc_xi * (-0.023112791253130805 * (x[33]) + -0.19704055186707384 * (x[43]) + -0.3598800810329153 * (x[47]) + 0.023112791253130805 * (t_t0[33]) + 0.19704055186707362 * (t_t0[43]) + 0.3598800810329151 * (t_t0[47]));
+    new_state[73] = bias_su[73] + (-0.08961892294526974 * (x[11]) + -0.46831584781193325 * (x[15]) + -0.022087418331416996 * (x[22]) + -0.41032977319887487 * (x[36]) + -0.22496888289546174 * (x[48]) + x[73] + 0.17923784589053937 * (t_t0[11]) + 0.9366316956238666 * (t_t0[15]) + 0.044174836662834 * (t_t0[22]) + 0.8206595463977497 * (t_t0[36]) + 0.4499377657909233 * (t_t0[48])) + sc_xi * (-0.1792378458905392 * (x[11]) + -0.9366316956238665 * (x[15]) + -0.044174836662833984 * (x[22]) + -0.8206595463977495 * (x[36]) + -0.44993776579092293 * (x[48]) + 0.17923784589053932 * (t_t0[11]) + 0.9366316956238665 * (t_t0[15]) + 0.044174836662833984 * (t_t0[22]) + 0.8206595463977495 * (t_t0[36]) + 0.44993776579092315 * (t_t0[48]));
+}
```

## Agent notes

```
### Round 1

**Whole-solve mean_us:** 60.36 at the start, 17.3 at the end (runs gave 17.3 to 18.4), so about 3.5x faster.

**Final profile** (`make profile`, instrumented build, BENCH mean 22.9 µs):
| region | total_us | pct |
|---|---|---|
| iterate | 90905 | 99.82% |
| residual | 164 | 0.18% |

**Optimizations applied:**
- **Fully unrolled `fom_iterate` with the coefficients written as literals** (60.4 to 18.8 µs). Every member matrix is fixed and small (about 620 nonzeros in total), and `fom_iterate` is checked on every unit vector, so writing the values as literals is allowed there. A generator script reads `data.npz` and writes one line of straight-line code per output row. It drops zero coefficients and multiplies by 1.0. This removes all CSR index arithmetic and indirect loads, and keeps values in registers between the t0, t1 and su blocks. Bias and activation bounds are still read from the arguments, and `sc_xi` stays a runtime argument, so the routine is correct for any schedule value.
- **Selection rows written as plain copies** (part of the step above). Rows 0..58 of `su` are `bias + t_t0[i]` or `bias + t_t1[i-49]`, because they are 0/1 selections with no other members. Rows 59..73 are the only ones that do real multiplies.
- **Factoring out shared coefficients within a row** (18.8 to about 17.3 µs). In `t1`, every entry of `t1_state`, `t1_t_t0` and their `__d_xi` versions is one of a few constants (-0.0823, +0.1645, -0.1645). So each row becomes `c * (sum of x over a group)`, and the same group sum is reused between the unscaled and the `sc_xi`-scaled parts. That replaces about 4 multiplies per nonzero with one per group.
- **Unchanged from before:** `su_p_p0 @ p0` and the constant biases are computed once per solve, and the residual is only computed on the last iteration. There are no linear solves in this method, so `fom_linsolve.c` was not touched.
- **Profiling:** `FOM_PROF_BEGIN/END` markers now wrap `iterate` and `residual` in `src/fom_solver.c`.

Blocked by rules:
- Merge each `su` row's `x[j]` and `t_t0[j]` terms into one `c * (t_t0[j] - x[j])`, since the coefficients are equal and opposite up to about 1e-16, and the same for the `__d_xi` pair | values must be exactly the matrix's; the tiny mismatches make the merged coefficient differ from the package data | about 5-10%
- Build flags such as `-ffast-math` or `-march=native` to let the compiler reassociate and vectorize the unrolled sums | the Makefile and build flags are fixed | unknown, maybe 10-20%

### Round 2

**Result:** no improvement this round, so the sources are unchanged from round 1.
- Initial whole-solve `mean_us`: 19.319 (reported at the start of the round). Re-measured here at about 16.5 to 17.6; the bench varies by roughly ±5% from run to run.
- Final whole-solve `mean_us`: the same code, so about 16.5 to 19.3 depending on the run.
- Speedup this round: 1.00x. Overall it stays at about 3.27x against the translated solver (63.119 us).

**Final profile (`make profile`), hottest first:**
| region | time | % |
|---|---|---|
| iterate | 90556 us | 99.81% |
| residual | 172 us | 0.19% |

**Optimizations tried this round:**
- I added `restrict` local aliases inside `fom_iterate` for the state, the temporaries, the new state and the activation bound pointers. The idea was to stop the compiler reloading `x[49..73]` and the bounds after each store. It passed `VERIFY OK`, but four bench runs each before and after showed no gain (about 17.4 us vs about 16.9 us, within noise), so I reverted it.

**Why the round-1 solver is close to optimal:**
- `fom_iterate` is already one fully unrolled pass over the fixed sparsity, at about 85 ns per iteration.
- Each `t0` row has 2 off-diagonal entries.
- Each `t1` row adds group sums of `x` and `t0`. The compiler computes each sum once and reuses it for both the base term and the `xi` term, since the expressions are identical.
- The `su` selection rows 0..58 are plain copies.
- The residual is computed only on the last iteration.
- The state buffers are swapped by pointer rather than copied.

**Ideas I rejected on numerical grounds:**
- **Merging coefficients in the `su` rows 59..73.** The `x` coefficient is about -c and the `t0` coefficient about 2c, so a row could in principle use one shared coefficient. The two values differ in the last bits, though, and verify's per-entry tolerance is designed to catch exactly that kind of change.
- **Combining `a + xi*b` into one coefficient each iteration.** Computing those coefficients costs as many FMAs as the products they replace.

Blocked by rules:
- Changing the compile flags so the compiler could vectorize the 49-row `t0` clip and the gather-heavy `su` rows | the Makefile and build flags are fixed | unknown, probably under 10%
```
