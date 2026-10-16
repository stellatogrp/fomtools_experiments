/* Link-test stub for the generated solver ABI (fomtools_full_pipeline).

   Stage 1 links this against the real deterministic objects to prove the
   multi-GB verify/bench binaries link and load before any agent time is
   spent. It computes nothing and is deleted (make clean-solver) afterwards. */
#include <string.h>

#include "fom_solver.h"
#include "fom_data.h"

#ifdef __cplusplus
extern "C" {
#endif

void fom_set_default_settings(FomSettings *settings) {
    settings->max_iter = FOM_MAX_ITER;
    settings->tol = FOM_TOL;
    settings->check_termination = FOM_CHECK_TERMINATION;
}

void fom_setup(FomWorkspace *work, const FomSettings *settings) {
    work->settings = *settings;
}

void fom_update_params(FomWorkspace *work, const FomParams *params) {
    memcpy(work->params.p0, params->p0, sizeof(work->params.p0));
}

fom_int fom_solve(FomWorkspace *work) {
    /* Touch the initial-state table so the data object is genuinely used. */
    memcpy(work->solution.x, FOM_STATE_INIT, sizeof(work->solution.x));
    work->solution.iter = 0;
    work->solution.residual = (fom_float)0;
    work->solution.status = FOM_MAX_ITER_REACHED;
    return FOM_MAX_ITER_REACHED;
}

void fom_cleanup(FomWorkspace *work) { (void)work; }

#ifdef __cplusplus
}
#endif
