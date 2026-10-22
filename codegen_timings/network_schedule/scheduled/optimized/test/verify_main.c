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
        static fom_float x[650], got[600], ref[600];
        fom_float worst_op = 0.0;
        int r, i, c;
        for (r = 0; r < 652; ++r) {
            fom_float err = 0.0, scale = 1.0;
            int bad = 0;
            for (c = 0; c < 650; ++c)
                x[c] = (r < 650) ? (fom_float)(c == r) : FOM_VERIFY_OP_T0_STATE_X[r - 650][c];
            for (i = 0; i < 600; ++i) got[i] = ref[i] = (fom_float)(i % 7 - 3);
            fom_apply_t0_state(x, got);
            fom_ref_apply_t0_state(x, ref);
            for (i = 0; i < 600; ++i) {
                fom_float d = got[i] - ref[i];
                fom_float a = ref[i] - (fom_float)(i % 7 - 3);  /* (M x)[i] */
                if (d < 0) d = -d;
                if (a < 0) a = -a;
                if (!(d <= err)) { err = d; bad = i; }  /* NaN-safe */
                if (1.0 + a > scale) scale = 1.0 + a;
            }
            if (!(err <= worst_op)) worst_op = err;
            if (!(err <= ops_tol * scale)) {
                if (r < 650)
                    printf("VERIFY FAILED: fom_apply_t0_state entry (%d, %d) is "
                           "off by %.3e from its matrix\n", bad, r, (double)err);
                else
                    printf("VERIFY FAILED: fom_apply_t0_state differs from its "
                           "matrix by %.3e in row %d on random probe %d\n",
                           (double)err, bad, r - 650);
                return 1;
            }
        }
        printf("OPSCHECK name=t0_state mode=basis probes=652 max_err=%.3e\n", (double)worst_op);
    }
    {
        /* fom_apply_t1_state: direct check against its matrix. */
        static fom_float x[650], got[20], ref[20];
        fom_float worst_op = 0.0;
        int r, i, c;
        for (r = 0; r < 652; ++r) {
            fom_float err = 0.0, scale = 1.0;
            int bad = 0;
            for (c = 0; c < 650; ++c)
                x[c] = (r < 650) ? (fom_float)(c == r) : FOM_VERIFY_OP_T1_STATE_X[r - 650][c];
            for (i = 0; i < 20; ++i) got[i] = ref[i] = (fom_float)(i % 7 - 3);
            fom_apply_t1_state(x, got);
            fom_ref_apply_t1_state(x, ref);
            for (i = 0; i < 20; ++i) {
                fom_float d = got[i] - ref[i];
                fom_float a = ref[i] - (fom_float)(i % 7 - 3);  /* (M x)[i] */
                if (d < 0) d = -d;
                if (a < 0) a = -a;
                if (!(d <= err)) { err = d; bad = i; }  /* NaN-safe */
                if (1.0 + a > scale) scale = 1.0 + a;
            }
            if (!(err <= worst_op)) worst_op = err;
            if (!(err <= ops_tol * scale)) {
                if (r < 650)
                    printf("VERIFY FAILED: fom_apply_t1_state entry (%d, %d) is "
                           "off by %.3e from its matrix\n", bad, r, (double)err);
                else
                    printf("VERIFY FAILED: fom_apply_t1_state differs from its "
                           "matrix by %.3e in row %d on random probe %d\n",
                           (double)err, bad, r - 650);
                return 1;
            }
        }
        printf("OPSCHECK name=t1_state mode=basis probes=652 max_err=%.3e\n", (double)worst_op);
    }
    {
        /* fom_apply_t1_state__d_xi: direct check against its matrix. */
        static fom_float x[650], got[20], ref[20];
        fom_float worst_op = 0.0;
        int r, i, c;
        for (r = 0; r < 652; ++r) {
            fom_float err = 0.0, scale = 1.0;
            int bad = 0;
            for (c = 0; c < 650; ++c)
                x[c] = (r < 650) ? (fom_float)(c == r) : FOM_VERIFY_OP_T1_STATE__D_XI_X[r - 650][c];
            for (i = 0; i < 20; ++i) got[i] = ref[i] = (fom_float)(i % 7 - 3);
            fom_apply_t1_state__d_xi(x, got);
            fom_ref_apply_t1_state__d_xi(x, ref);
            for (i = 0; i < 20; ++i) {
                fom_float d = got[i] - ref[i];
                fom_float a = ref[i] - (fom_float)(i % 7 - 3);  /* (M x)[i] */
                if (d < 0) d = -d;
                if (a < 0) a = -a;
                if (!(d <= err)) { err = d; bad = i; }  /* NaN-safe */
                if (1.0 + a > scale) scale = 1.0 + a;
            }
            if (!(err <= worst_op)) worst_op = err;
            if (!(err <= ops_tol * scale)) {
                if (r < 650)
                    printf("VERIFY FAILED: fom_apply_t1_state__d_xi entry (%d, %d) is "
                           "off by %.3e from its matrix\n", bad, r, (double)err);
                else
                    printf("VERIFY FAILED: fom_apply_t1_state__d_xi differs from its "
                           "matrix by %.3e in row %d on random probe %d\n",
                           (double)err, bad, r - 650);
                return 1;
            }
        }
        printf("OPSCHECK name=t1_state__d_xi mode=basis probes=652 max_err=%.3e\n", (double)worst_op);
    }
    {
        /* fom_apply_t1_t_t0: direct check against its matrix. */
        static fom_float x[600], got[20], ref[20];
        fom_float worst_op = 0.0;
        int r, i, c;
        for (r = 0; r < 602; ++r) {
            fom_float err = 0.0, scale = 1.0;
            int bad = 0;
            for (c = 0; c < 600; ++c)
                x[c] = (r < 600) ? (fom_float)(c == r) : FOM_VERIFY_OP_T1_T_T0_X[r - 600][c];
            for (i = 0; i < 20; ++i) got[i] = ref[i] = (fom_float)(i % 7 - 3);
            fom_apply_t1_t_t0(x, got);
            fom_ref_apply_t1_t_t0(x, ref);
            for (i = 0; i < 20; ++i) {
                fom_float d = got[i] - ref[i];
                fom_float a = ref[i] - (fom_float)(i % 7 - 3);  /* (M x)[i] */
                if (d < 0) d = -d;
                if (a < 0) a = -a;
                if (!(d <= err)) { err = d; bad = i; }  /* NaN-safe */
                if (1.0 + a > scale) scale = 1.0 + a;
            }
            if (!(err <= worst_op)) worst_op = err;
            if (!(err <= ops_tol * scale)) {
                if (r < 600)
                    printf("VERIFY FAILED: fom_apply_t1_t_t0 entry (%d, %d) is "
                           "off by %.3e from its matrix\n", bad, r, (double)err);
                else
                    printf("VERIFY FAILED: fom_apply_t1_t_t0 differs from its "
                           "matrix by %.3e in row %d on random probe %d\n",
                           (double)err, bad, r - 600);
                return 1;
            }
        }
        printf("OPSCHECK name=t1_t_t0 mode=basis probes=602 max_err=%.3e\n", (double)worst_op);
    }
    {
        /* fom_apply_t1_t_t0__d_xi: direct check against its matrix. */
        static fom_float x[600], got[20], ref[20];
        fom_float worst_op = 0.0;
        int r, i, c;
        for (r = 0; r < 602; ++r) {
            fom_float err = 0.0, scale = 1.0;
            int bad = 0;
            for (c = 0; c < 600; ++c)
                x[c] = (r < 600) ? (fom_float)(c == r) : FOM_VERIFY_OP_T1_T_T0__D_XI_X[r - 600][c];
            for (i = 0; i < 20; ++i) got[i] = ref[i] = (fom_float)(i % 7 - 3);
            fom_apply_t1_t_t0__d_xi(x, got);
            fom_ref_apply_t1_t_t0__d_xi(x, ref);
            for (i = 0; i < 20; ++i) {
                fom_float d = got[i] - ref[i];
                fom_float a = ref[i] - (fom_float)(i % 7 - 3);  /* (M x)[i] */
                if (d < 0) d = -d;
                if (a < 0) a = -a;
                if (!(d <= err)) { err = d; bad = i; }  /* NaN-safe */
                if (1.0 + a > scale) scale = 1.0 + a;
            }
            if (!(err <= worst_op)) worst_op = err;
            if (!(err <= ops_tol * scale)) {
                if (r < 600)
                    printf("VERIFY FAILED: fom_apply_t1_t_t0__d_xi entry (%d, %d) is "
                           "off by %.3e from its matrix\n", bad, r, (double)err);
                else
                    printf("VERIFY FAILED: fom_apply_t1_t_t0__d_xi differs from its "
                           "matrix by %.3e in row %d on random probe %d\n",
                           (double)err, bad, r - 600);
                return 1;
            }
        }
        printf("OPSCHECK name=t1_t_t0__d_xi mode=basis probes=602 max_err=%.3e\n", (double)worst_op);
    }
    {
        /* fom_apply_su_state: direct check against its matrix. */
        static fom_float x[650], got[650], ref[650];
        fom_float worst_op = 0.0;
        int r, i, c;
        for (r = 0; r < 652; ++r) {
            fom_float err = 0.0, scale = 1.0;
            int bad = 0;
            for (c = 0; c < 650; ++c)
                x[c] = (r < 650) ? (fom_float)(c == r) : FOM_VERIFY_OP_SU_STATE_X[r - 650][c];
            for (i = 0; i < 650; ++i) got[i] = ref[i] = (fom_float)(i % 7 - 3);
            fom_apply_su_state(x, got);
            fom_ref_apply_su_state(x, ref);
            for (i = 0; i < 650; ++i) {
                fom_float d = got[i] - ref[i];
                fom_float a = ref[i] - (fom_float)(i % 7 - 3);  /* (M x)[i] */
                if (d < 0) d = -d;
                if (a < 0) a = -a;
                if (!(d <= err)) { err = d; bad = i; }  /* NaN-safe */
                if (1.0 + a > scale) scale = 1.0 + a;
            }
            if (!(err <= worst_op)) worst_op = err;
            if (!(err <= ops_tol * scale)) {
                if (r < 650)
                    printf("VERIFY FAILED: fom_apply_su_state entry (%d, %d) is "
                           "off by %.3e from its matrix\n", bad, r, (double)err);
                else
                    printf("VERIFY FAILED: fom_apply_su_state differs from its "
                           "matrix by %.3e in row %d on random probe %d\n",
                           (double)err, bad, r - 650);
                return 1;
            }
        }
        printf("OPSCHECK name=su_state mode=basis probes=652 max_err=%.3e\n", (double)worst_op);
    }
    {
        /* fom_apply_su_state__d_xi: direct check against its matrix. */
        static fom_float x[650], got[650], ref[650];
        fom_float worst_op = 0.0;
        int r, i, c;
        for (r = 0; r < 652; ++r) {
            fom_float err = 0.0, scale = 1.0;
            int bad = 0;
            for (c = 0; c < 650; ++c)
                x[c] = (r < 650) ? (fom_float)(c == r) : FOM_VERIFY_OP_SU_STATE__D_XI_X[r - 650][c];
            for (i = 0; i < 650; ++i) got[i] = ref[i] = (fom_float)(i % 7 - 3);
            fom_apply_su_state__d_xi(x, got);
            fom_ref_apply_su_state__d_xi(x, ref);
            for (i = 0; i < 650; ++i) {
                fom_float d = got[i] - ref[i];
                fom_float a = ref[i] - (fom_float)(i % 7 - 3);  /* (M x)[i] */
                if (d < 0) d = -d;
                if (a < 0) a = -a;
                if (!(d <= err)) { err = d; bad = i; }  /* NaN-safe */
                if (1.0 + a > scale) scale = 1.0 + a;
            }
            if (!(err <= worst_op)) worst_op = err;
            if (!(err <= ops_tol * scale)) {
                if (r < 650)
                    printf("VERIFY FAILED: fom_apply_su_state__d_xi entry (%d, %d) is "
                           "off by %.3e from its matrix\n", bad, r, (double)err);
                else
                    printf("VERIFY FAILED: fom_apply_su_state__d_xi differs from its "
                           "matrix by %.3e in row %d on random probe %d\n",
                           (double)err, bad, r - 650);
                return 1;
            }
        }
        printf("OPSCHECK name=su_state__d_xi mode=basis probes=652 max_err=%.3e\n", (double)worst_op);
    }
    {
        /* fom_apply_su_t_t0: direct check against its matrix. */
        static fom_float x[600], got[650], ref[650];
        fom_float worst_op = 0.0;
        int r, i, c;
        for (r = 0; r < 602; ++r) {
            fom_float err = 0.0, scale = 1.0;
            int bad = 0;
            for (c = 0; c < 600; ++c)
                x[c] = (r < 600) ? (fom_float)(c == r) : FOM_VERIFY_OP_SU_T_T0_X[r - 600][c];
            for (i = 0; i < 650; ++i) got[i] = ref[i] = (fom_float)(i % 7 - 3);
            fom_apply_su_t_t0(x, got);
            fom_ref_apply_su_t_t0(x, ref);
            for (i = 0; i < 650; ++i) {
                fom_float d = got[i] - ref[i];
                fom_float a = ref[i] - (fom_float)(i % 7 - 3);  /* (M x)[i] */
                if (d < 0) d = -d;
                if (a < 0) a = -a;
                if (!(d <= err)) { err = d; bad = i; }  /* NaN-safe */
                if (1.0 + a > scale) scale = 1.0 + a;
            }
            if (!(err <= worst_op)) worst_op = err;
            if (!(err <= ops_tol * scale)) {
                if (r < 600)
                    printf("VERIFY FAILED: fom_apply_su_t_t0 entry (%d, %d) is "
                           "off by %.3e from its matrix\n", bad, r, (double)err);
                else
                    printf("VERIFY FAILED: fom_apply_su_t_t0 differs from its "
                           "matrix by %.3e in row %d on random probe %d\n",
                           (double)err, bad, r - 600);
                return 1;
            }
        }
        printf("OPSCHECK name=su_t_t0 mode=basis probes=602 max_err=%.3e\n", (double)worst_op);
    }
    {
        /* fom_apply_su_t_t0__d_xi: direct check against its matrix. */
        static fom_float x[600], got[650], ref[650];
        fom_float worst_op = 0.0;
        int r, i, c;
        for (r = 0; r < 602; ++r) {
            fom_float err = 0.0, scale = 1.0;
            int bad = 0;
            for (c = 0; c < 600; ++c)
                x[c] = (r < 600) ? (fom_float)(c == r) : FOM_VERIFY_OP_SU_T_T0__D_XI_X[r - 600][c];
            for (i = 0; i < 650; ++i) got[i] = ref[i] = (fom_float)(i % 7 - 3);
            fom_apply_su_t_t0__d_xi(x, got);
            fom_ref_apply_su_t_t0__d_xi(x, ref);
            for (i = 0; i < 650; ++i) {
                fom_float d = got[i] - ref[i];
                fom_float a = ref[i] - (fom_float)(i % 7 - 3);  /* (M x)[i] */
                if (d < 0) d = -d;
                if (a < 0) a = -a;
                if (!(d <= err)) { err = d; bad = i; }  /* NaN-safe */
                if (1.0 + a > scale) scale = 1.0 + a;
            }
            if (!(err <= worst_op)) worst_op = err;
            if (!(err <= ops_tol * scale)) {
                if (r < 600)
                    printf("VERIFY FAILED: fom_apply_su_t_t0__d_xi entry (%d, %d) is "
                           "off by %.3e from its matrix\n", bad, r, (double)err);
                else
                    printf("VERIFY FAILED: fom_apply_su_t_t0__d_xi differs from its "
                           "matrix by %.3e in row %d on random probe %d\n",
                           (double)err, bad, r - 600);
                return 1;
            }
        }
        printf("OPSCHECK name=su_t_t0__d_xi mode=basis probes=602 max_err=%.3e\n", (double)worst_op);
    }
    {
        /* fom_apply_su_t_t1: direct check against its matrix. */
        static fom_float x[20], got[650], ref[650];
        fom_float worst_op = 0.0;
        int r, i, c;
        for (r = 0; r < 22; ++r) {
            fom_float err = 0.0, scale = 1.0;
            int bad = 0;
            for (c = 0; c < 20; ++c)
                x[c] = (r < 20) ? (fom_float)(c == r) : FOM_VERIFY_OP_SU_T_T1_X[r - 20][c];
            for (i = 0; i < 650; ++i) got[i] = ref[i] = (fom_float)(i % 7 - 3);
            fom_apply_su_t_t1(x, got);
            fom_ref_apply_su_t_t1(x, ref);
            for (i = 0; i < 650; ++i) {
                fom_float d = got[i] - ref[i];
                fom_float a = ref[i] - (fom_float)(i % 7 - 3);  /* (M x)[i] */
                if (d < 0) d = -d;
                if (a < 0) a = -a;
                if (!(d <= err)) { err = d; bad = i; }  /* NaN-safe */
                if (1.0 + a > scale) scale = 1.0 + a;
            }
            if (!(err <= worst_op)) worst_op = err;
            if (!(err <= ops_tol * scale)) {
                if (r < 20)
                    printf("VERIFY FAILED: fom_apply_su_t_t1 entry (%d, %d) is "
                           "off by %.3e from its matrix\n", bad, r, (double)err);
                else
                    printf("VERIFY FAILED: fom_apply_su_t_t1 differs from its "
                           "matrix by %.3e in row %d on random probe %d\n",
                           (double)err, bad, r - 20);
                return 1;
            }
        }
        printf("OPSCHECK name=su_t_t1 mode=basis probes=22 max_err=%.3e\n", (double)worst_op);
    }
    {
        /* fom_apply_su_p_p0: direct check against its matrix. */
        static fom_float x[30], got[650], ref[650];
        fom_float worst_op = 0.0;
        int r, i, c;
        for (r = 0; r < 32; ++r) {
            fom_float err = 0.0, scale = 1.0;
            int bad = 0;
            for (c = 0; c < 30; ++c)
                x[c] = (r < 30) ? (fom_float)(c == r) : FOM_VERIFY_OP_SU_P_P0_X[r - 30][c];
            for (i = 0; i < 650; ++i) got[i] = ref[i] = (fom_float)(i % 7 - 3);
            fom_apply_su_p_p0(x, got);
            fom_ref_apply_su_p_p0(x, ref);
            for (i = 0; i < 650; ++i) {
                fom_float d = got[i] - ref[i];
                fom_float a = ref[i] - (fom_float)(i % 7 - 3);  /* (M x)[i] */
                if (d < 0) d = -d;
                if (a < 0) a = -a;
                if (!(d <= err)) { err = d; bad = i; }  /* NaN-safe */
                if (1.0 + a > scale) scale = 1.0 + a;
            }
            if (!(err <= worst_op)) worst_op = err;
            if (!(err <= ops_tol * scale)) {
                if (r < 30)
                    printf("VERIFY FAILED: fom_apply_su_p_p0 entry (%d, %d) is "
                           "off by %.3e from its matrix\n", bad, r, (double)err);
                else
                    printf("VERIFY FAILED: fom_apply_su_p_p0 differs from its "
                           "matrix by %.3e in row %d on random probe %d\n",
                           (double)err, bad, r - 30);
                return 1;
            }
        }
        printf("OPSCHECK name=su_p_p0 mode=basis probes=32 max_err=%.3e\n", (double)worst_op);
    }
    {
        /* fom_fused_t1: direct check against its members, out += t1_state @ state + sc_xi * (t1_state__d_xi @ state) + t1_t_t0 @ t_t0 + sc_xi * (t1_t_t0__d_xi @ t_t0). */
        static fom_float x0[650];
        static fom_float x1[600];
        static fom_float got[20], ref[20];
        fom_float sc[1];
        static const char *setting[] = {"all schedule values 0", "sc_xi = 1"};
        fom_float worst_op = 0.0, err, scale;
        int r, i, c, j, bad, probes = 0;
        (void)setting; (void)j; (void)sc;
        for (j = 0; j < 650; ++j) {
          for (r = 0; r <= 1; ++r) {
            for (c = 0; c < 650; ++c) x0[c] = 0.0;
            for (c = 0; c < 600; ++c) x1[c] = 0.0;
            x0[j] = 1.0;
            sc[0] = (fom_float)(r == 1);
            ++probes;
            for (i = 0; i < 20; ++i) got[i] = ref[i] = (fom_float)(i % 7 - 3);
            fom_fused_t1(x0, x1, sc[0], got);
            fom_ref_fused_t1(x0, x1, sc[0], ref);
            err = 0.0; scale = 1.0; bad = 0;
            for (i = 0; i < 20; ++i) {
                fom_float d = got[i] - ref[i];
                fom_float a = ref[i] - (fom_float)(i % 7 - 3);
                if (d < 0) d = -d;
                if (a < 0) a = -a;
                if (!(d <= err)) { err = d; bad = i; }  /* NaN-safe */
                if (1.0 + a > scale) scale = 1.0 + a;
            }
            if (!(err <= worst_op)) worst_op = err;
            if (!(err <= ops_tol * scale)) {
                printf("VERIFY FAILED: fom_fused_t1 entry (%d, %d) of state is off by %.3e from its matrices (%s)\n",
                       bad, j, (double)err, setting[r]);
                return 1;
            }
          }
        }
        for (j = 0; j < 600; ++j) {
          for (r = 0; r <= 1; ++r) {
            for (c = 0; c < 650; ++c) x0[c] = 0.0;
            for (c = 0; c < 600; ++c) x1[c] = 0.0;
            x1[j] = 1.0;
            sc[0] = (fom_float)(r == 1);
            ++probes;
            for (i = 0; i < 20; ++i) got[i] = ref[i] = (fom_float)(i % 7 - 3);
            fom_fused_t1(x0, x1, sc[0], got);
            fom_ref_fused_t1(x0, x1, sc[0], ref);
            err = 0.0; scale = 1.0; bad = 0;
            for (i = 0; i < 20; ++i) {
                fom_float d = got[i] - ref[i];
                fom_float a = ref[i] - (fom_float)(i % 7 - 3);
                if (d < 0) d = -d;
                if (a < 0) a = -a;
                if (!(d <= err)) { err = d; bad = i; }  /* NaN-safe */
                if (1.0 + a > scale) scale = 1.0 + a;
            }
            if (!(err <= worst_op)) worst_op = err;
            if (!(err <= ops_tol * scale)) {
                printf("VERIFY FAILED: fom_fused_t1 entry (%d, %d) of t_t0 is off by %.3e from its matrices (%s)\n",
                       bad, j, (double)err, setting[r]);
                return 1;
            }
          }
        }
        for (r = 0; r < 2; ++r) {
            for (c = 0; c < 650; ++c) x0[c] = FOM_VERIFY_FUSED_T1_X0[r][c];
            for (c = 0; c < 600; ++c) x1[c] = FOM_VERIFY_FUSED_T1_X1[r][c];
            sc[0] = FOM_VERIFY_FUSED_T1_SC[r][0];
            ++probes;
            for (i = 0; i < 20; ++i) got[i] = ref[i] = (fom_float)(i % 7 - 3);
            fom_fused_t1(x0, x1, sc[0], got);
            fom_ref_fused_t1(x0, x1, sc[0], ref);
            err = 0.0; scale = 1.0; bad = 0;
            for (i = 0; i < 20; ++i) {
                fom_float d = got[i] - ref[i];
                fom_float a = ref[i] - (fom_float)(i % 7 - 3);
                if (d < 0) d = -d;
                if (a < 0) a = -a;
                if (!(d <= err)) { err = d; bad = i; }  /* NaN-safe */
                if (1.0 + a > scale) scale = 1.0 + a;
            }
            if (!(err <= worst_op)) worst_op = err;
            if (!(err <= ops_tol * scale)) {
                printf("VERIFY FAILED: fom_fused_t1 differs from its matrices by %.3e in row %d on random probe %d\n", (double)err, bad, r);
                return 1;
            }
        }
        printf("OPSCHECK name=fused_t1 mode=basis probes=%d max_err=%.3e\n", probes, (double)worst_op);
    }
    {
        /* fom_fused_su: direct check against its members, out += su_state @ state + sc_xi * (su_state__d_xi @ state) + su_t_t0 @ t_t0 + sc_xi * (su_t_t0__d_xi @ t_t0) + su_t_t1 @ t_t1. */
        static fom_float x0[650];
        static fom_float x1[600];
        static fom_float x2[20];
        static fom_float got[650], ref[650];
        fom_float sc[1];
        static const char *setting[] = {"all schedule values 0", "sc_xi = 1"};
        fom_float worst_op = 0.0, err, scale;
        int r, i, c, j, bad, probes = 0;
        (void)setting; (void)j; (void)sc;
        for (j = 0; j < 650; ++j) {
          for (r = 0; r <= 1; ++r) {
            for (c = 0; c < 650; ++c) x0[c] = 0.0;
            for (c = 0; c < 600; ++c) x1[c] = 0.0;
            for (c = 0; c < 20; ++c) x2[c] = 0.0;
            x0[j] = 1.0;
            sc[0] = (fom_float)(r == 1);
            ++probes;
            for (i = 0; i < 650; ++i) got[i] = ref[i] = (fom_float)(i % 7 - 3);
            fom_fused_su(x0, x1, x2, sc[0], got);
            fom_ref_fused_su(x0, x1, x2, sc[0], ref);
            err = 0.0; scale = 1.0; bad = 0;
            for (i = 0; i < 650; ++i) {
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
        for (j = 0; j < 600; ++j) {
          for (r = 0; r <= 1; ++r) {
            for (c = 0; c < 650; ++c) x0[c] = 0.0;
            for (c = 0; c < 600; ++c) x1[c] = 0.0;
            for (c = 0; c < 20; ++c) x2[c] = 0.0;
            x1[j] = 1.0;
            sc[0] = (fom_float)(r == 1);
            ++probes;
            for (i = 0; i < 650; ++i) got[i] = ref[i] = (fom_float)(i % 7 - 3);
            fom_fused_su(x0, x1, x2, sc[0], got);
            fom_ref_fused_su(x0, x1, x2, sc[0], ref);
            err = 0.0; scale = 1.0; bad = 0;
            for (i = 0; i < 650; ++i) {
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
        for (j = 0; j < 20; ++j) {
          for (r = 0; r <= 1; ++r) {
            for (c = 0; c < 650; ++c) x0[c] = 0.0;
            for (c = 0; c < 600; ++c) x1[c] = 0.0;
            for (c = 0; c < 20; ++c) x2[c] = 0.0;
            x2[j] = 1.0;
            sc[0] = (fom_float)(r == 1);
            ++probes;
            for (i = 0; i < 650; ++i) got[i] = ref[i] = (fom_float)(i % 7 - 3);
            fom_fused_su(x0, x1, x2, sc[0], got);
            fom_ref_fused_su(x0, x1, x2, sc[0], ref);
            err = 0.0; scale = 1.0; bad = 0;
            for (i = 0; i < 650; ++i) {
                fom_float d = got[i] - ref[i];
                fom_float a = ref[i] - (fom_float)(i % 7 - 3);
                if (d < 0) d = -d;
                if (a < 0) a = -a;
                if (!(d <= err)) { err = d; bad = i; }  /* NaN-safe */
                if (1.0 + a > scale) scale = 1.0 + a;
            }
            if (!(err <= worst_op)) worst_op = err;
            if (!(err <= ops_tol * scale)) {
                printf("VERIFY FAILED: fom_fused_su entry (%d, %d) of t_t1 is off by %.3e from its matrices (%s)\n",
                       bad, j, (double)err, setting[r]);
                return 1;
            }
          }
        }
        for (r = 0; r < 2; ++r) {
            for (c = 0; c < 650; ++c) x0[c] = FOM_VERIFY_FUSED_SU_X0[r][c];
            for (c = 0; c < 600; ++c) x1[c] = FOM_VERIFY_FUSED_SU_X1[r][c];
            for (c = 0; c < 20; ++c) x2[c] = FOM_VERIFY_FUSED_SU_X2[r][c];
            sc[0] = FOM_VERIFY_FUSED_SU_SC[r][0];
            ++probes;
            for (i = 0; i < 650; ++i) got[i] = ref[i] = (fom_float)(i % 7 - 3);
            fom_fused_su(x0, x1, x2, sc[0], got);
            fom_ref_fused_su(x0, x1, x2, sc[0], ref);
            err = 0.0; scale = 1.0; bad = 0;
            for (i = 0; i < 650; ++i) {
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
        static fom_float x0[650];
        static fom_float bias[600], got[600], ref[600], pre[600];
        static fom_float act_lo[600], act_hi[600];
        FomAct_t0 hact;
        fom_float sc[1];
        static const char *setting[] = {"all schedule values 0", "activation off", "package activation", "random activation"};
        fom_float worst_op = 0.0, err, scale, m;
        int r, i, c, j, bad, touched, mode, probes = 0;
        (void)setting; (void)j; (void)sc; (void)m; (void)mode; (void)act_lo;
        (void)act_hi; (void)bias;
        for (i = 0; i < 600; ++i) pre[i] = 0.0;
        for (i = 0; i < 600; ++i) act_lo[i] = -FOM_VERIFY_HUGE;
        hact.lo = act_lo;
        for (i = 0; i < 600; ++i) act_hi[i] = FOM_VERIFY_HUGE;
        hact.hi = act_hi;
        for (i = 0; i < 600; ++i) bias[i] = FOM_VERIFY_STEP_T0_BIAS[0][i];
        for (j = 0; j < 650; ++j) {
          for (r = 0; r <= 0; ++r) {
            for (c = 0; c < 650; ++c) x0[c] = 0.0;
            x0[j] = 1.0;
            ++probes;
            for (i = 0; i < 600; ++i) {
                got[i] = (fom_float)(i % 7) - 3.5;
                ref[i] = -(fom_float)(i % 5) - 0.25;
            }
            fom_step_t0(x0, bias, &hact, got);
            fom_ref_step_t0(x0, bias, &hact, ref);
            err = 0.0; scale = 1.0; bad = 0; touched = -1;
            for (i = 0; i < 600; ++i) {
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
            for (c = 0; c < 650; ++c) x0[c] = FOM_VERIFY_STEP_T0_X0[r][c];
            for (i = 0; i < 600; ++i) bias[i] = FOM_VERIFY_STEP_T0_BIAS[r][i];
            for (i = 0; i < 600; ++i) act_lo[i] = -FOM_VERIFY_HUGE;
            hact.lo = act_lo;
            for (i = 0; i < 600; ++i) act_hi[i] = FOM_VERIFY_HUGE;
            hact.hi = act_hi;
            fom_ref_step_t0(x0, bias, &hact, pre);
            ++probes;
            for (i = 0; i < 600; ++i) {
                got[i] = (fom_float)(i % 7) - 3.5;
                ref[i] = -(fom_float)(i % 5) - 0.25;
            }
            fom_step_t0(x0, bias, &hact, got);
            fom_ref_step_t0(x0, bias, &hact, ref);
            err = 0.0; scale = 1.0; bad = 0; touched = -1;
            for (i = 0; i < 600; ++i) {
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
        for (i = 0; i < 600; ++i) pre[i] = 0.0;
        for (i = 0; i < 600; ++i) act_lo[i] = -FOM_VERIFY_HUGE;
        hact.lo = act_lo;
        for (i = 0; i < 600; ++i) act_hi[i] = FOM_VERIFY_HUGE;
        hact.hi = act_hi;
        for (r = 0; r < 2; ++r) {
            for (c = 0; c < 650; ++c) x0[c] = FOM_VERIFY_STEP_T0_X0[r][c];
            for (i = 0; i < 600; ++i) bias[i] = FOM_VERIFY_STEP_T0_BIAS[r][i];
            for (i = 0; i < 600; ++i) act_lo[i] = -FOM_VERIFY_HUGE;
            hact.lo = act_lo;
            for (i = 0; i < 600; ++i) act_hi[i] = FOM_VERIFY_HUGE;
            hact.hi = act_hi;
            fom_ref_step_t0(x0, bias, &hact, pre);
            ++probes;
            for (i = 0; i < 600; ++i) {
                got[i] = (fom_float)(i % 7) - 3.5;
                ref[i] = -(fom_float)(i % 5) - 0.25;
            }
            fom_step_t0(x0, bias, &FOM_ACT_t0, got);
            fom_ref_step_t0(x0, bias, &FOM_ACT_t0, ref);
            err = 0.0; scale = 1.0; bad = 0; touched = -1;
            for (i = 0; i < 600; ++i) {
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
        for (i = 0; i < 600; ++i) pre[i] = 0.0;
        for (i = 0; i < 600; ++i) act_lo[i] = -FOM_VERIFY_HUGE;
        hact.lo = act_lo;
        for (i = 0; i < 600; ++i) act_hi[i] = FOM_VERIFY_HUGE;
        hact.hi = act_hi;
        for (r = 0; r < 2; ++r) {
            for (c = 0; c < 650; ++c) x0[c] = FOM_VERIFY_STEP_T0_X0[r][c];
            for (i = 0; i < 600; ++i) bias[i] = FOM_VERIFY_STEP_T0_BIAS[r][i];
            for (i = 0; i < 600; ++i) act_lo[i] = -FOM_VERIFY_HUGE;
            hact.lo = act_lo;
            for (i = 0; i < 600; ++i) act_hi[i] = FOM_VERIFY_HUGE;
            hact.hi = act_hi;
            fom_ref_step_t0(x0, bias, &hact, pre);
            m = 0.0;
            for (i = 0; i < 600; ++i) m += ((pre[i] < 0) ? -pre[i] : pre[i]);
            m = (m > 0) ? m / 600 : 1.0;
            for (i = 0; i < 600; ++i) act_lo[i] = pre[i] + FOM_VERIFY_STEP_T0_R1[r][i] * m;
            hact.lo = act_lo;
            for (i = 0; i < 600; ++i) act_hi[i] = act_lo[i] + FOM_VERIFY_STEP_T0_R2[r][i] * m;
            hact.hi = act_hi;
            ++probes;
            for (i = 0; i < 600; ++i) {
                got[i] = (fom_float)(i % 7) - 3.5;
                ref[i] = -(fom_float)(i % 5) - 0.25;
            }
            fom_step_t0(x0, bias, &hact, got);
            fom_ref_step_t0(x0, bias, &hact, ref);
            err = 0.0; scale = 1.0; bad = 0; touched = -1;
            for (i = 0; i < 600; ++i) {
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
        for (i = 0; i < 600; ++i) pre[i] = 0.0;
        for (i = 0; i < 600; ++i) act_lo[i] = -FOM_VERIFY_HUGE;
        hact.lo = act_lo;
        for (i = 0; i < 600; ++i) act_hi[i] = FOM_VERIFY_HUGE;
        hact.hi = act_hi;
        printf("OPSCHECK name=step_t0 mode=basis probes=%d max_err=%.3e\n", probes, (double)worst_op);
    }
    {
        /* fom_step_t1: direct check against its members and the reference activation. */
        static fom_float x0[650];
        static fom_float x1[600];
        static fom_float bias[20], got[20], ref[20], pre[20];
        static fom_float act_lo[20], act_hi[20];
        FomAct_t1 hact;
        fom_float sc[1];
        static const char *setting[] = {"all schedule values 0", "sc_xi = 1", "activation off", "package activation", "random activation"};
        fom_float worst_op = 0.0, err, scale, m;
        int r, i, c, j, bad, touched, mode, probes = 0;
        (void)setting; (void)j; (void)sc; (void)m; (void)mode; (void)act_lo;
        (void)act_hi; (void)bias;
        for (i = 0; i < 20; ++i) pre[i] = 0.0;
        for (i = 0; i < 20; ++i) act_lo[i] = -FOM_VERIFY_HUGE;
        hact.lo = act_lo;
        for (i = 0; i < 20; ++i) bias[i] = FOM_VERIFY_STEP_T1_BIAS[0][i];
        for (j = 0; j < 650; ++j) {
          for (r = 0; r <= 1; ++r) {
            for (c = 0; c < 650; ++c) x0[c] = 0.0;
            for (c = 0; c < 600; ++c) x1[c] = 0.0;
            x0[j] = 1.0;
            sc[0] = (fom_float)(r == 1);
            ++probes;
            for (i = 0; i < 20; ++i) {
                got[i] = (fom_float)(i % 7) - 3.5;
                ref[i] = -(fom_float)(i % 5) - 0.25;
            }
            fom_step_t1(x0, x1, sc[0], bias, &hact, got);
            fom_ref_step_t1(x0, x1, sc[0], bias, &hact, ref);
            err = 0.0; scale = 1.0; bad = 0; touched = -1;
            for (i = 0; i < 20; ++i) {
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
                printf("VERIFY FAILED: fom_step_t1 changed selection row %d, which it must leave untouched\n", touched);
                return 1;
            }
            if (!(err <= ops_tol * scale)) {
                printf("VERIFY FAILED: fom_step_t1 entry (%d, %d) of state is off by %.3e from its matrices (%s)\n",
                       bad, j, (double)err, setting[r]);
                return 1;
            }
          }
        }
        for (i = 0; i < 20; ++i) bias[i] = FOM_VERIFY_STEP_T1_BIAS[0][i];
        for (j = 0; j < 600; ++j) {
          for (r = 0; r <= 1; ++r) {
            for (c = 0; c < 650; ++c) x0[c] = 0.0;
            for (c = 0; c < 600; ++c) x1[c] = 0.0;
            x1[j] = 1.0;
            sc[0] = (fom_float)(r == 1);
            ++probes;
            for (i = 0; i < 20; ++i) {
                got[i] = (fom_float)(i % 7) - 3.5;
                ref[i] = -(fom_float)(i % 5) - 0.25;
            }
            fom_step_t1(x0, x1, sc[0], bias, &hact, got);
            fom_ref_step_t1(x0, x1, sc[0], bias, &hact, ref);
            err = 0.0; scale = 1.0; bad = 0; touched = -1;
            for (i = 0; i < 20; ++i) {
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
                printf("VERIFY FAILED: fom_step_t1 changed selection row %d, which it must leave untouched\n", touched);
                return 1;
            }
            if (!(err <= ops_tol * scale)) {
                printf("VERIFY FAILED: fom_step_t1 entry (%d, %d) of t_t0 is off by %.3e from its matrices (%s)\n",
                       bad, j, (double)err, setting[r]);
                return 1;
            }
          }
        }
        for (r = 0; r < 2; ++r) {
            for (c = 0; c < 650; ++c) x0[c] = FOM_VERIFY_STEP_T1_X0[r][c];
            for (c = 0; c < 600; ++c) x1[c] = FOM_VERIFY_STEP_T1_X1[r][c];
            sc[0] = FOM_VERIFY_STEP_T1_SC[r][0];
            for (i = 0; i < 20; ++i) bias[i] = FOM_VERIFY_STEP_T1_BIAS[r][i];
            for (i = 0; i < 20; ++i) act_lo[i] = -FOM_VERIFY_HUGE;
            hact.lo = act_lo;
            fom_ref_step_t1(x0, x1, sc[0], bias, &hact, pre);
            ++probes;
            for (i = 0; i < 20; ++i) {
                got[i] = (fom_float)(i % 7) - 3.5;
                ref[i] = -(fom_float)(i % 5) - 0.25;
            }
            fom_step_t1(x0, x1, sc[0], bias, &hact, got);
            fom_ref_step_t1(x0, x1, sc[0], bias, &hact, ref);
            err = 0.0; scale = 1.0; bad = 0; touched = -1;
            for (i = 0; i < 20; ++i) {
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
                printf("VERIFY FAILED: fom_step_t1 changed selection row %d, which it must leave untouched\n", touched);
                return 1;
            }
            if (!(err <= ops_tol * scale)) {
                printf("VERIFY FAILED: fom_step_t1 differs from its reference by %.3e in row %d on random probe %d (activation off)\n",
                       (double)err, bad, r);
                return 1;
            }
        }
        for (i = 0; i < 20; ++i) pre[i] = 0.0;
        for (i = 0; i < 20; ++i) act_lo[i] = -FOM_VERIFY_HUGE;
        hact.lo = act_lo;
        for (r = 0; r < 2; ++r) {
            for (c = 0; c < 650; ++c) x0[c] = FOM_VERIFY_STEP_T1_X0[r][c];
            for (c = 0; c < 600; ++c) x1[c] = FOM_VERIFY_STEP_T1_X1[r][c];
            sc[0] = FOM_VERIFY_STEP_T1_SC[r][0];
            for (i = 0; i < 20; ++i) bias[i] = FOM_VERIFY_STEP_T1_BIAS[r][i];
            for (i = 0; i < 20; ++i) act_lo[i] = -FOM_VERIFY_HUGE;
            hact.lo = act_lo;
            fom_ref_step_t1(x0, x1, sc[0], bias, &hact, pre);
            ++probes;
            for (i = 0; i < 20; ++i) {
                got[i] = (fom_float)(i % 7) - 3.5;
                ref[i] = -(fom_float)(i % 5) - 0.25;
            }
            fom_step_t1(x0, x1, sc[0], bias, &FOM_ACT_t1, got);
            fom_ref_step_t1(x0, x1, sc[0], bias, &FOM_ACT_t1, ref);
            err = 0.0; scale = 1.0; bad = 0; touched = -1;
            for (i = 0; i < 20; ++i) {
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
                printf("VERIFY FAILED: fom_step_t1 changed selection row %d, which it must leave untouched\n", touched);
                return 1;
            }
            if (!(err <= ops_tol * scale)) {
                printf("VERIFY FAILED: fom_step_t1 differs from its reference by %.3e in row %d on random probe %d (package activation)\n",
                       (double)err, bad, r);
                return 1;
            }
        }
        for (i = 0; i < 20; ++i) pre[i] = 0.0;
        for (i = 0; i < 20; ++i) act_lo[i] = -FOM_VERIFY_HUGE;
        hact.lo = act_lo;
        for (r = 0; r < 2; ++r) {
            for (c = 0; c < 650; ++c) x0[c] = FOM_VERIFY_STEP_T1_X0[r][c];
            for (c = 0; c < 600; ++c) x1[c] = FOM_VERIFY_STEP_T1_X1[r][c];
            sc[0] = FOM_VERIFY_STEP_T1_SC[r][0];
            for (i = 0; i < 20; ++i) bias[i] = FOM_VERIFY_STEP_T1_BIAS[r][i];
            for (i = 0; i < 20; ++i) act_lo[i] = -FOM_VERIFY_HUGE;
            hact.lo = act_lo;
            fom_ref_step_t1(x0, x1, sc[0], bias, &hact, pre);
            m = 0.0;
            for (i = 0; i < 20; ++i) m += ((pre[i] < 0) ? -pre[i] : pre[i]);
            m = (m > 0) ? m / 20 : 1.0;
            for (i = 0; i < 20; ++i) act_lo[i] = pre[i] + FOM_VERIFY_STEP_T1_R1[r][i] * m;
            hact.lo = act_lo;
            ++probes;
            for (i = 0; i < 20; ++i) {
                got[i] = (fom_float)(i % 7) - 3.5;
                ref[i] = -(fom_float)(i % 5) - 0.25;
            }
            fom_step_t1(x0, x1, sc[0], bias, &hact, got);
            fom_ref_step_t1(x0, x1, sc[0], bias, &hact, ref);
            err = 0.0; scale = 1.0; bad = 0; touched = -1;
            for (i = 0; i < 20; ++i) {
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
                printf("VERIFY FAILED: fom_step_t1 changed selection row %d, which it must leave untouched\n", touched);
                return 1;
            }
            if (!(err <= ops_tol * scale)) {
                printf("VERIFY FAILED: fom_step_t1 differs from its reference by %.3e in row %d on random probe %d (random activation)\n",
                       (double)err, bad, r);
                return 1;
            }
        }
        for (i = 0; i < 20; ++i) pre[i] = 0.0;
        for (i = 0; i < 20; ++i) act_lo[i] = -FOM_VERIFY_HUGE;
        hact.lo = act_lo;
        printf("OPSCHECK name=step_t1 mode=basis probes=%d max_err=%.3e\n", probes, (double)worst_op);
    }
    {
        /* fom_step_su: direct check against its members. */
        static fom_float x0[650];
        static fom_float x1[600];
        static fom_float x2[20];
        static fom_float bias[650], got[650], ref[650], pre[650];
        static fom_float act_lo[650], act_hi[650];
        fom_float sc[1];
        static const char *setting[] = {"all schedule values 0", "sc_xi = 1", "random"};
        fom_float worst_op = 0.0, err, scale, m;
        int r, i, c, j, bad, touched, mode, probes = 0;
        (void)setting; (void)j; (void)sc; (void)m; (void)mode; (void)act_lo;
        (void)act_hi; (void)bias;
        for (i = 0; i < 650; ++i) pre[i] = 0.0;
        for (i = 0; i < 650; ++i) bias[i] = FOM_VERIFY_STEP_SU_BIAS[0][i];
        for (j = 0; j < 650; ++j) {
          for (r = 0; r <= 1; ++r) {
            for (c = 0; c < 650; ++c) x0[c] = 0.0;
            for (c = 0; c < 600; ++c) x1[c] = 0.0;
            for (c = 0; c < 20; ++c) x2[c] = 0.0;
            x0[j] = 1.0;
            sc[0] = (fom_float)(r == 1);
            ++probes;
            for (i = 0; i < 650; ++i) {
                got[i] = (fom_float)(i % 7) - 3.5;
                ref[i] = -(fom_float)(i % 5) - 0.25;
            }
            fom_step_su(x0, x1, x2, sc[0], bias, got);
            fom_ref_step_su(x0, x1, x2, sc[0], bias, ref);
            err = 0.0; scale = 1.0; bad = 0; touched = -1;
            for (i = 0; i < 650; ++i) {
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
        for (i = 0; i < 650; ++i) bias[i] = FOM_VERIFY_STEP_SU_BIAS[0][i];
        for (j = 0; j < 600; ++j) {
          for (r = 0; r <= 1; ++r) {
            for (c = 0; c < 650; ++c) x0[c] = 0.0;
            for (c = 0; c < 600; ++c) x1[c] = 0.0;
            for (c = 0; c < 20; ++c) x2[c] = 0.0;
            x1[j] = 1.0;
            sc[0] = (fom_float)(r == 1);
            ++probes;
            for (i = 0; i < 650; ++i) {
                got[i] = (fom_float)(i % 7) - 3.5;
                ref[i] = -(fom_float)(i % 5) - 0.25;
            }
            fom_step_su(x0, x1, x2, sc[0], bias, got);
            fom_ref_step_su(x0, x1, x2, sc[0], bias, ref);
            err = 0.0; scale = 1.0; bad = 0; touched = -1;
            for (i = 0; i < 650; ++i) {
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
        for (i = 0; i < 650; ++i) bias[i] = FOM_VERIFY_STEP_SU_BIAS[0][i];
        for (j = 0; j < 20; ++j) {
          for (r = 0; r <= 1; ++r) {
            for (c = 0; c < 650; ++c) x0[c] = 0.0;
            for (c = 0; c < 600; ++c) x1[c] = 0.0;
            for (c = 0; c < 20; ++c) x2[c] = 0.0;
            x2[j] = 1.0;
            sc[0] = (fom_float)(r == 1);
            ++probes;
            for (i = 0; i < 650; ++i) {
                got[i] = (fom_float)(i % 7) - 3.5;
                ref[i] = -(fom_float)(i % 5) - 0.25;
            }
            fom_step_su(x0, x1, x2, sc[0], bias, got);
            fom_ref_step_su(x0, x1, x2, sc[0], bias, ref);
            err = 0.0; scale = 1.0; bad = 0; touched = -1;
            for (i = 0; i < 650; ++i) {
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
                printf("VERIFY FAILED: fom_step_su entry (%d, %d) of t_t1 is off by %.3e from its matrices (%s)\n",
                       bad, j, (double)err, setting[r]);
                return 1;
            }
          }
        }
        for (r = 0; r < 2; ++r) {
            for (c = 0; c < 650; ++c) x0[c] = FOM_VERIFY_STEP_SU_X0[r][c];
            for (c = 0; c < 600; ++c) x1[c] = FOM_VERIFY_STEP_SU_X1[r][c];
            for (c = 0; c < 20; ++c) x2[c] = FOM_VERIFY_STEP_SU_X2[r][c];
            sc[0] = FOM_VERIFY_STEP_SU_SC[r][0];
            for (i = 0; i < 650; ++i) bias[i] = FOM_VERIFY_STEP_SU_BIAS[r][i];
            ++probes;
            for (i = 0; i < 650; ++i) {
                got[i] = (fom_float)(i % 7) - 3.5;
                ref[i] = -(fom_float)(i % 5) - 0.25;
            }
            fom_step_su(x0, x1, x2, sc[0], bias, got);
            fom_ref_step_su(x0, x1, x2, sc[0], bias, ref);
            err = 0.0; scale = 1.0; bad = 0; touched = -1;
            for (i = 0; i < 650; ++i) {
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
        /* fom_fused_su__rest: direct check against its members, leaving its selection rows untouched. */
        static fom_float x0[650];
        static fom_float x1[600];
        static fom_float x2[20];
        static fom_float bias[650], got[650], ref[650], pre[650];
        static fom_float act_lo[650], act_hi[650];
        static const unsigned char sel[650] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        fom_float sc[1];
        static const char *setting[] = {"all schedule values 0", "sc_xi = 1", "random"};
        fom_float worst_op = 0.0, err, scale, m;
        int r, i, c, j, bad, touched, mode, probes = 0;
        (void)setting; (void)j; (void)sc; (void)m; (void)mode; (void)act_lo;
        (void)act_hi; (void)bias;
        for (i = 0; i < 650; ++i) pre[i] = 0.0;
        for (i = 0; i < 650; ++i) bias[i] = FOM_VERIFY_STEP_SU_BIAS[0][i];
        for (j = 0; j < 650; ++j) {
          for (r = 0; r <= 1; ++r) {
            for (c = 0; c < 650; ++c) x0[c] = 0.0;
            for (c = 0; c < 600; ++c) x1[c] = 0.0;
            for (c = 0; c < 20; ++c) x2[c] = 0.0;
            x0[j] = 1.0;
            sc[0] = (fom_float)(r == 1);
            ++probes;
            for (i = 0; i < 650; ++i) {
                got[i] = (fom_float)(i % 7) - 3.5;
                ref[i] = got[i];
            }
            fom_fused_su__rest(x0, x1, x2, sc[0], got);
            fom_ref_fused_su(x0, x1, x2, sc[0], ref);
            err = 0.0; scale = 1.0; bad = 0; touched = -1;
            for (i = 0; i < 650; ++i) {
                if (sel[i]) {
                    if (!(got[i] == (fom_float)(i % 7) - 3.5)) touched = i;
                    continue;
                }
                {
                    fom_float d = got[i] - ref[i];
                    fom_float a = ref[i] - ((fom_float)(i % 7) - 3.5), b = pre[i];
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
                printf("VERIFY FAILED: fom_fused_su__rest changed selection row %d, which it must leave untouched\n", touched);
                return 1;
            }
            if (!(err <= ops_tol * scale)) {
                printf("VERIFY FAILED: fom_fused_su__rest entry (%d, %d) of state is off by %.3e from its matrices (%s)\n",
                       bad, j, (double)err, setting[r]);
                return 1;
            }
          }
        }
        for (i = 0; i < 650; ++i) bias[i] = FOM_VERIFY_STEP_SU_BIAS[0][i];
        for (j = 0; j < 600; ++j) {
          for (r = 0; r <= 1; ++r) {
            for (c = 0; c < 650; ++c) x0[c] = 0.0;
            for (c = 0; c < 600; ++c) x1[c] = 0.0;
            for (c = 0; c < 20; ++c) x2[c] = 0.0;
            x1[j] = 1.0;
            sc[0] = (fom_float)(r == 1);
            ++probes;
            for (i = 0; i < 650; ++i) {
                got[i] = (fom_float)(i % 7) - 3.5;
                ref[i] = got[i];
            }
            fom_fused_su__rest(x0, x1, x2, sc[0], got);
            fom_ref_fused_su(x0, x1, x2, sc[0], ref);
            err = 0.0; scale = 1.0; bad = 0; touched = -1;
            for (i = 0; i < 650; ++i) {
                if (sel[i]) {
                    if (!(got[i] == (fom_float)(i % 7) - 3.5)) touched = i;
                    continue;
                }
                {
                    fom_float d = got[i] - ref[i];
                    fom_float a = ref[i] - ((fom_float)(i % 7) - 3.5), b = pre[i];
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
                printf("VERIFY FAILED: fom_fused_su__rest changed selection row %d, which it must leave untouched\n", touched);
                return 1;
            }
            if (!(err <= ops_tol * scale)) {
                printf("VERIFY FAILED: fom_fused_su__rest entry (%d, %d) of t_t0 is off by %.3e from its matrices (%s)\n",
                       bad, j, (double)err, setting[r]);
                return 1;
            }
          }
        }
        for (i = 0; i < 650; ++i) bias[i] = FOM_VERIFY_STEP_SU_BIAS[0][i];
        for (j = 0; j < 20; ++j) {
          for (r = 0; r <= 1; ++r) {
            for (c = 0; c < 650; ++c) x0[c] = 0.0;
            for (c = 0; c < 600; ++c) x1[c] = 0.0;
            for (c = 0; c < 20; ++c) x2[c] = 0.0;
            x2[j] = 1.0;
            sc[0] = (fom_float)(r == 1);
            ++probes;
            for (i = 0; i < 650; ++i) {
                got[i] = (fom_float)(i % 7) - 3.5;
                ref[i] = got[i];
            }
            fom_fused_su__rest(x0, x1, x2, sc[0], got);
            fom_ref_fused_su(x0, x1, x2, sc[0], ref);
            err = 0.0; scale = 1.0; bad = 0; touched = -1;
            for (i = 0; i < 650; ++i) {
                if (sel[i]) {
                    if (!(got[i] == (fom_float)(i % 7) - 3.5)) touched = i;
                    continue;
                }
                {
                    fom_float d = got[i] - ref[i];
                    fom_float a = ref[i] - ((fom_float)(i % 7) - 3.5), b = pre[i];
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
                printf("VERIFY FAILED: fom_fused_su__rest changed selection row %d, which it must leave untouched\n", touched);
                return 1;
            }
            if (!(err <= ops_tol * scale)) {
                printf("VERIFY FAILED: fom_fused_su__rest entry (%d, %d) of t_t1 is off by %.3e from its matrices (%s)\n",
                       bad, j, (double)err, setting[r]);
                return 1;
            }
          }
        }
        for (r = 0; r < 2; ++r) {
            for (c = 0; c < 650; ++c) x0[c] = FOM_VERIFY_STEP_SU_X0[r][c];
            for (c = 0; c < 600; ++c) x1[c] = FOM_VERIFY_STEP_SU_X1[r][c];
            for (c = 0; c < 20; ++c) x2[c] = FOM_VERIFY_STEP_SU_X2[r][c];
            sc[0] = FOM_VERIFY_STEP_SU_SC[r][0];
            for (i = 0; i < 650; ++i) bias[i] = FOM_VERIFY_STEP_SU_BIAS[r][i];
            ++probes;
            for (i = 0; i < 650; ++i) {
                got[i] = (fom_float)(i % 7) - 3.5;
                ref[i] = got[i];
            }
            fom_fused_su__rest(x0, x1, x2, sc[0], got);
            fom_ref_fused_su(x0, x1, x2, sc[0], ref);
            err = 0.0; scale = 1.0; bad = 0; touched = -1;
            for (i = 0; i < 650; ++i) {
                if (sel[i]) {
                    if (!(got[i] == (fom_float)(i % 7) - 3.5)) touched = i;
                    continue;
                }
                {
                    fom_float d = got[i] - ref[i];
                    fom_float a = ref[i] - ((fom_float)(i % 7) - 3.5), b = pre[i];
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
                printf("VERIFY FAILED: fom_fused_su__rest changed selection row %d, which it must leave untouched\n", touched);
                return 1;
            }
            if (!(err <= ops_tol * scale)) {
                printf("VERIFY FAILED: fom_fused_su__rest differs from its reference by %.3e in row %d on random probe %d (random)\n",
                       (double)err, bad, r);
                return 1;
            }
        }
        printf("OPSCHECK name=fused_su__rest mode=basis probes=%d max_err=%.3e\n", probes, (double)worst_op);
    }
    {
        /* fom_step_su__rest: direct check against its members, leaving its selection rows untouched. */
        static fom_float x0[650];
        static fom_float x1[600];
        static fom_float x2[20];
        static fom_float bias[650], got[650], ref[650], pre[650];
        static fom_float act_lo[650], act_hi[650];
        static const unsigned char sel[650] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        fom_float sc[1];
        static const char *setting[] = {"all schedule values 0", "sc_xi = 1", "random"};
        fom_float worst_op = 0.0, err, scale, m;
        int r, i, c, j, bad, touched, mode, probes = 0;
        (void)setting; (void)j; (void)sc; (void)m; (void)mode; (void)act_lo;
        (void)act_hi; (void)bias;
        for (i = 0; i < 650; ++i) pre[i] = 0.0;
        for (i = 0; i < 650; ++i) bias[i] = FOM_VERIFY_STEP_SU_BIAS[0][i];
        for (j = 0; j < 650; ++j) {
          for (r = 0; r <= 1; ++r) {
            for (c = 0; c < 650; ++c) x0[c] = 0.0;
            for (c = 0; c < 600; ++c) x1[c] = 0.0;
            for (c = 0; c < 20; ++c) x2[c] = 0.0;
            x0[j] = 1.0;
            sc[0] = (fom_float)(r == 1);
            ++probes;
            for (i = 0; i < 650; ++i) {
                got[i] = (fom_float)(i % 7) - 3.5;
                ref[i] = -(fom_float)(i % 5) - 0.25;
            }
            fom_step_su__rest(x0, x1, x2, sc[0], bias, got);
            fom_ref_step_su(x0, x1, x2, sc[0], bias, ref);
            err = 0.0; scale = 1.0; bad = 0; touched = -1;
            for (i = 0; i < 650; ++i) {
                if (sel[i]) {
                    if (!(got[i] == (fom_float)(i % 7) - 3.5)) touched = i;
                    continue;
                }
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
                printf("VERIFY FAILED: fom_step_su__rest changed selection row %d, which it must leave untouched\n", touched);
                return 1;
            }
            if (!(err <= ops_tol * scale)) {
                printf("VERIFY FAILED: fom_step_su__rest entry (%d, %d) of state is off by %.3e from its matrices (%s)\n",
                       bad, j, (double)err, setting[r]);
                return 1;
            }
          }
        }
        for (i = 0; i < 650; ++i) bias[i] = FOM_VERIFY_STEP_SU_BIAS[0][i];
        for (j = 0; j < 600; ++j) {
          for (r = 0; r <= 1; ++r) {
            for (c = 0; c < 650; ++c) x0[c] = 0.0;
            for (c = 0; c < 600; ++c) x1[c] = 0.0;
            for (c = 0; c < 20; ++c) x2[c] = 0.0;
            x1[j] = 1.0;
            sc[0] = (fom_float)(r == 1);
            ++probes;
            for (i = 0; i < 650; ++i) {
                got[i] = (fom_float)(i % 7) - 3.5;
                ref[i] = -(fom_float)(i % 5) - 0.25;
            }
            fom_step_su__rest(x0, x1, x2, sc[0], bias, got);
            fom_ref_step_su(x0, x1, x2, sc[0], bias, ref);
            err = 0.0; scale = 1.0; bad = 0; touched = -1;
            for (i = 0; i < 650; ++i) {
                if (sel[i]) {
                    if (!(got[i] == (fom_float)(i % 7) - 3.5)) touched = i;
                    continue;
                }
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
                printf("VERIFY FAILED: fom_step_su__rest changed selection row %d, which it must leave untouched\n", touched);
                return 1;
            }
            if (!(err <= ops_tol * scale)) {
                printf("VERIFY FAILED: fom_step_su__rest entry (%d, %d) of t_t0 is off by %.3e from its matrices (%s)\n",
                       bad, j, (double)err, setting[r]);
                return 1;
            }
          }
        }
        for (i = 0; i < 650; ++i) bias[i] = FOM_VERIFY_STEP_SU_BIAS[0][i];
        for (j = 0; j < 20; ++j) {
          for (r = 0; r <= 1; ++r) {
            for (c = 0; c < 650; ++c) x0[c] = 0.0;
            for (c = 0; c < 600; ++c) x1[c] = 0.0;
            for (c = 0; c < 20; ++c) x2[c] = 0.0;
            x2[j] = 1.0;
            sc[0] = (fom_float)(r == 1);
            ++probes;
            for (i = 0; i < 650; ++i) {
                got[i] = (fom_float)(i % 7) - 3.5;
                ref[i] = -(fom_float)(i % 5) - 0.25;
            }
            fom_step_su__rest(x0, x1, x2, sc[0], bias, got);
            fom_ref_step_su(x0, x1, x2, sc[0], bias, ref);
            err = 0.0; scale = 1.0; bad = 0; touched = -1;
            for (i = 0; i < 650; ++i) {
                if (sel[i]) {
                    if (!(got[i] == (fom_float)(i % 7) - 3.5)) touched = i;
                    continue;
                }
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
                printf("VERIFY FAILED: fom_step_su__rest changed selection row %d, which it must leave untouched\n", touched);
                return 1;
            }
            if (!(err <= ops_tol * scale)) {
                printf("VERIFY FAILED: fom_step_su__rest entry (%d, %d) of t_t1 is off by %.3e from its matrices (%s)\n",
                       bad, j, (double)err, setting[r]);
                return 1;
            }
          }
        }
        for (r = 0; r < 2; ++r) {
            for (c = 0; c < 650; ++c) x0[c] = FOM_VERIFY_STEP_SU_X0[r][c];
            for (c = 0; c < 600; ++c) x1[c] = FOM_VERIFY_STEP_SU_X1[r][c];
            for (c = 0; c < 20; ++c) x2[c] = FOM_VERIFY_STEP_SU_X2[r][c];
            sc[0] = FOM_VERIFY_STEP_SU_SC[r][0];
            for (i = 0; i < 650; ++i) bias[i] = FOM_VERIFY_STEP_SU_BIAS[r][i];
            ++probes;
            for (i = 0; i < 650; ++i) {
                got[i] = (fom_float)(i % 7) - 3.5;
                ref[i] = -(fom_float)(i % 5) - 0.25;
            }
            fom_step_su__rest(x0, x1, x2, sc[0], bias, got);
            fom_ref_step_su(x0, x1, x2, sc[0], bias, ref);
            err = 0.0; scale = 1.0; bad = 0; touched = -1;
            for (i = 0; i < 650; ++i) {
                if (sel[i]) {
                    if (!(got[i] == (fom_float)(i % 7) - 3.5)) touched = i;
                    continue;
                }
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
                printf("VERIFY FAILED: fom_step_su__rest changed selection row %d, which it must leave untouched\n", touched);
                return 1;
            }
            if (!(err <= ops_tol * scale)) {
                printf("VERIFY FAILED: fom_step_su__rest differs from its reference by %.3e in row %d on random probe %d (random)\n",
                       (double)err, bad, r);
                return 1;
            }
        }
        printf("OPSCHECK name=step_su__rest mode=basis probes=%d max_err=%.3e\n", probes, (double)worst_op);
    }
    {
        /* fom_iterate: direct check against the chained reference steps. */
        static fom_float x[650];
        static fom_float bias_t0[600];
        static fom_float bias_t1[20];
        static fom_float bias_su[650];
        static fom_float g_t_t0[600], r_t_t0[600];
        static fom_float g_t_t1[20], r_t_t1[20];
        static fom_float g_new_state[650], r_new_state[650];
        static fom_float lo_t0[600], hi_t0[600];
        static fom_float lo_t1[20], hi_t1[20];
        FomAct_t0 hact_t0;
        FomAct_t1 hact_t1;
        const FomAct_t0 *pact_t0 = &FOM_ACT_t0;
        const FomAct_t1 *pact_t1 = &FOM_ACT_t1;
        fom_float sc[1];
        static const char *setting[] = {"all schedule values 0", "sc_xi = 1", "random schedule values", "activations off", "package activations", "random activations"};
        static const char *output[] = {"t_t0", "t_t1", "new_state"};
        fom_float worst_op = 0.0, err, scale, m;
        int r, i, j, bad, which, probes = 0;
        (void)setting; (void)output; (void)j; (void)sc; (void)m;
        (void)pact_t0; (void)lo_t0; (void)hi_t0; (void)pact_t1; (void)lo_t1; (void)hi_t1;
        for (i = 0; i < 600; ++i) lo_t0[i] = -FOM_VERIFY_HUGE;
        hact_t0.lo = lo_t0;
        for (i = 0; i < 600; ++i) hi_t0[i] = FOM_VERIFY_HUGE;
        hact_t0.hi = hi_t0;
        for (i = 0; i < 20; ++i) lo_t1[i] = -FOM_VERIFY_HUGE;
        hact_t1.lo = lo_t1;
        for (i = 0; i < 600; ++i) bias_t0[i] = 0.0;
        for (i = 0; i < 20; ++i) bias_t1[i] = 0.0;
        for (i = 0; i < 650; ++i) bias_su[i] = 0.0;
        for (j = 0; j < 650; ++j) {
          for (r = 0; r <= 2; ++r) {
            for (i = 0; i < 650; ++i) x[i] = 0.0;
            x[j] = 1.0;
            sc[0] = (r == 2) ? FOM_VERIFY_ITER_SC[2][0] : (fom_float)(r == 1);
            ++probes;
            for (i = 0; i < 600; ++i) { g_t_t0[i] = (fom_float)(i % 7) - 3.5; r_t_t0[i] = -(fom_float)(i % 5) - 0.25; }
            for (i = 0; i < 20; ++i) { g_t_t1[i] = (fom_float)(i % 7) - 3.5; r_t_t1[i] = -(fom_float)(i % 5) - 0.25; }
            for (i = 0; i < 650; ++i) { g_new_state[i] = (fom_float)(i % 7) - 3.5; r_new_state[i] = -(fom_float)(i % 5) - 0.25; }
            fom_iterate(x, sc[0], bias_t0, bias_t1, bias_su, &hact_t0, &hact_t1, g_t_t0, g_t_t1, g_new_state);
            fom_ref_iterate(x, sc[0], bias_t0, bias_t1, bias_su, &hact_t0, &hact_t1, r_t_t0, r_t_t1, r_new_state);
            err = 0.0; scale = 1.0; bad = 0; which = 0;
            for (i = 0; i < 600; ++i) {
                fom_float d = g_t_t0[i] - r_t_t0[i], a = r_t_t0[i];
                if (d < 0) d = -d;
                if (a < 0) a = -a;
                if (!(d <= err)) { err = d; bad = i; which = 0; }
                if (1.0 + a > scale) scale = 1.0 + a;
            }
            for (i = 0; i < 20; ++i) {
                fom_float d = g_t_t1[i] - r_t_t1[i], a = r_t_t1[i];
                if (d < 0) d = -d;
                if (a < 0) a = -a;
                if (!(d <= err)) { err = d; bad = i; which = 1; }
                if (1.0 + a > scale) scale = 1.0 + a;
            }
            for (i = 0; i < 650; ++i) {
                fom_float d = g_new_state[i] - r_new_state[i], a = r_new_state[i];
                if (d < 0) d = -d;
                if (a < 0) a = -a;
                if (!(d <= err)) { err = d; bad = i; which = 2; }
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
            for (i = 0; i < 650; ++i) x[i] = FOM_VERIFY_ITER_X[r][i];
            sc[0] = FOM_VERIFY_ITER_SC[r][0];
            for (i = 0; i < 600; ++i) bias_t0[i] = FOM_VERIFY_ITER_T0_BIAS[r][i];
            for (i = 0; i < 20; ++i) bias_t1[i] = FOM_VERIFY_ITER_T1_BIAS[r][i];
            for (i = 0; i < 650; ++i) bias_su[i] = FOM_VERIFY_ITER_SU_BIAS[r][i];
            ++probes;
            for (i = 0; i < 600; ++i) { g_t_t0[i] = (fom_float)(i % 7) - 3.5; r_t_t0[i] = -(fom_float)(i % 5) - 0.25; }
            for (i = 0; i < 20; ++i) { g_t_t1[i] = (fom_float)(i % 7) - 3.5; r_t_t1[i] = -(fom_float)(i % 5) - 0.25; }
            for (i = 0; i < 650; ++i) { g_new_state[i] = (fom_float)(i % 7) - 3.5; r_new_state[i] = -(fom_float)(i % 5) - 0.25; }
            fom_iterate(x, sc[0], bias_t0, bias_t1, bias_su, &hact_t0, &hact_t1, g_t_t0, g_t_t1, g_new_state);
            fom_ref_iterate(x, sc[0], bias_t0, bias_t1, bias_su, &hact_t0, &hact_t1, r_t_t0, r_t_t1, r_new_state);
            err = 0.0; scale = 1.0; bad = 0; which = 0;
            for (i = 0; i < 600; ++i) {
                fom_float d = g_t_t0[i] - r_t_t0[i], a = r_t_t0[i];
                if (d < 0) d = -d;
                if (a < 0) a = -a;
                if (!(d <= err)) { err = d; bad = i; which = 0; }
                if (1.0 + a > scale) scale = 1.0 + a;
            }
            for (i = 0; i < 20; ++i) {
                fom_float d = g_t_t1[i] - r_t_t1[i], a = r_t_t1[i];
                if (d < 0) d = -d;
                if (a < 0) a = -a;
                if (!(d <= err)) { err = d; bad = i; which = 1; }
                if (1.0 + a > scale) scale = 1.0 + a;
            }
            for (i = 0; i < 650; ++i) {
                fom_float d = g_new_state[i] - r_new_state[i], a = r_new_state[i];
                if (d < 0) d = -d;
                if (a < 0) a = -a;
                if (!(d <= err)) { err = d; bad = i; which = 2; }
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
            for (i = 0; i < 650; ++i) x[i] = FOM_VERIFY_ITER_X[r][i];
            sc[0] = FOM_VERIFY_ITER_SC[r][0];
            for (i = 0; i < 600; ++i) bias_t0[i] = FOM_VERIFY_ITER_T0_BIAS[r][i];
            for (i = 0; i < 20; ++i) bias_t1[i] = FOM_VERIFY_ITER_T1_BIAS[r][i];
            for (i = 0; i < 650; ++i) bias_su[i] = FOM_VERIFY_ITER_SU_BIAS[r][i];
            ++probes;
            for (i = 0; i < 600; ++i) { g_t_t0[i] = (fom_float)(i % 7) - 3.5; r_t_t0[i] = -(fom_float)(i % 5) - 0.25; }
            for (i = 0; i < 20; ++i) { g_t_t1[i] = (fom_float)(i % 7) - 3.5; r_t_t1[i] = -(fom_float)(i % 5) - 0.25; }
            for (i = 0; i < 650; ++i) { g_new_state[i] = (fom_float)(i % 7) - 3.5; r_new_state[i] = -(fom_float)(i % 5) - 0.25; }
            fom_iterate(x, sc[0], bias_t0, bias_t1, bias_su, pact_t0, pact_t1, g_t_t0, g_t_t1, g_new_state);
            fom_ref_iterate(x, sc[0], bias_t0, bias_t1, bias_su, pact_t0, pact_t1, r_t_t0, r_t_t1, r_new_state);
            err = 0.0; scale = 1.0; bad = 0; which = 0;
            for (i = 0; i < 600; ++i) {
                fom_float d = g_t_t0[i] - r_t_t0[i], a = r_t_t0[i];
                if (d < 0) d = -d;
                if (a < 0) a = -a;
                if (!(d <= err)) { err = d; bad = i; which = 0; }
                if (1.0 + a > scale) scale = 1.0 + a;
            }
            for (i = 0; i < 20; ++i) {
                fom_float d = g_t_t1[i] - r_t_t1[i], a = r_t_t1[i];
                if (d < 0) d = -d;
                if (a < 0) a = -a;
                if (!(d <= err)) { err = d; bad = i; which = 1; }
                if (1.0 + a > scale) scale = 1.0 + a;
            }
            for (i = 0; i < 650; ++i) {
                fom_float d = g_new_state[i] - r_new_state[i], a = r_new_state[i];
                if (d < 0) d = -d;
                if (a < 0) a = -a;
                if (!(d <= err)) { err = d; bad = i; which = 2; }
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
            for (i = 0; i < 650; ++i) x[i] = FOM_VERIFY_ITER_X[r][i];
            sc[0] = FOM_VERIFY_ITER_SC[r][0];
            for (i = 0; i < 600; ++i) bias_t0[i] = FOM_VERIFY_ITER_T0_BIAS[r][i];
            for (i = 0; i < 20; ++i) bias_t1[i] = FOM_VERIFY_ITER_T1_BIAS[r][i];
            for (i = 0; i < 650; ++i) bias_su[i] = FOM_VERIFY_ITER_SU_BIAS[r][i];
            fom_ref_iterate(x, sc[0], bias_t0, bias_t1, bias_su, &hact_t0, &hact_t1, r_t_t0, r_t_t1, r_new_state);
            m = 0.0;
            for (i = 0; i < 600; ++i) m += ((r_t_t0[i] < 0) ? -r_t_t0[i] : r_t_t0[i]);
            m = (m > 0) ? m / 600 : 1.0;
            for (i = 0; i < 600; ++i) lo_t0[i] = r_t_t0[i] + FOM_VERIFY_ITER_T0_R1[r][i] * m;
            hact_t0.lo = lo_t0;
            for (i = 0; i < 600; ++i) hi_t0[i] = lo_t0[i] + FOM_VERIFY_ITER_T0_R2[r][i] * m;
            hact_t0.hi = hi_t0;
            m = 0.0;
            for (i = 0; i < 20; ++i) m += ((r_t_t1[i] < 0) ? -r_t_t1[i] : r_t_t1[i]);
            m = (m > 0) ? m / 20 : 1.0;
            for (i = 0; i < 20; ++i) lo_t1[i] = r_t_t1[i] + FOM_VERIFY_ITER_T1_R1[r][i] * m;
            hact_t1.lo = lo_t1;
            ++probes;
            for (i = 0; i < 600; ++i) { g_t_t0[i] = (fom_float)(i % 7) - 3.5; r_t_t0[i] = -(fom_float)(i % 5) - 0.25; }
            for (i = 0; i < 20; ++i) { g_t_t1[i] = (fom_float)(i % 7) - 3.5; r_t_t1[i] = -(fom_float)(i % 5) - 0.25; }
            for (i = 0; i < 650; ++i) { g_new_state[i] = (fom_float)(i % 7) - 3.5; r_new_state[i] = -(fom_float)(i % 5) - 0.25; }
            fom_iterate(x, sc[0], bias_t0, bias_t1, bias_su, &hact_t0, &hact_t1, g_t_t0, g_t_t1, g_new_state);
            fom_ref_iterate(x, sc[0], bias_t0, bias_t1, bias_su, &hact_t0, &hact_t1, r_t_t0, r_t_t1, r_new_state);
            err = 0.0; scale = 1.0; bad = 0; which = 0;
            for (i = 0; i < 600; ++i) {
                fom_float d = g_t_t0[i] - r_t_t0[i], a = r_t_t0[i];
                if (d < 0) d = -d;
                if (a < 0) a = -a;
                if (!(d <= err)) { err = d; bad = i; which = 0; }
                if (1.0 + a > scale) scale = 1.0 + a;
            }
            for (i = 0; i < 20; ++i) {
                fom_float d = g_t_t1[i] - r_t_t1[i], a = r_t_t1[i];
                if (d < 0) d = -d;
                if (a < 0) a = -a;
                if (!(d <= err)) { err = d; bad = i; which = 1; }
                if (1.0 + a > scale) scale = 1.0 + a;
            }
            for (i = 0; i < 650; ++i) {
                fom_float d = g_new_state[i] - r_new_state[i], a = r_new_state[i];
                if (d < 0) d = -d;
                if (a < 0) a = -a;
                if (!(d <= err)) { err = d; bad = i; which = 2; }
                if (1.0 + a > scale) scale = 1.0 + a;
            }
            if (!(err <= worst_op)) worst_op = err;
            if (!(err <= ops_tol * scale)) {
                printf("VERIFY FAILED: fom_iterate differs from the reference by %.3e in %s row %d on random probe %d (random activations)\n",
                       (double)err, output[which], bad, r);
                return 1;
            }
            for (i = 0; i < 600; ++i) lo_t0[i] = -FOM_VERIFY_HUGE;
            hact_t0.lo = lo_t0;
            for (i = 0; i < 600; ++i) hi_t0[i] = FOM_VERIFY_HUGE;
            hact_t0.hi = hi_t0;
            for (i = 0; i < 20; ++i) lo_t1[i] = -FOM_VERIFY_HUGE;
            hact_t1.lo = lo_t1;
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
