"""Generate, check and time the solvers of one experiment.

    uv run python -m codegen_timings.run lasso --check-only     # no agent; seconds
    uv run python -m codegen_timings.run lasso --variants ista  # one variant, both agent phases

Run one experiment per process: the experiment folders each have a ``problem.py``.
A verified ``emitted/`` or ``optimized/`` is reused unless ``--force``.
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
    parser.add_argument("experiment", choices=["lasso", "portfolio", "network", "rapdhg"])
    parser.add_argument("--variants", nargs="*", default=None)
    parser.add_argument("--check-only", action="store_true")
    parser.add_argument("--force", action="store_true")
    args = parser.parse_args()

    add_control_to_path()
    cfg = load_config(args.experiment)
    module = load_recording_module(args.experiment)
    for variant in args.variants or module.VARIANTS:
        if variant not in module.VARIANTS:
            raise SystemExit(f"unknown variant {variant!r}; choose from {module.VARIANTS}")
        setup = module.build(variant, cfg)
        run_variant(args.experiment, variant, setup, args.check_only, args.force)


if __name__ == "__main__":
    main()
