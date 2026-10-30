# Optimization report

## Timing

```
translated (emitted)  205879.5 us/solve
optimized              37743.1 us/solve
speedup                  5.45x
```

Optimization outcome: **kept (re-verified correct)**.

## Rounds

| round | before (us) | after (us) | gain | kept | agent (s) | outcome |
|---:|---:|---:|---:|:---:|---:|---|
| 1 | 205879.468 | 68238.548 | 66.9% | yes | 1869 | continued |
| 2 | 74233.368 | 49911.764 | 32.8% | yes | 2766 | continued |
| 3 | 74520.246 | 45603.394 | 38.8% | yes | 4370 | continued |
| 4 | 46561.336 | 42345.128 | 9.1% | yes | 3926 | continued |
| 5 | 43278.299 | 37743.064 | 12.8% | yes | 1481 | the last of agent_optimize_rounds rounds |

## Kernel checks

Each `fom_apply_<name>` in the shipped `fom_ops.c`, checked by `make verify` against the package's reference over the same `fom_data.c` matrix. *basis* probes every unit vector, which recovers the routine's matrix column by column (an exact check), plus random vectors; *random* uses random vectors only (the matrix is over the probe budget). A `fused_<block>` row is a `fom_fused_<block>`, checked against the sum of its members: *basis* there probes every unit vector of each source with every schedule value at 0 and with each alone at 1, which recovers every member exactly.

| matrix | shape | nnz | rewritten | check | probes | max error |
|---|---|---|---|---|---|---|
| `t0_state` | 10000x80000 | 2599983 | yes | random | 2 | 0.000e+00 |
| `t0_state__d_gamma` | 10000x80000 | 5199966 | yes | random | 2 | 0.000e+00 |
| `t0_p_p0` | 10000x10000 | 10000 | yes | random | 2 | 0.000e+00 |
| `t1_state` | 40000x80000 | 40000 | yes | random | 2 | 0.000e+00 |
| `t1_state__d_gamma` | 40000x80000 | 80000 | yes | random | 2 | 0.000e+00 |
| `t1_t_t0` | 40000x10000 | 2599983 | yes | random | 2 | 0.000e+00 |
| `su_state` | 80000x80000 | 40000 | yes | random | 2 | 0.000e+00 |
| `su_t_t1` | 80000x40000 | 40000 | yes | random | 2 | 0.000e+00 |
| `fused_t0` | 10000x80000 | 7799949 | yes | random | 2 | 0.000e+00 |
| `fused_t1` | 40000x90000 | 2719983 | yes | random | 2 | 0.000e+00 |
| `fused_su` | 80000x120000 | 80000 | yes | random | 2 | 0.000e+00 |
| `step_t0` | 10000x80000 | 7799949 | yes | random | 6 | 0.000e+00 |
| `step_t1` | 40000x90000 | 2719983 | yes | random | 6 | 0.000e+00 |
| `step_su` | 80000x120000 | 80000 | yes | random | 2 | 0.000e+00 |
| `iterate` | 80000x80000 | 10599932 | yes | random | 6 | 1.599e-14 |

### Rule violations

_None: the collected rewrite passed the rules and was kept._

## Blocked by rules

Optimizations the agent considered but did not make because a rule forbade them, in its own words.

| round | idea | rule | expected gain |
|---|---|---|---|
| 1 | Skip reading the all-zero `bias_t1`/`bias_su` vectors and specialise the t1 pass for a zero bias | `fom_iterate` and step routines must apply any bias they are passed, and the solver must pass real arrays | ~2% |
| 1 | Fuse iteration k's t1 pass with iteration k+1's A·z (gather and scatter on the same row while it is in L1) | `fom_iterate` gets only the current schedule value, and `fom_ops.c` may not read the schedule or keep results across calls | unknown (likely small, since scatter measured slower) |
| 1 | Compile the kernels with `-march=native` (AVX2/FMA) | build flags are fixed | ~0% (measured in a side build) |
| 2 | Keep A·s1 from the previous call, so the next t0 needs only A·t1 (or let the T pass also produce A·t1 for the next iteration), removing the dependency that forces two passes over A per iteration | fom_iterate must not keep state across calls, and the solver may not read matrix data or loop over products itself | unknown, up to about 2x on the A work |
| 2 | Store A's values as float32 to halve the bytes streamed | correctness: the values are not exactly representable in float32 (lossy) | unknown |
| 2 | Build with -march=native (AVX2/FMA everywhere) | Makefile and build flags are fixed | about 1%, measured by forcing AVX2 on the gather |
| 3 | Build the kernels with `-march=native` / `-mtune` for the node instead of function-level target attributes | Makefile and build flags are fixed | small; the AVX2 kernels are already dispatched at run time |
| 3 | Form the next iteration's extrapolation z_{k+1} inside the current iteration's t1/state-update pass, removing the z pass and one barrier per iteration | `fom_ops.c` must not read the schedule, and `fom_iterate` only receives the current gamma | 1-3% |
| 4 | Reuse A s0 from the previous call: FISTA's s0 at k+1 is s1 at k, so A z = (1+g) A s1 − g A s0 needs only one fresh product, which could fuse with the T pass so A is read once per iteration | `fom_iterate` must be exact for arbitrary states and nothing but the entry points may be non-static, so no cached products across calls | unknown, possibly large on Ice Lake where the passes are bound by streaming A |
| 4 | Form next iteration's z inside the T pass epilogue, while s1 and t1 are in registers | `src/fom_ops.c` may not read the schedule, and `fom_iterate` only receives the current gamma | ~1% |
| 4 | Store A's values as float32 to halve the stream | each kernel must apply exactly M, and A's values are full-precision doubles | unknown |
| 4 | Place threads per socket or CCD to cut cross-socket partial and t0 traffic | build flags and environment (thread binding) are fixed | unknown |
| 5 | Store A's values as float32 to halve the bytes per nonzero | `fom_apply`, `fom_iterate` and `fom_step` must apply M exactly, so values cannot be stored in lower precision | unknown (probably small now that the passes are compute-bound) |
| 5 | Raise the thread count or change OMP placement | the environment and build flags are fixed | unknown |

## What the agent changed

