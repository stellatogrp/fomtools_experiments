#ifndef FOM_WORKSPACE_H
#define FOM_WORKSPACE_H

typedef double fom_float;
typedef int fom_int;

#define FOM_STATE_DIM 650
#define FOM_MAX_ITER 1
#define FOM_TOL ((fom_float)0)
#define FOM_USE_TOL 0
#define FOM_CHECK_TERMINATION 1
#define FOM_NUM_PARAMS 1
#define FOM_MAX_VEC_DIM 650
#define FOM_UNROLLED 1

#define FOM_SOLVED 0
#define FOM_MAX_ITER_REACHED 1

#define FOM_PARAM_P0_DIM 30
#define FOM_T0_DIM 600
#define FOM_T1_DIM 20
#define FOM_T2_DIM 600
#define FOM_T3_DIM 20
#define FOM_T4_DIM 30
#define FOM_T5_DIM 600
#define FOM_T6_DIM 20
#define FOM_T7_DIM 30
#define FOM_T8_DIM 600
#define FOM_T9_DIM 20
#define FOM_T10_DIM 30
#define FOM_T11_DIM 600
#define FOM_T12_DIM 20
#define FOM_T13_DIM 30
#define FOM_T14_DIM 600
#define FOM_T15_DIM 20
#define FOM_T16_DIM 30
#define FOM_T17_DIM 600
#define FOM_T18_DIM 20
#define FOM_T19_DIM 30
#define FOM_T20_DIM 600
#define FOM_T21_DIM 20
#define FOM_T22_DIM 30
#define FOM_T23_DIM 600
#define FOM_T24_DIM 20
#define FOM_T25_DIM 30
#define FOM_T26_DIM 600
#define FOM_T27_DIM 20
#define FOM_T28_DIM 30
#define FOM_T29_DIM 600
#define FOM_T30_DIM 20
#define FOM_T31_DIM 30
#define FOM_T32_DIM 600
#define FOM_T33_DIM 20
#define FOM_T34_DIM 30
#define FOM_T35_DIM 600
#define FOM_T36_DIM 20
#define FOM_T37_DIM 30
#define FOM_T38_DIM 600
#define FOM_T39_DIM 20
#define FOM_T40_DIM 30
#define FOM_T41_DIM 600
#define FOM_T42_DIM 20
#define FOM_T43_DIM 30
#define FOM_T44_DIM 600
#define FOM_T45_DIM 20
#define FOM_T46_DIM 30
#define FOM_T47_DIM 600
#define FOM_T48_DIM 20
#define FOM_T49_DIM 30
#define FOM_T50_DIM 600
#define FOM_T51_DIM 20
#define FOM_T52_DIM 30
#define FOM_T53_DIM 600
#define FOM_T54_DIM 20
#define FOM_T55_DIM 30
#define FOM_T56_DIM 600
#define FOM_T57_DIM 20
#define FOM_T58_DIM 30
#define FOM_T59_DIM 600
#define FOM_T60_DIM 20
#define FOM_T61_DIM 30
#define FOM_T62_DIM 600
#define FOM_T63_DIM 20
#define FOM_T64_DIM 30
#define FOM_T65_DIM 600
#define FOM_T66_DIM 20
#define FOM_T67_DIM 30
#define FOM_T68_DIM 600
#define FOM_T69_DIM 20
#define FOM_T70_DIM 30
#define FOM_T71_DIM 600
#define FOM_T72_DIM 20
#define FOM_T73_DIM 30
#define FOM_T74_DIM 600
#define FOM_T75_DIM 20
#define FOM_T76_DIM 30
#define FOM_T77_DIM 600
#define FOM_T78_DIM 20
#define FOM_T79_DIM 30
#define FOM_T80_DIM 600
#define FOM_T81_DIM 20
#define FOM_T82_DIM 30
#define FOM_T83_DIM 600
#define FOM_T84_DIM 20
#define FOM_T85_DIM 30
#define FOM_T86_DIM 600
#define FOM_T87_DIM 20
#define FOM_T88_DIM 30
#define FOM_T89_DIM 600
#define FOM_T90_DIM 20
#define FOM_T91_DIM 30
#define FOM_T92_DIM 600
#define FOM_T93_DIM 20
#define FOM_T94_DIM 30
#define FOM_T95_DIM 600
#define FOM_T96_DIM 20
#define FOM_T97_DIM 30
#define FOM_T98_DIM 600
#define FOM_T99_DIM 20
#define FOM_T100_DIM 30
#define FOM_T101_DIM 600
#define FOM_T102_DIM 20
#define FOM_T103_DIM 30
#define FOM_T104_DIM 600
#define FOM_T105_DIM 20
#define FOM_T106_DIM 30
#define FOM_T107_DIM 600
#define FOM_T108_DIM 20
#define FOM_T109_DIM 30
#define FOM_T110_DIM 600
#define FOM_T111_DIM 20
#define FOM_T112_DIM 30
#define FOM_T113_DIM 600
#define FOM_T114_DIM 20
#define FOM_T115_DIM 30
#define FOM_T116_DIM 600
#define FOM_T117_DIM 20
#define FOM_T118_DIM 30
#define FOM_T119_DIM 600
#define FOM_T120_DIM 20
#define FOM_T121_DIM 30
#define FOM_T122_DIM 600
#define FOM_T123_DIM 20
#define FOM_T124_DIM 30
#define FOM_T125_DIM 600
#define FOM_T126_DIM 20
#define FOM_T127_DIM 30
#define FOM_T128_DIM 600
#define FOM_T129_DIM 20
#define FOM_T130_DIM 30
#define FOM_T131_DIM 600
#define FOM_T132_DIM 20
#define FOM_T133_DIM 30
#define FOM_T134_DIM 600
#define FOM_T135_DIM 20
#define FOM_T136_DIM 30
#define FOM_T137_DIM 600
#define FOM_T138_DIM 20
#define FOM_T139_DIM 30
#define FOM_T140_DIM 600
#define FOM_T141_DIM 20
#define FOM_T142_DIM 30
#define FOM_T143_DIM 600
#define FOM_T144_DIM 20
#define FOM_T145_DIM 30
#define FOM_T146_DIM 600
#define FOM_T147_DIM 20
#define FOM_T148_DIM 30

