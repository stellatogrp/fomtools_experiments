"""ISTA and FISTA on the verified Lasso instance (lasso/lasso_verify run_022).

The data, the parameter box, the warm start and the parameter samples all come
from ``lasso/lasso_verify/cluster_run.py`` applied to that run's config, so the
generated solver runs the iteration that was verified. ISTA is stationary and
recorded once; FISTA carries ``(u_prev, u)`` and reads its momentum from a table
with the verification's offset: the step that produces ``u_{t+1}`` extrapolates
with ``gamma_{t-1}`` (0 for t = 0), i.e. 0, 0, gamma_1, gamma_2, ...
"""

from __future__ import annotations

import numpy as np

from codegen_timings.common import ITERATIONS, VariantSetup, load_adapter

VARIANTS = ["ista", "fista"]


def build(variant: str, cfg) -> VariantSetup:
    from fomtools.codegen.extract_scheduled import Recording as BuildRecording
    from fomtools.verifier import Verifier

    from common.recording import ParameterBox, ParameterBoxes, Recording, ScheduledRecording

    adapter = load_adapter("lasso")
    problem = adapter.make_problem(cfg.experiment.problem, ITERATIONS)
    D, eta, s0 = problem.D, problem.eta, problem.s0
    thr = problem.lambd * eta
    lb, ub = problem.x_lb, problem.x_ub
    boxes = ParameterBoxes(boxes=[ParameterBox("p0", lb, ub, 0.5 * (lb + ub))], state=s0)

    # The verification's FISTA coefficients: gammas = momentum(K), used one step late.
    gam = np.zeros(ITERATIONS)
    gam[1:] = adapter.momentum(ITERATIONS)[: ITERATIONS - 1]

    def prox_grad(w, x):
        v = w - eta * D.T @ (D @ w - x)
        return np.sign(v) * np.maximum(np.abs(v) - thr, 0.0)

    if variant == "ista":
        v = Verifier(method="simulator", obbt_solver=None)
        x = v.parameter(problem.p, lb=lb, ub=ub)
        u0 = v.iterate(problem.n, lb=s0, ub=s0)
        u, iterates = u0, []
        for _ in range(3):
            u = v.soft_threshold(u - eta * D.T @ (D @ u - x), lambd=thr)
            iterates.append(u)
        recording = Recording(
            verifier=v, next_state=iterates[0], state=[u0], params=[x], iterates=iterates
        )

        def numpy_solve(p):
            u = s0.copy()
            for _ in range(ITERATIONS):
                u = prox_grad(u, p["p0"])
            return u

    else:

        def build_fn(gamma: float) -> BuildRecording:
            v = Verifier(method="simulator", obbt_solver=None)
            x = v.parameter(problem.p, lb=lb, ub=ub)
            prev = v.iterate(problem.n, lb=s0, ub=s0)
            cur = v.iterate(problem.n, lb=s0, ub=s0)
            w = (1.0 + gamma) * cur - gamma * prev
            nxt = v.soft_threshold(w - eta * D.T @ (D @ w - x), lambd=thr)
            return BuildRecording(verifier=v, next_state=[cur, nxt], state=[prev, cur], params=[x])

        recording = ScheduledRecording(build=build_fn, schedules={"gamma": gam})

        def numpy_solve(p):
            prev = cur = s0.copy()
            for t in range(ITERATIONS):
                w = cur + gam[t] * (cur - prev)
                prev, cur = cur, prox_grad(w, p["p0"])
            return np.concatenate([prev, cur])

    # The verification loop itself (cluster_run's step), as a cross-check of the recurrence.
    gammas = adapter.momentum(ITERATIONS) if variant == "fista" else np.zeros(ITERATIONS)

    def verify_recurrence(xv):
        u = w = s0.copy()
        for k in range(1, ITERATIONS + 1):
            u_next = prox_grad(w, xv)
            w = u_next + gammas[k - 1] * (u_next - u)
            u = u_next
        return u

    spec = adapter.build(cfg, variant)
    samples = [{"p0": xv} for xv in spec.sim_samples]
    final = slice(-problem.n, None)
    checks = dict(
        numpy_vs_verify_recurrence=max(
            float(np.max(np.abs(numpy_solve(p)[final] - verify_recurrence(p["p0"]))))
            for p in samples[:5]
        )
    )
    info = dict(m=problem.p, n=problem.n, lambd=problem.lambd, eta=eta, method=variant)
    return VariantSetup(recording, boxes, numpy_solve, samples, checks=checks, info=info)
