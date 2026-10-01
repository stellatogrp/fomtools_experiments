"""Generalized min-cost network flow with gains, and its Ruiz-equilibrated variant.

The problem is

    minimize    μᵀz
    subject to  A_s z <= b_s,  A_d z = x,  0 <= z <= c,

on a complete bipartite graph from supply to demand nodes. One unit shipped on
edge ``e = (i, j)`` leaves supply node ``i`` and delivers ``g_e`` units at demand
node ``j``, so ``A_s[i, e] = 1`` and ``A_d[j, e] = -g_e``. The gains model losses
and conversions (yields, units, exchange rates) and spread over two orders of
magnitude, so ``[A_s; A_d]`` is badly scaled by construction. The demand ``x <= 0``
is the parameter, as in arXiv 2412.11330, Sec. 4.2.

Both variants run PDHG. The equilibrated one works in ``z = D ẑ``, ``v = E_s v̂``,
``w = E_d ŵ`` with ``E, D`` from two-sided Ruiz on ``[A_s; A_d]``. The parameter
itself is not scaled: it enters the iteration as ``E_d x``. Both variants report
the KKT residual of the original LP, so the bounds are comparable.
"""

from __future__ import annotations

from dataclasses import dataclass

import cvxpy as cp
import numpy as np

NS, ND = 3, 4
K = 10
SEED = 20
GAIN_SPREAD = 100.0
SUPPLY = 10.0
X_LB, X_UB = -6.0, -4.0


@dataclass(frozen=True)
class NetworkProblem:
    A_s: np.ndarray
    A_d: np.ndarray
    g: np.ndarray
    mu: np.ndarray
    c: np.ndarray
    b_s: np.ndarray
    x_lb: np.ndarray
    x_ub: np.ndarray
    z0: np.ndarray
    v0: np.ndarray
    w0: np.ndarray
    K: int

    @property
    def ns(self) -> int:
        return int(self.A_s.shape[0])

    @property
    def nd(self) -> int:
        return int(self.A_d.shape[0])

    @property
    def ne(self) -> int:
        return int(self.A_s.shape[1])


@dataclass(frozen=True)
class PDHGVariant:
    """PDHG (optionally with primal momentum) on the scaled LP.

    The original iterate is ``(z, v, w) = (D ẑ, E_s v̂, E_d ŵ)``. ``H(k)`` is the
    residual map of arXiv 2412.11330, eq. after (30), built from the scaled data
    and followed by ``blkdiag(D⁻¹, E_s⁻¹, E_d⁻¹)``, so ``H(k) (ŝ^k - ŝ^{k-1})`` is
    the KKT residual of the original LP in its own units.
    """

    name: str
    momentum: bool
    A_s: np.ndarray
    A_d: np.ndarray
    mu: np.ndarray
    c: np.ndarray
    b_s: np.ndarray
    e_s: np.ndarray
    e_d: np.ndarray
    d: np.ndarray
    eta: float
    z0: np.ndarray
    v0: np.ndarray
    w0: np.ndarray
    unscale_diag: np.ndarray

    def xi(self, k: int) -> float:
        """Momentum coefficient of step ``k -> k+1`` (Nesterov, ``k/(k+3)``); zero without momentum."""
        return k / (k + 3) if self.momentum else 0.0

    def H(self, k: int) -> np.ndarray:
        """Residual map for ``s^k - s^{k-1}``, ``k >= 1``; uses ``ξ_{k-1}``."""
        ne, ns, nd = self.A_s.shape[1], self.A_s.shape[0], self.A_d.shape[0]
        a = 1 + 2 * self.xi(k - 1)
        H_hat = -np.block(
            [
                [np.eye(ne) / self.eta, -self.A_s.T, self.A_d.T],
                [-a * self.A_s, np.eye(ns) / self.eta, np.zeros((ns, nd))],
                [a * self.A_d, np.zeros((nd, ns)), np.eye(nd) / self.eta],
            ]
        )
        return self.unscale_diag[:, None] * H_hat

    def unscale(self, z, v, w):
        return self.d * z, self.e_s * v, self.e_d * w


def solve_lp(problem: NetworkProblem, x: np.ndarray):
    """Primal-dual solution ``(z, v, w)`` in the PDHG sign convention, and the optimal value.

    The z-gradient of the Lagrangian is ``μ + A_sᵀv - A_dᵀw`` with ``v >= 0``.
    """
    return _solve_lp(problem.A_s, problem.A_d, problem.mu, problem.c, problem.b_s, x)


def _solve_lp(A_s, A_d, mu, c, b_s, x):
    z = cp.Variable(A_s.shape[1])
    supply, demand = A_s @ z <= b_s, A_d @ z == x
    prob = cp.Problem(cp.Minimize(mu @ z), [supply, demand, z >= 0, z <= c])
    prob.solve(solver=cp.CLARABEL)
    assert prob.status == cp.OPTIMAL, prob.status
    return z.value, supply.dual_value, -demand.dual_value, prob.value


