#include "fom_ops.h"
#include "fom_data.h"

void fom_apply_t0_state(const fom_float *x, fom_float *out) {
    fom_int i, j;
    for (i = 0; i < 15; ++i) {
        fom_float s = 0.0;
        for (j = 0; j < 30; ++j) s += FOM_MAT_t0_state[i * 30 + j] * x[j];
        out[i] += s;
    }
}

void fom_apply_t0_state__d_gamma(const fom_float *x, fom_float *out) {
    fom_int i, j;
    for (i = 0; i < 15; ++i) {
        fom_float s = 0.0;
        for (j = 0; j < 30; ++j) s += FOM_MAT_t0_state__d_gamma[i * 30 + j] * x[j];
        out[i] += s;
    }
}

void fom_apply_t0_p_p0(const fom_float *x, fom_float *out) {
    fom_int i, j;
    for (i = 0; i < 15; ++i) {
        fom_float s = 0.0;
        for (j = 0; j < 10; ++j) s += FOM_MAT_t0_p_p0[i * 10 + j] * x[j];
        out[i] += s;
    }
}

static const fom_int FOM_OP_SU_STATE_ROWPTR[31] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15};
static const fom_int FOM_OP_SU_STATE_COLIDX[15] = {15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29};
static const fom_float FOM_OP_SU_STATE_VALS[15] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
void fom_apply_su_state(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_OP_SU_STATE_ROWPTR[i]; p < FOM_OP_SU_STATE_ROWPTR[i + 1]; ++p) s += FOM_OP_SU_STATE_VALS[p] * x[FOM_OP_SU_STATE_COLIDX[p]];
        out[i] += s;
    }
}

static const fom_int FOM_OP_SU_T_T0_ROWPTR[31] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
static const fom_int FOM_OP_SU_T_T0_COLIDX[15] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};
static const fom_float FOM_OP_SU_T_T0_VALS[15] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
void fom_apply_su_t_t0(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_OP_SU_T_T0_ROWPTR[i]; p < FOM_OP_SU_T_T0_ROWPTR[i + 1]; ++p) s += FOM_OP_SU_T_T0_VALS[p] * x[FOM_OP_SU_T_T0_COLIDX[p]];
        out[i] += s;
    }
}
