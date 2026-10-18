"""Iterations and solve time to a tolerance, every solver stopping itself.

Run from ``portfolio/`` after ``generate`` (fomtools solvers generated with
``TerminationKind.OBJECTIVE``):

    python -m portfolio_cvxpygen.tolerance_stop [--N 200 500] [--rounds 7]

One stopping test for every solver, OSQP's: ``eps_abs = model.TOL``,
``eps_rel = 0``, unscaled residuals of the canonical problem, and nothing else
(OSQP's duality-gap test, on by default in OSQP 1.x, is off: ``check_dualgap =
False``), checked every
``model.CHECK_TERMINATION`` iterations, at most ``model.MAX_ITER``. The same ADMM
for every solver: rho fixed at ``model.OSQP_RHO`` (= the fomtools rho), adaptive
rho off, sigma 1e-6, alpha 1.6, cold start, no polishing.

``cvxpy_osqp``
    ``problem.solve(solver=OSQP)`` with those settings (OSQP's Ruiz scaling on).
``cpg_osqp``
    stock CVXPYgen + OSQP. CVXPYgen generates OSQP at rho 0.1 and exposes no
    setting for rho or adaptive rho, so both levels call the embedded
    ``osqp_update_rho`` once before the first solve and turn adaptive rho off: the
    C driver directly, the Python wrapper through ``ctypes`` on its own extension
    module (``fixed_rho_osqp``, which also turns ``check_dualgap`` off), because its ``cpg_solve`` resets the settings to
    OSQP's defaults before every solve.
``cpg_fom`` / ``cpg_fom_opt``
    CVXPYgen + fomtools, translated / AI-optimized, with the runtime ``tol``,
    ``check_termination`` and ``max_iter`` above.

Iterations and residuals come from the timed runs themselves: per solve, the
iteration count at the stop and ``max(pri_res, dua_res)`` there, at both levels
(C: one extra untimed pass over the sequence). Timing: interleaved rounds, the median of the per-round means, 3 warm-up solves and one
pass in Python, 1 warm-up and ``--passes`` (default ``C_PASSES``) timed passes in
C. ``--iters-only`` arms run once, for their iteration counts and residuals, and
are not timed. Two CVXPYgen wrappers are
not loaded into one Python process: every Python round of every arm is its own
subprocess (``--worker``).

Writes ``out_factor/stop_n{N}.json`` (per-solve iterations and residuals) and
``results_factor/timings_stop.csv``.
"""

from __future__ import annotations

import argparse
import csv
import ctypes
import json
import platform
import re
import subprocess
import sys
import time
from pathlib import Path

import cvxpy as cp
import numpy as np

from portfolio_cvxpygen.model import (
    CHECK_TERMINATION,
    MAX_ITER,
    N_SOLVES,
    OSQP_RHO,
    OUT,
    PORTFOLIO,
    RESULTS,
    SIZES,
    TOL,
    build,
    code_dir,
    holdings_sequence,
    load_cpg_solve,
    prefix,
)

ARMS = ("cvxpy_osqp", "cpg_osqp", "cpg_fom", "cpg_fom_opt")
C_ARMS = ("cpg_osqp", "cpg_fom", "cpg_fom_opt")
GENERATED = {"cpg_osqp": "osqp", "cpg_fom": "fom", "cpg_fom_opt": "fom_opt"}
#: ``bench_c`` build mode per C arm: OSQP at a fixed rho, or the fomtools solver.
C_MODE = {"cpg_osqp": "OFF", "cpg_fom": "FOM", "cpg_fom_opt": "FOM"}
BENCH_SRC = Path(__file__).parent / "bench_c"
WARMUP = 3
C_PASSES = 20

#: OSQP's stopping test, as CVXPY's OSQP interface takes it.
OSQP_STOP = dict(
    max_iter=MAX_ITER,
    check_termination=CHECK_TERMINATION,
    eps_abs=TOL,
    eps_rel=0.0,
    scaled_termination=False,
    check_dualgap=False,
)
CVXPY_OSQP = dict(
    OSQP_STOP,
    rho=OSQP_RHO,
    sigma=1e-6,
    alpha=1.6,
    adaptive_rho=False,
    polishing=False,
    warm_starting=False,
    warm_start=False,
)
#: The same test through the CVXPYgen wrappers' runtime settings.
CPG_STOP = {
    "cpg_osqp": dict(
        {k: v for k, v in OSQP_STOP.items() if k != "check_dualgap"},  # no setter: ctypes
        scaled_termination=0, warm_start=False,
    ),
    "cpg_fom": dict(max_iter=MAX_ITER, check_termination=CHECK_TERMINATION, tol=TOL),
    "cpg_fom_opt": dict(max_iter=MAX_ITER, check_termination=CHECK_TERMINATION, tol=TOL),
}

