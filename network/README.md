# Network flow: verifying equilibrated PDHG in unscaled variables

PDHG and momentum PDHG on a min-cost network flow with gains, each verified with
and without two-sided Ruiz equilibration. Practical solvers such as PDLP
equilibrate internally but check termination on the original problem. This
experiment verifies the equilibrated methods the same way. The paper's appendix
gives the formulation, the iterations, and the residual map in full.

## Problem

$$
\begin{array}{ll}
\text{minimize} & \mu^\top z \\
\text{subject to} & A_s z \le b_s,\quad A_d z = x,\quad 0 \le z \le c.
\end{array}
$$

- **Gains:** one unit shipped on edge $e = (i, j)$ leaves supply node $i$ and
  delivers $g_e$ units at demand node $j$, so $A_s[i, e] = 1$ and
  $A_d[j, e] = -g_e$. The gains are log-uniform on $[0.1, 10]$, which is what makes
  the matrix badly scaled.
- **Instance (`results/run_005`):** $n_s = 10$ supply and $n_d = 15$ demand nodes. Each
  supply–demand edge is present with probability 0.3 (seed 285, 49 edges). Costs
  are $\mu_e \sim U[1, 10]$, capacities $c_e \sim U[2, 6]$, and the supply is
  $b_s = 6$ at each node.
- **Parameter:** the demand $x \in [-6, -4]^{15}$.
- **Initial iterate:** the primal-dual solution at the box centre $x = -5$.
- **Step:** $\eta = 0.8 / \lVert [A_s; A_d] \rVert_2$, computed from the matrix the
  method iterates on.

## Variants

| variant | momentum $\xi_k$ | scaling |
|---|---|---|
| `unscaled` | $0$ | none |
| `ruiz` | $0$ | Ruiz, 30 passes |
| `unscaled_momentum` | $k/(k+3)$ | none |
| `ruiz_momentum` | $k/(k+3)$ | Ruiz, 30 passes |

The equilibrated methods iterate on $E_s A_s D$, $E_d A_d D$, $D\mu$, $D^{-1}c$, and
$E_s b_s$. The parameter is not scaled, so every variant shares the same parameter
set and the same samples. The residual is mapped back through
$\mathbf{blkdiag}(D^{-1}, E_s^{-1}, E_d^{-1})$ before it enters the objective, so
for every variant the VP measures the KKT residual of the *original* LP and the
bounds are directly comparable. $E$ and $D$ are constants, so equilibration adds
no steps or variables to the VP.

## Results

At $K = 39$, equilibration lowers the worst-case residual of PDHG from 0.88 to
0.45, and that of momentum PDHG from 0.73 to 0.17 (`results/run_006`, the run behind
the paper's figure). The paper's figure reads
`data/network_residual.csv` in the paper repository, which `plots/export_paper_data.py`
writes from this run.

## Reproduce

The defaults in `conf/experiment/network.yaml` are the paper's instance and horizon
($K \le 39$; the paper's run is `results/run_006`, the archived one `results/run_005`), so no overrides are needed:

```sh
uv run python -m fomexp.run experiment=network      # the four variants into a new results/run_NNN
uv run python -m fomexp.collect network run_NNN     # summary.csv, wide.csv, summary.json
```
