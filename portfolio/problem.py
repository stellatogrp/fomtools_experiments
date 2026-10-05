"""Portfolio rebalancing with an l1 trading cost, and its OSQP-style ADMM.

The problem is

    minimize    (kappa/2) z' Sigma z - mu' z + lambd ||z - x||_1
    subject to  1' z = 1,  0 <= z <= z_bar,

with risk aversion ``kappa``, transaction cost ``lambd``, and the vector
``z_bar`` of position limits. The parameter ``x`` is the portfolio currently held. Last period's
holdings were feasible for last period's problem, so ``x`` lies in the same set
as ``z``: the capped simplex ``{x : 1'x = 1, 0 <= x <= z_bar}``.

The l1 term is written with an epigraph variable ``tau >= |z - x|``, so the
optimization variable is ``y = (z, tau)`` and the constraint rows are

    budget     1' z          = 1
    position   0 <= z        <= z_bar
    sell       z - tau       <= x
    buy        z + tau       >= x

The parameter enters only through the one-sided bounds of the last two blocks.
Their projections are ``min(v, x) = x - relu(x - v)`` and
``max(v, x) = x + relu(v - x)``, so the ADMM iteration stays affine plus ReLU
and saturation, with constant saturation bounds.

ADMM is warm-started at the primal-dual solution of the problem for the balanced
portfolio ``x = 1/n``, computed with CVXPY. The warm start is the same point for
every parameter ``x``.
"""

from __future__ import annotations

from dataclasses import dataclass

import cvxpy as cp
import numpy as np
import scipy.sparse as spa
import scipy.sparse.linalg  # noqa: F401

N = 5
K = 5
SEED = 0
KAPPA = 1.0  # risk aversion
LAMBD = 0.01  # transaction cost
Z_BAR = 0.4 * np.ones(N)  # position limits
RHO = 1.0
SIGMA = 1e-4


@dataclass(frozen=True)
class PortfolioProblem:
    Sigma: np.ndarray
    mu: np.ndarray
    kappa: float
    lambd: float
    z_bar: np.ndarray
    rho: float
    sigma: float
    K: int

    @property
    def n(self) -> int:
        return int(self.mu.size)

    @property
    def P(self) -> spa.csc_matrix:
        """Hessian in ``y = (z, tau)``."""
        return spa.block_diag([self.kappa * self.Sigma, spa.csc_matrix((self.n, self.n))], "csc")

    @property
    def q(self) -> np.ndarray:
        return np.concatenate([-self.mu, self.lambd * np.ones(self.n)])

    @property
    def blocks(self) -> dict[str, spa.csc_matrix]:
        """Constraint rows in the order budget, position, sell, buy."""
        n, eye = self.n, spa.eye(self.n)
        return {
            "budget": spa.hstack([np.ones((1, n)), spa.csc_matrix((1, n))], "csc"),
            "position": spa.hstack([eye, spa.csc_matrix((n, n))], "csc"),
            "sell": spa.hstack([eye, -eye], "csc"),
            "buy": spa.hstack([eye, eye], "csc"),
        }

    @property
    def M(self) -> spa.csc_matrix:
        return spa.vstack(list(self.blocks.values()), "csc")

    @property
    def lhs(self) -> spa.csc_matrix:
        M = self.M
        return (self.P + self.sigma * spa.eye(M.shape[1]) + self.rho * M.T @ M).tocsc()

    @property
    def H(self) -> spa.csc_matrix:
        """Map from ``s^k - s^{k-1}`` to the primal-dual residual, ``s = (y, v)``."""
        M = self.M
        m, d = M.shape
        return -spa.bmat([[self.sigma * spa.eye(d), self.rho * M.T], [None, spa.eye(m)]], "csc")


def make_problem(seed: int = SEED) -> PortfolioProblem:
    rng = np.random.default_rng(seed)
    F = rng.normal(size=(N, N))
    Sigma = F @ F.T / N + 0.1 * np.eye(N)
    mu = rng.uniform(0.0, 0.5, size=N)
    return PortfolioProblem(
        Sigma=Sigma,
        mu=mu,
        kappa=KAPPA,
        lambd=LAMBD,
        z_bar=Z_BAR,
        rho=RHO,
        sigma=SIGMA,
        K=K,
    )


