# Optimization report

## Timing

```
translated (emitted)    1090.9 us/solve
optimized                 49.2 us/solve
speedup                 22.19x
```

Optimization outcome: **kept (re-verified correct)**.

## Rounds

| round | before (us) | after (us) | gain | kept | agent (s) | outcome |
|---:|---:|---:|---:|:---:|---:|---|
| 1 | 1090.903 | 69.718 | 93.6% | yes | 920 | continued |
| 2 | 72.941 | 65.338 | 10.4% | yes | 1749 | continued (agent stopped at its budget) |
| 3 | 66.089 | 50.746 | 23.2% | yes | 1129 | continued |
| 4 | 52.201 | 49.395 | 5.4% | yes | 748 | continued |
| 5 | 49.673 | 49.166 | 1.0% | yes | 329 | the round gained less than agent_optimize_min_gain |

## Caveats

- The agent was terminated at its wall-clock budget before it finished (in round 2), so it may never have written a summary. The timing and diff below are still sound: the solver on disk was re-verified and benchmarked. Raise `CodegenConfig.agent_optimize_timeout_s` to give it longer.

## Kernel checks

Each `fom_apply_<name>` in the shipped `fom_ops.c`, checked by `make verify` against the package's reference over the same `fom_data.c` matrix. *basis* probes every unit vector, which recovers the routine's matrix column by column (an exact check), plus random vectors; *random* uses random vectors only (the matrix is over the probe budget). A `fused_<block>` row is a `fom_fused_<block>`, checked against the sum of its members: *basis* there probes every unit vector of each source with every schedule value at 0 and with each alone at 1, which recovers every member exactly.

| matrix | shape | nnz | rewritten | check | probes | max error |
|---|---|---|---|---|---|---|
| `t0_state` | 120x400 | 120 | yes | basis | 402 | 0.000e+00 |
| `t0_state__d_taux` | 120x400 | 520 | yes | basis | 402 | 0.000e+00 |
| `t0_state__d_tth` | 120x400 | 520 | yes | basis | 402 | 0.000e+00 |
| `t0_p_p1` | 120x120 | 0 | yes | basis | 122 | 0.000e+00 |
| `t0_p_p1__d_tau` | 120x120 | 120 | yes | basis | 122 | 0.000e+00 |
| `t1_state` | 40x400 | 40 | yes | basis | 402 | 0.000e+00 |
| `t1_state__d_eta` | 40x400 | 1600 | yes | basis | 402 | 0.000e+00 |
| `t1_p_p0` | 40x40 | 0 | yes | basis | 42 | 0.000e+00 |
| `t1_p_p0__d_eta` | 40x40 | 40 | yes | basis | 42 | 0.000e+00 |
| `t1_t_t0` | 40x120 | 0 | yes | basis | 122 | 0.000e+00 |
| `t1_t_t0__d_eta` | 40x120 | 520 | yes | basis | 122 | 0.000e+00 |
| `su_state` | 400x400 | 40 | yes | basis | 402 | 0.000e+00 |
| `su_state__d_d1` | 400x400 | 160 | yes | basis | 402 | 0.000e+00 |
| `su_state__d_g2` | 400x400 | 160 | yes | basis | 402 | 0.000e+00 |
| `su_state__d_m1` | 400x400 | 40 | yes | basis | 402 | 0.000e+00 |
| `su_t_t0` | 400x120 | 0 | yes | basis | 122 | 0.000e+00 |
| `su_t_t0__d_d2` | 400x120 | 120 | yes | basis | 122 | 0.000e+00 |
| `su_t_t0__d_g1` | 400x120 | 120 | yes | basis | 122 | 0.000e+00 |
| `su_t_t1` | 400x40 | 0 | yes | basis | 42 | 0.000e+00 |
| `su_t_t1__d_d2` | 400x40 | 40 | yes | basis | 42 | 0.000e+00 |
| `su_t_t1__d_g1` | 400x40 | 40 | yes | basis | 42 | 0.000e+00 |
| `su_t_t1__d_m2` | 400x40 | 40 | yes | basis | 42 | 0.000e+00 |
| `fused_t0` | 120x400 | 1160 | yes | basis | 1202 | 0.000e+00 |
| `fused_t1` | 40x520 | 2160 | yes | basis | 1042 | 0.000e+00 |
| `fused_su` | 400x560 | 760 | yes | basis | 3922 | 0.000e+00 |
| `step_t0` | 120x400 | 1160 | yes | basis | 1206 | 3.553e-15 |
| `step_t1` | 40x520 | 2160 | yes | basis | 1046 | 1.455e-11 |
| `step_su` | 400x560 | 760 | yes | basis | 3922 | 0.000e+00 |
| `fused_su__rest` | 400x560 | 760 | yes | basis | 3922 | 0.000e+00 |
| `step_su__rest` | 400x560 | 760 | yes | basis | 3922 | 0.000e+00 |
| `iterate` | 400x400 | 4080 | yes | basis | 4406 | 1.091e-11 |

### Rule violations

_None: the collected rewrite passed the rules and was kept._

## Blocked by rules

Optimizations the agent considered but did not make because a rule forbade them, in its own words.

| round | idea | rule | expected gain |
|---|---|---|---|
| 1 | Pass the parameter bias as two vectors (A, B) plus the scalar tau/eta into `fom_iterate`, removing the per-iteration bias pass | headers / `fom_iterate` signature are fixed | ~10% (measured by removing the pass) |
| 1 | Use the near-arithmetic-progression of the Toeplitz coefficients (c_j ≈ c_1 + (j−1)δ for j ≥ 1) for an O(n) running-sum convolution | literal-exactness rule (δ is not a matrix entry, and the recurrence is not bit-exact to the stored coefficients) | ~5% |
| 1 | Fuse two iterations per `fom_iterate` call to overlap their dependency chains | `fom_iterate` signature (one iteration) is fixed | unknown |
| 1 | Build flags such as `-mcpu=native` or ffast-math style flags to get better code generation for the scalarized table products | Makefile / build flags are fixed | unknown |
| 3 | Precompute the bias vectors for the 64 distinct `tau`/`eta` values of a restart epoch once in `fom_update_params`, so the loop does no bias pass | `FomWorkspace` in the fixed header `include/fom_workspace.h` has no room for an 80 KB table, and a static table in the solver would be hidden, non-reentrant state | about 5 us (about 10%) |
| 3 | One fused `fom_iterate` pass that keeps `t0`, `t1` and `s0` in registers and skips the reloads | the fixed `fom_iterate` signature in `include/fom_ops.h` makes it add a `bias_su` vector (400 loads, all zeros here) every iteration | unknown; it measured about 3 us slower with the forced `bias_su` |
| 3 | Drop the `s4 = s0` copy by aliasing the state buffers | the fixed step signatures read `s0`, `s1`, `s4` and `s5` at fixed offsets from a single state pointer | about 1 us |
| 3 | Force inlining and per-call constant handling through build flags (e.g. `-mcpu`, inlining controls) instead of source pragmas | Makefile and build flags are fixed | unknown |
| 4 | Precompute the 64 distinct per-tau bias vectors once in `fom_update_params` into a static table, removing about 80 stores per iteration | workspace layout (`include/fom_workspace.h`) is fixed and the table (about 80 KB) does not fit its scratch. A static table outside the workspace is not reentrant, and the work would only move out of the timed region | about 4% of solve time, roughly zero net once update_params is counted |
| 4 | Store t1 once into a layout where it is both s0 of the next state and s4 of the one after, removing the s4 copy (about 5%) | the `fom_step_*` signatures fix a contiguous 400-entry state with s4 at offset 320, so the two-buffer ping-pong cannot alias s4 to the previous s0 | about 5% |
| 4 | Fold the bias add, the s2/s5 update and the s4 copy into the step_t1 output pass, so t1 never makes a round trip through memory | step signatures are fixed (step_t1 writes only t1), and the only fused alternative, `fom_iterate`, must also add bias_su across all 400 rows | unknown, probably 3–5% |
| 5 | Precompute one bias vector per distinct tau/eta schedule value (64 of each) in `fom_update_params`, removing the per-iteration bias pass | the workspace struct is in a fixed header with no room for a table (about 80 KB), and a static table in `fom_solver.c` would be large and not safe with more than one workspace | about 5 µs (≈10%), going by the knock-out test |
| 5 | Fold the scalar tau/eta bias scaling into `fom_step_t0`/`fom_step_t1` so no bias vector is written at all | the fixed step signatures take a bias vector, not a scale factor plus a base vector | up to about 6 µs (≈12%) |
| 5 | Advance s3 four steps at a time instead of two | t0 is written into the state buffer that the next step routine reads at a fixed offset, so keeping older t0 values needs a third state buffer, which the fixed workspace doesn't have | about 3 µs |
| 5 | Store s3 as an unnormalized running sum and rescale it only at restarts | the rescale needs division by schedule-dependent factors (division-free target) | about 3 µs |

## What the agent changed

