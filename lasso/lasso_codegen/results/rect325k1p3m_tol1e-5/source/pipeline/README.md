# Full rectangular sparse-Lasso pipeline (NumPy vs OpenMP vs CUDA)

Generates, verifies, optimizes and benchmarks ISTA and FISTA solvers for the
rect10k40k problem family at a larger size (set in `config.sh`; this run:
`FOM_M=325000 FOM_N=1300000`, 8 nonzeros/row). Output:
`benchmark/summary.csv` and `summary.md` with NumPy, OpenMP and CUDA medians
and the OpenMP and CUDA speedups over NumPy.

Scripts live in `~/fomtools_full_pipeline`; everything generated goes to
`$ROOT` on `/scratch/gpfs/BSTELLATO/dd7022/fomtools_codegen_runs/`.

## Stages

1. `01_scaffold.sbatch` (CPU node, no agent): extracts each IR, scaffolds
   OpenMP and CUDA targets with identical verification data, compiles the
   multi-GB deterministic OpenMP objects once, and link-tests stub binaries.
   `01b_cuda_objects.sbatch` compiles the deterministic CUDA objects.
2. `controller_builtin.sh perjob` runs on the login node (the agent needs
   network access). Per target it runs translation and code optimization
   through `fomtools_agent.py`, which builds the agent task as FOMTools'
   `generate_code` does: the built-in `phase_a_instructions` /
   `phase_b_instructions`, the target's verify command, `make bench` and 60
   turns for code optimization, and `make_agent_driver()`. Every `make` the
   controller or the agent runs goes through `bridge/make` to `bridge/onnode`
   and runs as a short Slurm job on an AMD EPYC 7H12 + A100 node
   (`bridge/holder` reuses one <=1 h test-queue job). After each phase:
   independent verification; then a fresh-build head-to-head of the two
   sources in one job, keeping code optimization if it is no more than 2%
   slower, and a final verification.
3. `03_benchmark.sh`: `prep` rebuilds every kept solver from source,
   re-verifies, and checksums; `round N` times one interleaved round as its
   own job pinned to the benchmark node; `finalize` writes the summary.

## Launch (from a Della login node)

```bash
cd ~/fomtools_full_pipeline
export FOM_M=325000 FOM_N=1300000 FOM_VARIANT=builtin
s1=$(sbatch --parsable 01_scaffold.sbatch)
sbatch --dependency=afterok:$s1 01b_cuda_objects.sbatch
# after both finish:
systemd-run --user --unit=fom-builtin --setenv=FOM_M=$FOM_M --setenv=FOM_N=$FOM_N \
  --setenv=FOM_VARIANT=$FOM_VARIANT --setenv=TOL_VERIFY=1e-5 \
  bash $PWD/controller_builtin.sh perjob
```

Progress: `tail -f $ROOT/controller.log`, `ls $ROOT/*/*/` (markers
`PHASE_A_VERIFIED`, `PHASE_B_DONE`, `FINAL_VERIFIED`). Only one controller
runs per `$ROOT`: it holds `controller.lock`, refreshes
`controller.heartbeat`, and the bridge refuses calls without the current
`RUN_TOKEN`. Restart after a failure: fix the cause, `rm $ROOT/PIPELINE_FAILED`,
and start the controller again; finished steps are skipped.

## Fixes relative to the rect10k40k run

- OpenMP is always built with `-fopenmp`; threads pinned
  (`OMP_PLACES=cores`, `OMP_PROC_BIND=close`).
- No stale binaries: every verify/bench/benchmark starts with
  `make clean-solver`; the translation/code-optimization gate times fresh builds of each source.
- CUDA builds native `sm_80` only (no PTX JIT of the matrix data).
- Optimization and the final benchmark run on AMD EPYC 7H12 + A100 nodes,
  checked at start (matches the paper); every benchmark round runs on one node.
- `-mcmodel=medium` and `ulimit -s unlimited` for the ~1-2 GB binaries and
  the large stack workspace.
- Deterministic files are checksummed; the agent may edit only the solver.
