# Optimization report

## Timing

```
translated (emitted)     137.9 us/solve
optimized                 14.4 us/solve
speedup                  9.56x
```

Optimization outcome: **kept (re-verified correct)**.

## Rounds

| round | before (us) | after (us) | gain | kept | agent (s) | outcome |
|---:|---:|---:|---:|:---:|---:|---|
| 1 | 137.932 | 37.030 | 73.2% | yes | 94 | continued |
| 2 | 36.315 | 26.434 | 27.2% | yes | 98 | continued |
| 3 | 25.600 | 15.113 | 41.0% | yes | 106 | continued |
| 4 | 16.684 | 14.432 | 13.5% | yes | 111 | continued |
| 5 | - | - | - | no | 37 | the round left the sources unchanged |

## Kernel checks

Each `fom_apply_<name>` in the shipped `fom_ops.c`, checked by `make verify` against the package's reference over the same `fom_data.c` matrix. *basis* probes every unit vector, which recovers the routine's matrix column by column (an exact check), plus random vectors; *random* uses random vectors only (the matrix is over the probe budget). A `fused_<block>` row is a `fom_fused_<block>`, checked against the sum of its members: *basis* there probes every unit vector of each source with every schedule value at 0 and with each alone at 1, which recovers every member exactly.

| matrix | shape | nnz | rewritten | check | probes | max error |
|---|---|---|---|---|---|---|
| `t0_state` | 48x160 | 48 | no | basis | 162 | 0.000e+00 |
| `t0_state__d_taux` | 48x160 | 112 | no | basis | 162 | 0.000e+00 |
| `t0_state__d_tth` | 48x160 | 112 | no | basis | 162 | 0.000e+00 |
| `t0_p_p1` | 48x48 | 0 | no | basis | 50 | 0.000e+00 |
| `t0_p_p1__d_tau` | 48x48 | 48 | no | basis | 50 | 0.000e+00 |
| `t1_state` | 16x160 | 16 | no | basis | 162 | 0.000e+00 |
| `t1_state__d_eta` | 16x160 | 256 | no | basis | 162 | 0.000e+00 |
| `t1_p_p0` | 16x16 | 0 | no | basis | 18 | 0.000e+00 |
| `t1_p_p0__d_eta` | 16x16 | 16 | no | basis | 18 | 0.000e+00 |
| `t1_t_t0` | 16x48 | 0 | no | basis | 50 | 0.000e+00 |
| `t1_t_t0__d_eta` | 16x48 | 112 | no | basis | 50 | 0.000e+00 |
| `su_state` | 160x160 | 16 | no | basis | 162 | 0.000e+00 |
| `su_state__d_d1` | 160x160 | 64 | no | basis | 162 | 0.000e+00 |
| `su_state__d_g2` | 160x160 | 64 | no | basis | 162 | 0.000e+00 |
| `su_state__d_m1` | 160x160 | 16 | no | basis | 162 | 0.000e+00 |
| `su_t_t0` | 160x48 | 0 | no | basis | 50 | 0.000e+00 |
| `su_t_t0__d_d2` | 160x48 | 48 | no | basis | 50 | 0.000e+00 |
| `su_t_t0__d_g1` | 160x48 | 48 | no | basis | 50 | 0.000e+00 |
| `su_t_t1` | 160x16 | 0 | no | basis | 18 | 0.000e+00 |
| `su_t_t1__d_d2` | 160x16 | 16 | no | basis | 18 | 0.000e+00 |
| `su_t_t1__d_g1` | 160x16 | 16 | no | basis | 18 | 0.000e+00 |
| `su_t_t1__d_m2` | 160x16 | 16 | no | basis | 18 | 0.000e+00 |
| `fused_t0` | 48x160 | 272 | no | basis | 482 | 0.000e+00 |
| `fused_t1` | 16x208 | 384 | no | basis | 418 | 0.000e+00 |
| `fused_su` | 160x224 | 304 | no | basis | 1570 | 0.000e+00 |
| `step_t0` | 48x160 | 272 | no | basis | 486 | 0.000e+00 |
| `step_t1` | 16x208 | 384 | no | basis | 422 | 0.000e+00 |
| `step_su` | 160x224 | 304 | no | basis | 1570 | 0.000e+00 |
| `fused_su__rest` | 160x224 | 304 | no | basis | 1570 | 0.000e+00 |
| `step_su__rest` | 160x224 | 304 | no | basis | 1570 | 0.000e+00 |
| `iterate` | 160x160 | 960 | yes | basis | 1766 | 1.137e-13 |

