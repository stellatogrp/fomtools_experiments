# Portfolio rebalancing: status (2026-09-30)

> **Update 2026-10-04.** `problem.py` now matches the paper's notation. `GAMMA` (risk aversion) is now `KAPPA = 1.0` and `KAPPA` (transaction cost) is now `LAMBD = 0.05`, so the objective reads $(\kappa/2) z^\top \Sigma z - \mu^\top z + \lambda \lVert z - x \rVert_1$. The position limit is now the vector $\bar z = 0.5\cdot\mathbf{1}$, where it used to be the scalar 0.6. **Every result below was computed with $\bar z = 0.6$ and has to be rerun.**

## Problem

$$
\begin{array}{ll}
\text{minimize} & \tfrac{\gamma}{2} z^\top \Sigma z - \mu^\top z + \kappa \lVert z - x \rVert_1 \\
\text{subject to} & \mathbf{1}^\top z = 1,\quad 0 \le z \le \bar z
\end{array}
$$

- **Parameter:** $x$ is the portfolio currently held. It was feasible last period, so it lies in the capped simplex $\{\mathbf{1}^\top x = 1,\ 0 \le x \le \bar z\}$.
- **Ablation:** the simplex compared with its bounding box $[0, \bar z]^n$. The two VPs differ by one `add_constraint(1ᵀx == 1)` call.
- **Data:** $n = 3$, $\gamma = 1$, $\kappa = 0.05$, $\bar z = 0.6$. $\Sigma = FF^\top/n + 0.1 I$ with Gaussian $F$, and $\mu \sim U[0, 0.5]$ (seed 0).
- **ℓ1 term:** written with an epigraph variable $\tau \ge |z - x|$, so $y = (z, \tau) \in \mathbf{R}^6$. There are 10 constraint rows: budget, position, sell ($z - \tau \le x$) and buy ($z + \tau \ge x$).
- **Algorithm:** OSQP-style ADMM with $\rho = 1$ and $\sigma = 10^{-4}$. The iterate is $s = (y, v) \in \mathbf{R}^{16}$.
  - Warm start at the current holdings: $z^0 = x$, $\tau^0 = 0$, $v^0 = My^0$.
  - The parameter enters only the one-sided bounds of the sell and buy rows. Those projections are written as $\min(v, x) = x - \mathrm{relu}(x - v)$ and $\max(v, x) = x + \mathrm{relu}(v - x)$, because `satlin` bounds must be constants.
  - Each iteration has 3 satlin, 6 ReLU and one 6×6 linear solve.
- **Metric:** the OSQP primal-dual residual $H(s^K - s^{K-1})$.
- **Check:** in a scratch script, ADMM agreed with the CVXPY/Clarabel solution to within $2\cdot 10^{-7}$ on 8 sampled $x$.

## MIP, infinity norm (the current plot)

Gurobi MIP with Gurobi OBBT. "Sample max" is the largest residual over 10,000 samples of $x$: uniform on the box, and uniform on the capped simplex (Dirichlet with rejection).

| K | box MIP | box sample max | simplex MIP | simplex sample max | box / simplex |
|---:|---:|---:|---:|---:|---:|
| 1 | 1.355 | 1.305 | 0.446 | 0.445 | 3.0x |
| 2 | 0.802 | 0.769 | 0.248 | 0.248 | 3.2x |
| 3 | 0.510 | 0.488 | 0.193 | 0.193 | 2.6x |
| 4 | 0.356 | 0.340 | 0.173 | 0.173 | 2.1x |
| 5 | 0.325 | 0.311 | 0.138 | 0.138 | 2.4x |

MIP solve times:

| K | box | simplex |
|---:|---:|---:|
| 1 | 0.02 s | 0.02 s |
| 2 | 0.05 s | 0.04 s |
| 3 | 0.07 s | 0.06 s |
| 4 | 0.23 s | 0.06 s |
| 5 | 4.43 s | 0.09 s |

The full run, both sets up to $K = 5$ plus sampling, takes about 5 s.

![bounds](results/portfolio_bounds.png)

## SDP

