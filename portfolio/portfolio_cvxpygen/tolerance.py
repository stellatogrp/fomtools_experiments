"""Average solve time to a tolerance, for each number of assets N.

Superseded by ``tolerance_stop``, in which every solver stops itself. This script
times the solvers at iteration counts found beforehand, and assumes fomtools
solvers generated with a fixed iteration count (``TerminationKind.MAX_ITERS``),
as in ``out_factor_fixed200_pre_objective/``. ``generate`` now builds
``TerminationKind.OBJECTIVE`` solvers, which stop at ``model.TOL`` on their own.
Kept for ``rho_sweep --check`` and the archived results.

Run from ``portfolio/`` in the ``algoverify`` conda env, after ``generate``:

    python -m portfolio_cvxpygen.tolerance [--N 10 20] [--tol 1e-4] [--rounds 7]

One stopping test for every solver: OSQP's, with ``eps_abs = tol`` and
``eps_rel = 0``, i.e. both unscaled residuals of the canonical problem,
``||Ax - clamp(Ax, l, u)||_inf`` and ``||Px + q + A'y||_inf``, at most ``tol``.
OSQP reports these residuals and so does the fomtools bridge.

1. **Iterations.** For each solver and each holdings draw, the first iteration
   whose solution passes the test, found in one run (no search over the count):

   - CVXPY + OSQP: OSQP's own test checked every iteration (``eps_abs = tol``,
     ``eps_rel = 0``, unscaled residuals, polishing off).
   - stock CVXPYgen + OSQP: the same, in C, over the draws in order. OSQP runs
     at a fixed rho (``model.OSQP_RHO``, adaptive rho off). Its Python wrapper
     resets the settings before every solve and cannot set rho, so its Python
     run is timed at the counts found in C.
   - fomtools: ``rho_sweep``'s NumPy replica of the recorded ADMM, which checks
     the residuals every iteration; the generated solver does not yet check
     this test while iterating. A doubling-then-bisection search of the
     generated solver confirms the counts on the first draws, and the timed C
     runs report the largest residual. The translated and optimized solvers run
     the same arithmetic, so they share the counts.

2. **Time.** Every solver then runs each draw for exactly its count, with no
   termination check, through Python and (CVXPYgen solvers) in C, as in
   ``benchmark``: 7 interleaved rounds, the median of the per-round means. The
   C driver also reports the largest residual over the timed solves.

Writes ``out/iters_tol_n{N}.json`` and ``results/timings_tol.csv``.
"""

from __future__ import annotations

import argparse
import csv
import json
import re
import subprocess
import time
from pathlib import Path

import cvxpy as cp
import numpy as np

from portfolio_cvxpygen.benchmark import C_PASSES, WARMUP, build_c, machine
from portfolio_cvxpygen.model import (
    FOM_OPTIONS,
    N_SOLVES,
    OSQP_OFF,
    OSQP_ON,
    OUT,
    RESULTS,
    SIZES,
    build,
    holdings_sequence,
    load_cpg_solve,
)

TOL = 1e-4
CAP = 20000  # largest iteration count searched
ARMS = ("cvxpy_osqp_on", "cpg_osqp", "cpg_fom", "cpg_fom_opt")
C_ARMS = ("cpg_osqp", "cpg_fom", "cpg_fom_opt")
#: Which generated solver (``model.code_dir``) each CVXPYgen arm runs.
GENERATED = {"cpg_osqp": "osqp", "cpg_fom": "fom", "cpg_fom_opt": "fom_opt"}


def _no_check(settings: dict) -> dict:
    """``settings`` with the iteration count left to the caller."""
    return {k: v for k, v in settings.items() if k not in ("max_iter", "check_termination")}


def python_arm(N: int, arm: str):
    """``solve(x, K) -> (pri_res, dua_res)``: one solve of ``arm`` for exactly K iterations."""
    problem, x, _ = build(N)  # one problem object per arm: CVXPY caches its solver
    if arm.startswith("cvxpy"):
        settings = _no_check(OSQP_OFF if arm == "cvxpy_osqp_off" else OSQP_ON)

        def solve(xv, K):
            x.value = xv
            problem.solve(solver=cp.OSQP, max_iter=int(K), check_termination=0, **settings)
            info = problem.solver_stats.extra_stats.info
            return info.prim_res, info.dual_res

        return solve

    # CVXPY's solve methods are registered globally, so each arm needs its own name.
    method = f"CPG_TOL_{arm}_n{N}"
    problem.register_solve(method, load_cpg_solve(N, GENERATED[arm]))
    extra = dict(check_termination=0, warm_start=False) if arm == "cpg_osqp" else {}

    def solve(xv, K):
        x.value = xv
        problem.solve(method=method, max_iter=int(K), **extra)
        stats = problem.solver_stats.extra_stats
        return stats["pri_res"], stats["dua_res"]

    return solve


