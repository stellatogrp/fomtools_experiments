# Optimization report

## Timing

```
translated (emitted)     138.7 us/solve
optimized                 12.0 us/solve
speedup                 11.57x
```

Optimization outcome: **kept (re-verified correct)**.

## Rounds

| round | before (us) | after (us) | gain | kept | agent (s) | outcome |
|---:|---:|---:|---:|:---:|---:|---|
| 1 | 138.687 | 17.642 | 87.3% | yes | 96 | continued |
| 2 | 13.568 | 11.984 | 11.7% | yes | 104 | continued |
| 3 | - | - | - | no | 117 | the round left the sources unchanged |

## Kernel checks

Each `fom_apply_<name>` in the shipped `fom_ops.c`, checked by `make verify` against the package's reference over the same `fom_data.c` matrix. *basis* probes every unit vector, which recovers the routine's matrix column by column (an exact check), plus random vectors; *random* uses random vectors only (the matrix is over the probe budget). A `fused_<block>` row is a `fom_fused_<block>`, checked against the sum of its members: *basis* there probes every unit vector of each source with every schedule value at 0 and with each alone at 1, which recovers every member exactly.

| matrix | shape | nnz | rewritten | check | probes | max error |
|---|---|---|---|---|---|---|
| `t0_state` | 48x160 | 48 | yes | basis | 162 | 0.000e+00 |
| `t0_state__d_taux` | 48x160 | 112 | yes | basis | 162 | 0.000e+00 |
| `t0_state__d_tth` | 48x160 | 112 | yes | basis | 162 | 0.000e+00 |
| `t0_p_p1` | 48x48 | 0 | yes | basis | 50 | 0.000e+00 |
| `t0_p_p1__d_tau` | 48x48 | 48 | yes | basis | 50 | 0.000e+00 |
| `t1_state` | 16x160 | 16 | yes | basis | 162 | 0.000e+00 |
| `t1_state__d_eta` | 16x160 | 256 | yes | basis | 162 | 0.000e+00 |
| `t1_p_p0` | 16x16 | 0 | yes | basis | 18 | 0.000e+00 |
| `t1_p_p0__d_eta` | 16x16 | 16 | yes | basis | 18 | 0.000e+00 |
| `t1_t_t0` | 16x48 | 0 | yes | basis | 50 | 0.000e+00 |
| `t1_t_t0__d_eta` | 16x48 | 112 | yes | basis | 50 | 0.000e+00 |
| `su_state` | 160x160 | 16 | yes | basis | 162 | 0.000e+00 |
| `su_state__d_d1` | 160x160 | 64 | yes | basis | 162 | 0.000e+00 |
| `su_state__d_g2` | 160x160 | 64 | yes | basis | 162 | 0.000e+00 |
| `su_state__d_m1` | 160x160 | 16 | yes | basis | 162 | 0.000e+00 |
| `su_t_t0` | 160x48 | 0 | yes | basis | 50 | 0.000e+00 |
| `su_t_t0__d_d2` | 160x48 | 48 | yes | basis | 50 | 0.000e+00 |
| `su_t_t0__d_g1` | 160x48 | 48 | yes | basis | 50 | 0.000e+00 |
| `su_t_t1` | 160x16 | 0 | yes | basis | 18 | 0.000e+00 |
| `su_t_t1__d_d2` | 160x16 | 16 | yes | basis | 18 | 0.000e+00 |
| `su_t_t1__d_g1` | 160x16 | 16 | yes | basis | 18 | 0.000e+00 |
| `su_t_t1__d_m2` | 160x16 | 16 | yes | basis | 18 | 0.000e+00 |
| `fused_t0` | 48x160 | 272 | yes | basis | 482 | 0.000e+00 |
| `fused_t1` | 16x208 | 384 | yes | basis | 418 | 0.000e+00 |
| `fused_su` | 160x224 | 304 | yes | basis | 1570 | 0.000e+00 |
| `step_t0` | 48x160 | 272 | yes | basis | 486 | 0.000e+00 |
| `step_t1` | 16x208 | 384 | yes | basis | 422 | 0.000e+00 |
| `step_su` | 160x224 | 304 | yes | basis | 1570 | 0.000e+00 |
| `fused_su__rest` | 160x224 | 304 | yes | basis | 1570 | 0.000e+00 |
| `step_su__rest` | 160x224 | 304 | yes | basis | 1570 | 0.000e+00 |
| `iterate` | 160x160 | 960 | yes | basis | 1766 | 1.137e-13 |

