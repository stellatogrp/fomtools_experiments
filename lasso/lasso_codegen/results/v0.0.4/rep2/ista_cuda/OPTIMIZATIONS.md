# Optimization report

## Timing

```
translated (emitted)   18153.3 us/solve
optimized               7368.3 us/solve
speedup                  2.46x
```

Optimization outcome: **kept (re-verified correct)**.

## Rounds

| round | before (us) | after (us) | gain | kept | agent (s) | outcome |
|---:|---:|---:|---:|:---:|---:|---|
| 1 | 18153.334 | 13307.455 | 26.7% | yes | 4496 | continued |
| 2 | 13335.400 | 8248.681 | 38.1% | yes | 4457 | continued |
| 3 | 8092.589 | 7368.322 | 8.9% | yes | 4456 | continued |
| 4 | - | - | - | no | 3868 | the round left the sources unchanged |

## Kernel checks

Each `fom_apply_<name>` in the shipped `fom_ops.c`, checked by `make verify` against the package's reference over the same `fom_data.c` matrix. *basis* probes every unit vector, which recovers the routine's matrix column by column (an exact check), plus random vectors; *random* uses random vectors only (the matrix is over the probe budget). A `fused_<block>` row is a `fom_fused_<block>`, checked against the sum of its members: *basis* there probes every unit vector of each source with every schedule value at 0 and with each alone at 1, which recovers every member exactly.

| matrix | shape | nnz | rewritten | check | probes | max error |
|---|---|---|---|---|---|---|
| `t0_state` | 10000x40000 | 2599983 | yes | random | 2 | 4.441e-15 |
| `t0_p_p0` | 10000x10000 | 10000 | yes | random | 2 | 0.000e+00 |
| `t1_state` | 40000x40000 | 40000 | yes | random | 2 | 0.000e+00 |
| `t1_t_t0` | 40000x10000 | 2599983 | yes | random | 2 | 4.441e-16 |
| `su_t_t1` | 40000x40000 | 40000 | yes | random | 2 | 0.000e+00 |

### Rule violations

_None: the collected rewrite passed the rules and was kept._

## Blocked by rules

Optimizations the agent considered but did not make because a rule forbade them, in its own words.

| round | idea | rule | expected gain |
|---|---|---|---|
| 1 | Read `t0_state` once per iteration for both products: `t1_t_t0` equals −0.0147 · `t0_state`ᵀ (checked entry by entry), so one stored matrix could serve both | each `fom_apply_<name>` may read only its own matrix's data, and no new float constants | up to ~40% less matrix traffic, plausibly 1.3–1.6× |
| 1 | Fuse the soft-threshold, state update and accumulator reset into the `t1_t_t0` matvec's final per-row write | `fom_apply_<name>` must compute exactly `out += M x` | ~2–3 µs/iter (~4%) |
| 1 | One persistent cooperative kernel running both matvecs and the update with grid syncs, removing per-kernel tail effects | matvec loops must live inside the separate `fom_apply_*` entry points, each launched as its own kernel | unknown, likely 5–10% |
| 1 | Build or persist the derived index arrays and L2 policy in `fom_setup` instead of lazily inside the first `fom_apply` call | `fom_setup` may not call `fom_apply_*`, and `fom_ops.cu` has no other setup hook | ~0 per solve (first-call cost only) |
| 1 | Pin host buffers or overlap the per-solve upload and download | `include/` workspace layout and the bench harness are fixed | <1% |
| 2 | Store A once and use it for both products. I checked that `t1_t_t0` has exactly the pattern of `t0_state` transposed, with every value scaled by -0.014703924… (equal to within a few ulps). One 26 MB copy would nearly fit in L2 instead of streaming 52 MB per iteration | each `fom_apply_<name>` and its helpers may read only its own matrix's data, and each must apply exactly its own matrix | about 30–45% |
| 2 | Fold the soft-threshold, residual and state update into the `t1_t_t0` kernel's epilogue, removing one kernel boundary per iteration | `fom_apply_<name>` must compute exactly `out += M x`; activations belong in the solver | about 3–5% (about 2 us per iteration) |
| 2 | Run the whole iteration as one persistent kernel with grid-wide syncs, so the x slices stay in shared memory and there are no kernel-boundary drains | matrix-vector work may not live in `src/fom_solver.cu`, and the `fom_apply_*` signatures (no stream or fusion hook) are fixed | unknown, likely 5–10% |
| 3 | Read only A (t0_state) in both matvecs, since t1_t_t0 equals -0.0147·A^T exactly (checked); 23 MB would then fit in persisting L2 and DRAM traffic per iteration would roughly halve | each fom_apply_<name> may read only its own matrix's data | ~30-40% |
| 3 | Fuse the soft-threshold, the t0 = c re-arm and the residual into the epilogue of the A^T kernel, and the threshold into the x staging of the next A x kernel, removing one kernel and one full drain per iteration | fom_apply_* must compute exactly out += M x, and matrix kernels must live in fom_ops.cu, not fom_solver.cu | ~8% (removing the update kernel measured ~7.5%) |
| 3 | Run each iteration as one persistent kernel with a grid-wide sync between the A x, A^T and threshold phases (no launch gaps) | matvecs must go through the separate fom_apply_* entry points | unknown, ~5-10% |
| 3 | Programmatic dependent launch so the next matvec's matrix loads start while the previous kernel drains | target constraint: the build is fixed at sm_80 and PDL needs sm_90 | ~5% |
| 3 | Shrink matrix values further with a lossy fp32+fp16 split (8 bytes per entry) | the kernel must apply exactly M (and the 1e-10 operator check) | ~8% |
| 4 | Store `t0_state` once and apply `t1_t_t0` as its scaled transpose (here `t1_t_t0` = −step·`t0_state`ᵀ), so one matrix copy is shared and could stay in L2 | `fom_apply_<name>` and its helpers may read only their own matrix's data | ~up to 2x less matrix traffic; unknown net, since a transposed apply needs a scatter or second layout |
| 4 | One persistent fused kernel per iteration batch (t0 product → soft → t1 product with grid-wide syncs), removing 3 launches per iteration and letting staging overlap the previous phase | matvec loops must live inside each `fom_apply_<name>`, and no code may read two matrices' data | ~3–6 us/iter (8–15%) |
| 4 | Fold the soft-threshold of y into the t0 kernel's x staging, dropping the state-update kernel | each `fom_apply_<name>` must compute exactly `out += M x` for its given `x` | ~1–2 us/iter |
| 4 | Lossy value compression (float32 or truncated mantissas) to cut the 9 bytes per entry | the direct `fom_apply` checks require applying exactly `M` | ~40% less matrix traffic, unknown net |
| 4 | Programmatic dependent launch or thread-block clusters (shared x staging across SMs) | target constraint: the build flags pin sm_80, and these features need sm_90 | unknown |

## What the agent changed

