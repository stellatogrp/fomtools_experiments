#!/bin/bash
# One verification variant per array task, on della. Submitted by
# `python -m fomexp.submit`, which allocates RUN_DIR and passes EXPERIMENT,
# RUN_DIR and GRB_LICENSE_FILE; the task index selects the variant.
#SBATCH --nodes=1
#SBATCH --ntasks=1
set -euo pipefail
cd "$SLURM_SUBMIT_DIR"
export GRB_LICENSE_FILE
mapfile -t overrides < "$RUN_DIR/${OVERRIDES_FILE:-overrides.txt}"
if [[ -n "${VARIANT:-}" ]]; then select="variant=$VARIANT"; else select="variant_index=$SLURM_ARRAY_TASK_ID"; fi
exec .venv/bin/python -m fomexp.run "experiment=$EXPERIMENT" "run_dir=$RUN_DIR" "$select" "${overrides[@]}"
