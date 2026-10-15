#ifndef FOM_DATA_H
#define FOM_DATA_H

#include "fom_workspace.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Initial state. */
extern const fom_float FOM_STATE_INIT[FOM_STATE_DIM];

#define FOM_MAT_t0_state_ROWS 15
#define FOM_MAT_t0_state_COLS 15
extern const fom_float FOM_MAT_t0_state[];

#define FOM_MAT_t0_p_p0_ROWS 15
#define FOM_MAT_t0_p_p0_COLS 10
extern const fom_float FOM_MAT_t0_p_p0[];

#define FOM_T0_LAMBDA ((fom_float)0.029093587409024832)

#ifdef __cplusplus
}
#endif

#endif /* FOM_DATA_H */
