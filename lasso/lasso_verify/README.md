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
  (seed 0, density 0.2, every row and column nonzero, columns normalized).
  $D^\top D$ has rank at most 25, so the problem is not strongly convex.
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
- **Metric:** $\lVert H(s^K - s^{K-1})\rVert_\infty$, which for both methods is
  the KKT residual at $u^K$ (derivation in the paper's appendix). Since
  $\gamma_0 = 0$, the two methods coincide for $K \le 2$.

## Verification

Both methods run through `python -m fomexp.run` (settings in `conf/`).

- **MIP:** Gurobi on the ∞-norm, with a 5% gap and a 3600 s limit per $K$, on 16
  threads. $K = 1, 2, \dots$ run on one verifier, so the worst case at $K-1$
  warm-starts $K$. The reported bound is Gurobi's best bound (`ObjBound`).
- **Simulator:** 1,000 uniform samples of $x$ in $X$. The table shows the sample
  maximum.

## Results

Run `results/run_022`, the run behind the paper's figure, cut at $K = 20$.

| K | ISTA bound | ISTA incumbent | ISTA sample max | FISTA bound | FISTA incumbent | FISTA sample max |
|---:|---:|---:|---:|---:|---:|---:|
| 1 | 2.7697 | 2.7697 | 1.6138 | 2.7697 | 2.7697 | 1.6138 |
| 2 | 2.2282 | 2.2242 | 1.1094 | 2.2282 | 2.2242 | 1.1094 |
| 3 | 1.7381 | 1.7173 | 0.8587 | 1.6036 | 1.5745 | 0.8063 |
| 4 | 1.4452 | 1.4445 | 0.7260 | 1.2737 | 1.2432 | 0.6394 |
| 5 | 1.2749 | 1.2603 | 0.6292 | 1.0391 | 1.0390 | 0.4729 |
| 6 | 1.1422 | 1.1061 | 0.5468 | 0.8355 | 0.8233 | 0.3426 |
| 7 | 1.0262 | 0.9797 | 0.4765 | 0.6506 | 0.6254 | 0.2996 |
| 8 | 0.9324 | 0.9056 | 0.4169 | 0.5297 | 0.5283 | 0.2599 |
| 9 | 0.8523 | 0.8176 | 0.3759 | 0.4169 | 0.4149 | 0.2160 |
| 10 | 0.7764 | 0.7394 | 0.3401 | 0.3150 | 0.3124 | 0.1707 |
| 11 | 0.7070 | 0.7058 | 0.3083 | 0.2586 | 0.2535 | 0.1265 |
| 12 | 0.6460 | 0.6432 | 0.2841 | 0.2173 | 0.2097 | 0.1015 |
| 13 | 0.6028 | 0.5862 | 0.2701 | 0.1724 | 0.1642 | 0.0829 |
| 14 | 0.5647 | 0.5639 | 0.2565 | 0.1393 | 0.1365 | 0.0666 |
| 15 | 0.5312 | 0.5294 | 0.2433 | 0.1338 | 0.1281 | 0.0583 |
| 16 | 0.4990 | 0.4972 | 0.2306 | 0.1405 | 0.1339 | 0.0590 |
| 17 | 0.4688 | 0.4671 | 0.2183 | 0.1349 | 0.1294 | 0.0540 |
| 18 | 0.4407 | 0.4390 | 0.2066 | 0.1226 | 0.1172 | 0.0489 |
| 19 | 0.4144 | 0.4127 | 0.1954 | 0.1018 | 0.0970 | 0.0485 |
| 20 | 0.3898 | 0.3881 | 0.1847 | 0.0915 | 0.0878 | 0.0482 |

- **FISTA pulls ahead from $K = 3$.** From there on its bound lies below the best
  ISTA trajectory Gurobi finds. At $K = 20$ the FISTA bound is 0.092 and the ISTA
  incumbent is 0.388.
- **Every $K \le 20$ reached the 5% gap.**

The paper also compares these bounds with probabilistic bounds on the 95th
percentile, which `plots/kl_bounds.py` computes from the simulator samples.

## Reproduce

The defaults in `conf/experiment/lasso.yaml` are the paper's instance and horizon
($K \le 20$), so no overrides are needed:

```sh
uv run python -m fomexp.run experiment=lasso        # both methods into a new results/run_NNN
uv run python -m fomexp.collect lasso run_NNN       # summary.csv, wide.csv, summary.json
```

`summary.json` holds the numbers the paper quotes about the run (hours, largest
model, statuses).
