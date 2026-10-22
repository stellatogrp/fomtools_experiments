#include "fom_ops.h"
#include "fom_profile.h"

#if defined(__clang__)
#define FOM_UNROLL_FULL _Pragma("clang loop unroll(full)")
#else
#define FOM_UNROLL_FULL
#endif
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
    /* Selection members (t0_state, t1_state, su_state*, su_t_*) applied as
       shifted copies; the general members unrolled over their fixed sparsity. */
    fom_int i;
    const fom_float *lo0 = act_t0->lo, *lo1 = act_t1->lo, *hi1 = act_t1->hi;
    /* d_tth = -d_taux with columns shifted by 128: fold both schedule terms
       into one 16-vector u = taux*x[0..15] - tth*x[128..143] */
    fom_float u[16];
    FOM_PROF_BEGIN(t0);
    for (i = 0; i < 16; ++i) u[i] = sc_taux * state[i] - sc_tth * state[128 + i];
    for (i = 0; i < 32; ++i) {
        fom_float v = bias_t0[i] + state[i + 16] + FOM_MAT_t0_state__d_taux_VALS[i] * u[i & 15];
        t_t0[i] = v < lo0[i] ? lo0[i] : v;
    }
    {
        /* rows 32..39 are a Toeplitz combination of differences of u; rows
           40..47 are their exact negatives */
        static const fom_float C[4] = {0.07259999999999998, 0.21772, 0.36284, 0.50796};
        fom_float r[4], q[4], A[8];
        fom_int j, k;
        for (j = 0; j < 4; ++j) { r[j] = u[4 * j + 3] - u[4 * j + 1]; q[j] = u[4 * j + 2] - u[4 * j]; }
        FOM_UNROLL_FULL
        for (j = 0; j < 4; ++j) {
            fom_float ar = 0.0, aq = 0.0;
            for (k = 0; k <= j; ++k) { ar += C[j - k] * r[k]; aq += C[j - k] * q[k]; }
            A[2 * j] = ar; A[2 * j + 1] = aq;
        }
        for (i = 32; i < 40; ++i) {
            fom_float v = bias_t0[i] + state[i + 16] + A[i - 32];
            t_t0[i] = v < lo0[i] ? lo0[i] : v;
            v = bias_t0[i + 8] + state[i + 24] - A[i - 32];
            t_t0[i + 8] = v < lo0[i + 8] ? lo0[i + 8] : v;
        }
    }
    FOM_PROF_END(t0);
    /* t1_state__d_eta: dense 16x16 on columns 144..159 made of 4x4 blocks that
       are symmetric circulants [a b c b]; each block is diagonal in the basis
       (x0+x2)+-(x1+x3), x0-x2, x1-x3, so 64 mults replace 256. */
    fom_float dd[16];
    FOM_PROF_BEGIN(t1_dense);
    {
        /* per-block circulant eigen-coefficients a+c+2b, a+c-2b, a-c (exact) */
        static const fom_float K1[16] = {-13.018647553983678, -11.523745370178833, -10.138870109850522, -8.781714635809358, -11.523745370178833, -11.322266315697716, -10.018801121546662, -8.828357091823438, -10.138870109850522, -10.018801121546662, -9.99685414987912, -8.875729076399999, -8.781714635809358, -8.828357091823438, -8.875729076399999, -9.023842};
        static const fom_float K2[16] = {-4.97627590000001, -3.4174543279999945, -1.9958183599999995, -0.6485535999999996, -3.4174543279999945, -2.514101604000002, -1.4200451999999997, -0.4631743999999989, -1.9958183599999995, -1.4200451999999997, -0.9442720399999995, -0.2777952000000008, -0.6485535999999996, -0.4631743999999989, -0.2777952000000008, -0.1924159999999997};
        static const fom_float K3[16] = {-78.79836558940245, -39.9150055444113, -14.486237631536042, -2.046802194016849, -39.9150055444113, -20.465349505059486, -7.435205591386116, -1.0546164120176003, -14.486237631536042, -7.435205591386116, -2.83448394521744, -0.38980525599999893, -2.046802194016849, -1.0546164120176003, -0.38980525599999893, -0.15576799999999968};
        const fom_float *sx = state + 144;
        fom_float ge[4], he[4], pe[4], qe[4];
        fom_int I, J;
        for (J = 0; J < 4; ++J) {
            fom_float e = sx[4 * J] + sx[4 * J + 2], f = sx[4 * J + 1] + sx[4 * J + 3];
            ge[J] = e + f; he[J] = e - f;
            pe[J] = sx[4 * J] - sx[4 * J + 2]; qe[J] = sx[4 * J + 1] - sx[4 * J + 3];
        }
        FOM_UNROLL_FULL
        for (I = 0; I < 4; ++I) {
            fom_float G = 0.0, H = 0.0, P = 0.0, Q = 0.0, E, F;
            for (J = 0; J < 4; ++J) {
                G += K1[4 * I + J] * ge[J]; H += K2[4 * I + J] * he[J];
                P += K3[4 * I + J] * pe[J]; Q += K3[4 * I + J] * qe[J];
            }
            E = 0.5 * (G + H); F = 0.5 * (G - H);
            dd[4 * I] = 0.5 * (E + P); dd[4 * I + 2] = 0.5 * (E - P);
            dd[4 * I + 1] = 0.5 * (F + Q); dd[4 * I + 3] = 0.5 * (F - Q);
        }
    }
    FOM_PROF_END(t1_dense);
    {
        /* t1_t_t0__d_eta: -t0[i] + t0[i+16] plus a Toeplitz combination of the
           differences t0[32+k] - t0[40+k]; rows 4m+2, 4m+3 negate rows 4m, 4m+1 */
        static const fom_float C[4] = {0.07259999999999998, 0.21772, 0.36284, 0.50796};
        fom_float D[8], B[8];
        fom_int m, k;
        for (k = 0; k < 8; ++k) D[k] = t_t0[32 + k] - t_t0[40 + k];
        FOM_UNROLL_FULL
        for (m = 0; m < 4; ++m) {
            fom_float b0 = 0.0, b1 = 0.0;
            for (k = m; k < 4; ++k) { b0 += C[k - m] * D[2 * k + 1]; b1 += C[k - m] * D[2 * k]; }
            B[2 * m] = b0; B[2 * m + 1] = b1;
        }
        for (m = 0; m < 4; ++m) {
            for (k = 0; k < 4; ++k) {
                fom_int ii = 4 * m + k;
                fom_float sb = (k < 2) ? B[2 * m + k] : -B[2 * m + k - 2];
                fom_float v = bias_t1[ii] + state[ii] + sc_eta * (dd[ii] - t_t0[ii] + t_t0[ii + 16] + sb);
                if (v < lo1[ii]) v = lo1[ii];
                if (v > hi1[ii]) v = hi1[ii];
                t_t1[ii] = v;
            }
        }
    }
    for (i = 0; i < 16; ++i) new_state[i] = bias_su[i] + sc_g2 * state[i + 64] + sc_g1 * t_t1[i];
    for (i = 16; i < 64; ++i) new_state[i] = bias_su[i] + sc_g2 * state[i + 64] + sc_g1 * t_t0[i - 16];
    for (i = 64; i < 80; ++i) new_state[i] = bias_su[i] + sc_d1 * state[i] + sc_d2 * t_t1[i - 64];
    for (i = 80; i < 128; ++i) new_state[i] = bias_su[i] + sc_d1 * state[i] + sc_d2 * t_t0[i - 80];
    for (i = 128; i < 144; ++i) new_state[i] = bias_su[i] + state[i - 128];
    for (i = 144; i < 160; ++i) new_state[i] = bias_su[i] + sc_m1 * state[i - 80] + sc_m2 * t_t1[i - 144];
}
