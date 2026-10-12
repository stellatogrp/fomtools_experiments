"""Repository paths and the numbered run folders ``<experiment>/results/run_NNN``."""

from __future__ import annotations

import re
from pathlib import Path

REPO = Path(__file__).resolve().parents[1]
CONF = REPO / "conf"
RUN_PATTERN = re.compile(r"^run_(\d{3,})$")


def results_dir(experiment_dir: str) -> Path:
    return REPO / experiment_dir / "results"


def taken_numbers(results: Path) -> set[int]:
    if not results.is_dir():
        return set()
    return {
        int(m.group(1))
        for p in results.iterdir()
        if p.is_dir() and (m := RUN_PATTERN.match(p.name))
    }


def peek_next_run_dir(results: Path) -> Path:
    """The folder ``next_run_dir`` would create, without creating it."""
    return results / f"run_{max(taken_numbers(results), default=0) + 1:03d}"


def next_run_dir(results: Path) -> Path:
    """Create and return the next ``run_NNN`` folder.

    ``mkdir`` without ``exist_ok`` is the lock: if two submissions pick the same
    number at once, the loser retries with the next one.
    """
    results.mkdir(parents=True, exist_ok=True)
    while True:
        run = peek_next_run_dir(results)
        try:
            run.mkdir()
            return run
        except FileExistsError:
            continue
