"""Check recording fidelity and the integrity of checked-in Lasso results."""

from __future__ import annotations

import csv
import re
import tempfile
from pathlib import Path

import numpy as np
from fomtools.codegen import Backend, CodegenConfig, TerminationSpec, Version
from fomtools.codegen.generate import extract_fom_ir
from fomtools.codegen.harness import representative_params
from fomtools.codegen.oracle import ReferenceOracle
from lasso_codegen.problem import build_problem, solve_fista, solve_ista
from lasso_codegen.recording import record_fista, record_ista
from lasso_codegen.summarize import main as summarize

ROOT = Path(__file__).parent
RESULTS = ROOT / "results" / "paper_40x80"
VERIFY = re.compile(
    r"worst max-abs error over 8 samples: ([0-9.eE+-]+) \(tol ([0-9.eE+-]+)\)"
)


def _extract(problem):
    config = CodegenConfig(
        version=Version.LARGE_SCALE,
        backend=Backend.CUDA,
        termination=TerminationSpec(max_iters=200),
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


def _check_recordings() -> None:
    problem = build_problem()
    ista_ir, fista_ir = _extract(problem)
    ista_error = np.max(
        np.abs(
            solve_ista(problem)
            - ReferenceOracle(ista_ir).solve(representative_params(ista_ir)).solution
        )
    )
    fista_error = np.max(
        np.abs(
            solve_fista(problem)
            - ReferenceOracle(fista_ir).solve(representative_params(fista_ir)).solution
        )
    )
    assert ista_error < 1e-10
    assert fista_error < 1e-10
    print(f"recordings OK: ISTA error={ista_error:.3e}, FISTA error={fista_error:.3e}")


def _check_verification_logs() -> None:
    paths = sorted(RESULTS.glob("verify_*.log"))
    assert len(paths) == 4, paths
    for path in paths:
        text = path.read_text()
        match = VERIFY.search(text)
        assert match and "VERIFY OK" in text, path
        error, tolerance = map(float, match.groups())
        assert error <= tolerance, path
        print(f"{path.name}: {error:.3e} <= {tolerance:.3e}")


def _check_summary() -> None:
    # Call the CLI implementation without duplicating its parsing logic.
    import sys

    with tempfile.TemporaryDirectory() as directory:
        output = Path(directory) / "summary.csv"
        previous = sys.argv
        try:
            sys.argv = [
                "lasso_codegen.summarize",
                str(RESULTS / "paper_final_compute_7rounds.txt"),
                "--output",
                str(output),
            ]
            summarize()
        finally:
            sys.argv = previous
        with output.open(newline="") as generated, (RESULTS / "summary.csv").open(
            newline=""
        ) as expected:
            assert list(csv.DictReader(generated)) == list(csv.DictReader(expected))
    print("summary.csv OK")


def main() -> None:
    _check_recordings()
    _check_verification_logs()
    _check_summary()
    print("LASSO CHECKS OK")


if __name__ == "__main__":
    main()
