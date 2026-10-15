"""Shared driver: record, translate, optimize, check, and time one variant.

Each experiment's ``<exp>/recording.py`` returns a :class:`VariantSetup` built
from the *verified* instance (the ``config.yaml`` of the verification run named
in :data:`RUNS`). This module runs the two agent phases through
``control/common/codegen_driver.py`` (so the translated solver in ``emitted/``
and the code-optimized one in ``optimized/`` both survive), checks both against
a direct NumPy loop on parameters drawn from the verified set, and times the
three at :data:`ITERATIONS` iterations per solve.
"""

from __future__ import annotations

import importlib.util
import json
import platform
import statistics
import subprocess
import sys
import time
from collections.abc import Callable
from dataclasses import dataclass, field
from datetime import datetime, timezone
from pathlib import Path

import numpy as np
from omegaconf import OmegaConf

REPO = Path(__file__).resolve().parents[1]
HERE = Path(__file__).resolve().parent

#: Iterations per solve, as in the §6.6 table caption.
ITERATIONS = 200

#: Agent budgets: 10 minutes per translation attempt, 10 minutes for code optimization.
AGENT_TIMEOUT_S = 600
AGENT_OPTIMIZE_TIMEOUT_S = 600

#: experiment -> (experiment folder, verification run whose instance is used)
RUNS = {
    "lasso": ("lasso/lasso_verify", "results/run_002"),
    "portfolio": ("portfolio", "results/run_002"),
    "network": ("network", "results/run_001"),
    "rapdhg": ("control/rapdhg/verify", "results/run_001"),
}

#: Parameter draws (from the verified set) used for the C-vs-NumPy check.
N_CHECK_SAMPLES = 20
#: Interleaved timing rounds; the median per arm is reported.
TIMING_ROUNDS = 7
NUMPY_REPEATS = 50


@dataclass
class VariantSetup:
    """Everything needed to generate and evaluate one variant's solver."""

    recording: object  # common.recording.Recording or ScheduledRecording
    boxes: object  # common.recording.ParameterBoxes
    numpy_solve: Callable[[dict], np.ndarray]  # {p0: ..} -> final state, emitted layout
    samples: list[dict]  # parameter draws from the verified set, keyed p0, p1, ...
    extra_samples: dict[str, list[dict]] = field(default_factory=dict)  # e.g. simplex
    checks: dict = field(default_factory=dict)  # recording-level checks, filled by build
    info: dict = field(default_factory=dict)


def add_control_to_path() -> None:
    """``control/`` holds the shared ``common`` package (codegen driver, CSolver)."""
    control = str(REPO / "control")
    if control not in sys.path:
        sys.path.append(control)


def load_config(experiment: str):
    folder, run = RUNS[experiment]
    return OmegaConf.load(REPO / folder / run / "config.yaml")


