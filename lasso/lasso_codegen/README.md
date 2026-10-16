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
| Large rectangular scaling, `D` and `D^T` separate (sepA), with hand-written baselines | 325,000 × 1,300,000 (2.6M nonzeros) | 8 nonzeros/row; 2/column | 7 interleaved rounds, one node, whole node per round | [`results/rect325k1p3m_sepA/`](results/rect325k1p3m_sepA/) |
| Dense rectangular, `D` and `D^T` separate (sepA), with hand-written baselines | 10,000 × 40,000 (2.6M nonzeros, 0.65%) | 260 nonzeros/row; 65/column | 7 interleaved rounds, one node | [`results/rect10k40k_dense_sepA/`](results/rect10k40k_dense_sepA/) |

Both studies solve

$$
\min_{z}\; \frac{1}{2}\lVert A z - b \rVert_2^2 + 0.02\lVert z \rVert_1.
$$

In every study `A` is a synthetic sparse random matrix with Gaussian values.

## Results

Each row reports the median of the interleaved round-level per-solve values,
in milliseconds. The NumPy column is FOMTools' reference execution of the
extracted iteration, not a hand-written implementation; the sepA study below
adds hand-written SciPy and CuPy/cuSPARSE baselines. The raw files in the linked result directories retain the
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

### Large rectangular scaling: 325,000 × 1,300,000, `D` and `D^T` separate (sepA)

The same instance, step size and algorithms, written so that FOMTools extracts
products with `D` and `D^T` (2.6M nonzeros each) instead of the merged operator
`I - eta D^T D` (19.5M; FISTA about 69M). All columns come from the same seven
interleaved rounds on one AMD EPYC 7H12 + A100 node (`della-i12g1`), each round
holding the whole node; seconds per 200-iteration solve.

| method | FOMTools reference (NumPy) | Hand-written SciPy | Generated OpenMP, original | Generated OpenMP, sepA | Hand-written CuPy | Generated CUDA, original | Generated CUDA, sepA |
|---|---:|---:|---:|---:|---:|---:|---:|
| ISTA | 7.304 | 4.368 | 1.133 | **0.4339** | 0.05050 | 0.1261 | **0.02368** |
| FISTA | 18.55 | 4.622 | 1.252 | **0.5517** | 0.05941 | 0.1289 | **0.02672** |

| speedup of sepA over | hand-written SciPy | hand-written CuPy (cuSPARSE) | original-form OpenMP | original-form CUDA |
|---|---:|---:|---:|---:|
| ISTA | 10.1× | 2.1× | 2.6× | 5.3× |
| FISTA | 8.4× | 2.2× | 2.3× | 4.8× |

The hand-written baselines keep `D` and `D^T` separate: SciPy on one CPU core
and CuPy on the GPU, whose sparse products call cuSPARSE. The FOMTools
reference column executes the sepA iteration. The original-form generated
CUDA solvers are 2.5× (ISTA) and 2.2× (FISTA) slower than CuPy, because every
iteration reads the merged operators. The original-form OpenMP and FISTA CUDA
solvers are those of `rect325k1p3m_tol1e-5`; the original-form ISTA CUDA
solver was generated for this comparison with the same pipeline.

### Dense rectangular: 10,000 × 40,000 with 260 nonzeros per row, `D` and `D^T` separate (sepA)

`D` has 260 nonzeros per row (0.65% of its entries, 2.6M in total). `D^T D` would
have 551M nonzeros (34.4% dense), so only the separated formulation is generated.
The generated solvers are compared with hand-written baselines that keep `D` and
`D^T` separate: SciPy (1 core) against the generated OpenMP solvers on the CPU,
and CuPy, whose sparse products call cuSPARSE, against the generated CUDA solvers
on the GPU. Seven interleaved rounds in one job on one AMD EPYC 7H12 + A100 node
(`della-i12g1`); seconds per 200-iteration solve.

| method | Hand-written SciPy (1 core) | Generated OpenMP (16 threads) | Hand-written CuPy (cuSPARSE) | Generated CUDA |
|---|---:|---:|---:|---:|
| ISTA | 1.250 | **0.08411** | 0.04781 | **0.02192** |
| FISTA | 1.264 | **0.08431** | 0.05585 | **0.02690** |

