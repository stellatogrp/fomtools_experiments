# Shared settings for the full rectangular sparse-Lasso ISTA/FISTA pipeline.
# Sourced by every stage; edit the problem size here and nowhere else.

# Problem: same construction as rect10k40k (8 nonzeros/row, n = 4m, so the
# 8m edge slots are exactly two seeded column permutations), seed 0,
# lambda 0.02, 200 iterations, float64.
export M=325000
export N=1300000
export K=8
export TAG="m${M}_n${N}_k${K}"

# Code and tools on Della. REPO is the checkout used for every rect study
# (paper_{ista,fista}_sparse20k.py, bench_python_sparse.py).
export REPO="$HOME/fomtools_rect10k40k_k32"
export PYTHON="$HOME/fomtools/.venv/bin/python"
export CLAUDE_BIN="$HOME/.local/bin/claude"
export PIPE="$HOME/fomtools_full_pipeline"
export CUDA_MODULE="cudatoolkit/12.9"

# All generated sources, binaries and logs live on scratch (multi-GB).
export ROOT="/scratch/gpfs/BSTELLATO/dd7022/fomtools_codegen_runs/full_${TAG}"

export PYTHONPATH="$REPO"
export FOMTOOLS_EXPERIMENT_M="$M"
export FOMTOOLS_EXPERIMENT_N="$N"
export FOMTOOLS_EXPERIMENT_A_NNZ_PER_ROW="$K"

# Threading for every OpenMP build/run and the NumPy reference. The final
# benchmark and all agent benchmarking use the same allocation shape.
export BENCH_THREADS=16
export OMP_NUM_THREADS="$BENCH_THREADS"
export OMP_PLACES=cores
export OMP_PROC_BIND=close

# Timed generated solves per benchmark call (after the 8 built-in warm-ups),
# NumPy repetitions per round (after 1 warm-up), and interleaved rounds.
export GEN_SOLVES=10
export NUMPY_REPS=3
export ROUNDS=7

# Local mode (controller.sh local): everything runs on della-stellato, the
# group's 2x AMD EPYC 7H12 + A100 node. Each run is pinned to one 16-core NUMA
# domain: OpenMP and NumPy to a quiet domain, CUDA to the GPU's domain
# (nvidia-smi topo -m: GPU0 affinity cores 32-47 = NUMA node 2).
export CPU_NUMA_NODE=3
export GPU_NUMA_NODE=2
export CUDA_HOME=/usr/local/cuda-12.9
if [[ -d "$CUDA_HOME/bin" ]]; then
  export PATH="$CUDA_HOME/bin:$PATH"
  export LD_LIBRARY_PATH="$CUDA_HOME/lib64${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
fi

# Copy of the CUDA 12.9 compiler on scratch: CPU compute nodes have no CUDA
# toolkit, and nvcc needs no GPU (used by 01b_cuda_objects.sbatch).
export SCRATCH_CUDA_HOME=/scratch/gpfs/BSTELLATO/dd7022/toolchains/cuda-12.9

# Per-job mode (controller.sh perjob): every make / benchmark step is its own
# <=1 h Slurm job on an AMD EPYC 7H12 + A100 node (the paper's hardware);
# <=1 h GPU jobs go to the test queue and start within minutes. Never H100.
export STEP_TIME=00:59:00
export STEP_MEM=64G
export STEP_CONSTRAINT="amd&rome&a100"
export H100_EXCLUDE="della-h19g[1-4],della-h20g[1-5],della-h21g[1-4],della-j11g1,della-j12g[1-3],della-j13g[1-3],della-j14g[1-3],della-j15g[1-3],della-j16g[1-3],della-j17g[1-2],della-k11g[1-3],della-k12g[1-3],della-k13g[1-3],della-k14g[1-3],della-k15g[1-3],della-k16g[1-3],della-k17g[1-3],della-k18g[1-3]"

# Order in which the controller generates targets. OpenMP first, so the CUDA
# objects (01b_cuda_objects.sbatch) can finish compiling meanwhile.
export TARGETS="ista/openmp ista/cuda fista/openmp fista/cuda"
