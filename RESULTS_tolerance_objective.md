# CVXPYgen portfolio sweep to tolerance 1e-4

These are the results behind the paper's portfolio figure. CVXPYgen with the FOMTools ADMM
solver is compared with CVXPYgen with OSQP, every solver stopping itself at the same
tolerance. The setup, the arms, the stopping test, and the shared ADMM settings are in
[`portfolio/portfolio_cvxpygen/README.md`](portfolio/portfolio_cvxpygen/README.md).

The iteration counts agree within 10%, so the speedup comes from cheaper iterations. In C,
the optimized FOMTools solver is 1.9x, 1.6x, 2.5x, and 3.4x faster than CVXPYgen + OSQP at
n = 200, 500, 1000, and 3000. The translated solver is 7 to 20% slower.

## Results

The main run is `tolerance_stop --rounds 3 --passes 3`
(`portfolio/portfolio_cvxpygen/results_factor/timings_stop.csv`), with OSQP's duality-gap
test off. Times are C milliseconds per solve: the median over 3 interleaved rounds of the
mean over 100 draws, on an Apple M4 Pro. The figure data is
`portfolio/portfolio_cvxpygen/results_factor/portfolio_cvxpygen_stop_wide.csv`.

| n | mean iters: OSQP / FOMTools | OSQP (ms) | FOMTools translated (ms) | FOMTools optimized (ms) | speedup |
|---:|---:|---:|---:|---:|---:|
| 200 | 99.0 / 100.0 | 3.13 | 3.77 | 1.63 | 1.92x |
| 500 | 109.0 / 109.5 | 7.38 | 8.47 | 4.55 | 1.62x |
| 1000 | 144.2 / 158.0 | 54.96 | 61.54 | 22.09 | 2.49x |
| 3000 | 291.2 / 297.5 | 318.72 | 341.84 | 94.13 | 3.39x |

All solvers stop on the same residual test. At every n, the largest residual at the stop
is at most 1e-4 for every arm, and the C and Python iteration counts agree for every
CVXPYgen arm. All 100 solves reach "solved" for every arm and n.

## Code generation

All eight FOMTools solvers pass the generated `make verify`, and all four optimizations
were kept. Wall time from the CVXPY problem to each solver, in seconds:

| n | translated | translated + optimized |
|---:|---:|---:|
| 200 | 49 | 498 |
| 500 | 70 | 384 |
| 1000 | 73 | 412 |
| 3000 | 239 | 911 |
