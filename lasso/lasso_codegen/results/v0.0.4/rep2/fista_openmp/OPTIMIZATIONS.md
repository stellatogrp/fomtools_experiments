# Optimization report

## Timing

```
translated (emitted)  214959.3 us/solve
optimized              26917.7 us/solve
speedup                  7.99x
```

Optimization outcome: **kept (re-verified correct)**.

## Rounds

| round | before (us) | after (us) | gain | kept | agent (s) | outcome |
|---:|---:|---:|---:|:---:|---:|---|
| 1 | 214959.340 | 71732.241 | 66.6% | yes | 3504 | continued |
| 2 | 37749.859 | 34780.611 | 7.9% | yes | 2994 | continued |
| 3 | 35311.336 | 26917.713 | 23.8% | yes | 3406 | continued |
| 4 | - | - | - | no | 2664 | the round left the sources unchanged |

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
| `iterate` | 80000x80000 | 10599932 | yes | random | 6 | 1.776e-14 |

### Rule violations

_None: the collected rewrite passed the rules and was kept._

## Blocked by rules

Optimizations the agent considered but did not make because a rule forbade them, in its own words.

| round | idea | rule | expected gain |
|---|---|---|---|
| 1 | Build the SELL layouts in `fom_setup` through a public init routine in `src/fom_ops.c`, instead of lazily on the first `fom_iterate` call | every function in fom_ops.c except the entry points must be static; fom_setup must not call fom_iterate | ~0 at steady state (only moves a one-time ~0.1 s setup out of the first solve) |
| 1 | AVX2/AVX-512 gathers and FMA for the SELL kernels (`-march`) | build flags are fixed | unknown, likely small: Zen 4 gathers run about 1 element per cycle, close to the current scalar rate |
| 2 | AVX2/AVX-512 kernels (FMA plus hardware gathers, which Ice Lake supports) through `-march` or `__attribute__((target))` | fixed build flags / portable embedded target (the build is SSE2-only) | unknown, likely 1.3–2x on both product kernels |
| 2 | Building the SELL layouts in `fom_setup` with a parallel first-touch, instead of lazily in the first `fom_iterate` call | `fom_ops.c` may export only the listed entry points, and `fom_setup` may not call `fom_iterate`/`fom_step_*` | about 0 on the bench (setup falls in warmup), lower first-solve latency |
| 3 | AVX2/FMA versions of the SELL gather and scatter kernels via `__attribute__((target("avx2,fma")))` with runtime CPU dispatch (4-wide value loads, fused multiply-add) | the build flags are fixed (SSE2 baseline), and forcing a wider instruction set from the source would get around them | the kernels are bound by load and instruction count and take about 85% of the time, so I estimate 15–30% on them; not measured |
| 4 | Compile the gather and scatter kernels with AVX2/FMA via `__attribute__((target))` plus a runtime CPU check, giving 256-bit value loads (fewer FP loads per entry) and FMA | build flags are fixed (`-O3`, no `-march`), and per-function target attributes would amount to changing them, which also hurts portability on the embedded target | unknown, roughly 10-20% on the gather passes |
| 4 | Inline-asm offset extraction (in-place `shr`/`movzwl`) to drop the register copies GCC adds | x86-specific asm conflicts with portable embedded C under fixed flags | unknown, under 5% |
| 4 | Choose the reduction groups from the actual CPU-to-L3 topology (on AMD one group of four threads spans two L3 caches) | needs OS topology queries (`sched_getcpu`, `/sys`) that don't belong in an allocation-free embedded solver | about 1% |

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
@@ -19,43 +20,80 @@
     memcpy(work->state, FOM_STATE_INIT, sizeof(work->state));
 }
 