Clarabel with Gurobi OBBT and all five cuts: RLT, triangle, ReLU envelope, cross, and subgradient monotonicity. $K = 1$ to $3$, and $K = 4$ for some simplex variants. All SDP numbers were rerun on 2026-10-01 on fomtools branch `vr/tightening` (uncommitted, on top of `db99867`). It contains the OBBT changes and the satlin hull rows listed at the end of this section. The earlier numbers, from `b63c275` without the hull rows, are given for comparison.

### Objective: use the two-norm

The infinity-norm objective is unusable in the SDP. `_apply_infinity_norm_objective` builds $\lVert r \rVert_\infty$ from a chain of $3 \cdot 16 - 1 = 47$ ReLUs: $|r_i| = \mathrm{relu}(r_i) + \mathrm{relu}(-r_i)$, followed by a running max. It then puts all 47 in one PSD cone. On the box at $K = 1$, with RLT only, that took 148 s and was 59% above the MIP. With every cut on, it had not finished after 24 CPU-minutes, at about 5 GB. This was measured before the hull rows and has not been rerun.

All SDP results below use `set_objective_norm(..., ord=2)`, which reports the **squared** 2-norm. The MIP is run with the same objective for comparison. For the MIP that makes it a nonconvex MIQP, which is slower than the infinity norm, although it stays cheap on the simplex.

| K | simplex sample max | simplex MIP | box sample max | box MIP |
|---:|---:|---:|---:|---:|
| 1 | 0.656 | 0.657 | 5.149 | 5.557 |
| 2 | 0.240 | 0.241 | 2.219 | 2.417 |
| 3 | 0.168 | 0.168 | 1.334 | 1.458 |
| 4 | 0.115 | 0.1155 | – | – |

MIP solve times:

| K | simplex | box |
|---:|---:|---:|
| 1 | 0.04 s | 0.01 s |
| 2 | 0.69 s | 1.1 s |
| 3 | 0.81 s | 11.6 s |
| 4 | 7.5 s | – |

The box was not run at K = 4.

### Results (simplex)

Values are squared 2-norms.

| K | MIP | full PSD cone | chordal, cut cones dropped | chordal, cut cones kept | chordal, RLT only |
|---:|---:|---:|---:|---:|---:|
| 1 | 0.657 | 0.6573 | 0.6574 | 0.6574 | 0.6574 |
| 2 | 0.241 | 0.2412 | 0.2412 | 0.2412 | 0.2412 |
| 3 | 0.168 | 0.1684 | 0.1687 | not finished (10 min cap) | 0.2013 |
| 4 | 0.1155 | 0.1155* | 0.1254 | – | – |

\* At K = 4 the full PSD cone is solved through Clarabel's own chordal decomposition (see below), which is equivalent to the full cone. The default chordal variant and RLT only were not run at K = 4.

Solve times (one solve per $K$):

| K | full PSD cone | cut cones dropped | cut cones kept | RLT only |
|---:|---:|---:|---:|---:|
| 1 | 3.3 s | 3.1 s | 3.0 s | 3.0 s |
| 2 | 29 s | 8.6 s | 104 s | 7.9 s |
| 3 | 141 s | 16 s | – | 15 s |
| 4 | 97 s* | 22 s | – | – |

SDP / MIP in norm terms, now and before the hull rows:

| K | full PSD cone | cut cones dropped | cut cones kept | RLT only |
|---:|---:|---:|---:|---:|
| 1 | 1.00 (was 1.09) | 1.00 (was 1.09) | 1.00 (was 1.09) | 1.00 (was 1.09) |
| 2 | 1.00 (was 1.03) | 1.00 (was 1.30) | 1.00 (was 1.03) | 1.00 (was 1.60) |
| 3 | 1.00 (was 1.01) | 1.00 (was 1.54) | – | 1.09 (was 2.48) |
| 4 | 1.00* | 1.04 | – | – |

