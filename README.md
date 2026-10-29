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
First, it translates the iteration into C or CUDA code. Second, it iteratively optimizes
the implementation. After each phase, the package compares the generated code with a
reference execution on sampled inputs. The package works with any coding agent. FOMTools
also serves as a CVXPYgen backend, generating and optimizing solvers for parametric CVXPY
problems. Our numerical examples cover sparse coding, network optimization, optimal
control, and portfolio rebalancing. On the small verified instances, the generated C
solvers are 37–190× faster than NumPy implementations of the same iterations, and code
optimization contributes 1.9–18.3× of this speedup. For a Lasso instance with 40,000
variables, the generated CUDA solvers are 2.1–2.2× faster than CuPy implementations. For
portfolio rebalancing with up to 3000 assets, solvers generated through CVXPYgen are
1.6–3.4× faster than CVXPYgen with OSQP at reaching a tolerance of 10⁻⁴.

## Dependencies

Install with [uv](https://docs.astral.sh/uv/):

```bash
git clone https://github.com/stellatogrp/fomtools_experiments
cd fomtools_experiments
uv sync
```

`uv sync` creates a Python 3.13 environment in `.venv/` and installs fomtools 0.0.5 from
PyPI (with its `codegen` and `gurobi` extras), the release that matches the paper. To
run against a local fomtools checkout instead, clone it next to this repository and run
`uv pip install -e "../fomtools[codegen,gurobi]"` after `uv sync`. In addition:

- **Verification** needs a [Gurobi](https://www.gurobi.com) license, set up as described in
  the [fomtools repository](https://github.com/stellatogrp/fomtools).
- **Code generation** needs a C compiler (and CMake for the CVXPYgen benchmark), the CUDA
  toolkit for GPU targets, and a coding agent. Set `FOMTOOLS_CODEGEN_AGENT=1` to let the
  scripts call the agent. The paper uses Claude Opus 5.5 through the Claude Agent SDK.
  Scripts that are run without the agent reuse the solvers already checked in or under
  `out/`.

## Instructions

### Running experiments

Unless a section says to `cd` into a folder, run the commands from the repository root.
Generated code and other large outputs go to untracked `out/` directories. Each folder's
README describes the problem, the methods, and the results in detail.

The verification experiments (lasso, network, and control) all run through one
[Hydra](https://hydra.cc) driver, `python -m fomexp.run experiment=<name>`. Its settings
are in [`conf/`](conf/), and the defaults reproduce the small instances. Each $K$ runs
under a one-hour Gurobi time limit with a 5% gap, $K = 1, 2, \dots$ warm-start each other,
and a run stops at a 20-hour budget. We therefore recommend a computing cluster with
16+ cores per job. A run writes `<experiment>/results/run_NNN/`, and
`python -m fomexp.collect <experiment> run_NNN` merges its per-variant CSVs. Any setting
can be overridden on the command line, e.g. a short local check:

```bash
uv run python -m fomexp.run experiment=lasso solver.time_limit=10 schedule.k_max=5
```

#### Lasso (Section 6.2)

Worst-case verification of ISTA and FISTA on a 25 × 50 sparse coding instance
([`lasso/lasso_verify/`](lasso/lasso_verify/)):

```bash
uv run python -m fomexp.run experiment=lasso experiment.problem.m=25 experiment.problem.n=50 experiment.problem.lambd=0.001
uv run python -m fomexp.collect lasso run_NNN
```

C solvers for the verified instance, timed against NumPy
([`codegen_timings/`](codegen_timings/)):

```bash
uv run python -m codegen_timings.run lasso --check-only
FOMTOOLS_CODEGEN_AGENT=1 uv run python -m codegen_timings.run lasso
```

Large-scale OpenMP and CUDA solvers, up to 325,000 × 1,300,000
([`lasso/lasso_codegen/`](lasso/lasso_codegen/)). Each study's result directory lists the
sources and the cluster jobs it ran with. The pipeline is:

```bash
cd lasso
FOMTOOLS_CODEGEN_AGENT=1 uv run python -m lasso_codegen.generate
uv run python -m lasso_codegen.benchmark
```

#### Network optimization (Section 6.3)

PDHG and momentum PDHG on a min-cost network flow with gains, unscaled and with Ruiz
equilibration ([`network/`](network/)):

```bash
uv run python -m fomexp.run experiment=network experiment.problem.ns=10 experiment.problem.nd=15 \
    experiment.problem.edge_prob=0.3 experiment.problem.seed=285 experiment.problem.supply=6 \
    experiment.problem.warm_start=center experiment.problem.eta_scale=0.8
uv run python -m fomexp.collect network run_NNN
```

C solvers for the four verified variants, and the scheduled vs. unrolled momentum
comparison:

```bash
FOMTOOLS_CODEGEN_AGENT=1 uv run python -m codegen_timings.run network
FOMTOOLS_CODEGEN_AGENT=1 uv run python -m codegen_timings.network_schedule
```

#### Optimal control (Section 6.4)

Model predictive control of a quadcopter ([`control/`](control/)). First, verify
restarted accelerated PDHG (restart periods 12 and 6) against plain PDHG on a horizon-5
program ([`control/rapdhg/verify/`](control/rapdhg/verify/)):

```bash
uv run python -m fomexp.run experiment=rapdhg experiment.problem.horizon=5
uv run python -m fomexp.collect rapdhg run_NNN
FOMTOOLS_CODEGEN_AGENT=1 uv run python -m codegen_timings.run rapdhg
```

Then generate the full rAPDHG controller and fly NumPy, translated C, and optimized C at
equal compute budgets ([`control/rapdhg/codegen/`](control/rapdhg/codegen/)):

```bash
cd control/rapdhg
FOMTOOLS_CODEGEN_AGENT=1 uv run python -m codegen.generate --max-iters 512
uv run python -m codegen.run_implementations
uv run python -m codegen.run_figures --reference figure8 --iterations 3 13 275 --no-gif
```

The same study with ADMM, which is not in the paper, is in [`control/osqp/`](control/osqp/).

#### Portfolio rebalancing (Section 6.5)

CVXPYgen with FOMTools ADMM against CVXPYgen with OSQP, solving to a tolerance of 10⁻⁴
for 200 to 3000 assets ([`portfolio/portfolio_cvxpygen/`](portfolio/portfolio_cvxpygen/),
results in [`RESULTS_tolerance_objective.md`](RESULTS_tolerance_objective.md)):

```bash
cd portfolio
FOMTOOLS_CODEGEN_AGENT=1 uv run python -m portfolio_cvxpygen.generate --force
uv run python -m portfolio_cvxpygen.tolerance_stop --rounds 3 --passes 3
uv run python -m portfolio_cvxpygen.plot
```

#### Figures and tables

Once the runs above have finished, these scripts write the plots, the probabilistic
(KL-inverse) bounds, and the CSVs the paper reads. The runs and $K$ cut-offs are set in
`RUNS` in `plots/make_plots.py`.

```bash
uv run python plots/make_plots.py              # plots/<experiment>.{csv,pdf,png}
uv run python plots/kl_bounds.py               # plots/kl_<experiment>.csv, kl_tables.md
uv run python plots/export_paper_data.py       # ../fomtools_paper/data/{lasso,network}_residual.csv, control_rapdhg.csv
uv run python -m codegen_timings.summarize     # codegen_timings/timings.csv
uv run python -m codegen_timings.paper_csv     # ../fomtools_paper/data/codegen_timing.csv
```

#### Supplementary: nonnegative least squares

PGD with and without Ruiz equilibration, verified with the MIP and SDP backends. This
experiment is not in the paper ([`nnls/`](nnls/)):

```bash
cd nnls
uv run python verify.py && uv run python check.py && uv run python plot.py
```

## Repository layout

| folder | contents |
|---|---|
| [`fomexp/`](fomexp/), [`conf/`](conf/) | Hydra driver and settings for every verification run |
| [`lasso/`](lasso/) | `lasso_verify/` (ISTA and FISTA verification), `lasso_codegen/` (large-scale OpenMP and CUDA) |
| [`network/`](network/) | network flow verification, with and without Ruiz equilibration |
| [`control/`](control/) | quadcopter MPC: `rapdhg/` (verification and code generation), `osqp/` (ADMM study), `common/` |
| [`portfolio/`](portfolio/) | `portfolio_cvxpygen/`, the CVXPYgen comparison |
| [`codegen_timings/`](codegen_timings/) | generated C solvers for the verified instances, with timings |
| [`plots/`](plots/) | figure, KL-bound, and paper-data scripts with their outputs |
| [`nnls/`](nnls/) | supplementary NNLS experiment |
