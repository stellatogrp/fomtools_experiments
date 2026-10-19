# Large-scale Lasso: generated OpenMP and CUDA solvers

ISTA and FISTA for

$$
\min_{z}\; \frac{1}{2}\lVert D z - x \rVert_2^2 + 0.02\lVert z \rVert_1,
$$

with a sparse random $D \in \mathbf{R}^{10{,}000 \times 40{,}000}$ that has 260
nonzeros per row (2.6M in total), at 200 iterations per solve in double precision.
The paper's GPU table and agent-time table come from
[`results/rect10k40k_dense_sepA/`](results/rect10k40k_dense_sepA/), which holds the
instance, the raw timings, the agent reports, and the verification logs.

The iteration is written with the residual $r = D w - x$ as its own step, so the
extracted iteration multiplies by $D$ and $D^\top$ separately, as a hand-written
solver would. The precomputed $I - \eta D^\top D$ would be 34% dense here, with
212 times the nonzeros of $D$.

## Results

Seconds per 200-iteration solve, as medians over seven interleaved rounds on one AMD
EPYC 7H12 + A100 node. The hand-written baselines also keep $D$ and $D^\top$
separate: SciPy runs on one core, and CuPy's sparse products call cuSPARSE.

| method | SciPy (1 core) | generated OpenMP (16 threads) | speedup | CuPy | generated CUDA | speedup |
|---|---:|---:|---:|---:|---:|---:|
| ISTA | 1.250 | 0.0841 | 14.9x | 0.0478 | 0.0219 | 2.18x |
| FISTA | 1.264 | 0.0843 | 15.0x | 0.0559 | 0.0269 | 2.08x |

Agent wall-clock time, in minutes:

| solver | translation | code optimization | total |
|---|---:|---:|---:|
| ISTA OpenMP | 6.3 | 17.7 | 24.3 |
| ISTA CUDA | 8.0 | 30.4 | 38.8 |
| FISTA OpenMP | 8.1 | 20.6 | 29.1 |
| FISTA CUDA | 8.8 | 25.9 | 35.1 |

In the FISTA solvers the agent folded the two momentum products, which share a
block of $D$, into one product on the extrapolated point. In the ISTA CUDA solver
it relaunched the fixed CSR kernels with 435 blocks of 23 threads instead of 40
blocks of 256, so that more of the A100's 108 SMs receive work. All four solvers
pass the generated-oracle check at tolerance `1e-5`.

## Running

```bash
cd lasso
FOMTOOLS_CODEGEN_AGENT=1 uv run python -m lasso_codegen.generate
uv run python -m lasso_codegen.benchmark
```

## Other archived studies (not in the paper)

[`sparse20k`](results/sparse20k/), [`rect10k40k`](results/rect10k40k/),
[`rect325k1p3m`](results/rect325k1p3m/),
[`rect325k1p3m_tol1e-5`](results/rect325k1p3m_tol1e-5/),
[`rect325k1p3m_sepA`](results/rect325k1p3m_sepA/), and
[`paper_40x80`](results/paper_40x80/).
