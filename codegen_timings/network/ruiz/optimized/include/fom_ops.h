#ifndef FOM_OPS_H
#define FOM_OPS_H

#include "fom_workspace.h"

#ifdef __cplusplus
extern "C" {
#endif

/* out += M @ x for each linear-op matrix M (CSR if structurally
   sparse, dense otherwise). Accumulating: zero `out` first. */

void fom_apply_t0_state(const fom_float *x, fom_float *out);  /* CSR 150x175 */
void fom_apply_t1_state(const fom_float *x, fom_float *out);  /* CSR 10x175 */
void fom_apply_t1_t_t0(const fom_float *x, fom_float *out);  /* CSR 10x150 */
void fom_apply_su_state(const fom_float *x, fom_float *out);  /* CSR 175x175 */
void fom_apply_su_t_t0(const fom_float *x, fom_float *out);  /* CSR 175x150 */
void fom_apply_su_t_t1(const fom_float *x, fom_float *out);  /* CSR 175x10 */
void fom_apply_su_p_p0(const fom_float *x, fom_float *out);  /* CSR 175x15 */

#ifdef __cplusplus
}
#endif

#endif /* FOM_OPS_H */