OFFSETS_C = r"""
#include <stddef.h>
#include <stdio.h>
#include "osqp_api_types.h"
int main(void) {
  OSQPSettings s;
  printf("%zu %zu %zu %zu %zu\n", offsetof(OSQPSettings, rho), offsetof(OSQPSettings, adaptive_rho),
         sizeof s.adaptive_rho, offsetof(OSQPSettings, check_dualgap), sizeof s.check_dualgap);
  return 0;
}
"""


def _settings_offsets(N: int) -> tuple[int, int, int, int, int]:
    """Byte offsets of ``rho``, ``adaptive_rho`` and ``check_dualgap`` in the generated
    ``OSQPSettings``, each integer field followed by its size (``OSQPInt`` is 8 bytes
    with ``OSQP_USE_LONG``)."""
    solver_code = OUT / code_dir(N, "osqp") / "c" / "solver_code"
    build_dir = OUT / "bench_build" / f"n{N}_osqp_offsets"
    build_dir.mkdir(parents=True, exist_ok=True)
    (build_dir / "offsets.c").write_text(OFFSETS_C)
    subprocess.run(
        ["cc", f"-I{solver_code / 'inc' / 'public'}", f"-I{solver_code}",
         str(build_dir / "offsets.c"), "-o", str(build_dir / "offsets")],
        check=True, capture_output=True,
    )
    out = subprocess.run([str(build_dir / "offsets")], check=True, capture_output=True, text=True)
    rho, adaptive, adaptive_size, dualgap, dualgap_size = (int(v) for v in out.stdout.split())
    return rho, adaptive, adaptive_size, dualgap, dualgap_size


def fixed_rho_osqp(N: int, rho: float = OSQP_RHO):
    """The CVXPYgen + OSQP ``cpg_solve`` at a fixed rho, adaptive rho off, and
    the duality-gap test off.

    The wrapper's ``cpg_solve`` calls ``cpg_module.set_solver_default_settings()``
    before every solve, which puts ``settings.rho`` back to 0.1 and adaptive rho
    on (and ``check_dualgap`` on). The KKT factorization and ``rho_vec`` hold the value of the last
    ``osqp_update_rho``, so this calls it once at ``rho`` and then, after every
    reset, writes ``rho``, ``adaptive_rho = 0`` and ``check_dualgap = 0`` back into the settings, which is
    what the C driver does once."""
    module = sys.modules[load_cpg_solve(N, "osqp").__module__]
    ext = module.cpg_module
    lib = ctypes.CDLL(ext.__file__)
    solver = ctypes.addressof(ctypes.c_byte.in_dll(lib, "solver"))
    settings = ctypes.addressof(ctypes.c_byte.in_dll(lib, "settings"))
    off_rho, off_adaptive, size_adaptive, off_dualgap, size_dualgap = _settings_offsets(N)
    c_osqp_int = {4: ctypes.c_int32, 8: ctypes.c_int64}
    settings_rho = ctypes.c_double.from_address(settings + off_rho)
    settings_adaptive = c_osqp_int[size_adaptive].from_address(settings + off_adaptive)
    settings_dualgap = c_osqp_int[size_dualgap].from_address(settings + off_dualgap)
    # solver.settings must point at the ``settings`` global for the writes to land.
    assert ctypes.c_void_p.from_address(solver).value == settings
    lib.osqp_update_rho.argtypes = [ctypes.c_void_p, ctypes.c_double]
    if lib.osqp_update_rho(solver, rho) != 0:
        raise RuntimeError("osqp_update_rho failed")
    assert settings_rho.value == rho

    class Ext:
        def __getattr__(self, name):
            return getattr(ext, name)

        @staticmethod
        def set_solver_default_settings():
            ext.set_solver_default_settings()
            settings_rho.value = rho
            settings_adaptive.value = 0
            settings_dualgap.value = 0

    module.cpg_module = Ext()
    return module.cpg_solve


