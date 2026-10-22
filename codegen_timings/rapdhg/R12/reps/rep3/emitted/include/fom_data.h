#ifndef FOM_DATA_H
#define FOM_DATA_H

#include "fom_workspace.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Initial state. */
extern const fom_float FOM_STATE_INIT[FOM_STATE_DIM];

/* Schedule d1: one value per iteration, indexed by the
   0-based loop counter. Read it through the accessor below, which
   cannot run off the end: the loop bound is the runtime
   settings.max_iter, which a caller may raise above FOM_MAX_ITER.
   Past the table the last value is held. */
#define FOM_SCHED_D1_LEN 200
extern const fom_float FOM_SCHED_d1[];
static inline fom_float fom_sched_d1(fom_int it) {
    return FOM_SCHED_d1[(it < FOM_SCHED_D1_LEN) ? it : (FOM_SCHED_D1_LEN - 1)];
}

/* Schedule d2: one value per iteration, indexed by the
   0-based loop counter. Read it through the accessor below, which
   cannot run off the end: the loop bound is the runtime
   settings.max_iter, which a caller may raise above FOM_MAX_ITER.
   Past the table the last value is held. */
#define FOM_SCHED_D2_LEN 200
extern const fom_float FOM_SCHED_d2[];
static inline fom_float fom_sched_d2(fom_int it) {
    return FOM_SCHED_d2[(it < FOM_SCHED_D2_LEN) ? it : (FOM_SCHED_D2_LEN - 1)];
}

/* Schedule eta: one value per iteration, indexed by the
   0-based loop counter. Read it through the accessor below, which
   cannot run off the end: the loop bound is the runtime
   settings.max_iter, which a caller may raise above FOM_MAX_ITER.
   Past the table the last value is held. */
#define FOM_SCHED_ETA_LEN 200
extern const fom_float FOM_SCHED_eta[];
static inline fom_float fom_sched_eta(fom_int it) {
    return FOM_SCHED_eta[(it < FOM_SCHED_ETA_LEN) ? it : (FOM_SCHED_ETA_LEN - 1)];
}

/* Schedule g1: one value per iteration, indexed by the
   0-based loop counter. Read it through the accessor below, which
   cannot run off the end: the loop bound is the runtime
   settings.max_iter, which a caller may raise above FOM_MAX_ITER.
   Past the table the last value is held. */
#define FOM_SCHED_G1_LEN 200
extern const fom_float FOM_SCHED_g1[];
static inline fom_float fom_sched_g1(fom_int it) {
    return FOM_SCHED_g1[(it < FOM_SCHED_G1_LEN) ? it : (FOM_SCHED_G1_LEN - 1)];
}

/* Schedule g2: one value per iteration, indexed by the
   0-based loop counter. Read it through the accessor below, which
   cannot run off the end: the loop bound is the runtime
   settings.max_iter, which a caller may raise above FOM_MAX_ITER.
   Past the table the last value is held. */
#define FOM_SCHED_G2_LEN 200
extern const fom_float FOM_SCHED_g2[];
static inline fom_float fom_sched_g2(fom_int it) {
    return FOM_SCHED_g2[(it < FOM_SCHED_G2_LEN) ? it : (FOM_SCHED_G2_LEN - 1)];
}

/* Schedule m1: one value per iteration, indexed by the
   0-based loop counter. Read it through the accessor below, which
   cannot run off the end: the loop bound is the runtime
   settings.max_iter, which a caller may raise above FOM_MAX_ITER.
   Past the table the last value is held. */
#define FOM_SCHED_M1_LEN 200
extern const fom_float FOM_SCHED_m1[];
static inline fom_float fom_sched_m1(fom_int it) {
    return FOM_SCHED_m1[(it < FOM_SCHED_M1_LEN) ? it : (FOM_SCHED_M1_LEN - 1)];
}

