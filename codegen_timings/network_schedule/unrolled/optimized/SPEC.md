# FOM specification: `network_unrolled`

The whole run of a first-order method, to be implemented as a single pass.

- dtype: `float64`
- state dimension: 650
- termination: max_iters (max_iters=1, tol=None, check_termination=1)

## This specification is unrolled

The steps below are **not** one iteration to repeat. They are the entire
recorded run, already in dependency order, and each carries its own
coefficient blocks because the method does something different on each
iteration. So:

- Do **not** write a loop over iterations. Execute the steps once, in the
  order given, then apply the state update to produce the answer.
- Ignore `work->settings.max_iter`. `FOM_MAX_ITER` is 1 and
  `FOM_UNROLLED` is 1 in `fom_workspace.h`; running the body more than once
  would run the algorithm more than once. The verify harness raises
  `max_iter` and requires the answer not to change.
- There is no residual-based early exit: with a single pass there is no
  previous iterate to compare against. Report `FOM_MAX_ITER_REACHED`.

Repeated blocks are shared rather than duplicated, so several steps may name
the same matrix. Where consecutive steps do turn out to be identical you may
implement them with a loop if that is faster, provided the generated code
still verifies -- that is an implementation choice, not a change to this
specification.

## Parameters (supplied at solve time)
- `p0`: dimension 30

## State layout

- `s0`: dimension 600
- `s1`: dimension 20
- `s2`: dimension 30

## Body (executed once, in order)

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

### `t0` (dim 600)
- linear_op (rhs) = t0_c + t0_state @ state
- activation = clip(., lower=t0_lo, upper=t0_hi)  (elementwise; bounds are vectors)

### `t1` (dim 20)
- linear_op (rhs) = t1_c + t1_state @ state + t1_t_t0 @ tmp:t0
- activation = relu

### `t2` (dim 600)
- linear_op (rhs) = t0_c + t2_state @ state + t2_p_p0 @ param:p0 + t2_t_t0 @ tmp:t0 + t2_t_t1 @ tmp:t1
- activation = clip(., lower=t0_lo, upper=t0_hi)  (elementwise; bounds are vectors)

### `t3` (dim 20)
- linear_op (rhs) = t1_c + t3_t_t0 @ tmp:t0 + t3_t_t1 @ tmp:t1 + t3_t_t2 @ tmp:t2
- activation = relu

### `t4` (dim 30)
- linear_op (rhs) = t4_state @ state + t4_p_p0 @ param:p0 + t4_t_t0 @ tmp:t0 + t4_t_t2 @ tmp:t2
- activation = identity

### `t5` (dim 600)
- linear_op (rhs) = t0_c + t5_t_t2 @ tmp:t2 + t2_t_t1 @ tmp:t3 + t5_t_t4 @ tmp:t4
- activation = clip(., lower=t0_lo, upper=t0_hi)  (elementwise; bounds are vectors)

### `t6` (dim 20)
- linear_op (rhs) = t1_c + t6_t_t2 @ tmp:t2 + t3_t_t1 @ tmp:t3 + t6_t_t5 @ tmp:t5
- activation = relu

### `t7` (dim 30)
- linear_op (rhs) = t7_p_p0 @ param:p0 + t7_t_t2 @ tmp:t2 + t7_t_t4 @ tmp:t4 + t7_t_t5 @ tmp:t5
- activation = identity

### `t8` (dim 600)
- linear_op (rhs) = t0_c + t5_t_t2 @ tmp:t5 + t2_t_t1 @ tmp:t6 + t5_t_t4 @ tmp:t7
- activation = clip(., lower=t0_lo, upper=t0_hi)  (elementwise; bounds are vectors)

### `t9` (dim 20)
- linear_op (rhs) = t1_c + t9_t_t5 @ tmp:t5 + t3_t_t1 @ tmp:t6 + t9_t_t8 @ tmp:t8
- activation = relu

### `t10` (dim 30)
- linear_op (rhs) = t7_p_p0 @ param:p0 + t10_t_t5 @ tmp:t5 + t7_t_t4 @ tmp:t7 + t10_t_t8 @ tmp:t8
- activation = identity

### `t11` (dim 600)
- linear_op (rhs) = t0_c + t5_t_t2 @ tmp:t8 + t2_t_t1 @ tmp:t9 + t5_t_t4 @ tmp:t10
- activation = clip(., lower=t0_lo, upper=t0_hi)  (elementwise; bounds are vectors)

### `t12` (dim 20)
- linear_op (rhs) = t1_c + t12_t_t8 @ tmp:t8 + t3_t_t1 @ tmp:t9 + t12_t_t11 @ tmp:t11
- activation = relu

### `t13` (dim 30)
- linear_op (rhs) = t7_p_p0 @ param:p0 + t13_t_t8 @ tmp:t8 + t7_t_t4 @ tmp:t10 + t13_t_t11 @ tmp:t11
- activation = identity

