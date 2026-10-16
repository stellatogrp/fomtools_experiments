#!/bin/bash
# Stage 2 controller (FOMTools built-in agent flow). Same as controller.sh, but
# translation and code optimization run through fomtools_agent.py: FOMTools'
# own instructions, default driver and verify loop, as generate_code does.
#
# Stage 2 controller. Runs on the login node (the agent needs network access)
# inside tmux; every `make` goes through bridge/make -> bridge/onnode.
#
# Per target: Phase A translation -> independent verification (bounded
# repair) -> Phase B optimization -> independent verification -> head-to-head
# timing of FRESH Phase-A and Phase-B builds -> final verification of the kept
# solver. Then runs the final benchmark (03_benchmark.sh).
# Restartable: finished steps are skipped via marker files in each target dir.
#
# Usage: controller.sh <perjob | local | allocation job id>
#   perjob: every make / benchmark step is its own <=1 h Slurm job on an AMD
#           EPYC 7H12 + A100 node (test queue, starts within minutes)
#   local:  everything on this machine (della-stellato), NUMA-pinned
#   <id>:   steps of one reserved Slurm allocation
set -Eeuo pipefail
source "$HOME/fomtools_full_pipeline/config.sh"
export FOMTOOLS_REMOTE_JOB_ID="${1:?usage: controller.sh <perjob | local | allocation job id>}"
export PATH="$PIPE/bridge:$PATH"
export FOMTOOLS_CUDA_RUN=1
export TOL_VERIFY="${TOL_VERIFY:?set TOL_VERIFY}"
LOCAL=0; PERJOB=0
[[ "$FOMTOOLS_REMOTE_JOB_ID" == local ]] && LOCAL=1
[[ "$FOMTOOLS_REMOTE_JOB_ID" == perjob ]] && PERJOB=1

exec >> "$ROOT/controller.log" 2>&1
# Only one controller per run directory: a second one exits immediately.
exec 8> "$ROOT/controller.lock"
if ! flock -n 8; then
  echo "[$(date -Is)] another controller holds $ROOT/controller.lock; exiting"
  exit 1
fi
# Cross-machine guard: login connections can land on different backend nodes,
# so a controller elsewhere is invisible to ps here. A live controller refreshes
# $ROOT/controller.heartbeat every 30 s; refuse to start if it is fresh.
HEARTBEAT="$ROOT/controller.heartbeat"
if [[ -f "$HEARTBEAT" ]] && (( $(date +%s) - $(stat -c %Y "$HEARTBEAT") < 120 )); then
  echo "[$(date -Is)] another controller is alive: $(cat "$HEARTBEAT"); exiting"
  exit 1
fi
# Fresh run token: bridge calls from any other (stale) process are refused.
export FOM_RUN_TOKEN="$(date +%s%N)-$$-$RANDOM"
echo "$FOM_RUN_TOKEN" > "$ROOT/RUN_TOKEN"
( while kill -0 $$ 2> /dev/null; do
    echo "host=$(hostname) ip=$(hostname -I | cut -d' ' -f1) pid=$$ token=$FOM_RUN_TOKEN at=$(date -Is)" > "$HEARTBEAT"
    sleep 30
  done ) &
log() { echo "[$(date -Is)] $*"; }
fail() {
  log "FAILED: $*"
  echo "$* ($(date -Is))" > "$ROOT/PIPELINE_FAILED"
  exit 1
}
trap 'fail "controller error at line $LINENO"' ERR

if [[ -f "$ROOT/PIPELINE_FAILED" ]]; then
  log "PIPELINE_FAILED exists; inspect and remove it before restarting"
  exit 1
fi

log "controller start tag=$TAG allocation=$FOMTOOLS_REMOTE_JOB_ID"
stage1_job="$(cat "$ROOT/STAGE1_JOB" 2>/dev/null || cat "$PIPE/STAGE1_JOB" 2>/dev/null || true)"
until [[ -f "$ROOT/SCAFFOLD_OK" ]]; do
  [[ -f "$ROOT/SCAFFOLD_FAILED" ]] && fail "stage 1 failed: $(cat "$ROOT/SCAFFOLD_FAILED")"
  if [[ $LOCAL == 1 || $PERJOB == 1 ]]; then
    if [[ -n $stage1_job && -z "$(squeue -h -j "$stage1_job" -o %T 2>/dev/null)" ]]; then
      sleep 30
      [[ -f "$ROOT/SCAFFOLD_OK" ]] || fail "stage-1 job $stage1_job ended without SCAFFOLD_OK"
    fi
  else
    [[ -n "$(squeue -h -j "$FOMTOOLS_REMOTE_JOB_ID" -o %T 2>/dev/null)" ]] \
      || fail "allocation $FOMTOOLS_REMOTE_JOB_ID left the queue before stage 1 finished"
  fi
  sleep 60
