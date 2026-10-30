"""Submit one verification experiment to della: one Slurm array task per variant.

    python -m fomexp.submit experiment=lasso              # sbatch --test-only, nothing created
    python -m fomexp.submit experiment=lasso --yes        # allocate run_NNN and submit
    python -m fomexp.submit experiment=lasso variant=fista run_dir=<run> --yes   # rerun one variant there

Run from the repository root on a della login node. Any Hydra override is passed
on to every task (``experiment.problem.m=30``, ``solver.time_limit=600``); the
resources come from ``conf/cluster/della.yaml`` (``cluster=della``, the default).
Each task runs ``fomexp.run`` with ``run_dir`` and ``variant_index`` set
(``fomexp/della/verify.sbatch``) and logs to ``run_NNN/slurm/``.
"""

from __future__ import annotations

import subprocess
import sys
from pathlib import Path

from fomexp.paths import REPO, next_run_dir, peek_next_run_dir, results_dir
from fomexp.plan import slurm_time, write_run_manifest
from fomexp.records import utc_now

JOB = REPO / "fomexp" / "della" / "verify.sbatch"


def sbatch_argv(cfg, run_dir, test_only: bool, overrides_file: str = "overrides.txt") -> list[str]:
    n = 1 if cfg.variant else len(cfg.experiment.variants)
    throttle = f"%{cfg.cluster.max_parallel}" if cfg.cluster.max_parallel else ""
    argv = [
        "sbatch",
        *(["--test-only"] if test_only else ["--parsable"]),
        f"--job-name=fom-{cfg.experiment.name}",
        f"--array=0-{n - 1}{throttle}",
        f"--cpus-per-task={cfg.cluster.cpus}",
        f"--mem={cfg.cluster.mem}",
        f"--time={slurm_time(cfg)}",
        f"--account={cfg.cluster.account}",
        f"--output={run_dir}/slurm/%x-{cfg.variant or '%a'}-%j.out",
        "--export=ALL,"
        f"EXPERIMENT={cfg.experiment.name},RUN_DIR={run_dir},VARIANT={cfg.variant or ''},"
        f"OVERRIDES_FILE={overrides_file},"
        f"GRB_LICENSE_FILE={cfg.cluster.license_file}",
    ]
    if cfg.cluster.partition:
        argv.append(f"--partition={cfg.cluster.partition}")
    if cfg.cluster.mail:
        argv += [f"--mail-user={cfg.cluster.mail}", f"--mail-type={cfg.cluster.mail_type}"]
    return [*argv, str(JOB)]


def main() -> None:
    from fomexp.run import compose_config

    args = sys.argv[1:]
    submit = "--yes" in args
    overrides = [a for a in args if a != "--yes"]
    cfg = compose_config(overrides)
    results = results_dir(cfg.experiment.dir)

    # The test-only check comes first, against the folder a submission would get.
    check = subprocess.run(
        sbatch_argv(
            cfg, Path(cfg.run_dir).resolve() if cfg.run_dir else peek_next_run_dir(results), True
        ),
        cwd=REPO,
        capture_output=True,
        text=True,
    )
    print((check.stderr or check.stdout).strip())
    if check.returncode != 0 or not submit:
        sys.exit(check.returncode if check.returncode else (0 if not submit else 1))

    task_overrides = [
        o for o in overrides if not o.startswith(("experiment=", "variant=", "run_dir="))
    ]
    if cfg.run_dir:  # rerun into an existing run folder; its manifest stays as it is
        run_dir = Path(cfg.run_dir).resolve()
        overrides_file = f"overrides_{cfg.variant or 'all'}_{utc_now().replace(':', '')}.txt"
    else:
        run_dir = next_run_dir(results)
        (run_dir / "slurm").mkdir()
        write_run_manifest(run_dir, cfg, overrides)
        overrides_file = "overrides.txt"
    (run_dir / overrides_file).write_text("".join(f"{o}\n" for o in task_overrides))
    job = subprocess.run(
        sbatch_argv(cfg, run_dir, test_only=False, overrides_file=overrides_file),
        cwd=REPO,
        capture_output=True,
        text=True,
        check=True,
    ).stdout.strip()
    print(f"submitted {job}: {cfg.experiment.name} {list(cfg.experiment.variants)} -> {run_dir}")


if __name__ == "__main__":
    main()