+/* bias of the state update (no constant): zeros */
+static fom_float fom_zero_su[FOM_STATE_DIM];
+
 fom_int fom_solve(FomWorkspace *work) {
-    fom_float *state = work->state;
+    fom_float *cur = work->state, *nxt = work->new_state, *tmp;
     fom_float *bias_t0 = work->acc;                 /* t0_p_p0 @ p0, hoisted */
     fom_float *bias_t1 = work->acc + FOM_T0_DIM;    /* zeros */
-    const fom_int half = FOM_T1_DIM;
     fom_int it, i, max_iter = work->settings.max_iter;
     fom_float res = 0;
 
     memset(work->acc, 0, sizeof(fom_float) * (FOM_T0_DIM + FOM_T1_DIM));
     fom_apply_t0_p_p0(work->params.p0, bias_t0);
-    memcpy(state, FOM_STATE_INIT, sizeof(work->state));
+    memcpy(cur, FOM_STATE_INIT, sizeof(work->state));
 
     work->solution.status = FOM_MAX_ITER_REACHED;
+    #ifdef _OPENMP
+    if (!(FOM_USE_TOL && work->settings.check_termination) && max_iter > 0) {
+        /* no tolerance test: one thread team for all the iterations, each
+           thread calling fom_iterate (which then needs no fork/join, nor
+           a barrier between iterations) */
+        FOM_PROF_BEGIN(iterate);
+        #pragma omp parallel private(it)
+        {
+            fom_float *c = cur, *n = nxt, *sw;
+            for (it = 0; it < max_iter; ++it) {
+                fom_iterate(c, fom_sched_gamma(it), bias_t0, bias_t1, fom_zero_su, &FOM_ACT_t0, &FOM_ACT_t1,
+                            work->t0, work->t1, n);
+                sw = c; c = n; n = sw;
+            }
+        }
+        FOM_PROF_END(iterate);
+        if (max_iter & 1) { tmp = cur; cur = nxt; nxt = tmp; }
+        /* residual of the last iteration, max|new - old| */
+        FOM_PROF_BEGIN(residual);
+        #pragma omp parallel for reduction(max:res)
+        for (i = 0; i < FOM_STATE_DIM; ++i) {
+            fom_float d = cur[i] - nxt[i];
+            d = d < 0 ? -d : d;
+            if (d > res) res = d;
+        }
+        FOM_PROF_END(residual);
+        it = max_iter;
+    } else
+    #endif
     for (it = 0; it < max_iter; ++it) {
         const fom_float g = fom_sched_gamma(it);
-        fom_step_t0(state, g, bias_t0, &FOM_ACT_t0, work->t0);
-        fom_step_t1(state, work->t0, g, bias_t1, &FOM_ACT_t1, work->t1);
-        /* state update: s0 <- s1, s1 <- t1 (selection copies) */
-        res = 0;
-        #pragma omp parallel for reduction(max:res)
-        for (i = 0; i < half; ++i) {
-            fom_float a = state[half + i], b = work->t1[i];
-            fom_float d0 = a - state[i], d1 = b - a;
-            d0 = d0 < 0 ? -d0 : d0;
-            d1 = d1 < 0 ? -d1 : d1;
-            if (d0 > res) res = d0;
-            if (d1 > res) res = d1;
-            state[i] = a;
-            state[half + i] = b;
+        FOM_PROF_BEGIN(iterate);
+        fom_iterate(cur, g, bias_t0, bias_t1, fom_zero_su, &FOM_ACT_t0, &FOM_ACT_t1,
+                    work->t0, work->t1, nxt);
+        FOM_PROF_END(iterate);
+        /* residual max|new - old|: only needed for a tolerance test or
+           for the reported value after the last iteration */
+        if ((FOM_USE_TOL && work->settings.check_termination) || it == max_iter - 1) {
+            FOM_PROF_BEGIN(residual);
+            res = 0;
+            #ifdef _OPENMP
+            #pragma omp parallel for reduction(max:res)
+            #endif
+            for (i = 0; i < FOM_STATE_DIM; ++i) {
+                fom_float d = nxt[i] - cur[i];
+                d = d < 0 ? -d : d;
+                if (d > res) res = d;
+            }
+            FOM_PROF_END(residual);
         }
+        tmp = cur; cur = nxt; nxt = tmp;
         if (FOM_USE_TOL && work->settings.check_termination && res < work->settings.tol) {
             work->solution.status = FOM_SOLVED;
             ++it;
             break;
         }
     }
-    memcpy(work->solution.x, state, sizeof(work->state));
+    memcpy(work->solution.x, cur, sizeof(work->state));
+    if (cur != work->state) memcpy(work->state, cur, sizeof(work->state));
     work->solution.iter = it;
     work->solution.residual = res;
     return work->solution.status;
--- src/fom_ops.c (translated)
+++ src/fom_ops.c (optimized)
@@ -1,5 +1,19 @@
 #include "fom_ops.h"
 #include "fom_data.h"
+#include "fom_profile.h"
+#include <stdint.h>
+#include <string.h>
+#ifdef _OPENMP
+#include <omp.h>
+#endif
+
+/* Per-phase timing inside the fom_iterate parallel region (master thread,
+   right after each worksharing barrier); no-ops unless -DFOM_PROFILE. */
+#ifdef FOM_PROFILE
+#define IT_PROF_MARK(name, t) do { double t_ = fom_now_ns(); fom_prof_add(name, t_ - (t)); (t) = t_; } while (0)
+#else
+#define IT_PROF_MARK(name, t) ((void)0)
+#endif
 
 void fom_apply_t0_state(const fom_float *x, fom_float *out) {
     fom_int i, p;
@@ -210,8 +224,650 @@
     }
 }
 
+/* ---- fom_iterate fast path ------------------------------------------
+   Structure checked once (lazily, on the first call) from the member data:
+   - t0_state = [0, A] and t0_state__d_gamma = [-A, A] (same pattern, values
+     equal to A up to roundoff), so t0 = A y with y = x + g (x - xp);
+   - t1_t_t0 = c A^T for one scalar c (same pattern transposed, every value
+     equal to c A up to roundoff; c found from the first entry with a
+     division-free Newton reciprocal), so t1_t_t0 t0 = c (A^T t0) and A is
+     streamed from one copy in both passes (it then stays in cache);
+   - t1_state and t1_state__d_gamma are diagonal blocks (one entry per row,
+     at x_j, resp. at xp_j and x_j), applied exactly from their own values;
+   - su_state / su_t_t1 are the shifts s0 <- s1, s1 <- t1.
+   A is held in a sliced ELLPACK (SELL) layout split by column blocks K of
+   IT_CB columns, one per thread, the same blocks in both passes: per block
+   the rows of A[:, K] are sorted by their entry count and packed IT_W at a
+   time, entries of the slice interleaved and padded with explicit zeros,
+   block-local columns stored as 16-bit byte offsets, four per 64-bit word
+   (one load per four entries: the gathers are bound by loads).
+   - A y: the thread of block K forms y_K and writes one partial of A y per
+     row, gathering from y_K (in L1).
+   - t0 = soft(bias + sum of the IT_NK partials), in two levels: each group
+     of IT_RG consecutive threads (likely sharing an L3) first sums its own
+     partials, so far fewer partials cross between caches (the reduction is
+     bound by those transfers).
+   - A^T t0: either a scatter over the same slices into the block's column
+     sums, or a gather over a second layout (see it_var); either way the
+     thread of block K then has all of (A^T t0)_K and finishes t1_K and the
+     next state there, with no barrier in between.
+   With one thread per block (thread K takes block K in every pass) the
+   threads synchronize point to point (it_flag): a phase waits only for the
+   threads whose output it reads. A solver may keep one team for all its
+   iterations and call fom_iterate from every thread: then there is no
+   fork, join nor barrier between iterations.
+   If any check fails the generic chained step routines are used. */
+#define IT_N 40000
+#define IT_M 10000
+#define IT_NK 16      /* column blocks of A (one per thread with 16 threads) */
+#define IT_CB 2500    /* columns per block, IT_NK * IT_CB == IT_N */
+#define IT_W 20       /* rows per slice of A[:, K] */
+#define IT_NS 500     /* slices per block, IT_NS * IT_W == IT_M */
+#define IT_PAD (IT_NK * IT_NS * IT_W)          /* room for the padding */
+static int it_mode = 0;  /* 0 unchecked, 1 fast path, -1 generic */
+static fom_float it_c;   /* t1_t_t0 = it_c A^T */
+static fom_int it_sp[IT_NK * IT_NS + 1];
+static fom_int it_slen[IT_NK * IT_NS];
+static uint16_t it_perm[IT_NK][IT_M];
+static fom_float it_k_val[FOM_MAT_t0_state_NNZ + IT_PAD] __attribute__((aligned(64)));
+static uint32_t it_k_col[(FOM_MAT_t0_state_NNZ + IT_PAD) / 2 + 2] __attribute__((aligned(64)));
+/* second layout, for the gather variant of t1_t_t0 t0: rows j of block K
+   split into IT_NB bands of t0 rows; per band the rows are sorted by their
+   entry count in the band and packed IT_W at a time, band-local byte
+   offsets, gathering from the t0 band (in L1) */
+#define IT_NB 4       /* t0 bands */
+#define IT_BR 2500    /* t0 rows per band, IT_NB * IT_BR == IT_M */
+#define IT_NS2 125    /* slices per (block, band), IT_NS2 * IT_W == IT_CB */
+#define IT_PAD2 (IT_NK * IT_NB * IT_NS2 * IT_W)
+static fom_int it_dsp[IT_NK * IT_NB * IT_NS2 + 1];
+static fom_int it_dlen[IT_NK * IT_NB * IT_NS2];
+static uint16_t it_dperm[IT_NK * IT_NB][IT_CB];
+static fom_float it_d_val[FOM_MAT_t1_t_t0_NNZ + IT_PAD2] __attribute__((aligned(64)));
+static uint32_t it_d_col[(FOM_MAT_t1_t_t0_NNZ + IT_PAD2) / 2 + 2] __attribute__((aligned(64)));
+/* which variant of t1_t_t0 t0 runs: the scatter over A's slices streams
+   half the data (wins when the two layouts do not fit in cache), the
+   gather over the second layout needs no stores (wins when they do); the
+   first IT_TUNE fast calls time both, alternating in runs of 32 calls (the
+   first 8 of a run left out: caches and clocks still warming up), and the
+   variant with the best time is kept */
+#define IT_TUNE 192
+#define IT_RG 4      /* partials per reduction group */
+#define IT_RR 2500   /* rows per group member, IT_RG * IT_RR == IT_M */
+#define IT_LRG 2     /* IT_RG == 1 << IT_LRG */
+#define IT_M2 625    /* t0 rows per thread in level 2, IT_NK * IT_M2 == IT_M */
+#define IT_TB 4      /* threads per t0 band in level 2, IT_NB * IT_TB == IT_NK */
+static int it_var = -1;
+static fom_int it_ntune = 0;
+static int it_have[2];
+static double it_best[2];
+/* workspace */
+static fom_int it_cur[IT_N];
+static fom_int it_len[IT_M];
+static fom_float it_part[IT_NK][IT_M];
+/* blocks are processed whole: with as many threads as blocks thread K
+   takes block K in both passes (so its share of the layouts stays in its
+   own caches); otherwise the blocks are claimed one at a time from a
+   shared counter. Whoever processes block K of the A^T pass has all of
+   (A^T t0)_K, so it finishes t1_K and the next state there. */
+static fom_int it_cnt1, it_cnt2;
+/* point-to-point sync with one thread per block: thread t sets its flag of
+   a phase to the call's epoch (release) when done, and a thread waits
+   (acquire) only for the threads whose output it reads */
+typedef struct { volatile unsigned v; char pad[60]; } it_flag;
+static it_flag it_fA[IT_NK], it_fL1[IT_NK], it_fL2[IT_NK], it_fT[IT_NK];
+/* per-thread call counters (equal for all threads between calls) and
+   whether the thread has passed the setup of the persistent-team path */
+static it_flag it_tep[IT_NK], it_seen[IT_NK];
+static unsigned it_sh_ep;  /* the epoch of a call of a team of another size */
+static inline void it_set(it_flag *f, unsigned e) { __atomic_store_n(&f->v, e, __ATOMIC_RELEASE); }
+static inline void it_wait(it_flag *f, unsigned e) {
+    while (__atomic_load_n(&f->v, __ATOMIC_ACQUIRE) != e) ;
+}
+static inline void it_wait2(it_flag *f, unsigned e0, unsigned e1) {
+    unsigned v;
+    while ((v = __atomic_load_n(&f->v, __ATOMIC_ACQUIRE)) != e0 && v != e1) ;
+}
+
+static fom_int it_claim(fom_int *cnt) {
+    fom_int c;
+    #ifdef _OPENMP
+    #pragma omp atomic capture
+    #endif
+    c = (*cnt)++;
+    return c;
+}
+
+static fom_float it_abs(fom_float a) { return a < 0 ? -a : a; }
+
+static int it_close(fom_float a, fom_float b) {
+    /* |a - b| <= 2^-40 |b| */
+    fom_float t = it_abs(b);
+    int k;
+    for (k = 0; k < 40; ++k) t *= 0.5;
+    return it_abs(a - b) <= t;
+}
+
+/* 1 / a without a division: scale |a| by powers of two into [0.5, 1),
+   then Newton steps r <- r (2 - b r) from r = 1 (error halves squared) */
+static fom_float it_recip(fom_float a) {
+    fom_float b = it_abs(a), sc = 1.0, r = 1.0;
+    int k;
+    while (b >= 1.0) { b *= 0.5; sc *= 0.5; }
+    while (b < 0.5) { b *= 2.0; sc *= 2.0; }
+    for (k = 0; k < 8; ++k) r = r * (2.0 - b * r);
+    r *= sc;
+    return a < 0 ? -r : r;
+}
+
+/* sort the n row segments by length (descending, stable) into perm and pack
+   them IT_W per slice: slice s gets offset sp[s] and length slen[s] (its
+   longest segment); segment of row t starts at it_cur[base + t] in (vals, cols)
+   with cols shifted by -off. Returns the next free slot, or -1 if the
+   padded slices do not fit below cap. */
+static fom_int it_pack(fom_int n, fom_int base, const fom_int *len,
+                       const fom_float *vals, const fom_int *cols, fom_int off,
+                       fom_int *sp, fom_int *slen, uint16_t *perm,
+                       fom_float *oval, uint32_t *ocol, fom_int q, fom_int cap) {
+    fom_int t, k, r, s, m, maxlen = 0;
+    for (t = 0; t < n; ++t) if (len[t] > maxlen) maxlen = len[t];
+    for (m = 0, k = maxlen; k >= 0; --k)
+        for (t = 0; t < n; ++t)
+            if (len[t] == k) perm[m++] = (uint16_t)t;
+    for (s = 0; s * IT_W < n; ++s) {
+        fom_int L = len[perm[s * IT_W]];
+        if (q < 0 || L * IT_W > cap - q) return -1;
+        sp[s] = q;
+        slen[s] = L;
+        for (k = 0; k < L; ++k)
+            for (r = 0; r < IT_W; ++r) {
+                t = perm[s * IT_W + r];
+                if (k < len[t]) {
+                    fom_int p = it_cur[base + t] + k;
+                    oval[q] = vals[p];
+                    ocol[q >> 1] |= (uint32_t)(8 * (cols[p] - off)) << (16 * (q & 1));
+                } else {
+                    oval[q] = 0.0;
+                }
+                ++q;
+            }
+    }
+    return q;
+}
+
+/* one SELL slice of IT_W rows, gather form: acc[r] = sum_k v[k IT_W + r] x[col],
+   accumulated as pairs of rows (one multiply-add per two entries); the
+   columns are stored as byte offsets 8 col, two 16-bit offsets per word */
+typedef double it_v2 __attribute__((vector_size(16)));
+static inline fom_float *it_at(const fom_float *x, uint64_t off) {
+    return (fom_float *)((const char *)x + off);
+}
+static inline void it_g4(it_v2 *a, const it_v2 *v, uint32_t c0, uint32_t c1,
+                         const fom_float *x) {
+    it_v2 xa = {*it_at(x, c0 & 0xffff), *it_at(x, c0 >> 16)},
+          xb = {*it_at(x, c1 & 0xffff), *it_at(x, c1 >> 16)};
+    a[0] += v[0] * xa;
+    a[1] += v[1] * xb;
+}
+typedef uint64_t it_w64 __attribute__((may_alias));
+static inline void it_g8(it_v2 *a, const it_v2 *v, uint64_t c, const fom_float *x) {
+    it_v2 xa = {*it_at(x, c & 0xffff), *it_at(x, (c >> 16) & 0xffff)},
+          xb = {*it_at(x, (c >> 32) & 0xffff), *it_at(x, c >> 48)};
+    a[0] += v[0] * xa;
+    a[1] += v[1] * xb;
+}
+static inline void it_slice(const fom_float *restrict vv, const uint32_t *restrict w32, fom_int L,
+                            const fom_float *restrict x, fom_float *acc) {
+    const it_v2 *v = (const it_v2 *)vv;
+    const it_w64 *w = (const it_w64 *)w32;
+    it_v2 a[IT_W / 2];
+    fom_int k, r;
+    for (r = 0; r < IT_W / 2; ++r) a[r] = (it_v2){0.0, 0.0};
+    for (k = 0; k < L; ++k, v += IT_W / 2, w += IT_W / 4) {
+        it_g8(a, v, w[0], x);
+        it_g8(a + 2, v + 2, w[1], x);
+        it_g8(a + 4, v + 4, w[2], x);
+        it_g8(a + 6, v + 6, w[3], x);
+        it_g8(a + 8, v + 8, w[4], x);
+    }
+    memcpy(acc, a, sizeof a);
+}
+
+/* the same slice, scatter form: y[col] += v[k IT_W + r] t[r], t held in
+   registers as pairs (one multiply per two entries) */
+static inline void it_s4(fom_float *restrict y, const it_v2 *v, uint32_t c0, uint32_t c1,
+                         it_v2 ta, it_v2 tb) {
+    it_v2 pa = v[0] * ta, pb = v[1] * tb;
+    *it_at(y, c0 & 0xffff) += pa[0];
+    *it_at(y, c0 >> 16) += pa[1];
+    *it_at(y, c1 & 0xffff) += pb[0];
+    *it_at(y, c1 >> 16) += pb[1];
+}
+static inline void it_scatter(const fom_float *restrict vv, const uint32_t *restrict w, fom_int L,
+                              const fom_float *t, fom_float *restrict y) {
+    const it_v2 *v = (const it_v2 *)vv;
+    it_v2 T[IT_W / 2];
+    fom_int k;
+    memcpy(T, t, sizeof T);
+    for (k = 0; k < L; ++k, v += IT_W / 2, w += IT_W / 2) {
+        it_s4(y, v, w[0], w[1], T[0], T[1]);
+        it_s4(y, v + 2, w[2], w[3], T[2], T[3]);
+        it_s4(y, v + 4, w[4], w[5], T[4], T[5]);
+        it_s4(y, v + 6, w[6], w[7], T[6], T[7]);
+        it_s4(y, v + 8, w[8], w[9], T[8], T[9]);
+    }
+}
+
+static int it_setup(void) {
+    fom_int i, j, k, n, p, q, K;
+    if (IT_RG != 1 << IT_LRG || IT_RG * IT_RR != IT_M || IT_NK * IT_M2 != IT_M || IT_RR != IT_M2 << IT_LRG ||
+        IT_BR != IT_M2 * IT_TB || IT_NB * IT_TB != IT_NK || IT_NB != IT_RG)
+        return -1;
+    if (IT_M > 65536 || 8 * IT_CB > 65536 || IT_NB * IT_NS2 != IT_NS || (IT_NK & (IT_NK - 1)) != 0 ||
+        IT_NK * IT_CB != IT_N || IT_NS * IT_W != IT_M || (IT_W & 3) != 0)
+        return -1;
+    /* t0 members: [0, A] and [-A, A], columns sorted within rows */
+    for (i = 0; i < IT_M; ++i) {
+        p = FOM_MAT_t0_state_ROWPTR[i];
+        n = FOM_MAT_t0_state_ROWPTR[i + 1] - p;
+        q = FOM_MAT_t0_state__d_gamma_ROWPTR[i];
+        if (FOM_MAT_t0_state__d_gamma_ROWPTR[i + 1] - q != 2 * n) return -1;
+        for (k = 0; k < n; ++k) {
+            fom_int c = FOM_MAT_t0_state_COLIDX[p + k];
+            fom_float a = FOM_MAT_t0_state_VALS[p + k];
+            if (c < IT_N || c >= 2 * IT_N) return -1;
+            if (k > 0 && c <= FOM_MAT_t0_state_COLIDX[p + k - 1]) return -1;
+            if (FOM_MAT_t0_state__d_gamma_COLIDX[q + k] != c - IT_N) return -1;
+            if (FOM_MAT_t0_state__d_gamma_COLIDX[q + n + k] != c) return -1;
+            if (!it_close(-FOM_MAT_t0_state__d_gamma_VALS[q + k], a)) return -1;
+            if (!it_close(FOM_MAT_t0_state__d_gamma_VALS[q + n + k], a)) return -1;
+        }
+    }
+    /* t1_t_t0 = c A^T: walking A by rows i visits row j of t1_t_t0 in
+       column order i, so it_cur[j] steps through that row */
+    if (FOM_MAT_t0_state_NNZ < 1 || FOM_MAT_t0_state_VALS[0] == 0.0) return -1;
+    {
+        fom_int c0 = FOM_MAT_t0_state_COLIDX[0] - IT_N;
+        if (FOM_MAT_t1_t_t0_ROWPTR[c0 + 1] - FOM_MAT_t1_t_t0_ROWPTR[c0] < 1) return -1;
+        it_c = FOM_MAT_t1_t_t0_VALS[FOM_MAT_t1_t_t0_ROWPTR[c0]] * it_recip(FOM_MAT_t0_state_VALS[0]);
+    }
+    for (j = 0; j < IT_N; ++j) it_cur[j] = FOM_MAT_t1_t_t0_ROWPTR[j];
+    for (i = 0; i < IT_M; ++i)
+        for (p = FOM_MAT_t0_state_ROWPTR[i]; p < FOM_MAT_t0_state_ROWPTR[i + 1]; ++p) {
+            j = FOM_MAT_t0_state_COLIDX[p] - IT_N;
+            q = it_cur[j]++;
+            if (q >= FOM_MAT_t1_t_t0_ROWPTR[j + 1] || FOM_MAT_t1_t_t0_COLIDX[q] != i) return -1;
+            if (!it_close(it_c * FOM_MAT_t0_state_VALS[p], FOM_MAT_t1_t_t0_VALS[q])) return -1;
+        }
+    for (j = 0; j < IT_N; ++j) if (it_cur[j] != FOM_MAT_t1_t_t0_ROWPTR[j + 1]) return -1;
+    /* gather layout of t1_t_t0: per (block K, band b), the band segments of
+       rows j of K (columns sorted within rows, so it_cur walks each row
+       band by band) */
+    if (IT_NB * IT_BR != IT_M || IT_NS2 * IT_W != IT_CB || 8 * IT_BR > 65536) return -1;
+    for (j = 0; j < IT_N; ++j) it_cur[j] = FOM_MAT_t1_t_t0_ROWPTR[j];
+    for (K = 0, q = 0; K < IT_NK; ++K) {
+        fom_int b;
+        for (b = 0; b < IT_NB; ++b) {
+            fom_int kb = K * IT_NB + b;
+            for (j = 0; j < IT_CB; ++j) {
+                fom_int c = K * IT_CB + j;
+                p = it_cur[c];
+                while (p < FOM_MAT_t1_t_t0_ROWPTR[c + 1] && FOM_MAT_t1_t_t0_COLIDX[p] < (b + 1) * IT_BR) ++p;
+                it_len[j] = p - it_cur[c];
+            }
+            q = it_pack(IT_CB, K * IT_CB, it_len, FOM_MAT_t1_t_t0_VALS, FOM_MAT_t1_t_t0_COLIDX, b * IT_BR,
+                        it_dsp + kb * IT_NS2, it_dlen + kb * IT_NS2, it_dperm[kb], it_d_val, it_d_col, q,
+                        FOM_MAT_t1_t_t0_NNZ + IT_PAD2);
+            if (q < 0) return -1;
+            for (j = 0; j < IT_CB; ++j) it_cur[K * IT_CB + j] += it_len[j];
+        }
+    }
+    it_dsp[IT_NK * IT_NB * IT_NS2] = q;
+    /* SELL layout: per column block K, the row segments of A[:, K] */
+    for (K = 0, q = 0; K < IT_NK; ++K) {
+        fom_int lo = IT_N + K * IT_CB, hi = lo + IT_CB;
+        for (i = 0; i < IT_M; ++i) {
+            fom_int e = FOM_MAT_t0_state_ROWPTR[i + 1];
+            p = FOM_MAT_t0_state_ROWPTR[i];
+            while (p < e && FOM_MAT_t0_state_COLIDX[p] < lo) ++p;
+            it_cur[i] = p;
+            while (p < e && FOM_MAT_t0_state_COLIDX[p] < hi) ++p;
+            it_len[i] = p - it_cur[i];
+        }
+        q = it_pack(IT_M, 0, it_len, FOM_MAT_t0_state_VALS, FOM_MAT_t0_state_COLIDX, lo,
+                    it_sp + K * IT_NS, it_slen + K * IT_NS, it_perm[K], it_k_val, it_k_col, q,
+                    FOM_MAT_t0_state_NNZ + IT_PAD);
+        if (q < 0) return -1;
+    }
+    it_sp[IT_NK * IT_NS] = q;
+    /* diagonal t1 members (entry j of t1_state, entries 2j, 2j + 1 of
+       t1_state__d_gamma) and the su shifts */
+    for (j = 0; j < IT_N; ++j) {
+        if (FOM_MAT_t1_state_ROWPTR[j] != j || FOM_MAT_t1_state_ROWPTR[j + 1] != j + 1) return -1;
+        if (FOM_MAT_t1_state_COLIDX[j] != IT_N + j) return -1;
+        if (FOM_MAT_t1_state__d_gamma_ROWPTR[j] != 2 * j || FOM_MAT_t1_state__d_gamma_ROWPTR[j + 1] != 2 * j + 2) return -1;
+        if (FOM_MAT_t1_state__d_gamma_COLIDX[2 * j] != j || FOM_MAT_t1_state__d_gamma_COLIDX[2 * j + 1] != IT_N + j) return -1;
+        p = FOM_MAT_su_state_ROWPTR[j];
+        if (FOM_MAT_su_state_ROWPTR[j + 1] - p != 1) return -1;
+        if (FOM_MAT_su_state_COLIDX[p] != IT_N + j || FOM_MAT_su_state_VALS[p] != 1.0) return -1;
+        if (FOM_MAT_su_state_ROWPTR[IT_N + j + 1] != FOM_MAT_su_state_ROWPTR[IT_N + j]) return -1;
+        if (FOM_MAT_su_t_t1_ROWPTR[j + 1] != FOM_MAT_su_t_t1_ROWPTR[j]) return -1;
+        p = FOM_MAT_su_t_t1_ROWPTR[IT_N + j];
+        if (FOM_MAT_su_t_t1_ROWPTR[IT_N + j + 1] - p != 1) return -1;
+        if (FOM_MAT_su_t_t1_COLIDX[p] != j || FOM_MAT_su_t_t1_VALS[p] != 1.0) return -1;
+    }
+    return 1;
+}
+
+/* level 1 of the t0 reduction: partial t = sum of the IT_RG partials of
+   its group (consecutive threads, likely sharing a cache), on the rows of
+   its member slot; in place, as no other member reads partial t on these
+   rows */
+static inline void it_level1(fom_int t) {
+    const fom_int g0 = t & ~(fom_int)(IT_RG - 1), m = t & (IT_RG - 1);
+    fom_float *pt = it_part[t];
+    fom_int i, K;
+    for (i = m * IT_RR; i < (m + 1) * IT_RR; ++i) {
+        fom_float v = it_part[g0][i];
+        for (K = g0 + 1; K < g0 + IT_RG; ++K) v += it_part[K][i];
+        pt[i] = v;
+    }
+}
+
+/* the next block of a pass for thread me: K = me with one thread per
+   block, else claimed; -1 when the pass is done */
+static inline fom_int it_next(fom_int *cnt, fom_int me, fom_int nth, int *first) {
+    fom_int K;
+    if (nth == IT_NK) {
+        K = *first ? me : -1;
+        *first = 0;
+        return K;
+    }
+    K = it_claim(cnt);
+    return K < IT_NK ? K : -1;
+}
+
+/* one call's work for thread me of nth */
+typedef struct {
+    const fom_float *xp, *x, *bias_t0, *bias_t1, *bias_su;
+    fom_float *t_t0, *t_t1, *new_state;
+    fom_float l0, l1, g, cc;
+    int var;
+} it_args;
+static double it_tp __attribute__((unused));  /* profiling clock (master) */
+
+static void it_body(const it_args *A, fom_int me, fom_int nth, unsigned ep) {
+    const fom_float *xp = A->xp, *x = A->x, *bias_t0 = A->bias_t0, *bias_t1 = A->bias_t1, *bias_su = A->bias_su;
+    fom_float *t_t0 = A->t_t0, *t_t1 = A->t_t1, *new_state = A->new_state;
+    const fom_float *d1 = FOM_MAT_t1_state_VALS, *dg = FOM_MAT_t1_state__d_gamma_VALS;
+    const fom_float l0 = A->l0, l1 = A->l1, g = A->g, cc = A->cc;
+    const int var = A->var;
+    fom_int i, j, r, s, t, K;
+    int first, p2p;
+    fom_float buf[IT_CB] __attribute__((aligned(64)));
+    /* part_K[i] = A[i, K] y_K, y_K = x_K + g (x_K - xp_K) */
+    first = 1;
+    while ((K = it_next(&it_cnt1, me, nth, &first)) >= 0) {
+        const uint16_t *perm = it_perm[K];
+        const fom_float *xk = x + K * IT_CB, *xpk = xp + K * IT_CB;
+        fom_float *pk = it_part[K];
+        for (j = 0; j < IT_CB; ++j) buf[j] = xk[j] + g * (xk[j] - xpk[j]);
+        for (s = 0; s < IT_NS; ++s) {
+            const fom_int q = it_sp[K * IT_NS + s];
+            fom_float a[IT_W];
+            it_slice(it_k_val + q, it_k_col + (q >> 1), it_slen[K * IT_NS + s], buf, a);
+            for (r = 0; r < IT_W; ++r) pk[perm[s * IT_W + r]] = a[r];
+        }
+    }
+    p2p = nth == IT_NK;
+    if (p2p) {
+        /* level 1 of thread me reads the partials of its group */
+        it_set(&it_fA[me], ep);
+        for (t = me & ~(fom_int)(IT_RG - 1); t < (me | (IT_RG - 1)) + 1; ++t) it_wait(&it_fA[t], ep);
+    } else {
+        #ifdef _OPENMP
+        #pragma omp barrier
+        #endif
+    }
+    #ifdef _OPENMP
+    #pragma omp master
+    #endif
+    IT_PROF_MARK("it_Ay_partial", it_tp);
+    if (p2p) {
+        it_level1(me);
+    } else {
+        #ifdef _OPENMP
+        #pragma omp for schedule(static)
+        #endif
+        for (t = 0; t < IT_NK; ++t) it_level1(t);
+    }
+    /* level 2: t0 = soft(bias + sum of the group partials), the
+       group sums of rows in slot m being in partials g0 + m */
+    if (p2p) {
+        /* thread me: rows [me IT_M2, (me + 1) IT_M2) of slot
+           me / IT_RG, whose group sums thread me / IT_RG + g IT_RG
+           wrote in level 1 */
+        const fom_int m = me >> IT_LRG;
+        it_set(&it_fL1[me], ep);
+        for (K = m; K < IT_NK; K += IT_RG) it_wait(&it_fL1[K], ep);
+        /* t0 is overwritten here: every thread must be done with
+           the A^T pass of the previous call (with a persistent
+           team there is no join in between) */
+        for (t = 0; t < IT_NK; ++t) it_wait2(&it_fT[t], ep - 1, ep);
+        for (i = me * IT_M2; i < (me + 1) * IT_M2; ++i) {
+            fom_float v = bias_t0[i];
+            for (K = m; K < IT_NK; K += IT_RG) v += it_part[K][i];
+            t_t0[i] = (v > l0) ? (v - l0) : ((v < -l0) ? (v + l0) : 0.0);
+        }
+        it_set(&it_fL2[me], ep);
+    } else {
+        #ifdef _OPENMP
+        #pragma omp for schedule(static)
+        #endif
+        for (i = 0; i < IT_M; ++i) {
+            fom_float v = bias_t0[i];
+            fom_int m = 0;
+            while (i >= (m + 1) * IT_RR) ++m;
+            for (K = m; K < IT_NK; K += IT_RG) v += it_part[K][i];
+            t_t0[i] = (v > l0) ? (v - l0) : ((v < -l0) ? (v + l0) : 0.0);
+        }
+    }
+    #ifdef _OPENMP
+    #pragma omp master
+    #endif
+    IT_PROF_MARK("it_t0_reduce", it_tp);
+    /* cs_K = (A^T t0)_K, then t1_K = soft(bias + diagonal terms +
+       c cs_K); s0 <- s1, s1 <- t1 */
+    first = 1;
+    while ((K = it_next(&it_cnt2, me, nth, &first)) >= 0) {
+        const uint16_t *perm = it_perm[K];
+        for (j = 0; j < IT_CB; ++j) buf[j] = 0.0;
+        if (var == 0) {
+            /* scatter over the slices of A[:, K], backwards: the
+               ones last read by the A y pass are still in L2 */
+            if (p2p) for (t = 0; t < IT_NK; ++t) it_wait(&it_fL2[t], ep);
+            for (s = IT_NS - 1; s >= 0; --s) {
+                const fom_int q = it_sp[K * IT_NS + s];
+                fom_float tt[IT_W];
+                for (r = 0; r < IT_W; ++r) tt[r] = t_t0[perm[s * IT_W + r]];
+                it_scatter(it_k_val + q, it_k_col + (q >> 1), it_slen[K * IT_NS + s], tt, buf);
+            }
+        } else {
+            /* gather over the (band, slice) units of block K */
+            fom_int bb, b, u;
+            for (bb = 0; bb < IT_NB; ++bb) {
+                /* band b of t0 is level 2 of threads b IT_TB
+                   and up: start with the band of this thread's group */
+                b = (bb + (me >> IT_LRG)) & (IT_NB - 1);
+                if (p2p) for (t = b * IT_TB; t < (b + 1) * IT_TB; ++t) it_wait(&it_fL2[t], ep);
+                for (u = 0; u < IT_NS2; ++u) {
+                    const fom_int kb = K * IT_NB + b, q = it_dsp[kb * IT_NS2 + u];
+                    const uint16_t *dperm = it_dperm[kb] + u * IT_W;
+                    fom_float a[IT_W];
+                    it_slice(it_d_val + q, it_d_col + (q >> 1), it_dlen[kb * IT_NS2 + u], t_t0 + b * IT_BR, a);
+                    for (r = 0; r < IT_W; ++r) buf[dperm[r]] += a[r];
+                }
+            }
+        }
+        for (j = 0; j < IT_CB; ++j) {
+            const fom_int jj = K * IT_CB + j;
+            fom_float v = bias_t1[jj] + d1[jj] * x[jj] + g * (dg[2 * jj] * xp[jj] + dg[2 * jj + 1] * x[jj]) + cc * buf[j];
+            v = (v > l1) ? (v - l1) : ((v < -l1) ? (v + l1) : 0.0);
+            t_t1[jj] = v;
+            new_state[jj] = bias_su[jj] + x[jj];
+            new_state[IT_N + jj] = bias_su[IT_N + jj] + v;
+        }
+    }
+    if (p2p) it_set(&it_fT[me], ep);
+}
+
+static void it_args_set(it_args *A, const fom_float *state, fom_float sc_gamma, const fom_float *bias_t0, const fom_float *bias_t1, const fom_float *bias_su, const FomAct_t0 *act_t0, const FomAct_t1 *act_t1, fom_float *t_t0, fom_float *t_t1, fom_float *new_state) {
+    A->xp = state;
+    A->x = state + IT_N;
+    A->bias_t0 = bias_t0;
+    A->bias_t1 = bias_t1;
+    A->bias_su = bias_su;
+    A->t_t0 = t_t0;
+    A->t_t1 = t_t1;
+    A->new_state = new_state;
+    A->l0 = act_t0->lambd;
+    A->l1 = act_t1->lambd;
+    A->g = sc_gamma;
+    /* variant: 0 scatter over A (cs = A^T t0, scaled by c), 1 gather
+       over the t1_t_t0 layout (cs = t1_t_t0 t0) */
+    A->var = it_var >= 0 ? it_var : (int)((it_ntune >> 5) & 1);
+    A->cc = A->var ? 1.0 : it_c;
+}
+
+/* tuning: the best time of each variant (one thread) */
+static void it_tune(int var, double tt0) {
+    double dt = fom_now_ns() - tt0;
+    if ((it_ntune & 31) >= 8 && (!it_have[var] || dt < it_best[var])) {
+        it_have[var] = 1;
+        it_best[var] = dt;
+    }
+    if (++it_ntune >= IT_TUNE) it_var = (it_best[1] < it_best[0]) ? 1 : 0;
+}
+
+/* called from one thread: the work runs in a parallel region of its own */
+static void it_iterate1(const fom_float *state, fom_float sc_gamma, const fom_float *bias_t0, const fom_float *bias_t1, const fom_float *bias_su, const FomAct_t0 *act_t0, const FomAct_t1 *act_t1, fom_float *t_t0, fom_float *t_t1, fom_float *new_state) {
+    if (it_mode == 0) it_mode = it_setup();
+    if (it_mode < 0) {
+        fom_step_t0(state, sc_gamma, bias_t0, act_t0, t_t0);
+        fom_step_t1(state, t_t0, sc_gamma, bias_t1, act_t1, t_t1);
+        fom_step_su(state, t_t1, bias_su, new_state);
+        return;
+    }
+    {
+        it_args A;
+        const int tuning = it_var < 0;
+        const unsigned ep = it_tep[0].v + 1;
+        double tt0 = 0;
+        fom_int t;
+        it_args_set(&A, state, sc_gamma, bias_t0, bias_t1, bias_su, act_t0, act_t1, t_t0, t_t1, new_state);
+        if (tuning) tt0 = fom_now_ns();
+        for (t = 0; t < IT_NK; ++t) it_tep[t].v = ep;
+        it_cnt1 = 0;
+        it_cnt2 = 0;
+        #ifdef FOM_PROFILE
+        it_tp = fom_now_ns();
+        #endif
+        #ifdef _OPENMP
+        #pragma omp parallel
+        #endif
+        {
+            fom_int me = 0, nth = 1;
+            #ifdef _OPENMP
+            me = omp_get_thread_num();
+            nth = omp_get_num_threads();
+            #endif
+            it_body(&A, me, nth, ep);
+        }
+        IT_PROF_MARK("it_ATt0_t1", it_tp);
+        for (t = 0; t < IT_NK; ++t) it_fT[t].v = ep;
+        if (tuning) it_tune(A.var, tt0);
+    }
+}
+
 void fom_iterate(const fom_float *state, fom_float sc_gamma, const fom_float *bias_t0, const fom_float *bias_t1, const fom_float *bias_su, const FomAct_t0 *act_t0, const FomAct_t1 *act_t1, fom_float *t_t0, fom_float *t_t1, fom_float *new_state) {
-    fom_step_t0(state, sc_gamma, bias_t0, act_t0, t_t0);
-    fom_step_t1(state, t_t0, sc_gamma, bias_t1, act_t1, t_t1);
-    fom_step_su(state, t_t1, bias_su, new_state);
-}
+    #ifdef _OPENMP
+    if (omp_in_parallel()) {
+        /* called by every thread of an enclosing team (a solver that keeps
+           one team for all its iterations): with one thread per block the
+           threads synchronize only through the flags, with no barrier
+           between iterations; otherwise one thread does the call */
+        const fom_int me = omp_get_thread_num(), nth = omp_get_num_threads();
+        if (nth == IT_NK) {
+            if (!it_seen[me].v) {
+                #pragma omp barrier
+                #pragma omp single
+                if (it_mode == 0) it_mode = it_setup();
+                it_seen[me].v = 1;
+            }
+            if (it_mode > 0) {
+                it_args A;
+                const int tuning = it_var < 0;
+                const unsigned ep = it_tep[me].v + 1;
+                double tt0 = 0;
+                if (tuning) {
+                    #pragma omp barrier
+                    tt0 = fom_now_ns();
+                }
+                it_args_set(&A, state, sc_gamma, bias_t0, bias_t1, bias_su, act_t0, act_t1, t_t0, t_t1, new_state);
+                it_tep[me].v = ep;
+                #ifdef FOM_PROFILE
+                #pragma omp master
+                it_tp = fom_now_ns();
+                #endif
+                it_body(&A, me, nth, ep);
+                #ifdef FOM_PROFILE
+                #pragma omp master
+                IT_PROF_MARK("it_ATt0_t1", it_tp);
+                #endif
+                if (tuning) {
+                    #pragma omp barrier
+                    #pragma omp master
+                    it_tune(A.var, tt0);
+                    #pragma omp barrier
+                }
+                return;
+            }
+        }
+        {
+            /* another team size: the same work with barriers, blocks
+               claimed from the shared counters */
+            it_args A;
+            int tuning;
+            unsigned ep;
+            double tt0 = 0;
+            #pragma omp barrier
+            #pragma omp single
+            {
+                if (it_mode == 0) it_mode = it_setup();
+                it_sh_ep = it_tep[0].v + 1;
+                it_cnt1 = 0;
+                it_cnt2 = 0;
+            }
+            if (it_mode < 0) {
+                #pragma omp master
+                it_iterate1(state, sc_gamma, bias_t0, bias_t1, bias_su, act_t0, act_t1, t_t0, t_t1, new_state);
+                #pragma omp barrier
+                return;
+            }
+            ep = it_sh_ep;
+            tuning = it_var < 0;
+            tt0 = fom_now_ns();
+            it_args_set(&A, state, sc_gamma, bias_t0, bias_t1, bias_su, act_t0, act_t1, t_t0, t_t1, new_state);
+            it_body(&A, me, nth, ep);
+            #pragma omp barrier
+            #pragma omp single
+            {
+                fom_int t;
+                for (t = 0; t < IT_NK; ++t) it_tep[t].v = it_fT[t].v = ep;
+                if (tuning) it_tune(A.var, tt0);
+            }
+            return;
+        }
+    }
+    #endif
+    it_iterate1(state, sc_gamma, bias_t0, bias_t1, bias_su, act_t0, act_t1, t_t0, t_t1, new_state);
+}
```

## Agent notes

```
### Round 1

