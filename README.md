# FOMTools experiments

This repository is by Vinit Ranjan, Daniel Deza, Anushka Acharya, Jisun Park, and
Bartolomeo Stellato, and contains the Python source code to reproduce the experiments in
our paper ["FOMTools: Development and AI-Assisted Code Generation for First-Order
Methods."](https://arxiv.org/abs/2610.26743)
The experiments rely on our package [FOMTools](https://github.com/stellatogrp/fomtools).

If you find this repository helpful in your publications, please consider citing our
paper and package:

```bibtex
@article{ranjan2026fomtools,
  title   = {{FOMTools}: Development and {AI}-Assisted Code Generation for First-Order Methods},
  author  = {Ranjan, Vinit and Deza, Daniel and Acharya, Anushka and Park, Jisun and Stellato, Bartolomeo},
  journal = {arXiv preprint arXiv:2610.26743},
  year    = {2026},
}
```

## Introduction

We present FOMTools, an open-source Python package for developing, analyzing, and
deploying first-order algorithms for parametric linear and convex quadratic optimization.
The user specifies an iteration in Python, with syntax close to its mathematical
description. The package represents the iteration as a computation graph of affine and
piecewise affine operations. To analyze worst-case performance, FOMTools formulates a
mixed-integer verification problem that maximizes a convergence residual over a specified
set of problem instances and initial iterates. For probabilistic performance analysis, a
batched GPU simulator collects algorithm trajectories and computes statistical bounds on
the probability of exceeding a specified convergence tolerance. From the same algorithm
description, FOMTools generates specialized solver code for embedded hardware, multicore
CPUs, and GPUs. The package first generates the numerical data, factorizations, and
matrix-vector routines without any agent. An LLM coding agent then works in two phases.
First, it translates the iteration into C or CUDA code. Second, it optimizes the
implementation in rounds. After each phase, the package compares the generated code with a
reference execution on sampled inputs. The package works with any coding agent. FOMTools
also serves as a CVXPYgen backend, generating and optimizing solvers for parametric CVXPY
problems. Our numerical examples cover sparse coding, network optimization, optimal
control, and portfolio rebalancing. On the small verified instances, the generated C
solvers are 32–162× faster than NumPy implementations of the same iterations, and code
optimization contributes 3.5–12× of this speedup. For a Lasso instance with 40,000
variables, the generated CUDA solvers are 3.3–4.6× faster than CuPy implementations. For
portfolio rebalancing with up to 3000 assets, solvers generated through CVXPYgen are
2.5–6.2× faster than CVXPYgen with OSQP at reaching a tolerance of 10⁻⁴. These are
medians over three independent agent runs per generated solver.

## Dependencies

Clone this repository next to fomtools and install with [uv](https://docs.astral.sh/uv/):

```bash
git clone https://github.com/stellatogrp/fomtools
git clone https://github.com/stellatogrp/fomtools_experiments
cd fomtools_experiments
uv sync
```

`uv sync` creates a Python 3.13 environment (`.python-version`) in `.venv/` and installs
fomtools (with its `codegen`, `gurobi` and `cvxpygen` extras) in editable mode from `../fomtools`, so the experiments
always run against the local checkout. In addition:

- **Verification** needs a [Gurobi](https://www.gurobi.com) license, set up as described in
  the [fomtools repository](https://github.com/stellatogrp/fomtools).
- **Code generation** needs a C compiler (and CMake for the CVXPYgen benchmark), the CUDA
  toolkit for GPU targets, and a coding agent. Set `FOMTOOLS_CODEGEN_AGENT=1` to let the
  scripts call the agent. The paper uses Claude Opus 5.5 through the Claude Agent SDK.
  Scripts that are run without the agent reuse the solvers already checked in or under
  `out/`.

## Reproducing the paper

Every result in the paper comes from one entry point below, run from the repository root
unless a line says `cd`. Code generation needs `FOMTOOLS_CODEGEN_AGENT=1` (the paper uses
Claude Opus 5.5: `ANTHROPIC_MODEL=claude-opus-5-5`); each generated solver is produced three
times by independent agent runs (`--reps 1 2 3`, the default), and the tables report the
median with the range. Everything is resumable: a finished piece is skipped unless
`--force`. Generated code goes to untracked `out/` folders; results, logs and reports are
tracked. Each folder's README has the details.

**Verification** (Sections 6.2-6.4; Gurobi, one Slurm array task per variant on Princeton's
della, `conf/cluster/della.yaml`). With no overrides the configs are the paper's instances
and horizons (lasso K <= 20, network K <= 39, control K <= 75):

```bash
uv run python -m fomexp.submit experiment=lasso --yes      # drop --yes for sbatch --test-only
uv run python -m fomexp.submit experiment=network --yes
uv run python -m fomexp.submit experiment=rapdhg --yes
uv run python -m fomexp.collect <experiment> run_NNN        # summary.csv, wide.csv, summary.json
```

Locally, `uv run python -m fomexp.run experiment=<name>` runs every variant in turn.

**Generated C solvers for the verified instances** (Tables in Sections 6.2-6.4 and the
appendix timing protocol; Apple M4 Pro):

```bash
FOMTOOLS_CODEGEN_AGENT=1 uv run python -m codegen_timings.run lasso --force
FOMTOOLS_CODEGEN_AGENT=1 uv run python -m codegen_timings.run network --force
FOMTOOLS_CODEGEN_AGENT=1 uv run python -m codegen_timings.run rapdhg --force
FOMTOOLS_CODEGEN_AGENT=1 uv run python -m codegen_timings.network_schedule --force   # scheduled vs. unrolled
uv run python -m codegen_timings.summarize
```

**Closed-loop flight** (Section 6.4; generate, extend the schedule tables, time each
implementation and fly the figure eight):

```bash
cd control/rapdhg && FOMTOOLS_CODEGEN_AGENT=1 uv run python -m codegen.run --force
```

**Portfolio rebalancing with CVXPYgen** (Section 6.5):

```bash
cd portfolio
FOMTOOLS_CODEGEN_AGENT=1 uv run python -m portfolio_cvxpygen.generate --force
uv run python -m portfolio_cvxpygen.tolerance_stop --rounds 3 --passes 3
uv run python -m portfolio_cvxpygen.plot
```

**Large-scale Lasso on a GPU** (Section 6.2; della: the agent runs on a login node, its
builds go to one-hour A100 holder jobs, and the benchmark runs in an A100 batch job). See
[`lasso/lasso_codegen/README.md`](lasso/lasso_codegen/README.md) for the one-time setup;
then, in tmux on the login node:

```bash
sbatch lasso/lasso_codegen/della/gate.sbatch                       # BASELINE_GATE_OK
bash lasso/lasso_codegen/della/holders.sh                          # window 1: keeps an A100 holder
bash -c 'source lasso/lasso_codegen/della/env.sh && cd lasso && exec "$PY" -m lasso_codegen.generate'   # window 2
sbatch --export=ALL,RUN=<name> lasso/lasso_codegen/della/bench.sbatch   # after generate finishes
```

**Paper data.** Both scripts are dry runs that print what they would write; `--write`
writes into the paper's `data/` (default
`~/Library/CloudStorage/Dropbox/work/research/papers/2026/fomtools/data`, or `--paper-data`
and `--paper-dir`). `paper_csv` also copies the flight figure of the median repetition to
the paper's `figures/`, and prints every number the paper's text quotes.

```bash
uv run python plots/make_plots.py && uv run python plots/kl_bounds.py
uv run python plots/export_paper_data.py [--write]
uv run python -m codegen_timings.paper_csv [--write]
```

## Repository layout

| folder | contents |
|---|---|
| [`fomexp/`](fomexp/), [`conf/`](conf/) | Hydra driver, settings and della submission for every verification run |
| [`lasso/`](lasso/) | `lasso_verify/` (ISTA and FISTA verification), `lasso_codegen/` (large-scale OpenMP and CUDA) |
| [`network/`](network/) | network flow verification, with and without Ruiz equilibration |
| [`control/`](control/) | quadcopter MPC: `rapdhg/verify/` and `rapdhg/codegen/`, shared `common/` |
| [`portfolio/`](portfolio/) | `portfolio_cvxpygen/`, the CVXPYgen comparison |
| [`codegen_timings/`](codegen_timings/) | generated C solvers for the verified instances, with timings |
| [`plots/`](plots/) | figure, KL-bound, and paper-data scripts with their outputs |
