"""Sanity checks for the Lasso experiment (run after ``verify.py``).

1. ISTA and FISTA converge to the Lasso solution from CVXPY (Clarabel).
2. ``H(s^k - s^{k-1})`` equals the KKT residual at ``u^k`` computed directly,
   ``(w^{k-1} - u^k)/η - Dᵀ(Dw^{k-1} - x) + Dᵀ(Du^k - x)``.
3. For each study, method and K, sampled residual <= MIP or SDP bound, and the
   simulator row matches the NumPy reference (the simulator runs in float32).
"""

from __future__ import annotations

import csv
from collections import defaultdict
from pathlib import Path

import cvxpy as cp
import numpy as np

from problem import METHODS, make_problem, momentum, prox_grad, residuals
from verify import STUDIES

RESULTS = Path(__file__).parent / "results"
TOL = 1e-4


def solve_lasso(D: np.ndarray, x: np.ndarray, lambd: float) -> float:
    u = cp.Variable(D.shape[1])
    prob = cp.Problem(cp.Minimize(0.5 * cp.sum_squares(D @ u - x) + lambd * cp.norm1(u)))
    prob.solve(solver=cp.CLARABEL)
    assert prob.status == cp.OPTIMAL, prob.status
    return prob.value


def check_solutions() -> None:
    # The problem is not strongly convex, so compare objective values rather than u.
    problem = make_problem()
    D, lambd = problem.D, problem.lambd
    X = np.random.default_rng(1).uniform(problem.x_lb, problem.x_ub, size=(20, problem.p))
    iters = 20_000
    gammas = {"ista": np.zeros(iters), "fista": momentum(iters)}
    for x in X:
        f_star = solve_lasso(D, x, lambd)
        for method in METHODS:
            u = w = problem.s0[None, :]
            for k in range(iters):
                u_next = prox_grad(problem, w, x[None, :])
                w = u_next + gammas[method][k] * (u_next - u)
                u = u_next
            f = 0.5 * np.sum((D @ u[0] - x) ** 2) + lambd * np.abs(u[0]).sum()
            assert abs(f - f_star) <= 1e-6 * max(1.0, abs(f_star)), (method, f, f_star)
    print("solutions: ISTA and FISTA reach the CVXPY (Clarabel) Lasso objective")


def check_kkt() -> None:
    problem = make_problem()
    D, eta, K = problem.D, problem.eta, problem.K
    X = np.random.default_rng(2).uniform(problem.x_lb, problem.x_ub, size=(500, problem.p))
    for method in METHODS:
        gammas = momentum(K) if method == "fista" else np.zeros(K)
        u = w = np.tile(problem.s0, (X.shape[0], 1))
        direct = np.empty((X.shape[0], K))
        for k in range(K):
            u_next = prox_grad(problem, w, X)
            kkt = (w - u_next) / eta - (w @ D.T - X) @ D + (u_next @ D.T - X) @ D
            direct[:, k] = np.abs(kkt).max(axis=1)
            w = u_next + gammas[k] * (u_next - u)
            u = u_next
        assert np.allclose(direct, residuals(problem, method, X, K), atol=1e-10), method
    print("kkt: H(s^k - s^{k-1}) equals the KKT residual at u^k for both methods")


def check_bounds(study: str) -> None:
    problem = make_problem()
    spec = STUDIES[study]
    K, ord, backend = spec["K"], spec["ord"], spec["backends"][0]
    values = defaultdict(dict)
    with open(RESULTS / f"{study}_bounds.csv") as f:
        for row in csv.DictReader(f):
            values[row["method"], row["backend"]][int(row["K"])] = float(row["value"])

    X = np.random.default_rng(3).uniform(problem.x_lb, problem.x_ub, size=(10_000, problem.p))
    X_sim = np.random.default_rng(0).uniform(problem.x_lb, problem.x_ub, size=(10_000, problem.p))
    for method in METHODS:
        sampled = residuals(problem, method, X, K, ord).max(axis=0)
        reference = residuals(problem, method, X_sim, K, ord).max(axis=0)
        for k in range(1, K + 1):
            bound, sim = values[method, backend][k], values[method, "simulator"][k]
            assert np.isclose(sim, reference[k - 1], rtol=1e-5), (method, k, sim, reference)
            assert max(sampled[k - 1], sim) <= bound * (1 + TOL), (method, k, sampled, sim, bound)
    print(f"bounds ({study}): simulator matches NumPy, and sampled <= {backend} for every method and K")

if __name__ == "__main__":
    check_solutions()
    check_kkt()
    for study in STUDIES:
        check_bounds(study)
