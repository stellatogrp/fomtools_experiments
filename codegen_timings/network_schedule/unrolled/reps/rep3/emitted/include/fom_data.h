#ifndef FOM_DATA_H
#define FOM_DATA_H

#include "fom_workspace.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Initial state. */
extern const fom_float FOM_STATE_INIT[FOM_STATE_DIM];

#define FOM_MAT_t0_state_ROWS 600
#define FOM_MAT_t0_state_COLS 650
#define FOM_MAT_t0_state_NNZ 1800
extern const fom_int FOM_MAT_t0_state_ROWPTR[];
extern const fom_int FOM_MAT_t0_state_COLIDX[];
extern const fom_float FOM_MAT_t0_state_VALS[];

#define FOM_MAT_t0_c_ROWS 600
#define FOM_MAT_t0_c_COLS 1
extern const fom_float FOM_MAT_t0_c[];

#define FOM_MAT_t0_lo_ROWS 600
#define FOM_MAT_t0_lo_COLS 1
extern const fom_float FOM_MAT_t0_lo[];

#define FOM_MAT_t0_hi_ROWS 600
#define FOM_MAT_t0_hi_COLS 1
extern const fom_float FOM_MAT_t0_hi[];

#define FOM_MAT_t1_state_ROWS 20
#define FOM_MAT_t1_state_COLS 650
#define FOM_MAT_t1_state_NNZ 620
extern const fom_int FOM_MAT_t1_state_ROWPTR[];
extern const fom_int FOM_MAT_t1_state_COLIDX[];
extern const fom_float FOM_MAT_t1_state_VALS[];

#define FOM_MAT_t1_t_t0_ROWS 20
#define FOM_MAT_t1_t_t0_COLS 600
#define FOM_MAT_t1_t_t0_NNZ 600
extern const fom_int FOM_MAT_t1_t_t0_ROWPTR[];
extern const fom_int FOM_MAT_t1_t_t0_COLIDX[];
extern const fom_float FOM_MAT_t1_t_t0_VALS[];

#define FOM_MAT_t1_c_ROWS 20
#define FOM_MAT_t1_c_COLS 1
extern const fom_float FOM_MAT_t1_c[];

#define FOM_MAT_t2_state_ROWS 600
#define FOM_MAT_t2_state_COLS 650
#define FOM_MAT_t2_state_NNZ 12600
extern const fom_int FOM_MAT_t2_state_ROWPTR[];
extern const fom_int FOM_MAT_t2_state_COLIDX[];
extern const fom_float FOM_MAT_t2_state_VALS[];

#define FOM_MAT_t2_p_p0_ROWS 600
#define FOM_MAT_t2_p_p0_COLS 30
#define FOM_MAT_t2_p_p0_NNZ 600
extern const fom_int FOM_MAT_t2_p_p0_ROWPTR[];
extern const fom_int FOM_MAT_t2_p_p0_COLIDX[];
extern const fom_float FOM_MAT_t2_p_p0_VALS[];

#define FOM_MAT_t2_t_t0_ROWS 600
#define FOM_MAT_t2_t_t0_COLS 600
#define FOM_MAT_t2_t_t0_NNZ 12000
extern const fom_int FOM_MAT_t2_t_t0_ROWPTR[];
extern const fom_int FOM_MAT_t2_t_t0_COLIDX[];
extern const fom_float FOM_MAT_t2_t_t0_VALS[];

#define FOM_MAT_t2_t_t1_ROWS 600
#define FOM_MAT_t2_t_t1_COLS 20
#define FOM_MAT_t2_t_t1_NNZ 600
extern const fom_int FOM_MAT_t2_t_t1_ROWPTR[];
extern const fom_int FOM_MAT_t2_t_t1_COLIDX[];
extern const fom_float FOM_MAT_t2_t_t1_VALS[];

#define FOM_MAT_t3_t_t0_ROWS 20
#define FOM_MAT_t3_t_t0_COLS 600
#define FOM_MAT_t3_t_t0_NNZ 600
extern const fom_int FOM_MAT_t3_t_t0_ROWPTR[];
extern const fom_int FOM_MAT_t3_t_t0_COLIDX[];
extern const fom_float FOM_MAT_t3_t_t0_VALS[];

#define FOM_MAT_t3_t_t1_ROWS 20
#define FOM_MAT_t3_t_t1_COLS 20
#define FOM_MAT_t3_t_t1_NNZ 20
extern const fom_int FOM_MAT_t3_t_t1_ROWPTR[];
extern const fom_int FOM_MAT_t3_t_t1_COLIDX[];
extern const fom_float FOM_MAT_t3_t_t1_VALS[];

#define FOM_MAT_t3_t_t2_ROWS 20
#define FOM_MAT_t3_t_t2_COLS 600
#define FOM_MAT_t3_t_t2_NNZ 600
extern const fom_int FOM_MAT_t3_t_t2_ROWPTR[];
extern const fom_int FOM_MAT_t3_t_t2_COLIDX[];
extern const fom_float FOM_MAT_t3_t_t2_VALS[];

#define FOM_MAT_t4_state_ROWS 30
#define FOM_MAT_t4_state_COLS 650
#define FOM_MAT_t4_state_NNZ 630
extern const fom_int FOM_MAT_t4_state_ROWPTR[];
extern const fom_int FOM_MAT_t4_state_COLIDX[];
extern const fom_float FOM_MAT_t4_state_VALS[];

#define FOM_MAT_t4_p_p0_ROWS 30
#define FOM_MAT_t4_p_p0_COLS 30
#define FOM_MAT_t4_p_p0_NNZ 30
extern const fom_int FOM_MAT_t4_p_p0_ROWPTR[];
extern const fom_int FOM_MAT_t4_p_p0_COLIDX[];
extern const fom_float FOM_MAT_t4_p_p0_VALS[];

#define FOM_MAT_t4_t_t0_ROWS 30
#define FOM_MAT_t4_t_t0_COLS 600
#define FOM_MAT_t4_t_t0_NNZ 600
extern const fom_int FOM_MAT_t4_t_t0_ROWPTR[];
extern const fom_int FOM_MAT_t4_t_t0_COLIDX[];
extern const fom_float FOM_MAT_t4_t_t0_VALS[];

#define FOM_MAT_t4_t_t2_ROWS 30
#define FOM_MAT_t4_t_t2_COLS 600
#define FOM_MAT_t4_t_t2_NNZ 600
extern const fom_int FOM_MAT_t4_t_t2_ROWPTR[];
extern const fom_int FOM_MAT_t4_t_t2_COLIDX[];
extern const fom_float FOM_MAT_t4_t_t2_VALS[];

#define FOM_MAT_t5_t_t2_ROWS 600
#define FOM_MAT_t5_t_t2_COLS 600
#define FOM_MAT_t5_t_t2_NNZ 600
extern const fom_int FOM_MAT_t5_t_t2_ROWPTR[];
extern const fom_int FOM_MAT_t5_t_t2_COLIDX[];
extern const fom_float FOM_MAT_t5_t_t2_VALS[];

#define FOM_MAT_t5_t_t4_ROWS 600
#define FOM_MAT_t5_t_t4_COLS 30
#define FOM_MAT_t5_t_t4_NNZ 600
extern const fom_int FOM_MAT_t5_t_t4_ROWPTR[];
extern const fom_int FOM_MAT_t5_t_t4_COLIDX[];
extern const fom_float FOM_MAT_t5_t_t4_VALS[];

#define FOM_MAT_t6_t_t2_ROWS 20
#define FOM_MAT_t6_t_t2_COLS 600
#define FOM_MAT_t6_t_t2_NNZ 600
extern const fom_int FOM_MAT_t6_t_t2_ROWPTR[];
extern const fom_int FOM_MAT_t6_t_t2_COLIDX[];
extern const fom_float FOM_MAT_t6_t_t2_VALS[];

#define FOM_MAT_t6_t_t5_ROWS 20
#define FOM_MAT_t6_t_t5_COLS 600
#define FOM_MAT_t6_t_t5_NNZ 600
extern const fom_int FOM_MAT_t6_t_t5_ROWPTR[];
extern const fom_int FOM_MAT_t6_t_t5_COLIDX[];
extern const fom_float FOM_MAT_t6_t_t5_VALS[];

#define FOM_MAT_t7_p_p0_ROWS 30
#define FOM_MAT_t7_p_p0_COLS 30
#define FOM_MAT_t7_p_p0_NNZ 30
extern const fom_int FOM_MAT_t7_p_p0_ROWPTR[];
extern const fom_int FOM_MAT_t7_p_p0_COLIDX[];
extern const fom_float FOM_MAT_t7_p_p0_VALS[];

#define FOM_MAT_t7_t_t2_ROWS 30
#define FOM_MAT_t7_t_t2_COLS 600
#define FOM_MAT_t7_t_t2_NNZ 600
extern const fom_int FOM_MAT_t7_t_t2_ROWPTR[];
extern const fom_int FOM_MAT_t7_t_t2_COLIDX[];
extern const fom_float FOM_MAT_t7_t_t2_VALS[];

#define FOM_MAT_t7_t_t4_ROWS 30
#define FOM_MAT_t7_t_t4_COLS 30
#define FOM_MAT_t7_t_t4_NNZ 30
extern const fom_int FOM_MAT_t7_t_t4_ROWPTR[];
extern const fom_int FOM_MAT_t7_t_t4_COLIDX[];
extern const fom_float FOM_MAT_t7_t_t4_VALS[];

#define FOM_MAT_t7_t_t5_ROWS 30
#define FOM_MAT_t7_t_t5_COLS 600
#define FOM_MAT_t7_t_t5_NNZ 600
extern const fom_int FOM_MAT_t7_t_t5_ROWPTR[];
extern const fom_int FOM_MAT_t7_t_t5_COLIDX[];
extern const fom_float FOM_MAT_t7_t_t5_VALS[];

#define FOM_MAT_t9_t_t5_ROWS 20
#define FOM_MAT_t9_t_t5_COLS 600
#define FOM_MAT_t9_t_t5_NNZ 600
extern const fom_int FOM_MAT_t9_t_t5_ROWPTR[];
extern const fom_int FOM_MAT_t9_t_t5_COLIDX[];
extern const fom_float FOM_MAT_t9_t_t5_VALS[];

#define FOM_MAT_t9_t_t8_ROWS 20
#define FOM_MAT_t9_t_t8_COLS 600
#define FOM_MAT_t9_t_t8_NNZ 600
extern const fom_int FOM_MAT_t9_t_t8_ROWPTR[];
extern const fom_int FOM_MAT_t9_t_t8_COLIDX[];
extern const fom_float FOM_MAT_t9_t_t8_VALS[];

#define FOM_MAT_t10_t_t5_ROWS 30
#define FOM_MAT_t10_t_t5_COLS 600
#define FOM_MAT_t10_t_t5_NNZ 600
extern const fom_int FOM_MAT_t10_t_t5_ROWPTR[];
extern const fom_int FOM_MAT_t10_t_t5_COLIDX[];
extern const fom_float FOM_MAT_t10_t_t5_VALS[];