### `t14` (dim 600)
- linear_op (rhs) = t0_c + t5_t_t2 @ tmp:t11 + t2_t_t1 @ tmp:t12 + t5_t_t4 @ tmp:t13
- activation = clip(., lower=t0_lo, upper=t0_hi)  (elementwise; bounds are vectors)

### `t15` (dim 20)
- linear_op (rhs) = t1_c + t15_t_t11 @ tmp:t11 + t3_t_t1 @ tmp:t12 + t15_t_t14 @ tmp:t14
- activation = relu

### `t16` (dim 30)
- linear_op (rhs) = t7_p_p0 @ param:p0 + t16_t_t11 @ tmp:t11 + t7_t_t4 @ tmp:t13 + t16_t_t14 @ tmp:t14
- activation = identity

### `t17` (dim 600)
- linear_op (rhs) = t0_c + t5_t_t2 @ tmp:t14 + t2_t_t1 @ tmp:t15 + t5_t_t4 @ tmp:t16
- activation = clip(., lower=t0_lo, upper=t0_hi)  (elementwise; bounds are vectors)

### `t18` (dim 20)
- linear_op (rhs) = t1_c + t18_t_t14 @ tmp:t14 + t3_t_t1 @ tmp:t15 + t18_t_t17 @ tmp:t17
- activation = relu

### `t19` (dim 30)
- linear_op (rhs) = t7_p_p0 @ param:p0 + t19_t_t14 @ tmp:t14 + t7_t_t4 @ tmp:t16 + t19_t_t17 @ tmp:t17
- activation = identity

### `t20` (dim 600)
- linear_op (rhs) = t0_c + t5_t_t2 @ tmp:t17 + t2_t_t1 @ tmp:t18 + t5_t_t4 @ tmp:t19
- activation = clip(., lower=t0_lo, upper=t0_hi)  (elementwise; bounds are vectors)

### `t21` (dim 20)
- linear_op (rhs) = t1_c + t21_t_t17 @ tmp:t17 + t3_t_t1 @ tmp:t18 + t21_t_t20 @ tmp:t20
- activation = relu

### `t22` (dim 30)
- linear_op (rhs) = t7_p_p0 @ param:p0 + t22_t_t17 @ tmp:t17 + t7_t_t4 @ tmp:t19 + t22_t_t20 @ tmp:t20
- activation = identity

### `t23` (dim 600)
- linear_op (rhs) = t0_c + t5_t_t2 @ tmp:t20 + t2_t_t1 @ tmp:t21 + t5_t_t4 @ tmp:t22
- activation = clip(., lower=t0_lo, upper=t0_hi)  (elementwise; bounds are vectors)

### `t24` (dim 20)
- linear_op (rhs) = t1_c + t24_t_t20 @ tmp:t20 + t3_t_t1 @ tmp:t21 + t24_t_t23 @ tmp:t23
- activation = relu

### `t25` (dim 30)
- linear_op (rhs) = t7_p_p0 @ param:p0 + t25_t_t20 @ tmp:t20 + t7_t_t4 @ tmp:t22 + t25_t_t23 @ tmp:t23
- activation = identity

### `t26` (dim 600)
- linear_op (rhs) = t0_c + t5_t_t2 @ tmp:t23 + t2_t_t1 @ tmp:t24 + t5_t_t4 @ tmp:t25
- activation = clip(., lower=t0_lo, upper=t0_hi)  (elementwise; bounds are vectors)

### `t27` (dim 20)
- linear_op (rhs) = t1_c + t27_t_t23 @ tmp:t23 + t3_t_t1 @ tmp:t24 + t27_t_t26 @ tmp:t26
- activation = relu

### `t28` (dim 30)
- linear_op (rhs) = t7_p_p0 @ param:p0 + t28_t_t23 @ tmp:t23 + t7_t_t4 @ tmp:t25 + t28_t_t26 @ tmp:t26
- activation = identity

### `t29` (dim 600)
- linear_op (rhs) = t0_c + t5_t_t2 @ tmp:t26 + t2_t_t1 @ tmp:t27 + t5_t_t4 @ tmp:t28
- activation = clip(., lower=t0_lo, upper=t0_hi)  (elementwise; bounds are vectors)

### `t30` (dim 20)
- linear_op (rhs) = t1_c + t30_t_t26 @ tmp:t26 + t3_t_t1 @ tmp:t27 + t30_t_t29 @ tmp:t29
- activation = relu

### `t31` (dim 30)
- linear_op (rhs) = t7_p_p0 @ param:p0 + t31_t_t26 @ tmp:t26 + t7_t_t4 @ tmp:t28 + t31_t_t29 @ tmp:t29
- activation = identity

### `t32` (dim 600)
- linear_op (rhs) = t0_c + t5_t_t2 @ tmp:t29 + t2_t_t1 @ tmp:t30 + t5_t_t4 @ tmp:t31
- activation = clip(., lower=t0_lo, upper=t0_hi)  (elementwise; bounds are vectors)

### `t33` (dim 20)
- linear_op (rhs) = t1_c + t33_t_t29 @ tmp:t29 + t3_t_t1 @ tmp:t30 + t33_t_t32 @ tmp:t32
- activation = relu

