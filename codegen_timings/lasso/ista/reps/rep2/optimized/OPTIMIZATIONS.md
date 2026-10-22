# Optimization report

## Timing

```
translated (emitted)      93.0 us/solve
optimized                 27.2 us/solve
speedup                  3.42x
```

Optimization outcome: **kept (re-verified correct)**.

## Rounds

| round | before (us) | after (us) | gain | kept | agent (s) | outcome |
|---:|---:|---:|---:|:---:|---:|---|
| 1 | 93.029 | 27.201 | 70.8% | yes | 69 | continued |
| 2 | 26.960 | 27.216 | -1.0% | no | 73 | the round's sources were not faster than the best so far |

## Kernel checks

Each `fom_apply_<name>` in the shipped `fom_ops.c`, checked by `make verify` against the package's reference over the same `fom_data.c` matrix. *basis* probes every unit vector, which recovers the routine's matrix column by column (an exact check), plus random vectors; *random* uses random vectors only (the matrix is over the probe budget). A `fused_<block>` row is a `fom_fused_<block>`, checked against the sum of its members: *basis* there probes every unit vector of each source with every schedule value at 0 and with each alone at 1, which recovers every member exactly.

| matrix | shape | nnz | rewritten | check | probes | max error |
|---|---|---|---|---|---|---|
| `t0_state` | 50x50 | 1730 | yes | basis | 52 | 0.000e+00 |
| `t0_p_p0` | 50x25 | 280 | yes | basis | 27 | 0.000e+00 |
| `su_t_t0` | 50x50 | 50 | yes | basis | 52 | 0.000e+00 |
| `step_t0` | 50x50 | 1730 | yes | basis | 56 | 1.776e-15 |
| `step_su` | 50x50 | 50 | yes | basis | 52 | 0.000e+00 |
| `iterate` | 50x50 | 1780 | yes | basis | 106 | 1.332e-15 |

### Rule violations

_None: the collected rewrite passed the rules and was kept._

## Blocked by rules

Optimizations the agent considered but did not make because a rule forbade them, in its own words.

| round | idea | rule | expected gain |
|---|---|---|---|
| 2 | Storing `t0_state` in float32 to halve the load traffic | the float64 accuracy that `make verify` requires | unknown, probably 10–20% if it were allowed |

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
@@ -31,13 +32,17 @@
     fom_int max_iter = work->settings.max_iter, k, i;
     fom_float res = 0.0;
 
+    FOM_PROF_BEGIN(bias);
     memset(bias, 0, FOM_T0_DIM * sizeof(fom_float));
     fom_apply_t0_p_p0(work->params.p0, bias);
+    FOM_PROF_END(bias);
+    FOM_PROF_BEGIN(iterate);
 
     for (k = 0; k < max_iter; ++k) {
         fom_step_t0(x, bias, &FOM_ACT_t0, y);
         tswap = x; x = y; y = tswap;
     }
+    FOM_PROF_END(iterate);
     if (max_iter > 0) {
         for (i = 0; i < FOM_STATE_DIM; ++i) {
             fom_float d = x[i] - y[i];
--- src/fom_ops.c (translated)
+++ src/fom_ops.c (optimized)
@@ -28,17 +28,40 @@
     }
 }
 
