"""The paper's large-scale Lasso instance and its ISTA and FISTA recordings.

The instance of ``results/rect10k40k_dense_sepA/`` (same RNG calls, so ``D`` and ``x``
are identical):

    min_z  0.5 ||D z - x||^2 + lambda ||z||_1,   D in R^{10,000 x 40,000}

with 260 nonzero slots per row of ``D`` (seeded column permutations, Gaussian values
of standard deviation 1/sqrt(260), duplicates summed), seed 0, lambda 0.02, step
size 1 / (||D||_1 ||D||_inf), initial iterate 2 * 1, and the parameter ``x`` in the
box [b - 1, b + 1].

Each iteration is recorded as two steps, so extraction keeps ``D`` and ``D^T``
separate instead of forming the 34%-dense ``I - eta D^T D``:

    r      = D w - x                     (a soft-threshold at 0: exactly the identity)
    s^{+}  = soft(w - eta D^T r, lambda eta)

with ``w = s`` for ISTA and, for FISTA, the paper's eq. (8): ``w^0 = s^0`` and
``w^t = s^t + gamma_t (s^t - s^{t-1})`` with ``gamma_t = (beta_{t-1} - 1) / beta_t``,
``beta_0 = 1``, so the coefficients are 0, 0, gamma_2, gamma_3, ... The archived run
used ``gamma_{t+1}`` at iteration ``t`` (one iteration early); `check_fista` holds the
recording to a textbook FISTA.

The size can be shrunk for smoke tests with ``LASSO_CODEGEN_M``, ``_N`` and ``_K``
(``M * K`` must be a multiple of ``N``); every module reads it through `size`.
"""

from __future__ import annotations

import os
from dataclasses import dataclass

import numpy as np
import scipy.sparse as spa

#: The paper's instance.
M, N, K = 10_000, 40_000, 260
SEED = 0
LAMBDA = 0.02
ITERATIONS = 200
#: Verification tolerance of the generated solvers (the paper's setting).
TOL_VERIFY = 1e-5

#: Materializing step: soft-threshold at 0 is exactly the identity, and unlike a
#: saturated-linear step it stores no bound vectors in the IR.
MATERIALIZE = 0.0


def size() -> tuple[int, int, int]:
    """``(m, n, nnz per row)``: the paper's, unless overridden for a smoke test."""
    return (
        int(os.environ.get("LASSO_CODEGEN_M", M)),
        int(os.environ.get("LASSO_CODEGEN_N", N)),
        int(os.environ.get("LASSO_CODEGEN_K", K)),
    )


@dataclass(frozen=True)
class Instance:
    D: spa.csr_matrix
    b: np.ndarray
    step_size: float
    lambd: float = LAMBDA

    @property
    def m(self) -> int:
        return self.D.shape[0]

    @property
    def n(self) -> int:
        return self.D.shape[1]

    @property
    def threshold(self) -> float:
        return self.lambd * self.step_size

    @property
    def x(self) -> np.ndarray:
        """The parameter value every timing uses: the midpoint of [b - 1, b + 1],
        as fomtools' ``representative_params`` picks it."""
        return 0.5 * ((self.b - 1.0) + (self.b + 1.0))