**Whole-solve time:** 211,175 µs at the start → about 37,000 µs at the end (quiet-node runs ranged 35.8–38.3 ms). That is a **~5.7× speedup**. The node is shared and timings vary up to 2× with load, so baseline and final were compared back to back. Worst end-to-end error is 1.1e-9 (tolerance 1e-5). `make verify` passes with OpenMP, in the serial build, and at 1, 3, 7 and 20 threads.

**Final profile** (`make profile`, quiet node; 39.4 ms per solve inside `iterate`):
| region | time per solve | % of iterate |
|---|---|---|
| it_Ay_partial (y and the A·y partials per column block) | 18.1 ms | 45.8% |
| it_ATt0_t1_su (t1_t_t0·t0, t1 soft-threshold, state shift) | 18.0 ms | 45.7% |
| it_t0_reduce (sum of the 16 partials, t0 soft-threshold) | 3.3 ms | 8.4% |
| residual (last iteration only) | 0.02 ms | ~0% |

**Optimizations applied:**
1. **One gather over A instead of two members for t0.** `t0_state = [0, A]` and `t0_state__d_gamma = [−A, A]` (same pattern, values equal up to roundoff). So t0 = A·y with y = x + γ(x − xp), formed once per column. This halves the bytes read for t0. A setup check enforces pattern equality and a relative tolerance of 2⁻⁴⁰ on the values; if it fails, the generic step routines are used.
2. **Exact diagonal t1 terms.** `t1_state` and `t1_state__d_gamma` have one or two entries per row (values 1+ε, ±1). They are applied elementwise from their own values. Using exact ±1 instead had raised the error from 1e-9 to 1e-7.
3. **State update as copies, no extra buffer passes.** `su_state` and `su_t_t1` are shifts. `new_state` is written inside the t1 loop, the state buffers are swapped by pointer, and the residual is computed only after the last iteration (`FOM_USE_TOL` = 0).
4. **Column-block split with sliced ELLPACK (SELL) for A·y.** Each of 16 threads owns 2500 columns. Its slice of y is 20 KB, so the random gathers stay in L1. Inside a block, rows are sorted by entry count and packed 10 per slice. This gives predictable loop lengths, since short variable-length rows were costing branch mispredictions, and padding is only 0.09%. Column indices are 16-bit. Each block writes one partial per row, so nothing needs zeroing, and a row reduction then gives t0.
5. **Banded SELL for `t1_t_t0`·t0.** The member's own data is used, unchanged. The same thread owns the same columns, so y and the state stay local. The rows are split into 4 bands of t0 (20 KB each, L1-resident), packed 10 per slice after sorting by length; padding is 0.02%.
6. **Fused t1 step.** Soft-threshold, bias, the diagonal terms and both state-shift copies are done in one pass per column block. One parallel region per iteration with 2 barriers. Static block-to-thread mapping keeps each CCD's slice of the matrix in its own L3.
7. **Parameter term hoisted.** `t0_p_p0`·p0 is computed once per solve. `fom_iterate` builds its layouts once, lazily, from the matrix data and checks the structure first.

