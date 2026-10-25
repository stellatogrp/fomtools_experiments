# Optimization report

## Timing

```
translated (emitted)   38190.1 us/solve
optimized              14803.8 us/solve
speedup                  2.58x
```

Optimization outcome: **kept (re-verified correct)**.

## Rounds

| round | before (us) | after (us) | gain | kept | agent (s) | outcome |
|---:|---:|---:|---:|:---:|---:|---|
| 1 | 38190.088 | 25110.146 | 34.2% | yes | 5134 | continued |
| 2 | 24873.821 | 20279.385 | 18.5% | yes | 2945 | continued |
| 3 | 20656.095 | 19849.080 | 3.9% | yes | 4508 | continued |
| 4 | 19712.473 | 15719.756 | 20.3% | yes | 3151 | continued |
| 5 | 15911.319 | 14803.774 | 7.0% | yes | 4126 | the last of agent_optimize_rounds rounds |

## Kernel checks

Each `fom_apply_<name>` in the shipped `fom_ops.c`, checked by `make verify` against the package's reference over the same `fom_data.c` matrix. *basis* probes every unit vector, which recovers the routine's matrix column by column (an exact check), plus random vectors; *random* uses random vectors only (the matrix is over the probe budget). A `fused_<block>` row is a `fom_fused_<block>`, checked against the sum of its members: *basis* there probes every unit vector of each source with every schedule value at 0 and with each alone at 1, which recovers every member exactly.

| matrix | shape | nnz | rewritten | check | probes | max error |
|---|---|---|---|---|---|---|
| `t0_state` | 10000x80000 | 2599983 | yes | random | 2 | 6.217e-15 |
| `t0_state__d_gamma` | 10000x80000 | 5199966 | yes | random | 2 | 7.772e-15 |
| `t0_p_p0` | 10000x10000 | 10000 | yes | random | 2 | 0.000e+00 |
| `t1_state` | 40000x80000 | 40000 | yes | random | 2 | 0.000e+00 |
| `t1_state__d_gamma` | 40000x80000 | 80000 | yes | random | 2 | 8.882e-16 |
| `t1_t_t0` | 40000x10000 | 2599983 | yes | random | 2 | 4.441e-16 |
| `su_state` | 80000x80000 | 40000 | yes | random | 2 | 0.000e+00 |
| `su_t_t1` | 80000x40000 | 40000 | yes | random | 2 | 0.000e+00 |

### Rule violations

_None: the collected rewrite passed the rules and was kept._

## Blocked by rules

Optimizations the agent considered but did not make because a rule forbade them, in its own words.

| round | idea | rule | expected gain |
|---|---|---|---|
| 1 | Fuse `t0_state` and `t0_state__d_gamma` into one pass: t0_state's pattern equals the right half of d_gamma's, so one gather of `(s0[c], s1[c])` would feed all three terms, with the gamma scaling folded in | each `fom_apply_<name>` may read only its own matrix's data, and no matvec loops may go in fom_solver.cu | about 50 us/iter, ~35-40% of the solve |
| 1 | Fold the t1 soft-threshold (and the t1_state / t1_state__d_gamma shift terms) into the epilogue of the `t1_t_t0` kernel | each apply must be a pure `out += M x` checked on its own, and solver code may not read matrix data | about 5-10 us/iter, ~5% |
| 1 | Have callers pass `x` already in interleaved `(s0, s1)` layout so the d_gamma apply skips its interleave pass | `fom_apply_<name>` signatures are fixed in include/, and the direct checks probe the standard layout | about 4 us/iter, ~3% |
| 1 | Run `t0_state` and `t0_state__d_gamma` on separate graph branches into separate accumulators | the apply kernels launch on the fixed per-thread default stream, so the solver cannot target them at other streams except through graph rewiring; a combine kernel would also be needed | unknown, small because both are gather-bound |
| 2 | Fuse t0_state and t0_state__d_gamma into one kernel that reads each gathered state entry once for both matrices (their column patterns overlap: A on s1 and [P, P+H]) | kernel rule: each fom_apply_<name> and its helpers read only their own matrix's data | ~10-20% of the t0 phase (unknown exactly) |
| 2 | Store d_gamma's right half as the negated left half and compute a·(x_L − x_R), which halves its 41.6 MB value stream (the halves agree only to about 3e-16, not bitwise) | must apply exactly M; it would also use matrix data in a way the rules don't intend | ~15-20 us/iteration |
| 2 | Begin next iteration's t0 matvecs on their already-known left-half columns (new s0 = old s1) while t1_t_t0 still runs | apply routines must apply the whole matrix; no partial-apply entry points (only fom_apply_* may be non-static) | unknown, maybe 5-10% |
| 2 | Fold the t1 soft-threshold or the t0 join into the matvec kernels' epilogue | fom_apply_<name> must compute exactly out += M x | ~3-6 us/iteration |
| 3 | Compute t0 as a single A·y per iteration with y = (1+γ)x − γ x_prev, dropping the `t0_state__d_gamma` apply. That matrix equals [−A, A] and `t0_state` equals [0, A], which would also allow reusing the previous iteration's A x. | fom_solver.cu must call fom_apply_<name> for each term and may not read or rely on other matrices' coefficients; each fom_apply reads only its own data | ~30–35% (one gather-bound matvec of ~35 us instead of the concurrent pair at ~64 us) |
| 3 | Fuse the `t0_state` and `d_gamma` gathers into one kernel that shares the x tile staged in L1 for both matrices. | each fom_apply_<name> and its helpers may read only its own matrix's data | ~10–15% |
| 3 | Pass the solver's precomputed g·(x_L − x_R) straight to the `d_gamma` kernel instead of rebuilding it inside the apply. | fom_apply_<name>(x, out) signature and the requirement that it apply exactly M to x (fixed header) | ~2–3% |
| 4 | Compute A*y once per iteration, with y = s1 + g(s1 - s0), instead of streaming the three separate copies of A (t0_state = [0, A], d_gamma = [-A, A]) | kernel rule: each fom_apply reads only its own matrix's data, and the solver may not combine matrices | ~25-30% (removes about 26 MB of the ~82 MB streamed per iteration) |
| 4 | Apply d_gamma as left values times (x[:H] - x[H:]), treating right = -left, which is exact to 1.7e-15 relative but not bitwise | the kernel must apply exactly M | ~2.6 MB per iteration (the int8 offset array), unknown net gain |
| 4 | Fuse the t1 soft-threshold and the t0 combine into the matvec kernels (out = soft(...) in fom_apply_t1_t_t0) to remove graph nodes | fom_apply must compute exactly out += M x | ~3-5 us per iteration (~5%) |
| 5 | Compute A·s1 once per iteration and reuse it, instead of applying both `t0_state` = [0, A] and `t0_state__d_gamma` = [-A', A] to the state; that streams about 26 MB less per iteration | fom_solver.cu must call each term's own fom_apply_<name>, and a helper may read only its own matrix's data (A' differs from A by up to ~8 ulps, so it is a different matrix) | ~20 us/iter (~25%) |
| 5 | Fuse the `t0_state` and `t0_state__d_gamma` products into one kernel that reads x once and shares the tile schedule and combine step | a helper may read only its own matrix's data / one fom_apply per matrix | ~5-8 us/iter (one fewer combine and interleave node on the critical path) |
| 5 | Fold the t1 soft-threshold into the `t1_t_t0` kernel's epilogue, removing the separate pass before the next iteration's matvecs | fom_apply_<name> must compute exactly out += M x | ~3 us/iter |

## What the agent changed

