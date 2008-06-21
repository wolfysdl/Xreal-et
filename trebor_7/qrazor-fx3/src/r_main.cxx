/// ============================================================================
/*
Copyright (C) 1997-2001 Id Software, Inc.
Copyright (C) 2003 Robert Beckebans <trebor_7@users.sourceforge.net>
Copyright (C) 2003, 2004  contributors of the XreaL project
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


/// includes ===================================================================
// system -------------------------------------------------------------------

// qrazor-fx ----------------------------------------------------------------
#include "r_local.h"


viddef_t	vid;

ref_import_t	ri;




float		r_depthmin, r_depthmax;

glconfig_t	gl_config;
glstate_t 	gl_state;

r_image_c*	r_img_default;
r_image_c*	r_img_white;
r_image_c*	r_img_black;
r_image_c*	r_img_flat;
r_image_c*	r_img_quadratic;
r_image_c*	r_img_cubemap_white;
r_image_c*	r_img_cubemap_normal;
r_image_c*	r_img_cubemap_sky;
r_image_c*	r_img_nofalloff;
r_image_c*	r_img_attenuation_3d;
r_image_c*	r_img_lightview_depth;
r_image_c*	r_img_lightview_color;

r_scene_t*	r_current_scene;


r_frustum_t	r_frustum;

uint_t		r_framecount;		// used for dlight push checking


int		r_depth_format;

uint_t		r_leafs_counter;
uint_t		r_cmds_counter;
uint_t		r_cmds_radiosity_counter;
uint_t		r_cmds_light_counter;

//
// view origin
//
vec3_c		r_up;
vec3_c		r_forward;
vec3_c		r_right;

vec3_c		r_origin;

bool		r_portal_view;	// if true, get vis data at
vec3_c		r_portal_org;	// portalorg instead of vieworg

bool		r_mirrorview;	// if true, lock pvs

cplane_c	r_clipplane;



r_entity_c	r_world_entity;
r_tree_c*	r_world_tree;

std::vector<index_t> 	r_quad_indexes;

bool		r_zfill;

//
// screen size info
//
r_refdef_t	r_newrefdef;


std::map<int, r_entity_c>	r_entities;
std::map<int, r_light_c>	r_lights;

int		r_particles_num;
r_particle_t	r_particles[MAX_PARTICLES];

int		r_polys_num;
r_poly_t	r_polys[MAX_POLYS];

r_scene_t	r_world_scene;


static uint_t	r_video_export_count = 0;


//
// renderer cvars
//
cvar_t	*r_lefthand;
cvar_t	*r_draw2d;
cvar_t	*r_drawentities;
cvar_t	*r_drawworld;
cvar_t	*r_drawparticles;
cvar_t	*r_drawpolygons;
cvar_t	*r_drawsky;
cvar_t	*r_drawextra;
cvar_t	*r_drawtranslucent;
cvar_t	*r_speeds;
cvar_t	*r_fullbright;
cvar_t	*r_lerpmodels;
cvar_t	*r_log;
cvar_t	*r_shadows;
cvar_t	*r_shadows_alpha;
cvar_t	*r_shadows_nudge;
cvar_t	*r_lighting;
cvar_t	*r_lighting_omni;
cvar_t	*r_lighting_proj;
cvar_t	*r_lightmap;
cvar_t	*r_lightscale;
cvar_t	*r_znear;
cvar_t	*r_zfar;
cvar_t	*r_nobind;
cvar_t	*r_picmip;
cvar_t	*r_skymip;
cvar_t	*r_showtris;
cvar_t	*r_showbbox;
cvar_t	*r_showareas;
cvar_t	*r_showareaportals;
cvar_t	*r_shownormals;
cvar_t	*r_showtangents;
cvar_t	*r_showbinormals;
cvar_t	*r_showinvisible;
cvar_t	*r_clear;
cvar_t	*r_cull;
cvar_t	*r_cullplanes;
cvar_t	*r_cullportals;
cvar_t	*r_polyblend;
cvar_t	*r_flashblend;
cvar_t	*r_playermip;
cvar_t	*r_drawbuffer;
cvar_t	*r_swapinterval;
cvar_t	*r_texturemode;
cvar_t	*r_polygonmode;
cvar_t  *r_lockpvs;
cvar_t	*r_vis;
cvar_t  *r_maxtexsize;
cvar_t	*r_finish;
cvar_t	*r_fastsky;
cvar_t	*r_subdivisions;
cvar_t	*r_flares;
cvar_t	*r_flaresize;
cvar_t	*r_flarefade;
cvar_t	*r_octree;
cvar_t	*r_bump_mapping;
cvar_t	*r_gloss;
cvar_t	*r_parallax;
cvar_t	*r_cmds_max;
cvar_t	*r_cmds_light_max;
cvar_t	*r_cmds_translucent_max;
cvar_t	*r_video_export;
cvar_t	*r_evalexpressions;

cvar_t	*r_arb_multitexture;
cvar_t	*r_arb_texture_compression;
cvar_t	*r_arb_vertex_buffer_object;

cvar_t	*r_ext_compiled_vertex_array;
cvar_t	*r_ext_texture_filter_anisotropic;
cvar_t	*r_ext_texture_filter_anisotropic_level;

cvar_t	*r_sgix_fbconfig;
cvar_t	*r_sgix_pbuffer;

cvar_t	*vid_fullscreen;
cvar_t	*vid_gamma;
cvar_t	*vid_mode;
cvar_t  *vid_gldriver;
cvar_t	*vid_colorbits;
cvar_t	*vid_depthbits;
cvar_t	*vid_stencilbits;

cvar_t	*vid_pbuffer_texsize;
cvar_t	*vid_pbuffer_colorbits;
cvar_t	*vid_pbuffer_depthbits;
cvar_t	*vid_pbuffer_stencilbits;

cvar_t	*vid_ref;







void 	R_DrawNULL(const vec3_c &origin, const vec3_c &angles)
{
#if 0
	vec3_c	vf, vr, vu, org;
	
	// calc axis
	Angles_ToVectors(angles, vf, vr, vu);
	
	vf *= 15;	vf += origin;
	vr *= 15;	vr += origin;
	vu *= 15;	vu += origin;
	
	org = origin;
	
	// draw axis
	//glDisable(GL_BLEND);
	//glDisable(GL_CULL_FACE);
	//glDepthMask(GL_FALSE);
	
	glBegin(GL_LINES);

	glColor4fv(color_red);
	glVertex3fv(org);
	glVertex3fv(vf);
	
	glColor4fv(color_green);
	glVertex3fv(org);
	glVertex3fv(vr);
	
	glColor4fv(color_blue);
	glVertex3fv(org);
	glVertex3fv(vu);
	
	glEnd();
	
	//glDepthMask(GL_TRUE);
	//glEnable(GL_BLEND);
	//glEnable(GL_CULL_FACE);

	//TODO draw using backend
#endif
}


void 	R_DrawBBox(const cbbox_c &bbox)
{
#if 1
	if(!r_showbbox->getValue())
		return;

	//
	// compute bbox edges
	//
	vec3_c vertexes[8];	// max x,y,z points in space
	
	vertexes[0].set(bbox._maxs[0], bbox._mins[1], bbox._mins[2]);
	vertexes[1].set(bbox._maxs[0], bbox._mins[1], bbox._maxs[2]);
	vertexes[2].set(bbox._mins[0], bbox._mins[1], bbox._maxs[2]);
	vertexes[3].set(bbox._mins[0], bbox._mins[1], bbox._mins[2]);
	
	vertexes[4].set(bbox._maxs[0], bbox._maxs[1], bbox._mins[2]);
	vertexes[5].set(bbox._maxs[0], bbox._maxs[1], bbox._maxs[2]);
	vertexes[6].set(bbox._mins[0], bbox._maxs[1], bbox._maxs[2]);
	vertexes[7].set(bbox._mins[0], bbox._maxs[1], bbox._mins[2]);
	
		
	
	//glDisable(GL_CULL_FACE);
	xglPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	//glDisable(GL_TEXTURE_2D);
	
	
	//
	// draw axis
	//
	
	// TODO
	
	//
	// draw bounds
	//
	xglColor4f(0, 0, 1, 1.0);
	
	xglBegin(GL_QUADS);
	
	// left side	
	//xglVertex3fv(vertexes[0]);
	//xglVertex3fv(vertexes[1]);
	//xglVertex3fv(vertexes[2]);
	//xglVertex3fv(vertexes[3]);
	
	// right side
	//xglVertex3fv(vertexes[4]);
	//xglVertex3fv(vertexes[5]);
	//xglVertex3fv(vertexes[6]);
	//xglVertex3fv(vertexes[7]);
	
	
	// front side
	xglVertex3fv(vertexes[0]);
	xglVertex3fv(vertexes[1]);
	xglVertex3fv(vertexes[5]);
	xglVertex3fv(vertexes[4]);
	
	// back side
	xglVertex3fv(vertexes[2]);
	xglVertex3fv(vertexes[3]);
	xglVertex3fv(vertexes[7]);
	xglVertex3fv(vertexes[6]);
	
	// top side
	xglVertex3fv(vertexes[1]); 
	xglVertex3fv(vertexes[2]);
	xglVertex3fv(vertexes[6]); 
	xglVertex3fv(vertexes[5]);
	
	// bottom side
	xglVertex3fv(vertexes[0]); 
	xglVertex3fv(vertexes[3]);
	xglVertex3fv(vertexes[7]); 
	xglVertex3fv(vertexes[4]);

	xglEnd();
	
	
	//xglEnable(GL_TEXTURE_2D);
	xglPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	//xglEnable(GL_CULL_FACE);
#endif
}


/*
=============
R_CalcTangentSpace
Tr3B - recoded from Nvidia's SDK
=============
*/
void	R_CalcTangentSpace(	vec3_c &vtangent, vec3_c &vbinormal, vec3_c &vnormal,
				const vec3_c &v0, const vec3_c &v1, const vec3_c &v2,
				const vec2_c &st0, const vec2_c &st1, const vec2_c &st2,
				const vec3_c &n	)
{
#if 1
	vec3_c cp;
	vec3_c e0(v1[0] - v0[0], st1[0] - st0[0], st1[1] - st0[1]);
	vec3_c e1(v2[0] - v0[0], st2[0] - st0[0], st2[1] - st0[1]);

	cp.crossProduct(e0,e1);
	
	vec3_c tangent;
	vec3_c binormal;
	vec3_c normal;
	
		
	if(fabs(cp[0]) > X_eps)
	{
		tangent[0]	= -cp[1] / cp[0];
		binormal[0]	= -cp[2] / cp[0];
	}

	e0[0] = v1[1] - v0[1];
	e1[0] = v2[1] - v0[1];

	cp.crossProduct(e0,e1);
	if(fabs(cp[0]) > X_eps)
	{
        	tangent[1]	= -cp[1] / cp[0];        
        	binormal[1]	= -cp[2] / cp[0];
	}

	e0[0] = v1[2] - v0[2];
	e1[0] = v2[2] - v0[2];

	cp.crossProduct(e0,e1);
	if(fabs(cp[0]) > X_eps)
	{
		tangent[2]	= -cp[1] / cp[0];        
		binormal[2]	= -cp[2] / cp[0];
	}


	// tangent...
	tangent.normalize();
		
	// binormal...
	binormal.normalize();
	
	// normal...
	// compute the cross product TxB
	normal.crossProduct(tangent, binormal);

	normal.normalize();

	
	// Gram-Schmidt orthogonalization process for B
	// compute the cross product B=NxT to obtain 
	// an orthogonal basis
	binormal.crossProduct(normal, tangent);
	
	if(normal.dotProduct(n) < 0)
	{
		normal.negate();
	}

	vtangent = tangent;
	vbinormal = binormal;
	vnormal = normal;
#else

	// other solution 
	// http://members.rogers.com/deseric/tangentspace.htm
	
	vec3_c planes[3];
	vec3_c e0;
	vec3_c e1;
	
	for(int i=0; i<3; i++)
	{
		e0 = vec3_c(v1[i] - v0[i], st1[0] - st0[0], st1[1] - st0[1]);
		e1 = vec3_c(v2[i] - v0[i], st2[0] - st0[0], st2[1] - st0[1]);
				
		planes[i].crossProduct(e0, e1);
	}


	vec3_c tangent;
	vec3_c binormal;
	vec3_c normal;
	
	//So your tangent space will be defined by this :
	//Normal = Normal of the triangle or Tangent X Binormal (careful with the cross product, 
	// you have to make sure the normal points in the right direction)
	//Tangent = ( dp(Fx(s,t)) / ds,  dp(Fy(s,t)) / ds, dp(Fz(s,t)) / ds )   or     ( -Bx/Ax, -By/Ay, - Bz/Az )
	//Binormal =  ( dp(Fx(s,t)) / dt,  dp(Fy(s,t)) / dt, dp(Fz(s,t)) / dt )  or     ( -Cx/Ax, -Cy/Ay, -Cz/Az )
	
	// tangent...
	tangent[0] = -planes[0][1]/planes[0][0];
	tangent[1] = -planes[1][1]/planes[1][0];
	tangent[2] = -planes[2][1]/planes[2][0];
	
	tangent.normalize();
	
	// binormal...
	binormal[0] = -planes[0][2]/planes[0][0];
	binormal[1] = -planes[1][2]/planes[1][0];
	binormal[2] = -planes[2][2]/planes[2][0];
	
	binormal.normalize();
	
	// compute the cross product TxB
	normal.crossProduct(tangent, binormal);
		
	normal.normalize();

	
	// Gram-Schmidt orthogonalization process for B
	// compute the cross product B=NxT to obtain 
	// an orthogonal basis
	//binormal.crossProduct(normal, tangent);
	
	if(normal.dotProduct(n) < 0)
	{
		normal.negate();
	}

	vtangent = tangent;
	vbinormal = binormal;
	vnormal = normal;
#endif
}


 
static void	R_ScreenShot() 
{
	int		i;
	byte		*buffer;
	std::string	checkname;
	 
	// find a file name to save it to
	for(i=0; i<=999; i++)
	{	
		checkname = "screenshots/qrazor-fxIII" + std::string(va("%03d", i)) + ".jpg";
		
		if(ri.VFS_FLoad(checkname, NULL) <= 0)
			break;	// file doesn't exist
	}
	
	if(i==1000)
	{
		ri.Com_Printf("R_ScreenShot_f: Couldn't create a file\n"); 
		return;
 	}
	
	buffer = new byte[vid.width * vid.height * 3];
	
	xglReadPixels(0, 0, vid.width, vid.height, GL_RGB, GL_UNSIGNED_BYTE, buffer);

	//save it smart and small
	IMG_WriteJPG(checkname, buffer, vid.width, vid.height, 95);
	//IMG_WriteTGA(checkname, buffer, vid.width, vid.height, 3);

	delete [] buffer;
	
	ri.Com_Printf("wrote %s\n", checkname.c_str());
}




