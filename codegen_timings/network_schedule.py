"""Scheduled vs unrolled code generation for Ruiz-equilibrated momentum PDHG.

    uv run python -m codegen_timings.network_schedule --check-only   # no agent; seconds
    FOMTOOLS_CODEGEN_AGENT=1 uv run python -m codegen_timings.network_schedule --force  # reps 1 2 3
    (rep 1 in ``network_schedule/<arm>/``, the others in ``<arm>/reps/rep<k>/``)

Momentum PDHG extrapolates with ``xi_t = t / (t + 3)``, so its iteration is not
stationary. fomtools can generate it two ways from the same method:

- **scheduled**: the recording is a function of ``xi``; extraction fits every
  coefficient block as ``M0 + xi M1`` and emits one loop body that reads ``xi``
  from a table at the loop counter.
- **unrolled**: the plain K-step loop the method is verified with, emitted as one
  straight-line body in which every iteration carries its own coefficient blocks.

Both solvers compute the same K iterations. Each arm runs the translation phase
and then the code optimization phase on a copy, under the same prompts and
budgets as the §6.6 table, so the comparison asks whether knowing the method is a
schedule lets the agent (and the compiler) produce faster code.

The instance is the network experiment's cluster instance (supply 4, warm start at
the box centre) at ``NS x ND`` nodes, larger than the verified 10 x 15, at the
verified horizon K = 50.
"""

from __future__ import annotations

import argparse
import json
import statistics
import time
from datetime import datetime, timezone
from pathlib import Path

import numpy as np

from codegen_timings.common import (
    AGENT_OPTIMIZE_TIMEOUT_S,
    AGENT_TIMEOUT_S,
    HERE,
    N_CHECK_SAMPLES,
    NUMPY_REPEATS,
    TIMING_ROUNDS,
    add_control_to_path,
    bench_us,
    load_adapter,
    machine,
    rep_dir,
)

NS, ND = 20, 30
K = 50
VARIANT = "ruiz_momentum"
ARMS = ["scheduled", "unrolled"]
#: The paper's instance, every problem setting spelled out so that the
#: verification defaults in conf/experiment/network.yaml (a different, smaller
#: instance) cannot move it: complete bipartite, 600 edges, eta = 0.0927.
OVERRIDES = [
    "experiment=network",
    f"experiment.problem.ns={NS}",
    f"experiment.problem.nd={ND}",
    "experiment.problem.edge_prob=1.0",
    "experiment.problem.seed=20",
    "experiment.problem.gain_spread=100.0",
    "experiment.problem.cost_range=[1.0,10.0]",
    "experiment.problem.capacity_range=[2.0,6.0]",
    "experiment.problem.supply=4",
    "experiment.problem.x_lb=-6.0",
    "experiment.problem.x_ub=-4.0",
    "experiment.problem.warm_start=center",
    "experiment.problem.ruiz_iters=30",
    "experiment.problem.eta_scale=0.5",
]
OUT = HERE / "network_schedule"


class Setup:
    """The instance, both recordings, and the NumPy reference, built once."""

    def __init__(self):
        from fomexp.run import compose_config

        self.cfg = compose_config(OVERRIDES)
        adapter = load_adapter("network")
        import problem as network_problem

        pcfg = self.cfg.experiment.problem
        self.problem = adapter.make_problem(pcfg, K)
        self.pv = adapter.make_variant(
            self.problem, VARIANT, int(pcfg.ruiz_iters), float(pcfg.eta_scale)
        )
        self._pdhg_run = network_problem.pdhg_run
        self.xi = np.array([self.pv.xi(t) for t in range(K)])
        p = self.problem
        self.samples = [
            {"p0": x}
            for x in np.random.default_rng(int(self.cfg.simulator.seed)).uniform(
                p.x_lb, p.x_ub, size=(int(self.cfg.simulator.n_samples), p.nd)
            )
        ]

    def record(self, v, xis):
        """Record ``len(xis)`` momentum PDHG steps; returns (param, initial state, iterates).

        The iterates live in trajectories, as in the verification, so the unrolled
        body stays linear in K (``w`` is an affine accumulation, not a step output).
        """
        p, pv = self.problem, self.pv
        E_d = np.diag(pv.e_d)
        x = v.parameter(p.nd, lb=p.x_lb, ub=p.x_ub)
        state = [v.iterate(len(a), lb=a, ub=a) for a in (pv.z0, pv.v0, pv.w0)]
        Z, Y, W = (v.iterate_trajectory(len(xis) + 1) for _ in range(3))
        Z[0], Y[0], W[0] = state
        iterates = []
        for k, xi in enumerate(xis):
            z, y, w = Z[k], Y[k], W[k]
            Z[k + 1] = v.satlin(
                z - pv.eta * (pv.mu + pv.A_s.T @ y - pv.A_d.T @ w), l=np.zeros(p.ne), u=pv.c
            )
            z_bar = (2.0 + 2.0 * xi) * Z[k + 1] - (1.0 + 2.0 * xi) * z
            Y[k + 1] = v.relu(y + pv.eta * (pv.A_s @ z_bar - pv.b_s))
            W[k + 1] = w + pv.eta * (E_d @ x - pv.A_d @ z_bar)
            iterates.append([Z[k + 1], Y[k + 1], W[k + 1]])
        return x, state, iterates

    def scheduled_recording(self):
        from fomtools.codegen.extract_scheduled import Recording as BuildRecording
        from fomtools.verifier import Verifier

        from common.recording import ScheduledRecording

        def build(xi: float) -> BuildRecording:
            v = Verifier(method="simulator", obbt_solver=None)
            x, state, iterates = self.record(v, [xi])
            return BuildRecording(verifier=v, next_state=iterates[0], state=state, params=[x])

        return ScheduledRecording(build=build, schedules={"xi": self.xi})

    def unrolled_recording(self):
        from fomtools.verifier import Verifier

        v = Verifier(method="simulator", obbt_solver=None)
        x, state, iterates = self.record(v, list(self.xi))
        return v, x, state, iterates

    def boxes(self):
        from common.recording import ParameterBox, ParameterBoxes

        p, pv = self.problem, self.pv
        s0 = np.concatenate([pv.z0, pv.v0, pv.w0])
        return ParameterBoxes(
            boxes=[ParameterBox("p0", p.x_lb, p.x_ub, 0.5 * (p.x_lb + p.x_ub))], state=s0
        )

    def numpy_solve(self, params: dict) -> np.ndarray:
        (z, y, w), _ = self._pdhg_run(self.pv, params["p0"][None, :], K, residuals=False)
        return np.concatenate([z[0], y[0], w[0]])