/* Schedule m2: one value per iteration, indexed by the
   0-based loop counter. Read it through the accessor below, which
   cannot run off the end: the loop bound is the runtime
   settings.max_iter, which a caller may raise above FOM_MAX_ITER.
   Past the table the last value is held. */
#define FOM_SCHED_M2_LEN 200
extern const fom_float FOM_SCHED_m2[];
static inline fom_float fom_sched_m2(fom_int it) {
    return FOM_SCHED_m2[(it < FOM_SCHED_M2_LEN) ? it : (FOM_SCHED_M2_LEN - 1)];
}

/* Schedule tau: one value per iteration, indexed by the
   0-based loop counter. Read it through the accessor below, which
   cannot run off the end: the loop bound is the runtime
   settings.max_iter, which a caller may raise above FOM_MAX_ITER.
   Past the table the last value is held. */
#define FOM_SCHED_TAU_LEN 200
extern const fom_float FOM_SCHED_tau[];
static inline fom_float fom_sched_tau(fom_int it) {
    return FOM_SCHED_tau[(it < FOM_SCHED_TAU_LEN) ? it : (FOM_SCHED_TAU_LEN - 1)];
}

/* Schedule taux: one value per iteration, indexed by the
   0-based loop counter. Read it through the accessor below, which
   cannot run off the end: the loop bound is the runtime
   settings.max_iter, which a caller may raise above FOM_MAX_ITER.
   Past the table the last value is held. */
#define FOM_SCHED_TAUX_LEN 200
extern const fom_float FOM_SCHED_taux[];
static inline fom_float fom_sched_taux(fom_int it) {
    return FOM_SCHED_taux[(it < FOM_SCHED_TAUX_LEN) ? it : (FOM_SCHED_TAUX_LEN - 1)];
}

/* Schedule tth: one value per iteration, indexed by the
   0-based loop counter. Read it through the accessor below, which
   cannot run off the end: the loop bound is the runtime
   settings.max_iter, which a caller may raise above FOM_MAX_ITER.
   Past the table the last value is held. */
#define FOM_SCHED_TTH_LEN 200
extern const fom_float FOM_SCHED_tth[];
static inline fom_float fom_sched_tth(fom_int it) {
    return FOM_SCHED_tth[(it < FOM_SCHED_TTH_LEN) ? it : (FOM_SCHED_TTH_LEN - 1)];
}

#define FOM_MAT_t0_state_ROWS 48
#define FOM_MAT_t0_state_COLS 160
#define FOM_MAT_t0_state_NNZ 48
extern const fom_int FOM_MAT_t0_state_ROWPTR[];
extern const fom_int FOM_MAT_t0_state_COLIDX[];
extern const fom_float FOM_MAT_t0_state_VALS[];

#define FOM_MAT_t0_p_p1_ROWS 48
#define FOM_MAT_t0_p_p1_COLS 48
#define FOM_MAT_t0_p_p1_NNZ 0
extern const fom_int FOM_MAT_t0_p_p1_ROWPTR[];
extern const fom_int FOM_MAT_t0_p_p1_COLIDX[];
extern const fom_float FOM_MAT_t0_p_p1_VALS[];

#define FOM_MAT_t1_state_ROWS 16
#define FOM_MAT_t1_state_COLS 160
#define FOM_MAT_t1_state_NNZ 16
extern const fom_int FOM_MAT_t1_state_ROWPTR[];
extern const fom_int FOM_MAT_t1_state_COLIDX[];
extern const fom_float FOM_MAT_t1_state_VALS[];

#define FOM_MAT_t1_p_p0_ROWS 16
#define FOM_MAT_t1_p_p0_COLS 16
#define FOM_MAT_t1_p_p0_NNZ 0
extern const fom_int FOM_MAT_t1_p_p0_ROWPTR[];
extern const fom_int FOM_MAT_t1_p_p0_COLIDX[];
extern const fom_float FOM_MAT_t1_p_p0_VALS[];

