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

void fom_apply_t1_state__d_xi(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 10; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t1_state__d_xi_ROWPTR[i]; p < FOM_MAT_t1_state__d_xi_ROWPTR[i + 1]; ++p) s += FOM_MAT_t1_state__d_xi_VALS[p] * x[FOM_MAT_t1_state__d_xi_COLIDX[p]];
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

void fom_apply_t1_t_t0__d_xi(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 10; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t1_t_t0__d_xi_ROWPTR[i]; p < FOM_MAT_t1_t_t0__d_xi_ROWPTR[i + 1]; ++p) s += FOM_MAT_t1_t_t0__d_xi_VALS[p] * x[FOM_MAT_t1_t_t0__d_xi_COLIDX[p]];
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

void fom_apply_su_state__d_xi(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 74; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_su_state__d_xi_ROWPTR[i]; p < FOM_MAT_su_state__d_xi_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_state__d_xi_VALS[p] * x[FOM_MAT_su_state__d_xi_COLIDX[p]];
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

void fom_apply_su_t_t0__d_xi(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 74; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_su_t_t0__d_xi_ROWPTR[i]; p < FOM_MAT_su_t_t0__d_xi_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t0__d_xi_VALS[p] * x[FOM_MAT_su_t_t0__d_xi_COLIDX[p]];
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

void fom_fused_t1(const fom_float *state, const fom_float *t_t0, fom_float sc_xi, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 10; ++i) {
        fom_float s;
        s = 0.0;
        for (p = FOM_MAT_t1_state_ROWPTR[i]; p < FOM_MAT_t1_state_ROWPTR[i + 1]; ++p) s += FOM_MAT_t1_state_VALS[p] * state[FOM_MAT_t1_state_COLIDX[p]];
        out[i] += s;
        s = 0.0;
        for (p = FOM_MAT_t1_state__d_xi_ROWPTR[i]; p < FOM_MAT_t1_state__d_xi_ROWPTR[i + 1]; ++p) s += FOM_MAT_t1_state__d_xi_VALS[p] * state[FOM_MAT_t1_state__d_xi_COLIDX[p]];
        out[i] += sc_xi * s;
        s = 0.0;
        for (p = FOM_MAT_t1_t_t0_ROWPTR[i]; p < FOM_MAT_t1_t_t0_ROWPTR[i + 1]; ++p) s += FOM_MAT_t1_t_t0_VALS[p] * t_t0[FOM_MAT_t1_t_t0_COLIDX[p]];
        out[i] += s;
        s = 0.0;
        for (p = FOM_MAT_t1_t_t0__d_xi_ROWPTR[i]; p < FOM_MAT_t1_t_t0__d_xi_ROWPTR[i + 1]; ++p) s += FOM_MAT_t1_t_t0__d_xi_VALS[p] * t_t0[FOM_MAT_t1_t_t0__d_xi_COLIDX[p]];
        out[i] += sc_xi * s;
    }
}

void fom_fused_su(const fom_float *state, const fom_float *t_t0, const fom_float *t_t1, fom_float sc_xi, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 74; ++i) {
        fom_float s;
        s = 0.0;
        for (p = FOM_MAT_su_state_ROWPTR[i]; p < FOM_MAT_su_state_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_state_VALS[p] * state[FOM_MAT_su_state_COLIDX[p]];
        out[i] += s;
        s = 0.0;
        for (p = FOM_MAT_su_state__d_xi_ROWPTR[i]; p < FOM_MAT_su_state__d_xi_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_state__d_xi_VALS[p] * state[FOM_MAT_su_state__d_xi_COLIDX[p]];
        out[i] += sc_xi * s;
        s = 0.0;
        for (p = FOM_MAT_su_t_t0_ROWPTR[i]; p < FOM_MAT_su_t_t0_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t0_VALS[p] * t_t0[FOM_MAT_su_t_t0_COLIDX[p]];
        out[i] += s;
        s = 0.0;
        for (p = FOM_MAT_su_t_t0__d_xi_ROWPTR[i]; p < FOM_MAT_su_t_t0__d_xi_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t0__d_xi_VALS[p] * t_t0[FOM_MAT_su_t_t0__d_xi_COLIDX[p]];
        out[i] += sc_xi * s;
        s = 0.0;
        for (p = FOM_MAT_su_t_t1_ROWPTR[i]; p < FOM_MAT_su_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t1_VALS[p] * t_t1[FOM_MAT_su_t_t1_COLIDX[p]];
        out[i] += s;
    }
}

void fom_step_t0(const fom_float *state, const fom_float *bias, const FomAct_t0 *act, fom_float *out) {
    const fom_float *lo = act->lo, *hi = act->hi;
    fom_float v;
    const fom_float h0 = 0.08226303133234722*state[49], h1 = 0.08226303133234722*state[50], h2 = 0.08226303133234722*state[51], h3 = 0.08226303133234722*state[52], h4 = 0.08226303133234722*state[53], h5 = 0.08226303133234722*state[54], h6 = 0.08226303133234722*state[55], h7 = 0.08226303133234722*state[56], h8 = 0.08226303133234722*state[57], h9 = 0.08226303133234722*state[58];
    v = bias[0] + (state[0] - h0 - 0.0696550616260675*state[59]); v = v < lo[0] ? lo[0] : v; out[0] = v > hi[0] ? hi[0] : v;
    v = bias[1] + (state[1] - h0 - 0.2558824463341391*state[60]); v = v < lo[1] ? lo[1] : v; out[1] = v > hi[1] ? hi[1] : v;
    v = bias[2] + (state[2] - h0 - 0.13345673088354346*state[62]); v = v < lo[2] ? lo[2] : v; out[2] = v > hi[2] ? hi[2] : v;
    v = bias[3] + (state[3] - h0 - 0.024964136788999065*state[65]); v = v < lo[3] ? lo[3] : v; out[3] = v > hi[3] ? hi[3] : v;
    v = bias[4] + (state[4] - h0 - 0.021966725812901995*state[71]); v = v < lo[4] ? lo[4] : v; out[4] = v > hi[4] ? hi[4] : v;
    v = bias[5] + (state[5] - h1 - 0.03758350075816168*state[60]); v = v < lo[5] ? lo[5] : v; out[5] = v > hi[5] ? hi[5] : v;
    v = bias[6] + (state[6] - h1 - 0.6175238612165264*state[61]); v = v < lo[6] ? lo[6] : v; out[6] = v > hi[6] ? hi[6] : v;
    v = bias[7] + (state[7] - h1 - 0.644834128427094*state[64]); v = v < lo[7] ? lo[7] : v; out[7] = v > hi[7] ? hi[7] : v;
    v = bias[8] + (state[8] - h1 - 0.037882339410371904*state[65]); v = v < lo[8] ? lo[8] : v; out[8] = v > hi[8] ? hi[8] : v;
    v = bias[9] + (state[9] - h1 - 0.7947858773306499*state[66]); v = v < lo[9] ? lo[9] : v; out[9] = v > hi[9] ? hi[9] : v;
    v = bias[10] + (state[10] - h1 - 0.01729459933480772*state[69]); v = v < lo[10] ? lo[10] : v; out[10] = v > hi[10] ? hi[10] : v;
    v = bias[11] + (state[11] - h1 - 0.08961892294526969*state[73]); v = v < lo[11] ? lo[11] : v; out[11] = v > hi[11] ? hi[11] : v;
    v = bias[12] + (state[12] - h2 - 0.33760783556686547*state[59]); v = v < lo[12] ? lo[12] : v; out[12] = v > hi[12] ? hi[12] : v;
    v = bias[13] + (state[13] - h2 - 0.04030704747187439*state[63]); v = v < lo[13] ? lo[13] : v; out[13] = v > hi[13] ? hi[13] : v;
    v = bias[14] + (state[14] - h2 - 0.00976037797556846*state[65]); v = v < lo[14] ? lo[14] : v; out[14] = v > hi[14] ? hi[14] : v;
    v = bias[15] + (state[15] - h2 - 0.4683158478119333*state[73]); v = v < lo[15] ? lo[15] : v; out[15] = v > hi[15] ? hi[15] : v;
    v = bias[16] + (state[16] - h3 - 0.2683650108072722*state[59]); v = v < lo[16] ? lo[16] : v; out[16] = v > hi[16] ? hi[16] : v;
    v = bias[17] + (state[17] - h3 - 0.04238843516524545*state[61]); v = v < lo[17] ? lo[17] : v; out[17] = v > hi[17] ? hi[17] : v;
    v = bias[18] + (state[18] - h3 - 0.029151569883073398*state[63]); v = v < lo[18] ? lo[18] : v; out[18] = v > hi[18] ? hi[18] : v;
    v = bias[19] + (state[19] - h3 - 0.3601143643899065*state[64]); v = v < lo[19] ? lo[19] : v; out[19] = v > hi[19] ? hi[19] : v;
    v = bias[20] + (state[20] - h3 - 0.2765836919316892*state[65]); v = v < lo[20] ? lo[20] : v; out[20] = v > hi[20] ? hi[20] : v;
    v = bias[21] + (state[21] - h3 - 0.03473597091735419*state[70]); v = v < lo[21] ? lo[21] : v; out[21] = v > hi[21] ? hi[21] : v;
    v = bias[22] + (state[22] - h3 - 0.022087418331416996*state[73]); v = v < lo[22] ? lo[22] : v; out[22] = v > hi[22] ? hi[22] : v;
    v = bias[23] + (state[23] - h4 - 0.6113240882469336*state[62]); v = v < lo[23] ? lo[23] : v; out[23] = v > hi[23] ? hi[23] : v;
    v = bias[24] + (state[24] - h4 - 0.013160484266981857*state[68]); v = v < lo[24] ? lo[24] : v; out[24] = v > hi[24] ? hi[24] : v;
    v = bias[25] + (state[25] - h4 - 0.15819624000738933*state[70]); v = v < lo[25] ? lo[25] : v; out[25] = v > hi[25] ? hi[25] : v;
    v = bias[26] + (state[26] - h5 - 0.037081661631488835*state[59]); v = v < lo[26] ? lo[26] : v; out[26] = v > hi[26] ? hi[26] : v;
    v = bias[27] + (state[27] - h5 - 0.08327630054464727*state[60]); v = v < lo[27] ? lo[27] : v; out[27] = v > hi[27] ? hi[27] : v;
    v = bias[28] + (state[28] - h5 - 0.5843595834842807*state[63]); v = v < lo[28] ? lo[28] : v; out[28] = v > hi[28] ? hi[28] : v;
    v = bias[29] + (state[29] - h5 - 0.05109107424493328*state[64]); v = v < lo[29] ? lo[29] : v; out[29] = v > hi[29] ? hi[29] : v;
    v = bias[30] + (state[30] - h5 - 0.014854938892497713*state[66]); v = v < lo[30] ? lo[30] : v; out[30] = v > hi[30] ? hi[30] : v;
    v = bias[31] + (state[31] - h5 - 0.6037269576533859*state[69]); v = v < lo[31] ? lo[31] : v; out[31] = v > hi[31] ? hi[31] : v;
    v = bias[32] + (state[32] - h5 - 0.021590409041307837*state[71]); v = v < lo[32] ? lo[32] : v; out[32] = v > hi[32] ? hi[32] : v;
    v = bias[33] + (state[33] - h5 - 0.011556395626565408*state[72]); v = v < lo[33] ? lo[33] : v; out[33] = v > hi[33] ? hi[33] : v;
    v = bias[34] + (state[34] - h6 - 0.3864320872326754*state[68]); v = v < lo[34] ? lo[34] : v; out[34] = v > hi[34] ? hi[34] : v;
    v = bias[35] + (state[35] - h6 - 0.2585411937087774*state[69]); v = v < lo[35] ? lo[35] : v; out[35] = v > hi[35] ? hi[35] : v;
    v = bias[36] + (state[36] - h6 - 0.41032977319887487*state[73]); v = v < lo[36] ? lo[36] : v; out[36] = v > hi[36] ? hi[36] : v;
    v = bias[37] + (state[37] - h7 - 0.023067940734351828*state[60]); v = v < lo[37] ? lo[37] : v; out[37] = v > hi[37] ? hi[37] : v;
    v = bias[38] + (state[38] - h7 - 0.018156664303604776*state[66]); v = v < lo[38] ? lo[38] : v; out[38] = v > hi[38] ? hi[38] : v;
    v = bias[39] + (state[39] - h8 - 0.37063781296154064*state[59]); v = v < lo[39] ? lo[39] : v; out[39] = v > hi[39] ? hi[39] : v;
    v = bias[40] + (state[40] - h8 - 0.13174930294003487*state[67]); v = v < lo[40] ? lo[40] : v; out[40] = v > hi[40] ? hi[40] : v;
    v = bias[41] + (state[41] - h8 - 0.14484430995648803*state[69]); v = v < lo[41] ? lo[41] : v; out[41] = v > hi[41] ? hi[41] : v;
    v = bias[42] + (state[42] - h8 - 0.03484606682994428*state[71]); v = v < lo[42] ? lo[42] : v; out[42] = v > hi[42] ? hi[42] : v;
    v = bias[43] + (state[43] - h8 - 0.09852027593353693*state[72]); v = v < lo[43] ? lo[43] : v; out[43] = v > hi[43] ? hi[43] : v;
    v = bias[44] + (state[44] - h9 - 0.03036587998612799*state[65]); v = v < lo[44] ? lo[44] : v; out[44] = v > hi[44] ? hi[44] : v;
    v = bias[45] + (state[45] - h9 - 0.14046939723165486*state[67]); v = v < lo[45] ? lo[45] : v; out[45] = v > hi[45] ? hi[45] : v;
    v = bias[46] + (state[46] - h9 - 0.3265308594327319*state[71]); v = v < lo[46] ? lo[46] : v; out[46] = v > hi[46] ? hi[46] : v;
    v = bias[47] + (state[47] - h9 - 0.17994004051645768*state[72]); v = v < lo[47] ? lo[47] : v; out[47] = v > hi[47] ? hi[47] : v;
    v = bias[48] + (state[48] - h9 - 0.22496888289546163*state[73]); v = v < lo[48] ? lo[48] : v; out[48] = v > hi[48] ? hi[48] : v;
}

void fom_step_t1(const fom_float *state, const fom_float *t_t0, fom_float sc_xi, const fom_float *bias, const FomAct_t1 *act, fom_float *out) {
    /* Each row has one uniform coefficient on its state group and one on its
       t_t0 group (both members), so factor them out of the group sums. */
    static const fom_int g[11] = {0, 5, 12, 16, 23, 26, 34, 37, 39, 44, 49};
    const fom_float *lo = act->lo;
    const fom_float cs = -0.08226303133234725 + sc_xi * -0.16452606266469433;
    const fom_float ct = 0.16452606266469444 + sc_xi * 0.16452606266469444;
    fom_int r, k;
    for (r = 0; r < 10; ++r) {
        fom_float ss = 0.0, st = 0.0, v;
        for (k = g[r]; k < g[r + 1]; ++k) { ss += state[k]; st += t_t0[k]; }
        v = bias[r] + state[49 + r] + cs * ss + ct * st;
        out[r] = v < lo[r] ? lo[r] : v;
    }
}

void fom_step_su(const fom_float *state, const fom_float *t_t0, const fom_float *t_t1, fom_float sc_xi, const fom_float *bias, fom_float *out) {
    fom_int i, r, p;
    for (r = 0; r < 74; ++r) {
        fom_float s, v;
        i = r;
        v = bias[i];
        s = 0.0;
        for (p = FOM_MAT_su_state_ROWPTR[i]; p < FOM_MAT_su_state_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_state_VALS[p] * state[FOM_MAT_su_state_COLIDX[p]];
        v += s;
        s = 0.0;
        for (p = FOM_MAT_su_state__d_xi_ROWPTR[i]; p < FOM_MAT_su_state__d_xi_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_state__d_xi_VALS[p] * state[FOM_MAT_su_state__d_xi_COLIDX[p]];
        v += sc_xi * s;
        s = 0.0;
        for (p = FOM_MAT_su_t_t0_ROWPTR[i]; p < FOM_MAT_su_t_t0_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t0_VALS[p] * t_t0[FOM_MAT_su_t_t0_COLIDX[p]];
        v += s;
        s = 0.0;
        for (p = FOM_MAT_su_t_t0__d_xi_ROWPTR[i]; p < FOM_MAT_su_t_t0__d_xi_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t0__d_xi_VALS[p] * t_t0[FOM_MAT_su_t_t0__d_xi_COLIDX[p]];
        v += sc_xi * s;
        s = 0.0;
        for (p = FOM_MAT_su_t_t1_ROWPTR[i]; p < FOM_MAT_su_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t1_VALS[p] * t_t1[FOM_MAT_su_t_t1_COLIDX[p]];
        v += s;
        out[i] = v;
    }
}

void fom_fused_su__rest(const fom_float *state, const fom_float *t_t0, const fom_float *t_t1, fom_float sc_xi, fom_float *out) {
    fom_int i, r, p;
    for (r = 0; r < FOM_REST_su_N; ++r) {
        fom_float s;
        i = FOM_REST_su_ROWS[r];
        s = 0.0;
        for (p = FOM_MAT_su_state_ROWPTR[i]; p < FOM_MAT_su_state_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_state_VALS[p] * state[FOM_MAT_su_state_COLIDX[p]];
        out[i] += s;
        s = 0.0;
        for (p = FOM_MAT_su_state__d_xi_ROWPTR[i]; p < FOM_MAT_su_state__d_xi_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_state__d_xi_VALS[p] * state[FOM_MAT_su_state__d_xi_COLIDX[p]];
        out[i] += sc_xi * s;
        s = 0.0;
        for (p = FOM_MAT_su_t_t0_ROWPTR[i]; p < FOM_MAT_su_t_t0_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t0_VALS[p] * t_t0[FOM_MAT_su_t_t0_COLIDX[p]];
        out[i] += s;
        s = 0.0;
        for (p = FOM_MAT_su_t_t0__d_xi_ROWPTR[i]; p < FOM_MAT_su_t_t0__d_xi_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t0__d_xi_VALS[p] * t_t0[FOM_MAT_su_t_t0__d_xi_COLIDX[p]];
        out[i] += sc_xi * s;
        s = 0.0;
        for (p = FOM_MAT_su_t_t1_ROWPTR[i]; p < FOM_MAT_su_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t1_VALS[p] * t_t1[FOM_MAT_su_t_t1_COLIDX[p]];
        out[i] += s;
    }
}

void fom_step_su__rest(const fom_float *state, const fom_float *t_t0, const fom_float *t_t1, fom_float sc_xi, const fom_float *bias, fom_float *out) {
    /* Every member coefficient of rows 59..73 is a multiple of one per-entry
       weight a_k (state: -a, t_t0: 2a, xi*state: -2a, xi*t_t0: 2a), and each
       t_t0 entry feeds exactly one row: form d_k = (2+2xi) t_k - (1+2xi) s_k once. */
    const fom_float c1 = 1.0 + 2.0 * sc_xi, c2 = 2.0 + 2.0 * sc_xi;
    fom_float d[49];
    fom_int k;
    for (k = 0; k < 49; ++k) d[k] = c2 * t_t0[k] - c1 * state[k];
    out[59] = bias[59] + state[59] + (0.06965506162606752*d[0] + 0.3376078355668656*d[12] + 0.2683650108072723*d[16] + 0.03708166163148883*d[26] + 0.37063781296154064*d[39]);
    out[60] = bias[60] + state[60] + (0.25588244633413904*d[1] + 0.03758350075816168*d[5] + 0.08327630054464728*d[27] + 0.023067940734351824*d[37]);
    out[61] = bias[61] + state[61] + (0.6175238612165267*d[6] + 0.04238843516524547*d[17]);
    out[62] = bias[62] + state[62] + (0.13345673088354354*d[2] + 0.611324088246934*d[23]);
    out[63] = bias[63] + state[63] + (0.040307047471874385*d[13] + 0.029151569883073405*d[18] + 0.5843595834842807*d[28]);
    out[64] = bias[64] + state[64] + (0.6448341284270941*d[7] + 0.36011436438990646*d[19] + 0.05109107424493329*d[29]);
    out[65] = bias[65] + state[65] + (0.02496413678899907*d[3] + 0.03788233941037191*d[8] + 0.00976037797556846*d[14] + 0.2765836919316893*d[20] + 0.030365879986127997*d[44]);
    out[66] = bias[66] + state[66] + (0.7947858773306501*d[9] + 0.014854938892497714*d[30] + 0.018156664303604783*d[38]);
    out[67] = bias[67] + state[67] + (0.131749302940035*d[40] + 0.14046939723165494*d[45]);
    out[68] = bias[68] + state[68] + (0.013160484266981862*d[24] + 0.38643208723267564*d[34]);
    out[69] = bias[69] + state[69] + (0.017294599334807734*d[10] + 0.6037269576533858*d[31] + 0.2585411937087774*d[35] + 0.14484430995648817*d[41]);
    out[70] = bias[70] + state[70] + (0.034735970917354184*d[21] + 0.15819624000738947*d[25]);
    out[71] = bias[71] + state[71] + (0.021966725812902*d[4] + 0.02159040904130785*d[32] + 0.03484606682994429*d[42] + 0.3265308594327321*d[46]);
    out[72] = bias[72] + state[72] + (0.01155639562656541*d[33] + 0.09852027593353693*d[43] + 0.17994004051645768*d[47]);
    out[73] = bias[73] + state[73] + (0.08961892294526974*d[11] + 0.46831584781193325*d[15] + 0.022087418331416996*d[22] + 0.41032977319887487*d[36] + 0.22496888289546174*d[48]);

}

void fom_iterate(const fom_float *state, fom_float sc_xi, const fom_float *bias_t0, const fom_float *bias_t1, const fom_float *bias_su, const FomAct_t0 *act_t0, const FomAct_t1 *act_t1, fom_float *t_t0, fom_float *t_t1, fom_float *new_state) {
    fom_step_t0(state, bias_t0, act_t0, t_t0);
    fom_step_t1(state, t_t0, sc_xi, bias_t1, act_t1, t_t1);
    fom_step_su(state, t_t0, t_t1, sc_xi, bias_su, new_state);
}
