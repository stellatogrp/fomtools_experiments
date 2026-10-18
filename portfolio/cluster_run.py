"""Cluster adapter: OSQP's ADMM on portfolio rebalancing over the capped simplex.

The instance is ``problem.make_problem(n, factors)``, the CVXPYgen comparison's
factor model (``portfolio_cvxpygen/model.py``) at the size in the config. The
parameter is ``p = (x, theta)``: the held portfolio ``x``, last period's
equal-weight portfolio after one period of returns in ``[-DRIFT, DRIFT]``,

    x_i = c_i (1 + r_i) / sum_j c_j (1 + r_j),   c = 1/n,  |r_j| <= DRIFT,

and the forecast ``mu = mu_bar + G theta`` of ``problem.MU_MODELS`` (config
``mu_model``, ``s_max``). Each coordinate of ``x`` ranges over the
exact interval ``drift_interval`` gives, and ``1'x = 1``, one ``add_constraint``
call. The set contains every drifted portfolio, and ADMM's warm start is the
solution at ``x = 1/n``, ``mu = mu_bar``, a point of it. The ``neutral`` model adds
``1' theta = 0``, a second ``add_constraint`` call.
"""

from __future__ import annotations

import numpy as np
import scipy.sparse as spa

from fomexp.kloop import VariantSpec
from portfolio_cvxpygen.model import DRIFT, IC, PROBLEM, S_MAX
from problem import (
    PortfolioProblem,
    admm_residuals,
    admm_step,
    balanced_warm_start,
    make_problem,
    sample_simplex,
)


def problem_from_cfg(cfg) -> PortfolioProblem:
    s_max = cfg.get("s_max")
    return make_problem(
        int(cfg.n),
        int(cfg.factors),
        str(cfg.get("mu_model", "box")),
        S_MAX if s_max is None else float(s_max),
    )


def build(cfg, variant: str) -> VariantSpec:
    problem = problem_from_cfg(cfg.experiment.problem)
    n = problem.n
    s0_val = balanced_warm_start(problem)
    if variant != "simplex":
        raise ValueError(f"unknown variant {variant!r}")
    sampler = sample_simplex
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
            p = vp.add_param(problem.p_dim, samples=samples)
            s0 = vp.add_iterate(s0_val.size, init=s0_val)
        else:
            p = vp.add_param(problem.p_dim, lb=problem.p_lb, ub=problem.p_ub)
            m = problem.p_dim - n
            budget = np.concatenate([np.ones(n), np.zeros(m)])
            vp.add_constraint(spa.csc_matrix(budget[None, :]) @ p == 1.0)
            if problem.neutral:
                neutral = np.concatenate([np.zeros(n), np.ones(m)])
                vp.add_constraint(spa.csc_matrix(neutral[None, :]) @ p == 0.0)
            s0 = vp.add_iterate(s0_val.size, lb=s0_val, ub=s0_val)
        s = vp.iterate_trajectory(length)
        s[0] = s0

        def step(k: int) -> None:
            s[k] = admm_step(vp, problem, p, s[k - 1])
            vp.set_objective_norm([problem.H @ (s[k] - s[k - 1])], ord=np.inf)

        return p, step

    Sigma = problem.F @ problem.F.T + problem.d * np.eye(n)
    info = dict(
        **{f"cfg_{k}": v for k, v in cfg.experiment.problem.items()},
        n=n,
        k=problem.k,
        **{f"model_{k}": v for k, v in PROBLEM.items()},
        drift=DRIFT,
        ic=IC,
        s_max=S_MAX,
        rho=problem.rho,
        sigma=problem.sigma,
        alpha=problem.alpha,
        rho_eq_scale=problem.rho_eq_scale,
        state_dim=int(s0_val.size),
        norm_H=float(np.linalg.norm(problem.H.toarray(), 2)),
        lambda_max_Sigma=float(np.linalg.eigvalsh(Sigma)[-1]),
        lambda_min_Sigma=float(np.linalg.eigvalsh(Sigma)[0]),
        F=problem.F,
        mu_bar=problem.mu_bar,
        s0=s0_val,
        x_lb=problem.x_lb,
        x_ub=problem.x_ub,
        G=problem.G,
        theta_lb=problem.th_lb,
        theta_ub=problem.th_ub,
        neutral=problem.neutral,
    )
    return VariantSpec(
        experiment="portfolio",
        variant=variant,
        setup=setup,
        sim_samples=sim_samples,
        start_samples=start_samples,
        info=info,
        reference=lambda P, K: admm_residuals(problem, P, K),
    )
