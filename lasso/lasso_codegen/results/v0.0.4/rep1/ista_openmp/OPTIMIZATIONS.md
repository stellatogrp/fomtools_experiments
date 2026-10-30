# Optimization report

## Timing

```
translated (emitted)   70329.8 us/solve
optimized              27112.4 us/solve
speedup                  2.59x
```

Optimization outcome: **kept (re-verified correct)**.

## Rounds

| round | before (us) | after (us) | gain | kept | agent (s) | outcome |
|---:|---:|---:|---:|:---:|---:|---|
| 1 | 70329.781 | 30546.841 | 56.6% | yes | 3182 | continued |
| 2 | 31429.981 | 29481.119 | 6.2% | yes | 1622 | continued |
| 3 | 29360.748 | 27112.443 | 7.7% | yes | 1077 | continued (agent stopped at its budget) |
| 4 | - | - | - | no | 20 | the round left the sources unchanged (agent stopped at its budget) |

## Caveats

- The agent was terminated at its wall-clock budget before it finished (in round 3, 4), so it may never have written a summary. The timing and diff below are still sound: the solver on disk was re-verified and benchmarked. Raise `CodegenConfig.agent_optimize_timeout_s` to give it longer.

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
| 1 | AVX2/FMA code generation (hardware gathers, 4-wide loads, fused multiply-add) for the gather kernels | fixed build flags (no `-march`); adding per-function `target` attributes would sidestep that rule and is not portable to the embedded target | unknown; the kernels are bound by load ports and dispatch width, so roughly 1.3–1.5x on about 85% of the runtime |
| 1 | Keeping the next iteration's `t0_state @ state` partials across calls (the cross-iteration fusion through `t1_t_t0 == c * t0_state^T`) | the "no state a routine keeps across calls" note, and `fom_iterate`'s fixed signature, which cannot return the next iteration's products | none in practice: I built it with a bitwise state check and the scatter was slower anyway |
| 2 | Compile the gather kernels for AVX2/FMA (vgatherdpd, 4-wide FMA) through a target attribute or `-march=znver3` | fixed Makefile/build flags (baseline x86-64 SSE2) and the embedded/portable target constraint | unknown, plausibly 20-40% since the passes are load-port bound |
| 2 | Pin threads or pick thread placement to balance CCX/L3 load (the affinity mask spreads 16 threads over 4 CCXs as 3/1/6/6) | fixed build/run environment (OMP_PROC_BIND/OMP_PLACES are not editable) | unknown, up to about 5% (per-thread pass times vary 56-65 µs) |

## What the agent changed

