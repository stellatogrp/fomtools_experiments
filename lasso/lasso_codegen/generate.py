"""Generate the optimized OpenMP and CUDA solvers for the large-scale Lasso.

Run from ``lasso/`` on the della9 login node, with ``della/env.sh`` sourced (from
bash) and ``della/holders.sh`` running in another tmux window:

    python -m lasso_codegen.generate                       # reps 1-3, all four solvers
    python -m lasso_codegen.generate --algorithms ista --backends openmp --reps 1

The two agent phases run here, on the login node: compute nodes cannot reach the
Claude API. Every ``make`` they call (verify, bench, profile, the binaries) goes
through ``della/make`` into the current 1 h A100 holder. Solvers run one after
another; one with a ``record.json`` is done and skipped, so a stopped run resumes.

The translation phase runs through ``generate_code`` and the code optimization
phase through ``run_code_optimization`` on the same directory, so the agent time of
each phase and the rounds are recorded separately. Writes the solver to
``lasso/out/codegen/rep<k>/<algorithm>_<backend>/`` and, last, ``record.json``
there: a run without it is incomplete and is regenerated from scratch.
"""

from __future__ import annotations

import argparse
import json
import os
import shutil
import socket
import subprocess
import sys
import time
from datetime import datetime, timezone
from pathlib import Path

from lasso_codegen import problem

OUT = Path(__file__).resolve().parents[1] / "out" / "codegen"
REPO = Path(__file__).resolve().parents[2]


def out_dir(algorithm: str, backend: str, rep: int) -> Path:
    return OUT / f"rep{rep}" / f"{algorithm}_{backend}"


def config(backend: str):
    from fomtools.codegen import Backend, CodegenConfig, TerminationSpec, Version

    return CodegenConfig(
        version=Version.LARGE_SCALE,
        backend=Backend.OPENMP if backend == "openmp" else Backend.CUDA,
        termination=TerminationSpec(max_iters=problem.ITERATIONS),
        tol_verify=problem.TOL_VERIFY,
    )


def fomtools_version() -> str:
    import fomtools

    return fomtools.__version__


def fomtools_path() -> str:
    import fomtools

    return str(Path(fomtools.__file__).resolve().parents[1])


def allocation_hardware() -> str:
    """The GPU and node of the held allocation the builds run in."""
    holder = os.environ.get("HOLDER_FILE", "")
    alloc = os.environ.get("FOM_ALLOC") or (
        Path(holder).read_text().strip() if holder and Path(holder).exists() else ""
    )
    if not alloc:
        return ""
    cmd = ["srun", f"--jobid={alloc}", "--overlap", "-N1", "-n1", "bash", "-c",
           "echo $(hostname): $(nvidia-smi -L | head -1)"]  # fmt: skip
    return subprocess.run(cmd, capture_output=True, text=True).stdout.strip()


def generate(algorithm: str, backend: str, rep: int) -> dict:
    from fomtools.codegen.agent import get_default_driver
    from fomtools.codegen.generate import generate_code, run_code_optimization

    sys.path.append(str(REPO))
    from fomexp.records import code_state, environment

    target = out_dir(algorithm, backend, rep)
    if target.exists():
        shutil.rmtree(target)  # an incomplete earlier run: start clean
    cfg = config(backend)
    driver = get_default_driver(cfg)
    inst = problem.instance()
    started = datetime.now(timezone.utc).isoformat(timespec="seconds")

    t0 = time.time()
    common = dict(out_dir=target, config=cfg, driver=driver, code_optimization=False, profile=False)
    if algorithm == "ista":
        rec = problem.record_ista(inst)
        result = generate_code(
            rec["verifier"],
            rec["next_state"],
            name=rec["name"],
            iterates=rec["iterates"],
            param_values=rec["param_values"],
            **common,
        )
    else:
        rec = problem.record_fista(inst)
        result = generate_code(rec["build"], name=rec["name"], schedules=rec["schedules"], **common)
    t_translate = time.time() - t0
    if not result.success:
        raise SystemExit(
            f"translation failed for {algorithm}/{backend}:\n{result.translate.log[-4000:]}"
        )

    t0 = time.time()
    opt = run_code_optimization(result.ir, result.target, target, driver, cfg, profile=True)
    t_optimize = time.time() - t0

    record = dict(
        algorithm=algorithm,
        backend=backend,
        rep=rep,
        size=dict(zip(("m", "n", "nnz_per_row"), problem.size())),
        D_nnz=int(inst.D.nnz),
        step_size=inst.step_size,
        iterations=problem.ITERATIONS,
        tol_verify=problem.TOL_VERIFY,
        translated_us=opt.translated_us,
        optimized_us=opt.final_us,
        optimization_kept=opt.kept,
        agent_wall_s=dict(translation=t_translate, code_optimization=t_optimize),
        optimization_rounds=[r.as_dict() for r in opt.rounds],
        rule_friction=opt.rule_friction,
        fomtools=dict(version=fomtools_version(), path=fomtools_path()),
        model=os.environ.get("ANTHROPIC_MODEL", ""),
        agent_host=socket.gethostname(),
        build_host=allocation_hardware(),
        alloc=os.environ.get("FOM_ALLOC", "holders.sh"),
        env={k: os.environ.get(k, "") for k in ("OMPFLAGS", "NVCCFLAGS", "OMP_NUM_THREADS")},
        code=code_state(),
        environment=environment(),
        started_utc=started,
        finished_utc=datetime.now(timezone.utc).isoformat(timespec="seconds"),
    )
    (target / "record.json").write_text(json.dumps(record, indent=2, default=str) + "\n")
    return record


def wait_for_holder() -> None:
    """Block until holders.sh has a running holder for the builds."""
    if os.environ.get("FOM_ALLOC") or not os.environ.get("HOLDER_FILE"):
        return
    holder = Path(os.environ["HOLDER_FILE"])
    while not (holder.exists() and holder.read_text().strip()):
        print("waiting for a holder (is della/holders.sh running?)", flush=True)
        time.sleep(60)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--algorithms", nargs="+", choices=("ista", "fista"), default=["ista", "fista"]
    )
    parser.add_argument(
        "--backends", nargs="+", choices=("openmp", "cuda"), default=["openmp", "cuda"]
    )
    parser.add_argument("--reps", nargs="+", type=int, default=[1, 2, 3])
    args = parser.parse_args()
    for rep in args.reps:
        for algorithm in args.algorithms:
            for backend in args.backends:
                name = f"{algorithm}/{backend} rep {rep}"
                if (out_dir(algorithm, backend, rep) / "record.json").exists():
                    print(f"{name}: done, skipped", flush=True)
                    continue
                wait_for_holder()
                print(f"{datetime.now().isoformat(timespec='seconds')} {name}: start", flush=True)
                try:
                    record = generate(algorithm, backend, rep)
                except (Exception, SystemExit) as err:  # one failure does not stop the sweep
                    print(f"{name}: FAILED: {err}", flush=True)
                    continue
                print(
                    f"{name}: translated {record['translated_us']} us, optimized "
                    f"{record['optimized_us']} us, agent "
                    f"{record['agent_wall_s']['translation']:.0f} + "
                    f"{record['agent_wall_s']['code_optimization']:.0f} s",
                    flush=True,
                )


if __name__ == "__main__":
    main()
