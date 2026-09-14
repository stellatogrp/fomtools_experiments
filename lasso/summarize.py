"""Summarize an interleaved benchmark log as a CSV table."""

from __future__ import annotations

import argparse
import csv
import re
import statistics
from collections import defaultdict
from pathlib import Path

ROOT = Path(__file__).parent
MEAN = re.compile(r"mean_us=([0-9.]+)")
NUMPY = re.compile(r"NUMPY_(ISTA|FISTA) median_us=([0-9.]+)")


def parse(path: Path) -> dict[str, list[float]]:
    values: dict[str, list[float]] = defaultdict(list)
    pending = None
    for raw in path.read_text().splitlines():
        line = raw.strip()
        numpy_match = NUMPY.fullmatch(line)
        if numpy_match:
            values[f"numpy_{numpy_match.group(1).lower()}"].append(float(numpy_match.group(2)))
            continue
        if line in {"CPU_ISTA", "CUDA_ISTA", "CPU_FISTA", "CUDA_FISTA"}:
            pending = line.lower()
            continue
        mean_match = MEAN.search(line)
        if pending and mean_match:
            values[pending].append(float(mean_match.group(1)))
            pending = None
    return values


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "input",
        nargs="?",
        type=Path,
        default=ROOT / "results" / "paper_40x80" / "paper_final_compute_7rounds.txt",
    )
    parser.add_argument(
        "--output",
        type=Path,
        default=ROOT / "out" / "summary.csv",
    )
    args = parser.parse_args()
    values = parse(args.input)
    expected = {
        "numpy_ista",
        "cpu_ista",
        "cuda_ista",
        "numpy_fista",
        "cpu_fista",
        "cuda_fista",
    }
    if set(values) != expected or len({len(v) for v in values.values()}) != 1:
        raise ValueError(f"incomplete benchmark log: {dict(values)}")

    rows = []
    for algorithm in ("ista", "fista"):
        numpy_us = statistics.median(values[f"numpy_{algorithm}"])
        cpu_us = statistics.median(values[f"cpu_{algorithm}"])
        cuda_us = statistics.median(values[f"cuda_{algorithm}"])
        rows.append(
            {
                "algorithm": algorithm.upper(),
                "numpy_median_us": f"{numpy_us:.6f}",
                "cpu_median_us": f"{cpu_us:.6f}",
                "cuda_median_us": f"{cuda_us:.6f}",
                "numpy_to_cpu": f"{numpy_us / cpu_us:.3f}",
                "numpy_to_cuda": f"{numpy_us / cuda_us:.3f}",
            }
        )
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with args.output.open("w", newline="") as output:
        writer = csv.DictWriter(output, fieldnames=list(rows[0]), lineterminator="\n")
        writer.writeheader()
        writer.writerows(rows)
    print(args.output)


if __name__ == "__main__":
    main()
