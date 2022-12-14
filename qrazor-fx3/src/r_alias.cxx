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

r_alias_mesh_frame_t::~r_alias_mesh_frame_t()
{

}


void	r_alias_mesh_frame_t::calcTangentSpaces(const std::vector<index_t> &indexes)
{
#if 1
	for(unsigned int i=0; i<vertexes.size(); i++)
	{
		tangents[i].clear();
		tangents[i].clear();
		tangents[i].clear();
		
		binormals[i].clear();
		binormals[i].clear();
		binormals[i].clear();
		
		normals[i].clear();
		normals[i].clear();
		normals[i].clear();
	}

	for(unsigned int i=0; i<indexes.size(); i += 3)
	{
		const vec3_c &v0 = vertexes[indexes[i+0]];
		const vec3_c &v1 = vertexes[indexes[i+1]];
		const vec3_c &v2 = vertexes[indexes[i+2]];
			
		const vec2_c &t0 = texcoords[indexes[i+0]];
		const vec2_c &t1 = texcoords[indexes[i+1]];
		const vec2_c &t2 = texcoords[indexes[i+2]];
			
		// compute the face normal based on vertex points
		vec3_c face_normal;
		face_normal.crossProduct(v2-v0, v1-v0);
		face_normal.normalize();
		
		vec3_c tangent, binormal, normal;
		
		R_CalcTangentSpace(tangent, binormal, normal, v0, v1, v2, t0, t1, t2, face_normal);
		
		tangents[indexes[i+0]] += tangent;
		tangents[indexes[i+1]] += tangent;
		tangents[indexes[i+2]] += tangent;
		
		binormals[indexes[i+0]] += binormal;
		binormals[indexes[i+1]] += binormal;
		binormals[indexes[i+2]] += binormal;
		
		normals[indexes[i+0]] += normal;
		normals[indexes[i+1]] += normal;
		normals[indexes[i+2]] += normal;
	}
	
	for(unsigned int i=0; i<vertexes.size(); i++)
	{
		tangents[i].normalize();
		tangents[i].normalize();
		tangents[i].normalize();
		
		binormals[i].normalize();
		binormals[i].normalize();
		binormals[i].normalize();
		
		normals[i].normalize();
		normals[i].normalize();
		normals[i].normalize();
	}
#endif
}




r_alias_frame_t::~r_alias_frame_t()
{
	X_purge<std::vector<r_alias_mesh_frame_t*> >(meshframes);
	
	X_purge<std::vector<r_alias_tag_t*> >(tags);
}






r_alias_model_c::r_alias_model_c(const std::string &name, byte *buffer, uint_t buffer_size)
:r_model_c(name, buffer, buffer_size, MOD_ALIAS)
{
	//ri.Com_Printf ("r_alias_model_c::ctor: %s\n", name);
}

r_alias_model_c::~r_alias_model_c()
{
	for(std::vector<r_alias_frame_t*>::const_iterator ir = _frames.begin(); ir != _frames.end(); ir++)
	{
		delete *ir;
	}
	
	_frames.clear();
}

bool	r_alias_model_c::setupTag(r_tag_t &tag, const r_entity_t &ent, const std::string &name)
{
	r_alias_frame_t*	frame;
	r_alias_frame_t*	frame_old;
	r_alias_tag_t		*atag;
	r_alias_tag_t		*atag_old;
	
	
	//ri.Com_Printf("r_alias_model_c::setupTag: model '%s'\n", getName());

	//
	// get frames
	//
	if(ent.frame < 0 || ent.frame >= (int)_frames.size())
	{
		ri.Com_Printf("r_alias_model_c::setupTag: model %s has no such frame %d\n", getName(), ent.frame);
		return false;
	}
	
	if(ent.frame_old < 0 || ent.frame_old >= (int)_frames.size())
	{
		ri.Com_Printf("r_alias_model_c::setupTag: model %s has no such frame %d\n", getName(), ent.frame_old);
		return false;
	}
	
	
	frame = _frames[ent.frame];
	frame_old = _frames[ent.frame_old];
		
	
	//
	// get tags
	//
	int						tag_num;
	std::vector<r_alias_tag_t*>::const_iterator	ir;
	
	for(tag_num=0, ir=frame->tags.begin(); ir != frame->tags.end(); tag_num++, ir++)
	{
		if(X_strcaseequal((*ir)->name.c_str(), name.c_str()))
			break;
	}
	
	if(tag_num < 0 || tag_num >= (int)frame->tags.size())
	{
		ri.Com_Printf("r_alias_model_c::setupTag: model '%s' has no such tag '%s' %i %i\n", getName(), name.c_str(), frame->tags.size(), tag_num);
		return false;
	}
		

	atag	= frame->tags[tag_num];
	atag_old = frame_old->tags[tag_num];


	//
	// lerp orientation and origin
	//	
	tag.quat.slerp(atag_old->quat, atag->quat, r_newrefdef.lerp);
	tag.origin.lerp(atag_old->origin, atag->origin, r_newrefdef.lerp);

	return true;
}


