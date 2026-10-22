"""Cluster adapter: rAPDHG (two restart periods) or plain PDHG on the quadcopter MPC.

The iteration is copied from ``verify.run`` and the program from
``verify.small_problem`` / ``verify.affine_data`` (which all stay as they are),
with every constant from the config; the step-size and scaling helpers are
imported from ``verify.verify``. With the default config this is the paper's
setup: horizon 5, restart periods 12 and 6 against PDHG, cold start at zero.
"""

from __future__ import annotations

import dataclasses
import sys
from pathlib import Path

import fomtools as ft
import numpy as np
from scipy.linalg import svdvals

# ``verify`` is a package run from control/rapdhg (``python -m verify.verify``), so
# that folder, not this one, must be on the path; its __init__ adds control/.
_HERE = Path(__file__).resolve().parent
sys.path[:] = [p for p in sys.path if Path(p).resolve() != _HERE]
sys.path.insert(0, str(_HERE.parent))

from codegen.mpc import RapdhgProblem, build_mpc  # noqa: E402
from common.quadcopter import TILT_LIMIT  # noqa: E402
from fomexp.kloop import VariantSpec  # noqa: E402
from verify import verify as rv  # noqa: E402


def small_problem(cfg, restart: int) -> RapdhgProblem:
    """``verify.small_problem`` with the time step, block and weight from ``cfg``."""
    p = build_mpc(
        dt=float(cfg.dt),
        lookahead=int(cfg.horizon) * float(cfg.dt),
        block=int(cfg.block),
        restart=restart,
        primal_weight=float(cfg.primal_weight),
        tilt_limit=TILT_LIMIT,
    )
    keep = np.abs(p.C).sum(axis=1) > 0
    C, F, e = p.C[keep], p.F[keep], p.e[keep]
    identity = np.eye(p.n)
    G = np.vstack([identity, -identity, C, -C])
    return dataclasses.replace(
        p, C=C, F=F, e=e, G=G, h=np.concatenate([p.hi, -p.lo, e, e]), norm_g=float(svdvals(G)[0])
    )


def affine_data(problem: RapdhgProblem, coords):
    """``verify.affine_data`` for the parameter coordinates ``coords``."""
    S = np.zeros((problem.A.shape[0], len(coords)))
    for j, i in enumerate(coords):
        S[i, j] = 1.0
    Qp = problem.Gx @ S
    FS = problem.F @ S
    h0 = np.concatenate([problem.hi, -problem.lo, problem.e, problem.e])
    Hp = np.vstack([np.zeros((2 * problem.n, S.shape[1])), -FS, FS])
    return Qp, h0, Hp


def build(cfg, variant: str) -> VariantSpec:
    pcfg = cfg.experiment.problem
    method_cfg = cfg.experiment.methods[variant]
    method, restart = str(method_cfg.method), int(method_cfg.restart)
    problem = small_problem(pcfg, restart)
    coords = tuple(int(c) for c in pcfg.param_coords)
    tilt = np.deg2rad(float(pcfg.tilt_range_deg))
    param_ub = np.array([tilt, tilt, float(pcfg.altitude_range)])
    assert len(coords) == len(param_ub), "param_coords must be (tilt, tilt, altitude)"
    param_lb = -param_ub

    n, m, R = problem.n, problem.m, rv.restart_period(problem, method)
    Qp, h0, Hp = affine_data(problem, coords)
    if pcfg.ruiz:
        d, e = rv.ruiz_kkt(problem.P, problem.G, iters=int(pcfg.ruiz_iters))
    else:
        d, e = np.ones(n), np.ones(m)
    scaled = rv.equilibrated(problem, d, e)
    Q, G = scaled.P, scaled.G
    unscale_d, unscale_p = np.diag(1.0 / d), np.diag(1.0 / e)
    step_scale = float(pcfg.step_scale)

    def draw(size: int, seed: int) -> np.ndarray:
        return np.random.default_rng(seed).uniform(param_lb, param_ub, size=(size, len(coords)))

    def setup(vp, mode, samples, length):
        if mode == "simulator":
            p = vp.parameter(len(coords), samples=samples)
            x0 = vp.iterate(n, init=np.zeros(n))
            y0 = vp.iterate(m, init=np.zeros(m))
        else:
            p = vp.parameter(len(coords), lb=param_lb, ub=param_ub)
            x0 = vp.iterate(n, lb=0.0, ub=0.0)
            y0 = vp.iterate(m, lb=0.0, ub=0.0)
        q = (d[:, None] * Qp) @ p
        h = e * h0 + (e[:, None] * Hp) @ p
        st = dict(x=x0, y=y0, x_prev=x0, xbar=x0, ybar=y0)

        def step(K: int) -> None:
            k = (K - 1) % R
            x, y, x_prev, xbar, ybar = st["x"], st["y"], st["x_prev"], st["xbar"], st["ybar"]
            binv, theta, eta, tau = rv.step_coefficients(scaled, k, method)
            eta, tau = step_scale * eta, step_scale * tau
            xmd = (1.0 - binv) * xbar + binv * x
            y_new = vp.relu(y + tau * (G @ ((1.0 + theta) * x - theta * x_prev) - h))
            x_new = x - eta * (Q @ xmd + q + G.T @ y_new)
            xbar = (1.0 - binv) * xbar + binv * x_new
            ybar = (1.0 - binv) * ybar + binv * y_new
            # Scaled residual of the inner step (see verify.py's docstring).
            r_d = (1.0 / eta) * (x - x_new) - Q @ (xmd - x_new)
            r_p = (1.0 / tau) * (y - y_new) - G @ ((x_new - x) - theta * (x - x_prev))
            if k == R - 1:
                # Restart: the next inner loop starts from the running average.
                x, y, x_prev = xbar, ybar, xbar
            else:
                x_prev, x, y = x, x_new, y_new
            st.update(x=x, y=y, x_prev=x_prev, xbar=xbar, ybar=ybar)
            vp.maximize(ft.norm([unscale_d @ r_d, unscale_p @ r_p], np.inf))

        return p, step

    reference = None
    if coords == tuple(rv.PARAM_COORDS):
        # verify.numpy_residuals reads its parameter coordinates from the module.
        def reference(X, K):
            return rv.numpy_residuals(
                problem, X, K, ruiz=bool(pcfg.ruiz), step_scale=step_scale, method=method
            )

    info = dict(
        **{f"cfg_{k}": v for k, v in pcfg.items() if k != "param_coords"},
        param_coords=" ".join(map(str, coords)),
        method=method,
        restart=restart,
        n=n,
        m=m,
        norm_q=problem.norm_q,
        norm_g=problem.norm_g,
        norm_q_scaled=scaled.norm_q,
        norm_g_scaled=scaled.norm_g,
        tilt_limit=float(TILT_LIMIT),
        param_lb=param_lb,
        param_ub=param_ub,
        ruiz_d=d,
        ruiz_e=e,
    )
    return VariantSpec(
        experiment="rapdhg",
        variant=variant,
        setup=setup,
        sim_samples=draw(int(cfg.simulator.n_samples), int(cfg.simulator.seed)),
        start_samples=draw(int(cfg.schedule.n_start_samples), int(cfg.schedule.start_samples_seed)),
        info=info,
        reference=reference,
    )
