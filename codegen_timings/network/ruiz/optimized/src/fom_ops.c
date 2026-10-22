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

static inline void k_t0(const fom_float *state, const fom_float *bias, const fom_float *lo0, const fom_float *hi0, fom_float *out) {
    fom_float v;
    v = bias[0] + state[0] + -0.3148342557928728 * state[49] + -0.1255906893098709 * state[59]; v = v < lo0[0] ? lo0[0] : v; v = v > hi0[0] ? hi0[0] : v; out[0] = v;
    v = bias[1] + state[1] + -0.1785106319392791 * state[49] + -0.31483425579287283 * state[60]; v = v < lo0[1] ? lo0[1] : v; v = v > hi0[1] ? hi0[1] : v; out[1] = v;
    v = bias[2] + state[2] + -0.3148342556510004 * state[49] + -0.18736322506593048 * state[62]; v = v < lo0[2] ? lo0[2] : v; v = v > hi0[2] ? hi0[2] : v; out[2] = v;
    v = bias[3] + state[3] + -0.3148342557928728 * state[49] + -0.05210541922399308 * state[65]; v = v < lo0[3] ? lo0[3] : v; v = v > hi0[3] ? hi0[3] : v; out[3] = v;
    v = bias[4] + state[4] + -0.3148342557928728 * state[49] + -0.04219712274031408 * state[71]; v = v < lo0[4] ? lo0[4] : v; v = v > hi0[4] ? hi0[4] : v; out[4] = v;
    v = bias[5] + state[5] + -0.3148342557928728 * state[50] + -0.08155612991849112 * state[60]; v = v < lo0[5] ? lo0[5] : v; v = v > hi0[5] ? hi0[5] : v; out[5] = v;
    v = bias[6] + state[6] + -0.1149099042416003 * state[50] + -0.31483425579287283 * state[61]; v = v < lo0[6] ? lo0[6] : v; v = v > hi0[6] ? hi0[6] : v; out[6] = v;
    v = bias[7] + state[7] + -0.11245022456106747 * state[50] + -0.3148342557928728 * state[64]; v = v < lo0[7] ? lo0[7] : v; v = v > hi0[7] ? hi0[7] : v; out[7] = v;
    v = bias[8] + state[8] + -0.3148342557928728 * state[50] + -0.07906843296231456 * state[65]; v = v < lo0[8] ? lo0[8] : v; v = v > hi0[8] ? hi0[8] : v; out[8] = v;
    v = bias[9] + state[9] + -0.1012882957183817 * state[50] + -0.3148342557928728 * state[66]; v = v < lo0[9] ? lo0[9] : v; v = v > hi0[9] ? hi0[9] : v; out[9] = v;
    v = bias[10] + state[10] + -0.3148342557928728 * state[50] + -0.024432609086831557 * state[69]; v = v < lo0[10] ? lo0[10] : v; v = v > hi0[10] ? hi0[10] : v; out[10] = v;
    v = bias[11] + state[11] + -0.31483425576776075 * state[50] + -0.14375068353499568 * state[73]; v = v < lo0[11] ? lo0[11] : v; v = v > hi0[11] ? hi0[11] : v; out[11] = v;
    v = bias[12] + state[12] + -0.16283459527310534 * state[51] + -0.3148342557655042 * state[59]; v = v < lo0[12] ? lo0[12] : v; v = v > hi0[12] ? hi0[12] : v; out[12] = v;
    v = bias[13] + state[13] + -0.3148342557928728 * state[51] + -0.057878934426039565 * state[63]; v = v < lo0[13] ? lo0[13] : v; v = v > hi0[13] ? hi0[13] : v; out[13] = v;
    v = bias[14] + state[14] + -0.3148342557928728 * state[51] + -0.020371967614988162 * state[65]; v = v < lo0[14] ? lo0[14] : v; v = v > hi0[14] ? hi0[14] : v; out[14] = v;
    v = bias[15] + state[15] + -0.13195167489181564 * state[51] + -0.31483425579287283 * state[73]; v = v < lo0[15] ? lo0[15] : v; v = v > hi0[15] ? hi0[15] : v; out[15] = v;
    v = bias[16] + state[16] + -0.20484874346487622 * state[52] + -0.31483425569820117 * state[59]; v = v < lo0[16] ? lo0[16] : v; v = v > hi0[16] ? hi0[16] : v; out[16] = v;
    v = bias[17] + state[17] + -0.3148342557928728 * state[52] + -0.059210692177281046 * state[61]; v = v < lo0[17] ? lo0[17] : v; v = v > hi0[17] ? hi0[17] : v; out[17] = v;
    v = bias[18] + state[18] + -0.3148342557928728 * state[52] + -0.04186021818779605 * state[63]; v = v < lo0[18] ? lo0[18] : v; v = v > hi0[18] ? hi0[18] : v; out[18] = v;
    v = bias[19] + state[19] + -0.2013575399297713 * state[52] + -0.3148342556220557 * state[64]; v = v < lo0[19] ? lo0[19] : v; v = v > hi0[19] ? hi0[19] : v; out[19] = v;
    v = bias[20] + state[20] + -0.1717002983294537 * state[52] + -0.31483425579287283 * state[65]; v = v < lo0[20] ? lo0[20] : v; v = v > hi0[20] ? hi0[20] : v; out[20] = v;
    v = bias[21] + state[21] + -0.3148342557928728 * state[52] + -0.09586520152267977 * state[70]; v = v < lo0[21] ? lo0[21] : v; v = v > hi0[21] ? hi0[21] : v; out[21] = v;
    v = bias[22] + state[22] + -0.3148342557928728 * state[52] + -0.03542869495158811 * state[73]; v = v < lo0[22] ? lo0[22] : v; v = v > hi0[22] ? hi0[22] : v; out[22] = v;
    v = bias[23] + state[23] + -0.11549111655165208 * state[53] + -0.3148342557928728 * state[62]; v = v < lo0[23] ? lo0[23] : v; v = v > hi0[23] ? hi0[23] : v; out[23] = v;
    v = bias[24] + state[24] + -0.3148342557928728 * state[53] + -0.023238864598874573 * state[68]; v = v < lo0[24] ? lo0[24] : v; v = v > hi0[24] ? hi0[24] : v; out[24] = v;
    v = bias[25] + state[25] + -0.22703157183385472 * state[53] + -0.31483425579287283 * state[70]; v = v < lo0[25] ? lo0[25] : v; v = v > hi0[25] ? hi0[25] : v; out[25] = v;
    v = bias[26] + state[26] + -0.3148342557928728 * state[54] + -0.06685962708718948 * state[59]; v = v < lo0[26] ? lo0[26] : v; v = v > hi0[26] ? hi0[26] : v; out[26] = v;
    v = bias[27] + state[27] + -0.31483425578928326 * state[54] + -0.1807094243289255 * state[60]; v = v < lo0[27] ? lo0[27] : v; v = v > hi0[27] ? hi0[27] : v; out[27] = v;
    v = bias[28] + state[28] + -0.11812566027056481 * state[54] + -0.31483425579287283 * state[63]; v = v < lo0[28] ? lo0[28] : v; v = v > hi0[28] ? hi0[28] : v; out[28] = v;
    v = bias[29] + state[29] + -0.3148342557928728 * state[54] + -0.06983942457332966 * state[64]; v = v < lo0[29] ? lo0[29] : v; v = v > hi0[29] ? hi0[29] : v; out[29] = v;
    v = bias[30] + state[30] + -0.3148342557928728 * state[54] + -0.01829049354312028 * state[66]; v = v < lo0[30] ? lo0[30] : v; v = v > hi0[30] ? hi0[30] : v; out[30] = v;
    v = bias[31] + state[31] + -0.11621549871906076 * state[54] + -0.3148342557928728 * state[69]; v = v < lo0[31] ? lo0[31] : v; v = v > hi0[31] ? hi0[31] : v; out[31] = v;
    v = bias[32] + state[32] + -0.3148342557928728 * state[54] + -0.04147423462601573 * state[71]; v = v < lo0[32] ? lo0[32] : v; v = v > hi0[32] ? hi0[32] : v; out[32] = v;
    v = bias[33] + state[33] + -0.3148342557928728 * state[54] + -0.029904608409446035 * state[72]; v = v < lo0[33] ? lo0[33] : v; v = v > hi0[33] ? hi0[33] : v; out[33] = v;
    v = bias[34] + state[34] + -0.2007269945571799 * state[55] + -0.3148342557928728 * state[68]; v = v < lo0[34] ? lo0[34] : v; v = v > hi0[34] ? hi0[34] : v; out[34] = v;
    v = bias[35] + state[35] + -0.31483425579287283 * state[55] + -0.26432062531400063 * state[69]; v = v < lo0[35] ? lo0[35] : v; v = v > hi0[35] ? hi0[35] : v; out[35] = v;
    v = bias[36] + state[36] + -0.20810320529794363 * state[55] + -0.31483425575411544 * state[73]; v = v < lo0[36] ? lo0[36] : v; v = v > hi0[36] ? hi0[36] : v; out[36] = v;
    v = bias[37] + state[37] + -0.3148342557928728 * state[56] + -0.050057390438124606 * state[60]; v = v < lo0[37] ? lo0[37] : v; v = v > hi0[37] ? hi0[37] : v; out[37] = v;
    v = bias[38] + state[38] + -0.3148342557928728 * state[56] + -0.022355820755170213 * state[66]; v = v < lo0[38] ? lo0[38] : v; v = v > hi0[38] ? hi0[38] : v; out[38] = v;
    v = bias[39] + state[39] + -0.14832333169428388 * state[57] + -0.3148342557928728 * state[59]; v = v < lo0[39] ? lo0[39] : v; v = v > hi0[39] ? hi0[39] : v; out[39] = v;
    v = bias[40] + state[40] + -0.25687797426285774 * state[57] + -0.3148342557740812 * state[67]; v = v < lo0[40] ? lo0[40] : v; v = v > hi0[40] ? hi0[40] : v; out[40] = v;
    v = bias[41] + state[41] + -0.3148342556269916 * state[57] + -0.2046259837099301 * state[69]; v = v < lo0[41] ? lo0[41] : v; v = v > hi0[41] ? hi0[41] : v; out[41] = v;
    v = bias[42] + state[42] + -0.3148342557928728 * state[57] + -0.0669377754137904 * state[71]; v = v < lo0[42] ? lo0[42] : v; v = v > hi0[42] ? hi0[42] : v; out[42] = v;
    v = bias[43] + state[43] + -0.31483425573999474 * state[57] + -0.2549419703939111 * state[72]; v = v < lo0[43] ? lo0[43] : v; v = v > hi0[43] ? hi0[43] : v; out[43] = v;
    v = bias[44] + state[44] + -0.3148342557928728 * state[58] + -0.063379996679072 * state[65]; v = v < lo0[44] ? lo0[44] : v; v = v > hi0[44] ? hi0[44] : v; out[44] = v;
    v = bias[45] + state[45] + -0.2409314392941177 * state[58] + -0.31483425579287283 * state[67]; v = v < lo0[45] ? lo0[45] : v; v = v > hi0[45] ? hi0[45] : v; out[45] = v;
    v = bias[46] + state[46] + -0.15802369728139173 * state[58] + -0.3148342557928728 * state[71]; v = v < lo0[46] ? lo0[46] : v; v = v > hi0[46] ? hi0[46] : v; out[46] = v;
    v = bias[47] + state[47] + -0.21287292492070603 * state[58] + -0.3148342557928728 * state[72]; v = v < lo0[47] ? lo0[47] : v; v = v > hi0[47] ? hi0[47] : v; out[47] = v;
    v = bias[48] + state[48] + -0.2746827012680003 * state[58] + -0.3148342555778952 * state[73]; v = v < lo0[48] ? lo0[48] : v; v = v > hi0[48] ? hi0[48] : v; out[48] = v;
}

