# Production solver

`fom_solver.c` is the marker-free production build of `src/fom_solver.c`: the
`FOM_PROF_*` profiling markers and the `fom_profile.h` include have been
stripped. Behaviour and performance are identical to the instrumented
source (the markers are no-ops outside `make profile`); this copy is a
clean, dependency-free drop-in for embedding.

Build it with `make release` (from the parent directory), or compile
`release/fom_solver.c` together with `src/fom_data.c` against `include/`.
