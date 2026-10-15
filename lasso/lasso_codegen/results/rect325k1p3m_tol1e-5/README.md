# Rectangular 325k by 1.3M result artifacts, verification tolerance 1e-5

This directory archives a second run of the large-scale sparse Lasso
ISTA/FISTA study in [`../rect325k1p3m/`](../rect325k1p3m/). The problem,
generated operators and benchmark protocol are the same. Two things differ:

- All four solvers were verified at tolerance `1e-5` (the earlier run used
  `1e-6`).
- The coding agent was driven with FOMTools' built-in translation and code
  optimization instructions (`fomtools/codegen/agent/prompts.py`), through
  FOMTools' default agent driver.

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
| verification tolerance | `1e-5` |

The generated operator files and compiled objects are those of
`rect325k1p3m`; only the tolerance in the generated verifier changed.
`matrix_stats.md` lists the per-iteration operators.

## Final benchmark

`production_interleaved_7rounds.txt` is the authoritative raw timing record.
It contains seven interleaved rounds; each round uses three NumPy repetitions
(after one warm-up) and ten solves for each generated solver (after eight
warm-ups). `summary.csv` reports the median of the seven round-level values.

| method | NumPy (s) | OpenMP (s) | CUDA (s) | OpenMP speedup | CUDA speedup |
|---|---:|---:|---:|---:|---:|
| ISTA | 14.813 | 1.177 | 0.125 | 12.6x | 118.4x |
| FISTA | 72.813 | 1.246 | 0.128 | 58.4x | 568.6x |

All seven rounds ran on `della-i12g2`, with an AMD EPYC 7H12 CPU and an
NVIDIA A100 PCIe 40 GB GPU on the same node; OpenMP used 16 pinned threads.
Across rounds every series varied by less than 1.7%.

## Verification and optimization

All four final solvers passed the eight-sample generated-oracle check with
tolerance `1e-5` immediately before timing; the `verify_*.log` files contain
the exact errors. The four `OPTIMIZATIONS_*.md` files preserve the translation to
code optimization reports, each based on a fresh-build head-to-head inside
one job, with the agent's final summary.
`generation_metadata.txt` records the generation jobs and nodes;
`benchmark_metadata.txt` records the benchmark node; `checksums.txt` records
the solver sources and the exact binaries that were timed.

## Reproduction sources

`source/` contains the ISTA/FISTA problem definitions, the NumPy-reference
benchmark driver, and the complete pipeline (`source/pipeline/`) as used for
this run. `controller_builtin.sh` and `fomtools_agent.py` run the agent
phases through FOMTools' built-in instructions and driver. The multi-GB
generated matrix sources and compiled binaries are not checked in; the
pipeline regenerates them deterministically.
