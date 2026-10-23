# Rerun plan: every experiment in the current paper, on one fomtools commit

Started Oct 22, 2026. Worktree `~/Documents/projects/fomtools_experiments-v004`, branch
`bs/rerun-v0.0.4`, fresh from `origin/main` (`aac19d3`); nothing carried over from the earlier
`bs/rerun-v0.0.3` attempt (kept locally for reference only, not merged).

**fomtools:** `main` at `25b3f73` (#152 code optimization rounds, fused/step/iterate kernels,
`tol_kernel`; #154 CVXPY-style API). The main clone `../fomtools` is detached there. Tag
v0.0.4 only after every rerun is done (Bart).

**Paper:** `~/Library/CloudStorage/Dropbox/work/research/papers/2026/fomtools` (git, `d97ea9f`).
Not the Overleaf copy (abandoned). Do not write into it without asking (step 5).

## Decisions (Bart, Oct 22)

- CPU codegen (agent and timings) on this Mac, `stellato-mbp-office` (M4 Pro); report the M4 Pro.
- Large-scale GPU study on della: agent on the della9 login node (builds go to 1 h A100
  `gputest` holders), then the benchmark in an A100 batch job. Account `bstellato`, scratch
  `/scratch/gpfs/BSTELLATO/$USER/`, never della-stellato GPUs.
- Verification (Gurobi) on della, one job at a time, after the GPU benchmark.
- Three independent agent repetitions per generated solver; report medians and ranges.
- Large-scale FISTA: fix the momentum to gamma_k (the archived run used gamma_{k+2}).
- Experiments not in the paper are deleted from this branch (`c41237e`).
- Agent: Claude Opus 5.5 (`ANTHROPIC_MODEL=claude-opus-5-5`), fomtools defaults (5 rounds,
  2% minimum gain) unless an experiment sets its budgets (codegen_timings: 600 s per phase).
- Reproducibility (Bart): every paper result comes from a clean, documented entry point
  (`python -m ...`, plus Slurm job files only where della needs them), listed in order in the
  README section "Reproducing the paper"; no ad hoc shell scripts; resumable after a crash.
- Commands on della go through Bart's ControlMaster with `ssh -o BatchMode=yes`; stop on the
  first connection failure. Job submissions through the slurm helper, test-only first.

## Experiments

| id | paper result | folder | runs |
|---|---|---|---|
| V1 | Fig lasso_residual (25x50, K <= 20) | `lasso/lasso_verify` | della, Gurobi |
| V2 | Fig network_residual (4 variants, K <= 39) | `network` | della, Gurobi |
| V3 | Fig control_rapdhg (K_r = 12, 6; K <= 75) | `control/rapdhg/verify` | della, Gurobi |
| C1 | Tab codegen lasso / network / control (8 solvers) | `codegen_timings` | Mac, 3 reps |
| C2 | Tab codegen_network_schedule | `codegen_timings/network_schedule.py` | Mac, 3 reps |
| C3 | Tab codegen_ista_fista + agent-time table (10k x 40k, K = 260) | `lasso/lasso_codegen` | della, 3 reps |
| C4 | Fig control_codegen_flight + per-iteration times | `control/rapdhg/codegen` | Mac, 3 reps |
| C5 | Fig portfolio_codegen_scaling (n <= 3000) | `portfolio/portfolio_cvxpygen` | Mac, 3 reps |

Numbers the paper quotes without a data file (verification hours and model sizes, agent
times, max deviations, flight iteration counts, portfolio generation times) get a tracked
results file in the producing folder.

## Steps

- [x] 0. Stop all earlier numerics (local batches, della tmux, holders; Oct 22).
- [x] 0b. Remove experiments not in the paper (`c41237e`).
- [x] 1. Migrate to the fomtools `25b3f73` API, one commit per folder: Verifier (`parameter`,
      `iterate`, `vp.relu/satlin/soft_threshold`, `maximize(ft.norm | ft.sum_squares)`,
      `.value`), sparse `MatrixData` (`.nnz()`, `.shape`), codegen through
      `run_code_optimization`, `--rep` repetitions, no generated `fom_data.c`/`test/` in git.
      Check: `uv run python -W error::DeprecationWarning` on each entry point's check-only path.
- [ ] 2. AI codegen (agent sessions): C1, C2, C4, C5 on the Mac; C3 on the della9 login node.
- [ ] 3. Numerics: timings (Mac), A100 benchmark (della), then V1, V2, V3 (della, Gurobi).
- [ ] 4. Compare with the paper; stop and ask at any difference above 20%.
- [ ] 5. Paper data export (ask first; the scripts must point at `papers/2026/fomtools/data`).

## Log

- 2026-10-22: plan written; fomtools pinned to `25b3f73` (#154 merged); dropped experiments removed
  (`c41237e`). Migration split in three parallel forks: verification (A), CPU codegen (B), GPU (C).
- 2026-10-22: migration done (forks A/B/C); verification smoke runs match the archived bounds; della
  verification via `python -m fomexp.submit` (`6c775f7`); README "Reproducing the paper" (`41af186`).
- 2026-10-22: C1, C2 done, 3 reps on `25b3f73` (`57dbe42`). Total vs NumPy 32-162x (paper 37-190x),
  code optimization 3.4-12x; network and control much faster than the paper, R6 slower (15 vs 10 us).
- 2026-10-23, decision (Bart): keep the new results; report medians and ranges of the 3 reps.
- 2026-10-23: C4 reps 1-2 (`a821899`) after the stale-library fix (`21e8013`); optimized C buys
  405 / 485 iterations at 40 us (paper 275). C3: 4 of 12 solvers; the rest retried after the
  overnight session-limit failures.

- 2026-10-23: C4 done, 3 reps (`a3c5e19`): optimized C buys 485 iterations at 40 us (median;
  paper 275), 0.082 us per iteration. C5 reps 1-2 timed: optimized vs OSQP 1.4 / 2.7 / 4.8 / 4.1x
  for n = 200 / 500 / 1000 / 3000 (paper 1.9 / 1.6 / 2.5 / 3.4x), iterations identical to the
  paper. Kept (Bart). The weekly Claude limit stopped the agents at 12:45 (reset 13:00); C5 rep 3
  and the remaining 7 C3 solvers restarted.
