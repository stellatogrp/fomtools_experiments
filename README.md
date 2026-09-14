# FOMTools experiments

Code to reproduce the experiments in the FOMTools paper.

[FOMTools](https://github.com/stellatogrp/fomtools) is a Python toolbox for analyzing
first-order optimization methods: it verifies the worst-case behavior of iterative
algorithms under parameter uncertainty and generates embedded C code for them. The
library, its documentation, and its tutorials live in the
[fomtools repository](https://github.com/stellatogrp/fomtools); this repository only
holds the scripts behind the paper's results.

## Setup

Clone this repository next to fomtools and install with [uv](https://docs.astral.sh/uv/):

```bash
git clone https://github.com/stellatogrp/fomtools
git clone https://github.com/stellatogrp/fomtools_experiments
cd fomtools_experiments
uv sync
```

`uv sync` installs fomtools (with its `codegen` extra) in editable mode from `../fomtools`,
so the experiments always run against the local checkout. Solver licenses (e.g. Gurobi)
are set up as described in the [fomtools repository](https://github.com/stellatogrp/fomtools).

## Experiments

| folder | experiment |
|---|---|
| [`control/`](control/) | model predictive control of a quadcopter with generated C solvers |
| [`lasso/`](lasso/) | ISTA and FISTA code generation for CPU and CUDA backends |

Each folder has its own README explaining how to reproduce its results. Generated
outputs are written to untracked `out/` directories.