static inline void k_t1(const fom_float *state, const fom_float *t_t0, const fom_float *bias, const fom_float *lo1, fom_float *out) {
    fom_float v;
    v = bias[0] + -0.3148342557928728 * state[0] + -0.1785106319392791 * state[1] + -0.3148342556510004 * state[2] + -0.3148342557928728 * state[3] + -0.3148342557928728 * state[4] + state[49] + 0.6296685115857455 * t_t0[0] + 0.3570212638785582 * t_t0[1] + 0.6296685113020009 * t_t0[2] + 0.6296685115857455 * t_t0[3] + 0.6296685115857455 * t_t0[4]; out[0] = v < lo1[0] ? lo1[0] : v;
    v = bias[1] + -0.3148342557928728 * state[5] + -0.1149099042416003 * state[6] + -0.11245022456106747 * state[7] + -0.3148342557928728 * state[8] + -0.1012882957183817 * state[9] + -0.3148342557928728 * state[10] + -0.31483425576776075 * state[11] + state[50] + 0.6296685115857455 * t_t0[5] + 0.2298198084832006 * t_t0[6] + 0.22490044912213494 * t_t0[7] + 0.6296685115857455 * t_t0[8] + 0.2025765914367634 * t_t0[9] + 0.6296685115857455 * t_t0[10] + 0.6296685115355215 * t_t0[11]; out[1] = v < lo1[1] ? lo1[1] : v;
    v = bias[2] + -0.16283459527310534 * state[12] + -0.3148342557928728 * state[13] + -0.3148342557928728 * state[14] + -0.13195167489181564 * state[15] + state[51] + 0.3256691905462107 * t_t0[12] + 0.6296685115857455 * t_t0[13] + 0.6296685115857455 * t_t0[14] + 0.2639033497836313 * t_t0[15]; out[2] = v < lo1[2] ? lo1[2] : v;
    v = bias[3] + -0.20484874346487622 * state[16] + -0.3148342557928728 * state[17] + -0.3148342557928728 * state[18] + -0.2013575399297713 * state[19] + -0.1717002983294537 * state[20] + -0.3148342557928728 * state[21] + -0.3148342557928728 * state[22] + state[52] + 0.40969748692975244 * t_t0[16] + 0.6296685115857455 * t_t0[17] + 0.6296685115857455 * t_t0[18] + 0.4027150798595426 * t_t0[19] + 0.3434005966589074 * t_t0[20] + 0.6296685115857455 * t_t0[21] + 0.6296685115857455 * t_t0[22]; out[3] = v < lo1[3] ? lo1[3] : v;
    v = bias[4] + -0.11549111655165208 * state[23] + -0.3148342557928728 * state[24] + -0.22703157183385472 * state[25] + state[53] + 0.23098223310330415 * t_t0[23] + 0.6296685115857455 * t_t0[24] + 0.45406314366770945 * t_t0[25]; out[4] = v < lo1[4] ? lo1[4] : v;
    v = bias[5] + -0.3148342557928728 * state[26] + -0.31483425578928326 * state[27] + -0.11812566027056481 * state[28] + -0.3148342557928728 * state[29] + -0.3148342557928728 * state[30] + -0.11621549871906076 * state[31] + -0.3148342557928728 * state[32] + -0.3148342557928728 * state[33] + state[54] + 0.6296685115857455 * t_t0[26] + 0.6296685115785665 * t_t0[27] + 0.23625132054112963 * t_t0[28] + 0.6296685115857455 * t_t0[29] + 0.6296685115857455 * t_t0[30] + 0.2324309974381215 * t_t0[31] + 0.6296685115857455 * t_t0[32] + 0.6296685115857455 * t_t0[33]; out[5] = v < lo1[5] ? lo1[5] : v;
    v = bias[6] + -0.2007269945571799 * state[34] + -0.31483425579287283 * state[35] + -0.20810320529794363 * state[36] + state[55] + 0.4014539891143598 * t_t0[34] + 0.6296685115857457 * t_t0[35] + 0.41620641059588726 * t_t0[36]; out[6] = v < lo1[6] ? lo1[6] : v;
    v = bias[7] + -0.3148342557928728 * state[37] + -0.3148342557928728 * state[38] + state[56] + 0.6296685115857455 * t_t0[37] + 0.6296685115857455 * t_t0[38]; out[7] = v < lo1[7] ? lo1[7] : v;
    v = bias[8] + -0.14832333169428388 * state[39] + -0.25687797426285774 * state[40] + -0.3148342556269916 * state[41] + -0.3148342557928728 * state[42] + -0.31483425573999474 * state[43] + state[57] + 0.29664666338856777 * t_t0[39] + 0.5137559485257155 * t_t0[40] + 0.6296685112539832 * t_t0[41] + 0.6296685115857455 * t_t0[42] + 0.6296685114799895 * t_t0[43]; out[8] = v < lo1[8] ? lo1[8] : v;
    v = bias[9] + -0.3148342557928728 * state[44] + -0.2409314392941177 * state[45] + -0.15802369728139173 * state[46] + -0.21287292492070603 * state[47] + -0.2746827012680003 * state[48] + state[58] + 0.6296685115857455 * t_t0[44] + 0.4818628785882354 * t_t0[45] + 0.31604739456278347 * t_t0[46] + 0.42574584984141206 * t_t0[47] + 0.5493654025360006 * t_t0[48]; out[9] = v < lo1[9] ? lo1[9] : v;
}

