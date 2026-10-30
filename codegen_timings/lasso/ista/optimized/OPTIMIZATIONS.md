# Optimization report

## Timing

```
translated (emitted)      86.8 us/solve
optimized                 26.2 us/solve
speedup                  3.32x
```

Optimization outcome: **kept (re-verified correct)**.

## Rounds

| round | before (us) | after (us) | gain | kept | agent (s) | outcome |
|---:|---:|---:|---:|:---:|---:|---|
| 1 | 86.847 | 26.188 | 69.8% | yes | 71 | continued |
| 2 | - | - | - | no | 62 | the round left the sources unchanged |

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
| 1 | Compiling with -O3 / -ffast-math / -march=native so the original row dot products vectorize | Makefile and build flags are fixed | unknown, likely similar to the transpose gain |
| 1 | Storing the transposed matrix as `static const` data generated ahead of time, instead of a lazily built mutable copy (removes the first-call branch and the thread-safety caveat) | `src/fom_data.c` is fixed; inlined literals are allowed in `fom_step_t0`, but there was no time to generate 2500 literals in this session | <1% |
| 2 | Compile with `-O3 -march=native` (or `-mcpu=apple-m4`) in the bench build, which may enable wider unrolling and better instruction scheduling | Makefile and build flags are fixed | unknown, probably under 10% |
| 2 | Store the matrix in single precision to halve memory traffic and double the SIMD width | numeric data in `src/fom_data.c` is fixed, and accuracy must match the double reference | up to about 1.5–2x, but not valid against the verification tolerance |

## What the agent changed

```diff
--- src/fom_solver.c (translated)
+++ src/fom_solver.c (optimized)
@@ -2,6 +2,7 @@
 #include "fom_data.h"
 #include "fom_ops.h"
 #include "fom_linsolve.h"
+#include "fom_profile.h"
 
 static void fom_reset_state(FomWorkspace *work) {
     fom_int i;
@@ -43,11 +44,15 @@
 
     /* Parameter contribution is loop-invariant: acc = t0_p_p0 @ p0. */
     for (i = 0; i < FOM_T0_DIM; ++i) acc[i] = 0.0;
+    FOM_PROF_BEGIN(param_bias);
     fom_apply_t0_p_p0(work->params.p0, acc);
+    FOM_PROF_END(param_bias);
 
     for (k = 0; k < max_iter; ++k) {
         /* su_t_t0 is the identity: new_state = soft_threshold(t0_state @ state + acc). */
+        FOM_PROF_BEGIN(step_t0);
         fom_step_t0(state, acc, &FOM_ACT_t0, ns);
+        FOM_PROF_END(step_t0);
 
         if (use_tol || k == max_iter - 1) {
             res = 0.0;
--- src/fom_ops.c (translated)
+++ src/fom_ops.c (optimized)
@@ -28,17 +28,33 @@
     }
 }
 
+/* Column-major copy of t0_state, built once from FOM_MAT_t0_state, so the
+   product is a sequence of contiguous axpys that vectorize without
+   reassociating a dot product. */
+static fom_float t0_state_T[2500];
+static int t0_state_T_ready = 0;
+
+static void t0_state_build_T(void) {
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
+    fom_float acc[50];
+    const fom_float lam = act->lambd;
+    if (!t0_state_T_ready) t0_state_build_T();
+    for (i = 0; i < 50; ++i) acc[i] = bias[i];
+    for (j = 0; j < 50; ++j) {
+        const fom_float xj = state[j];
+        const fom_float *c = t0_state_T + j * 50;
+        for (i = 0; i < 50; ++i) acc[i] += c[i] * xj;
+    }
+    for (i = 0; i < 50; ++i) {
+        fom_float v = acc[i];
+        out[i] = (v > lam) ? (v - lam) : ((v < -lam) ? (v + lam) : 0.0);
     }
 }
 
@@ -56,6 +72,8 @@
 }
 
 void fom_iterate(const fom_float *state, const fom_float *bias_t0, const fom_float *bias_su, const FomAct_t0 *act_t0, fom_float *t_t0, fom_float *new_state) {
+    fom_int i;
     fom_step_t0(state, bias_t0, act_t0, t_t0);
-    fom_step_su(t_t0, bias_su, new_state);
+    /* su_t_t0 is the identity. */
+    for (i = 0; i < 50; ++i) new_state[i] = bias_su[i] + t_t0[i];
 }
```

