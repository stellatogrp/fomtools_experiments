"""Basic plots and tikz-ready CSVs of the cluster runs, cut at each experiment's K.

    uv run python plots/make_plots.py

For every experiment it writes, into plots/:

- ``<experiment>.csv``: one row per K, with ``<variant>_bound`` (Gurobi's best
  bound, the certified upper bound), ``<variant>_incumbent``, ``<variant>_gap``,
  ``<variant>_status`` and ``<variant>_sample`` (max over the simulator samples);
- ``<experiment>.pdf`` and ``.png``: the bound (solid) and the sample max
  (dashed) per variant on a log scale. A hollow marker flags a K whose gap is
  above 5% (time-limited), where the bound is valid but loose.

The runs and the K cut-offs are set in RUNS below.
"""

from __future__ import annotations

import csv
from pathlib import Path

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt  # noqa: E402
import numpy as np  # noqa: E402
from matplotlib.ticker import FuncFormatter  # noqa: E402

REPO = Path(__file__).resolve().parents[1]
OUT = Path(__file__).resolve().parent

#: experiment -> (run folder, K cut-off, variants in plotting order)
RUNS = {
    "lasso": ("lasso/lasso_verify/results/run_022", 20, ["ista", "fista"]),
    "portfolio": ("portfolio/results/run_002", 100, ["simplex"]),
    "network": (
        "network/results/run_005",
        39,
        ["unscaled", "ruiz", "unscaled_momentum", "ruiz_momentum"],
    ),
    "rapdhg": ("control/rapdhg/verify/results/run_001", 75, ["R12", "R6"]),
}

#: Categorical slots 1-4 of the dataviz reference palette, in fixed order.
COLORS = ["#2a78d6", "#eb6834", "#1baf7a", "#eda100"]
INK, MUTED, GRID = "#1f1f1e", "#6b6a64", "#e4e3dc"
GAP_OK = 0.0501


def read(path: Path) -> dict[int, dict]:
    with open(path, newline="") as f:
        return {int(r["K"]): r for r in csv.DictReader(f) if not r.get("error")}


def load(run: Path, variant: str, k_max: int) -> list[dict]:
    bounds = read(run / variant / "bounds.csv")
    sim = read(run / variant / "simulator.csv")
    rows = []
    for K in range(1, k_max + 1):
        b, s = bounds.get(K), sim.get(K)
        rows.append(
            dict(
                K=K,
                bound=float(b["bound"]) if b else float("nan"),
                incumbent=float(b["incumbent"]) if b else float("nan"),
                gap=float(b["gap"]) if b else float("nan"),
                status=b["status"] if b else "",
                sample=float(s["max"]) if s else float("nan"),
            )
        )
    return rows


def write_csv(name: str, data: dict[str, list[dict]], k_max: int) -> None:
    fields = ["K"]
    for v in data:
        fields += [f"{v}_{c}" for c in ("bound", "incumbent", "gap", "status", "sample")]
    with open(OUT / f"{name}.csv", "w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=fields)
        writer.writeheader()
        for i in range(k_max):
            row = {"K": i + 1}
            for v, rows in data.items():
                r = rows[i]
                row.update(
                    {
                        f"{v}_bound": f"{r['bound']:.6e}",
                        f"{v}_incumbent": f"{r['incumbent']:.6e}",
                        f"{v}_gap": f"{r['gap']:.4f}",
                        f"{v}_status": r["status"],
                        f"{v}_sample": f"{r['sample']:.6e}",
                    }
                )
            writer.writerow(row)


def label_positions(ends: list[float], min_sep: float) -> list[float]:
    """Spread the right-end labels apart in log10 space, keeping their order."""
    logs = np.log10(ends)
    order = np.argsort(logs)
    placed = logs.copy()
    for a, b in zip(order[:-1], order[1:]):
        placed[b] = max(placed[b], placed[a] + min_sep)
    # Re-centre so the labels stay next to their curves on average.
    placed += np.mean(logs - placed)
    return list(10**placed)


def plot(name: str, data: dict[str, list[dict]], k_max: int) -> None:
    fig, ax = plt.subplots(figsize=(5.4, 3.8), layout="constrained")
    handles = []
    for color, (v, rows) in zip(COLORS, data.items()):
        K = [r["K"] for r in rows]
        (line,) = ax.plot(K, [r["bound"] for r in rows], color=color, lw=2, label=v)
        handles.append(line)
        ax.plot(K, [r["sample"] for r in rows], color=color, lw=1.5, ls="--")
        loose = [r for r in rows if r["gap"] > GAP_OK]
        ax.plot(
            [r["K"] for r in loose],
            [r["bound"] for r in loose],
            ls="none",
            marker="o",
            ms=6,
            mfc="white",
            mec=color,
            mew=1.5,
        )
    ax.set_yscale("log")
    lo, hi = ax.get_ylim()
    decades = np.log10(hi / lo)
    # Direct labels at the right ends, nudged apart so they never overlap.
    ends = [rows[-1]["bound"] for rows in data.values()]
    for v, y in zip(data, label_positions(ends, min_sep=0.055 * decades)):
        ax.annotate(
            v,
            (k_max, y),
            xytext=(5, 0),
            textcoords="offset points",
            va="center",
            fontsize=8,
            color=INK,
        )
    ax.set_xlim(1, k_max + max(3, k_max // 4))

    # One number format on the y axis; of the minor ticks, label only 2x and 5x.
    def minor(y, _):
        lead = round(y / 10 ** np.floor(np.log10(y)))
        return f"{y:g}" if lead in (2, 5) else ""

    ax.yaxis.set_major_formatter(FuncFormatter(lambda y, _: f"{y:g}"))
    ax.yaxis.set_minor_formatter(FuncFormatter(minor))
    ax.set_xlabel("K", color=INK)
    ax.set_ylabel("worst-case residual", color=INK)
    ax.set_title(name, color=INK, fontsize=11, loc="left")
    ax.grid(True, which="major", color=GRID, lw=0.8)
    for side in ("top", "right"):
        ax.spines[side].set_visible(False)
    for side in ("left", "bottom"):
        ax.spines[side].set_color(MUTED)
    ax.tick_params(which="both", colors=MUTED, labelsize=8)
    handles += [
        plt.Line2D([], [], color=MUTED, lw=2, label="MIP bound"),
        plt.Line2D([], [], color=MUTED, lw=1.5, ls="--", label="sample max"),
        plt.Line2D([], [], color=MUTED, ls="none", marker="o", mfc="white", label="gap > 5%"),
    ]
    # The legend sits below the axes so it never covers a curve.
    fig.legend(
        handles=handles,
        loc="outside lower center",
        ncol=min(len(handles), 4),
        fontsize=8,
        frameon=False,
    )
    for ext in ("pdf", "png"):
        fig.savefig(OUT / f"{name}.{ext}", dpi=200)
    plt.close(fig)


def main() -> None:
    for name, (run, k_max, variants) in RUNS.items():
        data = {v: load(REPO / run, v, k_max) for v in variants}
        write_csv(name, data, k_max)
        plot(name, data, k_max)
        loose = {v: [r["K"] for r in rows if r["gap"] > GAP_OK] for v, rows in data.items()}
        print(f"{name}: K = 1..{k_max} from {run}; K with gap > 5%: {loose}")


if __name__ == "__main__":
    main()
