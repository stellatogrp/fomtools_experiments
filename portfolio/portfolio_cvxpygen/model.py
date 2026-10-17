"""The portfolio rebalancing problem as a parametric CVXPY problem, for any N.

The instance is the verified one (``portfolio/results/run_002``: drift 0.15,
z_bar 0.3, the ``conf/experiment/portfolio.yaml`` defaults otherwise) with the
number of assets overridden. The current holdings ``x`` are the CVXPY parameter;
they enter only the epigraph of ``||z - x||_1``, so after canonicalization only
the inequality bounds depend on ``x`` and ``P``, ``A`` stay constant, as the
fomtools CVXPYgen front end requires.
"""

from __future__ import annotations

import importlib
import sys
from pathlib import Path
from types import SimpleNamespace

import cvxpy as cp
import numpy as np

PORTFOLIO = Path(__file__).resolve().parents[1]
ROOT = PORTFOLIO.parent
for path in (PORTFOLIO, ROOT):
    if str(path) not in sys.path:
        sys.path.insert(0, str(path))

from cluster_run import drift_interval, make_problem  # noqa: E402

# Factor-model sweep (branch bs/portfolio-cvxpygen-scale): its own output folders,
# so the dense-covariance results of the paper stay untouched.
OUT = Path(__file__).parent / "out_factor"
RESULTS = Path(__file__).parent / "results_factor"

SIZES = (200, 500, 1000, 3000)
FACTORS = 100  # k: Sigma = F F' + sigma_ridge I with F of size N x k
# Return forecasts vary between solves (Grinold and Kahn's alpha = IC x volatility x
# score): mu_t = mu_bar + IC * sigma * s_t, sigma_i = sqrt(Sigma_ii), s_t ~ N(0, I)
# truncated to [-S_MAX, S_MAX], which also gives the box the front end needs.
IC = 0.1
S_MAX = 3.0
ITERATIONS = 200
N_SOLVES = 100  # back-test length per timing round
DRIFT = 0.15

# portfolio.yaml, with the run_002 overrides (slurm/submit_all.sh)
PROBLEM = dict(
    kappa=1.0,
    lambd=0.01,
    z_bar=0.3,
    rho=1.0,
    sigma=1.0e-4,
    mu_high=0.5,
    sigma_ridge=0.1,
    seed=0,
)

# fomtools ADMM builder defaults (rho 0.1, sigma 1e-6, alpha 1.6, rho_eq_scale
# 1e3), which are OSQP's; the front end Ruiz-equilibrates the canonical QP
# (auto_equilibrate, on by default).
# Factor-model sweep: rho = 1, one step up from OSQP's default 0.1 on a decade
# grid. ``rho_sweep`` on draws held out from the back-test (seed 2): iterations to
# 1e-4 at N = 200 / 1000 are 447 / ~620 at 0.1, 83 / 149 at 1, 78 / 116 at the
# best value 0.7; 1 is used so the result does not rest on a finely tuned rho.
FOM_OPTIONS: dict = dict(rho=1.0)
STEP_SIZES = dict(rho=0.1, sigma=1.0e-6, alpha=1.6)

# OSQP with every extra off: plain ADMM with the same step sizes, a fixed
# number of iterations, a cold start. CVXPY takes all of these.
OSQP_OFF = dict(
    **STEP_SIZES,
    scaling=0,
    adaptive_rho=False,
    polishing=False,
    warm_starting=False,
    max_iter=ITERATIONS,
    check_termination=0,
)
# OSQP with its defaults and its extras on (scaling, adaptive rho, and polishing,
# which is off by default), but run for the same fixed number of iterations as
# every other arm: no termination check, and a cold start. The holdings are
# independent draws, but all lie near the equal-weight portfolio, so warm
# starting from the previous solve would let OSQP skip most of the work.
OSQP_ON = dict(
    polishing=True,
    warm_starting=False,
    warm_start=False,
    max_iter=ITERATIONS,
    check_termination=0,
)

# Stock CVXPYgen: OSQP with its defaults, the same fixed number of iterations,
# a cold start. Its rho, sigma, alpha and scaling are fixed at generation and
# adaptive_rho is not a runtime setting, so these runtime settings are all the
# API reaches. Embedded OSQP does adapt rho, and the adapted rho stays in the
# solver between solves; the Python wrapper resets the settings (rho included)
# before every solve, the C interface does not.
CPG_OSQP = dict(max_iter=ITERATIONS, check_termination=0, warm_start=False)


def instance(N: int):
    """The ``PortfolioProblem`` data (Sigma, mu, ...) for ``N`` assets."""
    return make_problem(SimpleNamespace(n=N, **PROBLEM), ITERATIONS)