### Rule violations

_None: the collected rewrite passed the rules and was kept._

## Blocked by rules

Optimizations the agent considered but did not make because a rule forbade them, in its own words.

| round | idea | rule | expected gain |
|---|---|---|---|
| 1 | Skip adding `bias_su` inside `fom_iterate`, since it is identically zero for this problem | fom_iterate must be correct for arbitrary biases (verify uses random biases) | ~1–2% |
| 1 | Precompute per-iteration `b0 = c0 + tau*d0` and `b1` tables for all 200 iterations in `fom_update_params` | not attempted (time budget), and it would need workspace scratch beyond `acc`, and the workspace header is fixed | ~2–4% |
| 3 | Skip the zero `bias_su` loads and adds in `fom_iterate` (su has no constant or parameter terms) | `fom_iterate` must apply any bias it is passed (verify uses random biases) | about 1-2% |
| 3 | Precompute every iteration's t0/t1 bias vectors (c + tau*d) at setup | fixed workspace and headers, no dynamic allocation | under 2% |
| 3 | Read the schedule tables directly or hoist schedule values across iterations inside `fom_ops.c` | nothing in `src/fom_ops.c` may read the schedule | unknown, probably small |

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
@@ -416,7 +417,155 @@
 }
 
 void fom_iterate(const fom_float *state, fom_float sc_taux, fom_float sc_tth, fom_float sc_eta, fom_float sc_d1, fom_float sc_g2, fom_float sc_m1, fom_float sc_d2, fom_float sc_g1, fom_float sc_m2, const fom_float *bias_t0, const fom_float *bias_t1, const fom_float *bias_su, const FomAct_t0 *act_t0, const FomAct_t1 *act_t1, fom_float *t_t0, fom_float *t_t1, fom_float *new_state) {
-    fom_step_t0(state, sc_taux, sc_tth, bias_t0, act_t0, t_t0);
-    fom_step_t1(state, t_t0, sc_eta, bias_t1, act_t1, t_t1);
-    fom_step_su(state, t_t0, t_t1, sc_d1, sc_g2, sc_m1, sc_d2, sc_g1, sc_m2, bias_su, new_state);
-}
+    fom_int i;
+    fom_float w[16], u[48], wm[16], e[8];
+    const fom_float *xm = state + 144;
+    FOM_PROF_BEGIN(t0);
+    /* d_tth's block on state[128..143] is the negated d_taux block on
+       state[0..15], so both collapse into one 48x16 product, unrolled
+       over its fixed sparsity with the coefficients inlined. */
+    for (i = 0; i < 16; ++i) w[i] = sc_taux * state[i] - sc_tth * state[i + 128];
+    u[0] = w[0];
+    u[1] = w[1];
+    u[2] = w[2];
+    u[3] = w[3];
+    u[4] = w[4];
+    u[5] = w[5];
+    u[6] = w[6];
+    u[7] = w[7];
+    u[8] = w[8];
+    u[9] = w[9];
+    u[10] = w[10];
+    u[11] = w[11];
+    u[12] = w[12];
+    u[13] = w[13];
+    u[14] = w[14];
+    u[15] = w[15];
+    u[16] = -w[0];
+    u[17] = -w[1];
+    u[18] = -w[2];
+    u[19] = -w[3];
+    u[20] = -w[4];
+    u[21] = -w[5];
+    u[22] = -w[6];
+    u[23] = -w[7];
+    u[24] = -w[8];
+    u[25] = -w[9];
+    u[26] = -w[10];
+    u[27] = -w[11];
+    u[28] = -w[12];
+    u[29] = -w[13];
+    u[30] = -w[14];
+    u[31] = -w[15];
+    u[32] = -0.07259999999999998 * w[1] + 0.07259999999999998 * w[3];
+    u[33] = -0.07259999999999998 * w[0] + 0.07259999999999998 * w[2];
+    u[34] = -0.21772 * w[1] + 0.21772 * w[3] - 0.07259999999999998 * w[5] + 0.07259999999999998 * w[7];
+    u[35] = -0.21772 * w[0] + 0.21772 * w[2] - 0.07259999999999998 * w[4] + 0.07259999999999998 * w[6];
+    u[36] = -0.36284 * w[1] + 0.36284 * w[3] - 0.21772 * w[5] + 0.21772 * w[7] - 0.07259999999999998 * w[9] + 0.07259999999999998 * w[11];
+    u[37] = -0.36284 * w[0] + 0.36284 * w[2] - 0.21772 * w[4] + 0.21772 * w[6] - 0.07259999999999998 * w[8] + 0.07259999999999998 * w[10];
+    u[38] = -0.50796 * w[1] + 0.50796 * w[3] - 0.36284 * w[5] + 0.36284 * w[7] - 0.21772 * w[9] + 0.21772 * w[11] - 0.07259999999999998 * w[13] + 0.07259999999999998 * w[15];
+    u[39] = -0.50796 * w[0] + 0.50796 * w[2] - 0.36284 * w[4] + 0.36284 * w[6] - 0.21772 * w[8] + 0.21772 * w[10] - 0.07259999999999998 * w[12] + 0.07259999999999998 * w[14];
+    u[40] = -u[32];
+    u[41] = -u[33];
+    u[42] = -u[34];
+    u[43] = -u[35];
+    u[44] = -u[36];
+    u[45] = -u[37];
+    u[46] = -u[38];
+    u[47] = -u[39];
+    for (i = 0; i < 48; ++i) {
+        fom_float v = bias_t0[i] + state[i + 16] + u[i];
+        if (v < act_t0->lo[i]) v = act_t0->lo[i];
+        t_t0[i] = v;
+    }
+    FOM_PROF_END(t0);
+    FOM_PROF_BEGIN(t1);
+    /* d_eta products: dense 16x16 block on state[144..159] plus t0 term.
+       Its t0 columns 32+j and 40+j carry opposite coefficients. */
+    for (i = 0; i < 8; ++i) e[i] = t_t0[32 + i] - t_t0[40 + i];
+    {
+        /* Each 4x4 block of the 16x16 d_eta block on state[144..159] is
+           [a b c b; b a b c; c b a b; b c b a], so in sum/difference
+           coordinates (x0+-x2, x1+-x3) it splits into decoupled parts. */
+        static const fom_float A[4][4] = {
+            {-43.89791365819715, -23.69280269675036, -10.276790933230652, -3.380968155960764},
+            {-23.69280269675036, -13.691766732454171, -6.577314376079723, -2.85019107896466},
+            {-10.276790933230652, -6.577314376079723, -4.1525235200785, -2.4832836970999996},
+            {-3.380968155960764, -2.85019107896466, -2.4832836970999996, -2.3819484999999996}};
+        static const fom_float B[4][4] = {
+            {-2.010592913495917, -2.0265727605447097, -2.0357629374626307, -2.0332902589523396},
+            {-2.0265727605447097, -2.2020411779244284, -2.1496889803866654, -2.09129567295586},
+            {-2.0357629374626307, -2.1496889803866654, -2.26314552746978, -2.1494834690999998},
+            {-2.0332902589523396, -2.09129567295586, -2.1494834690999998, -2.2078565}};
+        static const fom_float C[4][4] = {
+            {34.9004519312053, 16.222202847660945, 4.209446698305391, -1.3341659619439152},
+            {16.222202847660945, 6.773582772605312, 0.8578912153063928, -1.7955746669470596},
+            {4.209446698305391, 0.8578912153063928, -1.31803957486106, -2.0934784411000007},
+            {-1.3341659619439152, -1.7955746669470596, -2.0934784411000007, -2.2261805}};
+        fom_float sv[4], dv[4], pv[4], qv[4];
+        fom_int I, J;
+        for (J = 0; J < 4; ++J) {
+            sv[J] = xm[4 * J] + xm[4 * J + 2]; dv[J] = xm[4 * J] - xm[4 * J + 2];
+            pv[J] = xm[4 * J + 1] + xm[4 * J + 3]; qv[J] = xm[4 * J + 1] - xm[4 * J + 3];
+        }
+        for (I = 0; I < 4; ++I) {
+            fom_float E = 0, F = 0, G = 0, H = 0;
+            for (J = 0; J < 4; ++J) {
+                const fom_float ap = A[I][J] + C[I][J], am = A[I][J] - C[I][J], b2 = 2.0 * B[I][J];
+                E += ap * sv[J] + b2 * pv[J];
+                G += ap * pv[J] + b2 * sv[J];
+                F += am * dv[J];
+                H += am * qv[J];
+            }
+            wm[4 * I] = 0.5 * (E + F); wm[4 * I + 2] = 0.5 * (E - F);
+            wm[4 * I + 1] = 0.5 * (G + H); wm[4 * I + 3] = 0.5 * (G - H);
+        }
+    }
+    w[0] = wm[0]
+        + -t_t0[0] + t_t0[16] + 0.07259999999999998 * e[1] + 0.21772 * e[3] + 0.36284 * e[5] + 0.50796 * e[7];
+    w[1] = wm[1]
+        + -t_t0[1] + t_t0[17] + 0.07259999999999998 * e[0] + 0.21772 * e[2] + 0.36284 * e[4] + 0.50796 * e[6];
+    w[2] = wm[2]
+        + -t_t0[2] + t_t0[18] - 0.07259999999999998 * e[1] - 0.21772 * e[3] - 0.36284 * e[5] - 0.50796 * e[7];
+    w[3] = wm[3]
+        + -t_t0[3] + t_t0[19] - 0.07259999999999998 * e[0] - 0.21772 * e[2] - 0.36284 * e[4] - 0.50796 * e[6];
+    w[4] = wm[4]
+        + -t_t0[4] + t_t0[20] + 0.07259999999999998 * e[3] + 0.21772 * e[5] + 0.36284 * e[7];
+    w[5] = wm[5]
+        + -t_t0[5] + t_t0[21] + 0.07259999999999998 * e[2] + 0.21772 * e[4] + 0.36284 * e[6];
+    w[6] = wm[6]
+        + -t_t0[6] + t_t0[22] - 0.07259999999999998 * e[3] - 0.21772 * e[5] - 0.36284 * e[7];
+    w[7] = wm[7]
+        + -t_t0[7] + t_t0[23] - 0.07259999999999998 * e[2] - 0.21772 * e[4] - 0.36284 * e[6];
+    w[8] = wm[8]
+        + -t_t0[8] + t_t0[24] + 0.07259999999999998 * e[5] + 0.21772 * e[7];
+    w[9] = wm[9]
+        + -t_t0[9] + t_t0[25] + 0.07259999999999998 * e[4] + 0.21772 * e[6];
+    w[10] = wm[10]
+        + -t_t0[10] + t_t0[26] - 0.07259999999999998 * e[5] - 0.21772 * e[7];
+    w[11] = wm[11]
+        + -t_t0[11] + t_t0[27] - 0.07259999999999998 * e[4] - 0.21772 * e[6];
+    w[12] = wm[12]
+        + -t_t0[12] + t_t0[28] + 0.07259999999999998 * e[7];
+    w[13] = wm[13]
+        + -t_t0[13] + t_t0[29] + 0.07259999999999998 * e[6];
+    w[14] = wm[14]
+        + -t_t0[14] + t_t0[30] - 0.07259999999999998 * e[7];
+    w[15] = wm[15]
+        + -t_t0[15] + t_t0[31] - 0.07259999999999998 * e[6];
+    for (i = 0; i < 16; ++i) {
+        fom_float v = bias_t1[i] + state[i] + sc_eta * w[i];
+        if (v < act_t1->lo[i]) v = act_t1->lo[i];
+        if (v > act_t1->hi[i]) v = act_t1->hi[i];
+        t_t1[i] = v;
+    }
+    FOM_PROF_END(t1);
+    FOM_PROF_BEGIN(su);
+    for (i = 0; i < 16; ++i) new_state[i] = bias_su[i] + sc_g2 * state[i + 64] + sc_g1 * t_t1[i];
+    for (i = 16; i < 64; ++i) new_state[i] = bias_su[i] + sc_g2 * state[i + 64] + sc_g1 * t_t0[i - 16];
+    for (i = 64; i < 80; ++i) new_state[i] = bias_su[i] + sc_d1 * state[i] + sc_d2 * t_t1[i - 64];
+    for (i = 80; i < 128; ++i) new_state[i] = bias_su[i] + sc_d1 * state[i] + sc_d2 * t_t0[i - 80];
+    for (i = 128; i < 144; ++i) new_state[i] = bias_su[i] + state[i - 128];
+    for (i = 144; i < 160; ++i) new_state[i] = bias_su[i] + sc_m1 * state[i - 80] + sc_m2 * t_t1[i - 144];
+    FOM_PROF_END(su);
+}
```

## Agent notes

```
### Round 1

