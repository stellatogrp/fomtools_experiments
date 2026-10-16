#!/bin/bash
# After the clean regeneration of the original-form ISTA CUDA solver: add it to the
# 325k sepA comparison and rerun that benchmark (prep + seven whole-node rounds) with
# all eight solvers. Runs on the login node (systemd user service).
#  1. wait for $NEW/ista/cuda/FINAL_VERIFIED with PROVENANCE_OK; stop on PIPELINE_FAILED;
#  2. archive the sepA run's 7-solver benchmark (moved, not deleted);
#  3. copy the new solver in as orig_ista/cuda and become bench_supervisor.sh.
# Usage: rebench_with_orig_ista_cuda.sh NEW_ROOT SEPA_ROOT   (env: FOM_* of the sepA benchmark)
set -euo pipefail
NEW="${1:?new ROOT}"; SEPA="${2:?sepA ROOT}"
exec >> "$SEPA/rebench.log" 2>&1
log() { echo "[$(date -Is)] rebench: $*"; }
log "waiting for $NEW/ista/cuda (FINAL_VERIFIED + PROVENANCE_OK)"
until [[ -f "$NEW/ista/cuda/FINAL_VERIFIED" ]]; do
  [[ -f "$NEW/PIPELINE_FAILED" ]] && { log "regeneration failed: $(cat "$NEW/PIPELINE_FAILED"); stopping"; exit 1; }
  sleep 60
done
grep -qx PROVENANCE_OK <(tail -n 1 "$NEW/ista/cuda/provenance.log") \
  || { log "no PROVENANCE_OK for the new solver; stopping"; exit 1; }
log "new solver verified: $(tr '\n' ' ' < "$NEW/ista/cuda/FINAL_VERIFIED")"

stamp="$(date +%Y%m%dT%H%M%S)"
archive="$SEPA/benchmark_7solvers_$stamp"
[[ -d "$SEPA/benchmark" ]] && mv "$SEPA/benchmark" "$archive"
[[ -f "$SEPA/PIPELINE_COMPLETE" ]] && mv "$SEPA/PIPELINE_COMPLETE" "$archive/PIPELINE_COMPLETE"
log "archived the 7-solver benchmark to $archive"
[[ -e "$SEPA/orig_ista/cuda" ]] && { log "orig_ista/cuda already exists; stopping"; exit 1; }
mkdir -p "$SEPA/orig_ista"
cp -a "$NEW/ista/cuda" "$SEPA/orig_ista/cuda"
log "copied $NEW/ista/cuda -> $SEPA/orig_ista/cuda"
exec bash "$HOME/fomtools_full_pipeline/bench_supervisor.sh" controller 00:59:00