def iterations_to_tol(solve, xv, tol: float = TOL) -> int | None:
    """Smallest K with both residuals <= tol (doubling, then bisection); None past CAP."""

    def ok(K):
        return max(solve(xv, K)) <= tol

    hi = 1
    while not ok(hi):
        hi *= 2
        if hi > CAP:
            return None
    lo = hi // 2
    while hi - lo > 1:
        mid = (lo + hi) // 2
        lo, hi = (lo, mid) if ok(mid) else (mid, hi)
    return hi


def holdings_file(N: int, X: np.ndarray):
    path = OUT / "bench_build" / f"holdings_n{N}.bin"
    path.parent.mkdir(parents=True, exist_ok=True)
    X.astype(np.float64).tofile(path)
    return path


def c_osqp_iterations(N: int, X: np.ndarray, tol: float) -> list[int]:
    out = subprocess.run(
        [str(build_c(N, "cpg_osqp")), str(holdings_file(N, X)), str(len(X)), str(N), "0",
         f"search={tol:g}"],
        check=True, capture_output=True, text=True,
    ).stdout
    return [int(v) for v in re.search(r"iters=(\S+)", out).group(1).split(",")]


def cvxpy_osqp_iterations(N: int, X: np.ndarray, tol: float) -> list[int]:
    """CVXPY + OSQP: one solve per draw with OSQP's own test checked every
    iteration (``eps_abs = tol``, ``eps_rel = 0``, unscaled residuals), which stops
    at the first iteration where both residuals are at most ``tol``."""
    problem, x, _ = build(N)
    settings = dict(
        _no_check(OSQP_ON), polishing=False, max_iter=CAP, check_termination=1,
        eps_abs=tol, eps_rel=0.0, scaled_termination=False,
    )
    found = []
    for xv in X:
        x.value = xv
        problem.solve(solver=cp.OSQP, **settings)
        if problem.status != cp.OPTIMAL:
            raise RuntimeError(f"N={N} CVXPY + OSQP: {problem.status} within {CAP} iterations")
        found.append(int(problem.solver_stats.num_iters))
    return found


def fom_iterations(N: int, X: np.ndarray, tol: float) -> list[int]:
    """fomtools: the first iteration where both residuals are at most ``tol``, from
    ``rho_sweep``'s NumPy replica of the recorded ADMM (all draws in one batch).
    Checked against a bisection of the generated solver on the first draws."""
    from portfolio_cvxpygen.rho_sweep import CAP as REPLICA_CAP
    from portfolio_cvxpygen.rho_sweep import canonical, iterations

    qp, scaling, maps = canonical(N)
    found = iterations(qp, scaling, maps, X, FOM_OPTIONS["rho"], tol)
    if np.any(found > REPLICA_CAP):
        raise RuntimeError(f"N={N} fomtools: no iteration count up to {REPLICA_CAP} reaches {tol:g}")
    generated = python_arm(N, "cpg_fom_opt")
    for k in range(3):
        assert iterations_to_tol(generated, X[k], tol) == found[k], (N, k)
    return [int(v) for v in found]


def search(N: int, tol: float, reuse: dict | None = None) -> dict[str, list[int]]:
    """Iterations to ``tol`` per arm and draw, each the first passage of the
    stopping test in one run (no bisection over the iteration count). ``reuse``
    holds counts from an earlier search at the same ``tol``; the OSQP arms are
    taken from it when present, the fomtools arms are always searched."""
    X = holdings_sequence(N)
    reuse = reuse or {}
    iters: dict[str, list[int]] = {}
    iters["cvxpy_osqp_on"] = list(reuse.get("cvxpy_osqp_on") or cvxpy_osqp_iterations(N, X, tol))
    iters["cpg_osqp"] = list(reuse.get("cpg_osqp") or c_osqp_iterations(N, X, tol))
    # The translated and optimized solvers run the same arithmetic.
    iters["cpg_fom_opt"] = fom_iterations(N, X, tol)
    iters["cpg_fom"] = list(iters["cpg_fom_opt"])
    return iters


