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

/* Fused products: every per-iteration product of one output block in a
   single routine (parameter terms are left out, so they can be hoisted).
   Same accumulate convention; each is checked against its members. */
/* out += t0_state @ state + sc_taux * (t0_state__d_taux @ state) + sc_tth * (t0_state__d_tth @ state) */
void fom_fused_t0(const fom_float *state, fom_float sc_taux, fom_float sc_tth, fom_float *out);
/* out += t1_state @ state + sc_eta * (t1_state__d_eta @ state) + sc_eta * (t1_t_t0__d_eta @ t_t0) */
void fom_fused_t1(const fom_float *state, const fom_float *t_t0, fom_float sc_eta, fom_float *out);
/* out += su_state @ state + sc_d1 * (su_state__d_d1 @ state) + sc_g2 * (su_state__d_g2 @ state) + sc_m1 * (su_state__d_m1 @ state) + sc_d2 * (su_t_t0__d_d2 @ t_t0) + sc_g1 * (su_t_t0__d_g1 @ t_t0) + sc_d2 * (su_t_t1__d_d2 @ t_t1) + sc_g1 * (su_t_t1__d_g1 @ t_t1) + sc_m2 * (su_t_t1__d_m2 @ t_t1) */
void fom_fused_su(const fom_float *state, const fom_float *t_t0, const fom_float *t_t1, fom_float sc_d1, fom_float sc_g2, fom_float sc_m1, fom_float sc_d2, fom_float sc_g1, fom_float sc_m2, fom_float *out);

/* Step routines: a whole output block, out = act(bias + sum_k s_k M_k src_k),
   OVERWRITING out. `bias` is the constant plus the parameter terms (the
   caller computes it, so it can be hoisted); `act` carries the activation's
   parameters (pass &FOM_ACT_<block>). Checked against the members and a
   reference activation, with the activation off, on, and at random bounds. */
/* relu: v -> max(v, lo[i]) (lo = 0 by default) */
typedef struct { const fom_float *lo; } FomAct_t0;
extern const FomAct_t0 FOM_ACT_t0;
/* out = relu(bias + t0_state @ state + sc_taux * (t0_state__d_taux @ state) + sc_tth * (t0_state__d_tth @ state)) */
void fom_step_t0(const fom_float *state, fom_float sc_taux, fom_float sc_tth, const fom_float *bias, const FomAct_t0 *act, fom_float *out);
/* saturated linear: v -> min(max(v, lo[i]), hi[i]) */
typedef struct { const fom_float *lo; const fom_float *hi; } FomAct_t1;
extern const FomAct_t1 FOM_ACT_t1;
/* out = saturated_linear(bias + t1_state @ state + sc_eta * (t1_state__d_eta @ state) + sc_eta * (t1_t_t0__d_eta @ t_t0)) */
void fom_step_t1(const fom_float *state, const fom_float *t_t0, fom_float sc_eta, const fom_float *bias, const FomAct_t1 *act, fom_float *out);
/* out = bias + su_state @ state + sc_d1 * (su_state__d_d1 @ state) + sc_g2 * (su_state__d_g2 @ state) + sc_m1 * (su_state__d_m1 @ state) + sc_d2 * (su_t_t0__d_d2 @ t_t0) + sc_g1 * (su_t_t0__d_g1 @ t_t0) + sc_d2 * (su_t_t1__d_d2 @ t_t1) + sc_g1 * (su_t_t1__d_g1 @ t_t1) + sc_m2 * (su_t_t1__d_m2 @ t_t1) */
void fom_step_su(const fom_float *state, const fom_float *t_t0, const fom_float *t_t1, fom_float sc_d1, fom_float sc_g2, fom_float sc_m1, fom_float sc_d2, fom_float sc_g1, fom_float sc_m2, const fom_float *bias, fom_float *out);

/* The same, on every row but the block's selection rows (FOM_SELROWS_<block>
   in fom_data.h), which it leaves untouched: the solver writes those copies. */
void fom_fused_su__rest(const fom_float *state, const fom_float *t_t0, const fom_float *t_t1, fom_float sc_d1, fom_float sc_g2, fom_float sc_m1, fom_float sc_d2, fom_float sc_g1, fom_float sc_m2, fom_float *out);
void fom_step_su__rest(const fom_float *state, const fom_float *t_t0, const fom_float *t_t1, fom_float sc_d1, fom_float sc_g2, fom_float sc_m1, fom_float sc_d2, fom_float sc_g1, fom_float sc_m2, const fom_float *bias, fom_float *out);

/* One whole iteration: every body step in order (temporaries written to
   t_<name>), then the state update into new_state. Checked against the
   chained reference steps, temporaries included. */
void fom_iterate(const fom_float *state, fom_float sc_taux, fom_float sc_tth, fom_float sc_eta, fom_float sc_d1, fom_float sc_g2, fom_float sc_m1, fom_float sc_d2, fom_float sc_g1, fom_float sc_m2, const fom_float *bias_t0, const fom_float *bias_t1, const fom_float *bias_su, const FomAct_t0 *act_t0, const FomAct_t1 *act_t1, fom_float *t_t0, fom_float *t_t1, fom_float *new_state);

#ifdef __cplusplus
}
#endif

#endif /* FOM_OPS_H */
