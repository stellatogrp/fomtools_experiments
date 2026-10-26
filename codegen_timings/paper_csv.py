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
  (``portfolio_cvxpygen.plot``);
- ``codegen_lasso_rect.csv`` and ``codegen_lasso_agent_time.csv`` (``gpu``): the
  large-scale Lasso benchmark on della and the agent time per solver, from
  ``lasso/lasso_codegen/results/<GPU_RUN>/``;
- ``../figures/control_figure8_flight.pdf`` (``flight``): the closed-loop flight of
  the repetition whose optimized solver runs the median number of iterations in
  the figure's budget, from ``control/rapdhg/codegen/results/``.
"""

from __future__ import annotations

import argparse
import csv
import json
import re
import shutil
import statistics
from datetime import datetime
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
GPU_RUN = "v0.0.4"
GPU = REPO / "lasso" / "lasso_codegen" / "results" / GPU_RUN
FLIGHT = REPO / "control" / "rapdhg" / "codegen" / "results"

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
    "speedtotalmin",
    "speedtotalmax",
]


def _span(values) -> tuple[float, float, float]:
    return statistics.median(values), min(values), max(values)


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
                speedtotalmin=f"{t['speedup_total_min']:.0f}",
                speedtotalmax=f"{t['speedup_total_max']:.0f}",
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

    # Per repetition: speedups over OSQP, and the generation times.
    results = PORTFOLIO_WIDE.parent
    reps = [results, *sorted(results.glob("reps/rep*"))]
    for n in sorted({int(r["N"]) for r in rows}):
        speedups, translated, total, kept = [], [], [], []
        for d in reps:
            arms = {
                r["arm"]: float(r["us_per_solve"])
                for r in csv.DictReader(open(d / "timings_stop.csv"))
                if int(r["N"]) == n and r["level"] == "c"
            }
            speedups.append(arms["cpg_osqp"] / arms["cpg_fom_opt"])
            gen = json.loads((d / f"gen_n{n}.json").read_text())
            translated.append(gen["fom"]["wall_s"])
            total.append(gen["fom"]["wall_s"] + gen["fom_opt"]["wall_s"])
            kept.append(gen["fom_opt"].get("optimization_kept"))
        print(
            f"portfolio n={n}: OSQP/optimized per rep {[f'{x:.2f}' for x in speedups]}, "
            f"translation {min(translated):.0f}-{max(translated):.0f} s, translation + "
            f"code optimization {min(total) / 60:.1f}-{max(total) / 60:.1f} min, kept {kept}"
        )


def write_gpu(args) -> None:
    """The large-scale Lasso table (medians over the repetitions, with the range of the
    speedups) and the agent time per solver."""
    summary = list(csv.DictReader(open(GPU / "summary.csv")))

    def per_rep(algorithm: str, series: str) -> list[float]:
        return [
            float(r["median_us"])
            for r in summary
            if (r["algorithm"], r["series"]) == (algorithm, series) and r["rep"].isdigit()
        ]

    def baseline(algorithm: str, series: str) -> float:
        (us,) = [
            float(r["median_us"])
            for r in summary
            if (r["algorithm"], r["series"], r["rep"]) == (algorithm, series, "")
        ]
        return us

    rows = []
    for algorithm in ("ista", "fista"):
        scipy, cupy = baseline(algorithm, "scipy"), baseline(algorithm, "cupy")
        omp, cuda = per_rep(algorithm, "openmp"), per_rep(algorithm, "cuda")
        s_omp = [scipy / t for t in omp]
        s_cuda = [cupy / t for t in cuda]
        rows.append(
            dict(
                method=algorithm.upper(),
                scipys=f"{scipy / 1e6:.3f}",
                omps=f"{statistics.median(omp) / 1e6:.4f}",
                speedomp=f"{scipy / statistics.median(omp):.0f}",
                speedompmin=f"{min(s_omp):.0f}",
                speedompmax=f"{max(s_omp):.0f}",
                cupys=f"{cupy / 1e6:.4f}",
                cudas=f"{statistics.median(cuda) / 1e6:.4f}",
                speedcuda=f"{cupy / statistics.median(cuda):.1f}",
                speedcudamin=f"{min(s_cuda):.1f}",
                speedcudamax=f"{max(s_cuda):.1f}",
            )
        )
        print(f"gpu {algorithm}: {rows[-1]}")
    _out(args, "codegen_lasso_rect.csv", list(rows[0]), rows)

    agent, overhead = [], []
    for algorithm in ("ista", "fista"):
        for backend in ("openmp", "cuda"):
            records = [
                json.loads(p.read_text())
                for p in sorted(GPU.glob(f"rep*/{algorithm}_{backend}/record.json"))
            ]
            tr = [r["agent_wall_s"]["translation"] / 60 for r in records]
            opt = [r["agent_wall_s"]["code_optimization"] / 3600 for r in records]
            rounds = [len(r["optimization_rounds"]) for r in records]
            for r in records:
                wall = datetime.fromisoformat(r["finished_utc"]) - datetime.fromisoformat(
                    r["started_utc"]
                )
                overhead.append(wall.total_seconds() - sum(r["agent_wall_s"].values()))
            row = dict(method=algorithm.upper(), target="OpenMP" if backend == "openmp" else "CUDA")
            for key, values, fmt in (("tr", tr, "{:.1f}"), ("opt", opt, "{:.1f}")):
                med, lo, hi = _span(values)
                row.update(
                    {key: fmt.format(med), f"{key}min": fmt.format(lo), f"{key}max": fmt.format(hi)}
                )
            row.update(
                rounds=f"{statistics.median(rounds):.0f}",
                roundsmin=min(rounds),
                roundsmax=max(rounds),
            )
            agent.append(row)
            print(f"gpu agent {row}")
    print(f"gpu: time outside both phases {min(overhead):.0f}-{max(overhead):.0f} s per solver")
    _out(args, "codegen_lasso_agent_time.csv", list(agent[0]), agent)


def write_flight(args) -> None:
    """Copy the flight figure of the median repetition into the paper's figures/."""
    reps = [FLIGHT, *sorted(FLIGHT.glob("reps/rep*"))]
    figures = {d: json.loads((d / "manifest.json").read_text())["figure"] for d in reps}
    order = sorted(reps, key=lambda d: figures[d]["arms"]["optimized C"]["iterations"])
    median = order[len(order) // 2]
    for d in reps:
        arms = figures[d]["arms"]
        print(
            f"flight {d.relative_to(REPO)} at {figures[d]['budget_us']:.0f} us: "
            + ", ".join(
                f"{a} {v['iterations']} it at {v['us_per_iteration']:.3f} us"
                for a, v in arms.items()
            )
            + f"; translated/optimized per iteration "
            f"{arms['emitted C']['us_per_iteration'] / arms['optimized C']['us_per_iteration']:.1f}x"
            + ("  <- shown" if d == median else "")
        )
    path = Path(args.paper_dir).parent / "figures" / "control_figure8_flight.pdf"
    if not args.write:
        print(
            f"(dry run) would copy {median.relative_to(REPO)}/figures_figure8_flight.pdf to {path}"
        )
        return
    shutil.copyfile(median / "figures_figure8_flight.pdf", path)
    print(f"wrote {path}")


WRITERS = {
    "table": write_table,
    "network": write_network_schedule,
    "portfolio": write_portfolio_tolerance,
    "gpu": write_gpu,
    "flight": write_flight,
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