void	r_alias_model_c::drawFrameLerp(const r_command_t *cmd, r_render_type_e type)
{
	vec3_c	move;

	r_alias_frame_t*	frame;
	r_alias_frame_t*	frame_old;
	r_alias_mesh_frame_t*	mesh_frame;
	r_alias_mesh_frame_t*	mesh_frame_old;
	
	r_mesh_c*		mesh;
	
	if(!cmd)
		return;
	
	if(!cmd->getEntity())
		return;
	
	if(!(mesh = cmd->getEntityMesh()))
		return;
		
	//ri.Com_Printf("r_alias_model_c::drawFrameLerp: model '%s' %i\n", getName(), mb.mesh->indexes.size());
		
	//if(!r_lerpmodels->getInteger())
	//	cmd->getEntity()->getShared().backlerp = 0;
	
	
	//
	// get frames
	//
	if(cmd->getEntity()->getShared().frame < 0 || cmd->getEntity()->getShared().frame >= (int)_frames.size())
	{
		ri.Com_Printf("r_alias_model_c::drawFrameLerp: model %s has no such frame %d\n", getName(), cmd->getEntity()->getShared().frame);
		return;
	}
	
	if(cmd->getEntity()->getShared().frame_old < 0 || cmd->getEntity()->getShared().frame_old >= (int)_frames.size())
	{
		ri.Com_Printf("r_alias_model_c::drawFrameLerp: model %s has no such frame %d\n", getName(), cmd->getEntity()->getShared().frame_old);
		return;
	}
	
	
	frame = _frames[cmd->getEntity()->getShared().frame];
	frame_old = _frames[cmd->getEntity()->getShared().frame_old];
	
	
	//
	// get mesh frames
	//
	int mesh_num = -cmd->getInfoKey()-1;
	mesh_frame = frame->meshframes[mesh_num];
	mesh_frame_old = frame_old->meshframes[mesh_num];
	
	
	//
	// create movement using lerping between old and new frame translation lerping
	//
	move.lerp(frame_old->translate, frame->translate, r_newrefdef.lerp);
	

	//
	// update vertices
	//
	if(type != RENDER_TYPE_SHADOWING)
	{
		for(uint_t i=0; i<mesh->vertexes.size(); i++)
		{
			mesh->vertexes[i] = move;	
			vec3_c tmp(false); tmp.lerp(mesh_frame_old->vertexes[i], mesh_frame->vertexes[i], r_newrefdef.lerp);
			mesh->vertexes[i] += tmp;
			
			//FIXME do this somewhere else
			mesh->texcoords[i] = mesh_frame->texcoords[i];
			
			mesh->tangents[i].lerp(mesh_frame_old->tangents[i], mesh_frame->tangents[i], r_newrefdef.lerp);
			mesh->tangents[i].normalize();
			
			mesh->binormals[i].lerp(mesh_frame_old->binormals[i], mesh_frame->binormals[i], r_newrefdef.lerp);
			mesh->binormals[i].normalize();
		
			mesh->normals[i].lerp(mesh_frame_old->normals[i], mesh_frame->normals[i], r_newrefdef.lerp);
			mesh->normals[i].normalize();
		}

		if(cmd->getEntity()->getShared().flags & RF_FULLBRIGHT)
		{
			for(uint_t i=0; i<mesh->vertexes.size(); i++)
				mesh->lights[i] = mesh->normals[i];
		}
	}
	else
	{
		//TODO
	}
	
	
	//
	// draw lerped mesh
	//
	RB_RenderCommand(cmd, type);
}

bool	r_alias_model_c::cull(r_entity_c *ent)
{
	if(ent->getShared().flags & RF_WEAPONMODEL)
		return false;
	
	if(ent->getShared().flags & RF_VIEWERMODEL)
		return (!(r_mirror_view || r_portal_view));
	
	if(r_frustum.cull(ent->getAABB()))
		return true;
	
	if((r_mirror_view || r_portal_view) && r_cull->getInteger())
	{
		if(r_clipplane.distance(ent->getShared().origin) < -ent->getAABB().radius())
			return true;
	}
	
	return false;
}

const aabb_c	r_alias_model_c::createAABB(r_entity_c *ent) const
{
	// get frames
	if((ent->getShared().frame < 0) || (ent->getShared().frame >= (int)_frames.size()))
	{
		ri.Com_Printf("r_alias_model_c::createAABB: model %s has no such frame %d\n", getName(), ent->getShared().frame);
	}
	
	if((ent->getShared().frame_old < 0) || (ent->getShared().frame_old >= (int)_frames.size()))
	{
		ri.Com_Printf ("r_alias_model_c::createAABB: model %s has no such oldframe %d\n", getName(), ent->getShared().frame_old);
	}

	r_alias_frame_t* frame		= _frames[X_bound(0, ent->getShared().frame, (int)_frames.size()-1)];
	r_alias_frame_t* frame_old	= _frames[X_bound(0, ent->getShared().frame_old, (int)_frames.size()-1)];

	// compute axially aligned mins and maxs
	aabb_c aabb;
	aabb.clear();
	
	if(frame == frame_old)
	{
		aabb	= frame->bbox;
	}
	else
	{
		aabb = frame_old->bbox;
		aabb.mergeWith(frame->bbox);
	}
	
	return aabb;
}