def factor_instance(N: int, k: int = FACTORS):
    """Factor-model data for ``N`` assets: ``F`` (N x k, entries N(0, 1/k)), so that
    ``Sigma = F F' + sigma_ridge I`` has unit-scale factor risk as the dense instance,
    and ``mu ~ U[0, mu_high]``; the other constants as in ``PROBLEM``."""
    rng = np.random.default_rng([PROBLEM["seed"], N, k])
    F = rng.normal(scale=1 / np.sqrt(k), size=(N, k))
    mu = rng.uniform(0.0, PROBLEM["mu_high"], size=N)
    return SimpleNamespace(
        F=F,
        d=PROBLEM["sigma_ridge"],
        mu=mu,
        kappa=PROBLEM["kappa"],
        lambd=PROBLEM["lambd"],
        z_bar=PROBLEM["z_bar"] * np.ones(N),
    )


def x_bounds(N: int) -> tuple[np.ndarray, np.ndarray]:
    """Exact per-asset range of the drifted equal-weight portfolio (the verified box)."""
    return drift_interval(np.ones(N) / N, DRIFT)


class Parameters:
    """The holdings ``x`` and the return forecast ``mu`` as one stacked vector
    ``[x; mu]``, so callers set both with ``params.value = v``."""

    def __init__(self, x: cp.Parameter, mu: cp.Parameter):
        self.x, self.mu = x, mu

    @property
    def value(self) -> np.ndarray:
        return np.concatenate([self.x.value, self.mu.value])

    @value.setter
    def value(self, v: np.ndarray) -> None:
        n = self.x.size
        self.x.value, self.mu.value = v[:n], v[n:]


def forecast_scale(N: int) -> tuple[np.ndarray, np.ndarray]:
    """``(mu_bar, IC * sigma)``: the mean forecast and the scale of its updates."""
    data = factor_instance(N)
    sigma = np.sqrt(np.sum(data.F**2, axis=1) + data.d)
    return data.mu, IC * sigma


def param_bounds(N: int, params: Parameters) -> dict:
    """Boxes for the front end: the verified drift box for ``x``, the truncated
    forecast box for ``mu``."""
    mu_bar, scale = forecast_scale(N)
    return {params.x: x_bounds(N), params.mu: (mu_bar - S_MAX * scale, mu_bar + S_MAX * scale)}


def build(N: int):
    """``(problem, params, z)``: the parametric CVXPY problem, its parameters ``[x; mu]``
    and its variable."""
    data = factor_instance(N)
    x = cp.Parameter(N, name="x")
    x.value = np.ones(N) / N
    mu = cp.Parameter(N, name="mu")
    mu.value = data.mu
    z = cp.Variable(N, name="z")
    y = cp.Variable(data.F.shape[1], name="y")  # factor exposures, as in the CVXPYgen paper
    objective = (
        0.5 * data.kappa * (cp.sum_squares(y) + data.d * cp.sum_squares(z))
        - mu @ z
        + data.lambd * cp.norm1(z - x)
    )
    constraints = [y == data.F.T @ z, cp.sum(z) == 1, z >= 0, z <= data.z_bar]
    return cp.Problem(cp.Minimize(objective), constraints), Parameters(x, mu), z


def code_dir(N: int, arm: str) -> str:
    """Generated CVXPYgen directory beneath ``OUT``, which is also its module name."""
    return f"cpg_n{N}_{arm}"


def prefix(N: int, arm: str) -> str:
    """Unique CVXPYgen symbol prefix, so several generated wrappers can share one process."""
    return f"{arm}{N}_"


def load_cpg_solve(N: int, arm: str):
    """The generated ``cpg_solve`` of one solver, for ``problem.register_solve``."""
    if str(OUT) not in sys.path:
        sys.path.insert(0, str(OUT))
    return importlib.import_module(f"{code_dir(N, arm)}.cpg_solver").cpg_solve


def holdings_sequence(N: int, count: int = N_SOLVES, seed: int = 1) -> np.ndarray:
    """``count`` parameter vectors ``[x; mu]`` (shape ``count x 2N``): drifted equal-weight
    holdings ``(1 + r) / sum(1 + r)``, ``r ~ U[-drift, drift]``, and return forecasts
    ``mu_bar + IC * sigma * s``, ``s ~ N(0, I)`` truncated to ``[-S_MAX, S_MAX]``."""
    rng = np.random.default_rng([seed, N])
    growth = 1.0 + rng.uniform(-DRIFT, DRIFT, size=(count, N))
    X = growth / growth.sum(axis=1, keepdims=True)
    mu_bar, scale = forecast_scale(N)
    S = np.clip(rng.standard_normal(size=(count, N)), -S_MAX, S_MAX)
    return np.hstack([X, mu_bar + scale * S])
