"""Run one experiment variant (or, locally, every variant) through Hydra.

    python -m fomexp.run experiment=lasso variant=fista experiment.problem.m=30
    python -m fomexp.run experiment=lasso solver.time_limit=10 schedule.budget_hours=0.05

Without ``run_dir=...`` the next ``<experiment dir>/results/run_NNN`` is created.
The variant's outputs go to ``run_NNN/<variant>/``, which is also Hydra's run
directory, so ``.hydra/`` (config, overrides) lands beside the CSVs. Without a
variant, every variant runs in turn, each in its own process, into the same
``run_NNN``. On the cluster, ``slurm/submit.sh`` allocates the folder once and
each array task passes ``run_dir`` and ``variant_index``.
"""

from __future__ import annotations

import importlib.util
import logging
import subprocess
import sys
import time
from pathlib import Path

JOB_START = time.time()

import hydra  # noqa: E402
from hydra import compose, initialize_config_dir  # noqa: E402
from omegaconf import DictConfig  # noqa: E402

from fomexp.paths import CONF, REPO, next_run_dir, results_dir  # noqa: E402


def compose_config(overrides: list[str]) -> DictConfig:
    """The config without starting a Hydra run (no output folder is made)."""
    with initialize_config_dir(config_dir=str(CONF), version_base="1.3"):
        return compose(config_name="config", overrides=overrides)


def resolve_variant(cfg: DictConfig) -> str | None:
    variants = list(cfg.experiment.variants)
    if cfg.variant is not None:
        if cfg.variant not in variants:
            raise SystemExit(f"variant {cfg.variant!r} not in {variants}")
        return str(cfg.variant)
    if cfg.variant_index is not None:
        return variants[int(cfg.variant_index)]
    return None


def load_adapter(experiment_dir: str):
    """Import ``<experiment dir>/cluster_run.py`` with its folder on ``sys.path``.

    The experiment modules import their siblings by bare name (``from problem
    import ...``), so the folder goes first on the path. Only one experiment is
    loaded per process, so the clashing ``problem`` module names never meet.
    """
    folder = REPO / experiment_dir
    sys.path.insert(0, str(folder))
    spec = importlib.util.spec_from_file_location("cluster_run", folder / "cluster_run.py")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


@hydra.main(version_base="1.3", config_path="../conf", config_name="config")
def _driver(cfg: DictConfig) -> None:
    from hydra.core.hydra_config import HydraConfig

    from fomexp.kloop import run_variant

    # gurobipy also sends its log through Python logging, which Hydra routes to the
    # console and run.log; the full log already goes to gurobi_logs/K###.log.
    logging.getLogger("gurobipy").setLevel(logging.WARNING)
    variant = resolve_variant(cfg)
    out = Path(HydraConfig.get().runtime.output_dir)
    adapter = load_adapter(cfg.experiment.dir)
    spec = adapter.build(cfg, variant)
    print(f"{cfg.experiment.name}/{variant} -> {out}", flush=True)
    run_variant(cfg, spec, out, JOB_START)


def main() -> None:
    argv = [a for a in sys.argv[1:]]
    overrides = [a for a in argv if "=" in a and not a.startswith("hydra.")]
    cfg = compose_config(overrides)
    variant = resolve_variant(cfg)

    if cfg.run_dir is None:
        from fomexp.plan import write_run_manifest

        run_dir = next_run_dir(results_dir(cfg.experiment.dir))
        write_run_manifest(run_dir, cfg, overrides)
        argv.append(f"run_dir={run_dir}")
        print(f"run folder: {run_dir}", flush=True)
    else:
        run_dir = Path(cfg.run_dir)
        run_dir.mkdir(parents=True, exist_ok=True)

    if variant is None:
        # Each variant in its own process: @hydra.main runs once per process.
        failed = []
        for name in cfg.experiment.variants:
            done = subprocess.run(
                [sys.executable, "-m", "fomexp.run", *argv, f"variant={name}"], cwd=REPO
            )
            if done.returncode:
                failed.append(name)
        if failed:
            raise SystemExit(f"variants failed: {failed}; see their manifest.json and run.log")
        return

    sys.argv = [sys.argv[0], f"hydra.run.dir={run_dir / variant}", *argv]
    _driver()


if __name__ == "__main__":
    main()
