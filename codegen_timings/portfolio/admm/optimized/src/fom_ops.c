#include "fom_ops.h"
#include "fom_data.h"

static const fom_int FOM_OP_T0_STATE_ROWPTR[11] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
static const fom_int FOM_OP_T0_STATE_COLIDX[10] = {21, 22, 23, 24, 25, 26, 27, 28, 29, 30};
static const fom_float FOM_OP_T0_STATE_VALS[10] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
void fom_apply_t0_state(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 10; ++i) {
        fom_float s = 0.0;
        for (p = FOM_OP_T0_STATE_ROWPTR[i]; p < FOM_OP_T0_STATE_ROWPTR[i + 1]; ++p) s += FOM_OP_T0_STATE_VALS[p] * x[FOM_OP_T0_STATE_COLIDX[p]];
        out[i] += s;
    }
}

static const fom_int FOM_OP_T1_STATE_ROWPTR[11] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
static const fom_int FOM_OP_T1_STATE_COLIDX[10] = {31, 32, 33, 34, 35, 36, 37, 38, 39, 40};
static const fom_float FOM_OP_T1_STATE_VALS[10] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
void fom_apply_t1_state(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 10; ++i) {
        fom_float s = 0.0;
        for (p = FOM_OP_T1_STATE_ROWPTR[i]; p < FOM_OP_T1_STATE_ROWPTR[i + 1]; ++p) s += FOM_OP_T1_STATE_VALS[p] * x[FOM_OP_T1_STATE_COLIDX[p]];
        out[i] += s;
    }
}

static const fom_int FOM_OP_T1_P_P0_ROWPTR[11] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
static const fom_int FOM_OP_T1_P_P0_COLIDX[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
static const fom_float FOM_OP_T1_P_P0_VALS[10] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
void fom_apply_t1_p_p0(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 10; ++i) {
        fom_float s = 0.0;
        for (p = FOM_OP_T1_P_P0_ROWPTR[i]; p < FOM_OP_T1_P_P0_ROWPTR[i + 1]; ++p) s += FOM_OP_T1_P_P0_VALS[p] * x[FOM_OP_T1_P_P0_COLIDX[p]];
        out[i] += s;
    }
}

static const fom_int FOM_OP_T2_STATE_ROWPTR[11] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
static const fom_int FOM_OP_T2_STATE_COLIDX[10] = {41, 42, 43, 44, 45, 46, 47, 48, 49, 50};
static const fom_float FOM_OP_T2_STATE_VALS[10] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
void fom_apply_t2_state(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 10; ++i) {
        fom_float s = 0.0;
        for (p = FOM_OP_T2_STATE_ROWPTR[i]; p < FOM_OP_T2_STATE_ROWPTR[i + 1]; ++p) s += FOM_OP_T2_STATE_VALS[p] * x[FOM_OP_T2_STATE_COLIDX[p]];
        out[i] += s;
    }
}

static const fom_int FOM_OP_T2_P_P0_ROWPTR[11] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
static const fom_int FOM_OP_T2_P_P0_COLIDX[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
static const fom_float FOM_OP_T2_P_P0_VALS[10] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
void fom_apply_t2_p_p0(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 10; ++i) {
        fom_float s = 0.0;
        for (p = FOM_OP_T2_P_P0_ROWPTR[i]; p < FOM_OP_T2_P_P0_ROWPTR[i + 1]; ++p) s += FOM_OP_T2_P_P0_VALS[p] * x[FOM_OP_T2_P_P0_COLIDX[p]];
        out[i] += s;
    }
}

