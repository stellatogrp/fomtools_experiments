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

The metric is the scaled residual of the inner step, which recovers the KKT
conditions of ``minimize (1/2) x'Qx + c'x subject to Gx <= h``. With the step
at inner index k producing ``(x^{k+1}, y^{k+1})`` from ``x^k``, ``x^{k-1}`` and
``x_md``,

    r_d = (x^k - x^{k+1}) / eta_k - Q (x_md - x^{k+1})
        = Q x^{k+1} + c + G' y^{k+1},
    r_p = (y^k - y^{k+1}) / tau_k - G ((x^{k+1} - x^k) - theta_k (x^k - x^{k-1}))
        in h - G x^{k+1} + N_+(y^{k+1}),

the first from rearranging the primal step and the second from the prox lemma on
the dual projection, as in the appendix of the MIP verification paper. Both are
zero exactly at a KKT point, and dividing by the step sizes removes the factor
k + 1 by which rAPDHG's steps grow inside a restart period, which the unscaled
change ``||s^{k+1} - s^k||`` carries. The residual is that of the inner iterate
at every k, restart steps included; the restart then moves to the average.

Run from ``control/rapdhg``:

    python -m verify.verify                # MIP and simulator up to K_MAX at T = HORIZON
    python -m verify.verify --T 5 --K 12   # another horizon and length
    python -m verify.verify --T 5 --K 16 --restart 8
    python -m verify.verify --T 10 --K 25 --restart 8 --ruiz   # Ruiz-equilibrated
    python -m verify.verify --T 5 --K 25 --restart 12 --step-scale 2

``--method pdhg`` runs plain PDHG instead, the same dual-first iteration with no
acceleration, averaging or restarts: extrapolation ``theta = 1``, ``x_md = x^k``,
and constant steps ``tau = 1/||G||``, ``eta = 1/(||P||/2 + ||G||)``, which meet
the Condat-Vu condition ``1/eta - tau ||G||^2 >= ||P||/2``. The scaled residual
above applies unchanged.

``--step-scale a`` multiplies both of Theorem 1's step sizes ``eta_k`` and
``tau_k`` by ``a``. Any ``a > 1`` leaves the theorem's guarantee; at T = 5 a
factor of 2 converges faster in a sampled screen and 4 diverges. The residual
divides by the step sizes actually taken, so it stays the KKT residual.
    python -m verify.verify --check        # recording against the NumPy reference

