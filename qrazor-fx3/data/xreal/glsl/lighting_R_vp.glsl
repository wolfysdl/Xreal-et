/// ============================================================================
/*
Copyright (C) 2004 Robert Beckebans <trebor_7@users.sourceforge.net>
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
attribute vec4		attr_TexCoord1;

varying vec3		var_normal;
varying vec2		var_tex_diffuse;
varying vec2		var_tex_light;
varying vec2		var_tex_deluxe;

void	main()
{
	// transform vertex position into homogenous clip-space
	gl_Position = ftransform();
	
	// assign normal in object space
	var_normal = gl_Normal;
	
	// transform texcoords into diffusemap texture space
	var_tex_diffuse = (gl_TextureMatrix[0] * attr_TexCoord0).st;
	
	// transform texcoords_lm into lightmap texture space
	var_tex_light = (gl_TextureMatrix[1] * attr_TexCoord1).st;
	
	// transform texcoords_lm into deluxemap texture space
	var_tex_deluxe = (gl_TextureMatrix[2] * attr_TexCoord1).st;
}
