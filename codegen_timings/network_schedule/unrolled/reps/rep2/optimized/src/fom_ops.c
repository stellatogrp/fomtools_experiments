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

void fom_apply_t1_t_t0(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t1_t_t0_ROWPTR[i]; p < FOM_MAT_t1_t_t0_ROWPTR[i + 1]; ++p) s += FOM_MAT_t1_t_t0_VALS[p] * x[FOM_MAT_t1_t_t0_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t2_state(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 600; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t2_state_ROWPTR[i]; p < FOM_MAT_t2_state_ROWPTR[i + 1]; ++p) s += FOM_MAT_t2_state_VALS[p] * x[FOM_MAT_t2_state_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t2_p_p0(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 600; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t2_p_p0_ROWPTR[i]; p < FOM_MAT_t2_p_p0_ROWPTR[i + 1]; ++p) s += FOM_MAT_t2_p_p0_VALS[p] * x[FOM_MAT_t2_p_p0_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t2_t_t0(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 600; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t2_t_t0_ROWPTR[i]; p < FOM_MAT_t2_t_t0_ROWPTR[i + 1]; ++p) s += FOM_MAT_t2_t_t0_VALS[p] * x[FOM_MAT_t2_t_t0_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t2_t_t1(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 600; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t2_t_t1_ROWPTR[i]; p < FOM_MAT_t2_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t2_t_t1_VALS[p] * x[FOM_MAT_t2_t_t1_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t3_t_t0(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t3_t_t0_ROWPTR[i]; p < FOM_MAT_t3_t_t0_ROWPTR[i + 1]; ++p) s += FOM_MAT_t3_t_t0_VALS[p] * x[FOM_MAT_t3_t_t0_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t3_t_t1(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t3_t_t1_ROWPTR[i]; p < FOM_MAT_t3_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t3_t_t1_VALS[p] * x[FOM_MAT_t3_t_t1_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t3_t_t2(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t3_t_t2_ROWPTR[i]; p < FOM_MAT_t3_t_t2_ROWPTR[i + 1]; ++p) s += FOM_MAT_t3_t_t2_VALS[p] * x[FOM_MAT_t3_t_t2_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t4_state(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t4_state_ROWPTR[i]; p < FOM_MAT_t4_state_ROWPTR[i + 1]; ++p) s += FOM_MAT_t4_state_VALS[p] * x[FOM_MAT_t4_state_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t4_p_p0(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t4_p_p0_ROWPTR[i]; p < FOM_MAT_t4_p_p0_ROWPTR[i + 1]; ++p) s += FOM_MAT_t4_p_p0_VALS[p] * x[FOM_MAT_t4_p_p0_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t4_t_t0(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t4_t_t0_ROWPTR[i]; p < FOM_MAT_t4_t_t0_ROWPTR[i + 1]; ++p) s += FOM_MAT_t4_t_t0_VALS[p] * x[FOM_MAT_t4_t_t0_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t4_t_t2(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t4_t_t2_ROWPTR[i]; p < FOM_MAT_t4_t_t2_ROWPTR[i + 1]; ++p) s += FOM_MAT_t4_t_t2_VALS[p] * x[FOM_MAT_t4_t_t2_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t5_t_t2(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 600; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t5_t_t2_ROWPTR[i]; p < FOM_MAT_t5_t_t2_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t2_VALS[p] * x[FOM_MAT_t5_t_t2_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t5_t_t4(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 600; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t5_t_t4_ROWPTR[i]; p < FOM_MAT_t5_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t4_VALS[p] * x[FOM_MAT_t5_t_t4_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t6_t_t2(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t6_t_t2_ROWPTR[i]; p < FOM_MAT_t6_t_t2_ROWPTR[i + 1]; ++p) s += FOM_MAT_t6_t_t2_VALS[p] * x[FOM_MAT_t6_t_t2_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t6_t_t5(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t6_t_t5_ROWPTR[i]; p < FOM_MAT_t6_t_t5_ROWPTR[i + 1]; ++p) s += FOM_MAT_t6_t_t5_VALS[p] * x[FOM_MAT_t6_t_t5_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t7_p_p0(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t7_p_p0_ROWPTR[i]; p < FOM_MAT_t7_p_p0_ROWPTR[i + 1]; ++p) s += FOM_MAT_t7_p_p0_VALS[p] * x[FOM_MAT_t7_p_p0_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t7_t_t2(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t7_t_t2_ROWPTR[i]; p < FOM_MAT_t7_t_t2_ROWPTR[i + 1]; ++p) s += FOM_MAT_t7_t_t2_VALS[p] * x[FOM_MAT_t7_t_t2_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t7_t_t4(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t7_t_t4_ROWPTR[i]; p < FOM_MAT_t7_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t7_t_t4_VALS[p] * x[FOM_MAT_t7_t_t4_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t7_t_t5(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t7_t_t5_ROWPTR[i]; p < FOM_MAT_t7_t_t5_ROWPTR[i + 1]; ++p) s += FOM_MAT_t7_t_t5_VALS[p] * x[FOM_MAT_t7_t_t5_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t9_t_t5(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t9_t_t5_ROWPTR[i]; p < FOM_MAT_t9_t_t5_ROWPTR[i + 1]; ++p) s += FOM_MAT_t9_t_t5_VALS[p] * x[FOM_MAT_t9_t_t5_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t9_t_t8(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t9_t_t8_ROWPTR[i]; p < FOM_MAT_t9_t_t8_ROWPTR[i + 1]; ++p) s += FOM_MAT_t9_t_t8_VALS[p] * x[FOM_MAT_t9_t_t8_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t10_t_t5(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t10_t_t5_ROWPTR[i]; p < FOM_MAT_t10_t_t5_ROWPTR[i + 1]; ++p) s += FOM_MAT_t10_t_t5_VALS[p] * x[FOM_MAT_t10_t_t5_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t10_t_t8(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t10_t_t8_ROWPTR[i]; p < FOM_MAT_t10_t_t8_ROWPTR[i + 1]; ++p) s += FOM_MAT_t10_t_t8_VALS[p] * x[FOM_MAT_t10_t_t8_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t12_t_t8(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t12_t_t8_ROWPTR[i]; p < FOM_MAT_t12_t_t8_ROWPTR[i + 1]; ++p) s += FOM_MAT_t12_t_t8_VALS[p] * x[FOM_MAT_t12_t_t8_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t12_t_t11(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t12_t_t11_ROWPTR[i]; p < FOM_MAT_t12_t_t11_ROWPTR[i + 1]; ++p) s += FOM_MAT_t12_t_t11_VALS[p] * x[FOM_MAT_t12_t_t11_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t13_t_t8(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t13_t_t8_ROWPTR[i]; p < FOM_MAT_t13_t_t8_ROWPTR[i + 1]; ++p) s += FOM_MAT_t13_t_t8_VALS[p] * x[FOM_MAT_t13_t_t8_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t13_t_t11(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t13_t_t11_ROWPTR[i]; p < FOM_MAT_t13_t_t11_ROWPTR[i + 1]; ++p) s += FOM_MAT_t13_t_t11_VALS[p] * x[FOM_MAT_t13_t_t11_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t15_t_t11(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t15_t_t11_ROWPTR[i]; p < FOM_MAT_t15_t_t11_ROWPTR[i + 1]; ++p) s += FOM_MAT_t15_t_t11_VALS[p] * x[FOM_MAT_t15_t_t11_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t15_t_t14(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t15_t_t14_ROWPTR[i]; p < FOM_MAT_t15_t_t14_ROWPTR[i + 1]; ++p) s += FOM_MAT_t15_t_t14_VALS[p] * x[FOM_MAT_t15_t_t14_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t16_t_t11(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t16_t_t11_ROWPTR[i]; p < FOM_MAT_t16_t_t11_ROWPTR[i + 1]; ++p) s += FOM_MAT_t16_t_t11_VALS[p] * x[FOM_MAT_t16_t_t11_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t16_t_t14(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t16_t_t14_ROWPTR[i]; p < FOM_MAT_t16_t_t14_ROWPTR[i + 1]; ++p) s += FOM_MAT_t16_t_t14_VALS[p] * x[FOM_MAT_t16_t_t14_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t18_t_t14(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t18_t_t14_ROWPTR[i]; p < FOM_MAT_t18_t_t14_ROWPTR[i + 1]; ++p) s += FOM_MAT_t18_t_t14_VALS[p] * x[FOM_MAT_t18_t_t14_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t18_t_t17(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t18_t_t17_ROWPTR[i]; p < FOM_MAT_t18_t_t17_ROWPTR[i + 1]; ++p) s += FOM_MAT_t18_t_t17_VALS[p] * x[FOM_MAT_t18_t_t17_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t19_t_t14(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t19_t_t14_ROWPTR[i]; p < FOM_MAT_t19_t_t14_ROWPTR[i + 1]; ++p) s += FOM_MAT_t19_t_t14_VALS[p] * x[FOM_MAT_t19_t_t14_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t19_t_t17(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t19_t_t17_ROWPTR[i]; p < FOM_MAT_t19_t_t17_ROWPTR[i + 1]; ++p) s += FOM_MAT_t19_t_t17_VALS[p] * x[FOM_MAT_t19_t_t17_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t21_t_t17(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t21_t_t17_ROWPTR[i]; p < FOM_MAT_t21_t_t17_ROWPTR[i + 1]; ++p) s += FOM_MAT_t21_t_t17_VALS[p] * x[FOM_MAT_t21_t_t17_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t21_t_t20(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t21_t_t20_ROWPTR[i]; p < FOM_MAT_t21_t_t20_ROWPTR[i + 1]; ++p) s += FOM_MAT_t21_t_t20_VALS[p] * x[FOM_MAT_t21_t_t20_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t22_t_t17(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t22_t_t17_ROWPTR[i]; p < FOM_MAT_t22_t_t17_ROWPTR[i + 1]; ++p) s += FOM_MAT_t22_t_t17_VALS[p] * x[FOM_MAT_t22_t_t17_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t22_t_t20(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t22_t_t20_ROWPTR[i]; p < FOM_MAT_t22_t_t20_ROWPTR[i + 1]; ++p) s += FOM_MAT_t22_t_t20_VALS[p] * x[FOM_MAT_t22_t_t20_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t24_t_t20(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t24_t_t20_ROWPTR[i]; p < FOM_MAT_t24_t_t20_ROWPTR[i + 1]; ++p) s += FOM_MAT_t24_t_t20_VALS[p] * x[FOM_MAT_t24_t_t20_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t24_t_t23(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t24_t_t23_ROWPTR[i]; p < FOM_MAT_t24_t_t23_ROWPTR[i + 1]; ++p) s += FOM_MAT_t24_t_t23_VALS[p] * x[FOM_MAT_t24_t_t23_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t25_t_t20(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t25_t_t20_ROWPTR[i]; p < FOM_MAT_t25_t_t20_ROWPTR[i + 1]; ++p) s += FOM_MAT_t25_t_t20_VALS[p] * x[FOM_MAT_t25_t_t20_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t25_t_t23(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t25_t_t23_ROWPTR[i]; p < FOM_MAT_t25_t_t23_ROWPTR[i + 1]; ++p) s += FOM_MAT_t25_t_t23_VALS[p] * x[FOM_MAT_t25_t_t23_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t27_t_t23(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t27_t_t23_ROWPTR[i]; p < FOM_MAT_t27_t_t23_ROWPTR[i + 1]; ++p) s += FOM_MAT_t27_t_t23_VALS[p] * x[FOM_MAT_t27_t_t23_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t27_t_t26(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t27_t_t26_ROWPTR[i]; p < FOM_MAT_t27_t_t26_ROWPTR[i + 1]; ++p) s += FOM_MAT_t27_t_t26_VALS[p] * x[FOM_MAT_t27_t_t26_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t28_t_t23(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t28_t_t23_ROWPTR[i]; p < FOM_MAT_t28_t_t23_ROWPTR[i + 1]; ++p) s += FOM_MAT_t28_t_t23_VALS[p] * x[FOM_MAT_t28_t_t23_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t28_t_t26(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t28_t_t26_ROWPTR[i]; p < FOM_MAT_t28_t_t26_ROWPTR[i + 1]; ++p) s += FOM_MAT_t28_t_t26_VALS[p] * x[FOM_MAT_t28_t_t26_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t30_t_t26(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t30_t_t26_ROWPTR[i]; p < FOM_MAT_t30_t_t26_ROWPTR[i + 1]; ++p) s += FOM_MAT_t30_t_t26_VALS[p] * x[FOM_MAT_t30_t_t26_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t30_t_t29(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t30_t_t29_ROWPTR[i]; p < FOM_MAT_t30_t_t29_ROWPTR[i + 1]; ++p) s += FOM_MAT_t30_t_t29_VALS[p] * x[FOM_MAT_t30_t_t29_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t31_t_t26(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t31_t_t26_ROWPTR[i]; p < FOM_MAT_t31_t_t26_ROWPTR[i + 1]; ++p) s += FOM_MAT_t31_t_t26_VALS[p] * x[FOM_MAT_t31_t_t26_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t31_t_t29(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t31_t_t29_ROWPTR[i]; p < FOM_MAT_t31_t_t29_ROWPTR[i + 1]; ++p) s += FOM_MAT_t31_t_t29_VALS[p] * x[FOM_MAT_t31_t_t29_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t33_t_t29(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t33_t_t29_ROWPTR[i]; p < FOM_MAT_t33_t_t29_ROWPTR[i + 1]; ++p) s += FOM_MAT_t33_t_t29_VALS[p] * x[FOM_MAT_t33_t_t29_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t33_t_t32(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t33_t_t32_ROWPTR[i]; p < FOM_MAT_t33_t_t32_ROWPTR[i + 1]; ++p) s += FOM_MAT_t33_t_t32_VALS[p] * x[FOM_MAT_t33_t_t32_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t34_t_t29(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t34_t_t29_ROWPTR[i]; p < FOM_MAT_t34_t_t29_ROWPTR[i + 1]; ++p) s += FOM_MAT_t34_t_t29_VALS[p] * x[FOM_MAT_t34_t_t29_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t34_t_t32(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t34_t_t32_ROWPTR[i]; p < FOM_MAT_t34_t_t32_ROWPTR[i + 1]; ++p) s += FOM_MAT_t34_t_t32_VALS[p] * x[FOM_MAT_t34_t_t32_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t36_t_t32(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t36_t_t32_ROWPTR[i]; p < FOM_MAT_t36_t_t32_ROWPTR[i + 1]; ++p) s += FOM_MAT_t36_t_t32_VALS[p] * x[FOM_MAT_t36_t_t32_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t36_t_t35(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t36_t_t35_ROWPTR[i]; p < FOM_MAT_t36_t_t35_ROWPTR[i + 1]; ++p) s += FOM_MAT_t36_t_t35_VALS[p] * x[FOM_MAT_t36_t_t35_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t37_t_t32(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t37_t_t32_ROWPTR[i]; p < FOM_MAT_t37_t_t32_ROWPTR[i + 1]; ++p) s += FOM_MAT_t37_t_t32_VALS[p] * x[FOM_MAT_t37_t_t32_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t37_t_t35(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t37_t_t35_ROWPTR[i]; p < FOM_MAT_t37_t_t35_ROWPTR[i + 1]; ++p) s += FOM_MAT_t37_t_t35_VALS[p] * x[FOM_MAT_t37_t_t35_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t39_t_t35(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t39_t_t35_ROWPTR[i]; p < FOM_MAT_t39_t_t35_ROWPTR[i + 1]; ++p) s += FOM_MAT_t39_t_t35_VALS[p] * x[FOM_MAT_t39_t_t35_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t39_t_t38(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t39_t_t38_ROWPTR[i]; p < FOM_MAT_t39_t_t38_ROWPTR[i + 1]; ++p) s += FOM_MAT_t39_t_t38_VALS[p] * x[FOM_MAT_t39_t_t38_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t40_t_t35(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t40_t_t35_ROWPTR[i]; p < FOM_MAT_t40_t_t35_ROWPTR[i + 1]; ++p) s += FOM_MAT_t40_t_t35_VALS[p] * x[FOM_MAT_t40_t_t35_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t40_t_t38(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t40_t_t38_ROWPTR[i]; p < FOM_MAT_t40_t_t38_ROWPTR[i + 1]; ++p) s += FOM_MAT_t40_t_t38_VALS[p] * x[FOM_MAT_t40_t_t38_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t42_t_t38(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t42_t_t38_ROWPTR[i]; p < FOM_MAT_t42_t_t38_ROWPTR[i + 1]; ++p) s += FOM_MAT_t42_t_t38_VALS[p] * x[FOM_MAT_t42_t_t38_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t42_t_t41(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t42_t_t41_ROWPTR[i]; p < FOM_MAT_t42_t_t41_ROWPTR[i + 1]; ++p) s += FOM_MAT_t42_t_t41_VALS[p] * x[FOM_MAT_t42_t_t41_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t43_t_t38(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t43_t_t38_ROWPTR[i]; p < FOM_MAT_t43_t_t38_ROWPTR[i + 1]; ++p) s += FOM_MAT_t43_t_t38_VALS[p] * x[FOM_MAT_t43_t_t38_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t43_t_t41(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t43_t_t41_ROWPTR[i]; p < FOM_MAT_t43_t_t41_ROWPTR[i + 1]; ++p) s += FOM_MAT_t43_t_t41_VALS[p] * x[FOM_MAT_t43_t_t41_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t45_t_t41(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t45_t_t41_ROWPTR[i]; p < FOM_MAT_t45_t_t41_ROWPTR[i + 1]; ++p) s += FOM_MAT_t45_t_t41_VALS[p] * x[FOM_MAT_t45_t_t41_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t45_t_t44(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t45_t_t44_ROWPTR[i]; p < FOM_MAT_t45_t_t44_ROWPTR[i + 1]; ++p) s += FOM_MAT_t45_t_t44_VALS[p] * x[FOM_MAT_t45_t_t44_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t46_t_t41(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t46_t_t41_ROWPTR[i]; p < FOM_MAT_t46_t_t41_ROWPTR[i + 1]; ++p) s += FOM_MAT_t46_t_t41_VALS[p] * x[FOM_MAT_t46_t_t41_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t46_t_t44(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t46_t_t44_ROWPTR[i]; p < FOM_MAT_t46_t_t44_ROWPTR[i + 1]; ++p) s += FOM_MAT_t46_t_t44_VALS[p] * x[FOM_MAT_t46_t_t44_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t48_t_t44(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t48_t_t44_ROWPTR[i]; p < FOM_MAT_t48_t_t44_ROWPTR[i + 1]; ++p) s += FOM_MAT_t48_t_t44_VALS[p] * x[FOM_MAT_t48_t_t44_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t48_t_t47(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t48_t_t47_ROWPTR[i]; p < FOM_MAT_t48_t_t47_ROWPTR[i + 1]; ++p) s += FOM_MAT_t48_t_t47_VALS[p] * x[FOM_MAT_t48_t_t47_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t49_t_t44(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t49_t_t44_ROWPTR[i]; p < FOM_MAT_t49_t_t44_ROWPTR[i + 1]; ++p) s += FOM_MAT_t49_t_t44_VALS[p] * x[FOM_MAT_t49_t_t44_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t49_t_t47(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t49_t_t47_ROWPTR[i]; p < FOM_MAT_t49_t_t47_ROWPTR[i + 1]; ++p) s += FOM_MAT_t49_t_t47_VALS[p] * x[FOM_MAT_t49_t_t47_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t51_t_t47(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t51_t_t47_ROWPTR[i]; p < FOM_MAT_t51_t_t47_ROWPTR[i + 1]; ++p) s += FOM_MAT_t51_t_t47_VALS[p] * x[FOM_MAT_t51_t_t47_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t51_t_t50(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t51_t_t50_ROWPTR[i]; p < FOM_MAT_t51_t_t50_ROWPTR[i + 1]; ++p) s += FOM_MAT_t51_t_t50_VALS[p] * x[FOM_MAT_t51_t_t50_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t52_t_t47(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t52_t_t47_ROWPTR[i]; p < FOM_MAT_t52_t_t47_ROWPTR[i + 1]; ++p) s += FOM_MAT_t52_t_t47_VALS[p] * x[FOM_MAT_t52_t_t47_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t52_t_t50(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t52_t_t50_ROWPTR[i]; p < FOM_MAT_t52_t_t50_ROWPTR[i + 1]; ++p) s += FOM_MAT_t52_t_t50_VALS[p] * x[FOM_MAT_t52_t_t50_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t54_t_t50(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t54_t_t50_ROWPTR[i]; p < FOM_MAT_t54_t_t50_ROWPTR[i + 1]; ++p) s += FOM_MAT_t54_t_t50_VALS[p] * x[FOM_MAT_t54_t_t50_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t54_t_t53(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t54_t_t53_ROWPTR[i]; p < FOM_MAT_t54_t_t53_ROWPTR[i + 1]; ++p) s += FOM_MAT_t54_t_t53_VALS[p] * x[FOM_MAT_t54_t_t53_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t55_t_t50(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t55_t_t50_ROWPTR[i]; p < FOM_MAT_t55_t_t50_ROWPTR[i + 1]; ++p) s += FOM_MAT_t55_t_t50_VALS[p] * x[FOM_MAT_t55_t_t50_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t55_t_t53(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t55_t_t53_ROWPTR[i]; p < FOM_MAT_t55_t_t53_ROWPTR[i + 1]; ++p) s += FOM_MAT_t55_t_t53_VALS[p] * x[FOM_MAT_t55_t_t53_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t57_t_t53(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t57_t_t53_ROWPTR[i]; p < FOM_MAT_t57_t_t53_ROWPTR[i + 1]; ++p) s += FOM_MAT_t57_t_t53_VALS[p] * x[FOM_MAT_t57_t_t53_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t57_t_t56(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t57_t_t56_ROWPTR[i]; p < FOM_MAT_t57_t_t56_ROWPTR[i + 1]; ++p) s += FOM_MAT_t57_t_t56_VALS[p] * x[FOM_MAT_t57_t_t56_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t58_t_t53(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t58_t_t53_ROWPTR[i]; p < FOM_MAT_t58_t_t53_ROWPTR[i + 1]; ++p) s += FOM_MAT_t58_t_t53_VALS[p] * x[FOM_MAT_t58_t_t53_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t58_t_t56(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t58_t_t56_ROWPTR[i]; p < FOM_MAT_t58_t_t56_ROWPTR[i + 1]; ++p) s += FOM_MAT_t58_t_t56_VALS[p] * x[FOM_MAT_t58_t_t56_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t60_t_t56(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t60_t_t56_ROWPTR[i]; p < FOM_MAT_t60_t_t56_ROWPTR[i + 1]; ++p) s += FOM_MAT_t60_t_t56_VALS[p] * x[FOM_MAT_t60_t_t56_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t60_t_t59(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t60_t_t59_ROWPTR[i]; p < FOM_MAT_t60_t_t59_ROWPTR[i + 1]; ++p) s += FOM_MAT_t60_t_t59_VALS[p] * x[FOM_MAT_t60_t_t59_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t61_t_t56(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t61_t_t56_ROWPTR[i]; p < FOM_MAT_t61_t_t56_ROWPTR[i + 1]; ++p) s += FOM_MAT_t61_t_t56_VALS[p] * x[FOM_MAT_t61_t_t56_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t61_t_t59(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t61_t_t59_ROWPTR[i]; p < FOM_MAT_t61_t_t59_ROWPTR[i + 1]; ++p) s += FOM_MAT_t61_t_t59_VALS[p] * x[FOM_MAT_t61_t_t59_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t63_t_t59(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t63_t_t59_ROWPTR[i]; p < FOM_MAT_t63_t_t59_ROWPTR[i + 1]; ++p) s += FOM_MAT_t63_t_t59_VALS[p] * x[FOM_MAT_t63_t_t59_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t63_t_t62(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t63_t_t62_ROWPTR[i]; p < FOM_MAT_t63_t_t62_ROWPTR[i + 1]; ++p) s += FOM_MAT_t63_t_t62_VALS[p] * x[FOM_MAT_t63_t_t62_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t64_t_t59(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t64_t_t59_ROWPTR[i]; p < FOM_MAT_t64_t_t59_ROWPTR[i + 1]; ++p) s += FOM_MAT_t64_t_t59_VALS[p] * x[FOM_MAT_t64_t_t59_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t64_t_t62(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t64_t_t62_ROWPTR[i]; p < FOM_MAT_t64_t_t62_ROWPTR[i + 1]; ++p) s += FOM_MAT_t64_t_t62_VALS[p] * x[FOM_MAT_t64_t_t62_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t66_t_t62(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t66_t_t62_ROWPTR[i]; p < FOM_MAT_t66_t_t62_ROWPTR[i + 1]; ++p) s += FOM_MAT_t66_t_t62_VALS[p] * x[FOM_MAT_t66_t_t62_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t66_t_t65(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t66_t_t65_ROWPTR[i]; p < FOM_MAT_t66_t_t65_ROWPTR[i + 1]; ++p) s += FOM_MAT_t66_t_t65_VALS[p] * x[FOM_MAT_t66_t_t65_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t67_t_t62(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t67_t_t62_ROWPTR[i]; p < FOM_MAT_t67_t_t62_ROWPTR[i + 1]; ++p) s += FOM_MAT_t67_t_t62_VALS[p] * x[FOM_MAT_t67_t_t62_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t67_t_t65(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t67_t_t65_ROWPTR[i]; p < FOM_MAT_t67_t_t65_ROWPTR[i + 1]; ++p) s += FOM_MAT_t67_t_t65_VALS[p] * x[FOM_MAT_t67_t_t65_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t69_t_t65(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t69_t_t65_ROWPTR[i]; p < FOM_MAT_t69_t_t65_ROWPTR[i + 1]; ++p) s += FOM_MAT_t69_t_t65_VALS[p] * x[FOM_MAT_t69_t_t65_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t69_t_t68(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t69_t_t68_ROWPTR[i]; p < FOM_MAT_t69_t_t68_ROWPTR[i + 1]; ++p) s += FOM_MAT_t69_t_t68_VALS[p] * x[FOM_MAT_t69_t_t68_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t70_t_t65(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t70_t_t65_ROWPTR[i]; p < FOM_MAT_t70_t_t65_ROWPTR[i + 1]; ++p) s += FOM_MAT_t70_t_t65_VALS[p] * x[FOM_MAT_t70_t_t65_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t70_t_t68(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t70_t_t68_ROWPTR[i]; p < FOM_MAT_t70_t_t68_ROWPTR[i + 1]; ++p) s += FOM_MAT_t70_t_t68_VALS[p] * x[FOM_MAT_t70_t_t68_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t72_t_t68(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t72_t_t68_ROWPTR[i]; p < FOM_MAT_t72_t_t68_ROWPTR[i + 1]; ++p) s += FOM_MAT_t72_t_t68_VALS[p] * x[FOM_MAT_t72_t_t68_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t72_t_t71(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t72_t_t71_ROWPTR[i]; p < FOM_MAT_t72_t_t71_ROWPTR[i + 1]; ++p) s += FOM_MAT_t72_t_t71_VALS[p] * x[FOM_MAT_t72_t_t71_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t73_t_t68(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t73_t_t68_ROWPTR[i]; p < FOM_MAT_t73_t_t68_ROWPTR[i + 1]; ++p) s += FOM_MAT_t73_t_t68_VALS[p] * x[FOM_MAT_t73_t_t68_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t73_t_t71(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t73_t_t71_ROWPTR[i]; p < FOM_MAT_t73_t_t71_ROWPTR[i + 1]; ++p) s += FOM_MAT_t73_t_t71_VALS[p] * x[FOM_MAT_t73_t_t71_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t75_t_t71(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t75_t_t71_ROWPTR[i]; p < FOM_MAT_t75_t_t71_ROWPTR[i + 1]; ++p) s += FOM_MAT_t75_t_t71_VALS[p] * x[FOM_MAT_t75_t_t71_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t75_t_t74(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t75_t_t74_ROWPTR[i]; p < FOM_MAT_t75_t_t74_ROWPTR[i + 1]; ++p) s += FOM_MAT_t75_t_t74_VALS[p] * x[FOM_MAT_t75_t_t74_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t76_t_t71(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t76_t_t71_ROWPTR[i]; p < FOM_MAT_t76_t_t71_ROWPTR[i + 1]; ++p) s += FOM_MAT_t76_t_t71_VALS[p] * x[FOM_MAT_t76_t_t71_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t76_t_t74(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t76_t_t74_ROWPTR[i]; p < FOM_MAT_t76_t_t74_ROWPTR[i + 1]; ++p) s += FOM_MAT_t76_t_t74_VALS[p] * x[FOM_MAT_t76_t_t74_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t78_t_t74(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t78_t_t74_ROWPTR[i]; p < FOM_MAT_t78_t_t74_ROWPTR[i + 1]; ++p) s += FOM_MAT_t78_t_t74_VALS[p] * x[FOM_MAT_t78_t_t74_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t78_t_t77(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t78_t_t77_ROWPTR[i]; p < FOM_MAT_t78_t_t77_ROWPTR[i + 1]; ++p) s += FOM_MAT_t78_t_t77_VALS[p] * x[FOM_MAT_t78_t_t77_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t79_t_t74(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t79_t_t74_ROWPTR[i]; p < FOM_MAT_t79_t_t74_ROWPTR[i + 1]; ++p) s += FOM_MAT_t79_t_t74_VALS[p] * x[FOM_MAT_t79_t_t74_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t79_t_t77(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t79_t_t77_ROWPTR[i]; p < FOM_MAT_t79_t_t77_ROWPTR[i + 1]; ++p) s += FOM_MAT_t79_t_t77_VALS[p] * x[FOM_MAT_t79_t_t77_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t81_t_t77(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t81_t_t77_ROWPTR[i]; p < FOM_MAT_t81_t_t77_ROWPTR[i + 1]; ++p) s += FOM_MAT_t81_t_t77_VALS[p] * x[FOM_MAT_t81_t_t77_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t81_t_t80(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t81_t_t80_ROWPTR[i]; p < FOM_MAT_t81_t_t80_ROWPTR[i + 1]; ++p) s += FOM_MAT_t81_t_t80_VALS[p] * x[FOM_MAT_t81_t_t80_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t82_t_t77(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t82_t_t77_ROWPTR[i]; p < FOM_MAT_t82_t_t77_ROWPTR[i + 1]; ++p) s += FOM_MAT_t82_t_t77_VALS[p] * x[FOM_MAT_t82_t_t77_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t82_t_t80(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t82_t_t80_ROWPTR[i]; p < FOM_MAT_t82_t_t80_ROWPTR[i + 1]; ++p) s += FOM_MAT_t82_t_t80_VALS[p] * x[FOM_MAT_t82_t_t80_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t84_t_t80(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t84_t_t80_ROWPTR[i]; p < FOM_MAT_t84_t_t80_ROWPTR[i + 1]; ++p) s += FOM_MAT_t84_t_t80_VALS[p] * x[FOM_MAT_t84_t_t80_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t84_t_t83(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t84_t_t83_ROWPTR[i]; p < FOM_MAT_t84_t_t83_ROWPTR[i + 1]; ++p) s += FOM_MAT_t84_t_t83_VALS[p] * x[FOM_MAT_t84_t_t83_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t85_t_t80(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t85_t_t80_ROWPTR[i]; p < FOM_MAT_t85_t_t80_ROWPTR[i + 1]; ++p) s += FOM_MAT_t85_t_t80_VALS[p] * x[FOM_MAT_t85_t_t80_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t85_t_t83(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t85_t_t83_ROWPTR[i]; p < FOM_MAT_t85_t_t83_ROWPTR[i + 1]; ++p) s += FOM_MAT_t85_t_t83_VALS[p] * x[FOM_MAT_t85_t_t83_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t87_t_t83(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t87_t_t83_ROWPTR[i]; p < FOM_MAT_t87_t_t83_ROWPTR[i + 1]; ++p) s += FOM_MAT_t87_t_t83_VALS[p] * x[FOM_MAT_t87_t_t83_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t87_t_t86(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t87_t_t86_ROWPTR[i]; p < FOM_MAT_t87_t_t86_ROWPTR[i + 1]; ++p) s += FOM_MAT_t87_t_t86_VALS[p] * x[FOM_MAT_t87_t_t86_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t88_t_t83(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t88_t_t83_ROWPTR[i]; p < FOM_MAT_t88_t_t83_ROWPTR[i + 1]; ++p) s += FOM_MAT_t88_t_t83_VALS[p] * x[FOM_MAT_t88_t_t83_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t88_t_t86(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t88_t_t86_ROWPTR[i]; p < FOM_MAT_t88_t_t86_ROWPTR[i + 1]; ++p) s += FOM_MAT_t88_t_t86_VALS[p] * x[FOM_MAT_t88_t_t86_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t90_t_t86(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t90_t_t86_ROWPTR[i]; p < FOM_MAT_t90_t_t86_ROWPTR[i + 1]; ++p) s += FOM_MAT_t90_t_t86_VALS[p] * x[FOM_MAT_t90_t_t86_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t90_t_t89(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t90_t_t89_ROWPTR[i]; p < FOM_MAT_t90_t_t89_ROWPTR[i + 1]; ++p) s += FOM_MAT_t90_t_t89_VALS[p] * x[FOM_MAT_t90_t_t89_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t91_t_t86(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t91_t_t86_ROWPTR[i]; p < FOM_MAT_t91_t_t86_ROWPTR[i + 1]; ++p) s += FOM_MAT_t91_t_t86_VALS[p] * x[FOM_MAT_t91_t_t86_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t91_t_t89(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t91_t_t89_ROWPTR[i]; p < FOM_MAT_t91_t_t89_ROWPTR[i + 1]; ++p) s += FOM_MAT_t91_t_t89_VALS[p] * x[FOM_MAT_t91_t_t89_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t93_t_t89(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t93_t_t89_ROWPTR[i]; p < FOM_MAT_t93_t_t89_ROWPTR[i + 1]; ++p) s += FOM_MAT_t93_t_t89_VALS[p] * x[FOM_MAT_t93_t_t89_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t93_t_t92(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t93_t_t92_ROWPTR[i]; p < FOM_MAT_t93_t_t92_ROWPTR[i + 1]; ++p) s += FOM_MAT_t93_t_t92_VALS[p] * x[FOM_MAT_t93_t_t92_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t94_t_t89(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t94_t_t89_ROWPTR[i]; p < FOM_MAT_t94_t_t89_ROWPTR[i + 1]; ++p) s += FOM_MAT_t94_t_t89_VALS[p] * x[FOM_MAT_t94_t_t89_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t94_t_t92(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t94_t_t92_ROWPTR[i]; p < FOM_MAT_t94_t_t92_ROWPTR[i + 1]; ++p) s += FOM_MAT_t94_t_t92_VALS[p] * x[FOM_MAT_t94_t_t92_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t96_t_t92(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t96_t_t92_ROWPTR[i]; p < FOM_MAT_t96_t_t92_ROWPTR[i + 1]; ++p) s += FOM_MAT_t96_t_t92_VALS[p] * x[FOM_MAT_t96_t_t92_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t96_t_t95(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t96_t_t95_ROWPTR[i]; p < FOM_MAT_t96_t_t95_ROWPTR[i + 1]; ++p) s += FOM_MAT_t96_t_t95_VALS[p] * x[FOM_MAT_t96_t_t95_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t97_t_t92(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t97_t_t92_ROWPTR[i]; p < FOM_MAT_t97_t_t92_ROWPTR[i + 1]; ++p) s += FOM_MAT_t97_t_t92_VALS[p] * x[FOM_MAT_t97_t_t92_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t97_t_t95(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t97_t_t95_ROWPTR[i]; p < FOM_MAT_t97_t_t95_ROWPTR[i + 1]; ++p) s += FOM_MAT_t97_t_t95_VALS[p] * x[FOM_MAT_t97_t_t95_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t99_t_t95(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t99_t_t95_ROWPTR[i]; p < FOM_MAT_t99_t_t95_ROWPTR[i + 1]; ++p) s += FOM_MAT_t99_t_t95_VALS[p] * x[FOM_MAT_t99_t_t95_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t99_t_t98(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t99_t_t98_ROWPTR[i]; p < FOM_MAT_t99_t_t98_ROWPTR[i + 1]; ++p) s += FOM_MAT_t99_t_t98_VALS[p] * x[FOM_MAT_t99_t_t98_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t100_t_t95(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t100_t_t95_ROWPTR[i]; p < FOM_MAT_t100_t_t95_ROWPTR[i + 1]; ++p) s += FOM_MAT_t100_t_t95_VALS[p] * x[FOM_MAT_t100_t_t95_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t100_t_t98(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t100_t_t98_ROWPTR[i]; p < FOM_MAT_t100_t_t98_ROWPTR[i + 1]; ++p) s += FOM_MAT_t100_t_t98_VALS[p] * x[FOM_MAT_t100_t_t98_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t102_t_t98(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t102_t_t98_ROWPTR[i]; p < FOM_MAT_t102_t_t98_ROWPTR[i + 1]; ++p) s += FOM_MAT_t102_t_t98_VALS[p] * x[FOM_MAT_t102_t_t98_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t102_t_t101(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t102_t_t101_ROWPTR[i]; p < FOM_MAT_t102_t_t101_ROWPTR[i + 1]; ++p) s += FOM_MAT_t102_t_t101_VALS[p] * x[FOM_MAT_t102_t_t101_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t103_t_t98(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t103_t_t98_ROWPTR[i]; p < FOM_MAT_t103_t_t98_ROWPTR[i + 1]; ++p) s += FOM_MAT_t103_t_t98_VALS[p] * x[FOM_MAT_t103_t_t98_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t103_t_t101(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t103_t_t101_ROWPTR[i]; p < FOM_MAT_t103_t_t101_ROWPTR[i + 1]; ++p) s += FOM_MAT_t103_t_t101_VALS[p] * x[FOM_MAT_t103_t_t101_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t105_t_t101(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t105_t_t101_ROWPTR[i]; p < FOM_MAT_t105_t_t101_ROWPTR[i + 1]; ++p) s += FOM_MAT_t105_t_t101_VALS[p] * x[FOM_MAT_t105_t_t101_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t105_t_t104(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t105_t_t104_ROWPTR[i]; p < FOM_MAT_t105_t_t104_ROWPTR[i + 1]; ++p) s += FOM_MAT_t105_t_t104_VALS[p] * x[FOM_MAT_t105_t_t104_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t106_t_t101(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t106_t_t101_ROWPTR[i]; p < FOM_MAT_t106_t_t101_ROWPTR[i + 1]; ++p) s += FOM_MAT_t106_t_t101_VALS[p] * x[FOM_MAT_t106_t_t101_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t106_t_t104(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t106_t_t104_ROWPTR[i]; p < FOM_MAT_t106_t_t104_ROWPTR[i + 1]; ++p) s += FOM_MAT_t106_t_t104_VALS[p] * x[FOM_MAT_t106_t_t104_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t108_t_t104(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t108_t_t104_ROWPTR[i]; p < FOM_MAT_t108_t_t104_ROWPTR[i + 1]; ++p) s += FOM_MAT_t108_t_t104_VALS[p] * x[FOM_MAT_t108_t_t104_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t108_t_t107(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t108_t_t107_ROWPTR[i]; p < FOM_MAT_t108_t_t107_ROWPTR[i + 1]; ++p) s += FOM_MAT_t108_t_t107_VALS[p] * x[FOM_MAT_t108_t_t107_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t109_t_t104(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t109_t_t104_ROWPTR[i]; p < FOM_MAT_t109_t_t104_ROWPTR[i + 1]; ++p) s += FOM_MAT_t109_t_t104_VALS[p] * x[FOM_MAT_t109_t_t104_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t109_t_t107(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t109_t_t107_ROWPTR[i]; p < FOM_MAT_t109_t_t107_ROWPTR[i + 1]; ++p) s += FOM_MAT_t109_t_t107_VALS[p] * x[FOM_MAT_t109_t_t107_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t111_t_t107(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t111_t_t107_ROWPTR[i]; p < FOM_MAT_t111_t_t107_ROWPTR[i + 1]; ++p) s += FOM_MAT_t111_t_t107_VALS[p] * x[FOM_MAT_t111_t_t107_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t111_t_t110(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t111_t_t110_ROWPTR[i]; p < FOM_MAT_t111_t_t110_ROWPTR[i + 1]; ++p) s += FOM_MAT_t111_t_t110_VALS[p] * x[FOM_MAT_t111_t_t110_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t112_t_t107(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t112_t_t107_ROWPTR[i]; p < FOM_MAT_t112_t_t107_ROWPTR[i + 1]; ++p) s += FOM_MAT_t112_t_t107_VALS[p] * x[FOM_MAT_t112_t_t107_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t112_t_t110(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t112_t_t110_ROWPTR[i]; p < FOM_MAT_t112_t_t110_ROWPTR[i + 1]; ++p) s += FOM_MAT_t112_t_t110_VALS[p] * x[FOM_MAT_t112_t_t110_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t114_t_t110(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t114_t_t110_ROWPTR[i]; p < FOM_MAT_t114_t_t110_ROWPTR[i + 1]; ++p) s += FOM_MAT_t114_t_t110_VALS[p] * x[FOM_MAT_t114_t_t110_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t114_t_t113(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t114_t_t113_ROWPTR[i]; p < FOM_MAT_t114_t_t113_ROWPTR[i + 1]; ++p) s += FOM_MAT_t114_t_t113_VALS[p] * x[FOM_MAT_t114_t_t113_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t115_t_t110(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t115_t_t110_ROWPTR[i]; p < FOM_MAT_t115_t_t110_ROWPTR[i + 1]; ++p) s += FOM_MAT_t115_t_t110_VALS[p] * x[FOM_MAT_t115_t_t110_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t115_t_t113(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t115_t_t113_ROWPTR[i]; p < FOM_MAT_t115_t_t113_ROWPTR[i + 1]; ++p) s += FOM_MAT_t115_t_t113_VALS[p] * x[FOM_MAT_t115_t_t113_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t117_t_t113(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t117_t_t113_ROWPTR[i]; p < FOM_MAT_t117_t_t113_ROWPTR[i + 1]; ++p) s += FOM_MAT_t117_t_t113_VALS[p] * x[FOM_MAT_t117_t_t113_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t117_t_t116(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t117_t_t116_ROWPTR[i]; p < FOM_MAT_t117_t_t116_ROWPTR[i + 1]; ++p) s += FOM_MAT_t117_t_t116_VALS[p] * x[FOM_MAT_t117_t_t116_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t118_t_t113(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t118_t_t113_ROWPTR[i]; p < FOM_MAT_t118_t_t113_ROWPTR[i + 1]; ++p) s += FOM_MAT_t118_t_t113_VALS[p] * x[FOM_MAT_t118_t_t113_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t118_t_t116(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t118_t_t116_ROWPTR[i]; p < FOM_MAT_t118_t_t116_ROWPTR[i + 1]; ++p) s += FOM_MAT_t118_t_t116_VALS[p] * x[FOM_MAT_t118_t_t116_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t120_t_t116(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t120_t_t116_ROWPTR[i]; p < FOM_MAT_t120_t_t116_ROWPTR[i + 1]; ++p) s += FOM_MAT_t120_t_t116_VALS[p] * x[FOM_MAT_t120_t_t116_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t120_t_t119(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t120_t_t119_ROWPTR[i]; p < FOM_MAT_t120_t_t119_ROWPTR[i + 1]; ++p) s += FOM_MAT_t120_t_t119_VALS[p] * x[FOM_MAT_t120_t_t119_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t121_t_t116(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t121_t_t116_ROWPTR[i]; p < FOM_MAT_t121_t_t116_ROWPTR[i + 1]; ++p) s += FOM_MAT_t121_t_t116_VALS[p] * x[FOM_MAT_t121_t_t116_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t121_t_t119(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t121_t_t119_ROWPTR[i]; p < FOM_MAT_t121_t_t119_ROWPTR[i + 1]; ++p) s += FOM_MAT_t121_t_t119_VALS[p] * x[FOM_MAT_t121_t_t119_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t123_t_t119(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t123_t_t119_ROWPTR[i]; p < FOM_MAT_t123_t_t119_ROWPTR[i + 1]; ++p) s += FOM_MAT_t123_t_t119_VALS[p] * x[FOM_MAT_t123_t_t119_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t123_t_t122(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t123_t_t122_ROWPTR[i]; p < FOM_MAT_t123_t_t122_ROWPTR[i + 1]; ++p) s += FOM_MAT_t123_t_t122_VALS[p] * x[FOM_MAT_t123_t_t122_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t124_t_t119(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t124_t_t119_ROWPTR[i]; p < FOM_MAT_t124_t_t119_ROWPTR[i + 1]; ++p) s += FOM_MAT_t124_t_t119_VALS[p] * x[FOM_MAT_t124_t_t119_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t124_t_t122(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t124_t_t122_ROWPTR[i]; p < FOM_MAT_t124_t_t122_ROWPTR[i + 1]; ++p) s += FOM_MAT_t124_t_t122_VALS[p] * x[FOM_MAT_t124_t_t122_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t126_t_t122(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t126_t_t122_ROWPTR[i]; p < FOM_MAT_t126_t_t122_ROWPTR[i + 1]; ++p) s += FOM_MAT_t126_t_t122_VALS[p] * x[FOM_MAT_t126_t_t122_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t126_t_t125(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t126_t_t125_ROWPTR[i]; p < FOM_MAT_t126_t_t125_ROWPTR[i + 1]; ++p) s += FOM_MAT_t126_t_t125_VALS[p] * x[FOM_MAT_t126_t_t125_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t127_t_t122(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t127_t_t122_ROWPTR[i]; p < FOM_MAT_t127_t_t122_ROWPTR[i + 1]; ++p) s += FOM_MAT_t127_t_t122_VALS[p] * x[FOM_MAT_t127_t_t122_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t127_t_t125(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t127_t_t125_ROWPTR[i]; p < FOM_MAT_t127_t_t125_ROWPTR[i + 1]; ++p) s += FOM_MAT_t127_t_t125_VALS[p] * x[FOM_MAT_t127_t_t125_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t129_t_t125(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t129_t_t125_ROWPTR[i]; p < FOM_MAT_t129_t_t125_ROWPTR[i + 1]; ++p) s += FOM_MAT_t129_t_t125_VALS[p] * x[FOM_MAT_t129_t_t125_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t129_t_t128(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t129_t_t128_ROWPTR[i]; p < FOM_MAT_t129_t_t128_ROWPTR[i + 1]; ++p) s += FOM_MAT_t129_t_t128_VALS[p] * x[FOM_MAT_t129_t_t128_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t130_t_t125(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t130_t_t125_ROWPTR[i]; p < FOM_MAT_t130_t_t125_ROWPTR[i + 1]; ++p) s += FOM_MAT_t130_t_t125_VALS[p] * x[FOM_MAT_t130_t_t125_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t130_t_t128(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t130_t_t128_ROWPTR[i]; p < FOM_MAT_t130_t_t128_ROWPTR[i + 1]; ++p) s += FOM_MAT_t130_t_t128_VALS[p] * x[FOM_MAT_t130_t_t128_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t132_t_t128(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t132_t_t128_ROWPTR[i]; p < FOM_MAT_t132_t_t128_ROWPTR[i + 1]; ++p) s += FOM_MAT_t132_t_t128_VALS[p] * x[FOM_MAT_t132_t_t128_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t132_t_t131(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t132_t_t131_ROWPTR[i]; p < FOM_MAT_t132_t_t131_ROWPTR[i + 1]; ++p) s += FOM_MAT_t132_t_t131_VALS[p] * x[FOM_MAT_t132_t_t131_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t133_t_t128(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t133_t_t128_ROWPTR[i]; p < FOM_MAT_t133_t_t128_ROWPTR[i + 1]; ++p) s += FOM_MAT_t133_t_t128_VALS[p] * x[FOM_MAT_t133_t_t128_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t133_t_t131(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t133_t_t131_ROWPTR[i]; p < FOM_MAT_t133_t_t131_ROWPTR[i + 1]; ++p) s += FOM_MAT_t133_t_t131_VALS[p] * x[FOM_MAT_t133_t_t131_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t135_t_t131(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t135_t_t131_ROWPTR[i]; p < FOM_MAT_t135_t_t131_ROWPTR[i + 1]; ++p) s += FOM_MAT_t135_t_t131_VALS[p] * x[FOM_MAT_t135_t_t131_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t135_t_t134(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t135_t_t134_ROWPTR[i]; p < FOM_MAT_t135_t_t134_ROWPTR[i + 1]; ++p) s += FOM_MAT_t135_t_t134_VALS[p] * x[FOM_MAT_t135_t_t134_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t136_t_t131(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t136_t_t131_ROWPTR[i]; p < FOM_MAT_t136_t_t131_ROWPTR[i + 1]; ++p) s += FOM_MAT_t136_t_t131_VALS[p] * x[FOM_MAT_t136_t_t131_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t136_t_t134(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t136_t_t134_ROWPTR[i]; p < FOM_MAT_t136_t_t134_ROWPTR[i + 1]; ++p) s += FOM_MAT_t136_t_t134_VALS[p] * x[FOM_MAT_t136_t_t134_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t138_t_t134(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t138_t_t134_ROWPTR[i]; p < FOM_MAT_t138_t_t134_ROWPTR[i + 1]; ++p) s += FOM_MAT_t138_t_t134_VALS[p] * x[FOM_MAT_t138_t_t134_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t138_t_t137(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t138_t_t137_ROWPTR[i]; p < FOM_MAT_t138_t_t137_ROWPTR[i + 1]; ++p) s += FOM_MAT_t138_t_t137_VALS[p] * x[FOM_MAT_t138_t_t137_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t139_t_t134(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t139_t_t134_ROWPTR[i]; p < FOM_MAT_t139_t_t134_ROWPTR[i + 1]; ++p) s += FOM_MAT_t139_t_t134_VALS[p] * x[FOM_MAT_t139_t_t134_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t139_t_t137(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t139_t_t137_ROWPTR[i]; p < FOM_MAT_t139_t_t137_ROWPTR[i + 1]; ++p) s += FOM_MAT_t139_t_t137_VALS[p] * x[FOM_MAT_t139_t_t137_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t141_t_t137(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t141_t_t137_ROWPTR[i]; p < FOM_MAT_t141_t_t137_ROWPTR[i + 1]; ++p) s += FOM_MAT_t141_t_t137_VALS[p] * x[FOM_MAT_t141_t_t137_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t141_t_t140(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t141_t_t140_ROWPTR[i]; p < FOM_MAT_t141_t_t140_ROWPTR[i + 1]; ++p) s += FOM_MAT_t141_t_t140_VALS[p] * x[FOM_MAT_t141_t_t140_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t142_t_t137(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t142_t_t137_ROWPTR[i]; p < FOM_MAT_t142_t_t137_ROWPTR[i + 1]; ++p) s += FOM_MAT_t142_t_t137_VALS[p] * x[FOM_MAT_t142_t_t137_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t142_t_t140(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t142_t_t140_ROWPTR[i]; p < FOM_MAT_t142_t_t140_ROWPTR[i + 1]; ++p) s += FOM_MAT_t142_t_t140_VALS[p] * x[FOM_MAT_t142_t_t140_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t144_t_t140(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t144_t_t140_ROWPTR[i]; p < FOM_MAT_t144_t_t140_ROWPTR[i + 1]; ++p) s += FOM_MAT_t144_t_t140_VALS[p] * x[FOM_MAT_t144_t_t140_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t144_t_t143(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t144_t_t143_ROWPTR[i]; p < FOM_MAT_t144_t_t143_ROWPTR[i + 1]; ++p) s += FOM_MAT_t144_t_t143_VALS[p] * x[FOM_MAT_t144_t_t143_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t145_t_t140(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t145_t_t140_ROWPTR[i]; p < FOM_MAT_t145_t_t140_ROWPTR[i + 1]; ++p) s += FOM_MAT_t145_t_t140_VALS[p] * x[FOM_MAT_t145_t_t140_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t145_t_t143(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t145_t_t143_ROWPTR[i]; p < FOM_MAT_t145_t_t143_ROWPTR[i + 1]; ++p) s += FOM_MAT_t145_t_t143_VALS[p] * x[FOM_MAT_t145_t_t143_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t147_t_t143(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t147_t_t143_ROWPTR[i]; p < FOM_MAT_t147_t_t143_ROWPTR[i + 1]; ++p) s += FOM_MAT_t147_t_t143_VALS[p] * x[FOM_MAT_t147_t_t143_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t147_t_t146(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t147_t_t146_ROWPTR[i]; p < FOM_MAT_t147_t_t146_ROWPTR[i + 1]; ++p) s += FOM_MAT_t147_t_t146_VALS[p] * x[FOM_MAT_t147_t_t146_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t148_t_t143(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t148_t_t143_ROWPTR[i]; p < FOM_MAT_t148_t_t143_ROWPTR[i + 1]; ++p) s += FOM_MAT_t148_t_t143_VALS[p] * x[FOM_MAT_t148_t_t143_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_t148_t_t146(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_t148_t_t146_ROWPTR[i]; p < FOM_MAT_t148_t_t146_ROWPTR[i + 1]; ++p) s += FOM_MAT_t148_t_t146_VALS[p] * x[FOM_MAT_t148_t_t146_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_su_t_t146(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 650; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_su_t_t146_ROWPTR[i]; p < FOM_MAT_su_t_t146_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t146_VALS[p] * x[FOM_MAT_su_t_t146_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_su_t_t147(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 650; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_su_t_t147_ROWPTR[i]; p < FOM_MAT_su_t_t147_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t147_VALS[p] * x[FOM_MAT_su_t_t147_COLIDX[p]];
        out[i] += s;
    }
}

void fom_apply_su_t_t148(const fom_float *x, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 650; ++i) {
        fom_float s = 0.0;
        for (p = FOM_MAT_su_t_t148_ROWPTR[i]; p < FOM_MAT_su_t_t148_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t148_VALS[p] * x[FOM_MAT_su_t_t148_COLIDX[p]];
        out[i] += s;
    }
}

/* 20-row steps: both 20x600 members have row i on columns 30i..30i+29
   (COLIDX == p), and the 20x20 member is the identity selection. */
static void relu_step_20(const fom_float *va, const fom_float *a, const fom_float *b, const fom_float *vc, const fom_float *c, fom_float *out) {
    fom_int i, k;
    for (i = 0; i < 20; ++i) {
        fom_float s0 = b[i], s1 = 0.0;
        const fom_int o = 30 * i;
        for (k = 0; k < 30; ++k) { s0 += va[o + k] * a[o + k]; s1 += vc[o + k] * c[o + k]; }
        out[i] += s0 + s1;
    }
}

/* 30-row steps: both 30x600 members have row j on columns j + 30r, r = 0..19
   (entry 20j + r), and the 30x30 member is the identity selection. */
static void lin_step_30(const fom_float *va, const fom_float *a, const fom_float *b, const fom_float *vc, const fom_float *c, fom_float *out) {
    fom_int j, r;
    for (j = 0; j < 30; ++j) {
        fom_float s = b[j];
        const fom_float *pa = va + 20 * j, *pc = vc + 20 * j;
        for (r = 0; r < 20; ++r) s += pa[r] * a[30 * r + j] + pc[r] * c[30 * r + j];
        out[j] += s;
    }
}

/* Shared body of the 600-row steps: t5_t_t2 is the identity selection and
   t2_t_t1, t5_t_t4 have exactly one entry per row (ROWPTR[i] == i). */
static void clip_step_600(const fom_float *a, const fom_float *b, const fom_float *c, fom_float *out) {
    /* t2_t_t1 row 30i+k reads column i, t5_t_t4 row 30i+k reads column k. */
    fom_int i, k;
    for (i = 0; i < 20; ++i) {
        const fom_int o = 30 * i;
        const fom_float bi = b[i];
        for (k = 0; k < 30; ++k)
            out[o + k] += a[o + k] + FOM_MAT_t2_t_t1_VALS[o + k] * bi + FOM_MAT_t5_t_t4_VALS[o + k] * c[k];
    }
}

void fom_fused_t1(const fom_float *state, const fom_float *t_t0, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 20; ++i) {
        fom_float s;
        s = 0.0;
        for (p = FOM_MAT_t1_state_ROWPTR[i]; p < FOM_MAT_t1_state_ROWPTR[i + 1]; ++p) s += FOM_MAT_t1_state_VALS[p] * state[FOM_MAT_t1_state_COLIDX[p]];
        out[i] += s;
        s = 0.0;
        for (p = FOM_MAT_t1_t_t0_ROWPTR[i]; p < FOM_MAT_t1_t_t0_ROWPTR[i + 1]; ++p) s += FOM_MAT_t1_t_t0_VALS[p] * t_t0[FOM_MAT_t1_t_t0_COLIDX[p]];
        out[i] += s;
    }
}

void fom_fused_t2(const fom_float *state, const fom_float *t_t0, const fom_float *t_t1, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 600; ++i) {
        fom_float s;
        s = 0.0;
        for (p = FOM_MAT_t2_state_ROWPTR[i]; p < FOM_MAT_t2_state_ROWPTR[i + 1]; ++p) s += FOM_MAT_t2_state_VALS[p] * state[FOM_MAT_t2_state_COLIDX[p]];
        out[i] += s;
        s = 0.0;
        for (p = FOM_MAT_t2_t_t0_ROWPTR[i]; p < FOM_MAT_t2_t_t0_ROWPTR[i + 1]; ++p) s += FOM_MAT_t2_t_t0_VALS[p] * t_t0[FOM_MAT_t2_t_t0_COLIDX[p]];
        out[i] += s;
        s = 0.0;
        for (p = FOM_MAT_t2_t_t1_ROWPTR[i]; p < FOM_MAT_t2_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t2_t_t1_VALS[p] * t_t1[FOM_MAT_t2_t_t1_COLIDX[p]];
        out[i] += s;
    }
}

void fom_fused_t3(const fom_float *t_t0, const fom_float *t_t1, const fom_float *t_t2, fom_float *out) {
    relu_step_20(FOM_MAT_t3_t_t0_VALS, t_t0, t_t1, FOM_MAT_t3_t_t2_VALS, t_t2, out);
}

void fom_fused_t4(const fom_float *state, const fom_float *t_t0, const fom_float *t_t2, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 30; ++i) {
        fom_float s;
        s = 0.0;
        for (p = FOM_MAT_t4_state_ROWPTR[i]; p < FOM_MAT_t4_state_ROWPTR[i + 1]; ++p) s += FOM_MAT_t4_state_VALS[p] * state[FOM_MAT_t4_state_COLIDX[p]];
        out[i] += s;
        s = 0.0;
        for (p = FOM_MAT_t4_t_t0_ROWPTR[i]; p < FOM_MAT_t4_t_t0_ROWPTR[i + 1]; ++p) s += FOM_MAT_t4_t_t0_VALS[p] * t_t0[FOM_MAT_t4_t_t0_COLIDX[p]];
        out[i] += s;
        s = 0.0;
        for (p = FOM_MAT_t4_t_t2_ROWPTR[i]; p < FOM_MAT_t4_t_t2_ROWPTR[i + 1]; ++p) s += FOM_MAT_t4_t_t2_VALS[p] * t_t2[FOM_MAT_t4_t_t2_COLIDX[p]];
        out[i] += s;
    }
}

void fom_fused_t5(const fom_float *t_t2, const fom_float *t_t3, const fom_float *t_t4, fom_float *out) {
    clip_step_600(t_t2, t_t3, t_t4, out);
}

void fom_fused_t6(const fom_float *t_t2, const fom_float *t_t3, const fom_float *t_t5, fom_float *out) {
    relu_step_20(FOM_MAT_t6_t_t2_VALS, t_t2, t_t3, FOM_MAT_t6_t_t5_VALS, t_t5, out);
}

void fom_fused_t7(const fom_float *t_t2, const fom_float *t_t4, const fom_float *t_t5, fom_float *out) {
    lin_step_30(FOM_MAT_t7_t_t2_VALS, t_t2, t_t4, FOM_MAT_t7_t_t5_VALS, t_t5, out);
}

void fom_fused_t8(const fom_float *t_t5, const fom_float *t_t6, const fom_float *t_t7, fom_float *out) {
    clip_step_600(t_t5, t_t6, t_t7, out);
}

void fom_fused_t9(const fom_float *t_t5, const fom_float *t_t6, const fom_float *t_t8, fom_float *out) {
    relu_step_20(FOM_MAT_t9_t_t5_VALS, t_t5, t_t6, FOM_MAT_t9_t_t8_VALS, t_t8, out);
}

void fom_fused_t10(const fom_float *t_t5, const fom_float *t_t7, const fom_float *t_t8, fom_float *out) {
    lin_step_30(FOM_MAT_t10_t_t5_VALS, t_t5, t_t7, FOM_MAT_t10_t_t8_VALS, t_t8, out);
}

void fom_fused_t11(const fom_float *t_t8, const fom_float *t_t9, const fom_float *t_t10, fom_float *out) {
    clip_step_600(t_t8, t_t9, t_t10, out);
}

void fom_fused_t12(const fom_float *t_t8, const fom_float *t_t9, const fom_float *t_t11, fom_float *out) {
    relu_step_20(FOM_MAT_t12_t_t8_VALS, t_t8, t_t9, FOM_MAT_t12_t_t11_VALS, t_t11, out);
}

void fom_fused_t13(const fom_float *t_t8, const fom_float *t_t10, const fom_float *t_t11, fom_float *out) {
    lin_step_30(FOM_MAT_t13_t_t8_VALS, t_t8, t_t10, FOM_MAT_t13_t_t11_VALS, t_t11, out);
}

void fom_fused_t14(const fom_float *t_t11, const fom_float *t_t12, const fom_float *t_t13, fom_float *out) {
    clip_step_600(t_t11, t_t12, t_t13, out);
}

void fom_fused_t15(const fom_float *t_t11, const fom_float *t_t12, const fom_float *t_t14, fom_float *out) {
    relu_step_20(FOM_MAT_t15_t_t11_VALS, t_t11, t_t12, FOM_MAT_t15_t_t14_VALS, t_t14, out);
}

void fom_fused_t16(const fom_float *t_t11, const fom_float *t_t13, const fom_float *t_t14, fom_float *out) {
    lin_step_30(FOM_MAT_t16_t_t11_VALS, t_t11, t_t13, FOM_MAT_t16_t_t14_VALS, t_t14, out);
}

void fom_fused_t17(const fom_float *t_t14, const fom_float *t_t15, const fom_float *t_t16, fom_float *out) {
    clip_step_600(t_t14, t_t15, t_t16, out);
}

void fom_fused_t18(const fom_float *t_t14, const fom_float *t_t15, const fom_float *t_t17, fom_float *out) {
    relu_step_20(FOM_MAT_t18_t_t14_VALS, t_t14, t_t15, FOM_MAT_t18_t_t17_VALS, t_t17, out);
}

void fom_fused_t19(const fom_float *t_t14, const fom_float *t_t16, const fom_float *t_t17, fom_float *out) {
    lin_step_30(FOM_MAT_t19_t_t14_VALS, t_t14, t_t16, FOM_MAT_t19_t_t17_VALS, t_t17, out);
}

void fom_fused_t20(const fom_float *t_t17, const fom_float *t_t18, const fom_float *t_t19, fom_float *out) {
    clip_step_600(t_t17, t_t18, t_t19, out);
}

void fom_fused_t21(const fom_float *t_t17, const fom_float *t_t18, const fom_float *t_t20, fom_float *out) {
    relu_step_20(FOM_MAT_t21_t_t17_VALS, t_t17, t_t18, FOM_MAT_t21_t_t20_VALS, t_t20, out);
}

void fom_fused_t22(const fom_float *t_t17, const fom_float *t_t19, const fom_float *t_t20, fom_float *out) {
    lin_step_30(FOM_MAT_t22_t_t17_VALS, t_t17, t_t19, FOM_MAT_t22_t_t20_VALS, t_t20, out);
}

void fom_fused_t23(const fom_float *t_t20, const fom_float *t_t21, const fom_float *t_t22, fom_float *out) {
    clip_step_600(t_t20, t_t21, t_t22, out);
}

void fom_fused_t24(const fom_float *t_t20, const fom_float *t_t21, const fom_float *t_t23, fom_float *out) {
    relu_step_20(FOM_MAT_t24_t_t20_VALS, t_t20, t_t21, FOM_MAT_t24_t_t23_VALS, t_t23, out);
}

void fom_fused_t25(const fom_float *t_t20, const fom_float *t_t22, const fom_float *t_t23, fom_float *out) {
    lin_step_30(FOM_MAT_t25_t_t20_VALS, t_t20, t_t22, FOM_MAT_t25_t_t23_VALS, t_t23, out);
}

void fom_fused_t26(const fom_float *t_t23, const fom_float *t_t24, const fom_float *t_t25, fom_float *out) {
    clip_step_600(t_t23, t_t24, t_t25, out);
}

void fom_fused_t27(const fom_float *t_t23, const fom_float *t_t24, const fom_float *t_t26, fom_float *out) {
    relu_step_20(FOM_MAT_t27_t_t23_VALS, t_t23, t_t24, FOM_MAT_t27_t_t26_VALS, t_t26, out);
}

void fom_fused_t28(const fom_float *t_t23, const fom_float *t_t25, const fom_float *t_t26, fom_float *out) {
    lin_step_30(FOM_MAT_t28_t_t23_VALS, t_t23, t_t25, FOM_MAT_t28_t_t26_VALS, t_t26, out);
}

void fom_fused_t29(const fom_float *t_t26, const fom_float *t_t27, const fom_float *t_t28, fom_float *out) {
    clip_step_600(t_t26, t_t27, t_t28, out);
}

void fom_fused_t30(const fom_float *t_t26, const fom_float *t_t27, const fom_float *t_t29, fom_float *out) {
    relu_step_20(FOM_MAT_t30_t_t26_VALS, t_t26, t_t27, FOM_MAT_t30_t_t29_VALS, t_t29, out);
}

void fom_fused_t31(const fom_float *t_t26, const fom_float *t_t28, const fom_float *t_t29, fom_float *out) {
    lin_step_30(FOM_MAT_t31_t_t26_VALS, t_t26, t_t28, FOM_MAT_t31_t_t29_VALS, t_t29, out);
}

void fom_fused_t32(const fom_float *t_t29, const fom_float *t_t30, const fom_float *t_t31, fom_float *out) {
    clip_step_600(t_t29, t_t30, t_t31, out);
}

void fom_fused_t33(const fom_float *t_t29, const fom_float *t_t30, const fom_float *t_t32, fom_float *out) {
    relu_step_20(FOM_MAT_t33_t_t29_VALS, t_t29, t_t30, FOM_MAT_t33_t_t32_VALS, t_t32, out);
}

void fom_fused_t34(const fom_float *t_t29, const fom_float *t_t31, const fom_float *t_t32, fom_float *out) {
    lin_step_30(FOM_MAT_t34_t_t29_VALS, t_t29, t_t31, FOM_MAT_t34_t_t32_VALS, t_t32, out);
}

void fom_fused_t35(const fom_float *t_t32, const fom_float *t_t33, const fom_float *t_t34, fom_float *out) {
    clip_step_600(t_t32, t_t33, t_t34, out);
}

void fom_fused_t36(const fom_float *t_t32, const fom_float *t_t33, const fom_float *t_t35, fom_float *out) {
    relu_step_20(FOM_MAT_t36_t_t32_VALS, t_t32, t_t33, FOM_MAT_t36_t_t35_VALS, t_t35, out);
}

void fom_fused_t37(const fom_float *t_t32, const fom_float *t_t34, const fom_float *t_t35, fom_float *out) {
    lin_step_30(FOM_MAT_t37_t_t32_VALS, t_t32, t_t34, FOM_MAT_t37_t_t35_VALS, t_t35, out);
}

void fom_fused_t38(const fom_float *t_t35, const fom_float *t_t36, const fom_float *t_t37, fom_float *out) {
    clip_step_600(t_t35, t_t36, t_t37, out);
}

void fom_fused_t39(const fom_float *t_t35, const fom_float *t_t36, const fom_float *t_t38, fom_float *out) {
    relu_step_20(FOM_MAT_t39_t_t35_VALS, t_t35, t_t36, FOM_MAT_t39_t_t38_VALS, t_t38, out);
}

void fom_fused_t40(const fom_float *t_t35, const fom_float *t_t37, const fom_float *t_t38, fom_float *out) {
    lin_step_30(FOM_MAT_t40_t_t35_VALS, t_t35, t_t37, FOM_MAT_t40_t_t38_VALS, t_t38, out);
}

void fom_fused_t41(const fom_float *t_t38, const fom_float *t_t39, const fom_float *t_t40, fom_float *out) {
    clip_step_600(t_t38, t_t39, t_t40, out);
}

void fom_fused_t42(const fom_float *t_t38, const fom_float *t_t39, const fom_float *t_t41, fom_float *out) {
    relu_step_20(FOM_MAT_t42_t_t38_VALS, t_t38, t_t39, FOM_MAT_t42_t_t41_VALS, t_t41, out);
}

void fom_fused_t43(const fom_float *t_t38, const fom_float *t_t40, const fom_float *t_t41, fom_float *out) {
    lin_step_30(FOM_MAT_t43_t_t38_VALS, t_t38, t_t40, FOM_MAT_t43_t_t41_VALS, t_t41, out);
}

void fom_fused_t44(const fom_float *t_t41, const fom_float *t_t42, const fom_float *t_t43, fom_float *out) {
    clip_step_600(t_t41, t_t42, t_t43, out);
}

void fom_fused_t45(const fom_float *t_t41, const fom_float *t_t42, const fom_float *t_t44, fom_float *out) {
    relu_step_20(FOM_MAT_t45_t_t41_VALS, t_t41, t_t42, FOM_MAT_t45_t_t44_VALS, t_t44, out);
}

void fom_fused_t46(const fom_float *t_t41, const fom_float *t_t43, const fom_float *t_t44, fom_float *out) {
    lin_step_30(FOM_MAT_t46_t_t41_VALS, t_t41, t_t43, FOM_MAT_t46_t_t44_VALS, t_t44, out);
}

void fom_fused_t47(const fom_float *t_t44, const fom_float *t_t45, const fom_float *t_t46, fom_float *out) {
    clip_step_600(t_t44, t_t45, t_t46, out);
}

void fom_fused_t48(const fom_float *t_t44, const fom_float *t_t45, const fom_float *t_t47, fom_float *out) {
    relu_step_20(FOM_MAT_t48_t_t44_VALS, t_t44, t_t45, FOM_MAT_t48_t_t47_VALS, t_t47, out);
}

void fom_fused_t49(const fom_float *t_t44, const fom_float *t_t46, const fom_float *t_t47, fom_float *out) {
    lin_step_30(FOM_MAT_t49_t_t44_VALS, t_t44, t_t46, FOM_MAT_t49_t_t47_VALS, t_t47, out);
}

void fom_fused_t50(const fom_float *t_t47, const fom_float *t_t48, const fom_float *t_t49, fom_float *out) {
    clip_step_600(t_t47, t_t48, t_t49, out);
}

void fom_fused_t51(const fom_float *t_t47, const fom_float *t_t48, const fom_float *t_t50, fom_float *out) {
    relu_step_20(FOM_MAT_t51_t_t47_VALS, t_t47, t_t48, FOM_MAT_t51_t_t50_VALS, t_t50, out);
}

void fom_fused_t52(const fom_float *t_t47, const fom_float *t_t49, const fom_float *t_t50, fom_float *out) {
    lin_step_30(FOM_MAT_t52_t_t47_VALS, t_t47, t_t49, FOM_MAT_t52_t_t50_VALS, t_t50, out);
}

void fom_fused_t53(const fom_float *t_t50, const fom_float *t_t51, const fom_float *t_t52, fom_float *out) {
    clip_step_600(t_t50, t_t51, t_t52, out);
}

void fom_fused_t54(const fom_float *t_t50, const fom_float *t_t51, const fom_float *t_t53, fom_float *out) {
    relu_step_20(FOM_MAT_t54_t_t50_VALS, t_t50, t_t51, FOM_MAT_t54_t_t53_VALS, t_t53, out);
}

void fom_fused_t55(const fom_float *t_t50, const fom_float *t_t52, const fom_float *t_t53, fom_float *out) {
    lin_step_30(FOM_MAT_t55_t_t50_VALS, t_t50, t_t52, FOM_MAT_t55_t_t53_VALS, t_t53, out);
}

void fom_fused_t56(const fom_float *t_t53, const fom_float *t_t54, const fom_float *t_t55, fom_float *out) {
    clip_step_600(t_t53, t_t54, t_t55, out);
}

void fom_fused_t57(const fom_float *t_t53, const fom_float *t_t54, const fom_float *t_t56, fom_float *out) {
    relu_step_20(FOM_MAT_t57_t_t53_VALS, t_t53, t_t54, FOM_MAT_t57_t_t56_VALS, t_t56, out);
}

void fom_fused_t58(const fom_float *t_t53, const fom_float *t_t55, const fom_float *t_t56, fom_float *out) {
    lin_step_30(FOM_MAT_t58_t_t53_VALS, t_t53, t_t55, FOM_MAT_t58_t_t56_VALS, t_t56, out);
}

void fom_fused_t59(const fom_float *t_t56, const fom_float *t_t57, const fom_float *t_t58, fom_float *out) {
    clip_step_600(t_t56, t_t57, t_t58, out);
}

void fom_fused_t60(const fom_float *t_t56, const fom_float *t_t57, const fom_float *t_t59, fom_float *out) {
    relu_step_20(FOM_MAT_t60_t_t56_VALS, t_t56, t_t57, FOM_MAT_t60_t_t59_VALS, t_t59, out);
}

void fom_fused_t61(const fom_float *t_t56, const fom_float *t_t58, const fom_float *t_t59, fom_float *out) {
    lin_step_30(FOM_MAT_t61_t_t56_VALS, t_t56, t_t58, FOM_MAT_t61_t_t59_VALS, t_t59, out);
}

void fom_fused_t62(const fom_float *t_t59, const fom_float *t_t60, const fom_float *t_t61, fom_float *out) {
    clip_step_600(t_t59, t_t60, t_t61, out);
}

void fom_fused_t63(const fom_float *t_t59, const fom_float *t_t60, const fom_float *t_t62, fom_float *out) {
    relu_step_20(FOM_MAT_t63_t_t59_VALS, t_t59, t_t60, FOM_MAT_t63_t_t62_VALS, t_t62, out);
}

void fom_fused_t64(const fom_float *t_t59, const fom_float *t_t61, const fom_float *t_t62, fom_float *out) {
    lin_step_30(FOM_MAT_t64_t_t59_VALS, t_t59, t_t61, FOM_MAT_t64_t_t62_VALS, t_t62, out);
}

void fom_fused_t65(const fom_float *t_t62, const fom_float *t_t63, const fom_float *t_t64, fom_float *out) {
    clip_step_600(t_t62, t_t63, t_t64, out);
}

void fom_fused_t66(const fom_float *t_t62, const fom_float *t_t63, const fom_float *t_t65, fom_float *out) {
    relu_step_20(FOM_MAT_t66_t_t62_VALS, t_t62, t_t63, FOM_MAT_t66_t_t65_VALS, t_t65, out);
}

void fom_fused_t67(const fom_float *t_t62, const fom_float *t_t64, const fom_float *t_t65, fom_float *out) {
    lin_step_30(FOM_MAT_t67_t_t62_VALS, t_t62, t_t64, FOM_MAT_t67_t_t65_VALS, t_t65, out);
}

void fom_fused_t68(const fom_float *t_t65, const fom_float *t_t66, const fom_float *t_t67, fom_float *out) {
    clip_step_600(t_t65, t_t66, t_t67, out);
}

void fom_fused_t69(const fom_float *t_t65, const fom_float *t_t66, const fom_float *t_t68, fom_float *out) {
    relu_step_20(FOM_MAT_t69_t_t65_VALS, t_t65, t_t66, FOM_MAT_t69_t_t68_VALS, t_t68, out);
}

void fom_fused_t70(const fom_float *t_t65, const fom_float *t_t67, const fom_float *t_t68, fom_float *out) {
    lin_step_30(FOM_MAT_t70_t_t65_VALS, t_t65, t_t67, FOM_MAT_t70_t_t68_VALS, t_t68, out);
}

void fom_fused_t71(const fom_float *t_t68, const fom_float *t_t69, const fom_float *t_t70, fom_float *out) {
    clip_step_600(t_t68, t_t69, t_t70, out);
}

void fom_fused_t72(const fom_float *t_t68, const fom_float *t_t69, const fom_float *t_t71, fom_float *out) {
    relu_step_20(FOM_MAT_t72_t_t68_VALS, t_t68, t_t69, FOM_MAT_t72_t_t71_VALS, t_t71, out);
}

void fom_fused_t73(const fom_float *t_t68, const fom_float *t_t70, const fom_float *t_t71, fom_float *out) {
    lin_step_30(FOM_MAT_t73_t_t68_VALS, t_t68, t_t70, FOM_MAT_t73_t_t71_VALS, t_t71, out);
}

void fom_fused_t74(const fom_float *t_t71, const fom_float *t_t72, const fom_float *t_t73, fom_float *out) {
    clip_step_600(t_t71, t_t72, t_t73, out);
}

void fom_fused_t75(const fom_float *t_t71, const fom_float *t_t72, const fom_float *t_t74, fom_float *out) {
    relu_step_20(FOM_MAT_t75_t_t71_VALS, t_t71, t_t72, FOM_MAT_t75_t_t74_VALS, t_t74, out);
}

void fom_fused_t76(const fom_float *t_t71, const fom_float *t_t73, const fom_float *t_t74, fom_float *out) {
    lin_step_30(FOM_MAT_t76_t_t71_VALS, t_t71, t_t73, FOM_MAT_t76_t_t74_VALS, t_t74, out);
}

void fom_fused_t77(const fom_float *t_t74, const fom_float *t_t75, const fom_float *t_t76, fom_float *out) {
    clip_step_600(t_t74, t_t75, t_t76, out);
}

void fom_fused_t78(const fom_float *t_t74, const fom_float *t_t75, const fom_float *t_t77, fom_float *out) {
    relu_step_20(FOM_MAT_t78_t_t74_VALS, t_t74, t_t75, FOM_MAT_t78_t_t77_VALS, t_t77, out);
}

void fom_fused_t79(const fom_float *t_t74, const fom_float *t_t76, const fom_float *t_t77, fom_float *out) {
    lin_step_30(FOM_MAT_t79_t_t74_VALS, t_t74, t_t76, FOM_MAT_t79_t_t77_VALS, t_t77, out);
}

void fom_fused_t80(const fom_float *t_t77, const fom_float *t_t78, const fom_float *t_t79, fom_float *out) {
    clip_step_600(t_t77, t_t78, t_t79, out);
}

void fom_fused_t81(const fom_float *t_t77, const fom_float *t_t78, const fom_float *t_t80, fom_float *out) {
    relu_step_20(FOM_MAT_t81_t_t77_VALS, t_t77, t_t78, FOM_MAT_t81_t_t80_VALS, t_t80, out);
}

void fom_fused_t82(const fom_float *t_t77, const fom_float *t_t79, const fom_float *t_t80, fom_float *out) {
    lin_step_30(FOM_MAT_t82_t_t77_VALS, t_t77, t_t79, FOM_MAT_t82_t_t80_VALS, t_t80, out);
}

void fom_fused_t83(const fom_float *t_t80, const fom_float *t_t81, const fom_float *t_t82, fom_float *out) {
    clip_step_600(t_t80, t_t81, t_t82, out);
}

void fom_fused_t84(const fom_float *t_t80, const fom_float *t_t81, const fom_float *t_t83, fom_float *out) {
    relu_step_20(FOM_MAT_t84_t_t80_VALS, t_t80, t_t81, FOM_MAT_t84_t_t83_VALS, t_t83, out);
}

void fom_fused_t85(const fom_float *t_t80, const fom_float *t_t82, const fom_float *t_t83, fom_float *out) {
    lin_step_30(FOM_MAT_t85_t_t80_VALS, t_t80, t_t82, FOM_MAT_t85_t_t83_VALS, t_t83, out);
}

void fom_fused_t86(const fom_float *t_t83, const fom_float *t_t84, const fom_float *t_t85, fom_float *out) {
    clip_step_600(t_t83, t_t84, t_t85, out);
}

void fom_fused_t87(const fom_float *t_t83, const fom_float *t_t84, const fom_float *t_t86, fom_float *out) {
    relu_step_20(FOM_MAT_t87_t_t83_VALS, t_t83, t_t84, FOM_MAT_t87_t_t86_VALS, t_t86, out);
}

void fom_fused_t88(const fom_float *t_t83, const fom_float *t_t85, const fom_float *t_t86, fom_float *out) {
    lin_step_30(FOM_MAT_t88_t_t83_VALS, t_t83, t_t85, FOM_MAT_t88_t_t86_VALS, t_t86, out);
}

void fom_fused_t89(const fom_float *t_t86, const fom_float *t_t87, const fom_float *t_t88, fom_float *out) {
    clip_step_600(t_t86, t_t87, t_t88, out);
}

void fom_fused_t90(const fom_float *t_t86, const fom_float *t_t87, const fom_float *t_t89, fom_float *out) {
    relu_step_20(FOM_MAT_t90_t_t86_VALS, t_t86, t_t87, FOM_MAT_t90_t_t89_VALS, t_t89, out);
}

void fom_fused_t91(const fom_float *t_t86, const fom_float *t_t88, const fom_float *t_t89, fom_float *out) {
    lin_step_30(FOM_MAT_t91_t_t86_VALS, t_t86, t_t88, FOM_MAT_t91_t_t89_VALS, t_t89, out);
}

void fom_fused_t92(const fom_float *t_t89, const fom_float *t_t90, const fom_float *t_t91, fom_float *out) {
    clip_step_600(t_t89, t_t90, t_t91, out);
}

void fom_fused_t93(const fom_float *t_t89, const fom_float *t_t90, const fom_float *t_t92, fom_float *out) {
    relu_step_20(FOM_MAT_t93_t_t89_VALS, t_t89, t_t90, FOM_MAT_t93_t_t92_VALS, t_t92, out);
}

void fom_fused_t94(const fom_float *t_t89, const fom_float *t_t91, const fom_float *t_t92, fom_float *out) {
    lin_step_30(FOM_MAT_t94_t_t89_VALS, t_t89, t_t91, FOM_MAT_t94_t_t92_VALS, t_t92, out);
}

void fom_fused_t95(const fom_float *t_t92, const fom_float *t_t93, const fom_float *t_t94, fom_float *out) {
    clip_step_600(t_t92, t_t93, t_t94, out);
}

void fom_fused_t96(const fom_float *t_t92, const fom_float *t_t93, const fom_float *t_t95, fom_float *out) {
    relu_step_20(FOM_MAT_t96_t_t92_VALS, t_t92, t_t93, FOM_MAT_t96_t_t95_VALS, t_t95, out);
}

void fom_fused_t97(const fom_float *t_t92, const fom_float *t_t94, const fom_float *t_t95, fom_float *out) {
    lin_step_30(FOM_MAT_t97_t_t92_VALS, t_t92, t_t94, FOM_MAT_t97_t_t95_VALS, t_t95, out);
}

void fom_fused_t98(const fom_float *t_t95, const fom_float *t_t96, const fom_float *t_t97, fom_float *out) {
    clip_step_600(t_t95, t_t96, t_t97, out);
}

void fom_fused_t99(const fom_float *t_t95, const fom_float *t_t96, const fom_float *t_t98, fom_float *out) {
    relu_step_20(FOM_MAT_t99_t_t95_VALS, t_t95, t_t96, FOM_MAT_t99_t_t98_VALS, t_t98, out);
}

void fom_fused_t100(const fom_float *t_t95, const fom_float *t_t97, const fom_float *t_t98, fom_float *out) {
    lin_step_30(FOM_MAT_t100_t_t95_VALS, t_t95, t_t97, FOM_MAT_t100_t_t98_VALS, t_t98, out);
}

void fom_fused_t101(const fom_float *t_t98, const fom_float *t_t99, const fom_float *t_t100, fom_float *out) {
    clip_step_600(t_t98, t_t99, t_t100, out);
}

void fom_fused_t102(const fom_float *t_t98, const fom_float *t_t99, const fom_float *t_t101, fom_float *out) {
    relu_step_20(FOM_MAT_t102_t_t98_VALS, t_t98, t_t99, FOM_MAT_t102_t_t101_VALS, t_t101, out);
}

void fom_fused_t103(const fom_float *t_t98, const fom_float *t_t100, const fom_float *t_t101, fom_float *out) {
    lin_step_30(FOM_MAT_t103_t_t98_VALS, t_t98, t_t100, FOM_MAT_t103_t_t101_VALS, t_t101, out);
}

void fom_fused_t104(const fom_float *t_t101, const fom_float *t_t102, const fom_float *t_t103, fom_float *out) {
    clip_step_600(t_t101, t_t102, t_t103, out);
}

void fom_fused_t105(const fom_float *t_t101, const fom_float *t_t102, const fom_float *t_t104, fom_float *out) {
    relu_step_20(FOM_MAT_t105_t_t101_VALS, t_t101, t_t102, FOM_MAT_t105_t_t104_VALS, t_t104, out);
}

void fom_fused_t106(const fom_float *t_t101, const fom_float *t_t103, const fom_float *t_t104, fom_float *out) {
    lin_step_30(FOM_MAT_t106_t_t101_VALS, t_t101, t_t103, FOM_MAT_t106_t_t104_VALS, t_t104, out);
}

void fom_fused_t107(const fom_float *t_t104, const fom_float *t_t105, const fom_float *t_t106, fom_float *out) {
    clip_step_600(t_t104, t_t105, t_t106, out);
}

void fom_fused_t108(const fom_float *t_t104, const fom_float *t_t105, const fom_float *t_t107, fom_float *out) {
    relu_step_20(FOM_MAT_t108_t_t104_VALS, t_t104, t_t105, FOM_MAT_t108_t_t107_VALS, t_t107, out);
}

void fom_fused_t109(const fom_float *t_t104, const fom_float *t_t106, const fom_float *t_t107, fom_float *out) {
    lin_step_30(FOM_MAT_t109_t_t104_VALS, t_t104, t_t106, FOM_MAT_t109_t_t107_VALS, t_t107, out);
}

void fom_fused_t110(const fom_float *t_t107, const fom_float *t_t108, const fom_float *t_t109, fom_float *out) {
    clip_step_600(t_t107, t_t108, t_t109, out);
}

void fom_fused_t111(const fom_float *t_t107, const fom_float *t_t108, const fom_float *t_t110, fom_float *out) {
    relu_step_20(FOM_MAT_t111_t_t107_VALS, t_t107, t_t108, FOM_MAT_t111_t_t110_VALS, t_t110, out);
}

void fom_fused_t112(const fom_float *t_t107, const fom_float *t_t109, const fom_float *t_t110, fom_float *out) {
    lin_step_30(FOM_MAT_t112_t_t107_VALS, t_t107, t_t109, FOM_MAT_t112_t_t110_VALS, t_t110, out);
}

void fom_fused_t113(const fom_float *t_t110, const fom_float *t_t111, const fom_float *t_t112, fom_float *out) {
    clip_step_600(t_t110, t_t111, t_t112, out);
}

void fom_fused_t114(const fom_float *t_t110, const fom_float *t_t111, const fom_float *t_t113, fom_float *out) {
    relu_step_20(FOM_MAT_t114_t_t110_VALS, t_t110, t_t111, FOM_MAT_t114_t_t113_VALS, t_t113, out);
}

void fom_fused_t115(const fom_float *t_t110, const fom_float *t_t112, const fom_float *t_t113, fom_float *out) {
    lin_step_30(FOM_MAT_t115_t_t110_VALS, t_t110, t_t112, FOM_MAT_t115_t_t113_VALS, t_t113, out);
}

void fom_fused_t116(const fom_float *t_t113, const fom_float *t_t114, const fom_float *t_t115, fom_float *out) {
    clip_step_600(t_t113, t_t114, t_t115, out);
}

void fom_fused_t117(const fom_float *t_t113, const fom_float *t_t114, const fom_float *t_t116, fom_float *out) {
    relu_step_20(FOM_MAT_t117_t_t113_VALS, t_t113, t_t114, FOM_MAT_t117_t_t116_VALS, t_t116, out);
}

void fom_fused_t118(const fom_float *t_t113, const fom_float *t_t115, const fom_float *t_t116, fom_float *out) {
    lin_step_30(FOM_MAT_t118_t_t113_VALS, t_t113, t_t115, FOM_MAT_t118_t_t116_VALS, t_t116, out);
}

void fom_fused_t119(const fom_float *t_t116, const fom_float *t_t117, const fom_float *t_t118, fom_float *out) {
    clip_step_600(t_t116, t_t117, t_t118, out);
}

void fom_fused_t120(const fom_float *t_t116, const fom_float *t_t117, const fom_float *t_t119, fom_float *out) {
    relu_step_20(FOM_MAT_t120_t_t116_VALS, t_t116, t_t117, FOM_MAT_t120_t_t119_VALS, t_t119, out);
}

void fom_fused_t121(const fom_float *t_t116, const fom_float *t_t118, const fom_float *t_t119, fom_float *out) {
    lin_step_30(FOM_MAT_t121_t_t116_VALS, t_t116, t_t118, FOM_MAT_t121_t_t119_VALS, t_t119, out);
}

void fom_fused_t122(const fom_float *t_t119, const fom_float *t_t120, const fom_float *t_t121, fom_float *out) {
    clip_step_600(t_t119, t_t120, t_t121, out);
}

void fom_fused_t123(const fom_float *t_t119, const fom_float *t_t120, const fom_float *t_t122, fom_float *out) {
    relu_step_20(FOM_MAT_t123_t_t119_VALS, t_t119, t_t120, FOM_MAT_t123_t_t122_VALS, t_t122, out);
}

void fom_fused_t124(const fom_float *t_t119, const fom_float *t_t121, const fom_float *t_t122, fom_float *out) {
    lin_step_30(FOM_MAT_t124_t_t119_VALS, t_t119, t_t121, FOM_MAT_t124_t_t122_VALS, t_t122, out);
}

void fom_fused_t125(const fom_float *t_t122, const fom_float *t_t123, const fom_float *t_t124, fom_float *out) {
    clip_step_600(t_t122, t_t123, t_t124, out);
}

void fom_fused_t126(const fom_float *t_t122, const fom_float *t_t123, const fom_float *t_t125, fom_float *out) {
    relu_step_20(FOM_MAT_t126_t_t122_VALS, t_t122, t_t123, FOM_MAT_t126_t_t125_VALS, t_t125, out);
}

void fom_fused_t127(const fom_float *t_t122, const fom_float *t_t124, const fom_float *t_t125, fom_float *out) {
    lin_step_30(FOM_MAT_t127_t_t122_VALS, t_t122, t_t124, FOM_MAT_t127_t_t125_VALS, t_t125, out);
}

void fom_fused_t128(const fom_float *t_t125, const fom_float *t_t126, const fom_float *t_t127, fom_float *out) {
    clip_step_600(t_t125, t_t126, t_t127, out);
}

void fom_fused_t129(const fom_float *t_t125, const fom_float *t_t126, const fom_float *t_t128, fom_float *out) {
    relu_step_20(FOM_MAT_t129_t_t125_VALS, t_t125, t_t126, FOM_MAT_t129_t_t128_VALS, t_t128, out);
}

void fom_fused_t130(const fom_float *t_t125, const fom_float *t_t127, const fom_float *t_t128, fom_float *out) {
    lin_step_30(FOM_MAT_t130_t_t125_VALS, t_t125, t_t127, FOM_MAT_t130_t_t128_VALS, t_t128, out);
}

void fom_fused_t131(const fom_float *t_t128, const fom_float *t_t129, const fom_float *t_t130, fom_float *out) {
    clip_step_600(t_t128, t_t129, t_t130, out);
}

void fom_fused_t132(const fom_float *t_t128, const fom_float *t_t129, const fom_float *t_t131, fom_float *out) {
    relu_step_20(FOM_MAT_t132_t_t128_VALS, t_t128, t_t129, FOM_MAT_t132_t_t131_VALS, t_t131, out);
}

void fom_fused_t133(const fom_float *t_t128, const fom_float *t_t130, const fom_float *t_t131, fom_float *out) {
    lin_step_30(FOM_MAT_t133_t_t128_VALS, t_t128, t_t130, FOM_MAT_t133_t_t131_VALS, t_t131, out);
}

void fom_fused_t134(const fom_float *t_t131, const fom_float *t_t132, const fom_float *t_t133, fom_float *out) {
    clip_step_600(t_t131, t_t132, t_t133, out);
}

void fom_fused_t135(const fom_float *t_t131, const fom_float *t_t132, const fom_float *t_t134, fom_float *out) {
    relu_step_20(FOM_MAT_t135_t_t131_VALS, t_t131, t_t132, FOM_MAT_t135_t_t134_VALS, t_t134, out);
}

void fom_fused_t136(const fom_float *t_t131, const fom_float *t_t133, const fom_float *t_t134, fom_float *out) {
    lin_step_30(FOM_MAT_t136_t_t131_VALS, t_t131, t_t133, FOM_MAT_t136_t_t134_VALS, t_t134, out);
}

void fom_fused_t137(const fom_float *t_t134, const fom_float *t_t135, const fom_float *t_t136, fom_float *out) {
    clip_step_600(t_t134, t_t135, t_t136, out);
}

void fom_fused_t138(const fom_float *t_t134, const fom_float *t_t135, const fom_float *t_t137, fom_float *out) {
    relu_step_20(FOM_MAT_t138_t_t134_VALS, t_t134, t_t135, FOM_MAT_t138_t_t137_VALS, t_t137, out);
}

void fom_fused_t139(const fom_float *t_t134, const fom_float *t_t136, const fom_float *t_t137, fom_float *out) {
    lin_step_30(FOM_MAT_t139_t_t134_VALS, t_t134, t_t136, FOM_MAT_t139_t_t137_VALS, t_t137, out);
}

void fom_fused_t140(const fom_float *t_t137, const fom_float *t_t138, const fom_float *t_t139, fom_float *out) {
    clip_step_600(t_t137, t_t138, t_t139, out);
}

void fom_fused_t141(const fom_float *t_t137, const fom_float *t_t138, const fom_float *t_t140, fom_float *out) {
    relu_step_20(FOM_MAT_t141_t_t137_VALS, t_t137, t_t138, FOM_MAT_t141_t_t140_VALS, t_t140, out);
}

void fom_fused_t142(const fom_float *t_t137, const fom_float *t_t139, const fom_float *t_t140, fom_float *out) {
    lin_step_30(FOM_MAT_t142_t_t137_VALS, t_t137, t_t139, FOM_MAT_t142_t_t140_VALS, t_t140, out);
}

void fom_fused_t143(const fom_float *t_t140, const fom_float *t_t141, const fom_float *t_t142, fom_float *out) {
    clip_step_600(t_t140, t_t141, t_t142, out);
}

void fom_fused_t144(const fom_float *t_t140, const fom_float *t_t141, const fom_float *t_t143, fom_float *out) {
    relu_step_20(FOM_MAT_t144_t_t140_VALS, t_t140, t_t141, FOM_MAT_t144_t_t143_VALS, t_t143, out);
}

void fom_fused_t145(const fom_float *t_t140, const fom_float *t_t142, const fom_float *t_t143, fom_float *out) {
    lin_step_30(FOM_MAT_t145_t_t140_VALS, t_t140, t_t142, FOM_MAT_t145_t_t143_VALS, t_t143, out);
}

void fom_fused_t146(const fom_float *t_t143, const fom_float *t_t144, const fom_float *t_t145, fom_float *out) {
    clip_step_600(t_t143, t_t144, t_t145, out);
}

void fom_fused_t147(const fom_float *t_t143, const fom_float *t_t144, const fom_float *t_t146, fom_float *out) {
    relu_step_20(FOM_MAT_t147_t_t143_VALS, t_t143, t_t144, FOM_MAT_t147_t_t146_VALS, t_t146, out);
}

void fom_fused_t148(const fom_float *t_t143, const fom_float *t_t145, const fom_float *t_t146, fom_float *out) {
    lin_step_30(FOM_MAT_t148_t_t143_VALS, t_t143, t_t145, FOM_MAT_t148_t_t146_VALS, t_t146, out);
}

void fom_fused_su(const fom_float *t_t146, const fom_float *t_t147, const fom_float *t_t148, fom_float *out) {
    fom_int i, p;
    for (i = 0; i < 650; ++i) {
        fom_float s;
        s = 0.0;
        for (p = FOM_MAT_su_t_t146_ROWPTR[i]; p < FOM_MAT_su_t_t146_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t146_VALS[p] * t_t146[FOM_MAT_su_t_t146_COLIDX[p]];
        out[i] += s;
        s = 0.0;
        for (p = FOM_MAT_su_t_t147_ROWPTR[i]; p < FOM_MAT_su_t_t147_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t147_VALS[p] * t_t147[FOM_MAT_su_t_t147_COLIDX[p]];
        out[i] += s;
        s = 0.0;
        for (p = FOM_MAT_su_t_t148_ROWPTR[i]; p < FOM_MAT_su_t_t148_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t148_VALS[p] * t_t148[FOM_MAT_su_t_t148_COLIDX[p]];
        out[i] += s;
    }
}
