# Optimization report

## Timing

```
translated (emitted)      85.8 us/solve
optimized                 24.7 us/solve
speedup                  3.47x
```

Optimization outcome: **kept (re-verified correct)**.

## Rounds

| round | before (us) | after (us) | gain | kept | agent (s) | outcome |
|---:|---:|---:|---:|:---:|---:|---|
| 1 | 85.799 | 30.388 | 64.6% | yes | 100 | continued |
| 2 | 31.696 | 28.619 | 9.7% | yes | 97 | continued |
| 3 | 28.458 | 24.733 | 13.1% | yes | 104 | continued |
| 4 | - | - | - | no | 71 | the round left the sources unchanged |

## Kernel checks

Each `fom_apply_<name>` in the shipped `fom_ops.c`, checked by `make verify` against the package's reference over the same `fom_data.c` matrix. *basis* probes every unit vector, which recovers the routine's matrix column by column (an exact check), plus random vectors; *random* uses random vectors only (the matrix is over the probe budget). A `fused_<block>` row is a `fom_fused_<block>`, checked against the sum of its members: *basis* there probes every unit vector of each source with every schedule value at 0 and with each alone at 1, which recovers every member exactly.

| matrix | shape | nnz | rewritten | check | probes | max error |
|---|---|---|---|---|---|---|
| `t0_state` | 50x50 | 1730 | yes | basis | 52 | 8.882e-16 |
| `t0_p_p0` | 50x25 | 280 | yes | basis | 27 | 0.000e+00 |
| `su_t_t0` | 50x50 | 50 | yes | basis | 52 | 0.000e+00 |
| `step_t0` | 50x50 | 1730 | yes | basis | 56 | 1.776e-15 |
| `step_su` | 50x50 | 50 | yes | basis | 52 | 0.000e+00 |
| `iterate` | 50x50 | 1780 | yes | basis | 106 | 1.332e-15 |

### Rule violations

_None: the collected rewrite passed the rules and was kept._

## Blocked by rules

Optimizations the agent considered but did not make because a rule forbade them, in its own words.

| round | idea | rule | expected gain |
|---|---|---|---|
| 1 | Store a transposed or column-major copy of `t0_state`, computed at setup, for a register-resident outer-product formulation (25 vector accumulators, broadcast `x[j]`) | `fom_data.c` is fixed, and `fom_setup` may not probe matrices via `fom_apply_*` (inlining 2500 literals in `fom_step_t0` is allowed but impractical within the time budget) | unknown, likely <10% |
| 1 | Use `-ffast-math`, `-mcpu=native` or other build flags to let the compiler auto-vectorize the reduction | Makefile and build flags are fixed | small; hand-written NEON already covers it |
| 2 | Use the original row-major `FOM_MAT_t0_state` directly in the column kernel, relying on `I - t A^T A` being symmetric, to skip the transposed copy | the data is only symmetric to about 4e-18 (not bitwise), and verify compares exact entries | small (only removes the one-time transpose) |
| 2 | Apply `t0_state` as `I - (t A^T)(A)` with a rank-25 factor taken from `t0_p_p0` (`t A^T`), about half the FLOPs | kernel rule: `fom_step_t0` may read only its own matrix's data, not `t0_p_p0`, and recovering `t` or a factorization needs division | about 1.5 to 2x on the iteration |
| 3 | Apply M as x - (tA^T)(A x) using the rank-25 factor (`t0_p_p0` holds tA^T, so this is about 2x fewer flops) | `fom_apply_t0_*` / step helpers may read only their own matrix's data; rebuilding A would also need a division by t or a factorization at setup | ~1.5-2x on the iterations |
| 3 | Use symmetry to load each off-diagonal entry once | not blocked by a rule; skipped for lack of time (the triangular scatter is hard to vectorize) | unknown |

## What the agent changed

