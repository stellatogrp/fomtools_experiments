# Optimization report

## Timing

```
translated (emitted)   18047.9 us/solve
optimized               8745.1 us/solve
speedup                  2.06x
```

Optimization outcome: **kept (re-verified correct)**.

## Rounds

| round | before (us) | after (us) | gain | kept | agent (s) | outcome |
|---:|---:|---:|---:|:---:|---:|---|
| 1 | 18047.859 | 13028.802 | 27.8% | yes | 4269 | continued |
| 2 | 13244.577 | 11404.521 | 13.9% | yes | 5508 | continued |
| 3 | 11419.926 | 9827.593 | 13.9% | yes | 3998 | continued |
| 4 | 9810.379 | 8745.132 | 10.9% | yes | 1622 | continued (agent stopped at its budget) |
| 5 | - | - | - | no | 135 | the round left the sources unchanged (agent stopped at its budget) |

## Caveats

- The agent was terminated at its wall-clock budget before it finished (in round 4, 5), so it may never have written a summary. The timing and diff below are still sound: the solver on disk was re-verified and benchmarked. Raise `CodegenConfig.agent_optimize_timeout_s` to give it longer.

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
| 1 | Capture the 200-iteration loop in a CUDA graph | the `fom_apply_*` signatures are fixed and launch on the legacy default stream, which cannot be stream-captured | ~3-5% (launch gaps) |
| 1 | Fuse the soft-threshold/state update into the epilogue of the t1_t_t0 matvec | `fom_apply_<name>` must compute exactly out += M x, and matvec loops may not live in `fom_solver.cu` | ~5-10% (saves one kernel and a 320 KB round trip per iteration) |
| 1 | Use t1_t_t0's relation to t0_state (it appears to be a scaled A^T) to keep one 26 MB matrix, which would fit entirely in L2, instead of two | each `fom_apply_<name>` may read only its own matrix's data | potentially large, maybe 1.5-2x, since both products would run from L2 |
| 2 | Computing `G(A s)` with one matrix's data (`A^T` values are `-step × A` values), so only one 21 MB value array is streamed per iteration, which would fit the 40 MB L2 together with the indices | `fom_apply_<name>` may read only its own matrix's data (and its signature is fixed) | ~35–40% (about 40 us per iteration less DRAM traffic) |
| 2 | Fusing the soft-threshold into the epilogue of the `A^T` matvec (`ns = soft(s + G t0)` in one kernel), dropping the separate update kernel | each `fom_apply_<name>` must apply exactly `M` and accumulate into `out` | ~5% (about 3–4 us per iteration) |
| 2 | Fusing the `A s` and `A^T t0` launches into one persistent cooperative kernel with a grid-wide barrier | matrix-vector loops must live inside their own `fom_apply_<name>` and may not be written in `fom_solver.cu` | unknown, ~2–3% |
| 3 | Share the coefficients of `t0_state` (A) and `t1_t_t0` (−γAᵀ, the same nonzeros scaled) so one stream of about 26 MB serves both products, or keep that one copy L2-resident | each `fom_apply_<name>` may read only its own matrix's data | large, roughly 25-40% (DRAM traffic per iteration drops from 52 MB toward 26 MB) |
| 3 | Fuse the soft threshold and t0 = c re-arm into the epilogue of the `t1_t_t0` matvec kernel | matvec code must live in `fom_apply_*`, which must compute exactly out += M·x | ~2 µs per iteration (~4%) |
| 3 | Store the coefficients in float32 to halve the value stream | `fom_apply_<name>` must apply exactly M (checked directly against the reference) | ~35-40%, but it breaks exactness |

## What the agent changed

