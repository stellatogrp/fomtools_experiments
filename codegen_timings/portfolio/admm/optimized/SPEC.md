# FOM specification: `portfolio_admm`

One iteration of a first-order method, to be implemented as a fixed loop.

- dtype: `float64`
- state dimension: 51
- termination: max_iters (max_iters=200, tol=None, check_termination=1)

## Parameters (supplied at solve time)
- `p0`: dimension 10

## State layout

- `s0`: dimension 51

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

### `t0` (dim 10)
- linear_op (rhs) = t0_state @ state
- activation = clip(., lower=t0_lo, upper=t0_hi)  (elementwise; bounds are vectors)

### `t1` (dim 10)
- linear_op (rhs) = t1_state @ state + t1_p_p0 @ param:p0
- activation = relu

### `t2` (dim 10)
- linear_op (rhs) = t2_state @ state + t2_p_p0 @ param:p0
- activation = relu

### `t3` (dim 20)
- linear_op (rhs) = t3_c + t3_state @ state + t3_p_p0 @ param:p0 + t3_t_t0 @ tmp:t0 + t3_t_t1 @ tmp:t1 + t3_t_t2 @ tmp:t2
- solve: `t3 = fom_lin_solve_t3(rhs)` (LDLT factorization, dim 20, nnz 65)

## State update
- `state <- su_c + su_state @ state + su_t_t0 @ tmp:t0 + su_t_t1 @ tmp:t1 + su_t_t2 @ tmp:t2 + su_t_t3 @ tmp:t3 + su_p_p0 @ param:p0`

Residual = `max_abs(new_state - state)`.

All matrices/consts are provided numerically in `data.npz` / `fom_data.c`.