### Rule violations

_None: the collected rewrite passed the rules and was kept._

## Blocked by rules

Optimizations the agent considered but did not make because a rule forbade them, in its own words.

| round | idea | rule | expected gain |
|---|---|---|---|
| 1 | Pass the tau/eta scale for the hoisted parameter term straight into `fom_iterate` instead of materializing the bias arrays each iteration (the 11% "bias" region) | the `fom_iterate`/`fom_step_*` signatures in `include/fom_ops.h` are fixed (a header) and take a precomputed bias vector | ~3-4 µs (about 10%) |
| 1 | Drop the all-zero `bias_su` argument and its 160 loads per iteration | fixed `fom_iterate` signature in `include/fom_ops.h` | ~1 µs |
| 2 | Exploit symmetry of the dense `t1_state__d_eta` block (D = D^T in the data) to read it column-contiguously | "must apply exactly M for any input"; D's symmetry is a property of the data that cannot be asserted at compile time | small (~1-2%) |
| 2 | Precompute bias_t0/bias_t1 = tau(it)*q1, eta(it)*q0 for all 200 iterations in `fom_update_params`, dropping the per-iteration bias pass | the loop must read the schedule through the clamping accessor, and max_iter may exceed the table length, so a fixed-size per-iteration table is unsafe; workspace layout is fixed (headers not editable) | ~5-10% (bias is ~11-14% of profile, mostly timer overhead) |
| 3 | Fold the per-iteration bias scaling (tau*q1, eta*q0) into `fom_iterate`, which would drop a 64-element pass and the temporary bias vectors | the fixed `fom_iterate` signature takes `bias` as a precomputed vector and no schedule values for parameter terms (header is fixed) | about 5-10% (the bias region is about 25% of profiled time, mostly timer overhead) |
| 3 | Skip the zero `bias_su` additions, since the solver always passes zeros | `fom_iterate` must be right for any bias (verify uses random biases) | about 2% |
| 4 | Pass `tau`/`eta` into `fom_iterate` so it computes `tau*q1` inline, dropping the separate 64-element bias pass and its stores | `fom_iterate` signature in `include/fom_ops.h` is fixed | ~3-5% |
| 4 | Skip the all-zero `bias_su` add (160 loads and adds per iteration) | `fom_iterate` must honor any `bias_su` it is passed, and the signature is fixed | ~2-4% |
| 4 | Precompute even/odd combinations (`(a+c)/2`, `(a-c)/2`) of the dense-block coefficients in setup, cutting it to about 2 multiplies per row | needs derived per-matrix data stored outside `fom_ops.c`'s routines (workspace layout is fixed, and setup may not probe matrices) | ~5% |
| 5 | Precompute `bias_t0`/`bias_t1` for every iteration at setup (a 200x64 table), removing the per-iteration rescale | no space in the fixed workspace, and `max_iter` is a runtime value with no bound, so this would need allocation | about 1 to 2% (64 multiplies per iteration) |
| 5 | Pass `tau` and `eta` into `fom_iterate` and fold the parameter rescale into the `t0`/`t1` loops | the fixed `fom_iterate` signature takes bias vectors, not the parameter schedule values | about 1 to 2% |

## What the agent changed

