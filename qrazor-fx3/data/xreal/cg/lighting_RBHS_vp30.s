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
#semantic main.matrix_texture0
#semantic main.matrix_texture1
#semantic main.matrix_texture2
#semantic main.matrix_texture3
#semantic main.matrix_texture4
#semantic main.matrix_texture5
#var float4x4 matrix_model_view_projection :  : c[0], 4 : 1 : 1
#var float4x4 matrix_texture0 :  : c[4], 4 : 2 : 1
#var float4x4 matrix_texture1 :  : c[8], 4 : 3 : 1
#var float4x4 matrix_texture2 :  : c[12], 4 : 4 : 1
#var float4x4 matrix_texture3 :  : c[16], 4 : 5 : 1
#var float4x4 matrix_texture4 :  : c[20], 4 : 6 : 1
#var float4x4 matrix_texture5 :  : c[24], 4 : 7 : 1
#var float4 IN.position : $vin.POSITION : ATTR0 : 0 : 1
#var float4 IN.tex0 : $vin.TEXCOORD0 : ATTR8 : 0 : 1
#var float4 IN.tex1 : $vin.TEXCOORD1 : ATTR9 : 0 : 1
#var float3 IN.tangent : $vin.TANGENT : ATTR14 : 0 : 1
#var float3 IN.binormal : $vin.BINORMAL : ATTR15 : 0 : 1
#var float3 IN.normal : $vin.NORMAL : ATTR2 : 0 : 1
#var float4 main.hposition : $vout.POSITION : HPOS : -1 : 1
#var float4 main.position : $vout.TEXCOORD0 : TEX0 : -1 : 1
#var float4 main.tex_diffuse_bump : $vout.TEXCOORD1 : TEX1 : -1 : 1
#var float4 main.tex_height : $vout.TEXCOORD2 : TEX2 : -1 : 1
#var float4 main.tex_specular : $vout.TEXCOORD3 : TEX3 : -1 : 1
#var float4 main.tex_light_deluxe : $vout.TEXCOORD4 : TEX4 : -1 : 1
#var float3 main.tangent : $vout.TEXCOORD5 : TEX5 : -1 : 1
#var float3 main.binormal : $vout.TEXCOORD6 : TEX6 : -1 : 1
#var float3 main.normal : $vout.TEXCOORD7 : TEX7 : -1 : 1
b0:
	MOV o[TEX0], v[0];
	MOV o[TEX5].xyz, v[14];
	MOV o[TEX6].xyz, v[15];
	MOV o[TEX7].xyz, v[2];
	DP4 o[HPOS].x, c[0], v[0];
	DP4 o[HPOS].y, c[1], v[0];
	DP4 o[HPOS].z, c[2], v[0];
	DP4 o[HPOS].w, c[3], v[0];
	DP4 R0.x, c[4], v[8];
	DP4 R0.y, c[5], v[8];
	MOV o[TEX1].xy, R0.xyxx;
	DP4 R0.x, c[8], v[8];
	DP4 R0.y, c[9], v[8];
	MOV o[TEX1].zw, R0.xyxy;
	DP4 o[TEX2].x, c[12], v[8];
	DP4 o[TEX2].y, c[13], v[8];
	DP4 o[TEX2].z, c[14], v[8];
	DP4 o[TEX2].w, c[15], v[8];
	DP4 o[TEX3].x, c[16], v[8];
	DP4 o[TEX3].y, c[17], v[8];
	DP4 o[TEX3].z, c[18], v[8];
	DP4 o[TEX3].w, c[19], v[8];
	DP4 R0.x, c[20], v[9];
	DP4 R0.y, c[21], v[9];
	MOV o[TEX4].xy, R0.xyxx;
	DP4 R0.x, c[24], v[9];
	DP4 R0.y, c[25], v[9];
	MOV o[TEX4].zw, R0.xyxy;
END
# 36 instructions
# 1 temp registers
# End of program
