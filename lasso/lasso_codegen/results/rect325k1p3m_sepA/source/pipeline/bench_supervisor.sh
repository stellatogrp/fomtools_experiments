#!/bin/bash
# Final benchmark for a run whose rounds exceed the 1 h test queue (e.g. the
# K*-iteration convergence runs). Runs on the login node (systemd user service):
#  1. wait until every target in $TARGETS has FINAL_VERIFIED (stop if VERIFY_JOB failed);
#  2. prep, then ROUNDS rounds, each its own Slurm job pinned to $BENCH_PREFERRED_NODE
#     (FOM_BENCH_NODE=any: prep takes the first free AMD EPYC 7H12 + A100 node and
#     every round is pinned to the node prep ran on);
#  3. finalize (summary.csv / summary.md).
# A round is contaminated if any other job using >= 2 CPUs (any user's, including
# our own agent and holder jobs) ran on the benchmark node at any time during that
# round (Slurm accounting; decided before looking at the timings); it is moved to
# benchmark/discarded/ and rerun. With FOM_BENCH_NODE=any, prep avoids nodes running
# our holder jobs or claimed by another of our benchmarks, and this run registers
# its node in bench_nodes_in_use/ so new holders stay off it.
# Usage: bench_supervisor.sh VERIFY_JOBID|controller ROUND_TIME   (env: FOM_* as for config.sh)
# With "controller", wait for the controller to verify every target (stop on PIPELINE_FAILED).
set -Eeuo pipefail
source "$HOME/fomtools_full_pipeline/config.sh"
VERIFY_JOB="${1:?verify job id}"; ROUND_TIME="${2:?round time limit, e.g. 02:00:00}"
OUT="$ROOT/benchmark"; AUDIT="$OUT/rounds_audit.txt"; MAX_ATTEMPTS=6
NODE="$BENCH_PREFERRED_NODE"
mkdir -p "$OUT/discarded"
exec >> "$ROOT/bench_supervisor.log" 2>&1
log() { echo "[$(date -Is)] bench: $*"; }
REGISTRY="$PIPE/bench_nodes_in_use"
CLAIM="$REGISTRY/$(basename "$ROOT")"
mkdir -p "$REGISTRY"
trap 'rm -f "$CLAIM"' EXIT
log "start: targets=[$TARGETS] iterations=$ITERATIONS node=$NODE round_time=$ROUND_TIME verify_job=$VERIFY_JOB"

all_verified() { local t; for t in $TARGETS; do [[ -f "$ROOT/$t/FINAL_VERIFIED" ]] || return 1; done; }
until all_verified; do
  if [[ "$VERIFY_JOB" == controller ]]; then
    [[ -f "$ROOT/PIPELINE_FAILED" ]] && { log "controller failed: $(cat "$ROOT/PIPELINE_FAILED"); stopping"; exit 1; }
    sleep 120
    continue
  fi
  st="$(sacct -j "$VERIFY_JOB" -X -n -P -o State | head -1)"
  case "$st" in
    COMPLETED) sleep 30; all_verified || { log "verify job $VERIFY_JOB completed but not every target verified; stopping"; exit 1; } ;;
    FAILED*|CANCELLED*|TIMEOUT*|OUT_OF_ME*|NODE_FAIL*) log "verify job $VERIFY_JOB ended $st; stopping"; exit 1 ;;
    *) sleep 120 ;;
  esac
done
log "all targets verified"

# Extra targets (e.g. the original-form solvers in a variant run) are copied in
# only now, after every agent of this run has finished, so no agent saw them.
if [[ -n "${EXTRA_TARGETS:-}" ]]; then
  [[ -n "${ORIG_FROM:-}" ]] || { log "EXTRA_TARGETS set but ORIG_FROM is not; stopping"; exit 1; }
  for t in $EXTRA_TARGETS; do
    [[ -d "$ROOT/$t" ]] && continue
    src="$ORIG_FROM/${t#orig_}"
    [[ -f "$src/FINAL_VERIFIED" ]] || { log "no verified solver at $src; stopping"; exit 1; }
    mkdir -p "$(dirname "$ROOT/$t")"
    cp -a "$src" "$ROOT/$t"
    log "copied $src -> $ROOT/$t (solver_sha256 $(sed -n 's/^solver_sha256=//p' "$ROOT/$t/FINAL_VERIFIED"))"
  done
fi

