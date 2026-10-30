# CVXPYgen portfolio sweep to tolerance 1e-4

These are the results behind the paper's portfolio figure, from the rerun on fomtools
`25b3f73` with three independent agent runs per generated solver. CVXPYgen with the FOMTools
ADMM solver is compared with CVXPYgen with OSQP, every solver stopping itself at the same
tolerance. The setup, the arms, the stopping test, and the shared ADMM settings are in
[`portfolio/portfolio_cvxpygen/README.md`](portfolio/portfolio_cvxpygen/README.md).

The iteration counts agree within 10%, so the speedup comes from cheaper iterations. In C,
the optimized FOMTools solver is 3.3x, 2.5x, 6.2x, and 5.8x faster than CVXPYgen + OSQP at
n = 200, 500, 1000, and 3000 (medians over the three runs). The translated solver is 1 to
17% slower. In one run (n = 200, repetition 1) the agent's rewrite used a division, which the
embedded target forbids, so the package kept the translated solver.

## Results

Each repetition runs `tolerance_stop --rounds 3 --passes 3`
(`portfolio/portfolio_cvxpygen/results_factor/timings_stop.csv` and
`results_factor/reps/rep<k>/timings_stop.csv`), with OSQP's duality-gap test off. Times are
C milliseconds per solve: the median over 3 interleaved rounds of the mean over 100 draws,
on an Apple M4 Pro, then the median over the three repetitions
(`results_factor/portfolio_cvxpygen_stop_wide.csv`, the figure data).

| n | mean iters: OSQP / FOMTools | OSQP (ms) | FOMTools translated (ms) | FOMTools optimized (ms) | speedup | speedup per run |
|---:|---:|---:|---:|---:|---:|---:|
| 200 | 99.0 / 100.0 | 3.14 | 3.50 | 0.97 | 3.25x | 0.92x to 3.28x |
| 500 | 109.0 / 109.5 | 7.44 | 8.15 | 2.94 | 2.53x | 2.45x to 3.08x |
| 1000 | 144.2 / 158.0 | 54.42 | 63.81 | 8.72 | 6.24x | 3.62x to 7.31x |
| 3000 | 291.2 / 297.5 | 328.66 | 331.07 | 56.84 | 5.78x | 3.15x to 7.23x |

All solvers stop on the same residual test, and all solves reach the tolerance for every
arm, n and repetition.

## Code generation

All FOMTools solvers pass the generated `make verify`. Wall time from the CVXPY problem to
each solver, median and range over the three runs:

| n | translated (s) | translated + optimized (min) | optimizations kept |
|---:|---:|---:|---:|
| 200 | 42 (41 to 56) | 17.9 (7.8 to 24.3) | 2 of 3 |
| 500 | 46 (46 to 53) | 13.1 (12.0 to 27.6) | 3 of 3 |
| 1000 | 75 (70 to 80) | 32.7 (13.1 to 38.7) | 3 of 3 |
| 3000 | 255 (254 to 323) | 41.0 (37.0 to 42.9) | 3 of 3 |