- **The satlin hull rows close the gap.** Every variant is now exact at K = 1 and 2, and the full cone and the cut-cones-dropped variant are within 0.2% at K = 3. Before, K = 1 was 9% loose for every variant, and the chordal variants drifted apart with K.
  - **What the hull rows are:** fomtools writes the position projection $\mathrm{clip}(v, 0, \bar z)$ as two ReLUs, and before this change every cut acted on one ReLU at a time.
  - **What they add:** the two chords of the convex hull of $v \mapsto \mathrm{clip}(v, 0, \bar z)$ over the OBBT input bounds. They are registered as one inequality constraint, so the inequality cuts also multiply the rows with each other and with the other variables' bounds.
- **Chordal, cut cones dropped is the cheapest choice through K = 3, but it loosens after that.**
  - **Through K = 3:** it matches the full cone to 0.2% at K = 3 and is 9x faster (16 s against 141 s). Without the hull rows it was 54% loose there.
  - **At K = 4:** it is 4% loose (0.1254 against an exact 0.1155). The time still grows only about linearly (22 s), since every cone stays at size 54. With the cut cones gone, the cross-step cuts don't constrain anything, and that costs more as steps accumulate.
- **Eventually the full PSD cone is preferable.** It stays exact at K = 4. `full_psd_cone=True` is slow (141 s at K = 3), but Clarabel's own decomposition of the same cone reaches K = 4 in 97 s.
  - **Recommendation:** the cut-cones-dropped variant for short horizons, and the full cone through Clarabel's decomposition once K grows.
- **Chordal, cut cones kept** (the default) still blows up: 104 s at K = 2, and it didn't finish K = 3 within 10 minutes. The hull rows don't change its cone structure.
- **Chordal, RLT only** is exact up to K = 2 but 9% loose at K = 3, against 148% before.

### Box, full PSD cone

| K | MIP | full PSD cone | SDP / MIP (norm) |
|---:|---:|---:|---:|
| 1 | 5.557 | 5.5568 | 1.00 (was 1.04) |
| 2 | 2.417 | 2.4172 | 1.00 |
| 3 | 1.458 | 1.4582 | 1.00 |

Solve times: 3.1 s, 23 s, 108 s for K = 1, 2, 3.

It is now exact at every K. Before the hull rows, K = 1 was 6.044 (4% loose in norm terms).

### Clarabel's own chordal decomposition (simplex, K = 1 to 4)

This uses the chordal cut set, with RLT only on the pairs in fomtools' per-step cones and no per-cut cones. It is passed to Clarabel with a single PSD cone over $X$, and Clarabel's built-in decomposition and clique-graph merging produce the cones (on by default in Clarabel 0.11.1). This is a patch of the solve path that now lives in `lasso/lasso_verify/clarabel_dual.py`; fomtools is unchanged in this respect.

To be equivalent to the full cone, the problem is solved through its **dual**. A zero row in Clarabel's PSD slack fixes that entry to 0, so simply dropping unused entries of $X$ would restrict it rather than leave it free. In the dual, the PSD slack is $D^{-1}(q + A^\top y)$, whose unused entries really are zero, and $X$ is Clarabel's dual variable, which lies in the PSD-completable cone. Checked on the full-cone problem at K = 1: 0.7785 through the dual against 0.7749 through the normal path (before the hull rows).

Squared 2-norm. The ratio is SDP against the sample max over 10,000 samples, in norm terms.

| K | sample max | SDP | SDP / sample max (norm) | Clarabel status | size of $X$ | pattern entries used |
|---:|---:|---:|---:|---|---:|---:|
| 1 | 0.656 | 0.6573 | 1.00 (was 1.09) | Solved | 57 | 1,551 of 1,653 |
| 2 | 0.240 | 0.2412 | 1.00 (was 1.03) | Solved | 91 | 3,078 of 4,186 |
| 3 | 0.168 | 0.1683 | 1.00 (was 1.01) | Solved | 125 | 4,965 of 7,875 |
| 4 | 0.115 | 0.1155 | 1.00 (was 1.01) | AlmostSolved | 159 | 7,212 of 12,720 |

Solve times:

| K | now | before the hull rows |
|---:|---:|---:|
| 1 | 2.7 s | 3.5 s |
| 2 | 11 s | 14 s |
| 3 | 37 s | 44 s |
| 4 | 97 s | 90 s |

