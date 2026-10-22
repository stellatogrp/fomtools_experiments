#include "fom_ops.h"
#include "fom_data.h"

void fom_apply_t0_state(const fom_float *x, fom_float *out) {
    fom_int i, j;
    for (i = 0; i < 50; ++i) {
        fom_float s = 0.0;
        for (j = 0; j < 50; ++j) s += FOM_MAT_t0_state[i * 50 + j] * x[j];
        out[i] += s;
    }
}

void fom_apply_t0_p_p0(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 50; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t0_p_p0_ROWPTR[i]; p < FOM_MAT_t0_p_p0_ROWPTR[i + 1]; ++p) s += FOM_MAT_t0_p_p0_VALS[p] * x[FOM_MAT_t0_p_p0_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_su_t_t0(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 50; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_su_t_t0_ROWPTR[i]; p < FOM_MAT_su_t_t0_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t0_VALS[p] * x[FOM_MAT_su_t_t0_COLIDX[p]];
        out[i] += s;
    }
}

/* Column-major copy of t0_state, built on first use, so the product is a
   sequence of contiguous axpys (vectorizes without reassociation) and the
   columns whose state entry is zero (soft-threshold sparsity) are skipped. */
static fom_float t0_state_T[2500];
static int t0_state_T_ready = 0;

static void t0_state_T_init(void) {
    fom_int i, j;
    for (i = 0; i < 50; ++i)
        for (j = 0; j < 50; ++j) t0_state_T[j * 50 + i] = FOM_MAT_t0_state[i * 50 + j];
    t0_state_T_ready = 1;
}

void fom_step_t0(const fom_float *state, const fom_float *bias, const FomAct_t0 *act, fom_float *out) {
    fom_int i, j;
    fom_float v[50];
    const fom_float lam = act->lambd;
    if (!t0_state_T_ready) t0_state_T_init();
    for (i = 0; i < 50; ++i) v[i] = bias[i];
    fom_int nz[50], n = 0, k;
    for (j = 0; j < 50; ++j) { nz[n] = j; n += (state[j] != 0.0); }
    for (k = 0; k + 1 < n; k += 2) {
        const fom_float xa = state[nz[k]], xb = state[nz[k + 1]];
        const fom_float *ca = t0_state_T + nz[k] * 50, *cb = t0_state_T + nz[k + 1] * 50;
        for (i = 0; i < 50; ++i) v[i] = (v[i] + ca[i] * xa) + cb[i] * xb;
    }
    if (k < n) {
        const fom_float xa = state[nz[k]];
        const fom_float *ca = t0_state_T + nz[k] * 50;
        for (i = 0; i < 50; ++i) v[i] += ca[i] * xa;
    }
    for (i = 0; i < 50; ++i) {
        fom_float a = v[i];
        out[i] = (a > lam) ? (a - lam) : ((a < -lam) ? (a + lam) : 0.0);
    }
}

void fom_step_su(const fom_float *t_t0, const fom_float *bias, fom_float *out) {
    fom_int i, r, p;
    for (r = 0; r < 50; ++r) {
        fom_float s, v;
        i = r;
        v = bias[i];
        s = 0.0;
        for (p = FOM_MAT_su_t_t0_ROWPTR[i]; p < FOM_MAT_su_t_t0_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t0_VALS[p] * t_t0[FOM_MAT_su_t_t0_COLIDX[p]];
        v += s;
        out[i] = v;
    }
}

void fom_iterate(const fom_float *state, const fom_float *bias_t0, const fom_float *bias_su, const FomAct_t0 *act_t0, fom_float *t_t0, fom_float *new_state) {
    fom_step_t0(state, bias_t0, act_t0, t_t0);
    fom_step_su(t_t0, bias_su, new_state);
}
