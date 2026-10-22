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
        static fom_float x[160], got[48], ref[48];
        fom_float worst_op = 0.0;
        int r, i, c;
        for (r = 0; r < 162; ++r) {
            fom_float err = 0.0, scale = 1.0;
            int bad = 0;
            for (c = 0; c < 160; ++c)
                x[c] = (r < 160) ? (fom_float)(c == r) : FOM_VERIFY_OP_T0_STATE_X[r - 160][c];
            for (i = 0; i < 48; ++i) got[i] = ref[i] = (fom_float)(i % 7 - 3);
            fom_apply_t0_state(x, got);
            fom_ref_apply_t0_state(x, ref);
            for (i = 0; i < 48; ++i) {
                fom_float d = got[i] - ref[i];
                fom_float a = ref[i] - (fom_float)(i % 7 - 3);  /* (M x)[i] */
                if (d < 0) d = -d;
                if (a < 0) a = -a;
                if (!(d <= err)) { err = d; bad = i; }  /* NaN-safe */
                if (1.0 + a > scale) scale = 1.0 + a;
            }
            if (!(err <= worst_op)) worst_op = err;
            if (!(err <= ops_tol * scale)) {
                if (r < 160)
                    printf("VERIFY FAILED: fom_apply_t0_state entry (%d, %d) is "
                           "off by %.3e from its matrix\n", bad, r, (double)err);
                else
                    printf("VERIFY FAILED: fom_apply_t0_state differs from its "
                           "matrix by %.3e in row %d on random probe %d\n",
                           (double)err, bad, r - 160);
                return 1;
            }
        }
        printf("OPSCHECK name=t0_state mode=basis probes=162 max_err=%.3e\n", (double)worst_op);
    }
    {
        /* fom_apply_t0_state__d_taux: direct check against its matrix. */
        static fom_float x[160], got[48], ref[48];
        fom_float worst_op = 0.0;
        int r, i, c;
        for (r = 0; r < 162; ++r) {
            fom_float err = 0.0, scale = 1.0;
            int bad = 0;
            for (c = 0; c < 160; ++c)
                x[c] = (r < 160) ? (fom_float)(c == r) : FOM_VERIFY_OP_T0_STATE__D_TAUX_X[r - 160][c];
            for (i = 0; i < 48; ++i) got[i] = ref[i] = (fom_float)(i % 7 - 3);
            fom_apply_t0_state__d_taux(x, got);
            fom_ref_apply_t0_state__d_taux(x, ref);
            for (i = 0; i < 48; ++i) {
                fom_float d = got[i] - ref[i];
                fom_float a = ref[i] - (fom_float)(i % 7 - 3);  /* (M x)[i] */
                if (d < 0) d = -d;
                if (a < 0) a = -a;
                if (!(d <= err)) { err = d; bad = i; }  /* NaN-safe */
                if (1.0 + a > scale) scale = 1.0 + a;
            }
            if (!(err <= worst_op)) worst_op = err;
            if (!(err <= ops_tol * scale)) {
                if (r < 160)
                    printf("VERIFY FAILED: fom_apply_t0_state__d_taux entry (%d, %d) is "
                           "off by %.3e from its matrix\n", bad, r, (double)err);
                else
                    printf("VERIFY FAILED: fom_apply_t0_state__d_taux differs from its "
                           "matrix by %.3e in row %d on random probe %d\n",
                           (double)err, bad, r - 160);
                return 1;
            }
        }
        printf("OPSCHECK name=t0_state__d_taux mode=basis probes=162 max_err=%.3e\n", (double)worst_op);
    }
    {
        /* fom_apply_t0_state__d_tth: direct check against its matrix. */
        static fom_float x[160], got[48], ref[48];
        fom_float worst_op = 0.0;
        int r, i, c;
        for (r = 0; r < 162; ++r) {
            fom_float err = 0.0, scale = 1.0;
            int bad = 0;
            for (c = 0; c < 160; ++c)
                x[c] = (r < 160) ? (fom_float)(c == r) : FOM_VERIFY_OP_T0_STATE__D_TTH_X[r - 160][c];
            for (i = 0; i < 48; ++i) got[i] = ref[i] = (fom_float)(i % 7 - 3);
            fom_apply_t0_state__d_tth(x, got);
            fom_ref_apply_t0_state__d_tth(x, ref);
            for (i = 0; i < 48; ++i) {
                fom_float d = got[i] - ref[i];
                fom_float a = ref[i] - (fom_float)(i % 7 - 3);  /* (M x)[i] */
                if (d < 0) d = -d;
                if (a < 0) a = -a;
                if (!(d <= err)) { err = d; bad = i; }  /* NaN-safe */
                if (1.0 + a > scale) scale = 1.0 + a;
            }
            if (!(err <= worst_op)) worst_op = err;
            if (!(err <= ops_tol * scale)) {
                if (r < 160)
                    printf("VERIFY FAILED: fom_apply_t0_state__d_tth entry (%d, %d) is "
                           "off by %.3e from its matrix\n", bad, r, (double)err);
                else
                    printf("VERIFY FAILED: fom_apply_t0_state__d_tth differs from its "
                           "matrix by %.3e in row %d on random probe %d\n",
                           (double)err, bad, r - 160);
                return 1;
            }
        }
        printf("OPSCHECK name=t0_state__d_tth mode=basis probes=162 max_err=%.3e\n", (double)worst_op);
    }
    {
        /* fom_apply_t0_p_p1: direct check against its matrix. */
        static fom_float x[48], got[48], ref[48];
        fom_float worst_op = 0.0;
        int r, i, c;
        for (r = 0; r < 50; ++r) {
            fom_float err = 0.0, scale = 1.0;
            int bad = 0;
            for (c = 0; c < 48; ++c)
                x[c] = (r < 48) ? (fom_float)(c == r) : FOM_VERIFY_OP_T0_P_P1_X[r - 48][c];
            for (i = 0; i < 48; ++i) got[i] = ref[i] = (fom_float)(i % 7 - 3);
            fom_apply_t0_p_p1(x, got);
            fom_ref_apply_t0_p_p1(x, ref);
            for (i = 0; i < 48; ++i) {
                fom_float d = got[i] - ref[i];
                fom_float a = ref[i] - (fom_float)(i % 7 - 3);  /* (M x)[i] */
                if (d < 0) d = -d;
                if (a < 0) a = -a;
                if (!(d <= err)) { err = d; bad = i; }  /* NaN-safe */
                if (1.0 + a > scale) scale = 1.0 + a;
            }
            if (!(err <= worst_op)) worst_op = err;
            if (!(err <= ops_tol * scale)) {
                if (r < 48)
                    printf("VERIFY FAILED: fom_apply_t0_p_p1 entry (%d, %d) is "
                           "off by %.3e from its matrix\n", bad, r, (double)err);
                else
                    printf("VERIFY FAILED: fom_apply_t0_p_p1 differs from its "
                           "matrix by %.3e in row %d on random probe %d\n",
                           (double)err, bad, r - 48);
                return 1;
            }
        }
        printf("OPSCHECK name=t0_p_p1 mode=basis probes=50 max_err=%.3e\n", (double)worst_op);
    }
    {
        /* fom_apply_t0_p_p1__d_tau: direct check against its matrix. */
        static fom_float x[48], got[48], ref[48];
        fom_float worst_op = 0.0;
        int r, i, c;
        for (r = 0; r < 50; ++r) {
            fom_float err = 0.0, scale = 1.0;
            int bad = 0;
            for (c = 0; c < 48; ++c)
                x[c] = (r < 48) ? (fom_float)(c == r) : FOM_VERIFY_OP_T0_P_P1__D_TAU_X[r - 48][c];
            for (i = 0; i < 48; ++i) got[i] = ref[i] = (fom_float)(i % 7 - 3);
            fom_apply_t0_p_p1__d_tau(x, got);
            fom_ref_apply_t0_p_p1__d_tau(x, ref);
            for (i = 0; i < 48; ++i) {
                fom_float d = got[i] - ref[i];
                fom_float a = ref[i] - (fom_float)(i % 7 - 3);  /* (M x)[i] */
                if (d < 0) d = -d;
                if (a < 0) a = -a;
                if (!(d <= err)) { err = d; bad = i; }  /* NaN-safe */
                if (1.0 + a > scale) scale = 1.0 + a;
            }
            if (!(err <= worst_op)) worst_op = err;
            if (!(err <= ops_tol * scale)) {
                if (r < 48)
                    printf("VERIFY FAILED: fom_apply_t0_p_p1__d_tau entry (%d, %d) is "
                           "off by %.3e from its matrix\n", bad, r, (double)err);
                else
                    printf("VERIFY FAILED: fom_apply_t0_p_p1__d_tau differs from its "
                           "matrix by %.3e in row %d on random probe %d\n",
                           (double)err, bad, r - 48);
                return 1;
            }
        }
        printf("OPSCHECK name=t0_p_p1__d_tau mode=basis probes=50 max_err=%.3e\n", (double)worst_op);
    }
    {
        /* fom_apply_t1_state: direct check against its matrix. */
        static fom_float x[160], got[16], ref[16];
        fom_float worst_op = 0.0;
        int r, i, c;
        for (r = 0; r < 162; ++r) {
            fom_float err = 0.0, scale = 1.0;
            int bad = 0;
            for (c = 0; c < 160; ++c)
                x[c] = (r < 160) ? (fom_float)(c == r) : FOM_VERIFY_OP_T1_STATE_X[r - 160][c];
            for (i = 0; i < 16; ++i) got[i] = ref[i] = (fom_float)(i % 7 - 3);
            fom_apply_t1_state(x, got);
            fom_ref_apply_t1_state(x, ref);
            for (i = 0; i < 16; ++i) {
                fom_float d = got[i] - ref[i];
                fom_float a = ref[i] - (fom_float)(i % 7 - 3);  /* (M x)[i] */
                if (d < 0) d = -d;
                if (a < 0) a = -a;
                if (!(d <= err)) { err = d; bad = i; }  /* NaN-safe */
                if (1.0 + a > scale) scale = 1.0 + a;
            }
            if (!(err <= worst_op)) worst_op = err;
            if (!(err <= ops_tol * scale)) {
                if (r < 160)
                    printf("VERIFY FAILED: fom_apply_t1_state entry (%d, %d) is "
                           "off by %.3e from its matrix\n", bad, r, (double)err);
                else
                    printf("VERIFY FAILED: fom_apply_t1_state differs from its "
                           "matrix by %.3e in row %d on random probe %d\n",
                           (double)err, bad, r - 160);
                return 1;
            }
        }
        printf("OPSCHECK name=t1_state mode=basis probes=162 max_err=%.3e\n", (double)worst_op);
    }
    {
        /* fom_apply_t1_state__d_eta: direct check against its matrix. */
        static fom_float x[160], got[16], ref[16];
        fom_float worst_op = 0.0;
        int r, i, c;
        for (r = 0; r < 162; ++r) {
            fom_float err = 0.0, scale = 1.0;
            int bad = 0;
            for (c = 0; c < 160; ++c)
                x[c] = (r < 160) ? (fom_float)(c == r) : FOM_VERIFY_OP_T1_STATE__D_ETA_X[r - 160][c];
            for (i = 0; i < 16; ++i) got[i] = ref[i] = (fom_float)(i % 7 - 3);
            fom_apply_t1_state__d_eta(x, got);
            fom_ref_apply_t1_state__d_eta(x, ref);
            for (i = 0; i < 16; ++i) {
                fom_float d = got[i] - ref[i];
                fom_float a = ref[i] - (fom_float)(i % 7 - 3);  /* (M x)[i] */
                if (d < 0) d = -d;
                if (a < 0) a = -a;
                if (!(d <= err)) { err = d; bad = i; }  /* NaN-safe */
                if (1.0 + a > scale) scale = 1.0 + a;
            }
            if (!(err <= worst_op)) worst_op = err;
            if (!(err <= ops_tol * scale)) {
                if (r < 160)
                    printf("VERIFY FAILED: fom_apply_t1_state__d_eta entry (%d, %d) is "
                           "off by %.3e from its matrix\n", bad, r, (double)err);
                else
                    printf("VERIFY FAILED: fom_apply_t1_state__d_eta differs from its "
                           "matrix by %.3e in row %d on random probe %d\n",
                           (double)err, bad, r - 160);
                return 1;
            }
        }
        printf("OPSCHECK name=t1_state__d_eta mode=basis probes=162 max_err=%.3e\n", (double)worst_op);
    }
    {
        /* fom_apply_t1_p_p0: direct check against its matrix. */
        static fom_float x[16], got[16], ref[16];
        fom_float worst_op = 0.0;
        int r, i, c;
        for (r = 0; r < 18; ++r) {
            fom_float err = 0.0, scale = 1.0;
            int bad = 0;
            for (c = 0; c < 16; ++c)
                x[c] = (r < 16) ? (fom_float)(c == r) : FOM_VERIFY_OP_T1_P_P0_X[r - 16][c];
            for (i = 0; i < 16; ++i) got[i] = ref[i] = (fom_float)(i % 7 - 3);
            fom_apply_t1_p_p0(x, got);
            fom_ref_apply_t1_p_p0(x, ref);
            for (i = 0; i < 16; ++i) {
                fom_float d = got[i] - ref[i];
                fom_float a = ref[i] - (fom_float)(i % 7 - 3);  /* (M x)[i] */
                if (d < 0) d = -d;
                if (a < 0) a = -a;
                if (!(d <= err)) { err = d; bad = i; }  /* NaN-safe */
                if (1.0 + a > scale) scale = 1.0 + a;
            }
            if (!(err <= worst_op)) worst_op = err;
            if (!(err <= ops_tol * scale)) {
                if (r < 16)
                    printf("VERIFY FAILED: fom_apply_t1_p_p0 entry (%d, %d) is "
                           "off by %.3e from its matrix\n", bad, r, (double)err);
                else
                    printf("VERIFY FAILED: fom_apply_t1_p_p0 differs from its "
                           "matrix by %.3e in row %d on random probe %d\n",
                           (double)err, bad, r - 16);
                return 1;
            }
        }
        printf("OPSCHECK name=t1_p_p0 mode=basis probes=18 max_err=%.3e\n", (double)worst_op);
    }
    {
        /* fom_apply_t1_p_p0__d_eta: direct check against its matrix. */
        static fom_float x[16], got[16], ref[16];
        fom_float worst_op = 0.0;
        int r, i, c;
        for (r = 0; r < 18; ++r) {
            fom_float err = 0.0, scale = 1.0;
            int bad = 0;
            for (c = 0; c < 16; ++c)
                x[c] = (r < 16) ? (fom_float)(c == r) : FOM_VERIFY_OP_T1_P_P0__D_ETA_X[r - 16][c];
            for (i = 0; i < 16; ++i) got[i] = ref[i] = (fom_float)(i % 7 - 3);
            fom_apply_t1_p_p0__d_eta(x, got);
            fom_ref_apply_t1_p_p0__d_eta(x, ref);
            for (i = 0; i < 16; ++i) {
                fom_float d = got[i] - ref[i];
                fom_float a = ref[i] - (fom_float)(i % 7 - 3);  /* (M x)[i] */
                if (d < 0) d = -d;
                if (a < 0) a = -a;
                if (!(d <= err)) { err = d; bad = i; }  /* NaN-safe */
                if (1.0 + a > scale) scale = 1.0 + a;
            }
            if (!(err <= worst_op)) worst_op = err;
            if (!(err <= ops_tol * scale)) {
                if (r < 16)
                    printf("VERIFY FAILED: fom_apply_t1_p_p0__d_eta entry (%d, %d) is "
                           "off by %.3e from its matrix\n", bad, r, (double)err);
                else
                    printf("VERIFY FAILED: fom_apply_t1_p_p0__d_eta differs from its "
                           "matrix by %.3e in row %d on random probe %d\n",
                           (double)err, bad, r - 16);
                return 1;
            }
        }
        printf("OPSCHECK name=t1_p_p0__d_eta mode=basis probes=18 max_err=%.3e\n", (double)worst_op);
    }
    {
        /* fom_apply_t1_t_t0: direct check against its matrix. */
        static fom_float x[48], got[16], ref[16];
        fom_float worst_op = 0.0;
        int r, i, c;
        for (r = 0; r < 50; ++r) {
            fom_float err = 0.0, scale = 1.0;
            int bad = 0;
            for (c = 0; c < 48; ++c)
                x[c] = (r < 48) ? (fom_float)(c == r) : FOM_VERIFY_OP_T1_T_T0_X[r - 48][c];
            for (i = 0; i < 16; ++i) got[i] = ref[i] = (fom_float)(i % 7 - 3);
            fom_apply_t1_t_t0(x, got);
            fom_ref_apply_t1_t_t0(x, ref);
            for (i = 0; i < 16; ++i) {
                fom_float d = got[i] - ref[i];
                fom_float a = ref[i] - (fom_float)(i % 7 - 3);  /* (M x)[i] */
                if (d < 0) d = -d;
                if (a < 0) a = -a;
                if (!(d <= err)) { err = d; bad = i; }  /* NaN-safe */
                if (1.0 + a > scale) scale = 1.0 + a;
            }
            if (!(err <= worst_op)) worst_op = err;
            if (!(err <= ops_tol * scale)) {
                if (r < 48)
                    printf("VERIFY FAILED: fom_apply_t1_t_t0 entry (%d, %d) is "
                           "off by %.3e from its matrix\n", bad, r, (double)err);
                else
                    printf("VERIFY FAILED: fom_apply_t1_t_t0 differs from its "
                           "matrix by %.3e in row %d on random probe %d\n",
                           (double)err, bad, r - 48);
                return 1;
            }
        }
        printf("OPSCHECK name=t1_t_t0 mode=basis probes=50 max_err=%.3e\n", (double)worst_op);
    }
    {
        /* fom_apply_t1_t_t0__d_eta: direct check against its matrix. */
        static fom_float x[48], got[16], ref[16];
        fom_float worst_op = 0.0;
        int r, i, c;
        for (r = 0; r < 50; ++r) {
            fom_float err = 0.0, scale = 1.0;
            int bad = 0;
            for (c = 0; c < 48; ++c)
                x[c] = (r < 48) ? (fom_float)(c == r) : FOM_VERIFY_OP_T1_T_T0__D_ETA_X[r - 48][c];
            for (i = 0; i < 16; ++i) got[i] = ref[i] = (fom_float)(i % 7 - 3);
            fom_apply_t1_t_t0__d_eta(x, got);
            fom_ref_apply_t1_t_t0__d_eta(x, ref);
            for (i = 0; i < 16; ++i) {
                fom_float d = got[i] - ref[i];
                fom_float a = ref[i] - (fom_float)(i % 7 - 3);  /* (M x)[i] */
                if (d < 0) d = -d;
                if (a < 0) a = -a;
                if (!(d <= err)) { err = d; bad = i; }  /* NaN-safe */
                if (1.0 + a > scale) scale = 1.0 + a;
            }
            if (!(err <= worst_op)) worst_op = err;
            if (!(err <= ops_tol * scale)) {
                if (r < 48)
                    printf("VERIFY FAILED: fom_apply_t1_t_t0__d_eta entry (%d, %d) is "
                           "off by %.3e from its matrix\n", bad, r, (double)err);
                else
                    printf("VERIFY FAILED: fom_apply_t1_t_t0__d_eta differs from its "
                           "matrix by %.3e in row %d on random probe %d\n",
                           (double)err, bad, r - 48);
                return 1;
            }
        }
        printf("OPSCHECK name=t1_t_t0__d_eta mode=basis probes=50 max_err=%.3e\n", (double)worst_op);
    }
    {
        /* fom_apply_su_state: direct check against its matrix. */
        static fom_float x[160], got[160], ref[160];
        fom_float worst_op = 0.0;
        int r, i, c;
        for (r = 0; r < 162; ++r) {
            fom_float err = 0.0, scale = 1.0;
            int bad = 0;
            for (c = 0; c < 160; ++c)
                x[c] = (r < 160) ? (fom_float)(c == r) : FOM_VERIFY_OP_SU_STATE_X[r - 160][c];
            for (i = 0; i < 160; ++i) got[i] = ref[i] = (fom_float)(i % 7 - 3);
            fom_apply_su_state(x, got);
            fom_ref_apply_su_state(x, ref);
            for (i = 0; i < 160; ++i) {
                fom_float d = got[i] - ref[i];
                fom_float a = ref[i] - (fom_float)(i % 7 - 3);  /* (M x)[i] */
                if (d < 0) d = -d;
                if (a < 0) a = -a;
                if (!(d <= err)) { err = d; bad = i; }  /* NaN-safe */
                if (1.0 + a > scale) scale = 1.0 + a;
            }
            if (!(err <= worst_op)) worst_op = err;
            if (!(err <= ops_tol * scale)) {
                if (r < 160)
                    printf("VERIFY FAILED: fom_apply_su_state entry (%d, %d) is "
                           "off by %.3e from its matrix\n", bad, r, (double)err);
                else
                    printf("VERIFY FAILED: fom_apply_su_state differs from its "
                           "matrix by %.3e in row %d on random probe %d\n",
                           (double)err, bad, r - 160);
                return 1;
            }
        }
        printf("OPSCHECK name=su_state mode=basis probes=162 max_err=%.3e\n", (double)worst_op);
    }
    {
        /* fom_apply_su_state__d_d1: direct check against its matrix. */
        static fom_float x[160], got[160], ref[160];
        fom_float worst_op = 0.0;
        int r, i, c;
        for (r = 0; r < 162; ++r) {
            fom_float err = 0.0, scale = 1.0;
            int bad = 0;
            for (c = 0; c < 160; ++c)
                x[c] = (r < 160) ? (fom_float)(c == r) : FOM_VERIFY_OP_SU_STATE__D_D1_X[r - 160][c];
            for (i = 0; i < 160; ++i) got[i] = ref[i] = (fom_float)(i % 7 - 3);
            fom_apply_su_state__d_d1(x, got);
            fom_ref_apply_su_state__d_d1(x, ref);
            for (i = 0; i < 160; ++i) {
                fom_float d = got[i] - ref[i];
                fom_float a = ref[i] - (fom_float)(i % 7 - 3);  /* (M x)[i] */
                if (d < 0) d = -d;
                if (a < 0) a = -a;
                if (!(d <= err)) { err = d; bad = i; }  /* NaN-safe */
                if (1.0 + a > scale) scale = 1.0 + a;
            }
            if (!(err <= worst_op)) worst_op = err;
            if (!(err <= ops_tol * scale)) {
                if (r < 160)
                    printf("VERIFY FAILED: fom_apply_su_state__d_d1 entry (%d, %d) is "
                           "off by %.3e from its matrix\n", bad, r, (double)err);
                else
                    printf("VERIFY FAILED: fom_apply_su_state__d_d1 differs from its "
                           "matrix by %.3e in row %d on random probe %d\n",
                           (double)err, bad, r - 160);
                return 1;
            }
        }
        printf("OPSCHECK name=su_state__d_d1 mode=basis probes=162 max_err=%.3e\n", (double)worst_op);
    }
    {
        /* fom_apply_su_state__d_g2: direct check against its matrix. */
        static fom_float x[160], got[160], ref[160];
        fom_float worst_op = 0.0;
        int r, i, c;
        for (r = 0; r < 162; ++r) {
            fom_float err = 0.0, scale = 1.0;
            int bad = 0;
            for (c = 0; c < 160; ++c)
                x[c] = (r < 160) ? (fom_float)(c == r) : FOM_VERIFY_OP_SU_STATE__D_G2_X[r - 160][c];
            for (i = 0; i < 160; ++i) got[i] = ref[i] = (fom_float)(i % 7 - 3);
            fom_apply_su_state__d_g2(x, got);
            fom_ref_apply_su_state__d_g2(x, ref);
            for (i = 0; i < 160; ++i) {
                fom_float d = got[i] - ref[i];
                fom_float a = ref[i] - (fom_float)(i % 7 - 3);  /* (M x)[i] */
                if (d < 0) d = -d;
                if (a < 0) a = -a;
                if (!(d <= err)) { err = d; bad = i; }  /* NaN-safe */
                if (1.0 + a > scale) scale = 1.0 + a;
            }
            if (!(err <= worst_op)) worst_op = err;
            if (!(err <= ops_tol * scale)) {
                if (r < 160)
                    printf("VERIFY FAILED: fom_apply_su_state__d_g2 entry (%d, %d) is "
                           "off by %.3e from its matrix\n", bad, r, (double)err);
                else
                    printf("VERIFY FAILED: fom_apply_su_state__d_g2 differs from its "
                           "matrix by %.3e in row %d on random probe %d\n",
                           (double)err, bad, r - 160);
                return 1;
            }
        }
        printf("OPSCHECK name=su_state__d_g2 mode=basis probes=162 max_err=%.3e\n", (double)worst_op);
    }
    {
        /* fom_apply_su_state__d_m1: direct check against its matrix. */
        static fom_float x[160], got[160], ref[160];
        fom_float worst_op = 0.0;
        int r, i, c;
        for (r = 0; r < 162; ++r) {
            fom_float err = 0.0, scale = 1.0;
            int bad = 0;
            for (c = 0; c < 160; ++c)
                x[c] = (r < 160) ? (fom_float)(c == r) : FOM_VERIFY_OP_SU_STATE__D_M1_X[r - 160][c];
            for (i = 0; i < 160; ++i) got[i] = ref[i] = (fom_float)(i % 7 - 3);
            fom_apply_su_state__d_m1(x, got);
            fom_ref_apply_su_state__d_m1(x, ref);
            for (i = 0; i < 160; ++i) {
                fom_float d = got[i] - ref[i];
                fom_float a = ref[i] - (fom_float)(i % 7 - 3);  /* (M x)[i] */
                if (d < 0) d = -d;
                if (a < 0) a = -a;
                if (!(d <= err)) { err = d; bad = i; }  /* NaN-safe */
                if (1.0 + a > scale) scale = 1.0 + a;
            }
            if (!(err <= worst_op)) worst_op = err;
            if (!(err <= ops_tol * scale)) {
                if (r < 160)
                    printf("VERIFY FAILED: fom_apply_su_state__d_m1 entry (%d, %d) is "
                           "off by %.3e from its matrix\n", bad, r, (double)err);
                else
                    printf("VERIFY FAILED: fom_apply_su_state__d_m1 differs from its "
                           "matrix by %.3e in row %d on random probe %d\n",
                           (double)err, bad, r - 160);
                return 1;
            }
        }
        printf("OPSCHECK name=su_state__d_m1 mode=basis probes=162 max_err=%.3e\n", (double)worst_op);
    }
    {
        /* fom_apply_su_t_t0: direct check against its matrix. */
        static fom_float x[48], got[160], ref[160];
        fom_float worst_op = 0.0;
        int r, i, c;
        for (r = 0; r < 50; ++r) {
            fom_float err = 0.0, scale = 1.0;
            int bad = 0;
            for (c = 0; c < 48; ++c)
                x[c] = (r < 48) ? (fom_float)(c == r) : FOM_VERIFY_OP_SU_T_T0_X[r - 48][c];
            for (i = 0; i < 160; ++i) got[i] = ref[i] = (fom_float)(i % 7 - 3);
            fom_apply_su_t_t0(x, got);
            fom_ref_apply_su_t_t0(x, ref);
            for (i = 0; i < 160; ++i) {
                fom_float d = got[i] - ref[i];
                fom_float a = ref[i] - (fom_float)(i % 7 - 3);  /* (M x)[i] */
                if (d < 0) d = -d;
                if (a < 0) a = -a;
                if (!(d <= err)) { err = d; bad = i; }  /* NaN-safe */
                if (1.0 + a > scale) scale = 1.0 + a;
            }
            if (!(err <= worst_op)) worst_op = err;
            if (!(err <= ops_tol * scale)) {
                if (r < 48)
                    printf("VERIFY FAILED: fom_apply_su_t_t0 entry (%d, %d) is "
                           "off by %.3e from its matrix\n", bad, r, (double)err);
                else
                    printf("VERIFY FAILED: fom_apply_su_t_t0 differs from its "
                           "matrix by %.3e in row %d on random probe %d\n",
                           (double)err, bad, r - 48);
                return 1;
            }
        }
        printf("OPSCHECK name=su_t_t0 mode=basis probes=50 max_err=%.3e\n", (double)worst_op);
    }
    {
        /* fom_apply_su_t_t0__d_d2: direct check against its matrix. */
        static fom_float x[48], got[160], ref[160];
        fom_float worst_op = 0.0;
        int r, i, c;
        for (r = 0; r < 50; ++r) {
            fom_float err = 0.0, scale = 1.0;
            int bad = 0;
            for (c = 0; c < 48; ++c)
                x[c] = (r < 48) ? (fom_float)(c == r) : FOM_VERIFY_OP_SU_T_T0__D_D2_X[r - 48][c];
            for (i = 0; i < 160; ++i) got[i] = ref[i] = (fom_float)(i % 7 - 3);
            fom_apply_su_t_t0__d_d2(x, got);
            fom_ref_apply_su_t_t0__d_d2(x, ref);
            for (i = 0; i < 160; ++i) {
                fom_float d = got[i] - ref[i];
                fom_float a = ref[i] - (fom_float)(i % 7 - 3);  /* (M x)[i] */
                if (d < 0) d = -d;
                if (a < 0) a = -a;
                if (!(d <= err)) { err = d; bad = i; }  /* NaN-safe */
                if (1.0 + a > scale) scale = 1.0 + a;
            }
            if (!(err <= worst_op)) worst_op = err;
            if (!(err <= ops_tol * scale)) {
                if (r < 48)
                    printf("VERIFY FAILED: fom_apply_su_t_t0__d_d2 entry (%d, %d) is "
                           "off by %.3e from its matrix\n", bad, r, (double)err);
                else
                    printf("VERIFY FAILED: fom_apply_su_t_t0__d_d2 differs from its "
                           "matrix by %.3e in row %d on random probe %d\n",
                           (double)err, bad, r - 48);
                return 1;
            }
        }
        printf("OPSCHECK name=su_t_t0__d_d2 mode=basis probes=50 max_err=%.3e\n", (double)worst_op);
    }
    {
        /* fom_apply_su_t_t0__d_g1: direct check against its matrix. */
        static fom_float x[48], got[160], ref[160];
        fom_float worst_op = 0.0;
        int r, i, c;
        for (r = 0; r < 50; ++r) {
            fom_float err = 0.0, scale = 1.0;
            int bad = 0;
            for (c = 0; c < 48; ++c)
                x[c] = (r < 48) ? (fom_float)(c == r) : FOM_VERIFY_OP_SU_T_T0__D_G1_X[r - 48][c];
            for (i = 0; i < 160; ++i) got[i] = ref[i] = (fom_float)(i % 7 - 3);
            fom_apply_su_t_t0__d_g1(x, got);
            fom_ref_apply_su_t_t0__d_g1(x, ref);
            for (i = 0; i < 160; ++i) {
                fom_float d = got[i] - ref[i];
                fom_float a = ref[i] - (fom_float)(i % 7 - 3);  /* (M x)[i] */
                if (d < 0) d = -d;
                if (a < 0) a = -a;
                if (!(d <= err)) { err = d; bad = i; }  /* NaN-safe */
                if (1.0 + a > scale) scale = 1.0 + a;
            }
            if (!(err <= worst_op)) worst_op = err;
            if (!(err <= ops_tol * scale)) {
                if (r < 48)
                    printf("VERIFY FAILED: fom_apply_su_t_t0__d_g1 entry (%d, %d) is "
                           "off by %.3e from its matrix\n", bad, r, (double)err);
                else
                    printf("VERIFY FAILED: fom_apply_su_t_t0__d_g1 differs from its "
                           "matrix by %.3e in row %d on random probe %d\n",
                           (double)err, bad, r - 48);
                return 1;
            }
        }
        printf("OPSCHECK name=su_t_t0__d_g1 mode=basis probes=50 max_err=%.3e\n", (double)worst_op);
    }
    {
        /* fom_apply_su_t_t1: direct check against its matrix. */
        static fom_float x[16], got[160], ref[160];
        fom_float worst_op = 0.0;
        int r, i, c;
        for (r = 0; r < 18; ++r) {
            fom_float err = 0.0, scale = 1.0;
            int bad = 0;
            for (c = 0; c < 16; ++c)
                x[c] = (r < 16) ? (fom_float)(c == r) : FOM_VERIFY_OP_SU_T_T1_X[r - 16][c];
            for (i = 0; i < 160; ++i) got[i] = ref[i] = (fom_float)(i % 7 - 3);
            fom_apply_su_t_t1(x, got);
            fom_ref_apply_su_t_t1(x, ref);
            for (i = 0; i < 160; ++i) {
                fom_float d = got[i] - ref[i];
                fom_float a = ref[i] - (fom_float)(i % 7 - 3);  /* (M x)[i] */
                if (d < 0) d = -d;
                if (a < 0) a = -a;
                if (!(d <= err)) { err = d; bad = i; }  /* NaN-safe */
                if (1.0 + a > scale) scale = 1.0 + a;
            }
            if (!(err <= worst_op)) worst_op = err;
            if (!(err <= ops_tol * scale)) {
                if (r < 16)
                    printf("VERIFY FAILED: fom_apply_su_t_t1 entry (%d, %d) is "
                           "off by %.3e from its matrix\n", bad, r, (double)err);
                else
                    printf("VERIFY FAILED: fom_apply_su_t_t1 differs from its "
                           "matrix by %.3e in row %d on random probe %d\n",
                           (double)err, bad, r - 16);
                return 1;
            }
        }
        printf("OPSCHECK name=su_t_t1 mode=basis probes=18 max_err=%.3e\n", (double)worst_op);
    }
    {
        /* fom_apply_su_t_t1__d_d2: direct check against its matrix. */
        static fom_float x[16], got[160], ref[160];
        fom_float worst_op = 0.0;
        int r, i, c;
        for (r = 0; r < 18; ++r) {
            fom_float err = 0.0, scale = 1.0;
            int bad = 0;
            for (c = 0; c < 16; ++c)
                x[c] = (r < 16) ? (fom_float)(c == r) : FOM_VERIFY_OP_SU_T_T1__D_D2_X[r - 16][c];
            for (i = 0; i < 160; ++i) got[i] = ref[i] = (fom_float)(i % 7 - 3);
            fom_apply_su_t_t1__d_d2(x, got);
            fom_ref_apply_su_t_t1__d_d2(x, ref);
            for (i = 0; i < 160; ++i) {
                fom_float d = got[i] - ref[i];
                fom_float a = ref[i] - (fom_float)(i % 7 - 3);  /* (M x)[i] */
                if (d < 0) d = -d;
                if (a < 0) a = -a;
                if (!(d <= err)) { err = d; bad = i; }  /* NaN-safe */
                if (1.0 + a > scale) scale = 1.0 + a;
            }
            if (!(err <= worst_op)) worst_op = err;
            if (!(err <= ops_tol * scale)) {
                if (r < 16)
                    printf("VERIFY FAILED: fom_apply_su_t_t1__d_d2 entry (%d, %d) is "
                           "off by %.3e from its matrix\n", bad, r, (double)err);
                else
                    printf("VERIFY FAILED: fom_apply_su_t_t1__d_d2 differs from its "
                           "matrix by %.3e in row %d on random probe %d\n",
                           (double)err, bad, r - 16);
                return 1;
            }
        }
        printf("OPSCHECK name=su_t_t1__d_d2 mode=basis probes=18 max_err=%.3e\n", (double)worst_op);
    }
    {
        /* fom_apply_su_t_t1__d_g1: direct check against its matrix. */
        static fom_float x[16], got[160], ref[160];
        fom_float worst_op = 0.0;
        int r, i, c;
        for (r = 0; r < 18; ++r) {
            fom_float err = 0.0, scale = 1.0;
            int bad = 0;
            for (c = 0; c < 16; ++c)
                x[c] = (r < 16) ? (fom_float)(c == r) : FOM_VERIFY_OP_SU_T_T1__D_G1_X[r - 16][c];
            for (i = 0; i < 160; ++i) got[i] = ref[i] = (fom_float)(i % 7 - 3);
            fom_apply_su_t_t1__d_g1(x, got);
            fom_ref_apply_su_t_t1__d_g1(x, ref);
            for (i = 0; i < 160; ++i) {
                fom_float d = got[i] - ref[i];
                fom_float a = ref[i] - (fom_float)(i % 7 - 3);  /* (M x)[i] */
                if (d < 0) d = -d;
                if (a < 0) a = -a;
                if (!(d <= err)) { err = d; bad = i; }  /* NaN-safe */
                if (1.0 + a > scale) scale = 1.0 + a;
            }
            if (!(err <= worst_op)) worst_op = err;
            if (!(err <= ops_tol * scale)) {
                if (r < 16)
                    printf("VERIFY FAILED: fom_apply_su_t_t1__d_g1 entry (%d, %d) is "
                           "off by %.3e from its matrix\n", bad, r, (double)err);
                else
                    printf("VERIFY FAILED: fom_apply_su_t_t1__d_g1 differs from its "
                           "matrix by %.3e in row %d on random probe %d\n",
                           (double)err, bad, r - 16);
                return 1;
            }
        }
        printf("OPSCHECK name=su_t_t1__d_g1 mode=basis probes=18 max_err=%.3e\n", (double)worst_op);
    }
    {
        /* fom_apply_su_t_t1__d_m2: direct check against its matrix. */
        static fom_float x[16], got[160], ref[160];
        fom_float worst_op = 0.0;
        int r, i, c;
        for (r = 0; r < 18; ++r) {
            fom_float err = 0.0, scale = 1.0;
            int bad = 0;
            for (c = 0; c < 16; ++c)
                x[c] = (r < 16) ? (fom_float)(c == r) : FOM_VERIFY_OP_SU_T_T1__D_M2_X[r - 16][c];
            for (i = 0; i < 160; ++i) got[i] = ref[i] = (fom_float)(i % 7 - 3);
            fom_apply_su_t_t1__d_m2(x, got);
            fom_ref_apply_su_t_t1__d_m2(x, ref);
            for (i = 0; i < 160; ++i) {
                fom_float d = got[i] - ref[i];
                fom_float a = ref[i] - (fom_float)(i % 7 - 3);  /* (M x)[i] */
                if (d < 0) d = -d;
                if (a < 0) a = -a;
                if (!(d <= err)) { err = d; bad = i; }  /* NaN-safe */
                if (1.0 + a > scale) scale = 1.0 + a;
            }
            if (!(err <= worst_op)) worst_op = err;
            if (!(err <= ops_tol * scale)) {
                if (r < 16)
                    printf("VERIFY FAILED: fom_apply_su_t_t1__d_m2 entry (%d, %d) is "
                           "off by %.3e from its matrix\n", bad, r, (double)err);
                else
                    printf("VERIFY FAILED: fom_apply_su_t_t1__d_m2 differs from its "
                           "matrix by %.3e in row %d on random probe %d\n",
                           (double)err, bad, r - 16);
                return 1;
            }
        }
        printf("OPSCHECK name=su_t_t1__d_m2 mode=basis probes=18 max_err=%.3e\n", (double)worst_op);
    }
    {
        /* fom_fused_t0: direct check against its members, out += t0_state @ state + sc_taux * (t0_state__d_taux @ state) + sc_tth * (t0_state__d_tth @ state). */
        static fom_float x0[160];
        static fom_float got[48], ref[48];
        fom_float sc[2];
        static const char *setting[] = {"all schedule values 0", "sc_taux = 1", "sc_tth = 1"};
        fom_float worst_op = 0.0, err, scale;
        int r, i, c, j, bad, probes = 0;
        (void)setting; (void)j; (void)sc;
        for (j = 0; j < 160; ++j) {
          for (r = 0; r <= 2; ++r) {
            for (c = 0; c < 160; ++c) x0[c] = 0.0;
            x0[j] = 1.0;
            sc[0] = (fom_float)(r == 1);
            sc[1] = (fom_float)(r == 2);
            ++probes;
            for (i = 0; i < 48; ++i) got[i] = ref[i] = (fom_float)(i % 7 - 3);
            fom_fused_t0(x0, sc[0], sc[1], got);
            fom_ref_fused_t0(x0, sc[0], sc[1], ref);
            err = 0.0; scale = 1.0; bad = 0;
            for (i = 0; i < 48; ++i) {
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
            for (c = 0; c < 160; ++c) x0[c] = FOM_VERIFY_FUSED_T0_X0[r][c];
            sc[0] = FOM_VERIFY_FUSED_T0_SC[r][0];
            sc[1] = FOM_VERIFY_FUSED_T0_SC[r][1];
            ++probes;
            for (i = 0; i < 48; ++i) got[i] = ref[i] = (fom_float)(i % 7 - 3);
            fom_fused_t0(x0, sc[0], sc[1], got);
            fom_ref_fused_t0(x0, sc[0], sc[1], ref);
            err = 0.0; scale = 1.0; bad = 0;
            for (i = 0; i < 48; ++i) {
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
        /* fom_fused_t1: direct check against its members, out += t1_state @ state + sc_eta * (t1_state__d_eta @ state) + sc_eta * (t1_t_t0__d_eta @ t_t0). */
        static fom_float x0[160];
        static fom_float x1[48];
        static fom_float got[16], ref[16];
        fom_float sc[1];
        static const char *setting[] = {"all schedule values 0", "sc_eta = 1"};
        fom_float worst_op = 0.0, err, scale;
        int r, i, c, j, bad, probes = 0;
        (void)setting; (void)j; (void)sc;
        for (j = 0; j < 160; ++j) {
          for (r = 0; r <= 1; ++r) {
            for (c = 0; c < 160; ++c) x0[c] = 0.0;
            for (c = 0; c < 48; ++c) x1[c] = 0.0;
            x0[j] = 1.0;
            sc[0] = (fom_float)(r == 1);
            ++probes;
            for (i = 0; i < 16; ++i) got[i] = ref[i] = (fom_float)(i % 7 - 3);
            fom_fused_t1(x0, x1, sc[0], got);
            fom_ref_fused_t1(x0, x1, sc[0], ref);
            err = 0.0; scale = 1.0; bad = 0;
            for (i = 0; i < 16; ++i) {
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
        for (j = 0; j < 48; ++j) {
          for (r = 0; r <= 1; ++r) {
            for (c = 0; c < 160; ++c) x0[c] = 0.0;
            for (c = 0; c < 48; ++c) x1[c] = 0.0;
            x1[j] = 1.0;
            sc[0] = (fom_float)(r == 1);
            ++probes;
            for (i = 0; i < 16; ++i) got[i] = ref[i] = (fom_float)(i % 7 - 3);
            fom_fused_t1(x0, x1, sc[0], got);
            fom_ref_fused_t1(x0, x1, sc[0], ref);
            err = 0.0; scale = 1.0; bad = 0;
            for (i = 0; i < 16; ++i) {
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
            for (c = 0; c < 160; ++c) x0[c] = FOM_VERIFY_FUSED_T1_X0[r][c];
            for (c = 0; c < 48; ++c) x1[c] = FOM_VERIFY_FUSED_T1_X1[r][c];
            sc[0] = FOM_VERIFY_FUSED_T1_SC[r][0];
            ++probes;
            for (i = 0; i < 16; ++i) got[i] = ref[i] = (fom_float)(i % 7 - 3);
            fom_fused_t1(x0, x1, sc[0], got);
            fom_ref_fused_t1(x0, x1, sc[0], ref);
            err = 0.0; scale = 1.0; bad = 0;
            for (i = 0; i < 16; ++i) {
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
        /* fom_fused_su: direct check against its members, out += su_state @ state + sc_d1 * (su_state__d_d1 @ state) + sc_g2 * (su_state__d_g2 @ state) + sc_m1 * (su_state__d_m1 @ state) + sc_d2 * (su_t_t0__d_d2 @ t_t0) + sc_g1 * (su_t_t0__d_g1 @ t_t0) + sc_d2 * (su_t_t1__d_d2 @ t_t1) + sc_g1 * (su_t_t1__d_g1 @ t_t1) + sc_m2 * (su_t_t1__d_m2 @ t_t1). */
        static fom_float x0[160];
        static fom_float x1[48];
        static fom_float x2[16];
        static fom_float got[160], ref[160];
        fom_float sc[6];
        static const char *setting[] = {"all schedule values 0", "sc_d1 = 1", "sc_g2 = 1", "sc_m1 = 1", "sc_d2 = 1", "sc_g1 = 1", "sc_m2 = 1"};
        fom_float worst_op = 0.0, err, scale;
        int r, i, c, j, bad, probes = 0;
        (void)setting; (void)j; (void)sc;
        for (j = 0; j < 160; ++j) {
          for (r = 0; r <= 6; ++r) {
            for (c = 0; c < 160; ++c) x0[c] = 0.0;
            for (c = 0; c < 48; ++c) x1[c] = 0.0;
            for (c = 0; c < 16; ++c) x2[c] = 0.0;
            x0[j] = 1.0;
            sc[0] = (fom_float)(r == 1);
            sc[1] = (fom_float)(r == 2);
            sc[2] = (fom_float)(r == 3);
            sc[3] = (fom_float)(r == 4);
            sc[4] = (fom_float)(r == 5);
            sc[5] = (fom_float)(r == 6);
            ++probes;
            for (i = 0; i < 160; ++i) got[i] = ref[i] = (fom_float)(i % 7 - 3);
            fom_fused_su(x0, x1, x2, sc[0], sc[1], sc[2], sc[3], sc[4], sc[5], got);
            fom_ref_fused_su(x0, x1, x2, sc[0], sc[1], sc[2], sc[3], sc[4], sc[5], ref);
            err = 0.0; scale = 1.0; bad = 0;
            for (i = 0; i < 160; ++i) {
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
        for (j = 0; j < 48; ++j) {
          for (r = 0; r <= 6; ++r) {
            for (c = 0; c < 160; ++c) x0[c] = 0.0;
            for (c = 0; c < 48; ++c) x1[c] = 0.0;
            for (c = 0; c < 16; ++c) x2[c] = 0.0;
            x1[j] = 1.0;
            sc[0] = (fom_float)(r == 1);
            sc[1] = (fom_float)(r == 2);
            sc[2] = (fom_float)(r == 3);
            sc[3] = (fom_float)(r == 4);
            sc[4] = (fom_float)(r == 5);
            sc[5] = (fom_float)(r == 6);
            ++probes;
            for (i = 0; i < 160; ++i) got[i] = ref[i] = (fom_float)(i % 7 - 3);
            fom_fused_su(x0, x1, x2, sc[0], sc[1], sc[2], sc[3], sc[4], sc[5], got);
            fom_ref_fused_su(x0, x1, x2, sc[0], sc[1], sc[2], sc[3], sc[4], sc[5], ref);
            err = 0.0; scale = 1.0; bad = 0;
            for (i = 0; i < 160; ++i) {
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
        for (j = 0; j < 16; ++j) {
          for (r = 0; r <= 6; ++r) {
            for (c = 0; c < 160; ++c) x0[c] = 0.0;
            for (c = 0; c < 48; ++c) x1[c] = 0.0;
            for (c = 0; c < 16; ++c) x2[c] = 0.0;
            x2[j] = 1.0;
            sc[0] = (fom_float)(r == 1);
            sc[1] = (fom_float)(r == 2);
            sc[2] = (fom_float)(r == 3);
            sc[3] = (fom_float)(r == 4);
            sc[4] = (fom_float)(r == 5);
            sc[5] = (fom_float)(r == 6);
            ++probes;
            for (i = 0; i < 160; ++i) got[i] = ref[i] = (fom_float)(i % 7 - 3);
            fom_fused_su(x0, x1, x2, sc[0], sc[1], sc[2], sc[3], sc[4], sc[5], got);
            fom_ref_fused_su(x0, x1, x2, sc[0], sc[1], sc[2], sc[3], sc[4], sc[5], ref);
            err = 0.0; scale = 1.0; bad = 0;
            for (i = 0; i < 160; ++i) {
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
            for (c = 0; c < 160; ++c) x0[c] = FOM_VERIFY_FUSED_SU_X0[r][c];
            for (c = 0; c < 48; ++c) x1[c] = FOM_VERIFY_FUSED_SU_X1[r][c];
            for (c = 0; c < 16; ++c) x2[c] = FOM_VERIFY_FUSED_SU_X2[r][c];
            sc[0] = FOM_VERIFY_FUSED_SU_SC[r][0];
            sc[1] = FOM_VERIFY_FUSED_SU_SC[r][1];
            sc[2] = FOM_VERIFY_FUSED_SU_SC[r][2];
            sc[3] = FOM_VERIFY_FUSED_SU_SC[r][3];
            sc[4] = FOM_VERIFY_FUSED_SU_SC[r][4];
            sc[5] = FOM_VERIFY_FUSED_SU_SC[r][5];
            ++probes;
            for (i = 0; i < 160; ++i) got[i] = ref[i] = (fom_float)(i % 7 - 3);
            fom_fused_su(x0, x1, x2, sc[0], sc[1], sc[2], sc[3], sc[4], sc[5], got);
            fom_ref_fused_su(x0, x1, x2, sc[0], sc[1], sc[2], sc[3], sc[4], sc[5], ref);
            err = 0.0; scale = 1.0; bad = 0;
            for (i = 0; i < 160; ++i) {
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
        static fom_float x0[160];
        static fom_float bias[48], got[48], ref[48], pre[48];
        static fom_float act_lo[48], act_hi[48];
        FomAct_t0 hact;
        fom_float sc[2];
        static const char *setting[] = {"all schedule values 0", "sc_taux = 1", "sc_tth = 1", "activation off", "package activation", "random activation"};
        fom_float worst_op = 0.0, err, scale, m;
        int r, i, c, j, bad, touched, mode, probes = 0;
        (void)setting; (void)j; (void)sc; (void)m; (void)mode; (void)act_lo;
        (void)act_hi; (void)bias;
        for (i = 0; i < 48; ++i) pre[i] = 0.0;
        for (i = 0; i < 48; ++i) act_lo[i] = -FOM_VERIFY_HUGE;
        hact.lo = act_lo;
        for (i = 0; i < 48; ++i) bias[i] = FOM_VERIFY_STEP_T0_BIAS[0][i];
        for (j = 0; j < 160; ++j) {
          for (r = 0; r <= 2; ++r) {
            for (c = 0; c < 160; ++c) x0[c] = 0.0;
            x0[j] = 1.0;
            sc[0] = (fom_float)(r == 1);
            sc[1] = (fom_float)(r == 2);
            ++probes;
            for (i = 0; i < 48; ++i) {
                got[i] = (fom_float)(i % 7) - 3.5;
                ref[i] = -(fom_float)(i % 5) - 0.25;
            }
            fom_step_t0(x0, sc[0], sc[1], bias, &hact, got);
            fom_ref_step_t0(x0, sc[0], sc[1], bias, &hact, ref);
            err = 0.0; scale = 1.0; bad = 0; touched = -1;
            for (i = 0; i < 48; ++i) {
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
            for (c = 0; c < 160; ++c) x0[c] = FOM_VERIFY_STEP_T0_X0[r][c];
            sc[0] = FOM_VERIFY_STEP_T0_SC[r][0];
            sc[1] = FOM_VERIFY_STEP_T0_SC[r][1];
            for (i = 0; i < 48; ++i) bias[i] = FOM_VERIFY_STEP_T0_BIAS[r][i];
            for (i = 0; i < 48; ++i) act_lo[i] = -FOM_VERIFY_HUGE;
            hact.lo = act_lo;
            fom_ref_step_t0(x0, sc[0], sc[1], bias, &hact, pre);
            ++probes;
            for (i = 0; i < 48; ++i) {
                got[i] = (fom_float)(i % 7) - 3.5;
                ref[i] = -(fom_float)(i % 5) - 0.25;
            }
            fom_step_t0(x0, sc[0], sc[1], bias, &hact, got);
            fom_ref_step_t0(x0, sc[0], sc[1], bias, &hact, ref);
            err = 0.0; scale = 1.0; bad = 0; touched = -1;
            for (i = 0; i < 48; ++i) {
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
        for (i = 0; i < 48; ++i) pre[i] = 0.0;
        for (i = 0; i < 48; ++i) act_lo[i] = -FOM_VERIFY_HUGE;
        hact.lo = act_lo;
        for (r = 0; r < 2; ++r) {
            for (c = 0; c < 160; ++c) x0[c] = FOM_VERIFY_STEP_T0_X0[r][c];
            sc[0] = FOM_VERIFY_STEP_T0_SC[r][0];
            sc[1] = FOM_VERIFY_STEP_T0_SC[r][1];
            for (i = 0; i < 48; ++i) bias[i] = FOM_VERIFY_STEP_T0_BIAS[r][i];
            for (i = 0; i < 48; ++i) act_lo[i] = -FOM_VERIFY_HUGE;
            hact.lo = act_lo;
            fom_ref_step_t0(x0, sc[0], sc[1], bias, &hact, pre);
            ++probes;
            for (i = 0; i < 48; ++i) {
                got[i] = (fom_float)(i % 7) - 3.5;
                ref[i] = -(fom_float)(i % 5) - 0.25;
            }
            fom_step_t0(x0, sc[0], sc[1], bias, &FOM_ACT_t0, got);
            fom_ref_step_t0(x0, sc[0], sc[1], bias, &FOM_ACT_t0, ref);
            err = 0.0; scale = 1.0; bad = 0; touched = -1;
            for (i = 0; i < 48; ++i) {
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
        for (i = 0; i < 48; ++i) pre[i] = 0.0;
        for (i = 0; i < 48; ++i) act_lo[i] = -FOM_VERIFY_HUGE;
        hact.lo = act_lo;
        for (r = 0; r < 2; ++r) {
            for (c = 0; c < 160; ++c) x0[c] = FOM_VERIFY_STEP_T0_X0[r][c];
            sc[0] = FOM_VERIFY_STEP_T0_SC[r][0];
            sc[1] = FOM_VERIFY_STEP_T0_SC[r][1];
            for (i = 0; i < 48; ++i) bias[i] = FOM_VERIFY_STEP_T0_BIAS[r][i];
            for (i = 0; i < 48; ++i) act_lo[i] = -FOM_VERIFY_HUGE;
            hact.lo = act_lo;
            fom_ref_step_t0(x0, sc[0], sc[1], bias, &hact, pre);
            m = 0.0;
            for (i = 0; i < 48; ++i) m += ((pre[i] < 0) ? -pre[i] : pre[i]);
            m = (m > 0) ? m / 48 : 1.0;
            for (i = 0; i < 48; ++i) act_lo[i] = pre[i] + FOM_VERIFY_STEP_T0_R1[r][i] * m;
            hact.lo = act_lo;
            ++probes;
            for (i = 0; i < 48; ++i) {
                got[i] = (fom_float)(i % 7) - 3.5;
                ref[i] = -(fom_float)(i % 5) - 0.25;
            }
            fom_step_t0(x0, sc[0], sc[1], bias, &hact, got);
            fom_ref_step_t0(x0, sc[0], sc[1], bias, &hact, ref);
            err = 0.0; scale = 1.0; bad = 0; touched = -1;
            for (i = 0; i < 48; ++i) {
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
        for (i = 0; i < 48; ++i) pre[i] = 0.0;
        for (i = 0; i < 48; ++i) act_lo[i] = -FOM_VERIFY_HUGE;
        hact.lo = act_lo;
        printf("OPSCHECK name=step_t0 mode=basis probes=%d max_err=%.3e\n", probes, (double)worst_op);
    }
    {
        /* fom_step_t1: direct check against its members and the reference activation. */
        static fom_float x0[160];
        static fom_float x1[48];
        static fom_float bias[16], got[16], ref[16], pre[16];
        static fom_float act_lo[16], act_hi[16];
        FomAct_t1 hact;
        fom_float sc[1];
        static const char *setting[] = {"all schedule values 0", "sc_eta = 1", "activation off", "package activation", "random activation"};
        fom_float worst_op = 0.0, err, scale, m;
        int r, i, c, j, bad, touched, mode, probes = 0;
        (void)setting; (void)j; (void)sc; (void)m; (void)mode; (void)act_lo;
        (void)act_hi; (void)bias;
        for (i = 0; i < 16; ++i) pre[i] = 0.0;
        for (i = 0; i < 16; ++i) act_lo[i] = -FOM_VERIFY_HUGE;
        hact.lo = act_lo;
        for (i = 0; i < 16; ++i) act_hi[i] = FOM_VERIFY_HUGE;
        hact.hi = act_hi;
        for (i = 0; i < 16; ++i) bias[i] = FOM_VERIFY_STEP_T1_BIAS[0][i];
        for (j = 0; j < 160; ++j) {
          for (r = 0; r <= 1; ++r) {
            for (c = 0; c < 160; ++c) x0[c] = 0.0;
            for (c = 0; c < 48; ++c) x1[c] = 0.0;
            x0[j] = 1.0;
            sc[0] = (fom_float)(r == 1);
            ++probes;
            for (i = 0; i < 16; ++i) {
                got[i] = (fom_float)(i % 7) - 3.5;
                ref[i] = -(fom_float)(i % 5) - 0.25;
            }
            fom_step_t1(x0, x1, sc[0], bias, &hact, got);
            fom_ref_step_t1(x0, x1, sc[0], bias, &hact, ref);
            err = 0.0; scale = 1.0; bad = 0; touched = -1;
            for (i = 0; i < 16; ++i) {
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
        for (i = 0; i < 16; ++i) bias[i] = FOM_VERIFY_STEP_T1_BIAS[0][i];
        for (j = 0; j < 48; ++j) {
          for (r = 0; r <= 1; ++r) {
            for (c = 0; c < 160; ++c) x0[c] = 0.0;
            for (c = 0; c < 48; ++c) x1[c] = 0.0;
            x1[j] = 1.0;
            sc[0] = (fom_float)(r == 1);
            ++probes;
            for (i = 0; i < 16; ++i) {
                got[i] = (fom_float)(i % 7) - 3.5;
                ref[i] = -(fom_float)(i % 5) - 0.25;
            }
            fom_step_t1(x0, x1, sc[0], bias, &hact, got);
            fom_ref_step_t1(x0, x1, sc[0], bias, &hact, ref);
            err = 0.0; scale = 1.0; bad = 0; touched = -1;
            for (i = 0; i < 16; ++i) {
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
            for (c = 0; c < 160; ++c) x0[c] = FOM_VERIFY_STEP_T1_X0[r][c];
            for (c = 0; c < 48; ++c) x1[c] = FOM_VERIFY_STEP_T1_X1[r][c];
            sc[0] = FOM_VERIFY_STEP_T1_SC[r][0];
            for (i = 0; i < 16; ++i) bias[i] = FOM_VERIFY_STEP_T1_BIAS[r][i];
            for (i = 0; i < 16; ++i) act_lo[i] = -FOM_VERIFY_HUGE;
            hact.lo = act_lo;
            for (i = 0; i < 16; ++i) act_hi[i] = FOM_VERIFY_HUGE;
            hact.hi = act_hi;
            fom_ref_step_t1(x0, x1, sc[0], bias, &hact, pre);
            ++probes;
            for (i = 0; i < 16; ++i) {
                got[i] = (fom_float)(i % 7) - 3.5;
                ref[i] = -(fom_float)(i % 5) - 0.25;
            }
            fom_step_t1(x0, x1, sc[0], bias, &hact, got);
            fom_ref_step_t1(x0, x1, sc[0], bias, &hact, ref);
            err = 0.0; scale = 1.0; bad = 0; touched = -1;
            for (i = 0; i < 16; ++i) {
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
        for (i = 0; i < 16; ++i) pre[i] = 0.0;
        for (i = 0; i < 16; ++i) act_lo[i] = -FOM_VERIFY_HUGE;
        hact.lo = act_lo;
        for (i = 0; i < 16; ++i) act_hi[i] = FOM_VERIFY_HUGE;
        hact.hi = act_hi;
        for (r = 0; r < 2; ++r) {
            for (c = 0; c < 160; ++c) x0[c] = FOM_VERIFY_STEP_T1_X0[r][c];
            for (c = 0; c < 48; ++c) x1[c] = FOM_VERIFY_STEP_T1_X1[r][c];
            sc[0] = FOM_VERIFY_STEP_T1_SC[r][0];
            for (i = 0; i < 16; ++i) bias[i] = FOM_VERIFY_STEP_T1_BIAS[r][i];
            for (i = 0; i < 16; ++i) act_lo[i] = -FOM_VERIFY_HUGE;
            hact.lo = act_lo;
            for (i = 0; i < 16; ++i) act_hi[i] = FOM_VERIFY_HUGE;
            hact.hi = act_hi;
            fom_ref_step_t1(x0, x1, sc[0], bias, &hact, pre);
            ++probes;
            for (i = 0; i < 16; ++i) {
                got[i] = (fom_float)(i % 7) - 3.5;
                ref[i] = -(fom_float)(i % 5) - 0.25;
            }
            fom_step_t1(x0, x1, sc[0], bias, &FOM_ACT_t1, got);
            fom_ref_step_t1(x0, x1, sc[0], bias, &FOM_ACT_t1, ref);
            err = 0.0; scale = 1.0; bad = 0; touched = -1;
            for (i = 0; i < 16; ++i) {
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
        for (i = 0; i < 16; ++i) pre[i] = 0.0;
        for (i = 0; i < 16; ++i) act_lo[i] = -FOM_VERIFY_HUGE;
        hact.lo = act_lo;
        for (i = 0; i < 16; ++i) act_hi[i] = FOM_VERIFY_HUGE;
        hact.hi = act_hi;
        for (r = 0; r < 2; ++r) {
            for (c = 0; c < 160; ++c) x0[c] = FOM_VERIFY_STEP_T1_X0[r][c];
            for (c = 0; c < 48; ++c) x1[c] = FOM_VERIFY_STEP_T1_X1[r][c];
            sc[0] = FOM_VERIFY_STEP_T1_SC[r][0];
            for (i = 0; i < 16; ++i) bias[i] = FOM_VERIFY_STEP_T1_BIAS[r][i];
            for (i = 0; i < 16; ++i) act_lo[i] = -FOM_VERIFY_HUGE;
            hact.lo = act_lo;
            for (i = 0; i < 16; ++i) act_hi[i] = FOM_VERIFY_HUGE;
            hact.hi = act_hi;
            fom_ref_step_t1(x0, x1, sc[0], bias, &hact, pre);
            m = 0.0;
            for (i = 0; i < 16; ++i) m += ((pre[i] < 0) ? -pre[i] : pre[i]);
            m = (m > 0) ? m / 16 : 1.0;
            for (i = 0; i < 16; ++i) act_lo[i] = pre[i] + FOM_VERIFY_STEP_T1_R1[r][i] * m;
            hact.lo = act_lo;
            for (i = 0; i < 16; ++i) act_hi[i] = act_lo[i] + FOM_VERIFY_STEP_T1_R2[r][i] * m;
            hact.hi = act_hi;
            ++probes;
            for (i = 0; i < 16; ++i) {
                got[i] = (fom_float)(i % 7) - 3.5;
                ref[i] = -(fom_float)(i % 5) - 0.25;
            }
            fom_step_t1(x0, x1, sc[0], bias, &hact, got);
            fom_ref_step_t1(x0, x1, sc[0], bias, &hact, ref);
            err = 0.0; scale = 1.0; bad = 0; touched = -1;
            for (i = 0; i < 16; ++i) {
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
        for (i = 0; i < 16; ++i) pre[i] = 0.0;
        for (i = 0; i < 16; ++i) act_lo[i] = -FOM_VERIFY_HUGE;
        hact.lo = act_lo;
        for (i = 0; i < 16; ++i) act_hi[i] = FOM_VERIFY_HUGE;
        hact.hi = act_hi;
        printf("OPSCHECK name=step_t1 mode=basis probes=%d max_err=%.3e\n", probes, (double)worst_op);
    }
    {
        /* fom_step_su: direct check against its members. */
        static fom_float x0[160];
        static fom_float x1[48];
        static fom_float x2[16];
        static fom_float bias[160], got[160], ref[160], pre[160];
        static fom_float act_lo[160], act_hi[160];
        fom_float sc[6];
        static const char *setting[] = {"all schedule values 0", "sc_d1 = 1", "sc_g2 = 1", "sc_m1 = 1", "sc_d2 = 1", "sc_g1 = 1", "sc_m2 = 1", "random"};
        fom_float worst_op = 0.0, err, scale, m;
        int r, i, c, j, bad, touched, mode, probes = 0;
        (void)setting; (void)j; (void)sc; (void)m; (void)mode; (void)act_lo;
        (void)act_hi; (void)bias;
        for (i = 0; i < 160; ++i) pre[i] = 0.0;
        for (i = 0; i < 160; ++i) bias[i] = FOM_VERIFY_STEP_SU_BIAS[0][i];
        for (j = 0; j < 160; ++j) {
          for (r = 0; r <= 6; ++r) {
            for (c = 0; c < 160; ++c) x0[c] = 0.0;
            for (c = 0; c < 48; ++c) x1[c] = 0.0;
            for (c = 0; c < 16; ++c) x2[c] = 0.0;
            x0[j] = 1.0;
            sc[0] = (fom_float)(r == 1);
            sc[1] = (fom_float)(r == 2);
            sc[2] = (fom_float)(r == 3);
            sc[3] = (fom_float)(r == 4);
            sc[4] = (fom_float)(r == 5);
            sc[5] = (fom_float)(r == 6);
            ++probes;
            for (i = 0; i < 160; ++i) {
                got[i] = (fom_float)(i % 7) - 3.5;
                ref[i] = -(fom_float)(i % 5) - 0.25;
            }
            fom_step_su(x0, x1, x2, sc[0], sc[1], sc[2], sc[3], sc[4], sc[5], bias, got);
            fom_ref_step_su(x0, x1, x2, sc[0], sc[1], sc[2], sc[3], sc[4], sc[5], bias, ref);
            err = 0.0; scale = 1.0; bad = 0; touched = -1;
            for (i = 0; i < 160; ++i) {
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
        for (i = 0; i < 160; ++i) bias[i] = FOM_VERIFY_STEP_SU_BIAS[0][i];
        for (j = 0; j < 48; ++j) {
          for (r = 0; r <= 6; ++r) {
            for (c = 0; c < 160; ++c) x0[c] = 0.0;
            for (c = 0; c < 48; ++c) x1[c] = 0.0;
            for (c = 0; c < 16; ++c) x2[c] = 0.0;
            x1[j] = 1.0;
            sc[0] = (fom_float)(r == 1);
            sc[1] = (fom_float)(r == 2);
            sc[2] = (fom_float)(r == 3);
            sc[3] = (fom_float)(r == 4);
            sc[4] = (fom_float)(r == 5);
            sc[5] = (fom_float)(r == 6);
            ++probes;
            for (i = 0; i < 160; ++i) {
                got[i] = (fom_float)(i % 7) - 3.5;
                ref[i] = -(fom_float)(i % 5) - 0.25;
            }
            fom_step_su(x0, x1, x2, sc[0], sc[1], sc[2], sc[3], sc[4], sc[5], bias, got);
            fom_ref_step_su(x0, x1, x2, sc[0], sc[1], sc[2], sc[3], sc[4], sc[5], bias, ref);
            err = 0.0; scale = 1.0; bad = 0; touched = -1;
            for (i = 0; i < 160; ++i) {
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
        for (i = 0; i < 160; ++i) bias[i] = FOM_VERIFY_STEP_SU_BIAS[0][i];
        for (j = 0; j < 16; ++j) {
          for (r = 0; r <= 6; ++r) {
            for (c = 0; c < 160; ++c) x0[c] = 0.0;
            for (c = 0; c < 48; ++c) x1[c] = 0.0;
            for (c = 0; c < 16; ++c) x2[c] = 0.0;
            x2[j] = 1.0;
            sc[0] = (fom_float)(r == 1);
            sc[1] = (fom_float)(r == 2);
            sc[2] = (fom_float)(r == 3);
            sc[3] = (fom_float)(r == 4);
            sc[4] = (fom_float)(r == 5);
            sc[5] = (fom_float)(r == 6);
            ++probes;
            for (i = 0; i < 160; ++i) {
                got[i] = (fom_float)(i % 7) - 3.5;
                ref[i] = -(fom_float)(i % 5) - 0.25;
            }
            fom_step_su(x0, x1, x2, sc[0], sc[1], sc[2], sc[3], sc[4], sc[5], bias, got);
            fom_ref_step_su(x0, x1, x2, sc[0], sc[1], sc[2], sc[3], sc[4], sc[5], bias, ref);
            err = 0.0; scale = 1.0; bad = 0; touched = -1;
            for (i = 0; i < 160; ++i) {
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
            for (c = 0; c < 160; ++c) x0[c] = FOM_VERIFY_STEP_SU_X0[r][c];
            for (c = 0; c < 48; ++c) x1[c] = FOM_VERIFY_STEP_SU_X1[r][c];
            for (c = 0; c < 16; ++c) x2[c] = FOM_VERIFY_STEP_SU_X2[r][c];
            sc[0] = FOM_VERIFY_STEP_SU_SC[r][0];
            sc[1] = FOM_VERIFY_STEP_SU_SC[r][1];
            sc[2] = FOM_VERIFY_STEP_SU_SC[r][2];
            sc[3] = FOM_VERIFY_STEP_SU_SC[r][3];
            sc[4] = FOM_VERIFY_STEP_SU_SC[r][4];
            sc[5] = FOM_VERIFY_STEP_SU_SC[r][5];
            for (i = 0; i < 160; ++i) bias[i] = FOM_VERIFY_STEP_SU_BIAS[r][i];
            ++probes;
            for (i = 0; i < 160; ++i) {
                got[i] = (fom_float)(i % 7) - 3.5;
                ref[i] = -(fom_float)(i % 5) - 0.25;
            }
            fom_step_su(x0, x1, x2, sc[0], sc[1], sc[2], sc[3], sc[4], sc[5], bias, got);
            fom_ref_step_su(x0, x1, x2, sc[0], sc[1], sc[2], sc[3], sc[4], sc[5], bias, ref);
            err = 0.0; scale = 1.0; bad = 0; touched = -1;
            for (i = 0; i < 160; ++i) {
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
        static fom_float x0[160];
        static fom_float x1[48];
        static fom_float x2[16];
        static fom_float bias[160], got[160], ref[160], pre[160];
        static fom_float act_lo[160], act_hi[160];
        static const unsigned char sel[160] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        fom_float sc[6];
        static const char *setting[] = {"all schedule values 0", "sc_d1 = 1", "sc_g2 = 1", "sc_m1 = 1", "sc_d2 = 1", "sc_g1 = 1", "sc_m2 = 1", "random"};
        fom_float worst_op = 0.0, err, scale, m;
        int r, i, c, j, bad, touched, mode, probes = 0;
        (void)setting; (void)j; (void)sc; (void)m; (void)mode; (void)act_lo;
        (void)act_hi; (void)bias;
        for (i = 0; i < 160; ++i) pre[i] = 0.0;
        for (i = 0; i < 160; ++i) bias[i] = FOM_VERIFY_STEP_SU_BIAS[0][i];
        for (j = 0; j < 160; ++j) {
          for (r = 0; r <= 6; ++r) {
            for (c = 0; c < 160; ++c) x0[c] = 0.0;
            for (c = 0; c < 48; ++c) x1[c] = 0.0;
            for (c = 0; c < 16; ++c) x2[c] = 0.0;
            x0[j] = 1.0;
            sc[0] = (fom_float)(r == 1);
            sc[1] = (fom_float)(r == 2);
            sc[2] = (fom_float)(r == 3);
            sc[3] = (fom_float)(r == 4);
            sc[4] = (fom_float)(r == 5);
            sc[5] = (fom_float)(r == 6);
            ++probes;
            for (i = 0; i < 160; ++i) {
                got[i] = (fom_float)(i % 7) - 3.5;
                ref[i] = got[i];
            }
            fom_fused_su__rest(x0, x1, x2, sc[0], sc[1], sc[2], sc[3], sc[4], sc[5], got);
            fom_ref_fused_su(x0, x1, x2, sc[0], sc[1], sc[2], sc[3], sc[4], sc[5], ref);
            err = 0.0; scale = 1.0; bad = 0; touched = -1;
            for (i = 0; i < 160; ++i) {
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
        for (i = 0; i < 160; ++i) bias[i] = FOM_VERIFY_STEP_SU_BIAS[0][i];
        for (j = 0; j < 48; ++j) {
          for (r = 0; r <= 6; ++r) {
            for (c = 0; c < 160; ++c) x0[c] = 0.0;
            for (c = 0; c < 48; ++c) x1[c] = 0.0;
            for (c = 0; c < 16; ++c) x2[c] = 0.0;
            x1[j] = 1.0;
            sc[0] = (fom_float)(r == 1);
            sc[1] = (fom_float)(r == 2);
            sc[2] = (fom_float)(r == 3);
            sc[3] = (fom_float)(r == 4);
            sc[4] = (fom_float)(r == 5);
            sc[5] = (fom_float)(r == 6);
            ++probes;
            for (i = 0; i < 160; ++i) {
                got[i] = (fom_float)(i % 7) - 3.5;
                ref[i] = got[i];
            }
            fom_fused_su__rest(x0, x1, x2, sc[0], sc[1], sc[2], sc[3], sc[4], sc[5], got);
            fom_ref_fused_su(x0, x1, x2, sc[0], sc[1], sc[2], sc[3], sc[4], sc[5], ref);
            err = 0.0; scale = 1.0; bad = 0; touched = -1;
            for (i = 0; i < 160; ++i) {
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
        for (i = 0; i < 160; ++i) bias[i] = FOM_VERIFY_STEP_SU_BIAS[0][i];
        for (j = 0; j < 16; ++j) {
          for (r = 0; r <= 6; ++r) {
            for (c = 0; c < 160; ++c) x0[c] = 0.0;
            for (c = 0; c < 48; ++c) x1[c] = 0.0;
            for (c = 0; c < 16; ++c) x2[c] = 0.0;
            x2[j] = 1.0;
            sc[0] = (fom_float)(r == 1);
            sc[1] = (fom_float)(r == 2);
            sc[2] = (fom_float)(r == 3);
            sc[3] = (fom_float)(r == 4);
            sc[4] = (fom_float)(r == 5);
            sc[5] = (fom_float)(r == 6);
            ++probes;
            for (i = 0; i < 160; ++i) {
                got[i] = (fom_float)(i % 7) - 3.5;
                ref[i] = got[i];
            }
            fom_fused_su__rest(x0, x1, x2, sc[0], sc[1], sc[2], sc[3], sc[4], sc[5], got);
            fom_ref_fused_su(x0, x1, x2, sc[0], sc[1], sc[2], sc[3], sc[4], sc[5], ref);
            err = 0.0; scale = 1.0; bad = 0; touched = -1;
            for (i = 0; i < 160; ++i) {
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
            for (c = 0; c < 160; ++c) x0[c] = FOM_VERIFY_STEP_SU_X0[r][c];
            for (c = 0; c < 48; ++c) x1[c] = FOM_VERIFY_STEP_SU_X1[r][c];
            for (c = 0; c < 16; ++c) x2[c] = FOM_VERIFY_STEP_SU_X2[r][c];
            sc[0] = FOM_VERIFY_STEP_SU_SC[r][0];
            sc[1] = FOM_VERIFY_STEP_SU_SC[r][1];
            sc[2] = FOM_VERIFY_STEP_SU_SC[r][2];
            sc[3] = FOM_VERIFY_STEP_SU_SC[r][3];
            sc[4] = FOM_VERIFY_STEP_SU_SC[r][4];
            sc[5] = FOM_VERIFY_STEP_SU_SC[r][5];
            for (i = 0; i < 160; ++i) bias[i] = FOM_VERIFY_STEP_SU_BIAS[r][i];
            ++probes;
            for (i = 0; i < 160; ++i) {
                got[i] = (fom_float)(i % 7) - 3.5;
                ref[i] = got[i];
            }
            fom_fused_su__rest(x0, x1, x2, sc[0], sc[1], sc[2], sc[3], sc[4], sc[5], got);
            fom_ref_fused_su(x0, x1, x2, sc[0], sc[1], sc[2], sc[3], sc[4], sc[5], ref);
            err = 0.0; scale = 1.0; bad = 0; touched = -1;
            for (i = 0; i < 160; ++i) {
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
        static fom_float x0[160];
        static fom_float x1[48];
        static fom_float x2[16];
        static fom_float bias[160], got[160], ref[160], pre[160];
        static fom_float act_lo[160], act_hi[160];
        static const unsigned char sel[160] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        fom_float sc[6];
        static const char *setting[] = {"all schedule values 0", "sc_d1 = 1", "sc_g2 = 1", "sc_m1 = 1", "sc_d2 = 1", "sc_g1 = 1", "sc_m2 = 1", "random"};
        fom_float worst_op = 0.0, err, scale, m;
        int r, i, c, j, bad, touched, mode, probes = 0;
        (void)setting; (void)j; (void)sc; (void)m; (void)mode; (void)act_lo;
        (void)act_hi; (void)bias;
        for (i = 0; i < 160; ++i) pre[i] = 0.0;
        for (i = 0; i < 160; ++i) bias[i] = FOM_VERIFY_STEP_SU_BIAS[0][i];
        for (j = 0; j < 160; ++j) {
          for (r = 0; r <= 6; ++r) {
            for (c = 0; c < 160; ++c) x0[c] = 0.0;
            for (c = 0; c < 48; ++c) x1[c] = 0.0;
            for (c = 0; c < 16; ++c) x2[c] = 0.0;
            x0[j] = 1.0;
            sc[0] = (fom_float)(r == 1);
            sc[1] = (fom_float)(r == 2);
            sc[2] = (fom_float)(r == 3);
            sc[3] = (fom_float)(r == 4);
            sc[4] = (fom_float)(r == 5);
            sc[5] = (fom_float)(r == 6);
            ++probes;
            for (i = 0; i < 160; ++i) {
                got[i] = (fom_float)(i % 7) - 3.5;
                ref[i] = -(fom_float)(i % 5) - 0.25;
            }
            fom_step_su__rest(x0, x1, x2, sc[0], sc[1], sc[2], sc[3], sc[4], sc[5], bias, got);
            fom_ref_step_su(x0, x1, x2, sc[0], sc[1], sc[2], sc[3], sc[4], sc[5], bias, ref);
            err = 0.0; scale = 1.0; bad = 0; touched = -1;
            for (i = 0; i < 160; ++i) {
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
        for (i = 0; i < 160; ++i) bias[i] = FOM_VERIFY_STEP_SU_BIAS[0][i];
        for (j = 0; j < 48; ++j) {
          for (r = 0; r <= 6; ++r) {
            for (c = 0; c < 160; ++c) x0[c] = 0.0;
            for (c = 0; c < 48; ++c) x1[c] = 0.0;
            for (c = 0; c < 16; ++c) x2[c] = 0.0;
            x1[j] = 1.0;
            sc[0] = (fom_float)(r == 1);
            sc[1] = (fom_float)(r == 2);
            sc[2] = (fom_float)(r == 3);
            sc[3] = (fom_float)(r == 4);
            sc[4] = (fom_float)(r == 5);
            sc[5] = (fom_float)(r == 6);
            ++probes;
            for (i = 0; i < 160; ++i) {
                got[i] = (fom_float)(i % 7) - 3.5;
                ref[i] = -(fom_float)(i % 5) - 0.25;
            }
            fom_step_su__rest(x0, x1, x2, sc[0], sc[1], sc[2], sc[3], sc[4], sc[5], bias, got);
            fom_ref_step_su(x0, x1, x2, sc[0], sc[1], sc[2], sc[3], sc[4], sc[5], bias, ref);
            err = 0.0; scale = 1.0; bad = 0; touched = -1;
            for (i = 0; i < 160; ++i) {
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
        for (i = 0; i < 160; ++i) bias[i] = FOM_VERIFY_STEP_SU_BIAS[0][i];
        for (j = 0; j < 16; ++j) {
          for (r = 0; r <= 6; ++r) {
            for (c = 0; c < 160; ++c) x0[c] = 0.0;
            for (c = 0; c < 48; ++c) x1[c] = 0.0;
            for (c = 0; c < 16; ++c) x2[c] = 0.0;
            x2[j] = 1.0;
            sc[0] = (fom_float)(r == 1);
            sc[1] = (fom_float)(r == 2);
            sc[2] = (fom_float)(r == 3);
            sc[3] = (fom_float)(r == 4);
            sc[4] = (fom_float)(r == 5);
            sc[5] = (fom_float)(r == 6);
            ++probes;
            for (i = 0; i < 160; ++i) {
                got[i] = (fom_float)(i % 7) - 3.5;
                ref[i] = -(fom_float)(i % 5) - 0.25;
            }
            fom_step_su__rest(x0, x1, x2, sc[0], sc[1], sc[2], sc[3], sc[4], sc[5], bias, got);
            fom_ref_step_su(x0, x1, x2, sc[0], sc[1], sc[2], sc[3], sc[4], sc[5], bias, ref);
            err = 0.0; scale = 1.0; bad = 0; touched = -1;
            for (i = 0; i < 160; ++i) {
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
            for (c = 0; c < 160; ++c) x0[c] = FOM_VERIFY_STEP_SU_X0[r][c];
            for (c = 0; c < 48; ++c) x1[c] = FOM_VERIFY_STEP_SU_X1[r][c];
            for (c = 0; c < 16; ++c) x2[c] = FOM_VERIFY_STEP_SU_X2[r][c];
            sc[0] = FOM_VERIFY_STEP_SU_SC[r][0];
            sc[1] = FOM_VERIFY_STEP_SU_SC[r][1];
            sc[2] = FOM_VERIFY_STEP_SU_SC[r][2];
            sc[3] = FOM_VERIFY_STEP_SU_SC[r][3];
            sc[4] = FOM_VERIFY_STEP_SU_SC[r][4];
            sc[5] = FOM_VERIFY_STEP_SU_SC[r][5];
            for (i = 0; i < 160; ++i) bias[i] = FOM_VERIFY_STEP_SU_BIAS[r][i];
            ++probes;
            for (i = 0; i < 160; ++i) {
                got[i] = (fom_float)(i % 7) - 3.5;
                ref[i] = -(fom_float)(i % 5) - 0.25;
            }
            fom_step_su__rest(x0, x1, x2, sc[0], sc[1], sc[2], sc[3], sc[4], sc[5], bias, got);
            fom_ref_step_su(x0, x1, x2, sc[0], sc[1], sc[2], sc[3], sc[4], sc[5], bias, ref);
            err = 0.0; scale = 1.0; bad = 0; touched = -1;
            for (i = 0; i < 160; ++i) {
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
        static fom_float x[160];
        static fom_float bias_t0[48];
        static fom_float bias_t1[16];
        static fom_float bias_su[160];
        static fom_float g_t_t0[48], r_t_t0[48];
        static fom_float g_t_t1[16], r_t_t1[16];
        static fom_float g_new_state[160], r_new_state[160];
        static fom_float lo_t0[48], hi_t0[48];
        static fom_float lo_t1[16], hi_t1[16];
        FomAct_t0 hact_t0;
        FomAct_t1 hact_t1;
        const FomAct_t0 *pact_t0 = &FOM_ACT_t0;
        const FomAct_t1 *pact_t1 = &FOM_ACT_t1;
        fom_float sc[9];
        static const char *setting[] = {"all schedule values 0", "sc_taux = 1", "sc_tth = 1", "sc_eta = 1", "sc_d1 = 1", "sc_g2 = 1", "sc_m1 = 1", "sc_d2 = 1", "sc_g1 = 1", "sc_m2 = 1", "random schedule values", "activations off", "package activations", "random activations"};
        static const char *output[] = {"t_t0", "t_t1", "new_state"};
        fom_float worst_op = 0.0, err, scale, m;
        int r, i, j, bad, which, probes = 0;
        (void)setting; (void)output; (void)j; (void)sc; (void)m;
        (void)pact_t0; (void)lo_t0; (void)hi_t0; (void)pact_t1; (void)lo_t1; (void)hi_t1;
        for (i = 0; i < 48; ++i) lo_t0[i] = -FOM_VERIFY_HUGE;
        hact_t0.lo = lo_t0;
        for (i = 0; i < 16; ++i) lo_t1[i] = -FOM_VERIFY_HUGE;
        hact_t1.lo = lo_t1;
        for (i = 0; i < 16; ++i) hi_t1[i] = FOM_VERIFY_HUGE;
        hact_t1.hi = hi_t1;
        for (i = 0; i < 48; ++i) bias_t0[i] = 0.0;
        for (i = 0; i < 16; ++i) bias_t1[i] = 0.0;
        for (i = 0; i < 160; ++i) bias_su[i] = 0.0;
        for (j = 0; j < 160; ++j) {
          for (r = 0; r <= 10; ++r) {
            for (i = 0; i < 160; ++i) x[i] = 0.0;
            x[j] = 1.0;
            sc[0] = (r == 10) ? FOM_VERIFY_ITER_SC[2][0] : (fom_float)(r == 1);
            sc[1] = (r == 10) ? FOM_VERIFY_ITER_SC[2][1] : (fom_float)(r == 2);
            sc[2] = (r == 10) ? FOM_VERIFY_ITER_SC[2][2] : (fom_float)(r == 3);
            sc[3] = (r == 10) ? FOM_VERIFY_ITER_SC[2][3] : (fom_float)(r == 4);
            sc[4] = (r == 10) ? FOM_VERIFY_ITER_SC[2][4] : (fom_float)(r == 5);
            sc[5] = (r == 10) ? FOM_VERIFY_ITER_SC[2][5] : (fom_float)(r == 6);
            sc[6] = (r == 10) ? FOM_VERIFY_ITER_SC[2][6] : (fom_float)(r == 7);
            sc[7] = (r == 10) ? FOM_VERIFY_ITER_SC[2][7] : (fom_float)(r == 8);
            sc[8] = (r == 10) ? FOM_VERIFY_ITER_SC[2][8] : (fom_float)(r == 9);
            ++probes;
            for (i = 0; i < 48; ++i) { g_t_t0[i] = (fom_float)(i % 7) - 3.5; r_t_t0[i] = -(fom_float)(i % 5) - 0.25; }
            for (i = 0; i < 16; ++i) { g_t_t1[i] = (fom_float)(i % 7) - 3.5; r_t_t1[i] = -(fom_float)(i % 5) - 0.25; }
            for (i = 0; i < 160; ++i) { g_new_state[i] = (fom_float)(i % 7) - 3.5; r_new_state[i] = -(fom_float)(i % 5) - 0.25; }
            fom_iterate(x, sc[0], sc[1], sc[2], sc[3], sc[4], sc[5], sc[6], sc[7], sc[8], bias_t0, bias_t1, bias_su, &hact_t0, &hact_t1, g_t_t0, g_t_t1, g_new_state);
            fom_ref_iterate(x, sc[0], sc[1], sc[2], sc[3], sc[4], sc[5], sc[6], sc[7], sc[8], bias_t0, bias_t1, bias_su, &hact_t0, &hact_t1, r_t_t0, r_t_t1, r_new_state);
            err = 0.0; scale = 1.0; bad = 0; which = 0;
            for (i = 0; i < 48; ++i) {
                fom_float d = g_t_t0[i] - r_t_t0[i], a = r_t_t0[i];
                if (d < 0) d = -d;
                if (a < 0) a = -a;
                if (!(d <= err)) { err = d; bad = i; which = 0; }
                if (1.0 + a > scale) scale = 1.0 + a;
            }
            for (i = 0; i < 16; ++i) {
                fom_float d = g_t_t1[i] - r_t_t1[i], a = r_t_t1[i];
                if (d < 0) d = -d;
                if (a < 0) a = -a;
                if (!(d <= err)) { err = d; bad = i; which = 1; }
                if (1.0 + a > scale) scale = 1.0 + a;
            }
            for (i = 0; i < 160; ++i) {
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
            for (i = 0; i < 160; ++i) x[i] = FOM_VERIFY_ITER_X[r][i];
            sc[0] = FOM_VERIFY_ITER_SC[r][0];
            sc[1] = FOM_VERIFY_ITER_SC[r][1];
            sc[2] = FOM_VERIFY_ITER_SC[r][2];
            sc[3] = FOM_VERIFY_ITER_SC[r][3];
            sc[4] = FOM_VERIFY_ITER_SC[r][4];
            sc[5] = FOM_VERIFY_ITER_SC[r][5];
            sc[6] = FOM_VERIFY_ITER_SC[r][6];
            sc[7] = FOM_VERIFY_ITER_SC[r][7];
            sc[8] = FOM_VERIFY_ITER_SC[r][8];
            for (i = 0; i < 48; ++i) bias_t0[i] = FOM_VERIFY_ITER_T0_BIAS[r][i];
            for (i = 0; i < 16; ++i) bias_t1[i] = FOM_VERIFY_ITER_T1_BIAS[r][i];
            for (i = 0; i < 160; ++i) bias_su[i] = FOM_VERIFY_ITER_SU_BIAS[r][i];
            ++probes;
            for (i = 0; i < 48; ++i) { g_t_t0[i] = (fom_float)(i % 7) - 3.5; r_t_t0[i] = -(fom_float)(i % 5) - 0.25; }
            for (i = 0; i < 16; ++i) { g_t_t1[i] = (fom_float)(i % 7) - 3.5; r_t_t1[i] = -(fom_float)(i % 5) - 0.25; }
            for (i = 0; i < 160; ++i) { g_new_state[i] = (fom_float)(i % 7) - 3.5; r_new_state[i] = -(fom_float)(i % 5) - 0.25; }
            fom_iterate(x, sc[0], sc[1], sc[2], sc[3], sc[4], sc[5], sc[6], sc[7], sc[8], bias_t0, bias_t1, bias_su, &hact_t0, &hact_t1, g_t_t0, g_t_t1, g_new_state);
            fom_ref_iterate(x, sc[0], sc[1], sc[2], sc[3], sc[4], sc[5], sc[6], sc[7], sc[8], bias_t0, bias_t1, bias_su, &hact_t0, &hact_t1, r_t_t0, r_t_t1, r_new_state);
            err = 0.0; scale = 1.0; bad = 0; which = 0;
            for (i = 0; i < 48; ++i) {
                fom_float d = g_t_t0[i] - r_t_t0[i], a = r_t_t0[i];
                if (d < 0) d = -d;
                if (a < 0) a = -a;
                if (!(d <= err)) { err = d; bad = i; which = 0; }
                if (1.0 + a > scale) scale = 1.0 + a;
            }
            for (i = 0; i < 16; ++i) {
                fom_float d = g_t_t1[i] - r_t_t1[i], a = r_t_t1[i];
                if (d < 0) d = -d;
                if (a < 0) a = -a;
                if (!(d <= err)) { err = d; bad = i; which = 1; }
                if (1.0 + a > scale) scale = 1.0 + a;
            }
            for (i = 0; i < 160; ++i) {
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
            for (i = 0; i < 160; ++i) x[i] = FOM_VERIFY_ITER_X[r][i];
            sc[0] = FOM_VERIFY_ITER_SC[r][0];
            sc[1] = FOM_VERIFY_ITER_SC[r][1];
            sc[2] = FOM_VERIFY_ITER_SC[r][2];
            sc[3] = FOM_VERIFY_ITER_SC[r][3];
            sc[4] = FOM_VERIFY_ITER_SC[r][4];
            sc[5] = FOM_VERIFY_ITER_SC[r][5];
            sc[6] = FOM_VERIFY_ITER_SC[r][6];
            sc[7] = FOM_VERIFY_ITER_SC[r][7];
            sc[8] = FOM_VERIFY_ITER_SC[r][8];
            for (i = 0; i < 48; ++i) bias_t0[i] = FOM_VERIFY_ITER_T0_BIAS[r][i];
            for (i = 0; i < 16; ++i) bias_t1[i] = FOM_VERIFY_ITER_T1_BIAS[r][i];
            for (i = 0; i < 160; ++i) bias_su[i] = FOM_VERIFY_ITER_SU_BIAS[r][i];
            ++probes;
            for (i = 0; i < 48; ++i) { g_t_t0[i] = (fom_float)(i % 7) - 3.5; r_t_t0[i] = -(fom_float)(i % 5) - 0.25; }
            for (i = 0; i < 16; ++i) { g_t_t1[i] = (fom_float)(i % 7) - 3.5; r_t_t1[i] = -(fom_float)(i % 5) - 0.25; }
            for (i = 0; i < 160; ++i) { g_new_state[i] = (fom_float)(i % 7) - 3.5; r_new_state[i] = -(fom_float)(i % 5) - 0.25; }
            fom_iterate(x, sc[0], sc[1], sc[2], sc[3], sc[4], sc[5], sc[6], sc[7], sc[8], bias_t0, bias_t1, bias_su, pact_t0, pact_t1, g_t_t0, g_t_t1, g_new_state);
            fom_ref_iterate(x, sc[0], sc[1], sc[2], sc[3], sc[4], sc[5], sc[6], sc[7], sc[8], bias_t0, bias_t1, bias_su, pact_t0, pact_t1, r_t_t0, r_t_t1, r_new_state);
            err = 0.0; scale = 1.0; bad = 0; which = 0;
            for (i = 0; i < 48; ++i) {
                fom_float d = g_t_t0[i] - r_t_t0[i], a = r_t_t0[i];
                if (d < 0) d = -d;
                if (a < 0) a = -a;
                if (!(d <= err)) { err = d; bad = i; which = 0; }
                if (1.0 + a > scale) scale = 1.0 + a;
            }
            for (i = 0; i < 16; ++i) {
                fom_float d = g_t_t1[i] - r_t_t1[i], a = r_t_t1[i];
                if (d < 0) d = -d;
                if (a < 0) a = -a;
                if (!(d <= err)) { err = d; bad = i; which = 1; }
                if (1.0 + a > scale) scale = 1.0 + a;
            }
            for (i = 0; i < 160; ++i) {
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
            for (i = 0; i < 160; ++i) x[i] = FOM_VERIFY_ITER_X[r][i];
            sc[0] = FOM_VERIFY_ITER_SC[r][0];
            sc[1] = FOM_VERIFY_ITER_SC[r][1];
            sc[2] = FOM_VERIFY_ITER_SC[r][2];
            sc[3] = FOM_VERIFY_ITER_SC[r][3];
            sc[4] = FOM_VERIFY_ITER_SC[r][4];
            sc[5] = FOM_VERIFY_ITER_SC[r][5];
            sc[6] = FOM_VERIFY_ITER_SC[r][6];
            sc[7] = FOM_VERIFY_ITER_SC[r][7];
            sc[8] = FOM_VERIFY_ITER_SC[r][8];
            for (i = 0; i < 48; ++i) bias_t0[i] = FOM_VERIFY_ITER_T0_BIAS[r][i];
            for (i = 0; i < 16; ++i) bias_t1[i] = FOM_VERIFY_ITER_T1_BIAS[r][i];
            for (i = 0; i < 160; ++i) bias_su[i] = FOM_VERIFY_ITER_SU_BIAS[r][i];
            fom_ref_iterate(x, sc[0], sc[1], sc[2], sc[3], sc[4], sc[5], sc[6], sc[7], sc[8], bias_t0, bias_t1, bias_su, &hact_t0, &hact_t1, r_t_t0, r_t_t1, r_new_state);
            m = 0.0;
            for (i = 0; i < 48; ++i) m += ((r_t_t0[i] < 0) ? -r_t_t0[i] : r_t_t0[i]);
            m = (m > 0) ? m / 48 : 1.0;
            for (i = 0; i < 48; ++i) lo_t0[i] = r_t_t0[i] + FOM_VERIFY_ITER_T0_R1[r][i] * m;
            hact_t0.lo = lo_t0;
            m = 0.0;
            for (i = 0; i < 16; ++i) m += ((r_t_t1[i] < 0) ? -r_t_t1[i] : r_t_t1[i]);
            m = (m > 0) ? m / 16 : 1.0;
            for (i = 0; i < 16; ++i) lo_t1[i] = r_t_t1[i] + FOM_VERIFY_ITER_T1_R1[r][i] * m;
            hact_t1.lo = lo_t1;
            for (i = 0; i < 16; ++i) hi_t1[i] = lo_t1[i] + FOM_VERIFY_ITER_T1_R2[r][i] * m;
            hact_t1.hi = hi_t1;
            ++probes;
            for (i = 0; i < 48; ++i) { g_t_t0[i] = (fom_float)(i % 7) - 3.5; r_t_t0[i] = -(fom_float)(i % 5) - 0.25; }
            for (i = 0; i < 16; ++i) { g_t_t1[i] = (fom_float)(i % 7) - 3.5; r_t_t1[i] = -(fom_float)(i % 5) - 0.25; }
            for (i = 0; i < 160; ++i) { g_new_state[i] = (fom_float)(i % 7) - 3.5; r_new_state[i] = -(fom_float)(i % 5) - 0.25; }
            fom_iterate(x, sc[0], sc[1], sc[2], sc[3], sc[4], sc[5], sc[6], sc[7], sc[8], bias_t0, bias_t1, bias_su, &hact_t0, &hact_t1, g_t_t0, g_t_t1, g_new_state);
            fom_ref_iterate(x, sc[0], sc[1], sc[2], sc[3], sc[4], sc[5], sc[6], sc[7], sc[8], bias_t0, bias_t1, bias_su, &hact_t0, &hact_t1, r_t_t0, r_t_t1, r_new_state);
            err = 0.0; scale = 1.0; bad = 0; which = 0;
            for (i = 0; i < 48; ++i) {
                fom_float d = g_t_t0[i] - r_t_t0[i], a = r_t_t0[i];
                if (d < 0) d = -d;
                if (a < 0) a = -a;
                if (!(d <= err)) { err = d; bad = i; which = 0; }
                if (1.0 + a > scale) scale = 1.0 + a;
            }
            for (i = 0; i < 16; ++i) {
                fom_float d = g_t_t1[i] - r_t_t1[i], a = r_t_t1[i];
                if (d < 0) d = -d;
                if (a < 0) a = -a;
                if (!(d <= err)) { err = d; bad = i; which = 1; }
                if (1.0 + a > scale) scale = 1.0 + a;
            }
            for (i = 0; i < 160; ++i) {
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
            for (i = 0; i < 48; ++i) lo_t0[i] = -FOM_VERIFY_HUGE;
            hact_t0.lo = lo_t0;
            for (i = 0; i < 16; ++i) lo_t1[i] = -FOM_VERIFY_HUGE;
            hact_t1.lo = lo_t1;
            for (i = 0; i < 16; ++i) hi_t1[i] = FOM_VERIFY_HUGE;
            hact_t1.hi = hi_t1;
        }
        printf("OPSCHECK name=iterate mode=basis probes=%d max_err=%.3e\n", probes, (double)worst_op);
    }

    fom_set_default_settings(&settings);
    fom_setup(&work, &settings);

    for (s = 0; s < FOM_NUM_SAMPLES; ++s) {
        for (j = 0; j < FOM_PARAM_P0_DIM; ++j)
            params.p0[j] = FOM_VERIFY_P0[s][j];
        for (j = 0; j < FOM_PARAM_P1_DIM; ++j)
            params.p1[j] = FOM_VERIFY_P1[s][j];
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