### `t34` (dim 30)
- linear_op (rhs) = t7_p_p0 @ param:p0 + t34_t_t29 @ tmp:t29 + t7_t_t4 @ tmp:t31 + t34_t_t32 @ tmp:t32
- activation = identity

### `t35` (dim 600)
- linear_op (rhs) = t0_c + t5_t_t2 @ tmp:t32 + t2_t_t1 @ tmp:t33 + t5_t_t4 @ tmp:t34
- activation = clip(., lower=t0_lo, upper=t0_hi)  (elementwise; bounds are vectors)

### `t36` (dim 20)
- linear_op (rhs) = t1_c + t36_t_t32 @ tmp:t32 + t3_t_t1 @ tmp:t33 + t36_t_t35 @ tmp:t35
- activation = relu

### `t37` (dim 30)
- linear_op (rhs) = t7_p_p0 @ param:p0 + t37_t_t32 @ tmp:t32 + t7_t_t4 @ tmp:t34 + t37_t_t35 @ tmp:t35
- activation = identity

### `t38` (dim 600)
- linear_op (rhs) = t0_c + t5_t_t2 @ tmp:t35 + t2_t_t1 @ tmp:t36 + t5_t_t4 @ tmp:t37
- activation = clip(., lower=t0_lo, upper=t0_hi)  (elementwise; bounds are vectors)

### `t39` (dim 20)
- linear_op (rhs) = t1_c + t39_t_t35 @ tmp:t35 + t3_t_t1 @ tmp:t36 + t39_t_t38 @ tmp:t38
- activation = relu

### `t40` (dim 30)
- linear_op (rhs) = t7_p_p0 @ param:p0 + t40_t_t35 @ tmp:t35 + t7_t_t4 @ tmp:t37 + t40_t_t38 @ tmp:t38
- activation = identity

### `t41` (dim 600)
- linear_op (rhs) = t0_c + t5_t_t2 @ tmp:t38 + t2_t_t1 @ tmp:t39 + t5_t_t4 @ tmp:t40
- activation = clip(., lower=t0_lo, upper=t0_hi)  (elementwise; bounds are vectors)

### `t42` (dim 20)
- linear_op (rhs) = t1_c + t42_t_t38 @ tmp:t38 + t3_t_t1 @ tmp:t39 + t42_t_t41 @ tmp:t41
- activation = relu

### `t43` (dim 30)
- linear_op (rhs) = t7_p_p0 @ param:p0 + t43_t_t38 @ tmp:t38 + t7_t_t4 @ tmp:t40 + t43_t_t41 @ tmp:t41
- activation = identity

### `t44` (dim 600)
- linear_op (rhs) = t0_c + t5_t_t2 @ tmp:t41 + t2_t_t1 @ tmp:t42 + t5_t_t4 @ tmp:t43
- activation = clip(., lower=t0_lo, upper=t0_hi)  (elementwise; bounds are vectors)

### `t45` (dim 20)
- linear_op (rhs) = t1_c + t45_t_t41 @ tmp:t41 + t3_t_t1 @ tmp:t42 + t45_t_t44 @ tmp:t44
- activation = relu

### `t46` (dim 30)
- linear_op (rhs) = t7_p_p0 @ param:p0 + t46_t_t41 @ tmp:t41 + t7_t_t4 @ tmp:t43 + t46_t_t44 @ tmp:t44
- activation = identity

### `t47` (dim 600)
- linear_op (rhs) = t0_c + t5_t_t2 @ tmp:t44 + t2_t_t1 @ tmp:t45 + t5_t_t4 @ tmp:t46
- activation = clip(., lower=t0_lo, upper=t0_hi)  (elementwise; bounds are vectors)

### `t48` (dim 20)
- linear_op (rhs) = t1_c + t48_t_t44 @ tmp:t44 + t3_t_t1 @ tmp:t45 + t48_t_t47 @ tmp:t47
- activation = relu

### `t49` (dim 30)
- linear_op (rhs) = t7_p_p0 @ param:p0 + t49_t_t44 @ tmp:t44 + t7_t_t4 @ tmp:t46 + t49_t_t47 @ tmp:t47
- activation = identity

### `t50` (dim 600)
- linear_op (rhs) = t0_c + t5_t_t2 @ tmp:t47 + t2_t_t1 @ tmp:t48 + t5_t_t4 @ tmp:t49
- activation = clip(., lower=t0_lo, upper=t0_hi)  (elementwise; bounds are vectors)

### `t51` (dim 20)
- linear_op (rhs) = t1_c + t51_t_t47 @ tmp:t47 + t3_t_t1 @ tmp:t48 + t51_t_t50 @ tmp:t50
- activation = relu

### `t52` (dim 30)
- linear_op (rhs) = t7_p_p0 @ param:p0 + t52_t_t47 @ tmp:t47 + t7_t_t4 @ tmp:t49 + t52_t_t50 @ tmp:t50
- activation = identity

