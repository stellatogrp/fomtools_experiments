# Optimization report

## Timing

```
translated (emitted)   37917.3 us/solve
optimized              14862.1 us/solve
speedup                  2.55x
```

Optimization outcome: **kept (re-verified correct)**.

## Rounds

| round | before (us) | after (us) | gain | kept | agent (s) | outcome |
|---:|---:|---:|---:|:---:|---:|---|
| 1 | 37917.268 | 19849.913 | 47.6% | yes | 4028 | continued |
| 2 | 19763.043 | 17534.752 | 11.3% | yes | 3769 | continued |
| 3 | 17817.263 | 15605.881 | 12.4% | yes | 3688 | continued |
| 4 | 15673.117 | 14862.147 | 5.2% | yes | 4307 | continued |
| 5 | - | - | - | no | 2658 | the round left the sources unchanged |

## Kernel checks

Each `fom_apply_<name>` in the shipped `fom_ops.c`, checked by `make verify` against the package's reference over the same `fom_data.c` matrix. *basis* probes every unit vector, which recovers the routine's matrix column by column (an exact check), plus random vectors; *random* uses random vectors only (the matrix is over the probe budget). A `fused_<block>` row is a `fom_fused_<block>`, checked against the sum of its members: *basis* there probes every unit vector of each source with every schedule value at 0 and with each alone at 1, which recovers every member exactly.

