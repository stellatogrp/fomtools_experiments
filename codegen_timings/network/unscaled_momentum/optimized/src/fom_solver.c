#include "fom_solver.h"
#include "fom_data.h"
#include "fom_ops.h"
#include "fom_linsolve.h"
#include "fom_profile.h"

/* Fused operators over z = [x; t0; t1] (iteration-invariant, built once by
   probing the fixed sparse operators with unit vectors). Each block stores a
   union-pattern CSR with a base value a and a xi-coefficient d, so
   out_i = sum_p (a_p + xi d_p) z[col_p]. */
#define FOM_Z_X 0
#define FOM_Z_T0 FOM_STATE_DIM
#define FOM_Z_T1 (FOM_STATE_DIM + FOM_T0_DIM)
#define FOM_Z_DIM (FOM_STATE_DIM + FOM_T0_DIM + FOM_T1_DIM)
#define FOM_NNZ_CAP 1024

typedef void (*fom_op_fn)(const fom_float *, fom_float *);

typedef struct {
    fom_int rowptr[FOM_STATE_DIM + 1];
    fom_int col[FOM_NNZ_CAP];
    fom_float a[FOM_NNZ_CAP];
    fom_float d[FOM_NNZ_CAP];
} FomFused;

static FomFused g_t0, g_t1, g_su;
static int g_built = 0;
static int g_t0_ell3 = 0; /* every t0 row has exactly 3 entries */

/* triplet scratch */
static fom_int s_row[FOM_NNZ_CAP], s_col[FOM_NNZ_CAP];
static fom_float s_a[FOM_NNZ_CAP], s_d[FOM_NNZ_CAP];

static void fom_build_block(FomFused *F, fom_int nrows, const fom_op_fn *base,
                            const fom_op_fn *dxi, const fom_int *off,
                            const fom_int *dim, fom_int nsrc) {
    fom_float e[FOM_STATE_DIM];
    fom_float oa[FOM_STATE_DIM], od[FOM_STATE_DIM];
    fom_int n = 0, s, j, i, cnt[FOM_STATE_DIM + 1];
    for (s = 0; s < nsrc; ++s) {
        for (j = 0; j < dim[s]; ++j) e[j] = 0.0;
        for (j = 0; j < dim[s]; ++j) {
            e[j] = 1.0;
            for (i = 0; i < nrows; ++i) { oa[i] = 0.0; od[i] = 0.0; }
            if (base[s]) base[s](e, oa);
            if (dxi[s]) dxi[s](e, od);
            e[j] = 0.0;
            for (i = 0; i < nrows; ++i)
                if (oa[i] != 0.0 || od[i] != 0.0) {
                    s_row[n] = i; s_col[n] = off[s] + j;
                    s_a[n] = oa[i]; s_d[n] = od[i]; ++n;
                }
        }
    }
    for (i = 0; i <= nrows; ++i) cnt[i] = 0;
    for (s = 0; s < n; ++s) cnt[s_row[s] + 1]++;
    for (i = 0; i < nrows; ++i) cnt[i + 1] += cnt[i];
    for (i = 0; i <= nrows; ++i) F->rowptr[i] = cnt[i];
    for (s = 0; s < n; ++s) {
        const fom_int p = cnt[s_row[s]]++;
        F->col[p] = s_col[s]; F->a[p] = s_a[s]; F->d[p] = s_d[s];
    }
}

static void fom_build_fused(void) {
    if (g_built) return;
    {
        const fom_op_fn b[1] = {fom_apply_t0_state}, d[1] = {0};
        const fom_int off[1] = {FOM_Z_X}, dim[1] = {FOM_STATE_DIM};
        fom_build_block(&g_t0, FOM_T0_DIM, b, d, off, dim, 1);
    }
    {
        const fom_op_fn b[2] = {fom_apply_t1_state, fom_apply_t1_t_t0};
        const fom_op_fn d[2] = {fom_apply_t1_state__d_xi, fom_apply_t1_t_t0__d_xi};
        const fom_int off[2] = {FOM_Z_X, FOM_Z_T0}, dim[2] = {FOM_STATE_DIM, FOM_T0_DIM};
        fom_build_block(&g_t1, FOM_T1_DIM, b, d, off, dim, 2);
    }
    {
        const fom_op_fn b[3] = {fom_apply_su_state, fom_apply_su_t_t0, fom_apply_su_t_t1};
        const fom_op_fn d[3] = {fom_apply_su_state__d_xi, fom_apply_su_t_t0__d_xi, 0};
        const fom_int off[3] = {FOM_Z_X, FOM_Z_T0, FOM_Z_T1};
        const fom_int dim[3] = {FOM_STATE_DIM, FOM_T0_DIM, FOM_T1_DIM};
        fom_build_block(&g_su, FOM_STATE_DIM, b, d, off, dim, 3);
    }
    g_t0_ell3 = 1;
    { fom_int i; for (i = 0; i < FOM_T0_DIM; ++i) if (g_t0.rowptr[i] != 3 * i) g_t0_ell3 = 0; }
    if (g_t0.rowptr[FOM_T0_DIM] != 3 * FOM_T0_DIM) g_t0_ell3 = 0;
    g_built = 1;
}

