#!/bin/bash
# sepA run (D and D^T kept separate; same instance, step size and algorithm as the
# rect studies). Runs on the login node (systemd user service):
#  1. wait for sepa_check.py (sepA vs original: identical step size, same
#     iterates); stop if it fails;
#  2. wait for stage 1 (01_scaffold) and stage 1b (01b_cuda_objects);
#  3. become controller_builtin.sh (FOMTools' default prompts, clean-workdir
#     guard, provenance gate), stopping before the benchmark.
# Usage: sepa_launch.sh CHECK_JOBID STAGE1_JOBID STAGE1B_JOBID
set -euo pipefail
CHECK="${1:?check job}"; S1="${2:?stage-1 job}"; S1B="${3:?stage-1b job}"
export FOM_M=325000 FOM_N=1300000 FOM_VARIANT=sepA
export FOM_PROBLEM_DIR="$HOME/fomtools_full_pipeline/problems_v2"
export FOM_ISTA_TOL=1e-5 FOM_FISTA_TOL=1e-5
source "$HOME/fomtools_full_pipeline/config.sh"
exec >> "$ROOT/launch.log" 2>&1
log() { echo "[$(date -Is)] launch: $*"; }
fail() { log "STOP: $*"; echo "$* ($(date -Is))" > "$ROOT/PIPELINE_FAILED"; exit 1; }
state() { sacct -j "$1" -X -n -P -o State | head -1; }
ended_badly() { case "$(state "$1")" in FAILED*|CANCELLED*|TIMEOUT*|OUT_OF_ME*|NODE_FAIL*) return 0 ;; esac; return 1; }
log "start: ROOT=$ROOT PROBLEM_DIR=$PROBLEM_DIR check=$CHECK stage1=$S1 stage1b=$S1B"

until [[ -f "$ROOT/SEPA_CHECK.txt" ]]; do
  ended_badly "$CHECK" && fail "sepa_check job $CHECK ended $(state "$CHECK") without a result"
  [[ "$(state "$CHECK")" == COMPLETED ]] && { sleep 30; [[ -f "$ROOT/SEPA_CHECK.txt" ]] || fail "sepa_check job $CHECK completed without a result"; }
  sleep 60
done
log "sepa_check: $(tr '\n' ' ' < "$ROOT/SEPA_CHECK.txt")"
grep -qx SEPA_CHECK_OK "$ROOT/SEPA_CHECK.txt" || fail "sepA is not the same algorithm as the original (see SEPA_CHECK.txt)"

ready() {
  [[ -f "$ROOT/SCAFFOLD_OK" && -f "$ROOT/ista/cuda/CUDA_FIXED_OK" && -f "$ROOT/fista/cuda/CUDA_FIXED_OK" ]] \
    && grep -q CUDA_OBJECTS_OK "$ROOT/01b_cuda_objects.log" 2> /dev/null
}
until ready; do
  for j in "$S1" "$S1B"; do ended_badly "$j" && fail "job $j ended $(state "$j")"; done
  sleep 120
done
log "fixed files ready"

log "starting controller_builtin.sh perjob"
export TOL_VERIFY=1e-5 STOP_BEFORE_BENCHMARK=1
exec bash "$HOME/fomtools_full_pipeline/controller_builtin.sh" perjob
