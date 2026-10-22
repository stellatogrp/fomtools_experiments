#include "fom_ops.h"
#include "fom_data.h"

void fom_apply_t0_state(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 49; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t0_state_ROWPTR[i]; p < FOM_MAT_t0_state_ROWPTR[i + 1]; ++p) s += FOM_MAT_t0_state_VALS[p] * x[FOM_MAT_t0_state_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t1_state(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 10; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t1_state_ROWPTR[i]; p < FOM_MAT_t1_state_ROWPTR[i + 1]; ++p) s += FOM_MAT_t1_state_VALS[p] * x[FOM_MAT_t1_state_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t1_t_t0(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 10; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t1_t_t0_ROWPTR[i]; p < FOM_MAT_t1_t_t0_ROWPTR[i + 1]; ++p) s += FOM_MAT_t1_t_t0_VALS[p] * x[FOM_MAT_t1_t_t0_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_su_state(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 74; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_su_state_ROWPTR[i]; p < FOM_MAT_su_state_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_state_VALS[p] * x[FOM_MAT_su_state_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_su_t_t0(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 74; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_su_t_t0_ROWPTR[i]; p < FOM_MAT_su_t_t0_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t0_VALS[p] * x[FOM_MAT_su_t_t0_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_su_t_t1(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 74; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_su_t_t1_ROWPTR[i]; p < FOM_MAT_su_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t1_VALS[p] * x[FOM_MAT_su_t_t1_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_su_p_p0(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 74; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_su_p_p0_ROWPTR[i]; p < FOM_MAT_su_p_p0_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_p_p0_VALS[p] * x[FOM_MAT_su_p_p0_COLIDX[p]];
        out[i] += s;
    }
}

void fom_fused_t1(const fom_float *state, const fom_float *t_t0, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 10; ++i) {
        fom_float s;
        s = 0.0;
        for (p = FOM_MAT_t1_state_ROWPTR[i]; p < FOM_MAT_t1_state_ROWPTR[i + 1]; ++p) s += FOM_MAT_t1_state_VALS[p] * state[FOM_MAT_t1_state_COLIDX[p]];
        out[i] += s;
        s = 0.0;
        for (p = FOM_MAT_t1_t_t0_ROWPTR[i]; p < FOM_MAT_t1_t_t0_ROWPTR[i + 1]; ++p) s += FOM_MAT_t1_t_t0_VALS[p] * t_t0[FOM_MAT_t1_t_t0_COLIDX[p]];
        out[i] += s;
    }
}

void fom_fused_su(const fom_float *state, const fom_float *t_t0, const fom_float *t_t1, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 74; ++i) {
        fom_float s;
        s = 0.0;
        for (p = FOM_MAT_su_state_ROWPTR[i]; p < FOM_MAT_su_state_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_state_VALS[p] * state[FOM_MAT_su_state_COLIDX[p]];
        out[i] += s;
        s = 0.0;
        for (p = FOM_MAT_su_t_t0_ROWPTR[i]; p < FOM_MAT_su_t_t0_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t0_VALS[p] * t_t0[FOM_MAT_su_t_t0_COLIDX[p]];
        out[i] += s;
        s = 0.0;
        for (p = FOM_MAT_su_t_t1_ROWPTR[i]; p < FOM_MAT_su_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t1_VALS[p] * t_t1[FOM_MAT_su_t_t1_COLIDX[p]];
        out[i] += s;
    }
}

void fom_step_t0(const fom_float *state, const fom_float *bias, const FomAct_t0 *act, fom_float *out) {
    fom_int i, r, p;
    for (r = 0; r < 49; ++r) {
        fom_float s, v;
        i = r;
        v = bias[i];
        s = 0.0;
        for (p = FOM_MAT_t0_state_ROWPTR[i]; p < FOM_MAT_t0_state_ROWPTR[i + 1]; ++p) s += FOM_MAT_t0_state_VALS[p] * state[FOM_MAT_t0_state_COLIDX[p]];
        v += s;
        if (v < act->lo[i]) v = act->lo[i];
        if (v > act->hi[i]) v = act->hi[i];
        out[i] = v;
    }
}

void fom_step_t1(const fom_float *state, const fom_float *t_t0, const fom_float *bias, const FomAct_t1 *act, fom_float *out) {
    fom_int i, r, p;
    for (r = 0; r < 10; ++r) {
        fom_float s, v;
        i = r;
        v = bias[i];
        s = 0.0;
        for (p = FOM_MAT_t1_state_ROWPTR[i]; p < FOM_MAT_t1_state_ROWPTR[i + 1]; ++p) s += FOM_MAT_t1_state_VALS[p] * state[FOM_MAT_t1_state_COLIDX[p]];
        v += s;
        s = 0.0;
        for (p = FOM_MAT_t1_t_t0_ROWPTR[i]; p < FOM_MAT_t1_t_t0_ROWPTR[i + 1]; ++p) s += FOM_MAT_t1_t_t0_VALS[p] * t_t0[FOM_MAT_t1_t_t0_COLIDX[p]];
        v += s;
        if (v < act->lo[i]) v = act->lo[i];
        out[i] = v;
    }
}

void fom_step_su(const fom_float *state, const fom_float *t_t0, const fom_float *t_t1, const fom_float *bias, fom_float *out) {
    fom_int i, r, p;
    for (r = 0; r < 74; ++r) {
        fom_float s, v;
        i = r;
        v = bias[i];
        s = 0.0;
        for (p = FOM_MAT_su_state_ROWPTR[i]; p < FOM_MAT_su_state_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_state_VALS[p] * state[FOM_MAT_su_state_COLIDX[p]];
        v += s;
        s = 0.0;
        for (p = FOM_MAT_su_t_t0_ROWPTR[i]; p < FOM_MAT_su_t_t0_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t0_VALS[p] * t_t0[FOM_MAT_su_t_t0_COLIDX[p]];
        v += s;
        s = 0.0;
        for (p = FOM_MAT_su_t_t1_ROWPTR[i]; p < FOM_MAT_su_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t1_VALS[p] * t_t1[FOM_MAT_su_t_t1_COLIDX[p]];
        v += s;
        out[i] = v;
    }
}

void fom_fused_su__rest(const fom_float *state, const fom_float *t_t0, const fom_float *t_t1, fom_float *out) {
    fom_int i, r, p;
    for (r = 0; r < FOM_REST_su_N; ++r) {
        fom_float s;
        i = FOM_REST_su_ROWS[r];
        s = 0.0;
        for (p = FOM_MAT_su_state_ROWPTR[i]; p < FOM_MAT_su_state_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_state_VALS[p] * state[FOM_MAT_su_state_COLIDX[p]];
        out[i] += s;
        s = 0.0;
        for (p = FOM_MAT_su_t_t0_ROWPTR[i]; p < FOM_MAT_su_t_t0_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t0_VALS[p] * t_t0[FOM_MAT_su_t_t0_COLIDX[p]];
        out[i] += s;
        s = 0.0;
        for (p = FOM_MAT_su_t_t1_ROWPTR[i]; p < FOM_MAT_su_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t1_VALS[p] * t_t1[FOM_MAT_su_t_t1_COLIDX[p]];
        out[i] += s;
    }
}

void fom_step_su__rest(const fom_float *state, const fom_float *t_t0, const fom_float *t_t1, const fom_float *bias, fom_float *out) {
    fom_int i, r, p;
    for (r = 0; r < FOM_REST_su_N; ++r) {
        fom_float s, v;
        i = FOM_REST_su_ROWS[r];
        v = bias[i];
        s = 0.0;
        for (p = FOM_MAT_su_state_ROWPTR[i]; p < FOM_MAT_su_state_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_state_VALS[p] * state[FOM_MAT_su_state_COLIDX[p]];
        v += s;
        s = 0.0;
        for (p = FOM_MAT_su_t_t0_ROWPTR[i]; p < FOM_MAT_su_t_t0_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t0_VALS[p] * t_t0[FOM_MAT_su_t_t0_COLIDX[p]];
        v += s;
        s = 0.0;
        for (p = FOM_MAT_su_t_t1_ROWPTR[i]; p < FOM_MAT_su_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t1_VALS[p] * t_t1[FOM_MAT_su_t_t1_COLIDX[p]];
        v += s;
        out[i] = v;
    }
}

void fom_iterate(const fom_float *state, const fom_float *bias_t0, const fom_float *bias_t1, const fom_float *bias_su, const FomAct_t0 *act_t0, const FomAct_t1 *act_t1, fom_float *t_t0, fom_float *t_t1, fom_float *new_state) {
    const fom_float *x = state; const fom_float *lo0 = act_t0->lo, *hi0 = act_t0->hi, *lo1 = act_t1->lo;
    fom_float v, d, s;
    fom_float a59 = bias_su[59], a60 = bias_su[60], a61 = bias_su[61], a62 = bias_su[62], a63 = bias_su[63], a64 = bias_su[64], a65 = bias_su[65], a66 = bias_su[66], a67 = bias_su[67], a68 = bias_su[68], a69 = bias_su[69], a70 = bias_su[70], a71 = bias_su[71], a72 = bias_su[72], a73 = bias_su[73];
    { const fom_float xg = x[49];
    v = bias_t0[0] + x[0] + -0.08226303133234722 * xg + -0.0696550616260675 * x[59]; v = v < lo0[0] ? lo0[0] : v; v = v > hi0[0] ? hi0[0] : v; d = x[0] - 2.0 * v; t_t0[0] = v; new_state[0] = bias_su[0] + v;
    s = d; a59 += -0.0696550616260675 * d;
    v = bias_t0[1] + x[1] + -0.08226303133234722 * xg + -0.2558824463341391 * x[60]; v = v < lo0[1] ? lo0[1] : v; v = v > hi0[1] ? hi0[1] : v; d = x[1] - 2.0 * v; t_t0[1] = v; new_state[1] = bias_su[1] + v;
    s += d; a60 += -0.2558824463341391 * d;
    v = bias_t0[2] + x[2] + -0.08226303133234722 * xg + -0.13345673088354346 * x[62]; v = v < lo0[2] ? lo0[2] : v; v = v > hi0[2] ? hi0[2] : v; d = x[2] - 2.0 * v; t_t0[2] = v; new_state[2] = bias_su[2] + v;
    s += d; a62 += -0.13345673088354346 * d;
    v = bias_t0[3] + x[3] + -0.08226303133234722 * xg + -0.024964136788999065 * x[65]; v = v < lo0[3] ? lo0[3] : v; v = v > hi0[3] ? hi0[3] : v; d = x[3] - 2.0 * v; t_t0[3] = v; new_state[3] = bias_su[3] + v;
    s += d; a65 += -0.024964136788999065 * d;
    v = bias_t0[4] + x[4] + -0.08226303133234722 * xg + -0.021966725812901995 * x[71]; v = v < lo0[4] ? lo0[4] : v; v = v > hi0[4] ? hi0[4] : v; d = x[4] - 2.0 * v; t_t0[4] = v; new_state[4] = bias_su[4] + v;
    s += d; a71 += -0.021966725812901995 * d;
    v = bias_t1[0] + -0.08226303133234722 * s + xg; v = v < lo1[0] ? lo1[0] : v; t_t1[0] = v; new_state[49] = bias_su[49] + v; }
    { const fom_float xg = x[50];
    v = bias_t0[5] + x[5] + -0.08226303133234722 * xg + -0.03758350075816168 * x[60]; v = v < lo0[5] ? lo0[5] : v; v = v > hi0[5] ? hi0[5] : v; d = x[5] - 2.0 * v; t_t0[5] = v; new_state[5] = bias_su[5] + v;
    s = d; a60 += -0.03758350075816168 * d;
    v = bias_t0[6] + x[6] + -0.08226303133234722 * xg + -0.6175238612165264 * x[61]; v = v < lo0[6] ? lo0[6] : v; v = v > hi0[6] ? hi0[6] : v; d = x[6] - 2.0 * v; t_t0[6] = v; new_state[6] = bias_su[6] + v;
    s += d; a61 += -0.6175238612165264 * d;
    v = bias_t0[7] + x[7] + -0.08226303133234722 * xg + -0.644834128427094 * x[64]; v = v < lo0[7] ? lo0[7] : v; v = v > hi0[7] ? hi0[7] : v; d = x[7] - 2.0 * v; t_t0[7] = v; new_state[7] = bias_su[7] + v;
    s += d; a64 += -0.644834128427094 * d;
    v = bias_t0[8] + x[8] + -0.08226303133234722 * xg + -0.037882339410371904 * x[65]; v = v < lo0[8] ? lo0[8] : v; v = v > hi0[8] ? hi0[8] : v; d = x[8] - 2.0 * v; t_t0[8] = v; new_state[8] = bias_su[8] + v;
    s += d; a65 += -0.037882339410371904 * d;
    v = bias_t0[9] + x[9] + -0.08226303133234722 * xg + -0.7947858773306499 * x[66]; v = v < lo0[9] ? lo0[9] : v; v = v > hi0[9] ? hi0[9] : v; d = x[9] - 2.0 * v; t_t0[9] = v; new_state[9] = bias_su[9] + v;
    s += d; a66 += -0.7947858773306499 * d;
    v = bias_t0[10] + x[10] + -0.08226303133234722 * xg + -0.01729459933480772 * x[69]; v = v < lo0[10] ? lo0[10] : v; v = v > hi0[10] ? hi0[10] : v; d = x[10] - 2.0 * v; t_t0[10] = v; new_state[10] = bias_su[10] + v;
    s += d; a69 += -0.01729459933480772 * d;
    v = bias_t0[11] + x[11] + -0.08226303133234722 * xg + -0.08961892294526969 * x[73]; v = v < lo0[11] ? lo0[11] : v; v = v > hi0[11] ? hi0[11] : v; d = x[11] - 2.0 * v; t_t0[11] = v; new_state[11] = bias_su[11] + v;
    s += d; a73 += -0.08961892294526969 * d;
    v = bias_t1[1] + -0.08226303133234722 * s + xg; v = v < lo1[1] ? lo1[1] : v; t_t1[1] = v; new_state[50] = bias_su[50] + v; }
    { const fom_float xg = x[51];
    v = bias_t0[12] + x[12] + -0.08226303133234722 * xg + -0.33760783556686547 * x[59]; v = v < lo0[12] ? lo0[12] : v; v = v > hi0[12] ? hi0[12] : v; d = x[12] - 2.0 * v; t_t0[12] = v; new_state[12] = bias_su[12] + v;
    s = d; a59 += -0.33760783556686547 * d;
    v = bias_t0[13] + x[13] + -0.08226303133234722 * xg + -0.04030704747187439 * x[63]; v = v < lo0[13] ? lo0[13] : v; v = v > hi0[13] ? hi0[13] : v; d = x[13] - 2.0 * v; t_t0[13] = v; new_state[13] = bias_su[13] + v;
    s += d; a63 += -0.04030704747187439 * d;
    v = bias_t0[14] + x[14] + -0.08226303133234722 * xg + -0.00976037797556846 * x[65]; v = v < lo0[14] ? lo0[14] : v; v = v > hi0[14] ? hi0[14] : v; d = x[14] - 2.0 * v; t_t0[14] = v; new_state[14] = bias_su[14] + v;
    s += d; a65 += -0.00976037797556846 * d;
    v = bias_t0[15] + x[15] + -0.08226303133234722 * xg + -0.4683158478119333 * x[73]; v = v < lo0[15] ? lo0[15] : v; v = v > hi0[15] ? hi0[15] : v; d = x[15] - 2.0 * v; t_t0[15] = v; new_state[15] = bias_su[15] + v;
    s += d; a73 += -0.4683158478119333 * d;
    v = bias_t1[2] + -0.08226303133234722 * s + xg; v = v < lo1[2] ? lo1[2] : v; t_t1[2] = v; new_state[51] = bias_su[51] + v; }
    { const fom_float xg = x[52];
    v = bias_t0[16] + x[16] + -0.08226303133234722 * xg + -0.2683650108072722 * x[59]; v = v < lo0[16] ? lo0[16] : v; v = v > hi0[16] ? hi0[16] : v; d = x[16] - 2.0 * v; t_t0[16] = v; new_state[16] = bias_su[16] + v;
    s = d; a59 += -0.2683650108072722 * d;
    v = bias_t0[17] + x[17] + -0.08226303133234722 * xg + -0.04238843516524545 * x[61]; v = v < lo0[17] ? lo0[17] : v; v = v > hi0[17] ? hi0[17] : v; d = x[17] - 2.0 * v; t_t0[17] = v; new_state[17] = bias_su[17] + v;
    s += d; a61 += -0.04238843516524545 * d;
    v = bias_t0[18] + x[18] + -0.08226303133234722 * xg + -0.029151569883073398 * x[63]; v = v < lo0[18] ? lo0[18] : v; v = v > hi0[18] ? hi0[18] : v; d = x[18] - 2.0 * v; t_t0[18] = v; new_state[18] = bias_su[18] + v;
    s += d; a63 += -0.029151569883073398 * d;
    v = bias_t0[19] + x[19] + -0.08226303133234722 * xg + -0.3601143643899065 * x[64]; v = v < lo0[19] ? lo0[19] : v; v = v > hi0[19] ? hi0[19] : v; d = x[19] - 2.0 * v; t_t0[19] = v; new_state[19] = bias_su[19] + v;
    s += d; a64 += -0.3601143643899065 * d;
    v = bias_t0[20] + x[20] + -0.08226303133234722 * xg + -0.2765836919316892 * x[65]; v = v < lo0[20] ? lo0[20] : v; v = v > hi0[20] ? hi0[20] : v; d = x[20] - 2.0 * v; t_t0[20] = v; new_state[20] = bias_su[20] + v;
    s += d; a65 += -0.2765836919316892 * d;
    v = bias_t0[21] + x[21] + -0.08226303133234722 * xg + -0.03473597091735419 * x[70]; v = v < lo0[21] ? lo0[21] : v; v = v > hi0[21] ? hi0[21] : v; d = x[21] - 2.0 * v; t_t0[21] = v; new_state[21] = bias_su[21] + v;
    s += d; a70 += -0.03473597091735419 * d;
    v = bias_t0[22] + x[22] + -0.08226303133234722 * xg + -0.022087418331416996 * x[73]; v = v < lo0[22] ? lo0[22] : v; v = v > hi0[22] ? hi0[22] : v; d = x[22] - 2.0 * v; t_t0[22] = v; new_state[22] = bias_su[22] + v;
    s += d; a73 += -0.022087418331416996 * d;
    v = bias_t1[3] + -0.08226303133234722 * s + xg; v = v < lo1[3] ? lo1[3] : v; t_t1[3] = v; new_state[52] = bias_su[52] + v; }
    { const fom_float xg = x[53];
    v = bias_t0[23] + x[23] + -0.08226303133234722 * xg + -0.6113240882469336 * x[62]; v = v < lo0[23] ? lo0[23] : v; v = v > hi0[23] ? hi0[23] : v; d = x[23] - 2.0 * v; t_t0[23] = v; new_state[23] = bias_su[23] + v;
    s = d; a62 += -0.6113240882469336 * d;
    v = bias_t0[24] + x[24] + -0.08226303133234722 * xg + -0.013160484266981857 * x[68]; v = v < lo0[24] ? lo0[24] : v; v = v > hi0[24] ? hi0[24] : v; d = x[24] - 2.0 * v; t_t0[24] = v; new_state[24] = bias_su[24] + v;
    s += d; a68 += -0.013160484266981857 * d;
    v = bias_t0[25] + x[25] + -0.08226303133234722 * xg + -0.15819624000738933 * x[70]; v = v < lo0[25] ? lo0[25] : v; v = v > hi0[25] ? hi0[25] : v; d = x[25] - 2.0 * v; t_t0[25] = v; new_state[25] = bias_su[25] + v;
    s += d; a70 += -0.15819624000738933 * d;
    v = bias_t1[4] + -0.08226303133234722 * s + xg; v = v < lo1[4] ? lo1[4] : v; t_t1[4] = v; new_state[53] = bias_su[53] + v; }
    { const fom_float xg = x[54];
    v = bias_t0[26] + x[26] + -0.08226303133234722 * xg + -0.037081661631488835 * x[59]; v = v < lo0[26] ? lo0[26] : v; v = v > hi0[26] ? hi0[26] : v; d = x[26] - 2.0 * v; t_t0[26] = v; new_state[26] = bias_su[26] + v;
    s = d; a59 += -0.037081661631488835 * d;
    v = bias_t0[27] + x[27] + -0.08226303133234722 * xg + -0.08327630054464727 * x[60]; v = v < lo0[27] ? lo0[27] : v; v = v > hi0[27] ? hi0[27] : v; d = x[27] - 2.0 * v; t_t0[27] = v; new_state[27] = bias_su[27] + v;
    s += d; a60 += -0.08327630054464727 * d;
    v = bias_t0[28] + x[28] + -0.08226303133234722 * xg + -0.5843595834842807 * x[63]; v = v < lo0[28] ? lo0[28] : v; v = v > hi0[28] ? hi0[28] : v; d = x[28] - 2.0 * v; t_t0[28] = v; new_state[28] = bias_su[28] + v;
    s += d; a63 += -0.5843595834842807 * d;
    v = bias_t0[29] + x[29] + -0.08226303133234722 * xg + -0.05109107424493328 * x[64]; v = v < lo0[29] ? lo0[29] : v; v = v > hi0[29] ? hi0[29] : v; d = x[29] - 2.0 * v; t_t0[29] = v; new_state[29] = bias_su[29] + v;
    s += d; a64 += -0.05109107424493328 * d;
    v = bias_t0[30] + x[30] + -0.08226303133234722 * xg + -0.014854938892497713 * x[66]; v = v < lo0[30] ? lo0[30] : v; v = v > hi0[30] ? hi0[30] : v; d = x[30] - 2.0 * v; t_t0[30] = v; new_state[30] = bias_su[30] + v;
    s += d; a66 += -0.014854938892497713 * d;
    v = bias_t0[31] + x[31] + -0.08226303133234722 * xg + -0.6037269576533859 * x[69]; v = v < lo0[31] ? lo0[31] : v; v = v > hi0[31] ? hi0[31] : v; d = x[31] - 2.0 * v; t_t0[31] = v; new_state[31] = bias_su[31] + v;
    s += d; a69 += -0.6037269576533859 * d;
    v = bias_t0[32] + x[32] + -0.08226303133234722 * xg + -0.021590409041307837 * x[71]; v = v < lo0[32] ? lo0[32] : v; v = v > hi0[32] ? hi0[32] : v; d = x[32] - 2.0 * v; t_t0[32] = v; new_state[32] = bias_su[32] + v;
    s += d; a71 += -0.021590409041307837 * d;
    v = bias_t0[33] + x[33] + -0.08226303133234722 * xg + -0.011556395626565408 * x[72]; v = v < lo0[33] ? lo0[33] : v; v = v > hi0[33] ? hi0[33] : v; d = x[33] - 2.0 * v; t_t0[33] = v; new_state[33] = bias_su[33] + v;
    s += d; a72 += -0.011556395626565408 * d;
    v = bias_t1[5] + -0.08226303133234722 * s + xg; v = v < lo1[5] ? lo1[5] : v; t_t1[5] = v; new_state[54] = bias_su[54] + v; }
    { const fom_float xg = x[55];
    v = bias_t0[34] + x[34] + -0.08226303133234722 * xg + -0.3864320872326754 * x[68]; v = v < lo0[34] ? lo0[34] : v; v = v > hi0[34] ? hi0[34] : v; d = x[34] - 2.0 * v; t_t0[34] = v; new_state[34] = bias_su[34] + v;
    s = d; a68 += -0.3864320872326754 * d;
    v = bias_t0[35] + x[35] + -0.08226303133234722 * xg + -0.2585411937087774 * x[69]; v = v < lo0[35] ? lo0[35] : v; v = v > hi0[35] ? hi0[35] : v; d = x[35] - 2.0 * v; t_t0[35] = v; new_state[35] = bias_su[35] + v;
    s += d; a69 += -0.2585411937087774 * d;
    v = bias_t0[36] + x[36] + -0.08226303133234722 * xg + -0.41032977319887487 * x[73]; v = v < lo0[36] ? lo0[36] : v; v = v > hi0[36] ? hi0[36] : v; d = x[36] - 2.0 * v; t_t0[36] = v; new_state[36] = bias_su[36] + v;
    s += d; a73 += -0.41032977319887487 * d;
    v = bias_t1[6] + -0.08226303133234722 * s + xg; v = v < lo1[6] ? lo1[6] : v; t_t1[6] = v; new_state[55] = bias_su[55] + v; }
    { const fom_float xg = x[56];
    v = bias_t0[37] + x[37] + -0.08226303133234722 * xg + -0.023067940734351828 * x[60]; v = v < lo0[37] ? lo0[37] : v; v = v > hi0[37] ? hi0[37] : v; d = x[37] - 2.0 * v; t_t0[37] = v; new_state[37] = bias_su[37] + v;
    s = d; a60 += -0.023067940734351828 * d;
    v = bias_t0[38] + x[38] + -0.08226303133234722 * xg + -0.018156664303604776 * x[66]; v = v < lo0[38] ? lo0[38] : v; v = v > hi0[38] ? hi0[38] : v; d = x[38] - 2.0 * v; t_t0[38] = v; new_state[38] = bias_su[38] + v;
    s += d; a66 += -0.018156664303604776 * d;
    v = bias_t1[7] + -0.08226303133234722 * s + xg; v = v < lo1[7] ? lo1[7] : v; t_t1[7] = v; new_state[56] = bias_su[56] + v; }
    { const fom_float xg = x[57];
    v = bias_t0[39] + x[39] + -0.08226303133234722 * xg + -0.37063781296154064 * x[59]; v = v < lo0[39] ? lo0[39] : v; v = v > hi0[39] ? hi0[39] : v; d = x[39] - 2.0 * v; t_t0[39] = v; new_state[39] = bias_su[39] + v;
    s = d; a59 += -0.37063781296154064 * d;
    v = bias_t0[40] + x[40] + -0.08226303133234722 * xg + -0.13174930294003487 * x[67]; v = v < lo0[40] ? lo0[40] : v; v = v > hi0[40] ? hi0[40] : v; d = x[40] - 2.0 * v; t_t0[40] = v; new_state[40] = bias_su[40] + v;
    s += d; a67 += -0.13174930294003487 * d;
    v = bias_t0[41] + x[41] + -0.08226303133234722 * xg + -0.14484430995648803 * x[69]; v = v < lo0[41] ? lo0[41] : v; v = v > hi0[41] ? hi0[41] : v; d = x[41] - 2.0 * v; t_t0[41] = v; new_state[41] = bias_su[41] + v;
    s += d; a69 += -0.14484430995648803 * d;
    v = bias_t0[42] + x[42] + -0.08226303133234722 * xg + -0.03484606682994428 * x[71]; v = v < lo0[42] ? lo0[42] : v; v = v > hi0[42] ? hi0[42] : v; d = x[42] - 2.0 * v; t_t0[42] = v; new_state[42] = bias_su[42] + v;
    s += d; a71 += -0.03484606682994428 * d;
    v = bias_t0[43] + x[43] + -0.08226303133234722 * xg + -0.09852027593353693 * x[72]; v = v < lo0[43] ? lo0[43] : v; v = v > hi0[43] ? hi0[43] : v; d = x[43] - 2.0 * v; t_t0[43] = v; new_state[43] = bias_su[43] + v;
    s += d; a72 += -0.09852027593353693 * d;
    v = bias_t1[8] + -0.08226303133234722 * s + xg; v = v < lo1[8] ? lo1[8] : v; t_t1[8] = v; new_state[57] = bias_su[57] + v; }
    { const fom_float xg = x[58];
    v = bias_t0[44] + x[44] + -0.08226303133234722 * xg + -0.03036587998612799 * x[65]; v = v < lo0[44] ? lo0[44] : v; v = v > hi0[44] ? hi0[44] : v; d = x[44] - 2.0 * v; t_t0[44] = v; new_state[44] = bias_su[44] + v;
    s = d; a65 += -0.03036587998612799 * d;
    v = bias_t0[45] + x[45] + -0.08226303133234722 * xg + -0.14046939723165486 * x[67]; v = v < lo0[45] ? lo0[45] : v; v = v > hi0[45] ? hi0[45] : v; d = x[45] - 2.0 * v; t_t0[45] = v; new_state[45] = bias_su[45] + v;
    s += d; a67 += -0.14046939723165486 * d;
    v = bias_t0[46] + x[46] + -0.08226303133234722 * xg + -0.3265308594327319 * x[71]; v = v < lo0[46] ? lo0[46] : v; v = v > hi0[46] ? hi0[46] : v; d = x[46] - 2.0 * v; t_t0[46] = v; new_state[46] = bias_su[46] + v;
    s += d; a71 += -0.3265308594327319 * d;
    v = bias_t0[47] + x[47] + -0.08226303133234722 * xg + -0.17994004051645768 * x[72]; v = v < lo0[47] ? lo0[47] : v; v = v > hi0[47] ? hi0[47] : v; d = x[47] - 2.0 * v; t_t0[47] = v; new_state[47] = bias_su[47] + v;
    s += d; a72 += -0.17994004051645768 * d;
    v = bias_t0[48] + x[48] + -0.08226303133234722 * xg + -0.22496888289546163 * x[73]; v = v < lo0[48] ? lo0[48] : v; v = v > hi0[48] ? hi0[48] : v; d = x[48] - 2.0 * v; t_t0[48] = v; new_state[48] = bias_su[48] + v;
    s += d; a73 += -0.22496888289546163 * d;
    v = bias_t1[9] + -0.08226303133234722 * s + xg; v = v < lo1[9] ? lo1[9] : v; t_t1[9] = v; new_state[58] = bias_su[58] + v; }
    new_state[59] = a59 + x[59];
    new_state[60] = a60 + x[60];
    new_state[61] = a61 + x[61];
    new_state[62] = a62 + x[62];
    new_state[63] = a63 + x[63];
    new_state[64] = a64 + x[64];
    new_state[65] = a65 + x[65];
    new_state[66] = a66 + x[66];
    new_state[67] = a67 + x[67];
    new_state[68] = a68 + x[68];
    new_state[69] = a69 + x[69];
    new_state[70] = a70 + x[70];
    new_state[71] = a71 + x[71];
    new_state[72] = a72 + x[72];
    new_state[73] = a73 + x[73];
}