static inline void k_su_rest(const fom_float *state, const fom_float *t_t0, const fom_float *t_t1, const fom_float *bias, fom_float *out) {
    out[59] = bias[59] + -0.1255906893098709 * state[0] + -0.3148342557655042 * state[12] + -0.31483425569820117 * state[16] + -0.06685962708718948 * state[26] + -0.3148342557928728 * state[39] + state[59] + 0.2511813786197418 * t_t0[0] + 0.6296685115310084 * t_t0[12] + 0.6296685113964023 * t_t0[16] + 0.13371925417437897 * t_t0[26] + 0.6296685115857455 * t_t0[39];
    out[60] = bias[60] + -0.31483425579287283 * state[1] + -0.08155612991849112 * state[5] + -0.1807094243289255 * state[27] + -0.050057390438124606 * state[37] + state[60] + 0.6296685115857457 * t_t0[1] + 0.16311225983698224 * t_t0[5] + 0.361418848657851 * t_t0[27] + 0.10011478087624921 * t_t0[37];
    out[61] = bias[61] + -0.31483425579287283 * state[6] + -0.059210692177281046 * state[17] + state[61] + 0.6296685115857457 * t_t0[6] + 0.11842138435456209 * t_t0[17];
    out[62] = bias[62] + -0.18736322506593048 * state[2] + -0.3148342557928728 * state[23] + state[62] + 0.37472645013186096 * t_t0[2] + 0.6296685115857455 * t_t0[23];
    out[63] = bias[63] + -0.057878934426039565 * state[13] + -0.04186021818779605 * state[18] + -0.31483425579287283 * state[28] + state[63] + 0.11575786885207913 * t_t0[13] + 0.0837204363755921 * t_t0[18] + 0.6296685115857457 * t_t0[28];
    out[64] = bias[64] + -0.3148342557928728 * state[7] + -0.3148342556220557 * state[19] + -0.06983942457332966 * state[29] + state[64] + 0.6296685115857455 * t_t0[7] + 0.6296685112441114 * t_t0[19] + 0.13967884914665932 * t_t0[29];
    out[65] = bias[65] + -0.05210541922399308 * state[3] + -0.07906843296231456 * state[8] + -0.020371967614988162 * state[14] + -0.31483425579287283 * state[20] + -0.063379996679072 * state[44] + state[65] + 0.10421083844798616 * t_t0[3] + 0.1581368659246291 * t_t0[8] + 0.040743935229976325 * t_t0[14] + 0.6296685115857457 * t_t0[20] + 0.126759993358144 * t_t0[44];
    out[66] = bias[66] + -0.3148342557928728 * state[9] + -0.01829049354312028 * state[30] + -0.022355820755170213 * state[38] + state[66] + 0.6296685115857455 * t_t0[9] + 0.03658098708624056 * t_t0[30] + 0.044711641510340426 * t_t0[38];
    out[67] = bias[67] + -0.3148342557740812 * state[40] + -0.31483425579287283 * state[45] + state[67] + 0.6296685115481624 * t_t0[40] + 0.6296685115857457 * t_t0[45];
    out[68] = bias[68] + -0.023238864598874573 * state[24] + -0.3148342557928728 * state[34] + state[68] + 0.046477729197749146 * t_t0[24] + 0.6296685115857455 * t_t0[34];
    out[69] = bias[69] + -0.024432609086831557 * state[10] + -0.3148342557928728 * state[31] + -0.26432062531400063 * state[35] + -0.2046259837099301 * state[41] + state[69] + 0.048865218173663114 * t_t0[10] + 0.6296685115857455 * t_t0[31] + 0.5286412506280013 * t_t0[35] + 0.4092519674198602 * t_t0[41];
    out[70] = bias[70] + -0.09586520152267977 * state[21] + -0.31483425579287283 * state[25] + state[70] + 0.19173040304535954 * t_t0[21] + 0.6296685115857457 * t_t0[25];
    out[71] = bias[71] + -0.04219712274031408 * state[4] + -0.04147423462601573 * state[32] + -0.0669377754137904 * state[42] + -0.3148342557928728 * state[46] + state[71] + 0.08439424548062816 * t_t0[4] + 0.08294846925203146 * t_t0[32] + 0.1338755508275808 * t_t0[42] + 0.6296685115857455 * t_t0[46];
    out[72] = bias[72] + -0.029904608409446035 * state[33] + -0.2549419703939111 * state[43] + -0.3148342557928728 * state[47] + state[72] + 0.05980921681889207 * t_t0[33] + 0.5098839407878222 * t_t0[43] + 0.6296685115857455 * t_t0[47];
    out[73] = bias[73] + -0.14375068353499568 * state[11] + -0.31483425579287283 * state[15] + -0.03542869495158811 * state[22] + -0.31483425575411544 * state[36] + -0.3148342555778952 * state[48] + state[73] + 0.28750136706999135 * t_t0[11] + 0.6296685115857457 * t_t0[15] + 0.07085738990317622 * t_t0[22] + 0.6296685115082309 * t_t0[36] + 0.6296685111557904 * t_t0[48];
}