#define FOM_MAT_t1_t_t0_ROWS 16
#define FOM_MAT_t1_t_t0_COLS 48
#define FOM_MAT_t1_t_t0_NNZ 0
extern const fom_int FOM_MAT_t1_t_t0_ROWPTR[];
extern const fom_int FOM_MAT_t1_t_t0_COLIDX[];
extern const fom_float FOM_MAT_t1_t_t0_VALS[];

#define FOM_MAT_t1_lo_ROWS 16
#define FOM_MAT_t1_lo_COLS 1
extern const fom_float FOM_MAT_t1_lo[];

#define FOM_MAT_t1_hi_ROWS 16
#define FOM_MAT_t1_hi_COLS 1
extern const fom_float FOM_MAT_t1_hi[];

#define FOM_MAT_su_state_ROWS 160
#define FOM_MAT_su_state_COLS 160
#define FOM_MAT_su_state_NNZ 16
extern const fom_int FOM_MAT_su_state_ROWPTR[];
extern const fom_int FOM_MAT_su_state_COLIDX[];
extern const fom_float FOM_MAT_su_state_VALS[];

#define FOM_MAT_su_t_t0_ROWS 160
#define FOM_MAT_su_t_t0_COLS 48
#define FOM_MAT_su_t_t0_NNZ 0
extern const fom_int FOM_MAT_su_t_t0_ROWPTR[];
extern const fom_int FOM_MAT_su_t_t0_COLIDX[];
extern const fom_float FOM_MAT_su_t_t0_VALS[];

#define FOM_MAT_su_t_t1_ROWS 160
#define FOM_MAT_su_t_t1_COLS 16
#define FOM_MAT_su_t_t1_NNZ 0
extern const fom_int FOM_MAT_su_t_t1_ROWPTR[];
extern const fom_int FOM_MAT_su_t_t1_COLIDX[];
extern const fom_float FOM_MAT_su_t_t1_VALS[];

#define FOM_MAT_t0_state__d_taux_ROWS 48
#define FOM_MAT_t0_state__d_taux_COLS 160
#define FOM_MAT_t0_state__d_taux_NNZ 112
extern const fom_int FOM_MAT_t0_state__d_taux_ROWPTR[];
extern const fom_int FOM_MAT_t0_state__d_taux_COLIDX[];
extern const fom_float FOM_MAT_t0_state__d_taux_VALS[];

#define FOM_MAT_t0_state__d_tth_ROWS 48
#define FOM_MAT_t0_state__d_tth_COLS 160
#define FOM_MAT_t0_state__d_tth_NNZ 112
extern const fom_int FOM_MAT_t0_state__d_tth_ROWPTR[];
extern const fom_int FOM_MAT_t0_state__d_tth_COLIDX[];
extern const fom_float FOM_MAT_t0_state__d_tth_VALS[];

#define FOM_MAT_t0_p_p1__d_tau_ROWS 48
#define FOM_MAT_t0_p_p1__d_tau_COLS 48
#define FOM_MAT_t0_p_p1__d_tau_NNZ 48
extern const fom_int FOM_MAT_t0_p_p1__d_tau_ROWPTR[];
extern const fom_int FOM_MAT_t0_p_p1__d_tau_COLIDX[];
extern const fom_float FOM_MAT_t0_p_p1__d_tau_VALS[];

#define FOM_MAT_t1_state__d_eta_ROWS 16
#define FOM_MAT_t1_state__d_eta_COLS 160
#define FOM_MAT_t1_state__d_eta_NNZ 256
extern const fom_int FOM_MAT_t1_state__d_eta_ROWPTR[];
extern const fom_int FOM_MAT_t1_state__d_eta_COLIDX[];
extern const fom_float FOM_MAT_t1_state__d_eta_VALS[];

