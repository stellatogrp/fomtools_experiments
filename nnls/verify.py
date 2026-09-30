"""Worst-case unscaled PGD residuals for NNLS, with and without Ruiz scaling.

Runs every (variant, backend) pair over K = 1..5 and writes
``results/bounds.csv``.
"""

from __future__ import annotations

import csv
import time
from pathlib import Path

import numpy as np
from fomtools.verifier import Verifier

from problem import NNLSProblem, PGDVariant, make_problem, variant

RESULTS = Path(__file__).parent / "results"
N_SAMPLES = 10_000


def make_verifier(backend: str) -> Verifier:
    match backend:
        case "mip":
            return Verifier(
                method="mip", solver="gurobi", obbt_solver="gurobi", params={"OutputFlag": 0}
            )
        case "sdp":
            # The default SDP relaxation is loose here and grows with K. All the
            # cuts, kept on the chordal (non-full) PSD cones so the relaxation
            # can scale, make it nearly exact.
            return Verifier(
                method="sdp",
                solver="clarabel",
                obbt_solver="gurobi",
                rlt_cuts=True,
                triangle_cuts=True,
                relu_envelope_cuts=True,
                cross_cut=True,
                subgrad_mono_cut=True,
            )
        case "simulator":
            return Verifier(method="simulator", obbt_solver=None)
    raise ValueError(backend)


def run(problem: NNLSProblem, v: PGDVariant, backend: str, samples: np.ndarray) -> list[dict]:
    vp = make_verifier(backend)
    if backend == "simulator":
        x = vp.add_param(problem.m, samples=samples)
        s0 = vp.add_iterate(problem.n, init=v.s0)
    else:
        x = vp.add_param(problem.m, lb=problem.x_lb, ub=problem.x_ub)
        s0 = vp.add_iterate(problem.n, lb=v.s0, ub=v.s0)

    s = vp.iterate_trajectory(problem.K + 1)
    s[0] = s0
    rows = []
    for k in range(1, problem.K + 1):
        s[k] = vp.step.relu(s[k - 1] - v.t * v.A_hat.T @ (v.A_hat @ s[k - 1] - x))
        vp.set_infinity_norm_objective([v.H @ (s[k] - s[k - 1])])
        if backend == "simulator":
            continue
        start = time.perf_counter()
        value = vp.solve()
        rows.append(dict(K=k, value=float(value), solve_time=time.perf_counter() - start))

    if backend == "simulator":
        start = time.perf_counter()
        residuals = np.asarray(vp.solve())
        elapsed = time.perf_counter() - start
        rows = [
            dict(K=k, value=float(residuals[:, k - 1].max()), solve_time=elapsed)
            for k in range(1, problem.K + 1)
        ]
    return [dict(variant=v.name, backend=backend, **row) for row in rows]


def main() -> None:
    problem = make_problem()
    samples = np.random.default_rng(0).uniform(
        problem.x_lb, problem.x_ub, size=(N_SAMPLES, problem.m)
    )
    P = problem.A.T @ problem.A
    rows = []
    for equilibrate in (False, True):
        v = variant(problem, equilibrate)
        cond = np.linalg.cond(v.A_hat.T @ v.A_hat)
        print(f"{v.name}: cond(ÂᵀÂ) = {cond:.1f}, t = {v.t:.4g}, D = {np.diag(v.D).round(4)}")
        for backend in ("mip", "sdp", "simulator"):
            new = run(problem, v, backend, samples)
            print(f"  {backend:9s}", " ".join(f"{r['value']:.4f}" for r in new))
            rows += new
    print(f"cond(AᵀA) = {np.linalg.cond(P):.1f}")

    RESULTS.mkdir(exist_ok=True)
    with open(RESULTS / "bounds.csv", "w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=["variant", "backend", "K", "value", "solve_time"])
        writer.writeheader()
        writer.writerows(rows)


if __name__ == "__main__":
    main()
