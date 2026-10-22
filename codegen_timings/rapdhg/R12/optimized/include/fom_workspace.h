#ifndef FOM_WORKSPACE_H
#define FOM_WORKSPACE_H

typedef double fom_float;
typedef int fom_int;

#define FOM_STATE_DIM 160
#define FOM_MAX_ITER 200
#define FOM_TOL ((fom_float)0)
#define FOM_USE_TOL 0
#define FOM_USE_OBJECTIVE 0
#define FOM_CHECK_TERMINATION 1
#define FOM_NUM_PARAMS 2
#define FOM_MAX_VEC_DIM 160
#define FOM_UNROLLED 0

#define FOM_SOLVED 0
#define FOM_MAX_ITER_REACHED 1

#define FOM_PARAM_P0_DIM 16
#define FOM_PARAM_P1_DIM 48
#define FOM_T0_DIM 48
#define FOM_T1_DIM 16

typedef struct {
    fom_int max_iter;
    fom_float tol;
    fom_int check_termination;
} FomSettings;

typedef struct {
    fom_float p0[FOM_PARAM_P0_DIM];
    fom_float p1[FOM_PARAM_P1_DIM];
} FomParams;

typedef struct {
    fom_float x[FOM_STATE_DIM];
    fom_int iter;
    fom_float residual;
    fom_int status;
} FomSolution;

typedef struct {
    FomSettings settings;
    FomParams params;
    FomSolution solution;
    fom_float state[FOM_STATE_DIM];
    fom_float new_state[FOM_STATE_DIM];
    fom_float acc[FOM_MAX_VEC_DIM];
    fom_float t0[FOM_T0_DIM];
    fom_float t1[FOM_T1_DIM];
} FomWorkspace;

#endif /* FOM_WORKSPACE_H */
