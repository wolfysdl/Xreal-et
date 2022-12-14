!!FP1.0
# NV_fragment_program generated by NVIDIA Cg compiler
# cgc version 1.2.1001, build date Mar 17 2004  10:58:07
# command line args: -profile fp30
#vendor NVIDIA Corporation
#version 1.0.02
#profile fp30
#program main
#semantic main.diffusemap
#semantic main.bumpmap
#semantic main.specularmap
#semantic main.lightmap
#semantic main.deluxemap
#semantic main.view_origin
#semantic main.bump_scale
#semantic main.specular_exponent
#var sampler2D diffusemap :  : texunit 0 : 1 : 1
#var sampler2D bumpmap :  : texunit 1 : 2 : 1
#var sampler2D specularmap :  : texunit 2 : 3 : 1
#var sampler2D lightmap :  : texunit 3 : 4 : 1
#var sampler2D deluxemap :  : texunit 4 : 5 : 1
#var float3 view_origin :  :  : 6 : 1
#var float bump_scale :  :  : 7 : 1
#var float specular_exponent :  :  : 8 : 1
#var float4 IN.position : $vin.TEXCOORD0 : TEX0 : 0 : 1
#var float4 IN.tex_diffuse_bump : $vin.TEXCOORD1 : TEX1 : 0 : 1
#var float4 IN.tex_specular : $vin.TEXCOORD2 : TEX2 : 0 : 1
#var float4 IN.tex_light : $vin.TEXCOORD3 : TEX3 : 0 : 1
#var float4 IN.tex_deluxe : $vin.TEXCOORD4 : TEX4 : 0 : 1
#var float3 IN.tangent : $vin.TEXCOORD5 : TEX5 : 0 : 1
#var float3 IN.binormal : $vin.TEXCOORD6 : TEX6 : 0 : 1
#var float3 IN.normal : $vin.TEXCOORD7 : TEX7 : 0 : 1
#var half4 main.color : $vout.COLOR : COL : -1 : 1
DECLARE view_origin;
DECLARE bump_scale;
DECLARE specular_exponent;
TEX R0.xyz, f[TEX4].xyxx, TEX4, 2D;
TEX R1.xyz, f[TEX1].zwzz, TEX1, 2D;
ADDR R0.xyz, R0.xyzx, -{0.5}.x;
MULR R0.xyz, R0.xyzx, {2}.x;
ADDR R1.xyz, R1.xyzx, -{0.5}.x;
MULR R1.xyz, R1.xyzx, {2}.x;
MOVR H0.xy, R1.xyxx;
MULR R0.w, R1.z, bump_scale.x;
MOVH H0.z, R0.w;
DP3H H0.w, H0.xyzx, H0.xyzx;
RSQH H0.w, H0.w;
MULH H0.xyz, H0.w, H0.xyzx;
ADDR R1.xyz, view_origin.xyzx, -f[TEX0].xyzx;
DP3R R0.w, f[TEX5].xyzx, R1.xyzx;
MOVR R2.x, R0.w;
DP3R R0.w, f[TEX5].xyzx, R0.xyzx;
MOVR R3.x, R0.w;
DP3R R0.w, f[TEX6].xyzx, R1.xyzx;
MOVR R2.y, R0.w;
DP3R R0.w, f[TEX7].xyzx, R1.xyzx;
MOVR R2.z, R0.w;
DP3R R0.w, f[TEX6].xyzx, R0.xyzx;
MOVR R3.y, R0.w;
DP3R R0.x, f[TEX7].xyzx, R0.xyzx;
MOVR R3.z, R0.x;
DP3R R0.x, R3.xyzx, R3.xyzx;
RSQR R0.x, R0.x;
MULR R3.xyz, R0.x, R3.xyzx;
DP3X_SAT H0.w, H0.xyzx, R3.xyzx;
DP3R R0.x, R2.xyzx, R2.xyzx;
RSQR R0.x, R0.x;
MULR R2.xyz, R0.x, R2.xyzx;
ADDX H1.xyz, R3.xyzx, R2.xyzx;
DP3H H1.w, H1.xyzx, H1.xyzx;
RSQH H1.w, H1.w;
MULH H1.xyz, H1.w, H1.xyzx;
DP3X_SAT H0.x, H0.xyzx, H1.xyzx;
POWR R0.x, H0.x, specular_exponent.x;
TEX R1.xyz, f[TEX3].xyxx, TEX3, 2D;
TEX R2.xyz, f[TEX2].xyxx, TEX2, 2D;
MULX H0.xyz, R1.xyzx, H0.w;
MULR R1.xyz, R2.xyzx, R1.xyzx;
MULR R0.xyz, R1.xyzx, R0.x;
TEX R1, f[TEX1].xyxx, TEX0, 2D;
MULX H0.xyz, R1.xyzx, H0.xyzx;
MOVR H1.w, R1.w;
MOVX H1.xyz, H0.xyzx;
ADDH H0.xyz, H1.xyzx, R0.xyzx;
MOVH H1.xyz, H0.xyzx;
MOVH o[COLH], H1;
END
# 50 instructions, 4 R-regs, 2 H-regs.
# End of program