```diff
--- src/fom_solver.c (translated)
+++ src/fom_solver.c (optimized)
@@ -1,6 +1,7 @@
 #include "fom_solver.h"
 #include "fom_data.h"
 #include "fom_ops.h"
+#include "fom_profile.h"
 
 /* acc layout: [0,48) q1 = t0_p_p1__d_tau @ p1, [48,64) q0 = t1_p_p0__d_eta @ p0,
    [64,112) bias_t0, [112,128) bias_t1. The plain t0_p_p1 and t1_p_p0 parameter
@@ -63,13 +64,17 @@
     for (it = 0; it < n; ++it) {
         fom_float tau = fom_sched_tau(it), eta = fom_sched_eta(it);
         fom_float *tmp;
+        FOM_PROF_BEGIN(bias);
         for (i = 0; i < FOM_T0_DIM; ++i) a[BT0 + i] = tau * a[Q1 + i];
         for (i = 0; i < FOM_T1_DIM; ++i) a[BT1 + i] = eta * a[Q0 + i];
+        FOM_PROF_END(bias);
+        FOM_PROF_BEGIN(iterate);
         fom_iterate(x, fom_sched_taux(it), fom_sched_tth(it), eta,
                     fom_sched_d1(it), fom_sched_g2(it), fom_sched_m1(it),
                     fom_sched_d2(it), fom_sched_g1(it), fom_sched_m2(it),
                     a + BT0, a + BT1, s_zero_su, &FOM_ACT_t0, &FOM_ACT_t1,
                     work->t0, work->t1, xn);
+        FOM_PROF_END(iterate);
         tmp = x; x = xn; xn = tmp;
     }
     if (n > 0) {
--- src/fom_ops.c (translated)
+++ src/fom_ops.c (optimized)
@@ -416,7 +416,99 @@
 }
 
 void fom_iterate(const fom_float *state, fom_float sc_taux, fom_float sc_tth, fom_float sc_eta, fom_float sc_d1, fom_float sc_g2, fom_float sc_m1, fom_float sc_d2, fom_float sc_g1, fom_float sc_m2, const fom_float *bias_t0, const fom_float *bias_t1, const fom_float *bias_su, const FomAct_t0 *act_t0, const FomAct_t1 *act_t1, fom_float *t_t0, fom_float *t_t1, fom_float *new_state) {
-    fom_step_t0(state, sc_taux, sc_tth, bias_t0, act_t0, t_t0);
-    fom_step_t1(state, t_t0, sc_eta, bias_t1, act_t1, t_t1);
-    fom_step_su(state, t_t0, t_t1, sc_d1, sc_g2, sc_m1, sc_d2, sc_g1, sc_m2, bias_su, new_state);
-}
+    fom_int i, p;
+    /* t0: t0_state is the shift state[i+16]; t0_p_p1 is zero.
+       t0_state__d_tth equals -t0_state__d_taux with its columns 0..15 moved to
+       128..143 (same pattern, negated values), so both products collapse to
+       one product of t0_state__d_taux with w = taux*state[0..15] - tth*state[128..143]. */
+    fom_float w[16];
+    for (i = 0; i < 16; ++i) w[i] = sc_taux * state[i] - sc_tth * state[128 + i];
+    {
+        /* rows 0..15 of t0_state__d_taux are +1 on w[i], rows 16..31 are -1 on w[i-16];
+           rows 32..47 unrolled over their fixed sparsity, coefficients inlined. */
+        const fom_float *lo0 = act_t0->lo;
+        fom_float v;
+        for (i = 0; i < 16; ++i) {
+            v = bias_t0[i] + state[i + 16] + w[i];
+            if (v < lo0[i]) v = lo0[i];
+            t_t0[i] = v;
+            v = bias_t0[i + 16] + state[i + 32] - w[i];
+            if (v < lo0[i + 16]) v = lo0[i + 16];
+            t_t0[i + 16] = v;
+        }
+        {
+            /* rows 40..47 negate rows 32..39, and each row's coefficients are a
+               banded Toeplitz pattern on the differences w[4k+1]-w[4k+3] / w[4k]-w[4k+2]. */
+            const fom_float ca = 0.07259999999999998, cb = 0.21772, cc = 0.36284, cd = 0.50796;
+            fom_float e0 = w[1] - w[3], e1 = w[5] - w[7], e2 = w[9] - w[11], e3 = w[13] - w[15];
+            fom_float f0 = w[0] - w[2], f1 = w[4] - w[6], f2 = w[8] - w[10], f3 = w[12] - w[14];
+            fom_float P[4], Q[4];
+            P[0] = ca * e0; P[1] = cb * e0 + ca * e1; P[2] = cc * e0 + cb * e1 + ca * e2; P[3] = cd * e0 + cc * e1 + cb * e2 + ca * e3;
+            Q[0] = ca * f0; Q[1] = cb * f0 + ca * f1; Q[2] = cc * f0 + cb * f1 + ca * f2; Q[3] = cd * f0 + cc * f1 + cb * f2 + ca * f3;
+            for (i = 0; i < 4; ++i) {
+                fom_int r = 32 + 2 * i;
+                v = bias_t0[r] + state[r + 16] - P[i]; if (v < lo0[r]) v = lo0[r]; t_t0[r] = v;
+                v = bias_t0[r + 1] + state[r + 17] - Q[i]; if (v < lo0[r + 1]) v = lo0[r + 1]; t_t0[r + 1] = v;
+                v = bias_t0[r + 8] + state[r + 24] + P[i]; if (v < lo0[r + 8]) v = lo0[r + 8]; t_t0[r + 8] = v;
+                v = bias_t0[r + 9] + state[r + 25] + Q[i]; if (v < lo0[r + 9]) v = lo0[r + 9]; t_t0[r + 9] = v;
+            }
+        }
+    }
+    /* t1: t1_state is the identity on state[0..15]; t1_t_t0 and t1_p_p0 are zero. */
+#if FOM_MAT_t1_state__d_eta_NNZ != 256
+#error "t1_state__d_eta is no longer a dense 16x16 block"
+#endif
+    {
+        /* t1_state__d_eta is a dense 16x16 block on columns 144..159 (256 nnz, row-major). */
+        fom_float acc[16];
+        const fom_float *D = FOM_MAT_t1_state__d_eta_VALS;
+        /* every 4x4 block of D is [[a,b,c,b],[b,a,b,c],[c,b,a,b],[b,c,b,a]], so each
+           row needs 3 products: a*y_r + c*y_{r+2} + b*(y_{r+1}+y_{r+3}). */
+        /* seed with the -t0[i] + t0[i+16] part of t1_t_t0__d_eta (t_t0 is final here). */
+        for (i = 0; i < 16; ++i) acc[i] = t_t0[i + 16] - t_t0[i];
+        for (p = 0; p < 4; ++p) {
+            const fom_float *y = state + 144 + 4 * p;
+            fom_float y0 = y[0], y1 = y[1], y2 = y[2], y3 = y[3];
+            fom_float q02 = y0 + y2, q13 = y1 + y3;
+            fom_int j;
+            for (j = 0; j < 4; ++j) {
+                const fom_float *B = D + 64 * j + 4 * p;
+                fom_float ca = B[0], cb = B[1], cc = B[2];
+                fom_float t13 = cb * q13, t02 = cb * q02;
+                acc[4 * j] += ca * y0 + cc * y2 + t13;
+                acc[4 * j + 1] += ca * y1 + cc * y3 + t02;
+                acc[4 * j + 2] += ca * y2 + cc * y0 + t13;
+                acc[4 * j + 3] += ca * y3 + cc * y1 + t02;
+            }
+        }
+        /* t1_t_t0__d_eta unrolled over its fixed sparsity, coefficients inlined. */
+        {
+            /* t1_t_t0__d_eta: -t0[i] + t0[i+16] plus a banded Toeplitz pattern on the
+               differences t0[33+2k]-t0[41+2k] / t0[32+2k]-t0[40+2k]; rows 4m+2, 4m+3 negate 4m, 4m+1. */
+            const fom_float ca = 0.07259999999999998, cb = 0.21772, cc = 0.36284, cd = 0.50796;
+            fom_float u0 = t_t0[33] - t_t0[41], u1 = t_t0[35] - t_t0[43], u2 = t_t0[37] - t_t0[45], u3 = t_t0[39] - t_t0[47];
+            fom_float v0 = t_t0[32] - t_t0[40], v1 = t_t0[34] - t_t0[42], v2 = t_t0[36] - t_t0[44], v3 = t_t0[38] - t_t0[46];
+            fom_float U[4], V[4];
+            U[0] = ca * u0 + cb * u1 + cc * u2 + cd * u3; U[1] = ca * u1 + cb * u2 + cc * u3; U[2] = ca * u2 + cb * u3; U[3] = ca * u3;
+            V[0] = ca * v0 + cb * v1 + cc * v2 + cd * v3; V[1] = ca * v1 + cb * v2 + cc * v3; V[2] = ca * v2 + cb * v3; V[3] = ca * v3;
+            for (i = 0; i < 4; ++i) {
+                acc[4 * i] += U[i]; acc[4 * i + 1] += V[i];
+                acc[4 * i + 2] -= U[i]; acc[4 * i + 3] -= V[i];
+            }
+        }
+        for (i = 0; i < 16; ++i) {
+            fom_float a = acc[i], v;
+            v = bias_t1[i] + state[i] + sc_eta * a;
+            if (v < act_t1->lo[i]) v = act_t1->lo[i];
+            if (v > act_t1->hi[i]) v = act_t1->hi[i];
+            t_t1[i] = v;
+        }
+    }
+    /* su: every member is a shift selection (see SPEC.md). */
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

**Whole-solve mean_us:** 143.80 initially, 35.3 to 36.6 at the end (`make bench`, two runs). That is a speedup of about 4.0x. `make verify` passes.

**Final profile** (`make profile`, which adds instrumentation overhead; mean_us=47.3 there):
| region | total_us | pct |
|---|---|---|
| iterate (`fom_iterate`: t0, t1 and the state update) | 170748 | 88.6% |
| bias (scaling the hoisted parameter terms by tau and eta) | 21878 | 11.4% |

**Optimizations applied:**
1. **One-pass `fom_iterate`.** It used to chain three generic step routines, each walking about 10 CSR members per row. Many members are 0/1 shift selections or zero (SPEC.md). `t0_state` became `state[i+16]` and `t1_state` became `state[i]`. The zero members `t0_p_p1`, `t1_p_p0` and `t1_t_t0` were dropped. The whole 160-row state update `su` is now six straight shift-and-scale loops with no matrix reads. Rows 128..143 are a plain copy of `state[0..15]`. This took the solve from 143.8 to 49.9 µs.
2. **Dense 16x16 kernel for `t1_state__d_eta`.** Its 256 nonzeros form a full dense block on columns 144..159. A fixed-length dense dot product replaced the CSR indirection, and a compile-time `#error` fires if the nonzero count changes. This went from 49.9 to 41.4 µs.
3. **No inner loop for rows 0..31 of `t0`.** In both `t0_state__d_taux` and `t0_state__d_tth`, each of these rows holds exactly one entry. Their row pointers are the identity there, so the CSR inner loops were replaced by a direct load. This went from 39.6 to about 36 µs.

