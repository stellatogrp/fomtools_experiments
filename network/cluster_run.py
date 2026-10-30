"""Cluster adapter: PDHG / momentum PDHG on network flow with gains, unscaled or Ruiz.

The iteration is copied from ``verify.run`` and the data and variant builders
from ``problem.make_problem`` / ``problem.variant`` (which all stay as they
are), with every constant taken from the config. With the default config the
instance is the paper's.
"""

from __future__ import annotations

import fomtools as ft
import numpy as np

from fomexp.kloop import VariantSpec
from problem import NetworkProblem, PDHGVariant, _solve_lp, kkt_residual, pdhg_residuals, ruiz


def make_problem(cfg, K: int) -> NetworkProblem:
    """``problem.make_problem`` with every constant from ``cfg``, drawing in the same order."""
    ns, nd = int(cfg.ns), int(cfg.nd)
    rng = np.random.default_rng(int(cfg.seed))
    edge_prob = float(cfg.get("edge_prob", 1.0))
    if edge_prob >= 1.0:
        # Complete bipartite graph; draws nothing, so the data match problem.make_problem.
        edges = [(i, j) for i in range(ns) for j in range(nd)]
    else:
        # Random bipartite graph: each edge with probability edge_prob, then one random
        # edge added to any demand node with no supplier and any supply node with no edge.
        mask = rng.random((ns, nd)) < edge_prob
        for j in range(nd):
            if not mask[:, j].any():
                mask[rng.integers(ns), j] = True
        for i in range(ns):
            if not mask[i].any():
                mask[i, rng.integers(nd)] = True
        edges = [(i, j) for i in range(ns) for j in range(nd) if mask[i, j]]
    ne = len(edges)
    half = np.log(float(cfg.gain_spread)) / 2
    g = np.exp(rng.uniform(-half, half, ne))
    A_s, A_d = np.zeros((ns, ne)), np.zeros((nd, ne))
    for e, (i, j) in enumerate(edges):
        A_s[i, e] = 1.0
        A_d[j, e] = -g[e]
    mu = rng.uniform(*map(float, cfg.cost_range), ne)
    c = rng.uniform(*map(float, cfg.capacity_range), ne)
    b_s = np.full(ns, float(cfg.supply))
    x_lb, x_ub = np.full(nd, float(cfg.x_lb)), np.full(nd, float(cfg.x_ub))
    # Warm start at the largest demand (see problem.make_problem) or at the box centre.
    # Either way every demand in the box is feasible, since x_lb is (checked here).
    _solve_lp(A_s, A_d, mu, c, b_s, x_lb)
    x0 = {"largest": x_lb, "center": (x_lb + x_ub) / 2}[str(cfg.get("warm_start", "largest"))]
    z0, v0, w0, _ = _solve_lp(A_s, A_d, mu, c, b_s, x0)
    problem = NetworkProblem(A_s, A_d, g, mu, c, b_s, x_lb, x_ub, z0, v0, w0, K)
    assert kkt_residual(problem, z0, v0, w0, x0) < 1e-6
    return problem


def make_variant(
    problem: NetworkProblem, name: str, ruiz_iters: int, eta_scale: float
) -> PDHGVariant:
    """``problem.variant`` with the Ruiz passes and the step-size factor from the config."""
    equilibrate, momentum = name.startswith("ruiz"), name.endswith("_momentum")
    ns, nd, ne = problem.ns, problem.nd, problem.ne
    if equilibrate:
        r, d = ruiz(np.vstack([problem.A_s, problem.A_d]), iters=ruiz_iters)
    else:
        r, d = np.ones(ns + nd), np.ones(ne)
    e_s, e_d = r[:ns], r[ns:]
    A_s = e_s[:, None] * problem.A_s * d
    A_d = e_d[:, None] * problem.A_d * d
    eta = eta_scale / float(np.linalg.norm(np.vstack([A_s, A_d]), 2))
    return PDHGVariant(
        name=name,
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


def build(cfg, variant: str) -> VariantSpec:
    pcfg = cfg.experiment.problem
    problem = make_problem(pcfg, int(cfg.schedule.k_max))
    v = make_variant(problem, variant, int(pcfg.ruiz_iters), float(pcfg.eta_scale))
    ne, ns = problem.ne, problem.ns
    E_d = np.diag(v.e_d)

    def draw(size: int, seed: int) -> np.ndarray:
        return np.random.default_rng(seed).uniform(
            problem.x_lb, problem.x_ub, size=(size, problem.nd)
        )

    def setup(vp, mode, samples, length):
        if mode == "simulator":
            x = vp.parameter(problem.nd, samples=samples)
            init = [vp.iterate(len(a), init=a) for a in (v.z0, v.v0, v.w0)]
        else:
            x = vp.parameter(problem.nd, lb=problem.x_lb, ub=problem.x_ub)
            init = [vp.iterate(len(a), lb=a, ub=a) for a in (v.z0, v.v0, v.w0)]
        z, y, w = (vp.iterate_trajectory(length) for _ in range(3))
        z[0], y[0], w[0] = init

        def step(k: int) -> None:
            z[k] = vp.satlin(
                z[k - 1] - v.eta * (v.mu + v.A_s.T @ y[k - 1] - v.A_d.T @ w[k - 1]),
                l=np.zeros(ne),
                u=v.c,
            )
            z_bar = 2 * (z[k] + v.xi(k - 1) * (z[k] - z[k - 1])) - z[k - 1]
            y[k] = vp.relu(y[k - 1] + v.eta * (v.A_s @ z_bar - v.b_s))
            w[k] = w[k - 1] + v.eta * (E_d @ x - v.A_d @ z_bar)
            # H(k) unscales the residual, so every variant measures the original LP's KKT residual.
            H = v.H(k)
            H_z, H_v, H_w = H[:, :ne], H[:, ne : ne + ns], H[:, ne + ns :]
            vp.maximize(
                ft.norm(
                    H_z @ (z[k] - z[k - 1]) + H_v @ (y[k] - y[k - 1]) + H_w @ (w[k] - w[k - 1]),
                    np.inf,
                )
            )

        return x, step

    M = np.vstack([problem.A_s, problem.A_d])
    M_hat = np.vstack([v.A_s, v.A_d])
    info = dict(
        **{f"cfg_{k}": val for k, val in pcfg.items()},
        ne=problem.ne,
        eta=v.eta,
        norm_A=float(np.linalg.norm(M, 2)),
        norm_A_scaled=float(np.linalg.norm(M_hat, 2)),
        cond_A=float(np.linalg.cond(M)),
        cond_A_scaled=float(np.linalg.cond(M_hat)),
        gains=problem.g,
        costs=problem.mu,
        capacities=problem.c,
        z0=problem.z0,
        v0=problem.v0,
        w0=problem.w0,
        ruiz_row_scale=np.concatenate([v.e_s, v.e_d]),
        ruiz_col_scale=v.d,
    )
    return VariantSpec(
        experiment="network",
        variant=variant,
        setup=setup,
        sim_samples=draw(int(cfg.simulator.n_samples), int(cfg.simulator.seed)),
        start_samples=draw(int(cfg.schedule.n_start_samples), int(cfg.schedule.start_samples_seed)),
        info=info,
        reference=lambda X, K: pdhg_residuals(v, X, K),
    )
