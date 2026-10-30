# Optimization report

## Timing

```
translated (emitted)      31.8 us/solve
optimized                 12.4 us/solve
speedup                  2.57x
```

Optimization outcome: **kept (re-verified correct)**.

## Rounds

| round | before (us) | after (us) | gain | kept | agent (s) | outcome |
|---:|---:|---:|---:|:---:|---:|---|
| 1 | 31.794 | 13.619 | 57.2% | yes | 98 | continued |
| 2 | 15.857 | 13.272 | 16.3% | yes | 66 | continued |
| 3 | 15.171 | 13.710 | 9.6% | yes | 76 | continued |
| 4 | 12.549 | 12.358 | 1.5% | yes | 80 | the round gained less than agent_optimize_min_gain |

## Kernel checks

Each `fom_apply_<name>` in the shipped `fom_ops.c`, checked by `make verify` against the package's reference over the same `fom_data.c` matrix. *basis* probes every unit vector, which recovers the routine's matrix column by column (an exact check), plus random vectors; *random* uses random vectors only (the matrix is over the probe budget). A `fused_<block>` row is a `fom_fused_<block>`, checked against the sum of its members: *basis* there probes every unit vector of each source with every schedule value at 0 and with each alone at 1, which recovers every member exactly.

| matrix | shape | nnz | rewritten | check | probes | max error |
|---|---|---|---|---|---|---|
| `t0_state` | 49x74 | 147 | no | basis | 76 | 0.000e+00 |
| `t1_state` | 10x74 | 59 | no | basis | 76 | 0.000e+00 |
| `t1_t_t0` | 10x49 | 49 | no | basis | 51 | 0.000e+00 |
| `su_state` | 74x74 | 64 | no | basis | 76 | 0.000e+00 |
| `su_t_t0` | 74x49 | 98 | no | basis | 51 | 0.000e+00 |
| `su_t_t1` | 74x10 | 10 | no | basis | 12 | 0.000e+00 |
| `su_p_p0` | 74x15 | 15 | no | basis | 17 | 0.000e+00 |
| `fused_t1` | 10x123 | 108 | no | basis | 125 | 0.000e+00 |
| `fused_su` | 74x133 | 172 | no | basis | 135 | 0.000e+00 |
| `step_t0` | 49x74 | 147 | no | basis | 80 | 0.000e+00 |
| `step_t1` | 10x123 | 108 | no | basis | 129 | 0.000e+00 |
| `step_su` | 74x133 | 172 | no | basis | 135 | 0.000e+00 |
| `fused_su__rest` | 74x133 | 172 | no | basis | 135 | 0.000e+00 |
| `step_su__rest` | 74x133 | 172 | no | basis | 135 | 0.000e+00 |
| `iterate` | 74x74 | 427 | yes | basis | 154 | 8.882e-16 |

### Rule violations

_None: the collected rewrite passed the rules and was kept._

## Blocked by rules

Optimizations the agent considered but did not make because a rule forbade them, in its own words.

| round | idea | rule | expected gain |
|---|---|---|---|
| 1 | Skip the `bias_su` adds on rows 0..58, which are always zero for this package (`su_p_p0` only reaches rows 59..73) | `fom_iterate` must apply any bias it is given, and the constant/bias vectors may not be read in `fom_ops.c` | about 1% |
| 1 | Skip writing the `t_t0`/`t_t1` temporaries and keep only `new_state` (rows 0..58 already hold them) | `fom_iterate` must write every temporary | about 1-2% |
| 2 | Skipping the `t_t0`/`t_t1` temporary writes on all but the last iteration (the solver only needs them at the end) | `fom_iterate` must write every temporary, and the solver must call the checked routines | unknown, probably <3% |
| 3 | Skip writing `t_t0`/`t_t1` on every iteration except the last, since in the no-tolerance loop only `new_state` carries information | `fom_iterate` must write every temporary, and new non-static entry points in `src/fom_ops.c` are not allowed | ~10-15% (removes ~59 of ~180 stores per iteration) |
| 3 | Fold the constant bias `t0_c` together with `bias_su` and the bounds into one precomputed per-row vector inside `fom_iterate` | `src/fom_ops.c` may not read `FOM_ACT_*` or the constant and bound vectors, and `fom_setup` cannot call the iteration routines | unknown, small |
| 4 | Skip writing `t_t0`/`t_t1` on all but the last iteration (they are dead when `check_termination` is off) | `fom_iterate` must write every temporary, and any other kernel must be a non-static entry point in `src/fom_ops.c` or must not read matrix data in `src/fom_solver.c` | ~5-10% (98 of ~180 stores per iteration) |
| 4 | Unroll two iterations per call to hide cross-iteration latency and reuse loads | the `fom_iterate` signature is fixed to one iteration; adding a 2-step entry point breaks "every function but these entry points is static" | unknown, maybe 5-10% |
| 4 | Drop the `bias_su` additions on rows 0..58, where `su_p_p0` is zero for this data | `fom_iterate` must be exact for any bias, and specializing on the parameter matrix's zero rows inside `fom_iterate`'s bias would need reading `FOM_MAT_su_p_p0`, which is outside the iteration | ~2% |
| 4 | `-O3`/`-march=native` in the bench build (the profile build already uses -O3) | build flags and Makefile are fixed | unknown |

