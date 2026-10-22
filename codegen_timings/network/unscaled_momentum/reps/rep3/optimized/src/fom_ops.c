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

void fom_step_t0(const fom_float *restrict state, const fom_float *restrict bias, const FomAct_t0 *restrict act, fom_float *restrict out) {
    /* The state[49..58] coefficient is shared by every row of a group. */
    const fom_float g49 = 0.08226303133234722 * state[49];
    const fom_float g50 = 0.08226303133234722 * state[50];
    const fom_float g51 = 0.08226303133234722 * state[51];
    const fom_float g52 = 0.08226303133234722 * state[52];
    const fom_float g53 = 0.08226303133234722 * state[53];
    const fom_float g54 = 0.08226303133234722 * state[54];
    const fom_float g55 = 0.08226303133234722 * state[55];
    const fom_float g56 = 0.08226303133234722 * state[56];
    const fom_float g57 = 0.08226303133234722 * state[57];
    const fom_float g58 = 0.08226303133234722 * state[58];
    { fom_float v = (bias[0] + state[0]) - (g49 + 0.0696550616260675 * state[59]);
      if (v < act->lo[0]) v = act->lo[0];
      if (v > act->hi[0]) v = act->hi[0];
      out[0] = v; }
    { fom_float v = (bias[1] + state[1]) - (g49 + 0.2558824463341391 * state[60]);
      if (v < act->lo[1]) v = act->lo[1];
      if (v > act->hi[1]) v = act->hi[1];
      out[1] = v; }
    { fom_float v = (bias[2] + state[2]) - (g49 + 0.13345673088354346 * state[62]);
      if (v < act->lo[2]) v = act->lo[2];
      if (v > act->hi[2]) v = act->hi[2];
      out[2] = v; }
    { fom_float v = (bias[3] + state[3]) - (g49 + 0.024964136788999065 * state[65]);
      if (v < act->lo[3]) v = act->lo[3];
      if (v > act->hi[3]) v = act->hi[3];
      out[3] = v; }
    { fom_float v = (bias[4] + state[4]) - (g49 + 0.021966725812901995 * state[71]);
      if (v < act->lo[4]) v = act->lo[4];
      if (v > act->hi[4]) v = act->hi[4];
      out[4] = v; }
    { fom_float v = (bias[5] + state[5]) - (g50 + 0.03758350075816168 * state[60]);
      if (v < act->lo[5]) v = act->lo[5];
      if (v > act->hi[5]) v = act->hi[5];
      out[5] = v; }
    { fom_float v = (bias[6] + state[6]) - (g50 + 0.6175238612165264 * state[61]);
      if (v < act->lo[6]) v = act->lo[6];
      if (v > act->hi[6]) v = act->hi[6];
      out[6] = v; }
    { fom_float v = (bias[7] + state[7]) - (g50 + 0.644834128427094 * state[64]);
      if (v < act->lo[7]) v = act->lo[7];
      if (v > act->hi[7]) v = act->hi[7];
      out[7] = v; }
    { fom_float v = (bias[8] + state[8]) - (g50 + 0.037882339410371904 * state[65]);
      if (v < act->lo[8]) v = act->lo[8];
      if (v > act->hi[8]) v = act->hi[8];
      out[8] = v; }
    { fom_float v = (bias[9] + state[9]) - (g50 + 0.7947858773306499 * state[66]);
      if (v < act->lo[9]) v = act->lo[9];
      if (v > act->hi[9]) v = act->hi[9];
      out[9] = v; }
    { fom_float v = (bias[10] + state[10]) - (g50 + 0.01729459933480772 * state[69]);
      if (v < act->lo[10]) v = act->lo[10];
      if (v > act->hi[10]) v = act->hi[10];
      out[10] = v; }
    { fom_float v = (bias[11] + state[11]) - (g50 + 0.08961892294526969 * state[73]);
      if (v < act->lo[11]) v = act->lo[11];
      if (v > act->hi[11]) v = act->hi[11];
      out[11] = v; }
    { fom_float v = (bias[12] + state[12]) - (g51 + 0.33760783556686547 * state[59]);
      if (v < act->lo[12]) v = act->lo[12];
      if (v > act->hi[12]) v = act->hi[12];
      out[12] = v; }
    { fom_float v = (bias[13] + state[13]) - (g51 + 0.04030704747187439 * state[63]);
      if (v < act->lo[13]) v = act->lo[13];
      if (v > act->hi[13]) v = act->hi[13];
      out[13] = v; }
    { fom_float v = (bias[14] + state[14]) - (g51 + 0.00976037797556846 * state[65]);
      if (v < act->lo[14]) v = act->lo[14];
      if (v > act->hi[14]) v = act->hi[14];
      out[14] = v; }
    { fom_float v = (bias[15] + state[15]) - (g51 + 0.4683158478119333 * state[73]);
      if (v < act->lo[15]) v = act->lo[15];
      if (v > act->hi[15]) v = act->hi[15];
      out[15] = v; }
    { fom_float v = (bias[16] + state[16]) - (g52 + 0.2683650108072722 * state[59]);
      if (v < act->lo[16]) v = act->lo[16];
      if (v > act->hi[16]) v = act->hi[16];
      out[16] = v; }
    { fom_float v = (bias[17] + state[17]) - (g52 + 0.04238843516524545 * state[61]);
      if (v < act->lo[17]) v = act->lo[17];
      if (v > act->hi[17]) v = act->hi[17];
      out[17] = v; }
    { fom_float v = (bias[18] + state[18]) - (g52 + 0.029151569883073398 * state[63]);
      if (v < act->lo[18]) v = act->lo[18];
      if (v > act->hi[18]) v = act->hi[18];
      out[18] = v; }
    { fom_float v = (bias[19] + state[19]) - (g52 + 0.3601143643899065 * state[64]);
      if (v < act->lo[19]) v = act->lo[19];
      if (v > act->hi[19]) v = act->hi[19];
      out[19] = v; }
    { fom_float v = (bias[20] + state[20]) - (g52 + 0.2765836919316892 * state[65]);
      if (v < act->lo[20]) v = act->lo[20];
      if (v > act->hi[20]) v = act->hi[20];
      out[20] = v; }
    { fom_float v = (bias[21] + state[21]) - (g52 + 0.03473597091735419 * state[70]);
      if (v < act->lo[21]) v = act->lo[21];
      if (v > act->hi[21]) v = act->hi[21];
      out[21] = v; }
    { fom_float v = (bias[22] + state[22]) - (g52 + 0.022087418331416996 * state[73]);
      if (v < act->lo[22]) v = act->lo[22];
      if (v > act->hi[22]) v = act->hi[22];
      out[22] = v; }
    { fom_float v = (bias[23] + state[23]) - (g53 + 0.6113240882469336 * state[62]);
      if (v < act->lo[23]) v = act->lo[23];
      if (v > act->hi[23]) v = act->hi[23];
      out[23] = v; }
    { fom_float v = (bias[24] + state[24]) - (g53 + 0.013160484266981857 * state[68]);
      if (v < act->lo[24]) v = act->lo[24];
      if (v > act->hi[24]) v = act->hi[24];
      out[24] = v; }
    { fom_float v = (bias[25] + state[25]) - (g53 + 0.15819624000738933 * state[70]);
      if (v < act->lo[25]) v = act->lo[25];
      if (v > act->hi[25]) v = act->hi[25];
      out[25] = v; }
    { fom_float v = (bias[26] + state[26]) - (g54 + 0.037081661631488835 * state[59]);
      if (v < act->lo[26]) v = act->lo[26];
      if (v > act->hi[26]) v = act->hi[26];
      out[26] = v; }
    { fom_float v = (bias[27] + state[27]) - (g54 + 0.08327630054464727 * state[60]);
      if (v < act->lo[27]) v = act->lo[27];
      if (v > act->hi[27]) v = act->hi[27];
      out[27] = v; }
    { fom_float v = (bias[28] + state[28]) - (g54 + 0.5843595834842807 * state[63]);
      if (v < act->lo[28]) v = act->lo[28];
      if (v > act->hi[28]) v = act->hi[28];
      out[28] = v; }
    { fom_float v = (bias[29] + state[29]) - (g54 + 0.05109107424493328 * state[64]);
      if (v < act->lo[29]) v = act->lo[29];
      if (v > act->hi[29]) v = act->hi[29];
      out[29] = v; }
    { fom_float v = (bias[30] + state[30]) - (g54 + 0.014854938892497713 * state[66]);
      if (v < act->lo[30]) v = act->lo[30];
      if (v > act->hi[30]) v = act->hi[30];
      out[30] = v; }
    { fom_float v = (bias[31] + state[31]) - (g54 + 0.6037269576533859 * state[69]);
      if (v < act->lo[31]) v = act->lo[31];
      if (v > act->hi[31]) v = act->hi[31];
      out[31] = v; }
    { fom_float v = (bias[32] + state[32]) - (g54 + 0.021590409041307837 * state[71]);
      if (v < act->lo[32]) v = act->lo[32];
      if (v > act->hi[32]) v = act->hi[32];
      out[32] = v; }
    { fom_float v = (bias[33] + state[33]) - (g54 + 0.011556395626565408 * state[72]);
      if (v < act->lo[33]) v = act->lo[33];
      if (v > act->hi[33]) v = act->hi[33];
      out[33] = v; }
    { fom_float v = (bias[34] + state[34]) - (g55 + 0.3864320872326754 * state[68]);
      if (v < act->lo[34]) v = act->lo[34];
      if (v > act->hi[34]) v = act->hi[34];
      out[34] = v; }
    { fom_float v = (bias[35] + state[35]) - (g55 + 0.2585411937087774 * state[69]);
      if (v < act->lo[35]) v = act->lo[35];
      if (v > act->hi[35]) v = act->hi[35];
      out[35] = v; }
    { fom_float v = (bias[36] + state[36]) - (g55 + 0.41032977319887487 * state[73]);
      if (v < act->lo[36]) v = act->lo[36];
      if (v > act->hi[36]) v = act->hi[36];
      out[36] = v; }
    { fom_float v = (bias[37] + state[37]) - (g56 + 0.023067940734351828 * state[60]);
      if (v < act->lo[37]) v = act->lo[37];
      if (v > act->hi[37]) v = act->hi[37];
      out[37] = v; }
    { fom_float v = (bias[38] + state[38]) - (g56 + 0.018156664303604776 * state[66]);
      if (v < act->lo[38]) v = act->lo[38];
      if (v > act->hi[38]) v = act->hi[38];
      out[38] = v; }
    { fom_float v = (bias[39] + state[39]) - (g57 + 0.37063781296154064 * state[59]);
      if (v < act->lo[39]) v = act->lo[39];
      if (v > act->hi[39]) v = act->hi[39];
      out[39] = v; }
    { fom_float v = (bias[40] + state[40]) - (g57 + 0.13174930294003487 * state[67]);
      if (v < act->lo[40]) v = act->lo[40];
      if (v > act->hi[40]) v = act->hi[40];
      out[40] = v; }
    { fom_float v = (bias[41] + state[41]) - (g57 + 0.14484430995648803 * state[69]);
      if (v < act->lo[41]) v = act->lo[41];
      if (v > act->hi[41]) v = act->hi[41];
      out[41] = v; }
    { fom_float v = (bias[42] + state[42]) - (g57 + 0.03484606682994428 * state[71]);
      if (v < act->lo[42]) v = act->lo[42];
      if (v > act->hi[42]) v = act->hi[42];
      out[42] = v; }
    { fom_float v = (bias[43] + state[43]) - (g57 + 0.09852027593353693 * state[72]);
      if (v < act->lo[43]) v = act->lo[43];
      if (v > act->hi[43]) v = act->hi[43];
      out[43] = v; }
    { fom_float v = (bias[44] + state[44]) - (g58 + 0.03036587998612799 * state[65]);
      if (v < act->lo[44]) v = act->lo[44];
      if (v > act->hi[44]) v = act->hi[44];
      out[44] = v; }
    { fom_float v = (bias[45] + state[45]) - (g58 + 0.14046939723165486 * state[67]);
      if (v < act->lo[45]) v = act->lo[45];
      if (v > act->hi[45]) v = act->hi[45];
      out[45] = v; }
    { fom_float v = (bias[46] + state[46]) - (g58 + 0.3265308594327319 * state[71]);
      if (v < act->lo[46]) v = act->lo[46];
      if (v > act->hi[46]) v = act->hi[46];
      out[46] = v; }
    { fom_float v = (bias[47] + state[47]) - (g58 + 0.17994004051645768 * state[72]);
      if (v < act->lo[47]) v = act->lo[47];
      if (v > act->hi[47]) v = act->hi[47];
      out[47] = v; }
    { fom_float v = (bias[48] + state[48]) - (g58 + 0.22496888289546163 * state[73]);
      if (v < act->lo[48]) v = act->lo[48];
      if (v > act->hi[48]) v = act->hi[48];
      out[48] = v; }
}

