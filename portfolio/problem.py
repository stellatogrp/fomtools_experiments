"""Portfolio rebalancing with an l1 trading cost, and OSQP's ADMM on it.

The problem is the CVXPYgen comparison's (``portfolio_cvxpygen/model.py``, the
single source of its constants), at a size the MIP backend can verify:

    minimize    (kappa/2) (||f||^2 + d ||h||^2) - mu' h + lambd ||h - x||_1
    subject to  f = F' h,  1' h = 1,  0 <= h <= z_bar,

the factor model ``Sigma = F F' + d I`` written with the exposures ``f``, or, with
``exposures=False``, as the dense ``(kappa/2) h' Sigma h`` (no ``f``, no factor rows). The
parameters are the held portfolio ``x`` and the return forecast ``mu = mu_bar +
G theta``, stacked as ``p = (x, theta)``. ``x`` is the equal-weight portfolio
drifted by one period of returns in ``[-DRIFT, DRIFT]``: each coordinate lies in
``drift_interval`` and ``1' x = 1``. The forecast model ``MU_MODELS`` sets ``G``
and the set of ``theta``:

    box       G = I, theta in +- s_max IC sigma (``model.mu_bounds``; s_max = S_MAX)
    neutral   the same box, and 1' theta = 0 (cross-sectionally neutral updates)
    factor    G = F, theta in +- s_max IC in R^k (forecasts move with the factors)
    fixed     G = I, theta = 0: mu = mu_bar

The l1 term is written with an epigraph variable ``tau >= |h - x|``, so the
optimization variable is ``y = (h, tau, f)`` and the constraint rows are

    budget     1' h          = 1
    factor     F' h - f      = 0
    position   0 <= h        <= z_bar
    sell       h - tau       <= x
    buy        h + tau       >= x

``x`` enters only the one-sided bounds of the sell and buy rows, whose
projections are ``min(v, x) = x - relu(x - v)`` and ``max(v, x) = x + relu(v -
x)``; ``mu`` enters only the linear cost ``q(mu) = (-mu, lambd 1, 0)``.

ADMM is OSQP's with ``model.ADMM``'s rho, sigma and relaxation alpha, in the
variables ``s = (y, v)``. ``R`` is the diagonal of per-row rho. The CVXPYgen
comparison boosts rho by ``rho_eq_scale = 1e3`` on the equality rows; here
``EQ_SCALE = 1`` (see below).

    w      = Pi_[l(x), u(x)](v)
    y~     = (P + sigma I + M' R M)^{-1} (sigma y - q(mu) + M' R (2 w - v))
    y+     = alpha y~ + (1 - alpha) y
    v+     = v + alpha (M y~ - w)

With ``lam = R (v - w)``, ``(y~, w, lam)`` satisfy the normal cone conditions, and
the remaining primal and dual residuals ``M y~ - w`` and ``P y~ + q + M' lam`` are
``H (s+ - s)`` with ``H = -(1/alpha) [[sigma I, M' R], [0, I]]``. At ``alpha = 1``
and a scalar rho this is the paper's metric. The front end of the CVXPYgen
comparison also Ruiz-equilibrates its canonical QP; this iteration runs on the
problem as written.

ADMM is warm-started at the primal-dual solution for the balanced portfolio
``x = 1/n`` and the mean forecast ``mu = mu_bar`` (``theta = 0``), computed with CVXPY, the same
point for every parameter.
"""

from __future__ import annotations

from dataclasses import dataclass

import cvxpy as cp
import numpy as np
import scipy.sparse as spa
import scipy.sparse.linalg  # noqa: F401
from fomtools.linearexpression import stack

from portfolio_cvxpygen.model import (
    ADMM,
    DRIFT,
    IC,
    S_MAX,
    drift_interval,
    factor_instance,
    forecast_scale,
)

EQUALITY = ("budget", "factor")
# rho boost on the equality rows. The fomtools simulator runs in float32, and H
# multiplies the equality rows of v by R: with OSQP's 1e3 the float32 rounding of
# v reaches 3e-4 in the residual, as large as the residual itself at K = 100.
# Without the boost the simulator matches NumPy to 3e-7, and ADMM converges as
# fast (worst residual over 1000 simplex samples at K = 100: 1.1e-3 vs 0.9e-3).
EQ_SCALE = 1.0
MU_MODELS = ("box", "neutral", "factor", "fixed")


