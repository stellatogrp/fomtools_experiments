"""Iterations to tolerance of the fomtools ADMM solver as a function of rho.

Run from ``portfolio/`` after nothing else (no code generation, no agent):

    python -m portfolio_cvxpygen.rho_sweep [--N 200] [--rho 0.01 0.1 1] [--samples 20]

Runs the iteration the fomtools ``admm`` builder records
(``fomtools.cvxpygen.builders._admm_step``), in NumPy: the canonical QP from
CVXPYgen, Ruiz-equilibrated as the front end does (``auto_equilibrate``), rho
boosted by ``rho_eq_scale`` on the equality rows, sigma and alpha fixed, a cold
start, and one sparse factorization of the KKT matrix per rho. Every instance of
the back-test sequence runs as one column of a batch. An instance stops at the
first iteration where both unscaled residuals of the canonical problem,
``||Ax - clamp(Ax, l, u)||_inf`` and ``||Px + q + A'y||_inf``, are at most
``--tol``: the stopping test of ``tolerance``. The draws come from the same
distribution as the back-test sequence but another seed (``--seed``, default 2),
so rho is not tuned on the instances it is timed on. Prints, per rho, the mean and
largest iteration count; ``--check`` compares the counts at one rho with those
of the generated translated solver (``out_factor/cpg_n{N}_fom``, built at rho 0.1).
"""

from __future__ import annotations

import argparse

import numpy as np
import scipy.sparse as spa
import scipy.sparse.linalg as spla

from portfolio_cvxpygen.model import build, holdings_sequence, param_bounds

SIGMA, ALPHA, RHO_EQ_SCALE = 1e-6, 1.6, 1e3  # the admm builder defaults (OSQP's)
CAP = 20000


def canonical(N: int):
    """``(qp, scaling, maps)``: the canonical QP, its Ruiz scaling, and the affine maps
    ``q = Mq p``, ``u = Mu p`` from cvxpygen's flat parameter vector ``p``."""
    from cvxpygen.canonicalizer import Canonicalizer
    from fomtools.cvxpygen.qpdata import extract_qp_data
    from fomtools.cvxpygen.scaling import ruiz_for

    problem, params, _ = build(N)
    canon, _ = Canonicalizer(solver="OSQP").canonicalize(problem)
    qp = extract_qp_data(canon, param_bounds=param_bounds(N, params))
    pc, pi = canon.parameter_canon, canon.parameter_info
    cols = {pi.col_to_name_usp[c]: c for c in pi.col_to_name_usp}
    maps = dict(
        q=spa.csr_matrix(pc.p_id_to_mapping["q"]),
        u=spa.csr_matrix(pc.p_id_to_mapping["u"]),
        size=len(pi.flat_usp),
        x=cols["x"],
        mu=cols["mu"],
    )
    return qp, ruiz_for(qp), maps


def instance_data(qp, maps, V: np.ndarray):
    """Canonical ``q`` (n x B) and ``u`` (m x B) for the stacked ``[x; mu]`` columns of ``V``."""
    N = V.shape[1] // 2
    p = np.zeros((maps["size"], V.shape[0]))
    p[maps["x"] : maps["x"] + N] = V[:, :N].T
    p[maps["mu"] : maps["mu"] + N] = V[:, N:].T
    p[-1] = 1.0
    u = np.clip(maps["u"] @ p, -1e30, 1e30)
    return maps["q"] @ p, u


def iterations(qp, s, maps, V: np.ndarray, rho: float, tol: float) -> np.ndarray:
    """Iterations to ``tol`` for each row of ``V`` (``CAP + 1`` if not reached)."""
    n, m, n_eq = qp.n, qp.m, qp.n_eq
    q, u = instance_data(qp, maps, V)
    l = np.where(np.arange(m)[:, None] < n_eq, u, -1e30)
    # the equilibrated data the generated solver iterates on
    D, E, c = s.D, s.E, s.c
    Ps = spa.csc_matrix(c * (spa.diags(D) @ qp.P @ spa.diags(D)))
    As = spa.csc_matrix(spa.diags(E) @ qp.A @ spa.diags(D))
    qs = c * D[:, None] * q
    us = np.where(np.abs(u) >= 1e30, u, E[:, None] * u)  # rows: b (eq), u_ineq
    ls = np.where(np.arange(m)[:, None] < n_eq, us, -1e30)

    rho_vec = np.full(m, rho)
    rho_vec[:n_eq] *= RHO_EQ_SCALE
    kkt = spa.bmat([[Ps + SIGMA * spa.eye(n), As.T], [As, -spa.diags(1 / rho_vec)]], format="csc")
    lu = spla.splu(kkt)

    B = V.shape[0]
    x, z, y = np.zeros((n, B)), np.zeros((m, B)), np.zeros((m, B))
    done = np.full(B, CAP + 1)
    r = rho_vec[:, None]
    for k in range(1, CAP + 1):
        sol = lu.solve(np.vstack([SIGMA * x - qs, z - y / r]))
        x_t, nu = sol[:n], sol[n:]
        z_t = z + (nu - y) / r
        w = ALPHA * z_t + (1 - ALPHA) * z
        z_new = np.clip(w + y / r, ls, us)
        x = ALPHA * x_t + (1 - ALPHA) * x
        y = y + r * (w - z_new)
        z = z_new
        # unscaled residuals of the canonical problem, as the bridge reports them
        xo, yo = D[:, None] * x, E[:, None] * y / c
        Ax = qp.A @ xo
        pri = np.max(np.maximum(np.maximum(Ax - u, l - Ax), 0), axis=0)
        dua = np.max(np.abs(qp.P @ xo + q + qp.A.T @ yo), axis=0)
        newly = (done > CAP) & (pri <= tol) & (dua <= tol)
        done[newly] = k
        if np.all(done <= CAP):
            break
    return done


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--N", type=int, nargs="+", default=[200])
    parser.add_argument("--rho", type=float, nargs="+",
                        default=[0.01, 0.03, 0.1, 0.3, 1.0, 3.0, 10.0])
    parser.add_argument("--samples", type=int, default=20)
    parser.add_argument("--tol", type=float, default=1e-4)
    parser.add_argument("--seed", type=int, default=2,
                        help="draws for tuning; the back-test sequence uses seed 1")
    parser.add_argument("--check", action="store_true",
                        help="compare rho = 0.1 with the generated translated solver")
    args = parser.parse_args()

    for N in args.N:
        qp, s, maps = canonical(N)
        V = holdings_sequence(N, count=args.samples, seed=args.seed)
        for rho in args.rho:
            its = iterations(qp, s, maps, V, rho, args.tol)
            capped = int(np.sum(its > CAP))
            print(f"N={N} rho={rho:<6g} mean {its.mean():8.1f}  max {its.max():6d}"
                  + (f"  ({capped} not converged by {CAP})" if capped else ""), flush=True)
        if args.check:
            from portfolio_cvxpygen.tolerance import iterations_to_tol, python_arm

            mine = iterations(qp, s, maps, V[:3], 0.1, args.tol)
            solve = python_arm(N, "cpg_fom")
            gen = [iterations_to_tol(solve, v, args.tol) for v in V[:3]]
            print(f"N={N} check at rho 0.1: numpy {list(mine)} vs generated {gen}")


if __name__ == "__main__":
    main()
