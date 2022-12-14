!!VP2.0
# NV_vertex_program generated by NVIDIA Cg compiler
# cgc version 1.2.1001, build date Mar 17 2004  10:58:07
# command line args: -profile vp30
# nv30vp backend compiling 'main' program
#vendor NVIDIA Corporation
#version 1.0.02
#profile vp30
#program main
#semantic main.matrix_model_view_projection
#semantic main.matrix_model
#var float4x4 matrix_model_view_projection :  : c[0], 4 : 1 : 1
#var float4x4 matrix_model :  : c[4], 4 : 2 : 1
#var float4 IN.position : $vin.POSITION : ATTR0 : 0 : 1
#var float3 IN.normal : $vin.NORMAL : ATTR2 : 0 : 1
#var float4 main.hposition : $vout.POSITION : HPOS : -1 : 1
#var float4 main.position : $vout.TEXCOORD0 : TEX0 : -1 : 1
#var float3 main.normal : $vout.TEXCOORD1 : TEX1 : -1 : 1
#const c[8] = 0 1 2 0
b0:
	DP4 o[HPOS].x, c[0], v[0];
	DP4 o[HPOS].y, c[1], v[0];
	DP4 o[HPOS].z, c[2], v[0];
	DP4 o[HPOS].w, c[3], v[0];
	DP4 o[TEX0].x, c[4], v[0];
	DP4 o[TEX0].y, c[5], v[0];
	DP4 o[TEX0].z, c[6], v[0];
	DP4 o[TEX0].w, c[7], v[0];
	DP3 o[TEX1].x, c[4].xyzx, v[2].xyzx;
	DP3 o[TEX1].y, c[5].xyzx, v[2].xyzx;
	DP3 o[TEX1].z, c[6].xyzx, v[2].xyzx;
END
# 11 instructions
# 0 temp registers
# End of program
