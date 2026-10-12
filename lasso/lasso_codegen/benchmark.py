"""Run interleaved NumPy, optimized CPU, and optimized CUDA measurements."""

from __future__ import annotations

import argparse
import os
import subprocess
from pathlib import Path

from fomtools.codegen import Backend, CodegenConfig, TerminationSpec, Version
from fomtools.codegen.generate import extract_fom_ir
from fomtools.codegen.harness import representative_params, time_python_oracle
from lasso_codegen.problem import build_problem
from lasso_codegen.recording import record_fista, record_ista

ROOT = Path(__file__).parent
CODEGEN = ROOT / "out" / "codegen"


def _irs(problem, iterations: int):
    config = CodegenConfig(
        version=Version.LARGE_SCALE,
        backend=Backend.CUDA,
        termination=TerminationSpec(max_iters=iterations),
    )
    ista = record_ista(problem)
    ista_ir = extract_fom_ir(
        ista["verifier"],
        ista["next_state"],
        config=config,
        name=ista["name"],
        iterates=ista["iterates"],
        param_values=ista["param_values"],
    )
    fista = record_fista(problem)
    fista_ir = extract_fom_ir(
        fista["build"],
        config=config,
        name=fista["name"],
        schedules=fista["schedules"],
    )
    return ista_ir, fista_ir


def _generated_line(algorithm: str, backend: str, solves: int) -> str:
    directory = CODEGEN / f"{algorithm}_{backend}_optimized"
    binary = directory / "bench_bin"
    if not binary.exists():
        subprocess.run(["make", "compile"], cwd=directory, check=True)
    completed = subprocess.run(
        [str(binary), str(solves)],
        check=True,
        capture_output=True,
        text=True,
        env=os.environ.copy(),
    )
    line = next(line for line in completed.stdout.splitlines() if line.startswith("BENCH "))
    return line


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--rounds", type=int, default=7)
    parser.add_argument("--generated-solves", type=int, default=5000)
    parser.add_argument("--numpy-repetitions", type=int, default=50)
    parser.add_argument("--iterations", type=int, default=200)
    parser.add_argument(
        "--output",
        type=Path,
        default=ROOT / "out" / "paper_final_compute_7rounds.txt",
    )
    args = parser.parse_args()
    args.output.parent.mkdir(parents=True, exist_ok=True)

    problem = build_problem()
    ista_ir, fista_ir = _irs(problem, args.iterations)
    with args.output.open("w") as output:
        def emit(line: str) -> None:
            print(line)
            print(line, file=output, flush=True)

        for round_number in range(1, args.rounds + 1):
            emit(f"ROUND {round_number}")
            emit(
                "NUMPY_ISTA median_us="
                f"{time_python_oracle(ista_ir, representative_params(ista_ir), repeats=args.numpy_repetitions):.6f}"
            )
            emit("CPU_ISTA")
            emit(_generated_line("ista", "cpu", args.generated_solves))
            emit("CUDA_ISTA")
            emit(_generated_line("ista", "cuda", args.generated_solves))
            emit(
                "NUMPY_FISTA median_us="
                f"{time_python_oracle(fista_ir, representative_params(fista_ir), repeats=args.numpy_repetitions):.6f}"
            )
            emit("CPU_FISTA")
            emit(_generated_line("fista", "cpu", args.generated_solves))
            emit("CUDA_FISTA")
            emit(_generated_line("fista", "cuda", args.generated_solves))


if __name__ == "__main__":
    main()
