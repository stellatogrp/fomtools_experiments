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

#define FOM_MAT_t0_state_ROWS 15
#define FOM_MAT_t0_state_COLS 30
extern const fom_float FOM_MAT_t0_state[];

#define FOM_MAT_t0_p_p0_ROWS 15
#define FOM_MAT_t0_p_p0_COLS 10
extern const fom_float FOM_MAT_t0_p_p0[];

#define FOM_MAT_t0_state__d_gamma_ROWS 15
#define FOM_MAT_t0_state__d_gamma_COLS 30
extern const fom_float FOM_MAT_t0_state__d_gamma[];

#define FOM_T0_LAMBDA ((fom_float)0.029093587409024832)

#ifdef __cplusplus
}
#endif

#endif /* FOM_DATA_H */
