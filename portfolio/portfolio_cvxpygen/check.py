"""Accuracy of every arm at the settings it is timed with, for each N.

Run from ``portfolio/`` in the ``algoverify`` conda env, after ``generate``:

    python -m portfolio_cvxpygen.check [--N 10 20]

Not an equivalence check: OSQP and the fomtools solver are not expected to
produce the same iterates. For the first ``N_CHECK`` holdings of the back-test
sequence, solved in order (so ``osqp_on`` warm-starts as it does when timed),
it reports each arm's iteration count, the largest ``|z - z*|`` and the largest
relative objective gap against a Clarabel solve at 1e-10, so the timings can be
read against the accuracy each arm reaches. Writes ``out/check_n{N}.json``.
"""

from __future__ import annotations

import argparse
import json

import cvxpy as cp
import numpy as np

from portfolio_cvxpygen.benchmark import PY_ARMS, python_arm
from portfolio_cvxpygen.model import OUT, SIZES, build, holdings_sequence

N_CHECK = 20
CLARABEL = dict(tol_gap_abs=1e-10, tol_gap_rel=1e-10, tol_feas=1e-10)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--N", type=int, nargs="+", default=list(SIZES))
    parser.add_argument("--arms", nargs="+", choices=list(PY_ARMS), default=list(PY_ARMS))
    args = parser.parse_args()

    for N in args.N:
        X = holdings_sequence(N)[:N_CHECK]
        reference, x_ref, z_ref = build(N)
        z_star, obj_star = [], []
        for xv in X:
            x_ref.value = xv
            reference.solve(solver=cp.CLARABEL, **CLARABEL)
            z_star.append(z_ref.value.copy())
            obj_star.append(reference.value)

        report = dict(N=N, samples=len(X), arms={})
        for arm in args.arms:
            solve, z = python_arm(N, arm)
            iters, z_err, obj_gap = [], [], []
            for xv, zs, obj in zip(X, z_star, obj_star):
                iters.append(int(solve(xv)))
                z_err.append(float(np.max(np.abs(z.value - zs))))
                # the reference objective at this arm's z (the constraints hold only approximately)
                x_ref.value, z_ref.value = xv, z.value
                obj_gap.append(float(abs(reference.objective.value - obj) / abs(obj)))
            report["arms"][arm] = dict(
                iters_min=min(iters),
                iters_max=max(iters),
                max_abs_z_err=max(z_err),
                max_rel_obj_gap=max(obj_gap),
            )
            print(
                f"N={N} {arm:15s} iters {min(iters)}-{max(iters)}  "
                f"max|z-z*| {max(z_err):.2e}  obj gap {max(obj_gap):.2e}"
            )
        (OUT / f"check_n{N}.json").write_text(json.dumps(report, indent=2))


if __name__ == "__main__":
    main()
