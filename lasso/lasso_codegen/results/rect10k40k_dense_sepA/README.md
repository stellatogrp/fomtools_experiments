# Rectangular 10k by 40k with 260 nonzeros per row, D and D^T kept separate (sepA)

The study behind the paper's GPU table. The residual `r = D w - x` is its own step, so the
extracted iteration multiplies by `D` and `D^T` (2.6M nonzeros each). `D^T D` would have
551M nonzeros (34.4% dense), so only this formulation is generated. The hand-written
baselines (`source/baselines.py`) also keep `D` and `D^T` separate.

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

`production_interleaved_7rounds.txt` holds the raw timings: seven interleaved rounds in one
job on one AMD EPYC 7H12 + NVIDIA A100 node. `summary.csv` holds the medians, in seconds per
200-iteration solve.

| method | Hand-written SciPy (1 core) | Generated OpenMP (16 threads) | Hand-written CuPy (cuSPARSE) | Generated CUDA |
|---|---:|---:|---:|---:|
| ISTA | 1.250 | **0.08411** | 0.04781 | **0.02192** |
| FISTA | 1.264 | **0.08431** | 0.05585 | **0.02690** |

| speedup | CPU: generated OpenMP over SciPy (1 core) | GPU: generated CUDA over CuPy (cuSPARSE) |
|---|---:|---:|
| ISTA | 14.9x | 2.18x |
| FISTA | 15.0x | 2.08x |

## Agent time

Wall-clock minutes per solver. These include the package's checks after each phase and the
queueing of the agent's Slurm build jobs.

| solver | translation | code optimization | total |
|---|---:|---:|---:|
| ISTA OpenMP | 6.3 | 17.7 | 24.3 |
| ISTA CUDA | 8.0 | 30.4 | 38.8 |
| FISTA OpenMP | 8.1 | 20.6 | 29.1 |
| FISTA CUDA | 8.8 | 25.9 | 35.1 |
| all four | 31.2 | 94.7 | 127.3 |

## Verification

All four solvers were generated with FOMTools' built-in translation and code optimization
instructions. All four passed the eight-sample generated-oracle check at `1e-5` immediately
before timing (`verify_*.log`). `OPTIMIZATIONS_*.md` hold the agents' reports, and
`checksums.txt` records the solver sources and the binaries that were timed.

## Reproduction sources

`source/` holds the sepA problem definitions (`paper_*_sparse20k.py`, run with 10,000 rows,
40,000 columns and 260 nonzeros per row), the hand-written baselines and the complete pipeline
(`source/pipeline/`).
