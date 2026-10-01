"""Small nonstrongly convex Lasso instance for verifying ISTA and FISTA.

The problem is ``minimize (1/2)||Du - x||² + lambda ||u||₁`` with the
measurement ``x`` as the parameter. ``D`` is wide (``P < N``), so ``DᵀD`` is
singular and the problem is not strongly convex. The data follow the sparse
coding setup of Ranjan et al. (arXiv 2412.11330, Example 2.4 and Sec. 4.1),
with a dense ``D`` for now.

Both methods report the scaled fixed-point residual ``H(s^K - s^{K-1})`` from
Sec. 4.1 of that paper, which equals the Lasso KKT residual at ``u^K``:

    H_K = -(DᵀD - η⁻¹ I) [I, -γ_{K-2} I],   s^k = (u^k, u^{k-1}),

for FISTA, and ``H = -(DᵀD - η⁻¹ I)`` on ``s^k = u^k`` for ISTA (γ ≡ 0).
"""

from __future__ import annotations

from dataclasses import dataclass

import numpy as np

P, N = 3, 5
K = 10
SEED = 0
LAMBD = 0.1
N_STAR = 100
P_STAR = 0.1
METHODS = ("ista", "fista")


@dataclass(frozen=True)
class LassoProblem:
    D: np.ndarray
    lambd: float
    eta: float
    x_lb: np.ndarray
    x_ub: np.ndarray
    s0: np.ndarray
    K: int

    @property
    def p(self) -> int:
        return int(self.D.shape[0])

    @property
    def n(self) -> int:
        return int(self.D.shape[1])

    @property
    def G(self) -> np.ndarray:
        """``DᵀD - η⁻¹ I``, the KKT scaling shared by both methods."""
        return self.D.T @ self.D - np.eye(self.n) / self.eta


def make_D(rng: np.random.Generator) -> np.ndarray:
    """Gaussian ``N(0, 1/P)`` entries with unit-norm columns."""
    D = rng.normal(scale=np.sqrt(1.0 / P), size=(P, N))
    return D / np.linalg.norm(D, axis=0)


def make_problem(seed: int = SEED) -> LassoProblem:
    rng = np.random.default_rng(seed)
    D = make_D(rng)
    z_star = rng.normal(size=(N, N_STAR)) * rng.binomial(1, P_STAR, size=(N, N_STAR))
    x_samples = (D @ z_star).T
    x_lb, x_ub = x_samples.min(axis=0), x_samples.max(axis=0)
    assert (x_ub - x_lb > 0).all(), "degenerate parameter box"
    # Warm start at the unregularized (min-norm least-squares) solution of one sample.
    s0 = np.linalg.pinv(D) @ x_samples[rng.integers(N_STAR)]
    return LassoProblem(
        D=D,
        lambd=LAMBD,
        eta=1.0 / float(np.linalg.eigvalsh(D.T @ D)[-1]),
        x_lb=x_lb,
        x_ub=x_ub,
        s0=s0,
        K=K,
    )


def momentum(K: int) -> np.ndarray:
    """FISTA coefficients ``γ_k = (β_k - 1)/β_{k+1}`` for ``k = 0..K-1``, with ``β_0 = 1``."""
    betas = [1.0]
    for _ in range(K):
        betas.append((1.0 + np.sqrt(1.0 + 4.0 * betas[-1] ** 2)) / 2.0)
    return np.array([(betas[k] - 1.0) / betas[k + 1] for k in range(K)])


def residual_map(problem: LassoProblem, method: str, k: int) -> np.ndarray:
    """``H`` such that ``H(s^k - s^{k-1})`` is the KKT residual at ``u^k``."""
    if method == "ista":
        return -problem.G
    # u^k = T(w^{k-1}) with w^{k-1} = u^{k-1} + γ_{k-2}(u^{k-1} - u^{k-2}); w^0 = u^0.
    gamma = momentum(problem.K)[k - 2] if k >= 2 else 0.0
    return -problem.G @ np.hstack([np.eye(problem.n), -gamma * np.eye(problem.n)])


def soft_threshold(v: np.ndarray, threshold: float) -> np.ndarray:
    return np.sign(v) * np.maximum(np.abs(v) - threshold, 0.0)


def prox_grad(problem: LassoProblem, W: np.ndarray, X: np.ndarray) -> np.ndarray:
    """One ISTA step from each row of ``W`` for the matching row of ``X``."""
    D, eta = problem.D, problem.eta
    return soft_threshold(W - eta * (W @ D.T - X) @ D, problem.lambd * eta)


def residuals(
    problem: LassoProblem, method: str, X: np.ndarray, K: int, ord: float = np.inf
) -> np.ndarray:
    """Run ``method`` on the rows of ``X``; return ``||H(s^k - s^{k-1})||`` with shape ``(len(X), K)``."""
    gammas = momentum(K) if method == "fista" else np.zeros(K)
    u_prev = u = np.tile(problem.s0, (X.shape[0], 1))
    w = u
    out = np.empty((X.shape[0], K))
    for k in range(1, K + 1):
        u_next = prox_grad(problem, w, X)
        if method == "ista":
            ds = u_next - u
        else:
            ds = np.hstack([u_next - u, u - u_prev])
        out[:, k - 1] = np.linalg.norm(ds @ residual_map(problem, method, k).T, ord=ord, axis=1)
        w = u_next + gammas[k - 1] * (u_next - u)
        u_prev, u = u, u_next
    return out
