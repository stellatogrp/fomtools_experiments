"""The K loop shared by every experiment.

An adapter (``<experiment>/cluster_run.py``) returns a ``VariantSpec``; this
module runs it:

1. the simulator up to ``simulator.k_max``, which is cheap and gives the sampled
   curve even if the MIP loop is cut short;
2. the MIP for K = 1, 2, ... on ONE verifier, so fomtools' sequential warm start
   (the worst case at K - 1 run through step K) applies at every K, together
   with the sample-max start from ``set_start_samples``;
3. a stop once another K at the full per-K time limit would not fit in the
   budget, or at ``schedule.k_max``. A K that hits its time limit does not stop
   the loop: its ``ObjBound`` is still a valid upper bound, and its incumbent
   still warm-starts the next K.

Every number a solve exposes goes into ``bounds.csv``, one row per K, written
out after every K.
"""

from __future__ import annotations

import json
import math
import time
from collections.abc import Callable
from dataclasses import dataclass, field
from pathlib import Path

import numpy as np
from omegaconf import OmegaConf

from fomexp import gurobi
from fomexp.records import CsvLog, code_state, environment, utc_now, write_json

#: ``setup(vp, mode, samples, length) -> (param, step)``. ``mode`` is "mip" or
#: "simulator"; ``samples`` are the simulator's parameter samples (None for the
#: MIP); ``length`` is the trajectory length, K + 1. ``step(k)`` appends
#: iteration k (k >= 1) and sets the objective at k.
Setup = Callable[..., tuple[object, Callable[[int], None]]]


@dataclass
class VariantSpec:
    experiment: str
    variant: str
    setup: Setup
    sim_samples: np.ndarray
    start_samples: np.ndarray | None
    info: dict = field(default_factory=dict)
    #: ``reference(samples, K) -> residuals`` with shape ``(len(samples), K)``,
    #: an independent NumPy run used to cross-check the simulator.
    reference: Callable[[np.ndarray, int], np.ndarray] | None = None


def _scalar(value):
    if isinstance(value, np.generic):
        return value.item()
    return value


def _flatten(prefix: str, data, row: dict, extra: dict) -> None:
    """Scalars become ``prefix_key`` columns; anything else goes to ``extra_json``."""
    if data is None:
        return
    if isinstance(data, dict):
        for key, value in data.items():
            _flatten(f"{prefix}_{key}" if prefix else str(key), value, row, extra)
        return
    if isinstance(data, (list, tuple)) and all(isinstance(v, dict) for v in data):
        row[f"{prefix}_count"] = len(data)
        for i, item in enumerate(data):
            name = item.get("kind", i) if isinstance(item, dict) else i
            _flatten(f"{prefix}_{name}", {k: v for k, v in item.items() if k != "kind"}, row, extra)
        return
    value = _scalar(data)
    if value is None or isinstance(value, (bool, int, float, str)):
        row[prefix] = value
    else:
        extra[prefix] = np.asarray(value).tolist() if hasattr(value, "__len__") else str(value)


def _problem_rows(info: dict) -> list[dict]:
    """``problem.csv``: one ``name, value`` row per scalar, arrays summarized."""
    rows = []
    for key, value in info.items():
        value = _scalar(value)
        if isinstance(value, (bool, int, float, str)) or value is None:
            rows.append(dict(name=key, value=value))
        else:
            arr = np.asarray(value, dtype=float)
            rows.append(dict(name=f"{key}.shape", value="x".join(map(str, arr.shape))))
            if arr.size:
                for stat in ("min", "max", "mean"):
                    rows.append(dict(name=f"{key}.{stat}", value=float(getattr(arr, stat)())))
    return rows