+/* Column-major copy of t0_state, built on first use, so the product is a
+   sequence of contiguous axpys (vectorizes without reassociation) and the
+   columns whose state entry is zero (soft-threshold sparsity) are skipped. */
+static fom_float t0_state_T[2500];
+static int t0_state_T_ready = 0;
+
+static void t0_state_T_init(void) {
+    fom_int i, j;
+    for (i = 0; i < 50; ++i)
+        for (j = 0; j < 50; ++j) t0_state_T[j * 50 + i] = FOM_MAT_t0_state[i * 50 + j];
+    t0_state_T_ready = 1;
+}
+
 void fom_step_t0(const fom_float *state, const fom_float *bias, const FomAct_t0 *act, fom_float *out) {
-    fom_int i, r, j;
-    for (r = 0; r < 50; ++r) {
-        fom_float s, v;
-        i = r;
-        v = bias[i];
-        s = 0.0;
-        for (j = 0; j < 50; ++j) s += FOM_MAT_t0_state[i * 50 + j] * state[j];
-        v += s;
-        v = (v > act->lambd) ? (v - act->lambd) : ((v < -act->lambd) ? (v + act->lambd) : 0.0);
-        out[i] = v;
+    fom_int i, j;
+    fom_float v[50];
+    const fom_float lam = act->lambd;
+    if (!t0_state_T_ready) t0_state_T_init();
+    for (i = 0; i < 50; ++i) v[i] = bias[i];
+    fom_int nz[50], n = 0, k;
+    for (j = 0; j < 50; ++j) { nz[n] = j; n += (state[j] != 0.0); }
+    for (k = 0; k + 1 < n; k += 2) {
+        const fom_float xa = state[nz[k]], xb = state[nz[k + 1]];
+        const fom_float *ca = t0_state_T + nz[k] * 50, *cb = t0_state_T + nz[k + 1] * 50;
+        for (i = 0; i < 50; ++i) v[i] = (v[i] + ca[i] * xa) + cb[i] * xb;
+    }
+    if (k < n) {
+        const fom_float xa = state[nz[k]];
+        const fom_float *ca = t0_state_T + nz[k] * 50;
+        for (i = 0; i < 50; ++i) v[i] += ca[i] * xa;
+    }
+    for (i = 0; i < 50; ++i) {
+        fom_float a = v[i];
+        out[i] = (a > lam) ? (a - lam) : ((a < -lam) ? (a + lam) : 0.0);
     }
 }
```

## Agent notes

```
### Round 1

**Whole-solve mean_us:** initial 92.19 us, final 27.31 us. Speedup about 3.4x.

**Final profile (make profile), hottest first:**
- iterate (200 ISTA steps, `fom_step_t0`): 139859 us total over 5000 calls (about 28 us per solve), 99.54%
- bias (`t0_p_p0 @ p0`, once per solve): 641 us total (about 0.13 us per solve), 0.46%

**Optimizations applied:**
1. **Column-oriented dense product in `fom_step_t0`.** On first use I build a static column-major copy of the 50x50 dense `t0_state` from its own data, and the product becomes a series of contiguous axpys `v += A[:,j] * x[j]`. The original row dot product is a serial reduction that the compiler cannot vectorize without fast-math. The axpy form vectorizes cleanly with SIMD and keeps strict floating-point semantics.
2. **Skipping zero columns.** ISTA's soft-threshold makes the iterate sparse, the usual lasso behavior. Columns where `x[j] == 0` add nothing, so I build a branch-free index list of the nonzero entries and loop only over those. This is exact for every input: zero entries contribute exactly zero, and NaN is not skipped. This was the main gain.
3. **Two columns per pass.** The loop processes two nonzero columns per pass over `v`, which halves the loads and stores of the accumulator.
4. **Fused step.** Bias, product and soft-threshold run in a single routine that writes straight into the ping-pong destination. The state update is an identity selection, so the solver just swaps pointers, with no copy pass or `su` step. The bias `t0_p_p0 @ p0` is computed once per solve.

**Tried and reverted:**
- Four columns per pass: no faster, within noise.
- Holding 10-row chunks in registers across all columns: slower, about 39 us.

**Profiling:** `FOM_PROF_BEGIN/END` markers stay around the bias and iterate regions in `src/fom_solver.c`.

Blocked by rules:
- none

### Round 2

**Whole solve (`make bench` mean_us):** 93.029 µs for the translated solver, and 27.201 µs at the start of this round. The final version measures about 26.6 µs, roughly 3.5x faster than the translation. This round's gain is only about 3%, and the timer is noisy (±1.5 µs). In three alternating runs the old version measured 26.8, 28.2 and 28.2 µs, and the new one 26.7, 27.1 and 26.6 µs.

**Final profile (`make profile`), hottest first:**
| Region | Time | % |
|---|---|---|
| iterate | 144981 µs (5000 calls) | 99.58% |
| bias | 612 µs (5000 calls) | 0.42% |

**Optimizations:**
- **Kept this round:** the soft-threshold in `fom_step_t0` is now computed as `a - clamp(a, -lam, lam)`. It has no branches, so the final 50-row pass vectorizes. For every input it gives exactly `a-lam`, `a+lam` or 0, the same as the old three-way branch. `lam` is still read from `act`, so nothing is hardcoded.
- **Kept from earlier rounds:**
  - A column-major copy of the dense 50×50 `t0_state`, so the product becomes a series of contiguous axpys that vectorize.
  - Columns are skipped where the state entry is 0. Soft-threshold makes the iterate sparse, so this skips real work.
  - The constant `p0` term is computed once per solve.
  - The update `su_t_t0` is an identity, so the solver ping-pongs between the two state buffers instead of copying.
- **Tried and reverted:**
  - Blocking four columns per pass instead of two: about 29.5 µs, slower.
  - Keeping row chunks of 10 or 25 accumulators in registers across all nonzero columns: 42 µs and 31.5 µs, much slower. Each chunk's additions form one serial chain of FMAs, and their latency dominates.

**What's left:** the iteration is a dense 50×50 product followed by a threshold, already vectorized and skipping zero columns. The matrix (20 KB) stays in L1 cache. I see no further structure to exploit in it.

Blocked by rules:
- Storing `t0_state` in float32 to halve the load traffic | the float64 accuracy that `make verify` requires | unknown, probably 10–20% if it were allowed
```
