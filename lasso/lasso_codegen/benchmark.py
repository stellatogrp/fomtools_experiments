"""Final benchmark: every generated solver against SciPy and CuPy, interleaved.

Run inside one GPU job (``della/bench.sbatch``), from ``lasso/``:

    python -m lasso_codegen.benchmark --run v0.0.4 [--rounds 7]

First every solver under ``lasso/out/codegen/rep*/`` that has a ``record.json``
is rebuilt from its verified source and re-verified on this node. Then each round
times, per algorithm, SciPy (median of 3 solves), every repetition's OpenMP solver,
CuPy (mean of 10) and every repetition's CUDA solver (``bench_bin 10``: the mean of
10 solves after its built-in warm-ups), as the paper's run did.

Writes ``results/<run>/`` next to this file: ``timings_raw.csv`` (one row per
round and series), ``summary.csv`` (the median over rounds per repetition, and the
median over repetitions), ``metadata.txt``, and each solver's ``record.json`` and
``OPTIMIZATIONS.md``.
"""

from __future__ import annotations

import argparse
import csv
import os
import re
import shutil
import socket
import statistics
import subprocess
import sys
from collections import defaultdict
from pathlib import Path

from lasso_codegen import baselines, problem
from lasso_codegen.generate import OUT

HERE = Path(__file__).parent
MEAN = re.compile(r"mean_us=([0-9.eE+-]+)")
MEDIAN = re.compile(r"median_us=([0-9.eE+-]+)")
GEN_SOLVES = 10


def solvers() -> list[tuple[int, str, str, Path]]:
    """``(rep, algorithm, backend, directory)`` of every completed generation."""
    found = []
    for record in sorted(OUT.glob("rep*/*/record.json")):
        algorithm, backend = record.parent.name.split("_")
        found.append((int(record.parent.parent.name[3:]), algorithm, backend, record.parent))
    return found


def sh(cmd: list[str], cwd: Path | None = None) -> str:
    proc = subprocess.run(cmd, cwd=cwd, capture_output=True, text=True)
    if proc.returncode:
        raise SystemExit(f"{' '.join(cmd)} failed in {cwd}:\n{proc.stdout}\n{proc.stderr}")
    return proc.stdout + proc.stderr


def prep(found, out: Path) -> None:
    """Rebuild and re-verify every solver here; leave ``bench_bin`` built."""
    for rep, algorithm, backend, d in found:
        for binary in ("verify_bin", "bench_bin"):
            (d / binary).unlink(missing_ok=True)
        log = sh(["make", "verify"], cwd=d)
        label = f"rep{rep}_{algorithm}_{backend}"
        (out / f"verify_{label}.log").write_text(log)
        if "VERIFY OK" not in log:
            raise SystemExit(f"{label} does not verify on this node")
        sh(["make", "compile" if backend == "cuda" else "bench"], cwd=d)  # builds bench_bin


def generated_us(d: Path) -> float:
    return float(MEAN.search(sh([str(d / "bench_bin"), str(GEN_SOLVES)], cwd=d)).group(1))


def run_round(found, r: int) -> list[dict]:
    rows = []
    for algorithm in ("ista", "fista"):
        line = baselines.bench("scipy", algorithm)
        rows.append(
            dict(
                round=r,
                algorithm=algorithm,
                series="scipy",
                rep="",
                us=float(MEDIAN.search(line).group(1)),
            )
        )
        for backend, baseline in (("openmp", None), ("cuda", "cupy")):
            if baseline:
                line = baselines.bench(baseline, algorithm)
                rows.append(
                    dict(
                        round=r,
                        algorithm=algorithm,
                        series=baseline,
                        rep="",
                        us=float(MEAN.search(line).group(1)),
                    )
                )
            for rep, alg, be, d in found:
                if (alg, be) == (algorithm, backend):
                    rows.append(
                        dict(
                            round=r,
                            algorithm=algorithm,
                            series=backend,
                            rep=rep,
                            us=generated_us(d),
                        )
                    )
        print(f"round {r} {algorithm} done", flush=True)
    return rows


