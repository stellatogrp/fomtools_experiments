"""Worst-case rAPDHG residuals on a small, verifiable quadcopter program.

The generated controller runs on a 1 s lookahead at 10 ms (40 variables, 120
rows, restart period 64, 512 iterations). Every iteration projects all 120
dual rows through a ReLU, which the MIP encodes with one binary each, so that
program is far beyond what the VP can certify. This module verifies the same
plant, cost, tilt bound and algorithm on a program small enough to solve:

* the published 0.1 s matrices, a horizon of T = 4 predicted states and no
  move blocking, so 12 variables;
* the box rows of the thrusts and the tilt rows at every predicted state except
  the first. The first predicted state is the measured one, so its tilt rows do
  not involve the inputs at all and are dropped. That leaves 24 + 12 = 36 rows;
* restart period 4, so K up to 12 to 16 already contains three or four restarts.

The parameter is the measured state, not the boxes on ``q`` and ``h`` that code
generation uses. As in the paper's control family, the hover state is perturbed
in the two tilt angles and the altitude, every other coordinate is zero, and the
reference is hover at the origin. Then ``q = Gx x0`` and ``h(x0)`` are affine in
a three-dimensional parameter.

The iteration is Algorithm 1 written as a nested loop (``rapdhg.solve_as_written``)
with the restart applied explicitly, so no schedule tables are needed, and
without the materializing saturation of ``recording.py``: that step exists only to
keep the emitted C from applying ``Q`` three times, and in the MIP it would add
binaries with big-M constants of 1e6.

The metric is the change in the primal-dual iterate, ``||(x^k, y^k) - (x^{k-1},
y^{k-1})||_inf``, where the iterate after a restart is the restarted average.

Run from ``control/rapdhg``:

    python -m verify.verify                # MIP and simulator up to K_MAX at T = HORIZON
    python -m verify.verify --T 5 --K 12   # another horizon and length
    python -m verify.verify --check        # recording against the NumPy reference
"""

from __future__ import annotations

import argparse
import csv
import dataclasses
import time
from pathlib import Path

import numpy as np
from scipy.linalg import svdvals

from codegen import rapdhg
from codegen.mpc import RapdhgProblem, build_mpc
from common.quadcopter import TILT_LIMIT

OUT = Path(__file__).parent / "out"

DT = 0.1
HORIZON = 4  # predicted states; the lookahead is HORIZON * DT
BLOCK = 1
RESTART = 4
PRIMAL_WEIGHT = 1.0
K_MAX = 8

#: Perturbed state coordinates: pitch-like angle, roll-like angle, altitude.
#: See ``common.quadcopter`` for why coordinate 0 is the pitch-like one.
PARAM_COORDS = (0, 1, 5)
TILT_RANGE = np.deg2rad(20.0)
ALTITUDE_RANGE = 0.5
PARAM_LB = np.array([-TILT_RANGE, -TILT_RANGE, -ALTITUDE_RANGE])
PARAM_UB = -PARAM_LB

MIP_GAP = 0.05
N_SAMPLES = 10_000


def small_problem(horizon: int = HORIZON) -> RapdhgProblem:
    """The horizon-``T`` program, with the tilt rows that do not involve the inputs removed."""
    p = build_mpc(
        dt=DT,
        lookahead=horizon * DT,
        block=BLOCK,
        restart=RESTART,
        primal_weight=PRIMAL_WEIGHT,
        tilt_limit=TILT_LIMIT,
    )
    keep = np.abs(p.C).sum(axis=1) > 0
    C, F, e = p.C[keep], p.F[keep], p.e[keep]
    identity = np.eye(p.n)
    G = np.vstack([identity, -identity, C, -C])
    return dataclasses.replace(
        p,
        C=C,
        F=F,
        e=e,
        G=G,
        h=np.concatenate([p.hi, -p.lo, e, e]),
        norm_g=float(svdvals(G)[0]),
    )


def selection(problem: RapdhgProblem) -> np.ndarray:
    """Map the parameter (pitch, roll, altitude) into the full measured state."""
    S = np.zeros((problem.A.shape[0], len(PARAM_COORDS)))
    for j, i in enumerate(PARAM_COORDS):
        S[i, j] = 1.0
    return S


def affine_data(problem: RapdhgProblem):
    """``q = Q_p p`` and ``h = h_0 + H_p p`` for the parameter ``p``, with a hover reference."""
    S = selection(problem)
    Qp = problem.Gx @ S
    k = problem.num_state_rows
    FS = problem.F @ S
    h0 = np.concatenate([problem.hi, -problem.lo, problem.e, problem.e])
    Hp = np.vstack([np.zeros((2 * problem.n, S.shape[1])), -FS, FS])
    assert Hp.shape[0] == 2 * problem.n + 2 * k
    return Qp, h0, Hp


def make_verifier(backend: str):
    from fomtools.verifier import Verifier

    if backend == "mip":
        return Verifier(
            method="mip",
            solver="gurobi",
            obbt_solver="gurobi",
            params={"OutputFlag": 0, "MIPGap": MIP_GAP},
        )
    if backend == "simulator":
        return Verifier(method="simulator", obbt_solver=None)
    raise ValueError(backend)


