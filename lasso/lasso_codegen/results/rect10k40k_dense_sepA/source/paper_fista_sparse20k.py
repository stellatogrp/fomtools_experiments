"""Sparse rectangular Lasso FISTA instance, recorded with A and A^T kept separate.

Same instance and step size as paper_ista_sparse20k.py in this directory. Each
iteration is written as two steps, with y = x_k + gamma_k (x_k - x_{k-1}):

    r = A y - b                              (materialized; products with A)
    x_{k+1} = soft(y - t A^T r, lambda t)    (one product with A^T)

so no A^T A is formed. y is deliberately not its own step: FOMTools pads a step
that does not reference every state slot with dense zero blocks, which at this
size would be a 325k x 1.3M dense array.
"""

import numpy as np

from fomtools.codegen.extract_scheduled import Recording
from fomtools.verifier import Verifier
from paper_ista_sparse20k import MATERIALIZE, _instance, step_size_of


def build_fom():
    n, m, k, seed, lambd, A, b = _instance()
    step_size = step_size_of(A)
    z0 = np.full(n, 2.0)
    threshold = lambd * step_size
    At_scaled = (step_size * A.T).tocsr()

    def build(gamma):
        v = Verifier(method="simulator", obbt_solver=None)
        b_param = v.add_param(m, lb=b - 1.0, ub=b + 1.0)
        y_prev = v.add_iterate(n, lb=z0, ub=z0)
        y_curr = v.add_iterate(n, lb=z0, ub=z0)
        y = y_curr + gamma * (y_curr - y_prev)
        r = v.step.soft_threshold(A @ y - b_param, lambd=MATERIALIZE)
        y_next = v.step.soft_threshold(y - At_scaled @ r, lambd=threshold)
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
        "name": f"paper_fista_sparse_m{m}_n{n}_a{k}nnz_lambda002_sepA",
        "data": {
            "n": n,
            "m": m,
            "A_nnz": A.nnz,
            "lambda": lambd,
            "step_size": step_size,
            "seed": seed,
        },
    }