## What the agent changed

```diff
--- src/fom_solver.c (translated)
+++ src/fom_solver.c (optimized)
@@ -46,31 +46,49 @@
     for (i = 0; i < FOM_STATE_DIM; ++i) pc[i] = 0.0;
     fom_apply_su_p_p0(work->params.p0, pc);
 
-    for (k = 0; k < max_iter; ++k) {
-        fom_float *sw;
+    if (!use_tol) {
+        /* no termination check: run the iterations back to back, residual
+           only from the last one */
+        for (k = 0; k < max_iter; ++k) {
+            fom_float *sw;
+            FOM_PROF_BEGIN(iterate);
+            fom_iterate(x, FOM_MAT_t0_c, FOM_MAT_t1_c, pc, &FOM_ACT_t0, &FOM_ACT_t1, t0, t1, xn);
+            FOM_PROF_END(iterate);
+            sw = x; x = xn; xn = sw;
+        }
+        FOM_PROF_BEGIN(residual);
+        if (max_iter > 0) {
+            for (i = 0; i < FOM_STATE_DIM; ++i) {
+                fom_float d = x[i] - xn[i];
+                d = d < 0 ? -d : d;
+                res = d > res ? d : res;
+            }
+        }
+        FOM_PROF_END(residual);
+    } else {
+        for (k = 0; k < max_iter; ++k) {
+            fom_float *sw;
 
-        fom_step_t0(x, FOM_MAT_t0_c, &FOM_ACT_t0, t0);
-        fom_step_t1(x, t0, FOM_MAT_t1_c, &FOM_ACT_t1, t1);
-        /* su selection rows: xn[0..48] = t0, xn[49..58] = t1 */
-        for (i = 0; i < FOM_T0_DIM; ++i) xn[i] = t0[i];
-        for (i = 0; i < FOM_T1_DIM; ++i) xn[FOM_T0_DIM + i] = t1[i];
-        fom_step_su__rest(x, t0, t1, pc, xn);
+            FOM_PROF_BEGIN(iterate);
+            fom_iterate(x, FOM_MAT_t0_c, FOM_MAT_t1_c, pc, &FOM_ACT_t0, &FOM_ACT_t1, t0, t1, xn);
+            FOM_PROF_END(iterate);
 
-        if (use_tol || k == max_iter - 1) {
+            FOM_PROF_BEGIN(residual);
             res = 0.0;
             for (i = 0; i < FOM_STATE_DIM; ++i) {
                 fom_float d = xn[i] - x[i];
                 d = d < 0 ? -d : d;
                 res = d > res ? d : res;
             }
-        }
+            FOM_PROF_END(residual);
 
-        sw = x; x = xn; xn = sw;
+            sw = x; x = xn; xn = sw;
 
-        if (use_tol && res <= tol) {
-            status = FOM_SOLVED;
-            ++k;
-            break;
+            if (res <= tol) {
+                status = FOM_SOLVED;
+                ++k;
+                break;
+            }
         }
     }
 
--- src/fom_ops.c (translated)
+++ src/fom_ops.c (optimized)
@@ -181,7 +181,140 @@
 }
 
 void fom_iterate(const fom_float *state, const fom_float *bias_t0, const fom_float *bias_t1, const fom_float *bias_su, const FomAct_t0 *act_t0, const FomAct_t1 *act_t1, fom_float *t_t0, fom_float *t_t1, fom_float *new_state) {
-    fom_step_t0(state, bias_t0, act_t0, t_t0);
-    fom_step_t1(state, t_t0, bias_t1, act_t1, t_t1);
-    fom_step_su(state, t_t0, t_t1, bias_su, new_state);
-}
+    const fom_float *x = state; const fom_float *lo0 = act_t0->lo, *hi0 = act_t0->hi, *lo1 = act_t1->lo;
+    fom_float v, d, s;
+    fom_float a59 = bias_su[59], a60 = bias_su[60], a61 = bias_su[61], a62 = bias_su[62], a63 = bias_su[63], a64 = bias_su[64], a65 = bias_su[65], a66 = bias_su[66], a67 = bias_su[67], a68 = bias_su[68], a69 = bias_su[69], a70 = bias_su[70], a71 = bias_su[71], a72 = bias_su[72], a73 = bias_su[73];
+    { const fom_float xg = x[49];
+    v = bias_t0[0] + x[0] + -0.08226303133234722 * xg + -0.0696550616260675 * x[59]; v = v < lo0[0] ? lo0[0] : v; v = v > hi0[0] ? hi0[0] : v; d = x[0] - 2.0 * v; t_t0[0] = v; new_state[0] = bias_su[0] + v;
+    s = d; a59 += -0.0696550616260675 * d;
+    v = bias_t0[1] + x[1] + -0.08226303133234722 * xg + -0.2558824463341391 * x[60]; v = v < lo0[1] ? lo0[1] : v; v = v > hi0[1] ? hi0[1] : v; d = x[1] - 2.0 * v; t_t0[1] = v; new_state[1] = bias_su[1] + v;
+    s += d; a60 += -0.2558824463341391 * d;
+    v = bias_t0[2] + x[2] + -0.08226303133234722 * xg + -0.13345673088354346 * x[62]; v = v < lo0[2] ? lo0[2] : v; v = v > hi0[2] ? hi0[2] : v; d = x[2] - 2.0 * v; t_t0[2] = v; new_state[2] = bias_su[2] + v;
+    s += d; a62 += -0.13345673088354346 * d;
+    v = bias_t0[3] + x[3] + -0.08226303133234722 * xg + -0.024964136788999065 * x[65]; v = v < lo0[3] ? lo0[3] : v; v = v > hi0[3] ? hi0[3] : v; d = x[3] - 2.0 * v; t_t0[3] = v; new_state[3] = bias_su[3] + v;
+    s += d; a65 += -0.024964136788999065 * d;
+    v = bias_t0[4] + x[4] + -0.08226303133234722 * xg + -0.021966725812901995 * x[71]; v = v < lo0[4] ? lo0[4] : v; v = v > hi0[4] ? hi0[4] : v; d = x[4] - 2.0 * v; t_t0[4] = v; new_state[4] = bias_su[4] + v;
+    s += d; a71 += -0.021966725812901995 * d;
+    v = bias_t1[0] + -0.08226303133234722 * s + xg; v = v < lo1[0] ? lo1[0] : v; t_t1[0] = v; new_state[49] = bias_su[49] + v; }
+    { const fom_float xg = x[50];
+    v = bias_t0[5] + x[5] + -0.08226303133234722 * xg + -0.03758350075816168 * x[60]; v = v < lo0[5] ? lo0[5] : v; v = v > hi0[5] ? hi0[5] : v; d = x[5] - 2.0 * v; t_t0[5] = v; new_state[5] = bias_su[5] + v;
+    s = d; a60 += -0.03758350075816168 * d;
+    v = bias_t0[6] + x[6] + -0.08226303133234722 * xg + -0.6175238612165264 * x[61]; v = v < lo0[6] ? lo0[6] : v; v = v > hi0[6] ? hi0[6] : v; d = x[6] - 2.0 * v; t_t0[6] = v; new_state[6] = bias_su[6] + v;
+    s += d; a61 += -0.6175238612165264 * d;
+    v = bias_t0[7] + x[7] + -0.08226303133234722 * xg + -0.644834128427094 * x[64]; v = v < lo0[7] ? lo0[7] : v; v = v > hi0[7] ? hi0[7] : v; d = x[7] - 2.0 * v; t_t0[7] = v; new_state[7] = bias_su[7] + v;
+    s += d; a64 += -0.644834128427094 * d;
+    v = bias_t0[8] + x[8] + -0.08226303133234722 * xg + -0.037882339410371904 * x[65]; v = v < lo0[8] ? lo0[8] : v; v = v > hi0[8] ? hi0[8] : v; d = x[8] - 2.0 * v; t_t0[8] = v; new_state[8] = bias_su[8] + v;
+    s += d; a65 += -0.037882339410371904 * d;
+    v = bias_t0[9] + x[9] + -0.08226303133234722 * xg + -0.7947858773306499 * x[66]; v = v < lo0[9] ? lo0[9] : v; v = v > hi0[9] ? hi0[9] : v; d = x[9] - 2.0 * v; t_t0[9] = v; new_state[9] = bias_su[9] + v;
+    s += d; a66 += -0.7947858773306499 * d;
+    v = bias_t0[10] + x[10] + -0.08226303133234722 * xg + -0.01729459933480772 * x[69]; v = v < lo0[10] ? lo0[10] : v; v = v > hi0[10] ? hi0[10] : v; d = x[10] - 2.0 * v; t_t0[10] = v; new_state[10] = bias_su[10] + v;
+    s += d; a69 += -0.01729459933480772 * d;
+    v = bias_t0[11] + x[11] + -0.08226303133234722 * xg + -0.08961892294526969 * x[73]; v = v < lo0[11] ? lo0[11] : v; v = v > hi0[11] ? hi0[11] : v; d = x[11] - 2.0 * v; t_t0[11] = v; new_state[11] = bias_su[11] + v;
+    s += d; a73 += -0.08961892294526969 * d;
+    v = bias_t1[1] + -0.08226303133234722 * s + xg; v = v < lo1[1] ? lo1[1] : v; t_t1[1] = v; new_state[50] = bias_su[50] + v; }
+    { const fom_float xg = x[51];
+    v = bias_t0[12] + x[12] + -0.08226303133234722 * xg + -0.33760783556686547 * x[59]; v = v < lo0[12] ? lo0[12] : v; v = v > hi0[12] ? hi0[12] : v; d = x[12] - 2.0 * v; t_t0[12] = v; new_state[12] = bias_su[12] + v;
+    s = d; a59 += -0.33760783556686547 * d;
+    v = bias_t0[13] + x[13] + -0.08226303133234722 * xg + -0.04030704747187439 * x[63]; v = v < lo0[13] ? lo0[13] : v; v = v > hi0[13] ? hi0[13] : v; d = x[13] - 2.0 * v; t_t0[13] = v; new_state[13] = bias_su[13] + v;
+    s += d; a63 += -0.04030704747187439 * d;
+    v = bias_t0[14] + x[14] + -0.08226303133234722 * xg + -0.00976037797556846 * x[65]; v = v < lo0[14] ? lo0[14] : v; v = v > hi0[14] ? hi0[14] : v; d = x[14] - 2.0 * v; t_t0[14] = v; new_state[14] = bias_su[14] + v;
+    s += d; a65 += -0.00976037797556846 * d;
+    v = bias_t0[15] + x[15] + -0.08226303133234722 * xg + -0.4683158478119333 * x[73]; v = v < lo0[15] ? lo0[15] : v; v = v > hi0[15] ? hi0[15] : v; d = x[15] - 2.0 * v; t_t0[15] = v; new_state[15] = bias_su[15] + v;
+    s += d; a73 += -0.4683158478119333 * d;
+    v = bias_t1[2] + -0.08226303133234722 * s + xg; v = v < lo1[2] ? lo1[2] : v; t_t1[2] = v; new_state[51] = bias_su[51] + v; }
+    { const fom_float xg = x[52];
+    v = bias_t0[16] + x[16] + -0.08226303133234722 * xg + -0.2683650108072722 * x[59]; v = v < lo0[16] ? lo0[16] : v; v = v > hi0[16] ? hi0[16] : v; d = x[16] - 2.0 * v; t_t0[16] = v; new_state[16] = bias_su[16] + v;
+    s = d; a59 += -0.2683650108072722 * d;
+    v = bias_t0[17] + x[17] + -0.08226303133234722 * xg + -0.04238843516524545 * x[61]; v = v < lo0[17] ? lo0[17] : v; v = v > hi0[17] ? hi0[17] : v; d = x[17] - 2.0 * v; t_t0[17] = v; new_state[17] = bias_su[17] + v;
+    s += d; a61 += -0.04238843516524545 * d;
+    v = bias_t0[18] + x[18] + -0.08226303133234722 * xg + -0.029151569883073398 * x[63]; v = v < lo0[18] ? lo0[18] : v; v = v > hi0[18] ? hi0[18] : v; d = x[18] - 2.0 * v; t_t0[18] = v; new_state[18] = bias_su[18] + v;
+    s += d; a63 += -0.029151569883073398 * d;
+    v = bias_t0[19] + x[19] + -0.08226303133234722 * xg + -0.3601143643899065 * x[64]; v = v < lo0[19] ? lo0[19] : v; v = v > hi0[19] ? hi0[19] : v; d = x[19] - 2.0 * v; t_t0[19] = v; new_state[19] = bias_su[19] + v;
+    s += d; a64 += -0.3601143643899065 * d;
+    v = bias_t0[20] + x[20] + -0.08226303133234722 * xg + -0.2765836919316892 * x[65]; v = v < lo0[20] ? lo0[20] : v; v = v > hi0[20] ? hi0[20] : v; d = x[20] - 2.0 * v; t_t0[20] = v; new_state[20] = bias_su[20] + v;
+    s += d; a65 += -0.2765836919316892 * d;
+    v = bias_t0[21] + x[21] + -0.08226303133234722 * xg + -0.03473597091735419 * x[70]; v = v < lo0[21] ? lo0[21] : v; v = v > hi0[21] ? hi0[21] : v; d = x[21] - 2.0 * v; t_t0[21] = v; new_state[21] = bias_su[21] + v;
+    s += d; a70 += -0.03473597091735419 * d;
+    v = bias_t0[22] + x[22] + -0.08226303133234722 * xg + -0.022087418331416996 * x[73]; v = v < lo0[22] ? lo0[22] : v; v = v > hi0[22] ? hi0[22] : v; d = x[22] - 2.0 * v; t_t0[22] = v; new_state[22] = bias_su[22] + v;
+    s += d; a73 += -0.022087418331416996 * d;
+    v = bias_t1[3] + -0.08226303133234722 * s + xg; v = v < lo1[3] ? lo1[3] : v; t_t1[3] = v; new_state[52] = bias_su[52] + v; }
+    { const fom_float xg = x[53];
+    v = bias_t0[23] + x[23] + -0.08226303133234722 * xg + -0.6113240882469336 * x[62]; v = v < lo0[23] ? lo0[23] : v; v = v > hi0[23] ? hi0[23] : v; d = x[23] - 2.0 * v; t_t0[23] = v; new_state[23] = bias_su[23] + v;
+    s = d; a62 += -0.6113240882469336 * d;
+    v = bias_t0[24] + x[24] + -0.08226303133234722 * xg + -0.013160484266981857 * x[68]; v = v < lo0[24] ? lo0[24] : v; v = v > hi0[24] ? hi0[24] : v; d = x[24] - 2.0 * v; t_t0[24] = v; new_state[24] = bias_su[24] + v;
+    s += d; a68 += -0.013160484266981857 * d;
+    v = bias_t0[25] + x[25] + -0.08226303133234722 * xg + -0.15819624000738933 * x[70]; v = v < lo0[25] ? lo0[25] : v; v = v > hi0[25] ? hi0[25] : v; d = x[25] - 2.0 * v; t_t0[25] = v; new_state[25] = bias_su[25] + v;
+    s += d; a70 += -0.15819624000738933 * d;
+    v = bias_t1[4] + -0.08226303133234722 * s + xg; v = v < lo1[4] ? lo1[4] : v; t_t1[4] = v; new_state[53] = bias_su[53] + v; }
+    { const fom_float xg = x[54];
+    v = bias_t0[26] + x[26] + -0.08226303133234722 * xg + -0.037081661631488835 * x[59]; v = v < lo0[26] ? lo0[26] : v; v = v > hi0[26] ? hi0[26] : v; d = x[26] - 2.0 * v; t_t0[26] = v; new_state[26] = bias_su[26] + v;
+    s = d; a59 += -0.037081661631488835 * d;
+    v = bias_t0[27] + x[27] + -0.08226303133234722 * xg + -0.08327630054464727 * x[60]; v = v < lo0[27] ? lo0[27] : v; v = v > hi0[27] ? hi0[27] : v; d = x[27] - 2.0 * v; t_t0[27] = v; new_state[27] = bias_su[27] + v;
+    s += d; a60 += -0.08327630054464727 * d;
+    v = bias_t0[28] + x[28] + -0.08226303133234722 * xg + -0.5843595834842807 * x[63]; v = v < lo0[28] ? lo0[28] : v; v = v > hi0[28] ? hi0[28] : v; d = x[28] - 2.0 * v; t_t0[28] = v; new_state[28] = bias_su[28] + v;
+    s += d; a63 += -0.5843595834842807 * d;
+    v = bias_t0[29] + x[29] + -0.08226303133234722 * xg + -0.05109107424493328 * x[64]; v = v < lo0[29] ? lo0[29] : v; v = v > hi0[29] ? hi0[29] : v; d = x[29] - 2.0 * v; t_t0[29] = v; new_state[29] = bias_su[29] + v;
+    s += d; a64 += -0.05109107424493328 * d;
+    v = bias_t0[30] + x[30] + -0.08226303133234722 * xg + -0.014854938892497713 * x[66]; v = v < lo0[30] ? lo0[30] : v; v = v > hi0[30] ? hi0[30] : v; d = x[30] - 2.0 * v; t_t0[30] = v; new_state[30] = bias_su[30] + v;
+    s += d; a66 += -0.014854938892497713 * d;
+    v = bias_t0[31] + x[31] + -0.08226303133234722 * xg + -0.6037269576533859 * x[69]; v = v < lo0[31] ? lo0[31] : v; v = v > hi0[31] ? hi0[31] : v; d = x[31] - 2.0 * v; t_t0[31] = v; new_state[31] = bias_su[31] + v;
+    s += d; a69 += -0.6037269576533859 * d;
+    v = bias_t0[32] + x[32] + -0.08226303133234722 * xg + -0.021590409041307837 * x[71]; v = v < lo0[32] ? lo0[32] : v; v = v > hi0[32] ? hi0[32] : v; d = x[32] - 2.0 * v; t_t0[32] = v; new_state[32] = bias_su[32] + v;
+    s += d; a71 += -0.021590409041307837 * d;
+    v = bias_t0[33] + x[33] + -0.08226303133234722 * xg + -0.011556395626565408 * x[72]; v = v < lo0[33] ? lo0[33] : v; v = v > hi0[33] ? hi0[33] : v; d = x[33] - 2.0 * v; t_t0[33] = v; new_state[33] = bias_su[33] + v;
+    s += d; a72 += -0.011556395626565408 * d;
+    v = bias_t1[5] + -0.08226303133234722 * s + xg; v = v < lo1[5] ? lo1[5] : v; t_t1[5] = v; new_state[54] = bias_su[54] + v; }
+    { const fom_float xg = x[55];
+    v = bias_t0[34] + x[34] + -0.08226303133234722 * xg + -0.3864320872326754 * x[68]; v = v < lo0[34] ? lo0[34] : v; v = v > hi0[34] ? hi0[34] : v; d = x[34] - 2.0 * v; t_t0[34] = v; new_state[34] = bias_su[34] + v;
+    s = d; a68 += -0.3864320872326754 * d;
+    v = bias_t0[35] + x[35] + -0.08226303133234722 * xg + -0.2585411937087774 * x[69]; v = v < lo0[35] ? lo0[35] : v; v = v > hi0[35] ? hi0[35] : v; d = x[35] - 2.0 * v; t_t0[35] = v; new_state[35] = bias_su[35] + v;
+    s += d; a69 += -0.2585411937087774 * d;
+    v = bias_t0[36] + x[36] + -0.08226303133234722 * xg + -0.41032977319887487 * x[73]; v = v < lo0[36] ? lo0[36] : v; v = v > hi0[36] ? hi0[36] : v; d = x[36] - 2.0 * v; t_t0[36] = v; new_state[36] = bias_su[36] + v;
+    s += d; a73 += -0.41032977319887487 * d;
+    v = bias_t1[6] + -0.08226303133234722 * s + xg; v = v < lo1[6] ? lo1[6] : v; t_t1[6] = v; new_state[55] = bias_su[55] + v; }
+    { const fom_float xg = x[56];
+    v = bias_t0[37] + x[37] + -0.08226303133234722 * xg + -0.023067940734351828 * x[60]; v = v < lo0[37] ? lo0[37] : v; v = v > hi0[37] ? hi0[37] : v; d = x[37] - 2.0 * v; t_t0[37] = v; new_state[37] = bias_su[37] + v;
+    s = d; a60 += -0.023067940734351828 * d;
+    v = bias_t0[38] + x[38] + -0.08226303133234722 * xg + -0.018156664303604776 * x[66]; v = v < lo0[38] ? lo0[38] : v; v = v > hi0[38] ? hi0[38] : v; d = x[38] - 2.0 * v; t_t0[38] = v; new_state[38] = bias_su[38] + v;
+    s += d; a66 += -0.018156664303604776 * d;
+    v = bias_t1[7] + -0.08226303133234722 * s + xg; v = v < lo1[7] ? lo1[7] : v; t_t1[7] = v; new_state[56] = bias_su[56] + v; }
+    { const fom_float xg = x[57];
+    v = bias_t0[39] + x[39] + -0.08226303133234722 * xg + -0.37063781296154064 * x[59]; v = v < lo0[39] ? lo0[39] : v; v = v > hi0[39] ? hi0[39] : v; d = x[39] - 2.0 * v; t_t0[39] = v; new_state[39] = bias_su[39] + v;
+    s = d; a59 += -0.37063781296154064 * d;
+    v = bias_t0[40] + x[40] + -0.08226303133234722 * xg + -0.13174930294003487 * x[67]; v = v < lo0[40] ? lo0[40] : v; v = v > hi0[40] ? hi0[40] : v; d = x[40] - 2.0 * v; t_t0[40] = v; new_state[40] = bias_su[40] + v;
+    s += d; a67 += -0.13174930294003487 * d;
+    v = bias_t0[41] + x[41] + -0.08226303133234722 * xg + -0.14484430995648803 * x[69]; v = v < lo0[41] ? lo0[41] : v; v = v > hi0[41] ? hi0[41] : v; d = x[41] - 2.0 * v; t_t0[41] = v; new_state[41] = bias_su[41] + v;
+    s += d; a69 += -0.14484430995648803 * d;
+    v = bias_t0[42] + x[42] + -0.08226303133234722 * xg + -0.03484606682994428 * x[71]; v = v < lo0[42] ? lo0[42] : v; v = v > hi0[42] ? hi0[42] : v; d = x[42] - 2.0 * v; t_t0[42] = v; new_state[42] = bias_su[42] + v;
+    s += d; a71 += -0.03484606682994428 * d;
+    v = bias_t0[43] + x[43] + -0.08226303133234722 * xg + -0.09852027593353693 * x[72]; v = v < lo0[43] ? lo0[43] : v; v = v > hi0[43] ? hi0[43] : v; d = x[43] - 2.0 * v; t_t0[43] = v; new_state[43] = bias_su[43] + v;
+    s += d; a72 += -0.09852027593353693 * d;
+    v = bias_t1[8] + -0.08226303133234722 * s + xg; v = v < lo1[8] ? lo1[8] : v; t_t1[8] = v; new_state[57] = bias_su[57] + v; }
+    { const fom_float xg = x[58];
+    v = bias_t0[44] + x[44] + -0.08226303133234722 * xg + -0.03036587998612799 * x[65]; v = v < lo0[44] ? lo0[44] : v; v = v > hi0[44] ? hi0[44] : v; d = x[44] - 2.0 * v; t_t0[44] = v; new_state[44] = bias_su[44] + v;
+    s = d; a65 += -0.03036587998612799 * d;
+    v = bias_t0[45] + x[45] + -0.08226303133234722 * xg + -0.14046939723165486 * x[67]; v = v < lo0[45] ? lo0[45] : v; v = v > hi0[45] ? hi0[45] : v; d = x[45] - 2.0 * v; t_t0[45] = v; new_state[45] = bias_su[45] + v;
+    s += d; a67 += -0.14046939723165486 * d;
+    v = bias_t0[46] + x[46] + -0.08226303133234722 * xg + -0.3265308594327319 * x[71]; v = v < lo0[46] ? lo0[46] : v; v = v > hi0[46] ? hi0[46] : v; d = x[46] - 2.0 * v; t_t0[46] = v; new_state[46] = bias_su[46] + v;
+    s += d; a71 += -0.3265308594327319 * d;
+    v = bias_t0[47] + x[47] + -0.08226303133234722 * xg + -0.17994004051645768 * x[72]; v = v < lo0[47] ? lo0[47] : v; v = v > hi0[47] ? hi0[47] : v; d = x[47] - 2.0 * v; t_t0[47] = v; new_state[47] = bias_su[47] + v;
+    s += d; a72 += -0.17994004051645768 * d;
+    v = bias_t0[48] + x[48] + -0.08226303133234722 * xg + -0.22496888289546163 * x[73]; v = v < lo0[48] ? lo0[48] : v; v = v > hi0[48] ? hi0[48] : v; d = x[48] - 2.0 * v; t_t0[48] = v; new_state[48] = bias_su[48] + v;
+    s += d; a73 += -0.22496888289546163 * d;
+    v = bias_t1[9] + -0.08226303133234722 * s + xg; v = v < lo1[9] ? lo1[9] : v; t_t1[9] = v; new_state[58] = bias_su[58] + v; }
+    new_state[59] = a59 + x[59];
+    new_state[60] = a60 + x[60];
+    new_state[61] = a61 + x[61];
+    new_state[62] = a62 + x[62];
+    new_state[63] = a63 + x[63];
+    new_state[64] = a64 + x[64];
+    new_state[65] = a65 + x[65];
+    new_state[66] = a66 + x[66];
+    new_state[67] = a67 + x[67];
+    new_state[68] = a68 + x[68];
+    new_state[69] = a69 + x[69];
+    new_state[70] = a70 + x[70];
+    new_state[71] = a71 + x[71];
+    new_state[72] = a72 + x[72];
+    new_state[73] = a73 + x[73];
+}
```

