"""Plot worst-case KKT residuals from ``results/<study>_bounds.csv``.

For each study (``milp``, ``sdp``) writes ``results/lasso_<study>.{pdf,png}``
and a wide CSV ``results/lasso_<study>_wide.csv`` (one column per series) for
pgfplots. With ``--sim-samples N``, the sample max from ``results/<study>_sim<N>.csv``
(written by ``verify.py --samples N``) is added as a dashed line and as the
columns ``<method>_simulator<N>``.
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
METHODS = {"ista": ("C0", "ISTA"), "fista": ("C1", "FISTA")}
BACKENDS = {
    "mip": (dict(marker="o", linestyle="-", ms=3), "MIP bound"),
    "sdp": (dict(marker="x", linestyle="--", ms=6, zorder=3), "SDP"),
    "miqp": (dict(marker="o", linestyle="-", ms=3), "exact (MIQP)"),
    # Hollow and larger, so a sample max that sits on its MIP bound stays visible.
    "simulator": (dict(marker="s", linestyle=":", ms=7, mfc="none", mew=1.2), "sample max"),
}

YLABELS = {
    "milp": r"worst-case KKT residual ($\ell_\infty$)",
    "sdp": r"worst-case KKT residual ($\ell_2$)",
}


def plot_study(study: str, sim_samples: int | None = None) -> None:
    series = defaultdict(dict)
    with open(RESULTS / f"{study}_bounds.csv") as f:
        for row in csv.DictReader(f):
            series[row["method"], row["backend"]][int(row["K"])] = float(row["value"])
    sim_path = RESULTS / f"{study}_sim{sim_samples}.csv"
    if sim_samples and sim_path.exists():
        with open(sim_path) as f:
            for row in csv.DictReader(f):
                key = (row["method"], f"simulator{sim_samples}")
                series[key][int(row["K"])] = float(row["value"])
    ks = np.array(sorted(next(iter(series.values()))))

    fig, ax = plt.subplots(figsize=(5, 3.2))
    columns, header = [ks], ["K"]
    for method, (color, mlabel) in METHODS.items():
        for backend, (style, blabel) in BACKENDS.items():
            if (method, backend) not in series:
                continue
            values = np.array([series[method, backend][k] for k in ks])
            ax.semilogy(ks, values, color=color, label=f"{mlabel} {blabel}", **style)
            columns.append(values)
            header.append(f"{method}_{backend}")
        small = f"simulator{sim_samples}"
        if (method, small) in series:
            values = np.array([series[method, small][k] for k in ks])
            ax.semilogy(ks, values, color=color, ls="--", lw=1.2, label=f"{mlabel} sample max, {sim_samples} samples")
            columns.append(values)
            header.append(f"{method}_{small}")
    ax.set_xlabel("$K$")
    ax.set_ylabel(YLABELS[study])
    ax.set_xticks(ks)
    ax.legend(fontsize=8)
    ax.grid(alpha=0.3)
    fig.tight_layout()
    for ext in ("pdf", "png"):
        fig.savefig(RESULTS / f"lasso_{study}.{ext}", dpi=300)

    np.savetxt(
        RESULTS / f"lasso_{study}_wide.csv",
        np.column_stack(columns),
        delimiter=",",
        header=",".join(header),
        comments="",
        fmt=["%d"] + ["%.6e"] * (len(columns) - 1),
    )


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--sim-samples", type=int, default=None, help="dashed sample-max runs")
    args = parser.parse_args()
    for study in YLABELS:
        if (RESULTS / f"{study}_bounds.csv").exists():
            plot_study(study, args.sim_samples)


if __name__ == "__main__":
    main()
