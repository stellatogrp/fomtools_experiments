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
        for (p = FOM_MAT_t1_state__d_eta_ROWPTR[i]; p < FOM_MAT_t1_state__d_eta_ROWPTR[i + 1]; ++p) s += FOM_MAT_t1_state__d_eta_VALS[p] * state[FOM_MAT_t1_state__d_eta_COLIDX[p]];
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
    fom_int i, r, p;
    for (r = 0; r < 48; ++r) {
        fom_float s, v;
        i = r;
        v = bias[i];
        s = 0.0;
        for (p = FOM_MAT_t0_state_ROWPTR[i]; p < FOM_MAT_t0_state_ROWPTR[i + 1]; ++p) s += FOM_MAT_t0_state_VALS[p] * state[FOM_MAT_t0_state_COLIDX[p]];
        v += s;
        s = 0.0;
        for (p = FOM_MAT_t0_state__d_taux_ROWPTR[i]; p < FOM_MAT_t0_state__d_taux_ROWPTR[i + 1]; ++p) s += FOM_MAT_t0_state__d_taux_VALS[p] * state[FOM_MAT_t0_state__d_taux_COLIDX[p]];
        v += sc_taux * s;
        s = 0.0;
        for (p = FOM_MAT_t0_state__d_tth_ROWPTR[i]; p < FOM_MAT_t0_state__d_tth_ROWPTR[i + 1]; ++p) s += FOM_MAT_t0_state__d_tth_VALS[p] * state[FOM_MAT_t0_state__d_tth_COLIDX[p]];
        v += sc_tth * s;
        if (v < act->lo[i]) v = act->lo[i];
        out[i] = v;
    }
}

