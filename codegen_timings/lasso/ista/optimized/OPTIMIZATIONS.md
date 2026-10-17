# Optimization report

## Timing

```
translated (emitted)      91.8 us/solve
optimized                 22.5 us/solve
speedup                  4.08x
```

Optimization outcome: **kept (re-verified correct)**.

## What the agent changed

```diff
--- translated (emitted)
+++ optimized
@@ -2,6 +2,22 @@
 #include "fom_data.h"
 #include "fom_ops.h"
 #include "fom_linsolve.h"
+#include "fom_profile.h"
+
+/* Column-major copy of the dense t0_state matrix, built once at setup, so
+   the matvec becomes unit-stride column axpys that vectorize without
+   reassociating a reduction. */
+static fom_float T0_STATE_CM[FOM_T0_DIM * FOM_STATE_DIM];
+static int t0_cm_ready = 0;
+
+static void fom_build_cm(void) {
+    fom_int i, j;
+    if (t0_cm_ready) return;
+    for (i = 0; i < FOM_T0_DIM; ++i)
+        for (j = 0; j < FOM_STATE_DIM; ++j)
+            T0_STATE_CM[j * FOM_T0_DIM + i] = FOM_MAT_t0_state[i * FOM_STATE_DIM + j];
+    t0_cm_ready = 1;
+}
 
 static void fom_reset_state(FomWorkspace *work) {
     fom_int i;
@@ -17,6 +33,7 @@
 void fom_setup(FomWorkspace *work, const FomSettings *settings) {
     fom_int i;
     work->settings = *settings;
+    fom_build_cm();
     for (i = 0; i < FOM_PARAM_P0_DIM; ++i) work->params.p0[i] = 0.0;
     fom_reset_state(work);
     work->solution.iter = 0;
@@ -38,27 +55,37 @@
     fom_float *state = work->state;
     fom_float *ns = work->new_state;
     fom_float *acc = work->acc;
-    fom_float *t0 = work->t0;
     fom_float res = 0.0;
     fom_int status = FOM_MAX_ITER_REACHED;
-    fom_int k, i, ctr = 0;
+    fom_int k, i, j, ctr = 0;
+
+    fom_build_cm();
 
     /* Parameter contribution is loop-invariant: acc = t0_p_p0 @ p0. */
+    FOM_PROF_BEGIN(param_rhs);
     for (i = 0; i < FOM_T0_DIM; ++i) acc[i] = 0.0;
     fom_apply_t0_p_p0(work->params.p0, acc);
+    FOM_PROF_END(param_rhs);
 
+    FOM_PROF_BEGIN(iterations);
     for (k = 0; k < max_iter; ++k) {
-        /* t0 = soft_threshold(t0_state @ state + acc, lam) */
-        for (i = 0; i < FOM_T0_DIM; ++i) t0[i] = acc[i];
-        fom_apply_t0_state(state, t0);
+        /* v = t0_state @ state + acc  (two fused column axpys per pass) */
+        fom_float v[FOM_T0_DIM];
+        for (i = 0; i < FOM_T0_DIM; ++i) v[i] = acc[i];
+        for (j = 0; j < FOM_STATE_DIM; j += 2) {
+            const fom_float xa = state[j], xb = state[j + 1];
+            const fom_float *ca = T0_STATE_CM + j * FOM_T0_DIM;
+            const fom_float *cb = ca + FOM_T0_DIM;
+            for (i = 0; i < FOM_T0_DIM; ++i) {
+                fom_float t = v[i] + ca[i] * xa;
+                v[i] = t + cb[i] * xb;
+            }
+        }
+        /* su_t_t0 is the identity: state <- soft_threshold(v, lam) directly. */
         for (i = 0; i < FOM_T0_DIM; ++i) {
-            fom_float v = t0[i];
-            t0[i] = v > lam ? v - lam : (v < -lam ? v + lam : 0.0);
+            const fom_float t = v[i];
+            ns[i] = t > lam ? t - lam : (t < -lam ? t + lam : 0.0);
         }
-
-        /* state <- su_t_t0 @ t0 */
-        for (i = 0; i < FOM_STATE_DIM; ++i) ns[i] = 0.0;
-        fom_apply_su_t_t0(t0, ns);
 
         if (use_tol || k == max_iter - 1) {
             res = 0.0;
@@ -79,6 +106,7 @@
             break;
         }
     }
+    FOM_PROF_END(iterations);
 
     if (state != work->state)
         for (i = 0; i < FOM_STATE_DIM; ++i) work->state[i] = state[i];
```

## Agent notes

```
**Whole-solve `mean_us`:** initial 90.24 µs → final 22.49 µs (`make bench`), a **~4.0× speedup**. `make verify` gives VERIFY OK, worst max-abs error 1.5e-14 (tolerance 1e-6).

**Final profile breakdown** (`make profile`, mean 24.45 µs including timer overhead):

| Region | Total over 5000 solves | Share |
|---|---|---|
| `iterations` (200× dense 50×50 matvec plus soft-threshold) | 121.3 ms | 99.47% |
| `param_rhs` (`t0_p_p0 @ p0`, computed once per solve) | 0.65 ms | 0.53% |

**Optimizations applied:**
1. **`su_t_t0` is the identity.** Its CSR form has one entry of 1.0 per row on the diagonal. The soft-threshold result is therefore written straight into the new state. This removes the extra 50-element CSR matvec, the zeroing pass and the separate `t0` buffer from every iteration.
2. **`t0_state` is a dense, constant matrix.** It is transposed once, at setup, into a static column-major copy (no dynamic allocation). The matvec then becomes unit-stride column updates (`v += col_j * x_j`), which the compiler vectorizes. The reference helper does a serial row dot product, which cannot be vectorized at `-O3` without fast-math. This was the main gain: 90 → 25 µs.
3. **Two columns fused per pass.** Each element of `v` is updated with column j and then column j+1 in a single loop. This halves the loads and stores of `v` while keeping the same per-element addition order (~6% gain, 25 → 23.4 µs).
4. **The parameter term is constant within a solve.** `acc = t0_p_p0 @ p0` is computed once per solve rather than every iteration (this was already in the starting code).

**Tried and dropped because they were slower:**
- Skipping columns where the state entry is zero: the iterates here are dense, so it made no difference.
- An active-set index list: 30–44 µs.
- Fusing 4 or 5 columns per pass: 25–33 µs.
- Holding 10-row blocks of `v` in registers: 33 µs.
- Aligning the matrix to 64 bytes: within noise.

The last two rounds each gained less than 2%, so I stopped there. The `FOM_PROF_BEGIN/END` markers remain around the `param_rhs` and `iterations` regions. The code remains free of dynamic allocation and division.
```
