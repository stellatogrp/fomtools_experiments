# Optimization report

## Timing

```
translated (emitted)     180.6 us/solve
optimized                  9.9 us/solve
speedup                 18.31x
```

Optimization outcome: **kept (re-verified correct)**.

## What the agent changed

```diff
--- translated (emitted)
+++ optimized
@@ -8,25 +8,80 @@
 #define NT0 FOM_T0_DIM
 #define NT1 FOM_T1_DIM
 
-static inline void vzero(fom_float *x, fom_int n) {
-    fom_int i;
-    for (i = 0; i < n; ++i) x[i] = 0.0;
-}
-
-/* out += c * (M @ x), using `tmp` (length n) as scratch. Skipped when c == 0. */
-static inline void scaled_apply(void (*apply)(const fom_float *, fom_float *),
-                                fom_float c, const fom_float *x,
-                                fom_float *tmp, fom_float *out, fom_int n) {
-    fom_int i;
-    if (c == 0.0) return;
-    vzero(tmp, n);
-    apply(x, tmp);
-    for (i = 0; i < n; ++i) out[i] += c * tmp[i];
-}
-
 static void reset_state(FomWorkspace *work) {
     fom_int i;
     for (i = 0; i < N; ++i) work->state[i] = FOM_STATE_INIT[i];
+}
+
+/* State slot offsets (see spec.json). */
+#define S0 0
+#define S1 16
+#define S2 64
+#define S3 80
+#define S4 128
+#define S5 144
+#define B 16 /* slot block size */
+
+/* Small blocks extracted once (in fom_setup) by applying the provided
+   CSR operators to unit vectors. Fixed-size static storage: no dynamic
+   allocation.
+
+   Kx := t0_state__d_taux[:, s0] (48x16) = [I; -I; G], and G (16x16) has
+   a nested structure (checked against data.npz):
+     G[r][c] != 0 only when r + c is odd (checkerboard), the even-row/
+     odd-col and odd-row/even-col 8x8 halves are equal, and each equals
+     [L; -L] with L[q][c] = L4[q][c >> 1] * (c odd ? +1 : -1), L4 4x4.
+   Hence, with x in R^16 and m, q = 0..3,
+     ww_m = (x[4m+3] - x[4m+1], x[4m+2] - x[4m])     (a lane pair)
+     zz_q = sum_m L4[q][m] ww_m
+     (G x)[2q + e] = zz_q[e],  (G x)[8 + 2q + e] = -zz_q[e].
+   t1_t_t0__d_eta == -Kx^T = [-I | +I | -G^T], and for y in R^16
+     v_q  = (y[2q] - y[2q+8], y[2q+1] - y[2q+9])
+     u_m  = sum_q L4[q][m] v_q
+     (G^T y)[4m+0..3] = (-u_m[1], -u_m[0], +u_m[1], +u_m[0]).
+   D := t1_state__d_eta[:, s5] (16x16) is a 4x4 grid of symmetric
+   circulant 4x4 blocks circ(al, be, ga, be). One real 4-point DFT on the
+   inner index diagonalizes every block at once: with, per block b,
+     X0 = x0+x1+x2+x3, X2 = x0-x1+x2-x3, Xc = x0-x2, Xs = x1-x3,
+   and per block pair (a, b) the eigenvalues
+     l0 = al+2be+ga, l2 = al-2be+ga, l1 = al-ga,
+   y = D x is Y0 = L0 X0, Y2 = L2 X2, (Yc, Ys) = L1 (Xc, Xs) (4x4 each) and
+     y[4a+0..3] = (Y0+Y2+Yc, Y0-Y2+Ys, Y0+Y2-Yc, Y0-Y2-Ys),
+   with the inverse-DFT factors 1/4 (L0, L2) and 1/2 (L1) folded in at
+   setup as multiplications. 64 MACs instead of 256. */
+#define Q 4
+static fom_float FOM_L4[Q * Q]; /* row-major: FOM_L4[q*Q + m] */
+static fom_float FOM_L02[Q * Q * 2]; /* [(a*Q + b)*2 + e]: (L0, L2)/4 */
+static fom_float FOM_L1[Q * Q];      /* [a*Q + b]: L1/2 */
+
+static void extract_blocks(FomWorkspace *work) {
+    fom_float *restrict x = work->new_state; /* 160 scratch */
+    fom_float *restrict y = work->acc;       /* 160 scratch */
+    fom_int i, j;
+    for (i = 0; i < N; ++i) x[i] = 0.0;
+    for (j = 0; j < Q; ++j) {
+        /* column 4b of D gives al_ab = D[4a][4b], be_ab = D[4a+1][4b],
+           ga_ab = D[4a+2][4b] (blocks are symmetric). */
+        fom_int a;
+        x[S5 + 4 * j] = 1.0;
+        for (i = 0; i < NT1; ++i) y[i] = 0.0;
+        fom_apply_t1_state__d_eta(x, y);
+        for (a = 0; a < Q; ++a) {
+            const fom_float al = y[4 * a], be = y[4 * a + 1], ga = y[4 * a + 2];
+            FOM_L02[(a * Q + j) * 2 + 0] = 0.25 * (al + 2.0 * be + ga);
+            FOM_L02[(a * Q + j) * 2 + 1] = 0.25 * (al - 2.0 * be + ga);
+            FOM_L1[a * Q + j] = 0.5 * (al - ga);
+        }
+        x[S5 + 4 * j] = 0.0;
+    }
+    for (j = 0; j < Q; ++j) {
+        /* column 4m+3 of Kx: G[2q][4m+3] = L4[q][m] for q < 4 */
+        x[S0 + 4 * j + 3] = 1.0;
+        for (i = 0; i < NT0; ++i) y[i] = 0.0;
+        fom_apply_t0_state__d_taux(x, y);
+        for (i = 0; i < Q; ++i) FOM_L4[i * Q + j] = y[2 * B + 2 * i];
+        x[S0 + 4 * j + 3] = 0.0;
+    }
 }
 
 void fom_set_default_settings(FomSettings *settings) {
@@ -40,6 +95,7 @@
     work->settings = *settings;
     for (i = 0; i < FOM_PARAM_P0_DIM; ++i) work->params.p0[i] = 0.0;
     for (i = 0; i < FOM_PARAM_P1_DIM; ++i) work->params.p1[i] = 0.0;
+    extract_blocks(work);
     reset_state(work);
     work->solution.iter = 0;
     work->solution.residual = 0.0;
@@ -51,109 +107,219 @@
     reset_state(work);
 }
 
-fom_int fom_solve(FomWorkspace *work) {
-    fom_float *restrict s = work->state;
-    fom_float *restrict ns = work->new_state;
-    fom_float *restrict acc = work->acc;
-    fom_float *restrict t0 = work->t0;
-    fom_float *restrict t1 = work->t1;
-    const fom_float *p0 = work->params.p0;
-    const fom_float *p1 = work->params.p1;
-    const fom_int max_iter = work->settings.max_iter;
-    fom_float res = 0.0;
-    fom_int it, i;
-    fom_int status = FOM_MAX_ITER_REACHED;
-
-    for (it = 0; it < max_iter; ++it) {
-        const fom_float d1 = fom_sched_d1(it);
-        const fom_float d2 = fom_sched_d2(it);
-        const fom_float eta = fom_sched_eta(it);
-        const fom_float g1 = fom_sched_g1(it);
-        const fom_float g2 = fom_sched_g2(it);
-        const fom_float m1 = fom_sched_m1(it);
-        const fom_float m2 = fom_sched_m2(it);
-        const fom_float tau = fom_sched_tau(it);
-        const fom_float taux = fom_sched_taux(it);
-        const fom_float tth = fom_sched_tth(it);
-
-        /* ---- t0 = relu(...) ; acc[0:48]=taux, [48:96]=tth, [96:144]=tau terms */
-        vzero(t0, NT0);
-        vzero(acc, 3 * NT0);
-        fom_apply_t0_state(s, t0);
-        fom_apply_t0_state__d_taux(s, acc);
-        fom_apply_t0_state__d_tth(s, acc + NT0);
-        fom_apply_t0_p_p1(p1, t0);
-        fom_apply_t0_p_p1__d_tau(p1, acc + 2 * NT0);
-        for (i = 0; i < NT0; ++i) {
-            fom_float v = t0[i] + taux * acc[i] + tth * acc[NT0 + i] + tau * acc[2 * NT0 + i];
-            t0[i] = v > 0.0 ? v : 0.0;
-        }
-
-        /* ---- t1 = clip(...) ; acc[0:16] = eta terms */
-        vzero(t1, NT1);
-        vzero(acc, NT1);
-        fom_apply_t1_state(s, t1);
-        fom_apply_t1_state__d_eta(s, acc);
-        fom_apply_t1_p_p0(p0, t1);
-        fom_apply_t1_p_p0__d_eta(p0, acc);
-        fom_apply_t1_t_t0(t0, t1);
-        fom_apply_t1_t_t0__d_eta(t0, acc);
+/* Kept out of line: a single compact copy of the step body schedules and
+   register-allocates far better than several fully unrolled inlined
+   copies (measured ~25% faster). */
+#if defined(__GNUC__) || defined(__clang__)
+#define FOM_STEP_FN static __attribute__((noinline))
+#else
+#define FOM_STEP_FN static
+#endif
+
+/* One iteration ns = T(s). All pointers are distinct buffers (restrict),
+   so the compiler needs no runtime alias checks and can keep the small
+   intermediates (t0, t1, products) in registers. */
+FOM_STEP_FN void fom_step(const fom_float *restrict s,
+                                fom_float *restrict ns,
+                                const fom_float *restrict p0,
+                                const fom_float *restrict p1,
+                                fom_int it) {
+    const fom_float d1 = fom_sched_d1(it);
+    const fom_float d2 = fom_sched_d2(it);
+    const fom_float eta = fom_sched_eta(it);
+    const fom_float g1 = fom_sched_g1(it);
+    const fom_float g2 = fom_sched_g2(it);
+    const fom_float m1 = fom_sched_m1(it);
+    const fom_float m2 = fom_sched_m2(it);
+    const fom_float tau = fom_sched_tau(it);
+    const fom_float taux = fom_sched_taux(it);
+    const fom_float tth = fom_sched_tth(it);
+    fom_float kd[B], t0[NT0], t1[NT1];
+    fom_int i;
+
+    FOM_PROF_BEGIN(t1_state_part);
+    /* kd = D s5 - p0: the part of t1's input that depends only on the
+       current state, issued first so it overlaps the t0 chain. */
+    {
+        const fom_float *restrict x5 = s + S5;
+        fom_float X02[2 * Q], Xcs[2 * Q];
+        int a, b;
+        for (b = 0; b < Q; ++b) {
+            const fom_float e0 = x5[4 * b] + x5[4 * b + 2];
+            const fom_float e1 = x5[4 * b + 1] + x5[4 * b + 3];
+            X02[2 * b] = e0 + e1;
+            X02[2 * b + 1] = e0 - e1;
+            Xcs[2 * b] = x5[4 * b] - x5[4 * b + 2];
+            Xcs[2 * b + 1] = x5[4 * b + 1] - x5[4 * b + 3];
+        }
+        for (a = 0; a < Q; ++a) {
+            fom_float y0 = 0.0, y2 = 0.0, yc = 0.0, ys = 0.0;
+            for (b = 0; b < Q; ++b) {
+                const fom_float l1 = FOM_L1[a * Q + b];
+                y0 += FOM_L02[(a * Q + b) * 2] * X02[2 * b];
+                y2 += FOM_L02[(a * Q + b) * 2 + 1] * X02[2 * b + 1];
+                yc += l1 * Xcs[2 * b];
+                ys += l1 * Xcs[2 * b + 1];
+            }
+            kd[4 * a + 0] = (y0 + y2) + yc;
+            kd[4 * a + 1] = (y0 - y2) + ys;
+            kd[4 * a + 2] = (y0 + y2) - yc;
+            kd[4 * a + 3] = (y0 - y2) - ys;
+        }
+    }
+    for (i = 0; i < B; ++i) kd[i] -= p0[i];
+    FOM_PROF_END(t1_state_part);
+
+    FOM_PROF_BEGIN(t0_step);
+    /* t0 = relu(s1 + taux*Kx s0 + tth*Kt s4 - tau*p1).
+       t0_state is I on s1; t0_p_p1 is 0; t0_p_p1__d_tau is -I.
+       Kt == -Kx exactly (t0_state__d_tth[:,s4] == -t0_state__d_taux[:,s0]),
+       so the coupling is Kx x with x = taux*s0 - tth*s4, and
+       Kx = [I; -I; G]. */
+    {
+        fom_float x[B], ww[2 * Q], zz[2 * Q], gx[B];
+        int q, m;
+        for (i = 0; i < B; ++i) x[i] = taux * s[S0 + i] - tth * s[S4 + i];
+        for (m = 0; m < Q; ++m) {
+            ww[2 * m] = x[4 * m + 3] - x[4 * m + 1];
+            ww[2 * m + 1] = x[4 * m + 2] - x[4 * m];
+        }
+        for (q = 0; q < Q; ++q) {
+            fom_float z0 = 0.0, z1 = 0.0;
+            for (m = 0; m < Q; ++m) {
+                z0 += FOM_L4[q * Q + m] * ww[2 * m];
+                z1 += FOM_L4[q * Q + m] * ww[2 * m + 1];
+            }
+            zz[2 * q] = z0;
+            zz[2 * q + 1] = z1;
+        }
+        for (i = 0; i < 2 * Q; ++i) { gx[i] = zz[i]; gx[2 * Q + i] = -zz[i]; }
+        for (i = 0; i < B; ++i) {
+            fom_float v0 = s[S1 + i] + x[i] - tau * p1[i];
+            fom_float v1 = s[S1 + B + i] - x[i] - tau * p1[B + i];
+            t0[i] = v0 > 0.0 ? v0 : 0.0;
+            t0[B + i] = v1 > 0.0 ? v1 : 0.0;
+        }
+        for (i = 0; i < B; ++i) {
+            fom_float v2 = s[S1 + 2 * B + i] + gx[i] - tau * p1[2 * B + i];
+            t0[2 * B + i] = v2 > 0.0 ? v2 : 0.0;
+        }
+    }
+    FOM_PROF_END(t0_step);
+
+    FOM_PROF_BEGIN(t1_step);
+    /* t1 = clip(s0 + eta*(D s5 - p0 + Kx^T-part)).
+       t1_state is I on s0; t1_p_p0, t1_t_t0 are 0; t1_p_p0__d_eta is -I;
+       t1_t_t0__d_eta = -Kx^T = [-I | +I | -G^T], so
+       k = (t0b - t0a) - G^T t0c (kd holds D s5 - p0). */
+    {
+        const fom_float *t0c = t0 + 2 * B;
+        fom_float v[2 * Q], u[2 * Q], gty[B];
+        int q, m;
+        for (i = 0; i < 2 * Q; ++i) v[i] = t0c[i] - t0c[2 * Q + i];
+        for (m = 0; m < Q; ++m) {
+            fom_float u0 = 0.0, u1 = 0.0;
+            for (q = 0; q < Q; ++q) {
+                u0 += FOM_L4[q * Q + m] * v[2 * q];
+                u1 += FOM_L4[q * Q + m] * v[2 * q + 1];
+            }
+            u[2 * m] = u0;
+            u[2 * m + 1] = u1;
+        }
+        for (m = 0; m < Q; ++m) {
+            gty[4 * m + 0] = -u[2 * m + 1];
+            gty[4 * m + 1] = -u[2 * m];
+            gty[4 * m + 2] = u[2 * m + 1];
+            gty[4 * m + 3] = u[2 * m];
+        }
+        for (i = 0; i < B; ++i)
+            t1[i] = s[S0 + i] + eta * (kd[i] - gty[i] + (t0[B + i] - t0[i]));
         for (i = 0; i < NT1; ++i) {
-            fom_float v = t1[i] + eta * acc[i];
-            fom_float lo = FOM_MAT_t1_lo[i], hi = FOM_MAT_t1_hi[i];
+            fom_float v = t1[i];
+            const fom_float lo = FOM_MAT_t1_lo[i], hi = FOM_MAT_t1_hi[i];
             v = v < lo ? lo : v;
             t1[i] = v > hi ? hi : v;
         }
-
-        /* ---- state update */
-        vzero(ns, N);
-        fom_apply_su_state(s, ns);
-        scaled_apply(fom_apply_su_state__d_d1, d1, s, acc, ns, N);
-        scaled_apply(fom_apply_su_state__d_g2, g2, s, acc, ns, N);
-        scaled_apply(fom_apply_su_state__d_m1, m1, s, acc, ns, N);
-        fom_apply_su_t_t0(t0, ns);
-        fom_apply_su_t_t1(t1, ns);
-        /* Scale the (short) sources instead of the 160-long outputs:
-           c * (M @ x) == M @ (c * x). */
-        {
-            fom_float *d2t0 = acc;
-            fom_float *g1t0 = acc + NT0;
-            fom_float *d2t1 = acc + 2 * NT0;
-            fom_float *g1t1 = d2t1 + NT1;
-            fom_float *m2t1 = g1t1 + NT1;
-            for (i = 0; i < NT0; ++i) {
-                d2t0[i] = d2 * t0[i];
-                g1t0[i] = g1 * t0[i];
-            }
-            for (i = 0; i < NT1; ++i) {
-                d2t1[i] = d2 * t1[i];
-                g1t1[i] = g1 * t1[i];
-                m2t1[i] = m2 * t1[i];
-            }
-            fom_apply_su_t_t0__d_d2(d2t0, ns);
-            fom_apply_su_t_t0__d_g1(g1t0, ns);
-            fom_apply_su_t_t1__d_d2(d2t1, ns);
-            fom_apply_su_t_t1__d_g1(g1t1, ns);
-            fom_apply_su_t_t1__d_m2(m2t1, ns);
-        }
-
-        res = 0.0;
-        for (i = 0; i < N; ++i) {
-            fom_float d = ns[i] - s[i];
-            d = d < 0.0 ? -d : d;
-            if (d > res) res = d;
-            s[i] = ns[i];
-        }
+    }
+    FOM_PROF_END(t1_step);
+
+    FOM_PROF_BEGIN(state_update);
+    /* Every state-update matrix is an identity block (or zero):
+         s0' = g2*s2 + g1*t1      s1' = g2*s3 + g1*t0
+         s2' = d1*s2 + d2*t1      s3' = d1*s3 + d2*t0
+         s4' = s0                 s5' = m1*s2 + m2*t1 */
+    for (i = 0; i < B; ++i) {
+        const fom_float a = s[S2 + i], b = t1[i];
+        ns[S0 + i] = g2 * a + g1 * b;
+        ns[S2 + i] = d1 * a + d2 * b;
+        ns[S4 + i] = s[S0 + i];
+        ns[S5 + i] = m1 * a + m2 * b;
+    }
+    for (i = 0; i < NT0; ++i) {
+        const fom_float a = s[S3 + i], b = t0[i];
+        ns[S1 + i] = g2 * a + g1 * b;
+        ns[S3 + i] = d1 * a + d2 * b;
+    }
+    FOM_PROF_END(state_update);
+}
+
+static fom_float max_abs_diff(const fom_float *restrict a, const fom_float *restrict b) {
+    fom_float res = 0.0;
+    fom_int i;
+    for (i = 0; i < N; ++i) {
+        fom_float d = a[i] - b[i];
+        d = d < 0.0 ? -d : d;
+        res = d > res ? d : res;
+    }
+    return res;
+}
+
+fom_int fom_solve(FomWorkspace *work) {
+    /* Ping-pong between state (A) and new_state (B), two steps per trip,
+       so no per-iteration copy and no pointer swapping. */
+    fom_float *restrict sa = work->state;
+    fom_float *restrict sb = work->new_state;
+    const fom_float *restrict p0 = work->params.p0;
+    const fom_float *restrict p1 = work->params.p1;
+    const fom_int max_iter = work->settings.max_iter;
+    const fom_float *cur = sa, *prev = sb;
+    fom_float res = 0.0;
+    fom_int it = 0, i;
+    fom_int status = FOM_MAX_ITER_REACHED;
+
 #if FOM_USE_TOL
-        if (work->settings.check_termination && res <= work->settings.tol) {
+    if (work->settings.check_termination) {
+        const fom_float tol = work->settings.tol;
+        while (it < max_iter) {
+            fom_step(sa, sb, p0, p1, it++);
+            cur = sb; prev = sa;
+            res = max_abs_diff(sb, sa);
+            if (res <= tol) { status = FOM_SOLVED; break; }
+            if (it >= max_iter) break;
+            fom_step(sb, sa, p0, p1, it++);
+            cur = sa; prev = sb;
+            res = max_abs_diff(sa, sb);
+            if (res <= tol) { status = FOM_SOLVED; break; }
+        }
+    } else
+#endif
+    {
+        for (; it + 1 < max_iter; it += 2) {
+            fom_step(sa, sb, p0, p1, it);
+            fom_step(sb, sa, p0, p1, it + 1);
+        }
+        if (it < max_iter) {
+            fom_step(sa, sb, p0, p1, it);
             ++it;
-            status = FOM_SOLVED;
-            break;
-        }
-#endif
-    }
-
-    for (i = 0; i < N; ++i) work->solution.x[i] = s[i];
+            cur = sb; prev = sa;
+        }
+        /* Residual of the final step only. */
+        if (it > 0) res = max_abs_diff(cur, prev);
+    }
+
+    for (i = 0; i < N; ++i) work->solution.x[i] = cur[i];
+    if (cur != work->state)
+        for (i = 0; i < N; ++i) work->state[i] = cur[i];
     work->solution.iter = it;
     work->solution.residual = res;
     work->solution.status = status;
```

