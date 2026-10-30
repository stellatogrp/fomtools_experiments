# Optimization report

## Timing

```
translated (emitted)   93498.9 us/solve
optimized              33978.8 us/solve
speedup                  2.75x
```

Optimization outcome: **kept (re-verified correct)**.

## Rounds

| round | before (us) | after (us) | gain | kept | agent (s) | outcome |
|---:|---:|---:|---:|:---:|---:|---|
| 1 | 93498.855 | 46726.290 | 50.0% | yes | 2904 | continued |
| 2 | 46570.312 | 40229.575 | 13.6% | yes | 2855 | continued |
| 3 | 39216.942 | 34453.752 | 12.1% | yes | 3640 | continued |
| 4 | 34160.777 | 33978.818 | 0.5% | yes | 2267 | the round gained less than agent_optimize_min_gain |

## Kernel checks

Each `fom_apply_<name>` in the shipped `fom_ops.c`, checked by `make verify` against the package's reference over the same `fom_data.c` matrix. *basis* probes every unit vector, which recovers the routine's matrix column by column (an exact check), plus random vectors; *random* uses random vectors only (the matrix is over the probe budget). A `fused_<block>` row is a `fom_fused_<block>`, checked against the sum of its members: *basis* there probes every unit vector of each source with every schedule value at 0 and with each alone at 1, which recovers every member exactly.

| matrix | shape | nnz | rewritten | check | probes | max error |
|---|---|---|---|---|---|---|
| `t0_state` | 10000x40000 | 2599983 | yes | random | 2 | 0.000e+00 |
| `t0_p_p0` | 10000x10000 | 10000 | yes | random | 2 | 0.000e+00 |
| `t1_state` | 40000x40000 | 40000 | yes | random | 2 | 0.000e+00 |
| `t1_t_t0` | 40000x10000 | 2599983 | yes | random | 2 | 0.000e+00 |
| `su_t_t1` | 40000x40000 | 40000 | yes | random | 2 | 0.000e+00 |
| `fused_t1` | 40000x50000 | 2639983 | yes | random | 2 | 0.000e+00 |
| `step_t0` | 10000x40000 | 2599983 | yes | random | 6 | 0.000e+00 |
| `step_t1` | 40000x50000 | 2639983 | yes | random | 6 | 0.000e+00 |
| `step_su` | 40000x40000 | 40000 | yes | random | 2 | 0.000e+00 |
| `iterate` | 40000x40000 | 5279966 | yes | random | 6 | 6.661e-15 |

### Rule violations

_None: the collected rewrite passed the rules and was kept._

## Blocked by rules

Optimizations the agent considered but did not make because a rule forbade them, in its own words.

| round | idea | rule | expected gain |
|---|---|---|---|
| 1 | Repack the matrices once in `fom_setup` instead of on the first `fom_iterate` call | fom_solver.c may not read FOM_MAT data, and setup may not call fom_iterate or fom_apply | unknown; this only removes a one-time setup cost from the first solve |
| 1 | Add `-march=native` so AVX2/FMA code is generated without hand-written target attributes and run-time dispatch | Makefile and build flags are fixed | unknown, probably small; the kernels are hand-vectorized already |
| 1 | Choose the thread count or CPU placement, e.g. 32 threads to use more L3 slices | OMP_NUM_THREADS comes from the environment | about 1.5x when 32 threads were tried on the login node |
| 1 | Store A in float32 or another compressed format to halve memory traffic on bandwidth-bound nodes | exactness and the data in fom_data.c are fixed (not allowed to approximate the coefficients) | not tested |
| 2 | Store A/B values in a reduced-precision split (float32 plus a correction, or 48-bit truncation) to cut the bytes streamed from L3 per nonzero by 20–40% | `fom_iterate` must apply exactly M (1e-10 check), and the target constraints call for an exact apply | ~10–20% where L3 bandwidth binds (Intel node), unknown on AMD |
| 2 | One persistent OpenMP parallel region across iterations, removing the per-iteration fork/join | the clean version needs `fom_solve` to ask `fom_ops.c` which mode is active, but every function in `src/fom_ops.c` other than the entry points must be `static`; doing it inside the fixed `fom_iterate` signature needs fragile orphaned-construct state | ~1.5–2.5% (about 3.5 µs of 136 µs per iteration) |
| 3 | Store B's values as float32 to cut the dominant memory stream from 9 to 5 bytes per entry, so each thread's slice of B fits in L2 | kernels must apply exactly M (target and verify constraints) | 20-30% (removes the L3 stream and most of the miss stalls) |
| 3 | Use more than the 16 allotted cores (L2 is 1.25 MB per core; 26 or more cores would keep all of B L2-resident) | fixed bench environment and thread count (OMP_NUM_THREADS / cgroup) | unknown, likely large |
| 3 | Run the iteration loop inside one persistent parallel region, which needs solver-level orphaned worksharing across the `fom_iterate` boundary | `fom_iterate` signature and the iteration boundary in `include/`, plus the rule that `fom_solver.c` has no matrix loops | about 1% (fork/join and barrier) |

## What the agent changed