def run(
    problem: RapdhgProblem, backend: str, K: int, samples: np.ndarray | None = None
) -> list[dict]:
    """Record K rAPDHG iterations from a cold start and solve the VP at every k."""
    n, m, R = problem.n, problem.m, problem.restart
    Q, G = problem.P, problem.G
    Qp, h0, Hp = affine_data(problem)
    vp = make_verifier(backend)

    if backend == "simulator":
        p = vp.add_param(len(PARAM_COORDS), samples=samples)
        x0 = vp.add_iterate(n, init=np.zeros(n))
        y0 = vp.add_iterate(m, init=np.zeros(m))
    else:
        p = vp.add_param(len(PARAM_COORDS), lb=PARAM_LB, ub=PARAM_UB)
        x0 = vp.add_iterate(n, lb=0.0, ub=0.0)
        y0 = vp.add_iterate(m, lb=0.0, ub=0.0)
    q = Qp @ p
    h = h0 + Hp @ p

    rows = []
    x, y = x0, y0
    x_prev, xbar, ybar = x0, x0, y0
    for t in range(K):
        k = t % R
        binv, theta, eta, tau = rapdhg.coefficients(problem, k)
        xmd = (1.0 - binv) * xbar + binv * x
        y_new = vp.step.relu(y + tau * (G @ ((1.0 + theta) * x - theta * x_prev) - h))
        x_new = x - eta * (Q @ xmd + q + G.T @ y_new)
        xbar = (1.0 - binv) * xbar + binv * x_new
        ybar = (1.0 - binv) * ybar + binv * y_new
        x_prev, x_old, y_old = x, x, y
        if k == R - 1:
            # Restart: the next inner loop starts from the running average.
            x, y = xbar, ybar
            x_prev = xbar
        else:
            x, y = x_new, y_new
        vp.set_objective_norm([x - x_old, y - y_old], ord=np.inf)
        if backend == "simulator":
            continue
        start = time.perf_counter()
        value = vp.solve()
        elapsed = time.perf_counter() - start
        bound = float(vp.canonicalizer._solved_copy["ObjBound"])
        rows.append(
            dict(backend=backend, K=t + 1, value=float(value), bound=bound, solve_time=elapsed)
        )
        print(
            f"    mip K={t + 1}: incumbent {value:.4e}, bound {bound:.4e} ({elapsed:.2f} s)",
            flush=True,
        )

    if backend == "simulator":
        start = time.perf_counter()
        residuals = np.asarray(vp.solve())
        elapsed = time.perf_counter() - start
        rows = [
            dict(
                backend=backend,
                K=k + 1,
                value=float(residuals[:, k].max()),
                bound=float("nan"),
                solve_time=elapsed,
            )
            for k in range(K)
        ]
    return rows


def numpy_residuals(problem: RapdhgProblem, P: np.ndarray, K: int) -> np.ndarray:
    """The same metric from ``rapdhg.solve_as_written``, one row per parameter."""
    Qp, h0, Hp = affine_data(problem)
    out = np.empty((P.shape[0], K))
    for i, p in enumerate(P):
        q, h = Qp @ p, h0 + Hp @ p
        prev = np.zeros(problem.n + problem.m)
        for t in range(1, K + 1):
            x, y = rapdhg.solve_as_written(problem, q, t, h=h)
            cur = np.concatenate([x, y])
            out[i, t - 1] = np.abs(cur - prev).max()
            prev = cur
    return out


def sample_params(size: int, rng: np.random.Generator) -> np.ndarray:
    return rng.uniform(PARAM_LB, PARAM_UB, size=(size, len(PARAM_COORDS)))


def check(problem: RapdhgProblem) -> None:
    """The recorded iteration on the simulator against the NumPy nested loop."""
    K = 3 * problem.restart + 1
    P = sample_params(20, np.random.default_rng(1))
    sim = run(problem, "simulator", K, samples=P)
    ref = numpy_residuals(problem, P, K)
    sim_max = np.array([r["value"] for r in sim])
    print("K    simulator max   NumPy max      diff")
    for k in range(K):
        print(
            f"{k + 1:<4d} {sim_max[k]:.6e}   {ref[:, k].max():.6e}   {abs(sim_max[k] - ref[:, k].max()):.1e}"
        )


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--check", action="store_true")
    parser.add_argument("--K", type=int, default=K_MAX)
    parser.add_argument("--T", type=int, default=HORIZON)
    args = parser.parse_args()

    problem = small_problem(args.T)
    print(
        f"n = {problem.n}, m = {problem.m}, restart = {problem.restart}, "
        f"||Q|| = {problem.norm_q:.4g}, ||G|| = {problem.norm_g:.4g}"
    )
    if args.check:
        check(problem)
        return

    rows = run(problem, "mip", args.K)
    samples = sample_params(N_SAMPLES, np.random.default_rng(0))
    rows += run(problem, "simulator", args.K, samples=samples)
    print("  simulator", " ".join(f"{r['value']:.4e}" for r in rows if r["backend"] == "simulator"))

    OUT.mkdir(parents=True, exist_ok=True)
    with open(OUT / f"bounds_T{args.T}.csv", "w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=["backend", "K", "value", "bound", "solve_time"])
        writer.writeheader()
        writer.writerows(rows)


if __name__ == "__main__":
    main()