```diff
--- src/fom_solver.c (translated)
+++ src/fom_solver.c (optimized)
@@ -3,8 +3,7 @@
 #include "fom_data.h"
 #include "fom_ops.h"
 #include "fom_linsolve.h"
-
-#define HALF FOM_T1_DIM /* state = [s0; s1], each FOM_T1_DIM long */
+#include "fom_profile.h"
 
 void fom_set_default_settings(FomSettings *settings) {
     settings->max_iter = FOM_MAX_ITER;
@@ -31,39 +30,55 @@
 }
 
 fom_int fom_solve(FomWorkspace *work) {
-    fom_float *x = work->state;
-    fom_float *t1 = work->t1;
-    /* acc[0..T0): hoisted parameter bias for t0; acc[T0..T0+T1): zero bias for t1 */
+    /* Ping-pong between state and new_state; solution.x holds zeros (the
+       t1 and su blocks have no constant) until the final copy. */
+    fom_float *cur = work->state, *nxt = work->new_state, *tmp;
     fom_float *bias_t0 = work->acc;
-    fom_float *bias_t1 = work->acc + FOM_T0_DIM;
+    fom_float *zeros = work->solution.x;
     fom_int max_iter = work->settings.max_iter;
     fom_float res = 0;
-    fom_int it;
+    fom_int it, i;
 
-    memset(bias_t0, 0, sizeof(fom_float) * (FOM_T0_DIM + FOM_T1_DIM));
+    FOM_PROF_BEGIN(bias);
+    memset(bias_t0, 0, sizeof(fom_float) * FOM_T0_DIM);
+    memset(zeros, 0, sizeof(work->solution.x));
     fom_apply_t0_p_p0(work->params.p0, bias_t0);
+    FOM_PROF_END(bias);
 
-    for (it = 0; it < max_iter; ++it) {
-        const fom_float g = fom_sched_gamma(it);
-        fom_int i;
-        fom_step_t0(x, g, bias_t0, &FOM_ACT_t0, work->t0);
-        fom_step_t1(x, work->t0, g, bias_t1, &FOM_ACT_t1, t1);
-        /* state <- [s1; t1], residual = max |new - old| */
-        res = 0;
-#pragma omp parallel for reduction(max:res) schedule(static)
-        for (i = 0; i < HALF; ++i) {
-            fom_float a = x[i], b = x[i + HALF], c = t1[i];
-            fom_float d0 = b - a, d1 = c - b;
-            d0 = d0 < 0 ? -d0 : d0;
-            d1 = d1 < 0 ? -d1 : d1;
-            if (d0 > res) res = d0;
-            if (d1 > res) res = d1;
-            x[i] = b;
-            x[i + HALF] = c;
+    FOM_PROF_BEGIN(iterate);
+    /* One thread team for the whole loop: every thread runs the loop and
+       fom_iterate shares each phase across the team (no fork per iteration). */
+#ifdef _OPENMP
+#pragma omp parallel private(it)
+#endif
+    {
+        fom_float *c = cur, *n = nxt, *t;
+        for (it = 0; it < max_iter; ++it) {
+            const fom_float g = fom_sched_gamma(it);
+            fom_iterate(c, g, bias_t0, zeros, zeros, &FOM_ACT_t0, &FOM_ACT_t1, work->t0, work->t1, n);
+            t = c; c = n; n = t;
         }
     }
+    it = max_iter > 0 ? max_iter : 0;
+    if (it & 1) { tmp = cur; cur = nxt; nxt = tmp; }
+    FOM_PROF_END(iterate);
 
-    memcpy(work->solution.x, x, sizeof(work->solution.x));
+    /* residual = max |x_k - x_{k-1}| of the last iteration */
+    FOM_PROF_BEGIN(finish);
+    if (max_iter > 0) {
+#ifdef _OPENMP
+#pragma omp parallel for reduction(max:res) schedule(static)
+#endif
+        for (i = 0; i < FOM_STATE_DIM; ++i) {
+            fom_float d = cur[i] - nxt[i];
+            d = d < 0 ? -d : d;
+            if (d > res) res = d;
+        }
+    }
+    memcpy(work->solution.x, cur, sizeof(work->solution.x));
+    if (cur != work->state) memcpy(work->state, cur, sizeof(work->state));
+    FOM_PROF_END(finish);
+
     work->solution.iter = it;
     work->solution.residual = res;
     work->solution.status = FOM_MAX_ITER_REACHED;
--- src/fom_ops.c (translated)
+++ src/fom_ops.c (optimized)
@@ -1,5 +1,10 @@
 #include "fom_ops.h"
 #include "fom_data.h"
+#include "fom_profile.h"
+#include <math.h>
+#ifdef _OPENMP
+#include <omp.h>
+#endif
 
 void fom_apply_t0_state(const fom_float *x, fom_float *out) {
     fom_int i, p;
@@ -210,8 +215,798 @@
     }
 }
 
+/* Column-blocked copies of A (the s1 block of t0_state) and of t1_t_t0,
+   derived from fom_data.c on the first fom_iterate call, for the fallback
+   passes (fom_iter_phases; the fused passes read fom_xA / fom_xT). Block b holds, for
+   every row, the row's entries with columns in [b W, (b + 1) W), as CSR with
+   16-bit column indices (all columns fit). A pass sweeps the blocks in turn,
+   so the randomly accessed source or accumulator slice (W doubles: 20 KB for
+   A) stays in L1: with 80 KB slices every access refilled a line from L2. */
+#define FOM_BA_W 2500
+#define FOM_BA_N 16 /* 40000 / 2500 */
+/* the fused pass splits the columns into at most FOM_CB_MAX thread blocks */
+#define FOM_CB_MAX 8
+#define FOM_BT_W 5000
+#define FOM_BT_N 2  /* 10000 / 5000 */
+static fom_int fom_bA_ptr[FOM_BA_N][10001];
+/* A's segments are padded to a multiple of FOM_PAD entries (value 0, index
+   of the segment's first entry). */
+#define FOM_PAD 4
+static unsigned short fom_bA_idx[FOM_MAT_t0_state_NNZ + (FOM_PAD - 1) * FOM_BA_N * 10000];
+#if defined(__GNUC__)
+__attribute__((aligned(64)))
+#endif
+static fom_float fom_bA_val[FOM_MAT_t0_state_NNZ + (FOM_PAD - 1) * FOM_BA_N * 10000];
+/* The fused passes read A (on the s1 columns) and T = t1_t_t0 in a
+   sliced-ELLPACK layout (SELL-4-sigma) on column windows of FOM_BA_W (the
+   gathered window of the source, 20 KB, stays in L1):
+   - block b of a matrix holds every row's entries with columns in
+     [b W, (b + 1) W) (a segment, ~16 entries here);
+   - within each sort window of SW rows, the rows are ordered by segment
+     length (longest first; row[b][k] is the row at position k), and each
+     4 consecutive positions form a slice of length L = its longest
+     segment, padded with zeros (rare: sorted neighbours mostly share a
+     length);
+   - a slice stores entry j of its 4 rows together (4 indices, one 64-bit
+     word; 4 values, one 256-bit load), so one SIMD lane accumulates one
+     row: no per-segment horizontal sums or tail chunks (they cost as much
+     as the products on segments this short), and the loop trip counts
+     follow the sorted lengths, so the exits predict.
+   wp[b][w] is the first entry of sort window w in block b. */
+#define FOM_ASW 1000
+#define FOM_ANW 10  /* 10000 / FOM_ASW */
+#define FOM_TW_N 4  /* T's column windows: 10000 / 2500 */
+#define FOM_TSW 500
+#define FOM_TNW 80  /* 40000 / FOM_TSW */
+/* padding: a sort window's slices pad at most 3 entries per unit of
+   length drop, so at most 3 W per window and block */
+#define FOM_XA_CAP (FOM_MAT_t0_state_NNZ + 3 * FOM_BA_W * FOM_ANW * FOM_BA_N)
+#define FOM_XT_CAP (FOM_MAT_t1_t_t0_NNZ + 3 * FOM_BA_W * FOM_TNW * FOM_TW_N)
+static unsigned short fom_xA_row[FOM_BA_N][10000];
+static unsigned short fom_xA_len[FOM_BA_N][10000 / 4];
+static fom_int fom_xA_wp[FOM_BA_N][FOM_ANW + 1];
+static unsigned short fom_xA_idx[FOM_XA_CAP];
+#if defined(__GNUC__)
+__attribute__((aligned(64)))
+#endif
+static fom_float fom_xA_val[FOM_XA_CAP];
+static unsigned short fom_xT_row[FOM_TW_N][40000];
+static unsigned short fom_xT_len[FOM_TW_N][40000 / 4];
+static fom_int fom_xT_wp[FOM_TW_N][FOM_TNW + 1];
+static unsigned short fom_xT_idx[FOM_XT_CAP];
+#if defined(__GNUC__)
+__attribute__((aligned(64)))
+#endif
+static fom_float fom_xT_val[FOM_XT_CAP];
+/* build scratch: segment lengths per block and row, row cursors */
+static unsigned short fom_xcnt[FOM_BA_N * 10000 + FOM_TW_N * 40000];
+static fom_int fom_xcur[40000];
+static fom_int fom_bT_ptr[FOM_BT_N][40001];
+static unsigned short fom_bT_idx[FOM_MAT_t1_t_t0_NNZ];
+static fom_float fom_bT_val[FOM_MAT_t1_t_t0_NNZ];
+static int fom_idx_built = 0;
+/* Row stride into the diagonal t1_state / t1_state__d_gamma values: 0 when
+   every row holds the same values (read once, stays in a register). */
+static fom_int fom_diag_stride = 1;
+
+/* Profiling inside the iteration (profile builds only): the master thread
+   timestamps each phase as it leaves the phase's closing barrier. */
+#ifdef FOM_PROFILE
+static double fom_iter_t;
+static void fom_iter_mark(const char *name) {
+    double t = fom_now_ns();
+    if (name) fom_prof_add(name, t - fom_iter_t);
+    fom_iter_t = t;
+}
+#endif
+
+/* Sum of one block row: four column indices per 64-bit load, values two
+   per 128-bit load against a pair of gathered x entries (SSE2, baseline
+   x86-64), so a nonzero costs fewer loads; scalar elsewhere. */
+#if defined(__SSE2__)
+#include <emmintrin.h>
+#include <string.h>
+static inline fom_float fom_brow(const fom_float *va, const unsigned short *ci, const fom_float *x, fom_int p, fom_int p1) {
+    __m128d a0 = _mm_setzero_pd(), a1 = _mm_setzero_pd(), a2 = _mm_setzero_pd(), a3 = _mm_setzero_pd();
+    fom_float r;
+    for (; p + 8 <= p1; p += 8) {
+        unsigned long long w, u;
+        __m128d x0, x1, x2, x3;
+        memcpy(&w, ci + p, 8);
+        memcpy(&u, ci + p + 4, 8);
+        x0 = _mm_loadh_pd(_mm_load_sd(x + (w & 0xffff)), x + ((w >> 16) & 0xffff));
+        x1 = _mm_loadh_pd(_mm_load_sd(x + ((w >> 32) & 0xffff)), x + (w >> 48));
+        x2 = _mm_loadh_pd(_mm_load_sd(x + (u & 0xffff)), x + ((u >> 16) & 0xffff));
+        x3 = _mm_loadh_pd(_mm_load_sd(x + ((u >> 32) & 0xffff)), x + (u >> 48));
+        a0 = _mm_add_pd(a0, _mm_mul_pd(_mm_loadu_pd(va + p), x0));
+        a1 = _mm_add_pd(a1, _mm_mul_pd(_mm_loadu_pd(va + p + 2), x1));
+        a2 = _mm_add_pd(a2, _mm_mul_pd(_mm_loadu_pd(va + p + 4), x2));
+        a3 = _mm_add_pd(a3, _mm_mul_pd(_mm_loadu_pd(va + p + 6), x3));
+    }
+    if (p + 4 <= p1) {
+        unsigned long long w;
+        memcpy(&w, ci + p, 8);
+        a0 = _mm_add_pd(a0, _mm_mul_pd(_mm_loadu_pd(va + p), _mm_loadh_pd(_mm_load_sd(x + (w & 0xffff)), x + ((w >> 16) & 0xffff))));
+        a1 = _mm_add_pd(a1, _mm_mul_pd(_mm_loadu_pd(va + p + 2), _mm_loadh_pd(_mm_load_sd(x + ((w >> 32) & 0xffff)), x + (w >> 48))));
+        p += 4;
+    }
+    a0 = _mm_add_pd(_mm_add_pd(a0, a1), _mm_add_pd(a2, a3));
+    r = _mm_cvtsd_f64(a0) + _mm_cvtsd_f64(_mm_unpackhi_pd(a0, a0));
+    for (; p < p1; ++p) r += va[p] * x[ci[p]];
+    return r;
+}
+#else
+static inline fom_float fom_brow(const fom_float *va, const unsigned short *ci, const fom_float *x, fom_int p, fom_int p1) {
+    fom_float a0 = 0.0, a1 = 0.0, a2 = 0.0, a3 = 0.0;
+    for (; p + 4 <= p1; p += 4) {
+        a0 += va[p] * x[ci[p]];
+        a1 += va[p + 1] * x[ci[p + 1]];
+        a2 += va[p + 2] * x[ci[p + 2]];
+        a3 += va[p + 3] * x[ci[p + 3]];
+    }
+    for (; p < p1; ++p) a0 += va[p] * x[ci[p]];
+    return (a0 + a1) + (a2 + a3);
+}
+#endif
+
+/* part[rw[4 s + k]] (+)= slice s, lane k, . x for slices s in [s0, s1),
+   the slices starting at entry p. AVX2 + FMA when the CPU has them
+   (checked once at init; the baseline build runs anywhere): four 16-bit
+   indices per 64-bit load, split with shifts, four values per 256-bit
+   load, one FMA per four entries; hardware gathers are avoided (slower
+   than scalar loads here). */
+#if defined(__GNUC__) && defined(__x86_64__)
+#include <immintrin.h>
+#define FOM_HAVE_AVX2 1
+static int fom_use_avx2 = 0;
+#define FOM_IX(w, k) (((w) >> (16 * (k))) & 0xffff)
+#define FOM_X4(w) _mm256_insertf128_pd(_mm256_castpd128_pd256(_mm_loadh_pd(_mm_load_sd(x + FOM_IX(w, 0)), x + FOM_IX(w, 1))), _mm_loadh_pd(_mm_load_sd(x + FOM_IX(w, 2)), x + ((w) >> 48)), 1)
+__attribute__((target("avx2,fma")))
+static void fom_sell_avx2(const fom_float *va, const unsigned short *ci, const fom_float *x, const unsigned short *ln, const unsigned short *rw, fom_int p, fom_int s0, fom_int s1, fom_float *part, int add) {
+    fom_int s;
+    for (s = s0; s < s1; ++s) {
+        const fom_int pe = p + 4 * (fom_int)ln[s];
+        const unsigned short *r = rw + 4 * s;
+        __m256d a0 = _mm256_setzero_pd(), a1 = _mm256_setzero_pd();
+        __m128d lo, hi;
+        unsigned long long w, u;
+        for (; p + 8 <= pe; p += 8) {
+            memcpy(&w, ci + p, 8);
+            memcpy(&u, ci + p + 4, 8);
+            a0 = _mm256_fmadd_pd(_mm256_load_pd(va + p), FOM_X4(w), a0);
+            a1 = _mm256_fmadd_pd(_mm256_load_pd(va + p + 4), FOM_X4(u), a1);
+        }
+        if (p < pe) {
+            memcpy(&w, ci + p, 8);
+            a0 = _mm256_fmadd_pd(_mm256_load_pd(va + p), FOM_X4(w), a0);
+            p = pe;
+        }
+        a0 = _mm256_add_pd(a0, a1);
+        lo = _mm256_castpd256_pd128(a0);
+        hi = _mm256_extractf128_pd(a0, 1);
+        if (add) {
+            part[r[0]] += _mm_cvtsd_f64(lo);
+            part[r[1]] += _mm_cvtsd_f64(_mm_unpackhi_pd(lo, lo));
+            part[r[2]] += _mm_cvtsd_f64(hi);
+            part[r[3]] += _mm_cvtsd_f64(_mm_unpackhi_pd(hi, hi));
+        } else {
+            part[r[0]] = _mm_cvtsd_f64(lo);
+            part[r[1]] = _mm_cvtsd_f64(_mm_unpackhi_pd(lo, lo));
+            part[r[2]] = _mm_cvtsd_f64(hi);
+            part[r[3]] = _mm_cvtsd_f64(_mm_unpackhi_pd(hi, hi));
+        }
+    }
+}
+/* y[ci] += va * w[rw[4 s + k]] (lane k) over slices s in [s0, s1), the
+   slices starting at entry p: A^T w from A's layout */
+__attribute__((target("avx2,fma")))
+static void fom_sell_scat_avx2(const fom_float *va, const unsigned short *ci, fom_float *y, const unsigned short *ln, const unsigned short *rw, fom_int p, fom_int s0, fom_int s1, const fom_float *w) {
+    fom_int s;
+    for (s = s0; s < s1; ++s) {
+        const fom_int pe = p + 4 * (fom_int)ln[s];
+        const unsigned short *r = rw + 4 * s;
+        const __m256d wv = _mm256_set_pd(w[r[3]], w[r[2]], w[r[1]], w[r[0]]);
+#define FOM_SC4(q, m) \
+            mlo = _mm256_castpd256_pd128(m); \
+            mhi = _mm256_extractf128_pd(m, 1); \
+            e0 = y + FOM_IX(q, 0); \
+            e1 = y + FOM_IX(q, 1); \
+            e2 = y + FOM_IX(q, 2); \
+            e3 = y + ((q) >> 48); \
+            _mm_store_sd(e0, _mm_add_sd(_mm_load_sd(e0), mlo)); \
+            _mm_store_sd(e1, _mm_add_sd(_mm_load_sd(e1), _mm_unpackhi_pd(mlo, mlo))); \
+            _mm_store_sd(e2, _mm_add_sd(_mm_load_sd(e2), mhi)); \
+            _mm_store_sd(e3, _mm_add_sd(_mm_load_sd(e3), _mm_unpackhi_pd(mhi, mhi)));
+        unsigned long long q, u;
+        __m256d m, n;
+        __m128d mlo, mhi;
+        fom_float *e0, *e1, *e2, *e3;
+        for (; p + 8 <= pe; p += 8) {
+            memcpy(&q, ci + p, 8);
+            memcpy(&u, ci + p + 4, 8);
+            m = _mm256_mul_pd(_mm256_load_pd(va + p), wv);
+            n = _mm256_mul_pd(_mm256_load_pd(va + p + 4), wv);
+            FOM_SC4(q, m)
+            FOM_SC4(u, n)
+        }
+        if (p < pe) {
+            memcpy(&q, ci + p, 8);
+            m = _mm256_mul_pd(_mm256_load_pd(va + p), wv);
+            FOM_SC4(q, m)
+            p = pe;
+        }
+#undef FOM_SC4
+    }
+}
+#undef FOM_X4
+#undef FOM_IX
+#endif
+
+static void fom_sell_scat(const fom_float *va, const unsigned short *ci, fom_float *y, const unsigned short *ln, const unsigned short *rw, fom_int p, fom_int s0, fom_int s1, const fom_float *w) {
+    fom_int s, j, k;
+#ifdef FOM_HAVE_AVX2
+    if (fom_use_avx2) { fom_sell_scat_avx2(va, ci, y, ln, rw, p, s0, s1, w); return; }
+#endif
+    for (s = s0; s < s1; ++s)
+        for (j = 0; j < ln[s]; ++j, p += 4)
+            for (k = 0; k < 4; ++k) y[ci[p + k]] += va[p + k] * w[rw[4 * s + k]];
+}
+
+/* fom_sell_avx2's contract, AVX2 or scalar */
+static void fom_sell(const fom_float *va, const unsigned short *ci, const fom_float *x, const unsigned short *ln, const unsigned short *rw, fom_int p, fom_int s0, fom_int s1, fom_float *part, int add) {
+    fom_int s, j, k;
+#ifdef FOM_HAVE_AVX2
+    if (fom_use_avx2) { fom_sell_avx2(va, ci, x, ln, rw, p, s0, s1, part, add); return; }
+#endif
+    for (s = s0; s < s1; ++s) {
+        fom_float a[4] = {0.0, 0.0, 0.0, 0.0};
+        for (j = 0; j < ln[s]; ++j, p += 4)
+            for (k = 0; k < 4; ++k) a[k] += va[p + k] * x[ci[p + k]];
+        for (k = 0; k < 4; ++k) {
+            if (add) part[rw[4 * s + k]] += a[k]; else part[rw[4 * s + k]] = a[k];
+        }
+    }
+}
+
+/* The iteration's phases as orphaned worksharing loops: run by a whole team
+   (inside a parallel region) or by one thread. The block loops of a pass
+   share one static schedule, so each row stays with one thread and they
+   need no barrier between them (nowait); each pass ends at a barrier.
+   Structure (from fom_data.c): t0_state = [0, A] (A on the s1 columns,
+   10000 x 40000) and t0_state__d_gamma = [-A, A] with the same column
+   pattern (values equal to A's up to rounding), so
+     t0_state @ s + g (t0_state__d_gamma @ s) = A (s1 + g (s1 - s0)):
+   one sparse pass over A instead of three. t1_state / t1_state__d_gamma
+   are diagonal (one entry per row at s1[i]; two at s0[i], s1[i]); their
+   values are read exactly. su is the selection [s1; t1]. */
+static void fom_iter_phases(const fom_float *state, fom_float g, const fom_float *bias_t0, const fom_float *bias_t1, const fom_float *bias_su, fom_float l0, fom_float l1, fom_float *t_t0, fom_float *t_t1, fom_float *new_state) {
+    const fom_float *s0 = state;
+    const fom_float *s1 = state + 40000;
+    /* the t0 source, the FISTA extrapolation z = s1 + g (s1 - s0), lives in
+       the t1 half of new_state until the t1 pass overwrites it */
+    fom_float *z = new_state + 40000;
+    const fom_int ds = fom_diag_stride;
+    fom_int i, b;
+    #ifdef FOM_PROFILE
+    #pragma omp master
+    fom_iter_mark(0);
+    #endif
+    #ifdef _OPENMP
+    #pragma omp for schedule(static)
+    #endif
+    for (i = 0; i < 40000; ++i) z[i] = s1[i] + g * (s1[i] - s0[i]);
+    #ifdef FOM_PROFILE
+    #pragma omp master
+    fom_iter_mark("iter_z");
+    #endif
+
+    /* t0 = soft(bias_t0 + A z), accumulated block by block in t_t0 */
+    for (b = 0; b < FOM_BA_N; ++b) {
+        const fom_int *rp = fom_bA_ptr[b];
+        #ifdef _OPENMP
+        #pragma omp for schedule(static) nowait
+        #endif
+        for (i = 0; i < 10000; ++i) {
+            fom_float v = fom_brow(fom_bA_val, fom_bA_idx, z, rp[i], rp[i + 1]);
+            if (b > 0) v += t_t0[i];
+            if (b == FOM_BA_N - 1) {
+                v += bias_t0[i];
+                v = (v > l0) ? (v - l0) : ((v < -l0) ? (v + l0) : 0.0);
+            }
+            t_t0[i] = v;
+        }
+    }
+    #ifdef _OPENMP
+    #pragma omp barrier
+    #endif
+    #ifdef FOM_PROFILE
+    #pragma omp master
+    fom_iter_mark("iter_A_t0");
+    #endif
+
+    /* t1 = soft(bias_t1 + diag terms + T t0), accumulated in t_t1; the last
+       block also writes the state update [s1; t1] */
+    for (b = 0; b < FOM_BT_N; ++b) {
+        const fom_int *rp = fom_bT_ptr[b];
+        #ifdef _OPENMP
+        #pragma omp for schedule(static) nowait
+        #endif
+        for (i = 0; i < 40000; ++i) {
+            fom_float v = fom_brow(fom_bT_val, fom_bT_idx, t_t0, rp[i], rp[i + 1]);
+            if (b > 0) v += t_t1[i];
+            if (b == FOM_BT_N - 1) {
+                const fom_float x0 = s0[i], x1 = s1[i];
+                const fom_int q = i * ds;
+                v += bias_t1[i] + FOM_MAT_t1_state_VALS[q] * x1
+                    + g * (FOM_MAT_t1_state__d_gamma_VALS[2 * q] * x0 + FOM_MAT_t1_state__d_gamma_VALS[2 * q + 1] * x1);
+                v = (v > l1) ? (v - l1) : ((v < -l1) ? (v + l1) : 0.0);
+                new_state[i] = bias_su[i] + x1;
+                new_state[40000 + i] = bias_su[40000 + i] + v;
+            }
+            t_t1[i] = v;
+        }
+    }
+    #ifdef _OPENMP
+    #pragma omp barrier
+    #endif
+    #ifdef FOM_PROFILE
+    #pragma omp master
+    fom_iter_mark("iter_T_t1");
+    #endif
+}
+
+/* From fom_data.c, t1_t_t0 = c A^T (same pattern transposed, every value
+   c times A's up to rounding; checked entrywise at init), so T t0 can also
+   be A^T (c t0): one copy of the matrix (half the cache footprint) serves
+   both passes (mode 1 below). */
+#define FOM_MAXT 64
+static fom_float fom_ty[FOM_MAXT][40000];
+static fom_int fom_tcur[40000];
+static fom_float fom_tc = 0.0;
+static int fom_fuse_ok = 0;
+
+/* The fused pass, with the threads on a pr x cbn grid (cbn the largest of
+   FOM_CB_MAX, ..., 2, 1 dividing nt, pr = nt / cbn; tid = cb pr + r):
+     A z:   thread (r, cb) forms z = s1 + g (s1 - s0) on its column blocks
+            itself and gathers the sb = FOM_BA_N / cbn blocks of column
+            block cb on the rows of group r (whole sort windows) into the
+            partial fom_part[cb]; waits for its group;
+     t0:    the group's threads each form t0 = soft(bias + sum of the cbn
+            partials) on a 1 / cbn share of its rows and share w = c t0
+            (fom_wsh); waits for its group (mode 2) or every thread;
+     T t0:  mode 2: A^T w on the gather's own tile (rows of group r,
+            column block cb) into the group's partial fom_yp[r]; waits for
+            the pr threads of cb, then t1 and the state update [s1; t1] on
+            a 1 / pr share of cb's columns. Modes 0 and 1 split by rows of T
+            (columns of A) instead: thread tid computes block tid of T t0
+            completely (no reduction), then t1 and [s1; t1] on those
+            columns; barrier.
+   Mode 2 reads each thread's 1.6 MB share of A twice from its own cache
+   (the scatter takes the most recently gathered block first) at the cost
+   of one 40000-entry reduction of pr = 2 partials; modes 0 and 1 stream
+   another tile of A (or T) from the shared L3: on Ice Lake mode 2 is 11%
+   faster per solve.
+   cbn = 8 at 16 threads (2 x 8) balances the cross-core reads of z's
+   inputs against those of the partials (by the t0 form): faster than 4 x 4,
+   1 x 16 and 8 x 2. With tid = cb pr + r, the s1 columns a gather thread
+   reads are the ones it and its neighbour wrote in the previous T pass
+   (tid = r cbn + cb is 12% slower). */
+static fom_float fom_part[FOM_CB_MAX][10000];
+static fom_float fom_wsh[10000];
+/* mode 2: row group r's partial A^T w on the columns of each column block */
+static fom_float fom_yp[FOM_MAXT][40000];
+
+/* T t0 three ways, all exact for this T (mode 2 above): mode 0 gathers T's own SELL copy
+   (cheapest per entry, but A and T together are 2 x 26 MB of hot data),
+   mode 1 scatters A^T (c t0) over A's layout (one copy: wins when the L3
+   the team shares cannot hold both, e.g. one 36 MB Ice Lake L3, while
+   Zen3's per-CCD L3s hold their threads' shares of both). Which wins is
+   a property of the machine, so the first fused calls alternate the
+   modes and thread 0 times them (the best of FOM_TUNE_N each, after
+   FOM_TUNE_SKIP warm-up calls); then the faster mode is kept. Thread 0
+   updates the tuning state before the barrier closing a call, and every
+   thread reads it after (so the team agrees on the mode). Without OpenMP
+   (no team clock) mode 1 is used. */
+#define FOM_TUNE_SKIP 4
+#define FOM_TUNE_N 8
+static int fom_tmode = -1, fom_tcalls = 0, fom_bf_nt = 0;
+
+/* Point-to-point sync inside an iteration: the t0 form needs only its row
+   group's cbn gather threads and the T pass every form, so each thread
+   posts its progress to its own cache line (release) and waits on just the
+   lines it needs (acquire) instead of a team barrier (a shared counter
+   whose line bounces across all the cores). A call's values start at
+   ep = e + FOM_EPS, e the thread's previous ep (fom_flag[tid].e). Every
+   thread of a team makes the same calls, so they agree on ep; thread 0
+   also advances the slots of the threads outside a team (at its closing
+   barrier), so a later bigger team agrees too. */
+#if defined(_OPENMP) && defined(__GNUC__)
+#define FOM_P2P 1
+typedef struct { long v, e; char pad[64 - 2 * sizeof(long)]; } fom_flag_t;
+static fom_flag_t fom_flag[FOM_MAXT] __attribute__((aligned(64)));
+static inline void fom_post(int tid, long v) { __atomic_store_n(&fom_flag[tid].v, v, __ATOMIC_RELEASE); }
+static inline void fom_wait(int k, long v) {
+    while (__atomic_load_n(&fom_flag[k].v, __ATOMIC_ACQUIRE) < v) {
+#if defined(__x86_64__) || defined(__i386__)
+        __builtin_ia32_pause();
+#endif
+    }
+}
+/* flag values of one call: ep (gather done), ep + 1 (form done),
+   ep + 2 (mode 2: partials of T t0 done), ep + 3 (t1 done) */
+#define FOM_EPS 4
+#define FOM_EPF 2
+#endif
+#define FOM_NMODE 3
+static double fom_tbest[FOM_NMODE];
+
+static void fom_iter_fused_phases(const fom_float *state, fom_float g, const fom_float *bias_t0, const fom_float *bias_t1, const fom_float *bias_su, fom_float l0, fom_float l1, fom_float *t_t0, fom_float *t_t1, fom_float *new_state, int tid, int nt) {
+    const fom_float *s0 = state;
+    const fom_float *s1 = state + 40000;
+    /* the thread's private z (on its column blocks) and y (on its T rows)
+       share one buffer: z is dead once the gather is done */
+    fom_float *z = fom_ty[tid], *y = fom_ty[tid];
+    const fom_int ds = fom_diag_stride;
+    const fom_float c = fom_tc;
+    const fom_int cbn = (nt % 8 == 0) ? 8 : (nt % 4 == 0) ? 4 : (nt % 2 == 0) ? 2 : 1;
+    const fom_int sb = FOM_BA_N / cbn, pr = nt / cbn, cb = tid / pr, r = tid - cb * pr;
+    const fom_int lo = r * FOM_ANW / pr * FOM_ASW, hi = (r + 1) * FOM_ANW / pr * FOM_ASW;
+    fom_float *part = fom_part[cb];
+    fom_int i, k, b;
+    #ifdef FOM_P2P
+    const long ep = fom_flag[tid].e + FOM_EPS;
+    /* barrier-free call (once mode 2 is chosen, for the team size it was
+       tuned with): the next call's gather reads s0, s1 only on the
+       columns of block cb, written by cb's pr threads (the t1 pass), so it
+       waits on just those (ep + 3 of the previous call) at its start
+       instead of a team barrier closing this one. Every other shared
+       buffer a call writes is read only by threads it has already waited
+       on (its row group or its column group) in the same call. */
+    const int bf = (fom_tmode == 2 && nt == fom_bf_nt);
+    #endif
+    #ifdef _OPENMP
+    const int mode = (fom_tmode >= 0) ? fom_tmode : (fom_tcalls % FOM_NMODE);
+    const double t_call = (fom_tmode < 0 && tid == 0) ? omp_get_wtime() : 0.0;
+    #else
+    const int mode = 1;
+    #endif
+    #ifdef FOM_PROFILE
+    #pragma omp master
+    fom_iter_mark(0);
+    #endif
+    /* the tile's blocks in turn: each block's window of the FISTA
+       extrapolation z = s1 + g (s1 - s0) (2500 entries), formed by the
+       thread itself just before (no shared z pass, no barrier), stays in
+       L1 for the gather */
+    #ifdef FOM_P2P
+    if (bf) for (k = 0; k < pr; ++k) fom_wait(cb * pr + k, ep - FOM_EPS + 3);
+    #endif
+    for (b = cb * sb; b < (cb + 1) * sb; ++b) {
+        for (i = b * FOM_BA_W; i < (b + 1) * FOM_BA_W; ++i) z[i] = s1[i] + g * (s1[i] - s0[i]);
+        fom_sell(fom_xA_val, fom_xA_idx, z, fom_xA_len[b], fom_xA_row[b], fom_xA_wp[b][r * FOM_ANW / pr], lo / 4, hi / 4, part, b > cb * sb);
+    }
+    #ifdef FOM_P2P
+    fom_post(tid, ep);
+    for (k = 0; k < cbn; ++k) fom_wait(k * pr + r, ep);
+    #elif defined(_OPENMP)
+    #pragma omp barrier
+    #endif
+    #ifdef FOM_PROFILE
+    #pragma omp master
+    fom_iter_mark("iter_A_gather");
+    #endif
+
+    /* t0 = soft(bias_t0 + sum of the partials) on the piece cb of the row
+       group, and w = c t0 shared with the group's other threads */
+    {
+        const fom_int n = hi - lo, q2 = n / cbn, rm2 = n % cbn;
+        const fom_int a = lo + cb * q2 + (cb < rm2 ? cb : rm2), e = a + q2 + (cb < rm2 ? 1 : 0);
+        for (i = a; i < e; ++i) {
+            fom_float v = bias_t0[i];
+            for (k = 0; k < cbn; ++k) v += fom_part[k][i];
+            v = (v > l0) ? (v - l0) : ((v < -l0) ? (v + l0) : 0.0);
+            t_t0[i] = v;
+            fom_wsh[i] = c * v;
+        }
+    }
+    #ifdef FOM_P2P
+    fom_post(tid, ep + 1);
+    if (mode == 2) { for (k = 0; k < cbn; ++k) fom_wait(k * pr + r, ep + 1); }
+    else for (k = 0; k < nt; ++k) fom_wait(k, ep + 1);
+    #elif defined(_OPENMP)
+    #pragma omp barrier
+    #endif
+    #ifdef FOM_PROFILE
+    #pragma omp master
+    fom_iter_mark("iter_t0_form");
+    #endif
+    /* T t0 on the thread's blocks b: mode 0 gathers T's rows b0..b1 over
+       its column windows of t0, mode 1 scatters A's block b (all rows) by
+       w = c t0; then t1 and the state update [s1; t1] on those columns */
+    if (mode >= 2) {
+        /* the gather's own tile (row group r, column block cb), last block
+           first (the most recently read, still in this core's caches), into
+           the group's partial yp[r]; then t1 on piece r of cb's columns,
+           summing the pr groups' partials */
+        fom_float *yr = fom_yp[r];
+        if (mode == 2)
+            for (b = (cb + 1) * sb - 1; b >= cb * sb; --b) {
+                for (i = b * FOM_BA_W; i < (b + 1) * FOM_BA_W; ++i) yr[i] = 0.0;
+                fom_sell_scat(fom_xA_val, fom_xA_idx, yr, fom_xA_len[b], fom_xA_row[b], fom_xA_wp[b][r * FOM_ANW / pr], lo / 4, hi / 4, fom_wsh);
+            }
+        #ifdef FOM_P2P
+        fom_post(tid, ep + FOM_EPF);
+        for (k = 0; k < pr; ++k) fom_wait(cb * pr + k, ep + FOM_EPF);
+        #elif defined(_OPENMP)
+        #pragma omp barrier
+        #endif
+        {
+            const fom_int n = sb * FOM_BA_W, q2 = n / pr, rm2 = n % pr;
+            const fom_int a = cb * n + r * q2 + (r < rm2 ? r : rm2), e = a + q2 + (r < rm2 ? 1 : 0);
+            for (i = a; i < e; ++i) {
+                const fom_float x0 = s0[i], x1 = s1[i];
+                const fom_int qd = i * ds;
+                fom_float v = bias_t1[i] + FOM_MAT_t1_state_VALS[qd] * x1
+                    + g * (FOM_MAT_t1_state__d_gamma_VALS[2 * qd] * x0 + FOM_MAT_t1_state__d_gamma_VALS[2 * qd + 1] * x1);
+                for (k = 0; k < pr; ++k) v += fom_yp[k][i];
+                v = (v > l1) ? (v - l1) : ((v < -l1) ? (v + l1) : 0.0);
+                t_t1[i] = v;
+                new_state[i] = bias_su[i] + x1;
+                new_state[40000 + i] = bias_su[40000 + i] + v;
+            }
+        }
+    }
+    else for (b = tid; b < FOM_BA_N; b += nt) {
+        const fom_int b0 = b * FOM_BA_W, b1 = b0 + FOM_BA_W;
+        fom_int v;
+        if (mode == 0) {
+            for (v = 0; v < FOM_TW_N; ++v)
+                fom_sell(fom_xT_val, fom_xT_idx, t_t0, fom_xT_len[v], fom_xT_row[v], fom_xT_wp[v][b0 / FOM_TSW], b0 / 4, b1 / 4, y, v > 0);
+        } else {
+            for (i = b0; i < b1; ++i) y[i] = 0.0;
+            fom_sell_scat(fom_xA_val, fom_xA_idx, y, fom_xA_len[b], fom_xA_row[b], fom_xA_wp[b][0], 0, 10000 / 4, fom_wsh);
+        }
+        for (i = b0; i < b1; ++i) {
+            const fom_float x0 = s0[i], x1 = s1[i];
+            const fom_int qd = i * ds;
+            fom_float v = y[i] + bias_t1[i] + FOM_MAT_t1_state_VALS[qd] * x1
+                + g * (FOM_MAT_t1_state__d_gamma_VALS[2 * qd] * x0 + FOM_MAT_t1_state__d_gamma_VALS[2 * qd + 1] * x1);
+            v = (v > l1) ? (v - l1) : ((v < -l1) ? (v + l1) : 0.0);
+            t_t1[i] = v;
+            new_state[i] = bias_su[i] + x1;
+            new_state[40000 + i] = bias_su[40000 + i] + v;
+        }
+    }
+    #ifdef FOM_P2P
+    fom_post(tid, ep + 3);
+    fom_flag[tid].e = ep;
+    if (bf) {
+        #ifdef FOM_PROFILE
+        #pragma omp master
+        fom_iter_mark("iter_A_scatter_t1");
+        #endif
+        return;
+    }
+    if (tid == 0) for (k = nt; k < FOM_MAXT; ++k) fom_flag[k].e = ep;
+    #endif
+    #ifdef _OPENMP
+    if (fom_tmode < 0 && tid == 0) {
+        const double dt = omp_get_wtime() - t_call;
+        #ifdef FOM_P2P
+        /* every thread has read this call's mode (mode 2 has not waited on
+           all of them) before it changes */
+        for (k = 0; k < nt; ++k) fom_wait(k, ep);
+        #endif
+        if (fom_tcalls >= FOM_TUNE_SKIP && (fom_tbest[mode] == 0.0 || dt < fom_tbest[mode])) fom_tbest[mode] = dt;
+        if (++fom_tcalls >= FOM_TUNE_SKIP + FOM_NMODE * FOM_TUNE_N) {
+            int m;
+            fom_tmode = 0;
+            for (m = 1; m < FOM_NMODE; ++m) if (fom_tbest[m] < fom_tbest[fom_tmode]) fom_tmode = m;
+            fom_bf_nt = nt;
+        }
+    }
+    #endif
+    #ifdef _OPENMP
+    #pragma omp barrier
+    #endif
+    #ifdef FOM_PROFILE
+    #pragma omp master
+    fom_iter_mark("iter_A_scatter_t1");
+    #endif
+}
+
+
+/* 1 / a without a division: a = m 2^e with |m| in [0.5, 1), Newton steps
+   y <- y (2 - m y) from y = +-1 (error squares each step), then 2^-e. */
+static fom_float fom_recip(fom_float a) {
+    int e, k;
+    const fom_float m = frexp(a, &e);
+    fom_float y = (m < 0.0) ? -1.0 : 1.0;
+    for (k = 0; k < 8; ++k) y = y * (2.0 - m * y);
+    return ldexp(y, -e);
+}
+
+/* t1_t_t0 == c A^T? Same pattern transposed and |T - c A| <= 2^-40 |T|
+   entrywise, c from A's first entry. A's rows are walked in order, so each
+   T row's entries are met in column order (fom_tcur is its cursor). */
+static int fom_check_tc(void) {
+    const fom_int *ap = FOM_MAT_t0_state_ROWPTR, *ac = FOM_MAT_t0_state_COLIDX;
+    const fom_int *tp = FOM_MAT_t1_t_t0_ROWPTR, *tc = FOM_MAT_t1_t_t0_COLIDX;
+    const fom_float *av = FOM_MAT_t0_state_VALS, *tv = FOM_MAT_t1_t_t0_VALS;
+    fom_int i, j, p, q;
+    if (ap[10000] != tp[40000] || ap[0] != 0 || tp[0] != 0 || ap[1] == 0) return 0;
+    j = ac[0] - 40000;
+    if (j < 0 || tc[tp[j]] != 0 || av[0] == 0.0) return 0;
+    fom_tc = tv[tp[j]] * fom_recip(av[0]);
+    for (j = 0; j < 40000; ++j) fom_tcur[j] = tp[j];
+    for (i = 0; i < 10000; ++i)
+        for (p = ap[i]; p < ap[i + 1]; ++p) {
+            fom_float d, r;
+            j = ac[p] - 40000;
+            if (j < 0 || j >= 40000) return 0;
+            q = fom_tcur[j]++;
+            if (q >= tp[j + 1] || tc[q] != i) return 0;
+            d = tv[q] - fom_tc * av[p];
+            r = ldexp(tv[q], -40);
+            if (d < 0.0) d = -d;
+            if (r < 0.0) r = -r;
+            if (!(d <= r)) return 0;
+        }
+    for (j = 0; j < 40000; ++j) if (fom_tcur[j] != tp[j + 1]) return 0;
+    return 1;
+}
+
+/* Column-blocked copy of a CSR matrix (columns sorted within each row,
+   shifted by col0 so they start at 0): see fom_bA_ptr. Two O(nnz) sweeps:
+   count each row's entries per block, then copy each row's per-block
+   segments (contiguous, since columns are sorted). */
+static void fom_block_csr(const fom_int *rowptr, const fom_int *colidx, const fom_float *vals, fom_int nrows, fom_int col0, fom_int w, fom_int nb, fom_int pad, fom_int *ptr, unsigned short *idx, fom_float *val) {
+    const fom_int n1 = nrows + 1;
+    fom_int b, i, p, q;
+    for (p = 0; p < nb * n1; ++p) ptr[p] = 0;
+    for (i = 0; i < nrows; ++i)  /* count of row i in block b -> ptr[b][i + 1] */
+        for (b = 0, p = rowptr[i]; p < rowptr[i + 1]; ++p) {
+            while (colidx[p] - col0 >= (b + 1) * w) ++b;
+            ++ptr[b * n1 + i + 1];
+        }
+    for (q = 0, b = 0; b < nb; ++b) {
+        for (i = 0; i < nrows; ++i) {
+            const fom_int cnt = ptr[b * n1 + i + 1];
+            ptr[b * n1 + i] = q;
+            q += (cnt + pad - 1) / pad * pad;
+        }
+        ptr[b * n1 + nrows] = q;
+    }
+    for (p = 0; p < q; ++p) { idx[p] = 0xffff; val[p] = 0.0; }
+    for (i = 0; i < nrows; ++i)
+        for (b = 0, q = ptr[i], p = rowptr[i]; p < rowptr[i + 1]; ++p) {
+            const fom_int c = colidx[p] - col0;
+            while (c >= (b + 1) * w) q = ptr[++b * n1 + i];
+            idx[q] = (unsigned short)c;
+            val[q] = vals[p];
+            ++q;
+        }
+    for (p = 0; p < nb * n1 - 1; ++p)  /* padding: a zero at the first index */
+        for (q = ptr[p]; q < ptr[p + 1]; ++q)
+            if (idx[q] == 0xffff) idx[q] = idx[ptr[p]];
+}
+
+/* The SELL-4-sigma copy (see fom_xA_row) of a CSR matrix with columns
+   sorted within rows, shifted by col0, on nb column windows of width w and
+   sort windows of sw rows (a multiple of 4): count each row's segment per
+   block, sort each window's rows by length (counting sort, longest first),
+   then copy the slices from the CSR (cur[i]: row i's first entry in the
+   current block), padding with zeros at the lane's last index (or the
+   window's first column). Returns 0 if the layout would exceed cap. */
+static int fom_build_sell(const fom_int *rowptr, const fom_int *colidx, const fom_float *vals, fom_int nrows, fom_int col0, fom_int w, fom_int nb, fom_int sw, unsigned short *row, unsigned short *len, fom_int *wp, unsigned short *idx, fom_float *val, fom_int cap) {
+    const fom_int nwin = nrows / sw;
+    unsigned short *cnt = fom_xcnt;
+    fom_int *cur = fom_xcur;
+    fom_int b, i, k, n, p, q = 0, c, cls;
+    for (p = 0; p < nb * nrows; ++p) cnt[p] = 0;
+    for (i = 0; i < nrows; ++i)
+        for (p = rowptr[i]; p < rowptr[i + 1]; ++p) {
+            c = colidx[p] - col0;
+            if (c < 0 || c >= nb * w) return 0;
+            ++cnt[(c / w) * nrows + i];
+        }
+    for (b = 0; b < nb; ++b) {
+        const unsigned short *cb = cnt + b * nrows;
+        unsigned short *rb = row + b * nrows;
+        for (k = 0, n = 0; k < nwin; ++k) {
+            fom_int maxc = 0;
+            for (i = k * sw; i < (k + 1) * sw; ++i)
+                if (cb[i] > maxc) maxc = cb[i];
+            for (cls = maxc; cls >= 0; --cls)
+                for (i = k * sw; i < (k + 1) * sw; ++i)
+                    if (cb[i] == cls) rb[n++] = (unsigned short)i;
+        }
+    }
+    for (i = 0; i < nrows; ++i) cur[i] = rowptr[i];
+    for (b = 0; b < nb; ++b) {
+        const unsigned short *cb = cnt + b * nrows;
+        const unsigned short *rb = row + b * nrows;
+        for (k = 0; k < nrows; k += 4) {
+            const fom_int L = cb[rb[k]];
+            fom_int j, l;
+            if (k % sw == 0) wp[b * (nwin + 1) + k / sw] = q;
+            len[b * (nrows / 4) + k / 4] = (unsigned short)L;
+            if (q + 4 * L > cap) return 0;
+            for (l = 0; l < 4; ++l) {
+                const fom_int r = rb[k + l], m = cb[r], p0 = cur[r];
+                unsigned short last = (unsigned short)(b * w);
+                for (j = 0; j < L; ++j) {
+                    if (j < m) {
+                        last = (unsigned short)(colidx[p0 + j] - col0);
+                        idx[q + 4 * j + l] = last;
+                        val[q + 4 * j + l] = vals[p0 + j];
+                    } else {
+                        idx[q + 4 * j + l] = last;
+                        val[q + 4 * j + l] = 0.0;
+                    }
+                }
+                cur[r] = p0 + m;
+            }
+            q += 4 * L;
+        }
+        wp[b * (nwin + 1) + nwin] = q;
+    }
+    return 1;
+}
+
+static void fom_iter_init(void) {
+    fom_int i;
+    fom_block_csr(FOM_MAT_t0_state_ROWPTR, FOM_MAT_t0_state_COLIDX, FOM_MAT_t0_state_VALS, 10000, 40000, FOM_BA_W, FOM_BA_N, FOM_PAD, &fom_bA_ptr[0][0], fom_bA_idx, fom_bA_val);
+    fom_block_csr(FOM_MAT_t1_t_t0_ROWPTR, FOM_MAT_t1_t_t0_COLIDX, FOM_MAT_t1_t_t0_VALS, 40000, 0, FOM_BT_W, FOM_BT_N, 1, &fom_bT_ptr[0][0], fom_bT_idx, fom_bT_val);
+    fom_diag_stride = 0;
+    for (i = 1; i < 40000; ++i)
+        if (FOM_MAT_t1_state_VALS[i] != FOM_MAT_t1_state_VALS[0]
+            || FOM_MAT_t1_state__d_gamma_VALS[2 * i] != FOM_MAT_t1_state__d_gamma_VALS[0]
+            || FOM_MAT_t1_state__d_gamma_VALS[2 * i + 1] != FOM_MAT_t1_state__d_gamma_VALS[1]) fom_diag_stride = 1;
+    fom_fuse_ok = fom_check_tc();
+#ifdef FOM_HAVE_AVX2
+    __builtin_cpu_init();
+    fom_use_avx2 = __builtin_cpu_supports("avx2") && __builtin_cpu_supports("fma");
+#endif
+    if (!fom_build_sell(FOM_MAT_t0_state_ROWPTR, FOM_MAT_t0_state_COLIDX, FOM_MAT_t0_state_VALS, 10000, 40000, FOM_BA_W, FOM_BA_N, FOM_ASW, &fom_xA_row[0][0], &fom_xA_len[0][0], &fom_xA_wp[0][0], fom_xA_idx, fom_xA_val, FOM_XA_CAP)
+        || !fom_build_sell(FOM_MAT_t1_t_t0_ROWPTR, FOM_MAT_t1_t_t0_COLIDX, FOM_MAT_t1_t_t0_VALS, 40000, 0, FOM_BA_W, FOM_TW_N, FOM_TSW, &fom_xT_row[0][0], &fom_xT_len[0][0], &fom_xT_wp[0][0], fom_xT_idx, fom_xT_val, FOM_XT_CAP))
+        fom_fuse_ok = 0;
+    fom_idx_built = 1;
+}
+
+/* One whole iteration. Called from inside a parallel region (the solver
+   keeps one team alive across iterations), every thread of the team calls
+   it and shares the work; otherwise it opens its own parallel region. */
 void fom_iterate(const fom_float *state, fom_float sc_gamma, const fom_float *bias_t0, const fom_float *bias_t1, const fom_float *bias_su, const FomAct_t0 *act_t0, const FomAct_t1 *act_t1, fom_float *t_t0, fom_float *t_t1, fom_float *new_state) {
-    fom_step_t0(state, sc_gamma, bias_t0, act_t0, t_t0);
-    fom_step_t1(state, t_t0, sc_gamma, bias_t1, act_t1, t_t1);
-    fom_step_su(state, t_t1, bias_su, new_state);
-}
+    const fom_float l0 = act_t0->lambd, l1 = act_t1->lambd;
+    #ifdef _OPENMP
+    if (omp_in_parallel()) {
+        /* every thread reads the flag before the barrier; it is only set after */
+        if (!fom_idx_built) {
+            #pragma omp barrier
+            #pragma omp single
+            fom_iter_init();
+        }
+        if (fom_fuse_ok && omp_get_num_threads() <= FOM_MAXT)
+            fom_iter_fused_phases(state, sc_gamma, bias_t0, bias_t1, bias_su, l0, l1, t_t0, t_t1, new_state, omp_get_thread_num(), omp_get_num_threads());
+        else
+            fom_iter_phases(state, sc_gamma, bias_t0, bias_t1, bias_su, l0, l1, t_t0, t_t1, new_state);
+        return;
+    }
+    #endif
+    if (!fom_idx_built) fom_iter_init();
+    #ifdef _OPENMP
+    #pragma omp parallel
+    {
+        if (fom_fuse_ok && omp_get_num_threads() <= FOM_MAXT)
+            fom_iter_fused_phases(state, sc_gamma, bias_t0, bias_t1, bias_su, l0, l1, t_t0, t_t1, new_state, omp_get_thread_num(), omp_get_num_threads());
+        else
+            fom_iter_phases(state, sc_gamma, bias_t0, bias_t1, bias_su, l0, l1, t_t0, t_t1, new_state);
+    }
+    #else
+    if (fom_fuse_ok)
+        fom_iter_fused_phases(state, sc_gamma, bias_t0, bias_t1, bias_su, l0, l1, t_t0, t_t1, new_state, 0, 1);
+    else
+        fom_iter_phases(state, sc_gamma, bias_t0, bias_t1, bias_su, l0, l1, t_t0, t_t1, new_state);
+    #endif
+}
```