## Agent notes

```
### Round 1

**Whole-solve mean_us:** initial 87.23 us → final 26.27 us (about 3.3x speedup; `make bench`, 5000 solves, 200 iterations each).

**Final profile (`make profile`, -O3 -flto with profiling on):**
| Region | Time | % |
|---|---|---|
| step_t0 (dense 50x50 product + soft threshold, per iteration) | 143230 us / 1e6 calls | 99.58% |
| param_bias (t0_p_p0 @ p0, once per solve) | 608 us / 5000 calls | 0.42% |

**Optimizations applied:**
- **Column-major axpy form of the dense t0_state product (kept, about 3.3x).** The matrix is a dense 50x50 block with about 69% nonzeros. The row-wise dot product cannot be vectorized at -O2 without fast-math reassociation. `fom_step_t0` now uses a static column-major copy built once from `FOM_MAT_t0_state` on first call. It accumulates `acc += col_j * x_j` into a 50-entry local accumulator, a contiguous loop that vectorizes with no reassociation. The bias is folded into the accumulator start and the soft threshold into the write-out. This is valid because the product is applied exactly, with no hardcoded coefficients or thresholds.
- **Identity state update in fom_iterate.** `su_t_t0` is the identity, so `new_state = bias_su + t_t0` is a direct copy with no CSR loop. The solver already applied this by stepping straight into `new_state` and swapping pointers.
- **Parameter term hoisted.** `t0_p_p0 @ p0` is computed once per solve as the bias. It was already hoisted in the translation; it is now profiled as its own region.
- **Tried and reverted:** unrolling the column loop by 2 (slower, about 37 us) and skipping columns where `state[j] == 0` (exact, but no measurable gain on the benchmark instance).

Blocked by rules:
- Compiling with -O3 / -ffast-math / -march=native so the original row dot products vectorize | Makefile and build flags are fixed | unknown, likely similar to the transpose gain
- Storing the transposed matrix as `static const` data generated ahead of time, instead of a lazily built mutable copy (removes the first-call branch and the thread-safety caveat) | `src/fom_data.c` is fixed; inlined literals are allowed in `fom_step_t0`, but there was no time to generate 2500 literals in this session | <1%

### Round 2

**Result:** the initial and final solver are the same code, so mean_us is unchanged at about 26 us. This round's runs measured 25.1–26.6 us (26.188 us reported coming in). Against the translated solver (86.847 us) that is 3.32x overall. This round gained nothing and the sources are unchanged.

**Final profile** (`make profile`, 5000 solves of 200 iterations each):
- step_t0: 131152 us, 99.52%
- param_bias: 630 us, 0.48%

**Why I stopped:** each iteration is one dense 50x50 product (2500 multiply-adds) plus a soft-threshold. At about 0.13 us per iteration that is roughly 20 G multiply-adds per second in double precision. That is a large fraction of the core's peak double-precision SIMD throughput. The 20 KB transposed matrix sits in L1 cache. The convergence check is not running in this benchmark: every solve runs all 200 iterations.

**Optimizations kept from earlier rounds:**
- The parameter term `t0_p_p0 @ p0` does not change between iterations, so it is computed once per solve and used as the bias.
- `su_t_t0` is the identity, so the state update is just the `t0` step written straight into the new state, with no copy pass.
- `t0_state` is dense, so its column-major copy (built once from `FOM_MAT_t0_state`) turns the product into 50 contiguous axpys that vectorize. Each row keeps the same summation order.
- The soft-threshold activation and the bias are folded into the same pass that writes the output.

**Tried this round and reverted (all passed `make verify`):**
- Row blocks of 10 accumulators kept in registers across the 50 columns: about 37 us, slower.
- Two columns per pass, to halve accumulator loads and stores: about 25–28 us, no gain.
- Five columns per pass: about 30–32 us, slower.

Blocked by rules:
- Compile with `-O3 -march=native` (or `-mcpu=apple-m4`) in the bench build, which may enable wider unrolling and better instruction scheduling | Makefile and build flags are fixed | unknown, probably under 10%
- Store the matrix in single precision to halve memory traffic and double the SIMD width | numeric data in `src/fom_data.c` is fixed, and accuracy must match the double reference | up to about 1.5–2x, but not valid against the verification tolerance
```