### `t53` (dim 600)
- linear_op (rhs) = t0_c + t5_t_t2 @ tmp:t50 + t2_t_t1 @ tmp:t51 + t5_t_t4 @ tmp:t52
- activation = clip(., lower=t0_lo, upper=t0_hi)  (elementwise; bounds are vectors)

### `t54` (dim 20)
- linear_op (rhs) = t1_c + t54_t_t50 @ tmp:t50 + t3_t_t1 @ tmp:t51 + t54_t_t53 @ tmp:t53
- activation = relu

### `t55` (dim 30)
- linear_op (rhs) = t7_p_p0 @ param:p0 + t55_t_t50 @ tmp:t50 + t7_t_t4 @ tmp:t52 + t55_t_t53 @ tmp:t53
- activation = identity

### `t56` (dim 600)
- linear_op (rhs) = t0_c + t5_t_t2 @ tmp:t53 + t2_t_t1 @ tmp:t54 + t5_t_t4 @ tmp:t55
- activation = clip(., lower=t0_lo, upper=t0_hi)  (elementwise; bounds are vectors)

### `t57` (dim 20)
- linear_op (rhs) = t1_c + t57_t_t53 @ tmp:t53 + t3_t_t1 @ tmp:t54 + t57_t_t56 @ tmp:t56
- activation = relu

### `t58` (dim 30)
- linear_op (rhs) = t7_p_p0 @ param:p0 + t58_t_t53 @ tmp:t53 + t7_t_t4 @ tmp:t55 + t58_t_t56 @ tmp:t56
- activation = identity

### `t59` (dim 600)
- linear_op (rhs) = t0_c + t5_t_t2 @ tmp:t56 + t2_t_t1 @ tmp:t57 + t5_t_t4 @ tmp:t58
- activation = clip(., lower=t0_lo, upper=t0_hi)  (elementwise; bounds are vectors)

### `t60` (dim 20)
- linear_op (rhs) = t1_c + t60_t_t56 @ tmp:t56 + t3_t_t1 @ tmp:t57 + t60_t_t59 @ tmp:t59
- activation = relu

### `t61` (dim 30)
- linear_op (rhs) = t7_p_p0 @ param:p0 + t61_t_t56 @ tmp:t56 + t7_t_t4 @ tmp:t58 + t61_t_t59 @ tmp:t59
- activation = identity

### `t62` (dim 600)
- linear_op (rhs) = t0_c + t5_t_t2 @ tmp:t59 + t2_t_t1 @ tmp:t60 + t5_t_t4 @ tmp:t61
- activation = clip(., lower=t0_lo, upper=t0_hi)  (elementwise; bounds are vectors)

### `t63` (dim 20)
- linear_op (rhs) = t1_c + t63_t_t59 @ tmp:t59 + t3_t_t1 @ tmp:t60 + t63_t_t62 @ tmp:t62
- activation = relu

### `t64` (dim 30)
- linear_op (rhs) = t7_p_p0 @ param:p0 + t64_t_t59 @ tmp:t59 + t7_t_t4 @ tmp:t61 + t64_t_t62 @ tmp:t62
- activation = identity

### `t65` (dim 600)
- linear_op (rhs) = t0_c + t5_t_t2 @ tmp:t62 + t2_t_t1 @ tmp:t63 + t5_t_t4 @ tmp:t64
- activation = clip(., lower=t0_lo, upper=t0_hi)  (elementwise; bounds are vectors)

### `t66` (dim 20)
- linear_op (rhs) = t1_c + t66_t_t62 @ tmp:t62 + t3_t_t1 @ tmp:t63 + t66_t_t65 @ tmp:t65
- activation = relu

### `t67` (dim 30)
- linear_op (rhs) = t7_p_p0 @ param:p0 + t67_t_t62 @ tmp:t62 + t7_t_t4 @ tmp:t64 + t67_t_t65 @ tmp:t65
- activation = identity

### `t68` (dim 600)
- linear_op (rhs) = t0_c + t5_t_t2 @ tmp:t65 + t2_t_t1 @ tmp:t66 + t5_t_t4 @ tmp:t67
- activation = clip(., lower=t0_lo, upper=t0_hi)  (elementwise; bounds are vectors)

### `t69` (dim 20)
- linear_op (rhs) = t1_c + t69_t_t65 @ tmp:t65 + t3_t_t1 @ tmp:t66 + t69_t_t68 @ tmp:t68
- activation = relu

### `t70` (dim 30)
- linear_op (rhs) = t7_p_p0 @ param:p0 + t70_t_t65 @ tmp:t65 + t7_t_t4 @ tmp:t67 + t70_t_t68 @ tmp:t68
- activation = identity

### `t71` (dim 600)
- linear_op (rhs) = t0_c + t5_t_t2 @ tmp:t68 + t2_t_t1 @ tmp:t69 + t5_t_t4 @ tmp:t70
- activation = clip(., lower=t0_lo, upper=t0_hi)  (elementwise; bounds are vectors)

### `t72` (dim 20)
- linear_op (rhs) = t1_c + t72_t_t68 @ tmp:t68 + t3_t_t1 @ tmp:t69 + t72_t_t71 @ tmp:t71
- activation = relu

