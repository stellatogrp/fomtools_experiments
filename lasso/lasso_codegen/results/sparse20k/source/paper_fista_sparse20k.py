"""Paper-style sparse Lasso FISTA instance for backend scaling experiments."""

import os

import numpy as np
import scipy.sparse as spa
from fomtools.codegen.extract_scheduled import Recording
from fomtools.verifier import Verifier


def _instance():
    n = int(os.environ.get("FOMTOOLS_EXPERIMENT_N", "20000"))
    m = n
    nnz_per_row = int(os.environ.get("FOMTOOLS_EXPERIMENT_A_NNZ_PER_ROW", "8"))
    seed = 0
    lambd = 0.02
    rng = np.random.default_rng(seed)
    rows = np.tile(np.arange(m, dtype=np.int32), nnz_per_row)
    cols = np.concatenate([rng.permutation(n) for _ in range(nnz_per_row)]).astype(np.int32)
    values = rng.normal(scale=1.0 / np.sqrt(nnz_per_row), size=rows.size)
    A = spa.coo_matrix((values, (rows, cols)), shape=(m, n)).tocsr()
    A.sum_duplicates()
    A.eliminate_zeros()

    b = rng.normal(size=m)
    one_norm = float(np.asarray(np.abs(A).sum(axis=0)).max())
    inf_norm = float(np.asarray(np.abs(A).sum(axis=1)).max())
    lipschitz_bound = one_norm * inf_norm
    t = 1.0 / lipschitz_bound
    At = (spa.eye(n, format="csr") - t * (A.T @ A)).tocsr()
    Bt = (t * A.T).tocsr()
    At.eliminate_zeros()
    Bt.eliminate_zeros()
    return n, m, nnz_per_row, seed, lambd, b, t, At, Bt, A.nnz


def build_fom():
    n, m, k, seed, lambd, b, t, At, Bt, a_nnz = _instance()
    z0 = np.full(n, 2.0)
    threshold = lambd * t

    def build(gamma):
        v = Verifier(method="simulator", obbt_solver=None)
        b_param = v.add_param(m, lb=b - 1.0, ub=b + 1.0)
        y_prev = v.add_iterate(n, lb=z0, ub=z0)
        y_curr = v.add_iterate(n, lb=z0, ub=z0)
        extrapolated = y_curr + gamma * (y_curr - y_prev)
        y_next = v.step.soft_threshold(
            At @ extrapolated + Bt @ b_param,
            lambd=threshold,
        )
        return Recording(v, next_state=[y_curr, y_next], state=[y_prev, y_curr])

    def momentum():
        betas = [1.0]

        def gamma(k):
            while len(betas) <= k + 1:
                beta = betas[-1]
                betas.append((1.0 + np.sqrt(1.0 + 4.0 * beta * beta)) / 2.0)
            return (betas[k] - 1.0) / betas[k + 1]

        return gamma

    return {
        "build": build,
        "schedules": {"gamma": momentum()},
        "name": f"paper_fista_sparse_m{m}_n{n}_a{k}nnz_lambda002",
        "data": {
            "n": n,
            "m": m,
            "A_nnz": a_nnz,
            "At_nnz": At.nnz,
            "Bt_nnz": Bt.nnz,
            "lambda": lambd,
            "step_size": t,
            "seed": seed,
        },
    }