static const fom_int FOM_OP_T3_STATE_ROWPTR[21] = {0, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 53, 56, 59, 62, 65, 68, 71, 74, 77, 80};
static const fom_int FOM_OP_T3_STATE_COLIDX[80] = {0, 20, 21, 31, 41, 1, 20, 22, 32, 42, 2, 20, 23, 33, 43, 3, 20, 24, 34, 44, 4, 20, 25, 35, 45, 5, 20, 26, 36, 46, 6, 20, 27, 37, 47, 7, 20, 28, 38, 48, 8, 20, 29, 39, 49, 9, 20, 30, 40, 50, 10, 31, 41, 11, 32, 42, 12, 33, 43, 13, 34, 44, 14, 35, 45, 15, 36, 46, 16, 37, 47, 17, 38, 48, 18, 39, 49, 19, 40, 50};
static const fom_float FOM_OP_T3_STATE_VALS[80] = {0.0001, -1, -1, -1, -1, 0.0001, -1, -1, -1, -1, 0.0001, -1, -1, -1, -1, 0.0001, -1, -1, -1, -1, 0.0001, -1, -1, -1, -1, 0.0001, -1, -1, -1, -1, 0.0001, -1, -1, -1, -1, 0.0001, -1, -1, -1, -1, 0.0001, -1, -1, -1, -1, 0.0001, -1, -1, -1, -1, 0.0001, 1, -1, 0.0001, 1, -1, 0.0001, 1, -1, 0.0001, 1, -1, 0.0001, 1, -1, 0.0001, 1, -1, 0.0001, 1, -1, 0.0001, 1, -1, 0.0001, 1, -1, 0.0001, 1, -1};
void fom_apply_t3_state(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_OP_T3_STATE_ROWPTR[i]; p < FOM_OP_T3_STATE_ROWPTR[i + 1]; ++p) s += FOM_OP_T3_STATE_VALS[p] * x[FOM_OP_T3_STATE_COLIDX[p]];
        out[i] += s;
    }
}

static const fom_int FOM_OP_T3_P_P0_ROWPTR[21] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10};
static const fom_int FOM_OP_T3_P_P0_COLIDX[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
static const fom_float FOM_OP_T3_P_P0_VALS[10] = {4, 4, 4, 4, 4, 4, 4, 4, 4, 4};
void fom_apply_t3_p_p0(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_OP_T3_P_P0_ROWPTR[i]; p < FOM_OP_T3_P_P0_ROWPTR[i + 1]; ++p) s += FOM_OP_T3_P_P0_VALS[p] * x[FOM_OP_T3_P_P0_COLIDX[p]];
        out[i] += s;
    }
}

static const fom_int FOM_OP_T3_T_T0_ROWPTR[21] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10};
static const fom_int FOM_OP_T3_T_T0_COLIDX[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
static const fom_float FOM_OP_T3_T_T0_VALS[10] = {2, 2, 2, 2, 2, 2, 2, 2, 2, 2};
void fom_apply_t3_t_t0(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_OP_T3_T_T0_ROWPTR[i]; p < FOM_OP_T3_T_T0_ROWPTR[i + 1]; ++p) s += FOM_OP_T3_T_T0_VALS[p] * x[FOM_OP_T3_T_T0_COLIDX[p]];
        out[i] += s;
    }
}

static const fom_int FOM_OP_T3_T_T1_ROWPTR[21] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20};
static const fom_int FOM_OP_T3_T_T1_COLIDX[20] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
static const fom_float FOM_OP_T3_T_T1_VALS[20] = {-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2};
void fom_apply_t3_t_t1(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_OP_T3_T_T1_ROWPTR[i]; p < FOM_OP_T3_T_T1_ROWPTR[i + 1]; ++p) s += FOM_OP_T3_T_T1_VALS[p] * x[FOM_OP_T3_T_T1_COLIDX[p]];
        out[i] += s;
    }
}

static const fom_int FOM_OP_T3_T_T2_ROWPTR[21] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20};
static const fom_int FOM_OP_T3_T_T2_COLIDX[20] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
static const fom_float FOM_OP_T3_T_T2_VALS[20] = {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2};
void fom_apply_t3_t_t2(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_OP_T3_T_T2_ROWPTR[i]; p < FOM_OP_T3_T_T2_ROWPTR[i + 1]; ++p) s += FOM_OP_T3_T_T2_VALS[p] * x[FOM_OP_T3_T_T2_COLIDX[p]];
        out[i] += s;
    }
}