**Tried and dropped** (measured slower or no gain): fused single pass with per-thread scatter buffers (cross-CCD reduction too costly); in-L1 scatter for Aᵀt0 (store conflicts); fixed-width ELL; dynamic scheduling (about 2× slower because matrix data moves between CCD L3s); GCC 2-wide vector extensions; one persistent parallel region per solve; software prefetch and a chunk-major layout for the reduction.

Blocked by rules:
- Build the SELL layouts in `fom_setup` through a public init routine in `src/fom_ops.c`, instead of lazily on the first `fom_iterate` call | every function in fom_ops.c except the entry points must be static; fom_setup must not call fom_iterate | ~0 at steady state (only moves a one-time ~0.1 s setup out of the first solve)
- AVX2/AVX-512 gathers and FMA for the SELL kernels (`-march`) | build flags are fixed | unknown, likely small: Zen 4 gathers run about 1 element per cycle, close to the current scalar rate

### Round 2

**Whole-solve `mean_us`**
- Initial: 71732 us (given baseline, measured on an Intel Xeon Gold 6342 node).
- Final: 34314 us (`make bench`), measured on an AMD EPYC 7413 (Zen3) node after the bench node rotated partway through the round.
- Headline ratio: 2.09x. The two numbers come from different CPUs, so same-node comparisons are fairer:
  - Intel 6342: round-1 binary 67.5–69.3 ms vs the scatter variant 45.4–46.4 ms, about 1.5x. That was measured before the gather variant and the autotuner were added.
  - AMD 7413: round-1 binary 38.4 ms vs final 34.3–35.6 ms, about 1.1x.