#define FOM_MAT_t1_p_p0__d_eta_ROWS 16
#define FOM_MAT_t1_p_p0__d_eta_COLS 16
#define FOM_MAT_t1_p_p0__d_eta_NNZ 16
extern const fom_int FOM_MAT_t1_p_p0__d_eta_ROWPTR[];
extern const fom_int FOM_MAT_t1_p_p0__d_eta_COLIDX[];
extern const fom_float FOM_MAT_t1_p_p0__d_eta_VALS[];

#define FOM_MAT_t1_t_t0__d_eta_ROWS 16
#define FOM_MAT_t1_t_t0__d_eta_COLS 48
#define FOM_MAT_t1_t_t0__d_eta_NNZ 112
extern const fom_int FOM_MAT_t1_t_t0__d_eta_ROWPTR[];
extern const fom_int FOM_MAT_t1_t_t0__d_eta_COLIDX[];
extern const fom_float FOM_MAT_t1_t_t0__d_eta_VALS[];

#define FOM_MAT_su_state__d_d1_ROWS 160
#define FOM_MAT_su_state__d_d1_COLS 160
#define FOM_MAT_su_state__d_d1_NNZ 64
extern const fom_int FOM_MAT_su_state__d_d1_ROWPTR[];
extern const fom_int FOM_MAT_su_state__d_d1_COLIDX[];
extern const fom_float FOM_MAT_su_state__d_d1_VALS[];

#define FOM_MAT_su_state__d_g2_ROWS 160
#define FOM_MAT_su_state__d_g2_COLS 160
#define FOM_MAT_su_state__d_g2_NNZ 64
extern const fom_int FOM_MAT_su_state__d_g2_ROWPTR[];
extern const fom_int FOM_MAT_su_state__d_g2_COLIDX[];
extern const fom_float FOM_MAT_su_state__d_g2_VALS[];

#define FOM_MAT_su_state__d_m1_ROWS 160
#define FOM_MAT_su_state__d_m1_COLS 160
#define FOM_MAT_su_state__d_m1_NNZ 16
extern const fom_int FOM_MAT_su_state__d_m1_ROWPTR[];
extern const fom_int FOM_MAT_su_state__d_m1_COLIDX[];
extern const fom_float FOM_MAT_su_state__d_m1_VALS[];

#define FOM_MAT_su_t_t0__d_d2_ROWS 160
#define FOM_MAT_su_t_t0__d_d2_COLS 48
#define FOM_MAT_su_t_t0__d_d2_NNZ 48
extern const fom_int FOM_MAT_su_t_t0__d_d2_ROWPTR[];
extern const fom_int FOM_MAT_su_t_t0__d_d2_COLIDX[];
extern const fom_float FOM_MAT_su_t_t0__d_d2_VALS[];

#define FOM_MAT_su_t_t0__d_g1_ROWS 160
#define FOM_MAT_su_t_t0__d_g1_COLS 48
#define FOM_MAT_su_t_t0__d_g1_NNZ 48
extern const fom_int FOM_MAT_su_t_t0__d_g1_ROWPTR[];
extern const fom_int FOM_MAT_su_t_t0__d_g1_COLIDX[];
extern const fom_float FOM_MAT_su_t_t0__d_g1_VALS[];

#define FOM_MAT_su_t_t1__d_d2_ROWS 160
#define FOM_MAT_su_t_t1__d_d2_COLS 16
#define FOM_MAT_su_t_t1__d_d2_NNZ 16
extern const fom_int FOM_MAT_su_t_t1__d_d2_ROWPTR[];
extern const fom_int FOM_MAT_su_t_t1__d_d2_COLIDX[];
extern const fom_float FOM_MAT_su_t_t1__d_d2_VALS[];

#define FOM_MAT_su_t_t1__d_g1_ROWS 160
#define FOM_MAT_su_t_t1__d_g1_COLS 16
#define FOM_MAT_su_t_t1__d_g1_NNZ 16
extern const fom_int FOM_MAT_su_t_t1__d_g1_ROWPTR[];
extern const fom_int FOM_MAT_su_t_t1__d_g1_COLIDX[];
extern const fom_float FOM_MAT_su_t_t1__d_g1_VALS[];