done
if [[ $LOCAL == 1 ]]; then
  gpu="$(nvidia-smi -L)"
  cpu="$(lscpu | sed -n 's/^Model name: *//p')"
  grep -q "A100" <<<"$gpu" && ! grep -q "H100" <<<"$gpu" || fail "local GPU is not an A100: $gpu"
  grep -q "EPYC 7H12" <<<"$cpu" || fail "local CPU is not an AMD EPYC 7H12: $cpu"
  { echo "LOCAL_HOST=$(hostname) ($(hostname -I | cut -d' ' -f1))"; echo "$gpu"; echo "$cpu"
    echo "CPU_NUMA_NODE=$CPU_NUMA_NODE GPU_NUMA_NODE=$GPU_NUMA_NODE"; date -Is; } > "$ROOT/LOCAL_READY"
  log "local mode on $(hostname): $cpu / $gpu"
elif [[ $PERJOB == 1 ]]; then
  log "per-job mode: probing a test-queue step"
  onnode bash -c 'hostname; lscpu | sed -n "s/^Model name: *//p"; nvidia-smi -L' > "$ROOT/PERJOB_PROBE" 2>&1 \
    || fail "per-job probe step failed (see PERJOB_PROBE)"
  # The 7H12 requirement holds unless agent work was explicitly allowed on other A100
  # nodes (FOM_STEP_CONSTRAINT); final benchmarks always check for a 7H12 themselves.
  if [[ "$STEP_CONSTRAINT" == *rome* ]]; then
    grep -q "EPYC 7H12" "$ROOT/PERJOB_PROBE" || fail "per-job step not on an EPYC 7H12 node"
  fi
  grep -q "A100" "$ROOT/PERJOB_PROBE" && ! grep -q "H100" "$ROOT/PERJOB_PROBE" \
    || fail "per-job step GPU is not an A100"
  log "per-job probe ok: $(tr '\n' ' ' < "$ROOT/PERJOB_PROBE")"
else
  until grep -qs "ALLOCATION_JOB_ID=$FOMTOOLS_REMOTE_JOB_ID" "$ROOT/ALLOC_READY"; do
    [[ -n "$(squeue -h -j "$FOMTOOLS_REMOTE_JOB_ID" -o %T 2>/dev/null)" ]] \
      || fail "allocation $FOMTOOLS_REMOTE_JOB_ID ended before it was ready"
    [[ -f "$ROOT/PIPELINE_FAILED" ]] && exit 1
    sleep 60
  done
  log "allocation ready: $(grep HOST= "$ROOT/ALLOC_READY")"
fi

# ---------------------------------------------------------------- helpers --
# FOMTools' own agent flow: built-in instructions, default driver, verify loop.
run_fomtools_agent() {
  local phase=$1 backend=$2 rc=0
  log "fomtools agent phase $phase start"
  "$PYTHON" -u "$PIPE/fomtools_agent.py" "$PWD" "$backend" "$phase" "$TOL_VERIFY" \
    > "fomtools_phase_${phase}.log" 2>&1 || rc=$?
  log "fomtools agent phase $phase end rc=$rc"
  return $rc
}

source "$PIPE/workdir_guard.sh"

check_immutable() {
  sha256sum --quiet -c immutable.sha256 > "immutable_check.log" 2>&1 \
    || fail "$PWD: deterministic files changed (see immutable_check.log)"
}

# Fresh build + verify; returns 0 only on VERIFY OK.
verify_fresh() {
  local label=$1
  check_immutable
  make clean-solver > /dev/null 2>&1 || true
  make verify > "verify_${label}.log" 2>&1 || true
  if grep -q -E "relocation truncated|PC-relative offset overflow" "verify_${label}.log"; then
    fail "$PWD: linker size limit (verify_${label}.log)"
  fi
  grep -q "VERIFY OK" "verify_${label}.log"
}

# Fresh build + timed run of the current source; prints mean_us.
bench_fresh() {
  local label=$1
  check_immutable
  make clean-solver > /dev/null 2>&1 || true
  make bench BENCH_SOLVES=5 > "bench_${label}.log" 2>&1 || return 1
  sed -n 's/.*mean_us=\([0-9.]*\).*/\1/p' "bench_${label}.log" | tail -1
}

