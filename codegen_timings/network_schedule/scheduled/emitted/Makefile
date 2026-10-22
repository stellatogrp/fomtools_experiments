CC ?= cc
CFLAGS ?= -O2 -Wall -Iinclude
# Link-time optimization lets the compiler inline fom_apply_* (fom_ops.c)
# into the solver; override with LTOFLAGS= to build without it.
LTOFLAGS ?= -flto
BENCHFLAGS ?= -O3 -Iinclude
SRC = src/fom_solver.c src/fom_data.c src/fom_linsolve.c src/fom_ops.c src/fom_objective.c
# Package-written reference matvecs for verify's direct check of fom_ops.c.
VERIFY_SRC = $(wildcard test/ops_ref.c)

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
LIBEXT = dylib
else
LIBEXT = so
endif

.PHONY: verify bench profile lib release clean objective_guard

verify: $(SRC) $(VERIFY_SRC) test/verify_main.c
	@if [ -f test/kernel_check.json ] && command -v python3 >/dev/null; then python3 test/kernel_check.py test/kernel_check.json; fi
	$(CC) $(CFLAGS) $(LTOFLAGS) -Itest $(SRC) $(VERIFY_SRC) test/verify_main.c -o verify_bin -lm
	./verify_bin

# Build the shared library from the marker-free production source.
release: release/fom_solver.c src/fom_data.c release/fom_linsolve.c release/fom_ops.c src/fom_objective.c
	$(CC) $(CFLAGS) $(LTOFLAGS) -shared -fPIC release/fom_solver.c src/fom_data.c release/fom_linsolve.c release/fom_ops.c src/fom_objective.c -o libfomsolver.$(LIBEXT) -lm

bench: $(SRC) test/bench_main.c
	$(CC) $(BENCHFLAGS) $(LTOFLAGS) -Itest $(SRC) test/bench_main.c -o bench_bin -lm
	./bench_bin

profile: $(SRC) src/fom_profile.c test/bench_main.c
	$(CC) $(BENCHFLAGS) $(LTOFLAGS) -DFOM_PROFILE -Itest $(SRC) src/fom_profile.c test/bench_main.c -o profile_bin -lm
	./profile_bin

lib: $(SRC)
	$(CC) $(CFLAGS) $(LTOFLAGS) -shared -fPIC $(SRC) -o libfomsolver.$(LIBEXT) -lm

clean:
	rm -f verify_bin bench_bin profile_bin libfomsolver.so libfomsolver.dylib