def summarize(rows: list[dict]) -> list[dict]:
    per = defaultdict(list)
    for row in rows:
        per[(row["algorithm"], row["series"], row["rep"])].append(row["us"])
    summary = [
        dict(
            algorithm=a,
            series=s,
            rep=rep,
            median_us=statistics.median(v),
            min_us=min(v),
            max_us=max(v),
            rounds=len(v),
        )
        for (a, s, rep), v in sorted(per.items(), key=lambda kv: str(kv[0]))
    ]
    # The median over repetitions of each generated series, and the paper's speedups on it.
    over_reps = {}
    for a in ("ista", "fista"):
        for s in ("scipy", "cupy", "openmp", "cuda"):
            meds = [r["median_us"] for r in summary if (r["algorithm"], r["series"]) == (a, s)]
            if meds:
                over_reps[(a, s)] = statistics.median(meds)
                summary.append(dict(algorithm=a, series=s, rep="median", median_us=over_reps[(a, s)],
                                    min_us=min(meds), max_us=max(meds), rounds=len(meds)))  # fmt: skip
        for name, num, den in (
            ("openmp_vs_scipy", "scipy", "openmp"),
            ("cuda_vs_cupy", "cupy", "cuda"),
        ):
            if (a, num) in over_reps and (a, den) in over_reps:
                summary.append(dict(algorithm=a, series=name, rep="median",
                                    median_us=over_reps[(a, num)] / over_reps[(a, den)]))  # fmt: skip
    return summary


def write_csv(path: Path, rows: list[dict]) -> None:
    fields = list(dict.fromkeys(k for row in rows for k in row))
    with open(path, "w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=fields, restval="")
        writer.writeheader()
        writer.writerows(rows)


def metadata(found) -> str:
    def run(*cmd):
        try:
            return subprocess.run(cmd, capture_output=True, text=True).stdout.strip()
        except OSError:
            return ""

    sys.path.append(str(HERE.parents[1]))
    from fomexp.records import code_state

    m, n, k = problem.size()
    lines = [
        f"host={socket.gethostname()} slurm_job={os.environ.get('SLURM_JOB_ID', '')}",
        run("nvidia-smi", "-L"),
        "\n".join(
            ln
            for ln in run("lscpu").splitlines()
            if ln.startswith(("Model name", "Socket", "NUMA node("))
        ),
        f"shape={m}x{n} nnz_per_row={k} iterations={problem.ITERATIONS} dtype=float64",
        f"OMP_NUM_THREADS={os.environ.get('OMP_NUM_THREADS')} OMP_PLACES={os.environ.get('OMP_PLACES')} "
        f"OMP_PROC_BIND={os.environ.get('OMP_PROC_BIND')} OMPFLAGS={os.environ.get('OMPFLAGS')} "
        f"NVCCFLAGS={os.environ.get('NVCCFLAGS')}",
        run("gcc", "--version").splitlines()[0] if run("gcc", "--version") else "",
        run("nvcc", "--version").splitlines()[-1] if run("nvcc", "--version") else "",
        f"code={code_state()}",
        "solvers=" + " ".join(f"rep{rep}/{a}_{b}" for rep, a, b, _ in found),
    ]
    return "\n".join(lines) + "\n"


def require_a100_job() -> None:
    """Refuse to time anything outside a Slurm batch job on an A100 node."""
    if not os.environ.get("SLURM_JOB_ID"):
        raise SystemExit("benchmark: not inside a Slurm job; submit della/bench.sbatch")
    gpus = subprocess.run(
        ["nvidia-smi", "--query-gpu=name", "--format=csv,noheader"], capture_output=True, text=True
    ).stdout
    if "A100" not in gpus:
        raise SystemExit(f"benchmark: no A100 visible on {socket.gethostname()}; refusing")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--run", required=True, help="results/<run>/ to write")
    parser.add_argument("--rounds", type=int, default=7)
    args = parser.parse_args()
    require_a100_job()

    found = solvers()
    if not found:
        raise SystemExit(f"no generated solvers under {OUT}")
    out = HERE / "results" / args.run
    out.mkdir(parents=True, exist_ok=True)
    (out / "metadata.txt").write_text(metadata(found))
    prep(found, out)
    for rep, algorithm, backend, d in found:
        dest = out / f"rep{rep}" / f"{algorithm}_{backend}"
        dest.mkdir(parents=True, exist_ok=True)
        for name in ("record.json", "OPTIMIZATIONS.md"):
            if (d / name).exists():
                shutil.copy2(d / name, dest / name)

    rows = []
    for r in range(1, args.rounds + 1):
        rows += run_round(found, r)
        write_csv(out / "timings_raw.csv", rows)  # a killed job keeps the finished rounds
    summary = summarize(rows)
    write_csv(out / "summary.csv", summary)
    for row in summary:
        if row["rep"] == "median":
            print(row)


if __name__ == "__main__":
    main()
