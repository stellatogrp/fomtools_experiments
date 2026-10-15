"""Write the paper's §6.6 table data from the timing runs, and print the numbers its text quotes.

    uv run python -m codegen_timings.paper_csv

Writes ``../fomtools_paper/data/codegen_timing.csv`` (one row per generated
solver, grouped by family) so no number in the table is copied by hand.
"""

from __future__ import annotations

import csv
import json
import statistics

from codegen_timings.common import HERE, REPO

PAPER_CSV = REPO.parent / "fomtools_paper" / "data" / "codegen_timing.csv"

#: (experiment, variant, group, problem title, method label), in table order.
ROWS = [
    ("lasso", "ista", "Lasso", "Lasso", "ISTA"),
    ("lasso", "fista", "Lasso", "", "FISTA"),
    ("portfolio", "admm", "Portfolio", "Portfolio", "ADMM"),
    ("network", "unscaled", "Network", "Network", "PDHG"),
    ("network", "ruiz", "Network", "", "PDHG (Ruiz)"),
    ("network", "unscaled_momentum", "Network", "", "momentum PDHG"),
    ("network", "ruiz_momentum", "Network", "", "momentum PDHG (Ruiz)"),
    ("rapdhg", "R12", "Control", "Control", r"rAPDHG ($K_\mathrm{r} = 12$)"),
    ("rapdhg", "R6", "Control", "", r"rAPDHG ($K_\mathrm{r} = 6$)"),
    ("rapdhg", "pdhg", "Control", "", "PDHG"),
]
FIELDS = [
    "group",
    "problem",
    "method",
    "numpyus",
    "translatedus",
    "optimizedus",
    "speedtrans",
    "speedopt",
    "speedtotal",
]


def main() -> None:
    timings = {(e, v): json.loads((HERE / e / v / "timing.json").read_text()) for e, v, *_ in ROWS}
    rows = []
    for e, v, group, problem, method in ROWS:
        t = timings[(e, v)]
        rows.append(
            dict(
                group=group,
                problem=problem,
                method=method,
                numpyus=f"{t['numpy_us']:.0f}",
                translatedus=f"{t['translated_us']:.2f}",
                optimizedus=f"{t['optimized_us']:.2f}",
                speedtrans=f"{t['speedup_translation']:.0f}",
                speedopt=f"{t['speedup_optimization']:.1f}",
                speedtotal=f"{t['speedup_total']:.0f}",
            )
        )
    with open(PAPER_CSV, "w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=FIELDS)
        writer.writeheader()
        writer.writerows(rows)
    print(f"wrote {PAPER_CSV}")

    ts = list(timings.values())

    def rng(key):
        vals = [t[key] for t in ts]
        return min(vals), max(vals)

    for key in ("speedup_translation", "speedup_optimization", "speedup_total"):
        lo, hi = rng(key)
        print(f"{key}: {lo:.2f} .. {hi:.2f}")
    worst = max(max(d.values()) for t in ts for d in t["max_abs_dev"].values())
    print(f"worst max|C - NumPy|: {worst:.2e}")
    print(f"all optimizations kept: {all(t['optimization_kept'] for t in ts)}")

    for arm in ("numpy", "translated", "optimized"):
        spreads = []
        for t in ts:
            r = t["timing_rounds"][arm]
            spreads.append((max(r) - min(r)) / statistics.median(r))
        print(f"round spread {arm}: max (max-min)/median = {max(spreads):.1%}")
    for (e, v), t in timings.items():
        a = t["agent_wall_s"]
        neon = (HERE / e / v / "optimized" / "src" / "fom_solver.c").read_text().count("arm_neon.h")
        print(
            f"{e}/{v}: translation {a['translation']:.0f} s, code optimization "
            f"{a['code_optimization'] / 60:.1f} min, NEON={bool(neon)}"
        )
    print(f"machine: {ts[0]['machine']}")


if __name__ == "__main__":
    main()
