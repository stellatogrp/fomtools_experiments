"""rAPDHG (restart 12 and 6) and plain PDHG on the verified quadcopter program (run_001: horizon 5).

The program is ``control/rapdhg/verify/cluster_run.small_problem`` at the run's
config; the recording is the codegen form in ``control/rapdhg/codegen/recording.py``
(restart mux, materialized primal step) with coefficient tables built from the
verification's own ``step_coefficients`` and ``restart_period``, so PDHG gets
constant tables and rAPDHG the paper's schedule.

The solver's parameters are its natural inputs, the cost ``q`` and right-hand
side ``h``. The verified parameter is the 3-dim perturbation ``p`` with
``q = Qp p`` and ``h = h0 + Hp p``; the boxes on ``q`` and ``h`` are the exact
interval image of the verified box on ``p``, and every check uses verified ``p``
samples mapped through those maps.
"""

from __future__ import annotations

import numpy as np

from codegen_timings.common import ITERATIONS, VariantSetup, load_adapter

VARIANTS = ["R12", "R6", "pdhg"]


def tables(problem, method: str, n_iters: int) -> dict[str, np.ndarray]:
    """``rapdhg.schedule_tables`` with the verification's coefficient functions."""
    from codegen.rapdhg import SCHEDULES
    from verify import verify as rv

    K = rv.restart_period(problem, method)
    rows = {name: np.empty(n_iters) for name in SCHEDULES}
    for t in range(n_iters):
        k, kn = t % K, (t + 1) % K
        binv, theta, eta, tau = rv.step_coefficients(problem, k, method)
        binv_next = rv.step_coefficients(problem, kn, method)[0]
        r = 1.0 if k == K - 1 else 0.0
        rows["taux"][t] = tau * (1.0 + theta)
        rows["tth"][t] = tau * theta
        rows["tau"][t] = tau
        rows["eta"][t] = eta
        g1, g2 = 1.0 - r + r * binv, r - r * binv
        d1, d2 = 1.0 - binv, binv
        rows["g1"][t], rows["g2"][t] = g1, g2
        rows["d1"][t], rows["d2"][t] = d1, d2
        rows["m1"][t] = (1.0 - binv_next) * d1 + binv_next * g2
        rows["m2"][t] = (1.0 - binv_next) * d2 + binv_next * g1
    return rows


def build(variant: str, cfg) -> VariantSetup:
    adapter = load_adapter("rapdhg")  # puts control/rapdhg on the path

    from codegen import rapdhg as rap
    from codegen.recording import make_build
    from common.recording import ParameterBox, ParameterBoxes, ScheduledRecording
    from verify import verify as rv

    pcfg = cfg.experiment.problem
    method_cfg = cfg.experiment.methods[variant]
    method, restart = str(method_cfg.method), int(method_cfg.restart)
    assert not pcfg.ruiz and float(pcfg.step_scale) == 1.0, "only the verified setting is supported"
    problem = adapter.small_problem(pcfg, restart)
    coords = tuple(int(c) for c in pcfg.param_coords)
    Qp, h0, Hp = adapter.affine_data(problem, coords)
    scaled = rv.equilibrated(problem, np.ones(problem.n), np.ones(problem.m))
    tabs = tables(scaled, method, ITERATIONS)
    checks = {}
    if method == "rapdhg":
        ref = rap.schedule_tables(problem, ITERATIONS)
        checks["tables_vs_rapdhg_schedule_tables"] = max(
            float(np.max(np.abs(tabs[k] - ref[k]))) for k in tabs
        )

    spec = adapter.build(cfg, variant)
    p_ub = np.abs(spec.info["param_ub"])
    q_rad, h_rad = np.abs(Qp) @ p_ub, np.abs(Hp) @ p_ub
    p_mid = np.zeros(len(coords))
    boxes = ParameterBoxes(
        boxes=[
            ParameterBox("p0", -q_rad, q_rad, Qp @ p_mid),
            ParameterBox("p1", h0 - h_rad, h0 + h_rad, h0 + Hp @ p_mid),
        ],
        state=np.zeros(problem.n),
    )
    recording = ScheduledRecording(build=make_build(problem, boxes), schedules=tabs)

    def numpy_solve(p):
        return rap.solve(problem, p["p0"], ITERATIONS, tables=tabs, h=p["p1"]).state

    samples = [{"p0": Qp @ pv, "p1": h0 + Hp @ pv} for pv in spec.sim_samples]

    # The verification loop (verify.numpy_residuals' recurrence), final x after K steps.
    def verify_recurrence(pv):
        Q, G, R = scaled.P, scaled.G, rv.restart_period(problem, method)
        c, h = Qp @ pv, h0 + Hp @ pv
        x = x_prev = xbar = np.zeros(problem.n)
        y = ybar = np.zeros(problem.m)
        for t in range(ITERATIONS):
            k = t % R
            binv, theta, eta, tau = rv.step_coefficients(scaled, k, method)
            xmd = (1.0 - binv) * xbar + binv * x
            y_new = np.maximum(y + tau * (G @ (x + theta * (x - x_prev)) - h), 0.0)
            x_new = x - eta * (Q @ xmd + c + G.T @ y_new)
            xbar = (1.0 - binv) * xbar + binv * x_new
            ybar = (1.0 - binv) * ybar + binv * y_new
            if k == R - 1:
                x, y, x_prev = xbar, ybar, xbar
            else:
                x_prev, x, y = x, x_new, y_new
        return x

    checks["numpy_x_vs_verify_recurrence"] = max(
        float(np.max(np.abs(numpy_solve(s)[: problem.n] - verify_recurrence(pv))))
        for s, pv in zip(samples[:5], spec.sim_samples[:5])
    )
    info = dict(
        method=method,
        restart=restart,
        horizon=int(pcfg.horizon),
        n=problem.n,
        m=problem.m,
        param_coords=list(coords),
        state_dim=int(4 * problem.n + 2 * problem.m),
    )
    return VariantSetup(recording, boxes, numpy_solve, samples, checks=checks, info=info)
