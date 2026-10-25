# Optimization report

## Timing

```
translated (emitted)   19006.2 us/solve
optimized               9125.6 us/solve
speedup                  2.08x
```

Optimization outcome: **kept (re-verified correct)**.

## Rounds

| round | before (us) | after (us) | gain | kept | agent (s) | outcome |
|---:|---:|---:|---:|:---:|---:|---|
| 1 | 19006.229 | 11589.662 | 39.0% | yes | 4711 | continued |
| 2 | 11638.041 | 11356.601 | 2.4% | yes | 4743 | continued |
| 3 | 11499.425 | 9125.579 | 20.6% | yes | 4434 | continued |
| 4 | - | - | - | no | 1541 | the round left the sources unchanged (agent stopped at its budget) |

## Caveats

- The agent was terminated at its wall-clock budget before it finished (in round 4), so it may never have written a summary. The timing and diff below are still sound: the solver on disk was re-verified and benchmarked. Raise `CodegenConfig.agent_optimize_timeout_s` to give it longer.

## Kernel checks

Each `fom_apply_<name>` in the shipped `fom_ops.c`, checked by `make verify` against the package's reference over the same `fom_data.c` matrix. *basis* probes every unit vector, which recovers the routine's matrix column by column (an exact check), plus random vectors; *random* uses random vectors only (the matrix is over the probe budget). A `fused_<block>` row is a `fom_fused_<block>`, checked against the sum of its members: *basis* there probes every unit vector of each source with every schedule value at 0 and with each alone at 1, which recovers every member exactly.

| matrix | shape | nnz | rewritten | check | probes | max error |
|---|---|---|---|---|---|---|
| `t0_state` | 10000x40000 | 2599983 | yes | random | 2 | 3.997e-15 |
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
| 1 | Capture the 200-iteration loop in a CUDA graph to cut launch and boundary overhead | fixed `fom_apply_*` signatures launch on the legacy default stream, which can't be stream-captured, and the Makefile/build flags (e.g. `--default-stream per-thread`) are fixed | about 1–3% |
| 1 | Fuse the soft-threshold/residual update into the t1_t_t0 matvec epilogue (saves one kernel and a 320 KB round trip per iteration) | activations must stay in `src/fom_solver.cu`, and `fom_ops.cu` routines may only accumulate `out += M x` (no `fom_fused_*` routine is configured) | about 5% |
| 1 | Share one copy of A between t0_state and t1_t_t0, which appears to be a scaled Aᵀ, roughly halving the matrix bytes | each `fom_apply_<name>` may read only its own matrix's data | potentially 30–40% |
| 1 | Run the iteration's three kernels as one persistent cooperative kernel with grid syncs | matrix products must go through separate `fom_apply_*` entry points called from the solver | unknown, probably about 5% |
| 2 | Capture the 200 iterations (or batches of them) in a CUDA graph to remove per-launch overhead and inter-kernel gaps | the fixed `fom_apply_*` signatures take no stream and launch on the legacy default stream, which cannot be captured; the solver also cannot reach non-static helpers in `fom_ops.cu` | about 2–5% |
| 2 | Fuse the soft-threshold/residual update into the epilogue of the t1_t_t0 matvec kernel, and the t0 re-arm into the t0_state kernel | `fom_apply_<name>` must compute only out += M x, and the solver may not contain matvec loops or read matrix data | about 5–8% (saves one launch and an extra pass over 1.3 MB per iteration) |
| 2 | One persistent cooperative kernel per iteration (both matvecs plus the update, with grid barriers) | matvec loops may not live in `src/fom_solver.cu`, and each `fom_apply_*` may read only its own matrix's data | unknown, possibly 10%+ |
| 2 | Run t1_t_t0 as a transpose product of t0_state's data (t1_t_t0 has the structure of a scaled A^T), so only one 26 MB matrix stream is needed and both fit in L2 | each `fom_apply_<name>` and its helpers may read only their own matrix's data | about 30–40% (the measured L2 thrash penalty) |
| 2 | Store matrix values in fp32 to halve the dominant 8-byte value stream | the direct ops check requires 1e-10 relative accuracy against the matrix | about 30% |
| 3 | Store A once and use it for both products: `t1_t_t0` equals −0.0147 · `t0_state`ᵀ exactly (same sparsity, one constant ratio), and the per-iteration traffic would drop from 52 MB to 26 MB, which fits in L2 | kernel rule: each `fom_apply_<name>` and its helpers read only their own matrix's data | ~35–45% of solve time (the products are bandwidth-bound and roughly half their bytes would go away) |
| 3 | Capture the 200-iteration loop in a CUDA graph to remove launch gaps between the 3 kernels per iteration | fixed `fom_apply_*` signatures and build flags: kernels launch on the legacy default stream, which cannot be stream-captured | ~2–3% (about 1 us/iteration) |
| 3 | Fuse the soft-threshold/state step into the matvec kernels (e.g. soft-threshold during t0's x fill, or in t1's epilogue) | kernel rule: `fom_apply_<name>` must compute exactly `out += M @ x`, and the solver must not contain matvec loops | ~4–6% (the t1_update kernel and one kernel boundary per iteration) |

## What the agent changed

