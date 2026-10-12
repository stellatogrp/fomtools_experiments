"""Worst-case original-LP KKT residuals for PDHG and momentum PDHG, with and without Ruiz.

Runs the MIP backend (best bound at a 5% gap) and the simulator (sample max) over
K = 1..K for all four variants and writes ``results/bounds_mipinf.csv``. The
simulator is cross-checked against the NumPy reference iteration.
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
N_SAMPLES = 1_000
# Relative MIP gap: Gurobi stops once the incumbent is within 5% of the best bound.
MIP_GAP = 0.05


def run(
    problem: NetworkProblem, v: PDHGVariant, backend: str, K: int, samples: np.ndarray | None = None
) -> list[dict]:
    """Worst-case (MIP) or sampled (simulator) ``||H_k(s^k - s^{k-1})||∞`` for ``k = 1..K``."""
    ne, ns = problem.ne, problem.ns
    if backend == "mip":
        vp = Verifier(
            method="mip",
            solver="gurobi",
            obbt_solver="gurobi",
            params={"OutputFlag": 0, "MIPGap": MIP_GAP},
        )
        x = vp.add_param(problem.nd, lb=problem.x_lb, ub=problem.x_ub)
        if samples is not None:
            # The worst sample at each K is given to Gurobi as a MIP start.
            vp.set_start_samples({x: samples})
        init = [vp.add_iterate(len(a), lb=a, ub=a) for a in (v.z0, v.v0, v.w0)]
    else:
        vp = Verifier(method="simulator", obbt_solver=None)
        x = vp.add_param(problem.nd, samples=samples)
        init = [vp.add_iterate(len(a), init=a) for a in (v.z0, v.v0, v.w0)]
    z, y, w = (vp.iterate_trajectory(K + 1) for _ in range(3))
    z[0], y[0], w[0] = init
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
        # H(k) unscales the residual, so every variant measures the original LP's KKT residual.
        H = v.H(k)
        H_z, H_v, H_w = H[:, :ne], H[:, ne : ne + ns], H[:, ne + ns :]
        vp.set_objective_norm(
            [H_z @ (z[k] - z[k - 1]) + H_v @ (y[k] - y[k - 1]) + H_w @ (w[k] - w[k - 1])],
            ord=np.inf,
        )
        if backend != "mip":
            continue
        start = time.perf_counter()
        value = float(vp.solve())
        elapsed = time.perf_counter() - start
        # The best bound, not the incumbent, is the certified upper bound.
        bound = float(vp.canonicalizer._solved_copy["ObjBound"])
        print(f"  {v.name:17s} K={k:3d}  MIP {bound:.4f}  ({elapsed:.1f} s)", flush=True)
        rows.append(
            dict(variant=v.name, backend="mip", K=k, value=value, bound=bound, solve_time=elapsed)
        )

    if backend != "mip":
        start = time.perf_counter()
        residuals = np.asarray(vp.solve())
        elapsed = time.perf_counter() - start
        rows = [
            dict(
                variant=v.name,
                backend="simulator",
                K=k,
                value=float(residuals[:, k - 1].max()),
                bound=float("nan"),
                solve_time=elapsed,
            )
            for k in range(1, K + 1)
        ]
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
        rows += run(problem, v, "mip", K, samples)
        sim = run(problem, v, "simulator", K, samples)
        # Cross-check the simulator against the NumPy reference iteration. The
        # simulator computes in single precision, so they agree to about 1e-5.
        ref = pdhg_residuals(v, samples, K).max(axis=0)
        err = max(abs(r["value"] - ref[r["K"] - 1]) for r in sim)
        print(f"  simulator vs NumPy: max abs diff {err:.1e}")
        assert err < 1e-4, err
        rows += sim

    RESULTS.mkdir(exist_ok=True)
    with open(RESULTS / "bounds_mipinf.csv", "w", newline="") as f:
        fields = ["variant", "backend", "K", "value", "bound", "solve_time"]
        writer = csv.DictWriter(f, fieldnames=fields)
        writer.writeheader()
        writer.writerows(rows)


if __name__ == "__main__":
    main()