- **It matches the sample max to within 0.3% at every K,** so it is effectively exact through K = 4. It is also slightly faster than before up to K = 3, and 4x faster than the full cone at K = 3 (37 s against 141 s).
- **Solves now end "Solved" up to K = 3.** Before the hull rows, every solve ended "AlmostSolved".
- **The merged Clarabel cone sizes were not recorded in this rerun.** The pattern is unchanged, so they should match the earlier run: 23, 54 at K = 1; 23, 54, 66 at K = 2; 23, 54, 54, 87 at K = 3; 6 cones with the largest 102 at K = 4.
- **K = 5 was not rerun.** Before, it didn't finish inside a 5-minute cap on the whole run. Its $X$ is 193×193 with 9,819 of 18,721 entries used, and its largest merged cone was 123.
- **The largest cone grows about linearly with K** (54 → 66 → 87 → 102 → 123), because the cross-step cuts keep connecting every step into one clique. Keeping the pattern banded, for example with cross cuts only between adjacent steps, would be needed to scale further.

### Why the cost grows

The number of cuts grows about linearly in K. What makes the chordal version expensive is the number of PSD cones. The cut counts include the hull rows' inequality cuts (`_build_ineq_cuts`: 285, 570 and 855 for all five cuts at K = 1, 2, 3).

| | K = 1 | K = 2 | K = 3 |
|---|---:|---:|---:|
| cuts, RLT only | 6,037 | 11,150 | 16,263 |
| cuts, all five (chordal) | 6,199 | 11,815 | 17,772 |
| cuts, all five (full cone) | 6,607 | 17,255 | 32,868 |
| cones, RLT only (sizes) | 23, 54 | 23, 54, 54 | 23, 54, 54, 54 |
| cones, all five (chordal) | 2 | 16 (12 of size 42) | 43 (36 of size 42) |
| PSD entries, RLT only | 1,761 | 3,246 | 4,731 |
| PSD entries, all five (chordal) | 1,761 | 15,117 | 40,344 |
| full cone size | 57 | 91 | 125 |

- **RLT is still most of the cuts** (about 87%). It adds 4 cuts per pair of variables in a common cone, and each ADMM step adds one cone of size 54.
- **The hull rows add about 5% more cuts.** They don't change the cones, so the PSD entries are as before.
- **The cross and subgradient cuts each register their own PSD cone** so that their cross-step products bind. Those overlapping cones take the PSD entries from 4,731 to 40,344 at K = 3.
- **Without those cones, a cross-step cut is valid but vacuous.** Entries of $X$ outside every cone are free variables in the Clarabel formulation. With the hull rows, though, dropping the cones costs almost nothing in quality (0.1687 against 0.1684 at K = 3).

### Package changes

In fomtools `b63c275`:

1. **Triangle and ReLU-envelope cuts use the OBBT bounds.** Before, `_build_triangle_cuts` and `_build_relu_envelope_cuts` recomputed the ReLU input bounds by interval propagation from the leaf boxes, which can't see the simplex budget. They should use the intersection of `step.lb/ub` with the propagated bounds.
2. **OBBT reaches affine step outputs.** Before, `Affine.sync_output_bounds` recomputed output bounds by interval propagation through $A^{-1}\,\mathrm{arg}$ and discarded the OBBT-tightened input bounds. This affected the linear-solve output and the trajectory leaves $s^k$, and tightening them narrows their summed box widths 3–4x.
3. **Small crossings in OBBT results are repaired.** A crossing of about 2e-17 made a later `stack` raise "Lower bound cannot exceed upper bound".

On branch `vr/tightening` (uncommitted):

4. **Hull rows for the composite soft-threshold and satlin steps** (`SDPStepDecomposer._add_hull_rows`). Each step registers the convex hull of $v \mapsto z$ over its OBBT-tightened input bounds as one inequality constraint. For soft-threshold that is the two lines $-\lambda \le v - z \le \lambda$ and two chords; for satlin, the two chords, since the box normal-cone cuts already enforce $l \le z \le u$.

## Minor

The Gurobi OBBT instance ignores `params={"OutputFlag": 0}` and prints a full log for every tightening LP. It does not affect results, but it buries the output.
