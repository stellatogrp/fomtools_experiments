"""Sanity checks for the NNLS experiment (run after ``verify.py``).

1. Both variants converge to the NNLS solution from CVXPY (Clarabel) in the
   original variables ``z = D s``.
2. For each variant and K, sampled residual <= MIP bound <= SDP bound.
"""

from __future__ import annotations

import csv
from collections import defaultdict
from pathlib import Path

import cvxpy as cp
import numpy as np

from problem import make_problem, pgd_residuals, variant

RESULTS = Path(__file__).parent / "results"
TOL = 1e-4


def solve_nnls(A: np.ndarray, x: np.ndarray) -> np.ndarray:
    z = cp.Variable(A.shape[1])
    prob = cp.Problem(cp.Minimize(0.5 * cp.sum_squares(A @ z - x)), [z >= 0])
    prob.solve(solver=cp.CLARABEL)
    assert prob.status == cp.OPTIMAL, prob.status
    return z.value


def check_solutions() -> None:
    problem = make_problem()
    rng = np.random.default_rng(1)
    for x in rng.uniform(problem.x_lb, problem.x_ub, size=(20, problem.m)):
        z_star = solve_nnls(problem.A, x)
        for equilibrate in (False, True):
            v = variant(problem, equilibrate)
            s = v.s0.copy()
            for _ in range(20_000):
                s = np.maximum(s - v.t * v.A_hat.T @ (v.A_hat @ s - x), 0.0)
            z = v.D @ s
            assert np.allclose(z, z_star, atol=1e-6), (v.name, z, z_star)
    print("solutions: both variants reach the CVXPY (Clarabel) NNLS solution")


def check_bounds() -> None:
    problem = make_problem()
    values = defaultdict(dict)
    with open(RESULTS / "bounds.csv") as f:
        for row in csv.DictReader(f):
            values[row["variant"], row["backend"]][int(row["K"])] = float(row["value"])

    X = np.random.default_rng(2).uniform(problem.x_lb, problem.x_ub, size=(10_000, problem.m))
    for equilibrate in (False, True):
        v = variant(problem, equilibrate)
        sampled = pgd_residuals(v, X, problem.K).max(axis=0)
        for k in range(1, problem.K + 1):
            mip, sdp = values[v.name, "mip"][k], values[v.name, "sdp"][k]
            sim = values[v.name, "simulator"][k]
            assert max(sampled[k - 1], sim) <= mip * (1 + TOL), (v.name, k, sampled, sim, mip)
            assert mip <= sdp * (1 + TOL), (v.name, k, mip, sdp)
    print("bounds: sampled <= MIP <= SDP for every variant and K")


if __name__ == "__main__":
    check_solutions()
    check_bounds()
