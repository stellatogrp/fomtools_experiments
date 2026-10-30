#!/bin/bash
# Final benchmark: rebuild and re-verify every generated solver on one node, then
# seven interleaved rounds against SciPy (1 core) and CuPy. The paper run used an
# AMD EPYC 7H12 + A100 40 GB node; override the node class at submission with
# --constraint (e.g. "nomig&gpu40") or --nodelist. Submit from the project root:
#   sbatch --export=ALL,RUN=v0.0.4 lasso/lasso_codegen/della/bench.sbatch
#SBATCH --job-name=fom-bench
#SBATCH --account=bstellato
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=16
#SBATCH --mem=128G
#SBATCH --gres=gpu:1
#SBATCH --constraint=gpu80
#SBATCH --exclusive
#SBATCH --time=04:00:00
#SBATCH --output=slurm/logs/%x-%j.out

set -euo pipefail
# Nothing is timed outside a batch job on an A100 node (never on a login node).
if [[ -z "${SLURM_JOB_ID:-}" ]]; then
  echo "bench.sbatch: not inside a Slurm job; submit it with sbatch" >&2; exit 1
fi
if ! nvidia-smi --query-gpu=name --format=csv,noheader 2> /dev/null | grep -q "A100"; then
  echo "bench.sbatch: no A100 visible on $(hostname); refusing to benchmark" >&2; exit 1
fi
source "$SLURM_SUBMIT_DIR/lasso/lasso_codegen/della/env.sh"
ulimit -s unlimited
cd "$PROJECT_DIR/lasso"
"$PY" -m lasso_codegen.benchmark --run "${RUN:?set RUN, e.g. --export=ALL,RUN=v0.0.4}"
