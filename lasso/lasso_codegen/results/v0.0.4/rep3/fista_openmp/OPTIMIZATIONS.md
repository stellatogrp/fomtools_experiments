# Optimization report

## Timing

```
translated (emitted)  171117.7 us/solve
optimized              51121.6 us/solve
speedup                  3.35x
```

Optimization outcome: **kept (re-verified correct)**.

## Rounds

| round | before (us) | after (us) | gain | kept | agent (s) | outcome |
|---:|---:|---:|---:|:---:|---:|---|
| 1 | 171117.745 | 50057.636 | 70.7% | yes | 3608 | continued |
| 2 | 50524.469 | 31515.057 | 37.6% | yes | 2759 | continued |
| 3 | 57639.801 | 47159.465 | 18.2% | yes | 2674 | continued |
| 4 | 52308.040 | 51121.627 | 2.3% | yes | 1771 | continued |
| 5 | - | - | - | no | 2040 | the round left the sources unchanged |

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
| 1 | AVX2/AVX-512 (FMA, wider vectors, `vgatherdpd`) through `-march` or `__attribute__((target))`; the kernels are latency/throughput-bound at about 2 cycles per nonzero with SSE2 only | build flags are fixed | unknown, likely 10-20% |
| 1 | Computing y for the next iteration inside the current t1 pass (removes the y pass and a barrier) | the `fom_iterate` signature in `include/fom_ops.h` is fixed and does not receive the next gamma | ~6% (the `gather_y` region) |
| 1 | Documenting the NULL-means-zero bias convention in the header | headers are fixed | none (documentation only; it is commented in `src/fom_ops.c` and `src/fom_solver.c`) |
| 2 | Fold y_{k+1} = t1 + g_{k+1}(t1 − s1) into iteration k's B pass (removes the y phase and its barrier) | no schedule reads in fom_ops.c / fixed fom_iterate signature (it only receives the current gamma) | ~3-4% (y phase 6 us/iter plus its barrier) |
| 2 | Compile with -march=native (AVX2/FMA in all loops, including auto-vectorized ones) | Makefile/build flags fixed (worked around with a target attribute and runtime dispatch for the hot kernel) | <2% |
| 3 | Store A and B values in fewer bytes (e.g. a float plus a 16-bit correction, about 2^-40 relative error, 6 B instead of 8 B per value) to cut L3 traffic in the bandwidth-bound passes | a kernel must apply exactly M (lossy coefficients); the 1e-10 ops tolerance would probably accept it, but it changes the matrix | ~15-25% on the two main passes |
| 3 | Hoist y = s1 + g'(s1 - s0) for the next iteration into the B tail, to drop the y pass and one barrier | `src/fom_ops.c` may not read the schedule, and `fom_iterate` only receives the current gamma | ~2-4% |
| 3 | Slide the state through the contiguous `state`/`new_state`/`acc` workspace so the shift copy new_s0 = s1 becomes a no-op | fixed workspace layout (headers not editable); pointer arithmetic across struct members is not portable | ~1% |
| 4 | Store the values of A/B in compressed form (48-bit truncated or float32+correction mantissas; 6 instead of 8 bytes per value) to cut L3/DRAM traffic about 25–40% | the `make verify` ops tolerance of 1e-10 relative and the exactness requirement on `fom_iterate` (values are random and not float32-exact) | about 20–30% on bandwidth-bound nodes |
| 4 | Keep A·s0 from the previous iteration (A·y = (1+g)·A·s1 − g·A·s0, where s0 is the previous s1) to drop one third of the A work and the y pass | `fom_iterate` must be right for arbitrary input states; checking that the cache matches costs as much as recomputing it | about 5–10% (unknown) |
| 4 | Compute next iteration's y during the B phase to remove the y pass and its barrier | `src/fom_ops.c` may not read the schedule, and `fom_iterate` gets only the current gamma | about 3–4% |
| 5 | Fuse the next iteration's y = s1 + g'(s1 - s0) into the current B pass, removing the gather_y phase and its barrier | `fom_iterate` gets only this iteration's gamma, and `src/fom_ops.c` may not read the schedule (`fom_sched_*`, `FOM_SCHED_*`) | about 4% (gather_y is 1.2 of 28.7 ms on Zen 3) |
| 5 | Store B's values in float32 (or another lossy form) to halve the bytes streamed per nonzero | the `fom_apply`/`fom_iterate` checks require exactness to 1e-10 relative, which float32 values cannot meet | unknown, possibly 20–30% since the passes look bandwidth-limited |

## What the agent changed

