"""FOMTools recordings of the fixed ISTA and scheduled FISTA loops."""

from __future__ import annotations

import numpy as np
from fomtools.codegen.extract_scheduled import Recording
from fomtools.verifier import Verifier

from lasso.problem import LassoProblem


def record_ista(problem: LassoProblem, *, check_iterations: int = 3) -> dict:
    """Record ISTA and include successive iterates for the stationarity gate."""
    verifier = Verifier(method="simulator", obbt_solver=None)
    b_param = verifier.add_param(
        problem.m,
        lb=problem.b - 1.0,
        ub=problem.b + 1.0,
    )
    z = verifier.add_iterate(problem.n, lb=problem.initial, ub=problem.initial)
    iterates = []
    for _ in range(check_iterations):
        z = verifier.step.soft_threshold(
            problem.state_map @ z + problem.param_map @ b_param,
            lambd=problem.threshold,
        )
        iterates.append(z)
    return {
        "verifier": verifier,
        "next_state": iterates[0],
        "iterates": iterates,
        "param_values": {b_param: problem.b},
        "name": "paper_ista_m40_n80_lambda002",
    }


def record_fista(problem: LassoProblem) -> dict:
    """Record FISTA as one two-slot iteration plus its momentum schedule."""

    def build(gamma: float) -> Recording:
        verifier = Verifier(method="simulator", obbt_solver=None)
        b_param = verifier.add_param(
            problem.m,
            lb=problem.b - 1.0,
            ub=problem.b + 1.0,
        )
        previous = verifier.add_iterate(
            problem.n,
            lb=problem.initial,
            ub=problem.initial,
        )
        current = verifier.add_iterate(
            problem.n,
            lb=problem.initial,
            ub=problem.initial,
        )
        extrapolated = current + gamma * (current - previous)
        following = verifier.step.soft_threshold(
            problem.state_map @ extrapolated + problem.param_map @ b_param,
            lambd=problem.threshold,
        )
        return Recording(
            verifier,
            next_state=[current, following],
            state=[previous, current],
        )

    def momentum():
        betas = [1.0]

        def gamma(k: int) -> float:
            while len(betas) <= k + 1:
                beta = betas[-1]
                betas.append((1.0 + np.sqrt(1.0 + 4.0 * beta * beta)) / 2.0)
            return (betas[k] - 1.0) / betas[k + 1]

        return gamma

    return {
        "build": build,
        "schedules": {"gamma": momentum()},
        "name": "paper_fista_m40_n80_lambda002",
    }
