#include "fom_ops.h"
#include "fom_data.h"
#include "fom_profile.h"

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
    fom_float w[16], u[48], wm[16], e[8];
    const fom_float *xm = state + 144;
    FOM_PROF_BEGIN(t0);
    /* d_tth's block on state[128..143] is the negated d_taux block on
       state[0..15], so both collapse into one 48x16 product, unrolled
       over its fixed sparsity with the coefficients inlined. */
    for (i = 0; i < 16; ++i) w[i] = sc_taux * state[i] - sc_tth * state[i + 128];
    u[0] = w[0];
    u[1] = w[1];
    u[2] = w[2];
    u[3] = w[3];
    u[4] = w[4];
    u[5] = w[5];
    u[6] = w[6];
    u[7] = w[7];
    u[8] = w[8];
    u[9] = w[9];
    u[10] = w[10];
    u[11] = w[11];
    u[12] = w[12];
    u[13] = w[13];
    u[14] = w[14];
    u[15] = w[15];
    u[16] = -w[0];
    u[17] = -w[1];
    u[18] = -w[2];
    u[19] = -w[3];
    u[20] = -w[4];
    u[21] = -w[5];
    u[22] = -w[6];
    u[23] = -w[7];
    u[24] = -w[8];
    u[25] = -w[9];
    u[26] = -w[10];
    u[27] = -w[11];
    u[28] = -w[12];
    u[29] = -w[13];
    u[30] = -w[14];
    u[31] = -w[15];
    u[32] = -0.07259999999999998 * w[1] + 0.07259999999999998 * w[3];
    u[33] = -0.07259999999999998 * w[0] + 0.07259999999999998 * w[2];
    u[34] = -0.21772 * w[1] + 0.21772 * w[3] - 0.07259999999999998 * w[5] + 0.07259999999999998 * w[7];
    u[35] = -0.21772 * w[0] + 0.21772 * w[2] - 0.07259999999999998 * w[4] + 0.07259999999999998 * w[6];
    u[36] = -0.36284 * w[1] + 0.36284 * w[3] - 0.21772 * w[5] + 0.21772 * w[7] - 0.07259999999999998 * w[9] + 0.07259999999999998 * w[11];
    u[37] = -0.36284 * w[0] + 0.36284 * w[2] - 0.21772 * w[4] + 0.21772 * w[6] - 0.07259999999999998 * w[8] + 0.07259999999999998 * w[10];
    u[38] = -0.50796 * w[1] + 0.50796 * w[3] - 0.36284 * w[5] + 0.36284 * w[7] - 0.21772 * w[9] + 0.21772 * w[11] - 0.07259999999999998 * w[13] + 0.07259999999999998 * w[15];
    u[39] = -0.50796 * w[0] + 0.50796 * w[2] - 0.36284 * w[4] + 0.36284 * w[6] - 0.21772 * w[8] + 0.21772 * w[10] - 0.07259999999999998 * w[12] + 0.07259999999999998 * w[14];
    u[40] = -u[32];
    u[41] = -u[33];
    u[42] = -u[34];
    u[43] = -u[35];
    u[44] = -u[36];
    u[45] = -u[37];
    u[46] = -u[38];
    u[47] = -u[39];
    for (i = 0; i < 48; ++i) {
        fom_float v = bias_t0[i] + state[i + 16] + u[i];
        if (v < act_t0->lo[i]) v = act_t0->lo[i];
        t_t0[i] = v;
    }
    FOM_PROF_END(t0);
    FOM_PROF_BEGIN(t1);
    /* d_eta products: dense 16x16 block on state[144..159] plus t0 term.
       Its t0 columns 32+j and 40+j carry opposite coefficients. */
    for (i = 0; i < 8; ++i) e[i] = t_t0[32 + i] - t_t0[40 + i];
    {
        /* Each 4x4 block of the 16x16 d_eta block on state[144..159] is
           [a b c b; b a b c; c b a b; b c b a], so in sum/difference
           coordinates (x0+-x2, x1+-x3) it splits into decoupled parts. */
        static const fom_float A[4][4] = {
            {-43.89791365819715, -23.69280269675036, -10.276790933230652, -3.380968155960764},
            {-23.69280269675036, -13.691766732454171, -6.577314376079723, -2.85019107896466},
            {-10.276790933230652, -6.577314376079723, -4.1525235200785, -2.4832836970999996},
            {-3.380968155960764, -2.85019107896466, -2.4832836970999996, -2.3819484999999996}};
        static const fom_float B[4][4] = {
            {-2.010592913495917, -2.0265727605447097, -2.0357629374626307, -2.0332902589523396},
            {-2.0265727605447097, -2.2020411779244284, -2.1496889803866654, -2.09129567295586},
            {-2.0357629374626307, -2.1496889803866654, -2.26314552746978, -2.1494834690999998},
            {-2.0332902589523396, -2.09129567295586, -2.1494834690999998, -2.2078565}};
        static const fom_float C[4][4] = {
            {34.9004519312053, 16.222202847660945, 4.209446698305391, -1.3341659619439152},
            {16.222202847660945, 6.773582772605312, 0.8578912153063928, -1.7955746669470596},
            {4.209446698305391, 0.8578912153063928, -1.31803957486106, -2.0934784411000007},
            {-1.3341659619439152, -1.7955746669470596, -2.0934784411000007, -2.2261805}};
        fom_float sv[4], dv[4], pv[4], qv[4];
        fom_int I, J;
        for (J = 0; J < 4; ++J) {
            sv[J] = xm[4 * J] + xm[4 * J + 2]; dv[J] = xm[4 * J] - xm[4 * J + 2];
            pv[J] = xm[4 * J + 1] + xm[4 * J + 3]; qv[J] = xm[4 * J + 1] - xm[4 * J + 3];
        }
        for (I = 0; I < 4; ++I) {
            fom_float E = 0, F = 0, G = 0, H = 0;
            for (J = 0; J < 4; ++J) {
                const fom_float ap = A[I][J] + C[I][J], am = A[I][J] - C[I][J], b2 = 2.0 * B[I][J];
                E += ap * sv[J] + b2 * pv[J];
                G += ap * pv[J] + b2 * sv[J];
                F += am * dv[J];
                H += am * qv[J];
            }
            wm[4 * I] = 0.5 * (E + F); wm[4 * I + 2] = 0.5 * (E - F);
            wm[4 * I + 1] = 0.5 * (G + H); wm[4 * I + 3] = 0.5 * (G - H);
        }
    }
    w[0] = wm[0]
        + -t_t0[0] + t_t0[16] + 0.07259999999999998 * e[1] + 0.21772 * e[3] + 0.36284 * e[5] + 0.50796 * e[7];
    w[1] = wm[1]
        + -t_t0[1] + t_t0[17] + 0.07259999999999998 * e[0] + 0.21772 * e[2] + 0.36284 * e[4] + 0.50796 * e[6];
    w[2] = wm[2]
        + -t_t0[2] + t_t0[18] - 0.07259999999999998 * e[1] - 0.21772 * e[3] - 0.36284 * e[5] - 0.50796 * e[7];
    w[3] = wm[3]
        + -t_t0[3] + t_t0[19] - 0.07259999999999998 * e[0] - 0.21772 * e[2] - 0.36284 * e[4] - 0.50796 * e[6];
    w[4] = wm[4]
        + -t_t0[4] + t_t0[20] + 0.07259999999999998 * e[3] + 0.21772 * e[5] + 0.36284 * e[7];
    w[5] = wm[5]
        + -t_t0[5] + t_t0[21] + 0.07259999999999998 * e[2] + 0.21772 * e[4] + 0.36284 * e[6];
    w[6] = wm[6]
        + -t_t0[6] + t_t0[22] - 0.07259999999999998 * e[3] - 0.21772 * e[5] - 0.36284 * e[7];
    w[7] = wm[7]
        + -t_t0[7] + t_t0[23] - 0.07259999999999998 * e[2] - 0.21772 * e[4] - 0.36284 * e[6];
    w[8] = wm[8]
        + -t_t0[8] + t_t0[24] + 0.07259999999999998 * e[5] + 0.21772 * e[7];
    w[9] = wm[9]
        + -t_t0[9] + t_t0[25] + 0.07259999999999998 * e[4] + 0.21772 * e[6];
    w[10] = wm[10]
        + -t_t0[10] + t_t0[26] - 0.07259999999999998 * e[5] - 0.21772 * e[7];
    w[11] = wm[11]
        + -t_t0[11] + t_t0[27] - 0.07259999999999998 * e[4] - 0.21772 * e[6];
    w[12] = wm[12]
        + -t_t0[12] + t_t0[28] + 0.07259999999999998 * e[7];
    w[13] = wm[13]
        + -t_t0[13] + t_t0[29] + 0.07259999999999998 * e[6];
    w[14] = wm[14]
        + -t_t0[14] + t_t0[30] - 0.07259999999999998 * e[7];
    w[15] = wm[15]
        + -t_t0[15] + t_t0[31] - 0.07259999999999998 * e[6];
    for (i = 0; i < 16; ++i) {
        fom_float v = bias_t1[i] + state[i] + sc_eta * w[i];
        if (v < act_t1->lo[i]) v = act_t1->lo[i];
        if (v > act_t1->hi[i]) v = act_t1->hi[i];
        t_t1[i] = v;
    }
    FOM_PROF_END(t1);
    FOM_PROF_BEGIN(su);
    for (i = 0; i < 16; ++i) new_state[i] = bias_su[i] + sc_g2 * state[i + 64] + sc_g1 * t_t1[i];
    for (i = 16; i < 64; ++i) new_state[i] = bias_su[i] + sc_g2 * state[i + 64] + sc_g1 * t_t0[i - 16];
    for (i = 64; i < 80; ++i) new_state[i] = bias_su[i] + sc_d1 * state[i] + sc_d2 * t_t1[i - 64];
    for (i = 80; i < 128; ++i) new_state[i] = bias_su[i] + sc_d1 * state[i] + sc_d2 * t_t0[i - 80];
    for (i = 128; i < 144; ++i) new_state[i] = bias_su[i] + state[i - 128];
    for (i = 144; i < 160; ++i) new_state[i] = bias_su[i] + sc_m1 * state[i - 80] + sc_m2 * t_t1[i - 144];
    FOM_PROF_END(su);
}