def instance() -> Instance:
    m, n, k = size()
    rng = np.random.default_rng(SEED)
    edge_count = m * k
    if edge_count % n:
        raise ValueError("m * nnz_per_row must be divisible by n for balanced columns")
    rows = np.repeat(np.arange(m, dtype=np.int32), k)
    cols = np.concatenate([rng.permutation(n) for _ in range(edge_count // n)]).astype(np.int32)
    values = rng.normal(scale=1.0 / np.sqrt(k), size=edge_count)
    D = spa.coo_matrix((values, (rows, cols)), shape=(m, n)).tocsr()
    D.sum_duplicates()
    D.eliminate_zeros()
    b = rng.normal(size=m)
    one_norm = float(np.asarray(np.abs(D).sum(axis=0)).max())
    inf_norm = float(np.asarray(np.abs(D).sum(axis=1)).max())
    return Instance(D=D, b=b, step_size=1.0 / (one_norm * inf_norm))


def name(algorithm: str) -> str:
    m, n, k = size()
    return f"lasso_{algorithm}_m{m}_n{n}_k{k}_sepA"


def gamma_schedule():
    """FISTA's momentum at the 0-based iteration ``t`` (paper eq. (8)):
    ``gamma_0 = 0`` (``w^0 = s^0``) and ``gamma_t = (beta_{t-1} - 1) / beta_t``."""
    betas = [1.0]

    def gamma(t: int) -> float:
        if t == 0:
            return 0.0
        while len(betas) <= t:
            beta = betas[-1]
            betas.append((1.0 + np.sqrt(1.0 + 4.0 * beta * beta)) / 2.0)
        return (betas[t - 1] - 1.0) / betas[t]

    return gamma


def record_ista(inst: Instance) -> dict:
    """ISTA, with three successive iterates for the stationarity check."""
    import fomtools as ft

    z0 = np.full(inst.n, 2.0)
    Dt_scaled = (inst.step_size * inst.D.T).tocsr()
    vp = ft.Verifier(method="simulator", obbt_solver=None)
    x = vp.parameter(inst.m, lb=inst.b - 1.0, ub=inst.b + 1.0)
    z = vp.iterate(inst.n, lb=z0, ub=z0)
    iterates = []
    for _ in range(3):
        r = vp.soft_threshold(inst.D @ z - x, lambd=MATERIALIZE)
        z = vp.soft_threshold(z - Dt_scaled @ r, lambd=inst.threshold)
        iterates.append(z)
    return {
        "verifier": vp,
        "next_state": iterates[0],
        "iterates": iterates,
        "param_values": {x: inst.b},
        "name": name("ista"),
    }


def record_fista(inst: Instance) -> dict:
    """FISTA as one two-slot iteration plus its momentum schedule. The extrapolated
    point is not its own step: a step that leaves a state slot out is padded with
    dense zero blocks, which at this size would be enormous."""
    import fomtools as ft
    from fomtools.codegen.extract_scheduled import Recording

    z0 = np.full(inst.n, 2.0)
    Dt_scaled = (inst.step_size * inst.D.T).tocsr()

    def build(gamma):
        vp = ft.Verifier(method="simulator", obbt_solver=None)
        x = vp.parameter(inst.m, lb=inst.b - 1.0, ub=inst.b + 1.0)
        s_prev = vp.iterate(inst.n, lb=z0, ub=z0)
        s = vp.iterate(inst.n, lb=z0, ub=z0)
        w = s + gamma * (s - s_prev)
        r = vp.soft_threshold(inst.D @ w - x, lambd=MATERIALIZE)
        s_next = vp.soft_threshold(w - Dt_scaled @ r, lambd=inst.threshold)
        return Recording(vp, next_state=[s, s_next], state=[s_prev, s])

    return {"build": build, "schedules": {"gamma": gamma_schedule()}, "name": name("fista")}


def extract_ir(algorithm: str, config):
    """The fomtools IR of ``algorithm`` on the instance, under ``config``."""
    from fomtools.codegen.generate import extract_fom_ir

    inst = instance()
    if algorithm == "ista":
        rec = record_ista(inst)
        return extract_fom_ir(
            rec["verifier"],
            rec["next_state"],
            config=config,
            name=rec["name"],
            iterates=rec["iterates"],
            param_values=rec["param_values"],
        )
    rec = record_fista(inst)
    return extract_fom_ir(rec["build"], config=config, name=rec["name"], schedules=rec["schedules"])


def textbook_fista(inst: Instance, iterations: int) -> np.ndarray:
    """FISTA written out from Beck and Teboulle, independently of `gamma_schedule`:
    ``t_1 = 1``, ``y^1 = x^0``, ``x^k = prox(y^k)``, ``t_{k+1} = (1 + sqrt(1 + 4 t_k^2)) / 2``,
    ``y^{k+1} = x^k + ((t_k - 1) / t_{k+1}) (x^k - x^{k-1})``. Returns ``(x^{K-1}, x^K)``."""
    D, DT, eta, thr = inst.D, inst.D.T.tocsr(), inst.step_size, inst.threshold
    x_prev = x = np.full(inst.n, 2.0)
    y, t = x.copy(), 1.0
    for _ in range(iterations):
        v = y - eta * (DT @ (D @ y - inst.x))
        x_prev, x = x, np.sign(v) * np.maximum(np.abs(v) - thr, 0.0)
        t_next = (1.0 + np.sqrt(1.0 + 4.0 * t * t)) / 2.0
        y = x + ((t - 1.0) / t_next) * (x - x_prev)
        t = t_next
    return np.concatenate([x_prev, x])
