/* C-level timing of one generated CVXPYgen solver.
 *
 * Built once per solver by CMakeLists.txt with -DPREFIX=<cvxpygen prefix> (the
 * generated symbols are PREFIX##_cpg_solve etc.) and one of
 *   -DMODE_OFF  OSQP defaults with a fixed max_iter, no termination check, cold start;
 *               with -DBENCH_RHO=<value>, OSQP runs at that fixed rho instead of
 *               0.1 with adaptive rho (osqp_update_rho once before the first solve,
 *               adaptive rho off: CVXPYgen generates OSQP at rho 0.1 and exposes no
 *               setting for it, and the C solver carries an adapted rho from one
 *               solve to the next)
 *   -DMODE_FOM  the fomtools solver as generated (fixed iteration count, cold start)
 *
 * Usage: bench_cpg <holdings.bin> <count> <N> <passes> [iters]
 * holdings.bin holds count x 2N doubles (row-major), the back-test sequence: each
 * row is the holdings x followed by the return forecast mu. One warm-up pass, then
 * <passes> timed passes over the sequence in order; each solve updates every
 * entry of x and mu, canonicalizes and solves. Prints the mean
 * microseconds per solve, the mean iteration count, and the largest primal and
 * dual residual over the timed solves.
 *
 * [iters] is one of
 *   <K>             a fixed iteration count for every solve (default 200; MODE_OFF)
 *   @<file>         per-solve iteration counts, <count> int32 values, set through
 *                   cpg_set_solver_max_iter before each solve (any mode)
 *   search=<tol>    MODE_OFF only: one pass in order with OSQP's own termination
 *                   test checked every iteration (eps_abs = tol, eps_rel = 0, so
 *                   both unscaled residuals <= tol), printing each solve's
 *                   iteration count; nothing is timed
 *   stop=<tol>,<check>,<max_iter>
 *                   every solver stops itself: OSQP with eps_abs = tol,
 *                   eps_rel = 0, unscaled residuals, and check_dualgap = 0 (OSQP
 *                   1.x also stops only once the duality gap is below eps_abs;
 *                   off, its test is the residuals alone, as fomtools'); fomtools (generated with
 *                   TerminationKind.OBJECTIVE) with its runtime tol; both checked
 *                   every <check> iterations, at most <max_iter>. Timed as above;
 *                   the residual pass also prints each solve's iteration count
 *                   (iters=), max(pri_res, dua_res) at the stop (res=) and the
 *                   number of solves not reported solved (unsolved=)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "cpg_solve.h"
#include "cpg_workspace.h"
#if defined(MODE_OFF) /* the embedded OSQP: osqp_update_rho, the solver global */
#include "osqp_api_functions.h"
#include "workspace.h"
#endif

#define CAT_(a, b) a##b
#define CAT(a, b) CAT_(a, b)
#define F(name) CAT(PREFIX, name)

/* Every entry of x and mu from row k of the [x; mu] sequence. */
static void update(const double *v, int k, int n) {
  const double *row = v + (size_t)k * 2 * n;
  for (int i = 0; i < n; i++) F(_cpg_update_x)(i, row[i]);
  for (int i = 0; i < n; i++) F(_cpg_update_mu)(i, row[n + i]);
}

static double now_s(void) {
  struct timespec t;
  clock_gettime(CLOCK_MONOTONIC, &t);
  return (double)t.tv_sec + 1e-9 * (double)t.tv_nsec;
}

