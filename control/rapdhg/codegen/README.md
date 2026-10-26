# rAPDHG controller: code generation and closed-loop flight

The instance has 40 variables and 120 inequality rows, with a one-second horizon
at a 10 ms sampling period and the 30-degree tilt bound of `../README.md`. The
restart period is K = 64 and the primal weight is w = 1, chosen by flying the
closed loop (`tuning.py`). The step sizes and the restart are declared as
schedules, so the generated solver is a single loop. The restart is a coefficient
that is nonzero on every K-th iteration.

## Running it

From `control/rapdhg/`:

```
uv run python -m codegen.check_against_fomtools                          # the controller is the recorded algorithm
FOMTOOLS_CODEGEN_AGENT=1 uv run python -m codegen.run --force            # the paper: 3 repetitions, all steps
```

`codegen.run` does, for each repetition (`--reps`, default 1 2 3): `codegen.generate
--max-iters 512` (both agent phases; code optimization in rounds), `codegen.extend_tables`
(tables to `study.MAX_ITERS` = 65536, keeping every source the agent wrote),
`codegen.run_implementations` (each implementation's cost per iteration, flown over
the budget grid), and `codegen.run_figures --reference figure8` (the paper figure at
a 40 us budget, with the iterations that budget buys). Each step also runs on its own
with `--rep k`. Repetition k works in `out/reps/rep<k>/` (repetition 1 in `out/`);
the numbers go to the tracked `results/` (`results/reps/rep<k>/`):
`implementations.csv`, `OPTIMIZATIONS.md`, the figure PDFs and `manifest.json`
(fomtools commit, agent times and rounds, cost per iteration of each implementation,
the iterations behind the figure).

## Reference

The figure eight, flown from its position and velocity at time zero:

    x = a sin(w t),   y = (a/2) sin(2 w t),   z = h + c sin(w t / 2)

with `a = 1 m`, `c = 0.4 m`, `h = 1 m`, and `w = 2 pi / 4.0`. At this period the path
asks for about 31 degrees of tilt at its peaks, so the bound is active only there.

## Result (the rerun, three agent runs)

Medians over the three repetitions (`results/` and `results/reps/rep<k>/`,
`manifest.json`), at the figure's 40 us budget per sampling instant:

| implementation | us/iteration | vs NumPy | iterations in 40 us |
|---|---:|---:|---:|
| NumPy | 13.91 | 1.0x | 2 |
| translated C | 2.256 | 6.2x | 17 |
| optimized C | 0.0824 | 169x | 485 |

Code optimization makes each iteration 22 to 29 times faster in the three runs. The
NumPy controller loses the path, the translated solver cuts its corners, and the
optimized solver tracks it. The paper shows the flight of repetition 2, whose optimized
solver runs the median number of iterations (`results/reps/rep2/figures_figure8_flight.pdf`,
copied by `python -m codegen_timings.paper_csv --only flight --write` from the repository
root).

The optimizations differ between runs. In repetition 2 the agent used the lower
triangular Toeplitz structure of the tilt rows (55 two-wide FMAs instead of 440 sparse
products) and reduced the dense 40 x 40 block to 400 FMAs instead of 1600 with sum and
difference transforms. Each report is the run's `OPTIMIZATIONS.md`. The archived run
(before the rerun) ran 3, 13, and 275 iterations at 13.26, 2.87, and 0.144 us per iteration.

## Checks

`python -m codegen.check_against_fomtools`:

| check | result |
|---|---|
| flat recurrence vs Algorithm 1, across restarts | 2.8e-15 |
| extracted IR vs NumPy, 100 iterations | 3.6e-12 |
| converged rAPDHG vs Clarabel, relative objective | 0 |

`make verify` runs at `tol_verify = 1e-4`, an absolute tolerance. Verification
draws parameters from the corners of the flight's parameter box, where the
iterates are much larger than on any real flight. The worst error measured
against the reference oracle is 1.3e-7.