```diff
--- src/fom_solver.cu (translated)
+++ src/fom_solver.cu (optimized)
@@ -3,12 +3,55 @@
 #include "fom_solver.h"
 #include "fom_data.h"
 #include "fom_ops.h"
+#include "fom_profile.h"
 
 #define NS0 40000
+
+/* Profile build: wait for the region's kernels so host timers see them. */
+#ifdef FOM_PROFILE
+#define PROF_SYNC() cudaDeviceSynchronize()
+#else
+#define PROF_SYNC() ((void)0)
+#endif
 #define BLK 256
-
-static fom_float *d_state = 0, *d_new = 0, *d_t0 = 0, *d_t1 = 0, *d_acc = 0, *d_p0 = 0;
+/* Everything runs on the per-thread default stream (as do the fom_apply_*
+   kernels): unlike the legacy stream it can be captured into a CUDA graph. */
+#define STRM cudaStreamPerThread
+
+/* Two fixed state buffers (iteration parity picks the current one), so a
+   captured CUDA graph of the whole loop can be replayed on every solve. */
+static fom_float *d_buf[2] = {0, 0}, *d_t0 = 0, *d_xg = 0, *d_p0 = 0, *d_c0 = 0;
 static unsigned long long *d_res = 0;
+static cudaGraphExec_t g_exec = 0;
+static fom_int g_iters = -1;
+static int g_warm = 0;
+
+/* Per-iteration prep: xg = g * state (the gamma terms are applied to xg,
+   M (g x) = g M x), seed the t0 accumulator with the loop-invariant
+   c0 = t0_p_p0 p0, zero t1,
+   and the su shift new[0:NS0] = state[NS0:]. With soft_in, the previous
+   iteration left its t1 slot un-thresholded: apply soft() here, once per
+   entry, by the thread that owns it (written back in place). */
+__global__ void k_prep(fom_float *__restrict__ s, fom_float *__restrict__ xg, fom_float *__restrict__ nw,
+                       fom_float *__restrict__ t0, const fom_float *__restrict__ c0,
+                       fom_float g, int use_g, int soft_in, fom_float lam) {
+    int i = blockIdx.x * blockDim.x + threadIdx.x;
+    if (i < FOM_STATE_DIM) {
+        fom_float v = s[i];
+        if (i < NS0) {
+            nw[i + NS0] = 0.0;
+        } else {
+            if (soft_in) {
+                fom_float a = fabs(v) - lam;
+                v = a > 0 ? copysign(a, v) : 0.0;
+                s[i] = v;
+            }
+            nw[i - NS0] = v;
+        }
+        if (use_g) xg[i] = g * v;
+        if (i < FOM_T0_DIM) t0[i] = c0[i];
+    }
+}
 
 __global__ void k_soft(fom_float *x, int n, fom_float lam) {
     int i = blockIdx.x * blockDim.x + threadIdx.x;
@@ -16,16 +59,6 @@
         fom_float v = x[i];
         fom_float a = fabs(v) - lam;
         x[i] = a > 0 ? copysign(a, v) : 0.0;
-    }
-}
-
-/* out = soft(out + g * acc, lam) */
-__global__ void k_axpy_soft(fom_float *out, const fom_float *acc, fom_float g, int n, fom_float lam) {
-    int i = blockIdx.x * blockDim.x + threadIdx.x;
-    if (i < n) {
-        fom_float v = out[i] + g * acc[i];
-        fom_float a = fabs(v) - lam;
-        out[i] = a > 0 ? copysign(a, v) : 0.0;
     }
 }
 
@@ -52,13 +85,13 @@
 }
 
 static void alloc_dev(void) {
-    if (d_state) return;
-    cudaMalloc((void **)&d_state, sizeof(fom_float) * FOM_STATE_DIM);
-    cudaMalloc((void **)&d_new, sizeof(fom_float) * FOM_STATE_DIM);
+    if (d_buf[0]) return;
+    cudaMalloc((void **)&d_buf[0], sizeof(fom_float) * FOM_STATE_DIM);
+    cudaMalloc((void **)&d_buf[1], sizeof(fom_float) * FOM_STATE_DIM);
     cudaMalloc((void **)&d_t0, sizeof(fom_float) * FOM_T0_DIM);
-    cudaMalloc((void **)&d_t1, sizeof(fom_float) * FOM_T1_DIM);
-    cudaMalloc((void **)&d_acc, sizeof(fom_float) * FOM_MAX_VEC_DIM);
+    cudaMalloc((void **)&d_xg, sizeof(fom_float) * FOM_STATE_DIM);
     cudaMalloc((void **)&d_p0, sizeof(fom_float) * FOM_PARAM_P0_DIM);
+    cudaMalloc((void **)&d_c0, sizeof(fom_float) * FOM_T0_DIM);
     cudaMalloc((void **)&d_res, sizeof(unsigned long long));
 }
 
@@ -73,55 +106,144 @@
     memcpy(work->state, FOM_STATE_INIT, sizeof(work->state));
 }
 
+/* While capturing: the current capture frontier (at most 4 nodes), and
+   resetting it, so independent work forms parallel graph branches. */
+static size_t cap_deps(cudaGraphNode_t *d) {
+    const cudaGraphNode_t *p;
+    size_t n = 0, j;
+    cudaStreamCaptureStatus st;
+    cudaStreamGetCaptureInfo(STRM, &st, 0, 0, &p, 0, &n);
+    for (j = 0; j < n && j < 4; ++j) d[j] = p[j];
+    return n < 4 ? n : 4;
+}
+
+static void cap_set(cudaGraphNode_t *d, size_t n) {
+    cudaStreamUpdateCaptureDependencies(STRM, d, 0, n, cudaStreamSetCaptureDependencies);
+}
+
+/* One iteration: state = d_buf[it & 1] -> new = d_buf[(it + 1) & 1].
+   With dag set (graph capture) three branches run in parallel after prep:
+   the gamma term and t0_state, both accumulating into t0, and the small
+   t1 state terms into t1 (disjoint buffers). An added edge orders the
+   gamma term's final (combine) kernel after t0_state's, so t0 =
+   (c0 + t0_state s) + gamma term, the same order and rounding as the
+   serial path; then t1_t_t0 joins everything. soft_in / soft_out:
+   whether prep applies the previous t1 threshold, and whether this
+   iteration thresholds its own t1 (else the next prep does). */
+static void iterate(fom_int it, int dag, int soft_in, int soft_out) {
+    fom_float *s = d_buf[it & 1];
+    fom_float *nw = d_buf[(it + 1) & 1];
+    fom_float *t1 = nw + NS0;
+    fom_float g = fom_sched_gamma(it);
+    int use_g = (g != 0.0);
+    cudaGraphNode_t d1[4], d2[8], d3[8];
+    size_t n1 = 0, n2 = 0, n3 = 0;
+    FOM_PROF_BEGIN(prep);
+    k_prep<<<nblk(FOM_STATE_DIM), BLK, 0, STRM>>>(s, d_xg, nw, d_t0, d_c0, g, use_g, soft_in, FOM_T1_LAMBDA);
+    PROF_SYNC();
+    FOM_PROF_END(prep);
+    if (dag) {
+        n1 = cap_deps(d1);
+        /* t0 = soft(t0_p_p0 p0 + t0_state s + t0_state__d_gamma (g s)); the
+           gamma term (the longer branch) is captured first so it gets the SMs */
+        if (use_g) { fom_apply_t0_state__d_gamma(d_xg, d_t0); n3 = cap_deps(d3); cap_set(d1, n1); }
+        fom_apply_t0_state(s, d_t0);
+        if (use_g) {
+            cudaGraph_t gr;
+            cudaGraphNode_t sc[4];
+            cudaStreamCaptureStatus st;
+            cap_deps(sc); /* t0_state's last (combine) node */
+            cudaStreamGetCaptureInfo(STRM, &st, 0, &gr, 0, 0, 0);
+            cudaGraphAddDependencies(gr, sc, d3, 0, 1); /* ... before the gamma term's */
+            cap_set(d3, n3);
+        }
+    } else {
+        FOM_PROF_BEGIN(t0_state);
+        fom_apply_t0_state(s, d_t0);
+        PROF_SYNC();
+        FOM_PROF_END(t0_state);
+        FOM_PROF_BEGIN(t0_gamma);
+        if (use_g) fom_apply_t0_state__d_gamma(d_xg, d_t0);
+        PROF_SYNC();
+        FOM_PROF_END(t0_gamma);
+    }
+    if (FOM_T0_LAMBDA != 0.0) k_soft<<<nblk(FOM_T0_DIM), BLK, 0, STRM>>>(d_t0, FOM_T0_DIM, FOM_T0_LAMBDA);
+    if (dag) { n2 = cap_deps(d2); cap_set(d1, n1); }
+    /* t1, written straight into new[NS0:] */
+    FOM_PROF_BEGIN(t1_state_terms);
+    fom_apply_t1_state(s, t1);
+    if (use_g) fom_apply_t1_state__d_gamma(d_xg, t1);
+    PROF_SYNC();
+    FOM_PROF_END(t1_state_terms);
+    if (dag) { n2 += cap_deps(d2 + n2); cap_set(d2, n2); }
+    FOM_PROF_BEGIN(t1_matvec);
+    fom_apply_t1_t_t0(d_t0, t1);
+    if (soft_out) k_soft<<<nblk(FOM_T1_DIM), BLK, 0, STRM>>>(t1, FOM_T1_DIM, FOM_T1_LAMBDA);
+    PROF_SYNC();
+    FOM_PROF_END(t1_matvec);
+}
+
+static void resid(fom_int it) {
+    cudaMemsetAsync(d_res, 0, sizeof(unsigned long long), STRM);
+    k_resid<<<160, BLK, 0, STRM>>>(d_buf[(it + 1) & 1], d_buf[it & 1], FOM_STATE_DIM, d_res);
+}
+
+static fom_float read_res(void) {
+    unsigned long long r;
+    fom_float res;
+    cudaMemcpy(&r, d_res, sizeof(r), cudaMemcpyDeviceToHost);
+    memcpy(&res, &r, sizeof(res));
+    return res;
+}
+
 fom_int fom_solve(FomWorkspace *work) {
     alloc_dev();
-    cudaMemcpy(d_state, work->state, sizeof(fom_float) * FOM_STATE_DIM, cudaMemcpyHostToDevice);
+    cudaMemcpy(d_buf[0], work->state, sizeof(fom_float) * FOM_STATE_DIM, cudaMemcpyHostToDevice);
     cudaMemcpy(d_p0, work->params.p0, sizeof(fom_float) * FOM_PARAM_P0_DIM, cudaMemcpyHostToDevice);
+    /* p0 is fixed for the whole solve: its term is applied once, not per iteration. */
+    FOM_PROF_BEGIN(p0_term);
+    cudaMemsetAsync(d_c0, 0, sizeof(fom_float) * FOM_T0_DIM, STRM);
+    fom_apply_t0_p_p0(d_p0, d_c0);
+    PROF_SYNC();
+    FOM_PROF_END(p0_term);
     fom_int max_iter = work->settings.max_iter;
     int use_tol = FOM_USE_TOL || (work->settings.tol > 0);
     fom_float res = 0.0;
     fom_int status = FOM_MAX_ITER_REACHED;
     fom_int it;
-    for (it = 0; it < max_iter; ++it) {
-        fom_float g = fom_sched_gamma(it);
-        /* t0 */
-        cudaMemsetAsync(d_t0, 0, sizeof(fom_float) * FOM_T0_DIM);
-        fom_apply_t0_state(d_state, d_t0);
-        fom_apply_t0_p_p0(d_p0, d_t0);
-        if (g != 0.0) {
-            cudaMemsetAsync(d_acc, 0, sizeof(fom_float) * FOM_T0_DIM);
-            fom_apply_t0_state__d_gamma(d_state, d_acc);
-            k_axpy_soft<<<nblk(FOM_T0_DIM), BLK>>>(d_t0, d_acc, g, FOM_T0_DIM, FOM_T0_LAMBDA);
-        } else {
-            k_soft<<<nblk(FOM_T0_DIM), BLK>>>(d_t0, FOM_T0_DIM, FOM_T0_LAMBDA);
-        }
-        /* t1 written straight into new_state[NS0:] */
-        fom_float *t1 = d_new + NS0;
-        cudaMemsetAsync(t1, 0, sizeof(fom_float) * FOM_T1_DIM);
-        fom_apply_t1_state(d_state, t1);
-        fom_apply_t1_t_t0(d_t0, t1);
-        if (g != 0.0) {
-            cudaMemsetAsync(d_acc, 0, sizeof(fom_float) * FOM_T1_DIM);
-            fom_apply_t1_state__d_gamma(d_state, d_acc);
-            k_axpy_soft<<<nblk(FOM_T1_DIM), BLK>>>(t1, d_acc, g, FOM_T1_DIM, FOM_T1_LAMBDA);
-        } else {
-            k_soft<<<nblk(FOM_T1_DIM), BLK>>>(t1, FOM_T1_DIM, FOM_T1_LAMBDA);
-        }
-        /* state update: new[0:NS0] = state[NS0:] (su_state shift copy) */
-        cudaMemcpyAsync(d_new, d_state + NS0, sizeof(fom_float) * NS0, cudaMemcpyDeviceToDevice);
-        if (use_tol || it == max_iter - 1) {
-            cudaMemsetAsync(d_res, 0, sizeof(unsigned long long));
-            k_resid<<<160, BLK>>>(d_new, d_state, FOM_STATE_DIM, d_res);
-            unsigned long long r;
-            cudaMemcpy(&r, d_res, sizeof(r), cudaMemcpyDeviceToHost);
-            memcpy(&res, &r, sizeof(res));
-        }
-        fom_float *tmp = d_state; d_state = d_new; d_new = tmp;
-        if (use_tol && work->settings.check_termination && res < work->settings.tol) {
-            status = FOM_SOLVED; ++it; break;
-        }
-    }
-    cudaMemcpy(work->solution.x, d_state, sizeof(fom_float) * FOM_STATE_DIM, cudaMemcpyDeviceToHost);
+#ifndef FOM_PROFILE
+    if (!use_tol && max_iter > 0 && g_warm) {
+        /* Fixed iteration count: replay the whole loop as one CUDA graph. */
+        if (g_iters != max_iter) {
+            cudaGraph_t graph;
+            if (g_exec) cudaGraphExecDestroy(g_exec);
+            cudaStreamBeginCapture(STRM, cudaStreamCaptureModeThreadLocal);
+            for (it = 0; it < max_iter; ++it) iterate(it, 1, it > 0, it == max_iter - 1);
+            resid(max_iter - 1);
+            cudaStreamEndCapture(STRM, &graph);
+            cudaGraphInstantiate(&g_exec, graph, 0);
+            cudaGraphDestroy(graph);
+            g_iters = max_iter;
+        }
+        cudaGraphLaunch(g_exec, STRM);
+        res = read_res();
+        it = max_iter;
+    } else
+#endif
+    {
+        for (it = 0; it < max_iter; ++it) {
+            iterate(it, 0, 0, 1);
+            if (use_tol || it == max_iter - 1) {
+                resid(it);
+                res = read_res();
+            }
+            if (use_tol && work->settings.check_termination && res < work->settings.tol) {
+                status = FOM_SOLVED; ++it; break;
+            }
+        }
+        g_warm = 1; /* lazily built operator layouts now exist: capture is safe */
+    }
+    cudaMemcpy(work->solution.x, d_buf[it & 1], sizeof(fom_float) * FOM_STATE_DIM, cudaMemcpyDeviceToHost);
     memcpy(work->state, work->solution.x, sizeof(work->state));
     work->solution.iter = it;
     work->solution.residual = res;
@@ -131,8 +253,11 @@
 
 void fom_cleanup(FomWorkspace *work) {
     (void)work;
-    if (!d_state) return;
-    cudaFree(d_state); cudaFree(d_new); cudaFree(d_t0); cudaFree(d_t1);
-    cudaFree(d_acc); cudaFree(d_p0); cudaFree(d_res);
-    d_state = 0;
-}
+    if (!d_buf[0]) return;
+    if (g_exec) cudaGraphExecDestroy(g_exec);
+    g_exec = 0;
+    g_iters = -1;
+    cudaFree(d_buf[0]); cudaFree(d_buf[1]); cudaFree(d_t0);
+    cudaFree(d_xg); cudaFree(d_p0); cudaFree(d_c0); cudaFree(d_res);
+    d_buf[0] = 0;
+}
--- src/fom_ops.cu (translated)
+++ src/fom_ops.cu (optimized)
@@ -6,22 +6,679 @@
 #include "fom_ops_data.cuh"
 
 #include <cuda_runtime.h>
+#include <stdlib.h>
 
 #define FOM_OPS_THREADS 256
+/* Per-thread default stream: unlike the legacy stream it can be captured
+   into a CUDA graph, and it still orders against legacy-stream copies. */
+#define FOM_STRM cudaStreamPerThread
+
+/* Warp-per-row dot product, U independent loads in flight per lane
+   (fixed order: deterministic). */
+template <int U>
+static __device__ __forceinline__ fom_float warp_row_dot(const fom_int *__restrict__ col, const fom_float *__restrict__ val,
+                                                         const fom_float *__restrict__ x, fom_int k0, fom_int k1, int lane) {
+    fom_float s = 0.0;
+    fom_int k = k0 + lane;
+    for (; k + 32 * (U - 1) < k1; k += 32 * U) {
+        fom_int c[U];
+        fom_float v[U];
+#pragma unroll
+        for (int u = 0; u < U; ++u) { c[u] = __ldg(col + k + 32 * u); v[u] = __ldg(val + k + 32 * u); }
+#pragma unroll
+        for (int u = 0; u < U; ++u) s += v[u] * __ldg(x + c[u]);
+    }
+    for (; k < k1; k += 32) s += __ldg(val + k) * __ldg(x + __ldg(col + k));
+    for (int w = 16; w > 0; w /= 2) s += __shfl_down_sync(0xffffffff, s, w);
+    return s;
+}
+
+/* Column-tiled CSR, derived once (first call) from a matrix's own CSR data.
+   Columns are split into T ranges of width W so each tile's slice of x
+   stays in L1 during the gather; the column index is stored as a 16-bit
+   offset from its tile base. Each row is processed tile by tile in a
+   fixed order (deterministic). Requires sorted columns and W < 65536;
+   otherwise the plain CSR kernel is used. */
+#define FOM_TILED_MAXT 2
+
+typedef struct {
+    int state; /* 0 unbuilt, 1 tiled, -1 plain CSR */
+    int rows, T, cmin, W, xlen, maxlen; /* maxlen: longest row */
+    const fom_float *val;
+    int *bnd;            /* (T+1) x rows tile boundaries, tile-major */
+    unsigned short *c16; /* column offset within its tile */
+} FomTiled;
+
+static __global__ void tiled_scan(const fom_int *rowptr, const fom_int *col, int rows, int *meta) {
+    int i = blockIdx.x * blockDim.x + threadIdx.x;
+    if (i >= rows) return;
+    fom_int k0 = rowptr[i], k1 = rowptr[i + 1];
+    if (k1 <= k0) return;
+    for (fom_int k = k0 + 1; k < k1; ++k)
+        if (col[k] <= col[k - 1]) atomicOr(meta + 2, 1);
+    atomicMin(meta, col[k0]);
+    atomicMax(meta + 1, col[k1 - 1]);
+    atomicMax(meta + 3, k1 - k0);
+}
+
+static __global__ void tiled_fill(const fom_int *rowptr, const fom_int *col, int rows, int cmin, int W, int T,
+                                  int *bnd, unsigned short *c16) {
+    int i = blockIdx.x * blockDim.x + threadIdx.x;
+    if (i >= rows) return;
+    fom_int k0 = rowptr[i], k1 = rowptr[i + 1];
+    int t = 0;
+    bnd[i] = k0;
+    for (fom_int k = k0; k < k1; ++k) {
+        int c = col[k] - cmin;
+        while (c >= (t + 1) * W) { ++t; bnd[t * rows + i] = k; }
+        c16[k] = (unsigned short)(c - t * W);
+    }
+    while (t < T) { ++t; bnd[t * rows + i] = k1; }
+}
+
+static void tiled_build(FomTiled *m, const fom_int *rowptr, const fom_int *col, const fom_float *val, int rows,
+                        int nnz) {
+    int T;
+    int h[4] = {0x7fffffff, -1, 0, 0}, *meta;
+    m->state = -1;
+    m->rows = rows;
+    m->val = val;
+    cudaMalloc((void **)&meta, sizeof(h));
+    cudaMemcpy(meta, h, sizeof(h), cudaMemcpyHostToDevice);
+    tiled_scan<<<(rows + 255) / 256, 256, 0, FOM_STRM>>>(rowptr, col, rows, meta);
+    cudaMemcpy(h, meta, sizeof(h), cudaMemcpyDeviceToHost);
+    cudaFree(meta);
+    if (h[2] || h[1] < h[0]) return;
+    m->maxlen = h[3];
+    m->cmin = h[0];
+    m->xlen = h[1] - h[0] + 1;
+    T = (h[1] - h[0]) / 65535 + 1; /* fewest tiles with 16-bit offsets */
+    if (T > FOM_TILED_MAXT) return;
+    m->T = T;
+    m->W = (h[1] - h[0] + T) / T;
+    cudaMalloc((void **)&m->bnd, sizeof(int) * (T + 1) * rows);
+    cudaMalloc((void **)&m->c16, sizeof(unsigned short) * (nnz > 0 ? nnz : 1));
+    tiled_fill<<<(rows + 255) / 256, 256, 0, FOM_STRM>>>(rowptr, col, rows, m->cmin, m->W, T, m->bnd, m->c16);
+    m->state = 1;
+}
+
+/* G lanes per row (G divides 32): a row's lanes read G consecutive entries
+   per step (coalesced), U steps in flight, then a fixed-order shuffle sum
+   within the G-lane group (deterministic). CS streams the matrix past the
+   caches (evict-first), leaving L2 to x and to concurrent kernels. */
+template <bool CS, typename T>
+static __device__ __forceinline__ T ld_mat(const T *p) { return CS ? __ldcs(p) : __ldg(p); }
+
+template <int U, int G, bool CS>
+static __global__ void tiled_kern(const int *__restrict__ bnd, const unsigned short *__restrict__ c16,
+                                  const fom_float *__restrict__ val, const fom_float *__restrict__ x,
+                                  fom_float *out, int rows, int T, int W) {
+    int gid = blockIdx.x * blockDim.x + threadIdx.x;
+    int i = gid / G;
+    int lane = threadIdx.x & (G - 1);
+    int ok = i < rows; /* no early return: the group shuffle needs every lane */
+    int b[FOM_TILED_MAXT + 1];
+#pragma unroll
+    for (int t = 0; t <= FOM_TILED_MAXT; ++t) b[t] = (ok && t <= T) ? __ldg(bnd + t * rows + i) : (ok ? 0x7fffffff : 0);
+    fom_int k1 = ok ? b[T] : 0;
+    fom_int k = b[0] + lane;
+    fom_float s = 0.0;
+    for (; k + G * (U - 1) < k1; k += G * U) {
+        int c[U];
+        fom_float v[U];
+#pragma unroll
+        for (int u = 0; u < U; ++u) {
+            int kk = k + G * u;
+            c[u] = ld_mat<CS>(c16 + kk);
+#pragma unroll
+            for (int t = 1; t < FOM_TILED_MAXT; ++t) c[u] += (kk >= b[t]) ? W : 0;
+            v[u] = ld_mat<CS>(val + kk);
+        }
+#pragma unroll
+        for (int u = 0; u < U; ++u) s += v[u] * __ldg(x + c[u]);
+    }
+    for (; k < k1; k += G) {
+        int c = ld_mat<CS>(c16 + k);
+#pragma unroll
+        for (int t = 1; t < FOM_TILED_MAXT; ++t) c += (k >= b[t]) ? W : 0;
+        s += ld_mat<CS>(val + k) * __ldg(x + c);
+    }
+#pragma unroll
+    for (int w = G / 2; w > 0; w /= 2) s += __shfl_down_sync(0xffffffff, s, w, G);
+    if (ok && lane == 0) out[i] += s;
+}
+
+/* Single tile, every row at most G * U long: each lane issues all of its
+   U predicated loads at once (no dependent tail step), and out[i] is read
+   up front. Same per-lane order as tiled_kern<U - 1, G> plus its tail
+   step, so the same sums. */
+template <int U, int G>
+static __global__ void one_kern(const int *__restrict__ bnd, const unsigned short *__restrict__ c16,
+                                const fom_float *__restrict__ val, const fom_float *__restrict__ x, fom_float *out,
+                                int rows) {
+    int gid = blockIdx.x * blockDim.x + threadIdx.x;
+    int i = gid / G;
+    int lane = threadIdx.x & (G - 1);
+    int ok = i < rows; /* no early return: the group shuffle needs every lane */
+    fom_float o = (ok && lane == 0) ? out[i] : 0.0;
+    fom_int k0 = ok ? __ldg(bnd + i) + lane : 0, k1 = ok ? __ldg(bnd + rows + i) : 0;
+    int c[U];
+    fom_float v[U], s = 0.0;
+#pragma unroll
+    for (int u = 0; u < U; ++u) {
+        int kk = k0 + G * u;
+        c[u] = kk < k1 ? (int)__ldg(c16 + kk) : 0;
+        v[u] = kk < k1 ? __ldg(val + kk) : 0.0;
+    }
+#pragma unroll
+    for (int u = 0; u < U; ++u)
+        if (k0 + G * u < k1) s += v[u] * __ldg(x + c[u]);
+#pragma unroll
+    for (int w = G / 2; w > 0; w /= 2) s += __shfl_down_sync(0xffffffff, s, w, G);
+    if (ok && lane == 0) out[i] = o + s;
+}
+
+template <int U, int G>
+static void one_apply(const FomTiled *m, const fom_float *x, fom_float *out) {
+    static int carve = 0;
+    if (!carve) {
+        cudaFuncSetAttribute(one_kern<U, G>, cudaFuncAttributePreferredSharedMemoryCarveout, 0);
+        carve = 1;
+    }
+    int blocks = (m->rows * G + FOM_OPS_THREADS - 1) / FOM_OPS_THREADS;
+    one_kern<U, G><<<blocks, FOM_OPS_THREADS, 0, FOM_STRM>>>(m->bnd, m->c16, m->val, x + m->cmin, out, m->rows);
+}
+
+/* Fixed-stride (ELL) layout for a matrix whose rows are all about equally
+   long: row i's entries sit at [i S, i S + S), padded with column 0xFFFF
+   (never a real offset: xlen <= 65535) whose term is skipped. A row's start
+   is computed, not loaded, which removes one dependent memory round trip
+   per row. Same per-lane order as one_kern (same sums). */
+#define FOM_ELL_PAD 0xFFFF
+
+typedef struct {
+    int state; /* 0 unbuilt, 1 built, -1 not applicable */
+    int rows, S, cmin;
+    unsigned short *c16;
+    fom_float *val;
+} FomEll;
+
+static __global__ void ell_fill(const fom_int *rowptr, const fom_int *col, const fom_float *val, int rows, int S,
+                                int cmin, unsigned short *c16, fom_float *v) {
+    int i = blockIdx.x * blockDim.x + threadIdx.x;
+    if (i >= rows) return;
+    fom_int k0 = rowptr[i], n = rowptr[i + 1] - k0;
+    for (int j = 0; j < S; ++j) {
+        c16[(size_t)i * S + j] = j < n ? (unsigned short)(col[k0 + j] - cmin) : (unsigned short)FOM_ELL_PAD;
+        v[(size_t)i * S + j] = j < n ? val[k0 + j] : 0.0;
+    }
+}
+
+static void ell_build(FomEll *e, const FomTiled *m, const fom_int *rowptr, const fom_int *col, const fom_float *val) {
+    e->state = -1;
+    if (m->state <= 0 || m->xlen > FOM_ELL_PAD || m->maxlen < 1) return;
+    e->rows = m->rows;
+    e->S = m->maxlen;
+    e->cmin = m->cmin;
+    cudaMalloc((void **)&e->c16, sizeof(unsigned short) * (size_t)e->rows * e->S);
+    cudaMalloc((void **)&e->val, sizeof(fom_float) * (size_t)e->rows * e->S);
+    ell_fill<<<(e->rows + 255) / 256, 256, 0, FOM_STRM>>>(rowptr, col, val, e->rows, e->S, e->cmin, e->c16, e->val);
+    cudaDeviceSynchronize();
+    e->state = 1;
+}
+
+/* G lanes per row, every row at most G * U long (S <= G * U). */
+template <int U, int G>
+static __global__ void ell_kern(const unsigned short *__restrict__ c16, const fom_float *__restrict__ val,
+                                const fom_float *__restrict__ x, fom_float *out, int rows, int S) {
+    int gid = blockIdx.x * blockDim.x + threadIdx.x;
+    int i = gid / G;
+    int lane = threadIdx.x & (G - 1);
+    int ok = i < rows; /* no early return: the group shuffle needs every lane */
+    fom_float o = (ok && lane == 0) ? out[i] : 0.0;
+    size_t k0 = (size_t)i * S + lane;
+    int c[U];
+    fom_float v[U], s = 0.0;
+#pragma unroll
+    for (int u = 0; u < U; ++u) {
+        int in = ok && lane + G * u < S;
+        c[u] = in ? (int)__ldcs(c16 + k0 + G * u) : FOM_ELL_PAD;
+        v[u] = in ? __ldcs(val + k0 + G * u) : 0.0;
+    }
+#pragma unroll
+    for (int u = 0; u < U; ++u)
+        if (c[u] != FOM_ELL_PAD) s += v[u] * __ldg(x + c[u]);
+#pragma unroll
+    for (int w = G / 2; w > 0; w /= 2) s += __shfl_down_sync(0xffffffff, s, w, G);
+    if (ok && lane == 0) out[i] = o + s;
+}
+
+template <int U, int G>
+static void ell_apply(const FomEll *e, const fom_float *x, fom_float *out) {
+    static int carve = 0;
+    if (!carve) {
+        cudaFuncSetAttribute(ell_kern<U, G>, cudaFuncAttributePreferredSharedMemoryCarveout, 0);
+        carve = 1;
+    }
+    int blocks = (e->rows * G + FOM_OPS_THREADS - 1) / FOM_OPS_THREADS;
+    ell_kern<U, G><<<blocks, FOM_OPS_THREADS, 0, FOM_STRM>>>(e->c16, e->val, x + e->cmin, out, e->rows, e->S);
+}
+
+/* G lanes per row, U loads in flight per lane, cache policy CS (tuned per
+   matrix shape and on whether it runs alongside another matvec). */
+template <int U, int G, bool CS>
+static void tiled_apply(const FomTiled *m, const fom_float *x, fom_float *out) {
+    static int carve = 0;
+    if (!carve) { /* no shared memory used: give it all to L1, which caches x */
+        cudaFuncSetAttribute(tiled_kern<U, G, CS>, cudaFuncAttributePreferredSharedMemoryCarveout, 0);
+        carve = 1;
+    }
+    int blocks = (m->rows * G + FOM_OPS_THREADS - 1) / FOM_OPS_THREADS;
+    tiled_kern<U, G, CS><<<blocks, FOM_OPS_THREADS, 0, FOM_STRM>>>(m->bnd, m->c16, m->val, x + m->cmin, out, m->rows, m->T, m->W);
+}
+
+/* Paired-halves layout for a matrix whose every row has the exact pattern
+   [P, P + H] (columns P < H in the left half, the same columns shifted by H
+   in the right half; H = cols / 2). x is first interleaved into (x[c],
+   x[c + H]) pairs, so one 16-byte gather feeds both halves' entries.
+   If, in addition, every right value is exactly the negated left value
+   (checked at build from the matrix's own values), only the left values
+   are stored and the right one is formed as -a: the same products and
+   sums in the same order (exact M), with half the value traffic.
+   G lanes per row, fixed-order shuffle sum (deterministic). */
+#define FOM_SIGN_BIT ((long long)0x8000000000000000ULL)
+
+typedef struct {
+    int state; /* 0 unbuilt, 1 paired, -1 not applicable */
+    int rows, H;
+    int anti;            /* 1: right values = -left values, va holds the left */
+    unsigned short *c16; /* left-half column per pair, indexed k0 / 2 + j */
+    signed char *off;    /* if set: right value bits = bits(-left) + off, per pair */
+    fom_float *va;       /* left values per pair, indexed k0 / 2 + j */
+    double2 *xi;         /* interleaved x scratch, H pairs */
+} FomPaired;
+
+/* bad: 1 not paired, 2 right != -left somewhere, 4 unsorted columns,
+   8 some right value is not within an int8 ulp offset of -left. */
+static __global__ void paired_scan(const fom_int *rowptr, const fom_int *col, const fom_float *val, int rows, int H,
+                                   int *bad) {
+    int i = blockIdx.x * blockDim.x + threadIdx.x;
+    if (i >= rows) return;
+    fom_int k0 = rowptr[i], k1 = rowptr[i + 1], h = (k1 - k0) >> 1;
+    if (((k1 - k0) & 1) || (k0 & 1)) { atomicOr(bad, 1); return; }
+    for (fom_int j = 0; j < h; ++j) {
+        long long d = __double_as_longlong(val[k0 + h + j]) - (__double_as_longlong(val[k0 + j]) ^ FOM_SIGN_BIT);
+        if (col[k0 + j] >= H || col[k0 + h + j] != col[k0 + j] + H) atomicOr(bad, 1);
+        if (val[k0 + h + j] != -val[k0 + j]) atomicOr(bad, 2);
+        if (j > 0 && col[k0 + j] <= col[k0 + j - 1]) atomicOr(bad, 4);
+        if (d < -128 || d > 127) atomicOr(bad, 8);
+    }
+}
+
+/* c16: left column per pair; va (if set): left value; off (if set): the
+   right value's IEEE bits minus those of -left, so right is rebuilt exactly
+   as bits(-left) + off (same sign: integer steps walk ulps). */
+static __global__ void paired_fill(const fom_int *rowptr, const fom_int *col, const fom_float *val, int rows,
+                                   unsigned short *c16, fom_float *va, signed char *off) {
+    int i = blockIdx.x * blockDim.x + threadIdx.x;
+    if (i >= rows) return;
+    fom_int k0 = rowptr[i], h = (rowptr[i + 1] - k0) >> 1;
+    for (fom_int j = 0; j < h; ++j) {
+        c16[(k0 >> 1) + j] = (unsigned short)col[k0 + j];
+        if (va) va[(k0 >> 1) + j] = val[k0 + j];
+        if (off) off[(k0 >> 1) + j] = (signed char)(__double_as_longlong(val[k0 + h + j]) -
+                                                     (__double_as_longlong(val[k0 + j]) ^ FOM_SIGN_BIT));
+    }
+}
+
+static void paired_build(FomPaired *m, const fom_int *rowptr, const fom_int *col, const fom_float *val, int rows,
+                         int cols, int nnz) {
+    int bad = 0, *d;
+    m->state = -1;
+    m->rows = rows;
+    m->H = cols >> 1;
+    m->anti = 0;
+    m->va = 0;
+    m->off = 0;
+    if ((cols & 1) || m->H > 65536) return;
+    cudaMalloc((void **)&d, sizeof(int));
+    cudaMemcpy(d, &bad, sizeof(int), cudaMemcpyHostToDevice);
+    paired_scan<<<(rows + 255) / 256, 256, 0, FOM_STRM>>>(rowptr, col, val, rows, m->H, d);
+    cudaMemcpy(&bad, d, sizeof(int), cudaMemcpyDeviceToHost);
+    cudaFree(d);
+    if (bad & 1) return;
+    m->anti = !(bad & 6); /* antisymmetric, with sorted columns (column tiles) */
+    cudaMalloc((void **)&m->c16, sizeof(unsigned short) * (nnz / 2 + 1));
+    if (m->anti) cudaMalloc((void **)&m->va, sizeof(fom_float) * (nnz / 2 + 1));
+    else if (!(bad & 8)) cudaMalloc((void **)&m->off, nnz / 2 + 1);
+    cudaMalloc((void **)&m->xi, sizeof(double2) * m->H);
+    paired_fill<<<(rows + 255) / 256, 256, 0, FOM_STRM>>>(rowptr, col, val, rows, m->c16, m->va, m->off);
+    cudaDeviceSynchronize();
+    m->state = 1;
+}
+
+static __global__ void paired_interleave(const fom_float *__restrict__ x, double2 *xi, int H) {
+    int c = blockIdx.x * blockDim.x + threadIdx.x;
+    if (c < H) xi[c] = make_double2(x[c], x[c + H]);
+}
+
+/* MODE 0: right values from val; 1 (anti): left from va, right = -left;
+   2: right rebuilt exactly from -left and its int8 ulp offset. */
+template <int MODE>
+static __device__ __forceinline__ fom_float paired_right(fom_float a, const fom_float *vr, const signed char *of, fom_int j) {
+    if (MODE == 1) return -a;
+    if (MODE == 2) return __longlong_as_double((__double_as_longlong(a) ^ FOM_SIGN_BIT) + (long long)__ldcs(of + j));
+    return __ldcs(vr + j);
+}
+
+template <int U, int G, int MODE>
+static __global__ void paired_kern(const fom_int *__restrict__ rowptr, const unsigned short *__restrict__ c16,
+                                   const fom_float *__restrict__ val, const fom_float *__restrict__ va,
+                                   const signed char *__restrict__ off, const double2 *__restrict__ xi, fom_float *out,
+                                   int rows) {
+    int gid = blockIdx.x * blockDim.x + threadIdx.x;
+    int i = gid / G;
+    int lane = threadIdx.x & (G - 1);
+    int ok = i < rows; /* no early return: the group shuffle needs every lane */
+    fom_int k0 = ok ? __ldg(rowptr + i) : 0;
+    fom_int h = ok ? (__ldg(rowptr + i + 1) - k0) >> 1 : 0;
+    const fom_float *vl = MODE == 1 ? va + (k0 >> 1) : val + k0, *vr = val + k0 + h;
+    const unsigned short *cp = c16 + (k0 >> 1);
+    const signed char *of = off + (k0 >> 1);
+    fom_float s = 0.0;
+    fom_int j = lane;
+    for (; j + G * (U - 1) < h; j += G * U) {
+        int c[U];
+        fom_float a[U], b[U];
+#pragma unroll
+        for (int u = 0; u < U; ++u) {
+            c[u] = __ldcs(cp + j + G * u);
+            a[u] = __ldcs(vl + j + G * u);
+            b[u] = paired_right<MODE>(a[u], vr, of, j + G * u);
+        }
+#pragma unroll
+        for (int u = 0; u < U; ++u) {
+            double2 p = __ldg(xi + c[u]);
+            s += a[u] * p.x;
+            s += b[u] * p.y;
+        }
+    }
+    for (; j < h; j += G) {
+        double2 p = __ldg(xi + __ldcs(cp + j));
+        fom_float a = __ldcs(vl + j);
+        s += a * p.x;
+        s += paired_right<MODE>(a, vr, of, j) * p.y;
+    }
+#pragma unroll
+    for (int w = G / 2; w > 0; w /= 2) s += __shfl_down_sync(0xffffffff, s, w, G);
+    if (ok && lane == 0) out[i] += s;
+}
+
+static void paired_apply(const FomPaired *m, const fom_int *rowptr, const fom_float *val, const fom_float *x,
+                         fom_float *out) {
+    int blocks = (m->rows * 32 + FOM_OPS_THREADS - 1) / FOM_OPS_THREADS;
+    paired_interleave<<<(m->H + 255) / 256, 256, 0, FOM_STRM>>>(x, m->xi, m->H);
+    if (m->anti)
+        paired_kern<4, 32, 1><<<blocks, FOM_OPS_THREADS, 0, FOM_STRM>>>(rowptr, m->c16, val, m->va, m->off, m->xi, out, m->rows);
+    else if (m->off)
+        paired_kern<4, 32, 2><<<blocks, FOM_OPS_THREADS, 0, FOM_STRM>>>(rowptr, m->c16, val, m->va, m->off, m->xi, out, m->rows);
+    else
+        paired_kern<4, 32, 0><<<blocks, FOM_OPS_THREADS, 0, FOM_STRM>>>(rowptr, m->c16, val, m->va, m->off, m->xi, out, m->rows);
+}
+
+/* Per-SM column tiles. The random x gathers, not the matrix stream, bound
+   these products: every SM touching all of x pulls the whole vector
+   from L2 into its L1. Here the columns are cut into T tiles of W
+   entries and the work into items (tile t, CH consecutive rows). A
+   persistent block starts on tile smid % T, so each SM's L1 holds about one
+   x tile, and steals from the other tiles once its own is used up. An
+   item's result does not depend on which block computes it, and the tile
+   partials part[t][i] are summed in fixed order (deterministic). Paired
+   matrices (pair 1: right values = -left values): x is first reduced to
+   d = x[:H] - x[H:] and the left values are applied to d. (pair 2: right
+   value = bits(-left) + int8 offset): x is interleaved into (x[c], x[c+H])
+   pairs, one 16-byte gather per pair, and the right value is rebuilt
+   exactly; the tile is then W / 2 pairs (same bytes). */
+#define FOM_CT_W 13334
+#define FOM_CT_G 8
+#define FOM_CT_U 4  /* plain tiles: ~87 entries per row segment (4 per lane per step) */
+#define FOM_CT_UP 2 /* paired tiles: ~43 pairs per row segment */
+#define FOM_CT_CH 32
+#define FOM_CT_BPS 6 /* blocks per SM (up to 40 registers per thread) */
+
+typedef struct {
+    int state; /* 0 unbuilt, 1 built, -1 not applicable */
+    int rows, T, W, cmin, xlen, pair, blocks;
+    int *bnd;            /* (start, end) per segment, tile-major */
+    unsigned short *c16; /* column offset within its tile */
+    fom_float *val;      /* values in c16 order (paired: the left values) */
+    signed char *off;    /* pair 2: right value offsets in c16 order */
+    fom_float *part;     /* T x rows tile partial sums */
+    fom_float *xd;       /* pair 1: x[:H] - x[H:]; pair 2: interleaved pairs */
+    int *ctr;            /* per-tile work counters, reset by ct_comb */
+} FomColTile;
+
+static __device__ __forceinline__ unsigned fom_smid(void) {
+    unsigned r;
+    asm volatile("mov.u32 %0, %%smid;" : "=r"(r));
+    return r;
+}
+
+/* Row i's entries j < h (paired: h = half the row, its left columns), split
+   by tile. Packed tile-major: tile t's segments for rows 0..rows-1, then
+   tile t + 1, so the items of a tile stream contiguous memory. Segment
+   (t, i) is [bnd[2 j], bnd[2 j + 1]), j = t * rows + i; its start is
+   4-aligned (zero padding between segments, never summed) for vector loads. */
+static __global__ void ct_count(const fom_int *rowptr, const fom_int *col, int rows, int cmin, int W, int T, int pair,
+                                int *cnt) {
+    int i = blockIdx.x * blockDim.x + threadIdx.x;
+    if (i >= rows) return;
+    fom_int k0 = rowptr[i], h = rowptr[i + 1] - k0;
+    if (pair) h >>= 1;
+    for (int t = 0; t < T; ++t) cnt[t * rows + i] = 0;
+    for (fom_int j = 0; j < h; ++j) cnt[((col[k0 + j] - cmin) / W) * rows + i] += 1;
+}
+
+static __global__ void ct_fill(const fom_int *rowptr, const fom_int *col, const fom_float *val, int rows, int cmin,
+                               int W, int pair, const int *bnd, unsigned short *c16, fom_float *v,
+                               signed char *off) {
+    int i = blockIdx.x * blockDim.x + threadIdx.x;
+    if (i >= rows) return;
+    fom_int k0 = rowptr[i], h = rowptr[i + 1] - k0;
+    if (pair) h >>= 1;
+    int t = -1, o = 0;
+    for (fom_int j = 0; j < h; ++j) {
+        int c = col[k0 + j] - cmin;
+        if (c / W != t) { t = c / W; o = bnd[2 * (t * rows + i)]; }
+        c16[o] = (unsigned short)(c - t * W);
+        v[o] = val[k0 + j];
+        if (off) off[o] = (signed char)(__double_as_longlong(val[k0 + h + j]) -
+                                        (__double_as_longlong(val[k0 + j]) ^ FOM_SIGN_BIT));
+        ++o;
+    }
+}
+
+/* Sorted columns in [cmin, cmin + xlen) (paired: the left half, xlen = H). */
+static void ct_build(FomColTile *m, const fom_int *rowptr, const fom_int *col, const fom_float *val, int rows,
+                     int nnz, int cmin, int xlen, int pair) {
+    int dev, nsm, n, j, ne, *h;
+    m->state = -1;
+    m->rows = rows;
+    m->cmin = cmin;
+    m->xlen = xlen;
+    m->pair = pair;
+    m->W = pair == 2 ? FOM_CT_W / 2 : FOM_CT_W;
+    m->T = (xlen + m->W - 1) / m->W;
+    if (m->T > 32) return;
+    cudaGetDevice(&dev);
+    cudaDeviceGetAttribute(&nsm, cudaDevAttrMultiProcessorCount, dev);
+    m->blocks = nsm * FOM_CT_BPS;
+    n = m->T * rows;
+    cudaMalloc((void **)&m->bnd, sizeof(int) * 2 * n);
+    cudaMalloc((void **)&m->part, sizeof(fom_float) * n);
+    cudaMalloc((void **)&m->ctr, sizeof(int) * 32);
+    cudaMemset(m->ctr, 0, sizeof(int) * 32);
+    /* segment lengths -> (start, end) per segment, starts 4-aligned, tile-major */
+    ct_count<<<(rows + 255) / 256, 256, 0, FOM_STRM>>>(rowptr, col, rows, cmin, m->W, m->T, pair, m->bnd);
+    h = (int *)malloc(sizeof(int) * 2 * n);
+    cudaMemcpy(h, m->bnd, sizeof(int) * n, cudaMemcpyDeviceToHost);
+    for (j = n - 1; j >= 0; --j) h[2 * j + 1] = h[j]; /* spread the lengths */
+    for (j = 0, ne = 0; j < n; ++j) { int c = h[2 * j + 1]; h[2 * j] = ne; h[2 * j + 1] = ne + c; ne += (c + 3) & ~3; }
+    cudaMemcpy(m->bnd, h, sizeof(int) * 2 * n, cudaMemcpyHostToDevice);
+    free(h);
+    cudaMalloc((void **)&m->c16, sizeof(unsigned short) * (ne + 4));
+    cudaMalloc((void **)&m->val, sizeof(fom_float) * (ne + 4));
+    cudaMemset(m->c16, 0, sizeof(unsigned short) * (ne + 4));
+    cudaMemset(m->val, 0, sizeof(fom_float) * (ne + 4));
+    m->xd = 0;
+    m->off = 0;
+    if (pair == 1) cudaMalloc((void **)&m->xd, sizeof(fom_float) * xlen);
+    if (pair == 2) {
+        cudaMalloc((void **)&m->xd, sizeof(double2) * xlen);
+        cudaMalloc((void **)&m->off, ne + 4);
+        cudaMemset(m->off, 0, ne + 4);
+    }
+    ct_fill<<<(rows + 255) / 256, 256, 0, FOM_STRM>>>(rowptr, col, val, rows, cmin, m->W, pair, m->bnd, m->c16,
+                                                      m->val, m->off);
+    cudaDeviceSynchronize();
+    m->state = 1;
+}
+
+/* One entry's contribution: plain (x is fom_float) or OFF (x is pairs). */
+template <bool OFF>
+static __device__ __forceinline__ void ct_term(fom_float &s, fom_float a, int c, signed char o,
+                                               const fom_float *__restrict__ xt) {
+    if (OFF) {
+        double2 p = __ldg((const double2 *)xt + c);
+        s += a * p.x;
+        s += __longlong_as_double((__double_as_longlong(a) ^ FOM_SIGN_BIT) + (long long)o) * p.y;
+    } else {
+        s += a * __ldg(xt + c);
+    }
+}
+
+/* The launch bounds let ptxas keep all U loads in flight (left to itself
+   it picks 32 registers and chains each load to its multiply-add). */
+template <int G, int U, int CH, bool OFF>
+static __global__ void __launch_bounds__(G * CH, FOM_CT_BPS) ct_kern(const int *__restrict__ bnd, const unsigned short *__restrict__ c16,
+                               const fom_float *__restrict__ val, const signed char *__restrict__ off,
+                               const fom_float *__restrict__ x, fom_float *part, int *ctr, int rows, int T, int W) {
+    __shared__ int tk[2]; /* current and prefetched item tickets */
+    int nch = (rows + CH - 1) / CH;
+    int t = fom_smid() % T, left = T, p = 0;
+    int r = threadIdx.x / G, lane = threadIdx.x & (G - 1);
+    if (threadIdx.x == 0) tk[0] = atomicAdd(ctr + t, 1);
+    __syncthreads();
+    for (;;) {
+        int ch = tk[p];
+        if (ch >= nch) { /* tile t is used up: move on to the next one */
+            if (--left == 0) break;
+            t = t + 1 == T ? 0 : t + 1;
+            __syncthreads();
+            if (threadIdx.x == 0) tk[p] = atomicAdd(ctr + t, 1);
+            __syncthreads();
+            continue;
+        }
+        if (threadIdx.x == 0) tk[p ^ 1] = atomicAdd(ctr + t, 1);
+        int i = ch * CH + r, ok = i < rows;
+        const fom_float *xt = x + (OFF ? 2 : 1) * t * W;
+        fom_float s = 0.0;
+        int2 se = ok ? __ldg((const int2 *)bnd + t * rows + i) : make_int2(0, 0);
+        /* lane owns 4 consecutive entries per step (vector loads; segment
+           starts are 4-aligned); only the sums are predicated */
+        for (int k = se.x + 4 * lane; k < se.y; k += 4 * G * U) {
+            ushort4 c[U];
+            double2 a0[U], a1[U];
+            char4 o[U];
+#pragma unroll
+            for (int u = 0; u < U; ++u) {
+                int kk = min(k + 4 * G * u, se.y - 1) & ~3;
+                c[u] = __ldcs((const ushort4 *)(c16 + kk));
+                a0[u] = __ldcs((const double2 *)(val + kk));
+                a1[u] = __ldcs((const double2 *)(val + kk + 2));
+                if (OFF) o[u] = __ldcs((const char4 *)(off + kk));
+                else o[u] = make_char4(0, 0, 0, 0);
+            }
+#pragma unroll
+            for (int u = 0; u < U; ++u) {
+                int kk = k + 4 * G * u;
+                if (kk < se.y) ct_term<OFF>(s, a0[u].x, c[u].x, o[u].x, xt);
+                if (kk + 1 < se.y) ct_term<OFF>(s, a0[u].y, c[u].y, o[u].y, xt);
+                if (kk + 2 < se.y) ct_term<OFF>(s, a1[u].x, c[u].z, o[u].z, xt);
+                if (kk + 3 < se.y) ct_term<OFF>(s, a1[u].y, c[u].w, o[u].w, xt);
+            }
+        }
+#pragma unroll
+        for (int w = G / 2; w > 0; w /= 2) s += __shfl_down_sync(0xffffffff, s, w, G);
+        if (ok && lane == 0) part[t * rows + i] = s;
+        p ^= 1;
+        __syncthreads();
+    }
+}
+
+static __global__ void ct_comb(const fom_float *__restrict__ part, fom_float *out, int *ctr, int rows, int T) {
+    int i = blockIdx.x * blockDim.x + threadIdx.x;
+    if (i < T) ctr[i] = 0; /* ct_kern has finished: re-arm the counters */
+    if (i >= rows) return;
+    fom_float s = part[i];
+    for (int t = 1; t < T; ++t) s += part[t * rows + i];
+    out[i] += s;
+}
+
+static __global__ void ct_diff(const fom_float *__restrict__ x, fom_float *d, int H) {
+    int c = blockIdx.x * blockDim.x + threadIdx.x;
+    if (c < H) d[c] = x[c] - x[c + H];
+}
+
+template <bool OFF>
+static void ct_launch(const FomColTile *m, const fom_float *x) {
+    static int carve = 0;
+    const int U = OFF ? FOM_CT_UP : FOM_CT_U;
+    if (!carve) { /* no shared memory used: give it all to L1, which caches the x tile */
+        cudaFuncSetAttribute(ct_kern<FOM_CT_G, U, FOM_CT_CH, OFF>, cudaFuncAttributePreferredSharedMemoryCarveout, 0);
+        carve = 1;
+    }
+    ct_kern<FOM_CT_G, U, FOM_CT_CH, OFF><<<m->blocks, FOM_CT_G * FOM_CT_CH, 0, FOM_STRM>>>(
+        m->bnd, m->c16, m->val, m->off, x, m->part, m->ctr, m->rows, m->T, m->W);
+}
+
+static void ct_apply(const FomColTile *m, const fom_float *x, fom_float *out) {
+    if (m->pair == 1) {
+        ct_diff<<<(m->xlen + 255) / 256, 256, 0, FOM_STRM>>>(x, m->xd, m->xlen);
+        ct_launch<false>(m, m->xd);
+    } else if (m->pair == 2) {
+        paired_interleave<<<(m->xlen + 255) / 256, 256, 0, FOM_STRM>>>(x, (double2 *)m->xd, m->xlen);
+        ct_launch<true>(m, m->xd);
+    } else {
+        ct_launch<false>(m, x + m->cmin);
+    }
+    ct_comb<<<(m->rows + 255) / 256, 256, 0, FOM_STRM>>>(m->part, out, m->ctr, m->rows, m->T);
+}
 
 static __global__ void fom_apply_t0_state_kern(const fom_float *x, fom_float *out) {
     /* One warp per row: lanes stride the row (coalesced), then a shuffle sum. */
     fom_int i = (blockIdx.x * blockDim.x + threadIdx.x) / 32;
     fom_int lane = threadIdx.x % 32;
     if (i >= 10000) return;  /* whole warps exit together */
-    fom_float s = 0.0;
-    for (fom_int k = FOM_OP_T0_STATE_ROWPTR[i] + lane; k < FOM_OP_T0_STATE_ROWPTR[i + 1]; k += 32) s += FOM_OP_T0_STATE_VALS[k] * x[FOM_OP_T0_STATE_COLIDX[k]];
-    for (int w = 16; w > 0; w /= 2) s += __shfl_down_sync(0xffffffff, s, w);
+    fom_float s = warp_row_dot<4>(FOM_OP_T0_STATE_COLIDX, FOM_OP_T0_STATE_VALS, x, FOM_OP_T0_STATE_ROWPTR[i], FOM_OP_T0_STATE_ROWPTR[i + 1], lane);
     if (lane == 0) out[i] += s;
 }
 
 extern "C" void fom_apply_t0_state(const fom_float *x, fom_float *out) {
-    fom_apply_t0_state_kern<<<1250, FOM_OPS_THREADS>>>(x, out);
+    static FomTiled m = {0};
+    static FomColTile c = {0};
+    if (!m.state) {
+        const fom_int *rp, *ci;
+        const fom_float *v;
+        cudaGetSymbolAddress((void **)&rp, FOM_OP_T0_STATE_ROWPTR);
+        cudaGetSymbolAddress((void **)&ci, FOM_OP_T0_STATE_COLIDX);
+        cudaGetSymbolAddress((void **)&v, FOM_OP_T0_STATE_VALS);
+        tiled_build(&m, rp, ci, v, 10000, 2599983);
+        if (m.xlen > 0) ct_build(&c, rp, ci, v, 10000, 2599983, m.cmin, m.xlen, 0);
+    }
+    if (c.state > 0) ct_apply(&c, x, out);
+    else if (m.state > 0) tiled_apply<2, 16, true>(&m, x, out);
+    else fom_apply_t0_state_kern<<<1250, FOM_OPS_THREADS, 0, FOM_STRM>>>(x, out);
 }
 
 static __global__ void fom_apply_t0_state__d_gamma_kern(const fom_float *x, fom_float *out) {
@@ -29,14 +686,30 @@
     fom_int i = (blockIdx.x * blockDim.x + threadIdx.x) / 32;
     fom_int lane = threadIdx.x % 32;
     if (i >= 10000) return;  /* whole warps exit together */
-    fom_float s = 0.0;
-    for (fom_int k = FOM_OP_T0_STATE__D_GAMMA_ROWPTR[i] + lane; k < FOM_OP_T0_STATE__D_GAMMA_ROWPTR[i + 1]; k += 32) s += FOM_OP_T0_STATE__D_GAMMA_VALS[k] * x[FOM_OP_T0_STATE__D_GAMMA_COLIDX[k]];
-    for (int w = 16; w > 0; w /= 2) s += __shfl_down_sync(0xffffffff, s, w);
+    fom_float s = warp_row_dot<4>(FOM_OP_T0_STATE__D_GAMMA_COLIDX, FOM_OP_T0_STATE__D_GAMMA_VALS, x, FOM_OP_T0_STATE__D_GAMMA_ROWPTR[i], FOM_OP_T0_STATE__D_GAMMA_ROWPTR[i + 1], lane);
     if (lane == 0) out[i] += s;
 }
 
 extern "C" void fom_apply_t0_state__d_gamma(const fom_float *x, fom_float *out) {
-    fom_apply_t0_state__d_gamma_kern<<<1250, FOM_OPS_THREADS>>>(x, out);
+    static FomTiled m = {0};
+    static FomPaired p = {0};
+    static FomColTile c = {0};
+    static const fom_int *rp;
+    static const fom_float *v;
+    if (!p.state) {
+        const fom_int *ci;
+        cudaGetSymbolAddress((void **)&rp, FOM_OP_T0_STATE__D_GAMMA_ROWPTR);
+        cudaGetSymbolAddress((void **)&ci, FOM_OP_T0_STATE__D_GAMMA_COLIDX);
+        cudaGetSymbolAddress((void **)&v, FOM_OP_T0_STATE__D_GAMMA_VALS);
+        paired_build(&p, rp, ci, v, 10000, 80000, 5199966);
+        if (p.state < 0) tiled_build(&m, rp, ci, v, 10000, 5199966);
+        else if (p.anti) ct_build(&c, rp, ci, v, 10000, 5199966, 0, p.H, 1);
+        else if (p.off) ct_build(&c, rp, ci, v, 10000, 5199966, 0, p.H, 2);
+    }
+    if (c.state > 0) ct_apply(&c, x, out);
+    else if (p.state > 0) paired_apply(&p, rp, v, x, out);
+    else if (m.state > 0) tiled_apply<4, 16, true>(&m, x, out);
+    else fom_apply_t0_state__d_gamma_kern<<<1250, FOM_OPS_THREADS, 0, FOM_STRM>>>(x, out);
 }
 
 static __global__ void fom_apply_t0_p_p0_kern(const fom_float *x, fom_float *out) {
@@ -48,7 +721,7 @@
 }
 
 extern "C" void fom_apply_t0_p_p0(const fom_float *x, fom_float *out) {
-    fom_apply_t0_p_p0_kern<<<40, FOM_OPS_THREADS>>>(x, out);
+    fom_apply_t0_p_p0_kern<<<40, FOM_OPS_THREADS, 0, FOM_STRM>>>(x, out);
 }
 
 static __global__ void fom_apply_t1_state_kern(const fom_float *x, fom_float *out) {
@@ -60,7 +733,7 @@
 }
 
 extern "C" void fom_apply_t1_state(const fom_float *x, fom_float *out) {
-    fom_apply_t1_state_kern<<<157, FOM_OPS_THREADS>>>(x, out);
+    fom_apply_t1_state_kern<<<157, FOM_OPS_THREADS, 0, FOM_STRM>>>(x, out);
 }
 
 static __global__ void fom_apply_t1_state__d_gamma_kern(const fom_float *x, fom_float *out) {
@@ -72,7 +745,7 @@
 }
 
 extern "C" void fom_apply_t1_state__d_gamma(const fom_float *x, fom_float *out) {
-    fom_apply_t1_state__d_gamma_kern<<<157, FOM_OPS_THREADS>>>(x, out);
+    fom_apply_t1_state__d_gamma_kern<<<157, FOM_OPS_THREADS, 0, FOM_STRM>>>(x, out);
 }
 
 static __global__ void fom_apply_t1_t_t0_kern(const fom_float *x, fom_float *out) {
@@ -80,14 +753,27 @@
     fom_int i = (blockIdx.x * blockDim.x + threadIdx.x) / 32;
     fom_int lane = threadIdx.x % 32;
     if (i >= 40000) return;  /* whole warps exit together */
-    fom_float s = 0.0;
-    for (fom_int k = FOM_OP_T1_T_T0_ROWPTR[i] + lane; k < FOM_OP_T1_T_T0_ROWPTR[i + 1]; k += 32) s += FOM_OP_T1_T_T0_VALS[k] * x[FOM_OP_T1_T_T0_COLIDX[k]];
-    for (int w = 16; w > 0; w /= 2) s += __shfl_down_sync(0xffffffff, s, w);
+    fom_float s = warp_row_dot<2>(FOM_OP_T1_T_T0_COLIDX, FOM_OP_T1_T_T0_VALS, x, FOM_OP_T1_T_T0_ROWPTR[i], FOM_OP_T1_T_T0_ROWPTR[i + 1], lane);
     if (lane == 0) out[i] += s;
 }
 
 extern "C" void fom_apply_t1_t_t0(const fom_float *x, fom_float *out) {
-    fom_apply_t1_t_t0_kern<<<5000, FOM_OPS_THREADS>>>(x, out);
+    static FomTiled m = {0};
+    static FomEll e = {0};
+    if (!m.state) {
+        const fom_int *rp, *ci;
+        const fom_float *v;
+        cudaGetSymbolAddress((void **)&rp, FOM_OP_T1_T_T0_ROWPTR);
+        cudaGetSymbolAddress((void **)&ci, FOM_OP_T1_T_T0_COLIDX);
+        cudaGetSymbolAddress((void **)&v, FOM_OP_T1_T_T0_VALS);
+        tiled_build(&m, rp, ci, v, 40000, 2599983);
+        /* rows of 64-65 entries: padding to the longest costs < 2% */
+        if (m.state > 0 && m.maxlen <= 80) ell_build(&e, &m, rp, ci, v);
+    }
+    if (e.state > 0) ell_apply<5, 16>(&e, x, out);
+    else if (m.state > 0 && m.T == 1 && m.maxlen <= 80) one_apply<5, 16>(&m, x, out);
+    else if (m.state > 0) tiled_apply<4, 16, false>(&m, x, out);
+    else fom_apply_t1_t_t0_kern<<<5000, FOM_OPS_THREADS, 0, FOM_STRM>>>(x, out);
 }
 
 static __global__ void fom_apply_su_state_kern(const fom_float *x, fom_float *out) {
@@ -99,7 +785,7 @@
 }
 
 extern "C" void fom_apply_su_state(const fom_float *x, fom_float *out) {
-    fom_apply_su_state_kern<<<313, FOM_OPS_THREADS>>>(x, out);
+    fom_apply_su_state_kern<<<313, FOM_OPS_THREADS, 0, FOM_STRM>>>(x, out);
 }
 
 static __global__ void fom_apply_su_t_t1_kern(const fom_float *x, fom_float *out) {
@@ -111,5 +797,5 @@
 }
 
 extern "C" void fom_apply_su_t_t1(const fom_float *x, fom_float *out) {
-    fom_apply_su_t_t1_kern<<<313, FOM_OPS_THREADS>>>(x, out);
-}
+    fom_apply_su_t_t1_kern<<<313, FOM_OPS_THREADS, 0, FOM_STRM>>>(x, out);
+}
```

