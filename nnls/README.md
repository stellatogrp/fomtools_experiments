# NNLS: verifying equilibrated PGD

Projected gradient descent on nonnegative least squares, with and without Ruiz
equilibration, verified with the MIP and SDP backends. This experiment is not in the
paper. Run it from this directory:
`uv run python verify.py && uv run python check.py && uv run python plot.py`.
