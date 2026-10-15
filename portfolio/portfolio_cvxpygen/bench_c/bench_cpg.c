/* C-level timing of one generated CVXPYgen solver.
 *
 * Built once per solver by CMakeLists.txt with -DPREFIX=<cvxpygen prefix> (the
 * generated symbols are PREFIX##_cpg_solve etc.) and one of
 *   -DMODE_OFF  OSQP defaults with a fixed max_iter, no termination check, cold start
 *   -DMODE_FOM  the fomtools solver as generated (fixed iteration count, cold start)
 *
 * Usage: bench_cpg <holdings.bin> <count> <N> <passes> [iters]
 * holdings.bin holds count x N doubles (row-major), the back-test sequence. One
 * warm-up pass, then <passes> timed passes over the sequence in order; each
 * solve updates every entry of x, canonicalizes and solves. Prints the mean
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
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "cpg_solve.h"
#include "cpg_workspace.h"

#define CAT_(a, b) a##b
#define CAT(a, b) CAT_(a, b)
#define F(name) CAT(PREFIX, name)

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
  double *x = malloc(sizeof(double) * (size_t)count * (size_t)n);
  FILE *f = fopen(argv[1], "rb");
  if (!f || fread(x, sizeof(double), (size_t)count * n, f) != (size_t)count * n) {
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

#if defined(MODE_OFF)
  F(_cpg_set_solver_default_settings)();
  F(_cpg_set_solver_warm_starting)(0);
  if (strncmp(arg, "search=", 7) == 0) {
    double tol = atof(arg + 7);
    F(_cpg_set_solver_max_iter)(20000);
    F(_cpg_set_solver_check_termination)(1);
    F(_cpg_set_solver_eps_abs)(tol);
    F(_cpg_set_solver_eps_rel)(0.0);
    printf("iters=");
    for (int k = 0; k < count; k++) {
      for (int i = 0; i < n; i++) F(_cpg_update_x)(i, x[(size_t)k * n + i]);
      F(_cpg_solve)();
      printf("%d%s", (int)F(_CPG_Info).iter, k + 1 < count ? "," : "\n");
    }
    return 0;
  }
  F(_cpg_set_solver_check_termination)(0);
  if (!per_solve) F(_cpg_set_solver_max_iter)(atoi(arg));
#elif defined(MODE_FOM)
  if (!per_solve && strcmp(arg, "200") != 0) F(_cpg_set_solver_max_iter)(atoi(arg));
#else
#error "define one of MODE_OFF, MODE_FOM"
#endif

  double elapsed = 0.0, iters = 0.0, checksum = 0.0, max_pri = 0.0, max_dua = 0.0;
  for (int pass = 0; pass <= passes; pass++) {
    double start = now_s();
    for (int k = 0; k < count; k++) {
      if (per_solve) F(_cpg_set_solver_max_iter)(per_solve[k]);
      for (int i = 0; i < n; i++) F(_cpg_update_x)(i, x[(size_t)k * n + i]);
      F(_cpg_solve)();
      if (pass > 0) iters += F(_CPG_Info).iter;
      checksum += F(_CPG_Info).obj_val;
    }
    if (pass > 0) elapsed += now_s() - start;
  }
  /* The residuals of each solve, once more outside the timed passes. */
  for (int k = 0; k < count; k++) {
    if (per_solve) F(_cpg_set_solver_max_iter)(per_solve[k]);
    for (int i = 0; i < n; i++) F(_cpg_update_x)(i, x[(size_t)k * n + i]);
    F(_cpg_solve)();
    if (F(_CPG_Info).pri_res > max_pri) max_pri = F(_CPG_Info).pri_res;
    if (F(_CPG_Info).dua_res > max_dua) max_dua = F(_CPG_Info).dua_res;
  }
  int solves = passes * count;
  printf("us_per_solve=%.6f mean_iters=%.3f checksum=%.6e max_pri_res=%.3e max_dua_res=%.3e\n",
         1e6 * elapsed / solves, iters / solves, checksum, max_pri, max_dua);
  free(x);
  return 0;
}
