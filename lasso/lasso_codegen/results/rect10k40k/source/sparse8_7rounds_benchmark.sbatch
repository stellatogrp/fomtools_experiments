#!/bin/bash
#SBATCH --job-name=fom-rect-k8-r7
#SBATCH --account=bstellato
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=16
#SBATCH --mem=64G
#SBATCH --gres=gpu:1
#SBATCH --time=01:00:00
#SBATCH --output=/home/dd7022/fomtools_codegen_runs/rect10k40k_k8_7rounds/benchmark-%j.out

set -euo pipefail
module load cudatoolkit/12.9

REPO="$HOME/fomtools_rect10k40k"
SOURCE="$HOME/fomtools_codegen_runs/rect10k40k_study"
OUT="$HOME/fomtools_codegen_runs/rect10k40k_k8_7rounds"
PYTHON="$HOME/fomtools/.venv/bin/python"

mkdir -p "$OUT"
exec > >(tee -a "$OUT/benchmark-runner.log") 2>&1

export PYTHONPATH="$REPO"
export FOMTOOLS_EXPERIMENT_M=10000
export FOMTOOLS_EXPERIMENT_N=40000
export FOMTOOLS_EXPERIMENT_A_NNZ_PER_ROW=8
export FOMTOOLS_CUDA_RUN=1
export OMP_NUM_THREADS="$SLURM_CPUS_PER_TASK"

{
  echo "RUN_DATE=$(date -Is)"
  echo "SOURCE_COMMIT=$(git -C "$REPO" rev-parse HEAD)"
  echo "SOURCE_GENERATED_DIR=$SOURCE"
  echo "ALLOCATION_JOB_ID=$SLURM_JOB_ID"
  hostname
  nvidia-smi -L
  lscpu | grep -E 'Model name|Socket|Core|Thread|CPU\(s\)'
  nvcc --version | tail -1
  "$PYTHON" --version
} > "$OUT/benchmark_metadata.txt"

for algorithm in ista fista; do
  for backend in openmp cuda; do
    target="$SOURCE/prod_${algorithm}_${backend}_optimized"
    make -C "$target" verify 2>&1 | tee "$OUT/verify_${algorithm}_${backend}.log"
  done
done

for round in 1 2 3 4 5 6 7; do
  echo "ROUND $round"
  "$PYTHON" "$REPO/bench_python_sparse.py" ista --reps 3
  echo "CPU_OPENMP_ISTA"
  "$SOURCE/prod_ista_openmp_optimized/bench_bin" 10
  echo "CUDA_ISTA"
  "$SOURCE/prod_ista_cuda_optimized/bench_bin" 10
  "$PYTHON" "$REPO/bench_python_sparse.py" fista --reps 3
  echo "CPU_OPENMP_FISTA"
  "$SOURCE/prod_fista_openmp_optimized/bench_bin" 10
  echo "CUDA_FISTA"
  "$SOURCE/prod_fista_cuda_optimized/bench_bin" 10
done | tee "$OUT/production_interleaved_7rounds.txt"

echo "SPARSE8_7ROUND_OK $(date -Is)" | tee "$OUT/SPARSE8_7ROUND_OK"