```diff
--- src/fom_solver.c (translated)
+++ src/fom_solver.c (optimized)
@@ -2,6 +2,7 @@
 #include "fom_solver.h"
 #include "fom_data.h"
 #include "fom_ops.h"
+#include "fom_profile.h"
 
 /* acc[0..T0_DIM) holds the hoisted bias t0_p_p0 @ p0; t1 stays all zero and
    serves as the (empty) bias of the t1 step. t1_state and su_t_t1 are
@@ -36,27 +37,91 @@
     reset_state(work);
 }
 
+/* max |y - x| over the state */
+static fom_float max_abs_diff(const fom_float *y, const fom_float *x) {
+    fom_float res = 0;
+    fom_int i;
+    #ifdef _OPENMP
+    #pragma omp parallel for reduction(max:res)
+    #endif
+    for (i = 0; i < FOM_STATE_DIM; ++i) {
+        fom_float d = y[i] - x[i];
+        d = d < 0 ? -d : d;
+        res = d > res ? d : res;
+    }
+    return res;
+}
+
 fom_int fom_solve(FomWorkspace *work) {
     fom_float *x = work->state, *y = work->new_state, *tmp;
     fom_float res = 0;
-    fom_int k, i, n = work->settings.max_iter;
+    fom_int k, n = work->settings.max_iter;
     fom_int use_tol = FOM_USE_TOL && work->settings.check_termination;
     fom_int status = FOM_MAX_ITER_REACHED;
 
-    for (k = 0; k < n; ++k) {
-        fom_step_t0(x, work->acc, &FOM_ACT_t0, work->t0);
-        fom_step_t1(x, work->t0, work->t1, &FOM_ACT_t1, y);
-        res = 0;
+    if (use_tol || n < 2) {
+        for (k = 0; k < n; ++k) {
+            /* t1 == new state (su_t_t1 is the identity), so y takes both. */
+            FOM_PROF_BEGIN(iterate);
+            fom_iterate(x, work->acc, work->t1, work->t1, &FOM_ACT_t0, &FOM_ACT_t1, work->t0, y, y);
+            FOM_PROF_END(iterate);
+            /* the residual is only read on termination: skip it otherwise */
+            if (use_tol || k == n - 1) {
+                FOM_PROF_BEGIN(residual);
+                res = max_abs_diff(y, x);
+                FOM_PROF_END(residual);
+            }
+            tmp = x; x = y; y = tmp;
+            if (use_tol && res <= work->settings.tol) { status = FOM_SOLVED; ++k; break; }
+        }
+    } else {
+        /* A fixed number of iterations: the first on its own (it sets up
+           fom_iterate's fast path), the rest in one parallel region, every
+           thread calling fom_iterate, which shares each iteration's work
+           out among them (no fork and join per iteration). */
+        FOM_PROF_BEGIN(iterate);
+        fom_iterate(x, work->acc, work->t1, work->t1, &FOM_ACT_t0, &FOM_ACT_t1, work->t0, y, y);
+        FOM_PROF_END(iterate);
+        tmp = x; x = y; y = tmp;
         #ifdef _OPENMP
-        #pragma omp parallel for reduction(max:res)
+        #pragma omp parallel private(k)
         #endif
-        for (i = 0; i < FOM_STATE_DIM; ++i) {
-            fom_float d = y[i] - x[i];
-            d = d < 0 ? -d : d;
-            res = d > res ? d : res;
+        {
+            fom_float *xa = x, *ya = y, *ta;
+            fom_int i;
+            for (k = 1; k < n; ++k) {
+                FOM_PROF_BEGIN(iterate);
+                fom_iterate(xa, work->acc, work->t1, work->t1, &FOM_ACT_t0, &FOM_ACT_t1, work->t0, ya, ya);
+#ifdef FOM_PROFILE
+                #ifdef _OPENMP
+                #pragma omp master
+                #endif
+                FOM_PROF_END(iterate);
+#endif
+                ta = xa; xa = ya; ya = ta;
+            }
+            /* residual of the last iteration: new state xa, previous ya */
+            FOM_PROF_BEGIN(residual);
+            #ifdef _OPENMP
+            #pragma omp for reduction(max:res)
+            #endif
+            for (i = 0; i < FOM_STATE_DIM; ++i) {
+                fom_float d = xa[i] - ya[i];
+                d = d < 0 ? -d : d;
+                res = d > res ? d : res;
+            }
+#ifdef FOM_PROFILE
+            #ifdef _OPENMP
+            #pragma omp master
+            #endif
+            FOM_PROF_END(residual);
+#endif
+            #ifdef _OPENMP
+            #pragma omp master
+            #endif
+            x = xa;
         }
-        tmp = x; x = y; y = tmp;
-        if (use_tol && res <= work->settings.tol) { status = FOM_SOLVED; ++k; break; }
+        k = n;
     }
     if (x != work->state) memcpy(work->state, x, sizeof(work->state));
     memcpy(work->solution.x, work->state, sizeof(work->solution.x));
--- src/fom_ops.c (translated)
+++ src/fom_ops.c (optimized)
@@ -1,5 +1,7 @@
 #include "fom_ops.h"
 #include "fom_data.h"
+#include "fom_profile.h"
+#include <math.h>
 
 void fom_apply_t0_state(const fom_float *x, fom_float *out) {
     fom_int i, p;
@@ -130,8 +132,1073 @@
     }
 }
 
+/* Fast path for fom_iterate. t1_state and su_t_t1 are identities (checked
+   on first use), so with A = t0_state, B = t1_t_t0 an iteration is
+   t0 = act(b0 + A x), then t1 = act(b1 + x + B t0), written to t1 and the
+   new state at once.
+
+   Layout. On first use A and B are repacked (same values, reordered, no
+   arithmetic) into a cache-blocked sliced-ELL layout: rows are split into
+   chunks, columns into blocks (2^IT_SHB entries of t0 for B, one slab of
+   IT_CB2 entries of x for A), so the slice of the source vector a block
+   gathers from stays in L1. Within
+   each (chunk, block) rows are sorted by their entry count in that block
+   and taken 8 at a time; a group stores its 8 rows' entries column-major,
+   zero-padded to the longest (~1% padding here). Kernels run a branch-free
+   8-lane loop over a sequential stream with 16-bit block-local columns.
+
+   Two modes, chosen once in it_check:
+   2: two gather passes, over A (t0) and over B (t1). Fastest where both
+      repacked matrices (~52 MB) stay in cache. A is cut by columns into
+      one slab per thread, slab t covering the rows of x that thread t
+      wrote in its last t1 pass. Each group of IT_P = 4 neighbouring
+      threads (one core complex on Zen 2) gathers its slabs into a partial
+      of A x, thread t taking a quarter of the rows, and a short reduce
+      sums the 4 partials. x then moves only within a group, instead of
+      all of x (320 KB) to every thread.
+   1: B only. In this data A == cinv * B^T for one scalar (checked entry by
+      entry), so A x = cinv * B^T x is a scatter over B's layout. The t1
+      gather runs one sub-chunk (IT_CB rows) at a time; while that slice of B is
+      still in L2 the new state is scattered through it, giving the partials
+      of B^T x_new for the next call. They are reused only if that call is
+      handed exactly x_new (else recomputed). B is read from L3 once per
+      iteration, and the sub-chunks are swept in alternating directions so
+      the part of a thread's slice still in L2 is read first.
+      In this data B also has at most one entry per stratum of ~154
+      columns, so mode 1 normally uses the strata layout (it_st_build):
+      8-bit offsets, uniform rows, and with AVX-512VL the gather of
+      sub-chunk i+1 runs in one loop with the scatter of sub-chunk i. */
+#if defined(__GNUC__) && defined(__x86_64__)
+#define IT_AVX2 1
+#include <immintrin.h>
+static int it_avx2 = 0;
+static int it_avx512 = 0;
+#endif
+#if defined(__SSE2__)
+#define IT_SSE2 1
+#include <emmintrin.h>
+#endif
+#include <string.h>
+#ifdef __GNUC__
+#define IT_ALIGN __attribute__((aligned(64)))
+#else
+#define IT_ALIGN
+#endif
+#define IT_SHB 12 /* column block of B: 2^IT_SHB entries of t0 (fused mode) */
+#define IT_SHB2 11 /* the same in two-gather mode */
+#define IT_BW (1 << IT_SHB) /* widest block: IT_SHB2 <= IT_SHB, IT_CB2 <= IT_BW */
+#define IT_NSUP 16 /* super-chunks of rows: one per thread, own partials
+                      (16: the t0 slices below are cut with >> 4) */
+#define IT_NSUB 5 /* sub-chunks per super-chunk */
+#define IT_NCH (IT_NSUP * IT_NSUB)
+#define IT_CB 500  /* rows per sub-chunk: IT_NCH * IT_CB == t1_t_t0 rows */
+#define IT_CB2 2500 /* rows per chunk of B in two-gather mode (IT_NSUP chunks) */
+#define IT_KB ((FOM_MAT_t1_t_t0_COLS + (1 << IT_SHB2) - 1) >> IT_SHB2) /* blocks, at most */
+#define IT_LP 2 /* log2 of the threads sharing one column group of A's slabs */
+#define IT_P (1 << IT_LP)
+#define IT_RP (FOM_MAT_t0_state_ROWS >> IT_LP) /* rows of A per thread of a group */
+#define IT_GS ((IT_RP + 7) >> 3) /* groups per (row chunk, slab) of A */
+#define IT_GB1 (IT_NCH * ((IT_CB + 7) >> 3))    /* groups per block, fused mode */
+#define IT_GB2 (IT_NSUP * ((IT_CB2 + 7) >> 3)) /* groups per block, two-gather mode */
+#define IT_GBT (IT_GB1 > IT_GB2 ? IT_GB1 : IT_GB2)
+#define IT_CAPA (FOM_MAT_t0_state_NNZ + (FOM_MAT_t0_state_NNZ >> 3) + 64)
+#define IT_CAPB (FOM_MAT_t1_t_t0_NNZ + (FOM_MAT_t1_t_t0_NNZ >> 3) + 64)
+
+typedef struct {
+    fom_float *val;          /* padded entries, 8 per step */
+    unsigned short *col;     /* block-local column of each entry */
+    unsigned short *len;     /* steps of each group, [chunk][block][group] */
+    unsigned short *perm;    /* chunk-local row of each lane, 8 per group */
+    fom_int *start;          /* first entry of each chunk */
+    fom_int rows, cols, cr, nblk, ngrp, cap, sh, nch;
+    fom_int bw;              /* column block width: 2^sh, or the slab width */
+    fom_int *bst;            /* first entry of each [chunk][block], or NULL */
+} ItSell;
+
+static fom_float it_va[IT_CAPA] IT_ALIGN;
+static unsigned short it_ca[IT_CAPA];
+static unsigned short it_la[IT_P * IT_NSUP * IT_GS];
+static unsigned short it_pa[IT_P * IT_NSUP * IT_GS * 8];
+static fom_int it_sa[IT_P + 1];
+static fom_float it_vb[IT_CAPB] IT_ALIGN;
+static unsigned short it_cb[IT_CAPB];
+static unsigned short it_lb[IT_KB * IT_GBT];
+static unsigned short it_pb[IT_KB * IT_GBT * 8];
+static fom_int it_sb[IT_NCH + 1];
+static ItSell it_B = {it_vb, it_cb, it_lb, it_pb, it_sb, FOM_MAT_t1_t_t0_ROWS, FOM_MAT_t1_t_t0_COLS, IT_CB, (FOM_MAT_t1_t_t0_COLS + (1 << IT_SHB) - 1) >> IT_SHB, (IT_CB + 7) >> 3, IT_CAPB, IT_SHB, IT_NCH, 1 << IT_SHB, 0};
+/* Two-gather mode: A cut into IT_NSUP column slabs (blocks), slab t
+   holding the columns of x that super-chunk t of B produces (the rows
+   thread t writes), and into IT_P row chunks. Threads t of one group of
+   IT_P share the group's IT_P slabs, thread t taking row chunk t mod IT_P. */
+static fom_int it_bs[IT_P * IT_NSUP + 1];
+static ItSell it_S = {it_va, it_ca, it_la, it_pa, it_sa, FOM_MAT_t0_state_ROWS, FOM_MAT_t0_state_COLS, IT_RP, IT_NSUP, IT_GS, IT_CAPA, 0, IT_P, IT_CB2, it_bs};
+static fom_float it_sacc[IT_NSUP][IT_RP + 1];
+
+/* Partials, one row of IT_PST (a multiple of 4 KB) per super-chunk. The
+   rows start at it_pbase, placed so that a row sits 2 KB (mod 4 KB) from
+   t0: the fused pass loads t0 and stores the partials at the same offsets
+   inside a stratum, which would otherwise falsely alias (4K aliasing). */
+#define IT_PST ((FOM_MAT_t1_t_t0_COLS + 511) & ~511)
+static fom_float it_pbuf[IT_NSUP * IT_PST + 512] IT_ALIGN;
+static fom_float *it_pbase = it_pbuf;
+/* Memo across calls: the partials hold the partials of B^T it_memo_x. */
+static fom_float it_memo_x[FOM_MAT_t1_t_t0_ROWS];
+static int it_memo_ok = 0;
+/* order of the last t1 pass over each thread's sub-chunks (1: backward) */
+static int it_dir = 0;
+/* 1: fused single read of B (scatter for A x); 2: two gather passes. */
+static int it_mode = 2;
+static fom_float it_cinv;
+
+/* build scratch */
+static fom_int it_cursor[FOM_MAT_t1_t_t0_ROWS];
+#define IT_RMAX (FOM_MAT_t0_state_ROWS > IT_CB2 ? FOM_MAT_t0_state_ROWS : IT_CB2)
+static fom_int it_pos[IT_RMAX];
+static fom_int it_seg[IT_RMAX];
+static fom_int it_ord[((IT_RMAX + 7) >> 3) * 8];
+static fom_int it_cnt[IT_BW + 2];
+static int it_ready = 0; /* 0 unchecked, 1 fast path, -1 generic */
+
+/* Phase timers inside the parallel region (profile build only): the master
+   thread stamps the time after each work-shared loop's barrier. */
+#ifdef _OPENMP
+#include <omp.h>
+#define IT_MASTER (omp_get_thread_num() == 0)
+#else
+#define IT_MASTER 1
+#endif
+#ifdef FOM_PROFILE
+#define IT_STAMP(t) do { if (IT_MASTER) t = fom_now_ns(); } while (0)
+/* master thread's split of the fused-mode t1 pass */
+static double it_ph[4];
+#define IT_PH(k, t) do { if (IT_MASTER) { double n_ = fom_now_ns(); it_ph[k] += n_ - t; t = n_; } } while (0)
+#define IT_NOWAIT
+#else
+#define IT_STAMP(t) ((void)0)
+#define IT_PH(k, t) ((void)0)
+#define IT_NOWAIT nowait
+#endif
+
+static int it_build(ItSell *m, const fom_int *rp, const fom_int *ci, const fom_float *vx) {
+    fom_int ch, k, g, i, j, jj, l, o, q = 0, lo, hi, n, L;
+    fom_int ng8 = m->ngrp * 8;
+    if (rp[0] != 0 || m->cr * m->nch != m->rows) return -1;
+    for (i = 0; i < m->rows; ++i) {
+        if (rp[i + 1] < rp[i]) return -1;
+        for (j = rp[i]; j < rp[i + 1]; ++j) {
+            if (ci[j] < 0 || ci[j] >= m->cols) return -1;
+            if (j > rp[i] && ci[j] <= ci[j - 1]) return -1;
+        }
+    }
+    for (ch = 0; ch < m->nch; ++ch) {
+        fom_int r0 = ch * m->cr;
+        m->start[ch] = q;
+        for (i = 0; i < m->cr; ++i) it_pos[i] = rp[r0 + i];
+        for (k = 0; k < m->nblk; ++k) {
+            unsigned short *len = m->len + (ch * m->nblk + k) * m->ngrp;
+            unsigned short *perm = m->perm + (ch * m->nblk + k) * ng8;
+            lo = k * m->bw; hi = lo + m->bw;
+            if (m->bw > IT_BW) return -1;
+            if (m->bst) m->bst[ch * m->nblk + k] = q;
+            /* entry counts in this block, then rows sorted by count, longest first */
+            for (j = 0; j <= IT_BW + 1; ++j) it_cnt[j] = 0;
+            for (i = 0; i < m->cr; ++i) {
+                n = 0;
+                while (it_pos[i] + n < rp[r0 + i + 1] && ci[it_pos[i] + n] < hi) ++n;
+                it_seg[i] = n;
+                it_cnt[IT_BW - n + 1]++;
+            }
+            for (j = 1; j <= IT_BW + 1; ++j) it_cnt[j] += it_cnt[j - 1];
+            for (i = 0; i < m->cr; ++i) it_ord[it_cnt[IT_BW - it_seg[i]]++] = i;
+            for (i = m->cr; i < ng8; ++i) it_ord[i] = -1;
+            for (g = 0; g < m->ngrp; ++g) {
+                L = it_ord[8 * g] >= 0 ? it_seg[it_ord[8 * g]] : 0;
+                if (L > 65535 || q + 8 * L > m->cap) return -1;
+                len[g] = (unsigned short)L;
+                for (l = 0; l < 8; ++l) {
+                    i = it_ord[8 * g + l];
+                    perm[8 * g + l] = (unsigned short)(i >= 0 ? i : m->cr);
+                    n = i >= 0 ? it_seg[i] : 0;
+                    /* fused mode: lane l starts l/8 of the way into its row
+                       segment (wrapping), so the 8 lanes of a step sit at
+                       spread-out columns and the scatter's stores rarely alias */
+                    o = it_mode == 1 ? (l * n) >> 3 : 0;
+                    for (j = 0; j < L; ++j) {
+                        jj = j + o >= n ? j + o - n : j + o;
+                        m->val[q + 8 * j + l] = j < n ? vx[it_pos[i] + jj] : 0.0;
+                        m->col[q + 8 * j + l] = (unsigned short)(j < n ? ci[it_pos[i] + jj] - lo : 0);
+                    }
+                    if (i >= 0) it_pos[i] += n;
+                }
+                q += 8 * L;
+            }
+        }
+        for (i = 0; i < m->cr; ++i) if (it_pos[i] != rp[r0 + i + 1]) return -1;
+    }
+    m->start[m->nch] = q;
+    if (m->bst) m->bst[m->nch * m->nblk] = q;
+    return 1;
+}
+
+/* Division-free reciprocal: frexp scaling plus Newton y <- y (2 - m y). */
+static fom_float it_recip(fom_float a) {
+    int e, k;
+    fom_float m = frexp(a < 0.0 ? -a : a, &e), y = 2.0 - m;
+    for (k = 0; k < 7; ++k) y = y * (2.0 - m * y);
+    y = ldexp(y, -e);
+    return a < 0.0 ? -y : y;
+}
+
+/* Is t0_state == cinv * t1_t_t0^T, entry by entry (up to rounding)? */
+static int it_relation(void) {
+    fom_int i, p, q, j;
+    fom_float a, b, d, eps = 1.0;
+    for (i = 0; i < 40; ++i) eps *= 0.5;
+    if (FOM_MAT_t1_t_t0_ROWS != FOM_MAT_t0_state_COLS || FOM_MAT_t1_t_t0_COLS != FOM_MAT_t0_state_ROWS) return 0;
+    if (FOM_MAT_t1_t_t0_NNZ != FOM_MAT_t0_state_NNZ || FOM_MAT_t0_state_NNZ < 1) return 0;
+    if (FOM_MAT_t0_state_ROWPTR[0] != 0 || FOM_MAT_t0_state_ROWPTR[1] < 1) return 0;
+    j = FOM_MAT_t0_state_COLIDX[0];
+    if (j < 0 || j >= FOM_MAT_t1_t_t0_ROWS) return 0;
+    q = FOM_MAT_t1_t_t0_ROWPTR[j];
+    if (q >= FOM_MAT_t1_t_t0_ROWPTR[j + 1] || FOM_MAT_t1_t_t0_COLIDX[q] != 0) return 0;
+    b = FOM_MAT_t1_t_t0_VALS[q];
+    if (b == 0.0) return 0;
+    it_cinv = FOM_MAT_t0_state_VALS[0] * it_recip(b);
+    for (j = 0; j < FOM_MAT_t1_t_t0_ROWS; ++j) it_cursor[j] = FOM_MAT_t1_t_t0_ROWPTR[j];
+    for (i = 0; i < FOM_MAT_t0_state_ROWS; ++i)
+        for (p = FOM_MAT_t0_state_ROWPTR[i]; p < FOM_MAT_t0_state_ROWPTR[i + 1]; ++p) {
+            j = FOM_MAT_t0_state_COLIDX[p];
+            if (j < 0 || j >= FOM_MAT_t1_t_t0_ROWS) return 0;
+            q = it_cursor[j]++;
+            if (q >= FOM_MAT_t1_t_t0_ROWPTR[j + 1] || FOM_MAT_t1_t_t0_COLIDX[q] != i) return 0;
+            a = FOM_MAT_t0_state_VALS[p];
+            d = a - it_cinv * FOM_MAT_t1_t_t0_VALS[q];
+            d = d < 0.0 ? -d : d;
+            if (d > eps * (a < 0.0 ? -a : a)) return 0;
+        }
+    for (j = 0; j < FOM_MAT_t1_t_t0_ROWS; ++j)
+        if (it_cursor[j] != FOM_MAT_t1_t_t0_ROWPTR[j + 1]) return 0;
+    return 1;
+}
+
+/* Strata layout (fused mode). In this data every row of B holds at most one
+   entry per stratum, NS consecutive column ranges [sb[s], sb[s] + 255]
+   (sb[s] = floor(s * cols / NS), NS = the longest row; entries are given the
+   first free stratum that reaches them, checked entry by entry at setup,
+   else the layout above is used). Step s of a
+   group of 8 consecutive rows is then stratum s for all 8 lanes, so a step
+   stores 8 values and 8 one-byte offsets from the shared base sb[s] (9
+   bytes an entry instead of 10, empty slots zero-padded: ~0.1% here), and
+   no per-group lengths or row permutation are needed. Strata are cut into
+   blocks of IT_SPB, so the slice of the source vector a block touches stays
+   in L1. Entry order: [chunk][block][group][step][lane]. */
+#define IT_NSMAX 128
+#define IT_SPB 13
+#define IT_GR ((IT_CB + 7) >> 3) /* groups per sub-chunk (last one padded) */
+#define IT_CBP (IT_GR * 8)
+static int it_strata = 0;
+static fom_int it_ns, it_nkb;
+static fom_int it_stb[IT_NSMAX + 1];
+static fom_int it_kst[IT_NSMAX + 1];
+
+static int it_st_build(void) {
+    const fom_int *rp = FOM_MAT_t1_t_t0_ROWPTR, *ci = FOM_MAT_t1_t_t0_COLIDX;
+    const fom_float *vx = FOM_MAT_t1_t_t0_VALS;
+    unsigned char *off = (unsigned char *)it_cb;
+    fom_int ns = 0, i, j, b, k, ch, p, s, prev, cols = FOM_MAT_t1_t_t0_COLS;
+    for (i = 0; i < FOM_MAT_t1_t_t0_ROWS; ++i) if (rp[i + 1] - rp[i] > ns) ns = rp[i + 1] - rp[i];
+    if (ns < 1 || ns > IT_NSMAX || IT_NCH * IT_CBP * ns > IT_CAPB) return -1;
+    /* sb[j] = floor(j cols / ns), division-free */
+    for (j = 0, b = 0; j <= ns; ++j) {
+        while ((b + 1) * ns <= j * cols) ++b;
+        it_stb[j] = b;
+        if (j > 0 && it_stb[j] - it_stb[j - 1] > 256) return -1;
+    }
+    if (it_stb[ns] != cols) return -1;
+    for (k = 0; k * IT_SPB < ns; ++k) it_kst[k] = k * IT_SPB;
+    it_nkb = k; it_kst[k] = ns; it_ns = ns;
+    for (i = 0; i < IT_NCH * IT_CBP * ns; ++i) { it_vb[i] = 0.0; off[i] = 0; }
+    for (ch = 0; ch < IT_NCH; ++ch) {
+        fom_int c0 = ch * IT_CBP * ns;
+        for (i = 0; i < IT_CB; ++i) {
+            fom_int r = ch * IT_CB + i, g = i >> 3, l = i & 7;
+            prev = -1; s = 0; k = 0;
+            for (p = rp[r]; p < rp[r + 1]; ++p) {
+                fom_int c = ci[p], q;
+                if (c < 0 || c >= cols) return -1;
+                /* first free stratum whose closed range [stb[s], stb[s+1]]
+                   reaches c (neighbouring ranges share an end column) */
+                while (s < ns && c > it_stb[s + 1]) ++s;
+                if (s <= prev) s = prev + 1;
+                if (s >= ns || c < it_stb[s] || c - it_stb[s] > 255) return -1;
+                prev = s;
+                while (s >= it_kst[k + 1]) ++k;
+                q = c0 + IT_CBP * it_kst[k] + g * 8 * (it_kst[k + 1] - it_kst[k]) + (s - it_kst[k]) * 8 + l;
+                it_vb[q] = vx[p];
+                off[q] = (unsigned char)(c - it_stb[s]);
+            }
+        }
+    }
+    return 1;
+}
+
+static int it_check(void) {
+    fom_int i;
+    if (FOM_MAT_t1_t_t0_ROWS != 40000 || IT_CB >= 65535 || FOM_MAT_t0_state_ROWS >= 65535 || IT_CB2 > IT_BW) return -1;
+    for (i = 0; i < 40000; ++i) {
+        if (FOM_MAT_t1_state_ROWPTR[i] != i || FOM_MAT_t1_state_COLIDX[i] != i || FOM_MAT_t1_state_VALS[i] != 1.0) return -1;
+        if (FOM_MAT_su_t_t1_ROWPTR[i] != i || FOM_MAT_su_t_t1_COLIDX[i] != i || FOM_MAT_su_t_t1_VALS[i] != 1.0) return -1;
+    }
+    if (FOM_MAT_t1_state_ROWPTR[40000] != 40000 || FOM_MAT_su_t_t1_ROWPTR[40000] != 40000) return -1;
+#ifdef IT_AVX2
+    __builtin_cpu_init();
+    it_avx2 = __builtin_cpu_supports("avx2") && __builtin_cpu_supports("fma")
+        && __builtin_cpu_supports("bmi") && __builtin_cpu_supports("bmi2");
+    it_avx512 = it_avx2 && __builtin_cpu_supports("avx512f") && __builtin_cpu_supports("avx512vl");
+#endif
+    /* Mode. The fused pass reads B once per iteration but does a scatter
+       (about twice the memory operations of a gather), so it pays only when
+       the two matrices (~52 MB repacked) stream from DRAM, not from cache:
+       on Intel server parts one shared L3 (tens of MB) serves all threads,
+       while AMD Zen parts give each 8-core complex its own 32 MB L3. */
+    it_mode = 2;
+#if defined(IT_FORCE_MODE)
+    it_mode = IT_FORCE_MODE;
+#elif defined(IT_AVX2)
+    if (__builtin_cpu_is("intel")) it_mode = 1;
+#endif
+    if (it_mode == 1 && !it_relation()) it_mode = 2;
+    if (it_mode == 2) { /* one chunk of B per thread, as for A */
+        it_B.cr = IT_CB2; it_B.nch = IT_NSUP; it_B.ngrp = (IT_CB2 + 7) >> 3;
+        it_B.sh = IT_SHB2; it_B.bw = 1 << IT_SHB2; it_B.nblk = (FOM_MAT_t1_t_t0_COLS + (1 << IT_SHB2) - 1) >> IT_SHB2;
+    }
+    if (it_mode == 2 && (IT_NSUP * IT_CB2 != FOM_MAT_t0_state_COLS || (IT_RP << IT_LP) != FOM_MAT_t0_state_ROWS || IT_P > IT_NSUP)) return -1;
+    if (it_mode == 2 && it_build(&it_S, FOM_MAT_t0_state_ROWPTR, FOM_MAT_t0_state_COLIDX, FOM_MAT_t0_state_VALS) < 0) return -1;
+    if (it_mode == 1 && it_st_build() > 0) { it_strata = 1; return 1; }
+    if (it_build(&it_B, FOM_MAT_t1_t_t0_ROWPTR, FOM_MAT_t1_t_t0_COLIDX, FOM_MAT_t1_t_t0_VALS) < 0) return -1;
+    return 1;
+}
+
+/* Scatter form over the same layout: w += M[rows of chunk ch,:]^T x[rows]
+   (w is a private buffer of M's column count). Within a step the 8
+   lanes are different rows, so they may hit the same column: the plain
+   sequential read-modify-writes keep that exact. */
+static inline void it_sell_scatter(const ItSell *m, fom_int ch, const fom_float *x, fom_float *w) {
+    fom_int k, g, j, l, q = m->start[ch];
+    const fom_float *restrict v = m->val;
+    const unsigned short *restrict c = m->col;
+    const fom_float *xc = x + ch * m->cr;
+    for (k = 0; k < m->nblk; ++k) {
+        fom_float *restrict wk = w + (k << m->sh);
+        const unsigned short *len = m->len + (ch * m->nblk + k) * m->ngrp;
+        const unsigned short *perm = m->perm + (ch * m->nblk + k) * m->ngrp * 8;
+        for (g = 0; g < m->ngrp; ++g) {
+            fom_int L = len[g];
+#ifdef IT_SSE2
+            fom_float xl[8] IT_ALIGN;
+            __m128d x01, x23, x45, x67;
+            for (l = 0; l < 8; ++l) xl[l] = perm[8 * g + l] < m->cr ? xc[perm[8 * g + l]] : 0.0;
+            x01 = _mm_load_pd(xl); x23 = _mm_load_pd(xl + 2);
+            x45 = _mm_load_pd(xl + 4); x67 = _mm_load_pd(xl + 6);
+            /* products two at a time, then the 8 read-modify-writes in lane
+               order (16-bit indices four per 64-bit load) */
+            for (j = 0; j < L; ++j, q += 8) {
+                unsigned long long w0, w1;
+                __m128d p01 = _mm_mul_pd(_mm_load_pd(v + q), x01);
+                __m128d p23 = _mm_mul_pd(_mm_load_pd(v + q + 2), x23);
+                __m128d p45 = _mm_mul_pd(_mm_load_pd(v + q + 4), x45);
+                __m128d p67 = _mm_mul_pd(_mm_load_pd(v + q + 6), x67);
+                memcpy(&w0, c + q, 8);
+                memcpy(&w1, c + q + 4, 8);
+                wk[w0 & 0xffff] += _mm_cvtsd_f64(p01);
+                wk[(w0 >> 16) & 0xffff] += _mm_cvtsd_f64(_mm_unpackhi_pd(p01, p01));
+                wk[(w0 >> 32) & 0xffff] += _mm_cvtsd_f64(p23);
+                wk[w0 >> 48] += _mm_cvtsd_f64(_mm_unpackhi_pd(p23, p23));
+                wk[w1 & 0xffff] += _mm_cvtsd_f64(p45);
+                wk[(w1 >> 16) & 0xffff] += _mm_cvtsd_f64(_mm_unpackhi_pd(p45, p45));
+                wk[(w1 >> 32) & 0xffff] += _mm_cvtsd_f64(p67);
+                wk[w1 >> 48] += _mm_cvtsd_f64(_mm_unpackhi_pd(p67, p67));
+            }
+#else
+            fom_float xl[8];
+            for (l = 0; l < 8; ++l) xl[l] = perm[8 * g + l] < m->cr ? xc[perm[8 * g + l]] : 0.0;
+            for (j = 0; j < L; ++j, q += 8) {
+                wk[c[q]] += v[q] * xl[0];
+                wk[c[q + 1]] += v[q + 1] * xl[1];
+                wk[c[q + 2]] += v[q + 2] * xl[2];
+                wk[c[q + 3]] += v[q + 3] * xl[3];
+                wk[c[q + 4]] += v[q + 4] * xl[4];
+                wk[c[q + 5]] += v[q + 5] * xl[5];
+                wk[c[q + 6]] += v[q + 6] * xl[6];
+                wk[c[q + 7]] += v[q + 7] * xl[7];
+            }
+#endif
+        }
+    }
+}
+
+/* acc[0..cr) = rows of chunk ch of M times x (acc[cr] takes the padding). */
+static inline void it_sell_chunk(const ItSell *m, fom_int ch, const fom_float *x, fom_float *acc) {
+    fom_int k, g, j, l, q = m->start[ch];
+    const fom_float *v = m->val;
+    const unsigned short *c = m->col;
+    for (l = 0; l <= m->cr; ++l) acc[l] = 0.0;
+    for (k = 0; k < m->nblk; ++k) {
+        const fom_float *xk = x + (k << m->sh);
+        const unsigned short *len = m->len + (ch * m->nblk + k) * m->ngrp;
+        const unsigned short *perm = m->perm + (ch * m->nblk + k) * m->ngrp * 8;
+        for (g = 0; g < m->ngrp; ++g) {
+#ifdef IT_SSE2
+            __m128d a01 = _mm_setzero_pd(), a23 = _mm_setzero_pd(), a45 = _mm_setzero_pd(), a67 = _mm_setzero_pd();
+            fom_float a0, a1, a2, a3, a4, a5, a6, a7;
+            fom_int L = len[g];
+            /* 16-bit indices four per 64-bit load; values and x in pairs */
+            for (j = 0; j < L; ++j, q += 8) {
+                unsigned long long w0, w1;
+                __m128d x01, x23, x45, x67;
+                memcpy(&w0, c + q, 8);
+                memcpy(&w1, c + q + 4, 8);
+                x01 = _mm_loadh_pd(_mm_load_sd(xk + (w0 & 0xffff)), xk + ((w0 >> 16) & 0xffff));
+                x23 = _mm_loadh_pd(_mm_load_sd(xk + ((w0 >> 32) & 0xffff)), xk + (w0 >> 48));
+                x45 = _mm_loadh_pd(_mm_load_sd(xk + (w1 & 0xffff)), xk + ((w1 >> 16) & 0xffff));
+                x67 = _mm_loadh_pd(_mm_load_sd(xk + ((w1 >> 32) & 0xffff)), xk + (w1 >> 48));
+                a01 = _mm_add_pd(a01, _mm_mul_pd(_mm_load_pd(v + q), x01));
+                a23 = _mm_add_pd(a23, _mm_mul_pd(_mm_load_pd(v + q + 2), x23));
+                a45 = _mm_add_pd(a45, _mm_mul_pd(_mm_load_pd(v + q + 4), x45));
+                a67 = _mm_add_pd(a67, _mm_mul_pd(_mm_load_pd(v + q + 6), x67));
+            }
+            a0 = _mm_cvtsd_f64(a01); a1 = _mm_cvtsd_f64(_mm_unpackhi_pd(a01, a01));
+            a2 = _mm_cvtsd_f64(a23); a3 = _mm_cvtsd_f64(_mm_unpackhi_pd(a23, a23));
+            a4 = _mm_cvtsd_f64(a45); a5 = _mm_cvtsd_f64(_mm_unpackhi_pd(a45, a45));
+            a6 = _mm_cvtsd_f64(a67); a7 = _mm_cvtsd_f64(_mm_unpackhi_pd(a67, a67));
+#else
+            fom_float a0 = 0.0, a1 = 0.0, a2 = 0.0, a3 = 0.0, a4 = 0.0, a5 = 0.0, a6 = 0.0, a7 = 0.0;
+            fom_int L = len[g];
+            for (j = 0; j < L; ++j, q += 8) {
+                a0 += v[q] * xk[c[q]];
+                a1 += v[q + 1] * xk[c[q + 1]];
+                a2 += v[q + 2] * xk[c[q + 2]];
+                a3 += v[q + 3] * xk[c[q + 3]];
+                a4 += v[q + 4] * xk[c[q + 4]];
+                a5 += v[q + 5] * xk[c[q + 5]];
+                a6 += v[q + 6] * xk[c[q + 6]];
+                a7 += v[q + 7] * xk[c[q + 7]];
+            }
+#endif
+            acc[perm[8 * g]] += a0; acc[perm[8 * g + 1]] += a1;
+            acc[perm[8 * g + 2]] += a2; acc[perm[8 * g + 3]] += a3;
+            acc[perm[8 * g + 4]] += a4; acc[perm[8 * g + 5]] += a5;
+            acc[perm[8 * g + 6]] += a6; acc[perm[8 * g + 7]] += a7;
+        }
+    }
+}
+
+#ifdef IT_AVX2
+/* it_sell_chunk with 256-bit values and fma; x is still loaded in pairs
+   (gathers are slower than scalar loads here), then joined into 4 lanes. */
+__attribute__((target("avx2,fma,bmi,bmi2")))
+static void it_sell_chunk_avx2(const ItSell *m, fom_int ch, const fom_float *x, fom_float *acc) {
+    fom_int k, g, j, l, q = m->start[ch];
+    const fom_float *v = m->val;
+    const unsigned short *c = m->col;
+    double tmp[8];
+    for (l = 0; l <= m->cr; ++l) acc[l] = 0.0;
+    for (k = 0; k < m->nblk; ++k) {
+        const fom_float *xk = x + (k << m->sh);
+        const unsigned short *len = m->len + (ch * m->nblk + k) * m->ngrp;
+        const unsigned short *perm = m->perm + (ch * m->nblk + k) * m->ngrp * 8;
+        for (g = 0; g < m->ngrp; ++g) {
+            __m256d a0 = _mm256_setzero_pd(), a1 = _mm256_setzero_pd();
+            fom_int L = len[g];
+            for (j = 0; j < L; ++j, q += 8) {
+                unsigned long long w0, w1;
+                __m128d x01, x23, x45, x67;
+                memcpy(&w0, c + q, 8);
+                memcpy(&w1, c + q + 4, 8);
+                x01 = _mm_loadh_pd(_mm_load_sd(xk + (w0 & 0xffff)), xk + ((w0 >> 16) & 0xffff));
+                x23 = _mm_loadh_pd(_mm_load_sd(xk + ((w0 >> 32) & 0xffff)), xk + (w0 >> 48));
+                x45 = _mm_loadh_pd(_mm_load_sd(xk + (w1 & 0xffff)), xk + ((w1 >> 16) & 0xffff));
+                x67 = _mm_loadh_pd(_mm_load_sd(xk + ((w1 >> 32) & 0xffff)), xk + (w1 >> 48));
+                a0 = _mm256_fmadd_pd(_mm256_load_pd(v + q), _mm256_set_m128d(x23, x01), a0);
+                a1 = _mm256_fmadd_pd(_mm256_load_pd(v + q + 4), _mm256_set_m128d(x67, x45), a1);
+            }
+            _mm256_storeu_pd(tmp, a0);
+            _mm256_storeu_pd(tmp + 4, a1);
+            for (l = 0; l < 8; ++l) acc[perm[8 * g + l]] += tmp[l];
+        }
+    }
+}
+#endif
+
+
+
+/* acc[0..IT_CBP) = rows of sub-chunk ch of B times x (padded rows get 0) */
+static void it_st_gather(fom_int ch, const fom_float *x, fom_float *acc) {
+    fom_int k, g, s, l, ns = it_ns;
+    const fom_float *v = it_vb + ch * IT_CBP * ns;
+    const unsigned char *o = (const unsigned char *)it_cb + ch * IT_CBP * ns;
+    for (k = 0; k < it_nkb; ++k) {
+        fom_int s0 = it_kst[k], s1 = it_kst[k + 1];
+        for (g = 0; g < IT_GR; ++g) {
+            fom_float a[8] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
+            for (s = s0; s < s1; ++s, v += 8, o += 8) {
+                const fom_float *xs = x + it_stb[s];
+                for (l = 0; l < 8; ++l) a[l] += v[l] * xs[o[l]];
+            }
+            for (l = 0; l < 8; ++l) acc[8 * g + l] = (k ? acc[8 * g + l] : 0.0) + a[l];
+        }
+    }
+}
+
+#ifdef IT_AVX2
+__attribute__((target("avx2,fma,bmi,bmi2,prfchw")))
+static void it_st_gather_avx2(fom_int ch, const fom_float *x, fom_float *acc, const char *pw, const char *pe) {
+    fom_int k, g, s, ns = it_ns;
+    const fom_float *v = it_vb + ch * IT_CBP * ns;
+    const unsigned char *o = (const unsigned char *)it_cb + ch * IT_CBP * ns;
+    for (k = 0; k < it_nkb; ++k) {
+        fom_int s0 = it_kst[k], s1 = it_kst[k + 1];
+        for (g = 0; g < IT_GR; ++g) {
+            __m256d a0 = _mm256_setzero_pd(), a1 = _mm256_setzero_pd();
+            /* [pw, pe): lines to take ownership of meanwhile, 4 per group */
+            if (pw < pe) {
+                __builtin_prefetch(pw, 1, 3); __builtin_prefetch(pw + 64, 1, 3);
+                __builtin_prefetch(pw + 128, 1, 3); __builtin_prefetch(pw + 192, 1, 3);
+                pw += 256;
+            }
+            for (s = s0; s < s1; ++s, v += 8, o += 8) {
+                const fom_float *xs = x + it_stb[s];
+                unsigned int u0, u1;
+                __m128d x01, x23, x45, x67;
+                /* offsets as two 32-bit words: byte 0 and byte 1, then the
+                   same after one 16-bit shift */
+                memcpy(&u0, o, 4);
+                memcpy(&u1, o + 4, 4);
+                x01 = _mm_loadh_pd(_mm_load_sd(xs + (u0 & 0xff)), xs + ((u0 >> 8) & 0xff));
+                u0 >>= 16;
+                x23 = _mm_loadh_pd(_mm_load_sd(xs + (u0 & 0xff)), xs + (u0 >> 8));
+                x45 = _mm_loadh_pd(_mm_load_sd(xs + (u1 & 0xff)), xs + ((u1 >> 8) & 0xff));
+                u1 >>= 16;
+                x67 = _mm_loadh_pd(_mm_load_sd(xs + (u1 & 0xff)), xs + (u1 >> 8));
+                a0 = _mm256_fmadd_pd(_mm256_load_pd(v), _mm256_set_m128d(x23, x01), a0);
+                a1 = _mm256_fmadd_pd(_mm256_load_pd(v + 4), _mm256_set_m128d(x67, x45), a1);
+            }
+            if (k) {
+                a0 = _mm256_add_pd(_mm256_loadu_pd(acc + 8 * g), a0);
+                a1 = _mm256_add_pd(_mm256_loadu_pd(acc + 8 * g + 4), a1);
+            }
+            _mm256_storeu_pd(acc + 8 * g, a0);
+            _mm256_storeu_pd(acc + 8 * g + 4, a1);
+        }
+    }
+}
+#endif
+
+/* w += B[rows of sub-chunk ch,:]^T x[rows]. The 8 lanes of a step may hit
+   the same column: the sequential read-modify-writes keep that exact. */
+static void it_st_scatter(fom_int ch, const fom_float *x, fom_float *w) {
+    fom_int k, g, s, l, ns = it_ns;
+    const fom_float *v = it_vb + ch * IT_CBP * ns;
+    const unsigned char *o = (const unsigned char *)it_cb + ch * IT_CBP * ns;
+    const fom_float *xc = x + ch * IT_CB;
+    for (k = 0; k < it_nkb; ++k) {
+        fom_int s0 = it_kst[k], s1 = it_kst[k + 1];
+        for (g = 0; g < IT_GR; ++g) {
+            fom_float xl[8] IT_ALIGN;
+            if (8 * g + 8 <= IT_CB) for (l = 0; l < 8; ++l) xl[l] = xc[8 * g + l];
+            else for (l = 0; l < 8; ++l) xl[l] = 8 * g + l < IT_CB ? xc[8 * g + l] : 0.0;
+#ifdef IT_SSE2
+            {
+                __m128d x01 = _mm_load_pd(xl), x23 = _mm_load_pd(xl + 2);
+                __m128d x45 = _mm_load_pd(xl + 4), x67 = _mm_load_pd(xl + 6);
+                for (s = s0; s < s1; ++s, v += 8, o += 8) {
+                    fom_float *restrict ws = w + it_stb[s];
+                    unsigned long long u;
+                    __m128d p01 = _mm_mul_pd(_mm_load_pd(v), x01);
+                    __m128d p23 = _mm_mul_pd(_mm_load_pd(v + 2), x23);
+                    __m128d p45 = _mm_mul_pd(_mm_load_pd(v + 4), x45);
+                    __m128d p67 = _mm_mul_pd(_mm_load_pd(v + 6), x67);
+                    memcpy(&u, o, 8);
+                    ws[u & 0xff] += _mm_cvtsd_f64(p01);
+                    ws[(u >> 8) & 0xff] += _mm_cvtsd_f64(_mm_unpackhi_pd(p01, p01));
+                    ws[(u >> 16) & 0xff] += _mm_cvtsd_f64(p23);
+                    ws[(u >> 24) & 0xff] += _mm_cvtsd_f64(_mm_unpackhi_pd(p23, p23));
+                    ws[(u >> 32) & 0xff] += _mm_cvtsd_f64(p45);
+                    ws[(u >> 40) & 0xff] += _mm_cvtsd_f64(_mm_unpackhi_pd(p45, p45));
+                    ws[(u >> 48) & 0xff] += _mm_cvtsd_f64(p67);
+                    ws[u >> 56] += _mm_cvtsd_f64(_mm_unpackhi_pd(p67, p67));
+                }
+            }
+#else
+            for (s = s0; s < s1; ++s, v += 8, o += 8) {
+                fom_float *ws = w + it_stb[s];
+                for (l = 0; l < 8; ++l) ws[o[l]] += v[l] * xl[l];
+            }
+#endif
+        }
+    }
+}
+
+#ifdef IT_AVX2
+/* it_st_scatter in VEX encoding; lanes 0-3 take their offsets as one
+   32-bit word (ALU extraction), lanes 4-7 as single bytes (loads), to
+   balance the ALU and load ports of the read-modify-write chain. */
+__attribute__((target("avx2,bmi,bmi2")))
+static void it_st_scatter_avx2(fom_int ch, const fom_float *x, fom_float *w) {
+    fom_int k, g, s, l, ns = it_ns;
+    const fom_float *v = it_vb + ch * IT_CBP * ns;
+    const unsigned char *o = (const unsigned char *)it_cb + ch * IT_CBP * ns;
+    const fom_float *xc = x + ch * IT_CB;
+    for (k = 0; k < it_nkb; ++k) {
+        fom_int s0 = it_kst[k], s1 = it_kst[k + 1];
+        for (g = 0; g < IT_GR; ++g) {
+            fom_float xl[8] IT_ALIGN;
+            __m128d x01, x23, x45, x67;
+            if (8 * g + 8 <= IT_CB) for (l = 0; l < 8; ++l) xl[l] = xc[8 * g + l];
+            else for (l = 0; l < 8; ++l) xl[l] = 8 * g + l < IT_CB ? xc[8 * g + l] : 0.0;
+            x01 = _mm_load_pd(xl); x23 = _mm_load_pd(xl + 2);
+            x45 = _mm_load_pd(xl + 4); x67 = _mm_load_pd(xl + 6);
+            for (s = s0; s < s1; ++s, v += 8, o += 8) {
+                fom_float *restrict ws = w + it_stb[s];
+                unsigned int u;
+                __m128d p01 = _mm_mul_pd(_mm_load_pd(v), x01);
+                __m128d p23 = _mm_mul_pd(_mm_load_pd(v + 2), x23);
+                __m128d p45 = _mm_mul_pd(_mm_load_pd(v + 4), x45);
+                __m128d p67 = _mm_mul_pd(_mm_load_pd(v + 6), x67);
+                __m128d h01 = _mm_unpackhi_pd(p01, p01), h23 = _mm_unpackhi_pd(p23, p23);
+                __m128d h45 = _mm_unpackhi_pd(p45, p45), h67 = _mm_unpackhi_pd(p67, p67);
+                memcpy(&u, o, 4);
+                ws[u & 0xff] += _mm_cvtsd_f64(p01);
+                ws[(u >> 8) & 0xff] += _mm_cvtsd_f64(h01);
+                ws[(u >> 16) & 0xff] += _mm_cvtsd_f64(p23);
+                ws[u >> 24] += _mm_cvtsd_f64(h23);
+                ws[o[4]] += _mm_cvtsd_f64(p45);
+                ws[o[5]] += _mm_cvtsd_f64(h45);
+                ws[o[6]] += _mm_cvtsd_f64(p67);
+                ws[o[7]] += _mm_cvtsd_f64(h67);
+            }
+        }
+    }
+}
+
+/* One pass over two sub-chunks of the same shape: gathers sub-chunk chg
+   (acc = B[rows of chg,:] x, as it_st_gather_avx2) while scattering
+   sub-chunk chs (w += B[rows of chs,:]^T y, as it_st_scatter_avx2, in the
+   same order). The gather is load-bound and the scatter store-bound, so
+   interleaving their steps fills each other's idle ports; both walk the
+   same stratum s, so they share its base. */
+/* AVX-512VL only for its 32 vector registers (all vectors are 128/256-bit) */
+__attribute__((target("avx2,fma,bmi,bmi2,avx512f,avx512vl")))
+static void it_st_fused(fom_int chg, const fom_float *x, fom_float *acc, fom_int chs, const fom_float *y, fom_float *w) {
+    fom_int k, g, s, l, ns = it_ns;
+    const fom_float *vg = it_vb + chg * IT_CBP * ns, *vs = it_vb + chs * IT_CBP * ns;
+    const unsigned char *og = (const unsigned char *)it_cb + chg * IT_CBP * ns;
+    const unsigned char *os = (const unsigned char *)it_cb + chs * IT_CBP * ns;
+    const fom_float *yc = y + chs * IT_CB;
+    for (k = 0; k < it_nkb; ++k) {
+        fom_int s0 = it_kst[k], s1 = it_kst[k + 1];
+        for (g = 0; g < IT_GR; ++g) {
+            fom_float yl[8] IT_ALIGN;
+            __m128d y01, y23, y45, y67;
+            __m256d a0 = _mm256_setzero_pd(), a1 = _mm256_setzero_pd();
+            if (8 * g + 8 <= IT_CB) for (l = 0; l < 8; ++l) yl[l] = yc[8 * g + l];
+            else for (l = 0; l < 8; ++l) yl[l] = 8 * g + l < IT_CB ? yc[8 * g + l] : 0.0;
+            y01 = _mm_load_pd(yl); y23 = _mm_load_pd(yl + 2);
+            y45 = _mm_load_pd(yl + 4); y67 = _mm_load_pd(yl + 6);
+            for (s = s0; s < s1; ++s, vg += 8, og += 8, vs += 8, os += 8) {
+                const fom_float *xs = x + it_stb[s];
+                fom_float *restrict ws = w + it_stb[s];
+                unsigned int u0, u1, u;
+                __m128d x01, x23, x45, x67;
+                __m128d p01 = _mm_mul_pd(_mm_load_pd(vs), y01);
+                __m128d p23 = _mm_mul_pd(_mm_load_pd(vs + 2), y23);
+                __m128d p45 = _mm_mul_pd(_mm_load_pd(vs + 4), y45);
+                __m128d p67 = _mm_mul_pd(_mm_load_pd(vs + 6), y67);
+                __m128d h01 = _mm_unpackhi_pd(p01, p01), h23 = _mm_unpackhi_pd(p23, p23);
+                __m128d h45 = _mm_unpackhi_pd(p45, p45), h67 = _mm_unpackhi_pd(p67, p67);
+                memcpy(&u0, og, 4);
+                memcpy(&u1, og + 4, 4);
+                x01 = _mm_loadh_pd(_mm_load_sd(xs + (u0 & 0xff)), xs + ((u0 >> 8) & 0xff));
+                u0 >>= 16;
+                x23 = _mm_loadh_pd(_mm_load_sd(xs + (u0 & 0xff)), xs + (u0 >> 8));
+                x45 = _mm_loadh_pd(_mm_load_sd(xs + (u1 & 0xff)), xs + ((u1 >> 8) & 0xff));
+                u1 >>= 16;
+                x67 = _mm_loadh_pd(_mm_load_sd(xs + (u1 & 0xff)), xs + (u1 >> 8));
+                a0 = _mm256_fmadd_pd(_mm256_load_pd(vg), _mm256_set_m128d(x23, x01), a0);
+                a1 = _mm256_fmadd_pd(_mm256_load_pd(vg + 4), _mm256_set_m128d(x67, x45), a1);
+                memcpy(&u, os, 4);
+                ws[u & 0xff] += _mm_cvtsd_f64(p01);
+                ws[(u >> 8) & 0xff] += _mm_cvtsd_f64(h01);
+                ws[(u >> 16) & 0xff] += _mm_cvtsd_f64(p23);
+                ws[u >> 24] += _mm_cvtsd_f64(h23);
+                ws[os[4]] += _mm_cvtsd_f64(p45);
+                ws[os[5]] += _mm_cvtsd_f64(h45);
+                ws[os[6]] += _mm_cvtsd_f64(p67);
+                ws[os[7]] += _mm_cvtsd_f64(h67);
+            }
+            if (k) {
+                a0 = _mm256_add_pd(_mm256_loadu_pd(acc + 8 * g), a0);
+                a1 = _mm256_add_pd(_mm256_loadu_pd(acc + 8 * g + 4), a1);
+            }
+            _mm256_storeu_pd(acc + 8 * g, a0);
+            _mm256_storeu_pd(acc + 8 * g + 4, a1);
+        }
+    }
+}
+#endif
+
+/* the fused mode's two kernels over whichever layout it_check built */
+static inline void it_g1(fom_int ch, const fom_float *x, fom_float *acc) {
+    if (it_strata) {
+#ifdef IT_AVX2
+        if (it_avx2) { it_st_gather_avx2(ch, x, acc, 0, 0); return; }
+#endif
+        it_st_gather(ch, x, acc);
+        return;
+    }
+#ifdef IT_AVX2
+    if (it_avx2) { it_sell_chunk_avx2(&it_B, ch, x, acc); return; }
+#endif
+    it_sell_chunk(&it_B, ch, x, acc);
+}
+
+static inline void it_s1(fom_int ch, const fom_float *x, fom_float *w) {
+    if (it_strata) {
+#ifdef IT_AVX2
+        if (it_avx2) { it_st_scatter_avx2(ch, x, w); return; }
+#endif
+        it_st_scatter(ch, x, w);
+        return;
+    }
+    it_sell_scatter(&it_B, ch, x, w);
+}
+
+/* acc[0..cr) = (blocks k0..k0+nk of row chunk h of the slab layout) x
+   (acc[cr] takes the padding) */
+static void it_slabs(const ItSell *m, fom_int h, fom_int k0, fom_int nk, const fom_float *x, fom_float *acc) {
+    fom_int k, g, j, l;
+    const fom_float *v = m->val;
+    const unsigned short *c = m->col;
+    for (l = 0; l <= m->cr; ++l) acc[l] = 0.0;
+    for (k = k0; k < k0 + nk; ++k) {
+        fom_int q = m->bst[h * m->nblk + k];
+        const fom_float *xk = x + k * m->bw;
+        const unsigned short *len = m->len + (h * m->nblk + k) * m->ngrp;
+        const unsigned short *perm = m->perm + (h * m->nblk + k) * m->ngrp * 8;
+        for (g = 0; g < m->ngrp; ++g) {
+            fom_float a0 = 0.0, a1 = 0.0, a2 = 0.0, a3 = 0.0, a4 = 0.0, a5 = 0.0, a6 = 0.0, a7 = 0.0;
+            fom_int L = len[g];
+            for (j = 0; j < L; ++j, q += 8) {
+                a0 += v[q] * xk[c[q]];
+                a1 += v[q + 1] * xk[c[q + 1]];
+                a2 += v[q + 2] * xk[c[q + 2]];
+                a3 += v[q + 3] * xk[c[q + 3]];
+                a4 += v[q + 4] * xk[c[q + 4]];
+                a5 += v[q + 5] * xk[c[q + 5]];
+                a6 += v[q + 6] * xk[c[q + 6]];
+                a7 += v[q + 7] * xk[c[q + 7]];
+            }
+            acc[perm[8 * g]] += a0; acc[perm[8 * g + 1]] += a1;
+            acc[perm[8 * g + 2]] += a2; acc[perm[8 * g + 3]] += a3;
+            acc[perm[8 * g + 4]] += a4; acc[perm[8 * g + 5]] += a5;
+            acc[perm[8 * g + 6]] += a6; acc[perm[8 * g + 7]] += a7;
+        }
+    }
+}
+
+#ifdef IT_AVX2
+__attribute__((target("avx2,fma,bmi,bmi2")))
+static void it_slabs_avx2(const ItSell *m, fom_int h, fom_int k0, fom_int nk, const fom_float *x, fom_float *acc) {
+    fom_int k, g, j, l;
+    const fom_float *v = m->val;
+    const unsigned short *c = m->col;
+    double tmp[8];
+    for (l = 0; l <= m->cr; ++l) acc[l] = 0.0;
+    for (k = k0; k < k0 + nk; ++k) {
+        fom_int q = m->bst[h * m->nblk + k];
+        const fom_float *xk = x + k * m->bw;
+        const unsigned short *len = m->len + (h * m->nblk + k) * m->ngrp;
+        const unsigned short *perm = m->perm + (h * m->nblk + k) * m->ngrp * 8;
+        for (g = 0; g < m->ngrp; ++g) {
+            __m256d a0 = _mm256_setzero_pd(), a1 = _mm256_setzero_pd();
+            fom_int L = len[g];
+            for (j = 0; j < L; ++j, q += 8) {
+                unsigned long long w0, w1;
+                __m128d x01, x23, x45, x67;
+                memcpy(&w0, c + q, 8);
+                memcpy(&w1, c + q + 4, 8);
+                x01 = _mm_loadh_pd(_mm_load_sd(xk + (w0 & 0xffff)), xk + ((w0 >> 16) & 0xffff));
+                x23 = _mm_loadh_pd(_mm_load_sd(xk + ((w0 >> 32) & 0xffff)), xk + (w0 >> 48));
+                x45 = _mm_loadh_pd(_mm_load_sd(xk + (w1 & 0xffff)), xk + ((w1 >> 16) & 0xffff));
+                x67 = _mm_loadh_pd(_mm_load_sd(xk + ((w1 >> 32) & 0xffff)), xk + (w1 >> 48));
+                a0 = _mm256_fmadd_pd(_mm256_load_pd(v + q), _mm256_set_m128d(x23, x01), a0);
+                a1 = _mm256_fmadd_pd(_mm256_load_pd(v + q + 4), _mm256_set_m128d(x67, x45), a1);
+            }
+            _mm256_storeu_pd(tmp, a0);
+            _mm256_storeu_pd(tmp + 4, a1);
+            for (l = 0; l < 8; ++l) acc[perm[8 * g + l]] += tmp[l];
+        }
+    }
+}
+#endif
+
+/* The fast path's body, run by every thread of a team: either the region
+   fom_iterate opens itself, or (team = 1) the caller's, so a solve can run
+   its iterations in one region (orphaned work-sharing loops). Values shared
+   across the team are either computed alike by every thread from the
+   arguments and the static state below, which every thread reads before the
+   first barrier and the master rewrites only after it, or kept in statics
+   indexed by the call's parity. */
+#ifdef FOM_PROFILE
+static double it_ts[4];
+#endif
+static int it_par = 0;     /* parity of the call */
+static int it_stf[2];      /* stale flags, one per parity */
+#define IT_PART(s) (pb + (s) * IT_PST)
+
+/* t0[c0..c1) = act(b0 + cinv * sum_sup partial_sup) over the fused-mode partials */
+static inline void it_t0_cols(const fom_float *pb, fom_int c0, fom_int c1, const fom_float *bias_t0, fom_float cinv, fom_float l0, fom_float *t_t0) {
+    fom_int i, sup;
+    for (i = c0; i < c1; ++i) {
+        fom_float s = 0.0;
+        for (sup = 0; sup < IT_NSUP; ++sup) s += IT_PART(sup)[i];
+        s = bias_t0[i] + cinv * s;
+        t_t0[i] = (s > l0) ? (s - l0) : ((s < -l0) ? (s + l0) : 0.0);
+    }
+}
+
+static void it_body2(const fom_float *state, const fom_float *bias_t0, const fom_float *bias_t1, const fom_float *bias_su, fom_float l0, fom_float l1, fom_float *t_t0, fom_float *t_t1, fom_float *new_state) {
+    fom_int ch, i, r0;
+    fom_float acc[IT_CB2 + 1];
+    const fom_float *pb = it_pbuf;
+    IT_STAMP(it_ts[0]);
+    /* partial_s = (slabs of group s of A) x: thread t gathers row chunk
+       t mod IT_P of group s = t / IT_P, from the rows of x the group's
+       threads wrote */
+    #ifdef _OPENMP
+    #pragma omp for schedule(static)
+    #endif
+    for (ch = 0; ch < IT_NSUP; ++ch) {
+        fom_int h = ch & (IT_P - 1), g0 = ch >> IT_LP;
+#ifdef IT_AVX2
+        if (it_avx2) it_slabs_avx2(&it_S, h, g0 << IT_LP, IT_P, state, it_sacc[ch]); else
+#endif
+        it_slabs(&it_S, h, g0 << IT_LP, IT_P, state, it_sacc[ch]);
+        for (i = 0; i < IT_RP; ++i) it_pbuf[g0 * IT_PST + h * IT_RP + i] = it_sacc[ch][i];
+    }
+    IT_STAMP(it_ts[1]);
+    /* t0 = act(b0 + sum_s partial_s) = act(b0 + A x) */
+    #ifdef _OPENMP
+    #pragma omp for schedule(static)
+    #endif
+    for (i = 0; i < FOM_MAT_t0_state_ROWS; ++i) {
+        fom_float s = 0.0;
+        for (ch = 0; ch < (IT_NSUP >> IT_LP); ++ch) s += IT_PART(ch)[i];
+        s = bias_t0[i] + s;
+        t_t0[i] = (s > l0) ? (s - l0) : ((s < -l0) ? (s + l0) : 0.0);
+    }
+    IT_STAMP(it_ts[2]);
+    /* t1 = act(b1 + x + B t0), gathered over B's layout */
+    #ifdef _OPENMP
+    #pragma omp for schedule(static)
+    #endif
+    for (ch = 0; ch < IT_NSUP; ++ch) {
+        r0 = ch * IT_CB2;
+#ifdef IT_AVX2
+        if (it_avx2) it_sell_chunk_avx2(&it_B, ch, t_t0, acc); else
+#endif
+        it_sell_chunk(&it_B, ch, t_t0, acc);
+        for (i = 0; i < IT_CB2; ++i) {
+            fom_float s = bias_t1[r0 + i] + state[r0 + i] + acc[i];
+            s = (s > l1) ? (s - l1) : ((s < -l1) ? (s + l1) : 0.0);
+            t_t1[r0 + i] = s;
+            new_state[r0 + i] = bias_su[r0 + i] + s;
+        }
+    }
+    IT_STAMP(it_ts[3]);
+}
+
+static void it_body1(const fom_float *state, const fom_float *bias_t0, const fom_float *bias_t1, const fom_float *bias_su, fom_float l0, fom_float l1, fom_float *t_t0, fom_float *t_t1, fom_float *new_state, int team) {
+    /* per-call values, alike in every thread */
+    fom_float cinv = it_cinv;
+    fom_float *pb = it_pbuf + ((((size_t)t_t0 + 2048 - (size_t)it_pbuf) & 4095) >> 3);
+    int par = it_par, stale = !it_memo_ok || pb != it_pbase, odir = it_dir, dir;
+    fom_int sup, sub, s0, ch, i, r0, prev = 0;
+    double tp = 0.0;
+    fom_float acc[IT_CBP + 1];
+    /* strata layout with AVX-512VL: gather sub-chunk s0 while
+       scattering sub-chunk s0 - 1 (same order of the scatter's
+       additions) */
+#ifdef IT_AVX2
+    int pipe = it_strata && it_avx512;
+#else
+    const int pipe = 0;
+#endif
+    (void)tp;
+    IT_STAMP(it_ts[0]);
+    /* Are the partials from the last call those of this state? Each
+       thread checks its rows of the state and, assuming they are,
+       already reduces its slice of t0 = act(b0 + cinv * sum_sup
+       partial_sup) = act(b0 + A x): one barrier for both. */
+    #ifdef _OPENMP
+    #pragma omp for schedule(static)
+    #endif
+    for (sup = 0; sup < IT_NSUP; ++sup) {
+        int bad = 0;
+        for (i = sup * IT_NSUB * IT_CB; i < (sup + 1) * IT_NSUB * IT_CB; ++i) bad |= (state[i] != it_memo_x[i]);
+        if (bad) {
+            #ifdef _OPENMP
+            #pragma omp atomic write
+            #endif
+            it_stf[par] = 1;
+        }
+        it_t0_cols(pb, (sup * FOM_MAT_t1_t_t0_COLS) >> 4, ((sup + 1) * FOM_MAT_t1_t_t0_COLS) >> 4, bias_t0, cinv, l0, t_t0);
+    }
+    IT_STAMP(it_ts[1]);
+    {
+        int f;
+        #ifdef _OPENMP
+        #pragma omp atomic read
+        #endif
+        f = it_stf[par];
+        stale = stale || f;
+    }
+    /* if not, recompute them, partial_sup = B[rows of sup,:]^T x, and t0 */
+    if (stale) {
+        #ifdef _OPENMP
+        #pragma omp for schedule(static)
+        #endif
+        for (sup = 0; sup < IT_NSUP; ++sup) {
+            for (i = 0; i < FOM_MAT_t1_t_t0_COLS; ++i) IT_PART(sup)[i] = 0.0;
+            for (sub = 0; sub < IT_NSUB; ++sub) it_s1(sup * IT_NSUB + sub, state, IT_PART(sup));
+        }
+        #ifdef _OPENMP
+        #pragma omp for schedule(static)
+        #endif
+        for (sup = 0; sup < IT_NSUP; ++sup)
+            it_t0_cols(pb, (sup * FOM_MAT_t1_t_t0_COLS) >> 4, ((sup + 1) * FOM_MAT_t1_t_t0_COLS) >> 4, bias_t0, cinv, l0, t_t0);
+    }
+    /* Sweep the sub-chunks opposite to the last pass, so the part of
+       this thread's slice of B still in L2 is read first. */
+    dir = stale ? 1 : !odir;
+    /* Every thread has read the static state: the master updates it for
+       the next call (the partials are complete once the team is past the
+       end of this call: its barrier, or the region's end). */
+    if (IT_MASTER) {
+        it_stf[par ^ 1] = 0;
+        it_par = par ^ 1;
+        it_dir = dir;
+        it_pbase = pb;
+        it_memo_ok = 1;
+    }
+    IT_STAMP(it_ts[2]);
+    /* t1 = act(b1 + x + B t0) one sub-chunk at a time; while its slice of
+       B is still in L2, scatter the new state through it, giving the
+       partials of B^T x_new that the next call reuses. */
+    #ifdef _OPENMP
+    #pragma omp for schedule(static) IT_NOWAIT
+    #endif
+    for (sup = 0; sup < IT_NSUP; ++sup) {
+        if (!pipe) for (i = 0; i < FOM_MAT_t1_t_t0_COLS; ++i) IT_PART(sup)[i] = 0.0;
+        for (s0 = 0; s0 < IT_NSUB; ++s0) {
+            sub = dir ? IT_NSUB - 1 - s0 : s0;
+            ch = sup * IT_NSUB + sub;
+            r0 = ch * IT_CB;
+            IT_STAMP(tp);
+            if (pipe && s0 > 0) it_st_fused(ch, t_t0, acc, prev, it_memo_x, IT_PART(sup));
+#ifdef IT_AVX2
+            else if (pipe) {
+                /* the first gather takes ownership of the partials
+                   (just read by the other threads' reduce)
+                   meanwhile, then they are zeroed */
+                it_st_gather_avx2(ch, t_t0, acc, (const char *)IT_PART(sup), (const char *)(IT_PART(sup) + FOM_MAT_t1_t_t0_COLS));
+                for (i = 0; i < FOM_MAT_t1_t_t0_COLS; ++i) IT_PART(sup)[i] = 0.0;
+            }
+#endif
+            else it_g1(ch, t_t0, acc);
+            IT_PH(pipe && s0 > 0 ? 1 : 0, tp);
+            for (i = 0; i < IT_CB; ++i) {
+                fom_float s = bias_t1[r0 + i] + state[r0 + i] + acc[i];
+                s = (s > l1) ? (s - l1) : ((s < -l1) ? (s + l1) : 0.0);
+                t_t1[r0 + i] = s;
+                s = bias_su[r0 + i] + s;
+                new_state[r0 + i] = s;
+                it_memo_x[r0 + i] = s;
+            }
+            IT_PH(3, tp);
+            if (!pipe) it_s1(ch, it_memo_x, IT_PART(sup));
+            IT_PH(2, tp);
+            prev = ch;
+        }
+        if (pipe) it_s1(prev, it_memo_x, IT_PART(sup));
+        IT_PH(2, tp);
+    }
+    IT_STAMP(it_ts[3]);
+    if (team) {
+        #ifdef _OPENMP
+        #pragma omp barrier
+        #endif
+    }
+}
+
+/* Called by every thread of an enclosing parallel region (a solve running
+   its iterations in one region), it shares the work out among them;
+   otherwise it opens its own region. */
 void fom_iterate(const fom_float *state, const fom_float *bias_t0, const fom_float *bias_t1, const fom_float *bias_su, const FomAct_t0 *act_t0, const FomAct_t1 *act_t1, fom_float *t_t0, fom_float *t_t1, fom_float *new_state) {
-    fom_step_t0(state, bias_t0, act_t0, t_t0);
-    fom_step_t1(state, t_t0, bias_t1, act_t1, t_t1);
-    fom_step_su(t_t1, bias_su, new_state);
-}
+    fom_float l0 = act_t0->lambd, l1 = act_t1->lambd;
+    int team = 0;
+#ifdef _OPENMP
+    team = omp_in_parallel();
+#endif
+    if (team) {
+        /* it_ready is set by an earlier call outside the team (the solve
+           runs its first iteration that way), so every thread agrees */
+        if (it_ready <= 0) {
+            #ifdef _OPENMP
+            #pragma omp single
+            #endif
+            {
+                if (it_ready == 0) it_ready = it_check();
+                if (it_ready < 0) {
+                    fom_step_t0(state, bias_t0, act_t0, t_t0);
+                    fom_step_t1(state, t_t0, bias_t1, act_t1, t_t1);
+                    fom_step_su(t_t1, bias_su, new_state);
+                }
+            }
+            if (it_ready < 0) return;
+        }
+        if (it_mode == 2) {
+            it_body2(state, bias_t0, bias_t1, bias_su, l0, l1, t_t0, t_t1, new_state);
+        } else it_body1(state, bias_t0, bias_t1, bias_su, l0, l1, t_t0, t_t1, new_state, 1);
+    } else {
+        if (it_ready == 0) it_ready = it_check();
+        if (it_ready < 0) {
+            fom_step_t0(state, bias_t0, act_t0, t_t0);
+            fom_step_t1(state, t_t0, bias_t1, act_t1, t_t1);
+            fom_step_su(t_t1, bias_su, new_state);
+            return;
+        }
+        #ifdef _OPENMP
+        #pragma omp parallel
+        #endif
+        {
+            if (it_mode == 2) it_body2(state, bias_t0, bias_t1, bias_su, l0, l1, t_t0, t_t1, new_state);
+            else it_body1(state, bias_t0, bias_t1, bias_su, l0, l1, t_t0, t_t1, new_state, 0);
+        }
+    }
+#ifdef FOM_PROFILE
+    if (IT_MASTER) {
+        fom_prof_add(it_mode == 2 ? "it_t0_slab_gather" : "it_check_t0_reduce", it_ts[1] - it_ts[0]);
+        fom_prof_add(it_mode == 2 ? "it_t0_reduce" : "it_stale_recompute", it_ts[2] - it_ts[1]);
+        fom_prof_add(it_mode == 2 ? "it_t1_gather" : "it_t1_gather_scatter", it_ts[3] - it_ts[2]);
+        if (it_mode == 1) {
+            /* parts of it_t1_gather_scatter (master thread) */
+            fom_prof_add("it_t1__gather_only", it_ph[0]);
+            fom_prof_add("it_t1__fused_gather_scatter", it_ph[1]);
+            fom_prof_add("it_t1__scatter_only", it_ph[2]);
+            fom_prof_add("it_t1__rows_act", it_ph[3]);
+            it_ph[0] = it_ph[1] = it_ph[2] = it_ph[3] = 0.0;
+        }
+    }
+#endif
+}
```

