"""Time one sparse Python-reference solve, excluding construction/extraction."""

import argparse
import importlib
import statistics
import time

from fomtools.codegen.config import Backend, CodegenConfig, TerminationSpec, Version
from fomtools.codegen.generate import extract_fom_ir
from fomtools.codegen.harness import representative_params
from fomtools.codegen.oracle import ReferenceOracle

parser = argparse.ArgumentParser()
parser.add_argument("algorithm", choices=("ista", "fista"))
parser.add_argument("--reps", type=int, default=3)
args = parser.parse_args()

module = importlib.import_module(f"paper_{args.algorithm}_sparse20k")
fom = module.build_fom()
config = CodegenConfig(
    version=Version.LARGE_SCALE,
    backend=Backend.CUDA,
    termination=TerminationSpec(max_iters=200),
)
if fom.get("schedules"):
    ir = extract_fom_ir(fom["build"], config=config, name=fom["name"], schedules=fom["schedules"])
else:
    ir = extract_fom_ir(
        fom["verifier"],
        fom["next_state"],
        config=config,
        name=fom["name"],
        iterates=fom["iterates"],
        param_values=fom["param_values"],
    )

oracle = ReferenceOracle(ir)
params = representative_params(ir)
oracle.solve(param_values=params)
samples = []
for _ in range(args.reps):
    start = time.perf_counter_ns()
    oracle.solve(param_values=params)
    samples.append((time.perf_counter_ns() - start) / 1000.0)

print(
    f"NUMPY_{args.algorithm.upper()} reps={args.reps} "
    f"median_us={statistics.median(samples):.6f} min_us={min(samples):.6f}"
)