#define FOM_MAT_t10_t_t8_ROWS 30
#define FOM_MAT_t10_t_t8_COLS 600
#define FOM_MAT_t10_t_t8_NNZ 600
extern const fom_int FOM_MAT_t10_t_t8_ROWPTR[];
extern const fom_int FOM_MAT_t10_t_t8_COLIDX[];
extern const fom_float FOM_MAT_t10_t_t8_VALS[];

#define FOM_MAT_t12_t_t8_ROWS 20
#define FOM_MAT_t12_t_t8_COLS 600
#define FOM_MAT_t12_t_t8_NNZ 600
extern const fom_int FOM_MAT_t12_t_t8_ROWPTR[];
extern const fom_int FOM_MAT_t12_t_t8_COLIDX[];
extern const fom_float FOM_MAT_t12_t_t8_VALS[];

#define FOM_MAT_t12_t_t11_ROWS 20
#define FOM_MAT_t12_t_t11_COLS 600
#define FOM_MAT_t12_t_t11_NNZ 600
extern const fom_int FOM_MAT_t12_t_t11_ROWPTR[];
extern const fom_int FOM_MAT_t12_t_t11_COLIDX[];
extern const fom_float FOM_MAT_t12_t_t11_VALS[];

#define FOM_MAT_t13_t_t8_ROWS 30
#define FOM_MAT_t13_t_t8_COLS 600
#define FOM_MAT_t13_t_t8_NNZ 600
extern const fom_int FOM_MAT_t13_t_t8_ROWPTR[];
extern const fom_int FOM_MAT_t13_t_t8_COLIDX[];
extern const fom_float FOM_MAT_t13_t_t8_VALS[];

#define FOM_MAT_t13_t_t11_ROWS 30
#define FOM_MAT_t13_t_t11_COLS 600
#define FOM_MAT_t13_t_t11_NNZ 600
extern const fom_int FOM_MAT_t13_t_t11_ROWPTR[];
extern const fom_int FOM_MAT_t13_t_t11_COLIDX[];
extern const fom_float FOM_MAT_t13_t_t11_VALS[];

#define FOM_MAT_t15_t_t11_ROWS 20
#define FOM_MAT_t15_t_t11_COLS 600
#define FOM_MAT_t15_t_t11_NNZ 600
extern const fom_int FOM_MAT_t15_t_t11_ROWPTR[];
extern const fom_int FOM_MAT_t15_t_t11_COLIDX[];
extern const fom_float FOM_MAT_t15_t_t11_VALS[];

#define FOM_MAT_t15_t_t14_ROWS 20
#define FOM_MAT_t15_t_t14_COLS 600
#define FOM_MAT_t15_t_t14_NNZ 600
extern const fom_int FOM_MAT_t15_t_t14_ROWPTR[];
extern const fom_int FOM_MAT_t15_t_t14_COLIDX[];
extern const fom_float FOM_MAT_t15_t_t14_VALS[];

#define FOM_MAT_t16_t_t11_ROWS 30
#define FOM_MAT_t16_t_t11_COLS 600
#define FOM_MAT_t16_t_t11_NNZ 600
extern const fom_int FOM_MAT_t16_t_t11_ROWPTR[];
extern const fom_int FOM_MAT_t16_t_t11_COLIDX[];
extern const fom_float FOM_MAT_t16_t_t11_VALS[];

#define FOM_MAT_t16_t_t14_ROWS 30
#define FOM_MAT_t16_t_t14_COLS 600
#define FOM_MAT_t16_t_t14_NNZ 600
extern const fom_int FOM_MAT_t16_t_t14_ROWPTR[];
extern const fom_int FOM_MAT_t16_t_t14_COLIDX[];
extern const fom_float FOM_MAT_t16_t_t14_VALS[];

#define FOM_MAT_t18_t_t14_ROWS 20
#define FOM_MAT_t18_t_t14_COLS 600
#define FOM_MAT_t18_t_t14_NNZ 600
extern const fom_int FOM_MAT_t18_t_t14_ROWPTR[];
extern const fom_int FOM_MAT_t18_t_t14_COLIDX[];
extern const fom_float FOM_MAT_t18_t_t14_VALS[];

#define FOM_MAT_t18_t_t17_ROWS 20
#define FOM_MAT_t18_t_t17_COLS 600
#define FOM_MAT_t18_t_t17_NNZ 600
extern const fom_int FOM_MAT_t18_t_t17_ROWPTR[];
extern const fom_int FOM_MAT_t18_t_t17_COLIDX[];
extern const fom_float FOM_MAT_t18_t_t17_VALS[];

#define FOM_MAT_t19_t_t14_ROWS 30
#define FOM_MAT_t19_t_t14_COLS 600
#define FOM_MAT_t19_t_t14_NNZ 600
extern const fom_int FOM_MAT_t19_t_t14_ROWPTR[];
extern const fom_int FOM_MAT_t19_t_t14_COLIDX[];
extern const fom_float FOM_MAT_t19_t_t14_VALS[];

#define FOM_MAT_t19_t_t17_ROWS 30
#define FOM_MAT_t19_t_t17_COLS 600
#define FOM_MAT_t19_t_t17_NNZ 600
extern const fom_int FOM_MAT_t19_t_t17_ROWPTR[];
extern const fom_int FOM_MAT_t19_t_t17_COLIDX[];
extern const fom_float FOM_MAT_t19_t_t17_VALS[];

#define FOM_MAT_t21_t_t17_ROWS 20
#define FOM_MAT_t21_t_t17_COLS 600
#define FOM_MAT_t21_t_t17_NNZ 600
extern const fom_int FOM_MAT_t21_t_t17_ROWPTR[];
extern const fom_int FOM_MAT_t21_t_t17_COLIDX[];
extern const fom_float FOM_MAT_t21_t_t17_VALS[];

#define FOM_MAT_t21_t_t20_ROWS 20
#define FOM_MAT_t21_t_t20_COLS 600
#define FOM_MAT_t21_t_t20_NNZ 600
extern const fom_int FOM_MAT_t21_t_t20_ROWPTR[];
extern const fom_int FOM_MAT_t21_t_t20_COLIDX[];
extern const fom_float FOM_MAT_t21_t_t20_VALS[];

#define FOM_MAT_t22_t_t17_ROWS 30
#define FOM_MAT_t22_t_t17_COLS 600
#define FOM_MAT_t22_t_t17_NNZ 600
extern const fom_int FOM_MAT_t22_t_t17_ROWPTR[];
extern const fom_int FOM_MAT_t22_t_t17_COLIDX[];
extern const fom_float FOM_MAT_t22_t_t17_VALS[];

#define FOM_MAT_t22_t_t20_ROWS 30
#define FOM_MAT_t22_t_t20_COLS 600
#define FOM_MAT_t22_t_t20_NNZ 600
extern const fom_int FOM_MAT_t22_t_t20_ROWPTR[];
extern const fom_int FOM_MAT_t22_t_t20_COLIDX[];
extern const fom_float FOM_MAT_t22_t_t20_VALS[];

#define FOM_MAT_t24_t_t20_ROWS 20
#define FOM_MAT_t24_t_t20_COLS 600
#define FOM_MAT_t24_t_t20_NNZ 600
extern const fom_int FOM_MAT_t24_t_t20_ROWPTR[];
extern const fom_int FOM_MAT_t24_t_t20_COLIDX[];
extern const fom_float FOM_MAT_t24_t_t20_VALS[];

#define FOM_MAT_t24_t_t23_ROWS 20
#define FOM_MAT_t24_t_t23_COLS 600
#define FOM_MAT_t24_t_t23_NNZ 600
extern const fom_int FOM_MAT_t24_t_t23_ROWPTR[];
extern const fom_int FOM_MAT_t24_t_t23_COLIDX[];
extern const fom_float FOM_MAT_t24_t_t23_VALS[];

#define FOM_MAT_t25_t_t20_ROWS 30
#define FOM_MAT_t25_t_t20_COLS 600
#define FOM_MAT_t25_t_t20_NNZ 600
extern const fom_int FOM_MAT_t25_t_t20_ROWPTR[];
extern const fom_int FOM_MAT_t25_t_t20_COLIDX[];
extern const fom_float FOM_MAT_t25_t_t20_VALS[];

#define FOM_MAT_t25_t_t23_ROWS 30
#define FOM_MAT_t25_t_t23_COLS 600
#define FOM_MAT_t25_t_t23_NNZ 600
extern const fom_int FOM_MAT_t25_t_t23_ROWPTR[];
extern const fom_int FOM_MAT_t25_t_t23_COLIDX[];
extern const fom_float FOM_MAT_t25_t_t23_VALS[];

#define FOM_MAT_t27_t_t23_ROWS 20
#define FOM_MAT_t27_t_t23_COLS 600
#define FOM_MAT_t27_t_t23_NNZ 600
extern const fom_int FOM_MAT_t27_t_t23_ROWPTR[];
extern const fom_int FOM_MAT_t27_t_t23_COLIDX[];
extern const fom_float FOM_MAT_t27_t_t23_VALS[];

#define FOM_MAT_t27_t_t26_ROWS 20
#define FOM_MAT_t27_t_t26_COLS 600
#define FOM_MAT_t27_t_t26_NNZ 600
extern const fom_int FOM_MAT_t27_t_t26_ROWPTR[];
extern const fom_int FOM_MAT_t27_t_t26_COLIDX[];
extern const fom_float FOM_MAT_t27_t_t26_VALS[];

#define FOM_MAT_t28_t_t23_ROWS 30
#define FOM_MAT_t28_t_t23_COLS 600
#define FOM_MAT_t28_t_t23_NNZ 600
extern const fom_int FOM_MAT_t28_t_t23_ROWPTR[];
extern const fom_int FOM_MAT_t28_t_t23_COLIDX[];
extern const fom_float FOM_MAT_t28_t_t23_VALS[];

#define FOM_MAT_t28_t_t26_ROWS 30
#define FOM_MAT_t28_t_t26_COLS 600
#define FOM_MAT_t28_t_t26_NNZ 600
extern const fom_int FOM_MAT_t28_t_t26_ROWPTR[];
extern const fom_int FOM_MAT_t28_t_t26_COLIDX[];
extern const fom_float FOM_MAT_t28_t_t26_VALS[];

#define FOM_MAT_t30_t_t26_ROWS 20
#define FOM_MAT_t30_t_t26_COLS 600
#define FOM_MAT_t30_t_t26_NNZ 600
extern const fom_int FOM_MAT_t30_t_t26_ROWPTR[];
extern const fom_int FOM_MAT_t30_t_t26_COLIDX[];
extern const fom_float FOM_MAT_t30_t_t26_VALS[];

#define FOM_MAT_t30_t_t29_ROWS 20
#define FOM_MAT_t30_t_t29_COLS 600
#define FOM_MAT_t30_t_t29_NNZ 600
extern const fom_int FOM_MAT_t30_t_t29_ROWPTR[];
extern const fom_int FOM_MAT_t30_t_t29_COLIDX[];
extern const fom_float FOM_MAT_t30_t_t29_VALS[];

#define FOM_MAT_t31_t_t26_ROWS 30
#define FOM_MAT_t31_t_t26_COLS 600
#define FOM_MAT_t31_t_t26_NNZ 600
extern const fom_int FOM_MAT_t31_t_t26_ROWPTR[];
extern const fom_int FOM_MAT_t31_t_t26_COLIDX[];
extern const fom_float FOM_MAT_t31_t_t26_VALS[];