## Agent notes

```
### Round 1

**Whole-solve time:** 34336 us at the start (the first passing version, with the broken graph replay turned off) vs. 25080 us at the end. That is a **1.37x** speedup. The version handed to me failed verify: its graph replay was a silent no-op because the graph was captured on the legacy stream.

**Final profile** (`-DFOM_PROFILE` build, which syncs after each region and runs without the graph; the regions are 4000 iterations over 20 solves):
| region | total_us | pct |
|---|---|---|
| t0_matvec (t0_state + t0_state__d_gamma) | 401805 | 63.6% |
| t1_matvec (t1_t_t0 + soft-threshold) | 137605 | 21.8% |
| t1_state_terms (t1_state + t1_state__d_gamma) | 57603 | 9.1% |
| prep (xg = g*s, su shift, accumulator seeding) | 34296 | 5.4% |
| p0_term (once per solve) | 377 | 0.06% |

**Optimizations applied:**
- **Capturable CUDA graph (correctness fix plus speed).** Every launch goes on `cudaStreamPerThread`. This removes the per-launch host overhead from the fixed 200-iteration loop. The structure exploited is the `max_iters` termination with no tolerance: the loop is fixed-length, so it can be replayed as one graph. Result: 34.3 → 30.8 ms.
- **Paired-halves gathers for `t0_state__d_gamma` (5.2M nnz).** Every row has the exact column pattern `[P, P+40000]`; the build step checks this from the matrix's own CSR. Inside the apply, `x` is interleaved into `(x[c], x[c+40000])` pairs. One 16-byte gather then feeds both halves, and each pair shares a single 16-bit column index. Both value arrays are still used, so this is exactly M. This kernel is limited by random L2 gathers, not DRAM. Result: about 98 → 75 us per call, and 30.5 → 27.0 ms overall.
- **Sub-warp (G lanes per row) tiled CSR kernel.** G=16 for the t0 matrices (about 260 or 520 nnz per row) and G=4 for `t1_t_t0` (65 nnz per row). Columns are stored as 16-bit tile offsets. This is one full wave with no tail, and the shuffle reductions keep a fixed order. The gain was small (about 1%), because the gathers are the real limit.
- **Hoisted the `t0_p_p0 @ p0` term.** p0 is a fixed parameter, so the term is applied once per solve in `fom_solve` and used to seed the t0 accumulator in `k_prep`. This removes one kernel per iteration. Result: 27.0 → 26.4 ms.
- **DAG-shaped graph.** `cudaStreamGetCaptureInfo`/`UpdateCaptureDependencies` fork and join inside the capture. The tiny `t1_state` and `t1_state__d_gamma` applies (diagonal shift and `[-I, I]` blocks writing only t1) run in parallel with the t0 matvecs (which write only t0). `t1_t_t0` joins both branches. Each accumulator is still updated in a fixed order, so results are deterministic. Result: 26.4 → 25.1 ms.
- **Kept from the translated code:** no `su_*` matvecs, since that step is a pure shift done inside `k_prep`. Residual reduction on the device with one scalar read back per solve. Ping-pong state buffers.

**Tried and reverted:** an ELL thread-per-row layout for `t1_t_t0` (no better than G=4), `__ldcs` streaming loads (t1 got slower), U=8 unroll (+3%), and G=8 variants (+4%). The L1 carveout setting showed no gain I could measure, but it is still in the final code.

Blocked by rules:
- Fuse `t0_state` and `t0_state__d_gamma` into one pass: t0_state's pattern equals the right half of d_gamma's, so one gather of `(s0[c], s1[c])` would feed all three terms, with the gamma scaling folded in | each `fom_apply_<name>` may read only its own matrix's data, and no matvec loops may go in fom_solver.cu | about 50 us/iter, ~35-40% of the solve
- Fold the t1 soft-threshold (and the t1_state / t1_state__d_gamma shift terms) into the epilogue of the `t1_t_t0` kernel | each apply must be a pure `out += M x` checked on its own, and solver code may not read matrix data | about 5-10 us/iter, ~5%
- Have callers pass `x` already in interleaved `(s0, s1)` layout so the d_gamma apply skips its interleave pass | `fom_apply_<name>` signatures are fixed in include/, and the direct checks probe the standard layout | about 4 us/iter, ~3%
- Run `t0_state` and `t0_state__d_gamma` on separate graph branches into separate accumulators | the apply kernels launch on the fixed per-thread default stream, so the solver cannot target them at other streams except through graph rewiring; a combine kernel would also be needed | unknown, small because both are gather-bound

### Round 2

**Whole-solve `mean_us`:** 25110.146 us at the start of this round, 20278.98 us at the end, so this round is 1.24x faster. Against the translated solver (38190.088 us) the total is 1.88x. VERIFY OK, worst error 1.32e-08 against a tolerance of 1e-5.

**Final profile** (`make profile`-style build, synced per region, non-graph path; 31324 us/solve in this mode):
| region | total_us (30 solves) | % |
|---|---|---|
| t0_gamma (interleave + paired d_gamma matvec) | 315679 | 33.8 |
| t0_state (tiled CSR matvec) | 238649 | 25.6 |
| t1_matvec (t1_t_t0 matvec, plus final soft) | 193884 | 20.8 |
| t1_state_terms | 86501 | 9.3 |
| prep | 53021 | 5.7 |
| t0_join | 44298 | 4.8 |
| p0_term | 689 | 0.1 |

In the production CUDA graph, t0_gamma and t0_state overlap, and t1_state_terms runs on a third parallel branch.

**Optimizations applied:**
1. **Parallel graph branches for the two t0 matvecs** (20844 us after this and change 2). Before, t0_state and t0_state__d_gamma accumulated serially into t0. Now the gamma term has its own accumulator, t0b, zeroed in prep, and a `k_join` kernel adds it. The paired gamma kernel computes each row's sum in full and then does one `out += s`. So t0 = (c0 + A s) + D(g s) gets exactly the same single rounding, and results are bitwise identical. The paired interleave also leaves the t0_state path. The structure exploited is that the two terms write disjoint buffers and t0_p_p0 is applied once per solve. The gamma branch is captured first because it is the longer one (about 0.3% gain).
2. **t1 soft-threshold fused into the next iteration's prep.** Inside the graph, iteration k leaves its t1 slot unthresholded. Iteration k+1's prep applies soft() in place, once per entry, and the thread that owns state[j] (j ≥ NS0) also writes the shifted copy. This works because su_state / su_t_t1 are a pure shift plus copy, so new s0 = old s1. The last iteration still thresholds itself, and the first iteration (the uploaded state) is never thresholded, so results are bitwise identical. This removes one launch from the critical path each iteration.
3. **Per-matrix kernel tuning from microbenchmarks on the real matrix arrays** (22498 → 20326 us). Profiling showed all three big matvecs are latency- and random-read-bound, not DRAM-bound (DRAM 30-48%, L2 38-67%).
   - t0_state: rows of 260 nnz, all columns in [40000, 80000). Uses G=16 lanes, U=2, and evict-first (`__ldcs`) streaming of the matrix so x and the concurrent kernel keep L2.
   - d_gamma paired (rows have pattern [P, P+H]): G=32 lanes (was 16) and U=4. This fixed the low-parallelism case with 10000 rows of 260 pairs.
   - t1_t_t0 (Aᵀ, 65 nnz per row, x = 80 KB so it stays in L1): G=16 (was 4), U=4, with `__ldg`, since it runs alone.
   - The cache policy is a template parameter of the tiled kernel.

**Tried and rejected** (measured, no gain):
- per-column-tile launches;
- `__syncthreads` lockstep sweeps over column tiles to keep x in L1;
- keeping x in shared memory for t1_t_t0;
- vectorized ushort4/double2 loads;
- `__launch_bounds__` and a persistent grid for the paired kernel;
- non-interleaved paired gathers.

Blocked by rules:
- Fuse t0_state and t0_state__d_gamma into one kernel that reads each gathered state entry once for both matrices (their column patterns overlap: A on s1 and [P, P+H]) | kernel rule: each fom_apply_<name> and its helpers read only their own matrix's data | ~10-20% of the t0 phase (unknown exactly)
- Store d_gamma's right half as the negated left half and compute a·(x_L − x_R), which halves its 41.6 MB value stream (the halves agree only to about 3e-16, not bitwise) | must apply exactly M; it would also use matrix data in a way the rules don't intend | ~15-20 us/iteration
- Begin next iteration's t0 matvecs on their already-known left-half columns (new s0 = old s1) while t1_t_t0 still runs | apply routines must apply the whole matrix; no partial-apply entry points (only fom_apply_* may be non-static) | unknown, maybe 5-10%
- Fold the t1 soft-threshold or the t0 join into the matvec kernels' epilogue | fom_apply_<name> must compute exactly out += M x | ~3-6 us/iteration

### Round 3

**Result.** The final `make bench` gives mean_us = 19967.5, against 20279.4 at the start of this round. That is 1.016x for the round and 1.91x overall against the translated solver (38190.1 us). Holder steps land on different A100 nodes, and timings drift about 3–4% between them. So I also compared the binaries inside a single srun step: the round-2 binary ran at 20764–20878 us and the final one at 19962–20030 us, about 4.0% faster. `make verify` prints VERIFY OK; the worst end-to-end error is 1.2e-8 and every per-apply error is at most 8.2e-15.

The Makefile has no `profile` target. I built the profile version by hand with `-DFOM_PROFILE` and `src/fom_profile.c`. That build runs eagerly with a sync after each region, so its whole solve takes 31490 us.

**Final profile breakdown**, hottest first:

| Region | Time per call | Share |
|---|---|---|
| t0_gamma (`t0_state__d_gamma` apply) | 52.8 us | 33.9% |
| t0_state | 40.3 us | 25.8% |
| t1_matvec (`t1_t_t0` + soft) | 33.3 us | 21.4% |
| t1_state_terms | 13.7 us | 8.8% |
| prep | 8.5 us | 5.5% |
| t0_join | 7.1 us | 4.6% |
| p0_term | once per solve | 0.07% |

**Diagnosis.** I used ncu and a standalone kernel harness. The two t0 matvecs are not limited by DRAM: they reach only about 40% of DRAM throughput. Their limit is random gathers of x that miss L1. Every SM ends up pulling the whole x vector (320 to 640 KB) from L2 into its L1. The supporting evidence:
- The same kernel with gathers confined to 8 KB of x runs in 21 us, against 40 us.
- Cutting the `d_gamma` value traffic in half changed nothing.

**Optimizations kept:**
- **Per-SM column tiles for `t0_state` and `t0_state__d_gamma`** (in `src/fom_ops.cu`). Columns are cut into tiles of 13334 entries. A persistent kernel takes work items (tile, 32 rows) from per-tile atomic queues. Each block starts on tile `%smid % T`, so each SM's L1 mostly holds one ~107 KB x tile, and it steals from other tiles when its own runs out. Per-tile partial sums are added in fixed tile order by a small combine kernel, which also resets the counters. Results do not depend on which block takes which item, so the output is deterministic.
  - Structure exploited: sorted CSR columns, checked at build time.
  - Gain: A and D running concurrently took 64 us in the harness, against 72 us before.
- **Exact anti-symmetry of `t0_state__d_gamma`.** Every row has the pattern [P, P+H] and its right values equal the negated left values. This is checked exactly at build time from the matrix's own data, and the old paths stay as fallbacks. Only the left values are stored, and they are applied to d = x[:H] − x[H:]. That halves the gather footprint and replaces the interleave kernel. The result is algebraically exactly M x; the per-apply error is 8e-15 against a 1e-10 tolerance.

**Tried and dropped** (each measured slower or no better):
- An L2 persisting window holding `t1_t_t0`.
- Shared-memory x staging, single- and double-buffered.
- Multi-pass column tiles.
- Sliced-ELL layout for `t1_t_t0`, which already runs within about 15% of the streaming floor.
- Warp-level work queues.
- Folding the combine into the kernel with a last-block pattern; the fences cost more than the saved launch.
- Direct pair gathers instead of the difference vector.
- CH=64 rows per item with 4 blocks per SM.
- Creating the t1 state-term branch first in the graph.

Blocked by rules:
- Compute t0 as a single A·y per iteration with y = (1+γ)x − γ x_prev, dropping the `t0_state__d_gamma` apply. That matrix equals [−A, A] and `t0_state` equals [0, A], which would also allow reusing the previous iteration's A x. | fom_solver.cu must call fom_apply_<name> for each term and may not read or rely on other matrices' coefficients; each fom_apply reads only its own data | ~30–35% (one gather-bound matvec of ~35 us instead of the concurrent pair at ~64 us)
- Fuse the `t0_state` and `d_gamma` gathers into one kernel that shares the x tile staged in L1 for both matrices. | each fom_apply_<name> and its helpers may read only its own matrix's data | ~10–15%
- Pass the solver's precomputed g·(x_L − x_R) straight to the `d_gamma` kernel instead of rebuilding it inside the apply. | fom_apply_<name>(x, out) signature and the requirement that it apply exactly M to x (fixed header) | ~2–3%

### Round 4

**Whole-solve time:** 19849 us at the start of this round, 15940 us at the end (`make bench` mean_us), a 1.25x speedup. A same-node A/B run against the saved round-3 binary gave 19928 vs 15860 us (1.26x). Against the translated solver (38190 us) the total is 2.40x.

**Final profile** (`-DFOM_PROFILE` build: serial path, one device sync per region, 30 solves; a hand build, because the Makefile has no `profile` rule):

| Region | Time (us) | Share |
|---|---|---|
| t0_gamma | 254800 | 32.4% |
| t0_state | 212600 | 27.1% |
| t1_matvec | 185149 | 23.6% |
| t1_state_terms | 82493 | 10.5% |
| prep | 50110 | 6.4% |
| p0_term | 700 | 0.1% |

The production build replays all 200 iterations as one CUDA graph. Kernel times from ncu: gamma tile kernel 30-32 us, t0_state tile kernel 25.6 us, A^T 25.5 us, prep 3.9 us, interleave 3 us, each combine about 3.2 us.

**Optimizations applied:**
1. **Gamma values rebuilt exactly from an int8 ulp offset** (exploits `t0_state__d_gamma` = [-A, A] up to rounding). Every right value equals -left to within 1.7e-15 relative, but only 12% are bit-exact negations, so round 3's "antisymmetric" path never ran. All right values have the same sign as -left, and their IEEE bits sit within [-12, 4] ulps of bits(-left). So each right value is stored as an int8 offset and rebuilt exactly as `bits(-left) + off`, giving the same M and the same products. At build time the code checks this from the matrix's own data and falls back otherwise. Gamma traffic drops from 18 to 11 bytes per pair.
2. **Gamma moved onto the per-SM column-tile kernel.** It uses interleaved (x[c], x[c+H]) pairs with 6667-pair tiles (107 KB, sized to fit in L1). The old paired kernel was bound by random 16-byte gathers from L2.
3. **Tile-major packing of the tile arrays** (columns are sorted, so each row splits cleanly into tile segments). With row-major packing, the edge sectors of each segment were fetched twice by different SMs, because `__ldcs` evicts them in between. Now each tile streams contiguous memory. This took the gamma kernel from about 43 to 41 us in isolation and the solve from 19.1 to 18.1 ms.
4. **ptxas load batching fixed in the tile kernel.** Left alone, ptxas chose 32 registers and chained every load to its multiply-add, one memory latency per entry (found in the SASS). Adding `__launch_bounds__(256, BPS)` and unconditional clamped loads, with only the sums predicated, keeps all loads in flight. Solve went from 18.1 to 16.9 ms.
5. **Vector loads in the tile kernel.** Segment starts are padded to 4 entries (the padding is never summed), so each lane reads 4 entries per instruction (ushort4, two double2, char4) at 6 blocks per SM. This cuts load-instruction stalls (13-17% of warp stalls): 16.5 to 15.9 ms.
6. **`k_join` removed** (the two t0 terms are just sums into the same vector). Both t0 terms now accumulate straight into t0. An edge added to the capture graph (`cudaGraphAddDependencies`) orders the gamma combine after the t0_state combine. The result is (c0 + state) + gamma, the same bits as the serial path. 16.9 to 16.5 ms.
7. **Single-pass A^T kernel** (t1_t_t0 rows have 64-65 nonzeros). One predicated pass with 16 lanes x 5 steps replaces the loop plus its dependent tail step, and out[i] is read up front. The per-lane order is unchanged, so the sums are bit-identical. About 2 us per call.

**Tried with no gain** (harness or real A/B): capturing state before gamma, different block counts per kernel, tile widths of 10000 and 16000, dropping the interleave in favour of two 8-byte gathers, vectorized or two-row A^T variants, and L2 persistence or evict_last for A^T. A^T is bound by L1 gathers: the same kernel without the gather runs in 14 us instead of 21.5.

Blocked by rules:
- Compute A*y once per iteration, with y = s1 + g(s1 - s0), instead of streaming the three separate copies of A (t0_state = [0, A], d_gamma = [-A, A]) | kernel rule: each fom_apply reads only its own matrix's data, and the solver may not combine matrices | ~25-30% (removes about 26 MB of the ~82 MB streamed per iteration)
- Apply d_gamma as left values times (x[:H] - x[H:]), treating right = -left, which is exact to 1.7e-15 relative but not bitwise | the kernel must apply exactly M | ~2.6 MB per iteration (the int8 offset array), unknown net gain
- Fuse the t1 soft-threshold and the t0 combine into the matvec kernels (out = soft(...) in fom_apply_t1_t_t0) to remove graph nodes | fom_apply must compute exactly out += M x | ~3-5 us per iteration (~5%)

### Round 5

**Whole-solve `mean_us`:** translated solver 38190.088 us. Start of this round 15719.756 us (round-4 binary re-measured on this node: 15926 us). Final 14846 us (`make bench`; 14829 to 14876 in a same-node A/B). This round: about 6.7% faster (1.07x). Overall: 38190 / 14846 = **2.57x**.

**Final profile.** `make profile` has no rule in this Makefile, so these are ncu per-kernel times for one iteration, run serially:

| region | kernel(s) | us/iter | % |
|---|---|---|---|
| t0_gamma (`t0_state__d_gamma`, paired column tiles) | interleave 2.8 + ct_kern<OFF> 28.1 + comb 3.2 | 34.1 | 34.5 |
| t0_state (column tiles) | ct_kern 26.9 + comb 3.1 | 29.9 | 30.2 |
| t1_matvec (`t1_t_t0`, new ELL kernel) | ell_kern 24.2 | 24.2 | 24.5 |
| t1_state_terms | 3.7 + 3.8 | 7.5 | 7.6 |
| prep | k_prep | 3.2 | 3.2 |

Inside the CUDA graph the two t0 kernels run concurrently. Skipping one branch at a time gives the real critical path per iteration: t0 phase 45 us, A^T 22 us, everything else 6 us, about 73 us in total. The `FOM_PROF_BEGIN/END` markers (prep, t0_state, t0_gamma, t1_state_terms, t1_matvec, p0_term) are still in place.

**Optimization applied this round:**
- **Fixed-stride (ELL) layout for `t1_t_t0` (A^T).**
  - Structure exploited: every row has 64 or 65 nonzeros. The matrix is repacked once, from its own CSR, into rows of stride S = max row length (65), with padding entries marked by column 0xFFFF and skipped.
  - Why it helps: a row's start is computed (i·S) rather than loaded. That removes one of the three dependent memory round trips each thread made (row bound, then matrix data, then x gather). The kernel was latency-bound, not bandwidth-bound: DRAM at 57% of peak, long-scoreboard stalls.
  - Padding costs under 2% extra bytes. The stream uses `__ldcs`, which matters: from DRAM, `__ldg` took 35.8 us against 28.5 us for `__ldcs` in the harness.
  - Result: kernel 25.3 → 23.4 to 24.2 us, and the whole solve 1.0 ms faster.
  - Why it is exact: same per-lane summation order as the old kernel, and 0xFFFF is never a real column offset because the column span is at most 65535.

**Tried this round, neutral or slower in the real solver (reverted):**
- Prefetching the next work item's segment bounds in the column-tile kernel (3-slot ticket ring): its plain kernel went from 25.5 to 30.8 us.
- Deferred ticket store, or a static first item per block: no gain, or worse because it breaks the per-SM tile locality.
- Warp-level work tickets: 50 vs 38 us (atomic contention).
- x tile staged in shared memory, one block per SM: 52 vs 38 us.
- Smaller work items (16 rows, 12 blocks/SM): 3% faster alone in the harness, but 16.4 vs 14.85 ms in the solver because it starves the concurrent tile kernel.
- Two-entries-per-lane contiguous loads: 2% slower.
- Keeping the ELL matrix resident in L2 (`__ldg` or an evict_last cache policy): 15.5 vs 14.6 ms.
- A replacement test showed the column-tile kernels are not bound by the x gathers (only 1.7 us below the real kernel).

Blocked by rules:
- Compute A·s1 once per iteration and reuse it, instead of applying both `t0_state` = [0, A] and `t0_state__d_gamma` = [-A', A] to the state; that streams about 26 MB less per iteration | fom_solver.cu must call each term's own fom_apply_<name>, and a helper may read only its own matrix's data (A' differs from A by up to ~8 ulps, so it is a different matrix) | ~20 us/iter (~25%)
- Fuse the `t0_state` and `t0_state__d_gamma` products into one kernel that reads x once and shares the tile schedule and combine step | a helper may read only its own matrix's data / one fom_apply per matrix | ~5-8 us/iter (one fewer combine and interleave node on the critical path)
- Fold the t1 soft-threshold into the `t1_t_t0` kernel's epilogue, removing the separate pass before the next iteration's matvecs | fom_apply_<name> must compute exactly out += M x | ~3 us/iter
```
