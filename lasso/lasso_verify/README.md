# Lasso: verifying ISTA and FISTA on a nonstrongly convex instance

ISTA and FISTA on a sparse coding Lasso whose dictionary is wide, so the problem
is not strongly convex. The setup follows the sparse coding experiment of
[Ranjan et al.](https://arxiv.org/abs/2412.11330) (Example 2.4 and Sec. 4.1).
The instance is $10 \times 15$ with a sparse dictionary. The large-scale code
generation experiments are in [`../lasso_codegen/`](../lasso_codegen/).

## Problem

$$
\begin{array}{ll}
\text{minimize} & \tfrac12 \lVert Du - x \rVert_2^2 + \lambda \lVert u \rVert_1,
\end{array}
$$

with parameter $x$ and $\lambda = 0.1$.

- **Data:** $D \in \mathbf{R}^{10 \times 15}$ is a sparse Gaussian dictionary
  (seed 0), as in the paper:
  - Each entry is $N(0, 1/10)$ with probability 0.2 (`density` in
    `conf/experiment/lasso.yaml`) and 0 otherwise.
  - Every column and every row keeps at least one nonzero, so the columns can
    be normalized and no measurement is identically zero. Then the columns are
    normalized.
  - The draw has 32 nonzeros (1 to 6 per column). $D^\top D$ has rank 10, so
    the problem is not strongly convex.
- **Parameter set:** draw 100 sparse $z^\star$, each entry $N(0,1)$ with
  probability 0.1 and otherwise 0. $X$ is the tightest box containing the
  points $Dz^\star$.
- **Initial iterate:** $s^0$ is the min-norm least-squares solution
  ($\lambda = 0$) for one of those samples.
- **Step:** $\eta = 1/\lambda_{\max}(D^\top D) = 0.250$ for both methods.

## Methods and the residual map $H$

| method | iteration | state $s^k$ | $H$ |
|---|---|---|---|
| ISTA | $u^{k+1} = T_{\lambda\eta}((I - \eta D^\top D)u^k + \eta D^\top x)$ | $u^k$ | $-(D^\top D - \eta^{-1} I)$ |
| FISTA | $u^{k+1} = T_{\lambda\eta}((I - \eta D^\top D)w^k + \eta D^\top x)$, $w^{k+1} = u^{k+1} + \gamma_k(u^{k+1} - u^k)$ | $(u^k, u^{k-1})$ | $-(D^\top D - \eta^{-1} I)\,[I,\ -\gamma_{K-2} I]$ |

- **FISTA momentum:** $\gamma_k = (\beta_k - 1)/\beta_{k+1}$, where
  $\beta_0 = 1$ and $\beta_{k+1} = (1 + \sqrt{1 + 4\beta_k^2})/2$, with
  $w^0 = u^0$.
- **Why the two methods are comparable:** the metric is
  $\lVert H(s^K - s^{K-1})\rVert_\infty$. For both methods it equals the
  KKT residual at $u^K$:
  - The prox step gives
    $(w^{K-1} - u^K)/\eta - D^\top(Dw^{K-1} - x) \in \partial\lambda\lVert u^K\rVert_1$.
  - Adding $D^\top(Du^K - x)$ gives $(\eta^{-1} I - D^\top D)(w^{K-1} - u^K)$.

  This residual is an element of $\partial\lambda\lVert u^K\rVert_1 + D^\top(Du^K - x)$,
  so it is zero at a solution. Setting $\gamma \equiv 0$ in the FISTA formula
  recovers ISTA.
- **$K = 1, 2$ are identical:** $\gamma_0 = 0$, so FISTA equals ISTA for the
  first two iterations. Momentum first affects $u^3$.

## Verification

Each method runs through the cluster driver (`python -m fomexp.run`, settings in
`conf/`). See [`README_cluster.md`](../../README_cluster.md).

- **MIP:** Gurobi with Gurobi OBBT, on the ∞-norm, with `MIPGap` 5%, `MIPFocus`
  2 and a 3600 s limit per $K$ (16 threads). $K = 1, 2, \dots$ run on one
  verifier, so the worst case at $K-1$ warm-starts $K$.
  - The best of 10,000 samples is also given as a MIP start.
  - The reported bound is Gurobi's best bound (`ObjBound`). It is a certified
    upper bound on the worst case, even when a $K$ hits the time limit.
  - The loop stops when the next $K$ would overrun the 20 h budget.
- **Simulator:** 1,000 uniform samples of $x$ in $X$, cross-checked against
  the NumPy iteration in `problem.py`. The tables show the sample maximum.

## Results

Run `results/run_003`. **It is still running.** These numbers are a snapshot
from 2026-10-13, about 06:26 UTC: ISTA was on $K = 19$ and FISTA on $K = 10$.

| K | ISTA bound | ISTA incumbent | ISTA sample max | ISTA time | FISTA bound | FISTA incumbent | FISTA sample max | FISTA time |
|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| 1 | 2.2135 | 2.1911 | 1.8204 | 0.0 s | 2.2135 | 2.1911 | 1.8204 | 0.0 s |
| 2 | 1.4802 | 1.4209 | 1.3879 | 0.0 s | 1.4802 | 1.4209 | 1.3879 | 0.0 s |
| 3 | 1.1170 | 1.1103 | 1.0594 | 0.1 s | 1.0228 | 1.0048 | 0.9668 | 0.1 s |
| 4 | 0.8886 | 0.8504 | 0.8095 | 0.0 s | 0.6736 | 0.6736 | 0.6000 | 0.3 s |
| 5 | 0.6988 | 0.6832 | 0.6192 | 0.4 s | 0.4937 | 0.4760 | 0.3860 | 1.1 s |
| 6 | 0.5875 | 0.5867 | 0.4742 | 0.7 s | 0.4170 | 0.4122 | 0.2834 | 1.2 s |
| 7 | 0.5171 | 0.4928 | 0.3905 | 1.0 s | 0.3557 | 0.3406 | 0.2193 | 2.5 s |
| 8 | 0.4847 | 0.4633 | 0.3360 | 1.8 s | 0.2666 | 0.2554 | 0.1867 | 7.4 s |
| 9 | 0.4517 | 0.4348 | 0.2905 | 1.8 s | 0.2065 | 0.1967 | 0.1530 | 712.9 s |
| 10 | 0.4198 | 0.4011 | 0.2602 | 1.5 s | | | | |
| 11 | 0.3716 | 0.3701 | 0.2380 | 2.3 s | | | | |
| 12 | 0.3576 | 0.3414 | 0.2194 | 2.6 s | | | | |
| 13 | 0.3235 | 0.3132 | 0.1991 | 3.3 s | | | | |
| 14 | 0.2916 | 0.2796 | 0.1840 | 6.6 s | | | | |
| 15 | 0.2605 | 0.2510 | 0.1703 | 20.5 s | | | | |
| 16 | 0.2373 | 0.2262 | 0.1579 | 35.4 s | | | | |
| 17 | 0.2147 | 0.2045 | 0.1487 | 51.4 s | | | | |
| 18 | 0.2091 | 0.2015 | 0.1431 | 319.0 s | | | | |

Times are Gurobi's runtime for each $K$.

- **Every finished $K$ reached the 5% gap.** That is through $K = 18$ for ISTA
  and $K = 9$ for FISTA.
- **FISTA pulls ahead from $K = 3$.** At $K = 9$ its certified bound is 54%
  below ISTA's (0.207 vs 0.452).
- **Sampling underestimates the worst case.** The incumbent exceeds the sample
  max by up to 57% for ISTA (0.313 vs 0.199 at $K = 13$) and 29% for FISTA at
  $K = 9$.
- **FISTA gets hard first.** It needs under 10 s per $K$ through $K = 8$, then
  713 s at $K = 9$. ISTA stays under a minute through $K = 17$.
  - The relaxation is about as loose in absolute terms for both methods, but
    FISTA's worst case is smaller. At $K = 8$ the root bound is 2.2× the
    incumbent for FISTA and 1.5× for ISTA, so the same 5% relative gap takes
    far more branching (9,814 vs 1,260 nodes).
- **The sparse dictionary is what makes this size tractable.** The same size
  with a dense $D$ (`results/run_002`) needed 3,113 s at $K = 18$ for ISTA, and
  FISTA timed out from $K = 9$ (bound 0.280 at a 12% gap).

## Reproduce

```sh
# cluster (both methods, one array task each, into a new results/run_NNN)
bash slurm/submit.sh --exp lasso experiment.problem.m=10 experiment.problem.n=15

# or locally, e.g. a short check up to K = 6
.venv/bin/python -m fomexp.run experiment=lasso experiment.problem.m=10 experiment.problem.n=15 \
    schedule.k_max=6

# laptop, after ./sync.sh --pull lasso
uv run python -m fomexp.collect lasso run_003
```

`verify.py`, `check.py` and `plot.py` still build the earlier $3 \times 5$
instance (the module constants in `problem.py`), and write to `results/`.
