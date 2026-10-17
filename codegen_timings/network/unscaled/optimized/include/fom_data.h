#ifndef FOM_DATA_H
#define FOM_DATA_H

#include "fom_workspace.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Initial state. */
extern const fom_float FOM_STATE_INIT[FOM_STATE_DIM];

#define FOM_MAT_t0_c_ROWS 49
#define FOM_MAT_t0_c_COLS 1
extern const fom_float FOM_MAT_t0_c[];

#define FOM_MAT_t0_lo_ROWS 49
#define FOM_MAT_t0_lo_COLS 1
extern const fom_float FOM_MAT_t0_lo[];

#define FOM_MAT_t0_hi_ROWS 49
#define FOM_MAT_t0_hi_COLS 1
extern const fom_float FOM_MAT_t0_hi[];

#define FOM_MAT_t1_c_ROWS 10
#define FOM_MAT_t1_c_COLS 1
extern const fom_float FOM_MAT_t1_c[];


#ifdef __cplusplus
}
#endif

#endif /* FOM_DATA_H */