| matrix | shape | nnz | rewritten | check | probes | max error |
|---|---|---|---|---|---|---|
| `t0_state` | 10000x80000 | 2599983 | yes | random | 2 | 6.217e-15 |
| `t0_state__d_gamma` | 10000x80000 | 5199966 | yes | random | 2 | 8.216e-15 |
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
| 1 | Store A once and apply it in `t0_state`, `t0_state__d_gamma` (≈[−A, A]) and `t1_t_t0` (= −step·Aᵀ) | each `fom_apply_<name>` may read only its own matrix's data | ~2x less matrix traffic per iteration (~82 MB → ~40 MB), roughly 30–40 µs/iter |
| 1 | Fuse `t0_state` and `t0_state__d_gamma` into one pass computing A(s1 + γ(s1 − s0)) | routine reads only its own matrix data, and the solver may not read matrix data | ~35–40 µs/iter (about one full A pass) |
| 1 | Reuse A·x_k from iteration k's `t0_state` as the A·x_prev half of iteration k+1's `d_gamma` | the solver may not rely on relations between matrices it cannot read, and each apply must be called as-is | ~20 µs/iter |
| 1 | Fuse `t1_state`, `t1_state__d_gamma` and the soft-threshold into the `t1_t_t0` kernel | every product must go through its own checked `fom_apply_<name>`; no fused routine is declared in the config | ~5–8 µs/iter |
| 1 | Run independent applies on separate graph branches (for example, `t1_state`/`t1_state__d_gamma` alongside `t0_state`, or `t0_state` alongside `d_gamma` with separate accumulators) | the `fom_apply_<name>` signatures in `include/` have no stream argument, so every op is hardwired to one stream | 1–8 µs/iter (measured in a two-stream test) |
| 2 | Merge `t0_state` (A on s1) and `t0_state__d_gamma` (`[A', -A'']` on γs, same sparsity pattern, values negated up to a few ulps) into one pass over A computing A((1+γ)s1 − γ s0) | kernel rule: each `fom_apply_<name>` reads only its own matrix's data, and the solver must call every apply | about 26 MB/iter less HBM traffic, est. 25-30% of solve time |
| 2 | Fold the `a0 + a0b` combine into the `t1_t_t0` matvec's x read (removes a 1.8 us serial kernel per iteration) | fixed `fom_apply_<name>(x, out)` signature with a single x; no matvec loops allowed in `fom_solver.cu` | about 2% |
| 2 | Merge `t1_state` and `t1_state__d_gamma` (identity / ±1 two-entry rows) into one kernel, or into the activation kernel | kernel rule: one matrix per apply; the solver must not read `FOM_MAT` data | now about 0 (already hidden by concurrency), about 9 us/iter in the sequential path |
| 2 | Launch apply kernels on caller-chosen streams instead of capturing each apply separately | fixed `fom_apply` signatures, and only the `fom_apply_*` entry points may be non-static in `fom_ops.cu` | about 0 (worked around with child graphs; only adds setup work) |
| 3 | Fused kernel reading A once for t0_state and both halves of t0_state__d_gamma (d_gamma is [-A, A] up to a few ulps of t0_state's values) | kernel rule: each `fom_apply_<name>` reads only its own matrix's data | about 24 MB less DRAM traffic per iteration (55 → 31 MB), roughly 10–15 us/iteration (~15%) |
| 3 | Let t1_t_t0 (= -step * A^T) reuse t0_state's packed A in a transposed sweep instead of its own 27 MB copy | kernel rule: own-data only | unknown; it would free about 22 MB of the 25 MB L2 persisting budget for t0/d_gamma |
| 3 | Get d_gamma's product from A s1 and A s0 in the solver instead of a third full product | kernel rule: the solver must call `fom_apply_*` and cannot rely on coefficient relationships | about 25 us/iteration (~30%) |
| 3 | Fold `k_comb_soft` (a0 += a0b) into the d_gamma kernel's final per-row reduction | kernel rule: an apply must compute exactly out += M x | about 3 us/iteration (~4%) |
| 3 | Store values as float32 plus an int16 correction (6 bytes per value instead of 8; about 1e-14 relative error per entry) | kernel rule: each apply must apply exactly M | about 20% fewer matrix bytes, unknown net gain |
| 4 | Reuse the previous iteration's A·s1 as the s0 half of the gamma product, since the d_gamma left block is -A up to a few ulps (would remove about 14 MB per iteration, roughly 25% of the time) | solver must apply each matrix through its own `fom_apply_<name>`, and the matrices are not bitwise equal | ~20–25% |
| 4 | One fused kernel streaming A and the paired gamma matrix together (shared x staging, one launch, no combine kernel) | each `fom_apply_<name>` and its helpers may read only its own matrix's data | ~5–10% |
| 4 | Fold the a0 + a0b combine into the A^T kernel's input load | `fom_apply_t1_t_t0` must compute out += M·x for the x it is given | ~2–3% |
| 4 | Fixed persisting hit ratios written as literals (0.3, 0.2) | no new floating-point constants | none (worked around with integer byte counts) |
| 5 | Reuse `A s1` from the previous iteration to replace most of the `t0_state__d_gamma` product. That product equals `gamma*(A s1_k - A' s1_{k-1})` with `A'` within a few ulps of `A`, and `A s1_{k-1}` is the previous iteration's `t0_state` product. | The solver may not read matrix data to discover identities between operators, and each `fom_apply_<name>` must apply exactly its own matrix | about 25-35% (removes about 28.6 MB of matrix reads per iteration) |
| 5 | Fuse `t0_state` and `t0_d_gamma` into one kernel that shares the column indices and the staged x slice, since both read the same `A` sparsity pattern. | Each `fom_apply_<name>` and its helpers may read only its own matrix's data | about 10-20% (one kernel node and about 5 MB of index reads less per iteration) |
| 5 | Drop the combine kernel by having both `t0` products accumulate into one buffer with `atomicAdd`. | Not a hard rule: the target notes require reductions in a fixed order for deterministic results, and this would make the result's last bits depend on which kernel finishes first | about 4-5% (about 3.5 us per iteration) |
| 5 | Apply the soft-threshold and state shift inside `fom_apply_t1_t_t0`'s kernel, saving one kernel node. | `fom_apply_<name>` must compute exactly `out += M x`; activations belong in `src/fom_solver.cu` | about 4-5% (about 3.5 us per iteration) |

## What the agent changed

```diff
--- src/fom_solver.cu (translated)
+++ src/fom_solver.cu (optimized)
@@ -2,26 +2,37 @@
 #include "fom_solver.h"
 #include "fom_data.h"
 #include "fom_ops.h"
+#include "fom_profile.h"
 
 #define N0 FOM_T0_DIM
 #define N1 FOM_T1_DIM
 #define NS FOM_STATE_DIM
 #define RED_BLOCKS 160
 
-static fom_float *d_s, *d_ns, *d_p, *d_a0, *d_b0, *d_a1, *d_b1, *d_t0, *d_red;
+#ifdef FOM_PROFILE
+#define PROF_SYNC() cudaDeviceSynchronize()
+#else
+#define PROF_SYNC() ((void)0)
+#endif
+
+static fom_float *d_buf[2];
+static fom_float *d_s, *d_ns, *d_p, *d_gs, *d_c0, *d_a0, *d_a0b, *d_a1, *d_red;
 static int g_alloc = 0;
 
 static void ensure_alloc(void)
 {
     if (g_alloc) return;
-    cudaMalloc((void **)&d_s, NS * sizeof(fom_float));
-    cudaMalloc((void **)&d_ns, NS * sizeof(fom_float));
+    cudaMalloc((void **)&d_buf[0], NS * sizeof(fom_float));
+    cudaMalloc((void **)&d_buf[1], NS * sizeof(fom_float));
+    d_s = d_buf[0];
+    d_ns = d_buf[1];
+    cudaMalloc((void **)&d_gs, NS * sizeof(fom_float));
     cudaMalloc((void **)&d_p, FOM_PARAM_P0_DIM * sizeof(fom_float));
+    cudaMalloc((void **)&d_c0, N0 * sizeof(fom_float));
     cudaMalloc((void **)&d_a0, N0 * sizeof(fom_float));
-    cudaMalloc((void **)&d_b0, N0 * sizeof(fom_float));
-    cudaMalloc((void **)&d_t0, N0 * sizeof(fom_float));
     cudaMalloc((void **)&d_a1, N1 * sizeof(fom_float));
-    cudaMalloc((void **)&d_b1, N1 * sizeof(fom_float));
+    cudaMalloc((void **)&d_a0b, N0 * sizeof(fom_float));
+    cudaMemset(d_a0b, 0, N0 * sizeof(fom_float));
     cudaMalloc((void **)&d_red, RED_BLOCKS * sizeof(fom_float));
     g_alloc = 1;
 }
@@ -33,15 +44,59 @@
     return v > 0 ? a : (v < 0 ? -a : (fom_float)0);
 }
 
-/* out = soft(a + g*b, lam); b ignored when use_b == 0 */
-__global__ void k_act(const fom_float *a, const fom_float *b, fom_float g, int use_b,
-                      fom_float lam, fom_float *out, int n)
+/* Seed the accumulators (a0 = P p, which is loop-invariant; a1 = 0) and,
+   when gamma != 0, form gs = gamma * s so the gamma-scaled products
+   accumulate straight into a0 / a1. */
+__global__ void k_prep(const fom_float *c0, fom_float *a0, fom_float *a1,
+                       const fom_float *s, fom_float *gs, fom_float g, int use_g)
 {
     int i = blockIdx.x * blockDim.x + threadIdx.x;
-    if (i < n) {
-        fom_float v = a[i];
-        if (use_b) v += g * b[i];
-        out[i] = soft(v, lam);
+    if (i < N0) a0[i] = c0[i];
+    if (i < N1) a1[i] = 0.0;
+    if (use_g && i < NS) gs[i] = g * s[i];
+}
+
+/* in-place soft threshold */
+__global__ void k_soft(fom_float *a, fom_float lam, int n)
+{
+    int i = blockIdx.x * blockDim.x + threadIdx.x;
+    if (i < n) a[i] = soft(a[i], lam);
+}
+
+/* state update ns[0:N1] = s[N1:], ns[N1:] = soft(a1, lam), fused with the
+   next iteration's prep (a0 = P p, a1 = 0, gs = gamma_next * ns). Each
+   thread only touches index i of a1 / a0, so the reads and resets do not race. */
+__global__ void k_act_shift(fom_float *a1, const fom_float *s, fom_float lam, fom_float *ns,
+                            const fom_float *c0, fom_float *a0, fom_float *gs, fom_float gn, int use_gn,
+                            fom_float *a0b)
+{
+    int i = blockIdx.x * blockDim.x + threadIdx.x;
+    if (i < N1) {
+        fom_float lo = s[N1 + i];
+        fom_float hi = soft(a1[i], lam);
+        ns[i] = lo;
+        ns[N1 + i] = hi;
+        a1[i] = 0.0;
+        if (i < N0) {
+            a0[i] = c0[i];
+            if (a0b) a0b[i] = 0.0;
+        }
+        if (use_gn) {
+            gs[i] = gn * lo;
+            gs[N1 + i] = gn * hi;
+        }
+    }
+}
+
+/* t0 pre-activation = (c0 + A s) + D (gamma s): the two products run as
+   concurrent graph branches into a0 / a0b and are added here, in the same
+   order as the sequential accumulation. */
+__global__ void k_comb_soft(fom_float *a0, const fom_float *a0b, fom_float lam, int use_lam)
+{
+    int i = blockIdx.x * blockDim.x + threadIdx.x;
+    if (i < N0) {
+        fom_float v = a0[i] + a0b[i];
+        a0[i] = use_lam ? soft(v, lam) : v;
     }
 }
 
@@ -93,6 +148,10 @@
     for (int i = 0; i < NS; i++) work->state[i] = FOM_STATE_INIT[i];
 }
 
+/* The workspace is page-locked once, so the per-solve state / parameter /
+   solution transfers run as direct DMA instead of staged pageable copies. */
+static FomWorkspace *g_pinned = 0;
+
 void fom_setup(FomWorkspace *work, const FomSettings *settings)
 {
     work->settings = *settings;
@@ -101,6 +160,11 @@
     work->solution.residual = 0;
     work->solution.status = FOM_MAX_ITER_REACHED;
     ensure_alloc();
+    if (g_pinned != work) {
+        if (g_pinned) cudaHostUnregister(g_pinned);
+        g_pinned = (cudaHostRegister(work, sizeof(FomWorkspace), cudaHostRegisterDefault) == cudaSuccess) ? work : 0;
+        cudaGetLastError();
+    }
 }
 
 void fom_update_params(FomWorkspace *work, const FomParams *params)
@@ -109,50 +173,197 @@
     reset_state(work);
 }
 
+#define T 256
+
+/* Per-solve setup on the device: a0 seed c0 = P p (loop-invariant), a1 = 0,
+   gs = gamma_0 * s. */
+static void enqueue_start(void)
+{
+    cudaMemsetAsync(d_c0, 0, N0 * sizeof(fom_float), cudaStreamPerThread);
+    fom_apply_t0_p_p0(d_p, d_c0);
+    fom_float g0 = fom_sched_gamma(0);
+    k_prep<<<(NS + T - 1) / T, T, 0, cudaStreamPerThread>>>(d_c0, d_a0, d_a1, d_s, d_gs, g0, g0 != 0);
+}
+
+/* One iteration: ns = [s1, t1]; residual max|ns - s| into d_red if asked. */
+static void enqueue_iter(int it, int want_res)
+{
+    int use_g = (fom_sched_gamma(it) != 0);
+    fom_float gn = fom_sched_gamma(it + 1);
+    /* t0 = soft(A0 s + P p + D0 (gamma s), lam0) */
+    FOM_PROF_BEGIN(t0_state);
+    fom_apply_t0_state(d_s, d_a0);
+    PROF_SYNC();
+    FOM_PROF_END(t0_state);
+    FOM_PROF_BEGIN(t0_d_gamma);
+    if (use_g) fom_apply_t0_state__d_gamma(d_gs, d_a0);
+    PROF_SYNC();
+    FOM_PROF_END(t0_d_gamma);
+    if (FOM_T0_LAMBDA != 0) k_soft<<<(N0 + T - 1) / T, T, 0, cudaStreamPerThread>>>(d_a0, FOM_T0_LAMBDA, N0);
+    /* t1 = soft(A1 s + B t0 + D1 (gamma s), lam1) */
+    FOM_PROF_BEGIN(t1_small);
+    fom_apply_t1_state(d_s, d_a1);
+    if (use_g) fom_apply_t1_state__d_gamma(d_gs, d_a1);
+    PROF_SYNC();
+    FOM_PROF_END(t1_small);
+    FOM_PROF_BEGIN(t1_t_t0);
+    fom_apply_t1_t_t0(d_a0, d_a1);
+    PROF_SYNC();
+    FOM_PROF_END(t1_t_t0);
+    FOM_PROF_BEGIN(act_update);
+    k_act_shift<<<(N1 + T - 1) / T, T, 0, cudaStreamPerThread>>>(d_a1, d_s, FOM_T1_LAMBDA, d_ns, d_c0, d_a0, d_gs, gn, gn != 0, (fom_float *)0);
+    PROF_SYNC();
+    FOM_PROF_END(act_update);
+    if (want_res) {
+        k_maxdiff<<<RED_BLOCKS, 256, 0, cudaStreamPerThread>>>(d_ns, d_s, NS, d_red);
+        k_maxfinal<<<1, 256, 0, cudaStreamPerThread>>>(d_red, RED_BLOCKS);
+    }
+    fom_float *tmp = d_s; d_s = d_ns; d_ns = tmp;
+}
+
+/* Fixed-iteration solves (no tolerance test) replay one captured CUDA graph
+   of the whole loop; it is captured on the second solve, once the operators
+   have built their device layouts during the first. */
+static cudaGraphExec_t g_exec = 0;
+static int g_exec_maxit = -1, g_graph_off = 0, g_solves = 0;
+static fom_float *g_s_start = 0, *g_s_end = 0;
+
+/* Capture the work enqueued by the code between cap_begin() and cap_add()
+   on cudaStreamPerThread as a child-graph node of G that depends on deps.
+   The apply routines launch on that stream, so each one is captured on its
+   own and the nodes are wired into a DAG whose independent products run
+   concurrently. */
+static int g_cap_err = 0;
+
+static void cap_begin(void)
+{
+    if (cudaStreamBeginCapture(cudaStreamPerThread, cudaStreamCaptureModeThreadLocal) != cudaSuccess) g_cap_err = 1;
+}
+
+static cudaGraphNode_t cap_add(cudaGraph_t G, const cudaGraphNode_t *deps, size_t nd)
+{
+    cudaGraph_t sub = 0;
+    cudaGraphNode_t n = 0;
+    if (cudaStreamEndCapture(cudaStreamPerThread, &sub) != cudaSuccess || !sub) { g_cap_err = 1; return 0; }
+    if (cudaGraphAddChildGraphNode(&n, G, deps, nd, sub) != cudaSuccess) g_cap_err = 1;
+    cudaGraphDestroy(sub);
+    return n;
+}
+
+/* Iteration DAG: [A s -> a0] || [D (gamma s) -> a0b] || [A1 s + D1 (gamma s) -> a1],
+   then a0 += a0b, t1 product, activation/shift. Each accumulator sees its
+   terms in the same order as the sequential path, so results are identical. */
+static int graph_ready(int maxit)
+{
+    if (g_graph_off) return 0;
+    if (g_exec && g_exec_maxit == maxit) return 1;
+    if (g_exec) { cudaGraphExecDestroy(g_exec); g_exec = 0; }
+    cudaGraph_t G = 0;
+    fom_float *s0 = d_s, *ns0 = d_ns;
+    g_cap_err = 0;
+    cudaGetLastError();
+    if (cudaGraphCreate(&G, 0) != cudaSuccess) { g_graph_off = 1; return 0; }
+    cap_begin();
+    enqueue_start();
+    cudaGraphNode_t prev = cap_add(G, 0, 0);
+    for (int it = 0; it < maxit && !g_cap_err; it++) {
+        int use_g = (fom_sched_gamma(it) != 0);
+        fom_float gn = fom_sched_gamma(it + 1);
+        cudaGraphNode_t dep[3];
+        size_t nd = 0;
+        cap_begin();
+        fom_apply_t0_state(d_s, d_a0);
+        dep[nd++] = cap_add(G, &prev, 1);
+        if (use_g) {
+            cap_begin();
+            fom_apply_t0_state__d_gamma(d_gs, d_a0b);
+            dep[nd++] = cap_add(G, &prev, 1);
+        }
+        cap_begin();
+        fom_apply_t1_state(d_s, d_a1);
+        if (use_g) fom_apply_t1_state__d_gamma(d_gs, d_a1);
+        dep[nd++] = cap_add(G, &prev, 1);
+        cap_begin();
+        if (use_g || FOM_T0_LAMBDA != 0)
+            k_comb_soft<<<(N0 + T - 1) / T, T, 0, cudaStreamPerThread>>>(d_a0, d_a0b, FOM_T0_LAMBDA, FOM_T0_LAMBDA != 0);
+        fom_apply_t1_t_t0(d_a0, d_a1);
+        k_act_shift<<<(N1 + T - 1) / T, T, 0, cudaStreamPerThread>>>(d_a1, d_s, FOM_T1_LAMBDA, d_ns, d_c0, d_a0, d_gs, gn, gn != 0, d_a0b);
+        if (it == maxit - 1) {
+            k_maxdiff<<<RED_BLOCKS, 256, 0, cudaStreamPerThread>>>(d_ns, d_s, NS, d_red);
+            k_maxfinal<<<1, 256, 0, cudaStreamPerThread>>>(d_red, RED_BLOCKS);
+        }
+        prev = cap_add(G, dep, nd);
+        fom_float *tmp = d_s; d_s = d_ns; d_ns = tmp;
+    }
+    cudaError_t e = g_cap_err ? cudaErrorUnknown : cudaGraphInstantiate(&g_exec, G, 0);
+    cudaGraphDestroy(G);
+    g_s_start = s0;
+    g_s_end = d_s;
+    d_s = s0;
+    d_ns = ns0;
+    if (e != cudaSuccess) {
+        g_exec = 0;
+        g_graph_off = 1;
+        cudaGetLastError();
+        return 0;
+    }
+    g_exec_maxit = maxit;
+    return 1;
+}
+
 fom_int fom_solve(FomWorkspace *work)
 {
     ensure_alloc();
-    cudaMemcpy(d_s, work->state, NS * sizeof(fom_float), cudaMemcpyHostToDevice);
-    cudaMemcpy(d_p, work->params.p0, FOM_PARAM_P0_DIM * sizeof(fom_float), cudaMemcpyHostToDevice);
-    const int T = 256;
     const int maxit = work->settings.max_iter;
     const int use_tol = work->settings.check_termination && work->settings.tol > 0;
     fom_float res = 0;
     fom_int status = FOM_MAX_ITER_REACHED;
-    int it;
-    for (it = 0; it < maxit; it++) {
-        fom_float g = fom_sched_gamma(it);
-        int use_g = (g != 0);
-        /* t0 */
-        cudaMemsetAsync(d_a0, 0, N0 * sizeof(fom_float));
-        fom_apply_t0_state(d_s, d_a0);
-        fom_apply_t0_p_p0(d_p, d_a0);
-        if (use_g) {
-            cudaMemsetAsync(d_b0, 0, N0 * sizeof(fom_float));
-            fom_apply_t0_state__d_gamma(d_s, d_b0);
-        }
-        k_act<<<(N0 + T - 1) >> 8, T>>>(d_a0, d_b0, g, use_g, FOM_T0_LAMBDA, d_t0, N0);
-        /* t1 */
-        cudaMemsetAsync(d_a1, 0, N1 * sizeof(fom_float));
-        fom_apply_t1_state(d_s, d_a1);
-        fom_apply_t1_t_t0(d_t0, d_a1);
-        if (use_g) {
-            cudaMemsetAsync(d_b1, 0, N1 * sizeof(fom_float));
-            fom_apply_t1_state__d_gamma(d_s, d_b1);
-        }
-        /* state update: ns[0:N1] = s[N1:], ns[N1:] = t1 */
-        k_act<<<(N1 + T - 1) >> 8, T>>>(d_a1, d_b1, g, use_g, FOM_T1_LAMBDA, d_ns + N1, N1);
-        cudaMemcpyAsync(d_ns, d_s + N1, N1 * sizeof(fom_float), cudaMemcpyDeviceToDevice);
-        if (use_tol || it == maxit - 1) {
-            k_maxdiff<<<RED_BLOCKS, 256>>>(d_ns, d_s, NS, d_red);
-            k_maxfinal<<<1, 256>>>(d_red, RED_BLOCKS);
-            cudaMemcpy(&res, d_red, sizeof(fom_float), cudaMemcpyDeviceToHost);
-        }
-        fom_float *tmp = d_s; d_s = d_ns; d_ns = tmp;
-        if (use_tol && res < work->settings.tol) { status = FOM_SOLVED; it++; break; }
-    }
-    cudaMemcpy(work->state, d_s, NS * sizeof(fom_float), cudaMemcpyDeviceToHost);
-    for (int i = 0; i < NS; i++) work->solution.x[i] = work->state[i];
+    int it = 0;
+    int use_graph = 0;
+#ifndef FOM_PROFILE
+    use_graph = !use_tol && maxit > 0 && g_solves > 0 && graph_ready(maxit);
+#endif
+    g_solves++;
+    if (use_graph) {
+        d_s = g_s_start;
+        d_ns = (d_s == d_buf[0]) ? d_buf[1] : d_buf[0];
+    }
+    /* all transfers are queued on the iteration stream (the workspace is
+       page-locked), with a single host wait at the end */
+    cudaMemcpyAsync(d_s, work->state, NS * sizeof(fom_float), cudaMemcpyHostToDevice, cudaStreamPerThread);
+    cudaMemcpyAsync(d_p, work->params.p0, FOM_PARAM_P0_DIM * sizeof(fom_float), cudaMemcpyHostToDevice,
+                    cudaStreamPerThread);
+    if (use_graph) {
+        FOM_PROF_BEGIN(graph);
+        cudaGraphLaunch(g_exec, cudaStreamPerThread);
+        d_s = g_s_end;
+        d_ns = (d_s == d_buf[0]) ? d_buf[1] : d_buf[0];
+        cudaMemcpyAsync(&work->solution.residual, d_red, sizeof(fom_float), cudaMemcpyDeviceToHost,
+                        cudaStreamPerThread);
+        cudaMemcpyAsync(work->state, d_s, NS * sizeof(fom_float), cudaMemcpyDeviceToHost, cudaStreamPerThread);
+        cudaMemcpyAsync(work->solution.x, d_s, NS * sizeof(fom_float), cudaMemcpyDeviceToHost, cudaStreamPerThread);
+        cudaStreamSynchronize(cudaStreamPerThread);
+        res = work->solution.residual;
+        FOM_PROF_END(graph);
+        work->solution.iter = maxit;
+        work->solution.status = status;
+        return status;
+    } else {
+        enqueue_start();
+        for (it = 0; it < maxit; it++) {
+            int want = use_tol || it == maxit - 1;
+            enqueue_iter(it, want);
+            if (want) {
+                FOM_PROF_BEGIN(residual);
+                cudaMemcpy(&res, d_red, sizeof(fom_float), cudaMemcpyDeviceToHost);
+                FOM_PROF_END(residual);
+            }
+            if (use_tol && res < work->settings.tol) { status = FOM_SOLVED; it++; break; }
+        }
+    }
+    cudaMemcpyAsync(work->state, d_s, NS * sizeof(fom_float), cudaMemcpyDeviceToHost, cudaStreamPerThread);
+    cudaMemcpyAsync(work->solution.x, d_s, NS * sizeof(fom_float), cudaMemcpyDeviceToHost, cudaStreamPerThread);
+    cudaStreamSynchronize(cudaStreamPerThread);
     work->solution.iter = it;
     work->solution.residual = res;
     work->solution.status = status;
@@ -161,11 +372,12 @@
 
 void fom_cleanup(FomWorkspace *work)
 {
-    (void)work;
+    if (g_pinned && g_pinned == work) { cudaHostUnregister(g_pinned); g_pinned = 0; }
     if (!g_alloc) return;
-    cudaFree(d_s); cudaFree(d_ns); cudaFree(d_p);
-    cudaFree(d_a0); cudaFree(d_b0); cudaFree(d_t0);
-    cudaFree(d_a1); cudaFree(d_b1); cudaFree(d_red);
+    if (g_exec) { cudaGraphExecDestroy(g_exec); g_exec = 0; }
+    g_exec_maxit = -1;
+    cudaFree(d_buf[0]); cudaFree(d_buf[1]); cudaFree(d_p); cudaFree(d_gs);
+    cudaFree(d_c0); cudaFree(d_a0); cudaFree(d_a0b); cudaFree(d_a1); cudaFree(d_red);
     g_alloc = 0;
 }
 
--- src/fom_ops.cu (translated)
+++ src/fom_ops.cu (optimized)
@@ -9,7 +9,347 @@
 
 #define FOM_OPS_THREADS 256
 
-static __global__ void fom_apply_t0_state_kern(const fom_float *x, fom_float *out) {
+#include <stdlib.h>
+#include <stdint.h>
+#include <string.h>
+
+/* Host-side packing helpers (run once, lazily, on the first call of the
+   routine that owns the data; they only see the arrays passed to them). */
+static void *fom_dev_upload(const void *h, size_t bytes)
+{
+    void *d = 0;
+    if (cudaMalloc(&d, bytes) != cudaSuccess) return 0;
+    cudaMemcpy(d, h, bytes, cudaMemcpyHostToDevice);
+    return d;
+}
+
+/* Column-tiled layout for a wide CSR matrix: columns are split into tiles of
+   tc columns (pairs of columns for the paired layout) whose slice of x fits in
+   shared memory; entries are stored tile-major, row by row, with 16-bit column
+   offsets. Block (chunk, tile) stages its x slice in shared memory and writes a
+   per-row partial; the last block of a chunk adds the partials to out in tile
+   order (deterministic). Paired layout: row = [left | right], right column =
+   left column + shift, right value = -left value moved by a few ulps, stored
+   exactly as an int8 bit-pattern delta; the staged x slice interleaves the
+   left and right entries of each column so a pair is one 16-byte read. */
+typedef struct {
+    int ok, nt, tc, nrows, chunks, rpb;
+    fom_int base, shift, span;
+    int *seg;
+    uint16_t *c16;
+    fom_float *val;
+    signed char *d8;
+    fom_float *part;
+    unsigned *cnt;
+    void *blob;           /* val | c16 | d8 in one allocation */
+    size_t win_bytes;     /* L2 persisting window over blob (0: none) */
+    float hit;
+} fom_tiled;
+
+#define FOM_TILE_THREADS 1024
+#define FOM_TILE_G 16
+/* per-launch batching: U loads per lane per row, RB rows per batch */
+#ifndef T0_U
+#define T0_U 5
+#define T0_RB 2
+#define T1_U 3
+#define T1_RB 2
+#endif
+#define FOM_TILE_PAD (FOM_TILE_G * 8)
+
+template <int PAIR, int U, int RB>
+static __global__ void fom_tiled_kern(int nrows, int rpb, int tc, int nt, const int *__restrict__ seg,
+                                      const uint16_t *__restrict__ c16, const fom_float *__restrict__ val,
+                                      const signed char *__restrict__ d8, const fom_float *x, fom_int shift,
+                                      fom_int span, fom_float *part, unsigned *cnt, fom_float *out);
+#define FOM_TILED_K0 fom_tiled_kern<0, T0_U, T0_RB>
+#define FOM_TILED_K1 fom_tiled_kern<1, T1_U, T1_RB>
+
+static void fom_tiled_pack(fom_tiled *T, int nrows, const fom_int *rp, const fom_int *col,
+                           const fom_float *val, int pair, int tc, int chunks, int persist)
+{
+    T->ok = 0;
+    fom_int nnz = rp[nrows];
+    if (nnz <= 0) return;
+    fom_int shift = 0;
+    if (pair) {
+        shift = col[rp[0] + (rp[1] - rp[0]) / 2] - col[rp[0]];
+        for (int i = 0; i < nrows; i++) {
+            fom_int len = rp[i + 1] - rp[i];
+            if (len % 2) return;
+            for (fom_int k = 0; k < len / 2; k++) {
+                fom_int a = rp[i] + k, b = rp[i] + len / 2 + k;
+                if (col[b] != col[a] + shift) return;
+                fom_float na = -val[a];
+                long long d = 0;
+                memcpy(&d, &val[b], 8);
+                long long e = 0;
+                memcpy(&e, &na, 8);
+                if (d - e < -128 || d - e > 127) return;
+            }
+        }
+    }
+    /* left (or all) columns */
+    fom_int lo = col[0], hi = col[0];
+    for (int i = 0; i < nrows; i++) {
+        fom_int len = rp[i + 1] - rp[i], h = pair ? len / 2 : len;
+        for (fom_int k = 0; k < h; k++) {
+            fom_int c = col[rp[i] + k];
+            lo = c < lo ? c : lo;
+            hi = c > hi ? c : hi;
+        }
+    }
+    fom_int span = hi - lo + 1;
+    if (span > 65536) return;
+    int nt = (int)((span + tc - 1) / tc);
+    fom_int ne = pair ? nnz / 2 : nnz;
+    int *seg = (int *)malloc(((size_t)nt * nrows + 1) * sizeof(int));
+    size_t nep = (size_t)ne + FOM_TILE_PAD;
+    uint16_t *c16 = (uint16_t *)calloc(nep, sizeof(uint16_t));
+    fom_float *v = (fom_float *)calloc(nep, sizeof(fom_float));
+    signed char *d8 = (signed char *)calloc(nep, 1);
+    fom_int *pos = (fom_int *)malloc((size_t)nrows * sizeof(fom_int));
+    for (int i = 0; i < nrows; i++) pos[i] = rp[i];
+    fom_int q = 0;
+    for (int t = 0; t < nt; t++) {
+        for (int i = 0; i < nrows; i++) {
+            seg[(size_t)t * nrows + i] = (int)q;
+            fom_int len = rp[i + 1] - rp[i], h = pair ? len / 2 : len, end = rp[i] + h;
+            while (pos[i] < end && col[pos[i]] - lo < (fom_int)(t + 1) * tc) {
+                fom_int a = pos[i];
+                c16[q] = (uint16_t)(col[a] - lo);
+                v[q] = val[a];
+                if (pair) {
+                    fom_float na = -val[a];
+                    long long d = 0, e = 0;
+                    memcpy(&d, &val[a + h], 8);
+                    memcpy(&e, &na, 8);
+                    d8[q] = (signed char)(d - e);
+                }
+                q++;
+                pos[i]++;
+            }
+        }
+    }
+    seg[(size_t)nt * nrows] = (int)q;
+    T->nt = nt;
+    T->tc = tc;
+    T->nrows = nrows;
+    T->chunks = chunks;
+    T->rpb = (nrows + chunks - 1) / chunks;
+    T->base = lo;
+    T->shift = shift;
+    T->span = span;
+    T->seg = (int *)fom_dev_upload(seg, ((size_t)nt * nrows + 1) * sizeof(int));
+    size_t bv = nep * sizeof(fom_float), bc = nep * sizeof(uint16_t), bd = pair ? nep : 0;
+    size_t bytes = bv + bc + bd;
+    char *blob = 0;
+    if (cudaMalloc((void **)&blob, bytes) == cudaSuccess) {
+        cudaMemcpy(blob, v, bv, cudaMemcpyHostToDevice);
+        cudaMemcpy(blob + bv, c16, bc, cudaMemcpyHostToDevice);
+        if (pair) cudaMemcpy(blob + bv + bc, d8, bd, cudaMemcpyHostToDevice);
+    }
+    T->blob = blob;
+    T->val = (fom_float *)blob;
+    T->c16 = blob ? (uint16_t *)(blob + bv) : 0;
+    T->d8 = (blob && pair) ? (signed char *)(blob + bv + bc) : 0;
+    T->win_bytes = 0;
+    T->hit = 0;
+    if (persist && blob) {
+        /* keep this operator resident in the persisting part of L2 */
+        int pdev = 0, pmax = 0, wmax = 0;
+        cudaGetDevice(&pdev);
+        cudaDeviceGetAttribute(&pmax, cudaDevAttrMaxPersistingL2CacheSize, pdev);
+        cudaDeviceGetAttribute(&wmax, cudaDevAttrMaxAccessPolicyWindowSize, pdev);
+        if (pmax > 0 && wmax > 0) {
+            cudaDeviceSetLimit(cudaLimitPersistingL2CacheSize, (size_t)pmax);
+            size_t lim = 0;
+            cudaDeviceGetLimit(&lim, cudaLimitPersistingL2CacheSize);
+            /* persist a random `persist` percent of the window's lines, so the
+               operators of one iteration share the carve-out instead of the
+               last one launched owning it */
+            T->win_bytes = bytes < (size_t)wmax ? bytes : (size_t)wmax;
+            size_t keep = T->win_bytes / 100 * (size_t)persist;
+            keep = keep < lim ? keep : lim;
+            T->hit = (float)keep / (float)T->win_bytes;
+        }
+    }
+    /* one shared limit for both instantiations (each routine packs once) */
+    int dev = 0, smx = 0;
+    cudaGetDevice(&dev);
+    cudaDeviceGetAttribute(&smx, cudaDevAttrMaxSharedMemoryPerBlockOptin, dev);
+    smx -= 64;  /* static shared memory of the kernel */
+    if ((size_t)(pair ? 2 : 1) * tc * sizeof(fom_float) > (size_t)smx) smx = -1;
+    if (smx > 0) {
+        cudaFuncSetAttribute(FOM_TILED_K0, cudaFuncAttributeMaxDynamicSharedMemorySize, smx);
+        cudaFuncSetAttribute(FOM_TILED_K1, cudaFuncAttributeMaxDynamicSharedMemorySize, smx);
+    }
+    cudaMalloc((void **)&T->part, (size_t)nt * nrows * sizeof(fom_float));
+    cudaMalloc((void **)&T->cnt, chunks * sizeof(unsigned));
+    cudaMemset(T->cnt, 0, chunks * sizeof(unsigned));
+    T->ok = (smx > 0) && (q == ne) && T->seg && blob && T->part && T->cnt;
+    free(seg); free(c16); free(v); free(d8); free(pos);
+}
+
+template <int PAIR, int U, int RB>
+static __global__ void __launch_bounds__(FOM_TILE_THREADS, 1)
+fom_tiled_kern(int nrows, int rpb, int tc, int nt, const int *__restrict__ seg,
+               const uint16_t *__restrict__ c16, const fom_float *__restrict__ val,
+               const signed char *__restrict__ d8, const fom_float *x, fom_int shift,
+               fom_int span, fom_float *part, unsigned *cnt, fom_float *out)
+{
+    extern __shared__ fom_float sx[];
+    __shared__ int last;
+    int t = blockIdx.y;
+    fom_int c0 = (fom_int)t * tc;
+    fom_int cn = span - c0 < tc ? span - c0 : tc;
+    {
+        /* stage the x slice with 16-byte loads, several in flight per thread */
+        const fom_float *xa = x + c0, *xc = x + shift + c0;
+        if (!((size_t)tc & 1) && !((size_t)xa & 15) && (!PAIR || !((size_t)xc & 15))) {
+            const double2 *a2 = (const double2 *)xa, *b2 = (const double2 *)xc;
+            double2 *s2 = (double2 *)sx;
+            int h = (int)(cn >> 1);
+#pragma unroll 4
+            for (int j = threadIdx.x; j < h; j += blockDim.x) {
+                double2 u = a2[j];
+                if (PAIR) {
+                    double2 w = b2[j];
+                    s2[2 * j] = make_double2(u.x, w.x);
+                    s2[2 * j + 1] = make_double2(u.y, w.y);
+                } else {
+                    s2[j] = u;
+                }
+            }
+            if ((cn & 1) && threadIdx.x == 0) {
+                if (PAIR) s2[cn - 1] = make_double2(xa[cn - 1], xc[cn - 1]);
+                else sx[cn - 1] = xa[cn - 1];
+            }
+        } else {
+            for (int j = threadIdx.x; j < cn; j += blockDim.x) {
+                if (PAIR) ((double2 *)sx)[j] = make_double2(x[c0 + j], x[shift + c0 + j]);
+                else sx[j] = x[c0 + j];
+            }
+        }
+    }
+    __syncthreads();
+    int grp = threadIdx.x / FOM_TILE_G, l = threadIdx.x % FOM_TILE_G, ng = blockDim.x / FOM_TILE_G;
+    int r0 = blockIdx.x * rpb, r1 = r0 + rpb < nrows ? r0 + rpb : nrows;
+    /* RB rows per group at a time: their segment bounds, then the first U
+       entries per lane of each row, are all requested before any is used;
+       each lane still adds its entries l, l + G, l + 2G, ... in order. */
+    const size_t sb = (size_t)t * nrows;
+    for (int ib = r0; ib < r1; ib += ng * RB) {
+        int k0[RB], k1[RB];
+#pragma unroll
+        for (int r = 0; r < RB; r++) {
+            int i = ib + grp + r * ng;
+            k0[r] = k1[r] = 0;
+            if (i < r1) {
+                k0[r] = seg[sb + i];
+                k1[r] = seg[sb + i + 1];
+            }
+        }
+        fom_float va[RB][U];
+        int cc[RB][U], dd[RB][U];
+#pragma unroll
+        for (int r = 0; r < RB; r++) {
+#pragma unroll
+            for (int u = 0; u < U; u++) {
+                int kb = k0[r] + l + u * FOM_TILE_G;
+                va[r][u] = 0.0;
+                cc[r][u] = 0;
+                dd[r][u] = 0;
+                if (kb < k1[r]) {
+                    va[r][u] = val[kb];
+                    cc[r][u] = (int)c16[kb] - (int)c0;
+                    if (PAIR) dd[r][u] = (int)d8[kb];
+                }
+            }
+        }
+        fom_float s[RB];
+#pragma unroll
+        for (int r = 0; r < RB; r++) {
+            s[r] = 0.0;
+#pragma unroll
+            for (int u = 0; u < U; u++) {
+                if (k0[r] + l + u * FOM_TILE_G < k1[r]) {
+                    if (PAIR) {
+                        /* x pair (left, right) of this column in one 16-byte read */
+                        double2 xp = ((const double2 *)sx)[cc[r][u]];
+                        fom_float vb = __longlong_as_double(__double_as_longlong(-va[r][u]) + (long long)dd[r][u]);
+                        s[r] += va[r][u] * xp.x;
+                        s[r] += vb * xp.y;
+                    } else {
+                        s[r] += va[r][u] * sx[cc[r][u]];
+                    }
+                }
+            }
+            /* rows longer than U * G entries */
+            for (int kb = k0[r] + l + U * FOM_TILE_G; kb < k1[r]; kb += FOM_TILE_G) {
+                fom_float v = val[kb];
+                int c = (int)c16[kb] - (int)c0;
+                if (PAIR) {
+                    double2 xp = ((const double2 *)sx)[c];
+                    fom_float vb = __longlong_as_double(__double_as_longlong(-v) + (long long)d8[kb]);
+                    s[r] += v * xp.x;
+                    s[r] += vb * xp.y;
+                } else {
+                    s[r] += v * sx[c];
+                }
+            }
+        }
+#pragma unroll
+        for (int r = 0; r < RB; r++) {
+            for (int o = FOM_TILE_G / 2; o > 0; o /= 2) s[r] += __shfl_down_sync(0xffffffff, s[r], o, FOM_TILE_G);
+            int i = ib + grp + r * ng;
+            if (l == 0 && i < r1) part[sb + i] = s[r];
+        }
+    }
+    __threadfence();
+    __syncthreads();
+    if (threadIdx.x == 0) last = (atomicAdd(&cnt[blockIdx.x], 1u) == (unsigned)nt - 1);
+    __syncthreads();
+    if (last) {
+        __threadfence();
+        for (int i = r0 + threadIdx.x; i < r1; i += blockDim.x) {
+            fom_float s = 0.0;
+            for (int u = 0; u < nt; u++) s += ((volatile fom_float *)part)[(size_t)u * nrows + i];
+            out[i] += s;
+        }
+        if (threadIdx.x == 0) cnt[blockIdx.x] = 0;
+    }
+}
+
+static void fom_tiled_launch(const fom_tiled *T, int pair, const fom_float *x, fom_float *out)
+{
+    dim3 grid(T->chunks, T->nt);
+    size_t sm = (size_t)(pair ? 2 : 1) * T->tc * sizeof(fom_float);
+    fom_int span = T->span;  /* the last tile's x slice is clipped to it */
+    cudaLaunchConfig_t cfg;
+    memset(&cfg, 0, sizeof(cfg));
+    cfg.gridDim = grid;
+    cfg.blockDim = dim3(FOM_TILE_THREADS);
+    cfg.dynamicSmemBytes = sm;
+    cfg.stream = cudaStreamPerThread;
+    cudaLaunchAttribute at[1];
+    if (T->win_bytes) {
+        memset(at, 0, sizeof(at));
+        at[0].id = cudaLaunchAttributeAccessPolicyWindow;
+        at[0].val.accessPolicyWindow.base_ptr = T->blob;
+        at[0].val.accessPolicyWindow.num_bytes = T->win_bytes;
+        at[0].val.accessPolicyWindow.hitRatio = T->hit;
+        at[0].val.accessPolicyWindow.hitProp = cudaAccessPropertyPersisting;
+        at[0].val.accessPolicyWindow.missProp = cudaAccessPropertyStreaming;
+        cfg.attrs = at;
+        cfg.numAttrs = 1;
+    }
+    const fom_float *xb = x + T->base;
+    if (pair) cudaLaunchKernelEx(&cfg, FOM_TILED_K1, T->nrows, T->rpb, T->tc, T->nt, (const int *)T->seg, (const uint16_t *)T->c16, (const fom_float *)T->val, (const signed char *)T->d8, xb, T->shift, span, T->part, T->cnt, out);
+    else cudaLaunchKernelEx(&cfg, FOM_TILED_K0, T->nrows, T->rpb, T->tc, T->nt, (const int *)T->seg, (const uint16_t *)T->c16, (const fom_float *)T->val, (const signed char *)T->d8, xb, T->shift, span, T->part, T->cnt, out);
+}
+
+static __global__ void fom_apply_t0_state_csr_kern(const fom_float *x, fom_float *out) {
     /* One warp per row: lanes stride the row (coalesced), then a shuffle sum. */
     fom_int i = (blockIdx.x * blockDim.x + threadIdx.x) / 32;
     fom_int lane = threadIdx.x % 32;
@@ -20,11 +360,31 @@
     if (lane == 0) out[i] += s;
 }
 
+
+static fom_tiled t0s_tl;
+static int t0s_init = 0;
+
+static void fom_t0_state_pack(void)
+{
+    t0s_init = 1;
+    fom_int rp[10001];
+    cudaMemcpyFromSymbol(rp, FOM_OP_T0_STATE_ROWPTR, sizeof(rp));
+    fom_int nnz = rp[10000];
+    fom_int *col = (fom_int *)malloc((size_t)(nnz + 1) * sizeof(fom_int));
+    fom_float *val = (fom_float *)malloc((size_t)(nnz + 1) * sizeof(fom_float));
+    cudaMemcpyFromSymbol(col, FOM_OP_T0_STATE_COLIDX, (size_t)nnz * sizeof(fom_int));
+    cudaMemcpyFromSymbol(val, FOM_OP_T0_STATE_VALS, (size_t)nnz * sizeof(fom_float));
+    fom_tiled_pack(&t0s_tl, 10000, rp, col, val, 0, 10000, 27, 30);
+    free(col); free(val);
+}
+
 extern "C" void fom_apply_t0_state(const fom_float *x, fom_float *out) {
-    fom_apply_t0_state_kern<<<1250, FOM_OPS_THREADS>>>(x, out);
-}
-
-static __global__ void fom_apply_t0_state__d_gamma_kern(const fom_float *x, fom_float *out) {
+    if (!t0s_init) fom_t0_state_pack();
+    if (t0s_tl.ok) fom_tiled_launch(&t0s_tl, 0, x, out);
+    else fom_apply_t0_state_csr_kern<<<1250, FOM_OPS_THREADS, 0, cudaStreamPerThread>>>(x, out);
+}
+
+static __global__ void fom_apply_t0_state__d_gamma_csr_kern(const fom_float *x, fom_float *out) {
     /* One warp per row: lanes stride the row (coalesced), then a shuffle sum. */
     fom_int i = (blockIdx.x * blockDim.x + threadIdx.x) / 32;
     fom_int lane = threadIdx.x % 32;
@@ -35,8 +395,28 @@
     if (lane == 0) out[i] += s;
 }
 
+
+static fom_tiled t0g_tl;
+static int t0g_init = 0;
+
+static void fom_t0_state__d_gamma_pack(void)
+{
+    t0g_init = 1;
+    fom_int rp[10001];
+    cudaMemcpyFromSymbol(rp, FOM_OP_T0_STATE__D_GAMMA_ROWPTR, sizeof(rp));
+    fom_int nnz = rp[10000];
+    fom_int *col = (fom_int *)malloc((size_t)(nnz + 1) * sizeof(fom_int));
+    fom_float *val = (fom_float *)malloc((size_t)(nnz + 1) * sizeof(fom_float));
+    cudaMemcpyFromSymbol(col, FOM_OP_T0_STATE__D_GAMMA_COLIDX, (size_t)nnz * sizeof(fom_int));
+    cudaMemcpyFromSymbol(val, FOM_OP_T0_STATE__D_GAMMA_VALS, (size_t)nnz * sizeof(fom_float));
+    fom_tiled_pack(&t0g_tl, 10000, rp, col, val, 1, 5000, 13, 20);
+    free(col); free(val);
+}
+
 extern "C" void fom_apply_t0_state__d_gamma(const fom_float *x, fom_float *out) {
-    fom_apply_t0_state__d_gamma_kern<<<1250, FOM_OPS_THREADS>>>(x, out);
+    if (!t0g_init) fom_t0_state__d_gamma_pack();
+    if (t0g_tl.ok) fom_tiled_launch(&t0g_tl, 1, x, out);
+    else fom_apply_t0_state__d_gamma_csr_kern<<<1250, FOM_OPS_THREADS, 0, cudaStreamPerThread>>>(x, out);
 }
 
 static __global__ void fom_apply_t0_p_p0_kern(const fom_float *x, fom_float *out) {
@@ -48,7 +428,7 @@
 }
 
 extern "C" void fom_apply_t0_p_p0(const fom_float *x, fom_float *out) {
-    fom_apply_t0_p_p0_kern<<<40, FOM_OPS_THREADS>>>(x, out);
+    fom_apply_t0_p_p0_kern<<<40, FOM_OPS_THREADS, 0, cudaStreamPerThread>>>(x, out);
 }
 
 static __global__ void fom_apply_t1_state_kern(const fom_float *x, fom_float *out) {
@@ -60,7 +440,7 @@
 }
 
 extern "C" void fom_apply_t1_state(const fom_float *x, fom_float *out) {
-    fom_apply_t1_state_kern<<<157, FOM_OPS_THREADS>>>(x, out);
+    fom_apply_t1_state_kern<<<157, FOM_OPS_THREADS, 0, cudaStreamPerThread>>>(x, out);
 }
 
 static __global__ void fom_apply_t1_state__d_gamma_kern(const fom_float *x, fom_float *out) {
@@ -72,10 +452,10 @@
 }
 
 extern "C" void fom_apply_t1_state__d_gamma(const fom_float *x, fom_float *out) {
-    fom_apply_t1_state__d_gamma_kern<<<157, FOM_OPS_THREADS>>>(x, out);
-}
-
-static __global__ void fom_apply_t1_t_t0_kern(const fom_float *x, fom_float *out) {
+    fom_apply_t1_state__d_gamma_kern<<<157, FOM_OPS_THREADS, 0, cudaStreamPerThread>>>(x, out);
+}
+
+static __global__ void fom_apply_t1_t_t0_csr_kern(const fom_float *x, fom_float *out) {
     /* One warp per row: lanes stride the row (coalesced), then a shuffle sum. */
     fom_int i = (blockIdx.x * blockDim.x + threadIdx.x) / 32;
     fom_int lane = threadIdx.x % 32;
@@ -86,8 +466,123 @@
     if (lane == 0) out[i] += s;
 }
 
+
+/* Sliced ELL: T1T_G lanes per row, slot t of lane l of row i stored at
+   (t * rows + i) * T1T_G + l, so a warp reads 32 consecutive entries. */
+#define T1T_G 4
+static fom_float *t1t_val = 0;
+static uint16_t *t1t_c16 = 0;
+static fom_int *t1t_len = 0;
+static fom_int t1t_base = 0, t1t_w = 0;
+static size_t t1t_win = 0;  /* L2 persisting window over the ELL values and offsets */
+static float t1t_hit = 0;
+static int t1t_init = 0;
+
+static void fom_t1_t_t0_pack(void)
+{
+    t1t_init = 1;
+    const fom_int n = 40000;
+    fom_int *rp = (fom_int *)malloc((n + 1) * sizeof(fom_int));
+    cudaMemcpyFromSymbol(rp, FOM_OP_T1_T_T0_ROWPTR, (n + 1) * sizeof(fom_int));
+    fom_int nnz = rp[n], mx = 0;
+    for (fom_int i = 0; i < n; i++) mx = rp[i + 1] - rp[i] > mx ? rp[i + 1] - rp[i] : mx;
+    fom_int *col = (fom_int *)malloc((size_t)(nnz + 1) * sizeof(fom_int));
+    fom_float *val = (fom_float *)malloc((size_t)(nnz + 1) * sizeof(fom_float));
+    cudaMemcpyFromSymbol(col, FOM_OP_T1_T_T0_COLIDX, (size_t)nnz * sizeof(fom_int));
+    cudaMemcpyFromSymbol(val, FOM_OP_T1_T_T0_VALS, (size_t)nnz * sizeof(fom_float));
+    fom_int lo = nnz ? col[0] : 0, hi = lo;
+    for (fom_int k = 0; k < nnz; k++) { lo = col[k] < lo ? col[k] : lo; hi = col[k] > hi ? col[k] : hi; }
+    fom_int w = (mx + T1T_G - 1) / T1T_G;
+    size_t tot = (size_t)w * n * T1T_G;
+    if (hi - lo <= 65535 && tot > 0) {
+        fom_float *ev = (fom_float *)calloc(tot, sizeof(fom_float));
+        uint16_t *ec = (uint16_t *)calloc(tot, sizeof(uint16_t));
+        fom_int *ln = (fom_int *)malloc(n * sizeof(fom_int));
+        for (fom_int i = 0; i < n; i++) {
+            ln[i] = rp[i + 1] - rp[i];
+            for (fom_int k = 0; k < ln[i]; k++) {
+                size_t pos = ((size_t)(k / T1T_G) * n + i) * T1T_G + k % T1T_G;
+                ev[pos] = val[rp[i] + k];
+                ec[pos] = (uint16_t)(col[rp[i] + k] - lo);
+            }
+        }
+        /* values and column offsets in one allocation, so one L2 window covers both */
+        char *blob = 0;
+        if (cudaMalloc((void **)&blob, tot * (sizeof(fom_float) + sizeof(uint16_t))) == cudaSuccess) {
+            cudaMemcpy(blob, ev, tot * sizeof(fom_float), cudaMemcpyHostToDevice);
+            cudaMemcpy(blob + tot * sizeof(fom_float), ec, tot * sizeof(uint16_t), cudaMemcpyHostToDevice);
+        }
+        t1t_val = (fom_float *)blob;
+        t1t_c16 = blob ? (uint16_t *)(blob + tot * sizeof(fom_float)) : 0;
+        t1t_len = (fom_int *)fom_dev_upload(ln, n * sizeof(fom_int));
+        if (!t1t_val || !t1t_c16 || !t1t_len) t1t_val = 0;
+        int pdev = 0, pmax = 0, wmax = 0;
+        cudaGetDevice(&pdev);
+        cudaDeviceGetAttribute(&pmax, cudaDevAttrMaxPersistingL2CacheSize, pdev);
+        cudaDeviceGetAttribute(&wmax, cudaDevAttrMaxAccessPolicyWindowSize, pdev);
+        if (t1t_val && pmax > 0 && wmax > 0) {
+            cudaDeviceSetLimit(cudaLimitPersistingL2CacheSize, (size_t)pmax);
+            size_t lim = 0;
+            cudaDeviceGetLimit(&lim, cudaLimitPersistingL2CacheSize);
+            size_t vb = tot * (sizeof(fom_float) + sizeof(uint16_t));
+            t1t_win = vb < (size_t)wmax ? vb : (size_t)wmax;
+            /* 30% of its lines; the t0 operators hold the rest of the carve-out */
+            size_t keep = t1t_win / 100 * 30;
+            keep = keep < lim ? keep : lim;
+            t1t_hit = (float)keep / (float)t1t_win;
+        }
+        t1t_base = lo;
+        t1t_w = w;
+        free(ev); free(ec); free(ln);
+    }
+    free(rp); free(col); free(val);
+}
+
+/* One thread per (row, lane); a warp reads 32 consecutive slots. */
+static __global__ void fom_apply_t1_t_t0_ell_kern(const fom_float *x, fom_float *out, const fom_float *__restrict__ ev,
+                                                  const uint16_t *__restrict__ ec, const fom_int *__restrict__ ln,
+                                                  fom_int w) {
+    fom_int g = blockIdx.x * blockDim.x + threadIdx.x;
+    if (g >= 40000 * T1T_G) return;  /* whole warps (40000 * T1T_G % 32 == 0) */
+    fom_int i = g / T1T_G;
+    int l = threadIdx.x % T1T_G;
+    const size_t step = (size_t)40000 * T1T_G;
+    fom_int len = ln[i];
+    fom_float s = 0.0;
+    size_t pos = (size_t)g;
+#pragma unroll 8
+    for (fom_int t = 0; t < w; t++, pos += step) {
+        fom_float v = ev[pos];  /* padding slots hold 0 / column 0 */
+        fom_float xv = __ldg(x + ec[pos]);
+        if (t * T1T_G + l < len) s += v * xv;
+    }
+    for (int o = T1T_G / 2; o > 0; o /= 2) s += __shfl_down_sync(0xffffffff, s, o, T1T_G);
+    if (l == 0) out[i] += s;
+}
+
 extern "C" void fom_apply_t1_t_t0(const fom_float *x, fom_float *out) {
-    fom_apply_t1_t_t0_kern<<<5000, FOM_OPS_THREADS>>>(x, out);
+    if (!t1t_init) fom_t1_t_t0_pack();
+    if (t1t_val) {
+        cudaLaunchConfig_t cfg;
+        memset(&cfg, 0, sizeof(cfg));
+        cfg.gridDim = dim3((40000 * T1T_G + FOM_OPS_THREADS - 1) / FOM_OPS_THREADS);
+        cfg.blockDim = dim3(FOM_OPS_THREADS);
+        cfg.stream = cudaStreamPerThread;
+        cudaLaunchAttribute at[1];
+        if (t1t_win) {
+            memset(at, 0, sizeof(at));
+            at[0].id = cudaLaunchAttributeAccessPolicyWindow;
+            at[0].val.accessPolicyWindow.base_ptr = t1t_val;
+            at[0].val.accessPolicyWindow.num_bytes = t1t_win;
+            at[0].val.accessPolicyWindow.hitRatio = t1t_hit;
+            at[0].val.accessPolicyWindow.hitProp = cudaAccessPropertyPersisting;
+            at[0].val.accessPolicyWindow.missProp = cudaAccessPropertyStreaming;
+            cfg.attrs = at;
+            cfg.numAttrs = 1;
+        }
+        cudaLaunchKernelEx(&cfg, fom_apply_t1_t_t0_ell_kern, x + t1t_base, out, (const fom_float *)t1t_val,
+                           (const uint16_t *)t1t_c16, (const fom_int *)t1t_len, t1t_w);
+    } else fom_apply_t1_t_t0_csr_kern<<<5000, FOM_OPS_THREADS, 0, cudaStreamPerThread>>>(x, out);
 }
 
 static __global__ void fom_apply_su_state_kern(const fom_float *x, fom_float *out) {
@@ -99,7 +594,7 @@
 }
 
 extern "C" void fom_apply_su_state(const fom_float *x, fom_float *out) {
-    fom_apply_su_state_kern<<<313, FOM_OPS_THREADS>>>(x, out);
+    fom_apply_su_state_kern<<<313, FOM_OPS_THREADS, 0, cudaStreamPerThread>>>(x, out);
 }
 
 static __global__ void fom_apply_su_t_t1_kern(const fom_float *x, fom_float *out) {
@@ -111,5 +606,5 @@
 }
 
 extern "C" void fom_apply_su_t_t1(const fom_float *x, fom_float *out) {
-    fom_apply_su_t_t1_kern<<<313, FOM_OPS_THREADS>>>(x, out);
-}
+    fom_apply_su_t_t1_kern<<<313, FOM_OPS_THREADS, 0, cudaStreamPerThread>>>(x, out);
+}
```

## Agent notes

```
### Round 1

**Whole-solve time:** 37,927 µs → 19,927 µs mean (`make bench`), a **1.90x** speedup. `make verify` gives VERIFY OK, worst error 1.6e-8 (tol 1e-5).

**Final profile breakdown.** Profile build: plain loop with a device sync per region, so 26.2 ms per solve. 200 iterations per solve, 20 solves.

| Region | Time per call | Share |
|---|---|---|
| t0_d_gamma (`fom_apply_t0_state__d_gamma`) | 42.8 µs | 33.2% |
| t0_state (`fom_apply_t0_state`) | 38.8 µs | 30.0% |
| t1_t_t0 (`fom_apply_t1_t_t0`) | 22.7 µs | 17.6% |
| t1_small (t1_state + t1_state__d_gamma) | 15.7 µs | 12.2% (mostly sync and launch overhead) |
| act_update (soft-threshold, shift, next prep) | 8.9 µs | 6.9% |
| residual | 15 µs, once per solve | 0.06% |

In the production graph path an iteration takes about 100 µs. About 98 µs of that is the three large matvecs.

**Optimizations applied:**
1. **Column-tiled CSR with x in shared memory (`t0_state`, `d_gamma`).**
   - Structure exploited: each operator touches a contiguous column range of at most 40000 columns, and rows have sorted columns.
   - Columns are split into tiles whose slice of x fits in shared memory. Column indices become 16-bit offsets, and entries are stored tile-major.
   - Each block stages its x slice; 16 lanes work on each row segment. The tile partials are added in a fixed order by the last block of each row chunk, so results are deterministic.
   - The gathers had been the bottleneck: x is 320 KB, larger than L1.
2. **Paired layout for `t0_state__d_gamma`.**
   - Structure exploited: each row is [left | right], with right column = left column + 40000. Each right value is −(left value) up to 12 ulps (deltas lie in [−12, 4]).
   - One 16-bit index serves both entries. The right value is stored as an int8 bit-pattern delta and rebuilt bit-exactly on the device.
   - Bytes per pair drop from 24 to 11, while still applying exactly M.
   - Each layout is built lazily, on the routine's first call, from its own data, and checked there; any mismatch falls back to the original CSR kernel.
3. **Sliced ELL for `t1_t_t0`.**
   - Structure exploited: 40000 rows of 64–65 nonzeros over 10000 columns.
   - Four lanes per row, each warp reading 32 consecutive slots, with 16-bit column indices.
   - Its value array is kept in the persisting part of L2 through a per-launch access-policy window: 26 → 22 µs.
4. **Loop-invariant parameter term hoisted.**
   - Structure exploited: `t0_p_p0 @ p` does not change across iterations.
   - It is applied once per solve into `c0`, which then seeds the accumulator every iteration.
5. **Gamma folded into the input.**
   - Structure exploited: linearity, γ·(D s) = D(γ s).
   - The γ-scaled products accumulate straight into the main accumulators, which removes two buffers, their memsets and an activation pass. They are still skipped when γ = 0.
6. **Fused state update and next-iteration prep.**
   - Structure exploited: the state update is a pure shift plus copy (`su_state` and `su_t_t1` are selection matrices).
   - One kernel writes ns = [s1, soft(a1)] and also seeds a0 = c0, a1 = 0 and gs = γ_next·ns for the next iteration.
   - Since `FOM_T0_LAMBDA = 0`, the t0 soft-threshold is skipped; it is the identity in this sum.
7. **CUDA graph of the whole fixed-iteration loop.**
   - Structure exploited: with tol = 0 the solve always runs exactly `max_iter` iterations with a fixed γ schedule.
   - All launches name `cudaStreamPerThread` explicitly; the legacy stream cannot be captured. The loop, plus the final residual reduction, is captured once on the second solve, after the layouts are built, and replayed afterwards: 21.4 → 19.9 ms.
   - Solves with a tolerance and profile builds use the plain loop.

**Blocked by rules:**
- Store A once and apply it in `t0_state`, `t0_state__d_gamma` (≈[−A, A]) and `t1_t_t0` (= −step·Aᵀ) | each `fom_apply_<name>` may read only its own matrix's data | ~2x less matrix traffic per iteration (~82 MB → ~40 MB), roughly 30–40 µs/iter
- Fuse `t0_state` and `t0_state__d_gamma` into one pass computing A(s1 + γ(s1 − s0)) | routine reads only its own matrix data, and the solver may not read matrix data | ~35–40 µs/iter (about one full A pass)
- Reuse A·x_k from iteration k's `t0_state` as the A·x_prev half of iteration k+1's `d_gamma` | the solver may not rely on relations between matrices it cannot read, and each apply must be called as-is | ~20 µs/iter
- Fuse `t1_state`, `t1_state__d_gamma` and the soft-threshold into the `t1_t_t0` kernel | every product must go through its own checked `fom_apply_<name>`; no fused routine is declared in the config | ~5–8 µs/iter
- Run independent applies on separate graph branches (for example, `t1_state`/`t1_state__d_gamma` alongside `t0_state`, or `t0_state` alongside `d_gamma` with separate accumulators) | the `fom_apply_<name>` signatures in `include/` have no stream argument, so every op is hardwired to one stream | 1–8 µs/iter (measured in a two-stream test)

### Round 2

**Whole-solve time (`make bench` mean_us):** 19849.913 us at the start of this round, 17550 us final (17553 and 17545 in two runs). That is a 1.13x speedup this round and 2.16x against the translated solver (37917 us). `make verify`: VERIFY OK, worst max-abs error 1.567e-08 (tol 1e-05).

**Final profiling breakdown.** The `FOM_PROFILE` build runs the sequential, synchronized path, because the graph is disabled under profiling. Region, total, share:
- t0_d_gamma (paired `[A', -A'']` tiled matvec): 702.1 ms, 35.0%
- t0_state (A on s1, tiled): 565.2 ms, 28.2%
- t1_t_t0 (ELL matvec, L2-persisting): 352.9 ms, 17.6%
- t1_small (`t1_state` + `t1_state__d_gamma`): 244.7 ms, 12.2%
- act_update (soft-threshold, shift, reseed): 138.7 ms, 6.9%
- residual: 1.2 ms, 0.1%

In the production graph (nsys), one iteration takes about 89 us:
- **Concurrent region, about 66 us:** `t0_state`, `t0_d_gamma` and the `t1` small kernels run side by side.
- **Combine:** 1.8 us.
- **ELL:** 17.2 us.
- **act_shift:** 3.3 us.

**Optimizations applied:**
1. **Concurrent DAG for each iteration (about 11% of the gain).**
   - *Structure exploited:* the three products that read the state are independent of each other. These are `A s → a0`, `D(γ s) → a0b` and `A1 s + D1(γ s) → a1`.
   - *How it works:* the apply routines are hard-wired to `cudaStreamPerThread`. So each apply is captured into its own small graph, and the pieces are added as child-graph nodes with explicit dependencies. That lets the kernels overlap on the GPU, and the `t1` small kernels are now completely hidden.
   - *Why results are unchanged:* a small `k_comb_soft` kernel computes `a0 = (c0 + A s) + D(γ s)`. That is the same summation order as the sequential accumulation, so rounding is bit-identical. `a0b` is reset inside `k_act_shift`, so this adds no extra kernel.
2. **Vectorized x-tile staging plus SM split (about 0.5%, near noise).**
   - *Structure exploited:* each tile's x slice is contiguous and 16-byte aligned (tile widths 10000 and 5000 are even). So the shared-memory fill uses unrolled `double2` loads.
   - *SM split:* the row chunks of the two tiled kernels were cut from 54/27 to 27/13, so the two kernels share the SMs instead of queueing behind each other's single full wave.

**Tried and reverted (measured slower or no gain):**
- **L2 access-policy windows on the tiled operators** (Persisting share for `t0`, Normal/Streaming split for the paired operator): 18478 us. The window marks random lines rather than a fixed address set, so it churned the ELL's persisting set.
- **Keeping a fixed slice of `d_gamma` in L2, the rest evict-first:** 21374 us with `__ldcs`, 19322 us with `createpolicy` + `L2::cache_hint`.
- **16-byte entry-pair loads (even-padded segments) with `__launch_bounds__(1024,2)`:** 17713 us, no gain.
- **More rows in flight per thread:** it pushed registers above 32 and halved occupancy.
- **L2 prefetch of each block's range before the fill:** slower.
- **ELL lane and unroll variants:** the current G=4, unroll 8 was already best.

ncu shows the tiled kernels are latency/occupancy-bound: DRAM at about 35% of peak, issue slots 16% busy. The 32-register cap needed for two 1024-thread blocks per SM limits how many bytes each SM can have in flight.

Blocked by rules:
- Merge `t0_state` (A on s1) and `t0_state__d_gamma` (`[A', -A'']` on γs, same sparsity pattern, values negated up to a few ulps) into one pass over A computing A((1+γ)s1 − γ s0) | kernel rule: each `fom_apply_<name>` reads only its own matrix's data, and the solver must call every apply | about 26 MB/iter less HBM traffic, est. 25-30% of solve time
- Fold the `a0 + a0b` combine into the `t1_t_t0` matvec's x read (removes a 1.8 us serial kernel per iteration) | fixed `fom_apply_<name>(x, out)` signature with a single x; no matvec loops allowed in `fom_solver.cu` | about 2%
- Merge `t1_state` and `t1_state__d_gamma` (identity / ±1 two-entry rows) into one kernel, or into the activation kernel | kernel rule: one matrix per apply; the solver must not read `FOM_MAT` data | now about 0 (already hidden by concurrency), about 9 us/iter in the sequential path
- Launch apply kernels on caller-chosen streams instead of capturing each apply separately | fixed `fom_apply` signatures, and only the `fom_apply_*` entry points may be non-static in `fom_ops.cu` | about 0 (worked around with child graphs; only adds setup work)

### Round 3

**Whole-solve `mean_us`:** 17,534.75 us at the start of this round → **15,606 us** final (`make bench`: 15,608.2 and 15,603.6). That is **1.124x** this round, and 37,917 → 15,606 = **2.43x** over the translated solver. `make verify` passes: worst error 1.567e-8 (tol 1e-5), and every per-operator check is unchanged.

**Profile of the final solver** (`-DFOM_PROFILE`; this build runs the non-graph path with a device sync per region, so absolute times are inflated: 23.6 ms/solve):
| region | total_us | per-iteration us | % |
|---|---|---|---|
| t0_d_gamma (D_gamma, paired [-A, A]) | 638,046 | 37.5 | 31.9 |
| t0_state (A s1) | 555,907 | 32.7 | 27.8 |
| t1_t_t0 (-step A^T t0) | 385,886 | 22.7 | 19.3 |
| t1_small (t1_state + t1_state__d_gamma) | 266,494 | 15.7 | 13.3 |
| act_update (soft-threshold, shift, resets) | 152,583 | 9.0 | 7.6 |
| residual | 1,522 | (once per solve) | 0.1 |

In the production CUDA graph, t0_state and d_gamma run as concurrent branches, and the small t1 terms run beside them. ncu shows the two big tiled kernels at about 45% of DRAM peak, mostly stalled waiting on global loads.

**Optimizations applied:**
1. **Batched loads in the column-tiled sparse kernel** (`fom_tiled_kern`, used by t0_state and t0_state__d_gamma). ncu showed the kernel waiting on a serial chain per row: segment bound, then entries, then shuffle. Each 16-lane group now handles 2 rows at a time and issues all their entry loads (5 per lane for t0, 3 for d_gamma) before any multiply-add. Each lane still adds its entries in the same order, so results are bit-identical. Structure used: every row of A has about 260 entries (about 65 per 10,000-column tile, about 32 per 5,000-column pair tile), so one batch covers a whole row segment. Effect: about 79.5 → 72.8 us per iteration in the microbenchmark; real solve 17,848 → 15,956 us.
2. **Interleaved x pairs for d_gamma.** Each d_gamma row is a pair: the right column is the left column + 40000, with the right value = minus the left value plus an int8 bit-pattern delta. The staged x slice now stores `{x[c], x[c+shift]}` next to each other, so each pair needs one 16-byte shared-memory read instead of two random 8-byte reads. d_gamma 27.1 → 25.7 us; solve about −0.8%.
3. **Pinned workspace.** `fom_setup` page-locks the `FomWorkspace` once (`fom_cleanup` releases it), so the per-solve state, parameter and solution transfers are direct DMA. All transfers are queued on the iteration stream with a single host wait, and the solution is copied from device straight into both `state` and `solution.x` (no host copy loop). About −1.1%.

**Tried and reverted (each measured worse in a head-to-head run):**
- Specialized kernels for t1_state / t1_state__d_gamma: their rows are shifted diagonals, so the kernel needs no indices. They were +1.5% slower.
- Moving those small terms off the t0 ‖ d_gamma phase: +2%.
- A rewritten t1_t_t0 kernel with x staged in shared memory: faster alone, but it loses the L2-persisting window that keeps this operator's 22 MB of values in L2 across iterations.
- A column-major ELL layout for t1_t_t0, and a wider persisting window over its indices too: no gain.
- Retuning the tiled kernel: wider column tiles, other row-chunk counts, 512 or 256-thread blocks, a 32-register cap (spills), loading segment bounds or the first batch before staging x, L2 prefetch hints (they evict t1_t_t0's persisted lines). All slower.
- Running t0 and d_gamma sequentially: slower than concurrently.
- Streaming each block's entries into shared memory with `cp.async` (two stages): correct, but limited by instruction count and shared-memory traffic (2.5x the instructions); t0 took 29 us vs 20 us.

**Blocked by rules:**
- Fused kernel reading A once for t0_state and both halves of t0_state__d_gamma (d_gamma is [-A, A] up to a few ulps of t0_state's values) | kernel rule: each `fom_apply_<name>` reads only its own matrix's data | about 24 MB less DRAM traffic per iteration (55 → 31 MB), roughly 10–15 us/iteration (~15%)
- Let t1_t_t0 (= -step * A^T) reuse t0_state's packed A in a transposed sweep instead of its own 27 MB copy | kernel rule: own-data only | unknown; it would free about 22 MB of the 25 MB L2 persisting budget for t0/d_gamma
- Get d_gamma's product from A s1 and A s0 in the solver instead of a third full product | kernel rule: the solver must call `fom_apply_*` and cannot rely on coefficient relationships | about 25 us/iteration (~30%)
- Fold `k_comb_soft` (a0 += a0b) into the d_gamma kernel's final per-row reduction | kernel rule: an apply must compute exactly out += M x | about 3 us/iteration (~4%)
- Store values as float32 plus an int16 correction (6 bytes per value instead of 8; about 1e-14 relative error per entry) | kernel rule: each apply must apply exactly M | about 20% fewer matrix bytes, unknown net gain

### Round 4

**Whole-solve time:** translated 37917 us; start of round 4 15606 us; final 14812–14888 us across runs (last `make bench`: 14888 us). That is 2.55x over the translation and about 5% better than round 3. `make verify`: VERIFY OK, worst error 1.567e-08 (tol 1e-05).

**Profile of the final solver.** This comes from the `-DFOM_PROFILE` build, which runs the unbatched path with a device sync per region, so it is 22.6 ms per solve. Production replays a single CUDA graph.

| region | total_us | calls | pct |
|---|---|---|---|
| t0_d_gamma (gamma-scaled product, gamma·[-A, A] paired) | 619717 | 17800 | 30.9% |
| t0_state (A·s1) | 508706 | 17800 | 25.4% |
| t1_t_t0 (step·A^T·t0, sliced ELL) | 455037 | 17800 | 22.7% |
| t1_small (identity-like t1_state and d_gamma) | 262621 | 17800 | 13.1% |
| act_update (soft-threshold, shift, re-seed) | 154887 | 17800 | 7.7% |
| residual | 1674 | 89 | 0.1% |

**Optimizations applied this round**
- **Shared the L2 persisting space across all three big matrices.** The structure exploited is that one iteration streams about 82 MB of matrix data, and the same bytes are read every iteration, against a 40 MB L2 with a 25 MB persisting carve-out.
  - Before, only the A^T values had a window, holding the whole carve-out.
  - Now each matrix gets a window that marks a random fraction of its lines as persisting: 30% of the A^T values plus column offsets (stored in one allocation so one window covers both), 30% of A, and 20% of the paired gamma-product matrix.
  - The ratio is computed from integer byte counts, so no new float literals.
  - Gain: about 4.4% from splitting the carve-out, and about 1% more from covering the A^T column offsets.
  - Contiguous windows, other splits, and a bigger A^T share were all slower.

**Tried and rejected (all measured slower, or no better)**
- **Tiled-kernel parameters.** Smaller column tiles with more blocks, and 256 or 512 threads per block, were 10–200% slower.
- **x reads.** Reading x through L1 instead of staging it in shared memory, and prefetching the block's entry range into L2, were both slower.
- **Load pipelining.** Hoisting the first batch's loads above the x staging and prefetching segment bounds one batch ahead caused register spills (+15%).
- **Warp-per-row CSR kernel** for A: 40 us vs 29 us.
- **Running the two t0 products in sequence** into one accumulator to drop the combine kernel: +3.5%. Running them concurrently hides each kernel's tail.
- **Graph restructuring.** Moving the tiny t1 products next to the A^T product into a separate buffer added a graph node, costing about 3 us per iteration. Putting them before the big kernels was +4–8%.
- **Re-chunking the t0 products** so the pair fills exactly one wave: +1–13%.

Blocked by rules:
- Reuse the previous iteration's A·s1 as the s0 half of the gamma product, since the d_gamma left block is -A up to a few ulps (would remove about 14 MB per iteration, roughly 25% of the time) | solver must apply each matrix through its own `fom_apply_<name>`, and the matrices are not bitwise equal | ~20–25%
- One fused kernel streaming A and the paired gamma matrix together (shared x staging, one launch, no combine kernel) | each `fom_apply_<name>` and its helpers may read only its own matrix's data | ~5–10%
- Fold the a0 + a0b combine into the A^T kernel's input load | `fom_apply_t1_t_t0` must compute out += M·x for the x it is given | ~2–3%
- Fixed persisting hit ratios written as literals (0.3, 0.2) | no new floating-point constants | none (worked around with integer byte counts)

### Round 5

**Result:** no improvement found in round 5, so the sources are unchanged from round 4. `make verify` prints VERIFY OK.
- Translated solver: 37917 us. Start of round 5: 14862 us. Final: 14757 us (`make bench` mean_us; the difference is run-to-run noise, and the GPU host was now an A100-SXM4).
- Overall speedup vs. the translation: about 2.57x.

**Profile of the final solver** (`-DFOM_PROFILE`, which syncs after every region, so the regions run one after another; production replays one CUDA graph in which `t0_state`, `t0_d_gamma` and `t1_small` run concurrently):

| region | time per call | share |
|---|---|---|
| t0_d_gamma | 34.8 us | 30.5% |
| t0_state | 29.3 us | 25.8% |
| t1_t_t0 | 25.9 us | 22.7% |
| t1_small | 14.9 us | 13.1% |
| act_update | 8.9 us | 7.8% |
| residual | n/a | 0.1% |

**Tried this round, all reverted** (each passed verify but was slower or no faster):
- **Dictionary/split-CSR `t0_state`:** 18.7 ms (+25%). The idea came from the stale `data.npz`, where one block of the matrix has only 78 distinct values. The device header has no such block, so the split kernel just lost to the existing column-tiled kernel.
- **Per-tile value dictionary in the tiled kernel:** 16.0 ms; the dictionary itself made no difference.
- **Re-tuning the L2 persistence split, chunk counts and tile widths on the new GPU:** every setting was within about 1% of the current one or worse.
- **Moving the small `t1` branch in the graph:** making it serial costs about 7 us per iteration. So each kernel on the critical path costs about 3.5 us, and the two small ones (combine and activation) cost about 7 us per iteration.
- **Flattening the graph** with `cudaStreamBeginCaptureToGraph` instead of child-graph nodes: no change.
- **`t1_t_t0` with all of x staged in shared memory:** 15.1 to 18.3 ms. Staging x up front costs more than the faster gathers save.

**Optimizations in effect (from rounds 1 to 4):**
- The whole fixed 200-iteration loop is captured once as a CUDA graph, with one readback of the residual. This is valid because termination is max_iters only.
- `t0_state` and `t0_d_gamma` run as concurrent graph branches and are added in a fixed order, so results stay deterministic.
- `t0_d_gamma` uses a paired layout. Each row is [left | right] with right column = left column + 40000 and right value = -left value plus a few ulps, stored exactly as a one-byte delta.
- Both `t0` products use a column-tiled kernel with 16-bit column offsets and x staged in shared memory.
- `t1_t_t0` (65 nonzeros per row) uses a sliced ELL layout.
- The 25 MB persisting L2 region is split across the three big operators (30%, 20% and 30%).
- `P p` is computed once per solve, `gamma * s` is folded into the activation/shift kernel, and the workspace is page-locked for the host transfers.

Blocked by rules:
- Reuse `A s1` from the previous iteration to replace most of the `t0_state__d_gamma` product. That product equals `gamma*(A s1_k - A' s1_{k-1})` with `A'` within a few ulps of `A`, and `A s1_{k-1}` is the previous iteration's `t0_state` product. | The solver may not read matrix data to discover identities between operators, and each `fom_apply_<name>` must apply exactly its own matrix | about 25-35% (removes about 28.6 MB of matrix reads per iteration)
- Fuse `t0_state` and `t0_d_gamma` into one kernel that shares the column indices and the staged x slice, since both read the same `A` sparsity pattern. | Each `fom_apply_<name>` and its helpers may read only its own matrix's data | about 10-20% (one kernel node and about 5 MB of index reads less per iteration)
- Drop the combine kernel by having both `t0` products accumulate into one buffer with `atomicAdd`. | Not a hard rule: the target notes require reductions in a fixed order for deterministic results, and this would make the result's last bits depend on which kernel finishes first | about 4-5% (about 3.5 us per iteration)
- Apply the soft-threshold and state shift inside `fom_apply_t1_t_t0`'s kernel, saving one kernel node. | `fom_apply_<name>` must compute exactly `out += M x`; activations belong in `src/fom_solver.cu` | about 4-5% (about 3.5 us per iteration)
```