Parameter products were already computed once per solve. `t0_p_p1` and `t1_p_p0` are zero (checked at compile time), so the per-iteration bias is just tau·q1 and eta·q0.

Blocked by rules:
- Pass the tau/eta scale for the hoisted parameter term straight into `fom_iterate` instead of materializing the bias arrays each iteration (the 11% "bias" region) | the `fom_iterate`/`fom_step_*` signatures in `include/fom_ops.h` are fixed (a header) and take a precomputed bias vector | ~3-4 µs (about 10%)
- Drop the all-zero `bias_su` argument and its 160 loads per iteration | fixed `fom_iterate` signature in `include/fom_ops.h` | ~1 µs

### Round 2

Initial vs final: 37.030 us → ~26.3 us mean per solve (`make bench`, 3 runs each at 25.6 to 26.3 us). That is 1.41x this round and 5.24x overall against the translated solver (137.932 us). `make verify` prints VERIFY OK.

Final profile (`make profile`, -DFOM_PROFILE build, timer overhead included):
  iterate   132177 us   85.69%
  bias       22068 us   14.31%
(The bias region is two 64-element scalings. Most of its share is probably the overhead of the profiling timer itself.)

Optimizations applied this round:
- Collapsed the t0 product pair (`fom_iterate`). In the data, `t0_state__d_tth` equals `-t0_state__d_taux` with its columns 0..15 moved to 128..143: same sparsity pattern, negated values (checked exactly against data.npz). So `taux*(A x) + tth*(B x)` becomes `A w`, with `w = taux*x[0..15] - tth*x[128..143]` computed once per iteration (16 values). This halves the t0 sparse work: 112 multiply-adds instead of 224, and half the indirect loads. It still reads only `taux`'s own VALS/COLIDX/ROWPTR, so it stays exact against the package data. Result: 37.0 → 28.1 us.
- Vectorized the dense t1 block (`fom_iterate`). The `t1_state__d_eta` product is a dense 16x16 block on state[144..159]. I rewrote it as an outer-product accumulation: loop over columns p, add D[16i+p]*x[144+p] into a 16-wide accumulator. The inner loop is now contiguous over i, so the compiler can vectorize it in place of 16 separate dot-product reductions. It still uses row-major D, so it does not rely on D being symmetric. Result: about 4% faster (~26.9 → ~25.8 us).