#define FOM_MAT_t31_t_t29_ROWS 30
#define FOM_MAT_t31_t_t29_COLS 600
#define FOM_MAT_t31_t_t29_NNZ 600
extern const fom_int FOM_MAT_t31_t_t29_ROWPTR[];
extern const fom_int FOM_MAT_t31_t_t29_COLIDX[];
extern const fom_float FOM_MAT_t31_t_t29_VALS[];

#define FOM_MAT_t33_t_t29_ROWS 20
#define FOM_MAT_t33_t_t29_COLS 600
#define FOM_MAT_t33_t_t29_NNZ 600
extern const fom_int FOM_MAT_t33_t_t29_ROWPTR[];
extern const fom_int FOM_MAT_t33_t_t29_COLIDX[];
extern const fom_float FOM_MAT_t33_t_t29_VALS[];

#define FOM_MAT_t33_t_t32_ROWS 20
#define FOM_MAT_t33_t_t32_COLS 600
#define FOM_MAT_t33_t_t32_NNZ 600
extern const fom_int FOM_MAT_t33_t_t32_ROWPTR[];
extern const fom_int FOM_MAT_t33_t_t32_COLIDX[];
extern const fom_float FOM_MAT_t33_t_t32_VALS[];

#define FOM_MAT_t34_t_t29_ROWS 30
#define FOM_MAT_t34_t_t29_COLS 600
#define FOM_MAT_t34_t_t29_NNZ 600
extern const fom_int FOM_MAT_t34_t_t29_ROWPTR[];
extern const fom_int FOM_MAT_t34_t_t29_COLIDX[];
extern const fom_float FOM_MAT_t34_t_t29_VALS[];

#define FOM_MAT_t34_t_t32_ROWS 30
#define FOM_MAT_t34_t_t32_COLS 600
#define FOM_MAT_t34_t_t32_NNZ 600
extern const fom_int FOM_MAT_t34_t_t32_ROWPTR[];
extern const fom_int FOM_MAT_t34_t_t32_COLIDX[];
extern const fom_float FOM_MAT_t34_t_t32_VALS[];

#define FOM_MAT_t36_t_t32_ROWS 20
#define FOM_MAT_t36_t_t32_COLS 600
#define FOM_MAT_t36_t_t32_NNZ 600
extern const fom_int FOM_MAT_t36_t_t32_ROWPTR[];
extern const fom_int FOM_MAT_t36_t_t32_COLIDX[];
extern const fom_float FOM_MAT_t36_t_t32_VALS[];

#define FOM_MAT_t36_t_t35_ROWS 20
#define FOM_MAT_t36_t_t35_COLS 600
#define FOM_MAT_t36_t_t35_NNZ 600
extern const fom_int FOM_MAT_t36_t_t35_ROWPTR[];
extern const fom_int FOM_MAT_t36_t_t35_COLIDX[];
extern const fom_float FOM_MAT_t36_t_t35_VALS[];

#define FOM_MAT_t37_t_t32_ROWS 30
#define FOM_MAT_t37_t_t32_COLS 600
#define FOM_MAT_t37_t_t32_NNZ 600
extern const fom_int FOM_MAT_t37_t_t32_ROWPTR[];
extern const fom_int FOM_MAT_t37_t_t32_COLIDX[];
extern const fom_float FOM_MAT_t37_t_t32_VALS[];

#define FOM_MAT_t37_t_t35_ROWS 30
#define FOM_MAT_t37_t_t35_COLS 600
#define FOM_MAT_t37_t_t35_NNZ 600
extern const fom_int FOM_MAT_t37_t_t35_ROWPTR[];
extern const fom_int FOM_MAT_t37_t_t35_COLIDX[];
extern const fom_float FOM_MAT_t37_t_t35_VALS[];

#define FOM_MAT_t39_t_t35_ROWS 20
#define FOM_MAT_t39_t_t35_COLS 600
#define FOM_MAT_t39_t_t35_NNZ 600
extern const fom_int FOM_MAT_t39_t_t35_ROWPTR[];
extern const fom_int FOM_MAT_t39_t_t35_COLIDX[];
extern const fom_float FOM_MAT_t39_t_t35_VALS[];

#define FOM_MAT_t39_t_t38_ROWS 20
#define FOM_MAT_t39_t_t38_COLS 600
#define FOM_MAT_t39_t_t38_NNZ 600
extern const fom_int FOM_MAT_t39_t_t38_ROWPTR[];
extern const fom_int FOM_MAT_t39_t_t38_COLIDX[];
extern const fom_float FOM_MAT_t39_t_t38_VALS[];

#define FOM_MAT_t40_t_t35_ROWS 30
#define FOM_MAT_t40_t_t35_COLS 600
#define FOM_MAT_t40_t_t35_NNZ 600
extern const fom_int FOM_MAT_t40_t_t35_ROWPTR[];
extern const fom_int FOM_MAT_t40_t_t35_COLIDX[];
extern const fom_float FOM_MAT_t40_t_t35_VALS[];

#define FOM_MAT_t40_t_t38_ROWS 30
#define FOM_MAT_t40_t_t38_COLS 600
#define FOM_MAT_t40_t_t38_NNZ 600
extern const fom_int FOM_MAT_t40_t_t38_ROWPTR[];
extern const fom_int FOM_MAT_t40_t_t38_COLIDX[];
extern const fom_float FOM_MAT_t40_t_t38_VALS[];

#define FOM_MAT_t42_t_t38_ROWS 20
#define FOM_MAT_t42_t_t38_COLS 600
#define FOM_MAT_t42_t_t38_NNZ 600
extern const fom_int FOM_MAT_t42_t_t38_ROWPTR[];
extern const fom_int FOM_MAT_t42_t_t38_COLIDX[];
extern const fom_float FOM_MAT_t42_t_t38_VALS[];

#define FOM_MAT_t42_t_t41_ROWS 20
#define FOM_MAT_t42_t_t41_COLS 600
#define FOM_MAT_t42_t_t41_NNZ 600
extern const fom_int FOM_MAT_t42_t_t41_ROWPTR[];
extern const fom_int FOM_MAT_t42_t_t41_COLIDX[];
extern const fom_float FOM_MAT_t42_t_t41_VALS[];

#define FOM_MAT_t43_t_t38_ROWS 30
#define FOM_MAT_t43_t_t38_COLS 600
#define FOM_MAT_t43_t_t38_NNZ 600
extern const fom_int FOM_MAT_t43_t_t38_ROWPTR[];
extern const fom_int FOM_MAT_t43_t_t38_COLIDX[];
extern const fom_float FOM_MAT_t43_t_t38_VALS[];

#define FOM_MAT_t43_t_t41_ROWS 30
#define FOM_MAT_t43_t_t41_COLS 600
#define FOM_MAT_t43_t_t41_NNZ 600
extern const fom_int FOM_MAT_t43_t_t41_ROWPTR[];
extern const fom_int FOM_MAT_t43_t_t41_COLIDX[];
extern const fom_float FOM_MAT_t43_t_t41_VALS[];

#define FOM_MAT_t45_t_t41_ROWS 20
#define FOM_MAT_t45_t_t41_COLS 600
#define FOM_MAT_t45_t_t41_NNZ 600
extern const fom_int FOM_MAT_t45_t_t41_ROWPTR[];
extern const fom_int FOM_MAT_t45_t_t41_COLIDX[];
extern const fom_float FOM_MAT_t45_t_t41_VALS[];

#define FOM_MAT_t45_t_t44_ROWS 20
#define FOM_MAT_t45_t_t44_COLS 600
#define FOM_MAT_t45_t_t44_NNZ 600
extern const fom_int FOM_MAT_t45_t_t44_ROWPTR[];
extern const fom_int FOM_MAT_t45_t_t44_COLIDX[];
extern const fom_float FOM_MAT_t45_t_t44_VALS[];

#define FOM_MAT_t46_t_t41_ROWS 30
#define FOM_MAT_t46_t_t41_COLS 600
#define FOM_MAT_t46_t_t41_NNZ 600
extern const fom_int FOM_MAT_t46_t_t41_ROWPTR[];
extern const fom_int FOM_MAT_t46_t_t41_COLIDX[];
extern const fom_float FOM_MAT_t46_t_t41_VALS[];

#define FOM_MAT_t46_t_t44_ROWS 30
#define FOM_MAT_t46_t_t44_COLS 600
#define FOM_MAT_t46_t_t44_NNZ 600
extern const fom_int FOM_MAT_t46_t_t44_ROWPTR[];
extern const fom_int FOM_MAT_t46_t_t44_COLIDX[];
extern const fom_float FOM_MAT_t46_t_t44_VALS[];

#define FOM_MAT_t48_t_t44_ROWS 20
#define FOM_MAT_t48_t_t44_COLS 600
#define FOM_MAT_t48_t_t44_NNZ 600
extern const fom_int FOM_MAT_t48_t_t44_ROWPTR[];
extern const fom_int FOM_MAT_t48_t_t44_COLIDX[];
extern const fom_float FOM_MAT_t48_t_t44_VALS[];

#define FOM_MAT_t48_t_t47_ROWS 20
#define FOM_MAT_t48_t_t47_COLS 600
#define FOM_MAT_t48_t_t47_NNZ 600
extern const fom_int FOM_MAT_t48_t_t47_ROWPTR[];
extern const fom_int FOM_MAT_t48_t_t47_COLIDX[];
extern const fom_float FOM_MAT_t48_t_t47_VALS[];

#define FOM_MAT_t49_t_t44_ROWS 30
#define FOM_MAT_t49_t_t44_COLS 600
#define FOM_MAT_t49_t_t44_NNZ 600
extern const fom_int FOM_MAT_t49_t_t44_ROWPTR[];
extern const fom_int FOM_MAT_t49_t_t44_COLIDX[];
extern const fom_float FOM_MAT_t49_t_t44_VALS[];

#define FOM_MAT_t49_t_t47_ROWS 30
#define FOM_MAT_t49_t_t47_COLS 600
#define FOM_MAT_t49_t_t47_NNZ 600
extern const fom_int FOM_MAT_t49_t_t47_ROWPTR[];
extern const fom_int FOM_MAT_t49_t_t47_COLIDX[];
extern const fom_float FOM_MAT_t49_t_t47_VALS[];

#define FOM_MAT_t51_t_t47_ROWS 20
#define FOM_MAT_t51_t_t47_COLS 600
#define FOM_MAT_t51_t_t47_NNZ 600
extern const fom_int FOM_MAT_t51_t_t47_ROWPTR[];
extern const fom_int FOM_MAT_t51_t_t47_COLIDX[];
extern const fom_float FOM_MAT_t51_t_t47_VALS[];

#define FOM_MAT_t51_t_t50_ROWS 20
#define FOM_MAT_t51_t_t50_COLS 600
#define FOM_MAT_t51_t_t50_NNZ 600
extern const fom_int FOM_MAT_t51_t_t50_ROWPTR[];
extern const fom_int FOM_MAT_t51_t_t50_COLIDX[];
extern const fom_float FOM_MAT_t51_t_t50_VALS[];

