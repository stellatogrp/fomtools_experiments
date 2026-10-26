# rAPDHG verification (paper Sec. 6.4)

The horizon-5 quadcopter MPC of `../README.md`, verified for rAPDHG with restart
periods $K_\mathrm{r} = 12$ (`R12`) and $6$ (`R6`) for $K = 1, \dots, 75$. The
defaults in `conf/experiment/rapdhg.yaml` are the paper's instance and horizon (the
paper's run is `results/run_002`, the archived one `results/run_001`), so from the repository root:

```sh
uv run python -m fomexp.run experiment=rapdhg       # R12 and R6 into a new results/run_NNN
uv run python -m fomexp.collect rapdhg run_NNN      # summary.csv, wide.csv, summary.json
```

`cluster_run.py` is the adapter `fomexp.run` loads; `verify.py` holds the iteration,
the problem data and an independent NumPy reference that the simulator is checked
against.