@dataclass(frozen=True)
class PortfolioProblem:
    F: np.ndarray
    d: float
    mu_bar: np.ndarray
    kappa: float
    lambd: float
    z_bar: np.ndarray
    rho: float
    sigma: float
    alpha: float
    rho_eq_scale: float
    x_lb: np.ndarray
    x_ub: np.ndarray
    G: np.ndarray  # mu = mu_bar + G theta
    th_lb: np.ndarray
    th_ub: np.ndarray
    neutral: bool  # 1' theta = 0
    exposures: bool  # variables f = F'h (True) or the dense Sigma = F F' + d I (False)

    @property
    def n(self) -> int:
        return int(self.F.shape[0])

    @property
    def k(self) -> int:
        return int(self.F.shape[1])

    @property
    def kx(self) -> int:
        """Number of exposure variables and rows: ``k``, or 0 with the dense ``Sigma``."""
        return self.k if self.exposures else 0

    @property
    def Sigma(self) -> np.ndarray:
        return self.F @ self.F.T + self.d * np.eye(self.n)

    @property
    def p_lb(self) -> np.ndarray:
        return np.concatenate([self.x_lb, self.th_lb])

    @property
    def p_ub(self) -> np.ndarray:
        return np.concatenate([self.x_ub, self.th_ub])

    @property
    def p_dim(self) -> int:
        return self.n + int(self.G.shape[1])

    def mu(self, Theta: np.ndarray) -> np.ndarray:
        """Forecasts for the rows of ``Theta``."""
        return self.mu_bar + Theta @ self.G.T

    @property
    def P(self) -> spa.csc_matrix:
        """Hessian in ``y = (h, tau, f)``, or ``(h, tau)`` with the dense ``Sigma``."""
        n, k = self.n, self.kx
        if not self.exposures:
            return spa.block_diag([self.kappa * self.Sigma, spa.csc_matrix((n, n))], "csc")
        return spa.block_diag(
            [self.kappa * self.d * spa.eye(n), spa.csc_matrix((n, n)), self.kappa * spa.eye(k)],
            "csc",
        )

    @property
    def q0(self) -> np.ndarray:
        """``q(mu) = q0 - E mu``."""
        return np.concatenate([np.zeros(self.n), self.lambd * np.ones(self.n), np.zeros(self.kx)])

    @property
    def E(self) -> spa.csc_matrix:
        n, k = self.n, self.kx
        return spa.vstack([spa.eye(n), spa.csc_matrix((n + k, n))], "csc")

    @property
    def blocks(self) -> dict[str, spa.csc_matrix]:
        """Constraint rows in the order budget, factor (with exposures), position, sell, buy."""
        n, k, eye = self.n, self.kx, spa.eye(self.n)
        Znk, Znn = spa.csc_matrix((n, k)), spa.csc_matrix((n, n))
        blocks = {"budget": spa.hstack([np.ones((1, n)), spa.csc_matrix((1, n + k))], "csc")}
        if self.exposures:
            blocks["factor"] = spa.hstack(
                [spa.csc_matrix(self.F.T), spa.csc_matrix((k, n)), -spa.eye(k)], "csc"
            )
        blocks["position"] = spa.hstack([eye, Znn, Znk], "csc")
        blocks["sell"] = spa.hstack([eye, -eye, Znk], "csc")
        blocks["buy"] = spa.hstack([eye, eye, Znk], "csc")
        return blocks

    @property
    def block_rho(self) -> dict[str, float]:
        return {b: self.rho * (self.rho_eq_scale if b in EQUALITY else 1.0) for b in self.blocks}

    @property
    def M(self) -> spa.csc_matrix:
        return spa.vstack(list(self.blocks.values()), "csc")

    @property
    def R(self) -> spa.dia_matrix:
        return spa.diags(
            np.concatenate([np.full(B.shape[0], self.block_rho[b]) for b, B in self.blocks.items()])
        )

    @property
    def lhs(self) -> spa.csc_matrix:
        M = self.M
        return (self.P + self.sigma * spa.eye(M.shape[1]) + M.T @ self.R @ M).tocsc()

    @property
    def H(self) -> spa.csc_matrix:
        """Map from ``s^k - s^{k-1}`` to the primal-dual residual, ``s = (y, v)``."""
        M = self.M
        m, d = M.shape
        H = spa.bmat([[self.sigma * spa.eye(d), M.T @ self.R], [None, spa.eye(m)]], "csc")
        return -H / self.alpha


def make_problem(
    n: int,
    k: int,
    mu_model: str = "box",
    s_max: float = S_MAX,
    exposures: bool = True,
    sigma: float = ADMM["sigma"],
    alpha: float = ADMM["alpha"],
) -> PortfolioProblem:
    """The factor-model instance of ``model.factor_instance(n, k)`` with the ADMM of ``model.ADMM``
    (equality rows boosted by ``EQ_SCALE``) and the forecast model ``mu_model``."""
    data = factor_instance(n, k)
    x_lb, x_ub = drift_interval(np.full(n, 1.0 / n), DRIFT)
    if mu_model == "factor":
        G, th = data.F, s_max * IC * np.ones(k)
    elif mu_model in ("box", "neutral", "fixed"):
        G, th = np.eye(n), s_max * forecast_scale(n, k)[1] * (mu_model != "fixed")
    else:
        raise ValueError(f"mu_model {mu_model!r} not in {MU_MODELS}")
    return PortfolioProblem(
        F=data.F,
        d=float(data.d),
        mu_bar=data.mu,
        kappa=float(data.kappa),
        lambd=float(data.lambd),
        z_bar=data.z_bar,
        x_lb=np.maximum(x_lb, 0.0),
        x_ub=np.minimum(x_ub, data.z_bar),
        G=G,
        th_lb=-th,
        th_ub=th,
        neutral=mu_model == "neutral",
        exposures=exposures,
        rho=ADMM["rho"],
        sigma=sigma,
        alpha=alpha,
        rho_eq_scale=EQ_SCALE,
    )


