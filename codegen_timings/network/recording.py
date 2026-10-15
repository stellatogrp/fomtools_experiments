"""PDHG and momentum PDHG, unscaled and Ruiz, on the verified network (run_001: 10 x 15).

The data, scaling, warm start and samples come from ``network/cluster_run.py``.
The plain variants are stationary. The momentum variants extrapolate with
``xi = t / (t + 3)`` at loop counter t (``PDHGVariant.xi(k - 1)`` at step
k = t + 1), which enters affinely, so it is a schedule table; the extrapolation
only mixes the new and old ``z``, so the state stays ``(z, v, w)``.
"""

from __future__ import annotations

import sys

import numpy as np

from codegen_timings.common import ITERATIONS, VariantSetup, load_adapter

VARIANTS = ["unscaled", "ruiz", "unscaled_momentum", "ruiz_momentum"]


def build(variant: str, cfg) -> VariantSetup:
    from fomtools.codegen.extract_scheduled import Recording as BuildRecording
    from fomtools.verifier import Verifier

    from common.recording import ParameterBox, ParameterBoxes, Recording, ScheduledRecording

    adapter = load_adapter("network")
    pcfg = cfg.experiment.problem
    problem = adapter.make_problem(pcfg, ITERATIONS)
    pv = adapter.make_variant(problem, variant, int(pcfg.ruiz_iters), float(pcfg.eta_scale))
    pdhg_run = sys.modules["problem"].pdhg_run
    ne = problem.ne
    E_d = np.diag(pv.e_d)
    lb, ub = problem.x_lb, problem.x_ub
    s0 = np.concatenate([pv.z0, pv.v0, pv.w0])
    boxes = ParameterBoxes(boxes=[ParameterBox("p0", lb, ub, 0.5 * (lb + ub))], state=s0)

    def record(v, xi):
        x = v.add_param(problem.nd, lb=lb, ub=ub)
        z, y, w = (v.add_iterate(len(a), lb=a, ub=a) for a in (pv.z0, pv.v0, pv.w0))

        def step(z, y, w):
            z1 = v.step.satlin(
                z - pv.eta * (pv.mu + pv.A_s.T @ y - pv.A_d.T @ w), l=np.zeros(ne), u=pv.c
            )
            z_bar = (2.0 + 2.0 * xi) * z1 - (1.0 + 2.0 * xi) * z
            y1 = v.step.relu(y + pv.eta * (pv.A_s @ z_bar - pv.b_s))
            w1 = w + pv.eta * (E_d @ x - pv.A_d @ z_bar)
            return z1, y1, w1

        return x, (z, y, w), step

    if pv.momentum:
        xi_table = np.array([t / (t + 3.0) for t in range(ITERATIONS)])

        def build_fn(xi: float) -> BuildRecording:
            v = Verifier(method="simulator", obbt_solver=None)
            x, state, step = record(v, xi)
            return BuildRecording(
                verifier=v, next_state=list(step(*state)), state=list(state), params=[x]
            )

        recording = ScheduledRecording(build=build_fn, schedules={"xi": xi_table})
    else:
        v = Verifier(method="simulator", obbt_solver=None)
        x, state, step = record(v, 0.0)
        iterates, cur = [], state
        for _ in range(3):
            cur = step(*cur)
            iterates.append(list(cur))
        recording = Recording(
            verifier=v,
            next_state=iterates[0],
            state=list(state),
            params=[x],
            iterates=iterates,
        )

    def numpy_solve(p):
        (z, y, w), _ = pdhg_run(pv, p["p0"][None, :], ITERATIONS, residuals=False)
        return np.concatenate([z[0], y[0], w[0]])

    spec = adapter.build(cfg, variant)
    samples = [{"p0": xv} for xv in spec.sim_samples]
    info = dict(
        ns=problem.ns,
        nd=problem.nd,
        ne=ne,
        eta=pv.eta,
        momentum=pv.momentum,
        ruiz=variant.startswith("ruiz"),
        state_dim=int(s0.size),
    )
    return VariantSetup(recording, boxes, numpy_solve, samples, info=info)
