"""Scaffold the OpenMP and CUDA targets of one algorithm from a single IR.

Usage: python scaffold_pair.py {ista,fista}

The IR is extracted once. The OpenMP scaffold runs the reference oracle for
the eight verification samples; the CUDA scaffold reuses that verify_data.h
byte for byte, so both backends are checked against identical references.
Each generated Makefile is then replaced by an object-cached build: the
multi-GB deterministic sources compile once, only the solver recompiles.
"""

from __future__ import annotations

import json
import os
import sys
import time
from pathlib import Path

from fomtools.codegen.cli import _extract_fom, _load_module_fom
from fomtools.codegen.config import Backend, CodegenConfig, TerminationSpec, Version
from fomtools.codegen.emit import scaffold
from fomtools.codegen.targets.large_scale_cuda import CudaTarget
from fomtools.codegen.targets.large_scale_openmp import OpenMpTarget

ITERATIONS = int(os.environ.get("ITERATIONS", "200"))

# x86-64 small-model code can address only ~2 GB of static data; the FISTA
# matrices plus verification samples approach that, so all host objects use
# the medium model (large arrays go to .ldata/.lrodata). CUDA builds native
# sm_80 code only: embedding PTX would force a driver JIT of the multi-GB
# __device__ matrix data on every process start.
OPENMP_MAKEFILE = """\
# Object-cached OpenMP build (fomtools_full_pipeline). The deterministic
# sources (multi-GB matrix data) compile once; only the solver recompiles.
CC = cc
CFLAGS = -O3 -mcmodel=medium -fopenmp -Wall -Iinclude -Itest
SOLVER_SRC ?= src/fom_solver.c
BENCH_SOLVES ?= 3
HDRS = $(wildcard include/*.h)
FIXED = fom_ops.o fom_data.o fom_linsolve.o

.PHONY: verify bench profile fixed clean-solver

fixed: $(FIXED) verify_main.o bench_main.o prof_main.o prof_runtime.o

fom_ops.o: src/fom_ops.c $(HDRS)
\t$(CC) $(CFLAGS) -c $< -o $@
fom_data.o: src/fom_data.c $(HDRS)
\t$(CC) $(CFLAGS) -c $< -o $@
fom_linsolve.o: src/fom_linsolve.c $(HDRS)
\t$(CC) $(CFLAGS) -c $< -o $@
verify_main.o: test/verify_main.c test/verify_data.h $(HDRS)
\t$(CC) $(CFLAGS) -c $< -o $@
bench_main.o: test/bench_main.c test/verify_data.h $(HDRS)
\t$(CC) $(CFLAGS) -c $< -o $@
prof_main.o: test/bench_main.c test/verify_data.h $(HDRS)
\t$(CC) $(CFLAGS) -DFOM_PROFILE -c $< -o $@
prof_runtime.o: src/fom_profile.c $(HDRS)
\t$(CC) $(CFLAGS) -DFOM_PROFILE -c $< -o $@

fom_solver.o: $(SOLVER_SRC) $(HDRS)
\t$(CC) $(CFLAGS) -c $< -o $@
prof_solver.o: $(SOLVER_SRC) $(HDRS)
\t$(CC) $(CFLAGS) -DFOM_PROFILE -c $< -o $@

verify_bin: fom_solver.o $(FIXED) verify_main.o
\t$(CC) $(CFLAGS) $^ -o $@ -lm
bench_bin: fom_solver.o $(FIXED) bench_main.o
\t$(CC) $(CFLAGS) $^ -o $@ -lm
profile_bin: prof_solver.o $(FIXED) prof_main.o prof_runtime.o
\t$(CC) $(CFLAGS) $^ -o $@ -lm

verify: verify_bin
\t./verify_bin
bench: bench_bin
\t./bench_bin $(BENCH_SOLVES)
profile: profile_bin
\t./profile_bin $(BENCH_SOLVES)

clean-solver:
\trm -f fom_solver.o prof_solver.o verify_bin bench_bin profile_bin
"""

