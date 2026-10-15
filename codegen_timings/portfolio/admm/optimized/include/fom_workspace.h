#ifndef FOM_WORKSPACE_H
#define FOM_WORKSPACE_H

typedef double fom_float;
typedef int fom_int;

#define FOM_STATE_DIM 51
#define FOM_MAX_ITER 200
#define FOM_TOL ((fom_float)0)
#define FOM_USE_TOL 0
#define FOM_CHECK_TERMINATION 1
#define FOM_NUM_PARAMS 1
#define FOM_MAX_VEC_DIM 51
#define FOM_UNROLLED 0

#define FOM_SOLVED 0
#define FOM_MAX_ITER_REACHED 1

#define FOM_PARAM_P0_DIM 10
#define FOM_T0_DIM 10
#define FOM_T1_DIM 10
#define FOM_T2_DIM 10
#define FOM_T3_DIM 20

typedef struct {
    fom_int max_iter;
    fom_float tol;
    fom_int check_termination;
} FomSettings;

typedef struct {
    fom_float p0[FOM_PARAM_P0_DIM];
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
    fom_float t2[FOM_T2_DIM];
    fom_float t3[FOM_T3_DIM];
} FomWorkspace;

#endif /* FOM_WORKSPACE_H */