def load_adapter(experiment: str):
    """Import ``<experiment folder>/cluster_run.py`` with its folder first on the path."""
    folder = REPO / RUNS[experiment][0]
    sys.path.insert(0, str(folder))
    spec = importlib.util.spec_from_file_location("cluster_run", folder / "cluster_run.py")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def load_recording_module(experiment: str):
    path = HERE / experiment / "recording.py"
    spec = importlib.util.spec_from_file_location(f"codegen_timings_{experiment}", path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def codegen_config():
    from common import codegen_driver

    return codegen_driver.config_for(ITERATIONS).model_copy(
        update=dict(
            agent_timeout_s=AGENT_TIMEOUT_S, agent_optimize_timeout_s=AGENT_OPTIMIZE_TIMEOUT_S
        )
    )


def _scaled_err(a: np.ndarray, b: np.ndarray) -> float:
    return float(np.max(np.abs(a - b)) / max(1.0, float(np.max(np.abs(b)))))


def check_oracle(setup: VariantSetup, name: str, config) -> dict:
    """fomtools' reference oracle on the extracted IR against the NumPy loop."""
    from fomtools.codegen.oracle import ReferenceOracle

    from common import codegen_driver

    ir = codegen_driver.extract(setup.recording, setup.boxes, config=config, name=name)
    oracle = ReferenceOracle(ir)
    errs = [
        _scaled_err(oracle.solve(param_values=p).solution, setup.numpy_solve(p))
        for p in setup.samples[:5]
    ]
    return dict(
        oracle_vs_numpy_max_rel=max(errs),
        state_dim=int(np.asarray(ir.state.init).size),
        params={p.name: int(np.asarray(p.lb).size) for p in ir.params},
    )


def max_deviation(directory: Path, setup: VariantSetup) -> dict:
    """Max |C - NumPy| of the solver in ``directory`` at ITERATIONS, per sample set."""
    from common.csolver import CSolver

    solver = CSolver(directory)
    out = {}
    for label, samples in {"verified_set": setup.samples, **setup.extra_samples}.items():
        worst = 0.0
        for p in samples[:N_CHECK_SAMPLES]:
            worst = max(
                worst,
                float(
                    np.max(np.abs(solver.solve(iterations=ITERATIONS, **p) - setup.numpy_solve(p)))
                ),
            )
        out[label] = worst
    return out


def numpy_us(setup: VariantSetup, params: dict) -> float:
    for _ in range(3):
        setup.numpy_solve(params)
    times = []
    for _ in range(NUMPY_REPEATS):
        start = time.perf_counter()
        setup.numpy_solve(params)
        times.append((time.perf_counter() - start) * 1e6)
    return statistics.median(times)


def bench_us(directory: Path) -> float:
    from fomtools.codegen.harness import run_bench

    return float(run_bench(directory).mean_us)


def machine() -> dict:
    def sh(*cmd):
        try:
            return subprocess.run(cmd, capture_output=True, text=True).stdout.strip()
        except OSError:
            return ""

    import fomtools

    return dict(
        cpu=sh("sysctl", "-n", "machdep.cpu.brand_string") or platform.processor(),
        os=platform.platform(),
        compiler=sh("cc", "--version").splitlines()[0] if sh("cc", "--version") else "",
        python=platform.python_version(),
        numpy=np.__version__,
        fomtools_commit=sh(
            "git", "-C", str(REPO.parent / "fomtools"), "rev-parse", "--short", "HEAD"
        ),
        fomtools_branch=sh("git", "-C", str(REPO.parent / "fomtools"), "branch", "--show-current"),
        fomtools_path=str(Path(fomtools.__file__).parent),
    )


def run_variant(
    experiment: str, variant: str, setup: VariantSetup, check_only: bool, force: bool
) -> dict:
    from common import codegen_driver

    config = codegen_config()
    name = f"{experiment}_{variant}"
    out_dir = HERE / experiment / variant
    out_dir.mkdir(parents=True, exist_ok=True)

    check = {**setup.checks, **check_oracle(setup, name, config)}
    print(f"[{name}] oracle vs NumPy (rel): {check['oracle_vs_numpy_max_rel']:.2e}", flush=True)
    for key, value in setup.checks.items():
        print(f"[{name}] {key}: {value}", flush=True)
    (out_dir / "check.json").write_text(json.dumps(check, indent=2, default=float) + "\n")
    if check_only:
        return check

    started = datetime.now(timezone.utc).isoformat(timespec="seconds")
    t0 = time.time()
    ir, emitted = codegen_driver.translate(
        setup.recording, setup.boxes, out_dir, name, config=config, force=force
    )
    t_translate = time.time() - t0
    t0 = time.time()
    opt = codegen_driver.optimize(ir, emitted, out_dir, config=config, force=force)
    t_optimize = time.time() - t0

    deviation = {
        "translated": max_deviation(emitted, setup),
        "optimized": max_deviation(opt.path, setup),
    }

    params = setup.samples[0]
    rounds = {"numpy": [], "translated": [], "optimized": []}
    for _ in range(TIMING_ROUNDS):
        rounds["numpy"].append(numpy_us(setup, params))
        rounds["translated"].append(bench_us(emitted))
        rounds["optimized"].append(bench_us(opt.path))
    med = {k: statistics.median(v) for k, v in rounds.items()}

    result = dict(
        experiment=experiment,
        variant=variant,
        iterations=ITERATIONS,
        verified_run=str(Path(RUNS[experiment][0]) / RUNS[experiment][1]),
        numpy_us=med["numpy"],
        translated_us=med["translated"],
        optimized_us=med["optimized"],
        speedup_translation=med["numpy"] / med["translated"],
        speedup_optimization=med["translated"] / med["optimized"],
        speedup_total=med["numpy"] / med["optimized"],
        optimization_kept=bool(opt.kept),
        optimization_notes=opt.notes,
        max_abs_dev=deviation,
        timing_rounds=rounds,
        agent_wall_s=dict(translation=t_translate, code_optimization=t_optimize),
        agent_budgets_s=dict(
            translation=AGENT_TIMEOUT_S, code_optimization=AGENT_OPTIMIZE_TIMEOUT_S
        ),
        check=check,
        info=setup.info,
        machine=machine(),
        started_utc=started,
        finished_utc=datetime.now(timezone.utc).isoformat(timespec="seconds"),
    )
    (out_dir / "timing.json").write_text(json.dumps(result, indent=2, default=float) + "\n")
    print(
        f"[{name}] NumPy {med['numpy']:.2f} us | translated {med['translated']:.3f} us | "
        f"optimized {med['optimized']:.3f} us (kept={opt.kept}) | "
        f"speedups {result['speedup_translation']:.1f}x, {result['speedup_optimization']:.2f}x, "
        f"{result['speedup_total']:.1f}x | max|C-NumPy| {deviation}",
        flush=True,
    )
    return result
