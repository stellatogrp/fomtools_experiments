"""Wall time of the deterministic code-generation stages, without the coding agent.

Run from ``portfolio/``:

    python -m portfolio_cvxpygen.profile_codegen [--N 200 500] [--cprofile]

Follows the fomtools CVXPYgen front end (``FomtoolsInterface.generate_code``) up to
the agent: canonicalize, extract the QP, Ruiz-equilibrate, record the ADMM
iteration, extract the IR (including the KKT factorization), scaffold the
solver files, and time the Python reference solve the agent's checks run. Prints
the time of each stage, the size of the scaffolded data, and with ``--cprofile``
the top functions of the slowest stages.
"""

from __future__ import annotations

import argparse
import cProfile
import io
import pstats
import tempfile
import time
from pathlib import Path

from portfolio_cvxpygen.model import FOM_OPTIONS, ITERATIONS, build, param_bounds


def stages(N: int, out: Path, prof: cProfile.Profile | None):
    from cvxpygen.canonicalizer import Canonicalizer
    from fomtools.codegen import CodegenConfig, TerminationSpec
    from fomtools.codegen.emit import scaffold
    from fomtools.codegen.generate import extract_fom_ir
    from fomtools.codegen.harness import representative_params
    from fomtools.codegen.oracle import ReferenceOracle
    from fomtools.cvxpygen.builders import resolve_builder
    from fomtools.cvxpygen.qpdata import extract_qp_data
    from fomtools.cvxpygen.scaling import ruiz_for, scale_qp

    config = CodegenConfig(termination=TerminationSpec(max_iters=ITERATIONS))
    times, state = {}, {}

    def run(name, fn):
        if prof:
            prof.enable()
        start = time.perf_counter()
        result = fn()
        times[name] = time.perf_counter() - start
        if prof:
            prof.disable()
        print(f"  {name:12s} {times[name]:9.2f} s", flush=True)
        return result

    problem, params, _ = build(N)
    canon = run("canonicalize", lambda: Canonicalizer(solver="OSQP").canonicalize(problem)[0])
    qp = run("qp_data", lambda: extract_qp_data(canon, param_bounds=param_bounds(N, params)))
    qp_s = run("equilibrate", lambda: scale_qp(qp, ruiz_for(qp)))
    spec = run("record", lambda: resolve_builder("admm")(qp_s, **FOM_OPTIONS))
    ir = run("extract_ir", lambda: extract_fom_ir(
        spec.verifier, spec.next_state, config=config, state=list(spec.state),
        params=list(spec.params), name="fom", iterates=list(spec.iterates) or None,
        param_values=spec.param_values))
    run("scaffold", lambda: scaffold(ir, out, config))
    oracle = ReferenceOracle(ir)
    sample = representative_params(ir)
    run("oracle_solve", lambda: oracle.solve(param_values=sample))
    state["data_mb"] = sum(f.stat().st_size for f in out.rglob("*") if f.is_file()) / 1e6
    fac = next(iter(ir.factorizations.values()), None)
    state["kkt"] = (fac.dim, fac.nnz) if fac else None
    return times, state


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--N", type=int, nargs="+", default=[200, 500])
    parser.add_argument("--cprofile", action="store_true")
    args = parser.parse_args()
    for N in args.N:
        print(f"N={N}", flush=True)
        prof = cProfile.Profile() if args.cprofile else None
        with tempfile.TemporaryDirectory() as tmp:
            times, state = stages(N, Path(tmp), prof)
        print(f"  total {sum(times.values()):.1f} s, files {state['data_mb']:.1f} MB, "
              f"KKT (dim, factor nnz) {state['kkt']}")
        if prof:
            s = io.StringIO()
            pstats.Stats(prof, stream=s).sort_stats("cumulative").print_stats(25)
            print(s.getvalue())


if __name__ == "__main__":
    main()