### `t73` (dim 30)
- linear_op (rhs) = t7_p_p0 @ param:p0 + t73_t_t68 @ tmp:t68 + t7_t_t4 @ tmp:t70 + t73_t_t71 @ tmp:t71
- activation = identity

### `t74` (dim 600)
- linear_op (rhs) = t0_c + t5_t_t2 @ tmp:t71 + t2_t_t1 @ tmp:t72 + t5_t_t4 @ tmp:t73
- activation = clip(., lower=t0_lo, upper=t0_hi)  (elementwise; bounds are vectors)

### `t75` (dim 20)
- linear_op (rhs) = t1_c + t75_t_t71 @ tmp:t71 + t3_t_t1 @ tmp:t72 + t75_t_t74 @ tmp:t74
- activation = relu

### `t76` (dim 30)
- linear_op (rhs) = t7_p_p0 @ param:p0 + t76_t_t71 @ tmp:t71 + t7_t_t4 @ tmp:t73 + t76_t_t74 @ tmp:t74
- activation = identity

### `t77` (dim 600)
- linear_op (rhs) = t0_c + t5_t_t2 @ tmp:t74 + t2_t_t1 @ tmp:t75 + t5_t_t4 @ tmp:t76
- activation = clip(., lower=t0_lo, upper=t0_hi)  (elementwise; bounds are vectors)

### `t78` (dim 20)
- linear_op (rhs) = t1_c + t78_t_t74 @ tmp:t74 + t3_t_t1 @ tmp:t75 + t78_t_t77 @ tmp:t77
- activation = relu

### `t79` (dim 30)
- linear_op (rhs) = t7_p_p0 @ param:p0 + t79_t_t74 @ tmp:t74 + t7_t_t4 @ tmp:t76 + t79_t_t77 @ tmp:t77
- activation = identity

### `t80` (dim 600)
- linear_op (rhs) = t0_c + t5_t_t2 @ tmp:t77 + t2_t_t1 @ tmp:t78 + t5_t_t4 @ tmp:t79
- activation = clip(., lower=t0_lo, upper=t0_hi)  (elementwise; bounds are vectors)

### `t81` (dim 20)
- linear_op (rhs) = t1_c + t81_t_t77 @ tmp:t77 + t3_t_t1 @ tmp:t78 + t81_t_t80 @ tmp:t80
- activation = relu

### `t82` (dim 30)
- linear_op (rhs) = t7_p_p0 @ param:p0 + t82_t_t77 @ tmp:t77 + t7_t_t4 @ tmp:t79 + t82_t_t80 @ tmp:t80
- activation = identity

### `t83` (dim 600)
- linear_op (rhs) = t0_c + t5_t_t2 @ tmp:t80 + t2_t_t1 @ tmp:t81 + t5_t_t4 @ tmp:t82
- activation = clip(., lower=t0_lo, upper=t0_hi)  (elementwise; bounds are vectors)

### `t84` (dim 20)
- linear_op (rhs) = t1_c + t84_t_t80 @ tmp:t80 + t3_t_t1 @ tmp:t81 + t84_t_t83 @ tmp:t83
- activation = relu

### `t85` (dim 30)
- linear_op (rhs) = t7_p_p0 @ param:p0 + t85_t_t80 @ tmp:t80 + t7_t_t4 @ tmp:t82 + t85_t_t83 @ tmp:t83
- activation = identity

### `t86` (dim 600)
- linear_op (rhs) = t0_c + t5_t_t2 @ tmp:t83 + t2_t_t1 @ tmp:t84 + t5_t_t4 @ tmp:t85
- activation = clip(., lower=t0_lo, upper=t0_hi)  (elementwise; bounds are vectors)

### `t87` (dim 20)
- linear_op (rhs) = t1_c + t87_t_t83 @ tmp:t83 + t3_t_t1 @ tmp:t84 + t87_t_t86 @ tmp:t86
- activation = relu

### `t88` (dim 30)
- linear_op (rhs) = t7_p_p0 @ param:p0 + t88_t_t83 @ tmp:t83 + t7_t_t4 @ tmp:t85 + t88_t_t86 @ tmp:t86
- activation = identity

### `t89` (dim 600)
- linear_op (rhs) = t0_c + t5_t_t2 @ tmp:t86 + t2_t_t1 @ tmp:t87 + t5_t_t4 @ tmp:t88
- activation = clip(., lower=t0_lo, upper=t0_hi)  (elementwise; bounds are vectors)

### `t90` (dim 20)
- linear_op (rhs) = t1_c + t90_t_t86 @ tmp:t86 + t3_t_t1 @ tmp:t87 + t90_t_t89 @ tmp:t89
- activation = relu

### `t91` (dim 30)
- linear_op (rhs) = t7_p_p0 @ param:p0 + t91_t_t86 @ tmp:t86 + t7_t_t4 @ tmp:t88 + t91_t_t89 @ tmp:t89
- activation = identity

