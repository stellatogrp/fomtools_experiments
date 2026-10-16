"""Hand-written ISTA/FISTA baselines that keep D and D^T separate.

    python baselines.py check {ista,fista} --iters K [--cupy] [--oracle-dir DIR]
    python baselines.py bench {scipy,cupy} {ista,fista} --iters K

Same instance as paper_{ista,fista}_sparse20k.py (seed 0, lambda 0.02, step
eta = 1/(||D||_1 ||D||_inf), s^0 = 2*1, x = midpoint of the parameter box),
iterated as written in the paper:

    ISTA:  s^{k+1} = soft(s^k - eta D^T(D s^k - x), lambda eta)
    FISTA: w^k = s^k + gamma_k (s^k - s^{k-1}),  s^{k+1} = soft(w^k - eta D^T(D w^k - x), lambda eta)

Each iteration multiplies by D and D^T (2.6M nonzeros each) instead of the
precomputed I - eta D^T D that FOMTools extracts. SciPy runs on the CPU; CuPy's
sparse products call cuSPARSE on the GPU.

check: compares the iterate after --iters iterations with the FOMTools-form
iteration (the problem file's own At = I - eta D^T D, Bt = eta D^T) and, with
--oracle-dir, with FOMTools' reference output. bench: times complete solves,
excluding data construction and upload of D and x; CuPy includes the final
download of the solution. SciPy: 1 warm-up + 3 solves, median (as the NumPy
column); CuPy: 8 warm-ups + 10 solves, mean (as the generated solvers).
"""

from __future__ import annotations

import argparse
import importlib.util
import os
import statistics
import time

import numpy as np
import scipy.sparse as spa