# ----------------------------------------------------------------- target --
run_target() {
  local alg=$1 backend=$2
  local d="$ROOT/$alg/$backend" ext=c
  [[ $backend == cuda ]] && ext=cu
  local solver="src/fom_solver.$ext"
  cd "$d"
  if [[ -f FINAL_VERIFIED ]]; then
    log "$alg/$backend already final; skipping"
    return 0
  fi
  log "==== $alg/$backend"

  if [[ $backend == cuda && ! -f CUDA_FIXED_OK && $PERJOB == 1 ]]; then
    # The multi-GB nvcc compile runs in 01b_cuda_objects.sbatch (CPU job).
    local stage1b_job
    stage1b_job="$(cat "$ROOT/STAGE1B_JOB" 2>/dev/null || cat "$PIPE/STAGE1B_JOB" 2>/dev/null || true)"
    log "$alg/$backend waiting for CUDA objects (stage 1b job ${stage1b_job:-?})"
    until [[ -f CUDA_FIXED_OK ]]; do
      if [[ -n $stage1b_job && -z "$(squeue -h -j "$stage1b_job" -o %T 2>/dev/null)" ]]; then
        sleep 30
        [[ -f CUDA_FIXED_OK ]] || fail "$alg/cuda: stage 1b ended without CUDA_FIXED_OK (see $ROOT/01b_cuda_objects.log)"
      fi
      sleep 60
    done
    log "$alg/$backend CUDA objects ready"
  fi
  if [[ $backend == cuda && ! -f CUDA_FIXED_OK ]]; then
    # CPU nodes have no CUDA toolkit, so the multi-GB CUDA objects are built
    # here (on the A100 node) and link-tested with a stub before any agent.
    log "$alg/$backend building CUDA deterministic objects"
    check_immutable
    make -j4 fixed > build_fixed.log 2>&1 || fail "$alg/cuda: fixed-object build failed (build_fixed.log)"
    mkdir -p stub
    cp "$PIPE/stub_solver.c" stub/stub_solver.cu
    make SOLVER_SRC=stub/stub_solver.cu verify_bin bench_bin > stub_link.log 2>&1 \
      || fail "$alg/cuda: stub link failed (stub_link.log)"
    make clean-solver > /dev/null 2>&1 || true
    rm -rf stub
    date -Is > CUDA_FIXED_OK
    log "$alg/$backend CUDA objects built and stub-linked"
  fi

  if [[ ! -f PHASE_A_VERIFIED ]]; then
    # The translation agent must start from FOMTools' scaffold and nothing else.
    # (The log goes outside the target directory: a file created here would itself fail the check.)
    check_clean_workdir > "$ROOT/workdir_guard_${alg}_${backend}.log" 2>&1 \
      || fail "$alg/$backend: $(cat "$ROOT/workdir_guard_${alg}_${backend}.log")"
    # generate_code: phase A must succeed; then an independent fresh-build verify.
    run_fomtools_agent A "$backend" || fail "$alg/$backend: FOMTools phase A did not verify"
    verify_fresh phase_a || fail "$alg/$backend: phase A failed independent verification"
    cp "$solver" "phase_a_solver.$ext"
    date -Is > PHASE_A_VERIFIED
    log "$alg/$backend Phase A verified"
  fi

  if [[ ! -f PHASE_B_DONE ]]; then
    # generate_code: keep phase B only if it succeeded and re-verifies;
    # otherwise fall back to the known-good phase A solver.
    cp "phase_a_solver.$ext" "$solver"
    if run_fomtools_agent B "$backend" && ! cmp -s "phase_a_solver.$ext" "$solver" \
       && verify_fresh phase_b; then
      cp "$solver" "phase_b_solver.$ext"
      echo "verified" > PHASE_B_DONE
    else
      log "$alg/$backend Phase B not kept (failed, unchanged or not verified); Phase A kept"
      cp "phase_a_solver.$ext" "$solver"
      echo "not kept" > PHASE_B_DONE
    fi
  fi

  # Head-to-head on fresh builds of each source (never a leftover binary).
  local a b kept="phase_a" speedup="1"
  if [[ -f "phase_b_solver.$ext" ]] && ! cmp -s "phase_a_solver.$ext" "phase_b_solver.$ext"; then
    check_immutable
    onnode bash -c '
      set -e
      for v in phase_b phase_a; do
        cp "${v}_solver.'"$ext"'" "'"$solver"'"
        make clean-solver > /dev/null 2>&1
        make bench BENCH_SOLVES=5 > "bench_${v}.log" 2>&1
      done
      hostname > h2h_node.txt' > h2h.log 2>&1 || fail "$alg/$backend: head-to-head bench failed (h2h.log)"
    b=$(sed -n 's/.*mean_us=\([0-9.]*\).*/\1/p' bench_phase_b.log | tail -1)
    a=$(sed -n 's/.*mean_us=\([0-9.]*\).*/\1/p' bench_phase_a.log | tail -1)
    [[ -n $a && -n $b ]] || fail "$alg/$backend: could not parse bench output"
    # Same rule as fomtools.codegen.generate: keep Phase B unless it is >2% slower.
    if awk -v a="$a" -v b="$b" 'BEGIN { exit !(b <= 1.02 * a) }'; then
      kept="phase_b"
    fi
    speedup=$(awk -v a="$a" -v b="$b" 'BEGIN { printf "%.3f", a / b }')
  else
    a=$(bench_fresh phase_a) || fail "$alg/$backend: Phase A bench failed"
    b="n/a"
  fi
  cp "${kept}_solver.$ext" "$solver"
  verify_fresh final || fail "$alg/$backend: kept solver failed final verification"
  # Provenance gate: the translated and kept solvers must not copy any solver
  # elsewhere on scratch or in $HOME, and the agent logs must not mention reuse.
  "$PYTHON" "$PIPE/provenance_check.py" "$PWD" > provenance.log 2>&1 \
    || fail "$alg/$backend: provenance gate failed (provenance.log)"
  log "$alg/$backend provenance: $(tail -1 provenance.log)"

  {
    echo "# Optimization report: $alg / $backend ($TAG)"
    echo
    echo "Fresh-build head-to-head on $(cat h2h_node.txt 2>/dev/null || echo 'one node'), mean per solve over 5 solves after 8 warm-ups:"
    echo
    echo "| solver | mean_us |"
    echo "|---|---:|"
    echo "| Phase A | $a |"
    echo "| Phase B | $b |"
    echo
    echo "Kept: **$kept** (Phase-A / Phase-B time = $speedup). Final verification:"
    echo
    echo '```'
    grep "worst max-abs" verify_final.log || true
    echo '```'
    echo
    echo "## Diff (Phase A -> kept)"
    echo
    echo '```diff'
    diff -u "phase_a_solver.$ext" "$solver" || true
    echo '```'
    echo
    echo "## Agent notes (end of the Phase-B transcript)"
    echo
    echo '```'
    tail -n 60 agent_phase_B_fomtools.log 2>/dev/null || true
    echo '```'
  } > OPTIMIZATIONS.md

  {
    echo "kept=$kept"
    echo "phase_a_mean_us=$a"
    echo "phase_b_mean_us=$b"
    echo "solver_sha256=$(sha256sum "$solver" | cut -d' ' -f1)"
    date -Is
  } > FINAL_VERIFIED
  log "$alg/$backend final: kept=$kept a=$a b=$b"
}

