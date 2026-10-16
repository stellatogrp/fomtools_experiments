"""Check that the sepA formulation is the same algorithm as the original one.

Usage: python sepa_check.py OUT_MARKER   (env as for config.sh; PROBLEM_DIR = problems_v2)

For ISTA and FISTA: extract both formulations with FOMTools exactly as
scaffold_pair.py does (LARGE_SCALE, 200 iterations), run FOMTools' reference
execution on each at the nominal parameter, and compare step sizes and final
iterates. Writes OUT_MARKER with the results and exits 0 only if the step sizes
are identical and the iterates agree (ISTA <= 1e-9, FISTA <= 1e-5).
"""

from __future__ import annotations

import os
import sys
from pathlib import Path

import numpy as np

from fomtools.codegen.cli import _extract_fom, _load_module_fom
from fomtools.codegen.config import Backend, CodegenConfig, TerminationSpec, Version
from fomtools.codegen.harness import representative_params
from fomtools.codegen.oracle import ReferenceOracle

LIMITS = {"ista": 1e-9, "fista": 1e-5}


def run(path: Path, iters: int):
    fom = _load_module_fom(str(path))
    config = CodegenConfig(version=Version.LARGE_SCALE, backend=Backend.OPENMP,
                           termination=TerminationSpec(max_iters=iters))
    ir = _extract_fom(fom, config)
    sol = ReferenceOracle(ir).solve(param_values=representative_params(ir)).solution
    return np.asarray(sol, dtype=float), float(fom["data"]["step_size"]), ir.name


def main() -> int:
    marker = Path(sys.argv[1])
    iters = int(os.environ.get("ITERATIONS", "200"))
    sepa_dir = Path(os.environ["PROBLEM_DIR"])
    orig_dir = Path(os.environ["REPO"])
    lines, ok = [], True
    for alg in ("ista", "fista"):
        s_sepa, eta_sepa, name_sepa = run(sepa_dir / f"paper_{alg}_sparse20k.py", iters)
        s_orig, eta_orig, name_orig = run(orig_dir / f"paper_{alg}_sparse20k.py", iters)
        diff = float(np.max(np.abs(s_sepa - s_orig))) if s_sepa.shape == s_orig.shape else float("inf")
        good = eta_sepa == eta_orig and diff <= LIMITS[alg]
        ok &= good
        lines.append(f"{alg}: {name_sepa} vs {name_orig}: eta {eta_sepa!r} vs {eta_orig!r} "
                     f"(identical: {eta_sepa == eta_orig}); state shapes {s_sepa.shape} vs {s_orig.shape}; "
                     f"max abs diff after {iters} iterations {diff:.3e} (limit {LIMITS[alg]:.0e}) -> "
                     f"{'OK' if good else 'FAIL'}")
        print(lines[-1], flush=True)
    lines.append("SEPA_CHECK_OK" if ok else "SEPA_CHECK_FAILED")
    marker.write_text("\n".join(lines) + "\n")
    print(lines[-1])
    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