def codegen_config():
    from common import codegen_driver

    return codegen_driver.config_for(K).model_copy(
        update=dict(
            agent_timeout_s=AGENT_TIMEOUT_S, agent_optimize_timeout_s=AGENT_OPTIMIZE_TIMEOUT_S
        )
    )


def extract(setup: Setup, arm: str, config):
    """The IR of one arm, without an agent."""
    from fomtools.codegen.extract_unrolled import extract_unrolled_ir

    from common import codegen_driver

    name = f"network_{arm}"
    if arm == "scheduled":
        return codegen_driver.extract(setup.scheduled_recording(), setup.boxes(), config, name)
    v, x, state, iterates = setup.unrolled_recording()
    # roll=False: the method is known not to be stationary, so skip the roll attempt.
    return extract_unrolled_ir(
        v, iterates, config=config, name=name, state=state, params=[x], roll=False
    )


def ir_size(ir) -> dict:
    mats = ir.matrices.values() if isinstance(ir.matrices, dict) else ir.matrices
    mats = list(mats)
    return dict(
        unrolled=bool(ir.unrolled),
        body_steps=len(ir.body),
        coefficient_blocks=len(mats),
        # MatrixData may hold a sparse array: count from its shape and nnz().
        coefficient_entries_dense=int(sum(np.prod(m.shape) for m in mats)),
        coefficient_nonzeros=int(sum(m.nnz() for m in mats)),
        schedules=sorted(ir.schedules),
    )


def check(setup: Setup, ir) -> dict:
    from fomtools.codegen.oracle import ReferenceOracle

    oracle = ReferenceOracle(ir)
    errs = []
    for p in setup.samples[:5]:
        ref = setup.numpy_solve(p)
        out = oracle.solve(param_values=p).solution
        errs.append(float(np.max(np.abs(out - ref)) / max(1.0, float(np.max(np.abs(ref))))))
    return dict(oracle_vs_numpy_max_rel=max(errs), **ir_size(ir))


def translate(setup: Setup, arm: str, out_dir: Path, config, force: bool):
    from fomtools.codegen.generate import generate_code

    from common import codegen_driver

    name = f"network_{arm}"
    if arm == "scheduled":
        return codegen_driver.translate(
            setup.scheduled_recording(), setup.boxes(), out_dir, name, config=config, force=force
        )
    emitted = out_dir / "emitted"
    if not force:
        ir = extract(setup, arm, config)
        if codegen_driver._verifies(emitted, config, ir):
            print(f"reusing the verified solver already in {emitted}")
            return ir, emitted
    v, x, state, iterates = setup.unrolled_recording()
    result = generate_code(
        v,
        out_dir=emitted,
        config=config,
        driver=codegen_driver.default_driver(config),
        state=state,
        params=[x],
        name=name,
        code_optimization=False,
        iterates=iterates,
        roll=False,
    )
    if not result.translate.success:
        raise RuntimeError(
            f"the translation phase did not produce a verified solver in {emitted}.\n"
            f"{result.translate.log[-4000:]}"
        )
    return result.ir, emitted


