# Rectangular 10k by 40k with 260 nonzeros per row, D and D^T kept separate (sepA)

This directory archives a denser version of the 10,000 by 40,000 Lasso study of
[`../rect10k40k/`](../rect10k40k/): `D` has 260 nonzeros per row (0.65% of its entries)
instead of 8. The method is written as in [`../rect325k1p3m_sepA/`](../rect325k1p3m_sepA/):
the residual `r = D w - x` is materialized as its own step and the gradient is applied as
`D^T r`, so the extracted iteration multiplies by `D` and `D^T` (2.6M nonzeros each). At this
density `D^T D` would have 551M nonzeros (34.4% of its entries, 212 times the nonzeros of
`D`), so only the separated formulation is generated.

The generated solvers are compared with hand-written baselines that also keep `D` and `D^T`
separate (`source/baselines.py`): SciPy on one CPU core against the generated OpenMP solvers,
and CuPy, whose sparse products call cuSPARSE, against the generated CUDA solvers on the GPU.
Both baselines were checked against FOMTools' reference execution before any agent ran
(`baseline_gate.txt`) and again before timing (`baseline_check_*.log`).

## Fixed problem

| setting | value |
|---|---:|
| `D` | synthetic sparse random matrix, 10,000 by 40,000 |
| nonzeros of `D` | 2,599,983 (260 slots per row, 64 or 65 per column, Gaussian values) |
| seed | 0 |
| regularization `lambda` | 0.02 |
| step size | `1 / (||D||_1 ||D||_infinity)` = 0.0147039 |
| iterations per solve | 200 |
| precision | float64 |
| verification tolerance | `1e-5` |

`matrix_stats.md` lists `D`, the size of `D^T D` and the extracted operators.

## Final benchmark

`production_interleaved_7rounds.txt` is the authoritative raw timing record: seven interleaved
rounds in one job on `della-i12g1` (AMD EPYC 7H12, NVIDIA A100 PCIe 40 GB), with 16 cores and
one A100. Each round times hand-written SciPy (one warm-up, three solves, median), the
generated OpenMP solver (eight warm-ups, ten solves, mean), hand-written CuPy (same) and the
generated CUDA solver, for ISTA and then FISTA. `summary.csv` reports the median of the seven
round values. Times are seconds per 200-iteration solve.

| method | Hand-written SciPy (1 core) | Generated OpenMP (16 threads) | Hand-written CuPy (cuSPARSE) | Generated CUDA |
|---|---:|---:|---:|---:|
| ISTA | 1.250 | **0.08411** | 0.04781 | **0.02192** |
| FISTA | 1.264 | **0.08431** | 0.05585 | **0.02690** |

| speedup | CPU: generated OpenMP over SciPy (1 core) | GPU: generated CUDA over CuPy (cuSPARSE) |
|---|---:|---:|
| ISTA | 14.9x | 2.18x |
| FISTA | 15.0x | 2.08x |

Across rounds the generated solvers varied by at most 4.0% (FISTA OpenMP) and the
hand-written baselines by at most 2.8% (SciPy FISTA). The raw record also contains timings of
FOMTools' reference execution, which are not reported here.

## Code optimization and agent time

Fresh builds of the translated and code-optimized sources, timed in the same job on the agent's
node (AMD EPYC 7413, NVIDIA A100 SXM4 80 GB; milliseconds per solve, 5 solves after 8 warm-ups):

| solver | translation (ms) | code optimization (ms) | speedup |
|---|---:|---:|---:|
| ISTA OpenMP | 63.77 | 58.92 | 1.08x |
| ISTA CUDA | 28.29 | 21.45 | 1.32x |
| FISTA OpenMP | 231.47 | 100.37 | 2.31x |
| FISTA CUDA | 67.92 | 26.67 | 2.55x |

Agent wall-clock time per solver, in minutes. The total also covers the controller's checks
after each phase (independent verification, the head-to-head build and timing, the provenance
and transcript checks); every build and test the agent ran went through a short Slurm job, so
the times include that queueing.

| solver | translation | code optimization | total |
|---|---:|---:|---:|
| ISTA OpenMP | 6.3 | 17.7 | 24.3 |
| ISTA CUDA | 8.0 | 30.4 | 38.8 |
| FISTA OpenMP | 8.1 | 20.6 | 29.1 |
| FISTA CUDA | 8.8 | 25.9 | 35.1 |
| all four | 31.2 | 94.7 | 127.3 |

Extraction took 0.6 s (ISTA) and 1.2 s (FISTA), the scaffolds about 40 s, and the fixed CUDA
objects 3.3 minutes, all before any agent ran.

## Verification and generation

All four solvers were generated with FOMTools' built-in translation and code optimization
instructions and default agent driver. Before each translation the controller checked that the
target directory held only FOMTools' scaffold. Before a solver was accepted, a provenance check
compared it with every other solver source on the cluster storage (`provenance_*.log`; closest
line similarity 0.37, and a solver is rejected at 0.8), and a transcript check confirmed that no
tool call of the agent touched another run (`transcript_check_*.log`). The ISTA CUDA solver was
generated in its own controller run, during which every other run's solver sources, reports and
agent transcripts were unreadable. All four solvers passed the eight-sample generated-oracle
check at `1e-5` immediately before timing (`verify_*.log`; worst errors 0 and 2.7e-15 for
ISTA, 3.3e-8 and 3.1e-8 for FISTA). `OPTIMIZATIONS_*.md` hold each solver's translation to code
optimization report with the agent's notes; the ISTA CUDA agent reached FOMTools' default
limit of 60 turns during code optimization, and the solver it left was verified and kept.
`generation_metadata.txt` records the jobs and nodes, `benchmark_metadata.txt` the benchmark
node, and `checksums.txt` the solver sources and the exact binaries that were timed.

## Reproduction sources

`source/` holds the sepA problem definitions (`paper_*_sparse20k.py`, run with 10,000 rows,
40,000 columns and 260 nonzeros per row), the hand-written baselines and the complete pipeline
(`source/pipeline/`).