void fom_step_t1(const fom_float *restrict state, const fom_float *restrict t_t0, fom_float sc_xi, const fom_float *restrict bias, const FomAct_t1 *restrict act, fom_float *restrict out) {
    /* Each row couples one contiguous group of state/t0 entries with
       shared coefficients: sum the group (as a tree), scale once. */
    const fom_float ca = sc_xi * -0.16452606266469433 - 0.08226303133234725;
    const fom_float cb = 0.16452606266469444 + sc_xi * 0.16452606266469444;
    { fom_float v = (bias[0] + state[49]) + (ca * (((state[0] + state[1]) + (state[2] + state[3])) + state[4]) + cb * (((t_t0[0] + t_t0[1]) + (t_t0[2] + t_t0[3])) + t_t0[4]));
      if (v < act->lo[0]) v = act->lo[0];
      out[0] = v; }
    { fom_float v = (bias[1] + state[50]) + (ca * (((state[5] + state[6]) + (state[7] + state[8])) + ((state[9] + state[10]) + state[11])) + cb * (((t_t0[5] + t_t0[6]) + (t_t0[7] + t_t0[8])) + ((t_t0[9] + t_t0[10]) + t_t0[11])));
      if (v < act->lo[1]) v = act->lo[1];
      out[1] = v; }
    { fom_float v = (bias[2] + state[51]) + (ca * ((state[12] + state[13]) + (state[14] + state[15])) + cb * ((t_t0[12] + t_t0[13]) + (t_t0[14] + t_t0[15])));
      if (v < act->lo[2]) v = act->lo[2];
      out[2] = v; }
    { fom_float v = (bias[3] + state[52]) + (ca * (((state[16] + state[17]) + (state[18] + state[19])) + ((state[20] + state[21]) + state[22])) + cb * (((t_t0[16] + t_t0[17]) + (t_t0[18] + t_t0[19])) + ((t_t0[20] + t_t0[21]) + t_t0[22])));
      if (v < act->lo[3]) v = act->lo[3];
      out[3] = v; }
    { fom_float v = (bias[4] + state[53]) + (ca * ((state[23] + state[24]) + state[25]) + cb * ((t_t0[23] + t_t0[24]) + t_t0[25]));
      if (v < act->lo[4]) v = act->lo[4];
      out[4] = v; }
    { fom_float v = (bias[5] + state[54]) + (ca * (((state[26] + state[27]) + (state[28] + state[29])) + ((state[30] + state[31]) + (state[32] + state[33]))) + cb * (((t_t0[26] + t_t0[27]) + (t_t0[28] + t_t0[29])) + ((t_t0[30] + t_t0[31]) + (t_t0[32] + t_t0[33]))));
      if (v < act->lo[5]) v = act->lo[5];
      out[5] = v; }
    { fom_float v = (bias[6] + state[55]) + (ca * ((state[34] + state[35]) + state[36]) + cb * ((t_t0[34] + t_t0[35]) + t_t0[36]));
      if (v < act->lo[6]) v = act->lo[6];
      out[6] = v; }
    { fom_float v = (bias[7] + state[56]) + (ca * (state[37] + state[38]) + cb * (t_t0[37] + t_t0[38]));
      if (v < act->lo[7]) v = act->lo[7];
      out[7] = v; }
    { fom_float v = (bias[8] + state[57]) + (ca * (((state[39] + state[40]) + (state[41] + state[42])) + state[43]) + cb * (((t_t0[39] + t_t0[40]) + (t_t0[41] + t_t0[42])) + t_t0[43]));
      if (v < act->lo[8]) v = act->lo[8];
      out[8] = v; }
    { fom_float v = (bias[9] + state[58]) + (ca * (((state[44] + state[45]) + (state[46] + state[47])) + state[48]) + cb * (((t_t0[44] + t_t0[45]) + (t_t0[46] + t_t0[47])) + t_t0[48]));
      if (v < act->lo[9]) v = act->lo[9];
      out[9] = v; }
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

void fom_step_su__rest(const fom_float *restrict state, const fom_float *restrict t_t0, const fom_float *restrict t_t1, fom_float sc_xi, const fom_float *restrict bias, fom_float *restrict out) {
    out[59] = ((((bias[59] + state[59]) + ((-0.06965506162606752 + sc_xi * -0.1393101232521349) * state[0] + (-0.3376078355668656 + sc_xi * -0.6752156711337306) * state[12])) + (((-0.2683650108072723 + sc_xi * -0.536730021614544) * state[16] + (-0.03708166163148883 + sc_xi * -0.07416332326297768) * state[26]) + ((-0.37063781296154064 + sc_xi * -0.7412756259230812) * state[39] + (0.13931012325213496 + sc_xi * 0.13931012325213504) * t_t0[0]))) + (((0.6752156711337309 + sc_xi * 0.6752156711337306) * t_t0[12] + (0.5367300216145444 + sc_xi * 0.5367300216145442) * t_t0[16]) + ((0.0741633232629777 + sc_xi * 0.07416332326297763) * t_t0[26] + (0.7412756259230815 + sc_xi * 0.7412756259230803) * t_t0[39])));
    out[60] = ((((bias[60] + state[60]) + ((-0.25588244633413904 + sc_xi * -0.5117648926682782) * state[1] + (-0.03758350075816168 + sc_xi * -0.07516700151632334) * state[5])) + (((-0.08327630054464728 + sc_xi * -0.1665526010892945) * state[27] + (-0.023067940734351824 + sc_xi * -0.046135881468703656) * state[37]) + ((0.5117648926682782 + sc_xi * 0.511764892668278) * t_t0[1] + (0.07516700151632338 + sc_xi * 0.07516700151632327) * t_t0[5]))) + ((0.1665526010892946 + sc_xi * 0.1665526010892944) * t_t0[27] + (0.04613588146870367 + sc_xi * 0.0461358814687036) * t_t0[37]));
    out[61] = (((bias[61] + state[61]) + ((-0.6175238612165267 + sc_xi * -1.2350477224330518) * state[6] + (-0.04238843516524547 + sc_xi * -0.08477687033049083) * state[17])) + ((1.2350477224330532 + sc_xi * 1.2350477224330518) * t_t0[6] + (0.08477687033049092 + sc_xi * 0.08477687033049087) * t_t0[17]));
    out[62] = (((bias[62] + state[62]) + ((-0.13345673088354354 + sc_xi * -0.2669134617670867) * state[2] + (-0.611324088246934 + sc_xi * -1.2226481764938668) * state[23])) + ((0.266913461767087 + sc_xi * 0.2669134617670866) * t_t0[2] + (1.2226481764938675 + sc_xi * 1.2226481764938673) * t_t0[23]));
    out[63] = (((bias[63] + state[63]) + ((-0.040307047471874385 + sc_xi * -0.0806140949437488) * state[13] + (-0.029151569883073405 + sc_xi * -0.05830313976614678) * state[18])) + (((-0.5843595834842807 + sc_xi * -1.1687191669685617) * state[28] + (0.0806140949437488 + sc_xi * 0.08061409494374874) * t_t0[13]) + ((0.05830313976614679 + sc_xi * 0.05830313976614681) * t_t0[18] + (1.168719166968562 + sc_xi * 1.1687191669685606) * t_t0[28])));
    out[64] = (((bias[64] + state[64]) + ((-0.6448341284270941 + sc_xi * -1.2896682568541873) * state[7] + (-0.36011436438990646 + sc_xi * -0.7202287287798131) * state[19])) + (((-0.05109107424493329 + sc_xi * -0.10218214848986652) * state[29] + (1.289668256854188 + sc_xi * 1.2896682568541873) * t_t0[7]) + ((0.7202287287798131 + sc_xi * 0.7202287287798127) * t_t0[19] + (0.10218214848986659 + sc_xi * 0.10218214848986652) * t_t0[29])));
    out[65] = ((((bias[65] + state[65]) + ((-0.02496413678899907 + sc_xi * -0.04992827357799812) * state[3] + (-0.03788233941037191 + sc_xi * -0.0757646788207438) * state[8])) + (((-0.00976037797556846 + sc_xi * -0.019520755951136917) * state[14] + (-0.2765836919316893 + sc_xi * -0.553167383863378) * state[20]) + ((-0.030365879986127997 + sc_xi * -0.06073175997225594) * state[44] + (0.04992827357799813 + sc_xi * 0.04992827357799812) * t_t0[3]))) + (((0.07576467882074381 + sc_xi * 0.07576467882074377) * t_t0[8] + (0.01952075595113692 + sc_xi * 0.019520755951136917) * t_t0[14]) + ((0.5531673838633784 + sc_xi * 0.5531673838633783) * t_t0[20] + (0.060731759972255994 + sc_xi * 0.06073175997225592) * t_t0[44])));
    out[66] = (((bias[66] + state[66]) + ((-0.7947858773306501 + sc_xi * -1.5895717546612993) * state[9] + (-0.014854938892497714 + sc_xi * -0.02970987778499542) * state[30])) + (((-0.018156664303604783 + sc_xi * -0.036313328607209545) * state[38] + (1.5895717546613 + sc_xi * 1.5895717546612993) * t_t0[9]) + ((0.029709877784995432 + sc_xi * 0.029709877784995405) * t_t0[30] + (0.036313328607209566 + sc_xi * 0.03631332860720952) * t_t0[38])));
    out[67] = (((bias[67] + state[67]) + ((-0.131749302940035 + sc_xi * -0.2634986058800694) * state[40] + (-0.14046939723165494 + sc_xi * -0.2809387944633095) * state[45])) + ((0.2634986058800698 + sc_xi * 0.2634986058800696) * t_t0[40] + (0.28093879446330966 + sc_xi * 0.2809387944633098) * t_t0[45]));
    out[68] = (((bias[68] + state[68]) + ((-0.013160484266981862 + sc_xi * -0.026320968533963696) * state[24] + (-0.38643208723267564 + sc_xi * -0.7728641744653503) * state[34])) + ((0.02632096853396372 + sc_xi * 0.026320968533963696) * t_t0[24] + (0.7728641744653508 + sc_xi * 0.7728641744653507) * t_t0[34]));
    out[69] = ((((bias[69] + state[69]) + ((-0.017294599334807734 + sc_xi * -0.03458919866961541) * state[10] + (-0.6037269576533858 + sc_xi * -1.207453915306772) * state[31])) + (((-0.2585411937087774 + sc_xi * -0.5170823874175547) * state[35] + (-0.14484430995648817 + sc_xi * -0.2896886199129758) * state[41]) + ((0.03458919866961544 + sc_xi * 0.03458919866961544) * t_t0[10] + (1.2074539153067723 + sc_xi * 1.2074539153067712) * t_t0[31]))) + ((0.5170823874175549 + sc_xi * 0.5170823874175545) * t_t0[35] + (0.2896886199129762 + sc_xi * 0.28968861991297584) * t_t0[41]));
    out[70] = (((bias[70] + state[70]) + ((-0.034735970917354184 + sc_xi * -0.06947194183470837) * state[21] + (-0.15819624000738947 + sc_xi * -0.3163924800147784) * state[25])) + ((0.06947194183470837 + sc_xi * 0.06947194183470838) * t_t0[21] + (0.3163924800147787 + sc_xi * 0.3163924800147786) * t_t0[25]));
    out[71] = ((((bias[71] + state[71]) + ((-0.021966725812902 + sc_xi * -0.043933451625803976) * state[4] + (-0.02159040904130785 + sc_xi * -0.043180818082615646) * state[32])) + (((-0.03484606682994429 + sc_xi * -0.06969213365988852) * state[42] + (-0.3265308594327321 + sc_xi * -0.6530617188654634) * state[46]) + ((0.04393345162580401 + sc_xi * 0.04393345162580395) * t_t0[4] + (0.04318081808261569 + sc_xi * 0.04318081808261562) * t_t0[32]))) + ((0.06969213365988856 + sc_xi * 0.06969213365988855) * t_t0[42] + (0.6530617188654638 + sc_xi * 0.6530617188654637) * t_t0[46]));
    out[72] = (((bias[72] + state[72]) + ((-0.01155639562656541 + sc_xi * -0.023112791253130805) * state[33] + (-0.09852027593353693 + sc_xi * -0.19704055186707384) * state[43])) + (((-0.17994004051645768 + sc_xi * -0.3598800810329153) * state[47] + (0.023112791253130816 + sc_xi * 0.023112791253130805) * t_t0[33]) + ((0.19704055186707398 + sc_xi * 0.19704055186707362) * t_t0[43] + (0.35988008103291547 + sc_xi * 0.3598800810329151) * t_t0[47])));
    out[73] = ((((bias[73] + state[73]) + ((-0.08961892294526974 + sc_xi * -0.1792378458905392) * state[11] + (-0.46831584781193325 + sc_xi * -0.9366316956238665) * state[15])) + (((-0.022087418331416996 + sc_xi * -0.044174836662833984) * state[22] + (-0.41032977319887487 + sc_xi * -0.8206595463977495) * state[36]) + ((-0.22496888289546174 + sc_xi * -0.44993776579092293) * state[48] + (0.17923784589053937 + sc_xi * 0.17923784589053932) * t_t0[11]))) + (((0.9366316956238666 + sc_xi * 0.9366316956238665) * t_t0[15] + (0.044174836662834 + sc_xi * 0.044174836662833984) * t_t0[22]) + ((0.8206595463977497 + sc_xi * 0.8206595463977495) * t_t0[36] + (0.4499377657909233 + sc_xi * 0.44993776579092315) * t_t0[48])));
}

void fom_iterate(const fom_float *state, fom_float sc_xi, const fom_float *bias_t0, const fom_float *bias_t1, const fom_float *bias_su, const FomAct_t0 *act_t0, const FomAct_t1 *act_t1, fom_float *t_t0, fom_float *t_t1, fom_float *new_state) {
    fom_step_t0(state, bias_t0, act_t0, t_t0);
    fom_step_t1(state, t_t0, sc_xi, bias_t1, act_t1, t_t1);
    fom_step_su(state, t_t0, t_t1, sc_xi, bias_su, new_state);
}