```diff
--- src/fom_solver.c (translated)
+++ src/fom_solver.c (optimized)
@@ -1,14 +1,15 @@
 #include <string.h>
+#include <math.h>
 #include "fom_solver.h"
 #include "fom_data.h"
 #include "fom_ops.h"
+#include "fom_profile.h"
 
 /* acc layout: parameter terms, hoisted out of the loop. */
-#define B0A (0)    /* t0_p_p1 @ p1          (120) */
+/* t0_p_p1 and t1_p_p0 are zero matrices, so the bias is only the scaled terms. */
 #define B0B (120)  /* t0_p_p1__d_tau @ p1   (120) */
-#define B1A (240)  /* t1_p_p0 @ p0          (40)  */
 #define B1B (280)  /* t1_p_p0__d_eta @ p0   (40)  */
-#define BT1 (320)  /* per-iteration bias_t1 (40)  */
+#define BH (FOM_T0_DIM + FOM_T1_DIM)  /* offset of the odd-iteration biases in solution.x */
 
 void fom_set_default_settings(FomSettings *settings)
 {
@@ -21,9 +22,7 @@
 {
     fom_float *acc = work->acc;
     memset(acc, 0, sizeof(work->acc));
-    fom_apply_t0_p_p1(work->params.p1, acc + B0A);
     fom_apply_t0_p_p1__d_tau(work->params.p1, acc + B0B);
-    fom_apply_t1_p_p0(work->params.p0, acc + B1A);
     fom_apply_t1_p_p0__d_eta(work->params.p0, acc + B1B);
 }
 
@@ -45,50 +44,190 @@
     fom_hoist_params(work);
 }
 
+/* State update rows 0..159 (selection products): new s0 = g1 t1 + g2 s2,
+   new s1 = g1 t0 + g2 s3. */
+static void fom_update_lo(const fom_float *restrict x, const fom_float *restrict t0,
+                          const fom_float *restrict t1, fom_float g1, fom_float g2,
+                          fom_float *restrict xn)
+{
+    fom_int i;
+    for (i = 0; i < 40; ++i)
+        xn[i] = g1 * t1[i] + g2 * x[160 + i];
+    for (i = 0; i < 120; ++i)
+        xn[40 + i] = g1 * t0[i] + g2 * x[200 + i];
+}
+
+/* State update rows 200..319 (selection products): new s3 = d1 s3 + d2 t0.
+   Needs only t0, so it runs while t1 is still being computed. */
+static void fom_update_s3(const fom_float *restrict x, const fom_float *restrict t0,
+                          fom_float d1, fom_float d2, fom_float *restrict s3n)
+{
+    fom_int i;
+    for (i = 0; i < 120; ++i)
+        s3n[i] = d1 * x[200 + i] + d2 * t0[i];
+}
+
+/* Two s3 steps in one pass: s3 <- a s3 + b u + c v, in place (u, v: two t0s). */
+static void fom_update_s3x2(fom_float *restrict s3, const fom_float *restrict u,
+                            const fom_float *restrict v, fom_float a, fom_float b,
+                            fom_float c)
+{
+    fom_int i;
+    for (i = 0; i < 120; ++i)
+        s3[i] = a * s3[i] + b * u[i] + c * v[i];
+}
+
+/* State update rows 160..199 and 320..399 into hi = new_state + 160:
+   new s2 = d1 s2 + d2 t1, new s4 = s0, new s5 = m1 s2 + m2 t1. */
+static void fom_update_hi(const fom_float *restrict x, const fom_float *restrict t1,
+                          fom_float d1, fom_float d2, fom_float m1, fom_float m2,
+                          fom_float *restrict hi)
+{
+    fom_int i;
+    memcpy(hi + 160, x, 40 * sizeof(fom_float));  /* new s4 = s0: one block copy */
+    for (i = 0; i < 40; ++i) {
+        hi[i] = d1 * x[160 + i] + d2 * t1[i];
+        hi[200 + i] = m1 * x[160 + i] + m2 * t1[i];
+    }
+}
+
+/* First step of an s2 pair: only new s5 = m1 s2 + m2 t1 and new s4 = s0;
+   s2 itself is advanced on the second step. */
+static void fom_update_hi_first(const fom_float *restrict x, const fom_float *restrict t1,
+                                fom_float m1, fom_float m2, fom_float *restrict hi)
+{
+    fom_int i;
+    memcpy(hi + 160, x, 40 * sizeof(fom_float));
+    for (i = 0; i < 40; ++i)
+        hi[200 + i] = m1 * x[160 + i] + m2 * t1[i];
+}
+
+/* Second step of an s2 pair, s2 in place in hi (it holds s2 two steps back),
+   u = t1 of the previous step (s0 of x), v = t1 of this step:
+   new s2 = a s2 + b u + c v, new s5 = e s2 + f u + m2 v, new s4 = s0. */
+static void fom_update_hi_second(const fom_float *restrict x, const fom_float *restrict v,
+                                 fom_float a, fom_float b, fom_float c, fom_float e,
+                                 fom_float f, fom_float m2, fom_float *restrict hi)
+{
+    fom_int i;
+    memcpy(hi + 160, x, 40 * sizeof(fom_float));
+    for (i = 0; i < 40; ++i) {
+        const fom_float s2 = hi[i], u = x[i];
+        hi[i] = a * s2 + b * u + c * v[i];
+        hi[200 + i] = e * s2 + f * u + m2 * v[i];
+    }
+}
+
 fom_int fom_solve(FomWorkspace *work)
 {
     const fom_float *acc = work->acc;
-    fom_float *bt1 = work->acc + BT1;
+    /* solution.x is scratch until the end of the solve */
+    fom_float *const bias0 = work->solution.x;
     fom_float *x = work->state;
     fom_float *xn = work->new_state;
     fom_float res = 0;
-    fom_int it, i, k;
+    fom_float pd1 = 0, pd2 = 0;
+    fom_int it, i, pend = 0;
     const fom_int max_iter = work->settings.max_iter;
 
     for (it = 0; it < max_iter; ++it) {
-        const fom_float taux = fom_sched_taux(it);
-        const fom_float tth = fom_sched_tth(it);
         const fom_float tau = fom_sched_tau(it);
         const fom_float eta = fom_sched_eta(it);
-        const fom_float d1 = fom_sched_d1(it);
-        const fom_float g2 = fom_sched_g2(it);
-        const fom_float m1 = fom_sched_m1(it);
-        const fom_float d2 = fom_sched_d2(it);
-        const fom_float g1 = fom_sched_g1(it);
-        const fom_float m2 = fom_sched_m2(it);
-        fom_float *bt0 = xn; /* xn is free until the state update */
+        const fom_float d1 = fom_sched_d1(it), d2 = fom_sched_d2(it);
+        const fom_float g1 = fom_sched_g1(it), g2 = fom_sched_g2(it);
+        const fom_float m1 = fom_sched_m1(it), m2 = fom_sched_m2(it);
+        /* su is all selections with no constant term:
+           new s0 = g1 t1 + g2 s2, new s1 = g1 t0 + g2 s3, new s2 = d1 s2 + d2 t1,
+           new s3 = d1 s3 + d2 t0, new s4 = s0, new s5 = m1 s2 + m2 t1.
+           With g1 = 1, g2 = 0 (every iteration but the restarts) new s0/s1
+           are t1/t0 themselves, so the steps write straight into them. */
+        const int direct = (g1 == 1.0 && g2 == 0.0);
+        fom_float *t0 = direct ? xn + 40 : work->t0;
+        fom_float *t1 = direct ? xn : work->t1;
         fom_float *tmp;
-
-        for (i = 0; i < FOM_T0_DIM; ++i)
-            bt0[i] = acc[B0A + i] + tau * acc[B0B + i];
-        for (i = 0; i < FOM_T1_DIM; ++i)
-            bt1[i] = acc[B1A + i] + eta * acc[B1B + i];
-
-        fom_step_t0(x, taux, tth, bt0, &FOM_ACT_t0, work->t0);
-        fom_step_t1(x, work->t0, eta, bt1, &FOM_ACT_t1, work->t1);
-
-        memset(xn, 0, sizeof(work->new_state));
-        fom_fused_su__rest(x, work->t0, work->t1, d1, g2, m1, d2, g1, m2, xn);
-        for (k = 0; k < FOM_SELROWS_su_N; ++k)
-            xn[FOM_SELROWS_su_ROW[k]] = x[FOM_SELROWS_su_COL[k]];
+        const fom_float *bt0 = bias0 + ((it & 1) ? BH : 0);
+        const fom_float *bt1 = bt0 + FOM_T0_DIM;
+        /* s2 and s3 are read only by restarts (g2 != 0) and the final state,
+           so on plain iterations both advance two steps at a time */
+        const int pair = direct && it + 1 < max_iter;
+        const int second = pair && pend;
+
+        FOM_PROF_BEGIN(bias);
+        /* biases of iterations it and it+1 in one pass over acc, into the
+           two halves of solution.x (scratch) */
+        if (!(it & 1)) {
+            const fom_float tau2 = fom_sched_tau(it + 1), eta2 = fom_sched_eta(it + 1);
+            for (i = 0; i < FOM_T0_DIM; ++i) {
+                const fom_float a = acc[B0B + i];
+                bias0[i] = tau * a;
+                bias0[BH + i] = tau2 * a;
+            }
+            for (i = 0; i < FOM_T1_DIM; ++i) {
+                const fom_float a = acc[B1B + i];
+                bias0[FOM_T0_DIM + i] = eta * a;
+                bias0[BH + FOM_T0_DIM + i] = eta2 * a;
+            }
+        }
+        FOM_PROF_END(bias);
+
+        FOM_PROF_BEGIN(step_t0);
+        fom_step_t0(x, fom_sched_taux(it), fom_sched_tth(it), bt0, &FOM_ACT_t0, t0);
+        FOM_PROF_END(step_t0);
+        FOM_PROF_BEGIN(update_s3);
+        /* s3 is read only by restarts (g2 != 0) and the final state, so on
+           plain iterations it is advanced two steps at a time: t0(it-1)
+           is still s1 of x when t0(it) is written, and s3 stays in place
+           in the buffer that is x on the first step of the pair. */
+        if (pair) {
+            if (pend) {
+                /* xn holds s3(it-1): s3(it+1) = d1 (pd1 s3(it-1) + pd2 t0(it-1)) + d2 t0(it) */
+                fom_update_s3x2(xn + 200, x + 40, t0, d1 * pd1, d1 * pd2, d2);
+                pend = 0;
+            } else {
+                pd1 = d1;
+                pd2 = d2;
+                pend = 1;
+            }
+        } else {
+            if (pend) {
+                /* materialize s3(it) = pd1 s3(it-1) + pd2 t0(it-1) in x,
+                   and s2(it) = pd1 s2(it-1) + pd2 t1(it-1) likewise */
+                fom_update_s3(xn, x + 40, pd1, pd2, x + 200);
+                for (i = 0; i < 40; ++i)
+                    x[160 + i] = pd1 * xn[160 + i] + pd2 * x[i];
+                pend = 0;
+            }
+            fom_update_s3(x, t0, d1, d2, xn + 200);
+        }
+        FOM_PROF_END(update_s3);
+        FOM_PROF_BEGIN(step_t1);
+        fom_step_t1(x, t0, eta, bt1, &FOM_ACT_t1, t1);
+        FOM_PROF_END(step_t1);
+
+        FOM_PROF_BEGIN(update);
+        if (!direct)
+            fom_update_lo(x, t0, t1, g1, g2, xn);
+        if (second)
+            fom_update_hi_second(x, t1, d1 * pd1, d1 * pd2, d2, m1 * pd1, m1 * pd2, m2, xn + 160);
+        else if (pair)
+            fom_update_hi_first(x, t1, m1, m2, xn + 160);
+        else
+            fom_update_hi(x, t1, d1, d2, m1, m2, xn + 160);
+        FOM_PROF_END(update);
 
         if (it + 1 == max_iter) {
-            res = 0;
-            for (i = 0; i < FOM_STATE_DIM; ++i) {
-                fom_float d = xn[i] - x[i];
-                d = d < 0 ? -d : d;
-                res = d > res ? d : res;
-            }
+            if (direct) {
+                memcpy(work->t0, t0, sizeof(work->t0));
+                memcpy(work->t1, t1, sizeof(work->t1));
+            }
+            /* max |xn - x| with four independent running maxima */
+            fom_float r[4] = {0.0, 0.0, 0.0, 0.0};
+            for (i = 0; i < FOM_STATE_DIM; i += 4) {
+                fom_int j;
+                for (j = 0; j < 4; ++j)
+                    r[j] = fmax(r[j], fabs(xn[i + j] - x[i + j]));
+            }
+            res = fmax(fmax(r[0], r[1]), fmax(r[2], r[3]));
         }
         tmp = x; x = xn; xn = tmp;
     }
--- src/fom_ops.c (translated)
+++ src/fom_ops.c (optimized)
@@ -1,5 +1,16 @@
 #include "fom_ops.h"
 #include "fom_data.h"
+#include <math.h>
+#include <string.h>
+
+/* 2-wide vector values (GNU vector extension), so pairs stay in registers. */
+typedef fom_float fom_v2 __attribute__((vector_size(16)));
+static inline fom_v2 fom_ld2(const fom_float *p) { fom_v2 v; memcpy(&v, p, sizeof v); return v; }
+static inline void fom_st2(fom_float *p, fom_v2 v) { memcpy(p, &v, sizeof v); }
+static inline fom_v2 fom_bc(fom_float s) { fom_v2 v; v[0] = s; v[1] = s; return v; }
+static inline fom_v2 fom_swap(fom_v2 a) { fom_v2 v; v[0] = a[1]; v[1] = a[0]; return v; }
+static inline fom_v2 fom_zip1(fom_v2 a, fom_v2 b) { fom_v2 v; v[0] = a[0]; v[1] = b[0]; return v; }
+static inline fom_v2 fom_zip2(fom_v2 a, fom_v2 b) { fom_v2 v; v[0] = a[1]; v[1] = b[1]; return v; }
 
 void fom_apply_t0_state(const fom_float *x, fom_float *out) {
     fom_int i, p;
@@ -265,45 +276,115 @@
     }
 }
 
-void fom_step_t0(const fom_float *state, fom_float sc_taux, fom_float sc_tth, const fom_float *bias, const FomAct_t0 *act, fom_float *out) {
-    fom_int i, r, p;
-    for (r = 0; r < 120; ++r) {
-        fom_float s, v;
-        i = r;
-        v = bias[i];
-        s = 0.0;
-        for (p = FOM_MAT_t0_state_ROWPTR[i]; p < FOM_MAT_t0_state_ROWPTR[i + 1]; ++p) s += FOM_MAT_t0_state_VALS[p] * state[FOM_MAT_t0_state_COLIDX[p]];
-        v += s;
-        s = 0.0;
-        for (p = FOM_MAT_t0_state__d_taux_ROWPTR[i]; p < FOM_MAT_t0_state__d_taux_ROWPTR[i + 1]; ++p) s += FOM_MAT_t0_state__d_taux_VALS[p] * state[FOM_MAT_t0_state__d_taux_COLIDX[p]];
-        v += sc_taux * s;
-        s = 0.0;
-        for (p = FOM_MAT_t0_state__d_tth_ROWPTR[i]; p < FOM_MAT_t0_state__d_tth_ROWPTR[i + 1]; ++p) s += FOM_MAT_t0_state__d_tth_VALS[p] * state[FOM_MAT_t0_state__d_tth_COLIDX[p]];
-        v += sc_tth * s;
-        if (v < act->lo[i]) v = act->lo[i];
-        out[i] = v;
-    }
-}
-
-void fom_step_t1(const fom_float *state, const fom_float *t_t0, fom_float sc_eta, const fom_float *bias, const FomAct_t1 *act, fom_float *out) {
-    fom_int i, r, p;
-    for (r = 0; r < 40; ++r) {
-        fom_float s, v;
-        i = r;
-        v = bias[i];
-        s = 0.0;
-        for (p = FOM_MAT_t1_state_ROWPTR[i]; p < FOM_MAT_t1_state_ROWPTR[i + 1]; ++p) s += FOM_MAT_t1_state_VALS[p] * state[FOM_MAT_t1_state_COLIDX[p]];
-        v += s;
-        s = 0.0;
-        for (p = FOM_MAT_t1_state__d_eta_ROWPTR[i]; p < FOM_MAT_t1_state__d_eta_ROWPTR[i + 1]; ++p) s += FOM_MAT_t1_state__d_eta_VALS[p] * state[FOM_MAT_t1_state__d_eta_COLIDX[p]];
-        v += sc_eta * s;
-        s = 0.0;
-        for (p = FOM_MAT_t1_t_t0__d_eta_ROWPTR[i]; p < FOM_MAT_t1_t_t0__d_eta_ROWPTR[i + 1]; ++p) s += FOM_MAT_t1_t_t0__d_eta_VALS[p] * t_t0[FOM_MAT_t1_t_t0__d_eta_COLIDX[p]];
-        v += sc_eta * s;
-        if (v < act->lo[i]) v = act->lo[i];
-        if (v > act->hi[i]) v = act->hi[i];
-        out[i] = v;
-    }
+void fom_step_t0(const fom_float *restrict state, fom_float sc_taux, fom_float sc_tth, const fom_float *restrict bias, const FomAct_t0 *act, fom_float *restrict out) {
+    /* same structure as fom_iterate (see the comment there) */
+    static const fom_float GC[10] = {0.05880959999999999, 0.20320800000000006, 0.3483280000000001, 0.4934480000000001, 0.6385680000000002, 0.7836880000000006, 0.9288080000000004, 1.0739280000000009, 1.219048000000001, 1.364168000000001};
+    const fom_float *s0 = state, *s1 = state + 40, *s4 = state + 320;
+    const fom_float *restrict lo = act->lo;
+    const fom_float ntth = -sc_tth;
+    fom_float y[40], dy[20], gy[20];
+    fom_int i, k, m, p;
+
+    /* rows 0..79 = relu(bias + s1 + [I; -I] y), y = taux s0 - tth s4 */
+    for (i = 0; i < 40; ++i) {
+        const fom_float yi = sc_taux * s0[i] + ntth * s4[i];
+        y[i] = yi;
+        out[i] = fmax(bias[i] + s1[i] + yi, lo[i]);
+        out[40 + i] = fmax(bias[40 + i] + s1[40 + i] - yi, lo[40 + i]);
+    }
+    /* rows 80..119 = relu(bias + s1 + [G; -G] y) */
+    for (k = 0; k < 10; ++k) {
+        dy[2 * k] = y[4 * k + 3] - y[4 * k + 1];
+        dy[2 * k + 1] = y[4 * k + 2] - y[4 * k];
+    }
+    for (i = 0; i < 20; ++i) gy[i] = 0.0;
+    for (k = 0; k < 10; ++k)
+        for (m = k; m < 10; ++m)
+            for (p = 0; p < 2; ++p)
+                gy[2 * m + p] += GC[m - k] * dy[2 * k + p];
+    for (i = 0; i < 20; ++i) {
+        out[80 + i] = fmax(bias[80 + i] + s1[80 + i] + gy[i], lo[80 + i]);
+        out[100 + i] = fmax(bias[100 + i] + s1[100 + i] - gy[i], lo[100 + i]);
+    }
+}
+
+void fom_step_t1(const fom_float *restrict state, const fom_float *restrict t_t0, fom_float sc_eta, const fom_float *restrict bias, const FomAct_t1 *act, fom_float *restrict out) {
+    /* same structure as fom_iterate (see the comment there):
+       out = clip(bias + s0 + eta (H s5 - K^T t0)) */
+    /* rows of [MP MM MQ] (MQ appears twice in H, once per pair entry) */
+    static const fom_float HT[10][30] = {
+        {-34.33910113897671, -30.46418866865409, -26.48477507784061, -22.566572450576892, -18.75370310396163, -15.090314726655816, -11.620591012710658, -8.3887623040663, -5.4391162442872485, -2.642223378079356, -33.77938355115386, -28.612000292177953, -23.788355632402713, -19.150638992627513, -14.791814382852591, -10.804845813077538, -7.282697293302508, -4.318332833527525, -2.0047164437525, -0.43885995138300005, -5991.767023566703, -4557.670190227929, -3342.967857489809, -2340.165164174062, -1540.1884370928396, -930.9389470695601, -496.40650726565417, -215.86314980428375, -63.156941584223105, -6.12065847687113},
+        {-30.46418866865409, -28.94196259421897, -25.397772482956483, -21.754254036425472, -18.177200245524226, -14.71081559966262, -11.39933124506025, -8.287015638674966, -5.418185211119178, -2.6605151913951857, -28.612000292177953, -24.83848370510345, -20.586658739627424, -16.67857237335246, -12.956414027077566, -9.513147710802517, -6.441737434527501, -3.835147208252515, -1.7863410419775057, -0.3919000766130001, -4557.670190227929, -3486.448062452827, -2571.790724219666, -1811.335795594479, -1199.63593244056, -729.6970035582555, -391.5323476037979, -171.27688014855113, -50.38128699222665, -4.9021066859246805},
+        {-26.48477507784061, -25.397772482956483, -24.1721637116718, -20.929230256056368, -17.591680381175777, -14.325380776391055, -11.174610782283104, -8.183677566979512, -5.396926798395148, -2.679093104573968, -23.788355632402713, -20.586658739627424, -17.63573643605335, -14.206505754077401, -11.121013671302464, -8.221449608527509, -5.600777575752524, -3.351961582977495, -1.5679656402025004, -0.3449402018429999, -3342.967857489809, -2571.790724219666, -1910.1212719766925, -1354.3465937371132, -903.5341408933623, -553.6887644401568, -299.29124190364604, -131.85288952060475, -39.0300301146234, -3.8152689260322155},
+        {-22.566572450576892, -21.754254036425472, -20.929230256056368, -19.969697875707027, -16.997002477104743, -13.933917417390902, -10.94637549609868, -8.07872319800183, -5.375335885614298, -2.6979615924662936, -19.150638992627513, -16.67857237335246, -14.206505754077401, -11.985213724003472, -9.285613315527472, -6.929751506252515, -4.75981771697751, -2.8687759577025047, -1.3495902384275045, -0.29798032707300015, -2340.165164174062, -1811.335795594479, -1354.3465937371132, -968.5740570312427, -651.3527881496677, -402.71256805680605, -219.63562899050186, -97.59541681240005, -29.111442290551437, -2.8615505987152368},
+        {-18.75370310396163, -18.177200245524226, -17.591680381175777, -16.997002477104743, -16.271418782818603, -13.536331231122208, -10.714570411613188, -7.9721272514471915, -5.35340727218666, -2.717125199913303, -14.791814382852591, -12.956414027077566, -11.121013671302464, -9.285613315527472, -7.7009875489533615, -5.638053403977519, -3.9188578582025064, -2.3855903324274967, -1.1312148366525014, -0.2510204523030004, -1540.1884370928396, -1199.63593244056, -903.5341408933623, -651.3527881496677, -443.05311157060544, -276.5635985860838, -152.5172037915471, -68.50876721584774, -20.633924230083828, -2.0423790871968737},
+        {-15.090314726655816, -14.71081559966262, -14.325380776391055, -13.933917417390902, -13.536331231122208, -13.01092194045405, -10.479139694077642, -7.863864051615149, -5.331135676180528, -2.736588542841405, -10.804845813077538, -9.513147710802517, -8.221449608527509, -6.929751506252515, -5.638053403977519, -4.597129890903335, -3.0778979994274946, -1.9024047071524994, -0.9128394348775006, -0.20406057753299955, -930.9389470695601, -729.6970035582555, -553.6887644401568, -402.71256805680605, -276.5635985860838, -175.53364232214975, -97.88690570062906, -44.597313259803144, -13.60600803770277, -1.3592041002148232},
+        {-11.620591012710658, -11.39933124506025, -11.174610782283104, -10.94637549609868, -10.714570411613188, -10.479139694077642, -10.118422124649763, -7.753907521214939, -5.30851573305019, -2.7563563093741097, -7.282697293302508, -6.441737434527501, -5.600777575752524, -4.75981771697751, -3.9188578582025064, -3.0778979994274946, -2.487712729853323, -1.4192190818775012, -0.6944640331024994, -0.1571007027629998, -496.40650726565417, -391.5323476037979, -299.29124190364604, -219.63562899050186, -152.5172037915471, -97.88690570062906, -56.19471237486229, -25.865495863274106, -8.036359267422224, -0.8134980212117968},
+        {-8.3887623040663, -8.287015638674966, -8.183677566979512, -8.07872319800183, -7.9721272514471915, -7.863864051615149, -7.753907521214939, -7.520626664295005, -5.285541994343773, -2.776433260961258, -4.318332833527525, -3.835147208252515, -3.351961582977495, -2.8687759577025047, -2.3855903324274967, -1.9024047071524994, -1.4192190818775012, -1.1868080458033283, -0.47608863132750034, -0.11014082799300007, -215.86314980428375, -171.27688014855113, -131.85288952060475, -97.59541681240005, -68.50876721584774, -44.597313259803144, -25.865495863274106, -12.817631018903285, -3.933779010054678, -0.4067562629876249},
+        {-5.4391162442872485, -5.418185211119178, -5.396926798395148, -5.375335885614298, -5.35340727218666, -5.331135676180528, -5.30851573305019, -5.285541994343773, -5.140604415601564, -2.7968242335259, -2.0047164437525, -1.7863410419775057, -1.5679656402025004, -1.3495902384275045, -1.1312148366525014, -0.9128394348775006, -0.6944640331024994, -0.47608863132750034, -0.5084878187533253, -0.0631809532230001, -63.156941584223105, -50.38128699222665, -39.0300301146234, -29.111442290551437, -20.633924230083828, -13.60600803770277, -8.036359267422224, -3.933779010054678, -1.8070116269250234, -0.14049762789842157},
+        {-2.642223378079356, -2.6605151913951857, -2.679093104573968, -2.6979615924662936, -2.717125199913303, -2.736588542841405, -2.7563563093741097, -2.776433260961258, -2.7968242335259, -2.489531004892205, -0.43885995138300005, -0.3919000766130001, -0.3449402018429999, -0.29798032707300015, -0.2510204523030004, -0.20406057753299955, -0.1571007027629998, -0.11014082799300007, -0.0631809532230001, -0.24216952360154975, -6.12065847687113, -4.9021066859246805, -3.8152689260322155, -2.8615505987152368, -2.0423790871968737, -1.3592041002148232, -0.8134980212117968, -0.4067562629876249, -0.14049762789842157, -0.4660564339058426}};
+    static const fom_float GC[10] = {0.05880959999999999, 0.20320800000000006, 0.3483280000000001, 0.4934480000000001, 0.6385680000000002, 0.7836880000000006, 0.9288080000000004, 1.0739280000000009, 1.219048000000001, 1.364168000000001};
+    const fom_float *s0 = state, *s5 = state + 360;
+    const fom_float *restrict lo = act->lo, *restrict hi = act->hi;
+    fom_v2 spv[5], smv[5], dq[10], up[5], um[5], aq[10], wq[10];
+    fom_float sp[10], sm[10], pre[40];
+    fom_int i, j, k, l, m;
+
+    /* z = s5 block k: a = (z0+z2, z1+z3), sp = a0+a1, sm = a0-a1,
+       dq = (z0-z2, z1-z3) = (d[2k+1], d[2k]) (swapped pair) */
+    for (j = 0; j < 5; ++j) {
+        const fom_v2 za = fom_ld2(s5 + 8 * j), zb = fom_ld2(s5 + 8 * j + 2);
+        const fom_v2 zc = fom_ld2(s5 + 8 * j + 4), zd = fom_ld2(s5 + 8 * j + 6);
+        const fom_v2 a = za + zb, b = zc + zd;
+        const fom_v2 lo2 = fom_zip1(a, b), hi2 = fom_zip2(a, b);
+        spv[j] = lo2 + hi2;
+        smv[j] = lo2 - hi2;
+        dq[2 * j] = za - zb;
+        dq[2 * j + 1] = zc - zd;
+        sp[2 * j] = spv[j][0]; sp[2 * j + 1] = spv[j][1];
+        sm[2 * j] = smv[j][0]; sm[2 * j + 1] = smv[j][1];
+    }
+    for (j = 0; j < 5; ++j) { up[j] = fom_bc(0.0); um[j] = fom_bc(0.0); }
+    /* -G^T w enters only the q-differences: +sum_{m>=k} c_{m-k} w[2m+p] (swapped pairs),
+       computed first: it is independent of the HT pass */
+    for (m = 0; m < 10; ++m)
+        wq[m] = fom_swap(fom_ld2(t_t0 + 80 + 2 * m) - fom_ld2(t_t0 + 100 + 2 * m));
+    for (k = 0; k < 10; ++k) {
+        fom_v2 s = fom_bc(GC[0]) * wq[k];
+        for (m = k + 1; m < 10; ++m)
+            s += fom_bc(GC[m - k]) * wq[m];
+        aq[k] = s;
+    }
+    /* rows of [MP MM MQ(x2)] (symmetric: column = row), streamed as loads */
+#pragma clang loop unroll(disable)
+    for (l = 0; l < 10; ++l) {
+        const fom_float *h = HT[l];
+        const fom_v2 xp = fom_bc(sp[l]), xm = fom_bc(sm[l]), dl = dq[l];
+        for (j = 0; j < 5; ++j) {
+            up[j] += fom_ld2(h + 2 * j) * xp;
+            um[j] += fom_ld2(h + 10 + 2 * j) * xm;
+        }
+        for (j = 0; j < 5; ++j) {
+            const fom_v2 q = fom_ld2(h + 20 + 2 * j);
+            aq[2 * j] += fom_bc(q[0]) * dl;
+            aq[2 * j + 1] += fom_bc(q[1]) * dl;
+        }
+    }
+    /* hz[4k..4k+1] = (u0, u1) + (v[2k+1], v[2k]), hz[4k+2..4k+3] = (u0, u1) - (...) */
+    for (j = 0; j < 5; ++j) {
+        const fom_v2 u0 = up[j] + um[j], u1 = up[j] - um[j];
+        const fom_v2 ua = fom_zip1(u0, u1), ub = fom_zip2(u0, u1);
+        const fom_v2 h0 = ua + aq[2 * j], h1 = ua - aq[2 * j];
+        const fom_v2 h2 = ub + aq[2 * j + 1], h3 = ub - aq[2 * j + 1];
+        const fom_v2 hz[4] = {h0, h1, h2, h3};
+        for (m = 0; m < 4; ++m) {
+            i = 8 * j + 2 * m;
+            fom_st2(pre + i, fom_ld2(bias + i) + fom_ld2(s0 + i)
+                + fom_bc(sc_eta) * (hz[m] - (fom_ld2(t_t0 + i) - fom_ld2(t_t0 + 40 + i))));
+        }
+    }
+    for (i = 0; i < 40; ++i)
+        out[i] = fmin(fmax(pre[i], lo[i]), hi[i]);
 }
 
 void fom_step_su(const fom_float *state, const fom_float *t_t0, const fom_float *t_t1, fom_float sc_d1, fom_float sc_g2, fom_float sc_m1, fom_float sc_d2, fom_float sc_g1, fom_float sc_m2, const fom_float *bias, fom_float *out) {
@@ -415,8 +496,139 @@
     }
 }
 
-void fom_iterate(const fom_float *state, fom_float sc_taux, fom_float sc_tth, fom_float sc_eta, fom_float sc_d1, fom_float sc_g2, fom_float sc_m1, fom_float sc_d2, fom_float sc_g1, fom_float sc_m2, const fom_float *bias_t0, const fom_float *bias_t1, const fom_float *bias_su, const FomAct_t0 *act_t0, const FomAct_t1 *act_t1, fom_float *t_t0, fom_float *t_t1, fom_float *new_state) {
-    fom_step_t0(state, sc_taux, sc_tth, bias_t0, act_t0, t_t0);
-    fom_step_t1(state, t_t0, sc_eta, bias_t1, act_t1, t_t1);
-    fom_step_su(state, t_t0, t_t1, sc_d1, sc_g2, sc_m1, sc_d2, sc_g1, sc_m2, bias_su, new_state);
-}
+/* One pass over the iteration. Structure exploited (all checked by verify on
+   every unit vector):
+   - t0_state__d_taux = [K 0 0 0 0 0], t0_state__d_tth = [0 0 0 0 -K 0] with
+     K = [I; -I; G; -G] (40, 40, 20, 20 rows), so the two products collapse to
+     K (taux s0 - tth s4), and t1_t_t0__d_eta = -K^T.
+   - G (rows 80..99) is lower block Toeplitz: with j = 4k + 2q + r,
+     (G y)[2m+1-r] = -sum_{k<=m} c_{m-k} (y[4k+r] - y[4k+2+r]); c = GC.
+   - t1_state__d_eta = [0 .. 0 H], H symmetric with
+     H[4k+2q+r][4l+2q'+r'] = a_kl (q=q', r=r'), b_kl (q!=q', r=r'), c_kl (r!=r'),
+     so in sum/difference coordinates it splits into four 10x10 products:
+     MP = (a+b+2c)/4 on the full sums, MM = (a+b-2c)/4 on the r-differences of
+     the q-sums, MQ = (a-b)/2 on each r's q-differences (power-of-2 scalings
+     absorb the inverse transform). HT row l = [MP row l, MM row l, MQ row l
+     with each entry repeated for the two lanes of the pair layout].
+   - every su product is a block shift/copy. */
+#define FOM_N 40
+#define FOM_NB 10
+
+void fom_iterate(const fom_float *restrict state, fom_float sc_taux, fom_float sc_tth, fom_float sc_eta, fom_float sc_d1, fom_float sc_g2, fom_float sc_m1, fom_float sc_d2, fom_float sc_g1, fom_float sc_m2, const fom_float *restrict bias_t0, const fom_float *restrict bias_t1, const fom_float *restrict bias_su, const FomAct_t0 *act_t0, const FomAct_t1 *act_t1, fom_float *restrict t_t0, fom_float *restrict t_t1, fom_float *restrict new_state) {
+    static const fom_float HT[10][40] = {
+        {-34.33910113897671, -30.46418866865409, -26.48477507784061, -22.566572450576892, -18.75370310396163, -15.090314726655816, -11.620591012710658, -8.3887623040663, -5.4391162442872485, -2.642223378079356, -33.77938355115386, -28.612000292177953, -23.788355632402713, -19.150638992627513, -14.791814382852591, -10.804845813077538, -7.282697293302508, -4.318332833527525, -2.0047164437525, -0.43885995138300005, -5991.767023566703, -5991.767023566703, -4557.670190227929, -4557.670190227929, -3342.967857489809, -3342.967857489809, -2340.165164174062, -2340.165164174062, -1540.1884370928396, -1540.1884370928396, -930.9389470695601, -930.9389470695601, -496.40650726565417, -496.40650726565417, -215.86314980428375, -215.86314980428375, -63.156941584223105, -63.156941584223105, -6.12065847687113, -6.12065847687113},
+        {-30.46418866865409, -28.94196259421897, -25.397772482956483, -21.754254036425472, -18.177200245524226, -14.71081559966262, -11.39933124506025, -8.287015638674966, -5.418185211119178, -2.6605151913951857, -28.612000292177953, -24.83848370510345, -20.586658739627424, -16.67857237335246, -12.956414027077566, -9.513147710802517, -6.441737434527501, -3.835147208252515, -1.7863410419775057, -0.3919000766130001, -4557.670190227929, -4557.670190227929, -3486.448062452827, -3486.448062452827, -2571.790724219666, -2571.790724219666, -1811.335795594479, -1811.335795594479, -1199.63593244056, -1199.63593244056, -729.6970035582555, -729.6970035582555, -391.5323476037979, -391.5323476037979, -171.27688014855113, -171.27688014855113, -50.38128699222665, -50.38128699222665, -4.9021066859246805, -4.9021066859246805},
+        {-26.48477507784061, -25.397772482956483, -24.1721637116718, -20.929230256056368, -17.591680381175777, -14.325380776391055, -11.174610782283104, -8.183677566979512, -5.396926798395148, -2.679093104573968, -23.788355632402713, -20.586658739627424, -17.63573643605335, -14.206505754077401, -11.121013671302464, -8.221449608527509, -5.600777575752524, -3.351961582977495, -1.5679656402025004, -0.3449402018429999, -3342.967857489809, -3342.967857489809, -2571.790724219666, -2571.790724219666, -1910.1212719766925, -1910.1212719766925, -1354.3465937371132, -1354.3465937371132, -903.5341408933623, -903.5341408933623, -553.6887644401568, -553.6887644401568, -299.29124190364604, -299.29124190364604, -131.85288952060475, -131.85288952060475, -39.0300301146234, -39.0300301146234, -3.8152689260322155, -3.8152689260322155},
+        {-22.566572450576892, -21.754254036425472, -20.929230256056368, -19.969697875707027, -16.997002477104743, -13.933917417390902, -10.94637549609868, -8.07872319800183, -5.375335885614298, -2.6979615924662936, -19.150638992627513, -16.67857237335246, -14.206505754077401, -11.985213724003472, -9.285613315527472, -6.929751506252515, -4.75981771697751, -2.8687759577025047, -1.3495902384275045, -0.29798032707300015, -2340.165164174062, -2340.165164174062, -1811.335795594479, -1811.335795594479, -1354.3465937371132, -1354.3465937371132, -968.5740570312427, -968.5740570312427, -651.3527881496677, -651.3527881496677, -402.71256805680605, -402.71256805680605, -219.63562899050186, -219.63562899050186, -97.59541681240005, -97.59541681240005, -29.111442290551437, -29.111442290551437, -2.8615505987152368, -2.8615505987152368},
+        {-18.75370310396163, -18.177200245524226, -17.591680381175777, -16.997002477104743, -16.271418782818603, -13.536331231122208, -10.714570411613188, -7.9721272514471915, -5.35340727218666, -2.717125199913303, -14.791814382852591, -12.956414027077566, -11.121013671302464, -9.285613315527472, -7.7009875489533615, -5.638053403977519, -3.9188578582025064, -2.3855903324274967, -1.1312148366525014, -0.2510204523030004, -1540.1884370928396, -1540.1884370928396, -1199.63593244056, -1199.63593244056, -903.5341408933623, -903.5341408933623, -651.3527881496677, -651.3527881496677, -443.05311157060544, -443.05311157060544, -276.5635985860838, -276.5635985860838, -152.5172037915471, -152.5172037915471, -68.50876721584774, -68.50876721584774, -20.633924230083828, -20.633924230083828, -2.0423790871968737, -2.0423790871968737},
+        {-15.090314726655816, -14.71081559966262, -14.325380776391055, -13.933917417390902, -13.536331231122208, -13.01092194045405, -10.479139694077642, -7.863864051615149, -5.331135676180528, -2.736588542841405, -10.804845813077538, -9.513147710802517, -8.221449608527509, -6.929751506252515, -5.638053403977519, -4.597129890903335, -3.0778979994274946, -1.9024047071524994, -0.9128394348775006, -0.20406057753299955, -930.9389470695601, -930.9389470695601, -729.6970035582555, -729.6970035582555, -553.6887644401568, -553.6887644401568, -402.71256805680605, -402.71256805680605, -276.5635985860838, -276.5635985860838, -175.53364232214975, -175.53364232214975, -97.88690570062906, -97.88690570062906, -44.597313259803144, -44.597313259803144, -13.60600803770277, -13.60600803770277, -1.3592041002148232, -1.3592041002148232},
+        {-11.620591012710658, -11.39933124506025, -11.174610782283104, -10.94637549609868, -10.714570411613188, -10.479139694077642, -10.118422124649763, -7.753907521214939, -5.30851573305019, -2.7563563093741097, -7.282697293302508, -6.441737434527501, -5.600777575752524, -4.75981771697751, -3.9188578582025064, -3.0778979994274946, -2.487712729853323, -1.4192190818775012, -0.6944640331024994, -0.1571007027629998, -496.40650726565417, -496.40650726565417, -391.5323476037979, -391.5323476037979, -299.29124190364604, -299.29124190364604, -219.63562899050186, -219.63562899050186, -152.5172037915471, -152.5172037915471, -97.88690570062906, -97.88690570062906, -56.19471237486229, -56.19471237486229, -25.865495863274106, -25.865495863274106, -8.036359267422224, -8.036359267422224, -0.8134980212117968, -0.8134980212117968},
+        {-8.3887623040663, -8.287015638674966, -8.183677566979512, -8.07872319800183, -7.9721272514471915, -7.863864051615149, -7.753907521214939, -7.520626664295005, -5.285541994343773, -2.776433260961258, -4.318332833527525, -3.835147208252515, -3.351961582977495, -2.8687759577025047, -2.3855903324274967, -1.9024047071524994, -1.4192190818775012, -1.1868080458033283, -0.47608863132750034, -0.11014082799300007, -215.86314980428375, -215.86314980428375, -171.27688014855113, -171.27688014855113, -131.85288952060475, -131.85288952060475, -97.59541681240005, -97.59541681240005, -68.50876721584774, -68.50876721584774, -44.597313259803144, -44.597313259803144, -25.865495863274106, -25.865495863274106, -12.817631018903285, -12.817631018903285, -3.933779010054678, -3.933779010054678, -0.4067562629876249, -0.4067562629876249},
+        {-5.4391162442872485, -5.418185211119178, -5.396926798395148, -5.375335885614298, -5.35340727218666, -5.331135676180528, -5.30851573305019, -5.285541994343773, -5.140604415601564, -2.7968242335259, -2.0047164437525, -1.7863410419775057, -1.5679656402025004, -1.3495902384275045, -1.1312148366525014, -0.9128394348775006, -0.6944640331024994, -0.47608863132750034, -0.5084878187533253, -0.0631809532230001, -63.156941584223105, -63.156941584223105, -50.38128699222665, -50.38128699222665, -39.0300301146234, -39.0300301146234, -29.111442290551437, -29.111442290551437, -20.633924230083828, -20.633924230083828, -13.60600803770277, -13.60600803770277, -8.036359267422224, -8.036359267422224, -3.933779010054678, -3.933779010054678, -1.8070116269250234, -1.8070116269250234, -0.14049762789842157, -0.14049762789842157},
+        {-2.642223378079356, -2.6605151913951857, -2.679093104573968, -2.6979615924662936, -2.717125199913303, -2.736588542841405, -2.7563563093741097, -2.776433260961258, -2.7968242335259, -2.489531004892205, -0.43885995138300005, -0.3919000766130001, -0.3449402018429999, -0.29798032707300015, -0.2510204523030004, -0.20406057753299955, -0.1571007027629998, -0.11014082799300007, -0.0631809532230001, -0.24216952360154975, -6.12065847687113, -6.12065847687113, -4.9021066859246805, -4.9021066859246805, -3.8152689260322155, -3.8152689260322155, -2.8615505987152368, -2.8615505987152368, -2.0423790871968737, -2.0423790871968737, -1.3592041002148232, -1.3592041002148232, -0.8134980212117968, -0.8134980212117968, -0.4067562629876249, -0.4067562629876249, -0.14049762789842157, -0.14049762789842157, -0.4660564339058426, -0.4660564339058426}};
+    static const fom_float GC[10] = {0.05880959999999999, 0.20320800000000006, 0.3483280000000001, 0.4934480000000001, 0.6385680000000002, 0.7836880000000006, 0.9288080000000004, 1.0739280000000009, 1.219048000000001, 1.364168000000001};
+    const fom_float *s0 = state, *s1 = state + 40, *s2 = state + 160;
+    const fom_float *s3 = state + 200, *s4 = state + 320, *s5 = state + 360;
+    const fom_float *restrict lo0 = act_t0->lo;
+    const fom_float *restrict lo1 = act_t1->lo, *restrict hi1 = act_t1->hi;
+    /* pair layout: index 2m+p holds row 80+2m+p of G, i.e. r = 1-p */
+    fom_float y[FOM_N], e[FOM_N], dy[2 * FOM_NB], gy[2 * FOM_NB], w[2 * FOM_NB];
+    fom_float sp[FOM_NB], sm[FOM_NB];
+    fom_float d[2 * FOM_NB], acc[4 * FOM_NB], hz[FOM_N];
+    fom_float *const up = acc, *const um = acc + FOM_NB, *const v = acc + 2 * FOM_NB;
+    const fom_float ntth = -sc_tth;
+    fom_int i, k, l, m, p;
+
+    /* Each t0/t1 entry also goes straight into its su rows:
+       new_state[40 + i] = g1 t0[i] + g2 s3[i], new_state[200 + i] = d1 s3[i] + d2 t0[i],
+       new_state[i] = g1 t1[i] + g2 s2[i], new_state[160 + i] = d1 s2[i] + d2 t1[i],
+       new_state[360 + i] = m1 s2[i] + m2 t1[i], new_state[320 + i] = s0[i]. */
+
+    /* t0 rows 0..79 = relu(bias + s1 + K y), y = taux s0 - tth s4 */
+    for (i = 0; i < FOM_N; ++i) {
+        const fom_float yi = sc_taux * s0[i] + ntth * s4[i];
+        fom_float ta = bias_t0[i] + s1[i] + yi;
+        fom_float tb = bias_t0[40 + i] + s1[40 + i] - yi;
+        ta = fmax(ta, lo0[i]);
+        tb = fmax(tb, lo0[40 + i]);
+        y[i] = yi;
+        e[i] = ta - tb;
+        t_t0[i] = ta;
+        t_t0[40 + i] = tb;
+        new_state[40 + i] = bias_su[40 + i] + sc_g1 * ta + sc_g2 * s3[i];
+        new_state[80 + i] = bias_su[80 + i] + sc_g1 * tb + sc_g2 * s3[40 + i];
+        new_state[200 + i] = bias_su[200 + i] + sc_d1 * s3[i] + sc_d2 * ta;
+        new_state[240 + i] = bias_su[240 + i] + sc_d1 * s3[40 + i] + sc_d2 * tb;
+        new_state[320 + i] = bias_su[320 + i] + s0[i];
+    }
+
+    /* G y: (G y)[2m+p] = sum_{k<=m} c_{m-k} dy[2k+p], dy[2k+p] = y[4k+3-p] - y[4k+1-p] */
+    for (k = 0; k < FOM_NB; ++k) {
+        dy[2 * k] = y[4 * k + 3] - y[4 * k + 1];
+        dy[2 * k + 1] = y[4 * k + 2] - y[4 * k];
+    }
+    for (i = 0; i < 2 * FOM_NB; ++i) gy[i] = 0.0;
+    for (k = 0; k < FOM_NB; ++k)
+        for (m = k; m < FOM_NB; ++m)
+            for (p = 0; p < 2; ++p)
+                gy[2 * m + p] += GC[m - k] * dy[2 * k + p];
+    /* t0 rows 80..119 */
+    for (i = 0; i < 2 * FOM_NB; ++i) {
+        fom_float ta = bias_t0[80 + i] + s1[80 + i] + gy[i];
+        fom_float tb = bias_t0[100 + i] + s1[100 + i] - gy[i];
+        ta = fmax(ta, lo0[80 + i]);
+        tb = fmax(tb, lo0[100 + i]);
+        t_t0[80 + i] = ta;
+        t_t0[100 + i] = tb;
+        w[i] = ta - tb;
+    }
+    for (i = 80; i < 120; ++i) {
+        const fom_float t = t_t0[i];
+        new_state[40 + i] = bias_su[40 + i] + sc_g1 * t + sc_g2 * s3[i];
+        new_state[200 + i] = bias_su[200 + i] + sc_d1 * s3[i] + sc_d2 * t;
+    }
+
+    /* H s5 in sum/difference coordinates (d in the same pair layout) */
+    for (k = 0; k < FOM_NB; ++k) {
+        const fom_float *z = s5 + 4 * k;
+        const fom_float a0 = z[0] + z[2], a1 = z[1] + z[3];
+        sp[k] = a0 + a1;
+        sm[k] = a0 - a1;
+        d[2 * k] = z[1] - z[3];
+        d[2 * k + 1] = z[0] - z[2];
+    }
+    /* one pass over the rows l of [MP MM MQ(x2)] (all symmetric: column = row) */
+    for (k = 0; k < 4 * FOM_NB; ++k) acc[k] = 0.0;
+    for (l = 0; l < FOM_NB; ++l) {
+        const fom_float *h = HT[l];
+        const fom_float xp = sp[l], xm = sm[l];
+        for (k = 0; k < FOM_NB; ++k) {
+            acc[k] += h[k] * xp;
+            acc[FOM_NB + k] += h[FOM_NB + k] * xm;
+        }
+        for (k = 0; k < FOM_NB; ++k)
+            for (p = 0; p < 2; ++p)
+                acc[2 * FOM_NB + 2 * k + p] += h[2 * FOM_NB + 2 * k + p] * d[2 * l + p];
+    }
+    /* -G^T w enters only the q-differences: +sum_{m>=k} c_{m-k} w[2m+p] */
+    for (m = 0; m < FOM_NB; ++m)
+        for (k = 0; k <= m; ++k)
+            for (p = 0; p < 2; ++p)
+                v[2 * k + p] += GC[m - k] * w[2 * m + p];
+    for (k = 0; k < FOM_NB; ++k) {
+        const fom_float u0 = up[k] + um[k], u1 = up[k] - um[k];
+        hz[4 * k] = u0 + v[2 * k + 1];
+        hz[4 * k + 1] = u1 + v[2 * k];
+        hz[4 * k + 2] = u0 - v[2 * k + 1];
+        hz[4 * k + 3] = u1 - v[2 * k];
+    }
+    /* t1 = clip(bias + s0 + eta (H s5 - K^T t0)), K^T t0 = e + G^T w (-G^T w is in v) */
+    for (i = 0; i < FOM_N; ++i) {
+        fom_float t = bias_t1[i] + s0[i] + sc_eta * (hz[i] - e[i]);
+        t = fmin(fmax(t, lo1[i]), hi1[i]);
+        t_t1[i] = t;
+        new_state[i] = bias_su[i] + sc_g1 * t + sc_g2 * s2[i];
+        new_state[160 + i] = bias_su[160 + i] + sc_d1 * s2[i] + sc_d2 * t;
+        new_state[360 + i] = bias_su[360 + i] + sc_m1 * s2[i] + sc_m2 * t;
+    }
+}
```

