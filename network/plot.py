"""Plot worst-case original-LP KKT residuals from ``results/bounds_mipinf.csv``.

One panel per method (PDHG, momentum PDHG), each comparing the unscaled and the
Ruiz-equilibrated variant. Writes ``results/network_bounds_mipinf.{pdf,png}`` and a
wide CSV ``results/network_bounds_mipinf_wide.csv`` (one column per series) for pgfplots.
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
METHODS = {"": "PDHG", "_momentum": "Momentum PDHG"}
SCALINGS = {"unscaled": ("C0", "unscaled"), "ruiz": ("C1", "Ruiz")}
BACKENDS = {
    "mip": (dict(marker="o", linestyle="-", ms=3), "MIP"),
    "simulator": (dict(marker="s", linestyle=":", ms=3), "sample max"),
}


def main() -> None:
    series = defaultdict(dict)
    with open(RESULTS / "bounds_mipinf.csv") as f:
        for row in csv.DictReader(f):
            series[row["variant"], row["backend"]][int(row["K"])] = float(row["value"])
    ks = np.array(sorted(next(iter(series.values()))))

    fig, axes = plt.subplots(1, 2, figsize=(8, 3.2), sharey=True)
    columns, header = [ks], ["K"]
    for ax, (suffix, title) in zip(axes, METHODS.items()):
        for scaling, (color, slabel) in SCALINGS.items():
            for backend, (style, blabel) in BACKENDS.items():
                name = scaling + suffix
                values = np.array([series[name, backend][k] for k in ks])
                ax.plot(ks, values, color=color, label=f"{slabel} {blabel}", **style)
                columns.append(values)
                header.append(f"{name}_{backend}")
        ax.set_title(title, fontsize=10)
        ax.set_xlabel("$K$")
        ax.set_xticks(ks[:: max(1, len(ks) // 10)])
        ax.grid(alpha=0.3)
    axes[0].set_ylabel("worst-case KKT residual (original LP)")
    axes[0].legend(fontsize=8)
    fig.tight_layout()
    for ext in ("pdf", "png"):
        fig.savefig(RESULTS / f"network_bounds_mipinf.{ext}", dpi=300)

    np.savetxt(
        RESULTS / "network_bounds_mipinf_wide.csv",
        np.column_stack(columns),
        delimiter=",",
        header=",".join(header),
        comments="",
        fmt=["%d"] + ["%.6e"] * (len(columns) - 1),
    )


if __name__ == "__main__":
    main()
