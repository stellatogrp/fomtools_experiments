# Optimization report

## Timing

```
translated (emitted)     110.7 us/solve
optimized                 42.6 us/solve
speedup                  2.60x
```

Optimization outcome: **kept (re-verified correct)**.

## Rounds

| round | before (us) | after (us) | gain | kept | agent (s) | outcome |
|---:|---:|---:|---:|:---:|---:|---|
| 1 | 110.748 | 73.962 | 33.2% | yes | 200 | continued |
| 2 | 76.694 | 50.321 | 34.4% | yes | 343 | continued |
| 3 | 48.927 | 42.630 | 12.9% | yes | 156 | continued |
| 4 | - | - | - | no | 193 | the round left the sources unchanged |

## Kernel checks

Each `fom_apply_<name>` in the shipped `fom_ops.c`, checked by `make verify` against the package's reference over the same `fom_data.c` matrix. *basis* probes every unit vector, which recovers the routine's matrix column by column (an exact check), plus random vectors; *random* uses random vectors only (the matrix is over the probe budget). A `fused_<block>` row is a `fom_fused_<block>`, checked against the sum of its members: *basis* there probes every unit vector of each source with every schedule value at 0 and with each alone at 1, which recovers every member exactly.

| matrix | shape | nnz | rewritten | check | probes | max error |
|---|---|---|---|---|---|---|
| `t0_state` | 600x650 | 1800 | yes | basis | 652 | 0.000e+00 |
| `t1_state` | 20x650 | 620 | yes | basis | 652 | 0.000e+00 |
| `t1_t_t0` | 20x600 | 600 | yes | basis | 602 | 4.441e-16 |
| `t2_state` | 600x650 | 12600 | yes | basis | 652 | 0.000e+00 |
| `t2_p_p0` | 600x30 | 600 | yes | basis | 32 | 0.000e+00 |
| `t2_t_t0` | 600x600 | 12000 | yes | basis | 602 | 0.000e+00 |
| `t2_t_t1` | 600x20 | 600 | yes | basis | 22 | 0.000e+00 |
| `t3_t_t0` | 20x600 | 600 | yes | basis | 602 | 4.441e-16 |
| `t3_t_t1` | 20x20 | 20 | yes | basis | 22 | 0.000e+00 |
| `t3_t_t2` | 20x600 | 600 | yes | basis | 602 | 4.441e-16 |
| `t4_state` | 30x650 | 630 | yes | basis | 652 | 0.000e+00 |
| `t4_p_p0` | 30x30 | 30 | yes | basis | 32 | 0.000e+00 |
| `t4_t_t0` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t4_t_t2` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t5_t_t2` | 600x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t5_t_t4` | 600x30 | 600 | yes | basis | 32 | 0.000e+00 |
| `t6_t_t2` | 20x600 | 600 | yes | basis | 602 | 4.441e-16 |
| `t6_t_t5` | 20x600 | 600 | yes | basis | 602 | 8.882e-16 |
| `t7_p_p0` | 30x30 | 30 | yes | basis | 32 | 0.000e+00 |
| `t7_t_t2` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t7_t_t4` | 30x30 | 30 | yes | basis | 32 | 0.000e+00 |
| `t7_t_t5` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t9_t_t5` | 20x600 | 600 | yes | basis | 602 | 4.441e-16 |
| `t9_t_t8` | 20x600 | 600 | yes | basis | 602 | 8.882e-16 |
| `t10_t_t5` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t10_t_t8` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t12_t_t8` | 20x600 | 600 | yes | basis | 602 | 4.441e-16 |
| `t12_t_t11` | 20x600 | 600 | yes | basis | 602 | 4.441e-16 |
| `t13_t_t8` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t13_t_t11` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t15_t_t11` | 20x600 | 600 | yes | basis | 602 | 8.882e-16 |
| `t15_t_t14` | 20x600 | 600 | yes | basis | 602 | 8.882e-16 |
| `t16_t_t11` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t16_t_t14` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t18_t_t14` | 20x600 | 600 | yes | basis | 602 | 8.882e-16 |
| `t18_t_t17` | 20x600 | 600 | yes | basis | 602 | 8.882e-16 |
| `t19_t_t14` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t19_t_t17` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t21_t_t17` | 20x600 | 600 | yes | basis | 602 | 8.882e-16 |
| `t21_t_t20` | 20x600 | 600 | yes | basis | 602 | 8.882e-16 |
| `t22_t_t17` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t22_t_t20` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t24_t_t20` | 20x600 | 600 | yes | basis | 602 | 4.441e-16 |
| `t24_t_t23` | 20x600 | 600 | yes | basis | 602 | 8.882e-16 |
| `t25_t_t20` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t25_t_t23` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t27_t_t23` | 20x600 | 600 | yes | basis | 602 | 4.441e-16 |
| `t27_t_t26` | 20x600 | 600 | yes | basis | 602 | 8.882e-16 |
| `t28_t_t23` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t28_t_t26` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t30_t_t26` | 20x600 | 600 | yes | basis | 602 | 4.441e-16 |
| `t30_t_t29` | 20x600 | 600 | yes | basis | 602 | 8.882e-16 |
| `t31_t_t26` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t31_t_t29` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t33_t_t29` | 20x600 | 600 | yes | basis | 602 | 8.882e-16 |
| `t33_t_t32` | 20x600 | 600 | yes | basis | 602 | 8.882e-16 |
| `t34_t_t29` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t34_t_t32` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t36_t_t32` | 20x600 | 600 | yes | basis | 602 | 4.441e-16 |
| `t36_t_t35` | 20x600 | 600 | yes | basis | 602 | 8.882e-16 |
| `t37_t_t32` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t37_t_t35` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t39_t_t35` | 20x600 | 600 | yes | basis | 602 | 8.882e-16 |
| `t39_t_t38` | 20x600 | 600 | yes | basis | 602 | 8.882e-16 |
| `t40_t_t35` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t40_t_t38` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t42_t_t38` | 20x600 | 600 | yes | basis | 602 | 8.882e-16 |
| `t42_t_t41` | 20x600 | 600 | yes | basis | 602 | 8.882e-16 |
| `t43_t_t38` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t43_t_t41` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t45_t_t41` | 20x600 | 600 | yes | basis | 602 | 4.441e-16 |
| `t45_t_t44` | 20x600 | 600 | yes | basis | 602 | 8.882e-16 |
| `t46_t_t41` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t46_t_t44` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t48_t_t44` | 20x600 | 600 | yes | basis | 602 | 4.441e-16 |
| `t48_t_t47` | 20x600 | 600 | yes | basis | 602 | 8.882e-16 |
| `t49_t_t44` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t49_t_t47` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t51_t_t47` | 20x600 | 600 | yes | basis | 602 | 6.661e-16 |
| `t51_t_t50` | 20x600 | 600 | yes | basis | 602 | 8.882e-16 |
| `t52_t_t47` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t52_t_t50` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t54_t_t50` | 20x600 | 600 | yes | basis | 602 | 8.882e-16 |
| `t54_t_t53` | 20x600 | 600 | yes | basis | 602 | 4.441e-16 |
| `t55_t_t50` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t55_t_t53` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t57_t_t53` | 20x600 | 600 | yes | basis | 602 | 8.882e-16 |
| `t57_t_t56` | 20x600 | 600 | yes | basis | 602 | 8.882e-16 |
| `t58_t_t53` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t58_t_t56` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t60_t_t56` | 20x600 | 600 | yes | basis | 602 | 8.882e-16 |
| `t60_t_t59` | 20x600 | 600 | yes | basis | 602 | 8.882e-16 |
| `t61_t_t56` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t61_t_t59` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t63_t_t59` | 20x600 | 600 | yes | basis | 602 | 4.441e-16 |
| `t63_t_t62` | 20x600 | 600 | yes | basis | 602 | 4.441e-16 |
| `t64_t_t59` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t64_t_t62` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t66_t_t62` | 20x600 | 600 | yes | basis | 602 | 8.882e-16 |
| `t66_t_t65` | 20x600 | 600 | yes | basis | 602 | 4.441e-16 |
| `t67_t_t62` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t67_t_t65` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t69_t_t65` | 20x600 | 600 | yes | basis | 602 | 8.882e-16 |
| `t69_t_t68` | 20x600 | 600 | yes | basis | 602 | 8.882e-16 |
| `t70_t_t65` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t70_t_t68` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t72_t_t68` | 20x600 | 600 | yes | basis | 602 | 8.882e-16 |
| `t72_t_t71` | 20x600 | 600 | yes | basis | 602 | 8.882e-16 |
| `t73_t_t68` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t73_t_t71` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t75_t_t71` | 20x600 | 600 | yes | basis | 602 | 8.882e-16 |
| `t75_t_t74` | 20x600 | 600 | yes | basis | 602 | 8.882e-16 |
| `t76_t_t71` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t76_t_t74` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t78_t_t74` | 20x600 | 600 | yes | basis | 602 | 8.882e-16 |
| `t78_t_t77` | 20x600 | 600 | yes | basis | 602 | 8.882e-16 |
| `t79_t_t74` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t79_t_t77` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t81_t_t77` | 20x600 | 600 | yes | basis | 602 | 8.882e-16 |
| `t81_t_t80` | 20x600 | 600 | yes | basis | 602 | 1.776e-15 |
| `t82_t_t77` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t82_t_t80` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t84_t_t80` | 20x600 | 600 | yes | basis | 602 | 1.776e-15 |
| `t84_t_t83` | 20x600 | 600 | yes | basis | 602 | 8.882e-16 |
| `t85_t_t80` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t85_t_t83` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t87_t_t83` | 20x600 | 600 | yes | basis | 602 | 8.882e-16 |
| `t87_t_t86` | 20x600 | 600 | yes | basis | 602 | 8.882e-16 |
| `t88_t_t83` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t88_t_t86` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t90_t_t86` | 20x600 | 600 | yes | basis | 602 | 8.882e-16 |
| `t90_t_t89` | 20x600 | 600 | yes | basis | 602 | 8.882e-16 |
| `t91_t_t86` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t91_t_t89` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t93_t_t89` | 20x600 | 600 | yes | basis | 602 | 8.882e-16 |
| `t93_t_t92` | 20x600 | 600 | yes | basis | 602 | 8.882e-16 |
| `t94_t_t89` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t94_t_t92` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t96_t_t92` | 20x600 | 600 | yes | basis | 602 | 8.882e-16 |
| `t96_t_t95` | 20x600 | 600 | yes | basis | 602 | 1.332e-15 |
| `t97_t_t92` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t97_t_t95` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t99_t_t95` | 20x600 | 600 | yes | basis | 602 | 8.882e-16 |
| `t99_t_t98` | 20x600 | 600 | yes | basis | 602 | 4.441e-16 |
| `t100_t_t95` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t100_t_t98` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t102_t_t98` | 20x600 | 600 | yes | basis | 602 | 4.441e-16 |
| `t102_t_t101` | 20x600 | 600 | yes | basis | 602 | 8.882e-16 |
| `t103_t_t98` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t103_t_t101` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t105_t_t101` | 20x600 | 600 | yes | basis | 602 | 4.441e-16 |
| `t105_t_t104` | 20x600 | 600 | yes | basis | 602 | 8.882e-16 |
| `t106_t_t101` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t106_t_t104` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t108_t_t104` | 20x600 | 600 | yes | basis | 602 | 6.661e-16 |
| `t108_t_t107` | 20x600 | 600 | yes | basis | 602 | 8.882e-16 |
| `t109_t_t104` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t109_t_t107` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t111_t_t107` | 20x600 | 600 | yes | basis | 602 | 8.882e-16 |
| `t111_t_t110` | 20x600 | 600 | yes | basis | 602 | 4.441e-16 |
| `t112_t_t107` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t112_t_t110` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t114_t_t110` | 20x600 | 600 | yes | basis | 602 | 8.882e-16 |
| `t114_t_t113` | 20x600 | 600 | yes | basis | 602 | 8.882e-16 |
| `t115_t_t110` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t115_t_t113` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t117_t_t113` | 20x600 | 600 | yes | basis | 602 | 4.441e-16 |
| `t117_t_t116` | 20x600 | 600 | yes | basis | 602 | 1.776e-15 |
| `t118_t_t113` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t118_t_t116` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t120_t_t116` | 20x600 | 600 | yes | basis | 602 | 8.882e-16 |
| `t120_t_t119` | 20x600 | 600 | yes | basis | 602 | 8.882e-16 |
| `t121_t_t116` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t121_t_t119` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t123_t_t119` | 20x600 | 600 | yes | basis | 602 | 4.441e-16 |
| `t123_t_t122` | 20x600 | 600 | yes | basis | 602 | 8.882e-16 |
| `t124_t_t119` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t124_t_t122` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t126_t_t122` | 20x600 | 600 | yes | basis | 602 | 8.882e-16 |
| `t126_t_t125` | 20x600 | 600 | yes | basis | 602 | 8.882e-16 |
| `t127_t_t122` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t127_t_t125` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t129_t_t125` | 20x600 | 600 | yes | basis | 602 | 8.882e-16 |
| `t129_t_t128` | 20x600 | 600 | yes | basis | 602 | 8.882e-16 |
| `t130_t_t125` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t130_t_t128` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t132_t_t128` | 20x600 | 600 | yes | basis | 602 | 8.882e-16 |
| `t132_t_t131` | 20x600 | 600 | yes | basis | 602 | 1.776e-15 |
| `t133_t_t128` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t133_t_t131` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t135_t_t131` | 20x600 | 600 | yes | basis | 602 | 4.441e-16 |
| `t135_t_t134` | 20x600 | 600 | yes | basis | 602 | 8.882e-16 |
| `t136_t_t131` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t136_t_t134` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t138_t_t134` | 20x600 | 600 | yes | basis | 602 | 8.882e-16 |
| `t138_t_t137` | 20x600 | 600 | yes | basis | 602 | 8.882e-16 |
| `t139_t_t134` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t139_t_t137` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t141_t_t137` | 20x600 | 600 | yes | basis | 602 | 8.882e-16 |
| `t141_t_t140` | 20x600 | 600 | yes | basis | 602 | 8.882e-16 |
| `t142_t_t137` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t142_t_t140` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t144_t_t140` | 20x600 | 600 | yes | basis | 602 | 4.441e-16 |
| `t144_t_t143` | 20x600 | 600 | yes | basis | 602 | 8.882e-16 |
| `t145_t_t140` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t145_t_t143` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t147_t_t143` | 20x600 | 600 | yes | basis | 602 | 8.882e-16 |
| `t147_t_t146` | 20x600 | 600 | yes | basis | 602 | 8.882e-16 |
| `t148_t_t143` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t148_t_t146` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `su_t_t146` | 650x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `su_t_t147` | 650x20 | 20 | yes | basis | 22 | 0.000e+00 |
| `su_t_t148` | 650x30 | 30 | yes | basis | 32 | 0.000e+00 |
| `fused_t1` | 20x1250 | 1220 | yes | basis | 1252 | 4.441e-16 |
| `fused_t2` | 600x1270 | 25200 | yes | basis | 1272 | 0.000e+00 |
| `fused_t3` | 20x1220 | 1220 | yes | basis | 1222 | 8.882e-16 |
| `fused_t4` | 30x1850 | 1830 | yes | basis | 1852 | 8.882e-16 |
| `fused_t5` | 600x650 | 1800 | yes | basis | 652 | 1.776e-15 |
| `fused_t6` | 20x1220 | 1220 | yes | basis | 1222 | 1.055e-15 |
| `fused_t7` | 30x1230 | 1230 | yes | basis | 1232 | 1.332e-15 |
| `fused_t8` | 600x650 | 1800 | yes | basis | 652 | 8.882e-16 |
| `fused_t9` | 20x1220 | 1220 | yes | basis | 1222 | 8.882e-16 |
| `fused_t10` | 30x1230 | 1230 | yes | basis | 1232 | 8.882e-16 |
| `fused_t11` | 600x650 | 1800 | yes | basis | 652 | 8.882e-16 |
| `fused_t12` | 20x1220 | 1220 | yes | basis | 1222 | 1.221e-15 |
| `fused_t13` | 30x1230 | 1230 | yes | basis | 1232 | 8.882e-16 |
| `fused_t14` | 600x650 | 1800 | yes | basis | 652 | 1.776e-15 |
| `fused_t15` | 20x1220 | 1220 | yes | basis | 1222 | 1.110e-15 |
| `fused_t16` | 30x1230 | 1230 | yes | basis | 1232 | 8.882e-16 |
| `fused_t17` | 600x650 | 1800 | yes | basis | 652 | 8.882e-16 |
| `fused_t18` | 20x1220 | 1220 | yes | basis | 1222 | 1.332e-15 |
| `fused_t19` | 30x1230 | 1230 | yes | basis | 1232 | 8.882e-16 |
| `fused_t20` | 600x650 | 1800 | yes | basis | 652 | 8.882e-16 |
| `fused_t21` | 20x1220 | 1220 | yes | basis | 1222 | 1.776e-15 |
| `fused_t22` | 30x1230 | 1230 | yes | basis | 1232 | 8.882e-16 |
| `fused_t23` | 600x650 | 1800 | yes | basis | 652 | 1.776e-15 |
| `fused_t24` | 20x1220 | 1220 | yes | basis | 1222 | 1.776e-15 |
| `fused_t25` | 30x1230 | 1230 | yes | basis | 1232 | 1.332e-15 |
| `fused_t26` | 600x650 | 1800 | yes | basis | 652 | 8.882e-16 |
| `fused_t27` | 20x1220 | 1220 | yes | basis | 1222 | 1.110e-15 |
| `fused_t28` | 30x1230 | 1230 | yes | basis | 1232 | 8.882e-16 |
| `fused_t29` | 600x650 | 1800 | yes | basis | 652 | 8.882e-16 |
| `fused_t30` | 20x1220 | 1220 | yes | basis | 1222 | 2.220e-15 |
| `fused_t31` | 30x1230 | 1230 | yes | basis | 1232 | 8.882e-16 |
| `fused_t32` | 600x650 | 1800 | yes | basis | 652 | 1.332e-15 |
| `fused_t33` | 20x1220 | 1220 | yes | basis | 1222 | 8.882e-16 |
| `fused_t34` | 30x1230 | 1230 | yes | basis | 1232 | 8.882e-16 |
| `fused_t35` | 600x650 | 1800 | yes | basis | 652 | 1.776e-15 |
| `fused_t36` | 20x1220 | 1220 | yes | basis | 1222 | 2.887e-15 |
| `fused_t37` | 30x1230 | 1230 | yes | basis | 1232 | 8.882e-16 |
| `fused_t38` | 600x650 | 1800 | yes | basis | 652 | 1.776e-15 |
| `fused_t39` | 20x1220 | 1220 | yes | basis | 1222 | 8.882e-16 |
| `fused_t40` | 30x1230 | 1230 | yes | basis | 1232 | 8.882e-16 |
| `fused_t41` | 600x650 | 1800 | yes | basis | 652 | 8.882e-16 |
| `fused_t42` | 20x1220 | 1220 | yes | basis | 1222 | 1.776e-15 |
| `fused_t43` | 30x1230 | 1230 | yes | basis | 1232 | 8.882e-16 |
| `fused_t44` | 600x650 | 1800 | yes | basis | 652 | 8.882e-16 |
| `fused_t45` | 20x1220 | 1220 | yes | basis | 1222 | 8.882e-16 |
| `fused_t46` | 30x1230 | 1230 | yes | basis | 1232 | 8.882e-16 |
| `fused_t47` | 600x650 | 1800 | yes | basis | 652 | 8.882e-16 |
| `fused_t48` | 20x1220 | 1220 | yes | basis | 1222 | 1.776e-15 |
| `fused_t49` | 30x1230 | 1230 | yes | basis | 1232 | 6.661e-16 |
| `fused_t50` | 600x650 | 1800 | yes | basis | 652 | 1.776e-15 |
| `fused_t51` | 20x1220 | 1220 | yes | basis | 1222 | 8.882e-16 |
| `fused_t52` | 30x1230 | 1230 | yes | basis | 1232 | 8.882e-16 |
| `fused_t53` | 600x650 | 1800 | yes | basis | 652 | 8.882e-16 |
| `fused_t54` | 20x1220 | 1220 | yes | basis | 1222 | 1.554e-15 |
| `fused_t55` | 30x1230 | 1230 | yes | basis | 1232 | 1.332e-15 |
| `fused_t56` | 600x650 | 1800 | yes | basis | 652 | 8.882e-16 |
| `fused_t57` | 20x1220 | 1220 | yes | basis | 1222 | 1.110e-15 |
| `fused_t58` | 30x1230 | 1230 | yes | basis | 1232 | 8.882e-16 |
| `fused_t59` | 600x650 | 1800 | yes | basis | 652 | 8.882e-16 |
| `fused_t60` | 20x1220 | 1220 | yes | basis | 1222 | 1.776e-15 |
| `fused_t61` | 30x1230 | 1230 | yes | basis | 1232 | 8.882e-16 |
| `fused_t62` | 600x650 | 1800 | yes | basis | 652 | 1.776e-15 |
| `fused_t63` | 20x1220 | 1220 | yes | basis | 1222 | 3.553e-15 |
| `fused_t64` | 30x1230 | 1230 | yes | basis | 1232 | 1.332e-15 |
| `fused_t65` | 600x650 | 1800 | yes | basis | 652 | 1.776e-15 |
| `fused_t66` | 20x1220 | 1220 | yes | basis | 1222 | 1.776e-15 |
| `fused_t67` | 30x1230 | 1230 | yes | basis | 1232 | 8.882e-16 |
| `fused_t68` | 600x650 | 1800 | yes | basis | 652 | 8.882e-16 |
| `fused_t69` | 20x1220 | 1220 | yes | basis | 1222 | 1.776e-15 |
| `fused_t70` | 30x1230 | 1230 | yes | basis | 1232 | 8.882e-16 |
| `fused_t71` | 600x650 | 1800 | yes | basis | 652 | 8.882e-16 |
| `fused_t72` | 20x1220 | 1220 | yes | basis | 1222 | 1.776e-15 |
| `fused_t73` | 30x1230 | 1230 | yes | basis | 1232 | 8.882e-16 |
| `fused_t74` | 600x650 | 1800 | yes | basis | 652 | 8.882e-16 |
| `fused_t75` | 20x1220 | 1220 | yes | basis | 1222 | 8.882e-16 |
| `fused_t76` | 30x1230 | 1230 | yes | basis | 1232 | 1.776e-15 |
| `fused_t77` | 600x650 | 1800 | yes | basis | 652 | 8.882e-16 |
| `fused_t78` | 20x1220 | 1220 | yes | basis | 1222 | 1.776e-15 |
| `fused_t79` | 30x1230 | 1230 | yes | basis | 1232 | 8.882e-16 |
| `fused_t80` | 600x650 | 1800 | yes | basis | 652 | 8.882e-16 |
| `fused_t81` | 20x1220 | 1220 | yes | basis | 1222 | 1.776e-15 |
| `fused_t82` | 30x1230 | 1230 | yes | basis | 1232 | 8.882e-16 |
| `fused_t83` | 600x650 | 1800 | yes | basis | 652 | 8.882e-16 |
| `fused_t84` | 20x1220 | 1220 | yes | basis | 1222 | 1.776e-15 |
| `fused_t85` | 30x1230 | 1230 | yes | basis | 1232 | 8.882e-16 |
| `fused_t86` | 600x650 | 1800 | yes | basis | 652 | 1.776e-15 |
| `fused_t87` | 20x1220 | 1220 | yes | basis | 1222 | 1.332e-15 |
| `fused_t88` | 30x1230 | 1230 | yes | basis | 1232 | 8.882e-16 |
| `fused_t89` | 600x650 | 1800 | yes | basis | 652 | 1.776e-15 |
| `fused_t90` | 20x1220 | 1220 | yes | basis | 1222 | 3.553e-15 |
| `fused_t91` | 30x1230 | 1230 | yes | basis | 1232 | 8.882e-16 |
| `fused_t92` | 600x650 | 1800 | yes | basis | 652 | 8.882e-16 |
| `fused_t93` | 20x1220 | 1220 | yes | basis | 1222 | 1.776e-15 |
| `fused_t94` | 30x1230 | 1230 | yes | basis | 1232 | 1.776e-15 |
| `fused_t95` | 600x650 | 1800 | yes | basis | 652 | 1.776e-15 |
| `fused_t96` | 20x1220 | 1220 | yes | basis | 1222 | 1.776e-15 |
| `fused_t97` | 30x1230 | 1230 | yes | basis | 1232 | 1.776e-15 |
| `fused_t98` | 600x650 | 1800 | yes | basis | 652 | 8.882e-16 |
| `fused_t99` | 20x1220 | 1220 | yes | basis | 1222 | 1.776e-15 |
| `fused_t100` | 30x1230 | 1230 | yes | basis | 1232 | 8.882e-16 |
| `fused_t101` | 600x650 | 1800 | yes | basis | 652 | 1.776e-15 |
| `fused_t102` | 20x1220 | 1220 | yes | basis | 1222 | 1.332e-15 |
| `fused_t103` | 30x1230 | 1230 | yes | basis | 1232 | 8.882e-16 |
| `fused_t104` | 600x650 | 1800 | yes | basis | 652 | 8.882e-16 |
| `fused_t105` | 20x1220 | 1220 | yes | basis | 1222 | 3.553e-15 |
| `fused_t106` | 30x1230 | 1230 | yes | basis | 1232 | 1.332e-15 |
| `fused_t107` | 600x650 | 1800 | yes | basis | 652 | 8.882e-16 |
| `fused_t108` | 20x1220 | 1220 | yes | basis | 1222 | 1.776e-15 |
| `fused_t109` | 30x1230 | 1230 | yes | basis | 1232 | 1.221e-15 |
| `fused_t110` | 600x650 | 1800 | yes | basis | 652 | 8.882e-16 |
| `fused_t111` | 20x1220 | 1220 | yes | basis | 1222 | 1.776e-15 |
| `fused_t112` | 30x1230 | 1230 | yes | basis | 1232 | 1.776e-15 |
| `fused_t113` | 600x650 | 1800 | yes | basis | 652 | 8.882e-16 |
| `fused_t114` | 20x1220 | 1220 | yes | basis | 1222 | 1.776e-15 |
| `fused_t115` | 30x1230 | 1230 | yes | basis | 1232 | 8.882e-16 |
| `fused_t116` | 600x650 | 1800 | yes | basis | 652 | 1.776e-15 |
| `fused_t117` | 20x1220 | 1220 | yes | basis | 1222 | 1.776e-15 |
| `fused_t118` | 30x1230 | 1230 | yes | basis | 1232 | 8.882e-16 |
| `fused_t119` | 600x650 | 1800 | yes | basis | 652 | 8.882e-16 |
| `fused_t120` | 20x1220 | 1220 | yes | basis | 1222 | 1.776e-15 |
| `fused_t121` | 30x1230 | 1230 | yes | basis | 1232 | 8.882e-16 |
| `fused_t122` | 600x650 | 1800 | yes | basis | 652 | 1.776e-15 |
| `fused_t123` | 20x1220 | 1220 | yes | basis | 1222 | 2.220e-15 |
| `fused_t124` | 30x1230 | 1230 | yes | basis | 1232 | 8.882e-16 |
| `fused_t125` | 600x650 | 1800 | yes | basis | 652 | 8.882e-16 |
| `fused_t126` | 20x1220 | 1220 | yes | basis | 1222 | 1.332e-15 |
| `fused_t127` | 30x1230 | 1230 | yes | basis | 1232 | 8.882e-16 |
| `fused_t128` | 600x650 | 1800 | yes | basis | 652 | 8.882e-16 |
| `fused_t129` | 20x1220 | 1220 | yes | basis | 1222 | 2.665e-15 |
| `fused_t130` | 30x1230 | 1230 | yes | basis | 1232 | 8.882e-16 |
| `fused_t131` | 600x650 | 1800 | yes | basis | 652 | 8.882e-16 |
| `fused_t132` | 20x1220 | 1220 | yes | basis | 1222 | 8.882e-16 |
| `fused_t133` | 30x1230 | 1230 | yes | basis | 1232 | 8.882e-16 |
| `fused_t134` | 600x650 | 1800 | yes | basis | 652 | 8.882e-16 |
| `fused_t135` | 20x1220 | 1220 | yes | basis | 1222 | 1.776e-15 |
| `fused_t136` | 30x1230 | 1230 | yes | basis | 1232 | 8.882e-16 |
| `fused_t137` | 600x650 | 1800 | yes | basis | 652 | 8.882e-16 |
| `fused_t138` | 20x1220 | 1220 | yes | basis | 1222 | 2.220e-15 |
| `fused_t139` | 30x1230 | 1230 | yes | basis | 1232 | 8.882e-16 |
| `fused_t140` | 600x650 | 1800 | yes | basis | 652 | 8.882e-16 |
| `fused_t141` | 20x1220 | 1220 | yes | basis | 1222 | 1.776e-15 |
| `fused_t142` | 30x1230 | 1230 | yes | basis | 1232 | 8.882e-16 |
| `fused_t143` | 600x650 | 1800 | yes | basis | 652 | 1.776e-15 |
| `fused_t144` | 20x1220 | 1220 | yes | basis | 1222 | 1.332e-15 |
| `fused_t145` | 30x1230 | 1230 | yes | basis | 1232 | 8.882e-16 |
| `fused_t146` | 600x650 | 1800 | yes | basis | 652 | 8.882e-16 |
| `fused_t147` | 20x1220 | 1220 | yes | basis | 1222 | 1.776e-15 |
| `fused_t148` | 30x1230 | 1230 | yes | basis | 1232 | 8.882e-16 |
| `fused_su` | 650x650 | 650 | yes | basis | 652 | 0.000e+00 |

### Rule violations

_None: the collected rewrite passed the rules and was kept._

## Blocked by rules

Optimizations the agent considered but did not make because a rule forbade them, in its own words.

| round | idea | rule | expected gain |
|---|---|---|---|
| 1 | Fold the bias initialization and clip/relu activation into the fused 600-row routine, making one pass instead of three (copy, fused add, clip) | fused routines must only accumulate `out += sum M_k src_k`, and the activation and bias stay in `src/fom_solver.c` | about 10–15% |
| 1 | Reorder the 20×600 incidence gathers into a column-sorted (CSC/scatter) layout precomputed at setup, so the 600-entry source is read contiguously | `fom_setup` may not read matrix data or probe matrices; only the `fom_apply`/`fom_fused` routines may read their own data, and `static` precomputed tables would have to be hardcoded literals derived offline | unknown, likely 5–10% |
| 1 | Fuse consecutive steps (e.g. t8 → t9 → t10) into one routine to keep intermediates in registers | each fused routine may read only its own block's members, and the step boundary is fixed by the API | unknown |
| 2 | Fold the bias init (`out = c`) and the clip into the fused flow kernel, giving one pass over the 600-vector instead of three | fused routines must accumulate into `out`, and `src/fom_ops.c` may read only member data (not `t0_c`/`t0_lo`/`t0_hi`) | ~5-10% |
| 2 | Precompute the transposed (arc-major) layout of the 30-row dual values once in `fom_setup` so both value and source reads are contiguous | `fom_setup` must not rebuild matrix data, and `src/fom_solver.c` must not read `FOM_MAT_*` of multiplied matrices | unknown, ~5% |
| 3 | Fold each step's bias init and clip/relu activation into the fused kernel, giving one pass over the 600 rows instead of three | the fused routine must compute exactly `out += sum_k s_k M_k src_k` (an accumulating contract checked by verify), and solver-side loops cannot hold products | ~3-5% (init and clip are about 2 of the 3 memory passes per flow step) |
| 4 | Precompute transposed (column-major) copies of the dual-step matrices at setup so both operands are read contiguously | fused routines may read only their members' data, and setup cannot build derived matrix tables from probing (setup may not call apply/fused routines) | unknown, likely <5% |
| 4 | Fold the clip/relu activation and the bias initialization into the fused routines, saving two passes over each 600-entry block | `fom_fused_*` must compute exactly `out += sum_k s_k M_k src_k` with no activation | ~5-10% |
| 4 | Compile at -O3 with -march=native for wider vectorization | Makefile and build flags are fixed | unknown |

## What the agent changed

