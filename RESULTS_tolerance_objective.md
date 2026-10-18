# CVXPYgen portfolio sweep to tolerance 1e-4, every solver stopping itself

Question: with the new objective termination (fomtools PR #138,
`TerminationKind.OBJECTIVE`), does CVXPYgen + FOMTools need fewer iterations and
less time than CVXPYgen + OSQP to reach a given tolerance?

Short answer: the iteration counts are now about the same (within 10%), so the
speedup comes from cheaper iterations. Per solve, the AI-optimized FOMTools solver
beats CVXPYgen + OSQP by 1.9x / 1.6x / 2.4x / 3.4x in C at n = 200 / 500 / 1000 /
3000. The large iteration gaps seen earlier came from OSQP's duality-gap stopping
test, not from the method or the scaling (details below).

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
  `out_factor/tolerance_stop.log`):
  `tolerance_stop --rounds 3 --passes 3 --iters-only cvxpy_osqp`. It runs all arms
  in one interleaved run with OSQP's dual-gap test off, and every number in the main
  tables comes from it. Each round is one C run (1 warm-up pass + 3 timed passes
  over the 100 draws) plus one Python run (3 warm-up solves + 1 pass). Each Python
  run is its own process. Reported time is the median of 3 per-round means.
  CVXPY + OSQP runs once per draw for iterations only, so it has no timing. Total
  wall time 29.7 min.
- **7-round run** (`results_factor/timings_stop_7r.csv`, `out_factor/stop_7r_n{N}.json`,
  `out_factor/tolerance_stop_7r.log`): 7 rounds, 20 C passes, all arms timed,
  OSQP with its default test (dual gap on). It is the source of the "OSQP default
  test" column and the FOMTools cross-check below.
- Iterations and residuals come from the timed self-stopping runs themselves.
  There is no search over iteration counts and no NumPy replica. Per solve, the C
  driver logs the iteration count and `max(pri_res, dua_res)` at the stop from one
  extra untimed pass. The Python runs log them from the timed pass. C and Python
  counts agree for every CVXPYgen arm at every n, and all 3 (or 7) rounds stop at
  identical counts. All 100 solves reach "solved" for every arm and n.
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
| CVXPY + OSQP | 99.8 | 100 | 9.86e-05 | | (not timed) | 100.2 / 125 | 9.86e-05 | | 5.54 |
| CVXPYgen + OSQP | 99.0 | 100 | 9.89e-05 | 3.09 | 3.28 | 109.8 / 175 | 9.81e-05 | 3.48 | 3.62 |
| CVXPYgen + FOMTools, translated | 100.0 | 100 | 5.87e-05 | 3.76 | 3.75 | | | | |
| CVXPYgen + FOMTools, optimized | 100.0 | 100 | 5.87e-05 | 1.63 | 1.74 | | | | |

FOMTools optimized vs CVXPYgen + OSQP: **1.90x in C**, 1.89x in Python.
C time per iteration: 31.2 vs 16.3 us.

### n = 500

| arm | mean iters | max iters | max res | C ms | Python ms | default test: mean / max iters | max res | C ms | Python ms |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| CVXPY + OSQP | 106.5 | 125 | 9.98e-05 | | (not timed) | 171.0 / 250 | 7.53e-06 | | 16.41 |
| CVXPYgen + OSQP | 109.0 | 175 | 9.97e-05 | 7.32 | 7.55 | 217.2 / 400 | 4.61e-05 | 14.46 | 14.76 |
| CVXPYgen + FOMTools, translated | 109.5 | 125 | 9.88e-05 | 8.50 | 8.51 | | | | |
| CVXPYgen + FOMTools, optimized | 109.5 | 125 | 9.88e-05 | 4.51 | 4.65 | | | | |

FOMTools optimized vs CVXPYgen + OSQP: **1.62x in C**, 1.62x in Python.
C time per iteration: 67.1 vs 41.2 us.

### n = 1000

| arm | mean iters | max iters | max res | C ms | Python ms | default test: mean / max iters | max res | C ms | Python ms |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| CVXPY + OSQP | 150.8 | 200 | 9.92e-05 | | (not timed) | 299.5 / 375 | 1.91e-06 | | 132.11 |
| CVXPYgen + OSQP | 144.2 | 225 | 9.94e-05 | 53.86 | 62.86 | 383.5 / 575 | 3.14e-06 | 141.86 | 166.02 |
| CVXPYgen + FOMTools, translated | 158.0 | 200 | 9.87e-05 | 61.04 | 64.75 | | | | |
| CVXPYgen + FOMTools, optimized | 158.0 | 200 | 9.87e-05 | 22.09 | 22.39 | | | | |

FOMTools optimized vs CVXPYgen + OSQP: **2.44x in C**, 2.81x in Python.
C time per iteration: 373.4 vs 139.8 us.

### n = 3000

