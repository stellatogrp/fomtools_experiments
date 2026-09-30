# Checked-in result artifacts

`paper_final_compute_7rounds.txt` is the source for the headline table in
`../../README.md`. It contains seven interleaved measurements of NumPy, optimized
CPU C, and optimized CUDA for ISTA and FISTA.

`paper_final_optimized_7rounds.txt` is an independent seven-round repeat retained
as a reproducibility check. It gives the same qualitative and quantitative
conclusion; it is not pooled with the designated final run.

The four `OPTIMIZATIONS_*.md` files are the reports emitted by codegen Phase B.
They contain generation-time before/after measurements, source diffs, agent
notes, and the final verification statement. The four `verify_*.log` files are
the corresponding standalone verification outputs from the compute node.

`paper_benchmark_metadata.txt` is the original hardware capture. `manifest.json`
records the fixed problem and software parameters supplied during the run.

`summary.csv` is reproducible from the raw final file with:

```bash
uv run python -m lasso.summarize
```
