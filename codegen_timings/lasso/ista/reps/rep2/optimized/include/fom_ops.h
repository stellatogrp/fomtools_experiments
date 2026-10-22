#ifndef FOM_OPS_H
#define FOM_OPS_H

#include "fom_workspace.h"

#ifdef __cplusplus
extern "C" {
#endif

/* out += M @ x for each linear-op matrix M (CSR if structurally
   sparse, dense otherwise). Accumulating: zero `out` first. */

void fom_apply_t0_state(const fom_float *x, fom_float *out);  /* dense 50x50 */
void fom_apply_t0_p_p0(const fom_float *x, fom_float *out);  /* CSR 50x25 */
void fom_apply_su_t_t0(const fom_float *x, fom_float *out);  /* CSR 50x50 */

/* Step routines: a whole output block, out = act(bias + sum_k s_k M_k src_k),
   OVERWRITING out. `bias` is the constant plus the parameter terms (the
   caller computes it, so it can be hoisted); `act` carries the activation's
   parameters (pass &FOM_ACT_<block>). Checked against the members and a
   reference activation, with the activation off, on, and at random bounds. */
/* soft threshold: v -> sign(v) max(|v| - lambd, 0) */
typedef struct { fom_float lambd; } FomAct_t0;
extern const FomAct_t0 FOM_ACT_t0;
/* out = soft_threshold(bias + t0_state @ state) */
void fom_step_t0(const fom_float *state, const fom_float *bias, const FomAct_t0 *act, fom_float *out);
/* out = bias + su_t_t0 @ t_t0 */
void fom_step_su(const fom_float *t_t0, const fom_float *bias, fom_float *out);

/* One whole iteration: every body step in order (temporaries written to
   t_<name>), then the state update into new_state. Checked against the
   chained reference steps, temporaries included. */
void fom_iterate(const fom_float *state, const fom_float *bias_t0, const fom_float *bias_su, const FomAct_t0 *act_t0, fom_float *t_t0, fom_float *new_state);

#ifdef __cplusplus
}
#endif

#endif /* FOM_OPS_H */
