#include "fom_ops.h"
#include "fom_data.h"
#include "fom_profile.h"

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

void fom_iterate(const fom_float *restrict state, const fom_float *restrict bias_t0, const fom_float *restrict bias_t1, const fom_float *restrict bias_su, const FomAct_t0 *restrict act_t0, const FomAct_t1 *restrict act_t1, fom_float *restrict t_t0, fom_float *restrict t_t1, fom_float *restrict new_state) {
    /* t1_t_t0 = -2 t1_state[:, :49] and su_t_t0 = -2 su_state[:, :49] on the
       non-copy rows, so both blocks share w = state[:49] - 2 t_t0 */
    const fom_float *restrict lo0 = act_t0->lo, *restrict hi0 = act_t0->hi, *restrict lo1 = act_t1->lo;
    fom_float w[49];
    FOM_PROF_BEGIN(it_t0);
    { fom_float v = (bias_t0[0] + state[0]) + (-0.31483425579287277 * state[49] + -0.12559068930987091 * state[59]); if (v < lo0[0]) v = lo0[0]; if (v > hi0[0]) v = hi0[0]; t_t0[0] = v; new_state[0] = bias_su[0] + v; w[0] = state[0] - 2.0 * v; }
    { fom_float v = (bias_t0[1] + state[1]) + (-0.1785106319392791 * state[49] + -0.31483425579287283 * state[60]); if (v < lo0[1]) v = lo0[1]; if (v > hi0[1]) v = hi0[1]; t_t0[1] = v; new_state[1] = bias_su[1] + v; w[1] = state[1] - 2.0 * v; }
    { fom_float v = (bias_t0[2] + state[2]) + (-0.31483425565100043 * state[49] + -0.18736322506593048 * state[62]); if (v < lo0[2]) v = lo0[2]; if (v > hi0[2]) v = hi0[2]; t_t0[2] = v; new_state[2] = bias_su[2] + v; w[2] = state[2] - 2.0 * v; }
    { fom_float v = (bias_t0[3] + state[3]) + (-0.31483425579287277 * state[49] + -0.052105419223993081 * state[65]); if (v < lo0[3]) v = lo0[3]; if (v > hi0[3]) v = hi0[3]; t_t0[3] = v; new_state[3] = bias_su[3] + v; w[3] = state[3] - 2.0 * v; }
    { fom_float v = (bias_t0[4] + state[4]) + (-0.31483425579287277 * state[49] + -0.042197122740314078 * state[71]); if (v < lo0[4]) v = lo0[4]; if (v > hi0[4]) v = hi0[4]; t_t0[4] = v; new_state[4] = bias_su[4] + v; w[4] = state[4] - 2.0 * v; }
    { fom_float v = (bias_t0[5] + state[5]) + (-0.31483425579287277 * state[50] + -0.081556129918491121 * state[60]); if (v < lo0[5]) v = lo0[5]; if (v > hi0[5]) v = hi0[5]; t_t0[5] = v; new_state[5] = bias_su[5] + v; w[5] = state[5] - 2.0 * v; }
    { fom_float v = (bias_t0[6] + state[6]) + (-0.1149099042416003 * state[50] + -0.31483425579287283 * state[61]); if (v < lo0[6]) v = lo0[6]; if (v > hi0[6]) v = hi0[6]; t_t0[6] = v; new_state[6] = bias_su[6] + v; w[6] = state[6] - 2.0 * v; }
    { fom_float v = (bias_t0[7] + state[7]) + (-0.11245022456106747 * state[50] + -0.31483425579287277 * state[64]); if (v < lo0[7]) v = lo0[7]; if (v > hi0[7]) v = hi0[7]; t_t0[7] = v; new_state[7] = bias_su[7] + v; w[7] = state[7] - 2.0 * v; }
    { fom_float v = (bias_t0[8] + state[8]) + (-0.31483425579287277 * state[50] + -0.079068432962314555 * state[65]); if (v < lo0[8]) v = lo0[8]; if (v > hi0[8]) v = hi0[8]; t_t0[8] = v; new_state[8] = bias_su[8] + v; w[8] = state[8] - 2.0 * v; }
    { fom_float v = (bias_t0[9] + state[9]) + (-0.1012882957183817 * state[50] + -0.31483425579287277 * state[66]); if (v < lo0[9]) v = lo0[9]; if (v > hi0[9]) v = hi0[9]; t_t0[9] = v; new_state[9] = bias_su[9] + v; w[9] = state[9] - 2.0 * v; }
    { fom_float v = (bias_t0[10] + state[10]) + (-0.31483425579287277 * state[50] + -0.024432609086831557 * state[69]); if (v < lo0[10]) v = lo0[10]; if (v > hi0[10]) v = hi0[10]; t_t0[10] = v; new_state[10] = bias_su[10] + v; w[10] = state[10] - 2.0 * v; }
    { fom_float v = (bias_t0[11] + state[11]) + (-0.31483425576776075 * state[50] + -0.14375068353499568 * state[73]); if (v < lo0[11]) v = lo0[11]; if (v > hi0[11]) v = hi0[11]; t_t0[11] = v; new_state[11] = bias_su[11] + v; w[11] = state[11] - 2.0 * v; }
    { fom_float v = (bias_t0[12] + state[12]) + (-0.16283459527310534 * state[51] + -0.31483425576550422 * state[59]); if (v < lo0[12]) v = lo0[12]; if (v > hi0[12]) v = hi0[12]; t_t0[12] = v; new_state[12] = bias_su[12] + v; w[12] = state[12] - 2.0 * v; }
    { fom_float v = (bias_t0[13] + state[13]) + (-0.31483425579287277 * state[51] + -0.057878934426039565 * state[63]); if (v < lo0[13]) v = lo0[13]; if (v > hi0[13]) v = hi0[13]; t_t0[13] = v; new_state[13] = bias_su[13] + v; w[13] = state[13] - 2.0 * v; }
    { fom_float v = (bias_t0[14] + state[14]) + (-0.31483425579287277 * state[51] + -0.020371967614988162 * state[65]); if (v < lo0[14]) v = lo0[14]; if (v > hi0[14]) v = hi0[14]; t_t0[14] = v; new_state[14] = bias_su[14] + v; w[14] = state[14] - 2.0 * v; }
    { fom_float v = (bias_t0[15] + state[15]) + (-0.13195167489181564 * state[51] + -0.31483425579287283 * state[73]); if (v < lo0[15]) v = lo0[15]; if (v > hi0[15]) v = hi0[15]; t_t0[15] = v; new_state[15] = bias_su[15] + v; w[15] = state[15] - 2.0 * v; }
    { fom_float v = (bias_t0[16] + state[16]) + (-0.20484874346487622 * state[52] + -0.31483425569820117 * state[59]); if (v < lo0[16]) v = lo0[16]; if (v > hi0[16]) v = hi0[16]; t_t0[16] = v; new_state[16] = bias_su[16] + v; w[16] = state[16] - 2.0 * v; }
    { fom_float v = (bias_t0[17] + state[17]) + (-0.31483425579287277 * state[52] + -0.059210692177281046 * state[61]); if (v < lo0[17]) v = lo0[17]; if (v > hi0[17]) v = hi0[17]; t_t0[17] = v; new_state[17] = bias_su[17] + v; w[17] = state[17] - 2.0 * v; }
    { fom_float v = (bias_t0[18] + state[18]) + (-0.31483425579287277 * state[52] + -0.041860218187796047 * state[63]); if (v < lo0[18]) v = lo0[18]; if (v > hi0[18]) v = hi0[18]; t_t0[18] = v; new_state[18] = bias_su[18] + v; w[18] = state[18] - 2.0 * v; }
    { fom_float v = (bias_t0[19] + state[19]) + (-0.20135753992977129 * state[52] + -0.31483425562205569 * state[64]); if (v < lo0[19]) v = lo0[19]; if (v > hi0[19]) v = hi0[19]; t_t0[19] = v; new_state[19] = bias_su[19] + v; w[19] = state[19] - 2.0 * v; }
    { fom_float v = (bias_t0[20] + state[20]) + (-0.17170029832945369 * state[52] + -0.31483425579287283 * state[65]); if (v < lo0[20]) v = lo0[20]; if (v > hi0[20]) v = hi0[20]; t_t0[20] = v; new_state[20] = bias_su[20] + v; w[20] = state[20] - 2.0 * v; }
    { fom_float v = (bias_t0[21] + state[21]) + (-0.31483425579287277 * state[52] + -0.095865201522679772 * state[70]); if (v < lo0[21]) v = lo0[21]; if (v > hi0[21]) v = hi0[21]; t_t0[21] = v; new_state[21] = bias_su[21] + v; w[21] = state[21] - 2.0 * v; }
    { fom_float v = (bias_t0[22] + state[22]) + (-0.31483425579287277 * state[52] + -0.035428694951588111 * state[73]); if (v < lo0[22]) v = lo0[22]; if (v > hi0[22]) v = hi0[22]; t_t0[22] = v; new_state[22] = bias_su[22] + v; w[22] = state[22] - 2.0 * v; }
    { fom_float v = (bias_t0[23] + state[23]) + (-0.11549111655165208 * state[53] + -0.31483425579287277 * state[62]); if (v < lo0[23]) v = lo0[23]; if (v > hi0[23]) v = hi0[23]; t_t0[23] = v; new_state[23] = bias_su[23] + v; w[23] = state[23] - 2.0 * v; }
    { fom_float v = (bias_t0[24] + state[24]) + (-0.31483425579287277 * state[53] + -0.023238864598874573 * state[68]); if (v < lo0[24]) v = lo0[24]; if (v > hi0[24]) v = hi0[24]; t_t0[24] = v; new_state[24] = bias_su[24] + v; w[24] = state[24] - 2.0 * v; }
    { fom_float v = (bias_t0[25] + state[25]) + (-0.22703157183385472 * state[53] + -0.31483425579287283 * state[70]); if (v < lo0[25]) v = lo0[25]; if (v > hi0[25]) v = hi0[25]; t_t0[25] = v; new_state[25] = bias_su[25] + v; w[25] = state[25] - 2.0 * v; }
    { fom_float v = (bias_t0[26] + state[26]) + (-0.31483425579287277 * state[54] + -0.066859627087189483 * state[59]); if (v < lo0[26]) v = lo0[26]; if (v > hi0[26]) v = hi0[26]; t_t0[26] = v; new_state[26] = bias_su[26] + v; w[26] = state[26] - 2.0 * v; }
    { fom_float v = (bias_t0[27] + state[27]) + (-0.31483425578928326 * state[54] + -0.18070942432892551 * state[60]); if (v < lo0[27]) v = lo0[27]; if (v > hi0[27]) v = hi0[27]; t_t0[27] = v; new_state[27] = bias_su[27] + v; w[27] = state[27] - 2.0 * v; }
    { fom_float v = (bias_t0[28] + state[28]) + (-0.11812566027056481 * state[54] + -0.31483425579287283 * state[63]); if (v < lo0[28]) v = lo0[28]; if (v > hi0[28]) v = hi0[28]; t_t0[28] = v; new_state[28] = bias_su[28] + v; w[28] = state[28] - 2.0 * v; }
    { fom_float v = (bias_t0[29] + state[29]) + (-0.31483425579287277 * state[54] + -0.069839424573329659 * state[64]); if (v < lo0[29]) v = lo0[29]; if (v > hi0[29]) v = hi0[29]; t_t0[29] = v; new_state[29] = bias_su[29] + v; w[29] = state[29] - 2.0 * v; }
    { fom_float v = (bias_t0[30] + state[30]) + (-0.31483425579287277 * state[54] + -0.018290493543120279 * state[66]); if (v < lo0[30]) v = lo0[30]; if (v > hi0[30]) v = hi0[30]; t_t0[30] = v; new_state[30] = bias_su[30] + v; w[30] = state[30] - 2.0 * v; }
    { fom_float v = (bias_t0[31] + state[31]) + (-0.11621549871906076 * state[54] + -0.31483425579287277 * state[69]); if (v < lo0[31]) v = lo0[31]; if (v > hi0[31]) v = hi0[31]; t_t0[31] = v; new_state[31] = bias_su[31] + v; w[31] = state[31] - 2.0 * v; }
    { fom_float v = (bias_t0[32] + state[32]) + (-0.31483425579287277 * state[54] + -0.041474234626015731 * state[71]); if (v < lo0[32]) v = lo0[32]; if (v > hi0[32]) v = hi0[32]; t_t0[32] = v; new_state[32] = bias_su[32] + v; w[32] = state[32] - 2.0 * v; }
    { fom_float v = (bias_t0[33] + state[33]) + (-0.31483425579287277 * state[54] + -0.029904608409446035 * state[72]); if (v < lo0[33]) v = lo0[33]; if (v > hi0[33]) v = hi0[33]; t_t0[33] = v; new_state[33] = bias_su[33] + v; w[33] = state[33] - 2.0 * v; }
    { fom_float v = (bias_t0[34] + state[34]) + (-0.20072699455717991 * state[55] + -0.31483425579287277 * state[68]); if (v < lo0[34]) v = lo0[34]; if (v > hi0[34]) v = hi0[34]; t_t0[34] = v; new_state[34] = bias_su[34] + v; w[34] = state[34] - 2.0 * v; }
    { fom_float v = (bias_t0[35] + state[35]) + (-0.31483425579287283 * state[55] + -0.26432062531400063 * state[69]); if (v < lo0[35]) v = lo0[35]; if (v > hi0[35]) v = hi0[35]; t_t0[35] = v; new_state[35] = bias_su[35] + v; w[35] = state[35] - 2.0 * v; }
    { fom_float v = (bias_t0[36] + state[36]) + (-0.20810320529794363 * state[55] + -0.31483425575411544 * state[73]); if (v < lo0[36]) v = lo0[36]; if (v > hi0[36]) v = hi0[36]; t_t0[36] = v; new_state[36] = bias_su[36] + v; w[36] = state[36] - 2.0 * v; }
    { fom_float v = (bias_t0[37] + state[37]) + (-0.31483425579287277 * state[56] + -0.050057390438124606 * state[60]); if (v < lo0[37]) v = lo0[37]; if (v > hi0[37]) v = hi0[37]; t_t0[37] = v; new_state[37] = bias_su[37] + v; w[37] = state[37] - 2.0 * v; }
    { fom_float v = (bias_t0[38] + state[38]) + (-0.31483425579287277 * state[56] + -0.022355820755170213 * state[66]); if (v < lo0[38]) v = lo0[38]; if (v > hi0[38]) v = hi0[38]; t_t0[38] = v; new_state[38] = bias_su[38] + v; w[38] = state[38] - 2.0 * v; }
    { fom_float v = (bias_t0[39] + state[39]) + (-0.14832333169428388 * state[57] + -0.31483425579287277 * state[59]); if (v < lo0[39]) v = lo0[39]; if (v > hi0[39]) v = hi0[39]; t_t0[39] = v; new_state[39] = bias_su[39] + v; w[39] = state[39] - 2.0 * v; }
    { fom_float v = (bias_t0[40] + state[40]) + (-0.25687797426285774 * state[57] + -0.3148342557740812 * state[67]); if (v < lo0[40]) v = lo0[40]; if (v > hi0[40]) v = hi0[40]; t_t0[40] = v; new_state[40] = bias_su[40] + v; w[40] = state[40] - 2.0 * v; }
    { fom_float v = (bias_t0[41] + state[41]) + (-0.31483425562699158 * state[57] + -0.2046259837099301 * state[69]); if (v < lo0[41]) v = lo0[41]; if (v > hi0[41]) v = hi0[41]; t_t0[41] = v; new_state[41] = bias_su[41] + v; w[41] = state[41] - 2.0 * v; }
    { fom_float v = (bias_t0[42] + state[42]) + (-0.31483425579287277 * state[57] + -0.066937775413790396 * state[71]); if (v < lo0[42]) v = lo0[42]; if (v > hi0[42]) v = hi0[42]; t_t0[42] = v; new_state[42] = bias_su[42] + v; w[42] = state[42] - 2.0 * v; }
    { fom_float v = (bias_t0[43] + state[43]) + (-0.31483425573999474 * state[57] + -0.25494197039391109 * state[72]); if (v < lo0[43]) v = lo0[43]; if (v > hi0[43]) v = hi0[43]; t_t0[43] = v; new_state[43] = bias_su[43] + v; w[43] = state[43] - 2.0 * v; }
    { fom_float v = (bias_t0[44] + state[44]) + (-0.31483425579287277 * state[58] + -0.063379996679072001 * state[65]); if (v < lo0[44]) v = lo0[44]; if (v > hi0[44]) v = hi0[44]; t_t0[44] = v; new_state[44] = bias_su[44] + v; w[44] = state[44] - 2.0 * v; }
    { fom_float v = (bias_t0[45] + state[45]) + (-0.2409314392941177 * state[58] + -0.31483425579287283 * state[67]); if (v < lo0[45]) v = lo0[45]; if (v > hi0[45]) v = hi0[45]; t_t0[45] = v; new_state[45] = bias_su[45] + v; w[45] = state[45] - 2.0 * v; }
    { fom_float v = (bias_t0[46] + state[46]) + (-0.15802369728139173 * state[58] + -0.31483425579287277 * state[71]); if (v < lo0[46]) v = lo0[46]; if (v > hi0[46]) v = hi0[46]; t_t0[46] = v; new_state[46] = bias_su[46] + v; w[46] = state[46] - 2.0 * v; }
    { fom_float v = (bias_t0[47] + state[47]) + (-0.21287292492070603 * state[58] + -0.31483425579287277 * state[72]); if (v < lo0[47]) v = lo0[47]; if (v > hi0[47]) v = hi0[47]; t_t0[47] = v; new_state[47] = bias_su[47] + v; w[47] = state[47] - 2.0 * v; }
    { fom_float v = (bias_t0[48] + state[48]) + (-0.27468270126800032 * state[58] + -0.31483425557789518 * state[73]); if (v < lo0[48]) v = lo0[48]; if (v > hi0[48]) v = hi0[48]; t_t0[48] = v; new_state[48] = bias_su[48] + v; w[48] = state[48] - 2.0 * v; }
    FOM_PROF_END(it_t0);
    FOM_PROF_BEGIN(it_t1);
    { fom_float v = ((bias_t1[0] + -0.31483425579287277 * w[0]) + (-0.1785106319392791 * w[1] + -0.31483425565100043 * w[2])) + ((-0.31483425579287277 * w[3] + -0.31483425579287277 * w[4]) + state[49]); if (v < lo1[0]) v = lo1[0]; t_t1[0] = v; new_state[49] = bias_su[49] + v; }
    { fom_float v = (((bias_t1[1] + -0.31483425579287277 * w[5]) + -0.1149099042416003 * w[6]) + (-0.11245022456106747 * w[7] + -0.31483425579287277 * w[8])) + ((-0.1012882957183817 * w[9] + -0.31483425579287277 * w[10]) + (-0.31483425576776075 * w[11] + state[50])); if (v < lo1[1]) v = lo1[1]; t_t1[1] = v; new_state[50] = bias_su[50] + v; }
    { fom_float v = ((bias_t1[2] + -0.16283459527310534 * w[12]) + -0.31483425579287277 * w[13]) + ((-0.31483425579287277 * w[14] + -0.13195167489181564 * w[15]) + state[51]); if (v < lo1[2]) v = lo1[2]; t_t1[2] = v; new_state[51] = bias_su[51] + v; }
    { fom_float v = (((bias_t1[3] + -0.20484874346487622 * w[16]) + -0.31483425579287277 * w[17]) + (-0.31483425579287277 * w[18] + -0.20135753992977129 * w[19])) + ((-0.17170029832945369 * w[20] + -0.31483425579287277 * w[21]) + (-0.31483425579287277 * w[22] + state[52])); if (v < lo1[3]) v = lo1[3]; t_t1[3] = v; new_state[52] = bias_su[52] + v; }
    { fom_float v = ((bias_t1[4] + -0.11549111655165208 * w[23]) + -0.31483425579287277 * w[24]) + (-0.22703157183385472 * w[25] + state[53]); if (v < lo1[4]) v = lo1[4]; t_t1[4] = v; new_state[53] = bias_su[53] + v; }
    { fom_float v = (((bias_t1[5] + -0.31483425579287277 * w[26]) + -0.31483425578928326 * w[27]) + (-0.11812566027056481 * w[28] + -0.31483425579287277 * w[29])) + (((-0.31483425579287277 * w[30] + -0.11621549871906076 * w[31]) + -0.31483425579287277 * w[32]) + (-0.31483425579287277 * w[33] + state[54])); if (v < lo1[5]) v = lo1[5]; t_t1[5] = v; new_state[54] = bias_su[54] + v; }
    { fom_float v = ((bias_t1[6] + -0.20072699455717991 * w[34]) + -0.31483425579287283 * w[35]) + (-0.20810320529794363 * w[36] + state[55]); if (v < lo1[6]) v = lo1[6]; t_t1[6] = v; new_state[55] = bias_su[55] + v; }
    { fom_float v = (bias_t1[7] + -0.31483425579287277 * w[37]) + (-0.31483425579287277 * w[38] + state[56]); if (v < lo1[7]) v = lo1[7]; t_t1[7] = v; new_state[56] = bias_su[56] + v; }
    { fom_float v = ((bias_t1[8] + -0.14832333169428388 * w[39]) + (-0.25687797426285774 * w[40] + -0.31483425562699158 * w[41])) + ((-0.31483425579287277 * w[42] + -0.31483425573999474 * w[43]) + state[57]); if (v < lo1[8]) v = lo1[8]; t_t1[8] = v; new_state[57] = bias_su[57] + v; }
    { fom_float v = ((bias_t1[9] + -0.31483425579287277 * w[44]) + (-0.2409314392941177 * w[45] + -0.15802369728139173 * w[46])) + ((-0.21287292492070603 * w[47] + -0.27468270126800032 * w[48]) + state[58]); if (v < lo1[9]) v = lo1[9]; t_t1[9] = v; new_state[58] = bias_su[58] + v; }
    FOM_PROF_END(it_t1);
    FOM_PROF_BEGIN(it_su);
    new_state[59] = ((bias_su[59] + -0.12559068930987091 * w[0]) + (-0.31483425576550422 * w[12] + -0.31483425569820117 * w[16])) + ((-0.066859627087189483 * w[26] + -0.31483425579287277 * w[39]) + state[59]);
    new_state[60] = ((bias_su[60] + -0.31483425579287283 * w[1]) + -0.081556129918491121 * w[5]) + ((-0.18070942432892551 * w[27] + -0.050057390438124606 * w[37]) + state[60]);
    new_state[61] = (bias_su[61] + -0.31483425579287283 * w[6]) + (-0.059210692177281046 * w[17] + state[61]);
    new_state[62] = (bias_su[62] + -0.18736322506593048 * w[2]) + (-0.31483425579287277 * w[23] + state[62]);
    new_state[63] = ((bias_su[63] + -0.057878934426039565 * w[13]) + -0.041860218187796047 * w[18]) + (-0.31483425579287283 * w[28] + state[63]);
    new_state[64] = ((bias_su[64] + -0.31483425579287277 * w[7]) + -0.31483425562205569 * w[19]) + (-0.069839424573329659 * w[29] + state[64]);
    new_state[65] = ((bias_su[65] + -0.052105419223993081 * w[3]) + (-0.079068432962314555 * w[8] + -0.020371967614988162 * w[14])) + ((-0.31483425579287283 * w[20] + -0.063379996679072001 * w[44]) + state[65]);
    new_state[66] = ((bias_su[66] + -0.31483425579287277 * w[9]) + -0.018290493543120279 * w[30]) + (-0.022355820755170213 * w[38] + state[66]);
    new_state[67] = (bias_su[67] + -0.3148342557740812 * w[40]) + (-0.31483425579287283 * w[45] + state[67]);
    new_state[68] = (bias_su[68] + -0.023238864598874573 * w[24]) + (-0.31483425579287277 * w[34] + state[68]);
    new_state[69] = ((bias_su[69] + -0.024432609086831557 * w[10]) + -0.31483425579287277 * w[31]) + ((-0.26432062531400063 * w[35] + -0.2046259837099301 * w[41]) + state[69]);
    new_state[70] = (bias_su[70] + -0.095865201522679772 * w[21]) + (-0.31483425579287283 * w[25] + state[70]);
    new_state[71] = ((bias_su[71] + -0.042197122740314078 * w[4]) + -0.041474234626015731 * w[32]) + ((-0.066937775413790396 * w[42] + -0.31483425579287277 * w[46]) + state[71]);
    new_state[72] = ((bias_su[72] + -0.029904608409446035 * w[33]) + -0.25494197039391109 * w[43]) + (-0.31483425579287277 * w[47] + state[72]);
    new_state[73] = ((bias_su[73] + -0.14375068353499568 * w[11]) + (-0.31483425579287283 * w[15] + -0.035428694951588111 * w[22])) + ((-0.31483425575411544 * w[36] + -0.31483425557789518 * w[48]) + state[73]);
    FOM_PROF_END(it_su);
}
