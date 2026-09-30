"""Generate Phase-B-optimized CPU and CUDA solvers for ISTA and FISTA.

Run from the repository root:

    FOMTOOLS_CODEGEN_BACKEND=codex python -m lasso.generate

Generated artifacts are written beneath ``lasso/out/`` and are intentionally
untracked. Each optimized directory retains ``OPTIMIZATIONS.md`` and the full
Phase-A/Phase-B agent transcripts.
"""

from __future__ import annotations

import argparse
import shutil
from pathlib import Path

from fomtools.codegen import Backend, CodegenConfig, TerminationSpec, Version
from fomtools.codegen.generate import generate_code

from lasso.problem import build_problem
from lasso.recording import record_fista, record_ista

OUT = Path(__file__).parent / "out" / "codegen"


def _config(backend: str, max_iterations: int) -> CodegenConfig:
    if backend == "cpu":
        return CodegenConfig(
            version=Version.EMBEDDED,
            backend=Backend.C,
            termination=TerminationSpec(max_iters=max_iterations),
        )
    return CodegenConfig(
        version=Version.LARGE_SCALE,
        backend=Backend.CUDA,
        termination=TerminationSpec(max_iters=max_iterations),
    )


def _generate(algorithm: str, backend: str, max_iterations: int, force: bool) -> None:
    target = OUT / f"{algorithm}_{backend}_optimized"
    if target.exists():
        if not force:
            raise FileExistsError(f"{target} already exists; pass --force to regenerate")
        shutil.rmtree(target)

    problem = build_problem()
    recording = record_ista(problem) if algorithm == "ista" else record_fista(problem)
    kwargs = {
        "out_dir": target,
        "config": _config(backend, max_iterations),
        "name": recording["name"],
        "run_phase_b": True,
        "profile": True,
    }
    if algorithm == "ista":
        result = generate_code(
            recording["verifier"],
            recording["next_state"],
            iterates=recording["iterates"],
            param_values=recording["param_values"],
            **kwargs,
        )
    else:
        result = generate_code(
            recording["build"],
            schedules=recording["schedules"],
            **kwargs,
        )
    if not result.success:
        raise RuntimeError(f"code generation failed for {algorithm}/{backend}")
    print(f"generated and verified {algorithm}/{backend}: {target}")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--algorithm", choices=("ista", "fista", "all"), default="all")
    parser.add_argument("--backend", choices=("cpu", "cuda", "all"), default="all")
    parser.add_argument("--max-iterations", type=int, default=200)
    parser.add_argument("--force", action="store_true")
    args = parser.parse_args()

    algorithms = ("ista", "fista") if args.algorithm == "all" else (args.algorithm,)
    backends = ("cpu", "cuda") if args.backend == "all" else (args.backend,)
    for algorithm in algorithms:
        for backend in backends:
            _generate(algorithm, backend, args.max_iterations, args.force)


if __name__ == "__main__":
    main()