def solve_cvxpy(problem: PortfolioProblem, x: np.ndarray, mu: np.ndarray):
    """``(y, dual)``: a primal-dual solution of the epigraph form, with ``dual`` the
    multiplier of the rows ``l <= M y <= u`` (positive where the upper bound binds)."""
    n, k = problem.n, problem.k
    h, tau, f = cp.Variable(n), cp.Variable(n), cp.Variable(k)
    budget = cp.sum(h) == 1
    factor = problem.F.T @ h - f == 0
    pos_lo, pos_hi = h >= 0, h <= problem.z_bar
    sell, buy = h - tau <= x, h + tau >= x
    objective = (
        problem.kappa / 2 * (cp.sum_squares(f) + problem.d * cp.sum_squares(h))
        - mu @ h
        + problem.lambd * cp.sum(tau)
    )
    cp.Problem(cp.Minimize(objective), [budget, factor, pos_lo, pos_hi, sell, buy]).solve(
        solver="CLARABEL", tol_gap_abs=1e-12, tol_gap_rel=1e-12, tol_feas=1e-12
    )
    y = np.concatenate([h.value, tau.value, f.value][: 3 if problem.exposures else 2])
    # Inequality multipliers have a fixed sign convention; the equality ones are
    # recovered from stationarity, P y + q + M' dual = 0, rather than from CVXPY's
    # convention for equality duals.
    ineq = np.concatenate([pos_hi.dual_value - pos_lo.dual_value, sell.dual_value, -buy.dual_value])
    B = problem.blocks
    M_eq = spa.vstack([B[b] for b in EQUALITY if b in B]).toarray()
    M_in = spa.vstack([B["position"], B["sell"], B["buy"]])
    grad = problem.P @ y + problem.q0 - problem.E @ mu + M_in.T @ ineq
    eq = np.linalg.lstsq(M_eq.T, -grad, rcond=None)[0]
    dual = np.concatenate([eq, ineq])
    assert np.abs(problem.P @ y + problem.q0 - problem.E @ mu + problem.M.T @ dual).max() < 1e-7
    return y, dual


def balanced_warm_start(problem: PortfolioProblem) -> np.ndarray:
    """ADMM state ``s^0 = (y, v)`` at the primal-dual solution for ``x = 1/n``, ``mu = mu_bar``.

    With ``w = M y`` and the dual ``dual`` of the rows ``l <= M y <= u``, the ADMM
    fixed point has ``v = w + R^{-1} dual``, since ADMM recovers the dual as
    ``R (v - w)``.
    """
    y, dual = solve_cvxpy(problem, np.ones(problem.n) / problem.n, problem.mu_bar)
    return np.concatenate([y, problem.M @ y + dual / problem.R.diagonal()])


def split(problem: PortfolioProblem, p):
    """``(x, theta)`` from the stacked parameter (rows of an array, or an expression)."""
    n = problem.n
    if isinstance(p, np.ndarray):
        return p[..., :n], p[..., n:]
    return p[:n], p[n:]


def admm_step(vp, problem: PortfolioProblem, p, s):
    """One ADMM iteration recorded on the verifier ``vp``: ``s^k`` from ``s^{k-1}``."""
    n, k = problem.n, problem.kx
    d = 2 * n + k
    B, rho = problem.blocks, problem.block_rho
    alpha = problem.alpha
    x, theta = split(problem, p)
    y, v = s[:d], s[d:]
    v_budget, v_factor = v[:1], v[1 : 1 + k]
    o = 1 + k
    v_pos, v_sell, v_buy = v[o : o + n], v[o + n : o + 2 * n], v[o + 2 * n :]

    # w = projection of v onto [l(x), u(x)], block by block.
    w_pos = vp.step.satlin(v_pos, l=np.zeros(n), u=problem.z_bar)
    w_sell = x - vp.step.relu(x - v_sell)  # min(v, x)
    w_buy = x + vp.step.relu(v_buy - x)  # max(v, x)

    rhs = (
        problem.sigma * y
        - problem.q0
        + problem.E @ problem.mu_bar
        + spa.csc_matrix(problem.E @ problem.G) @ theta
        + rho["budget"] * B["budget"].T @ (2.0 - v_budget)
        + rho["position"] * B["position"].T @ (2 * w_pos - v_pos)
        + rho["sell"] * B["sell"].T @ (2 * w_sell - v_sell)
        + rho["buy"] * B["buy"].T @ (2 * w_buy - v_buy)
    )
    if k:
        rhs = rhs - rho["factor"] * B["factor"].T @ v_factor
    y_tilde = vp.step.implicit_linear(rhs, output_mat=problem.lhs)
    v_next = stack(
        [v_budget + alpha * (B["budget"] @ y_tilde - 1.0)]
        + ([v_factor + alpha * (B["factor"] @ y_tilde)] if k else [])
        + [
            v_pos + alpha * (B["position"] @ y_tilde - w_pos),
            v_sell + alpha * (B["sell"] @ y_tilde - w_sell),
            v_buy + alpha * (B["buy"] @ y_tilde - w_buy),
        ]
    )
    return stack([alpha * y_tilde + (1 - alpha) * y, v_next])


