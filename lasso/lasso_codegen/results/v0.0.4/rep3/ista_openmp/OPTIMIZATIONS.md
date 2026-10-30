# Optimization report

## Timing

```
translated (emitted)   57229.6 us/solve
optimized              37760.4 us/solve
speedup                  1.52x
```

Optimization outcome: **kept (re-verified correct)**.

## Rounds

| round | before (us) | after (us) | gain | kept | agent (s) | outcome |
|---:|---:|---:|---:|:---:|---:|---|
| 1 | 57229.616 | 32354.165 | 43.5% | yes | 1458 | continued |
| 2 | 32705.387 | 25668.621 | 21.5% | yes | 3430 | continued |
| 3 | 26289.163 | 24965.351 | 5.0% | yes | 2316 | continued |
| 4 | 54709.868 | 39977.941 | 26.9% | yes | 2336 | continued |
| 5 | 39116.994 | 37760.431 | 3.5% | yes | 2317 | the last of agent_optimize_rounds rounds |

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
| `iterate` | 40000x40000 | 5279966 | yes | random | 6 | 7.105e-15 |

### Rule violations

_None: the collected rewrite passed the rules and was kept._

## Blocked by rules

Optimizations the agent considered but did not make because a rule forbade them, in its own words.

| round | idea | rule | expected gain |
|---|---|---|---|
| 1 | AVX2/FMA (or `-march=native`) for 4-wide gathers and fused multiply-add in the SELL kernel | build flags are fixed (`-O3` without ISA flags), and per-function `target` attributes would bypass that and drop portability to the embedded target | estimated 1.3–1.8× on the two products |
| 1 | More OpenMP threads than `OMP_NUM_THREADS=16`: the kernels are bound per core and scale linearly | target/run configuration (I left the environment's thread count alone) | about linear in cores |
| 1 | Carrying iteration state (e.g. partial t0 sums from a column-partitioned pass) across `fom_iterate` calls to save a sync point | the fused/iterate routines must not depend on earlier calls (a static cache that changes results is forbidden) | about 5% |
| 1 | Building the SELL layout in `fom_setup` instead of lazily on the first `fom_iterate` call | `fom_solver.c` may not read matrix data, and `fom_ops.c` may expose no non-entry-point functions, so a setup hook cannot exist | no steady-state gain; it only moves about 10 ms of one-time build cost out of the first solve |
| 2 | Raise the thread count or pin threads to idle or same-CCX cores. The run uses 16 threads spread over shared cores on 2 sockets, and other jobs on CPUs 4–7 make some threads up to 15% slower; this is what the remaining reduce wait mostly reflects. | Makefile/environment (OMP_NUM_THREADS, OMP_PLACES, and the srun CPU set) are fixed | unknown, likely 10–20% on a quiet layout |
| 2 | Build the whole program with `-march=native` instead of per-function target attributes and run-time dispatch. | build flags are fixed | small, about 1–3% |
| 3 | Overlapping the per-iteration copies of `t_t1` and `t_t0` (writing them only on the last iteration). With su_t_t1 = I, `t_t1` equals new_state, and the solver copies state into it at the end anyway | the `fom_iterate` contract (every temporary written on every call), checked by `make verify` | ~1-2% (removes about 4,000 stores per thread per iteration) |
| 3 | Storing matrix values (or the gathered vectors) in float32 to halve the stream and gather traffic | target constraints: the float64 spec and exactness against the reference | unknown; the kernel is instruction-bound, so probably small |
| 4 | Using more than the 16 OpenMP threads the environment provides, for example spreading over more cores or L3 | build flags and environment are fixed | unknown, likely large |
| 4 | float32 copies of matrix values, to make each thread's share fit in L2 | the 1e-10 kernel tolerance in `make verify` | unknown, likely 20-30% |
| 5 | Use more than OMP_NUM_THREADS=16 cores (for example `num_threads(32)`), so each thread's ~0.8 MB slice of the matrix stays in its 1 MB L2 and the dots stop streaming from L3 | the target environment's thread count (the build/run setup is fixed) | probably 1.5–2x, untested |
| 5 | Store the matrix values in float32 to halve the bytes streamed and the L1/L2 footprint | the requirement that every kernel applies exactly M (values are not float32-exact) | ~10–20%, unknown |
| 5 | Compile the hot kernel with -march=znver4 or a newer GCC for Zen 4 scheduling | build flags/Makefile are fixed (GCC 11.5 has no znver4 target) | unknown, probably small |

## What the agent changed

```diff
--- src/fom_solver.c (translated)
+++ src/fom_solver.c (optimized)
@@ -2,6 +2,7 @@
 #include "fom_solver.h"
 #include "fom_data.h"
 #include "fom_ops.h"
+#include "fom_profile.h"
 
 /* t1 has no constant or parameter term: its step bias is zero. */
 static const fom_float fom_zero_t1[FOM_T1_DIM];
@@ -53,11 +54,35 @@
     fom_int status = FOM_MAX_ITER_REACHED;
     fom_int k;
 
-    for (k = 0; k < max_iter; ++k) {
-        fom_step_t0(cur, work->acc, &FOM_ACT_t0, work->t0);
-        /* su_t_t1 is the identity: t1 is the new state. */
-        fom_step_t1(cur, work->t0, fom_zero_t1, &FOM_ACT_t1, nxt);
-        if (use_tol || k == max_iter - 1) res = max_abs_diff(nxt, cur, FOM_STATE_DIM);
+    if (!use_tol && max_iter > 1) {
+        /* Fixed iteration count: every iteration runs inside one parallel
+           region, so the team is forked once per solve, not per iteration;
+           fom_iterate then runs as an orphaned team step. */
+        FOM_PROF_BEGIN(iterate);
+        #pragma omp parallel
+        {
+            fom_float *c = cur, *n = nxt, *sw;
+            fom_int kk;
+            for (kk = 0; kk < max_iter; ++kk) {
+                fom_iterate(c, work->acc, fom_zero_t1, fom_zero_t1, &FOM_ACT_t0, &FOM_ACT_t1, work->t0, work->t1, n);
+                sw = c; c = n; n = sw;
+            }
+        }
+        FOM_PROF_END(iterate);
+        for (k = 0; k < max_iter; ++k) { tmp = cur; cur = nxt; nxt = tmp; }
+        FOM_PROF_BEGIN(residual);
+        res = max_abs_diff(cur, nxt, FOM_STATE_DIM);
+        FOM_PROF_END(residual);
+    } else for (k = 0; k < max_iter; ++k) {
+        FOM_PROF_BEGIN(iterate);
+        /* t1 and su have zero bias (fom_zero_t1 serves both, same dim). */
+        fom_iterate(cur, work->acc, fom_zero_t1, fom_zero_t1, &FOM_ACT_t0, &FOM_ACT_t1, work->t0, work->t1, nxt);
+        FOM_PROF_END(iterate);
+        if (use_tol || k == max_iter - 1) {
+            FOM_PROF_BEGIN(residual);
+            res = max_abs_diff(nxt, cur, FOM_STATE_DIM);
+            FOM_PROF_END(residual);
+        }
         tmp = cur; cur = nxt; nxt = tmp;
         if (use_tol && res <= work->settings.tol) { status = FOM_SOLVED; ++k; break; }
     }
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
@@ -130,8 +135,1268 @@
     }
 }
 
+/* ---- fom_iterate ----------------------------------------------------
+   t0 rows gather from state over t0_state's CSR rows (~260 nnz) and t1
+   rows over t1_t_t0's CSR rows (~65 nnz), each with several independent
+   accumulators: without them every row is one serial chain of FP adds,
+   which (not memory) bounds the reference loops. When su_t_t1 is the
+   identity (checked once from its data), new_state is written in the same
+   pass as t1. Called standalone, one iteration is one parallel region
+   with barriers; called from the solver's persistent team, the iteration
+   runs as point-to-point dataflow (it_flow_body). */
+static int it_su_id = -1;
+static int it_t1s_id = -1;
+
+/* 1 when a 40000x40000 CSR matrix is exactly the identity. */
+static int it_is_identity(const fom_int *rp, const fom_int *ci, const fom_float *va) {
+    fom_int i;
+    if (rp[0] != 0) return 0;
+    for (i = 0; i < 40000; ++i)
+        if (rp[i + 1] != i + 1 || ci[i] != i || va[i] != 1.0) return 0;
+    return 1;
+}
+
+static fom_float it_dot8(const fom_int *ci, const fom_float *va, fom_int p, fom_int pe, const fom_float *x) {
+    fom_float s0 = 0.0, s1 = 0.0, s2 = 0.0, s3 = 0.0, s4 = 0.0, s5 = 0.0, s6 = 0.0, s7 = 0.0;
+    for (; p + 7 < pe; p += 8) {
+        s0 += va[p] * x[ci[p]];
+        s1 += va[p + 1] * x[ci[p + 1]];
+        s2 += va[p + 2] * x[ci[p + 2]];
+        s3 += va[p + 3] * x[ci[p + 3]];
+        s4 += va[p + 4] * x[ci[p + 4]];
+        s5 += va[p + 5] * x[ci[p + 5]];
+        s6 += va[p + 6] * x[ci[p + 6]];
+        s7 += va[p + 7] * x[ci[p + 7]];
+    }
+    for (; p < pe; ++p) s0 += va[p] * x[ci[p]];
+    return ((s0 + s1) + (s2 + s3)) + ((s4 + s5) + (s6 + s7));
+}
+
+/* SELL-C-sigma layout of t0_state (phase 1) and t1_t_t0 (phase 2), built
+   once from their data. Each thread owns a contiguous block of output rows;
+   its rows are cut into column panels at most IT_W wide, so the gathered
+   source segment (IT_W doubles, 16 KB) stays in L1: a random gather from L2 pulls a
+   whole cache line per entry and bounds the plain CSR loop. Within a panel
+   the thread's rows are sorted by segment length and grouped in chunks of
+   IT_C rows, stored slot-major and padded to the chunk's longest segment
+   with zero entries, so one fixed-shape loop with IT_C independent
+   accumulators serves a chunk. Chunk lanes past the last row write to a
+   per-thread dummy slot. */
+#define IT_C 16
+#define IT_W 3334
+#define IT_NT 64
+#define IT_NP 64
+#define IT_CBMAX 4
+#define IT_CAP (2 * FOM_MAT_t0_state_NNZ + FOM_MAT_t0_state_NNZ / 4 + 65536)
+#define IT_CCAP 65536
+#define IT_POSCAP (1 << 20)
+#define IT_Z0 (10000 + IT_C * IT_NT)
+#define IT_Z1 (40000 + IT_C * IT_NT)
+
+static int it_sell = -1;     /* -1 unbuilt, 0 unavailable, 1 ready */
+static int it_nt = 1;
+static unsigned long long it_si[IT_CAP / 4 + 4];  /* 4 packed 16-bit indices per word */
+static fom_float it_sv[IT_CAP];
+static fom_int it_cp[IT_CCAP];
+static fom_int it_cl[IT_CCAP];
+static fom_int it_co[IT_CCAP];
+static unsigned short it_cr[IT_CCAP][IT_C];
+static fom_int it_c1[IT_NT + 1], it_c2[IT_NT + 1];
+static fom_int it_r1[IT_NT + 1], it_r2[IT_NT + 1];
+static fom_int it_cur[40000], it_seg[40000], it_len[40000], it_ord[40000];
+static fom_int it_bkt[40002];
+static fom_float it_zp[IT_NT][IT_Z0];
+static int it_R = 1;
+static fom_int it_q0[IT_NT + 1], it_q1[IT_NT + 1];
+/* panels of the last it_build call, and per thread those of its phase-2
+   rows: chunks it_p2c[t][j] .. it_p2c[t][j+1] gather t0 rows
+   it_p2lo[t][j] .. it_p2hi[t][j] */
+static fom_int it_pc[IT_NP + 1], it_plo[IT_NP], it_phi[IT_NP];
+static int it_pn;
+static fom_int it_p2c[IT_NT][IT_NP + 1], it_p2lo[IT_NT][IT_NP], it_p2hi[IT_NT][IT_NP];
+static int it_p2n[IT_NT];
+static fom_int it_p1c[IT_NT][IT_NP + 1], it_p1lo[IT_NT][IT_NP], it_p1hi[IT_NT][IT_NP];
+static int it_p1n[IT_NT];
+static fom_float it_z1[IT_Z1];
+/* Chunk sums in chunk order (it_ob[c * IT_C + lane]; the slot past the
+   last chunk stays 0.0), and per thread and phase the slot of every row's
+   sum in each panel, row-major: it_pos[it_o1[t] + (i - it_q0[t]) * np + p]
+   (phase 1) and it_pos[it_o2[t] + (i - it_r2[t]) * np + p] (phase 2). The
+   flow iteration stores chunk sums with full-width stores and then adds a
+   row's panel sums in one gather, instead of a scalar read-modify-write per
+   lane at the end of every chunk. */
+#define IT_OBZ (IT_CCAP * IT_C)
+#if defined(__GNUC__)
+static fom_float it_ob[IT_OBZ + 8] __attribute__((aligned(64)));
+#else
+static fom_float it_ob[IT_OBZ + 8];
+#endif
+static fom_int it_pos[IT_POSCAP];
+static fom_int it_o1[IT_NT], it_o2[IT_NT];
+static int it_pos_ok = 0;
+
+/* Fill the slot map of rows [r0, r1) over the panels of the last it_build. */
+static int it_build_pos(fom_int r0, fom_int r1, fom_int *off) {
+    const fom_int np = it_pn;
+    fom_int i, c, o = *off;
+    int pi, r;
+    if ((fom_int)(r1 - r0) * np > IT_POSCAP - o) return 0;
+    for (i = 0; i < (r1 - r0) * np; ++i) it_pos[o + i] = IT_OBZ;
+    for (pi = 0; pi < np; ++pi)
+        for (c = it_pc[pi]; c < it_pc[pi + 1]; ++c)
+            for (r = 0; r < IT_C; ++r) {
+                fom_int row = it_cr[c][r];
+                if (row >= r0 && row < r1) it_pos[o + (row - r0) * np + pi] = c * IT_C + r;
+            }
+    *off = o + (r1 - r0) * np;
+    return 1;
+}
+
+/* Split [0, n) into nt near-equal contiguous parts (no division). */
+static void it_split(fom_int n, int nt, fom_int *b) {
+    fom_int q = 0, r, t;
+    while ((q + 1) * nt <= n) ++q;
+    r = n - q * nt;
+    for (t = 0; t <= nt; ++t) b[t] = t * q + (t < r ? t : r);
+}
+
+/* Append the chunks of rows [r0, r1) of a CSR matrix with ncols columns.
+   Row ids are stored as-is; empty lanes get `dummy`. 0 on overflow. */
+static int it_build(const fom_int *rp, const fom_int *ci, const fom_float *va, fom_int r0, fom_int r1, fom_int ncols, fom_int c_lo, fom_int c_hi, fom_int dummy, fom_int *q, fom_int *nc) {
+    fom_int i, lo, hi, k, n, maxl, np = 1, pb[IT_NP + 1];
+    int pi;
+    for (i = r0; i < r1; ++i) {
+        it_cur[i] = rp[i];
+        for (k = rp[i] + 1; k < rp[i + 1]; ++k) if (ci[k] <= ci[k - 1]) return 0;
+        if (rp[i + 1] > rp[i] && (ci[rp[i]] < 0 || ci[rp[i + 1] - 1] >= ncols)) return 0;
+        while (it_cur[i] < rp[i + 1] && ci[it_cur[i]] < c_lo) ++it_cur[i];
+    }
+    /* equal panels of at most IT_W columns over [c_lo, c_hi) */
+    while (np * IT_W < c_hi - c_lo) ++np;
+    if (np > IT_NP) return 0;
+    it_split(c_hi - c_lo, np, pb);
+    it_pn = np;
+    for (pi = 0; pi < np; ++pi) {
+        lo = c_lo + pb[pi];
+        hi = c_lo + pb[pi + 1];
+        it_pc[pi] = *nc; it_plo[pi] = lo; it_phi[pi] = hi;
+        maxl = 0;
+        for (i = r0; i < r1; ++i) {
+            fom_int c = it_cur[i];
+            it_seg[i] = c;
+            while (c < rp[i + 1] && ci[c] < hi) ++c;
+            it_len[i] = c - it_seg[i];
+            it_cur[i] = c;
+            if (it_len[i] > maxl) maxl = it_len[i];
+        }
+        /* counting sort by segment length, longest first */
+        for (k = 0; k <= maxl + 1; ++k) it_bkt[k] = 0;
+        for (i = r0; i < r1; ++i) ++it_bkt[maxl - it_len[i] + 1];
+        for (k = 1; k <= maxl + 1; ++k) it_bkt[k] += it_bkt[k - 1];
+        for (i = r0; i < r1; ++i) it_ord[it_bkt[maxl - it_len[i]]++] = i;
+        n = r1 - r0;
+        for (k = 0; k < n; k += IT_C) {
+            fom_int L = it_len[it_ord[k]], s, r, p;
+            if (L == 0) break;
+            if (*nc >= IT_CCAP || *q + L * IT_C > IT_CAP) return 0;
+            it_cp[*nc] = *q; it_cl[*nc] = L; it_co[*nc] = lo;
+            for (r = 0; r < IT_C; ++r) it_cr[*nc][r] = (unsigned short)(k + r < n ? it_ord[k + r] : dummy);
+            p = *q;
+            for (s = 0; s < L; ++s) {
+                for (r = 0; r < IT_C; ++r) {
+                    fom_int row = (k + r < n) ? it_ord[k + r] : -1;
+                    unsigned long long w = 0;
+                    if ((p & 3) == 0) it_si[p >> 2] = 0;
+                    if (row >= 0 && s < it_len[row]) {
+                        w = (unsigned long long)(ci[it_seg[row] + s] - lo);
+                        it_sv[p] = va[it_seg[row] + s];
+                    } else {
+                        it_sv[p] = 0.0;
+                    }
+                    it_si[p >> 2] |= w << (16 * (p & 3));
+                    ++p;
+                }
+            }
+            *q = p;
+            ++*nc;
+        }
+    }
+    it_pc[np] = *nc;
+    for (i = r0; i < r1; ++i) if (it_cur[i] != rp[i + 1] && ci[it_cur[i]] < c_hi) return 0;
+    return 1;
+}
+
+/* Phase 1 runs on an R x Cb thread grid: thread t = cb * R + rb owns rows
+   block rb of t0_state and columns block cb, so it reads only 1/Cb of the
+   state (written by other cores in the previous phase 2: pulling those
+   lines dominates a thread's cost) and the column block is the union of the
+   phase-2 row blocks of threads cb*R .. cb*R+R-1, its own among them. The
+   Cb partial sums of a row are then added in a short reduction pass. */
+static int it_build_all(void) {
+    fom_int q = 0, nc = 0, po = 0;
+    fom_int rb1[IT_NT + 1];
+    int t, cb = 1, rb;
+    it_nt = 1;
+#ifdef _OPENMP
+    it_nt = omp_get_max_threads();
+#endif
+    if (it_nt > IT_NT) it_nt = IT_NT;
+    if (it_nt < 1) it_nt = 1;
+    while (cb * 2 <= IT_CBMAX && (it_nt & (cb * 2 - 1)) == 0 && cb * 2 < it_nt) cb *= 2;
+    it_R = 1;
+    while (it_R * cb < it_nt) ++it_R;
+    if (it_R * cb != it_nt) { cb = 1; it_R = it_nt; }
+    it_split(10000, it_nt, it_r1);
+    it_split(40000, it_nt, it_r2);
+    it_split(10000, it_R, rb1);
+    for (t = 0; t < it_nt; ++t) {
+        int c = 0;
+        rb = t;
+        while (rb >= it_R) { rb -= it_R; ++c; }
+        it_q0[t] = rb1[rb]; it_q1[t] = rb1[rb + 1];
+        it_c1[t] = nc;
+        if (!it_build(FOM_MAT_t0_state_ROWPTR, FOM_MAT_t0_state_COLIDX, FOM_MAT_t0_state_VALS, rb1[rb], rb1[rb + 1], 40000, it_r2[c * it_R], it_r2[c * it_R + it_R], 10000 + IT_C * t, &q, &nc)) return 0;
+        {
+            int j;
+            it_p1n[t] = it_pn;
+            for (j = 0; j < it_pn; ++j) { it_p1c[t][j] = it_pc[j]; it_p1lo[t][j] = it_plo[j]; it_p1hi[t][j] = it_phi[j]; }
+            it_p1c[t][it_pn] = it_pc[it_pn];
+        }
+        it_o1[t] = po;
+        if (!it_build_pos(rb1[rb], rb1[rb + 1], &po)) it_pos_ok = -1;
+    }
+    it_c1[it_nt] = nc;
+    for (t = 0; t < it_nt; ++t) {
+        it_c2[t] = nc;
+        if (!it_build(FOM_MAT_t1_t_t0_ROWPTR, FOM_MAT_t1_t_t0_COLIDX, FOM_MAT_t1_t_t0_VALS, it_r2[t], it_r2[t + 1], 10000, 0, 10000, 40000 + IT_C * t, &q, &nc)) return 0;
+        {
+            int j;
+            it_p2n[t] = it_pn;
+            for (j = 0; j < it_pn; ++j) { it_p2c[t][j] = it_pc[j]; it_p2lo[t][j] = it_plo[j]; it_p2hi[t][j] = it_phi[j]; }
+            it_p2c[t][it_pn] = it_pc[it_pn];
+        }
+        it_o2[t] = po;
+        if (!it_build_pos(it_r2[t], it_r2[t + 1], &po)) it_pos_ok = -1;
+    }
+    it_c2[it_nt] = nc;
+    it_pos_ok = it_pos_ok == 0;
+    return 1;
+}
+
+/* acc[row] += sum over the chunks [cb, ce) of their rows' panel products.
+   With GCC/Clang vector extensions lanes are paired into 2-wide vectors
+   (plain SSE2 on x86-64): one packed load of values, two gathered sources
+   and one packed multiply-add per lane pair. */
+#if defined(__GNUC__)
+typedef fom_float it_v2 __attribute__((vector_size(2 * sizeof(fom_float))));
+#endif
+#if defined(__GNUC__) && defined(__x86_64__)
+#include <immintrin.h>
+#define IT_HAVE_AVX2 1
+static int it_avx2 = 0;
+static int it_avx512 = 0;
+/* AVX2/FMA chunk kernel (runtime-dispatched; the SSE2 loop below is the
+   portable fallback): IT_C / 4 ymm accumulators per chunk, values loaded
+   256 bits at a time straight into the FMA, sources loaded as scalars
+   (hardware gathers are slow on AMD) and paired with one insert. The 16-bit
+   indices are unpacked from 64-bit words with BMI1 bextr instead of being
+   loaded one by one: the load ports are the bottleneck (12 loads per 8
+   nonzeros this way, 18 with 16-bit loads). */
+#define IT_G4(k, xl) _mm256_insertf128_pd(_mm256_castpd128_pd256( \
+        _mm_loadh_pd(_mm_load_sd((xl) + ((k) & 0xFFFF)), (xl) + _bextr_u64((k), 16, 16))), \
+        _mm_loadh_pd(_mm_load_sd((xl) + _bextr_u64((k), 32, 16)), (xl) + ((k) >> 48)), 1)
+__attribute__((target("avx2,fma,bmi")))
+static void it_sell_apply_avx2(fom_int cb, fom_int ce, const fom_float *src, fom_float *acc) {
+    fom_int c, s;
+    int r;
+    for (c = cb; c < ce; ++c) {
+        const fom_float *xl = src + it_co[c];
+        const unsigned long long *ix = it_si + (it_cp[c] >> 2);
+        const fom_float *vv = it_sv + it_cp[c];
+        const unsigned short *cr = it_cr[c];
+        const fom_int L = it_cl[c];
+        __m256d a[IT_C / 4];
+        for (r = 0; r < IT_C / 4; ++r) a[r] = _mm256_setzero_pd();
+        for (s = 0; s < L; ++s) {
+            for (r = 0; r < IT_C / 4; ++r) {
+                const unsigned long long k = ix[r];
+                a[r] = _mm256_fmadd_pd(_mm256_loadu_pd(vv + 4 * r), IT_G4(k, xl), a[r]);
+            }
+            ix += IT_C / 4; vv += IT_C;
+        }
+        for (r = 0; r < IT_C / 4; ++r) {
+            const __m128d q0 = _mm256_castpd256_pd128(a[r]), q1 = _mm256_extractf128_pd(a[r], 1);
+            acc[cr[4 * r]] += _mm_cvtsd_f64(q0); acc[cr[4 * r + 1]] += _mm_cvtsd_f64(_mm_unpackhi_pd(q0, q0));
+            acc[cr[4 * r + 2]] += _mm_cvtsd_f64(q1); acc[cr[4 * r + 3]] += _mm_cvtsd_f64(_mm_unpackhi_pd(q1, q1));
+        }
+    }
+}
+/* As it_sell_apply_avx2, but chunk c's lane sums go to it_ob[c * IT_C ..]. */
+__attribute__((target("avx2,fma,bmi")))
+static void it_sell_store_avx2(fom_int cb, fom_int ce, const fom_float *src) {
+    fom_int c, s;
+    int r;
+    for (c = cb; c < ce; ++c) {
+        const fom_float *xl = src + it_co[c];
+        const unsigned long long *ix = it_si + (it_cp[c] >> 2);
+        const fom_float *vv = it_sv + it_cp[c];
+        const fom_int L = it_cl[c];
+        fom_float *ob = it_ob + c * IT_C;
+        __m256d a[IT_C / 4];
+        for (r = 0; r < IT_C / 4; ++r) a[r] = _mm256_setzero_pd();
+        for (s = 0; s < L; ++s) {
+            for (r = 0; r < IT_C / 4; ++r) {
+                const unsigned long long k = ix[r];
+                a[r] = _mm256_fmadd_pd(_mm256_loadu_pd(vv + 4 * r), IT_G4(k, xl), a[r]);
+            }
+            ix += IT_C / 4; vv += IT_C;
+        }
+        for (r = 0; r < IT_C / 4; ++r) _mm256_storeu_pd(ob + 4 * r, a[r]);
+    }
+}
+#endif
+static void it_sell_store(fom_int cb, fom_int ce, const fom_float *src) {
+#ifdef IT_HAVE_AVX2
+    if (it_avx2) { it_sell_store_avx2(cb, ce, src); return; }
+#endif
+    {
+        fom_int c, s;
+        int r;
+        for (c = cb; c < ce; ++c) {
+            const fom_float *xl = src + it_co[c];
+            const unsigned long long *ix = it_si + (it_cp[c] >> 2);
+            const fom_float *vv = it_sv + it_cp[c];
+            const fom_int L = it_cl[c];
+            fom_float aa[IT_C];
+            for (r = 0; r < IT_C; ++r) aa[r] = 0.0;
+            for (s = 0; s < L; ++s) {
+                for (r = 0; r < IT_C; ++r) aa[r] += vv[r] * xl[(ix[r >> 2] >> (16 * (r & 3))) & 0xFFFF];
+                ix += IT_C / 4;
+                vv += IT_C;
+            }
+            for (r = 0; r < IT_C; ++r) it_ob[c * IT_C + r] = aa[r];
+        }
+    }
+}
+
+/* Sum of a row's np panel sums. */
+static inline fom_float it_rowsum(const fom_int *pp, int np) {
+    fom_float s0 = 0.0, s1 = 0.0;
+    int p = 0;
+    for (; p + 1 < np; p += 2) { s0 += it_ob[pp[p]]; s1 += it_ob[pp[p + 1]]; }
+    if (p < np) s0 += it_ob[pp[p]];
+    return s0 + s1;
+}
+
+static void it_sell_apply(fom_int cb, fom_int ce, const fom_float *src, fom_float *acc) {
+#ifdef IT_HAVE_AVX2
+    if (it_avx2) { it_sell_apply_avx2(cb, ce, src, acc); return; }
+#endif
+
+    fom_int c, s;
+    for (c = cb; c < ce; ++c) {
+        const fom_float *xl = src + it_co[c];
+        const unsigned long long *ix = it_si + (it_cp[c] >> 2);
+        const fom_float *vv = it_sv + it_cp[c];
+        const unsigned short *cr = it_cr[c];
+        const fom_int L = it_cl[c];
+#if defined(__GNUC__)
+        it_v2 bb[IT_C / 2];
+        int r;
+        for (r = 0; r < IT_C / 2; ++r) bb[r] = (it_v2){0.0, 0.0};
+        for (s = 0; s < L; ++s) {
+            for (r = 0; r < IT_C / 4; ++r) {
+                it_v2 w0, w1;
+                const unsigned long long k = ix[r];
+                it_v2 g0 = {xl[k & 0xFFFF], xl[(k >> 16) & 0xFFFF]}, g1 = {xl[(k >> 32) & 0xFFFF], xl[k >> 48]};
+                __builtin_memcpy(&w0, vv + 4 * r, sizeof w0);
+                __builtin_memcpy(&w1, vv + 4 * r + 2, sizeof w1);
+                bb[2 * r] += w0 * g0; bb[2 * r + 1] += w1 * g1;
+            }
+            ix += IT_C / 4;
+            vv += IT_C;
+        }
+        for (r = 0; r < IT_C / 2; ++r) { acc[cr[2 * r]] += bb[r][0]; acc[cr[2 * r + 1]] += bb[r][1]; }
+#else
+        fom_float aa[IT_C];
+        int r;
+        for (r = 0; r < IT_C; ++r) aa[r] = 0.0;
+        for (s = 0; s < L; ++s) {
+            for (r = 0; r < IT_C; ++r) aa[r] += vv[r] * xl[(ix[r >> 2] >> (16 * (r & 3))) & 0xFFFF];
+            ix += IT_C / 4;
+            vv += IT_C;
+        }
+        for (r = 0; r < IT_C; ++r) acc[cr[r]] += aa[r];
+#endif
+    }
+}
+
+/* One iteration as executed by thread tid of an nth-thread team (every
+   barrier is orphaned, so this runs inside any enclosing parallel region).
+   orph: the team outlives this call, so end with a barrier before the
+   next iteration reads new_state. */
+static void it_sell_body(int tid, int nth, int orph, const fom_float *state, const fom_float *bias_t0, const fom_float *bias_t1, const fom_float *bias_su, fom_float l0, fom_float l1, int su_id, fom_float *t_t0, fom_float *t_t1, fom_float *new_state) {
+    const int ntp = it_nt;
+    const int t1s_id = it_t1s_id;
+    {
+        int t;
+        fom_int i, p;
+        FOM_PROF_BEGIN(sell_t0_state);
+        for (t = tid; t < ntp; t += nth) {
+            fom_float *z = it_zp[t];
+            for (i = it_q0[t]; i < it_q1[t]; ++i) z[i] = 0.0;
+            it_sell_apply(it_c1[t], it_c1[t + 1], state, z);
+        }
+        if (tid == 0) { FOM_PROF_END(sell_t0_state); }
+        FOM_PROF_BEGIN(barrier_wait);
+        #ifdef _OPENMP
+        #pragma omp barrier
+        #endif
+        if (tid == 0) { FOM_PROF_END(barrier_wait); }
+        FOM_PROF_BEGIN(t0_reduce);
+        for (t = tid; t < ntp; t += nth) {
+            const fom_int s0 = it_r1[t], s1 = it_r1[t + 1];
+            int u;
+            for (i = s0; i < s1; ++i) t_t0[i] = bias_t0[i];
+            for (u = 0; u < ntp; ++u) {
+                const fom_float *z = it_zp[u];
+                fom_int a = it_q0[u] > s0 ? it_q0[u] : s0, b = it_q1[u] < s1 ? it_q1[u] : s1;
+                for (i = a; i < b; ++i) t_t0[i] += z[i];
+            }
+            for (i = s0; i < s1; ++i) {
+                fom_float v = t_t0[i];
+                v = (v > l0) ? (v - l0) : ((v < -l0) ? (v + l0) : 0.0);
+                t_t0[i] = v;
+            }
+        }
+        #ifdef _OPENMP
+        #pragma omp barrier
+        #endif
+        if (tid == 0) { FOM_PROF_END(t0_reduce); }
+        FOM_PROF_BEGIN(sell_t1_t_t0);
+        for (t = tid; t < ntp; t += nth) {
+            for (i = it_r2[t]; i < it_r2[t + 1]; ++i) it_z1[i] = 0.0;
+            it_sell_apply(it_c2[t], it_c2[t + 1], t_t0, it_z1);
+            for (i = it_r2[t]; i < it_r2[t + 1]; ++i) {
+                fom_float v = bias_t1[i], s = 0.0;
+                if (t1s_id) {
+                    s = state[i];
+                } else {
+                    for (p = FOM_MAT_t1_state_ROWPTR[i]; p < FOM_MAT_t1_state_ROWPTR[i + 1]; ++p) s += FOM_MAT_t1_state_VALS[p] * state[FOM_MAT_t1_state_COLIDX[p]];
+                }
+                v += s;
+                v += it_z1[i];
+                v = (v > l1) ? (v - l1) : ((v < -l1) ? (v + l1) : 0.0);
+                t_t1[i] = v;
+                if (su_id) new_state[i] = bias_su[i] + v;
+            }
+        }
+        if (tid == 0) { FOM_PROF_END(sell_t1_t_t0); }
+        if (!su_id) {
+            #ifdef _OPENMP
+            #pragma omp barrier
+            #pragma omp for schedule(static)
+            #endif
+            for (i = 0; i < 40000; ++i) {
+                fom_float s = 0.0;
+                for (p = FOM_MAT_su_t_t1_ROWPTR[i]; p < FOM_MAT_su_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t1_VALS[p] * t_t1[FOM_MAT_su_t_t1_COLIDX[p]];
+                new_state[i] = bias_su[i] + s;
+            }
+        }
+        #ifdef _OPENMP
+        if (orph) {
+            #pragma omp barrier
+        }
+        #endif
+        (void)orph;
+    }
+}
+
+#if defined(__GNUC__) && defined(_OPENMP)
+#define IT_FLOW 1
+/* ---- point-to-point iteration ("flow") ------------------------------
+   Inside the solver's persistent team the three barriers of an iteration
+   become waits on per-thread epoch flags, each thread waiting only for the
+   threads it reads from, so a late thread delays its consumers instead of
+   the whole team:
+     phase 1 (e) of t   waits phase 2 (e-1) of the owners of its state block;
+     reduce  (e) of t   waits phase 1 (e) of the threads covering its rows;
+     phase 2 (e) of t   waits every reduce (e).
+   Phase 2 reads t0 from an internal double buffer (the t_t0 output is
+   written alongside but never read back): reduce (e+1) may only overwrite
+   the buffer phase 2 (e-1) read, and every thread passed phase 2 (e-1)
+   before it signalled reduce (e). Partials and state are covered by the
+   same chain (see the hazards noted at each wait). */
+typedef struct { int v; char pad[60]; } it_flag;
+static it_flag it_f1[IT_NT], it_fr[IT_NT], it_f2[IT_NT], it_ep[IT_NT];
+static fom_float it_t0b[2][10000];
+
+static void it_wait(const it_flag *f, int e) {
+    while (__atomic_load_n(&f->v, __ATOMIC_ACQUIRE) < e) {
+#if defined(__x86_64__) || defined(__i386__)
+        __builtin_ia32_pause();
+#endif
+    }
+}
+
+static void it_flow_body(int tid, const fom_float *state, const fom_float *bias_t0, const fom_float *bias_t1, const fom_float *bias_su, fom_float l0, fom_float l1, fom_float *t_t0, fom_float *t_t1, fom_float *new_state) {
+    const int ntp = it_nt, t = tid;
+    const int e = it_ep[t].v + 1;
+    fom_float *tb = it_t0b[e & 1];
+    fom_int i;
+    int u;
+    it_ep[t].v = e;
+    /* phase 1: needs state rows [lo, hi) from the phase-2 owners; it_zp[t]
+       was last read by reduce (e-1), which every thread finished before
+       this thread passed its phase-2 (e-1) wait */
+    {
+        fom_float *z = it_zp[t];
+        const int np = it_p1n[t];
+        int j, j0 = 0;
+        FOM_PROF_BEGIN(sell_t0_state);
+        /* panel by panel, each once the owners of its state rows are done,
+           starting with the panel holding this thread's own rows */
+        while (j0 + 1 < np && it_p1hi[t][j0] <= it_r2[t]) ++j0;
+        for (j = 0; j < np; ++j) {
+            int jj = j0 + j;
+            fom_int lo, hi;
+            if (jj >= np) jj -= np;
+            lo = it_p1lo[t][jj]; hi = it_p1hi[t][jj];
+            for (u = 0; u < ntp; ++u)
+                if (it_r2[u] < hi && it_r2[u + 1] > lo) it_wait(&it_f2[u], e - 1);
+            it_sell_store(it_p1c[t][jj], it_p1c[t][jj + 1], state);
+        }
+        {
+            const fom_int *pp = it_pos + it_o1[t], q0 = it_q0[t];
+            for (i = q0; i < it_q1[t]; ++i) z[i] = it_rowsum(pp + (i - q0) * np, np);
+        }
+        __atomic_store_n(&it_f1[t].v, e, __ATOMIC_RELEASE);
+        if (tid == 0) { FOM_PROF_END(sell_t0_state); }
+    }
+    /* reduce: rows [s0, s1) of t0 from the partials covering them */
+    {
+        const fom_int s0 = it_r1[t], s1 = it_r1[t + 1];
+        FOM_PROF_BEGIN(t0_reduce);
+        for (u = 0; u < ntp; ++u)
+            if (it_q0[u] < s1 && it_q1[u] > s0) {
+                fom_int a = it_q0[u] > s0 ? it_q0[u] : s0, b = it_q1[u] < s1 ? it_q1[u] : s1;
+                it_wait(&it_f1[u], e);
+                for (i = a; i < b; i += 8) __builtin_prefetch(it_zp[u] + i, 0, 3);
+            }
+        for (i = s0; i < s1; ++i) tb[i] = bias_t0[i];
+        for (u = 0; u < ntp; ++u) {
+            const fom_float *z = it_zp[u];
+            fom_int a = it_q0[u] > s0 ? it_q0[u] : s0, b = it_q1[u] < s1 ? it_q1[u] : s1;
+            for (i = a; i < b; ++i) tb[i] += z[i];
+        }
+        for (i = s0; i < s1; ++i) {
+            fom_float v = tb[i];
+            v = (v > l0) ? (v - l0) : ((v < -l0) ? (v + l0) : 0.0);
+            tb[i] = v;
+            t_t0[i] = v;
+        }
+        __atomic_store_n(&it_fr[t].v, e, __ATOMIC_RELEASE);
+        if (tid == 0) { FOM_PROF_END(t0_reduce); }
+    }
+    /* phase 2: all of t0; writes only this thread's rows of t1/new_state
+       (new_state rows were last read by phase 1 (e-1), finished by every
+       thread before it signalled reduce (e-1)... and reduce (e)) */
+    {
+        const int np = it_p2n[t];
+        int j, j0 = 0;
+        FOM_PROF_BEGIN(sell_t1_t_t0);
+        /* panel by panel, each once the reducers of its t0 rows are done,
+           starting with the panel of this thread's own slice */
+        while (j0 + 1 < np && it_p2hi[t][j0] <= it_r1[t]) ++j0;
+        for (j = 0; j < np; ++j) {
+            int jj = j0 + j;
+            fom_int lo, hi;
+            if (jj >= np) jj -= np;
+            lo = it_p2lo[t][jj]; hi = it_p2hi[t][jj];
+            for (u = 0; u < ntp; ++u)
+                if (it_r1[u] < hi && it_r1[u + 1] > lo) it_wait(&it_fr[u], e);
+            it_sell_store(it_p2c[t][jj], it_p2c[t][jj + 1], tb);
+        }
+        /* every reduce (e) is done (normally already seen panel by panel):
+           the hazards above rely on it */
+        FOM_PROF_BEGIN(barrier_wait);
+        for (u = 0; u < ntp; ++u) it_wait(&it_fr[u], e);
+        if (tid == 0) { FOM_PROF_END(barrier_wait); }
+        for (i = it_r2[t]; i < it_r2[t + 1]; ++i) {
+            fom_float v = bias_t1[i] + state[i];
+            v += it_rowsum(it_pos + it_o2[t] + (i - it_r2[t]) * np, np);
+            v = (v > l1) ? (v - l1) : ((v < -l1) ? (v + l1) : 0.0);
+            t_t1[i] = v;
+            new_state[i] = bias_su[i] + v;
+        }
+        __atomic_store_n(&it_f2[t].v, e, __ATOMIC_RELEASE);
+        if (tid == 0) { FOM_PROF_END(sell_t1_t_t0); }
+    }
+}
+
+/* ---- shared-storage iteration ("tr") --------------------------------
+   t1_t_t0 is checked once (exactly, from both matrices' data) to be
+   c * t0_state^T: the same pattern, every value c times its transposed
+   partner. One copy of t0_state, stored by columns, then serves both
+   products, halving the matrix bytes streamed per iteration. Thread t owns
+   state rows J_t (= columns J_t of t0_state), in SELL chunks of IT_C
+   columns sorted by length. Its fused pass per chunk takes the dot
+   products with c * t0 (gathers), applies the activations of t1 and the
+   state update, and at once, with the chunk still in L1, scatters
+   new_state[J_t] into its private partial P_t = t0_state[:, J_t] @
+   new_state[J_t]. The next call sums the 16 partials into t0, so a thread
+   never reads another's state rows. A partial is reused only if the state
+   block the next call receives equals what was written (kept in tr_sh);
+   otherwise it is recomputed from the given state. Padding entries point
+   at row 10000 of c * t0 (held at 0.0) and of the partials (never read). */
+#define TR_N0 10000
+#define TR_CAP (FOM_MAT_t0_state_NNZ + FOM_MAT_t0_state_NNZ / 4 + 65536)
+#define TR_CCAP 32768
+#define TR_PZ (TR_N0 + 8)
+#ifndef TR_NP
+#define TR_NP 4
+#endif
+#ifndef TR_G
+#define TR_G 1024
+#endif
+#define TR_GCAP 1024
+/* rows per block of the two-level reduce's stage A; TR_PIPE 0 turns off
+   the pipelined fused pass */
+#ifndef TR_RB
+#define TR_RB 128
+#endif
+#ifndef TR_PIPE
+#define TR_PIPE 1
+#endif
+static int tr_ok = 0;
+#define IT_CAL 6
+static it_flag it_cc[IT_NT];
+static double it_t_mark, it_t_tr;
+static int it_use_tr = 1;
+static fom_float tr_c;
+static unsigned long long tr_si[TR_CAP / 4 + 4];
+static fom_float tr_sv[TR_CAP];
+static fom_int tr_cp[TR_CCAP], tr_cl[TR_CCAP];
+static unsigned short tr_cr[TR_CCAP][IT_C];
+/* thread t owns groups tr_tg[t] .. tr_tg[t+1]; group g is state rows
+   tr_gs[g] .. tr_gs[g] + tr_gn[g], and its chunks tr_gc[g] .. tr_gc[g+1]
+   run panel by panel (lane rows are group-local, TR_G for empty lanes) */
+static fom_int tr_tg[IT_NT + 1], tr_gs[TR_GCAP], tr_gn[TR_GCAP], tr_gc[TR_GCAP + 1];
+static fom_int tr_atp[40001];
+static fom_int tr_ati[FOM_MAT_t0_state_NNZ];
+static fom_float tr_atv[FOM_MAT_t0_state_NNZ];
+static fom_float tr_P[2][IT_NT][TR_PZ] __attribute__((aligned(64)));
+static fom_float tr_tb[TR_PZ] __attribute__((aligned(64)));
+static fom_float tr_sh[40000];
+static it_flag tr_f1[IT_NT], tr_fr[IT_NT], tr_ep[IT_NT], tr_hv[IT_NT];
+/* Two-level reduce: the team is split into two halves (with close binding,
+   one per L3 cache domain). Stage A: thread l of half h sums its half's
+   partials over rows tr_hr[l] .. tr_hr[l+1] into tr_S[e & 1][h]. Stage B:
+   it adds the partner half's stage-A rows (thread l of the other half) and
+   writes c * t0 for those rows into its half's own copy tr_tbh[h], so the
+   fused pass never reads t0 across halves. tr_H = 0 keeps the flat reduce. */
+static int tr_H = 0;
+static fom_int tr_hr[IT_NT + 1];
+static fom_float tr_S[2][2][TR_PZ] __attribute__((aligned(64)));
+static fom_float tr_tbh[2][TR_PZ] __attribute__((aligned(64)));
+static it_flag tr_fa[IT_NT];
+
+/* 1 / a for a > 0 without a division: Newton on the mantissa m in
+   [0.5, 1) from 1.0, where the error 1 - m y squares every step. */
+static fom_float tr_recip(fom_float a) {
+    int e, k;
+    fom_float m = frexp(a, &e), y = 1.0;
+    for (k = 0; k < 8; ++k) y = y * (2.0 - m * y);
+    return ldexp(y, -e);
+}
+
+static int tr_build(void) {
+    const fom_int *rp = FOM_MAT_t0_state_ROWPTR, *ci = FOM_MAT_t0_state_COLIDX;
+    const fom_float *va = FOM_MAT_t0_state_VALS;
+    const fom_int *bp = FOM_MAT_t1_t_t0_ROWPTR, *bi = FOM_MAT_t1_t_t0_COLIDX;
+    const fom_float *bv = FOM_MAT_t1_t_t0_VALS;
+    fom_int nnz, i, j, k, q = 0, nc = 0;
+    fom_float a = 0.0, b = 0.0, cand[3];
+    int t, h, found = -1;
+    if (rp[0] != 0 || bp[0] != 0) return 0;
+    for (i = 0; i < 10000; ++i) if (rp[i + 1] < rp[i]) return 0;
+    nnz = rp[10000];
+    if (nnz > FOM_MAT_t0_state_NNZ || bp[40000] != nnz) return 0;
+    for (j = 0; j <= 40000; ++j) tr_atp[j] = 0;
+    for (k = 0; k < nnz; ++k) { if (ci[k] < 0 || ci[k] >= 40000) return 0; ++tr_atp[ci[k] + 1]; }
+    for (j = 0; j < 40000; ++j) tr_atp[j + 1] += tr_atp[j];
+    for (j = 0; j <= 40000; ++j) if (tr_atp[j] != bp[j]) return 0;
+    for (j = 0; j < 40000; ++j) it_cur[j] = tr_atp[j];
+    for (i = 0; i < 10000; ++i)
+        for (k = rp[i]; k < rp[i + 1]; ++k) { fom_int d = it_cur[ci[k]]++; tr_ati[d] = i; tr_atv[d] = va[k]; }
+    for (k = 0; k < nnz; ++k) if (bi[k] != tr_ati[k]) return 0;
+    for (k = 0; k < nnz; ++k) {
+        fom_float m = tr_atv[k] < 0.0 ? -tr_atv[k] : tr_atv[k];
+        if (m > a) { a = m; b = tr_atv[k] < 0.0 ? -bv[k] : bv[k]; }
+    }
+    if (!(a > 0.0)) return 0;
+    cand[0] = b * tr_recip(a);
+    cand[1] = nextafter(cand[0], 0.0);
+    cand[2] = nextafter(cand[0], 2.0 * cand[0]);
+    for (h = 0; h < 3 && found < 0; ++h) {
+        for (k = 0; k < nnz; ++k) if (!(bv[k] == cand[h] * tr_atv[k])) break;
+        if (k == nnz) found = h;
+    }
+    if (found < 0) return 0;
+    tr_c = cand[found];
+    {
+        fom_int pb[TR_NP + 1], gb[TR_GCAP + 1];
+        int ng = 0;
+        it_split(TR_N0, TR_NP, pb);
+        for (j = 0; j < 40000; ++j) it_cur[j] = tr_atp[j];
+        for (t = 0; t < it_nt; ++t) {
+            const fom_int r0 = it_r2[t], r1 = it_r2[t + 1];
+            int m = 1, gi, pi;
+            while (m * TR_G < r1 - r0) ++m;
+            if (ng + m > TR_GCAP) return 0;
+            it_split(r1 - r0, m, gb);
+            tr_tg[t] = ng;
+            for (gi = 0; gi < m; ++gi, ++ng) {
+                const fom_int g0 = r0 + gb[gi], g1 = r0 + gb[gi + 1], n = g1 - g0;
+                tr_gs[ng] = g0; tr_gn[ng] = n; tr_gc[ng] = nc;
+                for (pi = 0; pi < TR_NP; ++pi) {
+                    const fom_int hi = pb[pi + 1];
+                    fom_int maxl = 0;
+                    for (j = g0; j < g1; ++j) {
+                        fom_int cc = it_cur[j];
+                        it_seg[j] = cc;
+                        while (cc < tr_atp[j + 1] && tr_ati[cc] < hi) ++cc;
+                        it_len[j] = cc - it_seg[j];
+                        it_cur[j] = cc;
+                        if (it_len[j] > maxl) maxl = it_len[j];
+                    }
+                    for (k = 0; k <= maxl + 1; ++k) it_bkt[k] = 0;
+                    for (j = g0; j < g1; ++j) ++it_bkt[maxl - it_len[j] + 1];
+                    for (k = 1; k <= maxl + 1; ++k) it_bkt[k] += it_bkt[k - 1];
+                    for (j = g0; j < g1; ++j) it_ord[it_bkt[maxl - it_len[j]]++] = j;
+                    for (k = 0; k < n; k += IT_C) {
+                        const fom_int L = it_len[it_ord[k]];
+                        fom_int s, p = q;
+                        int r;
+                        if (L == 0) break;
+                        if (nc >= TR_CCAP || q + L * IT_C > TR_CAP) return 0;
+                        tr_cp[nc] = q; tr_cl[nc] = L;
+                        for (r = 0; r < IT_C; ++r) tr_cr[nc][r] = (unsigned short)((k + r < n) ? it_ord[k + r] - g0 : TR_G);
+                        for (s = 0; s < L; ++s)
+                            for (r = 0; r < IT_C; ++r) {
+                                fom_int row = (k + r < n) ? it_ord[k + r] : -1;
+                                unsigned long long w = TR_N0;
+                                if ((p & 3) == 0) tr_si[p >> 2] = 0;
+                                tr_sv[p] = 0.0;
+                                if (row >= 0 && s < it_len[row]) {
+                                    w = (unsigned long long)tr_ati[it_seg[row] + s];
+                                    tr_sv[p] = tr_atv[it_seg[row] + s];
+                                }
+                                tr_si[p >> 2] |= w << (16 * (p & 3));
+                                ++p;
+                            }
+                        q = p;
+                        ++nc;
+                    }
+                }
+            }
+        }
+        tr_tg[it_nt] = ng;
+        tr_gc[ng] = nc;
+        for (j = 0; j < 40000; ++j) if (it_cur[j] != tr_atp[j + 1]) return 0;
+    }
+    for (i = 0; i < TR_PZ; ++i) tr_tb[i] = tr_tbh[0][i] = tr_tbh[1][i] = 0.0;
+    tr_H = 0;
+    if (it_nt >= 4 && !(it_nt & 1)) {
+        const int H = it_nt / 2;
+        tr_H = H;
+        /* rows split in multiples of 8 (whole cache lines per thread) */
+        for (t = 0; t <= H; ++t) {
+            fom_int b8 = 0, n8 = TR_N0 / 8;
+            fom_int q8 = 0, r8;
+            while ((q8 + 1) * H <= n8) ++q8;
+            r8 = n8 - q8 * H;
+            b8 = t * q8 + (t < r8 ? t : r8);
+            tr_hr[t] = 8 * b8;
+        }
+        tr_hr[H] = TR_N0;
+    }
+    return 1;
+}
+
+/* Groups [gb, ge) of the calling thread. mode 0: P += columns @ state
+   rows. mode 1: the fused pass; per group, the dot products with tb panel
+   by panel (the tb panel stays in L1), then t1, new_state and tr_sh, then
+   the scatter P += columns @ new_state rows panel by panel (the P panel
+   stays in L1; the group's entries are re-read from L2). DOT(c) leaves a
+   chunk's IT_C dot products in ob[]. Instantiated per instruction set. */
+#define TR_RANGE_BODY(DOT, EXT, SCAT) \
+    fom_int g, gi, c, s, L; \
+    int r; \
+    for (gi = 0; gi < ge - gb; ++gi) { \
+        g = rev ? ge - 1 - gi : gb + gi; \
+        const fom_int g0 = tr_gs[g], n = tr_gn[g]; \
+        fom_float xg[TR_G + 1]; \
+        fom_int jj; \
+        if (mode) { \
+            for (jj = 0; jj <= TR_G; ++jj) xg[jj] = 0.0; \
+            for (c = tr_gc[g]; c < tr_gc[g + 1]; ++c) { \
+                fom_float ob[IT_C]; \
+                const unsigned short *cr = tr_cr[c]; \
+                L = tr_cl[c]; \
+                DOT(c); \
+                for (r = 0; r < IT_C; ++r) xg[cr[r]] += ob[r]; \
+            } \
+            for (jj = 0; jj < n; ++jj) { \
+                const fom_int j = g0 + jj; \
+                fom_float v = bias_t1[j] + state[j]; \
+                v += xg[jj]; \
+                v = (v > l1) ? (v - l1) : ((v < -l1) ? (v + l1) : 0.0); \
+                t_t1[j] = v; \
+                v = bias_su[j] + v; \
+                new_state[j] = v; \
+                tr_sh[j] = v; \
+                xg[jj] = v; \
+            } \
+        } else { \
+            for (jj = 0; jj < n; ++jj) xg[jj] = state[g0 + jj]; \
+        } \
+        xg[TR_G] = 0.0; \
+        for (c = tr_gc[g]; c < tr_gc[g + 1]; ++c) { \
+            const unsigned short *cr = tr_cr[c]; \
+            const unsigned long long *ix = tr_si + (tr_cp[c] >> 2); \
+            const fom_float *vv = tr_sv + tr_cp[c]; \
+            fom_float xr[IT_C]; \
+            for (r = 0; r < IT_C; ++r) xr[r] = xg[cr[r]]; \
+            L = tr_cl[c]; \
+            SCAT; \
+        } \
+    }
+#define TR_ARGS fom_int gb, fom_int ge, int rev, int mode, const fom_float *tb, const fom_float *state, const fom_float *bias_t1, const fom_float *bias_su, fom_float l1, fom_float *t_t1, fom_float *new_state, fom_float *P
+
+#define TR_DOT_C(c) do { \
+        const unsigned long long *ix = tr_si + (tr_cp[c] >> 2); \
+        const fom_float *vv = tr_sv + tr_cp[c]; \
+        for (r = 0; r < IT_C; ++r) ob[r] = 0.0; \
+        for (s = 0; s < L; ++s) { \
+            for (r = 0; r < IT_C; ++r) ob[r] += vv[r] * tb[(ix[r >> 2] >> (16 * (r & 3))) & 0xFFFF]; \
+            ix += IT_C / 4; vv += IT_C; \
+        } \
+    } while (0)
+#define TR_EXT_C(k, o) (((k) >> (o)) & 0xFFFF)
+#define TR_SCAT_C \
+            _Pragma("GCC unroll 2") \
+            for (s = 0; s < L; ++s) { \
+                for (r = 0; r < IT_C / 4; ++r) { \
+                    const unsigned long long k = ix[r]; \
+                    P[k & 0xFFFF] += vv[4 * r] * xr[4 * r]; \
+                    P[TR_EXT_C(k, 16)] += vv[4 * r + 1] * xr[4 * r + 1]; \
+                    P[TR_EXT_C(k, 32)] += vv[4 * r + 2] * xr[4 * r + 2]; \
+                    P[k >> 48] += vv[4 * r + 3] * xr[4 * r + 3]; \
+                } \
+                ix += IT_C / 4; vv += IT_C; \
+            }
+static void tr_range_c(TR_ARGS) { TR_RANGE_BODY(TR_DOT_C, TR_EXT_C, TR_SCAT_C) }
+
+#ifdef IT_HAVE_AVX2
+#define TR_EXT_BMI(k, o) _bextr_u64((k), (o), 16)
+#define TR_DOT_AVX(c) do { \
+        const unsigned long long *ix = tr_si + (tr_cp[c] >> 2); \
+        const fom_float *vv = tr_sv + tr_cp[c]; \
+        __m256d a[IT_C / 4]; \
+        for (r = 0; r < IT_C / 4; ++r) a[r] = _mm256_setzero_pd(); \
+        _Pragma("GCC unroll 2") \
+        for (s = 0; s < L; ++s) { \
+            for (r = 0; r < IT_C / 4; ++r) { \
+                const unsigned long long k = ix[r]; \
+                a[r] = _mm256_fmadd_pd(_mm256_loadu_pd(vv + 4 * r), IT_G4(k, tb), a[r]); \
+            } \
+            ix += IT_C / 4; vv += IT_C; \
+        } \
+        for (r = 0; r < IT_C / 4; ++r) _mm256_storeu_pd(ob + 4 * r, a[r]); \
+    } while (0)
+/* scatter with the products formed 4 at a time: one packed multiply, then
+   each lane added to its row (fewer loads than a scalar fma per entry) */
+#define TR_SCAT_AVX do { \
+            __m256d x4[IT_C / 4]; \
+            for (r = 0; r < IT_C / 4; ++r) x4[r] = _mm256_loadu_pd(xr + 4 * r); \
+            _Pragma("GCC unroll 2") \
+            for (s = 0; s < L; ++s) { \
+                for (r = 0; r < IT_C / 4; ++r) { \
+                    const unsigned long long k = ix[r]; \
+                    const __m256d pr = _mm256_mul_pd(_mm256_loadu_pd(vv + 4 * r), x4[r]); \
+                    const __m128d lo = _mm256_castpd256_pd128(pr), hi = _mm256_extractf128_pd(pr, 1); \
+                    P[k & 0xFFFF] += _mm_cvtsd_f64(lo); \
+                    P[TR_EXT_BMI(k, 16)] += _mm_cvtsd_f64(_mm_unpackhi_pd(lo, lo)); \
+                    P[TR_EXT_BMI(k, 32)] += _mm_cvtsd_f64(hi); \
+                    P[k >> 48] += _mm_cvtsd_f64(_mm_unpackhi_pd(hi, hi)); \
+                } \
+                ix += IT_C / 4; vv += IT_C; \
+            } \
+        } while (0)
+__attribute__((target("avx2,fma,bmi")))
+static void tr_range_avx2(TR_ARGS) { TR_RANGE_BODY(TR_DOT_AVX, TR_EXT_BMI, TR_SCAT_AVX) }
+/* same code with 32 vector registers, so xr[] stays in registers */
+__attribute__((target("avx512f,avx512vl,avx2,fma,bmi")))
+static void tr_range_avx512(TR_ARGS) { TR_RANGE_BODY(TR_DOT_AVX, TR_EXT_BMI, TR_SCAT_AVX) }
+#endif
+
+#ifdef IT_HAVE_AVX2
+/* Pipelined fused pass (mode 1): the scatter of group g (store-bound: one
+   random store per entry) runs step by step interleaved with the dot
+   products of the next group g + 1 (load-bound), so both reach the core
+   at once. Chunk i of g is paired with chunk i of g + 1; the scatter of g
+   only needs g's new state, finished before the pair loop starts. */
+#define TR_SSTEP do { \
+        for (r = 0; r < IT_C / 4; ++r) { \
+            const unsigned long long k = ixs[r]; \
+            const __m256d pr = _mm256_mul_pd(_mm256_loadu_pd(vvs + 4 * r), x4[r]); \
+            const __m128d lo = _mm256_castpd256_pd128(pr), hi = _mm256_extractf128_pd(pr, 1); \
+            P[k & 0xFFFF] += _mm_cvtsd_f64(lo); \
+            P[_bextr_u64(k, 16, 16)] += _mm_cvtsd_f64(_mm_unpackhi_pd(lo, lo)); \
+            P[_bextr_u64(k, 32, 16)] += _mm_cvtsd_f64(hi); \
+            P[k >> 48] += _mm_cvtsd_f64(_mm_unpackhi_pd(hi, hi)); \
+        } \
+        ixs += IT_C / 4; vvs += IT_C; \
+    } while (0)
+#define TR_DSTEP do { \
+        for (r = 0; r < IT_C / 4; ++r) { \
+            const unsigned long long k = ixd[r]; \
+            a[r] = _mm256_fmadd_pd(_mm256_loadu_pd(vvd + 4 * r), IT_G4(k, tb), a[r]); \
+        } \
+        ixd += IT_C / 4; vvd += IT_C; \
+    } while (0)
+#define TR_PIPE_BODY \
+    fom_float xa[2][TR_G + 1]; \
+    const int ng = (int)(ge - gb); \
+    int gi; \
+    fom_int jj, s, ls, ld; \
+    int r; \
+    if (ng <= 0) return; \
+    for (gi = -1; gi < ng; ++gi) { \
+        const fom_int g = gi < 0 ? -1 : (rev ? ge - 1 - gi : gb + gi); \
+        const fom_int gn = gi + 1 < ng ? (rev ? ge - 2 - gi : gb + gi + 1) : -1; \
+        fom_float *xs = xa[(gi + 2) & 1], *xd = xa[(gi + 1) & 1]; \
+        fom_int cs = 0, ce = 0, cd = 0, cde = 0; \
+        if (g >= 0) { cs = tr_gc[g]; ce = tr_gc[g + 1]; xs[TR_G] = 0.0; } \
+        if (gn >= 0) { cd = tr_gc[gn]; cde = tr_gc[gn + 1]; for (jj = 0; jj <= TR_G; ++jj) xd[jj] = 0.0; } \
+        while (cs < ce || cd < cde) { \
+            const unsigned long long *ixs = 0, *ixd = 0; \
+            const fom_float *vvs = 0, *vvd = 0; \
+            __m256d x4[IT_C / 4], a[IT_C / 4]; \
+            const int hs = cs < ce, hd = cd < cde; \
+            ls = 0; ld = 0; \
+            if (hs) { \
+                const unsigned short *cr = tr_cr[cs]; \
+                ixs = tr_si + (tr_cp[cs] >> 2); vvs = tr_sv + tr_cp[cs]; ls = tr_cl[cs]; \
+                for (r = 0; r < IT_C / 4; ++r) x4[r] = _mm256_set_pd(xs[cr[4 * r + 3]], xs[cr[4 * r + 2]], xs[cr[4 * r + 1]], xs[cr[4 * r]]); \
+            } \
+            if (hd) { \
+                ixd = tr_si + (tr_cp[cd] >> 2); vvd = tr_sv + tr_cp[cd]; ld = tr_cl[cd]; \
+                for (r = 0; r < IT_C / 4; ++r) a[r] = _mm256_setzero_pd(); \
+            } \
+            { \
+                const fom_int m = ls < ld ? ls : ld; \
+                for (s = 0; s < m; ++s) { TR_SSTEP; TR_DSTEP; } \
+                for (; s < ls; ++s) TR_SSTEP; \
+                for (s = m; s < ld; ++s) TR_DSTEP; \
+            } \
+            if (hs) ++cs; \
+            if (hd) { \
+                fom_float ob[IT_C]; \
+                const unsigned short *cr = tr_cr[cd]; \
+                for (r = 0; r < IT_C / 4; ++r) _mm256_storeu_pd(ob + 4 * r, a[r]); \
+                for (r = 0; r < IT_C; ++r) xd[cr[r]] += ob[r]; \
+                ++cd; \
+            } \
+        } \
+        if (gn >= 0) { \
+            const fom_int g0 = tr_gs[gn], n = tr_gn[gn]; \
+            for (jj = 0; jj < n; ++jj) { \
+                const fom_int j = g0 + jj; \
+                fom_float v = bias_t1[j] + state[j]; \
+                v += xd[jj]; \
+                v = (v > l1) ? (v - l1) : ((v < -l1) ? (v + l1) : 0.0); \
+                t_t1[j] = v; \
+                v = bias_su[j] + v; \
+                new_state[j] = v; \
+                tr_sh[j] = v; \
+                xd[jj] = v; \
+            } \
+        } \
+    }
+#define TR_PARGS fom_int gb, fom_int ge, int rev, const fom_float *tb, const fom_float *state, const fom_float *bias_t1, const fom_float *bias_su, fom_float l1, fom_float *t_t1, fom_float *new_state, fom_float *P
+__attribute__((target("avx2,fma,bmi")))
+static void tr_pipe_avx2(TR_PARGS) { TR_PIPE_BODY }
+__attribute__((target("avx512f,avx512vl,avx2,fma,bmi")))
+static void tr_pipe_avx512(TR_PARGS) { TR_PIPE_BODY }
+#endif
+
+static void tr_range(TR_ARGS) {
+#ifdef IT_HAVE_AVX2
+    if (mode && TR_PIPE) {
+        if (it_avx512) { tr_pipe_avx512(gb, ge, rev, tb, state, bias_t1, bias_su, l1, t_t1, new_state, P); return; }
+        if (it_avx2) { tr_pipe_avx2(gb, ge, rev, tb, state, bias_t1, bias_su, l1, t_t1, new_state, P); return; }
+    }
+    if (it_avx512) { tr_range_avx512(gb, ge, rev, mode, tb, state, bias_t1, bias_su, l1, t_t1, new_state, P); return; }
+    if (it_avx2) { tr_range_avx2(gb, ge, rev, mode, tb, state, bias_t1, bias_su, l1, t_t1, new_state, P); return; }
+#endif
+    tr_range_c(gb, ge, rev, mode, tb, state, bias_t1, bias_su, l1, t_t1, new_state, P);
+}
+
+static void tr_flow_body(int t, const fom_float *state, const fom_float *bias_t0, const fom_float *bias_t1, const fom_float *bias_su, fom_float l0, fom_float l1, fom_float *t_t0, fom_float *t_t1, fom_float *new_state) {
+    const int ntp = it_nt;
+    const int e = tr_ep[t].v + 1;
+    fom_float *P = tr_P[e & 1][t], *Pn = tr_P[(e + 1) & 1][t];
+    fom_int i;
+    int u;
+    tr_ep[t].v = e;
+    /* P_t for this call's state rows J_t: kept from the last fused pass if
+       the rows are what it wrote (other threads last read P_t in reduce
+       (e-1), finished before this thread passed its fused wait (e-1)) */
+    {
+        int ok = tr_hv[t].v == e;
+        FOM_PROF_BEGIN(tr_partial);
+        if (ok) {
+            int diff = 0;
+            for (i = it_r2[t]; i < it_r2[t + 1]; ++i) diff |= !(state[i] == tr_sh[i]);
+            ok = !diff;
+        }
+        if (!ok) {
+            for (i = 0; i < TR_PZ; ++i) P[i] = 0.0;
+            tr_range(tr_tg[t], tr_tg[t + 1], 0, 0, tr_tb, state, bias_t1, bias_su, l1, t_t1, new_state, P);
+        }
+        __atomic_store_n(&tr_f1[t].v, e, __ATOMIC_RELEASE);
+        /* clear the buffer the fused pass fills, while the others finish
+           (last read by reduce (e-1), which every thread finished before
+           this thread passed its fused wait (e-1)) */
+        for (i = 0; i < TR_PZ; ++i) Pn[i] = 0.0;
+        if (t == 0) { FOM_PROF_END(tr_partial); }
+    }
+    /* reduce: rows [s0, s1) of t0 from every partial; tr_tb (c * t0) was
+       last read by fused pass (e-1), which every thread finished before
+       signalling f1 (e) */
+    if (tr_H) {
+        const int H = tr_H, h = t >= H, l = t - h * H, pt = h ? l : l + H;
+        const fom_int s0 = tr_hr[l], s1 = tr_hr[l + 1];
+        fom_float *S = tr_S[e & 1][h];
+        const fom_float *So = tr_S[e & 1][!h];
+        fom_float *tb = tr_tbh[h];
+        /* stage A: the partials of this half (each P_u is read only by
+           its own half, so the hazards of the flat reduce hold per half) */
+        FOM_PROF_BEGIN(barrier_wait);
+        for (u = h * H; u < h * H + H; ++u) it_wait(&tr_f1[u], e);
+        if (t == 0) { FOM_PROF_END(barrier_wait); }
+        FOM_PROF_BEGIN(t0_reduce);
+        /* row blocks, every partial of the half per block: all H source
+           streams are in flight at once (same summation order per row) */
+        {
+            fom_int b0;
+            for (b0 = s0; b0 < s1; b0 += TR_RB) {
+                const fom_int b1 = b0 + TR_RB < s1 ? b0 + TR_RB : s1;
+                const fom_float *z0 = tr_P[e & 1][h * H];
+                for (i = b0; i < b1; ++i) S[i] = z0[i];
+                for (u = h * H + 1; u < h * H + H; ++u) {
+                    const fom_float *z = tr_P[e & 1][u];
+                    for (i = b0; i < b1; ++i) S[i] += z[i];
+                }
+            }
+        }
+        __atomic_store_n(&tr_fa[t].v, e, __ATOMIC_RELEASE);
+        if (t == 0) { FOM_PROF_END(t0_reduce); }
+        FOM_PROF_BEGIN(t0_reduce_partner);
+        /* stage B: the partner's rows; S (e & 1) of the partner was last
+           read by this thread's stage B (e-2), and tb by this half's fused
+           pass (e-1), which ended before every f1 (e) of this half */
+        it_wait(&tr_fa[pt], e);
+        for (i = s0; i < s1; ++i) {
+            fom_float v = bias_t0[i] + (h ? So[i] + S[i] : S[i] + So[i]);
+            v = (v > l0) ? (v - l0) : ((v < -l0) ? (v + l0) : 0.0);
+            tb[i] = tr_c * v;
+            if (h == 0) t_t0[i] = v;
+        }
+        __atomic_store_n(&tr_fr[t].v, e, __ATOMIC_RELEASE);
+        if (t == 0) { FOM_PROF_END(t0_reduce_partner); }
+        {
+            FOM_PROF_BEGIN(barrier_wait);
+            for (u = h * H; u < h * H + H; ++u) it_wait(&tr_fr[u], e);
+            if (t == 0) { FOM_PROF_END(barrier_wait); }
+        }
+        FOM_PROF_BEGIN(tr_fused);
+        tr_range(tr_tg[t], tr_tg[t + 1], e & 1, 1, tb, state, bias_t1, bias_su, l1, t_t1, new_state, Pn);
+        tr_hv[t].v = e + 1;
+        if (t == 0) { FOM_PROF_END(tr_fused); }
+        return;
+    }
+    {
+        const fom_int s0 = it_r1[t], s1 = it_r1[t + 1];
+        FOM_PROF_BEGIN(barrier_wait);
+        for (u = 0; u < ntp; ++u) it_wait(&tr_f1[u], e);
+        if (t == 0) { FOM_PROF_END(barrier_wait); }
+        FOM_PROF_BEGIN(t0_reduce);
+        for (i = s0; i < s1; ++i) tr_tb[i] = bias_t0[i];
+        for (u = 0; u < ntp; ++u) {
+            const fom_float *z = tr_P[e & 1][u];
+            for (i = s0; i < s1; ++i) tr_tb[i] += z[i];
+        }
+        for (i = s0; i < s1; ++i) {
+            fom_float v = tr_tb[i];
+            v = (v > l0) ? (v - l0) : ((v < -l0) ? (v + l0) : 0.0);
+            t_t0[i] = v;
+            tr_tb[i] = tr_c * v;
+        }
+        __atomic_store_n(&tr_fr[t].v, e, __ATOMIC_RELEASE);
+        if (t == 0) { FOM_PROF_END(t0_reduce); }
+    }
+    /* fused pass: needs all of c * t0, and every reduce (e) done with P_t */
+    {
+        FOM_PROF_BEGIN(barrier_wait);
+        for (u = 0; u < ntp; ++u) it_wait(&tr_fr[u], e);
+        if (t == 0) { FOM_PROF_END(barrier_wait); }
+        FOM_PROF_BEGIN(tr_fused);
+        tr_range(tr_tg[t], tr_tg[t + 1], e & 1, 1, tr_tb, state, bias_t1, bias_su, l1, t_t1, new_state, Pn);
+        tr_hv[t].v = e + 1;
+        if (t == 0) { FOM_PROF_END(tr_fused); }
+    }
+}
+#endif
+
+static void it_sell_iterate(const fom_float *state, const fom_float *bias_t0, const fom_float *bias_t1, const fom_float *bias_su, fom_float l0, fom_float l1, int su_id, fom_float *t_t0, fom_float *t_t1, fom_float *new_state) {
+    #ifdef _OPENMP
+    #pragma omp parallel num_threads(it_nt)
+    it_sell_body(omp_get_thread_num(), omp_get_num_threads(), 0, state, bias_t0, bias_t1, bias_su, l0, l1, su_id, t_t0, t_t1, new_state);
+    #else
+    it_sell_body(0, 1, 0, state, bias_t0, bias_t1, bias_su, l0, l1, su_id, t_t0, t_t1, new_state);
+    #endif
+}
+
+static void it_plain_iterate(const fom_float *state, const fom_float *bias_t0, const fom_float *bias_t1, const fom_float *bias_su, fom_float l0, fom_float l1, int su_id, fom_float *t_t0, fom_float *t_t1, fom_float *new_state);
+
+/* One-time setup from the matrix data: structure checks, kernel layouts,
+   instruction-set dispatch. */
+static int it_ready = 0;
+static void it_init_all(void) {
+    it_t1s_id = it_is_identity(FOM_MAT_t1_state_ROWPTR, FOM_MAT_t1_state_COLIDX, FOM_MAT_t1_state_VALS);
+    it_su_id = it_is_identity(FOM_MAT_su_t_t1_ROWPTR, FOM_MAT_su_t_t1_COLIDX, FOM_MAT_su_t_t1_VALS);
+    it_sell = it_build_all();
+#ifdef IT_FLOW
+    tr_ok = it_sell ? tr_build() : 0;
+#endif
+#ifdef IT_HAVE_AVX2
+    __builtin_cpu_init();
+    it_avx2 = __builtin_cpu_supports("avx2") && __builtin_cpu_supports("fma") && __builtin_cpu_supports("bmi");
+    it_avx512 = it_avx2 && __builtin_cpu_supports("avx512f") && __builtin_cpu_supports("avx512vl");
+#endif
+}
+
+/* fom_iterate may be called by every thread of an enclosing parallel
+   region (the solver keeps one team for the whole loop, saving a fork and
+   join per iteration); it then runs as one orphaned team step. */
 void fom_iterate(const fom_float *state, const fom_float *bias_t0, const fom_float *bias_t1, const fom_float *bias_su, const FomAct_t0 *act_t0, const FomAct_t1 *act_t1, fom_float *t_t0, fom_float *t_t1, fom_float *new_state) {
-    fom_step_t0(state, bias_t0, act_t0, t_t0);
-    fom_step_t1(state, t_t0, bias_t1, act_t1, t_t1);
-    fom_step_su(t_t1, bias_su, new_state);
-}
+    const fom_float l0 = act_t0->lambd, l1 = act_t1->lambd;
+    int su_id;
+    if (!__atomic_load_n(&it_ready, __ATOMIC_ACQUIRE)) {
+        #ifdef _OPENMP
+        #pragma omp critical(fom_it_init)
+        #endif
+        {
+            if (!it_ready) {
+                it_init_all();
+                __atomic_store_n(&it_ready, 1, __ATOMIC_RELEASE);
+            }
+        }
+    }
+    su_id = it_su_id;
+#ifdef _OPENMP
+    if (omp_in_parallel()) {
+#ifdef IT_FLOW
+        if (it_sell && it_pos_ok == 1 && su_id && it_t1s_id && omp_get_num_threads() == it_nt) {
+            const int t = omp_get_thread_num();
+            int use_tr = tr_ok;
+            if (tr_ok) {
+                /* Both flows are exact; which is faster depends on the
+                   machine (the shared copy halves the bytes per iteration
+                   but scatters). The first IT_CAL calls of the team time
+                   both, each call closed by a team barrier, and keep the
+                   faster: call 0 warms tr, 1-2 time it, 3 warms the
+                   two-copy flow, 4-5 time it. */
+                const int k = it_cc[t].v;
+                if (k < IT_CAL) {
+                    it_cc[t].v = k + 1;
+                    if (k < 3) tr_flow_body(t, state, bias_t0, bias_t1, bias_su, l0, l1, t_t0, t_t1, new_state);
+                    else it_flow_body(t, state, bias_t0, bias_t1, bias_su, l0, l1, t_t0, t_t1, new_state);
+                    #pragma omp barrier
+                    if (t == 0) {
+                        const double now = omp_get_wtime();
+                        if (k == 0 || k == 3) it_t_mark = now;
+                        if (k == 2) it_t_tr = now - it_t_mark;
+                        if (k == 5) it_use_tr = (now - it_t_mark) < it_t_tr ? 0 : 1;
+                    }
+                    if (k == IT_CAL - 1) {
+                        #pragma omp barrier
+                    }
+                    return;
+                }
+                use_tr = it_use_tr;
+            }
+            if (use_tr) tr_flow_body(t, state, bias_t0, bias_t1, bias_su, l0, l1, t_t0, t_t1, new_state);
+            else it_flow_body(t, state, bias_t0, bias_t1, bias_su, l0, l1, t_t0, t_t1, new_state);
+            return;
+        }
+#endif
+        if (it_sell) {
+            it_sell_body(omp_get_thread_num(), omp_get_num_threads(), 1, state, bias_t0, bias_t1, bias_su, l0, l1, su_id, t_t0, t_t1, new_state);
+        } else {
+            #pragma omp single
+            it_plain_iterate(state, bias_t0, bias_t1, bias_su, l0, l1, su_id, t_t0, t_t1, new_state);
+        }
+        return;
+    }
+#endif
+    if (it_sell) {
+        it_sell_iterate(state, bias_t0, bias_t1, bias_su, l0, l1, su_id, t_t0, t_t1, new_state);
+        return;
+    }
+    it_plain_iterate(state, bias_t0, bias_t1, bias_su, l0, l1, su_id, t_t0, t_t1, new_state);
+}
+
+static void it_plain_iterate(const fom_float *state, const fom_float *bias_t0, const fom_float *bias_t1, const fom_float *bias_su, fom_float l0, fom_float l1, int su_id, fom_float *t_t0, fom_float *t_t1, fom_float *new_state) {
+    fom_int i;
+    #ifdef _OPENMP
+    #pragma omp parallel private(i)
+    #endif
+    {
+        #ifdef _OPENMP
+        #pragma omp for schedule(static)
+        #endif
+        for (i = 0; i < 10000; ++i) {
+            fom_float v = bias_t0[i] + it_dot8(FOM_MAT_t0_state_COLIDX, FOM_MAT_t0_state_VALS, FOM_MAT_t0_state_ROWPTR[i], FOM_MAT_t0_state_ROWPTR[i + 1], state);
+            v = (v > l0) ? (v - l0) : ((v < -l0) ? (v + l0) : 0.0);
+            t_t0[i] = v;
+        }
+        #ifdef _OPENMP
+        #pragma omp for schedule(static)
+        #endif
+        for (i = 0; i < 40000; ++i) {
+            fom_float v = bias_t1[i], s = 0.0;
+            fom_int p;
+            for (p = FOM_MAT_t1_state_ROWPTR[i]; p < FOM_MAT_t1_state_ROWPTR[i + 1]; ++p) s += FOM_MAT_t1_state_VALS[p] * state[FOM_MAT_t1_state_COLIDX[p]];
+            v += s;
+            v += it_dot8(FOM_MAT_t1_t_t0_COLIDX, FOM_MAT_t1_t_t0_VALS, FOM_MAT_t1_t_t0_ROWPTR[i], FOM_MAT_t1_t_t0_ROWPTR[i + 1], t_t0);
+            v = (v > l1) ? (v - l1) : ((v < -l1) ? (v + l1) : 0.0);
+            t_t1[i] = v;
+            if (su_id) new_state[i] = bias_su[i] + v;
+        }
+        if (!su_id) {
+            #ifdef _OPENMP
+            #pragma omp for schedule(static)
+            #endif
+            for (i = 0; i < 40000; ++i) {
+                fom_float s = 0.0;
+                fom_int p;
+                for (p = FOM_MAT_su_t_t1_ROWPTR[i]; p < FOM_MAT_su_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t1_VALS[p] * t_t1[FOM_MAT_su_t_t1_COLIDX[p]];
+                new_state[i] = bias_su[i] + s;
+            }
+        }
+    }
+}
```

## Agent notes

```
### Round 1

**Whole-solve mean_us:** 57707.6 (initial) → 32280.7 (final), **1.79× speedup** (16 OpenMP threads, EPYC 7413 compute node).

**Final profile** (`make profile`, hottest first; the last three regions are nested inside `iterate` and timed by thread 0):
| region | total_us | calls | pct |
|---|---|---|---|
| iterate (whole `fom_iterate`) | 2023639 | 12400 | 51.2% (≈163 µs/iter) |
| sell_t0_state (t0 = act(bias + t0_state·x)) | 952575 | 12400 | 24.1% (≈77 µs/iter) |
| sell_t1_t_t0 (t1 = act(x + t1_t_t0·t0), new_state) | 885535 | 12400 | 22.4% (≈71 µs/iter) |
| barrier_wait (between the two phases) | 90743 | 12400 | 2.3% (≈7 µs/iter) |
| residual | 339 | 62 | 0.01% |

**Optimizations applied:**
- **One fused parallel region per iteration (`fom_iterate`).** The two body steps and the state update share one OpenMP region with one barrier, and t1 and new_state are written in the same pass. This is valid because `su_t_t1` and `t1_state` are exact identities, which is checked once from their CSR data, with a general path kept if not. In the solver, `fom_iterate` replaces the separate step calls and copies.
- **Several independent accumulators** instead of one serial FP-add chain per row. The reference loops were bound by add latency and the cost of each cache-line fill. About 10%.
- **SELL-C-σ layout with column panels for both products** (t0_state rows of about 260 nonzeros, t1_t_t0 rows of about 65), built once from the matrices' own data inside `fom_iterate`.
  - Each thread owns a contiguous block of rows.
  - Columns are cut into panels 4000 wide, so the gathered source slice (32 KB of x or t0) stays in L1.
  - Within a panel, rows are sorted by segment length and grouped into chunks of 8, stored slot-major and zero-padded. Padding is only about 1.2%.
  - One fixed-shape loop with 8 independent accumulators handles a chunk, so short rows cost no per-row branch overhead.
  - This exploits the random but sorted sparsity of A: L1 gathers are about twice as cheap as L2 gathers. Gain: 52 → 39.7 ms.
- **2-wide vector lanes** (GCC vector extensions, which produce plain SSE2 under the fixed `-O3`; there is a scalar fallback for other compilers). Pairs of lanes use one packed value load and one packed multiply-add. Gain: 39.7 → 35.0 ms.
- **Four 16-bit indices packed per 64-bit word**, built endian-independently. This is possible because panel-local column indices are below 4000. It cuts loads per nonzero from 2.5 to 1.75. Gain: 35.0 → 33.3 ms.
- **Identity `t1_state` folded in** as `+ state[i]`, checked from its data. Gain: about 1%.
- The parameter term `t0_p_p0·p0` is still computed once per `fom_update_params`, outside the solve.

**Blocked by rules:**
- AVX2/FMA (or `-march=native`) for 4-wide gathers and fused multiply-add in the SELL kernel | build flags are fixed (`-O3` without ISA flags), and per-function `target` attributes would bypass that and drop portability to the embedded target | estimated 1.3–1.8× on the two products
- More OpenMP threads than `OMP_NUM_THREADS=16`: the kernels are bound per core and scale linearly | target/run configuration (I left the environment's thread count alone) | about linear in cores
- Carrying iteration state (e.g. partial t0 sums from a column-partitioned pass) across `fom_iterate` calls to save a sync point | the fused/iterate routines must not depend on earlier calls (a static cache that changes results is forbidden) | about 5%
- Building the SELL layout in `fom_setup` instead of lazily on the first `fom_iterate` call | `fom_solver.c` may not read matrix data, and `fom_ops.c` may expose no non-entry-point functions, so a setup hook cannot exist | no steady-state gain; it only moves about 10 ms of one-time build cost out of the first solve

### Round 2

**Whole-solve time (`make bench` mean_us):** 32354 µs at the start of this round, about 25100 µs now (25069, 25155, 25402 in repeated runs). That is 1.29x this round and 2.28x against the translated solver (57230 µs). Correctness: `VERIFY OK`, worst error 3.6e-15 (tolerance 1e-5). It also passes at OMP_NUM_THREADS=1, 3, 6, 7 and in a serial build.

**Final profile (`make profile`, mean 25477 µs).** Region times are nested inside `iterate`, so the % column double-counts. The share of `iterate` is given in brackets.
- iterate (whole loop, one persistent team): 1997457 µs, 49.92%
- sell_t1_t_t0 (t1 = soft(x + B·t0), each panel waits for its t0 rows): 870763 µs, 21.76% [43.6%]
- sell_t0_state (partial A·x, each panel waits for its state rows): 772794 µs, 19.31% [38.7%]
- t0_reduce (wait for 4 partial sums, then add them and soft-threshold): 358662 µs, 8.96% [18.0%]
- barrier_wait (final check that all reduces are done): 1560 µs, 0.04%
- residual: 469 µs, 0.01%

**Optimizations applied**
1. **2D (row-block × column-block) split of A·x, with CB=4 column blocks.** Each thread multiplies a 2500-row × 10000-column block of t0_state, so it reads only a quarter of the state vector. The column blocks line up with the state rows the same threads write in phase 2, so most of those reads stay on the same CCX. A short pass then adds the 4 partial sums and applies the soft-threshold. Measurements showed each thread paid a large fixed cost to pull the whole 320 KB state vector written by other cores. About 10% faster; CB=1, 2 and 8 were all slower.
2. **One parallel region per solve instead of one per iteration.** The fixed iteration count (tol = 0) lets `fom_solve` fork the team once. `fom_iterate` detects it is inside a team and runs as an orphaned step. The first iteration, which builds the layouts, still runs on its own. About 4%.
3. **Point-to-point epoch flags instead of barriers.** Each thread waits only for the threads it reads from: the owners of its state block, its 3 row-block partners, and the reducers of t0. Phase 2 reads t0 from an internal double buffer (t_t0 is still written, never read back), which removes the write-after-read hazard. This needs su_t_t1 and t1_state to be identities, which is checked from their data at setup. About 3.6%.
4. **Panel-by-panel dataflow inside both phases.** Each column panel waits only for the producers of its source rows, starting with the thread's own panel, so a slow producer delays only that part of the work. About 4% for phase 1, plus a little for phase 2.
5. **AVX2+FMA kernel, selected at run time.** The bottleneck turned out to be load ports, not memory bandwidth: 16-bit index loads and AVX2 hardware gathers both made it slower. The kernel keeps 4 indices packed per 64-bit word and unpacks them with BMI1 `bextr`. Values are loaded 256 bits at a time straight into the FMA, and the gathered sources are scalar loads paired with one insert. The SSE2 kernel stays as the portable fallback. About 8% plus 2%.
6. **Retuned SELL shape: 16-row chunks, panels at most 2000 columns (16 KB) wide.** This halves per-chunk overhead and keeps gathered sources in L1. Padding is only 0.2%. About 2% plus 2%.
7. **Prefetch of the remote partial sums** once each partner has signalled. Neutral within noise; kept.

**Tried and dropped (slower or no change on the target):**
- Storing A once and rebuilding t1_t_t0 bit-exactly as fl(c·a) plus an 8-bit correction. The two matrices have the same pattern and B ≈ −0.0147·Aᵀ to within 1 ulp. It needs a scatter for A·x, which cost more than the halved memory traffic saved.
- Work stealing: moving work across CCXs cost more than the balance it gained.
- Splitting phase 2 across column halves.
- NUMA-local copies of the matrix data.
- Assigning each reduce to a partner thread.
- AVX-512 and hardware-gather kernels: they do not run on the target node.
- 32-row chunks; BMI2 `shrx`.

Blocked by rules:
- Raise the thread count or pin threads to idle or same-CCX cores. The run uses 16 threads spread over shared cores on 2 sockets, and other jobs on CPUs 4–7 make some threads up to 15% slower; this is what the remaining reduce wait mostly reflects. | Makefile/environment (OMP_NUM_THREADS, OMP_PLACES, and the srun CPU set) are fixed | unknown, likely 10–20% on a quiet layout
- Build the whole program with `-march=native` instead of per-function target attributes and run-time dispatch. | build flags are fixed | small, about 1–3%

### Round 3

**Whole-solve time (`make bench` mean_us, on the Slurm compute node):**
- Translated solver: 57229.6 us.
- Start of this round: 25668.6 us.
- Final: about 25000 us (runs of 24976 and 25138).
- This round: about 1.025x. Overall: about 2.29x.

**Final profile (`make profile`), hottest first.** The sub-regions are timed on thread 0 inside `iterate`, so they add up to it.
| region | total_us | % |
|---|---|---|
| iterate (whole parallel loop) | 2009211 | 49.91 |
| sell_t1_t_t0 (phase 2: t1 = S(x + t1_t_t0·t0)) | 901893 | 22.40 |
| sell_t0_state (phase 1: partial t0_state·x) | 840842 | 20.89 |
| t0_reduce (waits for the 4 partial producers, adds, thresholds) | 271508 | 6.74 |
| barrier_wait | 1637 | 0.04 |
| residual | 452 | 0.01 |

From `perf` on the compute node: the gather kernel is ~74% of cycles, spin-waits ~10%, and the combine/reduce/output loops make up the rest.

**Optimizations applied this round:**
- **Chunk sums stored contiguously, then combined per row (all chunks, sorted in length order).** Previously each 16-row chunk ended with 16 scalar read-modify-writes into rows scattered by the per-panel length sort. Now each chunk ends with four full-width vector stores into a chunk-ordered buffer. A per-row slot map, built once from the matrix structure, lets each row add its panel sums in one gather. This also removes the zeroing pass over the partial vectors. It is valid because the length sort gives each panel's chunks a fixed, data-independent row permutation. Measured ~2.5% faster on the target.
- **Panel width 2000 → 3334 columns (3 panels instead of 5).** With the combine step above, each panel costs one gather per row, so fewer panels means fewer gathers. A 26.7 KB source panel still fits Zen3's 32 KB L1; 5000 columns (40 KB) was 28% slower. Measured ~1.3% faster.

**Tried and rejected (all verified, all measured on the target):**
- Assigning on the first panel instead of zeroing: about +1% slower on Zen3.
- Processing panels in readiness order: neutral.
- Panel-level work stealing between neighbouring threads, in three variants: 1.5–2.5% slower. One core was consistently ~18% slower in phase 1, and the slowness followed the core, not the data block, so it is environmental. Stealing still did not pay off.
- A 2D grid where phase 2 uses the matching t1_t_t0 block, so each sync involves only 4 threads: 15% slower. The extra combine pass and cache-line invalidations cost more than the reduced waiting saved.
- Thread grid with 8 column blocks (Cb=8): ~6% slower.
- Chunk heights of 8 or 32 rows: slower.
- Prefetching the t0 panels: slower.
- Merging the t0-reduce passes into one: neutral.
- Count-based row segmentation: dropped after a cache simulation showed ~6x more L1 misses on the source vector.
- Microbenchmarks of other gather forms (blend, masked broadcast, AVX-512, hardware gathers) gave no kernel speedup.
- Exploiting `t1_t_t0 = c·t0_stateᵀ` (exact up to rounding): no gain possible. The kernel is instruction-bound, so sharing or scaling one matrix's values saves nothing.

Blocked by rules:
- Overlapping the per-iteration copies of `t_t1` and `t_t0` (writing them only on the last iteration). With su_t_t1 = I, `t_t1` equals new_state, and the solver copies state into it at the end anyway | the `fom_iterate` contract (every temporary written on every call), checked by `make verify` | ~1-2% (removes about 4,000 stores per thread per iteration)
- Storing matrix values (or the gathered vectors) in float32 to halve the stream and gather traffic | target constraints: the float64 spec and exactness against the reference | unknown; the kernel is instruction-bound, so probably small

### Round 4

**Machine note.** The held benchmark node changed this round. It is now an Intel Xeon Gold 6342 (Ice Lake-SP: 36 MB L3, 1.25 MB L2 per core; all 16 threads on one socket), shared with a GROMACS job. The 24,965 µs starting figure came from a different machine, so all numbers below were re-measured side by side on the current node.

**Timing (`make bench` mean_us, same node, interleaved runs)**
- Translated solver: 90,061 µs.
- Round-3 solver (start of this round): ~55,000 µs (54.6k to 57.3k).
- Final: ~39,700 µs (39.6k to 39.9k).
- Speedup this round: about 1.39x. Overall vs. translated on this node: about 2.27x.

**Final profile (`make profile`, 200 iterations per solve; `iterate` wraps everything)**

| region | total_us | pct |
|---|---|---|
| iterate | 2,031,847 | 50.05% |
| tr_fused | 1,690,567 | 41.64% |
| t0_reduce | 183,975 | 4.53% |
| barrier_wait | 103,903 | 2.56% |
| tr_partial | 48,963 | 1.21% |
| residual | 328 | 0.01% |

**Optimizations applied**
- **One shared copy of the matrix.** At setup I checked from the data of both matrices that `t1_t_t0` equals `c * t0_state^T` exactly: same pattern, every value bitwise `c` times its partner.
  - `c` is computed without division: a Newton reciprocal, then picking whichever of `c` and its two neighbouring doubles makes every product match.
  - If the check fails, the old code paths are used.
  - One column-stored copy of `t0_state` (26 MB instead of 52 MB) then serves both products. The round-3 solver was memory-bound here (IPC 1.4, about 100M L3 misses); the new path has almost no L3 misses.
- **Fused pass per column chunk.** Each thread owns 2,500 state rows, which are columns of `t0_state`. For each chunk it:
  - gathers the dot products with `c*t0`;
  - applies the `t1` soft-threshold and the state update;
  - scatters `A[:,J] x_new` into a private partial.
  
  Each thread then reads only its own state rows, so the state exchange between threads is gone. The next call adds up the 16 partials into `t0`. A partial is reused only if the state the next call receives matches a stored copy of what was written; otherwise it is recomputed from that state.
- **Panels and groups.** The `t0` index range is split into 3 panels, and columns are processed in groups of 1,024. Per group, all gathers run panel by panel, then the activations, then all scatters panel by panel. This keeps the source and partial panels in L1 and re-reads the group's data from L2. The first unpanelled version was no faster, at 14x the L1 misses.
- **Zig-zag group order.** The group order reverses every iteration, so about 1 MB of each thread's ~1.7 MB share is still in L2 when the next iteration starts: -8.5%.
- **Double-buffered partials.** The next partial is zeroed while waiting for the other threads, which takes the memset off the critical path: -6%.
- **Small gains (about 0.5 to 1% each):**
  - unroll-by-2 slot loops;
  - `bextr` index decoding;
  - removing the reduce-stage prefetch, which stalled on lines held modified in other cores;
  - a 16-bit chunk lane map;
  - a branch-free state-reuse check.
- **Adaptive path selection.** In the first parallel solve, `fom_iterate` times two warmed iterations of the new path and of the round-3 two-copy path (team barrier after each) and keeps the faster. Large-L3 machines, where the round-3 path may still win, therefore keep it. All iterations now run inside one parallel region, and setup and CPU detection happen inside it.

**Tried and reverted:**
- AVX-512 hardware gathers: 35 ms vs 22 ms. GDS microcode mitigation is active on this node.
- 16-bit index loads in the scatter.
- Reducers zeroing the partials they read.
- Software prefetch of the next group.
- Software-pipelining the gather of group g+1 with the scatter of group g.
- Packed-multiply scatter.
- 128-bit gather.

**Blocked by rules:**
- Using more than the 16 OpenMP threads the environment provides, for example spreading over more cores or L3 | build flags and environment are fixed | unknown, likely large
- float32 copies of matrix values, to make each thread's share fit in L2 | the 1e-10 kernel tolerance in `make verify` | unknown, likely 20-30%

### Round 5

**Whole-solve `mean_us`:** initial 39977.9 → final ~37500 (3 runs: 37570, 37427, 37543). That is 1.066x this round and 1.53x against the translated solver (57229.6).

**Final profile (`make profile`; percent of the `iterate` region, 37.85 ms/solve; per-iteration time in brackets):**
- tr_fused: 34307 us/solve, 90.6% [171.5 us/iter]. Fused per-thread pass: t1 dot products, activation and state update, then the scatter into the partial of t0.
- t0_reduce: 1206 us/solve, 3.2% [6.0 us/iter]. Stage A, the sum within each half of the team.
- tr_partial: 1045 us/solve, 2.8% [5.2 us/iter]. Reuse check plus clearing the partial buffer.
- t0_reduce_partner: 786 us/solve, 2.1% [3.9 us/iter]. Stage B: partner wait, cross-half add, activation.
- barrier_wait: 415 us/solve, 1.1%.
- residual: 6 us/solve, 0.0%.

**Optimizations applied this round:**
- **Two-level reduce matched to the L3 domains.** With OMP_PLACES=cores and close binding, threads 0–7 and 8–15 sit on two CCDs, each with its own 32 MB L3 (checked with lscpu).
  - Stage A: each thread sums only its own half's 8 partials over its 1/8 of the rows.
  - Stage B: it adds the partner half's result for the same rows and waits only on that one thread.
  - Each half keeps its own copy of c·t0, so the fused pass never reads t0 across CCDs.
  - This works because t0 is a sum of per-thread column partials, and an addition can be regrouped exactly.
  - Both halves add in the same order, so their copies of t0 are bitwise identical.
  - Gain: about 1.5%, plus the load imbalance between CCDs went away.
- **Pipelined fused pass.** The scatter `P += A[:,J_g] x_new[J_g]` is store-bound: one random store per entry, about one per cycle. The dot products `c A^T t0` are load-bound. The loop now interleaves them step by step: the scatter of group g runs alongside the dots of group g+1, pairing chunk i with chunk i.
  - This is valid because group g+1's dots don't depend on group g's scatter. Only group g's own new state is needed first.
  - Retuned to 4 row panels.
  - Gain: about 3.5% (measured on/off: 38.1 vs 39.5–39.9 ms).
- **Row-blocked stage A.** Stage A now sums all 8 partials for each block of 128 rows instead of one partial after another. This keeps 8 remote streams in flight at once, and the summation order per row is unchanged.
  - Stage A went from 8.4 to 5.9 us/iter, about 1.3% overall.

**Tried and dropped:** these were all slower or within noise.
- AVX-512 hardware gather for the dots: +55%.
- A row-major second copy of `A[:,J_t]` so the partials become gathers: +45%. The two copies overflow the per-CCD L3.
- Software prefetch (into L2 for the next group, ahead within the dots, and non-temporal in the scatter).
- Group size, panel count and SELL chunk width (8/16/32) sweeps.
- Moving the partial-buffer clear into spin-wait slack, or having the reducers zero the rows they read.
- Scalar-FMA scatter in the pipeline, and swapping the step order.
- tune=znver3.

Blocked by rules:
- Use more than OMP_NUM_THREADS=16 cores (for example `num_threads(32)`), so each thread's ~0.8 MB slice of the matrix stays in its 1 MB L2 and the dots stop streaming from L3 | the target environment's thread count (the build/run setup is fixed) | probably 1.5–2x, untested
- Store the matrix values in float32 to halve the bytes streamed and the L1/L2 footprint | the requirement that every kernel applies exactly M (values are not float32-exact) | ~10–20%, unknown
- Compile the hot kernel with -march=znver4 or a newer GCC for Zen 4 scheduling | build flags/Makefile are fixed (GCC 11.5 has no znver4 target) | unknown, probably small
```
