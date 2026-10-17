"""Time every arm on the back-test sequence, for each number of assets N.

Run from ``portfolio/`` in the ``algoverify`` conda env, after ``generate``:

    python -m portfolio_cvxpygen.benchmark [--N 10 20] [--rounds 7]

Arms (``model`` holds the settings):

``cvxpy_osqp_off`` / ``cvxpy_osqp_on``
    ``problem.solve(solver=OSQP)``: canonicalization in Python every call.
``cpg_osqp``
    stock CVXPYgen + OSQP, defaults, a fixed number of iterations, cold start.
``cpg_fom`` / ``cpg_fom_opt``
    CVXPYgen + the fomtools ADMM solver on the equilibrated QP, translated only /
    after the code optimization phase.

Two levels: ``python`` is the wall time of one ``problem.solve`` call (the
quantity in the CVXPYgen paper's Fig. 4); ``c`` is the generated code alone,
timed by ``bench_c/bench_cpg.c`` (CVXPYgen arms only). Each round runs every
arm once over the same ``N_SOLVES`` drifted holdings in order, after 3 warm-up
solves; rounds are interleaved and the median of the per-round means is kept.
Writes ``results/timings.csv`` and ``results/machine.json``.
"""

from __future__ import annotations

import argparse
import csv
import json
import platform
import re
import subprocess
import time
from pathlib import Path

import cvxpy as cp
import numpy as np

from portfolio_cvxpygen.model import (
    CPG_OSQP,
    ITERATIONS,
    N_SOLVES,
    OSQP_OFF,
    OSQP_ON,
    OUT,
    RESULTS,
    SIZES,
    build,
    code_dir,
    holdings_sequence,
    load_cpg_solve,
    prefix,
)

BENCH_SRC = Path(__file__).parent / "bench_c"
WARMUP = 3
C_PASSES = 20

PY_ARMS = {
    "cvxpy_osqp_off": ("osqp", None, OSQP_OFF),
    "cvxpy_osqp_on": ("osqp", None, OSQP_ON),
    "cpg_osqp": ("osqp", "osqp", CPG_OSQP),
    "cpg_fom": ("fom", "fom", {}),
    "cpg_fom_opt": ("fom_opt", "fom_opt", {}),
}
C_ARMS = {
    "cpg_osqp": ("osqp", "OFF"),
    "cpg_fom": ("fom", "FOM"),
    "cpg_fom_opt": ("fom_opt", "FOM"),
}


def python_arm(N: int, arm: str):
    """``(solve, z)``: ``solve(x)`` solves one holdings vector with ``arm`` and returns its
    iteration count; ``z`` is the CVXPY variable holding the solution."""
    _, generated, settings = PY_ARMS[arm]
    problem, x, z = build(N)  # one problem object per arm: CVXPY caches its solver
    if generated is None:

        def solve(xv):
            x.value = xv
            problem.solve(solver=cp.OSQP, **settings)
            return problem.solver_stats.num_iters

    else:
        # CVXPY's solve methods are registered globally, not per problem, so
        # each arm needs its own name or every arm would call the last one.
        method = f"CPG_{arm}_n{N}"
        problem.register_solve(method, load_cpg_solve(N, generated))

        def solve(xv):
            x.value = xv
            problem.solve(method=method, **settings)
            return problem.solver_stats.num_iters

    return solve, z


def time_python(solve, X: np.ndarray) -> tuple[float, float]:
    for xv in X[:WARMUP]:
        solve(xv)
    times, iters = [], []
    for xv in X:
        start = time.perf_counter()
        iters.append(solve(xv))
        times.append(time.perf_counter() - start)
    return 1e6 * float(np.mean(times)), float(np.mean(iters))


