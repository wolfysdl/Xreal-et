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

uniform samplerCube	u_colormap;
uniform vec3		u_view_origin;
uniform vec3		u_eta_ratio;
uniform float		u_fresnel_power;
uniform float		u_fresnel_scale;
uniform float		u_fresnel_bias;

varying vec3		var_vertex;
varying vec3		var_normal;

void	main()
{	
	/// compute incident ray
	vec3 I = normalize(var_vertex - u_view_origin);
	
	// compute normal
	vec3 N = normalize(var_normal);
	
	// compute reflection ray
	vec3 R = reflect(I, N);
	
	// compute fresnel term
	float fresnel = u_fresnel_bias + pow(1.0 - dot(I, N), u_fresnel_power) * u_fresnel_scale;
	vec3 fresnel_term = vec3(fresnel, fresnel, fresnel);
	
	// compute reflection color
	vec3 reflect_color = textureCube(u_colormap, R).rgb;
	
	// compute refraction color using a refraction ray for each channel
	vec3 refract_color;
	
	refract_color.r = textureCube(u_colormap, refract(I, N, u_eta_ratio.x)).r;
	refract_color.g = textureCube(u_colormap, refract(I, N, u_eta_ratio.y)).g;
	refract_color.b = textureCube(u_colormap, refract(I, N, u_eta_ratio.z)).b;
	
	// compute final color
	gl_FragColor.r = (1.0 - fresnel) * refract_color.r + reflect_color.r * fresnel;
	gl_FragColor.g = (1.0 - fresnel) * refract_color.g + reflect_color.g * fresnel;
	gl_FragColor.b = (1.0 - fresnel) * refract_color.b + reflect_color.b * fresnel;
	gl_FragColor.a = 1.0;
}
