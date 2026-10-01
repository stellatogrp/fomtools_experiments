"""Worst-case original-LP KKT residuals for PDHG and momentum PDHG, with and without Ruiz.

Runs the MIP backend and the sampled maximum over K = 1..K for all four variants
and writes ``results/bounds_mipinf.csv``.
"""

from __future__ import annotations

import argparse
import csv
import time
from pathlib import Path

import numpy as np
from fomtools.verifier import Verifier

from problem import NetworkProblem, PDHGVariant, make_problem, pdhg_residuals, variant

RESULTS = Path(__file__).parent / "results"
N_SAMPLES = 10_000


def run_mip(problem: NetworkProblem, v: PDHGVariant, K: int) -> list[dict]:
    vp = Verifier(method="mip", solver="gurobi", obbt_solver="gurobi", params={"OutputFlag": 0})
    ne, ns = problem.ne, problem.ns
    x = vp.add_param(problem.nd, lb=problem.x_lb, ub=problem.x_ub)
    z, y, w = (vp.iterate_trajectory(K + 1) for _ in range(3))
    z[0] = vp.add_iterate(ne, lb=v.z0, ub=v.z0)
    y[0] = vp.add_iterate(ns, lb=v.v0, ub=v.v0)
    w[0] = vp.add_iterate(problem.nd, lb=v.w0, ub=v.w0)
    E_d = np.diag(v.e_d)

    rows = []
    for k in range(1, K + 1):
        z[k] = vp.step.satlin(
            z[k - 1] - v.eta * (v.mu + v.A_s.T @ y[k - 1] - v.A_d.T @ w[k - 1]),
            l=np.zeros(ne),
            u=v.c,
        )
        z_bar = 2 * (z[k] + v.xi(k - 1) * (z[k] - z[k - 1])) - z[k - 1]
        y[k] = vp.step.relu(y[k - 1] + v.eta * (v.A_s @ z_bar - v.b_s))
        w[k] = w[k - 1] + v.eta * (E_d @ x - v.A_d @ z_bar)
        H = v.H(k)
        H_z, H_v, H_w = H[:, :ne], H[:, ne : ne + ns], H[:, ne + ns :]
        vp.set_objective_norm(
            [H_z @ (z[k] - z[k - 1]) + H_v @ (y[k] - y[k - 1]) + H_w @ (w[k] - w[k - 1])],
            ord=np.inf,
        )
        start = time.perf_counter()
        value = float(vp.solve())
        elapsed = time.perf_counter() - start
        print(f"  {v.name:17s} K={k:3d}  MIP {value:.4f}  ({elapsed:.1f} s)", flush=True)
        rows.append(dict(variant=v.name, backend="mip", K=k, value=value, solve_time=elapsed))
    return rows


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--K", type=int, default=None, help="largest K (default: problem.K)")
    args = parser.parse_args()

    problem = make_problem()
    K = args.K or problem.K
    samples = np.random.default_rng(0).uniform(
        problem.x_lb, problem.x_ub, size=(N_SAMPLES, problem.nd)
    )
    rows = []
    for momentum, equilibrate in ((m, e) for m in (False, True) for e in (False, True)):
        v = variant(problem, equilibrate, momentum)
        print(f"{v.name}: eta = {v.eta:.4g}, ||K|| = {0.5 / v.eta:.3g}")
        rows += run_mip(problem, v, K)
        start = time.perf_counter()
        sampled = pdhg_residuals(v, samples, K).max(axis=0)
        elapsed = time.perf_counter() - start
        rows += [
            dict(
                variant=v.name,
                backend="simulator",
                K=k,
                value=float(sampled[k - 1]),
                solve_time=elapsed,
            )
            for k in range(1, K + 1)
        ]

    RESULTS.mkdir(exist_ok=True)
    with open(RESULTS / "bounds_mipinf.csv", "w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=["variant", "backend", "K", "value", "solve_time"])
        writer.writeheader()
        writer.writerows(rows)


if __name__ == "__main__":
    main()
