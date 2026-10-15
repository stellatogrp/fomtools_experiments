#!/bin/bash
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
LOCAL=0; PERJOB=0
[[ "$FOMTOOLS_REMOTE_JOB_ID" == local ]] && LOCAL=1
[[ "$FOMTOOLS_REMOTE_JOB_ID" == perjob ]] && PERJOB=1

exec >> "$ROOT/controller.log" 2>&1
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
stage1_job="$(cat "$PIPE/STAGE1_JOB" 2>/dev/null || true)"
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
  grep -q "EPYC 7H12" "$ROOT/PERJOB_PROBE" || fail "per-job step not on an EPYC 7H12 node"
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

# ---------------------------------------------------------------- prompts --
rules() {
  local ext=$1
  cat <<EOF
RULES (apply throughout):
- Edit ONLY src/fom_solver.$ext. Do not edit the Makefile, include/, test/,
  src/fom_ops.*, src/fom_data.c, src/fom_linsolve.*, src/fom_profile.c,
  spec.json or data.npz. They are checksummed; any change aborts the run.
- NEVER open, cat, head, tail, grep or Read src/fom_ops.*, src/fom_data.c,
  test/verify_data.h or data.npz. They are multi-GB numeric literals and will
  overflow your context. Helper names, shapes and nnz are in
  include/fom_ops.h, MATRIX_STATS.md, SPEC.md and spec.json.
- Build and run ONLY through make: \`make verify\`, \`make bench\`,
  \`make profile\` (optionally BENCH_SOLVES=n). \`make\` runs on an AMD EPYC
  7H12 + A100 compute node with ${BENCH_THREADS} cores, as a short Slurm job: one
  call can take several minutes including queueing, so give it a long timeout
  or run it in the background and wait for it. Never run ./verify_bin,
  ./bench_bin, ./profile_bin, cc, gcc or nvcc yourself: they would run
  unpinned on a machine shared with other users.
- The problem is large (m=$M, n=$N, 200 iterations). A verify runs 8 full
  reference solves and links a ~1-2 GB binary; budget your iterations.
- FomWorkspace is a large struct; never put additional full-length arrays on
  the stack.
- Keep float64, the fixed 200 iterations, the algorithm and the problem.
  Every fom_solve must start from FOM_STATE_INIT (verify runs 8 samples back
  to back).
EOF
}

prompt_phase_a() {
  local backend=$1 ext=$2
  if [[ $backend == openmp ]]; then
    cat <<EOF
Implement src/fom_solver.c faithfully for the iteration described in SPEC.md.
Read SPEC.md, MATRIX_STATS.md, include/fom_solver.h, include/fom_workspace.h,
include/fom_data.h, include/fom_ops.h, include/fom_profile.h,
test/verify_main.c and test/bench_main.c first. Compute every matrix product
with the supplied fom_apply_* helpers (out += M @ x; zero the output first);
no dense matvecs and no hard-coded numeric data. The helpers are already
OpenMP-parallel; parallelize your own elementwise loops with
#pragma omp parallel for schedule(static) (the build uses -fopenmp). No
dynamic allocation and no VLAs. Run make verify and iterate until it prints
VERIFY OK, then stop with a short summary.

$(rules "$ext")
EOF
  else
    cat <<EOF
Implement src/fom_solver.cu faithfully for the iteration described in SPEC.md.
Read SPEC.md, MATRIX_STATS.md, include/fom_solver.h, include/fom_workspace.h,
include/fom_data.h, include/fom_ops.h, include/fom_profile.h,
test/verify_main.c and test/bench_main.c first. Every fom_apply_* helper takes
DEVICE pointers and accumulates out += M @ x (zero the output first, e.g.
cudaMemset). Allocate device buffers once (cudaMalloc in fom_setup is
allowed) and keep the iteration state resident on the GPU for all 200
iterations; copy parameters in, results out. Use float64. fom_solve must
synchronize all GPU work, including the final device-to-host copy, before it
returns, so wall-clock timing covers completed work. Include fom_profile.h
normally (no extern "C" wrapper); make profile compiles the profiler as CUDA
C++. Use FOM_PROF_BEGIN/END around the main regions with
cudaDeviceSynchronize() inside #ifdef FOM_PROFILE only. Run make verify and
iterate until it prints VERIFY OK, then stop with a short summary.

$(rules "$ext")
EOF
  fi
}

prompt_phase_b() {
  local backend=$1 ext=$2
  local device="${BENCH_THREADS} OpenMP threads"
  [[ $backend == cuda ]] && device="one A100 GPU"
  cat <<EOF
src/fom_solver.$ext has passed independent verification. Optimize it for the
wall-clock time of fom_solve on this node ($device). Work in a loop: run
make profile, find the hottest region, make ONE change, run make verify, then
make bench; keep the change only if it prints VERIFY OK and mean_us improves,
otherwise revert it. Use only exact structure documented in SPEC.md,
spec.json and MATRIX_STATS.md (identity or copy blocks, iteration-invariant
parameter terms, fixed max-iteration termination, exact algebraic relations
between the supplied operators); never approximate. Verification tolerance is
1e-6 absolute, so reordered floating-point work must still verify. Keep the
FOM_PROF_BEGIN/END markers around the main regions. Finish with make verify
and make bench, and end your reply with: initial mean_us, final mean_us, the
final profile regions, and a bullet list of the optimizations you kept.

$(rules "$ext")
EOF
}

prompt_repair() {
  local ext=$1 logfile=$2
  cat <<EOF
src/fom_solver.$ext failed independent verification. Read the last 80 lines
of $logfile (do not print the whole file), find the cause and fix ONLY
src/fom_solver.$ext without changing the algorithm, problem, iteration count
or precision. Run make verify and stop after it prints VERIFY OK.

$(rules "$ext")
EOF
}

# ---------------------------------------------------------------- helpers --
run_agent() {
  local name=$1 seconds=$2 prompt=$3 rc=0
  log "agent $name start"
  timeout "$seconds" "$CLAUDE_BIN" -p "$prompt" --add-dir "$PWD" \
    --allowedTools Bash Edit Write Read Glob Grep \
    --permission-mode bypassPermissions --output-format text \
    > "agent_${name}.log" 2>&1 || rc=$?
  log "agent $name end rc=$rc"
}

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

verify_or_repair() {
  local phase=$1 ext=$2 attempt
  for attempt in 1 2 3; do
    if verify_fresh "${phase}_${attempt}"; then
      return 0
    fi
    if [[ $attempt -lt 3 ]]; then
      run_agent "repair_${phase}_${attempt}" 5400 \
        "$(prompt_repair "$ext" "verify_${phase}_${attempt}.log")"
    fi
  done
  return 1
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
    stage1b_job="$(cat "$PIPE/STAGE1B_JOB" 2>/dev/null || true)"
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
    [[ -f $solver ]] || run_agent phase_a_translate 10800 "$(prompt_phase_a "$backend" "$ext")"
    [[ -f $solver ]] || fail "$alg/$backend: Phase A produced no $solver"
    verify_or_repair phase_a "$ext" || fail "$alg/$backend: Phase A never verified"
    cp "$solver" "phase_a_solver.$ext"
    date -Is > PHASE_A_VERIFIED
    log "$alg/$backend Phase A verified"
  fi

  if [[ ! -f PHASE_B_DONE ]]; then
    cp "phase_a_solver.$ext" "$solver"
    run_agent phase_b_optimize 14400 "$(prompt_phase_b "$backend" "$ext")"
    if verify_or_repair phase_b "$ext"; then
      cp "$solver" "phase_b_solver.$ext"
      echo "verified" > PHASE_B_DONE
    else
      log "$alg/$backend Phase B never verified; Phase A will be kept"
      echo "not verified" > PHASE_B_DONE
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
    tail -n 60 agent_phase_b_optimize.log 2>/dev/null || true
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

log "all targets final; starting the final benchmark"
cd "$ROOT"
if [[ $PERJOB == 1 ]]; then
  # prep picks a node; every round is its own <=1 h step pinned to that node.
  if [[ ! -f "$ROOT/benchmark/BENCH_NODE" ]]; then
    onnode bash "$PIPE/03_benchmark.sh" prep > "$ROOT/benchmark_prep.log" 2>&1 \
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