avoid_nodes() {  # H100s, nodes running our holder jobs, nodes claimed by our other benchmarks
  local avoid="$H100_EXCLUDE" n f
  for n in $(squeue -h -u "$USER" -n fom-holder -t RUNNING -o %N 2>/dev/null); do avoid="$avoid,$n"; done
  for f in "$REGISTRY"/*; do
    [[ -s $f && $f != "$CLAIM" ]] && avoid="$avoid,$(cat "$f")"
  done
  echo "$avoid"
}

step() {  # step NAME TIME ARGS...: run one bench_step job on $NODE (a free 7H12 + A100 node if "any")
  local name=$1 time=$2; shift 2
  local where=(--nodelist="$NODE")
  [[ "$NODE" == any ]] && where=(--exclude="$(avoid_nodes)")
  # Timed rounds take the whole node, so no other job can share it during a round.
  [[ $name == r* ]] && where+=(--exclusive)
  sbatch --parsable --wait "${where[@]}" --time="$time" --job-name="fom-bench-$name" \
    --export=ALL "$PIPE/bench_step.sbatch" "$@"
}

if [[ ! -f "$OUT/BENCH_NODE" ]]; then
  for attempt in 1 2; do
    step prep 00:59:00 prep > "$OUT/prep.step.log" 2>&1 && break || log "prep attempt $attempt failed"
  done
  [[ -f "$OUT/BENCH_NODE" ]] || { log "prep failed; stopping"; exit 1; }
fi
NODE="$(cat "$OUT/BENCH_NODE")"  # every round runs on the node prep ran on
echo "$NODE" > "$CLAIM"
log "prep done on $NODE; all rounds pinned there (registered in $REGISTRY)"

neighbours() {  # any other job (any user, >= 2 CPUs) overlapping round job $1 on $NODE
  local job=$1 start end
  read -r start end < <(sacct -j "$job" -X -n -P -o Start,End | tr '|' ' ')
  sacct -a -N "$NODE" -S "$start" -E "$end" -X -n -P -o JobID,User,JobName,AllocCPUS,Start,End \
    | awk -F'|' -v job="$job" -v s="$start" -v e="$end" \
        '$1 != job && $4 >= 2 && $5 != "Unknown" && $5 < e && ($6 == "Unknown" || $6 > s)'
}

check_round() {  # 0 if clean (keep), 1 if contaminated (moved aside)
  local r=$1 f="$OUT/round_$1.txt" job hits n
  job="$(grep -oE 'job=[0-9]+' "$f" | head -1 | cut -d= -f2)"
  hits="$(neighbours "$job")"
  if [[ -z "$hits" ]]; then echo "round $r job $job: clean" >> "$AUDIT"; return 0; fi
  n=$(ls "$OUT/discarded" | grep -c "^round_${r}_" || true)
  mv "$f" "$OUT/discarded/round_${r}_attempt$((n + 1)).txt"
  { echo "round $r job $job: CONTAMINATED -> discarded/round_${r}_attempt$((n + 1)).txt"
    echo "$hits" | sed 's/^/    neighbour: /'; } >> "$AUDIT"
  return 1
}

# BATCH_ROUNDS=1: all missing rounds back to back in one whole-node job (same rounds,
# same node, one queue wait instead of one per round); each round is still checked.
if [[ "${BATCH_ROUNDS:-0}" == 1 ]]; then
  for attempt in $(seq 1 "$MAX_ATTEMPTS"); do
    missing=()
    for r in $(seq 1 "$ROUNDS"); do [[ -f "$OUT/round_$r.txt" ]] || missing+=("$r"); done
    (( ${#missing[@]} )) || break
    log "batch attempt $attempt: rounds ${missing[*]}"
    step "rb$attempt" "$ROUND_TIME" rounds "${missing[@]}" > "$OUT/rounds_batch_${attempt}.step.log" 2>&1 \
      || log "batch job failed (attempt $attempt); keeping the rounds it completed"
    for r in "${missing[@]}"; do
      [[ -f "$OUT/round_$r.txt" ]] || continue
      if check_round "$r"; then log "round $r clean (batch attempt $attempt)"; else log "round $r contaminated; discarded"; fi
    done
  done
fi

for r in $(seq 1 "$ROUNDS"); do
  [[ "${BATCH_ROUNDS:-0}" == 1 && -f "$OUT/round_$r.txt" ]] && continue
  [[ "${BATCH_ROUNDS:-0}" == 1 ]] && { log "round $r still missing after $MAX_ATTEMPTS batch attempts; stopping"; exit 1; }
  for attempt in $(seq 1 "$MAX_ATTEMPTS"); do
    if [[ ! -f "$OUT/round_$r.txt" ]]; then
      step "r$r" "$ROUND_TIME" round "$r" > "$OUT/round_${r}.step.log" 2>&1 \
        || { log "round $r job failed (attempt $attempt)"; continue; }
    fi
    if check_round "$r"; then log "round $r clean (attempt $attempt)"; break; fi
    log "round $r contaminated; discarded (attempt $attempt)"
  done
  [[ -f "$OUT/round_$r.txt" ]] || { log "round $r: no clean round after $MAX_ATTEMPTS attempts; stopping"; exit 1; }
done

bash "$PIPE/03_benchmark.sh" finalize > "$ROOT/benchmark_finalize.log" 2>&1
log "all $ROUNDS rounds clean; summary written"
cat "$OUT/summary.md"
