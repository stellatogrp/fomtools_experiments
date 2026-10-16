# Rectangular 325k by 1.3M, D and D^T kept separate (sepA)

This directory archives a third run of the 325,000 by 1,300,000 sparse Lasso study of
[`../rect325k1p3m/`](../rect325k1p3m/) and [`../rect325k1p3m_tol1e-5/`](../rect325k1p3m_tol1e-5/).
The instance, the step size and the algorithms are the same. What changes is how the
method is written for FOMTools, and what the generated solvers are compared against.

- **Formulation.** The earlier studies write the ISTA step as one affine map,
  `soft((I - eta D^T D) s + eta D^T x)`, so FOMTools extracts the precomputed matrix
  `I - eta D^T D` (19.5M nonzeros; FISTA: two operators with about 69M nonzeros). Here the
  residual `r = D w - x` is materialized as its own step and the gradient is applied as
  `D^T r`, so the extracted iteration multiplies by `D` and `D^T` (2.6M nonzeros each) and
  never forms `D^T D`. FOMTools' reference execution confirms the two formulations compute
  the same iterates (`sepa_check.txt`: identical step size; maximum difference after 200
  iterations 9.8e-14 for ISTA and 3.5e-7 for FISTA).
- **Baselines.** Besides FOMTools' reference execution (NumPy), every round times two
  hand-written baselines that keep `D` and `D^T` separate: plain SciPy on one CPU core, and
  CuPy on the GPU, whose sparse products call cuSPARSE (`source/baselines.py`). Both were
  checked against FOMTools' reference before timing (`baseline_check_*.log`).
- **Original-form solvers in the same rounds.** Four original-form generated solvers are
  timed in the same rounds, so the two formulations are compared on one node: the verified
  OpenMP solvers and FISTA CUDA solver of `rect325k1p3m_tol1e-5`, and an ISTA CUDA solver
  generated from scratch for this comparison with the same pipeline, from that study's
  original-form scaffold.

## Fixed problem

| setting | value |
|---|---:|
| `D` | synthetic sparse random matrix, 325,000 by 1,300,000 |
| nonzeros of `D` | 2,600,000 (8 per row, 2 per column, Gaussian values) |
| seed | 0 |
| regularization `lambda` | 0.02 |
| step size | `1 / (||D||_1 ||D||_infinity)` = 0.0730896 (as in the earlier studies) |
| iterations per solve | 200 |
| precision | float64 |
| verification tolerance | `1e-5` |

`matrix_stats.md` lists the extracted sepA operators.

## Final benchmark

`production_interleaved_7rounds.txt` is the authoritative raw timing record: seven
interleaved rounds on `della-i12g1` (AMD EPYC 7H12, NVIDIA A100 PCIe 40 GB), in one job
holding the whole node. Each round times, in order, FOMTools' reference (one warm-up, three
solves, median), hand-written SciPy (same), the generated OpenMP solvers (eight warm-ups, ten
solves, mean), hand-written CuPy (same) and the generated CUDA solvers, eight generated
solvers in all. `summary.csv` reports the median of the seven round values. Times are
seconds per 200-iteration solve.

| method | FOMTools reference (NumPy) | Hand-written SciPy | Generated OpenMP, original | Generated OpenMP, sepA | Hand-written CuPy | Generated CUDA, original | Generated CUDA, sepA |
|---|---:|---:|---:|---:|---:|---:|---:|
| ISTA | 7.304 | 4.368 | 1.133 | **0.4339** | 0.05050 | 0.1261 | **0.02368** |
| FISTA | 18.55 | 4.622 | 1.252 | **0.5517** | 0.05941 | 0.1289 | **0.02672** |

The FOMTools reference column here executes the sepA iteration; for the original form it
takes 14.81 s and 72.81 s (`rect325k1p3m_tol1e-5`).

| speedup of sepA over | hand-written SciPy | hand-written CuPy (cuSPARSE) | original-form OpenMP | original-form CUDA |
|---|---:|---:|---:|---:|
| ISTA | 10.1x | 2.1x | 2.6x | 5.3x |
| FISTA | 8.4x | 2.2x | 2.3x | 4.8x |

The original-form generated CUDA solvers are 2.5x (ISTA) and 2.2x (FISTA) slower than
hand-written CuPy, because every iteration reads the merged operators. Across rounds the
generated solvers varied by at most 1.5% and the hand-written baselines by at most 6%
(SciPy ISTA). `rounds_audit.txt` records that no other job overlapped any round.

## Verification and generation

The four sepA solvers were generated with FOMTools' built-in translation and code
optimization instructions and default agent driver, on `della-i12g1`; the original-form ISTA
CUDA solver was generated the same way on `della-l09g3` and `della-l09g6` (AMD EPYC 7413,
NVIDIA A100 SXM4 80 GB). Before each translation the controller checked that the target
directory held only FOMTools' scaffold, and before a solver was accepted a provenance check
compared it with every other solver source on the cluster storage and scanned the agent logs
(`provenance_*.log`; the closest match had line similarity 0.32, and a solver is rejected at
0.8). All eight timed solvers passed the eight-sample generated-oracle check at `1e-5`
immediately before timing (`verify_*.log`). `OPTIMIZATIONS_*.md` hold the translation to code
optimization report, with the agent's final summary, of each sepA solver and of the
original-form ISTA CUDA solver; the other original-form reports are in
`../rect325k1p3m_tol1e-5/`. `generation_metadata.txt` records the jobs and nodes,
`benchmark_metadata.txt` the benchmark node, and `checksums.txt` the solver sources and the
exact binaries that were timed.

## Reproduction sources

`source/` holds the sepA problem definitions, the hand-written baselines, the equivalence
check and the complete pipeline (`source/pipeline/`). The original-form problem definitions
are in `../rect325k1p3m/source/`.