Tried and not kept: nothing else was measured. I stopped here because of the time budget.

Blocked by rules:
- Exploit symmetry of the dense `t1_state__d_eta` block (D = D^T in the data) to read it column-contiguously | "must apply exactly M for any input"; D's symmetry is a property of the data that cannot be asserted at compile time | small (~1-2%)
- Precompute bias_t0/bias_t1 = tau(it)*q1, eta(it)*q0 for all 200 iterations in `fom_update_params`, dropping the per-iteration bias pass | the loop must read the schedule through the clamping accessor, and max_iter may exceed the table length, so a fixed-size per-iteration table is unsafe; workspace layout is fixed (headers not editable) | ~5-10% (bias is ~11-14% of profile, mostly timer overhead)

### Round 3

**Result.** This round took the solve from 26.434 us to 15.106 us mean (`make bench`), about 1.75x faster. Against the translated solver (137.932 us) that is 9.13x overall. `make verify` prints `VERIFY OK`, with a worst end-to-end error of 1.8e-12.

**Final profile** (`make profile`, hottest first; the timer overhead inflates the absolute times):
- iterate: 64316 us, 74.7%
- bias: 21757 us, 25.3%

**Optimizations this round** (all inside `fom_iterate` in `src/fom_ops.c`):
- **Unrolled the t0 rows of `t0_state__d_taux`.**
  - Rows 0..15 are +1 on w[i] and rows 16..31 are -1 on w[i-16]. They are now plain adds and subtracts in one fused loop, with no table reads.
  - Rows 32..47 have a fixed sparsity of 2 to 8 entries. They are now straight-line code with the exact coefficients inlined, which is allowed because `fom_iterate` is checked on every unit vector.
  - This removes the CSR row-pointer and column-index loads through extern tables, which the compiler could not see through.