#define FOM_MAT_t52_t_t47_ROWS 30
#define FOM_MAT_t52_t_t47_COLS 600
#define FOM_MAT_t52_t_t47_NNZ 600
extern const fom_int FOM_MAT_t52_t_t47_ROWPTR[];
extern const fom_int FOM_MAT_t52_t_t47_COLIDX[];
extern const fom_float FOM_MAT_t52_t_t47_VALS[];

#define FOM_MAT_t52_t_t50_ROWS 30
#define FOM_MAT_t52_t_t50_COLS 600
#define FOM_MAT_t52_t_t50_NNZ 600
extern const fom_int FOM_MAT_t52_t_t50_ROWPTR[];
extern const fom_int FOM_MAT_t52_t_t50_COLIDX[];
extern const fom_float FOM_MAT_t52_t_t50_VALS[];

#define FOM_MAT_t54_t_t50_ROWS 20
#define FOM_MAT_t54_t_t50_COLS 600
#define FOM_MAT_t54_t_t50_NNZ 600
extern const fom_int FOM_MAT_t54_t_t50_ROWPTR[];
extern const fom_int FOM_MAT_t54_t_t50_COLIDX[];
extern const fom_float FOM_MAT_t54_t_t50_VALS[];

#define FOM_MAT_t54_t_t53_ROWS 20
#define FOM_MAT_t54_t_t53_COLS 600
#define FOM_MAT_t54_t_t53_NNZ 600
extern const fom_int FOM_MAT_t54_t_t53_ROWPTR[];
extern const fom_int FOM_MAT_t54_t_t53_COLIDX[];
extern const fom_float FOM_MAT_t54_t_t53_VALS[];

#define FOM_MAT_t55_t_t50_ROWS 30
#define FOM_MAT_t55_t_t50_COLS 600
#define FOM_MAT_t55_t_t50_NNZ 600
extern const fom_int FOM_MAT_t55_t_t50_ROWPTR[];
extern const fom_int FOM_MAT_t55_t_t50_COLIDX[];
extern const fom_float FOM_MAT_t55_t_t50_VALS[];

#define FOM_MAT_t55_t_t53_ROWS 30
#define FOM_MAT_t55_t_t53_COLS 600
#define FOM_MAT_t55_t_t53_NNZ 600
extern const fom_int FOM_MAT_t55_t_t53_ROWPTR[];
extern const fom_int FOM_MAT_t55_t_t53_COLIDX[];
extern const fom_float FOM_MAT_t55_t_t53_VALS[];

#define FOM_MAT_t57_t_t53_ROWS 20
#define FOM_MAT_t57_t_t53_COLS 600
#define FOM_MAT_t57_t_t53_NNZ 600
extern const fom_int FOM_MAT_t57_t_t53_ROWPTR[];
extern const fom_int FOM_MAT_t57_t_t53_COLIDX[];
extern const fom_float FOM_MAT_t57_t_t53_VALS[];

#define FOM_MAT_t57_t_t56_ROWS 20
#define FOM_MAT_t57_t_t56_COLS 600
#define FOM_MAT_t57_t_t56_NNZ 600
extern const fom_int FOM_MAT_t57_t_t56_ROWPTR[];
extern const fom_int FOM_MAT_t57_t_t56_COLIDX[];
extern const fom_float FOM_MAT_t57_t_t56_VALS[];

#define FOM_MAT_t58_t_t53_ROWS 30
#define FOM_MAT_t58_t_t53_COLS 600
#define FOM_MAT_t58_t_t53_NNZ 600
extern const fom_int FOM_MAT_t58_t_t53_ROWPTR[];
extern const fom_int FOM_MAT_t58_t_t53_COLIDX[];
extern const fom_float FOM_MAT_t58_t_t53_VALS[];

#define FOM_MAT_t58_t_t56_ROWS 30
#define FOM_MAT_t58_t_t56_COLS 600
#define FOM_MAT_t58_t_t56_NNZ 600
extern const fom_int FOM_MAT_t58_t_t56_ROWPTR[];
extern const fom_int FOM_MAT_t58_t_t56_COLIDX[];
extern const fom_float FOM_MAT_t58_t_t56_VALS[];

#define FOM_MAT_t60_t_t56_ROWS 20
#define FOM_MAT_t60_t_t56_COLS 600
#define FOM_MAT_t60_t_t56_NNZ 600
extern const fom_int FOM_MAT_t60_t_t56_ROWPTR[];
extern const fom_int FOM_MAT_t60_t_t56_COLIDX[];
extern const fom_float FOM_MAT_t60_t_t56_VALS[];

#define FOM_MAT_t60_t_t59_ROWS 20
#define FOM_MAT_t60_t_t59_COLS 600
#define FOM_MAT_t60_t_t59_NNZ 600
extern const fom_int FOM_MAT_t60_t_t59_ROWPTR[];
extern const fom_int FOM_MAT_t60_t_t59_COLIDX[];
extern const fom_float FOM_MAT_t60_t_t59_VALS[];

#define FOM_MAT_t61_t_t56_ROWS 30
#define FOM_MAT_t61_t_t56_COLS 600
#define FOM_MAT_t61_t_t56_NNZ 600
extern const fom_int FOM_MAT_t61_t_t56_ROWPTR[];
extern const fom_int FOM_MAT_t61_t_t56_COLIDX[];
extern const fom_float FOM_MAT_t61_t_t56_VALS[];

#define FOM_MAT_t61_t_t59_ROWS 30
#define FOM_MAT_t61_t_t59_COLS 600
#define FOM_MAT_t61_t_t59_NNZ 600
extern const fom_int FOM_MAT_t61_t_t59_ROWPTR[];
extern const fom_int FOM_MAT_t61_t_t59_COLIDX[];
extern const fom_float FOM_MAT_t61_t_t59_VALS[];

#define FOM_MAT_t63_t_t59_ROWS 20
#define FOM_MAT_t63_t_t59_COLS 600
#define FOM_MAT_t63_t_t59_NNZ 600
extern const fom_int FOM_MAT_t63_t_t59_ROWPTR[];
extern const fom_int FOM_MAT_t63_t_t59_COLIDX[];
extern const fom_float FOM_MAT_t63_t_t59_VALS[];

#define FOM_MAT_t63_t_t62_ROWS 20
#define FOM_MAT_t63_t_t62_COLS 600
#define FOM_MAT_t63_t_t62_NNZ 600
extern const fom_int FOM_MAT_t63_t_t62_ROWPTR[];
extern const fom_int FOM_MAT_t63_t_t62_COLIDX[];
extern const fom_float FOM_MAT_t63_t_t62_VALS[];

#define FOM_MAT_t64_t_t59_ROWS 30
#define FOM_MAT_t64_t_t59_COLS 600
#define FOM_MAT_t64_t_t59_NNZ 600
extern const fom_int FOM_MAT_t64_t_t59_ROWPTR[];
extern const fom_int FOM_MAT_t64_t_t59_COLIDX[];
extern const fom_float FOM_MAT_t64_t_t59_VALS[];

#define FOM_MAT_t64_t_t62_ROWS 30
#define FOM_MAT_t64_t_t62_COLS 600
#define FOM_MAT_t64_t_t62_NNZ 600
extern const fom_int FOM_MAT_t64_t_t62_ROWPTR[];
extern const fom_int FOM_MAT_t64_t_t62_COLIDX[];
extern const fom_float FOM_MAT_t64_t_t62_VALS[];

#define FOM_MAT_t66_t_t62_ROWS 20
#define FOM_MAT_t66_t_t62_COLS 600
#define FOM_MAT_t66_t_t62_NNZ 600
extern const fom_int FOM_MAT_t66_t_t62_ROWPTR[];
extern const fom_int FOM_MAT_t66_t_t62_COLIDX[];
extern const fom_float FOM_MAT_t66_t_t62_VALS[];

#define FOM_MAT_t66_t_t65_ROWS 20
#define FOM_MAT_t66_t_t65_COLS 600
#define FOM_MAT_t66_t_t65_NNZ 600
extern const fom_int FOM_MAT_t66_t_t65_ROWPTR[];
extern const fom_int FOM_MAT_t66_t_t65_COLIDX[];
extern const fom_float FOM_MAT_t66_t_t65_VALS[];

#define FOM_MAT_t67_t_t62_ROWS 30
#define FOM_MAT_t67_t_t62_COLS 600
#define FOM_MAT_t67_t_t62_NNZ 600
extern const fom_int FOM_MAT_t67_t_t62_ROWPTR[];
extern const fom_int FOM_MAT_t67_t_t62_COLIDX[];
extern const fom_float FOM_MAT_t67_t_t62_VALS[];

#define FOM_MAT_t67_t_t65_ROWS 30
#define FOM_MAT_t67_t_t65_COLS 600
#define FOM_MAT_t67_t_t65_NNZ 600
extern const fom_int FOM_MAT_t67_t_t65_ROWPTR[];
extern const fom_int FOM_MAT_t67_t_t65_COLIDX[];
extern const fom_float FOM_MAT_t67_t_t65_VALS[];

#define FOM_MAT_t69_t_t65_ROWS 20
#define FOM_MAT_t69_t_t65_COLS 600
#define FOM_MAT_t69_t_t65_NNZ 600
extern const fom_int FOM_MAT_t69_t_t65_ROWPTR[];
extern const fom_int FOM_MAT_t69_t_t65_COLIDX[];
extern const fom_float FOM_MAT_t69_t_t65_VALS[];

#define FOM_MAT_t69_t_t68_ROWS 20
#define FOM_MAT_t69_t_t68_COLS 600
#define FOM_MAT_t69_t_t68_NNZ 600
extern const fom_int FOM_MAT_t69_t_t68_ROWPTR[];
extern const fom_int FOM_MAT_t69_t_t68_COLIDX[];
extern const fom_float FOM_MAT_t69_t_t68_VALS[];

#define FOM_MAT_t70_t_t65_ROWS 30
#define FOM_MAT_t70_t_t65_COLS 600
#define FOM_MAT_t70_t_t65_NNZ 600
extern const fom_int FOM_MAT_t70_t_t65_ROWPTR[];
extern const fom_int FOM_MAT_t70_t_t65_COLIDX[];
extern const fom_float FOM_MAT_t70_t_t65_VALS[];

#define FOM_MAT_t70_t_t68_ROWS 30
#define FOM_MAT_t70_t_t68_COLS 600
#define FOM_MAT_t70_t_t68_NNZ 600
extern const fom_int FOM_MAT_t70_t_t68_ROWPTR[];
extern const fom_int FOM_MAT_t70_t_t68_COLIDX[];
extern const fom_float FOM_MAT_t70_t_t68_VALS[];

#define FOM_MAT_t72_t_t68_ROWS 20
#define FOM_MAT_t72_t_t68_COLS 600
#define FOM_MAT_t72_t_t68_NNZ 600
extern const fom_int FOM_MAT_t72_t_t68_ROWPTR[];
extern const fom_int FOM_MAT_t72_t_t68_COLIDX[];
extern const fom_float FOM_MAT_t72_t_t68_VALS[];

#define FOM_MAT_t72_t_t71_ROWS 20
#define FOM_MAT_t72_t_t71_COLS 600
#define FOM_MAT_t72_t_t71_NNZ 600
extern const fom_int FOM_MAT_t72_t_t71_ROWPTR[];
extern const fom_int FOM_MAT_t72_t_t71_COLIDX[];
extern const fom_float FOM_MAT_t72_t_t71_VALS[];

