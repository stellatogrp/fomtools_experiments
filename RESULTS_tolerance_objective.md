# CVXPYgen portfolio sweep to tolerance 1e-4, every solver stopping itself

Question: with the new objective termination (fomtools PR #138,
`TerminationKind.OBJECTIVE`), does CVXPYgen + FOMTools need fewer iterations and
less time than CVXPYgen + OSQP to reach a given tolerance?

Short answer: the iteration counts are about the same (within 10%), so the speedup
comes from cheaper iterations. Per solve, the AI-optimized FOMTools solver beats
CVXPYgen + OSQP by 1.9x / 1.6x / 2.5x / 3.4x in C at n = 200 / 500 / 1000 / 3000.
It beats CVXPY + OSQP through Python by 3.2x / 2.6x / 3.5x / 4.4x. The large
iteration gaps seen earlier came from OSQP's duality-gap stopping test, not from
the method or the scaling (details below).

Figure (`portfolio/portfolio_cvxpygen/results_factor/`):
`portfolio_cvxpygen_stop_paper.{pdf,png}` (LaTeX serif, paper fonts) and
`portfolio_cvxpygen_stop_slides.{pdf,png}` (Helvetica Neue, large fonts). Panel (a)
is time per solve, C solid and Python dashed. Panel (b) is iterations to 1e-4, the
mean with the largest as a whisker; the translated and optimized FOMTools solvers
have identical iterations, so they share one line. The data for pgfplots is in
`portfolio_cvxpygen_stop_wide.csv`. Reproduce with `python -m
portfolio_cvxpygen.plot --stop`; the full recipe is in
`portfolio/portfolio_cvxpygen/README.md`.

## Setup

- Factor model, draws and back-test as `benchmark.py`: 100 draws of `[x; mu]` per
  n, n = 200 / 500 / 1000 / 3000, cold start every solve.
- The same ADMM for every arm: rho = 1 fixed, adaptive rho off, sigma = 1e-6,
  alpha = 1.6, rho boosted 1e3 on equality rows, no polishing. CVXPYgen generates
  OSQP at rho 0.1 and has no setting for it, so `osqp_update_rho(1.0)` is called
  once before the first solve and adaptive rho is turned off. The C driver does
  this directly; the Python wrapper does it through `ctypes` on its own extension
  module, because its `cpg_solve` resets the settings before every solve.
- Every solver stops itself on the unscaled residuals of the canonical QP:
  `eps_abs = 1e-4`, `eps_rel = 0`, `scaled_termination = False`, checked every
  25 iterations, at most 4000. All iteration counts are therefore multiples of 25.
- **Main comparison: residuals only, for both.** OSQP 1.x also stops only once
  the duality gap is below `eps_abs` (`check_dualgap`, on by default). FOMTools'
  objective is the residuals alone. So `check_dualgap = False` for every OSQP arm
  (CVXPY: solver option; CVXPYgen: the C driver writes
  `solver.settings->check_dualgap = 0`, the Python wrapper writes it via ctypes
  after every settings reset). The runs with OSQP's default test (residuals +
  duality gap) are kept as a second column.
- FOMTools solvers are regenerated with `TerminationKind.OBJECTIVE`, `tol = 1e-4`,
  `check_termination = 25`, `max_iters = 4000`, `rho = 1`, `auto_equilibrate`.
  There are two of them: translated, and AI-optimized with the same agent settings
  as before (`generate.py`, 1200 s budget per phase).

### Protocol

- **Main run** (`results_factor/timings_stop.csv`, `out_factor/stop_n{N}.json`,
  `out_factor/tolerance_stop.log`): `tolerance_stop --rounds 3 --passes 3`. It
  runs all arms in one interleaved run with OSQP's dual-gap test off, CVXPY +
  OSQP included. Every number in the main tables and the figure comes from it.
  Each round is one C run (1 warm-up pass + 3 timed passes over the 100 draws)
  plus one Python run (3 warm-up solves + 1 pass). Each Python run is its own
  process. Reported time is the median of the 3 per-round means. Total wall time
  31 min.
- **7-round run** (`results_factor/timings_stop_7r.csv`, `out_factor/stop_7r_n{N}.json`,
  `out_factor/tolerance_stop_7r.log`): 7 rounds, 20 C passes, all arms timed,
  OSQP with its default test (dual gap on). It is the source of the "OSQP default
  test" column and the FOMTools cross-check below.
- Iterations and residuals come from the timed self-stopping runs themselves.
  There is no search over iteration counts and no NumPy replica. Per solve, the C
  driver logs the iteration count and `max(pri_res, dua_res)` at the stop from one
  extra untimed pass. The Python runs log them from the timed pass. C and Python
  counts agree for every CVXPYgen arm at every n, and all rounds stop at identical
  counts. All 100 solves reach "solved" for every arm and n.