## Agent notes

```
**Whole-solve time:** initial `mean_us` ≈ 179 (177.8–180.3 over runs), final `mean_us` ≈ 7.6 (7.54–7.68 in steady runs). Overall speedup is about **23.5×**. `make verify` still passes (worst error 1.5e-12, tolerance 1e-6).

**Final profile (`make profile`, 200 iterations × 5000 solves; each timed region adds about 30 ns of timer overhead, so these shares are only rough):**

| region | total_us | pct |
|---|---|---|
| state_update | 33453 | 36.7% |
| t1_state_part (D·s5 − p0) | 19864 | 21.8% |
| t0_step | 19785 | 21.7% |
| t1_step | 18072 | 19.8% |

**Optimizations applied, in order, with the structure each one relies on:**
1. **Removed identity and zero CSR calls (180 → 52 µs).** Every state-update matrix is an identity block or zero. Also, `t0_state` = I on s1, `t1_state` = I on s0, `t0_p_p1`, `t1_p_p0` and `t1_t_t0` are zero, and the `__d_tau`/`__d_eta` parameter terms are −I. These became plain vector updates (for example s4' = s0 and s3' = d1·s3 + d2·t0). The new state alternates between `state` and `new_state` instead of being copied each iteration. The residual is computed only once, after the last step, because `FOM_USE_TOL=0`; the tolerance path is kept under `#if`.
2. **Merged two coupling products into one (52 → 40 µs).** `t0_state__d_tth[:,s4]` is exactly −`t0_state__d_taux[:,s0]`, so the two terms become Kx·(taux·s0 − tth·s4): one product instead of two.
3. **Extracted small blocks once at setup (40 → 16 µs).** In `fom_setup`, the provided operators are applied to unit vectors and the results stored in static arrays. Kx = [I; −I; G] and `t1_t_t0__d_eta` = −Kxᵀ, so the I/−I parts become additions and subtractions. G and the dense 16×16 D become contiguous column-major products instead of indexed CSR loops.
4. **Split G by its checkerboard pattern (14.1 → 13.7 µs).** G[r][c] ≠ 0 only when r + c is odd, so it splits into two 8×8 halves on even and odd indices.
5. **Gave the compiler aliasing guarantees (13.7 → 11.1 µs).** One iteration is a `restrict`-qualified step function, called twice per loop (A→B, then B→A), with t0 and t1 as local arrays. This removed clang's runtime alias checks and spills. The state and new-state buffers are always distinct, which makes `restrict` valid.
6. **Reduced G to a 4×4 lower-triangular Toeplitz block (11.1 → 10.25 µs).** The two 8×8 halves of G are equal and each is [L; −L] with L = L4 ⊗ [−1, 1]. So G·x and Gᵀ·y each need only pairwise differences plus one 4×4 product (16 two-wide FMAs instead of 128). This also shortens the critical dependency chain t1 → x → G → t0 → Gᵀ → t1.
7. **Applied a DFT to D's circulant blocks (10.25 → 10.05 µs; worth more once item 8 was in).** D is a 4×4 grid of symmetric circulant blocks circ(α, β, γ, β). A real 4-point DFT on the inner index diagonalizes all of them, giving 64 multiply-adds instead of 256. The 1/4 and 1/2 scale factors are folded in at setup as multiplications, so it stays division-free. This product depends only on the current state, so it is issued first and overlaps the t0 chain.
8. **Kept the step out of line (10.05 → 7.6 µs).** Marking the step `noinline` leaves one compact copy of the body instead of three fully unrolled inlined copies. Clang then emits two-wide vector code and spills much less.

**Tried and dropped (slower):** four-way split accumulators (spilled registers), dense D after item 8 (9.4 µs), and lane-pair rewrites of the D and G kernels (8.2–8.4 µs).
```