def max_deviation(directory: Path, setup: Setup) -> float:
    from common.csolver import CSolver

    solver = CSolver(directory)
    return max(
        float(np.max(np.abs(solver.solve(iterations=K, **p) - setup.numpy_solve(p))))
        for p in setup.samples[:N_CHECK_SAMPLES]
    )


def numpy_us(setup: Setup, params: dict) -> float:
    for _ in range(3):
        setup.numpy_solve(params)
    times = []
    for _ in range(NUMPY_REPEATS):
        start = time.perf_counter()
        setup.numpy_solve(params)
        times.append((time.perf_counter() - start) * 1e6)
    return statistics.median(times)


def artifact_size(directory: Path) -> dict:
    solver = directory / "src" / "fom_solver.c"
    data = directory / "src" / "fom_data.c"
    lib = next(iter(directory.glob("libfomsolver.*")), None)
    return dict(
        solver_c_lines=len(solver.read_text().splitlines()) if solver.exists() else None,
        data_c_bytes=data.stat().st_size if data.exists() else None,
        library_bytes=lib.stat().st_size if lib else None,
    )


def run_arm(setup: Setup, arm: str, check_only: bool, force: bool, rep: int = 1) -> dict:
    from common import codegen_driver

    config = codegen_config()
    out_dir = rep_dir(OUT / arm, rep)
    out_dir.mkdir(parents=True, exist_ok=True)
    pre = check(setup, extract(setup, arm, config))
    print(f"[{arm}] pre-flight: {pre}", flush=True)
    (out_dir / "check.json").write_text(json.dumps(pre, indent=2, default=float) + "\n")
    if check_only:
        return pre

    started = datetime.now(timezone.utc).isoformat(timespec="seconds")
    t0 = time.time()
    ir, emitted = translate(setup, arm, out_dir, config, force)
    t_translate = time.time() - t0
    t0 = time.time()
    opt = codegen_driver.optimize(ir, emitted, out_dir, config=config, force=force)
    t_optimize = time.time() - t0

    deviation = {"translated": max_deviation(emitted, setup), "optimized": max_deviation(opt.path, setup)}
    params = setup.samples[0]
    rounds = {"numpy": [], "translated": [], "optimized": []}
    for _ in range(TIMING_ROUNDS):
        rounds["numpy"].append(numpy_us(setup, params))
        rounds["translated"].append(bench_us(emitted))
        rounds["optimized"].append(bench_us(opt.path))
    med = {k: statistics.median(v) for k, v in rounds.items()}

    result = dict(
        experiment="network_schedule",
        arm=arm,
        rep=rep,
        variant=VARIANT,
        iterations=K,
        overrides=OVERRIDES,
        numpy_us=med["numpy"],
        translated_us=med["translated"],
        optimized_us=med["optimized"],
        speedup_translation=med["numpy"] / med["translated"],
        speedup_optimization=med["translated"] / med["optimized"],
        speedup_total=med["numpy"] / med["optimized"],
        optimization_kept=bool(opt.kept),
        optimization_notes=opt.notes,
        optimization_rounds=opt.rounds,
        rule_friction=opt.rule_friction,
        max_abs_dev=deviation,
        timing_rounds=rounds,
        size=dict(
            ir=pre,
            translated=artifact_size(emitted),
            optimized=artifact_size(opt.path),
        ),
        agent_wall_s=dict(translation=t_translate, code_optimization=t_optimize),
        agent_budgets_s=dict(
            translation=AGENT_TIMEOUT_S, code_optimization=AGENT_OPTIMIZE_TIMEOUT_S
        ),
        instance=dict(ns=setup.problem.ns, nd=setup.problem.nd, ne=setup.problem.ne, eta=setup.pv.eta),
        machine=machine(),
        started_utc=started,
        finished_utc=datetime.now(timezone.utc).isoformat(timespec="seconds"),
    )
    (out_dir / "timing.json").write_text(json.dumps(result, indent=2, default=float) + "\n")
    print(
        f"[{arm}] NumPy {med['numpy']:.1f} us | translated {med['translated']:.2f} us | "
        f"optimized {med['optimized']:.2f} us (kept={opt.kept}) | "
        f"speedups {result['speedup_translation']:.1f}x, {result['speedup_optimization']:.2f}x, "
        f"{result['speedup_total']:.1f}x | max|C-NumPy| {deviation}",
        flush=True,
    )
    return result


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--arms", nargs="*", default=ARMS, choices=ARMS)
    parser.add_argument("--check-only", action="store_true")
    parser.add_argument("--force", action="store_true")
    parser.add_argument("--reps", nargs="+", type=int, default=[1, 2, 3])
    args = parser.parse_args()

    add_control_to_path()
    setup = Setup()
    for rep in [1] if args.check_only else args.reps:
        for arm in args.arms:
            run_arm(setup, arm, args.check_only, args.force, rep)


if __name__ == "__main__":
    main()
