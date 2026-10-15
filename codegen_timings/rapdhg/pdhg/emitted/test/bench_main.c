#include <stdio.h>
#include <stdlib.h>
#include "fom_solver.h"
#include "fom_profile.h"
#include "verify_data.h"

/* Benchmark: time full solves (each = the complete iteration loop).
   Usage: bench_bin [num_solves]. Prints a parseable BENCH line and,
   when built with -DFOM_PROFILE, a per-region PROF breakdown. */
int main(int argc, char **argv) {
    int reps = (argc > 1) ? atoi(argv[1]) : 5000;
    FomWorkspace work;
    FomSettings settings;
    FomParams params;
    int s, j;
    double total = 0.0, best = 1.0e30;

    fom_set_default_settings(&settings);
    fom_setup(&work, &settings);

    /* warmup */
    for (s = 0; s < FOM_NUM_SAMPLES; ++s) {
        for (j = 0; j < FOM_PARAM_P0_DIM; ++j)
            params.p0[j] = FOM_VERIFY_P0[s % FOM_NUM_SAMPLES][j];
        for (j = 0; j < FOM_PARAM_P1_DIM; ++j)
            params.p1[j] = FOM_VERIFY_P1[s % FOM_NUM_SAMPLES][j];
        fom_update_params(&work, &params);
        fom_solve(&work);
    }
    fom_prof_reset();

    for (s = 0; s < reps; ++s) {
        for (j = 0; j < FOM_PARAM_P0_DIM; ++j)
            params.p0[j] = FOM_VERIFY_P0[s % FOM_NUM_SAMPLES][j];
        for (j = 0; j < FOM_PARAM_P1_DIM; ++j)
            params.p1[j] = FOM_VERIFY_P1[s % FOM_NUM_SAMPLES][j];
        fom_update_params(&work, &params);
        double t0 = fom_now_ns();
        fom_solve(&work);
        double dt = fom_now_ns() - t0;
        total += dt;
        if (dt < best) best = dt;
    }

    printf("BENCH solves=%d total_us=%.4f mean_us=%.6f min_us=%.6f\n",
           reps, total / 1000.0, (total / 1000.0) / (double)reps, best / 1000.0);
    fom_prof_report();
    return 0;
}