static void	R_WriteVideoScreenShot() 
{
	std::string number = va("%04d", r_video_export_count);

	std::string filename = "video_export/frame" + number + ".tga";
	
	byte *buffer = new byte[vid.width * vid.height * 3];
	
	xglReadPixels(0, 0, vid.width, vid.height, GL_RGB, GL_UNSIGNED_BYTE, buffer);

	//save it fast
	IMG_WriteTGA(filename, buffer, vid.width, vid.height, 3);

	delete [] buffer;
	
	r_video_export_count++;
}


static void	R_ScreenShot_f()
{
	R_ScreenShot();
}



static void	R_CheckOpenGLExtensions()
{
	gl_config.arb_multitexture = false;
	gl_config.arb_texture_compression = false;
	gl_config.arb_vertex_buffer_object = false;
	
	gl_config.ext_compiled_vertex_array = false;
	gl_config.ext_texture_filter_anisotropic = false;	
        

	if(strstr(gl_config.extensions_string, "GL_ARB_multitexture"))
	{
		if(r_arb_multitexture->getValue())
		{
			ri.Com_Printf("...using GL_ARB_multitexture\n");
			//glMTexCoord2fSGIS 		= glGetProcAddress("glMultiTexCoord2fARB");
			//glActiveTextureARB	 	= glGetProcAddress("glActiveTextureARB");
			//glClientActiveTextureARB 	= glGetProcAddress("glClientActiveTextureARB");
			gl_config.arb_multitexture = true;
		}
		else
		{
			ri.Com_Printf("...ignoring GL_ARB_multitexture\n");
		}
	}
	else
	{
		ri.Com_Printf("...GL_ARB_multitexture not found\n" );
	}	
	
	if(strstr(gl_config.extensions_string, "GL_ARB_texture_compression"))
	{
		if(r_arb_texture_compression->getValue())
		{
			Com_Printf("...using GL_ARB_texture_compression\n");
			gl_config.arb_texture_compression = true;
		}
		else
		{
			ri.Com_Printf("...ignoring GL_ARB_texture_compression\n");
		}
	}
	else
	{
		ri.Com_Printf("...GL_ARB_texture_compression not found\n");
	}
	
	if(strstr(gl_config.extensions_string, "GL_ARB_vertex_buffer_object"))
	{
		if(r_arb_vertex_buffer_object->getValue())
		{
			ri.Com_Printf("...using GL_ARB_vertex_buffer_object\n");
			gl_config.arb_vertex_buffer_object = true;
			
			xglBindBufferARB = (void (GLAPIENTRY*) (GLenum, GLuint)) XGL_GetSymbol("glBindBufferARB");
			xglDeleteBuffersARB = (void (GLAPIENTRY*) (GLsizei, const GLuint *)) XGL_GetSymbol("glDeleteBuffersARB");
			xglGenBuffersARB = (void (GLAPIENTRY*) (GLsizei, GLuint *)) XGL_GetSymbol("glGenBuffersARB");
			xglIsBufferARB = (GLboolean (GLAPIENTRY*) (GLuint)) XGL_GetSymbol("glIsBufferARB");
			xglBufferDataARB = (void (GLAPIENTRY*) (GLenum, GLsizeiptrARB, const GLvoid *, GLenum)) XGL_GetSymbol("glBufferDataARB");
			xglBufferSubDataARB = (void (GLAPIENTRY*) (GLenum, GLintptrARB, GLsizeiptrARB, const GLvoid *)) XGL_GetSymbol("glBufferSubDataARB");
			xglGetBufferSubDataARB = (void (GLAPIENTRY*) (GLenum, GLintptrARB, GLsizeiptrARB, GLvoid *)) XGL_GetSymbol("glGetBufferSubDataARB");
			xglMapBufferARB = (GLvoid* (GLAPIENTRY*) (GLenum, GLenum)) XGL_GetSymbol("glMapBufferARB");
			xglUnmapBufferARB = (GLboolean (GLAPIENTRY*) (GLenum)) XGL_GetSymbol("glUnmapBufferARB");
			xglGetBufferParameterivARB = (void (GLAPIENTRY*) (GLenum, GLenum, GLint *)) XGL_GetSymbol("glGetBufferParameterivARB");
			xglGetBufferPointervARB = (void (GLAPIENTRY*) (GLenum, GLenum, GLvoid* *)) XGL_GetSymbol("glGetBufferPointervARB");
		}
		else
		{
			ri.Com_Printf("...ignoring GL_ARB_vertex_buffer_object\n");
		}
	}
	else
	{
		ri.Com_Printf("...GL_ARB_vertex_buffer_object not found\n");
	}
	
	if(strstr(gl_config.extensions_string, "GL_EXT_compiled_vertex_array") ||  strstr(gl_config.extensions_string, "GL_SGI_compiled_vertex_array"))
	{
		if(r_ext_compiled_vertex_array->getValue())
		{
			ri.Com_Printf("...using GL_EXT_compiled_vertex_array\n");
			xglLockArraysEXT = (void (GLAPIENTRY*) (GLint first, GLsizei count)) XGL_GetSymbol("glLockArraysEXT");
			xglUnlockArraysEXT = (void (GLAPIENTRY*) (void)) XGL_GetSymbol("glUnlockArraysEXT");
			gl_config.ext_compiled_vertex_array = true;
		}
		else
		{
			ri.Com_Printf("...ignoring GL_EXT_compiled_vertex_array\n");
		}
	}
	else
	{
		ri.Com_Printf("...GL_EXT_compiled_vertex_array not found\n");
	}
	
	if(strstr(gl_config.extensions_string, "GL_EXT_texture_filter_anisotropic"))
	{
		if(r_ext_texture_filter_anisotropic->getValue())
		{
			ri.Com_Printf("...using GL_EXT_texture_filter_anisotropic\n");
			gl_config.ext_texture_filter_anisotropic = true;
		}
		else
		{
			ri.Com_Printf("...ignoring GL_EXT_texture_filter_anisotropic\n");
		}
	}
	else
	{
		ri.Com_Printf("...GL_EXT_texture_filter_anisotropic not found\n");
	}
}


 
static void 	R_Strings_f()
{
	ri.Com_Printf("GL_VENDOR: %s\n", gl_config.vendor_string);
	ri.Com_Printf("GL_RENDERER: %s\n", gl_config.renderer_string);
	ri.Com_Printf("GL_VERSION: %s\n", gl_config.version_string);
	ri.Com_Printf("GL_EXTENSIONS: %s\n", gl_config.extensions_string);
}



