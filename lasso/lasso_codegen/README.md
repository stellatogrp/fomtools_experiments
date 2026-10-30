# Large-scale Lasso: generated OpenMP and CUDA solvers

The paper's GPU experiment (Sec. 6.2, Tables `tab:codegen_ista_fista` and
`tab:codegen_lasso_agent_time`): ISTA and FISTA for

$$
\min_{z}\; \frac{1}{2}\lVert D z - x \rVert_2^2 + 0.02\lVert z \rVert_1,
$$

with a sparse random $D \in \mathbf{R}^{10{,}000 \times 40{,}000}$ that has 260
nonzeros per row (2.6M in total), 200 iterations per solve in double precision,
generated for OpenMP (16 threads) and CUDA and timed against hand-written SciPy
(1 core) and CuPy (cuSPARSE) iterations. The iteration is written with the residual
$r = D w - x$ as its own step, so the extracted iteration multiplies by $D$ and
$D^\top$ separately, as a hand-written solver would; the precomputed
$I - \eta D^\top D$ would be 34% dense, with 212 times the nonzeros of $D$.

`results/rect10k40k_dense_sepA/` (in the `paper-rerun-outputs` release) is the archived
run behind the earlier paper draft (fomtools `b41c10a`), the reference for the rerun. Its FISTA momentum was one iteration early (coefficients
$\gamma_1, \gamma_2, \dots$ where the paper's eq. (8) has $0, \gamma_1, \dots$), in the
generated solvers and the baselines alike. This pipeline uses eq. (8), and its gate
holds FISTA to a textbook FISTA.

| file | what it does |
|---|---|
| `problem.py` | the instance, the ISTA/FISTA recordings, the momentum schedule, a textbook FISTA |
| `baselines.py` | SciPy and CuPy iterations and their timings |
| `gate.py` | before any agent: SciPy (and, for FISTA, the textbook iteration) against fomtools' reference execution |
| `generate.py` | every solver: translation (`generate_code`), then code optimization (`run_code_optimization`, in rounds), three repetitions |
| `benchmark.py` | on one A100 node: rebuild and re-verify every solver, then seven interleaved rounds |
| `della/` | `env.sh` (modules, flags, threads), the `make` and compiler wrapper, the holder loop, and the job files |

## How it runs on della

- **Agents on the login node.** Compute nodes cannot reach the Claude API, so
  `generate` and its coding agent run in tmux on della9.
- **Every build in a holder job.** `della/env.sh` puts `della/make` first on `PATH`,
  with `cc`, `gcc`, `clang` and `nvcc` linked to it. Every make target that builds or
  runs code, and every compiler call, runs with `srun --overlap` in the current
  holder: a 1 h job with one 80 GB A100 and 16 cores (`della/holder.sbatch`). della
  starts one-hour `gputest` jobs at once, while longer GPU jobs wait for days.
  `della/holders.sh` keeps one running and starts the next near the end of the
  current one. Without a holder the wrapper waits up to 30 minutes and then refuses;
  it never builds on the login node. The login node runs the agent, extraction,
  scaffolding and fomtools' reference executions for the verify data.
- **The benchmark only in an A100 batch job.** `della/bench.sbatch` (exclusive node)
  and `benchmark.py` both refuse to run outside a Slurm job or without a visible A100.
- **Resumable.** A solver is done when its `record.json` exists; `generate` skips
  those and regenerates an incomplete one from scratch, so after a login-node reboot
  it is restarted with the same command. Holders end on their own within the hour.
- **Never the della-stellato GPUs.** Every job uses the general partitions, account
  `bstellato`.

## Running

From the project root on della, `/scratch/gpfs/BSTELLATO/$USER/fomtools_rerun`, after
`slurm_agent.py sync up` from the laptop. Commands that source `env.sh` must run in
bash (it refuses zsh).

```bash
# Once: fomtools at the pinned commit next to the project, the environment, CuPy.
git clone https://github.com/stellatogrp/fomtools ../fomtools    # if missing
git -C ../fomtools fetch -q origin && git -C ../fomtools checkout -q --detach 25b3f73
uv sync
uv pip install --python .venv/bin/python cupy-cuda13x            # again after any uv sync
mkdir -p slurm/logs                                              # the jobs' --output

# Before any agent: the baselines against fomtools' reference (CPU job).
sbatch lasso/lasso_codegen/della/gate.sbatch                     # BASELINE_GATE_OK

# The agents (tmux on della9): window 1 keeps a holder, window 2 generates.
tmux new -s lasso
bash lasso/lasso_codegen/della/holders.sh
bash -c 'source lasso/lasso_codegen/della/env.sh && cd lasso && exec "$PY" -m lasso_codegen.generate'

# The final benchmark, once every solver has a record.json.
sbatch --export=ALL,RUN=v0.0.4 lasso/lasso_codegen/della/bench.sbatch
```

`generate` writes `lasso/out/codegen/rep<k>/<algorithm>_<backend>/` (untracked), each
ending with `record.json`: the fomtools commit, dirty flag, version and path, the build
node and GPU, the agent time per phase, the rounds, and what the agent says the rules
blocked. `--algorithms`, `--backends` and `--reps` restrict it. The benchmark writes
`results/<run>/`: `timings_raw.csv`, `summary.csv` (the median over rounds per
repetition, the median over repetitions, and the paper's two speedups),
`metadata.txt`, the verify logs and every solver's `record.json` and `OPTIMIZATIONS.md`.

For a smoke test of the Python side, shrink the instance with `LASSO_CODEGEN_M`,
`LASSO_CODEGEN_N` and `LASSO_CODEGEN_K`, e.g. `LASSO_CODEGEN_M=40 LASSO_CODEGEN_N=160
LASSO_CODEGEN_K=8 uv run python -m lasso_codegen.gate` from `lasso/`.

## Changes from the archived run

- FISTA momentum as in the paper's eq. (8) (see above).
- fomtools `25b3f73`: the agent may rewrite the matrix-vector products and use fused,
  step and whole-iteration routines under exact checks; code optimization runs in up
  to five rounds; the large-scale Makefiles build the matrix data once.
- Three repetitions of every solver; the paper reports their median and range.
- The agent's builds go to 1 h A100 80 GB holders (EPYC 7413 nodes); the benchmark runs
  on one A100 80 GB node. The archived benchmark used an EPYC 7H12 + A100 40 GB node.
- `cudatoolkit/13.3` (the archived run used 12.9), fomtools' generated Makefiles
  instead of a hand-written object cache, and no provenance or transcript gates.
