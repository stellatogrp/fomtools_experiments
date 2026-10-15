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

#define FOM_FAC_T3_DIM 20
void fom_lin_solve_t3(const fom_float *b, fom_float *x);  /* LDLT, dim 20, nnz 65 */

#ifdef __cplusplus
}
#endif

#endif /* FOM_LINSOLVE_H */