With ``--ruiz`` the iteration runs on the equilibrated program, as the network
experiment does: symmetric Ruiz on the KKT matrix ``[[Q, G'], [G, 0]]`` gives
diagonal ``D`` (variables) and ``E`` (rows), rAPDHG runs on ``DQD``, ``Dc``,
``EGD`` and ``Eh`` with ``x = D x_hat`` and ``y = E y_hat``, and its step sizes
come from the equilibrated norms. The parameter is not scaled; it enters as
``D c(p)`` and ``E h(p)``. The metric is still the KKT residual of the *original*
program, ``(D^{-1} r_d_hat, E^{-1} r_p_hat)``: the dual residual
``Q x + c + G' y`` in original units, and an element of ``h - G x + N_+(y)``,
since the normal cone of the orthant is invariant under positive diagonal
scaling.
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


def small_problem(horizon: int = HORIZON, restart: int = RESTART) -> RapdhgProblem:
    """The horizon-``T`` program, with the tilt rows that do not involve the inputs removed."""
    p = build_mpc(
        dt=DT,
        lookahead=horizon * DT,
        block=BLOCK,
        restart=restart,
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


def ruiz_kkt(Q: np.ndarray, G: np.ndarray, iters: int = 30) -> tuple[np.ndarray, np.ndarray]:
    """Symmetric Ruiz on ``[[Q, G'], [G, 0]]``: ``(d, e)`` with unit column inf-norms."""
    n, m = Q.shape[0], G.shape[0]
    M = np.block([[Q, G.T], [G, np.zeros((m, m))]])
    s = np.ones(n + m)
    for _ in range(iters):
        norms = np.abs(s[:, None] * M * s[None, :]).max(axis=0)
        s = s / np.sqrt(np.where(norms > 0, norms, 1.0))
    return s[:n], s[n:]


def equilibrated(problem: RapdhgProblem, d: np.ndarray, e: np.ndarray) -> RapdhgProblem:
    """The program in ``x = D x_hat``, ``y = E y_hat``; only ``P``, ``G`` and the norms change."""
    P = d[:, None] * problem.P * d[None, :]
    G = e[:, None] * problem.G * d[None, :]
    return dataclasses.replace(
        problem, P=P, G=G, norm_q=float(np.linalg.norm(P, 2)), norm_g=float(svdvals(G)[0])
    )


def scaling(problem: RapdhgProblem, ruiz: bool) -> tuple[np.ndarray, np.ndarray]:
    if ruiz:
        return ruiz_kkt(problem.P, problem.G)
    return np.ones(problem.n), np.ones(problem.m)


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


def step_coefficients(problem: RapdhgProblem, k: int, method: str):
    """``(1/beta_k, theta_k, eta_k, tau_k)`` at inner index k, for rAPDHG or plain PDHG."""
    if method == "pdhg":
        return 1.0, 1.0, 1.0 / (problem.norm_q / 2.0 + problem.norm_g), 1.0 / problem.norm_g
    return rapdhg.coefficients(problem, k)


def restart_period(problem: RapdhgProblem, method: str) -> int:
    """Plain PDHG never restarts, so its inner index never wraps."""
    return problem.restart if method == "rapdhg" else 10**9


def run(
    problem: RapdhgProblem,
    backend: str,
    K: int,
    samples: np.ndarray | None = None,
    ruiz: bool = False,
    step_scale: float = 1.0,
    method: str = "rapdhg",
) -> list[dict]:
    """Record K rAPDHG iterations from a cold start and solve the VP at every k."""
    n, m, R = problem.n, problem.m, restart_period(problem, method)
    Qp, h0, Hp = affine_data(problem)
    d, e = scaling(problem, ruiz)
    scaled = equilibrated(problem, d, e)
    Q, G = scaled.P, scaled.G
    # Map the scaled residual back to the original program's units.
    unscale_d, unscale_p = np.diag(1.0 / d), np.diag(1.0 / e)
    vp = make_verifier(backend)

    if backend == "simulator":
        p = vp.add_param(len(PARAM_COORDS), samples=samples)
        x0 = vp.add_iterate(n, init=np.zeros(n))
        y0 = vp.add_iterate(m, init=np.zeros(m))
    else:
        p = vp.add_param(len(PARAM_COORDS), lb=PARAM_LB, ub=PARAM_UB)
        x0 = vp.add_iterate(n, lb=0.0, ub=0.0)
        y0 = vp.add_iterate(m, lb=0.0, ub=0.0)
    q = (d[:, None] * Qp) @ p
    h = e * h0 + (e[:, None] * Hp) @ p

    rows = []
    x, y = x0, y0
    x_prev, xbar, ybar = x0, x0, y0
    for t in range(K):
        k = t % R
        binv, theta, eta, tau = step_coefficients(scaled, k, method)
        eta, tau = step_scale * eta, step_scale * tau
        xmd = (1.0 - binv) * xbar + binv * x
        y_new = vp.step.relu(y + tau * (G @ ((1.0 + theta) * x - theta * x_prev) - h))
        x_new = x - eta * (Q @ xmd + q + G.T @ y_new)
        xbar = (1.0 - binv) * xbar + binv * x_new
        ybar = (1.0 - binv) * ybar + binv * y_new
        # Scaled residual of the inner step (see the module docstring).
        r_d = (1.0 / eta) * (x - x_new) - Q @ (xmd - x_new)
        r_p = (1.0 / tau) * (y - y_new) - G @ ((x_new - x) - theta * (x - x_prev))
        if k == R - 1:
            # Restart: the next inner loop starts from the running average.
            x, y, x_prev = xbar, ybar, xbar
        else:
            x_prev, x, y = x, x_new, y_new
        vp.set_objective_norm([unscale_d @ r_d, unscale_p @ r_p], ord=np.inf)
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


def numpy_residuals(
    problem: RapdhgProblem,
    P: np.ndarray,
    K: int,
    ruiz: bool = False,
    step_scale: float = 1.0,
    method: str = "rapdhg",
) -> np.ndarray:
    """The same metric in NumPy, one row per parameter.

    The dual block is computed directly as ``Q x + c + G' y`` in the original
    variables rather than from the step, so the comparison also checks the first
    identity of the docstring and, with ``ruiz``, the unscaling.
    """
    Qp, h0, Hp = affine_data(problem)
    d, e = scaling(problem, ruiz)
    scaled = equilibrated(problem, d, e)
    Q, G, R = scaled.P, scaled.G, restart_period(problem, method)
    out = np.empty((P.shape[0], K))
    for i, p in enumerate(P):
        c_orig = Qp @ p
        c, h = d * c_orig, e * (h0 + Hp @ p)
        x = x_prev = xbar = np.zeros(problem.n)
        y = ybar = np.zeros(problem.m)
        for t in range(K):
            k = t % R
            binv, theta, eta, tau = step_coefficients(scaled, k, method)
            eta, tau = step_scale * eta, step_scale * tau
            xmd = (1.0 - binv) * xbar + binv * x
            y_new = np.maximum(y + tau * (G @ (x + theta * (x - x_prev)) - h), 0.0)
            x_new = x - eta * (Q @ xmd + c + G.T @ y_new)
            r_d = problem.P @ (d * x_new) + c_orig + problem.G.T @ (e * y_new)
            r_p = ((y - y_new) / tau - G @ ((x_new - x) - theta * (x - x_prev))) / e
            out[i, t] = max(np.abs(r_d).max(), np.abs(r_p).max())
            xbar = (1.0 - binv) * xbar + binv * x_new
            ybar = (1.0 - binv) * ybar + binv * y_new
            if k == R - 1:
                x, y, x_prev = xbar, ybar, xbar
            else:
                x_prev, x, y = x, x_new, y_new
    return out


def sample_params(size: int, rng: np.random.Generator) -> np.ndarray:
    return rng.uniform(PARAM_LB, PARAM_UB, size=(size, len(PARAM_COORDS)))


def check(
    problem: RapdhgProblem, ruiz: bool = False, step_scale: float = 1.0, method: str = "rapdhg"
) -> None:
    """The recorded iteration on the simulator against the NumPy nested loop."""
    K = 3 * problem.restart + 1
    P = sample_params(20, np.random.default_rng(1))
    sim = run(problem, "simulator", K, samples=P, ruiz=ruiz, step_scale=step_scale, method=method)
    ref = numpy_residuals(problem, P, K, ruiz=ruiz, step_scale=step_scale, method=method)
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
    parser.add_argument("--restart", type=int, default=RESTART)
    parser.add_argument("--ruiz", action="store_true")
    parser.add_argument("--step-scale", type=float, default=1.0)
    parser.add_argument("--method", choices=["rapdhg", "pdhg"], default="rapdhg")
    parser.add_argument(
        "--samples", type=int, default=None, help="simulator only, with this many samples"
    )
    args = parser.parse_args()

    problem = small_problem(args.T, args.restart)
    print(
        f"n = {problem.n}, m = {problem.m}, restart = {problem.restart}, "
        f"||Q|| = {problem.norm_q:.4g}, ||G|| = {problem.norm_g:.4g}"
    )
    if args.ruiz:
        scaled = equilibrated(problem, *scaling(problem, True))
        print(f"Ruiz: ||Q_hat|| = {scaled.norm_q:.4g}, ||G_hat|| = {scaled.norm_g:.4g}")
    if args.check:
        check(problem, args.ruiz, args.step_scale, args.method)
        return

    # The same seed draws the same leading rows, so a smaller sample is a subset of the default one.
    samples = sample_params(args.samples or N_SAMPLES, np.random.default_rng(0))
    rows = (
        []
        if args.samples
        else run(
            problem, "mip", args.K, ruiz=args.ruiz, step_scale=args.step_scale, method=args.method
        )
    )
    rows += run(
        problem,
        "simulator",
        args.K,
        samples=samples,
        ruiz=args.ruiz,
        step_scale=args.step_scale,
        method=args.method,
    )
    print("  simulator", " ".join(f"{r['value']:.4e}" for r in rows if r["backend"] == "simulator"))

    OUT.mkdir(parents=True, exist_ok=True)
    if args.method == "pdhg":
        name = f"bounds_T{args.T}_pdhg"
    else:
        name = f"bounds_T{args.T}" + ("" if args.restart == RESTART else f"_R{args.restart}")
    name += "_ruiz" if args.ruiz else ""
    name += f"_a{args.step_scale:g}" if args.step_scale != 1.0 else ""
    name += f"_sim{args.samples}" if args.samples else ""
    with open(OUT / f"{name}.csv", "w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=["backend", "K", "value", "bound", "solve_time"])
        writer.writeheader()
        writer.writerows(rows)


if __name__ == "__main__":
    main()
