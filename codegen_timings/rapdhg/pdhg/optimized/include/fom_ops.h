#ifndef FOM_OPS_H
#define FOM_OPS_H

#include "fom_workspace.h"

#ifdef __cplusplus
extern "C" {
#endif

/* out += M @ x for each linear-op matrix M (CSR if structurally
   sparse, dense otherwise). Accumulating: zero `out` first. */

void fom_apply_t0_state(const fom_float *x, fom_float *out);  /* CSR 48x160 */
void fom_apply_t0_state__d_taux(const fom_float *x, fom_float *out);  /* CSR 48x160 */
void fom_apply_t0_state__d_tth(const fom_float *x, fom_float *out);  /* CSR 48x160 */
void fom_apply_t0_p_p1(const fom_float *x, fom_float *out);  /* CSR 48x48 */
void fom_apply_t0_p_p1__d_tau(const fom_float *x, fom_float *out);  /* CSR 48x48 */
void fom_apply_t1_state(const fom_float *x, fom_float *out);  /* CSR 16x160 */
void fom_apply_t1_state__d_eta(const fom_float *x, fom_float *out);  /* CSR 16x160 */
void fom_apply_t1_p_p0(const fom_float *x, fom_float *out);  /* CSR 16x16 */
void fom_apply_t1_p_p0__d_eta(const fom_float *x, fom_float *out);  /* CSR 16x16 */
void fom_apply_t1_t_t0(const fom_float *x, fom_float *out);  /* CSR 16x48 */
void fom_apply_t1_t_t0__d_eta(const fom_float *x, fom_float *out);  /* CSR 16x48 */
void fom_apply_su_state(const fom_float *x, fom_float *out);  /* CSR 160x160 */
void fom_apply_su_state__d_d1(const fom_float *x, fom_float *out);  /* CSR 160x160 */
void fom_apply_su_state__d_g2(const fom_float *x, fom_float *out);  /* CSR 160x160 */
void fom_apply_su_state__d_m1(const fom_float *x, fom_float *out);  /* CSR 160x160 */
void fom_apply_su_t_t0(const fom_float *x, fom_float *out);  /* CSR 160x48 */
void fom_apply_su_t_t0__d_d2(const fom_float *x, fom_float *out);  /* CSR 160x48 */
void fom_apply_su_t_t0__d_g1(const fom_float *x, fom_float *out);  /* CSR 160x48 */
void fom_apply_su_t_t1(const fom_float *x, fom_float *out);  /* CSR 160x16 */
void fom_apply_su_t_t1__d_d2(const fom_float *x, fom_float *out);  /* CSR 160x16 */
void fom_apply_su_t_t1__d_g1(const fom_float *x, fom_float *out);  /* CSR 160x16 */
void fom_apply_su_t_t1__d_m2(const fom_float *x, fom_float *out);  /* CSR 160x16 */

#ifdef __cplusplus
}
#endif

#endif /* FOM_OPS_H */