| arm | mean iters | max iters | max res | C ms | Python ms | default test: mean / max iters | max res | C ms | Python ms |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| CVXPY + OSQP | 270.2 | 350 | 9.90e-05 | | (not timed) | 727.0 / 1000 | 6.27e-07 | | 862.63 |
| CVXPYgen + OSQP | 291.2 | 350 | 9.98e-05 | 324.28 | 316.56 | 994.5 / 1650 | 4.06e-07 | 1092.87 | 1076.10 |
| CVXPYgen + FOMTools, translated | 297.5 | 375 | 9.96e-05 | 345.10 | 396.43 | | | | |
| CVXPYgen + FOMTools, optimized | 297.5 | 375 | 9.96e-05 | 94.64 | 80.46 | | | | |

FOMTools optimized vs CVXPYgen + OSQP: **3.43x in C**, 3.93x in Python.
C time per iteration: 1113.4 vs 318.1 us.

### Summary across n (main run, C)

| n | iters: CVXPYgen + OSQP / FOMTools | C ms: CVXPYgen + OSQP / FOMTools opt | speedup C | speedup Python |
|---:|---:|---:|---:|---:|
| 200 | 99.0 / 100.0 | 3.09 / 1.63 | 1.90x | 1.89x |
| 500 | 109.0 / 109.5 | 7.32 / 4.51 | 1.62x | 1.62x |
| 1000 | 144.2 / 158.0 | 53.86 / 22.09 | 2.44x | 2.81x |
| 3000 | 291.2 / 297.5 | 324.28 / 94.64 | 3.43x | 3.93x |

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
   3.4x at n >= 500 (CVXPYgen + OSQP: 217 vs 109 at n = 500, 384 vs 144 at n = 1000,
   995 vs 291 at n = 3000).

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
check, with the dual-gap test still on and on 30 draws, showed 110 / 210 / 377 vs 100 / 171 / 293.
That gap was also the duality-gap test, which interacts with the cost scaling.)

## Where the speedup comes from

With the iteration counts about equal, the speedup is in the time per iteration.
The translated FOMTools solver is slightly slower than embedded OSQP per
iteration in C: OSQP's time per iteration is 0.83x / 0.86x / 0.97x / 0.96x the
translated solver's. The AI optimization then gives 2.3x / 1.9x / 2.8x / 3.6x over
the translated solver in C. Embedded OSQP's per-iteration cost grows faster with n
than FOMTools optimized's: 31 -> 1113 us vs 16 -> 318 us from n = 200 to 3000.

One oddity: at n = 3000 the optimized FOMTools solver is faster through the Python
wrapper (80.5 ms) than in the C driver (94.6 ms). This is reproducible (the 7-round
run gives 80.3 vs 94.5) and only happens at this size. Both builds use -O3 from the
same sources; I did not chase it further. The C number is the conservative one and
is used for the headline speedup.

## Cross-check against the 7-round run

The FOMTools arms are identical in both runs: same iterations and residuals per
solve. Their times agree within 1% at every n and level (largest gap -2.5%, n =
200 translated, Python):

| n | arm | level | main run ms | 7-round ms | diff |
|---:|---|---|---:|---:|---:|
| 200 | translated | c | 3.76 | 3.78 | -0.7% |
| 200 | translated | python | 3.75 | 3.84 | -2.5% |
| 200 | optimized | c | 1.63 | 1.64 | -0.3% |
| 200 | optimized | python | 1.74 | 1.72 | +0.7% |
| 500 | translated | c | 8.50 | 8.52 | -0.3% |
| 500 | translated | python | 8.51 | 8.51 | -0.1% |
| 500 | optimized | c | 4.51 | 4.52 | -0.2% |
| 500 | optimized | python | 4.65 | 4.67 | -0.5% |
| 1000 | translated | c | 61.04 | 60.65 | +0.7% |
| 1000 | translated | python | 64.75 | 64.33 | +0.7% |
| 1000 | optimized | c | 22.09 | 22.05 | +0.2% |
| 1000 | optimized | python | 22.39 | 22.33 | +0.3% |
| 3000 | translated | c | 345.10 | 344.41 | +0.2% |
| 3000 | translated | python | 396.43 | 395.44 | +0.2% |
| 3000 | optimized | c | 94.64 | 94.45 | +0.2% |
| 3000 | optimized | python | 80.46 | 80.25 | +0.3% |

Round spread in the main run (max |round - median| / median, C) is at most 2.4%.

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

## Files and caveats

- Archive of the pre-objective outputs: `portfolio/portfolio_cvxpygen/out_factor_fixed200_pre_objective/`
  (copies of `out_factor/` and `results_factor/` before regeneration).
- New scripts: `tolerance_stop.py` (this run) and `scaling_check.py`. The C driver
  has a new mode, `stop=<tol>,<check>,<max_iter>`. `generate.py` and `model.py`
  now build OBJECTIVE solvers. Because of that, the fixed-iteration `benchmark.py`
  `fom` arms would now stop at 1e-4, and the old search-based `tolerance.py` is
  stale.
- The Python CVXPYgen + OSQP arm reaches rho, adaptive rho and `check_dualgap`
  through `ctypes` on the wrapper's own extension module, which is outside
  CVXPYgen's public API. The C arm writes the same fields directly. Matching C and
  Python counts confirm the two run the same solver.
- No fomtools bugs found. Nothing pushed.