#define FOM_MAT_t73_t_t68_ROWS 30
#define FOM_MAT_t73_t_t68_COLS 600
#define FOM_MAT_t73_t_t68_NNZ 600
extern const fom_int FOM_MAT_t73_t_t68_ROWPTR[];
extern const fom_int FOM_MAT_t73_t_t68_COLIDX[];
extern const fom_float FOM_MAT_t73_t_t68_VALS[];

#define FOM_MAT_t73_t_t71_ROWS 30
#define FOM_MAT_t73_t_t71_COLS 600
#define FOM_MAT_t73_t_t71_NNZ 600
extern const fom_int FOM_MAT_t73_t_t71_ROWPTR[];
extern const fom_int FOM_MAT_t73_t_t71_COLIDX[];
extern const fom_float FOM_MAT_t73_t_t71_VALS[];

#define FOM_MAT_t75_t_t71_ROWS 20
#define FOM_MAT_t75_t_t71_COLS 600
#define FOM_MAT_t75_t_t71_NNZ 600
extern const fom_int FOM_MAT_t75_t_t71_ROWPTR[];
extern const fom_int FOM_MAT_t75_t_t71_COLIDX[];
extern const fom_float FOM_MAT_t75_t_t71_VALS[];

#define FOM_MAT_t75_t_t74_ROWS 20
#define FOM_MAT_t75_t_t74_COLS 600
#define FOM_MAT_t75_t_t74_NNZ 600
extern const fom_int FOM_MAT_t75_t_t74_ROWPTR[];
extern const fom_int FOM_MAT_t75_t_t74_COLIDX[];
extern const fom_float FOM_MAT_t75_t_t74_VALS[];

#define FOM_MAT_t76_t_t71_ROWS 30
#define FOM_MAT_t76_t_t71_COLS 600
#define FOM_MAT_t76_t_t71_NNZ 600
extern const fom_int FOM_MAT_t76_t_t71_ROWPTR[];
extern const fom_int FOM_MAT_t76_t_t71_COLIDX[];
extern const fom_float FOM_MAT_t76_t_t71_VALS[];

#define FOM_MAT_t76_t_t74_ROWS 30
#define FOM_MAT_t76_t_t74_COLS 600
#define FOM_MAT_t76_t_t74_NNZ 600
extern const fom_int FOM_MAT_t76_t_t74_ROWPTR[];
extern const fom_int FOM_MAT_t76_t_t74_COLIDX[];
extern const fom_float FOM_MAT_t76_t_t74_VALS[];

#define FOM_MAT_t78_t_t74_ROWS 20
#define FOM_MAT_t78_t_t74_COLS 600
#define FOM_MAT_t78_t_t74_NNZ 600
extern const fom_int FOM_MAT_t78_t_t74_ROWPTR[];
extern const fom_int FOM_MAT_t78_t_t74_COLIDX[];
extern const fom_float FOM_MAT_t78_t_t74_VALS[];

#define FOM_MAT_t78_t_t77_ROWS 20
#define FOM_MAT_t78_t_t77_COLS 600
#define FOM_MAT_t78_t_t77_NNZ 600
extern const fom_int FOM_MAT_t78_t_t77_ROWPTR[];
extern const fom_int FOM_MAT_t78_t_t77_COLIDX[];
extern const fom_float FOM_MAT_t78_t_t77_VALS[];

#define FOM_MAT_t79_t_t74_ROWS 30
#define FOM_MAT_t79_t_t74_COLS 600
#define FOM_MAT_t79_t_t74_NNZ 600
extern const fom_int FOM_MAT_t79_t_t74_ROWPTR[];
extern const fom_int FOM_MAT_t79_t_t74_COLIDX[];
extern const fom_float FOM_MAT_t79_t_t74_VALS[];

#define FOM_MAT_t79_t_t77_ROWS 30
#define FOM_MAT_t79_t_t77_COLS 600
#define FOM_MAT_t79_t_t77_NNZ 600
extern const fom_int FOM_MAT_t79_t_t77_ROWPTR[];
extern const fom_int FOM_MAT_t79_t_t77_COLIDX[];
extern const fom_float FOM_MAT_t79_t_t77_VALS[];

#define FOM_MAT_t81_t_t77_ROWS 20
#define FOM_MAT_t81_t_t77_COLS 600
#define FOM_MAT_t81_t_t77_NNZ 600
extern const fom_int FOM_MAT_t81_t_t77_ROWPTR[];
extern const fom_int FOM_MAT_t81_t_t77_COLIDX[];
extern const fom_float FOM_MAT_t81_t_t77_VALS[];

#define FOM_MAT_t81_t_t80_ROWS 20
#define FOM_MAT_t81_t_t80_COLS 600
#define FOM_MAT_t81_t_t80_NNZ 600
extern const fom_int FOM_MAT_t81_t_t80_ROWPTR[];
extern const fom_int FOM_MAT_t81_t_t80_COLIDX[];
extern const fom_float FOM_MAT_t81_t_t80_VALS[];

#define FOM_MAT_t82_t_t77_ROWS 30
#define FOM_MAT_t82_t_t77_COLS 600
#define FOM_MAT_t82_t_t77_NNZ 600
extern const fom_int FOM_MAT_t82_t_t77_ROWPTR[];
extern const fom_int FOM_MAT_t82_t_t77_COLIDX[];
extern const fom_float FOM_MAT_t82_t_t77_VALS[];

#define FOM_MAT_t82_t_t80_ROWS 30
#define FOM_MAT_t82_t_t80_COLS 600
#define FOM_MAT_t82_t_t80_NNZ 600
extern const fom_int FOM_MAT_t82_t_t80_ROWPTR[];
extern const fom_int FOM_MAT_t82_t_t80_COLIDX[];
extern const fom_float FOM_MAT_t82_t_t80_VALS[];

#define FOM_MAT_t84_t_t80_ROWS 20
#define FOM_MAT_t84_t_t80_COLS 600
#define FOM_MAT_t84_t_t80_NNZ 600
extern const fom_int FOM_MAT_t84_t_t80_ROWPTR[];
extern const fom_int FOM_MAT_t84_t_t80_COLIDX[];
extern const fom_float FOM_MAT_t84_t_t80_VALS[];

#define FOM_MAT_t84_t_t83_ROWS 20
#define FOM_MAT_t84_t_t83_COLS 600
#define FOM_MAT_t84_t_t83_NNZ 600
extern const fom_int FOM_MAT_t84_t_t83_ROWPTR[];
extern const fom_int FOM_MAT_t84_t_t83_COLIDX[];
extern const fom_float FOM_MAT_t84_t_t83_VALS[];

#define FOM_MAT_t85_t_t80_ROWS 30
#define FOM_MAT_t85_t_t80_COLS 600
#define FOM_MAT_t85_t_t80_NNZ 600
extern const fom_int FOM_MAT_t85_t_t80_ROWPTR[];
extern const fom_int FOM_MAT_t85_t_t80_COLIDX[];
extern const fom_float FOM_MAT_t85_t_t80_VALS[];

#define FOM_MAT_t85_t_t83_ROWS 30
#define FOM_MAT_t85_t_t83_COLS 600
#define FOM_MAT_t85_t_t83_NNZ 600
extern const fom_int FOM_MAT_t85_t_t83_ROWPTR[];
extern const fom_int FOM_MAT_t85_t_t83_COLIDX[];
extern const fom_float FOM_MAT_t85_t_t83_VALS[];

#define FOM_MAT_t87_t_t83_ROWS 20
#define FOM_MAT_t87_t_t83_COLS 600
#define FOM_MAT_t87_t_t83_NNZ 600
extern const fom_int FOM_MAT_t87_t_t83_ROWPTR[];
extern const fom_int FOM_MAT_t87_t_t83_COLIDX[];
extern const fom_float FOM_MAT_t87_t_t83_VALS[];

#define FOM_MAT_t87_t_t86_ROWS 20
#define FOM_MAT_t87_t_t86_COLS 600
#define FOM_MAT_t87_t_t86_NNZ 600
extern const fom_int FOM_MAT_t87_t_t86_ROWPTR[];
extern const fom_int FOM_MAT_t87_t_t86_COLIDX[];
extern const fom_float FOM_MAT_t87_t_t86_VALS[];

#define FOM_MAT_t88_t_t83_ROWS 30
#define FOM_MAT_t88_t_t83_COLS 600
#define FOM_MAT_t88_t_t83_NNZ 600
extern const fom_int FOM_MAT_t88_t_t83_ROWPTR[];
extern const fom_int FOM_MAT_t88_t_t83_COLIDX[];
extern const fom_float FOM_MAT_t88_t_t83_VALS[];

#define FOM_MAT_t88_t_t86_ROWS 30
#define FOM_MAT_t88_t_t86_COLS 600
#define FOM_MAT_t88_t_t86_NNZ 600
extern const fom_int FOM_MAT_t88_t_t86_ROWPTR[];
extern const fom_int FOM_MAT_t88_t_t86_COLIDX[];
extern const fom_float FOM_MAT_t88_t_t86_VALS[];

#define FOM_MAT_t90_t_t86_ROWS 20
#define FOM_MAT_t90_t_t86_COLS 600
#define FOM_MAT_t90_t_t86_NNZ 600
extern const fom_int FOM_MAT_t90_t_t86_ROWPTR[];
extern const fom_int FOM_MAT_t90_t_t86_COLIDX[];
extern const fom_float FOM_MAT_t90_t_t86_VALS[];

#define FOM_MAT_t90_t_t89_ROWS 20
#define FOM_MAT_t90_t_t89_COLS 600
#define FOM_MAT_t90_t_t89_NNZ 600
extern const fom_int FOM_MAT_t90_t_t89_ROWPTR[];
extern const fom_int FOM_MAT_t90_t_t89_COLIDX[];
extern const fom_float FOM_MAT_t90_t_t89_VALS[];

#define FOM_MAT_t91_t_t86_ROWS 30
#define FOM_MAT_t91_t_t86_COLS 600
#define FOM_MAT_t91_t_t86_NNZ 600
extern const fom_int FOM_MAT_t91_t_t86_ROWPTR[];
extern const fom_int FOM_MAT_t91_t_t86_COLIDX[];
extern const fom_float FOM_MAT_t91_t_t86_VALS[];

#define FOM_MAT_t91_t_t89_ROWS 30
#define FOM_MAT_t91_t_t89_COLS 600
#define FOM_MAT_t91_t_t89_NNZ 600
extern const fom_int FOM_MAT_t91_t_t89_ROWPTR[];
extern const fom_int FOM_MAT_t91_t_t89_COLIDX[];
extern const fom_float FOM_MAT_t91_t_t89_VALS[];

#define FOM_MAT_t93_t_t89_ROWS 20
#define FOM_MAT_t93_t_t89_COLS 600
#define FOM_MAT_t93_t_t89_NNZ 600
extern const fom_int FOM_MAT_t93_t_t89_ROWPTR[];
extern const fom_int FOM_MAT_t93_t_t89_COLIDX[];
extern const fom_float FOM_MAT_t93_t_t89_VALS[];

