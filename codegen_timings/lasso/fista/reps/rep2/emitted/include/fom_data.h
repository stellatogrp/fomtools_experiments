#ifndef FOM_DATA_H
#define FOM_DATA_H

#include "fom_workspace.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Initial state. */
extern const fom_float FOM_STATE_INIT[FOM_STATE_DIM];

/* Schedule gamma: one value per iteration, indexed by the
   0-based loop counter. Read it through the accessor below, which
   cannot run off the end: the loop bound is the runtime
   settings.max_iter, which a caller may raise above FOM_MAX_ITER.
   Past the table the last value is held. */
#define FOM_SCHED_GAMMA_LEN 200
extern const fom_float FOM_SCHED_gamma[];
static inline fom_float fom_sched_gamma(fom_int it) {
    return FOM_SCHED_gamma[(it < FOM_SCHED_GAMMA_LEN) ? it : (FOM_SCHED_GAMMA_LEN - 1)];
}

#define FOM_MAT_t0_state_ROWS 50
#define FOM_MAT_t0_state_COLS 100
#define FOM_MAT_t0_state_NNZ 1730
extern const fom_int FOM_MAT_t0_state_ROWPTR[];
extern const fom_int FOM_MAT_t0_state_COLIDX[];
extern const fom_float FOM_MAT_t0_state_VALS[];

#define FOM_MAT_t0_p_p0_ROWS 50
#define FOM_MAT_t0_p_p0_COLS 25
#define FOM_MAT_t0_p_p0_NNZ 280
extern const fom_int FOM_MAT_t0_p_p0_ROWPTR[];
extern const fom_int FOM_MAT_t0_p_p0_COLIDX[];
extern const fom_float FOM_MAT_t0_p_p0_VALS[];

#define FOM_MAT_su_state_ROWS 100
#define FOM_MAT_su_state_COLS 100
#define FOM_MAT_su_state_NNZ 50
extern const fom_int FOM_MAT_su_state_ROWPTR[];
extern const fom_int FOM_MAT_su_state_COLIDX[];
extern const fom_float FOM_MAT_su_state_VALS[];

#define FOM_MAT_su_t_t0_ROWS 100
#define FOM_MAT_su_t_t0_COLS 50
#define FOM_MAT_su_t_t0_NNZ 50
extern const fom_int FOM_MAT_su_t_t0_ROWPTR[];
extern const fom_int FOM_MAT_su_t_t0_COLIDX[];
extern const fom_float FOM_MAT_su_t_t0_VALS[];

#define FOM_MAT_t0_state__d_gamma_ROWS 50
#define FOM_MAT_t0_state__d_gamma_COLS 100
extern const fom_float FOM_MAT_t0_state__d_gamma[];

/* 0/1 selection matrices: (M @ x)[i] = x[FOM_SEL_<name>_SRC[i]], or 0
   where the index is -1. A solver may apply these itself (see SPEC.md). */
#define FOM_SEL_su_state_ROWS 100
extern const fom_int FOM_SEL_su_state_SRC[];

#define FOM_SEL_su_t_t0_ROWS 100
extern const fom_int FOM_SEL_su_t_t0_SRC[];

/* Selection rows: row FOM_SELROWS_<b>_ROW[k] of block <b> is exactly
   source FOM_SELROWS_<b>_SRC[k] (0-based, in the order of the block's
   fom_fused/fom_step arguments) at index FOM_SELROWS_<b>_COL[k]. A solver
   may write these rows itself (see SPEC.md and the __rest routines). */
/* su: rows 0..49 copy state[50..99]; rows 50..99 copy t_t0[0..49] */
#define FOM_SELROWS_su_N 100
extern const fom_int FOM_SELROWS_su_ROW[];
extern const fom_int FOM_SELROWS_su_SRC[];
extern const fom_int FOM_SELROWS_su_COL[];

#define FOM_T0_LAMBDA ((fom_float)0.0002212861544693803)

#ifdef __cplusplus
}
#endif

#endif /* FOM_DATA_H */
