#include "fom_ops.h"
#include "fom_data.h"

void fom_apply_t0_state(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 600; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t0_state_ROWPTR[i]; p < FOM_MAT_t0_state_ROWPTR[i + 1]; ++p) s += FOM_MAT_t0_state_VALS[p] * x[FOM_MAT_t0_state_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t1_state(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t1_state_ROWPTR[i]; p < FOM_MAT_t1_state_ROWPTR[i + 1]; ++p) s += FOM_MAT_t1_state_VALS[p] * x[FOM_MAT_t1_state_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t1_state__d_xi(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t1_state__d_xi_ROWPTR[i]; p < FOM_MAT_t1_state__d_xi_ROWPTR[i + 1]; ++p) s += FOM_MAT_t1_state__d_xi_VALS[p] * x[FOM_MAT_t1_state__d_xi_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t1_t_t0(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t1_t_t0_ROWPTR[i]; p < FOM_MAT_t1_t_t0_ROWPTR[i + 1]; ++p) s += FOM_MAT_t1_t_t0_VALS[p] * x[FOM_MAT_t1_t_t0_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t1_t_t0__d_xi(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t1_t_t0__d_xi_ROWPTR[i]; p < FOM_MAT_t1_t_t0__d_xi_ROWPTR[i + 1]; ++p) s += FOM_MAT_t1_t_t0__d_xi_VALS[p] * x[FOM_MAT_t1_t_t0__d_xi_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_su_state(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 650; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_su_state_ROWPTR[i]; p < FOM_MAT_su_state_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_state_VALS[p] * x[FOM_MAT_su_state_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_su_state__d_xi(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 650; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_su_state__d_xi_ROWPTR[i]; p < FOM_MAT_su_state__d_xi_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_state__d_xi_VALS[p] * x[FOM_MAT_su_state__d_xi_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_su_t_t0(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 650; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_su_t_t0_ROWPTR[i]; p < FOM_MAT_su_t_t0_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t0_VALS[p] * x[FOM_MAT_su_t_t0_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_su_t_t0__d_xi(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 650; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_su_t_t0__d_xi_ROWPTR[i]; p < FOM_MAT_su_t_t0__d_xi_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t0__d_xi_VALS[p] * x[FOM_MAT_su_t_t0__d_xi_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_su_t_t1(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 650; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_su_t_t1_ROWPTR[i]; p < FOM_MAT_su_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t1_VALS[p] * x[FOM_MAT_su_t_t1_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_su_p_p0(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 650; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_su_p_p0_ROWPTR[i]; p < FOM_MAT_su_p_p0_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_p_p0_VALS[p] * x[FOM_MAT_su_p_p0_COLIDX[p]];
        out[i] += s;
    }
}

void fom_fused_t1(const fom_float *state, const fom_float *t_t0, fom_float sc_xi, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
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
    for (i = 0; i < 650; ++i) {
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
    fom_int i, r, p;
    for (r = 0; r < 600; ++r) {
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

void fom_step_t1(const fom_float *state, const fom_float *t_t0, fom_float sc_xi, const fom_float *bias, const FomAct_t1 *act, fom_float *out) {
    fom_int i, r, p;
    for (r = 0; r < 20; ++r) {
        fom_float s, v;
        i = r;
        v = bias[i];
        s = 0.0;
        for (p = FOM_MAT_t1_state_ROWPTR[i]; p < FOM_MAT_t1_state_ROWPTR[i + 1]; ++p) s += FOM_MAT_t1_state_VALS[p] * state[FOM_MAT_t1_state_COLIDX[p]];
        v += s;
        s = 0.0;
        for (p = FOM_MAT_t1_state__d_xi_ROWPTR[i]; p < FOM_MAT_t1_state__d_xi_ROWPTR[i + 1]; ++p) s += FOM_MAT_t1_state__d_xi_VALS[p] * state[FOM_MAT_t1_state__d_xi_COLIDX[p]];
        v += sc_xi * s;
        s = 0.0;
        for (p = FOM_MAT_t1_t_t0_ROWPTR[i]; p < FOM_MAT_t1_t_t0_ROWPTR[i + 1]; ++p) s += FOM_MAT_t1_t_t0_VALS[p] * t_t0[FOM_MAT_t1_t_t0_COLIDX[p]];
        v += s;
        s = 0.0;
        for (p = FOM_MAT_t1_t_t0__d_xi_ROWPTR[i]; p < FOM_MAT_t1_t_t0__d_xi_ROWPTR[i + 1]; ++p) s += FOM_MAT_t1_t_t0__d_xi_VALS[p] * t_t0[FOM_MAT_t1_t_t0__d_xi_COLIDX[p]];
        v += sc_xi * s;
        if (v < act->lo[i]) v = act->lo[i];
        out[i] = v;
    }
}

void fom_step_su(const fom_float *state, const fom_float *t_t0, const fom_float *t_t1, fom_float sc_xi, const fom_float *bias, fom_float *out) {
    fom_int i, r, p;
    for (r = 0; r < 650; ++r) {
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
    fom_int i, r, p;
    for (r = 0; r < FOM_REST_su_N; ++r) {
        fom_float s, v;
        i = FOM_REST_su_ROWS[r];
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

void fom_iterate(const fom_float *state, fom_float sc_xi, const fom_float *bias_t0, const fom_float *bias_t1, const fom_float *bias_su, const FomAct_t0 *act_t0, const FomAct_t1 *act_t1, fom_float *t_t0, fom_float *t_t1, fom_float *new_state) {
    fom_step_t0(state, bias_t0, act_t0, t_t0);
    fom_step_t1(state, t_t0, sc_xi, bias_t1, act_t1, t_t1);
    fom_step_su(state, t_t0, t_t1, sc_xi, bias_su, new_state);
}