**Final profile (AMD node, `make profile`, mean 35.1 ms)**

`iterate` is the outer region, so the four `it_*` regions sit inside it.

| region | total_us | pct |
|---|---|---|
| iterate | 1993607 | 50.03% |
| it_Ay_partial | 863022 | 21.66% |
| it_ATt0 | 797369 | 20.01% |
| it_t0_reduce | 198952 | 4.99% |
| it_t1_su | 130724 | 3.28% |
| residual | 1169 | 0.03% |

**Optimizations applied**
- **One copy of A for both products (transpose-scaled structure).** `t1_t_t0` equals c·Aᵀ, with the same sparsity pattern transposed and every entry equal to c·A within 2^-40. This is checked entry by entry at setup. c is computed without a division (Newton reciprocal). So `t1_t_t0·t0 = c·(Aᵀ t0)` runs as a scatter over the same per-thread A blocks used for `A y`. That halves the matrix bytes per iteration (52 MB to 26 MB), which removed nearly all last-level-cache misses on Intel.
- **Zig-zag slice order.** `A y` walks each thread's block forward and `Aᵀ t0` walks it backward. Each pass starts on the slices the other just left in L2: a 63 → 50.7 ms step on Intel.
- **Autotuned `Aᵀ t0` variant.** On AMD Zen3 (32 MB L3 per 6-core CCD) both layouts fit in cache. There the store-free gather over a banded SELL layout of `t1_t_t0` beats the scatter: 34.7 vs 46.8 ms. Both variants are exact. The first 192 fast calls time them in runs of 32, skipping the first 8 of each run and keeping each variant's best time; the faster one is then kept.
- **Packed 16-bit byte offsets.** Block-local columns (at most 2500 per block, and 8·2500 < 65536) are stored as byte offsets, two per 32-bit word. Index extraction drops to one `movzwl` plus one `shr` per pair: about 4%. An earlier 4-per-64-bit packing gave about 5% over the u16 layout.
- **Paired SIMD kernels.** Slices are 20 rows wide, with values aligned to 64 bytes. Gathers form pairs in 16-byte vectors so one `mulpd`/`addpd` covers two entries (about 3%). The scatter keeps the 20 t0 values in registers as pairs, and `restrict` stops the reloads caused by aliasing.
- **Chunked work stealing.** Each block is split into 10 chunks claimed through an atomic counter, owner first. A thread that runs out of work takes whole chunks of other blocks, and steals during the scatter go to private column sums merged in the t1 pass. This absorbs systematically slow shared cores. Large chunks matter: 25 or 50 chunks per block cost 2–4% by breaking L2 locality.

