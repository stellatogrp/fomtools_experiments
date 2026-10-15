# FOM specification: `lasso_ista`

One iteration of a first-order method, to be implemented as a fixed loop.

- dtype: `float64`
- state dimension: 15
- termination: max_iters (max_iters=200, tol=None, check_termination=1)

## Parameters (supplied at solve time)
- `p0`: dimension 10

## State layout

- `s0`: dimension 15

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

### `t0` (dim 15)
- linear_op (rhs) = t0_state @ state + t0_p_p0 @ param:p0
- activation = soft_threshold(., lambda=0.029093587409024832)

## State update
- `state <- su_t_t0 @ tmp:t0`

Residual = `max_abs(new_state - state)`.

All matrices/consts are provided numerically in `data.npz` / `fom_data.c`.