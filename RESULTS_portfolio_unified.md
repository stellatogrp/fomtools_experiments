# Portfolio: one problem for verification and the CVXPYgen comparison

Outcome (Oct 18, 2026, decided by Bartolomeo): portfolio becomes the paper's
code-generation-at-scale example (the CVXPYgen comparison,
`RESULTS_tolerance_objective.md`), placed last. Its verification is a short
appendix paragraph on a small instance that demonstrates the coupled parameter set.
The Portfolio row of the generated-code tables (`tab:codegen_portfolio`) is dropped;
`codegen_timings/portfolio/` keeps its run_002 results as an archive. Instructions
were sent to the paper session; this branch does not edit the paper.

## Setup

One definition, `portfolio/portfolio_cvxpygen/model.py`, used by both experiments:
factor model `Sigma = F F' + d I` (F in R^{n x k}, entries N(0, 1/k), d = 0.1),
kappa = 1, lambda = 0.01, z_bar = 0.3; forecasts `mu = mu_bar + IC sigma_i s_i`
(mu_bar ~ U[0, 0.5], IC = 0.1, |s_i| <= S_MAX = 3); holdings drifted from 1/n by
returns in [-0.15, 0.15].

The verification (`portfolio/problem.py`, `portfolio/cluster_run.py`,
`conf/experiment/portfolio.yaml`) builds the same instance at n = 5, k = 2:

- parameter `p = (x, mu)`: x on the drift box with `1'x = 1` (one
  `add_constraint`), mu on its forecast box; simplex variant only;
- the plain ADMM written in the paper's appendix: rho = 1, sigma = 1e-4, alpha = 1
  (the CVXPYgen timings use OSQP's sigma = 1e-6, alpha = 1.6, see below), no rho
  boost on equality rows (the float32 simulator loses 3e-4 with OSQP's 1e3 boost);
- warm start at the primal-dual solution for x = 1/n, mu = mu_bar;
- metric `||H (s^K - s^{K-1})||_inf` with `H = -(1/alpha) [[sigma I, M'R], [0, I]]`;
- `portfolio/check.py`: ADMM matches CVXPY on `model.build` (the exposure
  formulation) to 2e-8; the simulator matches NumPy to 3e-7.

## Runs (della, 16 cores, Gurobi 13.0.0, 5% gap, 3600 s per K)

| run | ADMM | K within 5% gap | first time-limited K | cancelled at |
|---|---|---|---:|---:|
| run_003 (job 15786480) | sigma 1e-4, alpha 1 | 1..8 | 9 (gap 42%) | K = 14 (gap 334%) |
| run_004 (job 15786486) | sigma 1e-6, alpha 1.6 | 1..7 | 8 (gap 82%) | K = 13 (gap 943%) |

Past the wall the best bound rises with K while the incumbent falls: the relaxation
weakens, so more cores or time would buy one or two K at most.

## Paper data (run_003)

`portfolio/paper_data/portfolio_residual.csv`, in the paper's column format; MIP
cells only where the gap is within 5% (K = 1..8); KL 95% bound and sample max to
K = 100 (`plots/kl_bounds.py`, fresh seeds 1 and 2, 1000 samples each).

| K | MIP bound | 95% bound | sample max |
|---:|---:|---:|---:|
| 1 | 0.427 | 0.375 | 0.406 |
| 4 | 0.280 | 0.149 | 0.215 |
| 8 | 0.127 | 0.076 | 0.102 |
| 100 | | 6.4e-4 | 1.0e-3 |

- The certified worst case falls 3.4x over 8 iterations (the paper's current claim,
  three orders of magnitude in 100 iterations, was for the old dense x-only n = 10
  instance and no longer applies).
- MIP bound / 95% bound: 1.14 to 1.88 (paper's current text: 1.2 to 2.3).
- Quick look: `portfolio/paper_data/portfolio_residual_K8.{pdf,png}`.

## Why the MIP stops at K = 8 (local study, n = 5, 3 threads)

- Varying mu on the full S_MAX = 3 box is the main cost: OBBT cannot fix the
  sell/buy ReLUs, so binaries grow with K (155 at K = 6 against 88 for the old
  instance).
- OSQP's constants (alpha 1.6, sigma 1e-6) add to it: K = 6 took 460 s, against
  8 s with the plain ADMM.
- Neither a dense Sigma (no exposure rows), S_MAX = 1, factor-driven mu nor
  neutral updates moved the wall by more than about two K. These are config
  switches (`exposures`, `s_max`, `mu_model`).
- n = 10 is out of reach with the factor model: x only took 823 s at K = 5.
- The old instance (run_002: dense, x only, narrow drift set) reached K = 131 in
  about 2 h; the full capped simplex (run_001) stalled at K = 26 in 20 h.

## Notes for fomtools (not changed here)

- OBBT stops runs with "Bounds cross by more than tol=1e-07" at crossings of about
  1.2e-7 (ended both run_002 variants at K = 120 and 132).
- The simulator is fixed to float32, so residuals below about 1e-4 times the scale
  of R v are unreliable when rho is boosted on equality rows.
- Code optimization prompt: two agents read SPEC.md's "compute every `matrix @
  source` via the helpers" as forbidding fused hand-written loops in `fom_solver.c`
  and gained about 1x; the Oct 14 agent fused them and gained 10.7x. The prompt could
  say the rule binds the translation phase only.

## Reproduce

```bash
cd portfolio && python check.py --n 5
python -m fomexp.run experiment=portfolio variant=simplex experiment.problem.n=5   # or slurm/portfolio.slurm on della
python plots/kl_bounds.py --experiment portfolio
cd plots && python export_paper_data.py --only portfolio --out ../portfolio/paper_data
```