void fom_set_default_settings(FomSettings *settings) {
    settings->max_iter = FOM_MAX_ITER;
    settings->tol = FOM_TOL;
    settings->check_termination = FOM_CHECK_TERMINATION;
}

static void fom_reset_state(FomWorkspace *work) {
    fom_int i;
    for (i = 0; i < FOM_STATE_DIM; ++i) work->state[i] = FOM_STATE_INIT[i];
}

void fom_setup(FomWorkspace *work, const FomSettings *settings) {
    fom_int i;
    work->settings = *settings;
    for (i = 0; i < FOM_PARAM_P0_DIM; ++i) work->params.p0[i] = 0.0;
    fom_reset_state(work);
    work->solution.iter = 0;
    work->solution.residual = 0.0;
    work->solution.status = FOM_MAX_ITER_REACHED;
    fom_build_fused();
}

void fom_update_params(FomWorkspace *work, const FomParams *params) {
    work->params = *params;
    fom_reset_state(work);
}

fom_int fom_solve(FomWorkspace *work) {
    fom_float zbuf[2][FOM_Z_DIM];
    fom_float *z = zbuf[0];
    fom_float *zn = zbuf[1];
    fom_float res = 0.0;
    const fom_int max_iter = work->settings.max_iter;
    fom_int it, i, p;
    fom_float pterm[FOM_STATE_DIM];

    fom_build_fused();
    /* su_p_p0 @ p0 is iteration-invariant: hoist it. */
    for (i = 0; i < FOM_STATE_DIM; ++i) pterm[i] = 0.0;
    fom_apply_su_p_p0(work->params.p0, pterm);
    for (i = 0; i < FOM_STATE_DIM; ++i) z[i] = work->state[i];

    for (it = 0; it < max_iter; ++it) {
        const fom_float xi = fom_sched_xi(it);
        fom_float *t0 = z + FOM_Z_T0;
        fom_float *t1 = z + FOM_Z_T1;
        fom_float *xn = zn;

        FOM_PROF_BEGIN(t0);
        if (g_t0_ell3) {
            const fom_int *c = g_t0.col;
            const fom_float *a = g_t0.a;
            for (i = 0; i < FOM_T0_DIM; ++i) {
                fom_float v = FOM_MAT_t0_c[i] + a[3 * i] * z[c[3 * i]]
                            + a[3 * i + 1] * z[c[3 * i + 1]] + a[3 * i + 2] * z[c[3 * i + 2]];
                const fom_float lo = FOM_MAT_t0_lo[i];
                const fom_float hi = FOM_MAT_t0_hi[i];
                v = v < lo ? lo : v;
                v = v > hi ? hi : v;
                t0[i] = v;
            }
        } else
        for (i = 0; i < FOM_T0_DIM; ++i) {
            fom_float v = FOM_MAT_t0_c[i];
            const fom_float lo = FOM_MAT_t0_lo[i];
            const fom_float hi = FOM_MAT_t0_hi[i];
            for (p = g_t0.rowptr[i]; p < g_t0.rowptr[i + 1]; ++p)
                v += g_t0.a[p] * z[g_t0.col[p]];
            v = v < lo ? lo : v;
            v = v > hi ? hi : v;
            t0[i] = v;
        }
        FOM_PROF_END(t0);

        FOM_PROF_BEGIN(t1);
        for (i = 0; i < FOM_T1_DIM; ++i) {
            fom_float v = FOM_MAT_t1_c[i];
            for (p = g_t1.rowptr[i]; p < g_t1.rowptr[i + 1]; ++p)
                v += (g_t1.a[p] + xi * g_t1.d[p]) * z[g_t1.col[p]];
            t1[i] = v > 0.0 ? v : 0.0;
        }
        FOM_PROF_END(t1);

        FOM_PROF_BEGIN(state_update);
        for (i = 0; i < FOM_STATE_DIM; ++i) {
            fom_float v = pterm[i];
            for (p = g_su.rowptr[i]; p < g_su.rowptr[i + 1]; ++p)
                v += (g_su.a[p] + xi * g_su.d[p]) * z[g_su.col[p]];
            xn[i] = v;
        }
        FOM_PROF_END(state_update);

        if (it + 1 == max_iter) {
            res = 0.0;
            for (i = 0; i < FOM_STATE_DIM; ++i) {
                fom_float d = xn[i] - z[i];
                d = d < 0.0 ? -d : d;
                res = d > res ? d : res;
            }
        }

        { fom_float *tmp = z; z = zn; zn = tmp; }
    }

    for (i = 0; i < FOM_STATE_DIM; ++i) work->state[i] = z[i];
    for (i = 0; i < FOM_STATE_DIM; ++i) work->solution.x[i] = z[i];
    work->solution.iter = max_iter;
    work->solution.residual = res;
    work->solution.status = FOM_MAX_ITER_REACHED;
    return FOM_MAX_ITER_REACHED;
}

void fom_cleanup(FomWorkspace *work) {
    (void)work;
}