Tried and reverted (no gain or slower): rotating entries within a slice, a private copy of t0, scalar constant diagonals, sharing the bias arrays, fusing the t1 tail with per-block seq_cst done-counters, and slice width 16. A persistent parallel region was not attempted: fork/join costs only 3.1 us vs 0.9 us for a barrier, about 1% of the solve.

Blocked by rules:
- AVX2/AVX-512 kernels (FMA plus hardware gathers, which Ice Lake supports) through `-march` or `__attribute__((target))` | fixed build flags / portable embedded target (the build is SSE2-only) | unknown, likely 1.3–2x on both product kernels
- Building the SELL layouts in `fom_setup` with a parallel first-touch, instead of lazily in the first `fom_iterate` call | `fom_ops.c` may export only the listed entry points, and `fom_setup` may not call `fom_iterate`/`fom_step_*` | about 0 on the bench (setup falls in warmup), lower first-solve latency

### Round 3

**Whole-solve `mean_us`:** 34780.6 → 26607.4 (`make bench`), a 1.31x speedup this round and 8.08x against the translated solver (214959.3). The holder node changed mid-session (still an AMD EPYC 7413). An alternating A/B on that node gave 34225 → 26919 us (-21%).

**Final profile (`make profile`, mean_us 27006)**

`iterate` is the whole loop; the other regions are inside it. The share column is each region's part of `iterate`.