static void 	R_AddEntitiesToBuffer()
{
	if(!r_drawentities->getValue())
		return;

	for(std::map<int, r_entity_c>::iterator ir = r_entities.begin(); ir != r_entities.end(); ++ir)
	{
		if(ir->first == 0)
			continue;
			
		r_entity_c& ent = ir->second;
		
		if(r_mirrorview)
		{
			if(ent.getShared().flags & RF_WEAPONMODEL) 
				continue;
		}
		
		r_model_c *model = R_GetModelByNum(ent.getShared().model);
			
		if(!model)
		{
			//R_DrawNULL(r_current_entity->origin, r_current_entity->angles);
			continue;
		}
		
		if(ent.needsUpdate())
			model->precacheLightInteractions(&ent);
		ent.needsUpdate(false);
			
		model->addModelToList(&ent);
	}
}


void 	R_ShadowBlend()
{
	if(r_shadows->getValue() != 1)
		return;
#if 0
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 1, 1, 0, -99999, 99999);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	//RB_SetupOrtho (0, 0, vid.width, vid.height, -99999, 99999);
#endif
	
	xglDisable(GL_CULL_FACE);
	xglDisable(GL_DEPTH_TEST);
	
	xglDisable(GL_ALPHA_TEST);
	xglEnable(GL_BLEND);
	xglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	
	xglEnable(GL_STENCIL_TEST);
	xglStencilFunc(GL_NOTEQUAL, 128, ~0);
	xglStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	
	xglColor4f(0, 0, 0, X_bound(0.0f, r_shadows_alpha->getValue(), 1.0f));

	xglBegin(GL_TRIANGLES);
	xglVertex3f(-5, -5, 0);
	xglVertex3f(10, -5, 0);
	xglVertex3f(-5, 10, 0);

	xglEnd();

	xglDisable(GL_STENCIL_TEST);
	
	//glDisable(GL_BLEND);
	//glEnable(GL_TEXTURE_2D);
	//glEnable(GL_ALPHA_TEST);

	xglColor4fv(color_white);
}


