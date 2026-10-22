#include <stdio.h>
#include <math.h>
#include "fom_solver.h"
#include "fom_ops.h"
#include "ops_ref.h"
#include "verify_data.h"

/* Activation bounds far outside any iterate: the activation is off. */
#define FOM_VERIFY_HUGE ((fom_float)(sizeof(fom_float) == 4 ? 1e30 : 1e300))

int main(void) {
    FomWorkspace work;
    FomSettings settings;
    FomParams params;
    int s, j;
    fom_float tol = (fom_float)9.9999999999999995e-07;
    fom_float ops_tol = (fom_float)1e-10;
    fom_float worst = 0.0;
    int failures = 0;

    {
        /* fom_apply_t0_state: direct check against its matrix. */
        static fom_float x[100], got[50], ref[50];
        fom_float worst_op = 0.0;
        int r, i, c;
        for (r = 0; r < 102; ++r) {
            fom_float err = 0.0, scale = 1.0;
            int bad = 0;
            for (c = 0; c < 100; ++c)
                x[c] = (r < 100) ? (fom_float)(c == r) : FOM_VERIFY_OP_T0_STATE_X[r - 100][c];
            for (i = 0; i < 50; ++i) got[i] = ref[i] = (fom_float)(i % 7 - 3);
            fom_apply_t0_state(x, got);
            fom_ref_apply_t0_state(x, ref);
            for (i = 0; i < 50; ++i) {
                fom_float d = got[i] - ref[i];
                fom_float a = ref[i] - (fom_float)(i % 7 - 3);  /* (M x)[i] */
                if (d < 0) d = -d;
                if (a < 0) a = -a;
                if (!(d <= err)) { err = d; bad = i; }  /* NaN-safe */
                if (1.0 + a > scale) scale = 1.0 + a;
            }
            if (!(err <= worst_op)) worst_op = err;
            if (!(err <= ops_tol * scale)) {
                if (r < 100)
                    printf("VERIFY FAILED: fom_apply_t0_state entry (%d, %d) is "
                           "off by %.3e from its matrix\n", bad, r, (double)err);
                else
                    printf("VERIFY FAILED: fom_apply_t0_state differs from its "
                           "matrix by %.3e in row %d on random probe %d\n",
                           (double)err, bad, r - 100);
                return 1;
            }
        }
        printf("OPSCHECK name=t0_state mode=basis probes=102 max_err=%.3e\n", (double)worst_op);
    }
    {
        /* fom_apply_t0_state__d_gamma: direct check against its matrix. */
        static fom_float x[100], got[50], ref[50];
        fom_float worst_op = 0.0;
        int r, i, c;
        for (r = 0; r < 102; ++r) {
            fom_float err = 0.0, scale = 1.0;
            int bad = 0;
            for (c = 0; c < 100; ++c)
                x[c] = (r < 100) ? (fom_float)(c == r) : FOM_VERIFY_OP_T0_STATE__D_GAMMA_X[r - 100][c];
            for (i = 0; i < 50; ++i) got[i] = ref[i] = (fom_float)(i % 7 - 3);
            fom_apply_t0_state__d_gamma(x, got);
            fom_ref_apply_t0_state__d_gamma(x, ref);
            for (i = 0; i < 50; ++i) {
                fom_float d = got[i] - ref[i];
                fom_float a = ref[i] - (fom_float)(i % 7 - 3);  /* (M x)[i] */
                if (d < 0) d = -d;
                if (a < 0) a = -a;
                if (!(d <= err)) { err = d; bad = i; }  /* NaN-safe */
                if (1.0 + a > scale) scale = 1.0 + a;
            }
            if (!(err <= worst_op)) worst_op = err;
            if (!(err <= ops_tol * scale)) {
                if (r < 100)
                    printf("VERIFY FAILED: fom_apply_t0_state__d_gamma entry (%d, %d) is "
                           "off by %.3e from its matrix\n", bad, r, (double)err);
                else
                    printf("VERIFY FAILED: fom_apply_t0_state__d_gamma differs from its "
                           "matrix by %.3e in row %d on random probe %d\n",
                           (double)err, bad, r - 100);
                return 1;
            }
        }
        printf("OPSCHECK name=t0_state__d_gamma mode=basis probes=102 max_err=%.3e\n", (double)worst_op);
    }
    {
        /* fom_apply_t0_p_p0: direct check against its matrix. */
        static fom_float x[25], got[50], ref[50];
        fom_float worst_op = 0.0;
        int r, i, c;
        for (r = 0; r < 27; ++r) {
            fom_float err = 0.0, scale = 1.0;
            int bad = 0;
            for (c = 0; c < 25; ++c)
                x[c] = (r < 25) ? (fom_float)(c == r) : FOM_VERIFY_OP_T0_P_P0_X[r - 25][c];
            for (i = 0; i < 50; ++i) got[i] = ref[i] = (fom_float)(i % 7 - 3);
            fom_apply_t0_p_p0(x, got);
            fom_ref_apply_t0_p_p0(x, ref);
            for (i = 0; i < 50; ++i) {
                fom_float d = got[i] - ref[i];
                fom_float a = ref[i] - (fom_float)(i % 7 - 3);  /* (M x)[i] */
                if (d < 0) d = -d;
                if (a < 0) a = -a;
                if (!(d <= err)) { err = d; bad = i; }  /* NaN-safe */
                if (1.0 + a > scale) scale = 1.0 + a;
            }
            if (!(err <= worst_op)) worst_op = err;
            if (!(err <= ops_tol * scale)) {
                if (r < 25)
                    printf("VERIFY FAILED: fom_apply_t0_p_p0 entry (%d, %d) is "
                           "off by %.3e from its matrix\n", bad, r, (double)err);
                else
                    printf("VERIFY FAILED: fom_apply_t0_p_p0 differs from its "
                           "matrix by %.3e in row %d on random probe %d\n",
                           (double)err, bad, r - 25);
                return 1;
            }
        }
        printf("OPSCHECK name=t0_p_p0 mode=basis probes=27 max_err=%.3e\n", (double)worst_op);
    }
    {
        /* fom_apply_su_state: direct check against its matrix. */
        static fom_float x[100], got[100], ref[100];
        fom_float worst_op = 0.0;
        int r, i, c;
        for (r = 0; r < 102; ++r) {
            fom_float err = 0.0, scale = 1.0;
            int bad = 0;
            for (c = 0; c < 100; ++c)
                x[c] = (r < 100) ? (fom_float)(c == r) : FOM_VERIFY_OP_SU_STATE_X[r - 100][c];
            for (i = 0; i < 100; ++i) got[i] = ref[i] = (fom_float)(i % 7 - 3);
            fom_apply_su_state(x, got);
            fom_ref_apply_su_state(x, ref);
            for (i = 0; i < 100; ++i) {
                fom_float d = got[i] - ref[i];
                fom_float a = ref[i] - (fom_float)(i % 7 - 3);  /* (M x)[i] */
                if (d < 0) d = -d;
                if (a < 0) a = -a;
                if (!(d <= err)) { err = d; bad = i; }  /* NaN-safe */
                if (1.0 + a > scale) scale = 1.0 + a;
            }
            if (!(err <= worst_op)) worst_op = err;
            if (!(err <= ops_tol * scale)) {
                if (r < 100)
                    printf("VERIFY FAILED: fom_apply_su_state entry (%d, %d) is "
                           "off by %.3e from its matrix\n", bad, r, (double)err);
                else
                    printf("VERIFY FAILED: fom_apply_su_state differs from its "
                           "matrix by %.3e in row %d on random probe %d\n",
                           (double)err, bad, r - 100);
                return 1;
            }
        }
        printf("OPSCHECK name=su_state mode=basis probes=102 max_err=%.3e\n", (double)worst_op);
    }
    {
        /* fom_apply_su_t_t0: direct check against its matrix. */
        static fom_float x[50], got[100], ref[100];
        fom_float worst_op = 0.0;
        int r, i, c;
        for (r = 0; r < 52; ++r) {
            fom_float err = 0.0, scale = 1.0;
            int bad = 0;
            for (c = 0; c < 50; ++c)
                x[c] = (r < 50) ? (fom_float)(c == r) : FOM_VERIFY_OP_SU_T_T0_X[r - 50][c];
            for (i = 0; i < 100; ++i) got[i] = ref[i] = (fom_float)(i % 7 - 3);
            fom_apply_su_t_t0(x, got);
            fom_ref_apply_su_t_t0(x, ref);
            for (i = 0; i < 100; ++i) {
                fom_float d = got[i] - ref[i];
                fom_float a = ref[i] - (fom_float)(i % 7 - 3);  /* (M x)[i] */
                if (d < 0) d = -d;
                if (a < 0) a = -a;
                if (!(d <= err)) { err = d; bad = i; }  /* NaN-safe */
                if (1.0 + a > scale) scale = 1.0 + a;
            }
            if (!(err <= worst_op)) worst_op = err;
            if (!(err <= ops_tol * scale)) {
                if (r < 50)
                    printf("VERIFY FAILED: fom_apply_su_t_t0 entry (%d, %d) is "
                           "off by %.3e from its matrix\n", bad, r, (double)err);
                else
                    printf("VERIFY FAILED: fom_apply_su_t_t0 differs from its "
                           "matrix by %.3e in row %d on random probe %d\n",
                           (double)err, bad, r - 50);
                return 1;
            }
        }
        printf("OPSCHECK name=su_t_t0 mode=basis probes=52 max_err=%.3e\n", (double)worst_op);
    }
    {
        /* fom_fused_t0: direct check against its members, out += t0_state @ state + sc_gamma * (t0_state__d_gamma @ state). */
        static fom_float x0[100];
        static fom_float got[50], ref[50];
        fom_float sc[1];
        static const char *setting[] = {"all schedule values 0", "sc_gamma = 1"};
        fom_float worst_op = 0.0, err, scale;
        int r, i, c, j, bad, probes = 0;
        (void)setting; (void)j; (void)sc;
        for (j = 0; j < 100; ++j) {
          for (r = 0; r <= 1; ++r) {
            for (c = 0; c < 100; ++c) x0[c] = 0.0;
            x0[j] = 1.0;
            sc[0] = (fom_float)(r == 1);
            ++probes;
            for (i = 0; i < 50; ++i) got[i] = ref[i] = (fom_float)(i % 7 - 3);
            fom_fused_t0(x0, sc[0], got);
            fom_ref_fused_t0(x0, sc[0], ref);
            err = 0.0; scale = 1.0; bad = 0;
            for (i = 0; i < 50; ++i) {
                fom_float d = got[i] - ref[i];
                fom_float a = ref[i] - (fom_float)(i % 7 - 3);
                if (d < 0) d = -d;
                if (a < 0) a = -a;
                if (!(d <= err)) { err = d; bad = i; }  /* NaN-safe */
                if (1.0 + a > scale) scale = 1.0 + a;
            }
            if (!(err <= worst_op)) worst_op = err;
            if (!(err <= ops_tol * scale)) {
                printf("VERIFY FAILED: fom_fused_t0 entry (%d, %d) of state is off by %.3e from its matrices (%s)\n",
                       bad, j, (double)err, setting[r]);
                return 1;
            }
          }
        }
        for (r = 0; r < 2; ++r) {
            for (c = 0; c < 100; ++c) x0[c] = FOM_VERIFY_FUSED_T0_X0[r][c];
            sc[0] = FOM_VERIFY_FUSED_T0_SC[r][0];
            ++probes;
            for (i = 0; i < 50; ++i) got[i] = ref[i] = (fom_float)(i % 7 - 3);
            fom_fused_t0(x0, sc[0], got);
            fom_ref_fused_t0(x0, sc[0], ref);
            err = 0.0; scale = 1.0; bad = 0;
            for (i = 0; i < 50; ++i) {
                fom_float d = got[i] - ref[i];
                fom_float a = ref[i] - (fom_float)(i % 7 - 3);
                if (d < 0) d = -d;
                if (a < 0) a = -a;
                if (!(d <= err)) { err = d; bad = i; }  /* NaN-safe */
                if (1.0 + a > scale) scale = 1.0 + a;
            }
            if (!(err <= worst_op)) worst_op = err;
            if (!(err <= ops_tol * scale)) {
                printf("VERIFY FAILED: fom_fused_t0 differs from its matrices by %.3e in row %d on random probe %d\n", (double)err, bad, r);
                return 1;
            }
        }
        printf("OPSCHECK name=fused_t0 mode=basis probes=%d max_err=%.3e\n", probes, (double)worst_op);
    }
    {
        /* fom_fused_su: direct check against its members, out += su_state @ state + su_t_t0 @ t_t0. */
        static fom_float x0[100];
        static fom_float x1[50];
        static fom_float got[100], ref[100];
        fom_float sc[1];
        static const char *setting[] = {"all schedule values 0"};
        fom_float worst_op = 0.0, err, scale;
        int r, i, c, j, bad, probes = 0;
        (void)setting; (void)j; (void)sc;
        for (j = 0; j < 100; ++j) {
          for (r = 0; r <= 0; ++r) {
            for (c = 0; c < 100; ++c) x0[c] = 0.0;
            for (c = 0; c < 50; ++c) x1[c] = 0.0;
            x0[j] = 1.0;
            ++probes;
            for (i = 0; i < 100; ++i) got[i] = ref[i] = (fom_float)(i % 7 - 3);
            fom_fused_su(x0, x1, got);
            fom_ref_fused_su(x0, x1, ref);
            err = 0.0; scale = 1.0; bad = 0;
            for (i = 0; i < 100; ++i) {
                fom_float d = got[i] - ref[i];
                fom_float a = ref[i] - (fom_float)(i % 7 - 3);
                if (d < 0) d = -d;
                if (a < 0) a = -a;
                if (!(d <= err)) { err = d; bad = i; }  /* NaN-safe */
                if (1.0 + a > scale) scale = 1.0 + a;
            }
            if (!(err <= worst_op)) worst_op = err;
            if (!(err <= ops_tol * scale)) {
                printf("VERIFY FAILED: fom_fused_su entry (%d, %d) of state is off by %.3e from its matrices (%s)\n",
                       bad, j, (double)err, setting[r]);
                return 1;
            }
          }
        }
        for (j = 0; j < 50; ++j) {
          for (r = 0; r <= 0; ++r) {
            for (c = 0; c < 100; ++c) x0[c] = 0.0;
            for (c = 0; c < 50; ++c) x1[c] = 0.0;
            x1[j] = 1.0;
            ++probes;
            for (i = 0; i < 100; ++i) got[i] = ref[i] = (fom_float)(i % 7 - 3);
            fom_fused_su(x0, x1, got);
            fom_ref_fused_su(x0, x1, ref);
            err = 0.0; scale = 1.0; bad = 0;
            for (i = 0; i < 100; ++i) {
                fom_float d = got[i] - ref[i];
                fom_float a = ref[i] - (fom_float)(i % 7 - 3);
                if (d < 0) d = -d;
                if (a < 0) a = -a;
                if (!(d <= err)) { err = d; bad = i; }  /* NaN-safe */
                if (1.0 + a > scale) scale = 1.0 + a;
            }
            if (!(err <= worst_op)) worst_op = err;
            if (!(err <= ops_tol * scale)) {
                printf("VERIFY FAILED: fom_fused_su entry (%d, %d) of t_t0 is off by %.3e from its matrices (%s)\n",
                       bad, j, (double)err, setting[r]);
                return 1;
            }
          }
        }
        for (r = 0; r < 2; ++r) {
            for (c = 0; c < 100; ++c) x0[c] = FOM_VERIFY_FUSED_SU_X0[r][c];
            for (c = 0; c < 50; ++c) x1[c] = FOM_VERIFY_FUSED_SU_X1[r][c];
            ++probes;
            for (i = 0; i < 100; ++i) got[i] = ref[i] = (fom_float)(i % 7 - 3);
            fom_fused_su(x0, x1, got);
            fom_ref_fused_su(x0, x1, ref);
            err = 0.0; scale = 1.0; bad = 0;
            for (i = 0; i < 100; ++i) {
                fom_float d = got[i] - ref[i];
                fom_float a = ref[i] - (fom_float)(i % 7 - 3);
                if (d < 0) d = -d;
                if (a < 0) a = -a;
                if (!(d <= err)) { err = d; bad = i; }  /* NaN-safe */
                if (1.0 + a > scale) scale = 1.0 + a;
            }
            if (!(err <= worst_op)) worst_op = err;
            if (!(err <= ops_tol * scale)) {
                printf("VERIFY FAILED: fom_fused_su differs from its matrices by %.3e in row %d on random probe %d\n", (double)err, bad, r);
                return 1;
            }
        }
        printf("OPSCHECK name=fused_su mode=basis probes=%d max_err=%.3e\n", probes, (double)worst_op);
    }
    {
        /* fom_step_t0: direct check against its members and the reference activation. */
        static fom_float x0[100];
        static fom_float bias[50], got[50], ref[50], pre[50];
        static fom_float act_lo[50], act_hi[50];
        FomAct_t0 hact;
        fom_float sc[1];
        static const char *setting[] = {"all schedule values 0", "sc_gamma = 1", "activation off", "package activation", "random activation"};
        fom_float worst_op = 0.0, err, scale, m;
        int r, i, c, j, bad, touched, mode, probes = 0;
        (void)setting; (void)j; (void)sc; (void)m; (void)mode; (void)act_lo;
        (void)act_hi; (void)bias;
        for (i = 0; i < 50; ++i) pre[i] = 0.0;
        hact.lambd = 0.0;
        for (i = 0; i < 50; ++i) bias[i] = FOM_VERIFY_STEP_T0_BIAS[0][i];
        for (j = 0; j < 100; ++j) {
          for (r = 0; r <= 1; ++r) {
            for (c = 0; c < 100; ++c) x0[c] = 0.0;
            x0[j] = 1.0;
            sc[0] = (fom_float)(r == 1);
            ++probes;
            for (i = 0; i < 50; ++i) {
                got[i] = (fom_float)(i % 7) - 3.5;
                ref[i] = -(fom_float)(i % 5) - 0.25;
            }
            fom_step_t0(x0, sc[0], bias, &hact, got);
            fom_ref_step_t0(x0, sc[0], bias, &hact, ref);
            err = 0.0; scale = 1.0; bad = 0; touched = -1;
            for (i = 0; i < 50; ++i) {
                {
                    fom_float d = got[i] - ref[i];
                    fom_float a = ref[i], b = pre[i];
                    if (d < 0) d = -d;
                    if (a < 0) a = -a;
                    if (b < 0) b = -b;
                    if (b > a) a = b;
                    if (!(d <= err)) { err = d; bad = i; }  /* NaN-safe */
                    if (1.0 + a > scale) scale = 1.0 + a;
                }
            }
            if (!(err <= worst_op)) worst_op = err;
            if (touched >= 0) {
                printf("VERIFY FAILED: fom_step_t0 changed selection row %d, which it must leave untouched\n", touched);
                return 1;
            }
            if (!(err <= ops_tol * scale)) {
                printf("VERIFY FAILED: fom_step_t0 entry (%d, %d) of state is off by %.3e from its matrices (%s)\n",
                       bad, j, (double)err, setting[r]);
                return 1;
            }
          }
        }
        for (r = 0; r < 2; ++r) {
            for (c = 0; c < 100; ++c) x0[c] = FOM_VERIFY_STEP_T0_X0[r][c];
            sc[0] = FOM_VERIFY_STEP_T0_SC[r][0];
            for (i = 0; i < 50; ++i) bias[i] = FOM_VERIFY_STEP_T0_BIAS[r][i];
            hact.lambd = 0.0;
            fom_ref_step_t0(x0, sc[0], bias, &hact, pre);
            ++probes;
            for (i = 0; i < 50; ++i) {
                got[i] = (fom_float)(i % 7) - 3.5;
                ref[i] = -(fom_float)(i % 5) - 0.25;
            }
            fom_step_t0(x0, sc[0], bias, &hact, got);
            fom_ref_step_t0(x0, sc[0], bias, &hact, ref);
            err = 0.0; scale = 1.0; bad = 0; touched = -1;
            for (i = 0; i < 50; ++i) {
                {
                    fom_float d = got[i] - ref[i];
                    fom_float a = ref[i], b = pre[i];
                    if (d < 0) d = -d;
                    if (a < 0) a = -a;
                    if (b < 0) b = -b;
                    if (b > a) a = b;
                    if (!(d <= err)) { err = d; bad = i; }  /* NaN-safe */
                    if (1.0 + a > scale) scale = 1.0 + a;
                }
            }
            if (!(err <= worst_op)) worst_op = err;
            if (touched >= 0) {
                printf("VERIFY FAILED: fom_step_t0 changed selection row %d, which it must leave untouched\n", touched);
                return 1;
            }
            if (!(err <= ops_tol * scale)) {
                printf("VERIFY FAILED: fom_step_t0 differs from its reference by %.3e in row %d on random probe %d (activation off)\n",
                       (double)err, bad, r);
                return 1;
            }
        }
        for (i = 0; i < 50; ++i) pre[i] = 0.0;
        hact.lambd = 0.0;
        for (r = 0; r < 2; ++r) {
            for (c = 0; c < 100; ++c) x0[c] = FOM_VERIFY_STEP_T0_X0[r][c];
            sc[0] = FOM_VERIFY_STEP_T0_SC[r][0];
            for (i = 0; i < 50; ++i) bias[i] = FOM_VERIFY_STEP_T0_BIAS[r][i];
            hact.lambd = 0.0;
            fom_ref_step_t0(x0, sc[0], bias, &hact, pre);
            ++probes;
            for (i = 0; i < 50; ++i) {
                got[i] = (fom_float)(i % 7) - 3.5;
                ref[i] = -(fom_float)(i % 5) - 0.25;
            }
            fom_step_t0(x0, sc[0], bias, &FOM_ACT_t0, got);
            fom_ref_step_t0(x0, sc[0], bias, &FOM_ACT_t0, ref);
            err = 0.0; scale = 1.0; bad = 0; touched = -1;
            for (i = 0; i < 50; ++i) {
                {
                    fom_float d = got[i] - ref[i];
                    fom_float a = ref[i], b = pre[i];
                    if (d < 0) d = -d;
                    if (a < 0) a = -a;
                    if (b < 0) b = -b;
                    if (b > a) a = b;
                    if (!(d <= err)) { err = d; bad = i; }  /* NaN-safe */
                    if (1.0 + a > scale) scale = 1.0 + a;
                }
            }
            if (!(err <= worst_op)) worst_op = err;
            if (touched >= 0) {
                printf("VERIFY FAILED: fom_step_t0 changed selection row %d, which it must leave untouched\n", touched);
                return 1;
            }
            if (!(err <= ops_tol * scale)) {
                printf("VERIFY FAILED: fom_step_t0 differs from its reference by %.3e in row %d on random probe %d (package activation)\n",
                       (double)err, bad, r);
                return 1;
            }
        }
        for (i = 0; i < 50; ++i) pre[i] = 0.0;
        hact.lambd = 0.0;
        for (r = 0; r < 2; ++r) {
            for (c = 0; c < 100; ++c) x0[c] = FOM_VERIFY_STEP_T0_X0[r][c];
            sc[0] = FOM_VERIFY_STEP_T0_SC[r][0];
            for (i = 0; i < 50; ++i) bias[i] = FOM_VERIFY_STEP_T0_BIAS[r][i];
            hact.lambd = 0.0;
            fom_ref_step_t0(x0, sc[0], bias, &hact, pre);
            m = 0.0;
            for (i = 0; i < 50; ++i) m += ((pre[i] < 0) ? -pre[i] : pre[i]);
            m = (m > 0) ? m / 50 : 1.0;
            hact.lambd = 0.5 * m;
            ++probes;
            for (i = 0; i < 50; ++i) {
                got[i] = (fom_float)(i % 7) - 3.5;
                ref[i] = -(fom_float)(i % 5) - 0.25;
            }
            fom_step_t0(x0, sc[0], bias, &hact, got);
            fom_ref_step_t0(x0, sc[0], bias, &hact, ref);
            err = 0.0; scale = 1.0; bad = 0; touched = -1;
            for (i = 0; i < 50; ++i) {
                {
                    fom_float d = got[i] - ref[i];
                    fom_float a = ref[i], b = pre[i];
                    if (d < 0) d = -d;
                    if (a < 0) a = -a;
                    if (b < 0) b = -b;
                    if (b > a) a = b;
                    if (!(d <= err)) { err = d; bad = i; }  /* NaN-safe */
                    if (1.0 + a > scale) scale = 1.0 + a;
                }
            }
            if (!(err <= worst_op)) worst_op = err;
            if (touched >= 0) {
                printf("VERIFY FAILED: fom_step_t0 changed selection row %d, which it must leave untouched\n", touched);
                return 1;
            }
            if (!(err <= ops_tol * scale)) {
                printf("VERIFY FAILED: fom_step_t0 differs from its reference by %.3e in row %d on random probe %d (random activation)\n",
                       (double)err, bad, r);
                return 1;
            }
        }
        for (i = 0; i < 50; ++i) pre[i] = 0.0;
        hact.lambd = 0.0;
        printf("OPSCHECK name=step_t0 mode=basis probes=%d max_err=%.3e\n", probes, (double)worst_op);
    }
    {
        /* fom_step_su: direct check against its members. */
        static fom_float x0[100];
        static fom_float x1[50];
        static fom_float bias[100], got[100], ref[100], pre[100];
        static fom_float act_lo[100], act_hi[100];
        fom_float sc[1];
        static const char *setting[] = {"all schedule values 0", "random"};
        fom_float worst_op = 0.0, err, scale, m;
        int r, i, c, j, bad, touched, mode, probes = 0;
        (void)setting; (void)j; (void)sc; (void)m; (void)mode; (void)act_lo;
        (void)act_hi; (void)bias;
        for (i = 0; i < 100; ++i) pre[i] = 0.0;
        for (i = 0; i < 100; ++i) bias[i] = FOM_VERIFY_STEP_SU_BIAS[0][i];
        for (j = 0; j < 100; ++j) {
          for (r = 0; r <= 0; ++r) {
            for (c = 0; c < 100; ++c) x0[c] = 0.0;
            for (c = 0; c < 50; ++c) x1[c] = 0.0;
            x0[j] = 1.0;
            ++probes;
            for (i = 0; i < 100; ++i) {
                got[i] = (fom_float)(i % 7) - 3.5;
                ref[i] = -(fom_float)(i % 5) - 0.25;
            }
            fom_step_su(x0, x1, bias, got);
            fom_ref_step_su(x0, x1, bias, ref);
            err = 0.0; scale = 1.0; bad = 0; touched = -1;
            for (i = 0; i < 100; ++i) {
                {
                    fom_float d = got[i] - ref[i];
                    fom_float a = ref[i], b = pre[i];
                    if (d < 0) d = -d;
                    if (a < 0) a = -a;
                    if (b < 0) b = -b;
                    if (b > a) a = b;
                    if (!(d <= err)) { err = d; bad = i; }  /* NaN-safe */
                    if (1.0 + a > scale) scale = 1.0 + a;
                }
            }
            if (!(err <= worst_op)) worst_op = err;
            if (touched >= 0) {
                printf("VERIFY FAILED: fom_step_su changed selection row %d, which it must leave untouched\n", touched);
                return 1;
            }
            if (!(err <= ops_tol * scale)) {
                printf("VERIFY FAILED: fom_step_su entry (%d, %d) of state is off by %.3e from its matrices (%s)\n",
                       bad, j, (double)err, setting[r]);
                return 1;
            }
          }
        }
        for (i = 0; i < 100; ++i) bias[i] = FOM_VERIFY_STEP_SU_BIAS[0][i];
        for (j = 0; j < 50; ++j) {
          for (r = 0; r <= 0; ++r) {
            for (c = 0; c < 100; ++c) x0[c] = 0.0;
            for (c = 0; c < 50; ++c) x1[c] = 0.0;
            x1[j] = 1.0;
            ++probes;
            for (i = 0; i < 100; ++i) {
                got[i] = (fom_float)(i % 7) - 3.5;
                ref[i] = -(fom_float)(i % 5) - 0.25;
            }
            fom_step_su(x0, x1, bias, got);
            fom_ref_step_su(x0, x1, bias, ref);
            err = 0.0; scale = 1.0; bad = 0; touched = -1;
            for (i = 0; i < 100; ++i) {
                {
                    fom_float d = got[i] - ref[i];
                    fom_float a = ref[i], b = pre[i];
                    if (d < 0) d = -d;
                    if (a < 0) a = -a;
                    if (b < 0) b = -b;
                    if (b > a) a = b;
                    if (!(d <= err)) { err = d; bad = i; }  /* NaN-safe */
                    if (1.0 + a > scale) scale = 1.0 + a;
                }
            }
            if (!(err <= worst_op)) worst_op = err;
            if (touched >= 0) {
                printf("VERIFY FAILED: fom_step_su changed selection row %d, which it must leave untouched\n", touched);
                return 1;
            }
            if (!(err <= ops_tol * scale)) {
                printf("VERIFY FAILED: fom_step_su entry (%d, %d) of t_t0 is off by %.3e from its matrices (%s)\n",
                       bad, j, (double)err, setting[r]);
                return 1;
            }
          }
        }
        for (r = 0; r < 2; ++r) {
            for (c = 0; c < 100; ++c) x0[c] = FOM_VERIFY_STEP_SU_X0[r][c];
            for (c = 0; c < 50; ++c) x1[c] = FOM_VERIFY_STEP_SU_X1[r][c];
            for (i = 0; i < 100; ++i) bias[i] = FOM_VERIFY_STEP_SU_BIAS[r][i];
            ++probes;
            for (i = 0; i < 100; ++i) {
                got[i] = (fom_float)(i % 7) - 3.5;
                ref[i] = -(fom_float)(i % 5) - 0.25;
            }
            fom_step_su(x0, x1, bias, got);
            fom_ref_step_su(x0, x1, bias, ref);
            err = 0.0; scale = 1.0; bad = 0; touched = -1;
            for (i = 0; i < 100; ++i) {
                {
                    fom_float d = got[i] - ref[i];
                    fom_float a = ref[i], b = pre[i];
                    if (d < 0) d = -d;
                    if (a < 0) a = -a;
                    if (b < 0) b = -b;
                    if (b > a) a = b;
                    if (!(d <= err)) { err = d; bad = i; }  /* NaN-safe */
                    if (1.0 + a > scale) scale = 1.0 + a;
                }
            }
            if (!(err <= worst_op)) worst_op = err;
            if (touched >= 0) {
                printf("VERIFY FAILED: fom_step_su changed selection row %d, which it must leave untouched\n", touched);
                return 1;
            }
            if (!(err <= ops_tol * scale)) {
                printf("VERIFY FAILED: fom_step_su differs from its reference by %.3e in row %d on random probe %d (random)\n",
                       (double)err, bad, r);
                return 1;
            }
        }
        printf("OPSCHECK name=step_su mode=basis probes=%d max_err=%.3e\n", probes, (double)worst_op);
    }
    {
        /* fom_iterate: direct check against the chained reference steps. */
        static fom_float x[100];
        static fom_float bias_t0[50];
        static fom_float bias_su[100];
        static fom_float g_t_t0[50], r_t_t0[50];
        static fom_float g_new_state[100], r_new_state[100];
        static fom_float lo_t0[50], hi_t0[50];
        FomAct_t0 hact_t0;
        const FomAct_t0 *pact_t0 = &FOM_ACT_t0;
        fom_float sc[1];
        static const char *setting[] = {"all schedule values 0", "sc_gamma = 1", "random schedule values", "activations off", "package activations", "random activations"};
        static const char *output[] = {"t_t0", "new_state"};
        fom_float worst_op = 0.0, err, scale, m;
        int r, i, j, bad, which, probes = 0;
        (void)setting; (void)output; (void)j; (void)sc; (void)m;
        (void)pact_t0; (void)lo_t0; (void)hi_t0;
        hact_t0.lambd = 0.0;
        for (i = 0; i < 50; ++i) bias_t0[i] = 0.0;
        for (i = 0; i < 100; ++i) bias_su[i] = 0.0;
        for (j = 0; j < 100; ++j) {
          for (r = 0; r <= 2; ++r) {
            for (i = 0; i < 100; ++i) x[i] = 0.0;
            x[j] = 1.0;
            sc[0] = (r == 2) ? FOM_VERIFY_ITER_SC[2][0] : (fom_float)(r == 1);
            ++probes;
            for (i = 0; i < 50; ++i) { g_t_t0[i] = (fom_float)(i % 7) - 3.5; r_t_t0[i] = -(fom_float)(i % 5) - 0.25; }
            for (i = 0; i < 100; ++i) { g_new_state[i] = (fom_float)(i % 7) - 3.5; r_new_state[i] = -(fom_float)(i % 5) - 0.25; }
            fom_iterate(x, sc[0], bias_t0, bias_su, &hact_t0, g_t_t0, g_new_state);
            fom_ref_iterate(x, sc[0], bias_t0, bias_su, &hact_t0, r_t_t0, r_new_state);
            err = 0.0; scale = 1.0; bad = 0; which = 0;
            for (i = 0; i < 50; ++i) {
                fom_float d = g_t_t0[i] - r_t_t0[i], a = r_t_t0[i];
                if (d < 0) d = -d;
                if (a < 0) a = -a;
                if (!(d <= err)) { err = d; bad = i; which = 0; }
                if (1.0 + a > scale) scale = 1.0 + a;
            }
            for (i = 0; i < 100; ++i) {
                fom_float d = g_new_state[i] - r_new_state[i], a = r_new_state[i];
                if (d < 0) d = -d;
                if (a < 0) a = -a;
                if (!(d <= err)) { err = d; bad = i; which = 1; }
                if (1.0 + a > scale) scale = 1.0 + a;
            }
            if (!(err <= worst_op)) worst_op = err;
            if (!(err <= ops_tol * scale)) {
                printf("VERIFY FAILED: fom_iterate entry (%d, %d) of %s is off by %.3e from the reference (%s)\n",
                       bad, j, output[which], (double)err, setting[r]);
                return 1;
            }
          }
        }
        for (r = 0; r < 2; ++r) {
            for (i = 0; i < 100; ++i) x[i] = FOM_VERIFY_ITER_X[r][i];
            sc[0] = FOM_VERIFY_ITER_SC[r][0];
            for (i = 0; i < 50; ++i) bias_t0[i] = FOM_VERIFY_ITER_T0_BIAS[r][i];
            for (i = 0; i < 100; ++i) bias_su[i] = FOM_VERIFY_ITER_SU_BIAS[r][i];
            ++probes;
            for (i = 0; i < 50; ++i) { g_t_t0[i] = (fom_float)(i % 7) - 3.5; r_t_t0[i] = -(fom_float)(i % 5) - 0.25; }
            for (i = 0; i < 100; ++i) { g_new_state[i] = (fom_float)(i % 7) - 3.5; r_new_state[i] = -(fom_float)(i % 5) - 0.25; }
            fom_iterate(x, sc[0], bias_t0, bias_su, &hact_t0, g_t_t0, g_new_state);
            fom_ref_iterate(x, sc[0], bias_t0, bias_su, &hact_t0, r_t_t0, r_new_state);
            err = 0.0; scale = 1.0; bad = 0; which = 0;
            for (i = 0; i < 50; ++i) {
                fom_float d = g_t_t0[i] - r_t_t0[i], a = r_t_t0[i];
                if (d < 0) d = -d;
                if (a < 0) a = -a;
                if (!(d <= err)) { err = d; bad = i; which = 0; }
                if (1.0 + a > scale) scale = 1.0 + a;
            }
            for (i = 0; i < 100; ++i) {
                fom_float d = g_new_state[i] - r_new_state[i], a = r_new_state[i];
                if (d < 0) d = -d;
                if (a < 0) a = -a;
                if (!(d <= err)) { err = d; bad = i; which = 1; }
                if (1.0 + a > scale) scale = 1.0 + a;
            }
            if (!(err <= worst_op)) worst_op = err;
            if (!(err <= ops_tol * scale)) {
                printf("VERIFY FAILED: fom_iterate differs from the reference by %.3e in %s row %d on random probe %d (activations off)\n",
                       (double)err, output[which], bad, r);
                return 1;
            }
        }
        for (r = 0; r < 2; ++r) {
            for (i = 0; i < 100; ++i) x[i] = FOM_VERIFY_ITER_X[r][i];
            sc[0] = FOM_VERIFY_ITER_SC[r][0];
            for (i = 0; i < 50; ++i) bias_t0[i] = FOM_VERIFY_ITER_T0_BIAS[r][i];
            for (i = 0; i < 100; ++i) bias_su[i] = FOM_VERIFY_ITER_SU_BIAS[r][i];
            ++probes;
            for (i = 0; i < 50; ++i) { g_t_t0[i] = (fom_float)(i % 7) - 3.5; r_t_t0[i] = -(fom_float)(i % 5) - 0.25; }
            for (i = 0; i < 100; ++i) { g_new_state[i] = (fom_float)(i % 7) - 3.5; r_new_state[i] = -(fom_float)(i % 5) - 0.25; }
            fom_iterate(x, sc[0], bias_t0, bias_su, pact_t0, g_t_t0, g_new_state);
            fom_ref_iterate(x, sc[0], bias_t0, bias_su, pact_t0, r_t_t0, r_new_state);
            err = 0.0; scale = 1.0; bad = 0; which = 0;
            for (i = 0; i < 50; ++i) {
                fom_float d = g_t_t0[i] - r_t_t0[i], a = r_t_t0[i];
                if (d < 0) d = -d;
                if (a < 0) a = -a;
                if (!(d <= err)) { err = d; bad = i; which = 0; }
                if (1.0 + a > scale) scale = 1.0 + a;
            }
            for (i = 0; i < 100; ++i) {
                fom_float d = g_new_state[i] - r_new_state[i], a = r_new_state[i];
                if (d < 0) d = -d;
                if (a < 0) a = -a;
                if (!(d <= err)) { err = d; bad = i; which = 1; }
                if (1.0 + a > scale) scale = 1.0 + a;
            }
            if (!(err <= worst_op)) worst_op = err;
            if (!(err <= ops_tol * scale)) {
                printf("VERIFY FAILED: fom_iterate differs from the reference by %.3e in %s row %d on random probe %d (package activations)\n",
                       (double)err, output[which], bad, r);
                return 1;
            }
        }
        for (r = 0; r < 2; ++r) {
            for (i = 0; i < 100; ++i) x[i] = FOM_VERIFY_ITER_X[r][i];
            sc[0] = FOM_VERIFY_ITER_SC[r][0];
            for (i = 0; i < 50; ++i) bias_t0[i] = FOM_VERIFY_ITER_T0_BIAS[r][i];
            for (i = 0; i < 100; ++i) bias_su[i] = FOM_VERIFY_ITER_SU_BIAS[r][i];
            fom_ref_iterate(x, sc[0], bias_t0, bias_su, &hact_t0, r_t_t0, r_new_state);
            m = 0.0;
            for (i = 0; i < 50; ++i) m += ((r_t_t0[i] < 0) ? -r_t_t0[i] : r_t_t0[i]);
            m = (m > 0) ? m / 50 : 1.0;
            hact_t0.lambd = 0.5 * m;
            ++probes;
            for (i = 0; i < 50; ++i) { g_t_t0[i] = (fom_float)(i % 7) - 3.5; r_t_t0[i] = -(fom_float)(i % 5) - 0.25; }
            for (i = 0; i < 100; ++i) { g_new_state[i] = (fom_float)(i % 7) - 3.5; r_new_state[i] = -(fom_float)(i % 5) - 0.25; }
            fom_iterate(x, sc[0], bias_t0, bias_su, &hact_t0, g_t_t0, g_new_state);
            fom_ref_iterate(x, sc[0], bias_t0, bias_su, &hact_t0, r_t_t0, r_new_state);
            err = 0.0; scale = 1.0; bad = 0; which = 0;
            for (i = 0; i < 50; ++i) {
                fom_float d = g_t_t0[i] - r_t_t0[i], a = r_t_t0[i];
                if (d < 0) d = -d;
                if (a < 0) a = -a;
                if (!(d <= err)) { err = d; bad = i; which = 0; }
                if (1.0 + a > scale) scale = 1.0 + a;
            }
            for (i = 0; i < 100; ++i) {
                fom_float d = g_new_state[i] - r_new_state[i], a = r_new_state[i];
                if (d < 0) d = -d;
                if (a < 0) a = -a;
                if (!(d <= err)) { err = d; bad = i; which = 1; }
                if (1.0 + a > scale) scale = 1.0 + a;
            }
            if (!(err <= worst_op)) worst_op = err;
            if (!(err <= ops_tol * scale)) {
                printf("VERIFY FAILED: fom_iterate differs from the reference by %.3e in %s row %d on random probe %d (random activations)\n",
                       (double)err, output[which], bad, r);
                return 1;
            }
            hact_t0.lambd = 0.0;
        }
        printf("OPSCHECK name=iterate mode=basis probes=%d max_err=%.3e\n", probes, (double)worst_op);
    }

    fom_set_default_settings(&settings);
    fom_setup(&work, &settings);

    for (s = 0; s < FOM_NUM_SAMPLES; ++s) {
        for (j = 0; j < FOM_PARAM_P0_DIM; ++j)
            params.p0[j] = FOM_VERIFY_P0[s][j];
        fom_update_params(&work, &params);
        fom_solve(&work);
        fom_float err = 0.0;
        for (j = 0; j < FOM_STATE_DIM; ++j) {
            fom_float d = work.solution.x[j] - FOM_VERIFY_EXPECTED[s][j];
            if (d < 0) d = -d;
            if (!(d <= err)) err = d;  /* NaN-safe: a NaN difference sticks */
        }
        if (!(err <= worst)) worst = err;
        if (!(err <= tol)) {
            printf("sample %d: max abs error %.3e > tol %.3e\n", s, (double)err, (double)tol);
            failures++;
        }
    }
    printf("worst max-abs error over %d samples: %.3e (tol %.3e)\n",
           FOM_NUM_SAMPLES, (double)worst, (double)tol);
    if (failures) {
        printf("VERIFY FAILED: %d/%d samples exceed tol\n", failures, FOM_NUM_SAMPLES);
        return 1;
    }

    printf("VERIFY OK\n");
    return 0;
}