```diff
--- src/fom_solver.c (translated)
+++ src/fom_solver.c (optimized)
@@ -2,6 +2,10 @@
 #include "fom_solver.h"
 #include "fom_data.h"
 #include "fom_ops.h"
+#include "fom_profile.h"
+
+/* bias of t1 and su: no constant or parameter terms */
+static const fom_float fom_zero_bias[FOM_STATE_DIM];
 
 void fom_set_default_settings(FomSettings *settings) {
     settings->max_iter = FOM_MAX_ITER;
@@ -28,33 +32,40 @@
 }
 
 fom_int fom_solve(FomWorkspace *work) {
-    const fom_float lam = FOM_T1_LAMBDA;
     fom_float *bias_t0 = work->acc; /* t0_p_p0 @ p0, hoisted */
     fom_float *x = work->state, *xn = work->new_state, *tp;
     fom_float res = 0;
     fom_int k, i, n = work->settings.max_iter;
 
+    FOM_PROF_BEGIN(bias);
     memset(bias_t0, 0, FOM_T0_DIM * sizeof(fom_float));
     fom_apply_t0_p_p0(work->params.p0, bias_t0);
+    FOM_PROF_END(bias);
 
+    FOM_PROF_BEGIN(iterate);
+    /* one parallel region for the whole loop: fom_iterate's loops are
+       orphaned worksharing constructs and synchronize the team */
+#ifdef _OPENMP
+#pragma omp parallel private(k, tp) firstprivate(x, xn)
+#endif
     for (k = 0; k < n; ++k) {
-        fom_step_t0(x, bias_t0, &FOM_ACT_t0, work->t0);
-        memset(xn, 0, FOM_STATE_DIM * sizeof(fom_float));
-        fom_fused_t1(x, work->t0, xn);
-        res = 0;
-#ifdef _OPENMP
-#pragma omp parallel for reduction(max:res)
-#endif
+        fom_iterate(x, bias_t0, fom_zero_bias, fom_zero_bias, &FOM_ACT_t0, &FOM_ACT_t1,
+                    work->t0, work->t1, xn);
+        tp = x; x = xn; xn = tp;
+    }
+    if (n & 1) { tp = x; x = xn; xn = tp; }
+    FOM_PROF_END(iterate);
+
+    /* residual of the last iteration only (no tolerance check) */
+    FOM_PROF_BEGIN(residual);
+    if (n > 0) {
         for (i = 0; i < FOM_STATE_DIM; ++i) {
-            fom_float v = xn[i], d;
-            v = v > lam ? v - lam : (v < -lam ? v + lam : 0.0);
-            xn[i] = v;
-            d = v - x[i];
+            fom_float d = x[i] - xn[i];
             d = d < 0 ? -d : d;
             if (d > res) res = d;
         }
-        tp = x; x = xn; xn = tp;
     }
+    FOM_PROF_END(residual);
     if (x != work->state) memcpy(work->state, x, sizeof(work->state));
     memcpy(work->solution.x, work->state, sizeof(work->solution.x));
     work->solution.iter = n;
--- src/fom_ops.c (translated)
+++ src/fom_ops.c (optimized)
@@ -1,5 +1,10 @@
 #include "fom_ops.h"
 #include "fom_data.h"
+#include <string.h>
+#include "fom_profile.h"
+#ifdef _OPENMP
+#include <omp.h>
+#endif
 
 void fom_apply_t0_state(const fom_float *x, fom_float *out) {
     fom_int i, p;
@@ -130,8 +135,473 @@
     }
 }
 
+/* ---- fom_iterate ----------------------------------------------------
+   One iteration is t0 = soft(bias_t0 + t0_state x) followed by
+   t1 = soft(bias_t1 + t1_state x + t1_t_t0 t0), new_state = bias_su + su t1.
+   t1_state and su_t_t1 are diagonal here (checked on the first call; any
+   other pattern falls back to the step routines), so all the work is the
+   two 2.6M-entry products, done as gathers over re-laid-out copies of
+   t0_state and t1_t_t0 built on the first call (same values, re-ordered):
+   - t1_t_t0: rows in 16 groups (one per thread), columns in blocks of
+     2^IT_WB2 = 2048 so the t0 entries a block reads (16 KB) stay in L1
+     next to the streamed matrix data;
+   - t0_state: 16 column slices of IT_SW columns, the same columns whose
+     state entries the slice's thread wrote as t1 rows; each slice gives a
+     partial t0_state x, and the 16 partials are summed in a fixed order;
+   - each (row, block) piece is a segment; segments of equal length are
+     grouped 4 at a time and interleaved, so one SSE2 lane pair carries two
+     rows (no horizontal sums, loop trip counts come in long equal runs);
+     the 0-3 left over at each length are done one at a time;
+   - column indices are stored as uint16 (40000 and 10000 columns) and read
+     four at a time in the interleaved chunks, whose values start 16-byte
+     aligned (so each value pair is one aligned load folded into mulpd);
+   - both passes are bound by the matrix stream and the load ports, so
+     the layout keeps loads per entry low. */
+typedef fom_float it_v2 __attribute__((vector_size(16), aligned(8)));
+typedef fom_float it_v2a __attribute__((vector_size(16)));  /* chunk values: 16-byte aligned */
+/* each unit starts on an even entry (one unused slot at most) */
+#define IT_PAD 256
+
+typedef struct {
+    fom_float *v;          /* values, unit by unit */
+    unsigned short *ix;    /* their column indices */
+    fom_int *cs;           /* chunk k: 16 (or 4) equal-length segments, interleaved */
+    unsigned short *cl;    /* its length (entries per segment) */
+    unsigned short *cr;    /* its rows within the group, cr[16 k + u] */
+    fom_int *ss;           /* single segment k: entries ss[k] .. ss[k] + sl[k] */
+    unsigned short *sl, *sr;
+    fom_int *gc, *gq, *gs; /* (group g, block b) = unit gb: 16-chunks gc[gb] ..
+                              gq[gb], 4-chunks gq[gb] .. gc[gb + 1], singles
+                              gs[gb] .. gs[gb + 1] */
+    fom_int nb, wb, G, ng, ncols;
+} ItBlk;
+
+#define IT_WB2 11
+#define IT_G2 2500
+#define IT_NB2 ((10000 + (1 << IT_WB2) - 1) >> IT_WB2)
+#define IT_NG2 ((40000 + IT_G2 - 1) / IT_G2)
+#define IT_S2 (40000 * IT_NB2)
+
+static fom_float it_v2s[FOM_MAT_t1_t_t0_NNZ + IT_PAD] __attribute__((aligned(16)));
+static unsigned short it_ix2[FOM_MAT_t1_t_t0_NNZ + IT_PAD];
+static fom_int it_cs2[IT_S2 / 4 + 1], it_ss2[IT_S2 + 1];
+static fom_int it_gc2[IT_NG2 * IT_NB2 + 1], it_gq2[IT_NG2 * IT_NB2 + 1], it_gs2[IT_NG2 * IT_NB2 + 1];
+static unsigned short it_cl2[IT_S2 / 4 + 1], it_cr2[4 * IT_S2 + 16], it_sl2[IT_S2 + 1], it_sr2[IT_S2 + 1];
+static ItBlk it_L2 = {it_v2s, it_ix2, it_cs2, it_cl2, it_cr2, it_ss2, it_sl2, it_sr2, it_gc2, it_gq2, it_gs2, IT_NB2, IT_WB2, IT_G2, IT_NG2, 10000};
+/* t0_state by column slices of IT_SW columns (the t1 groups): slice g is
+   a 10000 x IT_SW matrix, stored as an ItBlk with one block whose "group"
+   is the slice and whose rows are absolute */
+#define IT_SW IT_G2
+#define IT_NS ((40000 + IT_SW - 1) / IT_SW)
+#if IT_NS != IT_NG2
+#error "the t1 groups must be the t0_state slices"
+#endif
+/* t0_state units: IT_TG threads share a column group of IT_TG slices
+   and split its rows, so there are IT_NJ partials to sum, not IT_NS */
+#ifndef IT_TG
+#define IT_TG 4
+#endif
+#define IT_NJ (IT_NS / IT_TG)
+#define IT_RB (10000 / IT_TG)
+#if IT_NJ * IT_TG != IT_NS || IT_RB * IT_TG != 10000
+#error "IT_TG must divide the slices and the rows"
+#endif
+#ifndef IT_NB3
+#define IT_NB3 IT_TG
+#endif
+#define IT_CW3 (IT_TG * IT_SW / IT_NB3)
+#if IT_CW3 * IT_NB3 != IT_TG * IT_SW
+#error "IT_NB3 must divide the group's columns"
+#endif
+#define IT_S3 (10000 * IT_NS * IT_NB3 / IT_TG)
+static fom_float it_v3[FOM_MAT_t0_state_NNZ + IT_PAD] __attribute__((aligned(16)));
+static unsigned short it_ix3[FOM_MAT_t0_state_NNZ + IT_PAD];
+static fom_int it_cs3[IT_S3 / 4 + 1], it_ss3[IT_S3 + 1], it_gc3[IT_NS * IT_NB3 + 1], it_gq3[IT_NS * IT_NB3 + 1], it_gs3[IT_NS * IT_NB3 + 1];
+static unsigned short it_cl3[IT_S3 / 4 + 1], it_cr3[4 * IT_S3 + 16], it_sl3[IT_S3 + 1], it_sr3[IT_S3 + 1];
+static ItBlk it_L3 = {it_v3, it_ix3, it_cs3, it_cl3, it_cr3, it_ss3, it_sl3, it_sr3, it_gc3, it_gq3, it_gs3, IT_NB3, 0, IT_RB, IT_NS, 40000};
+static fom_float it_part[IT_NJ][10000];
+static int it_ready = 0;  /* 0 unchecked, 1 blocked path, 2 fallback */
+
+#define IT_MAXSEG 4096
+static fom_int it_tmp[16][IT_MAXSEG];  /* CSR positions of a chunk's segments */
+static fom_int it_len[10000], it_rows[10000];
+
+typedef struct { fom_int q, kc, ks; } ItPos;
+
+/* it_tmp[u][..] = CSR positions of row i's entries with column in [c0, c1) */
+static void it_segment(const fom_int *rowptr, const fom_int *colidx, fom_int i, fom_int c0, fom_int c1, fom_int u) {
+    fom_int p, t = 0;
+    for (p = rowptr[i]; p < rowptr[i + 1]; ++p)
+        if (colidx[p] >= c0 && colidx[p] < c1) it_tmp[u][t++] = p;
+}
+
+/* rows it_rows[m0 .. m0 + w) all have len entries in [c0, c1): one unit */
+static void it_emit(ItBlk *L, ItPos *P, const fom_int *rowptr, const fom_int *colidx, const fom_float *vals,
+                    fom_int m0, fom_int w, fom_int len, fom_int c0, fom_int c1, fom_int rowbase) {
+    fom_int t, u;
+    for (u = 0; u < w; ++u) it_segment(rowptr, colidx, it_rows[m0 + u], c0, c1, u);
+    if (w > 1) {
+        L->cs[P->kc] = P->q;
+        L->cl[P->kc] = (unsigned short)len;
+        for (u = 0; u < w; ++u) L->cr[16 * P->kc + u] = (unsigned short)(it_rows[m0 + u] - rowbase);
+        P->kc++;
+    } else {
+        L->ss[P->ks] = P->q;
+        L->sl[P->ks] = (unsigned short)len;
+        L->sr[P->ks++] = (unsigned short)(it_rows[m0] - rowbase);
+    }
+    for (t = 0; t < len; ++t)
+        for (u = 0; u < w; ++u) {
+            L->v[P->q] = vals[it_tmp[u][t]];
+            L->ix[P->q++] = (unsigned short)colidx[it_tmp[u][t]];
+        }
+}
+
+/* Lay out rows [r0, r1), columns [c0, c1) as unit gb of L: segments of
+   equal length in chunks of 16, then of 4, then one at a time, longest
+   first. Returns 0 if a segment is too long for the scratch. */
+static int it_build_unit(ItBlk *L, ItPos *P, fom_int gb, const fom_int *rowptr, const fom_int *colidx, const fom_float *vals,
+                         fom_int r0, fom_int r1, fom_int c0, fom_int c1, fom_int rowbase) {
+    fom_int i, p, len, n, m, pass, maxlen = 0;
+    for (i = r0; i < r1; ++i) {
+        n = 0;
+        for (p = rowptr[i]; p < rowptr[i + 1]; ++p) n += (colidx[p] >= c0 && colidx[p] < c1);
+        it_len[i - r0] = n;
+        if (n > maxlen) maxlen = n;
+    }
+    if (maxlen > IT_MAXSEG) return 0;
+    P->q += P->q & 1;
+    L->gc[gb] = P->kc;
+    L->gs[gb] = P->ks;
+    for (pass = 0; pass < 3; ++pass) {
+        if (pass == 1) L->gq[gb] = P->kc;
+        for (len = maxlen; len > 0; --len) {
+            n = 0;
+            for (i = r0; i < r1; ++i) if (it_len[i - r0] == len) it_rows[n++] = i;
+            if (pass == 0)
+                for (m = 0; m + 16 <= n; m += 16) it_emit(L, P, rowptr, colidx, vals, m, 16, len, c0, c1, rowbase);
+            else if (pass == 1)
+                for (m = n - (n & 15); m + 4 <= n; m += 4) it_emit(L, P, rowptr, colidx, vals, m, 4, len, c0, c1, rowbase);
+            else
+                for (m = n - (n & 3); m < n; ++m) it_emit(L, P, rowptr, colidx, vals, m, 1, len, c0, c1, rowbase);
+        }
+    }
+    return 1;
+}
+
+static int it_check_cols(const fom_int *rowptr, const fom_int *colidx, fom_int nrows, fom_int ncols) {
+    fom_int p;
+    for (p = rowptr[0]; p < rowptr[nrows]; ++p) if (colidx[p] < 0 || colidx[p] >= ncols) return 0;
+    return 1;
+}
+
+static void it_init(void) {
+    fom_int j, g, b, ok = 1;
+    ItPos P2 = {0, 0, 0}, P3 = {0, 0, 0};
+    for (j = 0; j < 40000; ++j) {
+        if (FOM_MAT_t1_state_ROWPTR[j + 1] != FOM_MAT_t1_state_ROWPTR[j] + 1
+            || FOM_MAT_t1_state_COLIDX[FOM_MAT_t1_state_ROWPTR[j]] != j) ok = 0;
+        if (FOM_MAT_su_t_t1_ROWPTR[j + 1] != FOM_MAT_su_t_t1_ROWPTR[j] + 1
+            || FOM_MAT_su_t_t1_COLIDX[FOM_MAT_su_t_t1_ROWPTR[j]] != j) ok = 0;
+    }
+    if (ok) ok = it_check_cols(FOM_MAT_t0_state_ROWPTR, FOM_MAT_t0_state_COLIDX, 10000, 40000)
+                 && it_check_cols(FOM_MAT_t1_t_t0_ROWPTR, FOM_MAT_t1_t_t0_COLIDX, 40000, 10000);
+    /* t0_state: one unit per column slice, rows absolute */
+    for (g = 0; ok && g < IT_NS; ++g)
+        for (b = 0; ok && b < IT_NB3; ++b) {
+            const fom_int r0 = (g % IT_TG) * IT_RB, c0 = (g / IT_TG) * IT_TG * IT_SW + b * IT_CW3;
+            ok = it_build_unit(&it_L3, &P3, g * IT_NB3 + b, FOM_MAT_t0_state_ROWPTR, FOM_MAT_t0_state_COLIDX, FOM_MAT_t0_state_VALS, r0, r0 + IT_RB, c0, c0 + IT_CW3, r0);
+        }
+    if (ok) {
+        it_L3.gc[IT_NS * IT_NB3] = P3.kc;
+        it_L3.gs[IT_NS * IT_NB3] = P3.ks;
+    }
+    /* t1_t_t0: row groups x column blocks */
+    for (g = 0; ok && g < IT_NG2; ++g)
+        for (b = 0; ok && b < IT_NB2; ++b) {
+            const fom_int r0 = g * IT_G2, r1 = (r0 + IT_G2 < 40000) ? r0 + IT_G2 : 40000;
+            const fom_int c0 = b << IT_WB2, c1 = (c0 + (1 << IT_WB2) < 10000) ? c0 + (1 << IT_WB2) : 10000;
+            ok = it_build_unit(&it_L2, &P2, g * IT_NB2 + b, FOM_MAT_t1_t_t0_ROWPTR, FOM_MAT_t1_t_t0_COLIDX, FOM_MAT_t1_t_t0_VALS, r0, r1, c0, c1, r0);
+        }
+    if (ok) {
+        it_L2.gc[IT_NG2 * IT_NB2] = P2.kc;
+        it_L2.gs[IT_NG2 * IT_NB2] = P2.ks;
+    }
+    #ifdef _OPENMP
+    #pragma omp atomic write seq_cst
+    #endif
+    it_ready = ok ? 1 : 2;
+}
+
+/* acc[r] (+)= segment r of group g . x, for every segment of group g: with
+   assign, each row must have at most one segment in the group (slices) */
+static inline void it_blocks_dot(const ItBlk *L, fom_int b0, fom_int b1, const fom_float *x, fom_float *acc, const int assign) {
+    const fom_float *v = L->v;
+    const unsigned short *ix = L->ix;
+    fom_int b, k, p, e, u;
+    for (b = b0; b < b1; ++b) {
+        for (k = L->gc[b]; k < L->gq[b]; ++k) {
+            const unsigned short *r = L->cr + 16 * k;
+            it_v2 a[8];
+            for (u = 0; u < 8; ++u) a[u] = (it_v2){0.0, 0.0};
+            p = L->cs[k];
+            e = p + 16 * (fom_int)L->cl[k];
+            for (; p < e; p += 16)
+                for (u = 0; u < 8; u += 2) {
+                    unsigned long long w;
+                    memcpy(&w, ix + p + 2 * u, 8);
+                    {
+                        unsigned int lo = (unsigned int)w, hi = (unsigned int)(w >> 32);
+                        it_v2 x0 = {x[lo & 0xffff], x[lo >> 16]};
+                        it_v2 x1 = {x[hi & 0xffff], x[hi >> 16]};
+                        a[u] += *(const it_v2a *)(v + p + 2 * u) * x0;
+                        a[u + 1] += *(const it_v2a *)(v + p + 2 * u + 2) * x1;
+                    }
+                }
+            if (assign)
+                for (u = 0; u < 8; ++u) { acc[r[2 * u]] = a[u][0]; acc[r[2 * u + 1]] = a[u][1]; }
+            else
+                for (u = 0; u < 8; ++u) { acc[r[2 * u]] += a[u][0]; acc[r[2 * u + 1]] += a[u][1]; }
+        }
+        for (k = L->gq[b]; k < L->gc[b + 1]; ++k) {
+            const unsigned short *r = L->cr + 16 * k;
+            it_v2 a0 = {0.0, 0.0}, a1 = {0.0, 0.0};
+            p = L->cs[k];
+            e = p + 4 * (fom_int)L->cl[k];
+            for (; p < e; p += 4) {
+                unsigned long long w;
+                memcpy(&w, ix + p, 8);
+                {
+                    it_v2 x0 = {x[w & 0xffff], x[(w >> 16) & 0xffff]}, x1 = {x[(w >> 32) & 0xffff], x[w >> 48]};
+                    a0 += *(const it_v2a *)(v + p) * x0;
+                    a1 += *(const it_v2a *)(v + p + 2) * x1;
+                }
+            }
+            if (assign) { acc[r[0]] = a0[0]; acc[r[1]] = a0[1]; acc[r[2]] = a1[0]; acc[r[3]] = a1[1]; }
+            else { acc[r[0]] += a0[0]; acc[r[1]] += a0[1]; acc[r[2]] += a1[0]; acc[r[3]] += a1[1]; }
+        }
+        for (k = L->gs[b]; k < L->gs[b + 1]; ++k) {
+            const fom_int p1 = L->ss[k] + L->sl[k];
+            it_v2 a0 = {0.0, 0.0}, a1 = {0.0, 0.0};
+            fom_float s = 0.0;
+            for (p = L->ss[k]; p + 4 <= p1; p += 4) {
+                it_v2 x0 = {x[ix[p]], x[ix[p + 1]]};
+                it_v2 x1 = {x[ix[p + 2]], x[ix[p + 3]]};
+                a0 += *(const it_v2 *)(v + p) * x0;
+                a1 += *(const it_v2 *)(v + p + 2) * x1;
+            }
+            for (; p < p1; ++p) s += v[p] * x[ix[p]];
+            a0 += a1;
+            if (assign) acc[L->sr[k]] = (a0[0] + a0[1]) + s;
+            else acc[L->sr[k]] += (a0[0] + a0[1]) + s;
+        }
+    }
+}
+
+static inline void it_group_dot(const ItBlk *L, fom_int g, const fom_float *x, fom_float *acc, const int assign) {
+    it_blocks_dot(L, g * L->nb, (g + 1) * L->nb, x, acc, assign);
+}
+
+#ifdef _OPENMP
+#define IT_MASTER (omp_get_thread_num() == 0)
+#else
+#define IT_MASTER 1
+#endif
+
+/* The iteration body. Its loops are orphaned worksharing constructs: run
+   by every thread of an enclosing parallel region (the solver keeps one
+   open across iterations), or serially. */
+static void it_body(const fom_float *state, const fom_float *bias_t0, const fom_float *bias_t1, const fom_float *bias_su, fom_float l0, fom_float l1, fom_float *t_t0, fom_float *t_t1, fom_float *new_state) {
+    fom_int g, i;
+    FOM_PROF_BEGIN(it_t0_gather);
+    /* t0 = soft(bias_t0 + t0_state @ state): per column slice partials
+       (each thread reads the slice of state it wrote), then their sum */
+    #ifdef _OPENMP
+    #pragma omp for schedule(static)
+    #endif
+    for (g = 0; g < IT_NS; ++g) {
+        fom_float acc[IT_RB];
+        for (i = 0; i < IT_RB; ++i) acc[i] = 0.0;
+        it_group_dot(&it_L3, g, state, acc, 0);
+        memcpy(it_part[g / IT_TG] + (g % IT_TG) * IT_RB, acc, sizeof(acc));
+    }
+    if (IT_MASTER) FOM_PROF_END(it_t0_gather);
+    FOM_PROF_BEGIN(it_t0_reduce);
+    #ifdef _OPENMP
+    #pragma omp for schedule(static)
+    #endif
+    for (i = 0; i < 10000; ++i) {
+        fom_float s = 0.0, v;
+        for (g = 0; g < IT_NJ; ++g) s += it_part[g][i];
+        v = bias_t0[i] + s;
+        v = (v > l0) ? (v - l0) : ((v < -l0) ? (v + l0) : 0.0);
+        t_t0[i] = v;
+    }
+    if (IT_MASTER) FOM_PROF_END(it_t0_reduce);
+    FOM_PROF_BEGIN(it_t1_gather);
+    /* t1 = soft(bias_t1 + t1_state @ state + t1_t_t0 @ t0); new_state = bias_su + su @ t1.
+       No barrier after it: group g is slice g (IT_NG2 == IT_NS, both
+       loops static), so the next call's first loop reads only state the
+       same thread wrote here, and the barrier ending that loop orders
+       everything else (t_t0, it_part) */
+    #ifdef _OPENMP
+    #pragma omp for schedule(static) nowait
+    #endif
+    for (g = 0; g < IT_NG2; ++g) {
+        const fom_int g0 = g * IT_G2, n = (g0 + IT_G2 < 40000) ? IT_G2 : 40000 - g0;
+        fom_float acc[IT_G2];
+        for (i = 0; i < n; ++i) acc[i] = 0.0;
+        it_group_dot(&it_L2, g, t_t0, acc, 0);
+        for (i = 0; i < n; ++i) {
+            const fom_int j = g0 + i;
+            fom_float v = bias_t1[j] + FOM_MAT_t1_state_VALS[FOM_MAT_t1_state_ROWPTR[j]] * state[j];
+            v += acc[i];
+            v = (v > l1) ? (v - l1) : ((v < -l1) ? (v + l1) : 0.0);
+            t_t1[j] = v;
+            new_state[j] = bias_su[j] + FOM_MAT_su_t_t1_VALS[FOM_MAT_su_t_t1_ROWPTR[j]] * v;
+        }
+    }
+#if IT_TG > 1 && defined(_OPENMP)
+    /* the next t0 pass reads the state its column group wrote: with one
+       unit per thread only the group's IT_TG threads need to meet */
+    #pragma omp barrier
+#endif
+    if (IT_MASTER) FOM_PROF_END(it_t1_gather);
+}
+
+#ifdef _OPENMP
+/* With one thread per unit (team of IT_NS), the barriers become waits on
+   the few threads each phase depends on. Thread t's calls are numbered
+   e = 1, 2, ... (it_ep, written only by t); after each phase of call e it
+   publishes e in its own flag:
+   - it_fg[t].g: unit t of the t0 pass done (it_part rows of unit t);
+   - it_fg[t].r: rows [IT_R0 t, IT_R0 (t + 1)) of t0 reduced into
+     it_t0b[e & 1] (and t_t0);
+   - it_fg[t].s: t1 rows of slice t done (t_t1, new_state).
+   The t0 pass of call e waits for its group's state (.s >= e - 1), the
+   reduce for the IT_NJ units holding its rows (.g >= e), each t1 column
+   block for the reducers of its t0 rows (.r >= e). No thread can lap
+   another: finishing the t1 pass of call e needs every reducer of call
+   e, so every thread has left the t1 pass of call e - 1; hence it_part
+   is never overwritten while read, and the reduced t0 (read by every t1
+   pass) alternates between two buffers. */
+#define IT_R0 (10000 / IT_NS)
+#if IT_NB3 != IT_TG
+#error "the flagged t0 pass needs one column block per slice"
+#endif
+#if IT_R0 * IT_NS != 10000 || IT_RB % IT_R0 != 0
+#error "the reduce rows must split evenly within the t0 row blocks"
+#endif
+typedef struct { unsigned int g; char p0[60]; unsigned int r; char p1[60]; unsigned int s; char p2[60]; unsigned int ep; char p3[60]; } ItFlag;
+static ItFlag it_fg[IT_NS] __attribute__((aligned(64)));
+static fom_float it_t0b[2][10000];
+
+static inline void it_wait(const unsigned int *f, unsigned int e) {
+    while ((int)(__atomic_load_n(f, __ATOMIC_ACQUIRE) - e) < 0) {}
+}
+
+static void it_body_flags(fom_int t, const fom_float *state, const fom_float *bias_t0, const fom_float *bias_t1, const fom_float *bias_su, fom_float l0, fom_float l1, fom_float *t_t0, fom_float *t_t1, fom_float *new_state) {
+    const unsigned int e = ++it_fg[t].ep;
+    const fom_int j = t / IT_TG, k = t % IT_TG;
+    fom_int g, i, b;
+    fom_float *t0b = it_t0b[e & 1];
+    FOM_PROF_BEGIN(it_t0_gather);
+    {
+        fom_float acc[IT_RB];
+        for (i = 0; i < IT_RB; ++i) acc[i] = 0.0;
+        /* column block b of the unit is slice j IT_TG + b: start with the
+           thread's own, wait for each other slice's writer just before it */
+        for (g = 0; g < IT_TG; ++g) {
+            b = (k + g) % IT_TG;
+            if (g) it_wait(&it_fg[j * IT_TG + b].s, e - 1);
+            it_blocks_dot(&it_L3, t * IT_NB3 + b, t * IT_NB3 + b + 1, state, acc, 0);
+        }
+        memcpy(it_part[j] + k * IT_RB, acc, sizeof(acc));
+        __atomic_store_n(&it_fg[t].g, e, __ATOMIC_RELEASE);
+    }
+    if (t == 0) FOM_PROF_END(it_t0_gather);
+    FOM_PROF_BEGIN(it_t0_reduce);
+    {
+        const fom_int i0 = t * IT_R0, u = i0 / IT_RB;  /* the row block of the units to sum */
+        for (g = 0; g < IT_NJ; ++g) it_wait(&it_fg[g * IT_TG + u].g, e);
+        for (i = i0; i < i0 + IT_R0; ++i) {
+            fom_float s = 0.0, v;
+            for (g = 0; g < IT_NJ; ++g) s += it_part[g][i];
+            v = bias_t0[i] + s;
+            v = (v > l0) ? (v - l0) : ((v < -l0) ? (v + l0) : 0.0);
+            t0b[i] = v;
+            t_t0[i] = v;
+        }
+        __atomic_store_n(&it_fg[t].r, e, __ATOMIC_RELEASE);
+    }
+    if (t == 0) FOM_PROF_END(it_t0_reduce);
+    FOM_PROF_BEGIN(it_t1_gather);
+    {
+        const fom_int g0 = t * IT_G2;
+        fom_float acc[IT_G2];
+        for (i = 0; i < IT_G2; ++i) acc[i] = 0.0;
+        for (b = 0; b < IT_NB2; ++b) {
+            const fom_int c0 = b << IT_WB2, c1 = (c0 + (1 << IT_WB2) < 10000) ? c0 + (1 << IT_WB2) : 10000;
+            fom_int r;
+            for (r = c0 / IT_R0; r <= (c1 - 1) / IT_R0; ++r) it_wait(&it_fg[r].r, e);
+            it_blocks_dot(&it_L2, t * IT_NB2 + b, t * IT_NB2 + b + 1, t0b, acc, 0);
+        }
+        for (i = 0; i < IT_G2; ++i) {
+            const fom_int jj = g0 + i;
+            fom_float v = bias_t1[jj] + FOM_MAT_t1_state_VALS[FOM_MAT_t1_state_ROWPTR[jj]] * state[jj];
+            v += acc[i];
+            v = (v > l1) ? (v - l1) : ((v < -l1) ? (v + l1) : 0.0);
+            t_t1[jj] = v;
+            new_state[jj] = bias_su[jj] + FOM_MAT_su_t_t1_VALS[FOM_MAT_su_t_t1_ROWPTR[jj]] * v;
+        }
+        __atomic_store_n(&it_fg[t].s, e, __ATOMIC_RELEASE);
+    }
+    if (t == 0) FOM_PROF_END(it_t1_gather);
+}
+#endif
+
 void fom_iterate(const fom_float *state, const fom_float *bias_t0, const fom_float *bias_t1, const fom_float *bias_su, const FomAct_t0 *act_t0, const FomAct_t1 *act_t1, fom_float *t_t0, fom_float *t_t1, fom_float *new_state) {
-    fom_step_t0(state, bias_t0, act_t0, t_t0);
-    fom_step_t1(state, t_t0, bias_t1, act_t1, t_t1);
-    fom_step_su(t_t1, bias_su, new_state);
-}
+    const fom_float l0 = act_t0->lambd, l1 = act_t1->lambd;
+    int r;
+    #ifdef _OPENMP
+    #pragma omp atomic read seq_cst
+    #endif
+    r = it_ready;
+    if (!r) {
+        #ifdef _OPENMP
+        #pragma omp critical(fom_it_init)
+        #endif
+        {
+            if (!it_ready) it_init();
+        }
+        r = it_ready;
+    }
+    if (r != 1) {
+        #ifdef _OPENMP
+        #pragma omp single
+        #endif
+        {
+            fom_step_t0(state, bias_t0, act_t0, t_t0);
+            fom_step_t1(state, t_t0, bias_t1, act_t1, t_t1);
+            fom_step_su(t_t1, bias_su, new_state);
+        }
+        return;
+    }
+    #ifdef _OPENMP
+    if (!omp_in_parallel()) {
+        #pragma omp parallel
+        {
+            if (omp_get_num_threads() == IT_NS)
+                it_body_flags(omp_get_thread_num(), state, bias_t0, bias_t1, bias_su, l0, l1, t_t0, t_t1, new_state);
+            else
+                it_body(state, bias_t0, bias_t1, bias_su, l0, l1, t_t0, t_t1, new_state);
+        }
+        return;
+    }
+    if (omp_get_num_threads() == IT_NS) {
+        it_body_flags(omp_get_thread_num(), state, bias_t0, bias_t1, bias_su, l0, l1, t_t0, t_t1, new_state);
+        return;
+    }
+    #endif
+    it_body(state, bias_t0, bias_t1, bias_su, l0, l1, t_t0, t_t1, new_state);
+}
```

