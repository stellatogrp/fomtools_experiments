#ifndef FOM_LINSOLVE_H
#define FOM_LINSOLVE_H

#include "fom_workspace.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Solve M x = b for each affine operator. Direct methods (LDL^T / LU)
   apply a precomputed sparse factorization and are division-free
   (diagonals stored as reciprocals); the CG method runs preconditioned
   conjugate gradient (sparse matvecs + a diagonal preconditioner). No
   matrix inverse is ever formed or used. */

#ifdef __cplusplus
}
#endif

#endif /* FOM_LINSOLVE_H */
