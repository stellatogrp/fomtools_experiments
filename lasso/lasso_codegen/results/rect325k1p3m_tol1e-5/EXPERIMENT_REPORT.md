# Rectangular 325k by 1.3M sparse Lasso backend experiment, tolerance 1e-5

## Goal

Repeat the rect325k1p3m study (325,000 measurements, 1,300,000 decision
variables, 2,600,000 nonzeros in `A`) with verification tolerance `1e-5`
and FOMTools' built-in agent instructions, and time the resulting verified
ISTA and FISTA solvers for the OpenMP and CUDA targets.

## Method

The fixed problem uses seed 0, `lambda=0.02`, float64 arithmetic, and 200
iterations. Eight edge slots per row are built from two seeded permutations of
the 1,300,000 columns, the same generator as the rect10k40k study.

Generation reused stages 1 and 2 of rect325k1p3m: the extracted IRs,
the OpenMP and CUDA scaffolds with identical verification data (eight
reference solves), and the compiled deterministic objects (`nvcc 12.9`, native
`sm_80` only for CUDA). The only file that changed is the generated
verifier, whose tolerance is `1e-5`. The deterministic files were checksummed,
and the checksums were rechecked before every verification.

A coding agent then ran the translation and code optimization phases for all
four algorithm/backend pairs. Each phase used FOMTools' built-in
instructions (`phase_a_instructions` and `phase_b_instructions` from
`fomtools/codegen/agent/prompts.py`) and FOMTools' default agent driver with
its verify-and-reprompt loop, built as `generate_code` builds them
(`source/pipeline/fomtools_agent.py`). Every build, verification and
benchmark the agent requested ran inside a short Slurm job on an AMD EPYC
7H12 + A100 node. After code optimization, the translated and optimized
sources were each rebuilt from scratch and timed in the same job, and the
faster one was kept and re-verified.

The final benchmark rebuilt all four kept solvers from their verified
sources, re-verified them, recorded checksums, and then ran seven interleaved
rounds on `della-i12g2`. Each round was its own job pinned to that node and
checked the binaries against the recorded checksums. OpenMP used 16 threads
(`OMP_PLACES=cores`, `OMP_PROC_BIND=close`). Machine load and GPU use were
logged at the start of every round: the one-minute load was between 1.6 and
3.6 on 128 cores, and the GPU was idle each time.

The NumPy column is the FOMTools reference oracle on SciPy CSR matrices. It
evaluates the iteration as extracted, including the parameter product in every
iteration and, for FISTA, both 1.3M x 2.6M operators. The generated-code times
cover `fom_solve` only; parameter upload in `fom_update_params` is excluded.

## Results

| method | NumPy median (us/solve) | OpenMP median (us/solve) | CUDA median (us/solve) |
|---|---:|---:|---:|
| ISTA | 14812721.729 | 1177121.474 | 125099.056 |
| FISTA | 72812861.032 | 1245793.699 | 128049.853 |

| method | OpenMP speedup | CUDA speedup | CUDA vs OpenMP |
|---|---:|---:|---:|
| ISTA | 12.58x | 118.41x | 9.41x |
| FISTA | 58.45x | 568.63x | 9.73x |

Round-level ranges (us/solve):

| method | NumPy | OpenMP | CUDA |
|---|---|---|---|
| ISTA | 14775679-15020318 | 1169552-1179356 | 124998-125526 |
| FISTA | 72677110-73157535 | 1242271-1249459 | 127945-128200 |

All four final solvers passed the eight-sample generated-oracle check with a
`1e-5` tolerance:

| solver | worst max-absolute error |
|---|---:|
| ISTA OpenMP | 0.000e+00 |
| ISTA CUDA | 8.704e-14 |
| FISTA OpenMP | 1.403e-06 |
| FISTA CUDA | 1.377e-06 |

Code optimization, from the fresh-build head-to-head (milliseconds per
solve; 5 solves after 8 warm-ups; node in parentheses):

| solver | translation | code optimization | speedup |
|---|---:|---:|---:|
| ISTA OpenMP (della-i12g1) | 1209.232 | 1128.186 | 1.07x |
| ISTA CUDA (della-i12g2) | 125.084 | 124.998 | 1.00x |
| FISTA OpenMP (della-i12g2) | 5827.798 | 1254.083 | 4.65x |
| FISTA CUDA (della-i12g2) | 525.104 | 127.863 | 4.11x |

## Analysis

- Both FISTA agents used the identity `t0_state = [0, G]`,
  `t0_state__d_gamma = [-G, G]`. It moves the momentum from the operator onto
  the iterate, so one product with `G` replaces the two large products per
  iteration. Together with turning the state-update products into buffer
  swaps, this gives 4.65x (OpenMP) and 4.11x (CUDA) over the translations.
  FISTA now costs about the same per solve as ISTA on both backends.
- The two extracted copies of `G` differ by about `1e-15`. Over 200
  iterations that grows to about `1.4e-6` in the iterate, which the `1e-5`
  tolerance accepts and the `1e-6` tolerance of rect325k1p3m did not.
- Most of every ISTA iteration is the sparse matrix-vector product in the
  generated, checksummed operator file, which the agent may not edit. This
  caps the ISTA code-optimization gains (1.00x to 1.07x). The agents'
  profiles put the fixed product at about 93% (OpenMP) and 95% (CUDA) of the
  optimized solve.
- The OpenMP times depend on the node and on other jobs sharing its memory
  bandwidth. Code-optimization comparisons are therefore made within one job
  on one node, and every benchmark round is pinned to a single node. Absolute
  times from other nodes, including the `della-i12g1` times of
  rect325k1p3m, should not be compared with these.