def build_c(N: int, arm: str) -> Path:
    generated, mode = C_ARMS[arm]
    build_dir = OUT / "bench_build" / f"n{N}_{arm}"
    exe = build_dir / "bench_cpg"
    gen_dir = OUT / code_dir(N, generated)
    newest = max(f.stat().st_mtime for f in gen_dir.rglob("*") if f.is_file())
    # Rebuild when the generated solver changed since the last build: a stale
    # binary would time a different solver than the one being searched.
    if not exe.exists() or exe.stat().st_mtime < newest:
        subprocess.run(
            [
                "cmake",
                "-S", str(BENCH_SRC),
                "-B", str(build_dir),
                "-DCMAKE_BUILD_TYPE=Release",
                f"-DGEN_DIR={OUT / code_dir(N, generated)}",
                f"-DPREFIX={prefix(N, generated)}",
                f"-DMODE={mode}",
            ],
            check=True,
            capture_output=True,
        )
        subprocess.run(["cmake", "--build", str(build_dir), "--target", "bench_cpg", "-j"],
                       check=True, capture_output=True)
    return exe


def time_c(exe: Path, holdings: Path, N: int) -> tuple[float, float]:
    out = subprocess.run(
        [str(exe), str(holdings), str(N_SOLVES), str(N), str(C_PASSES), str(ITERATIONS)],
        check=True, capture_output=True, text=True,
    ).stdout
    us = float(re.search(r"us_per_solve=(\S+)", out).group(1))
    iters = float(re.search(r"mean_iters=(\S+)", out).group(1))
    return us, iters


def machine() -> dict:
    def sh(*cmd):
        try:
            return subprocess.run(cmd, capture_output=True, text=True).stdout.strip()
        except OSError:
            return ""

    import cvxpygen
    import osqp

    return dict(
        cpu=sh("sysctl", "-n", "machdep.cpu.brand_string") or platform.processor(),
        os=platform.platform(),
        compiler=sh("cc", "--version").splitlines()[0] if sh("cc", "--version") else "",
        python=platform.python_version(),
        numpy=np.__version__,
        cvxpy=cp.__version__,
        cvxpygen=getattr(cvxpygen, "__version__", "unknown"),
        osqp=osqp.__version__,
        date=time.strftime("%Y-%m-%d %H:%M:%S"),
    )


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--N", type=int, nargs="+", default=list(SIZES))
    parser.add_argument("--rounds", type=int, default=7)
    parser.add_argument("--arms", nargs="+", choices=list(PY_ARMS), default=list(PY_ARMS))
    args = parser.parse_args()

    RESULTS.mkdir(parents=True, exist_ok=True)
    rows = []
    for N in args.N:
        X = holdings_sequence(N)
        holdings = OUT / "bench_build" / f"holdings_n{N}.bin"
        holdings.parent.mkdir(parents=True, exist_ok=True)
        X.astype(np.float64).tofile(holdings)

        solvers = {arm: python_arm(N, arm)[0] for arm in args.arms}
        exes = {arm: build_c(N, arm) for arm in args.arms if arm in C_ARMS}
        samples: dict[tuple[str, str], list[tuple[float, float]]] = {}
        for r in range(args.rounds):
            for arm, solve in solvers.items():
                samples.setdefault((arm, "python"), []).append(time_python(solve, X))
                if arm in exes:
                    samples.setdefault((arm, "c"), []).append(time_c(exes[arm], holdings, N))
            print(f"N={N} round {r + 1}/{args.rounds}", flush=True)

        for (arm, level), values in samples.items():
            us = np.array([v[0] for v in values])
            med = float(np.median(us))
            rows.append(
                dict(
                    N=N,
                    arm=arm,
                    level=level,
                    us_per_solve=med,
                    mean_iters=float(np.median([v[1] for v in values])),
                    round_spread=float(np.max(np.abs(us - med)) / med),
                )
            )
            print(f"  {arm:15s} {level:6s} {med:12.2f} us  iters {rows[-1]['mean_iters']:.1f}")

    path = RESULTS / "timings.csv"
    existing = []
    if path.exists():  # keep rows for N not re-run
        with path.open() as f:
            existing = [r for r in csv.DictReader(f) if int(r["N"]) not in args.N]
    with path.open("w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=list(rows[0]))
        writer.writeheader()
        writer.writerows(existing + rows)
    (RESULTS / "machine.json").write_text(json.dumps(machine(), indent=2))
    print(f"wrote {path}")


if __name__ == "__main__":
    main()