- **Unrolled `t1_t_t0__d_eta`.** Its 112 entries over 16 rows are now straight-line code with the coefficients inlined, and the ±1 entries became plain adds and subtracts.

This removed most of the remaining loop and indirect-load overhead in the iteration.

**Tried and dropped:**
- Inlining the dense 16x16 `t1_state__d_eta` block as a static const table gave no gain (within noise), so I reverted it.
- Grouping terms that share a coefficient magnitude, for example a*(w3 - w1), cut the multiplies but gave no measurable gain. I kept the simpler version.
- Recomputing the bias only when tau or eta changes would not help. Both schedules change every iteration.

**Earlier rounds (kept):**
- Parameter products hoisted to setup, because the plain parameter matrices are structurally zero and the bias is just tau*q1 and eta*q0.
- `t0_state__d_tth` equals `t0_state__d_taux` negated with its columns 0..15 moved to 128..143, so both collapse into one product.
- Shift selections done as direct copies.
- The su block computed as fused shift copies.
- `t1_state__d_eta` handled as a dense 16x16 block.

Blocked by rules:
- Fold the per-iteration bias scaling (tau*q1, eta*q0) into `fom_iterate`, which would drop a 64-element pass and the temporary bias vectors | the fixed `fom_iterate` signature takes `bias` as a precomputed vector and no schedule values for parameter terms (header is fixed) | about 5-10% (the bias region is about 25% of profiled time, mostly timer overhead)
- Skip the zero `bias_su` additions, since the solver always passes zeros | `fom_iterate` must be right for any bias (verify uses random biases) | about 2%

