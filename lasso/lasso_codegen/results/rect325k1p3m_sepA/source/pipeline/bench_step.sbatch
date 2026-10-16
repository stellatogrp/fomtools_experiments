#!/bin/bash
# One benchmark step (03_benchmark.sh prep | round N) as its own Slurm job on an
# AMD EPYC 7H12 + A100 node; bench_supervisor.sh pins it with --nodelist and
# sets --time. Usage: sbatch [--nodelist=NODE] [--time=T] bench_step.sbatch prep|round N
#SBATCH --job-name=fom-bench-step
#SBATCH --account=bstellato
#SBATCH --gres=gpu:a100:1
#SBATCH --constraint=amd&rome&a100
# Never run on H100 nodes (redundant with gres/constraint, kept deliberately).
#SBATCH --exclude=della-h19g[1-4],della-h20g[1-5],della-h21g[1-4],della-j11g1,della-j12g[1-3],della-j13g[1-3],della-j14g[1-3],della-j15g[1-3],della-j16g[1-3],della-j17g[1-2],della-k11g[1-3],della-k12g[1-3],della-k13g[1-3],della-k14g[1-3],della-k15g[1-3],della-k16g[1-3],della-k17g[1-3],della-k18g[1-3]
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=16
#SBATCH --mem=240G
#SBATCH --time=00:59:00
#SBATCH --output=/scratch/gpfs/BSTELLATO/dd7022/fomtools_codegen_runs/slurm-%x-%j.out

set -euo pipefail
exec bash "$HOME/fomtools_full_pipeline/03_benchmark.sh" "$@"