**Whole-solve mean_us:** initial 139.52 → final 13.63 (13.5–13.9 over repeated runs). Overall speedup is about **10.2×**.

**Final profile breakdown** (`make profile`, instrumented build; the timers themselves cost a lot at this scale, so the instrumented run reports mean_us=28.6):
| region | total_us | % |
|---|---|---|
| su (state update) | 31209 | 42.4 |
| t0 (relu step) | 22888 | 31.1 |
| t1 (clip step) | 19440 | 26.4 |

**Optimizations applied:**
1. **One-pass `fom_iterate` (139.5 → 50.1 us).** This replaces the three generic step routines. Each of them ran 3 to 9 CSR loops per row, and all but a few of those multiplied by a 0/1 selection or by zero.
   - The selection products are now direct indexed copies, using the shift maps from SPEC.md: t0 += state[i+16], t1 += state[i], and the su shifts.
   - The `su` update is six branch-free, vectorizable segment loops: rows 0–15, 16–63, 64–79, 80–127, 128–143 and 144–159.
   - The zero matrices (`t0_p_p1`, `t1_p_p0`, `t1_t_t0`, `su_t_t0`, `su_t_t1`) are dropped.
   - Bias and activation are applied in the same pass, and results are written straight into t_t0, t_t1 and new_state.
