# Rectangular 325k by 1.3M result artifacts

This directory archives the completed large-scale sparse Lasso ISTA/FISTA
study. The input matrix has 325,000 rows, 1,300,000 columns and 2,600,000
nonzeros; the seven-round benchmark uses the verified, code-optimized
OpenMP and CUDA solvers.

## Fixed problem

| setting | value |
|---|---:|
| rows of `A` | 325,000 |
| columns / decision variables | 1,300,000 |
| nonzeros of `A` | 2,600,000 |
| seed | 0 |
| regularization `lambda` | 0.02 |
| iterations per solve | 200 |
| precision | float64 |
| nonzero slots per row | 8 |
| column occurrences | 2 |
| step size | `1 / (||A||_1 ||A||_infinity)` = 0.0730896 |

The construction is the rect10k40k generator at a larger size: the 2,600,000
edge slots are two seeded permutations of the 1,300,000 columns, so every row
has exactly eight nonzeros and every column two (no duplicates occurred).
`matrix_stats.md` lists the per-iteration operators (19.5M nonzeros for ISTA;
30.0M + 39.0M for FISTA).

## Final benchmark

`production_interleaved_7rounds.txt` is the authoritative raw timing record.
It contains seven interleaved rounds; each round uses three NumPy repetitions
(after one warm-up) and ten solves for each generated solver (after eight
warm-ups). `summary.csv` reports the median of the seven round-level values.

| method | NumPy (s) | OpenMP (s) | CUDA (s) | OpenMP speedup | CUDA speedup |
|---|---:|---:|---:|---:|---:|
| ISTA | 14.954 | 1.210 | 0.142 | 12.4x | 105.0x |
| FISTA | 73.375 | 5.219 | 0.455 | 14.1x | 161.2x |

All seven rounds ran on `della-i12g1`, with an AMD EPYC 7H12 CPU and an
NVIDIA A100 PCIe 40 GB GPU on the same node; OpenMP used 16 pinned threads.
Across rounds every series varied by less than 1.4%.

## Verification and optimization

All four final solvers passed the eight-sample generated-oracle check with
tolerance `1e-6` immediately before timing; the `verify_*.log` files contain
the exact errors. The four `OPTIMIZATIONS_*.md` files preserve the translation to
code optimization reports, each based on a fresh-build head-to-head inside
one job.
`generation_metadata.txt` records the generation jobs and nodes;
`benchmark_metadata.txt` records the benchmark node; `checksums.txt` records
the solver sources and the exact binaries that were timed.

## Reproduction sources

`source/` contains the ISTA/FISTA problem definitions, the NumPy-reference
benchmark driver, and the complete pipeline (`source/pipeline/`: scaffold,
CUDA-object, controller, bridge and benchmark scripts) as used for this run.
`matrix_stats.md` lists the generated operators. The multi-GB generated matrix
sources and compiled binaries are not checked in; the pipeline regenerates
them deterministically.
