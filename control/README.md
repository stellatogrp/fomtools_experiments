# Quadcopter control experiments

Model predictive control of a quadcopter linearized around hover, solved with
restarted accelerated PDHG (rAPDHG). The paper reports three results from this
folder:

| result | where |
|---|---|
| worst-case KKT residual of rAPDHG, restart periods 12 and 6, horizon 5 | `rapdhg/verify/results/run_002` |
| generated C solvers for that verified instance | `../codegen_timings/rapdhg/` |
| closed-loop figure-eight flight at 40 us per sampling instant | `rapdhg/codegen/` |

```
control/
├── common/     the plant, the simulator, the plots, the code-generation pipeline
├── rapdhg/     rAPDHG: verify/ (the MIP instance) and codegen/ (the flight)
└── osqp/       ADMM study on the same plant (not in the paper)
```

The verification runs through the shared driver from the repository root:

```
uv run python -m fomexp.run experiment=rapdhg experiment.problem.horizon=5
```

The flight study runs as a module from `rapdhg/`; see `rapdhg/README.md`.