void fom_step_t1(const fom_float *state, const fom_float *t_t0, fom_float sc_eta, const fom_float *bias, const FomAct_t1 *act, fom_float *out) {
    fom_int i, r, p;
    for (r = 0; r < 16; ++r) {
        fom_float s, v;
        i = r;
        v = bias[i];
        s = 0.0;
        for (p = FOM_MAT_t1_state_ROWPTR[i]; p < FOM_MAT_t1_state_ROWPTR[i + 1]; ++p) s += FOM_MAT_t1_state_VALS[p] * state[FOM_MAT_t1_state_COLIDX[p]];
        v += s;
        s = 0.0;
        for (p = FOM_MAT_t1_state__d_eta_ROWPTR[i]; p < FOM_MAT_t1_state__d_eta_ROWPTR[i + 1]; ++p) s += FOM_MAT_t1_state__d_eta_VALS[p] * state[FOM_MAT_t1_state__d_eta_COLIDX[p]];
        v += sc_eta * s;
        s = 0.0;
        for (p = FOM_MAT_t1_t_t0__d_eta_ROWPTR[i]; p < FOM_MAT_t1_t_t0__d_eta_ROWPTR[i + 1]; ++p) s += FOM_MAT_t1_t_t0__d_eta_VALS[p] * t_t0[FOM_MAT_t1_t_t0__d_eta_COLIDX[p]];
        v += sc_eta * s;
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
    fom_int i;
    /* t0: shift state[i+16] plus taux*A x[0:16] + tth*B x[128:144], where
       B is -A with columns shifted by 128 (same pattern, negated values), so
       both terms collapse to one CSR pass over w = taux*x[c] - tth*x[c+128]. */
    {
        fom_float w[16];
        for (i = 0; i < 16; ++i) w[i] = sc_taux * state[i] - sc_tth * state[i + 128];
        fom_float v;
        /* Unrolled over the fixed sparsity of t0_state__d_taux. */
        v = bias_t0[0] + state[16] + (w[0]);
        if (v < act_t0->lo[0]) v = act_t0->lo[0];
        t_t0[0] = v;
        v = bias_t0[1] + state[17] + (w[1]);
        if (v < act_t0->lo[1]) v = act_t0->lo[1];
        t_t0[1] = v;
        v = bias_t0[2] + state[18] + (w[2]);
        if (v < act_t0->lo[2]) v = act_t0->lo[2];
        t_t0[2] = v;
        v = bias_t0[3] + state[19] + (w[3]);
        if (v < act_t0->lo[3]) v = act_t0->lo[3];
        t_t0[3] = v;
        v = bias_t0[4] + state[20] + (w[4]);
        if (v < act_t0->lo[4]) v = act_t0->lo[4];
        t_t0[4] = v;
        v = bias_t0[5] + state[21] + (w[5]);
        if (v < act_t0->lo[5]) v = act_t0->lo[5];
        t_t0[5] = v;
        v = bias_t0[6] + state[22] + (w[6]);
        if (v < act_t0->lo[6]) v = act_t0->lo[6];
        t_t0[6] = v;
        v = bias_t0[7] + state[23] + (w[7]);
        if (v < act_t0->lo[7]) v = act_t0->lo[7];
        t_t0[7] = v;
        v = bias_t0[8] + state[24] + (w[8]);
        if (v < act_t0->lo[8]) v = act_t0->lo[8];
        t_t0[8] = v;
        v = bias_t0[9] + state[25] + (w[9]);
        if (v < act_t0->lo[9]) v = act_t0->lo[9];
        t_t0[9] = v;
        v = bias_t0[10] + state[26] + (w[10]);
        if (v < act_t0->lo[10]) v = act_t0->lo[10];
        t_t0[10] = v;
        v = bias_t0[11] + state[27] + (w[11]);
        if (v < act_t0->lo[11]) v = act_t0->lo[11];
        t_t0[11] = v;
        v = bias_t0[12] + state[28] + (w[12]);
        if (v < act_t0->lo[12]) v = act_t0->lo[12];
        t_t0[12] = v;
        v = bias_t0[13] + state[29] + (w[13]);
        if (v < act_t0->lo[13]) v = act_t0->lo[13];
        t_t0[13] = v;
        v = bias_t0[14] + state[30] + (w[14]);
        if (v < act_t0->lo[14]) v = act_t0->lo[14];
        t_t0[14] = v;
        v = bias_t0[15] + state[31] + (w[15]);
        if (v < act_t0->lo[15]) v = act_t0->lo[15];
        t_t0[15] = v;
        v = bias_t0[16] + state[32] + (-w[0]);
        if (v < act_t0->lo[16]) v = act_t0->lo[16];
        t_t0[16] = v;
        v = bias_t0[17] + state[33] + (-w[1]);
        if (v < act_t0->lo[17]) v = act_t0->lo[17];
        t_t0[17] = v;
        v = bias_t0[18] + state[34] + (-w[2]);
        if (v < act_t0->lo[18]) v = act_t0->lo[18];
        t_t0[18] = v;
        v = bias_t0[19] + state[35] + (-w[3]);
        if (v < act_t0->lo[19]) v = act_t0->lo[19];
        t_t0[19] = v;
        v = bias_t0[20] + state[36] + (-w[4]);
        if (v < act_t0->lo[20]) v = act_t0->lo[20];
        t_t0[20] = v;
        v = bias_t0[21] + state[37] + (-w[5]);
        if (v < act_t0->lo[21]) v = act_t0->lo[21];
        t_t0[21] = v;
        v = bias_t0[22] + state[38] + (-w[6]);
        if (v < act_t0->lo[22]) v = act_t0->lo[22];
        t_t0[22] = v;
        v = bias_t0[23] + state[39] + (-w[7]);
        if (v < act_t0->lo[23]) v = act_t0->lo[23];
        t_t0[23] = v;
        v = bias_t0[24] + state[40] + (-w[8]);
        if (v < act_t0->lo[24]) v = act_t0->lo[24];
        t_t0[24] = v;
        v = bias_t0[25] + state[41] + (-w[9]);
        if (v < act_t0->lo[25]) v = act_t0->lo[25];
        t_t0[25] = v;
        v = bias_t0[26] + state[42] + (-w[10]);
        if (v < act_t0->lo[26]) v = act_t0->lo[26];
        t_t0[26] = v;
        v = bias_t0[27] + state[43] + (-w[11]);
        if (v < act_t0->lo[27]) v = act_t0->lo[27];
        t_t0[27] = v;
        v = bias_t0[28] + state[44] + (-w[12]);
        if (v < act_t0->lo[28]) v = act_t0->lo[28];
        t_t0[28] = v;
        v = bias_t0[29] + state[45] + (-w[13]);
        if (v < act_t0->lo[29]) v = act_t0->lo[29];
        t_t0[29] = v;
        v = bias_t0[30] + state[46] + (-w[14]);
        if (v < act_t0->lo[30]) v = act_t0->lo[30];
        t_t0[30] = v;
        v = bias_t0[31] + state[47] + (-w[15]);
        if (v < act_t0->lo[31]) v = act_t0->lo[31];
        t_t0[31] = v;
        v = bias_t0[32] + state[48] + ((-0x1.295e9e1b0899fp-4) * w[1] + (0x1.295e9e1b0899fp-4) * w[3]);
        if (v < act_t0->lo[32]) v = act_t0->lo[32];
        t_t0[32] = v;
        v = bias_t0[33] + state[49] + ((-0x1.295e9e1b0899fp-4) * w[0] + (0x1.295e9e1b0899fp-4) * w[2]);
        if (v < act_t0->lo[33]) v = act_t0->lo[33];
        t_t0[33] = v;
        v = bias_t0[34] + state[50] + ((-0x1.bde3fbbd7b203p-3) * w[1] + (0x1.bde3fbbd7b203p-3) * w[3] + (-0x1.295e9e1b0899fp-4) * w[5] + (0x1.295e9e1b0899fp-4) * w[7]);
        if (v < act_t0->lo[34]) v = act_t0->lo[34];
        t_t0[34] = v;
        v = bias_t0[35] + state[51] + ((-0x1.bde3fbbd7b203p-3) * w[0] + (0x1.bde3fbbd7b203p-3) * w[2] + (-0x1.295e9e1b0899fp-4) * w[4] + (0x1.295e9e1b0899fp-4) * w[6]);
        if (v < act_t0->lo[35]) v = act_t0->lo[35];
        t_t0[35] = v;
        v = bias_t0[36] + state[52] + ((-0x1.738c5436b8f9bp-2) * w[1] + (0x1.738c5436b8f9bp-2) * w[3] + (-0x1.bde3fbbd7b203p-3) * w[5] + (0x1.bde3fbbd7b203p-3) * w[7] + (-0x1.295e9e1b0899fp-4) * w[9] + (0x1.295e9e1b0899fp-4) * w[11]);
        if (v < act_t0->lo[36]) v = act_t0->lo[36];
        t_t0[36] = v;
        v = bias_t0[37] + state[53] + ((-0x1.738c5436b8f9bp-2) * w[0] + (0x1.738c5436b8f9bp-2) * w[2] + (-0x1.bde3fbbd7b203p-3) * w[4] + (0x1.bde3fbbd7b203p-3) * w[6] + (-0x1.295e9e1b0899fp-4) * w[8] + (0x1.295e9e1b0899fp-4) * w[10]);
        if (v < act_t0->lo[37]) v = act_t0->lo[37];
        t_t0[37] = v;
        v = bias_t0[38] + state[54] + ((-0x1.041355475a31ap-1) * w[1] + (0x1.041355475a31ap-1) * w[3] + (-0x1.738c5436b8f9bp-2) * w[5] + (0x1.738c5436b8f9bp-2) * w[7] + (-0x1.bde3fbbd7b203p-3) * w[9] + (0x1.bde3fbbd7b203p-3) * w[11] + (-0x1.295e9e1b0899fp-4) * w[13] + (0x1.295e9e1b0899fp-4) * w[15]);
        if (v < act_t0->lo[38]) v = act_t0->lo[38];
        t_t0[38] = v;
        v = bias_t0[39] + state[55] + ((-0x1.041355475a31ap-1) * w[0] + (0x1.041355475a31ap-1) * w[2] + (-0x1.738c5436b8f9bp-2) * w[4] + (0x1.738c5436b8f9bp-2) * w[6] + (-0x1.bde3fbbd7b203p-3) * w[8] + (0x1.bde3fbbd7b203p-3) * w[10] + (-0x1.295e9e1b0899fp-4) * w[12] + (0x1.295e9e1b0899fp-4) * w[14]);
        if (v < act_t0->lo[39]) v = act_t0->lo[39];
        t_t0[39] = v;
        v = bias_t0[40] + state[56] + ((0x1.295e9e1b0899fp-4) * w[1] + (-0x1.295e9e1b0899fp-4) * w[3]);
        if (v < act_t0->lo[40]) v = act_t0->lo[40];
        t_t0[40] = v;
        v = bias_t0[41] + state[57] + ((0x1.295e9e1b0899fp-4) * w[0] + (-0x1.295e9e1b0899fp-4) * w[2]);
        if (v < act_t0->lo[41]) v = act_t0->lo[41];
        t_t0[41] = v;
        v = bias_t0[42] + state[58] + ((0x1.bde3fbbd7b203p-3) * w[1] + (-0x1.bde3fbbd7b203p-3) * w[3] + (0x1.295e9e1b0899fp-4) * w[5] + (-0x1.295e9e1b0899fp-4) * w[7]);
        if (v < act_t0->lo[42]) v = act_t0->lo[42];
        t_t0[42] = v;
        v = bias_t0[43] + state[59] + ((0x1.bde3fbbd7b203p-3) * w[0] + (-0x1.bde3fbbd7b203p-3) * w[2] + (0x1.295e9e1b0899fp-4) * w[4] + (-0x1.295e9e1b0899fp-4) * w[6]);
        if (v < act_t0->lo[43]) v = act_t0->lo[43];
        t_t0[43] = v;
        v = bias_t0[44] + state[60] + ((0x1.738c5436b8f9bp-2) * w[1] + (-0x1.738c5436b8f9bp-2) * w[3] + (0x1.bde3fbbd7b203p-3) * w[5] + (-0x1.bde3fbbd7b203p-3) * w[7] + (0x1.295e9e1b0899fp-4) * w[9] + (-0x1.295e9e1b0899fp-4) * w[11]);
        if (v < act_t0->lo[44]) v = act_t0->lo[44];
        t_t0[44] = v;
        v = bias_t0[45] + state[61] + ((0x1.738c5436b8f9bp-2) * w[0] + (-0x1.738c5436b8f9bp-2) * w[2] + (0x1.bde3fbbd7b203p-3) * w[4] + (-0x1.bde3fbbd7b203p-3) * w[6] + (0x1.295e9e1b0899fp-4) * w[8] + (-0x1.295e9e1b0899fp-4) * w[10]);
        if (v < act_t0->lo[45]) v = act_t0->lo[45];
        t_t0[45] = v;
        v = bias_t0[46] + state[62] + ((0x1.041355475a31ap-1) * w[1] + (-0x1.041355475a31ap-1) * w[3] + (0x1.738c5436b8f9bp-2) * w[5] + (-0x1.738c5436b8f9bp-2) * w[7] + (0x1.bde3fbbd7b203p-3) * w[9] + (-0x1.bde3fbbd7b203p-3) * w[11] + (0x1.295e9e1b0899fp-4) * w[13] + (-0x1.295e9e1b0899fp-4) * w[15]);
        if (v < act_t0->lo[46]) v = act_t0->lo[46];
        t_t0[46] = v;
        v = bias_t0[47] + state[63] + ((0x1.041355475a31ap-1) * w[0] + (-0x1.041355475a31ap-1) * w[2] + (0x1.738c5436b8f9bp-2) * w[4] + (-0x1.738c5436b8f9bp-2) * w[6] + (0x1.bde3fbbd7b203p-3) * w[8] + (-0x1.bde3fbbd7b203p-3) * w[10] + (0x1.295e9e1b0899fp-4) * w[12] + (-0x1.295e9e1b0899fp-4) * w[14]);
        if (v < act_t0->lo[47]) v = act_t0->lo[47];
        t_t0[47] = v;
    }
    /* t1_t_t0__d_eta @ t0, unrolled over its fixed sparsity. */
    fom_float ss[16];
    ss[0] = -t_t0[0] + t_t0[16] + (0x1.295e9e1b0899fp-4) * t_t0[33] + (0x1.bde3fbbd7b203p-3) * t_t0[35] + (0x1.738c5436b8f9bp-2) * t_t0[37] + (0x1.041355475a31ap-1) * t_t0[39] + (-0x1.295e9e1b0899fp-4) * t_t0[41] + (-0x1.bde3fbbd7b203p-3) * t_t0[43] + (-0x1.738c5436b8f9bp-2) * t_t0[45] + (-0x1.041355475a31ap-1) * t_t0[47];
    ss[1] = -t_t0[1] + t_t0[17] + (0x1.295e9e1b0899fp-4) * t_t0[32] + (0x1.bde3fbbd7b203p-3) * t_t0[34] + (0x1.738c5436b8f9bp-2) * t_t0[36] + (0x1.041355475a31ap-1) * t_t0[38] + (-0x1.295e9e1b0899fp-4) * t_t0[40] + (-0x1.bde3fbbd7b203p-3) * t_t0[42] + (-0x1.738c5436b8f9bp-2) * t_t0[44] + (-0x1.041355475a31ap-1) * t_t0[46];
    ss[2] = -t_t0[2] + t_t0[18] + (-0x1.295e9e1b0899fp-4) * t_t0[33] + (-0x1.bde3fbbd7b203p-3) * t_t0[35] + (-0x1.738c5436b8f9bp-2) * t_t0[37] + (-0x1.041355475a31ap-1) * t_t0[39] + (0x1.295e9e1b0899fp-4) * t_t0[41] + (0x1.bde3fbbd7b203p-3) * t_t0[43] + (0x1.738c5436b8f9bp-2) * t_t0[45] + (0x1.041355475a31ap-1) * t_t0[47];
    ss[3] = -t_t0[3] + t_t0[19] + (-0x1.295e9e1b0899fp-4) * t_t0[32] + (-0x1.bde3fbbd7b203p-3) * t_t0[34] + (-0x1.738c5436b8f9bp-2) * t_t0[36] + (-0x1.041355475a31ap-1) * t_t0[38] + (0x1.295e9e1b0899fp-4) * t_t0[40] + (0x1.bde3fbbd7b203p-3) * t_t0[42] + (0x1.738c5436b8f9bp-2) * t_t0[44] + (0x1.041355475a31ap-1) * t_t0[46];
    ss[4] = -t_t0[4] + t_t0[20] + (0x1.295e9e1b0899fp-4) * t_t0[35] + (0x1.bde3fbbd7b203p-3) * t_t0[37] + (0x1.738c5436b8f9bp-2) * t_t0[39] + (-0x1.295e9e1b0899fp-4) * t_t0[43] + (-0x1.bde3fbbd7b203p-3) * t_t0[45] + (-0x1.738c5436b8f9bp-2) * t_t0[47];
    ss[5] = -t_t0[5] + t_t0[21] + (0x1.295e9e1b0899fp-4) * t_t0[34] + (0x1.bde3fbbd7b203p-3) * t_t0[36] + (0x1.738c5436b8f9bp-2) * t_t0[38] + (-0x1.295e9e1b0899fp-4) * t_t0[42] + (-0x1.bde3fbbd7b203p-3) * t_t0[44] + (-0x1.738c5436b8f9bp-2) * t_t0[46];
    ss[6] = -t_t0[6] + t_t0[22] + (-0x1.295e9e1b0899fp-4) * t_t0[35] + (-0x1.bde3fbbd7b203p-3) * t_t0[37] + (-0x1.738c5436b8f9bp-2) * t_t0[39] + (0x1.295e9e1b0899fp-4) * t_t0[43] + (0x1.bde3fbbd7b203p-3) * t_t0[45] + (0x1.738c5436b8f9bp-2) * t_t0[47];
    ss[7] = -t_t0[7] + t_t0[23] + (-0x1.295e9e1b0899fp-4) * t_t0[34] + (-0x1.bde3fbbd7b203p-3) * t_t0[36] + (-0x1.738c5436b8f9bp-2) * t_t0[38] + (0x1.295e9e1b0899fp-4) * t_t0[42] + (0x1.bde3fbbd7b203p-3) * t_t0[44] + (0x1.738c5436b8f9bp-2) * t_t0[46];
    ss[8] = -t_t0[8] + t_t0[24] + (0x1.295e9e1b0899fp-4) * t_t0[37] + (0x1.bde3fbbd7b203p-3) * t_t0[39] + (-0x1.295e9e1b0899fp-4) * t_t0[45] + (-0x1.bde3fbbd7b203p-3) * t_t0[47];
    ss[9] = -t_t0[9] + t_t0[25] + (0x1.295e9e1b0899fp-4) * t_t0[36] + (0x1.bde3fbbd7b203p-3) * t_t0[38] + (-0x1.295e9e1b0899fp-4) * t_t0[44] + (-0x1.bde3fbbd7b203p-3) * t_t0[46];
    ss[10] = -t_t0[10] + t_t0[26] + (-0x1.295e9e1b0899fp-4) * t_t0[37] + (-0x1.bde3fbbd7b203p-3) * t_t0[39] + (0x1.295e9e1b0899fp-4) * t_t0[45] + (0x1.bde3fbbd7b203p-3) * t_t0[47];
    ss[11] = -t_t0[11] + t_t0[27] + (-0x1.295e9e1b0899fp-4) * t_t0[36] + (-0x1.bde3fbbd7b203p-3) * t_t0[38] + (0x1.295e9e1b0899fp-4) * t_t0[44] + (0x1.bde3fbbd7b203p-3) * t_t0[46];
    ss[12] = -t_t0[12] + t_t0[28] + (0x1.295e9e1b0899fp-4) * t_t0[39] + (-0x1.295e9e1b0899fp-4) * t_t0[47];
    ss[13] = -t_t0[13] + t_t0[29] + (0x1.295e9e1b0899fp-4) * t_t0[38] + (-0x1.295e9e1b0899fp-4) * t_t0[46];
    ss[14] = -t_t0[14] + t_t0[30] + (-0x1.295e9e1b0899fp-4) * t_t0[39] + (0x1.295e9e1b0899fp-4) * t_t0[47];
    ss[15] = -t_t0[15] + t_t0[31] + (-0x1.295e9e1b0899fp-4) * t_t0[38] + (0x1.295e9e1b0899fp-4) * t_t0[46];
    /* t1: shift state[i] plus eta * (CSR state term + CSR t0 term), clip. */
    for (i = 0; i < 16; ++i) {
        fom_float s, v, s0 = 0.0, s1 = 0.0, s2 = 0.0, s3 = 0.0;
        const fom_float *a = FOM_MAT_t1_state__d_eta_VALS + 16 * i;
        const fom_float *xs = state + 144;
        fom_int j;
        /* t1_state__d_eta is a dense 16x16 block on state[144:160];
           four independent partial sums break the FMA dependency chain. */
        for (j = 0; j < 16; j += 4) {
            s0 += a[j] * xs[j];
            s1 += a[j + 1] * xs[j + 1];
            s2 += a[j + 2] * xs[j + 2];
            s3 += a[j + 3] * xs[j + 3];
        }
        s = (s0 + s1) + (s2 + s3) + ss[i];
        v = bias_t1[i] + state[i] + sc_eta * s;
        if (v < act_t1->lo[i]) v = act_t1->lo[i];
        if (v > act_t1->hi[i]) v = act_t1->hi[i];
        t_t1[i] = v;
    }
    /* su: every member is a 0/1 shift. */
    for (i = 0; i < 16; ++i) new_state[i] = bias_su[i] + sc_g2 * state[i + 64] + sc_g1 * t_t1[i];
    for (i = 16; i < 64; ++i) new_state[i] = bias_su[i] + sc_g2 * state[i + 64] + sc_g1 * t_t0[i - 16];
    for (i = 64; i < 80; ++i) new_state[i] = bias_su[i] + sc_d1 * state[i] + sc_d2 * t_t1[i - 64];
    for (i = 80; i < 128; ++i) new_state[i] = bias_su[i] + sc_d1 * state[i] + sc_d2 * t_t0[i - 80];
    for (i = 128; i < 144; ++i) new_state[i] = bias_su[i] + state[i - 128];
    for (i = 144; i < 160; ++i) new_state[i] = bias_su[i] + sc_m1 * state[i - 80] + sc_m2 * t_t1[i - 144];
}