```diff
--- src/fom_solver.cu (translated)
+++ src/fom_solver.cu (optimized)
@@ -5,11 +5,20 @@
 
 #include <cuda_runtime.h>
 #include <string.h>
+extern "C" {
+#include "fom_profile.h"
+}
+
+#ifdef FOM_PROFILE
+#define FOM_SYNC() cudaDeviceSynchronize()
+#else
+#define FOM_SYNC() ((void)0)
+#endif
 
 #define TPB 256
 #define NB(n) (((n) + TPB - 1) >> 8)
 
-static fom_float *d_state, *d_new, *d_p0, *d_t0, *d_acc;
+static fom_float *d_state, *d_new, *d_p0, *d_t0, *d_acc, *d_c0;
 static unsigned long long *d_res;
 
 /* t0 = soft(acc, lam) in place. */
@@ -23,28 +32,48 @@
 
 /* t1 = soft(state + acc, lam) (t1_state = identity), new_state = t1
    (su_t_t1 = identity); residual = max |new - state| via atomicMax on
-   the bits of a nonnegative double (order-independent). */
-static __global__ void t1_kern(const fom_float *state, const fom_float *acc,
-                               fom_float *out, int n, fom_float lam,
-                               unsigned long long *res) {
-    __shared__ fom_float sm[TPB];
+   the bits of a nonnegative double (order-independent). Also re-arms the
+   accumulators for the next iteration: acc = 0 and t0 = c0 (the
+   iteration-invariant t0_p_p0 @ p0 term), since both were consumed. */
+static __global__ void __launch_bounds__(TPB) t1_kern(const fom_float *__restrict__ state, fom_float *acc,
+                               fom_float *__restrict__ out, int n, fom_float lam,
+                               unsigned long long *res, fom_float *__restrict__ t0,
+                               const fom_float *__restrict__ c0, int n0) {
+    __shared__ fom_float sm[TPB / 32];
     int i = blockIdx.x * blockDim.x + threadIdx.x;
     fom_float d = 0;
     if (i < n) {
         fom_float s = state[i];
         fom_float a = s + acc[i];
+        acc[i] = 0;
         fom_float t = a > lam ? a - lam : (a < -lam ? a + lam : (fom_float)0);
         out[i] = t;
         d = fabs(t - s);
     }
-    sm[threadIdx.x] = d;
+    if (i < n0) t0[i] = c0[i];
+    for (int w = 16; w > 0; w >>= 1) d = fmax(d, __shfl_down_sync(0xffffffff, d, w));
+    if ((threadIdx.x & 31) == 0) sm[threadIdx.x >> 5] = d;
     __syncthreads();
-    for (int k = TPB >> 1; k > 0; k >>= 1) {
-        if (threadIdx.x < k) sm[threadIdx.x] = fmax(sm[threadIdx.x], sm[threadIdx.x + k]);
-        __syncthreads();
+    if (threadIdx.x < 32) {
+        d = threadIdx.x < TPB / 32 ? sm[threadIdx.x] : 0;
+        for (int w = 16; w > 0; w >>= 1) d = fmax(d, __shfl_down_sync(0xffffffff, d, w));
+        if (threadIdx.x == 0)
+            atomicMax(res, (unsigned long long)__double_as_longlong(d));
     }
-    if (threadIdx.x == 0)
-        atomicMax(res, (unsigned long long)__double_as_longlong(sm[0]));
+}
+
+/* In-place step for every iteration but the last: v holds
+   state + t1_t_t0 @ t0 (accumulated straight into the state buffer, as
+   t1_state = identity), so v = soft(v, lam) is the new state
+   (su_t_t1 = identity); also re-arms t0 = c0. No residual is needed. */
+static __global__ void __launch_bounds__(TPB) soft_step_kern(fom_float *__restrict__ v, int n, fom_float lam,
+                                      fom_float *__restrict__ t0, const fom_float *__restrict__ c0, int n0) {
+    int i = blockIdx.x * blockDim.x + threadIdx.x;
+    if (i < n) {
+        fom_float a = v[i];
+        v[i] = a > lam ? a - lam : (a < -lam ? a + lam : (fom_float)0);
+    }
+    if (i < n0) t0[i] = c0[i];
 }
 
 void fom_set_default_settings(FomSettings *s) {
@@ -61,6 +90,7 @@
         cudaMalloc((void **)&d_new, sizeof(fom_float) * FOM_STATE_DIM);
         cudaMalloc((void **)&d_acc, sizeof(fom_float) * FOM_T1_DIM);
         cudaMalloc((void **)&d_t0, sizeof(fom_float) * FOM_T0_DIM);
+        cudaMalloc((void **)&d_c0, sizeof(fom_float) * FOM_T0_DIM);
         cudaMalloc((void **)&d_p0, sizeof(fom_float) * FOM_PARAM_P0_DIM);
         cudaMalloc((void **)&d_res, sizeof(unsigned long long));
     }
@@ -78,16 +108,34 @@
     cudaMemcpy(d_p0, work->params.p0, sizeof(fom_float) * FOM_PARAM_P0_DIM, cudaMemcpyHostToDevice);
     fom_int iter = 0;
     fom_float res = 0;
+    /* Iteration-invariant: c0 = t0_p_p0 @ p0; t0 starts at c0, acc at 0. */
+    cudaMemsetAsync(d_c0, 0, sizeof(fom_float) * n0);
+    fom_apply_t0_p_p0(d_p0, d_c0);
+    cudaMemcpyAsync(d_t0, d_c0, sizeof(fom_float) * n0, cudaMemcpyDeviceToDevice);
+    cudaMemsetAsync(d_acc, 0, sizeof(fom_float) * n1);
     for (iter = 0; iter < max_iter; ++iter) {
-        cudaMemsetAsync(d_t0, 0, sizeof(fom_float) * n0);
+        const int last = iter == max_iter - 1;
+        FOM_PROF_BEGIN(t0_matvec);
         fom_apply_t0_state(d_state, d_t0);
-        fom_apply_t0_p_p0(d_p0, d_t0);
         if (FOM_T0_LAMBDA != (fom_float)0) soft_kern<<<NB(n0), TPB>>>(d_t0, n0, FOM_T0_LAMBDA);
-        cudaMemsetAsync(d_acc, 0, sizeof(fom_float) * n1);
-        fom_apply_t1_t_t0(d_t0, d_acc);
-        if (iter == max_iter - 1) cudaMemsetAsync(d_res, 0, sizeof(unsigned long long));
-        t1_kern<<<NB(n1), TPB>>>(d_state, d_acc, d_new, n1, FOM_T1_LAMBDA, d_res);
-        fom_float *tmp = d_state; d_state = d_new; d_new = tmp;
+        FOM_SYNC();
+        FOM_PROF_END(t0_matvec);
+        FOM_PROF_BEGIN(t1_matvec);
+        /* The last iteration keeps the old state for the residual, so it
+           accumulates into the zeroed d_acc; the others into the state. */
+        fom_apply_t1_t_t0(d_t0, last ? d_acc : d_state);
+        FOM_SYNC();
+        FOM_PROF_END(t1_matvec);
+        FOM_PROF_BEGIN(t1_update);
+        if (last) {
+            cudaMemsetAsync(d_res, 0, sizeof(unsigned long long));
+            t1_kern<<<NB(n1), TPB>>>(d_state, d_acc, d_new, n1, FOM_T1_LAMBDA, d_res, d_t0, d_c0, n0);
+            fom_float *tmp = d_state; d_state = d_new; d_new = tmp;
+        } else {
+            soft_step_kern<<<NB(n1), TPB>>>(d_state, n1, FOM_T1_LAMBDA, d_t0, d_c0, n0);
+        }
+        FOM_SYNC();
+        FOM_PROF_END(t1_update);
     }
     if (max_iter > 0) {
         unsigned long long bits;
@@ -106,6 +154,6 @@
 void fom_cleanup(FomWorkspace *work) {
     (void)work;
     cudaFree(d_state); cudaFree(d_new); cudaFree(d_acc);
-    cudaFree(d_t0); cudaFree(d_p0); cudaFree(d_res);
+    cudaFree(d_t0); cudaFree(d_p0); cudaFree(d_c0); cudaFree(d_res);
     d_state = 0;
 }
--- src/fom_ops.cu (translated)
+++ src/fom_ops.cu (optimized)
@@ -1,6 +1,7 @@
 /* Deterministic device-side matrix-vector products (out += M @ x).
-   One warp per row for dense-enough rows, one thread per row for very
-   sparse ones. Matrix data lives in fom_ops_data.cuh as __device__ const
+   The two large products (t0_state, t1_t_t0) use repacked copies of their
+   own data built on first call (16-bit columns); the small identity-like
+   ones keep one thread per row. Matrix data lives in fom_ops_data.cuh as __device__ const
    arrays. All pointers are device pointers. */
 #include "fom_ops.h"
 #include "fom_ops_data.cuh"
@@ -9,19 +10,233 @@
 
 #define FOM_OPS_THREADS 256
 
-static __global__ void fom_apply_t0_state_kern(const fom_float *x, fom_float *out) {
-    /* One warp per row: lanes stride the row (coalesced), then a shuffle sum. */
-    fom_int i = (blockIdx.x * blockDim.x + threadIdx.x) / 32;
-    fom_int lane = threadIdx.x % 32;
-    if (i >= 10000) return;  /* whole warps exit together */
+/* t0_state: 10000 x 40000, ~260 nnz per row in random columns, so the
+   cost is the random gathers of x (320 KB, too big for L1). The columns
+   are split in two halves of 20000 (160 KB each, fits in one SM's shared
+   memory): half the SMs stage x[0:20000) in shared memory, the other half
+   x[20000:40000), and each pair of blocks (same rows) computes the two
+   partial row sums from shared memory. The pair's last block to finish
+   adds part0 + part1 into out (fixed order, deterministic). Column indices
+   are repacked once as 16-bit offsets within their part, plus per-row
+   split points. Falls back to a plain CSR kernel if the device cannot give
+   a block 160 KB of shared memory. */
+#ifndef T0_P
+#define T0_P 4      /* column parts */
+#endif
+#ifndef T0_MINB
+#define T0_MINB 1   /* blocks per SM */
+#endif
+#ifndef T0_U
+#define T0_U 6
+#endif
+#define T0_PS ((40000 + T0_P - 1) / T0_P)
+#ifndef T0_THREADS
+#define T0_THREADS 1024
+#endif
+#ifndef T0_L
+#define T0_L 16
+#endif
+struct fom_t0_state_copy { fom_float v[2599983]; unsigned short c[2599983]; };
+static __device__ fom_t0_state_copy fom_t0_state_m;
+static __device__ fom_int fom_t0_state_sp[T0_P + 1][10000];
+static __device__ fom_float fom_t0_state_part[T0_P][10000];
+static __device__ unsigned int fom_t0_state_cnt[1024];
+static int fom_t0_state_ready, fom_t0_state_pairs;
+
+
+/* Bank-aware order: on sm_80 a warp's 8-byte shared loads are served per
+   16-lane half-warp, conflict-free when the 16 doubles fall in distinct
+   double-banks (index mod 16). Within each segment [b, e) of a packed
+   row, reorder the entries (value and column together) so that each
+   aligned group of 16 slots takes one entry from each of the fullest
+   banks first. Only the per-row summation order changes. Host-side, once. */
+static void fom_bank_order(fom_float *v, unsigned short *c, fom_int b, fom_int e,
+                           fom_float *tv, unsigned short *tc) {
+    fom_int m = e - b;
+    if (m <= 1) return;
+    int cnt[16], head[16], start[16];
+    for (int q = 0; q < 16; ++q) cnt[q] = 0;
+    for (fom_int k = b; k < e; ++k) ++cnt[c[k] & 15];
+    start[0] = 0;
+    for (int q = 1; q < 16; ++q) start[q] = start[q - 1] + cnt[q - 1];
+    for (int q = 0; q < 16; ++q) head[q] = start[q];
+    for (fom_int k = b; k < e; ++k) {  /* bucket by bank, stable */
+        int q = c[k] & 15;
+        tv[head[q]] = v[k];
+        tc[head[q]] = c[k];
+        ++head[q];
+    }
+    for (int q = 0; q < 16; ++q) head[q] = start[q];
+    fom_int pos = b;
+    while (pos < e) {
+        int used[16];
+        for (int q = 0; q < 16; ++q) used[q] = 0;
+        int g = 0;
+        /* Fill one group of 16: fullest unused bank first; banks repeat
+           only once every non-empty bank is in the group. */
+        while (g < 16 && pos < e) {
+            int best = -1;
+            for (int q = 0; q < 16; ++q)
+                if (cnt[q] > 0 && !used[q] && (best < 0 || cnt[q] > cnt[best])) best = q;
+            if (best < 0) {
+                for (int q = 0; q < 16; ++q) used[q] = 0;
+                continue;
+            }
+            used[best] = 1;
+            v[pos] = tv[head[best]];
+            c[pos] = tc[head[best]];
+            ++head[best];
+            --cnt[best];
+            ++pos;
+            ++g;
+        }
+    }
+}
+
+static __global__ void fom_t0_state_pack(void) {
+    fom_int k = blockIdx.x * blockDim.x + threadIdx.x;
+    if (k < FOM_OP_T0_STATE_ROWPTR[10000]) {
+        fom_int c = FOM_OP_T0_STATE_COLIDX[k];
+        fom_t0_state_m.v[k] = FOM_OP_T0_STATE_VALS[k];
+        fom_t0_state_m.c[k] = (unsigned short)(c % T0_PS);
+    }
+    if (k < 10000) {
+        fom_int m = FOM_OP_T0_STATE_ROWPTR[k], e = FOM_OP_T0_STATE_ROWPTR[k + 1];
+        fom_t0_state_sp[0][k] = m;
+        for (int p = 1; p < T0_P; ++p) {
+            while (m < e && FOM_OP_T0_STATE_COLIDX[m] < p * T0_PS) ++m;
+            fom_t0_state_sp[p][k] = m;
+        }
+        fom_t0_state_sp[T0_P][k] = e;
+    }
+}
+
+static __global__ void __launch_bounds__(T0_THREADS, T0_MINB) fom_apply_t0_state_split(const fom_float *__restrict__ x, fom_float *out) {
+    extern __shared__ fom_float xs[];
+    __shared__ unsigned int last;
+    const int npair = gridDim.x / T0_P;
+    const int h = blockIdx.x / npair;
+    const int bid = blockIdx.x - h * npair;
+    const int n = 40000 - h * T0_PS < T0_PS ? 40000 - h * T0_PS : T0_PS;
+    const int lane = threadIdx.x % T0_L;
+    const int rpb = blockDim.x / T0_L;
+    /* The first row's first chunk is loaded before the x fill so the two
+       overlap; later rows prefetch the next row's chunk before reducing. */
+    fom_int i = bid * rpb + threadIdx.x / T0_L;
+    fom_int b = 0, e = 0;
+    fom_float v[T0_U];
+    unsigned short c[T0_U];
+#define T0_LOAD(row)                                                        \
+    do {                                                                    \
+        b = (row) < 10000 ? fom_t0_state_sp[h][(row)] : 0;                  \
+        e = (row) < 10000 ? fom_t0_state_sp[h + 1][(row)] : 0;              \
+        _Pragma("unroll") for (int u = 0; u < T0_U; ++u) {                  \
+            fom_int k = b + lane + u * T0_L;                                \
+            v[u] = k < e ? fom_t0_state_m.v[k] : 0.0;                       \
+            c[u] = k < e ? fom_t0_state_m.c[k] : 0;                         \
+        }                                                                   \
+    } while (0)
+    T0_LOAD(i);
+    /* Fill: all loads of a thread are issued before any store (one L2
+       round trip, not one per element). */
+    {
+        const fom_float *xh = x + h * T0_PS;
+        fom_float r[(T0_PS + T0_THREADS - 1) / T0_THREADS];
+#pragma unroll
+        for (int q = 0; q < (T0_PS + T0_THREADS - 1) / T0_THREADS; ++q) {
+            int j = threadIdx.x + q * T0_THREADS;
+            if (j < n) r[q] = __ldg(xh + j);
+        }
+#pragma unroll
+        for (int q = 0; q < (T0_PS + T0_THREADS - 1) / T0_THREADS; ++q) {
+            int j = threadIdx.x + q * T0_THREADS;
+            if (j < n) xs[j] = r[q];
+        }
+    }
+    __syncthreads();
+    for (; i < 10000; i += npair * rpb) {
+        fom_float s = 0.0;
+#pragma unroll
+        for (int u = 0; u < T0_U; ++u) s += v[u] * xs[c[u]];
+        for (fom_int k0 = b + lane + T0_L * T0_U; k0 < e; k0 += T0_L * T0_U) {
+#pragma unroll
+            for (int u = 0; u < T0_U; ++u) {
+                fom_int k = k0 + u * T0_L;
+                v[u] = k < e ? fom_t0_state_m.v[k] : 0.0;
+                c[u] = k < e ? fom_t0_state_m.c[k] : 0;
+            }
+#pragma unroll
+            for (int u = 0; u < T0_U; ++u) s += v[u] * xs[c[u]];
+        }
+        T0_LOAD(i + npair * rpb);
+        for (int w = T0_L / 2; w > 0; w >>= 1) s += __shfl_down_sync(0xffffffff, s, w, T0_L);
+        if (lane == 0) fom_t0_state_part[h][i] = s;
+    }
+#undef T0_LOAD
+    /* Last block of the group (same rows, all parts) to get here combines
+       the group's rows, adding the parts in fixed order. */
+    __threadfence();
+    __syncthreads();
+    if (threadIdx.x == 0) last = atomicAdd(&fom_t0_state_cnt[bid], 1u);
+    __syncthreads();
+    if (last != T0_P - 1) return;
+    __threadfence();
+    for (fom_int i = bid * rpb + threadIdx.x / T0_L; i < 10000; i += npair * rpb)
+        if (lane == 0) {
+            fom_float s = __ldcg(&fom_t0_state_part[0][i]);
+            for (int p = 1; p < T0_P; ++p) s += __ldcg(&fom_t0_state_part[p][i]);
+            out[i] += s;
+        }
+    if (threadIdx.x == 0) fom_t0_state_cnt[bid] = 0;
+}
+
+static __global__ void __launch_bounds__(256) fom_apply_t0_state_csr(const fom_float *__restrict__ x, fom_float *out) {
+    fom_int i = (blockIdx.x * blockDim.x + threadIdx.x) >> 5;
+    fom_int lane = threadIdx.x & 31;
+    if (i >= 10000) return;
     fom_float s = 0.0;
     for (fom_int k = FOM_OP_T0_STATE_ROWPTR[i] + lane; k < FOM_OP_T0_STATE_ROWPTR[i + 1]; k += 32) s += FOM_OP_T0_STATE_VALS[k] * x[FOM_OP_T0_STATE_COLIDX[k]];
-    for (int w = 16; w > 0; w /= 2) s += __shfl_down_sync(0xffffffff, s, w);
+    for (int w = 16; w > 0; w >>= 1) s += __shfl_down_sync(0xffffffff, s, w);
     if (lane == 0) out[i] += s;
 }
 
 extern "C" void fom_apply_t0_state(const fom_float *x, fom_float *out) {
-    fom_apply_t0_state_kern<<<1250, FOM_OPS_THREADS>>>(x, out);
+    if (!fom_t0_state_ready) {
+        int dev = 0, nsm = 0, per = 0;
+        cudaGetDevice(&dev);
+        cudaDeviceGetAttribute(&nsm, cudaDevAttrMultiProcessorCount, dev);
+        if (cudaFuncSetAttribute(fom_apply_t0_state_split, cudaFuncAttributeMaxDynamicSharedMemorySize,
+                                 T0_PS * (int)sizeof(fom_float)) == cudaSuccess &&
+            cudaOccupancyMaxActiveBlocksPerMultiprocessor(&per, fom_apply_t0_state_split, T0_THREADS,
+                                                          T0_PS * sizeof(fom_float)) == cudaSuccess &&
+            per >= 1 && nsm * per >= T0_P) {
+            int pairs = nsm * per / T0_P;
+            fom_t0_state_pairs = pairs > 1024 ? 1024 : pairs;
+            fom_t0_state_pack<<<(2599983 + 255) / 256, 256>>>();
+            {
+                static fom_t0_state_copy h;
+                static fom_int hsp[T0_P + 1][10000];
+                static fom_float tv[40000];
+                static unsigned short tc[40000];
+                cudaMemcpyFromSymbol(&h, fom_t0_state_m, sizeof(h));
+                cudaMemcpyFromSymbol(hsp, fom_t0_state_sp, sizeof(hsp));
+                for (int r = 0; r < 10000; ++r)
+                    for (int p = 0; p < T0_P; ++p)
+                        if (hsp[p + 1][r] - hsp[p][r] <= 40000)
+                            fom_bank_order(h.v, h.c, hsp[p][r], hsp[p + 1][r], tv, tc);
+                cudaMemcpyToSymbol(fom_t0_state_m, &h, sizeof(h));
+            }
+            cudaGetLastError();
+        } else {
+            cudaGetLastError();
+            fom_t0_state_pairs = -1;
+        }
+        fom_t0_state_ready = 1;
+    }
+    if (fom_t0_state_pairs > 0)
+        fom_apply_t0_state_split<<<T0_P * fom_t0_state_pairs, T0_THREADS, T0_PS * sizeof(fom_float)>>>(x, out);
+    else
+        fom_apply_t0_state_csr<<<1250, 256>>>(x, out);
 }
 
 static __global__ void fom_apply_t0_p_p0_kern(const fom_float *x, fom_float *out) {
@@ -48,19 +263,106 @@
     fom_apply_t1_state_kern<<<157, FOM_OPS_THREADS>>>(x, out);
 }
 
-static __global__ void fom_apply_t1_t_t0_kern(const fom_float *x, fom_float *out) {
-    /* One warp per row: lanes stride the row (coalesced), then a shuffle sum. */
-    fom_int i = (blockIdx.x * blockDim.x + threadIdx.x) / 32;
-    fom_int lane = threadIdx.x % 32;
-    if (i >= 40000) return;  /* whole warps exit together */
+/* t1_t_t0: 40000 rows of 64-65 nnz, columns < 10000 (16-bit). Repacked
+   once as sliced ELL (SELL-32): rows in slices of 32, each slice padded to
+   its longest row (here 65 for all, so ~no padding) and stored slot-major,
+   entry k of row 32*s + r at off[s] + 32*k + r. A warp owns one slice
+   (lane = row), so every value/column load is a fully coalesced 256 B /
+   64 B line with no row pointers on the critical path; T1_S warps split a
+   slice's slots and their partial sums are added in fixed order through
+   shared memory (deterministic). x (80 KB) is gathered through L1.
+   The matrix loads are evict-first (ld.global.cs): this 26 MB is read
+   once per iteration and would otherwise push t0_state's packed copy
+   (26 MB, re-read every iteration) out of the 40 MB L2.
+   Falls back to plain CSR if a row is longer than the packed capacity. */
+#define T1LD(p) __ldcs(p)
+#ifndef T1_S
+#define T1_S 4
+#endif
+#define T1_NS 1250              /* 40000 / 32 slices */
+#define T1_WMAX 65
+struct fom_t1_t_t0_copy { fom_float v[T1_NS * 32 * T1_WMAX]; unsigned short c[T1_NS * 32 * T1_WMAX]; };
+static __device__ fom_t1_t_t0_copy fom_t1_t_t0_m;
+static __device__ fom_int fom_t1_t_t0_off[T1_NS], fom_t1_t_t0_w[T1_NS];
+static int fom_t1_t_t0_ready;   /* 1: SELL, -1: CSR fallback */
+
+static __global__ void fom_t1_t_t0_pack(void) {
+    fom_int i = blockIdx.x * blockDim.x + threadIdx.x;
+    if (i >= 40000) return;
+    fom_int o = fom_t1_t_t0_off[i >> 5] + (i & 31), b = FOM_OP_T1_T_T0_ROWPTR[i];
+    for (fom_int k = b; k < FOM_OP_T1_T_T0_ROWPTR[i + 1]; ++k) {
+        fom_t1_t_t0_m.v[o + 32 * (k - b)] = FOM_OP_T1_T_T0_VALS[k];
+        fom_t1_t_t0_m.c[o + 32 * (k - b)] = (unsigned short)FOM_OP_T1_T_T0_COLIDX[k];
+    }
+}
+
+static __global__ void __launch_bounds__(32 * T1_S) fom_apply_t1_t_t0_sell(const fom_float *__restrict__ x, fom_float *out) {
+    __shared__ fom_float part[T1_S][32];
+    const int r = threadIdx.x & 31, sub = threadIdx.x >> 5, sl = blockIdx.x;
+    const fom_float *__restrict__ V = fom_t1_t_t0_m.v + fom_t1_t_t0_off[sl] + r;
+    const unsigned short *__restrict__ C = fom_t1_t_t0_m.c + fom_t1_t_t0_off[sl] + r;
+    const int w = fom_t1_t_t0_w[sl];
+    fom_float s = 0.0;
+    int k = sub;
+    for (; k + 3 * T1_S < w; k += 4 * T1_S) {
+        fom_float v0 = T1LD(V + 32 * k), v1 = T1LD(V + 32 * (k + T1_S)), v2 = T1LD(V + 32 * (k + 2 * T1_S)), v3 = T1LD(V + 32 * (k + 3 * T1_S));
+        int c0 = T1LD(C + 32 * k), c1 = T1LD(C + 32 * (k + T1_S)), c2 = T1LD(C + 32 * (k + 2 * T1_S)), c3 = T1LD(C + 32 * (k + 3 * T1_S));
+        s += v0 * __ldg(x + c0);
+        s += v1 * __ldg(x + c1);
+        s += v2 * __ldg(x + c2);
+        s += v3 * __ldg(x + c3);
+    }
+    for (; k < w; k += T1_S) s += T1LD(V + 32 * k) * __ldg(x + T1LD(C + 32 * k));
+    part[sub][r] = s;
+    __syncthreads();
+    if (sub == 0) {
+        fom_float t = part[0][r];
+        for (int q = 1; q < T1_S; ++q) t += part[q][r];
+        out[sl * 32 + r] += t;
+    }
+}
+
+static __global__ void __launch_bounds__(256) fom_apply_t1_t_t0_csr(const fom_float *__restrict__ x, fom_float *out) {
+    fom_int i = (blockIdx.x * blockDim.x + threadIdx.x) >> 5;
+    fom_int lane = threadIdx.x & 31;
+    if (i >= 40000) return;
     fom_float s = 0.0;
     for (fom_int k = FOM_OP_T1_T_T0_ROWPTR[i] + lane; k < FOM_OP_T1_T_T0_ROWPTR[i + 1]; k += 32) s += FOM_OP_T1_T_T0_VALS[k] * x[FOM_OP_T1_T_T0_COLIDX[k]];
-    for (int w = 16; w > 0; w /= 2) s += __shfl_down_sync(0xffffffff, s, w);
+    for (int w = 16; w > 0; w >>= 1) s += __shfl_down_sync(0xffffffff, s, w);
     if (lane == 0) out[i] += s;
 }
 
 extern "C" void fom_apply_t1_t_t0(const fom_float *x, fom_float *out) {
-    fom_apply_t1_t_t0_kern<<<5000, FOM_OPS_THREADS>>>(x, out);
+    if (!fom_t1_t_t0_ready) {
+        static fom_int rp[40001], off[T1_NS], w[T1_NS];
+        cudaMemcpyFromSymbol(rp, FOM_OP_T1_T_T0_ROWPTR, sizeof(rp));
+        fom_int tot = 0;
+        for (int s = 0; s < T1_NS; ++s) {
+            fom_int m = 0;
+            for (int r = 0; r < 32; ++r) {
+                fom_int n = rp[32 * s + r + 1] - rp[32 * s + r];
+                if (n > m) m = n;
+            }
+            off[s] = tot;
+            w[s] = m;
+            tot += 32 * m;
+        }
+        if (tot <= T1_NS * 32 * T1_WMAX) {
+            void *base = 0;
+            cudaMemcpyToSymbol(fom_t1_t_t0_off, off, sizeof(off));
+            cudaMemcpyToSymbol(fom_t1_t_t0_w, w, sizeof(w));
+            cudaGetSymbolAddress(&base, fom_t1_t_t0_m);
+            cudaMemset(base, 0, sizeof(fom_t1_t_t0_copy));   /* padding: v = 0, c = 0 */
+            fom_t1_t_t0_pack<<<(40000 + 255) / 256, 256>>>();
+            fom_t1_t_t0_ready = 1;
+        } else {
+            fom_t1_t_t0_ready = -1;
+        }
+    }
+    if (fom_t1_t_t0_ready > 0)
+        fom_apply_t1_t_t0_sell<<<T1_NS, 32 * T1_S>>>(x, out);
+    else
+        fom_apply_t1_t_t0_csr<<<5000, 256>>>(x, out);
 }
 
 static __global__ void fom_apply_su_t_t1_kern(const fom_float *x, fom_float *out) {
```

