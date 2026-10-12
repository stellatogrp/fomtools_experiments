"""Allocate a run folder and print what ``slurm/submit.sh`` needs to submit it.

    python -m fomexp.plan experiment=lasso experiment.problem.m=30 [--dry-run]

Creates ``<experiment dir>/results/run_NNN`` (next free number), writes the
resolved config and a run-level ``manifest.json`` there, and prints shell
assignments (``RUN_DIR=...``, ``N_VARIANTS=...``, the Slurm resources) for
``eval``. With ``--dry-run`` nothing is created and RUN_DIR is the folder a real
submission would get.
"""

from __future__ import annotations

import shlex
import sys
from pathlib import Path

from omegaconf import OmegaConf

from fomexp.paths import next_run_dir, peek_next_run_dir, results_dir
from fomexp.records import code_state, utc_now, write_json


def slurm_time(cfg) -> str:
    if cfg.cluster.time:
        return str(cfg.cluster.time)
    minutes = int(round(float(cfg.schedule.budget_hours) * 60 + float(cfg.cluster.slack_min)))
    return f"{minutes // 60}:{minutes % 60:02d}:00"


def write_run_manifest(run_dir: Path, cfg, overrides: list[str]) -> None:
    OmegaConf.save(cfg, run_dir / "config.yaml", resolve=True)
    write_json(
        run_dir / "manifest.json",
        dict(
            experiment=cfg.experiment.name,
            run_id=run_dir.name,
            created_utc=utc_now(),
            label=cfg.label,
            overrides=overrides,
            variants=list(cfg.experiment.variants),
            code=code_state(),
        ),
    )


def main() -> None:
    from fomexp.run import compose_config

    args = sys.argv[1:]
    dry = "--dry-run" in args
    overrides = [a for a in args if a != "--dry-run"]
    cfg = compose_config(overrides)
    results = results_dir(cfg.experiment.dir)
    if dry:
        run_dir = peek_next_run_dir(results)
    else:
        run_dir = next_run_dir(results)
        (run_dir / "slurm").mkdir()
        write_run_manifest(run_dir, cfg, overrides)
        # job.slurm reads the overrides from here (sbatch --export splits on commas);
        # it passes experiment= itself.
        (run_dir / "overrides.txt").write_text(
            "".join(f"{o}\n" for o in overrides if not o.startswith("experiment="))
        )
    values = dict(
        RUN_DIR=str(run_dir),
        EXPERIMENT=cfg.experiment.name,
        VARIANTS=" ".join(cfg.experiment.variants),
        N_VARIANTS=len(cfg.experiment.variants),
        CPUS=cfg.cluster.cpus,
        MEM=cfg.cluster.mem,
        TIME=slurm_time(cfg),
        PARTITION=cfg.cluster.partition or "",
        MAIL=cfg.cluster.mail or "",
        MAIL_TYPE=cfg.cluster.mail_type or "",
    )
    for key, value in values.items():
        print(f"{key}={shlex.quote(str(value))}")


if __name__ == "__main__":
    main()
