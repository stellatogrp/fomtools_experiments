"""Write the paper's code generation data from the timing runs, and print the numbers its text quotes.

    uv run python -m codegen_timings.paper_csv
    uv run python -m codegen_timings.paper_csv --only portfolio network

Writes, under ``../fomtools_paper/data/``, so no number in the paper is copied by hand:

- ``codegen_timing.csv`` (``table``): one row per generated solver of §6.6, grouped by family;
- ``codegen_portfolio_scaling.csv`` (``portfolio``): C time per solve of CVXPYgen with
  OSQP and with the fomtools ADMM backend against the number of assets, from
  ``portfolio/portfolio_cvxpygen/results/timings.csv``;
- ``codegen_network_schedule.csv`` (``network``): momentum PDHG generated with a
  scheduled and an unrolled iteration, from ``codegen_timings/network_schedule``.
"""

from __future__ import annotations

import argparse
import csv
import json
import re
import statistics

from codegen_timings.common import HERE, REPO

PAPER_DATA = REPO.parent / "fomtools_paper" / "data"
PAPER_CSV = PAPER_DATA / "codegen_timing.csv"
PORTFOLIO_TIMINGS = REPO / "portfolio" / "portfolio_cvxpygen" / "results" / "timings.csv"
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


def write_table() -> None:
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


def write_portfolio_scaling() -> None:
    """C time per 200-iteration solve against the number of assets."""
    arms = {"cpg_osqp": "osqpus", "cpg_fom": "fomus", "cpg_fom_opt": "fomoptus"}
    by_n: dict[int, dict] = {}
    with open(PORTFOLIO_TIMINGS, newline="") as f:
        for r in csv.DictReader(f):
            if r["level"] == "c" and r["arm"] in arms:
                by_n.setdefault(int(r["N"]), {})[arms[r["arm"]]] = float(r["us_per_solve"])
    out = PAPER_DATA / "codegen_portfolio_scaling.csv"
    with open(out, "w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=["N", "osqpus", "fomus", "fomoptus", "speedosqp", "speedopt"])
        writer.writeheader()
        for n in sorted(by_n):
            t = by_n[n]
            writer.writerow(
                dict(
                    N=n,
                    osqpus=f"{t['osqpus']:.2f}",
                    fomus=f"{t['fomus']:.2f}",
                    fomoptus=f"{t['fomoptus']:.2f}",
                    speedosqp=f"{t['osqpus'] / t['fomoptus']:.2f}",
                    speedopt=f"{t['fomus'] / t['fomoptus']:.2f}",
                )
            )
            print(
                f"portfolio N={n}: OSQP {t['osqpus']:.1f}, fomtools {t['fomus']:.1f} -> {t['fomoptus']:.1f} us; "
                f"OSQP/optimized {t['osqpus'] / t['fomoptus']:.2f}x, translated/optimized {t['fomus'] / t['fomoptus']:.2f}x"
            )
    print(f"wrote {out}")


def write_network_schedule() -> None:
    """Scheduled vs unrolled momentum PDHG: times and memory footprint (KB)."""
    header = (SCHEDULE / "scheduled" / "emitted" / "include" / "fom_data.h").read_text()
    sched_len = int(re.search(r"#define FOM_SCHED_XI_LEN (\d+)", header).group(1))
    rows = []
    for arm, label in (("scheduled", "scheduled"), ("unrolled", "unrolled")):
        t = json.loads((SCHEDULE / arm / "timing.json").read_text())
        ir, lib = t["size"]["ir"], t["size"]["optimized"]["library_bytes"]
        # Every table entry and coefficient is a double.
        sched_kb = f"{sched_len * 8 / 1000:.1f}" if ir["schedules"] else "--"
        rows.append(
            dict(
                method=label,
                steps=ir["body_steps"],
                blocks=ir["coefficient_blocks"],
                numpyus=f"{t['numpy_us']:.0f}",
                translatedus=f"{t['translated_us']:.1f}",
                optimizedus=f"{t['optimized_us']:.1f}",
                speedtotal=f"{t['speedup_total']:.1f}",
                schedkb=sched_kb,
                coefkb=f"{ir['coefficient_nonzeros'] * 8 / 1000:.0f}",
                libkb=f"{lib / 1000:.0f}",
            )
        )
        dev = max(t["max_abs_dev"].values())
        print(f"network {arm}: {rows[-1]}, max|C - NumPy| {dev:.1e}, kept {t['optimization_kept']}")
    out = PAPER_DATA / "codegen_network_schedule.csv"
    with open(out, "w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=list(rows[0]))
        writer.writeheader()
        writer.writerows(rows)
    print(f"wrote {out}")


WRITERS = {"table": write_table, "portfolio": write_portfolio_scaling, "network": write_network_schedule}


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--only", nargs="*", default=list(WRITERS), choices=list(WRITERS))
    for name in parser.parse_args().only:
        WRITERS[name]()


if __name__ == "__main__":
    main()