| Region | Total (µs) | Per iteration (µs) | Share of `iterate` |
|---|---|---|---|
| iterate (whole loop) | 2015670 | 134 | 100% |
| it_ATt0_t1 | 964020 | 64 | 47.8% |
| it_Ay_partial | 885088 | 59 | 43.9% |
| it_t0_reduce | 163165 | 11 | 8.1% |
| residual | 1623 | 22 per solve | 0.1% |

**Optimizations applied**

The profiling below showed the SELL gathers are bound by instruction and load throughput. Moving the data into L2 or shrinking the x footprint did not change their time, padding is under 0.2%, and the iterates are dense. So the wins came from communication, synchronization and load count.

1. **Whole column blocks, no work stealing.** The column blocks of A have equal entry counts (±0.2%), so thread K processes all of block K in both passes. Chunked stealing cost about 10%: claims contend on shared counters and blocks lose cache locality. This alone took 31.7 → 29.7 ms.
2. **t1 fused into the A^T pass, y fused into the A·y pass.** With whole-block ownership, the thread of block K has all of (A^T t0)_K. It finishes t1_K and the next state right away, which removes the column-sum scratch arrays, their merge and one barrier. y_K is local to the thread for the same reason, which removes the y barrier. Gain about 5%.
3. **Two-level t0 reduce.** The 16 partial vectors of A·y are 1.28 MB per iteration, and moving them between CCDs was the reduce's cost. Groups of 4 consecutive threads (usually in one L3) first sum their partials in place, each thread on a quarter of the rows. Then each row sums 4 group partials instead of 16. Gain about 4%.
4. **Point-to-point epoch flags instead of barriers** (release/acquire, padded to a cache line, used when there are 16 threads):
   - level 1 waits only for its group;
   - level 2 waits for one thread per group;
   - A^T band b waits only for the 4 threads that reduced band b, starting with its own group's band.

   Gain about 2.5%.
