# FOM specification: `rapdhg_pdhg`

One iteration of a first-order method, to be implemented as a fixed loop.

- dtype: `float64`
- state dimension: 160
- termination: max_iters (max_iters=200, tol=None, check_termination=1)

## Parameters (supplied at solve time)
- `p0`: dimension 16
- `p1`: dimension 48

## State layout

- `s0`: dimension 16
- `s1`: dimension 48
- `s2`: dimension 16
- `s3`: dimension 48
- `s4`: dimension 16
- `s5`: dimension 16

## Iteration-varying coefficients

Some terms below are scaled by a coefficient that changes with the
iteration. Each is tabulated in `fom_data.c`, one value per iteration,
and read through the accessor declared in `fom_data.h`:

- `fom_sched_d1(it)` — 200 values, starting 0, 0, 0, 0, ...
- `fom_sched_d2(it)` — 200 values, starting 1, 1, 1, 1, ...
- `fom_sched_eta(it)` — 200 values, starting 0.0188453, 0.0188453, 0.0188453, 0.0188453, ...
- `fom_sched_g1(it)` — 200 values, starting 1, 1, 1, 1, ...
- `fom_sched_g2(it)` — 200 values, starting 0, 0, 0, 0, ...
- `fom_sched_m1(it)` — 200 values, starting 0, 0, 0, 0, ...
- `fom_sched_m2(it)` — 200 values, starting 1, 1, 1, 1, ...
- `fom_sched_tau(it)` — 200 values, starting 0.462361, 0.462361, 0.462361, 0.462361, ...
- `fom_sched_taux(it)` — 200 values, starting 0.924722, 0.924722, 0.924722, 0.924722, ...
- `fom_sched_tth(it)` — 200 values, starting 0.462361, 0.462361, 0.462361, 0.462361, ...

`it` is the **0-based** loop counter. Always call the accessor rather
than indexing `FOM_SCHED_*` directly: the loop bound is the runtime
`work->settings.max_iter`, which a caller may raise above the table
length, and the accessor clamps. Hoist the value into a local at the
top of each iteration; it does not change within one iteration.

## Iteration body

Each step computes `tmp = activation(linear_op)`, except affine
steps which *solve* `M tmp = linear_op`. Always call the provided
`fom_lin_solve_<name>` routine in `fom_linsolve.c`; never form or use
a matrix inverse. The routine uses either a precomputed sparse direct
factorization (LDL^T / LU) or, for CG, preconditioned conjugate
gradient (sparse matrix-vector products, no factorization) — the
calling convention is identical either way.

Compute every `matrix @ source` via the accumulating helpers in
`fom_ops.c`: zero the output, then call `fom_apply_<matrix>(source, out)`
for each term (these are sparse CSR where the matrix is structured, so
do not write your own dense matrix-vector loops):

### `t0` (dim 48)
- linear_op (rhs) = t0_state @ state + fom_sched_taux(it) * (t0_state__d_taux @ state) + fom_sched_tth(it) * (t0_state__d_tth @ state) + t0_p_p1 @ param:p1 + fom_sched_tau(it) * (t0_p_p1__d_tau @ param:p1)
- activation = relu

### `t1` (dim 16)
- linear_op (rhs) = t1_state @ state + fom_sched_eta(it) * (t1_state__d_eta @ state) + t1_p_p0 @ param:p0 + fom_sched_eta(it) * (t1_p_p0__d_eta @ param:p0) + t1_t_t0 @ tmp:t0 + fom_sched_eta(it) * (t1_t_t0__d_eta @ tmp:t0)
- activation = clip(., lower=t1_lo, upper=t1_hi)  (elementwise; bounds are vectors)

## State update
- `state <- su_state @ state + fom_sched_d1(it) * (su_state__d_d1 @ state) + fom_sched_g2(it) * (su_state__d_g2 @ state) + fom_sched_m1(it) * (su_state__d_m1 @ state) + su_t_t0 @ tmp:t0 + fom_sched_d2(it) * (su_t_t0__d_d2 @ tmp:t0) + fom_sched_g1(it) * (su_t_t0__d_g1 @ tmp:t0) + su_t_t1 @ tmp:t1 + fom_sched_d2(it) * (su_t_t1__d_d2 @ tmp:t1) + fom_sched_g1(it) * (su_t_t1__d_g1 @ tmp:t1) + fom_sched_m2(it) * (su_t_t1__d_m2 @ tmp:t1)`

Residual = `max_abs(new_state - state)`.

All matrices/consts are provided numerically in `data.npz` / `fom_data.c`.