typedef struct {
    fom_int max_iter;
    fom_float tol;
    fom_int check_termination;
} FomSettings;

typedef struct {
    fom_float p0[FOM_PARAM_P0_DIM];
} FomParams;

typedef struct {
    fom_float x[FOM_STATE_DIM];
    fom_int iter;
    fom_float residual;
    fom_int status;
} FomSolution;

typedef struct {
    FomSettings settings;
    FomParams params;
    FomSolution solution;
    fom_float state[FOM_STATE_DIM];
    fom_float new_state[FOM_STATE_DIM];
    fom_float acc[FOM_MAX_VEC_DIM];
    fom_float t0[FOM_T0_DIM];
    fom_float t1[FOM_T1_DIM];
    fom_float t2[FOM_T2_DIM];
    fom_float t3[FOM_T3_DIM];
    fom_float t4[FOM_T4_DIM];
    fom_float t5[FOM_T5_DIM];
    fom_float t6[FOM_T6_DIM];
    fom_float t7[FOM_T7_DIM];
    fom_float t8[FOM_T8_DIM];
    fom_float t9[FOM_T9_DIM];
    fom_float t10[FOM_T10_DIM];
    fom_float t11[FOM_T11_DIM];
    fom_float t12[FOM_T12_DIM];
    fom_float t13[FOM_T13_DIM];
    fom_float t14[FOM_T14_DIM];
    fom_float t15[FOM_T15_DIM];
    fom_float t16[FOM_T16_DIM];
    fom_float t17[FOM_T17_DIM];
    fom_float t18[FOM_T18_DIM];
    fom_float t19[FOM_T19_DIM];
    fom_float t20[FOM_T20_DIM];
    fom_float t21[FOM_T21_DIM];
    fom_float t22[FOM_T22_DIM];
    fom_float t23[FOM_T23_DIM];
    fom_float t24[FOM_T24_DIM];
    fom_float t25[FOM_T25_DIM];
    fom_float t26[FOM_T26_DIM];
    fom_float t27[FOM_T27_DIM];
    fom_float t28[FOM_T28_DIM];
    fom_float t29[FOM_T29_DIM];
    fom_float t30[FOM_T30_DIM];
    fom_float t31[FOM_T31_DIM];
    fom_float t32[FOM_T32_DIM];
    fom_float t33[FOM_T33_DIM];
    fom_float t34[FOM_T34_DIM];
    fom_float t35[FOM_T35_DIM];
    fom_float t36[FOM_T36_DIM];
    fom_float t37[FOM_T37_DIM];
    fom_float t38[FOM_T38_DIM];
    fom_float t39[FOM_T39_DIM];
    fom_float t40[FOM_T40_DIM];
    fom_float t41[FOM_T41_DIM];
    fom_float t42[FOM_T42_DIM];
    fom_float t43[FOM_T43_DIM];
    fom_float t44[FOM_T44_DIM];
    fom_float t45[FOM_T45_DIM];
    fom_float t46[FOM_T46_DIM];
    fom_float t47[FOM_T47_DIM];
    fom_float t48[FOM_T48_DIM];
    fom_float t49[FOM_T49_DIM];
    fom_float t50[FOM_T50_DIM];
    fom_float t51[FOM_T51_DIM];
    fom_float t52[FOM_T52_DIM];
    fom_float t53[FOM_T53_DIM];
    fom_float t54[FOM_T54_DIM];
    fom_float t55[FOM_T55_DIM];
    fom_float t56[FOM_T56_DIM];
    fom_float t57[FOM_T57_DIM];
    fom_float t58[FOM_T58_DIM];
    fom_float t59[FOM_T59_DIM];
    fom_float t60[FOM_T60_DIM];
    fom_float t61[FOM_T61_DIM];
    fom_float t62[FOM_T62_DIM];
    fom_float t63[FOM_T63_DIM];
    fom_float t64[FOM_T64_DIM];
    fom_float t65[FOM_T65_DIM];
    fom_float t66[FOM_T66_DIM];
    fom_float t67[FOM_T67_DIM];
    fom_float t68[FOM_T68_DIM];
    fom_float t69[FOM_T69_DIM];
    fom_float t70[FOM_T70_DIM];
    fom_float t71[FOM_T71_DIM];
    fom_float t72[FOM_T72_DIM];
    fom_float t73[FOM_T73_DIM];
    fom_float t74[FOM_T74_DIM];
    fom_float t75[FOM_T75_DIM];
    fom_float t76[FOM_T76_DIM];
    fom_float t77[FOM_T77_DIM];
    fom_float t78[FOM_T78_DIM];
    fom_float t79[FOM_T79_DIM];
    fom_float t80[FOM_T80_DIM];
    fom_float t81[FOM_T81_DIM];
    fom_float t82[FOM_T82_DIM];
    fom_float t83[FOM_T83_DIM];
    fom_float t84[FOM_T84_DIM];
    fom_float t85[FOM_T85_DIM];
    fom_float t86[FOM_T86_DIM];
    fom_float t87[FOM_T87_DIM];
    fom_float t88[FOM_T88_DIM];
    fom_float t89[FOM_T89_DIM];
    fom_float t90[FOM_T90_DIM];
    fom_float t91[FOM_T91_DIM];
    fom_float t92[FOM_T92_DIM];
    fom_float t93[FOM_T93_DIM];
    fom_float t94[FOM_T94_DIM];
    fom_float t95[FOM_T95_DIM];
    fom_float t96[FOM_T96_DIM];
    fom_float t97[FOM_T97_DIM];
    fom_float t98[FOM_T98_DIM];
    fom_float t99[FOM_T99_DIM];
    fom_float t100[FOM_T100_DIM];
    fom_float t101[FOM_T101_DIM];
    fom_float t102[FOM_T102_DIM];
    fom_float t103[FOM_T103_DIM];
    fom_float t104[FOM_T104_DIM];
    fom_float t105[FOM_T105_DIM];
    fom_float t106[FOM_T106_DIM];
    fom_float t107[FOM_T107_DIM];
    fom_float t108[FOM_T108_DIM];
    fom_float t109[FOM_T109_DIM];
    fom_float t110[FOM_T110_DIM];
    fom_float t111[FOM_T111_DIM];
    fom_float t112[FOM_T112_DIM];
    fom_float t113[FOM_T113_DIM];
    fom_float t114[FOM_T114_DIM];
    fom_float t115[FOM_T115_DIM];
    fom_float t116[FOM_T116_DIM];
    fom_float t117[FOM_T117_DIM];
    fom_float t118[FOM_T118_DIM];
    fom_float t119[FOM_T119_DIM];
    fom_float t120[FOM_T120_DIM];
    fom_float t121[FOM_T121_DIM];
    fom_float t122[FOM_T122_DIM];
    fom_float t123[FOM_T123_DIM];
    fom_float t124[FOM_T124_DIM];
    fom_float t125[FOM_T125_DIM];
    fom_float t126[FOM_T126_DIM];
    fom_float t127[FOM_T127_DIM];
    fom_float t128[FOM_T128_DIM];
    fom_float t129[FOM_T129_DIM];
    fom_float t130[FOM_T130_DIM];
    fom_float t131[FOM_T131_DIM];
    fom_float t132[FOM_T132_DIM];
    fom_float t133[FOM_T133_DIM];
    fom_float t134[FOM_T134_DIM];
    fom_float t135[FOM_T135_DIM];
    fom_float t136[FOM_T136_DIM];
    fom_float t137[FOM_T137_DIM];
    fom_float t138[FOM_T138_DIM];
    fom_float t139[FOM_T139_DIM];
    fom_float t140[FOM_T140_DIM];
    fom_float t141[FOM_T141_DIM];
    fom_float t142[FOM_T142_DIM];
    fom_float t143[FOM_T143_DIM];
    fom_float t144[FOM_T144_DIM];
    fom_float t145[FOM_T145_DIM];
    fom_float t146[FOM_T146_DIM];
    fom_float t147[FOM_T147_DIM];
    fom_float t148[FOM_T148_DIM];
} FomWorkspace;

#endif /* FOM_WORKSPACE_H */