#define FOM_MAT_t93_t_t92_ROWS 20
#define FOM_MAT_t93_t_t92_COLS 600
#define FOM_MAT_t93_t_t92_NNZ 600
extern const fom_int FOM_MAT_t93_t_t92_ROWPTR[];
extern const fom_int FOM_MAT_t93_t_t92_COLIDX[];
extern const fom_float FOM_MAT_t93_t_t92_VALS[];

#define FOM_MAT_t94_t_t89_ROWS 30
#define FOM_MAT_t94_t_t89_COLS 600
#define FOM_MAT_t94_t_t89_NNZ 600
extern const fom_int FOM_MAT_t94_t_t89_ROWPTR[];
extern const fom_int FOM_MAT_t94_t_t89_COLIDX[];
extern const fom_float FOM_MAT_t94_t_t89_VALS[];

#define FOM_MAT_t94_t_t92_ROWS 30
#define FOM_MAT_t94_t_t92_COLS 600
#define FOM_MAT_t94_t_t92_NNZ 600
extern const fom_int FOM_MAT_t94_t_t92_ROWPTR[];
extern const fom_int FOM_MAT_t94_t_t92_COLIDX[];
extern const fom_float FOM_MAT_t94_t_t92_VALS[];

#define FOM_MAT_t96_t_t92_ROWS 20
#define FOM_MAT_t96_t_t92_COLS 600
#define FOM_MAT_t96_t_t92_NNZ 600
extern const fom_int FOM_MAT_t96_t_t92_ROWPTR[];
extern const fom_int FOM_MAT_t96_t_t92_COLIDX[];
extern const fom_float FOM_MAT_t96_t_t92_VALS[];

#define FOM_MAT_t96_t_t95_ROWS 20
#define FOM_MAT_t96_t_t95_COLS 600
#define FOM_MAT_t96_t_t95_NNZ 600
extern const fom_int FOM_MAT_t96_t_t95_ROWPTR[];
extern const fom_int FOM_MAT_t96_t_t95_COLIDX[];
extern const fom_float FOM_MAT_t96_t_t95_VALS[];

#define FOM_MAT_t97_t_t92_ROWS 30
#define FOM_MAT_t97_t_t92_COLS 600
#define FOM_MAT_t97_t_t92_NNZ 600
extern const fom_int FOM_MAT_t97_t_t92_ROWPTR[];
extern const fom_int FOM_MAT_t97_t_t92_COLIDX[];
extern const fom_float FOM_MAT_t97_t_t92_VALS[];

#define FOM_MAT_t97_t_t95_ROWS 30
#define FOM_MAT_t97_t_t95_COLS 600
#define FOM_MAT_t97_t_t95_NNZ 600
extern const fom_int FOM_MAT_t97_t_t95_ROWPTR[];
extern const fom_int FOM_MAT_t97_t_t95_COLIDX[];
extern const fom_float FOM_MAT_t97_t_t95_VALS[];

#define FOM_MAT_t99_t_t95_ROWS 20
#define FOM_MAT_t99_t_t95_COLS 600
#define FOM_MAT_t99_t_t95_NNZ 600
extern const fom_int FOM_MAT_t99_t_t95_ROWPTR[];
extern const fom_int FOM_MAT_t99_t_t95_COLIDX[];
extern const fom_float FOM_MAT_t99_t_t95_VALS[];

#define FOM_MAT_t99_t_t98_ROWS 20
#define FOM_MAT_t99_t_t98_COLS 600
#define FOM_MAT_t99_t_t98_NNZ 600
extern const fom_int FOM_MAT_t99_t_t98_ROWPTR[];
extern const fom_int FOM_MAT_t99_t_t98_COLIDX[];
extern const fom_float FOM_MAT_t99_t_t98_VALS[];

#define FOM_MAT_t100_t_t95_ROWS 30
#define FOM_MAT_t100_t_t95_COLS 600
#define FOM_MAT_t100_t_t95_NNZ 600
extern const fom_int FOM_MAT_t100_t_t95_ROWPTR[];
extern const fom_int FOM_MAT_t100_t_t95_COLIDX[];
extern const fom_float FOM_MAT_t100_t_t95_VALS[];

#define FOM_MAT_t100_t_t98_ROWS 30
#define FOM_MAT_t100_t_t98_COLS 600
#define FOM_MAT_t100_t_t98_NNZ 600
extern const fom_int FOM_MAT_t100_t_t98_ROWPTR[];
extern const fom_int FOM_MAT_t100_t_t98_COLIDX[];
extern const fom_float FOM_MAT_t100_t_t98_VALS[];

#define FOM_MAT_t102_t_t98_ROWS 20
#define FOM_MAT_t102_t_t98_COLS 600
#define FOM_MAT_t102_t_t98_NNZ 600
extern const fom_int FOM_MAT_t102_t_t98_ROWPTR[];
extern const fom_int FOM_MAT_t102_t_t98_COLIDX[];
extern const fom_float FOM_MAT_t102_t_t98_VALS[];

#define FOM_MAT_t102_t_t101_ROWS 20
#define FOM_MAT_t102_t_t101_COLS 600
#define FOM_MAT_t102_t_t101_NNZ 600
extern const fom_int FOM_MAT_t102_t_t101_ROWPTR[];
extern const fom_int FOM_MAT_t102_t_t101_COLIDX[];
extern const fom_float FOM_MAT_t102_t_t101_VALS[];

#define FOM_MAT_t103_t_t98_ROWS 30
#define FOM_MAT_t103_t_t98_COLS 600
#define FOM_MAT_t103_t_t98_NNZ 600
extern const fom_int FOM_MAT_t103_t_t98_ROWPTR[];
extern const fom_int FOM_MAT_t103_t_t98_COLIDX[];
extern const fom_float FOM_MAT_t103_t_t98_VALS[];

#define FOM_MAT_t103_t_t101_ROWS 30
#define FOM_MAT_t103_t_t101_COLS 600
#define FOM_MAT_t103_t_t101_NNZ 600
extern const fom_int FOM_MAT_t103_t_t101_ROWPTR[];
extern const fom_int FOM_MAT_t103_t_t101_COLIDX[];
extern const fom_float FOM_MAT_t103_t_t101_VALS[];

#define FOM_MAT_t105_t_t101_ROWS 20
#define FOM_MAT_t105_t_t101_COLS 600
#define FOM_MAT_t105_t_t101_NNZ 600
extern const fom_int FOM_MAT_t105_t_t101_ROWPTR[];
extern const fom_int FOM_MAT_t105_t_t101_COLIDX[];
extern const fom_float FOM_MAT_t105_t_t101_VALS[];

#define FOM_MAT_t105_t_t104_ROWS 20
#define FOM_MAT_t105_t_t104_COLS 600
#define FOM_MAT_t105_t_t104_NNZ 600
extern const fom_int FOM_MAT_t105_t_t104_ROWPTR[];
extern const fom_int FOM_MAT_t105_t_t104_COLIDX[];
extern const fom_float FOM_MAT_t105_t_t104_VALS[];

#define FOM_MAT_t106_t_t101_ROWS 30
#define FOM_MAT_t106_t_t101_COLS 600
#define FOM_MAT_t106_t_t101_NNZ 600
extern const fom_int FOM_MAT_t106_t_t101_ROWPTR[];
extern const fom_int FOM_MAT_t106_t_t101_COLIDX[];
extern const fom_float FOM_MAT_t106_t_t101_VALS[];

#define FOM_MAT_t106_t_t104_ROWS 30
#define FOM_MAT_t106_t_t104_COLS 600
#define FOM_MAT_t106_t_t104_NNZ 600
extern const fom_int FOM_MAT_t106_t_t104_ROWPTR[];
extern const fom_int FOM_MAT_t106_t_t104_COLIDX[];
extern const fom_float FOM_MAT_t106_t_t104_VALS[];

#define FOM_MAT_t108_t_t104_ROWS 20
#define FOM_MAT_t108_t_t104_COLS 600
#define FOM_MAT_t108_t_t104_NNZ 600
extern const fom_int FOM_MAT_t108_t_t104_ROWPTR[];
extern const fom_int FOM_MAT_t108_t_t104_COLIDX[];
extern const fom_float FOM_MAT_t108_t_t104_VALS[];

#define FOM_MAT_t108_t_t107_ROWS 20
#define FOM_MAT_t108_t_t107_COLS 600
#define FOM_MAT_t108_t_t107_NNZ 600
extern const fom_int FOM_MAT_t108_t_t107_ROWPTR[];
extern const fom_int FOM_MAT_t108_t_t107_COLIDX[];
extern const fom_float FOM_MAT_t108_t_t107_VALS[];

#define FOM_MAT_t109_t_t104_ROWS 30
#define FOM_MAT_t109_t_t104_COLS 600
#define FOM_MAT_t109_t_t104_NNZ 600
extern const fom_int FOM_MAT_t109_t_t104_ROWPTR[];
extern const fom_int FOM_MAT_t109_t_t104_COLIDX[];
extern const fom_float FOM_MAT_t109_t_t104_VALS[];

#define FOM_MAT_t109_t_t107_ROWS 30
#define FOM_MAT_t109_t_t107_COLS 600
#define FOM_MAT_t109_t_t107_NNZ 600
extern const fom_int FOM_MAT_t109_t_t107_ROWPTR[];
extern const fom_int FOM_MAT_t109_t_t107_COLIDX[];
extern const fom_float FOM_MAT_t109_t_t107_VALS[];

#define FOM_MAT_t111_t_t107_ROWS 20
#define FOM_MAT_t111_t_t107_COLS 600
#define FOM_MAT_t111_t_t107_NNZ 600
extern const fom_int FOM_MAT_t111_t_t107_ROWPTR[];
extern const fom_int FOM_MAT_t111_t_t107_COLIDX[];
extern const fom_float FOM_MAT_t111_t_t107_VALS[];

#define FOM_MAT_t111_t_t110_ROWS 20
#define FOM_MAT_t111_t_t110_COLS 600
#define FOM_MAT_t111_t_t110_NNZ 600
extern const fom_int FOM_MAT_t111_t_t110_ROWPTR[];
extern const fom_int FOM_MAT_t111_t_t110_COLIDX[];
extern const fom_float FOM_MAT_t111_t_t110_VALS[];

#define FOM_MAT_t112_t_t107_ROWS 30
#define FOM_MAT_t112_t_t107_COLS 600
#define FOM_MAT_t112_t_t107_NNZ 600
extern const fom_int FOM_MAT_t112_t_t107_ROWPTR[];
extern const fom_int FOM_MAT_t112_t_t107_COLIDX[];
extern const fom_float FOM_MAT_t112_t_t107_VALS[];

#define FOM_MAT_t112_t_t110_ROWS 30
#define FOM_MAT_t112_t_t110_COLS 600
#define FOM_MAT_t112_t_t110_NNZ 600
extern const fom_int FOM_MAT_t112_t_t110_ROWPTR[];
extern const fom_int FOM_MAT_t112_t_t110_COLIDX[];
extern const fom_float FOM_MAT_t112_t_t110_VALS[];

