"""Draft plots: the MIP bound with the KL 95% quantile bound in place of the sample max.

    uv run python plots/kl_bounds.py     # first: writes plots/kl_<experiment>.csv
    uv run python plots/kl_plots.py

Same runs, K cut-offs and styling as ``make_plots.py``; writes
``plots/draft_klplots/<experiment>.pdf`` and ``.png``.
"""

from __future__ import annotations

import csv

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt  # noqa: E402
import numpy as np  # noqa: E402
from make_plots import COLORS, GAP_OK, GRID, INK, MUTED, OUT, REPO, RUNS, label_positions, load  # noqa: E402
from matplotlib.ticker import FuncFormatter  # noqa: E402

DRAFT = OUT / "draft_klplots"


def kl_quantile(name: str, variant: str, k_max: int) -> tuple[list[float], str]:
    rows = list(csv.DictReader(open(OUT / f"kl_{name}.csv")))
    col = next(c for c in rows[0] if c.startswith(f"{variant}_kl_q") and not c.endswith("_perK"))
    return [float(r[col]) for r in rows[:k_max]], col.rsplit("_q", 1)[1]


def plot(name: str, k_max: int, variants: list[str], run: str) -> None:
    fig, ax = plt.subplots(figsize=(5.4, 3.8), layout="constrained")
    handles, ends = [], []
    for color, v in zip(COLORS, variants):
        rows = load(REPO / run, v, k_max)
        K = [r["K"] for r in rows]
        bound = [r["bound"] for r in rows]
        kl, q = kl_quantile(name, v, k_max)
        (line,) = ax.plot(K, bound, color=color, lw=2, label=v)
        handles.append(line)
        ax.plot(K, kl, color=color, lw=1.5, ls="--")
        loose = [r for r in rows if r["gap"] > GAP_OK]
        ax.plot([r["K"] for r in loose], [r["bound"] for r in loose], ls="none", marker="o",
                ms=6, mfc="white", mec=color, mew=1.5)
        ends.append(bound[-1])
    ax.set_yscale("log")
    lo, hi = ax.get_ylim()
    for v, y in zip(variants, label_positions(ends, min_sep=0.055 * np.log10(hi / lo))):
        ax.annotate(v, (k_max, y), xytext=(5, 0), textcoords="offset points", va="center",
                    fontsize=8, color=INK)
    ax.set_xlim(1, k_max + max(3, k_max // 4))

    def minor(y, _):
        lead = round(y / 10 ** np.floor(np.log10(y)))
        return f"{y:g}" if lead in (2, 5) else ""

    ax.yaxis.set_major_formatter(FuncFormatter(lambda y, _: f"{y:g}"))
    ax.yaxis.set_minor_formatter(FuncFormatter(minor))
    ax.set_xlabel("K", color=INK)
    ax.set_ylabel("residual", color=INK)
    ax.set_title(name, color=INK, fontsize=11, loc="left")
    ax.grid(True, which="major", color=GRID, lw=0.8)
    for side in ("top", "right"):
        ax.spines[side].set_visible(False)
    for side in ("left", "bottom"):
        ax.spines[side].set_color(MUTED)
    ax.tick_params(which="both", colors=MUTED, labelsize=8)
    handles += [
        plt.Line2D([], [], color=MUTED, lw=2, label="MIP bound"),
        plt.Line2D([], [], color=MUTED, lw=1.5, ls="--", label=f"KL {q}% bound (δ = 0.05)"),
        plt.Line2D([], [], color=MUTED, ls="none", marker="o", mfc="white", label="gap > 5%"),
    ]
    fig.legend(handles=handles, loc="outside lower center", ncol=4 if len(handles) <= 5 else 3,
               fontsize=8, frameon=False)
    for ext in ("pdf", "png"):
        fig.savefig(DRAFT / f"{name}.{ext}", dpi=200)
    plt.close(fig)
    print(f"wrote {(DRAFT / name).relative_to(REPO)}.{{pdf,png}}")


def main() -> None:
    DRAFT.mkdir(exist_ok=True)
    for name, (run, k_max, variants) in RUNS.items():
        plot(name, k_max, variants, run)


if __name__ == "__main__":
    main()
