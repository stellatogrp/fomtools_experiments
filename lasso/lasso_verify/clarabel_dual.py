"""Solve fomtools' SDP as one coupled PSD cone, decomposed by Clarabel itself.

Ported from the portfolio experiment's scratch patch; fomtools is unchanged.

fomtools poses   min q'x  s.t.  Aeq x = beq,  Aineq x <= bineq,  X = mat(x) PSD
(the value it reports is -min). ``install()`` replaces its per-cut cones with a
single cone over all of X and solves the dual as a Clarabel primal in (y, w):

    min  beq'y + bineq'w   s.t.  w >= 0,   D^{-1} (q + Aeq'y + Aineq'w) in PSD,

whose value equals fomtools' reported value. The PSD slack has structural zeros
wherever no row touches an entry of X, so Clarabel's chordal decomposition can
split it, and its dual variable (X) lives in the PSD-completable cone. That is
equivalent to one full cone. (In the primal, a zero row would instead fix the
entry to 0, which restricts X rather than leaving it free.)
"""

from __future__ import annotations

import clarabel
import numpy as np
import scipy.sparse as spa
from fomtools.solvers.clarabel import ClarabelSolver

# Filled by every solve: status, iterations, X size, used pattern entries.
LAST: dict = {}
_orig_setup = ClarabelSolver.setup_clarabel_data


def _setup_full_cone(self, n, C, Aeq_mats, beq, Aineq_mats, bineq, psd_cone_indices):
    _orig_setup(self, n, C, Aeq_mats, beq, Aineq_mats, bineq, [set(range(n))])


def _solve_dual(self, **kwargs):
    M = self.M.tocsc()
    d = -M.diagonal()
    assert M.shape[0] == M.shape[1] and (M - spa.diags(M.diagonal())).nnz == 0 and np.all(d > 0)
    n_eq, n_in = self.Aeq.shape[0], self.Aineq.shape[0]
    Dinv = spa.diags(1.0 / d)
    AT = spa.hstack([self.Aeq.T, self.Aineq.T]).tocsc()
    A = spa.vstack(
        [
            spa.hstack([spa.csc_matrix((n_in, n_eq)), -spa.eye(n_in)]),
            -(Dinv @ AT),
        ]
    ).tocsc()
    b = np.concatenate([np.zeros(n_in), Dinv @ self.q])
    q = np.concatenate([self.beq, self.bineq])
    n = int(round((np.sqrt(8 * M.shape[0] + 1) - 1) / 2))
    used = np.flatnonzero((abs(AT).sum(axis=1).A1 > 0) | (self.q != 0))
    cones = [clarabel.NonnegativeConeT(n_in), clarabel.PSDTriangleConeT(n)]
    settings = clarabel.DefaultSettings()
    settings.verbose = False
    settings.chordal_decomposition_enable = True
    sol = clarabel.DefaultSolver(spa.csc_matrix((q.size, q.size)), q, A, b, cones, settings).solve()
    LAST.update(
        status=str(sol.status),
        iterations=sol.iterations,
        psd_dim=n,
        pattern=f"{used.size} of {M.shape[0]}",
    )
    return sol.obj_val


def install() -> None:
    ClarabelSolver.setup_clarabel_data = _setup_full_cone
    ClarabelSolver.solve = _solve_dual