#define FOM_MAT_t114_t_t110_ROWS 20
#define FOM_MAT_t114_t_t110_COLS 600
#define FOM_MAT_t114_t_t110_NNZ 600
extern const fom_int FOM_MAT_t114_t_t110_ROWPTR[];
extern const fom_int FOM_MAT_t114_t_t110_COLIDX[];
extern const fom_float FOM_MAT_t114_t_t110_VALS[];

#define FOM_MAT_t114_t_t113_ROWS 20
#define FOM_MAT_t114_t_t113_COLS 600
#define FOM_MAT_t114_t_t113_NNZ 600
extern const fom_int FOM_MAT_t114_t_t113_ROWPTR[];
extern const fom_int FOM_MAT_t114_t_t113_COLIDX[];
extern const fom_float FOM_MAT_t114_t_t113_VALS[];

#define FOM_MAT_t115_t_t110_ROWS 30
#define FOM_MAT_t115_t_t110_COLS 600
#define FOM_MAT_t115_t_t110_NNZ 600
extern const fom_int FOM_MAT_t115_t_t110_ROWPTR[];
extern const fom_int FOM_MAT_t115_t_t110_COLIDX[];
extern const fom_float FOM_MAT_t115_t_t110_VALS[];

#define FOM_MAT_t115_t_t113_ROWS 30
#define FOM_MAT_t115_t_t113_COLS 600
#define FOM_MAT_t115_t_t113_NNZ 600
extern const fom_int FOM_MAT_t115_t_t113_ROWPTR[];
extern const fom_int FOM_MAT_t115_t_t113_COLIDX[];
extern const fom_float FOM_MAT_t115_t_t113_VALS[];

#define FOM_MAT_t117_t_t113_ROWS 20
#define FOM_MAT_t117_t_t113_COLS 600
#define FOM_MAT_t117_t_t113_NNZ 600
extern const fom_int FOM_MAT_t117_t_t113_ROWPTR[];
extern const fom_int FOM_MAT_t117_t_t113_COLIDX[];
extern const fom_float FOM_MAT_t117_t_t113_VALS[];

#define FOM_MAT_t117_t_t116_ROWS 20
#define FOM_MAT_t117_t_t116_COLS 600
#define FOM_MAT_t117_t_t116_NNZ 600
extern const fom_int FOM_MAT_t117_t_t116_ROWPTR[];
extern const fom_int FOM_MAT_t117_t_t116_COLIDX[];
extern const fom_float FOM_MAT_t117_t_t116_VALS[];

#define FOM_MAT_t118_t_t113_ROWS 30
#define FOM_MAT_t118_t_t113_COLS 600
#define FOM_MAT_t118_t_t113_NNZ 600
extern const fom_int FOM_MAT_t118_t_t113_ROWPTR[];
extern const fom_int FOM_MAT_t118_t_t113_COLIDX[];
extern const fom_float FOM_MAT_t118_t_t113_VALS[];

#define FOM_MAT_t118_t_t116_ROWS 30
#define FOM_MAT_t118_t_t116_COLS 600
#define FOM_MAT_t118_t_t116_NNZ 600
extern const fom_int FOM_MAT_t118_t_t116_ROWPTR[];
extern const fom_int FOM_MAT_t118_t_t116_COLIDX[];
extern const fom_float FOM_MAT_t118_t_t116_VALS[];

#define FOM_MAT_t120_t_t116_ROWS 20
#define FOM_MAT_t120_t_t116_COLS 600
#define FOM_MAT_t120_t_t116_NNZ 600
extern const fom_int FOM_MAT_t120_t_t116_ROWPTR[];
extern const fom_int FOM_MAT_t120_t_t116_COLIDX[];
extern const fom_float FOM_MAT_t120_t_t116_VALS[];

#define FOM_MAT_t120_t_t119_ROWS 20
#define FOM_MAT_t120_t_t119_COLS 600
#define FOM_MAT_t120_t_t119_NNZ 600
extern const fom_int FOM_MAT_t120_t_t119_ROWPTR[];
extern const fom_int FOM_MAT_t120_t_t119_COLIDX[];
extern const fom_float FOM_MAT_t120_t_t119_VALS[];

#define FOM_MAT_t121_t_t116_ROWS 30
#define FOM_MAT_t121_t_t116_COLS 600
#define FOM_MAT_t121_t_t116_NNZ 600
extern const fom_int FOM_MAT_t121_t_t116_ROWPTR[];
extern const fom_int FOM_MAT_t121_t_t116_COLIDX[];
extern const fom_float FOM_MAT_t121_t_t116_VALS[];

#define FOM_MAT_t121_t_t119_ROWS 30
#define FOM_MAT_t121_t_t119_COLS 600
#define FOM_MAT_t121_t_t119_NNZ 600
extern const fom_int FOM_MAT_t121_t_t119_ROWPTR[];
extern const fom_int FOM_MAT_t121_t_t119_COLIDX[];
extern const fom_float FOM_MAT_t121_t_t119_VALS[];

#define FOM_MAT_t123_t_t119_ROWS 20
#define FOM_MAT_t123_t_t119_COLS 600
#define FOM_MAT_t123_t_t119_NNZ 600
extern const fom_int FOM_MAT_t123_t_t119_ROWPTR[];
extern const fom_int FOM_MAT_t123_t_t119_COLIDX[];
extern const fom_float FOM_MAT_t123_t_t119_VALS[];

#define FOM_MAT_t123_t_t122_ROWS 20
#define FOM_MAT_t123_t_t122_COLS 600
#define FOM_MAT_t123_t_t122_NNZ 600
extern const fom_int FOM_MAT_t123_t_t122_ROWPTR[];
extern const fom_int FOM_MAT_t123_t_t122_COLIDX[];
extern const fom_float FOM_MAT_t123_t_t122_VALS[];

#define FOM_MAT_t124_t_t119_ROWS 30
#define FOM_MAT_t124_t_t119_COLS 600
#define FOM_MAT_t124_t_t119_NNZ 600
extern const fom_int FOM_MAT_t124_t_t119_ROWPTR[];
extern const fom_int FOM_MAT_t124_t_t119_COLIDX[];
extern const fom_float FOM_MAT_t124_t_t119_VALS[];

#define FOM_MAT_t124_t_t122_ROWS 30
#define FOM_MAT_t124_t_t122_COLS 600
#define FOM_MAT_t124_t_t122_NNZ 600
extern const fom_int FOM_MAT_t124_t_t122_ROWPTR[];
extern const fom_int FOM_MAT_t124_t_t122_COLIDX[];
extern const fom_float FOM_MAT_t124_t_t122_VALS[];

#define FOM_MAT_t126_t_t122_ROWS 20
#define FOM_MAT_t126_t_t122_COLS 600
#define FOM_MAT_t126_t_t122_NNZ 600
extern const fom_int FOM_MAT_t126_t_t122_ROWPTR[];
extern const fom_int FOM_MAT_t126_t_t122_COLIDX[];
extern const fom_float FOM_MAT_t126_t_t122_VALS[];

#define FOM_MAT_t126_t_t125_ROWS 20
#define FOM_MAT_t126_t_t125_COLS 600
#define FOM_MAT_t126_t_t125_NNZ 600
extern const fom_int FOM_MAT_t126_t_t125_ROWPTR[];
extern const fom_int FOM_MAT_t126_t_t125_COLIDX[];
extern const fom_float FOM_MAT_t126_t_t125_VALS[];

#define FOM_MAT_t127_t_t122_ROWS 30
#define FOM_MAT_t127_t_t122_COLS 600
#define FOM_MAT_t127_t_t122_NNZ 600
extern const fom_int FOM_MAT_t127_t_t122_ROWPTR[];
extern const fom_int FOM_MAT_t127_t_t122_COLIDX[];
extern const fom_float FOM_MAT_t127_t_t122_VALS[];

#define FOM_MAT_t127_t_t125_ROWS 30
#define FOM_MAT_t127_t_t125_COLS 600
#define FOM_MAT_t127_t_t125_NNZ 600
extern const fom_int FOM_MAT_t127_t_t125_ROWPTR[];
extern const fom_int FOM_MAT_t127_t_t125_COLIDX[];
extern const fom_float FOM_MAT_t127_t_t125_VALS[];

#define FOM_MAT_t129_t_t125_ROWS 20
#define FOM_MAT_t129_t_t125_COLS 600
#define FOM_MAT_t129_t_t125_NNZ 600
extern const fom_int FOM_MAT_t129_t_t125_ROWPTR[];
extern const fom_int FOM_MAT_t129_t_t125_COLIDX[];
extern const fom_float FOM_MAT_t129_t_t125_VALS[];

#define FOM_MAT_t129_t_t128_ROWS 20
#define FOM_MAT_t129_t_t128_COLS 600
#define FOM_MAT_t129_t_t128_NNZ 600
extern const fom_int FOM_MAT_t129_t_t128_ROWPTR[];
extern const fom_int FOM_MAT_t129_t_t128_COLIDX[];
extern const fom_float FOM_MAT_t129_t_t128_VALS[];

#define FOM_MAT_t130_t_t125_ROWS 30
#define FOM_MAT_t130_t_t125_COLS 600
#define FOM_MAT_t130_t_t125_NNZ 600
extern const fom_int FOM_MAT_t130_t_t125_ROWPTR[];
extern const fom_int FOM_MAT_t130_t_t125_COLIDX[];
extern const fom_float FOM_MAT_t130_t_t125_VALS[];

#define FOM_MAT_t130_t_t128_ROWS 30
#define FOM_MAT_t130_t_t128_COLS 600
#define FOM_MAT_t130_t_t128_NNZ 600
extern const fom_int FOM_MAT_t130_t_t128_ROWPTR[];
extern const fom_int FOM_MAT_t130_t_t128_COLIDX[];
extern const fom_float FOM_MAT_t130_t_t128_VALS[];

#define FOM_MAT_t132_t_t128_ROWS 20
#define FOM_MAT_t132_t_t128_COLS 600
#define FOM_MAT_t132_t_t128_NNZ 600
extern const fom_int FOM_MAT_t132_t_t128_ROWPTR[];
extern const fom_int FOM_MAT_t132_t_t128_COLIDX[];
extern const fom_float FOM_MAT_t132_t_t128_VALS[];

#define FOM_MAT_t132_t_t131_ROWS 20
#define FOM_MAT_t132_t_t131_COLS 600
#define FOM_MAT_t132_t_t131_NNZ 600
extern const fom_int FOM_MAT_t132_t_t131_ROWPTR[];
extern const fom_int FOM_MAT_t132_t_t131_COLIDX[];
extern const fom_float FOM_MAT_t132_t_t131_VALS[];

#define FOM_MAT_t133_t_t128_ROWS 30
#define FOM_MAT_t133_t_t128_COLS 600
#define FOM_MAT_t133_t_t128_NNZ 600
extern const fom_int FOM_MAT_t133_t_t128_ROWPTR[];
extern const fom_int FOM_MAT_t133_t_t128_COLIDX[];
extern const fom_float FOM_MAT_t133_t_t128_VALS[];

#define FOM_MAT_t133_t_t131_ROWS 30
#define FOM_MAT_t133_t_t131_COLS 600
#define FOM_MAT_t133_t_t131_NNZ 600
extern const fom_int FOM_MAT_t133_t_t131_ROWPTR[];
extern const fom_int FOM_MAT_t133_t_t131_COLIDX[];
extern const fom_float FOM_MAT_t133_t_t131_VALS[];

