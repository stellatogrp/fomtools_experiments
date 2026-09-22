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

Both studies solve

```text
minimize_z  0.5 ||A z - b||_2^2 + 0.02 ||z||_1.
```

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

## Artifact contents

Each result directory preserves the raw interleaved timing output, hardware and
software metadata, final verification logs, Phase-B optimization reports,
fixed-configuration manifest, and source scripts. Compiled binaries and
temporary generated directories are excluded.

All archived solvers passed their eight-sample generated-oracle verification
checks with tolerance `1e-6`; exact error lines are retained in each
`verify_*.log` file.