## Agent notes

```
### Round 1

**Whole-solve mean_us:** initial 19257 µs, final 11597 µs, a **1.66x** speedup. Problem: lasso ISTA, m=10000, n=40000, 200 iterations, A100 80GB PCIe.

**Final profile** (my own `-DFOM_PROFILE` build, which syncs the device after each region, so times are inflated; profile-build BENCH 14553 µs):

| Region | Total (µs) | Per call (µs) | % |
|---|---|---|---|
| t0_matvec (A·x, 10000×40000, 2.6M nnz) | 876366 | 31.8 | 44.1 |
| t1_matvec (t1_t_t0·t0, 40000×10000, 2.6M nnz) | 855120 | 31.0 | 43.0 |
| t1_update (soft-threshold + residual + accumulator re-arm) | 257320 | 9.3 | 12.9 |

Unsynced ncu times are about 33 µs (t0), 27 µs (t1) and 3.4 µs (update). The floor for streaming 26 MB from DRAM is about 22 µs.

**Optimizations applied:**
1. **16-bit column indices** for both big products, repacked once from their own data on first call. Valid because the column counts are 40000 and 10000, both below 65536. This cuts the matrix stream from 12 to 10 bytes per nonzero. Combined with the batched loads in item 3: 19.26 → 16.58 ms.
2. **Hoisted the iteration-invariant p0 term and removed the per-iteration memsets.** The structure exploited is that the `t0_p_p0 @ p0` term doesn't change across iterations. It is computed once per solve into `c0`. The fused update kernel then resets `t0 = c0` and `acc = 0` after both are consumed, and does a warp-shuffle max reduction for the residual. This leaves 3 kernels per iteration instead of 6 operations. 16.58 → 14.88 ms.
3. **Batched (ILP) loads.** Each lane issues U independent value/column loads before its U gathers. This targets latency, since rows are nearly uniform (t0 rows have 257–260 nonzeros, t1_t_t0 rows have 64–65). Tuned to 32 lanes × 5 for t0 and 16 lanes × 5 for t1. 14.88 → 14.24 ms.
4. **Column-split shared-memory staging of x for t0_state.** Structure: x has 40000 entries (320 KB) and is too big for L1, so the random gathers were the cost. They took about 25 of the 35 µs in a microbenchmark. The columns are split into 2 halves of 160 KB, which fits one SM's shared memory. Half the SMs stage x[0:20000) and the other half x[20000:40000). Paired blocks compute partial row sums over per-row split points, and the last block of each pair adds part0 + part1 into `out` in a fixed order, so results stay deterministic. A plain CSR kernel is the fallback if the device can't give a block 160 KB. 14.24 → 12.25 ms.
5. **L2 persistence for t0_state's packed values+columns (26 MB contiguous).** Structure: the two products total 52 MB per iteration against a 40 MB L2, so they evict each other. t0's copy is launched under a persisting access-policy window (device limit 25 MB). Pinning t0 helped more than pinning t1. 12.25 → about 11.6 ms.

**Tried and reverted (correct but slower):**
- Padded ELL with 128-bit vector loads (16.5 ms).
- A nonzero bitmap of x to skip gathers: the iterates are dense, so it can't pay off (16.3 ms).
- Shared-memory staging of x for t1: not better than its L1 hit rate.
- Splitting t0 into 4 parts with full occupancy (11.9–12.3 ms).
- Software pipelining of t0 rows (11.8 ms).

**Blocked by rules:**
- Capture the 200-iteration loop in a CUDA graph to cut launch and boundary overhead | fixed `fom_apply_*` signatures launch on the legacy default stream, which can't be stream-captured, and the Makefile/build flags (e.g. `--default-stream per-thread`) are fixed | about 1–3%
- Fuse the soft-threshold/residual update into the t1_t_t0 matvec epilogue (saves one kernel and a 320 KB round trip per iteration) | activations must stay in `src/fom_solver.cu`, and `fom_ops.cu` routines may only accumulate `out += M x` (no `fom_fused_*` routine is configured) | about 5%
- Share one copy of A between t0_state and t1_t_t0, which appears to be a scaled Aᵀ, roughly halving the matrix bytes | each `fom_apply_<name>` may read only its own matrix's data | potentially 30–40%
- Run the iteration's three kernels as one persistent cooperative kernel with grid syncs | matrix products must go through separate `fom_apply_*` entry points called from the solver | unknown, probably about 5%

### Round 2

Initial vs final whole-solve mean_us: 11589.662 us -> 11444.674 us (`make bench`), a 1.013x speedup this round. Against the translated solver it is 19006.229 -> 11444.674 us, 1.66x overall. Runs of the same binary vary by about ±2% on this shared GPU, so this round's gain is near the noise level. Back-to-back A/B runs showed the new ops at 11503–11590 us vs 11795–11883 us for the previous round's code.

Final profiling breakdown (FOM_PROFILE build, each region synchronized; 200 iterations per solve):
- t1_matvec (t1_t_t0 @ t0, 40000x10000, 2.6M nnz): 882488 us total, 31.1 us/call, 44.5%
- t0_matvec (t0_state @ state, 10000x40000, 2.6M nnz): 845636 us total, 29.8 us/call, 42.7%
- t1_update (soft-threshold, residual, accumulator re-arm): 254019 us total, 8.9 us/call, 12.8%

Optimizations applied (all in `src/fom_ops.cu`, inside `fom_apply_t0_state`):
- **Bank-aware nonzero order for t0_state.** Structure: rows are sorted CSR, and the split kernel gathers x from shared memory. ncu showed 5.2-way bank conflicts and mostly MIO/short-scoreboard stalls. At pack time, once on the host, each row-part's nonzeros are reordered so every 16-lane half-warp reads 16 distinct double-banks. Only the per-row summation order changes; it stays deterministic and the ops error is 4.4e-15, far below the 1e-10 tolerance. Gain about 1.5%.
- **Prefetch of the first row chunk before the x fill.** Structure: each warp's rows are known before the shared-memory fill, and the next row's chunk is loaded before the shuffle reduction. This overlaps matrix loads with the fill and the reductions. Gain about 1%.
- **Fully unrolled x fill.** Structure: the fixed part size (20000 doubles per block). SASS showed the fill as one serialized load per iteration (about 20 L2 round trips per thread). Now all of a thread's loads are issued before any store. Gain is in the noise, but it is never worse.

Tried and reverted (verified slower or no faster):
- t1 with x staged in shared memory, persistent blocks: +16%.
- Persistent, software-pipelined t1 with L1 gathers: +7%.
- Dynamic work-stealing row scheduling for t0: +20%.
- Different t1 lanes/unroll shapes, and t0 with 3 or 4 column parts: 4–9% slower.
- Serpentine (alternating) row order for t1, and evict-first streaming access windows on part of t1: no gain.
- Computing the residual reduction only on the last iteration: about 3% slower, reproducibly.

Measured diagnosis: removing either matvec drops the solve to 4.9 or 5.6 ms. Each matvec costs about 20–24 us per iteration alone but about 54 us together, because the two 26 MB matrices thrash the 40 MB L2 (25 MB of it is persisting for t0). The L2 persistence window on t0 is worth about 8%.

Blocked by rules:
- Capture the 200 iterations (or batches of them) in a CUDA graph to remove per-launch overhead and inter-kernel gaps | the fixed `fom_apply_*` signatures take no stream and launch on the legacy default stream, which cannot be captured; the solver also cannot reach non-static helpers in `fom_ops.cu` | about 2–5%
- Fuse the soft-threshold/residual update into the epilogue of the t1_t_t0 matvec kernel, and the t0 re-arm into the t0_state kernel | `fom_apply_<name>` must compute only out += M x, and the solver may not contain matvec loops or read matrix data | about 5–8% (saves one launch and an extra pass over 1.3 MB per iteration)
- One persistent cooperative kernel per iteration (both matvecs plus the update, with grid barriers) | matvec loops may not live in `src/fom_solver.cu`, and each `fom_apply_*` may read only its own matrix's data | unknown, possibly 10%+
- Run t1_t_t0 as a transpose product of t0_state's data (t1_t_t0 has the structure of a scaled A^T), so only one 26 MB matrix stream is needed and both fit in L2 | each `fom_apply_<name>` and its helpers may read only their own matrix's data | about 30–40% (the measured L2 thrash penalty)
- Store matrix values in fp32 to halve the dominant 8-byte value stream | the direct ops check requires 1e-10 relative accuracy against the matrix | about 30%

### Round 3

**Whole-solve `mean_us`:** 11356.6 at the start of this round, 9180–9307 at the end (two final `make bench` runs: 9307.3 and 9180.5). That is 1.23x this round and 2.07x over the translated solver (19006.2).

**Final profile breakdown** (FOM_PROFILE build, which adds a device sync per region, so the absolute times are inflated):

| Region | Time per call | Share |
|---|---|---|
| t1_matvec | 27.1 us | 43.4% |
| t0_matvec | 26.1 us | 41.8% |
| t1_update | 9.3 us | 14.9% |

**Optimizations applied:**
- **Sliced-ELL (SELL-32) layout for `t1_t_t0`.** Structure: 40000 rows with a near-constant 64–65 nnz and 16-bit columns, so padding is only 17 entries in 2.6M. Rows go in slices of 32, stored slot-major, so a warp owns a slice (lane = row). Every value and column load is a fully coalesced 256 B / 64 B line, with no row pointers or shuffle reduction on the critical path. Four warps split a slice's slots and their partial sums are added in a fixed order through shared memory, so results stay deterministic.
- **Evict-first loads for `t1_t_t0`'s matrix stream (`__ldcs`).** Structure: the two products stream 52 MB per iteration through a 40 MB L2. t1's 26 MB is read once per iteration, while t0_state's packed 26 MB copy is re-read every iteration. Marking t1's loads evict-first keeps t0's copy L2-resident. This gave a −5.6% whole-solve gain.
- **Removed the L2 persisting window.** Structure: as above. Once t1 is evict-first, the full 40 MB L2 keeps t0's copy resident better than the 25 MiB set-aside did (even a pure stream over the "persisted" region ran at DRAM speed). This gave −2.3%.
- **Four column parts for `t0_state` (80 KB of x per block instead of 160 KB), 16 lanes per row segment.** Structure: rows have ~260 nnz in random columns over 40000. Total shared-memory fill traffic is (#blocks × part size), so going from 2 to 4 parts halves it, from 17 MB to 8.6 MB per call. The fill alone was measured at ~7 us. Each part's ~65-nnz segment fits one 16-lane × 6-slot chunk.
- **State updated in place for all but the last iteration.** Structure: `t1_state` and `su_t_t1` are identities and the residual is only needed at `max_iter`. So `t1_t_t0 @ t0` is accumulated directly into the state buffer and soft-thresholded in place. The last iteration keeps the separate accumulator for the residual. This removes the accumulator round trip and the per-iteration max reduction (about 0.3%).

**Tried and rejected (no gain beyond noise):**
- Deeper unrolling of the t1 loads.
- A software-pipelined row loop in t0.
- A `cp.async` fill for t0.
- Higher-occupancy t0 configurations (P = 6/8/12 with 512–768-thread blocks).
- A SELL layout per column part with a per-slice atomic combine.
- An evict_last cache hint on t0's loads.
- A side-stream L2 prefetch of t1's matrix, which made things worse by evicting t0's copy.
- A sparsity skip on x: the iterate stays fully dense for all 200 iterations, so there was nothing to skip.

**Blocked by rules:**
- Store A once and use it for both products: `t1_t_t0` equals −0.0147 · `t0_state`ᵀ exactly (same sparsity, one constant ratio), and the per-iteration traffic would drop from 52 MB to 26 MB, which fits in L2 | kernel rule: each `fom_apply_<name>` and its helpers read only their own matrix's data | ~35–45% of solve time (the products are bandwidth-bound and roughly half their bytes would go away)
- Capture the 200-iteration loop in a CUDA graph to remove launch gaps between the 3 kernels per iteration | fixed `fom_apply_*` signatures and build flags: kernels launch on the legacy default stream, which cannot be stream-captured | ~2–3% (about 1 us/iteration)
- Fuse the soft-threshold/state step into the matvec kernels (e.g. soft-threshold during t0's x fill, or in t1's epilogue) | kernel rule: `fom_apply_<name>` must compute exactly `out += M @ x`, and the solver must not contain matvec loops | ~4–6% (the t1_update kernel and one kernel boundary per iteration)
```
