#ifndef FOM_SOLVER_H
#define FOM_SOLVER_H

#include "fom_workspace.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Fill settings with the generated defaults (max_iter, tol). */
void fom_set_default_settings(FomSettings *settings);

/* Initialize the workspace: copy settings and the initial state. */
void fom_setup(FomWorkspace *work, const FomSettings *settings);

/* Copy a new parameter vector set into the workspace. */
void fom_update_params(FomWorkspace *work, const FomParams *params);

/* Run the first-order method. Returns the status code. */
fom_int fom_solve(FomWorkspace *work);

/* Release resources (no-op for the embedded target). */
void fom_cleanup(FomWorkspace *work);

#ifdef __cplusplus
}
#endif

#endif /* FOM_SOLVER_H */