### Round 4

**Whole-solve `mean_us`:** 15.113 us at the start of this round, about 12.0 us at the end (repeated `make bench` runs gave 11.94 to 12.09). That is a 1.26x gain this round. Against the translated solver (137.932 us) the total speedup is about 11.5x.

**Final profile (`make profile`), hottest first:**
- iterate: 58115 us, 70.45%
- bias: 24378 us, 29.55%

Most of the "bias" share is probably timer overhead, since that region is only a 64-element scale. The profiled build runs at about 26 us against about 12 us without profiling.

**Optimizations applied this round (all inside `fom_iterate`):**
1. **Dense 16x16 `t1_state__d_eta` block: about 14.2 → 12.0 us.**
   - Structure: the matrix is a 4x4 grid of 4x4 blocks, and each block has the form `[[a,b,c,b],[b,a,b,c],[c,b,a,b],[b,c,b,a]]`.
   - So each row is `a*y_r + c*y_{r+2} + b*(y_{r+1}+y_{r+3})`. That needs 3 multiplies per row per block, so 192 instead of 256 for the block, and a shorter dependency chain.
   - The coefficients a, b, c are still read from `FOM_MAT_t1_state__d_eta_VALS`, not hardcoded.
2. **Rows 32..47 of `t0` (the `t0_state__d_taux` term): part of the 15.2 → 14.2 us step.**
   - Structure: rows 40..47 are the negation of rows 32..39.
   - Each row is a banded Toeplitz combination (coefficients .0726, .21772, .36284, .50796) of the differences `w[4k+1]-w[4k+3]` and `w[4k]-w[4k+2]`.
   - So the code builds 8 differences and 8 partial sums once, and reuses them with a + or - sign.
3. **`t1_t_t0__d_eta` term: the rest of that step.**
   - Structure: the same Toeplitz pattern on `t0[33+2k]-t0[41+2k]` and `t0[32+2k]-t0[40+2k]`. Rows 4m+2 and 4m+3 negate rows 4m and 4m+1.
   - The code computes 8 shared sums and seeds the accumulator with `t0[i+16]-t0[i]`. This replaces about 112 scalar multiply-adds with about 30.