def run_simulator(cfg, spec: VariantSpec, out: Path) -> None:
    from fomtools.verifier import Verifier

    K = int(cfg.simulator.k_max)
    samples = spec.sim_samples
    vp = Verifier(method="simulator", obbt_solver=None)
    _, step = spec.setup(vp, "simulator", samples, K + 1)
    start = time.perf_counter()
    for k in range(1, K + 1):
        step(k)
    t_build = time.perf_counter() - start
    start = time.perf_counter()
    residuals = np.asarray(vp.solve(), dtype=float)
    t_sim = time.perf_counter() - start
    np.savez_compressed(out / "simulator_residuals.npz", residuals=residuals, samples=samples)

    ref = None
    if spec.reference is not None and cfg.simulator.cross_check:
        ref = np.asarray(spec.reference(samples, K), dtype=float)
    rows = []
    for k in range(1, K + 1):
        col = residuals[:, k - 1]
        row = dict(
            experiment=spec.experiment,
            variant=spec.variant,
            K=k,
            n_samples=len(col),
            max=float(col.max()),
            mean=float(col.mean()),
            std=float(col.std()),
            min=float(col.min()),
            median=float(np.median(col)),
            p90=float(np.quantile(col, 0.9)),
            p99=float(np.quantile(col, 0.99)),
            argmax_index=int(col.argmax()),
            t_build=t_build,
            t_sim=t_sim,
        )
        if ref is not None:
            row["numpy_max"] = float(ref[:, k - 1].max())
            row["numpy_abs_diff"] = float(np.abs(ref[:, k - 1] - col).max())
        rows.append(row)
    CsvLog(out / "simulator.csv").extend(rows)
    print(
        f"[{spec.variant}] simulator K=1..{K}: {t_sim:.1f} s, max at K={K}: {rows[-1]['max']:.4e}",
        flush=True,
    )
    if ref is not None:
        worst = max(r["numpy_abs_diff"] for r in rows)
        print(f"[{spec.variant}] simulator vs NumPy: max abs diff {worst:.1e}", flush=True)


