"""Cluster adapter: ISTA or FISTA on the Lasso, infinity-norm KKT residual (the MILP study).

The iteration is copied from ``verify.run`` (which stays as it is); the data are
built from the config instead of ``problem.py``'s module constants. With the
default config the instance is the paper's (``problem.make_problem()``).
"""

from __future__ import annotations

import fomtools as ft
import numpy as np

from fomexp.kloop import VariantSpec
from problem import LassoProblem, make_D, momentum, residual_map, residuals


def make_problem(cfg, K: int) -> LassoProblem:
    """``problem.make_problem`` with every constant from ``cfg``, drawing in the same order."""
    m, n, n_star = int(cfg.m), int(cfg.n), int(cfg.n_star)
    rng = np.random.default_rng(int(cfg.seed))
    # Runs from before the density option (run_001, run_002) have no key and used a dense D.
    D = make_D(rng, m, n, float(cfg.get("density", 1.0)))
    # signal_scale multiplies every sparse signal, so it scales the measurement box
    # about its centre by the same factor; the draws themselves are unchanged.
    scale = float(cfg.get("signal_scale", 1.0))
    z_star = (
        scale * rng.normal(size=(n, n_star)) * rng.binomial(1, float(cfg.p_star), size=(n, n_star))
    )
    x_samples = (D @ z_star).T
    x_lb, x_ub = x_samples.min(axis=0), x_samples.max(axis=0)
    assert (x_ub - x_lb > 0).all(), "degenerate parameter box; raise n_star or p_star"
    # Warm start at the unregularized (min-norm least-squares) solution of one sample.
    s0 = np.linalg.pinv(D) @ x_samples[rng.integers(n_star)]
    return LassoProblem(
        D=D,
        lambd=float(cfg.lambd),
        eta=1.0 / float(np.linalg.eigvalsh(D.T @ D)[-1]),
        x_lb=x_lb,
        x_ub=x_ub,
        s0=s0,
        K=K,
    )


def build(cfg, variant: str) -> VariantSpec:
    K_max = max(int(cfg.schedule.k_max), int(cfg.simulator.k_max))
    problem = make_problem(cfg.experiment.problem, K_max)
    D, eta = problem.D, problem.eta
    gammas = momentum(K_max) if variant == "fista" else np.zeros(K_max)

    def draw(size: int, seed: int) -> np.ndarray:
        return np.random.default_rng(seed).uniform(
            problem.x_lb, problem.x_ub, size=(size, problem.p)
        )

    def setup(vp, mode, samples, length):
        if mode == "simulator":
            x = vp.parameter(problem.p, samples=samples)
            u0 = vp.iterate(problem.n, init=problem.s0)
        else:
            x = vp.parameter(problem.p, lb=problem.x_lb, ub=problem.x_ub)
            u0 = vp.iterate(problem.n, lb=problem.s0, ub=problem.s0)
        u = vp.iterate_trajectory(length)
        u[0] = u0
        state = {"w": u[0]}

        def step(k: int) -> None:
            w = state["w"]
            u[k] = vp.soft_threshold(w - eta * D.T @ (D @ w - x), lambd=problem.lambd * eta)
            if variant == "ista":
                ds = u[k] - u[k - 1]
            else:
                # s^k = (u^k, u^{k-1}), with u^{-1} = u^0.
                ds = ft.stack([u[k] - u[k - 1], u[k - 1] - u[max(k - 2, 0)]])
            vp.maximize(ft.norm([residual_map(problem, variant, k) @ ds], np.inf))
            state["w"] = u[k] + gammas[k - 1] * (u[k] - u[k - 1])

        return x, step

    info = dict(
        **{f"cfg_{k}": v for k, v in cfg.experiment.problem.items()},
        eta=eta,
        lambda_max_DtD=1.0 / eta,
        x_lb=problem.x_lb,
        x_ub=problem.x_ub,
        s0=problem.s0,
        D=D,
    )
    return VariantSpec(
        experiment="lasso",
        variant=variant,
        setup=setup,
        sim_samples=draw(int(cfg.simulator.n_samples), int(cfg.simulator.seed)),
        start_samples=draw(int(cfg.schedule.n_start_samples), int(cfg.schedule.start_samples_seed)),
        info=info,
        reference=lambda X, K: residuals(problem, variant, X, K),
    )
