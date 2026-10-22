#ifndef FOM_DATA_H
#define FOM_DATA_H

#include "fom_workspace.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Initial state. */
extern const fom_float FOM_STATE_INIT[FOM_STATE_DIM];

#define FOM_MAT_t0_state_ROWS 49
#define FOM_MAT_t0_state_COLS 74
#define FOM_MAT_t0_state_NNZ 147
extern const fom_int FOM_MAT_t0_state_ROWPTR[];
extern const fom_int FOM_MAT_t0_state_COLIDX[];
extern const fom_float FOM_MAT_t0_state_VALS[];

#define FOM_MAT_t0_c_ROWS 49
#define FOM_MAT_t0_c_COLS 1
extern const fom_float FOM_MAT_t0_c[];

#define FOM_MAT_t0_lo_ROWS 49
#define FOM_MAT_t0_lo_COLS 1
extern const fom_float FOM_MAT_t0_lo[];

#define FOM_MAT_t0_hi_ROWS 49
#define FOM_MAT_t0_hi_COLS 1
extern const fom_float FOM_MAT_t0_hi[];

#define FOM_MAT_t1_state_ROWS 10
#define FOM_MAT_t1_state_COLS 74
#define FOM_MAT_t1_state_NNZ 59
extern const fom_int FOM_MAT_t1_state_ROWPTR[];
extern const fom_int FOM_MAT_t1_state_COLIDX[];
extern const fom_float FOM_MAT_t1_state_VALS[];

#define FOM_MAT_t1_t_t0_ROWS 10
#define FOM_MAT_t1_t_t0_COLS 49
#define FOM_MAT_t1_t_t0_NNZ 49
extern const fom_int FOM_MAT_t1_t_t0_ROWPTR[];
extern const fom_int FOM_MAT_t1_t_t0_COLIDX[];
extern const fom_float FOM_MAT_t1_t_t0_VALS[];

#define FOM_MAT_t1_c_ROWS 10
#define FOM_MAT_t1_c_COLS 1
extern const fom_float FOM_MAT_t1_c[];

#define FOM_MAT_su_state_ROWS 74
#define FOM_MAT_su_state_COLS 74
#define FOM_MAT_su_state_NNZ 64
extern const fom_int FOM_MAT_su_state_ROWPTR[];
extern const fom_int FOM_MAT_su_state_COLIDX[];
extern const fom_float FOM_MAT_su_state_VALS[];

#define FOM_MAT_su_t_t0_ROWS 74
#define FOM_MAT_su_t_t0_COLS 49
#define FOM_MAT_su_t_t0_NNZ 98
extern const fom_int FOM_MAT_su_t_t0_ROWPTR[];
extern const fom_int FOM_MAT_su_t_t0_COLIDX[];
extern const fom_float FOM_MAT_su_t_t0_VALS[];

#define FOM_MAT_su_t_t1_ROWS 74
#define FOM_MAT_su_t_t1_COLS 10
#define FOM_MAT_su_t_t1_NNZ 10
extern const fom_int FOM_MAT_su_t_t1_ROWPTR[];
extern const fom_int FOM_MAT_su_t_t1_COLIDX[];
extern const fom_float FOM_MAT_su_t_t1_VALS[];

#define FOM_MAT_su_p_p0_ROWS 74
#define FOM_MAT_su_p_p0_COLS 15
#define FOM_MAT_su_p_p0_NNZ 15
extern const fom_int FOM_MAT_su_p_p0_ROWPTR[];
extern const fom_int FOM_MAT_su_p_p0_COLIDX[];
extern const fom_float FOM_MAT_su_p_p0_VALS[];

/* 0/1 selection matrices: (M @ x)[i] = x[FOM_SEL_<name>_SRC[i]], or 0
   where the index is -1. A solver may apply these itself (see SPEC.md). */
#define FOM_SEL_su_t_t1_ROWS 74
extern const fom_int FOM_SEL_su_t_t1_SRC[];

/* Selection rows: row FOM_SELROWS_<b>_ROW[k] of block <b> is exactly
   source FOM_SELROWS_<b>_SRC[k] (0-based, in the order of the block's
   fom_fused/fom_step arguments) at index FOM_SELROWS_<b>_COL[k]. A solver
   may write these rows itself (see SPEC.md and the __rest routines). */
/* su: rows 0..48 copy t_t0[0..48]; rows 49..58 copy t_t1[0..9] */
#define FOM_SELROWS_su_N 59
extern const fom_int FOM_SELROWS_su_ROW[];
extern const fom_int FOM_SELROWS_su_SRC[];
extern const fom_int FOM_SELROWS_su_COL[];

/* su: the rows a __rest routine computes (all but the selection rows). */
#define FOM_REST_su_N 15
extern const fom_int FOM_REST_su_ROWS[];


#ifdef __cplusplus
}
#endif

#endif /* FOM_DATA_H */
