/// ============================================================================
/*
Copyright (C) 2005 Robert Beckebans <trebor_7@users.sourceforge.net>
Please see the file "AUTHORS" for a list of contributors

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
/// ============================================================================

attribute vec4		attr_TexCoord0;
attribute vec3		attr_Tangent;
attribute vec3		attr_Binormal;
attribute vec3		attr_Light;

varying vec3		var_vertex;
varying vec2		var_tex_diffuse;
varying vec2		var_tex_bump;
varying mat3		var_mat_os2ts;
varying vec3		var_light;
varying vec3		var_color;

void	main()
{	
	// transform vertex position into homogenous clip-space
	gl_Position = ftransform();
	
	// assign position in object space
	var_vertex = gl_Vertex.xyz;
		
	// transform texcoords into diffusemap texture space
	var_tex_diffuse.st = (gl_TextureMatrix[0] * attr_TexCoord0).st;
	
	// transform texcoords into bumpmap texture space
	var_tex_bump.st = (gl_TextureMatrix[1] * attr_TexCoord0).st;
	
	// construct object-space-to-tangent-space 3x3 matrix
	var_mat_os2ts = mat3(	attr_Tangent.x, attr_Binormal.x, gl_Normal.x,
				attr_Tangent.y, attr_Binormal.y, gl_Normal.y,
				attr_Tangent.z, attr_Binormal.z, gl_Normal.z	);
				
				
	// assign vertex to light vector in object space
	var_light = attr_Light;
	
	// assign color
	var_color = gl_Color.rgb;
}