```diff
--- src/fom_solver.c (translated)
+++ src/fom_solver.c (optimized)
@@ -1,6 +1,7 @@
 #include "fom_solver.h"
 #include "fom_data.h"
 #include "fom_ops.h"
+#include "fom_profile.h"
 
 /* su_t_t0 is the identity (SPEC), so the state update is new_state = t0 and
    fom_step_t0 writes straight into the next state buffer. The parameter term
@@ -49,10 +50,13 @@
     fom_int k, i;
     fom_float res = 0.0;
 
+    FOM_PROF_BEGIN(iterations);
     for (k = 0; k < max_iter; ++k) {
         fom_step_t0(a, bias, &FOM_ACT_t0, b);
         t = a; a = b; b = t;
     }
+    FOM_PROF_END(iterations);
+    FOM_PROF_BEGIN(residual_output);
     /* a holds the final iterate, b the previous one. */
     if (max_iter > 0) {
         for (i = 0; i < FOM_STATE_DIM; ++i) {
@@ -65,6 +69,7 @@
         work->solution.x[i] = a[i];
         work->state[i] = FOM_STATE_INIT[i];
     }
+    FOM_PROF_END(residual_output);
     work->solution.iter = max_iter;
     work->solution.residual = res;
     work->solution.status = FOM_MAX_ITER_REACHED;
--- src/fom_ops.c (translated)
+++ src/fom_ops.c (optimized)
@@ -1,13 +1,69 @@
 #include "fom_ops.h"
 #include "fom_data.h"
 
+#if defined(__ARM_NEON)
+#include <arm_neon.h>
+#endif
+
+/* Dense 50x50 row dot products with independent partial sums (2-wide NEON
+   lanes, 4 chains) so the reduction pipelines and vectorizes without
+   -ffast-math. */
+static inline fom_float fom_dot50_t0_state(const fom_float *m, const fom_float *x) {
+#if defined(__ARM_NEON)
+    float64x2_t a0 = vdupq_n_f64(0.0), a1 = a0, a2 = a0, a3 = a0;
+    fom_int j;
+    for (j = 0; j < 48; j += 8) {
+        a0 = vfmaq_f64(a0, vld1q_f64(m + j), vld1q_f64(x + j));
+        a1 = vfmaq_f64(a1, vld1q_f64(m + j + 2), vld1q_f64(x + j + 2));
+        a2 = vfmaq_f64(a2, vld1q_f64(m + j + 4), vld1q_f64(x + j + 4));
+        a3 = vfmaq_f64(a3, vld1q_f64(m + j + 6), vld1q_f64(x + j + 6));
+    }
+    a0 = vfmaq_f64(a0, vld1q_f64(m + 48), vld1q_f64(x + 48));
+    a0 = vaddq_f64(vaddq_f64(a0, a1), vaddq_f64(a2, a3));
+    return vaddvq_f64(a0);
+#else
+    fom_float s0 = 0.0, s1 = 0.0, s2 = 0.0, s3 = 0.0;
+    fom_int j;
+    for (j = 0; j < 48; j += 4) {
+        s0 += m[j] * x[j]; s1 += m[j + 1] * x[j + 1];
+        s2 += m[j + 2] * x[j + 2]; s3 += m[j + 3] * x[j + 3];
+    }
+    s0 += m[48] * x[48]; s1 += m[49] * x[49];
+    return (s0 + s1) + (s2 + s3);
+#endif
+}
+
+#define FOM_T0_RB 10
+#if defined(__ARM_NEON)
+/* FOM_T0_RB rows at once, two chains per row: each x pair is loaded once for
+   all of them (2*FOM_T0_RB independent FMA chains). */
+static inline void fom_dot50xR_t0_state(const fom_float *m, const fom_float *x, fom_float *r) {
+    float64x2_t a[FOM_T0_RB][2];
+    fom_int j, k;
+    for (k = 0; k < FOM_T0_RB; ++k) { a[k][0] = vdupq_n_f64(0.0); a[k][1] = a[k][0]; }
+    for (j = 0; j < 48; j += 4) {
+        float64x2_t x0 = vld1q_f64(x + j), x1 = vld1q_f64(x + j + 2);
+        for (k = 0; k < FOM_T0_RB; ++k) {
+            a[k][0] = vfmaq_f64(a[k][0], vld1q_f64(m + k * 50 + j), x0);
+            a[k][1] = vfmaq_f64(a[k][1], vld1q_f64(m + k * 50 + j + 2), x1);
+        }
+    }
+    {
+        float64x2_t x0 = vld1q_f64(x + 48);
+        for (k = 0; k < FOM_T0_RB; ++k) a[k][0] = vfmaq_f64(a[k][0], vld1q_f64(m + k * 50 + 48), x0);
+    }
+    for (k = 0; k < FOM_T0_RB; ++k) r[k] = vaddvq_f64(vaddq_f64(a[k][0], a[k][1]));
+}
+#else
+static inline void fom_dot50xR_t0_state(const fom_float *m, const fom_float *x, fom_float *r) {
+    fom_int k;
+    for (k = 0; k < FOM_T0_RB; ++k) r[k] = fom_dot50_t0_state(m + k * 50, x);
+}
+#endif
+
 void fom_apply_t0_state(const fom_float *x, fom_float *out) {
-    fom_int i, j;
-    for (i = 0; i < 50; ++i) {
-        fom_float s = 0.0;
-        for (j = 0; j < 50; ++j) s += FOM_MAT_t0_state[i * 50 + j] * x[j];
-        out[i] += s;
-    }
+    fom_int i;
+    for (i = 0; i < 50; ++i) out[i] += fom_dot50_t0_state(FOM_MAT_t0_state + i * 50, x);
 }
 
 void fom_apply_t0_p_p0(const fom_float *x, fom_float *out) {
@@ -28,18 +84,1189 @@
     }
 }
 
+#if defined(__ARM_NEON)
+/* Column-oriented form: out = sum_j M[:, j] * x[j] over a transposed copy of
+   M (built once from FOM_MAT_t0_state), 25 two-wide accumulators covering all
+   50 rows, no horizontal reductions. */
+/* Unrolled over the fixed sparsity of t0_state in 2-row blocks: all-zero
+   2x1 blocks (col j, rows 2r..2r+1) are skipped; coefficients are the
+   matrix's exact values packed column-block-major. */
+static void fom_step_t0_impl(const fom_float *state, const fom_float *bias, const FomAct_t0 *act, fom_float *out) {
+    static const fom_float tab[2184] = {0x1.8eb394c6fe292p-1, -0x1.0e411c862411fp-7, -0x1.289168ad09c3dp-4, 0x1.811d395ea1e63p-5, 0x1.18f7f9e8bd5adp-4, 0x0.0p+0, 0x1.c89ea5fcc43aep-8, 0x1.1ff83a89f999dp-5, 0x1.84c1bb8826a44p-5, 0x1.35bfcf7d902a5p-4, 0x1.90c525a7a6e13p-7, 0x1.74a95130ce4d1p-6, 0x0.0p+0, 0x1.44193fe32be51p-7, -0x1.eac2d815d0cb7p-6, 0x0.0p+0, 0x1.0a3df0e42b2b3p-6, -0x1.f01e378787b5fp-5, -0x1.d8adee6bc4705p-7, 0x1.1de3cc3f1521fp-4, 0x0.0p+0, -0x1.3e10313561b1ep-5, -0x1.25870675a5451p-6, 0x1.5a9e99a7f2205p-6, -0x1.20eeeacdb4bc5p-4, -0x1.d5587013316cep-5, 0x1.0cf3ddd46d22bp-4, -0x1.d25e64724cba4p-5, 0x1.cc61b9991e0d2p-7, 0x1.1abffe6167448p-5, -0x1.9eb0aef39d402p-5, -0x1.23daba74aa26ap-6, 0x1.1a7de318d6770p-6, 0x1.76ddb9db46695p-8, 0x1.6b0bd3c1bd257p-4, -0x1.4b590b2a8bb81p-4, -0x1.ba7bf4fe0de1dp-8, 0x1.0231cfbfd23dbp-5, -0x1.a9acc03c069e6p-9, 0x1.471e563a59a4ep-7, 0x0.0p+0, -0x1.547d325a69de8p-5, 0x1.0bb98b9fc7dcbp-5, -0x1.0fed11cbfc278p-5, 0x1.0eab0281ea4e0p-5, 0x1.d3713ffb9fa3ap-7, -0x1.5ce1eb8b6c1b9p-6, 0x1.ac2e2839fbae9p-5, 0x0.0p+0, -0x1.a499e1c370504p-8, -0x1.0e411c8624121p-7, 0x1.8eb394c6fe292p-1, -0x1.dd0466e6c7ca8p-5, 0x1.6824a65ebb434p-6, 0x1.1a185f293f6cap-4, 0x1.32e76420eddcbp-13, 0x0.0p+0, -0x1.edf304695c968p-8, 0x0.0p+0, 0x1.6cc1dfbbd0a4fp-10, 0x1.c81b7b766b1e1p-5, 0x0.0p+0, 0x1.9dd55e529bad7p-6, 0x1.9b0bc27107444p-7, 0x0.0p+0, 0x1.1501cf0caae71p-3, -0x1.fbbca5e08ec3cp-8, 0x0.0p+0, 0x1.a107eb52ba71cp-5, -0x1.087e642634088p-6, 0x1.fe9bdc41f3ec5p-4, 0x1.08e3d0ca6a4ccp-6, 0x1.e15f5149a98aap-7, 0x0.0p+0, 0x1.50ee22f0b2d97p-8, -0x1.00273e2bb40eap-5, 0x1.70f179eeb3690p-8, 0x1.9634838f705e9p-5, -0x1.9f57fd7e6ddc9p-5, 0x0.0p+0, 0x1.c05371ea0ee8dp-5, 0x0.0p+0, 0x1.57f270e3a28b0p-4, 0x1.0857f6deb51e3p-6, -0x1.15248834e5ef6p-6, 0x1.33555fcb62f80p-5, 0x0.0p+0, 0x1.f9d3db13e1031p-4, 0x1.fa60e255df896p-8, 0x1.c15e1874d1a70p-8, 0x0.0p+0, -0x1.2161ad501e2e2p-7, -0x1.289168ad09c3dp-4, 0x0.0p+0, 0x1.8eb394c6fe292p-1, -0x1.faa2d47c86f25p-6, 0x0.0p+0, 0x1.2d8b885e813d8p-4, 0x0.0p+0, -0x1.60c506a12f1dap-4, 0x1.143b3b4ed9bb4p-4, 0x0.0p+0, 0x0.0p+0, 0x1.0d021bcf52a03p-6, 0x0.0p+0, -0x1.d57e34b7aac67p-8, 0x0.0p+0, 0x1.2733adc8e9402p-4, 0x0.0p+0, 0x1.68c94f3e4c8a7p-5, 0x1.d87971d1cff04p-5, 0x0.0p+0, 0x1.728dd578de6cbp-4, 0x0.0p+0, 0x1.811d395ea1e63p-5, 0x0.0p+0, -0x1.faa2d47c86f25p-6, 0x1.8eb394c6fe292p-1, 0x0.0p+0, 0x1.465cfa7d39872p-6, 0x1.015dc8fb14db3p-8, -0x1.7965febf6b30ep-6, 0x1.5033ee6347331p-7, 0x0.0p+0, -0x1.30820051fc200p-4, -0x1.5630cbf80ce85p-4, 0x1.cb44a88fa10e7p-6, -0x1.336f765d65c15p-5, 0x0.0p+0, 0x1.2acb5445460bcp-8, -0x1.b5776ab6d2bafp-8, -0x1.6f33a3c516c3ep-5, -0x1.19721d61afd4cp-6, 0x0.0p+0, -0x1.ca8e8b50cc7a6p-4, 0x1.2d084d2ebdc57p-9, 0x1.775ed40f71493p-6, 0x1.a739448b89886p-5, 0x1.c76b0416ee499p-5, -0x1.ea42582a16d0dp-6, -0x1.1e9a6be8caae3p-7, -0x1.5b13e9bbe9dfep-7, 0x0.0p+0, 0x1.bd2bb25ed15a5p-7, -0x1.1665a9c6916c6p-7, 0x1.5fe5a2a4d0035p-5, 0x1.709380d729d08p-6, -0x1.089ed05dbe6f6p-3, 0x1.4d324bf95d8fcp-5, -0x1.4c8fc2ed31efcp-6, 0x1.bc8a845e1c2f1p-7, 0x1.099ff46971491p-4, -0x1.2c9447d1898fbp-6, 0x1.a17dab5200623p-9, 0x1.341fcbc77f7e0p-8, 0x1.c45ebc14a6c76p-7, 0x0.0p+0, -0x1.997a5ff215bb8p-6, 0x1.18f7f9e8bd5adp-4, -0x1.dd0466e6c7ca8p-5, 0x1.8eb394c6fe292p-1, -0x1.d178cb5c52897p-4, 0x1.937511a7905c6p-13, -0x1.186ec151fe7ccp-6, -0x1.13b57432e10bdp-9, -0x1.01d0c2cbf339bp-4, 0x1.6b8e7c6fe423cp-11, 0x0.0p+0, -0x1.a1ca19d8e30d8p-7, 0x1.886e546a18faep-4, 0x1.4ffd8b3447163p-7, 0x1.92368086060ccp-5, 0x0.0p+0, 0x1.04f6dfc4d6a2bp-3, 0x1.66f21f66f451dp-5, -0x1.2471c00a1fa04p-6, 0x1.1e8956200a9f8p-9, 0x0.0p+0, 0x1.23e52f52cc057p-4, -0x1.234a56655cfbfp-4, 0x1.1eeae8d045152p-5, 0x0.0p+0, -0x1.3d618f9f5dd12p-11, 0x0.0p+0, 0x1.175dc02d9fd87p-6, -0x1.2568a6f4eb7edp-4, 0x0.0p+0, 0x1.5822fa2b0846cp-5, 0x1.a02e342518979p-6, -0x1.03338bd38253dp-3, -0x1.652a999bbd095p-8, 0x1.6834c490dd368p-4, -0x1.3a6a0c127b1fbp-7, 0x1.0f78904c09f1ep-6, 0x1.2a3547bc1172dp-5, -0x1.09b3f56128de5p-6, 0x0.0p+0, 0x1.a54314a7dd54bp-8, 0x0.0p+0, -0x1.14efce2680e28p-4, 0x1.447d8c8d0abb3p-5, 0x1.c4fcb9f1a1843p-7, -0x1.b62122448a19bp-9, 0x0.0p+0, 0x0.0p+0, -0x1.23a1c4c1c07a9p-4, 0x0.0p+0, 0x1.6824a65ebb434p-6, 0x0.0p+0, 0x1.465cfa7d39873p-6, -0x1.d178cb5c52897p-4, 0x1.8eb394c6fe292p-1, 0x0.0p+0, 0x1.f6b59c7afca8ep-4, -0x1.3e8eae95f4987p-7, 0x0.0p+0, 0x0.0p+0, -0x1.66d8abfaad179p-5, -0x1.14bc4f940b691p-9, 0x1.7bbed7c8cff74p-4, -0x1.56d1c82924987p-4, 0x1.880c57f3c643bp-4, 0x0.0p+0, 0x1.8271e2dab0d4cp-5, 0x1.e26fa1037d4edp-5, 0x1.bb997070d9b03p-9, -0x1.277b1ed6528a0p-8, 0x1.00aea9bb25002p-6, 0x1.c98d44b781486p-8, -0x1.9e8677b2ac8a3p-4, 0x1.d509830260a9ap-7, -0x1.4d8efa628bb91p-7, -0x1.d9c01bb583e02p-9, -0x1.3dec83c7ef223p-11, 0x1.8df1a4e68240bp-6, -0x1.0b4ac21c99cf8p-5, 0x0.0p+0, 0x1.046d5f1073b83p-4, 0x1.4b77402624990p-5, -0x1.5e6f3e5c802f4p-3, -0x1.1e355dcdec0fap-7, 0x1.4a42241c842a5p-5, 0x1.7ba9a95f4b072p-6, -0x1.ce6e8f50502dbp-5, 0x1.6c30c46de0ffcp-7, -0x1.83a9d6be335acp-5, -0x1.739fc6c86a147p-4, -0x1.e31deb4f18adfp-6, 0x0.0p+0, 0x1.9c9cc8c3d7d44p-7, -0x1.76cd918915b37p-6, 0x0.0p+0, 0x1.a23d00eaa59ecp-12, -0x1.0c9a7a021f170p-6, -0x1.c223b6b7769e3p-6, -0x1.356b178473054p-4, 0x1.c89ea5fcc43aep-8, 0x1.1a185f293f6cap-4, 0x0.0p+0, 0x1.015dc8fb14db4p-8, 0x1.937511a7905c7p-13, 0x0.0p+0, 0x1.8eb394c6fe291p-1, 0x1.043559e79bceap-9, 0x0.0p+0, 0x1.3e309e350aa9dp-6, -0x1.af14c06b0bd96p-5, -0x1.5cfd1f701f29ap-7, -0x1.1da731b1d2e2bp-4, 0x1.93f0a7b6b78d5p-6, -0x1.96b7386a06d77p-6, -0x1.0a4ba41a6b37bp-5, 0x1.a155406a88cd1p-7, 0x0.0p+0, -0x1.ecc8c26da4e09p-8, -0x1.76c9efc8edf68p-7, 0x1.87d3dc30931cdp-4, -0x1.42d31607c618cp-6, 0x1.9ef437d2bf3bfp-6, 0x1.5a99703f42987p-9, 0x0.0p+0, -0x1.6f2efbdbccd64p-5, 0x1.76cbe5fd74bb2p-6, 0x1.822838e00fba5p-6, -0x1.5712d315f6b62p-7, -0x1.4e881eeacaa81p-7, 0x1.c1b9aa8bc47f7p-6, -0x1.101f628d6fd36p-5, -0x1.c1cdc801145f7p-4, -0x1.2d173e2107f30p-5, 0x1.a8917ca39e153p-7, 0x0.0p+0, 0x1.f518544925a3ap-5, 0x1.58508637a4e4fp-9, 0x0.0p+0, -0x1.33803f116022dp-5, 0x1.d1b30e38ed413p-8, 0x1.a6171908d47c1p-7, 0x0.0p+0, 0x1.3b90849dbd4a6p-6, 0x1.1ff83a89f999dp-5, 0x1.32e76420eddcbp-13, 0x0.0p+0, -0x1.7965febf6b30ep-6, -0x1.186ec151fe7cdp-6, 0x1.f6b59c7afca8dp-4, 0x1.043559e79bceap-9, 0x1.8eb394c6fe292p-1, 0x1.487647183196dp-4, -0x1.574b0fbeff461p-7, 0x1.d4f2fb6106524p-8, 0x1.1d000a5563c3fp-4, 0x1.3d80301fc2fd6p-6, 0x0.0p+0, 0x1.3dd453a090409p-4, -0x1.3982f88b532a8p-6, 0x0.0p+0, -0x1.fb6b69d3445e8p-5, -0x1.200f1a99c19c9p-8, -0x1.1c8e6b83ad22cp-4, 0x1.b176d256c2935p-5, 0x0.0p+0, -0x1.65622e6518cccp-6, 0x1.def3969454854p-9, -0x1.e32d57d349e11p-6, 0x1.5e6587e6e4c46p-5, -0x1.9af6f7f229463p-8, -0x1.0eed00a18813dp-18, -0x1.3de195c13a09ep-6, -0x1.7488e0decda04p-5, 0x0.0p+0, -0x1.07f7b15cc907fp-5, -0x1.13d9f086e3e9ep-5, 0x1.9a7f95371ca67p-4, 0x1.f2f95a0a63d17p-8, 0x1.f8d15c3b979a9p-8, -0x1.88a8c5da2807bp-5, 0x1.1824b909fae5ep-4, -0x1.4f71975535b79p-6, 0x1.1d07afb44df7ap-4, 0x1.f6f0f9ea0299bp-5, 0x1.5a650de5e0dc7p-7, -0x1.2a631ccc852f5p-7, -0x1.804ad1377ad8cp-4, 0x1.15572d72f324ap-4, 0x1.01ca471bb75e1p-8, 0x0.0p+0, 0x1.7dcfbae81a0bdp-6, 0x0.0p+0, 0x1.1f20bcc59060ep-4, 0x1.84c1bb8826a44p-5, 0x0.0p+0, 0x0.0p+0, 0x1.5033ee6347331p-7, -0x1.13b57432e10b5p-9, -0x1.3e8eae95f4988p-7, 0x0.0p+0, 0x1.487647183196dp-4, 0x1.8eb394c6fe292p-1, -0x1.2745ef738493ap-4, -0x1.1cc21c6123c12p-6, 0x0.0p+0, -0x1.21a7aba325193p-6, -0x1.4520e9102e920p-5, 0x0.0p+0, 0x1.405e2c88c17e8p-5, -0x1.c80da2e15d09dp-6, 0x1.542af23ee9a96p-5, -0x1.300c0a1f4bbd4p-5, 0x0.0p+0, 0x1.f7fcefb26e858p-6, 0x1.555b5c7a2889ep-5, 0x0.0p+0, -0x1.f0c54eb1ecc29p-5, 0x0.0p+0, -0x1.0d8924195c6bep-5, 0x0.0p+0, -0x1.46a89b7464b6cp-5, 0x1.3c2df01d5dd3dp-5, 0x0.0p+0, 0x0.0p+0, 0x1.2b16ecd4a4f31p-12, 0x1.7e5ff408ccc5cp-5, -0x1.597011fb16f11p-7, -0x1.0abe017bf162dp-7, 0x1.6e20c69f6428dp-6, 0x0.0p+0, -0x1.b4f1a932d62b4p-5, 0x1.702b71600c254p-5, 0x1.a892ff64cc260p-4, -0x1.89b837379d309p-7, 0x0.0p+0, -0x1.2aea845854b8fp-6, 0x0.0p+0, 0x0.0p+0, -0x1.02e77be8bdc72p-5, 0x1.35bfcf7d902a5p-4, -0x1.edf304695c967p-8, -0x1.01d0c2cbf339bp-4, 0x0.0p+0, 0x1.3e309e350aa9ep-6, -0x1.574b0fbeff461p-7, -0x1.2745ef738493ap-4, 0x1.8eb394c6fe292p-1, -0x1.96e98b4607c50p-6, -0x1.49ae4abc53a66p-9, 0x0.0p+0, 0x1.18e1b2ad89743p-6, 0x1.56ff4395f1a4cp-6, 0x0.0p+0, -0x1.bc406af5ea32cp-7, 0x0.0p+0, 0x0.0p+0, -0x1.89334a950051fp-4, -0x1.d416a1c7a1b58p-5, -0x1.103d128056bf2p-5, 0x0.0p+0, -0x1.f8e601f3721fcp-7, 0x0.0p+0, 0x1.f74a7c4c0db39p-5, 0x1.ae7deab51814ap-7, 0x0.0p+0, 0x1.8be07cd2ca2fcp-5, 0x0.0p+0, -0x1.0c62b7d047d6dp-7, 0x0.0p+0, 0x1.172ae74ca11d9p-3, 0x1.58da9430746fbp-4, 0x0.0p+0, -0x1.02058dde10ea0p-6, 0x1.4d457c3fdfc4cp-3, 0x1.ed4cf6be03637p-4, 0x0.0p+0, -0x1.2a2fc513818e5p-5, 0x1.90c525a7a6e14p-7, 0x0.0p+0, 0x0.0p+0, -0x1.30820051fc200p-4, 0x1.6b8e7c6fe423cp-11, 0x0.0p+0, -0x1.af14c06b0bd96p-5, 0x1.d4f2fb6106525p-8, 0x1.8eb394c6fe292p-1, -0x1.77e67e3e3f03fp-6, -0x1.922c36e25ff7ep-4, 0x1.8ab8763705aebp-8, -0x1.7b80c50f0bb59p-4, -0x1.3c4be57eed12ap-4, -0x1.13e1dd65744b5p-4, -0x1.48f2b177c7bd1p-6, -0x1.1d56968c9358cp-5, -0x1.6666987781273p-6, 0x1.e64e081c2b611p-4, 0x1.3034da265a1c8p-8, 0x1.2ac238df11fc9p-4, 0x1.5bf7a3908e4cap-4, -0x1.2f4a9c3d98d46p-5, 0x1.6d5f3ca1b963ap-7, 0x1.2e16856d5be00p-5, -0x1.dac1e02e8b4c7p-4, 0x1.535d4ddfedc60p-5, -0x1.2a2fb50d6db52p-4, 0x0.0p+0, 0x1.17334e53d27dbp-6, -0x1.180022baf89afp-6, -0x1.81631b00cf9f2p-4, 0x1.98bd40f2075cep-7, 0x0.0p+0, 0x0.0p+0, 0x1.33a53393f6433p-5, 0x1.74a95130ce4d1p-6, 0x1.6cc1dfbbd0a4fp-10, 0x0.0p+0, -0x1.5630cbf80ce85p-4, 0x0.0p+0, -0x1.66d8abfaad178p-5, -0x1.5cfd1f701f29ap-7, 0x1.1d000a5563c40p-4, -0x1.77e67e3e3f03fp-6, 0x1.8eb394c6fe292p-1, 0x0.0p+0, -0x1.f6a72c086607dp-6, -0x1.ab53815d5632ap-5, 0x1.42cd1f61563c9p-4, -0x1.9cebed6badd40p-5, 0x1.4a03319e7b8e9p-5, -0x1.4bc0989b77263p-8, 0x0.0p+0, -0x1.e0a17e5a9cd4cp-6, 0x1.e37fe590b23cdp-5, -0x1.20c1d62a0c7e5p-5, -0x1.41f38e6e7339cp-6, 0x1.68c6eee57eadap-8, -0x1.4149cec30a1a6p-7, 0x0.0p+0, 0x1.5a577f1512c78p-6, -0x1.7537cfb3fdb3fp-5, 0x1.de1a2e28d69b7p-7, 0x1.5fabc835d3f57p-5, -0x1.b0efcbb50c22fp-4, -0x1.d99f75d5f4b3ap-6, -0x1.45bc3b986d5a1p-7, -0x1.fbacae374ca0ap-7, -0x1.3e1f249249d0ap-6, -0x1.29c43bb14a020p-7, -0x1.c3a3d02cd8f2bp-5, 0x1.cc94ab761021ep-6, -0x1.381a256f0f20cp-5, -0x1.1b56824fbdf97p-4, -0x1.40d09f66df233p-5, 0x1.f6b8b194e37c7p-6, 0x1.243aeb632dbfap-7, -0x1.257cd3f745c02p-6, 0x1.9dadd1e4be53dp-7, 0x1.e411fbf250d5fp-13, -0x1.4c921488a40f2p-5, -0x1.047f1455b2eacp-6, -0x1.cf11c6fe22dc1p-5, 0x0.0p+0, 0x1.c81b7b766b1e0p-5, -0x1.a1ca19d8e30d8p-7, -0x1.14bc4f940b691p-9, 0x0.0p+0, 0x1.3d80301fc2fd6p-6, -0x1.1cc21c6123c11p-6, 0x0.0p+0, 0x1.8eb394c6fe292p-1, 0x0.0p+0, 0x0.0p+0, -0x1.1a71b0295601dp-7, 0x0.0p+0, 0x1.8c888273ff424p-8, -0x1.a7cea648b9a8ap-3, 0x0.0p+0, 0x1.b5d2705302252p-8, 0x0.0p+0, 0x1.4c2cbed9d499bp-7, -0x1.5efaa5219ae16p-8, 0x0.0p+0, 0x1.36a77d202d111p-7, 0x0.0p+0, 0x1.70d58377f8295p-6, -0x1.b02aee6c65aadp-7, -0x1.eb6e72e9e3272p-4, -0x1.ee8fc41688931p-6, 0x0.0p+0, 0x0.0p+0, 0x1.89f5029cb0c50p-5, 0x1.44193fe32be51p-7, 0x0.0p+0, 0x1.886e546a18fafp-4, 0x1.7bbed7c8cff75p-4, 0x0.0p+0, -0x1.f6a72c086607dp-6, 0x0.0p+0, 0x1.8eb394c6fe292p-1, 0x0.0p+0, -0x1.14c59f3a266eep-5, -0x1.13d6a896c7799p-3, 0x0.0p+0, -0x1.84fc2e56bd2d5p-4, 0x0.0p+0, 0x0.0p+0, 0x1.ae8066ec5a691p-4, 0x0.0p+0, 0x1.a78bb682adc22p-4, -0x1.ff7ebb2e17717p-4, 0x1.ad028b9030d51p-4, 0x1.1a11453bc9a2ap-4, 0x0.0p+0, 0x0.0p+0, 0x1.28e0da4bf6f1fp-4, 0x0.0p+0, -0x1.79e7ecf6a7e58p-5, 0x0.0p+0, -0x1.9025c2b863152p-5, 0x0.0p+0, 0x1.bf09c75d01b95p-5, 0x0.0p+0, 0x1.63993e4df2892p-7, -0x1.eac2d815d0cb9p-6, 0x1.9dd55e529bad7p-6, 0x0.0p+0, 0x1.cb44a88fa10e7p-6, 0x1.4ffd8b3447167p-7, -0x1.56d1c82924986p-4, -0x1.1da731b1d2e2bp-4, 0x1.3dd453a090409p-4, -0x1.21a7aba325192p-6, -0x1.96e98b4607c4fp-6, -0x1.922c36e25ff7dp-4, -0x1.ab53815d5632ap-5, 0x1.8eb394c6fe292p-1, 0x1.fe0cd2f85a33fp-4, 0x0.0p+0, 0x1.c3804810f7c3ap-7, -0x1.20128d819d37bp-4, -0x1.3e85b1e4739b8p-5, 0x0.0p+0, 0x1.6f1390443125ep-8, 0x1.c79d756588e27p-8, 0x1.54c50579e1b3ap-7, 0x1.411c098e23131p-7, 0x1.07d55968234d2p-8, 0x1.675266a75e26cp-4, -0x1.1ed3612dd4b33p-4, -0x1.20d1424f289b6p-6, 0x1.6802ccc0a02a2p-6, 0x0.0p+0, 0x1.9ac6712fd498ap-5, 0x1.b4549a4b93cfap-7, -0x1.56098af03d751p-5, -0x1.6f6b36ca371dcp-6, -0x1.954a795192fd0p-5, -0x1.80a21207e3415p-7, -0x1.0594fd4413cb4p-3, -0x1.97511f5464342p-5, -0x1.4b80aa0ef6da5p-6, -0x1.61c463500d512p-4, -0x1.12c4373248683p-5, 0x1.6f4ffd6129c97p-4, 0x1.35bd97f0345b0p-5, 0x1.f9634e31cde85p-6, -0x1.248923bd24b49p-4, 0x1.05f1dc267a952p-4, -0x1.a552c46f7f37ap-8, 0x0.0p+0, -0x1.8481251bef754p-7, 0x0.0p+0, 0x1.9b0bc27107443p-7, 0x0.0p+0, -0x1.336f765d65c15p-5, 0x1.92368086060cbp-5, 0x1.880c57f3c643ap-4, 0x1.93f0a7b6b78d6p-6, -0x1.3982f88b532a9p-6, -0x1.4520e9102e91fp-5, -0x1.49ae4abc53a66p-9, 0x1.8ab8763705aeap-8, 0x1.42cd1f61563c8p-4, -0x1.1a71b0295601cp-7, -0x1.14c59f3a266eep-5, 0x1.fe0cd2f85a33fp-4, 0x1.8eb394c6fe292p-1, 0x0.0p+0, -0x1.fd3f55c7881bdp-5, 0x1.9d4a00d563c17p-7, 0x1.1052fcb6793eap-6, -0x1.2d937cd5c42c6p-6, 0x0.0p+0, -0x1.1e737438b13d4p-6, 0x1.8bbf6566e8663p-8, 0x1.f2ff0815d4031p-12, 0x1.c3cbe6f242612p-6, -0x1.1ff1531a2f340p-7, -0x1.36706054b7b32p-11, 0x0.0p+0, 0x1.ff4aab77a229cp-6, 0x0.0p+0, -0x1.94669cfa41758p-9, 0x1.607ab8ebf1faep-10, 0x1.195d9812a6cfdp-6, 0x0.0p+0, -0x1.e1687247aa3a1p-7, 0x1.32076d6c0a055p-5, 0x1.76f88253b5dadp-4, 0x0.0p+0, -0x1.842e7096d7a60p-7, 0x1.4a3853db6b793p-3, 0x1.6bc85c58ad86ep-6, 0x0.0p+0, 0x1.a51fb22fdaa5fp-5, -0x1.ed6fb29dbebe7p-6, 0x1.b1f878bbf3056p-9, 0x0.0p+0, 0x1.4fde7679246d1p-4, 0x0.0p+0, 0x1.1d574d2669563p-10, 0x1.0a3df0e42b2b3p-6, 0x0.0p+0, 0x0.0p+0, -0x1.9cebed6badd40p-5, 0x0.0p+0, -0x1.13d6a896c7799p-3, 0x1.8eb394c6fe292p-1, 0x0.0p+0, -0x1.78f2444af48d7p-4, 0x0.0p+0, 0x0.0p+0, 0x1.61a67a9022ee0p-3, -0x1.a42f67e7e8ce5p-3, 0x0.0p+0, 0x1.cf6d92a444526p-4, 0x0.0p+0, 0x0.0p+0, 0x1.84dd3c96727e5p-6, -0x1.f01e378787b5fp-5, 0x1.1501cf0caae72p-3, 0x0.0p+0, 0x1.2acb5445460bcp-8, 0x1.04f6dfc4d6a2bp-3, 0x1.8271e2dab0d4cp-5, 0x0.0p+0, -0x1.fb6b69d3445e8p-5, 0x1.405e2c88c17e8p-5, 0x1.18e1b2ad89742p-6, 0x0.0p+0, 0x1.4a03319e7b8e9p-5, 0x1.c3804810f7c3ep-7, -0x1.fd3f55c7881bdp-5, 0x0.0p+0, 0x1.8eb394c6fe292p-1, 0x1.e0a6b22daedf9p-8, 0x0.0p+0, 0x1.05b36f9f66cd8p-7, 0x0.0p+0, -0x1.1871706a6fe43p-3, -0x1.f20f27dcbec49p-5, -0x1.67f78f4b685cbp-4, 0x1.85203333e6366p-8, -0x1.7eb30384504f4p-8, -0x1.56f9070943c65p-6, -0x1.ec742a7fdee9ap-6, 0x1.4a2a1d1217a23p-7, 0x0.0p+0, -0x1.5bf42a8211fb0p-6, -0x1.92dda89b67ca8p-5, 0x1.22f0abbe4acd9p-4, 0x1.4ae6091dfe66ep-8, -0x1.110846bf138d8p-5, -0x1.00ad82960c644p-5, 0x1.560fdc0c67cc2p-6, -0x1.ad858df2a73a8p-6, 0x1.7a0a327d5b8eep-5, 0x1.600d9c09eefe0p-5, -0x1.780a570f69d7bp-5, 0x1.68e1d18ec930bp-6, 0x1.1eb96ce8a9eb6p-6, 0x0.0p+0, -0x1.011d0d43a3f4ap-5, 0x1.151822ddf6570p-5, 0x1.15b703a1f2896p-4, 0x0.0p+0, 0x1.1204196c588d8p-4, -0x1.d8adee6bc4705p-7, -0x1.fbbca5e08ec3ap-8, 0x0.0p+0, -0x1.b5776ab6d2bafp-8, 0x1.66f21f66f451dp-5, 0x1.e26fa1037d4efp-5, -0x1.96b7386a06d77p-6, -0x1.200f1a99c19c8p-8, -0x1.c80da2e15d09dp-6, 0x1.56ff4395f1a4cp-6, -0x1.7b80c50f0bb59p-4, -0x1.4bc0989b77262p-8, 0x0.0p+0, -0x1.84fc2e56bd2d4p-4, -0x1.20128d819d37bp-4, 0x1.9d4a00d563c19p-7, -0x1.78f2444af48d7p-4, 0x1.e0a6b22daedf9p-8, 0x1.8eb394c6fe292p-1, -0x1.c5c1b4f9b177ap-5, -0x1.8478d4a336c58p-10, -0x1.a956f0a26ea1ep-11, -0x1.fe9a45508b44cp-9, 0x1.a3e06962cd13ap-5, -0x1.8152bfbf8b14dp-5, 0x1.bf1a2a228b36dp-6, 0x1.5da4c50c54049p-4, -0x1.5958585a8ff00p-9, -0x1.61059b1563da0p-4, 0x1.1766b978dc38ep-4, 0x1.817561168b783p-5, 0x1.431ff2994d267p-5, 0x1.706b7fcdbf6aep-8, 0x1.964c85f682e10p-6, 0x1.c7bc7f907d1d4p-6, -0x1.482c30670313ap-5, -0x1.eb4e7f71c6a0ap-5, -0x1.779039525d027p-5, -0x1.d8ab9bbdaf405p-5, 0x1.0188a5a28936bp-9, -0x1.ab1bb2a03c66ep-11, -0x1.ec5b354a69543p-5, -0x1.58cae7defdc2ep-6, -0x1.0392fd1b5cdd2p-9, 0x1.0139bd078d044p-4, -0x1.be7f97b32d248p-5, -0x1.4389b29fd302cp-5, -0x1.8846ce9454eeap-6, 0x1.c7f7f9b6925f8p-4, 0x1.5e2c6bb647d31p-8, 0x1.1de3cc3f1521fp-4, 0x0.0p+0, 0x1.2d8b885e813d8p-4, -0x1.6f33a3c516c3dp-5, -0x1.2471c00a1fa04p-6, 0x1.bb997070d9b03p-9, -0x1.0a4ba41a6b37bp-5, -0x1.1c8e6b83ad22bp-4, 0x1.542af23ee9a97p-5, 0x0.0p+0, -0x1.3c4be57eed12ap-4, 0x0.0p+0, 0x1.8c888273ff424p-8, 0x0.0p+0, -0x1.3e85b1e4739b7p-5, 0x1.1052fcb6793eap-6, -0x1.c5c1b4f9b1778p-5, 0x1.8eb394c6fe292p-1, 0x1.a764b40c381e9p-7, 0x0.0p+0, -0x1.a2be2cce87628p-8, 0x0.0p+0, -0x1.127a0e0a06a28p-9, 0x1.f4807402f3b65p-5, 0x1.141f9e6ae10adp-4, 0x0.0p+0, 0x0.0p+0, 0x1.4a44e28828519p-5, -0x1.157a1ac92bd22p-5, -0x1.5ee8cc06e23a2p-7, 0x0.0p+0, 0x1.286ff0fe1f7b8p-6, 0x1.e2ff8c7ca83a0p-11, -0x1.c629216d43a8cp-4, -0x1.c1183457ba8d7p-5, -0x1.f6a07e4ad9140p-8, -0x1.7cb5dd267ba0ep-7, 0x1.f0c80ccfd8af7p-9, 0x0.0p+0, 0x1.250d35c0f5c60p-6, -0x1.11bdc59a194d2p-8, -0x1.a562fd2f8c7cbp-5, 0x1.9a5b7121d7e55p-8, 0x0.0p+0, 0x0.0p+0, 0x1.a107eb52ba71cp-5, 0x0.0p+0, -0x1.19721d61afd4cp-6, 0x1.1e8956200a9fcp-9, -0x1.277b1ed6528a0p-8, 0x1.a155406a88cd0p-7, 0x1.b176d256c2934p-5, -0x1.300c0a1f4bbd3p-5, 0x0.0p+0, 0x0.0p+0, -0x1.e0a17e5a9cd4cp-6, -0x1.a7cea648b9a8ap-3, 0x0.0p+0, 0x0.0p+0, -0x1.2d937cd5c42c6p-6, 0x0.0p+0, 0x1.05b36f9f66cd8p-7, -0x1.8478d4a336c58p-10, 0x1.a764b40c381e8p-7, 0x1.8eb394c6fe292p-1, 0x0.0p+0, 0x1.8ded922a6932dp-6, 0x0.0p+0, 0x0.0p+0, -0x1.d85a2ab386b7dp-6, -0x1.bb8399bf26876p-9, 0x1.858693a4b1120p-7, -0x1.91e1604cd1334p-6, 0x1.e6583985b3282p-6, 0x1.0ab94db641e61p-6, -0x1.76c0e4f4c4aacp-7, 0x0.0p+0, -0x1.2986849220aaep-7, 0x1.18e14bca372c2p-7, 0x1.89d161b231e81p-5, -0x1.cd710de1bcad5p-6, -0x1.c1542422f0786p-4, -0x1.c430ce71aa5a4p-6, -0x1.792f7303c2a63p-7, 0x0.0p+0, 0x1.cdaf53e4f96e1p-6, -0x1.3e10313561b1ep-5, -0x1.087e642634088p-6, 0x0.0p+0, 0x1.00aea9bb25002p-6, 0x0.0p+0, 0x1.e37fe590b23cdp-5, 0x0.0p+0, 0x1.ae8066ec5a692p-4, 0x1.6f1390443125ep-8, 0x0.0p+0, 0x1.61a67a9022ee0p-3, 0x0.0p+0, -0x1.a956f0a26e94bp-11, 0x0.0p+0, 0x0.0p+0, 0x1.8eb394c6fe292p-1, 0x0.0p+0, -0x1.1ec7789b75626p-6, -0x1.47ee8df2b6a6cp-6, -0x1.4f656dd94397dp-6, 0x0.0p+0, -0x1.db7d8d910337dp-6, 0x1.34c9fc2d2dfddp-3, 0x1.6974c9592ddd2p-5, -0x1.69a2eaed755f0p-4, 0x1.44a7127a293efp-6, 0x1.94f00e69894d8p-5, -0x1.2f735209671d0p-6, -0x1.494254c4d7d63p-4, 0x1.ccb9100ed3f46p-7, -0x1.a2296fd5aa17fp-5, 0x0.0p+0, 0x0.0p+0, -0x1.50cbefc6ca526p-7, 0x1.2391c89e9661cp-4, 0x1.696565a931c1dp-7, -0x1.e03823da02a41p-10, 0x1.dd966b70ffc02p-6, 0x1.026c8f44a55e0p-3, 0x0.0p+0, -0x1.25870675a5451p-6, 0x1.fe9bdc41f3ec5p-4, 0x0.0p+0, -0x1.ca8e8b50cc7a6p-4, 0x1.23e52f52cc057p-4, 0x1.c98d44b781486p-8, -0x1.ecc8c26da4e09p-8, -0x1.65622e6518cccp-6, -0x1.13e1dd65744b5p-4, -0x1.20c1d62a0c7e5p-5, 0x1.c79d756588e27p-8, -0x1.1e737438b13d5p-6, 0x0.0p+0, -0x1.1871706a6fe42p-3, -0x1.fe9a45508b449p-9, -0x1.a2be2cce87628p-8, 0x1.8eb394c6fe291p-1, -0x1.2174b76d97919p-5, -0x1.4e52daf4b63a6p-4, -0x1.119f4da6e3bc1p-8, -0x1.127301f64b0c7p-8, -0x1.705aaace45c80p-11, -0x1.691ca5067b77cp-7, -0x1.a6852b672062fp-7, 0x1.9c9b25e67e8a0p-5, -0x1.33407ebd0e3c5p-6, -0x1.b97e0cfb4d9edp-7, 0x1.1606c86d9cee3p-5, 0x1.3c361bcd9e005p-9, 0x0.0p+0, -0x1.090d6f035b892p-5, -0x1.97fc705cddfcbp-9, 0x1.2349b9bb4558ap-4, -0x1.6fcdacab3b6c8p-4, -0x1.024c72273e345p-7, -0x1.c643e8cc2059bp-5, 0x0.0p+0, 0x1.acf30d8c042d1p-6, -0x1.ae2dbe30a839ep-6, -0x1.833eb1032198fp-5, 0x0.0p+0, 0x1.5332639c1f980p-6, 0x0.0p+0, 0x1.095cc874bfdd6p-6, 0x1.5a9e99a7f2205p-6, 0x1.08e3d0ca6a4ccp-6, 0x0.0p+0, 0x1.2d084d2ebdc57p-9, -0x1.234a56655cfbfp-4, -0x1.9e8677b2ac8a3p-4, -0x1.76c9efc8edf67p-7, 0x1.def3969454855p-9, -0x1.48f2b177c7bd0p-6, -0x1.41f38e6e7339cp-6, 0x0.0p+0, 0x1.a78bb682adc22p-4, 0x1.54c50579e1b38p-7, 0x1.8bbf6566e8664p-8, 0x0.0p+0, -0x1.f20f27dcbec49p-5, 0x1.a3e06962cd13ap-5, 0x0.0p+0, 0x0.0p+0, -0x1.1ec7789b75626p-6, -0x1.2174b76d97918p-5, 0x1.8eb394c6fe292p-1, -0x1.dd5c8af6fa6e1p-10, -0x1.6c9824300d77dp-5, -0x1.ab8ca5d835099p-5, 0x1.bfc85354d2e3dp-8, -0x1.29688b4ca3e0cp-9, -0x1.cecf2c6f6057ep-7, 0x1.1c81f1f3ca85dp-4, 0x1.3be75f5dbc984p-8, -0x1.5531e6baf4c67p-4, -0x1.b8ba8dd3c3771p-5, 0x0.0p+0, 0x1.7059c9e66c4aap-5, -0x1.4062da3c951dap-6, -0x1.c47f31cf82135p-8, -0x1.96e4a3c1c5be2p-7, 0x1.2904a4cf643c0p-5, 0x1.364153e8baf72p-7, -0x1.438ddf2af44f2p-6, 0x1.1bb65112e8dcbp-6, 0x1.92bc574cc2e5ap-5, 0x1.5a35f13a62fcdp-6, 0x1.0080b5cb5e286p-5, 0x1.f6eb82724d485p-6, 0x1.b40a6cff9e4aap-5, -0x1.20eeeacdb4bc5p-4, 0x1.e15f5149a98b0p-7, 0x0.0p+0, 0x1.775ed40f71494p-6, 0x1.1eeae8d045152p-5, 0x1.d509830260a9ap-7, 0x1.87d3dc30931cdp-4, -0x1.e32d57d349e11p-6, 0x1.f7fcefb26e857p-6, -0x1.bc406af5ea32bp-7, -0x1.1d56968c9358bp-5, 0x1.68c6eee57ead9p-8, 0x1.b5d2705302251p-8, 0x0.0p+0, 0x1.411c098e23135p-7, 0x1.f2ff0815d4049p-12, 0x0.0p+0, -0x1.67f78f4b685cbp-4, -0x1.8152bfbf8b14dp-5, -0x1.127a0e0a06a1fp-9, 0x1.8ded922a6932dp-6, -0x1.47ee8df2b6a6cp-6, -0x1.4e52daf4b63a6p-4, -0x1.dd5c8af6fa6e1p-10, 0x1.8eb394c6fe292p-1, -0x1.d3ba50515a5fep-6, 0x1.f231d0a13c21ap-6, -0x1.4659ffcb6c938p-5, -0x1.c035de270d070p-8, 0x1.ca29f590bc546p-6, 0x0.0p+0, 0x1.4380cb0b66350p-7, -0x1.f1868faf65cf2p-7, 0x1.6abf50cfa6f8fp-7, 0x1.7298068ec2c85p-4, -0x1.ac3ccc22f890dp-6, 0x1.75163b0627086p-5, 0x1.e3ed1451ce66bp-10, 0x1.bb2bc2f64bf91p-6, 0x1.3313a46947ff3p-7, -0x1.e46c61bb714c7p-5, -0x1.471fef19e9dfep-4, 0x0.0p+0, -0x1.cb0ffbe429682p-5, 0x1.7e714c1b51764p-6, -0x1.6417911eecbccp-5, 0x0.0p+0, 0x1.9ad22c55dfb91p-5, 0x0.0p+0, 0x1.306ec8599be68p-7, -0x1.d5587013316cep-5, 0x0.0p+0, -0x1.60c506a12f1dbp-4, 0x1.a739448b89887p-5, 0x0.0p+0, -0x1.4d8efa628bb91p-7, -0x1.42d31607c618cp-6, 0x1.5e6587e6e4c46p-5, 0x1.555b5c7a2889ep-5, 0x0.0p+0, -0x1.6666987781273p-6, -0x1.4149cec30a1a6p-7, 0x1.07d55968234d2p-8, 0x1.c3cbe6f242612p-6, 0x0.0p+0, 0x1.85203333e6366p-8, 0x1.bf1a2a228b36cp-6, 0x1.f4807402f3b67p-5, 0x0.0p+0, -0x1.4f656dd94397dp-6, -0x1.119f4da6e3bc1p-8, -0x1.6c9824300d77ep-5, -0x1.d3ba50515a5fep-6, 0x1.8eb394c6fe292p-1, -0x1.dcafb8ada7452p-9, 0x1.1ea9138d64564p-7, -0x1.5bd3cca7e6e78p-9, 0x1.9b5753ada6028p-7, 0x1.35fb379388eaap-4, 0x1.8410cb9ec35cap-5, -0x1.5d0a966cffec3p-5, 0x1.f8258a44553f5p-8, 0x0.0p+0, 0x1.c3fec58d354dep-5, -0x1.76b352788b457p-6, -0x1.0ea065aca8482p-5, 0x1.6ca2f8800a51fp-5, -0x1.7f313a930cd25p-6, -0x1.1a3d22869ca96p-5, 0x1.99ab6c172a0b5p-4, 0x1.3e75dee4e6eb0p-5, -0x1.5c2b3009edf01p-5, 0x1.e7ddb4cc59a53p-5, -0x1.750b1019d94a9p-7, 0x1.58bc85c8d979ap-5, -0x1.4c64cc3571629p-7, 0x1.2616e4210afa3p-5, 0x1.21e6a62351109p-4, 0x1.0cf3ddd46d22bp-4, 0x1.50ee22f0b2d97p-8, 0x1.143b3b4ed9bb5p-4, 0x0.0p+0, -0x1.3d618f9f5dd12p-11, -0x1.d9c01bb583e02p-9, 0x1.9ef437d2bf3bdp-6, -0x1.9af6f7f229462p-8, 0x1.e64e081c2b60fp-4, 0x0.0p+0, 0x1.675266a75e26cp-4, -0x1.1ff1531a2f33fp-7, 0x0.0p+0, -0x1.7eb30384504f4p-8, 0x1.5da4c50c54049p-4, 0x1.141f9e6ae10acp-4, -0x1.127301f64b0c7p-8, -0x1.ab8ca5d835099p-5, 0x1.f231d0a13c21ap-6, -0x1.dcafb8ada7450p-9, 0x1.8eb394c6fe292p-1, 0x1.50b538619484bp-7, 0x1.712031487ed8fp-4, -0x1.2fc597db79ff5p-4, 0x1.08c52fd3b4bfdp-5, -0x1.8623b0f67a9cdp-8, -0x1.07b83a6e826b3p-5, 0x1.9e136e624d778p-4, 0x0.0p+0, -0x1.59a3a71eed4b6p-6, -0x1.f2bb68453785fp-6, 0x0.0p+0, -0x1.f837004760cf8p-6, 0x1.2b1b0da010995p-4, 0x1.caa740ed5ff57p-6, 0x1.11e38336621ddp-8, 0x0.0p+0, 0x1.576dc577a9340p-4, -0x1.d25e64724cba4p-5, -0x1.00273e2bb40eap-5, 0x0.0p+0, -0x1.3dec83c7ef223p-11, 0x1.5a99703f42988p-9, -0x1.0eed00a18813dp-18, -0x1.f0c54eb1ecc29p-5, -0x1.89334a950051fp-4, 0x1.3034da265a1c8p-8, 0x1.5a577f1512c78p-6, -0x1.1ed3612dd4b32p-4, -0x1.36706054b7b32p-11, 0x0.0p+0, -0x1.56f9070943c65p-6, -0x1.5958585a8fefcp-9, 0x0.0p+0, 0x0.0p+0, -0x1.db7d8d910337cp-6, -0x1.705aaace45c80p-11, 0x1.bfc85354d2e3dp-8, -0x1.4659ffcb6c938p-5, 0x1.1ea9138d64564p-7, 0x1.50b538619484cp-7, 0x1.8eb394c6fe291p-1, -0x1.4b5eed613fd52p-5, 0x0.0p+0, -0x1.071bcc159a587p-6, -0x1.656df4f6dc0bfp-7, 0x1.92fc809152180p-4, 0x0.0p+0, 0x0.0p+0, 0x1.ca3f03d1a7bf1p-6, -0x1.3b549bec97e48p-4, 0x0.0p+0, 0x0.0p+0, -0x1.5504125ef19bap-5, 0x1.53d37aeae8ff1p-3, 0x1.e842a2741725ap-5, 0x1.343f1fba51c90p-4, 0x1.884e6d875a446p-7, 0x1.c36dc0204e3f5p-4, 0x1.26ddf66262ac8p-3, 0x0.0p+0, -0x1.ee867e6ecfa2cp-7, 0x1.cc61b9991e0d1p-7, 0x1.70f179eeb3691p-8, 0x1.175dc02d9fd87p-6, 0x1.8df1a4e68240ap-6, 0x0.0p+0, -0x1.3de195c13a09ep-6, 0x0.0p+0, -0x1.d416a1c7a1b59p-5, 0x0.0p+0, -0x1.7537cfb3fdb3ep-5, 0x0.0p+0, -0x1.ff7ebb2e17717p-4, -0x1.20d1424f289b7p-6, 0x0.0p+0, -0x1.a42f67e7e8ce5p-3, -0x1.ec742a7fdee9bp-6, -0x1.61059b1563da0p-4, 0x0.0p+0, 0x0.0p+0, 0x1.34c9fc2d2dfdcp-3, -0x1.691ca5067b77cp-7, -0x1.29688b4ca3e0cp-9, -0x1.c035de270d070p-8, -0x1.5bd3cca7e6e78p-9, 0x0.0p+0, -0x1.4b5eed613fd53p-5, 0x1.8eb394c6fe292p-1, -0x1.7ce9cdde60e3ap-8, 0x1.adac7af8c0e95p-4, -0x1.53153bedce89ap-6, -0x1.47864cc018a96p-7, 0x1.f14f1df53294bp-6, 0x1.1ace2cbf3c0dfp-9, 0x1.688a76983849fp-6, -0x1.91a27cb6df687p-7, 0x0.0p+0, -0x1.b1a91ebe326ecp-8, 0x1.431855beee662p-6, 0x0.0p+0, -0x1.33de9b5c585c0p-9, 0x1.d689e7c47ef73p-5, 0x1.22a06a2ebbffap-5, 0x1.2e604f0b6340ep-7, -0x1.2604b59435702p-7, 0x1.17e49063a4282p-4, 0x1.9fbb0aa35a60bp-5, 0x1.0c009691fa879p-6, 0x1.0cf77fa5fd1c9p-6, 0x1.1abffe6167448p-5, 0x1.9634838f705e8p-5, -0x1.2568a6f4eb7ecp-4, -0x1.0b4ac21c99cf8p-5, -0x1.6f2efbdbccd65p-5, -0x1.7488e0decda04p-5, -0x1.0d8924195c6bep-5, -0x1.103d128056bf2p-5, 0x0.0p+0, 0x1.de1a2e28d69b6p-7, 0x0.0p+0, 0x1.ad028b9030d50p-4, 0x1.6802ccc0a02a1p-6, 0x1.ff4aab77a229cp-6, 0x0.0p+0, 0x1.4a2a1d1217a25p-7, 0x1.1766b978dc38ep-4, 0x0.0p+0, 0x0.0p+0, 0x1.6974c9592ddd3p-5, -0x1.a6852b672062fp-7, -0x1.cecf2c6f60580p-7, 0x1.ca29f590bc546p-6, 0x1.9b5753ada6029p-7, -0x1.7ce9cdde60e39p-8, 0x1.8eb394c6fe292p-1, 0x0.0p+0, -0x1.83b79fda49439p-5, 0x1.3dc4ef43a09bcp-4, 0x1.2c76669449fe0p-7, 0x1.289449eb2d7cbp-8, 0x1.2d68260742e68p-4, 0x1.36d9f96ea96e2p-6, 0x0.0p+0, 0x1.b51be737563a0p-5, 0x1.3fd7321aef3e9p-4, -0x1.324224d75e9f6p-8, -0x1.86725532d364dp-5, -0x1.65972dbcb32f7p-5, -0x1.06ba979c2b79ap-4, 0x1.267a7ef107333p-10, 0x0.0p+0, -0x1.3cf09dc368ecdp-4, 0x1.275f3b06e88bep-7, -0x1.9eb0aef39d402p-5, 0x0.0p+0, 0x0.0p+0, 0x1.c76b0416ee499p-5, 0x1.76cbe5fd74bb2p-6, 0x0.0p+0, 0x1.2ac238df11fc9p-4, 0x1.5fabc835d3f57p-5, 0x0.0p+0, 0x1.1a11453bc9a2ap-4, 0x1.cf6d92a444526p-4, 0x0.0p+0, 0x1.817561168b783p-5, 0x0.0p+0, 0x0.0p+0, -0x1.69a2eaed755efp-4, 0x1.9c9b25e67e8a0p-5, 0x1.1c81f1f3ca85dp-4, 0x0.0p+0, 0x1.35fb379388ea9p-4, 0x1.712031487ed8ep-4, -0x1.071bcc159a587p-6, 0x1.adac7af8c0e95p-4, 0x0.0p+0, 0x1.8eb394c6fe292p-1, 0x0.0p+0, 0x0.0p+0, -0x1.8da53eec67749p-7, -0x1.fb8c984075404p-6, 0x1.bdf6b74958e4fp-5, 0x0.0p+0, -0x1.a1918fbaca882p-7, 0x1.a2c3e6f2f711dp-7, 0x1.4b785eae5423bp-7, -0x1.6184e7d1531fcp-5, 0x0.0p+0, 0x0.0p+0, -0x1.0a15366a2af80p-3, -0x1.23daba74aa26ap-6, 0x0.0p+0, 0x1.0d021bcf52a03p-6, -0x1.ea42582a16d0cp-6, 0x1.5822fa2b0846cp-5, 0x1.046d5f1073b83p-4, 0x1.822838e00fba4p-6, -0x1.07f7b15cc907fp-5, -0x1.46a89b7464b6cp-5, -0x1.f8e601f3721fcp-7, 0x1.5bf7a3908e4c9p-4, -0x1.b0efcbb50c230p-4, 0x1.9ac6712fd498ap-5, -0x1.94669cfa41757p-9, 0x0.0p+0, -0x1.5bf42a8211fb0p-6, 0x1.431ff2994d264p-5, 0x1.4a44e28828518p-5, -0x1.d85a2ab386b7dp-6, 0x1.44a7127a293f0p-6, -0x1.33407ebd0e3c5p-6, 0x1.3be75f5dbc984p-8, 0x1.4380cb0b66350p-7, 0x1.8410cb9ec35cap-5, -0x1.2fc597db79ff5p-4, -0x1.656df4f6dc0c0p-7, -0x1.53153bedce89ap-6, -0x1.83b79fda4943ap-5, 0x0.0p+0, 0x1.8eb394c6fe292p-1, 0x1.bc84e2a5e1334p-6, 0x1.63dae9a08a806p-4, 0x1.af569401b92a7p-8, -0x1.b0887579a3fddp-5, -0x1.68d5386c9f935p-7, 0x1.f28ff7ab01f50p-5, 0x1.cca11fc9b6d02p-7, 0x1.5211430a5249bp-4, 0x0.0p+0, -0x1.8e90e959ee251p-4, 0x1.3967865446a1ap-5, 0x1.397b32b40739bp-7, -0x1.9db1a3f8e630fp-6, 0x1.6fe8b8cb55f05p-5, 0x1.7ae8d08fd4f01p-11, 0x1.c06c5ee297806p-7, -0x1.1cab435fe5325p-5, 0x1.0a776f7b6f829p-7, 0x1.1a7de318d676fp-6, -0x1.9f57fd7e6ddc9p-5, 0x0.0p+0, -0x1.1e9a6be8caae3p-7, 0x1.a02e34251897ap-6, 0x1.4b77402624990p-5, -0x1.5712d315f6b62p-7, -0x1.13d9f086e3e9fp-5, 0x1.3c2df01d5dd3dp-5, 0x0.0p+0, -0x1.2f4a9c3d98d46p-5, -0x1.d99f75d5f4b3ap-6, 0x1.b4549a4b93cfbp-7, 0x1.607ab8ebf1faep-10, 0x0.0p+0, -0x1.92dda89b67ca7p-5, 0x1.706b7fcdbf6abp-8, -0x1.157a1ac92bd22p-5, -0x1.bb8399bf26877p-9, 0x0.0p+0, -0x1.b97e0cfb4d9edp-7, -0x1.5531e6baf4c66p-4, -0x1.f1868faf65cf2p-7, -0x1.5d0a966cffec2p-5, 0x1.08c52fd3b4bfep-5, 0x0.0p+0, -0x1.47864cc018a97p-7, 0x1.3dc4ef43a09bcp-4, 0x0.0p+0, 0x1.bc84e2a5e1333p-6, 0x1.8eb394c6fe292p-1, 0x1.f55f8e2ef9f3cp-5, -0x1.b9c40123fe2d4p-9, 0x1.eaf4b74b08509p-5, 0x1.0dc289c18fb2cp-7, -0x1.4f985759faa6bp-5, 0x1.036f191637934p-8, 0x1.d0d956a25b810p-7, 0x1.0dfa6bf6c943bp-5, 0x1.a1dc97d6ed388p-5, 0x0.0p+0, 0x1.3d351b8a1f4e8p-4, 0x1.21c5e90aff451p-6, -0x1.c091d8eb1188dp-9, 0x0.0p+0, 0x1.e8f43d52e99aap-6, 0x1.76ddb9db46695p-8, 0x0.0p+0, -0x1.d57e34b7aac68p-8, -0x1.5b13e9bbe9dfep-7, -0x1.03338bd38253dp-3, -0x1.5e6f3e5c802f4p-3, -0x1.4e881eeacaa80p-7, 0x1.9a7f95371ca67p-4, 0x1.6d5f3ca1b963ap-7, -0x1.45bc3b986d5a1p-7, 0x0.0p+0, 0x1.28e0da4bf6f1fp-4, -0x1.56098af03d751p-5, 0x1.195d9812a6cfdp-6, 0x0.0p+0, 0x1.22f0abbe4acd9p-4, 0x1.964c85f682e10p-6, -0x1.5ee8cc06e23a4p-7, 0x1.858693a4b1120p-7, 0x0.0p+0, 0x1.1606c86d9cee3p-5, -0x1.b8ba8dd3c3772p-5, 0x1.6abf50cfa6f8fp-7, 0x1.f8258a44553f7p-8, -0x1.8623b0f67a9cfp-8, 0x0.0p+0, 0x1.f14f1df53294bp-6, 0x1.2c76669449fe0p-7, 0x0.0p+0, 0x1.63dae9a08a806p-4, 0x1.f55f8e2ef9f3bp-5, 0x1.8eb394c6fe292p-1, 0x1.c1359ac31d1d1p-7, 0x1.0cc2afb38e186p-11, -0x1.44fa1862660ecp-9, 0x1.febab3bf780e6p-8, -0x1.4b352113b584bp-7, -0x1.55d1db86428dep-4, 0x0.0p+0, -0x1.a510100fb784dp-6, 0x1.a7d4e81806712p-9, 0x1.0f8b32ceb94e2p-7, 0x1.ac7db4d68643bp-9, 0x1.91b4266e9111bp-8, 0x0.0p+0, 0x1.89f746cd1397fp-7, 0x0.0p+0, -0x1.8a565702c8513p-4, 0x1.6b0bd3c1bd256p-4, 0x1.c05371ea0ee8cp-5, -0x1.652a999bbd094p-8, -0x1.1e355dcdec0fap-7, 0x1.c1b9aa8bc47f7p-6, 0x1.f2f95a0a63d17p-8, 0x0.0p+0, -0x1.fbacae374ca09p-7, -0x1.6f6b36ca371dcp-6, 0x0.0p+0, 0x0.0p+0, 0x1.4ae6091dfe66ep-8, 0x1.c7bc7f907d1d3p-6, 0x0.0p+0, -0x1.91e1604cd1335p-6, 0x1.94f00e69894d8p-5, 0x1.3c361bcd9e005p-9, 0x0.0p+0, 0x1.7298068ec2c85p-4, 0x0.0p+0, 0x0.0p+0, 0x1.92fc809152180p-4, 0x1.1ace2cbf3c0dfp-9, 0x1.289449eb2d7ccp-8, 0x0.0p+0, 0x1.af569401b92a6p-8, -0x1.b9c40123fe2d4p-9, 0x1.c1359ac31d1d3p-7, 0x1.8eb394c6fe292p-1, 0x1.060bf35d28befp-4, 0x0.0p+0, -0x1.867878bbe9b3fp-5, 0x0.0p+0, -0x1.fb287ddb43172p-8, 0x0.0p+0, 0x1.10353d9286bdfp-4, 0x0.0p+0, -0x1.324a164ffc0e4p-5, 0x0.0p+0, -0x1.fa6109ca22718p-4, 0x0.0p+0, -0x1.a63197e481224p-8, -0x1.4b590b2a8bb81p-4, 0x0.0p+0, 0x0.0p+0, 0x1.bd2bb25ed15a5p-7, 0x1.6834c490dd368p-4, 0x1.4a42241c842a5p-5, -0x1.101f628d6fd36p-5, 0x1.f8d15c3b979a9p-8, 0x1.2b16ecd4a4f0bp-12, 0x1.f74a7c4c0db39p-5, 0x0.0p+0, -0x1.3e1f249249d0ap-6, 0x0.0p+0, -0x1.79e7ecf6a7e57p-5, -0x1.954a795192fd1p-5, -0x1.e1687247aa3a1p-7, 0x1.84dd3c96727e5p-6, -0x1.110846bf138d8p-5, -0x1.482c30670313ap-5, 0x1.286ff0fe1f7b8p-6, 0x1.e6583985b3283p-6, -0x1.2f735209671d0p-6, 0x0.0p+0, 0x1.7059c9e66c4a9p-5, -0x1.ac3ccc22f890cp-6, 0x1.c3fec58d354dep-5, 0x1.688a7698384a0p-6, 0x1.2d68260742e68p-4, -0x1.8da53eec67749p-7, -0x1.b0887579a3fddp-5, 0x1.eaf4b74b0850ap-5, 0x1.0cc2afb38e17dp-11, 0x1.060bf35d28befp-4, 0x1.8eb394c6fe292p-1, 0x0.0p+0, 0x1.324525bdf062ep-6, -0x1.23124b6d5d267p-5, 0x1.119631c990250p-7, 0x0.0p+0, -0x1.4e041c4dc1435p-5, -0x1.8bcc7c80df51ep-6, 0x1.ebe37543cf354p-6, 0x0.0p+0, 0x1.034eb5554beffp-5, -0x1.ba7bf4fe0de1dp-8, 0x1.57f270e3a28afp-4, 0x0.0p+0, -0x1.1665a9c6916c6p-7, -0x1.3a6a0c127b1fbp-7, 0x1.7ba9a95f4b073p-6, -0x1.c1cdc801145f7p-4, -0x1.88a8c5da2807ap-5, 0x1.7e5ff408ccc5cp-5, 0x1.ae7deab51814ap-7, 0x1.2e16856d5be00p-5, -0x1.29c43bb14a021p-7, 0x1.4c2cbed9d499bp-7, 0x0.0p+0, -0x1.80a21207e3414p-7, 0x1.32076d6c0a055p-5, 0x0.0p+0, -0x1.00ad82960c643p-5, -0x1.eb4e7f71c6a09p-5, 0x1.e2ff8c7ca8397p-11, 0x1.0ab94db641e62p-6, -0x1.494254c4d7d63p-4, -0x1.090d6f035b893p-5, -0x1.4062da3c951d9p-6, 0x1.75163b0627086p-5, -0x1.76b352788b457p-6, -0x1.07b83a6e826b3p-5, 0x0.0p+0, -0x1.91a27cb6df687p-7, 0x1.36d9f96ea96e1p-6, 0x0.0p+0, -0x1.68d5386c9f937p-7, 0x1.0dc289c18fb2cp-7, -0x1.44fa1862660ebp-9, 0x1.8eb394c6fe292p-1, 0x1.48f55fb340a97p-5, -0x1.d32a4d1f07e84p-5, 0x0.0p+0, 0x1.9ebeb52e0a75bp-5, -0x1.4ee56e7529d80p-6, 0x1.15d669c47e9dcp-8, -0x1.ef45b0090af2ap-5, 0x1.d6d1291485ac9p-4, 0x1.379dbc91248c3p-8, -0x1.0c3f5d475f56fp-9, 0x0.0p+0, 0x1.20b54967bca8ap-3, -0x1.2f3660dbf6fb6p-7, 0x1.0231cfbfd23dbp-5, 0x1.0857f6deb51e3p-6, 0x1.2733adc8e9402p-4, 0x1.5fe5a2a4d0035p-5, 0x1.0f78904c09f1ep-6, -0x1.ce6e8f50502dap-5, -0x1.2d173e2107f31p-5, 0x1.1824b909fae5ep-4, -0x1.597011fb16f11p-7, 0x0.0p+0, -0x1.dac1e02e8b4c7p-4, -0x1.c3a3d02cd8f2bp-5, -0x1.5efaa5219ae16p-8, 0x0.0p+0, -0x1.0594fd4413cb3p-3, 0x1.76f88253b5dadp-4, 0x0.0p+0, 0x1.560fdc0c67cc3p-6, -0x1.779039525d028p-5, -0x1.c629216d43a8bp-4, -0x1.76c0e4f4c4aadp-7, 0x1.ccb9100ed3f46p-7, -0x1.97fc705cddfcbp-9, -0x1.c47f31cf82134p-8, 0x1.e3ed1451ce65fp-10, -0x1.0ea065aca8481p-5, 0x1.9e136e624d779p-4, 0x1.ca3f03d1a7bf1p-6, 0x0.0p+0, 0x1.f28ff7ab01f51p-5, -0x1.4f985759faa6bp-5, 0x1.febab3bf780e6p-8, -0x1.867878bbe9b3fp-5, 0x1.324525bdf062dp-6, 0x1.48f55fb340a97p-5, 0x1.8eb394c6fe292p-1, 0x1.2b4c579a5ed6bp-8, -0x1.03a6485498e34p-7, -0x1.a28cd36cc134dp-4, 0x1.4385648b61b9ap-5, 0x0.0p+0, 0x1.05a79a6443aa9p-5, -0x1.6455c63bcc4b3p-5, -0x1.3fd0ebeec54fcp-4, 0x1.a7f800a0db886p-8, -0x1.07d9351b0ec96p-4, 0x0.0p+0, -0x1.93a8349599311p-15, -0x1.a9acc03c069e3p-9, 0x0.0p+0, 0x0.0p+0, 0x1.709380d729d08p-6, 0x1.2a3547bc1172dp-5, 0x1.6c30c46de0ffdp-7, 0x1.a8917ca39e153p-7, -0x1.4f71975535b79p-6, -0x1.0abe017bf162ep-7, 0x1.8be07cd2ca2fcp-5, 0x1.535d4ddfedc60p-5, 0x1.cc94ab761021fp-6, -0x1.97511f5464341p-5, 0x0.0p+0, 0x0.0p+0, -0x1.ad858df2a73a8p-6, -0x1.d8ab9bbdaf405p-5, -0x1.c1183457ba8d7p-5, 0x0.0p+0, -0x1.a2296fd5aa180p-5, 0x1.2349b9bb4558ap-4, -0x1.96e4a3c1c5be2p-7, 0x1.bb2bc2f64bf90p-6, 0x1.6ca2f8800a51ep-5, 0x0.0p+0, -0x1.3b549bec97e48p-4, -0x1.b1a91ebe326edp-8, 0x1.b51be737563a1p-5, -0x1.fb8c984075403p-6, 0x1.cca11fc9b6d01p-7, 0x1.036f191637933p-8, -0x1.4b352113b584ap-7, 0x0.0p+0, -0x1.23124b6d5d267p-5, -0x1.d32a4d1f07e85p-5, 0x1.2b4c579a5ed6ap-8, 0x1.8eb394c6fe291p-1, 0x1.19c46416d35b1p-4, 0x1.bd15bbf562067p-7, -0x1.97147a35e2626p-10, 0x1.d36636065cda8p-5, 0x1.2a48036c0fa71p-6, 0x1.07254a96b76a2p-3, 0x1.585c2f9229148p-7, -0x1.54ad3761dd2dfp-10, 0x0.0p+0, 0x1.6ea96c1924d72p-4, 0x1.5b1dd5b36b73fp-6, 0x1.471e563a59a4ep-7, 0x0.0p+0, 0x0.0p+0, -0x1.089ed05dbe6f5p-3, -0x1.09b3f56128de4p-6, -0x1.83a9d6be335acp-5, 0x0.0p+0, 0x1.1d07afb44df7ap-4, 0x1.6e20c69f6428ep-6, 0x0.0p+0, -0x1.2a2fb50d6db52p-4, -0x1.381a256f0f20bp-5, 0x0.0p+0, -0x1.9025c2b863151p-5, -0x1.4b80aa0ef6da5p-6, -0x1.842e7096d7a60p-7, 0x0.0p+0, 0x1.7a0a327d5b8eep-5, 0x1.0188a5a28936ap-9, -0x1.f6a07e4ad9140p-8, -0x1.6fcdacab3b6c8p-4, 0x1.2904a4cf643bfp-5, 0x1.3313a46947ff3p-7, -0x1.7f313a930cd25p-6, 0x1.431855beee662p-6, 0x1.3fd7321aef3eap-4, 0x1.bdf6b74958e4fp-5, 0x1.5211430a5249bp-4, 0x1.d0d956a25b80cp-7, -0x1.55d1db86428ddp-4, -0x1.fb287ddb43171p-8, 0x1.119631c990250p-7, 0x0.0p+0, -0x1.03a6485498e34p-7, 0x1.19c46416d35b1p-4, 0x1.8eb394c6fe292p-1, 0x0.0p+0, 0x1.91fe4f3d0534bp-5, 0x0.0p+0, 0x1.cfa0dfcc56f9ep-6, -0x1.d0f5017962bc3p-6, 0x0.0p+0, 0x1.4f8cd476aa095p-7, 0x0.0p+0, 0x0.0p+0, -0x1.c32b8e91ba48ap-5, 0x0.0p+0, -0x1.15248834e5ef6p-6, 0x0.0p+0, 0x1.4d324bf95d8fcp-5, 0x0.0p+0, -0x1.739fc6c86a147p-4, 0x1.f518544925a3ap-5, 0x1.f6f0f9ea0299cp-5, 0x0.0p+0, -0x1.0c62b7d047d6dp-7, 0x0.0p+0, -0x1.1b56824fbdf97p-4, -0x1.61c463500d512p-4, 0x1.4a3853db6b794p-3, 0x0.0p+0, 0x1.600d9c09eefe0p-5, -0x1.ab1bb2a03c66ep-11, -0x1.7cb5dd267ba0dp-7, -0x1.024c72273e345p-7, 0x1.364153e8baf72p-7, -0x1.e46c61bb714c7p-5, -0x1.1a3d22869ca96p-5, 0x0.0p+0, -0x1.324224d75e9f7p-8, 0x1.0dfa6bf6c943bp-5, 0x0.0p+0, 0x1.9ebeb52e0a75bp-5, -0x1.a28cd36cc134dp-4, 0x1.bd15bbf562067p-7, 0x0.0p+0, 0x1.8eb394c6fe292p-1, 0x0.0p+0, 0x0.0p+0, -0x1.20d9bc31a22fep-5, 0x0.0p+0, -0x1.018d900fc49d7p-4, 0x0.0p+0, 0x1.d7b6251ae9ddap-11, -0x1.547d325a69de8p-5, 0x1.33555fcb62f82p-5, 0x1.68c94f3e4c8a7p-5, -0x1.4c8fc2ed31efap-6, 0x1.a54314a7dd554p-8, -0x1.e31deb4f18adep-6, 0x1.58508637a4e4fp-9, 0x1.5a650de5e0dc7p-7, -0x1.b4f1a932d62b5p-5, 0x0.0p+0, 0x0.0p+0, -0x1.40d09f66df232p-5, 0x1.36a77d202d111p-7, 0x1.bf09c75d01b94p-5, -0x1.12c4373248685p-5, 0x1.6bc85c58ad86ep-6, 0x0.0p+0, -0x1.780a570f69d7cp-5, -0x1.ec5b354a69544p-5, 0x1.f0c80ccfd8afbp-9, -0x1.2986849220aadp-7, -0x1.50cbefc6ca525p-7, -0x1.c643e8cc2059bp-5, -0x1.438ddf2af44f1p-6, -0x1.471fef19e9dfep-4, 0x1.99ab6c172a0b6p-4, -0x1.59a3a71eed4b6p-6, -0x1.5504125ef19bap-5, -0x1.33de9b5c585c0p-9, -0x1.86725532d364ep-5, 0x0.0p+0, -0x1.8e90e959ee251p-4, 0x1.a1dc97d6ed389p-5, -0x1.a510100fb784cp-6, 0x1.10353d9286bdfp-4, -0x1.4e041c4dc1436p-5, -0x1.4ee56e7529d80p-6, 0x1.4385648b61b9bp-5, -0x1.97147a35e262bp-10, 0x1.91fe4f3d0534bp-5, 0x0.0p+0, 0x1.8eb394c6fe291p-1, -0x1.f9b5a31a34c55p-8, 0x1.efc68014e84efp-8, -0x1.f89e4892ed88cp-6, -0x1.586d0af9dbaeep-7, -0x1.766230edd1bbcp-6, 0x1.de10ebafb0f97p-6, -0x1.36d0bff9e78c5p-6, -0x1.c4f2948f15bc4p-6, 0x1.0bb98b9fc7dcbp-5, 0x0.0p+0, 0x1.d87971d1cff04p-5, 0x1.bc8a845e1c2f2p-7, 0x0.0p+0, -0x1.2a631ccc852f5p-7, 0x1.702b71600c254p-5, 0x1.172ae74ca11d9p-3, 0x0.0p+0, 0x1.f6b8b194e37c7p-6, 0x1.6f4ffd6129c97p-4, 0x0.0p+0, 0x0.0p+0, 0x1.68e1d18ec930bp-6, -0x1.58cae7defdc2ep-6, 0x0.0p+0, 0x1.18e14bca372c2p-7, 0x0.0p+0, 0x0.0p+0, 0x1.3e75dee4e6eb0p-5, -0x1.f2bb68453785ep-6, 0x1.53d37aeae8ff1p-3, 0x1.d689e7c47ef72p-5, 0x0.0p+0, 0x0.0p+0, 0x1.3967865446a1ap-5, 0x0.0p+0, 0x1.a7d4e81806711p-9, 0x1.15d669c47e9dcp-8, 0x0.0p+0, 0x1.d36636065cda7p-5, 0x0.0p+0, 0x0.0p+0, -0x1.f9b5a31a34c52p-8, 0x1.8eb394c6fe291p-1, -0x1.5aa8b56e97039p-4, -0x1.5040a82e3ad43p-4, 0x1.035f528ef96cep-6, -0x1.4f04180bb19a6p-3, -0x1.efe206042bcaep-4, 0x0.0p+0, 0x1.98a7ad3c6df8cp-7, -0x1.0fed11cbfc279p-5, 0x0.0p+0, 0x0.0p+0, 0x1.099ff46971491p-4, -0x1.14efce2680e28p-4, 0x1.9c9cc8c3d7d45p-7, 0x0.0p+0, -0x1.804ad1377ad8bp-4, 0x1.a892ff64cc260p-4, 0x1.58da9430746fbp-4, 0x1.17334e53d27dcp-6, 0x1.243aeb632dbfap-7, 0x1.70d58377f8295p-6, 0x0.0p+0, 0x1.35bd97f0345b0p-5, 0x1.a51fb22fdaa60p-5, 0x0.0p+0, 0x1.1eb96ce8a9eb6p-6, -0x1.0392fd1b5cdd1p-9, 0x1.250d35c0f5c62p-6, 0x1.89d161b231e82p-5, 0x0.0p+0, 0x1.acf30d8c042d2p-6, 0x0.0p+0, -0x1.cb0ffbe429682p-5, -0x1.5c2b3009edf01p-5, 0x0.0p+0, 0x1.e842a2741725ap-5, 0x1.22a06a2ebbff9p-5, 0x0.0p+0, -0x1.a1918fbaca882p-7, 0x1.397b32b40739bp-7, 0x0.0p+0, 0x1.0f8b32ceb94e2p-7, -0x1.ef45b0090af2ap-5, 0x1.05a79a6443aa8p-5, 0x1.2a48036c0fa72p-6, 0x1.cfa0dfcc56f9fp-6, 0x0.0p+0, 0x1.efc68014e84f0p-8, -0x1.5aa8b56e9703ap-4, 0x1.8eb394c6fe292p-1, 0x1.5d63eaefa4683p-4, 0x1.4066a16cd9544p-7, -0x1.9dd7ccfdeb5bap-4, -0x1.3247c7629bbecp-4, 0x1.0eab0281ea4e1p-5, 0x0.0p+0, 0x1.728dd578de6ccp-4, -0x1.2c9447d1898f9p-6, 0x1.447d8c8d0abb3p-5, -0x1.76cd918915b38p-6, 0x0.0p+0, 0x1.15572d72f324ap-4, -0x1.89b837379d307p-7, 0x0.0p+0, -0x1.180022baf89afp-6, -0x1.257cd3f745c02p-6, -0x1.b02aee6c65aadp-7, 0x0.0p+0, 0x1.f9634e31cde85p-6, -0x1.ed6fb29dbebe8p-6, 0x1.0139bd078d044p-4, -0x1.11bdc59a194d5p-8, -0x1.cd710de1bcad6p-6, 0x1.2391c89e9661dp-4, -0x1.ae2dbe30a839dp-6, 0x1.1bb65112e8dcbp-6, 0x1.7e714c1b51764p-6, 0x1.e7ddb4cc59a54p-5, -0x1.f837004760cf9p-6, 0x1.343f1fba51c91p-4, 0x1.2e604f0b6340ep-7, -0x1.65972dbcb32f7p-5, 0x1.a2c3e6f2f711cp-7, -0x1.9db1a3f8e630fp-6, 0x0.0p+0, 0x1.ac7db4d68643cp-9, 0x1.d6d1291485acap-4, -0x1.6455c63bcc4b3p-5, 0x1.07254a96b76a2p-3, -0x1.d0f5017962bc2p-6, 0x0.0p+0, -0x1.f89e4892ed88dp-6, -0x1.5040a82e3ad44p-4, 0x1.5d63eaefa4683p-4, 0x1.8eb394c6fe292p-1, 0x0.0p+0, 0x1.db1560d6c30a4p-10, 0x0.0p+0, -0x1.ff5212e3faaf3p-4, 0x0.0p+0, 0x1.d3713ffb9fa38p-7, 0x1.f9d3db13e1031p-4, 0x0.0p+0, 0x1.a17dab5200624p-9, 0x1.c4fcb9f1a1845p-7, 0x0.0p+0, -0x1.33803f116022dp-5, 0x1.01ca471bb75e1p-8, 0x0.0p+0, -0x1.02058dde10ea0p-6, -0x1.81631b00cf9f2p-4, 0x1.9dadd1e4be53dp-7, -0x1.eb6e72e9e3273p-4, 0x0.0p+0, -0x1.248923bd24b49p-4, 0x1.b1f878bbf3056p-9, 0x0.0p+0, -0x1.011d0d43a3f4ap-5, -0x1.be7f97b32d248p-5, -0x1.a562fd2f8c7cbp-5, -0x1.c1542422f0787p-4, 0x1.696565a931c1cp-7, -0x1.833eb10321990p-5, 0x1.92bc574cc2e5ap-5, -0x1.6417911eecbcdp-5, -0x1.750b1019d94a9p-7, 0x1.2b1b0da010996p-4, 0x1.884e6d875a444p-7, -0x1.2604b59435702p-7, -0x1.06ba979c2b79ap-4, 0x1.4b785eae5423bp-7, 0x1.6fe8b8cb55f05p-5, 0x1.3d351b8a1f4e8p-4, 0x1.91b4266e9111bp-8, -0x1.324a164ffc0e4p-5, 0x0.0p+0, 0x1.379dbc91248c2p-8, -0x1.3fd0ebeec54fcp-4, 0x1.585c2f9229148p-7, 0x0.0p+0, -0x1.20d9bc31a22fep-5, -0x1.586d0af9dbaeep-7, 0x1.035f528ef96cep-6, 0x1.4066a16cd9545p-7, 0x0.0p+0, 0x1.8eb394c6fe292p-1, 0x1.614cbff025269p-8, -0x1.0828ef5ada8b6p-7, 0x0.0p+0, 0x1.26cded106dde3p-5, -0x1.5ce1eb8b6c1b9p-6, 0x1.fa60e255df895p-8, 0x0.0p+0, 0x1.341fcbc77f7e1p-8, -0x1.b62122448a19bp-9, 0x1.a23d00eaa59ecp-12, 0x1.d1b30e38ed413p-8, 0x0.0p+0, -0x1.2aea845854b8fp-6, 0x1.4d457c3fdfc4cp-3, 0x1.98bd40f2075cep-7, 0x1.e411fbf250d5fp-13, -0x1.ee8fc41688931p-6, 0x0.0p+0, 0x1.05f1dc267a952p-4, 0x0.0p+0, 0x0.0p+0, 0x1.151822ddf6570p-5, -0x1.4389b29fd302cp-5, 0x1.9a5b7121d7e56p-8, -0x1.c430ce71aa5a4p-6, -0x1.e03823da02a42p-10, 0x0.0p+0, 0x1.5a35f13a62fcdp-6, 0x0.0p+0, 0x1.58bc85c8d979ap-5, 0x1.caa740ed5ff57p-6, 0x1.c36dc0204e3f6p-4, 0x1.17e49063a4282p-4, 0x1.267a7ef107333p-10, -0x1.6184e7d1531fdp-5, 0x1.7ae8d08fd4f00p-11, 0x1.21c5e90aff452p-6, 0x0.0p+0, 0x0.0p+0, -0x1.8bcc7c80df51fp-6, -0x1.0c3f5d475f56fp-9, 0x1.a7f800a0db887p-8, -0x1.54ad3761dd2dep-10, 0x1.4f8cd476aa095p-7, 0x0.0p+0, -0x1.766230edd1bbcp-6, -0x1.4f04180bb19a7p-3, -0x1.9dd7ccfdeb5bap-4, 0x1.db1560d6c30a4p-10, 0x1.614cbff02526ep-8, 0x1.8eb394c6fe292p-1, -0x1.27fe58218c203p-3, 0x1.a5138c42092ccp-9, -0x1.13f23a9eddc09p-5, 0x1.ac2e2839fbae8p-5, 0x1.c15e1874d1a70p-8, 0x0.0p+0, 0x1.c45ebc14a6c77p-7, 0x0.0p+0, -0x1.0c9a7a021f16fp-6, 0x1.a6171908d47c1p-7, 0x1.7dcfbae81a0bep-6, 0x0.0p+0, 0x1.ed4cf6be03637p-4, 0x0.0p+0, -0x1.4c921488a40f3p-5, -0x1.a552c46f7f37cp-8, 0x1.4fde7679246d2p-4, 0x0.0p+0, 0x1.15b703a1f2897p-4, -0x1.8846ce9454eebp-6, 0x0.0p+0, -0x1.792f7303c2a63p-7, 0x1.dd966b70ffc01p-6, 0x1.5332639c1f980p-6, 0x1.0080b5cb5e287p-5, 0x1.9ad22c55dfb91p-5, -0x1.4c64cc357162ap-7, 0x1.11e38336621ddp-8, 0x1.26ddf66262ac8p-3, 0x1.9fbb0aa35a60bp-5, 0x0.0p+0, 0x0.0p+0, 0x1.c06c5ee297806p-7, -0x1.c091d8eb1188ep-9, 0x1.89f746cd1397fp-7, -0x1.fa6109ca22718p-4, 0x1.ebe37543cf355p-6, 0x0.0p+0, -0x1.07d9351b0ec96p-4, -0x1.018d900fc49d7p-4, 0x1.de10ebafb0f97p-6, -0x1.efe206042bcafp-4, -0x1.3247c7629bbecp-4, 0x0.0p+0, -0x1.0828ef5ada8b6p-7, -0x1.27fe58218c202p-3, 0x1.8eb394c6fe292p-1, 0x0.0p+0, 0x1.1a6d6fdb2fc16p-9, 0x0.0p+0, -0x1.c223b6b7769e3p-6, 0x0.0p+0, -0x1.047f1455b2eacp-6, 0x1.c7f7f9b6925f8p-4, 0x0.0p+0, 0x0.0p+0, 0x1.026c8f44a55e0p-3, 0x0.0p+0, 0x1.f6eb82724d485p-6, 0x0.0p+0, 0x1.2616e4210afa3p-5, 0x1.0c009691fa879p-6, -0x1.3cf09dc368ecdp-4, 0x0.0p+0, -0x1.1cab435fe5325p-5, 0x1.20b54967bca8ap-3, 0x0.0p+0, 0x1.6ea96c1924d72p-4, 0x0.0p+0, 0x0.0p+0, -0x1.36d0bff9e78c5p-6, -0x1.ff5212e3faaf3p-4, 0x0.0p+0, 0x1.a5138c42092ccp-9, 0x0.0p+0, 0x1.8eb394c6fe292p-1, 0x0.0p+0, -0x1.a499e1c370503p-8, -0x1.2161ad501e2e1p-7, 0x0.0p+0, -0x1.997a5ff215bb9p-6, -0x1.23a1c4c1c07a9p-4, -0x1.356b178473054p-4, 0x1.3b90849dbd4a6p-6, 0x1.1f20bcc59060ep-4, -0x1.02e77be8bdc72p-5, -0x1.2a2fc513818e5p-5, 0x1.33a53393f6433p-5, -0x1.cf11c6fe22dc2p-5, 0x1.89f5029cb0c50p-5, 0x1.63993e4df2891p-7, -0x1.8481251bef756p-7, 0x1.1d574d2669564p-10, 0x0.0p+0, 0x1.1204196c588d8p-4, 0x1.5e2c6bb647d31p-8, 0x0.0p+0, 0x1.cdaf53e4f96e1p-6, 0x0.0p+0, 0x1.095cc874bfdd7p-6, 0x1.b40a6cff9e4aap-5, 0x1.306ec8599be68p-7, 0x1.21e6a62351108p-4, 0x1.576dc577a9340p-4, -0x1.ee867e6ecfa2bp-7, 0x1.0cf77fa5fd1cap-6, 0x1.275f3b06e88bep-7, -0x1.0a15366a2af80p-3, 0x1.0a776f7b6f829p-7, 0x1.e8f43d52e99a9p-6, -0x1.8a565702c8513p-4, -0x1.a63197e481224p-8, 0x1.034eb5554beffp-5, -0x1.2f3660dbf6fb6p-7, -0x1.93a8349599312p-15, 0x1.5b1dd5b36b73fp-6, -0x1.c32b8e91ba48bp-5, 0x1.d7b6251ae9ddap-11, -0x1.c4f2948f15bc5p-6, 0x1.98a7ad3c6df8cp-7, 0x0.0p+0, 0x0.0p+0, 0x1.26cded106dde3p-5, -0x1.13f23a9eddc09p-5, 0x1.1a6d6fdb2fc16p-9, 0x0.0p+0, 0x1.8eb394c6fe292p-1};
+    float64x2_t a[25], xj;
+    fom_int r;
+    const fom_float *t = tab;
+    const float64x2_t lam = vdupq_n_f64(act->lambd);
+    const float64x2_t nlam = vnegq_f64(lam);
+    const float64x2_t z = vdupq_n_f64(0.0);
+    for (r = 0; r < 25; ++r) a[r] = vld1q_f64(bias + 2 * r);
+    xj = vld1q_dup_f64(state + 0);
+    a[0] = vfmaq_f64(a[0], vld1q_f64(t), xj); t += 2;
+    a[1] = vfmaq_f64(a[1], vld1q_f64(t), xj); t += 2;
+    a[2] = vfmaq_f64(a[2], vld1q_f64(t), xj); t += 2;
+    a[3] = vfmaq_f64(a[3], vld1q_f64(t), xj); t += 2;
+    a[4] = vfmaq_f64(a[4], vld1q_f64(t), xj); t += 2;
+    a[5] = vfmaq_f64(a[5], vld1q_f64(t), xj); t += 2;
+    a[6] = vfmaq_f64(a[6], vld1q_f64(t), xj); t += 2;
+    a[7] = vfmaq_f64(a[7], vld1q_f64(t), xj); t += 2;
+    a[8] = vfmaq_f64(a[8], vld1q_f64(t), xj); t += 2;
+    a[9] = vfmaq_f64(a[9], vld1q_f64(t), xj); t += 2;
+    a[10] = vfmaq_f64(a[10], vld1q_f64(t), xj); t += 2;
+    a[11] = vfmaq_f64(a[11], vld1q_f64(t), xj); t += 2;
+    a[12] = vfmaq_f64(a[12], vld1q_f64(t), xj); t += 2;
+    a[13] = vfmaq_f64(a[13], vld1q_f64(t), xj); t += 2;
+    a[14] = vfmaq_f64(a[14], vld1q_f64(t), xj); t += 2;
+    a[15] = vfmaq_f64(a[15], vld1q_f64(t), xj); t += 2;
+    a[16] = vfmaq_f64(a[16], vld1q_f64(t), xj); t += 2;
+    a[17] = vfmaq_f64(a[17], vld1q_f64(t), xj); t += 2;
+    a[18] = vfmaq_f64(a[18], vld1q_f64(t), xj); t += 2;
+    a[19] = vfmaq_f64(a[19], vld1q_f64(t), xj); t += 2;
+    a[20] = vfmaq_f64(a[20], vld1q_f64(t), xj); t += 2;
+    a[21] = vfmaq_f64(a[21], vld1q_f64(t), xj); t += 2;
+    a[22] = vfmaq_f64(a[22], vld1q_f64(t), xj); t += 2;
+    a[23] = vfmaq_f64(a[23], vld1q_f64(t), xj); t += 2;
+    a[24] = vfmaq_f64(a[24], vld1q_f64(t), xj); t += 2;
+    xj = vld1q_dup_f64(state + 1);
+    a[0] = vfmaq_f64(a[0], vld1q_f64(t), xj); t += 2;
+    a[2] = vfmaq_f64(a[2], vld1q_f64(t), xj); t += 2;
+    a[3] = vfmaq_f64(a[3], vld1q_f64(t), xj); t += 2;
+    a[4] = vfmaq_f64(a[4], vld1q_f64(t), xj); t += 2;
+    a[5] = vfmaq_f64(a[5], vld1q_f64(t), xj); t += 2;
+    a[6] = vfmaq_f64(a[6], vld1q_f64(t), xj); t += 2;
+    a[7] = vfmaq_f64(a[7], vld1q_f64(t), xj); t += 2;
+    a[8] = vfmaq_f64(a[8], vld1q_f64(t), xj); t += 2;
+    a[9] = vfmaq_f64(a[9], vld1q_f64(t), xj); t += 2;
+    a[10] = vfmaq_f64(a[10], vld1q_f64(t), xj); t += 2;
+    a[11] = vfmaq_f64(a[11], vld1q_f64(t), xj); t += 2;
+    a[12] = vfmaq_f64(a[12], vld1q_f64(t), xj); t += 2;
+    a[13] = vfmaq_f64(a[13], vld1q_f64(t), xj); t += 2;
+    a[14] = vfmaq_f64(a[14], vld1q_f64(t), xj); t += 2;
+    a[16] = vfmaq_f64(a[16], vld1q_f64(t), xj); t += 2;
+    a[17] = vfmaq_f64(a[17], vld1q_f64(t), xj); t += 2;
+    a[18] = vfmaq_f64(a[18], vld1q_f64(t), xj); t += 2;
+    a[20] = vfmaq_f64(a[20], vld1q_f64(t), xj); t += 2;
+    a[22] = vfmaq_f64(a[22], vld1q_f64(t), xj); t += 2;
+    a[23] = vfmaq_f64(a[23], vld1q_f64(t), xj); t += 2;
+    a[24] = vfmaq_f64(a[24], vld1q_f64(t), xj); t += 2;
+    xj = vld1q_dup_f64(state + 2);
+    a[0] = vfmaq_f64(a[0], vld1q_f64(t), xj); t += 2;
+    a[1] = vfmaq_f64(a[1], vld1q_f64(t), xj); t += 2;
+    a[9] = vfmaq_f64(a[9], vld1q_f64(t), xj); t += 2;
+    a[12] = vfmaq_f64(a[12], vld1q_f64(t), xj); t += 2;
+    a[13] = vfmaq_f64(a[13], vld1q_f64(t), xj); t += 2;
+    a[15] = vfmaq_f64(a[15], vld1q_f64(t), xj); t += 2;
+    a[16] = vfmaq_f64(a[16], vld1q_f64(t), xj); t += 2;
+    a[18] = vfmaq_f64(a[18], vld1q_f64(t), xj); t += 2;
+    a[20] = vfmaq_f64(a[20], vld1q_f64(t), xj); t += 2;
+    a[21] = vfmaq_f64(a[21], vld1q_f64(t), xj); t += 2;
+    a[22] = vfmaq_f64(a[22], vld1q_f64(t), xj); t += 2;
+    xj = vld1q_dup_f64(state + 3);
+    a[0] = vfmaq_f64(a[0], vld1q_f64(t), xj); t += 2;
+    a[1] = vfmaq_f64(a[1], vld1q_f64(t), xj); t += 2;
+    a[2] = vfmaq_f64(a[2], vld1q_f64(t), xj); t += 2;
+    a[3] = vfmaq_f64(a[3], vld1q_f64(t), xj); t += 2;
+    a[4] = vfmaq_f64(a[4], vld1q_f64(t), xj); t += 2;
+    a[5] = vfmaq_f64(a[5], vld1q_f64(t), xj); t += 2;
+    a[7] = vfmaq_f64(a[7], vld1q_f64(t), xj); t += 2;
+    a[8] = vfmaq_f64(a[8], vld1q_f64(t), xj); t += 2;
+    a[9] = vfmaq_f64(a[9], vld1q_f64(t), xj); t += 2;
+    a[10] = vfmaq_f64(a[10], vld1q_f64(t), xj); t += 2;
+    a[11] = vfmaq_f64(a[11], vld1q_f64(t), xj); t += 2;
+    a[12] = vfmaq_f64(a[12], vld1q_f64(t), xj); t += 2;
+    a[15] = vfmaq_f64(a[15], vld1q_f64(t), xj); t += 2;
+    a[16] = vfmaq_f64(a[16], vld1q_f64(t), xj); t += 2;
+    a[17] = vfmaq_f64(a[17], vld1q_f64(t), xj); t += 2;
+    a[18] = vfmaq_f64(a[18], vld1q_f64(t), xj); t += 2;
+    a[19] = vfmaq_f64(a[19], vld1q_f64(t), xj); t += 2;
+    a[20] = vfmaq_f64(a[20], vld1q_f64(t), xj); t += 2;
+    a[21] = vfmaq_f64(a[21], vld1q_f64(t), xj); t += 2;
+    a[22] = vfmaq_f64(a[22], vld1q_f64(t), xj); t += 2;
+    a[23] = vfmaq_f64(a[23], vld1q_f64(t), xj); t += 2;
+    a[24] = vfmaq_f64(a[24], vld1q_f64(t), xj); t += 2;
+    xj = vld1q_dup_f64(state + 4);
+    a[0] = vfmaq_f64(a[0], vld1q_f64(t), xj); t += 2;
+    a[2] = vfmaq_f64(a[2], vld1q_f64(t), xj); t += 2;
+    a[3] = vfmaq_f64(a[3], vld1q_f64(t), xj); t += 2;
+    a[4] = vfmaq_f64(a[4], vld1q_f64(t), xj); t += 2;
+    a[5] = vfmaq_f64(a[5], vld1q_f64(t), xj); t += 2;
+    a[6] = vfmaq_f64(a[6], vld1q_f64(t), xj); t += 2;
+    a[7] = vfmaq_f64(a[7], vld1q_f64(t), xj); t += 2;
+    a[8] = vfmaq_f64(a[8], vld1q_f64(t), xj); t += 2;
+    a[9] = vfmaq_f64(a[9], vld1q_f64(t), xj); t += 2;
+    a[10] = vfmaq_f64(a[10], vld1q_f64(t), xj); t += 2;
+    a[11] = vfmaq_f64(a[11], vld1q_f64(t), xj); t += 2;
+    a[12] = vfmaq_f64(a[12], vld1q_f64(t), xj); t += 2;
+    a[13] = vfmaq_f64(a[13], vld1q_f64(t), xj); t += 2;
+    a[14] = vfmaq_f64(a[14], vld1q_f64(t), xj); t += 2;
+    a[15] = vfmaq_f64(a[15], vld1q_f64(t), xj); t += 2;
+    a[16] = vfmaq_f64(a[16], vld1q_f64(t), xj); t += 2;
+    a[17] = vfmaq_f64(a[17], vld1q_f64(t), xj); t += 2;
+    a[18] = vfmaq_f64(a[18], vld1q_f64(t), xj); t += 2;
+    a[19] = vfmaq_f64(a[19], vld1q_f64(t), xj); t += 2;
+    a[20] = vfmaq_f64(a[20], vld1q_f64(t), xj); t += 2;
+    a[21] = vfmaq_f64(a[21], vld1q_f64(t), xj); t += 2;
+    a[22] = vfmaq_f64(a[22], vld1q_f64(t), xj); t += 2;
+    a[23] = vfmaq_f64(a[23], vld1q_f64(t), xj); t += 2;
+    a[24] = vfmaq_f64(a[24], vld1q_f64(t), xj); t += 2;
+    xj = vld1q_dup_f64(state + 5);
+    a[0] = vfmaq_f64(a[0], vld1q_f64(t), xj); t += 2;
+    a[1] = vfmaq_f64(a[1], vld1q_f64(t), xj); t += 2;
+    a[2] = vfmaq_f64(a[2], vld1q_f64(t), xj); t += 2;
+    a[3] = vfmaq_f64(a[3], vld1q_f64(t), xj); t += 2;
+    a[4] = vfmaq_f64(a[4], vld1q_f64(t), xj); t += 2;
+    a[5] = vfmaq_f64(a[5], vld1q_f64(t), xj); t += 2;
+    a[6] = vfmaq_f64(a[6], vld1q_f64(t), xj); t += 2;
+    a[7] = vfmaq_f64(a[7], vld1q_f64(t), xj); t += 2;
+    a[8] = vfmaq_f64(a[8], vld1q_f64(t), xj); t += 2;
+    a[9] = vfmaq_f64(a[9], vld1q_f64(t), xj); t += 2;
+    a[10] = vfmaq_f64(a[10], vld1q_f64(t), xj); t += 2;
+    a[11] = vfmaq_f64(a[11], vld1q_f64(t), xj); t += 2;
+    a[12] = vfmaq_f64(a[12], vld1q_f64(t), xj); t += 2;
+    a[13] = vfmaq_f64(a[13], vld1q_f64(t), xj); t += 2;
+    a[14] = vfmaq_f64(a[14], vld1q_f64(t), xj); t += 2;
+    a[15] = vfmaq_f64(a[15], vld1q_f64(t), xj); t += 2;
+    a[16] = vfmaq_f64(a[16], vld1q_f64(t), xj); t += 2;
+    a[17] = vfmaq_f64(a[17], vld1q_f64(t), xj); t += 2;
+    a[18] = vfmaq_f64(a[18], vld1q_f64(t), xj); t += 2;
+    a[19] = vfmaq_f64(a[19], vld1q_f64(t), xj); t += 2;
+    a[20] = vfmaq_f64(a[20], vld1q_f64(t), xj); t += 2;
+    a[21] = vfmaq_f64(a[21], vld1q_f64(t), xj); t += 2;
+    a[22] = vfmaq_f64(a[22], vld1q_f64(t), xj); t += 2;
+    a[23] = vfmaq_f64(a[23], vld1q_f64(t), xj); t += 2;
+    a[24] = vfmaq_f64(a[24], vld1q_f64(t), xj); t += 2;
+    xj = vld1q_dup_f64(state + 6);
+    a[0] = vfmaq_f64(a[0], vld1q_f64(t), xj); t += 2;
+    a[1] = vfmaq_f64(a[1], vld1q_f64(t), xj); t += 2;
+    a[2] = vfmaq_f64(a[2], vld1q_f64(t), xj); t += 2;
+    a[3] = vfmaq_f64(a[3], vld1q_f64(t), xj); t += 2;
+    a[4] = vfmaq_f64(a[4], vld1q_f64(t), xj); t += 2;
+    a[5] = vfmaq_f64(a[5], vld1q_f64(t), xj); t += 2;
+    a[7] = vfmaq_f64(a[7], vld1q_f64(t), xj); t += 2;
+    a[9] = vfmaq_f64(a[9], vld1q_f64(t), xj); t += 2;
+    a[10] = vfmaq_f64(a[10], vld1q_f64(t), xj); t += 2;
+    a[11] = vfmaq_f64(a[11], vld1q_f64(t), xj); t += 2;
+    a[12] = vfmaq_f64(a[12], vld1q_f64(t), xj); t += 2;
+    a[13] = vfmaq_f64(a[13], vld1q_f64(t), xj); t += 2;
+    a[14] = vfmaq_f64(a[14], vld1q_f64(t), xj); t += 2;
+    a[15] = vfmaq_f64(a[15], vld1q_f64(t), xj); t += 2;
+    a[16] = vfmaq_f64(a[16], vld1q_f64(t), xj); t += 2;
+    a[17] = vfmaq_f64(a[17], vld1q_f64(t), xj); t += 2;
+    a[18] = vfmaq_f64(a[18], vld1q_f64(t), xj); t += 2;
+    a[19] = vfmaq_f64(a[19], vld1q_f64(t), xj); t += 2;
+    a[20] = vfmaq_f64(a[20], vld1q_f64(t), xj); t += 2;
+    a[22] = vfmaq_f64(a[22], vld1q_f64(t), xj); t += 2;
+    a[23] = vfmaq_f64(a[23], vld1q_f64(t), xj); t += 2;
+    a[24] = vfmaq_f64(a[24], vld1q_f64(t), xj); t += 2;
+    xj = vld1q_dup_f64(state + 7);
+    a[0] = vfmaq_f64(a[0], vld1q_f64(t), xj); t += 2;
+    a[1] = vfmaq_f64(a[1], vld1q_f64(t), xj); t += 2;
+    a[2] = vfmaq_f64(a[2], vld1q_f64(t), xj); t += 2;
+    a[3] = vfmaq_f64(a[3], vld1q_f64(t), xj); t += 2;
+    a[4] = vfmaq_f64(a[4], vld1q_f64(t), xj); t += 2;
+    a[5] = vfmaq_f64(a[5], vld1q_f64(t), xj); t += 2;
+    a[6] = vfmaq_f64(a[6], vld1q_f64(t), xj); t += 2;
+    a[7] = vfmaq_f64(a[7], vld1q_f64(t), xj); t += 2;
+    a[8] = vfmaq_f64(a[8], vld1q_f64(t), xj); t += 2;
+    a[9] = vfmaq_f64(a[9], vld1q_f64(t), xj); t += 2;
+    a[10] = vfmaq_f64(a[10], vld1q_f64(t), xj); t += 2;
+    a[11] = vfmaq_f64(a[11], vld1q_f64(t), xj); t += 2;
+    a[12] = vfmaq_f64(a[12], vld1q_f64(t), xj); t += 2;
+    a[13] = vfmaq_f64(a[13], vld1q_f64(t), xj); t += 2;
+    a[14] = vfmaq_f64(a[14], vld1q_f64(t), xj); t += 2;
+    a[15] = vfmaq_f64(a[15], vld1q_f64(t), xj); t += 2;
+    a[16] = vfmaq_f64(a[16], vld1q_f64(t), xj); t += 2;
+    a[17] = vfmaq_f64(a[17], vld1q_f64(t), xj); t += 2;
+    a[18] = vfmaq_f64(a[18], vld1q_f64(t), xj); t += 2;
+    a[19] = vfmaq_f64(a[19], vld1q_f64(t), xj); t += 2;
+    a[20] = vfmaq_f64(a[20], vld1q_f64(t), xj); t += 2;
+    a[21] = vfmaq_f64(a[21], vld1q_f64(t), xj); t += 2;
+    a[22] = vfmaq_f64(a[22], vld1q_f64(t), xj); t += 2;
+    a[23] = vfmaq_f64(a[23], vld1q_f64(t), xj); t += 2;
+    a[24] = vfmaq_f64(a[24], vld1q_f64(t), xj); t += 2;
+    xj = vld1q_dup_f64(state + 8);
+    a[0] = vfmaq_f64(a[0], vld1q_f64(t), xj); t += 2;
+    a[1] = vfmaq_f64(a[1], vld1q_f64(t), xj); t += 2;
+    a[2] = vfmaq_f64(a[2], vld1q_f64(t), xj); t += 2;
+    a[3] = vfmaq_f64(a[3], vld1q_f64(t), xj); t += 2;
+    a[4] = vfmaq_f64(a[4], vld1q_f64(t), xj); t += 2;
+    a[6] = vfmaq_f64(a[6], vld1q_f64(t), xj); t += 2;
+    a[7] = vfmaq_f64(a[7], vld1q_f64(t), xj); t += 2;
+    a[8] = vfmaq_f64(a[8], vld1q_f64(t), xj); t += 2;
+    a[9] = vfmaq_f64(a[9], vld1q_f64(t), xj); t += 2;
+    a[10] = vfmaq_f64(a[10], vld1q_f64(t), xj); t += 2;
+    a[12] = vfmaq_f64(a[12], vld1q_f64(t), xj); t += 2;
+    a[13] = vfmaq_f64(a[13], vld1q_f64(t), xj); t += 2;
+    a[14] = vfmaq_f64(a[14], vld1q_f64(t), xj); t += 2;
+    a[15] = vfmaq_f64(a[15], vld1q_f64(t), xj); t += 2;
+    a[16] = vfmaq_f64(a[16], vld1q_f64(t), xj); t += 2;
+    a[17] = vfmaq_f64(a[17], vld1q_f64(t), xj); t += 2;
+    a[18] = vfmaq_f64(a[18], vld1q_f64(t), xj); t += 2;
+    a[19] = vfmaq_f64(a[19], vld1q_f64(t), xj); t += 2;
+    a[20] = vfmaq_f64(a[20], vld1q_f64(t), xj); t += 2;
+    a[21] = vfmaq_f64(a[21], vld1q_f64(t), xj); t += 2;
+    a[22] = vfmaq_f64(a[22], vld1q_f64(t), xj); t += 2;
+    a[23] = vfmaq_f64(a[23], vld1q_f64(t), xj); t += 2;
+    a[24] = vfmaq_f64(a[24], vld1q_f64(t), xj); t += 2;
+    xj = vld1q_dup_f64(state + 9);
+    a[0] = vfmaq_f64(a[0], vld1q_f64(t), xj); t += 2;
+    a[2] = vfmaq_f64(a[2], vld1q_f64(t), xj); t += 2;
+    a[3] = vfmaq_f64(a[3], vld1q_f64(t), xj); t += 2;
+    a[4] = vfmaq_f64(a[4], vld1q_f64(t), xj); t += 2;
+    a[7] = vfmaq_f64(a[7], vld1q_f64(t), xj); t += 2;
+    a[8] = vfmaq_f64(a[8], vld1q_f64(t), xj); t += 2;
+    a[9] = vfmaq_f64(a[9], vld1q_f64(t), xj); t += 2;
+    a[12] = vfmaq_f64(a[12], vld1q_f64(t), xj); t += 2;
+    a[13] = vfmaq_f64(a[13], vld1q_f64(t), xj); t += 2;
+    a[14] = vfmaq_f64(a[14], vld1q_f64(t), xj); t += 2;
+    a[15] = vfmaq_f64(a[15], vld1q_f64(t), xj); t += 2;
+    a[17] = vfmaq_f64(a[17], vld1q_f64(t), xj); t += 2;
+    a[18] = vfmaq_f64(a[18], vld1q_f64(t), xj); t += 2;
+    a[19] = vfmaq_f64(a[19], vld1q_f64(t), xj); t += 2;
+    a[20] = vfmaq_f64(a[20], vld1q_f64(t), xj); t += 2;
+    a[21] = vfmaq_f64(a[21], vld1q_f64(t), xj); t += 2;
+    a[22] = vfmaq_f64(a[22], vld1q_f64(t), xj); t += 2;
+    a[23] = vfmaq_f64(a[23], vld1q_f64(t), xj); t += 2;
+    a[24] = vfmaq_f64(a[24], vld1q_f64(t), xj); t += 2;
+    xj = vld1q_dup_f64(state + 10);
+    a[0] = vfmaq_f64(a[0], vld1q_f64(t), xj); t += 2;
+    a[1] = vfmaq_f64(a[1], vld1q_f64(t), xj); t += 2;
+    a[2] = vfmaq_f64(a[2], vld1q_f64(t), xj); t += 2;
+    a[3] = vfmaq_f64(a[3], vld1q_f64(t), xj); t += 2;
+    a[5] = vfmaq_f64(a[5], vld1q_f64(t), xj); t += 2;
+    a[7] = vfmaq_f64(a[7], vld1q_f64(t), xj); t += 2;
+    a[9] = vfmaq_f64(a[9], vld1q_f64(t), xj); t += 2;
+    a[11] = vfmaq_f64(a[11], vld1q_f64(t), xj); t += 2;
+    a[12] = vfmaq_f64(a[12], vld1q_f64(t), xj); t += 2;
+    a[13] = vfmaq_f64(a[13], vld1q_f64(t), xj); t += 2;
+    a[15] = vfmaq_f64(a[15], vld1q_f64(t), xj); t += 2;
+    a[16] = vfmaq_f64(a[16], vld1q_f64(t), xj); t += 2;
+    a[18] = vfmaq_f64(a[18], vld1q_f64(t), xj); t += 2;
+    a[19] = vfmaq_f64(a[19], vld1q_f64(t), xj); t += 2;
+    a[21] = vfmaq_f64(a[21], vld1q_f64(t), xj); t += 2;
+    a[22] = vfmaq_f64(a[22], vld1q_f64(t), xj); t += 2;
+    a[23] = vfmaq_f64(a[23], vld1q_f64(t), xj); t += 2;
+    a[24] = vfmaq_f64(a[24], vld1q_f64(t), xj); t += 2;
+    xj = vld1q_dup_f64(state + 11);
+    a[0] = vfmaq_f64(a[0], vld1q_f64(t), xj); t += 2;
+    a[1] = vfmaq_f64(a[1], vld1q_f64(t), xj); t += 2;
+    a[2] = vfmaq_f64(a[2], vld1q_f64(t), xj); t += 2;
+    a[3] = vfmaq_f64(a[3], vld1q_f64(t), xj); t += 2;
+    a[5] = vfmaq_f64(a[5], vld1q_f64(t), xj); t += 2;
+    a[6] = vfmaq_f64(a[6], vld1q_f64(t), xj); t += 2;
+    a[7] = vfmaq_f64(a[7], vld1q_f64(t), xj); t += 2;
+    a[8] = vfmaq_f64(a[8], vld1q_f64(t), xj); t += 2;
+    a[9] = vfmaq_f64(a[9], vld1q_f64(t), xj); t += 2;
+    a[10] = vfmaq_f64(a[10], vld1q_f64(t), xj); t += 2;
+    a[11] = vfmaq_f64(a[11], vld1q_f64(t), xj); t += 2;
+    a[12] = vfmaq_f64(a[12], vld1q_f64(t), xj); t += 2;
+    a[13] = vfmaq_f64(a[13], vld1q_f64(t), xj); t += 2;
+    a[14] = vfmaq_f64(a[14], vld1q_f64(t), xj); t += 2;
+    a[15] = vfmaq_f64(a[15], vld1q_f64(t), xj); t += 2;
+    a[16] = vfmaq_f64(a[16], vld1q_f64(t), xj); t += 2;
+    a[17] = vfmaq_f64(a[17], vld1q_f64(t), xj); t += 2;
+    a[18] = vfmaq_f64(a[18], vld1q_f64(t), xj); t += 2;
+    a[19] = vfmaq_f64(a[19], vld1q_f64(t), xj); t += 2;
+    a[20] = vfmaq_f64(a[20], vld1q_f64(t), xj); t += 2;
+    a[21] = vfmaq_f64(a[21], vld1q_f64(t), xj); t += 2;
+    a[22] = vfmaq_f64(a[22], vld1q_f64(t), xj); t += 2;
+    a[23] = vfmaq_f64(a[23], vld1q_f64(t), xj); t += 2;
+    a[24] = vfmaq_f64(a[24], vld1q_f64(t), xj); t += 2;
+    xj = vld1q_dup_f64(state + 12);
+    a[0] = vfmaq_f64(a[0], vld1q_f64(t), xj); t += 2;
+    a[2] = vfmaq_f64(a[2], vld1q_f64(t), xj); t += 2;
+    a[3] = vfmaq_f64(a[3], vld1q_f64(t), xj); t += 2;
+    a[4] = vfmaq_f64(a[4], vld1q_f64(t), xj); t += 2;
+    a[6] = vfmaq_f64(a[6], vld1q_f64(t), xj); t += 2;
+    a[7] = vfmaq_f64(a[7], vld1q_f64(t), xj); t += 2;
+    a[9] = vfmaq_f64(a[9], vld1q_f64(t), xj); t += 2;
+    a[10] = vfmaq_f64(a[10], vld1q_f64(t), xj); t += 2;
+    a[12] = vfmaq_f64(a[12], vld1q_f64(t), xj); t += 2;
+    a[18] = vfmaq_f64(a[18], vld1q_f64(t), xj); t += 2;
+    a[20] = vfmaq_f64(a[20], vld1q_f64(t), xj); t += 2;
+    a[21] = vfmaq_f64(a[21], vld1q_f64(t), xj); t += 2;
+    a[22] = vfmaq_f64(a[22], vld1q_f64(t), xj); t += 2;
+    a[23] = vfmaq_f64(a[23], vld1q_f64(t), xj); t += 2;
+    a[24] = vfmaq_f64(a[24], vld1q_f64(t), xj); t += 2;
+    xj = vld1q_dup_f64(state + 13);
+    a[0] = vfmaq_f64(a[0], vld1q_f64(t), xj); t += 2;
+    a[2] = vfmaq_f64(a[2], vld1q_f64(t), xj); t += 2;
+    a[5] = vfmaq_f64(a[5], vld1q_f64(t), xj); t += 2;
+    a[6] = vfmaq_f64(a[6], vld1q_f64(t), xj); t += 2;
+    a[7] = vfmaq_f64(a[7], vld1q_f64(t), xj); t += 2;
+    a[8] = vfmaq_f64(a[8], vld1q_f64(t), xj); t += 2;
+    a[9] = vfmaq_f64(a[9], vld1q_f64(t), xj); t += 2;
+    a[10] = vfmaq_f64(a[10], vld1q_f64(t), xj); t += 2;
+    a[11] = vfmaq_f64(a[11], vld1q_f64(t), xj); t += 2;
+    a[14] = vfmaq_f64(a[14], vld1q_f64(t), xj); t += 2;
+    a[15] = vfmaq_f64(a[15], vld1q_f64(t), xj); t += 2;
+    a[16] = vfmaq_f64(a[16], vld1q_f64(t), xj); t += 2;
+    a[17] = vfmaq_f64(a[17], vld1q_f64(t), xj); t += 2;
+    a[19] = vfmaq_f64(a[19], vld1q_f64(t), xj); t += 2;
+    a[20] = vfmaq_f64(a[20], vld1q_f64(t), xj); t += 2;
+    a[24] = vfmaq_f64(a[24], vld1q_f64(t), xj); t += 2;
+    xj = vld1q_dup_f64(state + 14);
+    a[0] = vfmaq_f64(a[0], vld1q_f64(t), xj); t += 2;
+    a[1] = vfmaq_f64(a[1], vld1q_f64(t), xj); t += 2;
+    a[2] = vfmaq_f64(a[2], vld1q_f64(t), xj); t += 2;
+    a[3] = vfmaq_f64(a[3], vld1q_f64(t), xj); t += 2;
+    a[4] = vfmaq_f64(a[4], vld1q_f64(t), xj); t += 2;
+    a[5] = vfmaq_f64(a[5], vld1q_f64(t), xj); t += 2;
+    a[7] = vfmaq_f64(a[7], vld1q_f64(t), xj); t += 2;
+    a[8] = vfmaq_f64(a[8], vld1q_f64(t), xj); t += 2;
+    a[9] = vfmaq_f64(a[9], vld1q_f64(t), xj); t += 2;
+    a[10] = vfmaq_f64(a[10], vld1q_f64(t), xj); t += 2;
+    a[11] = vfmaq_f64(a[11], vld1q_f64(t), xj); t += 2;
+    a[12] = vfmaq_f64(a[12], vld1q_f64(t), xj); t += 2;
+    a[13] = vfmaq_f64(a[13], vld1q_f64(t), xj); t += 2;
+    a[14] = vfmaq_f64(a[14], vld1q_f64(t), xj); t += 2;
+    a[15] = vfmaq_f64(a[15], vld1q_f64(t), xj); t += 2;
+    a[16] = vfmaq_f64(a[16], vld1q_f64(t), xj); t += 2;
+    a[17] = vfmaq_f64(a[17], vld1q_f64(t), xj); t += 2;
+    a[18] = vfmaq_f64(a[18], vld1q_f64(t), xj); t += 2;
+    a[19] = vfmaq_f64(a[19], vld1q_f64(t), xj); t += 2;
+    a[20] = vfmaq_f64(a[20], vld1q_f64(t), xj); t += 2;
+    a[21] = vfmaq_f64(a[21], vld1q_f64(t), xj); t += 2;
+    a[22] = vfmaq_f64(a[22], vld1q_f64(t), xj); t += 2;
+    a[23] = vfmaq_f64(a[23], vld1q_f64(t), xj); t += 2;
+    a[24] = vfmaq_f64(a[24], vld1q_f64(t), xj); t += 2;
+    xj = vld1q_dup_f64(state + 15);
+    a[0] = vfmaq_f64(a[0], vld1q_f64(t), xj); t += 2;
+    a[1] = vfmaq_f64(a[1], vld1q_f64(t), xj); t += 2;
+    a[2] = vfmaq_f64(a[2], vld1q_f64(t), xj); t += 2;
+    a[3] = vfmaq_f64(a[3], vld1q_f64(t), xj); t += 2;
+    a[4] = vfmaq_f64(a[4], vld1q_f64(t), xj); t += 2;
+    a[5] = vfmaq_f64(a[5], vld1q_f64(t), xj); t += 2;
+    a[6] = vfmaq_f64(a[6], vld1q_f64(t), xj); t += 2;
+    a[7] = vfmaq_f64(a[7], vld1q_f64(t), xj); t += 2;
+    a[8] = vfmaq_f64(a[8], vld1q_f64(t), xj); t += 2;
+    a[9] = vfmaq_f64(a[9], vld1q_f64(t), xj); t += 2;
+    a[10] = vfmaq_f64(a[10], vld1q_f64(t), xj); t += 2;
+    a[11] = vfmaq_f64(a[11], vld1q_f64(t), xj); t += 2;
+    a[12] = vfmaq_f64(a[12], vld1q_f64(t), xj); t += 2;
+    a[13] = vfmaq_f64(a[13], vld1q_f64(t), xj); t += 2;
+    a[14] = vfmaq_f64(a[14], vld1q_f64(t), xj); t += 2;
+    a[15] = vfmaq_f64(a[15], vld1q_f64(t), xj); t += 2;
+    a[16] = vfmaq_f64(a[16], vld1q_f64(t), xj); t += 2;
+    a[17] = vfmaq_f64(a[17], vld1q_f64(t), xj); t += 2;
+    a[18] = vfmaq_f64(a[18], vld1q_f64(t), xj); t += 2;
+    a[19] = vfmaq_f64(a[19], vld1q_f64(t), xj); t += 2;
+    a[20] = vfmaq_f64(a[20], vld1q_f64(t), xj); t += 2;
+    a[21] = vfmaq_f64(a[21], vld1q_f64(t), xj); t += 2;
+    a[22] = vfmaq_f64(a[22], vld1q_f64(t), xj); t += 2;
+    a[23] = vfmaq_f64(a[23], vld1q_f64(t), xj); t += 2;
+    a[24] = vfmaq_f64(a[24], vld1q_f64(t), xj); t += 2;
+    xj = vld1q_dup_f64(state + 16);
+    a[0] = vfmaq_f64(a[0], vld1q_f64(t), xj); t += 2;
+    a[5] = vfmaq_f64(a[5], vld1q_f64(t), xj); t += 2;
+    a[6] = vfmaq_f64(a[6], vld1q_f64(t), xj); t += 2;
+    a[8] = vfmaq_f64(a[8], vld1q_f64(t), xj); t += 2;
+    a[9] = vfmaq_f64(a[9], vld1q_f64(t), xj); t += 2;
+    a[10] = vfmaq_f64(a[10], vld1q_f64(t), xj); t += 2;
+    a[14] = vfmaq_f64(a[14], vld1q_f64(t), xj); t += 2;
+    a[15] = vfmaq_f64(a[15], vld1q_f64(t), xj); t += 2;
+    a[17] = vfmaq_f64(a[17], vld1q_f64(t), xj); t += 2;
+    xj = vld1q_dup_f64(state + 17);
+    a[0] = vfmaq_f64(a[0], vld1q_f64(t), xj); t += 2;
+    a[1] = vfmaq_f64(a[1], vld1q_f64(t), xj); t += 2;
+    a[2] = vfmaq_f64(a[2], vld1q_f64(t), xj); t += 2;
+    a[3] = vfmaq_f64(a[3], vld1q_f64(t), xj); t += 2;
+    a[4] = vfmaq_f64(a[4], vld1q_f64(t), xj); t += 2;
+    a[5] = vfmaq_f64(a[5], vld1q_f64(t), xj); t += 2;
+    a[7] = vfmaq_f64(a[7], vld1q_f64(t), xj); t += 2;
+    a[8] = vfmaq_f64(a[8], vld1q_f64(t), xj); t += 2;
+    a[9] = vfmaq_f64(a[9], vld1q_f64(t), xj); t += 2;
+    a[10] = vfmaq_f64(a[10], vld1q_f64(t), xj); t += 2;
+    a[11] = vfmaq_f64(a[11], vld1q_f64(t), xj); t += 2;
+    a[12] = vfmaq_f64(a[12], vld1q_f64(t), xj); t += 2;
+    a[13] = vfmaq_f64(a[13], vld1q_f64(t), xj); t += 2;
+    a[14] = vfmaq_f64(a[14], vld1q_f64(t), xj); t += 2;
+    a[15] = vfmaq_f64(a[15], vld1q_f64(t), xj); t += 2;
+    a[16] = vfmaq_f64(a[16], vld1q_f64(t), xj); t += 2;
+    a[17] = vfmaq_f64(a[17], vld1q_f64(t), xj); t += 2;
+    a[18] = vfmaq_f64(a[18], vld1q_f64(t), xj); t += 2;
+    a[19] = vfmaq_f64(a[19], vld1q_f64(t), xj); t += 2;
+    a[20] = vfmaq_f64(a[20], vld1q_f64(t), xj); t += 2;
+    a[21] = vfmaq_f64(a[21], vld1q_f64(t), xj); t += 2;
+    a[22] = vfmaq_f64(a[22], vld1q_f64(t), xj); t += 2;
+    a[23] = vfmaq_f64(a[23], vld1q_f64(t), xj); t += 2;
+    a[24] = vfmaq_f64(a[24], vld1q_f64(t), xj); t += 2;
+    xj = vld1q_dup_f64(state + 18);
+    a[0] = vfmaq_f64(a[0], vld1q_f64(t), xj); t += 2;
+    a[1] = vfmaq_f64(a[1], vld1q_f64(t), xj); t += 2;
+    a[2] = vfmaq_f64(a[2], vld1q_f64(t), xj); t += 2;
+    a[3] = vfmaq_f64(a[3], vld1q_f64(t), xj); t += 2;
+    a[4] = vfmaq_f64(a[4], vld1q_f64(t), xj); t += 2;
+    a[5] = vfmaq_f64(a[5], vld1q_f64(t), xj); t += 2;
+    a[6] = vfmaq_f64(a[6], vld1q_f64(t), xj); t += 2;
+    a[7] = vfmaq_f64(a[7], vld1q_f64(t), xj); t += 2;
+    a[8] = vfmaq_f64(a[8], vld1q_f64(t), xj); t += 2;
+    a[9] = vfmaq_f64(a[9], vld1q_f64(t), xj); t += 2;
+    a[10] = vfmaq_f64(a[10], vld1q_f64(t), xj); t += 2;
+    a[11] = vfmaq_f64(a[11], vld1q_f64(t), xj); t += 2;
+    a[12] = vfmaq_f64(a[12], vld1q_f64(t), xj); t += 2;
+    a[13] = vfmaq_f64(a[13], vld1q_f64(t), xj); t += 2;
+    a[14] = vfmaq_f64(a[14], vld1q_f64(t), xj); t += 2;
+    a[15] = vfmaq_f64(a[15], vld1q_f64(t), xj); t += 2;
+    a[16] = vfmaq_f64(a[16], vld1q_f64(t), xj); t += 2;
+    a[17] = vfmaq_f64(a[17], vld1q_f64(t), xj); t += 2;
+    a[18] = vfmaq_f64(a[18], vld1q_f64(t), xj); t += 2;
+    a[19] = vfmaq_f64(a[19], vld1q_f64(t), xj); t += 2;
+    a[20] = vfmaq_f64(a[20], vld1q_f64(t), xj); t += 2;
+    a[21] = vfmaq_f64(a[21], vld1q_f64(t), xj); t += 2;
+    a[22] = vfmaq_f64(a[22], vld1q_f64(t), xj); t += 2;
+    a[23] = vfmaq_f64(a[23], vld1q_f64(t), xj); t += 2;
+    a[24] = vfmaq_f64(a[24], vld1q_f64(t), xj); t += 2;
+    xj = vld1q_dup_f64(state + 19);
+    a[0] = vfmaq_f64(a[0], vld1q_f64(t), xj); t += 2;
+    a[1] = vfmaq_f64(a[1], vld1q_f64(t), xj); t += 2;
+    a[2] = vfmaq_f64(a[2], vld1q_f64(t), xj); t += 2;
+    a[3] = vfmaq_f64(a[3], vld1q_f64(t), xj); t += 2;
+    a[4] = vfmaq_f64(a[4], vld1q_f64(t), xj); t += 2;
+    a[5] = vfmaq_f64(a[5], vld1q_f64(t), xj); t += 2;
+    a[6] = vfmaq_f64(a[6], vld1q_f64(t), xj); t += 2;
+    a[7] = vfmaq_f64(a[7], vld1q_f64(t), xj); t += 2;
+    a[9] = vfmaq_f64(a[9], vld1q_f64(t), xj); t += 2;
+    a[10] = vfmaq_f64(a[10], vld1q_f64(t), xj); t += 2;
+    a[11] = vfmaq_f64(a[11], vld1q_f64(t), xj); t += 2;
+    a[12] = vfmaq_f64(a[12], vld1q_f64(t), xj); t += 2;
+    a[13] = vfmaq_f64(a[13], vld1q_f64(t), xj); t += 2;
+    a[15] = vfmaq_f64(a[15], vld1q_f64(t), xj); t += 2;
+    a[16] = vfmaq_f64(a[16], vld1q_f64(t), xj); t += 2;
+    a[17] = vfmaq_f64(a[17], vld1q_f64(t), xj); t += 2;
+    a[18] = vfmaq_f64(a[18], vld1q_f64(t), xj); t += 2;
+    a[19] = vfmaq_f64(a[19], vld1q_f64(t), xj); t += 2;
+    a[20] = vfmaq_f64(a[20], vld1q_f64(t), xj); t += 2;
+    a[21] = vfmaq_f64(a[21], vld1q_f64(t), xj); t += 2;
+    a[22] = vfmaq_f64(a[22], vld1q_f64(t), xj); t += 2;
+    a[23] = vfmaq_f64(a[23], vld1q_f64(t), xj); t += 2;
+    xj = vld1q_dup_f64(state + 20);
+    a[0] = vfmaq_f64(a[0], vld1q_f64(t), xj); t += 2;
+    a[1] = vfmaq_f64(a[1], vld1q_f64(t), xj); t += 2;
+    a[2] = vfmaq_f64(a[2], vld1q_f64(t), xj); t += 2;
+    a[3] = vfmaq_f64(a[3], vld1q_f64(t), xj); t += 2;
+    a[4] = vfmaq_f64(a[4], vld1q_f64(t), xj); t += 2;
+    a[5] = vfmaq_f64(a[5], vld1q_f64(t), xj); t += 2;
+    a[6] = vfmaq_f64(a[6], vld1q_f64(t), xj); t += 2;
+    a[7] = vfmaq_f64(a[7], vld1q_f64(t), xj); t += 2;
+    a[8] = vfmaq_f64(a[8], vld1q_f64(t), xj); t += 2;
+    a[9] = vfmaq_f64(a[9], vld1q_f64(t), xj); t += 2;
+    a[10] = vfmaq_f64(a[10], vld1q_f64(t), xj); t += 2;
+    a[12] = vfmaq_f64(a[12], vld1q_f64(t), xj); t += 2;
+    a[15] = vfmaq_f64(a[15], vld1q_f64(t), xj); t += 2;
+    a[16] = vfmaq_f64(a[16], vld1q_f64(t), xj); t += 2;
+    a[17] = vfmaq_f64(a[17], vld1q_f64(t), xj); t += 2;
+    a[18] = vfmaq_f64(a[18], vld1q_f64(t), xj); t += 2;
+    a[20] = vfmaq_f64(a[20], vld1q_f64(t), xj); t += 2;
+    a[21] = vfmaq_f64(a[21], vld1q_f64(t), xj); t += 2;
+    a[22] = vfmaq_f64(a[22], vld1q_f64(t), xj); t += 2;
+    a[23] = vfmaq_f64(a[23], vld1q_f64(t), xj); t += 2;
+    a[24] = vfmaq_f64(a[24], vld1q_f64(t), xj); t += 2;
+    xj = vld1q_dup_f64(state + 21);
+    a[0] = vfmaq_f64(a[0], vld1q_f64(t), xj); t += 2;
+    a[2] = vfmaq_f64(a[2], vld1q_f64(t), xj); t += 2;
+    a[5] = vfmaq_f64(a[5], vld1q_f64(t), xj); t += 2;
+    a[6] = vfmaq_f64(a[6], vld1q_f64(t), xj); t += 2;
+    a[7] = vfmaq_f64(a[7], vld1q_f64(t), xj); t += 2;
+    a[8] = vfmaq_f64(a[8], vld1q_f64(t), xj); t += 2;
+    a[9] = vfmaq_f64(a[9], vld1q_f64(t), xj); t += 2;
+    a[10] = vfmaq_f64(a[10], vld1q_f64(t), xj); t += 2;
+    a[11] = vfmaq_f64(a[11], vld1q_f64(t), xj); t += 2;
+    a[12] = vfmaq_f64(a[12], vld1q_f64(t), xj); t += 2;
+    a[13] = vfmaq_f64(a[13], vld1q_f64(t), xj); t += 2;
+    a[14] = vfmaq_f64(a[14], vld1q_f64(t), xj); t += 2;
+    a[15] = vfmaq_f64(a[15], vld1q_f64(t), xj); t += 2;
+    a[17] = vfmaq_f64(a[17], vld1q_f64(t), xj); t += 2;
+    a[18] = vfmaq_f64(a[18], vld1q_f64(t), xj); t += 2;
+    a[19] = vfmaq_f64(a[19], vld1q_f64(t), xj); t += 2;
+    a[20] = vfmaq_f64(a[20], vld1q_f64(t), xj); t += 2;
+    a[22] = vfmaq_f64(a[22], vld1q_f64(t), xj); t += 2;
+    a[23] = vfmaq_f64(a[23], vld1q_f64(t), xj); t += 2;
+    a[24] = vfmaq_f64(a[24], vld1q_f64(t), xj); t += 2;
+    xj = vld1q_dup_f64(state + 22);
+    a[0] = vfmaq_f64(a[0], vld1q_f64(t), xj); t += 2;
+    a[1] = vfmaq_f64(a[1], vld1q_f64(t), xj); t += 2;
+    a[2] = vfmaq_f64(a[2], vld1q_f64(t), xj); t += 2;
+    a[3] = vfmaq_f64(a[3], vld1q_f64(t), xj); t += 2;
+    a[5] = vfmaq_f64(a[5], vld1q_f64(t), xj); t += 2;
+    a[7] = vfmaq_f64(a[7], vld1q_f64(t), xj); t += 2;
+    a[8] = vfmaq_f64(a[8], vld1q_f64(t), xj); t += 2;
+    a[9] = vfmaq_f64(a[9], vld1q_f64(t), xj); t += 2;
+    a[11] = vfmaq_f64(a[11], vld1q_f64(t), xj); t += 2;
+    a[12] = vfmaq_f64(a[12], vld1q_f64(t), xj); t += 2;
+    a[13] = vfmaq_f64(a[13], vld1q_f64(t), xj); t += 2;
+    a[14] = vfmaq_f64(a[14], vld1q_f64(t), xj); t += 2;
+    a[15] = vfmaq_f64(a[15], vld1q_f64(t), xj); t += 2;
+    a[16] = vfmaq_f64(a[16], vld1q_f64(t), xj); t += 2;
+    a[17] = vfmaq_f64(a[17], vld1q_f64(t), xj); t += 2;
+    a[18] = vfmaq_f64(a[18], vld1q_f64(t), xj); t += 2;
+    a[19] = vfmaq_f64(a[19], vld1q_f64(t), xj); t += 2;
+    a[20] = vfmaq_f64(a[20], vld1q_f64(t), xj); t += 2;
+    a[21] = vfmaq_f64(a[21], vld1q_f64(t), xj); t += 2;
+    a[22] = vfmaq_f64(a[22], vld1q_f64(t), xj); t += 2;
+    a[23] = vfmaq_f64(a[23], vld1q_f64(t), xj); t += 2;
+    a[24] = vfmaq_f64(a[24], vld1q_f64(t), xj); t += 2;
+    xj = vld1q_dup_f64(state + 23);
+    a[0] = vfmaq_f64(a[0], vld1q_f64(t), xj); t += 2;
+    a[1] = vfmaq_f64(a[1], vld1q_f64(t), xj); t += 2;
+    a[2] = vfmaq_f64(a[2], vld1q_f64(t), xj); t += 2;
+    a[3] = vfmaq_f64(a[3], vld1q_f64(t), xj); t += 2;
+    a[5] = vfmaq_f64(a[5], vld1q_f64(t), xj); t += 2;
+    a[6] = vfmaq_f64(a[6], vld1q_f64(t), xj); t += 2;
+    a[7] = vfmaq_f64(a[7], vld1q_f64(t), xj); t += 2;
+    a[8] = vfmaq_f64(a[8], vld1q_f64(t), xj); t += 2;
+    a[9] = vfmaq_f64(a[9], vld1q_f64(t), xj); t += 2;
+    a[10] = vfmaq_f64(a[10], vld1q_f64(t), xj); t += 2;
+    a[11] = vfmaq_f64(a[11], vld1q_f64(t), xj); t += 2;
+    a[12] = vfmaq_f64(a[12], vld1q_f64(t), xj); t += 2;
+    a[13] = vfmaq_f64(a[13], vld1q_f64(t), xj); t += 2;
+    a[14] = vfmaq_f64(a[14], vld1q_f64(t), xj); t += 2;
+    a[15] = vfmaq_f64(a[15], vld1q_f64(t), xj); t += 2;
+    a[16] = vfmaq_f64(a[16], vld1q_f64(t), xj); t += 2;
+    a[17] = vfmaq_f64(a[17], vld1q_f64(t), xj); t += 2;
+    a[18] = vfmaq_f64(a[18], vld1q_f64(t), xj); t += 2;
+    a[19] = vfmaq_f64(a[19], vld1q_f64(t), xj); t += 2;
+    a[20] = vfmaq_f64(a[20], vld1q_f64(t), xj); t += 2;
+    a[22] = vfmaq_f64(a[22], vld1q_f64(t), xj); t += 2;
+    a[23] = vfmaq_f64(a[23], vld1q_f64(t), xj); t += 2;
+    a[24] = vfmaq_f64(a[24], vld1q_f64(t), xj); t += 2;
+    xj = vld1q_dup_f64(state + 24);
+    a[0] = vfmaq_f64(a[0], vld1q_f64(t), xj); t += 2;
+    a[1] = vfmaq_f64(a[1], vld1q_f64(t), xj); t += 2;
+    a[2] = vfmaq_f64(a[2], vld1q_f64(t), xj); t += 2;
+    a[3] = vfmaq_f64(a[3], vld1q_f64(t), xj); t += 2;
+    a[4] = vfmaq_f64(a[4], vld1q_f64(t), xj); t += 2;
+    a[5] = vfmaq_f64(a[5], vld1q_f64(t), xj); t += 2;
+    a[6] = vfmaq_f64(a[6], vld1q_f64(t), xj); t += 2;
+    a[7] = vfmaq_f64(a[7], vld1q_f64(t), xj); t += 2;
+    a[8] = vfmaq_f64(a[8], vld1q_f64(t), xj); t += 2;
+    a[9] = vfmaq_f64(a[9], vld1q_f64(t), xj); t += 2;
+    a[10] = vfmaq_f64(a[10], vld1q_f64(t), xj); t += 2;
+    a[11] = vfmaq_f64(a[11], vld1q_f64(t), xj); t += 2;
+    a[12] = vfmaq_f64(a[12], vld1q_f64(t), xj); t += 2;
+    a[13] = vfmaq_f64(a[13], vld1q_f64(t), xj); t += 2;
+    a[14] = vfmaq_f64(a[14], vld1q_f64(t), xj); t += 2;
+    a[15] = vfmaq_f64(a[15], vld1q_f64(t), xj); t += 2;
+    a[16] = vfmaq_f64(a[16], vld1q_f64(t), xj); t += 2;
+    a[17] = vfmaq_f64(a[17], vld1q_f64(t), xj); t += 2;
+    a[18] = vfmaq_f64(a[18], vld1q_f64(t), xj); t += 2;
+    a[19] = vfmaq_f64(a[19], vld1q_f64(t), xj); t += 2;
+    a[20] = vfmaq_f64(a[20], vld1q_f64(t), xj); t += 2;
+    a[21] = vfmaq_f64(a[21], vld1q_f64(t), xj); t += 2;
+    a[22] = vfmaq_f64(a[22], vld1q_f64(t), xj); t += 2;
+    a[23] = vfmaq_f64(a[23], vld1q_f64(t), xj); t += 2;
+    a[24] = vfmaq_f64(a[24], vld1q_f64(t), xj); t += 2;
+    xj = vld1q_dup_f64(state + 25);
+    a[0] = vfmaq_f64(a[0], vld1q_f64(t), xj); t += 2;
+    a[1] = vfmaq_f64(a[1], vld1q_f64(t), xj); t += 2;
+    a[2] = vfmaq_f64(a[2], vld1q_f64(t), xj); t += 2;
+    a[3] = vfmaq_f64(a[3], vld1q_f64(t), xj); t += 2;
+    a[4] = vfmaq_f64(a[4], vld1q_f64(t), xj); t += 2;
+    a[5] = vfmaq_f64(a[5], vld1q_f64(t), xj); t += 2;
+    a[7] = vfmaq_f64(a[7], vld1q_f64(t), xj); t += 2;
+    a[8] = vfmaq_f64(a[8], vld1q_f64(t), xj); t += 2;
+    a[9] = vfmaq_f64(a[9], vld1q_f64(t), xj); t += 2;
+    a[10] = vfmaq_f64(a[10], vld1q_f64(t), xj); t += 2;
+    a[11] = vfmaq_f64(a[11], vld1q_f64(t), xj); t += 2;
+    a[12] = vfmaq_f64(a[12], vld1q_f64(t), xj); t += 2;
+    a[13] = vfmaq_f64(a[13], vld1q_f64(t), xj); t += 2;
+    a[14] = vfmaq_f64(a[14], vld1q_f64(t), xj); t += 2;
+    a[15] = vfmaq_f64(a[15], vld1q_f64(t), xj); t += 2;
+    a[16] = vfmaq_f64(a[16], vld1q_f64(t), xj); t += 2;
+    a[17] = vfmaq_f64(a[17], vld1q_f64(t), xj); t += 2;
+    a[18] = vfmaq_f64(a[18], vld1q_f64(t), xj); t += 2;
+    a[19] = vfmaq_f64(a[19], vld1q_f64(t), xj); t += 2;
+    a[20] = vfmaq_f64(a[20], vld1q_f64(t), xj); t += 2;
+    a[21] = vfmaq_f64(a[21], vld1q_f64(t), xj); t += 2;
+    a[22] = vfmaq_f64(a[22], vld1q_f64(t), xj); t += 2;
+    a[23] = vfmaq_f64(a[23], vld1q_f64(t), xj); t += 2;
+    a[24] = vfmaq_f64(a[24], vld1q_f64(t), xj); t += 2;
+    xj = vld1q_dup_f64(state + 26);
+    a[0] = vfmaq_f64(a[0], vld1q_f64(t), xj); t += 2;
+    a[1] = vfmaq_f64(a[1], vld1q_f64(t), xj); t += 2;
+    a[2] = vfmaq_f64(a[2], vld1q_f64(t), xj); t += 2;
+    a[3] = vfmaq_f64(a[3], vld1q_f64(t), xj); t += 2;
+    a[5] = vfmaq_f64(a[5], vld1q_f64(t), xj); t += 2;
+    a[7] = vfmaq_f64(a[7], vld1q_f64(t), xj); t += 2;
+    a[8] = vfmaq_f64(a[8], vld1q_f64(t), xj); t += 2;
+    a[9] = vfmaq_f64(a[9], vld1q_f64(t), xj); t += 2;
+    a[11] = vfmaq_f64(a[11], vld1q_f64(t), xj); t += 2;
+    a[12] = vfmaq_f64(a[12], vld1q_f64(t), xj); t += 2;
+    a[13] = vfmaq_f64(a[13], vld1q_f64(t), xj); t += 2;
+    a[15] = vfmaq_f64(a[15], vld1q_f64(t), xj); t += 2;
+    a[16] = vfmaq_f64(a[16], vld1q_f64(t), xj); t += 2;
+    a[18] = vfmaq_f64(a[18], vld1q_f64(t), xj); t += 2;
+    a[20] = vfmaq_f64(a[20], vld1q_f64(t), xj); t += 2;
+    a[21] = vfmaq_f64(a[21], vld1q_f64(t), xj); t += 2;
+    a[22] = vfmaq_f64(a[22], vld1q_f64(t), xj); t += 2;
+    a[23] = vfmaq_f64(a[23], vld1q_f64(t), xj); t += 2;
+    a[24] = vfmaq_f64(a[24], vld1q_f64(t), xj); t += 2;
+    xj = vld1q_dup_f64(state + 27);
+    a[0] = vfmaq_f64(a[0], vld1q_f64(t), xj); t += 2;
+    a[2] = vfmaq_f64(a[2], vld1q_f64(t), xj); t += 2;
+    a[3] = vfmaq_f64(a[3], vld1q_f64(t), xj); t += 2;
+    a[4] = vfmaq_f64(a[4], vld1q_f64(t), xj); t += 2;
+    a[5] = vfmaq_f64(a[5], vld1q_f64(t), xj); t += 2;
+    a[7] = vfmaq_f64(a[7], vld1q_f64(t), xj); t += 2;
+    a[8] = vfmaq_f64(a[8], vld1q_f64(t), xj); t += 2;
+    a[9] = vfmaq_f64(a[9], vld1q_f64(t), xj); t += 2;
+    a[10] = vfmaq_f64(a[10], vld1q_f64(t), xj); t += 2;
+    a[11] = vfmaq_f64(a[11], vld1q_f64(t), xj); t += 2;
+    a[12] = vfmaq_f64(a[12], vld1q_f64(t), xj); t += 2;
+    a[13] = vfmaq_f64(a[13], vld1q_f64(t), xj); t += 2;
+    a[14] = vfmaq_f64(a[14], vld1q_f64(t), xj); t += 2;
+    a[15] = vfmaq_f64(a[15], vld1q_f64(t), xj); t += 2;
+    a[17] = vfmaq_f64(a[17], vld1q_f64(t), xj); t += 2;
+    a[18] = vfmaq_f64(a[18], vld1q_f64(t), xj); t += 2;
+    a[19] = vfmaq_f64(a[19], vld1q_f64(t), xj); t += 2;
+    a[20] = vfmaq_f64(a[20], vld1q_f64(t), xj); t += 2;
+    a[21] = vfmaq_f64(a[21], vld1q_f64(t), xj); t += 2;
+    a[22] = vfmaq_f64(a[22], vld1q_f64(t), xj); t += 2;
+    a[23] = vfmaq_f64(a[23], vld1q_f64(t), xj); t += 2;
+    a[24] = vfmaq_f64(a[24], vld1q_f64(t), xj); t += 2;
+    xj = vld1q_dup_f64(state + 28);
+    a[0] = vfmaq_f64(a[0], vld1q_f64(t), xj); t += 2;
+    a[2] = vfmaq_f64(a[2], vld1q_f64(t), xj); t += 2;
+    a[3] = vfmaq_f64(a[3], vld1q_f64(t), xj); t += 2;
+    a[4] = vfmaq_f64(a[4], vld1q_f64(t), xj); t += 2;
+    a[5] = vfmaq_f64(a[5], vld1q_f64(t), xj); t += 2;
+    a[6] = vfmaq_f64(a[6], vld1q_f64(t), xj); t += 2;
+    a[7] = vfmaq_f64(a[7], vld1q_f64(t), xj); t += 2;
+    a[8] = vfmaq_f64(a[8], vld1q_f64(t), xj); t += 2;
+    a[9] = vfmaq_f64(a[9], vld1q_f64(t), xj); t += 2;
+    a[10] = vfmaq_f64(a[10], vld1q_f64(t), xj); t += 2;
+    a[11] = vfmaq_f64(a[11], vld1q_f64(t), xj); t += 2;
+    a[12] = vfmaq_f64(a[12], vld1q_f64(t), xj); t += 2;
+    a[13] = vfmaq_f64(a[13], vld1q_f64(t), xj); t += 2;
+    a[14] = vfmaq_f64(a[14], vld1q_f64(t), xj); t += 2;
+    a[15] = vfmaq_f64(a[15], vld1q_f64(t), xj); t += 2;
+    a[16] = vfmaq_f64(a[16], vld1q_f64(t), xj); t += 2;
+    a[17] = vfmaq_f64(a[17], vld1q_f64(t), xj); t += 2;
+    a[18] = vfmaq_f64(a[18], vld1q_f64(t), xj); t += 2;
+    a[19] = vfmaq_f64(a[19], vld1q_f64(t), xj); t += 2;
+    a[20] = vfmaq_f64(a[20], vld1q_f64(t), xj); t += 2;
+    a[21] = vfmaq_f64(a[21], vld1q_f64(t), xj); t += 2;
+    a[22] = vfmaq_f64(a[22], vld1q_f64(t), xj); t += 2;
+    a[23] = vfmaq_f64(a[23], vld1q_f64(t), xj); t += 2;
+    a[24] = vfmaq_f64(a[24], vld1q_f64(t), xj); t += 2;
+    xj = vld1q_dup_f64(state + 29);
+    a[0] = vfmaq_f64(a[0], vld1q_f64(t), xj); t += 2;
+    a[2] = vfmaq_f64(a[2], vld1q_f64(t), xj); t += 2;
+    a[3] = vfmaq_f64(a[3], vld1q_f64(t), xj); t += 2;
+    a[4] = vfmaq_f64(a[4], vld1q_f64(t), xj); t += 2;
+    a[5] = vfmaq_f64(a[5], vld1q_f64(t), xj); t += 2;
+    a[6] = vfmaq_f64(a[6], vld1q_f64(t), xj); t += 2;
+    a[7] = vfmaq_f64(a[7], vld1q_f64(t), xj); t += 2;
+    a[8] = vfmaq_f64(a[8], vld1q_f64(t), xj); t += 2;
+    a[9] = vfmaq_f64(a[9], vld1q_f64(t), xj); t += 2;
+    a[10] = vfmaq_f64(a[10], vld1q_f64(t), xj); t += 2;
+    a[11] = vfmaq_f64(a[11], vld1q_f64(t), xj); t += 2;
+    a[12] = vfmaq_f64(a[12], vld1q_f64(t), xj); t += 2;
+    a[14] = vfmaq_f64(a[14], vld1q_f64(t), xj); t += 2;
+    a[15] = vfmaq_f64(a[15], vld1q_f64(t), xj); t += 2;
+    a[16] = vfmaq_f64(a[16], vld1q_f64(t), xj); t += 2;
+    a[17] = vfmaq_f64(a[17], vld1q_f64(t), xj); t += 2;
+    a[18] = vfmaq_f64(a[18], vld1q_f64(t), xj); t += 2;
+    a[19] = vfmaq_f64(a[19], vld1q_f64(t), xj); t += 2;
+    a[20] = vfmaq_f64(a[20], vld1q_f64(t), xj); t += 2;
+    a[22] = vfmaq_f64(a[22], vld1q_f64(t), xj); t += 2;
+    a[23] = vfmaq_f64(a[23], vld1q_f64(t), xj); t += 2;
+    a[24] = vfmaq_f64(a[24], vld1q_f64(t), xj); t += 2;
+    xj = vld1q_dup_f64(state + 30);
+    a[0] = vfmaq_f64(a[0], vld1q_f64(t), xj); t += 2;
+    a[1] = vfmaq_f64(a[1], vld1q_f64(t), xj); t += 2;
+    a[3] = vfmaq_f64(a[3], vld1q_f64(t), xj); t += 2;
+    a[5] = vfmaq_f64(a[5], vld1q_f64(t), xj); t += 2;
+    a[6] = vfmaq_f64(a[6], vld1q_f64(t), xj); t += 2;
+    a[8] = vfmaq_f64(a[8], vld1q_f64(t), xj); t += 2;
+    a[9] = vfmaq_f64(a[9], vld1q_f64(t), xj); t += 2;
+    a[10] = vfmaq_f64(a[10], vld1q_f64(t), xj); t += 2;
+    a[11] = vfmaq_f64(a[11], vld1q_f64(t), xj); t += 2;
+    a[12] = vfmaq_f64(a[12], vld1q_f64(t), xj); t += 2;
+    a[13] = vfmaq_f64(a[13], vld1q_f64(t), xj); t += 2;
+    a[14] = vfmaq_f64(a[14], vld1q_f64(t), xj); t += 2;
+    a[15] = vfmaq_f64(a[15], vld1q_f64(t), xj); t += 2;
+    a[17] = vfmaq_f64(a[17], vld1q_f64(t), xj); t += 2;
+    a[19] = vfmaq_f64(a[19], vld1q_f64(t), xj); t += 2;
+    a[21] = vfmaq_f64(a[21], vld1q_f64(t), xj); t += 2;
+    a[22] = vfmaq_f64(a[22], vld1q_f64(t), xj); t += 2;
+    a[23] = vfmaq_f64(a[23], vld1q_f64(t), xj); t += 2;
+    a[24] = vfmaq_f64(a[24], vld1q_f64(t), xj); t += 2;
+    xj = vld1q_dup_f64(state + 31);
+    a[0] = vfmaq_f64(a[0], vld1q_f64(t), xj); t += 2;
+    a[1] = vfmaq_f64(a[1], vld1q_f64(t), xj); t += 2;
+    a[2] = vfmaq_f64(a[2], vld1q_f64(t), xj); t += 2;
+    a[3] = vfmaq_f64(a[3], vld1q_f64(t), xj); t += 2;
+    a[4] = vfmaq_f64(a[4], vld1q_f64(t), xj); t += 2;
+    a[5] = vfmaq_f64(a[5], vld1q_f64(t), xj); t += 2;
+    a[7] = vfmaq_f64(a[7], vld1q_f64(t), xj); t += 2;
+    a[8] = vfmaq_f64(a[8], vld1q_f64(t), xj); t += 2;
+    a[9] = vfmaq_f64(a[9], vld1q_f64(t), xj); t += 2;
+    a[10] = vfmaq_f64(a[10], vld1q_f64(t), xj); t += 2;
+    a[11] = vfmaq_f64(a[11], vld1q_f64(t), xj); t += 2;
+    a[12] = vfmaq_f64(a[12], vld1q_f64(t), xj); t += 2;
+    a[13] = vfmaq_f64(a[13], vld1q_f64(t), xj); t += 2;
+    a[14] = vfmaq_f64(a[14], vld1q_f64(t), xj); t += 2;
+    a[15] = vfmaq_f64(a[15], vld1q_f64(t), xj); t += 2;
+    a[16] = vfmaq_f64(a[16], vld1q_f64(t), xj); t += 2;
+    a[17] = vfmaq_f64(a[17], vld1q_f64(t), xj); t += 2;
+    a[18] = vfmaq_f64(a[18], vld1q_f64(t), xj); t += 2;
+    a[19] = vfmaq_f64(a[19], vld1q_f64(t), xj); t += 2;
+    a[20] = vfmaq_f64(a[20], vld1q_f64(t), xj); t += 2;
+    a[21] = vfmaq_f64(a[21], vld1q_f64(t), xj); t += 2;
+    a[22] = vfmaq_f64(a[22], vld1q_f64(t), xj); t += 2;
+    a[23] = vfmaq_f64(a[23], vld1q_f64(t), xj); t += 2;
+    a[24] = vfmaq_f64(a[24], vld1q_f64(t), xj); t += 2;
+    xj = vld1q_dup_f64(state + 32);
+    a[0] = vfmaq_f64(a[0], vld1q_f64(t), xj); t += 2;
+    a[1] = vfmaq_f64(a[1], vld1q_f64(t), xj); t += 2;
+    a[2] = vfmaq_f64(a[2], vld1q_f64(t), xj); t += 2;
+    a[3] = vfmaq_f64(a[3], vld1q_f64(t), xj); t += 2;
+    a[4] = vfmaq_f64(a[4], vld1q_f64(t), xj); t += 2;
+    a[5] = vfmaq_f64(a[5], vld1q_f64(t), xj); t += 2;
+    a[7] = vfmaq_f64(a[7], vld1q_f64(t), xj); t += 2;
+    a[8] = vfmaq_f64(a[8], vld1q_f64(t), xj); t += 2;
+    a[9] = vfmaq_f64(a[9], vld1q_f64(t), xj); t += 2;
+    a[10] = vfmaq_f64(a[10], vld1q_f64(t), xj); t += 2;
+    a[11] = vfmaq_f64(a[11], vld1q_f64(t), xj); t += 2;
+    a[12] = vfmaq_f64(a[12], vld1q_f64(t), xj); t += 2;
+    a[13] = vfmaq_f64(a[13], vld1q_f64(t), xj); t += 2;
+    a[14] = vfmaq_f64(a[14], vld1q_f64(t), xj); t += 2;
+    a[15] = vfmaq_f64(a[15], vld1q_f64(t), xj); t += 2;
+    a[16] = vfmaq_f64(a[16], vld1q_f64(t), xj); t += 2;
+    a[17] = vfmaq_f64(a[17], vld1q_f64(t), xj); t += 2;
+    a[18] = vfmaq_f64(a[18], vld1q_f64(t), xj); t += 2;
+    a[19] = vfmaq_f64(a[19], vld1q_f64(t), xj); t += 2;
+    a[20] = vfmaq_f64(a[20], vld1q_f64(t), xj); t += 2;
+    a[22] = vfmaq_f64(a[22], vld1q_f64(t), xj); t += 2;
+    a[23] = vfmaq_f64(a[23], vld1q_f64(t), xj); t += 2;
+    a[24] = vfmaq_f64(a[24], vld1q_f64(t), xj); t += 2;
+    xj = vld1q_dup_f64(state + 33);
+    a[0] = vfmaq_f64(a[0], vld1q_f64(t), xj); t += 2;
+    a[1] = vfmaq_f64(a[1], vld1q_f64(t), xj); t += 2;
+    a[2] = vfmaq_f64(a[2], vld1q_f64(t), xj); t += 2;
+    a[3] = vfmaq_f64(a[3], vld1q_f64(t), xj); t += 2;
+    a[5] = vfmaq_f64(a[5], vld1q_f64(t), xj); t += 2;
+    a[6] = vfmaq_f64(a[6], vld1q_f64(t), xj); t += 2;
+    a[7] = vfmaq_f64(a[7], vld1q_f64(t), xj); t += 2;
+    a[8] = vfmaq_f64(a[8], vld1q_f64(t), xj); t += 2;
+    a[9] = vfmaq_f64(a[9], vld1q_f64(t), xj); t += 2;
+    a[10] = vfmaq_f64(a[10], vld1q_f64(t), xj); t += 2;
+    a[11] = vfmaq_f64(a[11], vld1q_f64(t), xj); t += 2;
+    a[12] = vfmaq_f64(a[12], vld1q_f64(t), xj); t += 2;
+    a[13] = vfmaq_f64(a[13], vld1q_f64(t), xj); t += 2;
+    a[14] = vfmaq_f64(a[14], vld1q_f64(t), xj); t += 2;
+    a[15] = vfmaq_f64(a[15], vld1q_f64(t), xj); t += 2;
+    a[16] = vfmaq_f64(a[16], vld1q_f64(t), xj); t += 2;
+    a[17] = vfmaq_f64(a[17], vld1q_f64(t), xj); t += 2;
+    a[18] = vfmaq_f64(a[18], vld1q_f64(t), xj); t += 2;
+    a[19] = vfmaq_f64(a[19], vld1q_f64(t), xj); t += 2;
+    a[20] = vfmaq_f64(a[20], vld1q_f64(t), xj); t += 2;
+    a[21] = vfmaq_f64(a[21], vld1q_f64(t), xj); t += 2;
+    a[22] = vfmaq_f64(a[22], vld1q_f64(t), xj); t += 2;
+    a[23] = vfmaq_f64(a[23], vld1q_f64(t), xj); t += 2;
+    a[24] = vfmaq_f64(a[24], vld1q_f64(t), xj); t += 2;
+    xj = vld1q_dup_f64(state + 34);
+    a[0] = vfmaq_f64(a[0], vld1q_f64(t), xj); t += 2;
+    a[2] = vfmaq_f64(a[2], vld1q_f64(t), xj); t += 2;
+    a[3] = vfmaq_f64(a[3], vld1q_f64(t), xj); t += 2;
+    a[5] = vfmaq_f64(a[5], vld1q_f64(t), xj); t += 2;
+    a[7] = vfmaq_f64(a[7], vld1q_f64(t), xj); t += 2;
+    a[8] = vfmaq_f64(a[8], vld1q_f64(t), xj); t += 2;
+    a[9] = vfmaq_f64(a[9], vld1q_f64(t), xj); t += 2;
+    a[10] = vfmaq_f64(a[10], vld1q_f64(t), xj); t += 2;
+    a[11] = vfmaq_f64(a[11], vld1q_f64(t), xj); t += 2;
+    a[12] = vfmaq_f64(a[12], vld1q_f64(t), xj); t += 2;
+    a[13] = vfmaq_f64(a[13], vld1q_f64(t), xj); t += 2;
+    a[14] = vfmaq_f64(a[14], vld1q_f64(t), xj); t += 2;
+    a[15] = vfmaq_f64(a[15], vld1q_f64(t), xj); t += 2;
+    a[16] = vfmaq_f64(a[16], vld1q_f64(t), xj); t += 2;
+    a[17] = vfmaq_f64(a[17], vld1q_f64(t), xj); t += 2;
+    a[18] = vfmaq_f64(a[18], vld1q_f64(t), xj); t += 2;
+    a[19] = vfmaq_f64(a[19], vld1q_f64(t), xj); t += 2;
+    a[20] = vfmaq_f64(a[20], vld1q_f64(t), xj); t += 2;
+    a[22] = vfmaq_f64(a[22], vld1q_f64(t), xj); t += 2;
+    a[23] = vfmaq_f64(a[23], vld1q_f64(t), xj); t += 2;
+    a[24] = vfmaq_f64(a[24], vld1q_f64(t), xj); t += 2;
+    xj = vld1q_dup_f64(state + 35);
+    a[0] = vfmaq_f64(a[0], vld1q_f64(t), xj); t += 2;
+    a[1] = vfmaq_f64(a[1], vld1q_f64(t), xj); t += 2;
+    a[2] = vfmaq_f64(a[2], vld1q_f64(t), xj); t += 2;
+    a[3] = vfmaq_f64(a[3], vld1q_f64(t), xj); t += 2;
+    a[4] = vfmaq_f64(a[4], vld1q_f64(t), xj); t += 2;
+    a[5] = vfmaq_f64(a[5], vld1q_f64(t), xj); t += 2;
+    a[6] = vfmaq_f64(a[6], vld1q_f64(t), xj); t += 2;
+    a[7] = vfmaq_f64(a[7], vld1q_f64(t), xj); t += 2;
+    a[8] = vfmaq_f64(a[8], vld1q_f64(t), xj); t += 2;
+    a[9] = vfmaq_f64(a[9], vld1q_f64(t), xj); t += 2;
+    a[10] = vfmaq_f64(a[10], vld1q_f64(t), xj); t += 2;
+    a[11] = vfmaq_f64(a[11], vld1q_f64(t), xj); t += 2;
+    a[12] = vfmaq_f64(a[12], vld1q_f64(t), xj); t += 2;
+    a[14] = vfmaq_f64(a[14], vld1q_f64(t), xj); t += 2;
+    a[15] = vfmaq_f64(a[15], vld1q_f64(t), xj); t += 2;
+    a[16] = vfmaq_f64(a[16], vld1q_f64(t), xj); t += 2;
+    a[17] = vfmaq_f64(a[17], vld1q_f64(t), xj); t += 2;
+    a[18] = vfmaq_f64(a[18], vld1q_f64(t), xj); t += 2;
+    a[19] = vfmaq_f64(a[19], vld1q_f64(t), xj); t += 2;
+    a[20] = vfmaq_f64(a[20], vld1q_f64(t), xj); t += 2;
+    a[23] = vfmaq_f64(a[23], vld1q_f64(t), xj); t += 2;
+    a[24] = vfmaq_f64(a[24], vld1q_f64(t), xj); t += 2;
+    xj = vld1q_dup_f64(state + 36);
+    a[0] = vfmaq_f64(a[0], vld1q_f64(t), xj); t += 2;
+    a[1] = vfmaq_f64(a[1], vld1q_f64(t), xj); t += 2;
+    a[2] = vfmaq_f64(a[2], vld1q_f64(t), xj); t += 2;
+    a[3] = vfmaq_f64(a[3], vld1q_f64(t), xj); t += 2;
+    a[4] = vfmaq_f64(a[4], vld1q_f64(t), xj); t += 2;
+    a[5] = vfmaq_f64(a[5], vld1q_f64(t), xj); t += 2;
+    a[6] = vfmaq_f64(a[6], vld1q_f64(t), xj); t += 2;
+    a[7] = vfmaq_f64(a[7], vld1q_f64(t), xj); t += 2;
+    a[8] = vfmaq_f64(a[8], vld1q_f64(t), xj); t += 2;
+    a[9] = vfmaq_f64(a[9], vld1q_f64(t), xj); t += 2;
+    a[10] = vfmaq_f64(a[10], vld1q_f64(t), xj); t += 2;
+    a[11] = vfmaq_f64(a[11], vld1q_f64(t), xj); t += 2;
+    a[12] = vfmaq_f64(a[12], vld1q_f64(t), xj); t += 2;
+    a[13] = vfmaq_f64(a[13], vld1q_f64(t), xj); t += 2;
+    a[14] = vfmaq_f64(a[14], vld1q_f64(t), xj); t += 2;
+    a[15] = vfmaq_f64(a[15], vld1q_f64(t), xj); t += 2;
+    a[16] = vfmaq_f64(a[16], vld1q_f64(t), xj); t += 2;
+    a[18] = vfmaq_f64(a[18], vld1q_f64(t), xj); t += 2;
+    a[19] = vfmaq_f64(a[19], vld1q_f64(t), xj); t += 2;
+    a[20] = vfmaq_f64(a[20], vld1q_f64(t), xj); t += 2;
+    a[21] = vfmaq_f64(a[21], vld1q_f64(t), xj); t += 2;
+    a[22] = vfmaq_f64(a[22], vld1q_f64(t), xj); t += 2;
+    a[23] = vfmaq_f64(a[23], vld1q_f64(t), xj); t += 2;
+    a[24] = vfmaq_f64(a[24], vld1q_f64(t), xj); t += 2;
+    xj = vld1q_dup_f64(state + 37);
+    a[0] = vfmaq_f64(a[0], vld1q_f64(t), xj); t += 2;
+    a[1] = vfmaq_f64(a[1], vld1q_f64(t), xj); t += 2;
+    a[2] = vfmaq_f64(a[2], vld1q_f64(t), xj); t += 2;
+    a[3] = vfmaq_f64(a[3], vld1q_f64(t), xj); t += 2;
+    a[4] = vfmaq_f64(a[4], vld1q_f64(t), xj); t += 2;
+    a[5] = vfmaq_f64(a[5], vld1q_f64(t), xj); t += 2;
+    a[6] = vfmaq_f64(a[6], vld1q_f64(t), xj); t += 2;
+    a[7] = vfmaq_f64(a[7], vld1q_f64(t), xj); t += 2;
+    a[8] = vfmaq_f64(a[8], vld1q_f64(t), xj); t += 2;
+    a[9] = vfmaq_f64(a[9], vld1q_f64(t), xj); t += 2;
+    a[10] = vfmaq_f64(a[10], vld1q_f64(t), xj); t += 2;
+    a[11] = vfmaq_f64(a[11], vld1q_f64(t), xj); t += 2;
+    a[12] = vfmaq_f64(a[12], vld1q_f64(t), xj); t += 2;
+    a[13] = vfmaq_f64(a[13], vld1q_f64(t), xj); t += 2;
+    a[15] = vfmaq_f64(a[15], vld1q_f64(t), xj); t += 2;
+    a[16] = vfmaq_f64(a[16], vld1q_f64(t), xj); t += 2;
+    a[17] = vfmaq_f64(a[17], vld1q_f64(t), xj); t += 2;
+    a[18] = vfmaq_f64(a[18], vld1q_f64(t), xj); t += 2;
+    a[19] = vfmaq_f64(a[19], vld1q_f64(t), xj); t += 2;
+    a[20] = vfmaq_f64(a[20], vld1q_f64(t), xj); t += 2;
+    a[21] = vfmaq_f64(a[21], vld1q_f64(t), xj); t += 2;
+    a[22] = vfmaq_f64(a[22], vld1q_f64(t), xj); t += 2;
+    a[23] = vfmaq_f64(a[23], vld1q_f64(t), xj); t += 2;
+    a[24] = vfmaq_f64(a[24], vld1q_f64(t), xj); t += 2;
+    xj = vld1q_dup_f64(state + 38);
+    a[0] = vfmaq_f64(a[0], vld1q_f64(t), xj); t += 2;
+    a[1] = vfmaq_f64(a[1], vld1q_f64(t), xj); t += 2;
+    a[2] = vfmaq_f64(a[2], vld1q_f64(t), xj); t += 2;
+    a[3] = vfmaq_f64(a[3], vld1q_f64(t), xj); t += 2;
+    a[4] = vfmaq_f64(a[4], vld1q_f64(t), xj); t += 2;
+    a[5] = vfmaq_f64(a[5], vld1q_f64(t), xj); t += 2;
+    a[7] = vfmaq_f64(a[7], vld1q_f64(t), xj); t += 2;
+    a[8] = vfmaq_f64(a[8], vld1q_f64(t), xj); t += 2;
+    a[9] = vfmaq_f64(a[9], vld1q_f64(t), xj); t += 2;
+    a[10] = vfmaq_f64(a[10], vld1q_f64(t), xj); t += 2;
+    a[11] = vfmaq_f64(a[11], vld1q_f64(t), xj); t += 2;
+    a[12] = vfmaq_f64(a[12], vld1q_f64(t), xj); t += 2;
+    a[13] = vfmaq_f64(a[13], vld1q_f64(t), xj); t += 2;
+    a[14] = vfmaq_f64(a[14], vld1q_f64(t), xj); t += 2;
+    a[15] = vfmaq_f64(a[15], vld1q_f64(t), xj); t += 2;
+    a[16] = vfmaq_f64(a[16], vld1q_f64(t), xj); t += 2;
+    a[17] = vfmaq_f64(a[17], vld1q_f64(t), xj); t += 2;
+    a[18] = vfmaq_f64(a[18], vld1q_f64(t), xj); t += 2;
+    a[19] = vfmaq_f64(a[19], vld1q_f64(t), xj); t += 2;
+    a[20] = vfmaq_f64(a[20], vld1q_f64(t), xj); t += 2;
+    a[21] = vfmaq_f64(a[21], vld1q_f64(t), xj); t += 2;
+    a[22] = vfmaq_f64(a[22], vld1q_f64(t), xj); t += 2;
+    a[23] = vfmaq_f64(a[23], vld1q_f64(t), xj); t += 2;
+    a[24] = vfmaq_f64(a[24], vld1q_f64(t), xj); t += 2;
+    xj = vld1q_dup_f64(state + 39);
+    a[0] = vfmaq_f64(a[0], vld1q_f64(t), xj); t += 2;
+    a[1] = vfmaq_f64(a[1], vld1q_f64(t), xj); t += 2;
+    a[2] = vfmaq_f64(a[2], vld1q_f64(t), xj); t += 2;
+    a[3] = vfmaq_f64(a[3], vld1q_f64(t), xj); t += 2;
+    a[4] = vfmaq_f64(a[4], vld1q_f64(t), xj); t += 2;
+    a[5] = vfmaq_f64(a[5], vld1q_f64(t), xj); t += 2;
+    a[6] = vfmaq_f64(a[6], vld1q_f64(t), xj); t += 2;
+    a[7] = vfmaq_f64(a[7], vld1q_f64(t), xj); t += 2;
+    a[8] = vfmaq_f64(a[8], vld1q_f64(t), xj); t += 2;
+    a[9] = vfmaq_f64(a[9], vld1q_f64(t), xj); t += 2;
+    a[11] = vfmaq_f64(a[11], vld1q_f64(t), xj); t += 2;
+    a[12] = vfmaq_f64(a[12], vld1q_f64(t), xj); t += 2;
+    a[14] = vfmaq_f64(a[14], vld1q_f64(t), xj); t += 2;
+    a[15] = vfmaq_f64(a[15], vld1q_f64(t), xj); t += 2;
+    a[16] = vfmaq_f64(a[16], vld1q_f64(t), xj); t += 2;
+    a[17] = vfmaq_f64(a[17], vld1q_f64(t), xj); t += 2;
+    a[18] = vfmaq_f64(a[18], vld1q_f64(t), xj); t += 2;
+    a[19] = vfmaq_f64(a[19], vld1q_f64(t), xj); t += 2;
+    a[20] = vfmaq_f64(a[20], vld1q_f64(t), xj); t += 2;
+    a[21] = vfmaq_f64(a[21], vld1q_f64(t), xj); t += 2;
+    a[22] = vfmaq_f64(a[22], vld1q_f64(t), xj); t += 2;
+    a[23] = vfmaq_f64(a[23], vld1q_f64(t), xj); t += 2;
+    a[24] = vfmaq_f64(a[24], vld1q_f64(t), xj); t += 2;
+    xj = vld1q_dup_f64(state + 40);
+    a[0] = vfmaq_f64(a[0], vld1q_f64(t), xj); t += 2;
+    a[1] = vfmaq_f64(a[1], vld1q_f64(t), xj); t += 2;
+    a[2] = vfmaq_f64(a[2], vld1q_f64(t), xj); t += 2;
+    a[3] = vfmaq_f64(a[3], vld1q_f64(t), xj); t += 2;
+    a[4] = vfmaq_f64(a[4], vld1q_f64(t), xj); t += 2;
+    a[5] = vfmaq_f64(a[5], vld1q_f64(t), xj); t += 2;
+    a[7] = vfmaq_f64(a[7], vld1q_f64(t), xj); t += 2;
+    a[8] = vfmaq_f64(a[8], vld1q_f64(t), xj); t += 2;
+    a[9] = vfmaq_f64(a[9], vld1q_f64(t), xj); t += 2;
+    a[11] = vfmaq_f64(a[11], vld1q_f64(t), xj); t += 2;
+    a[12] = vfmaq_f64(a[12], vld1q_f64(t), xj); t += 2;
+    a[14] = vfmaq_f64(a[14], vld1q_f64(t), xj); t += 2;
+    a[16] = vfmaq_f64(a[16], vld1q_f64(t), xj); t += 2;
+    a[18] = vfmaq_f64(a[18], vld1q_f64(t), xj); t += 2;
+    a[19] = vfmaq_f64(a[19], vld1q_f64(t), xj); t += 2;
+    a[20] = vfmaq_f64(a[20], vld1q_f64(t), xj); t += 2;
+    a[22] = vfmaq_f64(a[22], vld1q_f64(t), xj); t += 2;
+    a[23] = vfmaq_f64(a[23], vld1q_f64(t), xj); t += 2;
+    a[24] = vfmaq_f64(a[24], vld1q_f64(t), xj); t += 2;
+    xj = vld1q_dup_f64(state + 41);
+    a[0] = vfmaq_f64(a[0], vld1q_f64(t), xj); t += 2;
+    a[1] = vfmaq_f64(a[1], vld1q_f64(t), xj); t += 2;
+    a[2] = vfmaq_f64(a[2], vld1q_f64(t), xj); t += 2;
+    a[3] = vfmaq_f64(a[3], vld1q_f64(t), xj); t += 2;
+    a[4] = vfmaq_f64(a[4], vld1q_f64(t), xj); t += 2;
+    a[5] = vfmaq_f64(a[5], vld1q_f64(t), xj); t += 2;
+    a[6] = vfmaq_f64(a[6], vld1q_f64(t), xj); t += 2;
+    a[7] = vfmaq_f64(a[7], vld1q_f64(t), xj); t += 2;
+    a[8] = vfmaq_f64(a[8], vld1q_f64(t), xj); t += 2;
+    a[9] = vfmaq_f64(a[9], vld1q_f64(t), xj); t += 2;
+    a[10] = vfmaq_f64(a[10], vld1q_f64(t), xj); t += 2;
+    a[11] = vfmaq_f64(a[11], vld1q_f64(t), xj); t += 2;
+    a[12] = vfmaq_f64(a[12], vld1q_f64(t), xj); t += 2;
+    a[13] = vfmaq_f64(a[13], vld1q_f64(t), xj); t += 2;
+    a[14] = vfmaq_f64(a[14], vld1q_f64(t), xj); t += 2;
+    a[15] = vfmaq_f64(a[15], vld1q_f64(t), xj); t += 2;
+    a[16] = vfmaq_f64(a[16], vld1q_f64(t), xj); t += 2;
+    a[17] = vfmaq_f64(a[17], vld1q_f64(t), xj); t += 2;
+    a[18] = vfmaq_f64(a[18], vld1q_f64(t), xj); t += 2;
+    a[19] = vfmaq_f64(a[19], vld1q_f64(t), xj); t += 2;
+    a[20] = vfmaq_f64(a[20], vld1q_f64(t), xj); t += 2;
+    a[21] = vfmaq_f64(a[21], vld1q_f64(t), xj); t += 2;
+    a[22] = vfmaq_f64(a[22], vld1q_f64(t), xj); t += 2;
+    a[23] = vfmaq_f64(a[23], vld1q_f64(t), xj); t += 2;
+    a[24] = vfmaq_f64(a[24], vld1q_f64(t), xj); t += 2;
+    xj = vld1q_dup_f64(state + 42);
+    a[0] = vfmaq_f64(a[0], vld1q_f64(t), xj); t += 2;
+    a[1] = vfmaq_f64(a[1], vld1q_f64(t), xj); t += 2;
+    a[3] = vfmaq_f64(a[3], vld1q_f64(t), xj); t += 2;
+    a[4] = vfmaq_f64(a[4], vld1q_f64(t), xj); t += 2;
+    a[5] = vfmaq_f64(a[5], vld1q_f64(t), xj); t += 2;
+    a[7] = vfmaq_f64(a[7], vld1q_f64(t), xj); t += 2;
+    a[8] = vfmaq_f64(a[8], vld1q_f64(t), xj); t += 2;
+    a[9] = vfmaq_f64(a[9], vld1q_f64(t), xj); t += 2;
+    a[10] = vfmaq_f64(a[10], vld1q_f64(t), xj); t += 2;
+    a[12] = vfmaq_f64(a[12], vld1q_f64(t), xj); t += 2;
+    a[13] = vfmaq_f64(a[13], vld1q_f64(t), xj); t += 2;
+    a[14] = vfmaq_f64(a[14], vld1q_f64(t), xj); t += 2;
+    a[15] = vfmaq_f64(a[15], vld1q_f64(t), xj); t += 2;
+    a[16] = vfmaq_f64(a[16], vld1q_f64(t), xj); t += 2;
+    a[18] = vfmaq_f64(a[18], vld1q_f64(t), xj); t += 2;
+    a[19] = vfmaq_f64(a[19], vld1q_f64(t), xj); t += 2;
+    a[20] = vfmaq_f64(a[20], vld1q_f64(t), xj); t += 2;
+    a[21] = vfmaq_f64(a[21], vld1q_f64(t), xj); t += 2;
+    a[22] = vfmaq_f64(a[22], vld1q_f64(t), xj); t += 2;
+    a[23] = vfmaq_f64(a[23], vld1q_f64(t), xj); t += 2;
+    a[24] = vfmaq_f64(a[24], vld1q_f64(t), xj); t += 2;
+    xj = vld1q_dup_f64(state + 43);
+    a[0] = vfmaq_f64(a[0], vld1q_f64(t), xj); t += 2;
+    a[1] = vfmaq_f64(a[1], vld1q_f64(t), xj); t += 2;
+    a[2] = vfmaq_f64(a[2], vld1q_f64(t), xj); t += 2;
+    a[3] = vfmaq_f64(a[3], vld1q_f64(t), xj); t += 2;
+    a[4] = vfmaq_f64(a[4], vld1q_f64(t), xj); t += 2;
+    a[5] = vfmaq_f64(a[5], vld1q_f64(t), xj); t += 2;
+    a[6] = vfmaq_f64(a[6], vld1q_f64(t), xj); t += 2;
+    a[7] = vfmaq_f64(a[7], vld1q_f64(t), xj); t += 2;
+    a[8] = vfmaq_f64(a[8], vld1q_f64(t), xj); t += 2;
+    a[9] = vfmaq_f64(a[9], vld1q_f64(t), xj); t += 2;
+    a[10] = vfmaq_f64(a[10], vld1q_f64(t), xj); t += 2;
+    a[11] = vfmaq_f64(a[11], vld1q_f64(t), xj); t += 2;
+    a[12] = vfmaq_f64(a[12], vld1q_f64(t), xj); t += 2;
+    a[13] = vfmaq_f64(a[13], vld1q_f64(t), xj); t += 2;
+    a[14] = vfmaq_f64(a[14], vld1q_f64(t), xj); t += 2;
+    a[15] = vfmaq_f64(a[15], vld1q_f64(t), xj); t += 2;
+    a[16] = vfmaq_f64(a[16], vld1q_f64(t), xj); t += 2;
+    a[18] = vfmaq_f64(a[18], vld1q_f64(t), xj); t += 2;
+    a[19] = vfmaq_f64(a[19], vld1q_f64(t), xj); t += 2;
+    a[20] = vfmaq_f64(a[20], vld1q_f64(t), xj); t += 2;
+    a[21] = vfmaq_f64(a[21], vld1q_f64(t), xj); t += 2;
+    a[22] = vfmaq_f64(a[22], vld1q_f64(t), xj); t += 2;
+    a[23] = vfmaq_f64(a[23], vld1q_f64(t), xj); t += 2;
+    xj = vld1q_dup_f64(state + 44);
+    a[0] = vfmaq_f64(a[0], vld1q_f64(t), xj); t += 2;
+    a[1] = vfmaq_f64(a[1], vld1q_f64(t), xj); t += 2;
+    a[2] = vfmaq_f64(a[2], vld1q_f64(t), xj); t += 2;
+    a[3] = vfmaq_f64(a[3], vld1q_f64(t), xj); t += 2;
+    a[4] = vfmaq_f64(a[4], vld1q_f64(t), xj); t += 2;
+    a[5] = vfmaq_f64(a[5], vld1q_f64(t), xj); t += 2;
+    a[6] = vfmaq_f64(a[6], vld1q_f64(t), xj); t += 2;
+    a[7] = vfmaq_f64(a[7], vld1q_f64(t), xj); t += 2;
+    a[9] = vfmaq_f64(a[9], vld1q_f64(t), xj); t += 2;
+    a[10] = vfmaq_f64(a[10], vld1q_f64(t), xj); t += 2;
+    a[11] = vfmaq_f64(a[11], vld1q_f64(t), xj); t += 2;
+    a[12] = vfmaq_f64(a[12], vld1q_f64(t), xj); t += 2;
+    a[13] = vfmaq_f64(a[13], vld1q_f64(t), xj); t += 2;
+    a[14] = vfmaq_f64(a[14], vld1q_f64(t), xj); t += 2;
+    a[15] = vfmaq_f64(a[15], vld1q_f64(t), xj); t += 2;
+    a[16] = vfmaq_f64(a[16], vld1q_f64(t), xj); t += 2;
+    a[18] = vfmaq_f64(a[18], vld1q_f64(t), xj); t += 2;
+    a[19] = vfmaq_f64(a[19], vld1q_f64(t), xj); t += 2;
+    a[20] = vfmaq_f64(a[20], vld1q_f64(t), xj); t += 2;
+    a[21] = vfmaq_f64(a[21], vld1q_f64(t), xj); t += 2;
+    a[22] = vfmaq_f64(a[22], vld1q_f64(t), xj); t += 2;
+    a[23] = vfmaq_f64(a[23], vld1q_f64(t), xj); t += 2;
+    a[24] = vfmaq_f64(a[24], vld1q_f64(t), xj); t += 2;
+    xj = vld1q_dup_f64(state + 45);
+    a[0] = vfmaq_f64(a[0], vld1q_f64(t), xj); t += 2;
+    a[1] = vfmaq_f64(a[1], vld1q_f64(t), xj); t += 2;
+    a[2] = vfmaq_f64(a[2], vld1q_f64(t), xj); t += 2;
+    a[3] = vfmaq_f64(a[3], vld1q_f64(t), xj); t += 2;
+    a[4] = vfmaq_f64(a[4], vld1q_f64(t), xj); t += 2;
+    a[5] = vfmaq_f64(a[5], vld1q_f64(t), xj); t += 2;
+    a[6] = vfmaq_f64(a[6], vld1q_f64(t), xj); t += 2;
+    a[7] = vfmaq_f64(a[7], vld1q_f64(t), xj); t += 2;
+    a[8] = vfmaq_f64(a[8], vld1q_f64(t), xj); t += 2;
+    a[9] = vfmaq_f64(a[9], vld1q_f64(t), xj); t += 2;
+    a[10] = vfmaq_f64(a[10], vld1q_f64(t), xj); t += 2;
+    a[11] = vfmaq_f64(a[11], vld1q_f64(t), xj); t += 2;
+    a[12] = vfmaq_f64(a[12], vld1q_f64(t), xj); t += 2;
+    a[13] = vfmaq_f64(a[13], vld1q_f64(t), xj); t += 2;
+    a[14] = vfmaq_f64(a[14], vld1q_f64(t), xj); t += 2;
+    a[15] = vfmaq_f64(a[15], vld1q_f64(t), xj); t += 2;
+    a[16] = vfmaq_f64(a[16], vld1q_f64(t), xj); t += 2;
+    a[17] = vfmaq_f64(a[17], vld1q_f64(t), xj); t += 2;
+    a[18] = vfmaq_f64(a[18], vld1q_f64(t), xj); t += 2;
+    a[19] = vfmaq_f64(a[19], vld1q_f64(t), xj); t += 2;
+    a[20] = vfmaq_f64(a[20], vld1q_f64(t), xj); t += 2;
+    a[21] = vfmaq_f64(a[21], vld1q_f64(t), xj); t += 2;
+    a[22] = vfmaq_f64(a[22], vld1q_f64(t), xj); t += 2;
+    a[23] = vfmaq_f64(a[23], vld1q_f64(t), xj); t += 2;
+    a[24] = vfmaq_f64(a[24], vld1q_f64(t), xj); t += 2;
+    xj = vld1q_dup_f64(state + 46);
+    a[0] = vfmaq_f64(a[0], vld1q_f64(t), xj); t += 2;
+    a[1] = vfmaq_f64(a[1], vld1q_f64(t), xj); t += 2;
+    a[2] = vfmaq_f64(a[2], vld1q_f64(t), xj); t += 2;
+    a[3] = vfmaq_f64(a[3], vld1q_f64(t), xj); t += 2;
+    a[4] = vfmaq_f64(a[4], vld1q_f64(t), xj); t += 2;
+    a[5] = vfmaq_f64(a[5], vld1q_f64(t), xj); t += 2;
+    a[6] = vfmaq_f64(a[6], vld1q_f64(t), xj); t += 2;
+    a[7] = vfmaq_f64(a[7], vld1q_f64(t), xj); t += 2;
+    a[8] = vfmaq_f64(a[8], vld1q_f64(t), xj); t += 2;
+    a[9] = vfmaq_f64(a[9], vld1q_f64(t), xj); t += 2;
+    a[10] = vfmaq_f64(a[10], vld1q_f64(t), xj); t += 2;
+    a[11] = vfmaq_f64(a[11], vld1q_f64(t), xj); t += 2;
+    a[12] = vfmaq_f64(a[12], vld1q_f64(t), xj); t += 2;
+    a[13] = vfmaq_f64(a[13], vld1q_f64(t), xj); t += 2;
+    a[14] = vfmaq_f64(a[14], vld1q_f64(t), xj); t += 2;
+    a[15] = vfmaq_f64(a[15], vld1q_f64(t), xj); t += 2;
+    a[16] = vfmaq_f64(a[16], vld1q_f64(t), xj); t += 2;
+    a[17] = vfmaq_f64(a[17], vld1q_f64(t), xj); t += 2;
+    a[18] = vfmaq_f64(a[18], vld1q_f64(t), xj); t += 2;
+    a[19] = vfmaq_f64(a[19], vld1q_f64(t), xj); t += 2;
+    a[20] = vfmaq_f64(a[20], vld1q_f64(t), xj); t += 2;
+    a[21] = vfmaq_f64(a[21], vld1q_f64(t), xj); t += 2;
+    a[22] = vfmaq_f64(a[22], vld1q_f64(t), xj); t += 2;
+    a[23] = vfmaq_f64(a[23], vld1q_f64(t), xj); t += 2;
+    a[24] = vfmaq_f64(a[24], vld1q_f64(t), xj); t += 2;
+    xj = vld1q_dup_f64(state + 47);
+    a[0] = vfmaq_f64(a[0], vld1q_f64(t), xj); t += 2;
+    a[1] = vfmaq_f64(a[1], vld1q_f64(t), xj); t += 2;
+    a[2] = vfmaq_f64(a[2], vld1q_f64(t), xj); t += 2;
+    a[3] = vfmaq_f64(a[3], vld1q_f64(t), xj); t += 2;
+    a[4] = vfmaq_f64(a[4], vld1q_f64(t), xj); t += 2;
+    a[5] = vfmaq_f64(a[5], vld1q_f64(t), xj); t += 2;
+    a[7] = vfmaq_f64(a[7], vld1q_f64(t), xj); t += 2;
+    a[8] = vfmaq_f64(a[8], vld1q_f64(t), xj); t += 2;
+    a[9] = vfmaq_f64(a[9], vld1q_f64(t), xj); t += 2;
+    a[10] = vfmaq_f64(a[10], vld1q_f64(t), xj); t += 2;
+    a[11] = vfmaq_f64(a[11], vld1q_f64(t), xj); t += 2;
+    a[12] = vfmaq_f64(a[12], vld1q_f64(t), xj); t += 2;
+    a[13] = vfmaq_f64(a[13], vld1q_f64(t), xj); t += 2;
+    a[14] = vfmaq_f64(a[14], vld1q_f64(t), xj); t += 2;
+    a[15] = vfmaq_f64(a[15], vld1q_f64(t), xj); t += 2;
+    a[16] = vfmaq_f64(a[16], vld1q_f64(t), xj); t += 2;
+    a[17] = vfmaq_f64(a[17], vld1q_f64(t), xj); t += 2;
+    a[18] = vfmaq_f64(a[18], vld1q_f64(t), xj); t += 2;
+    a[20] = vfmaq_f64(a[20], vld1q_f64(t), xj); t += 2;
+    a[21] = vfmaq_f64(a[21], vld1q_f64(t), xj); t += 2;
+    a[22] = vfmaq_f64(a[22], vld1q_f64(t), xj); t += 2;
+    a[23] = vfmaq_f64(a[23], vld1q_f64(t), xj); t += 2;
+    a[24] = vfmaq_f64(a[24], vld1q_f64(t), xj); t += 2;
+    xj = vld1q_dup_f64(state + 48);
+    a[2] = vfmaq_f64(a[2], vld1q_f64(t), xj); t += 2;
+    a[5] = vfmaq_f64(a[5], vld1q_f64(t), xj); t += 2;
+    a[9] = vfmaq_f64(a[9], vld1q_f64(t), xj); t += 2;
+    a[10] = vfmaq_f64(a[10], vld1q_f64(t), xj); t += 2;
+    a[11] = vfmaq_f64(a[11], vld1q_f64(t), xj); t += 2;
+    a[12] = vfmaq_f64(a[12], vld1q_f64(t), xj); t += 2;
+    a[14] = vfmaq_f64(a[14], vld1q_f64(t), xj); t += 2;
+    a[15] = vfmaq_f64(a[15], vld1q_f64(t), xj); t += 2;
+    a[18] = vfmaq_f64(a[18], vld1q_f64(t), xj); t += 2;
+    a[19] = vfmaq_f64(a[19], vld1q_f64(t), xj); t += 2;
+    a[20] = vfmaq_f64(a[20], vld1q_f64(t), xj); t += 2;
+    a[22] = vfmaq_f64(a[22], vld1q_f64(t), xj); t += 2;
+    a[23] = vfmaq_f64(a[23], vld1q_f64(t), xj); t += 2;
+    a[24] = vfmaq_f64(a[24], vld1q_f64(t), xj); t += 2;
+    xj = vld1q_dup_f64(state + 49);
+    a[0] = vfmaq_f64(a[0], vld1q_f64(t), xj); t += 2;
+    a[1] = vfmaq_f64(a[1], vld1q_f64(t), xj); t += 2;
+    a[2] = vfmaq_f64(a[2], vld1q_f64(t), xj); t += 2;
+    a[3] = vfmaq_f64(a[3], vld1q_f64(t), xj); t += 2;
+    a[4] = vfmaq_f64(a[4], vld1q_f64(t), xj); t += 2;
+    a[5] = vfmaq_f64(a[5], vld1q_f64(t), xj); t += 2;
+    a[6] = vfmaq_f64(a[6], vld1q_f64(t), xj); t += 2;
+    a[7] = vfmaq_f64(a[7], vld1q_f64(t), xj); t += 2;
+    a[8] = vfmaq_f64(a[8], vld1q_f64(t), xj); t += 2;
+    a[9] = vfmaq_f64(a[9], vld1q_f64(t), xj); t += 2;
+    a[10] = vfmaq_f64(a[10], vld1q_f64(t), xj); t += 2;
+    a[11] = vfmaq_f64(a[11], vld1q_f64(t), xj); t += 2;
+    a[12] = vfmaq_f64(a[12], vld1q_f64(t), xj); t += 2;
+    a[13] = vfmaq_f64(a[13], vld1q_f64(t), xj); t += 2;
+    a[14] = vfmaq_f64(a[14], vld1q_f64(t), xj); t += 2;
+    a[15] = vfmaq_f64(a[15], vld1q_f64(t), xj); t += 2;
+    a[16] = vfmaq_f64(a[16], vld1q_f64(t), xj); t += 2;
+    a[17] = vfmaq_f64(a[17], vld1q_f64(t), xj); t += 2;
+    a[18] = vfmaq_f64(a[18], vld1q_f64(t), xj); t += 2;
+    a[19] = vfmaq_f64(a[19], vld1q_f64(t), xj); t += 2;
+    a[20] = vfmaq_f64(a[20], vld1q_f64(t), xj); t += 2;
+    a[21] = vfmaq_f64(a[21], vld1q_f64(t), xj); t += 2;
+    a[22] = vfmaq_f64(a[22], vld1q_f64(t), xj); t += 2;
+    a[23] = vfmaq_f64(a[23], vld1q_f64(t), xj); t += 2;
+    a[24] = vfmaq_f64(a[24], vld1q_f64(t), xj); t += 2;
+    for (r = 0; r < 25; ++r) {
+        float64x2_t v = a[r];
+        float64x2_t hi = vsubq_f64(v, lam), lo = vaddq_f64(v, lam);
+        float64x2_t res = vbslq_f64(vcgtq_f64(v, lam), hi, vbslq_f64(vcltq_f64(v, nlam), lo, z));
+        vst1q_f64(out + 2 * r, res);
+    }
+}
+#else
+static void fom_step_t0_impl(const fom_float *state, const fom_float *bias, const FomAct_t0 *act, fom_float *out) {
+    fom_int i, k;
+    const fom_float lam = act->lambd;
+    for (i = 0; i < 50; i += FOM_T0_RB) {
+        fom_float d[FOM_T0_RB];
+        fom_dot50xR_t0_state(FOM_MAT_t0_state + i * 50, state, d);
+        for (k = 0; k < FOM_T0_RB; ++k) {
+            fom_float v = bias[i + k] + d[k];
+            out[i + k] = (v > lam) ? (v - lam) : ((v < -lam) ? (v + lam) : 0.0);
+        }
+    }
+}
+
+#endif
+
 void fom_step_t0(const fom_float *state, const fom_float *bias, const FomAct_t0 *act, fom_float *out) {
-    fom_int i, r, j;
-    for (r = 0; r < 50; ++r) {
-        fom_float s, v;
-        i = r;
-        v = bias[i];
-        s = 0.0;
-        for (j = 0; j < 50; ++j) s += FOM_MAT_t0_state[i * 50 + j] * state[j];
-        v += s;
-        v = (v > act->lambd) ? (v - act->lambd) : ((v < -act->lambd) ? (v + act->lambd) : 0.0);
-        out[i] = v;
-    }
+    fom_step_t0_impl(state, bias, act, out);
 }
 
 void fom_step_su(const fom_float *t_t0, const fom_float *bias, fom_float *out) {
```

