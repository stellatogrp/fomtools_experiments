"""Portfolio rebalancing with an l1 trading cost, and its OSQP-style ADMM.

The problem is

    minimize    (gamma/2) z' Sigma z - mu' z + kappa ||z - x||_1
    subject to  1' z = 1,  0 <= z <= z_bar,

where the parameter ``x`` is the portfolio currently held. Last period's
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

ADMM is warm-started at the current holdings: ``z^0 = x``, ``tau^0 = 0`` and
``v^0 = M y^0``, which is zero dual.
"""

from __future__ import annotations

from dataclasses import dataclass

import numpy as np
import scipy.sparse as spa
import scipy.sparse.linalg  # noqa: F401

N = 3
K = 5
SEED = 0
GAMMA = 1.0
KAPPA = 0.05
Z_BAR = 0.6
RHO = 1.0
SIGMA = 1e-4


@dataclass(frozen=True)
class PortfolioProblem:
    Sigma: np.ndarray
    mu: np.ndarray
    gamma: float
    kappa: float
    z_bar: float
    rho: float
    sigma: float
    K: int

    @property
    def n(self) -> int:
        return int(self.mu.size)

    # Every matrix below is sparse. fomtools' ``stack`` adds the coefficients of
    # a leaf that appears in several components, and sparse + dense gives an
    # ``np.matrix`` that breaks its bound propagation.

    @property
    def P(self) -> spa.csc_matrix:
        """Hessian in ``y = (z, tau)``."""
        return spa.block_diag([self.gamma * self.Sigma, spa.csc_matrix((self.n, self.n))], "csc")

    @property
    def q(self) -> np.ndarray:
        return np.concatenate([-self.mu, self.kappa * np.ones(self.n)])

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
        gamma=GAMMA,
        kappa=KAPPA,
        z_bar=Z_BAR,
        rho=RHO,
        sigma=SIGMA,
        K=K,
    )


def warm_start(problem: PortfolioProblem, X: np.ndarray) -> np.ndarray:
    """Rows of ``s^0 = (x, 0, M (x, 0))`` for each row ``x`` of ``X``."""
    y0 = np.hstack([X, np.zeros_like(X)])
    return np.hstack([y0, y0 @ problem.M.T])


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
