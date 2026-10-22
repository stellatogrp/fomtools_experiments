# Environment shared by every della step of the large-scale Lasso run: sourced by
# the agent run on the login node, the make wrapper's GPU steps (they inherit it), and the
# job scripts. Keeps the build flags and threading of the agent's builds and of the
# final benchmark identical.
if [ -z "${BASH_SOURCE:-}" ]; then  # zsh has no BASH_SOURCE: the paths below would be wrong
  echo "env.sh: source it from bash (e.g. bash -c 'source .../env.sh && ...')" >&2
  return 1
fi
PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)"
export PROJECT_DIR
PY="$PROJECT_DIR/.venv/bin/python"
# The id of the running 1 h holder the builds go to, kept up to date by holders.sh.
export HOLDER_FILE="$PROJECT_DIR/lasso/out/codegen/holder_current"
# The make wrapper first: on the login node it sends every build to the holder
# (inside a Slurm job it is plain make).
export PATH="$PROJECT_DIR/lasso/lasso_codegen/della:$PATH"

set +u
type module > /dev/null 2>&1 || source /etc/profile.d/modules.sh 2> /dev/null || true
# The paper run used CUDA 12.9; the newest toolkit on della is 13.3.
module load "${CUDA_MODULE:-cudatoolkit/13.3}" > /dev/null 2>&1 \
  || echo "env.sh: could not load ${CUDA_MODULE:-cudatoolkit/13.3} on $(hostname)" >&2
set -u

# OpenMP: 16 threads, one per core, packed (the paper run's settings). The generated
# Makefile builds serial unless OMPFLAGS says otherwise.
export OMP_NUM_THREADS=16 OMP_PLACES=cores OMP_PROC_BIND=close
export OMPFLAGS=-fopenmp
# CUDA: native sm_80 (A100) code only, as in the paper run; this replaces the
# Makefile's default NVCCFLAGS, so it repeats its include flags.
export NVCCFLAGS="-O3 -Iinclude -Itest -gencode arch=compute_80,code=sm_80"
# The login node has no GPU: make the CUDA target run verify (in the allocation)
# instead of only compiling.
export FOMTOOLS_CUDA_RUN=1
export FOMTOOLS_CODEGEN_AGENT=1
# Logs are files: flush every line so progress shows while an agent runs.
export PYTHONUNBUFFERED=1
export ANTHROPIC_MODEL="${ANTHROPIC_MODEL:-claude-opus-5-5}"