```diff
--- src/fom_solver.cu (translated)
+++ src/fom_solver.cu (optimized)
@@ -2,8 +2,22 @@
 #include "fom_solver.h"
 #include "fom_data.h"
 #include "fom_ops.h"
+#include "fom_profile.h"
 
-static fom_float *d_s = 0, *d_ns = 0, *d_c = 0, *d_t0 = 0, *d_acc = 0, *d_p = 0, *d_res = 0;
+#ifdef FOM_PROFILE
+#define FOM_PSYNC() cudaDeviceSynchronize()
+#else
+#define FOM_PSYNC() ((void)0)
+#endif
+
+/* All iteration work runs on cudaStreamPerThread (as do the two big
+   fom_apply_* kernels), which is capturable: FOM_GRAPH_ITERS iterations are
+   captured once into a CUDA graph and replayed, cutting per-kernel launch
+   and inter-kernel gaps. It synchronizes with the legacy default stream. */
+#define FOM_STREAM cudaStreamPerThread
+#define FOM_GRAPH_ITERS 66
+static cudaGraphExec_t g_exec = 0;
+static fom_float *d_s = 0, *d_ns = 0, *d_c = 0, *d_t0 = 0, *d_p = 0, *d_res = 0;
 
 __global__ void k_soft(fom_float *x, int n, fom_float lam) {
     int i = blockIdx.x * blockDim.x + threadIdx.x;
@@ -13,13 +27,16 @@
     }
 }
 
-/* ns = soft(s + acc, lam) */
-__global__ void k_t1(const fom_float *s, const fom_float *acc, fom_float *ns, int n, fom_float lam) {
+/* s = soft(s, lam) in place (s already holds s + G t0, accumulated by
+   fom_apply_t1_t_t0 since t1_state is the identity); also re-arms t0 = c
+   for the next iteration, replacing a memcpy. */
+__global__ void k_t1(fom_float *s, int n, fom_float lam, const fom_float *c, fom_float *t0, int n0) {
     int i = blockIdx.x * blockDim.x + threadIdx.x;
     if (i < n) {
-        fom_float v = s[i] + acc[i];
-        ns[i] = v > lam ? v - lam : (v < -lam ? v + lam : (fom_float)0);
+        fom_float v = s[i];
+        s[i] = v > lam ? v - lam : (v < -lam ? v + lam : (fom_float)0);
     }
+    if (i < n0) t0[i] = c[i];
 }
 
 /* single-block deterministic max|a-b| */
@@ -54,7 +71,6 @@
         cudaMalloc(&d_ns, sizeof(fom_float) * FOM_STATE_DIM);
         cudaMalloc(&d_c, sizeof(fom_float) * FOM_T0_DIM);
         cudaMalloc(&d_t0, sizeof(fom_float) * FOM_T0_DIM);
-        cudaMalloc(&d_acc, sizeof(fom_float) * FOM_T1_DIM);
         cudaMalloc(&d_p, sizeof(fom_float) * FOM_PARAM_P0_DIM);
         cudaMalloc(&d_res, sizeof(fom_float));
     }
@@ -65,34 +81,75 @@
     reset_state(work);
 }
 
-fom_int fom_solve(FomWorkspace *work) {
+/* one ISTA iteration, in place on d_s: t0 = c + A s; s += G t0;
+   s = soft(s). On the last one the previous iterate is kept in d_ns for
+   the residual max|s+ - s|. */
+static void iterate(int last) {
     const int n0 = FOM_T0_DIM, n1 = FOM_T1_DIM, B = 256;
     const int g0 = (n0 + B - 1) >> 8, g1 = (n1 + B - 1) >> 8;
-    cudaMemcpy(d_s, work->state, sizeof(fom_float) * FOM_STATE_DIM, cudaMemcpyHostToDevice);
-    cudaMemcpy(d_p, work->params.p0, sizeof(fom_float) * FOM_PARAM_P0_DIM, cudaMemcpyHostToDevice);
+    FOM_PROF_BEGIN(apply_t0_state);
+    fom_apply_t0_state(d_s, d_t0);
+    if (FOM_T0_LAMBDA != (fom_float)0) k_soft<<<g0, B, 0, FOM_STREAM>>>(d_t0, n0, FOM_T0_LAMBDA);
+    FOM_PSYNC();
+    FOM_PROF_END(apply_t0_state);
+    if (last) cudaMemcpyAsync(d_ns, d_s, sizeof(fom_float) * n1, cudaMemcpyDeviceToDevice, FOM_STREAM);
+    FOM_PROF_BEGIN(apply_t1_t_t0);
+    fom_apply_t1_t_t0(d_t0, d_s);
+    FOM_PSYNC();
+    FOM_PROF_END(apply_t1_t_t0);
+    FOM_PROF_BEGIN(soft_update);
+    k_t1<<<g1, B, 0, FOM_STREAM>>>(d_s, n1, FOM_T1_LAMBDA, d_c, d_t0, n0);
+    if (last) k_maxdiff<<<1, 1024, 0, FOM_STREAM>>>(d_s, d_ns, n1, d_res);
+    FOM_PSYNC();
+    FOM_PROF_END(soft_update);
+}
+
+fom_int fom_solve(FomWorkspace *work) {
+    const int n0 = FOM_T0_DIM;
+    FOM_PROF_BEGIN(upload);
+    cudaMemcpyAsync(d_s, work->state, sizeof(fom_float) * FOM_STATE_DIM, cudaMemcpyHostToDevice, FOM_STREAM);
+    cudaMemcpyAsync(d_p, work->params.p0, sizeof(fom_float) * FOM_PARAM_P0_DIM, cudaMemcpyHostToDevice, FOM_STREAM);
     /* constant part of t0: P @ p0 */
-    cudaMemset(d_c, 0, sizeof(fom_float) * n0);
+    cudaMemsetAsync(d_c, 0, sizeof(fom_float) * n0);
     fom_apply_t0_p_p0(d_p, d_c);
+    cudaMemcpyAsync(d_t0, d_c, sizeof(fom_float) * n0, cudaMemcpyDeviceToDevice);
+    FOM_PSYNC();
+    FOM_PROF_END(upload);
 
     int maxit = work->settings.max_iter;
     fom_float res = 0;
-    int k;
-    for (k = 0; k < maxit; k++) {
-        cudaMemcpyAsync(d_t0, d_c, sizeof(fom_float) * n0, cudaMemcpyDeviceToDevice);
-        fom_apply_t0_state(d_s, d_t0);
-        if (FOM_T0_LAMBDA != (fom_float)0) k_soft<<<g0, B>>>(d_t0, n0, FOM_T0_LAMBDA);
-        cudaMemsetAsync(d_acc, 0, sizeof(fom_float) * n1);
-        fom_apply_t1_t_t0(d_t0, d_acc);
-        k_t1<<<g1, B>>>(d_s, d_acc, d_ns, n1, FOM_T1_LAMBDA);
-        if (k == maxit - 1) {
-            k_maxdiff<<<1, 1024>>>(d_ns, d_s, n1, d_res);
-            cudaMemcpy(&res, d_res, sizeof(fom_float), cudaMemcpyDeviceToHost);
+    int k = 0;
+    if (maxit > 0) {
+        /* first iteration direct (it also triggers the one-time setup of the
+           fom_apply_* kernels, which cannot happen during capture) */
+        iterate(maxit == 1);
+        k = 1;
+    }
+#ifndef FOM_PROFILE
+    /* FOM_GRAPH_ITERS non-final iterations per graph launch */
+    if (maxit - 1 - k >= FOM_GRAPH_ITERS) {
+        if (!g_exec) {
+            cudaGraph_t g;
+            cudaStreamBeginCapture(FOM_STREAM, cudaStreamCaptureModeThreadLocal);
+            for (int j = 0; j < FOM_GRAPH_ITERS; j++) iterate(0);
+            cudaStreamEndCapture(FOM_STREAM, &g);
+            cudaGraphInstantiate(&g_exec, g, 0);
+            cudaGraphDestroy(g);
         }
-        fom_float *t = d_s; d_s = d_ns; d_ns = t;
+        while (maxit - 1 - k >= FOM_GRAPH_ITERS) {
+            cudaGraphLaunch(g_exec, FOM_STREAM);
+            k += FOM_GRAPH_ITERS;
+        }
     }
-    cudaMemcpy(work->state, d_s, sizeof(fom_float) * FOM_STATE_DIM, cudaMemcpyDeviceToHost);
+#endif
+    for (; k < maxit; k++) iterate(k == maxit - 1);
+    fom_float *fin = d_s;
+    FOM_PROF_BEGIN(download);
+    if (maxit > 0) cudaMemcpy(&res, d_res, sizeof(fom_float), cudaMemcpyDeviceToHost);
+    cudaMemcpy(work->state, fin, sizeof(fom_float) * FOM_STATE_DIM, cudaMemcpyDeviceToHost);
+    FOM_PROF_END(download);
     for (int i = 0; i < FOM_STATE_DIM; i++) work->solution.x[i] = work->state[i];
-    work->solution.iter = k;
+    work->solution.iter = maxit > 0 ? maxit : 0;
     work->solution.residual = res;
     work->solution.status = FOM_MAX_ITER_REACHED;
     return FOM_MAX_ITER_REACHED;
@@ -102,7 +159,11 @@
     (void)work;
     if (d_s) {
         cudaFree(d_s); cudaFree(d_ns); cudaFree(d_c); cudaFree(d_t0);
-        cudaFree(d_acc); cudaFree(d_p); cudaFree(d_res);
+        cudaFree(d_p); cudaFree(d_res);
         d_s = 0;
     }
+    if (g_exec) {
+        cudaGraphExecDestroy(g_exec);
+        g_exec = 0;
+    }
 }
--- src/fom_ops.cu (translated)
+++ src/fom_ops.cu (optimized)
@@ -1,6 +1,8 @@
 /* Deterministic device-side matrix-vector products (out += M @ x).
-   One warp per row for dense-enough rows, one thread per row for very
-   sparse ones. Matrix data lives in fom_ops_data.cuh as __device__ const
+   t0_state is 2D-tiled with its x slab staged in shared memory; t1_t_t0
+   uses a few lanes per row over uint16 column indices with L2 eviction
+   hints; the identity/diagonal ones
+   keep one thread per row. Matrix data lives in fom_ops_data.cuh as __device__ const
    arrays. All pointers are device pointers. */
 #include "fom_ops.h"
 #include "fom_ops_data.cuh"
@@ -8,20 +10,408 @@
 #include <cuda_runtime.h>
 
 #define FOM_OPS_THREADS 256
-
-static __global__ void fom_apply_t0_state_kern(const fom_float *x, fom_float *out) {
-    /* One warp per row: lanes stride the row (coalesced), then a shuffle sum. */
-    fom_int i = (blockIdx.x * blockDim.x + threadIdx.x) / 32;
-    fom_int lane = threadIdx.x % 32;
-    if (i >= 10000) return;  /* whole warps exit together */
-    fom_float s = 0.0;
-    for (fom_int k = FOM_OP_T0_STATE_ROWPTR[i] + lane; k < FOM_OP_T0_STATE_ROWPTR[i + 1]; k += 32) s += FOM_OP_T0_STATE_VALS[k] * x[FOM_OP_T0_STATE_COLIDX[k]];
-    for (int w = 16; w > 0; w /= 2) s += __shfl_down_sync(0xffffffff, s, w);
-    if (lane == 0) out[i] += s;
-}
-
+#ifndef T0_HINT
+#define T0_HINT 2
+#endif
+#ifndef T1_HINT
+#define T1_HINT 1
+#endif
+#ifndef T1_LANES
+#define T1_LANES 2
+#endif
+
+/* Padded row layout: both big products have < 65536 columns and rows of
+   nearly equal length, so at the first apply each is re-laid (on the
+   device, from its own CSR) into rows of stride W (max row length rounded
+   up to 4), with uint16 column indices and zero padding. Each lane then
+   reads groups of 4 entries with one 32-byte value load and one 8-byte
+   index load (vs. 8 scalar loads), cutting the load-instruction count of
+   the bandwidth-bound matvecs. */
+typedef struct {
+    const fom_int *rp;
+    unsigned short *col;
+    fom_float *val;
+    fom_int w;
+} fom_padded;
+
+static __global__ void fom_pad_rows(const fom_int *rp, const fom_int *ci, const fom_float *va,
+                                    unsigned short *col, fom_float *val, fom_int nrows, fom_int w) {
+    fom_int t = blockIdx.x * blockDim.x + threadIdx.x;
+    if (t >= nrows * w) return;
+    fom_int i = t / w, j = t % w;
+    fom_int k = rp[i] + j;
+    bool in = k < rp[i + 1];
+    col[t] = in ? (unsigned short)ci[k] : (unsigned short)0;
+    val[t] = in ? va[k] : 0.0;
+}
+
+static void fom_pad_setup(fom_padded *P, const fom_int *rp_d, const fom_int *ci_d, const fom_float *va_d,
+                          const fom_int *rp_h, fom_int nrows) {
+    fom_int m = 0;
+    for (fom_int i = 0; i < nrows; i++) if (rp_h[i + 1] - rp_h[i] > m) m = rp_h[i + 1] - rp_h[i];
+    P->w = (m + 3) & ~3;
+    P->rp = rp_d;
+    size_t n = (size_t)nrows * P->w;
+    cudaMalloc((void **)&P->col, n * sizeof(unsigned short));
+    cudaMalloc((void **)&P->val, n * sizeof(fom_float));
+    fom_pad_rows<<<(unsigned)((n + 255) / 256), 256>>>(rp_d, ci_d, va_d, P->col, P->val, nrows, P->w);
+}
+
+
+/* Interleaved ELL layout for t1_t_t0: its rows hold 64 or 65 entries, so
+   it is re-laid (on the device, from its own CSR) with every row padded to
+   W (even) entries (zero value, column 0), stored column-major in pairs:
+   entries 2q, 2q+1 of row i sit at pair index q * N + i. TPR threads take
+   a row (each a contiguous range of pairs, in order) and a warp reads
+   32/TPR consecutive rows' pairs with fully coalesced 16-byte value and
+   4-byte index loads; values bypass L1 (L2 evict-last) so L1 keeps x. */
+static __global__ void fom_ell_rows(const fom_int *rp, const fom_int *ci, const fom_float *va,
+                                    unsigned short *col, fom_float *val, fom_int nrows, fom_int w) {
+    fom_int t = blockIdx.x * blockDim.x + threadIdx.x;
+    if (t >= nrows * w) return;
+    fom_int i = t / w, j = t % w;
+    fom_int k = rp[i] + j;
+    bool in = k < rp[i + 1];
+    size_t o = ((size_t)(j >> 1) * nrows + i) * 2 + (j & 1);
+    col[o] = in ? (unsigned short)ci[k] : (unsigned short)0;
+    val[o] = in ? va[k] : 0.0;
+}
+
+template <int TPR>
+static __global__ void __launch_bounds__(256) fom_spmv_ell(const unsigned short *__restrict__ col, const fom_float *__restrict__ val,
+                                    const fom_float *__restrict__ x, fom_float *__restrict__ out, fom_int nrows, int NP) {
+    const int RPW = 32 / TPR;
+    fom_int t = blockIdx.x * blockDim.x + threadIdx.x;
+    int lane = threadIdx.x & 31, sub = lane / RPW;
+    fom_int i = (t >> 5) * RPW + lane % RPW;
+    bool live = i < nrows;
+    fom_float s = 0.0;
+    unsigned long long pol;
+    asm volatile("createpolicy.fractional.L2::evict_last.b64 %0;" : "=l"(pol));
+    if (live) {
+        const int q0 = sub * NP / TPR, q1 = (sub + 1) * NP / TPR;
+#pragma unroll 8
+        for (int q = q0; q < q1; q++) {
+            size_t o = (size_t)q * nrows + i;
+            double2 a;
+            unsigned c;
+            asm volatile("ld.global.cg.L2::cache_hint.v2.f64 {%0,%1}, [%2], %3;" : "=d"(a.x), "=d"(a.y) : "l"(val + 2 * o), "l"(pol));
+            asm volatile("ld.global.cg.L2::cache_hint.u32 %0, [%1], %2;" : "=r"(c) : "l"(col + 2 * o), "l"(pol));
+            s += a.x * __ldg(x + (c & 0xffffu));
+            s += a.y * __ldg(x + (c >> 16));
+        }
+    }
+#pragma unroll
+    for (int o = RPW; o < 32; o *= 2) s += __shfl_xor_sync(0xffffffff, s, o);
+    if (live && sub == 0) out[i] += s;
+}
+
+/* L2 eviction priority of the coefficient streams. The two big products
+   stream 52 MB per iteration, more than the 40 MB L2: HINT 1 marks a
+   product's values and indices evict-last (they stay L2-resident across
+   iterations), HINT 2 streams a product evict-first so it does not displace
+   them. */
+template <int HINT>
+static __device__ __forceinline__ void fom_ld4(const fom_float *v, const unsigned short *c, unsigned long long pol,
+                                               double2 &a, double2 &b, ushort4 &ci) {
+    if (HINT == 0) {
+        a = __ldg((const double2 *)v);
+        b = __ldg((const double2 *)(v + 2));
+        ci = __ldg((const ushort4 *)c);
+    } else if (HINT == 2) {
+        a = __ldcs((const double2 *)v);
+        b = __ldcs((const double2 *)(v + 2));
+        ci = __ldcs((const ushort4 *)c);
+    } else {
+        asm volatile("ld.global.nc.L2::cache_hint.v2.f64 {%0,%1}, [%2], %3;" : "=d"(a.x), "=d"(a.y) : "l"(v), "l"(pol));
+        asm volatile("ld.global.nc.L2::cache_hint.v2.f64 {%0,%1}, [%2], %3;" : "=d"(b.x), "=d"(b.y) : "l"(v + 2), "l"(pol));
+        asm volatile("ld.global.nc.L2::cache_hint.v4.u16 {%0,%1,%2,%3}, [%4], %5;"
+                     : "=h"(ci.x), "=h"(ci.y), "=h"(ci.z), "=h"(ci.w) : "l"(c), "l"(pol));
+    }
+}
+
+/* LANES consecutive lanes per row (LANES divides 32); lane l takes groups
+   of 4 entries l, l+LANES, ... (fixed order), then a fixed-order
+   xor-shuffle tree (deterministic). */
+template <int LANES, int HINT>
+static __global__ void fom_spmv_pad(const fom_int *__restrict__ rowptr, const unsigned short *__restrict__ col,
+                                    const fom_float *__restrict__ vals, fom_int w, const fom_float *__restrict__ x,
+                                    fom_float *__restrict__ out, fom_int nrows) {
+    fom_int t = blockIdx.x * blockDim.x + threadIdx.x;
+    fom_int i = t / LANES;
+    fom_int lane = t % LANES;
+    bool live = i < nrows;
+    fom_float s = 0.0;
+    unsigned long long pol = 0;
+    if (HINT == 1) asm volatile("createpolicy.fractional.L2::evict_last.b64 %0;" : "=l"(pol));
+    if (live) {
+        fom_int n = __ldg(rowptr + i + 1) - __ldg(rowptr + i);
+        const fom_float *vr = vals + (size_t)i * w;
+        const unsigned short *cr = col + (size_t)i * w;
+#pragma unroll 4
+        for (fom_int g = lane * 4; g < n; g += 4 * LANES) {
+            double2 a, b;
+            ushort4 c;
+            fom_ld4<HINT>(vr + g, cr + g, pol, a, b, c);
+            fom_float x0 = __ldg(x + c.x), x1 = __ldg(x + c.y), x2 = __ldg(x + c.z), x3 = __ldg(x + c.w);
+            s += a.x * x0;
+            if (g + 1 < n) s += a.y * x1;
+            if (g + 2 < n) s += b.x * x2;
+            if (g + 3 < n) s += b.y * x3;
+        }
+    }
+#pragma unroll
+    for (int o = LANES / 2; o > 0; o /= 2) s += __shfl_xor_sync(0xffffffff, s, o, LANES);
+    if (live && lane == 0) out[i] += s;
+}
+
+/* t0_state (10000 x 40000, ~260 random columns per row) is bound not by
+   its value stream but by the L2 traffic of its random gathers of x
+   (320 KB, each gather a 32-byte sector, little reuse per SM). It is
+   therefore 2D-tiled: a block takes a tile of rs rows x cs columns, stages
+   that x slab (32 KB) in shared memory with coalesced loads, gathers from
+   there and writes one partial sum per row; x is read only nrg times
+   (6.4 MB vs 83 MB of sectors). The tile layout is built at the first
+   apply, on the host, from this matrix's own CSR. */
+template <int LANES, int HINT>
+static __global__ void fom_spmv_tile(const int2 *__restrict__ rr, const unsigned short *__restrict__ col,
+                                     const fom_float *__restrict__ val, const fom_float *__restrict__ x,
+                                     fom_float *__restrict__ part, unsigned *__restrict__ cnt,
+                                     fom_float *__restrict__ out, fom_int nr, fom_int nc, fom_int rs, fom_int cs, fom_int nct) {
+    extern __shared__ fom_float xs[];
+    __shared__ int last;
+    const fom_int rg = blockIdx.x / nct, ct = blockIdx.x % nct;
+    const fom_int c0 = ct * cs, cn = min(cs, nc - c0), r0 = rg * rs, rn = min(rs, nr - r0);
+    for (fom_int k = threadIdx.x; k < cn; k += blockDim.x) xs[k] = __ldg(x + c0 + k);
+    __syncthreads();
+    const fom_int lane = threadIdx.x % LANES, nslot = blockDim.x / LANES;
+    const int2 *rb = rr + (size_t)blockIdx.x * rs;
+    for (fom_int base = 0; base < rn; base += nslot) {
+        fom_int lr = base + threadIdx.x / LANES;
+        bool live = lr < rn;
+        fom_float s = 0.0;
+        if (live) {
+            int2 q = __ldcs(rb + lr);
+            unsigned long long pol = 0;
+            if (HINT == 1) asm volatile("createpolicy.fractional.L2::evict_last.b64 %0;" : "=l"(pol));
+#pragma unroll 4
+            for (fom_int g = lane * 2; g < q.y; g += 2 * LANES) {
+                double2 a;
+                ushort2 c;
+                if (HINT == 1) {
+                    asm volatile("ld.global.nc.L2::cache_hint.v2.f64 {%0,%1}, [%2], %3;" : "=d"(a.x), "=d"(a.y) : "l"(val + q.x + g), "l"(pol));
+                    asm volatile("ld.global.nc.L2::cache_hint.v2.u16 {%0,%1}, [%2], %3;" : "=h"(c.x), "=h"(c.y) : "l"(col + q.x + g), "l"(pol));
+                } else {
+                    a = __ldcs((const double2 *)(val + q.x + g));
+                    c = __ldcs((const ushort2 *)(col + q.x + g));
+                }
+                s += a.x * xs[c.x];
+                if (g + 1 < q.y) s += a.y * xs[c.y];
+            }
+        }
+#pragma unroll
+        for (int o = LANES / 2; o > 0; o /= 2) s += __shfl_xor_sync(0xffffffff, s, o, LANES);
+        if (live && lane == 0) part[(size_t)ct * nr + r0 + lr] = s;
+    }
+    /* the last column tile of this row group to finish sums the partials
+       in fixed column-tile order (deterministic) */
+    __threadfence();
+    __syncthreads();
+    if (threadIdx.x == 0) last = atomicAdd(cnt + rg, 1u) == (unsigned)(nct - 1);
+    __syncthreads();
+    if (!last) return;
+    __threadfence();
+    for (fom_int lr = threadIdx.x; lr < rn; lr += blockDim.x) {
+        fom_float s = 0.0;
+        for (fom_int c = 0; c < nct; c++) s += __ldcg(part + (size_t)c * nr + r0 + lr);
+        out[r0 + lr] += s;
+    }
+    if (threadIdx.x == 0) cnt[rg] = 0;
+}
+
+/* host: re-lay CSR (rp, ci, va) into tiles of rs rows x cs columns; tile
+   b = rg * nct + ct holds, for each of its rows, the entries with columns
+   in the tile (local ushort index, order kept), padded to an even count.
+   Returns the number of stored entries. */
+static fom_int fom_tile_build(const fom_int *rp, const fom_int *ci, const fom_float *va, fom_int nr, fom_int nc,
+                              fom_int rs, fom_int cs, int2 *rr, unsigned short *cl, fom_float *vl) {
+    fom_int nrg = (nr + rs - 1) / rs, nct = (nc + cs - 1) / cs, t = 0;
+    for (fom_int rg = 0; rg < nrg; rg++)
+        for (fom_int ct = 0; ct < nct; ct++)
+            for (fom_int lr = 0; lr < rs; lr++) {
+                fom_int i = rg * rs + lr, o = t;
+                if (i < nr)
+                    for (fom_int k = rp[i]; k < rp[i + 1]; k++) {
+                        if (ci[k] / cs != ct) continue;
+                        cl[t] = (unsigned short)(ci[k] - ct * cs);
+                        vl[t] = va[k];
+                        t++;
+                    }
+                rr[(size_t)(rg * nct + ct) * rs + lr] = make_int2(o, t - o);
+                if (t & 1) { cl[t] = 0; vl[t] = 0.0; t++; }
+            }
+    return t;
+}
+
+/* Warp-chunked tiles: each tile (rg, ct) of rs rows x cs columns gets
+   one thread per row. Its rows are sorted by their entry count in the tile
+   (descending) and dealt to warps of 32; a warp chunk stores the entries
+   of its 32 rows column-major in pairs (pair q of lane l at off + q*32 + l),
+   padded (zero value, column 0) to the chunk's longest row, so a warp
+   reads 512 contiguous bytes of values per step with no per-row
+   descriptor loads. Each row sums its tile entries sequentially in CSR
+   order. cd[chunk] = {pair offset, pairs}; pm[slot] = local row (or
+   0xffff for an empty slot). */
+template <int RSP>
+static __global__ void __launch_bounds__(RSP) fom_spmv_tile2(const int2 *__restrict__ cd, const unsigned short *__restrict__ pm,
+                                     const unsigned *__restrict__ col, const double2 *__restrict__ val, const fom_float *__restrict__ x,
+                                     fom_float *__restrict__ part, unsigned *__restrict__ cnt,
+                                     fom_float *__restrict__ out, fom_int nr, fom_int nc, fom_int rs, fom_int cs, fom_int nct) {
+    extern __shared__ fom_float xs[];
+    __shared__ int last;
+    const fom_int rg = blockIdx.x / nct, ct = blockIdx.x % nct;
+    const fom_int c0 = ct * cs, cn = min(cs, nc - c0), r0 = rg * rs, rn = min(rs, nr - r0);
+    const int lane = threadIdx.x & 31;
+    const int2 d = __ldg(cd + (size_t)blockIdx.x * (RSP / 32) + (threadIdx.x >> 5));
+    const unsigned r = __ldg(pm + (size_t)blockIdx.x * RSP + threadIdx.x);
+    for (fom_int k = threadIdx.x; k < cn; k += RSP) xs[k] = __ldg(x + c0 + k);
+    __syncthreads();
+    fom_float s = 0.0;
+    const double2 *vp = val + d.x + lane;
+    const unsigned *cp = col + d.x + lane;
+#pragma unroll 4
+    for (int q = 0; q < d.y; q++) {
+        double2 a = __ldcs(vp + q * 32);
+        unsigned c = __ldcs(cp + q * 32);
+        s += a.x * xs[c & 0xffffu];
+        s += a.y * xs[c >> 16];
+    }
+    if (r != 0xffffu) part[(size_t)ct * nr + r0 + r] = s;
+    /* the last column tile of this row group to finish sums the partials
+       in fixed column-tile order (deterministic) */
+    __threadfence();
+    __syncthreads();
+    if (threadIdx.x == 0) last = atomicAdd(cnt + rg, 1u) == (unsigned)(nct - 1);
+    __syncthreads();
+    if (!last) return;
+    __threadfence();
+    for (fom_int lr = threadIdx.x; lr < rn; lr += RSP) {
+        fom_float t = 0.0;
+        for (fom_int c = 0; c < nct; c++) t += __ldcg(part + (size_t)c * nr + r0 + lr);
+        out[r0 + lr] += t;
+    }
+    if (threadIdx.x == 0) cnt[rg] = 0;
+}
+
+/* host: build the warp-chunked tiles (see fom_spmv_tile2) from CSR
+   (rp, ci, va); rsp = rs rounded up to 32. Returns stored pairs. */
+static fom_int fom_tile2_build(const fom_int *rp, const fom_int *ci, const fom_float *va, fom_int nr, fom_int nc,
+                               fom_int rs, fom_int cs, fom_int rsp, int2 *cd, unsigned short *pm,
+                               unsigned short *cl, fom_float *vl, fom_int *tcnt, fom_int *tbeg, fom_int *ord) {
+    fom_int nrg = (nr + rs - 1) / rs, nct = (nc + cs - 1) / cs, t = 0;
+    for (fom_int rg = 0; rg < nrg; rg++)
+        for (fom_int ct = 0; ct < nct; ct++) {
+            fom_int b = rg * nct + ct;
+            /* per-row count and first CSR position in this tile */
+            for (fom_int lr = 0; lr < rsp; lr++) {
+                fom_int i = rg * rs + lr;
+                tcnt[lr] = -1;
+                if (lr < rs && i < nr) {
+                    tcnt[lr] = 0;
+                    tbeg[lr] = rp[i];
+                    for (fom_int k = rp[i]; k < rp[i + 1]; k++) if (ci[k] / cs == ct) tcnt[lr]++;
+                }
+                ord[lr] = lr;
+            }
+            /* stable insertion sort of slots by count, descending */
+            for (fom_int u = 1; u < rsp; u++) {
+                fom_int v = ord[u], j = u;
+                while (j > 0 && tcnt[ord[j - 1]] < tcnt[v]) { ord[j] = ord[j - 1]; j--; }
+                ord[j] = v;
+            }
+            for (fom_int wc = 0; wc < rsp / 32; wc++) {
+                fom_int mx = 0;
+                for (int l = 0; l < 32; l++) if (tcnt[ord[wc * 32 + l]] > mx) mx = tcnt[ord[wc * 32 + l]];
+                fom_int np = (mx + 1) / 2;
+                cd[(size_t)b * (rsp / 32) + wc] = make_int2(t, np);
+                for (int l = 0; l < 32; l++) {
+                    fom_int lr = ord[wc * 32 + l], e = 0;
+                    pm[(size_t)b * rsp + wc * 32 + l] = tcnt[lr] < 0 ? (unsigned short)0xffff : (unsigned short)lr;
+                    for (fom_int q = 0; q < 2 * np; q++) { cl[2 * (t + q / 2 * 32 + l) + q % 2] = 0; vl[2 * (t + q / 2 * 32 + l) + q % 2] = 0.0; }
+                    if (tcnt[lr] > 0) {
+                        fom_int i = rg * rs + lr;
+                        for (fom_int k = tbeg[lr]; k < rp[i + 1]; k++) {
+                            if (ci[k] / cs != ct) continue;
+                            size_t o = 2 * (size_t)(t + e / 2 * 32 + l) + e % 2;
+                            cl[o] = (unsigned short)(ci[k] - ct * cs);
+                            vl[o] = va[k];
+                            e++;
+                        }
+                    }
+                }
+                t += np * 32;
+            }
+        }
+    return t;
+}
+
+#ifndef T0_RS
+#define T0_RS 500
+#endif
+#ifndef T0_CS
+#define T0_CS 4000
+#endif
+#define T0_RSP ((T0_RS + 31) / 32 * 32)
 extern "C" void fom_apply_t0_state(const fom_float *x, fom_float *out) {
-    fom_apply_t0_state_kern<<<1250, FOM_OPS_THREADS>>>(x, out);
+    enum { NR = 10000, NC = 40000, NNZ = 2599983, NRG = (NR + T0_RS - 1) / T0_RS, NCT = (NC + T0_CS - 1) / T0_CS,
+           NT = NRG * NCT, NCH = NT * (T0_RSP / 32) };
+    static int2 *cd;
+    static unsigned short *pm;
+    static unsigned *col;
+    static double2 *val;
+    static fom_float *part;
+    static unsigned *cnt;
+    static bool ready = false;
+    if (!ready) {
+        /* capacity in pairs: nnz/2 pairs plus at most 32 * (spread + 1) padding
+           pairs per chunk; NNZ + 32 * NCH leaves ample room */
+        const size_t CAP = (size_t)NNZ + (size_t)NCH * 32;
+        static fom_int rp_h[NR + 1], ci_h[NNZ], tcnt[T0_RSP], tbeg[T0_RSP], ord[T0_RSP];
+        static fom_float va_h[NNZ];
+        static int2 cd_h[NCH];
+        static unsigned short pm_h[NT * T0_RSP];
+        unsigned short *cl_h = 0;
+        fom_float *vl_h = 0;
+        cudaMallocHost((void **)&cl_h, 2 * CAP * sizeof(unsigned short));
+        cudaMallocHost((void **)&vl_h, 2 * CAP * sizeof(fom_float));
+        const fom_int *rp, *ci;
+        const fom_float *va;
+        cudaGetSymbolAddress((void **)&rp, FOM_OP_T0_STATE_ROWPTR);
+        cudaGetSymbolAddress((void **)&ci, FOM_OP_T0_STATE_COLIDX);
+        cudaGetSymbolAddress((void **)&va, FOM_OP_T0_STATE_VALS);
+        cudaMemcpy(rp_h, rp, sizeof(rp_h), cudaMemcpyDeviceToHost);
+        cudaMemcpy(ci_h, ci, sizeof(ci_h), cudaMemcpyDeviceToHost);
+        cudaMemcpy(va_h, va, sizeof(va_h), cudaMemcpyDeviceToHost);
+        fom_int t = fom_tile2_build(rp_h, ci_h, va_h, NR, NC, T0_RS, T0_CS, T0_RSP, cd_h, pm_h,
+                                    cl_h, vl_h, tcnt, tbeg, ord);
+        cudaMalloc((void **)&cd, sizeof(cd_h));
+        cudaMalloc((void **)&pm, sizeof(pm_h));
+        cudaMalloc((void **)&col, t * sizeof(unsigned));
+        cudaMalloc((void **)&val, t * sizeof(double2));
+        cudaMalloc((void **)&part, (size_t)NCT * NR * sizeof(fom_float));
+        cudaMalloc((void **)&cnt, NRG * sizeof(unsigned));
+        cudaMemset(cnt, 0, NRG * sizeof(unsigned));
+        cudaMemcpy(cd, cd_h, sizeof(cd_h), cudaMemcpyHostToDevice);
+        cudaMemcpy(pm, pm_h, sizeof(pm_h), cudaMemcpyHostToDevice);
+        cudaMemcpy(col, cl_h, t * sizeof(unsigned), cudaMemcpyHostToDevice);
+        cudaMemcpy(val, vl_h, t * sizeof(double2), cudaMemcpyHostToDevice);
+        cudaFreeHost(cl_h);
+        cudaFreeHost(vl_h);
+        cudaFuncSetAttribute(fom_spmv_tile2<T0_RSP>, cudaFuncAttributeMaxDynamicSharedMemorySize, T0_CS * sizeof(fom_float));
+        ready = true;
+    }
+    fom_spmv_tile2<T0_RSP><<<NT, T0_RSP, T0_CS * sizeof(fom_float), cudaStreamPerThread>>>(
+        cd, pm, col, val, x, part, cnt, out, NR, NC, T0_RS, T0_CS, NCT);
 }
 
 static __global__ void fom_apply_t0_p_p0_kern(const fom_float *x, fom_float *out) {
@@ -48,19 +438,33 @@
     fom_apply_t1_state_kern<<<157, FOM_OPS_THREADS>>>(x, out);
 }
 
-static __global__ void fom_apply_t1_t_t0_kern(const fom_float *x, fom_float *out) {
-    /* One warp per row: lanes stride the row (coalesced), then a shuffle sum. */
-    fom_int i = (blockIdx.x * blockDim.x + threadIdx.x) / 32;
-    fom_int lane = threadIdx.x % 32;
-    if (i >= 40000) return;  /* whole warps exit together */
-    fom_float s = 0.0;
-    for (fom_int k = FOM_OP_T1_T_T0_ROWPTR[i] + lane; k < FOM_OP_T1_T_T0_ROWPTR[i + 1]; k += 32) s += FOM_OP_T1_T_T0_VALS[k] * x[FOM_OP_T1_T_T0_COLIDX[k]];
-    for (int w = 16; w > 0; w /= 2) s += __shfl_down_sync(0xffffffff, s, w);
-    if (lane == 0) out[i] += s;
-}
-
+#ifndef T1_TPR
+#define T1_TPR 2
+#endif
 extern "C" void fom_apply_t1_t_t0(const fom_float *x, fom_float *out) {
-    fom_apply_t1_t_t0_kern<<<5000, FOM_OPS_THREADS>>>(x, out);
+    enum { N = 40000 };
+    static unsigned short *col;
+    static fom_float *val;
+    static fom_int w;
+    static bool ready = false;
+    if (!ready) {
+        const fom_int *rp, *ci;
+        const fom_float *va;
+        static fom_int rp_h[N + 1];
+        cudaGetSymbolAddress((void **)&rp, FOM_OP_T1_T_T0_ROWPTR);
+        cudaMemcpy(rp_h, rp, sizeof(rp_h), cudaMemcpyDeviceToHost);
+        w = 0;
+        for (fom_int i = 0; i < N; i++) if (rp_h[i + 1] - rp_h[i] > w) w = rp_h[i + 1] - rp_h[i];
+        w = (w + 1) & ~1;
+        cudaGetSymbolAddress((void **)&ci, FOM_OP_T1_T_T0_COLIDX);
+        cudaGetSymbolAddress((void **)&va, FOM_OP_T1_T_T0_VALS);
+        cudaMalloc((void **)&col, (size_t)N * w * sizeof(unsigned short));
+        cudaMalloc((void **)&val, (size_t)N * w * sizeof(fom_float));
+        fom_ell_rows<<<(N * w + 255) / 256, 256>>>(rp, ci, va, col, val, N, w);
+        cudaFuncSetAttribute(fom_spmv_ell<T1_TPR>, cudaFuncAttributePreferredSharedMemoryCarveout, 0);
+        ready = true;
+    }
+    fom_spmv_ell<T1_TPR><<<(N * T1_TPR + FOM_OPS_THREADS - 1) / FOM_OPS_THREADS, FOM_OPS_THREADS, 0, cudaStreamPerThread>>>(col, val, x, out, N, w / 2);
 }
 
 static __global__ void fom_apply_su_t_t1_kern(const fom_float *x, fom_float *out) {
```

