#ifndef FOM_OPS_H
#define FOM_OPS_H

#include "fom_workspace.h"

#ifdef __cplusplus
extern "C" {
#endif

/* out += M @ x for each linear-op matrix M (CSR if structurally
   sparse, dense otherwise). Accumulating: zero `out` first. */

void fom_apply_t0_state(const fom_float *x, fom_float *out);  /* CSR 600x650 */
void fom_apply_t1_state(const fom_float *x, fom_float *out);  /* CSR 20x650 */
void fom_apply_t1_t_t0(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t2_state(const fom_float *x, fom_float *out);  /* CSR 600x650 */
void fom_apply_t2_p_p0(const fom_float *x, fom_float *out);  /* CSR 600x30 */
void fom_apply_t2_t_t0(const fom_float *x, fom_float *out);  /* CSR 600x600 */
void fom_apply_t2_t_t1(const fom_float *x, fom_float *out);  /* CSR 600x20 */
void fom_apply_t3_t_t0(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t3_t_t1(const fom_float *x, fom_float *out);  /* CSR 20x20 */
void fom_apply_t3_t_t2(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t4_state(const fom_float *x, fom_float *out);  /* CSR 30x650 */
void fom_apply_t4_p_p0(const fom_float *x, fom_float *out);  /* CSR 30x30 */
void fom_apply_t4_t_t0(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t4_t_t2(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t5_t_t2(const fom_float *x, fom_float *out);  /* CSR 600x600 */
void fom_apply_t5_t_t4(const fom_float *x, fom_float *out);  /* CSR 600x30 */
void fom_apply_t6_t_t2(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t6_t_t5(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t7_p_p0(const fom_float *x, fom_float *out);  /* CSR 30x30 */
void fom_apply_t7_t_t2(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t7_t_t4(const fom_float *x, fom_float *out);  /* CSR 30x30 */
void fom_apply_t7_t_t5(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t9_t_t5(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t9_t_t8(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t10_t_t5(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t10_t_t8(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t12_t_t8(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t12_t_t11(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t13_t_t8(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t13_t_t11(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t15_t_t11(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t15_t_t14(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t16_t_t11(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t16_t_t14(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t18_t_t14(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t18_t_t17(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t19_t_t14(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t19_t_t17(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t21_t_t17(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t21_t_t20(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t22_t_t17(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t22_t_t20(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t24_t_t20(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t24_t_t23(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t25_t_t20(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t25_t_t23(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t27_t_t23(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t27_t_t26(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t28_t_t23(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t28_t_t26(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t30_t_t26(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t30_t_t29(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t31_t_t26(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t31_t_t29(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t33_t_t29(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t33_t_t32(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t34_t_t29(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t34_t_t32(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t36_t_t32(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t36_t_t35(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t37_t_t32(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t37_t_t35(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t39_t_t35(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t39_t_t38(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t40_t_t35(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t40_t_t38(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t42_t_t38(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t42_t_t41(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t43_t_t38(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t43_t_t41(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t45_t_t41(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t45_t_t44(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t46_t_t41(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t46_t_t44(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t48_t_t44(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t48_t_t47(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t49_t_t44(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t49_t_t47(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t51_t_t47(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t51_t_t50(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t52_t_t47(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t52_t_t50(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t54_t_t50(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t54_t_t53(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t55_t_t50(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t55_t_t53(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t57_t_t53(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t57_t_t56(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t58_t_t53(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t58_t_t56(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t60_t_t56(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t60_t_t59(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t61_t_t56(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t61_t_t59(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t63_t_t59(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t63_t_t62(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t64_t_t59(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t64_t_t62(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t66_t_t62(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t66_t_t65(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t67_t_t62(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t67_t_t65(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t69_t_t65(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t69_t_t68(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t70_t_t65(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t70_t_t68(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t72_t_t68(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t72_t_t71(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t73_t_t68(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t73_t_t71(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t75_t_t71(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t75_t_t74(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t76_t_t71(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t76_t_t74(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t78_t_t74(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t78_t_t77(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t79_t_t74(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t79_t_t77(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t81_t_t77(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t81_t_t80(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t82_t_t77(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t82_t_t80(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t84_t_t80(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t84_t_t83(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t85_t_t80(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t85_t_t83(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t87_t_t83(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t87_t_t86(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t88_t_t83(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t88_t_t86(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t90_t_t86(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t90_t_t89(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t91_t_t86(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t91_t_t89(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t93_t_t89(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t93_t_t92(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t94_t_t89(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t94_t_t92(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t96_t_t92(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t96_t_t95(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t97_t_t92(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t97_t_t95(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t99_t_t95(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t99_t_t98(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t100_t_t95(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t100_t_t98(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t102_t_t98(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t102_t_t101(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t103_t_t98(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t103_t_t101(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t105_t_t101(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t105_t_t104(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t106_t_t101(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t106_t_t104(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t108_t_t104(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t108_t_t107(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t109_t_t104(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t109_t_t107(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t111_t_t107(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t111_t_t110(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t112_t_t107(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t112_t_t110(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t114_t_t110(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t114_t_t113(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t115_t_t110(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t115_t_t113(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t117_t_t113(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t117_t_t116(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t118_t_t113(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t118_t_t116(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t120_t_t116(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t120_t_t119(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t121_t_t116(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t121_t_t119(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t123_t_t119(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t123_t_t122(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t124_t_t119(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t124_t_t122(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t126_t_t122(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t126_t_t125(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t127_t_t122(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t127_t_t125(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t129_t_t125(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t129_t_t128(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t130_t_t125(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t130_t_t128(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t132_t_t128(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t132_t_t131(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t133_t_t128(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t133_t_t131(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t135_t_t131(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t135_t_t134(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t136_t_t131(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t136_t_t134(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t138_t_t134(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t138_t_t137(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t139_t_t134(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t139_t_t137(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t141_t_t137(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t141_t_t140(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t142_t_t137(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t142_t_t140(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t144_t_t140(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t144_t_t143(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t145_t_t140(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t145_t_t143(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t147_t_t143(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t147_t_t146(const fom_float *x, fom_float *out);  /* CSR 20x600 */
void fom_apply_t148_t_t143(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_t148_t_t146(const fom_float *x, fom_float *out);  /* CSR 30x600 */
void fom_apply_su_t_t146(const fom_float *x, fom_float *out);  /* CSR 650x600 */
void fom_apply_su_t_t147(const fom_float *x, fom_float *out);  /* CSR 650x20 */
void fom_apply_su_t_t148(const fom_float *x, fom_float *out);  /* CSR 650x30 */

/* Fused products: every per-iteration product of one output block in a
   single routine (parameter terms are left out, so they can be hoisted).
   Same accumulate convention; each is checked against its members. */
/* out += t1_state @ state + t1_t_t0 @ t_t0 */
void fom_fused_t1(const fom_float *state, const fom_float *t_t0, fom_float *out);
/* out += t2_state @ state + t2_t_t0 @ t_t0 + t2_t_t1 @ t_t1 */
void fom_fused_t2(const fom_float *state, const fom_float *t_t0, const fom_float *t_t1, fom_float *out);
/* out += t3_t_t0 @ t_t0 + t3_t_t1 @ t_t1 + t3_t_t2 @ t_t2 */
void fom_fused_t3(const fom_float *t_t0, const fom_float *t_t1, const fom_float *t_t2, fom_float *out);
/* out += t4_state @ state + t4_t_t0 @ t_t0 + t4_t_t2 @ t_t2 */
void fom_fused_t4(const fom_float *state, const fom_float *t_t0, const fom_float *t_t2, fom_float *out);
/* out += t5_t_t2 @ t_t2 + t2_t_t1 @ t_t3 + t5_t_t4 @ t_t4 */
void fom_fused_t5(const fom_float *t_t2, const fom_float *t_t3, const fom_float *t_t4, fom_float *out);
/* out += t6_t_t2 @ t_t2 + t3_t_t1 @ t_t3 + t6_t_t5 @ t_t5 */
void fom_fused_t6(const fom_float *t_t2, const fom_float *t_t3, const fom_float *t_t5, fom_float *out);
/* out += t7_t_t2 @ t_t2 + t7_t_t4 @ t_t4 + t7_t_t5 @ t_t5 */
void fom_fused_t7(const fom_float *t_t2, const fom_float *t_t4, const fom_float *t_t5, fom_float *out);
/* out += t5_t_t2 @ t_t5 + t2_t_t1 @ t_t6 + t5_t_t4 @ t_t7 */
void fom_fused_t8(const fom_float *t_t5, const fom_float *t_t6, const fom_float *t_t7, fom_float *out);
/* out += t9_t_t5 @ t_t5 + t3_t_t1 @ t_t6 + t9_t_t8 @ t_t8 */
void fom_fused_t9(const fom_float *t_t5, const fom_float *t_t6, const fom_float *t_t8, fom_float *out);
/* out += t10_t_t5 @ t_t5 + t7_t_t4 @ t_t7 + t10_t_t8 @ t_t8 */
void fom_fused_t10(const fom_float *t_t5, const fom_float *t_t7, const fom_float *t_t8, fom_float *out);
/* out += t5_t_t2 @ t_t8 + t2_t_t1 @ t_t9 + t5_t_t4 @ t_t10 */
void fom_fused_t11(const fom_float *t_t8, const fom_float *t_t9, const fom_float *t_t10, fom_float *out);
/* out += t12_t_t8 @ t_t8 + t3_t_t1 @ t_t9 + t12_t_t11 @ t_t11 */
void fom_fused_t12(const fom_float *t_t8, const fom_float *t_t9, const fom_float *t_t11, fom_float *out);
/* out += t13_t_t8 @ t_t8 + t7_t_t4 @ t_t10 + t13_t_t11 @ t_t11 */
void fom_fused_t13(const fom_float *t_t8, const fom_float *t_t10, const fom_float *t_t11, fom_float *out);
/* out += t5_t_t2 @ t_t11 + t2_t_t1 @ t_t12 + t5_t_t4 @ t_t13 */
void fom_fused_t14(const fom_float *t_t11, const fom_float *t_t12, const fom_float *t_t13, fom_float *out);
/* out += t15_t_t11 @ t_t11 + t3_t_t1 @ t_t12 + t15_t_t14 @ t_t14 */
void fom_fused_t15(const fom_float *t_t11, const fom_float *t_t12, const fom_float *t_t14, fom_float *out);
/* out += t16_t_t11 @ t_t11 + t7_t_t4 @ t_t13 + t16_t_t14 @ t_t14 */
void fom_fused_t16(const fom_float *t_t11, const fom_float *t_t13, const fom_float *t_t14, fom_float *out);
/* out += t5_t_t2 @ t_t14 + t2_t_t1 @ t_t15 + t5_t_t4 @ t_t16 */
void fom_fused_t17(const fom_float *t_t14, const fom_float *t_t15, const fom_float *t_t16, fom_float *out);
/* out += t18_t_t14 @ t_t14 + t3_t_t1 @ t_t15 + t18_t_t17 @ t_t17 */
void fom_fused_t18(const fom_float *t_t14, const fom_float *t_t15, const fom_float *t_t17, fom_float *out);
/* out += t19_t_t14 @ t_t14 + t7_t_t4 @ t_t16 + t19_t_t17 @ t_t17 */
void fom_fused_t19(const fom_float *t_t14, const fom_float *t_t16, const fom_float *t_t17, fom_float *out);
/* out += t5_t_t2 @ t_t17 + t2_t_t1 @ t_t18 + t5_t_t4 @ t_t19 */
void fom_fused_t20(const fom_float *t_t17, const fom_float *t_t18, const fom_float *t_t19, fom_float *out);
/* out += t21_t_t17 @ t_t17 + t3_t_t1 @ t_t18 + t21_t_t20 @ t_t20 */
void fom_fused_t21(const fom_float *t_t17, const fom_float *t_t18, const fom_float *t_t20, fom_float *out);
/* out += t22_t_t17 @ t_t17 + t7_t_t4 @ t_t19 + t22_t_t20 @ t_t20 */
void fom_fused_t22(const fom_float *t_t17, const fom_float *t_t19, const fom_float *t_t20, fom_float *out);
/* out += t5_t_t2 @ t_t20 + t2_t_t1 @ t_t21 + t5_t_t4 @ t_t22 */
void fom_fused_t23(const fom_float *t_t20, const fom_float *t_t21, const fom_float *t_t22, fom_float *out);
/* out += t24_t_t20 @ t_t20 + t3_t_t1 @ t_t21 + t24_t_t23 @ t_t23 */
void fom_fused_t24(const fom_float *t_t20, const fom_float *t_t21, const fom_float *t_t23, fom_float *out);
/* out += t25_t_t20 @ t_t20 + t7_t_t4 @ t_t22 + t25_t_t23 @ t_t23 */
void fom_fused_t25(const fom_float *t_t20, const fom_float *t_t22, const fom_float *t_t23, fom_float *out);
/* out += t5_t_t2 @ t_t23 + t2_t_t1 @ t_t24 + t5_t_t4 @ t_t25 */
void fom_fused_t26(const fom_float *t_t23, const fom_float *t_t24, const fom_float *t_t25, fom_float *out);
/* out += t27_t_t23 @ t_t23 + t3_t_t1 @ t_t24 + t27_t_t26 @ t_t26 */
void fom_fused_t27(const fom_float *t_t23, const fom_float *t_t24, const fom_float *t_t26, fom_float *out);
/* out += t28_t_t23 @ t_t23 + t7_t_t4 @ t_t25 + t28_t_t26 @ t_t26 */
void fom_fused_t28(const fom_float *t_t23, const fom_float *t_t25, const fom_float *t_t26, fom_float *out);
/* out += t5_t_t2 @ t_t26 + t2_t_t1 @ t_t27 + t5_t_t4 @ t_t28 */
void fom_fused_t29(const fom_float *t_t26, const fom_float *t_t27, const fom_float *t_t28, fom_float *out);
/* out += t30_t_t26 @ t_t26 + t3_t_t1 @ t_t27 + t30_t_t29 @ t_t29 */
void fom_fused_t30(const fom_float *t_t26, const fom_float *t_t27, const fom_float *t_t29, fom_float *out);
/* out += t31_t_t26 @ t_t26 + t7_t_t4 @ t_t28 + t31_t_t29 @ t_t29 */
void fom_fused_t31(const fom_float *t_t26, const fom_float *t_t28, const fom_float *t_t29, fom_float *out);
/* out += t5_t_t2 @ t_t29 + t2_t_t1 @ t_t30 + t5_t_t4 @ t_t31 */
void fom_fused_t32(const fom_float *t_t29, const fom_float *t_t30, const fom_float *t_t31, fom_float *out);
/* out += t33_t_t29 @ t_t29 + t3_t_t1 @ t_t30 + t33_t_t32 @ t_t32 */
void fom_fused_t33(const fom_float *t_t29, const fom_float *t_t30, const fom_float *t_t32, fom_float *out);
/* out += t34_t_t29 @ t_t29 + t7_t_t4 @ t_t31 + t34_t_t32 @ t_t32 */
void fom_fused_t34(const fom_float *t_t29, const fom_float *t_t31, const fom_float *t_t32, fom_float *out);
/* out += t5_t_t2 @ t_t32 + t2_t_t1 @ t_t33 + t5_t_t4 @ t_t34 */
void fom_fused_t35(const fom_float *t_t32, const fom_float *t_t33, const fom_float *t_t34, fom_float *out);
/* out += t36_t_t32 @ t_t32 + t3_t_t1 @ t_t33 + t36_t_t35 @ t_t35 */
void fom_fused_t36(const fom_float *t_t32, const fom_float *t_t33, const fom_float *t_t35, fom_float *out);
/* out += t37_t_t32 @ t_t32 + t7_t_t4 @ t_t34 + t37_t_t35 @ t_t35 */
void fom_fused_t37(const fom_float *t_t32, const fom_float *t_t34, const fom_float *t_t35, fom_float *out);
/* out += t5_t_t2 @ t_t35 + t2_t_t1 @ t_t36 + t5_t_t4 @ t_t37 */
void fom_fused_t38(const fom_float *t_t35, const fom_float *t_t36, const fom_float *t_t37, fom_float *out);
/* out += t39_t_t35 @ t_t35 + t3_t_t1 @ t_t36 + t39_t_t38 @ t_t38 */
void fom_fused_t39(const fom_float *t_t35, const fom_float *t_t36, const fom_float *t_t38, fom_float *out);
/* out += t40_t_t35 @ t_t35 + t7_t_t4 @ t_t37 + t40_t_t38 @ t_t38 */
void fom_fused_t40(const fom_float *t_t35, const fom_float *t_t37, const fom_float *t_t38, fom_float *out);
/* out += t5_t_t2 @ t_t38 + t2_t_t1 @ t_t39 + t5_t_t4 @ t_t40 */
void fom_fused_t41(const fom_float *t_t38, const fom_float *t_t39, const fom_float *t_t40, fom_float *out);
/* out += t42_t_t38 @ t_t38 + t3_t_t1 @ t_t39 + t42_t_t41 @ t_t41 */
void fom_fused_t42(const fom_float *t_t38, const fom_float *t_t39, const fom_float *t_t41, fom_float *out);
/* out += t43_t_t38 @ t_t38 + t7_t_t4 @ t_t40 + t43_t_t41 @ t_t41 */
void fom_fused_t43(const fom_float *t_t38, const fom_float *t_t40, const fom_float *t_t41, fom_float *out);
/* out += t5_t_t2 @ t_t41 + t2_t_t1 @ t_t42 + t5_t_t4 @ t_t43 */
void fom_fused_t44(const fom_float *t_t41, const fom_float *t_t42, const fom_float *t_t43, fom_float *out);
/* out += t45_t_t41 @ t_t41 + t3_t_t1 @ t_t42 + t45_t_t44 @ t_t44 */
void fom_fused_t45(const fom_float *t_t41, const fom_float *t_t42, const fom_float *t_t44, fom_float *out);
/* out += t46_t_t41 @ t_t41 + t7_t_t4 @ t_t43 + t46_t_t44 @ t_t44 */
void fom_fused_t46(const fom_float *t_t41, const fom_float *t_t43, const fom_float *t_t44, fom_float *out);
/* out += t5_t_t2 @ t_t44 + t2_t_t1 @ t_t45 + t5_t_t4 @ t_t46 */
void fom_fused_t47(const fom_float *t_t44, const fom_float *t_t45, const fom_float *t_t46, fom_float *out);
/* out += t48_t_t44 @ t_t44 + t3_t_t1 @ t_t45 + t48_t_t47 @ t_t47 */
void fom_fused_t48(const fom_float *t_t44, const fom_float *t_t45, const fom_float *t_t47, fom_float *out);
/* out += t49_t_t44 @ t_t44 + t7_t_t4 @ t_t46 + t49_t_t47 @ t_t47 */
void fom_fused_t49(const fom_float *t_t44, const fom_float *t_t46, const fom_float *t_t47, fom_float *out);
/* out += t5_t_t2 @ t_t47 + t2_t_t1 @ t_t48 + t5_t_t4 @ t_t49 */
void fom_fused_t50(const fom_float *t_t47, const fom_float *t_t48, const fom_float *t_t49, fom_float *out);
/* out += t51_t_t47 @ t_t47 + t3_t_t1 @ t_t48 + t51_t_t50 @ t_t50 */
void fom_fused_t51(const fom_float *t_t47, const fom_float *t_t48, const fom_float *t_t50, fom_float *out);
/* out += t52_t_t47 @ t_t47 + t7_t_t4 @ t_t49 + t52_t_t50 @ t_t50 */
void fom_fused_t52(const fom_float *t_t47, const fom_float *t_t49, const fom_float *t_t50, fom_float *out);
/* out += t5_t_t2 @ t_t50 + t2_t_t1 @ t_t51 + t5_t_t4 @ t_t52 */
void fom_fused_t53(const fom_float *t_t50, const fom_float *t_t51, const fom_float *t_t52, fom_float *out);
/* out += t54_t_t50 @ t_t50 + t3_t_t1 @ t_t51 + t54_t_t53 @ t_t53 */
void fom_fused_t54(const fom_float *t_t50, const fom_float *t_t51, const fom_float *t_t53, fom_float *out);
/* out += t55_t_t50 @ t_t50 + t7_t_t4 @ t_t52 + t55_t_t53 @ t_t53 */
void fom_fused_t55(const fom_float *t_t50, const fom_float *t_t52, const fom_float *t_t53, fom_float *out);
/* out += t5_t_t2 @ t_t53 + t2_t_t1 @ t_t54 + t5_t_t4 @ t_t55 */
void fom_fused_t56(const fom_float *t_t53, const fom_float *t_t54, const fom_float *t_t55, fom_float *out);
/* out += t57_t_t53 @ t_t53 + t3_t_t1 @ t_t54 + t57_t_t56 @ t_t56 */
void fom_fused_t57(const fom_float *t_t53, const fom_float *t_t54, const fom_float *t_t56, fom_float *out);
/* out += t58_t_t53 @ t_t53 + t7_t_t4 @ t_t55 + t58_t_t56 @ t_t56 */
void fom_fused_t58(const fom_float *t_t53, const fom_float *t_t55, const fom_float *t_t56, fom_float *out);
/* out += t5_t_t2 @ t_t56 + t2_t_t1 @ t_t57 + t5_t_t4 @ t_t58 */
void fom_fused_t59(const fom_float *t_t56, const fom_float *t_t57, const fom_float *t_t58, fom_float *out);
/* out += t60_t_t56 @ t_t56 + t3_t_t1 @ t_t57 + t60_t_t59 @ t_t59 */
void fom_fused_t60(const fom_float *t_t56, const fom_float *t_t57, const fom_float *t_t59, fom_float *out);
/* out += t61_t_t56 @ t_t56 + t7_t_t4 @ t_t58 + t61_t_t59 @ t_t59 */
void fom_fused_t61(const fom_float *t_t56, const fom_float *t_t58, const fom_float *t_t59, fom_float *out);
/* out += t5_t_t2 @ t_t59 + t2_t_t1 @ t_t60 + t5_t_t4 @ t_t61 */
void fom_fused_t62(const fom_float *t_t59, const fom_float *t_t60, const fom_float *t_t61, fom_float *out);
/* out += t63_t_t59 @ t_t59 + t3_t_t1 @ t_t60 + t63_t_t62 @ t_t62 */
void fom_fused_t63(const fom_float *t_t59, const fom_float *t_t60, const fom_float *t_t62, fom_float *out);
/* out += t64_t_t59 @ t_t59 + t7_t_t4 @ t_t61 + t64_t_t62 @ t_t62 */
void fom_fused_t64(const fom_float *t_t59, const fom_float *t_t61, const fom_float *t_t62, fom_float *out);
/* out += t5_t_t2 @ t_t62 + t2_t_t1 @ t_t63 + t5_t_t4 @ t_t64 */
void fom_fused_t65(const fom_float *t_t62, const fom_float *t_t63, const fom_float *t_t64, fom_float *out);
/* out += t66_t_t62 @ t_t62 + t3_t_t1 @ t_t63 + t66_t_t65 @ t_t65 */
void fom_fused_t66(const fom_float *t_t62, const fom_float *t_t63, const fom_float *t_t65, fom_float *out);
/* out += t67_t_t62 @ t_t62 + t7_t_t4 @ t_t64 + t67_t_t65 @ t_t65 */
void fom_fused_t67(const fom_float *t_t62, const fom_float *t_t64, const fom_float *t_t65, fom_float *out);
/* out += t5_t_t2 @ t_t65 + t2_t_t1 @ t_t66 + t5_t_t4 @ t_t67 */
void fom_fused_t68(const fom_float *t_t65, const fom_float *t_t66, const fom_float *t_t67, fom_float *out);
/* out += t69_t_t65 @ t_t65 + t3_t_t1 @ t_t66 + t69_t_t68 @ t_t68 */
void fom_fused_t69(const fom_float *t_t65, const fom_float *t_t66, const fom_float *t_t68, fom_float *out);
/* out += t70_t_t65 @ t_t65 + t7_t_t4 @ t_t67 + t70_t_t68 @ t_t68 */
void fom_fused_t70(const fom_float *t_t65, const fom_float *t_t67, const fom_float *t_t68, fom_float *out);
/* out += t5_t_t2 @ t_t68 + t2_t_t1 @ t_t69 + t5_t_t4 @ t_t70 */
void fom_fused_t71(const fom_float *t_t68, const fom_float *t_t69, const fom_float *t_t70, fom_float *out);
/* out += t72_t_t68 @ t_t68 + t3_t_t1 @ t_t69 + t72_t_t71 @ t_t71 */
void fom_fused_t72(const fom_float *t_t68, const fom_float *t_t69, const fom_float *t_t71, fom_float *out);
/* out += t73_t_t68 @ t_t68 + t7_t_t4 @ t_t70 + t73_t_t71 @ t_t71 */
void fom_fused_t73(const fom_float *t_t68, const fom_float *t_t70, const fom_float *t_t71, fom_float *out);
/* out += t5_t_t2 @ t_t71 + t2_t_t1 @ t_t72 + t5_t_t4 @ t_t73 */
void fom_fused_t74(const fom_float *t_t71, const fom_float *t_t72, const fom_float *t_t73, fom_float *out);
/* out += t75_t_t71 @ t_t71 + t3_t_t1 @ t_t72 + t75_t_t74 @ t_t74 */
void fom_fused_t75(const fom_float *t_t71, const fom_float *t_t72, const fom_float *t_t74, fom_float *out);
/* out += t76_t_t71 @ t_t71 + t7_t_t4 @ t_t73 + t76_t_t74 @ t_t74 */
void fom_fused_t76(const fom_float *t_t71, const fom_float *t_t73, const fom_float *t_t74, fom_float *out);
/* out += t5_t_t2 @ t_t74 + t2_t_t1 @ t_t75 + t5_t_t4 @ t_t76 */
void fom_fused_t77(const fom_float *t_t74, const fom_float *t_t75, const fom_float *t_t76, fom_float *out);
/* out += t78_t_t74 @ t_t74 + t3_t_t1 @ t_t75 + t78_t_t77 @ t_t77 */
void fom_fused_t78(const fom_float *t_t74, const fom_float *t_t75, const fom_float *t_t77, fom_float *out);
/* out += t79_t_t74 @ t_t74 + t7_t_t4 @ t_t76 + t79_t_t77 @ t_t77 */
void fom_fused_t79(const fom_float *t_t74, const fom_float *t_t76, const fom_float *t_t77, fom_float *out);
/* out += t5_t_t2 @ t_t77 + t2_t_t1 @ t_t78 + t5_t_t4 @ t_t79 */
void fom_fused_t80(const fom_float *t_t77, const fom_float *t_t78, const fom_float *t_t79, fom_float *out);
/* out += t81_t_t77 @ t_t77 + t3_t_t1 @ t_t78 + t81_t_t80 @ t_t80 */
void fom_fused_t81(const fom_float *t_t77, const fom_float *t_t78, const fom_float *t_t80, fom_float *out);
/* out += t82_t_t77 @ t_t77 + t7_t_t4 @ t_t79 + t82_t_t80 @ t_t80 */
void fom_fused_t82(const fom_float *t_t77, const fom_float *t_t79, const fom_float *t_t80, fom_float *out);
/* out += t5_t_t2 @ t_t80 + t2_t_t1 @ t_t81 + t5_t_t4 @ t_t82 */
void fom_fused_t83(const fom_float *t_t80, const fom_float *t_t81, const fom_float *t_t82, fom_float *out);
/* out += t84_t_t80 @ t_t80 + t3_t_t1 @ t_t81 + t84_t_t83 @ t_t83 */
void fom_fused_t84(const fom_float *t_t80, const fom_float *t_t81, const fom_float *t_t83, fom_float *out);
/* out += t85_t_t80 @ t_t80 + t7_t_t4 @ t_t82 + t85_t_t83 @ t_t83 */
void fom_fused_t85(const fom_float *t_t80, const fom_float *t_t82, const fom_float *t_t83, fom_float *out);
/* out += t5_t_t2 @ t_t83 + t2_t_t1 @ t_t84 + t5_t_t4 @ t_t85 */
void fom_fused_t86(const fom_float *t_t83, const fom_float *t_t84, const fom_float *t_t85, fom_float *out);
/* out += t87_t_t83 @ t_t83 + t3_t_t1 @ t_t84 + t87_t_t86 @ t_t86 */
void fom_fused_t87(const fom_float *t_t83, const fom_float *t_t84, const fom_float *t_t86, fom_float *out);
/* out += t88_t_t83 @ t_t83 + t7_t_t4 @ t_t85 + t88_t_t86 @ t_t86 */
void fom_fused_t88(const fom_float *t_t83, const fom_float *t_t85, const fom_float *t_t86, fom_float *out);
/* out += t5_t_t2 @ t_t86 + t2_t_t1 @ t_t87 + t5_t_t4 @ t_t88 */
void fom_fused_t89(const fom_float *t_t86, const fom_float *t_t87, const fom_float *t_t88, fom_float *out);
/* out += t90_t_t86 @ t_t86 + t3_t_t1 @ t_t87 + t90_t_t89 @ t_t89 */
void fom_fused_t90(const fom_float *t_t86, const fom_float *t_t87, const fom_float *t_t89, fom_float *out);
/* out += t91_t_t86 @ t_t86 + t7_t_t4 @ t_t88 + t91_t_t89 @ t_t89 */
void fom_fused_t91(const fom_float *t_t86, const fom_float *t_t88, const fom_float *t_t89, fom_float *out);
/* out += t5_t_t2 @ t_t89 + t2_t_t1 @ t_t90 + t5_t_t4 @ t_t91 */
void fom_fused_t92(const fom_float *t_t89, const fom_float *t_t90, const fom_float *t_t91, fom_float *out);
/* out += t93_t_t89 @ t_t89 + t3_t_t1 @ t_t90 + t93_t_t92 @ t_t92 */
void fom_fused_t93(const fom_float *t_t89, const fom_float *t_t90, const fom_float *t_t92, fom_float *out);
/* out += t94_t_t89 @ t_t89 + t7_t_t4 @ t_t91 + t94_t_t92 @ t_t92 */
void fom_fused_t94(const fom_float *t_t89, const fom_float *t_t91, const fom_float *t_t92, fom_float *out);
/* out += t5_t_t2 @ t_t92 + t2_t_t1 @ t_t93 + t5_t_t4 @ t_t94 */
void fom_fused_t95(const fom_float *t_t92, const fom_float *t_t93, const fom_float *t_t94, fom_float *out);
/* out += t96_t_t92 @ t_t92 + t3_t_t1 @ t_t93 + t96_t_t95 @ t_t95 */
void fom_fused_t96(const fom_float *t_t92, const fom_float *t_t93, const fom_float *t_t95, fom_float *out);
/* out += t97_t_t92 @ t_t92 + t7_t_t4 @ t_t94 + t97_t_t95 @ t_t95 */
void fom_fused_t97(const fom_float *t_t92, const fom_float *t_t94, const fom_float *t_t95, fom_float *out);
/* out += t5_t_t2 @ t_t95 + t2_t_t1 @ t_t96 + t5_t_t4 @ t_t97 */
void fom_fused_t98(const fom_float *t_t95, const fom_float *t_t96, const fom_float *t_t97, fom_float *out);
/* out += t99_t_t95 @ t_t95 + t3_t_t1 @ t_t96 + t99_t_t98 @ t_t98 */
void fom_fused_t99(const fom_float *t_t95, const fom_float *t_t96, const fom_float *t_t98, fom_float *out);
/* out += t100_t_t95 @ t_t95 + t7_t_t4 @ t_t97 + t100_t_t98 @ t_t98 */
void fom_fused_t100(const fom_float *t_t95, const fom_float *t_t97, const fom_float *t_t98, fom_float *out);
/* out += t5_t_t2 @ t_t98 + t2_t_t1 @ t_t99 + t5_t_t4 @ t_t100 */
void fom_fused_t101(const fom_float *t_t98, const fom_float *t_t99, const fom_float *t_t100, fom_float *out);
/* out += t102_t_t98 @ t_t98 + t3_t_t1 @ t_t99 + t102_t_t101 @ t_t101 */
void fom_fused_t102(const fom_float *t_t98, const fom_float *t_t99, const fom_float *t_t101, fom_float *out);
/* out += t103_t_t98 @ t_t98 + t7_t_t4 @ t_t100 + t103_t_t101 @ t_t101 */
void fom_fused_t103(const fom_float *t_t98, const fom_float *t_t100, const fom_float *t_t101, fom_float *out);
/* out += t5_t_t2 @ t_t101 + t2_t_t1 @ t_t102 + t5_t_t4 @ t_t103 */
void fom_fused_t104(const fom_float *t_t101, const fom_float *t_t102, const fom_float *t_t103, fom_float *out);
/* out += t105_t_t101 @ t_t101 + t3_t_t1 @ t_t102 + t105_t_t104 @ t_t104 */
void fom_fused_t105(const fom_float *t_t101, const fom_float *t_t102, const fom_float *t_t104, fom_float *out);
/* out += t106_t_t101 @ t_t101 + t7_t_t4 @ t_t103 + t106_t_t104 @ t_t104 */
void fom_fused_t106(const fom_float *t_t101, const fom_float *t_t103, const fom_float *t_t104, fom_float *out);
/* out += t5_t_t2 @ t_t104 + t2_t_t1 @ t_t105 + t5_t_t4 @ t_t106 */
void fom_fused_t107(const fom_float *t_t104, const fom_float *t_t105, const fom_float *t_t106, fom_float *out);
/* out += t108_t_t104 @ t_t104 + t3_t_t1 @ t_t105 + t108_t_t107 @ t_t107 */
void fom_fused_t108(const fom_float *t_t104, const fom_float *t_t105, const fom_float *t_t107, fom_float *out);
/* out += t109_t_t104 @ t_t104 + t7_t_t4 @ t_t106 + t109_t_t107 @ t_t107 */
void fom_fused_t109(const fom_float *t_t104, const fom_float *t_t106, const fom_float *t_t107, fom_float *out);
/* out += t5_t_t2 @ t_t107 + t2_t_t1 @ t_t108 + t5_t_t4 @ t_t109 */
void fom_fused_t110(const fom_float *t_t107, const fom_float *t_t108, const fom_float *t_t109, fom_float *out);
/* out += t111_t_t107 @ t_t107 + t3_t_t1 @ t_t108 + t111_t_t110 @ t_t110 */
void fom_fused_t111(const fom_float *t_t107, const fom_float *t_t108, const fom_float *t_t110, fom_float *out);
/* out += t112_t_t107 @ t_t107 + t7_t_t4 @ t_t109 + t112_t_t110 @ t_t110 */
void fom_fused_t112(const fom_float *t_t107, const fom_float *t_t109, const fom_float *t_t110, fom_float *out);
/* out += t5_t_t2 @ t_t110 + t2_t_t1 @ t_t111 + t5_t_t4 @ t_t112 */
void fom_fused_t113(const fom_float *t_t110, const fom_float *t_t111, const fom_float *t_t112, fom_float *out);
/* out += t114_t_t110 @ t_t110 + t3_t_t1 @ t_t111 + t114_t_t113 @ t_t113 */
void fom_fused_t114(const fom_float *t_t110, const fom_float *t_t111, const fom_float *t_t113, fom_float *out);
/* out += t115_t_t110 @ t_t110 + t7_t_t4 @ t_t112 + t115_t_t113 @ t_t113 */
void fom_fused_t115(const fom_float *t_t110, const fom_float *t_t112, const fom_float *t_t113, fom_float *out);
/* out += t5_t_t2 @ t_t113 + t2_t_t1 @ t_t114 + t5_t_t4 @ t_t115 */
void fom_fused_t116(const fom_float *t_t113, const fom_float *t_t114, const fom_float *t_t115, fom_float *out);
/* out += t117_t_t113 @ t_t113 + t3_t_t1 @ t_t114 + t117_t_t116 @ t_t116 */
void fom_fused_t117(const fom_float *t_t113, const fom_float *t_t114, const fom_float *t_t116, fom_float *out);
/* out += t118_t_t113 @ t_t113 + t7_t_t4 @ t_t115 + t118_t_t116 @ t_t116 */
void fom_fused_t118(const fom_float *t_t113, const fom_float *t_t115, const fom_float *t_t116, fom_float *out);
/* out += t5_t_t2 @ t_t116 + t2_t_t1 @ t_t117 + t5_t_t4 @ t_t118 */
void fom_fused_t119(const fom_float *t_t116, const fom_float *t_t117, const fom_float *t_t118, fom_float *out);
/* out += t120_t_t116 @ t_t116 + t3_t_t1 @ t_t117 + t120_t_t119 @ t_t119 */
void fom_fused_t120(const fom_float *t_t116, const fom_float *t_t117, const fom_float *t_t119, fom_float *out);
/* out += t121_t_t116 @ t_t116 + t7_t_t4 @ t_t118 + t121_t_t119 @ t_t119 */
void fom_fused_t121(const fom_float *t_t116, const fom_float *t_t118, const fom_float *t_t119, fom_float *out);
/* out += t5_t_t2 @ t_t119 + t2_t_t1 @ t_t120 + t5_t_t4 @ t_t121 */
void fom_fused_t122(const fom_float *t_t119, const fom_float *t_t120, const fom_float *t_t121, fom_float *out);
/* out += t123_t_t119 @ t_t119 + t3_t_t1 @ t_t120 + t123_t_t122 @ t_t122 */
void fom_fused_t123(const fom_float *t_t119, const fom_float *t_t120, const fom_float *t_t122, fom_float *out);
/* out += t124_t_t119 @ t_t119 + t7_t_t4 @ t_t121 + t124_t_t122 @ t_t122 */
void fom_fused_t124(const fom_float *t_t119, const fom_float *t_t121, const fom_float *t_t122, fom_float *out);
/* out += t5_t_t2 @ t_t122 + t2_t_t1 @ t_t123 + t5_t_t4 @ t_t124 */
void fom_fused_t125(const fom_float *t_t122, const fom_float *t_t123, const fom_float *t_t124, fom_float *out);
/* out += t126_t_t122 @ t_t122 + t3_t_t1 @ t_t123 + t126_t_t125 @ t_t125 */
void fom_fused_t126(const fom_float *t_t122, const fom_float *t_t123, const fom_float *t_t125, fom_float *out);
/* out += t127_t_t122 @ t_t122 + t7_t_t4 @ t_t124 + t127_t_t125 @ t_t125 */
void fom_fused_t127(const fom_float *t_t122, const fom_float *t_t124, const fom_float *t_t125, fom_float *out);
/* out += t5_t_t2 @ t_t125 + t2_t_t1 @ t_t126 + t5_t_t4 @ t_t127 */
void fom_fused_t128(const fom_float *t_t125, const fom_float *t_t126, const fom_float *t_t127, fom_float *out);
/* out += t129_t_t125 @ t_t125 + t3_t_t1 @ t_t126 + t129_t_t128 @ t_t128 */
void fom_fused_t129(const fom_float *t_t125, const fom_float *t_t126, const fom_float *t_t128, fom_float *out);
/* out += t130_t_t125 @ t_t125 + t7_t_t4 @ t_t127 + t130_t_t128 @ t_t128 */
void fom_fused_t130(const fom_float *t_t125, const fom_float *t_t127, const fom_float *t_t128, fom_float *out);
/* out += t5_t_t2 @ t_t128 + t2_t_t1 @ t_t129 + t5_t_t4 @ t_t130 */
void fom_fused_t131(const fom_float *t_t128, const fom_float *t_t129, const fom_float *t_t130, fom_float *out);
/* out += t132_t_t128 @ t_t128 + t3_t_t1 @ t_t129 + t132_t_t131 @ t_t131 */
void fom_fused_t132(const fom_float *t_t128, const fom_float *t_t129, const fom_float *t_t131, fom_float *out);
/* out += t133_t_t128 @ t_t128 + t7_t_t4 @ t_t130 + t133_t_t131 @ t_t131 */
void fom_fused_t133(const fom_float *t_t128, const fom_float *t_t130, const fom_float *t_t131, fom_float *out);
/* out += t5_t_t2 @ t_t131 + t2_t_t1 @ t_t132 + t5_t_t4 @ t_t133 */
void fom_fused_t134(const fom_float *t_t131, const fom_float *t_t132, const fom_float *t_t133, fom_float *out);
/* out += t135_t_t131 @ t_t131 + t3_t_t1 @ t_t132 + t135_t_t134 @ t_t134 */
void fom_fused_t135(const fom_float *t_t131, const fom_float *t_t132, const fom_float *t_t134, fom_float *out);
/* out += t136_t_t131 @ t_t131 + t7_t_t4 @ t_t133 + t136_t_t134 @ t_t134 */
void fom_fused_t136(const fom_float *t_t131, const fom_float *t_t133, const fom_float *t_t134, fom_float *out);
/* out += t5_t_t2 @ t_t134 + t2_t_t1 @ t_t135 + t5_t_t4 @ t_t136 */
void fom_fused_t137(const fom_float *t_t134, const fom_float *t_t135, const fom_float *t_t136, fom_float *out);
/* out += t138_t_t134 @ t_t134 + t3_t_t1 @ t_t135 + t138_t_t137 @ t_t137 */
void fom_fused_t138(const fom_float *t_t134, const fom_float *t_t135, const fom_float *t_t137, fom_float *out);
/* out += t139_t_t134 @ t_t134 + t7_t_t4 @ t_t136 + t139_t_t137 @ t_t137 */
void fom_fused_t139(const fom_float *t_t134, const fom_float *t_t136, const fom_float *t_t137, fom_float *out);
/* out += t5_t_t2 @ t_t137 + t2_t_t1 @ t_t138 + t5_t_t4 @ t_t139 */
void fom_fused_t140(const fom_float *t_t137, const fom_float *t_t138, const fom_float *t_t139, fom_float *out);
/* out += t141_t_t137 @ t_t137 + t3_t_t1 @ t_t138 + t141_t_t140 @ t_t140 */
void fom_fused_t141(const fom_float *t_t137, const fom_float *t_t138, const fom_float *t_t140, fom_float *out);
/* out += t142_t_t137 @ t_t137 + t7_t_t4 @ t_t139 + t142_t_t140 @ t_t140 */
void fom_fused_t142(const fom_float *t_t137, const fom_float *t_t139, const fom_float *t_t140, fom_float *out);
/* out += t5_t_t2 @ t_t140 + t2_t_t1 @ t_t141 + t5_t_t4 @ t_t142 */
void fom_fused_t143(const fom_float *t_t140, const fom_float *t_t141, const fom_float *t_t142, fom_float *out);
/* out += t144_t_t140 @ t_t140 + t3_t_t1 @ t_t141 + t144_t_t143 @ t_t143 */
void fom_fused_t144(const fom_float *t_t140, const fom_float *t_t141, const fom_float *t_t143, fom_float *out);
/* out += t145_t_t140 @ t_t140 + t7_t_t4 @ t_t142 + t145_t_t143 @ t_t143 */
void fom_fused_t145(const fom_float *t_t140, const fom_float *t_t142, const fom_float *t_t143, fom_float *out);
/* out += t5_t_t2 @ t_t143 + t2_t_t1 @ t_t144 + t5_t_t4 @ t_t145 */
void fom_fused_t146(const fom_float *t_t143, const fom_float *t_t144, const fom_float *t_t145, fom_float *out);
/* out += t147_t_t143 @ t_t143 + t3_t_t1 @ t_t144 + t147_t_t146 @ t_t146 */
void fom_fused_t147(const fom_float *t_t143, const fom_float *t_t144, const fom_float *t_t146, fom_float *out);
/* out += t148_t_t143 @ t_t143 + t7_t_t4 @ t_t145 + t148_t_t146 @ t_t146 */
void fom_fused_t148(const fom_float *t_t143, const fom_float *t_t145, const fom_float *t_t146, fom_float *out);
/* out += su_t_t146 @ t_t146 + su_t_t147 @ t_t147 + su_t_t148 @ t_t148 */
void fom_fused_su(const fom_float *t_t146, const fom_float *t_t147, const fom_float *t_t148, fom_float *out);

#ifdef __cplusplus
}
#endif

#endif /* FOM_OPS_H */
