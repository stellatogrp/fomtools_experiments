# Large-scale Lasso backend experiments

This directory archives reproducible sparse Lasso ISTA/FISTA experiments for
FOMTools' large-scale OpenMP CPU and CUDA GPU targets. Each study holds the
algorithm, seed, regularization, precision, and fixed 200-iteration horizon
constant across backends; only the emitted backend changes.

## Studies

| study | matrix shape | sparse degree | timing protocol | result directory |
|---|---:|---:|---|---|
| Square sparse scaling | 20,000 × 20,000 | about 8 nonzeros/row and column | 3 interleaved rounds | [`results/sparse20k/`](results/sparse20k/) |
| High-dimensional rectangular scaling | 10,000 × 40,000 | 8 nonzero slots/row; nominally 2/column | 7 interleaved rounds | [`results/rect10k40k/`](results/rect10k40k/) |
| Large rectangular scaling | 325,000 × 1,300,000 (2.6M nonzeros) | 8 nonzeros/row; 2/column | 7 interleaved rounds, one node | [`results/rect325k1p3m/`](results/rect325k1p3m/) |
| Large rectangular scaling, tolerance `1e-5` | 325,000 × 1,300,000 (2.6M nonzeros) | 8 nonzeros/row; 2/column | 7 interleaved rounds, one node | [`results/rect325k1p3m_tol1e-5/`](results/rect325k1p3m_tol1e-5/) |

Both studies solve

$$
\min_{z}\; \frac{1}{2}\lVert A z - b \rVert_2^2 + 0.02\lVert z \rVert_1.
$$

## Results

Each row reports the median of the interleaved round-level per-solve values,
in milliseconds. The raw files in the linked result directories retain the
underlying microsecond measurements and are the
authoritative records.

### Square sparse scaling: 20,000 × 20,000

| method | NumPy (ms) | OpenMP (ms) | CUDA (ms) |
|---|---:|---:|---:|
| ISTA | 260.543 | 250.459 | 5.449 |
| FISTA | 980.285 | 800.844 | 5.575 |

### High-dimensional rectangular scaling: 10,000 × 40,000

| method | NumPy (ms) | OpenMP (ms) | CUDA (ms) |
|---|---:|---:|---:|
| ISTA | 200.722 | 161.347 | 5.419 |
| FISTA | 769.242 | 605.328 | 6.675 |

### Large rectangular scaling: 325,000 × 1,300,000

| method | NumPy (s) | OpenMP (s) | CUDA (s) |
|---|---:|---:|---:|
| ISTA | 14.954 | 1.210 | 0.142 |
| FISTA | 73.375 | 5.219 | 0.455 |

