"""Fixed Lasso instance and NumPy references used by every experiment arm."""

from __future__ import annotations

from dataclasses import dataclass

import numpy as np


@dataclass(frozen=True)
class LassoProblem:
    """Numerical data for ``0.5 ||Az-b||² + lambda ||z||₁``."""

    A: np.ndarray
    b: np.ndarray
    state_map: np.ndarray
    param_map: np.ndarray
    initial: np.ndarray
    threshold: float
    step_size: float
    lambd: float
    seed: int

    @property
    def m(self) -> int:
        return int(self.A.shape[0])

    @property
    def n(self) -> int:
        return int(self.A.shape[1])


def build_problem(
    *, n: int = 80, m: int = 40, seed: int = 0, lambd: float = 0.02
) -> LassoProblem:
    """Build the exact seeded dense instance used for the reported table."""
    rng = np.random.default_rng(seed)
    A = rng.normal(size=(m, n))
    b = rng.normal(size=m)
    gram = A.T @ A
    step_size = 1.0 / float(np.linalg.eigvalsh(gram).max())
    return LassoProblem(
        A=A,
        b=b,
        # Preserve the original experiment's evaluation order exactly:
        # matrix multiplication and multiplication have equal precedence, so
        # ``step_size * A.T @ A`` is evaluated as ``(step_size * A.T) @ A``.
        state_map=np.eye(n) - step_size * A.T @ A,
        param_map=step_size * A.T,
        initial=np.full(n, 2.0),
        threshold=lambd * step_size,
        step_size=step_size,
        lambd=lambd,
        seed=seed,
    )


def soft_threshold(x: np.ndarray, threshold: float) -> np.ndarray:
    return np.sign(x) * np.maximum(np.abs(x) - threshold, 0.0)


def solve_ista(problem: LassoProblem, *, iterations: int = 200) -> np.ndarray:
    """Run the same fixed-count ISTA loop used by the generated solver."""
    z = problem.initial.copy()
    param_term = problem.param_map @ problem.b
    for _ in range(iterations):
        z = soft_threshold(problem.state_map @ z + param_term, problem.threshold)
    return z


def solve_fista(problem: LassoProblem, *, iterations: int = 200) -> np.ndarray:
    """Run textbook FISTA and return its two-slot generated-code state."""
    previous = problem.initial.copy()
    current = problem.initial.copy()
    beta = 1.0
    param_term = problem.param_map @ problem.b
    for _ in range(iterations):
        beta_next = (1.0 + np.sqrt(1.0 + 4.0 * beta * beta)) / 2.0
        gamma = (beta - 1.0) / beta_next
        extrapolated = current + gamma * (current - previous)
        following = soft_threshold(
            problem.state_map @ extrapolated + param_term,
            problem.threshold,
        )
        previous, current, beta = current, following, beta_next
    return np.concatenate([previous, current])