def kkt_residual(problem: NetworkProblem, z, v, w, x) -> float:
    """Natural (projection) KKT residual of the original LP, in the ∞-norm."""
    grad = problem.mu + problem.A_s.T @ v - problem.A_d.T @ w
    return float(
        np.abs(
            np.concatenate(
                [
                    z - np.clip(z - grad, 0.0, problem.c),
                    v - np.maximum(v + problem.A_s @ z - problem.b_s, 0.0),
                    problem.A_d @ z - x,
                ]
            )
        ).max()
    )


def make_problem(seed: int = SEED) -> NetworkProblem:
    rng = np.random.default_rng(seed)
    edges = [(i, j) for i in range(NS) for j in range(ND)]
    ne = len(edges)
    half = np.log(GAIN_SPREAD) / 2
    g = np.exp(rng.uniform(-half, half, ne))
    A_s, A_d = np.zeros((NS, ne)), np.zeros((ND, ne))
    for e, (i, j) in enumerate(edges):
        A_s[i, e] = 1.0
        A_d[j, e] = -g[e]
    mu = rng.uniform(1.0, 10.0, ne)
    c = rng.uniform(2.0, 6.0, ne)
    b_s = np.full(NS, SUPPLY)
    x_lb, x_ub = np.full(ND, X_LB), np.full(ND, X_UB)

    # Warm start at the largest demand. Flows into demand node j serve only j,
    # so scaling them down keeps every smaller demand in the box feasible.
    z0, v0, w0, _ = _solve_lp(A_s, A_d, mu, c, b_s, x_lb)
    problem = NetworkProblem(A_s, A_d, g, mu, c, b_s, x_lb, x_ub, z0, v0, w0, K)
    assert kkt_residual(problem, z0, v0, w0, x_lb) < 1e-6
    return problem


def ruiz(M: np.ndarray, iters: int = 30) -> tuple[np.ndarray, np.ndarray]:
    """Two-sided Ruiz: ``(r, d)`` such that ``diag(r) M diag(d)`` has unit row and column inf-norms."""
    r, d = np.ones(M.shape[0]), np.ones(M.shape[1])
    for _ in range(iters):
        scaled = np.abs(r[:, None] * M * d[None, :])
        r = r / np.sqrt(scaled.max(axis=1))
        d = d / np.sqrt(scaled.max(axis=0))
    return r, d


def variant(problem: NetworkProblem, equilibrate: bool, momentum: bool = False) -> PDHGVariant:
    ns, nd, ne = problem.ns, problem.nd, problem.ne
    if equilibrate:
        r, d = ruiz(np.vstack([problem.A_s, problem.A_d]))
    else:
        r, d = np.ones(ns + nd), np.ones(ne)
    e_s, e_d = r[:ns], r[ns:]
    A_s = e_s[:, None] * problem.A_s * d
    A_d = e_d[:, None] * problem.A_d * d
    eta = 0.5 / float(np.linalg.norm(np.vstack([A_s, A_d]), 2))
    return PDHGVariant(
        name=("ruiz" if equilibrate else "unscaled") + ("_momentum" if momentum else ""),
        momentum=momentum,
        A_s=A_s,
        A_d=A_d,
        mu=d * problem.mu,
        c=problem.c / d,
        b_s=e_s * problem.b_s,
        e_s=e_s,
        e_d=e_d,
        d=d,
        eta=eta,
        z0=problem.z0 / d,
        v0=problem.v0 / e_s,
        w0=problem.w0 / e_d,
        unscale_diag=1.0 / np.concatenate([d, e_s, e_d]),
    )


def pdhg_run(v: PDHGVariant, X: np.ndarray, K: int, residuals: bool = True):
    """Run PDHG on the rows of ``X`` for ``K`` steps.

    Returns the final scaled iterates and, if requested, ``||H(ŝ^k - ŝ^{k-1})||∞``
    with shape ``(len(X), K)``.
    """
    n = X.shape[0]
    z, y, w = (np.tile(a, (n, 1)) for a in (v.z0, v.v0, v.w0))
    Xs = X * v.e_d
    out = np.empty((n, K)) if residuals else None
    for k in range(K):
        z_next = np.clip(z - v.eta * (v.mu + y @ v.A_s - w @ v.A_d), 0.0, v.c)
        z_tilde = z_next + v.xi(k) * (z_next - z)
        z_bar = 2 * z_tilde - z
        y_next = np.maximum(y + v.eta * (z_bar @ v.A_s.T - v.b_s), 0.0)
        w_next = w + v.eta * (Xs - z_bar @ v.A_d.T)
        if residuals:
            delta = np.hstack([z_next - z, y_next - y, w_next - w])
            out[:, k] = np.abs(delta @ v.H(k + 1).T).max(axis=1)
        z, y, w = z_next, y_next, w_next
    return (z, y, w), out


def pdhg_residuals(v: PDHGVariant, X: np.ndarray, K: int) -> np.ndarray:
    return pdhg_run(v, X, K)[1]
