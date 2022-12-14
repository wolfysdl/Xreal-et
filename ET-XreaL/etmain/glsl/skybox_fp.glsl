/*
===========================================================================
Copyright (C) 2006-2009 Robert Beckebans <trebor_7@users.sourceforge.net>

This file is part of XreaL source code.

XreaL source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

XreaL source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with XreaL source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

/* skybox_fp.glsl */

uniform samplerCube	u_ColorMap;
uniform vec3		u_ViewOrigin;

varying vec3		var_Position;

void	main()
{
	// compute incident ray
	vec3 I = normalize(var_Position - u_ViewOrigin);
	
	gl_FragColor = textureCube(u_ColorMap, I).rgba;
}