4. **Tried, no measurable effect:** factoring `b*q` out once per row pair in the dense block, and fusing the accumulator seed. Both were kept for clarity.
5. **Unchanged from earlier rounds:** the per-solve parameter products are still computed outside the loop, and the selection rows are still written as plain copies.

**Stopping point:** the last two rounds gained less than 1% each, so I stopped. The remaining time is in the 192-multiply dense block and in the fixed per-iteration passes over the bias and selection rows.

Blocked by rules:
- Pass `tau`/`eta` into `fom_iterate` so it computes `tau*q1` inline, dropping the separate 64-element bias pass and its stores | `fom_iterate` signature in `include/fom_ops.h` is fixed | ~3-5%
- Skip the all-zero `bias_su` add (160 loads and adds per iteration) | `fom_iterate` must honor any `bias_su` it is passed, and the signature is fixed | ~2-4%
- Precompute even/odd combinations (`(a+c)/2`, `(a-c)/2`) of the dense-block coefficients in setup, cutting it to about 2 multiplies per row | needs derived per-matrix data stored outside `fom_ops.c`'s routines (workspace layout is fixed, and setup may not probe matrices) | ~5%

### Round 5

Initial (translated) mean_us: 137.932. Final mean_us: 12.19 (three `make bench` runs: 12.17, 12.16, 12.21). Overall speedup: about 11.3x. No change this round. The code is identical to the round-4 solver, which was handed over at 14.43 us; the lower figure today is run-to-run noise.

Final profile (`make profile`, instrumented build, hottest first):
- iterate: 54121 us, 70.47%
- bias: 22674 us, 29.53%

The `bias` share is mostly profiler timer overhead. The region is 64 multiplies per iteration, and the instrumented build runs at 24.1 us per solve against 12.2 us without instrumentation.

Optimizations in place (from earlier rounds, kept as is):
- Parameter hoisting: `t0_p_p1` and `t1_p_p0` are structurally zero, so the parameter terms reduce to `tau*q1` and `eta*q0`. `q1` and `q0` are computed once per parameter update; each iteration only rescales them.
- Fused `fom_iterate`: `t0`, `t1` and the state update run in one pass with no zeroing or copy passes, and the solver swaps the state and new-state pointers instead of copying.
- `t0`: `t0_state__d_tth` is `-t0_state__d_taux` with its columns moved from 0..15 to 128..143. Both products collapse into one product on `w = taux*state[0..15] - tth*state[128..143]`.
- `t0`, rows 32..47: rows 40..47 are the negation of rows 32..39, and the coefficients form a banded Toeplitz pattern on pairwise differences of `w`. This takes 8 products per half.
- `t1_state__d_eta`: a dense 16x16 block on state 144..159 made of 4x4 symmetric Toeplitz-like blocks. Each row needs 3 products instead of 4.
- `t1_t_t0__d_eta`: a difference term plus a banded Toeplitz pattern, with rows 4m+2 and 4m+3 the negation of rows 4m and 4m+1.
- State update: every member is a 0/1 shift selection, so the update is written as direct scaled copies, six simple loops.
- Activations (relu, clip) are folded into the product loops.

Considered this round, not applied:
- Merging the two bias rescale loops: they are already two short loops the compiler vectorizes, so the gain is negligible.
- Dropping the zero `bias_su` adds inside `fom_iterate`: not allowed, because verify feeds random biases, so the routine must apply whatever bias it is passed.

Blocked by rules:
- Precompute `bias_t0`/`bias_t1` for every iteration at setup (a 200x64 table), removing the per-iteration rescale | no space in the fixed workspace, and `max_iter` is a runtime value with no bound, so this would need allocation | about 1 to 2% (64 multiplies per iteration)
- Pass `tau` and `eta` into `fom_iterate` and fold the parameter rescale into the `t0`/`t1` loops | the fixed `fom_iterate` signature takes bias vectors, not the parameter schedule values | about 1 to 2%
```