def time_python(solve, X, K) -> float:
    for xv, k in zip(X[:WARMUP], K[:WARMUP]):
        solve(xv, k)
    times = []
    for xv, k in zip(X, K):
        start = time.perf_counter()
        solve(xv, k)
        times.append(time.perf_counter() - start)
    return 1e6 * float(np.mean(times))


def time_c(N: int, arm: str, holdings, K) -> dict:
    iters_path = OUT / "bench_build" / f"iters_tol_n{N}_{arm}.bin"
    np.asarray(K, dtype=np.int32).tofile(iters_path)
    out = subprocess.run(
        [str(build_c(N, arm)), str(holdings), str(N_SOLVES), str(N), str(C_PASSES), f"@{iters_path}"],
        check=True, capture_output=True, text=True,
    ).stdout
    fields = dict(re.findall(r"(\w+)=(\S+)", out))
    return {k: float(fields[k]) for k in ("us_per_solve", "mean_iters", "max_pri_res", "max_dua_res")}


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--N", type=int, nargs="+", default=list(SIZES))
    parser.add_argument("--tol", type=float, default=TOL)
    parser.add_argument("--rounds", type=int, default=7)
    parser.add_argument("--search-only", action="store_true",
                        help="find and save the iteration counts, skip the timing rounds")
    parser.add_argument("--reuse-from", type=Path, default=None,
                        help="folder with iters_tol_n{N}.json from an earlier search; "
                        "reuses its OSQP iteration counts")
    args = parser.parse_args()

    RESULTS.mkdir(parents=True, exist_ok=True)
    rows = []
    for N in args.N:
        iters_path = OUT / f"iters_tol_n{N}.json"
        reuse = None
        if args.reuse_from and (args.reuse_from / f"iters_tol_n{N}.json").exists():
            reuse = json.loads((args.reuse_from / f"iters_tol_n{N}.json").read_text())
            if reuse.get("tol") != args.tol:
                reuse = None
        iters = search(N, args.tol, reuse)
        iters_path.write_text(json.dumps({"tol": args.tol, **iters}))
        print(f"N={N} iterations to {args.tol:g}: "
              + "  ".join(f"{a} {np.mean(iters[a]):.0f}" for a in ARMS), flush=True)
        if args.search_only:
            continue

        X = holdings_sequence(N)
        holdings = holdings_file(N, X)
        solvers = {arm: python_arm(N, arm) for arm in ARMS}
        samples: dict[tuple[str, str], list] = {}
        for r in range(args.rounds):
            for arm in ARMS:
                samples.setdefault((arm, "python"), []).append(
                    dict(us_per_solve=time_python(solvers[arm], X, iters[arm]))
                )
                if arm in C_ARMS:
                    samples.setdefault((arm, "c"), []).append(time_c(N, arm, holdings, iters[arm]))
            print(f"N={N} round {r + 1}/{args.rounds}", flush=True)

        for (arm, level), values in samples.items():
            us = np.array([v["us_per_solve"] for v in values])
            med = float(np.median(us))
            row = dict(
                N=N, arm=arm, level=level, tol=args.tol, us_per_solve=med,
                mean_iters=float(np.mean(iters[arm])),
                round_spread=float(np.max(np.abs(us - med)) / med),
                max_pri_res=values[0].get("max_pri_res", ""),
                max_dua_res=values[0].get("max_dua_res", ""),
            )
            rows.append(row)
            res = f"  max res {row['max_pri_res']:.1e}/{row['max_dua_res']:.1e}" if level == "c" else ""
            print(f"  {arm:15s} {level:6s} {med:12.2f} us  iters {row['mean_iters']:.0f}{res}")

    if args.search_only:
        return
    path = RESULTS / "timings_tol.csv"
    existing = []
    if path.exists():  # keep rows for N not re-run
        with path.open() as f:
            existing = [r for r in csv.DictReader(f) if int(r["N"]) not in args.N]
    with path.open("w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=list(rows[0]))
        writer.writeheader()
        writer.writerows(existing + rows)
    (RESULTS / "machine_tol.json").write_text(json.dumps(machine(), indent=2))
    print(f"wrote {path}")


if __name__ == "__main__":
    main()
