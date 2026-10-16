#!/bin/bash
# Run launcher (login node, systemd user service; environment FOM_* set by the
# caller, as for config.sh):
#  1. wait for stage 1 (01_scaffold) and the baseline gate (baseline_gate.sh);
#     stop if the gate fails;
#  2. wait for stage 1b (01b_cuda_objects);
#  3. become controller_builtin.sh (FOMTools' default prompts, clean-workdir guard,
#     provenance gate), stopping before the benchmark.
# Usage: gate_launch.sh STAGE1_JOBID GATE_JOBID STAGE1B_JOBID   (env: TOL_VERIFY)
set -euo pipefail
S1="${1:?stage-1 job}"; GATE="${2:?gate job}"; S1B="${3:?stage-1b job}"
source "$HOME/fomtools_full_pipeline/config.sh"
exec >> "$ROOT/launch.log" 2>&1
log() { echo "[$(date -Is)] launch: $*"; }
fail() { log "STOP: $*"; echo "$* ($(date -Is))" > "$ROOT/PIPELINE_FAILED"; exit 1; }
state() { sacct -j "$1" -X -n -P -o State | head -1; }
ended_badly() { case "$(state "$1")" in FAILED*|CANCELLED*|TIMEOUT*|OUT_OF_ME*|NODE_FAIL*) return 0 ;; esac; return 1; }
log "start: ROOT=$ROOT PROBLEM_DIR=$PROBLEM_DIR M=$M N=$N K=$K stage1=$S1 gate=$GATE stage1b=$S1B"

until [[ -f "$ROOT/BASELINE_GATE.txt" ]]; do
  for j in "$S1" "$GATE"; do ended_badly "$j" && fail "job $j ended $(state "$j") before the gate result"; done
  [[ "$(state "$GATE")" == COMPLETED ]] && { sleep 30; [[ -f "$ROOT/BASELINE_GATE.txt" ]] || fail "gate job $GATE completed without a result"; }
  sleep 60
done
log "baseline gate: $(tr '\n' ' ' < "$ROOT/BASELINE_GATE.txt")"
grep -qx BASELINE_GATE_OK "$ROOT/BASELINE_GATE.txt" || fail "baseline gate failed (see BASELINE_GATE.txt)"

ready() {
  local a
  [[ -f "$ROOT/SCAFFOLD_OK" ]] && grep -q CUDA_OBJECTS_OK "$ROOT/01b_cuda_objects.log" 2> /dev/null || return 1
  for a in $ALGORITHMS; do [[ -f "$ROOT/$a/cuda/CUDA_FIXED_OK" ]] || return 1; done
}
until ready; do
  ended_badly "$S1B" && fail "stage 1b job $S1B ended $(state "$S1B")"
  sleep 60
done
log "fixed files ready"

log "starting controller_builtin.sh perjob"
export TOL_VERIFY="${TOL_VERIFY:?set TOL_VERIFY}" STOP_BEFORE_BENCHMARK=1
exec bash "$HOME/fomtools_full_pipeline/controller_builtin.sh" perjob