### `t92` (dim 600)
- linear_op (rhs) = t0_c + t5_t_t2 @ tmp:t89 + t2_t_t1 @ tmp:t90 + t5_t_t4 @ tmp:t91
- activation = clip(., lower=t0_lo, upper=t0_hi)  (elementwise; bounds are vectors)

### `t93` (dim 20)
- linear_op (rhs) = t1_c + t93_t_t89 @ tmp:t89 + t3_t_t1 @ tmp:t90 + t93_t_t92 @ tmp:t92
- activation = relu

### `t94` (dim 30)
- linear_op (rhs) = t7_p_p0 @ param:p0 + t94_t_t89 @ tmp:t89 + t7_t_t4 @ tmp:t91 + t94_t_t92 @ tmp:t92
- activation = identity

### `t95` (dim 600)
- linear_op (rhs) = t0_c + t5_t_t2 @ tmp:t92 + t2_t_t1 @ tmp:t93 + t5_t_t4 @ tmp:t94
- activation = clip(., lower=t0_lo, upper=t0_hi)  (elementwise; bounds are vectors)

### `t96` (dim 20)
- linear_op (rhs) = t1_c + t96_t_t92 @ tmp:t92 + t3_t_t1 @ tmp:t93 + t96_t_t95 @ tmp:t95
- activation = relu

### `t97` (dim 30)
- linear_op (rhs) = t7_p_p0 @ param:p0 + t97_t_t92 @ tmp:t92 + t7_t_t4 @ tmp:t94 + t97_t_t95 @ tmp:t95
- activation = identity

### `t98` (dim 600)
- linear_op (rhs) = t0_c + t5_t_t2 @ tmp:t95 + t2_t_t1 @ tmp:t96 + t5_t_t4 @ tmp:t97
- activation = clip(., lower=t0_lo, upper=t0_hi)  (elementwise; bounds are vectors)

### `t99` (dim 20)
- linear_op (rhs) = t1_c + t99_t_t95 @ tmp:t95 + t3_t_t1 @ tmp:t96 + t99_t_t98 @ tmp:t98
- activation = relu

### `t100` (dim 30)
- linear_op (rhs) = t7_p_p0 @ param:p0 + t100_t_t95 @ tmp:t95 + t7_t_t4 @ tmp:t97 + t100_t_t98 @ tmp:t98
- activation = identity

### `t101` (dim 600)
- linear_op (rhs) = t0_c + t5_t_t2 @ tmp:t98 + t2_t_t1 @ tmp:t99 + t5_t_t4 @ tmp:t100
- activation = clip(., lower=t0_lo, upper=t0_hi)  (elementwise; bounds are vectors)

### `t102` (dim 20)
- linear_op (rhs) = t1_c + t102_t_t98 @ tmp:t98 + t3_t_t1 @ tmp:t99 + t102_t_t101 @ tmp:t101
- activation = relu

### `t103` (dim 30)
- linear_op (rhs) = t7_p_p0 @ param:p0 + t103_t_t98 @ tmp:t98 + t7_t_t4 @ tmp:t100 + t103_t_t101 @ tmp:t101
- activation = identity

### `t104` (dim 600)
- linear_op (rhs) = t0_c + t5_t_t2 @ tmp:t101 + t2_t_t1 @ tmp:t102 + t5_t_t4 @ tmp:t103
- activation = clip(., lower=t0_lo, upper=t0_hi)  (elementwise; bounds are vectors)

### `t105` (dim 20)
- linear_op (rhs) = t1_c + t105_t_t101 @ tmp:t101 + t3_t_t1 @ tmp:t102 + t105_t_t104 @ tmp:t104
- activation = relu

### `t106` (dim 30)
- linear_op (rhs) = t7_p_p0 @ param:p0 + t106_t_t101 @ tmp:t101 + t7_t_t4 @ tmp:t103 + t106_t_t104 @ tmp:t104
- activation = identity

### `t107` (dim 600)
- linear_op (rhs) = t0_c + t5_t_t2 @ tmp:t104 + t2_t_t1 @ tmp:t105 + t5_t_t4 @ tmp:t106
- activation = clip(., lower=t0_lo, upper=t0_hi)  (elementwise; bounds are vectors)

### `t108` (dim 20)
- linear_op (rhs) = t1_c + t108_t_t104 @ tmp:t104 + t3_t_t1 @ tmp:t105 + t108_t_t107 @ tmp:t107
- activation = relu

### `t109` (dim 30)
- linear_op (rhs) = t7_p_p0 @ param:p0 + t109_t_t104 @ tmp:t104 + t7_t_t4 @ tmp:t106 + t109_t_t107 @ tmp:t107
- activation = identity

### `t110` (dim 600)
- linear_op (rhs) = t0_c + t5_t_t2 @ tmp:t107 + t2_t_t1 @ tmp:t108 + t5_t_t4 @ tmp:t109
- activation = clip(., lower=t0_lo, upper=t0_hi)  (elementwise; bounds are vectors)

### `t111` (dim 20)
- linear_op (rhs) = t1_c + t111_t_t107 @ tmp:t107 + t3_t_t1 @ tmp:t108 + t111_t_t110 @ tmp:t110
- activation = relu

