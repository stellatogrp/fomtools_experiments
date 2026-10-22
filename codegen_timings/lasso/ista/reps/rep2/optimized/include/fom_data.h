#ifndef FOM_DATA_H
#define FOM_DATA_H

#include "fom_workspace.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Initial state. */
extern const fom_float FOM_STATE_INIT[FOM_STATE_DIM];

#define FOM_MAT_t0_state_ROWS 50
#define FOM_MAT_t0_state_COLS 50
extern const fom_float FOM_MAT_t0_state[];

#define FOM_MAT_t0_p_p0_ROWS 50
#define FOM_MAT_t0_p_p0_COLS 25
#define FOM_MAT_t0_p_p0_NNZ 280
extern const fom_int FOM_MAT_t0_p_p0_ROWPTR[];
extern const fom_int FOM_MAT_t0_p_p0_COLIDX[];
extern const fom_float FOM_MAT_t0_p_p0_VALS[];

#define FOM_MAT_su_t_t0_ROWS 50
#define FOM_MAT_su_t_t0_COLS 50
#define FOM_MAT_su_t_t0_NNZ 50
extern const fom_int FOM_MAT_su_t_t0_ROWPTR[];
extern const fom_int FOM_MAT_su_t_t0_COLIDX[];
extern const fom_float FOM_MAT_su_t_t0_VALS[];

/* 0/1 selection matrices: (M @ x)[i] = x[FOM_SEL_<name>_SRC[i]], or 0
   where the index is -1. A solver may apply these itself (see SPEC.md). */
#define FOM_SEL_su_t_t0_ROWS 50
extern const fom_int FOM_SEL_su_t_t0_SRC[];

/* Selection rows: row FOM_SELROWS_<b>_ROW[k] of block <b> is exactly
   source FOM_SELROWS_<b>_SRC[k] (0-based, in the order of the block's
   fom_fused/fom_step arguments) at index FOM_SELROWS_<b>_COL[k]. A solver
   may write these rows itself (see SPEC.md and the __rest routines). */
/* su: rows 0..49 copy t_t0[0..49] */
#define FOM_SELROWS_su_N 50
extern const fom_int FOM_SELROWS_su_ROW[];
extern const fom_int FOM_SELROWS_su_SRC[];
extern const fom_int FOM_SELROWS_su_COL[];

#define FOM_T0_LAMBDA ((fom_float)0.0002212861544693803)

#ifdef __cplusplus
}
#endif

#endif /* FOM_DATA_H */