def instance():
    """D, b, eta, lambda exactly as paper_*_sparse20k._instance builds them (same RNG calls)."""
    m = int(os.environ.get("FOMTOOLS_EXPERIMENT_M", "10000"))
    n = int(os.environ.get("FOMTOOLS_EXPERIMENT_N", "40000"))
    nnz_per_row = int(os.environ.get("FOMTOOLS_EXPERIMENT_A_NNZ_PER_ROW", "8"))
    lambd = 0.02
    rng = np.random.default_rng(0)
    edge_count = m * nnz_per_row
    rows = np.repeat(np.arange(m, dtype=np.int32), nnz_per_row)
    cols = np.concatenate([rng.permutation(n) for _ in range(edge_count // n)]).astype(np.int32)
    values = rng.normal(scale=1.0 / np.sqrt(nnz_per_row), size=edge_count)
    D = spa.coo_matrix((values, (rows, cols)), shape=(m, n)).tocsr()
    D.sum_duplicates()
    D.eliminate_zeros()
    b = rng.normal(size=m)
    one_norm = float(np.asarray(np.abs(D).sum(axis=0)).max())
    inf_norm = float(np.asarray(np.abs(D).sum(axis=1)).max())
    eta = 1.0 / (one_norm * inf_norm)
    x = 0.5 * ((b - 1.0) + (b + 1.0))  # representative_params: midpoint of [b - 1, b + 1]
    return D, x, eta, lambd


def gammas(K: int) -> list[float]:
    """gamma_k = (beta_k - 1)/beta_{k+1}, beta_0 = 1, indexed by the 0-based iteration."""
    betas = [1.0]
    while len(betas) <= K + 1:
        betas.append((1.0 + np.sqrt(1.0 + 4.0 * betas[-1] ** 2)) / 2.0)
    return [(betas[k] - 1.0) / betas[k + 1] for k in range(K)]


def solve(xp, D, DT, x, eta, thr, algorithm, K, gam):
    """Run K iterations with array module xp (numpy or cupy); return the state as FOMTools stores it."""
    n = D.shape[1]
    s = xp.full(n, 2.0)
    s_prev = s
    for k in range(K):
        w = s if algorithm == "ista" else s + gam[k] * (s - s_prev)
        v = w - eta * (DT @ (D @ w - x))
        s_prev, s = s, xp.sign(v) * xp.maximum(xp.abs(v) - thr, 0.0)
    return s if algorithm == "ista" else xp.concatenate([s_prev, s])


def fomtools_form(algorithm, K, gam):
    """The same iteration in FOMTools' extracted form, from the problem file's own matrices.

    Always the original formulation in $REPO, loaded by path: in a variant run
    (e.g. sepA) the module name on sys.path resolves to the variant instead.
    """
    path = os.path.join(os.environ["REPO"], f"paper_{algorithm}_sparse20k.py")
    spec = importlib.util.spec_from_file_location(f"original_{algorithm}", path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    n, _, _, _, lambd, b, eta, At, Bt, _ = module._instance()
    x = 0.5 * ((b - 1.0) + (b + 1.0))
    px, thr = Bt @ x, lambd * eta
    s = s_prev = np.full(n, 2.0)
    for k in range(K):
        w = s if algorithm == "ista" else s + gam[k] * (s - s_prev)
        v = At @ w + px
        s_prev, s = s, np.sign(v) * np.maximum(np.abs(v) - thr, 0.0)
    return s if algorithm == "ista" else np.concatenate([s_prev, s])


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("mode", choices=("check", "bench"))
    ap.add_argument("args", nargs="+")
    ap.add_argument("--iters", type=int, required=True)
    ap.add_argument("--cupy", action="store_true")
    ap.add_argument("--oracle-dir")
    ap.add_argument("--tol", type=float, default=None, help="check: fail if any difference exceeds this")
    ap.add_argument("--skip-form", action="store_true",
                    help="check: skip the original-form iteration (too large to build for dense D); "
                         "compare with --oracle-dir only")
    a = ap.parse_args()

    D, x, eta, lambd = instance()
    DT = D.T.tocsr()
    thr = lambd * eta
    K = a.iters

    if a.mode == "check":
        (algorithm,) = a.args
        gam = gammas(K)
        mine = solve(np, D, DT, x, eta, thr, algorithm, K, gam)
        diffs = []
        if not a.skip_form:
            form = fomtools_form(algorithm, K, gam)
            diffs.append(float(np.max(np.abs(mine - form))))
            print(f"CHECK {algorithm} K={K} scipy vs FOMTools-form: max abs diff {diffs[-1]:.3e}")
        elif not a.oracle_dir:
            raise SystemExit("--skip-form needs --oracle-dir")
        if a.cupy:
            import cupy as cp
            import cupyx.scipy.sparse as csp

            g = solve(cp, csp.csr_matrix(D), csp.csr_matrix(DT), cp.asarray(x), eta, thr, algorithm, K, gam)
            diffs.append(float(np.max(np.abs(cp.asnumpy(g) - mine))))
            print(f"CHECK {algorithm} K={K} cupy vs scipy: max abs diff {diffs[-1]:.3e}")
        if a.oracle_dir:
            from fomtools.codegen.harness import representative_params
            from fomtools.codegen.oracle import ReferenceOracle
            from fomtools.codegen.spec import load

            ir = load(a.oracle_dir)
            assert ir.termination.max_iters == K, (ir.termination.max_iters, K)
            ref = np.asarray(ReferenceOracle(ir).solve(param_values=representative_params(ir)).solution)
            diffs.append(float(np.max(np.abs(mine - ref))))
            print(f"CHECK {algorithm} K={K} scipy vs FOMTools reference: max abs diff {diffs[-1]:.3e}")
        if a.tol is not None and max(diffs) > a.tol:
            print(f"CHECK_FAILED {algorithm}: {max(diffs):.3e} > tol {a.tol:.1e}")
            return 1
        print(f"CHECK_OK {algorithm}")
        return 0

    backend, algorithm = a.args
    gam = gammas(K)
    upper = algorithm.upper()
    if backend == "scipy":
        solve(np, D, DT, x, eta, thr, algorithm, K, gam)  # warm-up
        samples = []
        for _ in range(3):
            t0 = time.perf_counter_ns()
            solve(np, D, DT, x, eta, thr, algorithm, K, gam)
            samples.append((time.perf_counter_ns() - t0) / 1000.0)
        print(f"SCIPY_{upper} reps=3 median_us={statistics.median(samples):.6f} min_us={min(samples):.6f}")
    else:
        import cupy as cp
        import cupyx.scipy.sparse as csp

        Dg, DTg, xg = csp.csr_matrix(D), csp.csr_matrix(DT), cp.asarray(x)
        dev = cp.cuda.Device()
        for _ in range(8):
            cp.asnumpy(solve(cp, Dg, DTg, xg, eta, thr, algorithm, K, gam))
        samples = []
        for _ in range(10):
            dev.synchronize()
            t0 = time.perf_counter_ns()
            cp.asnumpy(solve(cp, Dg, DTg, xg, eta, thr, algorithm, K, gam))  # includes the download
            samples.append((time.perf_counter_ns() - t0) / 1000.0)
        print(f"CUPY_{upper}")
        print(f"BENCH solves=10 total_us={sum(samples):.4f} mean_us={statistics.mean(samples):.6f} "
              f"min_us={min(samples):.6f}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
