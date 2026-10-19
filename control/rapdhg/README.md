# rAPDHG for the quadcopter controller

Restarted accelerated primal-dual hybrid gradient (Lu and Yang, arXiv:2311.07710,
Algorithm 1) on the condensed quadcopter program. Its iteration is matrix-vector
products and vector arithmetic, with no linear system solve.

## Formulation

The method solves `minimize (1/2) z'Qz + q'z subject to G z <= h(x)`. The condensed
program has the thrust box `lo <= z <= hi` and a 30-degree bound on the two tilt
angles at the end of every input block, `-e <= F x + C z <= e`, stacked one-sided:

    G = [ I ; -I ; C ; -C ],    h(x) = [ hi ; -lo ; e - F x ; e + F x ]

The right-hand side moves with the measured state `x`, so the generated solver
takes two parameters per sampling instant, `q` and `h`. The plant, cost,
reference, and move blocking come from `../common/condensed_qp.py`.

## Layout

```
rapdhg/
├── verify/
│   └── cluster_run.py             the horizon-5 instance for fomexp.run (R12, R6)
└── codegen/                       the generated controller and its flights; see its README
    ├── mpc.py                     the program in one-sided inequality form
    ├── rapdhg.py, recording.py    the iteration, and its recording for code generation
    ├── check_against_fomtools.py  the controller is the recorded algorithm
    ├── generate.py                both agent phases
    ├── run_implementations.py     NumPy, translated C, optimized C at equal budgets
    └── run_figures.py             the paper's flight figure
```

## Running

Verification runs from the repository root (see `../README.md`). The flight
study runs from this directory:

```
python -m codegen.check_against_fomtools
FOMTOOLS_CODEGEN_AGENT=1 python -m codegen.generate --max-iters 512
python -m codegen.run_implementations
python -m codegen.run_figures --reference figure8 --iterations 3 13 275 --no-gif
```