5. **Persistent thread team.** fom_solve keeps one team for all iterations and every thread calls fom_iterate, so there is no fork/join per iteration (measured 2.9 µs). The flags make the dependencies across iterations explicit: level 2 overwrites t0 only after every thread has finished the previous call's A^T pass. Gain about 2.6%.
6. **Four 16-bit column offsets per 64-bit load.** This cuts index loads in the load-port-bound gather, worth about 1–1.5%. 16-bit row permutations were neutral; I kept them because they are smaller.

**Blocked by rules:**
- AVX2/FMA versions of the SELL gather and scatter kernels via `__attribute__((target("avx2,fma")))` with runtime CPU dispatch (4-wide value loads, fused multiply-add) | the build flags are fixed (SSE2 baseline), and forcing a wider instruction set from the source would get around them | the kernels are bound by load and instruction count and take about 85% of the time, so I estimate 15–30% on them; not measured

### Round 4

**Result: no change this round.** The sources are the round-3 solver and still pass `make verify`. Seven changes were tried; none helped on either node type.

- Initial: 26917.7 µs (round-3 figure, AMD EPYC 7413); 214959.3 µs for the translated solver.
- Final: same code. 26728 µs on AMD at the start of this session; 39529 µs after the holder moved to an Intel Xeon Gold 6342 (della-l01g13). The difference is the node, not the code.
- Speedup over the translation: 8.0x on AMD (26.7 ms), 5.4x on Intel (39.5 ms).

**Final profile** (`make profile`, Intel node; `iterate` encloses the three `it_*` regions):

| Region | Time (µs) | % |
|---|---|---|
| iterate | 2009498 | 50.0 |
| it_ATt0_t1 | 1147079 | 28.6 |
| it_Ay_partial | 791983 | 19.7 |
| it_t0_reduce | 68892 | 1.7 |
| residual | 513 | 0.0 |

On AMD the same split was Ay 42%, Aᵀt0 47%, reduce plus waits about 10%.

**What the analysis showed (no code kept):**
- **Gather kernel limit:** with the fixed `-O3` flags (SSE2 only), the gather kernel tops out at about 0.86 cycles/entry when its data fits in L2, limited by two FP loads per cycle. In the solver it runs at about 1.18 on AMD.
  - Giving it L2-resident data did not speed it up, so it is compute-bound, not starved for data.
  - `perf` shows about 17% of cycles stalled on a full load queue and about 7% in spin waits.
- **Permuted partial-sum stores:** writing the Ay partial sums in permuted row order costs about 13% of the Ay pass. Removing them is not free, though: every working version just moved the cost elsewhere (see below).
- **Intel:** the autotuner rightly picks the scatter variant there, because the two layouts (52 MB) do not fit in cache. Its speed comes from L2 reuse: the Aᵀ pass walks backwards over what the Ay pass just read.

**Tried and rejected** (all A/B tested on the same node):
- **Un-permuting the Ay partials:** either inside the Ay thread through a small L1 buffer, or later in the level-1 reduction via an inverse permutation (rows sorted within each 2500-row slot).
  - The Ay pass got 8% faster, but level 1 got 2.5x slower, and prefetching the partial regions did not fix that.
  - Net effect on both CPUs: neutral.
- **Software prefetch** of the value and index streams, at 320 to 2048 bytes ahead and several locality hints: 3-5% slower on AMD, neutral on Intel.
- **Loading the next step's index words one iteration early:** 4% slower.
- **First t0 band stores instead of adding to a zeroed buffer:** neutral.
- **Hybrid Aᵀt0** (gather some t0 bands from the transposed layout, scatter the rest), with the autotuner picking how many: every variant verified, but on Intel even one gathered band was 12% slower because it breaks the L2 reuse; on AMD the full gather already wins.
- **The scatter variant on AMD:** confirmed 34 ms against 27 ms for the gather, so the autotuner's choice is right.

Blocked by rules:
- Compile the gather and scatter kernels with AVX2/FMA via `__attribute__((target))` plus a runtime CPU check, giving 256-bit value loads (fewer FP loads per entry) and FMA | build flags are fixed (`-O3`, no `-march`), and per-function target attributes would amount to changing them, which also hurts portability on the embedded target | unknown, roughly 10-20% on the gather passes
- Inline-asm offset extraction (in-place `shr`/`movzwl`) to drop the register copies GCC adds | x86-specific asm conflicts with portable embedded C under fixed flags | unknown, under 5%
- Choose the reduction groups from the actual CPU-to-L3 topology (on AMD one group of four threads spans two L3 caches) | needs OS topology queries (`sched_getcpu`, `/sys`) that don't belong in an allocation-free embedded solver | about 1%
```
