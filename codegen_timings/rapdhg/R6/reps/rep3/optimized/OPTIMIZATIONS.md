# Optimization report

## Timing

```
translated (emitted)     154.8 us/solve
optimized                 13.6 us/solve
speedup                 11.41x
```

Optimization outcome: **kept (re-verified correct)**.

## Rounds

| round | before (us) | after (us) | gain | kept | agent (s) | outcome |
|---:|---:|---:|---:|:---:|---:|---|
| 1 | 154.783 | 28.921 | 81.3% | yes | 124 | continued |
| 2 | 30.238 | 15.476 | 48.8% | yes | 170 | continued |
| 3 | 14.559 | 13.571 | 6.8% | yes | 61 | continued |
| 4 | - | - | - | no | 48 | the round left the sources unchanged |

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
| 1 | Hardcoding the `t1_t_t0__d_eta` index pattern and the d_eta/d_taux values as literals in `fom_iterate`, to drop the indirect loads | not blocked; literals are allowed inside `fom_iterate`, but I ran out of the time budget | ~5–10% |
| 1 | Folding the per-iteration bias build (`a0 + tau*b0`, `a1 + eta*b1`) into `fom_iterate`, which would save two 48- and 16-element passes per iteration | fixed `fom_iterate` signature in `include/fom_ops.h` (the bias is passed in as a finished array) and the rule that `src/fom_ops.c` must not read the schedule | ~3% |
| 1 | Skipping the zero `bias_su` add in the state update | `fom_iterate` must honour any `bias_su`, and the signature is fixed | ~1–2% |
| 2 | Hoist the per-iteration bias computation (`a0 + tau*b0`, where `t0_p_p1` is all zeros) or drop it in `fom_solver.c` | `fom_solver.c` may not read `FOM_MAT_*` data for matrices it multiplies by, so it cannot see that the member is zero | under 2% |
| 2 | Precompute the circulant eigen-coefficients once at setup from the matrix data, instead of using literal tables | setup may not probe or apply matrices, and ops routines have no setup hook | none (literals already give this) |
| 3 | Dropping the per-iteration add of the all-zero su bias (160 adds) inside `fom_iterate` | fom_iterate must be exact for any bias, and fom_ops.c may not read the constant vectors | ~1-2% |
| 3 | Dropping the per-iteration bias_t0/bias_t1 FMAs, since t0_p_p1 and t1_p_p0 are zero matrices | src/fom_solver.c must not read FOM_MAT data to detect that, and every routine must stay general | ~1-2% |
| 4 | Running the whole 200-iteration loop inside `fom_ops.c` (one call, schedule values kept in registers) | `src/fom_ops.c` may not read the schedule | small (a few %), since LTO already inlines the call |
| 4 | Precomputing per-iteration biases (`tau*b0`, `eta*b1`) for all 200 iterations in `fom_update_params` | not blocked by any rule; it moves the same work and has no expected gain | ~0 |

## What the agent changed

