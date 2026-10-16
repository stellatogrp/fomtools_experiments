"""Plot solve time against the number of assets from ``results/timings.csv``.

Run from ``portfolio/``:

    python -m portfolio_cvxpygen.plot

Left: time per ``problem.solve`` call through Python, for all five arms (the
analogue of the CVXPYgen paper's Fig. 4). Right: the generated code alone, for
the three CVXPYgen solvers. One colour per tool, solid for a fixed 200
iterations (OSQP extras off, and the fomtools solver), dashed for OSQP with its
defaults. Writes ``results/portfolio_cvxpygen_scaling.{pdf,png}`` and the wide
CSV ``results/portfolio_cvxpygen_scaling_wide.csv`` (one column per series) for
pgfplots.
"""

from __future__ import annotations

import argparse
import csv
from collections import defaultdict

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt  # noqa: E402
import numpy as np  # noqa: E402

from portfolio_cvxpygen.model import RESULTS  # noqa: E402

ARMS = {
    "cvxpy_osqp_off": ("C0", "-", "CVXPY + OSQP, extras off"),
    "cvxpy_osqp_on": ("C0", "--", "CVXPY + OSQP, defaults"),
    "cpg_osqp": ("C1", "-", "CVXPYgen + OSQP, defaults"),
    "cpg_fom": ("#74c476", "-", "CVXPYgen + fomtools, translated"),
    "cpg_fom_opt": ("#006d2c", "-", "CVXPYgen + fomtools, optimized"),
}
LEVELS = {"python": "through Python (problem.solve)", "c": "generated C code"}


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--tol", action="store_true", help="plot timings_tol.csv (time to tolerance)")
    args = parser.parse_args()
    suffix = "_tol" if args.tol else ""

    series = defaultdict(dict)
    with open(RESULTS / f"timings{suffix}.csv") as f:
        for row in csv.DictReader(f):
            series[row["arm"], row["level"]][int(row["N"])] = float(row["us_per_solve"])
    sizes = np.array(sorted({n for values in series.values() for n in values}))

    fig, axes = plt.subplots(1, 2, figsize=(9, 3.4), sharey=True)
    columns, header = [sizes], ["N"]
    for ax, (level, title) in zip(axes, LEVELS.items()):
        for arm, (color, linestyle, label) in ARMS.items():
            if (arm, level) not in series:
                continue
            values = np.array([series[arm, level].get(n, np.nan) for n in sizes])
            ax.loglog(sizes, values, color=color, linestyle=linestyle, marker="o", ms=3,
                      label=label)
            columns.append(values)
            header.append(f"{arm}_{level}")
        ax.set_title(title, fontsize=10)
        ax.set_xlabel("number of assets $N$")
        ax.grid(True, which="major", alpha=0.3)
    axes[0].set_ylabel(r"time per solve ($\mu$s)")
    handles, labels = axes[0].get_legend_handles_labels()
    fig.legend(handles, labels, loc="upper center", ncol=3, fontsize=8, frameon=False)
    fig.tight_layout(rect=(0, 0, 1, 0.86))
    for ext in ("pdf", "png"):
        fig.savefig(RESULTS / f"portfolio_cvxpygen_scaling{suffix}.{ext}", dpi=200)

    with open(RESULTS / f"portfolio_cvxpygen_scaling{suffix}_wide.csv", "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(header)
        writer.writerows(np.column_stack(columns).tolist())
    print(f"wrote {RESULTS / f'portfolio_cvxpygen_scaling{suffix}.pdf'}")


if __name__ == "__main__":
    main()
