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

#define FOM_MAT_t1_lo_ROWS 16
#define FOM_MAT_t1_lo_COLS 1
extern const fom_float FOM_MAT_t1_lo[];

#define FOM_MAT_t1_hi_ROWS 16
#define FOM_MAT_t1_hi_COLS 1
extern const fom_float FOM_MAT_t1_hi[];


#ifdef __cplusplus
}
#endif

#endif /* FOM_DATA_H */