## Agent notes

```
### Round 1

Initial whole-solve mean_us: 18438.7. Final mean_us: 12907.0. Speedup: 1.43x.

Final profile (from a `-DFOM_PROFILE` build that syncs the device per region, so absolute times are inflated: 15.67 ms there vs 12.91 ms in the real bench):
- apply_t0_state (A s, 10000x40000, 260 nnz/row): 42.1 us/iter, 53.9%
- apply_t1_t_t0 (G t0, 40000x10000, 65 nnz/row): 27.1 us/iter, 34.7%
- soft_update (soft-threshold, re-arm accumulators, final residual): 8.0 us/iter, 10.3%
- download: 0.7%; upload (incl. P p0): 0.4%

Optimizations applied:
- uint16 column indices for both big CSR products, built once on the first apply call. Both have fewer than 65536 columns (40000 and 10000), so each nonzero costs 10 B instead of 12 B. The products are bandwidth-bound at about 26 MB each per iteration. (18.4 -> 15.3 ms, together with the next item.)
- Folded the per-iteration memset of acc and the memcpy of c into t0 into the soft-threshold update kernel. The t0_p_p0 term (P p0) does not change across iterations, so it is computed once per solve.
- Retuned threads per row to match the nearly uniform row lengths (257-260 and 64-65 nnz): 8 lanes per row for A s and 4 for G t0, with a fixed-order xor-shuffle reduction so results are deterministic. (-> 14.6 ms)
- L2 eviction-priority hints. The two products stream 52 MB per iteration, more than the A100's 40 MB L2. t1_t_t0's values and indices are loaded evict_last (an `L2::cache_hint` policy) and t0_state's are loaded evict_first (`__ldcs`), so more of t1's data stays in L2 from one iteration to the next. (-> 12.9 ms)
- Kept `FOM_PROF_BEGIN/END` around the upload, the two applies, the soft update and the download.

Tried and reverted (slower):
- skipping zero entries of the iterate s (it is not sparse enough)
- unrolled multi-load chunks per thread
- a block-flat CSR kernel staged through shared memory (2x slower, because shared memory took L1 away from the x gathers)
- column tiling of A to keep x slices in L1
- raising the persisting-L2 limit
- swapping which matrix gets evict_last, or applying it to t1's values only
- other lane widths

Blocked by rules:
- Capture the 200-iteration loop in a CUDA graph | the `fom_apply_*` signatures are fixed and launch on the legacy default stream, which cannot be stream-captured | ~3-5% (launch gaps)
- Fuse the soft-threshold/state update into the epilogue of the t1_t_t0 matvec | `fom_apply_<name>` must compute exactly out += M x, and matvec loops may not live in `fom_solver.cu` | ~5-10% (saves one kernel and a 320 KB round trip per iteration)
- Use t1_t_t0's relation to t0_state (it appears to be a scaled A^T) to keep one 26 MB matrix, which would fit entirely in L2, instead of two | each `fom_apply_<name>` may read only its own matrix's data | potentially large, maybe 1.5-2x, since both products would run from L2

### Round 2

**Whole-solve `mean_us`:** 13028.8 us at the start of round 2, 11430.2 us at the end: 1.14x this round. Against the translated solver (18047.9 us) it is 1.58x overall.

**Final profile breakdown (`-DFOM_PROFILE` build, hottest first):**

| region | total_us | calls | pct |
|---|---|---|---|
| apply_t0_state (`A s` + `P p0` term, streamed from DRAM) | 1108034 | 26800 | 55.3% |
| apply_t1_t_t0 (`-step A^T t0`, L2-resident) | 659625 | 26800 | 32.9% |
| soft_update (soft-threshold, accumulator reset, final residual) | 222189 | 26800 | 11.1% |
| upload | 8216 | 134 | 0.4% |
| download | 6521 | 134 | 0.3% |

The profile build syncs after every region and skips the CUDA graph, so its mean is 15.0 ms. The production build runs about 57 us per iteration.

**Optimizations applied:**
- **Padded row layout with vector loads** (`fom_ops.cu`, both big matvecs): 13108 → 12738 us. Row lengths are nearly uniform (`A` has 257–260 entries per row, `A^T` has 64–65) and both matrices have fewer than 65536 columns. So at the first apply, each matrix is re-laid on the device from its own CSR. Rows get a stride of the maximum row length rounded up to 4, with zero padding and uint16 column indices. Each lane then reads 4 entries with two 16-byte value loads and one 8-byte index load, instead of 8 scalar loads. Lanes per row were retuned to 8 for `A` and 2 for `A^T`. Before this change the kernels were limited by load-instruction issue: they reached only 34% of peak DRAM throughput (`A`) and 47% of peak L2 throughput (`A^T`).
- **Max-L1 carveout** on the two SpMV kernels: → 12373 us. They use no shared memory, so all of L1 can cache the gathered vector. `t0` (80 KB) fits entirely, and more of `s` (320 KB) stays cached. A diagnostic build showed the random gather of `s` costs about 14 us of the ~43 us `A s` call.
- **CUDA graph of 66 iterations** (`fom_solver.cu`, plus launching the two SpMVs on `cudaStreamPerThread` in `fom_ops.cu`): about −4.5%. The run uses a fixed number of iterations (200) with ping-pong buffers, so the body can be captured once and replayed. The first and last iterations run directly: the first does the one-time layout setup, the last computes the residual. The per-thread default stream synchronizes with the legacy stream, so the direct checks still see the results.
- **Unroll 4 in the padded SpMV loop:** about −2.5%. Each lane keeps more 32-byte value sectors and index loads in flight, which matters for the DRAM-latency-bound `A s` (only about 36% of warp slots are active).

**Tried and rejected (measured slower or no change):**
- Batching K scalar loads per lane before the gathers: no change.
- Marking part of `A`'s stream evict-last too (fractional L2 policy at 0.25 and 0.5): slower.
- Swapping which matrix stays L2-resident: about 13.6 ms, slower.
- `L1::no_allocate` on the matrix streams: 12.6–13.0 ms, slower.
- Non-volatile hint asm, and other lane counts (4/2, 8/1, 16/2): within noise.
- Skipping value loads when `x` is zero: no use, because the iterates are dense (about 0.01% zeros).

**Blocked by rules:**
- Computing `G(A s)` with one matrix's data (`A^T` values are `-step × A` values), so only one 21 MB value array is streamed per iteration, which would fit the 40 MB L2 together with the indices | `fom_apply_<name>` may read only its own matrix's data (and its signature is fixed) | ~35–40% (about 40 us per iteration less DRAM traffic)
- Fusing the soft-threshold into the epilogue of the `A^T` matvec (`ns = soft(s + G t0)` in one kernel), dropping the separate update kernel | each `fom_apply_<name>` must apply exactly `M` and accumulate into `out` | ~5% (about 3–4 us per iteration)
- Fusing the `A s` and `A^T t0` launches into one persistent cooperative kernel with a grid-wide barrier | matrix-vector loops must live inside their own `fom_apply_<name>` and may not be written in `fom_solver.cu` | unknown, ~2–3%

### Round 3

**Whole-solve `mean_us`:** 11404.5 at the start of this round, 9811.3 at the end (1.16x this round). Against the translated solver (18047.9) that is 1.84x overall. `make verify` prints `VERIFY OK`, worst error 3.6e-15.

**Final profile breakdown (per iteration, 200 iterations per solve).** `make profile` does not exist in this Makefile and `nsys` failed, so these times come from cudaEvent timings of each kernel run alone. The `FOM_PROF_BEGIN/END` regions are still in `fom_solver.cu`.

| region | time per iteration | share |
|---|---|---|
| `apply_t0_state` (A·s, tiled kernel) | ~27 µs | ~55% |
| `apply_t1_t_t0` (G·t0, in place into s) | ~20 µs | ~41% |
| `soft_update` (in-place soft threshold plus t0 = c re-arm) | ~2 µs | ~4% |
| upload / download | once per solve | <1% |

**Optimizations applied:**
1. **2D-tiled A·s with the `x` slab in shared memory** (`fom_apply_t0_state`). Measurements showed this product was limited by L2 traffic from its random gathers of the 320 KB `x`, not by its value stream. Each gather costs a 32-byte sector and gets about 0.6 uses per SM, so ~83 MB of sectors per product. It was also not limited by occupancy: lane counts 4 to 16 all took 37 to 47 µs.
   - **New layout:** at the first apply, the matrix's own CSR is re-laid on the host into tiles of 500 rows × 4000 columns. Each tile stores uint16 local column indices, and each row-tile is padded to an even length.
   - **Kernel:** each block stages its 32 KB `x` slab with coalesced loads and gathers from shared memory. It writes one partial sum per row. The last tile of each row group to finish (found with an atomic counter) sums the partials in a fixed column-tile order, so results are deterministic.
   - **Effect:** `x` is now read 20 times (6.4 MB) instead of as 83 MB of sectors. The kernel went from 37 to 41 µs down to about 27 µs.
   - **What made it valid:** about 260 random columns per row, under 65536 columns, and a dense iterate.
   - **Tried first:** a single 160 KB column-half per SM. It lost, because 108 SMs × 320 KB of `x` staging cost about 15 µs.
2. **In-place ISTA update.** `t1_state` and `su_t_t1` are identity matrices. So `fom_apply_t1_t_t0` now accumulates G·t0 directly into `s`, and `s` is then soft-thresholded in place. This removed the accumulator buffer, its zeroing, and the s + acc read. Only the last iteration keeps a copy of the previous iterate, for the residual. Gain about 1%.

**Tried and reverted (verified, but not faster):**
- **Nonzero-bitmap skipping in A·s:** the iterate is fully dense (lambda ≈ 3e-4 and the initial state is nonzero), so it was 30% slower.
- **Prefetching row ranges and first value groups before staging `x`:** no gain. With 2 blocks per SM the register cap is 32 and the prefetch arrays spill.
- **Tiling t1 the same way:** no gain, because its 80 KB `x` already caches well.
- **t1 with 4 or 8 lanes per row:** slower inside the captured graph.
- **Swapping which product is L2 evict-last:** slower.
- **Pinning t1's coefficients with an L2 persisting access-policy window** (via `cudaLaunchKernelEx`): +5% slower, with or without the evict-last hint.

Blocked by rules:
- Share the coefficients of `t0_state` (A) and `t1_t_t0` (−γAᵀ, the same nonzeros scaled) so one stream of about 26 MB serves both products, or keep that one copy L2-resident | each `fom_apply_<name>` may read only its own matrix's data | large, roughly 25-40% (DRAM traffic per iteration drops from 52 MB toward 26 MB)
- Fuse the soft threshold and t0 = c re-arm into the epilogue of the `t1_t_t0` matvec kernel | matvec code must live in `fom_apply_*`, which must compute exactly out += M·x | ~2 µs per iteration (~4%)
- Store the coefficients in float32 to halve the value stream | `fom_apply_<name>` must apply exactly M (checked directly against the reference) | ~35-40%, but it breaks exactness
```
