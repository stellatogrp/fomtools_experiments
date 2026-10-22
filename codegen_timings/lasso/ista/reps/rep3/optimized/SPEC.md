# FOM specification: `lasso_ista`

One iteration of a first-order method, to be implemented as a fixed loop.

- dtype: `float64`
- state dimension: 50
- termination: max_iters (max_iters=200, tol=None, check_termination=1)

## Parameters (supplied at solve time)
- `p0`: dimension 25

## State layout

- `s0`: dimension 50

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

### `t0` (dim 50)
- linear_op (rhs) = t0_state @ state + t0_p_p0 @ param:p0
- activation = soft_threshold(., lambda=0.0002212861544693803)

## State update
- `state <- su_t_t0 @ tmp:t0`

Residual = `max_abs(new_state - state)`.

## Selection matrices

These products are copies: each matrix has at most one nonzero per row, and
every nonzero is exactly 1, so `(M @ x)[i] = x[src[i]]`. On the CPU targets
`fom_data.h` carries each map as `FOM_SEL_<name>_SRC` (-1 for an empty row).

- `su_t_t0`: identity

## Selection rows

Rows of an output block that are a plain copy: exactly one unscaled product
has exactly one entry there, equal to 1, and the others none. On the CPU
targets `fom_data.h` lists them as `FOM_SELROWS_<block>_ROW/SRC/COL`, and a
`fom_fused_<block>__rest` / `fom_step_<block>__rest` computes the other rows.

- `su` (50 of 50 rows): rows 0..49 copy t_t0[0..49]

All matrices/consts are provided numerically in `data.npz` / `fom_data.c`.