def run_mip(cfg, spec: VariantSpec, out: Path, job_start: float, manifest: dict) -> None:
    from fomtools.verifier import Verifier

    solver_cfg, schedule = cfg.solver, cfg.schedule
    k_max = int(schedule.k_max)
    budget = float(schedule.budget_hours) * 3600.0
    time_limit = float(solver_cfg.time_limit)
    margin = float(schedule.build_margin_min) * 60.0
    logs = out / "gurobi_logs"
    if solver_cfg.save_logs:
        logs.mkdir(exist_ok=True)

    vp = Verifier(
        method="mip",
        solver="gurobi",
        obbt_solver=solver_cfg.obbt_solver,
        params=gurobi.construction_params(solver_cfg),
        obbt_params=gurobi.obbt_params(solver_cfg),
    )
    param, step = spec.setup(vp, "mip", None, k_max + 1)
    if schedule.sample_start and spec.start_samples is not None:
        vp.set_start_samples({param: spec.start_samples})

    bounds = CsvLog(out / "bounds.csv")
    worst = CsvLog(out / "worst_case.csv")
    solve_kwargs = dict(
        warm_start=bool(schedule.sequential_warm_start or schedule.sample_start),
        tighten_objective_bounds=solver_cfg.tighten_objective_bounds,
        convex_hull_cuts=bool(solver_cfg.convex_hull_cuts),
        cut_rounds=int(solver_cfg.cut_rounds),
    )
    overhead = 0.0  # the last K's time outside Gurobi, as a guess for the next one
    stop = "k_max"
    for k in range(1, k_max + 1):
        elapsed = time.time() - job_start
        needed = time_limit + 1.5 * overhead + margin
        if elapsed + needed > budget:
            stop = "budget"
            print(
                f"[{spec.variant}] stopping before K={k}: {elapsed / 3600:.3f} h used, "
                f"{needed / 3600:.3f} h needed, budget {budget / 3600:.3f} h",
                flush=True,
            )
            break
        manifest["k_in_progress"] = k
        write_json(out / "manifest.json", manifest)

        log_file = logs / f"K{k:03d}.log" if solver_cfg.save_logs else None
        params = gurobi.solve_params(solver_cfg, time_limit, log_file)
        error = ""
        t_build = t_solve = math.nan
        try:
            start = time.perf_counter()
            step(k)
            t_build = time.perf_counter() - start
            start = time.perf_counter()
            try:
                value = float(vp.solve(**solve_kwargs, **params))
            except AttributeError as exc:
                # No incumbent: ObjVal is undefined, but the bound and the rest still are.
                value, error = math.nan, f"{type(exc).__name__}: {exc}"
            t_solve = time.perf_counter() - start
        except Exception as exc:
            # Anything else ends the loop, but the failing K still gets a row.
            bounds.append(
                dict(
                    experiment=spec.experiment,
                    variant=spec.variant,
                    K=k,
                    run_id=out.parent.name,
                    timestamp_utc=utc_now(),
                    error=f"{type(exc).__name__}: {exc}",
                    t_build=t_build,
                    cumulative_time=time.time() - job_start,
                )
            )
            raise

        canon = vp.canonicalizer
        row: dict = dict(
            experiment=spec.experiment,
            variant=spec.variant,
            K=k,
            run_id=out.parent.name,
            slurm_job_id=manifest["environment"]["slurm"].get("SLURM_JOB_ID", ""),
            slurm_array_task_id=manifest["environment"]["slurm"].get("SLURM_ARRAY_TASK_ID", ""),
            timestamp_utc=utc_now(),
            vp_return_value=value,
            error=error,
        )
        row.update(gurobi.solve_record(canon._solved_copy))
        row.update(
            t_build=t_build,
            t_solve=t_solve,
            t_outside_gurobi=t_solve
            - (row["gurobi_runtime"] if not math.isnan(row["gurobi_runtime"]) else 0.0),
            t_total=t_build + t_solve,
            cumulative_time=time.time() - job_start,
        )
        extra: dict = {}
        _flatten("objbound", getattr(vp, "objective_bound_stats", None), row, extra)
        _flatten("warmstart", getattr(canon, "warm_start_stats", None), row, extra)
        _flatten("cuts", getattr(vp, "cut_stats", None) or None, row, extra)
        _flatten("separation", getattr(vp, "separation_stats", None) or None, row, extra)
        if log_file is not None:
            _flatten("log", gurobi.parse_log(log_file), row, extra)
        row["extra_json"] = json.dumps(extra) if extra else ""
        bounds.append(row)
        overhead = row["t_outside_gurobi"] + t_build

        if canon._solved_copy.has_solution():
            x = np.asarray(canon.get_solution_value(param), dtype=float).reshape(-1)
            worst.append(
                dict(
                    K=k, incumbent=row["incumbent"], **{f"x_{i}": float(v) for i, v in enumerate(x)}
                )
            )

        print(
            f"[{spec.variant}] K={k:3d}  bound {row['bound']:.4e}  incumbent {row['incumbent']:.4e}  "
            f"gap {row['gap']:.3f}  {row['status']:10s} gurobi {row['gurobi_runtime']:.1f} s  "
            f"total {row['t_total']:.1f} s  elapsed {row['cumulative_time'] / 3600:.2f} h",
            flush=True,
        )
    manifest["k_in_progress"] = None
    manifest["stop_reason"] = stop
    manifest["last_k"] = max((int(r["K"]) for r in bounds.rows if not r.get("error")), default=0)


def run_variant(cfg, spec: VariantSpec, out: Path, job_start: float) -> None:
    out.mkdir(parents=True, exist_ok=True)
    write_json(out / "problem.json", spec.info)
    CsvLog(out / "problem.csv").extend(_problem_rows(spec.info))
    manifest = dict(
        experiment=spec.experiment,
        variant=spec.variant,
        started_utc=utc_now(),
        finished_utc=None,
        stop_reason="running",
        environment=environment(),
        code=code_state(),
        config=OmegaConf.to_container(cfg, resolve=True),
    )
    write_json(out / "manifest.json", manifest)
    try:
        if cfg.simulator.enabled:
            run_simulator(cfg, spec, out)
        if cfg.schedule.run_mip:
            run_mip(cfg, spec, out, job_start, manifest)
        else:
            manifest["stop_reason"] = "simulator_only"
    except BaseException as exc:
        manifest["stop_reason"] = f"error: {type(exc).__name__}: {exc}"
        raise
    finally:
        manifest["finished_utc"] = utc_now()
        manifest["wall_hours"] = (time.time() - job_start) / 3600.0
        write_json(out / "manifest.json", manifest)
