"""Hand-written ISTA and FISTA that keep D and D^T separate: SciPy and CuPy.

    python -m lasso_codegen.baselines {scipy,cupy} {ista,fista}

Ported from ``lasso_codegen/results/rect10k40k_dense_sepA/source/baselines.py``.
Each iteration is written as in the paper,

    ISTA:  s^{k+1} = soft(s^k - eta D^T (D s^k - x), lambda eta)
    FISTA: w^k = s^k + gamma_k (s^k - s^{k-1}),  s^{k+1} = soft(w^k - eta D^T (D w^k - x), lambda eta)

SciPy runs on one CPU core; CuPy's sparse products call cuSPARSE. A timing excludes
building and uploading ``D`` and ``x``; CuPy's includes downloading the solution.
SciPy: 1 warm-up and 3 solves, median. CuPy: 8 warm-ups and 10 solves, mean (as the
generated solvers' ``bench_bin``). Each prints one line the benchmark parses.
"""

from __future__ import annotations

import argparse
import statistics
import time

import numpy as np

from lasso_codegen import problem


def gammas(iterations: int) -> list[float]:
    gamma = problem.gamma_schedule()
    return [gamma(k) for k in range(iterations)]


def solve(xp, D, DT, x, inst, algorithm: str, iterations: int, gam):
    """``iterations`` iterations with array module ``xp`` (numpy or cupy); the
    state as fomtools stores it (FISTA: previous and current iterate)."""
    eta, thr = inst.step_size, inst.threshold
    s = xp.full(D.shape[1], 2.0)
    s_prev = s
    for k in range(iterations):
        w = s if algorithm == "ista" else s + gam[k] * (s - s_prev)
        v = w - eta * (DT @ (D @ w - x))
        s_prev, s = s, xp.sign(v) * xp.maximum(xp.abs(v) - thr, 0.0)
    return s if algorithm == "ista" else xp.concatenate([s_prev, s])


def scipy_solve(inst, algorithm: str, iterations: int = problem.ITERATIONS) -> np.ndarray:
    return solve(
        np, inst.D, inst.D.T.tocsr(), inst.x, inst, algorithm, iterations, gammas(iterations)
    )


def bench(backend: str, algorithm: str, iterations: int = problem.ITERATIONS) -> str:
    inst = problem.instance()
    gam = gammas(iterations)
    upper = algorithm.upper()
    if backend == "scipy":
        D, DT = inst.D, inst.D.T.tocsr()
        solve(np, D, DT, inst.x, inst, algorithm, iterations, gam)  # warm-up
        samples = []
        for _ in range(3):
            t0 = time.perf_counter_ns()
            solve(np, D, DT, inst.x, inst, algorithm, iterations, gam)
            samples.append((time.perf_counter_ns() - t0) / 1000.0)
        return f"SCIPY_{upper} reps=3 median_us={statistics.median(samples):.6f} min_us={min(samples):.6f}"
    import cupy as cp
    import cupyx.scipy.sparse as csp

    Dg, DTg, xg = csp.csr_matrix(inst.D), csp.csr_matrix(inst.D.T.tocsr()), cp.asarray(inst.x)
    device = cp.cuda.Device()
    for _ in range(8):
        cp.asnumpy(solve(cp, Dg, DTg, xg, inst, algorithm, iterations, gam))
    samples = []
    for _ in range(10):
        device.synchronize()
        t0 = time.perf_counter_ns()
        cp.asnumpy(solve(cp, Dg, DTg, xg, inst, algorithm, iterations, gam))
        samples.append((time.perf_counter_ns() - t0) / 1000.0)
    return (
        f"CUPY_{upper} solves=10 mean_us={statistics.mean(samples):.6f} min_us={min(samples):.6f}"
    )


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("backend", choices=("scipy", "cupy"))
    parser.add_argument("algorithm", choices=("ista", "fista"))
    args = parser.parse_args()
    print(bench(args.backend, args.algorithm), flush=True)


if __name__ == "__main__":
    main()
