"""Plot worst-case ADMM residuals from ``results/bounds.csv``.

Writes ``results/portfolio_bounds.{pdf,png}`` and a wide CSV
``results/portfolio_bounds_wide.csv`` (one column per series) for pgfplots.
With ``--sim-samples N``, the sample max from ``results/bounds_sim<N>.csv``
(written by ``verify.py --samples N``) is added as a dashed line and as the
columns ``<set>_simulator<N>``.
"""

from __future__ import annotations

import argparse
import csv
from collections import defaultdict
from pathlib import Path

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt  # noqa: E402
import numpy as np  # noqa: E402

RESULTS = Path(__file__).parent / "results"
PARAM_SETS = {"box": ("C0", "box"), "simplex": ("C1", "simplex")}
BACKENDS = {
    "mip_bound": (dict(marker="o", linestyle="-", ms=3), "MIP bound"),
    "mip": (dict(marker=".", linestyle="--", ms=3, alpha=0.6), "MIP incumbent"),
    "sdp": (dict(marker="x", linestyle="--", ms=6, zorder=3), "SDP"),
    # Hollow and larger, so a sample max that sits on its MIP bound stays visible.
    "simulator": (dict(marker="s", linestyle=":", ms=7, mfc="none", mew=1.2), "sample max"),
}


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--sim-samples", type=int, default=None, help="dashed sample-max runs")
    args = parser.parse_args()
    small = f"simulator{args.sim_samples}"
    styles = dict(BACKENDS)
    if args.sim_samples:
        styles[small] = (dict(linestyle="--", lw=1.2), f"sample max, {args.sim_samples} samples")
        with open(RESULTS / f"bounds_sim{args.sim_samples}.csv") as f:
            sim_rows = list(csv.DictReader(f))
    else:
        sim_rows = []

    series = defaultdict(dict)
    with open(RESULTS / "bounds.csv") as f:
        for row in csv.DictReader(f):
            key, k = (row["param_set"], row["backend"]), int(row["K"])
            series[key][k] = float(row["value"])
            # The MIP stops at a relative gap, so its best bound is the certified worst case.
            if row["backend"] == "mip" and row.get("bound") not in (None, "", "nan"):
                series[row["param_set"], "mip_bound"][k] = float(row["bound"])
    for row in sim_rows:
        series[row["param_set"], small][int(row["K"])] = float(row["value"])
    ks = np.array(sorted(next(iter(series.values()))))

    fig, ax = plt.subplots(figsize=(5, 3.2))
    columns, header = [ks], ["K"]
    for param_set, (color, plabel) in PARAM_SETS.items():
        for backend, (style, blabel) in styles.items():
            if (param_set, backend) not in series:
                continue
            values = np.array([series[param_set, backend][k] for k in ks])
            ax.semilogy(ks, values, color=color, label=f"{plabel} {blabel}", **style)
            columns.append(values)
            header.append(f"{param_set}_{backend}")
    ax.set_xlabel("$K$")
    ax.set_ylabel("worst-case residual")
    ax.set_xticks(ks)
    ax.legend(fontsize=8)
    ax.grid(alpha=0.3)
    fig.tight_layout()
    for ext in ("pdf", "png"):
        fig.savefig(RESULTS / f"portfolio_bounds.{ext}", dpi=300)

    np.savetxt(
        RESULTS / "portfolio_bounds_wide.csv",
        np.column_stack(columns),
        delimiter=",",
        header=",".join(header),
        comments="",
        fmt=["%d"] + ["%.6e"] * (len(columns) - 1),
    )


if __name__ == "__main__":
    main()
