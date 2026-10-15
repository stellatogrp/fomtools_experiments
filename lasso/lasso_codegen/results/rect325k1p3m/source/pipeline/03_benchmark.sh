#!/bin/bash
# Stage 3: final interleaved benchmark on an AMD EPYC 7H12 + A100 node.
#
#   03_benchmark.sh            all stages in one go (allocation / local mode)
#   03_benchmark.sh prep       rebuild every kept solver from its verified
#                              source, re-verify, checksum; records the node
#   03_benchmark.sh round N    one interleaved round (NumPy, OpenMP, CUDA for
#                              ISTA and FISTA) into benchmark/round_N.txt
#   03_benchmark.sh finalize   concatenate rounds, write summary.csv/.md
#
# Per-job mode runs prep, then each round as its own <=1 h job pinned to the
# prep node, then finalize on the login node.
set -euo pipefail
source "$HOME/fomtools_full_pipeline/config.sh"
command -v nvcc > /dev/null || module load "$CUDA_MODULE" 2> /dev/null || true
ulimit -s unlimited
OUT="$ROOT/benchmark"
mkdir -p "$OUT"

check_hardware() {
  local gpu cpu
  gpu="$(nvidia-smi -L)"
  cpu="$(lscpu | sed -n 's/^Model name: *//p')"
  if ! grep -q "A100" <<<"$gpu" || grep -q "H100" <<<"$gpu"; then
    echo "not on an A100 node: $gpu"; exit 1
  fi
  if ! grep -q "EPYC 7H12" <<<"$cpu"; then
    echo "not on an AMD EPYC 7H12 node: $cpu"; exit 1
  fi
}

solver_of() { [[ $1 == */cuda ]] && echo src/fom_solver.cu || echo src/fom_solver.c; }

prep() {
  check_hardware
  {
    echo "RUN_DATE=$(date -Is)"
    echo "SLURM_JOB_ID=${SLURM_JOB_ID:-}"
    echo "HOST=$(hostname)"
    nvidia-smi -L
    lscpu | grep -E 'Model name|Socket|Core|Thread|CPU\(s\)|NUMA node'
    echo "SHAPE=${M}x${N} nnz_per_row=$K iterations=200 lambda=0.02 seed=0 dtype=float64"
    echo "REPO_COMMIT=$(git -C "$REPO" rev-parse HEAD)"
    echo "OMP_NUM_THREADS=$OMP_NUM_THREADS OMP_PLACES=$OMP_PLACES OMP_PROC_BIND=$OMP_PROC_BIND"
    echo "ROUNDS=$ROUNDS GEN_SOLVES=$GEN_SOLVES NUMPY_REPS=$NUMPY_REPS"
    echo "MODE=${FOMTOOLS_REMOTE_JOB_ID:-} PIN_LOCAL=${PIN_LOCAL:-0}"
    gcc --version | head -1
    nvcc --version | tail -1
    "$PYTHON" --version
    "$PYTHON" -c "import numpy, scipy; print('numpy', numpy.__version__, 'scipy', scipy.__version__)"
  } > "$OUT/benchmark_metadata.txt"

  : > "$OUT/checksums.txt"
  local target d label solver expected actual
  for target in $TARGETS; do
    d="$ROOT/$target"
    label="${target/\//_}"
    solver="$(solver_of "$target")"
    test -f "$d/FINAL_VERIFIED"
    (cd "$d" && sha256sum --quiet -c immutable.sha256)
    expected="$(sed -n 's/^solver_sha256=//p' "$d/FINAL_VERIFIED")"
    actual="$(sha256sum "$d/$solver" | cut -d' ' -f1)"
    if [[ "$expected" != "$actual" ]]; then
      echo "$target: solver source differs from the verified one"; exit 1
    fi
    /usr/bin/make -C "$d" clean-solver
    /usr/bin/make -C "$d" verify 2>&1 | tee "$OUT/verify_${label}.log"
    grep -q "VERIFY OK" "$OUT/verify_${label}.log"
    /usr/bin/make -C "$d" bench_bin
    (cd "$d" && sha256sum "$solver" bench_bin) | sed "s|^|$target |" >> "$OUT/checksums.txt"
  done
  hostname > "$OUT/BENCH_NODE"
  echo "PREP_OK $(date -Is) on $(hostname)"
}

round() {
  local r=$1
  check_hardware
  # Same binaries as prep, on the same node.
  [[ "$(hostname)" == "$(cat "$OUT/BENCH_NODE")" ]] || { echo "round $r not on the prep node"; exit 1; }
  local target d
  for target in $TARGETS; do
    d="$ROOT/$target"
    grep "^$target " "$OUT/checksums.txt" | cut -d' ' -f2- | (cd "$d" && sha256sum --quiet -c -)
  done

  # Local mode (shared machine): pin NumPy/OpenMP to a quiet NUMA domain and
  # CUDA to the GPU's domain.
  local cpu_pin=() gpu_pin=() alg upper
  if [[ "${PIN_LOCAL:-0}" == 1 ]]; then
    cpu_pin=(numactl --cpunodebind="$CPU_NUMA_NODE" --preferred="$CPU_NUMA_NODE")
    gpu_pin=(numactl --cpunodebind="$GPU_NUMA_NODE" --preferred="$GPU_NUMA_NODE")
  fi
  {
    echo "ROUND $r"
    echo "MACHINE_STATE $(date -Is) host=$(hostname) job=${SLURM_JOB_ID:-} load=[$(cut -d' ' -f1-3 /proc/loadavg)]" \
      "gpu=[$(nvidia-smi --query-gpu=utilization.gpu,memory.used --format=csv,noheader)]"
    for alg in ista fista; do
      upper="${alg^^}"
      "${cpu_pin[@]}" "$PYTHON" "$REPO/bench_python_sparse.py" "$alg" --reps "$NUMPY_REPS"
      echo "CPU_OPENMP_$upper"
      (cd "$ROOT/$alg/openmp" && "${cpu_pin[@]}" ./bench_bin "$GEN_SOLVES")
      echo "CUDA_$upper"
      (cd "$ROOT/$alg/cuda" && "${gpu_pin[@]}" ./bench_bin "$GEN_SOLVES")
    done
  } > "$OUT/round_${r}.partial" 2>&1
  mv "$OUT/round_${r}.partial" "$OUT/round_${r}.txt"
  cat "$OUT/round_${r}.txt"
}

finalize() {
  local log="$OUT/production_interleaved_${ROUNDS}rounds.txt" r target
  : > "$log"
  for r in $(seq 1 "$ROUNDS"); do
    cat "$OUT/round_${r}.txt" >> "$log"
  done
  "$PYTHON" "$PIPE/summarize.py" "$log" --rounds "$ROUNDS" --out-dir "$OUT"
  for target in $TARGETS; do
    cp "$ROOT/$target/OPTIMIZATIONS.md" "$OUT/OPTIMIZATIONS_${target/\//_}.md"
  done
  date -Is > "$ROOT/PIPELINE_COMPLETE"
  echo "BENCHMARK_OK $(date -Is)"
}

case "${1:-all}" in
  prep) prep ;;
  round) round "${2:?round number}" ;;
  finalize) finalize ;;
  all)
    exec > >(tee -a "$OUT/benchmark-runner.log") 2>&1
    echo "BENCHMARK_START $(date -Is)"
    prep
    for r in $(seq 1 "$ROUNDS"); do round "$r"; done
    finalize
    ;;
  *) echo "usage: 03_benchmark.sh [all|prep|round N|finalize]"; exit 2 ;;
esac
