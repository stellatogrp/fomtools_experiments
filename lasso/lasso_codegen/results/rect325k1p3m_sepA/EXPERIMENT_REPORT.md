# Rectangular 325k by 1.3M sparse Lasso, D and D^T kept separate (sepA)

## Goal

The original formulation of the 325k by 1.3M study makes FOMTools extract the merged
operator `I - eta D^T D`, which has 7.5 times the nonzeros of `D`. This study writes the same
method so that the extracted iteration keeps `D` and `D^T` separate, generates verified
OpenMP and CUDA solvers for it, and times them against hand-written SciPy and
CuPy/cuSPARSE baselines and against the original-form generated solvers, in the same rounds
on one node.

## Method

The instance is unchanged: `D` is a synthetic sparse random matrix with 325,000 rows,
1,300,000 columns and 2,600,000 nonzeros (eight per row from two seeded column permutations,
Gaussian values), seed 0, `lambda = 0.02`, step size `1 / (||D||_1 ||D||_infinity)`, 200
iterations, float64.

**Formulation.** Each iteration is recorded as two steps (`source/paper_*_sparse20k.py`):
the residual `r = D w - x` (a soft-threshold at zero, which is exactly the identity, so the
residual is materialized as its own step) and `s+ = soft(w - eta D^T r, lambda eta)`, with
`w = s` for ISTA and `w = s + gamma_k (s - s_prev)` for FISTA. The extracted operators
(`matrix_stats.md`) are products with `D` and `D^T` plus identity blocks; `D^T D` is never
formed. Before any agent ran, `sepa_check.txt` compared FOMTools' reference execution of both
formulations: the step sizes are identical, and the iterates agree to 9.8e-14 (ISTA) and
3.5e-7 (FISTA) after 200 iterations.

**Generation.** Extraction, scaffolding and the deterministic OpenMP and CUDA objects ran
as CPU jobs. A coding agent then ran the translation and code optimization phases for all
four algorithm/backend pairs with FOMTools' built-in instructions and default agent driver,
verification tolerance `1e-5`; every build, verification and benchmark it requested ran in a
short Slurm job on an AMD EPYC 7H12 + A100 node (`della-i12g1`). Before each translation the
controller checked that the target directory held only FOMTools' scaffold. After code
optimization the translated and optimized sources were rebuilt from scratch and timed in the
same job, the faster was kept (code optimization unless more than 2% slower) and re-verified,
and a provenance check compared both sources with every other solver source on the cluster
storage and scanned the agent logs (`provenance_*.log`). The original-form ISTA CUDA solver
was generated for this comparison by the same controller, from a copy of the original-form
scaffold of `rect325k1p3m_tol1e-5` (its fixed files checked against the recorded checksums), on
`della-l09g3` and `della-l09g6` (AMD EPYC 7413, NVIDIA A100 SXM4 80 GB).

**Baselines.** `source/baselines.py` implements ISTA and FISTA directly with `D` and `D^T`:
SciPy on one CPU core, and CuPy on the GPU (sparse products through cuSPARSE). Before timing,
both were checked against the FOMTools-form iteration (`baseline_check_*.log`; ISTA within
1e-13, FISTA within 3.5e-8, CuPy within 5e-9 of SciPy). Their times cover the solve; CuPy
includes downloading the solution, as the generated solvers do.

**Benchmark.** All eight timed solvers (four sepA; four original-form: the OpenMP solvers
and FISTA CUDA solver of `rect325k1p3m_tol1e-5` and the ISTA CUDA solver generated here) were
rebuilt from their verified sources, re-verified at `1e-5` and checksummed on `della-i12g1`.
Seven interleaved rounds followed, back to back in one job on `della-i12g1` that held the
whole node (no other job could run on it), checked against the recorded checksums;
`rounds_audit.txt` confirms that no other job overlapped any round. OpenMP used 16 threads
(`OMP_PLACES=cores`, `OMP_PROC_BIND=close`). The one-minute load during the rounds was between
0.6 and 7.3 on 128 cores, all from the benchmark itself, and the GPU was idle at the start of
every round.

## Results

Median of seven rounds, seconds per 200-iteration solve:

| method | FOMTools reference (NumPy) | Hand-written SciPy | Generated OpenMP, original | Generated OpenMP, sepA | Hand-written CuPy | Generated CUDA, original | Generated CUDA, sepA |
|---|---:|---:|---:|---:|---:|---:|---:|
| ISTA | 7.304 | 4.368 | 1.133 | 0.4339 | 0.05050 | 0.1261 | 0.02368 |
| FISTA | 18.55 | 4.622 | 1.252 | 0.5517 | 0.05941 | 0.1289 | 0.02672 |

Round-level ranges (seconds):

| method | FOMTools reference | SciPy | OpenMP, original | OpenMP, sepA | CuPy | CUDA, original | CUDA, sepA |
|---|---|---|---|---|---|---|---|
| ISTA | 7.165-7.520 | 4.242-4.490 | 1.1294-1.1345 | 0.4330-0.4351 | 0.04998-0.05090 | 0.1261-0.1262 | 0.02362-0.02374 |
| FISTA | 18.46-18.59 | 4.606-4.744 | 1.2501-1.2544 | 0.5506-0.5532 | 0.05872-0.06015 | 0.1289-0.1292 | 0.02658-0.02696 |

Final verification (eight samples, tolerance `1e-5`), worst max-absolute error:

| solver | sepA | original form |
|---|---:|---:|
| ISTA OpenMP | 0.000e+00 | 0.000e+00 |
| ISTA CUDA | 4.441e-15 | 8.704e-14 |
| FISTA OpenMP | 7.913e-07 | 1.403e-06 |
| FISTA CUDA | 6.968e-07 | 1.377e-06 |

Code optimization of the sepA solvers, from the fresh-build head-to-head on `della-i12g1`
(seconds per solve; 5 solves after 8 warm-ups):

| solver | translation | code optimization | speedup |
|---|---:|---:|---:|
| ISTA OpenMP | 0.633 | 0.434 | 1.46x |
| ISTA CUDA | 0.0464 | 0.0227 | 2.04x |
| FISTA OpenMP | 1.925 | 0.550 | 3.50x |
| FISTA CUDA | 0.1467 | 0.0259 | 5.66x |

The original-form ISTA CUDA solver went from 0.1344 s (translation) to 0.1065 s (code
optimization), 1.26x, in the same kind of head-to-head on `della-l09g6`.

## Analysis

- Against hand-written baselines that already keep `D` and `D^T` separate, the sepA
  generated solvers are 10.1x (ISTA) and 8.4x (FISTA) faster on the CPU and 2.1x and 2.2x
  faster than CuPy/cuSPARSE on the GPU. The original-form generated CUDA solvers are 2.5x
  (ISTA) and 2.2x (FISTA) slower than CuPy, because each of their iterations reads the merged
  operators.
- Each sepA iteration reads about 5.2M matrix nonzeros instead of 19.5M (ISTA) and, after the
  agents' FISTA rewrite, about 5.2M (OpenMP) to 6.6M (CUDA) instead of 30M (FISTA), so the sepA solvers are
  2.3x to 5.3x faster than the original-form ones on the same hardware.
- In their final summaries the agents report checking the extracted operators numerically and
  exploiting the identity blocks of the sepA iteration: the state-update and residual
  identity products become in-place updates or pointer rotations, behind setup-time probes
  with a generic fallback. For FISTA, the two momentum terms `t0_state = [0, A]` and
  `t0_state__d_gamma = [-A, A]` (up to sign) share one block, so one product with `D`
  replaces two. The
  remaining work is the two products with `D` and `D^T` in the fixed operator files (about 94%
  of the OpenMP ISTA solve, 81% of the CUDA FISTA solve).
- For the original-form ISTA CUDA solver the agent reports removing the identity state-update
  product and updating the state in place, keeping the state vector in an L2 persisting
  window (set-aside 1.5 times the window), and re-priming the accumulator with the hoisted
  parameter term on a side stream overlapped with the matrix-vector product. The product with
  the merged 19.5M-nonzero operator in the fixed operator file remains 95% of its solve.
- Each round held the whole node, so no other job shared it during a timed measurement.
