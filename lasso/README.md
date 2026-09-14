# Lasso ISTA/FISTA backend experiments

This experiment generates the same fixed-count Lasso iteration for two target
backends: embedded C on a CPU and large-scale CUDA on a GPU. It includes both
ISTA and textbook FISTA so the scheduled-coefficient path is exercised as well
as the stationary path.

The point is backend coverage, not a claim that a GPU wins on a tiny problem.
At 80 variables, kernel-launch and host/device finalization costs are larger than
the useful parallel work, so optimized CUDA is slower than optimized CPU while
still substantially faster than the NumPy reference.

## Fixed experiment

The optimization problem is

```text
minimize_z  0.5 ||A z - b||_2^2 + 0.02 ||z||_1,
```

with the following fixed choices:

| setting | value |
|---|---:|
| rows of `A` | 40 |
| columns/state dimension | 80 |
| random seed | 0 |
| regularization `lambda` | 0.02 |
| step size | `1 / lambda_max(A.T @ A)` |
| iterations per solve | 200 |
| precision | float64 |
| verification samples | 8 |

Every arm uses the same matrix, parameter, initialization, iteration count, and
arithmetic precision. Between CPU and CUDA, only the code-generation target
changes. FISTA carries the two-slot state `(y[k-1], y[k])` and tabulates its
textbook momentum coefficient for all 200 iterations.

## Reproduction

From the repository root, with the sibling `fomtools` checkout on `main`:

```bash
uv sync
FOMTOOLS_CODEGEN_BACKEND=codex uv run python -m lasso.generate
```

The generation command runs both agent phases and writes four untracked solver
directories under `lasso/out/codegen/`. Each contains the generated source,
verification harness, full agent transcripts, and `OPTIMIZATIONS.md`.

Run the benchmark on one Della GPU node so all six arms see the same host:

```bash
uv run python -m lasso.benchmark \
  --rounds 7 \
  --generated-solves 5000 \
  --numpy-repetitions 50
```

Summarize either the checked-in run or a new run:

```bash
uv run python -m lasso.summarize
uv run python -m lasso.summarize lasso/out/paper_final_compute_7rounds.txt
```

Run the inexpensive recording/result integrity checks without invoking an agent
or compiling a solver:

```bash
uv run python -m lasso.check
```

Generated directories remain under ignored `out/`; the exact raw measurements,
verification logs, hardware record, and optimization reports used below are
checked into `results/paper_40x80/`.

The separate 20,000-by-20,000 sparse scaling study is archived in
[`results/sparse20k/`](results/sparse20k/). It uses the large-scale OpenMP and
CUDA targets with the same ISTA/FISTA style and fixed-count measurement
protocol.

## Hardware and software

- Host: `della-l08g1`
- CPU: AMD EPYC 7413 24-Core Processor
- GPU: NVIDIA A100-SXM4-80GB
- FOMTools commit: `b2127e772435d8311e8f10b0a71ba9cac0d01dbe`
- Python: 3.14.6
- CUDA compiler: 12.9

## Results

Median of the seven interleaved round-level measurements, in microseconds per
complete 200-iteration solve:

The NumPy arm is FOMTools' `ReferenceOracle` executing the extracted IR at the
midpoint parameter `p0 = b`; this is also the oracle used to produce generated
verification data.

| method | NumPy | optimized CPU | optimized CUDA | NumPy / CPU | NumPy / CUDA |
|---|---:|---:|---:|---:|---:|
| ISTA | 4209.380 | **271.029** | 778.881 | 15.53x | 5.40x |
| FISTA | 7604.105 | **264.897** | 577.938 | 28.71x | 13.16x |

CUDA took 2.87 times the CPU time for ISTA and 2.18 times the CPU time for
FISTA. That is expected at this scale: the state transform is only 80 by 80,
while a CUDA solve still pays launch, synchronization, and finalization costs.
The generated CUDA solver nevertheless keeps state device-resident across all
200 iterations and demonstrates that the same recorded method targets a GPU
backend without changing its mathematical definition.

## Optimization-phase results

The timings below are the generation-time Phase-A/Phase-B measurements recorded
inside each optimization report. They are separate from the seven-round table
above, which is the reported benchmark.

| method/backend | Phase A | Phase B | Phase-B speedup | outcome |
|---|---:|---:|---:|---|
| ISTA CPU | 789.822 us | 271.245 us | 2.91x | kept, re-verified |
| ISTA CUDA | 4202.010 us | 726.633 us | 5.78x | kept, re-verified |
| FISTA CPU | 4261.969 us | 267.021 us | 15.96x | kept, re-verified |
| FISTA CUDA | 9198.076 us | 558.110 us | 16.48x | kept, re-verified |

The complete source diffs and agent explanations are preserved in:

- `results/paper_40x80/OPTIMIZATIONS_ista_cpu.md`
- `results/paper_40x80/OPTIMIZATIONS_ista_cuda.md`
- `results/paper_40x80/OPTIMIZATIONS_fista_cpu.md`
- `results/paper_40x80/OPTIMIZATIONS_fista_cuda.md`

## Correctness checks

All optimized solvers were checked against the generated NumPy oracle over eight
parameter samples:

| solver | worst max-absolute error | tolerance | result |
|---|---:|---:|---|
| ISTA CPU | 6.661e-14 | 1.0e-6 | pass |
| ISTA CUDA | 5.640e-14 | 1.0e-6 | pass |
| FISTA CPU | 1.120e-11 | 1.0e-6 | pass |
| FISTA CUDA | 1.105e-11 | 1.0e-6 | pass |

ISTA additionally records three successive iterations and passes them to the
stationarity gate. FISTA uses the scheduled-extraction path, which probes the
momentum dependence and validates it against a held-out coefficient value. This
prevents the first zero-momentum FISTA iteration from being incorrectly frozen
and repeated as ISTA.

## Interpretation

The experiment supports two deliberately separate conclusions:

1. FOMTools can generate verified implementations of ISTA and scheduled FISTA
   for both CPU C and CUDA GPU targets from the same high-level recording.
2. A GPU is not automatically faster. With only 80 output rows, there is too
   little work per launch to amortize GPU overhead. The larger sparse scaling
   study should be reported separately after its complete solvers have passed
   verification and its timings are available.

No result from an incomplete larger run is included here.