```diff
--- src/fom_solver.cu (translated)
+++ src/fom_solver.cu (optimized)
@@ -3,10 +3,42 @@
 #include "fom_solver.h"
 #include "fom_data.h"
 #include "fom_ops.h"
-
-static fom_float *d_s, *d_ns, *d_c, *d_t0, *d_acc, *d_p;
+#include "fom_profile.h"
+
+/* All device work runs on the per-thread default stream (as do the
+   fom_apply_* kernels), which can be captured into a CUDA graph. */
+#define ST cudaStreamPerThread
+
+/* d_y holds the state; the t1 product accumulates into it in place
+   (t1_state and su_t_t1 are identity selections), and k_t1 thresholds it
+   there. d_old keeps the previous state when the residual is needed. */
+static fom_float *d_y, *d_old, *d_c, *d_t0, *d_p;
 static unsigned long long *d_res;
 static int d_ready = 0;
+/* The workspace is page-locked while set up, so the per-solve uploads and
+   the solution download are direct DMA transfers. */
+static FomWorkspace *g_pinned = 0;
+/* Two plain iterations, captured once after the
+   first eager solve (whose fom_apply_* calls do their one-time setup). */
+static cudaGraphExec_t g_pair;
+/* GB_PAIRS pairs in one graph, so long runs pay one graph launch per
+   2 * GB_PAIRS iterations instead of per pair. */
+#ifndef GB_PAIRS
+#define GB_PAIRS 33
+#endif
+static cudaGraphExec_t g_big;
+static int g_big_ok = 0;
+static int g_state = 0; /* 0: not captured yet, 1: ready, -1: unavailable */
+
+/* Profile builds time each region on the GPU: sync at region ends and run
+   every iteration eagerly (no graph). Production builds do neither. */
+#ifdef FOM_PROFILE
+#define PROF_SYNC() cudaStreamSynchronize(ST)
+#define USE_GRAPH 0
+#else
+#define PROF_SYNC() ((void)0)
+#define USE_GRAPH 1
+#endif
 
 __device__ __forceinline__ double soft(double v, double lam) {
     return v > lam ? v - lam : (v < -lam ? v + lam : 0.0);
@@ -18,17 +50,19 @@
     if (i < n) x[i] = soft(x[i], lam);
 }
 
-/* ns = soft(s + acc, lam) (t1_state and su_t_t1 are identity selections);
-   optionally res = max|ns - s| (nonneg doubles order like their bits). */
-__global__ void k_t1(const fom_float *s, const fom_float *acc, fom_float *ns,
-                     int n, double lam, unsigned long long *res) {
+/* y = soft(y, lam) in place (y = s + t1_t_t0 @ t0 on entry); also
+   re-arms the next iteration's t0 = c. Optionally res = max|y - old|
+   (nonneg doubles order like their bits). */
+__global__ void k_t1(fom_float *y, const fom_float *old, const fom_float *c,
+                     fom_float *t0, int m, int n, double lam, unsigned long long *res) {
     int i = blockIdx.x * blockDim.x + threadIdx.x;
     double d = 0.0;
     if (i < n) {
-        double v = soft(s[i] + acc[i], lam);
-        ns[i] = v;
-        d = fabs(v - s[i]);
-    }
+        double v = soft(y[i], lam);
+        y[i] = v;
+        if (res) d = fabs(v - old[i]);
+    }
+    if (i < m) t0[i] = c[i];
     if (res) {
         for (int o = 16; o > 0; o >>= 1) d = fmax(d, __shfl_down_sync(0xffffffffu, d, o));
         __shared__ double sm[8];
@@ -51,9 +85,8 @@
 
 static void alloc_dev(void) {
     if (d_ready) return;
-    cudaMalloc((void **)&d_s, sizeof(fom_float) * FOM_STATE_DIM);
-    cudaMalloc((void **)&d_ns, sizeof(fom_float) * FOM_STATE_DIM);
-    cudaMalloc((void **)&d_acc, sizeof(fom_float) * FOM_T1_DIM);
+    cudaMalloc((void **)&d_y, sizeof(fom_float) * FOM_STATE_DIM);
+    cudaMalloc((void **)&d_old, sizeof(fom_float) * FOM_STATE_DIM);
     cudaMalloc((void **)&d_c, sizeof(fom_float) * FOM_T0_DIM);
     cudaMalloc((void **)&d_t0, sizeof(fom_float) * FOM_T0_DIM);
     cudaMalloc((void **)&d_p, sizeof(fom_float) * FOM_PARAM_P0_DIM);
@@ -68,6 +101,11 @@
     work->solution.residual = 0;
     work->solution.status = FOM_MAX_ITER_REACHED;
     alloc_dev();
+    if (g_pinned != work) {
+        if (g_pinned) cudaHostUnregister(g_pinned);
+        g_pinned = cudaHostRegister(work, sizeof(*work), cudaHostRegisterDefault) == cudaSuccess ? work : 0;
+        cudaGetLastError();
+    }
 }
 
 void fom_update_params(FomWorkspace *work, const FomParams *params) {
@@ -75,41 +113,105 @@
     memcpy(work->state, FOM_STATE_INIT, sizeof(work->state));
 }
 
-fom_int fom_solve(FomWorkspace *work) {
+/* One iteration on d_y. Expects t0 = c on entry and leaves it so for
+   the next one (k_t1 re-arms it). */
+static void iterate(int need_res) {
     const int n = FOM_STATE_DIM, m = FOM_T0_DIM, B = 256;
     const int gn = (n + B - 1) >> 8, gm = (m + B - 1) >> 8;
     const double lam0 = FOM_T0_LAMBDA, lam1 = FOM_T1_LAMBDA;
+    FOM_PROF_BEGIN(t0_matvec);
+    fom_apply_t0_state(d_y, d_t0);
+    if (lam0 != 0.0) k_soft<<<gm, B, 0, ST>>>(d_t0, m, lam0);
+    PROF_SYNC();
+    FOM_PROF_END(t0_matvec);
+    FOM_PROF_BEGIN(t1_matvec);
+    if (need_res) {
+        cudaMemcpyAsync(d_old, d_y, sizeof(fom_float) * n, cudaMemcpyDeviceToDevice, ST);
+        cudaMemsetAsync(d_res, 0, sizeof(unsigned long long), ST);
+    }
+    fom_apply_t1_t_t0(d_t0, d_y);
+    PROF_SYNC();
+    FOM_PROF_END(t1_matvec);
+    FOM_PROF_BEGIN(state_update);
+    k_t1<<<gn, B, 0, ST>>>(d_y, d_old, d_c, d_t0, m, n, lam1, need_res ? d_res : 0);
+    PROF_SYNC();
+    FOM_PROF_END(state_update);
+}
+
+static void capture_pair(void) {
+    cudaGraph_t g;
+    g_state = -1;
+    if (cudaStreamBeginCapture(ST, cudaStreamCaptureModeThreadLocal) != cudaSuccess) { cudaGetLastError(); return; }
+    iterate(0);
+    iterate(0);
+    if (cudaStreamEndCapture(ST, &g) != cudaSuccess) { cudaGetLastError(); return; }
+    if (cudaGraphInstantiate(&g_pair, g, 0) == cudaSuccess) g_state = 1;
+    cudaGraphDestroy(g);
+    cudaGetLastError();
+    if (g_state != 1) return;
+    if (cudaStreamBeginCapture(ST, cudaStreamCaptureModeThreadLocal) != cudaSuccess) { cudaGetLastError(); return; }
+    for (int p = 0; p < GB_PAIRS; p++) {
+        iterate(0);
+        iterate(0);
+    }
+    if (cudaStreamEndCapture(ST, &g) != cudaSuccess) { cudaGetLastError(); return; }
+    g_big_ok = cudaGraphInstantiate(&g_big, g, 0) == cudaSuccess;
+    cudaGraphDestroy(g);
+    cudaGetLastError();
+}
+
+fom_int fom_solve(FomWorkspace *work) {
+    const int n = FOM_STATE_DIM, m = FOM_T0_DIM;
     alloc_dev();
-    cudaMemcpy(d_s, work->state, sizeof(fom_float) * n, cudaMemcpyHostToDevice);
-    cudaMemcpy(d_p, work->params.p0, sizeof(fom_float) * FOM_PARAM_P0_DIM, cudaMemcpyHostToDevice);
-    /* constant part of t0: t0_p_p0 @ p0 */
-    cudaMemset(d_c, 0, sizeof(fom_float) * m);
+    /* uploads stay on the stream: the host goes on to queue the iterations */
+    cudaMemcpyAsync(d_y, work->state, sizeof(fom_float) * n, cudaMemcpyHostToDevice, ST);
+    cudaMemcpyAsync(d_p, work->params.p0, sizeof(fom_float) * FOM_PARAM_P0_DIM, cudaMemcpyHostToDevice, ST);
+    /* constant part of t0: t0_p_p0 @ p0; then arm t0 = c */
+    FOM_PROF_BEGIN(setup_const);
+    cudaMemsetAsync(d_c, 0, sizeof(fom_float) * m, ST);
     fom_apply_t0_p_p0(d_p, d_c);
+    cudaMemcpyAsync(d_t0, d_c, sizeof(fom_float) * m, cudaMemcpyDeviceToDevice, ST);
+    PROF_SYNC();
+    FOM_PROF_END(setup_const);
 
     const int max_iter = work->settings.max_iter;
     const int use_tol = work->settings.check_termination && work->settings.tol > 0;
     fom_int status = FOM_MAX_ITER_REACHED;
     double res = 0.0;
+    unsigned long long r = 0;
     int k = 0;
+    /* Without a tolerance only the last iteration's residual is read:
+       run all but the last iteration (pairwise) as graph launches. */
+    if (!use_tol && g_state == 1) {
+        FOM_PROF_BEGIN(graph_iters);
+        if (g_big_ok)
+            for (; k + 2 * GB_PAIRS < max_iter; k += 2 * GB_PAIRS) cudaGraphLaunch(g_big, ST);
+        for (; k + 2 < max_iter; k += 2) cudaGraphLaunch(g_pair, ST);
+        FOM_PROF_END(graph_iters);
+    }
     while (k < max_iter) {
         int need_res = use_tol || (k == max_iter - 1);
-        cudaMemcpyAsync(d_t0, d_c, sizeof(fom_float) * m, cudaMemcpyDeviceToDevice, 0);
-        fom_apply_t0_state(d_s, d_t0);
-        if (lam0 != 0.0) k_soft<<<gm, B>>>(d_t0, m, lam0);
-        cudaMemsetAsync(d_acc, 0, sizeof(fom_float) * n, 0);
-        fom_apply_t1_t_t0(d_t0, d_acc);
-        if (need_res) cudaMemsetAsync(d_res, 0, sizeof(unsigned long long), 0);
-        k_t1<<<gn, B>>>(d_s, d_acc, d_ns, n, lam1, need_res ? d_res : 0);
-        fom_float *t = d_s; d_s = d_ns; d_ns = t;
+        iterate(need_res);
         k++;
         if (need_res) {
-            unsigned long long r;
-            cudaMemcpy(&r, d_res, sizeof(r), cudaMemcpyDeviceToHost);
-            memcpy(&res, &r, sizeof(res));
+            FOM_PROF_BEGIN(residual_readback);
+            cudaMemcpyAsync(&r, d_res, sizeof(r), cudaMemcpyDeviceToHost, ST);
+            /* without a tolerance this is the last iteration: its residual
+               comes back with the solution, under one sync below */
+            if (use_tol) {
+                cudaStreamSynchronize(ST);
+                memcpy(&res, &r, sizeof(res));
+            }
+            FOM_PROF_END(residual_readback);
             if (use_tol && res <= work->settings.tol) { status = FOM_SOLVED; break; }
         }
     }
-    cudaMemcpy(work->state, d_s, sizeof(fom_float) * n, cudaMemcpyDeviceToHost);
+    FOM_PROF_BEGIN(download);
+    cudaMemcpyAsync(work->state, d_y, sizeof(fom_float) * n, cudaMemcpyDeviceToHost, ST);
+    cudaStreamSynchronize(ST);
+    FOM_PROF_END(download);
+    memcpy(&res, &r, sizeof(res));
+    if (USE_GRAPH && !use_tol && g_state == 0) capture_pair();
     memcpy(work->solution.x, work->state, sizeof(work->state));
     work->solution.iter = k;
     work->solution.residual = res;
@@ -119,8 +221,13 @@
 
 void fom_cleanup(FomWorkspace *work) {
     (void)work;
+    if (g_pinned) { cudaHostUnregister(g_pinned); g_pinned = 0; }
     if (!d_ready) return;
-    cudaFree(d_s); cudaFree(d_ns); cudaFree(d_acc); cudaFree(d_c);
+    if (g_state == 1) cudaGraphExecDestroy(g_pair);
+    if (g_big_ok) cudaGraphExecDestroy(g_big);
+    g_state = 0;
+    g_big_ok = 0;
+    cudaFree(d_y); cudaFree(d_old); cudaFree(d_c);
     cudaFree(d_t0); cudaFree(d_p); cudaFree(d_res);
     d_ready = 0;
 }
--- src/fom_ops.cu (translated)
+++ src/fom_ops.cu (optimized)
@@ -1,27 +1,368 @@
 /* Deterministic device-side matrix-vector products (out += M @ x).
-   One warp per row for dense-enough rows, one thread per row for very
-   sparse ones. Matrix data lives in fom_ops_data.cuh as __device__ const
-   arrays. All pointers are device pointers. */
+   The two big operators use a column-tiled layout (see tiled_kern); the
+   identity ones keep one thread per row. Matrix data lives in fom_ops_data.cuh as __device__ const
+   arrays. All pointers are device pointers. Kernels launch on the
+   per-thread default stream, which orders with the legacy default stream
+   (so callers' cudaMemset/cudaMemcpy still order around them) and, unlike
+   it, can be captured into a CUDA graph by the solver. */
 #include "fom_ops.h"
 #include "fom_ops_data.cuh"
 
 #include <cuda_runtime.h>
+#include <stdlib.h>
+#include <string.h>
 
 #define FOM_OPS_THREADS 256
-
-static __global__ void fom_apply_t0_state_kern(const fom_float *x, fom_float *out) {
-    /* One warp per row: lanes stride the row (coalesced), then a shuffle sum. */
-    fom_int i = (blockIdx.x * blockDim.x + threadIdx.x) / 32;
-    fom_int lane = threadIdx.x % 32;
-    if (i >= 10000) return;  /* whole warps exit together */
+#define FOM_OPS_STREAM cudaStreamPerThread
+
+/* Column-tiled CSR for the two big random-sparse operators. A row-wise
+   kernel gathers x at random from L2, which caps t0_state (x = 320 KB)
+   near 600 GB/s. Instead the columns are split into NT tiles of TC
+   columns whose x slice (<= 80 KB) fits in shared memory, and the entries
+   are repacked tile-major, row by row, with 16-bit column offsets (built
+   on the first call from the operator's own CSR, which its fom_apply_
+   routine passes in). Block (chunk, tile) stages its x slice, then G-lane
+   groups compute per-row sums over that tile, U entries per lane and RB
+   rows per group in flight. With one tile the sums go straight to out;
+   otherwise they are per-tile partials and the last block of a chunk adds
+   them to out in tile order (deterministic).
+
+   Each entry is stored losslessly in 9 bytes instead of 10: all values
+   are normal doubles whose exponents span at most 32 values, so the
+   11-bit exponent field of a value's bits holds the exponent minus the
+   operator's smallest one (5 bits, ebase found at setup) and the low 6
+   bits of its column offset; a byte array holds the column's high bits
+   (offsets < 2^14). Decoding restores the exact double. */
+#define TL_THREADS 1024
+
+typedef struct {
+    int ok;
+    int *seg;              /* [NT * NR + 1] tile-major row segments */
+    unsigned char *chi;    /* column offset inside the tile >> 6 */
+    unsigned long long *val; /* value bits, exponent field = e - ebase | col & 63 */
+    int ebase;
+    fom_float *part;       /* [NT * NR] per-tile row partials (NT > 1) */
+    unsigned *cnt;         /* [CH] finished tiles per chunk (NT > 1) */
+    void *blob;            /* val | chi */
+    size_t bytes;          /* L2 access-policy window over blob */
+    float hit;
+} tiled_csr;
+
+template <int NR, int NC, int TC, int CH, int G, int U, int RB>
+static __global__ void __launch_bounds__(TL_THREADS, 1)
+tiled_kern(const int *__restrict__ seg, const unsigned char *__restrict__ chi,
+           const unsigned long long *__restrict__ val, const int ebase, const fom_float *x,
+           fom_float *part, unsigned *cnt, fom_float *out) {
+    constexpr int NT = (NC + TC - 1) / TC, RPB = (NR + CH - 1) / CH, NG = TL_THREADS / G;
+    extern __shared__ fom_float sx[];
+    __shared__ int last;
+    const int t = NT > 1 ? blockIdx.y : 0, c0 = t * TC;
+    const int cn = NC - c0 < TC ? NC - c0 : TC;
+    const int grp = threadIdx.x / G, l = threadIdx.x % G;
+    const int r0 = blockIdx.x * RPB, r1 = r0 + RPB < NR ? r0 + RPB : NR;
+    const int *sg = seg + t * NR;
+    int k0[RB], k1[RB], n0[RB], n1[RB];
+    unsigned long long va[RB][U];
+    unsigned cc[RB][U];
+    /* exact value and column offset of packed entry (w, h) */
+    auto dec = [&](unsigned long long w, unsigned h, int &c) {
+        unsigned hi = (unsigned)(w >> 32), f = (hi >> 20) & 0x7FFu;
+        c = (int)((f & 63u) | (h << 6));
+        hi = (hi & 0x800FFFFFu) | (((f >> 6) + (unsigned)ebase) << 20);
+        return __hiloint2double((int)hi, (int)(unsigned)w);
+    };
+    /* segment bounds of the RB rows of this group's batch at row ib */
+    auto bounds = [&](int ib) {
+#pragma unroll
+        for (int r = 0; r < RB; r++) {
+            int i = ib + grp + r * NG;
+            n0[r] = n1[r] = 0;
+            if (i < r1) { n0[r] = sg[i]; n1[r] = sg[i + 1]; }
+        }
+    };
+    /* the first U entries per lane of the batch whose bounds are in n0/n1;
+       then fetch the following batch's bounds, so the value loads never
+       wait on a segment load */
+    auto load = [&](int ib) {
+#pragma unroll
+        for (int r = 0; r < RB; r++) { k0[r] = n0[r]; k1[r] = n1[r]; }
+#pragma unroll
+        for (int r = 0; r < RB; r++) {
+#pragma unroll
+            for (int u = 0; u < U; u++) {
+                int k = k0[r] + l + u * G;
+                va[r][u] = 0;
+                cc[r][u] = 0;
+                if (k < k1[r]) { va[r][u] = val[k]; cc[r][u] = chi[k]; }
+            }
+        }
+        bounds(ib + NG * RB);
+    };
+    bounds(r0);
+    /* the first batch's loads are in flight while x is staged */
+    load(r0);
+    {
+        const double2 *x2 = (const double2 *)(x + c0);
+        double2 *s2 = (double2 *)sx;
+#pragma unroll 4
+        for (int j = threadIdx.x; j < (cn >> 1); j += TL_THREADS) s2[j] = x2[j];
+        if ((cn & 1) && threadIdx.x == 0) sx[cn - 1] = x[c0 + cn - 1];
+    }
+    __syncthreads();
+    for (int ib = r0; ib < r1; ib += NG * RB) {
+        fom_float s[RB];
+#pragma unroll
+        for (int r = 0; r < RB; r++) {
+            s[r] = 0.0;
+#pragma unroll
+            for (int u = 0; u < U; u++)
+                if (k0[r] + l + u * G < k1[r]) {
+                    int c;
+                    fom_float v = dec(va[r][u], cc[r][u], c);
+                    s[r] += v * sx[c];
+                }
+            for (int k = k0[r] + l + U * G; k < k1[r]; k += G) {
+                int c;
+                fom_float v = dec(val[k], chi[k], c);
+                s[r] += v * sx[c];
+            }
+        }
+        /* next batch's loads overlap this batch's reductions */
+        load(ib + NG * RB);
+#pragma unroll
+        for (int r = 0; r < RB; r++) {
+            for (int o = G / 2; o > 0; o /= 2) s[r] += __shfl_down_sync(0xffffffff, s[r], o, G);
+            int i = ib + grp + r * NG;
+            if (l == 0 && i < r1) {
+                if (NT == 1) out[i] += s[r];
+                else part[t * NR + i] = s[r];
+            }
+        }
+    }
+    if (NT == 1) return;
+    __threadfence();
+    __syncthreads();
+    if (threadIdx.x == 0) last = (atomicAdd(&cnt[blockIdx.x], 1u) == (unsigned)NT - 1);
+    __syncthreads();
+    if (last) {
+        __threadfence();
+        for (int i = r0 + threadIdx.x; i < r1; i += TL_THREADS) {
+            fom_float s = 0.0;
+            for (int u = 0; u < NT; u++) s += ((volatile fom_float *)part)[u * NR + i];
+            out[i] += s;
+        }
+        if (threadIdx.x == 0) cnt[blockIdx.x] = 0;
+    }
+}
+
+/* Reorder one row segment (n entries) so that each run of G consecutive
+   entries, read by the G lanes of a group at once, has distinct column
+   residues mod 16 where possible: a double at column c sits in bank pair
+   c % 16, so such a run is one conflict-free shared-memory wavefront.
+   Each run takes one entry from each residue bucket, fullest first. Only
+   the order of the row's sum changes. */
+static void bank_order(unsigned short *c, fom_float *v, int n, int G) {
+    unsigned short bc[16][256];
+    fom_float bv[16][256];
+    int cnt[16] = {0}, k = 0;
+    if (n > 16 * 256) return;
+    for (int j = 0; j < n; j++) {
+        int r = c[j] % 16;
+        if (cnt[r] == 256) return;
+        bc[r][cnt[r]] = c[j];
+        bv[r][cnt[r]] = v[j];
+        cnt[r]++;
+    }
+    while (k < n) {
+        int used[16] = {0}, run = 0;
+        while (run < G && k < n) {
+            /* fullest bucket not used in this run (any bucket if all are) */
+            int best = -1;
+            for (int r = 0; r < 16; r++)
+                if (cnt[r] > 0 && !used[r] && (best < 0 || cnt[r] > cnt[best])) best = r;
+            if (best < 0) {
+                for (int r = 0; r < 16; r++)
+                    if (cnt[r] > 0 && (best < 0 || cnt[r] > cnt[best])) best = r;
+            }
+            used[best] = 1;
+            cnt[best]--;
+            c[k] = bc[best][cnt[best]];
+            v[k] = bv[best][cnt[best]];
+            k++;
+            run++;
+        }
+    }
+}
+
+/* Repack rp/col/val (host copies of one operator's CSR, nnz entries,
+   columns sorted within each row) into T; persist `persist` percent of
+   the packed lines in L2. Leaves T->ok = 0 if anything fails. */
+template <int NR, int NC, int TC, int CH, int G, int U, int RB>
+static void tiled_pack(tiled_csr *T, const fom_int *rp, const fom_int *col, const fom_float *val,
+                       int persist) {
+    constexpr int NT = (NC + TC - 1) / TC;
+    T->ok = 0;
+    fom_int nnz = rp[NR];
+    int *seg = (int *)malloc(sizeof(int) * ((size_t)NT * NR + 1));
+    unsigned short *c16 = (unsigned short *)malloc(sizeof(unsigned short) * (size_t)nnz);
+    fom_float *v = (fom_float *)malloc(sizeof(fom_float) * (size_t)nnz);
+    fom_int *pos = (fom_int *)malloc(sizeof(fom_int) * NR);
+    int good = seg && c16 && v && pos;
+    if (good) {
+        for (int i = 0; i < NR; i++) pos[i] = rp[i];
+        fom_int q = 0;
+        for (int t = 0; t < NT; t++)
+            for (int i = 0; i < NR; i++) {
+                seg[t * NR + i] = (int)q;
+                while (pos[i] < rp[i + 1] && col[pos[i]] < (fom_int)(t + 1) * TC) {
+                    c16[q] = (unsigned short)(col[pos[i]] - t * TC);
+                    v[q] = val[pos[i]];
+                    q++;
+                    pos[i]++;
+                }
+                bank_order(c16 + seg[t * NR + i], v + seg[t * NR + i], (int)(q - seg[t * NR + i]), G);
+            }
+        seg[NT * NR] = (int)q;
+        for (int i = 0; i < NR; i++) good &= pos[i] == rp[i + 1];  /* sorted rows only */
+    }
+    /* exponent range check, then encode (see tiled_kern) */
+    static_assert(TC <= (1 << 14), "column offsets must fit in 14 bits");
+    int emin = 0x7FF, emax = 0;
+    unsigned long long *wv = 0;
+    unsigned char *ch = 0;
+    if (good) {
+        for (fom_int k = 0; k < nnz; k++) {
+            unsigned long long b;
+            memcpy(&b, v + k, sizeof(b));
+            int e = (int)((b >> 52) & 0x7FF);
+            if (e < emin) emin = e;
+            if (e > emax) emax = e;
+        }
+        good = nnz > 0 && emin > 0 && emax < 0x7FF && emax - emin < 32;
+        T->ebase = emin;
+    }
+    if (good) {
+        wv = (unsigned long long *)malloc(sizeof(unsigned long long) * (size_t)nnz);
+        ch = (unsigned char *)malloc((size_t)nnz);
+        good = wv && ch;
+    }
+    if (good)
+        for (fom_int k = 0; k < nnz; k++) {
+            unsigned long long b;
+            memcpy(&b, v + k, sizeof(b));
+            unsigned long long f = ((((b >> 52) & 0x7FF) - (unsigned long long)emin) << 6) | (c16[k] & 63u);
+            wv[k] = (b & 0x800FFFFFFFFFFFFFull) | (f << 52);
+            ch[k] = (unsigned char)(c16[k] >> 6);
+        }
+    size_t bv = sizeof(unsigned long long) * (size_t)nnz, bc = (size_t)nnz;
+    char *blob = 0;
+    if (good) good = cudaMalloc((void **)&blob, bv + bc) == cudaSuccess;
+    if (good) {
+        cudaMemcpy(blob, wv, bv, cudaMemcpyHostToDevice);
+        cudaMemcpy(blob + bv, ch, bc, cudaMemcpyHostToDevice);
+        T->blob = blob;
+        T->bytes = bv + bc;
+        T->val = (unsigned long long *)blob;
+        T->chi = (unsigned char *)(blob + bv);
+        good = cudaMalloc((void **)&T->seg, sizeof(int) * ((size_t)NT * NR + 1)) == cudaSuccess;
+    }
+    if (good) {
+        cudaMemcpy(T->seg, seg, sizeof(int) * ((size_t)NT * NR + 1), cudaMemcpyHostToDevice);
+        T->part = 0;
+        T->cnt = 0;
+        if (NT > 1) {
+            good = cudaMalloc((void **)&T->part, sizeof(fom_float) * NT * NR) == cudaSuccess &&
+                   cudaMalloc((void **)&T->cnt, sizeof(unsigned) * CH) == cudaSuccess;
+            if (good) cudaMemset(T->cnt, 0, sizeof(unsigned) * CH);
+        }
+    }
+    if (good)
+        good = cudaFuncSetAttribute(tiled_kern<NR, NC, TC, CH, G, U, RB>,
+                                    cudaFuncAttributeMaxDynamicSharedMemorySize,
+                                    (int)(sizeof(fom_float) * TC)) == cudaSuccess;
+    T->hit = 0;
+    int dev = 0, maxp = 0, wmax = 0;
+    cudaGetDevice(&dev);
+    cudaDeviceGetAttribute(&maxp, cudaDevAttrMaxPersistingL2CacheSize, dev);
+    cudaDeviceGetAttribute(&wmax, cudaDevAttrMaxAccessPolicyWindowSize, dev);
+    if (good && persist > 0 && maxp > 0 && wmax > 0 &&
+        cudaDeviceSetLimit(cudaLimitPersistingL2CacheSize, (size_t)maxp) == cudaSuccess) {
+        if (T->bytes > (size_t)wmax) T->bytes = (size_t)wmax;
+        size_t keep = T->bytes / 100 * (size_t)persist;
+        if (keep > (size_t)maxp) keep = (size_t)maxp;
+        T->hit = (float)keep / (float)T->bytes;
+    }
+    cudaGetLastError();
+    free(seg); free(c16); free(v); free(pos); free(wv); free(ch);
+    T->ok = good;
+}
+
+template <int NR, int NC, int TC, int CH, int G, int U, int RB>
+static void tiled_launch(const tiled_csr *T, const fom_float *x, fom_float *out) {
+    cudaLaunchConfig_t cfg;
+    cudaLaunchAttribute attr[1];
+    cfg.gridDim = dim3(CH, (NC + TC - 1) / TC);
+    cfg.blockDim = dim3(TL_THREADS);
+    cfg.dynamicSmemBytes = sizeof(fom_float) * TC;
+    cfg.stream = FOM_OPS_STREAM;
+    cfg.attrs = attr;
+    cfg.numAttrs = 0;
+    if (T->hit > 0) {
+        attr[0].id = cudaLaunchAttributeAccessPolicyWindow;
+        attr[0].val.accessPolicyWindow.base_ptr = T->blob;
+        attr[0].val.accessPolicyWindow.num_bytes = T->bytes;
+        attr[0].val.accessPolicyWindow.hitRatio = T->hit;
+        attr[0].val.accessPolicyWindow.hitProp = cudaAccessPropertyPersisting;
+        attr[0].val.accessPolicyWindow.missProp = cudaAccessPropertyStreaming;
+        cfg.numAttrs = 1;
+    }
+    cudaLaunchKernelEx(&cfg, tiled_kern<NR, NC, TC, CH, G, U, RB>, (const int *)T->seg,
+                       (const unsigned char *)T->chi, (const unsigned long long *)T->val, T->ebase, x, T->part,
+                       T->cnt, out);
+}
+
+/* t0_state: 10000 x 40000, ~260 nnz/row: 4 tiles of 10000 columns, 27
+   row chunks (one 1024-thread block per SM), ~65 entries per row and
+   tile, so 16 lanes x 5 loads cover a row segment in one round. Half of
+   its packed lines persist in L2, half of t1_t_t0's (the 25 MB carve-out
+   holds about half of the 52 MB both stream per iteration). */
+#ifndef T0_CH
+#define T0_CH 27
+#define T0_G 16
+#define T0_U 5
+#define T0_RB 1
+#define T0_PERSIST 50
+#define T0_TC 10000
+#endif
+#define T0_CFG 10000, 40000, T0_TC, T0_CH, T0_G, T0_U, T0_RB
+static tiled_csr t0s_tl;
+
+/* Row-wise fallback (a warp per row) if the tiled layout cannot be built. */
+static __global__ void fom_apply_t0_state_csr_kern(const fom_float *x, fom_float *out) {
+    int i = (blockIdx.x * blockDim.x + threadIdx.x) / 32, lane = threadIdx.x % 32;
+    if (i >= 10000) return;
     fom_float s = 0.0;
-    for (fom_int k = FOM_OP_T0_STATE_ROWPTR[i] + lane; k < FOM_OP_T0_STATE_ROWPTR[i + 1]; k += 32) s += FOM_OP_T0_STATE_VALS[k] * x[FOM_OP_T0_STATE_COLIDX[k]];
+    for (fom_int k = FOM_OP_T0_STATE_ROWPTR[i] + lane; k < FOM_OP_T0_STATE_ROWPTR[i + 1]; k += 32)
+        s += FOM_OP_T0_STATE_VALS[k] * x[FOM_OP_T0_STATE_COLIDX[k]];
     for (int w = 16; w > 0; w /= 2) s += __shfl_down_sync(0xffffffff, s, w);
     if (lane == 0) out[i] += s;
 }
 
+static void fom_apply_t0_state_pack(void) {
+    static fom_int rp[10001], col[2599983];
+    static fom_float val[2599983];
+    cudaMemcpyFromSymbol(rp, FOM_OP_T0_STATE_ROWPTR, sizeof(rp));
+    cudaMemcpyFromSymbol(col, FOM_OP_T0_STATE_COLIDX, sizeof(col));
+    cudaMemcpyFromSymbol(val, FOM_OP_T0_STATE_VALS, sizeof(val));
+    t0s_tl.ok = 0;
+    if (rp[10000] == 2599983) tiled_pack<T0_CFG>(&t0s_tl, rp, col, val, T0_PERSIST);
+}
+
 extern "C" void fom_apply_t0_state(const fom_float *x, fom_float *out) {
-    fom_apply_t0_state_kern<<<1250, FOM_OPS_THREADS>>>(x, out);
+    static int ready = 0;
+    if (!ready) { fom_apply_t0_state_pack(); ready = 1; }
+    if (t0s_tl.ok) tiled_launch<T0_CFG>(&t0s_tl, x, out);
+    else fom_apply_t0_state_csr_kern<<<1250, FOM_OPS_THREADS, 0, FOM_OPS_STREAM>>>(x, out);
 }
 
 static __global__ void fom_apply_t0_p_p0_kern(const fom_float *x, fom_float *out) {
@@ -33,7 +374,7 @@
 }
 
 extern "C" void fom_apply_t0_p_p0(const fom_float *x, fom_float *out) {
-    fom_apply_t0_p_p0_kern<<<40, FOM_OPS_THREADS>>>(x, out);
+    fom_apply_t0_p_p0_kern<<<40, FOM_OPS_THREADS, 0, FOM_OPS_STREAM>>>(x, out);
 }
 
 static __global__ void fom_apply_t1_state_kern(const fom_float *x, fom_float *out) {
@@ -45,22 +386,47 @@
 }
 
 extern "C" void fom_apply_t1_state(const fom_float *x, fom_float *out) {
-    fom_apply_t1_state_kern<<<157, FOM_OPS_THREADS>>>(x, out);
-}
-
-static __global__ void fom_apply_t1_t_t0_kern(const fom_float *x, fom_float *out) {
-    /* One warp per row: lanes stride the row (coalesced), then a shuffle sum. */
-    fom_int i = (blockIdx.x * blockDim.x + threadIdx.x) / 32;
-    fom_int lane = threadIdx.x % 32;
-    if (i >= 40000) return;  /* whole warps exit together */
+    fom_apply_t1_state_kern<<<157, FOM_OPS_THREADS, 0, FOM_OPS_STREAM>>>(x, out);
+}
+
+/* t1_t_t0: 40000 x 10000, ~65 nnz/row: all of x (80 KB) is one tile,
+   108 row chunks, 16 lanes x 5 loads per row. */
+#ifndef T1_CH
+#define T1_CH 108
+#define T1_G 16
+#define T1_U 5
+#define T1_RB 1
+#define T1_PERSIST 50
+#define T1_TC 10000
+#endif
+#define T1_CFG 40000, 10000, T1_TC, T1_CH, T1_G, T1_U, T1_RB
+static tiled_csr t1t_tl;
+
+static __global__ void fom_apply_t1_t_t0_csr_kern(const fom_float *x, fom_float *out) {
+    int i = (blockIdx.x * blockDim.x + threadIdx.x) / 16, lane = threadIdx.x % 16;
+    if (i >= 40000) return;
     fom_float s = 0.0;
-    for (fom_int k = FOM_OP_T1_T_T0_ROWPTR[i] + lane; k < FOM_OP_T1_T_T0_ROWPTR[i + 1]; k += 32) s += FOM_OP_T1_T_T0_VALS[k] * x[FOM_OP_T1_T_T0_COLIDX[k]];
-    for (int w = 16; w > 0; w /= 2) s += __shfl_down_sync(0xffffffff, s, w);
+    for (fom_int k = FOM_OP_T1_T_T0_ROWPTR[i] + lane; k < FOM_OP_T1_T_T0_ROWPTR[i + 1]; k += 16)
+        s += FOM_OP_T1_T_T0_VALS[k] * x[FOM_OP_T1_T_T0_COLIDX[k]];
+    for (int w = 8; w > 0; w /= 2) s += __shfl_down_sync(0xffffffff, s, w, 16);
     if (lane == 0) out[i] += s;
 }
 
+static void fom_apply_t1_t_t0_pack(void) {
+    static fom_int rp[40001], col[2599983];
+    static fom_float val[2599983];
+    cudaMemcpyFromSymbol(rp, FOM_OP_T1_T_T0_ROWPTR, sizeof(rp));
+    cudaMemcpyFromSymbol(col, FOM_OP_T1_T_T0_COLIDX, sizeof(col));
+    cudaMemcpyFromSymbol(val, FOM_OP_T1_T_T0_VALS, sizeof(val));
+    t1t_tl.ok = 0;
+    if (rp[40000] == 2599983) tiled_pack<T1_CFG>(&t1t_tl, rp, col, val, T1_PERSIST);
+}
+
 extern "C" void fom_apply_t1_t_t0(const fom_float *x, fom_float *out) {
-    fom_apply_t1_t_t0_kern<<<5000, FOM_OPS_THREADS>>>(x, out);
+    static int ready = 0;
+    if (!ready) { fom_apply_t1_t_t0_pack(); ready = 1; }
+    if (t1t_tl.ok) tiled_launch<T1_CFG>(&t1t_tl, x, out);
+    else fom_apply_t1_t_t0_csr_kern<<<2500, FOM_OPS_THREADS, 0, FOM_OPS_STREAM>>>(x, out);
 }
 
 static __global__ void fom_apply_su_t_t1_kern(const fom_float *x, fom_float *out) {
@@ -72,5 +438,5 @@
 }
 
 extern "C" void fom_apply_su_t_t1(const fom_float *x, fom_float *out) {
-    fom_apply_su_t_t1_kern<<<157, FOM_OPS_THREADS>>>(x, out);
-}
+    fom_apply_su_t_t1_kern<<<157, FOM_OPS_THREADS, 0, FOM_OPS_STREAM>>>(x, out);
+}
```

