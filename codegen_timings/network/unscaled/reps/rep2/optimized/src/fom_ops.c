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
    fom_float v;
    fom_float x0 = state[0], x1 = state[1], x2 = state[2], x3 = state[3], x4 = state[4], x5 = state[5], x6 = state[6], x7 = state[7], x8 = state[8], x9 = state[9], x10 = state[10], x11 = state[11], x12 = state[12], x13 = state[13], x14 = state[14], x15 = state[15], x16 = state[16], x17 = state[17], x18 = state[18], x19 = state[19], x20 = state[20], x21 = state[21], x22 = state[22], x23 = state[23], x24 = state[24], x25 = state[25], x26 = state[26], x27 = state[27], x28 = state[28], x29 = state[29], x30 = state[30], x31 = state[31], x32 = state[32], x33 = state[33], x34 = state[34], x35 = state[35], x36 = state[36], x37 = state[37], x38 = state[38], x39 = state[39], x40 = state[40], x41 = state[41], x42 = state[42], x43 = state[43], x44 = state[44], x45 = state[45], x46 = state[46], x47 = state[47], x48 = state[48], x49 = state[49], x50 = state[50], x51 = state[51], x52 = state[52], x53 = state[53], x54 = state[54], x55 = state[55], x56 = state[56], x57 = state[57], x58 = state[58], x59 = state[59], x60 = state[60], x61 = state[61], x62 = state[62], x63 = state[63], x64 = state[64], x65 = state[65], x66 = state[66], x67 = state[67], x68 = state[68], x69 = state[69], x70 = state[70], x71 = state[71], x72 = state[72], x73 = state[73];
    fom_float a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30, a31, a32, a33, a34, a35, a36, a37, a38, a39, a40, a41, a42, a43, a44, a45, a46, a47, a48;
    fom_float w0, w1, w2, w3, w4, w5, w6, w7, w8, w9, w10, w11, w12, w13, w14, w15, w16, w17, w18, w19, w20, w21, w22, w23, w24, w25, w26, w27, w28, w29, w30, w31, w32, w33, w34, w35, w36, w37, w38, w39, w40, w41, w42, w43, w44, w45, w46, w47, w48;
    fom_float b0, b1, b2, b3, b4, b5, b6, b7, b8, b9;
    const fom_float *lo0 = act_t0->lo, *hi0 = act_t0->hi, *lo1 = act_t1->lo;
    v = bias_t0[0] + x0 + -0.08226303133234722 * x49 + -0.0696550616260675 * x59;
    v = v < lo0[0] ? lo0[0] : v; v = v > hi0[0] ? hi0[0] : v; a0 = v;
    v = bias_t0[1] + x1 + -0.08226303133234722 * x49 + -0.2558824463341391 * x60;
    v = v < lo0[1] ? lo0[1] : v; v = v > hi0[1] ? hi0[1] : v; a1 = v;
    v = bias_t0[2] + x2 + -0.08226303133234722 * x49 + -0.13345673088354346 * x62;
    v = v < lo0[2] ? lo0[2] : v; v = v > hi0[2] ? hi0[2] : v; a2 = v;
    v = bias_t0[3] + x3 + -0.08226303133234722 * x49 + -0.024964136788999065 * x65;
    v = v < lo0[3] ? lo0[3] : v; v = v > hi0[3] ? hi0[3] : v; a3 = v;
    v = bias_t0[4] + x4 + -0.08226303133234722 * x49 + -0.021966725812901995 * x71;
    v = v < lo0[4] ? lo0[4] : v; v = v > hi0[4] ? hi0[4] : v; a4 = v;
    v = bias_t0[5] + x5 + -0.08226303133234722 * x50 + -0.03758350075816168 * x60;
    v = v < lo0[5] ? lo0[5] : v; v = v > hi0[5] ? hi0[5] : v; a5 = v;
    v = bias_t0[6] + x6 + -0.08226303133234722 * x50 + -0.6175238612165264 * x61;
    v = v < lo0[6] ? lo0[6] : v; v = v > hi0[6] ? hi0[6] : v; a6 = v;
    v = bias_t0[7] + x7 + -0.08226303133234722 * x50 + -0.644834128427094 * x64;
    v = v < lo0[7] ? lo0[7] : v; v = v > hi0[7] ? hi0[7] : v; a7 = v;
    v = bias_t0[8] + x8 + -0.08226303133234722 * x50 + -0.037882339410371904 * x65;
    v = v < lo0[8] ? lo0[8] : v; v = v > hi0[8] ? hi0[8] : v; a8 = v;
    v = bias_t0[9] + x9 + -0.08226303133234722 * x50 + -0.7947858773306499 * x66;
    v = v < lo0[9] ? lo0[9] : v; v = v > hi0[9] ? hi0[9] : v; a9 = v;
    v = bias_t0[10] + x10 + -0.08226303133234722 * x50 + -0.01729459933480772 * x69;
    v = v < lo0[10] ? lo0[10] : v; v = v > hi0[10] ? hi0[10] : v; a10 = v;
    v = bias_t0[11] + x11 + -0.08226303133234722 * x50 + -0.08961892294526969 * x73;
    v = v < lo0[11] ? lo0[11] : v; v = v > hi0[11] ? hi0[11] : v; a11 = v;
    v = bias_t0[12] + x12 + -0.08226303133234722 * x51 + -0.33760783556686547 * x59;
    v = v < lo0[12] ? lo0[12] : v; v = v > hi0[12] ? hi0[12] : v; a12 = v;
    v = bias_t0[13] + x13 + -0.08226303133234722 * x51 + -0.04030704747187439 * x63;
    v = v < lo0[13] ? lo0[13] : v; v = v > hi0[13] ? hi0[13] : v; a13 = v;
    v = bias_t0[14] + x14 + -0.08226303133234722 * x51 + -0.00976037797556846 * x65;
    v = v < lo0[14] ? lo0[14] : v; v = v > hi0[14] ? hi0[14] : v; a14 = v;
    v = bias_t0[15] + x15 + -0.08226303133234722 * x51 + -0.4683158478119333 * x73;
    v = v < lo0[15] ? lo0[15] : v; v = v > hi0[15] ? hi0[15] : v; a15 = v;
    v = bias_t0[16] + x16 + -0.08226303133234722 * x52 + -0.2683650108072722 * x59;
    v = v < lo0[16] ? lo0[16] : v; v = v > hi0[16] ? hi0[16] : v; a16 = v;
    v = bias_t0[17] + x17 + -0.08226303133234722 * x52 + -0.04238843516524545 * x61;
    v = v < lo0[17] ? lo0[17] : v; v = v > hi0[17] ? hi0[17] : v; a17 = v;
    v = bias_t0[18] + x18 + -0.08226303133234722 * x52 + -0.029151569883073398 * x63;
    v = v < lo0[18] ? lo0[18] : v; v = v > hi0[18] ? hi0[18] : v; a18 = v;
    v = bias_t0[19] + x19 + -0.08226303133234722 * x52 + -0.3601143643899065 * x64;
    v = v < lo0[19] ? lo0[19] : v; v = v > hi0[19] ? hi0[19] : v; a19 = v;
    v = bias_t0[20] + x20 + -0.08226303133234722 * x52 + -0.2765836919316892 * x65;
    v = v < lo0[20] ? lo0[20] : v; v = v > hi0[20] ? hi0[20] : v; a20 = v;
    v = bias_t0[21] + x21 + -0.08226303133234722 * x52 + -0.03473597091735419 * x70;
    v = v < lo0[21] ? lo0[21] : v; v = v > hi0[21] ? hi0[21] : v; a21 = v;
    v = bias_t0[22] + x22 + -0.08226303133234722 * x52 + -0.022087418331416996 * x73;
    v = v < lo0[22] ? lo0[22] : v; v = v > hi0[22] ? hi0[22] : v; a22 = v;
    v = bias_t0[23] + x23 + -0.08226303133234722 * x53 + -0.6113240882469336 * x62;
    v = v < lo0[23] ? lo0[23] : v; v = v > hi0[23] ? hi0[23] : v; a23 = v;
    v = bias_t0[24] + x24 + -0.08226303133234722 * x53 + -0.013160484266981857 * x68;
    v = v < lo0[24] ? lo0[24] : v; v = v > hi0[24] ? hi0[24] : v; a24 = v;
    v = bias_t0[25] + x25 + -0.08226303133234722 * x53 + -0.15819624000738933 * x70;
    v = v < lo0[25] ? lo0[25] : v; v = v > hi0[25] ? hi0[25] : v; a25 = v;
    v = bias_t0[26] + x26 + -0.08226303133234722 * x54 + -0.037081661631488835 * x59;
    v = v < lo0[26] ? lo0[26] : v; v = v > hi0[26] ? hi0[26] : v; a26 = v;
    v = bias_t0[27] + x27 + -0.08226303133234722 * x54 + -0.08327630054464727 * x60;
    v = v < lo0[27] ? lo0[27] : v; v = v > hi0[27] ? hi0[27] : v; a27 = v;
    v = bias_t0[28] + x28 + -0.08226303133234722 * x54 + -0.5843595834842807 * x63;
    v = v < lo0[28] ? lo0[28] : v; v = v > hi0[28] ? hi0[28] : v; a28 = v;
    v = bias_t0[29] + x29 + -0.08226303133234722 * x54 + -0.05109107424493328 * x64;
    v = v < lo0[29] ? lo0[29] : v; v = v > hi0[29] ? hi0[29] : v; a29 = v;
    v = bias_t0[30] + x30 + -0.08226303133234722 * x54 + -0.014854938892497713 * x66;
    v = v < lo0[30] ? lo0[30] : v; v = v > hi0[30] ? hi0[30] : v; a30 = v;
    v = bias_t0[31] + x31 + -0.08226303133234722 * x54 + -0.6037269576533859 * x69;
    v = v < lo0[31] ? lo0[31] : v; v = v > hi0[31] ? hi0[31] : v; a31 = v;
    v = bias_t0[32] + x32 + -0.08226303133234722 * x54 + -0.021590409041307837 * x71;
    v = v < lo0[32] ? lo0[32] : v; v = v > hi0[32] ? hi0[32] : v; a32 = v;
    v = bias_t0[33] + x33 + -0.08226303133234722 * x54 + -0.011556395626565408 * x72;
    v = v < lo0[33] ? lo0[33] : v; v = v > hi0[33] ? hi0[33] : v; a33 = v;
    v = bias_t0[34] + x34 + -0.08226303133234722 * x55 + -0.3864320872326754 * x68;
    v = v < lo0[34] ? lo0[34] : v; v = v > hi0[34] ? hi0[34] : v; a34 = v;
    v = bias_t0[35] + x35 + -0.08226303133234722 * x55 + -0.2585411937087774 * x69;
    v = v < lo0[35] ? lo0[35] : v; v = v > hi0[35] ? hi0[35] : v; a35 = v;
    v = bias_t0[36] + x36 + -0.08226303133234722 * x55 + -0.41032977319887487 * x73;
    v = v < lo0[36] ? lo0[36] : v; v = v > hi0[36] ? hi0[36] : v; a36 = v;
    v = bias_t0[37] + x37 + -0.08226303133234722 * x56 + -0.023067940734351828 * x60;
    v = v < lo0[37] ? lo0[37] : v; v = v > hi0[37] ? hi0[37] : v; a37 = v;
    v = bias_t0[38] + x38 + -0.08226303133234722 * x56 + -0.018156664303604776 * x66;
    v = v < lo0[38] ? lo0[38] : v; v = v > hi0[38] ? hi0[38] : v; a38 = v;
    v = bias_t0[39] + x39 + -0.08226303133234722 * x57 + -0.37063781296154064 * x59;
    v = v < lo0[39] ? lo0[39] : v; v = v > hi0[39] ? hi0[39] : v; a39 = v;
    v = bias_t0[40] + x40 + -0.08226303133234722 * x57 + -0.13174930294003487 * x67;
    v = v < lo0[40] ? lo0[40] : v; v = v > hi0[40] ? hi0[40] : v; a40 = v;
    v = bias_t0[41] + x41 + -0.08226303133234722 * x57 + -0.14484430995648803 * x69;
    v = v < lo0[41] ? lo0[41] : v; v = v > hi0[41] ? hi0[41] : v; a41 = v;
    v = bias_t0[42] + x42 + -0.08226303133234722 * x57 + -0.03484606682994428 * x71;
    v = v < lo0[42] ? lo0[42] : v; v = v > hi0[42] ? hi0[42] : v; a42 = v;
    v = bias_t0[43] + x43 + -0.08226303133234722 * x57 + -0.09852027593353693 * x72;
    v = v < lo0[43] ? lo0[43] : v; v = v > hi0[43] ? hi0[43] : v; a43 = v;
    v = bias_t0[44] + x44 + -0.08226303133234722 * x58 + -0.03036587998612799 * x65;
    v = v < lo0[44] ? lo0[44] : v; v = v > hi0[44] ? hi0[44] : v; a44 = v;
    v = bias_t0[45] + x45 + -0.08226303133234722 * x58 + -0.14046939723165486 * x67;
    v = v < lo0[45] ? lo0[45] : v; v = v > hi0[45] ? hi0[45] : v; a45 = v;
    v = bias_t0[46] + x46 + -0.08226303133234722 * x58 + -0.3265308594327319 * x71;
    v = v < lo0[46] ? lo0[46] : v; v = v > hi0[46] ? hi0[46] : v; a46 = v;
    v = bias_t0[47] + x47 + -0.08226303133234722 * x58 + -0.17994004051645768 * x72;
    v = v < lo0[47] ? lo0[47] : v; v = v > hi0[47] ? hi0[47] : v; a47 = v;
    v = bias_t0[48] + x48 + -0.08226303133234722 * x58 + -0.22496888289546163 * x73;
    v = v < lo0[48] ? lo0[48] : v; v = v > hi0[48] ? hi0[48] : v; a48 = v;
    /* t_t0 coefficients are -2x the state coefficients on cols 0..48: share w = 2 t0 - s0. */
    w0 = a0 + a0 - x0;
    w1 = a1 + a1 - x1;
    w2 = a2 + a2 - x2;
    w3 = a3 + a3 - x3;
    w4 = a4 + a4 - x4;
    w5 = a5 + a5 - x5;
    w6 = a6 + a6 - x6;
    w7 = a7 + a7 - x7;
    w8 = a8 + a8 - x8;
    w9 = a9 + a9 - x9;
    w10 = a10 + a10 - x10;
    w11 = a11 + a11 - x11;
    w12 = a12 + a12 - x12;
    w13 = a13 + a13 - x13;
    w14 = a14 + a14 - x14;
    w15 = a15 + a15 - x15;
    w16 = a16 + a16 - x16;
    w17 = a17 + a17 - x17;
    w18 = a18 + a18 - x18;
    w19 = a19 + a19 - x19;
    w20 = a20 + a20 - x20;
    w21 = a21 + a21 - x21;
    w22 = a22 + a22 - x22;
    w23 = a23 + a23 - x23;
    w24 = a24 + a24 - x24;
    w25 = a25 + a25 - x25;
    w26 = a26 + a26 - x26;
    w27 = a27 + a27 - x27;
    w28 = a28 + a28 - x28;
    w29 = a29 + a29 - x29;
    w30 = a30 + a30 - x30;
    w31 = a31 + a31 - x31;
    w32 = a32 + a32 - x32;
    w33 = a33 + a33 - x33;
    w34 = a34 + a34 - x34;
    w35 = a35 + a35 - x35;
    w36 = a36 + a36 - x36;
    w37 = a37 + a37 - x37;
    w38 = a38 + a38 - x38;
    w39 = a39 + a39 - x39;
    w40 = a40 + a40 - x40;
    w41 = a41 + a41 - x41;
    w42 = a42 + a42 - x42;
    w43 = a43 + a43 - x43;
    w44 = a44 + a44 - x44;
    w45 = a45 + a45 - x45;
    w46 = a46 + a46 - x46;
    w47 = a47 + a47 - x47;
    w48 = a48 + a48 - x48;
    v = bias_t1[0] + 0.08226303133234722 * (((w0 + w1) + (w2 + w3)) + w4) + x49;
    v = v < lo1[0] ? lo1[0] : v; b0 = v;
    v = bias_t1[1] + 0.08226303133234722 * (((w5 + w6) + (w7 + w8)) + ((w9 + w10) + w11)) + x50;
    v = v < lo1[1] ? lo1[1] : v; b1 = v;
    v = bias_t1[2] + 0.08226303133234722 * ((w12 + w13) + (w14 + w15)) + x51;
    v = v < lo1[2] ? lo1[2] : v; b2 = v;
    v = bias_t1[3] + 0.08226303133234722 * (((w16 + w17) + (w18 + w19)) + ((w20 + w21) + w22)) + x52;
    v = v < lo1[3] ? lo1[3] : v; b3 = v;
    v = bias_t1[4] + 0.08226303133234722 * ((w23 + w24) + w25) + x53;
    v = v < lo1[4] ? lo1[4] : v; b4 = v;
    v = bias_t1[5] + 0.08226303133234722 * (((w26 + w27) + (w28 + w29)) + ((w30 + w31) + (w32 + w33))) + x54;
    v = v < lo1[5] ? lo1[5] : v; b5 = v;
    v = bias_t1[6] + 0.08226303133234722 * ((w34 + w35) + w36) + x55;
    v = v < lo1[6] ? lo1[6] : v; b6 = v;
    v = bias_t1[7] + 0.08226303133234722 * (w37 + w38) + x56;
    v = v < lo1[7] ? lo1[7] : v; b7 = v;
    v = bias_t1[8] + 0.08226303133234722 * (((w39 + w40) + (w41 + w42)) + w43) + x57;
    v = v < lo1[8] ? lo1[8] : v; b8 = v;
    v = bias_t1[9] + 0.08226303133234722 * (((w44 + w45) + (w46 + w47)) + w48) + x58;
    v = v < lo1[9] ? lo1[9] : v; b9 = v;
    new_state[0] = bias_su[0] + a0;
    new_state[1] = bias_su[1] + a1;
    new_state[2] = bias_su[2] + a2;
    new_state[3] = bias_su[3] + a3;
    new_state[4] = bias_su[4] + a4;
    new_state[5] = bias_su[5] + a5;
    new_state[6] = bias_su[6] + a6;
    new_state[7] = bias_su[7] + a7;
    new_state[8] = bias_su[8] + a8;
    new_state[9] = bias_su[9] + a9;
    new_state[10] = bias_su[10] + a10;
    new_state[11] = bias_su[11] + a11;
    new_state[12] = bias_su[12] + a12;
    new_state[13] = bias_su[13] + a13;
    new_state[14] = bias_su[14] + a14;
    new_state[15] = bias_su[15] + a15;
    new_state[16] = bias_su[16] + a16;
    new_state[17] = bias_su[17] + a17;
    new_state[18] = bias_su[18] + a18;
    new_state[19] = bias_su[19] + a19;
    new_state[20] = bias_su[20] + a20;
    new_state[21] = bias_su[21] + a21;
    new_state[22] = bias_su[22] + a22;
    new_state[23] = bias_su[23] + a23;
    new_state[24] = bias_su[24] + a24;
    new_state[25] = bias_su[25] + a25;
    new_state[26] = bias_su[26] + a26;
    new_state[27] = bias_su[27] + a27;
    new_state[28] = bias_su[28] + a28;
    new_state[29] = bias_su[29] + a29;
    new_state[30] = bias_su[30] + a30;
    new_state[31] = bias_su[31] + a31;
    new_state[32] = bias_su[32] + a32;
    new_state[33] = bias_su[33] + a33;
    new_state[34] = bias_su[34] + a34;
    new_state[35] = bias_su[35] + a35;
    new_state[36] = bias_su[36] + a36;
    new_state[37] = bias_su[37] + a37;
    new_state[38] = bias_su[38] + a38;
    new_state[39] = bias_su[39] + a39;
    new_state[40] = bias_su[40] + a40;
    new_state[41] = bias_su[41] + a41;
    new_state[42] = bias_su[42] + a42;
    new_state[43] = bias_su[43] + a43;
    new_state[44] = bias_su[44] + a44;
    new_state[45] = bias_su[45] + a45;
    new_state[46] = bias_su[46] + a46;
    new_state[47] = bias_su[47] + a47;
    new_state[48] = bias_su[48] + a48;
    new_state[49] = bias_su[49] + b0;
    new_state[50] = bias_su[50] + b1;
    new_state[51] = bias_su[51] + b2;
    new_state[52] = bias_su[52] + b3;
    new_state[53] = bias_su[53] + b4;
    new_state[54] = bias_su[54] + b5;
    new_state[55] = bias_su[55] + b6;
    new_state[56] = bias_su[56] + b7;
    new_state[57] = bias_su[57] + b8;
    new_state[58] = bias_su[58] + b9;
    new_state[59] = (bias_su[59] + x59) + (((0.0696550616260675 * w0 + 0.33760783556686547 * w12) + (0.2683650108072722 * w16 + 0.037081661631488835 * w26)) + 0.37063781296154064 * w39);
    new_state[60] = (bias_su[60] + x60) + ((0.2558824463341391 * w1 + 0.03758350075816168 * w5) + (0.08327630054464727 * w27 + 0.023067940734351828 * w37));
    new_state[61] = (bias_su[61] + x61) + (0.6175238612165264 * w6 + 0.04238843516524545 * w17);
    new_state[62] = (bias_su[62] + x62) + (0.13345673088354346 * w2 + 0.6113240882469336 * w23);
    new_state[63] = (bias_su[63] + x63) + ((0.04030704747187439 * w13 + 0.029151569883073398 * w18) + 0.5843595834842807 * w28);
    new_state[64] = (bias_su[64] + x64) + ((0.644834128427094 * w7 + 0.3601143643899065 * w19) + 0.05109107424493328 * w29);
    new_state[65] = (bias_su[65] + x65) + (((0.024964136788999065 * w3 + 0.037882339410371904 * w8) + (0.00976037797556846 * w14 + 0.2765836919316892 * w20)) + 0.03036587998612799 * w44);
    new_state[66] = (bias_su[66] + x66) + ((0.7947858773306499 * w9 + 0.014854938892497713 * w30) + 0.018156664303604776 * w38);
    new_state[67] = (bias_su[67] + x67) + (0.13174930294003487 * w40 + 0.14046939723165486 * w45);
    new_state[68] = (bias_su[68] + x68) + (0.013160484266981857 * w24 + 0.3864320872326754 * w34);
    new_state[69] = (bias_su[69] + x69) + ((0.01729459933480772 * w10 + 0.6037269576533859 * w31) + (0.2585411937087774 * w35 + 0.14484430995648803 * w41));
    new_state[70] = (bias_su[70] + x70) + (0.03473597091735419 * w21 + 0.15819624000738933 * w25);
    new_state[71] = (bias_su[71] + x71) + ((0.021966725812901995 * w4 + 0.021590409041307837 * w32) + (0.03484606682994428 * w42 + 0.3265308594327319 * w46));
    new_state[72] = (bias_su[72] + x72) + ((0.011556395626565408 * w33 + 0.09852027593353693 * w43) + 0.17994004051645768 * w47);
    new_state[73] = (bias_su[73] + x73) + (((0.08961892294526969 * w11 + 0.4683158478119333 * w15) + (0.022087418331416996 * w22 + 0.41032977319887487 * w36)) + 0.22496888289546163 * w48);
    t_t0[0] = a0;
    t_t0[1] = a1;
    t_t0[2] = a2;
    t_t0[3] = a3;
    t_t0[4] = a4;
    t_t0[5] = a5;
    t_t0[6] = a6;
    t_t0[7] = a7;
    t_t0[8] = a8;
    t_t0[9] = a9;
    t_t0[10] = a10;
    t_t0[11] = a11;
    t_t0[12] = a12;
    t_t0[13] = a13;
    t_t0[14] = a14;
    t_t0[15] = a15;
    t_t0[16] = a16;
    t_t0[17] = a17;
    t_t0[18] = a18;
    t_t0[19] = a19;
    t_t0[20] = a20;
    t_t0[21] = a21;
    t_t0[22] = a22;
    t_t0[23] = a23;
    t_t0[24] = a24;
    t_t0[25] = a25;
    t_t0[26] = a26;
    t_t0[27] = a27;
    t_t0[28] = a28;
    t_t0[29] = a29;
    t_t0[30] = a30;
    t_t0[31] = a31;
    t_t0[32] = a32;
    t_t0[33] = a33;
    t_t0[34] = a34;
    t_t0[35] = a35;
    t_t0[36] = a36;
    t_t0[37] = a37;
    t_t0[38] = a38;
    t_t0[39] = a39;
    t_t0[40] = a40;
    t_t0[41] = a41;
    t_t0[42] = a42;
    t_t0[43] = a43;
    t_t0[44] = a44;
    t_t0[45] = a45;
    t_t0[46] = a46;
    t_t0[47] = a47;
    t_t0[48] = a48;
    t_t1[0] = b0;
    t_t1[1] = b1;
    t_t1[2] = b2;
    t_t1[3] = b3;
    t_t1[4] = b4;
    t_t1[5] = b5;
    t_t1[6] = b6;
    t_t1[7] = b7;
    t_t1[8] = b8;
    t_t1[9] = b9;
}
