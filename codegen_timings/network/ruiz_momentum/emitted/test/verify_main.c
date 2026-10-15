#include <stdio.h>
#include <math.h>
#include "fom_solver.h"
#include "verify_data.h"

int main(void) {
    FomWorkspace work;
    FomSettings settings;
    FomParams params;
    int s, j;
    fom_float tol = (fom_float)9.9999999999999995e-07;
    fom_float worst = 0.0;
    int failures = 0;

    fom_set_default_settings(&settings);
    fom_setup(&work, &settings);

    for (s = 0; s < FOM_NUM_SAMPLES; ++s) {
        for (j = 0; j < FOM_PARAM_P0_DIM; ++j)
            params.p0[j] = FOM_VERIFY_P0[s][j];
        fom_update_params(&work, &params);
        fom_solve(&work);
        fom_float err = 0.0;
        for (j = 0; j < FOM_STATE_DIM; ++j) {
            fom_float d = work.solution.x[j] - FOM_VERIFY_EXPECTED[s][j];
            if (d < 0) d = -d;
            if (d > err) err = d;
        }
        if (err > worst) worst = err;
        if (err > tol) {
            printf("sample %d: max abs error %.3e > tol %.3e\n", s, (double)err, (double)tol);
            failures++;
        }
    }
    printf("worst max-abs error over %d samples: %.3e (tol %.3e)\n",
           FOM_NUM_SAMPLES, (double)worst, (double)tol);
    if (failures) {
        printf("VERIFY FAILED: %d/%d samples exceed tol\n", failures, FOM_NUM_SAMPLES);
        return 1;
    }

    printf("VERIFY OK\n");
    return 0;
}
