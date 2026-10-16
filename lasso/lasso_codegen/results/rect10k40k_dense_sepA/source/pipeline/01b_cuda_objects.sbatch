#!/bin/bash
# Stage 1b (CPU only, no GPU): compile the multi-GB deterministic CUDA objects
# once and link-test stub binaries. CPU nodes have no CUDA toolkit, so this
# uses a copy of /usr/local/cuda-12.9 on scratch (nvcc needs no GPU). Doing it
# here keeps the hour-long nvcc runs out of the <=1 h GPU test-queue jobs.
#SBATCH --job-name=fom-full-cuda-objs
#SBATCH --account=bstellato
#SBATCH --partition=cpu
#SBATCH --constraint=genoa
# Never run on H100 nodes (redundant with --partition=cpu, kept deliberately).
#SBATCH --exclude=della-h19g[1-4],della-h20g[1-5],della-h21g[1-4],della-j11g1,della-j12g[1-3],della-j13g[1-3],della-j14g[1-3],della-j15g[1-3],della-j16g[1-3],della-j17g[1-2],della-k11g[1-3],della-k12g[1-3],della-k13g[1-3],della-k14g[1-3],della-k15g[1-3],della-k16g[1-3],della-k17g[1-3],della-k18g[1-3]
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=8
#SBATCH --mem=256G
#SBATCH --time=08:00:00
#SBATCH --output=/scratch/gpfs/BSTELLATO/dd7022/fomtools_codegen_runs/slurm-%x-%j.out

set -euo pipefail
source "$HOME/fomtools_full_pipeline/config.sh"
export CUDA_HOME="$SCRATCH_CUDA_HOME"
export PATH="$CUDA_HOME/bin:$PATH"
ulimit -s unlimited
test -f "$ROOT/SCAFFOLD_OK"
exec > >(tee -a "$ROOT/01b_cuda_objects.log") 2>&1
echo "CUDA_OBJECTS_START $(date -Is) host=$(hostname) nvcc=$(nvcc --version | tail -1)"

build() {
  local d="$ROOT/$1/cuda"
  rm -f "$d/CUDA_FIXED_OK"
  (cd "$d" && sha256sum --quiet -c immutable.sha256)
  /usr/bin/time -v make -C "$d" -j3 fixed > "$d/build_fixed.log" 2>&1
  mkdir -p "$d/stub"
  cp "$PIPE/stub_solver.c" "$d/stub/stub_solver.cu"
  make -C "$d" SOLVER_SRC=stub/stub_solver.cu verify_bin bench_bin > "$d/stub_link.log" 2>&1
  ls -l "$d/verify_bin" "$d/bench_bin" >> "$d/stub_link.log"
  make -C "$d" clean-solver > /dev/null
  rm -rf "$d/stub"
  date -Is > "$d/CUDA_FIXED_OK"
}

algorithms=($ALGORITHMS)
pids=()
for algorithm in "${algorithms[@]}"; do
  build "$algorithm" &
  pids+=("$!")
done
status=0
for i in "${!algorithms[@]}"; do
  algorithm="${algorithms[$i]}"
  if wait "${pids[$i]}"; then
    echo "CUDA_FIXED_OK $algorithm $(date -Is)"
    grep -E "Elapsed|Maximum resident" "$ROOT/$algorithm/cuda/build_fixed.log"
    tail -2 "$ROOT/$algorithm/cuda/stub_link.log"
  else
    echo "CUDA_FIXED_FAILED $algorithm; see $ROOT/$algorithm/cuda/build_fixed.log and stub_link.log"
    status=1
  fi
done
[[ $status == 0 ]] && echo "CUDA_OBJECTS_OK $(date -Is)"
exit "$status"
