#include "fom_ops.h"
#include "fom_data.h"

void fom_apply_t0_state(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 48; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t0_state_ROWPTR[i]; p < FOM_MAT_t0_state_ROWPTR[i + 1]; ++p) s += FOM_MAT_t0_state_VALS[p] * x[FOM_MAT_t0_state_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t0_state__d_taux(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 48; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t0_state__d_taux_ROWPTR[i]; p < FOM_MAT_t0_state__d_taux_ROWPTR[i + 1]; ++p) s += FOM_MAT_t0_state__d_taux_VALS[p] * x[FOM_MAT_t0_state__d_taux_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t0_state__d_tth(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 48; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t0_state__d_tth_ROWPTR[i]; p < FOM_MAT_t0_state__d_tth_ROWPTR[i + 1]; ++p) s += FOM_MAT_t0_state__d_tth_VALS[p] * x[FOM_MAT_t0_state__d_tth_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t0_p_p1(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 48; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t0_p_p1_ROWPTR[i]; p < FOM_MAT_t0_p_p1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t0_p_p1_VALS[p] * x[FOM_MAT_t0_p_p1_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t0_p_p1__d_tau(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 48; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t0_p_p1__d_tau_ROWPTR[i]; p < FOM_MAT_t0_p_p1__d_tau_ROWPTR[i + 1]; ++p) s += FOM_MAT_t0_p_p1__d_tau_VALS[p] * x[FOM_MAT_t0_p_p1__d_tau_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t1_state(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 16; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t1_state_ROWPTR[i]; p < FOM_MAT_t1_state_ROWPTR[i + 1]; ++p) s += FOM_MAT_t1_state_VALS[p] * x[FOM_MAT_t1_state_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t1_state__d_eta(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 16; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t1_state__d_eta_ROWPTR[i]; p < FOM_MAT_t1_state__d_eta_ROWPTR[i + 1]; ++p) s += FOM_MAT_t1_state__d_eta_VALS[p] * x[FOM_MAT_t1_state__d_eta_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t1_p_p0(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 16; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t1_p_p0_ROWPTR[i]; p < FOM_MAT_t1_p_p0_ROWPTR[i + 1]; ++p) s += FOM_MAT_t1_p_p0_VALS[p] * x[FOM_MAT_t1_p_p0_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t1_p_p0__d_eta(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 16; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t1_p_p0__d_eta_ROWPTR[i]; p < FOM_MAT_t1_p_p0__d_eta_ROWPTR[i + 1]; ++p) s += FOM_MAT_t1_p_p0__d_eta_VALS[p] * x[FOM_MAT_t1_p_p0__d_eta_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t1_t_t0(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 16; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t1_t_t0_ROWPTR[i]; p < FOM_MAT_t1_t_t0_ROWPTR[i + 1]; ++p) s += FOM_MAT_t1_t_t0_VALS[p] * x[FOM_MAT_t1_t_t0_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t1_t_t0__d_eta(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 16; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t1_t_t0__d_eta_ROWPTR[i]; p < FOM_MAT_t1_t_t0__d_eta_ROWPTR[i + 1]; ++p) s += FOM_MAT_t1_t_t0__d_eta_VALS[p] * x[FOM_MAT_t1_t_t0__d_eta_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_su_state(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 160; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_su_state_ROWPTR[i]; p < FOM_MAT_su_state_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_state_VALS[p] * x[FOM_MAT_su_state_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_su_state__d_d1(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 160; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_su_state__d_d1_ROWPTR[i]; p < FOM_MAT_su_state__d_d1_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_state__d_d1_VALS[p] * x[FOM_MAT_su_state__d_d1_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_su_state__d_g2(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 160; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_su_state__d_g2_ROWPTR[i]; p < FOM_MAT_su_state__d_g2_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_state__d_g2_VALS[p] * x[FOM_MAT_su_state__d_g2_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_su_state__d_m1(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 160; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_su_state__d_m1_ROWPTR[i]; p < FOM_MAT_su_state__d_m1_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_state__d_m1_VALS[p] * x[FOM_MAT_su_state__d_m1_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_su_t_t0(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 160; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_su_t_t0_ROWPTR[i]; p < FOM_MAT_su_t_t0_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t0_VALS[p] * x[FOM_MAT_su_t_t0_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_su_t_t0__d_d2(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 160; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_su_t_t0__d_d2_ROWPTR[i]; p < FOM_MAT_su_t_t0__d_d2_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t0__d_d2_VALS[p] * x[FOM_MAT_su_t_t0__d_d2_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_su_t_t0__d_g1(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 160; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_su_t_t0__d_g1_ROWPTR[i]; p < FOM_MAT_su_t_t0__d_g1_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t0__d_g1_VALS[p] * x[FOM_MAT_su_t_t0__d_g1_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_su_t_t1(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 160; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_su_t_t1_ROWPTR[i]; p < FOM_MAT_su_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t1_VALS[p] * x[FOM_MAT_su_t_t1_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_su_t_t1__d_d2(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 160; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_su_t_t1__d_d2_ROWPTR[i]; p < FOM_MAT_su_t_t1__d_d2_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t1__d_d2_VALS[p] * x[FOM_MAT_su_t_t1__d_d2_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_su_t_t1__d_g1(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 160; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_su_t_t1__d_g1_ROWPTR[i]; p < FOM_MAT_su_t_t1__d_g1_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t1__d_g1_VALS[p] * x[FOM_MAT_su_t_t1__d_g1_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_su_t_t1__d_m2(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 160; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_su_t_t1__d_m2_ROWPTR[i]; p < FOM_MAT_su_t_t1__d_m2_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t1__d_m2_VALS[p] * x[FOM_MAT_su_t_t1__d_m2_COLIDX[p]];
        out[i] += s;
    }
}

void fom_fused_t0(const fom_float *state, fom_float sc_taux, fom_float sc_tth, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 48; ++i) {
        fom_float s;
        s = 0.0;
        for (p = FOM_MAT_t0_state_ROWPTR[i]; p < FOM_MAT_t0_state_ROWPTR[i + 1]; ++p) s += FOM_MAT_t0_state_VALS[p] * state[FOM_MAT_t0_state_COLIDX[p]];
        out[i] += s;
        s = 0.0;
        for (p = FOM_MAT_t0_state__d_taux_ROWPTR[i]; p < FOM_MAT_t0_state__d_taux_ROWPTR[i + 1]; ++p) s += FOM_MAT_t0_state__d_taux_VALS[p] * state[FOM_MAT_t0_state__d_taux_COLIDX[p]];
        out[i] += sc_taux * s;
        s = 0.0;
        for (p = FOM_MAT_t0_state__d_tth_ROWPTR[i]; p < FOM_MAT_t0_state__d_tth_ROWPTR[i + 1]; ++p) s += FOM_MAT_t0_state__d_tth_VALS[p] * state[FOM_MAT_t0_state__d_tth_COLIDX[p]];
        out[i] += sc_tth * s;
    }
}

void fom_fused_t1(const fom_float *state, const fom_float *t_t0, fom_float sc_eta, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 16; ++i) {
        fom_float s;
        s = 0.0;
        for (p = FOM_MAT_t1_state_ROWPTR[i]; p < FOM_MAT_t1_state_ROWPTR[i + 1]; ++p) s += FOM_MAT_t1_state_VALS[p] * state[FOM_MAT_t1_state_COLIDX[p]];
        out[i] += s;
        s = 0.0;
        /* t1_state__d_eta is a dense 16x16 block on state columns 144..159. */
        const fom_float *w = FOM_MAT_t1_state__d_eta_VALS + 16 * i;
        fom_int j;
        for (j = 0; j < 16; ++j) s += w[j] * state[144 + j];
        out[i] += sc_eta * s;
        s = 0.0;
        for (p = FOM_MAT_t1_t_t0__d_eta_ROWPTR[i]; p < FOM_MAT_t1_t_t0__d_eta_ROWPTR[i + 1]; ++p) s += FOM_MAT_t1_t_t0__d_eta_VALS[p] * t_t0[FOM_MAT_t1_t_t0__d_eta_COLIDX[p]];
        out[i] += sc_eta * s;
    }
}

void fom_fused_su(const fom_float *state, const fom_float *t_t0, const fom_float *t_t1, fom_float sc_d1, fom_float sc_g2, fom_float sc_m1, fom_float sc_d2, fom_float sc_g1, fom_float sc_m2, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 160; ++i) {
        fom_float s;
        s = 0.0;
        for (p = FOM_MAT_su_state_ROWPTR[i]; p < FOM_MAT_su_state_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_state_VALS[p] * state[FOM_MAT_su_state_COLIDX[p]];
        out[i] += s;
        s = 0.0;
        for (p = FOM_MAT_su_state__d_d1_ROWPTR[i]; p < FOM_MAT_su_state__d_d1_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_state__d_d1_VALS[p] * state[FOM_MAT_su_state__d_d1_COLIDX[p]];
        out[i] += sc_d1 * s;
        s = 0.0;
        for (p = FOM_MAT_su_state__d_g2_ROWPTR[i]; p < FOM_MAT_su_state__d_g2_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_state__d_g2_VALS[p] * state[FOM_MAT_su_state__d_g2_COLIDX[p]];
        out[i] += sc_g2 * s;
        s = 0.0;
        for (p = FOM_MAT_su_state__d_m1_ROWPTR[i]; p < FOM_MAT_su_state__d_m1_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_state__d_m1_VALS[p] * state[FOM_MAT_su_state__d_m1_COLIDX[p]];
        out[i] += sc_m1 * s;
        s = 0.0;
        for (p = FOM_MAT_su_t_t0__d_d2_ROWPTR[i]; p < FOM_MAT_su_t_t0__d_d2_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t0__d_d2_VALS[p] * t_t0[FOM_MAT_su_t_t0__d_d2_COLIDX[p]];
        out[i] += sc_d2 * s;
        s = 0.0;
        for (p = FOM_MAT_su_t_t0__d_g1_ROWPTR[i]; p < FOM_MAT_su_t_t0__d_g1_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t0__d_g1_VALS[p] * t_t0[FOM_MAT_su_t_t0__d_g1_COLIDX[p]];
        out[i] += sc_g1 * s;
        s = 0.0;
        for (p = FOM_MAT_su_t_t1__d_d2_ROWPTR[i]; p < FOM_MAT_su_t_t1__d_d2_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t1__d_d2_VALS[p] * t_t1[FOM_MAT_su_t_t1__d_d2_COLIDX[p]];
        out[i] += sc_d2 * s;
        s = 0.0;
        for (p = FOM_MAT_su_t_t1__d_g1_ROWPTR[i]; p < FOM_MAT_su_t_t1__d_g1_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t1__d_g1_VALS[p] * t_t1[FOM_MAT_su_t_t1__d_g1_COLIDX[p]];
        out[i] += sc_g1 * s;
        s = 0.0;
        for (p = FOM_MAT_su_t_t1__d_m2_ROWPTR[i]; p < FOM_MAT_su_t_t1__d_m2_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t1__d_m2_VALS[p] * t_t1[FOM_MAT_su_t_t1__d_m2_COLIDX[p]];
        out[i] += sc_m2 * s;
    }
}

void fom_step_t0(const fom_float *state, fom_float sc_taux, fom_float sc_tth, const fom_float *bias, const FomAct_t0 *act, fom_float *out) {
    /* t0_state is the shift out[i] += state[i + 16]; the two scheduled
       members share one product on y (below). */
    fom_int i;
    /* __d_tth equals -__d_taux shifted to columns 128..143 (checked per
       member by verify), so both collapse to one product on
       y = sc_taux * state[0..15] - sc_tth * state[128..143]. */
    fom_float y[16];
    for (i = 0; i < 16; ++i) y[i] = sc_taux * state[i] - sc_tth * state[i + 128];
    /* Rows 0..31 of both scheduled members hold one entry each, in column
       i mod 16 (__d_taux) and 128 + i mod 16 (__d_tth), stored at p = i. */
    for (i = 0; i < 32; ++i) {
        const fom_int c = i & 15;
        fom_float v = bias[i] + state[i + 16] + FOM_MAT_t0_state__d_taux_VALS[i] * y[c];
        if (v < act->lo[i]) v = act->lo[i];
        out[i] = v;
    }
    const fom_float *TA = FOM_MAT_t0_state__d_taux_VALS;
    /* Rows 32..47: fixed sparsity of __d_taux unrolled. */
    { fom_float v = bias[32] + state[48] + (TA[32] * y[1] + TA[33] * y[3]); if (v < act->lo[32]) v = act->lo[32]; out[32] = v; }
    { fom_float v = bias[33] + state[49] + (TA[34] * y[0] + TA[35] * y[2]); if (v < act->lo[33]) v = act->lo[33]; out[33] = v; }
    { fom_float v = bias[34] + state[50] + (TA[36] * y[1] + TA[37] * y[3] + TA[38] * y[5] + TA[39] * y[7]); if (v < act->lo[34]) v = act->lo[34]; out[34] = v; }
    { fom_float v = bias[35] + state[51] + (TA[40] * y[0] + TA[41] * y[2] + TA[42] * y[4] + TA[43] * y[6]); if (v < act->lo[35]) v = act->lo[35]; out[35] = v; }
    { fom_float v = bias[36] + state[52] + (TA[44] * y[1] + TA[45] * y[3] + TA[46] * y[5] + TA[47] * y[7] + TA[48] * y[9] + TA[49] * y[11]); if (v < act->lo[36]) v = act->lo[36]; out[36] = v; }
    { fom_float v = bias[37] + state[53] + (TA[50] * y[0] + TA[51] * y[2] + TA[52] * y[4] + TA[53] * y[6] + TA[54] * y[8] + TA[55] * y[10]); if (v < act->lo[37]) v = act->lo[37]; out[37] = v; }
    { fom_float v = bias[38] + state[54] + (TA[56] * y[1] + TA[57] * y[3] + TA[58] * y[5] + TA[59] * y[7] + TA[60] * y[9] + TA[61] * y[11] + TA[62] * y[13] + TA[63] * y[15]); if (v < act->lo[38]) v = act->lo[38]; out[38] = v; }
    { fom_float v = bias[39] + state[55] + (TA[64] * y[0] + TA[65] * y[2] + TA[66] * y[4] + TA[67] * y[6] + TA[68] * y[8] + TA[69] * y[10] + TA[70] * y[12] + TA[71] * y[14]); if (v < act->lo[39]) v = act->lo[39]; out[39] = v; }
    { fom_float v = bias[40] + state[56] + (TA[72] * y[1] + TA[73] * y[3]); if (v < act->lo[40]) v = act->lo[40]; out[40] = v; }
    { fom_float v = bias[41] + state[57] + (TA[74] * y[0] + TA[75] * y[2]); if (v < act->lo[41]) v = act->lo[41]; out[41] = v; }
    { fom_float v = bias[42] + state[58] + (TA[76] * y[1] + TA[77] * y[3] + TA[78] * y[5] + TA[79] * y[7]); if (v < act->lo[42]) v = act->lo[42]; out[42] = v; }
    { fom_float v = bias[43] + state[59] + (TA[80] * y[0] + TA[81] * y[2] + TA[82] * y[4] + TA[83] * y[6]); if (v < act->lo[43]) v = act->lo[43]; out[43] = v; }
    { fom_float v = bias[44] + state[60] + (TA[84] * y[1] + TA[85] * y[3] + TA[86] * y[5] + TA[87] * y[7] + TA[88] * y[9] + TA[89] * y[11]); if (v < act->lo[44]) v = act->lo[44]; out[44] = v; }
    { fom_float v = bias[45] + state[61] + (TA[90] * y[0] + TA[91] * y[2] + TA[92] * y[4] + TA[93] * y[6] + TA[94] * y[8] + TA[95] * y[10]); if (v < act->lo[45]) v = act->lo[45]; out[45] = v; }
    { fom_float v = bias[46] + state[62] + (TA[96] * y[1] + TA[97] * y[3] + TA[98] * y[5] + TA[99] * y[7] + TA[100] * y[9] + TA[101] * y[11] + TA[102] * y[13] + TA[103] * y[15]); if (v < act->lo[46]) v = act->lo[46]; out[46] = v; }
    { fom_float v = bias[47] + state[63] + (TA[104] * y[0] + TA[105] * y[2] + TA[106] * y[4] + TA[107] * y[6] + TA[108] * y[8] + TA[109] * y[10] + TA[110] * y[12] + TA[111] * y[14]); if (v < act->lo[47]) v = act->lo[47]; out[47] = v; }
}

void fom_step_t1(const fom_float *state, const fom_float *t_t0, fom_float sc_eta, const fom_float *bias, const FomAct_t1 *act, fom_float *out) {
    /* t1_state is the shift out[i] += state[i]; both eta members share sc_eta.
       t1_state__d_eta is a dense 16x16 block on state columns 144..159. */
    const fom_float *TV = FOM_MAT_t1_t_t0__d_eta_VALS;
    fom_int i, j;
    fom_float u[16];
    /* t1_t_t0__d_eta: fixed sparsity unrolled. */
    u[0] = TV[0] * t_t0[0] + TV[1] * t_t0[16] + TV[2] * t_t0[33] + TV[3] * t_t0[35] + TV[4] * t_t0[37] + TV[5] * t_t0[39] + TV[6] * t_t0[41] + TV[7] * t_t0[43] + TV[8] * t_t0[45] + TV[9] * t_t0[47];
    u[1] = TV[10] * t_t0[1] + TV[11] * t_t0[17] + TV[12] * t_t0[32] + TV[13] * t_t0[34] + TV[14] * t_t0[36] + TV[15] * t_t0[38] + TV[16] * t_t0[40] + TV[17] * t_t0[42] + TV[18] * t_t0[44] + TV[19] * t_t0[46];
    u[2] = TV[20] * t_t0[2] + TV[21] * t_t0[18] + TV[22] * t_t0[33] + TV[23] * t_t0[35] + TV[24] * t_t0[37] + TV[25] * t_t0[39] + TV[26] * t_t0[41] + TV[27] * t_t0[43] + TV[28] * t_t0[45] + TV[29] * t_t0[47];
    u[3] = TV[30] * t_t0[3] + TV[31] * t_t0[19] + TV[32] * t_t0[32] + TV[33] * t_t0[34] + TV[34] * t_t0[36] + TV[35] * t_t0[38] + TV[36] * t_t0[40] + TV[37] * t_t0[42] + TV[38] * t_t0[44] + TV[39] * t_t0[46];
    u[4] = TV[40] * t_t0[4] + TV[41] * t_t0[20] + TV[42] * t_t0[35] + TV[43] * t_t0[37] + TV[44] * t_t0[39] + TV[45] * t_t0[43] + TV[46] * t_t0[45] + TV[47] * t_t0[47];
    u[5] = TV[48] * t_t0[5] + TV[49] * t_t0[21] + TV[50] * t_t0[34] + TV[51] * t_t0[36] + TV[52] * t_t0[38] + TV[53] * t_t0[42] + TV[54] * t_t0[44] + TV[55] * t_t0[46];
    u[6] = TV[56] * t_t0[6] + TV[57] * t_t0[22] + TV[58] * t_t0[35] + TV[59] * t_t0[37] + TV[60] * t_t0[39] + TV[61] * t_t0[43] + TV[62] * t_t0[45] + TV[63] * t_t0[47];
    u[7] = TV[64] * t_t0[7] + TV[65] * t_t0[23] + TV[66] * t_t0[34] + TV[67] * t_t0[36] + TV[68] * t_t0[38] + TV[69] * t_t0[42] + TV[70] * t_t0[44] + TV[71] * t_t0[46];
    u[8] = TV[72] * t_t0[8] + TV[73] * t_t0[24] + TV[74] * t_t0[37] + TV[75] * t_t0[39] + TV[76] * t_t0[45] + TV[77] * t_t0[47];
    u[9] = TV[78] * t_t0[9] + TV[79] * t_t0[25] + TV[80] * t_t0[36] + TV[81] * t_t0[38] + TV[82] * t_t0[44] + TV[83] * t_t0[46];
    u[10] = TV[84] * t_t0[10] + TV[85] * t_t0[26] + TV[86] * t_t0[37] + TV[87] * t_t0[39] + TV[88] * t_t0[45] + TV[89] * t_t0[47];
    u[11] = TV[90] * t_t0[11] + TV[91] * t_t0[27] + TV[92] * t_t0[36] + TV[93] * t_t0[38] + TV[94] * t_t0[44] + TV[95] * t_t0[46];
    u[12] = TV[96] * t_t0[12] + TV[97] * t_t0[28] + TV[98] * t_t0[39] + TV[99] * t_t0[47];
    u[13] = TV[100] * t_t0[13] + TV[101] * t_t0[29] + TV[102] * t_t0[38] + TV[103] * t_t0[46];
    u[14] = TV[104] * t_t0[14] + TV[105] * t_t0[30] + TV[106] * t_t0[39] + TV[107] * t_t0[47];
    u[15] = TV[108] * t_t0[15] + TV[109] * t_t0[31] + TV[110] * t_t0[38] + TV[111] * t_t0[46];
    for (i = 0; i < 16; ++i) {
        const fom_float *w = FOM_MAT_t1_state__d_eta_VALS + 16 * i;
        fom_float s = u[i], v;
        for (j = 0; j < 16; ++j) s += w[j] * state[144 + j];
        v = bias[i] + state[i] + sc_eta * s;
        if (v < act->lo[i]) v = act->lo[i];
        if (v > act->hi[i]) v = act->hi[i];
        out[i] = v;
    }
}

void fom_step_su(const fom_float *state, const fom_float *t_t0, const fom_float *t_t1, fom_float sc_d1, fom_float sc_g2, fom_float sc_m1, fom_float sc_d2, fom_float sc_g1, fom_float sc_m2, const fom_float *bias, fom_float *out) {
    fom_int i, r, p;
    for (r = 0; r < 160; ++r) {
        fom_float s, v;
        i = r;
        v = bias[i];
        s = 0.0;
        for (p = FOM_MAT_su_state_ROWPTR[i]; p < FOM_MAT_su_state_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_state_VALS[p] * state[FOM_MAT_su_state_COLIDX[p]];
        v += s;
        s = 0.0;
        for (p = FOM_MAT_su_state__d_d1_ROWPTR[i]; p < FOM_MAT_su_state__d_d1_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_state__d_d1_VALS[p] * state[FOM_MAT_su_state__d_d1_COLIDX[p]];
        v += sc_d1 * s;
        s = 0.0;
        for (p = FOM_MAT_su_state__d_g2_ROWPTR[i]; p < FOM_MAT_su_state__d_g2_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_state__d_g2_VALS[p] * state[FOM_MAT_su_state__d_g2_COLIDX[p]];
        v += sc_g2 * s;
        s = 0.0;
        for (p = FOM_MAT_su_state__d_m1_ROWPTR[i]; p < FOM_MAT_su_state__d_m1_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_state__d_m1_VALS[p] * state[FOM_MAT_su_state__d_m1_COLIDX[p]];
        v += sc_m1 * s;
        s = 0.0;
        for (p = FOM_MAT_su_t_t0__d_d2_ROWPTR[i]; p < FOM_MAT_su_t_t0__d_d2_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t0__d_d2_VALS[p] * t_t0[FOM_MAT_su_t_t0__d_d2_COLIDX[p]];
        v += sc_d2 * s;
        s = 0.0;
        for (p = FOM_MAT_su_t_t0__d_g1_ROWPTR[i]; p < FOM_MAT_su_t_t0__d_g1_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t0__d_g1_VALS[p] * t_t0[FOM_MAT_su_t_t0__d_g1_COLIDX[p]];
        v += sc_g1 * s;
        s = 0.0;
        for (p = FOM_MAT_su_t_t1__d_d2_ROWPTR[i]; p < FOM_MAT_su_t_t1__d_d2_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t1__d_d2_VALS[p] * t_t1[FOM_MAT_su_t_t1__d_d2_COLIDX[p]];
        v += sc_d2 * s;
        s = 0.0;
        for (p = FOM_MAT_su_t_t1__d_g1_ROWPTR[i]; p < FOM_MAT_su_t_t1__d_g1_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t1__d_g1_VALS[p] * t_t1[FOM_MAT_su_t_t1__d_g1_COLIDX[p]];
        v += sc_g1 * s;
        s = 0.0;
        for (p = FOM_MAT_su_t_t1__d_m2_ROWPTR[i]; p < FOM_MAT_su_t_t1__d_m2_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t1__d_m2_VALS[p] * t_t1[FOM_MAT_su_t_t1__d_m2_COLIDX[p]];
        v += sc_m2 * s;
        out[i] = v;
    }
}

void fom_fused_su__rest(const fom_float *state, const fom_float *t_t0, const fom_float *t_t1, fom_float sc_d1, fom_float sc_g2, fom_float sc_m1, fom_float sc_d2, fom_float sc_g1, fom_float sc_m2, fom_float *out) {
    fom_int i, r, p;
    for (r = 0; r < FOM_REST_su_N; ++r) {
        fom_float s;
        i = FOM_REST_su_ROWS[r];
        s = 0.0;
        for (p = FOM_MAT_su_state_ROWPTR[i]; p < FOM_MAT_su_state_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_state_VALS[p] * state[FOM_MAT_su_state_COLIDX[p]];
        out[i] += s;
        s = 0.0;
        for (p = FOM_MAT_su_state__d_d1_ROWPTR[i]; p < FOM_MAT_su_state__d_d1_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_state__d_d1_VALS[p] * state[FOM_MAT_su_state__d_d1_COLIDX[p]];
        out[i] += sc_d1 * s;
        s = 0.0;
        for (p = FOM_MAT_su_state__d_g2_ROWPTR[i]; p < FOM_MAT_su_state__d_g2_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_state__d_g2_VALS[p] * state[FOM_MAT_su_state__d_g2_COLIDX[p]];
        out[i] += sc_g2 * s;
        s = 0.0;
        for (p = FOM_MAT_su_state__d_m1_ROWPTR[i]; p < FOM_MAT_su_state__d_m1_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_state__d_m1_VALS[p] * state[FOM_MAT_su_state__d_m1_COLIDX[p]];
        out[i] += sc_m1 * s;
        s = 0.0;
        for (p = FOM_MAT_su_t_t0__d_d2_ROWPTR[i]; p < FOM_MAT_su_t_t0__d_d2_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t0__d_d2_VALS[p] * t_t0[FOM_MAT_su_t_t0__d_d2_COLIDX[p]];
        out[i] += sc_d2 * s;
        s = 0.0;
        for (p = FOM_MAT_su_t_t0__d_g1_ROWPTR[i]; p < FOM_MAT_su_t_t0__d_g1_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t0__d_g1_VALS[p] * t_t0[FOM_MAT_su_t_t0__d_g1_COLIDX[p]];
        out[i] += sc_g1 * s;
        s = 0.0;
        for (p = FOM_MAT_su_t_t1__d_d2_ROWPTR[i]; p < FOM_MAT_su_t_t1__d_d2_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t1__d_d2_VALS[p] * t_t1[FOM_MAT_su_t_t1__d_d2_COLIDX[p]];
        out[i] += sc_d2 * s;
        s = 0.0;
        for (p = FOM_MAT_su_t_t1__d_g1_ROWPTR[i]; p < FOM_MAT_su_t_t1__d_g1_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t1__d_g1_VALS[p] * t_t1[FOM_MAT_su_t_t1__d_g1_COLIDX[p]];
        out[i] += sc_g1 * s;
        s = 0.0;
        for (p = FOM_MAT_su_t_t1__d_m2_ROWPTR[i]; p < FOM_MAT_su_t_t1__d_m2_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t1__d_m2_VALS[p] * t_t1[FOM_MAT_su_t_t1__d_m2_COLIDX[p]];
        out[i] += sc_m2 * s;
    }
}

void fom_step_su__rest(const fom_float *state, const fom_float *t_t0, const fom_float *t_t1, fom_float sc_d1, fom_float sc_g2, fom_float sc_m1, fom_float sc_d2, fom_float sc_g1, fom_float sc_m2, const fom_float *bias, fom_float *out) {
    fom_int i, r, p;
    for (r = 0; r < FOM_REST_su_N; ++r) {
        fom_float s, v;
        i = FOM_REST_su_ROWS[r];
        v = bias[i];
        s = 0.0;
        for (p = FOM_MAT_su_state_ROWPTR[i]; p < FOM_MAT_su_state_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_state_VALS[p] * state[FOM_MAT_su_state_COLIDX[p]];
        v += s;
        s = 0.0;
        for (p = FOM_MAT_su_state__d_d1_ROWPTR[i]; p < FOM_MAT_su_state__d_d1_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_state__d_d1_VALS[p] * state[FOM_MAT_su_state__d_d1_COLIDX[p]];
        v += sc_d1 * s;
        s = 0.0;
        for (p = FOM_MAT_su_state__d_g2_ROWPTR[i]; p < FOM_MAT_su_state__d_g2_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_state__d_g2_VALS[p] * state[FOM_MAT_su_state__d_g2_COLIDX[p]];
        v += sc_g2 * s;
        s = 0.0;
        for (p = FOM_MAT_su_state__d_m1_ROWPTR[i]; p < FOM_MAT_su_state__d_m1_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_state__d_m1_VALS[p] * state[FOM_MAT_su_state__d_m1_COLIDX[p]];
        v += sc_m1 * s;
        s = 0.0;
        for (p = FOM_MAT_su_t_t0__d_d2_ROWPTR[i]; p < FOM_MAT_su_t_t0__d_d2_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t0__d_d2_VALS[p] * t_t0[FOM_MAT_su_t_t0__d_d2_COLIDX[p]];
        v += sc_d2 * s;
        s = 0.0;
        for (p = FOM_MAT_su_t_t0__d_g1_ROWPTR[i]; p < FOM_MAT_su_t_t0__d_g1_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t0__d_g1_VALS[p] * t_t0[FOM_MAT_su_t_t0__d_g1_COLIDX[p]];
        v += sc_g1 * s;
        s = 0.0;
        for (p = FOM_MAT_su_t_t1__d_d2_ROWPTR[i]; p < FOM_MAT_su_t_t1__d_d2_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t1__d_d2_VALS[p] * t_t1[FOM_MAT_su_t_t1__d_d2_COLIDX[p]];
        v += sc_d2 * s;
        s = 0.0;
        for (p = FOM_MAT_su_t_t1__d_g1_ROWPTR[i]; p < FOM_MAT_su_t_t1__d_g1_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t1__d_g1_VALS[p] * t_t1[FOM_MAT_su_t_t1__d_g1_COLIDX[p]];
        v += sc_g1 * s;
        s = 0.0;
        for (p = FOM_MAT_su_t_t1__d_m2_ROWPTR[i]; p < FOM_MAT_su_t_t1__d_m2_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t1__d_m2_VALS[p] * t_t1[FOM_MAT_su_t_t1__d_m2_COLIDX[p]];
        v += sc_m2 * s;
        out[i] = v;
    }
}

void fom_iterate(const fom_float *state, fom_float sc_taux, fom_float sc_tth, fom_float sc_eta, fom_float sc_d1, fom_float sc_g2, fom_float sc_m1, fom_float sc_d2, fom_float sc_g1, fom_float sc_m2, const fom_float *bias_t0, const fom_float *bias_t1, const fom_float *bias_su, const FomAct_t0 *act_t0, const FomAct_t1 *act_t1, fom_float *t_t0, fom_float *t_t1, fom_float *new_state) {
    fom_step_t0(state, sc_taux, sc_tth, bias_t0, act_t0, t_t0);
    fom_step_t1(state, t_t0, sc_eta, bias_t1, act_t1, t_t1);
    fom_step_su(state, t_t0, t_t1, sc_d1, sc_g2, sc_m1, sc_d2, sc_g1, sc_m2, bias_su, new_state);
}
