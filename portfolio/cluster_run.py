"""Cluster adapter: portfolio rebalancing ADMM, box or capped simplex.

The iteration is copied from ``verify.run`` (which stays as it is); the data are
built from the config instead of ``problem.py``'s module constants. With the
default config the instance is the paper's (``problem.make_problem()``).

With ``experiment.problem.drift = r`` the held portfolio ``x`` is no longer any
point of ``[0, z_bar]^n`` (box) or of the capped simplex, but last period's
equal-weight portfolio after one period of returns in ``[-r, r]``:

    x_i = c_i (1 + r_i) / sum_j c_j (1 + r_j),   c = 1/n,  |r_j| <= r.

Each coordinate of that set ranges over the exact interval ``drift_interval``
gives (``x_i`` is largest when asset i gains ``r`` and every other asset loses
``r``). "box" is the product of those intervals; "simplex" adds ``1'x = 1``. Both
contain every drifted portfolio, and ADMM's warm start is the solution at
``x = c``, the centre of both.
"""

from __future__ import annotations

import numpy as np
import scipy.sparse as spa
from fomtools.linearexpression import stack

from fomexp.kloop import VariantSpec
from problem import (
    PortfolioProblem,
    admm_residuals,
    balanced_warm_start,
    sample_box,
    sample_simplex,
)


def make_problem(cfg, K: int) -> PortfolioProblem:
    """``problem.make_problem`` with every constant from ``cfg``."""
    n = int(cfg.n)
    rng = np.random.default_rng(int(cfg.seed))
    F = rng.normal(size=(n, n))
    Sigma = F @ F.T / n + float(cfg.sigma_ridge) * np.eye(n)
    mu = rng.uniform(0.0, float(cfg.mu_high), size=n)
    return PortfolioProblem(
        Sigma=Sigma,
        mu=mu,
        kappa=float(cfg.kappa),
        lambd=float(cfg.lambd),
        z_bar=float(cfg.z_bar) * np.ones(n),
        rho=float(cfg.rho),
        sigma=float(cfg.sigma),
        K=K,
    )


def drift_interval(c: np.ndarray, r: float) -> tuple[np.ndarray, np.ndarray]:
    """Exact per-asset range of ``c * (1 + r_i) / sum_j c_j (1 + r_j)`` over ``|r_j| <= r``."""
    up, down = c * (1 + r), (1 - c) * (1 - r)
    lo_num, lo_rest = c * (1 - r), (1 - c) * (1 + r)
    return lo_num / (lo_num + lo_rest), up / (up + down)


def sample_interval_box(lb, ub, size: int, rng: np.random.Generator) -> np.ndarray:
    return rng.uniform(lb, ub, size=(size, lb.size))


def sample_interval_simplex(lb, ub, size: int, rng: np.random.Generator) -> np.ndarray:
    """Uniform samples of ``{x : 1'x = 1, lb <= x <= ub}``.

    Draw the first ``n - 1`` coordinates uniformly from their intervals and set
    the last to ``1 - sum``; keep the draws whose last coordinate lands in its
    interval. The map to the first ``n - 1`` coordinates is linear and
    one-to-one on the hyperplane, so the kept points are uniform on the
    polytope. For intervals of equal width the acceptance rate is about
    ``1 / sqrt(n - 1)`` (0.4 at n = 10), and every batch is vectorized.
    """
    n, out, kept = lb.size, [], 0
    while kept < size:
        batch = max(2 * (size - kept), 1024)
        head = rng.uniform(lb[:-1], ub[:-1], size=(batch, n - 1))
        last = 1.0 - head.sum(axis=1)
        ok = (last >= lb[-1]) & (last <= ub[-1])
        out.append(np.column_stack([head[ok], last[ok]]))
        kept += int(ok.sum())
    return np.vstack(out)[:size]


def build(cfg, variant: str) -> VariantSpec:
    pcfg = cfg.experiment.problem
    problem = make_problem(pcfg, int(cfg.schedule.k_max))
    n, rho, sigma = problem.n, problem.rho, problem.sigma
    B = problem.blocks
    s0_val = balanced_warm_start(problem)
    drift = pcfg.get("drift")
    if drift is None:
        x_lb, x_ub = np.zeros(n), problem.z_bar
        sampler = {"box": sample_box, "simplex": sample_simplex}[variant]
    else:
        x_lb, x_ub = drift_interval(np.full(n, 1.0 / n), float(drift))
        x_lb, x_ub = np.maximum(x_lb, 0.0), np.minimum(x_ub, problem.z_bar)
        draw = {"box": sample_interval_box, "simplex": sample_interval_simplex}[variant]

        def sampler(_problem, size, rng):
            return draw(x_lb, x_ub, size, rng)

    sim_samples = sampler(
        problem, int(cfg.simulator.n_samples), np.random.default_rng(int(cfg.simulator.seed))
    )
    start_samples = sampler(
        problem,
        int(cfg.schedule.n_start_samples),
        np.random.default_rng(int(cfg.schedule.start_samples_seed)),
    )

    def setup(vp, mode, samples, length):
        if mode == "simulator":
            x = vp.add_param(n, samples=samples)
            s0 = vp.add_iterate(s0_val.size, init=s0_val)
        else:
            x = vp.add_param(n, lb=x_lb, ub=x_ub)
            if variant == "simplex":
                vp.add_constraint(spa.csc_matrix(np.ones((1, n))) @ x == 1.0)
            s0 = vp.add_iterate(s0_val.size, lb=s0_val, ub=s0_val)
        s = vp.iterate_trajectory(length)
        s[0] = s0
        d = 2 * n

        def step(k: int) -> None:
            y, v = s[k - 1][:d], s[k - 1][d:]
            v_budget, v_pos = v[:1], v[1 : 1 + n]
            v_sell, v_buy = v[1 + n : 1 + 2 * n], v[1 + 2 * n :]
            w_pos = vp.step.satlin(v_pos, l=np.zeros(n), u=problem.z_bar)
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
            vp.set_objective_norm([problem.H @ (s[k] - s[k - 1])], ord=np.inf)

        return x, step

    info = dict(
        **{f"cfg_{k}": v for k, v in cfg.experiment.problem.items()},
        n=n,
        state_dim=int(s0_val.size),
        norm_H=float(np.linalg.norm(problem.H.toarray(), 2)),
        lambda_max_Sigma=float(np.linalg.eigvalsh(problem.Sigma)[-1]),
        lambda_min_Sigma=float(np.linalg.eigvalsh(problem.Sigma)[0]),
        mu=problem.mu,
        Sigma=problem.Sigma,
        s0=s0_val,
        x_lb=x_lb,
        x_ub=x_ub,
    )
    return VariantSpec(
        experiment="portfolio",
        variant=variant,
        setup=setup,
        sim_samples=sim_samples,
        start_samples=start_samples,
        info=info,
        reference=lambda X, K: admm_residuals(problem, X, K),
    )
