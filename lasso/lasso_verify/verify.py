"""Worst-case KKT residuals of ISTA and FISTA on a small nonstrongly convex Lasso.

Two studies, each writing ``results/<study>_bounds.csv``:

- ``milp``: ∞-norm, K = 1..10, Gurobi MIP against the sample max. ``value`` is
  Gurobi's best bound (``ObjBound``), a certified upper bound on the worst case
  at the 5% gap; the incumbent (``ObjVal``) is kept in ``objval``.
- ``sdp``: 2-norm, K = 1..3, Clarabel SDP against the sample max. The SDP has
  all five cuts, and every PSD cone is coupled into one cone over the whole
  moment matrix, which Clarabel decomposes itself (see ``clarabel_dual.py``).
  fomtools maximizes the squared 2-norm; ``value`` is its square root, and
  ``status`` is Clarabel's termination status.

Run ``python verify.py [milp|sdp]``; with no argument both studies run.
"""

from __future__ import annotations

import argparse
import csv
import time
from pathlib import Path

import numpy as np
from fomtools.linearexpression import stack
from fomtools.verifier import Verifier

import clarabel_dual
from problem import METHODS, LassoProblem, make_problem, momentum, residual_map

RESULTS = Path(__file__).parent / "results"
N_SAMPLES = 10_000
STUDIES = {
    "milp": dict(backends=("mip", "simulator"), ord=np.inf, K=10),
    "sdp": dict(backends=("sdp", "simulator"), ord=2, K=3),
}


def make_verifier(backend: str) -> Verifier:
    match backend:
        case "mip":
            return Verifier(
                method="mip",
                solver="gurobi",
                obbt_solver="gurobi",
                params={"OutputFlag": 0, "MIPGap": 0.05},
            )
        case "sdp":
            # All cuts on fomtools' chordal cut set (RLT only on cone-mates), then
            # one coupled cone over X that Clarabel decomposes, as in portfolio.
            clarabel_dual.install()
            return Verifier(
                method="sdp",
                solver="clarabel",
                obbt_solver="gurobi",
                obbt_params={"OutputFlag": 0},
                rlt_cuts=True,
                triangle_cuts=True,
                relu_envelope_cuts=True,
                cross_cut=True,
                subgrad_mono_cut=True,
            )
        case "simulator":
            return Verifier(method="simulator", obbt_solver=None)
    raise ValueError(backend)


def run(
    problem: LassoProblem,
    method: str,
    backend: str,
    samples: np.ndarray,
    K: int,
    ord: float,
) -> list[dict]:
    D, eta = problem.D, problem.eta
    gammas = momentum(K) if method == "fista" else np.zeros(K)
    # fomtools returns the squared norm for ord=2.
    unsquare = np.sqrt if ord == 2 else (lambda v: v)
    vp = make_verifier(backend)
    if backend == "simulator":
        x = vp.add_param(problem.p, samples=samples)
        u0 = vp.add_iterate(problem.n, init=problem.s0)
    else:
        x = vp.add_param(problem.p, lb=problem.x_lb, ub=problem.x_ub)
        u0 = vp.add_iterate(problem.n, lb=problem.s0, ub=problem.s0)

    u = vp.iterate_trajectory(K + 1)
    u[0] = u0
    w = u[0]
    rows = []
    for k in range(1, K + 1):
        u[k] = vp.step.soft_threshold(w - eta * D.T @ (D @ w - x), lambd=problem.lambd * eta)
        if method == "ista":
            ds = u[k] - u[k - 1]
        else:
            # s^k = (u^k, u^{k-1}), with u^{-1} = u^0.
            ds = stack([u[k] - u[k - 1], u[k - 1] - u[max(k - 2, 0)]])
        vp.set_objective_norm([residual_map(problem, method, k) @ ds], ord=ord)
        w = u[k] + gammas[k - 1] * (u[k] - u[k - 1])
        if backend == "simulator":
            continue
        start = time.perf_counter()
        objval = vp.solve()
        elapsed = time.perf_counter() - start
        if backend == "mip":
            # The best bound, not the incumbent, is the certified upper bound.
            bound, psd_dim, status = vp.canonicalizer._solved_copy["ObjBound"], "", ""
        else:
            bound, objval = unsquare(objval), ""
            # Side of the single coupled moment matrix handed to Clarabel.
            psd_dim, status = clarabel_dual.LAST["psd_dim"], clarabel_dual.LAST["status"]
        rows.append(
            dict(
                K=k,
                value=float(bound),
                objval=objval,
                psd_dim=psd_dim,
                status=status,
                solve_time=elapsed,
            )
        )
        size = (
            f", PSD {psd_dim}x{psd_dim}, {clarabel_dual.LAST['pattern']} entries used, {status}"
            if psd_dim
            else ""
        )
        print(f"    {backend} K={k}: {bound:.4f} ({elapsed:.2f} s{size})", flush=True)

    if backend == "simulator":
        start = time.perf_counter()
        residuals = unsquare(np.asarray(vp.solve()))
        elapsed = time.perf_counter() - start
        rows = [
            dict(
                K=k,
                value=float(residuals[:, k - 1].max()),
                objval="",
                psd_dim="",
                status="",
                solve_time=elapsed,
            )
            for k in range(1, K + 1)
        ]
    return [dict(method=method, backend=backend, **row) for row in rows]


def run_study(problem: LassoProblem, study: str, samples: np.ndarray) -> None:
    spec = STUDIES[study]
    print(f"== {study}: ord={spec['ord']}, K=1..{spec['K']}")
    rows = []
    for method in METHODS:
        print(method)
        for backend in spec["backends"]:
            new = run(problem, method, backend, samples, spec["K"], spec["ord"])
            print(f"  {backend:9s}", " ".join(f"{r['value']:.4f}" for r in new), flush=True)
            rows += new

    RESULTS.mkdir(exist_ok=True)
    with open(RESULTS / f"{study}_bounds.csv", "w", newline="") as f:
        fields = ["method", "backend", "K", "value", "objval", "psd_dim", "status", "solve_time"]
        writer = csv.DictWriter(f, fieldnames=fields)
        writer.writeheader()
        writer.writerows(rows)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("study", nargs="?", choices=(*STUDIES, "all"), default="all")
    args = parser.parse_args()

    problem = make_problem()
    print(f"eta = {problem.eta:.4g}, x in [{problem.x_lb.round(3)}, {problem.x_ub.round(3)}]")
    print(f"s0 = {problem.s0.round(3)}, gamma = {momentum(problem.K).round(3)}")
    samples = np.random.default_rng(0).uniform(
        problem.x_lb, problem.x_ub, size=(N_SAMPLES, problem.p)
    )
    for study in STUDIES if args.study == "all" else (args.study,):
        run_study(problem, study, samples)


if __name__ == "__main__":
    main()
