"""Why CVXPYgen + OSQP needs more iterations: OSQP's Ruiz scaling, fixed at generation.

Run from ``portfolio/`` (no code generation):

    python -m portfolio_cvxpygen.scaling_check [--N 200 500]

OSQP (Python, ``osqp``) on the canonical QP of ``rho_sweep.canonical``, with the
stopping test and fixed rho of ``tolerance_stop``, over the back-test draws, three ways:

``per-draw``  set up on each draw: Ruiz scaling from that draw's ``q``, ``l``, ``u``,
              as CVXPY sets OSQP up on every cold solve;
``nominal``   set up once on the parameter values at generation (``x = 1/N``,
              ``mu = mu_bar``), then ``update(q, l, u)`` per draw: the scaling stays
              that of the nominal data, as in CVXPYgen's embedded OSQP;
``none``      ``scaling = 0``.

Prints the mean and largest iteration count of each.
"""

from __future__ import annotations

import argparse

import numpy as np
import osqp
import scipy.sparse as spa

from portfolio_cvxpygen.model import SIZES, factor_instance, holdings_sequence
from portfolio_cvxpygen.rho_sweep import canonical, instance_data
from portfolio_cvxpygen.tolerance_stop import CVXPY_OSQP

# tolerance_stop's OSQP settings; warm_start is CVXPY's name for warm_starting.
SETTINGS = dict({k: v for k, v in CVXPY_OSQP.items() if k != "warm_start"}, verbose=False)


def iterations(N: int) -> dict[str, list[int]]:
    qp, _, maps = canonical(N)
    P, A = spa.triu(qp.P, format="csc"), qp.A.tocsc()

    def bounds(u):  # osqp takes infinities, not cvxpygen's 1e30
        u = np.where(u >= 1e30, np.inf, u)
        return np.where(np.arange(qp.m) < qp.n_eq, u, -np.inf), u

    nominal = np.concatenate([np.ones(N) / N, factor_instance(N).mu])[None]
    qn, un = (a[:, 0] for a in instance_data(qp, maps, nominal))
    # contiguous columns: osqp.update reads a strided view's memory as if contiguous
    Q, U = (np.asfortranarray(a) for a in instance_data(qp, maps, holdings_sequence(N)))
    fixed = osqp.OSQP()
    fixed.setup(P, qn, A, *bounds(un), scaling=10, **SETTINGS)
    found = {"per-draw": [], "nominal": [], "none": []}
    for b in range(Q.shape[1]):
        for name in found:
            if name == "nominal":
                solver = fixed
                solver.update(q=Q[:, b], l=bounds(U[:, b])[0], u=bounds(U[:, b])[1])
            else:
                solver = osqp.OSQP()
                solver.setup(P, Q[:, b], A, *bounds(U[:, b]), scaling=10 if name == "per-draw" else 0,
                             **SETTINGS)
            result = solver.solve()
            assert result.info.status == "solved", (N, name, b, result.info.status)
            found[name].append(int(result.info.iter))
    return found


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--N", type=int, nargs="+", default=list(SIZES))
    args = parser.parse_args()
    for N in args.N:
        found = iterations(N)
        print(f"N={N}  " + "  ".join(f"{k} {np.mean(v):.1f} (max {max(v)})" for k, v in found.items()),
              flush=True)


if __name__ == "__main__":
    main()
