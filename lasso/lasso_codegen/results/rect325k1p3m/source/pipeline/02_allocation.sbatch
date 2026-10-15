#!/bin/bash
# Stage 2 allocation holder: one A100 + 16 cores on an AMD EPYC 7H12 node (the
# paper's hardware: della-i14g/i12g), reserved for the whole
# agent loop AND the final benchmark, so optimization decisions and reported
# timings come from the same node. The login-node controller drives `make`
# here through bridge/make (srun --overlap). This job only waits.
#SBATCH --job-name=fom-full-a100
#SBATCH --account=bstellato
#SBATCH --gres=gpu:a100:1
#SBATCH --constraint=amd&rome&a100
# Never run on H100 nodes (redundant with gres/constraint, kept deliberately).
#SBATCH --exclude=della-h19g[1-4],della-h20g[1-5],della-h21g[1-4],della-j11g1,della-j12g[1-3],della-j13g[1-3],della-j14g[1-3],della-j15g[1-3],della-j16g[1-3],della-j17g[1-2],della-k11g[1-3],della-k12g[1-3],della-k13g[1-3],della-k14g[1-3],della-k15g[1-3],della-k16g[1-3],della-k17g[1-3],della-k18g[1-3]
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=16
#SBATCH --mem=240G
#SBATCH --time=48:00:00
#SBATCH --output=/scratch/gpfs/BSTELLATO/dd7022/fomtools_codegen_runs/slurm-%x-%j.out

set -euo pipefail
source "$HOME/fomtools_full_pipeline/config.sh"
test -f "$ROOT/SCAFFOLD_OK"
module load "$CUDA_MODULE"

gpu="$(nvidia-smi -L)"
if ! grep -q "A100" <<<"$gpu" || grep -q "H100" <<<"$gpu"; then
  echo "allocated GPU is not an A100: $gpu" | tee "$ROOT/PIPELINE_FAILED"
  exit 1
fi
# The paper reports an AMD EPYC 7H12 CPU and an A100 on the same node.
cpu="$(lscpu | sed -n 's/^Model name: *//p')"
if ! grep -q "EPYC 7H12" <<<"$cpu"; then
  echo "allocated CPU is not an AMD EPYC 7H12: $cpu ($(hostname))" | tee "$ROOT/PIPELINE_FAILED"
  exit 1
fi

{
  echo "ALLOCATION_JOB_ID=$SLURM_JOB_ID"
  echo "HOST=$(hostname)"
  echo "$gpu"
  lscpu | grep -E 'Model name|Socket|Core|Thread|CPU\(s\)'
  echo "ALLOCATED_CPUS=$SLURM_CPUS_PER_TASK"
  date -Is
} > "$ROOT/ALLOC_READY"

trap 'date -Is > "$ROOT/ALLOC_ENDED"' EXIT
while [[ ! -f "$ROOT/PIPELINE_COMPLETE" && ! -f "$ROOT/PIPELINE_FAILED" \
        && ! -f "$ROOT/ALLOC_RELEASE" ]]; do
  sleep 30
done