## Agent notes

```
### Round 1

**Whole-solve time:** 70,341 µs initially, 31,040 µs final (`make bench` mean). That is a **2.27x speedup**. Every kept step passed `make verify`, in both the OpenMP and serial builds.

**Final profile** (`make profile`, 30,891 µs per solve; `iterate` encloses the three `it_*` regions):

| Region | total_us | % of profile | per solve |
|---|---|---|---|
| iterate (the 200-iteration loop) | 2,003,664 | 50.0 | 30.8 ms |
| it_t1_gather (`t1_t_t0` @ t0 + t1/new_state) | 910,692 | 22.7 | 14.0 ms (70 µs/iter) |
| it_t0_gather (`t0_state` @ state, per column slice) | 863,660 | 21.6 | 13.3 ms (66 µs/iter) |
| it_t0_reduce (sum of 16 partials + soft-threshold) | 225,549 | 5.6 | 3.5 ms (17 µs/iter, mostly waiting on the slowest thread) |
| residual | 1,807 | 0.05 | 28 µs |
| bias (`t0_p_p0` @ p0, once per solve) | 1,192 | 0.03 | 18 µs |

**Optimizations applied**, each with the structure that made it valid:
1. **The whole iteration is one `fom_iterate` call.** `t1_state` and `su_t_t1` are identity diagonals (checked once on the first call; otherwise it falls back to the step routines). So `t1`, `new_state` and the soft-threshold are written in the same loop as the `t1_t_t0` product. The zeroing, copy and separate activation passes are gone.
2. **Gather-friendly copies of `t0_state` and `t1_t_t0`.** They are built on the first `fom_iterate` call from the CSR data: same values, re-ordered, and nothing is cached across calls.
   - Column indices are stored as uint16, since the matrices have 40,000 and 10,000 columns.
   - Columns are cut into blocks or slices so the source vector being read stays in L1.
   - Within each block, row segments are sorted by length and interleaved 16 at a time (leftovers in 4-row chunks, then one by one). One SSE2 lane pair then carries two rows: no horizontal sums, and loop trip counts come in long, predictable runs.
   - Indices are read two at a time with one 32-bit load. The loop was load-port bound, so this alone gained about 5%.
3. **`t0_state` is processed in 16 column slices.** Each slice matches the 2,500 state entries its thread wrote in the previous iteration's `t1` pass, so there is no cross-core traffic on the state vector. Every row has at most one segment per slice, so partials are stored directly, with no zeroing pass. Zeroing had cost about 30 µs per iteration because other cores held those cache lines. The 16 partials are then summed in a fixed order.
4. **One OpenMP parallel region per solve.** `fom_iterate`'s loops are orphaned worksharing constructs, leaving three barriers per iteration and no fork/join. Static schedules keep each thread on the same data every iteration.
5. **Residual computed only once.** `tol = 0` and the residual is only reported, so it is computed after the last iteration rather than in every iteration.

**Blocked by rules:**
- AVX2/FMA code generation (hardware gathers, 4-wide loads, fused multiply-add) for the gather kernels | fixed build flags (no `-march`); adding per-function `target` attributes would sidestep that rule and is not portable to the embedded target | unknown; the kernels are bound by load ports and dispatch width, so roughly 1.3–1.5x on about 85% of the runtime
- Keeping the next iteration's `t0_state @ state` partials across calls (the cross-iteration fusion through `t1_t_t0 == c * t0_state^T`) | the "no state a routine keeps across calls" note, and `fom_iterate`'s fixed signature, which cannot return the next iteration's products | none in practice: I built it with a bitwise state check and the scatter was slower anyway

### Round 2

**Whole-solve mean_us:** 30546.8 at the start of this round → 29185.4 final (`make bench`; 28950 under `make profile`). That is 1.05x this round. Against the translated solver (70329.8 us) it is 2.41x overall.

**Final profile (`make profile`, 70 solves × 200 iterations), hottest first:**
| region | total_us | % |
|---|---|---|
| iterate (encloses the three `it_*` regions) | 2021926 | 50.01 |
| it_t0_gather (t0_state @ state, per-slice partials) | 912569 | 22.57 |
| it_t1_gather (t1_t_t0 @ t0, soft-threshold, state write) | 894303 | 22.12 |
| it_t0_reduce (sum of 16 partials, soft-threshold) | 210956 | 5.22 |
| residual | 1948 | 0.05 |
| bias | 1227 | 0.03 |

That is roughly 65 + 64 + 15 µs per iteration.

**Optimizations applied this round** (all in `fom_iterate` in `src/fom_ops.c`):
- **Narrower t1_t_t0 column blocks (2^12 → 2^11 = 2048 columns).** The t0 values a block gathers are now 16 KB, so they stay in L1 next to the streamed matrix data. Valid because t1_t_t0 is a general sparse 40000×10000 matrix and the block width only changes the layout. Gain about 2.5%; a sweep over 2^10 to 2^14 found 2^11 best.
- **No barrier after the t1 loop.** Group g of t1_t_t0 rows is the same column slice g of t0_state, and both loops use a static schedule over 16 iterations. So the next call's first loop reads only state the same thread just wrote, and the barrier ending that loop orders t_t0 and the partials. A compile-time `#error` guards that the two partitions match. Gain about 1%.
- **64-bit index reads.** The uint16 column indices are read four at a time instead of two in the interleaved chunks, which cuts index loads. Direct 16-bit loads were 7% slower, which showed the kernel is load-port sensitive. Gain about 1%.
- **16-byte aligned chunk values.** Each layout unit starts on an even entry (at most one unused slot, `IT_PAD` slack), so the value pairs are aligned loads folded into `mulpd`. Gain about 2%.

