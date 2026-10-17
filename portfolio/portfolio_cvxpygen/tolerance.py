"""Average solve time to a tolerance, for each number of assets N.

Run from ``portfolio/`` in the ``algoverify`` conda env, after ``generate``:

    python -m portfolio_cvxpygen.tolerance [--N 10 20] [--tol 1e-4] [--rounds 7]

One stopping test for every solver: OSQP's, with ``eps_abs = tol`` and
``eps_rel = 0``, i.e. both unscaled residuals of the canonical problem,
``||Ax - clamp(Ax, l, u)||_inf`` and ``||Px + q + A'y||_inf``, at most ``tol``.
OSQP reports these residuals and so does the fomtools bridge.

1. **Iterations.** For each solver and each holdings draw, the smallest
   iteration count whose solution passes the test:

   - CVXPY + OSQP (both settings) and fomtools: a doubling then bisection search
     over ``max_iter`` through Python, reading the residuals each solve reports.
     The translated and optimized fomtools solvers run the same arithmetic, so
     they share the counts found with the optimized one (checked on a few draws).
   - stock CVXPYgen + OSQP: in C, with OSQP's own test checked every iteration,
     over the draws in order. Embedded OSQP adapts rho and keeps it between
     solves, and the timed C run repeats the same order. Its Python wrapper
     resets the settings before every solve, which leaves rho out of step with
     the factored matrix and stalls convergence; that is a wrapper quirk, so its
     Python run is timed at the counts found in C.

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


def search(N: int, tol: float, reuse: dict | None = None) -> dict[str, list[int]]:
    """Iterations to ``tol`` per arm and draw. ``reuse`` holds counts from an earlier
    search at the same ``tol``; the OSQP arms are taken from it when present (they
    do not depend on the fomtools solver), the fomtools arms are always searched."""
    X = holdings_sequence(N)
    iters: dict[str, list[int]] = {}
    for arm in ("cvxpy_osqp_on", "cpg_fom_opt"):
        if reuse and arm in reuse and not arm.startswith("cpg_fom"):
            iters[arm] = list(reuse[arm])
            continue
        solve = python_arm(N, arm)
        found = [iterations_to_tol(solve, xv, tol) for xv in X]
        if any(k is None for k in found):
            raise RuntimeError(f"N={N} {arm}: no iteration count up to {CAP} reaches {tol:g}")
        iters[arm] = found
    # The translated solver runs the same arithmetic as the optimized one.
    translated = python_arm(N, "cpg_fom")
    for k in range(3):
        assert iterations_to_tol(translated, X[k], tol) == iters["cpg_fom_opt"][k], (N, k)
    iters["cpg_fom"] = list(iters["cpg_fom_opt"])
    iters["cpg_osqp"] = (
        list(reuse["cpg_osqp"]) if reuse and "cpg_osqp" in reuse else c_osqp_iterations(N, X, tol)
    )
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
