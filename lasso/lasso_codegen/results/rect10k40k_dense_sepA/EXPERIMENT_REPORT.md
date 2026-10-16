# Rectangular 10k by 40k sparse Lasso with 260 nonzeros per row, D and D^T kept separate (sepA)

## Goal

Generate verified OpenMP and CUDA ISTA and FISTA solvers for a Lasso instance whose `D` is far
denser than in the earlier rectangular studies, written so that the extracted iteration keeps
`D` and `D^T` separate, and time them against hand-written baselines that do the same work:
SciPy on one CPU core against the generated OpenMP solvers, and CuPy/cuSPARSE against the
generated CUDA solvers on the same GPU.

## Method

**Instance.** `D` is a synthetic sparse random matrix with 10,000 rows, 40,000 columns and
2,599,983 nonzeros: 260 slots per row from 65 seeded permutations of the columns (duplicates
summed, leaving 257 to 260 per row and 64 or 65 per column), Gaussian values with standard
deviation `1/sqrt(260)`. Seed 0, `lambda = 0.02`, step size `1 / (||D||_1 ||D||_infinity)`
(0.0147039), 200 iterations, float64. `D^T D` would have 551,103,506 nonzeros (34.4% dense), so
the original formulation, which extracts `I - eta D^T D`, is not run.

**Formulation.** Each iteration is recorded as two steps (`source/paper_*_sparse20k.py`): the
residual `r = D w - x` (a soft-threshold at zero, which is exactly the identity, so the residual
is materialized as its own step) and `s+ = soft(w - eta D^T r, lambda eta)`, with `w = s` for
ISTA and `w = s + gamma_k (s - s_prev)` for FISTA. The extracted operators (`matrix_stats.md`)
are products with `D` and `D^T` plus identity blocks.

**Baseline gate.** Before any agent ran, a CPU job checked the hand-written SciPy iteration
against this run's FOMTools reference execution: they agree to 2.665e-15 (ISTA) and 1.724e-8
(FISTA) after 200 iterations (`baseline_gate.txt`).

**Generation.** Extraction, scaffolding and the deterministic OpenMP and CUDA objects ran as
CPU jobs. A coding agent then ran the translation and code optimization phases for each
algorithm/backend pair with FOMTools' built-in instructions and default agent driver,
verification tolerance `1e-5`; every build, verification and benchmark it requested ran in a
short Slurm job on an A100 node (AMD EPYC 7413, NVIDIA A100 SXM4 80 GB). Before each
translation the controller checked that the target directory held only FOMTools' scaffold.
After code optimization the translated and optimized sources were rebuilt from scratch and timed
in the same job, the faster was kept (code optimization unless more than 2% slower) and
re-verified, a provenance check compared both sources with every other solver source on the
cluster storage (`provenance_*.log`), and a transcript check confirmed that none of the agent's
tool calls touched another run (`transcript_check_*.log`). The ISTA CUDA solver was generated
in its own controller run from the same fixed files; while that agent ran, every other run's
solver sources, reports and agent transcripts were unreadable.

**Baselines.** `source/baselines.py` implements ISTA and FISTA directly with `D` and `D^T`
(`D^T` stored as its own CSR matrix): SciPy, whose sparse matrix-vector products are
single-threaded, and CuPy on the GPU, whose products call cuSPARSE. Their times cover the solve;
CuPy includes downloading the solution, as the generated solvers do.

**Benchmark.** One Slurm job on `della-i12g1` (AMD EPYC 7H12, NVIDIA A100 PCIe 40 GB) with 16
cores and one A100 rebuilt the four solvers from their verified sources, re-verified them at
`1e-5`, checksummed them, re-checked both baselines (`baseline_check_*.log`; CuPy within 3.9e-10
of SciPy) and then ran seven interleaved rounds back to back. OpenMP used 16 threads
(`OMP_PLACES=cores`, `OMP_PROC_BIND=close`).

## Results

Median of seven rounds, seconds per 200-iteration solve:

| method | Hand-written SciPy (1 core) | Generated OpenMP (16 threads) | Hand-written CuPy (cuSPARSE) | Generated CUDA |
|---|---:|---:|---:|---:|
| ISTA | 1.250 | 0.08411 | 0.04781 | 0.02192 |
| FISTA | 1.264 | 0.08431 | 0.05585 | 0.02690 |

Speedups: generated OpenMP over SciPy 14.9x (ISTA) and 15.0x (FISTA); generated CUDA over CuPy
2.18x and 2.08x; generated CUDA over generated OpenMP 3.84x and 3.13x.

Round-level ranges (seconds):

| method | SciPy | OpenMP | CuPy | CUDA |
|---|---|---|---|---|
| ISTA | 1.239-1.263 | 0.0834-0.0842 | 0.04742-0.04847 | 0.02191-0.02193 |
| FISTA | 1.250-1.285 | 0.0838-0.0871 | 0.05550-0.05667 | 0.02688-0.02693 |

Final verification (eight samples, tolerance `1e-5`), worst max-absolute error:

| solver | OpenMP | CUDA |
|---|---:|---:|
| ISTA | 0.000e+00 | 2.665e-15 |
| FISTA | 3.274e-08 | 3.132e-08 |

Code optimization, from the fresh-build head-to-head on the agent's node (milliseconds per
solve; 5 solves after 8 warm-ups):

| solver | translation | code optimization | speedup |
|---|---:|---:|---:|
| ISTA OpenMP | 63.77 | 58.92 | 1.08x |
| ISTA CUDA | 28.29 | 21.45 | 1.32x |
| FISTA OpenMP | 231.47 | 100.37 | 2.31x |
| FISTA CUDA | 67.92 | 26.67 | 2.55x |

Agent wall-clock minutes (including the controller's checks after each phase and the queueing
of the short build jobs):

| solver | translation | code optimization | total |
|---|---:|---:|---:|
| ISTA OpenMP | 6.3 | 17.7 | 24.3 |
| ISTA CUDA | 8.0 | 30.4 | 38.8 |
| FISTA OpenMP | 8.1 | 20.6 | 29.1 |
| FISTA CUDA | 8.8 | 25.9 | 35.1 |
| all four | 31.2 | 94.7 | 127.3 |

## Analysis

- On the GPU, where both sides use the same A100 and the same `D` and `D^T` products, the
  generated CUDA solvers are 2.18x (ISTA) and 2.08x (FISTA) faster than CuPy/cuSPARSE.
- On the CPU the generated OpenMP solvers are 14.9x and 15.0x faster than SciPy, whose sparse
  products run on one core while the generated solvers use 16 threads.
- All four solvers drop the soft-threshold at zero and compute the loop-invariant parameter
  term `-p0` once, using it to seed the accumulator. The OpenMP solvers and the FISTA CUDA
  solver also turn the identity state-update and residual products into in-place updates or
  pointer bookkeeping. For FISTA both backends recognize that `t0_state = [0, B]` and
  `t0_state__d_gamma = [-B, B]` share one block, form the extrapolated point `y` as a vector
  and replace two products by one product per iteration (on CUDA, one 2.6M-nonzero product
  with `D` replaces 7.8M nonzeros of products); the CUDA solver keeps the iterates in a three-buffer ring so the
  state update is a pointer rotation.
- The ISTA CUDA solver keeps the identity products but re-launches the fixed CSR kernels of
  FOMTools' operator file with launch shapes tuned for the A100 (for the 10,000-row product
  with `D`, 435 blocks of 23 threads instead of 40 blocks of 256, so far more of the 108 SMs
  receive work; one thread still computes one whole row, so results are unchanged), runs the
  identity product on a second stream, captures the whole solve as one CUDA graph, and chooses
  an L2 persisting window at setup by timing seven candidates.
- The remaining time is in the two products with `D` and `D^T` from the fixed operator files
  (about 99% of the optimized ISTA OpenMP solve and 91% of the FISTA OpenMP solve, per the
  agents' profiles).