2. **Collapsing two t0 products into one (50.1 → 43.4 us).** The `t0_state__d_tth` block on state[128..143] is exactly the negation of the `t0_state__d_taux` block on state[0..15]. So `taux*A x + tth*B x = A_blk (taux*x[0:16] - tth*x[128:144])`. That is one 16-vector combine plus one 48×16 product of 112 nonzeros, in place of two. It holds for any schedule values.
3. **Unrolling the remaining products with the coefficients inlined (43.4 → 13.6 us).** This covers the 48×16 t0 block (112 nonzeros), the dense 16×16 `t1_state__d_eta` block (it only touches state[144..159]) and `t1_t_t0__d_eta` (112 nonzeros).
   - Each was unrolled over its fixed sparsity, with round-trip-exact values, inside `fom_iterate`. This is allowed there because verify checks it on every unit vector.
   - This removes all index and rowptr loads and the gathers.
   - Both eta terms share one multiply by `sc_eta`.

**Kept as before:** the parameter terms are still computed once per solve, outside the loop. The residual is still computed only on the last iteration, since there is no tolerance check.

Blocked by rules:
- Skip adding `bias_su` inside `fom_iterate`, since it is identically zero for this problem | fom_iterate must be correct for arbitrary biases (verify uses random biases) | ~1–2%
- Precompute per-iteration `b0 = c0 + tau*d0` and `b1` tables for all 200 iterations in `fom_update_params` | not attempted (time budget), and it would need workspace scratch beyond `acc`, and the workspace header is fixed | ~2–4%

