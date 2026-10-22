"""Write the paper's code generation data from the timing runs, and print the numbers its text quotes.

    uv run python -m codegen_timings.paper_csv                     # dry run: print only
    uv run python -m codegen_timings.paper_csv --write             # write into the paper
    uv run python -m codegen_timings.paper_csv --only portfolio network --write

Every number is the median over the agent repetitions (``summarize``), so no number
in the paper is copied by hand. Writes, under ``--paper-dir`` (default
``~/Library/CloudStorage/Dropbox/work/research/papers/2026/fomtools/data``):

- ``codegen_timing.csv`` (``table``): one row per generated solver of the code
  generation tables, grouped by family;
- ``codegen_network_schedule.csv`` (``network``): momentum PDHG generated with a
  scheduled and an unrolled iteration, from ``codegen_timings/network_schedule``;
- ``codegen_portfolio_tolerance.csv`` (``portfolio``): time per solve to the tolerance
  of CVXPYgen with OSQP and with the fomtools ADMM backend, translated and
  optimized, against the number of assets, from
  ``portfolio/portfolio_cvxpygen/results_factor/portfolio_cvxpygen_stop_wide.csv``
  (``portfolio_cvxpygen.plot``).
"""

from __future__ import annotations

import argparse
import csv
import json
import re
import statistics
from pathlib import Path

from codegen_timings.common import HERE, REPO
from codegen_timings.summarize import _row, median_row, repetitions

PAPER_DATA = Path.home() / "Library/CloudStorage/Dropbox/work/research/papers/2026/fomtools/data"
PORTFOLIO_WIDE = (
    REPO
    / "portfolio"
    / "portfolio_cvxpygen"
    / "results_factor"
    / "portfolio_cvxpygen_stop_wide.csv"
)
SCHEDULE = HERE / "network_schedule"

