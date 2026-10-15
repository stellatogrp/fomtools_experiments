"""Summarize an interleaved NumPy / OpenMP / CUDA benchmark log.

Usage: python summarize.py LOG [--rounds 7] [--out-dir DIR]

Each round contributes one value per series: the NumPy median of its
repetitions (``NUMPY_<ALG> ... median_us=``) and the generated-code mean of
its timed solves (``BENCH ... mean_us=`` after ``CPU_OPENMP_<ALG>`` /
``CUDA_<ALG>``). Reported times are medians of the round values; speedups
are NumPy median / backend median. Writes summary.csv and summary.md.
"""

from __future__ import annotations

import argparse
import csv
import re
import statistics
from collections import defaultdict
from pathlib import Path

ALGORITHMS = ("ISTA", "FISTA")
SERIES = ("numpy", "openmp", "cuda")
NUMPY = re.compile(r"NUMPY_(ISTA|FISTA) .*median_us=([0-9.]+)")
MARKER = re.compile(r"(CPU_OPENMP|CUDA)_(ISTA|FISTA)")
MEAN = re.compile(r"^BENCH .*mean_us=([0-9.]+)")


def parse(path: Path) -> dict[tuple[str, str], list[float]]:
    values: dict[tuple[str, str], list[float]] = defaultdict(list)
    pending = None
    rounds = 0
    for raw in path.read_text().splitlines():
        line = raw.strip()
        if line.startswith("ROUND "):
            rounds += 1
            continue
        match = NUMPY.match(line)
        if match:
            values[(match.group(1), "numpy")].append(float(match.group(2)))
            continue
        match = MARKER.fullmatch(line)
        if match:
            series = "openmp" if match.group(1) == "CPU_OPENMP" else "cuda"
            pending = (match.group(2), series)
            continue
        match = MEAN.match(line)
        if match and pending:
            values[pending].append(float(match.group(1)))
            pending = None
    values[("_rounds", "_")] = [float(rounds)]
    return values


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("log", type=Path)
    parser.add_argument("--rounds", type=int, default=7)
    parser.add_argument("--out-dir", type=Path, default=None)
    args = parser.parse_args()
    out_dir = args.out_dir or args.log.parent

    values = parse(args.log)
    rounds = int(values.pop(("_rounds", "_"))[0])
    if rounds != args.rounds:
        raise SystemExit(f"expected {args.rounds} rounds, found {rounds}")
    for algorithm in ALGORITHMS:
        for series in SERIES:
            count = len(values[(algorithm, series)])
            if count != rounds:
                raise SystemExit(f"{algorithm}/{series}: {count} values for {rounds} rounds")

    rows = []
    for algorithm in ALGORITHMS:
        med = {s: statistics.median(values[(algorithm, s)]) for s in SERIES}
        rows.append(
            {
                "algorithm": algorithm,
                "numpy_median_us": f"{med['numpy']:.3f}",
                "openmp_median_us": f"{med['openmp']:.3f}",
                "cuda_median_us": f"{med['cuda']:.3f}",
                "openmp_speedup": f"{med['numpy'] / med['openmp']:.2f}",
                "cuda_speedup": f"{med['numpy'] / med['cuda']:.2f}",
                "cuda_vs_openmp": f"{med['openmp'] / med['cuda']:.2f}",
                "numpy_min_max_us": "{:.0f}-{:.0f}".format(
                    min(values[(algorithm, "numpy")]), max(values[(algorithm, "numpy")])
                ),
                "openmp_min_max_us": "{:.0f}-{:.0f}".format(
                    min(values[(algorithm, "openmp")]), max(values[(algorithm, "openmp")])
                ),
                "cuda_min_max_us": "{:.0f}-{:.0f}".format(
                    min(values[(algorithm, "cuda")]), max(values[(algorithm, "cuda")])
                ),
            }
        )

    out_dir.mkdir(parents=True, exist_ok=True)
    with (out_dir / "summary.csv").open("w", newline="") as handle:
        writer = csv.DictWriter(handle, fieldnames=list(rows[0]), lineterminator="\n")
        writer.writeheader()
        writer.writerows(rows)

    lines = [
        f"Median of {rounds} interleaved round values; speedup = NumPy / backend.",
        "",
        "| method | NumPy (s) | OpenMP (s) | CUDA (s) | OpenMP speedup | CUDA speedup |",
        "|---|---:|---:|---:|---:|---:|",
    ]
    for row in rows:
        lines.append(
            "| {algorithm} | {numpy:.3f} | {openmp:.3f} | {cuda:.4f} | {o}x | {c}x |".format(
                algorithm=row["algorithm"],
                numpy=float(row["numpy_median_us"]) / 1e6,
                openmp=float(row["openmp_median_us"]) / 1e6,
                cuda=float(row["cuda_median_us"]) / 1e6,
                o=row["openmp_speedup"],
                c=row["cuda_speedup"],
            )
        )
    (out_dir / "summary.md").write_text("\n".join(lines) + "\n")
    print("\n".join(lines))


if __name__ == "__main__":
    main()
