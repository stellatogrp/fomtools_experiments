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
    /* t0_state has exactly 3 nonzeros per row (ROWPTR[i] = 3 i). */
    const fom_int *restrict ci = FOM_MAT_t0_state_COLIDX;
    const fom_float *restrict va = FOM_MAT_t0_state_VALS;
    const fom_float *restrict lo = act->lo, *restrict hi = act->hi;
    fom_int i;
    /* Fixed pattern (checked on every unit vector): row 30 j + k reads
       state[30 j + k] (coefficient 1), state[600 + j] and state[620 + k].
       No gathers, so the inner loop vectorizes. */
    fom_int j, k;
    (void)ci;
    for (j = 0; j < 20; ++j) {
        const fom_float xj = state[600 + j];
        const fom_float *restrict xr = state + 30 * j;
        const fom_float *restrict xk = state + 620;
        const fom_float *restrict b = bias + 30 * j;
        const fom_float *restrict l = lo + 30 * j, *restrict h = hi + 30 * j;
        const fom_float *restrict v3 = va + 90 * j;
        fom_float *restrict o = out + 30 * j;
        for (k = 0; k < 30; ++k) {
            fom_float v = b[k] + xr[k] + v3[3 * k + 1] * xj + v3[3 * k + 2] * xk[k];
            v = v < l[k] ? l[k] : v;
            v = v > h[k] ? h[k] : v;
            o[k] = v;
        }
    }
    (void)i;
}