#: (experiment, variant, group, problem title, method label), in table order.
ROWS = [
    ("lasso", "ista", "Lasso", "Lasso", "ISTA"),
    ("lasso", "fista", "Lasso", "", "FISTA"),
    ("network", "unscaled", "Network", "Network", "PDHG"),
    ("network", "ruiz", "Network", "", "PDHG (Ruiz)"),
    ("network", "unscaled_momentum", "Network", "", "momentum PDHG"),
    ("network", "ruiz_momentum", "Network", "", "momentum PDHG (Ruiz)"),
    ("rapdhg", "R12", "Control", "Control", r"rAPDHG ($K_\mathrm{r} = 12$)"),
    ("rapdhg", "R6", "Control", "", r"rAPDHG ($K_\mathrm{r} = 6$)"),
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


def _out(args, name: str, fields: list[str], rows: list[dict]) -> None:
    path = Path(args.paper_dir) / name
    if not args.write:
        print(f"(dry run) would write {path}: {len(rows)} rows")
        return
    with open(path, "w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=fields)
        writer.writeheader()
        writer.writerows(rows)
    print(f"wrote {path}")


def _timings(base: Path) -> list[dict]:
    """Every repetition's ``timing.json`` of the solver in ``base``."""
    return [json.loads(p.read_text()) for p in repetitions(base)]


def write_table(args) -> None:
    rows, medians, reps = [], [], []
    for e, v, group, problem, method in ROWS:
        runs = _timings(HERE / e / v)
        reps += runs
        t = median_row([_row(r) for r in runs])
        medians.append(t)
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
        print(
            f"{e}/{v}: {rows[-1]['optimizedus']} us [{t['optimized_us_min']:.2f}, "
            f"{t['optimized_us_max']:.2f}], total {rows[-1]['speedtotal']}x "
            f"[{t['speedup_total_min']:.0f}, {t['speedup_total_max']:.0f}] over {t['reps']} reps"
        )
    _out(args, "codegen_timing.csv", FIELDS, rows)

    for key in ("speedup_translation", "speedup_optimization", "speedup_total"):
        vals = [m[key] for m in medians]
        print(f"{key} (medians): {min(vals):.2f} .. {max(vals):.2f}")
    worst = max(max(d.values()) for t in reps for d in t["max_abs_dev"].values())
    print(f"worst max|C - NumPy| over every repetition: {worst:.2e}")
    print(f"all optimizations kept: {all(t['optimization_kept'] for t in reps)}")
    for arm in ("numpy", "translated", "optimized"):
        spreads = [
            (max(r) - min(r)) / statistics.median(r)
            for t in reps
            for r in [t["timing_rounds"][arm]]
        ]
        print(f"round spread {arm}: max (max-min)/median = {max(spreads):.1%}")
    translation = [t["agent_wall_s"]["translation"] for t in reps]
    optimization = [t["agent_wall_s"]["code_optimization"] for t in reps]
    print(
        f"agent time: translation {min(translation):.0f}-{max(translation):.0f} s, code "
        f"optimization {min(optimization) / 60:.1f}-{max(optimization) / 60:.1f} min "
        f"over {len(reps)} runs"
    )
    print(f"machine: {reps[0]['machine']}")


def write_network_schedule(args) -> None:
    """Scheduled vs unrolled momentum PDHG: median times and memory footprint (KB)."""
    header = (SCHEDULE / "scheduled" / "emitted" / "include" / "fom_data.h").read_text()
    sched_len = int(re.search(r"#define FOM_SCHED_XI_LEN (\d+)", header).group(1))
    rows = []
    for arm in ("scheduled", "unrolled"):
        runs = _timings(SCHEDULE / arm)
        med = {
            k: statistics.median(r[k] for r in runs)
            for k in ("numpy_us", "translated_us", "optimized_us", "speedup_total")
        }
        ir = runs[0]["size"]["ir"]
        lib = statistics.median(r["size"]["optimized"]["library_bytes"] for r in runs)
        # Every table entry and coefficient is a double.
        sched_kb = f"{sched_len * 8 / 1000:.1f}" if ir["schedules"] else "--"
        rows.append(
            dict(
                method=arm,
                steps=ir["body_steps"],
                blocks=ir["coefficient_blocks"],
                numpyus=f"{med['numpy_us']:.0f}",
                translatedus=f"{med['translated_us']:.1f}",
                optimizedus=f"{med['optimized_us']:.1f}",
                speedtotal=f"{med['speedup_total']:.1f}",
                schedkb=sched_kb,
                coefkb=f"{ir['coefficient_nonzeros'] * 8 / 1000:.0f}",
                libkb=f"{lib / 1000:.0f}",
            )
        )
        dev = max(max(r["max_abs_dev"].values()) for r in runs)
        times = [round(r["optimized_us"], 1) for r in runs]
        print(f"network {arm}: {rows[-1]}, optimized per rep {times}, max|C - NumPy| {dev:.1e}")
    _out(args, "codegen_network_schedule.csv", list(rows[0]), rows)


def write_portfolio_tolerance(args) -> None:
    """Time per solve to the tolerance (ms) and iterations, median over the repetitions."""
    times = dict(osqpms="cpg_osqp_c_ms", fomms="cpg_fom_c_ms", fomoptms="cpg_fom_opt_c_ms")
    iters = dict(osqpiters="cpg_osqp_mean_iters", fomiters="cpg_fom_opt_mean_iters")
    rows = []
    with open(PORTFOLIO_WIDE, newline="") as f:
        for r in csv.DictReader(f):
            row = dict(N=r["n"])
            row.update({k: f"{float(r[c]):.3f}" for k, c in times.items()})
            row.update({k: f"{float(r[c]):.2f}" for k, c in iters.items()})
            rows.append(row)
            osqp, fom, opt = (float(r[c]) for c in times.values())
            print(
                f"portfolio n={r['n']}: OSQP/optimized {osqp / opt:.2f}x, translated/OSQP {fom / osqp:.2f}"
            )
    _out(args, "codegen_portfolio_tolerance.csv", ["N", *times, *iters], rows)


WRITERS = {
    "table": write_table,
    "network": write_network_schedule,
    "portfolio": write_portfolio_tolerance,
}


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--only", nargs="*", default=list(WRITERS), choices=list(WRITERS))
    parser.add_argument("--paper-dir", default=str(PAPER_DATA), help="the paper's data/ folder")
    parser.add_argument("--write", action="store_true", help="write the CSVs (default: dry run)")
    args = parser.parse_args()
    for name in args.only:
        WRITERS[name](args)


if __name__ == "__main__":
    main()
