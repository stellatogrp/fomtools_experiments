"""Write the paper's §6.2–6.5 figure CSVs from the cluster runs, and print the facts the notes quote.

    uv run python plots/export_paper_data.py
    uv run python plots/export_paper_data.py --only network   # rewrite one experiment's CSV

Uses the runs and K cut-offs of ``make_plots.RUNS`` and keeps the column names
the tikz code in ``fomtools_paper/main.tex`` reads, so the figures pick up the
new data without edits to their ``\\addplot`` lines.
"""

from __future__ import annotations

import argparse
import csv
from pathlib import Path

import numpy as np
from make_plots import REPO, RUNS, load, read

PAPER_DATA = REPO.parent / "fomtools_paper" / "data"
GAP_OK = 0.0501


def fmt(x: float) -> str:
    return "" if np.isnan(x) else f"{x:.6e}"


ONLY: set[str] = set()


def write(name: str, fields: list[str], rows: list[dict], experiment: str) -> None:
    if ONLY and experiment not in ONLY:
        return
    with open(PAPER_DATA / name, "w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=fields)
        writer.writeheader()
        writer.writerows(rows)
    print(f"wrote {PAPER_DATA / name} ({len(rows)} rows)")


def problem_info(run: Path, variant: str) -> dict:
    return {r["name"]: r["value"] for r in csv.DictReader(open(run / variant / "problem.csv"))}


def prob95(name: str, variant: str, k_max: int) -> list[float]:
    """The KL 95% quantile bound of ``kl_bounds.py`` (plots/kl_<name>.csv), K = 1..k_max."""
    rows = list(csv.DictReader(open(REPO / "plots" / f"kl_{name}.csv")))
    return [float(r[f"{variant}_kl_q95"]) for r in rows[:k_max]]


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--only", nargs="*", default=[], choices=list(RUNS))
    ONLY.update(parser.parse_args().only)
    data = {}
    for name, (run, k_max, variants) in RUNS.items():
        data[name] = {v: load(REPO / run, v, k_max) for v in variants}
        for v in variants:
            for r, p in zip(data[name][v], prob95(name, v, k_max), strict=True):
                r["prob95"] = p

    # Lasso: <m>_mip is the certified bound (as before), <m>_simulator1000 the sample max,
    # <m>_prob95 the KL 95% quantile bound of kl_bounds.py.
    rows, fields = [], ["K"]
    for v in ("ista", "fista"):
        fields += [f"{v}_mip", f"{v}_incumbent", f"{v}_gap", f"{v}_simulator1000", f"{v}_prob95"]
    for i, K in enumerate(r["K"] for r in data["lasso"]["ista"]):
        row = {"K": K}
        for v in ("ista", "fista"):
            r = data["lasso"][v][i]
            row.update({f"{v}_mip": fmt(r["bound"]), f"{v}_incumbent": fmt(r["incumbent"]),
                        f"{v}_gap": f"{r['gap']:.4f}", f"{v}_simulator1000": fmt(r["sample"]),
                        f"{v}_prob95": fmt(r["prob95"])})
        rows.append(row)
    write("lasso_residual.csv", fields, rows, "lasso")

    # Network: <v>_simulator is the 10^3-sample max, as before.
    net = ["unscaled", "ruiz", "unscaled_momentum", "ruiz_momentum"]
    rows, fields = [], ["K"]
    for v in net:
        fields += [f"{v}_mip_bound", f"{v}_incumbent", f"{v}_gap", f"{v}_simulator", f"{v}_prob95"]
    for i, K in enumerate(r["K"] for r in data["network"]["unscaled"]):
        row = {"K": K}
        for v in net:
            r = data["network"][v][i]
            row.update({f"{v}_mip_bound": fmt(r["bound"]), f"{v}_incumbent": fmt(r["incumbent"]),
                        f"{v}_gap": f"{r['gap']:.4f}", f"{v}_simulator": fmt(r["sample"]),
                        f"{v}_prob95": fmt(r["prob95"])})
        rows.append(row)
    write("network_residual.csv", fields, rows, "network")

    # rAPDHG: bound_<v>, sample1000_<v>, time_<v> (wall time per K, s), gap_<v>.
    run_c = REPO / RUNS["rapdhg"][0]
    ctl = ["R12", "R6"]
    times = {v: read(run_c / v / "bounds.csv") for v in ctl}
    rows, fields = [], ["K"]
    for v in ctl:
        fields += [f"bound_{v}", f"sample1000_{v}", f"time_{v}", f"gap_{v}", f"prob95_{v}"]
    for i, K in enumerate(r["K"] for r in data["rapdhg"]["R12"]):
        row = {"K": K}
        for v in ctl:
            r = data["rapdhg"][v][i]
            row.update({f"bound_{v}": fmt(r["bound"]), f"sample1000_{v}": fmt(r["sample"]),
                        f"time_{v}": f"{float(times[v][K]['t_total']):.4f}",
                        f"gap_{v}": f"{r['gap']:.4f}", f"prob95_{v}": fmt(r["prob95"])})
        rows.append(row)
    write("control_rapdhg.csv", fields, rows, "rapdhg")

    # ---- facts for the notes ----------------------------------------------------
    print("\n== facts")
    for name, by_v in data.items():
        for v, rows_v in by_v.items():
            b = np.array([r["bound"] for r in rows_v])
            loose = [r["K"] for r in rows_v if r["gap"] > GAP_OK]
            rises = [int(k) for k in np.where(np.diff(b) > 0)[0] + 2]
            print(f"{name}/{v}: bound K=1 {b[0]:.4g}, K=end {b[-1]:.4g}, min {b.min():.4g} at K={int(b.argmin()) + 1}; "
                  f"gap>5% at {loose}; bound rises at K={rises}")
    for name, by_v in data.items():
        for v, rows_v in by_v.items():
            ratio = np.array([r["bound"] / r["prob95"] for r in rows_v])
            over = [r["K"] for r in rows_v if r["prob95"] > r["bound"]]
            print(f"{name}/{v}: MIP / prob95 min {ratio.min():.2f} median {np.median(ratio):.2f} "
                  f"max {ratio.max():.2f}; prob95 above MIP at K={over}")
    lasso = data["lasso"]
    first_better = next((r["K"] for r, s in zip(lasso["fista"], lasso["ista"]) if r["bound"] < s["bound"] - 1e-9), None)
    print(f"lasso: FISTA bound below ISTA from K={first_better}; equal for K<=2: "
          f"{all(abs(a['bound'] - b['bound']) < 1e-9 for a, b in zip(lasso['ista'][:2], lasso['fista'][:2]))}")
    nw = data["network"]
    for K in (20, RUNS["network"][1]):
        vals = {v: nw[v][K - 1]["bound"] for v in nw}
        print(f"network K={K}: " + ", ".join(f"{v} {x:.3f}" for v, x in vals.items()))
    worse = [r["K"] for r, u in zip(nw["ruiz"], nw["unscaled"]) if r["bound"] > u["bound"]]
    print(f"network: ruiz above unscaled at K={worse}")
    info_u = problem_info(REPO / RUNS["network"][0], "unscaled")
    info_r = problem_info(REPO / RUNS["network"][0], "ruiz")
    print(f"network: ||A|| {float(info_u['norm_A']):.3f} -> {float(info_r['norm_A_scaled']):.3f}")
    print(f"lasso eta: {float(problem_info(REPO / RUNS['lasso'][0], 'ista')['eta']):.4f}")
    ct = data["rapdhg"]
    for v in ct:
        b = [r["bound"] for r in ct[v]]
        below = [k + 1 for k in range(len(b)) if all(x < 1.0 for x in b[k:])]
        print(f"rapdhg/{v}: certified budget for eps=1 within K<=30: {below[0] if below else None} "
              f"(bound there {b[below[0] - 1]:.4f})" if below else f"rapdhg/{v}: never stays below 1")
    for K in (25, 30):
        print(f"rapdhg K={K}: " + ", ".join(f"{v} {ct[v][K - 1]['bound']:.3f}" for v in ct))


if __name__ == "__main__":
    main()