void fom_step_t1(const fom_float *state, const fom_float *t_t0, fom_float sc_xi, const fom_float *bias, const FomAct_t1 *act, fom_float *out) {
    /* Fixed pattern (checked on every unit vector): row i of t1_state reads
       columns 30 i .. 30 i + 29 then 600 + i; t1_state__d_xi, t1_t_t0 and
       t1_t_t0__d_xi read the same 30 columns, and
       t1_t_t0__d_xi = -t1_state__d_xi, so xi (D x + E t) = xi E (t - x). */
    const fom_float *restrict lo = act->lo;
    fom_int i, k;
    for (i = 0; i < 20; ++i) {
        const fom_float *restrict xb = state + 30 * i;
        const fom_float *restrict tb = t_t0 + 30 * i;
        const fom_float *restrict va = FOM_MAT_t1_state_VALS + FOM_MAT_t1_state_ROWPTR[i];
        const fom_float *restrict wa = FOM_MAT_t1_t_t0_VALS + FOM_MAT_t1_t_t0_ROWPTR[i];
        const fom_float *restrict vd = FOM_MAT_t1_t_t0__d_xi_VALS + FOM_MAT_t1_t_t0__d_xi_ROWPTR[i];
        fom_float sa[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0}, da[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
        fom_float s, ud, v;
        fom_int m;
        /* Six independent partial sums break the serial FMA chain. */
        for (k = 0; k < 30; k += 6) {
            for (m = 0; m < 6; ++m) {
                sa[m] += va[k + m] * xb[k + m] + wa[k + m] * tb[k + m];
                da[m] += vd[k + m] * (tb[k + m] - xb[k + m]);
            }
        }
        s = (sa[0] + sa[1]) + (sa[2] + sa[3]) + (sa[4] + sa[5]);
        ud = (da[0] + da[1]) + (da[2] + da[3]) + (da[4] + da[5]);
        v = bias[i] + va[30] * state[600 + i] + s + sc_xi * ud;
        if (v < lo[i]) v = lo[i];
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
    /* Fixed pattern (checked on every unit vector): rest rows are 620 + k,
       k < 30. Row 620 + k of su_state reads columns 30 j + k (j < 20) then
       620 + k; su_state__d_xi, su_t_t0, su_t_t0__d_xi read the same 30 j + k
       and su_state__d_xi = -su_t_t0__d_xi there, so
       xi (D x + E t) = xi E (t - x). su_t_t1 is empty on these rows.
       Loop j outside so the 30 rows run as contiguous vectors. */
    const fom_float *restrict S = FOM_MAT_su_state_VALS + FOM_MAT_su_state_ROWPTR[620];
    const fom_float *restrict T = FOM_MAT_su_t_t0_VALS + FOM_MAT_su_t_t0_ROWPTR[620];
    const fom_float *restrict E = FOM_MAT_su_t_t0__d_xi_VALS + FOM_MAT_su_t_t0__d_xi_ROWPTR[620];
    fom_float acc[30], ud[30];
    fom_int j, k;
    (void)t_t1;
    for (k = 0; k < 30; ++k) { acc[k] = bias[620 + k] + S[21 * k + 20] * state[620 + k]; ud[k] = 0.0; }
    for (j = 0; j < 20; ++j) {
        const fom_float *restrict xr = state + 30 * j;
        const fom_float *restrict tr = t_t0 + 30 * j;
        for (k = 0; k < 30; ++k) {
            acc[k] += S[21 * k + j] * xr[k] + T[20 * k + j] * tr[k];
            ud[k] += E[20 * k + j] * (tr[k] - xr[k]);
        }
    }
    for (k = 0; k < 30; ++k) out[620 + k] = acc[k] + sc_xi * ud[k];
}

void fom_iterate(const fom_float *state, fom_float sc_xi, const fom_float *bias_t0, const fom_float *bias_t1, const fom_float *bias_su, const FomAct_t0 *act_t0, const FomAct_t1 *act_t1, fom_float *t_t0, fom_float *t_t1, fom_float *new_state) {
    /* One pass over the 20 blocks j (30 entries each): t0 block j, then
       t1[j] (reads only state/t0 block j), then the su rest rows 620 + k
       accumulate block j. Same fixed patterns as the step routines above;
       the strided coefficients are repacked once into contiguous tables.
       t_t0 / t_t1 may alias new_state (selection rows), so no restrict. */
    static fom_float A1[600], A2[600], ST[600], TT[600], ET[600];
    static int init = 0;
    const fom_float *restrict lo0 = act_t0->lo, *restrict hi0 = act_t0->hi;
    const fom_float *restrict lo1 = act_t1->lo;
    const fom_float *restrict S = FOM_MAT_su_state_VALS + FOM_MAT_su_state_ROWPTR[620];
    fom_float acc[30], ud[30];
    fom_int j, k;
    if (!init) {
        const fom_float *T = FOM_MAT_su_t_t0_VALS + FOM_MAT_su_t_t0_ROWPTR[620];
        const fom_float *E = FOM_MAT_su_t_t0__d_xi_VALS + FOM_MAT_su_t_t0__d_xi_ROWPTR[620];
        for (j = 0; j < 20; ++j)
            for (k = 0; k < 30; ++k) {
                A1[30 * j + k] = FOM_MAT_t0_state_VALS[3 * (30 * j + k) + 1];
                A2[30 * j + k] = FOM_MAT_t0_state_VALS[3 * (30 * j + k) + 2];
                ST[30 * j + k] = S[21 * k + j];
                TT[30 * j + k] = T[20 * k + j];
                ET[30 * j + k] = E[20 * k + j];
            }
        init = 1;
    }
    for (k = 0; k < 30; ++k) { acc[k] = bias_su[620 + k] + S[21 * k + 20] * state[620 + k]; ud[k] = 0.0; }
    for (j = 0; j < 20; ++j) {
        const fom_float *restrict xr = state + 30 * j;
        const fom_float *restrict xk = state + 620;
        const fom_float xj = state[600 + j];
        const fom_float *restrict b = bias_t0 + 30 * j;
        const fom_float *restrict l = lo0 + 30 * j, *restrict h = hi0 + 30 * j;
        const fom_float *restrict a1 = A1 + 30 * j, *restrict a2 = A2 + 30 * j;
        const fom_float *restrict st = ST + 30 * j, *restrict tt = TT + 30 * j, *restrict et = ET + 30 * j;
        const fom_float *restrict va = FOM_MAT_t1_state_VALS + FOM_MAT_t1_state_ROWPTR[j];
        const fom_float *restrict wa = FOM_MAT_t1_t_t0_VALS + FOM_MAT_t1_t_t0_ROWPTR[j];
        const fom_float *restrict vd = FOM_MAT_t1_t_t0__d_xi_VALS + FOM_MAT_t1_t_t0__d_xi_ROWPTR[j];
        fom_float tb[30];
        fom_float sa[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0}, da[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
        fom_float s, d, v;
        fom_int m;
        for (k = 0; k < 30; ++k) {
            fom_float t = b[k] + xr[k] + a1[k] * xj + a2[k] * xk[k];
            t = t < l[k] ? l[k] : t;
            t = t > h[k] ? h[k] : t;
            tb[k] = t;
            acc[k] += st[k] * xr[k] + tt[k] * t;
            ud[k] += et[k] * (t - xr[k]);
        }
        /* Six independent partial sums break the serial FMA chain. */
        for (k = 0; k < 30; k += 6)
            for (m = 0; m < 6; ++m) {
                sa[m] += va[k + m] * xr[k + m] + wa[k + m] * tb[k + m];
                da[m] += vd[k + m] * (tb[k + m] - xr[k + m]);
            }
        s = (sa[0] + sa[1]) + (sa[2] + sa[3]) + (sa[4] + sa[5]);
        d = (da[0] + da[1]) + (da[2] + da[3]) + (da[4] + da[5]);
        /* When t_t0 aliases new_state (solver selection rows), skip the
           redundant copy; new_state is written last either way. */
        if (t_t0 != new_state)
            for (k = 0; k < 30; ++k) t_t0[30 * j + k] = tb[k];
        for (k = 0; k < 30; ++k) new_state[30 * j + k] = bias_su[30 * j + k] + tb[k];
        v = bias_t1[j] + va[30] * xj + s + sc_xi * d;
        if (v < lo1[j]) v = lo1[j];
        t_t1[j] = v;
        new_state[600 + j] = bias_su[600 + j] + v;
    }
    for (k = 0; k < 30; ++k) new_state[620 + k] = acc[k] + sc_xi * ud[k];
}
