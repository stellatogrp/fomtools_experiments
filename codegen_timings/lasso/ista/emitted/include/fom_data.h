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

#define FOM_T0_LAMBDA ((fom_float)0.0002212861544693803)

#ifdef __cplusplus
}
#endif

#endif /* FOM_DATA_H */