### Round 2

**Whole-solve `mean_us`**
- Reported at the start of this round: 17.642 us.
- Same solver re-measured this session: 13.5–15.8 us (noisy machine).
- Final: 11.7–12.4 us across runs, about 12.0 us typical.
- Speedup this round: about 1.1x against the re-measured baseline (1.47x against the reported 17.642 us).
- Overall against the translated solver (138.687 us): about 11.5x.

**Final profile (`make profile`, hottest first)**

| Region | Time | Share |
|---|---|---|
| su | 30838 us | 42.3% |
| t0 | 23341 us | 32.0% |
| t1 | 18712 us | 25.7% |

The profile total is inflated by timer overhead (1,000,000 region calls). The shares still show that the three blocks cost about the same.

**Optimizations applied**
- **t1, dense 16x16 d_eta block on `state[144..159]`:** every 4x4 block of it has the form `[a b c b; b a b c; c b a b; b c b a]`.
  - In sum/difference coordinates (`x0±x2`, `x1±x3`) it splits into one coupled 8x8 part (coefficients `a+c` and `2b`) and two 4x4 parts that share the coefficients `a-c`.
  - The outputs are recovered with a factor of 0.5.
  - This cuts 256 multiply-adds to about 96.
  - The coefficients are kept as `static const` tables inside `fom_iterate`.
  - The largest error verify reported for the per-iteration routine (`iterate`) is 1.7e-13.