int main(int argc, char *argv[]) {
  if (argc < 5) {
    fprintf(stderr, "usage: %s holdings.bin count N passes [max_iter]\n", argv[0]);
    return 2;
  }
  int count = atoi(argv[2]), n = atoi(argv[3]), passes = atoi(argv[4]);
  double *x = malloc(sizeof(double) * (size_t)count * 2 * (size_t)n);
  FILE *f = fopen(argv[1], "rb");
  if (!f || fread(x, sizeof(double), (size_t)count * 2 * n, f) != (size_t)count * 2 * n) {
    fprintf(stderr, "cannot read %s\n", argv[1]);
    return 1;
  }
  fclose(f);

  const char *arg = argc > 5 ? argv[5] : "200";
  int *per_solve = NULL;
  if (arg[0] == '@') {
    per_solve = malloc(sizeof(int) * (size_t)count);
    FILE *g = fopen(arg + 1, "rb");
    if (!g || fread(per_solve, sizeof(int), (size_t)count, g) != (size_t)count) {
      fprintf(stderr, "cannot read %s\n", arg + 1);
      return 1;
    }
    fclose(g);
  }

  int stop = strncmp(arg, "stop=", 5) == 0;
  double stop_tol = 0.0;
  int stop_check = 0, stop_max_iter = 0;
  if (stop && sscanf(arg + 5, "%lf,%d,%d", &stop_tol, &stop_check, &stop_max_iter) != 3) {
    fprintf(stderr, "expected stop=<tol>,<check>,<max_iter>, got %s\n", arg);
    return 2;
  }
  int *stop_iters = stop ? malloc(sizeof(int) * (size_t)count) : NULL;
  double *stop_res = stop ? malloc(sizeof(double) * (size_t)count) : NULL;
  int unsolved = 0;

#if defined(MODE_OFF)
  F(_cpg_set_solver_default_settings)();
#if defined(BENCH_RHO)
  if (osqp_update_rho(&solver, BENCH_RHO) != 0) {
    fprintf(stderr, "osqp_update_rho(%g) failed\n", (double)BENCH_RHO);
    return 1;
  }
  solver.settings->adaptive_rho = OSQP_ADAPTIVE_RHO_UPDATE_DISABLED;
#endif
  F(_cpg_set_solver_warm_starting)(0);
  if (strncmp(arg, "search=", 7) == 0) {
    double tol = atof(arg + 7);
    F(_cpg_set_solver_max_iter)(20000);
    F(_cpg_set_solver_check_termination)(1);
    F(_cpg_set_solver_eps_abs)(tol);
    F(_cpg_set_solver_eps_rel)(0.0);
    printf("iters=");
    for (int k = 0; k < count; k++) {
      update(x, k, n);
      F(_cpg_solve)();
      printf("%d%s", (int)F(_CPG_Info).iter, k + 1 < count ? "," : "\n");
    }
    return 0;
  }
  if (stop) {
    F(_cpg_set_solver_max_iter)(stop_max_iter);
    F(_cpg_set_solver_check_termination)(stop_check);
    F(_cpg_set_solver_eps_abs)(stop_tol);
    F(_cpg_set_solver_eps_rel)(0.0);
    F(_cpg_set_solver_scaled_termination)(0);
    solver.settings->check_dualgap = 0; /* no cpg setter for it */
  } else {
    F(_cpg_set_solver_check_termination)(0);
    if (!per_solve) F(_cpg_set_solver_max_iter)(atoi(arg));
  }
#define SOLVED() (strcmp(F(_CPG_Info).status, "solved") == 0)
#elif defined(MODE_FOM)
  if (stop) {
    F(_cpg_set_solver_max_iter)(stop_max_iter);
    F(_cpg_set_solver_tol)(stop_tol);
    F(_cpg_set_solver_check_termination)(stop_check);
  } else if (!per_solve && strcmp(arg, "200") != 0) {
    F(_cpg_set_solver_max_iter)(atoi(arg));
  }
#define SOLVED() (F(_CPG_Info).status == 0) /* FOM_SOLVED */
#else
#error "define one of MODE_OFF, MODE_FOM"
#endif

  double elapsed = 0.0, iters = 0.0, checksum = 0.0, max_pri = 0.0, max_dua = 0.0;
  for (int pass = 0; pass <= passes; pass++) {
    double start = now_s();
    for (int k = 0; k < count; k++) {
      if (per_solve) F(_cpg_set_solver_max_iter)(per_solve[k]);
      update(x, k, n);
      F(_cpg_solve)();
      if (pass > 0) iters += F(_CPG_Info).iter;
      checksum += F(_CPG_Info).obj_val;
    }
    if (pass > 0) elapsed += now_s() - start;
  }
  /* The residuals of each solve, once more outside the timed passes. */
  for (int k = 0; k < count; k++) {
    if (per_solve) F(_cpg_set_solver_max_iter)(per_solve[k]);
    update(x, k, n);
    F(_cpg_solve)();
    if (F(_CPG_Info).pri_res > max_pri) max_pri = F(_CPG_Info).pri_res;
    if (F(_CPG_Info).dua_res > max_dua) max_dua = F(_CPG_Info).dua_res;
    if (stop) {
      stop_iters[k] = (int)F(_CPG_Info).iter;
      stop_res[k] = F(_CPG_Info).pri_res > F(_CPG_Info).dua_res ? F(_CPG_Info).pri_res
                                                               : F(_CPG_Info).dua_res;
      unsolved += !SOLVED();
    }
  }
  int solves = passes * count;
  printf("us_per_solve=%.6f mean_iters=%.3f checksum=%.6e max_pri_res=%.3e max_dua_res=%.3e\n",
         1e6 * elapsed / solves, iters / solves, checksum, max_pri, max_dua);
  if (stop) {
    printf("unsolved=%d\niters=", unsolved);
    for (int k = 0; k < count; k++) printf("%d%s", stop_iters[k], k + 1 < count ? "," : "\n");
    printf("res=");
    for (int k = 0; k < count; k++) printf("%.3e%s", stop_res[k], k + 1 < count ? "," : "\n");
  }
  free(x);
  return 0;
}
