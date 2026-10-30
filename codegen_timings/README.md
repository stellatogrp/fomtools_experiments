# Generated code timings (paper Sections 6.2–6.4)

Embedded C solvers for every algorithm variant of the three verification
experiments, generated on **exactly the instances and parameter sets that were
verified**, and timed against a direct NumPy loop at 200 iterations per solve.

| Experiment | Verified run (instance source) | Solvers |
|---|---|---|
| lasso | `lasso/lasso_verify/results/run_022` (25 x 50, lambda 0.001) | `ista`, `fista` |
| network | `network/results/run_005` (10 x 15, 49 edges, supply 6, eta 0.8/\|\|A\|\|) | `unscaled`, `ruiz`, `unscaled_momentum`, `ruiz_momentum` |
| rapdhg | `control/rapdhg/verify/results/run_001` (horizon 5) | `R12`, `R6` |

Each `<exp>/recording.py` rebuilds the instance from that run's `config.yaml`
through the experiment's `cluster_run.py`, records one iteration for codegen
(schedule tables for FISTA momentum, network momentum and rAPDHG's
coefficients and restarts), and supplies the NumPy loop and the verified
parameter samples.

## Running

From the repository root, with fomtools 0.0.5 from `uv sync` (the paper's runs used
commit `25b3f73`, whose code generation is the same as in 0.0.5). Every paper number is
the median over three independent agent repetitions:

```bash
uv run python -m codegen_timings.run lasso --check-only                       # no agent; seconds
FOMTOOLS_CODEGEN_AGENT=1 uv run python -m codegen_timings.run lasso --force    # reps 1 2 3
FOMTOOLS_CODEGEN_AGENT=1 uv run python -m codegen_timings.run network --force
FOMTOOLS_CODEGEN_AGENT=1 uv run python -m codegen_timings.run rapdhg --force
FOMTOOLS_CODEGEN_AGENT=1 uv run python -m codegen_timings.network_schedule --force   # scheduled vs. unrolled, 600 edges, K = 50
uv run python -m codegen_timings.summarize        # -> timings.csv (medians, ranges), timings_reps.csv (every rep)
uv run python -m codegen_timings.paper_csv        # dry run: prints the numbers the text quotes
uv run python -m codegen_timings.paper_csv --write   # -> the paper's data/ (codegen_timing.csv, codegen_network_schedule.csv, ...)
```

Use `ANTHROPIC_MODEL=claude-opus-5-5` for the paper's agent. `--check-only`
compares fomtools' reference oracle on the extracted program with the NumPy loop,
and the NumPy loop with the verification's own recurrence. Each repetition is an
independent pair of agent runs: translation (10 minutes), then code optimization on
a copy in up to 5 rounds of 10 minutes each, stopping at a round that gains less
than 2% (`run_code_optimization`). `--reps 2 3` runs only some repetitions; a
directory that already holds a verified solver is reused unless `--force` (the
matrix data and verify harness, not committed, are regenerated first, which needs
the fomtools commit recorded in its `timing.json`).

## Output, per variant

```
<exp>/<variant>/                 repetition 1 (repetition k: <exp>/<variant>/reps/rep<k>/)
  check.json     pre-flight checks (untracked)
  emitted/       translation phase: the full generated tree, agent_translation.log (untracked)
  optimized/     code optimization phase on a copy; only OPTIMIZATIONS.md is tracked
  timing.json    NumPy / translated / optimized us per solve, speedups, max |C - NumPy|,
                 agent wall time, optimization rounds, what the rules blocked
                 (rule_friction), machine (CPU, compiler, versions, fomtools commit)
```

The repository tracks `timing.json` and `OPTIMIZATIONS.md` of every repetition. The
generated solvers and agent logs of the paper rerun are in the `paper-rerun-outputs`
release.

## Timing protocol

- NumPy: a direct single-instance loop of the same iteration, no residuals,
  median of 50 solves after 3 warm-ups, at the first verified parameter sample.
- C: `make bench` in each solver directory (`-O3`, mean over 5000 solves cycling
  through the verify samples).
- 7 interleaved rounds; the median per arm is reported.
- Agreement: max |C - NumPy| over 20 verified-set samples at 200 iterations, for
  both the translated and the optimized solver.
