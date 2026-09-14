# Sparse 20k ISTA/FISTA backend experiment

## Goal and hypothesis

Test whether the paper-style sparse Lasso ISTA and FISTA recordings can be
generated, verified, and benchmarked at 20,000 by 20,000 scale for both a
large-scale OpenMP CPU target and a CUDA GPU target. The hypothesis was that
native sparse coefficient storage would keep extraction and serialization
feasible, and that the generated solvers would pass the same numerical checks
on both backends.

## Method

The fixed instance has `m = n = 20,000`, seed 0, `lambda = 0.02`, 200
iterations, and float64 arithmetic. The design matrix is constructed from
eight seeded random permutations with seeded Gaussian values, giving about
eight nonzeros per row and column. The step size is the reciprocal of the
deterministic `||A||_1 ||A||_infinity` upper bound. Phase B was enabled for all
four generated solver/backend combinations. Measurements used three
interleaved rounds, ten generated solves per timing, and three NumPy reference
repetitions per round.

## Results

| method | NumPy median (µs/solve) | OpenMP median (µs/solve) | CUDA median (µs/solve) |
|---|---:|---:|---:|
| ISTA | 260543.474 | 250458.695 | 5448.602 |
| FISTA | 980284.507 | 800844.219 | 5575.185 |

All four final generated solvers passed verification against eight generated
oracle samples with tolerance `1e-6`:

| solver | worst max-absolute error |
|---|---:|
| ISTA OpenMP | 0.000e+00 |
| ISTA CUDA | 4.263e-14 |
| FISTA OpenMP | 5.922e-10 |
| FISTA CUDA | 5.715e-10 |

## Analysis and next steps

The extraction, sparse serialization, compilation, optimization, verification,
and timing stages all completed in allocation `13843780`. The complete raw
timing record, hardware capture, verification logs, source definitions, and
Phase-B reports are preserved in this directory. Any further scaling study
should keep the problem definition fixed while changing only the selected
dimension or sparsity parameter.