- Machine: stellato-mbp-office (MacBook Pro M4 Pro), MacOSX26.5 SDK,
  `results_factor/machine_stop.json`.

## Results

C ms and Python ms are per solve. "max res" is the largest `max(pri_res, dua_res)`
at the stop over the 100 solves (and over both levels for CVXPYgen arms). The
right block is OSQP with its default test (residuals + duality gap), from the
7-round run.

### n = 200

| arm | mean iters | max iters | max res | C ms | Python ms | default test: mean / max iters | max res | C ms | Python ms |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| CVXPY + OSQP | 99.8 | 100 | 9.86e-05 | | 5.62 | 100.2 / 125 | 9.86e-05 | | 5.54 |
| CVXPYgen + OSQP | 99.0 | 100 | 9.89e-05 | 3.13 | 3.32 | 109.8 / 175 | 9.81e-05 | 3.48 | 3.62 |
| CVXPYgen + FOMTools, translated | 100.0 | 100 | 5.87e-05 | 3.77 | 3.83 | | | | |
| CVXPYgen + FOMTools, optimized | 100.0 | 100 | 5.87e-05 | 1.63 | 1.73 | | | | |

### n = 500

| arm | mean iters | max iters | max res | C ms | Python ms | default test: mean / max iters | max res | C ms | Python ms |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| CVXPY + OSQP | 106.5 | 125 | 9.98e-05 | | 12.06 | 171.0 / 250 | 7.53e-06 | | 16.41 |
| CVXPYgen + OSQP | 109.0 | 175 | 9.97e-05 | 7.38 | 7.60 | 217.2 / 400 | 4.61e-05 | 14.46 | 14.76 |
| CVXPYgen + FOMTools, translated | 109.5 | 125 | 9.88e-05 | 8.47 | 8.55 | | | | |
| CVXPYgen + FOMTools, optimized | 109.5 | 125 | 9.88e-05 | 4.55 | 4.72 | | | | |

### n = 1000

| arm | mean iters | max iters | max res | C ms | Python ms | default test: mean / max iters | max res | C ms | Python ms |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| CVXPY + OSQP | 150.8 | 200 | 9.92e-05 | | 77.76 | 299.5 / 375 | 1.91e-06 | | 132.11 |
| CVXPYgen + OSQP | 144.2 | 225 | 9.94e-05 | 54.96 | 64.00 | 383.5 / 575 | 3.14e-06 | 141.86 | 166.02 |
| CVXPYgen + FOMTools, translated | 158.0 | 200 | 9.87e-05 | 61.54 | 66.44 | | | | |
| CVXPYgen + FOMTools, optimized | 158.0 | 200 | 9.87e-05 | 22.09 | 22.43 | | | | |

### n = 3000

| arm | mean iters | max iters | max res | C ms | Python ms | default test: mean / max iters | max res | C ms | Python ms |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| CVXPY + OSQP | 270.2 | 350 | 9.90e-05 | | 355.81 | 727.0 / 1000 | 6.27e-07 | | 862.63 |
| CVXPYgen + OSQP | 291.2 | 350 | 9.98e-05 | 318.72 | 314.29 | 994.5 / 1650 | 4.06e-07 | 1092.87 | 1076.10 |
| CVXPYgen + FOMTools, translated | 297.5 | 375 | 9.96e-05 | 341.84 | 392.36 | | | | |
| CVXPYgen + FOMTools, optimized | 297.5 | 375 | 9.96e-05 | 94.13 | 80.24 | | | | |

### Summary across n (main run)

| n | mean iters: CVXPYgen + OSQP / FOMTools | C ms: CVXPYgen + OSQP / FOMTools opt | speedup C | speedup Python | speedup vs CVXPY + OSQP (Python) |
|---:|---:|---:|---:|---:|---:|
| 200 | 99.0 / 100.0 | 3.13 / 1.63 | 1.92x | 1.92x | 3.25x |
| 500 | 109.0 / 109.5 | 7.38 / 4.55 | 1.62x | 1.61x | 2.56x |
| 1000 | 144.2 / 158.0 | 54.96 / 22.09 | 2.49x | 2.85x | 3.47x |
| 3000 | 291.2 / 297.5 | 318.72 / 94.13 | 3.39x | 3.92x | 4.43x |

C time per iteration, CVXPYgen + OSQP vs FOMTools optimized: 31.7 vs 16.3 us
(n = 200), 67.7 vs 41.6 (500), 381.0 vs 139.8 (1000), 1094.4 vs 316.4 (3000).

## Did each solver stop on the same test?