```diff
--- src/fom_solver.c (translated)
+++ src/fom_solver.c (optimized)
@@ -1,3 +1,4 @@
+#include "fom_profile.h"
 #include "fom_solver.h"
 #include "fom_data.h"
 #include "fom_ops.h"
@@ -27,549 +28,845 @@
     for (i = 0; i < 600; ++i) { fom_float v = work->t0[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t0[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
     /* t1 */
     for (i = 0; i < 20; ++i) work->t1[i] = FOM_MAT_t1_c[i];
+    { FOM_PROF_BEGIN(step_30_dual);
     fom_fused_t1(state, work->t0, work->t1);
+    FOM_PROF_END(step_30_dual); }
     for (i = 0; i < 20; ++i) if (work->t1[i] < 0.0) work->t1[i] = 0.0;
     /* t2 */
     for (i = 0; i < 600; ++i) work->t2[i] = FOM_MAT_t0_c[i];
     fom_apply_t2_p_p0(p0, work->t2);
+    { FOM_PROF_BEGIN(step_600_flow);
     fom_fused_t2(state, work->t0, work->t1, work->t2);
+    FOM_PROF_END(step_600_flow); }
     for (i = 0; i < 600; ++i) { fom_float v = work->t2[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t2[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
     /* t3 */
     for (i = 0; i < 20; ++i) work->t3[i] = FOM_MAT_t1_c[i];
+    { FOM_PROF_BEGIN(step_20_relu);
     fom_fused_t3(work->t0, work->t1, work->t2, work->t3);
+    FOM_PROF_END(step_20_relu); }
     for (i = 0; i < 20; ++i) if (work->t3[i] < 0.0) work->t3[i] = 0.0;
     /* t4 */
     for (i = 0; i < 30; ++i) work->t4[i] = 0.0;
     fom_apply_t4_p_p0(p0, work->t4);
+    { FOM_PROF_BEGIN(step_30_dual);
     fom_fused_t4(state, work->t0, work->t2, work->t4);
+    FOM_PROF_END(step_30_dual); }
     /* t5 */
     for (i = 0; i < 600; ++i) work->t5[i] = FOM_MAT_t0_c[i];
+    { FOM_PROF_BEGIN(step_600_flow);
     fom_fused_t5(work->t2, work->t3, work->t4, work->t5);
+    FOM_PROF_END(step_600_flow); }
     for (i = 0; i < 600; ++i) { fom_float v = work->t5[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t5[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
     /* t6 */
     for (i = 0; i < 20; ++i) work->t6[i] = FOM_MAT_t1_c[i];
+    { FOM_PROF_BEGIN(step_20_relu);
     fom_fused_t6(work->t2, work->t3, work->t5, work->t6);
+    FOM_PROF_END(step_20_relu); }
     for (i = 0; i < 20; ++i) if (work->t6[i] < 0.0) work->t6[i] = 0.0;
     /* t7 */
     for (i = 0; i < 30; ++i) work->t7[i] = pc[i];
+    { FOM_PROF_BEGIN(step_30_dual);
     fom_fused_t7(work->t2, work->t4, work->t5, work->t7);
+    FOM_PROF_END(step_30_dual); }
     /* t8 */
     for (i = 0; i < 600; ++i) work->t8[i] = FOM_MAT_t0_c[i];
+    { FOM_PROF_BEGIN(step_600_flow);
     fom_fused_t8(work->t5, work->t6, work->t7, work->t8);
+    FOM_PROF_END(step_600_flow); }
     for (i = 0; i < 600; ++i) { fom_float v = work->t8[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t8[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
     /* t9 */
     for (i = 0; i < 20; ++i) work->t9[i] = FOM_MAT_t1_c[i];
+    { FOM_PROF_BEGIN(step_20_relu);
     fom_fused_t9(work->t5, work->t6, work->t8, work->t9);
+    FOM_PROF_END(step_20_relu); }
     for (i = 0; i < 20; ++i) if (work->t9[i] < 0.0) work->t9[i] = 0.0;
     /* t10 */
     for (i = 0; i < 30; ++i) work->t10[i] = pc[i];
+    { FOM_PROF_BEGIN(step_30_dual);
     fom_fused_t10(work->t5, work->t7, work->t8, work->t10);
+    FOM_PROF_END(step_30_dual); }
     /* t11 */
     for (i = 0; i < 600; ++i) work->t11[i] = FOM_MAT_t0_c[i];
+    { FOM_PROF_BEGIN(step_600_flow);
     fom_fused_t11(work->t8, work->t9, work->t10, work->t11);
+    FOM_PROF_END(step_600_flow); }
     for (i = 0; i < 600; ++i) { fom_float v = work->t11[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t11[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
     /* t12 */
     for (i = 0; i < 20; ++i) work->t12[i] = FOM_MAT_t1_c[i];
+    { FOM_PROF_BEGIN(step_20_relu);
     fom_fused_t12(work->t8, work->t9, work->t11, work->t12);
+    FOM_PROF_END(step_20_relu); }
     for (i = 0; i < 20; ++i) if (work->t12[i] < 0.0) work->t12[i] = 0.0;
     /* t13 */
     for (i = 0; i < 30; ++i) work->t13[i] = pc[i];
+    { FOM_PROF_BEGIN(step_30_dual);
     fom_fused_t13(work->t8, work->t10, work->t11, work->t13);
+    FOM_PROF_END(step_30_dual); }
     /* t14 */
     for (i = 0; i < 600; ++i) work->t14[i] = FOM_MAT_t0_c[i];
+    { FOM_PROF_BEGIN(step_600_flow);
     fom_fused_t14(work->t11, work->t12, work->t13, work->t14);
+    FOM_PROF_END(step_600_flow); }
     for (i = 0; i < 600; ++i) { fom_float v = work->t14[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t14[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
     /* t15 */
     for (i = 0; i < 20; ++i) work->t15[i] = FOM_MAT_t1_c[i];
+    { FOM_PROF_BEGIN(step_20_relu);
     fom_fused_t15(work->t11, work->t12, work->t14, work->t15);
+    FOM_PROF_END(step_20_relu); }
     for (i = 0; i < 20; ++i) if (work->t15[i] < 0.0) work->t15[i] = 0.0;
     /* t16 */
     for (i = 0; i < 30; ++i) work->t16[i] = pc[i];
+    { FOM_PROF_BEGIN(step_30_dual);
     fom_fused_t16(work->t11, work->t13, work->t14, work->t16);
+    FOM_PROF_END(step_30_dual); }
     /* t17 */
     for (i = 0; i < 600; ++i) work->t17[i] = FOM_MAT_t0_c[i];
+    { FOM_PROF_BEGIN(step_600_flow);
     fom_fused_t17(work->t14, work->t15, work->t16, work->t17);
+    FOM_PROF_END(step_600_flow); }
     for (i = 0; i < 600; ++i) { fom_float v = work->t17[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t17[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
     /* t18 */
     for (i = 0; i < 20; ++i) work->t18[i] = FOM_MAT_t1_c[i];
+    { FOM_PROF_BEGIN(step_20_relu);
     fom_fused_t18(work->t14, work->t15, work->t17, work->t18);
+    FOM_PROF_END(step_20_relu); }
     for (i = 0; i < 20; ++i) if (work->t18[i] < 0.0) work->t18[i] = 0.0;
     /* t19 */
     for (i = 0; i < 30; ++i) work->t19[i] = pc[i];
+    { FOM_PROF_BEGIN(step_30_dual);
     fom_fused_t19(work->t14, work->t16, work->t17, work->t19);
+    FOM_PROF_END(step_30_dual); }
     /* t20 */
     for (i = 0; i < 600; ++i) work->t20[i] = FOM_MAT_t0_c[i];
+    { FOM_PROF_BEGIN(step_600_flow);
     fom_fused_t20(work->t17, work->t18, work->t19, work->t20);
+    FOM_PROF_END(step_600_flow); }
     for (i = 0; i < 600; ++i) { fom_float v = work->t20[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t20[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
     /* t21 */
     for (i = 0; i < 20; ++i) work->t21[i] = FOM_MAT_t1_c[i];
+    { FOM_PROF_BEGIN(step_20_relu);
     fom_fused_t21(work->t17, work->t18, work->t20, work->t21);
+    FOM_PROF_END(step_20_relu); }
     for (i = 0; i < 20; ++i) if (work->t21[i] < 0.0) work->t21[i] = 0.0;
     /* t22 */
     for (i = 0; i < 30; ++i) work->t22[i] = pc[i];
+    { FOM_PROF_BEGIN(step_30_dual);
     fom_fused_t22(work->t17, work->t19, work->t20, work->t22);
+    FOM_PROF_END(step_30_dual); }
     /* t23 */
     for (i = 0; i < 600; ++i) work->t23[i] = FOM_MAT_t0_c[i];
+    { FOM_PROF_BEGIN(step_600_flow);
     fom_fused_t23(work->t20, work->t21, work->t22, work->t23);
+    FOM_PROF_END(step_600_flow); }
     for (i = 0; i < 600; ++i) { fom_float v = work->t23[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t23[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
     /* t24 */
     for (i = 0; i < 20; ++i) work->t24[i] = FOM_MAT_t1_c[i];
+    { FOM_PROF_BEGIN(step_20_relu);
     fom_fused_t24(work->t20, work->t21, work->t23, work->t24);
+    FOM_PROF_END(step_20_relu); }
     for (i = 0; i < 20; ++i) if (work->t24[i] < 0.0) work->t24[i] = 0.0;
     /* t25 */
     for (i = 0; i < 30; ++i) work->t25[i] = pc[i];
+    { FOM_PROF_BEGIN(step_30_dual);
     fom_fused_t25(work->t20, work->t22, work->t23, work->t25);
+    FOM_PROF_END(step_30_dual); }
     /* t26 */
     for (i = 0; i < 600; ++i) work->t26[i] = FOM_MAT_t0_c[i];
+    { FOM_PROF_BEGIN(step_600_flow);
     fom_fused_t26(work->t23, work->t24, work->t25, work->t26);
+    FOM_PROF_END(step_600_flow); }
     for (i = 0; i < 600; ++i) { fom_float v = work->t26[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t26[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
     /* t27 */
     for (i = 0; i < 20; ++i) work->t27[i] = FOM_MAT_t1_c[i];
+    { FOM_PROF_BEGIN(step_20_relu);
     fom_fused_t27(work->t23, work->t24, work->t26, work->t27);
+    FOM_PROF_END(step_20_relu); }
     for (i = 0; i < 20; ++i) if (work->t27[i] < 0.0) work->t27[i] = 0.0;
     /* t28 */
     for (i = 0; i < 30; ++i) work->t28[i] = pc[i];
+    { FOM_PROF_BEGIN(step_30_dual);
     fom_fused_t28(work->t23, work->t25, work->t26, work->t28);
+    FOM_PROF_END(step_30_dual); }
     /* t29 */
     for (i = 0; i < 600; ++i) work->t29[i] = FOM_MAT_t0_c[i];
+    { FOM_PROF_BEGIN(step_600_flow);
     fom_fused_t29(work->t26, work->t27, work->t28, work->t29);
+    FOM_PROF_END(step_600_flow); }
     for (i = 0; i < 600; ++i) { fom_float v = work->t29[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t29[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
     /* t30 */
     for (i = 0; i < 20; ++i) work->t30[i] = FOM_MAT_t1_c[i];
+    { FOM_PROF_BEGIN(step_20_relu);
     fom_fused_t30(work->t26, work->t27, work->t29, work->t30);
+    FOM_PROF_END(step_20_relu); }
     for (i = 0; i < 20; ++i) if (work->t30[i] < 0.0) work->t30[i] = 0.0;
     /* t31 */
     for (i = 0; i < 30; ++i) work->t31[i] = pc[i];
+    { FOM_PROF_BEGIN(step_30_dual);
     fom_fused_t31(work->t26, work->t28, work->t29, work->t31);
+    FOM_PROF_END(step_30_dual); }
     /* t32 */
     for (i = 0; i < 600; ++i) work->t32[i] = FOM_MAT_t0_c[i];
+    { FOM_PROF_BEGIN(step_600_flow);
     fom_fused_t32(work->t29, work->t30, work->t31, work->t32);
+    FOM_PROF_END(step_600_flow); }
     for (i = 0; i < 600; ++i) { fom_float v = work->t32[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t32[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
     /* t33 */
     for (i = 0; i < 20; ++i) work->t33[i] = FOM_MAT_t1_c[i];
+    { FOM_PROF_BEGIN(step_20_relu);
     fom_fused_t33(work->t29, work->t30, work->t32, work->t33);
+    FOM_PROF_END(step_20_relu); }
     for (i = 0; i < 20; ++i) if (work->t33[i] < 0.0) work->t33[i] = 0.0;
     /* t34 */
     for (i = 0; i < 30; ++i) work->t34[i] = pc[i];
+    { FOM_PROF_BEGIN(step_30_dual);
     fom_fused_t34(work->t29, work->t31, work->t32, work->t34);
+    FOM_PROF_END(step_30_dual); }
     /* t35 */
     for (i = 0; i < 600; ++i) work->t35[i] = FOM_MAT_t0_c[i];
+    { FOM_PROF_BEGIN(step_600_flow);
     fom_fused_t35(work->t32, work->t33, work->t34, work->t35);
+    FOM_PROF_END(step_600_flow); }
     for (i = 0; i < 600; ++i) { fom_float v = work->t35[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t35[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
     /* t36 */
     for (i = 0; i < 20; ++i) work->t36[i] = FOM_MAT_t1_c[i];
+    { FOM_PROF_BEGIN(step_20_relu);
     fom_fused_t36(work->t32, work->t33, work->t35, work->t36);
+    FOM_PROF_END(step_20_relu); }
     for (i = 0; i < 20; ++i) if (work->t36[i] < 0.0) work->t36[i] = 0.0;
     /* t37 */
     for (i = 0; i < 30; ++i) work->t37[i] = pc[i];
+    { FOM_PROF_BEGIN(step_30_dual);
     fom_fused_t37(work->t32, work->t34, work->t35, work->t37);
+    FOM_PROF_END(step_30_dual); }
     /* t38 */
     for (i = 0; i < 600; ++i) work->t38[i] = FOM_MAT_t0_c[i];
+    { FOM_PROF_BEGIN(step_600_flow);
     fom_fused_t38(work->t35, work->t36, work->t37, work->t38);
+    FOM_PROF_END(step_600_flow); }
     for (i = 0; i < 600; ++i) { fom_float v = work->t38[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t38[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
     /* t39 */
     for (i = 0; i < 20; ++i) work->t39[i] = FOM_MAT_t1_c[i];
+    { FOM_PROF_BEGIN(step_20_relu);
     fom_fused_t39(work->t35, work->t36, work->t38, work->t39);
+    FOM_PROF_END(step_20_relu); }
     for (i = 0; i < 20; ++i) if (work->t39[i] < 0.0) work->t39[i] = 0.0;
     /* t40 */
     for (i = 0; i < 30; ++i) work->t40[i] = pc[i];
+    { FOM_PROF_BEGIN(step_30_dual);
     fom_fused_t40(work->t35, work->t37, work->t38, work->t40);
+    FOM_PROF_END(step_30_dual); }
     /* t41 */
     for (i = 0; i < 600; ++i) work->t41[i] = FOM_MAT_t0_c[i];
+    { FOM_PROF_BEGIN(step_600_flow);
     fom_fused_t41(work->t38, work->t39, work->t40, work->t41);
+    FOM_PROF_END(step_600_flow); }
     for (i = 0; i < 600; ++i) { fom_float v = work->t41[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t41[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
     /* t42 */
     for (i = 0; i < 20; ++i) work->t42[i] = FOM_MAT_t1_c[i];
+    { FOM_PROF_BEGIN(step_20_relu);
     fom_fused_t42(work->t38, work->t39, work->t41, work->t42);
+    FOM_PROF_END(step_20_relu); }
     for (i = 0; i < 20; ++i) if (work->t42[i] < 0.0) work->t42[i] = 0.0;
     /* t43 */
     for (i = 0; i < 30; ++i) work->t43[i] = pc[i];
+    { FOM_PROF_BEGIN(step_30_dual);
     fom_fused_t43(work->t38, work->t40, work->t41, work->t43);
+    FOM_PROF_END(step_30_dual); }
     /* t44 */
     for (i = 0; i < 600; ++i) work->t44[i] = FOM_MAT_t0_c[i];
+    { FOM_PROF_BEGIN(step_600_flow);
     fom_fused_t44(work->t41, work->t42, work->t43, work->t44);
+    FOM_PROF_END(step_600_flow); }
     for (i = 0; i < 600; ++i) { fom_float v = work->t44[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t44[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
     /* t45 */
     for (i = 0; i < 20; ++i) work->t45[i] = FOM_MAT_t1_c[i];
+    { FOM_PROF_BEGIN(step_20_relu);
     fom_fused_t45(work->t41, work->t42, work->t44, work->t45);
+    FOM_PROF_END(step_20_relu); }
     for (i = 0; i < 20; ++i) if (work->t45[i] < 0.0) work->t45[i] = 0.0;
     /* t46 */
     for (i = 0; i < 30; ++i) work->t46[i] = pc[i];
+    { FOM_PROF_BEGIN(step_30_dual);
     fom_fused_t46(work->t41, work->t43, work->t44, work->t46);
+    FOM_PROF_END(step_30_dual); }
     /* t47 */
     for (i = 0; i < 600; ++i) work->t47[i] = FOM_MAT_t0_c[i];
+    { FOM_PROF_BEGIN(step_600_flow);
     fom_fused_t47(work->t44, work->t45, work->t46, work->t47);
+    FOM_PROF_END(step_600_flow); }
     for (i = 0; i < 600; ++i) { fom_float v = work->t47[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t47[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
     /* t48 */
     for (i = 0; i < 20; ++i) work->t48[i] = FOM_MAT_t1_c[i];
+    { FOM_PROF_BEGIN(step_20_relu);
     fom_fused_t48(work->t44, work->t45, work->t47, work->t48);
+    FOM_PROF_END(step_20_relu); }
     for (i = 0; i < 20; ++i) if (work->t48[i] < 0.0) work->t48[i] = 0.0;
     /* t49 */
     for (i = 0; i < 30; ++i) work->t49[i] = pc[i];
+    { FOM_PROF_BEGIN(step_30_dual);
     fom_fused_t49(work->t44, work->t46, work->t47, work->t49);
+    FOM_PROF_END(step_30_dual); }
     /* t50 */
     for (i = 0; i < 600; ++i) work->t50[i] = FOM_MAT_t0_c[i];
+    { FOM_PROF_BEGIN(step_600_flow);
     fom_fused_t50(work->t47, work->t48, work->t49, work->t50);
+    FOM_PROF_END(step_600_flow); }
     for (i = 0; i < 600; ++i) { fom_float v = work->t50[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t50[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
     /* t51 */
     for (i = 0; i < 20; ++i) work->t51[i] = FOM_MAT_t1_c[i];
+    { FOM_PROF_BEGIN(step_20_relu);
     fom_fused_t51(work->t47, work->t48, work->t50, work->t51);
+    FOM_PROF_END(step_20_relu); }
     for (i = 0; i < 20; ++i) if (work->t51[i] < 0.0) work->t51[i] = 0.0;
     /* t52 */
     for (i = 0; i < 30; ++i) work->t52[i] = pc[i];
+    { FOM_PROF_BEGIN(step_30_dual);
     fom_fused_t52(work->t47, work->t49, work->t50, work->t52);
+    FOM_PROF_END(step_30_dual); }
     /* t53 */
     for (i = 0; i < 600; ++i) work->t53[i] = FOM_MAT_t0_c[i];
+    { FOM_PROF_BEGIN(step_600_flow);
     fom_fused_t53(work->t50, work->t51, work->t52, work->t53);
+    FOM_PROF_END(step_600_flow); }
     for (i = 0; i < 600; ++i) { fom_float v = work->t53[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t53[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
     /* t54 */
     for (i = 0; i < 20; ++i) work->t54[i] = FOM_MAT_t1_c[i];
+    { FOM_PROF_BEGIN(step_20_relu);
     fom_fused_t54(work->t50, work->t51, work->t53, work->t54);
+    FOM_PROF_END(step_20_relu); }
     for (i = 0; i < 20; ++i) if (work->t54[i] < 0.0) work->t54[i] = 0.0;
     /* t55 */
     for (i = 0; i < 30; ++i) work->t55[i] = pc[i];
+    { FOM_PROF_BEGIN(step_30_dual);
     fom_fused_t55(work->t50, work->t52, work->t53, work->t55);
+    FOM_PROF_END(step_30_dual); }
     /* t56 */
     for (i = 0; i < 600; ++i) work->t56[i] = FOM_MAT_t0_c[i];
+    { FOM_PROF_BEGIN(step_600_flow);
     fom_fused_t56(work->t53, work->t54, work->t55, work->t56);
+    FOM_PROF_END(step_600_flow); }
     for (i = 0; i < 600; ++i) { fom_float v = work->t56[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t56[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
     /* t57 */
     for (i = 0; i < 20; ++i) work->t57[i] = FOM_MAT_t1_c[i];
+    { FOM_PROF_BEGIN(step_20_relu);
     fom_fused_t57(work->t53, work->t54, work->t56, work->t57);
+    FOM_PROF_END(step_20_relu); }
     for (i = 0; i < 20; ++i) if (work->t57[i] < 0.0) work->t57[i] = 0.0;
     /* t58 */
     for (i = 0; i < 30; ++i) work->t58[i] = pc[i];
+    { FOM_PROF_BEGIN(step_30_dual);
     fom_fused_t58(work->t53, work->t55, work->t56, work->t58);
+    FOM_PROF_END(step_30_dual); }
     /* t59 */
     for (i = 0; i < 600; ++i) work->t59[i] = FOM_MAT_t0_c[i];
+    { FOM_PROF_BEGIN(step_600_flow);
     fom_fused_t59(work->t56, work->t57, work->t58, work->t59);
+    FOM_PROF_END(step_600_flow); }
     for (i = 0; i < 600; ++i) { fom_float v = work->t59[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t59[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
     /* t60 */
     for (i = 0; i < 20; ++i) work->t60[i] = FOM_MAT_t1_c[i];
+    { FOM_PROF_BEGIN(step_20_relu);
     fom_fused_t60(work->t56, work->t57, work->t59, work->t60);
+    FOM_PROF_END(step_20_relu); }
     for (i = 0; i < 20; ++i) if (work->t60[i] < 0.0) work->t60[i] = 0.0;
     /* t61 */
     for (i = 0; i < 30; ++i) work->t61[i] = pc[i];
+    { FOM_PROF_BEGIN(step_30_dual);
     fom_fused_t61(work->t56, work->t58, work->t59, work->t61);
+    FOM_PROF_END(step_30_dual); }
     /* t62 */
     for (i = 0; i < 600; ++i) work->t62[i] = FOM_MAT_t0_c[i];
+    { FOM_PROF_BEGIN(step_600_flow);
     fom_fused_t62(work->t59, work->t60, work->t61, work->t62);
+    FOM_PROF_END(step_600_flow); }
     for (i = 0; i < 600; ++i) { fom_float v = work->t62[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t62[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
     /* t63 */
     for (i = 0; i < 20; ++i) work->t63[i] = FOM_MAT_t1_c[i];
+    { FOM_PROF_BEGIN(step_20_relu);
     fom_fused_t63(work->t59, work->t60, work->t62, work->t63);
+    FOM_PROF_END(step_20_relu); }
     for (i = 0; i < 20; ++i) if (work->t63[i] < 0.0) work->t63[i] = 0.0;
     /* t64 */
     for (i = 0; i < 30; ++i) work->t64[i] = pc[i];
+    { FOM_PROF_BEGIN(step_30_dual);
     fom_fused_t64(work->t59, work->t61, work->t62, work->t64);
+    FOM_PROF_END(step_30_dual); }
     /* t65 */
     for (i = 0; i < 600; ++i) work->t65[i] = FOM_MAT_t0_c[i];
+    { FOM_PROF_BEGIN(step_600_flow);
     fom_fused_t65(work->t62, work->t63, work->t64, work->t65);
+    FOM_PROF_END(step_600_flow); }
     for (i = 0; i < 600; ++i) { fom_float v = work->t65[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t65[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
     /* t66 */
     for (i = 0; i < 20; ++i) work->t66[i] = FOM_MAT_t1_c[i];
+    { FOM_PROF_BEGIN(step_20_relu);
     fom_fused_t66(work->t62, work->t63, work->t65, work->t66);
+    FOM_PROF_END(step_20_relu); }
     for (i = 0; i < 20; ++i) if (work->t66[i] < 0.0) work->t66[i] = 0.0;
     /* t67 */
     for (i = 0; i < 30; ++i) work->t67[i] = pc[i];
+    { FOM_PROF_BEGIN(step_30_dual);
     fom_fused_t67(work->t62, work->t64, work->t65, work->t67);
+    FOM_PROF_END(step_30_dual); }
     /* t68 */
     for (i = 0; i < 600; ++i) work->t68[i] = FOM_MAT_t0_c[i];
+    { FOM_PROF_BEGIN(step_600_flow);
     fom_fused_t68(work->t65, work->t66, work->t67, work->t68);
+    FOM_PROF_END(step_600_flow); }
     for (i = 0; i < 600; ++i) { fom_float v = work->t68[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t68[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
     /* t69 */
     for (i = 0; i < 20; ++i) work->t69[i] = FOM_MAT_t1_c[i];
+    { FOM_PROF_BEGIN(step_20_relu);
     fom_fused_t69(work->t65, work->t66, work->t68, work->t69);
+    FOM_PROF_END(step_20_relu); }
     for (i = 0; i < 20; ++i) if (work->t69[i] < 0.0) work->t69[i] = 0.0;
     /* t70 */
     for (i = 0; i < 30; ++i) work->t70[i] = pc[i];
+    { FOM_PROF_BEGIN(step_30_dual);
     fom_fused_t70(work->t65, work->t67, work->t68, work->t70);
+    FOM_PROF_END(step_30_dual); }
     /* t71 */
     for (i = 0; i < 600; ++i) work->t71[i] = FOM_MAT_t0_c[i];
+    { FOM_PROF_BEGIN(step_600_flow);
     fom_fused_t71(work->t68, work->t69, work->t70, work->t71);
+    FOM_PROF_END(step_600_flow); }
     for (i = 0; i < 600; ++i) { fom_float v = work->t71[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t71[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
     /* t72 */
     for (i = 0; i < 20; ++i) work->t72[i] = FOM_MAT_t1_c[i];
+    { FOM_PROF_BEGIN(step_20_relu);
     fom_fused_t72(work->t68, work->t69, work->t71, work->t72);
+    FOM_PROF_END(step_20_relu); }
     for (i = 0; i < 20; ++i) if (work->t72[i] < 0.0) work->t72[i] = 0.0;
     /* t73 */
     for (i = 0; i < 30; ++i) work->t73[i] = pc[i];
+    { FOM_PROF_BEGIN(step_30_dual);
     fom_fused_t73(work->t68, work->t70, work->t71, work->t73);
+    FOM_PROF_END(step_30_dual); }
     /* t74 */
     for (i = 0; i < 600; ++i) work->t74[i] = FOM_MAT_t0_c[i];
+    { FOM_PROF_BEGIN(step_600_flow);
     fom_fused_t74(work->t71, work->t72, work->t73, work->t74);
+    FOM_PROF_END(step_600_flow); }
     for (i = 0; i < 600; ++i) { fom_float v = work->t74[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t74[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
     /* t75 */
     for (i = 0; i < 20; ++i) work->t75[i] = FOM_MAT_t1_c[i];
+    { FOM_PROF_BEGIN(step_20_relu);
     fom_fused_t75(work->t71, work->t72, work->t74, work->t75);
+    FOM_PROF_END(step_20_relu); }
     for (i = 0; i < 20; ++i) if (work->t75[i] < 0.0) work->t75[i] = 0.0;
     /* t76 */
     for (i = 0; i < 30; ++i) work->t76[i] = pc[i];
+    { FOM_PROF_BEGIN(step_30_dual);
     fom_fused_t76(work->t71, work->t73, work->t74, work->t76);
+    FOM_PROF_END(step_30_dual); }
     /* t77 */
     for (i = 0; i < 600; ++i) work->t77[i] = FOM_MAT_t0_c[i];
+    { FOM_PROF_BEGIN(step_600_flow);
     fom_fused_t77(work->t74, work->t75, work->t76, work->t77);
+    FOM_PROF_END(step_600_flow); }
     for (i = 0; i < 600; ++i) { fom_float v = work->t77[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t77[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
     /* t78 */
     for (i = 0; i < 20; ++i) work->t78[i] = FOM_MAT_t1_c[i];
+    { FOM_PROF_BEGIN(step_20_relu);
     fom_fused_t78(work->t74, work->t75, work->t77, work->t78);
+    FOM_PROF_END(step_20_relu); }
     for (i = 0; i < 20; ++i) if (work->t78[i] < 0.0) work->t78[i] = 0.0;
     /* t79 */
     for (i = 0; i < 30; ++i) work->t79[i] = pc[i];
+    { FOM_PROF_BEGIN(step_30_dual);
     fom_fused_t79(work->t74, work->t76, work->t77, work->t79);
+    FOM_PROF_END(step_30_dual); }
     /* t80 */
     for (i = 0; i < 600; ++i) work->t80[i] = FOM_MAT_t0_c[i];
+    { FOM_PROF_BEGIN(step_600_flow);
     fom_fused_t80(work->t77, work->t78, work->t79, work->t80);
+    FOM_PROF_END(step_600_flow); }
     for (i = 0; i < 600; ++i) { fom_float v = work->t80[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t80[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
     /* t81 */
     for (i = 0; i < 20; ++i) work->t81[i] = FOM_MAT_t1_c[i];
+    { FOM_PROF_BEGIN(step_20_relu);
     fom_fused_t81(work->t77, work->t78, work->t80, work->t81);
+    FOM_PROF_END(step_20_relu); }
     for (i = 0; i < 20; ++i) if (work->t81[i] < 0.0) work->t81[i] = 0.0;
     /* t82 */
     for (i = 0; i < 30; ++i) work->t82[i] = pc[i];
+    { FOM_PROF_BEGIN(step_30_dual);
     fom_fused_t82(work->t77, work->t79, work->t80, work->t82);
+    FOM_PROF_END(step_30_dual); }
     /* t83 */
     for (i = 0; i < 600; ++i) work->t83[i] = FOM_MAT_t0_c[i];
+    { FOM_PROF_BEGIN(step_600_flow);
     fom_fused_t83(work->t80, work->t81, work->t82, work->t83);
+    FOM_PROF_END(step_600_flow); }
     for (i = 0; i < 600; ++i) { fom_float v = work->t83[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t83[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
     /* t84 */
     for (i = 0; i < 20; ++i) work->t84[i] = FOM_MAT_t1_c[i];
+    { FOM_PROF_BEGIN(step_20_relu);
     fom_fused_t84(work->t80, work->t81, work->t83, work->t84);
+    FOM_PROF_END(step_20_relu); }
     for (i = 0; i < 20; ++i) if (work->t84[i] < 0.0) work->t84[i] = 0.0;
     /* t85 */
     for (i = 0; i < 30; ++i) work->t85[i] = pc[i];
+    { FOM_PROF_BEGIN(step_30_dual);
     fom_fused_t85(work->t80, work->t82, work->t83, work->t85);
+    FOM_PROF_END(step_30_dual); }
     /* t86 */
     for (i = 0; i < 600; ++i) work->t86[i] = FOM_MAT_t0_c[i];
+    { FOM_PROF_BEGIN(step_600_flow);
     fom_fused_t86(work->t83, work->t84, work->t85, work->t86);
+    FOM_PROF_END(step_600_flow); }
     for (i = 0; i < 600; ++i) { fom_float v = work->t86[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t86[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
     /* t87 */
     for (i = 0; i < 20; ++i) work->t87[i] = FOM_MAT_t1_c[i];
+    { FOM_PROF_BEGIN(step_20_relu);
     fom_fused_t87(work->t83, work->t84, work->t86, work->t87);
+    FOM_PROF_END(step_20_relu); }
     for (i = 0; i < 20; ++i) if (work->t87[i] < 0.0) work->t87[i] = 0.0;
     /* t88 */
     for (i = 0; i < 30; ++i) work->t88[i] = pc[i];
+    { FOM_PROF_BEGIN(step_30_dual);
     fom_fused_t88(work->t83, work->t85, work->t86, work->t88);
+    FOM_PROF_END(step_30_dual); }
     /* t89 */
     for (i = 0; i < 600; ++i) work->t89[i] = FOM_MAT_t0_c[i];
+    { FOM_PROF_BEGIN(step_600_flow);
     fom_fused_t89(work->t86, work->t87, work->t88, work->t89);
+    FOM_PROF_END(step_600_flow); }
     for (i = 0; i < 600; ++i) { fom_float v = work->t89[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t89[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
     /* t90 */
     for (i = 0; i < 20; ++i) work->t90[i] = FOM_MAT_t1_c[i];
+    { FOM_PROF_BEGIN(step_20_relu);
     fom_fused_t90(work->t86, work->t87, work->t89, work->t90);
+    FOM_PROF_END(step_20_relu); }
     for (i = 0; i < 20; ++i) if (work->t90[i] < 0.0) work->t90[i] = 0.0;
     /* t91 */
     for (i = 0; i < 30; ++i) work->t91[i] = pc[i];
+    { FOM_PROF_BEGIN(step_30_dual);
     fom_fused_t91(work->t86, work->t88, work->t89, work->t91);
+    FOM_PROF_END(step_30_dual); }
     /* t92 */
     for (i = 0; i < 600; ++i) work->t92[i] = FOM_MAT_t0_c[i];
+    { FOM_PROF_BEGIN(step_600_flow);
     fom_fused_t92(work->t89, work->t90, work->t91, work->t92);
+    FOM_PROF_END(step_600_flow); }
     for (i = 0; i < 600; ++i) { fom_float v = work->t92[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t92[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
     /* t93 */
     for (i = 0; i < 20; ++i) work->t93[i] = FOM_MAT_t1_c[i];
+    { FOM_PROF_BEGIN(step_20_relu);
     fom_fused_t93(work->t89, work->t90, work->t92, work->t93);
+    FOM_PROF_END(step_20_relu); }
     for (i = 0; i < 20; ++i) if (work->t93[i] < 0.0) work->t93[i] = 0.0;
     /* t94 */
     for (i = 0; i < 30; ++i) work->t94[i] = pc[i];
+    { FOM_PROF_BEGIN(step_30_dual);
     fom_fused_t94(work->t89, work->t91, work->t92, work->t94);
+    FOM_PROF_END(step_30_dual); }
     /* t95 */
     for (i = 0; i < 600; ++i) work->t95[i] = FOM_MAT_t0_c[i];
+    { FOM_PROF_BEGIN(step_600_flow);
     fom_fused_t95(work->t92, work->t93, work->t94, work->t95);
+    FOM_PROF_END(step_600_flow); }
     for (i = 0; i < 600; ++i) { fom_float v = work->t95[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t95[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
     /* t96 */
     for (i = 0; i < 20; ++i) work->t96[i] = FOM_MAT_t1_c[i];
+    { FOM_PROF_BEGIN(step_20_relu);
     fom_fused_t96(work->t92, work->t93, work->t95, work->t96);
+    FOM_PROF_END(step_20_relu); }
     for (i = 0; i < 20; ++i) if (work->t96[i] < 0.0) work->t96[i] = 0.0;
     /* t97 */
     for (i = 0; i < 30; ++i) work->t97[i] = pc[i];
+    { FOM_PROF_BEGIN(step_30_dual);
     fom_fused_t97(work->t92, work->t94, work->t95, work->t97);
+    FOM_PROF_END(step_30_dual); }
     /* t98 */
     for (i = 0; i < 600; ++i) work->t98[i] = FOM_MAT_t0_c[i];
+    { FOM_PROF_BEGIN(step_600_flow);
     fom_fused_t98(work->t95, work->t96, work->t97, work->t98);
+    FOM_PROF_END(step_600_flow); }
     for (i = 0; i < 600; ++i) { fom_float v = work->t98[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t98[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
     /* t99 */
     for (i = 0; i < 20; ++i) work->t99[i] = FOM_MAT_t1_c[i];
+    { FOM_PROF_BEGIN(step_20_relu);
     fom_fused_t99(work->t95, work->t96, work->t98, work->t99);
+    FOM_PROF_END(step_20_relu); }
     for (i = 0; i < 20; ++i) if (work->t99[i] < 0.0) work->t99[i] = 0.0;
     /* t100 */
     for (i = 0; i < 30; ++i) work->t100[i] = pc[i];
+    { FOM_PROF_BEGIN(step_30_dual);
     fom_fused_t100(work->t95, work->t97, work->t98, work->t100);
+    FOM_PROF_END(step_30_dual); }
     /* t101 */
     for (i = 0; i < 600; ++i) work->t101[i] = FOM_MAT_t0_c[i];
+    { FOM_PROF_BEGIN(step_600_flow);
     fom_fused_t101(work->t98, work->t99, work->t100, work->t101);
+    FOM_PROF_END(step_600_flow); }
     for (i = 0; i < 600; ++i) { fom_float v = work->t101[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t101[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
     /* t102 */
     for (i = 0; i < 20; ++i) work->t102[i] = FOM_MAT_t1_c[i];
+    { FOM_PROF_BEGIN(step_20_relu);
     fom_fused_t102(work->t98, work->t99, work->t101, work->t102);
+    FOM_PROF_END(step_20_relu); }
     for (i = 0; i < 20; ++i) if (work->t102[i] < 0.0) work->t102[i] = 0.0;
     /* t103 */
     for (i = 0; i < 30; ++i) work->t103[i] = pc[i];
+    { FOM_PROF_BEGIN(step_30_dual);
     fom_fused_t103(work->t98, work->t100, work->t101, work->t103);
+    FOM_PROF_END(step_30_dual); }
     /* t104 */
     for (i = 0; i < 600; ++i) work->t104[i] = FOM_MAT_t0_c[i];
+    { FOM_PROF_BEGIN(step_600_flow);
     fom_fused_t104(work->t101, work->t102, work->t103, work->t104);
+    FOM_PROF_END(step_600_flow); }
     for (i = 0; i < 600; ++i) { fom_float v = work->t104[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t104[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
     /* t105 */
     for (i = 0; i < 20; ++i) work->t105[i] = FOM_MAT_t1_c[i];
+    { FOM_PROF_BEGIN(step_20_relu);
     fom_fused_t105(work->t101, work->t102, work->t104, work->t105);
+    FOM_PROF_END(step_20_relu); }
     for (i = 0; i < 20; ++i) if (work->t105[i] < 0.0) work->t105[i] = 0.0;
     /* t106 */
     for (i = 0; i < 30; ++i) work->t106[i] = pc[i];
+    { FOM_PROF_BEGIN(step_30_dual);
     fom_fused_t106(work->t101, work->t103, work->t104, work->t106);
+    FOM_PROF_END(step_30_dual); }
     /* t107 */
     for (i = 0; i < 600; ++i) work->t107[i] = FOM_MAT_t0_c[i];
+    { FOM_PROF_BEGIN(step_600_flow);
     fom_fused_t107(work->t104, work->t105, work->t106, work->t107);
+    FOM_PROF_END(step_600_flow); }
     for (i = 0; i < 600; ++i) { fom_float v = work->t107[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t107[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
     /* t108 */
     for (i = 0; i < 20; ++i) work->t108[i] = FOM_MAT_t1_c[i];
+    { FOM_PROF_BEGIN(step_20_relu);
     fom_fused_t108(work->t104, work->t105, work->t107, work->t108);
+    FOM_PROF_END(step_20_relu); }
     for (i = 0; i < 20; ++i) if (work->t108[i] < 0.0) work->t108[i] = 0.0;
     /* t109 */
     for (i = 0; i < 30; ++i) work->t109[i] = pc[i];
+    { FOM_PROF_BEGIN(step_30_dual);
     fom_fused_t109(work->t104, work->t106, work->t107, work->t109);
+    FOM_PROF_END(step_30_dual); }
     /* t110 */
     for (i = 0; i < 600; ++i) work->t110[i] = FOM_MAT_t0_c[i];
+    { FOM_PROF_BEGIN(step_600_flow);
     fom_fused_t110(work->t107, work->t108, work->t109, work->t110);
+    FOM_PROF_END(step_600_flow); }
     for (i = 0; i < 600; ++i) { fom_float v = work->t110[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t110[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
     /* t111 */
     for (i = 0; i < 20; ++i) work->t111[i] = FOM_MAT_t1_c[i];
+    { FOM_PROF_BEGIN(step_20_relu);
     fom_fused_t111(work->t107, work->t108, work->t110, work->t111);
+    FOM_PROF_END(step_20_relu); }
     for (i = 0; i < 20; ++i) if (work->t111[i] < 0.0) work->t111[i] = 0.0;
     /* t112 */
     for (i = 0; i < 30; ++i) work->t112[i] = pc[i];
+    { FOM_PROF_BEGIN(step_30_dual);
     fom_fused_t112(work->t107, work->t109, work->t110, work->t112);
+    FOM_PROF_END(step_30_dual); }
     /* t113 */
     for (i = 0; i < 600; ++i) work->t113[i] = FOM_MAT_t0_c[i];
+    { FOM_PROF_BEGIN(step_600_flow);
     fom_fused_t113(work->t110, work->t111, work->t112, work->t113);
+    FOM_PROF_END(step_600_flow); }
     for (i = 0; i < 600; ++i) { fom_float v = work->t113[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t113[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
     /* t114 */
     for (i = 0; i < 20; ++i) work->t114[i] = FOM_MAT_t1_c[i];
+    { FOM_PROF_BEGIN(step_20_relu);
     fom_fused_t114(work->t110, work->t111, work->t113, work->t114);
+    FOM_PROF_END(step_20_relu); }
     for (i = 0; i < 20; ++i) if (work->t114[i] < 0.0) work->t114[i] = 0.0;
     /* t115 */
     for (i = 0; i < 30; ++i) work->t115[i] = pc[i];
+    { FOM_PROF_BEGIN(step_30_dual);
     fom_fused_t115(work->t110, work->t112, work->t113, work->t115);
+    FOM_PROF_END(step_30_dual); }
     /* t116 */
     for (i = 0; i < 600; ++i) work->t116[i] = FOM_MAT_t0_c[i];
+    { FOM_PROF_BEGIN(step_600_flow);
     fom_fused_t116(work->t113, work->t114, work->t115, work->t116);
+    FOM_PROF_END(step_600_flow); }
     for (i = 0; i < 600; ++i) { fom_float v = work->t116[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t116[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
     /* t117 */
     for (i = 0; i < 20; ++i) work->t117[i] = FOM_MAT_t1_c[i];
+    { FOM_PROF_BEGIN(step_20_relu);
     fom_fused_t117(work->t113, work->t114, work->t116, work->t117);
+    FOM_PROF_END(step_20_relu); }
     for (i = 0; i < 20; ++i) if (work->t117[i] < 0.0) work->t117[i] = 0.0;
     /* t118 */
     for (i = 0; i < 30; ++i) work->t118[i] = pc[i];
+    { FOM_PROF_BEGIN(step_30_dual);
     fom_fused_t118(work->t113, work->t115, work->t116, work->t118);
+    FOM_PROF_END(step_30_dual); }
     /* t119 */
     for (i = 0; i < 600; ++i) work->t119[i] = FOM_MAT_t0_c[i];
+    { FOM_PROF_BEGIN(step_600_flow);
     fom_fused_t119(work->t116, work->t117, work->t118, work->t119);
+    FOM_PROF_END(step_600_flow); }
     for (i = 0; i < 600; ++i) { fom_float v = work->t119[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t119[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
     /* t120 */
     for (i = 0; i < 20; ++i) work->t120[i] = FOM_MAT_t1_c[i];
+    { FOM_PROF_BEGIN(step_20_relu);
     fom_fused_t120(work->t116, work->t117, work->t119, work->t120);
+    FOM_PROF_END(step_20_relu); }
     for (i = 0; i < 20; ++i) if (work->t120[i] < 0.0) work->t120[i] = 0.0;
     /* t121 */
     for (i = 0; i < 30; ++i) work->t121[i] = pc[i];
+    { FOM_PROF_BEGIN(step_30_dual);
     fom_fused_t121(work->t116, work->t118, work->t119, work->t121);
+    FOM_PROF_END(step_30_dual); }
     /* t122 */
     for (i = 0; i < 600; ++i) work->t122[i] = FOM_MAT_t0_c[i];
+    { FOM_PROF_BEGIN(step_600_flow);
     fom_fused_t122(work->t119, work->t120, work->t121, work->t122);
+    FOM_PROF_END(step_600_flow); }
     for (i = 0; i < 600; ++i) { fom_float v = work->t122[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t122[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
     /* t123 */
     for (i = 0; i < 20; ++i) work->t123[i] = FOM_MAT_t1_c[i];
+    { FOM_PROF_BEGIN(step_20_relu);
     fom_fused_t123(work->t119, work->t120, work->t122, work->t123);
+    FOM_PROF_END(step_20_relu); }
     for (i = 0; i < 20; ++i) if (work->t123[i] < 0.0) work->t123[i] = 0.0;
     /* t124 */
     for (i = 0; i < 30; ++i) work->t124[i] = pc[i];
+    { FOM_PROF_BEGIN(step_30_dual);
     fom_fused_t124(work->t119, work->t121, work->t122, work->t124);
+    FOM_PROF_END(step_30_dual); }
     /* t125 */
     for (i = 0; i < 600; ++i) work->t125[i] = FOM_MAT_t0_c[i];
+    { FOM_PROF_BEGIN(step_600_flow);
     fom_fused_t125(work->t122, work->t123, work->t124, work->t125);
+    FOM_PROF_END(step_600_flow); }
     for (i = 0; i < 600; ++i) { fom_float v = work->t125[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t125[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
     /* t126 */
     for (i = 0; i < 20; ++i) work->t126[i] = FOM_MAT_t1_c[i];
+    { FOM_PROF_BEGIN(step_20_relu);
     fom_fused_t126(work->t122, work->t123, work->t125, work->t126);
+    FOM_PROF_END(step_20_relu); }
     for (i = 0; i < 20; ++i) if (work->t126[i] < 0.0) work->t126[i] = 0.0;
     /* t127 */
     for (i = 0; i < 30; ++i) work->t127[i] = pc[i];
+    { FOM_PROF_BEGIN(step_30_dual);
     fom_fused_t127(work->t122, work->t124, work->t125, work->t127);
+    FOM_PROF_END(step_30_dual); }
     /* t128 */
     for (i = 0; i < 600; ++i) work->t128[i] = FOM_MAT_t0_c[i];
+    { FOM_PROF_BEGIN(step_600_flow);
     fom_fused_t128(work->t125, work->t126, work->t127, work->t128);
+    FOM_PROF_END(step_600_flow); }
     for (i = 0; i < 600; ++i) { fom_float v = work->t128[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t128[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
     /* t129 */
     for (i = 0; i < 20; ++i) work->t129[i] = FOM_MAT_t1_c[i];
+    { FOM_PROF_BEGIN(step_20_relu);
     fom_fused_t129(work->t125, work->t126, work->t128, work->t129);
+    FOM_PROF_END(step_20_relu); }
     for (i = 0; i < 20; ++i) if (work->t129[i] < 0.0) work->t129[i] = 0.0;
     /* t130 */
     for (i = 0; i < 30; ++i) work->t130[i] = pc[i];
+    { FOM_PROF_BEGIN(step_30_dual);
     fom_fused_t130(work->t125, work->t127, work->t128, work->t130);
+    FOM_PROF_END(step_30_dual); }
     /* t131 */
     for (i = 0; i < 600; ++i) work->t131[i] = FOM_MAT_t0_c[i];
+    { FOM_PROF_BEGIN(step_600_flow);
     fom_fused_t131(work->t128, work->t129, work->t130, work->t131);
+    FOM_PROF_END(step_600_flow); }
     for (i = 0; i < 600; ++i) { fom_float v = work->t131[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t131[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
     /* t132 */
     for (i = 0; i < 20; ++i) work->t132[i] = FOM_MAT_t1_c[i];
+    { FOM_PROF_BEGIN(step_20_relu);
     fom_fused_t132(work->t128, work->t129, work->t131, work->t132);
+    FOM_PROF_END(step_20_relu); }
     for (i = 0; i < 20; ++i) if (work->t132[i] < 0.0) work->t132[i] = 0.0;
     /* t133 */
     for (i = 0; i < 30; ++i) work->t133[i] = pc[i];
+    { FOM_PROF_BEGIN(step_30_dual);
     fom_fused_t133(work->t128, work->t130, work->t131, work->t133);
+    FOM_PROF_END(step_30_dual); }
     /* t134 */
     for (i = 0; i < 600; ++i) work->t134[i] = FOM_MAT_t0_c[i];
+    { FOM_PROF_BEGIN(step_600_flow);
     fom_fused_t134(work->t131, work->t132, work->t133, work->t134);
+    FOM_PROF_END(step_600_flow); }
     for (i = 0; i < 600; ++i) { fom_float v = work->t134[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t134[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
     /* t135 */
     for (i = 0; i < 20; ++i) work->t135[i] = FOM_MAT_t1_c[i];
+    { FOM_PROF_BEGIN(step_20_relu);
     fom_fused_t135(work->t131, work->t132, work->t134, work->t135);
+    FOM_PROF_END(step_20_relu); }
     for (i = 0; i < 20; ++i) if (work->t135[i] < 0.0) work->t135[i] = 0.0;
     /* t136 */
     for (i = 0; i < 30; ++i) work->t136[i] = pc[i];
+    { FOM_PROF_BEGIN(step_30_dual);
     fom_fused_t136(work->t131, work->t133, work->t134, work->t136);
+    FOM_PROF_END(step_30_dual); }
     /* t137 */
     for (i = 0; i < 600; ++i) work->t137[i] = FOM_MAT_t0_c[i];
+    { FOM_PROF_BEGIN(step_600_flow);
     fom_fused_t137(work->t134, work->t135, work->t136, work->t137);
+    FOM_PROF_END(step_600_flow); }
     for (i = 0; i < 600; ++i) { fom_float v = work->t137[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t137[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
     /* t138 */
     for (i = 0; i < 20; ++i) work->t138[i] = FOM_MAT_t1_c[i];
+    { FOM_PROF_BEGIN(step_20_relu);
     fom_fused_t138(work->t134, work->t135, work->t137, work->t138);
+    FOM_PROF_END(step_20_relu); }
     for (i = 0; i < 20; ++i) if (work->t138[i] < 0.0) work->t138[i] = 0.0;
     /* t139 */
     for (i = 0; i < 30; ++i) work->t139[i] = pc[i];
+    { FOM_PROF_BEGIN(step_30_dual);
     fom_fused_t139(work->t134, work->t136, work->t137, work->t139);
+    FOM_PROF_END(step_30_dual); }
     /* t140 */
     for (i = 0; i < 600; ++i) work->t140[i] = FOM_MAT_t0_c[i];
+    { FOM_PROF_BEGIN(step_600_flow);
     fom_fused_t140(work->t137, work->t138, work->t139, work->t140);
+    FOM_PROF_END(step_600_flow); }
     for (i = 0; i < 600; ++i) { fom_float v = work->t140[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t140[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
     /* t141 */
     for (i = 0; i < 20; ++i) work->t141[i] = FOM_MAT_t1_c[i];
+    { FOM_PROF_BEGIN(step_20_relu);
     fom_fused_t141(work->t137, work->t138, work->t140, work->t141);
+    FOM_PROF_END(step_20_relu); }
     for (i = 0; i < 20; ++i) if (work->t141[i] < 0.0) work->t141[i] = 0.0;
     /* t142 */
     for (i = 0; i < 30; ++i) work->t142[i] = pc[i];
+    { FOM_PROF_BEGIN(step_30_dual);
     fom_fused_t142(work->t137, work->t139, work->t140, work->t142);
+    FOM_PROF_END(step_30_dual); }
     /* t143 */
     for (i = 0; i < 600; ++i) work->t143[i] = FOM_MAT_t0_c[i];
+    { FOM_PROF_BEGIN(step_600_flow);
     fom_fused_t143(work->t140, work->t141, work->t142, work->t143);
+    FOM_PROF_END(step_600_flow); }
     for (i = 0; i < 600; ++i) { fom_float v = work->t143[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t143[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
     /* t144 */
     for (i = 0; i < 20; ++i) work->t144[i] = FOM_MAT_t1_c[i];
+    { FOM_PROF_BEGIN(step_20_relu);
     fom_fused_t144(work->t140, work->t141, work->t143, work->t144);
+    FOM_PROF_END(step_20_relu); }
     for (i = 0; i < 20; ++i) if (work->t144[i] < 0.0) work->t144[i] = 0.0;
     /* t145 */
     for (i = 0; i < 30; ++i) work->t145[i] = pc[i];
+    { FOM_PROF_BEGIN(step_30_dual);
     fom_fused_t145(work->t140, work->t142, work->t143, work->t145);
+    FOM_PROF_END(step_30_dual); }
     /* t146 */
     for (i = 0; i < 600; ++i) work->t146[i] = FOM_MAT_t0_c[i];
+    { FOM_PROF_BEGIN(step_600_flow);
     fom_fused_t146(work->t143, work->t144, work->t145, work->t146);
+    FOM_PROF_END(step_600_flow); }
     for (i = 0; i < 600; ++i) { fom_float v = work->t146[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t146[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
     /* t147 */
     for (i = 0; i < 20; ++i) work->t147[i] = FOM_MAT_t1_c[i];
+    { FOM_PROF_BEGIN(step_20_relu);
     fom_fused_t147(work->t143, work->t144, work->t146, work->t147);
+    FOM_PROF_END(step_20_relu); }
     for (i = 0; i < 20; ++i) if (work->t147[i] < 0.0) work->t147[i] = 0.0;
     /* t148 */
     for (i = 0; i < 30; ++i) work->t148[i] = pc[i];
+    { FOM_PROF_BEGIN(step_30_dual);
     fom_fused_t148(work->t143, work->t145, work->t146, work->t148);
+    FOM_PROF_END(step_30_dual); }
     r = 0.0;
     for (i = 0; i < 600; ++i) { d = work->t146[i] - work->state[i]; if (d < 0) d = -d; if (d > r) r = d; work->state[i] = work->t146[i]; }
     for (i = 0; i < 20; ++i) { d = work->t147[i] - work->state[600 + i]; if (d < 0) d = -d; if (d > r) r = d; work->state[600 + i] = work->t147[i]; }
--- src/fom_ops.c (translated)
+++ src/fom_ops.c (optimized)
@@ -23,7 +23,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t1_t_t0_ROWPTR[i]; p < FOM_MAT_t1_t_t0_ROWPTR[i + 1]; ++p) s += FOM_MAT_t1_t_t0_VALS[p] * x[FOM_MAT_t1_t_t0_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t1_t_t0_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -41,7 +41,7 @@
     fom_int i, p;
     for (i = 0; i < 600; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t2_p_p0_ROWPTR[i]; p < FOM_MAT_t2_p_p0_ROWPTR[i + 1]; ++p) s += FOM_MAT_t2_p_p0_VALS[p] * x[FOM_MAT_t2_p_p0_COLIDX[p]];
+        s += FOM_MAT_t2_p_p0_VALS[i] * x[FOM_MAT_t2_p_p0_COLIDX[i]];
         out[i] += s;
     }
 }
@@ -59,7 +59,7 @@
     fom_int i, p;
     for (i = 0; i < 600; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t2_t_t1_ROWPTR[i]; p < FOM_MAT_t2_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t2_t_t1_VALS[p] * x[FOM_MAT_t2_t_t1_COLIDX[p]];
+        s += FOM_MAT_t2_t_t1_VALS[i] * x[FOM_MAT_t2_t_t1_COLIDX[i]];
         out[i] += s;
     }
 }
@@ -68,7 +68,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t3_t_t0_ROWPTR[i]; p < FOM_MAT_t3_t_t0_ROWPTR[i + 1]; ++p) s += FOM_MAT_t3_t_t0_VALS[p] * x[FOM_MAT_t3_t_t0_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t3_t_t0_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -77,7 +77,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t3_t_t1_ROWPTR[i]; p < FOM_MAT_t3_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t3_t_t1_VALS[p] * x[FOM_MAT_t3_t_t1_COLIDX[p]];
+        s += FOM_MAT_t3_t_t1_VALS[i] * x[FOM_MAT_t3_t_t1_COLIDX[i]];
         out[i] += s;
     }
 }
@@ -86,7 +86,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t3_t_t2_ROWPTR[i]; p < FOM_MAT_t3_t_t2_ROWPTR[i + 1]; ++p) s += FOM_MAT_t3_t_t2_VALS[p] * x[FOM_MAT_t3_t_t2_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t3_t_t2_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -104,7 +104,7 @@
     fom_int i, p;
     for (i = 0; i < 30; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t4_p_p0_ROWPTR[i]; p < FOM_MAT_t4_p_p0_ROWPTR[i + 1]; ++p) s += FOM_MAT_t4_p_p0_VALS[p] * x[FOM_MAT_t4_p_p0_COLIDX[p]];
+        s += FOM_MAT_t4_p_p0_VALS[i] * x[FOM_MAT_t4_p_p0_COLIDX[i]];
         out[i] += s;
     }
 }
@@ -131,7 +131,7 @@
     fom_int i, p;
     for (i = 0; i < 600; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t5_t_t2_ROWPTR[i]; p < FOM_MAT_t5_t_t2_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t2_VALS[p] * x[FOM_MAT_t5_t_t2_COLIDX[p]];
+        s += FOM_MAT_t5_t_t2_VALS[i] * x[FOM_MAT_t5_t_t2_COLIDX[i]];
         out[i] += s;
     }
 }
@@ -140,7 +140,7 @@
     fom_int i, p;
     for (i = 0; i < 600; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t5_t_t4_ROWPTR[i]; p < FOM_MAT_t5_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t4_VALS[p] * x[FOM_MAT_t5_t_t4_COLIDX[p]];
+        s += FOM_MAT_t5_t_t4_VALS[i] * x[FOM_MAT_t5_t_t4_COLIDX[i]];
         out[i] += s;
     }
 }
@@ -149,7 +149,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t6_t_t2_ROWPTR[i]; p < FOM_MAT_t6_t_t2_ROWPTR[i + 1]; ++p) s += FOM_MAT_t6_t_t2_VALS[p] * x[FOM_MAT_t6_t_t2_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t6_t_t2_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -158,7 +158,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t6_t_t5_ROWPTR[i]; p < FOM_MAT_t6_t_t5_ROWPTR[i + 1]; ++p) s += FOM_MAT_t6_t_t5_VALS[p] * x[FOM_MAT_t6_t_t5_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t6_t_t5_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -167,7 +167,7 @@
     fom_int i, p;
     for (i = 0; i < 30; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t7_p_p0_ROWPTR[i]; p < FOM_MAT_t7_p_p0_ROWPTR[i + 1]; ++p) s += FOM_MAT_t7_p_p0_VALS[p] * x[FOM_MAT_t7_p_p0_COLIDX[p]];
+        s += FOM_MAT_t7_p_p0_VALS[i] * x[FOM_MAT_t7_p_p0_COLIDX[i]];
         out[i] += s;
     }
 }
@@ -185,7 +185,7 @@
     fom_int i, p;
     for (i = 0; i < 30; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t7_t_t4_ROWPTR[i]; p < FOM_MAT_t7_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t7_t_t4_VALS[p] * x[FOM_MAT_t7_t_t4_COLIDX[p]];
+        s += FOM_MAT_t7_t_t4_VALS[i] * x[FOM_MAT_t7_t_t4_COLIDX[i]];
         out[i] += s;
     }
 }
@@ -203,7 +203,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t9_t_t5_ROWPTR[i]; p < FOM_MAT_t9_t_t5_ROWPTR[i + 1]; ++p) s += FOM_MAT_t9_t_t5_VALS[p] * x[FOM_MAT_t9_t_t5_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t9_t_t5_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -212,7 +212,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t9_t_t8_ROWPTR[i]; p < FOM_MAT_t9_t_t8_ROWPTR[i + 1]; ++p) s += FOM_MAT_t9_t_t8_VALS[p] * x[FOM_MAT_t9_t_t8_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t9_t_t8_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -239,7 +239,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t12_t_t8_ROWPTR[i]; p < FOM_MAT_t12_t_t8_ROWPTR[i + 1]; ++p) s += FOM_MAT_t12_t_t8_VALS[p] * x[FOM_MAT_t12_t_t8_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t12_t_t8_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -248,7 +248,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t12_t_t11_ROWPTR[i]; p < FOM_MAT_t12_t_t11_ROWPTR[i + 1]; ++p) s += FOM_MAT_t12_t_t11_VALS[p] * x[FOM_MAT_t12_t_t11_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t12_t_t11_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -275,7 +275,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t15_t_t11_ROWPTR[i]; p < FOM_MAT_t15_t_t11_ROWPTR[i + 1]; ++p) s += FOM_MAT_t15_t_t11_VALS[p] * x[FOM_MAT_t15_t_t11_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t15_t_t11_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -284,7 +284,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t15_t_t14_ROWPTR[i]; p < FOM_MAT_t15_t_t14_ROWPTR[i + 1]; ++p) s += FOM_MAT_t15_t_t14_VALS[p] * x[FOM_MAT_t15_t_t14_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t15_t_t14_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -311,7 +311,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t18_t_t14_ROWPTR[i]; p < FOM_MAT_t18_t_t14_ROWPTR[i + 1]; ++p) s += FOM_MAT_t18_t_t14_VALS[p] * x[FOM_MAT_t18_t_t14_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t18_t_t14_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -320,7 +320,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t18_t_t17_ROWPTR[i]; p < FOM_MAT_t18_t_t17_ROWPTR[i + 1]; ++p) s += FOM_MAT_t18_t_t17_VALS[p] * x[FOM_MAT_t18_t_t17_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t18_t_t17_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -347,7 +347,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t21_t_t17_ROWPTR[i]; p < FOM_MAT_t21_t_t17_ROWPTR[i + 1]; ++p) s += FOM_MAT_t21_t_t17_VALS[p] * x[FOM_MAT_t21_t_t17_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t21_t_t17_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -356,7 +356,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t21_t_t20_ROWPTR[i]; p < FOM_MAT_t21_t_t20_ROWPTR[i + 1]; ++p) s += FOM_MAT_t21_t_t20_VALS[p] * x[FOM_MAT_t21_t_t20_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t21_t_t20_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -383,7 +383,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t24_t_t20_ROWPTR[i]; p < FOM_MAT_t24_t_t20_ROWPTR[i + 1]; ++p) s += FOM_MAT_t24_t_t20_VALS[p] * x[FOM_MAT_t24_t_t20_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t24_t_t20_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -392,7 +392,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t24_t_t23_ROWPTR[i]; p < FOM_MAT_t24_t_t23_ROWPTR[i + 1]; ++p) s += FOM_MAT_t24_t_t23_VALS[p] * x[FOM_MAT_t24_t_t23_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t24_t_t23_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -419,7 +419,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t27_t_t23_ROWPTR[i]; p < FOM_MAT_t27_t_t23_ROWPTR[i + 1]; ++p) s += FOM_MAT_t27_t_t23_VALS[p] * x[FOM_MAT_t27_t_t23_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t27_t_t23_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -428,7 +428,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t27_t_t26_ROWPTR[i]; p < FOM_MAT_t27_t_t26_ROWPTR[i + 1]; ++p) s += FOM_MAT_t27_t_t26_VALS[p] * x[FOM_MAT_t27_t_t26_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t27_t_t26_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -455,7 +455,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t30_t_t26_ROWPTR[i]; p < FOM_MAT_t30_t_t26_ROWPTR[i + 1]; ++p) s += FOM_MAT_t30_t_t26_VALS[p] * x[FOM_MAT_t30_t_t26_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t30_t_t26_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -464,7 +464,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t30_t_t29_ROWPTR[i]; p < FOM_MAT_t30_t_t29_ROWPTR[i + 1]; ++p) s += FOM_MAT_t30_t_t29_VALS[p] * x[FOM_MAT_t30_t_t29_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t30_t_t29_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -491,7 +491,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t33_t_t29_ROWPTR[i]; p < FOM_MAT_t33_t_t29_ROWPTR[i + 1]; ++p) s += FOM_MAT_t33_t_t29_VALS[p] * x[FOM_MAT_t33_t_t29_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t33_t_t29_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -500,7 +500,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t33_t_t32_ROWPTR[i]; p < FOM_MAT_t33_t_t32_ROWPTR[i + 1]; ++p) s += FOM_MAT_t33_t_t32_VALS[p] * x[FOM_MAT_t33_t_t32_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t33_t_t32_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -527,7 +527,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t36_t_t32_ROWPTR[i]; p < FOM_MAT_t36_t_t32_ROWPTR[i + 1]; ++p) s += FOM_MAT_t36_t_t32_VALS[p] * x[FOM_MAT_t36_t_t32_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t36_t_t32_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -536,7 +536,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t36_t_t35_ROWPTR[i]; p < FOM_MAT_t36_t_t35_ROWPTR[i + 1]; ++p) s += FOM_MAT_t36_t_t35_VALS[p] * x[FOM_MAT_t36_t_t35_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t36_t_t35_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -563,7 +563,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t39_t_t35_ROWPTR[i]; p < FOM_MAT_t39_t_t35_ROWPTR[i + 1]; ++p) s += FOM_MAT_t39_t_t35_VALS[p] * x[FOM_MAT_t39_t_t35_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t39_t_t35_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -572,7 +572,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t39_t_t38_ROWPTR[i]; p < FOM_MAT_t39_t_t38_ROWPTR[i + 1]; ++p) s += FOM_MAT_t39_t_t38_VALS[p] * x[FOM_MAT_t39_t_t38_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t39_t_t38_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -599,7 +599,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t42_t_t38_ROWPTR[i]; p < FOM_MAT_t42_t_t38_ROWPTR[i + 1]; ++p) s += FOM_MAT_t42_t_t38_VALS[p] * x[FOM_MAT_t42_t_t38_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t42_t_t38_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -608,7 +608,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t42_t_t41_ROWPTR[i]; p < FOM_MAT_t42_t_t41_ROWPTR[i + 1]; ++p) s += FOM_MAT_t42_t_t41_VALS[p] * x[FOM_MAT_t42_t_t41_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t42_t_t41_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -635,7 +635,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t45_t_t41_ROWPTR[i]; p < FOM_MAT_t45_t_t41_ROWPTR[i + 1]; ++p) s += FOM_MAT_t45_t_t41_VALS[p] * x[FOM_MAT_t45_t_t41_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t45_t_t41_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -644,7 +644,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t45_t_t44_ROWPTR[i]; p < FOM_MAT_t45_t_t44_ROWPTR[i + 1]; ++p) s += FOM_MAT_t45_t_t44_VALS[p] * x[FOM_MAT_t45_t_t44_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t45_t_t44_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -671,7 +671,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t48_t_t44_ROWPTR[i]; p < FOM_MAT_t48_t_t44_ROWPTR[i + 1]; ++p) s += FOM_MAT_t48_t_t44_VALS[p] * x[FOM_MAT_t48_t_t44_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t48_t_t44_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -680,7 +680,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t48_t_t47_ROWPTR[i]; p < FOM_MAT_t48_t_t47_ROWPTR[i + 1]; ++p) s += FOM_MAT_t48_t_t47_VALS[p] * x[FOM_MAT_t48_t_t47_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t48_t_t47_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -707,7 +707,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t51_t_t47_ROWPTR[i]; p < FOM_MAT_t51_t_t47_ROWPTR[i + 1]; ++p) s += FOM_MAT_t51_t_t47_VALS[p] * x[FOM_MAT_t51_t_t47_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t51_t_t47_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -716,7 +716,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t51_t_t50_ROWPTR[i]; p < FOM_MAT_t51_t_t50_ROWPTR[i + 1]; ++p) s += FOM_MAT_t51_t_t50_VALS[p] * x[FOM_MAT_t51_t_t50_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t51_t_t50_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -743,7 +743,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t54_t_t50_ROWPTR[i]; p < FOM_MAT_t54_t_t50_ROWPTR[i + 1]; ++p) s += FOM_MAT_t54_t_t50_VALS[p] * x[FOM_MAT_t54_t_t50_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t54_t_t50_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -752,7 +752,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t54_t_t53_ROWPTR[i]; p < FOM_MAT_t54_t_t53_ROWPTR[i + 1]; ++p) s += FOM_MAT_t54_t_t53_VALS[p] * x[FOM_MAT_t54_t_t53_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t54_t_t53_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -779,7 +779,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t57_t_t53_ROWPTR[i]; p < FOM_MAT_t57_t_t53_ROWPTR[i + 1]; ++p) s += FOM_MAT_t57_t_t53_VALS[p] * x[FOM_MAT_t57_t_t53_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t57_t_t53_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -788,7 +788,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t57_t_t56_ROWPTR[i]; p < FOM_MAT_t57_t_t56_ROWPTR[i + 1]; ++p) s += FOM_MAT_t57_t_t56_VALS[p] * x[FOM_MAT_t57_t_t56_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t57_t_t56_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -815,7 +815,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t60_t_t56_ROWPTR[i]; p < FOM_MAT_t60_t_t56_ROWPTR[i + 1]; ++p) s += FOM_MAT_t60_t_t56_VALS[p] * x[FOM_MAT_t60_t_t56_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t60_t_t56_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -824,7 +824,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t60_t_t59_ROWPTR[i]; p < FOM_MAT_t60_t_t59_ROWPTR[i + 1]; ++p) s += FOM_MAT_t60_t_t59_VALS[p] * x[FOM_MAT_t60_t_t59_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t60_t_t59_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -851,7 +851,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t63_t_t59_ROWPTR[i]; p < FOM_MAT_t63_t_t59_ROWPTR[i + 1]; ++p) s += FOM_MAT_t63_t_t59_VALS[p] * x[FOM_MAT_t63_t_t59_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t63_t_t59_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -860,7 +860,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t63_t_t62_ROWPTR[i]; p < FOM_MAT_t63_t_t62_ROWPTR[i + 1]; ++p) s += FOM_MAT_t63_t_t62_VALS[p] * x[FOM_MAT_t63_t_t62_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t63_t_t62_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -887,7 +887,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t66_t_t62_ROWPTR[i]; p < FOM_MAT_t66_t_t62_ROWPTR[i + 1]; ++p) s += FOM_MAT_t66_t_t62_VALS[p] * x[FOM_MAT_t66_t_t62_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t66_t_t62_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -896,7 +896,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t66_t_t65_ROWPTR[i]; p < FOM_MAT_t66_t_t65_ROWPTR[i + 1]; ++p) s += FOM_MAT_t66_t_t65_VALS[p] * x[FOM_MAT_t66_t_t65_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t66_t_t65_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -923,7 +923,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t69_t_t65_ROWPTR[i]; p < FOM_MAT_t69_t_t65_ROWPTR[i + 1]; ++p) s += FOM_MAT_t69_t_t65_VALS[p] * x[FOM_MAT_t69_t_t65_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t69_t_t65_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -932,7 +932,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t69_t_t68_ROWPTR[i]; p < FOM_MAT_t69_t_t68_ROWPTR[i + 1]; ++p) s += FOM_MAT_t69_t_t68_VALS[p] * x[FOM_MAT_t69_t_t68_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t69_t_t68_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -959,7 +959,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t72_t_t68_ROWPTR[i]; p < FOM_MAT_t72_t_t68_ROWPTR[i + 1]; ++p) s += FOM_MAT_t72_t_t68_VALS[p] * x[FOM_MAT_t72_t_t68_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t72_t_t68_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -968,7 +968,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t72_t_t71_ROWPTR[i]; p < FOM_MAT_t72_t_t71_ROWPTR[i + 1]; ++p) s += FOM_MAT_t72_t_t71_VALS[p] * x[FOM_MAT_t72_t_t71_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t72_t_t71_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -995,7 +995,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t75_t_t71_ROWPTR[i]; p < FOM_MAT_t75_t_t71_ROWPTR[i + 1]; ++p) s += FOM_MAT_t75_t_t71_VALS[p] * x[FOM_MAT_t75_t_t71_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t75_t_t71_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -1004,7 +1004,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t75_t_t74_ROWPTR[i]; p < FOM_MAT_t75_t_t74_ROWPTR[i + 1]; ++p) s += FOM_MAT_t75_t_t74_VALS[p] * x[FOM_MAT_t75_t_t74_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t75_t_t74_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -1031,7 +1031,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t78_t_t74_ROWPTR[i]; p < FOM_MAT_t78_t_t74_ROWPTR[i + 1]; ++p) s += FOM_MAT_t78_t_t74_VALS[p] * x[FOM_MAT_t78_t_t74_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t78_t_t74_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -1040,7 +1040,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t78_t_t77_ROWPTR[i]; p < FOM_MAT_t78_t_t77_ROWPTR[i + 1]; ++p) s += FOM_MAT_t78_t_t77_VALS[p] * x[FOM_MAT_t78_t_t77_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t78_t_t77_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -1067,7 +1067,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t81_t_t77_ROWPTR[i]; p < FOM_MAT_t81_t_t77_ROWPTR[i + 1]; ++p) s += FOM_MAT_t81_t_t77_VALS[p] * x[FOM_MAT_t81_t_t77_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t81_t_t77_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -1076,7 +1076,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t81_t_t80_ROWPTR[i]; p < FOM_MAT_t81_t_t80_ROWPTR[i + 1]; ++p) s += FOM_MAT_t81_t_t80_VALS[p] * x[FOM_MAT_t81_t_t80_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t81_t_t80_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -1103,7 +1103,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t84_t_t80_ROWPTR[i]; p < FOM_MAT_t84_t_t80_ROWPTR[i + 1]; ++p) s += FOM_MAT_t84_t_t80_VALS[p] * x[FOM_MAT_t84_t_t80_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t84_t_t80_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -1112,7 +1112,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t84_t_t83_ROWPTR[i]; p < FOM_MAT_t84_t_t83_ROWPTR[i + 1]; ++p) s += FOM_MAT_t84_t_t83_VALS[p] * x[FOM_MAT_t84_t_t83_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t84_t_t83_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -1139,7 +1139,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t87_t_t83_ROWPTR[i]; p < FOM_MAT_t87_t_t83_ROWPTR[i + 1]; ++p) s += FOM_MAT_t87_t_t83_VALS[p] * x[FOM_MAT_t87_t_t83_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t87_t_t83_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -1148,7 +1148,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t87_t_t86_ROWPTR[i]; p < FOM_MAT_t87_t_t86_ROWPTR[i + 1]; ++p) s += FOM_MAT_t87_t_t86_VALS[p] * x[FOM_MAT_t87_t_t86_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t87_t_t86_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -1175,7 +1175,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t90_t_t86_ROWPTR[i]; p < FOM_MAT_t90_t_t86_ROWPTR[i + 1]; ++p) s += FOM_MAT_t90_t_t86_VALS[p] * x[FOM_MAT_t90_t_t86_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t90_t_t86_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -1184,7 +1184,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t90_t_t89_ROWPTR[i]; p < FOM_MAT_t90_t_t89_ROWPTR[i + 1]; ++p) s += FOM_MAT_t90_t_t89_VALS[p] * x[FOM_MAT_t90_t_t89_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t90_t_t89_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -1211,7 +1211,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t93_t_t89_ROWPTR[i]; p < FOM_MAT_t93_t_t89_ROWPTR[i + 1]; ++p) s += FOM_MAT_t93_t_t89_VALS[p] * x[FOM_MAT_t93_t_t89_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t93_t_t89_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -1220,7 +1220,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t93_t_t92_ROWPTR[i]; p < FOM_MAT_t93_t_t92_ROWPTR[i + 1]; ++p) s += FOM_MAT_t93_t_t92_VALS[p] * x[FOM_MAT_t93_t_t92_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t93_t_t92_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -1247,7 +1247,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t96_t_t92_ROWPTR[i]; p < FOM_MAT_t96_t_t92_ROWPTR[i + 1]; ++p) s += FOM_MAT_t96_t_t92_VALS[p] * x[FOM_MAT_t96_t_t92_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t96_t_t92_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -1256,7 +1256,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t96_t_t95_ROWPTR[i]; p < FOM_MAT_t96_t_t95_ROWPTR[i + 1]; ++p) s += FOM_MAT_t96_t_t95_VALS[p] * x[FOM_MAT_t96_t_t95_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t96_t_t95_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -1283,7 +1283,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t99_t_t95_ROWPTR[i]; p < FOM_MAT_t99_t_t95_ROWPTR[i + 1]; ++p) s += FOM_MAT_t99_t_t95_VALS[p] * x[FOM_MAT_t99_t_t95_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t99_t_t95_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -1292,7 +1292,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t99_t_t98_ROWPTR[i]; p < FOM_MAT_t99_t_t98_ROWPTR[i + 1]; ++p) s += FOM_MAT_t99_t_t98_VALS[p] * x[FOM_MAT_t99_t_t98_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t99_t_t98_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -1319,7 +1319,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t102_t_t98_ROWPTR[i]; p < FOM_MAT_t102_t_t98_ROWPTR[i + 1]; ++p) s += FOM_MAT_t102_t_t98_VALS[p] * x[FOM_MAT_t102_t_t98_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t102_t_t98_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -1328,7 +1328,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t102_t_t101_ROWPTR[i]; p < FOM_MAT_t102_t_t101_ROWPTR[i + 1]; ++p) s += FOM_MAT_t102_t_t101_VALS[p] * x[FOM_MAT_t102_t_t101_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t102_t_t101_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -1355,7 +1355,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t105_t_t101_ROWPTR[i]; p < FOM_MAT_t105_t_t101_ROWPTR[i + 1]; ++p) s += FOM_MAT_t105_t_t101_VALS[p] * x[FOM_MAT_t105_t_t101_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t105_t_t101_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -1364,7 +1364,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t105_t_t104_ROWPTR[i]; p < FOM_MAT_t105_t_t104_ROWPTR[i + 1]; ++p) s += FOM_MAT_t105_t_t104_VALS[p] * x[FOM_MAT_t105_t_t104_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t105_t_t104_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -1391,7 +1391,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t108_t_t104_ROWPTR[i]; p < FOM_MAT_t108_t_t104_ROWPTR[i + 1]; ++p) s += FOM_MAT_t108_t_t104_VALS[p] * x[FOM_MAT_t108_t_t104_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t108_t_t104_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -1400,7 +1400,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t108_t_t107_ROWPTR[i]; p < FOM_MAT_t108_t_t107_ROWPTR[i + 1]; ++p) s += FOM_MAT_t108_t_t107_VALS[p] * x[FOM_MAT_t108_t_t107_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t108_t_t107_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -1427,7 +1427,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t111_t_t107_ROWPTR[i]; p < FOM_MAT_t111_t_t107_ROWPTR[i + 1]; ++p) s += FOM_MAT_t111_t_t107_VALS[p] * x[FOM_MAT_t111_t_t107_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t111_t_t107_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -1436,7 +1436,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t111_t_t110_ROWPTR[i]; p < FOM_MAT_t111_t_t110_ROWPTR[i + 1]; ++p) s += FOM_MAT_t111_t_t110_VALS[p] * x[FOM_MAT_t111_t_t110_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t111_t_t110_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -1463,7 +1463,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t114_t_t110_ROWPTR[i]; p < FOM_MAT_t114_t_t110_ROWPTR[i + 1]; ++p) s += FOM_MAT_t114_t_t110_VALS[p] * x[FOM_MAT_t114_t_t110_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t114_t_t110_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -1472,7 +1472,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t114_t_t113_ROWPTR[i]; p < FOM_MAT_t114_t_t113_ROWPTR[i + 1]; ++p) s += FOM_MAT_t114_t_t113_VALS[p] * x[FOM_MAT_t114_t_t113_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t114_t_t113_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -1499,7 +1499,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t117_t_t113_ROWPTR[i]; p < FOM_MAT_t117_t_t113_ROWPTR[i + 1]; ++p) s += FOM_MAT_t117_t_t113_VALS[p] * x[FOM_MAT_t117_t_t113_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t117_t_t113_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -1508,7 +1508,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t117_t_t116_ROWPTR[i]; p < FOM_MAT_t117_t_t116_ROWPTR[i + 1]; ++p) s += FOM_MAT_t117_t_t116_VALS[p] * x[FOM_MAT_t117_t_t116_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t117_t_t116_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -1535,7 +1535,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t120_t_t116_ROWPTR[i]; p < FOM_MAT_t120_t_t116_ROWPTR[i + 1]; ++p) s += FOM_MAT_t120_t_t116_VALS[p] * x[FOM_MAT_t120_t_t116_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t120_t_t116_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -1544,7 +1544,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t120_t_t119_ROWPTR[i]; p < FOM_MAT_t120_t_t119_ROWPTR[i + 1]; ++p) s += FOM_MAT_t120_t_t119_VALS[p] * x[FOM_MAT_t120_t_t119_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t120_t_t119_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -1571,7 +1571,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t123_t_t119_ROWPTR[i]; p < FOM_MAT_t123_t_t119_ROWPTR[i + 1]; ++p) s += FOM_MAT_t123_t_t119_VALS[p] * x[FOM_MAT_t123_t_t119_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t123_t_t119_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -1580,7 +1580,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t123_t_t122_ROWPTR[i]; p < FOM_MAT_t123_t_t122_ROWPTR[i + 1]; ++p) s += FOM_MAT_t123_t_t122_VALS[p] * x[FOM_MAT_t123_t_t122_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t123_t_t122_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -1607,7 +1607,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t126_t_t122_ROWPTR[i]; p < FOM_MAT_t126_t_t122_ROWPTR[i + 1]; ++p) s += FOM_MAT_t126_t_t122_VALS[p] * x[FOM_MAT_t126_t_t122_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t126_t_t122_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -1616,7 +1616,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t126_t_t125_ROWPTR[i]; p < FOM_MAT_t126_t_t125_ROWPTR[i + 1]; ++p) s += FOM_MAT_t126_t_t125_VALS[p] * x[FOM_MAT_t126_t_t125_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t126_t_t125_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -1643,7 +1643,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t129_t_t125_ROWPTR[i]; p < FOM_MAT_t129_t_t125_ROWPTR[i + 1]; ++p) s += FOM_MAT_t129_t_t125_VALS[p] * x[FOM_MAT_t129_t_t125_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t129_t_t125_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -1652,7 +1652,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t129_t_t128_ROWPTR[i]; p < FOM_MAT_t129_t_t128_ROWPTR[i + 1]; ++p) s += FOM_MAT_t129_t_t128_VALS[p] * x[FOM_MAT_t129_t_t128_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t129_t_t128_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -1679,7 +1679,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t132_t_t128_ROWPTR[i]; p < FOM_MAT_t132_t_t128_ROWPTR[i + 1]; ++p) s += FOM_MAT_t132_t_t128_VALS[p] * x[FOM_MAT_t132_t_t128_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t132_t_t128_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -1688,7 +1688,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t132_t_t131_ROWPTR[i]; p < FOM_MAT_t132_t_t131_ROWPTR[i + 1]; ++p) s += FOM_MAT_t132_t_t131_VALS[p] * x[FOM_MAT_t132_t_t131_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t132_t_t131_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -1715,7 +1715,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t135_t_t131_ROWPTR[i]; p < FOM_MAT_t135_t_t131_ROWPTR[i + 1]; ++p) s += FOM_MAT_t135_t_t131_VALS[p] * x[FOM_MAT_t135_t_t131_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t135_t_t131_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -1724,7 +1724,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t135_t_t134_ROWPTR[i]; p < FOM_MAT_t135_t_t134_ROWPTR[i + 1]; ++p) s += FOM_MAT_t135_t_t134_VALS[p] * x[FOM_MAT_t135_t_t134_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t135_t_t134_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -1751,7 +1751,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t138_t_t134_ROWPTR[i]; p < FOM_MAT_t138_t_t134_ROWPTR[i + 1]; ++p) s += FOM_MAT_t138_t_t134_VALS[p] * x[FOM_MAT_t138_t_t134_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t138_t_t134_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -1760,7 +1760,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t138_t_t137_ROWPTR[i]; p < FOM_MAT_t138_t_t137_ROWPTR[i + 1]; ++p) s += FOM_MAT_t138_t_t137_VALS[p] * x[FOM_MAT_t138_t_t137_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t138_t_t137_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -1787,7 +1787,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t141_t_t137_ROWPTR[i]; p < FOM_MAT_t141_t_t137_ROWPTR[i + 1]; ++p) s += FOM_MAT_t141_t_t137_VALS[p] * x[FOM_MAT_t141_t_t137_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t141_t_t137_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -1796,7 +1796,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t141_t_t140_ROWPTR[i]; p < FOM_MAT_t141_t_t140_ROWPTR[i + 1]; ++p) s += FOM_MAT_t141_t_t140_VALS[p] * x[FOM_MAT_t141_t_t140_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t141_t_t140_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -1823,7 +1823,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t144_t_t140_ROWPTR[i]; p < FOM_MAT_t144_t_t140_ROWPTR[i + 1]; ++p) s += FOM_MAT_t144_t_t140_VALS[p] * x[FOM_MAT_t144_t_t140_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t144_t_t140_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -1832,7 +1832,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t144_t_t143_ROWPTR[i]; p < FOM_MAT_t144_t_t143_ROWPTR[i + 1]; ++p) s += FOM_MAT_t144_t_t143_VALS[p] * x[FOM_MAT_t144_t_t143_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t144_t_t143_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -1859,7 +1859,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t147_t_t143_ROWPTR[i]; p < FOM_MAT_t147_t_t143_ROWPTR[i + 1]; ++p) s += FOM_MAT_t147_t_t143_VALS[p] * x[FOM_MAT_t147_t_t143_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t147_t_t143_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -1868,7 +1868,7 @@
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s = 0.0;
-        for (p = FOM_MAT_t147_t_t146_ROWPTR[i]; p < FOM_MAT_t147_t_t146_ROWPTR[i + 1]; ++p) s += FOM_MAT_t147_t_t146_VALS[p] * x[FOM_MAT_t147_t_t146_COLIDX[p]];
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t147_t_t146_VALS[p] * x[p];
         out[i] += s;
     }
 }
@@ -1918,7 +1918,7 @@
     }
 }
 
-void fom_fused_t1(const fom_float *state, const fom_float *t_t0, fom_float *out) {
+void fom_fused_t1(const fom_float *restrict state, const fom_float *restrict t_t0, fom_float *restrict out) {
     fom_int i, p;
     for (i = 0; i < 20; ++i) {
         fom_float s;
@@ -1926,12 +1926,12 @@
         for (p = FOM_MAT_t1_state_ROWPTR[i]; p < FOM_MAT_t1_state_ROWPTR[i + 1]; ++p) s += FOM_MAT_t1_state_VALS[p] * state[FOM_MAT_t1_state_COLIDX[p]];
         out[i] += s;
         s = 0.0;
-        for (p = FOM_MAT_t1_t_t0_ROWPTR[i]; p < FOM_MAT_t1_t_t0_ROWPTR[i + 1]; ++p) s += FOM_MAT_t1_t_t0_VALS[p] * t_t0[FOM_MAT_t1_t_t0_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t2(const fom_float *state, const fom_float *t_t0, const fom_float *t_t1, fom_float *out) {
+        for (p = 30 * i; p < 30 * i + 30; ++p) s += FOM_MAT_t1_t_t0_VALS[p] * t_t0[p];
+        out[i] += s;
+    }
+}
+
+void fom_fused_t2(const fom_float *restrict state, const fom_float *restrict t_t0, const fom_float *restrict t_t1, fom_float *restrict out) {
     fom_int i, p;
     for (i = 0; i < 600; ++i) {
         fom_float s;
@@ -1942,2348 +1942,1718 @@
         for (p = FOM_MAT_t2_t_t0_ROWPTR[i]; p < FOM_MAT_t2_t_t0_ROWPTR[i + 1]; ++p) s += FOM_MAT_t2_t_t0_VALS[p] * t_t0[FOM_MAT_t2_t_t0_COLIDX[p]];
         out[i] += s;
         s = 0.0;
-        for (p = FOM_MAT_t2_t_t1_ROWPTR[i]; p < FOM_MAT_t2_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t2_t_t1_VALS[p] * t_t1[FOM_MAT_t2_t_t1_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t3(const fom_float *t_t0, const fom_float *t_t1, const fom_float *t_t2, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 20; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t3_t_t0_ROWPTR[i]; p < FOM_MAT_t3_t_t0_ROWPTR[i + 1]; ++p) s += FOM_MAT_t3_t_t0_VALS[p] * t_t0[FOM_MAT_t3_t_t0_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t3_t_t1_ROWPTR[i]; p < FOM_MAT_t3_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t3_t_t1_VALS[p] * t_t1[FOM_MAT_t3_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t3_t_t2_ROWPTR[i]; p < FOM_MAT_t3_t_t2_ROWPTR[i + 1]; ++p) s += FOM_MAT_t3_t_t2_VALS[p] * t_t2[FOM_MAT_t3_t_t2_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t4(const fom_float *state, const fom_float *t_t0, const fom_float *t_t2, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 30; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t4_state_ROWPTR[i]; p < FOM_MAT_t4_state_ROWPTR[i + 1]; ++p) s += FOM_MAT_t4_state_VALS[p] * state[FOM_MAT_t4_state_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t4_t_t0_ROWPTR[i]; p < FOM_MAT_t4_t_t0_ROWPTR[i + 1]; ++p) s += FOM_MAT_t4_t_t0_VALS[p] * t_t0[FOM_MAT_t4_t_t0_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t4_t_t2_ROWPTR[i]; p < FOM_MAT_t4_t_t2_ROWPTR[i + 1]; ++p) s += FOM_MAT_t4_t_t2_VALS[p] * t_t2[FOM_MAT_t4_t_t2_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t5(const fom_float *t_t2, const fom_float *t_t3, const fom_float *t_t4, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 600; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t2_ROWPTR[i]; p < FOM_MAT_t5_t_t2_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t2_VALS[p] * t_t2[FOM_MAT_t5_t_t2_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t2_t_t1_ROWPTR[i]; p < FOM_MAT_t2_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t2_t_t1_VALS[p] * t_t3[FOM_MAT_t2_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t4_ROWPTR[i]; p < FOM_MAT_t5_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t4_VALS[p] * t_t4[FOM_MAT_t5_t_t4_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t6(const fom_float *t_t2, const fom_float *t_t3, const fom_float *t_t5, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 20; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t6_t_t2_ROWPTR[i]; p < FOM_MAT_t6_t_t2_ROWPTR[i + 1]; ++p) s += FOM_MAT_t6_t_t2_VALS[p] * t_t2[FOM_MAT_t6_t_t2_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t3_t_t1_ROWPTR[i]; p < FOM_MAT_t3_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t3_t_t1_VALS[p] * t_t3[FOM_MAT_t3_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t6_t_t5_ROWPTR[i]; p < FOM_MAT_t6_t_t5_ROWPTR[i + 1]; ++p) s += FOM_MAT_t6_t_t5_VALS[p] * t_t5[FOM_MAT_t6_t_t5_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t7(const fom_float *t_t2, const fom_float *t_t4, const fom_float *t_t5, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 30; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t7_t_t2_ROWPTR[i]; p < FOM_MAT_t7_t_t2_ROWPTR[i + 1]; ++p) s += FOM_MAT_t7_t_t2_VALS[p] * t_t2[FOM_MAT_t7_t_t2_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t7_t_t4_ROWPTR[i]; p < FOM_MAT_t7_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t7_t_t4_VALS[p] * t_t4[FOM_MAT_t7_t_t4_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t7_t_t5_ROWPTR[i]; p < FOM_MAT_t7_t_t5_ROWPTR[i + 1]; ++p) s += FOM_MAT_t7_t_t5_VALS[p] * t_t5[FOM_MAT_t7_t_t5_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t8(const fom_float *t_t5, const fom_float *t_t6, const fom_float *t_t7, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 600; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t2_ROWPTR[i]; p < FOM_MAT_t5_t_t2_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t2_VALS[p] * t_t5[FOM_MAT_t5_t_t2_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t2_t_t1_ROWPTR[i]; p < FOM_MAT_t2_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t2_t_t1_VALS[p] * t_t6[FOM_MAT_t2_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t4_ROWPTR[i]; p < FOM_MAT_t5_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t4_VALS[p] * t_t7[FOM_MAT_t5_t_t4_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t9(const fom_float *t_t5, const fom_float *t_t6, const fom_float *t_t8, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 20; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t9_t_t5_ROWPTR[i]; p < FOM_MAT_t9_t_t5_ROWPTR[i + 1]; ++p) s += FOM_MAT_t9_t_t5_VALS[p] * t_t5[FOM_MAT_t9_t_t5_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t3_t_t1_ROWPTR[i]; p < FOM_MAT_t3_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t3_t_t1_VALS[p] * t_t6[FOM_MAT_t3_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t9_t_t8_ROWPTR[i]; p < FOM_MAT_t9_t_t8_ROWPTR[i + 1]; ++p) s += FOM_MAT_t9_t_t8_VALS[p] * t_t8[FOM_MAT_t9_t_t8_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t10(const fom_float *t_t5, const fom_float *t_t7, const fom_float *t_t8, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 30; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t10_t_t5_ROWPTR[i]; p < FOM_MAT_t10_t_t5_ROWPTR[i + 1]; ++p) s += FOM_MAT_t10_t_t5_VALS[p] * t_t5[FOM_MAT_t10_t_t5_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t7_t_t4_ROWPTR[i]; p < FOM_MAT_t7_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t7_t_t4_VALS[p] * t_t7[FOM_MAT_t7_t_t4_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t10_t_t8_ROWPTR[i]; p < FOM_MAT_t10_t_t8_ROWPTR[i + 1]; ++p) s += FOM_MAT_t10_t_t8_VALS[p] * t_t8[FOM_MAT_t10_t_t8_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t11(const fom_float *t_t8, const fom_float *t_t9, const fom_float *t_t10, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 600; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t2_ROWPTR[i]; p < FOM_MAT_t5_t_t2_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t2_VALS[p] * t_t8[FOM_MAT_t5_t_t2_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t2_t_t1_ROWPTR[i]; p < FOM_MAT_t2_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t2_t_t1_VALS[p] * t_t9[FOM_MAT_t2_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t4_ROWPTR[i]; p < FOM_MAT_t5_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t4_VALS[p] * t_t10[FOM_MAT_t5_t_t4_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t12(const fom_float *t_t8, const fom_float *t_t9, const fom_float *t_t11, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 20; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t12_t_t8_ROWPTR[i]; p < FOM_MAT_t12_t_t8_ROWPTR[i + 1]; ++p) s += FOM_MAT_t12_t_t8_VALS[p] * t_t8[FOM_MAT_t12_t_t8_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t3_t_t1_ROWPTR[i]; p < FOM_MAT_t3_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t3_t_t1_VALS[p] * t_t9[FOM_MAT_t3_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t12_t_t11_ROWPTR[i]; p < FOM_MAT_t12_t_t11_ROWPTR[i + 1]; ++p) s += FOM_MAT_t12_t_t11_VALS[p] * t_t11[FOM_MAT_t12_t_t11_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t13(const fom_float *t_t8, const fom_float *t_t10, const fom_float *t_t11, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 30; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t13_t_t8_ROWPTR[i]; p < FOM_MAT_t13_t_t8_ROWPTR[i + 1]; ++p) s += FOM_MAT_t13_t_t8_VALS[p] * t_t8[FOM_MAT_t13_t_t8_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t7_t_t4_ROWPTR[i]; p < FOM_MAT_t7_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t7_t_t4_VALS[p] * t_t10[FOM_MAT_t7_t_t4_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t13_t_t11_ROWPTR[i]; p < FOM_MAT_t13_t_t11_ROWPTR[i + 1]; ++p) s += FOM_MAT_t13_t_t11_VALS[p] * t_t11[FOM_MAT_t13_t_t11_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t14(const fom_float *t_t11, const fom_float *t_t12, const fom_float *t_t13, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 600; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t2_ROWPTR[i]; p < FOM_MAT_t5_t_t2_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t2_VALS[p] * t_t11[FOM_MAT_t5_t_t2_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t2_t_t1_ROWPTR[i]; p < FOM_MAT_t2_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t2_t_t1_VALS[p] * t_t12[FOM_MAT_t2_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t4_ROWPTR[i]; p < FOM_MAT_t5_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t4_VALS[p] * t_t13[FOM_MAT_t5_t_t4_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t15(const fom_float *t_t11, const fom_float *t_t12, const fom_float *t_t14, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 20; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t15_t_t11_ROWPTR[i]; p < FOM_MAT_t15_t_t11_ROWPTR[i + 1]; ++p) s += FOM_MAT_t15_t_t11_VALS[p] * t_t11[FOM_MAT_t15_t_t11_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t3_t_t1_ROWPTR[i]; p < FOM_MAT_t3_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t3_t_t1_VALS[p] * t_t12[FOM_MAT_t3_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t15_t_t14_ROWPTR[i]; p < FOM_MAT_t15_t_t14_ROWPTR[i + 1]; ++p) s += FOM_MAT_t15_t_t14_VALS[p] * t_t14[FOM_MAT_t15_t_t14_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t16(const fom_float *t_t11, const fom_float *t_t13, const fom_float *t_t14, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 30; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t16_t_t11_ROWPTR[i]; p < FOM_MAT_t16_t_t11_ROWPTR[i + 1]; ++p) s += FOM_MAT_t16_t_t11_VALS[p] * t_t11[FOM_MAT_t16_t_t11_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t7_t_t4_ROWPTR[i]; p < FOM_MAT_t7_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t7_t_t4_VALS[p] * t_t13[FOM_MAT_t7_t_t4_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t16_t_t14_ROWPTR[i]; p < FOM_MAT_t16_t_t14_ROWPTR[i + 1]; ++p) s += FOM_MAT_t16_t_t14_VALS[p] * t_t14[FOM_MAT_t16_t_t14_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t17(const fom_float *t_t14, const fom_float *t_t15, const fom_float *t_t16, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 600; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t2_ROWPTR[i]; p < FOM_MAT_t5_t_t2_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t2_VALS[p] * t_t14[FOM_MAT_t5_t_t2_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t2_t_t1_ROWPTR[i]; p < FOM_MAT_t2_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t2_t_t1_VALS[p] * t_t15[FOM_MAT_t2_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t4_ROWPTR[i]; p < FOM_MAT_t5_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t4_VALS[p] * t_t16[FOM_MAT_t5_t_t4_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t18(const fom_float *t_t14, const fom_float *t_t15, const fom_float *t_t17, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 20; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t18_t_t14_ROWPTR[i]; p < FOM_MAT_t18_t_t14_ROWPTR[i + 1]; ++p) s += FOM_MAT_t18_t_t14_VALS[p] * t_t14[FOM_MAT_t18_t_t14_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t3_t_t1_ROWPTR[i]; p < FOM_MAT_t3_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t3_t_t1_VALS[p] * t_t15[FOM_MAT_t3_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t18_t_t17_ROWPTR[i]; p < FOM_MAT_t18_t_t17_ROWPTR[i + 1]; ++p) s += FOM_MAT_t18_t_t17_VALS[p] * t_t17[FOM_MAT_t18_t_t17_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t19(const fom_float *t_t14, const fom_float *t_t16, const fom_float *t_t17, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 30; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t19_t_t14_ROWPTR[i]; p < FOM_MAT_t19_t_t14_ROWPTR[i + 1]; ++p) s += FOM_MAT_t19_t_t14_VALS[p] * t_t14[FOM_MAT_t19_t_t14_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t7_t_t4_ROWPTR[i]; p < FOM_MAT_t7_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t7_t_t4_VALS[p] * t_t16[FOM_MAT_t7_t_t4_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t19_t_t17_ROWPTR[i]; p < FOM_MAT_t19_t_t17_ROWPTR[i + 1]; ++p) s += FOM_MAT_t19_t_t17_VALS[p] * t_t17[FOM_MAT_t19_t_t17_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t20(const fom_float *t_t17, const fom_float *t_t18, const fom_float *t_t19, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 600; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t2_ROWPTR[i]; p < FOM_MAT_t5_t_t2_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t2_VALS[p] * t_t17[FOM_MAT_t5_t_t2_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t2_t_t1_ROWPTR[i]; p < FOM_MAT_t2_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t2_t_t1_VALS[p] * t_t18[FOM_MAT_t2_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t4_ROWPTR[i]; p < FOM_MAT_t5_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t4_VALS[p] * t_t19[FOM_MAT_t5_t_t4_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t21(const fom_float *t_t17, const fom_float *t_t18, const fom_float *t_t20, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 20; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t21_t_t17_ROWPTR[i]; p < FOM_MAT_t21_t_t17_ROWPTR[i + 1]; ++p) s += FOM_MAT_t21_t_t17_VALS[p] * t_t17[FOM_MAT_t21_t_t17_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t3_t_t1_ROWPTR[i]; p < FOM_MAT_t3_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t3_t_t1_VALS[p] * t_t18[FOM_MAT_t3_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t21_t_t20_ROWPTR[i]; p < FOM_MAT_t21_t_t20_ROWPTR[i + 1]; ++p) s += FOM_MAT_t21_t_t20_VALS[p] * t_t20[FOM_MAT_t21_t_t20_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t22(const fom_float *t_t17, const fom_float *t_t19, const fom_float *t_t20, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 30; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t22_t_t17_ROWPTR[i]; p < FOM_MAT_t22_t_t17_ROWPTR[i + 1]; ++p) s += FOM_MAT_t22_t_t17_VALS[p] * t_t17[FOM_MAT_t22_t_t17_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t7_t_t4_ROWPTR[i]; p < FOM_MAT_t7_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t7_t_t4_VALS[p] * t_t19[FOM_MAT_t7_t_t4_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t22_t_t20_ROWPTR[i]; p < FOM_MAT_t22_t_t20_ROWPTR[i + 1]; ++p) s += FOM_MAT_t22_t_t20_VALS[p] * t_t20[FOM_MAT_t22_t_t20_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t23(const fom_float *t_t20, const fom_float *t_t21, const fom_float *t_t22, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 600; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t2_ROWPTR[i]; p < FOM_MAT_t5_t_t2_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t2_VALS[p] * t_t20[FOM_MAT_t5_t_t2_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t2_t_t1_ROWPTR[i]; p < FOM_MAT_t2_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t2_t_t1_VALS[p] * t_t21[FOM_MAT_t2_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t4_ROWPTR[i]; p < FOM_MAT_t5_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t4_VALS[p] * t_t22[FOM_MAT_t5_t_t4_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t24(const fom_float *t_t20, const fom_float *t_t21, const fom_float *t_t23, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 20; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t24_t_t20_ROWPTR[i]; p < FOM_MAT_t24_t_t20_ROWPTR[i + 1]; ++p) s += FOM_MAT_t24_t_t20_VALS[p] * t_t20[FOM_MAT_t24_t_t20_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t3_t_t1_ROWPTR[i]; p < FOM_MAT_t3_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t3_t_t1_VALS[p] * t_t21[FOM_MAT_t3_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t24_t_t23_ROWPTR[i]; p < FOM_MAT_t24_t_t23_ROWPTR[i + 1]; ++p) s += FOM_MAT_t24_t_t23_VALS[p] * t_t23[FOM_MAT_t24_t_t23_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t25(const fom_float *t_t20, const fom_float *t_t22, const fom_float *t_t23, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 30; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t25_t_t20_ROWPTR[i]; p < FOM_MAT_t25_t_t20_ROWPTR[i + 1]; ++p) s += FOM_MAT_t25_t_t20_VALS[p] * t_t20[FOM_MAT_t25_t_t20_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t7_t_t4_ROWPTR[i]; p < FOM_MAT_t7_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t7_t_t4_VALS[p] * t_t22[FOM_MAT_t7_t_t4_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t25_t_t23_ROWPTR[i]; p < FOM_MAT_t25_t_t23_ROWPTR[i + 1]; ++p) s += FOM_MAT_t25_t_t23_VALS[p] * t_t23[FOM_MAT_t25_t_t23_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t26(const fom_float *t_t23, const fom_float *t_t24, const fom_float *t_t25, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 600; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t2_ROWPTR[i]; p < FOM_MAT_t5_t_t2_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t2_VALS[p] * t_t23[FOM_MAT_t5_t_t2_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t2_t_t1_ROWPTR[i]; p < FOM_MAT_t2_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t2_t_t1_VALS[p] * t_t24[FOM_MAT_t2_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t4_ROWPTR[i]; p < FOM_MAT_t5_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t4_VALS[p] * t_t25[FOM_MAT_t5_t_t4_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t27(const fom_float *t_t23, const fom_float *t_t24, const fom_float *t_t26, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 20; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t27_t_t23_ROWPTR[i]; p < FOM_MAT_t27_t_t23_ROWPTR[i + 1]; ++p) s += FOM_MAT_t27_t_t23_VALS[p] * t_t23[FOM_MAT_t27_t_t23_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t3_t_t1_ROWPTR[i]; p < FOM_MAT_t3_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t3_t_t1_VALS[p] * t_t24[FOM_MAT_t3_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t27_t_t26_ROWPTR[i]; p < FOM_MAT_t27_t_t26_ROWPTR[i + 1]; ++p) s += FOM_MAT_t27_t_t26_VALS[p] * t_t26[FOM_MAT_t27_t_t26_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t28(const fom_float *t_t23, const fom_float *t_t25, const fom_float *t_t26, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 30; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t28_t_t23_ROWPTR[i]; p < FOM_MAT_t28_t_t23_ROWPTR[i + 1]; ++p) s += FOM_MAT_t28_t_t23_VALS[p] * t_t23[FOM_MAT_t28_t_t23_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t7_t_t4_ROWPTR[i]; p < FOM_MAT_t7_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t7_t_t4_VALS[p] * t_t25[FOM_MAT_t7_t_t4_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t28_t_t26_ROWPTR[i]; p < FOM_MAT_t28_t_t26_ROWPTR[i + 1]; ++p) s += FOM_MAT_t28_t_t26_VALS[p] * t_t26[FOM_MAT_t28_t_t26_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t29(const fom_float *t_t26, const fom_float *t_t27, const fom_float *t_t28, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 600; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t2_ROWPTR[i]; p < FOM_MAT_t5_t_t2_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t2_VALS[p] * t_t26[FOM_MAT_t5_t_t2_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t2_t_t1_ROWPTR[i]; p < FOM_MAT_t2_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t2_t_t1_VALS[p] * t_t27[FOM_MAT_t2_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t4_ROWPTR[i]; p < FOM_MAT_t5_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t4_VALS[p] * t_t28[FOM_MAT_t5_t_t4_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t30(const fom_float *t_t26, const fom_float *t_t27, const fom_float *t_t29, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 20; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t30_t_t26_ROWPTR[i]; p < FOM_MAT_t30_t_t26_ROWPTR[i + 1]; ++p) s += FOM_MAT_t30_t_t26_VALS[p] * t_t26[FOM_MAT_t30_t_t26_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t3_t_t1_ROWPTR[i]; p < FOM_MAT_t3_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t3_t_t1_VALS[p] * t_t27[FOM_MAT_t3_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t30_t_t29_ROWPTR[i]; p < FOM_MAT_t30_t_t29_ROWPTR[i + 1]; ++p) s += FOM_MAT_t30_t_t29_VALS[p] * t_t29[FOM_MAT_t30_t_t29_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t31(const fom_float *t_t26, const fom_float *t_t28, const fom_float *t_t29, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 30; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t31_t_t26_ROWPTR[i]; p < FOM_MAT_t31_t_t26_ROWPTR[i + 1]; ++p) s += FOM_MAT_t31_t_t26_VALS[p] * t_t26[FOM_MAT_t31_t_t26_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t7_t_t4_ROWPTR[i]; p < FOM_MAT_t7_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t7_t_t4_VALS[p] * t_t28[FOM_MAT_t7_t_t4_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t31_t_t29_ROWPTR[i]; p < FOM_MAT_t31_t_t29_ROWPTR[i + 1]; ++p) s += FOM_MAT_t31_t_t29_VALS[p] * t_t29[FOM_MAT_t31_t_t29_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t32(const fom_float *t_t29, const fom_float *t_t30, const fom_float *t_t31, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 600; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t2_ROWPTR[i]; p < FOM_MAT_t5_t_t2_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t2_VALS[p] * t_t29[FOM_MAT_t5_t_t2_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t2_t_t1_ROWPTR[i]; p < FOM_MAT_t2_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t2_t_t1_VALS[p] * t_t30[FOM_MAT_t2_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t4_ROWPTR[i]; p < FOM_MAT_t5_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t4_VALS[p] * t_t31[FOM_MAT_t5_t_t4_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t33(const fom_float *t_t29, const fom_float *t_t30, const fom_float *t_t32, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 20; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t33_t_t29_ROWPTR[i]; p < FOM_MAT_t33_t_t29_ROWPTR[i + 1]; ++p) s += FOM_MAT_t33_t_t29_VALS[p] * t_t29[FOM_MAT_t33_t_t29_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t3_t_t1_ROWPTR[i]; p < FOM_MAT_t3_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t3_t_t1_VALS[p] * t_t30[FOM_MAT_t3_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t33_t_t32_ROWPTR[i]; p < FOM_MAT_t33_t_t32_ROWPTR[i + 1]; ++p) s += FOM_MAT_t33_t_t32_VALS[p] * t_t32[FOM_MAT_t33_t_t32_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t34(const fom_float *t_t29, const fom_float *t_t31, const fom_float *t_t32, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 30; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t34_t_t29_ROWPTR[i]; p < FOM_MAT_t34_t_t29_ROWPTR[i + 1]; ++p) s += FOM_MAT_t34_t_t29_VALS[p] * t_t29[FOM_MAT_t34_t_t29_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t7_t_t4_ROWPTR[i]; p < FOM_MAT_t7_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t7_t_t4_VALS[p] * t_t31[FOM_MAT_t7_t_t4_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t34_t_t32_ROWPTR[i]; p < FOM_MAT_t34_t_t32_ROWPTR[i + 1]; ++p) s += FOM_MAT_t34_t_t32_VALS[p] * t_t32[FOM_MAT_t34_t_t32_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t35(const fom_float *t_t32, const fom_float *t_t33, const fom_float *t_t34, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 600; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t2_ROWPTR[i]; p < FOM_MAT_t5_t_t2_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t2_VALS[p] * t_t32[FOM_MAT_t5_t_t2_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t2_t_t1_ROWPTR[i]; p < FOM_MAT_t2_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t2_t_t1_VALS[p] * t_t33[FOM_MAT_t2_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t4_ROWPTR[i]; p < FOM_MAT_t5_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t4_VALS[p] * t_t34[FOM_MAT_t5_t_t4_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t36(const fom_float *t_t32, const fom_float *t_t33, const fom_float *t_t35, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 20; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t36_t_t32_ROWPTR[i]; p < FOM_MAT_t36_t_t32_ROWPTR[i + 1]; ++p) s += FOM_MAT_t36_t_t32_VALS[p] * t_t32[FOM_MAT_t36_t_t32_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t3_t_t1_ROWPTR[i]; p < FOM_MAT_t3_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t3_t_t1_VALS[p] * t_t33[FOM_MAT_t3_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t36_t_t35_ROWPTR[i]; p < FOM_MAT_t36_t_t35_ROWPTR[i + 1]; ++p) s += FOM_MAT_t36_t_t35_VALS[p] * t_t35[FOM_MAT_t36_t_t35_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t37(const fom_float *t_t32, const fom_float *t_t34, const fom_float *t_t35, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 30; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t37_t_t32_ROWPTR[i]; p < FOM_MAT_t37_t_t32_ROWPTR[i + 1]; ++p) s += FOM_MAT_t37_t_t32_VALS[p] * t_t32[FOM_MAT_t37_t_t32_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t7_t_t4_ROWPTR[i]; p < FOM_MAT_t7_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t7_t_t4_VALS[p] * t_t34[FOM_MAT_t7_t_t4_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t37_t_t35_ROWPTR[i]; p < FOM_MAT_t37_t_t35_ROWPTR[i + 1]; ++p) s += FOM_MAT_t37_t_t35_VALS[p] * t_t35[FOM_MAT_t37_t_t35_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t38(const fom_float *t_t35, const fom_float *t_t36, const fom_float *t_t37, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 600; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t2_ROWPTR[i]; p < FOM_MAT_t5_t_t2_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t2_VALS[p] * t_t35[FOM_MAT_t5_t_t2_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t2_t_t1_ROWPTR[i]; p < FOM_MAT_t2_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t2_t_t1_VALS[p] * t_t36[FOM_MAT_t2_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t4_ROWPTR[i]; p < FOM_MAT_t5_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t4_VALS[p] * t_t37[FOM_MAT_t5_t_t4_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t39(const fom_float *t_t35, const fom_float *t_t36, const fom_float *t_t38, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 20; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t39_t_t35_ROWPTR[i]; p < FOM_MAT_t39_t_t35_ROWPTR[i + 1]; ++p) s += FOM_MAT_t39_t_t35_VALS[p] * t_t35[FOM_MAT_t39_t_t35_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t3_t_t1_ROWPTR[i]; p < FOM_MAT_t3_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t3_t_t1_VALS[p] * t_t36[FOM_MAT_t3_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t39_t_t38_ROWPTR[i]; p < FOM_MAT_t39_t_t38_ROWPTR[i + 1]; ++p) s += FOM_MAT_t39_t_t38_VALS[p] * t_t38[FOM_MAT_t39_t_t38_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t40(const fom_float *t_t35, const fom_float *t_t37, const fom_float *t_t38, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 30; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t40_t_t35_ROWPTR[i]; p < FOM_MAT_t40_t_t35_ROWPTR[i + 1]; ++p) s += FOM_MAT_t40_t_t35_VALS[p] * t_t35[FOM_MAT_t40_t_t35_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t7_t_t4_ROWPTR[i]; p < FOM_MAT_t7_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t7_t_t4_VALS[p] * t_t37[FOM_MAT_t7_t_t4_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t40_t_t38_ROWPTR[i]; p < FOM_MAT_t40_t_t38_ROWPTR[i + 1]; ++p) s += FOM_MAT_t40_t_t38_VALS[p] * t_t38[FOM_MAT_t40_t_t38_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t41(const fom_float *t_t38, const fom_float *t_t39, const fom_float *t_t40, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 600; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t2_ROWPTR[i]; p < FOM_MAT_t5_t_t2_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t2_VALS[p] * t_t38[FOM_MAT_t5_t_t2_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t2_t_t1_ROWPTR[i]; p < FOM_MAT_t2_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t2_t_t1_VALS[p] * t_t39[FOM_MAT_t2_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t4_ROWPTR[i]; p < FOM_MAT_t5_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t4_VALS[p] * t_t40[FOM_MAT_t5_t_t4_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t42(const fom_float *t_t38, const fom_float *t_t39, const fom_float *t_t41, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 20; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t42_t_t38_ROWPTR[i]; p < FOM_MAT_t42_t_t38_ROWPTR[i + 1]; ++p) s += FOM_MAT_t42_t_t38_VALS[p] * t_t38[FOM_MAT_t42_t_t38_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t3_t_t1_ROWPTR[i]; p < FOM_MAT_t3_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t3_t_t1_VALS[p] * t_t39[FOM_MAT_t3_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t42_t_t41_ROWPTR[i]; p < FOM_MAT_t42_t_t41_ROWPTR[i + 1]; ++p) s += FOM_MAT_t42_t_t41_VALS[p] * t_t41[FOM_MAT_t42_t_t41_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t43(const fom_float *t_t38, const fom_float *t_t40, const fom_float *t_t41, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 30; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t43_t_t38_ROWPTR[i]; p < FOM_MAT_t43_t_t38_ROWPTR[i + 1]; ++p) s += FOM_MAT_t43_t_t38_VALS[p] * t_t38[FOM_MAT_t43_t_t38_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t7_t_t4_ROWPTR[i]; p < FOM_MAT_t7_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t7_t_t4_VALS[p] * t_t40[FOM_MAT_t7_t_t4_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t43_t_t41_ROWPTR[i]; p < FOM_MAT_t43_t_t41_ROWPTR[i + 1]; ++p) s += FOM_MAT_t43_t_t41_VALS[p] * t_t41[FOM_MAT_t43_t_t41_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t44(const fom_float *t_t41, const fom_float *t_t42, const fom_float *t_t43, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 600; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t2_ROWPTR[i]; p < FOM_MAT_t5_t_t2_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t2_VALS[p] * t_t41[FOM_MAT_t5_t_t2_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t2_t_t1_ROWPTR[i]; p < FOM_MAT_t2_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t2_t_t1_VALS[p] * t_t42[FOM_MAT_t2_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t4_ROWPTR[i]; p < FOM_MAT_t5_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t4_VALS[p] * t_t43[FOM_MAT_t5_t_t4_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t45(const fom_float *t_t41, const fom_float *t_t42, const fom_float *t_t44, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 20; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t45_t_t41_ROWPTR[i]; p < FOM_MAT_t45_t_t41_ROWPTR[i + 1]; ++p) s += FOM_MAT_t45_t_t41_VALS[p] * t_t41[FOM_MAT_t45_t_t41_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t3_t_t1_ROWPTR[i]; p < FOM_MAT_t3_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t3_t_t1_VALS[p] * t_t42[FOM_MAT_t3_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t45_t_t44_ROWPTR[i]; p < FOM_MAT_t45_t_t44_ROWPTR[i + 1]; ++p) s += FOM_MAT_t45_t_t44_VALS[p] * t_t44[FOM_MAT_t45_t_t44_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t46(const fom_float *t_t41, const fom_float *t_t43, const fom_float *t_t44, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 30; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t46_t_t41_ROWPTR[i]; p < FOM_MAT_t46_t_t41_ROWPTR[i + 1]; ++p) s += FOM_MAT_t46_t_t41_VALS[p] * t_t41[FOM_MAT_t46_t_t41_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t7_t_t4_ROWPTR[i]; p < FOM_MAT_t7_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t7_t_t4_VALS[p] * t_t43[FOM_MAT_t7_t_t4_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t46_t_t44_ROWPTR[i]; p < FOM_MAT_t46_t_t44_ROWPTR[i + 1]; ++p) s += FOM_MAT_t46_t_t44_VALS[p] * t_t44[FOM_MAT_t46_t_t44_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t47(const fom_float *t_t44, const fom_float *t_t45, const fom_float *t_t46, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 600; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t2_ROWPTR[i]; p < FOM_MAT_t5_t_t2_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t2_VALS[p] * t_t44[FOM_MAT_t5_t_t2_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t2_t_t1_ROWPTR[i]; p < FOM_MAT_t2_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t2_t_t1_VALS[p] * t_t45[FOM_MAT_t2_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t4_ROWPTR[i]; p < FOM_MAT_t5_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t4_VALS[p] * t_t46[FOM_MAT_t5_t_t4_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t48(const fom_float *t_t44, const fom_float *t_t45, const fom_float *t_t47, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 20; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t48_t_t44_ROWPTR[i]; p < FOM_MAT_t48_t_t44_ROWPTR[i + 1]; ++p) s += FOM_MAT_t48_t_t44_VALS[p] * t_t44[FOM_MAT_t48_t_t44_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t3_t_t1_ROWPTR[i]; p < FOM_MAT_t3_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t3_t_t1_VALS[p] * t_t45[FOM_MAT_t3_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t48_t_t47_ROWPTR[i]; p < FOM_MAT_t48_t_t47_ROWPTR[i + 1]; ++p) s += FOM_MAT_t48_t_t47_VALS[p] * t_t47[FOM_MAT_t48_t_t47_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t49(const fom_float *t_t44, const fom_float *t_t46, const fom_float *t_t47, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 30; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t49_t_t44_ROWPTR[i]; p < FOM_MAT_t49_t_t44_ROWPTR[i + 1]; ++p) s += FOM_MAT_t49_t_t44_VALS[p] * t_t44[FOM_MAT_t49_t_t44_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t7_t_t4_ROWPTR[i]; p < FOM_MAT_t7_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t7_t_t4_VALS[p] * t_t46[FOM_MAT_t7_t_t4_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t49_t_t47_ROWPTR[i]; p < FOM_MAT_t49_t_t47_ROWPTR[i + 1]; ++p) s += FOM_MAT_t49_t_t47_VALS[p] * t_t47[FOM_MAT_t49_t_t47_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t50(const fom_float *t_t47, const fom_float *t_t48, const fom_float *t_t49, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 600; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t2_ROWPTR[i]; p < FOM_MAT_t5_t_t2_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t2_VALS[p] * t_t47[FOM_MAT_t5_t_t2_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t2_t_t1_ROWPTR[i]; p < FOM_MAT_t2_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t2_t_t1_VALS[p] * t_t48[FOM_MAT_t2_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t4_ROWPTR[i]; p < FOM_MAT_t5_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t4_VALS[p] * t_t49[FOM_MAT_t5_t_t4_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t51(const fom_float *t_t47, const fom_float *t_t48, const fom_float *t_t50, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 20; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t51_t_t47_ROWPTR[i]; p < FOM_MAT_t51_t_t47_ROWPTR[i + 1]; ++p) s += FOM_MAT_t51_t_t47_VALS[p] * t_t47[FOM_MAT_t51_t_t47_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t3_t_t1_ROWPTR[i]; p < FOM_MAT_t3_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t3_t_t1_VALS[p] * t_t48[FOM_MAT_t3_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t51_t_t50_ROWPTR[i]; p < FOM_MAT_t51_t_t50_ROWPTR[i + 1]; ++p) s += FOM_MAT_t51_t_t50_VALS[p] * t_t50[FOM_MAT_t51_t_t50_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t52(const fom_float *t_t47, const fom_float *t_t49, const fom_float *t_t50, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 30; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t52_t_t47_ROWPTR[i]; p < FOM_MAT_t52_t_t47_ROWPTR[i + 1]; ++p) s += FOM_MAT_t52_t_t47_VALS[p] * t_t47[FOM_MAT_t52_t_t47_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t7_t_t4_ROWPTR[i]; p < FOM_MAT_t7_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t7_t_t4_VALS[p] * t_t49[FOM_MAT_t7_t_t4_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t52_t_t50_ROWPTR[i]; p < FOM_MAT_t52_t_t50_ROWPTR[i + 1]; ++p) s += FOM_MAT_t52_t_t50_VALS[p] * t_t50[FOM_MAT_t52_t_t50_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t53(const fom_float *t_t50, const fom_float *t_t51, const fom_float *t_t52, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 600; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t2_ROWPTR[i]; p < FOM_MAT_t5_t_t2_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t2_VALS[p] * t_t50[FOM_MAT_t5_t_t2_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t2_t_t1_ROWPTR[i]; p < FOM_MAT_t2_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t2_t_t1_VALS[p] * t_t51[FOM_MAT_t2_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t4_ROWPTR[i]; p < FOM_MAT_t5_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t4_VALS[p] * t_t52[FOM_MAT_t5_t_t4_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t54(const fom_float *t_t50, const fom_float *t_t51, const fom_float *t_t53, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 20; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t54_t_t50_ROWPTR[i]; p < FOM_MAT_t54_t_t50_ROWPTR[i + 1]; ++p) s += FOM_MAT_t54_t_t50_VALS[p] * t_t50[FOM_MAT_t54_t_t50_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t3_t_t1_ROWPTR[i]; p < FOM_MAT_t3_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t3_t_t1_VALS[p] * t_t51[FOM_MAT_t3_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t54_t_t53_ROWPTR[i]; p < FOM_MAT_t54_t_t53_ROWPTR[i + 1]; ++p) s += FOM_MAT_t54_t_t53_VALS[p] * t_t53[FOM_MAT_t54_t_t53_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t55(const fom_float *t_t50, const fom_float *t_t52, const fom_float *t_t53, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 30; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t55_t_t50_ROWPTR[i]; p < FOM_MAT_t55_t_t50_ROWPTR[i + 1]; ++p) s += FOM_MAT_t55_t_t50_VALS[p] * t_t50[FOM_MAT_t55_t_t50_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t7_t_t4_ROWPTR[i]; p < FOM_MAT_t7_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t7_t_t4_VALS[p] * t_t52[FOM_MAT_t7_t_t4_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t55_t_t53_ROWPTR[i]; p < FOM_MAT_t55_t_t53_ROWPTR[i + 1]; ++p) s += FOM_MAT_t55_t_t53_VALS[p] * t_t53[FOM_MAT_t55_t_t53_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t56(const fom_float *t_t53, const fom_float *t_t54, const fom_float *t_t55, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 600; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t2_ROWPTR[i]; p < FOM_MAT_t5_t_t2_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t2_VALS[p] * t_t53[FOM_MAT_t5_t_t2_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t2_t_t1_ROWPTR[i]; p < FOM_MAT_t2_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t2_t_t1_VALS[p] * t_t54[FOM_MAT_t2_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t4_ROWPTR[i]; p < FOM_MAT_t5_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t4_VALS[p] * t_t55[FOM_MAT_t5_t_t4_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t57(const fom_float *t_t53, const fom_float *t_t54, const fom_float *t_t56, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 20; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t57_t_t53_ROWPTR[i]; p < FOM_MAT_t57_t_t53_ROWPTR[i + 1]; ++p) s += FOM_MAT_t57_t_t53_VALS[p] * t_t53[FOM_MAT_t57_t_t53_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t3_t_t1_ROWPTR[i]; p < FOM_MAT_t3_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t3_t_t1_VALS[p] * t_t54[FOM_MAT_t3_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t57_t_t56_ROWPTR[i]; p < FOM_MAT_t57_t_t56_ROWPTR[i + 1]; ++p) s += FOM_MAT_t57_t_t56_VALS[p] * t_t56[FOM_MAT_t57_t_t56_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t58(const fom_float *t_t53, const fom_float *t_t55, const fom_float *t_t56, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 30; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t58_t_t53_ROWPTR[i]; p < FOM_MAT_t58_t_t53_ROWPTR[i + 1]; ++p) s += FOM_MAT_t58_t_t53_VALS[p] * t_t53[FOM_MAT_t58_t_t53_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t7_t_t4_ROWPTR[i]; p < FOM_MAT_t7_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t7_t_t4_VALS[p] * t_t55[FOM_MAT_t7_t_t4_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t58_t_t56_ROWPTR[i]; p < FOM_MAT_t58_t_t56_ROWPTR[i + 1]; ++p) s += FOM_MAT_t58_t_t56_VALS[p] * t_t56[FOM_MAT_t58_t_t56_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t59(const fom_float *t_t56, const fom_float *t_t57, const fom_float *t_t58, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 600; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t2_ROWPTR[i]; p < FOM_MAT_t5_t_t2_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t2_VALS[p] * t_t56[FOM_MAT_t5_t_t2_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t2_t_t1_ROWPTR[i]; p < FOM_MAT_t2_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t2_t_t1_VALS[p] * t_t57[FOM_MAT_t2_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t4_ROWPTR[i]; p < FOM_MAT_t5_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t4_VALS[p] * t_t58[FOM_MAT_t5_t_t4_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t60(const fom_float *t_t56, const fom_float *t_t57, const fom_float *t_t59, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 20; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t60_t_t56_ROWPTR[i]; p < FOM_MAT_t60_t_t56_ROWPTR[i + 1]; ++p) s += FOM_MAT_t60_t_t56_VALS[p] * t_t56[FOM_MAT_t60_t_t56_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t3_t_t1_ROWPTR[i]; p < FOM_MAT_t3_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t3_t_t1_VALS[p] * t_t57[FOM_MAT_t3_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t60_t_t59_ROWPTR[i]; p < FOM_MAT_t60_t_t59_ROWPTR[i + 1]; ++p) s += FOM_MAT_t60_t_t59_VALS[p] * t_t59[FOM_MAT_t60_t_t59_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t61(const fom_float *t_t56, const fom_float *t_t58, const fom_float *t_t59, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 30; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t61_t_t56_ROWPTR[i]; p < FOM_MAT_t61_t_t56_ROWPTR[i + 1]; ++p) s += FOM_MAT_t61_t_t56_VALS[p] * t_t56[FOM_MAT_t61_t_t56_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t7_t_t4_ROWPTR[i]; p < FOM_MAT_t7_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t7_t_t4_VALS[p] * t_t58[FOM_MAT_t7_t_t4_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t61_t_t59_ROWPTR[i]; p < FOM_MAT_t61_t_t59_ROWPTR[i + 1]; ++p) s += FOM_MAT_t61_t_t59_VALS[p] * t_t59[FOM_MAT_t61_t_t59_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t62(const fom_float *t_t59, const fom_float *t_t60, const fom_float *t_t61, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 600; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t2_ROWPTR[i]; p < FOM_MAT_t5_t_t2_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t2_VALS[p] * t_t59[FOM_MAT_t5_t_t2_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t2_t_t1_ROWPTR[i]; p < FOM_MAT_t2_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t2_t_t1_VALS[p] * t_t60[FOM_MAT_t2_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t4_ROWPTR[i]; p < FOM_MAT_t5_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t4_VALS[p] * t_t61[FOM_MAT_t5_t_t4_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t63(const fom_float *t_t59, const fom_float *t_t60, const fom_float *t_t62, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 20; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t63_t_t59_ROWPTR[i]; p < FOM_MAT_t63_t_t59_ROWPTR[i + 1]; ++p) s += FOM_MAT_t63_t_t59_VALS[p] * t_t59[FOM_MAT_t63_t_t59_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t3_t_t1_ROWPTR[i]; p < FOM_MAT_t3_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t3_t_t1_VALS[p] * t_t60[FOM_MAT_t3_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t63_t_t62_ROWPTR[i]; p < FOM_MAT_t63_t_t62_ROWPTR[i + 1]; ++p) s += FOM_MAT_t63_t_t62_VALS[p] * t_t62[FOM_MAT_t63_t_t62_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t64(const fom_float *t_t59, const fom_float *t_t61, const fom_float *t_t62, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 30; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t64_t_t59_ROWPTR[i]; p < FOM_MAT_t64_t_t59_ROWPTR[i + 1]; ++p) s += FOM_MAT_t64_t_t59_VALS[p] * t_t59[FOM_MAT_t64_t_t59_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t7_t_t4_ROWPTR[i]; p < FOM_MAT_t7_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t7_t_t4_VALS[p] * t_t61[FOM_MAT_t7_t_t4_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t64_t_t62_ROWPTR[i]; p < FOM_MAT_t64_t_t62_ROWPTR[i + 1]; ++p) s += FOM_MAT_t64_t_t62_VALS[p] * t_t62[FOM_MAT_t64_t_t62_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t65(const fom_float *t_t62, const fom_float *t_t63, const fom_float *t_t64, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 600; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t2_ROWPTR[i]; p < FOM_MAT_t5_t_t2_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t2_VALS[p] * t_t62[FOM_MAT_t5_t_t2_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t2_t_t1_ROWPTR[i]; p < FOM_MAT_t2_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t2_t_t1_VALS[p] * t_t63[FOM_MAT_t2_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t4_ROWPTR[i]; p < FOM_MAT_t5_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t4_VALS[p] * t_t64[FOM_MAT_t5_t_t4_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t66(const fom_float *t_t62, const fom_float *t_t63, const fom_float *t_t65, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 20; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t66_t_t62_ROWPTR[i]; p < FOM_MAT_t66_t_t62_ROWPTR[i + 1]; ++p) s += FOM_MAT_t66_t_t62_VALS[p] * t_t62[FOM_MAT_t66_t_t62_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t3_t_t1_ROWPTR[i]; p < FOM_MAT_t3_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t3_t_t1_VALS[p] * t_t63[FOM_MAT_t3_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t66_t_t65_ROWPTR[i]; p < FOM_MAT_t66_t_t65_ROWPTR[i + 1]; ++p) s += FOM_MAT_t66_t_t65_VALS[p] * t_t65[FOM_MAT_t66_t_t65_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t67(const fom_float *t_t62, const fom_float *t_t64, const fom_float *t_t65, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 30; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t67_t_t62_ROWPTR[i]; p < FOM_MAT_t67_t_t62_ROWPTR[i + 1]; ++p) s += FOM_MAT_t67_t_t62_VALS[p] * t_t62[FOM_MAT_t67_t_t62_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t7_t_t4_ROWPTR[i]; p < FOM_MAT_t7_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t7_t_t4_VALS[p] * t_t64[FOM_MAT_t7_t_t4_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t67_t_t65_ROWPTR[i]; p < FOM_MAT_t67_t_t65_ROWPTR[i + 1]; ++p) s += FOM_MAT_t67_t_t65_VALS[p] * t_t65[FOM_MAT_t67_t_t65_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t68(const fom_float *t_t65, const fom_float *t_t66, const fom_float *t_t67, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 600; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t2_ROWPTR[i]; p < FOM_MAT_t5_t_t2_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t2_VALS[p] * t_t65[FOM_MAT_t5_t_t2_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t2_t_t1_ROWPTR[i]; p < FOM_MAT_t2_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t2_t_t1_VALS[p] * t_t66[FOM_MAT_t2_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t4_ROWPTR[i]; p < FOM_MAT_t5_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t4_VALS[p] * t_t67[FOM_MAT_t5_t_t4_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t69(const fom_float *t_t65, const fom_float *t_t66, const fom_float *t_t68, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 20; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t69_t_t65_ROWPTR[i]; p < FOM_MAT_t69_t_t65_ROWPTR[i + 1]; ++p) s += FOM_MAT_t69_t_t65_VALS[p] * t_t65[FOM_MAT_t69_t_t65_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t3_t_t1_ROWPTR[i]; p < FOM_MAT_t3_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t3_t_t1_VALS[p] * t_t66[FOM_MAT_t3_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t69_t_t68_ROWPTR[i]; p < FOM_MAT_t69_t_t68_ROWPTR[i + 1]; ++p) s += FOM_MAT_t69_t_t68_VALS[p] * t_t68[FOM_MAT_t69_t_t68_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t70(const fom_float *t_t65, const fom_float *t_t67, const fom_float *t_t68, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 30; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t70_t_t65_ROWPTR[i]; p < FOM_MAT_t70_t_t65_ROWPTR[i + 1]; ++p) s += FOM_MAT_t70_t_t65_VALS[p] * t_t65[FOM_MAT_t70_t_t65_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t7_t_t4_ROWPTR[i]; p < FOM_MAT_t7_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t7_t_t4_VALS[p] * t_t67[FOM_MAT_t7_t_t4_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t70_t_t68_ROWPTR[i]; p < FOM_MAT_t70_t_t68_ROWPTR[i + 1]; ++p) s += FOM_MAT_t70_t_t68_VALS[p] * t_t68[FOM_MAT_t70_t_t68_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t71(const fom_float *t_t68, const fom_float *t_t69, const fom_float *t_t70, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 600; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t2_ROWPTR[i]; p < FOM_MAT_t5_t_t2_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t2_VALS[p] * t_t68[FOM_MAT_t5_t_t2_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t2_t_t1_ROWPTR[i]; p < FOM_MAT_t2_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t2_t_t1_VALS[p] * t_t69[FOM_MAT_t2_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t4_ROWPTR[i]; p < FOM_MAT_t5_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t4_VALS[p] * t_t70[FOM_MAT_t5_t_t4_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t72(const fom_float *t_t68, const fom_float *t_t69, const fom_float *t_t71, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 20; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t72_t_t68_ROWPTR[i]; p < FOM_MAT_t72_t_t68_ROWPTR[i + 1]; ++p) s += FOM_MAT_t72_t_t68_VALS[p] * t_t68[FOM_MAT_t72_t_t68_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t3_t_t1_ROWPTR[i]; p < FOM_MAT_t3_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t3_t_t1_VALS[p] * t_t69[FOM_MAT_t3_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t72_t_t71_ROWPTR[i]; p < FOM_MAT_t72_t_t71_ROWPTR[i + 1]; ++p) s += FOM_MAT_t72_t_t71_VALS[p] * t_t71[FOM_MAT_t72_t_t71_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t73(const fom_float *t_t68, const fom_float *t_t70, const fom_float *t_t71, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 30; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t73_t_t68_ROWPTR[i]; p < FOM_MAT_t73_t_t68_ROWPTR[i + 1]; ++p) s += FOM_MAT_t73_t_t68_VALS[p] * t_t68[FOM_MAT_t73_t_t68_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t7_t_t4_ROWPTR[i]; p < FOM_MAT_t7_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t7_t_t4_VALS[p] * t_t70[FOM_MAT_t7_t_t4_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t73_t_t71_ROWPTR[i]; p < FOM_MAT_t73_t_t71_ROWPTR[i + 1]; ++p) s += FOM_MAT_t73_t_t71_VALS[p] * t_t71[FOM_MAT_t73_t_t71_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t74(const fom_float *t_t71, const fom_float *t_t72, const fom_float *t_t73, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 600; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t2_ROWPTR[i]; p < FOM_MAT_t5_t_t2_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t2_VALS[p] * t_t71[FOM_MAT_t5_t_t2_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t2_t_t1_ROWPTR[i]; p < FOM_MAT_t2_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t2_t_t1_VALS[p] * t_t72[FOM_MAT_t2_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t4_ROWPTR[i]; p < FOM_MAT_t5_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t4_VALS[p] * t_t73[FOM_MAT_t5_t_t4_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t75(const fom_float *t_t71, const fom_float *t_t72, const fom_float *t_t74, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 20; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t75_t_t71_ROWPTR[i]; p < FOM_MAT_t75_t_t71_ROWPTR[i + 1]; ++p) s += FOM_MAT_t75_t_t71_VALS[p] * t_t71[FOM_MAT_t75_t_t71_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t3_t_t1_ROWPTR[i]; p < FOM_MAT_t3_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t3_t_t1_VALS[p] * t_t72[FOM_MAT_t3_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t75_t_t74_ROWPTR[i]; p < FOM_MAT_t75_t_t74_ROWPTR[i + 1]; ++p) s += FOM_MAT_t75_t_t74_VALS[p] * t_t74[FOM_MAT_t75_t_t74_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t76(const fom_float *t_t71, const fom_float *t_t73, const fom_float *t_t74, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 30; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t76_t_t71_ROWPTR[i]; p < FOM_MAT_t76_t_t71_ROWPTR[i + 1]; ++p) s += FOM_MAT_t76_t_t71_VALS[p] * t_t71[FOM_MAT_t76_t_t71_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t7_t_t4_ROWPTR[i]; p < FOM_MAT_t7_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t7_t_t4_VALS[p] * t_t73[FOM_MAT_t7_t_t4_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t76_t_t74_ROWPTR[i]; p < FOM_MAT_t76_t_t74_ROWPTR[i + 1]; ++p) s += FOM_MAT_t76_t_t74_VALS[p] * t_t74[FOM_MAT_t76_t_t74_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t77(const fom_float *t_t74, const fom_float *t_t75, const fom_float *t_t76, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 600; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t2_ROWPTR[i]; p < FOM_MAT_t5_t_t2_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t2_VALS[p] * t_t74[FOM_MAT_t5_t_t2_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t2_t_t1_ROWPTR[i]; p < FOM_MAT_t2_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t2_t_t1_VALS[p] * t_t75[FOM_MAT_t2_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t4_ROWPTR[i]; p < FOM_MAT_t5_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t4_VALS[p] * t_t76[FOM_MAT_t5_t_t4_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t78(const fom_float *t_t74, const fom_float *t_t75, const fom_float *t_t77, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 20; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t78_t_t74_ROWPTR[i]; p < FOM_MAT_t78_t_t74_ROWPTR[i + 1]; ++p) s += FOM_MAT_t78_t_t74_VALS[p] * t_t74[FOM_MAT_t78_t_t74_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t3_t_t1_ROWPTR[i]; p < FOM_MAT_t3_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t3_t_t1_VALS[p] * t_t75[FOM_MAT_t3_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t78_t_t77_ROWPTR[i]; p < FOM_MAT_t78_t_t77_ROWPTR[i + 1]; ++p) s += FOM_MAT_t78_t_t77_VALS[p] * t_t77[FOM_MAT_t78_t_t77_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t79(const fom_float *t_t74, const fom_float *t_t76, const fom_float *t_t77, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 30; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t79_t_t74_ROWPTR[i]; p < FOM_MAT_t79_t_t74_ROWPTR[i + 1]; ++p) s += FOM_MAT_t79_t_t74_VALS[p] * t_t74[FOM_MAT_t79_t_t74_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t7_t_t4_ROWPTR[i]; p < FOM_MAT_t7_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t7_t_t4_VALS[p] * t_t76[FOM_MAT_t7_t_t4_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t79_t_t77_ROWPTR[i]; p < FOM_MAT_t79_t_t77_ROWPTR[i + 1]; ++p) s += FOM_MAT_t79_t_t77_VALS[p] * t_t77[FOM_MAT_t79_t_t77_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t80(const fom_float *t_t77, const fom_float *t_t78, const fom_float *t_t79, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 600; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t2_ROWPTR[i]; p < FOM_MAT_t5_t_t2_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t2_VALS[p] * t_t77[FOM_MAT_t5_t_t2_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t2_t_t1_ROWPTR[i]; p < FOM_MAT_t2_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t2_t_t1_VALS[p] * t_t78[FOM_MAT_t2_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t4_ROWPTR[i]; p < FOM_MAT_t5_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t4_VALS[p] * t_t79[FOM_MAT_t5_t_t4_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t81(const fom_float *t_t77, const fom_float *t_t78, const fom_float *t_t80, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 20; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t81_t_t77_ROWPTR[i]; p < FOM_MAT_t81_t_t77_ROWPTR[i + 1]; ++p) s += FOM_MAT_t81_t_t77_VALS[p] * t_t77[FOM_MAT_t81_t_t77_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t3_t_t1_ROWPTR[i]; p < FOM_MAT_t3_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t3_t_t1_VALS[p] * t_t78[FOM_MAT_t3_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t81_t_t80_ROWPTR[i]; p < FOM_MAT_t81_t_t80_ROWPTR[i + 1]; ++p) s += FOM_MAT_t81_t_t80_VALS[p] * t_t80[FOM_MAT_t81_t_t80_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t82(const fom_float *t_t77, const fom_float *t_t79, const fom_float *t_t80, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 30; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t82_t_t77_ROWPTR[i]; p < FOM_MAT_t82_t_t77_ROWPTR[i + 1]; ++p) s += FOM_MAT_t82_t_t77_VALS[p] * t_t77[FOM_MAT_t82_t_t77_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t7_t_t4_ROWPTR[i]; p < FOM_MAT_t7_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t7_t_t4_VALS[p] * t_t79[FOM_MAT_t7_t_t4_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t82_t_t80_ROWPTR[i]; p < FOM_MAT_t82_t_t80_ROWPTR[i + 1]; ++p) s += FOM_MAT_t82_t_t80_VALS[p] * t_t80[FOM_MAT_t82_t_t80_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t83(const fom_float *t_t80, const fom_float *t_t81, const fom_float *t_t82, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 600; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t2_ROWPTR[i]; p < FOM_MAT_t5_t_t2_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t2_VALS[p] * t_t80[FOM_MAT_t5_t_t2_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t2_t_t1_ROWPTR[i]; p < FOM_MAT_t2_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t2_t_t1_VALS[p] * t_t81[FOM_MAT_t2_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t4_ROWPTR[i]; p < FOM_MAT_t5_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t4_VALS[p] * t_t82[FOM_MAT_t5_t_t4_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t84(const fom_float *t_t80, const fom_float *t_t81, const fom_float *t_t83, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 20; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t84_t_t80_ROWPTR[i]; p < FOM_MAT_t84_t_t80_ROWPTR[i + 1]; ++p) s += FOM_MAT_t84_t_t80_VALS[p] * t_t80[FOM_MAT_t84_t_t80_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t3_t_t1_ROWPTR[i]; p < FOM_MAT_t3_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t3_t_t1_VALS[p] * t_t81[FOM_MAT_t3_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t84_t_t83_ROWPTR[i]; p < FOM_MAT_t84_t_t83_ROWPTR[i + 1]; ++p) s += FOM_MAT_t84_t_t83_VALS[p] * t_t83[FOM_MAT_t84_t_t83_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t85(const fom_float *t_t80, const fom_float *t_t82, const fom_float *t_t83, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 30; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t85_t_t80_ROWPTR[i]; p < FOM_MAT_t85_t_t80_ROWPTR[i + 1]; ++p) s += FOM_MAT_t85_t_t80_VALS[p] * t_t80[FOM_MAT_t85_t_t80_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t7_t_t4_ROWPTR[i]; p < FOM_MAT_t7_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t7_t_t4_VALS[p] * t_t82[FOM_MAT_t7_t_t4_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t85_t_t83_ROWPTR[i]; p < FOM_MAT_t85_t_t83_ROWPTR[i + 1]; ++p) s += FOM_MAT_t85_t_t83_VALS[p] * t_t83[FOM_MAT_t85_t_t83_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t86(const fom_float *t_t83, const fom_float *t_t84, const fom_float *t_t85, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 600; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t2_ROWPTR[i]; p < FOM_MAT_t5_t_t2_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t2_VALS[p] * t_t83[FOM_MAT_t5_t_t2_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t2_t_t1_ROWPTR[i]; p < FOM_MAT_t2_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t2_t_t1_VALS[p] * t_t84[FOM_MAT_t2_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t4_ROWPTR[i]; p < FOM_MAT_t5_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t4_VALS[p] * t_t85[FOM_MAT_t5_t_t4_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t87(const fom_float *t_t83, const fom_float *t_t84, const fom_float *t_t86, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 20; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t87_t_t83_ROWPTR[i]; p < FOM_MAT_t87_t_t83_ROWPTR[i + 1]; ++p) s += FOM_MAT_t87_t_t83_VALS[p] * t_t83[FOM_MAT_t87_t_t83_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t3_t_t1_ROWPTR[i]; p < FOM_MAT_t3_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t3_t_t1_VALS[p] * t_t84[FOM_MAT_t3_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t87_t_t86_ROWPTR[i]; p < FOM_MAT_t87_t_t86_ROWPTR[i + 1]; ++p) s += FOM_MAT_t87_t_t86_VALS[p] * t_t86[FOM_MAT_t87_t_t86_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t88(const fom_float *t_t83, const fom_float *t_t85, const fom_float *t_t86, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 30; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t88_t_t83_ROWPTR[i]; p < FOM_MAT_t88_t_t83_ROWPTR[i + 1]; ++p) s += FOM_MAT_t88_t_t83_VALS[p] * t_t83[FOM_MAT_t88_t_t83_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t7_t_t4_ROWPTR[i]; p < FOM_MAT_t7_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t7_t_t4_VALS[p] * t_t85[FOM_MAT_t7_t_t4_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t88_t_t86_ROWPTR[i]; p < FOM_MAT_t88_t_t86_ROWPTR[i + 1]; ++p) s += FOM_MAT_t88_t_t86_VALS[p] * t_t86[FOM_MAT_t88_t_t86_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t89(const fom_float *t_t86, const fom_float *t_t87, const fom_float *t_t88, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 600; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t2_ROWPTR[i]; p < FOM_MAT_t5_t_t2_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t2_VALS[p] * t_t86[FOM_MAT_t5_t_t2_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t2_t_t1_ROWPTR[i]; p < FOM_MAT_t2_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t2_t_t1_VALS[p] * t_t87[FOM_MAT_t2_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t4_ROWPTR[i]; p < FOM_MAT_t5_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t4_VALS[p] * t_t88[FOM_MAT_t5_t_t4_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t90(const fom_float *t_t86, const fom_float *t_t87, const fom_float *t_t89, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 20; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t90_t_t86_ROWPTR[i]; p < FOM_MAT_t90_t_t86_ROWPTR[i + 1]; ++p) s += FOM_MAT_t90_t_t86_VALS[p] * t_t86[FOM_MAT_t90_t_t86_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t3_t_t1_ROWPTR[i]; p < FOM_MAT_t3_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t3_t_t1_VALS[p] * t_t87[FOM_MAT_t3_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t90_t_t89_ROWPTR[i]; p < FOM_MAT_t90_t_t89_ROWPTR[i + 1]; ++p) s += FOM_MAT_t90_t_t89_VALS[p] * t_t89[FOM_MAT_t90_t_t89_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t91(const fom_float *t_t86, const fom_float *t_t88, const fom_float *t_t89, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 30; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t91_t_t86_ROWPTR[i]; p < FOM_MAT_t91_t_t86_ROWPTR[i + 1]; ++p) s += FOM_MAT_t91_t_t86_VALS[p] * t_t86[FOM_MAT_t91_t_t86_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t7_t_t4_ROWPTR[i]; p < FOM_MAT_t7_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t7_t_t4_VALS[p] * t_t88[FOM_MAT_t7_t_t4_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t91_t_t89_ROWPTR[i]; p < FOM_MAT_t91_t_t89_ROWPTR[i + 1]; ++p) s += FOM_MAT_t91_t_t89_VALS[p] * t_t89[FOM_MAT_t91_t_t89_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t92(const fom_float *t_t89, const fom_float *t_t90, const fom_float *t_t91, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 600; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t2_ROWPTR[i]; p < FOM_MAT_t5_t_t2_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t2_VALS[p] * t_t89[FOM_MAT_t5_t_t2_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t2_t_t1_ROWPTR[i]; p < FOM_MAT_t2_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t2_t_t1_VALS[p] * t_t90[FOM_MAT_t2_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t4_ROWPTR[i]; p < FOM_MAT_t5_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t4_VALS[p] * t_t91[FOM_MAT_t5_t_t4_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t93(const fom_float *t_t89, const fom_float *t_t90, const fom_float *t_t92, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 20; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t93_t_t89_ROWPTR[i]; p < FOM_MAT_t93_t_t89_ROWPTR[i + 1]; ++p) s += FOM_MAT_t93_t_t89_VALS[p] * t_t89[FOM_MAT_t93_t_t89_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t3_t_t1_ROWPTR[i]; p < FOM_MAT_t3_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t3_t_t1_VALS[p] * t_t90[FOM_MAT_t3_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t93_t_t92_ROWPTR[i]; p < FOM_MAT_t93_t_t92_ROWPTR[i + 1]; ++p) s += FOM_MAT_t93_t_t92_VALS[p] * t_t92[FOM_MAT_t93_t_t92_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t94(const fom_float *t_t89, const fom_float *t_t91, const fom_float *t_t92, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 30; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t94_t_t89_ROWPTR[i]; p < FOM_MAT_t94_t_t89_ROWPTR[i + 1]; ++p) s += FOM_MAT_t94_t_t89_VALS[p] * t_t89[FOM_MAT_t94_t_t89_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t7_t_t4_ROWPTR[i]; p < FOM_MAT_t7_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t7_t_t4_VALS[p] * t_t91[FOM_MAT_t7_t_t4_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t94_t_t92_ROWPTR[i]; p < FOM_MAT_t94_t_t92_ROWPTR[i + 1]; ++p) s += FOM_MAT_t94_t_t92_VALS[p] * t_t92[FOM_MAT_t94_t_t92_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t95(const fom_float *t_t92, const fom_float *t_t93, const fom_float *t_t94, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 600; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t2_ROWPTR[i]; p < FOM_MAT_t5_t_t2_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t2_VALS[p] * t_t92[FOM_MAT_t5_t_t2_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t2_t_t1_ROWPTR[i]; p < FOM_MAT_t2_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t2_t_t1_VALS[p] * t_t93[FOM_MAT_t2_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t4_ROWPTR[i]; p < FOM_MAT_t5_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t4_VALS[p] * t_t94[FOM_MAT_t5_t_t4_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t96(const fom_float *t_t92, const fom_float *t_t93, const fom_float *t_t95, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 20; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t96_t_t92_ROWPTR[i]; p < FOM_MAT_t96_t_t92_ROWPTR[i + 1]; ++p) s += FOM_MAT_t96_t_t92_VALS[p] * t_t92[FOM_MAT_t96_t_t92_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t3_t_t1_ROWPTR[i]; p < FOM_MAT_t3_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t3_t_t1_VALS[p] * t_t93[FOM_MAT_t3_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t96_t_t95_ROWPTR[i]; p < FOM_MAT_t96_t_t95_ROWPTR[i + 1]; ++p) s += FOM_MAT_t96_t_t95_VALS[p] * t_t95[FOM_MAT_t96_t_t95_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t97(const fom_float *t_t92, const fom_float *t_t94, const fom_float *t_t95, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 30; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t97_t_t92_ROWPTR[i]; p < FOM_MAT_t97_t_t92_ROWPTR[i + 1]; ++p) s += FOM_MAT_t97_t_t92_VALS[p] * t_t92[FOM_MAT_t97_t_t92_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t7_t_t4_ROWPTR[i]; p < FOM_MAT_t7_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t7_t_t4_VALS[p] * t_t94[FOM_MAT_t7_t_t4_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t97_t_t95_ROWPTR[i]; p < FOM_MAT_t97_t_t95_ROWPTR[i + 1]; ++p) s += FOM_MAT_t97_t_t95_VALS[p] * t_t95[FOM_MAT_t97_t_t95_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t98(const fom_float *t_t95, const fom_float *t_t96, const fom_float *t_t97, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 600; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t2_ROWPTR[i]; p < FOM_MAT_t5_t_t2_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t2_VALS[p] * t_t95[FOM_MAT_t5_t_t2_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t2_t_t1_ROWPTR[i]; p < FOM_MAT_t2_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t2_t_t1_VALS[p] * t_t96[FOM_MAT_t2_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t4_ROWPTR[i]; p < FOM_MAT_t5_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t4_VALS[p] * t_t97[FOM_MAT_t5_t_t4_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t99(const fom_float *t_t95, const fom_float *t_t96, const fom_float *t_t98, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 20; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t99_t_t95_ROWPTR[i]; p < FOM_MAT_t99_t_t95_ROWPTR[i + 1]; ++p) s += FOM_MAT_t99_t_t95_VALS[p] * t_t95[FOM_MAT_t99_t_t95_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t3_t_t1_ROWPTR[i]; p < FOM_MAT_t3_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t3_t_t1_VALS[p] * t_t96[FOM_MAT_t3_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t99_t_t98_ROWPTR[i]; p < FOM_MAT_t99_t_t98_ROWPTR[i + 1]; ++p) s += FOM_MAT_t99_t_t98_VALS[p] * t_t98[FOM_MAT_t99_t_t98_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t100(const fom_float *t_t95, const fom_float *t_t97, const fom_float *t_t98, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 30; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t100_t_t95_ROWPTR[i]; p < FOM_MAT_t100_t_t95_ROWPTR[i + 1]; ++p) s += FOM_MAT_t100_t_t95_VALS[p] * t_t95[FOM_MAT_t100_t_t95_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t7_t_t4_ROWPTR[i]; p < FOM_MAT_t7_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t7_t_t4_VALS[p] * t_t97[FOM_MAT_t7_t_t4_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t100_t_t98_ROWPTR[i]; p < FOM_MAT_t100_t_t98_ROWPTR[i + 1]; ++p) s += FOM_MAT_t100_t_t98_VALS[p] * t_t98[FOM_MAT_t100_t_t98_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t101(const fom_float *t_t98, const fom_float *t_t99, const fom_float *t_t100, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 600; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t2_ROWPTR[i]; p < FOM_MAT_t5_t_t2_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t2_VALS[p] * t_t98[FOM_MAT_t5_t_t2_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t2_t_t1_ROWPTR[i]; p < FOM_MAT_t2_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t2_t_t1_VALS[p] * t_t99[FOM_MAT_t2_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t4_ROWPTR[i]; p < FOM_MAT_t5_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t4_VALS[p] * t_t100[FOM_MAT_t5_t_t4_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t102(const fom_float *t_t98, const fom_float *t_t99, const fom_float *t_t101, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 20; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t102_t_t98_ROWPTR[i]; p < FOM_MAT_t102_t_t98_ROWPTR[i + 1]; ++p) s += FOM_MAT_t102_t_t98_VALS[p] * t_t98[FOM_MAT_t102_t_t98_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t3_t_t1_ROWPTR[i]; p < FOM_MAT_t3_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t3_t_t1_VALS[p] * t_t99[FOM_MAT_t3_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t102_t_t101_ROWPTR[i]; p < FOM_MAT_t102_t_t101_ROWPTR[i + 1]; ++p) s += FOM_MAT_t102_t_t101_VALS[p] * t_t101[FOM_MAT_t102_t_t101_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t103(const fom_float *t_t98, const fom_float *t_t100, const fom_float *t_t101, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 30; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t103_t_t98_ROWPTR[i]; p < FOM_MAT_t103_t_t98_ROWPTR[i + 1]; ++p) s += FOM_MAT_t103_t_t98_VALS[p] * t_t98[FOM_MAT_t103_t_t98_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t7_t_t4_ROWPTR[i]; p < FOM_MAT_t7_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t7_t_t4_VALS[p] * t_t100[FOM_MAT_t7_t_t4_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t103_t_t101_ROWPTR[i]; p < FOM_MAT_t103_t_t101_ROWPTR[i + 1]; ++p) s += FOM_MAT_t103_t_t101_VALS[p] * t_t101[FOM_MAT_t103_t_t101_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t104(const fom_float *t_t101, const fom_float *t_t102, const fom_float *t_t103, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 600; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t2_ROWPTR[i]; p < FOM_MAT_t5_t_t2_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t2_VALS[p] * t_t101[FOM_MAT_t5_t_t2_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t2_t_t1_ROWPTR[i]; p < FOM_MAT_t2_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t2_t_t1_VALS[p] * t_t102[FOM_MAT_t2_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t4_ROWPTR[i]; p < FOM_MAT_t5_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t4_VALS[p] * t_t103[FOM_MAT_t5_t_t4_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t105(const fom_float *t_t101, const fom_float *t_t102, const fom_float *t_t104, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 20; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t105_t_t101_ROWPTR[i]; p < FOM_MAT_t105_t_t101_ROWPTR[i + 1]; ++p) s += FOM_MAT_t105_t_t101_VALS[p] * t_t101[FOM_MAT_t105_t_t101_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t3_t_t1_ROWPTR[i]; p < FOM_MAT_t3_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t3_t_t1_VALS[p] * t_t102[FOM_MAT_t3_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t105_t_t104_ROWPTR[i]; p < FOM_MAT_t105_t_t104_ROWPTR[i + 1]; ++p) s += FOM_MAT_t105_t_t104_VALS[p] * t_t104[FOM_MAT_t105_t_t104_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t106(const fom_float *t_t101, const fom_float *t_t103, const fom_float *t_t104, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 30; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t106_t_t101_ROWPTR[i]; p < FOM_MAT_t106_t_t101_ROWPTR[i + 1]; ++p) s += FOM_MAT_t106_t_t101_VALS[p] * t_t101[FOM_MAT_t106_t_t101_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t7_t_t4_ROWPTR[i]; p < FOM_MAT_t7_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t7_t_t4_VALS[p] * t_t103[FOM_MAT_t7_t_t4_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t106_t_t104_ROWPTR[i]; p < FOM_MAT_t106_t_t104_ROWPTR[i + 1]; ++p) s += FOM_MAT_t106_t_t104_VALS[p] * t_t104[FOM_MAT_t106_t_t104_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t107(const fom_float *t_t104, const fom_float *t_t105, const fom_float *t_t106, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 600; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t2_ROWPTR[i]; p < FOM_MAT_t5_t_t2_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t2_VALS[p] * t_t104[FOM_MAT_t5_t_t2_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t2_t_t1_ROWPTR[i]; p < FOM_MAT_t2_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t2_t_t1_VALS[p] * t_t105[FOM_MAT_t2_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t4_ROWPTR[i]; p < FOM_MAT_t5_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t4_VALS[p] * t_t106[FOM_MAT_t5_t_t4_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t108(const fom_float *t_t104, const fom_float *t_t105, const fom_float *t_t107, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 20; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t108_t_t104_ROWPTR[i]; p < FOM_MAT_t108_t_t104_ROWPTR[i + 1]; ++p) s += FOM_MAT_t108_t_t104_VALS[p] * t_t104[FOM_MAT_t108_t_t104_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t3_t_t1_ROWPTR[i]; p < FOM_MAT_t3_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t3_t_t1_VALS[p] * t_t105[FOM_MAT_t3_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t108_t_t107_ROWPTR[i]; p < FOM_MAT_t108_t_t107_ROWPTR[i + 1]; ++p) s += FOM_MAT_t108_t_t107_VALS[p] * t_t107[FOM_MAT_t108_t_t107_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t109(const fom_float *t_t104, const fom_float *t_t106, const fom_float *t_t107, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 30; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t109_t_t104_ROWPTR[i]; p < FOM_MAT_t109_t_t104_ROWPTR[i + 1]; ++p) s += FOM_MAT_t109_t_t104_VALS[p] * t_t104[FOM_MAT_t109_t_t104_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t7_t_t4_ROWPTR[i]; p < FOM_MAT_t7_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t7_t_t4_VALS[p] * t_t106[FOM_MAT_t7_t_t4_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t109_t_t107_ROWPTR[i]; p < FOM_MAT_t109_t_t107_ROWPTR[i + 1]; ++p) s += FOM_MAT_t109_t_t107_VALS[p] * t_t107[FOM_MAT_t109_t_t107_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t110(const fom_float *t_t107, const fom_float *t_t108, const fom_float *t_t109, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 600; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t2_ROWPTR[i]; p < FOM_MAT_t5_t_t2_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t2_VALS[p] * t_t107[FOM_MAT_t5_t_t2_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t2_t_t1_ROWPTR[i]; p < FOM_MAT_t2_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t2_t_t1_VALS[p] * t_t108[FOM_MAT_t2_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t4_ROWPTR[i]; p < FOM_MAT_t5_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t4_VALS[p] * t_t109[FOM_MAT_t5_t_t4_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t111(const fom_float *t_t107, const fom_float *t_t108, const fom_float *t_t110, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 20; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t111_t_t107_ROWPTR[i]; p < FOM_MAT_t111_t_t107_ROWPTR[i + 1]; ++p) s += FOM_MAT_t111_t_t107_VALS[p] * t_t107[FOM_MAT_t111_t_t107_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t3_t_t1_ROWPTR[i]; p < FOM_MAT_t3_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t3_t_t1_VALS[p] * t_t108[FOM_MAT_t3_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t111_t_t110_ROWPTR[i]; p < FOM_MAT_t111_t_t110_ROWPTR[i + 1]; ++p) s += FOM_MAT_t111_t_t110_VALS[p] * t_t110[FOM_MAT_t111_t_t110_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t112(const fom_float *t_t107, const fom_float *t_t109, const fom_float *t_t110, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 30; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t112_t_t107_ROWPTR[i]; p < FOM_MAT_t112_t_t107_ROWPTR[i + 1]; ++p) s += FOM_MAT_t112_t_t107_VALS[p] * t_t107[FOM_MAT_t112_t_t107_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t7_t_t4_ROWPTR[i]; p < FOM_MAT_t7_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t7_t_t4_VALS[p] * t_t109[FOM_MAT_t7_t_t4_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t112_t_t110_ROWPTR[i]; p < FOM_MAT_t112_t_t110_ROWPTR[i + 1]; ++p) s += FOM_MAT_t112_t_t110_VALS[p] * t_t110[FOM_MAT_t112_t_t110_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t113(const fom_float *t_t110, const fom_float *t_t111, const fom_float *t_t112, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 600; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t2_ROWPTR[i]; p < FOM_MAT_t5_t_t2_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t2_VALS[p] * t_t110[FOM_MAT_t5_t_t2_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t2_t_t1_ROWPTR[i]; p < FOM_MAT_t2_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t2_t_t1_VALS[p] * t_t111[FOM_MAT_t2_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t4_ROWPTR[i]; p < FOM_MAT_t5_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t4_VALS[p] * t_t112[FOM_MAT_t5_t_t4_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t114(const fom_float *t_t110, const fom_float *t_t111, const fom_float *t_t113, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 20; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t114_t_t110_ROWPTR[i]; p < FOM_MAT_t114_t_t110_ROWPTR[i + 1]; ++p) s += FOM_MAT_t114_t_t110_VALS[p] * t_t110[FOM_MAT_t114_t_t110_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t3_t_t1_ROWPTR[i]; p < FOM_MAT_t3_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t3_t_t1_VALS[p] * t_t111[FOM_MAT_t3_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t114_t_t113_ROWPTR[i]; p < FOM_MAT_t114_t_t113_ROWPTR[i + 1]; ++p) s += FOM_MAT_t114_t_t113_VALS[p] * t_t113[FOM_MAT_t114_t_t113_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t115(const fom_float *t_t110, const fom_float *t_t112, const fom_float *t_t113, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 30; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t115_t_t110_ROWPTR[i]; p < FOM_MAT_t115_t_t110_ROWPTR[i + 1]; ++p) s += FOM_MAT_t115_t_t110_VALS[p] * t_t110[FOM_MAT_t115_t_t110_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t7_t_t4_ROWPTR[i]; p < FOM_MAT_t7_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t7_t_t4_VALS[p] * t_t112[FOM_MAT_t7_t_t4_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t115_t_t113_ROWPTR[i]; p < FOM_MAT_t115_t_t113_ROWPTR[i + 1]; ++p) s += FOM_MAT_t115_t_t113_VALS[p] * t_t113[FOM_MAT_t115_t_t113_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t116(const fom_float *t_t113, const fom_float *t_t114, const fom_float *t_t115, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 600; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t2_ROWPTR[i]; p < FOM_MAT_t5_t_t2_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t2_VALS[p] * t_t113[FOM_MAT_t5_t_t2_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t2_t_t1_ROWPTR[i]; p < FOM_MAT_t2_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t2_t_t1_VALS[p] * t_t114[FOM_MAT_t2_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t4_ROWPTR[i]; p < FOM_MAT_t5_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t4_VALS[p] * t_t115[FOM_MAT_t5_t_t4_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t117(const fom_float *t_t113, const fom_float *t_t114, const fom_float *t_t116, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 20; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t117_t_t113_ROWPTR[i]; p < FOM_MAT_t117_t_t113_ROWPTR[i + 1]; ++p) s += FOM_MAT_t117_t_t113_VALS[p] * t_t113[FOM_MAT_t117_t_t113_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t3_t_t1_ROWPTR[i]; p < FOM_MAT_t3_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t3_t_t1_VALS[p] * t_t114[FOM_MAT_t3_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t117_t_t116_ROWPTR[i]; p < FOM_MAT_t117_t_t116_ROWPTR[i + 1]; ++p) s += FOM_MAT_t117_t_t116_VALS[p] * t_t116[FOM_MAT_t117_t_t116_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t118(const fom_float *t_t113, const fom_float *t_t115, const fom_float *t_t116, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 30; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t118_t_t113_ROWPTR[i]; p < FOM_MAT_t118_t_t113_ROWPTR[i + 1]; ++p) s += FOM_MAT_t118_t_t113_VALS[p] * t_t113[FOM_MAT_t118_t_t113_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t7_t_t4_ROWPTR[i]; p < FOM_MAT_t7_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t7_t_t4_VALS[p] * t_t115[FOM_MAT_t7_t_t4_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t118_t_t116_ROWPTR[i]; p < FOM_MAT_t118_t_t116_ROWPTR[i + 1]; ++p) s += FOM_MAT_t118_t_t116_VALS[p] * t_t116[FOM_MAT_t118_t_t116_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t119(const fom_float *t_t116, const fom_float *t_t117, const fom_float *t_t118, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 600; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t2_ROWPTR[i]; p < FOM_MAT_t5_t_t2_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t2_VALS[p] * t_t116[FOM_MAT_t5_t_t2_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t2_t_t1_ROWPTR[i]; p < FOM_MAT_t2_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t2_t_t1_VALS[p] * t_t117[FOM_MAT_t2_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t4_ROWPTR[i]; p < FOM_MAT_t5_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t4_VALS[p] * t_t118[FOM_MAT_t5_t_t4_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t120(const fom_float *t_t116, const fom_float *t_t117, const fom_float *t_t119, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 20; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t120_t_t116_ROWPTR[i]; p < FOM_MAT_t120_t_t116_ROWPTR[i + 1]; ++p) s += FOM_MAT_t120_t_t116_VALS[p] * t_t116[FOM_MAT_t120_t_t116_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t3_t_t1_ROWPTR[i]; p < FOM_MAT_t3_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t3_t_t1_VALS[p] * t_t117[FOM_MAT_t3_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t120_t_t119_ROWPTR[i]; p < FOM_MAT_t120_t_t119_ROWPTR[i + 1]; ++p) s += FOM_MAT_t120_t_t119_VALS[p] * t_t119[FOM_MAT_t120_t_t119_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t121(const fom_float *t_t116, const fom_float *t_t118, const fom_float *t_t119, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 30; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t121_t_t116_ROWPTR[i]; p < FOM_MAT_t121_t_t116_ROWPTR[i + 1]; ++p) s += FOM_MAT_t121_t_t116_VALS[p] * t_t116[FOM_MAT_t121_t_t116_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t7_t_t4_ROWPTR[i]; p < FOM_MAT_t7_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t7_t_t4_VALS[p] * t_t118[FOM_MAT_t7_t_t4_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t121_t_t119_ROWPTR[i]; p < FOM_MAT_t121_t_t119_ROWPTR[i + 1]; ++p) s += FOM_MAT_t121_t_t119_VALS[p] * t_t119[FOM_MAT_t121_t_t119_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t122(const fom_float *t_t119, const fom_float *t_t120, const fom_float *t_t121, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 600; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t2_ROWPTR[i]; p < FOM_MAT_t5_t_t2_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t2_VALS[p] * t_t119[FOM_MAT_t5_t_t2_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t2_t_t1_ROWPTR[i]; p < FOM_MAT_t2_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t2_t_t1_VALS[p] * t_t120[FOM_MAT_t2_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t4_ROWPTR[i]; p < FOM_MAT_t5_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t4_VALS[p] * t_t121[FOM_MAT_t5_t_t4_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t123(const fom_float *t_t119, const fom_float *t_t120, const fom_float *t_t122, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 20; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t123_t_t119_ROWPTR[i]; p < FOM_MAT_t123_t_t119_ROWPTR[i + 1]; ++p) s += FOM_MAT_t123_t_t119_VALS[p] * t_t119[FOM_MAT_t123_t_t119_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t3_t_t1_ROWPTR[i]; p < FOM_MAT_t3_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t3_t_t1_VALS[p] * t_t120[FOM_MAT_t3_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t123_t_t122_ROWPTR[i]; p < FOM_MAT_t123_t_t122_ROWPTR[i + 1]; ++p) s += FOM_MAT_t123_t_t122_VALS[p] * t_t122[FOM_MAT_t123_t_t122_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t124(const fom_float *t_t119, const fom_float *t_t121, const fom_float *t_t122, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 30; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t124_t_t119_ROWPTR[i]; p < FOM_MAT_t124_t_t119_ROWPTR[i + 1]; ++p) s += FOM_MAT_t124_t_t119_VALS[p] * t_t119[FOM_MAT_t124_t_t119_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t7_t_t4_ROWPTR[i]; p < FOM_MAT_t7_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t7_t_t4_VALS[p] * t_t121[FOM_MAT_t7_t_t4_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t124_t_t122_ROWPTR[i]; p < FOM_MAT_t124_t_t122_ROWPTR[i + 1]; ++p) s += FOM_MAT_t124_t_t122_VALS[p] * t_t122[FOM_MAT_t124_t_t122_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t125(const fom_float *t_t122, const fom_float *t_t123, const fom_float *t_t124, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 600; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t2_ROWPTR[i]; p < FOM_MAT_t5_t_t2_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t2_VALS[p] * t_t122[FOM_MAT_t5_t_t2_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t2_t_t1_ROWPTR[i]; p < FOM_MAT_t2_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t2_t_t1_VALS[p] * t_t123[FOM_MAT_t2_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t4_ROWPTR[i]; p < FOM_MAT_t5_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t4_VALS[p] * t_t124[FOM_MAT_t5_t_t4_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t126(const fom_float *t_t122, const fom_float *t_t123, const fom_float *t_t125, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 20; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t126_t_t122_ROWPTR[i]; p < FOM_MAT_t126_t_t122_ROWPTR[i + 1]; ++p) s += FOM_MAT_t126_t_t122_VALS[p] * t_t122[FOM_MAT_t126_t_t122_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t3_t_t1_ROWPTR[i]; p < FOM_MAT_t3_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t3_t_t1_VALS[p] * t_t123[FOM_MAT_t3_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t126_t_t125_ROWPTR[i]; p < FOM_MAT_t126_t_t125_ROWPTR[i + 1]; ++p) s += FOM_MAT_t126_t_t125_VALS[p] * t_t125[FOM_MAT_t126_t_t125_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t127(const fom_float *t_t122, const fom_float *t_t124, const fom_float *t_t125, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 30; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t127_t_t122_ROWPTR[i]; p < FOM_MAT_t127_t_t122_ROWPTR[i + 1]; ++p) s += FOM_MAT_t127_t_t122_VALS[p] * t_t122[FOM_MAT_t127_t_t122_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t7_t_t4_ROWPTR[i]; p < FOM_MAT_t7_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t7_t_t4_VALS[p] * t_t124[FOM_MAT_t7_t_t4_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t127_t_t125_ROWPTR[i]; p < FOM_MAT_t127_t_t125_ROWPTR[i + 1]; ++p) s += FOM_MAT_t127_t_t125_VALS[p] * t_t125[FOM_MAT_t127_t_t125_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t128(const fom_float *t_t125, const fom_float *t_t126, const fom_float *t_t127, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 600; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t2_ROWPTR[i]; p < FOM_MAT_t5_t_t2_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t2_VALS[p] * t_t125[FOM_MAT_t5_t_t2_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t2_t_t1_ROWPTR[i]; p < FOM_MAT_t2_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t2_t_t1_VALS[p] * t_t126[FOM_MAT_t2_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t4_ROWPTR[i]; p < FOM_MAT_t5_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t4_VALS[p] * t_t127[FOM_MAT_t5_t_t4_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t129(const fom_float *t_t125, const fom_float *t_t126, const fom_float *t_t128, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 20; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t129_t_t125_ROWPTR[i]; p < FOM_MAT_t129_t_t125_ROWPTR[i + 1]; ++p) s += FOM_MAT_t129_t_t125_VALS[p] * t_t125[FOM_MAT_t129_t_t125_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t3_t_t1_ROWPTR[i]; p < FOM_MAT_t3_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t3_t_t1_VALS[p] * t_t126[FOM_MAT_t3_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t129_t_t128_ROWPTR[i]; p < FOM_MAT_t129_t_t128_ROWPTR[i + 1]; ++p) s += FOM_MAT_t129_t_t128_VALS[p] * t_t128[FOM_MAT_t129_t_t128_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t130(const fom_float *t_t125, const fom_float *t_t127, const fom_float *t_t128, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 30; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t130_t_t125_ROWPTR[i]; p < FOM_MAT_t130_t_t125_ROWPTR[i + 1]; ++p) s += FOM_MAT_t130_t_t125_VALS[p] * t_t125[FOM_MAT_t130_t_t125_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t7_t_t4_ROWPTR[i]; p < FOM_MAT_t7_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t7_t_t4_VALS[p] * t_t127[FOM_MAT_t7_t_t4_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t130_t_t128_ROWPTR[i]; p < FOM_MAT_t130_t_t128_ROWPTR[i + 1]; ++p) s += FOM_MAT_t130_t_t128_VALS[p] * t_t128[FOM_MAT_t130_t_t128_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t131(const fom_float *t_t128, const fom_float *t_t129, const fom_float *t_t130, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 600; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t2_ROWPTR[i]; p < FOM_MAT_t5_t_t2_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t2_VALS[p] * t_t128[FOM_MAT_t5_t_t2_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t2_t_t1_ROWPTR[i]; p < FOM_MAT_t2_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t2_t_t1_VALS[p] * t_t129[FOM_MAT_t2_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t4_ROWPTR[i]; p < FOM_MAT_t5_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t4_VALS[p] * t_t130[FOM_MAT_t5_t_t4_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t132(const fom_float *t_t128, const fom_float *t_t129, const fom_float *t_t131, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 20; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t132_t_t128_ROWPTR[i]; p < FOM_MAT_t132_t_t128_ROWPTR[i + 1]; ++p) s += FOM_MAT_t132_t_t128_VALS[p] * t_t128[FOM_MAT_t132_t_t128_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t3_t_t1_ROWPTR[i]; p < FOM_MAT_t3_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t3_t_t1_VALS[p] * t_t129[FOM_MAT_t3_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t132_t_t131_ROWPTR[i]; p < FOM_MAT_t132_t_t131_ROWPTR[i + 1]; ++p) s += FOM_MAT_t132_t_t131_VALS[p] * t_t131[FOM_MAT_t132_t_t131_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t133(const fom_float *t_t128, const fom_float *t_t130, const fom_float *t_t131, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 30; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t133_t_t128_ROWPTR[i]; p < FOM_MAT_t133_t_t128_ROWPTR[i + 1]; ++p) s += FOM_MAT_t133_t_t128_VALS[p] * t_t128[FOM_MAT_t133_t_t128_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t7_t_t4_ROWPTR[i]; p < FOM_MAT_t7_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t7_t_t4_VALS[p] * t_t130[FOM_MAT_t7_t_t4_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t133_t_t131_ROWPTR[i]; p < FOM_MAT_t133_t_t131_ROWPTR[i + 1]; ++p) s += FOM_MAT_t133_t_t131_VALS[p] * t_t131[FOM_MAT_t133_t_t131_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t134(const fom_float *t_t131, const fom_float *t_t132, const fom_float *t_t133, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 600; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t2_ROWPTR[i]; p < FOM_MAT_t5_t_t2_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t2_VALS[p] * t_t131[FOM_MAT_t5_t_t2_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t2_t_t1_ROWPTR[i]; p < FOM_MAT_t2_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t2_t_t1_VALS[p] * t_t132[FOM_MAT_t2_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t4_ROWPTR[i]; p < FOM_MAT_t5_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t4_VALS[p] * t_t133[FOM_MAT_t5_t_t4_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t135(const fom_float *t_t131, const fom_float *t_t132, const fom_float *t_t134, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 20; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t135_t_t131_ROWPTR[i]; p < FOM_MAT_t135_t_t131_ROWPTR[i + 1]; ++p) s += FOM_MAT_t135_t_t131_VALS[p] * t_t131[FOM_MAT_t135_t_t131_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t3_t_t1_ROWPTR[i]; p < FOM_MAT_t3_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t3_t_t1_VALS[p] * t_t132[FOM_MAT_t3_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t135_t_t134_ROWPTR[i]; p < FOM_MAT_t135_t_t134_ROWPTR[i + 1]; ++p) s += FOM_MAT_t135_t_t134_VALS[p] * t_t134[FOM_MAT_t135_t_t134_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t136(const fom_float *t_t131, const fom_float *t_t133, const fom_float *t_t134, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 30; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t136_t_t131_ROWPTR[i]; p < FOM_MAT_t136_t_t131_ROWPTR[i + 1]; ++p) s += FOM_MAT_t136_t_t131_VALS[p] * t_t131[FOM_MAT_t136_t_t131_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t7_t_t4_ROWPTR[i]; p < FOM_MAT_t7_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t7_t_t4_VALS[p] * t_t133[FOM_MAT_t7_t_t4_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t136_t_t134_ROWPTR[i]; p < FOM_MAT_t136_t_t134_ROWPTR[i + 1]; ++p) s += FOM_MAT_t136_t_t134_VALS[p] * t_t134[FOM_MAT_t136_t_t134_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t137(const fom_float *t_t134, const fom_float *t_t135, const fom_float *t_t136, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 600; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t2_ROWPTR[i]; p < FOM_MAT_t5_t_t2_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t2_VALS[p] * t_t134[FOM_MAT_t5_t_t2_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t2_t_t1_ROWPTR[i]; p < FOM_MAT_t2_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t2_t_t1_VALS[p] * t_t135[FOM_MAT_t2_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t4_ROWPTR[i]; p < FOM_MAT_t5_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t4_VALS[p] * t_t136[FOM_MAT_t5_t_t4_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t138(const fom_float *t_t134, const fom_float *t_t135, const fom_float *t_t137, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 20; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t138_t_t134_ROWPTR[i]; p < FOM_MAT_t138_t_t134_ROWPTR[i + 1]; ++p) s += FOM_MAT_t138_t_t134_VALS[p] * t_t134[FOM_MAT_t138_t_t134_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t3_t_t1_ROWPTR[i]; p < FOM_MAT_t3_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t3_t_t1_VALS[p] * t_t135[FOM_MAT_t3_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t138_t_t137_ROWPTR[i]; p < FOM_MAT_t138_t_t137_ROWPTR[i + 1]; ++p) s += FOM_MAT_t138_t_t137_VALS[p] * t_t137[FOM_MAT_t138_t_t137_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t139(const fom_float *t_t134, const fom_float *t_t136, const fom_float *t_t137, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 30; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t139_t_t134_ROWPTR[i]; p < FOM_MAT_t139_t_t134_ROWPTR[i + 1]; ++p) s += FOM_MAT_t139_t_t134_VALS[p] * t_t134[FOM_MAT_t139_t_t134_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t7_t_t4_ROWPTR[i]; p < FOM_MAT_t7_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t7_t_t4_VALS[p] * t_t136[FOM_MAT_t7_t_t4_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t139_t_t137_ROWPTR[i]; p < FOM_MAT_t139_t_t137_ROWPTR[i + 1]; ++p) s += FOM_MAT_t139_t_t137_VALS[p] * t_t137[FOM_MAT_t139_t_t137_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t140(const fom_float *t_t137, const fom_float *t_t138, const fom_float *t_t139, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 600; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t2_ROWPTR[i]; p < FOM_MAT_t5_t_t2_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t2_VALS[p] * t_t137[FOM_MAT_t5_t_t2_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t2_t_t1_ROWPTR[i]; p < FOM_MAT_t2_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t2_t_t1_VALS[p] * t_t138[FOM_MAT_t2_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t4_ROWPTR[i]; p < FOM_MAT_t5_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t4_VALS[p] * t_t139[FOM_MAT_t5_t_t4_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t141(const fom_float *t_t137, const fom_float *t_t138, const fom_float *t_t140, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 20; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t141_t_t137_ROWPTR[i]; p < FOM_MAT_t141_t_t137_ROWPTR[i + 1]; ++p) s += FOM_MAT_t141_t_t137_VALS[p] * t_t137[FOM_MAT_t141_t_t137_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t3_t_t1_ROWPTR[i]; p < FOM_MAT_t3_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t3_t_t1_VALS[p] * t_t138[FOM_MAT_t3_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t141_t_t140_ROWPTR[i]; p < FOM_MAT_t141_t_t140_ROWPTR[i + 1]; ++p) s += FOM_MAT_t141_t_t140_VALS[p] * t_t140[FOM_MAT_t141_t_t140_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t142(const fom_float *t_t137, const fom_float *t_t139, const fom_float *t_t140, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 30; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t142_t_t137_ROWPTR[i]; p < FOM_MAT_t142_t_t137_ROWPTR[i + 1]; ++p) s += FOM_MAT_t142_t_t137_VALS[p] * t_t137[FOM_MAT_t142_t_t137_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t7_t_t4_ROWPTR[i]; p < FOM_MAT_t7_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t7_t_t4_VALS[p] * t_t139[FOM_MAT_t7_t_t4_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t142_t_t140_ROWPTR[i]; p < FOM_MAT_t142_t_t140_ROWPTR[i + 1]; ++p) s += FOM_MAT_t142_t_t140_VALS[p] * t_t140[FOM_MAT_t142_t_t140_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t143(const fom_float *t_t140, const fom_float *t_t141, const fom_float *t_t142, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 600; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t2_ROWPTR[i]; p < FOM_MAT_t5_t_t2_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t2_VALS[p] * t_t140[FOM_MAT_t5_t_t2_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t2_t_t1_ROWPTR[i]; p < FOM_MAT_t2_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t2_t_t1_VALS[p] * t_t141[FOM_MAT_t2_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t4_ROWPTR[i]; p < FOM_MAT_t5_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t4_VALS[p] * t_t142[FOM_MAT_t5_t_t4_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t144(const fom_float *t_t140, const fom_float *t_t141, const fom_float *t_t143, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 20; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t144_t_t140_ROWPTR[i]; p < FOM_MAT_t144_t_t140_ROWPTR[i + 1]; ++p) s += FOM_MAT_t144_t_t140_VALS[p] * t_t140[FOM_MAT_t144_t_t140_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t3_t_t1_ROWPTR[i]; p < FOM_MAT_t3_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t3_t_t1_VALS[p] * t_t141[FOM_MAT_t3_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t144_t_t143_ROWPTR[i]; p < FOM_MAT_t144_t_t143_ROWPTR[i + 1]; ++p) s += FOM_MAT_t144_t_t143_VALS[p] * t_t143[FOM_MAT_t144_t_t143_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t145(const fom_float *t_t140, const fom_float *t_t142, const fom_float *t_t143, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 30; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t145_t_t140_ROWPTR[i]; p < FOM_MAT_t145_t_t140_ROWPTR[i + 1]; ++p) s += FOM_MAT_t145_t_t140_VALS[p] * t_t140[FOM_MAT_t145_t_t140_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t7_t_t4_ROWPTR[i]; p < FOM_MAT_t7_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t7_t_t4_VALS[p] * t_t142[FOM_MAT_t7_t_t4_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t145_t_t143_ROWPTR[i]; p < FOM_MAT_t145_t_t143_ROWPTR[i + 1]; ++p) s += FOM_MAT_t145_t_t143_VALS[p] * t_t143[FOM_MAT_t145_t_t143_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t146(const fom_float *t_t143, const fom_float *t_t144, const fom_float *t_t145, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 600; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t2_ROWPTR[i]; p < FOM_MAT_t5_t_t2_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t2_VALS[p] * t_t143[FOM_MAT_t5_t_t2_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t2_t_t1_ROWPTR[i]; p < FOM_MAT_t2_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t2_t_t1_VALS[p] * t_t144[FOM_MAT_t2_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t5_t_t4_ROWPTR[i]; p < FOM_MAT_t5_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t5_t_t4_VALS[p] * t_t145[FOM_MAT_t5_t_t4_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t147(const fom_float *t_t143, const fom_float *t_t144, const fom_float *t_t146, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 20; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t147_t_t143_ROWPTR[i]; p < FOM_MAT_t147_t_t143_ROWPTR[i + 1]; ++p) s += FOM_MAT_t147_t_t143_VALS[p] * t_t143[FOM_MAT_t147_t_t143_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t3_t_t1_ROWPTR[i]; p < FOM_MAT_t3_t_t1_ROWPTR[i + 1]; ++p) s += FOM_MAT_t3_t_t1_VALS[p] * t_t144[FOM_MAT_t3_t_t1_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t147_t_t146_ROWPTR[i]; p < FOM_MAT_t147_t_t146_ROWPTR[i + 1]; ++p) s += FOM_MAT_t147_t_t146_VALS[p] * t_t146[FOM_MAT_t147_t_t146_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_t148(const fom_float *t_t143, const fom_float *t_t145, const fom_float *t_t146, fom_float *out) {
-    fom_int i, p;
-    for (i = 0; i < 30; ++i) {
-        fom_float s;
-        s = 0.0;
-        for (p = FOM_MAT_t148_t_t143_ROWPTR[i]; p < FOM_MAT_t148_t_t143_ROWPTR[i + 1]; ++p) s += FOM_MAT_t148_t_t143_VALS[p] * t_t143[FOM_MAT_t148_t_t143_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t7_t_t4_ROWPTR[i]; p < FOM_MAT_t7_t_t4_ROWPTR[i + 1]; ++p) s += FOM_MAT_t7_t_t4_VALS[p] * t_t145[FOM_MAT_t7_t_t4_COLIDX[p]];
-        out[i] += s;
-        s = 0.0;
-        for (p = FOM_MAT_t148_t_t146_ROWPTR[i]; p < FOM_MAT_t148_t_t146_ROWPTR[i + 1]; ++p) s += FOM_MAT_t148_t_t146_VALS[p] * t_t146[FOM_MAT_t148_t_t146_COLIDX[p]];
-        out[i] += s;
-    }
-}
-
-void fom_fused_su(const fom_float *t_t146, const fom_float *t_t147, const fom_float *t_t148, fom_float *out) {
+        s += FOM_MAT_t2_t_t1_VALS[i] * t_t1[FOM_MAT_t2_t_t1_COLIDX[i]];
+        out[i] += s;
+    }
+}
+
+void fom_fused_t3(const fom_float *restrict t_t0, const fom_float *restrict t_t1, const fom_float *restrict t_t2, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 20; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t3_t_t1_VALS[i] * t_t1[FOM_MAT_t3_t_t1_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 30 * i; p < 30 * i + 30; ++p) { fom_int c = p; s += FOM_MAT_t3_t_t0_VALS[p] * t_t0[c] + FOM_MAT_t3_t_t2_VALS[p] * t_t2[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t4(const fom_float *restrict state, const fom_float *restrict t_t0, const fom_float *restrict t_t2, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 30; ++i) {
+        fom_float s;
+        s = 0.0;
+        for (p = FOM_MAT_t4_state_ROWPTR[i]; p < FOM_MAT_t4_state_ROWPTR[i + 1]; ++p) { fom_int c = FOM_MAT_t4_state_COLIDX[p]; s += FOM_MAT_t4_state_VALS[p] * state[c]; }
+        out[i] += s;
+        s = 0.0;
+        for (p = 20 * i; p < 20 * i + 20; ++p) { fom_int c = 30 * (p - 20 * i) + i; s += FOM_MAT_t4_t_t0_VALS[p] * t_t0[c] + FOM_MAT_t4_t_t2_VALS[p] * t_t2[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t5(const fom_float *restrict t_t2, const fom_float *restrict t_t3, const fom_float *restrict t_t4, fom_float *restrict out) {
+    fom_int i, p;
+    for (p = 0; p < 20; ++p) {
+        const fom_float y = t_t3[p];
+        for (i = 30 * p; i < 30 * p + 30; ++i)
+            out[i] += FOM_MAT_t5_t_t2_VALS[i] * t_t2[i] + FOM_MAT_t2_t_t1_VALS[i] * y + FOM_MAT_t5_t_t4_VALS[i] * t_t4[i - 30 * p];
+    }
+}
+
+void fom_fused_t6(const fom_float *restrict t_t2, const fom_float *restrict t_t3, const fom_float *restrict t_t5, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 20; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t3_t_t1_VALS[i] * t_t3[FOM_MAT_t3_t_t1_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 30 * i; p < 30 * i + 30; ++p) { fom_int c = p; s += FOM_MAT_t6_t_t2_VALS[p] * t_t2[c] + FOM_MAT_t6_t_t5_VALS[p] * t_t5[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t7(const fom_float *restrict t_t2, const fom_float *restrict t_t4, const fom_float *restrict t_t5, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 30; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t7_t_t4_VALS[i] * t_t4[FOM_MAT_t7_t_t4_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 20 * i; p < 20 * i + 20; ++p) { fom_int c = 30 * (p - 20 * i) + i; s += FOM_MAT_t7_t_t2_VALS[p] * t_t2[c] + FOM_MAT_t7_t_t5_VALS[p] * t_t5[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t8(const fom_float *restrict t_t5, const fom_float *restrict t_t6, const fom_float *restrict t_t7, fom_float *restrict out) {
+    fom_int i, p;
+    for (p = 0; p < 20; ++p) {
+        const fom_float y = t_t6[p];
+        for (i = 30 * p; i < 30 * p + 30; ++i)
+            out[i] += FOM_MAT_t5_t_t2_VALS[i] * t_t5[i] + FOM_MAT_t2_t_t1_VALS[i] * y + FOM_MAT_t5_t_t4_VALS[i] * t_t7[i - 30 * p];
+    }
+}
+
+void fom_fused_t9(const fom_float *restrict t_t5, const fom_float *restrict t_t6, const fom_float *restrict t_t8, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 20; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t3_t_t1_VALS[i] * t_t6[FOM_MAT_t3_t_t1_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 30 * i; p < 30 * i + 30; ++p) { fom_int c = p; s += FOM_MAT_t9_t_t5_VALS[p] * t_t5[c] + FOM_MAT_t9_t_t8_VALS[p] * t_t8[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t10(const fom_float *restrict t_t5, const fom_float *restrict t_t7, const fom_float *restrict t_t8, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 30; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t7_t_t4_VALS[i] * t_t7[FOM_MAT_t7_t_t4_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 20 * i; p < 20 * i + 20; ++p) { fom_int c = 30 * (p - 20 * i) + i; s += FOM_MAT_t10_t_t5_VALS[p] * t_t5[c] + FOM_MAT_t10_t_t8_VALS[p] * t_t8[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t11(const fom_float *restrict t_t8, const fom_float *restrict t_t9, const fom_float *restrict t_t10, fom_float *restrict out) {
+    fom_int i, p;
+    for (p = 0; p < 20; ++p) {
+        const fom_float y = t_t9[p];
+        for (i = 30 * p; i < 30 * p + 30; ++i)
+            out[i] += FOM_MAT_t5_t_t2_VALS[i] * t_t8[i] + FOM_MAT_t2_t_t1_VALS[i] * y + FOM_MAT_t5_t_t4_VALS[i] * t_t10[i - 30 * p];
+    }
+}
+
+void fom_fused_t12(const fom_float *restrict t_t8, const fom_float *restrict t_t9, const fom_float *restrict t_t11, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 20; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t3_t_t1_VALS[i] * t_t9[FOM_MAT_t3_t_t1_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 30 * i; p < 30 * i + 30; ++p) { fom_int c = p; s += FOM_MAT_t12_t_t8_VALS[p] * t_t8[c] + FOM_MAT_t12_t_t11_VALS[p] * t_t11[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t13(const fom_float *restrict t_t8, const fom_float *restrict t_t10, const fom_float *restrict t_t11, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 30; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t7_t_t4_VALS[i] * t_t10[FOM_MAT_t7_t_t4_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 20 * i; p < 20 * i + 20; ++p) { fom_int c = 30 * (p - 20 * i) + i; s += FOM_MAT_t13_t_t8_VALS[p] * t_t8[c] + FOM_MAT_t13_t_t11_VALS[p] * t_t11[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t14(const fom_float *restrict t_t11, const fom_float *restrict t_t12, const fom_float *restrict t_t13, fom_float *restrict out) {
+    fom_int i, p;
+    for (p = 0; p < 20; ++p) {
+        const fom_float y = t_t12[p];
+        for (i = 30 * p; i < 30 * p + 30; ++i)
+            out[i] += FOM_MAT_t5_t_t2_VALS[i] * t_t11[i] + FOM_MAT_t2_t_t1_VALS[i] * y + FOM_MAT_t5_t_t4_VALS[i] * t_t13[i - 30 * p];
+    }
+}
+
+void fom_fused_t15(const fom_float *restrict t_t11, const fom_float *restrict t_t12, const fom_float *restrict t_t14, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 20; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t3_t_t1_VALS[i] * t_t12[FOM_MAT_t3_t_t1_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 30 * i; p < 30 * i + 30; ++p) { fom_int c = p; s += FOM_MAT_t15_t_t11_VALS[p] * t_t11[c] + FOM_MAT_t15_t_t14_VALS[p] * t_t14[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t16(const fom_float *restrict t_t11, const fom_float *restrict t_t13, const fom_float *restrict t_t14, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 30; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t7_t_t4_VALS[i] * t_t13[FOM_MAT_t7_t_t4_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 20 * i; p < 20 * i + 20; ++p) { fom_int c = 30 * (p - 20 * i) + i; s += FOM_MAT_t16_t_t11_VALS[p] * t_t11[c] + FOM_MAT_t16_t_t14_VALS[p] * t_t14[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t17(const fom_float *restrict t_t14, const fom_float *restrict t_t15, const fom_float *restrict t_t16, fom_float *restrict out) {
+    fom_int i, p;
+    for (p = 0; p < 20; ++p) {
+        const fom_float y = t_t15[p];
+        for (i = 30 * p; i < 30 * p + 30; ++i)
+            out[i] += FOM_MAT_t5_t_t2_VALS[i] * t_t14[i] + FOM_MAT_t2_t_t1_VALS[i] * y + FOM_MAT_t5_t_t4_VALS[i] * t_t16[i - 30 * p];
+    }
+}
+
+void fom_fused_t18(const fom_float *restrict t_t14, const fom_float *restrict t_t15, const fom_float *restrict t_t17, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 20; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t3_t_t1_VALS[i] * t_t15[FOM_MAT_t3_t_t1_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 30 * i; p < 30 * i + 30; ++p) { fom_int c = p; s += FOM_MAT_t18_t_t14_VALS[p] * t_t14[c] + FOM_MAT_t18_t_t17_VALS[p] * t_t17[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t19(const fom_float *restrict t_t14, const fom_float *restrict t_t16, const fom_float *restrict t_t17, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 30; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t7_t_t4_VALS[i] * t_t16[FOM_MAT_t7_t_t4_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 20 * i; p < 20 * i + 20; ++p) { fom_int c = 30 * (p - 20 * i) + i; s += FOM_MAT_t19_t_t14_VALS[p] * t_t14[c] + FOM_MAT_t19_t_t17_VALS[p] * t_t17[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t20(const fom_float *restrict t_t17, const fom_float *restrict t_t18, const fom_float *restrict t_t19, fom_float *restrict out) {
+    fom_int i, p;
+    for (p = 0; p < 20; ++p) {
+        const fom_float y = t_t18[p];
+        for (i = 30 * p; i < 30 * p + 30; ++i)
+            out[i] += FOM_MAT_t5_t_t2_VALS[i] * t_t17[i] + FOM_MAT_t2_t_t1_VALS[i] * y + FOM_MAT_t5_t_t4_VALS[i] * t_t19[i - 30 * p];
+    }
+}
+
+void fom_fused_t21(const fom_float *restrict t_t17, const fom_float *restrict t_t18, const fom_float *restrict t_t20, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 20; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t3_t_t1_VALS[i] * t_t18[FOM_MAT_t3_t_t1_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 30 * i; p < 30 * i + 30; ++p) { fom_int c = p; s += FOM_MAT_t21_t_t17_VALS[p] * t_t17[c] + FOM_MAT_t21_t_t20_VALS[p] * t_t20[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t22(const fom_float *restrict t_t17, const fom_float *restrict t_t19, const fom_float *restrict t_t20, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 30; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t7_t_t4_VALS[i] * t_t19[FOM_MAT_t7_t_t4_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 20 * i; p < 20 * i + 20; ++p) { fom_int c = 30 * (p - 20 * i) + i; s += FOM_MAT_t22_t_t17_VALS[p] * t_t17[c] + FOM_MAT_t22_t_t20_VALS[p] * t_t20[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t23(const fom_float *restrict t_t20, const fom_float *restrict t_t21, const fom_float *restrict t_t22, fom_float *restrict out) {
+    fom_int i, p;
+    for (p = 0; p < 20; ++p) {
+        const fom_float y = t_t21[p];
+        for (i = 30 * p; i < 30 * p + 30; ++i)
+            out[i] += FOM_MAT_t5_t_t2_VALS[i] * t_t20[i] + FOM_MAT_t2_t_t1_VALS[i] * y + FOM_MAT_t5_t_t4_VALS[i] * t_t22[i - 30 * p];
+    }
+}
+
+void fom_fused_t24(const fom_float *restrict t_t20, const fom_float *restrict t_t21, const fom_float *restrict t_t23, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 20; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t3_t_t1_VALS[i] * t_t21[FOM_MAT_t3_t_t1_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 30 * i; p < 30 * i + 30; ++p) { fom_int c = p; s += FOM_MAT_t24_t_t20_VALS[p] * t_t20[c] + FOM_MAT_t24_t_t23_VALS[p] * t_t23[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t25(const fom_float *restrict t_t20, const fom_float *restrict t_t22, const fom_float *restrict t_t23, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 30; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t7_t_t4_VALS[i] * t_t22[FOM_MAT_t7_t_t4_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 20 * i; p < 20 * i + 20; ++p) { fom_int c = 30 * (p - 20 * i) + i; s += FOM_MAT_t25_t_t20_VALS[p] * t_t20[c] + FOM_MAT_t25_t_t23_VALS[p] * t_t23[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t26(const fom_float *restrict t_t23, const fom_float *restrict t_t24, const fom_float *restrict t_t25, fom_float *restrict out) {
+    fom_int i, p;
+    for (p = 0; p < 20; ++p) {
+        const fom_float y = t_t24[p];
+        for (i = 30 * p; i < 30 * p + 30; ++i)
+            out[i] += FOM_MAT_t5_t_t2_VALS[i] * t_t23[i] + FOM_MAT_t2_t_t1_VALS[i] * y + FOM_MAT_t5_t_t4_VALS[i] * t_t25[i - 30 * p];
+    }
+}
+
+void fom_fused_t27(const fom_float *restrict t_t23, const fom_float *restrict t_t24, const fom_float *restrict t_t26, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 20; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t3_t_t1_VALS[i] * t_t24[FOM_MAT_t3_t_t1_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 30 * i; p < 30 * i + 30; ++p) { fom_int c = p; s += FOM_MAT_t27_t_t23_VALS[p] * t_t23[c] + FOM_MAT_t27_t_t26_VALS[p] * t_t26[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t28(const fom_float *restrict t_t23, const fom_float *restrict t_t25, const fom_float *restrict t_t26, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 30; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t7_t_t4_VALS[i] * t_t25[FOM_MAT_t7_t_t4_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 20 * i; p < 20 * i + 20; ++p) { fom_int c = 30 * (p - 20 * i) + i; s += FOM_MAT_t28_t_t23_VALS[p] * t_t23[c] + FOM_MAT_t28_t_t26_VALS[p] * t_t26[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t29(const fom_float *restrict t_t26, const fom_float *restrict t_t27, const fom_float *restrict t_t28, fom_float *restrict out) {
+    fom_int i, p;
+    for (p = 0; p < 20; ++p) {
+        const fom_float y = t_t27[p];
+        for (i = 30 * p; i < 30 * p + 30; ++i)
+            out[i] += FOM_MAT_t5_t_t2_VALS[i] * t_t26[i] + FOM_MAT_t2_t_t1_VALS[i] * y + FOM_MAT_t5_t_t4_VALS[i] * t_t28[i - 30 * p];
+    }
+}
+
+void fom_fused_t30(const fom_float *restrict t_t26, const fom_float *restrict t_t27, const fom_float *restrict t_t29, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 20; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t3_t_t1_VALS[i] * t_t27[FOM_MAT_t3_t_t1_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 30 * i; p < 30 * i + 30; ++p) { fom_int c = p; s += FOM_MAT_t30_t_t26_VALS[p] * t_t26[c] + FOM_MAT_t30_t_t29_VALS[p] * t_t29[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t31(const fom_float *restrict t_t26, const fom_float *restrict t_t28, const fom_float *restrict t_t29, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 30; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t7_t_t4_VALS[i] * t_t28[FOM_MAT_t7_t_t4_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 20 * i; p < 20 * i + 20; ++p) { fom_int c = 30 * (p - 20 * i) + i; s += FOM_MAT_t31_t_t26_VALS[p] * t_t26[c] + FOM_MAT_t31_t_t29_VALS[p] * t_t29[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t32(const fom_float *restrict t_t29, const fom_float *restrict t_t30, const fom_float *restrict t_t31, fom_float *restrict out) {
+    fom_int i, p;
+    for (p = 0; p < 20; ++p) {
+        const fom_float y = t_t30[p];
+        for (i = 30 * p; i < 30 * p + 30; ++i)
+            out[i] += FOM_MAT_t5_t_t2_VALS[i] * t_t29[i] + FOM_MAT_t2_t_t1_VALS[i] * y + FOM_MAT_t5_t_t4_VALS[i] * t_t31[i - 30 * p];
+    }
+}
+
+void fom_fused_t33(const fom_float *restrict t_t29, const fom_float *restrict t_t30, const fom_float *restrict t_t32, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 20; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t3_t_t1_VALS[i] * t_t30[FOM_MAT_t3_t_t1_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 30 * i; p < 30 * i + 30; ++p) { fom_int c = p; s += FOM_MAT_t33_t_t29_VALS[p] * t_t29[c] + FOM_MAT_t33_t_t32_VALS[p] * t_t32[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t34(const fom_float *restrict t_t29, const fom_float *restrict t_t31, const fom_float *restrict t_t32, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 30; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t7_t_t4_VALS[i] * t_t31[FOM_MAT_t7_t_t4_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 20 * i; p < 20 * i + 20; ++p) { fom_int c = 30 * (p - 20 * i) + i; s += FOM_MAT_t34_t_t29_VALS[p] * t_t29[c] + FOM_MAT_t34_t_t32_VALS[p] * t_t32[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t35(const fom_float *restrict t_t32, const fom_float *restrict t_t33, const fom_float *restrict t_t34, fom_float *restrict out) {
+    fom_int i, p;
+    for (p = 0; p < 20; ++p) {
+        const fom_float y = t_t33[p];
+        for (i = 30 * p; i < 30 * p + 30; ++i)
+            out[i] += FOM_MAT_t5_t_t2_VALS[i] * t_t32[i] + FOM_MAT_t2_t_t1_VALS[i] * y + FOM_MAT_t5_t_t4_VALS[i] * t_t34[i - 30 * p];
+    }
+}
+
+void fom_fused_t36(const fom_float *restrict t_t32, const fom_float *restrict t_t33, const fom_float *restrict t_t35, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 20; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t3_t_t1_VALS[i] * t_t33[FOM_MAT_t3_t_t1_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 30 * i; p < 30 * i + 30; ++p) { fom_int c = p; s += FOM_MAT_t36_t_t32_VALS[p] * t_t32[c] + FOM_MAT_t36_t_t35_VALS[p] * t_t35[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t37(const fom_float *restrict t_t32, const fom_float *restrict t_t34, const fom_float *restrict t_t35, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 30; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t7_t_t4_VALS[i] * t_t34[FOM_MAT_t7_t_t4_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 20 * i; p < 20 * i + 20; ++p) { fom_int c = 30 * (p - 20 * i) + i; s += FOM_MAT_t37_t_t32_VALS[p] * t_t32[c] + FOM_MAT_t37_t_t35_VALS[p] * t_t35[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t38(const fom_float *restrict t_t35, const fom_float *restrict t_t36, const fom_float *restrict t_t37, fom_float *restrict out) {
+    fom_int i, p;
+    for (p = 0; p < 20; ++p) {
+        const fom_float y = t_t36[p];
+        for (i = 30 * p; i < 30 * p + 30; ++i)
+            out[i] += FOM_MAT_t5_t_t2_VALS[i] * t_t35[i] + FOM_MAT_t2_t_t1_VALS[i] * y + FOM_MAT_t5_t_t4_VALS[i] * t_t37[i - 30 * p];
+    }
+}
+
+void fom_fused_t39(const fom_float *restrict t_t35, const fom_float *restrict t_t36, const fom_float *restrict t_t38, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 20; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t3_t_t1_VALS[i] * t_t36[FOM_MAT_t3_t_t1_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 30 * i; p < 30 * i + 30; ++p) { fom_int c = p; s += FOM_MAT_t39_t_t35_VALS[p] * t_t35[c] + FOM_MAT_t39_t_t38_VALS[p] * t_t38[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t40(const fom_float *restrict t_t35, const fom_float *restrict t_t37, const fom_float *restrict t_t38, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 30; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t7_t_t4_VALS[i] * t_t37[FOM_MAT_t7_t_t4_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 20 * i; p < 20 * i + 20; ++p) { fom_int c = 30 * (p - 20 * i) + i; s += FOM_MAT_t40_t_t35_VALS[p] * t_t35[c] + FOM_MAT_t40_t_t38_VALS[p] * t_t38[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t41(const fom_float *restrict t_t38, const fom_float *restrict t_t39, const fom_float *restrict t_t40, fom_float *restrict out) {
+    fom_int i, p;
+    for (p = 0; p < 20; ++p) {
+        const fom_float y = t_t39[p];
+        for (i = 30 * p; i < 30 * p + 30; ++i)
+            out[i] += FOM_MAT_t5_t_t2_VALS[i] * t_t38[i] + FOM_MAT_t2_t_t1_VALS[i] * y + FOM_MAT_t5_t_t4_VALS[i] * t_t40[i - 30 * p];
+    }
+}
+
+void fom_fused_t42(const fom_float *restrict t_t38, const fom_float *restrict t_t39, const fom_float *restrict t_t41, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 20; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t3_t_t1_VALS[i] * t_t39[FOM_MAT_t3_t_t1_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 30 * i; p < 30 * i + 30; ++p) { fom_int c = p; s += FOM_MAT_t42_t_t38_VALS[p] * t_t38[c] + FOM_MAT_t42_t_t41_VALS[p] * t_t41[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t43(const fom_float *restrict t_t38, const fom_float *restrict t_t40, const fom_float *restrict t_t41, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 30; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t7_t_t4_VALS[i] * t_t40[FOM_MAT_t7_t_t4_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 20 * i; p < 20 * i + 20; ++p) { fom_int c = 30 * (p - 20 * i) + i; s += FOM_MAT_t43_t_t38_VALS[p] * t_t38[c] + FOM_MAT_t43_t_t41_VALS[p] * t_t41[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t44(const fom_float *restrict t_t41, const fom_float *restrict t_t42, const fom_float *restrict t_t43, fom_float *restrict out) {
+    fom_int i, p;
+    for (p = 0; p < 20; ++p) {
+        const fom_float y = t_t42[p];
+        for (i = 30 * p; i < 30 * p + 30; ++i)
+            out[i] += FOM_MAT_t5_t_t2_VALS[i] * t_t41[i] + FOM_MAT_t2_t_t1_VALS[i] * y + FOM_MAT_t5_t_t4_VALS[i] * t_t43[i - 30 * p];
+    }
+}
+
+void fom_fused_t45(const fom_float *restrict t_t41, const fom_float *restrict t_t42, const fom_float *restrict t_t44, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 20; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t3_t_t1_VALS[i] * t_t42[FOM_MAT_t3_t_t1_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 30 * i; p < 30 * i + 30; ++p) { fom_int c = p; s += FOM_MAT_t45_t_t41_VALS[p] * t_t41[c] + FOM_MAT_t45_t_t44_VALS[p] * t_t44[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t46(const fom_float *restrict t_t41, const fom_float *restrict t_t43, const fom_float *restrict t_t44, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 30; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t7_t_t4_VALS[i] * t_t43[FOM_MAT_t7_t_t4_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 20 * i; p < 20 * i + 20; ++p) { fom_int c = 30 * (p - 20 * i) + i; s += FOM_MAT_t46_t_t41_VALS[p] * t_t41[c] + FOM_MAT_t46_t_t44_VALS[p] * t_t44[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t47(const fom_float *restrict t_t44, const fom_float *restrict t_t45, const fom_float *restrict t_t46, fom_float *restrict out) {
+    fom_int i, p;
+    for (p = 0; p < 20; ++p) {
+        const fom_float y = t_t45[p];
+        for (i = 30 * p; i < 30 * p + 30; ++i)
+            out[i] += FOM_MAT_t5_t_t2_VALS[i] * t_t44[i] + FOM_MAT_t2_t_t1_VALS[i] * y + FOM_MAT_t5_t_t4_VALS[i] * t_t46[i - 30 * p];
+    }
+}
+
+void fom_fused_t48(const fom_float *restrict t_t44, const fom_float *restrict t_t45, const fom_float *restrict t_t47, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 20; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t3_t_t1_VALS[i] * t_t45[FOM_MAT_t3_t_t1_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 30 * i; p < 30 * i + 30; ++p) { fom_int c = p; s += FOM_MAT_t48_t_t44_VALS[p] * t_t44[c] + FOM_MAT_t48_t_t47_VALS[p] * t_t47[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t49(const fom_float *restrict t_t44, const fom_float *restrict t_t46, const fom_float *restrict t_t47, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 30; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t7_t_t4_VALS[i] * t_t46[FOM_MAT_t7_t_t4_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 20 * i; p < 20 * i + 20; ++p) { fom_int c = 30 * (p - 20 * i) + i; s += FOM_MAT_t49_t_t44_VALS[p] * t_t44[c] + FOM_MAT_t49_t_t47_VALS[p] * t_t47[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t50(const fom_float *restrict t_t47, const fom_float *restrict t_t48, const fom_float *restrict t_t49, fom_float *restrict out) {
+    fom_int i, p;
+    for (p = 0; p < 20; ++p) {
+        const fom_float y = t_t48[p];
+        for (i = 30 * p; i < 30 * p + 30; ++i)
+            out[i] += FOM_MAT_t5_t_t2_VALS[i] * t_t47[i] + FOM_MAT_t2_t_t1_VALS[i] * y + FOM_MAT_t5_t_t4_VALS[i] * t_t49[i - 30 * p];
+    }
+}
+
+void fom_fused_t51(const fom_float *restrict t_t47, const fom_float *restrict t_t48, const fom_float *restrict t_t50, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 20; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t3_t_t1_VALS[i] * t_t48[FOM_MAT_t3_t_t1_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 30 * i; p < 30 * i + 30; ++p) { fom_int c = p; s += FOM_MAT_t51_t_t47_VALS[p] * t_t47[c] + FOM_MAT_t51_t_t50_VALS[p] * t_t50[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t52(const fom_float *restrict t_t47, const fom_float *restrict t_t49, const fom_float *restrict t_t50, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 30; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t7_t_t4_VALS[i] * t_t49[FOM_MAT_t7_t_t4_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 20 * i; p < 20 * i + 20; ++p) { fom_int c = 30 * (p - 20 * i) + i; s += FOM_MAT_t52_t_t47_VALS[p] * t_t47[c] + FOM_MAT_t52_t_t50_VALS[p] * t_t50[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t53(const fom_float *restrict t_t50, const fom_float *restrict t_t51, const fom_float *restrict t_t52, fom_float *restrict out) {
+    fom_int i, p;
+    for (p = 0; p < 20; ++p) {
+        const fom_float y = t_t51[p];
+        for (i = 30 * p; i < 30 * p + 30; ++i)
+            out[i] += FOM_MAT_t5_t_t2_VALS[i] * t_t50[i] + FOM_MAT_t2_t_t1_VALS[i] * y + FOM_MAT_t5_t_t4_VALS[i] * t_t52[i - 30 * p];
+    }
+}
+
+void fom_fused_t54(const fom_float *restrict t_t50, const fom_float *restrict t_t51, const fom_float *restrict t_t53, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 20; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t3_t_t1_VALS[i] * t_t51[FOM_MAT_t3_t_t1_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 30 * i; p < 30 * i + 30; ++p) { fom_int c = p; s += FOM_MAT_t54_t_t50_VALS[p] * t_t50[c] + FOM_MAT_t54_t_t53_VALS[p] * t_t53[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t55(const fom_float *restrict t_t50, const fom_float *restrict t_t52, const fom_float *restrict t_t53, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 30; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t7_t_t4_VALS[i] * t_t52[FOM_MAT_t7_t_t4_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 20 * i; p < 20 * i + 20; ++p) { fom_int c = 30 * (p - 20 * i) + i; s += FOM_MAT_t55_t_t50_VALS[p] * t_t50[c] + FOM_MAT_t55_t_t53_VALS[p] * t_t53[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t56(const fom_float *restrict t_t53, const fom_float *restrict t_t54, const fom_float *restrict t_t55, fom_float *restrict out) {
+    fom_int i, p;
+    for (p = 0; p < 20; ++p) {
+        const fom_float y = t_t54[p];
+        for (i = 30 * p; i < 30 * p + 30; ++i)
+            out[i] += FOM_MAT_t5_t_t2_VALS[i] * t_t53[i] + FOM_MAT_t2_t_t1_VALS[i] * y + FOM_MAT_t5_t_t4_VALS[i] * t_t55[i - 30 * p];
+    }
+}
+
+void fom_fused_t57(const fom_float *restrict t_t53, const fom_float *restrict t_t54, const fom_float *restrict t_t56, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 20; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t3_t_t1_VALS[i] * t_t54[FOM_MAT_t3_t_t1_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 30 * i; p < 30 * i + 30; ++p) { fom_int c = p; s += FOM_MAT_t57_t_t53_VALS[p] * t_t53[c] + FOM_MAT_t57_t_t56_VALS[p] * t_t56[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t58(const fom_float *restrict t_t53, const fom_float *restrict t_t55, const fom_float *restrict t_t56, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 30; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t7_t_t4_VALS[i] * t_t55[FOM_MAT_t7_t_t4_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 20 * i; p < 20 * i + 20; ++p) { fom_int c = 30 * (p - 20 * i) + i; s += FOM_MAT_t58_t_t53_VALS[p] * t_t53[c] + FOM_MAT_t58_t_t56_VALS[p] * t_t56[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t59(const fom_float *restrict t_t56, const fom_float *restrict t_t57, const fom_float *restrict t_t58, fom_float *restrict out) {
+    fom_int i, p;
+    for (p = 0; p < 20; ++p) {
+        const fom_float y = t_t57[p];
+        for (i = 30 * p; i < 30 * p + 30; ++i)
+            out[i] += FOM_MAT_t5_t_t2_VALS[i] * t_t56[i] + FOM_MAT_t2_t_t1_VALS[i] * y + FOM_MAT_t5_t_t4_VALS[i] * t_t58[i - 30 * p];
+    }
+}
+
+void fom_fused_t60(const fom_float *restrict t_t56, const fom_float *restrict t_t57, const fom_float *restrict t_t59, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 20; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t3_t_t1_VALS[i] * t_t57[FOM_MAT_t3_t_t1_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 30 * i; p < 30 * i + 30; ++p) { fom_int c = p; s += FOM_MAT_t60_t_t56_VALS[p] * t_t56[c] + FOM_MAT_t60_t_t59_VALS[p] * t_t59[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t61(const fom_float *restrict t_t56, const fom_float *restrict t_t58, const fom_float *restrict t_t59, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 30; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t7_t_t4_VALS[i] * t_t58[FOM_MAT_t7_t_t4_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 20 * i; p < 20 * i + 20; ++p) { fom_int c = 30 * (p - 20 * i) + i; s += FOM_MAT_t61_t_t56_VALS[p] * t_t56[c] + FOM_MAT_t61_t_t59_VALS[p] * t_t59[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t62(const fom_float *restrict t_t59, const fom_float *restrict t_t60, const fom_float *restrict t_t61, fom_float *restrict out) {
+    fom_int i, p;
+    for (p = 0; p < 20; ++p) {
+        const fom_float y = t_t60[p];
+        for (i = 30 * p; i < 30 * p + 30; ++i)
+            out[i] += FOM_MAT_t5_t_t2_VALS[i] * t_t59[i] + FOM_MAT_t2_t_t1_VALS[i] * y + FOM_MAT_t5_t_t4_VALS[i] * t_t61[i - 30 * p];
+    }
+}
+
+void fom_fused_t63(const fom_float *restrict t_t59, const fom_float *restrict t_t60, const fom_float *restrict t_t62, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 20; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t3_t_t1_VALS[i] * t_t60[FOM_MAT_t3_t_t1_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 30 * i; p < 30 * i + 30; ++p) { fom_int c = p; s += FOM_MAT_t63_t_t59_VALS[p] * t_t59[c] + FOM_MAT_t63_t_t62_VALS[p] * t_t62[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t64(const fom_float *restrict t_t59, const fom_float *restrict t_t61, const fom_float *restrict t_t62, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 30; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t7_t_t4_VALS[i] * t_t61[FOM_MAT_t7_t_t4_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 20 * i; p < 20 * i + 20; ++p) { fom_int c = 30 * (p - 20 * i) + i; s += FOM_MAT_t64_t_t59_VALS[p] * t_t59[c] + FOM_MAT_t64_t_t62_VALS[p] * t_t62[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t65(const fom_float *restrict t_t62, const fom_float *restrict t_t63, const fom_float *restrict t_t64, fom_float *restrict out) {
+    fom_int i, p;
+    for (p = 0; p < 20; ++p) {
+        const fom_float y = t_t63[p];
+        for (i = 30 * p; i < 30 * p + 30; ++i)
+            out[i] += FOM_MAT_t5_t_t2_VALS[i] * t_t62[i] + FOM_MAT_t2_t_t1_VALS[i] * y + FOM_MAT_t5_t_t4_VALS[i] * t_t64[i - 30 * p];
+    }
+}
+
+void fom_fused_t66(const fom_float *restrict t_t62, const fom_float *restrict t_t63, const fom_float *restrict t_t65, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 20; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t3_t_t1_VALS[i] * t_t63[FOM_MAT_t3_t_t1_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 30 * i; p < 30 * i + 30; ++p) { fom_int c = p; s += FOM_MAT_t66_t_t62_VALS[p] * t_t62[c] + FOM_MAT_t66_t_t65_VALS[p] * t_t65[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t67(const fom_float *restrict t_t62, const fom_float *restrict t_t64, const fom_float *restrict t_t65, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 30; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t7_t_t4_VALS[i] * t_t64[FOM_MAT_t7_t_t4_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 20 * i; p < 20 * i + 20; ++p) { fom_int c = 30 * (p - 20 * i) + i; s += FOM_MAT_t67_t_t62_VALS[p] * t_t62[c] + FOM_MAT_t67_t_t65_VALS[p] * t_t65[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t68(const fom_float *restrict t_t65, const fom_float *restrict t_t66, const fom_float *restrict t_t67, fom_float *restrict out) {
+    fom_int i, p;
+    for (p = 0; p < 20; ++p) {
+        const fom_float y = t_t66[p];
+        for (i = 30 * p; i < 30 * p + 30; ++i)
+            out[i] += FOM_MAT_t5_t_t2_VALS[i] * t_t65[i] + FOM_MAT_t2_t_t1_VALS[i] * y + FOM_MAT_t5_t_t4_VALS[i] * t_t67[i - 30 * p];
+    }
+}
+
+void fom_fused_t69(const fom_float *restrict t_t65, const fom_float *restrict t_t66, const fom_float *restrict t_t68, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 20; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t3_t_t1_VALS[i] * t_t66[FOM_MAT_t3_t_t1_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 30 * i; p < 30 * i + 30; ++p) { fom_int c = p; s += FOM_MAT_t69_t_t65_VALS[p] * t_t65[c] + FOM_MAT_t69_t_t68_VALS[p] * t_t68[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t70(const fom_float *restrict t_t65, const fom_float *restrict t_t67, const fom_float *restrict t_t68, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 30; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t7_t_t4_VALS[i] * t_t67[FOM_MAT_t7_t_t4_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 20 * i; p < 20 * i + 20; ++p) { fom_int c = 30 * (p - 20 * i) + i; s += FOM_MAT_t70_t_t65_VALS[p] * t_t65[c] + FOM_MAT_t70_t_t68_VALS[p] * t_t68[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t71(const fom_float *restrict t_t68, const fom_float *restrict t_t69, const fom_float *restrict t_t70, fom_float *restrict out) {
+    fom_int i, p;
+    for (p = 0; p < 20; ++p) {
+        const fom_float y = t_t69[p];
+        for (i = 30 * p; i < 30 * p + 30; ++i)
+            out[i] += FOM_MAT_t5_t_t2_VALS[i] * t_t68[i] + FOM_MAT_t2_t_t1_VALS[i] * y + FOM_MAT_t5_t_t4_VALS[i] * t_t70[i - 30 * p];
+    }
+}
+
+void fom_fused_t72(const fom_float *restrict t_t68, const fom_float *restrict t_t69, const fom_float *restrict t_t71, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 20; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t3_t_t1_VALS[i] * t_t69[FOM_MAT_t3_t_t1_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 30 * i; p < 30 * i + 30; ++p) { fom_int c = p; s += FOM_MAT_t72_t_t68_VALS[p] * t_t68[c] + FOM_MAT_t72_t_t71_VALS[p] * t_t71[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t73(const fom_float *restrict t_t68, const fom_float *restrict t_t70, const fom_float *restrict t_t71, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 30; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t7_t_t4_VALS[i] * t_t70[FOM_MAT_t7_t_t4_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 20 * i; p < 20 * i + 20; ++p) { fom_int c = 30 * (p - 20 * i) + i; s += FOM_MAT_t73_t_t68_VALS[p] * t_t68[c] + FOM_MAT_t73_t_t71_VALS[p] * t_t71[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t74(const fom_float *restrict t_t71, const fom_float *restrict t_t72, const fom_float *restrict t_t73, fom_float *restrict out) {
+    fom_int i, p;
+    for (p = 0; p < 20; ++p) {
+        const fom_float y = t_t72[p];
+        for (i = 30 * p; i < 30 * p + 30; ++i)
+            out[i] += FOM_MAT_t5_t_t2_VALS[i] * t_t71[i] + FOM_MAT_t2_t_t1_VALS[i] * y + FOM_MAT_t5_t_t4_VALS[i] * t_t73[i - 30 * p];
+    }
+}
+
+void fom_fused_t75(const fom_float *restrict t_t71, const fom_float *restrict t_t72, const fom_float *restrict t_t74, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 20; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t3_t_t1_VALS[i] * t_t72[FOM_MAT_t3_t_t1_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 30 * i; p < 30 * i + 30; ++p) { fom_int c = p; s += FOM_MAT_t75_t_t71_VALS[p] * t_t71[c] + FOM_MAT_t75_t_t74_VALS[p] * t_t74[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t76(const fom_float *restrict t_t71, const fom_float *restrict t_t73, const fom_float *restrict t_t74, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 30; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t7_t_t4_VALS[i] * t_t73[FOM_MAT_t7_t_t4_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 20 * i; p < 20 * i + 20; ++p) { fom_int c = 30 * (p - 20 * i) + i; s += FOM_MAT_t76_t_t71_VALS[p] * t_t71[c] + FOM_MAT_t76_t_t74_VALS[p] * t_t74[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t77(const fom_float *restrict t_t74, const fom_float *restrict t_t75, const fom_float *restrict t_t76, fom_float *restrict out) {
+    fom_int i, p;
+    for (p = 0; p < 20; ++p) {
+        const fom_float y = t_t75[p];
+        for (i = 30 * p; i < 30 * p + 30; ++i)
+            out[i] += FOM_MAT_t5_t_t2_VALS[i] * t_t74[i] + FOM_MAT_t2_t_t1_VALS[i] * y + FOM_MAT_t5_t_t4_VALS[i] * t_t76[i - 30 * p];
+    }
+}
+
+void fom_fused_t78(const fom_float *restrict t_t74, const fom_float *restrict t_t75, const fom_float *restrict t_t77, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 20; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t3_t_t1_VALS[i] * t_t75[FOM_MAT_t3_t_t1_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 30 * i; p < 30 * i + 30; ++p) { fom_int c = p; s += FOM_MAT_t78_t_t74_VALS[p] * t_t74[c] + FOM_MAT_t78_t_t77_VALS[p] * t_t77[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t79(const fom_float *restrict t_t74, const fom_float *restrict t_t76, const fom_float *restrict t_t77, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 30; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t7_t_t4_VALS[i] * t_t76[FOM_MAT_t7_t_t4_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 20 * i; p < 20 * i + 20; ++p) { fom_int c = 30 * (p - 20 * i) + i; s += FOM_MAT_t79_t_t74_VALS[p] * t_t74[c] + FOM_MAT_t79_t_t77_VALS[p] * t_t77[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t80(const fom_float *restrict t_t77, const fom_float *restrict t_t78, const fom_float *restrict t_t79, fom_float *restrict out) {
+    fom_int i, p;
+    for (p = 0; p < 20; ++p) {
+        const fom_float y = t_t78[p];
+        for (i = 30 * p; i < 30 * p + 30; ++i)
+            out[i] += FOM_MAT_t5_t_t2_VALS[i] * t_t77[i] + FOM_MAT_t2_t_t1_VALS[i] * y + FOM_MAT_t5_t_t4_VALS[i] * t_t79[i - 30 * p];
+    }
+}
+
+void fom_fused_t81(const fom_float *restrict t_t77, const fom_float *restrict t_t78, const fom_float *restrict t_t80, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 20; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t3_t_t1_VALS[i] * t_t78[FOM_MAT_t3_t_t1_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 30 * i; p < 30 * i + 30; ++p) { fom_int c = p; s += FOM_MAT_t81_t_t77_VALS[p] * t_t77[c] + FOM_MAT_t81_t_t80_VALS[p] * t_t80[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t82(const fom_float *restrict t_t77, const fom_float *restrict t_t79, const fom_float *restrict t_t80, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 30; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t7_t_t4_VALS[i] * t_t79[FOM_MAT_t7_t_t4_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 20 * i; p < 20 * i + 20; ++p) { fom_int c = 30 * (p - 20 * i) + i; s += FOM_MAT_t82_t_t77_VALS[p] * t_t77[c] + FOM_MAT_t82_t_t80_VALS[p] * t_t80[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t83(const fom_float *restrict t_t80, const fom_float *restrict t_t81, const fom_float *restrict t_t82, fom_float *restrict out) {
+    fom_int i, p;
+    for (p = 0; p < 20; ++p) {
+        const fom_float y = t_t81[p];
+        for (i = 30 * p; i < 30 * p + 30; ++i)
+            out[i] += FOM_MAT_t5_t_t2_VALS[i] * t_t80[i] + FOM_MAT_t2_t_t1_VALS[i] * y + FOM_MAT_t5_t_t4_VALS[i] * t_t82[i - 30 * p];
+    }
+}
+
+void fom_fused_t84(const fom_float *restrict t_t80, const fom_float *restrict t_t81, const fom_float *restrict t_t83, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 20; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t3_t_t1_VALS[i] * t_t81[FOM_MAT_t3_t_t1_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 30 * i; p < 30 * i + 30; ++p) { fom_int c = p; s += FOM_MAT_t84_t_t80_VALS[p] * t_t80[c] + FOM_MAT_t84_t_t83_VALS[p] * t_t83[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t85(const fom_float *restrict t_t80, const fom_float *restrict t_t82, const fom_float *restrict t_t83, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 30; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t7_t_t4_VALS[i] * t_t82[FOM_MAT_t7_t_t4_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 20 * i; p < 20 * i + 20; ++p) { fom_int c = 30 * (p - 20 * i) + i; s += FOM_MAT_t85_t_t80_VALS[p] * t_t80[c] + FOM_MAT_t85_t_t83_VALS[p] * t_t83[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t86(const fom_float *restrict t_t83, const fom_float *restrict t_t84, const fom_float *restrict t_t85, fom_float *restrict out) {
+    fom_int i, p;
+    for (p = 0; p < 20; ++p) {
+        const fom_float y = t_t84[p];
+        for (i = 30 * p; i < 30 * p + 30; ++i)
+            out[i] += FOM_MAT_t5_t_t2_VALS[i] * t_t83[i] + FOM_MAT_t2_t_t1_VALS[i] * y + FOM_MAT_t5_t_t4_VALS[i] * t_t85[i - 30 * p];
+    }
+}
+
+void fom_fused_t87(const fom_float *restrict t_t83, const fom_float *restrict t_t84, const fom_float *restrict t_t86, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 20; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t3_t_t1_VALS[i] * t_t84[FOM_MAT_t3_t_t1_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 30 * i; p < 30 * i + 30; ++p) { fom_int c = p; s += FOM_MAT_t87_t_t83_VALS[p] * t_t83[c] + FOM_MAT_t87_t_t86_VALS[p] * t_t86[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t88(const fom_float *restrict t_t83, const fom_float *restrict t_t85, const fom_float *restrict t_t86, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 30; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t7_t_t4_VALS[i] * t_t85[FOM_MAT_t7_t_t4_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 20 * i; p < 20 * i + 20; ++p) { fom_int c = 30 * (p - 20 * i) + i; s += FOM_MAT_t88_t_t83_VALS[p] * t_t83[c] + FOM_MAT_t88_t_t86_VALS[p] * t_t86[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t89(const fom_float *restrict t_t86, const fom_float *restrict t_t87, const fom_float *restrict t_t88, fom_float *restrict out) {
+    fom_int i, p;
+    for (p = 0; p < 20; ++p) {
+        const fom_float y = t_t87[p];
+        for (i = 30 * p; i < 30 * p + 30; ++i)
+            out[i] += FOM_MAT_t5_t_t2_VALS[i] * t_t86[i] + FOM_MAT_t2_t_t1_VALS[i] * y + FOM_MAT_t5_t_t4_VALS[i] * t_t88[i - 30 * p];
+    }
+}
+
+void fom_fused_t90(const fom_float *restrict t_t86, const fom_float *restrict t_t87, const fom_float *restrict t_t89, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 20; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t3_t_t1_VALS[i] * t_t87[FOM_MAT_t3_t_t1_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 30 * i; p < 30 * i + 30; ++p) { fom_int c = p; s += FOM_MAT_t90_t_t86_VALS[p] * t_t86[c] + FOM_MAT_t90_t_t89_VALS[p] * t_t89[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t91(const fom_float *restrict t_t86, const fom_float *restrict t_t88, const fom_float *restrict t_t89, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 30; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t7_t_t4_VALS[i] * t_t88[FOM_MAT_t7_t_t4_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 20 * i; p < 20 * i + 20; ++p) { fom_int c = 30 * (p - 20 * i) + i; s += FOM_MAT_t91_t_t86_VALS[p] * t_t86[c] + FOM_MAT_t91_t_t89_VALS[p] * t_t89[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t92(const fom_float *restrict t_t89, const fom_float *restrict t_t90, const fom_float *restrict t_t91, fom_float *restrict out) {
+    fom_int i, p;
+    for (p = 0; p < 20; ++p) {
+        const fom_float y = t_t90[p];
+        for (i = 30 * p; i < 30 * p + 30; ++i)
+            out[i] += FOM_MAT_t5_t_t2_VALS[i] * t_t89[i] + FOM_MAT_t2_t_t1_VALS[i] * y + FOM_MAT_t5_t_t4_VALS[i] * t_t91[i - 30 * p];
+    }
+}
+
+void fom_fused_t93(const fom_float *restrict t_t89, const fom_float *restrict t_t90, const fom_float *restrict t_t92, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 20; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t3_t_t1_VALS[i] * t_t90[FOM_MAT_t3_t_t1_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 30 * i; p < 30 * i + 30; ++p) { fom_int c = p; s += FOM_MAT_t93_t_t89_VALS[p] * t_t89[c] + FOM_MAT_t93_t_t92_VALS[p] * t_t92[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t94(const fom_float *restrict t_t89, const fom_float *restrict t_t91, const fom_float *restrict t_t92, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 30; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t7_t_t4_VALS[i] * t_t91[FOM_MAT_t7_t_t4_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 20 * i; p < 20 * i + 20; ++p) { fom_int c = 30 * (p - 20 * i) + i; s += FOM_MAT_t94_t_t89_VALS[p] * t_t89[c] + FOM_MAT_t94_t_t92_VALS[p] * t_t92[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t95(const fom_float *restrict t_t92, const fom_float *restrict t_t93, const fom_float *restrict t_t94, fom_float *restrict out) {
+    fom_int i, p;
+    for (p = 0; p < 20; ++p) {
+        const fom_float y = t_t93[p];
+        for (i = 30 * p; i < 30 * p + 30; ++i)
+            out[i] += FOM_MAT_t5_t_t2_VALS[i] * t_t92[i] + FOM_MAT_t2_t_t1_VALS[i] * y + FOM_MAT_t5_t_t4_VALS[i] * t_t94[i - 30 * p];
+    }
+}
+
+void fom_fused_t96(const fom_float *restrict t_t92, const fom_float *restrict t_t93, const fom_float *restrict t_t95, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 20; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t3_t_t1_VALS[i] * t_t93[FOM_MAT_t3_t_t1_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 30 * i; p < 30 * i + 30; ++p) { fom_int c = p; s += FOM_MAT_t96_t_t92_VALS[p] * t_t92[c] + FOM_MAT_t96_t_t95_VALS[p] * t_t95[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t97(const fom_float *restrict t_t92, const fom_float *restrict t_t94, const fom_float *restrict t_t95, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 30; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t7_t_t4_VALS[i] * t_t94[FOM_MAT_t7_t_t4_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 20 * i; p < 20 * i + 20; ++p) { fom_int c = 30 * (p - 20 * i) + i; s += FOM_MAT_t97_t_t92_VALS[p] * t_t92[c] + FOM_MAT_t97_t_t95_VALS[p] * t_t95[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t98(const fom_float *restrict t_t95, const fom_float *restrict t_t96, const fom_float *restrict t_t97, fom_float *restrict out) {
+    fom_int i, p;
+    for (p = 0; p < 20; ++p) {
+        const fom_float y = t_t96[p];
+        for (i = 30 * p; i < 30 * p + 30; ++i)
+            out[i] += FOM_MAT_t5_t_t2_VALS[i] * t_t95[i] + FOM_MAT_t2_t_t1_VALS[i] * y + FOM_MAT_t5_t_t4_VALS[i] * t_t97[i - 30 * p];
+    }
+}
+
+void fom_fused_t99(const fom_float *restrict t_t95, const fom_float *restrict t_t96, const fom_float *restrict t_t98, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 20; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t3_t_t1_VALS[i] * t_t96[FOM_MAT_t3_t_t1_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 30 * i; p < 30 * i + 30; ++p) { fom_int c = p; s += FOM_MAT_t99_t_t95_VALS[p] * t_t95[c] + FOM_MAT_t99_t_t98_VALS[p] * t_t98[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t100(const fom_float *restrict t_t95, const fom_float *restrict t_t97, const fom_float *restrict t_t98, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 30; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t7_t_t4_VALS[i] * t_t97[FOM_MAT_t7_t_t4_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 20 * i; p < 20 * i + 20; ++p) { fom_int c = 30 * (p - 20 * i) + i; s += FOM_MAT_t100_t_t95_VALS[p] * t_t95[c] + FOM_MAT_t100_t_t98_VALS[p] * t_t98[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t101(const fom_float *restrict t_t98, const fom_float *restrict t_t99, const fom_float *restrict t_t100, fom_float *restrict out) {
+    fom_int i, p;
+    for (p = 0; p < 20; ++p) {
+        const fom_float y = t_t99[p];
+        for (i = 30 * p; i < 30 * p + 30; ++i)
+            out[i] += FOM_MAT_t5_t_t2_VALS[i] * t_t98[i] + FOM_MAT_t2_t_t1_VALS[i] * y + FOM_MAT_t5_t_t4_VALS[i] * t_t100[i - 30 * p];
+    }
+}
+
+void fom_fused_t102(const fom_float *restrict t_t98, const fom_float *restrict t_t99, const fom_float *restrict t_t101, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 20; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t3_t_t1_VALS[i] * t_t99[FOM_MAT_t3_t_t1_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 30 * i; p < 30 * i + 30; ++p) { fom_int c = p; s += FOM_MAT_t102_t_t98_VALS[p] * t_t98[c] + FOM_MAT_t102_t_t101_VALS[p] * t_t101[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t103(const fom_float *restrict t_t98, const fom_float *restrict t_t100, const fom_float *restrict t_t101, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 30; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t7_t_t4_VALS[i] * t_t100[FOM_MAT_t7_t_t4_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 20 * i; p < 20 * i + 20; ++p) { fom_int c = 30 * (p - 20 * i) + i; s += FOM_MAT_t103_t_t98_VALS[p] * t_t98[c] + FOM_MAT_t103_t_t101_VALS[p] * t_t101[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t104(const fom_float *restrict t_t101, const fom_float *restrict t_t102, const fom_float *restrict t_t103, fom_float *restrict out) {
+    fom_int i, p;
+    for (p = 0; p < 20; ++p) {
+        const fom_float y = t_t102[p];
+        for (i = 30 * p; i < 30 * p + 30; ++i)
+            out[i] += FOM_MAT_t5_t_t2_VALS[i] * t_t101[i] + FOM_MAT_t2_t_t1_VALS[i] * y + FOM_MAT_t5_t_t4_VALS[i] * t_t103[i - 30 * p];
+    }
+}
+
+void fom_fused_t105(const fom_float *restrict t_t101, const fom_float *restrict t_t102, const fom_float *restrict t_t104, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 20; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t3_t_t1_VALS[i] * t_t102[FOM_MAT_t3_t_t1_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 30 * i; p < 30 * i + 30; ++p) { fom_int c = p; s += FOM_MAT_t105_t_t101_VALS[p] * t_t101[c] + FOM_MAT_t105_t_t104_VALS[p] * t_t104[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t106(const fom_float *restrict t_t101, const fom_float *restrict t_t103, const fom_float *restrict t_t104, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 30; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t7_t_t4_VALS[i] * t_t103[FOM_MAT_t7_t_t4_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 20 * i; p < 20 * i + 20; ++p) { fom_int c = 30 * (p - 20 * i) + i; s += FOM_MAT_t106_t_t101_VALS[p] * t_t101[c] + FOM_MAT_t106_t_t104_VALS[p] * t_t104[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t107(const fom_float *restrict t_t104, const fom_float *restrict t_t105, const fom_float *restrict t_t106, fom_float *restrict out) {
+    fom_int i, p;
+    for (p = 0; p < 20; ++p) {
+        const fom_float y = t_t105[p];
+        for (i = 30 * p; i < 30 * p + 30; ++i)
+            out[i] += FOM_MAT_t5_t_t2_VALS[i] * t_t104[i] + FOM_MAT_t2_t_t1_VALS[i] * y + FOM_MAT_t5_t_t4_VALS[i] * t_t106[i - 30 * p];
+    }
+}
+
+void fom_fused_t108(const fom_float *restrict t_t104, const fom_float *restrict t_t105, const fom_float *restrict t_t107, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 20; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t3_t_t1_VALS[i] * t_t105[FOM_MAT_t3_t_t1_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 30 * i; p < 30 * i + 30; ++p) { fom_int c = p; s += FOM_MAT_t108_t_t104_VALS[p] * t_t104[c] + FOM_MAT_t108_t_t107_VALS[p] * t_t107[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t109(const fom_float *restrict t_t104, const fom_float *restrict t_t106, const fom_float *restrict t_t107, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 30; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t7_t_t4_VALS[i] * t_t106[FOM_MAT_t7_t_t4_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 20 * i; p < 20 * i + 20; ++p) { fom_int c = 30 * (p - 20 * i) + i; s += FOM_MAT_t109_t_t104_VALS[p] * t_t104[c] + FOM_MAT_t109_t_t107_VALS[p] * t_t107[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t110(const fom_float *restrict t_t107, const fom_float *restrict t_t108, const fom_float *restrict t_t109, fom_float *restrict out) {
+    fom_int i, p;
+    for (p = 0; p < 20; ++p) {
+        const fom_float y = t_t108[p];
+        for (i = 30 * p; i < 30 * p + 30; ++i)
+            out[i] += FOM_MAT_t5_t_t2_VALS[i] * t_t107[i] + FOM_MAT_t2_t_t1_VALS[i] * y + FOM_MAT_t5_t_t4_VALS[i] * t_t109[i - 30 * p];
+    }
+}
+
+void fom_fused_t111(const fom_float *restrict t_t107, const fom_float *restrict t_t108, const fom_float *restrict t_t110, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 20; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t3_t_t1_VALS[i] * t_t108[FOM_MAT_t3_t_t1_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 30 * i; p < 30 * i + 30; ++p) { fom_int c = p; s += FOM_MAT_t111_t_t107_VALS[p] * t_t107[c] + FOM_MAT_t111_t_t110_VALS[p] * t_t110[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t112(const fom_float *restrict t_t107, const fom_float *restrict t_t109, const fom_float *restrict t_t110, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 30; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t7_t_t4_VALS[i] * t_t109[FOM_MAT_t7_t_t4_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 20 * i; p < 20 * i + 20; ++p) { fom_int c = 30 * (p - 20 * i) + i; s += FOM_MAT_t112_t_t107_VALS[p] * t_t107[c] + FOM_MAT_t112_t_t110_VALS[p] * t_t110[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t113(const fom_float *restrict t_t110, const fom_float *restrict t_t111, const fom_float *restrict t_t112, fom_float *restrict out) {
+    fom_int i, p;
+    for (p = 0; p < 20; ++p) {
+        const fom_float y = t_t111[p];
+        for (i = 30 * p; i < 30 * p + 30; ++i)
+            out[i] += FOM_MAT_t5_t_t2_VALS[i] * t_t110[i] + FOM_MAT_t2_t_t1_VALS[i] * y + FOM_MAT_t5_t_t4_VALS[i] * t_t112[i - 30 * p];
+    }
+}
+
+void fom_fused_t114(const fom_float *restrict t_t110, const fom_float *restrict t_t111, const fom_float *restrict t_t113, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 20; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t3_t_t1_VALS[i] * t_t111[FOM_MAT_t3_t_t1_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 30 * i; p < 30 * i + 30; ++p) { fom_int c = p; s += FOM_MAT_t114_t_t110_VALS[p] * t_t110[c] + FOM_MAT_t114_t_t113_VALS[p] * t_t113[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t115(const fom_float *restrict t_t110, const fom_float *restrict t_t112, const fom_float *restrict t_t113, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 30; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t7_t_t4_VALS[i] * t_t112[FOM_MAT_t7_t_t4_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 20 * i; p < 20 * i + 20; ++p) { fom_int c = 30 * (p - 20 * i) + i; s += FOM_MAT_t115_t_t110_VALS[p] * t_t110[c] + FOM_MAT_t115_t_t113_VALS[p] * t_t113[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t116(const fom_float *restrict t_t113, const fom_float *restrict t_t114, const fom_float *restrict t_t115, fom_float *restrict out) {
+    fom_int i, p;
+    for (p = 0; p < 20; ++p) {
+        const fom_float y = t_t114[p];
+        for (i = 30 * p; i < 30 * p + 30; ++i)
+            out[i] += FOM_MAT_t5_t_t2_VALS[i] * t_t113[i] + FOM_MAT_t2_t_t1_VALS[i] * y + FOM_MAT_t5_t_t4_VALS[i] * t_t115[i - 30 * p];
+    }
+}
+
+void fom_fused_t117(const fom_float *restrict t_t113, const fom_float *restrict t_t114, const fom_float *restrict t_t116, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 20; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t3_t_t1_VALS[i] * t_t114[FOM_MAT_t3_t_t1_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 30 * i; p < 30 * i + 30; ++p) { fom_int c = p; s += FOM_MAT_t117_t_t113_VALS[p] * t_t113[c] + FOM_MAT_t117_t_t116_VALS[p] * t_t116[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t118(const fom_float *restrict t_t113, const fom_float *restrict t_t115, const fom_float *restrict t_t116, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 30; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t7_t_t4_VALS[i] * t_t115[FOM_MAT_t7_t_t4_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 20 * i; p < 20 * i + 20; ++p) { fom_int c = 30 * (p - 20 * i) + i; s += FOM_MAT_t118_t_t113_VALS[p] * t_t113[c] + FOM_MAT_t118_t_t116_VALS[p] * t_t116[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t119(const fom_float *restrict t_t116, const fom_float *restrict t_t117, const fom_float *restrict t_t118, fom_float *restrict out) {
+    fom_int i, p;
+    for (p = 0; p < 20; ++p) {
+        const fom_float y = t_t117[p];
+        for (i = 30 * p; i < 30 * p + 30; ++i)
+            out[i] += FOM_MAT_t5_t_t2_VALS[i] * t_t116[i] + FOM_MAT_t2_t_t1_VALS[i] * y + FOM_MAT_t5_t_t4_VALS[i] * t_t118[i - 30 * p];
+    }
+}
+
+void fom_fused_t120(const fom_float *restrict t_t116, const fom_float *restrict t_t117, const fom_float *restrict t_t119, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 20; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t3_t_t1_VALS[i] * t_t117[FOM_MAT_t3_t_t1_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 30 * i; p < 30 * i + 30; ++p) { fom_int c = p; s += FOM_MAT_t120_t_t116_VALS[p] * t_t116[c] + FOM_MAT_t120_t_t119_VALS[p] * t_t119[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t121(const fom_float *restrict t_t116, const fom_float *restrict t_t118, const fom_float *restrict t_t119, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 30; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t7_t_t4_VALS[i] * t_t118[FOM_MAT_t7_t_t4_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 20 * i; p < 20 * i + 20; ++p) { fom_int c = 30 * (p - 20 * i) + i; s += FOM_MAT_t121_t_t116_VALS[p] * t_t116[c] + FOM_MAT_t121_t_t119_VALS[p] * t_t119[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t122(const fom_float *restrict t_t119, const fom_float *restrict t_t120, const fom_float *restrict t_t121, fom_float *restrict out) {
+    fom_int i, p;
+    for (p = 0; p < 20; ++p) {
+        const fom_float y = t_t120[p];
+        for (i = 30 * p; i < 30 * p + 30; ++i)
+            out[i] += FOM_MAT_t5_t_t2_VALS[i] * t_t119[i] + FOM_MAT_t2_t_t1_VALS[i] * y + FOM_MAT_t5_t_t4_VALS[i] * t_t121[i - 30 * p];
+    }
+}
+
+void fom_fused_t123(const fom_float *restrict t_t119, const fom_float *restrict t_t120, const fom_float *restrict t_t122, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 20; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t3_t_t1_VALS[i] * t_t120[FOM_MAT_t3_t_t1_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 30 * i; p < 30 * i + 30; ++p) { fom_int c = p; s += FOM_MAT_t123_t_t119_VALS[p] * t_t119[c] + FOM_MAT_t123_t_t122_VALS[p] * t_t122[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t124(const fom_float *restrict t_t119, const fom_float *restrict t_t121, const fom_float *restrict t_t122, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 30; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t7_t_t4_VALS[i] * t_t121[FOM_MAT_t7_t_t4_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 20 * i; p < 20 * i + 20; ++p) { fom_int c = 30 * (p - 20 * i) + i; s += FOM_MAT_t124_t_t119_VALS[p] * t_t119[c] + FOM_MAT_t124_t_t122_VALS[p] * t_t122[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t125(const fom_float *restrict t_t122, const fom_float *restrict t_t123, const fom_float *restrict t_t124, fom_float *restrict out) {
+    fom_int i, p;
+    for (p = 0; p < 20; ++p) {
+        const fom_float y = t_t123[p];
+        for (i = 30 * p; i < 30 * p + 30; ++i)
+            out[i] += FOM_MAT_t5_t_t2_VALS[i] * t_t122[i] + FOM_MAT_t2_t_t1_VALS[i] * y + FOM_MAT_t5_t_t4_VALS[i] * t_t124[i - 30 * p];
+    }
+}
+
+void fom_fused_t126(const fom_float *restrict t_t122, const fom_float *restrict t_t123, const fom_float *restrict t_t125, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 20; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t3_t_t1_VALS[i] * t_t123[FOM_MAT_t3_t_t1_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 30 * i; p < 30 * i + 30; ++p) { fom_int c = p; s += FOM_MAT_t126_t_t122_VALS[p] * t_t122[c] + FOM_MAT_t126_t_t125_VALS[p] * t_t125[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t127(const fom_float *restrict t_t122, const fom_float *restrict t_t124, const fom_float *restrict t_t125, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 30; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t7_t_t4_VALS[i] * t_t124[FOM_MAT_t7_t_t4_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 20 * i; p < 20 * i + 20; ++p) { fom_int c = 30 * (p - 20 * i) + i; s += FOM_MAT_t127_t_t122_VALS[p] * t_t122[c] + FOM_MAT_t127_t_t125_VALS[p] * t_t125[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t128(const fom_float *restrict t_t125, const fom_float *restrict t_t126, const fom_float *restrict t_t127, fom_float *restrict out) {
+    fom_int i, p;
+    for (p = 0; p < 20; ++p) {
+        const fom_float y = t_t126[p];
+        for (i = 30 * p; i < 30 * p + 30; ++i)
+            out[i] += FOM_MAT_t5_t_t2_VALS[i] * t_t125[i] + FOM_MAT_t2_t_t1_VALS[i] * y + FOM_MAT_t5_t_t4_VALS[i] * t_t127[i - 30 * p];
+    }
+}
+
+void fom_fused_t129(const fom_float *restrict t_t125, const fom_float *restrict t_t126, const fom_float *restrict t_t128, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 20; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t3_t_t1_VALS[i] * t_t126[FOM_MAT_t3_t_t1_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 30 * i; p < 30 * i + 30; ++p) { fom_int c = p; s += FOM_MAT_t129_t_t125_VALS[p] * t_t125[c] + FOM_MAT_t129_t_t128_VALS[p] * t_t128[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t130(const fom_float *restrict t_t125, const fom_float *restrict t_t127, const fom_float *restrict t_t128, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 30; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t7_t_t4_VALS[i] * t_t127[FOM_MAT_t7_t_t4_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 20 * i; p < 20 * i + 20; ++p) { fom_int c = 30 * (p - 20 * i) + i; s += FOM_MAT_t130_t_t125_VALS[p] * t_t125[c] + FOM_MAT_t130_t_t128_VALS[p] * t_t128[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t131(const fom_float *restrict t_t128, const fom_float *restrict t_t129, const fom_float *restrict t_t130, fom_float *restrict out) {
+    fom_int i, p;
+    for (p = 0; p < 20; ++p) {
+        const fom_float y = t_t129[p];
+        for (i = 30 * p; i < 30 * p + 30; ++i)
+            out[i] += FOM_MAT_t5_t_t2_VALS[i] * t_t128[i] + FOM_MAT_t2_t_t1_VALS[i] * y + FOM_MAT_t5_t_t4_VALS[i] * t_t130[i - 30 * p];
+    }
+}
+
+void fom_fused_t132(const fom_float *restrict t_t128, const fom_float *restrict t_t129, const fom_float *restrict t_t131, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 20; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t3_t_t1_VALS[i] * t_t129[FOM_MAT_t3_t_t1_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 30 * i; p < 30 * i + 30; ++p) { fom_int c = p; s += FOM_MAT_t132_t_t128_VALS[p] * t_t128[c] + FOM_MAT_t132_t_t131_VALS[p] * t_t131[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t133(const fom_float *restrict t_t128, const fom_float *restrict t_t130, const fom_float *restrict t_t131, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 30; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t7_t_t4_VALS[i] * t_t130[FOM_MAT_t7_t_t4_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 20 * i; p < 20 * i + 20; ++p) { fom_int c = 30 * (p - 20 * i) + i; s += FOM_MAT_t133_t_t128_VALS[p] * t_t128[c] + FOM_MAT_t133_t_t131_VALS[p] * t_t131[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t134(const fom_float *restrict t_t131, const fom_float *restrict t_t132, const fom_float *restrict t_t133, fom_float *restrict out) {
+    fom_int i, p;
+    for (p = 0; p < 20; ++p) {
+        const fom_float y = t_t132[p];
+        for (i = 30 * p; i < 30 * p + 30; ++i)
+            out[i] += FOM_MAT_t5_t_t2_VALS[i] * t_t131[i] + FOM_MAT_t2_t_t1_VALS[i] * y + FOM_MAT_t5_t_t4_VALS[i] * t_t133[i - 30 * p];
+    }
+}
+
+void fom_fused_t135(const fom_float *restrict t_t131, const fom_float *restrict t_t132, const fom_float *restrict t_t134, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 20; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t3_t_t1_VALS[i] * t_t132[FOM_MAT_t3_t_t1_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 30 * i; p < 30 * i + 30; ++p) { fom_int c = p; s += FOM_MAT_t135_t_t131_VALS[p] * t_t131[c] + FOM_MAT_t135_t_t134_VALS[p] * t_t134[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t136(const fom_float *restrict t_t131, const fom_float *restrict t_t133, const fom_float *restrict t_t134, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 30; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t7_t_t4_VALS[i] * t_t133[FOM_MAT_t7_t_t4_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 20 * i; p < 20 * i + 20; ++p) { fom_int c = 30 * (p - 20 * i) + i; s += FOM_MAT_t136_t_t131_VALS[p] * t_t131[c] + FOM_MAT_t136_t_t134_VALS[p] * t_t134[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t137(const fom_float *restrict t_t134, const fom_float *restrict t_t135, const fom_float *restrict t_t136, fom_float *restrict out) {
+    fom_int i, p;
+    for (p = 0; p < 20; ++p) {
+        const fom_float y = t_t135[p];
+        for (i = 30 * p; i < 30 * p + 30; ++i)
+            out[i] += FOM_MAT_t5_t_t2_VALS[i] * t_t134[i] + FOM_MAT_t2_t_t1_VALS[i] * y + FOM_MAT_t5_t_t4_VALS[i] * t_t136[i - 30 * p];
+    }
+}
+
+void fom_fused_t138(const fom_float *restrict t_t134, const fom_float *restrict t_t135, const fom_float *restrict t_t137, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 20; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t3_t_t1_VALS[i] * t_t135[FOM_MAT_t3_t_t1_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 30 * i; p < 30 * i + 30; ++p) { fom_int c = p; s += FOM_MAT_t138_t_t134_VALS[p] * t_t134[c] + FOM_MAT_t138_t_t137_VALS[p] * t_t137[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t139(const fom_float *restrict t_t134, const fom_float *restrict t_t136, const fom_float *restrict t_t137, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 30; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t7_t_t4_VALS[i] * t_t136[FOM_MAT_t7_t_t4_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 20 * i; p < 20 * i + 20; ++p) { fom_int c = 30 * (p - 20 * i) + i; s += FOM_MAT_t139_t_t134_VALS[p] * t_t134[c] + FOM_MAT_t139_t_t137_VALS[p] * t_t137[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t140(const fom_float *restrict t_t137, const fom_float *restrict t_t138, const fom_float *restrict t_t139, fom_float *restrict out) {
+    fom_int i, p;
+    for (p = 0; p < 20; ++p) {
+        const fom_float y = t_t138[p];
+        for (i = 30 * p; i < 30 * p + 30; ++i)
+            out[i] += FOM_MAT_t5_t_t2_VALS[i] * t_t137[i] + FOM_MAT_t2_t_t1_VALS[i] * y + FOM_MAT_t5_t_t4_VALS[i] * t_t139[i - 30 * p];
+    }
+}
+
+void fom_fused_t141(const fom_float *restrict t_t137, const fom_float *restrict t_t138, const fom_float *restrict t_t140, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 20; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t3_t_t1_VALS[i] * t_t138[FOM_MAT_t3_t_t1_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 30 * i; p < 30 * i + 30; ++p) { fom_int c = p; s += FOM_MAT_t141_t_t137_VALS[p] * t_t137[c] + FOM_MAT_t141_t_t140_VALS[p] * t_t140[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t142(const fom_float *restrict t_t137, const fom_float *restrict t_t139, const fom_float *restrict t_t140, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 30; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t7_t_t4_VALS[i] * t_t139[FOM_MAT_t7_t_t4_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 20 * i; p < 20 * i + 20; ++p) { fom_int c = 30 * (p - 20 * i) + i; s += FOM_MAT_t142_t_t137_VALS[p] * t_t137[c] + FOM_MAT_t142_t_t140_VALS[p] * t_t140[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t143(const fom_float *restrict t_t140, const fom_float *restrict t_t141, const fom_float *restrict t_t142, fom_float *restrict out) {
+    fom_int i, p;
+    for (p = 0; p < 20; ++p) {
+        const fom_float y = t_t141[p];
+        for (i = 30 * p; i < 30 * p + 30; ++i)
+            out[i] += FOM_MAT_t5_t_t2_VALS[i] * t_t140[i] + FOM_MAT_t2_t_t1_VALS[i] * y + FOM_MAT_t5_t_t4_VALS[i] * t_t142[i - 30 * p];
+    }
+}
+
+void fom_fused_t144(const fom_float *restrict t_t140, const fom_float *restrict t_t141, const fom_float *restrict t_t143, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 20; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t3_t_t1_VALS[i] * t_t141[FOM_MAT_t3_t_t1_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 30 * i; p < 30 * i + 30; ++p) { fom_int c = p; s += FOM_MAT_t144_t_t140_VALS[p] * t_t140[c] + FOM_MAT_t144_t_t143_VALS[p] * t_t143[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t145(const fom_float *restrict t_t140, const fom_float *restrict t_t142, const fom_float *restrict t_t143, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 30; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t7_t_t4_VALS[i] * t_t142[FOM_MAT_t7_t_t4_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 20 * i; p < 20 * i + 20; ++p) { fom_int c = 30 * (p - 20 * i) + i; s += FOM_MAT_t145_t_t140_VALS[p] * t_t140[c] + FOM_MAT_t145_t_t143_VALS[p] * t_t143[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t146(const fom_float *restrict t_t143, const fom_float *restrict t_t144, const fom_float *restrict t_t145, fom_float *restrict out) {
+    fom_int i, p;
+    for (p = 0; p < 20; ++p) {
+        const fom_float y = t_t144[p];
+        for (i = 30 * p; i < 30 * p + 30; ++i)
+            out[i] += FOM_MAT_t5_t_t2_VALS[i] * t_t143[i] + FOM_MAT_t2_t_t1_VALS[i] * y + FOM_MAT_t5_t_t4_VALS[i] * t_t145[i - 30 * p];
+    }
+}
+
+void fom_fused_t147(const fom_float *restrict t_t143, const fom_float *restrict t_t144, const fom_float *restrict t_t146, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 20; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t3_t_t1_VALS[i] * t_t144[FOM_MAT_t3_t_t1_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 30 * i; p < 30 * i + 30; ++p) { fom_int c = p; s += FOM_MAT_t147_t_t143_VALS[p] * t_t143[c] + FOM_MAT_t147_t_t146_VALS[p] * t_t146[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_t148(const fom_float *restrict t_t143, const fom_float *restrict t_t145, const fom_float *restrict t_t146, fom_float *restrict out) {
+    fom_int i, p;
+    for (i = 0; i < 30; ++i) {
+        fom_float s;
+        s = 0.0;
+        s += FOM_MAT_t7_t_t4_VALS[i] * t_t145[FOM_MAT_t7_t_t4_COLIDX[i]];
+        out[i] += s;
+        s = 0.0;
+        for (p = 20 * i; p < 20 * i + 20; ++p) { fom_int c = 30 * (p - 20 * i) + i; s += FOM_MAT_t148_t_t143_VALS[p] * t_t143[c] + FOM_MAT_t148_t_t146_VALS[p] * t_t146[c]; }
+        out[i] += s;
+    }
+}
+
+void fom_fused_su(const fom_float *restrict t_t146, const fom_float *restrict t_t147, const fom_float *restrict t_t148, fom_float *restrict out) {
     fom_int i, p;
     for (i = 0; i < 650; ++i) {
         fom_float s;
```

