"""Gurobi parameters from the config, and every number a finished solve exposes."""

from __future__ import annotations

import math
import re
from pathlib import Path

#: Model attributes recorded after every solve, as ``column: attribute``.
ATTRIBUTES = {
    "bound": "ObjBound",
    "incumbent": "ObjVal",
    "gap": "MIPGap",
    "status_code": "Status",
    "sol_count": "SolCount",
    "nodes": "NodeCount",
    "open_nodes": "OpenNodeCount",
    "simplex_iters": "IterCount",
    "barrier_iters": "BarIterCount",
    "work": "Work",
    "gurobi_runtime": "Runtime",
    "max_mem_used_gb": "MaxMemUsed",
    "num_vars": "NumVars",
    "num_bin_vars": "NumBinVars",
    "num_int_vars": "NumIntVars",
    "num_constrs": "NumConstrs",
    "num_sos": "NumSOS",
    "num_qconstrs": "NumQConstrs",
    "num_genconstrs": "NumGenConstrs",
    "num_nzs": "NumNZs",
    "max_coeff": "MaxCoeff",
    "min_coeff": "MinCoeff",
    "max_bound": "MaxBound",
    "min_bound": "MinBound",
    "max_rhs": "MaxRHS",
    "min_rhs": "MinRHS",
    "max_obj_coeff": "MaxObjCoeff",
    "min_obj_coeff": "MinObjCoeff",
    "fingerprint": "Fingerprint",
}

STATUS_NAMES = {
    1: "LOADED",
    2: "OPTIMAL",
    3: "INFEASIBLE",
    4: "INF_OR_UNBD",
    5: "UNBOUNDED",
    6: "CUTOFF",
    7: "ITERATION_LIMIT",
    8: "NODE_LIMIT",
    9: "TIME_LIMIT",
    10: "SOLUTION_LIMIT",
    11: "INTERRUPTED",
    12: "NUMERIC",
    13: "SUBOPTIMAL",
    14: "INPROGRESS",
    15: "USER_OBJ_LIMIT",
    16: "WORK_LIMIT",
    17: "MEM_LIMIT",
}


def construction_params(solver_cfg) -> dict:
    """Parameters fixed for the whole run, given to ``Verifier(params=...)``.

    No ``OutputFlag``: fomtools solvers, and the OBBT LPs, are silent by default.
    The main solve turns logging on per K, into its own file (``solve_params``).
    """
    params = {
        "MIPGap": float(solver_cfg.mip_gap),
        "MIPFocus": int(solver_cfg.mip_focus),
        "Threads": int(solver_cfg.threads),
        "Seed": int(solver_cfg.seed),
    }
    params.update(dict(solver_cfg.get("extra_params") or {}))
    return params


def obbt_params(solver_cfg) -> dict:
    return {
        "Threads": int(solver_cfg.threads),
        **dict(solver_cfg.get("obbt_params") or {}),
    }


def solve_params(solver_cfg, time_limit: float, log_file: Path | None) -> dict:
    """Per-K parameters for ``vp.solve(**...)``; they apply to the main MIP only."""
    params: dict = {"TimeLimit": float(time_limit)}
    if log_file is not None:
        # LogToConsole first, so turning OutputFlag on never prints to the job log.
        params.update(LogToConsole=0, LogFile=str(log_file), OutputFlag=1)
    return params


def _get(model, attribute: str):
    try:
        value = model[attribute]
    except Exception:  # noqa: BLE001 - an attribute without a value is a NaN, not a crash
        return math.nan
    return value


def solve_record(solved) -> dict:
    """Every attribute in ``ATTRIBUTES`` of the solved model, NaN where undefined."""
    row = {column: _get(solved, attribute) for column, attribute in ATTRIBUTES.items()}
    code = row["status_code"]
    row["status"] = STATUS_NAMES.get(int(code), str(code)) if not _isnan(code) else ""
    row["time_limit_hit"] = row["status"] == "TIME_LIMIT"
    row["bound_valid"] = row["status"] not in (
        "INFEASIBLE",
        "INF_OR_UNBD",
        "UNBOUNDED",
        "NUMERIC",
        "",
    )
    if not _isnan(row["bound"]) and not _isnan(row["incumbent"]):
        row["abs_gap"] = row["bound"] - row["incumbent"]
    else:
        row["abs_gap"] = math.nan
    for name in ("MIPGap", "TimeLimit", "MIPFocus", "Threads", "Seed"):
        try:
            row[f"param_{name}"] = solved.model.getParamInfo(name)[2]
        except Exception:  # noqa: BLE001
            row[f"param_{name}"] = math.nan
    return row


def _isnan(value) -> bool:
    return isinstance(value, float) and math.isnan(value)


_ROOT = re.compile(r"^Root relaxation: objective\s+([-+0-9.eE]+)")
_ROOT_TIME = re.compile(r"^Root relaxation: .*,\s*([0-9.]+) seconds")
_NODE_LINE = re.compile(r"^[H* ]\s*\d+\s+\d+\s")


def parse_log(path: Path) -> dict:
    """What the Gurobi log has and the model attributes do not.

    The root relaxation bound and time, the presolved model size, the first
    incumbent and when it was found, the cut counts, and the number of
    heuristic solutions. Fields the log lacks are left out.
    """
    out: dict = {}
    if not path.exists():
        return out
    lines = path.read_text(errors="replace").splitlines()
    cuts: dict[str, int] = {}
    in_cuts = False
    first_incumbent_time = None
    for line in lines:
        if m := _ROOT.match(line):
            out["root_bound"] = float(m.group(1))
            if t := _ROOT_TIME.match(line):
                out["root_time"] = float(t.group(1))
        elif line.startswith("Presolved:"):
            nums = re.findall(r"(\d+) (rows|columns|nonzeros)", line)
            for value, kind in nums:
                out[f"presolved_{kind}"] = int(value)
        elif m := re.match(
            r"^Variable types: (\d+) continuous, (\d+) integer \((\d+) binary\)", line
        ):
            out["presolved_continuous"] = int(m.group(1))
            out["presolved_integer"] = int(m.group(2))
            out["presolved_binary"] = int(m.group(3))
        elif m := re.match(r"^Found heuristic solution: objective\s+([-+0-9.eE]+)", line):
            out["heuristic_solutions"] = out.get("heuristic_solutions", 0) + 1
            out.setdefault("first_heuristic_objective", float(m.group(1)))
        elif m := re.match(r"^Loaded user MIP start (\d+) with objective\s+([-+0-9.eE]+)", line):
            out[f"user_start{m.group(1)}_objective"] = float(m.group(2))
        elif m := re.match(r"^User MIP start (\d+) did not produce a new incumbent", line):
            out[f"user_start{m.group(1)}_no_improvement"] = True
        elif m := re.match(r"^MIP start from previous solve .*objective\s+([-+0-9.eE]+)", line):
            out["previous_start_objective"] = float(m.group(1))
        elif line.startswith("Cutting planes:"):
            in_cuts = True
        elif in_cuts:
            if m := re.match(r"^\s+([A-Za-z\- ]+?):\s+(\d+)$", line):
                cuts[m.group(1).strip()] = int(m.group(2))
            else:
                in_cuts = False
        if first_incumbent_time is None and _NODE_LINE.match(line) and line[:1] in ("H", "*"):
            if t := re.search(r"(\d+)s\s*$", line):
                first_incumbent_time = float(t.group(1))
    for name, count in cuts.items():
        out["cuts_" + re.sub(r"\W+", "_", name.lower()).strip("_")] = count
    if first_incumbent_time is not None:
        out["t_first_tree_incumbent"] = first_incumbent_time
    return out
