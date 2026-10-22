# Generated solver: `network_ruiz_momentum` (embedded C)

OSQP-codegen-style embedded solver for a verified first-order method.
No dynamic memory allocation; division-free.

## Build, verify, benchmark, profile
```
make verify    # build and check against the reference oracle
make bench     # time full solves (prints a BENCH line)
make profile   # per-region timing breakdown (prints PROF lines)
make lib       # shared library for the Python wrapper
```
Wrap hot regions of `fom_solve` with `FOM_PROF_BEGIN(name)` /
`FOM_PROF_END(name)` (from `fom_profile.h`) to get a profiling
breakdown; the macros are no-ops in the normal (verify/lib) build.

## ABI
See `include/fom_solver.h`. Typical usage:
```c
FomWorkspace work; FomSettings s;
fom_set_default_settings(&s);
fom_setup(&work, &s);
fom_update_params(&work, &params);
fom_solve(&work);
/* work.solution.x holds the solution */
```

`src/fom_solver.c` implements the iteration; all numeric data lives in
`src/fom_data.c`. The algorithm is specified in `SPEC.md`.