| speedup | CPU: generated OpenMP over SciPy (1 core) | GPU: generated CUDA over CuPy (cuSPARSE) |
|---|---:|---:|
| ISTA | 14.9× | 2.18× |
| FISTA | 15.0× | 2.08× |

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

### Large rectangular scaling: 325,000 × 1,300,000, `D` and `D^T` separate (sepA)

Fresh builds of the translated and code-optimized sources, timed in the same
job on `della-i12g1`.

| solver | translation (s) | code optimization (s) | speedup |
|---|---:|---:|---:|
| ISTA OpenMP | 0.633 | 0.434 | 1.46× |
| ISTA CUDA | 0.0464 | 0.0227 | 2.04× |
| FISTA OpenMP | 1.925 | 0.550 | 3.50× |
| FISTA CUDA | 0.1467 | 0.0259 | 5.66× |

The original-form ISTA CUDA solver of this comparison went from 0.1344 s to
0.1065 s (1.26×) in the same kind of head-to-head on `della-l09g6`.

### Dense rectangular: 10,000 × 40,000 with 260 nonzeros per row, `D` and `D^T` separate (sepA)

Fresh builds of the translated and code-optimized sources, timed in the same
job on the agent's node (AMD EPYC 7413 + A100 SXM4 80 GB), in milliseconds.

| solver | translation (ms) | code optimization (ms) | speedup |
|---|---:|---:|---:|
| ISTA OpenMP | 63.77 | 58.92 | 1.08× |
| ISTA CUDA | 28.29 | 21.45 | 1.32× |
| FISTA OpenMP | 231.47 | 100.37 | 2.31× |
| FISTA CUDA | 67.92 | 26.67 | 2.55× |

Agent wall-clock time per solver, in minutes, including the controller's checks
after each phase and the queueing of the short build jobs:

| solver | translation | code optimization | total |
|---|---:|---:|---:|
| ISTA OpenMP | 6.3 | 17.7 | 24.3 |
| ISTA CUDA | 8.0 | 30.4 | 38.8 |
| FISTA OpenMP | 8.1 | 20.6 | 29.1 |
| FISTA CUDA | 8.8 | 25.9 | 35.1 |
| all four | 31.2 | 94.7 | 127.3 |

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

The sepA study's agents checked the extracted operators numerically and
exploited the identity blocks of the sepA iteration: the residual and
state-update identity products became in-place updates or pointer rotations,
behind setup-time probes with a generic fallback, and the loop-invariant
parameter term was hoisted. For FISTA both backends folded the two momentum
terms, which share one block of `D`, into one product with `D` on the
extrapolated point, and fused the soft-threshold with the next extrapolation.
On CUDA they kept the iteration vectors in an L2 persisting window and
page-locked the solution buffer. The two products with `D` and `D^T` in the
fixed operator files remain most of the solve (about 94% for OpenMP ISTA, 81%
for CUDA FISTA).

The dense sepA study's agents found the same structure: all four solvers drop
the soft-threshold at zero and seed the accumulator with the loop-invariant
parameter term; the OpenMP solvers and the FISTA CUDA solver turn the identity
products into in-place updates or pointer bookkeeping, and both FISTA solvers
fold the two momentum terms into one product per iteration (on CUDA, one
2.6M-nonzero product replaces 7.8M nonzeros of products, with the iterates in a
three-buffer ring). The ISTA CUDA solver instead re-launches the fixed CSR
kernels with launch shapes tuned for the A100 (435 blocks of 23 threads instead
of 40 blocks of 256 for the 10,000-row product with `D`), captures the whole
solve as one CUDA graph and chooses an L2 persisting window at setup. The two
products with `D` and `D^T` remain most of the solve (about 99% for OpenMP ISTA,
91% for OpenMP FISTA).

All archived solvers passed their eight-sample generated-oracle verification
checks with tolerance `1e-6` (`1e-5` for `rect325k1p3m_tol1e-5`,
`rect325k1p3m_sepA` and `rect10k40k_dense_sepA`); exact error lines are retained
in each `verify_*.log` file.