## Agent notes

```
### Round 1

**Whole-solve mean_us:** 87.50 initially, about 29–30 finally (last bench 28.80). Overall speedup is about 3.0x. Minimum time went from 80 to 23 µs.

**Final profile breakdown (`make profile`):**
| region | total_us (5000 solves) | % |
|---|---|---|
| iterations (200 × `fom_step_t0`: dense 50×50 matvec + bias + soft-threshold) | 158313 | 99.89 |
| residual_output (max-abs residual, copy-out, state reset) | 176 | 0.11 |

**Optimizations applied:**
- **Vectorized dense row dot products** in `fom_step_t0` and `fom_apply_t0_state`, using 2-wide NEON FMAs with several independent accumulator chains. Structure: `t0_state` is dense 50×50, stored row-major and contiguous (only 1 zero). The original scalar reduction over `j` could not vectorize without `-ffast-math`, and its single dependency chain was bound by FMA latency. 87.5 → 46 µs.
- **Row blocking (2, then 5, then 10 rows per pass)**, so each pair of `x` values is loaded once and reused across the block. This also gives 20 independent FMA chains that hide the FMA latency. Structure: 50 rows split evenly into blocks of 10, and every row has length 50 (24 pairs plus one tail pair). 46 → ~36 → ~30 → ~29 µs.
- **Fused bias and soft-threshold into the block epilogue.** `lambda` is read once from the `act` argument, and results go straight to `out`. Structure: `su_t_t0` is the identity, so the solver already swaps state buffers and skips the state-update copy. The constant `t0_p_p0 @ p0` term stays hoisted into `acc` once per parameter update.
- Kept the `FOM_PROF_BEGIN/END` markers around the iteration loop and the residual/output section.

Blocked by rules:
- Store a transposed or column-major copy of `t0_state`, computed at setup, for a register-resident outer-product formulation (25 vector accumulators, broadcast `x[j]`) | `fom_data.c` is fixed, and `fom_setup` may not probe matrices via `fom_apply_*` (inlining 2500 literals in `fom_step_t0` is allowed but impractical within the time budget) | unknown, likely <10%
- Use `-ffast-math`, `-mcpu=native` or other build flags to let the compiler auto-vectorize the reduction | Makefile and build flags are fixed | small; hand-written NEON already covers it

### Round 2

**Whole-solve time**
- Translated solver: 85.799 us.
- Start of this round: 30.388 us (pipeline figure). My own reruns of the round-1 kernel gave 28.5 to 31.7 us.
- Final: about 28.6 us mean over interleaved A/B runs, against about 30.3 us for the round-1 kernel. That is roughly 5% this round and about 3.0x overall.
- The final standalone `make bench` gave 30.24 us. Its best-case time was 22 us, against 23 to 24 us before. The machine noise (±1.5 us) is about as large as the gain.

**Final profile** (`make profile`, hottest first; that run's BENCH mean was 29.04 us):
- iterations: 144631 us, 99.87%
- residual_output: 191 us, 0.13%

**Optimizations applied**
- **Column-oriented dense kernel for `fom_step_t0`.** `t0_state` is a dense 50x50 matrix. The kernel now uses a transposed copy of it, built once from `FOM_MAT_t0_state` into a static buffer the first time the step is called (no allocation). It computes `out = bias + sum_j M[:,j] * x[j]` with 25 two-wide NEON accumulators that cover all 50 rows. The accumulators start at the bias, and each step broadcasts one `x[j]` and does 25 contiguous loads with fused multiply-adds. This removes the 50 horizontal reductions per iteration that the round-1 row-block kernel needed. The soft-threshold activation is now applied with vector compare-and-select and stored straight into the next state. `lambda` still comes from `act`, and no literals were added.
- To satisfy the one-definition kernel rule, the NEON and portable bodies became a static `fom_step_t0_impl` inside `#if`. The exported `fom_step_t0` is a single plain wrapper.
- Kept from earlier rounds: `su_t_t0` is the identity, so the step writes straight into a ping-pong state buffer. The parameter term `t0_p_p0 @ p0` is computed once per parameter update.
- Tried and dropped: unrolling over two columns at a time with lane-indexed FMAs (no gain), and row-block sizes 5 and 25 for the old kernel (both slower than 10).
- `FOM_PROF_BEGIN/END` markers are still around the `iterations` and `residual_output` regions in `fom_solver.c`.

