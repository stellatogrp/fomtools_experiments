"""Plot the worst-case scaled KKT residual of rAPDHG and PDHG against K.

Reads the ``out/bounds_T{T}*.csv`` files that ``verify.verify`` writes and
produces, for horizon T:

* ``out/{name}.{pdf,png}``: top panel the MIP best bound (solid) and the
  simulator's sample max (hollow) against K, one color per run, with restart
  iterations marked; optionally a tolerance line at ``--eps`` and, per run, a
  star at its certified budget, the first K from which the bound stays at or
  below the tolerance through the last K. Bottom panel the MIP solve time.
* ``out/{name}_wide.csv``: one column per series, for pgfplots.

With ``--sim-samples N``, the simulator-only file that ``verify.verify --samples N``
wrote for a run is drawn as a dashed line in the run's color.

    python -m verify.plot --T 5 --runs 6 12 25 pdhg --eps 1.0 --name rapdhg_T5_restarts

A run is a restart period, optionally followed by ``ruiz`` for the equilibrated
variant and ``a<scale>`` for a step-size multiplier (``8``, ``8ruiz``, ``12a2``),
or ``pdhg`` for plain PDHG. A restart period of at least the largest K is labeled
"no restart".
"""

from __future__ import annotations

import argparse
import csv
import re
from collections import defaultdict
from pathlib import Path

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt  # noqa: E402
import numpy as np  # noqa: E402
from matplotlib.lines import Line2D  # noqa: E402

from verify.verify import RESTART  # noqa: E402

OUT = Path(__file__).parent / "out"
COLORS = ("C0", "C1", "C2", "C3", "C4")


def parse_run(run: str) -> tuple[int | None, bool, float]:
    """``(restart, ruiz, step_scale)``; ``restart`` is None for plain PDHG."""
    if run == "pdhg":
        return None, False, 1.0
    match = re.fullmatch(r"(\d+)(ruiz)?(?:a([\d.]+))?", run)
    if match is None:
        raise ValueError(f"cannot parse run {run!r}")
    return int(match[1]), match[2] is not None, float(match[3] or 1.0)


def results_file(T: int, restart: int | None, ruiz: bool = False, step_scale: float = 1.0) -> Path:
    if restart is None:
        return OUT / f"bounds_T{T}_pdhg.csv"
    suffix = ("" if restart == RESTART else f"_R{restart}") + ("_ruiz" if ruiz else "")
    suffix += f"_a{step_scale:g}" if step_scale != 1.0 else ""
    return OUT / f"bounds_T{T}{suffix}.csv"


def sim_file(
    T: int, restart: int | None, ruiz: bool, step_scale: float, samples: int
) -> Path | None:
    """The simulator-only file with ``samples`` samples for this run, if one exists."""
    base = results_file(T, restart, ruiz, step_scale)
    path = base.with_name(f"{base.stem}_sim{samples}.csv")
    return path if path.exists() else None


def load(path: Path) -> dict[str, dict[int, dict[str, float]]]:
    series: dict[str, dict[int, dict[str, float]]] = defaultdict(dict)
    with open(path) as f:
        for row in csv.DictReader(f):
            series[row["backend"]][int(row["K"])] = {
                key: float(row[key]) for key in ("value", "bound", "solve_time")
            }
    return series


def certified_budget(ks: np.ndarray, bound: np.ndarray, eps: float) -> int | None:
    """First K from which the bound stays at or below ``eps`` through the last K."""
    above = np.nonzero(bound > eps)[0]
    if above.size == 0:
        return int(ks[0])
    last_above = above[-1]
    return int(ks[last_above + 1]) if last_above + 1 < ks.size else None