def python_arm(N: int, arm: str):
    """``solve(x) -> (iters, max residual, solved)``: one self-stopping solve of ``arm``."""
    problem, x, _ = build(N)
    if arm == "cvxpy_osqp":

        def solve(xv):
            x.value = xv
            problem.solve(solver=cp.OSQP, **CVXPY_OSQP)
            info = problem.solver_stats.extra_stats.info
            return int(info.iter), max(info.prim_res, info.dual_res), problem.status == cp.OPTIMAL

        return solve

    method = f"CPG_STOP_{arm}_n{N}"
    cpg_solve = fixed_rho_osqp(N) if arm == "cpg_osqp" else load_cpg_solve(N, GENERATED[arm])
    problem.register_solve(method, cpg_solve)
    settings = CPG_STOP[arm]

    def solve(xv):
        x.value = xv
        problem.solve(method=method, **settings)
        stats = problem.solver_stats.extra_stats
        solved = stats["status"] == "solved" if arm == "cpg_osqp" else str(stats["status"]).startswith("0")
        return int(stats["iter"]), max(stats["pri_res"], stats["dua_res"]), solved

    return solve


def worker(N: int, arm: str) -> dict:
    """One Python round of ``arm``: mean time per solve over the sequence, and the
    iterations and residuals of every solve."""
    X = holdings_sequence(N)
    solve = python_arm(N, arm)
    for xv in X[:WARMUP]:
        solve(xv)
    times, iters, res, solved = [], [], [], []
    for xv in X:
        start = time.perf_counter()
        k, r, ok = solve(xv)
        times.append(time.perf_counter() - start)
        iters.append(k)
        res.append(r)
        solved.append(ok)
    return dict(us_per_solve=1e6 * float(np.mean(times)), iters=iters, res=res,
                unsolved=int(len(solved) - sum(solved)))


def run_worker(N: int, arm: str) -> dict:
    out = subprocess.run(
        [sys.executable, "-m", "portfolio_cvxpygen.tolerance_stop", "--worker", str(N), arm],
        check=True, capture_output=True, text=True, cwd=PORTFOLIO,
    ).stdout
    return json.loads(out.strip().splitlines()[-1])


def holdings_file(N: int) -> Path:
    path = OUT / "bench_build" / f"holdings_n{N}.bin"
    path.parent.mkdir(parents=True, exist_ok=True)
    holdings_sequence(N).astype(np.float64).tofile(path)
    return path


def build_c(N: int, arm: str) -> Path:
    generated, mode = GENERATED[arm], C_MODE[arm]
    build_dir = OUT / "bench_build" / f"n{N}_{arm}"
    exe = build_dir / "bench_cpg"
    gen_dir = OUT / code_dir(N, generated)
    defines = [
        "-DCMAKE_BUILD_TYPE=Release",
        f"-DGEN_DIR={gen_dir}",
        f"-DPREFIX={prefix(N, generated)}",
        f"-DMODE={mode}",
        *([f"-DBENCH_RHO={OSQP_RHO}"] if mode == "OFF" else []),
    ]
    key_file = build_dir / "bench_defines.txt"
    sources = [*gen_dir.rglob("*"), *BENCH_SRC.iterdir()]
    newest = max(f.stat().st_mtime for f in sources if f.is_file())
    # Rebuild when the generated solver, the driver or a -D value (rho) changed
    # since the last build: a stale binary would time a different solver.
    key = "\n".join(defines)
    if (not exe.exists() or exe.stat().st_mtime < newest
            or not key_file.exists() or key_file.read_text() != key):
        (build_dir / "CMakeCache.txt").unlink(missing_ok=True)  # no cached -D values
        subprocess.run(["cmake", "-S", str(BENCH_SRC), "-B", str(build_dir), *defines],
                       check=True, capture_output=True)
        subprocess.run(["cmake", "--build", str(build_dir), "--target", "bench_cpg", "-j"],
                       check=True, capture_output=True)
        key_file.write_text(key)
    return exe


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


def time_c(N: int, arm: str, holdings: Path, passes: int = C_PASSES) -> dict:
    out = subprocess.run(
        [str(build_c(N, arm)), str(holdings), str(N_SOLVES), str(N), str(passes),
         f"stop={TOL:g},{CHECK_TERMINATION},{MAX_ITER}"],
        check=True, capture_output=True, text=True,
    ).stdout
    fields = dict(re.findall(r"(\w+)=(\S+)", out))
    return dict(
        us_per_solve=float(fields["us_per_solve"]),
        iters=[int(v) for v in fields["iters"].split(",")],
        res=[float(v) for v in fields["res"].split(",")],
        unsolved=int(fields["unsolved"]),
    )