**Remaining bound.** The loop is now limited by memory bandwidth: 1250 two-wide matrix loads per iteration over 200 iterations.

Blocked by rules:
- Use the original row-major `FOM_MAT_t0_state` directly in the column kernel, relying on `I - t A^T A` being symmetric, to skip the transposed copy | the data is only symmetric to about 4e-18 (not bitwise), and verify compares exact entries | small (only removes the one-time transpose)
- Apply `t0_state` as `I - (t A^T)(A)` with a rank-25 factor taken from `t0_p_p0` (`t A^T`), about half the FLOPs | kernel rule: `fom_step_t0` may read only its own matrix's data, not `t0_p_p0`, and recovering `t` or a factorization needs division | about 1.5 to 2x on the iteration

### Round 3

**Whole-solve `mean_us`:** 28.62 us at the start of this round, now about 24.5 us (runs ranged 24.3 to 26.7; min_us fell from 22 to 19). That is about 1.17x this round and 3.5x against the 85.8 us translation.

**Final profile (`make profile`), hottest first:**
- iterations: 122203 us, 99.87%
- residual_output: 164 us, 0.13%

**Optimizations applied this round:**
- **Unrolled `fom_step_t0` that skips zero blocks.** The 50x50 `t0_state` matrix has only 1730 nonzeros out of 2500. Its rows are grouped in adjacent pairs, which gives 1250 column blocks of size 2x1. 158 of those blocks are entirely zero.
  - The kernel is fully unrolled over this fixed pattern. It skips the zero blocks, so it does 1092 vector FMAs instead of 1250.
  - The coefficients sit in a `static const` table inside the routine, packed in the order they are used, so the loads are sequential.
  - All 25 accumulators stay in registers. The bias and the soft-threshold are folded in, and the result is written straight to the next state buffer.
  - This is valid because `fom_step_t0` is checked on every unit vector, which allows exact hex-float literals there. The hand-built transpose from earlier rounds is gone.