## Agent notes

```
### Round 1

**Whole-solve time:** `mean_us` went from 30.70 to about 13.0 (12.8 to 15.1 across runs). That is about a 2.3x speedup. `make verify` prints `VERIFY OK`, with a worst error of 8.0e-15 against a tolerance of 1e-6.

**Final profile (`make profile`; the timer overhead inflates the totals):**
| Region | Time | Share |
|---|---|---|
| iterate | 61.9 ms per 1e6 calls | 76.8% |
| residual | 18.7 ms per 1e6 calls | 23.2% |

The residual is computed only on the last iteration, so that region is almost all timer overhead.

**Optimizations applied:**
1. **One fully unrolled `fom_iterate` pass.** The products have a fixed sparsity: 147, 59, 49, 64 and 98 nonzeros. So the routine is straight-line code with the coefficients written as exact literals, which the rules allow inside `fom_iterate`. Zero coefficients are skipped and unit coefficients need no multiply. Bias, clip and relu are folded into the same pass. The CSR index loops, the zeroing and copy passes, and the four separate calls are gone. Gain: 30.7 to about 14.2 us.
2. **Copy rows and the shift written in the same pass.** Rows 0..48 and 49..58 of `su` are plain copies of `t0` and `t1`. They are written as `bias_su[i] + v` straight from registers while each value is computed. The `su_t_t1` shift is free, because `t1` is already in locals. Verify checks these rows with random biases, so the bias must stay in.
3. **Shared columns between member matrices.** Inside `su` (rows 59..73) and `t1`, wherever `su_state`/`t1_state` and `su_t_t0`/`t1_t_t0` both touch column j, the `t_t0` coefficient is exactly -2 times the state coefficient. So `c*x[j] + (-2c)*t0[j]` becomes `c*(x[j] - 2.0*t0[j])`. Terms that share the same `c` (every `t1` row) are factored once per row. This roughly halves the multiplies in those rows. Gain: about 14.2 to about 13.0 us; the runs are noisy.

There were no linear solves to optimize (`fom_linsolve.c` is empty), and `su_p_p0 @ p0` stays computed once per solve.

Blocked by rules:
- Skip the `bias_su` adds on rows 0..58, which are always zero for this package (`su_p_p0` only reaches rows 59..73) | `fom_iterate` must apply any bias it is given, and the constant/bias vectors may not be read in `fom_ops.c` | about 1%
- Skip writing the `t_t0`/`t_t1` temporaries and keep only `new_state` (rows 0..58 already hold them) | `fom_iterate` must write every temporary | about 1-2%

### Round 2

**Whole-solve `mean_us`**
- Start of round 2: 13.619 us (pipeline figure). My own reruns of the unchanged code gave 11.2 to 15.1 us.
- Final: about 11.9 us on the last `make bench`. In alternating old/new runs it ranged 12.6 to 14.5 us, with old and new interleaved.
- Speedup this round: about 1.0 to 1.05x, inside the noise. In side-by-side runs the new code was faster in 3 of 4 pairs, by roughly 5 to 8%.
- Against the translated solver (31.794 us): about 2.3 to 2.7x overall.

**Final profile (`make profile`), hottest first**
- iterate: 63299 us, 99.71% (1,000,000 calls)
- residual: 184 us, 0.29% (5,000 calls)

**Optimizations applied**
1. **Residual moved out of the iteration loop when no tolerance is checked.** The spec has tol=None, so only the final iteration's residual is ever used. When `check_termination` is off or tol <= 0, the solver now runs `fom_iterate` back to back with just a pointer swap. It computes `max|x_new - x|` once after the loop. This removes a per-iteration branch, and the residual profile region drops from 23% to 0.3%. Part of that 23% was profiling-timer overhead, not real compute. The tol > 0 path is kept separately and behaves exactly as before.
2. **Shared intermediate `d[i] = x[i] - 2*t0[i]` in `fom_iterate`.** The t1 rows and the su rest rows (59..73) both use `t1_state x + t1_t_t0 t0` and `su_state x + su_t_t0 t0`. These collapse to coefficients times `(x[i] - 2 t0[i])`, a relation between member matrices that round 1 had already folded in. Each difference is now computed once, right after the clip of row i, and both blocks reuse it. Before, the expression was repeated in both places and the compiler had to recognise the duplicate. The same expression is evaluated, so results do not change; verify's worst error is 5.3e-15.

**Considered but not done (no rule involved)**
- SIMD-vectorizing the t0 block, whose rows come in runs that share `x[49+j]` with a scattered second term. With about 60 ns per iteration left and ±10% bench noise, I didn't expect a measurable gain.

Blocked by rules:
- Skipping the `t_t0`/`t_t1` temporary writes on all but the last iteration (the solver only needs them at the end) | `fom_iterate` must write every temporary, and the solver must call the checked routines | unknown, probably <3%

### Round 3

**Whole-solve time.** The round started at 13.272 us (2.40x over the translated solver's 31.794 us). The final solver measures 12.2 to 14.8 us per solve, against 13.0 to 15.0 us for the starting code, both measured interleaved in this session. Over 7 interleaved pairs the starting code averaged about 14.05 us and the final code about 13.90 us. That is roughly a 1% gain, within the ±10% run-to-run noise. Taking the gain as about 1%, the speedup stays about 2.4x over the translated solver. This round made no meaningful improvement, so the pipeline should stop here.

**Final profile (`make profile`):**
- `iterate`: 76754 us, 99.76%
- `residual`: 185 us, 0.24%

**Optimizations applied this round:**
- In `fom_iterate`, `x[49..73]` (the 10 `s1` entries and the 15 `s2` entries) are now loaded once into local registers at the top of the routine.
  - Structure exploited: every row of `t0`, the `t1` rows and the `su` rows 59..73 reuse these same 25 state entries many times.
  - Before the change, the stores to `t_t0` and `new_state` could alias `state`, so the compiler reloaded `x[...]` after every store.
  - With locals, loads in `fom_iterate` drop from 518 to 432 (about 17% fewer, counted in the `-O3` assembly).
  - `make verify` still prints `VERIFY OK`, with worst error 5.3e-15.

**Already in place from earlier rounds:**
- `fom_iterate` is fully unrolled with the coefficients inlined.
- The selection rows (`su` rows 0..58 copy `t0` and `t1`) are folded into the `t0` and `t1` row computations.
- The `su_t_t0` and `t1_t_t0` products share one intermediate, `d = x[0..48] - 2*t0`, which collapses two products into one pass.
- `su_p_p0 @ p0` is computed once per solve.
- With no tolerance set, the residual is computed only after the last iteration.

**What remains:** what is left of each iteration is mostly the required writes. That is about 180 stores per iteration, for `t_t0`, `t_t1` and `new_state`.

Blocked by rules:
- Skip writing `t_t0`/`t_t1` on every iteration except the last, since in the no-tolerance loop only `new_state` carries information | `fom_iterate` must write every temporary, and new non-static entry points in `src/fom_ops.c` are not allowed | ~10-15% (removes ~59 of ~180 stores per iteration)
- Fold the constant bias `t0_c` together with `bias_su` and the bounds into one precomputed per-row vector inside `fom_iterate` | `src/fom_ops.c` may not read `FOM_ACT_*` or the constant and bound vectors, and `fom_setup` cannot call the iteration routines | unknown, small

### Round 4

**Whole-solve mean_us:** 31.794 us (translated) -> ~12.8 us final (median of 6 `make bench` runs), an overall speedup of about 2.48x. The start of this round was ~13.3 us by the same measure (13.71 us as reported), so this round gained about 4%. Benchmark noise is about ±7% per run.

**Final profile (`make profile`, instrumented -O3 build):**
- iterate: 70537 us, 99.70%
- residual: 211 us, 0.30%

**Optimizations applied (cumulative; this round's is last):**
- **Fused one-pass `fom_iterate` with literal coefficients (earlier rounds).** All matrices are fixed and sparse. Each `t0` row is `x[i] + c*x[49+g] + c_i*x[col_i]`, a three-term stencil. `su_t_t1` and `su_t_t0` on rows 0..58 are selection copies (the SELROWS structure), so the new state is written straight from the clipped values.
- **Shared intermediate `d = x - 2*t0` (earlier rounds).** The products `t1_state`, `t1_t_t0`, `su_state` and `su_t_t0` combine so that `t1` and state rows 59..73 depend on `t0` only through `d = x - 2*t0`. That makes them sums of 2 to 8 terms instead of separate products.
- **Parameter term hoisted out of the loop (earlier rounds).** `su_p_p0 @ p0` is computed once per solve as the `bias_su` vector.
- **Group-wise ordering in `fom_iterate` (this round).** The `t1_t_t0` incidence structure means each `t1` row g reads a contiguous block of `t0` rows, and each of those rows feeds exactly one `su` row in 59..73. So each block is computed together and then `t1[g]` is emitted at once. `x[49+g]` is loaded once per group, and the `d` values feed the `t1` sum and 15 running `su` accumulators immediately. The 49-entry `d[]` stack array and its spills are gone. Summation order is unchanged, so results are bit-identical.

**Considered but not done:** further work on the termination-free loop, which is already latency-bound at about 60 ns per iteration. Ideas like skipping the `t_t0`/`t_t1` writes in intermediate iterations are blocked by rules, as listed below.

Blocked by rules:
- Skip writing `t_t0`/`t_t1` on all but the last iteration (they are dead when `check_termination` is off) | `fom_iterate` must write every temporary, and any other kernel must be a non-static entry point in `src/fom_ops.c` or must not read matrix data in `src/fom_solver.c` | ~5-10% (98 of ~180 stores per iteration)
- Unroll two iterations per call to hide cross-iteration latency and reuse loads | the `fom_iterate` signature is fixed to one iteration; adding a 2-step entry point breaks "every function but these entry points is static" | unknown, maybe 5-10%
- Drop the `bias_su` additions on rows 0..58, where `su_p_p0` is zero for this data | `fom_iterate` must be exact for any bias, and specializing on the parameter matrix's zero rows inside `fom_iterate`'s bias would need reading `FOM_MAT_su_p_p0`, which is outside the iteration | ~2%
- `-O3`/`-march=native` in the bench build (the profile build already uses -O3) | build flags and Makefile are fixed | unknown
```
