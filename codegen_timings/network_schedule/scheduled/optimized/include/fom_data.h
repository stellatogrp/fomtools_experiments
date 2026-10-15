#ifndef FOM_DATA_H
#define FOM_DATA_H

#include "fom_workspace.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Initial state. */
extern const fom_float FOM_STATE_INIT[FOM_STATE_DIM];

/* Schedule xi: one value per iteration, indexed by the
   0-based loop counter. Read it through the accessor below, which
   cannot run off the end: the loop bound is the runtime
   settings.max_iter, which a caller may raise above FOM_MAX_ITER.
   Past the table the last value is held. */
#define FOM_SCHED_XI_LEN 50
extern const fom_float FOM_SCHED_xi[];
static inline fom_float fom_sched_xi(fom_int it) {
    return FOM_SCHED_xi[(it < FOM_SCHED_XI_LEN) ? it : (FOM_SCHED_XI_LEN - 1)];
}

#define FOM_MAT_t0_c_ROWS 600
#define FOM_MAT_t0_c_COLS 1
extern const fom_float FOM_MAT_t0_c[];

#define FOM_MAT_t0_lo_ROWS 600
#define FOM_MAT_t0_lo_COLS 1
extern const fom_float FOM_MAT_t0_lo[];

#define FOM_MAT_t0_hi_ROWS 600
#define FOM_MAT_t0_hi_COLS 1
extern const fom_float FOM_MAT_t0_hi[];

#define FOM_MAT_t1_c_ROWS 20
#define FOM_MAT_t1_c_COLS 1
extern const fom_float FOM_MAT_t1_c[];


#ifdef __cplusplus
}
#endif

#endif /* FOM_DATA_H */