**Tried and rejected** (each verified, then measured slower or no better):
- **Sharing one copy of A.** t1_t_t0 equals fl(c·t0_state^T) bit for bit. c can be recovered without division (frexp plus Newton reciprocal, checked on every entry), so one 26 MB copy of A can serve both products instead of 52 MB.
  - As a fused single column pass (gather t0, then scatter the next A·x): 56.6 ms. The random 80 KB gathers and scatters run out of L2.
  - As two passes with A^T·t0 scattered into an L1 slice accumulator: 50.4 ms.
  - With lane-split accumulators: 68.8 ms.
  - A streaming microbenchmark showed each pass is bound by the per-core stream rate (about 30 GB/s), not by L3 capacity, so halving the footprint alone gave nothing.
- **Splitting the t0_state slices into column sub-blocks:** slower (shorter segments).
- **Reduce in sequential-stream order:** 5% slower, because it loses the parallelism of 16 concurrent remote reads.
- **Software prefetch in the reduce:** within noise.
- **Exploiting sparsity:** none to use. The state stays about 40000/40000 nonzero (λ is tiny), and the values use the full 53-bit mantissa, so they can't be compressed.

Blocked by rules:
- Compile the gather kernels for AVX2/FMA (vgatherdpd, 4-wide FMA) through a target attribute or `-march=znver3` | fixed Makefile/build flags (baseline x86-64 SSE2) and the embedded/portable target constraint | unknown, plausibly 20-40% since the passes are load-port bound
- Pin threads or pick thread placement to balance CCX/L3 load (the affinity mask spreads 16 threads over 4 CCXs as 3/1/6/6) | fixed build/run environment (OMP_PROC_BIND/OMP_PLACES are not editable) | unknown, up to about 5% (per-thread pass times vary 56-65 µs)
```