for target in $TARGETS; do
  run_target "${target%/*}" "${target#*/}"
done

if [[ "${STOP_BEFORE_BENCHMARK:-0}" == 1 ]]; then
  log "all targets final; stopping before the benchmark (STOP_BEFORE_BENCHMARK=1)"
  exit 0
fi
log "all targets final; starting the final benchmark"
cd "$ROOT"
if [[ $PERJOB == 1 ]]; then
  # prep picks a node; every round is its own <=1 h step pinned to that node.
  if [[ ! -f "$ROOT/benchmark/BENCH_NODE" ]]; then
    STEP_NODE="${BENCH_PREFERRED_NODE:-}" onnode bash "$PIPE/03_benchmark.sh" prep \
      > "$ROOT/benchmark_prep.log" 2>&1 \
      || fail "benchmark prep failed (benchmark_prep.log)"
  fi
  bench_node="$(cat "$ROOT/benchmark/BENCH_NODE")"
  log "benchmark node: $bench_node"
  for r in $(seq 1 "$ROUNDS"); do
    [[ -f "$ROOT/benchmark/round_${r}.txt" ]] && continue
    for attempt in 1 2; do
      STEP_NODE="$bench_node" onnode bash "$PIPE/03_benchmark.sh" round "$r" \
        > "$ROOT/benchmark/round_${r}.step.log" 2>&1 && break
      log "round $r attempt $attempt failed; see benchmark/round_${r}.step.log"
    done
    [[ -f "$ROOT/benchmark/round_${r}.txt" ]] || fail "benchmark round $r failed"
    log "benchmark round $r/$ROUNDS done"
  done
  bash "$PIPE/03_benchmark.sh" finalize > "$ROOT/benchmark_finalize.log" 2>&1 \
    || fail "benchmark finalize failed (benchmark_finalize.log)"
elif [[ $LOCAL == 1 ]]; then
  PIN_LOCAL=1 bash "$PIPE/03_benchmark.sh" \
    || fail "final benchmark failed (see $ROOT/benchmark/benchmark-runner.log)"
else
  srun --overlap --jobid="$FOMTOOLS_REMOTE_JOB_ID" --nodes=1 --ntasks=1 \
    --cpus-per-task="$BENCH_THREADS" --chdir="$ROOT" bash "$PIPE/03_benchmark.sh" \
    || fail "final benchmark failed (see $ROOT/benchmark/benchmark-runner.log)"
fi
test -f "$ROOT/PIPELINE_COMPLETE" || fail "benchmark finished without PIPELINE_COMPLETE"
log "pipeline complete"
