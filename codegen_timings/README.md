# Generated code timings (paper §6.6)

Embedded C solvers for every algorithm variant of the four verification
experiments, generated on **exactly the instances and parameter sets that were
verified**, and timed against a direct NumPy loop at 200 iterations per solve.

| Experiment | Verified run (instance source) | Solvers |
|---|---|---|
| lasso | `lasso/lasso_verify/results/run_002` (10 x 15) | `ista`, `fista` |
| portfolio | `portfolio/results/run_002` (n = 10, drift 0.15, z̄ = 0.3) | `admm` (box and simplex share the iteration; checked on both sets) |
| network | `network/results/run_001` (10 x 15) | `unscaled`, `ruiz`, `unscaled_momentum`, `ruiz_momentum` |
| rapdhg | `control/rapdhg/verify/results/run_001` (horizon 5) | `R12`, `R6`, `pdhg` |

Each `<exp>/recording.py` rebuilds the instance from that run's `config.yaml`
through the experiment's `cluster_run.py`, records one iteration for codegen
(schedule tables for FISTA momentum, network momentum and rAPDHG's
coefficients and restarts), and supplies the NumPy loop and the verified
parameter samples.

## Running

```bash
uv run python -m codegen_timings.run <exp> --check-only          # no agent; seconds
uv run python -m codegen_timings.run <exp> --variants <variant>  # both agent phases
uv run python -m codegen_timings.summarize                       # -> timings.csv
uv run python -m codegen_timings.paper_csv                       # -> ../fomtools_paper/data/codegen_timing.csv (§6.6 table)
```

`--check-only` compares fomtools' reference oracle on the extracted program
with the NumPy loop, and the NumPy loop with the verification's own recurrence.
The agent phases use the Claude backend with a 10-minute budget each
(translation per attempt, code optimization once). A directory that already
holds a verified solver is reused unless `--force`.

## Output, per variant

```
<exp>/<variant>/
  check.json     pre-flight checks
  emitted/       translation phase: the full generated tree, agent_translation.log
  optimized/     code optimization phase on a copy: OPTIMIZATIONS.md, agent_code_optimization.log
  timing.json    NumPy / translated / optimized us per solve, speedups, max |C - NumPy|,
                 agent wall time, machine (CPU, compiler, versions)
```

## Timing protocol

- NumPy: a direct single-instance loop of the same iteration, no residuals,
  median of 50 solves after 3 warm-ups, at the first verified parameter sample.
- C: `make bench` in each solver directory (`-O3`, mean over 5000 solves cycling
  through the verify samples).
- 7 interleaved rounds; the median per arm is reported.
- Agreement: max |C - NumPy| over 20 verified-set samples at 200 iterations, for
  both the translated and the optimized solver.
