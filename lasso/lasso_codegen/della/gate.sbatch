#!/bin/bash
# Baseline gate, before any agent: SciPy with D and D^T against fomtools' reference
# execution. CPU only. Submit from the project root.
#SBATCH --job-name=fom-gate
#SBATCH --account=bstellato
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=1
#SBATCH --mem=32G
#SBATCH --time=01:00:00
#SBATCH --output=slurm/logs/%x-%j.out

set -euo pipefail
source "$SLURM_SUBMIT_DIR/lasso/lasso_codegen/della/env.sh"
export OMP_NUM_THREADS=1
cd "$PROJECT_DIR/lasso"
"$PY" -m lasso_codegen.gate