## Agent notes

```
### Round 1

**Whole-solve mean_us:** initial 1095.6 µs, final ~69–71 µs (latest `make bench`: 68.7 µs). That is a **~15.7× speedup**. `make verify` gives VERIFY OK (worst sample error 6.8e-08, tol 1e-4; iterate basis-probe max_err 1.1e-11).

**Final profile (`make profile`, 512 iterations per solve, timer overhead included):**
| region | total_us | calls | pct |
|---|---|---|---|
| iterate (`fom_iterate`, the whole iteration) | 313883 | 2560000 | 79.1% |
| bias (per-iteration `A + tau*B`, `A + eta*B` parameter bias) | 82946 | 2560000 | 20.9% |

The bias share is inflated by the ~30 ns per-call timer. Removing that pass entirely saves only ~7 µs per solve (~10%).

**Optimizations applied:**
1. **One fused `fom_iterate` pass** (1095 → 189 µs). It replaces the step routines plus the memset and copy passes. Every su product is a 0/1 block shift, so each t0/t1 entry goes straight into its su rows (s0,s1 = g1·t + g2·s23; s2,s3 = d1·s23 + d2·t; s4 = s0; s5 = m1·s2 + m2·t1).
2. **Shared K across three products.** `t0_state__d_taux = [K 0…]`, `t0_state__d_tth = [0… −K 0]` and `t1_t_t0__d_eta = −Kᵀ`, with K = [I; −I; G; −G]. So taux·K s0 + tth·(−K) s4 becomes one product K(taux·s0 − tth·s4). Kᵀt0 becomes (t0a − t0b) + Gᵀ(t0c − t0d). The work drops from 3×520 indexed nnz to two products with the 20×40 G.
3. **G is lower block-Toeplitz** (10 distinct coefficients, ± pairs on columns 4k+r and 4k+2+r). G y and Gᵀw become two length-10 triangular convolutions of row differences, in an interleaved pair layout so both parities run as 2-wide ops.
4. **H (dense, symmetric 40×40 `t1_state__d_eta` block) has a Z2×Z2 symmetry.** Its entries depend only on whether the index pairs (q, r) match. A sum/difference transform splits H into four 10×10 products (MP=(a+b+2c)/4, MM=(a+b−2c)/4, MQ=(a−b)/2 used twice). That is 400 MACs instead of 1600, and the power-of-2 scalings absorb the inverse transform. The tables are `static const` literals inside `fom_iterate`, which is unit-vector-probed. Steps 2–4 together: 189 → 87 µs.
5. **Vectorizable layout:** flat pair-layout loops for the G rows and the H output, separate su-row passes, `restrict` on `fom_iterate`'s arguments, and a single interleaved table HT for the three 10×10 matrices. 87 → ~70 µs.
6. Parameter terms stay hoisted to `fom_update_params`. The su block has no bias, so a static zero vector is passed. `solution.x` is used as bias scratch until the end of the solve. No allocation and no division anywhere.

**Blocked by rules:**
- Pass the parameter bias as two vectors (A, B) plus the scalar tau/eta into `fom_iterate`, removing the per-iteration bias pass | headers / `fom_iterate` signature are fixed | ~10% (measured by removing the pass)
- Use the near-arithmetic-progression of the Toeplitz coefficients (c_j ≈ c_1 + (j−1)δ for j ≥ 1) for an O(n) running-sum convolution | literal-exactness rule (δ is not a matrix entry, and the recurrence is not bit-exact to the stored coefficients) | ~5%
- Fuse two iterations per `fom_iterate` call to overlap their dependency chains | `fom_iterate` signature (one iteration) is fixed | unknown
- Build flags such as `-mcpu=native` or ffast-math style flags to get better code generation for the scalarized table products | Makefile / build flags are fixed | unknown

### Round 3

**Result.** This round took the solver from about 63.5 us to 51.4 us per solve (1.24x), and `make verify` still prints `VERIFY OK`.
- The task statement gave 65.338 us as the starting point. I measured 63.5 us for that same code in an alternating A/B run against the final version, using identical Makefile flags.
- When the final binary is run directly, it reads 51.3 to 52.2 us.
- `make bench` reads 55.5 to 58 us, because it runs right after a compile and is about 10% slow then. The prebuilt binaries give 51.4 us.
- Against the translated solver (1090.903 us) that is about 21.2x overall, up from 16.70x.

**Final profile (`make profile`).** The profile build was broken at the start of the round: it defined the `update` region twice and failed to compile. I fixed that by renaming one of them to `update_s3`.

| Region | Time (us) | Share |
|---|---|---|
| step_t1 | 104636 | 27.7% |
| bias | 85058 | 22.6% |
| step_t0 | 69972 | 18.6% |
| update | 62133 | 16.5% |
| update_s3 | 55345 | 14.7% |

The profiled mean is 129 us against 51 us unprofiled, because each region costs only 20 to 40 ns. Timer overhead therefore inflates the small regions. Removing one region at a time from the real solver gives a more reliable split: step_t0 about 13.5 us, step_t1 about 13 us, state updates about 14 us, bias about 5.5 us, loop and schedule overhead about 5 us.

**Optimizations applied**
1. **2-wide vector rewrite of `fom_step_t1`** (57 to 35 ns per call when measured alone).
   - In the old code, the fully unrolled `HT` product turned the `static const` coefficients into about 1100 instructions that rebuild constants (`mov`/`movk`/`dup`), plus spills.
   - The rewrite uses GNU `vector_size(16)` pairs for the sum and difference coordinates of `s5`. It streams `HT` row by row as vector loads, in a loop kept rolled on purpose.
   - It also folds the pair swaps into the layout, so all 2x2 transposes are `zip`/`ext` instructions and nothing round-trips through `ld4`/`ld2` on stack arrays.
2. **Deduplicated `MQ` in `HT`.** The matrix is stored twice, once per pair entry (`h[20+2k] == h[21+2k]`, checked exactly). I store one copy (a 10x30 table instead of 10x40) and apply it with lane-broadcast FMAs. That is 50 fewer 16-byte loads per iteration, about 1 us.
3. **Skipped the zero parameter matrices.** `t0_p_p1` and `t1_p_p0` are zero matrices, so the per-iteration bias is just `tau*b0` and `eta*b1`. That is 160 fewer loads per iteration, about 1 us.
4. **Updated `s3` every two iterations, in place.**
   - `s3` (the running average of `t0`) is read only at restarts (g2 ≠ 0, every 64 iterations) and in the final state.
   - With the two state buffers, `t0(it-1)` is still `s1` of `x` when `t0(it)` is written. So plain iterations advance `s3` two steps in one pass: `s3 <- d1*pd1 s3 + d1*pd2 t0(it-1) + d2 t0(it)`, with no division.
   - On restart iterations and the last iteration, `s3` is computed in full first, so the restart rows and the residual see exact values.
   - This saves about a third of the `s3` loads and stores, about 1.8 us.

**Tried and dropped** (all measured with an alternating A/B inside the real solver):
- vectorized `fom_step_t0` (4 us slower, spills once inlined by LTO)
- `noinline` on the steps
- symmetric upper-block storage for MP and MM (spills; 2.5 to 5 us slower)
- splitting the Toeplitz chains, and reordering the `HT` pass against the Toeplitz pass
- fused vector clip in t1
- computing the biases for two iterations per pass (neutral)
- unrolling the update and row loops (neutral)
- `fom_iterate` as the iteration body (about 3 us slower)

The last few rounds each gained under 2%, so I stopped.

Blocked by rules:
- Precompute the bias vectors for the 64 distinct `tau`/`eta` values of a restart epoch once in `fom_update_params`, so the loop does no bias pass | `FomWorkspace` in the fixed header `include/fom_workspace.h` has no room for an 80 KB table, and a static table in the solver would be hidden, non-reentrant state | about 5 us (about 10%)
- One fused `fom_iterate` pass that keeps `t0`, `t1` and `s0` in registers and skips the reloads | the fixed `fom_iterate` signature in `include/fom_ops.h` makes it add a `bias_su` vector (400 loads, all zeros here) every iteration | unknown; it measured about 3 us slower with the forced `bias_su`
- Drop the `s4 = s0` copy by aliasing the state buffers | the fixed step signatures read `s0`, `s1`, `s4` and `s5` at fixed offsets from a single state pointer | about 1 us
- Force inlining and per-call constant handling through build flags (e.g. `-mcpu`, inlining controls) instead of source pragmas | Makefile and build flags are fixed | unknown

### Round 4

**Whole-solve `mean_us`:** initial (translated) 1090.903 → start of this round 50.746 → final **49.0** (steady `make bench`: 49.00 and 49.07; the first run right after compiling reads high). This round is about 4.5% faster, confirmed by two interleaved A/B runs at 0.956 and 0.954. The overall speedup is **22.3x**.

**Final profile (`make profile`), hottest first:**
| region | total_us | % |
|---|---|---|
| step_t1 | 102931 | 27.95 |
| bias | 81957 | 22.26 |
| step_t0 | 69078 | 18.76 |
| update | 60133 | 16.33 |
| update_s3 | 54134 | 14.70 |

At about 100 ns per iteration, the timer overhead dominates these per-region numbers. Ablation is more informative. I calibrated the clock at about 4.66 GHz, so an iteration is about 450 cycles. Breaking every dependency between iterations still left about 400 cycles, so the loop is limited by throughput, not latency. Stores are the scarce resource:
- Removing 20 loads and nothing else gained nothing.
- Removing 20 loads plus 20 stores gained about 5%.

**Optimizations applied (all in `src/fom_solver.c`):**
1. **s4 copy as one block copy (about 2%).** The su rows 320..359 (new s4 = s0) are a selection row. I moved this copy out of the interleaved s2/s5 loop into a single `memcpy`. That lets the s2/s5 loop vectorize 4-wide with paired loads and stores, and makes the copy a wide block copy.
2. **s2/s5 advanced two iterations at a time (about 1.5%).** s2 is read only by restarts (g2 ≠ 0) and the final state. Neither step routine reads it. This is the same structure the s3 pairing already used. Every iteration still writes s5 (step_t1 needs it).
   - On the first iteration of a pair, only s5 = m1·s2 + m2·t1 is written.
   - On the second, s2 is updated in place from s2 two steps back, the previous t1 (still in s0 of x) and the new t1. s5 is formed from the same inputs.
   - Before restarts and the last iteration, s2 is written out in full, sharing the existing s3 flag and its d1/d2 values.
   - Net effect: about 10 fewer stores per iteration.
3. **Residual as four independent `fmax` accumulators.** This replaces a serial 400-long compare-and-select chain at the end of the solve. It is exact, since max does no rounding. The gain is at noise level (expected about 0.4%), and I kept it because it is strictly less serial work.
4. The `FOM_PROF_BEGIN/END` markers are kept around bias, step_t0, update_s3, step_t1 and update.

**Tried and dropped (all verified correct, none faster):**
- **t0 Toeplitz conv with even/odd split accumulators:** about 0.7% slower. It shortens dependency chains, but chains are not the limit.
- **y loop rewritten with 2-wide vectors and no y[] buffer:** 5% slower.
- **y loop as a rolled per-block loop:** 5% slower.
- **Full unroll of the y loop and hi loop:** no change.
- **HT product split into two fully unrolled passes:** 6% slower. Clang turns the literal table into immediates.
- **HT loop unrolled ×2:** noise.
- **Two iterations' biases in one pass over acc:** no change.
- **Inline (`__builtin_memcpy_inline`) s4 copy:** no change.
- **Calling the existing one-pass `fom_iterate`:** 35% slower, because it must add bias_su (all zeros) to all 400 rows.

**Structure checked but not exploitable:**
- **The bias terms:** `t0_p_p1__d_tau = -I` and `t1_p_p0__d_eta = -I`, so the biases are -tau·p1 and -eta·p0. They genuinely change every iteration.
- **The blocks of H:** in sum/difference coordinates, MP and MM are rank-2 semiseparable and MQ is rank-4. At n = 10 a generator-based matvec saves about 1 FP op per element, and it would need coefficients that are not matrix entries.

Blocked by rules:
- Precompute the 64 distinct per-tau bias vectors once in `fom_update_params` into a static table, removing about 80 stores per iteration | workspace layout (`include/fom_workspace.h`) is fixed and the table (about 80 KB) does not fit its scratch. A static table outside the workspace is not reentrant, and the work would only move out of the timed region | about 4% of solve time, roughly zero net once update_params is counted
- Store t1 once into a layout where it is both s0 of the next state and s4 of the one after, removing the s4 copy (about 5%) | the `fom_step_*` signatures fix a contiguous 400-entry state with s4 at offset 320, so the two-buffer ping-pong cannot alias s4 to the previous s0 | about 5%
- Fold the bias add, the s2/s5 update and the s4 copy into the step_t1 output pass, so t1 never makes a round trip through memory | step signatures are fixed (step_t1 writes only t1), and the only fused alternative, `fom_iterate`, must also add bias_su across all 400 rows | unknown, probably 3–5%

### Round 5

**Whole solve (`make bench` mean_us):** 1090.903 µs for the translated solver, 49.395 µs at the start of this round, about 48.6 µs at the end (paired A/B runs). That is about 22.4x overall. This round gained about 0.7%, which is near the noise floor.

**Final profile (`make profile`, 512 iterations × 5000 solves).** The timer calls themselves cost a lot at about 95 ns per iteration, so the split is only rough:

| Region | Time | Share |
|---|---|---|
| step_t1 | 106425 µs | 27.6% |
| bias | 88717 µs | 23.0% |
| step_t0 | 74306 µs | 19.3% |
| update | 61392 µs | 15.9% |
| update_s3 | 55148 µs | 14.3% |

Knock-out tests (dropping a pass, which gives wrong results but shows its cost) put the real cost at about 6 µs for bias and about 6 µs for update_s3.

**Optimization applied this round:**
- **Paired bias pass.** The parameter terms enter only as scalar multiples of fixed vectors: `bias_t0 = tau·a0` and `bias_t1 = eta·a1`, with `a0` and `a1` computed once per parameter update. So each even iteration now writes the biases for iterations it and it+1 in one pass over `a0` and `a1`. Each `acc` load and the loop overhead now serve two iterations. The two bias sets sit in the two halves of `solution.x`, which is free scratch during the solve.

**Tried and reverted (all verified correct, all slower):**
- step_t1: moving the `Gᵀ t0` Toeplitz part after the HT pass to shorten the t0→t1 dependency chain. With two partial sums it took 52.6 µs; with one accumulator, 50.2 µs.
- step_t1: doing the final clip with vector min/max instead of a separate scalar pass. No gain.
- step_t0: computing the G-convolution with 2-wide vectors. 53.5 µs.
- Fusing the next pair's t0 biases into the paired s3 update pass. 49.7 µs against 48.2 µs.
- Moving the s3 update after step_t1 so step_t1 starts earlier. About 1% slower.

**Structure exploited in earlier rounds, still in place:**
- The su block is all selections, so its products are plain copies.
- t0 and t1 are written straight into the new state on non-restart iterations.
- s2 and s3 (running averages that only restarts and the final state read) advance two steps at a time.
- The t0 product is `[I; −I; G; −G]` with G a lower-triangular Toeplitz block.
- The t1 product is a symmetric dense block plus the t0 term.
- The bias is hoisted into `fom_update_params`.

Blocked by rules:
- Precompute one bias vector per distinct tau/eta schedule value (64 of each) in `fom_update_params`, removing the per-iteration bias pass | the workspace struct is in a fixed header with no room for a table (about 80 KB), and a static table in `fom_solver.c` would be large and not safe with more than one workspace | about 5 µs (≈10%), going by the knock-out test
- Fold the scalar tau/eta bias scaling into `fom_step_t0`/`fom_step_t1` so no bias vector is written at all | the fixed step signatures take a bias vector, not a scale factor plus a base vector | up to about 6 µs (≈12%)
- Advance s3 four steps at a time instead of two | t0 is written into the state buffer that the next step routine reads at a fixed offset, so keeping older t0 values needs a third state buffer, which the fixed workspace doesn't have | about 3 µs
- Store s3 as an unnormalized running sum and rescale it only at restarts | the rescale needs division by schedule-dependent factors (division-free target) | about 3 µs
```
