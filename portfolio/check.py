"""Check the ADMM of ``problem.py`` against CVXPY on sampled parameters.

    python check.py [--n 10] [--k 2] [--samples 20] [--iters 5000]

For samples of ``p = (x, mu)`` from the parameter set, runs ADMM from the
balanced warm start and compares the holdings ``h`` with a Clarabel solve of
the CVXPYgen comparison's problem (``portfolio_cvxpygen.model.build``, written
with ``norm1`` and the exposures, independently of the epigraph form here). Also
checks that the warm start is a fixed point at the balanced parameter, and that
the simulator backend reproduces the NumPy residuals.
"""

from __future__ import annotations

import argparse

import cvxpy as cp
import numpy as np
from fomtools.verifier import Verifier
from omegaconf import OmegaConf

from portfolio_cvxpygen.model import build
from problem import (
    admm_residuals,
    admm_run,
    admm_step,
    balanced_warm_start,
    make_problem,
    sample_simplex,
)

CONF = OmegaConf.load(__file__.replace("portfolio/check.py", "conf/experiment/portfolio.yaml"))


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--n", type=int, default=int(CONF.problem.n))
    parser.add_argument("--k", type=int, default=int(CONF.problem.factors))
    parser.add_argument("--samples", type=int, default=20)
    parser.add_argument("--iters", type=int, default=5000)
    args = parser.parse_args()

    problem = make_problem(args.n, args.k)
    n = problem.n
    rng = np.random.default_rng(123)
    print(f"n = {n}, k = {problem.k}, x in [{problem.x_lb.min():.4f}, {problem.x_ub.max():.4f}], "
          f"mu in [{problem.mu_lb.min():.3f}, {problem.mu_ub.max():.3f}]")

    balanced = np.concatenate([np.ones(n) / n, problem.mu_bar])[None]
    print(f"warm start fixed-point residual at the balanced parameter: "
          f"{admm_residuals(problem, balanced, 5).max():.1e}")

    cvx, params, z = build(n, problem.k)
    for name, sampler in (("simplex", sample_simplex),):
        Pm = sampler(problem, args.samples, rng)
        s, res = admm_run(problem, Pm, args.iters)
        h_ref = []
        for p in Pm:
            params.value = p
            cvx.solve(solver=cp.CLARABEL, tol_gap_abs=1e-11, tol_gap_rel=1e-11, tol_feas=1e-11)
            h_ref.append(z.value.copy())
        err = np.abs(s[:, :n] - np.array(h_ref)).max()
        print(f"{name:8s} max |h_admm - h_cvxpy| = {err:.1e} after {args.iters} iterations "
              f"(residual {res[:, -1].max():.1e}; at K = 100: {res[:, 99].max():.1e})")

        vp = Verifier(method="simulator", obbt_solver=None)
        s0 = balanced_warm_start(problem)
        p = vp.add_param(2 * n, samples=Pm)
        traj = vp.iterate_trajectory(11)
        traj[0] = vp.add_iterate(s0.size, init=s0)
        for k in range(1, 11):
            traj[k] = admm_step(vp, problem, p, traj[k - 1])
            vp.set_objective_norm([problem.H @ (traj[k] - traj[k - 1])], ord=np.inf)
        sim = np.asarray(vp.solve(), dtype=float)[:, :10]
        print(f"{name:8s} simulator vs NumPy residuals, K = 1..10: {np.abs(sim - res[:, :10]).max():.1e}")


if __name__ == "__main__":
    main()
