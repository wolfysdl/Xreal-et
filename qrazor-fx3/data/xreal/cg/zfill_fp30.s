!!FP1.0
# NV_fragment_program generated by NVIDIA Cg compiler
# cgc version 1.2.1001, build date Mar 17 2004  10:58:07
# command line args: -profile fp30
#vendor NVIDIA Corporation
#version 1.0.02
#profile fp30
#program main
#semantic main.decal
#var sampler2D decal :  : texunit 0 : 1 : 1
#var float4 IN.texture0 : $vin.TEXCOORD0 : TEX0 : 0 : 1
#var half4 main.color : $vout.COLOR : COL : -1 : 1
TEX R0.w, f[TEX0].xyxx, TEX0, 2D;
MOVH o[COLH].w, R0.w;
MOVH o[COLH].xyz, {0, 0, 0}.xyzx;
END
# 3 instructions, 1 R-regs, 0 H-regs.
# End of program