## Agent notes

```
### Round 1

**Whole-solve mean_us (200 ISTA iterations, 16 OpenMP threads)**
- Initial: 95,016 us (AMD EPYC 7413 node); 94,290 us (Intel Xeon 6342 node).
- Final: 29,055 us on AMD, **3.27x**; 46,547 us on Intel, **2.03x**.
- The build holder moved between the two node types during the session, so both are reported. The code picks its mode at run time for each.

**Final profile breakdown, hottest first**
- Intel node (mode 1, fused), per solve:
  - it_t1_gather_scatter: 218 us/iter, 46.9% (the profiler prints the enclosing `iterate` region too, so percentages sum to 50%)
  - it_t0_reduce: 10 us/iter, 2.2%
  - it_memo_check_scatter: 3 us/iter, 0.6%
  - residual: 0.01%
- AMD node (mode 2, two gathers):
  - it_t0_gather: 76 us/iter, 26.2%
  - it_t1_gather: 67 us/iter, 23.2%
  - residual: 0.01%

**Optimizations applied**
1. **Whole iteration in `fom_iterate`, one parallel region per iteration.** `t1_state` and `su_t_t1` are identities, so t1 is written straight into the next state and the copy pass is gone. The parameter term `t0_p_p0 @ p0` is computed once per solve.
2. **Residual only on the last iteration.** `FOM_USE_TOL = 0`, so the residual is only read at termination.
3. **16-bit column indices.** Both matrices have at most 40,000 columns. The repacked index data is half the size.
4. **Cache-blocked sliced-ELL repacking (SELL-8-sigma)**, built from the matrix data on first use. It only reorders values and does no arithmetic.
   - Each thread owns a row chunk. Columns are split into 2048-entry blocks, so the gathered vector slice stays in L1.
   - Within each chunk and block, rows are sorted by entry count and packed 8 at a time, column-major. Padding is about 1%, because the matrices are uniformly sparse (257-260 nonzeros per row).
   - The kernel is a branch-free 8-lane loop over one sequential stream. Each load fetches 4 indices, and x is loaded in pairs. An AVX2+FMA version is picked at run time, with an SSE2 fallback.
   - Effect on AMD: 46 → 29 ms.
5. **B-only fused mode (mode 1).** In this data `t1_t_t0 == c * t0_state^T` exactly; the relation is checked entry by entry at startup. So A x = (1/c) B^T x becomes a scatter over B's layout, and 1/c is computed by a division-free Newton iteration.
   - B is processed in 250-row sub-chunks. Each sub-chunk is gathered, and then, while its data is still in L2, the new state is scattered through it. This yields the partial sums of A·x_new for the next call.
   - Those partials are reused only if the next call's state matches x_new bit for bit. Otherwise they are recomputed in the same order, so results are bitwise identical either way.
   - B is read from memory once per iteration instead of twice. Effect on Intel: 55-61 → 47 ms.
6. **Mode selection by CPU vendor.**
   - On AMD the core is the limit and a scatter costs about 2x a gather, so two gathers win: 29 ms vs 46 ms for the fused mode.
   - On Intel memory bandwidth is the limit, so reading B once wins: 47 ms vs 59 ms.
   - The choice is fixed per machine, so results are deterministic on a given machine.

**Tried and rejected**
- AVX-512 and AVX2 hardware gathers: slower on Zen 3, Zen 4 and Ice Lake.
- Column blocking applied directly to the original CSR: 25-50% slower.
- A persistent parallel region spanning the whole solve.
- Software prefetching of x, and of the next sub-chunk.
- 16-lane groups.
- A two-step unroll with four FMA accumulators.

**Caveat:** the repacked layouts take about 60 MB of static memory. There is no dynamic allocation.

Blocked by rules:
- Repack the matrices once in `fom_setup` instead of on the first `fom_iterate` call | fom_solver.c may not read FOM_MAT data, and setup may not call fom_iterate or fom_apply | unknown; this only removes a one-time setup cost from the first solve
- Add `-march=native` so AVX2/FMA code is generated without hand-written target attributes and run-time dispatch | Makefile and build flags are fixed | unknown, probably small; the kernels are hand-vectorized already
- Choose the thread count or CPU placement, e.g. 32 threads to use more L3 slices | OMP_NUM_THREADS comes from the environment | about 1.5x when 32 threads were tried on the login node
- Store A in float32 or another compressed format to halve memory traffic on bandwidth-bound nodes | exactness and the data in fom_data.c are fixed (not allowed to approximate the coefficients) | not tested

### Round 2

**Whole-solve `mean_us`** (`make bench`, or interleaved A/B runs of `bench_bin`):
- Intel Xeon Gold 6342 node (della-l04g5), where round 1 measured 46726.290 us: round-1 code 46.6–47.7 ms, this round's code 40.0–41.4 ms. That is 1.13–1.16x for the round and about 2.3x against the translated solver (93498.855 us).
- AMD EPYC 7413 node (della-l07g6): round-1 code 28.9–29.6 ms, final code 26.9–27.3 ms, about 1.09x.
- The holder allocation moved nodes mid-session, so numbers are only comparable within one node.
- The final benchmark node class (EPYC 7H12, Zen 2) takes the two-gather mode, which got the AMD-side changes.

**Final profile** (`make profile`, AMD node; "iterate" is the parent region, the `it_*` regions are inside it):
- iterate: 2017258 us, 50.63%, about 136 us per iteration
- it_t0_slab_gather: 967533 us, 24.29%, about 65 us per iteration
- it_t1_gather: 888309 us, 22.30%, about 60 us per iteration
- it_t0_reduce: 110458 us, 2.77%, about 7.5 us per iteration
- residual: 420 us, 0.01%

On Intel, the scatter mode reports `it_t1_gather_scatter` ≈ 186 us per iteration, then `it_check_t0_reduce`.

**Optimizations applied**
1. **Column-slab partition of A in two-gather mode** (non-Intel CPUs), about 7% on AMD.
   - Structure: x is the previous state, and thread t wrote rows [2500t, 2500t+2500) of x in its B pass. So A is cut into 16 column slabs that match those rows.
   - Each group of 4 neighbouring threads (one Zen 2 core complex) gathers its 4 slabs, each thread taking a quarter of A's rows (an L1-resident 2500-entry x block per slab). A short reduce then sums 4 partials into t0.
   - Each thread used to pull all of x (320 KB) across cores every iteration. Now x stays within the group plus 20 KB of partials. Profiling showed this cross-core traffic was significant: threads sharing an L3 with more of our threads finished about 10% later.
   - The reduce moved from 16 µs (one partial per thread) to 6 µs with groups of 4.
2. **Snake (alternating-direction) sweep over each thread's B slice** in the Intel scatter mode, about 7%. A thread's slice of B is 1.6 MB against a 1.25 MB L2. Reversing the sub-chunk order every call means the part still in L2 is read first instead of being evicted by an LRU forward sweep.
3. **Retuned scatter-mode blocking**: 500-row sub-chunks and 4096-wide column blocks of t0 (was 250 rows and 2048), about 5% on Intel. This means fewer groups and less per-group overhead, and t0 blocks still fit Ice Lake's 48 KB L1. The two-gather mode keeps 2048-wide blocks, which measured best there.
4. **Lane rotation in the scatter mode**: each of the 8 lanes in a group starts its row segment l/8 of the way in. Lanes no longer walk nearby columns in lockstep, which reduces store-to-load aliasing in the scatter (about 2% on Intel). The two-gather mode doesn't use it, because it slightly slows the pure gathers.
5. **Memo check merged with the t0 reduce** in scatter mode. Each thread checks its slice of the state and reduces its t0 slice in one phase, redoing the reduce only if the memo is stale. This saves a barrier per iteration (about 0.5%).
6. **BMI/BMI2 added to the AVX2 kernels' target attribute**, with a runtime CPU check, about 1%.

Mode choice is unchanged (Intel → B-only scatter mode, otherwise two gathers) and was confirmed on both nodes. Forced the other way it costs 60 ms vs 41 ms on Intel and 46 ms vs 27 ms on AMD.

**Tried and rejected** (measured, no gain or slower):
- AVX-512 and AVX2 hardware gathers (slow under the gather-data-sampling microcode mitigation on Intel, slower on Zen 3 too)
- software and NTA prefetching
- 2x unrolling
- direct 16-bit index loads
- 32-bit-half index extraction (5% slower)
- conflict-aware lane ordering (exact column and mod-512 key)
- work stealing on B pieces
- a flag-pipelined reduce (only ~1%, not worth the spin-waits)
- smaller B and A chunks

Exploiting sparsity of x_new − x is impossible here: every coordinate changes every iteration.

Blocked by rules:
- Store A/B values in a reduced-precision split (float32 plus a correction, or 48-bit truncation) to cut the bytes streamed from L3 per nonzero by 20–40% | `fom_iterate` must apply exactly M (1e-10 check), and the target constraints call for an exact apply | ~10–20% where L3 bandwidth binds (Intel node), unknown on AMD
- One persistent OpenMP parallel region across iterations, removing the per-iteration fork/join | the clean version needs `fom_solve` to ask `fom_ops.c` which mode is active, but every function in `src/fom_ops.c` other than the entry points must be `static`; doing it inside the fixed `fom_iterate` signature needs fragile orphaned-construct state | ~1.5–2.5% (about 3.5 µs of 136 µs per iteration)

### Round 3

**Whole-solve time:** 40229.6 us at the start of this round, 34093 us at the end (`make bench` mean_us), a 1.18x gain this round. Against the translated solver (93498.9 us) that is 2.74x overall.

**Final profile** (`make profile`, 34.7 ms per solve, share of the solve, hottest first):

| Region | Time per solve | Share |
|---|---|---|
| iterate (whole iteration) | 34.67 ms | 100% |
| it_t1_gather_scatter | 31.83 ms | 91.8% |
| it_t1__fused_gather_scatter (master thread) | 22.70 ms | 65.5% |
| it_t1__gather_only (first sub-chunk) | 4.72 ms | 13.6% |
| it_t1__scatter_only (last sub-chunk) | 3.48 ms | 10.0% |
| it_check_t0_reduce | 1.99 ms | 5.7% |
| it_t1__rows_act | 0.73 ms | 2.1% |
| it_stale_recompute | 0.15 ms | 0.4% |
| residual | 0.006 ms | ~0% |

The `it_t1__*` rows are the master thread's split of `it_t1_gather_scatter`.

**Optimizations applied**
1. **Strata layout with 8-bit offsets.** Every row of `t1_t_t0` (B, which is a constant times `t0_state` transposed) has at most one entry in each of 65 column ranges ("strata") of about 154 columns. Neighbouring ranges share one end column, so entries take the first free stratum. Setup checks this entry by entry and falls back to the old layout if it fails.
   - All 8 lanes of an ELL step share one base column, so each entry stores a 1-byte offset instead of a 16-bit column: 9 bytes per entry instead of 10.
   - Rows have uniform length, so groups are 8 consecutive rows. The per-group length and row-permutation arrays go away, and the accumulators load and store as contiguous vectors.
   - Gain: about 4%.
2. **Index extraction from two 32-bit words.** Gather offsets are pulled out with low-byte, high-byte, one shift, and repeat, instead of shifting a 64-bit word per lane. This cut the gather step from about 40 to 34 instructions; about 2%.
3. **Fused gather and scatter.** All strata sub-chunks have the same shape, so one loop gathers sub-chunk i+1 (B·t0) while scattering sub-chunk i (Bᵀ·x_new) at the same stratum, sharing its base. The scatter's addition order is unchanged, so results are bit-identical.
   - It needs AVX-512VL only for its 32 vector registers; every vector op is 128 or 256 bits wide. It is dispatched at runtime, and the AVX2 kernels run unfused otherwise.
   - Retuned with it: 13 strata per block (keeps both source windows in L1) and 5 sub-chunks per thread. Gain: about 5%.
4. **Partials placed on a 4 KB-multiple stride**, 2 KB offset from t0 (mod 4 KB). The memo of partials is recomputed if the placement changes. Gain: about 1.5%.
5. **Overlapped ownership of the partials.** The other threads' reduce leaves each thread's partial lines shared in their caches. The thread now issues `prefetchw` for those lines during its first gather and zeroes them afterwards, so the ownership upgrades overlap compute. The `memset` share of cycles fell from 5.7% to 0.6%; total gain about 1.5%.
6. **Small cleanups:** full groups load their 8 source values directly instead of through a per-lane bounds check (about 0.5%). Added master-thread profile regions inside the main phase.

**Tried and dropped (no gain or slower):**
- AVX-512 hardware gathers (about 2x slower here, likely the Gather Data Sampling microcode mitigation).
- Software prefetching of the matrix stream, the partials in the reduce, or t0.
- Inline-asm scatter, in both indexed-address and `lea` forms.
- 128-bit FMAs in the gather.
- Vectorizing or reordering the reduce.
- Skipping the zeroing by scattering x_new − x_old into the existing partials (slower; the timing-only check without zeroing ran at 38.4 ms).
- Huge pages, and other sub-chunk or stratum-block sizes.

Blocked by rules:
- Store B's values as float32 to cut the dominant memory stream from 9 to 5 bytes per entry, so each thread's slice of B fits in L2 | kernels must apply exactly M (target and verify constraints) | 20-30% (removes the L3 stream and most of the miss stalls)
- Use more than the 16 allotted cores (L2 is 1.25 MB per core; 26 or more cores would keep all of B L2-resident) | fixed bench environment and thread count (OMP_NUM_THREADS / cgroup) | unknown, likely large
- Run the iteration loop inside one persistent parallel region, which needs solver-level orphaned worksharing across the `fom_iterate` boundary | `fom_iterate` signature and the iteration boundary in `include/`, plus the rule that `fom_solver.c` has no matrix loops | about 1% (fork/join and barrier)

### Round 4

**Whole-solve time (`make bench`, mean_us)**
- Translated solver: 93498.9 us. Start of this round: 34453.8 us.
- Final: about 33900 us. Three alternating A/B runs gave 33.80 to 33.98 ms for the new version against 34.21 to 34.31 ms for round 3.
- This round: 1.016x (about 1.1%). Overall: about 2.76x.

**Final profile (`make profile`, mean 34189 us), hottest first**
| region | total_us | pct |
|---|---|---|
| iterate (whole fom_iterate) | 2014436 | 34.30% |
| it_t1_gather_scatter (B gather plus B^T x scatter pass) | 1877321 | 31.96% |
| it_t1__fused_gather_scatter (part of the row above) | 1332582 | 22.69% |
| it_t1__gather_only (first sub-chunk, part of the row above) | 277712 | 4.73% |
| it_t1__scatter_only (last sub-chunk, part of the row above) | 203548 | 3.47% |
| it_check_t0_reduce (memo check plus 16-partial reduce into t0) | 115819 | 1.97% |
| it_t1__rows_act (part of the t1 pass) | 44141 | 0.75% |
| it_stale_recompute | 7697 | 0.13% |
| residual | 272 | 0.00% |

**Kept optimization**
- **One parallel region for the whole solve.** The iteration count is fixed (`FOM_USE_TOL = 0`, max_iter = 200), so `fom_solve` runs the first iteration on its own and iterations 1 to 199 inside a single parallel region. Every thread calls `fom_iterate`. When it detects `omp_in_parallel()`, it shares the work out with orphaned `omp for` loops and ends with a barrier, instead of opening a new region each iteration. That removes a fork and join per iteration.
  - To make this safe, state shared across calls follows one rule: every thread reads it before the first barrier, and only the master rewrites it after that barrier. This covers the partial-buffer base, the memo-valid flag and the sweep direction. The stale flag has one copy per call parity.
  - The first iteration runs outside the region so the one-time layout build happens before the team starts.
  - The residual becomes a work-shared max-reduction on the last iteration.
  - When called outside a region (as `make verify` does), `fom_iterate` behaves as before.

**Tried and reverted (all measured on the compute node)**
- **Replacing the scatter with a second gather.** I built a per-thread column-slab layout of A = t0_state, so both products would be gathers. That needs about 50 MB of repacked data, which overflows the 36 MB L3, and the solve slowed to 48 to 52 ms. Round 3's single-copy design (B only, 23.6 MB) fits in L3 and is better on this machine. In isolation a gather costs 0.42 ns per entry against 0.56 for the scatter, so this is only a win where both copies fit in cache.
- **Dynamic scheduling** of the gather and reduce work: 67 ms. Work units lose their cache-resident data.
- **Software prefetch** of the 16 partial slices before the reduce: 37.7 ms. Reading the partials one at a time instead of interleaved: 36.2 ms. Interleaving the 16 streams gives more memory-level parallelism.
- **Sub-chunk sizes** of 2, 4, 10 and 20 per thread (5 stays best). **Strata per block** of 9, 17 and 22 (13 kept; 17 is within noise).

**Why little is left:** in a 16-thread microbenchmark the fused gather/scatter kernel costs about 0.98 ns per entry, and the real pass already runs at about that speed. Its inner loop does about 30 loads per 8-entry step, close to the load-port limit. What remains is cross-core traffic: the 16-partial reduce (about 10 us per iteration) and the first gather pulling t0 from other cores (about 10 us per iteration). That is inherent to splitting the rows of B across threads.

Blocked by rules:
- none
```
