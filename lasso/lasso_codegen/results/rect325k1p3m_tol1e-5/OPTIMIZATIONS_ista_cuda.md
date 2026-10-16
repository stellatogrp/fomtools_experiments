# Optimization report: ista / cuda (m325000_n1300000_k8, tolerance 1e-5)

Fresh-build head-to-head on della-i12g2, mean per solve over 5 solves after 8 warm-ups:

| solver | mean_us |
|---|---:|
| Phase A | 125083.594000 |
| Phase B | 124997.618200 |

Kept: **phase_b** (Phase-A / Phase-B time = 1.001). Final verification:

```
worst max-abs error over 8 samples: 8.704e-14 (tol 1.000e-05)
```

## Diff (Phase A -> kept)

```diff
--- phase_a_solver.cu	2026-10-15 15:51:48.730195000 -0400
+++ src/fom_solver.cu	2026-10-15 16:00:34.862090000 -0400
@@ -50,8 +50,43 @@
       slower than the fused one.

    What is left on the critical path per iteration is exactly two kernels:
-   the mandated SpMV (~603 us, 95% of the solve) and the fused
+   the mandated SpMV (~604 us, 95% of the solve) and the fused
    activation+reseed (~27 us for 41.6 MB, i.e. at the DRAM roofline).
+
+   Why there is nothing material left.  `cuobjdump -res-usage fom_ops.o`
+   reports REG:32 SHARED:0 for `fom_apply_t0_state_kern`, so at its fixed
+   <<<5079,256>>> launch it already runs at full 64-warp occupancy and uses
+   no shared memory -- occupancy, block shape and L1/shared carveout are all
+   saturated or irrelevant.  Its SASS inner loop is
+   LDG.CONSTANT colidx -> LDG.CONSTANT vals -> dependent LDG.64 x[col] -> DFMA,
+   unrolled by four.  Per warp instruction that is ~16 (colidx, 60 B stride),
+   ~31 (vals, 120 B stride) and 32 (random gather) distinct 128 B lines, i.e.
+   ~79 address wavefronts for 32 useful doubles: the kernel is bound by L1/L2
+   sector throughput, not by DRAM (it moves ~260 MB in 604 us = 430 GB/s,
+   28% of peak).  None of that is reachable from this file, and the measured
+   knobs that are reachable all came out flat (see the list below).
+
+   Knobs measured on an A100 at 20 solves and rejected as noise (baseline
+   mean 124936 us / min 124524 us):
+     - accessPolicyWindow.missProp = Streaming (mark the CSR stream and the
+       accumulator evict-first so they cannot displace the pinned state):
+       mean 124908 us, min 124616 us.
+     - cudaFuncAttributePreferredSharedMemoryCarveout = MaxL1 plus
+       cudaFuncCachePreferL1 on the SpMV kernel symbol: mean 124861 us,
+       min 124533 us -- the driver default is already max L1 for a kernel
+       with SHARED:0, and it would have coupled this file to an internal
+       symbol name in the fixed fom_ops.cu for nothing.
+   Earlier sessions measured and rejected: overlapping work on a helper
+   stream (any concurrent kernel while the window is active costs 2x),
+   widening the window to 20.8 MB (starves the CSR stream, 172 vs 125 ms),
+   re-pointing the window per iteration (serialises stream 0), and splitting
+   the fused activation into threshold + copy (~25% slower).  CUDA graphs
+   are pointless here: each iteration is ~622 us of GPU work against a ~3 us
+   CPU enqueue, so the launch queue never runs dry.
+
+   Early exit on convergence is deliberately NOT done: the spec fixes
+   max_iters = 200 with tol = null (FOM_USE_TOL 0), so stopping early would
+   be fitting the verifier's 1e-5 tolerance rather than running the method.
 */

 #include <cuda_runtime.h>
```

## Agent summary (end of the code-optimization transcript)

The summary below is the agent's own report, quoted verbatim. Its numbers, for example the
L2 window's 150.1 -> 125.4 ms, are the agent's measurements, partly from earlier sessions,
and were not re-measured in this run.

### Whole-solve timing

| | mean_us | min_us |
|---|---:|---:|
| initial (solver as handed to this phase) | 124936 | 124524 |
| final | 124832 | 124535 |
| **speedup** | **1.00x** | — |

This phase found no further speedup: the solver was already at the hardware
floor when this round started, and every new knob tried came out inside
measurement noise. Earlier rounds on this same target took it from 160.6 ms to
124.2 ms (1.29x), and from 150.1 ms with the L2 window disabled (1.21x from
that optimization alone); those numbers are from prior sessions on the same
A100, not re-measured here. All figures are 20-solve runs; `min_us` is the
contention-robust statistic on this shared node.

### Final profiling breakdown

`make profile BENCH_SOLVES=10` (profile build inserts a device sync per region,
so its 127230 us mean is ~2% above the 124832 us production mean):

| region | total_us (10 solves) | per iteration | pct |
|---|---:|---:|---:|
| `spmv_t0_state` | 1209402 | 604.7 us | 95.07% |
| `threshold_reseed` | 54096 | 27.2 us | 4.25% |
| `download` | 5687 | — (once/solve) | 0.45% |
| `upload` | 2567 | — (once/solve) | 0.20% |
| `soft_threshold` | 197 | — (last iter only) | 0.02% |
| `residual` | 185 | — (last iter only) | 0.01% |

