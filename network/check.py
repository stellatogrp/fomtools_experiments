"""Sanity checks for the network experiment, all in the original (unscaled) variables.

1. ``H(k)(ŝ^k - ŝ^{k-1})`` is the KKT residual of the original LP: the demand
   rows equal ``A_d z^k - x`` exactly, and the flow and supply rows equal the
   Lagrangian gradient wherever the normal cone is ``{0}``.
2. Every variant converges to the CVXPY (Clarabel) optimum of the original LP,
   after mapping back with ``z = D ẑ``, ``v = E_s v̂``, ``w = E_d ŵ``.
3. Every vertex of the demand box is feasible.
4. If ``results/bounds_mipinf.csv`` exists: sampled residual <= MIP bound.
"""

from __future__ import annotations

import csv
import itertools
from collections import defaultdict
from pathlib import Path

import numpy as np

from problem import kkt_residual, make_problem, pdhg_residuals, pdhg_run, solve_lp, variant

RESULTS = Path(__file__).parent / "results"
VARIANTS = [(eq, mom) for mom in (False, True) for eq in (False, True)]
TOL = 1e-4


def check_residual_map() -> None:
    p = make_problem()
    X = np.random.default_rng(3).uniform(p.x_lb, p.x_ub, size=(20, p.nd))
    for eq, mom in VARIANTS:
        v = variant(p, eq, mom)
        for K in range(1, 16):
            (z1, y1, w1), _ = pdhg_run(v, X, K, residuals=False)
            (z0, y0, w0), _ = pdhg_run(v, X, K - 1, residuals=False)
            r = np.hstack([z1 - z0, y1 - y0, w1 - w0]) @ v.H(K).T
            for n, x in enumerate(X):
                z, y, w = v.unscale(z1[n], y1[n], w1[n])
                r_z, r_v, r_w = np.split(r[n], [p.ne, p.ne + p.ns])
                assert np.allclose(r_w, p.A_d @ z - x, atol=1e-9), (v.name, K)
                grad = p.mu + p.A_s.T @ y - p.A_d.T @ w
                inner = (z > 1e-9) & (z < p.c - 1e-9)
                assert np.allclose(r_z[inner], grad[inner], atol=1e-9), (v.name, K)
                pos = y > 1e-9
                assert np.allclose(r_v[pos], (p.b_s - p.A_s @ z)[pos], atol=1e-9), (v.name, K)
    print("residual map: H(k)(s^k - s^{k-1}) is the original KKT residual for every variant")


def check_convergence() -> None:
    p = make_problem()
    X = np.random.default_rng(1).uniform(p.x_lb, p.x_ub, size=(20, p.nd))
    for eq, mom in VARIANTS:
        v = variant(p, eq, mom)
        (z, y, w), _ = pdhg_run(v, X, 200_000, residuals=False)
        for n, x in enumerate(X):
            zo, yo, wo = v.unscale(z[n], y[n], w[n])
            _, _, _, opt = solve_lp(p, x)
            assert abs(p.mu @ zo - opt) <= 1e-5 * (1 + abs(opt)), (v.name, p.mu @ zo, opt)
            assert kkt_residual(p, zo, yo, wo, x) < 1e-6, (v.name, kkt_residual(p, zo, yo, wo, x))
    print("convergence: every variant reaches the CVXPY (Clarabel) optimum in original variables")


def check_feasibility() -> None:
    p = make_problem()
    for corner in itertools.product(*zip(p.x_lb, p.x_ub)):
        solve_lp(p, np.array(corner))
    print(f"feasibility: all {2**p.nd} vertices of the demand box are feasible")


def check_bounds() -> None:
    path = RESULTS / "bounds_mipinf.csv"
    if not path.exists():
        return
    p = make_problem()
    values = defaultdict(dict)
    with open(path) as f:
        for row in csv.DictReader(f):
            values[row["variant"], row["backend"]][int(row["K"])] = float(row["value"])
    X = np.random.default_rng(2).uniform(p.x_lb, p.x_ub, size=(10_000, p.nd))
    for eq, mom in VARIANTS:
        v = variant(p, eq, mom)
        K = max(values[v.name, "mip"])
        sampled = pdhg_residuals(v, X, K).max(axis=0)
        for k in range(1, K + 1):
            mip, sim = values[v.name, "mip"][k], values[v.name, "simulator"][k]
            assert max(sampled[k - 1], sim) <= mip * (1 + TOL), (
                v.name,
                k,
                sampled[k - 1],
                sim,
                mip,
            )
    print("bounds: sampled <= MIP for every variant and K")


if __name__ == "__main__":
    check_residual_map()
    check_feasibility()
    check_convergence()
    check_bounds()
