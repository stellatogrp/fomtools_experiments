"""Plot worst-case unscaled residuals from ``results/bounds.csv``.

Writes ``results/nnls_bounds.{pdf,png}`` and a wide CSV
``results/nnls_bounds_wide.csv`` (one column per series) for pgfplots.
"""

from __future__ import annotations

import csv
from collections import defaultdict
from pathlib import Path

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt  # noqa: E402
import numpy as np  # noqa: E402

RESULTS = Path(__file__).parent / "results"
VARIANTS = {"unscaled": ("C0", "PGD"), "ruiz": ("C1", "Ruiz PGD")}
BACKENDS = {
    "mip": (dict(marker="o", linestyle="-", ms=3), "MIP"),
    "sdp": (dict(marker="x", linestyle="--", ms=6, zorder=3), "SDP"),
    "simulator": (dict(marker="s", linestyle=":", ms=3), "sample max"),
}


def main() -> None:
    series = defaultdict(dict)
    with open(RESULTS / "bounds.csv") as f:
        for row in csv.DictReader(f):
            series[row["variant"], row["backend"]][int(row["K"])] = float(row["value"])
    ks = np.array(sorted(next(iter(series.values()))))

    fig, ax = plt.subplots(figsize=(5, 3.2))
    columns, header = [ks], ["K"]
    for variant, (color, vlabel) in VARIANTS.items():
        for backend, (style, blabel) in BACKENDS.items():
            values = np.array([series[variant, backend][k] for k in ks])
            ax.semilogy(ks, values, color=color, label=f"{vlabel} {blabel}", **style)
            columns.append(values)
            header.append(f"{variant}_{backend}")
    ax.set_xlabel("$K$")
    ax.set_ylabel("worst-case residual (unscaled)")
    ax.set_xticks(ks)
    ax.legend(fontsize=8)
    ax.grid(alpha=0.3)
    fig.tight_layout()
    for ext in ("pdf", "png"):
        fig.savefig(RESULTS / f"nnls_bounds.{ext}", dpi=300)

    np.savetxt(
        RESULTS / "nnls_bounds_wide.csv",
        np.column_stack(columns),
        delimiter=",",
        header=",".join(header),
        comments="",
        fmt=["%d"] + ["%.6e"] * (len(columns) - 1),
    )


if __name__ == "__main__":
    main()