static const fom_int FOM_OP_SU_STATE_ROWPTR[52] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31};
static const fom_int FOM_OP_SU_STATE_COLIDX[31] = {20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50};
static const fom_float FOM_OP_SU_STATE_VALS[31] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
void fom_apply_su_state(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 51; ++i) {
        fom_float s = 0.0;
        for (p = FOM_OP_SU_STATE_ROWPTR[i]; p < FOM_OP_SU_STATE_ROWPTR[i + 1]; ++p) s += FOM_OP_SU_STATE_VALS[p] * x[FOM_OP_SU_STATE_COLIDX[p]];
        out[i] += s;
    }
}

static const fom_int FOM_OP_SU_T_T0_ROWPTR[52] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10};
static const fom_int FOM_OP_SU_T_T0_COLIDX[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
static const fom_float FOM_OP_SU_T_T0_VALS[10] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
void fom_apply_su_t_t0(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 51; ++i) {
        fom_float s = 0.0;
        for (p = FOM_OP_SU_T_T0_ROWPTR[i]; p < FOM_OP_SU_T_T0_ROWPTR[i + 1]; ++p) s += FOM_OP_SU_T_T0_VALS[p] * x[FOM_OP_SU_T_T0_COLIDX[p]];
        out[i] += s;
    }
}

static const fom_int FOM_OP_SU_T_T1_ROWPTR[52] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10};
static const fom_int FOM_OP_SU_T_T1_COLIDX[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
static const fom_float FOM_OP_SU_T_T1_VALS[10] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
void fom_apply_su_t_t1(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 51; ++i) {
        fom_float s = 0.0;
        for (p = FOM_OP_SU_T_T1_ROWPTR[i]; p < FOM_OP_SU_T_T1_ROWPTR[i + 1]; ++p) s += FOM_OP_SU_T_T1_VALS[p] * x[FOM_OP_SU_T_T1_COLIDX[p]];
        out[i] += s;
    }
}

static const fom_int FOM_OP_SU_T_T2_ROWPTR[52] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
static const fom_int FOM_OP_SU_T_T2_COLIDX[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
static const fom_float FOM_OP_SU_T_T2_VALS[10] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
void fom_apply_su_t_t2(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 51; ++i) {
        fom_float s = 0.0;
        for (p = FOM_OP_SU_T_T2_ROWPTR[i]; p < FOM_OP_SU_T_T2_ROWPTR[i + 1]; ++p) s += FOM_OP_SU_T_T2_VALS[p] * x[FOM_OP_SU_T_T2_COLIDX[p]];
        out[i] += s;
    }
}

static const fom_int FOM_OP_SU_T_T3_ROWPTR[52] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 42, 44, 46, 48, 50, 52, 54, 56, 58, 60, 62, 64, 66, 68, 70, 72, 74, 76, 78, 80};
static const fom_int FOM_OP_SU_T_T3_COLIDX[80] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 10, 1, 11, 2, 12, 3, 13, 4, 14, 5, 15, 6, 16, 7, 17, 8, 18, 9, 19, 0, 10, 1, 11, 2, 12, 3, 13, 4, 14, 5, 15, 6, 16, 7, 17, 8, 18, 9, 19};
static const fom_float FOM_OP_SU_T_T3_VALS[80] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
void fom_apply_su_t_t3(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 51; ++i) {
        fom_float s = 0.0;
        for (p = FOM_OP_SU_T_T3_ROWPTR[i]; p < FOM_OP_SU_T_T3_ROWPTR[i + 1]; ++p) s += FOM_OP_SU_T_T3_VALS[p] * x[FOM_OP_SU_T_T3_COLIDX[p]];
        out[i] += s;
    }
}

static const fom_int FOM_OP_SU_P_P0_ROWPTR[52] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20};
static const fom_int FOM_OP_SU_P_P0_COLIDX[20] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
static const fom_float FOM_OP_SU_P_P0_VALS[20] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
void fom_apply_su_p_p0(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 51; ++i) {
        fom_float s = 0.0;
        for (p = FOM_OP_SU_P_P0_ROWPTR[i]; p < FOM_OP_SU_P_P0_ROWPTR[i + 1]; ++p) s += FOM_OP_SU_P_P0_VALS[p] * x[FOM_OP_SU_P_P0_COLIDX[p]];
        out[i] += s;
    }
}