def run_label(restart: int | None, ruiz: bool, step_scale: float, k_max: int) -> str:
    if restart is None:
        return "PDHG"
    label = "rAPDHG, " + ("no restart" if restart >= k_max else f"restart every {restart}")
    label += ", Ruiz" if ruiz else ""
    label += f", steps x{step_scale:g}" if step_scale != 1.0 else ""
    return label


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--T", type=int, default=5)
    parser.add_argument("--runs", nargs="+", default=["4", "8"])
    parser.add_argument("--eps", type=float, default=None, help="tolerance line and budgets")
    parser.add_argument("--sim-samples", type=int, default=None, help="dashed sample-max runs")
    parser.add_argument("--name", default=None, help="output stem; default rapdhg_T{T}")
    args = parser.parse_args()
    stem = args.name or f"rapdhg_T{args.T}"

    fig, (ax, ax_time) = plt.subplots(
        2, 1, figsize=(8.0, 4.8), sharex=True, gridspec_kw={"height_ratios": [3, 1.3]}
    )
    handles = []
    columns, header = [], []
    for color, run in zip(COLORS, args.runs):
        restart, ruiz, step_scale = parse_run(run)
        data = load(results_file(args.T, restart, ruiz, step_scale))
        ks = np.array(sorted(data["mip"]))
        bound = np.array([data["mip"][k]["bound"] for k in ks])
        sample = np.array([data["simulator"][k]["value"] for k in ks])
        times = np.array([data["mip"][k]["solve_time"] for k in ks])
        label = run_label(restart, ruiz, step_scale, int(ks.max()))

        (line,) = ax.semilogy(ks, bound, color=color, lw=1.6, marker="o", ms=3, label=label)
        handles.append(line)
        ax.semilogy(ks, sample, color=color, ls="none", marker="s", ms=6, mfc="none", mew=1.0)
        small_path = args.sim_samples and sim_file(args.T, restart, ruiz, step_scale, args.sim_samples)
        small = None
        if small_path:
            small_data = load(small_path)["simulator"]
            small = np.array([small_data[k]["value"] for k in ks])
            ax.semilogy(ks, small, color=color, ls="--", lw=1.2)
        if restart is not None and restart < ks.max():
            # Restart iterations: the step that ends each period.
            restarts = ks[ks % restart == 0]
            ax.semilogy(
                restarts,
                bound[restarts - 1],
                color=color,
                ls="none",
                marker="D",
                ms=6,
                mec="white",
                mew=1.0,
            )
        if args.eps is not None:
            budget = certified_budget(ks, bound, args.eps)
            if budget is not None:
                ax.semilogy(
                    [budget],
                    [bound[budget - 1]],
                    color=color,
                    ls="none",
                    marker="*",
                    ms=13,
                    mec="black",
                    mew=0.6,
                    zorder=5,
                )
            print(f"{label}: certified budget at eps = {args.eps}: {budget}")
        ax_time.plot(ks, times, color=color, lw=1.6, marker="o", ms=3)

        if not columns:
            columns.append(ks)
            header.append("K")
        columns += [bound, sample, times]
        tag = (
            "pdhg"
            if restart is None
            else (
                f"R{restart}"
                + ("_ruiz" if ruiz else "")
                + (f"_a{step_scale:g}" if step_scale != 1.0 else "")
            )
        )
        header += [f"bound_{tag}", f"sample_{tag}", f"time_{tag}"]
        if small is not None:
            columns.append(small)
            header.append(f"sample{args.sim_samples}_{tag}")

    if args.eps is not None:
        ax.axhline(args.eps, color="0.35", ls="--", lw=1.0)
        handles.append(
            Line2D([], [], color="0.35", ls="--", lw=1.0, label=rf"$\epsilon = {args.eps:g}$")
        )
        handles.append(
            Line2D(
                [],
                [],
                color="0.5",
                ls="none",
                marker="*",
                ms=11,
                mec="black",
                mew=0.6,
                label=r"certified budget ($\leq \epsilon$ from here on)",
            )
        )
    handles.append(
        Line2D([], [], color="0.5", ls="none", marker="s", ms=6, mfc="none", label="sample max")
    )
    if args.sim_samples:
        handles.append(
            Line2D(
                [], [], color="0.5", ls="--", lw=1.2, label=f"sample max, {args.sim_samples} samples"
            )
        )
    handles.append(
        Line2D([], [], color="0.5", ls="none", marker="D", ms=6, mec="white", label="restart")
    )
    ax.set_ylabel("worst-case scaled KKT residual")
    ax.grid(alpha=0.3, which="both")
    ax.legend(handles=handles, fontsize=7.5, loc="upper left", bbox_to_anchor=(1.02, 1.0))
    ax.set_title(f"Quadcopter control, horizon {args.T}; solid lines are MIP bounds", fontsize=9)
    ax_time.set_xlabel("$K$")
    ax_time.set_ylabel("MIP time (s)")
    ax_time.set_ylim(bottom=0)
    ax_time.grid(alpha=0.3)
    ax_time.set_xticks(columns[0][columns[0] % 4 == 0] if len(columns[0]) > 12 else columns[0])
    fig.tight_layout()
    for ext in ("pdf", "png"):
        fig.savefig(OUT / f"{stem}.{ext}", dpi=300)

    np.savetxt(
        OUT / f"{stem}_wide.csv",
        np.column_stack(columns),
        delimiter=",",
        header=",".join(header),
        comments="",
        fmt=["%d"] + ["%.6e"] * (len(columns) - 1),
    )


if __name__ == "__main__":
    main()