#define FOM_MAT_su_t_t1__d_m2_ROWS 160
#define FOM_MAT_su_t_t1__d_m2_COLS 16
#define FOM_MAT_su_t_t1__d_m2_NNZ 16
extern const fom_int FOM_MAT_su_t_t1__d_m2_ROWPTR[];
extern const fom_int FOM_MAT_su_t_t1__d_m2_COLIDX[];
extern const fom_float FOM_MAT_su_t_t1__d_m2_VALS[];

/* 0/1 selection matrices: (M @ x)[i] = x[FOM_SEL_<name>_SRC[i]], or 0
   where the index is -1. A solver may apply these itself (see SPEC.md). */
#define FOM_SEL_t0_state_ROWS 48
extern const fom_int FOM_SEL_t0_state_SRC[];

#define FOM_SEL_t0_p_p1_ROWS 48
extern const fom_int FOM_SEL_t0_p_p1_SRC[];

#define FOM_SEL_t1_state_ROWS 16
extern const fom_int FOM_SEL_t1_state_SRC[];

#define FOM_SEL_t1_p_p0_ROWS 16
extern const fom_int FOM_SEL_t1_p_p0_SRC[];

#define FOM_SEL_t1_t_t0_ROWS 16
extern const fom_int FOM_SEL_t1_t_t0_SRC[];

#define FOM_SEL_su_state_ROWS 160
extern const fom_int FOM_SEL_su_state_SRC[];

#define FOM_SEL_su_state__d_d1_ROWS 160
extern const fom_int FOM_SEL_su_state__d_d1_SRC[];

#define FOM_SEL_su_state__d_g2_ROWS 160
extern const fom_int FOM_SEL_su_state__d_g2_SRC[];

#define FOM_SEL_su_state__d_m1_ROWS 160
extern const fom_int FOM_SEL_su_state__d_m1_SRC[];

#define FOM_SEL_su_t_t0_ROWS 160
extern const fom_int FOM_SEL_su_t_t0_SRC[];

#define FOM_SEL_su_t_t0__d_d2_ROWS 160
extern const fom_int FOM_SEL_su_t_t0__d_d2_SRC[];

#define FOM_SEL_su_t_t0__d_g1_ROWS 160
extern const fom_int FOM_SEL_su_t_t0__d_g1_SRC[];

#define FOM_SEL_su_t_t1_ROWS 160
extern const fom_int FOM_SEL_su_t_t1_SRC[];

#define FOM_SEL_su_t_t1__d_d2_ROWS 160
extern const fom_int FOM_SEL_su_t_t1__d_d2_SRC[];

#define FOM_SEL_su_t_t1__d_g1_ROWS 160
extern const fom_int FOM_SEL_su_t_t1__d_g1_SRC[];

#define FOM_SEL_su_t_t1__d_m2_ROWS 160
extern const fom_int FOM_SEL_su_t_t1__d_m2_SRC[];

/* Selection rows: row FOM_SELROWS_<b>_ROW[k] of block <b> is exactly
   source FOM_SELROWS_<b>_SRC[k] (0-based, in the order of the block's
   fom_fused/fom_step arguments) at index FOM_SELROWS_<b>_COL[k]. A solver
   may write these rows itself (see SPEC.md and the __rest routines). */
/* su: rows 128..143 copy state[0..15] */
#define FOM_SELROWS_su_N 16
extern const fom_int FOM_SELROWS_su_ROW[];
extern const fom_int FOM_SELROWS_su_SRC[];
extern const fom_int FOM_SELROWS_su_COL[];

/* su: the rows a __rest routine computes (all but the selection rows). */
#define FOM_REST_su_N 144
extern const fom_int FOM_REST_su_ROWS[];


#ifdef __cplusplus
}
#endif

#endif /* FOM_DATA_H */
