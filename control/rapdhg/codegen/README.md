# rAPDHG controller: code generation and closed-loop flight

The instance has 40 variables and 120 inequality rows, with a one-second horizon
at a 10 ms sampling period and the 30-degree tilt bound of `../README.md`. The
restart period is K = 64 and the primal weight is w = 1, chosen by flying the
closed loop (`tuning.py`). The step sizes and the restart are declared as
schedules, so the generated solver is a single loop. The restart is a coefficient
that is nonzero on every K-th iteration.

## Running it

```
python -m codegen.check_against_fomtools    # the controller is the recorded algorithm
python -m codegen.generate --max-iters 512  # both C solvers (FOMTOOLS_CODEGEN_AGENT=1)
python -m codegen.run_implementations       # per-iteration cost of each implementation
python -m codegen.run_figures --reference figure8 --iterations 3 13 275 --no-gif   # paper figure
```

## Reference

The figure eight, flown from its position and velocity at time zero:

    x = a sin(w t),   y = (a/2) sin(2 w t),   z = h + c sin(w t / 2)

with `a = 1 m`, `c = 0.4 m`, `h = 1 m`, and `w = 2 pi / 4.0`. At this period the path
asks for about 31 degrees of tilt at its peaks, so the bound is active only there.

## Result

| implementation | us/iteration | vs NumPy |
|---|---:|---:|
| NumPy | 13.26 | 1.0x |
| translated C | 2.87 | 4.6x |
| optimized C | 0.144 | 92x |

With 40 us of computation per sampling instant, the three implementations run 3,
13, and 275 iterations. The NumPy controller loses the path, the translated solver
cuts its corners, and the optimized solver tracks it
(`out/figures_figure8_flight.pdf`).

In the optimization phase the agent dropped the structurally zero blocks and
turned the identity blocks into copies. It applied a two-level Hadamard
factorization to the dense 40 x 40 condensed Hessian, and it collapsed the
Toeplitz coupling block of the tilt rows into two ten-point convolutions per
direction. Its report is `out/codegen/optimized/OPTIMIZATIONS.md`.

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
