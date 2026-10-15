#ifndef FOM_OPS_H
#define FOM_OPS_H

#include "fom_workspace.h"

#ifdef __cplusplus
extern "C" {
#endif

/* out += M @ x for each linear-op matrix M (CSR if structurally
   sparse, dense otherwise). Accumulating: zero `out` first. */

void fom_apply_t0_state(const fom_float *x, fom_float *out);  /* CSR 600x650 */
void fom_apply_t1_state(const fom_float *x, fom_float *out);  /* CSR 20x650 */
void fom_apply_t1_state__d_xi(const fom_float *x, fom_float *out);  /* CSR 20x650 */
void fom_apply_t1_t_t0(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t1_t_t0__d_xi(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_su_state(const fom_float *x, fom_float *out);  /* CSR 650x650 */
void fom_apply_su_state__d_xi(const fom_float *x, fom_float *out);  /* CSR 650x650 */
void fom_apply_su_t_t0(const fom_float *x, fom_float *out);  /* CSR 650x600 */
void fom_apply_su_t_t0__d_xi(const fom_float *x, fom_float *out);  /* CSR 650x600 */
void fom_apply_su_t_t1(const fom_float *x, fom_float *out);  /* CSR 650x20 */
void fom_apply_su_p_p0(const fom_float *x, fom_float *out);  /* CSR 650x30 */

#ifdef __cplusplus
}
#endif

#endif /* FOM_OPS_H */
