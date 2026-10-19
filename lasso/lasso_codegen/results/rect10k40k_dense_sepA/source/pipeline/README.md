# Pipeline for the dense sepA run (10k by 40k, 260 nonzeros per row)

The Slurm pipeline that generated, verified, and benchmarked this study's solvers.
Settings come from `config.sh` and these environment variables:
`FOM_M=10000 FOM_N=40000 FOM_K=260 FOM_VARIANT=sepA FOM_ISTA_TOL=1e-5 FOM_FISTA_TOL=1e-5 FOM_STEP_CONSTRAINT=a100`.
The problem files are `../paper_*_sparse20k.py`.

1. `01_scaffold.sbatch` and `01b_cuda_objects.sbatch` run extraction, the scaffolds, and
   the fixed objects. `baseline_gate.sh` checks the hand-written baselines
   (`../baselines.py`) against the FOMTools reference execution.
2. `gate_launch.sh` starts `controller_builtin.sh`, which runs translation and code
   optimization through `fomtools_agent.py` and verifies each solver after each phase.
3. `bench_step.sbatch all` runs the seven interleaved rounds with `BASELINES=1` and
   writes `summary.csv` (`summarize.py`).