void	r_alias_model_c::addModelToList(r_entity_c *ent)
{
	//ri.Com_Printf("r_alias_model_c::addModelToList: model '%s'\n", getName());

	if((ent->getShared().flags & RF_WEAPONMODEL) && (r_lefthand->getInteger() == 2))
		return;

	if(/*(ent->isStatic() && !ent->isVisFramed()) ||*/ cull(ent))
	{
		return;
	}
	else
	{
		ent->setFrameCount();
		c_entities++;
	}
		//TODO
	//else
		//r_entvisframe[e->number][(r_
	
	//ri.Com_Printf("r_alias_model_c::addModelToList: model '%s'\n", getName());
		
	//
	// add meshes to the mesh buffer lists with appropiate skins
	//
	for(unsigned i=0; i<_meshes.size(); i++)
	{
		r_mesh_c*		mesh = _meshes[i];
		r_shader_c*		shader = NULL;
		
		if(ent->getShared().custom_shader != -1)
		{
			shader = R_GetShaderByNum(ent->getShared().custom_shader);
		}
		else if(ent->getShared().custom_skin != -1)
		{
			r_model_skin_c *skin = R_GetSkinByNum(ent->getShared().custom_skin);
			
			shader = skin->getShader(mesh->name);
			
		}
		else if(i >= 0 && i < _shaders.size())
		{
			shader = _shaders[i]->getShader();
		}
		else
		{
			ri.Com_Error(ERR_DROP, "r_alias_model_c::addModelToList: no way to create command");
			continue;
		}
		
		if(!r_showinvisible->getInteger() && shader->hasFlags(SHADER_NODRAW))
			continue;
			
		if(r_envmap && shader->hasFlags(SHADER_NOENVMAP))
			continue;
		
		RB_AddCommand(ent, this, mesh, shader, NULL, NULL, -(i+1), r_origin.distance(ent->getShared().origin));
		
		for(std::vector<r_light_c*>::iterator ir = r_lights.begin(); ir != r_lights.end(); ++ir)
		{
			r_light_c* light = *ir;
			
			if(!light)
				continue;
		
			if(!light->isVisible())
				continue;
			
			if(light->getShared().radius_aabb.intersect(ent->getShared().origin, mesh->bbox.radius()))
				RB_AddCommand(ent, this, mesh, shader, light, NULL, -(i+1), 0);
		}
	}
}


void 	r_alias_model_c::draw(const r_command_t *cmd, r_render_type_e type)
{
	//ri.Com_Printf("r_alias_model_c::draw: model '%s'\n", getName());
	
	if(!cmd)
		return;
		
	if((cmd->getEntity()->getShared().flags & RF_WEAPONMODEL) && (type == RENDER_TYPE_SHADOWING))
		return;
	
	if(cmd->getEntity()->getShared().flags & RF_DEPTHHACK) // hack the depth range to prevent view model from poking into walls
		xglDepthRange(r_depthmin, r_depthmin + 0.3*(r_depthmax-r_depthmin));

	if((cmd->getEntity()->getShared().flags & RF_WEAPONMODEL) && (r_lefthand->getInteger() == 1))
	{
		cmd->getEntity()->setupTransformLeftHanded();
		xglFrontFace(GL_CW);
	}
	else
	{
		cmd->getEntity()->setupTransform();
	}
	
	RB_SetupModelviewMatrix(cmd->getEntity()->getTransform());
	
	drawFrameLerp(cmd, type);
	
	if((cmd->getEntity()->getShared().flags & RF_WEAPONMODEL) && (r_lefthand->getInteger() == 1)) 
		xglFrontFace(GL_CCW);
	
	if(cmd->getEntity()->getShared().flags & RF_DEPTHHACK)
		xglDepthRange(r_depthmin, r_depthmax);
}



void	r_alias_model_c::setupMeshes()
{
	for(unsigned int mesh_num=0; mesh_num<_meshes.size(); mesh_num++)
	{
		r_mesh_c*	mesh = _meshes[mesh_num];
		
		mesh->calcEdges();
				
		for(unsigned int frame_num=0; frame_num<_frames.size(); frame_num++)
		{
			r_alias_frame_t *frame = _frames[frame_num];
			
			r_alias_mesh_frame_t *meshframe = frame->meshframes[mesh_num];
				
			meshframe->calcTangentSpaces(mesh->indexes);			
		}
	}
}


























