# Rectangular 10k by 40k sparse Lasso backend experiment

## Goal

Evaluate verified generated ISTA and FISTA solvers for large-scale OpenMP and
CUDA targets on a high-dimensional Lasso instance with 10,000 measurements and
40,000 decision variables.

## Method

The fixed problem uses seed 0, `lambda=0.02`, float64 arithmetic, and 200
iterations. Eight edge slots per row are built from two seeded permutations of
the 40,000 columns. Phase B was enabled for all four algorithm/backend pairs.
The final benchmark rebuilt and re-verified those solvers, then ran seven
interleaved rounds with three NumPy repetitions and ten generated solves per
round.

## Results

| method | NumPy median (µs/solve) | OpenMP median (µs/solve) | CUDA median (µs/solve) |
|---|---:|---:|---:|
| ISTA | 200721.665 | 161347.372 | 5418.549 |
| FISTA | 769241.593 | 605327.768 | 6674.942 |

All four final solvers passed the eight-sample generated-oracle check with a
`1e-6` tolerance:

| solver | worst max-absolute error |
|---|---:|
| ISTA OpenMP | 0.000e+00 |
| ISTA CUDA | 6.573e-14 |
| FISTA OpenMP | 4.957e-07 |
| FISTA CUDA | 5.115e-07 |

## Analysis and next steps

The full-shape sparse IR passed extraction and round-trip checks before
generation. The final benchmark hardware is recorded separately from the
generation hardware; comparisons with results from another node should account
for that difference. The denser `k=32` experiment is a separate pending study
and is not included in this artifact set.
