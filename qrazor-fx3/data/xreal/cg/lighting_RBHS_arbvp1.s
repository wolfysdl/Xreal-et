!!ARBvp1.0
# ARB_vertex_program generated by NVIDIA Cg compiler
# cgc version 1.2.1001, build date Mar 17 2004  10:58:07
# command line args: -profile arbvp1
# nv30vp backend compiling 'main' program
#vendor NVIDIA Corporation
#version 1.0.02
#profile arbvp1
#program main
#var float4 IN.position : $vin.ATTR0 : ATTR0 : 0 : 1
#var float4 IN.tex0 : $vin.ATTR8 : ATTR8 : 0 : 1
#var float4 IN.tex1 : $vin.ATTR9 : ATTR9 : 0 : 1
#var float3 IN.tangent : $vin.ATTR14 : ATTR14 : 0 : 1
#var float3 IN.binormal : $vin.ATTR15 : ATTR15 : 0 : 1
#var float3 IN.normal : $vin.ATTR2 : ATTR2 : 0 : 1
#var float4 main.hposition : $vout.POSITION : HPOS : -1 : 1
#var float4 main.position : $vout.TEXCOORD0 : TEX0 : -1 : 1
#var float4 main.tex_diffuse_bump : $vout.TEXCOORD1 : TEX1 : -1 : 1
#var float4 main.tex_height : $vout.TEXCOORD2 : TEX2 : -1 : 1
#var float4 main.tex_specular : $vout.TEXCOORD3 : TEX3 : -1 : 1
#var float4 main.tex_light_deluxe : $vout.TEXCOORD4 : TEX4 : -1 : 1
#var float3 main.tangent : $vout.TEXCOORD5 : TEX5 : -1 : 1
#var float3 main.binormal : $vout.TEXCOORD6 : TEX6 : -1 : 1
#var float3 main.normal : $vout.TEXCOORD7 : TEX7 : -1 : 1
TEMP R0;
ATTRIB v2 = vertex.attrib[2];
ATTRIB v15 = vertex.attrib[15];
ATTRIB v14 = vertex.attrib[14];
ATTRIB v9 = vertex.attrib[9];
ATTRIB v8 = vertex.attrib[8];
ATTRIB v0 = vertex.attrib[0];
PARAM s283[4] = { state.matrix.texture[5] };
PARAM s279[4] = { state.matrix.texture[4] };
PARAM s275[4] = { state.matrix.texture[3] };
PARAM s271[4] = { state.matrix.texture[2] };
PARAM s267[4] = { state.matrix.texture[1] };
PARAM s263[4] = { state.matrix.texture[0] };
PARAM s259[4] = { state.matrix.mvp };
	MOV result.texcoord[0], v0;
	MOV result.texcoord[5].xyz, v14;
	MOV result.texcoord[6].xyz, v15;
	MOV result.texcoord[7].xyz, v2;
	DP4 result.position.x, s259[0], v0;
	DP4 result.position.y, s259[1], v0;
	DP4 result.position.z, s259[2], v0;
	DP4 result.position.w, s259[3], v0;
	DP4 R0.x, s263[0], v8;
	DP4 R0.y, s263[1], v8;
	MOV result.texcoord[1].xy, R0.xyxx;
	DP4 R0.x, s267[0], v8;
	DP4 R0.y, s267[1], v8;
	MOV result.texcoord[1].zw, R0.xyxy;
	DP4 result.texcoord[2].x, s271[0], v8;
	DP4 result.texcoord[2].y, s271[1], v8;
	DP4 result.texcoord[2].z, s271[2], v8;
	DP4 result.texcoord[2].w, s271[3], v8;
	DP4 result.texcoord[3].x, s275[0], v8;
	DP4 result.texcoord[3].y, s275[1], v8;
	DP4 result.texcoord[3].z, s275[2], v8;
	DP4 result.texcoord[3].w, s275[3], v8;
	DP4 R0.x, s279[0], v9;
	DP4 R0.y, s279[1], v9;
	MOV result.texcoord[4].xy, R0.xyxx;
	DP4 R0.x, s283[0], v9;
	DP4 R0.y, s283[1], v9;
	MOV result.texcoord[4].zw, R0.xyxy;
END
# 36 instructions
# 1 temp registers
# End of program