```diff
--- src/fom_solver.c (translated)
+++ src/fom_solver.c (optimized)
@@ -2,6 +2,7 @@
 #include "fom_solver.h"
 #include "fom_data.h"
 #include "fom_ops.h"
+#include "fom_profile.h"
 
 static const fom_float FOM_ZERO_SU[FOM_STATE_DIM] = {0};
 
@@ -51,17 +52,24 @@
         fom_float *tmp;
         for (i = 0; i < FOM_T0_DIM; ++i) bias_t0[i] = a0[i] + tau * b0[i];
         for (i = 0; i < FOM_T1_DIM; ++i) bias_t1[i] = a1[i] + eta * b1[i];
+        FOM_PROF_BEGIN(iterate);
         fom_iterate(x, fom_sched_taux(it), fom_sched_tth(it), eta,
                     fom_sched_d1(it), fom_sched_g2(it), fom_sched_m1(it),
                     fom_sched_d2(it), fom_sched_g1(it), fom_sched_m2(it),
                     bias_t0, bias_t1, FOM_ZERO_SU, &FOM_ACT_t0, &FOM_ACT_t1,
                     work->t0, work->t1, xn);
+        FOM_PROF_END(iterate);
+        FOM_PROF_BEGIN(residual);
+        /* residual is only observable at termination checks and the last iteration */
+        if (use_tol || it + 1 == max_iter) {
         res = 0;
         for (i = 0; i < FOM_STATE_DIM; ++i) {
             fom_float d = xn[i] - x[i];
             if (d < 0) d = -d;
             if (d > res) res = d;
         }
+        }
+        FOM_PROF_END(residual);
         tmp = x; x = xn; xn = tmp;
         if (use_tol && res <= tol) { status = FOM_SOLVED; ++it; break; }
     }
--- src/fom_ops.c (translated)
+++ src/fom_ops.c (optimized)
@@ -1,4 +1,11 @@
 #include "fom_ops.h"
+#include "fom_profile.h"
+
+#if defined(__clang__)
+#define FOM_UNROLL_FULL _Pragma("clang loop unroll(full)")
+#else
+#define FOM_UNROLL_FULL
+#endif
 #include "fom_data.h"
 
 void fom_apply_t0_state(const fom_float *x, fom_float *out) {
@@ -416,7 +423,99 @@
 }
 
 void fom_iterate(const fom_float *state, fom_float sc_taux, fom_float sc_tth, fom_float sc_eta, fom_float sc_d1, fom_float sc_g2, fom_float sc_m1, fom_float sc_d2, fom_float sc_g1, fom_float sc_m2, const fom_float *bias_t0, const fom_float *bias_t1, const fom_float *bias_su, const FomAct_t0 *act_t0, const FomAct_t1 *act_t1, fom_float *t_t0, fom_float *t_t1, fom_float *new_state) {
-    fom_step_t0(state, sc_taux, sc_tth, bias_t0, act_t0, t_t0);
-    fom_step_t1(state, t_t0, sc_eta, bias_t1, act_t1, t_t1);
-    fom_step_su(state, t_t0, t_t1, sc_d1, sc_g2, sc_m1, sc_d2, sc_g1, sc_m2, bias_su, new_state);
-}
+    /* Selection members (t0_state, t1_state, su_state*, su_t_*) applied as
+       shifted copies; the general members unrolled over their fixed sparsity. */
+    fom_int i;
+    const fom_float *lo0 = act_t0->lo, *lo1 = act_t1->lo, *hi1 = act_t1->hi;
+    /* d_tth = -d_taux with columns shifted by 128: fold both schedule terms
+       into one 16-vector u = taux*x[0..15] - tth*x[128..143] */
+    fom_float u[16];
+    FOM_PROF_BEGIN(t0);
+    for (i = 0; i < 16; ++i) u[i] = sc_taux * state[i] - sc_tth * state[128 + i];
+    for (i = 0; i < 32; ++i) {
+        fom_float v = bias_t0[i] + state[i + 16] + FOM_MAT_t0_state__d_taux_VALS[i] * u[i & 15];
+        t_t0[i] = v < lo0[i] ? lo0[i] : v;
+    }
+    {
+        /* rows 32..39 are a Toeplitz combination of differences of u; rows
+           40..47 are their exact negatives */
+        static const fom_float C[4] = {0.07259999999999998, 0.21772, 0.36284, 0.50796};
+        fom_float r[4], q[4], A[8];
+        fom_int j, k;
+        for (j = 0; j < 4; ++j) { r[j] = u[4 * j + 3] - u[4 * j + 1]; q[j] = u[4 * j + 2] - u[4 * j]; }
+        FOM_UNROLL_FULL
+        for (j = 0; j < 4; ++j) {
+            fom_float ar = 0.0, aq = 0.0;
+            for (k = 0; k <= j; ++k) { ar += C[j - k] * r[k]; aq += C[j - k] * q[k]; }
+            A[2 * j] = ar; A[2 * j + 1] = aq;
+        }
+        for (i = 32; i < 40; ++i) {
+            fom_float v = bias_t0[i] + state[i + 16] + A[i - 32];
+            t_t0[i] = v < lo0[i] ? lo0[i] : v;
+            v = bias_t0[i + 8] + state[i + 24] - A[i - 32];
+            t_t0[i + 8] = v < lo0[i + 8] ? lo0[i + 8] : v;
+        }
+    }
+    FOM_PROF_END(t0);
+    /* t1_state__d_eta: dense 16x16 on columns 144..159 made of 4x4 blocks that
+       are symmetric circulants [a b c b]; each block is diagonal in the basis
+       (x0+x2)+-(x1+x3), x0-x2, x1-x3, so 64 mults replace 256. */
+    fom_float dd[16];
+    FOM_PROF_BEGIN(t1_dense);
+    {
+        /* per-block circulant eigen-coefficients a+c+2b, a+c-2b, a-c (exact) */
+        static const fom_float K1[16] = {-13.018647553983678, -11.523745370178833, -10.138870109850522, -8.781714635809358, -11.523745370178833, -11.322266315697716, -10.018801121546662, -8.828357091823438, -10.138870109850522, -10.018801121546662, -9.99685414987912, -8.875729076399999, -8.781714635809358, -8.828357091823438, -8.875729076399999, -9.023842};
+        static const fom_float K2[16] = {-4.97627590000001, -3.4174543279999945, -1.9958183599999995, -0.6485535999999996, -3.4174543279999945, -2.514101604000002, -1.4200451999999997, -0.4631743999999989, -1.9958183599999995, -1.4200451999999997, -0.9442720399999995, -0.2777952000000008, -0.6485535999999996, -0.4631743999999989, -0.2777952000000008, -0.1924159999999997};
+        static const fom_float K3[16] = {-78.79836558940245, -39.9150055444113, -14.486237631536042, -2.046802194016849, -39.9150055444113, -20.465349505059486, -7.435205591386116, -1.0546164120176003, -14.486237631536042, -7.435205591386116, -2.83448394521744, -0.38980525599999893, -2.046802194016849, -1.0546164120176003, -0.38980525599999893, -0.15576799999999968};
+        const fom_float *sx = state + 144;
+        fom_float ge[4], he[4], pe[4], qe[4];
+        fom_int I, J;
+        for (J = 0; J < 4; ++J) {
+            fom_float e = sx[4 * J] + sx[4 * J + 2], f = sx[4 * J + 1] + sx[4 * J + 3];
+            ge[J] = e + f; he[J] = e - f;
+            pe[J] = sx[4 * J] - sx[4 * J + 2]; qe[J] = sx[4 * J + 1] - sx[4 * J + 3];
+        }
+        FOM_UNROLL_FULL
+        for (I = 0; I < 4; ++I) {
+            fom_float G = 0.0, H = 0.0, P = 0.0, Q = 0.0, E, F;
+            for (J = 0; J < 4; ++J) {
+                G += K1[4 * I + J] * ge[J]; H += K2[4 * I + J] * he[J];
+                P += K3[4 * I + J] * pe[J]; Q += K3[4 * I + J] * qe[J];
+            }
+            E = 0.5 * (G + H); F = 0.5 * (G - H);
+            dd[4 * I] = 0.5 * (E + P); dd[4 * I + 2] = 0.5 * (E - P);
+            dd[4 * I + 1] = 0.5 * (F + Q); dd[4 * I + 3] = 0.5 * (F - Q);
+        }
+    }
+    FOM_PROF_END(t1_dense);
+    {
+        /* t1_t_t0__d_eta: -t0[i] + t0[i+16] plus a Toeplitz combination of the
+           differences t0[32+k] - t0[40+k]; rows 4m+2, 4m+3 negate rows 4m, 4m+1 */
+        static const fom_float C[4] = {0.07259999999999998, 0.21772, 0.36284, 0.50796};
+        fom_float D[8], B[8];
+        fom_int m, k;
+        for (k = 0; k < 8; ++k) D[k] = t_t0[32 + k] - t_t0[40 + k];
+        FOM_UNROLL_FULL
+        for (m = 0; m < 4; ++m) {
+            fom_float b0 = 0.0, b1 = 0.0;
+            for (k = m; k < 4; ++k) { b0 += C[k - m] * D[2 * k + 1]; b1 += C[k - m] * D[2 * k]; }
+            B[2 * m] = b0; B[2 * m + 1] = b1;
+        }
+        for (m = 0; m < 4; ++m) {
+            for (k = 0; k < 4; ++k) {
+                fom_int ii = 4 * m + k;
+                fom_float sb = (k < 2) ? B[2 * m + k] : -B[2 * m + k - 2];
+                fom_float v = bias_t1[ii] + state[ii] + sc_eta * (dd[ii] - t_t0[ii] + t_t0[ii + 16] + sb);
+                if (v < lo1[ii]) v = lo1[ii];
+                if (v > hi1[ii]) v = hi1[ii];
+                t_t1[ii] = v;
+            }
+        }
+    }
+    for (i = 0; i < 16; ++i) new_state[i] = bias_su[i] + sc_g2 * state[i + 64] + sc_g1 * t_t1[i];
+    for (i = 16; i < 64; ++i) new_state[i] = bias_su[i] + sc_g2 * state[i + 64] + sc_g1 * t_t0[i - 16];
+    for (i = 64; i < 80; ++i) new_state[i] = bias_su[i] + sc_d1 * state[i] + sc_d2 * t_t1[i - 64];
+    for (i = 80; i < 128; ++i) new_state[i] = bias_su[i] + sc_d1 * state[i] + sc_d2 * t_t0[i - 80];
+    for (i = 128; i < 144; ++i) new_state[i] = bias_su[i] + state[i - 128];
+    for (i = 144; i < 160; ++i) new_state[i] = bias_su[i] + sc_m1 * state[i - 80] + sc_m2 * t_t1[i - 144];
+}
```