### `t112` (dim 30)
- linear_op (rhs) = t7_p_p0 @ param:p0 + t112_t_t107 @ tmp:t107 + t7_t_t4 @ tmp:t109 + t112_t_t110 @ tmp:t110
- activation = identity

### `t113` (dim 600)
- linear_op (rhs) = t0_c + t5_t_t2 @ tmp:t110 + t2_t_t1 @ tmp:t111 + t5_t_t4 @ tmp:t112
- activation = clip(., lower=t0_lo, upper=t0_hi)  (elementwise; bounds are vectors)

### `t114` (dim 20)
- linear_op (rhs) = t1_c + t114_t_t110 @ tmp:t110 + t3_t_t1 @ tmp:t111 + t114_t_t113 @ tmp:t113
- activation = relu

### `t115` (dim 30)
- linear_op (rhs) = t7_p_p0 @ param:p0 + t115_t_t110 @ tmp:t110 + t7_t_t4 @ tmp:t112 + t115_t_t113 @ tmp:t113
- activation = identity

### `t116` (dim 600)
- linear_op (rhs) = t0_c + t5_t_t2 @ tmp:t113 + t2_t_t1 @ tmp:t114 + t5_t_t4 @ tmp:t115
- activation = clip(., lower=t0_lo, upper=t0_hi)  (elementwise; bounds are vectors)

### `t117` (dim 20)
- linear_op (rhs) = t1_c + t117_t_t113 @ tmp:t113 + t3_t_t1 @ tmp:t114 + t117_t_t116 @ tmp:t116
- activation = relu

### `t118` (dim 30)
- linear_op (rhs) = t7_p_p0 @ param:p0 + t118_t_t113 @ tmp:t113 + t7_t_t4 @ tmp:t115 + t118_t_t116 @ tmp:t116
- activation = identity

### `t119` (dim 600)
- linear_op (rhs) = t0_c + t5_t_t2 @ tmp:t116 + t2_t_t1 @ tmp:t117 + t5_t_t4 @ tmp:t118
- activation = clip(., lower=t0_lo, upper=t0_hi)  (elementwise; bounds are vectors)

### `t120` (dim 20)
- linear_op (rhs) = t1_c + t120_t_t116 @ tmp:t116 + t3_t_t1 @ tmp:t117 + t120_t_t119 @ tmp:t119
- activation = relu

### `t121` (dim 30)
- linear_op (rhs) = t7_p_p0 @ param:p0 + t121_t_t116 @ tmp:t116 + t7_t_t4 @ tmp:t118 + t121_t_t119 @ tmp:t119
- activation = identity

### `t122` (dim 600)
- linear_op (rhs) = t0_c + t5_t_t2 @ tmp:t119 + t2_t_t1 @ tmp:t120 + t5_t_t4 @ tmp:t121
- activation = clip(., lower=t0_lo, upper=t0_hi)  (elementwise; bounds are vectors)

### `t123` (dim 20)
- linear_op (rhs) = t1_c + t123_t_t119 @ tmp:t119 + t3_t_t1 @ tmp:t120 + t123_t_t122 @ tmp:t122
- activation = relu

### `t124` (dim 30)
- linear_op (rhs) = t7_p_p0 @ param:p0 + t124_t_t119 @ tmp:t119 + t7_t_t4 @ tmp:t121 + t124_t_t122 @ tmp:t122
- activation = identity

### `t125` (dim 600)
- linear_op (rhs) = t0_c + t5_t_t2 @ tmp:t122 + t2_t_t1 @ tmp:t123 + t5_t_t4 @ tmp:t124
- activation = clip(., lower=t0_lo, upper=t0_hi)  (elementwise; bounds are vectors)

### `t126` (dim 20)
- linear_op (rhs) = t1_c + t126_t_t122 @ tmp:t122 + t3_t_t1 @ tmp:t123 + t126_t_t125 @ tmp:t125
- activation = relu

### `t127` (dim 30)
- linear_op (rhs) = t7_p_p0 @ param:p0 + t127_t_t122 @ tmp:t122 + t7_t_t4 @ tmp:t124 + t127_t_t125 @ tmp:t125
- activation = identity

### `t128` (dim 600)
- linear_op (rhs) = t0_c + t5_t_t2 @ tmp:t125 + t2_t_t1 @ tmp:t126 + t5_t_t4 @ tmp:t127
- activation = clip(., lower=t0_lo, upper=t0_hi)  (elementwise; bounds are vectors)

### `t129` (dim 20)
- linear_op (rhs) = t1_c + t129_t_t125 @ tmp:t125 + t3_t_t1 @ tmp:t126 + t129_t_t128 @ tmp:t128
- activation = relu

### `t130` (dim 30)
- linear_op (rhs) = t7_p_p0 @ param:p0 + t130_t_t125 @ tmp:t125 + t7_t_t4 @ tmp:t127 + t130_t_t128 @ tmp:t128
- activation = identity