95% of the solve is a single kernel in the immutable `fom_ops.cu`.

### Optimizations in the final solver, and the structure each exploits

1. **Hoist `t0_p_p0 @ p0` out of the loop** into `d_pconst`.
   *Structure:* that term reads only the parameter block, never the state, so
   it is loop-invariant. Saves a 2.6M-nnz SpMV per iteration.

2. **Drop the state-update SpMV entirely.** `fom_setup` applies `su_t_t0` to
   two unrelated hashed probe vectors and demands a bitwise-zero difference
   each time; the activation then writes straight into the state buffer.
   *Structure:* `su_t_t0` is 1300000x1300000 with exactly 1300000 nonzeros, so
   each row holds at most one entry `a` at some column `j` and returns
   `a*r[j]`; that can equal `r[i]` for two independent probes only if `j == i`
   and `a == 1`. The probe is what makes the elision valid — the generic
   ping-pong path still exists as a fallback if it ever disagrees. Removes a
   1.3M-nnz SpMV and a 10.4 MB zero-fill per iteration.

3. **Pin the state vector in L2** with a persisting access-policy window
   (10.4 MB window inside a 16 MB set-aside), set once on the default stream.
   *Structure:* eliding the state update removes the need to ping-pong, so the
   state lives at ONE fixed device address for the whole solve — which is the
   precondition for a persisting window. The SpMV gathers that vector 19.5M
   times at random while streaming ~234 MB of CSR past it, which would
   otherwise evict it every pass. Worth ~17% (150.1 -> 125.4 ms). Set-aside
   size was swept: 12-17 MB is a flat optimum.

4. **Seed the accumulator with `pconst` instead of zeroing it.**
   *Structure:* the mandated helpers are accumulating (`out += M @ x`), so the
   SpMV's own `+=` performs the parameter add for free — no separate add pass.

5. **Fuse activation and reseed into one kernel** (`x = soft(acc)` and
   `acc = pconst` in a single pass, `double2` accesses, `__ldcs`/`__stwt` on
   the streaming operands).
   *Structure:* both passes touch the same 41.6 MB index space, but fusing
   issues two loads and two stores per element from one launch, keeping both
   DRAM directions busy. The split version moves identical bytes and measures
   ~25% slower. At ~1.9 TB/s effective this is at/above the DRAM roofline
   (part of the traffic is absorbed by the L2 window), so it has no headroom.

6. **Device-resident loop, pinned host staging.** `FOM_STATE_INIT` lives on the
   device; per solve the only PCIe traffic is the 2.6 MB parameter upload and
   one 10.4 MB solution download, over a single `cudaHostRegister` range
   covering `params.p0`, `solution.x` and `state` (they are contiguous in
   `FomWorkspace`; separate registrations collide because the host backs them
   with transparent huge pages). The redundant D2H mirror into `work->state`
   is dropped — nothing reads it.

#### Measured and rejected (documented in the file so they are not retried)

This round: `missProp = Streaming` (124908 us); max-L1 carveout +
`cudaFuncCachePreferL1` on the SpMV kernel symbol (124861 us) — the driver
default is already max L1 for a `SHARED:0` kernel, and keeping it would have
coupled the solver to an internal symbol name in the fixed `fom_ops.cu` for
nothing. CUDA graphs were ruled out by inspection: ~622 us of GPU work per
iteration against a ~3 us CPU enqueue means the launch queue never runs dry.
Earlier rounds: helper-stream overlap (any concurrent kernel while the window
is active costs 2x), a 20.8 MB window (starves the CSR stream, 172 vs 125 ms),
per-iteration window re-pointing (serialises stream 0).

#### Deliberately not done

Early exit on convergence. The iterate settles well inside the verifier's 1e-5
tolerance long before iteration 200, so stopping early would pass — but
`spec.json` fixes `max_iters = 200` with `tol = null` (`FOM_USE_TOL 0`), so
that would be fitting the verifier rather than running the specified method.

#### Why this is the floor

`cuobjdump -res-usage fom_ops.o` reports REG:32, SHARED:0 for
`fom_apply_t0_state_kern`, so at its fixed `<<<5079,256>>>` launch it already
runs at full 64-warp occupancy with no shared memory — occupancy, block shape
and L1/shared carveout are all saturated or irrelevant. Its SASS inner loop is
`LDG.CONSTANT colidx -> LDG.CONSTANT vals -> dependent LDG.64 x[col] -> DFMA`,
unrolled by four, costing ~16 + ~31 + 32 = ~79 distinct 128 B address
wavefronts per warp instruction for 32 useful doubles. The kernel is bound by
L1/L2 sector throughput, not DRAM: 260 MB in 604 us is 430 GB/s, 28% of peak.
That pattern is fixed by `fom_ops.cu` and the CSR layout, neither of which is
editable. With the SpMV at 95.07% and the one remaining elementwise pass at
the DRAM roofline, the whole solve is within ~3% of SpMV-only time.
