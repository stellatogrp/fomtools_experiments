"""Generate, check and time the solvers of one experiment.

    uv run python -m codegen_timings.run lasso --check-only     # no agent; seconds
    FOMTOOLS_CODEGEN_AGENT=1 uv run python -m codegen_timings.run lasso --force   # reps 1 2 3
    FOMTOOLS_CODEGEN_AGENT=1 uv run python -m codegen_timings.run lasso --variants ista --reps 2

Run one experiment per process: the experiment folders each have a ``problem.py``.
Each repetition is an independent pair of agent runs (translation, then code
optimization); rep 1 lives in ``<experiment>/<variant>/``, the others in
``<experiment>/<variant>/reps/rep<k>/``, and the variants of one repetition run
before the next repetition starts. A verified ``emitted/`` or ``optimized/`` is
reused unless ``--force``.
"""

from __future__ import annotations

import argparse

from codegen_timings.common import (
    add_control_to_path,
    load_config,
    load_recording_module,
    run_variant,
)


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("experiment", choices=["lasso", "network", "rapdhg"])
    parser.add_argument("--variants", nargs="*", default=None)
    parser.add_argument("--check-only", action="store_true")
    parser.add_argument("--force", action="store_true")
    parser.add_argument("--reps", nargs="+", type=int, default=[1, 2, 3])
    args = parser.parse_args()

    add_control_to_path()
    cfg = load_config(args.experiment)
    module = load_recording_module(args.experiment)
    variants = args.variants or module.VARIANTS
    for variant in variants:
        if variant not in module.VARIANTS:
            raise SystemExit(f"unknown variant {variant!r}; choose from {module.VARIANTS}")
    setups = {variant: module.build(variant, cfg) for variant in variants}
    for rep in [1] if args.check_only else args.reps:
        for variant in variants:
            run_variant(args.experiment, variant, setups[variant], args.check_only, args.force, rep)


if __name__ == "__main__":
    main()
