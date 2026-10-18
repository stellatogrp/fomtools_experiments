"""OSQP's ADMM on the verified portfolio instance (the run in ``common.RUNS``).

The factor model of ``portfolio/problem.py`` at the verified size, with the
parameter ``p = (x, mu)``. Its box is the verified one (the drift box of ``x``
times the forecast box of ``mu``); the C-vs-NumPy check runs on samples of the
verified set (``1'x = 1``). The step is ``problem.admm_step``, with the balanced
warm start.
"""

from __future__ import annotations

import sys

import numpy as np

from codegen_timings.common import ITERATIONS, VariantSetup, load_adapter

VARIANTS = ["admm"]


def build(variant: str, cfg) -> VariantSetup:
    from fomtools.verifier import Verifier

    from common.recording import ParameterBox, ParameterBoxes, Recording

    adapter = load_adapter("portfolio")
    pcfg = cfg.experiment.problem
    problem = adapter.problem_from_cfg(pcfg)
    problem_mod = sys.modules["problem"]
    admm_run, admm_step = problem_mod.admm_run, problem_mod.admm_step
    n = problem.n
    s0 = adapter.balanced_warm_start(problem)
    lb, ub = problem.p_lb, problem.p_ub
    boxes = ParameterBoxes(boxes=[ParameterBox("p0", lb, ub, 0.5 * (lb + ub))], state=s0)

    v = Verifier(method="simulator", obbt_solver=None)
    p = v.add_param(problem.p_dim, lb=lb, ub=ub)
    s_init = v.add_iterate(s0.size, lb=s0, ub=s0)
    traj = v.iterate_trajectory(4)
    traj[0] = s_init
    for k in range(1, 4):
        traj[k] = admm_step(v, problem, p, traj[k - 1])
    iterates = [traj[k] for k in range(1, 4)]
    recording = Recording(
        verifier=v, next_state=iterates[0], state=[s_init], params=[p], iterates=iterates
    )

    def numpy_solve(params):
        return admm_run(problem, params["p0"][None, :], ITERATIONS, s0)[0][0]

    spec = adapter.build(cfg, "simplex")
    samples = [{"p0": pv} for pv in spec.sim_samples]
    # Same recurrence as the verification's NumPy reference (admm_residuals), one
    # sample at a time.
    X = spec.sim_samples[:5]
    batch = admm_run(problem, X, 50, s0)[1]
    single = np.vstack([admm_run(problem, pv[None, :], 50, s0)[1] for pv in X])
    checks = {"numpy_vs_verify_residuals_simplex": float(np.max(np.abs(single - batch)))}
    info = dict(n=n, k=problem.k, mu_model=str(pcfg.get("mu_model", "box")), state_dim=int(s0.size))
    return VariantSetup(recording, boxes, numpy_solve, samples, checks=checks, info=info)