#define FOM_MAT_t135_t_t131_ROWS 20
#define FOM_MAT_t135_t_t131_COLS 600
#define FOM_MAT_t135_t_t131_NNZ 600
extern const fom_int FOM_MAT_t135_t_t131_ROWPTR[];
extern const fom_int FOM_MAT_t135_t_t131_COLIDX[];
extern const fom_float FOM_MAT_t135_t_t131_VALS[];

#define FOM_MAT_t135_t_t134_ROWS 20
#define FOM_MAT_t135_t_t134_COLS 600
#define FOM_MAT_t135_t_t134_NNZ 600
extern const fom_int FOM_MAT_t135_t_t134_ROWPTR[];
extern const fom_int FOM_MAT_t135_t_t134_COLIDX[];
extern const fom_float FOM_MAT_t135_t_t134_VALS[];

#define FOM_MAT_t136_t_t131_ROWS 30
#define FOM_MAT_t136_t_t131_COLS 600
#define FOM_MAT_t136_t_t131_NNZ 600
extern const fom_int FOM_MAT_t136_t_t131_ROWPTR[];
extern const fom_int FOM_MAT_t136_t_t131_COLIDX[];
extern const fom_float FOM_MAT_t136_t_t131_VALS[];

#define FOM_MAT_t136_t_t134_ROWS 30
#define FOM_MAT_t136_t_t134_COLS 600
#define FOM_MAT_t136_t_t134_NNZ 600
extern const fom_int FOM_MAT_t136_t_t134_ROWPTR[];
extern const fom_int FOM_MAT_t136_t_t134_COLIDX[];
extern const fom_float FOM_MAT_t136_t_t134_VALS[];

#define FOM_MAT_t138_t_t134_ROWS 20
#define FOM_MAT_t138_t_t134_COLS 600
#define FOM_MAT_t138_t_t134_NNZ 600
extern const fom_int FOM_MAT_t138_t_t134_ROWPTR[];
extern const fom_int FOM_MAT_t138_t_t134_COLIDX[];
extern const fom_float FOM_MAT_t138_t_t134_VALS[];

#define FOM_MAT_t138_t_t137_ROWS 20
#define FOM_MAT_t138_t_t137_COLS 600
#define FOM_MAT_t138_t_t137_NNZ 600
extern const fom_int FOM_MAT_t138_t_t137_ROWPTR[];
extern const fom_int FOM_MAT_t138_t_t137_COLIDX[];
extern const fom_float FOM_MAT_t138_t_t137_VALS[];

#define FOM_MAT_t139_t_t134_ROWS 30
#define FOM_MAT_t139_t_t134_COLS 600
#define FOM_MAT_t139_t_t134_NNZ 600
extern const fom_int FOM_MAT_t139_t_t134_ROWPTR[];
extern const fom_int FOM_MAT_t139_t_t134_COLIDX[];
extern const fom_float FOM_MAT_t139_t_t134_VALS[];

#define FOM_MAT_t139_t_t137_ROWS 30
#define FOM_MAT_t139_t_t137_COLS 600
#define FOM_MAT_t139_t_t137_NNZ 600
extern const fom_int FOM_MAT_t139_t_t137_ROWPTR[];
extern const fom_int FOM_MAT_t139_t_t137_COLIDX[];
extern const fom_float FOM_MAT_t139_t_t137_VALS[];

#define FOM_MAT_t141_t_t137_ROWS 20
#define FOM_MAT_t141_t_t137_COLS 600
#define FOM_MAT_t141_t_t137_NNZ 600
extern const fom_int FOM_MAT_t141_t_t137_ROWPTR[];
extern const fom_int FOM_MAT_t141_t_t137_COLIDX[];
extern const fom_float FOM_MAT_t141_t_t137_VALS[];

#define FOM_MAT_t141_t_t140_ROWS 20
#define FOM_MAT_t141_t_t140_COLS 600
#define FOM_MAT_t141_t_t140_NNZ 600
extern const fom_int FOM_MAT_t141_t_t140_ROWPTR[];
extern const fom_int FOM_MAT_t141_t_t140_COLIDX[];
extern const fom_float FOM_MAT_t141_t_t140_VALS[];

#define FOM_MAT_t142_t_t137_ROWS 30
#define FOM_MAT_t142_t_t137_COLS 600
#define FOM_MAT_t142_t_t137_NNZ 600
extern const fom_int FOM_MAT_t142_t_t137_ROWPTR[];
extern const fom_int FOM_MAT_t142_t_t137_COLIDX[];
extern const fom_float FOM_MAT_t142_t_t137_VALS[];

#define FOM_MAT_t142_t_t140_ROWS 30
#define FOM_MAT_t142_t_t140_COLS 600
#define FOM_MAT_t142_t_t140_NNZ 600
extern const fom_int FOM_MAT_t142_t_t140_ROWPTR[];
extern const fom_int FOM_MAT_t142_t_t140_COLIDX[];
extern const fom_float FOM_MAT_t142_t_t140_VALS[];

#define FOM_MAT_t144_t_t140_ROWS 20
#define FOM_MAT_t144_t_t140_COLS 600
#define FOM_MAT_t144_t_t140_NNZ 600
extern const fom_int FOM_MAT_t144_t_t140_ROWPTR[];
extern const fom_int FOM_MAT_t144_t_t140_COLIDX[];
extern const fom_float FOM_MAT_t144_t_t140_VALS[];

#define FOM_MAT_t144_t_t143_ROWS 20
#define FOM_MAT_t144_t_t143_COLS 600
#define FOM_MAT_t144_t_t143_NNZ 600
extern const fom_int FOM_MAT_t144_t_t143_ROWPTR[];
extern const fom_int FOM_MAT_t144_t_t143_COLIDX[];
extern const fom_float FOM_MAT_t144_t_t143_VALS[];

#define FOM_MAT_t145_t_t140_ROWS 30
#define FOM_MAT_t145_t_t140_COLS 600
#define FOM_MAT_t145_t_t140_NNZ 600
extern const fom_int FOM_MAT_t145_t_t140_ROWPTR[];
extern const fom_int FOM_MAT_t145_t_t140_COLIDX[];
extern const fom_float FOM_MAT_t145_t_t140_VALS[];

#define FOM_MAT_t145_t_t143_ROWS 30
#define FOM_MAT_t145_t_t143_COLS 600
#define FOM_MAT_t145_t_t143_NNZ 600
extern const fom_int FOM_MAT_t145_t_t143_ROWPTR[];
extern const fom_int FOM_MAT_t145_t_t143_COLIDX[];
extern const fom_float FOM_MAT_t145_t_t143_VALS[];

#define FOM_MAT_t147_t_t143_ROWS 20
#define FOM_MAT_t147_t_t143_COLS 600
#define FOM_MAT_t147_t_t143_NNZ 600
extern const fom_int FOM_MAT_t147_t_t143_ROWPTR[];
extern const fom_int FOM_MAT_t147_t_t143_COLIDX[];
extern const fom_float FOM_MAT_t147_t_t143_VALS[];

#define FOM_MAT_t147_t_t146_ROWS 20
#define FOM_MAT_t147_t_t146_COLS 600
#define FOM_MAT_t147_t_t146_NNZ 600
extern const fom_int FOM_MAT_t147_t_t146_ROWPTR[];
extern const fom_int FOM_MAT_t147_t_t146_COLIDX[];
extern const fom_float FOM_MAT_t147_t_t146_VALS[];

#define FOM_MAT_t148_t_t143_ROWS 30
#define FOM_MAT_t148_t_t143_COLS 600
#define FOM_MAT_t148_t_t143_NNZ 600
extern const fom_int FOM_MAT_t148_t_t143_ROWPTR[];
extern const fom_int FOM_MAT_t148_t_t143_COLIDX[];
extern const fom_float FOM_MAT_t148_t_t143_VALS[];

#define FOM_MAT_t148_t_t146_ROWS 30
#define FOM_MAT_t148_t_t146_COLS 600
#define FOM_MAT_t148_t_t146_NNZ 600
extern const fom_int FOM_MAT_t148_t_t146_ROWPTR[];
extern const fom_int FOM_MAT_t148_t_t146_COLIDX[];
extern const fom_float FOM_MAT_t148_t_t146_VALS[];

#define FOM_MAT_su_t_t146_ROWS 650
#define FOM_MAT_su_t_t146_COLS 600
#define FOM_MAT_su_t_t146_NNZ 600
extern const fom_int FOM_MAT_su_t_t146_ROWPTR[];
extern const fom_int FOM_MAT_su_t_t146_COLIDX[];
extern const fom_float FOM_MAT_su_t_t146_VALS[];

#define FOM_MAT_su_t_t147_ROWS 650
#define FOM_MAT_su_t_t147_COLS 20
#define FOM_MAT_su_t_t147_NNZ 20
extern const fom_int FOM_MAT_su_t_t147_ROWPTR[];
extern const fom_int FOM_MAT_su_t_t147_COLIDX[];
extern const fom_float FOM_MAT_su_t_t147_VALS[];

#define FOM_MAT_su_t_t148_ROWS 650
#define FOM_MAT_su_t_t148_COLS 30
#define FOM_MAT_su_t_t148_NNZ 30
extern const fom_int FOM_MAT_su_t_t148_ROWPTR[];
extern const fom_int FOM_MAT_su_t_t148_COLIDX[];
extern const fom_float FOM_MAT_su_t_t148_VALS[];

/* 0/1 selection matrices: (M @ x)[i] = x[FOM_SEL_<name>_SRC[i]], or 0
   where the index is -1. A solver may apply these itself (see SPEC.md). */
#define FOM_SEL_t3_t_t1_ROWS 20
extern const fom_int FOM_SEL_t3_t_t1_SRC[];

#define FOM_SEL_t5_t_t2_ROWS 600
extern const fom_int FOM_SEL_t5_t_t2_SRC[];

#define FOM_SEL_t7_t_t4_ROWS 30
extern const fom_int FOM_SEL_t7_t_t4_SRC[];

#define FOM_SEL_su_t_t146_ROWS 650
extern const fom_int FOM_SEL_su_t_t146_SRC[];

#define FOM_SEL_su_t_t147_ROWS 650
extern const fom_int FOM_SEL_su_t_t147_SRC[];

#define FOM_SEL_su_t_t148_ROWS 650
extern const fom_int FOM_SEL_su_t_t148_SRC[];

/* Selection rows: row FOM_SELROWS_<b>_ROW[k] of block <b> is exactly
   source FOM_SELROWS_<b>_SRC[k] (0-based, in the order of the block's
   fom_fused/fom_step arguments) at index FOM_SELROWS_<b>_COL[k]. A solver
   may write these rows itself (see SPEC.md and the __rest routines). */
/* su: rows 0..599 copy t_t146[0..599]; rows 600..619 copy t_t147[0..19]; rows 620..649 copy t_t148[0..29] */
#define FOM_SELROWS_su_N 650
extern const fom_int FOM_SELROWS_su_ROW[];
extern const fom_int FOM_SELROWS_su_SRC[];
extern const fom_int FOM_SELROWS_su_COL[];


#ifdef __cplusplus
}
#endif

#endif /* FOM_DATA_H */
