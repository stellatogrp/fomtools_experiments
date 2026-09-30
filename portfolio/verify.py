"""Worst-case ADMM residuals for portfolio rebalancing, with and without the budget on ``x``.

The two parameter sets are the capped simplex ``{1'x = 1, 0 <= x <= z_bar}``
and its bounding box ``[0, z_bar]^n``. They differ by one ``add_constraint``
call. Runs every (set, backend) pair over K = 1..5 and writes
``results/bounds.csv``.
"""

from __future__ import annotations

import csv
import time
from pathlib import Path

import numpy as np
import scipy.sparse as spa
from fomtools.linearexpression import stack
from fomtools.verifier import Verifier

from problem import PortfolioProblem, make_problem, sample_box, sample_simplex

RESULTS = Path(__file__).parent / "results"
N_SAMPLES = 10_000
PARAM_SETS = ("box", "simplex")
# The SDP is left out for now; see the README.
BACKENDS = ("mip", "simulator")


def make_verifier(backend: str) -> Verifier:
    match backend:
        case "mip":
            return Verifier(
                method="mip", solver="gurobi", obbt_solver="gurobi", params={"OutputFlag": 0}
            )
        case "sdp":
            # RLT cuts only, on the default chordal PSD cones. With every cut on,
            # the SDP exhausted memory here.
            return Verifier(method="sdp", solver="clarabel", obbt_solver="gurobi", rlt_cuts=True)
        case "simulator":
            return Verifier(method="simulator", obbt_solver=None)
    raise ValueError(backend)


def run(
    problem: PortfolioProblem,
    param_set: str,
    backend: str,
    samples: np.ndarray,
    ord: int | float = np.inf,
) -> list[dict]:
    n, K = problem.n, problem.K
    rho, sigma = problem.rho, problem.sigma
    # All matrices are sparse to work around a fomtools stack() bug (see problem.py).
    B, M = problem.blocks, problem.M
    vp = make_verifier(backend)

    if backend == "simulator":
        # The simulator takes the parameter set through its samples.
        x = vp.add_param(n, samples=samples)
        tau0 = vp.add_iterate(n, init=np.zeros(n))
    else:
        x = vp.add_param(n, lb=0.0, ub=problem.z_bar)
        if param_set == "simplex":
            vp.add_constraint(spa.csc_matrix(np.ones((1, n))) @ x == 1.0)
        tau0 = vp.add_iterate(n, lb=0.0, ub=0.0)

    # Warm start at the current holdings: y^0 = (x, 0), v^0 = M y^0.
    y0 = stack([x, tau0])
    s = vp.iterate_trajectory(K + 1)
    s[0] = stack([y0, M @ y0])

    d = 2 * n
    rows = []
    for k in range(1, K + 1):
        y, v = s[k - 1][:d], s[k - 1][d:]
        v_budget, v_pos = v[:1], v[1 : 1 + n]
        v_sell, v_buy = v[1 + n : 1 + 2 * n], v[1 + 2 * n :]

        # w = projection of v onto [l(x), u(x)], block by block.
        w_pos = vp.step.satlin(v_pos, l=np.zeros(n), u=np.full(n, problem.z_bar))
        w_sell = x - vp.step.relu(x - v_sell)  # min(v, x)
        w_buy = x + vp.step.relu(v_buy - x)  # max(v, x)

        rhs = (
            sigma * y
            - problem.q
            + rho * B["budget"].T @ (2.0 - v_budget)
            + rho * B["position"].T @ (2 * w_pos - v_pos)
            + rho * B["sell"].T @ (2 * w_sell - v_sell)
            + rho * B["buy"].T @ (2 * w_buy - v_buy)
        )
        y_next = vp.step.implicit_linear(rhs, output_mat=problem.lhs)
        v_next = stack(
            [
                v_budget + B["budget"] @ y_next - 1.0,
                v_pos + B["position"] @ y_next - w_pos,
                v_sell + B["sell"] @ y_next - w_sell,
                v_buy + B["buy"] @ y_next - w_buy,
            ]
        )
        s[k] = stack([y_next, v_next])
        vp.set_objective_norm([problem.H @ (s[k] - s[k - 1])], ord=ord)
        if backend == "simulator":
            continue
        start = time.perf_counter()
        value = vp.solve()
        rows.append(dict(K=k, value=float(value), solve_time=time.perf_counter() - start))
        print(
            f"    {backend} K={k}: {rows[-1]['value']:.4f} ({rows[-1]['solve_time']:.2f} s)",
            flush=True,
        )

    if backend == "simulator":
        start = time.perf_counter()
        residuals = np.asarray(vp.solve())
        elapsed = time.perf_counter() - start
        rows = [
            dict(K=k, value=float(residuals[:, k - 1].max()), solve_time=elapsed)
            for k in range(1, K + 1)
        ]
    return [dict(param_set=param_set, backend=backend, **row) for row in rows]


def main() -> None:
    problem = make_problem()
    rng = np.random.default_rng(0)
    samples = {
        "box": sample_box(problem, N_SAMPLES, rng),
        "simplex": sample_simplex(problem, N_SAMPLES, rng),
    }
    rows = []
    for param_set in PARAM_SETS:
        print(param_set)
        for backend in BACKENDS:
            new = run(problem, param_set, backend, samples[param_set])
            print(f"  {backend:9s}", " ".join(f"{r['value']:.4f}" for r in new), flush=True)
            rows += new

    RESULTS.mkdir(exist_ok=True)
    with open(RESULTS / "bounds.csv", "w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=["param_set", "backend", "K", "value", "solve_time"])
        writer.writeheader()
        writer.writerows(rows)


if __name__ == "__main__":
    main()
