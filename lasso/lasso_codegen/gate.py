"""Baseline gate: the hand-written SciPy iteration against fomtools' reference.

    python -m lasso_codegen.gate [--tol 1e-6]

Run once before any agent, as a CPU job (``della/gate.sbatch``). For each algorithm,
the SciPy iteration with ``D`` and ``D^T`` kept separate must reproduce fomtools'
reference execution of the extracted IR after 200 iterations, at the parameter
every timing uses. Otherwise the baselines and the generated solvers would not
be computing the same thing. FISTA must also match a textbook FISTA
(`problem.textbook_fista`), which pins the momentum schedule. Prints
``BASELINE_GATE_OK`` or exits 1.
"""

from __future__ import annotations

import argparse

import numpy as np

from lasso_codegen import baselines, problem
from lasso_codegen.generate import config


def reference(algorithm: str) -> np.ndarray:
    from fomtools.codegen.harness import representative_params
    from fomtools.codegen.oracle import ReferenceOracle

    ir = problem.extract_ir(algorithm, config("openmp"))
    return np.asarray(ReferenceOracle(ir).solve(param_values=representative_params(ir)).solution)


def check(algorithm: str) -> dict[str, float]:
    """Max abs differences from fomtools' reference execution of the extracted IR."""
    ref = reference(algorithm)
    inst = problem.instance()
    diffs = {"scipy": float(np.max(np.abs(baselines.scipy_solve(inst, algorithm) - ref)))}
    if algorithm == "fista":
        textbook = problem.textbook_fista(inst, problem.ITERATIONS)
        diffs["textbook"] = float(np.max(np.abs(textbook - ref)))
    return diffs


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--tol", type=float, default=1e-6)
    args = parser.parse_args()
    ok = True
    for algorithm in ("ista", "fista"):
        for against, diff in check(algorithm).items():
            print(
                f"CHECK {algorithm} K={problem.ITERATIONS} {against} vs fomtools reference: "
                f"max abs diff {diff:.3e}",
                flush=True,
            )
            ok &= diff <= args.tol
    m, n, k = problem.size()
    print(f"shape m={m} n={n} nnz_per_row={k}")
    print("BASELINE_GATE_OK" if ok else "BASELINE_GATE_FAILED")
    raise SystemExit(0 if ok else 1)


if __name__ == "__main__":
    main()