- **Tried and reverted: pairing rows by shared zero columns.** A greedy pairing raised the zero-block count to 254. But it needed gathered bias loads and per-lane stores, and it ran 2x slower (about 54 us).

**Structure noted but not used:** I - M is symmetric with rank 25, since the problem is a 25x50 lasso.

Blocked by rules:
- Apply M as x - (tA^T)(A x) using the rank-25 factor (`t0_p_p0` holds tA^T, so this is about 2x fewer flops) | `fom_apply_t0_*` / step helpers may read only their own matrix's data; rebuilding A would also need a division by t or a factorization at setup | ~1.5-2x on the iterations
- Use symmetry to load each off-diagonal entry once | not blocked by a rule; skipped for lack of time (the triangular scatter is hard to vectorize) | unknown

### Round 4

**Result:** the initial translated solver took 85.799 us per solve (`make bench` mean_us). The final solver takes 23.896 us, an overall speedup of 3.59x. Round 4 gained nothing, and the sources are the same as round 3's (24.733 us reported then; the difference is benchmark noise). `make verify` prints `VERIFY OK`.

**Final profile** (`make profile`, 5000 solves):
| Region | Time | Share |
|---|---|---|
| iterations | 120580 us | 99.84% |
| residual_output | 194 us | 0.16% |