void fom_step_t0(const fom_float *state, const fom_float *bias, const FomAct_t0 *act, fom_float *out) {
    k_t0(state, bias, act->lo, act->hi, out);
}

void fom_step_t1(const fom_float *state, const fom_float *t_t0, const fom_float *bias, const FomAct_t1 *act, fom_float *out) {
    k_t1(state, t_t0, bias, act->lo, out);
}

void fom_step_su(const fom_float *state, const fom_float *t_t0, const fom_float *t_t1, const fom_float *bias, fom_float *out) {
    fom_int i;
    for (i = 0; i < 49; ++i) out[i] = bias[i] + t_t0[i];
    for (i = 0; i < 10; ++i) out[49 + i] = bias[49 + i] + t_t1[i];
    k_su_rest(state, t_t0, t_t1, bias, out);
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
    k_su_rest(state, t_t0, t_t1, bias, out);
}

void fom_iterate(const fom_float *state, const fom_float *bias_t0, const fom_float *bias_t1, const fom_float *bias_su, const FomAct_t0 *act_t0, const FomAct_t1 *act_t1, fom_float *t_t0, fom_float *t_t1, fom_float *new_state) {
    /* t1_t_t0 = -2 t1_state[:, :49] and su_t_t0 = -2 su_state[:, :49] (rest rows):
       both blocks only see w = state[:49] - 2 t0, the extrapolated primal */
    const fom_float *lo1 = act_t1->lo;
    fom_float w[49];
    fom_float v;
    fom_int i;
    k_t0(state, bias_t0, act_t0->lo, act_t0->hi, t_t0);
    for (i = 0; i < 49; ++i) w[i] = state[i] - 2.0 * t_t0[i];
    v = bias_t1[0] + state[49] + -0.3148342557928728 * w[0] + -0.1785106319392791 * w[1] + -0.3148342556510004 * w[2] + -0.3148342557928728 * w[3] + -0.3148342557928728 * w[4]; t_t1[0] = v < lo1[0] ? lo1[0] : v;
    v = bias_t1[1] + state[50] + -0.3148342557928728 * w[5] + -0.1149099042416003 * w[6] + -0.11245022456106747 * w[7] + -0.3148342557928728 * w[8] + -0.1012882957183817 * w[9] + -0.3148342557928728 * w[10] + -0.31483425576776075 * w[11]; t_t1[1] = v < lo1[1] ? lo1[1] : v;
    v = bias_t1[2] + state[51] + -0.16283459527310534 * w[12] + -0.3148342557928728 * w[13] + -0.3148342557928728 * w[14] + -0.13195167489181564 * w[15]; t_t1[2] = v < lo1[2] ? lo1[2] : v;
    v = bias_t1[3] + state[52] + -0.20484874346487622 * w[16] + -0.3148342557928728 * w[17] + -0.3148342557928728 * w[18] + -0.2013575399297713 * w[19] + -0.1717002983294537 * w[20] + -0.3148342557928728 * w[21] + -0.3148342557928728 * w[22]; t_t1[3] = v < lo1[3] ? lo1[3] : v;
    v = bias_t1[4] + state[53] + -0.11549111655165208 * w[23] + -0.3148342557928728 * w[24] + -0.22703157183385472 * w[25]; t_t1[4] = v < lo1[4] ? lo1[4] : v;
    v = bias_t1[5] + state[54] + -0.3148342557928728 * w[26] + -0.31483425578928326 * w[27] + -0.11812566027056481 * w[28] + -0.3148342557928728 * w[29] + -0.3148342557928728 * w[30] + -0.11621549871906076 * w[31] + -0.3148342557928728 * w[32] + -0.3148342557928728 * w[33]; t_t1[5] = v < lo1[5] ? lo1[5] : v;
    v = bias_t1[6] + state[55] + -0.2007269945571799 * w[34] + -0.31483425579287283 * w[35] + -0.20810320529794363 * w[36]; t_t1[6] = v < lo1[6] ? lo1[6] : v;
    v = bias_t1[7] + state[56] + -0.3148342557928728 * w[37] + -0.3148342557928728 * w[38]; t_t1[7] = v < lo1[7] ? lo1[7] : v;
    v = bias_t1[8] + state[57] + -0.14832333169428388 * w[39] + -0.25687797426285774 * w[40] + -0.3148342556269916 * w[41] + -0.3148342557928728 * w[42] + -0.31483425573999474 * w[43]; t_t1[8] = v < lo1[8] ? lo1[8] : v;
    v = bias_t1[9] + state[58] + -0.3148342557928728 * w[44] + -0.2409314392941177 * w[45] + -0.15802369728139173 * w[46] + -0.21287292492070603 * w[47] + -0.2746827012680003 * w[48]; t_t1[9] = v < lo1[9] ? lo1[9] : v;

    if (new_state != t_t0) {
        for (i = 0; i < 49; ++i) new_state[i] = bias_su[i] + t_t0[i];
        for (i = 0; i < 10; ++i) new_state[49 + i] = bias_su[49 + i] + t_t1[i];
    }
    new_state[59] = bias_su[59] + state[59] + -0.1255906893098709 * w[0] + -0.3148342557655042 * w[12] + -0.31483425569820117 * w[16] + -0.06685962708718948 * w[26] + -0.3148342557928728 * w[39];
    new_state[60] = bias_su[60] + state[60] + -0.31483425579287283 * w[1] + -0.08155612991849112 * w[5] + -0.1807094243289255 * w[27] + -0.050057390438124606 * w[37];
    new_state[61] = bias_su[61] + state[61] + -0.31483425579287283 * w[6] + -0.059210692177281046 * w[17];
    new_state[62] = bias_su[62] + state[62] + -0.18736322506593048 * w[2] + -0.3148342557928728 * w[23];
    new_state[63] = bias_su[63] + state[63] + -0.057878934426039565 * w[13] + -0.04186021818779605 * w[18] + -0.31483425579287283 * w[28];
    new_state[64] = bias_su[64] + state[64] + -0.3148342557928728 * w[7] + -0.3148342556220557 * w[19] + -0.06983942457332966 * w[29];
    new_state[65] = bias_su[65] + state[65] + -0.05210541922399308 * w[3] + -0.07906843296231456 * w[8] + -0.020371967614988162 * w[14] + -0.31483425579287283 * w[20] + -0.063379996679072 * w[44];
    new_state[66] = bias_su[66] + state[66] + -0.3148342557928728 * w[9] + -0.01829049354312028 * w[30] + -0.022355820755170213 * w[38];
    new_state[67] = bias_su[67] + state[67] + -0.3148342557740812 * w[40] + -0.31483425579287283 * w[45];
    new_state[68] = bias_su[68] + state[68] + -0.023238864598874573 * w[24] + -0.3148342557928728 * w[34];
    new_state[69] = bias_su[69] + state[69] + -0.024432609086831557 * w[10] + -0.3148342557928728 * w[31] + -0.26432062531400063 * w[35] + -0.2046259837099301 * w[41];
    new_state[70] = bias_su[70] + state[70] + -0.09586520152267977 * w[21] + -0.31483425579287283 * w[25];
    new_state[71] = bias_su[71] + state[71] + -0.04219712274031408 * w[4] + -0.04147423462601573 * w[32] + -0.0669377754137904 * w[42] + -0.3148342557928728 * w[46];
    new_state[72] = bias_su[72] + state[72] + -0.029904608409446035 * w[33] + -0.2549419703939111 * w[43] + -0.3148342557928728 * w[47];
    new_state[73] = bias_su[73] + state[73] + -0.14375068353499568 * w[11] + -0.31483425579287283 * w[15] + -0.03542869495158811 * w[22] + -0.31483425575411544 * w[36] + -0.3148342555778952 * w[48];
}
