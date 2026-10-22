# Optimization report

## Timing

```
translated (emitted)     111.5 us/solve
optimized                 47.7 us/solve
speedup                  2.34x
```

Optimization outcome: **kept (re-verified correct)**.

## Rounds

| round | before (us) | after (us) | gain | kept | agent (s) | outcome |
|---:|---:|---:|---:|:---:|---:|---|
| 1 | 111.479 | 47.670 | 57.2% | yes | 305 | continued |
| 2 | 48.088 | 48.573 | -1.0% | no | 236 | the round's sources were not faster than the best so far |

## Kernel checks

Each `fom_apply_<name>` in the shipped `fom_ops.c`, checked by `make verify` against the package's reference over the same `fom_data.c` matrix. *basis* probes every unit vector, which recovers the routine's matrix column by column (an exact check), plus random vectors; *random* uses random vectors only (the matrix is over the probe budget). A `fused_<block>` row is a `fom_fused_<block>`, checked against the sum of its members: *basis* there probes every unit vector of each source with every schedule value at 0 and with each alone at 1, which recovers every member exactly.

| matrix | shape | nnz | rewritten | check | probes | max error |
|---|---|---|---|---|---|---|
| `t0_state` | 600x650 | 1800 | yes | basis | 652 | 0.000e+00 |
| `t1_state` | 20x650 | 620 | yes | basis | 652 | 0.000e+00 |
| `t1_t_t0` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t2_state` | 600x650 | 12600 | yes | basis | 652 | 0.000e+00 |
| `t2_p_p0` | 600x30 | 600 | yes | basis | 32 | 0.000e+00 |
| `t2_t_t0` | 600x600 | 12000 | yes | basis | 602 | 0.000e+00 |
| `t2_t_t1` | 600x20 | 600 | yes | basis | 22 | 0.000e+00 |
| `t3_t_t0` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t3_t_t1` | 20x20 | 20 | yes | basis | 22 | 0.000e+00 |
| `t3_t_t2` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t4_state` | 30x650 | 630 | yes | basis | 652 | 0.000e+00 |
| `t4_p_p0` | 30x30 | 30 | yes | basis | 32 | 0.000e+00 |
| `t4_t_t0` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t4_t_t2` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t5_t_t2` | 600x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t5_t_t4` | 600x30 | 600 | yes | basis | 32 | 0.000e+00 |
| `t6_t_t2` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t6_t_t5` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t7_p_p0` | 30x30 | 30 | yes | basis | 32 | 0.000e+00 |
| `t7_t_t2` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t7_t_t4` | 30x30 | 30 | yes | basis | 32 | 0.000e+00 |
| `t7_t_t5` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t9_t_t5` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t9_t_t8` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t10_t_t5` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t10_t_t8` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t12_t_t8` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t12_t_t11` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t13_t_t8` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t13_t_t11` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t15_t_t11` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t15_t_t14` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t16_t_t11` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t16_t_t14` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t18_t_t14` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t18_t_t17` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t19_t_t14` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t19_t_t17` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t21_t_t17` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t21_t_t20` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t22_t_t17` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t22_t_t20` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t24_t_t20` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t24_t_t23` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t25_t_t20` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t25_t_t23` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t27_t_t23` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t27_t_t26` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t28_t_t23` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t28_t_t26` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t30_t_t26` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t30_t_t29` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t31_t_t26` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t31_t_t29` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t33_t_t29` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t33_t_t32` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t34_t_t29` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t34_t_t32` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t36_t_t32` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t36_t_t35` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t37_t_t32` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t37_t_t35` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t39_t_t35` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t39_t_t38` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t40_t_t35` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t40_t_t38` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t42_t_t38` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t42_t_t41` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t43_t_t38` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t43_t_t41` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t45_t_t41` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t45_t_t44` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t46_t_t41` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t46_t_t44` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t48_t_t44` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t48_t_t47` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t49_t_t44` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t49_t_t47` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t51_t_t47` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t51_t_t50` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t52_t_t47` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t52_t_t50` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t54_t_t50` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t54_t_t53` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t55_t_t50` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t55_t_t53` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t57_t_t53` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t57_t_t56` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t58_t_t53` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t58_t_t56` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t60_t_t56` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t60_t_t59` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t61_t_t56` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t61_t_t59` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t63_t_t59` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t63_t_t62` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t64_t_t59` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t64_t_t62` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t66_t_t62` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t66_t_t65` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t67_t_t62` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t67_t_t65` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t69_t_t65` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t69_t_t68` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t70_t_t65` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t70_t_t68` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t72_t_t68` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t72_t_t71` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t73_t_t68` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t73_t_t71` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t75_t_t71` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t75_t_t74` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t76_t_t71` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t76_t_t74` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t78_t_t74` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t78_t_t77` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t79_t_t74` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t79_t_t77` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t81_t_t77` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t81_t_t80` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t82_t_t77` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t82_t_t80` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t84_t_t80` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t84_t_t83` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t85_t_t80` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t85_t_t83` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t87_t_t83` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t87_t_t86` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t88_t_t83` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t88_t_t86` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t90_t_t86` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t90_t_t89` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t91_t_t86` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t91_t_t89` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t93_t_t89` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t93_t_t92` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t94_t_t89` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t94_t_t92` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t96_t_t92` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t96_t_t95` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t97_t_t92` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t97_t_t95` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t99_t_t95` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t99_t_t98` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t100_t_t95` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t100_t_t98` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t102_t_t98` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t102_t_t101` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t103_t_t98` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t103_t_t101` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t105_t_t101` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t105_t_t104` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t106_t_t101` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t106_t_t104` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t108_t_t104` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t108_t_t107` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t109_t_t104` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t109_t_t107` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t111_t_t107` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t111_t_t110` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t112_t_t107` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t112_t_t110` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t114_t_t110` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t114_t_t113` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t115_t_t110` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t115_t_t113` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t117_t_t113` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t117_t_t116` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t118_t_t113` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t118_t_t116` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t120_t_t116` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t120_t_t119` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t121_t_t116` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t121_t_t119` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t123_t_t119` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t123_t_t122` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t124_t_t119` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t124_t_t122` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t126_t_t122` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t126_t_t125` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t127_t_t122` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t127_t_t125` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t129_t_t125` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t129_t_t128` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t130_t_t125` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t130_t_t128` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t132_t_t128` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t132_t_t131` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t133_t_t128` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t133_t_t131` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t135_t_t131` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t135_t_t134` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t136_t_t131` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t136_t_t134` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t138_t_t134` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t138_t_t137` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t139_t_t134` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t139_t_t137` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t141_t_t137` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t141_t_t140` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t142_t_t137` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t142_t_t140` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t144_t_t140` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t144_t_t143` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t145_t_t140` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t145_t_t143` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t147_t_t143` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t147_t_t146` | 20x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t148_t_t143` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `t148_t_t146` | 30x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `su_t_t146` | 650x600 | 600 | yes | basis | 602 | 0.000e+00 |
| `su_t_t147` | 650x20 | 20 | yes | basis | 22 | 0.000e+00 |
| `su_t_t148` | 650x30 | 30 | yes | basis | 32 | 0.000e+00 |
| `fused_t1` | 20x1250 | 1220 | yes | basis | 1252 | 0.000e+00 |
| `fused_t2` | 600x1270 | 25200 | yes | basis | 1272 | 0.000e+00 |
| `fused_t3` | 20x1220 | 1220 | yes | basis | 1222 | 8.882e-16 |
| `fused_t4` | 30x1850 | 1830 | yes | basis | 1852 | 0.000e+00 |
| `fused_t5` | 600x650 | 1800 | yes | basis | 652 | 1.776e-15 |
| `fused_t6` | 20x1220 | 1220 | yes | basis | 1222 | 8.882e-16 |
| `fused_t7` | 30x1230 | 1230 | yes | basis | 1232 | 2.665e-15 |
| `fused_t8` | 600x650 | 1800 | yes | basis | 652 | 8.882e-16 |
| `fused_t9` | 20x1220 | 1220 | yes | basis | 1222 | 8.882e-16 |
| `fused_t10` | 30x1230 | 1230 | yes | basis | 1232 | 8.882e-16 |
| `fused_t11` | 600x650 | 1800 | yes | basis | 652 | 8.882e-16 |
| `fused_t12` | 20x1220 | 1220 | yes | basis | 1222 | 1.776e-15 |
| `fused_t13` | 30x1230 | 1230 | yes | basis | 1232 | 1.332e-15 |
| `fused_t14` | 600x650 | 1800 | yes | basis | 652 | 1.776e-15 |
| `fused_t15` | 20x1220 | 1220 | yes | basis | 1222 | 1.332e-15 |
| `fused_t16` | 30x1230 | 1230 | yes | basis | 1232 | 9.992e-16 |
| `fused_t17` | 600x650 | 1800 | yes | basis | 652 | 8.882e-16 |
| `fused_t18` | 20x1220 | 1220 | yes | basis | 1222 | 1.776e-15 |
| `fused_t19` | 30x1230 | 1230 | yes | basis | 1232 | 1.776e-15 |
| `fused_t20` | 600x650 | 1800 | yes | basis | 652 | 8.882e-16 |
| `fused_t21` | 20x1220 | 1220 | yes | basis | 1222 | 1.277e-15 |
| `fused_t22` | 30x1230 | 1230 | yes | basis | 1232 | 1.332e-15 |
| `fused_t23` | 600x650 | 1800 | yes | basis | 652 | 1.776e-15 |
| `fused_t24` | 20x1220 | 1220 | yes | basis | 1222 | 1.110e-15 |
| `fused_t25` | 30x1230 | 1230 | yes | basis | 1232 | 8.882e-16 |
| `fused_t26` | 600x650 | 1800 | yes | basis | 652 | 8.882e-16 |
| `fused_t27` | 20x1220 | 1220 | yes | basis | 1222 | 8.882e-16 |
| `fused_t28` | 30x1230 | 1230 | yes | basis | 1232 | 1.776e-15 |
| `fused_t29` | 600x650 | 1800 | yes | basis | 652 | 8.882e-16 |
| `fused_t30` | 20x1220 | 1220 | yes | basis | 1222 | 2.220e-15 |
| `fused_t31` | 30x1230 | 1230 | yes | basis | 1232 | 1.776e-15 |
| `fused_t32` | 600x650 | 1800 | yes | basis | 652 | 1.332e-15 |
| `fused_t33` | 20x1220 | 1220 | yes | basis | 1222 | 1.776e-15 |
| `fused_t34` | 30x1230 | 1230 | yes | basis | 1232 | 1.776e-15 |
| `fused_t35` | 600x650 | 1800 | yes | basis | 652 | 1.776e-15 |
| `fused_t36` | 20x1220 | 1220 | yes | basis | 1222 | 1.554e-15 |
| `fused_t37` | 30x1230 | 1230 | yes | basis | 1232 | 8.882e-16 |
| `fused_t38` | 600x650 | 1800 | yes | basis | 652 | 1.776e-15 |
| `fused_t39` | 20x1220 | 1220 | yes | basis | 1222 | 1.221e-15 |
| `fused_t40` | 30x1230 | 1230 | yes | basis | 1232 | 1.332e-15 |
| `fused_t41` | 600x650 | 1800 | yes | basis | 652 | 8.882e-16 |
| `fused_t42` | 20x1220 | 1220 | yes | basis | 1222 | 1.332e-15 |
| `fused_t43` | 30x1230 | 1230 | yes | basis | 1232 | 8.882e-16 |
| `fused_t44` | 600x650 | 1800 | yes | basis | 652 | 8.882e-16 |
| `fused_t45` | 20x1220 | 1220 | yes | basis | 1222 | 1.776e-15 |
| `fused_t46` | 30x1230 | 1230 | yes | basis | 1232 | 8.882e-16 |
| `fused_t47` | 600x650 | 1800 | yes | basis | 652 | 8.882e-16 |
| `fused_t48` | 20x1220 | 1220 | yes | basis | 1222 | 1.776e-15 |
| `fused_t49` | 30x1230 | 1230 | yes | basis | 1232 | 8.882e-16 |
| `fused_t50` | 600x650 | 1800 | yes | basis | 652 | 1.776e-15 |
| `fused_t51` | 20x1220 | 1220 | yes | basis | 1222 | 8.882e-16 |
| `fused_t52` | 30x1230 | 1230 | yes | basis | 1232 | 8.882e-16 |
| `fused_t53` | 600x650 | 1800 | yes | basis | 652 | 8.882e-16 |
| `fused_t54` | 20x1220 | 1220 | yes | basis | 1222 | 8.882e-16 |
| `fused_t55` | 30x1230 | 1230 | yes | basis | 1232 | 8.882e-16 |
| `fused_t56` | 600x650 | 1800 | yes | basis | 652 | 8.882e-16 |
| `fused_t57` | 20x1220 | 1220 | yes | basis | 1222 | 1.776e-15 |
| `fused_t58` | 30x1230 | 1230 | yes | basis | 1232 | 8.882e-16 |
| `fused_t59` | 600x650 | 1800 | yes | basis | 652 | 8.882e-16 |
| `fused_t60` | 20x1220 | 1220 | yes | basis | 1222 | 1.776e-15 |
| `fused_t61` | 30x1230 | 1230 | yes | basis | 1232 | 1.110e-15 |
| `fused_t62` | 600x650 | 1800 | yes | basis | 652 | 8.882e-16 |
| `fused_t63` | 20x1220 | 1220 | yes | basis | 1222 | 3.553e-15 |
| `fused_t64` | 30x1230 | 1230 | yes | basis | 1232 | 1.332e-15 |
| `fused_t65` | 600x650 | 1800 | yes | basis | 652 | 1.776e-15 |
| `fused_t66` | 20x1220 | 1220 | yes | basis | 1222 | 1.776e-15 |
| `fused_t67` | 30x1230 | 1230 | yes | basis | 1232 | 8.882e-16 |
| `fused_t68` | 600x650 | 1800 | yes | basis | 652 | 8.882e-16 |
| `fused_t69` | 20x1220 | 1220 | yes | basis | 1222 | 8.882e-16 |
| `fused_t70` | 30x1230 | 1230 | yes | basis | 1232 | 8.882e-16 |
| `fused_t71` | 600x650 | 1800 | yes | basis | 652 | 8.882e-16 |
| `fused_t72` | 20x1220 | 1220 | yes | basis | 1222 | 1.776e-15 |
| `fused_t73` | 30x1230 | 1230 | yes | basis | 1232 | 8.882e-16 |
| `fused_t74` | 600x650 | 1800 | yes | basis | 652 | 8.882e-16 |
| `fused_t75` | 20x1220 | 1220 | yes | basis | 1222 | 1.776e-15 |
| `fused_t76` | 30x1230 | 1230 | yes | basis | 1232 | 1.776e-15 |
| `fused_t77` | 600x650 | 1800 | yes | basis | 652 | 8.882e-16 |
| `fused_t78` | 20x1220 | 1220 | yes | basis | 1222 | 1.110e-15 |
| `fused_t79` | 30x1230 | 1230 | yes | basis | 1232 | 1.332e-15 |
| `fused_t80` | 600x650 | 1800 | yes | basis | 652 | 8.882e-16 |
| `fused_t81` | 20x1220 | 1220 | yes | basis | 1222 | 1.776e-15 |
| `fused_t82` | 30x1230 | 1230 | yes | basis | 1232 | 8.882e-16 |
| `fused_t83` | 600x650 | 1800 | yes | basis | 652 | 8.882e-16 |
| `fused_t84` | 20x1220 | 1220 | yes | basis | 1222 | 1.776e-15 |
| `fused_t85` | 30x1230 | 1230 | yes | basis | 1232 | 8.882e-16 |
| `fused_t86` | 600x650 | 1800 | yes | basis | 652 | 1.776e-15 |
| `fused_t87` | 20x1220 | 1220 | yes | basis | 1222 | 8.882e-16 |
| `fused_t88` | 30x1230 | 1230 | yes | basis | 1232 | 1.110e-15 |
| `fused_t89` | 600x650 | 1800 | yes | basis | 652 | 1.776e-15 |
| `fused_t90` | 20x1220 | 1220 | yes | basis | 1222 | 3.553e-15 |
| `fused_t91` | 30x1230 | 1230 | yes | basis | 1232 | 8.882e-16 |
| `fused_t92` | 600x650 | 1800 | yes | basis | 652 | 8.882e-16 |
| `fused_t93` | 20x1220 | 1220 | yes | basis | 1222 | 1.332e-15 |
| `fused_t94` | 30x1230 | 1230 | yes | basis | 1232 | 1.776e-15 |
| `fused_t95` | 600x650 | 1800 | yes | basis | 652 | 1.776e-15 |
| `fused_t96` | 20x1220 | 1220 | yes | basis | 1222 | 1.554e-15 |
| `fused_t97` | 30x1230 | 1230 | yes | basis | 1232 | 2.887e-15 |
| `fused_t98` | 600x650 | 1800 | yes | basis | 652 | 8.882e-16 |
| `fused_t99` | 20x1220 | 1220 | yes | basis | 1222 | 1.776e-15 |
| `fused_t100` | 30x1230 | 1230 | yes | basis | 1232 | 8.882e-16 |
| `fused_t101` | 600x650 | 1800 | yes | basis | 652 | 1.776e-15 |
| `fused_t102` | 20x1220 | 1220 | yes | basis | 1222 | 8.882e-16 |
| `fused_t103` | 30x1230 | 1230 | yes | basis | 1232 | 8.882e-16 |
| `fused_t104` | 600x650 | 1800 | yes | basis | 652 | 8.882e-16 |
| `fused_t105` | 20x1220 | 1220 | yes | basis | 1222 | 2.665e-15 |
| `fused_t106` | 30x1230 | 1230 | yes | basis | 1232 | 8.882e-16 |
| `fused_t107` | 600x650 | 1800 | yes | basis | 652 | 8.882e-16 |
| `fused_t108` | 20x1220 | 1220 | yes | basis | 1222 | 8.882e-16 |
| `fused_t109` | 30x1230 | 1230 | yes | basis | 1232 | 1.776e-15 |
| `fused_t110` | 600x650 | 1800 | yes | basis | 652 | 8.882e-16 |
| `fused_t111` | 20x1220 | 1220 | yes | basis | 1222 | 1.332e-15 |
| `fused_t112` | 30x1230 | 1230 | yes | basis | 1232 | 1.776e-15 |
| `fused_t113` | 600x650 | 1800 | yes | basis | 652 | 8.882e-16 |
| `fused_t114` | 20x1220 | 1220 | yes | basis | 1222 | 8.882e-16 |
| `fused_t115` | 30x1230 | 1230 | yes | basis | 1232 | 1.776e-15 |
| `fused_t116` | 600x650 | 1800 | yes | basis | 652 | 1.776e-15 |
| `fused_t117` | 20x1220 | 1220 | yes | basis | 1222 | 8.882e-16 |
| `fused_t118` | 30x1230 | 1230 | yes | basis | 1232 | 1.776e-15 |
| `fused_t119` | 600x650 | 1800 | yes | basis | 652 | 8.882e-16 |
| `fused_t120` | 20x1220 | 1220 | yes | basis | 1222 | 8.882e-16 |
| `fused_t121` | 30x1230 | 1230 | yes | basis | 1232 | 1.332e-15 |
| `fused_t122` | 600x650 | 1800 | yes | basis | 652 | 1.776e-15 |
| `fused_t123` | 20x1220 | 1220 | yes | basis | 1222 | 1.776e-15 |
| `fused_t124` | 30x1230 | 1230 | yes | basis | 1232 | 1.776e-15 |
| `fused_t125` | 600x650 | 1800 | yes | basis | 652 | 8.882e-16 |
| `fused_t126` | 20x1220 | 1220 | yes | basis | 1222 | 8.882e-16 |
| `fused_t127` | 30x1230 | 1230 | yes | basis | 1232 | 1.776e-15 |
| `fused_t128` | 600x650 | 1800 | yes | basis | 652 | 8.882e-16 |
| `fused_t129` | 20x1220 | 1220 | yes | basis | 1222 | 1.776e-15 |
| `fused_t130` | 30x1230 | 1230 | yes | basis | 1232 | 1.332e-15 |
| `fused_t131` | 600x650 | 1800 | yes | basis | 652 | 8.882e-16 |
| `fused_t132` | 20x1220 | 1220 | yes | basis | 1222 | 3.553e-15 |
| `fused_t133` | 30x1230 | 1230 | yes | basis | 1232 | 1.776e-15 |
| `fused_t134` | 600x650 | 1800 | yes | basis | 652 | 8.882e-16 |
| `fused_t135` | 20x1220 | 1220 | yes | basis | 1222 | 1.776e-15 |
| `fused_t136` | 30x1230 | 1230 | yes | basis | 1232 | 8.882e-16 |
| `fused_t137` | 600x650 | 1800 | yes | basis | 652 | 8.882e-16 |
| `fused_t138` | 20x1220 | 1220 | yes | basis | 1222 | 1.776e-15 |
| `fused_t139` | 30x1230 | 1230 | yes | basis | 1232 | 1.332e-15 |
| `fused_t140` | 600x650 | 1800 | yes | basis | 652 | 8.882e-16 |
| `fused_t141` | 20x1220 | 1220 | yes | basis | 1222 | 1.776e-15 |
| `fused_t142` | 30x1230 | 1230 | yes | basis | 1232 | 1.332e-15 |
| `fused_t143` | 600x650 | 1800 | yes | basis | 652 | 1.776e-15 |
| `fused_t144` | 20x1220 | 1220 | yes | basis | 1222 | 2.665e-15 |
| `fused_t145` | 30x1230 | 1230 | yes | basis | 1232 | 8.882e-16 |
| `fused_t146` | 600x650 | 1800 | yes | basis | 652 | 8.882e-16 |
| `fused_t147` | 20x1220 | 1220 | yes | basis | 1222 | 2.220e-15 |
| `fused_t148` | 30x1230 | 1230 | yes | basis | 1232 | 8.882e-16 |
| `fused_su` | 650x650 | 650 | yes | basis | 652 | 0.000e+00 |

