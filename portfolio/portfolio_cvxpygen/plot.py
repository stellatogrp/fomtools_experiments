"""Plot the self-stopping experiment from ``results_factor/timings_stop.csv``.

Run from ``portfolio/``:

    python -m portfolio_cvxpygen.plot

Plots ``timings_stop.csv`` (``tolerance_stop``: every solver stops
itself at 1e-4): (a) time per solve, C solid and Python dashed; (b) iterations to
the tolerance, mean with the largest as a whisker. Two styles: ``paper`` (LaTeX
serif, paper-size fonts) and ``slides`` (Helvetica Neue through XeLaTeX, large
fonts). Writes ``portfolio_cvxpygen_stop_{paper,slides}.{pdf,png}`` and the wide
CSV ``portfolio_cvxpygen_stop_wide.csv``. ``slides_c`` is a single slide panel:
time per solve of the generated C code, CVXPYgen + OSQP vs the two fomtools
solvers, with the speedup of the optimized one at each n
(``portfolio_cvxpygen_stop_slides_c.{pdf,png}``).
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

# Okabe-Ito hues (colour-blind safe), softened: a reference, OSQP, fomtools x 2.
STOP_ARMS = {
    "cvxpy_osqp": ("#8a96a3", "CVXPY + OSQP"),
    "cpg_osqp": ("#e0a33b", "CVXPYgen + OSQP"),
    "cpg_fom": ("#7fbfe0", "CVXPYgen + FOMTools, translated"),
    "cpg_fom_opt": ("#1f6fa8", "CVXPYgen + FOMTools, optimized"),
}
STYLES = {
    "paper": dict(
        rc={"text.usetex": True, "font.family": "serif", "font.serif": ["Computer Modern Roman"],
            "font.size": 9, "axes.labelsize": 9, "legend.fontsize": 8,
            "xtick.labelsize": 8, "ytick.labelsize": 8},
        figsize=(6.5, 2.6), lw=1.4, ms=4,
    ),
    "slides": dict(
        rc={"pgf.texsystem": "xelatex", "pgf.rcfonts": False, "font.family": "sans-serif",
            "font.size": 22, "axes.labelsize": 22, "legend.fontsize": 18,
            "xtick.labelsize": 18, "ytick.labelsize": 18,
            # Helvetica Neue for text; math stays in LaTeX's Computer Modern.
            "pgf.preamble": "\\usepackage{fontspec}\\setmainfont{Helvetica Neue}"
                            "\\setsansfont{Helvetica Neue}"},
        figsize=(15, 6.2), lw=3, ms=9,
    ),
}


def load_stop(path):
    """``{(arm, level): {N: row}}`` from a ``tolerance_stop`` CSV."""
    rows = defaultdict(dict)
    with open(path) as f:
        for row in csv.DictReader(f):
            rows[row["arm"], row["level"]][int(row["N"])] = row
    return rows


def plot_stop(path, style: str) -> None:
    rows = load_stop(path)
    sizes = np.array(sorted({n for values in rows.values() for n in values}))
    spec = STYLES[style]
    with plt.rc_context(spec["rc"]):
        fig, (ax_t, ax_k) = plt.subplots(1, 2, figsize=spec["figsize"])
        for arm, (color, label) in STOP_ARMS.items():
            for level, ls, marker in (("c", "-", "o"), ("python", "--", "s")):
                values = rows.get((arm, level), {})
                ms_ = [float(values[n]["us_per_solve"]) / 1e3 if n in values and values[n]["us_per_solve"]
                       else np.nan for n in sizes]
                if np.all(np.isnan(ms_)):
                    continue
                ax_t.loglog(sizes, ms_, color=color, ls=ls, marker=marker, lw=spec["lw"],
                            ms=spec["ms"], mfc="white" if level == "python" else color)
            if arm == "cpg_fom":
                continue  # same iterations as the optimized solver
            values = rows[arm, "python"]
            mean = np.array([float(values[n]["mean_iters"]) for n in sizes])
            top = np.array([float(values[n]["max_iters"]) for n in sizes])
            name = "CVXPYgen + FOMTools" if arm == "cpg_fom_opt" else label
            shift = {"cvxpy_osqp": 0.96, "cpg_osqp": 1.0, "cpg_fom_opt": 1.04}[arm]  # whiskers apart
            ax_k.errorbar(sizes * shift, mean, yerr=[np.zeros_like(mean), top - mean], color=color,
                          marker="o", lw=spec["lw"], ms=spec["ms"], capsize=spec["ms"] / 2,
                          label=name)
        ax_t.set_ylabel("time per solve (ms)")
        ax_k.set_ylabel(r"iterations to $10^{-4}$")
        ax_k.set_xscale("log")
        ax_k.set_ylim(bottom=0)
        for ax, tag in ((ax_t, "(a)"), (ax_k, "(b)")):
            ax.set_xlabel("number of assets $n$")
            ax.set_xticks(sizes, [str(n) for n in sizes])
            ax.minorticks_off()
            ax.grid(True, which="major", alpha=0.3)
            ax.set_title(tag, loc="left")
        from matplotlib.lines import Line2D
        arm_handles = [Line2D([], [], color=c, lw=spec["lw"], label=l) for c, l in STOP_ARMS.values()]
        level_handles = [
            Line2D([], [], color="0.3", ls="-", marker="o", lw=spec["lw"], ms=spec["ms"], label="C"),
            Line2D([], [], color="0.3", ls="--", marker="s", mfc="white", lw=spec["lw"], ms=spec["ms"],
                   label="Python"),
        ]
        fig.legend(handles=arm_handles + level_handles, loc="upper center", ncol=3, frameon=False)
        fig.tight_layout(rect=(0, 0, 1, 0.8 if style == "paper" else 0.86))
        for ext in ("pdf", "png"):
            out = RESULTS / f"portfolio_cvxpygen_stop_{style}.{ext}"
            fig.savefig(out, dpi=200, **({"backend": "pgf"} if style == "slides" else {}))
        plt.close(fig)
    print(f"wrote {RESULTS / f'portfolio_cvxpygen_stop_{style}.pdf'}")


def plot_stop_slides_c(path) -> None:
    rows = load_stop(path)
    sizes = np.array(sorted(rows["cpg_osqp", "c"]))
    spec = STYLES["slides"]
    ms = {arm: np.array([float(rows[arm, "c"][n]["us_per_solve"]) / 1e3 for n in sizes])
          for arm in ("cpg_osqp", "cpg_fom", "cpg_fom_opt")}
    with plt.rc_context(spec["rc"]):
        fig, ax = plt.subplots(figsize=(11, 6.5))
        for arm, values in ms.items():
            color, label = STOP_ARMS[arm]
            ax.loglog(sizes, values, color=color, marker="o", lw=spec["lw"], ms=spec["ms"],
                      label=label)
        for n, a, b in zip(sizes, ms["cpg_osqp"], ms["cpg_fom_opt"]):
            ax.annotate(f"{a / b:.1f}$\\times$", (n, b), textcoords="offset points",
                        xytext=(0, -34), ha="center", color=STOP_ARMS["cpg_fom_opt"][0])
        ax.set_xlabel("number of assets $n$")
        ax.set_ylabel("time per solve, C (ms)")
        ax.set_xticks(sizes, [str(n) for n in sizes])
        ax.minorticks_off()
        ax.set_ylim(ms["cpg_fom_opt"].min() / 2.2, ms["cpg_osqp"].max() * 1.6)
        ax.grid(True, which="major", alpha=0.3)
        ax.legend(loc="upper left", frameon=False)
        fig.tight_layout()
        for ext in ("pdf", "png"):
            fig.savefig(RESULTS / f"portfolio_cvxpygen_stop_slides_c.{ext}", dpi=200, backend="pgf")
        plt.close(fig)
    print(f"wrote {RESULTS / 'portfolio_cvxpygen_stop_slides_c.pdf'}")


def write_stop_wide(path) -> None:
    """One row per n: time (ms) per arm and level, mean and largest iterations per arm."""
    rows = load_stop(path)
    sizes = sorted({n for values in rows.values() for n in values})
    header, columns = ["n"], [sizes]
    for arm in STOP_ARMS:
        for level in ("c", "python"):
            if (arm, level) in rows:
                values = rows[arm, level]
                header.append(f"{arm}_{level}_ms")
                columns.append([float(values[n]["us_per_solve"]) / 1e3 if values[n]["us_per_solve"]
                                else "" for n in sizes])
        values = rows[arm, "python"]
        header += [f"{arm}_mean_iters", f"{arm}_max_iters"]
        columns += [[values[n]["mean_iters"] for n in sizes], [values[n]["max_iters"] for n in sizes]]
    out = RESULTS / "portfolio_cvxpygen_stop_wide.csv"
    with open(out, "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(header)
        writer.writerows(zip(*columns))
    print(f"wrote {out}")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--csv", type=str, default=None, help="CSV to plot")
    parser.add_argument("--style", nargs="+", choices=[*STYLES, "slides_c"],
                        default=[*STYLES, "slides_c"])
    args = parser.parse_args()
    path = args.csv or RESULTS / "timings_stop.csv"
    write_stop_wide(path)
    for style in args.style:
        plot_stop_slides_c(path) if style == "slides_c" else plot_stop(path, style)


if __name__ == "__main__":
    main()
