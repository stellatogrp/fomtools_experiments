"""Cluster driver for the verification experiments.

Each experiment folder holds a ``cluster_run.py`` adapter that turns a Hydra
config into the problem data and the per-k iteration; this package runs the K
loop, records every number the solves produce, and numbers the run folders.
"""