### Rule violations

_None: the collected rewrite passed the rules and was kept._

## Blocked by rules

Optimizations the agent considered but did not make because a rule forbade them, in its own words.

| round | idea | rule | expected gain |
|---|---|---|---|
| 1 | Fusing the bias initialization and clip/relu activation into the fused kernel, which would save two passes over 600 entries for each of the 50 steps | the bias (t0_c) and the bounds (t0_lo/hi) are not member data of `fom_fused_*`, and the fused signature is fixed in the headers | ~5-10% |
| 1 | Merging consecutive steps into one routine to keep intermediates in registers | `fom_iterate`/`fom_step_*` not exposed for this spec and headers fixed | unknown |
| 2 | Fold the `t0_c` init and the clip into the fused 600-row routine (one pass instead of three) | a fused routine may read only its member matrices' data, so it cannot read `t0_c`, `t0_lo` or `t0_hi`; there is no `fom_step_<block>` entry point for these blocks | about 5–10% |
| 2 | Store `lin_step_30` coefficients transposed, built once at setup, for contiguous vector loads | `fom_setup` may not probe the matrices; precomputed tables are allowed only as `static const` literal tables inside each of about 49 fused routines, which was not feasible in this round's time | unknown, probably 3–5% |

## What the agent changed

```diff
--- src/fom_solver.c (translated)
+++ src/fom_solver.c (optimized)
@@ -1,6 +1,7 @@
 #include "fom_solver.h"
 #include "fom_data.h"
 #include "fom_ops.h"
+#include "fom_profile.h"
 
 static void reset_state(FomWorkspace *work) {
     fom_int i;
@@ -35,555 +36,1153 @@
     for (i = 0; i < 30; ++i) pp7[i] = 0.0;
     fom_apply_t7_p_p0(work->params.p0, pp7);
     /* t0 */
+    {
+    FOM_PROF_BEGIN(steps_600);
     for (i = 0; i < 600; ++i) work->t0[i] = FOM_MAT_t0_c[i];
     fom_apply_t0_state(work->state, work->t0);
     for (i = 0; i < 600; ++i) { fom_float v = work->t0[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t0[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
+    FOM_PROF_END(steps_600);
+    }
     /* t1 */
+    {
+    FOM_PROF_BEGIN(steps_20_relu);
     for (i = 0; i < 20; ++i) work->t1[i] = FOM_MAT_t1_c[i];
     fom_fused_t1(work->state, work->t0, work->t1);
     for (i = 0; i < 20; ++i) if (work->t1[i] < 0.0) work->t1[i] = 0.0;
+    FOM_PROF_END(steps_20_relu);
+    }
     /* t2 */
+    {
+    FOM_PROF_BEGIN(steps_600);
     for (i = 0; i < 600; ++i) work->t2[i] = FOM_MAT_t0_c[i];
     fom_apply_t2_p_p0(work->params.p0, work->t2);
     fom_fused_t2(work->state, work->t0, work->t1, work->t2);
     for (i = 0; i < 600; ++i) { fom_float v = work->t2[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t2[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
+    FOM_PROF_END(steps_600);
+    }
     /* t3 */
+    {
+    FOM_PROF_BEGIN(steps_20_relu);
     for (i = 0; i < 20; ++i) work->t3[i] = FOM_MAT_t1_c[i];
     fom_fused_t3(work->t0, work->t1, work->t2, work->t3);
     for (i = 0; i < 20; ++i) if (work->t3[i] < 0.0) work->t3[i] = 0.0;
+    FOM_PROF_END(steps_20_relu);
+    }
     /* t4 */
+    {
+    FOM_PROF_BEGIN(steps_30_lin);
     for (i = 0; i < 30; ++i) work->t4[i] = 0.0;
     fom_apply_t4_p_p0(work->params.p0, work->t4);
     fom_fused_t4(work->state, work->t0, work->t2, work->t4);
+    FOM_PROF_END(steps_30_lin);
+    }
     /* t5 */
+    {
+    FOM_PROF_BEGIN(steps_600);
     for (i = 0; i < 600; ++i) work->t5[i] = FOM_MAT_t0_c[i];
     fom_fused_t5(work->t2, work->t3, work->t4, work->t5);
     for (i = 0; i < 600; ++i) { fom_float v = work->t5[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t5[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
+    FOM_PROF_END(steps_600);
+    }
     /* t6 */
+    {
+    FOM_PROF_BEGIN(steps_20_relu);
     for (i = 0; i < 20; ++i) work->t6[i] = FOM_MAT_t1_c[i];
     fom_fused_t6(work->t2, work->t3, work->t5, work->t6);
     for (i = 0; i < 20; ++i) if (work->t6[i] < 0.0) work->t6[i] = 0.0;
+    FOM_PROF_END(steps_20_relu);
+    }
     /* t7 */
+    {
+    FOM_PROF_BEGIN(steps_30_lin);
     for (i = 0; i < 30; ++i) work->t7[i] = pp7[i];
     fom_fused_t7(work->t2, work->t4, work->t5, work->t7);
+    FOM_PROF_END(steps_30_lin);
+    }
     /* t8 */
+    {
+    FOM_PROF_BEGIN(steps_600);
     for (i = 0; i < 600; ++i) work->t8[i] = FOM_MAT_t0_c[i];
     fom_fused_t8(work->t5, work->t6, work->t7, work->t8);
     for (i = 0; i < 600; ++i) { fom_float v = work->t8[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t8[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
+    FOM_PROF_END(steps_600);
+    }
     /* t9 */
+    {
+    FOM_PROF_BEGIN(steps_20_relu);
     for (i = 0; i < 20; ++i) work->t9[i] = FOM_MAT_t1_c[i];
     fom_fused_t9(work->t5, work->t6, work->t8, work->t9);
     for (i = 0; i < 20; ++i) if (work->t9[i] < 0.0) work->t9[i] = 0.0;
+    FOM_PROF_END(steps_20_relu);
+    }
     /* t10 */
+    {
+    FOM_PROF_BEGIN(steps_30_lin);
     for (i = 0; i < 30; ++i) work->t10[i] = pp7[i];
     fom_fused_t10(work->t5, work->t7, work->t8, work->t10);
+    FOM_PROF_END(steps_30_lin);
+    }
     /* t11 */
+    {
+    FOM_PROF_BEGIN(steps_600);
     for (i = 0; i < 600; ++i) work->t11[i] = FOM_MAT_t0_c[i];
     fom_fused_t11(work->t8, work->t9, work->t10, work->t11);
     for (i = 0; i < 600; ++i) { fom_float v = work->t11[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t11[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
+    FOM_PROF_END(steps_600);
+    }
     /* t12 */
+    {
+    FOM_PROF_BEGIN(steps_20_relu);
     for (i = 0; i < 20; ++i) work->t12[i] = FOM_MAT_t1_c[i];
     fom_fused_t12(work->t8, work->t9, work->t11, work->t12);
     for (i = 0; i < 20; ++i) if (work->t12[i] < 0.0) work->t12[i] = 0.0;
+    FOM_PROF_END(steps_20_relu);
+    }
     /* t13 */
+    {
+    FOM_PROF_BEGIN(steps_30_lin);
     for (i = 0; i < 30; ++i) work->t13[i] = pp7[i];
     fom_fused_t13(work->t8, work->t10, work->t11, work->t13);
+    FOM_PROF_END(steps_30_lin);
+    }
     /* t14 */
+    {
+    FOM_PROF_BEGIN(steps_600);
     for (i = 0; i < 600; ++i) work->t14[i] = FOM_MAT_t0_c[i];
     fom_fused_t14(work->t11, work->t12, work->t13, work->t14);
     for (i = 0; i < 600; ++i) { fom_float v = work->t14[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t14[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
+    FOM_PROF_END(steps_600);
+    }
     /* t15 */
+    {
+    FOM_PROF_BEGIN(steps_20_relu);
     for (i = 0; i < 20; ++i) work->t15[i] = FOM_MAT_t1_c[i];
     fom_fused_t15(work->t11, work->t12, work->t14, work->t15);
     for (i = 0; i < 20; ++i) if (work->t15[i] < 0.0) work->t15[i] = 0.0;
+    FOM_PROF_END(steps_20_relu);
+    }
     /* t16 */
+    {
+    FOM_PROF_BEGIN(steps_30_lin);
     for (i = 0; i < 30; ++i) work->t16[i] = pp7[i];
     fom_fused_t16(work->t11, work->t13, work->t14, work->t16);
+    FOM_PROF_END(steps_30_lin);
+    }
     /* t17 */
+    {
+    FOM_PROF_BEGIN(steps_600);
     for (i = 0; i < 600; ++i) work->t17[i] = FOM_MAT_t0_c[i];
     fom_fused_t17(work->t14, work->t15, work->t16, work->t17);
     for (i = 0; i < 600; ++i) { fom_float v = work->t17[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t17[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
+    FOM_PROF_END(steps_600);
+    }
     /* t18 */
+    {
+    FOM_PROF_BEGIN(steps_20_relu);
     for (i = 0; i < 20; ++i) work->t18[i] = FOM_MAT_t1_c[i];
     fom_fused_t18(work->t14, work->t15, work->t17, work->t18);
     for (i = 0; i < 20; ++i) if (work->t18[i] < 0.0) work->t18[i] = 0.0;
+    FOM_PROF_END(steps_20_relu);
+    }
     /* t19 */
+    {
+    FOM_PROF_BEGIN(steps_30_lin);
     for (i = 0; i < 30; ++i) work->t19[i] = pp7[i];
     fom_fused_t19(work->t14, work->t16, work->t17, work->t19);
+    FOM_PROF_END(steps_30_lin);
+    }
     /* t20 */
+    {
+    FOM_PROF_BEGIN(steps_600);
     for (i = 0; i < 600; ++i) work->t20[i] = FOM_MAT_t0_c[i];
     fom_fused_t20(work->t17, work->t18, work->t19, work->t20);
     for (i = 0; i < 600; ++i) { fom_float v = work->t20[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t20[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
+    FOM_PROF_END(steps_600);
+    }
     /* t21 */
+    {
+    FOM_PROF_BEGIN(steps_20_relu);
     for (i = 0; i < 20; ++i) work->t21[i] = FOM_MAT_t1_c[i];
     fom_fused_t21(work->t17, work->t18, work->t20, work->t21);
     for (i = 0; i < 20; ++i) if (work->t21[i] < 0.0) work->t21[i] = 0.0;
+    FOM_PROF_END(steps_20_relu);
+    }
     /* t22 */
+    {
+    FOM_PROF_BEGIN(steps_30_lin);
     for (i = 0; i < 30; ++i) work->t22[i] = pp7[i];
     fom_fused_t22(work->t17, work->t19, work->t20, work->t22);
+    FOM_PROF_END(steps_30_lin);
+    }
     /* t23 */
+    {
+    FOM_PROF_BEGIN(steps_600);
     for (i = 0; i < 600; ++i) work->t23[i] = FOM_MAT_t0_c[i];
     fom_fused_t23(work->t20, work->t21, work->t22, work->t23);
     for (i = 0; i < 600; ++i) { fom_float v = work->t23[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t23[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
+    FOM_PROF_END(steps_600);
+    }
     /* t24 */
+    {
+    FOM_PROF_BEGIN(steps_20_relu);
     for (i = 0; i < 20; ++i) work->t24[i] = FOM_MAT_t1_c[i];
     fom_fused_t24(work->t20, work->t21, work->t23, work->t24);
     for (i = 0; i < 20; ++i) if (work->t24[i] < 0.0) work->t24[i] = 0.0;
+    FOM_PROF_END(steps_20_relu);
+    }
     /* t25 */
+    {
+    FOM_PROF_BEGIN(steps_30_lin);
     for (i = 0; i < 30; ++i) work->t25[i] = pp7[i];
     fom_fused_t25(work->t20, work->t22, work->t23, work->t25);
+    FOM_PROF_END(steps_30_lin);
+    }
     /* t26 */
+    {
+    FOM_PROF_BEGIN(steps_600);
     for (i = 0; i < 600; ++i) work->t26[i] = FOM_MAT_t0_c[i];
     fom_fused_t26(work->t23, work->t24, work->t25, work->t26);
     for (i = 0; i < 600; ++i) { fom_float v = work->t26[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t26[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
+    FOM_PROF_END(steps_600);
+    }
     /* t27 */
+    {
+    FOM_PROF_BEGIN(steps_20_relu);
     for (i = 0; i < 20; ++i) work->t27[i] = FOM_MAT_t1_c[i];
     fom_fused_t27(work->t23, work->t24, work->t26, work->t27);
     for (i = 0; i < 20; ++i) if (work->t27[i] < 0.0) work->t27[i] = 0.0;
+    FOM_PROF_END(steps_20_relu);
+    }
     /* t28 */
+    {
+    FOM_PROF_BEGIN(steps_30_lin);
     for (i = 0; i < 30; ++i) work->t28[i] = pp7[i];
     fom_fused_t28(work->t23, work->t25, work->t26, work->t28);
+    FOM_PROF_END(steps_30_lin);
+    }
     /* t29 */
+    {
+    FOM_PROF_BEGIN(steps_600);
     for (i = 0; i < 600; ++i) work->t29[i] = FOM_MAT_t0_c[i];
     fom_fused_t29(work->t26, work->t27, work->t28, work->t29);
     for (i = 0; i < 600; ++i) { fom_float v = work->t29[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t29[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
+    FOM_PROF_END(steps_600);
+    }
     /* t30 */
+    {
+    FOM_PROF_BEGIN(steps_20_relu);
     for (i = 0; i < 20; ++i) work->t30[i] = FOM_MAT_t1_c[i];
     fom_fused_t30(work->t26, work->t27, work->t29, work->t30);
     for (i = 0; i < 20; ++i) if (work->t30[i] < 0.0) work->t30[i] = 0.0;
+    FOM_PROF_END(steps_20_relu);
+    }
     /* t31 */
+    {
+    FOM_PROF_BEGIN(steps_30_lin);
     for (i = 0; i < 30; ++i) work->t31[i] = pp7[i];
     fom_fused_t31(work->t26, work->t28, work->t29, work->t31);
+    FOM_PROF_END(steps_30_lin);
+    }
     /* t32 */
+    {
+    FOM_PROF_BEGIN(steps_600);
     for (i = 0; i < 600; ++i) work->t32[i] = FOM_MAT_t0_c[i];
     fom_fused_t32(work->t29, work->t30, work->t31, work->t32);
     for (i = 0; i < 600; ++i) { fom_float v = work->t32[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t32[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
+    FOM_PROF_END(steps_600);
+    }
     /* t33 */
+    {
+    FOM_PROF_BEGIN(steps_20_relu);
     for (i = 0; i < 20; ++i) work->t33[i] = FOM_MAT_t1_c[i];
     fom_fused_t33(work->t29, work->t30, work->t32, work->t33);
     for (i = 0; i < 20; ++i) if (work->t33[i] < 0.0) work->t33[i] = 0.0;
+    FOM_PROF_END(steps_20_relu);
+    }
     /* t34 */
+    {
+    FOM_PROF_BEGIN(steps_30_lin);
     for (i = 0; i < 30; ++i) work->t34[i] = pp7[i];
     fom_fused_t34(work->t29, work->t31, work->t32, work->t34);
+    FOM_PROF_END(steps_30_lin);
+    }
     /* t35 */
+    {
+    FOM_PROF_BEGIN(steps_600);
     for (i = 0; i < 600; ++i) work->t35[i] = FOM_MAT_t0_c[i];
     fom_fused_t35(work->t32, work->t33, work->t34, work->t35);
     for (i = 0; i < 600; ++i) { fom_float v = work->t35[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t35[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
+    FOM_PROF_END(steps_600);
+    }
     /* t36 */
+    {
+    FOM_PROF_BEGIN(steps_20_relu);
     for (i = 0; i < 20; ++i) work->t36[i] = FOM_MAT_t1_c[i];
     fom_fused_t36(work->t32, work->t33, work->t35, work->t36);
     for (i = 0; i < 20; ++i) if (work->t36[i] < 0.0) work->t36[i] = 0.0;
+    FOM_PROF_END(steps_20_relu);
+    }
     /* t37 */
+    {
+    FOM_PROF_BEGIN(steps_30_lin);
     for (i = 0; i < 30; ++i) work->t37[i] = pp7[i];
     fom_fused_t37(work->t32, work->t34, work->t35, work->t37);
+    FOM_PROF_END(steps_30_lin);
+    }
     /* t38 */
+    {
+    FOM_PROF_BEGIN(steps_600);
     for (i = 0; i < 600; ++i) work->t38[i] = FOM_MAT_t0_c[i];
     fom_fused_t38(work->t35, work->t36, work->t37, work->t38);
     for (i = 0; i < 600; ++i) { fom_float v = work->t38[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t38[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
+    FOM_PROF_END(steps_600);
+    }
     /* t39 */
+    {
+    FOM_PROF_BEGIN(steps_20_relu);
     for (i = 0; i < 20; ++i) work->t39[i] = FOM_MAT_t1_c[i];
     fom_fused_t39(work->t35, work->t36, work->t38, work->t39);
     for (i = 0; i < 20; ++i) if (work->t39[i] < 0.0) work->t39[i] = 0.0;
+    FOM_PROF_END(steps_20_relu);
+    }
     /* t40 */
+    {
+    FOM_PROF_BEGIN(steps_30_lin);
     for (i = 0; i < 30; ++i) work->t40[i] = pp7[i];
     fom_fused_t40(work->t35, work->t37, work->t38, work->t40);
+    FOM_PROF_END(steps_30_lin);
+    }
     /* t41 */
+    {
+    FOM_PROF_BEGIN(steps_600);
     for (i = 0; i < 600; ++i) work->t41[i] = FOM_MAT_t0_c[i];
     fom_fused_t41(work->t38, work->t39, work->t40, work->t41);
     for (i = 0; i < 600; ++i) { fom_float v = work->t41[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t41[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
+    FOM_PROF_END(steps_600);
+    }
     /* t42 */
+    {
+    FOM_PROF_BEGIN(steps_20_relu);
     for (i = 0; i < 20; ++i) work->t42[i] = FOM_MAT_t1_c[i];
     fom_fused_t42(work->t38, work->t39, work->t41, work->t42);
     for (i = 0; i < 20; ++i) if (work->t42[i] < 0.0) work->t42[i] = 0.0;
+    FOM_PROF_END(steps_20_relu);
+    }
     /* t43 */
+    {
+    FOM_PROF_BEGIN(steps_30_lin);
     for (i = 0; i < 30; ++i) work->t43[i] = pp7[i];
     fom_fused_t43(work->t38, work->t40, work->t41, work->t43);
+    FOM_PROF_END(steps_30_lin);
+    }
     /* t44 */
+    {
+    FOM_PROF_BEGIN(steps_600);
     for (i = 0; i < 600; ++i) work->t44[i] = FOM_MAT_t0_c[i];
     fom_fused_t44(work->t41, work->t42, work->t43, work->t44);
     for (i = 0; i < 600; ++i) { fom_float v = work->t44[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t44[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
+    FOM_PROF_END(steps_600);
+    }
     /* t45 */
+    {
+    FOM_PROF_BEGIN(steps_20_relu);
     for (i = 0; i < 20; ++i) work->t45[i] = FOM_MAT_t1_c[i];
     fom_fused_t45(work->t41, work->t42, work->t44, work->t45);
     for (i = 0; i < 20; ++i) if (work->t45[i] < 0.0) work->t45[i] = 0.0;
+    FOM_PROF_END(steps_20_relu);
+    }
     /* t46 */
+    {
+    FOM_PROF_BEGIN(steps_30_lin);
     for (i = 0; i < 30; ++i) work->t46[i] = pp7[i];
     fom_fused_t46(work->t41, work->t43, work->t44, work->t46);
+    FOM_PROF_END(steps_30_lin);
+    }
     /* t47 */
+    {
+    FOM_PROF_BEGIN(steps_600);
     for (i = 0; i < 600; ++i) work->t47[i] = FOM_MAT_t0_c[i];
     fom_fused_t47(work->t44, work->t45, work->t46, work->t47);
     for (i = 0; i < 600; ++i) { fom_float v = work->t47[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t47[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
+    FOM_PROF_END(steps_600);
+    }
     /* t48 */
+    {
+    FOM_PROF_BEGIN(steps_20_relu);
     for (i = 0; i < 20; ++i) work->t48[i] = FOM_MAT_t1_c[i];
     fom_fused_t48(work->t44, work->t45, work->t47, work->t48);
     for (i = 0; i < 20; ++i) if (work->t48[i] < 0.0) work->t48[i] = 0.0;
+    FOM_PROF_END(steps_20_relu);
+    }
     /* t49 */
+    {
+    FOM_PROF_BEGIN(steps_30_lin);
     for (i = 0; i < 30; ++i) work->t49[i] = pp7[i];
     fom_fused_t49(work->t44, work->t46, work->t47, work->t49);
+    FOM_PROF_END(steps_30_lin);
+    }
     /* t50 */
+    {
+    FOM_PROF_BEGIN(steps_600);
     for (i = 0; i < 600; ++i) work->t50[i] = FOM_MAT_t0_c[i];
     fom_fused_t50(work->t47, work->t48, work->t49, work->t50);
     for (i = 0; i < 600; ++i) { fom_float v = work->t50[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t50[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
+    FOM_PROF_END(steps_600);
+    }
     /* t51 */
+    {
+    FOM_PROF_BEGIN(steps_20_relu);
     for (i = 0; i < 20; ++i) work->t51[i] = FOM_MAT_t1_c[i];
     fom_fused_t51(work->t47, work->t48, work->t50, work->t51);
     for (i = 0; i < 20; ++i) if (work->t51[i] < 0.0) work->t51[i] = 0.0;
+    FOM_PROF_END(steps_20_relu);
+    }
     /* t52 */
+    {
+    FOM_PROF_BEGIN(steps_30_lin);
     for (i = 0; i < 30; ++i) work->t52[i] = pp7[i];
     fom_fused_t52(work->t47, work->t49, work->t50, work->t52);
+    FOM_PROF_END(steps_30_lin);
+    }
     /* t53 */
+    {
+    FOM_PROF_BEGIN(steps_600);
     for (i = 0; i < 600; ++i) work->t53[i] = FOM_MAT_t0_c[i];
     fom_fused_t53(work->t50, work->t51, work->t52, work->t53);
     for (i = 0; i < 600; ++i) { fom_float v = work->t53[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t53[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
+    FOM_PROF_END(steps_600);
+    }
     /* t54 */
+    {
+    FOM_PROF_BEGIN(steps_20_relu);
     for (i = 0; i < 20; ++i) work->t54[i] = FOM_MAT_t1_c[i];
     fom_fused_t54(work->t50, work->t51, work->t53, work->t54);
     for (i = 0; i < 20; ++i) if (work->t54[i] < 0.0) work->t54[i] = 0.0;
+    FOM_PROF_END(steps_20_relu);
+    }
     /* t55 */
+    {
+    FOM_PROF_BEGIN(steps_30_lin);
     for (i = 0; i < 30; ++i) work->t55[i] = pp7[i];
     fom_fused_t55(work->t50, work->t52, work->t53, work->t55);
+    FOM_PROF_END(steps_30_lin);
+    }
     /* t56 */
+    {
+    FOM_PROF_BEGIN(steps_600);
     for (i = 0; i < 600; ++i) work->t56[i] = FOM_MAT_t0_c[i];
     fom_fused_t56(work->t53, work->t54, work->t55, work->t56);
     for (i = 0; i < 600; ++i) { fom_float v = work->t56[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t56[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
+    FOM_PROF_END(steps_600);
+    }
     /* t57 */
+    {
+    FOM_PROF_BEGIN(steps_20_relu);
     for (i = 0; i < 20; ++i) work->t57[i] = FOM_MAT_t1_c[i];
     fom_fused_t57(work->t53, work->t54, work->t56, work->t57);
     for (i = 0; i < 20; ++i) if (work->t57[i] < 0.0) work->t57[i] = 0.0;
+    FOM_PROF_END(steps_20_relu);
+    }
     /* t58 */
+    {
+    FOM_PROF_BEGIN(steps_30_lin);
     for (i = 0; i < 30; ++i) work->t58[i] = pp7[i];
     fom_fused_t58(work->t53, work->t55, work->t56, work->t58);
+    FOM_PROF_END(steps_30_lin);
+    }
     /* t59 */
+    {
+    FOM_PROF_BEGIN(steps_600);
     for (i = 0; i < 600; ++i) work->t59[i] = FOM_MAT_t0_c[i];
     fom_fused_t59(work->t56, work->t57, work->t58, work->t59);
     for (i = 0; i < 600; ++i) { fom_float v = work->t59[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t59[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
+    FOM_PROF_END(steps_600);
+    }
     /* t60 */
+    {
+    FOM_PROF_BEGIN(steps_20_relu);
     for (i = 0; i < 20; ++i) work->t60[i] = FOM_MAT_t1_c[i];
     fom_fused_t60(work->t56, work->t57, work->t59, work->t60);
     for (i = 0; i < 20; ++i) if (work->t60[i] < 0.0) work->t60[i] = 0.0;
+    FOM_PROF_END(steps_20_relu);
+    }
     /* t61 */
+    {
+    FOM_PROF_BEGIN(steps_30_lin);
     for (i = 0; i < 30; ++i) work->t61[i] = pp7[i];
     fom_fused_t61(work->t56, work->t58, work->t59, work->t61);
+    FOM_PROF_END(steps_30_lin);
+    }
     /* t62 */
+    {
+    FOM_PROF_BEGIN(steps_600);
     for (i = 0; i < 600; ++i) work->t62[i] = FOM_MAT_t0_c[i];
     fom_fused_t62(work->t59, work->t60, work->t61, work->t62);
     for (i = 0; i < 600; ++i) { fom_float v = work->t62[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t62[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
+    FOM_PROF_END(steps_600);
+    }
     /* t63 */
+    {
+    FOM_PROF_BEGIN(steps_20_relu);
     for (i = 0; i < 20; ++i) work->t63[i] = FOM_MAT_t1_c[i];
     fom_fused_t63(work->t59, work->t60, work->t62, work->t63);
     for (i = 0; i < 20; ++i) if (work->t63[i] < 0.0) work->t63[i] = 0.0;
+    FOM_PROF_END(steps_20_relu);
+    }
     /* t64 */
+    {
+    FOM_PROF_BEGIN(steps_30_lin);
     for (i = 0; i < 30; ++i) work->t64[i] = pp7[i];
     fom_fused_t64(work->t59, work->t61, work->t62, work->t64);
+    FOM_PROF_END(steps_30_lin);
+    }
     /* t65 */
+    {
+    FOM_PROF_BEGIN(steps_600);
     for (i = 0; i < 600; ++i) work->t65[i] = FOM_MAT_t0_c[i];
     fom_fused_t65(work->t62, work->t63, work->t64, work->t65);
     for (i = 0; i < 600; ++i) { fom_float v = work->t65[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t65[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
+    FOM_PROF_END(steps_600);
+    }
     /* t66 */
+    {
+    FOM_PROF_BEGIN(steps_20_relu);
     for (i = 0; i < 20; ++i) work->t66[i] = FOM_MAT_t1_c[i];
     fom_fused_t66(work->t62, work->t63, work->t65, work->t66);
     for (i = 0; i < 20; ++i) if (work->t66[i] < 0.0) work->t66[i] = 0.0;
+    FOM_PROF_END(steps_20_relu);
+    }
     /* t67 */
+    {
+    FOM_PROF_BEGIN(steps_30_lin);
     for (i = 0; i < 30; ++i) work->t67[i] = pp7[i];
     fom_fused_t67(work->t62, work->t64, work->t65, work->t67);
+    FOM_PROF_END(steps_30_lin);
+    }
     /* t68 */
+    {
+    FOM_PROF_BEGIN(steps_600);
     for (i = 0; i < 600; ++i) work->t68[i] = FOM_MAT_t0_c[i];
     fom_fused_t68(work->t65, work->t66, work->t67, work->t68);
     for (i = 0; i < 600; ++i) { fom_float v = work->t68[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t68[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
+    FOM_PROF_END(steps_600);
+    }
     /* t69 */
+    {
+    FOM_PROF_BEGIN(steps_20_relu);
     for (i = 0; i < 20; ++i) work->t69[i] = FOM_MAT_t1_c[i];
     fom_fused_t69(work->t65, work->t66, work->t68, work->t69);
     for (i = 0; i < 20; ++i) if (work->t69[i] < 0.0) work->t69[i] = 0.0;
+    FOM_PROF_END(steps_20_relu);
+    }
     /* t70 */
+    {
+    FOM_PROF_BEGIN(steps_30_lin);
     for (i = 0; i < 30; ++i) work->t70[i] = pp7[i];
     fom_fused_t70(work->t65, work->t67, work->t68, work->t70);
+    FOM_PROF_END(steps_30_lin);
+    }
     /* t71 */
+    {
+    FOM_PROF_BEGIN(steps_600);
     for (i = 0; i < 600; ++i) work->t71[i] = FOM_MAT_t0_c[i];
     fom_fused_t71(work->t68, work->t69, work->t70, work->t71);
     for (i = 0; i < 600; ++i) { fom_float v = work->t71[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t71[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
+    FOM_PROF_END(steps_600);
+    }
     /* t72 */
+    {
+    FOM_PROF_BEGIN(steps_20_relu);
     for (i = 0; i < 20; ++i) work->t72[i] = FOM_MAT_t1_c[i];
     fom_fused_t72(work->t68, work->t69, work->t71, work->t72);
     for (i = 0; i < 20; ++i) if (work->t72[i] < 0.0) work->t72[i] = 0.0;
+    FOM_PROF_END(steps_20_relu);
+    }
     /* t73 */
+    {
+    FOM_PROF_BEGIN(steps_30_lin);
     for (i = 0; i < 30; ++i) work->t73[i] = pp7[i];
     fom_fused_t73(work->t68, work->t70, work->t71, work->t73);
+    FOM_PROF_END(steps_30_lin);
+    }
     /* t74 */
+    {
+    FOM_PROF_BEGIN(steps_600);
     for (i = 0; i < 600; ++i) work->t74[i] = FOM_MAT_t0_c[i];
     fom_fused_t74(work->t71, work->t72, work->t73, work->t74);
     for (i = 0; i < 600; ++i) { fom_float v = work->t74[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t74[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
+    FOM_PROF_END(steps_600);
+    }
     /* t75 */
+    {
+    FOM_PROF_BEGIN(steps_20_relu);
     for (i = 0; i < 20; ++i) work->t75[i] = FOM_MAT_t1_c[i];
     fom_fused_t75(work->t71, work->t72, work->t74, work->t75);
     for (i = 0; i < 20; ++i) if (work->t75[i] < 0.0) work->t75[i] = 0.0;
+    FOM_PROF_END(steps_20_relu);
+    }
     /* t76 */
+    {
+    FOM_PROF_BEGIN(steps_30_lin);
     for (i = 0; i < 30; ++i) work->t76[i] = pp7[i];
     fom_fused_t76(work->t71, work->t73, work->t74, work->t76);
+    FOM_PROF_END(steps_30_lin);
+    }
     /* t77 */
+    {
+    FOM_PROF_BEGIN(steps_600);
     for (i = 0; i < 600; ++i) work->t77[i] = FOM_MAT_t0_c[i];
     fom_fused_t77(work->t74, work->t75, work->t76, work->t77);
     for (i = 0; i < 600; ++i) { fom_float v = work->t77[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t77[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
+    FOM_PROF_END(steps_600);
+    }
     /* t78 */
+    {
+    FOM_PROF_BEGIN(steps_20_relu);
     for (i = 0; i < 20; ++i) work->t78[i] = FOM_MAT_t1_c[i];
     fom_fused_t78(work->t74, work->t75, work->t77, work->t78);
     for (i = 0; i < 20; ++i) if (work->t78[i] < 0.0) work->t78[i] = 0.0;
+    FOM_PROF_END(steps_20_relu);
+    }
     /* t79 */
+    {
+    FOM_PROF_BEGIN(steps_30_lin);
     for (i = 0; i < 30; ++i) work->t79[i] = pp7[i];
     fom_fused_t79(work->t74, work->t76, work->t77, work->t79);
+    FOM_PROF_END(steps_30_lin);
+    }
     /* t80 */
+    {
+    FOM_PROF_BEGIN(steps_600);
     for (i = 0; i < 600; ++i) work->t80[i] = FOM_MAT_t0_c[i];
     fom_fused_t80(work->t77, work->t78, work->t79, work->t80);
     for (i = 0; i < 600; ++i) { fom_float v = work->t80[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t80[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
+    FOM_PROF_END(steps_600);
+    }
     /* t81 */
+    {
+    FOM_PROF_BEGIN(steps_20_relu);
     for (i = 0; i < 20; ++i) work->t81[i] = FOM_MAT_t1_c[i];
     fom_fused_t81(work->t77, work->t78, work->t80, work->t81);
     for (i = 0; i < 20; ++i) if (work->t81[i] < 0.0) work->t81[i] = 0.0;
+    FOM_PROF_END(steps_20_relu);
+    }
     /* t82 */
+    {
+    FOM_PROF_BEGIN(steps_30_lin);
     for (i = 0; i < 30; ++i) work->t82[i] = pp7[i];
     fom_fused_t82(work->t77, work->t79, work->t80, work->t82);
+    FOM_PROF_END(steps_30_lin);
+    }
     /* t83 */
+    {
+    FOM_PROF_BEGIN(steps_600);
     for (i = 0; i < 600; ++i) work->t83[i] = FOM_MAT_t0_c[i];
     fom_fused_t83(work->t80, work->t81, work->t82, work->t83);
     for (i = 0; i < 600; ++i) { fom_float v = work->t83[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t83[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
+    FOM_PROF_END(steps_600);
+    }
     /* t84 */
+    {
+    FOM_PROF_BEGIN(steps_20_relu);
     for (i = 0; i < 20; ++i) work->t84[i] = FOM_MAT_t1_c[i];
     fom_fused_t84(work->t80, work->t81, work->t83, work->t84);
     for (i = 0; i < 20; ++i) if (work->t84[i] < 0.0) work->t84[i] = 0.0;
+    FOM_PROF_END(steps_20_relu);
+    }
     /* t85 */
+    {
+    FOM_PROF_BEGIN(steps_30_lin);
     for (i = 0; i < 30; ++i) work->t85[i] = pp7[i];
     fom_fused_t85(work->t80, work->t82, work->t83, work->t85);
+    FOM_PROF_END(steps_30_lin);
+    }
     /* t86 */
+    {
+    FOM_PROF_BEGIN(steps_600);
     for (i = 0; i < 600; ++i) work->t86[i] = FOM_MAT_t0_c[i];
     fom_fused_t86(work->t83, work->t84, work->t85, work->t86);
     for (i = 0; i < 600; ++i) { fom_float v = work->t86[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t86[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
+    FOM_PROF_END(steps_600);
+    }
     /* t87 */
+    {
+    FOM_PROF_BEGIN(steps_20_relu);
     for (i = 0; i < 20; ++i) work->t87[i] = FOM_MAT_t1_c[i];
     fom_fused_t87(work->t83, work->t84, work->t86, work->t87);
     for (i = 0; i < 20; ++i) if (work->t87[i] < 0.0) work->t87[i] = 0.0;
+    FOM_PROF_END(steps_20_relu);
+    }
     /* t88 */
+    {
+    FOM_PROF_BEGIN(steps_30_lin);
     for (i = 0; i < 30; ++i) work->t88[i] = pp7[i];
     fom_fused_t88(work->t83, work->t85, work->t86, work->t88);
+    FOM_PROF_END(steps_30_lin);
+    }
     /* t89 */
+    {
+    FOM_PROF_BEGIN(steps_600);
     for (i = 0; i < 600; ++i) work->t89[i] = FOM_MAT_t0_c[i];
     fom_fused_t89(work->t86, work->t87, work->t88, work->t89);
     for (i = 0; i < 600; ++i) { fom_float v = work->t89[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t89[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
+    FOM_PROF_END(steps_600);
+    }
     /* t90 */
+    {
+    FOM_PROF_BEGIN(steps_20_relu);
     for (i = 0; i < 20; ++i) work->t90[i] = FOM_MAT_t1_c[i];
     fom_fused_t90(work->t86, work->t87, work->t89, work->t90);
     for (i = 0; i < 20; ++i) if (work->t90[i] < 0.0) work->t90[i] = 0.0;
+    FOM_PROF_END(steps_20_relu);
+    }
     /* t91 */
+    {
+    FOM_PROF_BEGIN(steps_30_lin);
     for (i = 0; i < 30; ++i) work->t91[i] = pp7[i];
     fom_fused_t91(work->t86, work->t88, work->t89, work->t91);
+    FOM_PROF_END(steps_30_lin);
+    }
     /* t92 */
+    {
+    FOM_PROF_BEGIN(steps_600);
     for (i = 0; i < 600; ++i) work->t92[i] = FOM_MAT_t0_c[i];
     fom_fused_t92(work->t89, work->t90, work->t91, work->t92);
     for (i = 0; i < 600; ++i) { fom_float v = work->t92[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t92[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
+    FOM_PROF_END(steps_600);
+    }
     /* t93 */
+    {
+    FOM_PROF_BEGIN(steps_20_relu);
     for (i = 0; i < 20; ++i) work->t93[i] = FOM_MAT_t1_c[i];
     fom_fused_t93(work->t89, work->t90, work->t92, work->t93);
     for (i = 0; i < 20; ++i) if (work->t93[i] < 0.0) work->t93[i] = 0.0;
+    FOM_PROF_END(steps_20_relu);
+    }
     /* t94 */
+    {
+    FOM_PROF_BEGIN(steps_30_lin);
     for (i = 0; i < 30; ++i) work->t94[i] = pp7[i];
     fom_fused_t94(work->t89, work->t91, work->t92, work->t94);
+    FOM_PROF_END(steps_30_lin);
+    }
     /* t95 */
+    {
+    FOM_PROF_BEGIN(steps_600);
     for (i = 0; i < 600; ++i) work->t95[i] = FOM_MAT_t0_c[i];
     fom_fused_t95(work->t92, work->t93, work->t94, work->t95);
     for (i = 0; i < 600; ++i) { fom_float v = work->t95[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t95[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
+    FOM_PROF_END(steps_600);
+    }
     /* t96 */
+    {
+    FOM_PROF_BEGIN(steps_20_relu);
     for (i = 0; i < 20; ++i) work->t96[i] = FOM_MAT_t1_c[i];
     fom_fused_t96(work->t92, work->t93, work->t95, work->t96);
     for (i = 0; i < 20; ++i) if (work->t96[i] < 0.0) work->t96[i] = 0.0;
+    FOM_PROF_END(steps_20_relu);
+    }
     /* t97 */
+    {
+    FOM_PROF_BEGIN(steps_30_lin);
     for (i = 0; i < 30; ++i) work->t97[i] = pp7[i];
     fom_fused_t97(work->t92, work->t94, work->t95, work->t97);
+    FOM_PROF_END(steps_30_lin);
+    }
     /* t98 */
+    {
+    FOM_PROF_BEGIN(steps_600);
     for (i = 0; i < 600; ++i) work->t98[i] = FOM_MAT_t0_c[i];
     fom_fused_t98(work->t95, work->t96, work->t97, work->t98);
     for (i = 0; i < 600; ++i) { fom_float v = work->t98[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t98[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
+    FOM_PROF_END(steps_600);
+    }
     /* t99 */
+    {
+    FOM_PROF_BEGIN(steps_20_relu);
     for (i = 0; i < 20; ++i) work->t99[i] = FOM_MAT_t1_c[i];
     fom_fused_t99(work->t95, work->t96, work->t98, work->t99);
     for (i = 0; i < 20; ++i) if (work->t99[i] < 0.0) work->t99[i] = 0.0;
+    FOM_PROF_END(steps_20_relu);
+    }
     /* t100 */
+    {
+    FOM_PROF_BEGIN(steps_30_lin);
     for (i = 0; i < 30; ++i) work->t100[i] = pp7[i];
     fom_fused_t100(work->t95, work->t97, work->t98, work->t100);
+    FOM_PROF_END(steps_30_lin);
+    }
     /* t101 */
+    {
+    FOM_PROF_BEGIN(steps_600);
     for (i = 0; i < 600; ++i) work->t101[i] = FOM_MAT_t0_c[i];
     fom_fused_t101(work->t98, work->t99, work->t100, work->t101);
     for (i = 0; i < 600; ++i) { fom_float v = work->t101[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t101[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
+    FOM_PROF_END(steps_600);
+    }
     /* t102 */
+    {
+    FOM_PROF_BEGIN(steps_20_relu);
     for (i = 0; i < 20; ++i) work->t102[i] = FOM_MAT_t1_c[i];
     fom_fused_t102(work->t98, work->t99, work->t101, work->t102);
     for (i = 0; i < 20; ++i) if (work->t102[i] < 0.0) work->t102[i] = 0.0;
+    FOM_PROF_END(steps_20_relu);
+    }
     /* t103 */
+    {
+    FOM_PROF_BEGIN(steps_30_lin);
     for (i = 0; i < 30; ++i) work->t103[i] = pp7[i];
     fom_fused_t103(work->t98, work->t100, work->t101, work->t103);
+    FOM_PROF_END(steps_30_lin);
+    }
     /* t104 */
+    {
+    FOM_PROF_BEGIN(steps_600);
     for (i = 0; i < 600; ++i) work->t104[i] = FOM_MAT_t0_c[i];
     fom_fused_t104(work->t101, work->t102, work->t103, work->t104);
     for (i = 0; i < 600; ++i) { fom_float v = work->t104[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t104[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
+    FOM_PROF_END(steps_600);
+    }
     /* t105 */
+    {
+    FOM_PROF_BEGIN(steps_20_relu);
     for (i = 0; i < 20; ++i) work->t105[i] = FOM_MAT_t1_c[i];
     fom_fused_t105(work->t101, work->t102, work->t104, work->t105);
     for (i = 0; i < 20; ++i) if (work->t105[i] < 0.0) work->t105[i] = 0.0;
+    FOM_PROF_END(steps_20_relu);
+    }
     /* t106 */
+    {
+    FOM_PROF_BEGIN(steps_30_lin);
     for (i = 0; i < 30; ++i) work->t106[i] = pp7[i];
     fom_fused_t106(work->t101, work->t103, work->t104, work->t106);
+    FOM_PROF_END(steps_30_lin);
+    }
     /* t107 */
+    {
+    FOM_PROF_BEGIN(steps_600);
     for (i = 0; i < 600; ++i) work->t107[i] = FOM_MAT_t0_c[i];
     fom_fused_t107(work->t104, work->t105, work->t106, work->t107);
     for (i = 0; i < 600; ++i) { fom_float v = work->t107[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t107[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
+    FOM_PROF_END(steps_600);
+    }
     /* t108 */
+    {
+    FOM_PROF_BEGIN(steps_20_relu);
     for (i = 0; i < 20; ++i) work->t108[i] = FOM_MAT_t1_c[i];
     fom_fused_t108(work->t104, work->t105, work->t107, work->t108);
     for (i = 0; i < 20; ++i) if (work->t108[i] < 0.0) work->t108[i] = 0.0;
+    FOM_PROF_END(steps_20_relu);
+    }
     /* t109 */
+    {
+    FOM_PROF_BEGIN(steps_30_lin);
     for (i = 0; i < 30; ++i) work->t109[i] = pp7[i];
     fom_fused_t109(work->t104, work->t106, work->t107, work->t109);
+    FOM_PROF_END(steps_30_lin);
+    }
     /* t110 */
+    {
+    FOM_PROF_BEGIN(steps_600);
     for (i = 0; i < 600; ++i) work->t110[i] = FOM_MAT_t0_c[i];
     fom_fused_t110(work->t107, work->t108, work->t109, work->t110);
     for (i = 0; i < 600; ++i) { fom_float v = work->t110[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t110[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
+    FOM_PROF_END(steps_600);
+    }
     /* t111 */
+    {
+    FOM_PROF_BEGIN(steps_20_relu);
     for (i = 0; i < 20; ++i) work->t111[i] = FOM_MAT_t1_c[i];
     fom_fused_t111(work->t107, work->t108, work->t110, work->t111);
     for (i = 0; i < 20; ++i) if (work->t111[i] < 0.0) work->t111[i] = 0.0;
+    FOM_PROF_END(steps_20_relu);
+    }
     /* t112 */
+    {
+    FOM_PROF_BEGIN(steps_30_lin);
     for (i = 0; i < 30; ++i) work->t112[i] = pp7[i];
     fom_fused_t112(work->t107, work->t109, work->t110, work->t112);
+    FOM_PROF_END(steps_30_lin);
+    }
     /* t113 */
+    {
+    FOM_PROF_BEGIN(steps_600);
     for (i = 0; i < 600; ++i) work->t113[i] = FOM_MAT_t0_c[i];
     fom_fused_t113(work->t110, work->t111, work->t112, work->t113);
     for (i = 0; i < 600; ++i) { fom_float v = work->t113[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t113[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
+    FOM_PROF_END(steps_600);
+    }
     /* t114 */
+    {
+    FOM_PROF_BEGIN(steps_20_relu);
     for (i = 0; i < 20; ++i) work->t114[i] = FOM_MAT_t1_c[i];
     fom_fused_t114(work->t110, work->t111, work->t113, work->t114);
     for (i = 0; i < 20; ++i) if (work->t114[i] < 0.0) work->t114[i] = 0.0;
+    FOM_PROF_END(steps_20_relu);
+    }
     /* t115 */
+    {
+    FOM_PROF_BEGIN(steps_30_lin);
     for (i = 0; i < 30; ++i) work->t115[i] = pp7[i];
     fom_fused_t115(work->t110, work->t112, work->t113, work->t115);
+    FOM_PROF_END(steps_30_lin);
+    }
     /* t116 */
+    {
+    FOM_PROF_BEGIN(steps_600);
     for (i = 0; i < 600; ++i) work->t116[i] = FOM_MAT_t0_c[i];
     fom_fused_t116(work->t113, work->t114, work->t115, work->t116);
     for (i = 0; i < 600; ++i) { fom_float v = work->t116[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t116[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
+    FOM_PROF_END(steps_600);
+    }
     /* t117 */
+    {
+    FOM_PROF_BEGIN(steps_20_relu);
     for (i = 0; i < 20; ++i) work->t117[i] = FOM_MAT_t1_c[i];
     fom_fused_t117(work->t113, work->t114, work->t116, work->t117);
     for (i = 0; i < 20; ++i) if (work->t117[i] < 0.0) work->t117[i] = 0.0;
+    FOM_PROF_END(steps_20_relu);
+    }
     /* t118 */
+    {
+    FOM_PROF_BEGIN(steps_30_lin);
     for (i = 0; i < 30; ++i) work->t118[i] = pp7[i];
     fom_fused_t118(work->t113, work->t115, work->t116, work->t118);
+    FOM_PROF_END(steps_30_lin);
+    }
     /* t119 */
+    {
+    FOM_PROF_BEGIN(steps_600);
     for (i = 0; i < 600; ++i) work->t119[i] = FOM_MAT_t0_c[i];
     fom_fused_t119(work->t116, work->t117, work->t118, work->t119);
     for (i = 0; i < 600; ++i) { fom_float v = work->t119[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t119[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
+    FOM_PROF_END(steps_600);
+    }
     /* t120 */
+    {
+    FOM_PROF_BEGIN(steps_20_relu);
     for (i = 0; i < 20; ++i) work->t120[i] = FOM_MAT_t1_c[i];
     fom_fused_t120(work->t116, work->t117, work->t119, work->t120);
     for (i = 0; i < 20; ++i) if (work->t120[i] < 0.0) work->t120[i] = 0.0;
+    FOM_PROF_END(steps_20_relu);
+    }
     /* t121 */
+    {
+    FOM_PROF_BEGIN(steps_30_lin);
     for (i = 0; i < 30; ++i) work->t121[i] = pp7[i];
     fom_fused_t121(work->t116, work->t118, work->t119, work->t121);
+    FOM_PROF_END(steps_30_lin);
+    }
     /* t122 */
+    {
+    FOM_PROF_BEGIN(steps_600);
     for (i = 0; i < 600; ++i) work->t122[i] = FOM_MAT_t0_c[i];
     fom_fused_t122(work->t119, work->t120, work->t121, work->t122);
     for (i = 0; i < 600; ++i) { fom_float v = work->t122[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t122[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
+    FOM_PROF_END(steps_600);
+    }
     /* t123 */
+    {
+    FOM_PROF_BEGIN(steps_20_relu);
     for (i = 0; i < 20; ++i) work->t123[i] = FOM_MAT_t1_c[i];
     fom_fused_t123(work->t119, work->t120, work->t122, work->t123);
     for (i = 0; i < 20; ++i) if (work->t123[i] < 0.0) work->t123[i] = 0.0;
+    FOM_PROF_END(steps_20_relu);
+    }
     /* t124 */
+    {
+    FOM_PROF_BEGIN(steps_30_lin);
     for (i = 0; i < 30; ++i) work->t124[i] = pp7[i];
     fom_fused_t124(work->t119, work->t121, work->t122, work->t124);
+    FOM_PROF_END(steps_30_lin);
+    }
     /* t125 */
+    {
+    FOM_PROF_BEGIN(steps_600);
     for (i = 0; i < 600; ++i) work->t125[i] = FOM_MAT_t0_c[i];
     fom_fused_t125(work->t122, work->t123, work->t124, work->t125);
     for (i = 0; i < 600; ++i) { fom_float v = work->t125[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t125[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
+    FOM_PROF_END(steps_600);
+    }
     /* t126 */
+    {
+    FOM_PROF_BEGIN(steps_20_relu);
     for (i = 0; i < 20; ++i) work->t126[i] = FOM_MAT_t1_c[i];
     fom_fused_t126(work->t122, work->t123, work->t125, work->t126);
     for (i = 0; i < 20; ++i) if (work->t126[i] < 0.0) work->t126[i] = 0.0;
+    FOM_PROF_END(steps_20_relu);
+    }
     /* t127 */
+    {
+    FOM_PROF_BEGIN(steps_30_lin);
     for (i = 0; i < 30; ++i) work->t127[i] = pp7[i];
     fom_fused_t127(work->t122, work->t124, work->t125, work->t127);
+    FOM_PROF_END(steps_30_lin);
+    }
     /* t128 */
+    {
+    FOM_PROF_BEGIN(steps_600);
     for (i = 0; i < 600; ++i) work->t128[i] = FOM_MAT_t0_c[i];
     fom_fused_t128(work->t125, work->t126, work->t127, work->t128);
     for (i = 0; i < 600; ++i) { fom_float v = work->t128[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t128[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
+    FOM_PROF_END(steps_600);
+    }
     /* t129 */
+    {
+    FOM_PROF_BEGIN(steps_20_relu);
     for (i = 0; i < 20; ++i) work->t129[i] = FOM_MAT_t1_c[i];
     fom_fused_t129(work->t125, work->t126, work->t128, work->t129);
     for (i = 0; i < 20; ++i) if (work->t129[i] < 0.0) work->t129[i] = 0.0;
+    FOM_PROF_END(steps_20_relu);
+    }
     /* t130 */
+    {
+    FOM_PROF_BEGIN(steps_30_lin);
     for (i = 0; i < 30; ++i) work->t130[i] = pp7[i];
     fom_fused_t130(work->t125, work->t127, work->t128, work->t130);
+    FOM_PROF_END(steps_30_lin);
+    }
     /* t131 */
+    {
+    FOM_PROF_BEGIN(steps_600);
     for (i = 0; i < 600; ++i) work->t131[i] = FOM_MAT_t0_c[i];
     fom_fused_t131(work->t128, work->t129, work->t130, work->t131);
     for (i = 0; i < 600; ++i) { fom_float v = work->t131[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t131[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
+    FOM_PROF_END(steps_600);
+    }
     /* t132 */
+    {
+    FOM_PROF_BEGIN(steps_20_relu);
     for (i = 0; i < 20; ++i) work->t132[i] = FOM_MAT_t1_c[i];
     fom_fused_t132(work->t128, work->t129, work->t131, work->t132);
     for (i = 0; i < 20; ++i) if (work->t132[i] < 0.0) work->t132[i] = 0.0;
+    FOM_PROF_END(steps_20_relu);
+    }
     /* t133 */
+    {
+    FOM_PROF_BEGIN(steps_30_lin);
     for (i = 0; i < 30; ++i) work->t133[i] = pp7[i];
     fom_fused_t133(work->t128, work->t130, work->t131, work->t133);
+    FOM_PROF_END(steps_30_lin);
+    }
     /* t134 */
+    {
+    FOM_PROF_BEGIN(steps_600);
     for (i = 0; i < 600; ++i) work->t134[i] = FOM_MAT_t0_c[i];
     fom_fused_t134(work->t131, work->t132, work->t133, work->t134);
     for (i = 0; i < 600; ++i) { fom_float v = work->t134[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t134[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
+    FOM_PROF_END(steps_600);
+    }
     /* t135 */
+    {
+    FOM_PROF_BEGIN(steps_20_relu);
     for (i = 0; i < 20; ++i) work->t135[i] = FOM_MAT_t1_c[i];
     fom_fused_t135(work->t131, work->t132, work->t134, work->t135);
     for (i = 0; i < 20; ++i) if (work->t135[i] < 0.0) work->t135[i] = 0.0;
+    FOM_PROF_END(steps_20_relu);
+    }
     /* t136 */
+    {
+    FOM_PROF_BEGIN(steps_30_lin);
     for (i = 0; i < 30; ++i) work->t136[i] = pp7[i];
     fom_fused_t136(work->t131, work->t133, work->t134, work->t136);
+    FOM_PROF_END(steps_30_lin);
+    }
     /* t137 */
+    {
+    FOM_PROF_BEGIN(steps_600);
     for (i = 0; i < 600; ++i) work->t137[i] = FOM_MAT_t0_c[i];
     fom_fused_t137(work->t134, work->t135, work->t136, work->t137);
     for (i = 0; i < 600; ++i) { fom_float v = work->t137[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t137[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
+    FOM_PROF_END(steps_600);
+    }
     /* t138 */
+    {
+    FOM_PROF_BEGIN(steps_20_relu);
     for (i = 0; i < 20; ++i) work->t138[i] = FOM_MAT_t1_c[i];
     fom_fused_t138(work->t134, work->t135, work->t137, work->t138);
     for (i = 0; i < 20; ++i) if (work->t138[i] < 0.0) work->t138[i] = 0.0;
+    FOM_PROF_END(steps_20_relu);
+    }
     /* t139 */
+    {
+    FOM_PROF_BEGIN(steps_30_lin);
     for (i = 0; i < 30; ++i) work->t139[i] = pp7[i];
     fom_fused_t139(work->t134, work->t136, work->t137, work->t139);
+    FOM_PROF_END(steps_30_lin);
+    }
     /* t140 */
+    {
+    FOM_PROF_BEGIN(steps_600);
     for (i = 0; i < 600; ++i) work->t140[i] = FOM_MAT_t0_c[i];
     fom_fused_t140(work->t137, work->t138, work->t139, work->t140);
     for (i = 0; i < 600; ++i) { fom_float v = work->t140[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t140[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
+    FOM_PROF_END(steps_600);
+    }
     /* t141 */
+    {
+    FOM_PROF_BEGIN(steps_20_relu);
     for (i = 0; i < 20; ++i) work->t141[i] = FOM_MAT_t1_c[i];
     fom_fused_t141(work->t137, work->t138, work->t140, work->t141);
     for (i = 0; i < 20; ++i) if (work->t141[i] < 0.0) work->t141[i] = 0.0;
+    FOM_PROF_END(steps_20_relu);
+    }
     /* t142 */
+    {
+    FOM_PROF_BEGIN(steps_30_lin);
     for (i = 0; i < 30; ++i) work->t142[i] = pp7[i];
     fom_fused_t142(work->t137, work->t139, work->t140, work->t142);
+    FOM_PROF_END(steps_30_lin);
+    }
     /* t143 */
+    {
+    FOM_PROF_BEGIN(steps_600);
     for (i = 0; i < 600; ++i) work->t143[i] = FOM_MAT_t0_c[i];
     fom_fused_t143(work->t140, work->t141, work->t142, work->t143);
     for (i = 0; i < 600; ++i) { fom_float v = work->t143[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t143[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
+    FOM_PROF_END(steps_600);
+    }
     /* t144 */
+    {
+    FOM_PROF_BEGIN(steps_20_relu);
     for (i = 0; i < 20; ++i) work->t144[i] = FOM_MAT_t1_c[i];
     fom_fused_t144(work->t140, work->t141, work->t143, work->t144);
     for (i = 0; i < 20; ++i) if (work->t144[i] < 0.0) work->t144[i] = 0.0;
+    FOM_PROF_END(steps_20_relu);
+    }
     /* t145 */
+    {
+    FOM_PROF_BEGIN(steps_30_lin);
     for (i = 0; i < 30; ++i) work->t145[i] = pp7[i];
     fom_fused_t145(work->t140, work->t142, work->t143, work->t145);
+    FOM_PROF_END(steps_30_lin);
+    }
     /* t146 */
+    {
+    FOM_PROF_BEGIN(steps_600);
     for (i = 0; i < 600; ++i) work->t146[i] = FOM_MAT_t0_c[i];
     fom_fused_t146(work->t143, work->t144, work->t145, work->t146);
     for (i = 0; i < 600; ++i) { fom_float v = work->t146[i]; v = v < FOM_MAT_t0_lo[i] ? FOM_MAT_t0_lo[i] : v; work->t146[i] = v > FOM_MAT_t0_hi[i] ? FOM_MAT_t0_hi[i] : v; }
+    FOM_PROF_END(steps_600);
+    }
     /* t147 */
+    {
+    FOM_PROF_BEGIN(steps_20_relu);
     for (i = 0; i < 20; ++i) work->t147[i] = FOM_MAT_t1_c[i];
     fom_fused_t147(work->t143, work->t144, work->t146, work->t147);
     for (i = 0; i < 20; ++i) if (work->t147[i] < 0.0) work->t147[i] = 0.0;
+    FOM_PROF_END(steps_20_relu);
+    }
     /* t148 */
+    {
+    FOM_PROF_BEGIN(steps_30_lin);
     for (i = 0; i < 30; ++i) work->t148[i] = pp7[i];
     fom_fused_t148(work->t143, work->t145, work->t146, work->t148);
+    FOM_PROF_END(steps_30_lin);
+    }
     /* final state */
+    {
+    FOM_PROF_BEGIN(state_update);
     for (i = 0; i < FOM_STATE_DIM; ++i) work->new_state[i] = 0.0;
     fom_fused_su(work->t146, work->t147, work->t148, work->new_state);
     for (i = 0; i < FOM_STATE_DIM; ++i) {
@@ -592,6 +1191,8 @@
         if (d > res) res = d;
         work->state[i] = work->new_state[i];
         work->solution.x[i] = work->new_state[i];
+    }
+    FOM_PROF_END(state_update);
     }
     work->solution.iter = 1;
     work->solution.residual = res;
--- src/fom_ops.c (translated)
+++ src/fom_ops.c (optimized)
@@ -1915,6 +1915,43 @@
         fom_float s = 0.0;
         for (p = FOM_MAT_su_t_t148_ROWPTR[i]; p < FOM_MAT_su_t_t148_ROWPTR[i + 1]; ++p) s += FOM_MAT_su_t_t148_VALS[p] * x[FOM_MAT_su_t_t148_COLIDX[p]];
         out[i] += s;
+    }
+}
+
+/* 20-row steps: both 20x600 members have row i on columns 30i..30i+29
+   (COLIDX == p), and the 20x20 member is the identity selection. */
+static void relu_step_20(const fom_float *va, const fom_float *a, const fom_float *b, const fom_float *vc, const fom_float *c, fom_float *out) {
+    fom_int i, k;
+    for (i = 0; i < 20; ++i) {
+        fom_float s0 = b[i], s1 = 0.0;
+        const fom_int o = 30 * i;
+        for (k = 0; k < 30; ++k) { s0 += va[o + k] * a[o + k]; s1 += vc[o + k] * c[o + k]; }
+        out[i] += s0 + s1;
+    }
+}
+
+/* 30-row steps: both 30x600 members have row j on columns j + 30r, r = 0..19
+   (entry 20j + r), and the 30x30 member is the identity selection. */
+static void lin_step_30(const fom_float *va, const fom_float *a, const fom_float *b, const fom_float *vc, const fom_float *c, fom_float *out) {
+    fom_int j, r;
+    for (j = 0; j < 30; ++j) {
+        fom_float s = b[j];
+        const fom_float *pa = va + 20 * j, *pc = vc + 20 * j;
+        for (r = 0; r < 20; ++r) s += pa[r] * a[30 * r + j] + pc[r] * c[30 * r + j];
+        out[j] += s;
+    }
+}
+
+/* Shared body of the 600-row steps: t5_t_t2 is the identity selection and
+   t2_t_t1, t5_t_t4 have exactly one entry per row (ROWPTR[i] == i). */
+static void clip_step_600(const fom_float *a, const fom_float *b, const fom_float *c, fom_float *out) {
+    /* t2_t_t1 row 30i+k reads column i, t5_t_t4 row 30i+k reads column k. */
+    fom_int i, k;
+    for (i = 0; i < 20; ++i) {
+        const fom_int o = 30 * i;
+        const fom_float bi = b[i];
+        for (k = 0; k < 30; ++k)
+            out[o + k] += a[o + k] + FOM_MAT_t2_t_t1_VALS[o + k] * bi + FOM_MAT_t5_t_t4_VALS[o + k] * c[k];
     }
 }
 
@@ -1948,19 +1985,7 @@
 }
 
 void fom_fused_t3(const fom_float *t_t0, const fom_float *t_t1, const fom_float *t_t2, fom_float *out) {
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
+    relu_step_20(FOM_MAT_t3_t_t0_VALS, t_t0, t_t1, FOM_MAT_t3_t_t2_VALS, t_t2, out);
 }
 
 void fom_fused_t4(const fom_float *state, const fom_float *t_t0, const fom_float *t_t2, fom_float *out) {
@@ -1980,2307 +2005,579 @@
 }
 
 void fom_fused_t5(const fom_float *t_t2, const fom_float *t_t3, const fom_float *t_t4, fom_float *out) {
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
+    clip_step_600(t_t2, t_t3, t_t4, out);
 }
 
 void fom_fused_t6(const fom_float *t_t2, const fom_float *t_t3, const fom_float *t_t5, fom_float *out) {
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
+    relu_step_20(FOM_MAT_t6_t_t2_VALS, t_t2, t_t3, FOM_MAT_t6_t_t5_VALS, t_t5, out);
 }
 
 void fom_fused_t7(const fom_float *t_t2, const fom_float *t_t4, const fom_float *t_t5, fom_float *out) {
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
+    lin_step_30(FOM_MAT_t7_t_t2_VALS, t_t2, t_t4, FOM_MAT_t7_t_t5_VALS, t_t5, out);
 }
 
 void fom_fused_t8(const fom_float *t_t5, const fom_float *t_t6, const fom_float *t_t7, fom_float *out) {
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
+    clip_step_600(t_t5, t_t6, t_t7, out);
 }
 
 void fom_fused_t9(const fom_float *t_t5, const fom_float *t_t6, const fom_float *t_t8, fom_float *out) {
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
+    relu_step_20(FOM_MAT_t9_t_t5_VALS, t_t5, t_t6, FOM_MAT_t9_t_t8_VALS, t_t8, out);
 }
 
 void fom_fused_t10(const fom_float *t_t5, const fom_float *t_t7, const fom_float *t_t8, fom_float *out) {
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
+    lin_step_30(FOM_MAT_t10_t_t5_VALS, t_t5, t_t7, FOM_MAT_t10_t_t8_VALS, t_t8, out);
 }
 
 void fom_fused_t11(const fom_float *t_t8, const fom_float *t_t9, const fom_float *t_t10, fom_float *out) {
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
+    clip_step_600(t_t8, t_t9, t_t10, out);
 }
 
 void fom_fused_t12(const fom_float *t_t8, const fom_float *t_t9, const fom_float *t_t11, fom_float *out) {
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
+    relu_step_20(FOM_MAT_t12_t_t8_VALS, t_t8, t_t9, FOM_MAT_t12_t_t11_VALS, t_t11, out);
 }
 
 void fom_fused_t13(const fom_float *t_t8, const fom_float *t_t10, const fom_float *t_t11, fom_float *out) {
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
+    lin_step_30(FOM_MAT_t13_t_t8_VALS, t_t8, t_t10, FOM_MAT_t13_t_t11_VALS, t_t11, out);
 }
 
 void fom_fused_t14(const fom_float *t_t11, const fom_float *t_t12, const fom_float *t_t13, fom_float *out) {
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
+    clip_step_600(t_t11, t_t12, t_t13, out);
 }
 
 void fom_fused_t15(const fom_float *t_t11, const fom_float *t_t12, const fom_float *t_t14, fom_float *out) {
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
+    relu_step_20(FOM_MAT_t15_t_t11_VALS, t_t11, t_t12, FOM_MAT_t15_t_t14_VALS, t_t14, out);
 }
 
 void fom_fused_t16(const fom_float *t_t11, const fom_float *t_t13, const fom_float *t_t14, fom_float *out) {
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
+    lin_step_30(FOM_MAT_t16_t_t11_VALS, t_t11, t_t13, FOM_MAT_t16_t_t14_VALS, t_t14, out);
 }
 
 void fom_fused_t17(const fom_float *t_t14, const fom_float *t_t15, const fom_float *t_t16, fom_float *out) {
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
+    clip_step_600(t_t14, t_t15, t_t16, out);
 }
 
 void fom_fused_t18(const fom_float *t_t14, const fom_float *t_t15, const fom_float *t_t17, fom_float *out) {
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
+    relu_step_20(FOM_MAT_t18_t_t14_VALS, t_t14, t_t15, FOM_MAT_t18_t_t17_VALS, t_t17, out);
 }
 
 void fom_fused_t19(const fom_float *t_t14, const fom_float *t_t16, const fom_float *t_t17, fom_float *out) {
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
+    lin_step_30(FOM_MAT_t19_t_t14_VALS, t_t14, t_t16, FOM_MAT_t19_t_t17_VALS, t_t17, out);
 }
 
 void fom_fused_t20(const fom_float *t_t17, const fom_float *t_t18, const fom_float *t_t19, fom_float *out) {
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
+    clip_step_600(t_t17, t_t18, t_t19, out);
 }
 
 void fom_fused_t21(const fom_float *t_t17, const fom_float *t_t18, const fom_float *t_t20, fom_float *out) {
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
+    relu_step_20(FOM_MAT_t21_t_t17_VALS, t_t17, t_t18, FOM_MAT_t21_t_t20_VALS, t_t20, out);
 }
 
 void fom_fused_t22(const fom_float *t_t17, const fom_float *t_t19, const fom_float *t_t20, fom_float *out) {
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
+    lin_step_30(FOM_MAT_t22_t_t17_VALS, t_t17, t_t19, FOM_MAT_t22_t_t20_VALS, t_t20, out);
 }
 
 void fom_fused_t23(const fom_float *t_t20, const fom_float *t_t21, const fom_float *t_t22, fom_float *out) {
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
+    clip_step_600(t_t20, t_t21, t_t22, out);
 }
 
 void fom_fused_t24(const fom_float *t_t20, const fom_float *t_t21, const fom_float *t_t23, fom_float *out) {
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
+    relu_step_20(FOM_MAT_t24_t_t20_VALS, t_t20, t_t21, FOM_MAT_t24_t_t23_VALS, t_t23, out);
 }
 
 void fom_fused_t25(const fom_float *t_t20, const fom_float *t_t22, const fom_float *t_t23, fom_float *out) {
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
+    lin_step_30(FOM_MAT_t25_t_t20_VALS, t_t20, t_t22, FOM_MAT_t25_t_t23_VALS, t_t23, out);
 }
 
 void fom_fused_t26(const fom_float *t_t23, const fom_float *t_t24, const fom_float *t_t25, fom_float *out) {
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
+    clip_step_600(t_t23, t_t24, t_t25, out);
 }
 
 void fom_fused_t27(const fom_float *t_t23, const fom_float *t_t24, const fom_float *t_t26, fom_float *out) {
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
+    relu_step_20(FOM_MAT_t27_t_t23_VALS, t_t23, t_t24, FOM_MAT_t27_t_t26_VALS, t_t26, out);
 }
 
 void fom_fused_t28(const fom_float *t_t23, const fom_float *t_t25, const fom_float *t_t26, fom_float *out) {
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
+    lin_step_30(FOM_MAT_t28_t_t23_VALS, t_t23, t_t25, FOM_MAT_t28_t_t26_VALS, t_t26, out);
 }
 
 void fom_fused_t29(const fom_float *t_t26, const fom_float *t_t27, const fom_float *t_t28, fom_float *out) {
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
+    clip_step_600(t_t26, t_t27, t_t28, out);
 }
 
 void fom_fused_t30(const fom_float *t_t26, const fom_float *t_t27, const fom_float *t_t29, fom_float *out) {
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
+    relu_step_20(FOM_MAT_t30_t_t26_VALS, t_t26, t_t27, FOM_MAT_t30_t_t29_VALS, t_t29, out);
 }
 
 void fom_fused_t31(const fom_float *t_t26, const fom_float *t_t28, const fom_float *t_t29, fom_float *out) {
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
+    lin_step_30(FOM_MAT_t31_t_t26_VALS, t_t26, t_t28, FOM_MAT_t31_t_t29_VALS, t_t29, out);
 }
 
 void fom_fused_t32(const fom_float *t_t29, const fom_float *t_t30, const fom_float *t_t31, fom_float *out) {
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
+    clip_step_600(t_t29, t_t30, t_t31, out);
 }
 
 void fom_fused_t33(const fom_float *t_t29, const fom_float *t_t30, const fom_float *t_t32, fom_float *out) {
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
+    relu_step_20(FOM_MAT_t33_t_t29_VALS, t_t29, t_t30, FOM_MAT_t33_t_t32_VALS, t_t32, out);
 }
 
 void fom_fused_t34(const fom_float *t_t29, const fom_float *t_t31, const fom_float *t_t32, fom_float *out) {
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
+    lin_step_30(FOM_MAT_t34_t_t29_VALS, t_t29, t_t31, FOM_MAT_t34_t_t32_VALS, t_t32, out);
 }
 
 void fom_fused_t35(const fom_float *t_t32, const fom_float *t_t33, const fom_float *t_t34, fom_float *out) {
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
+    clip_step_600(t_t32, t_t33, t_t34, out);
 }
 
 void fom_fused_t36(const fom_float *t_t32, const fom_float *t_t33, const fom_float *t_t35, fom_float *out) {
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
+    relu_step_20(FOM_MAT_t36_t_t32_VALS, t_t32, t_t33, FOM_MAT_t36_t_t35_VALS, t_t35, out);
 }
 
 void fom_fused_t37(const fom_float *t_t32, const fom_float *t_t34, const fom_float *t_t35, fom_float *out) {
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
+    lin_step_30(FOM_MAT_t37_t_t32_VALS, t_t32, t_t34, FOM_MAT_t37_t_t35_VALS, t_t35, out);
 }
 
 void fom_fused_t38(const fom_float *t_t35, const fom_float *t_t36, const fom_float *t_t37, fom_float *out) {
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
+    clip_step_600(t_t35, t_t36, t_t37, out);
 }
 
 void fom_fused_t39(const fom_float *t_t35, const fom_float *t_t36, const fom_float *t_t38, fom_float *out) {
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
+    relu_step_20(FOM_MAT_t39_t_t35_VALS, t_t35, t_t36, FOM_MAT_t39_t_t38_VALS, t_t38, out);
 }
 
 void fom_fused_t40(const fom_float *t_t35, const fom_float *t_t37, const fom_float *t_t38, fom_float *out) {
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
+    lin_step_30(FOM_MAT_t40_t_t35_VALS, t_t35, t_t37, FOM_MAT_t40_t_t38_VALS, t_t38, out);
 }
 
 void fom_fused_t41(const fom_float *t_t38, const fom_float *t_t39, const fom_float *t_t40, fom_float *out) {
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
+    clip_step_600(t_t38, t_t39, t_t40, out);
 }
 
 void fom_fused_t42(const fom_float *t_t38, const fom_float *t_t39, const fom_float *t_t41, fom_float *out) {
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
+    relu_step_20(FOM_MAT_t42_t_t38_VALS, t_t38, t_t39, FOM_MAT_t42_t_t41_VALS, t_t41, out);
 }
 
 void fom_fused_t43(const fom_float *t_t38, const fom_float *t_t40, const fom_float *t_t41, fom_float *out) {
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
+    lin_step_30(FOM_MAT_t43_t_t38_VALS, t_t38, t_t40, FOM_MAT_t43_t_t41_VALS, t_t41, out);
 }
 
 void fom_fused_t44(const fom_float *t_t41, const fom_float *t_t42, const fom_float *t_t43, fom_float *out) {
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
+    clip_step_600(t_t41, t_t42, t_t43, out);
 }
 
 void fom_fused_t45(const fom_float *t_t41, const fom_float *t_t42, const fom_float *t_t44, fom_float *out) {
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
+    relu_step_20(FOM_MAT_t45_t_t41_VALS, t_t41, t_t42, FOM_MAT_t45_t_t44_VALS, t_t44, out);
 }
 
 void fom_fused_t46(const fom_float *t_t41, const fom_float *t_t43, const fom_float *t_t44, fom_float *out) {
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
+    lin_step_30(FOM_MAT_t46_t_t41_VALS, t_t41, t_t43, FOM_MAT_t46_t_t44_VALS, t_t44, out);
 }
 
 void fom_fused_t47(const fom_float *t_t44, const fom_float *t_t45, const fom_float *t_t46, fom_float *out) {
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
+    clip_step_600(t_t44, t_t45, t_t46, out);
 }
 
 void fom_fused_t48(const fom_float *t_t44, const fom_float *t_t45, const fom_float *t_t47, fom_float *out) {
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
+    relu_step_20(FOM_MAT_t48_t_t44_VALS, t_t44, t_t45, FOM_MAT_t48_t_t47_VALS, t_t47, out);
 }
 
 void fom_fused_t49(const fom_float *t_t44, const fom_float *t_t46, const fom_float *t_t47, fom_float *out) {
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
+    lin_step_30(FOM_MAT_t49_t_t44_VALS, t_t44, t_t46, FOM_MAT_t49_t_t47_VALS, t_t47, out);
 }
 
 void fom_fused_t50(const fom_float *t_t47, const fom_float *t_t48, const fom_float *t_t49, fom_float *out) {
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
+    clip_step_600(t_t47, t_t48, t_t49, out);
 }
 
 void fom_fused_t51(const fom_float *t_t47, const fom_float *t_t48, const fom_float *t_t50, fom_float *out) {
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
+    relu_step_20(FOM_MAT_t51_t_t47_VALS, t_t47, t_t48, FOM_MAT_t51_t_t50_VALS, t_t50, out);
 }
 
 void fom_fused_t52(const fom_float *t_t47, const fom_float *t_t49, const fom_float *t_t50, fom_float *out) {
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
+    lin_step_30(FOM_MAT_t52_t_t47_VALS, t_t47, t_t49, FOM_MAT_t52_t_t50_VALS, t_t50, out);
 }
 
 void fom_fused_t53(const fom_float *t_t50, const fom_float *t_t51, const fom_float *t_t52, fom_float *out) {
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
+    clip_step_600(t_t50, t_t51, t_t52, out);
 }
 
 void fom_fused_t54(const fom_float *t_t50, const fom_float *t_t51, const fom_float *t_t53, fom_float *out) {
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
+    relu_step_20(FOM_MAT_t54_t_t50_VALS, t_t50, t_t51, FOM_MAT_t54_t_t53_VALS, t_t53, out);
 }
 
 void fom_fused_t55(const fom_float *t_t50, const fom_float *t_t52, const fom_float *t_t53, fom_float *out) {
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
+    lin_step_30(FOM_MAT_t55_t_t50_VALS, t_t50, t_t52, FOM_MAT_t55_t_t53_VALS, t_t53, out);
 }
 
 void fom_fused_t56(const fom_float *t_t53, const fom_float *t_t54, const fom_float *t_t55, fom_float *out) {
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
+    clip_step_600(t_t53, t_t54, t_t55, out);
 }
 
 void fom_fused_t57(const fom_float *t_t53, const fom_float *t_t54, const fom_float *t_t56, fom_float *out) {
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
+    relu_step_20(FOM_MAT_t57_t_t53_VALS, t_t53, t_t54, FOM_MAT_t57_t_t56_VALS, t_t56, out);
 }
 
 void fom_fused_t58(const fom_float *t_t53, const fom_float *t_t55, const fom_float *t_t56, fom_float *out) {
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
+    lin_step_30(FOM_MAT_t58_t_t53_VALS, t_t53, t_t55, FOM_MAT_t58_t_t56_VALS, t_t56, out);
 }
 
 void fom_fused_t59(const fom_float *t_t56, const fom_float *t_t57, const fom_float *t_t58, fom_float *out) {
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
+    clip_step_600(t_t56, t_t57, t_t58, out);
 }
 
 void fom_fused_t60(const fom_float *t_t56, const fom_float *t_t57, const fom_float *t_t59, fom_float *out) {
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
+    relu_step_20(FOM_MAT_t60_t_t56_VALS, t_t56, t_t57, FOM_MAT_t60_t_t59_VALS, t_t59, out);
 }
 
 void fom_fused_t61(const fom_float *t_t56, const fom_float *t_t58, const fom_float *t_t59, fom_float *out) {
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
+    lin_step_30(FOM_MAT_t61_t_t56_VALS, t_t56, t_t58, FOM_MAT_t61_t_t59_VALS, t_t59, out);
 }
 
 void fom_fused_t62(const fom_float *t_t59, const fom_float *t_t60, const fom_float *t_t61, fom_float *out) {
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
+    clip_step_600(t_t59, t_t60, t_t61, out);
 }
 
 void fom_fused_t63(const fom_float *t_t59, const fom_float *t_t60, const fom_float *t_t62, fom_float *out) {
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
+    relu_step_20(FOM_MAT_t63_t_t59_VALS, t_t59, t_t60, FOM_MAT_t63_t_t62_VALS, t_t62, out);
 }
 
 void fom_fused_t64(const fom_float *t_t59, const fom_float *t_t61, const fom_float *t_t62, fom_float *out) {
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
+    lin_step_30(FOM_MAT_t64_t_t59_VALS, t_t59, t_t61, FOM_MAT_t64_t_t62_VALS, t_t62, out);
 }
 
 void fom_fused_t65(const fom_float *t_t62, const fom_float *t_t63, const fom_float *t_t64, fom_float *out) {
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
+    clip_step_600(t_t62, t_t63, t_t64, out);
 }
 
 void fom_fused_t66(const fom_float *t_t62, const fom_float *t_t63, const fom_float *t_t65, fom_float *out) {
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
+    relu_step_20(FOM_MAT_t66_t_t62_VALS, t_t62, t_t63, FOM_MAT_t66_t_t65_VALS, t_t65, out);
 }
 
 void fom_fused_t67(const fom_float *t_t62, const fom_float *t_t64, const fom_float *t_t65, fom_float *out) {
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
+    lin_step_30(FOM_MAT_t67_t_t62_VALS, t_t62, t_t64, FOM_MAT_t67_t_t65_VALS, t_t65, out);
 }
 
 void fom_fused_t68(const fom_float *t_t65, const fom_float *t_t66, const fom_float *t_t67, fom_float *out) {
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
+    clip_step_600(t_t65, t_t66, t_t67, out);
 }
 
 void fom_fused_t69(const fom_float *t_t65, const fom_float *t_t66, const fom_float *t_t68, fom_float *out) {
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
+    relu_step_20(FOM_MAT_t69_t_t65_VALS, t_t65, t_t66, FOM_MAT_t69_t_t68_VALS, t_t68, out);
 }
 
 void fom_fused_t70(const fom_float *t_t65, const fom_float *t_t67, const fom_float *t_t68, fom_float *out) {
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
+    lin_step_30(FOM_MAT_t70_t_t65_VALS, t_t65, t_t67, FOM_MAT_t70_t_t68_VALS, t_t68, out);
 }
 
 void fom_fused_t71(const fom_float *t_t68, const fom_float *t_t69, const fom_float *t_t70, fom_float *out) {
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
+    clip_step_600(t_t68, t_t69, t_t70, out);
 }
 
 void fom_fused_t72(const fom_float *t_t68, const fom_float *t_t69, const fom_float *t_t71, fom_float *out) {
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
+    relu_step_20(FOM_MAT_t72_t_t68_VALS, t_t68, t_t69, FOM_MAT_t72_t_t71_VALS, t_t71, out);
 }
 
 void fom_fused_t73(const fom_float *t_t68, const fom_float *t_t70, const fom_float *t_t71, fom_float *out) {
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
+    lin_step_30(FOM_MAT_t73_t_t68_VALS, t_t68, t_t70, FOM_MAT_t73_t_t71_VALS, t_t71, out);
 }
 
 void fom_fused_t74(const fom_float *t_t71, const fom_float *t_t72, const fom_float *t_t73, fom_float *out) {
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
+    clip_step_600(t_t71, t_t72, t_t73, out);
 }
 
 void fom_fused_t75(const fom_float *t_t71, const fom_float *t_t72, const fom_float *t_t74, fom_float *out) {
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
+    relu_step_20(FOM_MAT_t75_t_t71_VALS, t_t71, t_t72, FOM_MAT_t75_t_t74_VALS, t_t74, out);
 }
 
 void fom_fused_t76(const fom_float *t_t71, const fom_float *t_t73, const fom_float *t_t74, fom_float *out) {
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
+    lin_step_30(FOM_MAT_t76_t_t71_VALS, t_t71, t_t73, FOM_MAT_t76_t_t74_VALS, t_t74, out);
 }
 
 void fom_fused_t77(const fom_float *t_t74, const fom_float *t_t75, const fom_float *t_t76, fom_float *out) {
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
+    clip_step_600(t_t74, t_t75, t_t76, out);
 }
 
 void fom_fused_t78(const fom_float *t_t74, const fom_float *t_t75, const fom_float *t_t77, fom_float *out) {
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
+    relu_step_20(FOM_MAT_t78_t_t74_VALS, t_t74, t_t75, FOM_MAT_t78_t_t77_VALS, t_t77, out);
 }
 
 void fom_fused_t79(const fom_float *t_t74, const fom_float *t_t76, const fom_float *t_t77, fom_float *out) {
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
+    lin_step_30(FOM_MAT_t79_t_t74_VALS, t_t74, t_t76, FOM_MAT_t79_t_t77_VALS, t_t77, out);
 }
 
 void fom_fused_t80(const fom_float *t_t77, const fom_float *t_t78, const fom_float *t_t79, fom_float *out) {
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
+    clip_step_600(t_t77, t_t78, t_t79, out);
 }
 
 void fom_fused_t81(const fom_float *t_t77, const fom_float *t_t78, const fom_float *t_t80, fom_float *out) {
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
+    relu_step_20(FOM_MAT_t81_t_t77_VALS, t_t77, t_t78, FOM_MAT_t81_t_t80_VALS, t_t80, out);
 }
 
 void fom_fused_t82(const fom_float *t_t77, const fom_float *t_t79, const fom_float *t_t80, fom_float *out) {
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
+    lin_step_30(FOM_MAT_t82_t_t77_VALS, t_t77, t_t79, FOM_MAT_t82_t_t80_VALS, t_t80, out);
 }
 
 void fom_fused_t83(const fom_float *t_t80, const fom_float *t_t81, const fom_float *t_t82, fom_float *out) {
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
+    clip_step_600(t_t80, t_t81, t_t82, out);
 }
 
 void fom_fused_t84(const fom_float *t_t80, const fom_float *t_t81, const fom_float *t_t83, fom_float *out) {
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
+    relu_step_20(FOM_MAT_t84_t_t80_VALS, t_t80, t_t81, FOM_MAT_t84_t_t83_VALS, t_t83, out);
 }
 
 void fom_fused_t85(const fom_float *t_t80, const fom_float *t_t82, const fom_float *t_t83, fom_float *out) {
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
+    lin_step_30(FOM_MAT_t85_t_t80_VALS, t_t80, t_t82, FOM_MAT_t85_t_t83_VALS, t_t83, out);
 }
 
 void fom_fused_t86(const fom_float *t_t83, const fom_float *t_t84, const fom_float *t_t85, fom_float *out) {
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
+    clip_step_600(t_t83, t_t84, t_t85, out);
 }
 
 void fom_fused_t87(const fom_float *t_t83, const fom_float *t_t84, const fom_float *t_t86, fom_float *out) {
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
+    relu_step_20(FOM_MAT_t87_t_t83_VALS, t_t83, t_t84, FOM_MAT_t87_t_t86_VALS, t_t86, out);
 }
 
 void fom_fused_t88(const fom_float *t_t83, const fom_float *t_t85, const fom_float *t_t86, fom_float *out) {
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
+    lin_step_30(FOM_MAT_t88_t_t83_VALS, t_t83, t_t85, FOM_MAT_t88_t_t86_VALS, t_t86, out);
 }
 
 void fom_fused_t89(const fom_float *t_t86, const fom_float *t_t87, const fom_float *t_t88, fom_float *out) {
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
+    clip_step_600(t_t86, t_t87, t_t88, out);
 }
 
 void fom_fused_t90(const fom_float *t_t86, const fom_float *t_t87, const fom_float *t_t89, fom_float *out) {
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
+    relu_step_20(FOM_MAT_t90_t_t86_VALS, t_t86, t_t87, FOM_MAT_t90_t_t89_VALS, t_t89, out);
 }
 
 void fom_fused_t91(const fom_float *t_t86, const fom_float *t_t88, const fom_float *t_t89, fom_float *out) {
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
+    lin_step_30(FOM_MAT_t91_t_t86_VALS, t_t86, t_t88, FOM_MAT_t91_t_t89_VALS, t_t89, out);
 }
 
 void fom_fused_t92(const fom_float *t_t89, const fom_float *t_t90, const fom_float *t_t91, fom_float *out) {
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
+    clip_step_600(t_t89, t_t90, t_t91, out);
 }
 
 void fom_fused_t93(const fom_float *t_t89, const fom_float *t_t90, const fom_float *t_t92, fom_float *out) {
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
+    relu_step_20(FOM_MAT_t93_t_t89_VALS, t_t89, t_t90, FOM_MAT_t93_t_t92_VALS, t_t92, out);
 }
 
 void fom_fused_t94(const fom_float *t_t89, const fom_float *t_t91, const fom_float *t_t92, fom_float *out) {
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
+    lin_step_30(FOM_MAT_t94_t_t89_VALS, t_t89, t_t91, FOM_MAT_t94_t_t92_VALS, t_t92, out);
 }
 
 void fom_fused_t95(const fom_float *t_t92, const fom_float *t_t93, const fom_float *t_t94, fom_float *out) {
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
+    clip_step_600(t_t92, t_t93, t_t94, out);
 }
 
 void fom_fused_t96(const fom_float *t_t92, const fom_float *t_t93, const fom_float *t_t95, fom_float *out) {
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
+    relu_step_20(FOM_MAT_t96_t_t92_VALS, t_t92, t_t93, FOM_MAT_t96_t_t95_VALS, t_t95, out);
 }
 
 void fom_fused_t97(const fom_float *t_t92, const fom_float *t_t94, const fom_float *t_t95, fom_float *out) {
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
+    lin_step_30(FOM_MAT_t97_t_t92_VALS, t_t92, t_t94, FOM_MAT_t97_t_t95_VALS, t_t95, out);
 }
 
 void fom_fused_t98(const fom_float *t_t95, const fom_float *t_t96, const fom_float *t_t97, fom_float *out) {
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
+    clip_step_600(t_t95, t_t96, t_t97, out);
 }
 
 void fom_fused_t99(const fom_float *t_t95, const fom_float *t_t96, const fom_float *t_t98, fom_float *out) {
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
+    relu_step_20(FOM_MAT_t99_t_t95_VALS, t_t95, t_t96, FOM_MAT_t99_t_t98_VALS, t_t98, out);
 }
 
 void fom_fused_t100(const fom_float *t_t95, const fom_float *t_t97, const fom_float *t_t98, fom_float *out) {
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
+    lin_step_30(FOM_MAT_t100_t_t95_VALS, t_t95, t_t97, FOM_MAT_t100_t_t98_VALS, t_t98, out);
 }
 
 void fom_fused_t101(const fom_float *t_t98, const fom_float *t_t99, const fom_float *t_t100, fom_float *out) {
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
+    clip_step_600(t_t98, t_t99, t_t100, out);
 }
 
 void fom_fused_t102(const fom_float *t_t98, const fom_float *t_t99, const fom_float *t_t101, fom_float *out) {
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
+    relu_step_20(FOM_MAT_t102_t_t98_VALS, t_t98, t_t99, FOM_MAT_t102_t_t101_VALS, t_t101, out);
 }
 
 void fom_fused_t103(const fom_float *t_t98, const fom_float *t_t100, const fom_float *t_t101, fom_float *out) {
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
+    lin_step_30(FOM_MAT_t103_t_t98_VALS, t_t98, t_t100, FOM_MAT_t103_t_t101_VALS, t_t101, out);
 }
 
 void fom_fused_t104(const fom_float *t_t101, const fom_float *t_t102, const fom_float *t_t103, fom_float *out) {
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
+    clip_step_600(t_t101, t_t102, t_t103, out);
 }
 
 void fom_fused_t105(const fom_float *t_t101, const fom_float *t_t102, const fom_float *t_t104, fom_float *out) {
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
+    relu_step_20(FOM_MAT_t105_t_t101_VALS, t_t101, t_t102, FOM_MAT_t105_t_t104_VALS, t_t104, out);
 }
 
 void fom_fused_t106(const fom_float *t_t101, const fom_float *t_t103, const fom_float *t_t104, fom_float *out) {
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
+    lin_step_30(FOM_MAT_t106_t_t101_VALS, t_t101, t_t103, FOM_MAT_t106_t_t104_VALS, t_t104, out);
 }
 
 void fom_fused_t107(const fom_float *t_t104, const fom_float *t_t105, const fom_float *t_t106, fom_float *out) {
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
+    clip_step_600(t_t104, t_t105, t_t106, out);
 }
 
 void fom_fused_t108(const fom_float *t_t104, const fom_float *t_t105, const fom_float *t_t107, fom_float *out) {
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
+    relu_step_20(FOM_MAT_t108_t_t104_VALS, t_t104, t_t105, FOM_MAT_t108_t_t107_VALS, t_t107, out);
 }
 
 void fom_fused_t109(const fom_float *t_t104, const fom_float *t_t106, const fom_float *t_t107, fom_float *out) {
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
+    lin_step_30(FOM_MAT_t109_t_t104_VALS, t_t104, t_t106, FOM_MAT_t109_t_t107_VALS, t_t107, out);
 }
 
 void fom_fused_t110(const fom_float *t_t107, const fom_float *t_t108, const fom_float *t_t109, fom_float *out) {
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
+    clip_step_600(t_t107, t_t108, t_t109, out);
 }
 
 void fom_fused_t111(const fom_float *t_t107, const fom_float *t_t108, const fom_float *t_t110, fom_float *out) {
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
+    relu_step_20(FOM_MAT_t111_t_t107_VALS, t_t107, t_t108, FOM_MAT_t111_t_t110_VALS, t_t110, out);
 }
 
 void fom_fused_t112(const fom_float *t_t107, const fom_float *t_t109, const fom_float *t_t110, fom_float *out) {
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
+    lin_step_30(FOM_MAT_t112_t_t107_VALS, t_t107, t_t109, FOM_MAT_t112_t_t110_VALS, t_t110, out);
 }
 
 void fom_fused_t113(const fom_float *t_t110, const fom_float *t_t111, const fom_float *t_t112, fom_float *out) {
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
+    clip_step_600(t_t110, t_t111, t_t112, out);
 }
 
 void fom_fused_t114(const fom_float *t_t110, const fom_float *t_t111, const fom_float *t_t113, fom_float *out) {
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
+    relu_step_20(FOM_MAT_t114_t_t110_VALS, t_t110, t_t111, FOM_MAT_t114_t_t113_VALS, t_t113, out);
 }
 
 void fom_fused_t115(const fom_float *t_t110, const fom_float *t_t112, const fom_float *t_t113, fom_float *out) {
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
+    lin_step_30(FOM_MAT_t115_t_t110_VALS, t_t110, t_t112, FOM_MAT_t115_t_t113_VALS, t_t113, out);
 }
 
 void fom_fused_t116(const fom_float *t_t113, const fom_float *t_t114, const fom_float *t_t115, fom_float *out) {
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
+    clip_step_600(t_t113, t_t114, t_t115, out);
 }
 
 void fom_fused_t117(const fom_float *t_t113, const fom_float *t_t114, const fom_float *t_t116, fom_float *out) {
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
+    relu_step_20(FOM_MAT_t117_t_t113_VALS, t_t113, t_t114, FOM_MAT_t117_t_t116_VALS, t_t116, out);
 }
 
 void fom_fused_t118(const fom_float *t_t113, const fom_float *t_t115, const fom_float *t_t116, fom_float *out) {
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
+    lin_step_30(FOM_MAT_t118_t_t113_VALS, t_t113, t_t115, FOM_MAT_t118_t_t116_VALS, t_t116, out);
 }
 
 void fom_fused_t119(const fom_float *t_t116, const fom_float *t_t117, const fom_float *t_t118, fom_float *out) {
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
+    clip_step_600(t_t116, t_t117, t_t118, out);
 }
 
 void fom_fused_t120(const fom_float *t_t116, const fom_float *t_t117, const fom_float *t_t119, fom_float *out) {
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
+    relu_step_20(FOM_MAT_t120_t_t116_VALS, t_t116, t_t117, FOM_MAT_t120_t_t119_VALS, t_t119, out);
 }
 
 void fom_fused_t121(const fom_float *t_t116, const fom_float *t_t118, const fom_float *t_t119, fom_float *out) {
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
+    lin_step_30(FOM_MAT_t121_t_t116_VALS, t_t116, t_t118, FOM_MAT_t121_t_t119_VALS, t_t119, out);
 }
 
 void fom_fused_t122(const fom_float *t_t119, const fom_float *t_t120, const fom_float *t_t121, fom_float *out) {
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
+    clip_step_600(t_t119, t_t120, t_t121, out);
 }
 
 void fom_fused_t123(const fom_float *t_t119, const fom_float *t_t120, const fom_float *t_t122, fom_float *out) {
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
+    relu_step_20(FOM_MAT_t123_t_t119_VALS, t_t119, t_t120, FOM_MAT_t123_t_t122_VALS, t_t122, out);
 }
 
 void fom_fused_t124(const fom_float *t_t119, const fom_float *t_t121, const fom_float *t_t122, fom_float *out) {
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
+    lin_step_30(FOM_MAT_t124_t_t119_VALS, t_t119, t_t121, FOM_MAT_t124_t_t122_VALS, t_t122, out);
 }
 
 void fom_fused_t125(const fom_float *t_t122, const fom_float *t_t123, const fom_float *t_t124, fom_float *out) {
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
+    clip_step_600(t_t122, t_t123, t_t124, out);
 }
 
 void fom_fused_t126(const fom_float *t_t122, const fom_float *t_t123, const fom_float *t_t125, fom_float *out) {
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
+    relu_step_20(FOM_MAT_t126_t_t122_VALS, t_t122, t_t123, FOM_MAT_t126_t_t125_VALS, t_t125, out);
 }
 
 void fom_fused_t127(const fom_float *t_t122, const fom_float *t_t124, const fom_float *t_t125, fom_float *out) {
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
+    lin_step_30(FOM_MAT_t127_t_t122_VALS, t_t122, t_t124, FOM_MAT_t127_t_t125_VALS, t_t125, out);
 }
 
 void fom_fused_t128(const fom_float *t_t125, const fom_float *t_t126, const fom_float *t_t127, fom_float *out) {
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
+    clip_step_600(t_t125, t_t126, t_t127, out);
 }
 
 void fom_fused_t129(const fom_float *t_t125, const fom_float *t_t126, const fom_float *t_t128, fom_float *out) {
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
+    relu_step_20(FOM_MAT_t129_t_t125_VALS, t_t125, t_t126, FOM_MAT_t129_t_t128_VALS, t_t128, out);
 }
 
 void fom_fused_t130(const fom_float *t_t125, const fom_float *t_t127, const fom_float *t_t128, fom_float *out) {
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
+    lin_step_30(FOM_MAT_t130_t_t125_VALS, t_t125, t_t127, FOM_MAT_t130_t_t128_VALS, t_t128, out);
 }
 
 void fom_fused_t131(const fom_float *t_t128, const fom_float *t_t129, const fom_float *t_t130, fom_float *out) {
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
+    clip_step_600(t_t128, t_t129, t_t130, out);
 }
 
 void fom_fused_t132(const fom_float *t_t128, const fom_float *t_t129, const fom_float *t_t131, fom_float *out) {
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
+    relu_step_20(FOM_MAT_t132_t_t128_VALS, t_t128, t_t129, FOM_MAT_t132_t_t131_VALS, t_t131, out);
 }
 
 void fom_fused_t133(const fom_float *t_t128, const fom_float *t_t130, const fom_float *t_t131, fom_float *out) {
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
+    lin_step_30(FOM_MAT_t133_t_t128_VALS, t_t128, t_t130, FOM_MAT_t133_t_t131_VALS, t_t131, out);
 }
 
 void fom_fused_t134(const fom_float *t_t131, const fom_float *t_t132, const fom_float *t_t133, fom_float *out) {
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
+    clip_step_600(t_t131, t_t132, t_t133, out);
 }
 
 void fom_fused_t135(const fom_float *t_t131, const fom_float *t_t132, const fom_float *t_t134, fom_float *out) {
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
+    relu_step_20(FOM_MAT_t135_t_t131_VALS, t_t131, t_t132, FOM_MAT_t135_t_t134_VALS, t_t134, out);
 }
 
 void fom_fused_t136(const fom_float *t_t131, const fom_float *t_t133, const fom_float *t_t134, fom_float *out) {
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
+    lin_step_30(FOM_MAT_t136_t_t131_VALS, t_t131, t_t133, FOM_MAT_t136_t_t134_VALS, t_t134, out);
 }
 
 void fom_fused_t137(const fom_float *t_t134, const fom_float *t_t135, const fom_float *t_t136, fom_float *out) {
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
+    clip_step_600(t_t134, t_t135, t_t136, out);
 }
 
 void fom_fused_t138(const fom_float *t_t134, const fom_float *t_t135, const fom_float *t_t137, fom_float *out) {
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
+    relu_step_20(FOM_MAT_t138_t_t134_VALS, t_t134, t_t135, FOM_MAT_t138_t_t137_VALS, t_t137, out);
 }
 
 void fom_fused_t139(const fom_float *t_t134, const fom_float *t_t136, const fom_float *t_t137, fom_float *out) {
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
+    lin_step_30(FOM_MAT_t139_t_t134_VALS, t_t134, t_t136, FOM_MAT_t139_t_t137_VALS, t_t137, out);
 }
 
 void fom_fused_t140(const fom_float *t_t137, const fom_float *t_t138, const fom_float *t_t139, fom_float *out) {
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
+    clip_step_600(t_t137, t_t138, t_t139, out);
 }
 
 void fom_fused_t141(const fom_float *t_t137, const fom_float *t_t138, const fom_float *t_t140, fom_float *out) {
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
+    relu_step_20(FOM_MAT_t141_t_t137_VALS, t_t137, t_t138, FOM_MAT_t141_t_t140_VALS, t_t140, out);
 }
 
 void fom_fused_t142(const fom_float *t_t137, const fom_float *t_t139, const fom_float *t_t140, fom_float *out) {
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
+    lin_step_30(FOM_MAT_t142_t_t137_VALS, t_t137, t_t139, FOM_MAT_t142_t_t140_VALS, t_t140, out);
 }
 
 void fom_fused_t143(const fom_float *t_t140, const fom_float *t_t141, const fom_float *t_t142, fom_float *out) {
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
+    clip_step_600(t_t140, t_t141, t_t142, out);
 }
 
 void fom_fused_t144(const fom_float *t_t140, const fom_float *t_t141, const fom_float *t_t143, fom_float *out) {
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
+    relu_step_20(FOM_MAT_t144_t_t140_VALS, t_t140, t_t141, FOM_MAT_t144_t_t143_VALS, t_t143, out);
 }
 
 void fom_fused_t145(const fom_float *t_t140, const fom_float *t_t142, const fom_float *t_t143, fom_float *out) {
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
+    lin_step_30(FOM_MAT_t145_t_t140_VALS, t_t140, t_t142, FOM_MAT_t145_t_t143_VALS, t_t143, out);
 }
 
 void fom_fused_t146(const fom_float *t_t143, const fom_float *t_t144, const fom_float *t_t145, fom_float *out) {
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
+    clip_step_600(t_t143, t_t144, t_t145, out);
 }
 
 void fom_fused_t147(const fom_float *t_t143, const fom_float *t_t144, const fom_float *t_t146, fom_float *out) {
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
+    relu_step_20(FOM_MAT_t147_t_t143_VALS, t_t143, t_t144, FOM_MAT_t147_t_t146_VALS, t_t146, out);
 }
 
 void fom_fused_t148(const fom_float *t_t143, const fom_float *t_t145, const fom_float *t_t146, fom_float *out) {
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
+    lin_step_30(FOM_MAT_t148_t_t143_VALS, t_t143, t_t145, FOM_MAT_t148_t_t146_VALS, t_t146, out);
 }
 
 void fom_fused_su(const fom_float *t_t146, const fom_float *t_t147, const fom_float *t_t148, fom_float *out) {
```

