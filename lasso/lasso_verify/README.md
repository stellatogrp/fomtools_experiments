# Lasso: verifying ISTA and FISTA on a nonstrongly convex instance

ISTA and FISTA on a sparse coding Lasso whose dictionary is wide, so the problem
is not strongly convex. The setup follows the sparse coding experiment of
[Ranjan et al.](https://arxiv.org/abs/2412.11330) (Example 2.4 and Sec. 4.1).
The instance is $25 \times 50$ with a sparse dictionary. The large-scale code
generation experiments are in [`../lasso_codegen/`](../lasso_codegen/).

## Problem

$$
\begin{array}{ll}
\text{minimize} & \tfrac12 \lVert Du - x \rVert_2^2 + \lambda \lVert u \rVert_1,
\end{array}
$$

with parameter $x$ and $\lambda = 0.001$.

- **Data:** $D \in \mathbf{R}^{25 \times 50}$ is a sparse Gaussian dictionary
  (seed 0), as in the paper:
  - Each entry is $N(0, 1/25)$ with probability 0.2 (`density` in
    `conf/experiment/lasso.yaml`) and 0 otherwise.
  - Every column and every row keeps at least one nonzero, so the columns can
    be normalized and no measurement is identically zero. Then the columns are
    normalized.
  - $D^\top D$ has rank at most 25 < 50, so the problem is not strongly convex.
- **Parameter set:** draw 100 sparse $z^\star$, each entry $N(0,1)$ with
  probability 0.1 and otherwise 0. $X$ is the tightest box containing the
  points $Dz^\star$.
- **Initial iterate:** $s^0$ is the min-norm least-squares solution
  ($\lambda = 0$) for one of those samples.
- **Step:** $\eta = 1/\lambda_{\max}(D^\top D) = 0.221$ for both methods.

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
`conf/`). See [Running experiments](../../README.md#running-experiments) in the
root README.

- **MIP:** Gurobi with Gurobi OBBT, on the ∞-norm, with `MIPGap` 5%, `MIPFocus`
  2 and a 3600 s limit per $K$ (16 threads). $K = 1, 2, \dots$ run on one
  verifier, so the worst case at $K-1$ warm-starts $K$.
  - The best of 10,000 samples is also given as a MIP start.
  - The reported bound is Gurobi's best bound (`ObjBound`). It is a certified
    upper bound on the worst case, even when a $K$ hits the time limit.
  - The loop stops when the next $K$ would overrun the 20 h budget. The paper
    reports $K \le 20$.
- **Simulator:** 1,000 uniform samples of $x$ in $X$, cross-checked against
  the NumPy iteration in `problem.py`. The tables show the sample maximum.

## Results

Run `results/run_022`, the run behind the paper's figure, cut at $K = 20$.

| K | ISTA bound | ISTA incumbent | ISTA sample max | ISTA time | FISTA bound | FISTA incumbent | FISTA sample max | FISTA time |
|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| 1 | 2.7697 | 2.7697 | 1.6138 | 1.8 s | 2.7697 | 2.7697 | 1.6138 | 1.9 s |
| 2 | 2.2282 | 2.2242 | 1.1094 | 0.1 s | 2.2282 | 2.2242 | 1.1094 | 0.1 s |
| 3 | 1.7381 | 1.7173 | 0.8587 | 45.5 s | 1.6036 | 1.5745 | 0.8063 | 1.0 s |
| 4 | 1.4452 | 1.4445 | 0.7260 | 5.5 s | 1.2737 | 1.2432 | 0.6394 | 14.8 s |
| 5 | 1.2749 | 1.2603 | 0.6292 | 0.6 s | 1.0391 | 1.0390 | 0.4729 | 27.4 s |
| 6 | 1.1422 | 1.1061 | 0.5468 | 2.8 s | 0.8355 | 0.8233 | 0.3426 | 5.5 s |
| 7 | 1.0262 | 0.9797 | 0.4765 | 39.1 s | 0.6506 | 0.6254 | 0.2996 | 23.8 s |
| 8 | 0.9324 | 0.9056 | 0.4169 | 29.5 s | 0.5297 | 0.5283 | 0.2599 | 55.9 s |
| 9 | 0.8523 | 0.8176 | 0.3759 | 1.8 s | 0.4169 | 0.4149 | 0.2160 | 5.7 s |
| 10 | 0.7764 | 0.7394 | 0.3401 | 11.4 s | 0.3150 | 0.3124 | 0.1707 | 12.3 s |
| 11 | 0.7070 | 0.7058 | 0.3083 | 48.4 s | 0.2586 | 0.2535 | 0.1265 | 467.6 s |
| 12 | 0.6460 | 0.6432 | 0.2841 | 496.1 s | 0.2173 | 0.2097 | 0.1015 | 8.4 s |
| 13 | 0.6028 | 0.5862 | 0.2701 | 16.6 s | 0.1724 | 0.1642 | 0.0829 | 120.3 s |
| 14 | 0.5647 | 0.5639 | 0.2565 | 173.7 s | 0.1393 | 0.1365 | 0.0666 | 483.2 s |
| 15 | 0.5312 | 0.5294 | 0.2433 | 12.5 s | 0.1338 | 0.1281 | 0.0583 | 687.7 s |
| 16 | 0.4990 | 0.4972 | 0.2306 | 8.6 s | 0.1405 | 0.1339 | 0.0590 | 61.8 s |
| 17 | 0.4688 | 0.4671 | 0.2183 | 7.6 s | 0.1349 | 0.1294 | 0.0540 | 1303.7 s |
| 18 | 0.4407 | 0.4390 | 0.2066 | 8.9 s | 0.1226 | 0.1172 | 0.0489 | 795.4 s |
| 19 | 0.4144 | 0.4127 | 0.1954 | 10.3 s | 0.1018 | 0.0970 | 0.0485 | 921.4 s |
| 20 | 0.3898 | 0.3881 | 0.1847 | 12.7 s | 0.0915 | 0.0878 | 0.0482 | 3249.3 s |

Times are Gurobi's runtime for each $K$.

- **Every $K \le 20$ reached the 5% gap.** ISTA continued within the gap through
  $K = 30$. FISTA hit the time limit at $K = 21$ and $22$, where the bound is valid
  but loose.
- **FISTA pulls ahead from $K = 3$.** At $K = 20$ its certified bound is 77%
  below ISTA's (0.092 vs 0.390).
- **Sampling underestimates the worst case.** The incumbent is 1.7 to 2.3 times
  the sample max for ISTA and 1.7 to 2.4 times for FISTA.
- **FISTA gets hard first.** ISTA needs at most 496 s for any $K \le 20$ (933 s
  in total). FISTA needs 3,249 s at $K = 20$ (8,247 s in total).

## Reproduce

```sh
# both methods, one after the other, into a new results/run_NNN
uv run python -m fomexp.run experiment=lasso experiment.problem.m=25 experiment.problem.n=50 \
    experiment.problem.lambd=0.001

# a short check up to K = 6
uv run python -m fomexp.run experiment=lasso experiment.problem.m=25 experiment.problem.n=50 \
    experiment.problem.lambd=0.001 schedule.k_max=6

# merge the per-variant CSVs
uv run python -m fomexp.collect lasso run_NNN
```

`verify.py`, `check.py` and `plot.py` still build the earlier $3 \times 5$
instance (the module constants in `problem.py`), and write to `results/`.
