"""CSV and JSON records for a run.

``CsvLog`` rewrites its whole file after every row, through a temporary file and
an atomic rename. A K loop writes at most a few hundred rows, so this costs
nothing, and it means a job killed at any moment leaves a complete CSV of every
finished K, whose header is the union of the keys seen so far: a column that
first appears at K = 7 (a warm-start statistic, say) is not lost.
"""

from __future__ import annotations

import csv
import json
import os
import platform
import socket
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path

from fomexp.paths import REPO

SLURM_VARS = (
    "SLURM_JOB_ID",
    "SLURM_ARRAY_JOB_ID",
    "SLURM_ARRAY_TASK_ID",
    "SLURM_JOB_NAME",
    "SLURM_JOB_PARTITION",
    "SLURM_JOB_NODELIST",
    "SLURM_CPUS_PER_TASK",
    "SLURM_CPUS_ON_NODE",
    "SLURM_MEM_PER_NODE",
    "SLURM_MEM_PER_CPU",
)


def utc_now() -> str:
    return datetime.now(timezone.utc).isoformat(timespec="seconds")


class CsvLog:
    def __init__(self, path: Path):
        self.path = Path(path)
        self.rows: list[dict] = []
        self.fields: list[str] = []

    def append(self, row: dict) -> None:
        self.rows.append(row)
        for key in row:
            if key not in self.fields:
                self.fields.append(key)
        self.flush()

    def extend(self, rows: list[dict]) -> None:
        for row in rows:
            self.rows.append(row)
            for key in row:
                if key not in self.fields:
                    self.fields.append(key)
        self.flush()

    def flush(self) -> None:
        self.path.parent.mkdir(parents=True, exist_ok=True)
        tmp = self.path.with_suffix(self.path.suffix + ".tmp")
        with open(tmp, "w", newline="") as f:
            writer = csv.DictWriter(f, fieldnames=self.fields, restval="")
            writer.writeheader()
            writer.writerows(self.rows)
        os.replace(tmp, self.path)


def write_json(path: Path, data: dict) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    tmp = path.with_suffix(path.suffix + ".tmp")
    tmp.write_text(json.dumps(data, indent=2, default=_jsonable) + "\n")
    os.replace(tmp, path)


def _jsonable(value):
    try:
        import numpy as np

        if isinstance(value, np.generic):
            return value.item()
        if isinstance(value, np.ndarray):
            return value.tolist()
    except ImportError:
        pass
    return str(value)


def git_state(path: Path) -> dict:
    """Commit hash and dirty flag of the repository at ``path``; empty if there is none.

    On the cluster the trees arrive by rsync without ``.git``; ``code_state``
    then falls back to ``.code_state.json``, which ``sync.sh`` writes on the
    laptop just before every push.
    """

    def git(*args: str) -> str:
        return subprocess.run(
            ["git", "-C", str(path), *args], capture_output=True, text=True, check=True
        ).stdout.strip()

    try:
        return {"commit": git("rev-parse", "HEAD"), "dirty": bool(git("status", "--porcelain"))}
    except (subprocess.CalledProcessError, FileNotFoundError):
        return {}


def environment() -> dict:
    """Host, interpreter, package versions and Slurm variables of this process."""
    info = {
        "host": socket.gethostname(),
        "platform": platform.platform(),
        "python": sys.version.split()[0],
        "cpu_count": os.cpu_count(),
        "slurm": {k: os.environ[k] for k in SLURM_VARS if k in os.environ},
        "grb_license_file": os.environ.get("GRB_LICENSE_FILE", ""),
    }
    for name in ("numpy", "scipy", "gurobipy", "fomtools", "hydra", "cvxpy", "clarabel"):
        try:
            module = __import__(name)
            info[f"{name}_version"] = getattr(module, "__version__", "")
        except ImportError:
            info[f"{name}_version"] = None
    try:
        import gurobipy

        info["gurobi_version"] = ".".join(map(str, gurobipy.gurobi.version()))
    except ImportError:
        pass
    return info


def code_state() -> dict:
    state = {
        "fomtools_experiments": git_state(REPO),
        "fomtools": git_state(REPO.parent / "fomtools"),
    }
    if all(state.values()):
        return state
    synced = REPO / ".code_state.json"
    if synced.exists():
        return {"source": "sync.sh", **json.loads(synced.read_text())}
    return state