def check_levels(N: int, record: dict) -> None:
    """Every CVXPYgen arm stops at the same iteration in C and in Python, solve by
    solve: the same generated solver, the same settings (for ``cpg_osqp``, this is
    what shows the ``fixed_rho_osqp`` override took effect)."""
    for arm in C_ARMS:
        if f"{arm}/c" not in record or f"{arm}/python" not in record:
            continue
        c, py = record[f"{arm}/c"]["iters"], record[f"{arm}/python"]["iters"]
        diff = [k for k, (a, b) in enumerate(zip(c, py)) if a != b]
        if len(c) != len(py) or diff:
            k = diff[0] if diff else min(len(c), len(py))
            raise AssertionError(f"N={N} {arm}: C and Python iterations differ at solve {k}"
                                 f" ({len(diff)} solves differ, lengths {len(c)}, {len(py)})")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--N", type=int, nargs="+", default=list(SIZES))
    parser.add_argument("--rounds", type=int, default=7)
    parser.add_argument("--arms", nargs="+", choices=ARMS, default=list(ARMS))
    parser.add_argument("--passes", type=int, default=C_PASSES,
                        help="timed passes over the sequence per C run")
    parser.add_argument("--iters-only", nargs="*", choices=ARMS, default=[],
                        help="arms run once for their iterations and residuals, not timed")
    parser.add_argument("--worker", nargs=2, metavar=("N", "ARM"), help=argparse.SUPPRESS)
    args = parser.parse_args()
    if args.worker:
        print(json.dumps(worker(int(args.worker[0]), args.worker[1])))
        return

    RESULTS.mkdir(parents=True, exist_ok=True)
    rows = []
    for N in args.N:
        holdings = holdings_file(N)
        samples: dict[tuple[str, str], list[dict]] = {}
        for arm in args.iters_only:
            samples[(arm, "python")] = [run_worker(N, arm)]
        timed = [arm for arm in args.arms if arm not in args.iters_only]
        for r in range(args.rounds):
            for arm in timed:
                samples.setdefault((arm, "python"), []).append(run_worker(N, arm))
                if arm in C_ARMS:
                    samples.setdefault((arm, "c"), []).append(
                        time_c(N, arm, holdings, args.passes))
            print(f"N={N} round {r + 1}/{args.rounds}", flush=True)

        record = {}
        for (arm, level), values in samples.items():
            # Cold starts on a fixed sequence: every round stops at the same counts.
            for v in values[1:]:
                assert v["iters"] == values[0]["iters"], (N, arm, level)
            us = np.array([v["us_per_solve"] for v in values])
            med = float(np.median(us))
            first = values[0]
            untimed = arm in args.iters_only
            row = dict(
                N=N, arm=arm, level=level, tol=TOL, check_termination=CHECK_TERMINATION,
                us_per_solve="" if untimed else med,
                mean_iters=float(np.mean(first["iters"])),
                max_iters=int(np.max(first["iters"])),
                max_res=float(np.max(first["res"])),
                unsolved=first["unsolved"],
                round_spread="" if untimed else float(np.max(np.abs(us - med)) / med),
                rounds=0 if untimed else args.rounds,
                passes=(args.passes if level == "c" else 1) if not untimed else "",
            )
            rows.append(row)
            record[f"{arm}/{level}"] = dict(iters=first["iters"], res=first["res"],
                                            unsolved=first["unsolved"], us_per_round=us.tolist())
            timing = "   (not timed)" if untimed else f"{med:12.2f} us"
            print(f"  {arm:12s} {level:6s} {timing}  iters {row['mean_iters']:.1f}"
                  f" (max {row['max_iters']})  max res {row['max_res']:.2e}"
                  f"  unsolved {row['unsolved']}", flush=True)
        record_path = OUT / f"stop_n{N}.json"  # keeps the arms not re-run
        earlier = json.loads(record_path.read_text()) if record_path.exists() else {}
        record = dict(earlier, tol=TOL, check_termination=CHECK_TERMINATION, max_iter=MAX_ITER, **record)
        check_levels(N, record)
        record_path.write_text(json.dumps(record))

    path = RESULTS / "timings_stop.csv"
    existing = []
    if path.exists():  # keep rows for (N, arm) not re-run
        with path.open() as f:
            existing = [r for r in csv.DictReader(f)
                        if int(r["N"]) not in args.N or r["arm"] not in args.arms]
    with path.open("w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=list(rows[0]))
        writer.writeheader()
        writer.writerows(existing + rows)
    (RESULTS / "machine_stop.json").write_text(json.dumps(machine(), indent=2))
    print(f"wrote {path}")


if __name__ == "__main__":
    main()
