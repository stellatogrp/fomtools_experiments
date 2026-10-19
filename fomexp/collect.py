"""Merge a run's per-variant CSVs.

    python -m fomexp.collect lasso run_001        # or a path to the run folder

Writes two files into the run folder:

- ``summary.csv``: every variant's ``bounds.csv`` rows stacked, each joined with
  that variant's simulator statistics at the same K (``sim_*`` columns);
- ``wide.csv``: one row per K, with the columns named as the experiment's
  ``plot.py`` and the paper's ``data/*.csv`` name them, so the existing plotting
  code reads it unchanged.
"""

from __future__ import annotations

import csv
import sys
from pathlib import Path

from fomexp.paths import REPO

DIRS = {
    "lasso": "lasso/lasso_verify",
    "network": "network",
    "rapdhg": "control/rapdhg/verify",
}

#: Wide-column templates: ``{v}`` is the variant; the value is (source, column).
WIDE = {
    "lasso": {"{v}_mip": ("bounds", "bound"), "{v}_simulator": ("sim", "max")},
    "network": {"{v}_mip_bound": ("bounds", "bound"), "{v}_simulator": ("sim", "max")},
    "rapdhg": {
        "bound_{v}": ("bounds", "bound"),
        "sample_{v}": ("sim", "max"),
        "time_{v}": ("bounds", "t_total"),
    },
}


def read(path: Path) -> list[dict]:
    if not path.exists():
        return []
    with open(path, newline="") as f:
        return list(csv.DictReader(f))


def write(path: Path, rows: list[dict]) -> None:
    fields: list[str] = []
    for row in rows:
        fields += [k for k in row if k not in fields]
    with open(path, "w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=fields, restval="")
        writer.writeheader()
        writer.writerows(rows)


def collect(run: Path, experiment: str) -> None:
    variants = sorted(
        p.name for p in run.iterdir() if p.is_dir() and (p / "manifest.json").exists()
    )
    summary, wide = [], {}
    for v in variants:
        bounds = {int(r["K"]): r for r in read(run / v / "bounds.csv")}
        sim = {int(r["K"]): r for r in read(run / v / "simulator.csv")}
        for K, row in sorted(bounds.items()):
            joined = dict(row)
            for key, value in sim.get(K, {}).items():
                if key not in ("experiment", "variant", "K"):
                    joined[f"sim_{key}"] = value
            summary.append(joined)
        for K in sorted(set(bounds) | set(sim)):
            row = wide.setdefault(K, {"K": K})
            for template, (source, column) in WIDE[experiment].items():
                table = bounds if source == "bounds" else sim
                row[template.format(v=v)] = table.get(K, {}).get(column, "")
    last = {
        v: max((int(r["K"]) for r in read(run / v / "bounds.csv")), default=0) for v in variants
    }
    # The simulator runs to k_max; the wide table stops at the largest K any MIP reached.
    k_end = max(last.values(), default=0) or max(wide, default=0)
    write(run / "summary.csv", summary)
    write(run / "wide.csv", [wide[K] for K in sorted(wide) if K <= k_end])
    print(f"{run}: variants {variants}, last MIP K {last}")
    print(f"  wrote {run / 'summary.csv'} and {run / 'wide.csv'}")


def main() -> None:
    if len(sys.argv) != 3 or sys.argv[1] not in DIRS:
        raise SystemExit(f"usage: python -m fomexp.collect {{{','.join(DIRS)}}} run_NNN")
    experiment, run = sys.argv[1], Path(sys.argv[2])
    if not run.is_absolute() and not run.exists():
        run = REPO / DIRS[experiment] / "results" / run
    collect(run, experiment)


if __name__ == "__main__":
    main()
