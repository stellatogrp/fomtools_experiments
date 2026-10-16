"""Summarize an interleaved benchmark log.

Usage: python summarize.py LOG [--rounds 7] [--out-dir DIR]

Each round contributes one value per series:
  numpy   FOMTools reference execution (NumPy), median of its repetitions   (NUMPY_<ALG> ... median_us=)
  scipy   hand-written SciPy with D and D^T, median of its repetitions      (SCIPY_<ALG> ... median_us=)
  openmp       generated OpenMP solver, mean of its timed solves            (BENCH ... after CPU_OPENMP_<ALG>)
  openmp_orig  original-form generated OpenMP solver in a variant run       (BENCH ... after CPU_OPENMP_ORIG_<ALG>)
  cupy         hand-written CuPy/cuSPARSE with D and D^T, mean of solves    (BENCH ... after CUPY_<ALG>)
  cuda         generated CUDA solver, mean of its timed solves              (BENCH ... after CUDA_<ALG>)
  cuda_orig    original-form generated CUDA solver in a variant run         (BENCH ... after CUDA_ORIG_<ALG>)
A series may be absent for a method (shown as "-"); a series that is present for
a method must have one value per round. $SERIES_VARIANT (e.g. "sepA") labels the
generated solvers of a variant run. Reported times are medians of the round values; speedups are
ratios of those medians. Writes summary.csv and summary.md.
"""

from __future__ import annotations

import argparse
import csv
import os
import re
import statistics
from collections import defaultdict
from pathlib import Path

ALGORITHMS = tuple(a.upper() for a in os.environ.get("ALGORITHMS", "ista fista").split())
SERIES = ("numpy", "scipy", "openmp", "openmp_orig", "cupy", "cuda", "cuda_orig")
_VARIANT = os.environ.get("SERIES_VARIANT", "")
LABEL = {
    "numpy": "FOMTools reference (NumPy)",
    "scipy": "Hand-written SciPy",
    "openmp": f"Generated OpenMP ({_VARIANT})" if _VARIANT else "Generated OpenMP",
    "openmp_orig": "Generated OpenMP (original form)",
    "cupy": "Hand-written CuPy",
    "cuda": f"Generated CUDA ({_VARIANT})" if _VARIANT else "Generated CUDA",
    "cuda_orig": "Generated CUDA (original form)",
}
# (name, numerator series, denominator series): numerator time / denominator time.
SPEEDUPS = (
    ("openmp_speedup", "numpy", "openmp"),
    ("cuda_speedup", "numpy", "cuda"),
    ("cuda_vs_openmp", "openmp", "cuda"),
    ("openmp_vs_scipy", "scipy", "openmp"),
    ("cuda_vs_scipy", "scipy", "cuda"),
    ("cuda_vs_cupy", "cupy", "cuda"),
    ("scipy_vs_numpy", "numpy", "scipy"),
    ("openmp_vs_openmp_orig", "openmp_orig", "openmp"),
    ("cuda_vs_cuda_orig", "cuda_orig", "cuda"),
    ("openmp_orig_vs_scipy", "scipy", "openmp_orig"),
    ("cuda_orig_vs_cupy", "cupy", "cuda_orig"),
)
MEDIAN_LINE = re.compile(r"(NUMPY|SCIPY)_(ISTA|FISTA) .*median_us=([0-9.]+)")
MARKER = re.compile(r"(CPU_OPENMP_ORIG|CPU_OPENMP|CUPY|CUDA_ORIG|CUDA)_(ISTA|FISTA)")
MEAN = re.compile(r"^BENCH .*mean_us=([0-9.]+)")
MARKER_SERIES = {"CPU_OPENMP": "openmp", "CPU_OPENMP_ORIG": "openmp_orig", "CUPY": "cupy",
                 "CUDA": "cuda", "CUDA_ORIG": "cuda_orig"}


def parse(path: Path) -> tuple[dict[tuple[str, str], list[float]], int]:
    values: dict[tuple[str, str], list[float]] = defaultdict(list)
    pending = None
    rounds = 0
    for raw in path.read_text().splitlines():
        line = raw.strip()
        if line.startswith("ROUND "):
            rounds += 1
            continue
        match = MEDIAN_LINE.match(line)
        if match:
            values[(match.group(2), match.group(1).lower())].append(float(match.group(3)))
            continue
        match = MARKER.fullmatch(line)
        if match:
            pending = (match.group(2), MARKER_SERIES[match.group(1)])
            continue
        match = MEAN.match(line)
        if match and pending:
            values[pending].append(float(match.group(1)))
            pending = None
    return values, rounds


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("log", type=Path)
    parser.add_argument("--rounds", type=int, default=7)
    parser.add_argument("--out-dir", type=Path, default=None)
    args = parser.parse_args()
    out_dir = args.out_dir or args.log.parent

    values, rounds = parse(args.log)
    if rounds != args.rounds:
        raise SystemExit(f"expected {args.rounds} rounds, found {rounds}")
    present = [s for s in SERIES if any(values.get((a, s)) for a in ALGORITHMS)]
    for algorithm in ALGORITHMS:
        for series in present:
            count = len(values.get((algorithm, series), []))
            if count not in (0, rounds):  # absent for this method, or one value per round
                raise SystemExit(f"{algorithm}/{series}: {count} values for {rounds} rounds")
    speedups = [(n, a, b) for n, a, b in SPEEDUPS if a in present and b in present]

    rows = []
    for algorithm in ALGORITHMS:
        med = {s: statistics.median(values[(algorithm, s)]) for s in present if values.get((algorithm, s))}
        row = {"algorithm": algorithm}
        for s in present:
            row[f"{s}_median_us"] = f"{med[s]:.3f}" if s in med else ""
        for name, num, den in speedups:
            row[name] = f"{med[num] / med[den]:.2f}" if num in med and den in med else ""
        for s in present:
            row[f"{s}_min_max_us"] = "{:.0f}-{:.0f}".format(
                min(values[(algorithm, s)]), max(values[(algorithm, s)])
            ) if s in med else ""
        rows.append(row)

    out_dir.mkdir(parents=True, exist_ok=True)
    with (out_dir / "summary.csv").open("w", newline="") as handle:
        writer = csv.DictWriter(handle, fieldnames=list(rows[0]), lineterminator="\n")
        writer.writeheader()
        writer.writerows(rows)

    speedup_cols = speedups
    header = ["method"] + [f"{LABEL[s]} (s)" for s in present] + [
        f"{LABEL[b]} vs {LABEL[a]}" for _, a, b in speedup_cols
    ]
    lines = [
        f"Median of {rounds} interleaved round values; \"A vs B\" = how many times faster A is than B.",
        "",
        "| " + " | ".join(header) + " |",
        "|---|" + "---:|" * (len(header) - 1),
    ]
    for row in rows:
        cells = [row["algorithm"]] + [
            f"{float(row[f'{s}_median_us']) / 1e6:.4g}" if row[f"{s}_median_us"] else "-" for s in present
        ]
        cells += [f"{row[n]}x" if row[n] else "-" for n, _, _ in speedup_cols]
        lines.append("| " + " | ".join(cells) + " |")
    (out_dir / "summary.md").write_text("\n".join(lines) + "\n")
    print("\n".join(lines))


if __name__ == "__main__":
    main()