**Optimizations in place (from earlier rounds):**
- **Identity state update:** `su_t_t0` is the identity, so `fom_step_t0` writes straight into the next state buffer and the two buffers are pointer-swapped. There is no state-update pass or copy.
- **Hoisted parameter term:** `t0_p_p0 @ p0` is constant across iterations. It is computed once in `fom_setup` / `fom_update_params` as the bias.
- **Column-oriented, unrolled 50x50 product:** `fom_step_t0` keeps 25 two-wide NEON accumulators covering all 50 rows, so no horizontal reductions are needed. The coefficients are inlined as literals, which the unit-vector-checked step routine allows. 2x1 blocks that are all zero are skipped (2184 stored entries out of 2500).
- **Fused bias and activation:** the bias is loaded into the accumulators and the soft-threshold is applied branch-free with NEON selects. The result is written to the destination in the same pass.

**Tried this round and reverted:**
- Skipping columns where the state entry is zero, to exploit the sparse lasso iterate. It passed verify but was 2.6x slower (62 us), because the branches broke the scheduling of the unrolled kernel.
- Lane-indexed FMAs (`vfmaq_laneq_f64`) on paired state loads, to halve the broadcast loads. It passed verify but was within noise (24.0 to 26.0 us).

The kernel runs at about 120 ns per iteration for about 1092 two-wide FMAs. It looks limited by FMA and load throughput, so I see no further structural slack within the rules.

Blocked by rules:
- none
```
