"""Generate the three CVXPYgen solvers for each number of assets N.

Run from ``portfolio/`` in the ``algoverify`` conda env:

    FOMTOOLS_CODEGEN_AGENT=1 python -m portfolio_cvxpygen.generate [--N 10 20] [--arms fom] [--force]

Per N, beneath ``portfolio_cvxpygen/out/`` (flat names: cvxpygen imports the
code directory as a module, so it cannot contain a path separator):

``cpg_n{N}_osqp``
    stock CVXPYgen + OSQP, generated with OSQP's defaults (cvxpygen fixes rho,
    sigma, alpha and scaling at generation and exposes no way to change them);
    the ``off``/``on`` arms differ only in its runtime settings.
``cpg_n{N}_fom``
    CVXPYgen + the fomtools ADMM solver (``fomtools.cvxpygen.generate_solver``,
    paper Sec. 5.3) on the equilibrated canonical QP, 200 iterations per solve;
    translation phase only.
``cpg_n{N}_fom_opt``
    the same, followed by the code optimization phase (10-minute budget).

A directory that already holds a compiled wrapper is reused unless ``--force``.
``gen_n{N}.json`` records generation wall times and the canonical problem sizes.
"""

from __future__ import annotations

import argparse
import contextlib
import json
import time

from portfolio_cvxpygen.model import (
    FOM_OPTIONS,
    ITERATIONS,
    OUT,
    SIZES,
    build,
    code_dir,
    prefix,
    x_bounds,
)

ARMS = ("osqp", "fom", "fom_opt")
OPTIMIZE_TIMEOUT_S = 600  # the paper's Sec. 6.6 budget per phase


def compiled(N: int, arm: str) -> bool:
    return any((OUT / code_dir(N, arm)).glob("cpg_module*.so"))


def generate_osqp(N: int) -> dict:
    from cvxpygen import cpg

    problem, _, _ = build(N)
    start = time.perf_counter()
    with contextlib.chdir(OUT):
        cpg.generate_code(
            problem, code_dir=code_dir(N, "osqp"), solver="OSQP", prefix=prefix(N, "osqp"), wrapper=True
        )
    return dict(wall_s=time.perf_counter() - start)


def generate_fom(N: int, arm: str) -> dict:
    import fomtools.cvxpygen as fcpg
    from fomtools.codegen import CodegenConfig, TerminationSpec

    optimize = arm == "fom_opt"
    problem, x, _ = build(N)
    config = CodegenConfig(
        termination=TerminationSpec(max_iters=ITERATIONS),
        agent_optimize_timeout_s=OPTIMIZE_TIMEOUT_S,
    )
    start = time.perf_counter()
    with contextlib.chdir(OUT):
        result = fcpg.generate_solver(
            problem,
            code_dir(N, arm),
            prefix=prefix(N, arm),
            fom="admm",
            fom_options=FOM_OPTIONS,
            param_bounds={x: x_bounds(N)},
            config=config,
            auto_equilibrate=True,
            code_optimization=optimize,
        )
    info = dict(
        wall_s=time.perf_counter() - start,
        fom_options=FOM_OPTIONS,
        auto_equilibrate=True,
        code_optimization=optimize,
        out_dir=str(result.out_dir),
    )
    if optimize:
        info["optimization_kept"] = result.optimization_kept
        if result.timing is not None:
            info["timing"] = {
                k: v for k, v in vars(result.timing).items() if isinstance(v, (int, float, str))
            }
    return info


def canonical_sizes(N: int) -> dict:
    """Canonical QP sizes, and which canonical pieces depend on ``x`` (only ``u_ineq`` should)."""
    from cvxpygen.canonicalizer import Canonicalizer
    from fomtools.cvxpygen.qpdata import extract_qp_data

    problem, x, _ = build(N)
    canon, _ = Canonicalizer(solver="OSQP").canonicalize(problem)
    qp = extract_qp_data(canon, param_bounds={x: x_bounds(N)})
    return dict(
        n=qp.n,
        n_eq=qp.n_eq,
        n_ineq=qp.n_ineq,
        nnz_P=int(qp.P.nnz),
        nnz_A=int(qp.A.nnz),
        param_dependent=sorted(qp.param_dependent),
    )


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--N", type=int, nargs="+", default=list(SIZES))
    parser.add_argument("--arms", nargs="+", choices=ARMS, default=list(ARMS))
    parser.add_argument("--force", action="store_true")
    args = parser.parse_args()

    for N in args.N:
        record_path = OUT / f"gen_n{N}.json"
        record = json.loads(record_path.read_text()) if record_path.exists() else {}
        for arm in args.arms:
            if compiled(N, arm) and not args.force:
                print(f"N={N} {arm}: reusing {code_dir(N, arm)}")
                continue
            print(f"N={N} {arm}: generating {code_dir(N, arm)}", flush=True)
            OUT.mkdir(parents=True, exist_ok=True)
            record[arm] = generate_osqp(N) if arm == "osqp" else generate_fom(N, arm)
            print(f"N={N} {arm}: {record[arm]['wall_s']:.1f} s", flush=True)
        try:
            record["sizes"] = canonical_sizes(N)
        except Exception as e:  # sizes are informational only
            record["sizes"] = dict(error=repr(e))
        record_path.write_text(json.dumps(record, indent=2))


if __name__ == "__main__":
    main()
