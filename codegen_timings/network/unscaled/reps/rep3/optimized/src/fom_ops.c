#include <math.h>
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

void fom_iterate(const fom_float *restrict state, const fom_float *restrict bias_t0, const fom_float *restrict bias_t1, const fom_float *restrict bias_su, const FomAct_t0 *act_t0, const FomAct_t1 *act_t1, fom_float *restrict t_t0, fom_float *restrict t_t1, fom_float *restrict new_state) {
    const fom_float *restrict lo0 = act_t0->lo, *restrict hi0 = act_t0->hi, *restrict lo1 = act_t1->lo;
    fom_float v;
    fom_float w[49];
    v = bias_t0[0] + state[0] - (0.08226303133234722) * state[49] - (0.069655061626067494) * state[59];
    v = fmin(fmax(v, lo0[0]), hi0[0]); t_t0[0] = v; w[0] = (2.0 * v - state[0]); new_state[0] = bias_su[0] + v;
    v = bias_t0[1] + state[1] - (0.08226303133234722) * state[49] - (0.25588244633413909) * state[60];
    v = fmin(fmax(v, lo0[1]), hi0[1]); t_t0[1] = v; w[1] = (2.0 * v - state[1]); new_state[1] = bias_su[1] + v;
    v = bias_t0[2] + state[2] - (0.08226303133234722) * state[49] - (0.13345673088354346) * state[62];
    v = fmin(fmax(v, lo0[2]), hi0[2]); t_t0[2] = v; w[2] = (2.0 * v - state[2]); new_state[2] = bias_su[2] + v;
    v = bias_t0[3] + state[3] - (0.08226303133234722) * state[49] - (0.024964136788999065) * state[65];
    v = fmin(fmax(v, lo0[3]), hi0[3]); t_t0[3] = v; w[3] = (2.0 * v - state[3]); new_state[3] = bias_su[3] + v;
    v = bias_t0[4] + state[4] - (0.08226303133234722) * state[49] - (0.021966725812901995) * state[71];
    v = fmin(fmax(v, lo0[4]), hi0[4]); t_t0[4] = v; w[4] = (2.0 * v - state[4]); new_state[4] = bias_su[4] + v;
    v = bias_t0[5] + state[5] - (0.08226303133234722) * state[50] - (0.037583500758161678) * state[60];
    v = fmin(fmax(v, lo0[5]), hi0[5]); t_t0[5] = v; w[5] = (2.0 * v - state[5]); new_state[5] = bias_su[5] + v;
    v = bias_t0[6] + state[6] - (0.08226303133234722) * state[50] - (0.61752386121652636) * state[61];
    v = fmin(fmax(v, lo0[6]), hi0[6]); t_t0[6] = v; w[6] = (2.0 * v - state[6]); new_state[6] = bias_su[6] + v;
    v = bias_t0[7] + state[7] - (0.08226303133234722) * state[50] - (0.64483412842709398) * state[64];
    v = fmin(fmax(v, lo0[7]), hi0[7]); t_t0[7] = v; w[7] = (2.0 * v - state[7]); new_state[7] = bias_su[7] + v;
    v = bias_t0[8] + state[8] - (0.08226303133234722) * state[50] - (0.037882339410371904) * state[65];
    v = fmin(fmax(v, lo0[8]), hi0[8]); t_t0[8] = v; w[8] = (2.0 * v - state[8]); new_state[8] = bias_su[8] + v;
    v = bias_t0[9] + state[9] - (0.08226303133234722) * state[50] - (0.79478587733064987) * state[66];
    v = fmin(fmax(v, lo0[9]), hi0[9]); t_t0[9] = v; w[9] = (2.0 * v - state[9]); new_state[9] = bias_su[9] + v;
    v = bias_t0[10] + state[10] - (0.08226303133234722) * state[50] - (0.017294599334807721) * state[69];
    v = fmin(fmax(v, lo0[10]), hi0[10]); t_t0[10] = v; w[10] = (2.0 * v - state[10]); new_state[10] = bias_su[10] + v;
    v = bias_t0[11] + state[11] - (0.08226303133234722) * state[50] - (0.089618922945269686) * state[73];
    v = fmin(fmax(v, lo0[11]), hi0[11]); t_t0[11] = v; w[11] = (2.0 * v - state[11]); new_state[11] = bias_su[11] + v;
    v = bias_t0[12] + state[12] - (0.08226303133234722) * state[51] - (0.33760783556686547) * state[59];
    v = fmin(fmax(v, lo0[12]), hi0[12]); t_t0[12] = v; w[12] = (2.0 * v - state[12]); new_state[12] = bias_su[12] + v;
    v = bias_t0[13] + state[13] - (0.08226303133234722) * state[51] - (0.040307047471874392) * state[63];
    v = fmin(fmax(v, lo0[13]), hi0[13]); t_t0[13] = v; w[13] = (2.0 * v - state[13]); new_state[13] = bias_su[13] + v;
    v = bias_t0[14] + state[14] - (0.08226303133234722) * state[51] - (0.00976037797556846) * state[65];
    v = fmin(fmax(v, lo0[14]), hi0[14]); t_t0[14] = v; w[14] = (2.0 * v - state[14]); new_state[14] = bias_su[14] + v;
    v = bias_t0[15] + state[15] - (0.08226303133234722) * state[51] - (0.46831584781193331) * state[73];
    v = fmin(fmax(v, lo0[15]), hi0[15]); t_t0[15] = v; w[15] = (2.0 * v - state[15]); new_state[15] = bias_su[15] + v;
    v = bias_t0[16] + state[16] - (0.08226303133234722) * state[52] - (0.26836501080727221) * state[59];
    v = fmin(fmax(v, lo0[16]), hi0[16]); t_t0[16] = v; w[16] = (2.0 * v - state[16]); new_state[16] = bias_su[16] + v;
    v = bias_t0[17] + state[17] - (0.08226303133234722) * state[52] - (0.042388435165245451) * state[61];
    v = fmin(fmax(v, lo0[17]), hi0[17]); t_t0[17] = v; w[17] = (2.0 * v - state[17]); new_state[17] = bias_su[17] + v;
    v = bias_t0[18] + state[18] - (0.08226303133234722) * state[52] - (0.029151569883073398) * state[63];
    v = fmin(fmax(v, lo0[18]), hi0[18]); t_t0[18] = v; w[18] = (2.0 * v - state[18]); new_state[18] = bias_su[18] + v;
    v = bias_t0[19] + state[19] - (0.08226303133234722) * state[52] - (0.36011436438990652) * state[64];
    v = fmin(fmax(v, lo0[19]), hi0[19]); t_t0[19] = v; w[19] = (2.0 * v - state[19]); new_state[19] = bias_su[19] + v;
    v = bias_t0[20] + state[20] - (0.08226303133234722) * state[52] - (0.27658369193168919) * state[65];
    v = fmin(fmax(v, lo0[20]), hi0[20]); t_t0[20] = v; w[20] = (2.0 * v - state[20]); new_state[20] = bias_su[20] + v;
    v = bias_t0[21] + state[21] - (0.08226303133234722) * state[52] - (0.034735970917354191) * state[70];
    v = fmin(fmax(v, lo0[21]), hi0[21]); t_t0[21] = v; w[21] = (2.0 * v - state[21]); new_state[21] = bias_su[21] + v;
    v = bias_t0[22] + state[22] - (0.08226303133234722) * state[52] - (0.022087418331416996) * state[73];
    v = fmin(fmax(v, lo0[22]), hi0[22]); t_t0[22] = v; w[22] = (2.0 * v - state[22]); new_state[22] = bias_su[22] + v;
    v = bias_t0[23] + state[23] - (0.08226303133234722) * state[53] - (0.61132408824693363) * state[62];
    v = fmin(fmax(v, lo0[23]), hi0[23]); t_t0[23] = v; w[23] = (2.0 * v - state[23]); new_state[23] = bias_su[23] + v;
    v = bias_t0[24] + state[24] - (0.08226303133234722) * state[53] - (0.013160484266981857) * state[68];
    v = fmin(fmax(v, lo0[24]), hi0[24]); t_t0[24] = v; w[24] = (2.0 * v - state[24]); new_state[24] = bias_su[24] + v;
    v = bias_t0[25] + state[25] - (0.08226303133234722) * state[53] - (0.15819624000738933) * state[70];
    v = fmin(fmax(v, lo0[25]), hi0[25]); t_t0[25] = v; w[25] = (2.0 * v - state[25]); new_state[25] = bias_su[25] + v;
    v = bias_t0[26] + state[26] - (0.08226303133234722) * state[54] - (0.037081661631488835) * state[59];
    v = fmin(fmax(v, lo0[26]), hi0[26]); t_t0[26] = v; w[26] = (2.0 * v - state[26]); new_state[26] = bias_su[26] + v;
    v = bias_t0[27] + state[27] - (0.08226303133234722) * state[54] - (0.083276300544647269) * state[60];
    v = fmin(fmax(v, lo0[27]), hi0[27]); t_t0[27] = v; w[27] = (2.0 * v - state[27]); new_state[27] = bias_su[27] + v;
    v = bias_t0[28] + state[28] - (0.08226303133234722) * state[54] - (0.58435958348428074) * state[63];
    v = fmin(fmax(v, lo0[28]), hi0[28]); t_t0[28] = v; w[28] = (2.0 * v - state[28]); new_state[28] = bias_su[28] + v;
    v = bias_t0[29] + state[29] - (0.08226303133234722) * state[54] - (0.051091074244933282) * state[64];
    v = fmin(fmax(v, lo0[29]), hi0[29]); t_t0[29] = v; w[29] = (2.0 * v - state[29]); new_state[29] = bias_su[29] + v;
    v = bias_t0[30] + state[30] - (0.08226303133234722) * state[54] - (0.014854938892497713) * state[66];
    v = fmin(fmax(v, lo0[30]), hi0[30]); t_t0[30] = v; w[30] = (2.0 * v - state[30]); new_state[30] = bias_su[30] + v;
    v = bias_t0[31] + state[31] - (0.08226303133234722) * state[54] - (0.60372695765338591) * state[69];
    v = fmin(fmax(v, lo0[31]), hi0[31]); t_t0[31] = v; w[31] = (2.0 * v - state[31]); new_state[31] = bias_su[31] + v;
    v = bias_t0[32] + state[32] - (0.08226303133234722) * state[54] - (0.021590409041307837) * state[71];
    v = fmin(fmax(v, lo0[32]), hi0[32]); t_t0[32] = v; w[32] = (2.0 * v - state[32]); new_state[32] = bias_su[32] + v;
    v = bias_t0[33] + state[33] - (0.08226303133234722) * state[54] - (0.011556395626565408) * state[72];
    v = fmin(fmax(v, lo0[33]), hi0[33]); t_t0[33] = v; w[33] = (2.0 * v - state[33]); new_state[33] = bias_su[33] + v;
    v = bias_t0[34] + state[34] - (0.08226303133234722) * state[55] - (0.38643208723267541) * state[68];
    v = fmin(fmax(v, lo0[34]), hi0[34]); t_t0[34] = v; w[34] = (2.0 * v - state[34]); new_state[34] = bias_su[34] + v;
    v = bias_t0[35] + state[35] - (0.08226303133234722) * state[55] - (0.25854119370877737) * state[69];
    v = fmin(fmax(v, lo0[35]), hi0[35]); t_t0[35] = v; w[35] = (2.0 * v - state[35]); new_state[35] = bias_su[35] + v;
    v = bias_t0[36] + state[36] - (0.08226303133234722) * state[55] - (0.41032977319887487) * state[73];
    v = fmin(fmax(v, lo0[36]), hi0[36]); t_t0[36] = v; w[36] = (2.0 * v - state[36]); new_state[36] = bias_su[36] + v;
    v = bias_t0[37] + state[37] - (0.08226303133234722) * state[56] - (0.023067940734351828) * state[60];
    v = fmin(fmax(v, lo0[37]), hi0[37]); t_t0[37] = v; w[37] = (2.0 * v - state[37]); new_state[37] = bias_su[37] + v;
    v = bias_t0[38] + state[38] - (0.08226303133234722) * state[56] - (0.018156664303604776) * state[66];
    v = fmin(fmax(v, lo0[38]), hi0[38]); t_t0[38] = v; w[38] = (2.0 * v - state[38]); new_state[38] = bias_su[38] + v;
    v = bias_t0[39] + state[39] - (0.08226303133234722) * state[57] - (0.37063781296154064) * state[59];
    v = fmin(fmax(v, lo0[39]), hi0[39]); t_t0[39] = v; w[39] = (2.0 * v - state[39]); new_state[39] = bias_su[39] + v;
    v = bias_t0[40] + state[40] - (0.08226303133234722) * state[57] - (0.13174930294003487) * state[67];
    v = fmin(fmax(v, lo0[40]), hi0[40]); t_t0[40] = v; w[40] = (2.0 * v - state[40]); new_state[40] = bias_su[40] + v;
    v = bias_t0[41] + state[41] - (0.08226303133234722) * state[57] - (0.14484430995648803) * state[69];
    v = fmin(fmax(v, lo0[41]), hi0[41]); t_t0[41] = v; w[41] = (2.0 * v - state[41]); new_state[41] = bias_su[41] + v;
    v = bias_t0[42] + state[42] - (0.08226303133234722) * state[57] - (0.034846066829944282) * state[71];
    v = fmin(fmax(v, lo0[42]), hi0[42]); t_t0[42] = v; w[42] = (2.0 * v - state[42]); new_state[42] = bias_su[42] + v;
    v = bias_t0[43] + state[43] - (0.08226303133234722) * state[57] - (0.098520275933536933) * state[72];
    v = fmin(fmax(v, lo0[43]), hi0[43]); t_t0[43] = v; w[43] = (2.0 * v - state[43]); new_state[43] = bias_su[43] + v;
    v = bias_t0[44] + state[44] - (0.08226303133234722) * state[58] - (0.03036587998612799) * state[65];
    v = fmin(fmax(v, lo0[44]), hi0[44]); t_t0[44] = v; w[44] = (2.0 * v - state[44]); new_state[44] = bias_su[44] + v;
    v = bias_t0[45] + state[45] - (0.08226303133234722) * state[58] - (0.14046939723165486) * state[67];
    v = fmin(fmax(v, lo0[45]), hi0[45]); t_t0[45] = v; w[45] = (2.0 * v - state[45]); new_state[45] = bias_su[45] + v;
    v = bias_t0[46] + state[46] - (0.08226303133234722) * state[58] - (0.32653085943273191) * state[71];
    v = fmin(fmax(v, lo0[46]), hi0[46]); t_t0[46] = v; w[46] = (2.0 * v - state[46]); new_state[46] = bias_su[46] + v;
    v = bias_t0[47] + state[47] - (0.08226303133234722) * state[58] - (0.17994004051645768) * state[72];
    v = fmin(fmax(v, lo0[47]), hi0[47]); t_t0[47] = v; w[47] = (2.0 * v - state[47]); new_state[47] = bias_su[47] + v;
    v = bias_t0[48] + state[48] - (0.08226303133234722) * state[58] - (0.22496888289546163) * state[73];
    v = fmin(fmax(v, lo0[48]), hi0[48]); t_t0[48] = v; w[48] = (2.0 * v - state[48]); new_state[48] = bias_su[48] + v;
    v = bias_t1[0] + state[49] + (0.08226303133234722) * (((w[0] + w[1]) + (w[2] + w[3])) + w[4]);
    v = fmax(v, lo1[0]); t_t1[0] = v; new_state[49] = bias_su[49] + v;
    v = bias_t1[1] + state[50] + (0.08226303133234722) * (((w[5] + w[6]) + (w[7] + w[8])) + ((w[9] + w[10]) + w[11]));
    v = fmax(v, lo1[1]); t_t1[1] = v; new_state[50] = bias_su[50] + v;
    v = bias_t1[2] + state[51] + (0.08226303133234722) * ((w[12] + w[13]) + (w[14] + w[15]));
    v = fmax(v, lo1[2]); t_t1[2] = v; new_state[51] = bias_su[51] + v;
    v = bias_t1[3] + state[52] + (0.08226303133234722) * (((w[16] + w[17]) + (w[18] + w[19])) + ((w[20] + w[21]) + w[22]));
    v = fmax(v, lo1[3]); t_t1[3] = v; new_state[52] = bias_su[52] + v;
    v = bias_t1[4] + state[53] + (0.08226303133234722) * ((w[23] + w[24]) + w[25]);
    v = fmax(v, lo1[4]); t_t1[4] = v; new_state[53] = bias_su[53] + v;
    v = bias_t1[5] + state[54] + (0.08226303133234722) * (((w[26] + w[27]) + (w[28] + w[29])) + ((w[30] + w[31]) + (w[32] + w[33])));
    v = fmax(v, lo1[5]); t_t1[5] = v; new_state[54] = bias_su[54] + v;
    v = bias_t1[6] + state[55] + (0.08226303133234722) * ((w[34] + w[35]) + w[36]);
    v = fmax(v, lo1[6]); t_t1[6] = v; new_state[55] = bias_su[55] + v;
    v = bias_t1[7] + state[56] + (0.08226303133234722) * (w[37] + w[38]);
    v = fmax(v, lo1[7]); t_t1[7] = v; new_state[56] = bias_su[56] + v;
    v = bias_t1[8] + state[57] + (0.08226303133234722) * (((w[39] + w[40]) + (w[41] + w[42])) + w[43]);
    v = fmax(v, lo1[8]); t_t1[8] = v; new_state[57] = bias_su[57] + v;
    v = bias_t1[9] + state[58] + (0.08226303133234722) * (((w[44] + w[45]) + (w[46] + w[47])) + w[48]);
    v = fmax(v, lo1[9]); t_t1[9] = v; new_state[58] = bias_su[58] + v;
    new_state[59] = (bias_su[59] + state[59]) + ((((0.069655061626067494) * w[0] + (0.33760783556686547) * w[12]) + ((0.26836501080727221) * w[16] + (0.037081661631488835) * w[26])) + (0.37063781296154064) * w[39]);
    new_state[60] = (bias_su[60] + state[60]) + (((0.25588244633413909) * w[1] + (0.037583500758161678) * w[5]) + ((0.083276300544647269) * w[27] + (0.023067940734351828) * w[37]));
    new_state[61] = (bias_su[61] + state[61]) + ((0.61752386121652636) * w[6] + (0.042388435165245451) * w[17]);
    new_state[62] = (bias_su[62] + state[62]) + ((0.13345673088354346) * w[2] + (0.61132408824693363) * w[23]);
    new_state[63] = (bias_su[63] + state[63]) + (((0.040307047471874392) * w[13] + (0.029151569883073398) * w[18]) + (0.58435958348428074) * w[28]);
    new_state[64] = (bias_su[64] + state[64]) + (((0.64483412842709398) * w[7] + (0.36011436438990652) * w[19]) + (0.051091074244933282) * w[29]);
    new_state[65] = (bias_su[65] + state[65]) + ((((0.024964136788999065) * w[3] + (0.037882339410371904) * w[8]) + ((0.00976037797556846) * w[14] + (0.27658369193168919) * w[20])) + (0.03036587998612799) * w[44]);
    new_state[66] = (bias_su[66] + state[66]) + (((0.79478587733064987) * w[9] + (0.014854938892497713) * w[30]) + (0.018156664303604776) * w[38]);
    new_state[67] = (bias_su[67] + state[67]) + ((0.13174930294003487) * w[40] + (0.14046939723165486) * w[45]);
    new_state[68] = (bias_su[68] + state[68]) + ((0.013160484266981857) * w[24] + (0.38643208723267541) * w[34]);
    new_state[69] = (bias_su[69] + state[69]) + (((0.017294599334807721) * w[10] + (0.60372695765338591) * w[31]) + ((0.25854119370877737) * w[35] + (0.14484430995648803) * w[41]));
    new_state[70] = (bias_su[70] + state[70]) + ((0.034735970917354191) * w[21] + (0.15819624000738933) * w[25]);
    new_state[71] = (bias_su[71] + state[71]) + (((0.021966725812901995) * w[4] + (0.021590409041307837) * w[32]) + ((0.034846066829944282) * w[42] + (0.32653085943273191) * w[46]));
    new_state[72] = (bias_su[72] + state[72]) + (((0.011556395626565408) * w[33] + (0.098520275933536933) * w[43]) + (0.17994004051645768) * w[47]);
    new_state[73] = (bias_su[73] + state[73]) + ((((0.089618922945269686) * w[11] + (0.46831584781193331) * w[15]) + ((0.022087418331416996) * w[22] + (0.41032977319887487) * w[36])) + (0.22496888289546163) * w[48]);
}
