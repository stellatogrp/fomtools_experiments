"""Sparse rectangular Lasso ISTA instance, recorded with A and A^T kept separate.

Same instance, step size and algorithm as the rect studies (seed 0, lambda 0.02,
two column permutations, step 1 / (||A||_1 ||A||_infinity)). One change to how
the method is written: the gradient A^T (A z - b) is applied as two sparse
products. The residual r = A z - b is materialized as its own step (a
soft-threshold with lambda = 0, which is exactly the identity), so extraction
does not fuse A^T A into one matrix.
"""

import os

import numpy as np
import scipy.sparse as spa

from fomtools.verifier import Verifier

#: Materializing step: soft-threshold at 0 is exactly the identity, and unlike a
#: saturated-linear step it stores no bound vectors in the IR.
MATERIALIZE = 0.0


def _instance():
    m = int(os.environ.get("FOMTOOLS_EXPERIMENT_M", "10000"))
    n = int(os.environ.get("FOMTOOLS_EXPERIMENT_N", "40000"))
    nnz_per_row = int(os.environ.get("FOMTOOLS_EXPERIMENT_A_NNZ_PER_ROW", "8"))
    seed = 0
    lambd = 0.02
    rng = np.random.default_rng(seed)

    edge_count = m * nnz_per_row
    if edge_count % n:
        raise ValueError("m * nnz_per_row must be divisible by n for balanced columns")
    rows = np.repeat(np.arange(m, dtype=np.int32), nnz_per_row)
    cols = np.concatenate(
        [rng.permutation(n) for _ in range(edge_count // n)]
    ).astype(np.int32)
    values = rng.normal(scale=1.0 / np.sqrt(nnz_per_row), size=edge_count)
    A = spa.coo_matrix((values, (rows, cols)), shape=(m, n)).tocsr()
    A.sum_duplicates()
    A.eliminate_zeros()
    b = rng.normal(size=m)
    return n, m, nnz_per_row, seed, lambd, A, b


def step_size_of(A):
    """1 / (||A||_1 ||A||_infinity), exactly as the rect studies compute it."""
    one_norm = float(np.asarray(np.abs(A).sum(axis=0)).max())
    inf_norm = float(np.asarray(np.abs(A).sum(axis=1)).max())
    return 1.0 / (one_norm * inf_norm)


def build_fom():
    n, m, k, seed, lambd, A, b = _instance()
    step_size = step_size_of(A)
    z0 = np.full(n, 2.0)
    At_scaled = (step_size * A.T).tocsr()

    v = Verifier(method="simulator", obbt_solver=None)
    b_param = v.add_param(m, lb=b - 1.0, ub=b + 1.0)
    z = v.add_iterate(n, lb=z0, ub=z0)

    iterates = []
    for _ in range(3):
        r = v.step.soft_threshold(A @ z - b_param, lambd=MATERIALIZE)
        z = v.step.soft_threshold(z - At_scaled @ r, lambd=lambd * step_size)
        iterates.append(z)

    return {
        "verifier": v,
        "next_state": iterates[0],
        "iterates": iterates,
        "param_values": {b_param: b},
        "name": f"paper_ista_sparse_m{m}_n{n}_a{k}nnz_lambda002_sepA",
        "data": {
            "n": n,
            "m": m,
            "A_nnz": A.nnz,
            "lambda": lambd,
            "step_size": step_size,
            "seed": seed,
        },
    }