```diff
--- src/fom_solver.c (translated)
+++ src/fom_solver.c (optimized)
@@ -1,10 +1,59 @@
+/* getcpu (glibc 2.29+): the NUMA node a thread runs on */
+#if defined(__linux__) && !defined(_GNU_SOURCE)
+#define _GNU_SOURCE
+#endif
 #include <string.h>
 #include "fom_solver.h"
 #include "fom_data.h"
 #include "fom_ops.h"
 #include "fom_linsolve.h"
+#include "fom_profile.h"
+#ifdef _OPENMP
+#include <omp.h>
+#endif
+#ifdef __linux__
+#include <sched.h>
+#endif
 
 #define NH FOM_T1_DIM /* half of the state: s0 | s1 */
+
+/* Team size of the solve. The iteration is bound by cache traffic and by
+   one barrier per phase, and each barrier waits for its slowest thread: a
+   few threads placed on another NUMA node (another socket's caches and
+   memory) cost more than they add. When the team's threads past some
+   point all sit on other nodes than the master's and are at most a
+   quarter of the team, the solve runs on the leading threads only (with
+   OMP_PROC_BIND set, a smaller team keeps the leading places). */
+#define SV_MAXT 256
+static int sv_nt = 0; /* 0: the default team */
+
+static int sv_node(void) {
+#if defined(__linux__) && defined(__GLIBC__) && (__GLIBC__ > 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 29))
+    unsigned int cpu, node;
+    if (getcpu(&cpu, &node) == 0) return (int)node;
+#endif
+    return -1;
+}
+
+static void sv_pick_team(void) {
+#ifdef _OPENMP
+    static int nodes[SV_MAXT];
+    int T = 1, t, lead;
+    if (omp_in_parallel() || omp_get_proc_bind() == omp_proc_bind_false) return;
+    #pragma omp parallel
+    {
+        const int id = omp_get_thread_num();
+        #pragma omp single
+        T = omp_get_num_threads();
+        if (id < SV_MAXT) nodes[id] = sv_node();
+    }
+    if (T > SV_MAXT || nodes[0] < 0) return;
+    for (lead = 1; lead < T && nodes[lead] == nodes[0]; ++lead) {}
+    for (t = lead; t < T; ++t)
+        if (nodes[t] == nodes[0]) return; /* not a trailing block */
+    if (lead < T && 4 * (T - lead) <= T) sv_nt = lead;
+#endif
+}
 
 void fom_set_default_settings(FomSettings *settings) {
     settings->max_iter = FOM_MAX_ITER;
@@ -14,6 +63,7 @@
 
 void fom_setup(FomWorkspace *work, const FomSettings *settings) {
     work->settings = *settings;
+    if (sv_nt == 0) sv_pick_team();
     memcpy(work->state, FOM_STATE_INIT, sizeof(work->state));
     work->solution.iter = 0;
     work->solution.residual = 0.0;
@@ -26,50 +76,74 @@
 }
 
 fom_int fom_solve(FomWorkspace *work) {
-    fom_float *state = work->state;
     fom_float *bias_t0 = work->acc;               /* t0_p_p0 @ p0, hoisted */
-    fom_float *bias_t1 = work->acc + FOM_T0_DIM;  /* zero bias */
     fom_float *t0 = work->t0, *t1 = work->t1;
+    /* ping-pong the state between work->state and solution.x: no copies */
+    fom_float *cur = work->state, *nxt = work->solution.x;
     const fom_int max_iter = work->settings.max_iter;
     const fom_int use_tol = FOM_USE_TOL && work->settings.check_termination;
     const fom_float tol = work->settings.tol;
     fom_float res = 0.0;
     fom_int it, i;
 
+    FOM_PROF_BEGIN(bias);
     memset(bias_t0, 0, FOM_T0_DIM * sizeof(fom_float));
     fom_apply_t0_p_p0(work->params.p0, bias_t0);
-    memset(bias_t1, 0, FOM_T1_DIM * sizeof(fom_float));
+    FOM_PROF_END(bias);
 
     work->solution.status = FOM_MAX_ITER_REACHED;
-    for (it = 0; it < max_iter; ++it) {
-        const fom_float g = fom_sched_gamma(it);
-        fom_step_t0(state, g, bias_t0, &FOM_ACT_t0, t0);
-        fom_step_t1(state, t0, g, bias_t1, &FOM_ACT_t1, t1);
-        /* su: new s0 = old s1, new s1 = t1 */
-        if (use_tol || it == max_iter - 1) {
-            fom_float m = 0.0;
-            #ifdef _OPENMP
-            #pragma omp parallel for reduction(max:m)
-            #endif
-            for (i = 0; i < NH; ++i) {
-                fom_float a = state[NH + i] - state[i];
-                fom_float b = t1[i] - state[NH + i];
-                a = a < 0 ? -a : a;
-                b = b < 0 ? -b : b;
-                a = a > b ? a : b;
-                if (a > m) m = a;
+    it = 0;
+    /* one parallel region over the whole loop: fom_iterate, called by every
+       thread, runs each iteration on this team (no fork/join per iteration)
+       and returns after a barrier */
+    #ifdef _OPENMP
+    #pragma omp parallel private(i) num_threads(sv_nt > 0 ? sv_nt : omp_get_max_threads())
+    #endif
+    {
+        fom_float *c = cur, *n = nxt, *sw;
+        fom_int k;
+        int master = 1;
+        #ifdef _OPENMP
+        master = omp_get_thread_num() == 0;
+        #endif
+        for (k = 0; k < max_iter; ++k) {
+            const fom_float g = fom_sched_gamma(k);
+            FOM_PROF_BEGIN(iterate);
+            /* t1 and su have no constant or parameter terms: their biases
+               are zero, passed as NULL so fom_iterate does not stream them */
+            fom_iterate(c, g, bias_t0, NULL, NULL, &FOM_ACT_t0, &FOM_ACT_t1, t0, t1, n);
+            if (master) { FOM_PROF_END(iterate); }
+            if (use_tol || k == max_iter - 1) {
+                FOM_PROF_BEGIN(residual);
+                #ifdef _OPENMP
+                #pragma omp single
+                #endif
+                res = 0.0;
+                #ifdef _OPENMP
+                #pragma omp for reduction(max:res)
+                #endif
+                for (i = 0; i < FOM_STATE_DIM; ++i) {
+                    fom_float a = n[i] - c[i];
+                    a = a < 0 ? -a : a;
+                    if (a > res) res = a;
+                }
+                if (master) { FOM_PROF_END(residual); }
             }
-            res = m;
+            sw = c; c = n; n = sw;
+            if (use_tol && res <= tol) { ++k; break; }
         }
-        memcpy(state, state + NH, NH * sizeof(fom_float));
-        memcpy(state + NH, t1, NH * sizeof(fom_float));
-        if (use_tol && res <= tol) {
-            work->solution.status = FOM_SOLVED;
-            ++it;
-            break;
+        if (master) {
+            cur = c;
+            it = k;
+            if (use_tol && it > 0 && res <= tol) work->solution.status = FOM_SOLVED;
         }
     }
-    memcpy(work->solution.x, state, sizeof(work->solution.x));
+    /* the final state lives in both work->state (a later solve continues
+       from it, as in the plain loop) and solution.x */
+    if (cur != work->state)
+        memcpy(work->state, cur, sizeof(work->state));
+    else
+        memcpy(work->solution.x, cur, sizeof(work->solution.x));
     work->solution.iter = it;
     work->solution.residual = res;
     return work->solution.status;
--- src/fom_ops.c (translated)
+++ src/fom_ops.c (optimized)
@@ -1,5 +1,21 @@
 #include "fom_ops.h"
 #include "fom_data.h"
+#include "fom_profile.h"
+#include <stdint.h>
+#include <string.h>
+#include <math.h>
+#ifdef _OPENMP
+#include <omp.h>
+#endif
+#ifdef __SSE2__
+#include <emmintrin.h>
+#include <xmmintrin.h>
+#endif
+/* x86-64 GCC/Clang: an AVX2 + FMA kernel, picked at run time by CPU */
+#if defined(__x86_64__) && defined(__GNUC__) && defined(__SSE2__)
+#include <immintrin.h>
+#define IT_HAVE_AVX2 1
+#endif
 
 void fom_apply_t0_state(const fom_float *x, fom_float *out) {
     fom_int i, p;
@@ -210,8 +226,867 @@
     }
 }
 
+/* ---- fused iteration (FISTA on lasso) ----
+   Structure exploited, checked once from the matrix data (else the generic
+   step routines run):
+   - t0_state = [0 | A] and t0_state__d_gamma = [-A | A] (same pattern, values
+     equal to rounding), so t0 = act(bias + A y) with y = s1 + g (s1 - s0):
+     one product with A instead of three;
+   - t1_t_t0 = B = c A^T (same pattern, values equal to rounding);
+   - t1_state = [0 | u I], t1_state__d_gamma = [w0 I | w1 I] (u, w0, w1 scalars);
+   - su is a pure shift: new state = [s1 | t1].
+   Column indices of A and B fit in 16 bits: compact copies cut traffic.
+   Three exact variants (it_run_gather with or without a streamed copy of
+   t0, it_run_bonly), picked by timing.
+   The gather variant runs A and B in SELL-4 layouts (4 rows per SIMD
+   slice), column-blocked so the gathered source stays in L1, A in
+   row x column tiles so each slice of y is shared by few threads. Each
+   thread's tile and window of B rows is split into IT_SPLIT units that
+   idle threads take over (work stealing): a thread on a busy core or on
+   another socket no longer sets the pace of every barrier.
+   Called inside a parallel region, fom_iterate runs on the caller's team
+   (it_iterate_team): no fork or join per iteration. */
+#define IT_NH 40000
+#define IT_M 10000
+#define IT_PFD 256
+#define IT_MAXT 64
+#define IT_TUNE 16
+static unsigned short it_aidx[FOM_MAT_t0_state_NNZ];
+static unsigned short it_bidx[FOM_MAT_t1_t_t0_NNZ];
+static fom_float it_acc[IT_MAXT][IT_M];
+/* SELL-4 layouts of A (gather from y) and B (gather from t0) for the
+   gather variant: columns split into blocks (one block of the source stays
+   in L1/L2 while a pass runs over it); the rows in one window (one thread's
+   chunk) sorted by their length in the block and packed 4 at a time, entry
+   l of the 4 rows interleaved, padded with zeros to the longest. 4 rows run
+   in SIMD lanes: no per-row loop exit or horizontal sum. */
+#define IT_SA_NBMAX 64
+#define IT_BWMAX 3400
+#define IT_SB_NB 4
+/* each thread's A tile and window of B rows split into IT_SPLIT row units:
+   a thread runs its own units first, then takes the units other threads
+   have not started (a thread slowed by a busy core or a remote socket
+   no longer holds up the barrier) */
+#define IT_SPLIT 4
+#define IT_MAXW (IT_SPLIT * IT_MAXT + 2)
+#define IT_SA_SL (IT_SA_NBMAX * (IT_M / 4 + IT_MAXW))
+#define IT_SB_SL (IT_SB_NB * (IT_NH / 4 + IT_MAXW))
+#define IT_SA_CAP (FOM_MAT_t0_state_NNZ + FOM_MAT_t0_state_NNZ / 8 + 8 * IT_SA_SL)
+#define IT_SB_CAP (FOM_MAT_t1_t_t0_NNZ + FOM_MAT_t1_t_t0_NNZ / 8 + 16 * IT_SB_SL)
+typedef struct {
+    fom_float *v;         /* 4 values per group */
+    unsigned short *ix;   /* 4 source indices per group */
+    fom_int *g;           /* first group of each slice (+ end) */
+    fom_int *r;           /* 4 output rows per slice (nrows: a dummy) */
+    fom_int *wb;          /* first slice of (block b, window w): wb[b * IT_MAXW + w] */
+    fom_int nb, bw, nw, w, nrows, cap, maxsl;
+} ItSell;
+static fom_float it_sa_v[IT_SA_CAP];
+static unsigned short it_sa_ix[IT_SA_CAP];
+static fom_int it_sa_g[IT_SA_SL + 1], it_sa_r[4 * IT_SA_SL], it_sa_wb[IT_SA_NBMAX * IT_MAXW];
+static fom_float it_sb_v[IT_SB_CAP];
+static unsigned short it_sb_ix[IT_SB_CAP];
+static fom_int it_sb_g[IT_SB_SL + 1], it_sb_r[4 * IT_SB_SL], it_sb_wb[IT_SB_NB * IT_MAXW];
+static ItSell it_sa = {it_sa_v, it_sa_ix, it_sa_g, it_sa_r, it_sa_wb, 0, 0, 0, 0, IT_M, IT_SA_CAP, IT_SA_SL};
+static ItSell it_sb = {it_sb_v, it_sb_ix, it_sb_g, it_sb_r, it_sb_wb, IT_SB_NB, 0, 0, 0, IT_NH, IT_SB_CAP, IT_SB_SL};
+/* A in tiles: IT_R row windows x IT_C column groups (IT_R IT_C = threads),
+   each group IT_SUB column blocks; tile (r, c) adds the products of its
+   rows into it_pg[c], whose IT_C partials then sum to A y. Each slice of y
+   is read by the IT_R threads of its group only, and the partials cost a
+   reduction over IT_C vectors instead of IT_C IT_R. */
+static fom_int it_R = 1, it_C = 1, it_sub = 1;
+#ifndef IT_RMAX
+#define IT_RMAX 4
+#endif
+/* owner w = tile (row window it_wr[w], column group it_wc[w]) */
+static fom_int it_wr[IT_MAXT], it_wc[IT_MAXT];
+/* units taken so far from each owner's list (A tiles, B windows); it_nob:
+   owners of B windows */
+static struct { fom_int n; char pad[64 - sizeof(fom_int)]; } it_cnt_a[IT_MAXT], it_cnt_b[IT_MAXT];
+static fom_int it_nob = 1;
+static fom_float it_pg[IT_MAXT][IT_M + 4] __attribute__((aligned(64)));
+static fom_float it_partb[IT_NH + 1];
+static fom_int it_wrow[IT_NH + 4], it_wlen[IT_NH + 4];
+/* y, read by the threads of a column group, and (variant 3) a copy of t0,
+   read by every thread: written with streaming stores, which skip the
+   read-for-ownership of lines the other cores hold. Variant 1 gathers t0
+   straight from t_t0 instead: no store fence and no trip through memory,
+   faster when every thread shares one socket's caches, slower when a thread
+   sits on another socket. */
+static fom_float it_y[IT_NH] __attribute__((aligned(64)));
+static fom_float it_t0n[IT_M] __attribute__((aligned(64)));
+static fom_int it_pos[IT_NH];
+static fom_float it_c, it_rc; /* t1_t_t0 = it_c * A^T, it_rc = 1 / it_c */
+static int it_mode = 0; /* 0 unchecked, 1 fast, -1 generic */
+static int it_avx2 = 0; /* the CPU runs AVX2 + FMA (x86-64 GCC/Clang builds) */
+
+/* threads the iteration runs on: the enclosing team when fom_iterate is
+   called inside a parallel region, else a new region's */
+static int it_nthreads(void) {
+#ifdef _OPENMP
+    return omp_in_parallel() ? omp_get_num_threads() : omp_get_max_threads();
+#else
+    return 1;
+#endif
+}
+
+/* 1/a by Newton's iteration from a power-of-two guess: division-free */
+static fom_float it_recip(fom_float a) {
+    fom_float b = a < 0 ? -a : a, r;
+    int e, k;
+    (void)frexp(b, &e);
+    r = ldexp(1.0, -e);
+    for (k = 0; k < 8; ++k) r = r * (2.0 - b * r);
+    return a < 0 ? -r : r;
+}
+
+/* Pack the CSR matrix (rowptr, 16-bit cols, vals; ncols columns) into S.
+   Windows of S->w rows (a multiple of 4, about one per thread); returns -1
+   if the static storage is too small. */
+static int it_sell_build(ItSell *S, const fom_int *rowptr, const unsigned short *col, const fom_float *val, fom_int ncols) {
+    fom_int T = 1, b, w, r0, r1, i, k, l, n = 0, s = 0, maxlen, len;
+    T = it_nthreads();
+    if (T > IT_MAXT) T = IT_MAXT;
+    S->bw = 4;
+    while (S->bw * S->nb < ncols) S->bw += 4;
+    S->w = 4;
+    while (S->w * T * IT_SPLIT < S->nrows) S->w += 4;
+    it_nob = T;
+    if (S == &it_sa) { /* tiles: it_R row windows of IT_SPLIT units */
+        S->w = 4;
+        while (S->w * it_R * IT_SPLIT < S->nrows) S->w += 4;
+    }
+    S->nw = 0;
+    for (r0 = 0; r0 < S->nrows; r0 += S->w) S->nw++;
+    if (S->nw + 1 > IT_MAXW) return -1;
+    for (b = 0; b < S->nb; ++b) {
+        const fom_int clo = b * S->bw, chi = clo + S->bw;
+        for (w = 0, r0 = 0; w < S->nw; ++w, r0 += S->w) {
+            fom_int m = 0;
+            r1 = r0 + S->w < S->nrows ? r0 + S->w : S->nrows;
+            S->wb[b * IT_MAXW + w] = s;
+            /* rows of the window, longest first (bucket by length) */
+            maxlen = 0;
+            for (i = r0; i < r1; ++i) {
+                len = 0;
+                for (k = rowptr[i]; k < rowptr[i + 1]; ++k) len += col[k] >= clo && col[k] < chi;
+                it_wlen[i - r0] = len;
+                if (len > maxlen) maxlen = len;
+            }
+            for (len = maxlen; len >= 0; --len)
+                for (i = r0; i < r1; ++i)
+                    if (it_wlen[i - r0] == len) it_wrow[m++] = i;
+            while (m & 3) it_wrow[m++] = S->nrows; /* dummy rows, length 0 */
+            for (i = 0; i < m; i += 4) {
+                fom_int L = 0, lane;
+                if (s >= S->maxsl) return -1;
+                for (lane = 0; lane < 4; ++lane) {
+                    fom_int rr = it_wrow[i + lane];
+                    S->r[4 * s + lane] = rr;
+                    if (rr < S->nrows) {
+                        len = 0;
+                        for (k = rowptr[rr]; k < rowptr[rr + 1]; ++k) len += col[k] >= clo && col[k] < chi;
+                        if (len > L) L = len;
+                    }
+                }
+                if (4 * (n + L) > S->cap) return -1;
+                S->g[s] = n;
+                for (lane = 0; lane < 4; ++lane) {
+                    fom_int rr = it_wrow[i + lane], pad = clo;
+                    l = 0;
+                    if (rr < S->nrows)
+                        for (k = rowptr[rr]; k < rowptr[rr + 1]; ++k)
+                            if (col[k] >= clo && col[k] < chi) {
+                                S->v[4 * (n + l) + lane] = val[k];
+                                S->ix[4 * (n + l) + lane] = col[k];
+                                pad = col[k];
+                                ++l;
+                            }
+                    /* zeros at an index of the same row */
+                    for (; l < L; ++l) {
+                        S->v[4 * (n + l) + lane] = 0.0;
+                        S->ix[4 * (n + l) + lane] = (unsigned short)pad;
+                    }
+                }
+                n += L;
+                ++s;
+            }
+        }
+        S->wb[b * IT_MAXW + S->nw] = s;
+    }
+    S->g[s] = n;
+    return 0;
+}
+
+static int it_check(void) {
+    fom_int i, j, p, n, q, pmax = 0, imax = 0;
+    fom_float m = 0.0, tol, b = 0.0, tolb;
+    for (p = 0; p < FOM_MAT_t0_state_NNZ; ++p) {
+        fom_float a = FOM_MAT_t0_state_VALS[p];
+        a = a < 0 ? -a : a;
+        if (a > m) { m = a; pmax = p; }
+    }
+    if (!(m > 0.0)) return -1;
+    tol = m;
+    for (i = 0; i < 40; ++i) tol *= 0.5;
+    if (FOM_MAT_t0_state_ROWPTR[0] != 0 || FOM_MAT_t0_state__d_gamma_ROWPTR[0] != 0) return -1;
+    for (i = 0; i < IT_M; ++i) {
+        p = FOM_MAT_t0_state_ROWPTR[i];
+        n = FOM_MAT_t0_state_ROWPTR[i + 1] - p;
+        q = FOM_MAT_t0_state__d_gamma_ROWPTR[i];
+        if (n < 0 || FOM_MAT_t0_state__d_gamma_ROWPTR[i + 1] - q != 2 * n) return -1;
+        if (pmax >= p && pmax < p + n) imax = i;
+        for (j = 0; j < n; ++j) {
+            fom_int c = FOM_MAT_t0_state_COLIDX[p + j];
+            fom_float v = FOM_MAT_t0_state_VALS[p + j], d0, d1;
+            if (c < IT_NH || c >= 2 * IT_NH) return -1;
+            if (FOM_MAT_t0_state__d_gamma_COLIDX[q + j] != c - IT_NH) return -1;
+            if (FOM_MAT_t0_state__d_gamma_COLIDX[q + n + j] != c) return -1;
+            d0 = FOM_MAT_t0_state__d_gamma_VALS[q + j] + v;
+            d1 = FOM_MAT_t0_state__d_gamma_VALS[q + n + j] - v;
+            d0 = d0 < 0 ? -d0 : d0;
+            d1 = d1 < 0 ? -d1 : d1;
+            if (d0 > tol || d1 > tol) return -1;
+            it_aidx[p + j] = (unsigned short)(c - IT_NH);
+        }
+    }
+    for (j = 0; j < IT_NH; ++j) {
+        if (FOM_MAT_t1_state_ROWPTR[j] != j || FOM_MAT_t1_state_COLIDX[j] != IT_NH + j) return -1;
+        if (FOM_MAT_t1_state_VALS[j] != FOM_MAT_t1_state_VALS[0]) return -1;
+        if (FOM_MAT_t1_state__d_gamma_ROWPTR[j] != 2 * j) return -1;
+        if (FOM_MAT_t1_state__d_gamma_COLIDX[2 * j] != j || FOM_MAT_t1_state__d_gamma_COLIDX[2 * j + 1] != IT_NH + j) return -1;
+        if (FOM_MAT_t1_state__d_gamma_VALS[2 * j] != FOM_MAT_t1_state__d_gamma_VALS[0]) return -1;
+        if (FOM_MAT_t1_state__d_gamma_VALS[2 * j + 1] != FOM_MAT_t1_state__d_gamma_VALS[1]) return -1;
+        if (FOM_MAT_su_state_ROWPTR[j] != j || FOM_MAT_su_state_COLIDX[j] != IT_NH + j || FOM_MAT_su_state_VALS[j] != 1.0) return -1;
+        if (FOM_MAT_su_t_t1_ROWPTR[j] != 0 || FOM_MAT_su_t_t1_ROWPTR[IT_NH + j] != j) return -1;
+        if (FOM_MAT_su_t_t1_COLIDX[j] != j || FOM_MAT_su_t_t1_VALS[j] != 1.0) return -1;
+        if (FOM_MAT_su_state_ROWPTR[IT_NH + j] != IT_NH) return -1;
+    }
+    if (FOM_MAT_t1_state_ROWPTR[IT_NH] != IT_NH || FOM_MAT_t1_state__d_gamma_ROWPTR[IT_NH] != 2 * IT_NH) return -1;
+    if (FOM_MAT_su_state_ROWPTR[2 * IT_NH] != IT_NH || FOM_MAT_su_t_t1_ROWPTR[2 * IT_NH] != IT_NH) return -1;
+    /* t1_t_t0 = c A^T: c from the largest entry of A, then every entry checked */
+    if (FOM_MAT_t1_t_t0_ROWPTR[0] != 0) return -1;
+    j = FOM_MAT_t0_state_COLIDX[pmax] - IT_NH;
+    for (q = FOM_MAT_t1_t_t0_ROWPTR[j]; q < FOM_MAT_t1_t_t0_ROWPTR[j + 1]; ++q)
+        if (FOM_MAT_t1_t_t0_COLIDX[q] == imax) b = FOM_MAT_t1_t_t0_VALS[q];
+    it_c = b * it_recip(FOM_MAT_t0_state_VALS[pmax]);
+    tolb = (it_c < 0 ? -it_c : it_c) * tol;
+    for (j = 0; j < IT_NH; ++j) it_pos[j] = FOM_MAT_t1_t_t0_ROWPTR[j];
+    for (i = 0; i < IT_M; ++i) {
+        for (p = FOM_MAT_t0_state_ROWPTR[i]; p < FOM_MAT_t0_state_ROWPTR[i + 1]; ++p) {
+            fom_float d;
+            j = it_aidx[p];
+            q = it_pos[j]++;
+            if (q >= FOM_MAT_t1_t_t0_ROWPTR[j + 1] || FOM_MAT_t1_t_t0_COLIDX[q] != i) return -1;
+            d = FOM_MAT_t1_t_t0_VALS[q] - it_c * FOM_MAT_t0_state_VALS[p];
+            d = d < 0 ? -d : d;
+            if (d > tolb) return -1;
+        }
+    }
+    for (j = 0; j < IT_NH; ++j)
+        if (it_pos[j] != FOM_MAT_t1_t_t0_ROWPTR[j + 1]) return -1;
+    for (q = 0; q < FOM_MAT_t1_t_t0_NNZ; ++q) it_bidx[q] = (unsigned short)FOM_MAT_t1_t_t0_COLIDX[q];
+    /* tiles for T threads: it_R row windows (the largest divisor of T up
+       to IT_RMAX: 3 x 5 tiles beat 1 x 15 for 15 threads), it_C = T / it_R
+       column groups of it_sub blocks of at most IT_BWMAX columns */
+    {
+        fom_int T = it_nthreads(), r, c, w;
+        if (T > IT_MAXT) T = IT_MAXT;
+        it_R = 1;
+        for (r = 2; r <= IT_RMAX && r <= T; ++r) {
+            for (c = r; c < T; c += r) {}
+            if (c == T) it_R = r;
+        }
+        for (it_C = 0, c = 0; c < T; c += it_R) ++it_C;
+        for (w = 0, c = 0; c < it_C; ++c)
+            for (r = 0; r < it_R; ++r, ++w) { it_wr[w] = r; it_wc[w] = c; }
+        it_sub = 1;
+        while (it_C * it_sub * IT_BWMAX < IT_NH) ++it_sub;
+        if (it_C * it_sub > IT_SA_NBMAX) return -1;
+        it_sa.nb = it_C * it_sub;
+    }
+    if (it_sell_build(&it_sa, FOM_MAT_t0_state_ROWPTR, it_aidx, FOM_MAT_t0_state_VALS, IT_NH) < 0) return -1;
+    if (it_sell_build(&it_sb, FOM_MAT_t1_t_t0_ROWPTR, it_bidx, FOM_MAT_t1_t_t0_VALS, IT_M) < 0) return -1;
+    it_rc = it_recip(it_c);
+#ifdef IT_HAVE_AVX2
+    __builtin_cpu_init();
+    it_avx2 = __builtin_cpu_supports("avx2") && __builtin_cpu_supports("fma");
+#endif
+    return 1;
+}
+
+/* sum_k v[k] * x[ix[k]]: four 16-bit indices per 64-bit load and the values
+   in SSE2 pairs, so the gather issues ~1.75 loads per nonzero instead of 3 */
+static inline fom_float it_dot(const fom_float *v, const unsigned short *ix, const fom_float *x, fom_int n) {
+    fom_int k = 0;
+    fom_float s;
+#ifdef __SSE2__
+    __m128d a0 = _mm_setzero_pd(), a1 = _mm_setzero_pd();
+    for (; k + 4 <= n; k += 4) {
+        uint64_t w;
+        __m128d x0, x1;
+        memcpy(&w, ix + k, sizeof(w));
+        x0 = _mm_loadh_pd(_mm_load_sd(x + (w & 0xffff)), x + ((w >> 16) & 0xffff));
+        x1 = _mm_loadh_pd(_mm_load_sd(x + ((w >> 32) & 0xffff)), x + (w >> 48));
+        a0 = _mm_add_pd(a0, _mm_mul_pd(_mm_loadu_pd(v + k), x0));
+        a1 = _mm_add_pd(a1, _mm_mul_pd(_mm_loadu_pd(v + k + 2), x1));
+    }
+    a0 = _mm_add_pd(a0, a1);
+    s = _mm_cvtsd_f64(a0) + _mm_cvtsd_f64(_mm_unpackhi_pd(a0, a0));
+#else
+    fom_float s1 = 0.0, s2 = 0.0, s3 = 0.0;
+    s = 0.0;
+    for (; k + 4 <= n; k += 4) {
+        s += v[k] * x[ix[k]];
+        s1 += v[k + 1] * x[ix[k + 1]];
+        s2 += v[k + 2] * x[ix[k + 2]];
+        s3 += v[k + 3] * x[ix[k + 3]];
+    }
+    s = (s + s1) + (s2 + s3);
+#endif
+    for (; k < n; ++k) s += v[k] * x[ix[k]];
+    return s;
+}
+
+/* it_dot with prefetches behind, for a backward walk over the rows */
+static inline fom_float it_dot_pfb(const fom_float *v, const unsigned short *ix, const fom_float *x, fom_int n) {
+#ifdef __SSE2__
+    fom_int k;
+    for (k = 0; k < n; k += 8) _mm_prefetch((const char *)(v + k - IT_PFD), _MM_HINT_T0);
+    for (k = 0; k < n; k += 32) _mm_prefetch((const char *)(ix + k - IT_PFD), _MM_HINT_T0);
+#endif
+    return it_dot(v, ix, x, n);
+}
+
+/* acc[ix[k]] += v[k] * t (the indices of one row are distinct) */
+static inline void it_axpy(const fom_float *v, const unsigned short *ix, fom_float t, fom_float *acc, fom_int n) {
+    fom_int k = 0;
+#ifdef __SSE2__
+    for (k = 0; k < n; k += 8) _mm_prefetch((const char *)(v + k + IT_PFD), _MM_HINT_T0);
+    for (k = 0; k < n; k += 32) _mm_prefetch((const char *)(ix + k + IT_PFD), _MM_HINT_T0);
+    k = 0;
+#endif
+#ifdef __SSE2__
+    {
+        const __m128d tt = _mm_set1_pd(t);
+        for (; k + 4 <= n; k += 4) {
+            uint64_t w;
+            fom_float *a0, *a1, *a2, *a3;
+            __m128d p0, p1;
+            memcpy(&w, ix + k, sizeof(w));
+            a0 = acc + (w & 0xffff);
+            a1 = acc + ((w >> 16) & 0xffff);
+            a2 = acc + ((w >> 32) & 0xffff);
+            a3 = acc + (w >> 48);
+            p0 = _mm_mul_pd(_mm_loadu_pd(v + k), tt);
+            p1 = _mm_mul_pd(_mm_loadu_pd(v + k + 2), tt);
+            _mm_store_sd(a0, _mm_add_sd(_mm_load_sd(a0), p0));
+            _mm_storeh_pd(a1, _mm_add_pd(_mm_load1_pd(a1), p0));
+            _mm_store_sd(a2, _mm_add_sd(_mm_load_sd(a2), p1));
+            _mm_storeh_pd(a3, _mm_add_pd(_mm_load1_pd(a3), p1));
+        }
+    }
+#else
+    for (; k + 4 <= n; k += 4) {
+        uint64_t w;
+        memcpy(&w, ix + k, sizeof(w));
+        acc[w & 0xffff] += v[k] * t;
+        acc[(w >> 16) & 0xffff] += v[k + 1] * t;
+        acc[(w >> 32) & 0xffff] += v[k + 2] * t;
+        acc[w >> 48] += v[k + 3] * t;
+    }
+#endif
+    for (; k < n; ++k) acc[ix[k]] += v[k] * t;
+}
+
+/* next unit of an owner's list (>= IT_SPLIT: none left) */
+static inline fom_int it_grab(fom_int *cnt) {
+    fom_int u;
+    #ifdef _OPENMP
+    #pragma omp atomic capture
+    #endif
+    u = (*cnt)++;
+    return u;
+}
+
+/* out[r[k]] (=, or += if acc) sum_l v[4l+k] x[ix[4l+k]] over the slices
+   [s0, s1) of S: 4 rows per slice in SIMD lanes, two chains of groups */
+static inline void it_sell_pass(const ItSell *S, fom_int s0, fom_int s1, const fom_float *x, fom_float *out, int acc) {
+    fom_int s;
+    for (s = s0; s < s1; ++s) {
+        const fom_int g = S->g[s], L = S->g[s + 1] - g;
+        const fom_float *v = S->v + 4 * g;
+        const unsigned short *ix = S->ix + 4 * g;
+        const fom_int *r = S->r + 4 * s;
+        fom_int l = 0;
+#ifdef __SSE2__
+        __m128d a0 = _mm_setzero_pd(), a1 = _mm_setzero_pd(), c0 = _mm_setzero_pd(), c1 = _mm_setzero_pd();
+        double o[2];
+        for (; l + 2 <= L; l += 2) {
+            uint64_t w, u;
+            memcpy(&w, ix + 4 * l, sizeof(w));
+            memcpy(&u, ix + 4 * l + 4, sizeof(u));
+            a0 = _mm_add_pd(a0, _mm_mul_pd(_mm_loadu_pd(v + 4 * l), _mm_loadh_pd(_mm_load_sd(x + (w & 0xffff)), x + ((w >> 16) & 0xffff))));
+            a1 = _mm_add_pd(a1, _mm_mul_pd(_mm_loadu_pd(v + 4 * l + 2), _mm_loadh_pd(_mm_load_sd(x + ((w >> 32) & 0xffff)), x + (w >> 48))));
+            c0 = _mm_add_pd(c0, _mm_mul_pd(_mm_loadu_pd(v + 4 * l + 4), _mm_loadh_pd(_mm_load_sd(x + (u & 0xffff)), x + ((u >> 16) & 0xffff))));
+            c1 = _mm_add_pd(c1, _mm_mul_pd(_mm_loadu_pd(v + 4 * l + 6), _mm_loadh_pd(_mm_load_sd(x + ((u >> 32) & 0xffff)), x + (u >> 48))));
+        }
+        if (l < L) {
+            uint64_t w;
+            memcpy(&w, ix + 4 * l, sizeof(w));
+            a0 = _mm_add_pd(a0, _mm_mul_pd(_mm_loadu_pd(v + 4 * l), _mm_loadh_pd(_mm_load_sd(x + (w & 0xffff)), x + ((w >> 16) & 0xffff))));
+            a1 = _mm_add_pd(a1, _mm_mul_pd(_mm_loadu_pd(v + 4 * l + 2), _mm_loadh_pd(_mm_load_sd(x + ((w >> 32) & 0xffff)), x + (w >> 48))));
+        }
+        a0 = _mm_add_pd(a0, c0);
+        a1 = _mm_add_pd(a1, c1);
+        if (acc) {
+            _mm_storeu_pd(o, a0);
+            out[r[0]] += o[0]; out[r[1]] += o[1];
+            _mm_storeu_pd(o, a1);
+            out[r[2]] += o[0]; out[r[3]] += o[1];
+        } else {
+            _mm_storeu_pd(o, a0);
+            out[r[0]] = o[0]; out[r[1]] = o[1];
+            _mm_storeu_pd(o, a1);
+            out[r[2]] = o[0]; out[r[3]] = o[1];
+        }
+#else
+        fom_float a[4] = {0.0, 0.0, 0.0, 0.0};
+        fom_int k;
+        for (; l < L; ++l)
+            for (k = 0; k < 4; ++k) a[k] += v[4 * l + k] * x[ix[4 * l + k]];
+        for (k = 0; k < 4; ++k) out[r[k]] = acc ? out[r[k]] + a[k] : a[k];
+#endif
+    }
+}
+
+#ifdef IT_HAVE_AVX2
+/* it_sell_pass (always storing) with AVX2 + FMA: one 4-lane accumulator per
+   chain, the 4 lanes being the slice's 4 rows */
+__attribute__((target("avx2,fma"), noinline))
+static void it_sell_pass_avx2(const ItSell *S, fom_int s0, fom_int s1, const fom_float *x, fom_float *out, int acc) {
+    fom_int s;
+    for (s = s0; s < s1; ++s) {
+        const fom_int g = S->g[s], L = S->g[s + 1] - g;
+        const fom_float *v = S->v + 4 * g;
+        const unsigned short *ix = S->ix + 4 * g;
+        const fom_int *r = S->r + 4 * s;
+        fom_int l = 0;
+        __m256d a = _mm256_setzero_pd(), c = _mm256_setzero_pd();
+        double o[4];
+        for (; l + 2 <= L; l += 2) {
+            uint64_t w, u;
+            __m128d x0, x1, y0, y1;
+            memcpy(&w, ix + 4 * l, sizeof(w));
+            memcpy(&u, ix + 4 * l + 4, sizeof(u));
+            x0 = _mm_loadh_pd(_mm_load_sd(x + (w & 0xffff)), x + ((w >> 16) & 0xffff));
+            x1 = _mm_loadh_pd(_mm_load_sd(x + ((w >> 32) & 0xffff)), x + (w >> 48));
+            y0 = _mm_loadh_pd(_mm_load_sd(x + (u & 0xffff)), x + ((u >> 16) & 0xffff));
+            y1 = _mm_loadh_pd(_mm_load_sd(x + ((u >> 32) & 0xffff)), x + (u >> 48));
+            a = _mm256_fmadd_pd(_mm256_loadu_pd(v + 4 * l), _mm256_set_m128d(x1, x0), a);
+            c = _mm256_fmadd_pd(_mm256_loadu_pd(v + 4 * l + 4), _mm256_set_m128d(y1, y0), c);
+        }
+        if (l < L) {
+            uint64_t w;
+            __m128d x0, x1;
+            memcpy(&w, ix + 4 * l, sizeof(w));
+            x0 = _mm_loadh_pd(_mm_load_sd(x + (w & 0xffff)), x + ((w >> 16) & 0xffff));
+            x1 = _mm_loadh_pd(_mm_load_sd(x + ((w >> 32) & 0xffff)), x + (w >> 48));
+            a = _mm256_fmadd_pd(_mm256_loadu_pd(v + 4 * l), _mm256_set_m128d(x1, x0), a);
+        }
+        _mm256_storeu_pd(o, _mm256_add_pd(a, c));
+        if (acc) {
+            out[r[0]] += o[0]; out[r[1]] += o[1]; out[r[2]] += o[2]; out[r[3]] += o[3];
+        } else {
+            out[r[0]] = o[0]; out[r[1]] = o[1]; out[r[2]] = o[2]; out[r[3]] = o[3];
+        }
+    }
+}
+#define IT_SELL_PASS(S, s0, s1, x, out, acc) (it_avx2 ? it_sell_pass_avx2(S, s0, s1, x, out, acc) : it_sell_pass(S, s0, s1, x, out, acc))
+#else
+#define IT_SELL_PASS it_sell_pass
+#endif
+
+/* Variant 1, two gathers: t0 by rows of A (gather from y), then t1 by rows
+   of B (gather from t0), both in the SELL-4 layouts. Reads A and B. */
+static void it_team_gather(const fom_float *state, fom_float g, const fom_float *bias_t0, const fom_float *bias_t1, const fom_float *bias_su, fom_float l0, fom_float l1, fom_float *t_t0, fom_float *t_t1, fom_float *new_state, int t0copy) {
+    const fom_float *t0src = t0copy ? it_t0n : t_t0;
+    const fom_float *s0 = state, *s1 = state + IT_NH;
+    const fom_float u = FOM_MAT_t1_state_VALS[0];
+    const fom_float w0 = FOM_MAT_t1_state__d_gamma_VALS[0], w1 = FOM_MAT_t1_state__d_gamma_VALS[1];
+    {
+        int tid = 0;
+        fom_int i, b, k, w, rlo, rhi;
+        FOM_PROF_BEGIN(gather_y);
+        #ifdef _OPENMP
+        tid = omp_get_thread_num();
+        #endif
+        /* the last call took every B unit: rearm the lists */
+        #ifdef _OPENMP
+        #pragma omp for schedule(static) nowait
+        #endif
+        for (w = 0; w < it_nob; ++w) it_cnt_b[w].n = 0;
+        #ifdef _OPENMP
+        #pragma omp for schedule(static) nowait
+        #endif
+        for (i = 0; i < IT_NH / 2; ++i) {
+#ifdef __SSE2__
+            const __m128d a = _mm_loadu_pd(s1 + 2 * i), d = _mm_sub_pd(a, _mm_loadu_pd(s0 + 2 * i));
+            _mm_stream_pd(it_y + 2 * i, _mm_add_pd(a, _mm_mul_pd(_mm_set1_pd(g), d)));
+#else
+            it_y[2 * i] = s1[2 * i] + g * (s1[2 * i] - s0[2 * i]);
+            it_y[2 * i + 1] = s1[2 * i + 1] + g * (s1[2 * i + 1] - s0[2 * i + 1]);
+#endif
+        }
+#ifdef __SSE2__
+        _mm_sfence();
+#endif
+        #ifdef _OPENMP
+        #pragma omp barrier
+        #endif
+        if (tid == 0) { FOM_PROF_END(gather_y); }
+        FOM_PROF_BEGIN(gather_t0_tiles_of_A);
+        /* owner w = tile (row window it_wr[w], column group it_wc[w]),
+           IT_SPLIT row units each; start with the own list */
+        for (k = 0, w = tid; k < it_R * it_C; ++k, ++w) {
+            fom_int un;
+            while (w >= it_R * it_C) w -= it_R * it_C;
+            while ((un = it_grab(&it_cnt_a[w].n)) < IT_SPLIT) {
+                const fom_int r = it_wr[w] * IT_SPLIT + un, c = it_wc[w];
+                if (r >= it_sa.nw) continue; /* no rows left for this unit */
+                for (b = c * it_sub; b < (c + 1) * it_sub; ++b)
+                    IT_SELL_PASS(&it_sa, it_sa.wb[b * IT_MAXW + r], it_sa.wb[b * IT_MAXW + r + 1], it_y, it_pg[c], b > c * it_sub);
+            }
+        }
+        #ifdef _OPENMP
+        #pragma omp barrier
+        #endif
+        if (tid == 0) { FOM_PROF_END(gather_t0_tiles_of_A); }
+        FOM_PROF_BEGIN(gather_t0_reduce);
+        /* every A unit is taken: rearm the lists for the next call */
+        #ifdef _OPENMP
+        #pragma omp for schedule(static) nowait
+        #endif
+        for (w = 0; w < it_R * it_C; ++w) it_cnt_a[w].n = 0;
+        #ifdef _OPENMP
+        #pragma omp for schedule(static) nowait
+        #endif
+        for (rlo = 0; rlo < IT_M / 2; ++rlo) {
+            fom_float v, v1;
+            i = 2 * rlo;
+            v = it_pg[0][i]; v1 = it_pg[0][i + 1];
+            for (b = 1; b < it_C; ++b) { v += it_pg[b][i]; v1 += it_pg[b][i + 1]; }
+            v += bias_t0[i];
+            v1 += bias_t0[i + 1];
+            v = (v > l0) ? (v - l0) : ((v < -l0) ? (v + l0) : 0.0);
+            v1 = (v1 > l0) ? (v1 - l0) : ((v1 < -l0) ? (v1 + l0) : 0.0);
+            t_t0[i] = v;
+            t_t0[i + 1] = v1;
+            if (t0copy) {
+#ifdef __SSE2__
+                _mm_stream_pd(it_t0n + i, _mm_set_pd(v1, v));
+#else
+                it_t0n[i] = v;
+                it_t0n[i + 1] = v1;
+#endif
+            }
+        }
+#ifdef __SSE2__
+        if (t0copy) _mm_sfence();
+#endif
+        #ifdef _OPENMP
+        #pragma omp barrier
+        #endif
+        if (tid == 0) { FOM_PROF_END(gather_t0_reduce); }
+        FOM_PROF_BEGIN(gather_t1_rows_of_B);
+        /* owner w: windows w IT_SPLIT + un of rows of B, own list first */
+        for (k = 0, w = tid; k < it_nob; ++k, ++w) {
+            fom_int un;
+            while (w >= it_nob) w -= it_nob;
+            while ((un = it_grab(&it_cnt_b[w].n)) < IT_SPLIT) {
+                const fom_int x = w * IT_SPLIT + un;
+                if (x >= it_sb.nw) continue;
+                for (b = 0; b < it_sb.nb; ++b)
+                    IT_SELL_PASS(&it_sb, it_sb.wb[b * IT_MAXW + x], it_sb.wb[b * IT_MAXW + x + 1], t0src, it_partb, b > 0);
+                rlo = x * it_sb.w;
+                rhi = rlo + it_sb.w < IT_NH ? rlo + it_sb.w : IT_NH;
+                for (i = rlo; i < rhi; ++i) {
+                    fom_float v = u * s1[i] + g * (w0 * s0[i] + w1 * s1[i]) + it_partb[i];
+                    if (bias_t1) v += bias_t1[i];
+                    v = (v > l1) ? (v - l1) : ((v < -l1) ? (v + l1) : 0.0);
+                    t_t1[i] = v;
+                    if (bias_su) {
+                        new_state[i] = bias_su[i] + s1[i];
+                        new_state[IT_NH + i] = bias_su[IT_NH + i] + v;
+                    } else { /* NULL biases are zero: not read */
+                        new_state[i] = s1[i];
+                        new_state[IT_NH + i] = v;
+                    }
+                }
+            }
+        }
+        if (tid == 0) { FOM_PROF_END(gather_t1_rows_of_B); }
+        (void)tid;
+    }
+}
+
+/* the iteration on a new team (fom_iterate called outside a parallel region) */
+static void it_run_gather(const fom_float *state, fom_float g, const fom_float *bias_t0, const fom_float *bias_t1, const fom_float *bias_su, fom_float l0, fom_float l1, fom_float *t_t0, fom_float *t_t1, fom_float *new_state, int t0copy) {
+    #ifdef _OPENMP
+    #pragma omp parallel
+    #endif
+    it_team_gather(state, g, bias_t0, bias_t1, bias_su, l0, l1, t_t0, t_t1, new_state, t0copy);
+}
+
+#ifdef IT_HAVE_AVX2
+/* step 3 of the B-only variant with AVX2 + FMA dots: rows hi-1 .. lo */
+__attribute__((target("avx2,fma"), noinline))
+static void it_bonly_t1_avx2(fom_int lo, fom_int hi, const fom_float *s0, const fom_float *s1, fom_float g, fom_float u, fom_float w0, fom_float w1, fom_float l1, const fom_float *bias_t1, const fom_float *bias_su, const fom_float *t_t0, fom_float *t_t1, fom_float *new_state) {
+    fom_int j;
+    for (j = hi - 1; j >= lo; --j) {
+        const fom_int p = FOM_MAT_t1_t_t0_ROWPTR[j], n = FOM_MAT_t1_t_t0_ROWPTR[j + 1] - p;
+        const fom_float *v = FOM_MAT_t1_t_t0_VALS + p;
+        const unsigned short *ix = it_bidx + p;
+        __m256d a = _mm256_setzero_pd(), c = _mm256_setzero_pd();
+        __m128d h;
+        fom_float d, x;
+        fom_int k;
+        for (k = 0; k < n; k += 8) _mm_prefetch((const char *)(v + k - IT_PFD), _MM_HINT_T0);
+        for (k = 0; k < n; k += 32) _mm_prefetch((const char *)(ix + k - IT_PFD), _MM_HINT_T0);
+        k = 0;
+        for (; k + 8 <= n; k += 8) {
+            uint64_t w, q;
+            memcpy(&w, ix + k, sizeof(w));
+            memcpy(&q, ix + k + 4, sizeof(q));
+            a = _mm256_fmadd_pd(_mm256_loadu_pd(v + k), _mm256_set_m128d(
+                _mm_loadh_pd(_mm_load_sd(t_t0 + ((w >> 32) & 0xffff)), t_t0 + (w >> 48)),
+                _mm_loadh_pd(_mm_load_sd(t_t0 + (w & 0xffff)), t_t0 + ((w >> 16) & 0xffff))), a);
+            c = _mm256_fmadd_pd(_mm256_loadu_pd(v + k + 4), _mm256_set_m128d(
+                _mm_loadh_pd(_mm_load_sd(t_t0 + ((q >> 32) & 0xffff)), t_t0 + (q >> 48)),
+                _mm_loadh_pd(_mm_load_sd(t_t0 + (q & 0xffff)), t_t0 + ((q >> 16) & 0xffff))), c);
+        }
+        a = _mm256_add_pd(a, c);
+        h = _mm_add_pd(_mm256_castpd256_pd128(a), _mm256_extractf128_pd(a, 1));
+        d = _mm_cvtsd_f64(h) + _mm_cvtsd_f64(_mm_unpackhi_pd(h, h));
+        for (; k < n; ++k) d += v[k] * t_t0[ix[k]];
+        x = u * s1[j] + g * (w0 * s0[j] + w1 * s1[j]) + d;
+        if (bias_t1) x += bias_t1[j];
+        x = (x > l1) ? (x - l1) : ((x < -l1) ? (x + l1) : 0.0);
+        t_t1[j] = x;
+        if (bias_su) {
+            new_state[j] = bias_su[j] + s1[j];
+            new_state[IT_NH + j] = bias_su[IT_NH + j] + x;
+        } else {
+            new_state[j] = s1[j];
+            new_state[IT_NH + j] = x;
+        }
+    }
+}
+#endif
+
+/* Variant 2, B only (t1_t_t0 = c A^T): A y = (1/c) B^T y by scattering
+   rows of B into per-thread accumulators, then t1 by gathering the same
+   rows (walked backwards, still in L2). Reads B twice and A never: best
+   when A and B together do not fit in cache. */
+static void it_team_bonly(const fom_float *state, fom_float g, const fom_float *bias_t0, const fom_float *bias_t1, const fom_float *bias_su, fom_float l0, fom_float l1, fom_float *t_t0, fom_float *t_t1, fom_float *new_state) {
+    const fom_float *s0 = state, *s1 = state + IT_NH;
+    const fom_float u = FOM_MAT_t1_state_VALS[0];
+    const fom_float w0 = FOM_MAT_t1_state__d_gamma_VALS[0], w1 = FOM_MAT_t1_state__d_gamma_VALS[1];
+    const fom_float rc = it_rc;
+    {
+        fom_int i, j, k, tid = 0, T = 1, lo = IT_NH, hi = 0;
+        fom_float *acc;
+        #ifdef _OPENMP
+        tid = omp_get_thread_num();
+        T = omp_get_num_threads();
+        #endif
+        FOM_PROF_BEGIN(bonly_scatter_BTy);
+        acc = it_acc[tid];
+        memset(acc, 0, sizeof(it_acc[0]));
+        /* 1. acc = B^T y over the thread's rows of B */
+        #ifdef _OPENMP
+        #pragma omp for schedule(static) nowait
+        #endif
+        for (j = 0; j < IT_NH; ++j) {
+            fom_int p = FOM_MAT_t1_t_t0_ROWPTR[j];
+            if (j < lo) lo = j;
+            hi = j + 1;
+            it_axpy(FOM_MAT_t1_t_t0_VALS + p, it_bidx + p, s1[j] + g * (s1[j] - s0[j]), acc, FOM_MAT_t1_t_t0_ROWPTR[j + 1] - p);
+        }
+        #ifdef _OPENMP
+        #pragma omp barrier
+        #endif
+        if (tid == 0) { FOM_PROF_END(bonly_scatter_BTy); }
+        FOM_PROF_BEGIN(bonly_reduce_t0);
+        /* 2. t0 = act(bias + (1/c) sum of the accumulators) */
+        #ifdef _OPENMP
+        #pragma omp for schedule(static)
+        #endif
+        for (i = 0; i < IT_M; ++i) {
+            fom_float v = 0.0;
+            for (k = 0; k < T; ++k) v += it_acc[k][i];
+            v = bias_t0[i] + rc * v;
+            v = (v > l0) ? (v - l0) : ((v < -l0) ? (v + l0) : 0.0);
+            t_t0[i] = v;
+        }
+        if (tid == 0) { FOM_PROF_END(bonly_reduce_t0); }
+        FOM_PROF_BEGIN(bonly_gather_t1);
+        /* 3. t1 = act(... + B t0) and the shift, over the same rows, walked
+           backwards so the pass starts on the data phase 1 left in L2 (and
+           the next phase 1 on the data this one leaves) */
+#ifdef IT_HAVE_AVX2
+        if (it_avx2) it_bonly_t1_avx2(lo, hi, s0, s1, g, u, w0, w1, l1, bias_t1, bias_su, t_t0, t_t1, new_state);
+        else
+#endif
+        if (bias_t1 && bias_su) {
+            for (j = hi - 1; j >= lo; --j) {
+                fom_int p = FOM_MAT_t1_t_t0_ROWPTR[j];
+                fom_float v = bias_t1[j] + u * s1[j] + g * (w0 * s0[j] + w1 * s1[j])
+                    + it_dot_pfb(FOM_MAT_t1_t_t0_VALS + p, it_bidx + p, t_t0, FOM_MAT_t1_t_t0_ROWPTR[j + 1] - p);
+                v = (v > l1) ? (v - l1) : ((v < -l1) ? (v + l1) : 0.0);
+                t_t1[j] = v;
+                new_state[j] = bias_su[j] + s1[j];
+                new_state[IT_NH + j] = bias_su[IT_NH + j] + v;
+            }
+        } else { /* NULL biases are zero: not read */
+            for (j = hi - 1; j >= lo; --j) {
+                fom_int p = FOM_MAT_t1_t_t0_ROWPTR[j];
+                fom_float v = u * s1[j] + g * (w0 * s0[j] + w1 * s1[j])
+                    + it_dot_pfb(FOM_MAT_t1_t_t0_VALS + p, it_bidx + p, t_t0, FOM_MAT_t1_t_t0_ROWPTR[j + 1] - p);
+                v = (v > l1) ? (v - l1) : ((v < -l1) ? (v + l1) : 0.0);
+                t_t1[j] = v;
+                new_state[j] = s1[j];
+                new_state[IT_NH + j] = v;
+            }
+        }
+        if (tid == 0) { FOM_PROF_END(bonly_gather_t1); }
+    }
+}
+
+static void it_run_bonly(const fom_float *state, fom_float g, const fom_float *bias_t0, const fom_float *bias_t1, const fom_float *bias_su, fom_float l0, fom_float l1, fom_float *t_t0, fom_float *t_t1, fom_float *new_state) {
+    int nt = 1;
+    #ifdef _OPENMP
+    nt = omp_get_max_threads();
+    if (nt > IT_MAXT) nt = IT_MAXT;
+    #pragma omp parallel num_threads(nt)
+    #endif
+    it_team_bonly(state, g, bias_t0, bias_t1, bias_su, l0, l1, t_t0, t_t1, new_state);
+    (void)nt;
+}
+
+/* All variants are exact (the gather ones bitwise equal); which is fastest
+   depends on the cache hierarchy and on where the threads sit (measured: B
+   only and gather each win by ~17% on one of two server CPUs, gather from
+   t_t0 or from the streamed copy by ~14% with all threads on one socket or
+   not). The first 3 * IT_TUNE calls of an OpenMP build time each in turn,
+   then the fastest one is kept. */
+static int it_variant = 0; /* 0 tuning, 1 gather, 2 B only, 3 gather via a copy of t0 */
+#ifdef _OPENMP
+static int it_tuned = 0;
+static double it_best[3];
+#endif
+
+/* bias_t1 / bias_su may both be NULL, meaning zero (the solver's case):
+   then they are not read. */
+#ifdef _OPENMP
+/* fom_iterate called by every thread of a team (a solver keeping one
+   parallel region over its whole loop): the same steps, run by that team
+   with no fork or join per iteration; returns after a barrier. Every thread
+   takes the same path: the flags read here change only between barriers
+   that the whole team passes. */
+static void it_iterate_team(const fom_float *state, fom_float sc_gamma, const fom_float *bias_t0, const fom_float *bias_t1, const fom_float *bias_su, const FomAct_t0 *act_t0, const FomAct_t1 *act_t1, fom_float *t_t0, fom_float *t_t1, fom_float *new_state) {
+    const fom_float l0 = act_t0->lambd, l1 = act_t1->lambd;
+    double t = 0.0;
+    int v, n;
+    if (it_mode == 0) { /* every thread reads 0 before anyone can pass */
+        #pragma omp barrier
+        #pragma omp single
+        it_mode = it_check();
+    }
+    if (it_mode < 0 || omp_get_num_threads() > IT_MAXT) {
+        /* generic steps: one thread runs the call (the parallel regions
+           inside run nested) */
+        #pragma omp single
+        {
+            fom_step_t0(state, sc_gamma, bias_t0, act_t0, t_t0);
+            fom_step_t1(state, t_t0, sc_gamma, bias_t1, act_t1, t_t1);
+            fom_step_su(state, t_t1, bias_su, new_state);
+        }
+        return;
+    }
+    if (it_variant != 0) {
+        if (it_variant == 2) {
+            it_team_bonly(state, sc_gamma, bias_t0, bias_t1, bias_su, l0, l1, t_t0, t_t1, new_state);
+        } else {
+            FOM_PROF_BEGIN(variant_gather);
+            it_team_gather(state, sc_gamma, bias_t0, bias_t1, bias_su, l0, l1, t_t0, t_t1, new_state, it_variant == 3);
+            if (omp_get_thread_num() == 0) { FOM_PROF_END(variant_gather); }
+        }
+        #pragma omp barrier
+        return;
+    }
+    /* tuning, as in fom_iterate; the master keeps the score */
+    v = it_tuned < IT_TUNE ? 0 : (it_tuned < 2 * IT_TUNE ? 1 : 2);
+    n = it_tuned - v * IT_TUNE;
+    #pragma omp master
+    t = omp_get_wtime();
+    if (v == 1) it_team_bonly(state, sc_gamma, bias_t0, bias_t1, bias_su, l0, l1, t_t0, t_t1, new_state);
+    else it_team_gather(state, sc_gamma, bias_t0, bias_t1, bias_su, l0, l1, t_t0, t_t1, new_state, v == 2);
+    #pragma omp barrier
+    #pragma omp master
+    {
+        t = omp_get_wtime() - t;
+        if (4 * n == IT_TUNE || (4 * n > IT_TUNE && t < it_best[v])) it_best[v] = t;
+        if (++it_tuned == 3 * IT_TUNE) {
+            it_variant = it_best[1] < it_best[0] ? 2 : 1;
+            if (it_best[2] < it_best[it_variant - 1]) it_variant = 3;
+        }
+    }
+    #pragma omp barrier
+}
+#endif
+
 void fom_iterate(const fom_float *state, fom_float sc_gamma, const fom_float *bias_t0, const fom_float *bias_t1, const fom_float *bias_su, const FomAct_t0 *act_t0, const FomAct_t1 *act_t1, fom_float *t_t0, fom_float *t_t1, fom_float *new_state) {
-    fom_step_t0(state, sc_gamma, bias_t0, act_t0, t_t0);
-    fom_step_t1(state, t_t0, sc_gamma, bias_t1, act_t1, t_t1);
-    fom_step_su(state, t_t1, bias_su, new_state);
-}
+    const fom_float l0 = act_t0->lambd, l1 = act_t1->lambd;
+#ifdef _OPENMP
+    if (omp_in_parallel()) {
+        it_iterate_team(state, sc_gamma, bias_t0, bias_t1, bias_su, act_t0, act_t1, t_t0, t_t1, new_state);
+        return;
+    }
+#endif
+    if (it_mode == 0) it_mode = it_check();
+    if (it_mode < 0) {
+        fom_step_t0(state, sc_gamma, bias_t0, act_t0, t_t0);
+        fom_step_t1(state, t_t0, sc_gamma, bias_t1, act_t1, t_t1);
+        fom_step_su(state, t_t1, bias_su, new_state);
+        return;
+    }
+#ifdef _OPENMP
+    if (it_variant == 0) {
+        /* IT_TUNE calls of each variant in a row; the first quarter of each
+           block warms the caches and is not scored */
+        const int v = it_tuned < IT_TUNE ? 0 : (it_tuned < 2 * IT_TUNE ? 1 : 2), n = it_tuned - v * IT_TUNE;
+        double t = omp_get_wtime();
+        if (v == 1) it_run_bonly(state, sc_gamma, bias_t0, bias_t1, bias_su, l0, l1, t_t0, t_t1, new_state);
+        else it_run_gather(state, sc_gamma, bias_t0, bias_t1, bias_su, l0, l1, t_t0, t_t1, new_state, v == 2);
+        t = omp_get_wtime() - t;
+        if (4 * n == IT_TUNE || (4 * n > IT_TUNE && t < it_best[v])) it_best[v] = t;
+        if (++it_tuned == 3 * IT_TUNE) {
+            it_variant = it_best[1] < it_best[0] ? 2 : 1;
+            if (it_best[2] < it_best[it_variant - 1]) it_variant = 3;
+        }
+        return;
+    }
+#else
+    if (it_variant == 0) it_variant = 1;
+#endif
+    if (it_variant == 2) {
+        FOM_PROF_BEGIN(variant_bonly);
+        it_run_bonly(state, sc_gamma, bias_t0, bias_t1, bias_su, l0, l1, t_t0, t_t1, new_state);
+        FOM_PROF_END(variant_bonly);
+    } else {
+        FOM_PROF_BEGIN(variant_gather);
+        it_run_gather(state, sc_gamma, bias_t0, bias_t1, bias_su, l0, l1, t_t0, t_t1, new_state, it_variant == 3);
+        FOM_PROF_END(variant_gather);
+    }
+}
```

