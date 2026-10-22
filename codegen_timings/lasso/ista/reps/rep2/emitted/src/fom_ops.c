#include "fom_ops.h"
#include "fom_data.h"

void fom_apply_t0_state(const fom_float *x, fom_float *out) {
    fom_int i, j;
    for (i = 0; i < 50; ++i) {
        fom_float s = 0.0;
        for (j = 0; j < 50; ++j) s += FOM_MAT_t0_state[i * 50 + j] * x[j];
        out[i] += s;
    }
}

void fom_apply_t0_p_p0(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 50; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t0_p_p0_ROWPTR[i]; p < FOM_MAT_t0_p_p0_ROWPTR[i + 1]; ++p) s += FOM_MAT_t0_p_p0_VALS[p] * x[FOM_MAT_t0_p_p0_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_su_t_t0(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 50; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_su_t_t0_ROWPTR[i]; p < FOM_MAT_su_t_t0_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t0_VALS[p] * x[FOM_MAT_su_t_t0_COLIDX[p]];
        out[i] += s;
    }
}

void fom_step_t0(const fom_float *state, const fom_float *bias, const FomAct_t0 *act, fom_float *out) {
    fom_int i, r, j;
    for (r = 0; r < 50; ++r) {
        fom_float s, v;
        i = r;
        v = bias[i];
        s = 0.0;
        for (j = 0; j < 50; ++j) s += FOM_MAT_t0_state[i * 50 + j] * state[j];
        v += s;
        v = (v > act->lambd) ? (v - act->lambd) : ((v < -act->lambd) ? (v + act->lambd) : 0.0);
        out[i] = v;
    }
}

void fom_step_su(const fom_float *t_t0, const fom_float *bias, fom_float *out) {
    fom_int i, r, p;
    for (r = 0; r < 50; ++r) {
        fom_float s, v;
        i = r;
        v = bias[i];
        s = 0.0;
        for (p = FOM_MAT_su_t_t0_ROWPTR[i]; p < FOM_MAT_su_t_t0_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t0_VALS[p] * t_t0[FOM_MAT_su_t_t0_COLIDX[p]];
        v += s;
        out[i] = v;
    }
}

void fom_iterate(const fom_float *state, const fom_float *bias_t0, const fom_float *bias_su, const FomAct_t0 *act_t0, fom_float *t_t0, fom_float *new_state) {
    fom_step_t0(state, bias_t0, act_t0, t_t0);
    fom_step_su(t_t0, bias_su, new_state);
}