def balanced_warm_start(problem: PortfolioProblem) -> np.ndarray:
    """ADMM state ``s^0 = (y, v)`` at the primal-dual solution for ``x = 1/n``.

    The problem is solved with CVXPY in the epigraph form above. With ``w = M y``
    and the dual ``dual`` of the rows ``l <= M y <= u`` (positive where the upper
    bound binds), the ADMM fixed point has ``v = w + dual / rho``, since ADMM
    recovers the dual as ``rho (v - w)``.
    """
    n = problem.n
    x = np.ones(n) / n
    z, tau = cp.Variable(n), cp.Variable(n)
    budget = cp.sum(z) == 1
    pos_lo, pos_hi = z >= 0, z <= problem.z_bar
    sell, buy = z - tau <= x, z + tau >= x
    objective = (
        problem.kappa / 2 * cp.quad_form(z, problem.Sigma)
        - problem.mu @ z
        + problem.lambd * cp.sum(tau)
    )
    cp.Problem(cp.Minimize(objective), [budget, pos_lo, pos_hi, sell, buy]).solve(
        solver="CLARABEL", tol_gap_abs=1e-12, tol_gap_rel=1e-12, tol_feas=1e-12
    )
    y = np.concatenate([z.value, tau.value])
    dual_rest = np.concatenate(
        [pos_hi.dual_value - pos_lo.dual_value, sell.dual_value, -buy.dual_value]
    )
    # Fix the sign of the budget multiplier through stationarity, P y + q + M' dual = 0,
    # rather than relying on CVXPY's convention for equality duals.
    nu = float(np.atleast_1d(budget.dual_value)[0])
    candidates = [np.concatenate([[sgn * nu], dual_rest]) for sgn in (1.0, -1.0)]
    dual = min(candidates, key=lambda d: np.abs(problem.P @ y + problem.q + problem.M.T @ d).max())
    assert np.abs(problem.P @ y + problem.q + problem.M.T @ dual).max() < 1e-7
    return np.concatenate([y, problem.M @ y + dual / problem.rho])


def warm_start(problem: PortfolioProblem, X: np.ndarray) -> np.ndarray:
    """Rows of the balanced warm start ``s^0``, one per row of ``X``."""
    return np.tile(balanced_warm_start(problem), (X.shape[0], 1))


def project(problem: PortfolioProblem, V: np.ndarray, X: np.ndarray) -> np.ndarray:
    """Project the rows of ``V`` onto ``[l(x), u(x)]`` for the matching rows of ``X``."""
    n = problem.n
    budget = np.ones((V.shape[0], 1))
    position = np.clip(V[:, 1 : 1 + n], 0.0, problem.z_bar)
    sell = np.minimum(V[:, 1 + n : 1 + 2 * n], X)
    buy = np.maximum(V[:, 1 + 2 * n :], X)
    return np.hstack([budget, position, sell, buy])


def admm_residuals(problem: PortfolioProblem, X: np.ndarray, K: int) -> np.ndarray:
    """Run ADMM on the rows of ``X``; return ``||H(s^k - s^{k-1})||∞`` with shape ``(len(X), K)``."""
    M, d = problem.M, 2 * problem.n
    lhs = spa.linalg.splu(problem.lhs)
    s = warm_start(problem, X)
    out = np.empty((X.shape[0], K))
    for k in range(K):
        y, v = s[:, :d], s[:, d:]
        w = project(problem, v, X)
        rhs = problem.sigma * y - problem.q + problem.rho * (2 * w - v) @ M
        y_next = lhs.solve(rhs.T).T
        v_next = v + y_next @ M.T - w
        s_next = np.hstack([y_next, v_next])
        out[:, k] = np.abs((s_next - s) @ problem.H.T).max(axis=1)
        s = s_next
    return out


def sample_simplex(problem: PortfolioProblem, size: int, rng: np.random.Generator) -> np.ndarray:
    """Uniform samples of the capped simplex, by rejection from the Dirichlet(1)."""
    out = np.empty((0, problem.n))
    while out.shape[0] < size:
        X = rng.dirichlet(np.ones(problem.n), size=2 * size)
        out = np.vstack([out, X[(X <= problem.z_bar).all(axis=1)]])
    return out[:size]


def sample_box(problem: PortfolioProblem, size: int, rng: np.random.Generator) -> np.ndarray:
    return rng.uniform(0.0, problem.z_bar, size=(size, problem.n))
