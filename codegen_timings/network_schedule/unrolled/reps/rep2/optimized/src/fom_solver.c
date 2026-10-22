#include "fom_solver.h"
#include "fom_data.h"
#include "fom_ops.h"
#include "fom_profile.h"

static void reset_state(FomWorkspace *work) {
    fom_int i;
    for (i = 0; i < FOM_STATE_DIM; ++i) work->state[i] = FOM_STATE_INIT[i];
}

void fom_set_default_settings(FomSettings *settings) {
    settings->max_iter = FOM_MAX_ITER;
    settings->tol = FOM_TOL;
    settings->check_termination = FOM_CHECK_TERMINATION;
}

void fom_setup(FomWorkspace *work, const FomSettings *settings) {
    fom_int i;
    if (settings) work->settings = *settings; else fom_set_default_settings(&work->settings);
    for (i = 0; i < FOM_PARAM_P0_DIM; ++i) work->params.p0[i] = 0.0;
    reset_state(work);
    work->solution.iter = 0;
    work->solution.residual = 0.0;
    work->solution.status = FOM_MAX_ITER_REACHED;
}

void fom_update_params(FomWorkspace *work, const FomParams *params) {
    work->params = *params;
    reset_state(work);
}

fom_int fom_solve(FomWorkspace *work) {
    fom_int i;
    fom_float *pp7 = work->acc;
    fom_float res = 0.0;
    for (i = 0; i < 30; ++i) pp7[i] = 0.0;
    fom_apply_t7_p_p0(work->params.p0, pp7);
    /* t0 */
    {
    FOM_PROF_BEGIN(steps_600);
    for (i = 0; i < 600; ++i) work->t0[i] = FOM_MAT_t0_c[i];
    fom_apply_t0_state(work->state, work->t0);
    for (i = 0; i < 600; ++i) { fom_float v = work->t0[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t0[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
    FOM_PROF_END(steps_600);
    }
    /* t1 */
    {
    FOM_PROF_BEGIN(steps_20_relu);
    for (i = 0; i < 20; ++i) work->t1[i] = FOM_MAT_t1_c[i];
    fom_fused_t1(work->state, work->t0, work->t1);
    for (i = 0; i < 20; ++i) if (work->t1[i] < 0.0) work->t1[i] = 0.0;
    FOM_PROF_END(steps_20_relu);
    }
    /* t2 */
    {
    FOM_PROF_BEGIN(steps_600);
    for (i = 0; i < 600; ++i) work->t2[i] = FOM_MAT_t0_c[i];
    fom_apply_t2_p_p0(work->params.p0, work->t2);
    fom_fused_t2(work->state, work->t0, work->t1, work->t2);
    for (i = 0; i < 600; ++i) { fom_float v = work->t2[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t2[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
    FOM_PROF_END(steps_600);
    }
    /* t3 */
    {
    FOM_PROF_BEGIN(steps_20_relu);
    for (i = 0; i < 20; ++i) work->t3[i] = FOM_MAT_t1_c[i];
    fom_fused_t3(work->t0, work->t1, work->t2, work->t3);
    for (i = 0; i < 20; ++i) if (work->t3[i] < 0.0) work->t3[i] = 0.0;
    FOM_PROF_END(steps_20_relu);
    }
    /* t4 */
    {
    FOM_PROF_BEGIN(steps_30_lin);
    for (i = 0; i < 30; ++i) work->t4[i] = 0.0;
    fom_apply_t4_p_p0(work->params.p0, work->t4);
    fom_fused_t4(work->state, work->t0, work->t2, work->t4);
    FOM_PROF_END(steps_30_lin);
    }
    /* t5 */
    {
    FOM_PROF_BEGIN(steps_600);
    for (i = 0; i < 600; ++i) work->t5[i] = FOM_MAT_t0_c[i];
    fom_fused_t5(work->t2, work->t3, work->t4, work->t5);
    for (i = 0; i < 600; ++i) { fom_float v = work->t5[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t5[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
    FOM_PROF_END(steps_600);
    }
    /* t6 */
    {
    FOM_PROF_BEGIN(steps_20_relu);
    for (i = 0; i < 20; ++i) work->t6[i] = FOM_MAT_t1_c[i];
    fom_fused_t6(work->t2, work->t3, work->t5, work->t6);
    for (i = 0; i < 20; ++i) if (work->t6[i] < 0.0) work->t6[i] = 0.0;
    FOM_PROF_END(steps_20_relu);
    }
    /* t7 */
    {
    FOM_PROF_BEGIN(steps_30_lin);
    for (i = 0; i < 30; ++i) work->t7[i] = pp7[i];
    fom_fused_t7(work->t2, work->t4, work->t5, work->t7);
    FOM_PROF_END(steps_30_lin);
    }
    /* t8 */
    {
    FOM_PROF_BEGIN(steps_600);
    for (i = 0; i < 600; ++i) work->t8[i] = FOM_MAT_t0_c[i];
    fom_fused_t8(work->t5, work->t6, work->t7, work->t8);
    for (i = 0; i < 600; ++i) { fom_float v = work->t8[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t8[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
    FOM_PROF_END(steps_600);
    }
    /* t9 */
    {
    FOM_PROF_BEGIN(steps_20_relu);
    for (i = 0; i < 20; ++i) work->t9[i] = FOM_MAT_t1_c[i];
    fom_fused_t9(work->t5, work->t6, work->t8, work->t9);
    for (i = 0; i < 20; ++i) if (work->t9[i] < 0.0) work->t9[i] = 0.0;
    FOM_PROF_END(steps_20_relu);
    }
    /* t10 */
    {
    FOM_PROF_BEGIN(steps_30_lin);
    for (i = 0; i < 30; ++i) work->t10[i] = pp7[i];
    fom_fused_t10(work->t5, work->t7, work->t8, work->t10);
    FOM_PROF_END(steps_30_lin);
    }
    /* t11 */
    {
    FOM_PROF_BEGIN(steps_600);
    for (i = 0; i < 600; ++i) work->t11[i] = FOM_MAT_t0_c[i];
    fom_fused_t11(work->t8, work->t9, work->t10, work->t11);
    for (i = 0; i < 600; ++i) { fom_float v = work->t11[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t11[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
    FOM_PROF_END(steps_600);
    }
    /* t12 */
    {
    FOM_PROF_BEGIN(steps_20_relu);
    for (i = 0; i < 20; ++i) work->t12[i] = FOM_MAT_t1_c[i];
    fom_fused_t12(work->t8, work->t9, work->t11, work->t12);
    for (i = 0; i < 20; ++i) if (work->t12[i] < 0.0) work->t12[i] = 0.0;
    FOM_PROF_END(steps_20_relu);
    }
    /* t13 */
    {
    FOM_PROF_BEGIN(steps_30_lin);
    for (i = 0; i < 30; ++i) work->t13[i] = pp7[i];
    fom_fused_t13(work->t8, work->t10, work->t11, work->t13);
    FOM_PROF_END(steps_30_lin);
    }
    /* t14 */
    {
    FOM_PROF_BEGIN(steps_600);
    for (i = 0; i < 600; ++i) work->t14[i] = FOM_MAT_t0_c[i];
    fom_fused_t14(work->t11, work->t12, work->t13, work->t14);
    for (i = 0; i < 600; ++i) { fom_float v = work->t14[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t14[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
    FOM_PROF_END(steps_600);
    }
    /* t15 */
    {
    FOM_PROF_BEGIN(steps_20_relu);
    for (i = 0; i < 20; ++i) work->t15[i] = FOM_MAT_t1_c[i];
    fom_fused_t15(work->t11, work->t12, work->t14, work->t15);
    for (i = 0; i < 20; ++i) if (work->t15[i] < 0.0) work->t15[i] = 0.0;
    FOM_PROF_END(steps_20_relu);
    }
    /* t16 */
    {
    FOM_PROF_BEGIN(steps_30_lin);
    for (i = 0; i < 30; ++i) work->t16[i] = pp7[i];
    fom_fused_t16(work->t11, work->t13, work->t14, work->t16);
    FOM_PROF_END(steps_30_lin);
    }
    /* t17 */
    {
    FOM_PROF_BEGIN(steps_600);
    for (i = 0; i < 600; ++i) work->t17[i] = FOM_MAT_t0_c[i];
    fom_fused_t17(work->t14, work->t15, work->t16, work->t17);
    for (i = 0; i < 600; ++i) { fom_float v = work->t17[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t17[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
    FOM_PROF_END(steps_600);
    }
    /* t18 */
    {
    FOM_PROF_BEGIN(steps_20_relu);
    for (i = 0; i < 20; ++i) work->t18[i] = FOM_MAT_t1_c[i];
    fom_fused_t18(work->t14, work->t15, work->t17, work->t18);
    for (i = 0; i < 20; ++i) if (work->t18[i] < 0.0) work->t18[i] = 0.0;
    FOM_PROF_END(steps_20_relu);
    }
    /* t19 */
    {
    FOM_PROF_BEGIN(steps_30_lin);
    for (i = 0; i < 30; ++i) work->t19[i] = pp7[i];
    fom_fused_t19(work->t14, work->t16, work->t17, work->t19);
    FOM_PROF_END(steps_30_lin);
    }
    /* t20 */
    {
    FOM_PROF_BEGIN(steps_600);
    for (i = 0; i < 600; ++i) work->t20[i] = FOM_MAT_t0_c[i];
    fom_fused_t20(work->t17, work->t18, work->t19, work->t20);
    for (i = 0; i < 600; ++i) { fom_float v = work->t20[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t20[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
    FOM_PROF_END(steps_600);
    }
    /* t21 */
    {
    FOM_PROF_BEGIN(steps_20_relu);
    for (i = 0; i < 20; ++i) work->t21[i] = FOM_MAT_t1_c[i];
    fom_fused_t21(work->t17, work->t18, work->t20, work->t21);
    for (i = 0; i < 20; ++i) if (work->t21[i] < 0.0) work->t21[i] = 0.0;
    FOM_PROF_END(steps_20_relu);
    }
    /* t22 */
    {
    FOM_PROF_BEGIN(steps_30_lin);
    for (i = 0; i < 30; ++i) work->t22[i] = pp7[i];
    fom_fused_t22(work->t17, work->t19, work->t20, work->t22);
    FOM_PROF_END(steps_30_lin);
    }
    /* t23 */
    {
    FOM_PROF_BEGIN(steps_600);
    for (i = 0; i < 600; ++i) work->t23[i] = FOM_MAT_t0_c[i];
    fom_fused_t23(work->t20, work->t21, work->t22, work->t23);
    for (i = 0; i < 600; ++i) { fom_float v = work->t23[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t23[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
    FOM_PROF_END(steps_600);
    }
    /* t24 */
    {
    FOM_PROF_BEGIN(steps_20_relu);
    for (i = 0; i < 20; ++i) work->t24[i] = FOM_MAT_t1_c[i];
    fom_fused_t24(work->t20, work->t21, work->t23, work->t24);
    for (i = 0; i < 20; ++i) if (work->t24[i] < 0.0) work->t24[i] = 0.0;
    FOM_PROF_END(steps_20_relu);
    }
    /* t25 */
    {
    FOM_PROF_BEGIN(steps_30_lin);
    for (i = 0; i < 30; ++i) work->t25[i] = pp7[i];
    fom_fused_t25(work->t20, work->t22, work->t23, work->t25);
    FOM_PROF_END(steps_30_lin);
    }
    /* t26 */
    {
    FOM_PROF_BEGIN(steps_600);
    for (i = 0; i < 600; ++i) work->t26[i] = FOM_MAT_t0_c[i];
    fom_fused_t26(work->t23, work->t24, work->t25, work->t26);
    for (i = 0; i < 600; ++i) { fom_float v = work->t26[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t26[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
    FOM_PROF_END(steps_600);
    }
    /* t27 */
    {
    FOM_PROF_BEGIN(steps_20_relu);
    for (i = 0; i < 20; ++i) work->t27[i] = FOM_MAT_t1_c[i];
    fom_fused_t27(work->t23, work->t24, work->t26, work->t27);
    for (i = 0; i < 20; ++i) if (work->t27[i] < 0.0) work->t27[i] = 0.0;
    FOM_PROF_END(steps_20_relu);
    }
    /* t28 */
    {
    FOM_PROF_BEGIN(steps_30_lin);
    for (i = 0; i < 30; ++i) work->t28[i] = pp7[i];
    fom_fused_t28(work->t23, work->t25, work->t26, work->t28);
    FOM_PROF_END(steps_30_lin);
    }
    /* t29 */
    {
    FOM_PROF_BEGIN(steps_600);
    for (i = 0; i < 600; ++i) work->t29[i] = FOM_MAT_t0_c[i];
    fom_fused_t29(work->t26, work->t27, work->t28, work->t29);
    for (i = 0; i < 600; ++i) { fom_float v = work->t29[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t29[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
    FOM_PROF_END(steps_600);
    }
    /* t30 */
    {
    FOM_PROF_BEGIN(steps_20_relu);
    for (i = 0; i < 20; ++i) work->t30[i] = FOM_MAT_t1_c[i];
    fom_fused_t30(work->t26, work->t27, work->t29, work->t30);
    for (i = 0; i < 20; ++i) if (work->t30[i] < 0.0) work->t30[i] = 0.0;
    FOM_PROF_END(steps_20_relu);
    }
    /* t31 */
    {
    FOM_PROF_BEGIN(steps_30_lin);
    for (i = 0; i < 30; ++i) work->t31[i] = pp7[i];
    fom_fused_t31(work->t26, work->t28, work->t29, work->t31);
    FOM_PROF_END(steps_30_lin);
    }
    /* t32 */
    {
    FOM_PROF_BEGIN(steps_600);
    for (i = 0; i < 600; ++i) work->t32[i] = FOM_MAT_t0_c[i];
    fom_fused_t32(work->t29, work->t30, work->t31, work->t32);
    for (i = 0; i < 600; ++i) { fom_float v = work->t32[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t32[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
    FOM_PROF_END(steps_600);
    }
    /* t33 */
    {
    FOM_PROF_BEGIN(steps_20_relu);
    for (i = 0; i < 20; ++i) work->t33[i] = FOM_MAT_t1_c[i];
    fom_fused_t33(work->t29, work->t30, work->t32, work->t33);
    for (i = 0; i < 20; ++i) if (work->t33[i] < 0.0) work->t33[i] = 0.0;
    FOM_PROF_END(steps_20_relu);
    }
    /* t34 */
    {
    FOM_PROF_BEGIN(steps_30_lin);
    for (i = 0; i < 30; ++i) work->t34[i] = pp7[i];
    fom_fused_t34(work->t29, work->t31, work->t32, work->t34);
    FOM_PROF_END(steps_30_lin);
    }
    /* t35 */
    {
    FOM_PROF_BEGIN(steps_600);
    for (i = 0; i < 600; ++i) work->t35[i] = FOM_MAT_t0_c[i];
    fom_fused_t35(work->t32, work->t33, work->t34, work->t35);
    for (i = 0; i < 600; ++i) { fom_float v = work->t35[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t35[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
    FOM_PROF_END(steps_600);
    }
    /* t36 */
    {
    FOM_PROF_BEGIN(steps_20_relu);
    for (i = 0; i < 20; ++i) work->t36[i] = FOM_MAT_t1_c[i];
    fom_fused_t36(work->t32, work->t33, work->t35, work->t36);
    for (i = 0; i < 20; ++i) if (work->t36[i] < 0.0) work->t36[i] = 0.0;
    FOM_PROF_END(steps_20_relu);
    }
    /* t37 */
    {
    FOM_PROF_BEGIN(steps_30_lin);
    for (i = 0; i < 30; ++i) work->t37[i] = pp7[i];
    fom_fused_t37(work->t32, work->t34, work->t35, work->t37);
    FOM_PROF_END(steps_30_lin);
    }
    /* t38 */
    {
    FOM_PROF_BEGIN(steps_600);
    for (i = 0; i < 600; ++i) work->t38[i] = FOM_MAT_t0_c[i];
    fom_fused_t38(work->t35, work->t36, work->t37, work->t38);
    for (i = 0; i < 600; ++i) { fom_float v = work->t38[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t38[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
    FOM_PROF_END(steps_600);
    }
    /* t39 */
    {
    FOM_PROF_BEGIN(steps_20_relu);
    for (i = 0; i < 20; ++i) work->t39[i] = FOM_MAT_t1_c[i];
    fom_fused_t39(work->t35, work->t36, work->t38, work->t39);
    for (i = 0; i < 20; ++i) if (work->t39[i] < 0.0) work->t39[i] = 0.0;
    FOM_PROF_END(steps_20_relu);
    }
    /* t40 */
    {
    FOM_PROF_BEGIN(steps_30_lin);
    for (i = 0; i < 30; ++i) work->t40[i] = pp7[i];
    fom_fused_t40(work->t35, work->t37, work->t38, work->t40);
    FOM_PROF_END(steps_30_lin);
    }
    /* t41 */
    {
    FOM_PROF_BEGIN(steps_600);
    for (i = 0; i < 600; ++i) work->t41[i] = FOM_MAT_t0_c[i];
    fom_fused_t41(work->t38, work->t39, work->t40, work->t41);
    for (i = 0; i < 600; ++i) { fom_float v = work->t41[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t41[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
    FOM_PROF_END(steps_600);
    }
    /* t42 */
    {
    FOM_PROF_BEGIN(steps_20_relu);
    for (i = 0; i < 20; ++i) work->t42[i] = FOM_MAT_t1_c[i];
    fom_fused_t42(work->t38, work->t39, work->t41, work->t42);
    for (i = 0; i < 20; ++i) if (work->t42[i] < 0.0) work->t42[i] = 0.0;
    FOM_PROF_END(steps_20_relu);
    }
    /* t43 */
    {
    FOM_PROF_BEGIN(steps_30_lin);
    for (i = 0; i < 30; ++i) work->t43[i] = pp7[i];
    fom_fused_t43(work->t38, work->t40, work->t41, work->t43);
    FOM_PROF_END(steps_30_lin);
    }
    /* t44 */
    {
    FOM_PROF_BEGIN(steps_600);
    for (i = 0; i < 600; ++i) work->t44[i] = FOM_MAT_t0_c[i];
    fom_fused_t44(work->t41, work->t42, work->t43, work->t44);
    for (i = 0; i < 600; ++i) { fom_float v = work->t44[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t44[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
    FOM_PROF_END(steps_600);
    }
    /* t45 */
    {
    FOM_PROF_BEGIN(steps_20_relu);
    for (i = 0; i < 20; ++i) work->t45[i] = FOM_MAT_t1_c[i];
    fom_fused_t45(work->t41, work->t42, work->t44, work->t45);
    for (i = 0; i < 20; ++i) if (work->t45[i] < 0.0) work->t45[i] = 0.0;
    FOM_PROF_END(steps_20_relu);
    }
    /* t46 */
    {
    FOM_PROF_BEGIN(steps_30_lin);
    for (i = 0; i < 30; ++i) work->t46[i] = pp7[i];
    fom_fused_t46(work->t41, work->t43, work->t44, work->t46);
    FOM_PROF_END(steps_30_lin);
    }
    /* t47 */
    {
    FOM_PROF_BEGIN(steps_600);
    for (i = 0; i < 600; ++i) work->t47[i] = FOM_MAT_t0_c[i];
    fom_fused_t47(work->t44, work->t45, work->t46, work->t47);
    for (i = 0; i < 600; ++i) { fom_float v = work->t47[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t47[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
    FOM_PROF_END(steps_600);
    }
    /* t48 */
    {
    FOM_PROF_BEGIN(steps_20_relu);
    for (i = 0; i < 20; ++i) work->t48[i] = FOM_MAT_t1_c[i];
    fom_fused_t48(work->t44, work->t45, work->t47, work->t48);
    for (i = 0; i < 20; ++i) if (work->t48[i] < 0.0) work->t48[i] = 0.0;
    FOM_PROF_END(steps_20_relu);
    }
    /* t49 */
    {
    FOM_PROF_BEGIN(steps_30_lin);
    for (i = 0; i < 30; ++i) work->t49[i] = pp7[i];
    fom_fused_t49(work->t44, work->t46, work->t47, work->t49);
    FOM_PROF_END(steps_30_lin);
    }
    /* t50 */
    {
    FOM_PROF_BEGIN(steps_600);
    for (i = 0; i < 600; ++i) work->t50[i] = FOM_MAT_t0_c[i];
    fom_fused_t50(work->t47, work->t48, work->t49, work->t50);
    for (i = 0; i < 600; ++i) { fom_float v = work->t50[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t50[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
    FOM_PROF_END(steps_600);
    }
    /* t51 */
    {
    FOM_PROF_BEGIN(steps_20_relu);
    for (i = 0; i < 20; ++i) work->t51[i] = FOM_MAT_t1_c[i];
    fom_fused_t51(work->t47, work->t48, work->t50, work->t51);
    for (i = 0; i < 20; ++i) if (work->t51[i] < 0.0) work->t51[i] = 0.0;
    FOM_PROF_END(steps_20_relu);
    }
    /* t52 */
    {
    FOM_PROF_BEGIN(steps_30_lin);
    for (i = 0; i < 30; ++i) work->t52[i] = pp7[i];
    fom_fused_t52(work->t47, work->t49, work->t50, work->t52);
    FOM_PROF_END(steps_30_lin);
    }
    /* t53 */
    {
    FOM_PROF_BEGIN(steps_600);
    for (i = 0; i < 600; ++i) work->t53[i] = FOM_MAT_t0_c[i];
    fom_fused_t53(work->t50, work->t51, work->t52, work->t53);
    for (i = 0; i < 600; ++i) { fom_float v = work->t53[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t53[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
    FOM_PROF_END(steps_600);
    }
    /* t54 */
    {
    FOM_PROF_BEGIN(steps_20_relu);
    for (i = 0; i < 20; ++i) work->t54[i] = FOM_MAT_t1_c[i];
    fom_fused_t54(work->t50, work->t51, work->t53, work->t54);
    for (i = 0; i < 20; ++i) if (work->t54[i] < 0.0) work->t54[i] = 0.0;
    FOM_PROF_END(steps_20_relu);
    }
    /* t55 */
    {
    FOM_PROF_BEGIN(steps_30_lin);
    for (i = 0; i < 30; ++i) work->t55[i] = pp7[i];
    fom_fused_t55(work->t50, work->t52, work->t53, work->t55);
    FOM_PROF_END(steps_30_lin);
    }
    /* t56 */
    {
    FOM_PROF_BEGIN(steps_600);
    for (i = 0; i < 600; ++i) work->t56[i] = FOM_MAT_t0_c[i];
    fom_fused_t56(work->t53, work->t54, work->t55, work->t56);
    for (i = 0; i < 600; ++i) { fom_float v = work->t56[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t56[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
    FOM_PROF_END(steps_600);
    }
    /* t57 */
    {
    FOM_PROF_BEGIN(steps_20_relu);
    for (i = 0; i < 20; ++i) work->t57[i] = FOM_MAT_t1_c[i];
    fom_fused_t57(work->t53, work->t54, work->t56, work->t57);
    for (i = 0; i < 20; ++i) if (work->t57[i] < 0.0) work->t57[i] = 0.0;
    FOM_PROF_END(steps_20_relu);
    }
    /* t58 */
    {
    FOM_PROF_BEGIN(steps_30_lin);
    for (i = 0; i < 30; ++i) work->t58[i] = pp7[i];
    fom_fused_t58(work->t53, work->t55, work->t56, work->t58);
    FOM_PROF_END(steps_30_lin);
    }
    /* t59 */
    {
    FOM_PROF_BEGIN(steps_600);
    for (i = 0; i < 600; ++i) work->t59[i] = FOM_MAT_t0_c[i];
    fom_fused_t59(work->t56, work->t57, work->t58, work->t59);
    for (i = 0; i < 600; ++i) { fom_float v = work->t59[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t59[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
    FOM_PROF_END(steps_600);
    }
    /* t60 */
    {
    FOM_PROF_BEGIN(steps_20_relu);
    for (i = 0; i < 20; ++i) work->t60[i] = FOM_MAT_t1_c[i];
    fom_fused_t60(work->t56, work->t57, work->t59, work->t60);
    for (i = 0; i < 20; ++i) if (work->t60[i] < 0.0) work->t60[i] = 0.0;
    FOM_PROF_END(steps_20_relu);
    }
    /* t61 */
    {
    FOM_PROF_BEGIN(steps_30_lin);
    for (i = 0; i < 30; ++i) work->t61[i] = pp7[i];
    fom_fused_t61(work->t56, work->t58, work->t59, work->t61);
    FOM_PROF_END(steps_30_lin);
    }
    /* t62 */
    {
    FOM_PROF_BEGIN(steps_600);
    for (i = 0; i < 600; ++i) work->t62[i] = FOM_MAT_t0_c[i];
    fom_fused_t62(work->t59, work->t60, work->t61, work->t62);
    for (i = 0; i < 600; ++i) { fom_float v = work->t62[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t62[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
    FOM_PROF_END(steps_600);
    }
    /* t63 */
    {
    FOM_PROF_BEGIN(steps_20_relu);
    for (i = 0; i < 20; ++i) work->t63[i] = FOM_MAT_t1_c[i];
    fom_fused_t63(work->t59, work->t60, work->t62, work->t63);
    for (i = 0; i < 20; ++i) if (work->t63[i] < 0.0) work->t63[i] = 0.0;
    FOM_PROF_END(steps_20_relu);
    }
    /* t64 */
    {
    FOM_PROF_BEGIN(steps_30_lin);
    for (i = 0; i < 30; ++i) work->t64[i] = pp7[i];
    fom_fused_t64(work->t59, work->t61, work->t62, work->t64);
    FOM_PROF_END(steps_30_lin);
    }
    /* t65 */
    {
    FOM_PROF_BEGIN(steps_600);
    for (i = 0; i < 600; ++i) work->t65[i] = FOM_MAT_t0_c[i];
    fom_fused_t65(work->t62, work->t63, work->t64, work->t65);
    for (i = 0; i < 600; ++i) { fom_float v = work->t65[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t65[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
    FOM_PROF_END(steps_600);
    }
    /* t66 */
    {
    FOM_PROF_BEGIN(steps_20_relu);
    for (i = 0; i < 20; ++i) work->t66[i] = FOM_MAT_t1_c[i];
    fom_fused_t66(work->t62, work->t63, work->t65, work->t66);
    for (i = 0; i < 20; ++i) if (work->t66[i] < 0.0) work->t66[i] = 0.0;
    FOM_PROF_END(steps_20_relu);
    }
    /* t67 */
    {
    FOM_PROF_BEGIN(steps_30_lin);
    for (i = 0; i < 30; ++i) work->t67[i] = pp7[i];
    fom_fused_t67(work->t62, work->t64, work->t65, work->t67);
    FOM_PROF_END(steps_30_lin);
    }
    /* t68 */
    {
    FOM_PROF_BEGIN(steps_600);
    for (i = 0; i < 600; ++i) work->t68[i] = FOM_MAT_t0_c[i];
    fom_fused_t68(work->t65, work->t66, work->t67, work->t68);
    for (i = 0; i < 600; ++i) { fom_float v = work->t68[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t68[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
    FOM_PROF_END(steps_600);
    }
    /* t69 */
    {
    FOM_PROF_BEGIN(steps_20_relu);
    for (i = 0; i < 20; ++i) work->t69[i] = FOM_MAT_t1_c[i];
    fom_fused_t69(work->t65, work->t66, work->t68, work->t69);
    for (i = 0; i < 20; ++i) if (work->t69[i] < 0.0) work->t69[i] = 0.0;
    FOM_PROF_END(steps_20_relu);
    }
    /* t70 */
    {
    FOM_PROF_BEGIN(steps_30_lin);
    for (i = 0; i < 30; ++i) work->t70[i] = pp7[i];
    fom_fused_t70(work->t65, work->t67, work->t68, work->t70);
    FOM_PROF_END(steps_30_lin);
    }
    /* t71 */
    {
    FOM_PROF_BEGIN(steps_600);
    for (i = 0; i < 600; ++i) work->t71[i] = FOM_MAT_t0_c[i];
    fom_fused_t71(work->t68, work->t69, work->t70, work->t71);
    for (i = 0; i < 600; ++i) { fom_float v = work->t71[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t71[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
    FOM_PROF_END(steps_600);
    }
    /* t72 */
    {
    FOM_PROF_BEGIN(steps_20_relu);
    for (i = 0; i < 20; ++i) work->t72[i] = FOM_MAT_t1_c[i];
    fom_fused_t72(work->t68, work->t69, work->t71, work->t72);
    for (i = 0; i < 20; ++i) if (work->t72[i] < 0.0) work->t72[i] = 0.0;
    FOM_PROF_END(steps_20_relu);
    }
    /* t73 */
    {
    FOM_PROF_BEGIN(steps_30_lin);
    for (i = 0; i < 30; ++i) work->t73[i] = pp7[i];
    fom_fused_t73(work->t68, work->t70, work->t71, work->t73);
    FOM_PROF_END(steps_30_lin);
    }
    /* t74 */
    {
    FOM_PROF_BEGIN(steps_600);
    for (i = 0; i < 600; ++i) work->t74[i] = FOM_MAT_t0_c[i];
    fom_fused_t74(work->t71, work->t72, work->t73, work->t74);
    for (i = 0; i < 600; ++i) { fom_float v = work->t74[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t74[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
    FOM_PROF_END(steps_600);
    }
    /* t75 */
    {
    FOM_PROF_BEGIN(steps_20_relu);
    for (i = 0; i < 20; ++i) work->t75[i] = FOM_MAT_t1_c[i];
    fom_fused_t75(work->t71, work->t72, work->t74, work->t75);
    for (i = 0; i < 20; ++i) if (work->t75[i] < 0.0) work->t75[i] = 0.0;
    FOM_PROF_END(steps_20_relu);
    }
    /* t76 */
    {
    FOM_PROF_BEGIN(steps_30_lin);
    for (i = 0; i < 30; ++i) work->t76[i] = pp7[i];
    fom_fused_t76(work->t71, work->t73, work->t74, work->t76);
    FOM_PROF_END(steps_30_lin);
    }
    /* t77 */
    {
    FOM_PROF_BEGIN(steps_600);
    for (i = 0; i < 600; ++i) work->t77[i] = FOM_MAT_t0_c[i];
    fom_fused_t77(work->t74, work->t75, work->t76, work->t77);
    for (i = 0; i < 600; ++i) { fom_float v = work->t77[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t77[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
    FOM_PROF_END(steps_600);
    }
    /* t78 */
    {
    FOM_PROF_BEGIN(steps_20_relu);
    for (i = 0; i < 20; ++i) work->t78[i] = FOM_MAT_t1_c[i];
    fom_fused_t78(work->t74, work->t75, work->t77, work->t78);
    for (i = 0; i < 20; ++i) if (work->t78[i] < 0.0) work->t78[i] = 0.0;
    FOM_PROF_END(steps_20_relu);
    }
    /* t79 */
    {
    FOM_PROF_BEGIN(steps_30_lin);
    for (i = 0; i < 30; ++i) work->t79[i] = pp7[i];
    fom_fused_t79(work->t74, work->t76, work->t77, work->t79);
    FOM_PROF_END(steps_30_lin);
    }
    /* t80 */
    {
    FOM_PROF_BEGIN(steps_600);
    for (i = 0; i < 600; ++i) work->t80[i] = FOM_MAT_t0_c[i];
    fom_fused_t80(work->t77, work->t78, work->t79, work->t80);
    for (i = 0; i < 600; ++i) { fom_float v = work->t80[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t80[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
    FOM_PROF_END(steps_600);
    }
    /* t81 */
    {
    FOM_PROF_BEGIN(steps_20_relu);
    for (i = 0; i < 20; ++i) work->t81[i] = FOM_MAT_t1_c[i];
    fom_fused_t81(work->t77, work->t78, work->t80, work->t81);
    for (i = 0; i < 20; ++i) if (work->t81[i] < 0.0) work->t81[i] = 0.0;
    FOM_PROF_END(steps_20_relu);
    }
    /* t82 */
    {
    FOM_PROF_BEGIN(steps_30_lin);
    for (i = 0; i < 30; ++i) work->t82[i] = pp7[i];
    fom_fused_t82(work->t77, work->t79, work->t80, work->t82);
    FOM_PROF_END(steps_30_lin);
    }
    /* t83 */
    {
    FOM_PROF_BEGIN(steps_600);
    for (i = 0; i < 600; ++i) work->t83[i] = FOM_MAT_t0_c[i];
    fom_fused_t83(work->t80, work->t81, work->t82, work->t83);
    for (i = 0; i < 600; ++i) { fom_float v = work->t83[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t83[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
    FOM_PROF_END(steps_600);
    }
    /* t84 */
    {
    FOM_PROF_BEGIN(steps_20_relu);
    for (i = 0; i < 20; ++i) work->t84[i] = FOM_MAT_t1_c[i];
    fom_fused_t84(work->t80, work->t81, work->t83, work->t84);
    for (i = 0; i < 20; ++i) if (work->t84[i] < 0.0) work->t84[i] = 0.0;
    FOM_PROF_END(steps_20_relu);
    }
    /* t85 */
    {
    FOM_PROF_BEGIN(steps_30_lin);
    for (i = 0; i < 30; ++i) work->t85[i] = pp7[i];
    fom_fused_t85(work->t80, work->t82, work->t83, work->t85);
    FOM_PROF_END(steps_30_lin);
    }
    /* t86 */
    {
    FOM_PROF_BEGIN(steps_600);
    for (i = 0; i < 600; ++i) work->t86[i] = FOM_MAT_t0_c[i];
    fom_fused_t86(work->t83, work->t84, work->t85, work->t86);
    for (i = 0; i < 600; ++i) { fom_float v = work->t86[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t86[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
    FOM_PROF_END(steps_600);
    }
    /* t87 */
    {
    FOM_PROF_BEGIN(steps_20_relu);
    for (i = 0; i < 20; ++i) work->t87[i] = FOM_MAT_t1_c[i];
    fom_fused_t87(work->t83, work->t84, work->t86, work->t87);
    for (i = 0; i < 20; ++i) if (work->t87[i] < 0.0) work->t87[i] = 0.0;
    FOM_PROF_END(steps_20_relu);
    }
    /* t88 */
    {
    FOM_PROF_BEGIN(steps_30_lin);
    for (i = 0; i < 30; ++i) work->t88[i] = pp7[i];
    fom_fused_t88(work->t83, work->t85, work->t86, work->t88);
    FOM_PROF_END(steps_30_lin);
    }
    /* t89 */
    {
    FOM_PROF_BEGIN(steps_600);
    for (i = 0; i < 600; ++i) work->t89[i] = FOM_MAT_t0_c[i];
    fom_fused_t89(work->t86, work->t87, work->t88, work->t89);
    for (i = 0; i < 600; ++i) { fom_float v = work->t89[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t89[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
    FOM_PROF_END(steps_600);
    }
    /* t90 */
    {
    FOM_PROF_BEGIN(steps_20_relu);
    for (i = 0; i < 20; ++i) work->t90[i] = FOM_MAT_t1_c[i];
    fom_fused_t90(work->t86, work->t87, work->t89, work->t90);
    for (i = 0; i < 20; ++i) if (work->t90[i] < 0.0) work->t90[i] = 0.0;
    FOM_PROF_END(steps_20_relu);
    }
    /* t91 */
    {
    FOM_PROF_BEGIN(steps_30_lin);
    for (i = 0; i < 30; ++i) work->t91[i] = pp7[i];
    fom_fused_t91(work->t86, work->t88, work->t89, work->t91);
    FOM_PROF_END(steps_30_lin);
    }
    /* t92 */
    {
    FOM_PROF_BEGIN(steps_600);
    for (i = 0; i < 600; ++i) work->t92[i] = FOM_MAT_t0_c[i];
    fom_fused_t92(work->t89, work->t90, work->t91, work->t92);
    for (i = 0; i < 600; ++i) { fom_float v = work->t92[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t92[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
    FOM_PROF_END(steps_600);
    }
    /* t93 */
    {
    FOM_PROF_BEGIN(steps_20_relu);
    for (i = 0; i < 20; ++i) work->t93[i] = FOM_MAT_t1_c[i];
    fom_fused_t93(work->t89, work->t90, work->t92, work->t93);
    for (i = 0; i < 20; ++i) if (work->t93[i] < 0.0) work->t93[i] = 0.0;
    FOM_PROF_END(steps_20_relu);
    }
    /* t94 */
    {
    FOM_PROF_BEGIN(steps_30_lin);
    for (i = 0; i < 30; ++i) work->t94[i] = pp7[i];
    fom_fused_t94(work->t89, work->t91, work->t92, work->t94);
    FOM_PROF_END(steps_30_lin);
    }
    /* t95 */
    {
    FOM_PROF_BEGIN(steps_600);
    for (i = 0; i < 600; ++i) work->t95[i] = FOM_MAT_t0_c[i];
    fom_fused_t95(work->t92, work->t93, work->t94, work->t95);
    for (i = 0; i < 600; ++i) { fom_float v = work->t95[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t95[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
    FOM_PROF_END(steps_600);
    }
    /* t96 */
    {
    FOM_PROF_BEGIN(steps_20_relu);
    for (i = 0; i < 20; ++i) work->t96[i] = FOM_MAT_t1_c[i];
    fom_fused_t96(work->t92, work->t93, work->t95, work->t96);
    for (i = 0; i < 20; ++i) if (work->t96[i] < 0.0) work->t96[i] = 0.0;
    FOM_PROF_END(steps_20_relu);
    }
    /* t97 */
    {
    FOM_PROF_BEGIN(steps_30_lin);
    for (i = 0; i < 30; ++i) work->t97[i] = pp7[i];
    fom_fused_t97(work->t92, work->t94, work->t95, work->t97);
    FOM_PROF_END(steps_30_lin);
    }
    /* t98 */
    {
    FOM_PROF_BEGIN(steps_600);
    for (i = 0; i < 600; ++i) work->t98[i] = FOM_MAT_t0_c[i];
    fom_fused_t98(work->t95, work->t96, work->t97, work->t98);
    for (i = 0; i < 600; ++i) { fom_float v = work->t98[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t98[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
    FOM_PROF_END(steps_600);
    }
    /* t99 */
    {
    FOM_PROF_BEGIN(steps_20_relu);
    for (i = 0; i < 20; ++i) work->t99[i] = FOM_MAT_t1_c[i];
    fom_fused_t99(work->t95, work->t96, work->t98, work->t99);
    for (i = 0; i < 20; ++i) if (work->t99[i] < 0.0) work->t99[i] = 0.0;
    FOM_PROF_END(steps_20_relu);
    }
    /* t100 */
    {
    FOM_PROF_BEGIN(steps_30_lin);
    for (i = 0; i < 30; ++i) work->t100[i] = pp7[i];
    fom_fused_t100(work->t95, work->t97, work->t98, work->t100);
    FOM_PROF_END(steps_30_lin);
    }
    /* t101 */
    {
    FOM_PROF_BEGIN(steps_600);
    for (i = 0; i < 600; ++i) work->t101[i] = FOM_MAT_t0_c[i];
    fom_fused_t101(work->t98, work->t99, work->t100, work->t101);
    for (i = 0; i < 600; ++i) { fom_float v = work->t101[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t101[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
    FOM_PROF_END(steps_600);
    }
    /* t102 */
    {
    FOM_PROF_BEGIN(steps_20_relu);
    for (i = 0; i < 20; ++i) work->t102[i] = FOM_MAT_t1_c[i];
    fom_fused_t102(work->t98, work->t99, work->t101, work->t102);
    for (i = 0; i < 20; ++i) if (work->t102[i] < 0.0) work->t102[i] = 0.0;
    FOM_PROF_END(steps_20_relu);
    }
    /* t103 */
    {
    FOM_PROF_BEGIN(steps_30_lin);
    for (i = 0; i < 30; ++i) work->t103[i] = pp7[i];
    fom_fused_t103(work->t98, work->t100, work->t101, work->t103);
    FOM_PROF_END(steps_30_lin);
    }
    /* t104 */
    {
    FOM_PROF_BEGIN(steps_600);
    for (i = 0; i < 600; ++i) work->t104[i] = FOM_MAT_t0_c[i];
    fom_fused_t104(work->t101, work->t102, work->t103, work->t104);
    for (i = 0; i < 600; ++i) { fom_float v = work->t104[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t104[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
    FOM_PROF_END(steps_600);
    }
    /* t105 */
    {
    FOM_PROF_BEGIN(steps_20_relu);
    for (i = 0; i < 20; ++i) work->t105[i] = FOM_MAT_t1_c[i];
    fom_fused_t105(work->t101, work->t102, work->t104, work->t105);
    for (i = 0; i < 20; ++i) if (work->t105[i] < 0.0) work->t105[i] = 0.0;
    FOM_PROF_END(steps_20_relu);
    }
    /* t106 */
    {
    FOM_PROF_BEGIN(steps_30_lin);
    for (i = 0; i < 30; ++i) work->t106[i] = pp7[i];
    fom_fused_t106(work->t101, work->t103, work->t104, work->t106);
    FOM_PROF_END(steps_30_lin);
    }
    /* t107 */
    {
    FOM_PROF_BEGIN(steps_600);
    for (i = 0; i < 600; ++i) work->t107[i] = FOM_MAT_t0_c[i];
    fom_fused_t107(work->t104, work->t105, work->t106, work->t107);
    for (i = 0; i < 600; ++i) { fom_float v = work->t107[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t107[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
    FOM_PROF_END(steps_600);
    }
    /* t108 */
    {
    FOM_PROF_BEGIN(steps_20_relu);
    for (i = 0; i < 20; ++i) work->t108[i] = FOM_MAT_t1_c[i];
    fom_fused_t108(work->t104, work->t105, work->t107, work->t108);
    for (i = 0; i < 20; ++i) if (work->t108[i] < 0.0) work->t108[i] = 0.0;
    FOM_PROF_END(steps_20_relu);
    }
    /* t109 */
    {
    FOM_PROF_BEGIN(steps_30_lin);
    for (i = 0; i < 30; ++i) work->t109[i] = pp7[i];
    fom_fused_t109(work->t104, work->t106, work->t107, work->t109);
    FOM_PROF_END(steps_30_lin);
    }
    /* t110 */
    {
    FOM_PROF_BEGIN(steps_600);
    for (i = 0; i < 600; ++i) work->t110[i] = FOM_MAT_t0_c[i];
    fom_fused_t110(work->t107, work->t108, work->t109, work->t110);
    for (i = 0; i < 600; ++i) { fom_float v = work->t110[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t110[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
    FOM_PROF_END(steps_600);
    }
    /* t111 */
    {
    FOM_PROF_BEGIN(steps_20_relu);
    for (i = 0; i < 20; ++i) work->t111[i] = FOM_MAT_t1_c[i];
    fom_fused_t111(work->t107, work->t108, work->t110, work->t111);
    for (i = 0; i < 20; ++i) if (work->t111[i] < 0.0) work->t111[i] = 0.0;
    FOM_PROF_END(steps_20_relu);
    }
    /* t112 */
    {
    FOM_PROF_BEGIN(steps_30_lin);
    for (i = 0; i < 30; ++i) work->t112[i] = pp7[i];
    fom_fused_t112(work->t107, work->t109, work->t110, work->t112);
    FOM_PROF_END(steps_30_lin);
    }
    /* t113 */
    {
    FOM_PROF_BEGIN(steps_600);
    for (i = 0; i < 600; ++i) work->t113[i] = FOM_MAT_t0_c[i];
    fom_fused_t113(work->t110, work->t111, work->t112, work->t113);
    for (i = 0; i < 600; ++i) { fom_float v = work->t113[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t113[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
    FOM_PROF_END(steps_600);
    }
    /* t114 */
    {
    FOM_PROF_BEGIN(steps_20_relu);
    for (i = 0; i < 20; ++i) work->t114[i] = FOM_MAT_t1_c[i];
    fom_fused_t114(work->t110, work->t111, work->t113, work->t114);
    for (i = 0; i < 20; ++i) if (work->t114[i] < 0.0) work->t114[i] = 0.0;
    FOM_PROF_END(steps_20_relu);
    }
    /* t115 */
    {
    FOM_PROF_BEGIN(steps_30_lin);
    for (i = 0; i < 30; ++i) work->t115[i] = pp7[i];
    fom_fused_t115(work->t110, work->t112, work->t113, work->t115);
    FOM_PROF_END(steps_30_lin);
    }
    /* t116 */
    {
    FOM_PROF_BEGIN(steps_600);
    for (i = 0; i < 600; ++i) work->t116[i] = FOM_MAT_t0_c[i];
    fom_fused_t116(work->t113, work->t114, work->t115, work->t116);
    for (i = 0; i < 600; ++i) { fom_float v = work->t116[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t116[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
    FOM_PROF_END(steps_600);
    }
    /* t117 */
    {
    FOM_PROF_BEGIN(steps_20_relu);
    for (i = 0; i < 20; ++i) work->t117[i] = FOM_MAT_t1_c[i];
    fom_fused_t117(work->t113, work->t114, work->t116, work->t117);
    for (i = 0; i < 20; ++i) if (work->t117[i] < 0.0) work->t117[i] = 0.0;
    FOM_PROF_END(steps_20_relu);
    }
    /* t118 */
    {
    FOM_PROF_BEGIN(steps_30_lin);
    for (i = 0; i < 30; ++i) work->t118[i] = pp7[i];
    fom_fused_t118(work->t113, work->t115, work->t116, work->t118);
    FOM_PROF_END(steps_30_lin);
    }
    /* t119 */
    {
    FOM_PROF_BEGIN(steps_600);
    for (i = 0; i < 600; ++i) work->t119[i] = FOM_MAT_t0_c[i];
    fom_fused_t119(work->t116, work->t117, work->t118, work->t119);
    for (i = 0; i < 600; ++i) { fom_float v = work->t119[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t119[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
    FOM_PROF_END(steps_600);
    }
    /* t120 */
    {
    FOM_PROF_BEGIN(steps_20_relu);
    for (i = 0; i < 20; ++i) work->t120[i] = FOM_MAT_t1_c[i];
    fom_fused_t120(work->t116, work->t117, work->t119, work->t120);
    for (i = 0; i < 20; ++i) if (work->t120[i] < 0.0) work->t120[i] = 0.0;
    FOM_PROF_END(steps_20_relu);
    }
    /* t121 */
    {
    FOM_PROF_BEGIN(steps_30_lin);
    for (i = 0; i < 30; ++i) work->t121[i] = pp7[i];
    fom_fused_t121(work->t116, work->t118, work->t119, work->t121);
    FOM_PROF_END(steps_30_lin);
    }
    /* t122 */
    {
    FOM_PROF_BEGIN(steps_600);
    for (i = 0; i < 600; ++i) work->t122[i] = FOM_MAT_t0_c[i];
    fom_fused_t122(work->t119, work->t120, work->t121, work->t122);
    for (i = 0; i < 600; ++i) { fom_float v = work->t122[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t122[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
    FOM_PROF_END(steps_600);
    }
    /* t123 */
    {
    FOM_PROF_BEGIN(steps_20_relu);
    for (i = 0; i < 20; ++i) work->t123[i] = FOM_MAT_t1_c[i];
    fom_fused_t123(work->t119, work->t120, work->t122, work->t123);
    for (i = 0; i < 20; ++i) if (work->t123[i] < 0.0) work->t123[i] = 0.0;
    FOM_PROF_END(steps_20_relu);
    }
    /* t124 */
    {
    FOM_PROF_BEGIN(steps_30_lin);
    for (i = 0; i < 30; ++i) work->t124[i] = pp7[i];
    fom_fused_t124(work->t119, work->t121, work->t122, work->t124);
    FOM_PROF_END(steps_30_lin);
    }
    /* t125 */
    {
    FOM_PROF_BEGIN(steps_600);
    for (i = 0; i < 600; ++i) work->t125[i] = FOM_MAT_t0_c[i];
    fom_fused_t125(work->t122, work->t123, work->t124, work->t125);
    for (i = 0; i < 600; ++i) { fom_float v = work->t125[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t125[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
    FOM_PROF_END(steps_600);
    }
    /* t126 */
    {
    FOM_PROF_BEGIN(steps_20_relu);
    for (i = 0; i < 20; ++i) work->t126[i] = FOM_MAT_t1_c[i];
    fom_fused_t126(work->t122, work->t123, work->t125, work->t126);
    for (i = 0; i < 20; ++i) if (work->t126[i] < 0.0) work->t126[i] = 0.0;
    FOM_PROF_END(steps_20_relu);
    }
    /* t127 */
    {
    FOM_PROF_BEGIN(steps_30_lin);
    for (i = 0; i < 30; ++i) work->t127[i] = pp7[i];
    fom_fused_t127(work->t122, work->t124, work->t125, work->t127);
    FOM_PROF_END(steps_30_lin);
    }
    /* t128 */
    {
    FOM_PROF_BEGIN(steps_600);
    for (i = 0; i < 600; ++i) work->t128[i] = FOM_MAT_t0_c[i];
    fom_fused_t128(work->t125, work->t126, work->t127, work->t128);
    for (i = 0; i < 600; ++i) { fom_float v = work->t128[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t128[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
    FOM_PROF_END(steps_600);
    }
    /* t129 */
    {
    FOM_PROF_BEGIN(steps_20_relu);
    for (i = 0; i < 20; ++i) work->t129[i] = FOM_MAT_t1_c[i];
    fom_fused_t129(work->t125, work->t126, work->t128, work->t129);
    for (i = 0; i < 20; ++i) if (work->t129[i] < 0.0) work->t129[i] = 0.0;
    FOM_PROF_END(steps_20_relu);
    }
    /* t130 */
    {
    FOM_PROF_BEGIN(steps_30_lin);
    for (i = 0; i < 30; ++i) work->t130[i] = pp7[i];
    fom_fused_t130(work->t125, work->t127, work->t128, work->t130);
    FOM_PROF_END(steps_30_lin);
    }
    /* t131 */
    {
    FOM_PROF_BEGIN(steps_600);
    for (i = 0; i < 600; ++i) work->t131[i] = FOM_MAT_t0_c[i];
    fom_fused_t131(work->t128, work->t129, work->t130, work->t131);
    for (i = 0; i < 600; ++i) { fom_float v = work->t131[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t131[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
    FOM_PROF_END(steps_600);
    }
    /* t132 */
    {
    FOM_PROF_BEGIN(steps_20_relu);
    for (i = 0; i < 20; ++i) work->t132[i] = FOM_MAT_t1_c[i];
    fom_fused_t132(work->t128, work->t129, work->t131, work->t132);
    for (i = 0; i < 20; ++i) if (work->t132[i] < 0.0) work->t132[i] = 0.0;
    FOM_PROF_END(steps_20_relu);
    }
    /* t133 */
    {
    FOM_PROF_BEGIN(steps_30_lin);
    for (i = 0; i < 30; ++i) work->t133[i] = pp7[i];
    fom_fused_t133(work->t128, work->t130, work->t131, work->t133);
    FOM_PROF_END(steps_30_lin);
    }
    /* t134 */
    {
    FOM_PROF_BEGIN(steps_600);
    for (i = 0; i < 600; ++i) work->t134[i] = FOM_MAT_t0_c[i];
    fom_fused_t134(work->t131, work->t132, work->t133, work->t134);
    for (i = 0; i < 600; ++i) { fom_float v = work->t134[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t134[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
    FOM_PROF_END(steps_600);
    }
    /* t135 */
    {
    FOM_PROF_BEGIN(steps_20_relu);
    for (i = 0; i < 20; ++i) work->t135[i] = FOM_MAT_t1_c[i];
    fom_fused_t135(work->t131, work->t132, work->t134, work->t135);
    for (i = 0; i < 20; ++i) if (work->t135[i] < 0.0) work->t135[i] = 0.0;
    FOM_PROF_END(steps_20_relu);
    }
    /* t136 */
    {
    FOM_PROF_BEGIN(steps_30_lin);
    for (i = 0; i < 30; ++i) work->t136[i] = pp7[i];
    fom_fused_t136(work->t131, work->t133, work->t134, work->t136);
    FOM_PROF_END(steps_30_lin);
    }
    /* t137 */
    {
    FOM_PROF_BEGIN(steps_600);
    for (i = 0; i < 600; ++i) work->t137[i] = FOM_MAT_t0_c[i];
    fom_fused_t137(work->t134, work->t135, work->t136, work->t137);
    for (i = 0; i < 600; ++i) { fom_float v = work->t137[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t137[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
    FOM_PROF_END(steps_600);
    }
    /* t138 */
    {
    FOM_PROF_BEGIN(steps_20_relu);
    for (i = 0; i < 20; ++i) work->t138[i] = FOM_MAT_t1_c[i];
    fom_fused_t138(work->t134, work->t135, work->t137, work->t138);
    for (i = 0; i < 20; ++i) if (work->t138[i] < 0.0) work->t138[i] = 0.0;
    FOM_PROF_END(steps_20_relu);
    }
    /* t139 */
    {
    FOM_PROF_BEGIN(steps_30_lin);
    for (i = 0; i < 30; ++i) work->t139[i] = pp7[i];
    fom_fused_t139(work->t134, work->t136, work->t137, work->t139);
    FOM_PROF_END(steps_30_lin);
    }
    /* t140 */
    {
    FOM_PROF_BEGIN(steps_600);
    for (i = 0; i < 600; ++i) work->t140[i] = FOM_MAT_t0_c[i];
    fom_fused_t140(work->t137, work->t138, work->t139, work->t140);
    for (i = 0; i < 600; ++i) { fom_float v = work->t140[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t140[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
    FOM_PROF_END(steps_600);
    }
    /* t141 */
    {
    FOM_PROF_BEGIN(steps_20_relu);
    for (i = 0; i < 20; ++i) work->t141[i] = FOM_MAT_t1_c[i];
    fom_fused_t141(work->t137, work->t138, work->t140, work->t141);
    for (i = 0; i < 20; ++i) if (work->t141[i] < 0.0) work->t141[i] = 0.0;
    FOM_PROF_END(steps_20_relu);
    }
    /* t142 */
    {
    FOM_PROF_BEGIN(steps_30_lin);
    for (i = 0; i < 30; ++i) work->t142[i] = pp7[i];
    fom_fused_t142(work->t137, work->t139, work->t140, work->t142);
    FOM_PROF_END(steps_30_lin);
    }
    /* t143 */
    {
    FOM_PROF_BEGIN(steps_600);
    for (i = 0; i < 600; ++i) work->t143[i] = FOM_MAT_t0_c[i];
    fom_fused_t143(work->t140, work->t141, work->t142, work->t143);
    for (i = 0; i < 600; ++i) { fom_float v = work->t143[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t143[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
    FOM_PROF_END(steps_600);
    }
    /* t144 */
    {
    FOM_PROF_BEGIN(steps_20_relu);
    for (i = 0; i < 20; ++i) work->t144[i] = FOM_MAT_t1_c[i];
    fom_fused_t144(work->t140, work->t141, work->t143, work->t144);
    for (i = 0; i < 20; ++i) if (work->t144[i] < 0.0) work->t144[i] = 0.0;
    FOM_PROF_END(steps_20_relu);
    }
    /* t145 */
    {
    FOM_PROF_BEGIN(steps_30_lin);
    for (i = 0; i < 30; ++i) work->t145[i] = pp7[i];
    fom_fused_t145(work->t140, work->t142, work->t143, work->t145);
    FOM_PROF_END(steps_30_lin);
    }
    /* t146 */
    {
    FOM_PROF_BEGIN(steps_600);
    for (i = 0; i < 600; ++i) work->t146[i] = FOM_MAT_t0_c[i];
    fom_fused_t146(work->t143, work->t144, work->t145, work->t146);
    for (i = 0; i < 600; ++i) { fom_float v = work->t146[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t146[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
    FOM_PROF_END(steps_600);
    }
    /* t147 */
    {
    FOM_PROF_BEGIN(steps_20_relu);
    for (i = 0; i < 20; ++i) work->t147[i] = FOM_MAT_t1_c[i];
    fom_fused_t147(work->t143, work->t144, work->t146, work->t147);
    for (i = 0; i < 20; ++i) if (work->t147[i] < 0.0) work->t147[i] = 0.0;
    FOM_PROF_END(steps_20_relu);
    }
    /* t148 */
    {
    FOM_PROF_BEGIN(steps_30_lin);
    for (i = 0; i < 30; ++i) work->t148[i] = pp7[i];
    fom_fused_t148(work->t143, work->t145, work->t146, work->t148);
    FOM_PROF_END(steps_30_lin);
    }
    /* final state */
    {
    FOM_PROF_BEGIN(state_update);
    for (i = 0; i < FOM_STATE_DIM; ++i) work->new_state[i] = 0.0;
    fom_fused_su(work->t146, work->t147, work->t148, work->new_state);
    for (i = 0; i < FOM_STATE_DIM; ++i) {
        fom_float d = work->new_state[i] - work->state[i];
        d = d < 0.0 ? -d : d;
        if (d > res) res = d;
        work->state[i] = work->new_state[i];
        work->solution.x[i] = work->new_state[i];
    }
    FOM_PROF_END(state_update);
    }
    work->solution.iter = 1;
    work->solution.residual = res;
    work->solution.status = FOM_MAX_ITER_REACHED;
    return FOM_MAX_ITER_REACHED;
}

void fom_cleanup(FomWorkspace *work) {
    (void)work;
}
