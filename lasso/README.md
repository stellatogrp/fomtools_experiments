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
in microseconds. The raw files in the linked result directories are the
authoritative records.

### Square sparse scaling: 20,000 × 20,000

| method | NumPy | OpenMP | CUDA |
|---|---:|---:|---:|
| ISTA | 260543.474 | 250458.695 | 5448.602 |
| FISTA | 980284.507 | 800844.219 | 5575.185 |

### High-dimensional rectangular scaling: 10,000 × 40,000

| method | NumPy | OpenMP | CUDA |
|---|---:|---:|---:|
| ISTA | 200721.665 | 161347.372 | 5418.549 |
| FISTA | 769241.593 | 605327.768 | 6674.942 |

The studies ran on different Della nodes and GPU models. They demonstrate
backend-specific generation within each study; their absolute timing numbers
should not be used as a controlled cross-hardware comparison.

## Artifact contents

Each result directory preserves the raw interleaved timing output, hardware and
software metadata, final verification logs, Phase-B optimization reports,
fixed-configuration manifest, and source scripts. Compiled binaries and
temporary generated directories are excluded.

All archived solvers passed their eight-sample generated-oracle verification
checks with tolerance `1e-6`; exact error lines are retained in each
`verify_*.log` file.
