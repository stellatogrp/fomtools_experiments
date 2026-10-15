CC ?= cc
CFLAGS ?= -O2 -Wall -Iinclude
BENCHFLAGS ?= -O3 -Iinclude
SRC = src/fom_solver.c src/fom_data.c src/fom_linsolve.c src/fom_ops.c

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
LIBEXT = dylib
else
LIBEXT = so
endif

.PHONY: verify bench profile lib release clean

verify: $(SRC) test/verify_main.c
	$(CC) $(CFLAGS) -Itest $(SRC) test/verify_main.c -o verify_bin -lm
	./verify_bin

# Build the shared library from the marker-free production source.
release: release/fom_solver.c src/fom_data.c release/fom_linsolve.c src/fom_ops.c
	$(CC) $(CFLAGS) -shared -fPIC release/fom_solver.c src/fom_data.c release/fom_linsolve.c src/fom_ops.c -o libfomsolver.$(LIBEXT) -lm

bench: $(SRC) test/bench_main.c
	$(CC) $(BENCHFLAGS) -Itest $(SRC) test/bench_main.c -o bench_bin -lm
	./bench_bin

profile: $(SRC) src/fom_profile.c test/bench_main.c
	$(CC) $(BENCHFLAGS) -DFOM_PROFILE -Itest $(SRC) src/fom_profile.c test/bench_main.c -o profile_bin -lm
	./profile_bin

lib: $(SRC)
	$(CC) $(CFLAGS) -shared -fPIC $(SRC) -o libfomsolver.$(LIBEXT) -lm

clean:
	rm -f verify_bin bench_bin profile_bin libfomsolver.so libfomsolver.dylib
