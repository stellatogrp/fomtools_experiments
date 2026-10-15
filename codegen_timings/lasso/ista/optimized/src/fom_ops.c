#include "fom_ops.h"
#include "fom_data.h"

void fom_apply_t0_state(const fom_float *x, fom_float *out) {
    fom_int i, j;
    for (i = 0; i < 15; ++i) {
        fom_float s = 0.0;
        for (j = 0; j < 15; ++j) s += FOM_MAT_t0_state[i * 15 + j] * x[j];
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

static const fom_int FOM_OP_SU_T_T0_ROWPTR[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
static const fom_int FOM_OP_SU_T_T0_COLIDX[15] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};
static const fom_float FOM_OP_SU_T_T0_VALS[15] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
void fom_apply_su_t_t0(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 15; ++i) {
        fom_float s = 0.0;
        for (p = FOM_OP_SU_T_T0_ROWPTR[i]; p < FOM_OP_SU_T_T0_ROWPTR[i + 1]; ++p) s += FOM_OP_SU_T_T0_VALS[p] * x[FOM_OP_SU_T_T0_COLIDX[p]];
        out[i] += s;
    }
}
