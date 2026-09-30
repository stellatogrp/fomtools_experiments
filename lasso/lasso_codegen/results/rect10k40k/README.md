# Rectangular 10k by 40k result artifacts

This directory archives the completed high-dimensional sparse Lasso ISTA/FISTA
study. The input matrix has 10,000 rows and 40,000 columns; the seven-round
benchmark uses the verified, Phase-B-optimized OpenMP and CUDA solvers.

## Fixed problem

| setting | value |
|---|---:|
| rows of `A` | 10,000 |
| columns / decision variables | 40,000 |
| seed | 0 |
| regularization `lambda` | 0.02 |
| iterations per solve | 200 |
| precision | float64 |
| nonzero slots per row | 8 |
| nominal column occurrences | 2 |
| step size | `1 / (||A||_1 ||A||_infinity)` |

The generator constructs the 80,000 edge slots from two seeded permutations of
the 40,000 columns. It therefore fixes the row degree at eight and the nominal
column degree at two before rare duplicate removal.

## Final benchmark

`production_interleaved_7rounds.txt` is the authoritative raw timing record.
It contains seven interleaved rounds; each round uses three NumPy repetitions
and ten solves for each generated solver. `summary.csv` reports the median of
the seven round-level values.

| method | NumPy (µs/solve) | OpenMP (µs/solve) | CUDA (µs/solve) |
|---|---:|---:|---:|
| ISTA | 200721.665 | 161347.372 | 5418.549 |
| FISTA | 769241.593 | 605327.768 | 6674.942 |

The final benchmark ran on `della-i12g2`, with an AMD EPYC 7H12 CPU and an
NVIDIA A100 PCIe 40 GB GPU. Do not compare these absolute times directly to a
study run on another Della node without accounting for the hardware change.

## Verification and optimization

All four final solvers passed the eight-sample generated-oracle check with
tolerance `1e-6`; the `verify_*.log` files contain the exact errors. The four
`OPTIMIZATIONS_*.md` files preserve the complete Phase-B reports. They were
generated in the earlier code-generation allocation; `generation_metadata.txt`
records that allocation, while `benchmark_metadata.txt` records the final
seven-round allocation.

## Reproduction sources

`source/` contains the rectangular ISTA/FISTA definitions, Python-reference
benchmark driver, and the submitted seven-round Slurm script. `tier3_ir_m10000_n40000.txt`
records full-shape sparse-IR and round-trip checks. Compiled binaries and
temporary scaffold directories are intentionally not checked in.