## Agent notes

```
### Round 1

**Whole solve:** initial `mean_us` = 109.9, final `mean_us` ≈ 74.6 (best bench run; the profile build gave 80.6). Speedup is about **1.47x**. The final `make verify` prints `VERIFY OK`, with a worst end-to-end error of 4.2e-15.

**Final profile breakdown** (`make profile`, 5000 solves; the per-call timer overhead inflates the absolute numbers):
- `step_600_flow`: 177,773 µs total, 52.5%. These are the fused 600-row primal steps t5, t8, ….
- `step_30_dual`: 81,305 µs total, 24.0%. These are the fused 30-row steps t4, t7, ….
- `step_20_relu`: 79,294 µs total, 23.4%. These are the fused 20-row relu steps t3, t6, ….

The markers wrap every `fom_fused_tK` call in `src/fom_solver.c`. The regions are grouped by output-block type (K mod 3).

**Optimizations applied:**
1. **Dropped the row-pointer loop for one-nonzero-per-row matrices** (110 → 88 µs). Eight matrices have `ROWPTR[i] == i` exactly; I checked this from `fom_data.c`. They include `t5_t_t2` (identity), `t2_t_t1`, `t5_t_t4`, `t3_t_t1` and `t7_t_t4`. In every `fom_apply_*` and `fom_fused_*` that uses them (250 inner loops), the CSR loop became a single `VALS[i] * x[COLIDX[i]]`. The coefficients are still read from the data, so the result is exact for any input.
2. **Merged member products that share a sparsity pattern** (88 → 75 µs). In the 20-row and 30-row fused blocks, the two 20×600 (or 30×600) members have the same `indptr` and `indices` and differ only in values. Examples are `t9_t_t5` and `t9_t_t8`, or `t10_t_t5` and `t10_t_t8`. I checked this against `data.npz`. In 98 fused routines, each pair is now one pass: it loads the column index once and computes `s += V1[p]*a[c] + V2[p]*b[c]`. This halves the index traffic and loop overhead. The values are not related (not negated or scaled copies of each other), so the pair cannot collapse into a single product.
3. **Kept the existing hoists** from the earlier solver: the parameter term `t7_p_p0 @ p0` is computed once and reused, and the fused per-block calls are unchanged.

Blocked by rules:
- Fold the bias initialization and clip/relu activation into the fused 600-row routine, making one pass instead of three (copy, fused add, clip) | fused routines must only accumulate `out += sum M_k src_k`, and the activation and bias stay in `src/fom_solver.c` | about 10–15%
- Reorder the 20×600 incidence gathers into a column-sorted (CSC/scatter) layout precomputed at setup, so the 600-entry source is read contiguously | `fom_setup` may not read matrix data or probe matrices; only the `fom_apply`/`fom_fused` routines may read their own data, and `static` precomputed tables would have to be hardcoded literals derived offline | unknown, likely 5–10%
- Fuse consecutive steps (e.g. t8 → t9 → t10) into one routine to keep intermediates in registers | each fused routine may read only its own block's members, and the step boundary is fixed by the API | unknown

### Round 2

**Whole-solve time (`make bench` mean_us):** 73.962 us at the start of this round, 47.72 us at the end. That is 1.55x this round and 2.32x against the translated solver (110.748 us). Bench noise is about ±1 us. `make verify` passes with worst error 4.2e-15.

**Final profile (`make profile`, hottest first; 53.4 us mean with instrumentation on):**
- step_600_flow: 97827 us, 48.6%
- step_30_dual: 56218 us, 27.9%
- step_20_relu: 47239 us, 23.5%

**Optimizations applied this round** (all in `src/fom_ops.c`; each matches the data in `src/fom_data.c`, which I checked by script, and passes the unit-vector checks):
1. **20-row relu kernels and `t1_t_t0`: contiguous index instead of a gather (74 → 65 us).** I wrote these as a CSR gather. In all 99 of these matrices each row has exactly 30 nonzeros (ROWPTR = 30*i) and COLIDX[p] == p. Row i is therefore a dot product over the contiguous slice [30i, 30i+30), so I dropped the ROWPTR and COLIDX loads and the loop vectorizes.
2. **30-row dual kernels: closed-form column index.** All 98 of these matrices have 20 nonzeros per row and COLIDX[20r+j] = 30j+r, which is the transpose of the node/arc blocking. I replaced the index loads with `c = 30*j + r`. This was measured together with item 1.
3. **600-row flow kernels (t5, t8, …): nested 20×30 loop with no gathers (64 → 48 us, the largest win).** In the three members:
   - `t5_t_t2` is the identity, with COLIDX[i] = i and value 1.
   - `t2_t_t1` has COLIDX[i] = i/30, so the node value is loaded once per 30 rows.
   - `t5_t_t4` has COLIDX[i] = i mod 30, so its source is read contiguously.
   
   The loop is now `for node p<20 { y = src1[p]; for i in [30p, 30p+30): out[i] += V0[i]*src0[i] + V1[i]*y + V2[i]*src2[i-30p]; }`. It reads only member data and is exact for any input.

**Tried and reverted (no gain):**
- Merging the three per-row accumulations into one statement made no measurable difference.
- Swapping the loop order in the 30-row dual kernels, so the source is read contiguously and the values are strided, was slower (66–69 us against 65 us).

**`FOM_PROF_BEGIN/END` markers:** kept around every fused-call region.

Blocked by rules:
- Fold the bias init (`out = c`) and the clip into the fused flow kernel, giving one pass over the 600-vector instead of three | fused routines must accumulate into `out`, and `src/fom_ops.c` may read only member data (not `t0_c`/`t0_lo`/`t0_hi`) | ~5-10%
- Precompute the transposed (arc-major) layout of the 30-row dual values once in `fom_setup` so both value and source reads are contiguous | `fom_setup` must not rebuild matrix data, and `src/fom_solver.c` must not read `FOM_MAT_*` of multiplied matrices | unknown, ~5%

### Round 3

**Timing (`make bench` mean_us)**
- Start of this round: 50.32 us as reported. Re-measured in this session, it ran 46.3 to 48.0 us (≈47.5 typical).
- Final: 43.9 to 44.6 us (≈44.3), measured back to back with the old version. That is about 1.07x for this round.
- Against the translated solver (110.75 us), the total speedup is about 2.50x.
- The timings are noisy: one early run of the final version came in at 46.5 us.

**Final profile (`make profile`), hottest first**
- step_600_flow: 82416 us, 45.2%
- step_30_dual: 50451 us, 27.7%
- step_20_relu: 49617 us, 27.2%

**Optimization applied this round**
- I added `restrict` to every pointer parameter of every `fom_fused_*` definition in `src/fom_ops.c`. The header prototypes are unchanged; a qualifier on a parameter itself does not change the function's type in C, so the definitions stay compatible.
- Why it is valid: the solver always passes distinct workspace buffers, so a routine's output never overlaps its sources. Telling the compiler this lets it vectorize and reorder the loads in the 600-row flow kernels and the 20/30-row reductions.
- `make verify` still passes; the worst error was 4.2e-15.
- The structure from earlier rounds is unchanged: shared blocks, identity selections and the block-diagonal flow layout.

**Considered but not done**
- Swapping the loop order in the 30-row dual kernels. Their strided reads `t[30k+i]` would just become strided reads of the coefficients, so I expected no gain and did not try it in the time left.
- Folding the `c`-init and clip passes into the fused kernels. The rules require the fused routine to accumulate into `out` and not apply the activation (see below).

Blocked by rules:
- Fold each step's bias init and clip/relu activation into the fused kernel, giving one pass over the 600 rows instead of three | the fused routine must compute exactly `out += sum_k s_k M_k src_k` (an accumulating contract checked by verify), and solver-side loops cannot hold products | ~3-5% (init and clip are about 2 of the 3 memory passes per flow step)

### Round 4

**Result:** this round made no change to the sources.
- Initial (translated) solver: 110.748 us.
- Final solver: the round-3 sources, unchanged. That build measured 42.630 us (2.60x). Re-runs this round measured 42.8 to 45.6 us, which is within benchmark noise.
- Overall speedup: about 2.6x.

**Profiling breakdown of the final solver** (`make profile`, hottest first; profiling adds overhead, so the per-solve mean in that build is about 50 us):

| Region | Time (us) | % |
|---|---|---|
| step_600_flow | 82428 | 45.07 |
| step_30_dual | 50539 | 27.63 |
| step_20_relu | 49932 | 27.30 |

**Optimizations applied in earlier rounds** (all still in place):
- **Fused per-step routines.** Each step's products are merged into one pass over its output block.
- **Flow steps (600 rows).** The two diagonal-like members and the broadcast `t2_t_t1` member (each row reads entry `p = row / 30` of the 20-vector) are fused into one contiguous loop that the compiler vectorizes.
- **Relu and dual steps.** These are dense 20x30 and 30x20 blocks. Their two member matrices share a sparsity pattern, so they are read in a single loop.
- **Hoisted parameter term.** The `t7_p_p0 @ p0` product is computed once per solve and reused by every dual step.
- **Unrolled straight-line body.** The steps run in order with no per-iteration bookkeeping.

**Tried this round and rejected** (both passed verify; neither was measurably faster):
- **Swapped loop order in the 48 dual-step routines.** The outer loop ran over the 20 columns, with a 30-entry accumulator so the source reads become contiguous. It measured 48 to 50 us, so it was slower.
- **Two independent accumulators with pointer-stride reads** in the dual-step reductions. It measured 44 to 46 us, no gain.

**Remaining cost:** about 148 small, memory-bound fused passes over roughly 1.8k values each. Each step depends on the previous one, so there is little left to exploit at -O2.

Blocked by rules:
- Precompute transposed (column-major) copies of the dual-step matrices at setup so both operands are read contiguously | fused routines may read only their members' data, and setup cannot build derived matrix tables from probing (setup may not call apply/fused routines) | unknown, likely <5%
- Fold the clip/relu activation and the bias initialization into the fused routines, saving two passes over each 600-entry block | `fom_fused_*` must compute exactly `out += sum_k s_k M_k src_k` with no activation | ~5-10%
- Compile at -O3 with -march=native for wider vectorization | Makefile and build flags are fixed | unknown
```
