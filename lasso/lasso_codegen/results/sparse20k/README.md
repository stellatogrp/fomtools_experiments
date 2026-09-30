# Sparse 20k result artifacts

This directory archives the completed sparse Lasso ISTA/FISTA backend study on
Princeton's Della cluster. It contains the raw measurements, verification logs,
Phase-B optimization reports, hardware metadata, and the source files used to
define and benchmark the instance. Compiled binaries and temporary scaffold
directories are intentionally excluded.

## Fixed problem

| setting | value |
|---|---:|
| rows and columns | 20,000 × 20,000 |
| seed | 0 |
| regularization `lambda` | 0.02 |
| iterations per solve | 200 |
| precision | float64 |
| matrix construction | eight seeded random permutations with seeded Gaussian values |
| sparsity | approximately eight nonzeros per row and column |
| step size | reciprocal of `||A||_1 ||A||_infinity` |

The problem, algorithm, seed, iteration count, and precision are fixed across
the CPU/OpenMP and CUDA targets. Only the generated backend changes.

## Measurement

`production_interleaved_3rounds.txt` contains three interleaved rounds. Each
round uses three NumPy reference solves for the reported median and ten solves
of each generated solver for the reported mean. `summary.csv` contains the
round-level medians:

| method | NumPy (µs/solve) | CPU/OpenMP (µs/solve) | CUDA (µs/solve) |
|---|---:|---:|---:|
| ISTA | 260543.474 | 250458.695 | 5448.602 |
| FISTA | 980284.507 | 800844.219 | 5575.185 |

The raw file is the authoritative measurement record; the table is provided
for convenient comparison without pooling the rounds.

## Validation and optimization

All four final solvers passed the eight-sample generated-oracle check with a
`1e-6` tolerance. The corresponding logs are `verify_*.log`. The four
`OPTIMIZATIONS_*.md` files are the complete Phase-B reports, including source
diffs, profiling notes, and re-verification statements.

## Hardware and software

- Node: `della-l02g5`
- CPU: two-socket Intel Xeon Gold 6342, 48 logical CPUs at 2.80 GHz
- GPU: NVIDIA A100 80 GB PCIe
- CUDA compiler: 12.9.86
- Python: 3.14.6
- FOMTools source commit: `ca3b8c2538a36abe28faf3025d20af4562aa0c49`
- Slurm allocation: `13843780`

## Reproduction sources

The `source/` directory preserves the sparse ISTA/FISTA definitions and the
Python benchmark driver. The generated solver directories are not checked in;
they were produced on Della with Phase B enabled for large-scale OpenMP and
CUDA targets. `manifest.json` records the fixed run parameters.