## Agent notes

```
### Round 1

**Whole-solve mean_us:** 18290.0 (initial) → 13295.2 (final), a **1.38× speedup**. `make verify` passes (worst error 2.7e-15) and the kernel-rule check passes.

**Final profile.** This comes from a hand-built `-DFOM_PROFILE` bench (the CUDA Makefile has no `profile` target). In it each region syncs the GPU at its end and the CUDA graph is off, so the times are real GPU times. That build runs at 16640 µs per solve.

| region | total_us (30 solves) | per call | % |
|---|---|---|---|
| t0_matvec (`t0_state` @ s, 10000×40000, 2.6M nnz) | 263350 | 43.9 µs | 53.3 |
| t1_matvec (`t1_t_t0` @ t0, 40000×10000, 2.6M nnz) | 172219 | 28.7 µs | 34.8 |
| state_update (fused soft-threshold, mostly sync overhead here) | 57745 | 9.6 µs | 11.7 |
| setup_const (`t0_p_p0` @ p0 once per solve) | 694 | 23 µs | 0.1 |
| residual_readback (once per solve) | 271 | 9 µs | 0.05 |

In production the iteration is about 66 µs, almost all of it the two matvecs.

**Optimizations applied:**
1. **16-bit column indices** for both big operators. Both have fewer than 65536 columns (40000 and 10000), so each operator's own `COLIDX` is narrowed once, on its first `fom_apply` call, into a static device array. That streams 10 instead of 12 bytes per nonzero (about 4–5% per iteration).
2. **Row-group SpMV tuned to row length.** Rows are uniform: `t0_state` has 257–260 nnz per row and `t1_t_t0` has 64–65. `t0_state` uses 8 lanes per row and `t1_t_t0` uses 16, each with 8 or 4 independent loads in flight per lane. Each row is summed in a fixed order, so results are deterministic. This plus item 1 cut the matvec pair from about 73 to about 66 µs.
3. **Fused vector resets into the state-update kernel.** `t1_state` and `su_t_t1` are identity matrices and λ0 = 0. The update kernel already computes `s+ = soft(s + acc, λ1)`; it now also re-arms the next iteration by setting `acc = 0` and `t0 = c`. This removes one memset and one device-to-device copy per iteration. The constant term `c = t0_p_p0 @ p0` is computed once per solve, since it doesn't change across iterations.
4. **CUDA graph of two iterations.** Termination is `max_iters` with tol = 0, so only the last residual is ever read. Two iterations (ping-pong state buffers) are captured once after the first eager solve and launched 99 times per solve; the last iteration runs eagerly and computes the residual. All kernels, including those in `fom_ops.cu`, launch on `cudaStreamPerThread`. Unlike the legacy default stream it can be captured, and it still orders with the legacy-stream memsets and copies that callers make.
5. **L2 persistence.** The two operators stream about 52 MB per iteration, more than the 40 MB L2, so they evict each other. An access-policy window on `t1_t_t0`'s values (20.8 MB) and one on `t0_state`'s 16-bit indices (5.2 MB) together fill the 25 MB persisting set-aside (13.90 → 13.27 ms).

**Tried and rejected** (all measured):
- Shared-memory `x` slabs, which hit bank conflicts and the cost of loading `x` into shared memory in every block.
- Column-quarter or L1-phased passes.
- L1 no-allocate, evict-first and evict-last hints, and bypassing L1 for the gather, which made it 2× slower.
- Deeper unrolling and other block sizes.
- Persisting `t0_state`'s values instead of `t1_t_t0`'s.
- Packing `t1_t_t0`'s values and indices into one window.

`t0_state`'s gathers into the 40000-entry `x` mostly miss L1. That limits it to about 35–40 µs, and none of these changed it.

Blocked by rules:
- Read `t0_state` once per iteration for both products: `t1_t_t0` equals −0.0147 · `t0_state`ᵀ (checked entry by entry), so one stored matrix could serve both | each `fom_apply_<name>` may read only its own matrix's data, and no new float constants | up to ~40% less matrix traffic, plausibly 1.3–1.6×
- Fuse the soft-threshold, state update and accumulator reset into the `t1_t_t0` matvec's final per-row write | `fom_apply_<name>` must compute exactly `out += M x` | ~2–3 µs/iter (~4%)
- One persistent cooperative kernel running both matvecs and the update with grid syncs, removing per-kernel tail effects | matvec loops must live inside the separate `fom_apply_*` entry points, each launched as its own kernel | unknown, likely 5–10%
- Build or persist the derived index arrays and L2 policy in `fom_setup` instead of lazily inside the first `fom_apply` call | `fom_setup` may not call `fom_apply_*`, and `fom_ops.cu` has no other setup hook | ~0 per solve (first-call cost only)
- Pin host buffers or overlap the per-solve upload and download | `include/` workspace layout and the bench harness are fixed | <1%

### Round 2

**Whole-solve time (`make bench` mean_us):** 13307 us at the start of this round, **8254 us** at the end (8233 to 8275 us over 5 runs). That is **1.61x** this round and 18153 → 8254 = **2.20x** against the translated solver. `make verify` prints VERIFY OK: the worst error is 3.6e-15 against a tolerance of 1e-5, and the direct checks of `t0_state` and `t1_t_t0` show errors of 4e-15 and 4e-16. One earlier run read 8172 us, but rebuilding the same code back to back showed that was a fast outlier, not a real gain.

**Profile of the final solver.** There is no `make profile` target in this Makefile, so I built the bench by hand with `-DFOM_PROFILE` and `src/fom_profile.c`. That build syncs after every region and runs each iteration without the CUDA graph, so each region also includes about 5 us of launch and sync cost that production avoids.

| region | time per call | share |
|---|---|---|
| t1_matvec (`t1_t_t0`, 40000x10000) | 24.1 us | 42.1% |
| t0_matvec (`t0_state`, 10000x40000) | 24.0 us | 41.9% |
| state_update (soft-threshold, residual, re-arm) | 9.0 us | 15.7% |
| setup_const (once per solve) | 18 us | 0.16% |
| residual_readback (once per solve) | 10 us | 0.09% |

**Optimizations this round** (each one passed verify and was kept only if it measured faster):
1. **Column-tiled `t0_state`: 13.3 → 9.0 ms.** The matrix has about 260 nonzeros per row at random columns out of 40000. The old row-wise kernel gathered x (320 KB) at random from L2, which held it near 600 GB/s at 36% occupancy. Making each row group wider did not help, so the gather pattern was the limit, not occupancy.
   - Now the columns are split into 4 tiles of 10000. Each tile's 80 KB slice of x fits in shared memory.
   - The entries are repacked once, tile by tile, with 16-bit column offsets, from the matrix's own CSR on the first call.
   - Each block (27 row chunks × 4 tiles = 108 blocks, one per SM) stages its x slice and writes per-row partial sums. The last block of each chunk adds them in tile order, so results stay deterministic.
   - I adapted this from the sibling `fista_cuda` solver, which uses the same matrix.
2. **Same tiled kernel for `t1_t_t0`, with 50/50 L2 persistence: 9.0 → 8.7 ms.** All of x (10000 entries, 80 KB) fits in one tile, so there are no partial sums. The 25 MB persisting L2 region is split evenly between the two operators' packed data; the sweep showed 50/50 was best. I tuned 16 lanes × 5 loads per row and one row per group for both kernels: a row segment has about 65 entries, so all loads go out in one round.
3. **Prefetching inside the kernel: 8.7 → 8.4 ms.** This attacks load latency, which ncu showed was the limit. The first batch's matrix loads now go out before the x-staging barrier. Each batch's segment bounds are fetched one batch ahead, so value loads never wait on a dependent segment load.
4. **Bank-aware entry order: about 2% on the two kernels.** A double at column c sits in shared-memory bank pair c mod 16, and the 16 lanes reading one row hit about 5-way bank conflicts. At pack time each row's entries are reordered so each run of 16 reads has distinct residues where possible. Only the summation order changes, and it is fixed, so results stay deterministic.
5. **Page-locked workspace: about 1.1%.** The solver now calls `cudaHostRegister` on the workspace in `fom_setup`, so the per-solve upload of state and p0 and the download of the solution use direct DMA.

**Tried and reverted:**
- 2 blocks per SM (512-thread blocks, or `__launch_bounds__(1024,2)`): register spills, 10–40% slower.
- Wider or narrower tiles for `t0_state` (5000, 8000, 13334 and 20000 columns): all slower than 10000.
- Other lane counts, loads per lane and rows per batch: all slower or no better.
- Double-buffering the value registers: about 1% in my harness, not worth the extra code.
- Normal instead of streaming miss policy, and mixing the two policies between operators: 10–15% slower.
- Persistence splits other than 50/50: within noise or worse.

Blocked by rules:
- Store A once and use it for both products. I checked that `t1_t_t0` has exactly the pattern of `t0_state` transposed, with every value scaled by -0.014703924… (equal to within a few ulps). One 26 MB copy would nearly fit in L2 instead of streaming 52 MB per iteration | each `fom_apply_<name>` and its helpers may read only its own matrix's data, and each must apply exactly its own matrix | about 30–45%
- Fold the soft-threshold, residual and state update into the `t1_t_t0` kernel's epilogue, removing one kernel boundary per iteration | `fom_apply_<name>` must compute exactly `out += M x`; activations belong in the solver | about 3–5% (about 2 us per iteration)
- Run the whole iteration as one persistent kernel with grid-wide syncs, so the x slices stay in shared memory and there are no kernel-boundary drains | matrix-vector work may not live in `src/fom_solver.cu`, and the `fom_apply_*` signatures (no stream or fusion hook) are fixed | unknown, likely 5–10%

### Round 3

**Whole-solve time:** 8248.7 us at the start of this round, now 7335 to 7363 us (`make bench` mean_us; the final run gave 7363.3 us). That is about 1.12x faster this round. Against the translated solver (18153.3 us) it is about 2.47x overall. `make verify` prints VERIFY OK; the worst end-to-end error is 3.6e-15, and the operator checks give 4.4e-15 or less.

**Final profile breakdown.** This comes from a `-DFOM_PROFILE` build, which runs every iteration eagerly with a sync at the end of each region, so absolute times include launch and sync overhead:

| Region | Total time | Share |
|---|---|---|
| t0_matvec (A x) | 850.9 ms | 42.5% |
| t1_matvec (A^T step) | 842.0 ms | 42.0% |
| state_update (soft-threshold) | 297.9 ms | 14.9% |
| setup_const | 7.1 ms | 0.4% |
| download | 3.5 ms | 0.2% |
| residual_readback | 1.5 ms | 0.1% |

The production build runs the iterations as CUDA graphs, at about 36.7 us per iteration.

**Optimizations applied this round:**
- **Lossless 9-byte matrix entries instead of 10** (`fom_ops.cu`). Every coefficient of both big operators is a normal double, and their exponents span 25 values (at most 32 allowed). So each value's 11-bit exponent field can hold a 5-bit offset from the smallest exponent plus the low 6 bits of its column offset. A separate byte array holds the high column bits; tile offsets are below 2^14. Decoding uses only 32-bit integer ops on the high word and restores the exact double. The minimum exponent is computed at setup from the operator's own CSR. If the range check fails, the code falls back to the row-wise kernel. This cuts matrix traffic by about 10% and was worth about 2%.
- **Retuned L2 persistence** after the change. The best setting is still 50/50; values from 35 to 60 were tried. Before the encoding change, 40/40 had been about 3% better than 50/50.
- **Larger CUDA graphs** (`fom_solver.cu`): 66 iterations per graph launch instead of 2, so a 200-iteration solve uses 3 graph launches instead of 99. This removes the gaps between graph launches (about 1.9%). It is valid because the run has a fixed iteration count and no tolerance check.
- **Async uploads and one sync per solve.** The state and parameter uploads are queued on the stream, and the final residual and solution come back under a single sync (about 1.2%).
- **In-place state accumulation.** `t1_state` and `su_t_t1` are identity selections and the A^T kernel adds each full row sum once, so the A^T product now accumulates straight into the state vector (`y += B t0`). The update kernel then thresholds y in place. This drops the separate accumulator and its zeroing, and the results are bitwise identical. The previous state is copied only on the last iteration, for the residual (about 2.6%).

**Tried and rejected** (all measured slower or within noise):
- L2 prefetch of future batches;
- gathering x through L1 instead of staging it in shared memory;
- `cp.async` staging of x;
- staggered staging offsets per block;
- smaller column tiles (5000 instead of 10000);
- other lane-group, unroll and row-batch settings (G, U, RB);
- different block sizes for the update kernel.

The two-iteration A x / A^T step pattern is now close to the A100's L2 bandwidth limit: about 64 MB moves from L2 to the SMs per iteration, and staging x accounts for 17 MB of that.

Blocked by rules:
- Read only A (t0_state) in both matvecs, since t1_t_t0 equals -0.0147·A^T exactly (checked); 23 MB would then fit in persisting L2 and DRAM traffic per iteration would roughly halve | each fom_apply_<name> may read only its own matrix's data | ~30-40%
- Fuse the soft-threshold, the t0 = c re-arm and the residual into the epilogue of the A^T kernel, and the threshold into the x staging of the next A x kernel, removing one kernel and one full drain per iteration | fom_apply_* must compute exactly out += M x, and matrix kernels must live in fom_ops.cu, not fom_solver.cu | ~8% (removing the update kernel measured ~7.5%)
- Run each iteration as one persistent kernel with a grid-wide sync between the A x, A^T and threshold phases (no launch gaps) | matvecs must go through the separate fom_apply_* entry points | unknown, ~5-10%
- Programmatic dependent launch so the next matvec's matrix loads start while the previous kernel drains | target constraint: the build is fixed at sm_80 and PDL needs sm_90 | ~5%
- Shrink matrix values further with a lossy fp32+fp16 split (8 bytes per entry) | the kernel must apply exactly M (and the 1e-10 operator check) | ~8%

### Round 4

**Result:** no improvement this round. The sources are unchanged from round 3.
- Initial `mean_us`: 7368.3 (round-3 best, A100-SXM4 node). Final: unchanged code, 7502 us when re-benched on the A100 PCIe node the allocation moved to. The difference is the node, not the code.
- Overall speedup vs the translated solver (18153.3 us): 2.46x, unchanged.

**Profiling breakdown (final solver, `-DFOM_PROFILE` build, which syncs at each region end and runs without graphs):**
| region | total_us (50 solves) | % |
|---|---|---|
| t0_matvec | 236987 | 42.84 |
| t1_matvec | 230058 | 41.58 |
| state_update | 82643 | 14.94 |
| setup_const | 2014 | 0.36 |
| download | 1131 | 0.20 |
| residual_readback | 404 | 0.07 |

In the production graph build, one iteration is ~37 us. That equals the two column-tiled sparse matvecs launched back to back, so the soft-threshold/re-arm kernel and graph gaps add nothing visible.

**Tried this round, all rejected (each tested against the baseline in the same process, repeated, with median timing):**
- Tuning lanes per entry, loads per lane, rows per batch and L2-persist split (G/U/RB/PERSIST). No robust gain: U=6 looked 8% faster but was cross-process noise on a shared GPU, and other settings were 5–30% slower.
- Even-aligned segments so each lane loads two packed entries with one 16-byte load. This halves the global load instructions (the kernel showed load-unit throttle stalls), but runtime stayed the same.
- Smaller column tiles to cut x-staging traffic (t1 at 5000 columns; t0 at 8000 or 5000). This exploits the 2-D tile × row-chunk split: total staging bytes = blocks × tile width. Every variant was 1–4 us/iter slower because of more per-row-segment overhead and more partial sums.
- Asynchronous staging (cp.async), a per-block rotated staging order, and gathering x from global memory for the first batch while staging. Each was 0–12 us/iter slower.
- Cache hints on the matrix stream (`__ldcs`, `__ldcg`, `__ldlu`). All slower; bypassing L1 cost +9 us/iter because adjacent lanes and segments share L1 lines.
- Skipping zero entries of x in t0 (exact, since v·0 adds nothing). Not applicable: the soft-thresholded iterate stays ≥99.99% nonzero.

**Diagnostics behind these decisions:**
- With its matrix fully L2-resident, one matvec still takes ~16 us. A plain streaming kernel over the same bytes takes ~9 us, so the kernel is not DRAM-bound.
- Removing the x staging saves ~2.8 us per matvec. Removing the decode and shared-memory gather saves only ~0.7 us per matvec.

Blocked by rules:
- Store `t0_state` once and apply `t1_t_t0` as its scaled transpose (here `t1_t_t0` = −step·`t0_state`ᵀ), so one matrix copy is shared and could stay in L2 | `fom_apply_<name>` and its helpers may read only their own matrix's data | ~up to 2x less matrix traffic; unknown net, since a transposed apply needs a scatter or second layout
- One persistent fused kernel per iteration batch (t0 product → soft → t1 product with grid-wide syncs), removing 3 launches per iteration and letting staging overlap the previous phase | matvec loops must live inside each `fom_apply_<name>`, and no code may read two matrices' data | ~3–6 us/iter (8–15%)
- Fold the soft-threshold of y into the t0 kernel's x staging, dropping the state-update kernel | each `fom_apply_<name>` must compute exactly `out += M x` for its given `x` | ~1–2 us/iter
- Lossy value compression (float32 or truncated mantissas) to cut the 9 bytes per entry | the direct `fom_apply` checks require applying exactly `M` | ~40% less matrix traffic, unknown net
- Programmatic dependent launch or thread-block clusters (shared x staging across SMs) | target constraint: the build flags pin sm_80, and these features need sm_90 | unknown
```