## Agent notes

```
### Round 1

**Whole-solve mean_us:** initial 108.88 µs, final about 50 µs (49.1 to 51.7 across runs). Overall speedup is about 2.2x.

**Final profile breakdown** (`make profile`, including timer overhead; bench mean 52.6 µs under profiling):

| Region | Total time | Calls | Share |
|---|---|---|---|
| `steps_600` | 127.5 ms | 250k | 51.9% |
| `steps_20_relu` | 65.8 ms | 250k | 26.8% |
| `steps_30_lin` | 46.6 ms | 245k | 19.0% |
| `state_update` | 5.6 ms | 5k | 2.3% |

**Optimizations applied:**
1. **Shared kernel for the 48 600-row fused steps (108.9 → 79.5 µs).** These are `fom_fused_t8`, `t11`, `t14` and the rest. They now call one static helper, `clip_step_600`. Three facts made this valid:
   - `t5_t_t2` is the identity selection, so its product is a plain `a[i]`.
   - `t2_t_t1` and `t5_t_t4` have exactly one entry per row (ROWPTR[i] = i), so the inner CSR loops and row-pointer loads go away.
   - All 48 steps share the same three member matrices.

   I checked each of these against `data.npz` before relying on it.
2. **Specialized kernels for the 49 20-row steps and 48 30-row steps (79.5 → 64.5 µs).**
   - In each 20x600 member, row i covers the contiguous columns 30i..30i+29, so COLIDX = p and the index gather is gone.
   - In each 30x600 member, row j covers columns j+30r. That is a fixed stride, so the index loads are gone.
   - The 20x20 and 30x30 members (`t3_t_t1`, `t7_t_t4`) are identity selections, applied as plain adds.
   - The helpers take the members' VALS pointers, so the coefficients still come from `fom_data.c`. Every pattern was checked in Python before rewriting.
3. **Removed the gathers in the 600-row kernel (62.6 → 51.4 µs).** `t2_t_t1` row 30i+k reads column i, and `t5_t_t4` row 30i+k reads column k. With that incidence structure the loop splits into 20 blocks of 30, each using a scalar broadcast plus a contiguous vector. That makes it vectorizable.
4. **Two accumulators in the 20-row reduction.** This one was neutral within noise.
5. **Profiling markers.** Every step in `fom_solve` is wrapped in its own scope by step type (`steps_600`, `steps_20_relu`, `steps_30_lin`), plus `state_update`.

Blocked by rules:
- Fusing the bias initialization and clip/relu activation into the fused kernel, which would save two passes over 600 entries for each of the 50 steps | the bias (t0_c) and the bounds (t0_lo/hi) are not member data of `fom_fused_*`, and the fused signature is fixed in the headers | ~5-10%
- Merging consecutive steps into one routine to keep intermediates in registers | `fom_iterate`/`fom_step_*` not exposed for this spec and headers fixed | unknown

### Round 2

**Whole-solve time:** initial (translated) 111.479 us → start of round 47.670 us → final about 45.0 us `make bench` mean_us (runs ranged 44.5–47.0; min 39–40 us vs 42–44 us before). That is about 2.48x overall versus the translation, and about 5% this round. `make verify` prints `VERIFY OK`.

**Final profile** (`make profile`, hottest first; the timer itself adds overhead at 250k calls per region):
- steps_600: 124255 us, 50.4%
- steps_20_relu: 67171 us, 27.3%
- steps_30_lin: 49293 us, 20.0%
- state_update: 5668 us, 2.3%

**Optimizations applied this round:**
- **`restrict` on the shared step kernels** (`clip_step_600`, `relu_step_20`, `lin_step_30` in `src/fom_ops.c`). Every fused output block writes a fresh tmp vector that never overlaps its sources or the constant matrix data. Saying so drops the compiler's runtime overlap checks and lets it vectorize and keep values in registers. Gain is about 4–5%.
- **Solver-side `init_600` / `clip_600` helpers** with `restrict` and hoisted `lo`/`hi` loads. All 50 clip steps share the same `t0_c`, `t0_lo` and `t0_hi` vectors (the spec reuses these blocks). The bound clip now has one alias-free loop body. Gain is about 1–2%.
- **Tried and reverted:** swapping the loops in `lin_step_30` (r outer, contiguous reads of the source). Its stride-20 coefficient access made it no faster.

**Earlier rounds' structure, still in place:**
- In the 600-row steps, `t5_t_t2` is an identity selection, and `t2_t_t1` and `t5_t_t4` have one entry per row with a broadcast index pattern.
- The 20-row steps use contiguous 30-column row blocks.
- The 30-row steps use a stride-30 column pattern.

Blocked by rules:
- Fold the `t0_c` init and the clip into the fused 600-row routine (one pass instead of three) | a fused routine may read only its member matrices' data, so it cannot read `t0_c`, `t0_lo` or `t0_hi`; there is no `fom_step_<block>` entry point for these blocks | about 5–10%
- Store `lin_step_30` coefficients transposed, built once at setup, for contiguous vector loads | `fom_setup` may not probe the matrices; precomputed tables are allowed only as `static const` literal tables inside each of about 49 fused routines, which was not feasible in this round's time | unknown, probably 3–5%
```
