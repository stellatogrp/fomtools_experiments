# Full rectangular sparse-Lasso pipeline (NumPy vs OpenMP vs CUDA)

Generates, verifies, optimizes and benchmarks ISTA and FISTA solvers for the
rect10k40k problem family at a larger size (set in `config.sh`; default
325,000 x 1,300,000, 8 nonzeros/row). Output: `benchmark/summary.csv` and
`summary.md` with NumPy, OpenMP and CUDA medians and the OpenMP and CUDA
speedups over NumPy.

Scripts live in `~/fomtools_full_pipeline`; everything generated goes to
`$ROOT` on `/scratch/gpfs/BSTELLATO/dd7022/fomtools_codegen_runs/`.

## Stages

1. `01_scaffold.sbatch` (CPU node, no agent, ~2-4 h): extracts each IR,
   scaffolds OpenMP and CUDA targets with identical verification data,
   compiles the multi-GB deterministic objects once, and link-tests stub
   binaries. Writes `SCAFFOLD_OK`.
2. `02_allocation.sbatch` (one A100 + 16 cores on an AMD EPYC 7H12 node, the
   paper's hardware; up to 48 h) holds the
   node; `controller.sh` runs on the login node in tmux (the agent needs
   network access) and forwards every `make` there via `bridge/make`. Per
   target: translation, verify, code optimization, verify, fresh-build head-to-head, final
   verify. Then it runs stage 3 on the same allocation.
3. `03_benchmark.sh`: rebuilds every kept solver from source, re-verifies,
   checksums, and times `ROUNDS` interleaved rounds. `03_benchmark.sbatch`
   reruns it alone.

## Launch (from a Della login node)

```bash
cd ~/fomtools_full_pipeline
s1=$(sbatch --parsable 01_scaffold.sbatch)
s2=$(sbatch --parsable --dependency=afterok:$s1 02_allocation.sbatch)
tmux new-session -d -s fom_full "bash $PWD/controller.sh $s2"
```

Progress: `tail -f $ROOT/01_scaffold.log`, `tail -f $ROOT/controller.log`,
`ls $ROOT/*/*/` (markers `PHASE_A_VERIFIED`, `PHASE_B_DONE`,
`FINAL_VERIFIED`). Stop: `touch $ROOT/ALLOC_RELEASE` (releases the A100) and
`tmux kill-session -t fom_full`.

Restart after a failure: fix the cause, `rm $ROOT/PIPELINE_FAILED`, start a
new allocation and controller; finished steps are skipped.

## Fixes relative to the rect10k40k run

- OpenMP is always built with `-fopenmp`; threads pinned
  (`OMP_PLACES=cores`, `OMP_PROC_BIND=close`).
- No stale binaries: every verify/bench/benchmark starts with
  `make clean-solver`; the translation/code-optimization gate times fresh builds of each source.
- CUDA builds native `sm_80` only (no PTX JIT of the matrix data).
- Generation, optimization and the final benchmark run on the same node, an
  AMD EPYC 7H12 + A100 node (della-i14g), checked at start (matches the paper).
- `-mcmodel=medium` and `ulimit -s unlimited` for the ~1-2 GB binaries and
  the large stack workspace.
- Deterministic files are checksummed; the agent may edit only the solver.
