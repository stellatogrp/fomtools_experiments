#!/bin/bash
# Gate before any agent runs (CPU job after stage 1): the hand-written SciPy
# iteration with D and D^T must reproduce this run's FOMTools reference execution
# (from its own scaffold) after $ITERATIONS iterations, for every algorithm.
# Writes $ROOT/BASELINE_GATE.txt ending in BASELINE_GATE_OK or BASELINE_GATE_FAILED.
set -uo pipefail
source "$HOME/fomtools_full_pipeline/config.sh"
out="$ROOT/BASELINE_GATE.txt"
: > "$out.partial"
ok=1
for alg in $ALGORITHMS; do
  "$PYTHON" -u "$PIPE/baselines.py" check "$alg" --iters "$ITERATIONS" --skip-form \
    --oracle-dir "$ROOT/$alg/openmp" --tol "${BASELINE_TOL:-1e-6}" >> "$out.partial" 2>&1 || ok=0
done
echo "eta/lambda/shape from $PROBLEM_DIR (M=$M N=$N K=$K ITERATIONS=$ITERATIONS)" >> "$out.partial"
[[ $ok == 1 ]] && echo BASELINE_GATE_OK >> "$out.partial" || echo BASELINE_GATE_FAILED >> "$out.partial"
mv "$out.partial" "$out"
cat "$out"
[[ $ok == 1 ]]
