"""Collect every ``timing.json`` into ``codegen_timings/timings.csv`` and ``timings_reps.csv``.

    uv run python -m codegen_timings.summarize

One row per generated solver, in the columns of the paper's §6.6 table
(NumPy, translated and optimized microseconds per solve, and the three
speedups), plus the agreement with NumPy and whether the optimization was kept.

The agent is nondeterministic, so each solver is generated several times
(``run --reps``): ``<variant>/timing.json`` is repetition 1 and
``<variant>/reps/rep<k>/timing.json`` the others. Every repetition is a row of
``timings_reps.csv``; ``timings.csv`` has the median over them, with the range of
the optimized time and of the speedups.
"""

from __future__ import annotations

import csv
import json
import statistics
from pathlib import Path

from codegen_timings.common import HERE, RUNS

FIELDS = [
    "experiment",
    "variant",
    "iterations",
    "numpy_us",
    "translated_us",
    "optimized_us",
    "speedup_translation",
    "speedup_optimization",
    "speedup_total",
    "optimization_kept",
    "max_abs_dev_translated",
    "max_abs_dev_optimized",
    "verified_run",
]
#: Columns that are medians over the repetitions.
NUMERIC = [
    "numpy_us",
    "translated_us",
    "optimized_us",
    "speedup_translation",
    "speedup_optimization",
    "speedup_total",
]
#: ... with their range alongside.
RANGED = ["optimized_us", "speedup_optimization", "speedup_total"]


def repetitions(base: Path) -> list[Path]:
    """The ``timing.json`` of every repetition of the solver in ``base``."""
    return [
        p for p in [base / "timing.json", *sorted(base.glob("reps/rep*/timing.json"))] if p.exists()
    ]


def _row(t: dict) -> dict:
    return {
        **{k: t[k] for k in FIELDS if k in t},
        "rep": t.get("rep", 1),
        "max_abs_dev_translated": max(t["max_abs_dev"]["translated"].values()),
        "max_abs_dev_optimized": max(t["max_abs_dev"]["optimized"].values()),
        "fomtools_commit": t["machine"].get("fomtools_commit", ""),
        "translation_s": t["agent_wall_s"]["translation"],
        "code_optimization_s": t["agent_wall_s"]["code_optimization"],
        "rounds": len(t.get("optimization_rounds", [])),
    }


def median_row(runs: list[dict]) -> dict:
    """The median of ``runs`` (the repetitions of one solver), with ranges."""
    row = {k: runs[0][k] for k in FIELDS if k in runs[0]}
    row.update({k: statistics.median(r[k] for r in runs) for k in NUMERIC})
    row["optimization_kept"] = all(r["optimization_kept"] for r in runs)
    for k in ("max_abs_dev_translated", "max_abs_dev_optimized"):
        row[k] = max(r[k] for r in runs)
    row["reps"] = len(runs)
    for k in RANGED:
        row[f"{k}_min"] = min(r[k] for r in runs)
        row[f"{k}_max"] = max(r[k] for r in runs)
    return row


def _write(path: Path, fields: list[str], rows: list[dict]) -> None:
    with open(path, "w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=fields, extrasaction="ignore")
        writer.writeheader()
        writer.writerows(rows)


def main() -> None:
    reps, rows = [], []
    for experiment in RUNS:
        for base in sorted(p.parent for p in (HERE / experiment).glob("*/timing.json")):
            runs = [_row(json.loads(p.read_text())) for p in repetitions(base)]
            reps += runs
            rows.append(median_row(runs))
    ranged = [f"{k}_{e}" for k in RANGED for e in ("min", "max")]
    _write(HERE / "timings.csv", [*FIELDS, "reps", *ranged], rows)
    rep_fields = [
        "rep",
        *FIELDS,
        "fomtools_commit",
        "translation_s",
        "code_optimization_s",
        "rounds",
    ]
    _write(HERE / "timings_reps.csv", rep_fields, reps)
    for r in rows:
        print(
            f"{r['experiment']:9s} {r['variant']:17s} NumPy {r['numpy_us']:9.2f} us  "
            f"translated {r['translated_us']:8.3f} us  optimized {r['optimized_us']:8.3f} us "
            f"[{r['optimized_us_min']:.2f}, {r['optimized_us_max']:.2f}]  "
            f"x{r['speedup_translation']:.0f} x{r['speedup_optimization']:.2f} "
            f"x{r['speedup_total']:.0f} [{r['speedup_total_min']:.0f}, {r['speedup_total_max']:.0f}]"
            f"  ({r['reps']} reps)"
        )
    print(f"wrote {HERE / 'timings.csv'} ({len(rows)} rows), timings_reps.csv ({len(reps)} runs)")


if __name__ == "__main__":
    main()