void	R_SetupFrame()
{
	//
	// well a new frame is born ... :)
	//
	r_framecount++;


	//
	// build the transformation matrix for the given view angles
	//
	r_origin = r_newrefdef.view_origin;

	Angles_ToVectors(r_newrefdef.view_angles, r_forward, r_right, r_up);


	//
	// update our scenegraph
	//
	if(r_world_tree)
		r_world_tree->update();
}

void 	R_DrawWorld()
{
	if(!r_drawworld->getValue())
		return;
	
	if(!r_world_tree)
		return;

	if(r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;
		
	r_world_entity.setupTransform();
		
	RB_SetupModelviewMatrix(r_world_entity.getTransform());
	
	r_world_tree->draw();
}



static void	R_BeginFrame()
{
	// change modes if necessary
	if(vid_mode->isModified() || vid_fullscreen->isModified())
	{	
		// FIXME: only restart if CDS is required
		//cvar_t	*ref;
		//ref = 
		ri.Cvar_Get("vid_ref", "gl", 0);
		//ref->modified = true;
	}


	//Tr3B - update hardware gamma
	if(vid_gamma->isModified())
	{
		vid_gamma->isModified(false);

		if(gl_state.hwgamma)
			GLimp_Gamma();
	}

	GLimp_BeginFrame();
	
	// assign shader time
	RB_SetShaderTime(ri.Sys_Milliseconds() * 0.001f);
	
	// go into 2D mode
	RB_SetupGL2D();
		
	// clear screen if desired
	RB_Clear();
}

/*
================
R_RenderView

r_newrefdef must be set before the first call
================
*/
static void 	R_RenderFrame(const r_refdef_t &fd)
{
	int	time_start = 0;
	int	time_setup = 0;
	int	time_world = 0;
	int	time_entities = 0;
	int	time_end = 0;

	r_newrefdef = fd;

	if(!r_world_tree && !(r_newrefdef.rdflags & RDF_NOWORLDMODEL))
		ri.Com_Error(ERR_DROP, "R_RenderFrame: NULL worldmodel");
		
	if(r_speeds->getInteger())
		time_start = ri.Sys_Milliseconds();
		
	r_current_scene = &r_world_scene;
			
	RB_BeginBackendFrame();

	R_SetupFrame();
			
	RB_SetupGL3D();
	
	if(r_speeds->getInteger())
		time_setup = ri.Sys_Milliseconds();
	
	R_DrawWorld();
	
	R_DrawSky();
	
	if(r_speeds->getInteger())
		time_world = ri.Sys_Milliseconds();
		
	R_AddEntitiesToBuffer();
	
	RB_RenderCommands();
	
	if(r_speeds->getInteger())
		time_entities = ri.Sys_Milliseconds();
		
	//R_AddPolysToBuffer();
		
	//R_DrawParticles();
	
	RB_SetupGL2D();
	
	//R_ShadowBlend();
		
	RB_EndBackendFrame();
	
	RB_CheckForError();
	
	if(r_speeds->getInteger())
		time_end = ri.Sys_Milliseconds();
	
	if(r_speeds->getInteger())
	{
		ri.Com_Printf("%4i leafs %4i cmds %4i radiosity cmds %4i light cmds %4i tris %4i flushes %4i exp\n",
			r_leafs_counter,
			r_cmds_counter,
			r_cmds_radiosity_counter,
			r_cmds_light_counter,
			rb_triangles_counter,
			rb_flushes_counter,
			rb_expressions_counter);
			
		int	all, setup, world, entities;

		all = time_end - time_start;
		setup = time_setup - time_start;
		world = time_world - time_setup;
		entities = time_entities - time_world;
		
		//ri.Com_Printf("%4i all %4i setup %4i world %4i entities\n", all, setup, world, entities);	
		//ri.Com_Printf("%4i ents %4i lights %4i particles %4i polies\n", r_entities.size(), r_lights.size(), r_particles_num, r_polys_num);
	}
}

static void	R_EndFrame()
{
	if(r_video_export->getInteger())
		R_WriteVideoScreenShot();
		
	GLimp_EndFrame();
}


static void	R_BenchCalcTangentSpaces_f()
{
	int start, end;
	r_mesh_c m;
	
	if(ri.Cmd_Argc() != 2)
	{
		ri.Com_Printf("usage: benchcalctangentspaces <vertices number>\n");
		return;
	}
	
	int vertexes_num = atoi(ri.Cmd_Argv(1));
	if(!vertexes_num)
		vertexes_num = 10000;
	
	m.fillVertexes(vertexes_num);
	
	m.indexes = std::vector<index_t>(vertexes_num*6);
	for(uint_t i=0; i<m.indexes.size(); i++)
	{
		m.indexes[i] = i % (vertexes_num-1);
	}
	
	start = ri.Sys_Microseconds();
	m.calcTangentSpaces();
	end = ri.Sys_Microseconds();
	ri.Com_Printf("calcTangentSpaces: %i\n", end - start);
	
	start = ri.Sys_Microseconds();
	m.calcTangentSpaces2();
	end = ri.Sys_Microseconds();
	ri.Com_Printf("calcTangentSpaces2: %i\n", end - start);
	
//	start = ri.Sys_Microseconds();
//	m.calcTangentSpaces3();
//	end = ri.Sys_Microseconds();
//	ri.Com_Printf("calcTangentSpaces3: %i\n", end - start);
}


static void 	R_Register()
{
	r_lefthand 		= ri.Cvar_Get("hand", "0", CVAR_USERINFO | CVAR_ARCHIVE );
	
	r_fullbright 		= ri.Cvar_Get("r_fullbright", "0", CVAR_NONE);
	r_draw2d 		= ri.Cvar_Get("r_draw2d", "1", CVAR_NONE);
	r_drawentities 		= ri.Cvar_Get("r_drawentities", "1", CVAR_ARCHIVE);
	r_drawworld 		= ri.Cvar_Get("r_drawworld", "1", CVAR_ARCHIVE);
	r_drawparticles		= ri.Cvar_Get("r_drawparticles", "1", CVAR_ARCHIVE);
	r_drawpolygons		= ri.Cvar_Get("r_drawpolygons", "1", CVAR_ARCHIVE);
	r_drawsky		= ri.Cvar_Get("r_drawsky", "1", CVAR_ARCHIVE);
	r_drawextra		= ri.Cvar_Get("r_drawextra", "1", CVAR_ARCHIVE);
	r_drawtranslucent	= ri.Cvar_Get("r_drawtranslucent", "1", CVAR_ARCHIVE);
	r_lerpmodels 		= ri.Cvar_Get("r_lerpmodels", "1", CVAR_NONE);
	r_speeds 		= ri.Cvar_Get("r_speeds", "0", CVAR_NONE);
	r_log	 		= ri.Cvar_Get("r_log", "0", 0 );
	r_shadows 		= ri.Cvar_Get("r_shadows", "0", CVAR_ARCHIVE );
	r_shadows_alpha		= ri.Cvar_Get("r_shadows_alpha", "0.5", CVAR_ARCHIVE);
	r_shadows_nudge		= ri.Cvar_Get("r_shadows_nudge", "1", CVAR_ARCHIVE);
	r_lighting 		= ri.Cvar_Get("r_lighting", "1", CVAR_ARCHIVE);
	r_lighting_omni		= ri.Cvar_Get("r_lighting_omni", "1", CVAR_ARCHIVE);
	r_lighting_proj 	= ri.Cvar_Get("r_lighting_proj", "1", CVAR_ARCHIVE);
	r_lightmap		= ri.Cvar_Get("r_lightmap", "1", CVAR_ARCHIVE);
	r_lightscale		= ri.Cvar_Get("r_lightscale", "4", CVAR_ARCHIVE);
	r_znear			= ri.Cvar_Get("r_znear", "4.0", CVAR_ARCHIVE);
	r_zfar			= ri.Cvar_Get("r_zfar", "65536.0", CVAR_ARCHIVE);
	r_nobind 		= ri.Cvar_Get("r_nobind", "0", CVAR_NONE);
	r_picmip 		= ri.Cvar_Get("r_picmip", "0", CVAR_ARCHIVE);
	r_skymip 		= ri.Cvar_Get("r_skymip", "0", CVAR_ARCHIVE);

	r_showtris 		= ri.Cvar_Get("r_showtris", "0", CVAR_NONE);
	r_showbbox 		= ri.Cvar_Get("r_showbbox", "0", CVAR_NONE);
	r_showareas		= ri.Cvar_Get("r_showareas", "0", CVAR_NONE);
	r_showareaportals	= ri.Cvar_Get("r_showareaportals", "0", CVAR_NONE);
	r_shownormals 		= ri.Cvar_Get("r_shownormals", "0", CVAR_NONE);
	r_showtangents		= ri.Cvar_Get("r_showtangents", "0", CVAR_NONE);
	r_showbinormals		= ri.Cvar_Get("r_showbinormals", "0", CVAR_NONE);
	r_showinvisible		= ri.Cvar_Get("r_showinvisible", "0", CVAR_NONE);

	r_clear 		= ri.Cvar_Get("r_clear", "1", CVAR_NONE);
	r_cull 			= ri.Cvar_Get("r_cull", "1", CVAR_NONE);
	r_cullplanes		= ri.Cvar_Get("r_cullplanes", "1", CVAR_NONE);
	r_cullportals		= ri.Cvar_Get("r_cullportals", "1", CVAR_NONE);
	r_polyblend 		= ri.Cvar_Get("r_polyblend", "1", CVAR_ARCHIVE);
	r_flashblend 		= ri.Cvar_Get("r_flashblend", "0", CVAR_ARCHIVE);
	r_playermip 		= ri.Cvar_Get("r_playermip", "0", 0);
	r_texturemode 		= ri.Cvar_Get("r_texturemode", "GL_LINEAR_MIPMAP_NEAREST", CVAR_NONE);
	r_polygonmode		= ri.Cvar_Get("r_polygonmode", "GL_FILL", CVAR_NONE);
	r_lockpvs 		= ri.Cvar_Get("r_lockpvs", "0", CVAR_NONE);
	r_vis 			= ri.Cvar_Get("r_vis", "1", CVAR_NONE);
	r_maxtexsize		= ri.Cvar_Get("r_maxtexsize", "1024", CVAR_ARCHIVE);
	r_finish		= ri.Cvar_Get("r_finish", "1", CVAR_ARCHIVE);
	r_subdivisions		= ri.Cvar_Get("r_subdivisions", "0", CVAR_ARCHIVE);
	r_flares		= ri.Cvar_Get("r_flares", "0", CVAR_ARCHIVE);
	r_flaresize		= ri.Cvar_Get("r_flaresize", "15", CVAR_NONE);
	r_flarefade		= ri.Cvar_Get("r_flarefade", "7", CVAR_NONE);
	r_octree		= ri.Cvar_Get("r_octree", "0", CVAR_ARCHIVE);
	r_bump_mapping		= ri.Cvar_Get("r_bump_mapping", "1", CVAR_ARCHIVE);
	r_gloss			= ri.Cvar_Get("r_gloss", "1", CVAR_ARCHIVE);
	r_parallax		= ri.Cvar_Get("r_parallax", "1", CVAR_ARCHIVE);
	r_cmds_max		= ri.Cvar_Get("r_cmds_max", "8192", CVAR_ARCHIVE);
	r_cmds_light_max	= ri.Cvar_Get("r_cmds_light_max", "8192", CVAR_ARCHIVE);
	r_cmds_translucent_max	= ri.Cvar_Get("r_cmds_translucent_max", "8192", CVAR_ARCHIVE);
	r_video_export		= ri.Cvar_Get("r_video_export", "0", CVAR_NONE);
	r_evalexpressions	= ri.Cvar_Get("r_evalexpressions", "1", CVAR_NONE);
	
	r_drawbuffer 		= ri.Cvar_Get("r_drawbuffer", "GL_BACK", CVAR_NONE);

	// opengl extensions
	r_arb_multitexture 	= ri.Cvar_Get("r_arb_multitexture", "1", CVAR_ARCHIVE);
	r_arb_texture_compression = ri.Cvar_Get("r_arb_texture_compression", "0", CVAR_ARCHIVE);
	r_arb_vertex_buffer_object = ri.Cvar_Get("r_arb_vertex_buffer_object", "1", CVAR_ARCHIVE);
	
	r_ext_compiled_vertex_array = ri.Cvar_Get("r_ext_compiled_vertex_array", "0", CVAR_ARCHIVE);
	r_ext_texture_filter_anisotropic = ri.Cvar_Get("r_ext_texture_filter_anisotropic", "0", CVAR_ARCHIVE);
	r_ext_texture_filter_anisotropic_level = ri.Cvar_Get("r_ext_texture_filter_anisotropic_level", "1", CVAR_ARCHIVE);

	r_sgix_fbconfig		= ri.Cvar_Get("r_sgix_fbconfig", "1", CVAR_ARCHIVE);
	r_sgix_pbuffer		= ri.Cvar_Get("r_sgix_pbuffer", "1", CVAR_ARCHIVE);

	vid_fullscreen 		= ri.Cvar_Get("vid_fullscreen", "0", CVAR_ARCHIVE);
	vid_gamma 		= ri.Cvar_Get("vid_gamma", "1.0", CVAR_ARCHIVE);
	vid_mode 		= ri.Cvar_Get("vid_mode", "0", CVAR_ARCHIVE);
#ifdef _WIN32
	vid_gldriver 		= ri.Cvar_Get("vid_gldriver", "opengl32", CVAR_ARCHIVE);
#elif __linux__
	vid_gldriver 		= ri.Cvar_Get("vid_gldriver", "libGL.so.1", CVAR_ARCHIVE);
#else
	vid_gldriver 		= ri.Cvar_Get("vid_gldriver", "libGL.so", CVAR_ARCHIVE);
#endif
	vid_colorbits		= ri.Cvar_Get("vid_colorbits", "0", CVAR_ARCHIVE);
	vid_depthbits		= ri.Cvar_Get("vid_depthbits", "0", CVAR_ARCHIVE);
	vid_stencilbits		= ri.Cvar_Get("vid_stencilbits", "0", CVAR_ARCHIVE);
	
	vid_pbuffer_texsize	= ri.Cvar_Get("vid_pbuffer_texsize", "1024", CVAR_ARCHIVE);
	vid_pbuffer_colorbits	= ri.Cvar_Get("vid_pbuffer_colorbits", "16", CVAR_ARCHIVE);
	vid_pbuffer_depthbits	= ri.Cvar_Get("vid_pbuffer_depthbits", "0", CVAR_ARCHIVE);
	vid_pbuffer_stencilbits	= ri.Cvar_Get("vid_pbuffer_stencilbits", "0", CVAR_ARCHIVE);
	
	vid_ref 		= ri.Cvar_Get("vid_ref", "glx", CVAR_ARCHIVE);



	
	ri.Cmd_AddCommand("imagelist", 		R_ImageList_f);	
	ri.Cmd_AddCommand("screenshot",		R_ScreenShot_f);
	ri.Cmd_AddCommand("modellist", 		R_Modellist_f);
	ri.Cmd_AddCommand("gl_strings",		R_Strings_f);
	ri.Cmd_AddCommand("shaderlist",		R_ShaderList_f);
	ri.Cmd_AddCommand("shadercachelist",	R_ShaderCacheList_f);
	ri.Cmd_AddCommand("shadersearch",	R_ShaderSearch_f);
	ri.Cmd_AddCommand("spirittest",		R_SpiritTest_f);
	ri.Cmd_AddCommand("skinlist",		R_SkinList_f);
	
	ri.Cmd_AddCommand("benchcalctangentspaces",	R_BenchCalcTangentSpaces_f);
}

static bool 	R_SetMode()
{
	r_serr_e err;
	bool fullscreen;

	if(vid_fullscreen->isModified() && !gl_config.allow_cds)
	{
		ri.Com_Printf("R_SetMode: CDS not allowed with this driver\n");
		ri.Cvar_SetValue("vid_fullscreen", !vid_fullscreen->getValue());
		vid_fullscreen->isModified(false);
	}

	fullscreen = (bool)vid_fullscreen->getInteger();

	vid_fullscreen->isModified(false);
	vid_mode->isModified(false);

	if((err = (r_serr_e)GLimp_SetMode((int*)&vid.width, (int*)&vid.height, vid_mode->getInteger(), fullscreen)) == RSERR_OK)
	{
		gl_state.prev_mode = vid_mode->getInteger();
	}
	else
	{
		if(err == RSERR_INVALID_FULLSCREEN)
		{
			ri.Cvar_SetValue("vid_fullscreen", 0);
			vid_fullscreen->isModified(false);

			ri.Com_Printf("R_SetMode: fullscreen unavailable in this mode\n");
			
			if((err = (r_serr_e)GLimp_SetMode((int*)&vid.width, (int*)&vid.height, vid_mode->getInteger(), false)) == RSERR_OK)
				return true;
		}
		else if(err == RSERR_INVALID_MODE)
		{
			ri.Cvar_SetValue("r_mode", gl_state.prev_mode );
			vid_mode->isModified(false);
			
			ri.Com_Printf("R_SetMode: invalid mode\n" );
		}

		// try setting it back to something safe
		if((err = (r_serr_e)GLimp_SetMode((int*)&vid.width, (int*)&vid.height, gl_state.prev_mode, false)) != RSERR_OK)
		{
			ri.Com_Printf("R_SetMode: could not revert to safe mode\n");
			return false;
		}
	}
	return true;
}

void	R_InitTree(r_tree_type_e type, const std::string &name)
{
	switch(type)
	{
		case TREE_BSP:
			r_world_tree = new r_bsptree_c("maps/" + name);
			break;
		
		case TREE_PROC:
			r_world_tree = new r_proctree_c("maps/" + name + ".proc");
			break;
	}
}

void	R_ShutdownTree()
{
	if(r_world_tree)
		delete r_world_tree;
		
	r_world_tree = NULL;
}


static void	R_InitQuadIndexes()
{
	r_quad_indexes.push_back(0);
	r_quad_indexes.push_back(1);
	r_quad_indexes.push_back(2);
	r_quad_indexes.push_back(0);
	r_quad_indexes.push_back(2);
	r_quad_indexes.push_back(3);
}


bool 	R_Init(void *hinstance, void *hWnd)
{	
	ri.Com_Printf("------- R_Init -------\n");

	ri.Com_Printf("ref_gl version: "REF_VERSION"\n");

	R_Register();
	
	// initialize our QGL dynamic bindings
	if(!XGL_Init(vid_gldriver->getString()))
	{
		XGL_Shutdown();
        	ri.Com_Printf("R_Init: could not load \"%s\"\n", vid_gldriver->getString());
		return false;
	}
	
	// initialize OS-specific parts of OpenGL
	if(!GLimp_Init(hinstance, hWnd))
	{
		return false;
	}

	// set our "safe" modes
	gl_state.prev_mode = 3;
  	gl_config.allow_cds = true;

	if(gl_config.allow_cds)
		ri.Com_Printf("...allowing CDS\n" );
	else
		ri.Com_Printf("...disabling CDS\n" );

	// create the window and set up the context
	if(!R_SetMode())
	{
		XGL_Shutdown();
		ri.Com_Printf("R_Init: could not R_SetMode()\n");
		return false;
	}

	// get our various GL strings
	gl_config.vendor_string = (const char*)xglGetString (GL_VENDOR);
	ri.Com_Printf("GL_VENDOR: %s\n", gl_config.vendor_string );
	
	gl_config.renderer_string = (const char*)xglGetString (GL_RENDERER);
	ri.Com_Printf("GL_RENDERER: %s\n", gl_config.renderer_string );
	
	gl_config.version_string = (const char*)xglGetString (GL_VERSION);
	ri.Com_Printf("GL_VERSION: %s\n", gl_config.version_string );
	
	gl_config.extensions_string = (const char*)xglGetString (GL_EXTENSIONS);
	ri.Com_Printf("GL_EXTENSIONS: %s\n", gl_config.extensions_string );


	// grab optional OpenGL extensions
	R_CheckOpenGLExtensions();
	
	// setup shadow mapping support
	GLimp_InitPbuffer(false, true);
	GLimp_ActivatePbuffer();
	xglClearColor(0.6, 0.1, 0.91, 0.0);
	xglEnable(GL_DEPTH_TEST);
	{
		//TODO recode
		
        	GLint depth_bits;
        	xglGetIntegerv(GL_DEPTH_BITS, & depth_bits);
        
        	if(depth_bits == 16)
			r_depth_format = GL_DEPTH_COMPONENT16_ARB;
        	else
			r_depth_format = GL_DEPTH_COMPONENT24_ARB;
	}
	GLimp_DeactivatePbuffer();

	// setup image system
	R_InitImages();
		
        // setup Q3A style shader system
	R_InitShaders();
	
	// setup skin system
	R_InitSkins();
	
	// setup MD5 animation system
	R_InitAnimations();
	
	// setup quad indices for 2d drawing
	R_InitQuadIndexes();

	// setup default images
	R_InitDraw();
	
	R_InitParticles();
	R_InitPolys();
	
	// setup sky
	//R_InitSkyDome();
	
	// setup scenegraph
	//R_InitTree();
	
	// setup OpenGL renderer backend
	RB_InitBackend();
	
	RB_CheckForError();
	
	ri.Com_Printf("------- R_Init completed-------\n");

	return true;
}


void 	R_Shutdown()
{		
	ri.Com_Printf("------- R_Shutdown -------\n");
	
	//
	// remove cmds
	//
	ri.Cmd_RemoveCommand("imagelist");
	ri.Cmd_RemoveCommand("screenshot");
	ri.Cmd_RemoveCommand("modellist");
	ri.Cmd_RemoveCommand("gl_strings");
	ri.Cmd_RemoveCommand("shaderlist");
	ri.Cmd_RemoveCommand("shadercachelist");
	ri.Cmd_RemoveCommand("shadersearch");
	ri.Cmd_RemoveCommand("spirittest");
	ri.Cmd_RemoveCommand("skinlist");
	

	//
	// shutdown helper system
	//
	R_ShutdownTree();
	
	R_ShutdownModels();
	
	R_ShutdownAnimations();

	RB_ShutdownBackend();
	
	R_ShutdownSkins();
	
	R_ShutdownShaders();

	R_ShutdownImages();

	GLimp_Shutdown();
	
	XGL_Shutdown();
}




#if 0
static void 	R_DrawBeam( r_entity_t *e )
{
#define NUM_BEAM_SEGS 6

	int	i;
	float r, g, b;

	vec3_t perpvec;
	vec3_t direction, normalized_direction;
	vec3_t	start_points[NUM_BEAM_SEGS], end_points[NUM_BEAM_SEGS];
	vec3_t oldorigin, origin;

	oldorigin[0] = e->oldorigin[0];
	oldorigin[1] = e->oldorigin[1];
	oldorigin[2] = e->oldorigin[2];

	origin[0] = e->origin[0];
	origin[1] = e->origin[1];
	origin[2] = e->origin[2];


	normalized_direction[0] = direction[0] = oldorigin[0] - origin[0];
	normalized_direction[1] = direction[1] = oldorigin[1] - origin[1];
	normalized_direction[2] = direction[2] = oldorigin[2] - origin[2];

	if ( Vector3_Normalize( normalized_direction ) == 0 )
		return;

	Vector3_Perpendicular( perpvec, normalized_direction );
	Vector3_Scale( perpvec, e->frame / 2, perpvec );


	for ( i = 0; i < 6; i++ )
	{
		RotatePointAroundVector( start_points[i], normalized_direction, perpvec, (360.0/NUM_BEAM_SEGS)*i );
		Vector3_Add( start_points[i], origin, start_points[i] );
		Vector3_Add( start_points[i], direction, end_points[i] );
	}

	
	r = ( r_8to24table[e->skin_num & 0xFF] ) & 0xFF;
	g = ( r_8to24table[e->skin_num & 0xFF] >> 8 ) & 0xFF;
	b = ( r_8to24table[e->skin_num & 0xFF] >> 16 ) & 0xFF;

	r *= 1/255.0F;
	g *= 1/255.0F;
	b *= 1/255.0F;
	
	Vector3_Set (e->color, r, g, b);


	//glColor4fv( r, g, b, e->color[3] );
	glColor4fv (e->color);

	glBegin( GL_TRIANGLE_STRIP );
	for ( i = 0; i < NUM_BEAM_SEGS; i++ )
	{
		glVertex3fv( start_points[i] );
		glVertex3fv( end_points[i] );
		glVertex3fv( start_points[(i+1)%NUM_BEAM_SEGS] );
		glVertex3fv( end_points[(i+1)%NUM_BEAM_SEGS] );
	}
	glEnd();
}
#endif



bool	R_SetupTag(r_tag_t &tag, const r_entity_t &ent, const std::string &name)
{
	if(name.empty())
		return false;
		
	tag.origin.clear();
	tag.quat.identity();
	
	r_model_c *mod = R_GetModelByNum(ent.model);
	if(!mod)
		return false;
	else
		return mod->setupTag(tag, ent, name);
}


bool	R_SetupAnimation(int model, int animation)
{
	r_skel_animation_c *anim = R_GetAnimationByNum(animation);
	if(!anim)
	{
		ri.Com_Printf("R_SetupAnimation: couldn't get animation by number");
		return false;
	}

	r_model_c *mod = R_GetModelByNum(model);
	if(!mod)
		return false;
	else
		return mod->setupAnimation(anim);
}



static void	R_ClearScene()
{
//	r_entities.clear();
//	r_lights.clear();
	r_particles_num = 0;
	r_polys_num = 0;
}

static void	R_AddEntity(int entity_num, const r_entity_t &shared)
{
	std::map<int, r_entity_c>::iterator ir = r_entities.find(entity_num);
	
	if(ir == r_entities.end())
	{
		r_entities.insert(std::make_pair(entity_num, r_entity_c(shared, true)));
	}
	else
	{
		ir->second = r_entity_c(shared, true);
	}
}

static void	R_UpdateEntity(int entity_num, const r_entity_t &shared, bool update)
{
	std::map<int, r_entity_c>::iterator ir = r_entities.find(entity_num);
	
	if(ir != r_entities.end())
	{
		if(!update && ir->second.needsUpdate())
			update = true;
			
		ir->second = r_entity_c(shared, update);
	}
}

static void	R_RemoveEntity(int entity_num)
{
	std::map<int, r_entity_c>::iterator ir = r_entities.find(entity_num);
	
	if(ir != r_entities.end())
	{
		r_entities.erase(ir);
	}
}


static void	R_AddLight(int entity_num, const r_entity_t &shared)
{
	std::map<int, r_light_c>::iterator ir = r_lights.find(entity_num);
	
	if(ir == r_lights.end())
	{
		r_lights.insert(std::make_pair(entity_num, r_light_c(shared)));
	}
	else
	{
		ir->second = r_light_c(shared);
	}
}

static void	R_UpdateLight(int entity_num, const r_entity_t &shared)
{
	std::map<int, r_light_c>::iterator ir = r_lights.find(entity_num);
	
	if(ir != r_lights.end())
	{
		ir->second = r_light_c(shared);
	}
}

static void	R_RemoveLight(int entity_num)
{
	std::map<int, r_light_c>::iterator ir = r_lights.find(entity_num);
	
	if(ir != r_lights.end())
	{
		r_lights.erase(ir);
	}
}

static void	R_AddParticle(const r_particle_t &part)
{
	if(r_particles_num >= MAX_PARTICLES)
		return;
		
	r_particles[r_particles_num++] = part;
}

static void	R_AddPoly(const r_poly_t &poly)
{
	if(r_polys_num >= MAX_POLYS)
		return;
		
	r_polys[r_polys_num++] = poly;
}





#ifdef __cplusplus
extern "C" {
#endif


ref_export_t 	GetRefAPI(ref_import_t rimp)
{
	ref_export_t	re;

	ri = rimp;

	re.api_version 			= REF_API_VERSION;

	re.Init       			= R_Init;

	re.Shutdown			= R_Shutdown;

	re.R_BeginRegistration 		= R_BeginRegistration;

	re.R_RegisterModel		= R_RegisterModelExp;
	re.R_RegisterAnimation		= R_RegisterAnimationExp;
	re.R_RegisterSkin		= R_RegisterSkinExp;
	re.R_RegisterShader		= R_RegisterShaderExp;
	re.R_RegisterPic		= R_RegisterPicExp;
	re.R_RegisterParticle		= R_RegisterParticleExp;
	re.R_RegisterLight		= R_RegisterLightExp;
	re.R_EndRegistration 		= R_EndRegistration;

	re.R_BeginFrame			= R_BeginFrame;
	re.R_RenderFrame		= R_RenderFrame;
	re.R_EndFrame			= R_EndFrame;

	re.R_DrawPic			= R_DrawPicExp;
	re.R_DrawStretchPic 		= R_DrawStretchPicExp;
	re.R_DrawFill			= R_DrawFill;
	
	re.R_SetSky			= R_SetSky;
	
	re.R_ClearScene			= R_ClearScene;
	
	re.R_AddEntity			= R_AddEntity;
	re.R_UpdateEntity		= R_UpdateEntity;
	re.R_RemoveEntity		= R_RemoveEntity;
	
	re.R_AddLight			= R_AddLight;
	re.R_UpdateLight		= R_UpdateLight;
	re.R_RemoveLight		= R_RemoveLight;
	
	re.R_AddParticle		= R_AddParticle;
	re.R_AddPoly			= R_AddPoly;
	
	re.R_SetupTag			= R_SetupTag;
	re.R_SetupAnimation		= R_SetupAnimation;
		
	re.AppActivate 		   	= GLimp_AppActivate;
	
	Swap_Init();

	return re;
}

#ifdef __cplusplus
}
#endif


#ifndef REF_HARD_LINKED
// this is only here so the functions in x_shared.c and x_shwin.c can link
void 	Com_Error(err_type_e type, const char *fmt, ...)
{
	va_list		argptr;
	char		text[1024];

	va_start(argptr, fmt);
	vsprintf(text, fmt, argptr);
	va_end(argptr);

	ri.Com_Error(type, "%s", text);
}

void	Com_Printf(const char *fmt, ...)
{
	va_list		argptr;
	char		text[1024];

	va_start(argptr, fmt);
	vsprintf(text, fmt, argptr);
	va_end(argptr);

	ri.Com_Printf("%s", text);
}

#endif





