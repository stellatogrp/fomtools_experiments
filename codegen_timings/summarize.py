"""Collect every ``<exp>/<variant>/timing.json`` into ``codegen_timings/timings.csv``.

    uv run python -m codegen_timings.summarize

One row per generated solver, in the columns of the paper's §6.6 table
(NumPy, translated and optimized microseconds per solve, and the three
speedups), plus the agreement with NumPy and whether the optimization was kept.
"""

from __future__ import annotations

import csv
import json

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


def main() -> None:
    rows = []
    for experiment in RUNS:
        for path in sorted((HERE / experiment).glob("*/timing.json")):
            t = json.loads(path.read_text())
            rows.append(
                {
                    **{k: t[k] for k in FIELDS if k in t},
                    "max_abs_dev_translated": max(t["max_abs_dev"]["translated"].values()),
                    "max_abs_dev_optimized": max(t["max_abs_dev"]["optimized"].values()),
                }
            )
    out = HERE / "timings.csv"
    with open(out, "w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=FIELDS)
        writer.writeheader()
        writer.writerows(rows)
    for r in rows:
        print(
            f"{r['experiment']:9s} {r['variant']:17s} NumPy {r['numpy_us']:9.2f} us  "
            f"translated {r['translated_us']:8.3f} us  optimized {r['optimized_us']:8.3f} us  "
            f"x{r['speedup_translation']:.0f} x{r['speedup_optimization']:.2f} x{r['speedup_total']:.0f}"
        )
    print(f"wrote {out} ({len(rows)} rows)")


if __name__ == "__main__":
    main()