CUDA_MAKEFILE = """\
# Object-cached CUDA build (fomtools_full_pipeline). The deterministic
# sources (multi-GB __device__ matrix data) compile once; only the solver
# recompiles. Native sm_80 (A100) code only, no embedded PTX.
CC = cc
NVCC = nvcc
ARCH = -gencode arch=compute_80,code=sm_80
NVCCFLAGS = -O3 $(ARCH) -Iinclude -Itest -Xcompiler -mcmodel=medium
CFLAGS = -O3 -mcmodel=medium -Iinclude -Itest
SOLVER_SRC ?= src/fom_solver.cu
BENCH_SOLVES ?= 3
HDRS = $(wildcard include/*.h)
FIXED = fom_ops.o fom_data.o fom_linsolve.o

.PHONY: verify bench profile fixed clean-solver

fixed: $(FIXED) verify_main.o bench_main.o prof_main.o prof_runtime.o

fom_ops.o: src/fom_ops.cu $(HDRS)
\t$(NVCC) $(NVCCFLAGS) -c $< -o $@
fom_linsolve.o: src/fom_linsolve.cu $(HDRS)
\t$(NVCC) $(NVCCFLAGS) -c $< -o $@
fom_data.o: src/fom_data.c $(HDRS)
\t$(CC) $(CFLAGS) -c $< -o $@
verify_main.o: test/verify_main.c test/verify_data.h $(HDRS)
\t$(CC) $(CFLAGS) -c $< -o $@
bench_main.o: test/bench_main.c test/verify_data.h $(HDRS)
\t$(CC) $(CFLAGS) -c $< -o $@
# Profiling objects are compiled as CUDA C++ so fom_prof_* linkage matches the
# .cu solver; production objects never contain instrumentation.
prof_main.o: test/bench_main.c test/verify_data.h $(HDRS)
\t$(NVCC) $(NVCCFLAGS) -DFOM_PROFILE -x cu -c $< -o $@
prof_runtime.o: src/fom_profile.c $(HDRS)
\t$(NVCC) $(NVCCFLAGS) -DFOM_PROFILE -x cu -c $< -o $@

fom_solver.o: $(SOLVER_SRC) $(HDRS)
\t$(NVCC) $(NVCCFLAGS) -c $< -o $@
prof_solver.o: $(SOLVER_SRC) $(HDRS)
\t$(NVCC) $(NVCCFLAGS) -DFOM_PROFILE -c $< -o $@

verify_bin: fom_solver.o $(FIXED) verify_main.o
\t$(NVCC) $(NVCCFLAGS) $^ -o $@
bench_bin: fom_solver.o $(FIXED) bench_main.o
\t$(NVCC) $(NVCCFLAGS) $^ -o $@
profile_bin: prof_solver.o $(FIXED) prof_main.o prof_runtime.o
\t$(NVCC) $(NVCCFLAGS) $^ -o $@

verify: verify_bin
\t./verify_bin
bench: bench_bin
\t./bench_bin $(BENCH_SOLVES)
profile: profile_bin
\t./profile_bin $(BENCH_SOLVES)

clean-solver:
\trm -f fom_solver.o prof_solver.o verify_bin bench_bin profile_bin
"""


def _config(backend: Backend, tol_verify: float | None = None) -> CodegenConfig:
    return CodegenConfig(
        version=Version.LARGE_SCALE,
        backend=backend,
        termination=TerminationSpec(max_iters=ITERATIONS),
        tol_verify=tol_verify,
    )


def _matrix_stats(ir) -> str:
    lines = [
        "# Matrix statistics",
        "",
        "Generated from the IR. Use this file and include/fom_ops.h instead of",
        "opening src/fom_ops.* (multi-GB numeric literals).",
        "",
        "| matrix | shape | storage | nnz |",
        "|---|---|---|---:|",
    ]
    for name, mat in ir.matrices.items():
        array = mat.array
        nnz = getattr(array, "nnz", None)
        shape = getattr(array, "shape", None)
        storage = "csr" if nnz is not None else "dense"
        if nnz is None and shape is not None:
            nnz = int(array.size)
        lines.append(f"| `{name}` | {shape} | {storage} | {nnz} |")
    return "\n".join(lines) + "\n"


def main() -> None:
    algorithm = sys.argv[1]
    if algorithm not in ("ista", "fista"):
        raise SystemExit("usage: scaffold_pair.py {ista,fista}")
    root = Path(os.environ["ROOT"]) / algorithm
    module = Path(os.environ.get("PROBLEM_DIR") or os.environ["REPO"]) / f"paper_{algorithm}_sparse20k.py"
    tol = os.environ.get("FISTA_TOL_VERIFY" if algorithm == "fista" else "ISTA_TOL_VERIFY")
    tol = float(tol) if tol else None

    started = time.perf_counter()
    fom = _load_module_fom(str(module))
    ir = _extract_fom(fom, _config(Backend.OPENMP))
    extracted = time.perf_counter()
    print(f"EXTRACT_OK {algorithm} name={ir.name} seconds={extracted - started:.1f}", flush=True)
    if ir.termination.max_iters != ITERATIONS:
        raise RuntimeError(f"unexpected iteration count {ir.termination.max_iters}")

    openmp_dir = root / "openmp"
    omp_config = _config(Backend.OPENMP, tol)
    scaffold(ir, openmp_dir, omp_config, target=OpenMpTarget(omp_config))
    print(f"OPENMP_SCAFFOLD_OK seconds={time.perf_counter() - extracted:.1f}", flush=True)

    verify_data = (openmp_dir / "test" / "verify_data.h").read_text()

    class SharedOracleCudaTarget(CudaTarget):
        def _verify_data(self, loaded_ir):
            return verify_data

    cuda_dir = root / "cuda"
    cuda_started = time.perf_counter()
    cuda_config = _config(Backend.CUDA, tol)
    scaffold(ir, cuda_dir, cuda_config, target=SharedOracleCudaTarget(cuda_config))
    print(f"CUDA_SCAFFOLD_OK seconds={time.perf_counter() - cuda_started:.1f}", flush=True)

    stats = _matrix_stats(ir)
    for directory, makefile in ((openmp_dir, OPENMP_MAKEFILE), (cuda_dir, CUDA_MAKEFILE)):
        (directory / "Makefile.generated").write_text((directory / "Makefile").read_text())
        (directory / "Makefile").write_text(makefile)
        (directory / "MATRIX_STATS.md").write_text(stats)

    summary = {
        "algorithm": algorithm,
        "ir_name": ir.name,
        "m": int(os.environ["M"]),
        "n": int(os.environ["N"]),
        "nnz_per_row": int(os.environ["K"]),
        "iterations": ITERATIONS,
        "state_dim": int(ir.state_dim),
        "problem_module": str(module),
        "tol_verify": tol,
        "seconds_total": round(time.perf_counter() - started, 1),
    }
    (root / "scaffold_summary.json").write_text(json.dumps(summary, indent=2) + "\n")
    print("SCAFFOLD_PAIR_OK " + json.dumps(summary), flush=True)


if __name__ == "__main__":
    main()
