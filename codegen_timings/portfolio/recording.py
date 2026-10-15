"""OSQP-style ADMM on the verified portfolio instance (portfolio run_002: n = 10, drift 0.15).

Box and simplex verify the *same* iteration over two parameter sets, so there is
one solver, ``admm``. Its parameter box is the verified drift box; the C-vs-NumPy
check runs on samples from both verified sets (box and capped-simplex drift).
The step is ``portfolio/cluster_run.py``'s, with the balanced warm start.
"""

from __future__ import annotations

import sys

import numpy as np
import scipy.sparse as spa
import scipy.sparse.linalg  # noqa: F401
from fomtools.linearexpression import stack

from codegen_timings.common import ITERATIONS, VariantSetup, load_adapter

VARIANTS = ["admm"]


def build(variant: str, cfg) -> VariantSetup:
    from fomtools.verifier import Verifier

    from common.recording import ParameterBox, ParameterBoxes, Recording

    adapter = load_adapter("portfolio")
    pcfg = cfg.experiment.problem
    problem = adapter.make_problem(pcfg, ITERATIONS)
    project = sys.modules["problem"].project
    n, rho, sigma = problem.n, problem.rho, problem.sigma
    B = problem.blocks
    s0 = adapter.balanced_warm_start(problem)
    lb, ub = adapter.drift_interval(np.full(n, 1.0 / n), float(pcfg.drift))
    lb, ub = np.maximum(lb, 0.0), np.minimum(ub, problem.z_bar)
    boxes = ParameterBoxes(boxes=[ParameterBox("p0", lb, ub, 0.5 * (lb + ub))], state=s0)

    v = Verifier(method="simulator", obbt_solver=None)
    x = v.add_param(n, lb=lb, ub=ub)
    s_init = v.add_iterate(s0.size, lb=s0, ub=s0)
    d = 2 * n

    def step(s):
        y, vv = s[:d], s[d:]
        v_budget, v_pos = vv[:1], vv[1 : 1 + n]
        v_sell, v_buy = vv[1 + n : 1 + 2 * n], vv[1 + 2 * n :]
        w_pos = v.step.satlin(v_pos, l=np.zeros(n), u=problem.z_bar)
        w_sell = x - v.step.relu(x - v_sell)  # min(v, x)
        w_buy = x + v.step.relu(v_buy - x)  # max(v, x)
        rhs = (
            sigma * y
            - problem.q
            + rho * B["budget"].T @ (2.0 - v_budget)
            + rho * B["position"].T @ (2 * w_pos - v_pos)
            + rho * B["sell"].T @ (2 * w_sell - v_sell)
            + rho * B["buy"].T @ (2 * w_buy - v_buy)
        )
        y_next = v.step.implicit_linear(rhs, output_mat=problem.lhs)
        v_next = stack(
            [
                v_budget + B["budget"] @ y_next - 1.0,
                v_pos + B["position"] @ y_next - w_pos,
                v_sell + B["sell"] @ y_next - w_sell,
                v_buy + B["buy"] @ y_next - w_buy,
            ]
        )
        return stack([y_next, v_next])

    traj = v.iterate_trajectory(4)
    traj[0] = s_init
    for k in range(1, 4):
        traj[k] = step(traj[k - 1])
    iterates = [traj[k] for k in range(1, 4)]
    recording = Recording(
        verifier=v, next_state=iterates[0], state=[s_init], params=[x], iterates=iterates
    )

    M = problem.M
    lu = spa.linalg.splu(problem.lhs)

    def numpy_solve(p):
        xv = p["p0"]
        s = s0.copy()
        for _ in range(ITERATIONS):
            y, vv = s[:d], s[d:]
            w = project(problem, vv[None, :], xv[None, :])[0]
            rhs = sigma * y - problem.q + rho * (M.T @ (2 * w - vv))
            y_next = lu.solve(rhs)
            s = np.concatenate([y_next, vv + M @ y_next - w])
        return s

    box_spec = adapter.build(cfg, "box")
    simplex_spec = adapter.build(cfg, "simplex")
    samples = [{"p0": xv} for xv in box_spec.sim_samples]
    simplex = [{"p0": xv} for xv in simplex_spec.sim_samples]
    # Same recurrence as the verification's NumPy reference (admm_residuals).
    admm_residuals = sys.modules["problem"].admm_residuals
    K = 50
    checks = {}
    for label, smp in (("box", samples), ("simplex", simplex)):
        X = np.array([p["p0"] for p in smp[:5]])
        ref = admm_residuals(problem, X, K)
        mine = []
        for xv in X:
            s = s0.copy()
            res = []
            for _ in range(K):
                s_prev = s
                y, vv = s[:d], s[d:]
                w = project(problem, vv[None, :], xv[None, :])[0]
                y_next = lu.solve(sigma * y - problem.q + rho * (M.T @ (2 * w - vv)))
                s = np.concatenate([y_next, vv + M @ y_next - w])
                res.append(np.abs(problem.H @ (s - s_prev)).max())
            mine.append(res)
        checks[f"numpy_vs_verify_residuals_{label}"] = float(np.max(np.abs(np.array(mine) - ref)))
    info = dict(n=n, drift=float(pcfg.drift), z_bar=float(pcfg.z_bar), state_dim=int(s0.size))
    return VariantSetup(
        recording,
        boxes,
        numpy_solve,
        samples,
        extra_samples={"simplex_set": simplex},
        checks=checks,
        info=info,
    )
