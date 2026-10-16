#!/bin/bash
# Stage 1 (CPU only, no agent): extract both algorithms, scaffold OpenMP and
# CUDA targets with shared verification data, compile the OpenMP deterministic
# objects once, and link-test a full-size OpenMP stub binary. CPU nodes have no
# CUDA toolkit, so the controller builds and link-tests the CUDA objects on the
# A100 node before the first CUDA agent starts.
#SBATCH --job-name=fom-full-scaffold
#SBATCH --account=bstellato
#SBATCH --partition=cpu
#SBATCH --constraint=genoa
# Never run on H100 nodes (redundant with --partition=cpu, kept deliberately).
#SBATCH --exclude=della-h19g[1-4],della-h20g[1-5],della-h21g[1-4],della-j11g1,della-j12g[1-3],della-j13g[1-3],della-j14g[1-3],della-j15g[1-3],della-j16g[1-3],della-j17g[1-2],della-k11g[1-3],della-k12g[1-3],della-k13g[1-3],della-k14g[1-3],della-k15g[1-3],della-k16g[1-3],della-k17g[1-3],della-k18g[1-3]
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=8
#SBATCH --mem=320G
#SBATCH --time=12:00:00
#SBATCH --output=/scratch/gpfs/BSTELLATO/dd7022/fomtools_codegen_runs/slurm-%x-%j.out

set -euo pipefail
source "$HOME/fomtools_full_pipeline/config.sh"
ulimit -s unlimited

mkdir -p "$ROOT"
exec > >(tee -a "$ROOT/01_scaffold.log") 2>&1
rm -f "$ROOT/SCAFFOLD_OK" "$ROOT/SCAFFOLD_FAILED"
trap 'rc=$?; [[ $rc -ne 0 ]] && echo "stage 1 failed rc=$rc at $(date -Is)" > "$ROOT/SCAFFOLD_FAILED"' EXIT

echo "SCAFFOLD_START $(date -Is) tag=$TAG job=$SLURM_JOB_ID"
{
  echo "RUN_DATE=$(date -Is)"
  echo "SLURM_JOB_ID=$SLURM_JOB_ID"
  echo "HOST=$(hostname)"
  echo "SHAPE=${M}x${N} nnz_per_row=$K iterations=$ITERATIONS lambda=0.02 seed=0 dtype=float64"
  echo "REPO_COMMIT=$(git -C "$REPO" rev-parse HEAD)"
  lscpu | grep -E 'Model name|Socket|Core|Thread|CPU\(s\)'
  gcc --version | head -1
  "$PYTHON" --version
} > "$ROOT/scaffold_metadata.txt"

for algorithm in $ALGORITHMS; do
  echo "== $algorithm: extract + scaffold $(date -Is)"
  /usr/bin/time -v "$PYTHON" -u "$PIPE/scaffold_pair.py" "$algorithm" 2>&1 \
    | tee "$ROOT/scaffold_${algorithm}.log"
  grep -q SCAFFOLD_PAIR_OK "$ROOT/scaffold_${algorithm}.log"

  # Compile the OpenMP deterministic objects once (the slow multi-GB
  # translation units; agents only recompile the solver), then link and run a
  # full-size stub binary: proves the stack limit and static-data layout.
  d="$ROOT/$algorithm/openmp"
  if /usr/bin/time -v make -C "$d" -j4 fixed > "$d/build_fixed.log" 2>&1; then
    echo "FIXED_OK $algorithm/openmp $(date -Is)"
  else
    echo "FIXED_FAILED $algorithm/openmp; see $d/build_fixed.log"
    exit 1
  fi
  grep -E "Elapsed|Maximum resident" "$d/build_fixed.log"
  mkdir -p "$d/stub"
  cp "$PIPE/stub_solver.c" "$d/stub/stub_solver.c"
  make -C "$d" SOLVER_SRC=stub/stub_solver.c verify_bin bench_bin 2>&1 | tee "$d/stub_link.log"
  (cd "$d" && ./bench_bin 1) | tee -a "$d/stub_link.log"
  ls -l "$d/verify_bin" "$d/bench_bin" | tee -a "$d/stub_link.log"
  make -C "$d" clean-solver
  rm -rf "$d/stub"
  echo "STUB_LINK_OK $algorithm/openmp"

  # Everything except the solver source is immutable from here on.
  for backend in openmp cuda; do
    d="$ROOT/$algorithm/$backend"
    (cd "$d" && find include test -type f -print0 | sort -z | xargs -0 sha256sum \
      && sha256sum Makefile spec.json src/fom_ops.* src/fom_data.c src/fom_linsolve.* \
         src/fom_profile.c) > "$d/immutable.sha256"
  done
  du -sh "$ROOT/$algorithm"/*
done

date -Is > "$ROOT/SCAFFOLD_OK"
echo "SCAFFOLD_OK $(date -Is)"
