# Rectangular 325k by 1.3M sparse Lasso backend experiment

## Goal

Evaluate verified generated ISTA and FISTA solvers for large-scale OpenMP and
CUDA targets on a Lasso instance whose NumPy reference solve takes seconds to
a minute: 325,000 measurements, 1,300,000 decision variables, and 2,600,000
nonzeros in `A`.

## Method

The fixed problem uses seed 0, `lambda=0.02`, float64 arithmetic, and 200
iterations. Eight edge slots per row are built from two seeded permutations of
the 1,300,000 columns, the same generator as the rect10k40k study.

Generation ran in three stages:

1. A CPU job extracted both methods, scaffolded the OpenMP and CUDA targets
   with identical verification data (eight reference solves), compiled the
   deterministic OpenMP objects once, and link-tested full-size stub binaries.
2. A second CPU job compiled the deterministic CUDA objects (`nvcc 12.9`,
   native `sm_80` only).
3. A coding agent ran the translation and code optimization phases for all
   four algorithm/backend pairs. Every build, verification and benchmark
   it requested ran inside a short Slurm job on an AMD EPYC 7H12 + A100 node.
   After code optimization, the translated and optimized sources were each
   rebuilt from scratch and timed in the same job, and the faster one was kept
   and re-verified.

The final benchmark rebuilt all four kept solvers from their verified
sources, re-verified them, recorded checksums, and then ran seven interleaved
rounds on `della-i12g1`. Each round was its own job pinned to that node and
checked the binaries against the recorded checksums. OpenMP used 16 threads
(`OMP_PLACES=cores`, `OMP_PROC_BIND=close`). Machine load and GPU use were
logged at the start of every round: load was between 1.7 and 10.9 on 128
cores, and the GPU was idle each time.

The NumPy column is the FOMTools reference oracle on SciPy CSR matrices. It
evaluates the iteration as extracted, including the parameter product in every
iteration and, for FISTA, both 1.3M x 2.6M operators. The generated-code times
cover `fom_solve` only; parameter upload in `fom_update_params` is excluded.

## Results

| method | NumPy median (us/solve) | OpenMP median (us/solve) | CUDA median (us/solve) |
|---|---:|---:|---:|
| ISTA | 14953733.763 | 1210185.993 | 142472.195 |
| FISTA | 73375302.343 | 5219036.599 | 455149.881 |

| method | OpenMP speedup | CUDA speedup | CUDA vs OpenMP |
|---|---:|---:|---:|
| ISTA | 12.36x | 104.96x | 8.49x |
| FISTA | 14.06x | 161.21x | 11.47x |

Round-level ranges (us/solve):

| method | NumPy | OpenMP | CUDA |
|---|---|---|---|
| ISTA | 14826506-15030218 | 1207742-1212634 | 142438-142524 |
| FISTA | 72806257-73695969 | 5210362-5227673 | 455065-455390 |

All four final solvers passed the eight-sample generated-oracle check with a
`1e-6` tolerance:

| solver | worst max-absolute error |
|---|---:|
| ISTA OpenMP | 0.000e+00 |
| ISTA CUDA | 8.704e-14 |
| FISTA OpenMP | 2.100e-07 |
| FISTA CUDA | 2.000e-07 |

Code optimization, from the fresh-build head-to-head (milliseconds per
solve; 5 solves after 8 warm-ups; node in parentheses):

| solver | translation | code optimization | speedup |
|---|---:|---:|---:|
| ISTA OpenMP (della-i12g2) | 1464.288 | 1201.853 | 1.22x |
| ISTA CUDA (della-i12g2) | 159.652 | 142.242 | 1.12x |
| FISTA OpenMP (della-i14g1) | 10390.282 | 9065.550 | 1.15x |
| FISTA CUDA (della-i12g1) | 721.227 | 456.135 | 1.58x |

## Analysis

- OpenMP is a genuine 16-thread build (`-fopenmp`), rebuilt from the kept
  source immediately before timing. The rect10k40k OpenMP column was measured
  with a serial binary built from the translated (unoptimized) source; this study fixes both
  problems.
- Most of every iteration is the sparse matrix-vector products in the
  generated, checksummed operator files, which the agent may not edit. This
  caps the ISTA code-optimization gains (1.12x to 1.22x), as in rect10k40k.
- The FISTA agents found the identity `t0_state = [0, G]`,
  `t0_state__d_gamma = [-G, G]`. Exploiting it would replace two large
  products with one per iteration (2.9x to 3.2x at 10k by 40k). It was
  rejected here: the two extracted copies of `G` differ by about `1e-15`,
  and over 200 iterations that grows to about `1.3e-6`, just above the
  `1e-6` tolerance. Making the extraction emit bit-identical blocks would
  re-enable it.
- The same FISTA OpenMP binary took 6.11 s on `della-i12g2` and 10.39 s on
  the busier `della-i14g1` (both EPYC 7H12 + A100). Code-optimization
  comparisons are therefore made within one job on one node, and every benchmark round is
  pinned to a single node. Absolute times from other nodes should not be
  compared with these.

## Next steps

The 500,000 by 2,000,000 instance uses the same pipeline and benchmark node.
