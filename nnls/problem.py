"""Poorly column-scaled NNLS instance and its Ruiz-equilibrated variant.

The problem is ``minimize (1/2)||Az - x||² subject to z >= 0`` with the
measurement ``x`` as the parameter. ``A = A0 diag(c)`` is a well-conditioned
Gaussian matrix whose columns are rescaled over a range of ``SCALE_RATIO``, the
kind of ill-conditioning that diagonal equilibration removes.

Both variants run projected gradient descent. The equilibrated one works in
``z = D ẑ`` with ``D`` from symmetric Ruiz on ``AᵀA``; the parameter is not
scaled, since row scaling would change the NNLS solution. Both variants report
their residual in the original variables, so the bounds are comparable.
"""

from __future__ import annotations

from dataclasses import dataclass

import numpy as np

M, N = 5, 3
K = 5
SEED = 3
SCALE_RATIO = 30.0
X_LB, X_UB = 0.0, 1.0


@dataclass(frozen=True)
class NNLSProblem:
    A: np.ndarray
    x_lb: np.ndarray
    x_ub: np.ndarray
    z0: np.ndarray
    K: int

    @property
    def m(self) -> int:
        return int(self.A.shape[0])

    @property
    def n(self) -> int:
        return int(self.A.shape[1])


@dataclass(frozen=True)
class PGDVariant:
    """PGD on ``min (1/2)||Âs - x||², s >= 0`` with the unscaled residual map ``H``.

    The original iterate is ``z = D s``. ``H (s^k - s^{k-1})`` is the gradient
    mapping of the original problem, ``(1/t) I`` without scaling and
    ``(1/t) D⁻¹`` with it.
    """

    name: str
    A_hat: np.ndarray
    D: np.ndarray
    t: float
    s0: np.ndarray
    H: np.ndarray


def make_problem(seed: int = SEED, scale_ratio: float = SCALE_RATIO) -> NNLSProblem:
    rng = np.random.default_rng(seed)
    A0 = rng.normal(size=(M, N))
    half = np.log10(scale_ratio) / 2
    c = np.logspace(-half, half, N)
    return NNLSProblem(
        A=A0 * c,
        x_lb=np.full(M, X_LB),
        x_ub=np.full(M, X_UB),
        z0=np.zeros(N),
        K=K,
    )


def ruiz(P: np.ndarray, iters: int = 25) -> np.ndarray:
    """Symmetric Ruiz equilibration: ``d`` such that ``diag(d) P diag(d)`` has unit row inf-norms."""
    d = np.ones(P.shape[0])
    for _ in range(iters):
        scaled = d[:, None] * P * d[None, :]
        d = d / np.sqrt(np.abs(scaled).max(axis=1))
    return d


def variant(problem: NNLSProblem, equilibrate: bool) -> PGDVariant:
    d = ruiz(problem.A.T @ problem.A) if equilibrate else np.ones(problem.n)
    A_hat = problem.A * d
    t = 1.0 / float(np.linalg.eigvalsh(A_hat.T @ A_hat)[-1])
    return PGDVariant(
        name="ruiz" if equilibrate else "unscaled",
        A_hat=A_hat,
        D=np.diag(d),
        t=t,
        s0=problem.z0 / d,
        H=np.diag(1.0 / d) / t,
    )


def pgd_residuals(v: PGDVariant, X: np.ndarray, K: int) -> np.ndarray:
    """Run PGD on the rows of ``X``; return ``||H(s^k - s^{k-1})||∞`` with shape ``(len(X), K)``."""
    s = np.tile(v.s0, (X.shape[0], 1))
    out = np.empty((X.shape[0], K))
    for k in range(K):
        s_next = np.maximum(s - v.t * (s @ v.A_hat.T - X) @ v.A_hat, 0.0)
        out[:, k] = np.abs((s_next - s) @ v.H.T).max(axis=1)
        s = s_next
    return out