### `t131` (dim 600)
- linear_op (rhs) = t0_c + t5_t_t2 @ tmp:t128 + t2_t_t1 @ tmp:t129 + t5_t_t4 @ tmp:t130
- activation = clip(., lower=t0_lo, upper=t0_hi)  (elementwise; bounds are vectors)

### `t132` (dim 20)
- linear_op (rhs) = t1_c + t132_t_t128 @ tmp:t128 + t3_t_t1 @ tmp:t129 + t132_t_t131 @ tmp:t131
- activation = relu

### `t133` (dim 30)
- linear_op (rhs) = t7_p_p0 @ param:p0 + t133_t_t128 @ tmp:t128 + t7_t_t4 @ tmp:t130 + t133_t_t131 @ tmp:t131
- activation = identity

### `t134` (dim 600)
- linear_op (rhs) = t0_c + t5_t_t2 @ tmp:t131 + t2_t_t1 @ tmp:t132 + t5_t_t4 @ tmp:t133
- activation = clip(., lower=t0_lo, upper=t0_hi)  (elementwise; bounds are vectors)

### `t135` (dim 20)
- linear_op (rhs) = t1_c + t135_t_t131 @ tmp:t131 + t3_t_t1 @ tmp:t132 + t135_t_t134 @ tmp:t134
- activation = relu

### `t136` (dim 30)
- linear_op (rhs) = t7_p_p0 @ param:p0 + t136_t_t131 @ tmp:t131 + t7_t_t4 @ tmp:t133 + t136_t_t134 @ tmp:t134
- activation = identity

### `t137` (dim 600)
- linear_op (rhs) = t0_c + t5_t_t2 @ tmp:t134 + t2_t_t1 @ tmp:t135 + t5_t_t4 @ tmp:t136
- activation = clip(., lower=t0_lo, upper=t0_hi)  (elementwise; bounds are vectors)

### `t138` (dim 20)
- linear_op (rhs) = t1_c + t138_t_t134 @ tmp:t134 + t3_t_t1 @ tmp:t135 + t138_t_t137 @ tmp:t137
- activation = relu

### `t139` (dim 30)
- linear_op (rhs) = t7_p_p0 @ param:p0 + t139_t_t134 @ tmp:t134 + t7_t_t4 @ tmp:t136 + t139_t_t137 @ tmp:t137
- activation = identity

### `t140` (dim 600)
- linear_op (rhs) = t0_c + t5_t_t2 @ tmp:t137 + t2_t_t1 @ tmp:t138 + t5_t_t4 @ tmp:t139
- activation = clip(., lower=t0_lo, upper=t0_hi)  (elementwise; bounds are vectors)

### `t141` (dim 20)
- linear_op (rhs) = t1_c + t141_t_t137 @ tmp:t137 + t3_t_t1 @ tmp:t138 + t141_t_t140 @ tmp:t140
- activation = relu

### `t142` (dim 30)
- linear_op (rhs) = t7_p_p0 @ param:p0 + t142_t_t137 @ tmp:t137 + t7_t_t4 @ tmp:t139 + t142_t_t140 @ tmp:t140
- activation = identity

### `t143` (dim 600)
- linear_op (rhs) = t0_c + t5_t_t2 @ tmp:t140 + t2_t_t1 @ tmp:t141 + t5_t_t4 @ tmp:t142
- activation = clip(., lower=t0_lo, upper=t0_hi)  (elementwise; bounds are vectors)

### `t144` (dim 20)
- linear_op (rhs) = t1_c + t144_t_t140 @ tmp:t140 + t3_t_t1 @ tmp:t141 + t144_t_t143 @ tmp:t143
- activation = relu

### `t145` (dim 30)
- linear_op (rhs) = t7_p_p0 @ param:p0 + t145_t_t140 @ tmp:t140 + t7_t_t4 @ tmp:t142 + t145_t_t143 @ tmp:t143
- activation = identity

### `t146` (dim 600)
- linear_op (rhs) = t0_c + t5_t_t2 @ tmp:t143 + t2_t_t1 @ tmp:t144 + t5_t_t4 @ tmp:t145
- activation = clip(., lower=t0_lo, upper=t0_hi)  (elementwise; bounds are vectors)

### `t147` (dim 20)
- linear_op (rhs) = t1_c + t147_t_t143 @ tmp:t143 + t3_t_t1 @ tmp:t144 + t147_t_t146 @ tmp:t146
- activation = relu

### `t148` (dim 30)
- linear_op (rhs) = t7_p_p0 @ param:p0 + t148_t_t143 @ tmp:t143 + t7_t_t4 @ tmp:t145 + t148_t_t146 @ tmp:t146
- activation = identity

## Final state (the answer)
- `state <- su_t_t146 @ tmp:t146 + su_t_t147 @ tmp:t147 + su_t_t148 @ tmp:t148`

Apply this once, after the steps above, and report
`FOM_MAX_ITER_REACHED` with `iter = 1`. The residual is
`max_abs(new_state - state)` as usual, but nothing terminates on it.

All matrices/consts are provided numerically in `data.npz` / `fom_data.c`.