## Agent notes

```
### Round 1

**Whole-solve mean_us:** 154.85 at the start, 29.03 at the end (`make bench`). That is about **5.3× faster**. `make verify` prints `VERIFY OK` (worst error 3.8e-12, tolerance 1e-6).

**Final profile (`make profile`, 200 iterations × 5000 solves; timer overhead included):**
| region | total_us | % |
|---|---|---|
| iterate (`fom_iterate`: t0, t1, state update) | 136920 | 88.9 |
| residual (now an empty branch on all but the last iteration) | 17185 | 11.1 |

**Optimizations applied (in order):**
1. **One-pass `fom_iterate` with selection products written as shifted copies (155 → 63 µs).** It no longer makes three step calls, each running about 10 generic CSR loops per row. Structure used: `t0_state`, `t1_state` and all `su_*` members are 0/1 shift selections, and the `su_t_*`, `t0_p_p1`, `t1_p_p0` and `t1_t_t0` members are zero. So t0 adds `state[i+16]`, t1 adds `state[i]`, and the state update becomes six short affine loops (g2/g1 on rows 0..63, d1/d2 on rows 64..127, a plain copy on rows 128..143, m1/m2 on rows 144..159). Only the 4 genuinely general matrices keep CSR loops.
2. **Merged the d_taux and d_tth loops into one pass (63 → 45.5 µs).** Structure used: `t0_state__d_tth` has exactly the same rowptr and colidx as `t0_state__d_taux`, shifted by 128 columns, so one index walk drives both.
3. **Dense kernel for `t1_state__d_eta`.** Structure used: it is a full 16×16 row-major block on columns 144..159, so the kernel reads `VALS[16i+k] * state[144+k]` directly with no colidx. It now uses 4 independent accumulators, which shortens the dependency chain since the compiler cannot reorder the double sum (63 → 45.5 µs, then 30.5 → 28.7 µs).
4. **Residual computed only when it is observable (45.5 → 32.9 µs).** Structure used: `FOM_USE_TOL = 0` (tol=None), so `max|Δx|` is needed only on the final iteration, for `solution.residual`. It is still computed every iteration whenever tolerance checking is on.
5. **Rows 0..31 of d_taux/d_tth handled as a vectorizable loop (32.9 → 30.5 µs).** Structure used: each of these rows has a single entry at column i mod 16 (`p = i`). Only rows 32..47 keep the CSR walk.
6. **Parameter terms hoisted.** These were already outside the loop and are unchanged. Profile markers `iterate` and `residual` are kept in `src/fom_solver.c`.

Tried and reverted: splitting the `t1_t_t0__d_eta` CSR loop into two accumulator chains made it slower (32.1 µs).

Blocked by rules:
- Hardcoding the `t1_t_t0__d_eta` index pattern and the d_eta/d_taux values as literals in `fom_iterate`, to drop the indirect loads | not blocked; literals are allowed inside `fom_iterate`, but I ran out of the time budget | ~5–10%
- Folding the per-iteration bias build (`a0 + tau*b0`, `a1 + eta*b1`) into `fom_iterate`, which would save two 48- and 16-element passes per iteration | fixed `fom_iterate` signature in `include/fom_ops.h` (the bias is passed in as a finished array) and the rule that `src/fom_ops.c` must not read the schedule | ~3%
- Skipping the zero `bias_su` add in the state update | `fom_iterate` must honour any `bias_su`, and the signature is fixed | ~1–2%

### Round 2

**Whole-solve `mean_us`:** 28.921 us at the start of round 2, 14.3 us at the end (3 bench runs: 14.28 / 14.34 / 14.38). That is a 2.02x speedup this round, and 10.8x against the translated solver (154.783 us).

**Final profile (`make profile`), hottest first.** The profile build's timer overhead inflates the solve to 38.1 us. `t0` and `t1_dense` are nested inside `iterate`.
| region | total_us | pct |
|---|---|---|
| iterate | 123850 | 66.70% |
| t0 (inside iterate) | 25181 | 13.56% |
| t1_dense (inside iterate) | 18872 | 10.16% |
| residual | 17769 | 9.57% |

**Optimizations applied (all in `fom_iterate`):**
1. **Folded `d_tth` into `d_taux`.** `t0_state__d_tth` is exactly `-t0_state__d_taux`, with its columns shifted by 128 (checked bitwise on the data). Both schedule terms collapse into one 16-vector `u = taux*x[0..15] - tth*x[128..143]`, followed by a single sparse product. This halves the t0 work: 112 multiplies instead of 224.
2. **Block-circulant trick for the 16x16 dense block.** `t1_state__d_eta` is made of 4x4 blocks, and each block is a symmetric circulant `[a b c b]` (checked exactly). Every block is diagonal in the basis `(x0+x2)±(x1+x3)`, `x0-x2`, `x1-x3`. So the product becomes 4 small coupled 4x4 sums with coefficients `a+c+2b`, `a+c-2b` and `a-c`, which are exact literal tables. This needs 64 multiplies instead of 256, and the back-transform uses only the 0.5 constant, so it stays division-free.
3. **Unrolled the fixed sparsity with literal coefficients.** This covers t0 rows 32..47 (80 nonzeros) and the `t1_t_t0__d_eta` product (112 nonzeros). Unit coefficients are written as plain adds and subtracts, and the literals are generated from `data.npz` with `repr`, so they are exact. Each row's terms are selected by a `switch` on the row index, which removes all index and rowptr loads. On its own this took the solve from about 23 us to 17 us.
4. **Forced full loop unrolling.** Clang's `loop unroll(full)` pragma, wrapped in a portable `FOM_UNROLL_FULL` macro that does nothing on other compilers, is applied to the switch-dispatched row loops and the circulant loop. The compiler then resolves every `switch` at compile time into straight-line code. This took it from 17.1 us to 14.4 us.

Blocked by rules:
- Hoist the per-iteration bias computation (`a0 + tau*b0`, where `t0_p_p1` is all zeros) or drop it in `fom_solver.c` | `fom_solver.c` may not read `FOM_MAT_*` data for matrices it multiplies by, so it cannot see that the member is zero | under 2%
- Precompute the circulant eigen-coefficients once at setup from the matrix data, instead of using literal tables | setup may not probe or apply matrices, and ops routines have no setup hook | none (literals already give this)

### Round 3

This round took the whole-solve `mean_us` from 15.476 us to about 13.0 us. `make bench` gave 13.84, 13.08 and 12.84 on three runs. That is about 1.19x for the round and 11.9x against the translated solver (154.783 us). `make verify` prints `VERIFY OK`, with a worst error of 3.8e-12 against a tolerance of 1e-6.

Profile of the final solver (`make profile`; the profiling build adds timer overhead):

| region | total_us | pct |
|---|---|---|
| iterate (whole `fom_iterate`, including the t1 rows and the state update) | 123027 | 66.7% |
| t0 | 24569 | 13.3% |
| t1_dense | 18853 | 10.2% |
| residual | 18045 | 9.8% |

Optimizations applied this round, both in `fom_iterate` in `src/fom_ops.c`:
- **t0 rows 32..47.** The folded `t0_state__d_taux`/`d_tth` product on those rows has two kinds of structure:
  - Rows 40..47 are the exact negatives of rows 32..39.
  - Rows 32..39 are a lower-triangular Toeplitz combination, with coefficients {0.0726, 0.21772, 0.36284, 0.50796}, of the differences u[4j+3]-u[4j+1] and u[4j+2]-u[4j].
  
  The new code computes 8 short Toeplitz sums once and reuses them with a flipped sign. This replaces 16 separate rows of up to 8 terms each.
- **t1 rows (`t1_t_t0__d_eta`).** The t0 entries 32..47 only ever appear as the differences t0[32+k]-t0[40+k]. They enter through the same Toeplitz coefficients. Rows 4m+2 and 4m+3 are the negatives of rows 4m and 4m+1, except for the -t0[i]+t0[i+16] selection part. The new code forms 8 differences, then 8 Toeplitz sums, then applies them to all 16 rows with a sign.

Structure exploited in earlier rounds and kept: the selection and shift copies for every su member and for t0_state/t1_state; the d_tth = -d_taux column shift folded into one vector u; the circulant 4x4 blocks of the dense `t1_state__d_eta`, diagonalized with 64 multiplies instead of 256; the hoisted parameter terms; and the residual computed only when it is observable.

Blocked by rules:
- Dropping the per-iteration add of the all-zero su bias (160 adds) inside `fom_iterate` | fom_iterate must be exact for any bias, and fom_ops.c may not read the constant vectors | ~1-2%
- Dropping the per-iteration bias_t0/bias_t1 FMAs, since t0_p_p1 and t1_p_p0 are zero matrices | src/fom_solver.c must not read FOM_MAT data to detect that, and every routine must stay general | ~1-2%

### Round 4

**Result:** no further speedup this round, so the sources are unchanged.
- Translated solver: 154.783 us. Final solver: about 13.0 us `mean_us` (three repeat runs gave 12.9 to 14.7 us). The overall speedup stays at about 11.4x to 11.9x.
- Each iteration takes about 65 ns, over 200 iterations.

**Final profile** (`make profile`). Timer overhead dominates the regions. `t0` and `t1_dense` are nested inside `iterate`.
| region | total_us | pct |
|---|---|---|
| iterate | 121698 | 66.8% |
| t0 | 24486 | 13.4% |
| t1_dense | 18528 | 10.2% |
| residual | 17571 | 9.6% |

**Optimizations in place** (from earlier rounds, kept as they are):
- **Selection products as shifted copies:** `t0_state`, `t1_state`, all `su_*` members and the selection rows `su` 128..143. Each is a 0/1 shift map.
- **Merged `t0` schedule terms:** `t0_state__d_tth` equals `-t0_state__d_taux` with columns shifted by 128. Both terms fold into one 16-vector `u = taux*x[0..15] - tth*x[128..143]`.
- **Toeplitz rows in `t0`:** rows 32..47 of `d_taux` are a lower-triangular Toeplitz combination of differences of `u`. Rows 40..47 are the exact negatives of rows 32..39, so one 4-tap convolution serves both halves.
- **Circulant blocks in `t1_state__d_eta`:** this dense 16x16 matrix is made of 4x4 symmetric circulant blocks `[a b c b]`. A fixed butterfly basis diagonalizes every block, so 64 multiplies replace 256. The eigen-coefficients are exact data.
- **Structure in `t1_t_t0__d_eta`:** it is `-t0[i] + t0[i+16]` plus a Toeplitz combination of `t0[32+k] - t0[40+k]`. Rows `4m+2` and `4m+3` negate rows `4m` and `4m+1`.
- **One fused pass:** `fom_iterate` computes `t0`, `t1` and the state update in one pass with the activations inlined. There are no zero or copy passes.
- **Hoisted parameter terms:** these are computed once per solve.
- **Residual only when needed:** it is computed only when observable (termination check or last iteration), and `tol=None` here.

**Tried this round:**
- Dropping the `a0` and `a1` bias adds, since `t0_p_p1` and `t1_p_p0` are zero matrices. It verified, but no gain showed above the ±10% benchmark noise, so I reverted it.
- The build already uses `-flto`, so the call overhead of `fom_iterate` is already gone.

Blocked by rules:
- Running the whole 200-iteration loop inside `fom_ops.c` (one call, schedule values kept in registers) | `src/fom_ops.c` may not read the schedule | small (a few %), since LTO already inlines the call
- Precomputing per-iteration biases (`tau*b0`, `eta*b1`) for all 200 iterations in `fom_update_params` | not blocked by any rule; it moves the same work and has no expected gain | ~0
```