def project(problem: PortfolioProblem, V: np.ndarray, X: np.ndarray) -> np.ndarray:
    """Project the rows of ``V`` onto ``[l(x), u(x)]`` for the matching rows of ``X``."""
    n, k = problem.n, problem.kx
    o = 1 + k
    return np.hstack(
        [
            np.ones((V.shape[0], 1)),
            np.zeros((V.shape[0], k)),
            np.clip(V[:, o : o + n], 0.0, problem.z_bar),
            np.minimum(V[:, o + n : o + 2 * n], X),
            np.maximum(V[:, o + 2 * n :], X),
        ]
    )


def admm_run(problem: PortfolioProblem, Pm: np.ndarray, K: int, s0: np.ndarray | None = None):
    """Run ADMM on the parameter rows ``Pm = [x, mu]``; return the final states and
    ``||H(s^k - s^{k-1})||_inf`` with shape ``(len(Pm), K)``."""
    M, R, alpha, H = problem.M, problem.R, problem.alpha, problem.H
    d = M.shape[1]
    X, Theta = split(problem, Pm)
    MU = problem.mu(Theta)
    lhs = spa.linalg.splu(problem.lhs)
    s = np.tile(balanced_warm_start(problem) if s0 is None else s0, (Pm.shape[0], 1))
    q = problem.q0[None, :] - MU @ problem.E.T
    out = np.empty((Pm.shape[0], K))
    for k in range(K):
        y, v = s[:, :d], s[:, d:]
        w = project(problem, v, X)
        rhs = problem.sigma * y - q + (2 * w - v) @ R @ M
        y_tilde = lhs.solve(np.ascontiguousarray(rhs.T)).T
        s_next = np.hstack([alpha * y_tilde + (1 - alpha) * y, v + alpha * (y_tilde @ M.T - w)])
        out[:, k] = np.abs((s_next - s) @ H.T).max(axis=1)
        s = s_next
    return s, out


def admm_residuals(problem: PortfolioProblem, Pm: np.ndarray, K: int) -> np.ndarray:
    return admm_run(problem, Pm, K)[1]


def sample_hyperplane(lb, ub, total, size: int, rng: np.random.Generator) -> np.ndarray:
    """Uniform samples of ``{u : 1'u = total, lb <= u <= ub}``.

    Draw the first ``m - 1`` coordinates uniformly from their intervals and set the
    last to ``total - sum``; keep the draws whose last coordinate lands in its
    interval. The map to the first ``m - 1`` coordinates is linear and one-to-one
    on the hyperplane, so the kept points are uniform on the polytope.
    """
    m, out, kept = lb.size, [], 0
    while kept < size:
        batch = max(2 * (size - kept), 1024)
        head = rng.uniform(lb[:-1], ub[:-1], size=(batch, m - 1))
        last = total - head.sum(axis=1)
        ok = (last >= lb[-1]) & (last <= ub[-1])
        out.append(np.column_stack([head[ok], last[ok]]))
        kept += int(ok.sum())
    return np.vstack(out)[:size]


def sample_simplex(problem: PortfolioProblem, size: int, rng: np.random.Generator) -> np.ndarray:
    """Uniform samples of ``p = (x, theta)``: ``x`` on ``{1'x = 1, x_lb <= x <= x_ub}``,
    ``theta`` on its box (and ``1' theta = 0`` when ``neutral``)."""
    X = sample_hyperplane(problem.x_lb, problem.x_ub, 1.0, size, rng)
    if problem.neutral:
        Theta = sample_hyperplane(problem.th_lb, problem.th_ub, 0.0, size, rng)
    else:
        Theta = rng.uniform(problem.th_lb, problem.th_ub, size=(size, problem.th_lb.size))
    return np.hstack([X, Theta])