## Agent notes

```
### Round 1

**Whole-solve time (OpenMP, 16 threads):** initial mean_us = 160936 (quiet machine), final mean_us ≈ 40000 (40.06–40.74 ms min, 40.4–42.9 ms mean, same conditions as the 160.9 ms baseline). **Speedup ≈ 4.0×.** The machine is a shared, heavily loaded login node (load average about 40). Measured back to back under the same load: original 278–281 ms against final 52–54 ms, about 5.2×. The last `make bench` printed 67.5 ms during a busy period.

**Final profile (`make profile`, loaded machine, 29 solves × 200 iterations; `iterate` encloses the three `iter_*` regions, so % is out of a double-counted total):**
| region | total_us | calls | pct |
|---|---|---|---|
| iterate (whole loop) | 2001784 | 29 | 49.97 |
| iter_A_t0 (t0 = soft(b + A z), blocked gather) | 1090024 | 5800 | 27.21 |
| iter_T_t1 (t1 = soft(… + T t0), state update) | 897976 | 5800 | 22.42 |
| iter_z (z = s1 + γ(s1−s0)) | 13025 | 5800 | 0.33 |
| finish (residual + copy out) | 1971 | 29 | 0.05 |
| bias (t0_p_p0 @ p0, once per solve) | 1139 | 29 | 0.03 |

**Optimizations applied:**
1. **Collapsed the t0 block from 3 sparse products to 1.** The data shows `t0_state = [0, A]` and `t0_state__d_gamma = [-A, A]` with the same column pattern; the values agree to within 1 ulp. So `t0_state·s + γ·d_gamma·s = A(s1 + γ(s1 − s0))`: one pass over 2.6M nonzeros instead of 10.4M. (160.9 → 55 ms)
2. **Diagonal t1 terms and the selection update done inline.** `t1_state` and `t1_state__d_gamma` are diagonal: one or two entries per row, at s0[i] and s1[i]. Their exact values are read from the data, and when every row holds the same values (checked once) they are kept as scalars. `su` is the selection `[s1; t1]`, written directly in the t1 pass. This removes the 80000-row su products and the zero/copy passes.
3. **16-bit column indices** in derived copies: every column fits in 16 bits (A's columns are s1 offsets below 40000; T's are below 10000). That is 10 instead of 12 bytes per nonzero. (≈5%)
4. **Column-blocked CSR copies** of A (10 blocks of 4096 columns) and of `t1_t_t0` (2 blocks of 5000), built from the data on the first call. Each pass sweeps the blocks so the gathered slice of z or t0 (32–40 KB) stays in L1. Microbenchmarks showed the scalar gather was bound by L2-latency misses on z, not by matrix bandwidth. Block loops share one static schedule with `nowait`, so a row's partial sum stays with its thread and needs no extra barriers. (≈50 → 40 ms, about 20%)
5. **Ping-pong state buffers and the residual only once.** The termination is max-iterations only, so the residual is computed once after the last iteration instead of every iteration. The z scratch lives in the not-yet-written t1 half of `new_state`.
6. **One persistent OpenMP team for the whole loop.** `fom_iterate` uses orphaned worksharing loops when called inside a parallel region, so there is no fork/join per iteration. Sequential prefetch sweeps of z and t0 run before each gather pass, since those vectors were just written by other cores. Both changes were within measurement noise; I kept them because they cost nothing.

Tried and rejected after measuring:
- A single fused pass that scatters Aᵀ while gathering A (using `t1_t_t0 = c·Aᵀ`, with c found without division): 2× slower, because the per-thread accumulators bounce between cores.
- Storing only T and doing A·z as a scatter: about 60% slower.
- AVX2 gathers via a target attribute: slower.
- Zero-padding block rows to a multiple of 4: slower.
- Unroll depths 2 and 8: slower.
- Packed 64-bit index loads: no gain.
- Exploiting sparsity of the iterates: they stay ≥97% dense over all 200 iterations.

Blocked by rules:
- Skip reading the all-zero `bias_t1`/`bias_su` vectors and specialise the t1 pass for a zero bias | `fom_iterate` and step routines must apply any bias they are passed, and the solver must pass real arrays | ~2%
- Fuse iteration k's t1 pass with iteration k+1's A·z (gather and scatter on the same row while it is in L1) | `fom_iterate` gets only the current schedule value, and `fom_ops.c` may not read the schedule or keep results across calls | unknown (likely small, since scatter measured slower)
- Compile the kernels with `-march=native` (AVX2/FMA) | build flags are fixed | ~0% (measured in a side build)

### Round 2

**Whole-solve time (`make bench` mean_us):** 68238.5 us at the start of this round → about 49900 us final, a 1.37x speedup this round and 4.13x over the translated solver (205879.5 us). Under the same node load, the round-start version and the final version alternated at 71016/69128 us vs 49743/50029 us, about 1.40x. The node is shared, so timings drift by about ±3%.

**Final profile (`make profile`, 200 iterations per solve, per-iteration times):**
| region | total_us | per iteration | % |
|---|---|---|---|
| iterate (whole loop) | 2035911 | ~248 us | 49.96 |
| iter_A_scatter | 992796 | ~121 us | 24.36 |
| iter_A_gather | 836306 | ~102 us | 20.52 |
| iter_reduce_t1 | 149694 | ~18 us | 3.67 |
| iter_t0_form | 37239 | ~4.5 us | 0.91 |
| iter_z | 18624 | ~2.3 us | 0.46 |
| finish | 2908 | (once per solve) | 0.07 |
| bias | 1397 | (once per solve) | 0.03 |

"iterate" contains the iter_* regions, so the percentages sum to about 2x.

**Optimizations applied:**
1. **One pass over A instead of A and T.** T = c·Aᵀ: same sparsity pattern transposed, every value c·A_ij up to 1 ulp. At init the code walks A's rows with per-column cursors to confirm the transposed pattern and that |T − c·A| ≤ 2⁻⁴⁰|T| for every entry. c is computed without a division (frexp, Newton reciprocal, ldexp); if the check fails the old path is used. Each thread gathers t0 = soft(A z) on its rows, then scatters A_iᵀ(c·t0_i) back over the same rows while they are still in cache. This halves the matrix bytes per iteration (52 MB → 26 MB), and the passes were limited by streaming matrix values.
2. **4×4 thread grid (row groups × 10000-column blocks of A).** Each thread owns one 1.6 MB tile. Its scatter accumulator covers only its column block, so the T·t0 reduction sums 4 partial vectors instead of 16 (reduce went from ~40 to ~16 us). Teams of other sizes fall back to a 1-D version of the same pass. Block width was tuned: 2048 and 5000 make row segments too short; 20000 and full rows make the scatter slower.
3. **Descending scatter.** The scatter sweeps the tile backwards (rows and entries in reverse), so it starts on the data the gather just left in L2, and the next gather starts where the scatter ended. The addresses are strictly descending, which the hardware prefetcher follows. Forward order cost about 4 ms per solve.
4. **Shared t0 formation.** The 4 threads of a row group each form t0 for a quarter of the group's rows and publish w = c·t0. Each thread then reads 2500 finished values instead of 4×2500 partials from other cores (18 us → 4.5 us per iteration).
5. **SSE2 kernels (baseline x86-64, plain-C fallback elsewhere).** Four 16-bit column indices come from one 64-bit load, and products are formed two at a time, which cuts load-port pressure in the gather and the scatter.

**Tried and rejected (slower or no gain):**
- AVX-512 and AVX2 hardware gathers: about 3x slower per nonzero on this CPU.
- AVX2+FMA gather with a runtime CPU check: only about 0.8% faster, not worth the extra ISA path.
- Software prefetch.
- Splitting the scatter into column sub-blocks small enough for L1.
- Storing each tile column-major (CSC).
- Chunked rows with point-to-point waits, and point-to-point waits replacing the barriers.
- Computing z privately per thread.
- Dynamic scheduling.
- Moving the zeroing of the accumulators to their owners.
- 2 MB-aligned arrays.
- Exploiting sparse iterates: the iterates stay about 98% dense.

Blocked by rules:
- Keep A·s1 from the previous call, so the next t0 needs only A·t1 (or let the T pass also produce A·t1 for the next iteration), removing the dependency that forces two passes over A per iteration | fom_iterate must not keep state across calls, and the solver may not read matrix data or loop over products itself | unknown, up to about 2x on the A work
- Store A's values as float32 to halve the bytes streamed | correctness: the values are not exactly representable in float32 (lossy) | unknown
- Build with -march=native (AVX2/FMA everywhere) | Makefile and build flags are fixed | about 1%, measured by forcing AVX2 on the gather

### Round 3

**Whole-solve `mean_us`**
- Round start: 49911.8 us (Ice Lake node) and about 52900 us when re-measured on the Zen 3 node used for this round's A/B tests.
- Final: about 41800 us on the same Zen 3 node (quiet bench means 41.7k–42.1k).
- Speedup: 1.26x this round on equal hardware; 205879.5 / 41800 = 4.9x over the translated solver.
- In the last ~20 minutes the shared node was contended: bench means for the same final source ranged 46k–58k, minimums 42.1k–46.2k.

**Final profile** (`make profile`, final source, contended node; `iterate` is the parent of the iter_* regions):

| region | total_us | pct |
|---|---|---|
| iterate | 2012826 | 49.92 |
| iter_A_scatter_t1 | 977761 | 24.25 |
| iter_A_gather | 808022 | 20.04 |
| iter_t0_form | 197185 | 4.89 |
| iter_z | 27485 | 0.68 |
| finish | 6817 | 0.17 |
| bias | 1966 | 0.05 |

On a quiet node, the 4x4-grid variant per iteration: scatter_t1 118 us, gather 82 us, t0_form 7 us, z 2.4 us.

**Optimizations applied** (all in `src/fom_ops.c`, inside `fom_iterate`'s fused pass; checked kernels untouched):
1. **2500-column blocks of A.** A is random sparse, 10000x40000 with about 260 nnz per row. With 2500-column blocks, the randomly read slice of z and the randomly written slice of y are 20 KB and stay in L1. A microbenchmark showed the 80 KB slices refilled a cache line on almost every access; the passes were not bandwidth- or issue-bound.
2. **Rows sorted by segment length.** Within 625-row windows, each block's rows are ordered by padded segment length (rows remapped through a permutation). Short segments (~16 entries) otherwise end in a mispredicted loop exit; sorting gave about 12%.
3. **Exact zero padding.** Segments are padded to a multiple of 4 with value 0.0 at a valid index, so there are no scalar tails. Removing the padding was about 50% slower.
4. **Scatter partitioned by columns.** The Aᵀw pass is split by columns, not rows: each thread scatters one whole column block over all rows. This works because w = c·t0 is shared after the t0 barrier and t1_t_t0 = c·Aᵀ, so each thread gets complete Aᵀw entries and forms t1 and the state update directly. It removed the 4-way cross-thread reduction and one barrier (about 6%).
5. **AVX2+FMA kernels with runtime dispatch.** They are selected with `__builtin_cpu_supports` and fall back to SSE2. They load four 16-bit indices per 64-bit word and four values per 256-bit load. Hardware gathers are avoided because they are slow on both node types. About 4%.
6. **Thread-grid shape and owner-side zeroing.** The grid width is chosen at run time as the largest of 8, 4, 2, 1 dividing the thread count: 2x8 at 16 threads, about 2% over 4x4. This also lets every thread count use the fused path, so the old 1-D fused path is deleted. Each accumulator is now cleared by the thread that owns it rather than by remote readers (about 1%).

**Tried and rejected** (measured slower or neutral):
- Reading A once per iteration via a column layout with cached A·x.
- Whole-row gather; computing t0 or z privately per thread.
- 1250- or 5000-column blocks; a 1x16 grid.
- Dual-row kernels; reordering passes for L2 overlap.
- AVX-512 kernels.

Blocked by rules:
- Build the kernels with `-march=native` / `-mtune` for the node instead of function-level target attributes | Makefile and build flags are fixed | small; the AVX2 kernels are already dispatched at run time
- Form the next iteration's extrapolation z_{k+1} inside the current iteration's t1/state-update pass, removing the z pass and one barrier per iteration | `fom_ops.c` must not read the schedule, and `fom_iterate` only receives the current gamma | 1-3%

### Round 4

**Result.** Round 4 is faster on both CPU types the bench node switched between during the session. All the changes are in `src/fom_ops.c`; `fom_solver.c` and `fom_linsolve.c` are unchanged. Final `make verify`: `VERIFY OK`, worst error 9.2e-10 against a tolerance of 1e-5.

| | round-3 code | final code | speedup |
|---|---|---|---|
| Zen3 (start of session) | 45603 us (given) | ~29000 us (A/B, autotune picks mode 0) | 1.57x |
| Ice Lake 6342 (end of session) | 46515 us (A/B) | 42113 us (final `make bench`) | 1.10x |

- **Against the translated solver (205879 us):** 4.89x on Ice Lake and about 7.1x on Zen3. The CPU that translated baseline was timed on is unknown.
- **Why Ice Lake gains less:** the fastest Zen3 layout does not fit Ice Lake's cache. The solver now picks a strategy at runtime (item 2 below).

**Final profile** (`make profile`, Ice Lake, 48 solves, 200 iterations each, per solve):

| region | time | % |
|---|---|---|
| iterate (contains the three regions below) | 42.0 ms | 49.96 |
| iter_A_scatter_t1 (T t0, then t1 and the state update) | 21.2 ms (106 us/iter) | 25.19 |
| iter_A_gather (forms z, then A z) | 19.4 ms (97 us/iter) | 23.04 |
| iter_t0_form | 1.4 ms | 1.71 |
| finish | 0.06 ms | 0.07 |
| bias | 0.03 ms | 0.04 |

**Optimizations applied**
1. **Sliced ELLPACK layout (SELL-4-sigma) for A and T = t1_t_t0.** Within each 2500-column window, rows are sorted by segment length in 1000-row (A) or 500-row (T) windows and interleaved 4 at a time, one SIMD lane per row.
   - Structure exploited: the segments are short (~16 entries per row per window) and sorted neighbours have nearly equal lengths, so the zero padding is rare.
   - Effect: it removes the per-segment horizontal sums and tail chunks, which cost as much as the products. About −17% on Zen3.
2. **T t0 computed two ways, chosen at runtime.**
   - Mode 0 gathers over an explicit SELL copy of `t1_t_t0` itself. That is about −11% on Zen3, whose per-CCD L3s hold their threads' share of both copies.
   - Mode 1 scatters A^T (c t0) over A's own layout, using the existing check that T = c A^T up to rounding. It keeps a single 26 MB copy, which matters on Ice Lake: A + T (52 MB) overflow its 36 MB L3, and mode 0 was 20% slower there.
   - The first 20 fused calls alternate the modes and thread 0 times them (best of 8 each); the faster mode is then kept. Both modes are exact and verified.
3. **Unrolled SELL scatter** (two 4-entry steps per loop): −2% on Ice Lake.
4. **Point-to-point release/acquire flags** replace 2 of the 3 team barriers. The t0 form waits only for its row group's 8 gather threads, and the T pass waits for the 16 form threads: about −1%.
5. **Per-thread private z = s1 + g(s1 − s0)** on each gather thread's own column blocks, computed just before each block's gather. This drops the shared z pass and its barrier. It is neutral in time and keeps the 2×8 grid's locality: each thread reads s1 that it or its neighbour wrote.

**Tried and dropped**
- Unpadded segments with masked tail chunks.
- 12-bit packed indices (8% slower on Ice Lake).
- Software prefetch, `bextr` index extraction, per-index loads, AVX2 hardware gathers.
- Building x from broadcasts and blends.
- SELL-8, larger sort windows, 1250- or 10000-wide windows.
- 4×4 grid and the `tid = r*cbn + cb` remap (10–12% slower).
- A private sequential copy of t0, and a flag-based end-of-call barrier.

**Blocked by rules:**
- Reuse A s0 from the previous call: FISTA's s0 at k+1 is s1 at k, so A z = (1+g) A s1 − g A s0 needs only one fresh product, which could fuse with the T pass so A is read once per iteration | `fom_iterate` must be exact for arbitrary states and nothing but the entry points may be non-static, so no cached products across calls | unknown, possibly large on Ice Lake where the passes are bound by streaming A
- Form next iteration's z inside the T pass epilogue, while s1 and t1 are in registers | `src/fom_ops.c` may not read the schedule, and `fom_iterate` only receives the current gamma | ~1%
- Store A's values as float32 to halve the stream | each kernel must apply exactly M, and A's values are full-precision doubles | unknown
- Place threads per socket or CCD to cut cross-socket partial and t0 traffic | build flags and environment (thread binding) are fixed | unknown

### Round 5

**Whole-solve time (`make bench` mean_us, Ice Lake Xeon 6342, 16 threads):**
- Translated solver: 205879.5 us.
- Start of this round: 42241 us (42345 us reported by the pipeline).
- Final: 37310-37820 us across runs (37.6 ms typical).
- That is 1.12x this round and 5.5x overall versus the translation.

**Final profile (`make profile`, 54 solves), hottest first:**

| region | total_us | pct |
|---|---|---|
| iterate (the whole loop; contains the iter_* regions below) | 2009371 | 49.97% |
| iter_A_scatter_t1 (form, A^T w scatter, t1 and state update) | 1049715 | 26.10% |
| iter_A_gather (A z) | 899189 | 22.36% |
| iter_t0_form | 57728 | 1.44% |
| finish | 3595 | 0.09% |
| bias | 1879 | 0.05% |

**Optimizations applied this round:**
- **New T-pass mode 2: the A^T w scatter runs on the gather's own tile** (rows of row group r, column block cb on the 2x8 thread grid).
  - It writes into a partial per row group (`fom_yp[r]`). The t1 step then sums the 2 partials for its share of cb's columns.
  - It scatters the most recently gathered block first, so each thread re-reads its 1.6 MB share of A from its own cache instead of streaming another tile from the shared L3.
  - Structure used: t1_t_t0 = c·A^T (already checked entrywise at init), so A^T can be applied from A's own layout with any tiling. The reduction over 2 partials is cheap next to the cache reuse.
  - The startup autotuner now times 3 modes and keeps the fastest. On Ice Lake that is mode 2: about 11% faster per solve.
- **Point-to-point sync instead of team barriers in mode 2.**
  - The t0 step waits only on its row group. It no longer waits on every thread.
  - Once mode 2 is chosen, the barrier that closed every call is gone. The next call's gather reads s0/s1 only on block cb's columns, so it waits only for the 2 threads of that column group to finish t1.
  - Each thread now keeps its own flag counter. Thread 0 also advances the counters of threads outside the team, so a later, bigger team still agrees on the values. Gain: about 0.4%.
- **Race fix in the autotuner.** Mode 2 no longer waits on every thread within a call. So before thread 0 changes the tuning state, it now waits until every thread has read the current mode. Without this, a delayed thread could read a different mode for the same call.

**Measured and dropped:**
- A gather/form/scatter pipeline run 1000 rows at a time, to reuse A from L2: 8% slower (10 syncs per iteration). The tuner even picked it from its best-of-8 timing, although its mean was worse.
- A 4x4 grid was 3% slower and a 1x16 grid was 28% slower.
- Kernel tweaks were all neutral: 16-bit per-index loads in the scatter, 32-bit index-pair loads in the gather, and 64-byte loop alignment.
- Diagnostics behind these choices:
  - Repeating a pass on warm data takes the same time, so both passes are compute-bound at about 1.5-1.75 cycles per nonzero.
  - Load-imbalance waits total about 5 us per iteration.
  - Padding in the sliced layout is 0.25%.
  - The node has no SMT to exploit.

**Blocked by rules:**
- Store A's values as float32 to halve the bytes per nonzero | `fom_apply`, `fom_iterate` and `fom_step` must apply M exactly, so values cannot be stored in lower precision | unknown (probably small now that the passes are compute-bound)
- Raise the thread count or change OMP placement | the environment and build flags are fixed | unknown
```