## Agent notes

```
### Round 1

**Whole solve (200 iterations), `mean_us`**
- Initial: 216,231 us on the first node (Ice Lake Xeon Gold 6342). The same original code measured 170,496 us on the final node (AMD EPYC 7413, Zen 3).
- Final: 48,920 us on the Zen 3 node, a **3.5x** speedup on the same hardware.
- On the Ice Lake node, the best version measured there (B-only design, before the last tweaks) ran in 52,193 us, a **4.1x** speedup. The final code was not re-timed on Ice Lake.

**Final profile (Zen 3, `make profile`, 41 solves, ~49.5 ms/solve), hottest first**
| Region | Time per solve | Share |
|---|---|---|
| `gather_t0_rows_of_A` | 24.4 ms | 49% |
| `gather_t1_rows_of_B` | 21.2 ms | 43% |
| `gather_y` (includes the parallel-region fork) | 3.0 ms | 6% |
| `bias` | 0.02 ms | <0.1% |
| `residual` | 0.02 ms | <0.1% |

`iterate` and `variant_gather` wrap the whole iteration (~49.4 ms, 99.8%), so they double-count the regions above.

**Optimizations applied** (all of them live in `fom_iterate`. Every structure is checked once from the matrix data; if any check fails, the generic step routines run.)
1. **t0 needs one product with A instead of three.** Structure: `t0_state = [0 | A]` and `t0_state__d_gamma = [-A | A]` have the same pattern and values equal to within 4e-16. So t0 = act(bias + A·y) with y = s1 + g(s1 − s0). This cut traffic per iteration from about 125 MB to 52 MB. Speedup 216 → 72 ms on Ice Lake.
2. **Compact 16-bit column indices.** Structure: the column counts (40000 and 10000) fit in 16 bits. Four indices are read per 64-bit load and values are multiplied in SSE2 pairs, so a gather costs about 1.75 loads per nonzero instead of 3.
3. **Diagonal blocks become scalars.** Structure: `t1_state` and `t1_state__d_gamma` hold one constant value each per diagonal (1±ε, checked exactly equal on every row). Three scalars replace about 1 MB per iteration of value reads.
4. **State update is a pure shift.** Structure: `su` copies s1 and t1 (selection rows). It is fused into the t1 loop, and the state ping-pongs between `work->state` and `solution.x`, so no copies or zeroing passes remain.
5. **Software prefetch** (T0 hint, 128 to 256 elements ahead) on the matrix streams: about 5%.
6. **B-only variant**, selected where A and B together don't fit in cache (it won on Ice Lake). Structure: `t1_t_t0 = c·Aᵀ`, with pattern equal and values within 1.3e-15 relative, every entry checked. c comes from a division-free Newton reciprocal. A·y = (1/c)·Bᵀy is computed by scattering B's rows into per-thread accumulators; each thread owns its y entries, so no barrier is needed. Then t1 gathers over the same rows, walked backwards so they are still in L2. Only 26 MB per iteration is read, and A never.
7. **Column-blocked copy of A for the gather variant** (4 blocks of 10000 columns). One 80 KB slice of y stays in Zen 3's 512 KB L2: 53.4 → 50.7 ms.
8. **Zero biases passed as NULL** and not read: about 2%.
9. **Timed selection between the two exact variants**, as described above. Each variant wins by about 17% on one of the two CPUs.

**Tried and dropped** (no gain or slower): non-temporal prefetch, snake (alternating-direction) row order, a fused gather+scatter pass that caches Bᵀs between calls, dynamic scheduling, work stealing, one parallel region for the whole solve, reducer-side zeroing of the accumulators, alignment peeling, 8-wide unrolling.

Blocked by rules:
- AVX2/AVX-512 (FMA, wider vectors, `vgatherdpd`) through `-march` or `__attribute__((target))`; the kernels are latency/throughput-bound at about 2 cycles per nonzero with SSE2 only | build flags are fixed | unknown, likely 10-20%
- Computing y for the next iteration inside the current t1 pass (removes the y pass and a barrier) | the `fom_iterate` signature in `include/fom_ops.h` is fixed and does not receive the next gamma | ~6% (the `gather_y` region)
- Documenting the NULL-means-zero bias convention in the header | headers are fixed | none (documentation only; it is commented in `src/fom_ops.c` and `src/fom_solver.c`)

### Round 2

**Whole-solve `mean_us`:** 50057.6 us at the start of round 2, 31636.5 us at the end (1.58x this round; 5.41x vs the 171117.7 us translation). Run head-to-head on the same node, round 1 measured 51.1 ms and the final version 31.6 ms (1.62x). Run-to-run noise is about ±2%.

**Final profile** (`make profile`, 200 iterations x 63 solves; `iterate` contains `variant_gather`, which contains the four phases):

| Region | Total (us) | Per iteration | % |
|---|---|---|---|
| iterate | 1999866 | 158.7 us | 33.8 |
| variant_gather | 1996808 | 158.5 us | 33.8 |
| gather_t1_rows_of_B | 862680 | 68.5 us | 14.6 |
| gather_t0_tiles_of_A | 838114 | 66.5 us | 14.2 |
| gather_t0_reduce | 134340 | 10.7 us | 2.3 |
| gather_y | 78490 | 6.2 us | 1.3 |
| residual | 1241 | | 0.02 |
| bias | 1109 | | 0.02 |

**Optimizations applied** (all in `src/fom_ops.c`, gather variant of `fom_iterate`):
1. **SELL-4 layout for A (gathers y) and B = c·Aᵀ (gathers t0).**
   - Rows are sorted by length within each thread's window and packed 4 per slice, interleaved, so the 4 rows run in SIMD lanes.
   - This removes the per-row loop-exit mispredicts and horizontal sums that made short blocked rows expensive.
   - Valid because A is uniformly random: exactly 260 nonzeros per row and 64-65 per column, so padding is only 0.3%.
   - Gain: 48.5 to 36.8 ms.
2. **Column blocking (A: 12 blocks of about 3334 columns; B: 4 blocks of 2500).** Each gathered source block (27 KB / 20 KB) stays in L1 while a pass runs over it. The block counts were swept, and these were best.
3. **Streaming stores for y and an internal copy of t0.**
   - Every thread reads these vectors, so normal stores pay a read-for-ownership and cross-CCD invalidations.
   - The y loop was measured at about 4 GB/s and was coherence-bound.
   - Gain: about 4%.
4. **AVX2+FMA SELL kernel, picked at run time by `__builtin_cpu_supports`.** Scalar gathers feed 256-bit FMAs; the SSE2 kernel stays as the fallback. Gain: about 3.5%.
5. **A in 4×4 row × column tiles.**
   - Each slice of y is read by only the 4 threads of its column group.
   - t0 becomes a 4-way partial-sum reduction, instead of either 16 readers per y line or a 16-way reduction.
   - The tile split is chosen at setup (R = 4, 2 or 1 row windows, the largest dividing T) and tiles go through `omp for`, so results are correct for any thread count.
   - Gain: about 2.5%.
6. **Removed a leftover per-call `getenv` in `fom_iterate`.** Gain: about 1 ms.

**Tried and reverted:**
- **Single read of A** (dot each row, then scatter it while still in L1): 2x slower, because of the 40000-entry per-thread scatter and its reduction.
- **AVX2 hardware gather** (vgatherdpd): slower on Zen 3.
- **Software prefetch** of the streams, of the next block's source, and in the reduce: no gain or slower.
- **Interleaving values and indices** into one stream: slower.
- **12-bit packed indices:** slower.
- **Work-stealing balance** across threads: no net gain.
- **Pure column partition of A:** about 1% faster, but needs 44 MB more static data.
- **Taking ownership of y's lines early** during the B pass: the cost just moved into the B pass.
- **y-sparsity skipping:** the iterates stay dense for all 200 iterations.

Blocked by rules:
- Fold y_{k+1} = t1 + g_{k+1}(t1 − s1) into iteration k's B pass (removes the y phase and its barrier) | no schedule reads in fom_ops.c / fixed fom_iterate signature (it only receives the current gamma) | ~3-4% (y phase 6 us/iter plus its barrier)
- Compile with -march=native (AVX2/FMA in all loops, including auto-vectorized ones) | Makefile/build flags fixed (worked around with a target attribute and runtime dispatch for the hot kernel) | <2%

### Round 3

**Initial vs. final whole-solve `mean_us`**
- Initial: 31515 us, measured on the earlier quiet EPYC 7413 node (all 16 threads on one socket).
- The allocation then moved to della-l07g6. There one thread sits on the other socket (CPU 36) and other tenants share the CCDs, so `make bench` swings between 35 and 60 ms from run to run.
- Same node, alternating repeated runs, median of 6:
  - starting solver ~53 ms vs. final ~38 ms, about **1.4x**;
  - restricted to 12 local cores: 67.5 ms (starting) vs. 56 ms (final).
- Last `make bench` of the final solver: 34688 us.
- On the quiet node, the only change measured there took 31.5 ms to 30.7 ms (about 1.025x).
- Cumulative vs. the translated solver: 171118 / 34688 = about 4.9x on this node's last run. The noise here makes that figure soft.

**Final profile (`make profile`, noisy node, mean 46871 us per solve; pct is of the summed region total, and regions nest)**

| Region | total_us | pct |
|---|---|---|
| iterate | 2007232 | 35.5% |
| variant_gather | 1820287 | 32.2% |
| gather_t0_tiles_of_A | 745017 | 13.2% |
| gather_t1_rows_of_B | 619325 | 11.0% |
| gather_t0_reduce | 236207 | 4.2% |
| gather_y | 217941 | 3.9% |
| residual | 1403 | 0.02% |
| bias | 1004 | 0.02% |

**Optimizations applied**
1. **Work stealing for the A and B passes (largest gain on uneven nodes).**
   - Each thread's A tile (row window × column group) and its window of B = c·Aᵀ rows is split into 4 row units in the SELL-4 layout.
   - A thread runs its own units first, then takes units other threads haven't started, using one atomic counter per owner.
   - This is valid because units write disjoint output rows: A-tile units write rows of `it_pg[c]`; B units write rows of `it_partb`, t1 and the new state.
   - Owners keep their data locality when the load is balanced. The matrix data is read-only, so a stolen unit leaves the owner's cached copy valid.
   - On the cross-socket/noisy node this lowered the median from about 56 to about 44 ms, and per-thread A/B times went from 63-118 us to an even ~70-80 us. Neutral or slightly positive on 12 local cores.
2. **Persistent OpenMP team.**
   - `fom_solve` opens one parallel region for the whole loop.
   - Called inside a region, `fom_iterate` runs on the caller's team (`it_iterate_team`) and ends with a barrier, so there is no fork/join per iteration.
   - The branch flags it reads change only between barriers the whole team passes, so all threads take the same path.
   - About 2-5% better in both placements; it matters most when one thread sits on a remote socket.
3. **B pass gathers t0 straight from `t_t0`, picked at run time.**
   - The previous code wrote a streamed copy of t0 and fenced it.
   - The direct read is about 2.5% faster with all threads on one socket, but about 14% slower with a thread on another socket.
   - Both gathers are bitwise identical, so the existing timing tuner now chooses among 3 exact variants: gather from `t_t0`, gather from the streamed copy, or B-only.

**Tried and rejected (measured, not blocked)**
- AVX2 hardware gather: 47 vs. 31.5 ms on Zen 3.
- Software prefetch of the SELL streams (several distances): 3-7% slower in the solver, although it helped an isolated microbenchmark.
- 32-bit index-pair decode: about 5% slower, because it adds loads.
- Prefetching each y/t0 block before its pass: neutral.
- Per-thread private y: much slower.
- Row-window-major tile order with a local reduce: neutral.
- Other tile shapes (R = 2, 8, 16): neutral or slower.
- Normal stores for y: neutral.
- Aliasing t1 into the new state: neutral.
- Splitting into 8 units: worse.
- Dynamic or stolen scheduling of the small y and reduce loops: worse.
- Exploiting a sparse y: y was dense (39000-40000 nonzeros every iteration).
- Storing A in float32 exactly: no A or B entry is float32-representable.

Blocked by rules:
- Store A and B values in fewer bytes (e.g. a float plus a 16-bit correction, about 2^-40 relative error, 6 B instead of 8 B per value) to cut L3 traffic in the bandwidth-bound passes | a kernel must apply exactly M (lossy coefficients); the 1e-10 ops tolerance would probably accept it, but it changes the matrix | ~15-25% on the two main passes
- Hoist y = s1 + g'(s1 - s0) for the next iteration into the B tail, to drop the y pass and one barrier | `src/fom_ops.c` may not read the schedule, and `fom_iterate` only receives the current gamma | ~2-4%
- Slide the state through the contiguous `state`/`new_state`/`acc` workspace so the shift copy new_s0 = s1 becomes a no-op | fixed workspace layout (headers not editable); pointer arithmetic across struct members is not portable | ~1%

### Round 4

**Timing (whole-solve `mean_us`)**
- Start of round: 47159 µs (from the round brief; 171117 µs for the translated solver).
- AMD EPYC 7413 node (15 cores on one socket plus 1 thread on the other socket):
  - The round-3 solver gave 36000–48000 µs, median about 38000 µs (interleaved runs, very noisy).
  - The final version on that node: `make bench` 31028 µs. The interleaved median of the team-size change alone was 33.5 ms, steady from run to run.
  - Speedup about 1.2–1.5x this round. Against the translated solver: 5.5x.
- Intel Xeon Gold 6342 node (16 cores on one socket; the allocation moved there mid-round):
  - Interleaved control: round-3 binary 53688 µs, final 52320 µs (2.6% faster).
  - Final `make bench` there: 50860 µs.

**Final profile (Intel node, where the tuner picks the B-only variant), hottest first**
| region | total | % |
|---|---|---|
| iterate | 2019014 µs | 50.1% |
| bonly_scatter_BTy | 1171148 µs | 29.1% |
| bonly_gather_t1 | 718521 µs | 17.8% |
| bonly_reduce_t0 | 118287 µs | 2.9% |
| residual | 385 µs | 0.01% |
| bias | 350 µs | 0.01% |

`iterate` contains the other regions.

On the AMD node the tuner picks the gather variant instead. Per iteration that is about 156 µs:
| region | % |
|---|---|
| gather_t1_rows_of_B | 46% |
| gather_t0_tiles_of_A | 45% |
| gather_t0_reduce | 4.7% |
| gather_y | 4.2% |

**Optimizations applied**
1. **Team size chosen from the machine layout** (`fom_solver.c`).
   - `fom_setup` checks once which NUMA node (memory/socket domain) each OpenMP thread runs on.
   - If the trailing threads sit on another node and are at most a quarter of the team, the solve runs only on the leading threads, via `num_threads`.
   - Why it pays: the iteration is bound by cache traffic and per-phase barriers. One thread on the other socket made every barrier wait and pulled data across the link.
   - Effect on AMD: about 38–48 → 33 ms, and the run-to-run spread disappeared. It changes nothing on a single-socket placement such as the Intel node.
   - Letting that thread sit idle inside the work-stealing loop did not help, because it still paid every barrier.
2. **Any-divisor tile grid for A** (`fom_ops.c`).
   - The tiles of `A` (the shared block of `t0_state` and the d_gamma matrix) were limited to 4, 2 or 1 row windows, i.e. powers of two.
   - With 15 threads that gave 1×15 tiles, so the reduce summed 15 partial vectors.
   - Now the row-window count is the largest divisor of T up to 4, giving 3×5 tiles for 15 threads.
   - Effect on AMD: 33.3 → 31.0 ms. On Intel, 4×4 stays best for 16 threads.
3. **AVX2+FMA row dot in the B-only gather pass.**
   - Structure used: `t1_t_t0 = c·Aᵀ`, read as CSR rows with 16-bit indices.
   - One 256-bit fused multiply-add per 4 nonzeros, where the old SSE2 code did a multiply and an add per pair.
   - Effect on Intel: about 3% (53.8 → 52.3 ms).

**Tried and rejected (slower or within noise)**
- Row-only tiles for A with the t0 activation fused into the A pass (worse: y is shared by every thread).
- Ignoring the cross-socket thread for work but keeping it in the team.
- Different column-block widths and block counts for B.
- Hardware gathers (`vgatherdpd`): about 3x slower on Ice Lake.
- Thread-private y to remove the y barrier (10% slower on Intel).
- Counter-triggered reduce done by the last finisher instead of a reduce barrier (5% slower).
- Column-split B so the scatter accumulator fits in L1: scatter got faster, gather got slower, net about 1%.
- AVX2 scatter.
- Prefetch-distance tuning.

Perf counters on Intel show almost no last-level-cache misses. The products are bound by L3 bandwidth and instruction count at about 10 bytes per nonzero, and that byte count cannot shrink exactly (see below).

Blocked by rules:
- Store the values of A/B in compressed form (48-bit truncated or float32+correction mantissas; 6 instead of 8 bytes per value) to cut L3/DRAM traffic about 25–40% | the `make verify` ops tolerance of 1e-10 relative and the exactness requirement on `fom_iterate` (values are random and not float32-exact) | about 20–30% on bandwidth-bound nodes
- Keep A·s0 from the previous iteration (A·y = (1+g)·A·s1 − g·A·s0, where s0 is the previous s1) to drop one third of the A work and the y pass | `fom_iterate` must be right for arbitrary input states; checking that the cache matches costs as much as recomputing it | about 5–10% (unknown)
- Compute next iteration's y during the B phase to remove the y pass and its barrier | `src/fom_ops.c` may not read the schedule, and `fom_iterate` gets only the current gamma | about 3–4%

### Round 5

**Result:** no further speedup; sources left unchanged at the round-4 solver.
- Initial `mean_us`: 51121.6 (Ice Lake Xeon 6342 node). Final, same code: 28860.6 on the AMD EPYC 7413 (Zen 3) node the benchmark rotated to mid-round. The difference comes from the hardware, not the code.
- Speedup this round: 1.00x. The total remains 171117.7 / 51121.6 = 3.35x over the translation, measured on Ice Lake.

**Final profile (Zen 3 node, `make profile`, 70 solves).** Per-solve times are shown. `iterate` contains the other regions.
- iterate: 28.73 ms (100% of the solve)
- gather_t1_rows_of_B: 13.28 ms (46%)
- gather_t0_tiles_of_A: 12.70 ms (44%)
- gather_y: 1.20 ms (4%)
- gather_t0_reduce: 1.17 ms (4%)
- residual: 0.02 ms (<0.1%)
- bias: 0.02 ms (<0.1%)

The pipeline picks a variant by timing at run time. On Ice Lake it picks B-only (scatter about 29 ms, gather about 18 ms per solve). On Zen 3 it picks the A+B gather variant (B-only takes about 62 ms there).

**Tried this round; each passed verify or was a timing probe, but none was faster, so all were reverted:**
- **Fused single pass over B.** The next state is [s1 | t1], so B^T s1 and B^T s0 can be cached from the previous call. Each thread checks its own rows of the state bitwise against the cache and rebuilds them on a mismatch. It then scatters B_j^T t1_j while row j is still in registers, so B is read once per iteration instead of twice. It verified OK but was slower: 57 vs 51 ms on Ice Lake and 69 vs 62 ms on Zen 3 with B-only forced. The scatter is limited by the read-modify-write of the accumulator, not by streaming B.
- **Column-blocked scatter.** Rows of B are split at column-block boundaries (columns are sorted within a row) so the accumulator slice stays in L1. It was 3–15% slower: the per-segment overhead outweighed the gain.
- **Software prefetch.** Prefetching accumulator lines for upcoming rows made the scatter about 19% slower. NTA or T0 prefetch of the SELL value and index streams made the solve 3–6% slower.
- **Hardware gather (`vgatherdpd`) in the SELL kernel.** It was 1.55x slower even on Zen 3.
- **Folding the t0 reduction into the A pass.** The thread that finishes a row unit's last column tile does the reduction, removing one phase. It was 7% slower.
- **Regular stores for y instead of non-temporal stores.** No change.
- **Sweeps of column-block width, B column blocks and tile rows.** The current values are a local optimum on Zen 3.
- **Data checks.** Values of A and B have full 52-bit mantissas (no exact compression), y and t1 stay dense (no sparsity to skip), and SELL padding is 0.3%.

The kernels sit at about 1.4 cycles per nonzero and appear limited by the memory hierarchy. 16 threads run 2.4x faster than 8, which looks like an L3 capacity effect rather than per-core compute.

Blocked by rules:
- Fuse the next iteration's y = s1 + g'(s1 - s0) into the current B pass, removing the gather_y phase and its barrier | `fom_iterate` gets only this iteration's gamma, and `src/fom_ops.c` may not read the schedule (`fom_sched_*`, `FOM_SCHED_*`) | about 4% (gather_y is 1.2 of 28.7 ms on Zen 3)
- Store B's values in float32 (or another lossy form) to halve the bytes streamed per nonzero | the `fom_apply`/`fom_iterate` checks require exactness to 1e-10 relative, which float32 values cannot meet | unknown, possibly 20–30% since the passes look bandwidth-limited
```