Yes, with the dual-gap test off. At every n the largest residual at the stop is
just below 1e-4 for every arm (9.86e-05 to 9.98e-05). The one exception is FOMTools
at n = 200: all 100 solves stop at iteration 100 (5.87e-05), so the residual at
iteration 75 must be above 1e-4. All values are at most 1e-4, so the FOMTools stop
matches OSQP's residual test.

How the mismatch was found: in the 7-round run, OSQP's largest residual at the
stop was far below 1e-4 at n >= 500 (CVXPY 7.5e-6 / 1.9e-6 / 6.3e-7, CVXPYgen
4.6e-5 / 3.1e-6 / 4.1e-7). I traced OSQP's residuals every 25 iterations on two
n = 1000 draws (CVXPY's OSQP data, osqp Python). Its own reported unscaled
`prim_res` and `dual_res` were both below 1e-4 at iteration 150 (draw 0) and 125
(draw 1), but OSQP stopped at 300 and 250. The cause is `check_termination` in
OSQP 1.x (`auxil.c`): with `check_dualgap` on (the default in double builds), it
also requires `|duality gap| < eps_abs`, and that binds at n >= 500. The
residual definitions themselves match. OSQP's `prim_res` is `||Ax - z||` with the
`z` iterate, slightly above the bridge's `||Ax - clamp(Ax, l, u)||`, so OSQP is if
anything a bit stricter. Cost scaling and the boosted equality rho are undone in
the unscaled residuals. With `check_dualgap` off, OSQP's counts are exactly the
first passage of the residual test at multiples of 25, and these are the main
columns. The embedded CVXPYgen OSQP honors the flag at run time: the largest
residual moves to about 1e-4 and C and Python counts agree.

## The iteration gap

With the same test, the iteration counts are within 10% of each other: FOMTools
takes 1.01x / 1.00x / 1.10x / 1.02x the CVXPYgen + OSQP mean. The earlier gaps had
two causes:

1. **Adaptive rho**, before rho was fixed. The earlier "FOMTools 97 vs CVXPYgen
   OSQP 204 at n = 500" was CVXPYgen's C solver carrying its adapted rho from one
   solve to the next (see `model.py`).
2. **OSQP's duality-gap test**, at fixed rho. It multiplies OSQP's counts by 2x to
   3.4x at n >= 500 (CVXPYgen + OSQP: 217 vs 109 at n = 500, 384 vs 144 at n =
   1000, 995 vs 291 at n = 3000).

The Ruiz-scaling hypothesis was checked with `scaling_check.py`: OSQP Python on
the canonical QP with the same settings and the dual-gap test off.

| n | per-draw scaling (as CVXPY) | nominal-parameter scaling (as CVXPYgen) | no scaling |
|---:|---:|---:|---:|
| 200 | 99.8 (max 100) | 99.0 (max 100) | 100.0 (max 100) |
| 500 | 106.5 (max 125) | 109.0 (max 175) | 105.8 (max 150) |
| 1000 | 150.8 (max 200) | 144.2 (max 225) | 158.2 (max 175) |

The nominal-scaling column reproduces the CVXPYgen + OSQP counts exactly. The
per-draw column reproduces CVXPY + OSQP exactly. The differences are under 10% and
go both ways, so the fixed Ruiz scaling is not the cause. (A first version of this
check, with the dual-gap test still on and on 30 draws, showed 110 / 210 / 377 vs
100 / 171 / 293. That gap was also the duality-gap test, which interacts with the
cost scaling.)

## Where the speedup comes from

With the iteration counts about equal, the speedup is in the time per iteration.
The translated FOMTools solver is slightly slower than embedded OSQP per
iteration in C: OSQP's time per iteration is 0.84x / 0.87x / 0.98x / 0.95x the
translated solver's. The AI optimization then gives 2.3x / 1.9x / 2.8x / 3.6x over
the translated solver in C. Embedded OSQP's per-iteration cost grows faster with n
than FOMTools optimized's: 32 -> 1094 us vs 16 -> 316 us from n = 200 to 3000.

## C vs Python at n = 3000: a code-layout effect

At n = 3000 the optimized FOMTools solver runs faster through the Python wrapper
(80.2 ms) than in the C driver (94.1 ms). The translated solver goes the other way
(392.4 vs 341.8 ms). At n <= 1000, C and Python agree within a few percent
(Python a little slower, by the wrapper overhead). Both n = 3000 gaps reproduce
exactly in the 7-round run. What I checked:

- **Not wrapper overhead.** Inside the Python process, the solver's own timer
  (`CPG_Info.time`) reads 80.2 ms (optimized) and 394.2 ms (translated), within
  0.5 ms of the wall time per `problem.solve`. The difference is in the compiled
  solver code itself.
- **Same flags.** Both builds compile the same sources at `-O3 -DNDEBUG`. The SDK
  makes no difference (default and 26.5 give the same times). `-fPIC` and linking
  through a static library, as the wrapper does, make no difference either.
- **Link order moves the optimized solver by 19%.** Building the C driver by hand
  with the same sources and flags, only the order of the object files changes the
  time per solve. CMake's glob order (`cpg_fom_bridge, fom_data, fom_linsolve,
  fom_ops, fom_solver`) gives 94.0 ms. Listing `fom_solver.c` first gives 79.0 ms.
  For the translated solver both orders give the same time (339 to 343 ms), yet
  its wrapper build is 15% slower.

So at n = 3000 the timings depend on code layout (function and loop alignment)
by up to about 15 to 20%, in either direction depending on the solver and the
build. Apple M-series cores are known to be sensitive to this. It is not a
measurement error: each number is a stable, reproducible property of one binary.
For the paper I'd report the C driver numbers as measured, which is conservative
for the headline (3.4x in C vs 3.9x in Python). The sensitivity at n = 3000 is
worth one sentence. The build experiments are ad hoc and not in the repository.

## Cross-check against the 7-round run

The FOMTools arms are identical in both runs: same iterations and residuals per
solve. Their times agree within 1% at every n and level except n = 1000
translated (+1.5% C, +3.3% Python):

| n | arm | level | main run ms | 7-round ms | diff |
|---:|---|---|---:|---:|---:|
| 200 | translated | c | 3.77 | 3.78 | -0.4% |
| 200 | translated | python | 3.83 | 3.84 | -0.5% |
| 200 | optimized | c | 1.63 | 1.64 | -0.2% |
| 200 | optimized | python | 1.73 | 1.72 | +0.6% |
| 500 | translated | c | 8.47 | 8.52 | -0.6% |
| 500 | translated | python | 8.55 | 8.51 | +0.4% |
| 500 | optimized | c | 4.55 | 4.52 | +0.7% |
| 500 | optimized | python | 4.72 | 4.67 | +0.9% |
| 1000 | translated | c | 61.54 | 60.65 | +1.5% |
| 1000 | translated | python | 66.44 | 64.33 | +3.3% |
| 1000 | optimized | c | 22.09 | 22.05 | +0.2% |
| 1000 | optimized | python | 22.43 | 22.33 | +0.4% |
| 3000 | translated | c | 341.84 | 344.41 | -0.7% |
| 3000 | translated | python | 392.36 | 395.44 | -0.8% |
| 3000 | optimized | c | 94.13 | 94.45 | -0.3% |
| 3000 | optimized | python | 80.24 | 80.25 | -0.0% |

Round spread in the main run (max |round - median| / median, C) is at most 2.6%.
An earlier light run with CVXPY + OSQP untimed (`timings_stop_prev_untimed_cvxpy.csv`)
also agrees with the main run within 2.6% on every timed arm.

## Code generation

All eight FOMTools solvers pass the generated `make verify` (`VERIFY OK`, re-run
after generation), and all four optimizations were kept.

| n | translate (s) | translate + optimize (s) | agent's own C benchmark, before -> after (us) |
|---:|---:|---:|---|
| 200 | 49 | 498 | 3544 -> 1568 |
| 500 | 70 | 384 | 8119 -> 4251 |
| 1000 | 73 | 412 | 60461 -> 21900 |
| 3000 | 239 | 911 | 362066 -> 78938 |

The optimization phase alone takes about 450 / 314 / 339 / 672 s (one agent turn
each, not truncated). The agent's benchmark uses its own protocol, so its numbers
differ from the table above.

## For the paper text

- State that OSQP runs with `check_dualgap = False`, so both solvers stop on the
  same residual test. With OSQP's default test it needs 2x to 3.4x more iterations
  at n >= 500 (right-hand columns above). A footnote with those numbers shows the
  comparison doesn't handicap OSQP.
- State that both run the same ADMM: rho = 1 fixed, adaptive rho off. For the
  Python CVXPYgen + OSQP arm these settings go in through `ctypes`, because
  CVXPYgen doesn't expose them. Matching C and Python iteration counts confirm the
  two levels run the same solver.
- One sentence on code-layout sensitivity at n = 3000 (previous section).

## Files and caveats

- Archive of the pre-objective outputs: `portfolio/portfolio_cvxpygen/out_factor_fixed200_pre_objective/`
  (copies of `out_factor/` and `results_factor/` before regeneration). The
  fixed-200 `benchmark.py` and the search-based `tolerance.py` need those solvers;
  see the README.
- Scripts: `tolerance_stop.py` (this run), `scaling_check.py`, `plot.py --stop`.
  The C driver has a new mode, `stop=<tol>,<check>,<max_iter>`. `generate.py` and
  `model.py` now build OBJECTIVE solvers.
- No fomtools bugs found. Nothing pushed.