- **t1, the `t_t0` term:** columns `32+j` and `40+j` always carry opposite coefficients.
  - So `e[j] = t_t0[32+j] - t_t0[40+j]` is formed once.
  - That halves the multiplies of this term.
- **t0:** rows 40..47 of the collapsed `d_taux`/`d_tth` product are the negation of rows 32..39, so they are written as `u[40+k] = -u[32+k]`. On its own this measured about neutral; it is a small cleanup.

**Blocked by rules:**
- none

### Round 3

**Result: no further speedup found; sources left unchanged from round 2.**

- Initial (translated) solver: 138.687 us. Start of this round: 11.984 us. Final: about 11.7 us (`make bench` mean_us, runs vary between 11.6 and 12.0 us). That is 11.6x to 11.9x overall, unchanged this round.
- Final profile (`make profile`, instrumented build, hottest first):
  - su: 32375 us, 42.45%
  - t0: 24333 us, 31.90%
  - t1: 19562 us, 25.65%

Optimizations tried this round (all verified, none kept):
- **t0 rows 32..39 rewritten in difference form.** These rows depend only on the differences w[4j+3]-w[4j+1] and w[4j+2]-w[4j], with Toeplitz coefficients c0..c3. Precomputing the 8 differences cuts the multiplies by about half.
- **Shared sums for the t0 terms of t1.** In rows 4k..4k+3 these terms are +P, +Q, -P, -Q of shared Toeplitz sums of e = t0[32..39] - t0[40..47]. Computing P and Q once removes the duplicated work.
- **Precomputed tables for the dense d_eta block.** The block's sum/difference split uses A+C, A-C and 2B. I stored these as static constant tables instead of computing them on every call.
- In the interleaved A/B test (8 runs each), the rewrite averaged 11.90 us against 11.72 us for the existing code, so I reverted it. The iteration is about 60 ns. It is limited by the dependent chain from t0 (clamp) to t1 (clip) to the next state, and by load/store traffic, not by FLOPs.

Optimizations carried over from earlier rounds (still in place):
- **One-pass iteration in `fom_iterate`.** All three blocks run in one call, and the selection products and selection rows are written as direct shifts and copies.
- **t0: d_tth folded into d_taux.** The d_tth block on state[128..143] is the negated d_taux block on state[0..15], so both collapse into one 48x16 product, unrolled with inlined coefficients. Rows 16..31 and 40..47 are signed copies of other rows.
- **t1: dense 16x16 d_eta block decoupled.** Its 4x4 blocks have the pattern [a b c b; b a b c; c b a b; b c b a]. In sum/difference coordinates it splits into independent parts.
- **Bias terms computed once per solve.** The parameter terms are applied once per solve, and the bias for each iteration is just c + s*d. The residual is computed only on the last iteration, because tol=None.

Blocked by rules:
- Skip the zero `bias_su` loads and adds in `fom_iterate` (su has no constant or parameter terms) | `fom_iterate` must apply any bias it is passed (verify uses random biases) | about 1-2%
- Precompute every iteration's t0/t1 bias vectors (c + tau*d) at setup | fixed workspace and headers, no dynamic allocation | under 2%
- Read the schedule tables directly or hoist schedule values across iterations inside `fom_ops.c` | nothing in `src/fom_ops.c` may read the schedule | unknown, probably small
```