This study ran on one AMD EPYC 7H12 + A100 node with 16-thread OpenMP builds
(the earlier studies' OpenMP binaries were serial; see the study report).

### Large rectangular scaling: 325,000 × 1,300,000, tolerance `1e-5`

| method | NumPy (s) | OpenMP (s) | CUDA (s) | OpenMP speedup | CUDA speedup |
|---|---:|---:|---:|---:|---:|
| ISTA | 14.813 | 1.177 | 0.125 | 12.6× | 118.4× |
| FISTA | 72.813 | 1.246 | 0.128 | 58.4× | 568.6× |

Same problem and generated operators as the previous study, with all four
solvers verified at tolerance `1e-5` and the agent driven by FOMTools'
built-in translation and code optimization instructions. All rounds ran on
one AMD EPYC 7H12 + A100 node (`della-i12g2`) with 16-thread OpenMP builds.

The studies ran on different Della nodes and GPU models. They demonstrate
backend-specific generation within each study; their absolute timing numbers
should not be used as a controlled cross-hardware comparison.

## Phase-B codegen optimization

These are the separate code-generation measurements recorded in the Phase-B
optimization reports, in milliseconds per solve. They are not the final
interleaved benchmark measurements above. Every optimized solver was retained
only after re-verification.

### Square sparse scaling: 20,000 × 20,000

| solver | Phase A (ms) | Phase B (ms) | Phase-A → Phase-B |
|---|---:|---:|---:|
| ISTA OpenMP | 250.960 | 208.162 | 1.21× |
| ISTA CUDA | 8.124 | 5.415 | 1.50× |
| FISTA OpenMP | 800.941 | 203.135 | 3.94× |
| FISTA CUDA | 18.844 | 5.508 | 3.42× |

### High-dimensional rectangular scaling: 10,000 × 40,000

| solver | Phase A (ms) | Phase B (ms) | Phase-A → Phase-B |
|---|---:|---:|---:|
| ISTA OpenMP | 144.437 | 100.795 | 1.43× |
| ISTA CUDA | 6.713 | 5.682 | 1.18× |
| FISTA OpenMP | 420.554 | 146.887 | 2.86× |
| FISTA CUDA | 14.840 | 4.583 | 3.24× |

### Large rectangular scaling: 325,000 × 1,300,000

Fresh builds of the translated and code-optimized sources, timed in the same
job (node in parentheses).

| solver | translation (s) | code optimization (s) | speedup |
|---|---:|---:|---:|
| ISTA OpenMP (della-i12g2) | 1.464 | 1.202 | 1.22× |
| ISTA CUDA (della-i12g2) | 0.160 | 0.142 | 1.12× |
| FISTA OpenMP (della-i14g1) | 10.390 | 9.066 | 1.15× |
| FISTA CUDA (della-i12g1) | 0.721 | 0.456 | 1.58× |

### Large rectangular scaling: 325,000 × 1,300,000, tolerance `1e-5`

Fresh builds of the translated and code-optimized sources, timed in the same
job (node in parentheses).

| solver | translation (s) | code optimization (s) | speedup |
|---|---:|---:|---:|
| ISTA OpenMP (della-i12g1) | 1.209 | 1.128 | 1.07× |
| ISTA CUDA (della-i12g2) | 0.125 | 0.125 | 1.00× |
| FISTA OpenMP (della-i12g2) | 5.828 | 1.254 | 4.65× |
| FISTA CUDA (della-i12g2) | 0.525 | 0.128 | 4.11× |

### Optimization tricks used

Both studies applied the following algebra- and layout-aware transformations:

- Hoisted the parameter sparse matrix-vector product out of the 200-iteration
  loop because the parameter remains fixed during a solve.
- Replaced identity or permutation state-update CSR operations with direct
  assignments, eliminating sparse launches, output clearing, and copy passes.
- Fused soft-thresholding with the state update and computed the residual only
  at the final iteration because termination is fixed at 200 iterations.
- For FISTA, exploited the two-state momentum structure to eliminate one of
  the otherwise redundant sparse matrix-vector products per iteration.
- For CUDA, retained state on the device through the iteration loop, used
  pointer swaps, and downloaded results only after the final iteration.

The square sparse study additionally tuned the 20,000-element CUDA kernels and
used direct block updates for the FISTA state. The rectangular study cached the
previous `B*x` FISTA product on OpenMP, and fused CUDA accumulator clearing into
the elementwise activation kernel.

The 325k × 1.3M study's code optimization kept the parameter hoisting, direct state-update copies,
fused activation and final-iteration residual. It also pinned the CUDA state
vector in L2 cache and pinned the host transfer buffers. Its FISTA agents found
the one-product momentum identity but could not use it: the two extracted
operator blocks differ by about `1e-15`, which over 200 iterations exceeds the
`1e-6` verification tolerance. This explains its smaller FISTA
code-optimization gains.

The 325k × 1.3M tolerance-`1e-5` study's FISTA agents used that identity on
both backends: `t0_state = [0, G]` and `t0_state__d_gamma = [-G, G]`, so the
momentum moves onto the iterate and one product with `G` replaces the two
large products per iteration (error about `1.4e-6`, within `1e-5`). Together
with the following changes this gave 4.65× (OpenMP) and 4.11× (CUDA):

- Replaced the shift-matrix state update with buffer relabelling, removing both
  state-update products, the output clearing and the copy.
- Hoisted the parameter product out of the solve and used it to seed the
  accumulating matrix-vector output, so the add costs no extra pass.
- Fused soft-thresholding, the momentum extrapolation and the next
  accumulator seed into one sweep; computed the residual only at the final
  iteration.
- On CUDA, kept the gathered vector in an L2 persisting window sized for the
  matrix's column density, and page-locked the host output buffer.

Its ISTA agents hoisted the parameter product into the accumulator seed,
removed the identity state-update product (checked at setup with probe
vectors), and fused soft-thresholding with the accumulator reseed. On OpenMP
they also computed the residual only at the final iteration and used
non-temporal stores for the next iterate; on CUDA they kept the state vector in
an L2 persisting window. The fixed matrix-vector product is about 93%
(OpenMP) and 95% (CUDA) of the optimized ISTA solve, which limits the gains.

All archived solvers passed their eight-sample generated-oracle verification
checks with tolerance `1e-6` (`1e-5` for `rect325k1p3m_tol1e-5`); exact error
lines are retained in each `verify_*.log` file.
