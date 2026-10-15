# Optimization report: ista / openmp (m325000_n1300000_k8)

Fresh-build head-to-head on della-i12g2, mean per solve over 5 solves after 8 warm-ups:

| solver | mean_us |
|---|---:|
| Phase A | 1464288.217000 |
| Phase B | 1201852.592400 |

Kept: **phase_b** (Phase-A / Phase-B time = 1.218). Final verification:

```
worst max-abs error over 8 samples: 0.000e+00 (tol 1.000e-06)
```

## Diff (Phase A -> kept)

```diff
--- phase_a_solver.c	2026-10-14 16:57:12.377765656 -0400
+++ src/fom_solver.c	2026-10-14 17:07:00.541755286 -0400
@@ -4,6 +4,9 @@
 #include "fom_ops.h"
 #include "fom_profile.h"

+/* Iteration-invariant parameter term t0_p_p0 @ p0 (static, not on stack). */
+static fom_float fom_t0_bias[FOM_T0_DIM];
+
 void fom_set_default_settings(FomSettings *settings) {
     settings->max_iter = FOM_MAX_ITER;
     settings->tol = FOM_TOL;
@@ -26,7 +29,8 @@
     fom_float *state = work->state;
     fom_float *new_state = work->new_state;
     fom_float *acc = work->acc;
-    fom_float *t0 = work->t0;
+    /* The activation is applied in place: t0 aliases acc. */
+    fom_float *t0 = work->acc;
     const fom_float lam = FOM_T0_LAMBDA;
     fom_float residual = 0.0;
     fom_int k, i;
@@ -35,36 +39,47 @@
 #pragma omp parallel for schedule(static)
     for (i = 0; i < FOM_STATE_DIM; ++i) state[i] = FOM_STATE_INIT[i];

+    /* p0 is fixed during a solve: compute t0_p_p0 @ p0 once. */
+#pragma omp parallel for schedule(static)
+    for (i = 0; i < FOM_T0_DIM; ++i) fom_t0_bias[i] = 0.0;
+    fom_apply_t0_p_p0(work->params.p0, fom_t0_bias);
+
     for (k = 0; k < work->settings.max_iter; ++k) {
         /* t0 = soft_threshold(t0_state @ state + t0_p_p0 @ p0, lambda) */
         FOM_PROF_BEGIN(t0_linop);
 #pragma omp parallel for schedule(static)
-        for (i = 0; i < FOM_T0_DIM; ++i) acc[i] = 0.0;
+        for (i = 0; i < FOM_T0_DIM; ++i) acc[i] = fom_t0_bias[i];
         fom_apply_t0_state(state, acc);
-        fom_apply_t0_p_p0(work->params.p0, acc);
         FOM_PROF_END(t0_linop);

         FOM_PROF_BEGIN(t0_act);
 #pragma omp parallel for schedule(static)
         for (i = 0; i < FOM_T0_DIM; ++i) {
-            fom_float v = acc[i];
+            fom_float v = t0[i];
             t0[i] = (v > lam) ? v - lam : ((v < -lam) ? v + lam : 0.0);
+            new_state[i] = 0.0; /* su_t_t0 accumulator, zeroed while hot */
         }
         FOM_PROF_END(t0_act);

         /* state <- su_t_t0 @ t0 */
         FOM_PROF_BEGIN(state_update);
-#pragma omp parallel for schedule(static)
-        for (i = 0; i < FOM_STATE_DIM; ++i) new_state[i] = 0.0;
         fom_apply_su_t_t0(t0, new_state);

-        residual = 0.0;
+        /* Termination is fixed at max_iter, so the residual is only
+           needed after the last step; otherwise just swap buffers. */
+        if (k == work->settings.max_iter - 1) {
+            residual = 0.0;
 #pragma omp parallel for schedule(static) reduction(max:residual)
-        for (i = 0; i < FOM_STATE_DIM; ++i) {
-            fom_float d = new_state[i] - state[i];
-            if (d < 0) d = -d;
-            if (d > residual) residual = d;
-            state[i] = new_state[i];
+            for (i = 0; i < FOM_STATE_DIM; ++i) {
+                fom_float d = new_state[i] - state[i];
+                if (d < 0) d = -d;
+                if (d > residual) residual = d;
+            }
+        }
+        {
+            fom_float *tmp = state;
+            state = new_state;
+            new_state = tmp;
         }
         FOM_PROF_END(state_update);
     }
```

## Agent notes (end of the Phase-B transcript)

```
I cut `fom_solve` from about 1.467 s to about 1.204 s per solve, an 18% speedup. The final `make verify` printed VERIFY OK with a worst error of 0.000e+00, and `make bench` gave mean_us = 1,204,397.

About 89% of the time is now spent inside `fom_apply_t0_state`, the provided sparse matrix-vector product (19.5M nonzeros, run 200 times). That code is in `src/fom_ops.c`, which I'm not allowed to change, so little speedup is left on the `fom_solver.c` side.

**Changes I tried and reverted** (each verified OK but wasn't faster):
- **One fused pass for activation, the `acc ← bias` reset and zeroing `new_state`:** 1,294,067 vs 1,285,115.
- **Moving the state buffers into 2 MB-aligned static arrays with huge pages:** 1,292,959 vs 1,282,775. I hoped this would speed up the matrix product's random reads from the state vector; it didn't.
- **Zeroing `new_state` at the top of the loop, together with the bias copy:** 1,263,420 vs 1,207,692. The large matrix product runs in between and pushes the zeroed buffer out of cache.

Runs varied by about 1% from one to the next, so after the first few changes I compared results using 5 solves per bench.

**Initial mean_us:** 1,466,855.97 (baseline `make bench`, 3 solves)
**Final mean_us:** 1,204,397.20 (final `make bench`, 3 solves; the 5-solve profile run gave 1,204,988)

**Final profile regions** (`make profile BENCH_SOLVES=5`):
| region | total_us | calls | pct |
|---|---:|---:|---:|
| t0_linop | 5,345,604 | 1000 | 88.93 |
| state_update | 346,666 | 1000 | 5.77 |
| t0_act | 318,759 | 1000 | 5.30 |

**Optimizations kept:**
- **Compute `t0_p_p0 @ p0` once per solve:** `p0` doesn't change during a solve, so this product is stored once in a static `fom_t0_bias` array (not on the stack). Each iteration then just copies it into `acc` before the matrix product. The rounding is identical to before (error stays 0). 1,466,856 → 1,390,784.
- **Swap buffers instead of copying, and compute the residual only on the last iteration:** with a fixed 200 iterations the residual is only needed at the end. `state` and `new_state` now swap pointers each step, and the result is copied into `solution.x` at the end. 1,390,784 → 1,285,115.
- **Soft-threshold `acc` in place:** `su_t_t0` now reads `acc` directly, so the separate `t0` buffer isn't written each step. 1,282,775 → 1,207,692.
- **Zero `new_state` inside the activation pass:** this removes one parallel pass, and the buffer is still in cache when `su_t_t0` adds into it. The gain is marginal (1,207,692 → 1,204,756, about 0.25%) and within the noise level.

The `FOM_PROF_BEGIN/END` markers (`t0_linop`, `t0_act`, `state_update`) are still in place, and every solve still starts from `FOM_STATE_INIT`. Only `src/fom_solver.c` was edited.
```
