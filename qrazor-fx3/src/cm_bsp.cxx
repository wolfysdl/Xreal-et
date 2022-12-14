/// ============================================================================
/*
Copyright (C) 1997-2001 Id Software, Inc.
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

/// includes ===================================================================
// system -------------------------------------------------------------------
#include <boost/crc.hpp>

// qrazor-fx ----------------------------------------------------------------
#include "common.h"
#include "cvar.h"
#include "vfs.h"
#include "files.h"

#include "cm.h"
#include "cm_md3.h"


#define	MAX_CM_AREAS		(MAX_BSP_AREAS)
#define	MAX_CM_LEAFS		(MAX_BSP_LEAFS)
#define	MAX_CM_VISIBILITY	(MAX_BSP_VISIBILITY)

// box tracing
// 1/32 epsilon to keep floating point happy
#define	CLIP_EPSILON	(0.03125)


std::vector<cmodel_c*>		cm_models;
static int			cm_checkcount = 0;

int		cm_pointcontents;
int		cm_traces;
int		cm_brush_traces;
int		cm_mesh_traces;
int		cm_surf_traces;


class cmodel_box_c :
public cmodel_c
{
public:
	cmodel_box_c()
	:cmodel_c("box", NULL, 0)
	{
	}

	int			faces_first;
	int			faces_num;

	int			brushes_first;
	int			brushes_num;
};

struct cnode_t
{
	plane_c*		plane;
	int			children[2];		// negative numbers are leafs
	int			checkcount;
};

struct cleaf_t
{
	int			contents;
	int			cluster;
	int			area;
	
	int			leafsurfaces_first;
	int			leafsurfaces_num;
	
	int			leafbrushes_first;
	int			leafbrushes_num;
	
	int			leafpatches_first;
	int			leafpatches_num;
};

struct cshader_t
{
	cshader_t()
	{
		flags		= X_SURF_NONE;
		contents	= X_CONT_NONE;
	}

	uint_t			flags;
	uint_t			contents;
};

struct cbrushside_t
{
	plane_c*		plane;
	cshader_t*		shader;
//	cshader_t*		shader;
};

struct cmesh_t
{
	std::vector<vec3_c>	vertexes;
		
	std::vector<vec3_c>	normals;
	
	std::vector<index_t>	indexes;
};

struct csurface_t
{
	int			face_type;
	
	int			vertexes_first;
	int			vertexes_num;
	
	int			shader_num;
	
	plane_c			plane;			// BSPST_PLANAR only
	
	int			mesh_cp[2];		// BSPST_BEZIER only
	
	cmesh_t			mesh;
	
	int			checkcount;
};

struct cbrush_t
{
	int			getContents(const vec3_c &p) const;
	
	void			testBox
	(
		const vec3_c &start, const aabb_c &aabb,
		trace_t &trace
	);

	void			clipBox
	(
		const vec3_c &start, const vec3_c &end, const aabb_c &aabb,
		trace_t &trace
	);

	int			contents;

	std::vector<cbrushside_t*>
				sides;	
//	int			sides_first;
//	int			sides_num;
	
	int			checkcount;		// to avoid repeated testings
};

int	cbrush_t::getContents(const vec3_c &p) const
{
	uint_t j;

	for(j=0; j<sides.size(); j++)
	{
		const cbrushside_t* brushside = sides[j];
				
		if(brushside->plane->distance(p) > 0)
			break;
	}
		
	if(j == sides.size())
		return contents;
	else
		return X_CONT_NONE;
}

void	cbrush_t::testBox
(
	const vec3_c &start, const aabb_c &aabb,
	trace_t &trace
)
{
	plane_c	*plane;
	vec3_c		ofs;
	float		d1;
	cbrushside_t	*side;

	if(!sides.size())
		return;

	for(uint_t i=0; i<sides.size(); i++)
	{
		side = sides[i];
		plane = side->plane;

		// general box case

		// push the plane out apropriately for mins/maxs

		// FIXME: use signbits into 8 way lookup for each mins/maxs
		for(uint_t j=0; j<3; j++)
		{
			if(plane->_normal[j] < 0)
				ofs[j] = aabb._maxs[j];
			else
				ofs[j] = aabb._mins[j];
		}
		
		ofs += start;
		
		d1 = plane->distance(ofs);

		// if completely in front of face, no intersection
		if(d1 > 0)
			return;
	}

	// inside this brush
	trace.startsolid = trace.allsolid = true;
	trace.fraction = 0;
	trace.pos_flags = X_SURF_NONE;
	trace.pos_contents = X_CONT_NONE;
	trace.neg_contents = contents;
}

void	cbrush_t::clipBox
(
	const vec3_c &p1, const vec3_c &p2, const aabb_c &aabb,
	trace_t &trace
)
{
	plane_c	*plane, *clipplane;
	float		enterfrac, leavefrac;
	vec3_c		ofs, ofs_ext;
	float		d1, d2;
	bool		getout, startout;
	float		f;
	cbrushside_t	*side, *leadside;

	enterfrac = -1;
	leavefrac = 1;
	clipplane = NULL;

	if(!sides.size())
		return;

//	cm_brush_traces++;

	getout = false;
	startout = false;
	leadside = NULL;

	for(uint_t i=0; i<sides.size(); i++)
	{
		side = sides[i];
		plane = side->plane;

		// FIXME: special case for axial

		if(!aabb.isZero())
		{	// general box case

			// push the plane out apropriately for mins/maxs

			// FIXME: use signbits into 8 way lookup for each mins/maxs
			for(int j=0; j<3; j++)
			{
				if(plane->_normal[j] < 0)
					ofs[j] = aabb._maxs[j];
				else
					ofs[j] = aabb._mins[j];
			}
					
			ofs_ext = ofs + p1; 
			d1 = plane->distance(ofs_ext);
			
			ofs_ext = ofs + p2;
			d2 = plane->distance(ofs_ext);	
		}
		else
		{	// special point case
			d1 = plane->distance(p1);
			d2 = plane->distance(p2);
		}
		
		
		if(d2 > 0)
			getout = true;	// endpoint is not in solid
		
		if(d1 > 0)
			startout = true;

		// if completely in front of face, no intersection
		if(d1 > 0 && d2 >= d1)
			return;

		if(d1 <= 0 && d2 <= 0)
			continue;

		// crosses face
		if(d1 > d2)
		{	
			// enter
			f = (d1-CLIP_EPSILON) / (d1-d2);
			
			if(f > enterfrac)
			{
				enterfrac = f;
				clipplane = plane;
				leadside = side;
			}
		}
		else
		{	
			// leave
			f = (d1+CLIP_EPSILON) / (d1-d2);
			
			if(f < leavefrac)
				leavefrac = f;
		}
	}

	if(!startout)
	{	
		// original point was inside brush
		trace.startsolid = true;
		
		if(!getout)
			trace.allsolid = true;
		
		return;
	}
	
	if(enterfrac < leavefrac)
	{
		if(enterfrac > -1 && enterfrac < trace.fraction)
		{
			if(enterfrac < 0)
				enterfrac = 0;
			
			trace.fraction = enterfrac;
			trace.plane = *clipplane;
			//trace->surface = leadside->shader;
			trace.pos_flags = leadside->shader->flags;
			trace.pos_contents = leadside->shader->contents;
			trace.neg_contents = contents;
		}
	}
}

struct cpatch_t
{
	aabb_c			bbox_abs;
	
	int			brushes_num;
	cbrush_t*		brushes;
	
	cshader_t*		shader;
	
	int			checkcount;
};

/*
struct csurface_t
{
	int			shader_num;
	
	cmesh_t			mesh;
};
*/

struct carea_t
{
	carea_t()
	{
		memset(numareaportals, 0, sizeof(numareaportals));
	}
	
	int	numareaportals[MAX_CM_AREAS];
};


class cmodel_bsp_c :
public cmodel_c
{
public:
	cmodel_bsp_c(const std::string &name, byte *buffer, uint_t buffer_size, const bsp_dheader_t &header);
	
	virtual void	load()				{}
	virtual int	leafContents(int leafnum) const;
	virtual int	leafCluster(int leafnum) const;
	virtual int	leafArea(int leafnum) const;

	virtual const char*	entityString() const;

private:
	void		pointLeafnum_r(const vec3_c &p, int nodenum, int &leafnum) const;

public:
	// returns an ORed contents mask
	virtual int	pointContents(const vec3_c &p) const;
	virtual int	pointContents(const vec3_c &p, const vec3_c &origin, const quaternion_c &quat) const;

	virtual int	pointLeafnum(const vec3_c &p) const;
	virtual int	pointAreanum(const vec3_c &p) const;

private:
	void		testInLeaf
	(
		int leafnum, 
		const vec3_c &start, const vec3_c &end, const aabb_c &aabb,
		int mask, 
		trace_t &trace
	);

	void		traceToLeaf
	(
		int leafnum,
		const vec3_c &start, const vec3_c &end, const aabb_c &aabb,
		int mask,
		trace_t &trace
	);

	void		hullCheck_r
	(
		int nodenum,
		float p1f, float p2f,
		const vec3_c &p1, const vec3_c &p2, const vec3_c &extents,
		const vec3_c &start, const vec3_c &end, const aabb_c &aabb,
		int mask,
		trace_t &trace
	);
	
public:
	virtual trace_t	traceAABB
	(
		const vec3_c &start, const vec3_c &end, const aabb_c &aabb, 
		int mask
	);

	virtual trace_t	traceOBB
	(
		const vec3_c &start, const vec3_c &end, const aabb_c &aabb, 
		int mask,
		const vec3_c &origin, const quaternion_c &quat
	);

private:
	void		boxLeafnums_r(int nodenum, const aabb_c &aabb, std::vector<int> &list, int &topnode, int max) const;

public:
	// returns topnode
	virtual int	boxLeafnums(const aabb_c &bbox, std::vector<int> &list, int max) const;

	// returns NULL if bad cluster
	virtual const byte*	clusterPVS(int cluster)	const;

	virtual int	getClosestAreaPortal(const vec3_c &p) const		{return -1;}
	virtual bool	getAreaPortalState(int portal) const			{return true;}
	virtual void	setAreaPortalState(int portal, bool open) const		{}
	virtual bool	areasConnected(int area1, int area2) const		{return true;}
	
private:

	void		loadShaders(const bsp_lump_t &l);
	void		loadPlanes(const bsp_lump_t &l);
	void		loadLeafBrushes(const bsp_lump_t &l);
	void		loadBrushes(const bsp_lump_t &l);
	void		loadBrushSides(const bsp_lump_t &l);
	void		loadVertexes(const bsp_lump_t &l);
	void		loadNormals(const bsp_lump_t &l);
	void		loadIndexes(const bsp_lump_t &l);
	
	void		createMesh(const bsp_dsurface_t *in, cmesh_t &mesh);
	void		createBezierMesh(const bsp_dsurface_t *in, cmesh_t &mesh);
	void		loadSurfaces(const bsp_lump_t &l);
	
	void		loadLeafSurfaces(const bsp_lump_t &l);
	void		loadLeafs(const bsp_lump_t &l);
	void		loadNodes(const bsp_lump_t &l);
	void		loadModels(const bsp_lump_t &l);
	void		loadVisibility(const bsp_lump_t &l);
	void		loadEntityString(const bsp_lump_t &l);


	std::vector<cshader_t>		_shaders;
	std::vector<plane_c>		_planes;

	std::vector<cbrushside_t>	_brushsides;

	std::vector<cnode_t>		_nodes;

	std::vector<cleaf_t>		_leafs;

	std::vector<int>		_leafsurfaces;
	std::vector<int>		_leafbrushes;

	std::vector<cbrush_t>		_brushes;

	std::vector<byte>		_pvs;
	int				_pvs_clusters_num;
	int				_pvs_clusters_size;

	std::vector<carea_t>		_areas;

	std::vector<vec3_c>		_vertexes;
	std::vector<vec3_c>		_normals;
	std::vector<index_t>		_indexes;

	std::vector<csurface_t>		_surfaces;

	std::string			_entitystring;
};

cmodel_bsp_c::cmodel_bsp_c(const std::string &name, byte *buffer, uint_t buffer_size, const bsp_dheader_t &header)
:cmodel_c(name, buffer, buffer_size)
{
	loadShaders(header.lumps[BSP_LUMP_SHADERS]);//, bsp);
	loadPlanes(header.lumps[BSP_LUMP_PLANES]);//, bsp);
	loadLeafBrushes(header.lumps[BSP_LUMP_LEAFBRUSHES]);//, bsp);
	loadBrushSides(header.lumps[BSP_LUMP_BRUSHSIDES]);//, bsp);
	loadBrushes(header.lumps[BSP_LUMP_BRUSHES]);//, bsp);
	loadVertexes(header.lumps[BSP_LUMP_VERTEXES]);
	loadNormals(header.lumps[BSP_LUMP_VERTEXES]);
	loadIndexes(header.lumps[BSP_LUMP_INDEXES]);
	loadSurfaces(header.lumps[BSP_LUMP_SURFACES]);//, bsp);
	loadLeafSurfaces(header.lumps[BSP_LUMP_LEAFSURFACES]);//, bsp);
	loadLeafs(header.lumps[BSP_LUMP_LEAFS]);//, bsp);
	loadNodes(header.lumps[BSP_LUMP_NODES]);//, bsp);
	loadModels(header.lumps[BSP_LUMP_MODELS]);//, bsp);
	loadVisibility(header.lumps[BSP_LUMP_VISIBILITY]);
	loadEntityString(header.lumps[BSP_LUMP_ENTITIES]);

//	floodAreaConnections();
}

int	cmodel_bsp_c::leafContents(int leafnum) const
{
	if(leafnum < 0 || leafnum >= (int)_leafs.size())
		Com_Error(ERR_DROP, "cmodel_bsp_c::leafContents: out of range %i", leafnum);
	
	return _leafs[leafnum].contents;
}

int	cmodel_bsp_c::leafCluster(int leafnum) const
{
	if(leafnum < 0 || leafnum >= (int)_leafs.size())
		Com_Error(ERR_DROP, "cmodel_bsp_c::leafCluster: out of range %i", leafnum);
	
	return _leafs[leafnum].cluster;
}

int	cmodel_bsp_c::leafArea(int leafnum) const
{
	if(leafnum < 0 || leafnum >= (int)_leafs.size())
		Com_Error(ERR_DROP, "cmodel_bsp_c::leafArea: out of range %i", leafnum);
	
	return _leafs[leafnum].area;
}

const char*	cmodel_bsp_c::entityString() const
{
	return _entitystring.c_str();
}


void	cmodel_bsp_c::pointLeafnum_r(const vec3_c &p, int nodenum, int &leafnum) const
{
//	cm_pointcontents++;		// optimize counter
	
	if(nodenum < 0)
	{
		leafnum = -1 -nodenum;
		
		if(leafnum < 0 || leafnum >= (int)_leafs.size())
		{
			Com_Error(ERR_DROP, "cmodel_bsp_c::pointLeafnum_r: bad leafnum %i", leafnum);
		}
		return;
	}

	try
	{
		const cnode_t& node = _nodes.at(nodenum);

		if(!node.plane)
		{
			Com_Error(ERR_DROP, "cmodel_bsp_c::pointLeafnum_r: bad node %i", nodenum);
		}

		plane_side_e s = node.plane->onSide(p);

		switch(s)
		{
			case SIDE_FRONT:
			{
				pointLeafnum_r(p, node.children[SIDE_FRONT], leafnum);
				break;
			}

			case SIDE_BACK:
			{
				pointLeafnum_r(p, node.children[SIDE_BACK], leafnum);
				break;
			}

			default:
				break;
		}
	}
	catch(...)
	{
		Com_Error(ERR_DROP, "cmodel_bsp_c::pointLeafnum_r: exception thrown");
	}
}


int	cmodel_bsp_c::pointContents(const vec3_c &p) const
{
	int	contents = X_CONT_NONE;
	
	try
	{
		const cleaf_t& leaf = _leafs.at(pointLeafnum(p));
	
		for(int i=0; i<leaf.leafbrushes_num; i++)
		{
			const cbrush_t& brush = _brushes[_leafbrushes[leaf.leafbrushes_first + i]];
		
			// check if brush actually adds something to contents
			if((contents & brush.contents) == brush.contents )
				continue;

			contents |= brush.getContents(p);
		}
	}
	catch(...)
	{
		Com_Error(ERR_DROP, "CM_PointContents: exception thrown");
	}
	
	return contents;
}

/*
==================
CM_TransformedPointContents

Handles offseting and rotation of the end points for moving and
rotating entities
==================
*/
int	cmodel_bsp_c::pointContents(const vec3_c &p, const vec3_c &origin, const quaternion_c &quat) const
{
	// subtract origin offset
	vec3_c p_l = p - origin;

	// rotate start and end into the models frame of reference
	if(quat != quat_identity)
	{
		p_l.rotate(quat);
	}
	
	return pointContents(p_l);
}

int	cmodel_bsp_c::pointLeafnum(const vec3_c &p) const
{
	int leafnum = -1;
	pointLeafnum_r(p, 0, leafnum);
	return leafnum;
}

int	cmodel_bsp_c::pointAreanum(const vec3_c &p) const
{
	int leafnum = pointLeafnum(p);
	int areanum = leafArea(leafnum);
	
	return areanum;
}


void	cmodel_bsp_c::testInLeaf
(
	int leafnum, 
	const vec3_c &start, const vec3_c &end, const aabb_c &aabb,
	int mask, 
	trace_t &trace
)
{
	const cleaf_t& leaf = _leafs[leafnum];
	
	if(!(leaf.contents & mask))
		return;
	
//	if(cm_use_brushes->getInteger())
	{
		// trace line against all brushes in the leaf
		for(int i=0; i<leaf.leafbrushes_num; i++)
		{
			cbrush_t& brush = _brushes[_leafbrushes[leaf.leafbrushes_first + i]];
		
			if(brush.checkcount == cm_checkcount)
				continue;	// already checked this brush in another leaf
		
			brush.checkcount = cm_checkcount;

			if(!(brush.contents & mask))
				continue;
		
			brush.testBox(start, aabb, trace);
		
			if(!trace.fraction)
				return;
		}
	}
	
	/*
	if(cm_use_patches->integer)
	{
		// trace line against all patches in the leaf
		for(int i=0; i<leaf->leafpatches_num; i++)
		{
			cpatch_t *patch = &cm_patches[cm_leafpatches[leaf->leafpatches_first + i]];
		
			if(patch->checkcount == cm_checkcount)
				continue;	// already checked this patch in another leaf
		
			patch->checkcount = cm_checkcount;

			if(!(patch->shader->contents & trace_contents))
				continue;
		
			if(!patch->bbox_abs.intersect(trace_bbox_abs))
				continue;
		
			for(int j=0; j<patch->brushes_num; j++)
			{
				CM_TestBoxInBrush(trace_bbox, trace_start, &trace_trace, &patch->brushes[j]);
			
				if(!trace_trace.fraction)
					return;
			}
		}
	}
	
	if(cm_use_meshes->integer)
	{
		// trace line against all surfaces in the leaf
		for(int i=0; i<leaf->leafsurfaces_num; i++)
		{
			csurface_t *surf = &cm_surfaces[cm_leafsurfaces[leaf->leafsurfaces_first + i]];
		
			if(surf->checkcount == cm_checkcount)
				continue;	// already checked this brush in another leaf
			
			surf->checkcount = cm_checkcount;
		
			cshader_t *shader = &cm_shaders[surf->shader_num];
		
			if(!(shader->contents & trace_contents))
				continue;
		
			CM_TestBoxInMesh(trace_bbox, trace_start, &trace_trace, surf->mesh, shader);
		
			if(!trace_trace.fraction)
				return;
		}
	}
	*/
}

void	cmodel_bsp_c::traceToLeaf
(
	int leafnum, 
	const vec3_c &start, const vec3_c &end, const aabb_c &aabb,
	int mask, 
	trace_t &trace
)
{
	const cleaf_t& leaf = _leafs[leafnum];
	
	if(!(leaf.contents & mask))
		return;
	
//	if(cm_use_brushes->getInteger())
	{
		// trace line against all brushes in the leaf
		for(int i=0; i<leaf.leafbrushes_num; i++)
		{
			cbrush_t& brush = _brushes[_leafbrushes[leaf.leafbrushes_first + i]];
		
			if(brush.checkcount == cm_checkcount)
				continue;	// already checked this brush in another leaf
		
			brush.checkcount = cm_checkcount;

			if(!(brush.contents & mask))
				continue;
		
			brush.clipBox(start, end, aabb, trace);
		
			if(!trace.fraction)
				return;
		}
	}
	
	/*
	if(cm_use_patches->integer)
	{
		// trace line against all patches in the leaf
		for(int i=0; i<leaf->leafpatches_num; i++)
		{
			cpatch_t *patch = &cm_patches[cm_leafpatches[leaf->leafpatches_first + i]];
		
			if(patch->checkcount == cm_checkcount)
				continue;	// already checked this patch in another leaf
			
			patch->checkcount = cm_checkcount;

			if(!(patch->shader->contents & trace_contents))
				continue;
		
			if(!patch->bbox_abs.intersect(trace_bbox_abs))
				continue;
		
			for(int j=0; j<patch->brushes_num; j++)
			{
				CM_ClipBoxToPatch(trace_bbox, trace_start, trace_end, &trace_trace, &patch->brushes[j]);
			
				if(!trace_trace.fraction)
					return;
			}
		}
	}
	
	if(cm_use_meshes->integer)
	{
		// trace line against all surfaces in the leaf
		for(int i=0; i<leaf->leafsurfaces_num; i++)
		{
			csurface_t *surf = &cm_surfaces[cm_leafsurfaces[leaf->leafsurfaces_first + i]];
		
			if(surf->checkcount == cm_checkcount)
				continue;	// already checked this brush in another leaf
		
			surf->checkcount = cm_checkcount;
		
			cshader_t *shader = &cm_shaders[surf->shader_num];
		
			if(!(shader->contents & trace_contents))
				continue;
		
			CM_ClipBoxToMesh(trace_bbox, trace_start, trace_end, &trace_trace, surf->mesh, shader);
		
			if(!trace_trace.fraction)
				return;
		}
	}
	*/
}

void	cmodel_bsp_c::hullCheck_r
(
	int nodenum, 
	float p1f, float p2f, 
	const vec3_c &p1, const vec3_c &p2, const vec3_c &extents,
	const vec3_c &start, const vec3_c &end, const aabb_c &aabb,
	int mask,
	trace_t &trace
)
{
	cnode_t		*node;
	plane_c	*plane;
	float		t1, t2, offset;
	float		frac, frac2;
	float		idist;
	int		i;
	vec3_t		mid;
	int		side;
	float		midf;

	if(trace.fraction <= p1f)
		return;		// already hit something nearer

	// if < 0, we are in a leaf node
	if(nodenum < 0)
	{
		traceToLeaf(-1 - nodenum, start, end, aabb, mask, trace);
		return;
	}

	//
	// find the point distances to the seperating plane
	// and the offset for the size of the box
	//
	node = &_nodes[nodenum];
	plane = node->plane;

	if(plane->getType() < 3)
	{
		t1 = p1[plane->getType()] - plane->_dist;
		t2 = p2[plane->getType()] - plane->_dist;
		
		offset = extents[plane->getType()];
	}
	else
	{
		t1 = plane->_normal.dotProduct(p1) - plane->_dist;
		t2 = plane->_normal.dotProduct(p2) - plane->_dist;
		
		if(aabb.isZero())
			offset = 0;
		else
		{
			offset = 	fabs(extents[0]*plane->_normal[0]) +
					fabs(extents[1]*plane->_normal[1]) +
					fabs(extents[2]*plane->_normal[2]);
		}
	}


	// see which sides we need to consider
	if(t1 >= offset && t2 >= offset)
	{
		hullCheck_r
		(
			node->children[SIDE_FRONT], 
			p1f, p2f, p1, p2, extents,
			start, end, aabb,
			mask,
			trace
		);
		return;
	}
	
	if(t1 < -offset && t2 < -offset)
	{
		hullCheck_r
		(
			node->children[SIDE_BACK], 
			p1f, p2f, p1, p2, extents,
			start, end, aabb,
			mask,
			trace
		);
		return;
	}

	// put the crosspoint CLIP_EPSILON pixels on the near side
	if(t1 < t2)
	{
		idist = 1.0/(t1-t2);
		side = 1;
		frac2 = (t1 + offset + CLIP_EPSILON)*idist;
		frac  = (t1 - offset + CLIP_EPSILON)*idist;
	}
	else if(t1 > t2)
	{
		idist = 1.0/(t1-t2);
		side = 0;
		frac2 = (t1 - offset - CLIP_EPSILON)*idist;
		frac  = (t1 + offset + CLIP_EPSILON)*idist;
	}
	else
	{
		side = 0;
		frac = 1;
		frac2 = 0;
	}

	// move up to the node
	X_clamp(frac, 0, 1);
		
	midf = p1f + (p2f - p1f)*frac;
	
	for(i=0; i<3; i++)
		mid[i] = p1[i] + frac*(p2[i] - p1[i]);

//	hullCheck_r(node->children[side], p1f, midf, p1, mid);
	hullCheck_r
	(
		node->children[side], 
		p1f, midf, p1, mid, extents,
		start, end, aabb,
		mask,
		trace
	);


	// go past the node
	X_clamp(frac2, 0, 1);
		
	midf = p1f + (p2f - p1f)*frac2;
	
	for(i=0; i<3; i++)
		mid[i] = p1[i] + frac2*(p2[i] - p1[i]);

//	hullCheck_r(node->children[side^1], midf, p2f, mid, p2);
	hullCheck_r
	(
		node->children[side^1], 
		midf, p2f, mid, p2, extents,
		start, end, aabb,
		mask,
		trace
	);
}


trace_t	cmodel_bsp_c::traceAABB
(
	const vec3_c &start, const vec3_c &end, const aabb_c &aabb,
	int mask
)
{
	cm_checkcount++;		// for multi-check avoidance
	cm_traces++;			// for statistics, may be zeroed

	// fill in a default trace
	trace_t trace;
	trace.fraction = 1.0;

	
	// build a bounding box of the entire move
	/*
	Tr3B - move this to class cmodel_inline_c::trace() ...

	vec3_c	p;
	aabb_c	aabb_w;
	aabb_w.clear();
	
	p = start + aabb._mins;
	aabb_w.addPoint(p);
	
	p = start + aabb._maxs;
	aabb_w.addPoint(p);
	
	p = end + aabb._mins;
	aabb_w.addPoint(p);
	
	p = end + aabb._maxs;
	aabb_w.addPoint(p);
	*/
	
	// check for position test special case
	if(start == end)
	{
		std::vector<int> leafs;

		aabb_c	c;
		c._mins = start + aabb._mins - vec3_c(1.0, 1.0, 1.0);
		c._maxs = start + aabb._maxs + vec3_c(1.0, 1.0, 1.0);

		boxLeafnums(c, leafs, 1024);
		
		for(std::vector<int>::const_iterator ir = leafs.begin(); ir != leafs.end(); ++ir)
		{
			testInLeaf(*ir, start, end, aabb, mask, trace);
			
			if(trace.allsolid)
				break;
		}
		
		trace.pos = start;
		
		return trace;
	}

	// check for point special case
	vec3_c extents;
	if(!aabb.isZero())
	{
		extents[0] = -aabb._mins[0] > aabb._maxs[0] ? -aabb._mins[0] : aabb._maxs[0];
		extents[1] = -aabb._mins[1] > aabb._maxs[1] ? -aabb._mins[1] : aabb._maxs[1];
		extents[2] = -aabb._mins[2] > aabb._maxs[2] ? -aabb._mins[2] : aabb._maxs[2];
	}

	// general sweeping through world
	hullCheck_r
	(
		0,
		0, 1, start, end, extents,
		start, end, aabb,
		mask,
		trace
	);

	// set final position
	if(trace.fraction == 1.0)
	{
		trace.pos = end;
	}
	else
	{
		trace.pos = start + ((end - start) * trace.fraction);
	}
	
	return trace;
}


/*
==================
CM_TransformedBoxTrace

Handles offseting and rotation of the end points for moving and
rotating entities
==================
*/
trace_t	cmodel_bsp_c::traceOBB
(
	const vec3_c &start, const vec3_c &end, const aabb_c &aabb,
	int mask, 
	const vec3_c &origin, const quaternion_c &quat
)
{
	// subtract origin offset
	vec3_c start_l = start - origin;
	vec3_c end_l = end - origin;

	// rotate start and end into the models frame of reference
	if((quat != quat_identity))
	{
		start_l.rotate(quat);
		end_l.rotate(quat);
	}

	// sweep the box through the model
	trace_t trace = traceAABB(start_l, end_l, aabb, mask);

	if((quat != quat_identity) && trace.fraction != 1.0)
	{
		// FIXME: figure out how to do this with existing angles
		//a = angles;
		//a.negate();
		//trace.plane.rotate(a);
		quaternion_c q = quat;
		q.inverse();
		trace.plane.rotate(q);
	}
	
	trace.pos = start + ((end - start) * trace.fraction);
	
	return trace;
}


/*
=============
CM_BoxLeafnums

Fills in a list of all the leafs touched
=============
*/
void	cmodel_bsp_c::boxLeafnums_r(int nodenum, const aabb_c &aabb, std::vector<int> &list, int &topnode, int max) const
{
	if(nodenum < 0)
	{
		int leafnum = -1 -nodenum;
		
		if(leafnum < 0 || leafnum >= (int)_leafs.size())
		{
			Com_Error(ERR_DROP, "cmodel_bsp_c::boxLeafnums_r: bad leafnum %i", leafnum);
		}

		if((int)list.size() < max)
			list.push_back(leafnum);
		return;
	}

	try
	{
		const cnode_t& node = _nodes.at(nodenum);

		plane_side_e s = node.plane->onSide(aabb);

		switch(s)
		{
			case SIDE_FRONT:
			{
				boxLeafnums_r(node.children[0], aabb, list, topnode, max);
				break;
			}

			case SIDE_BACK:
			{
				boxLeafnums_r(node.children[1], aabb, list, topnode, max);
				break;
			}

			case SIDE_CROSS:
			{
				// go down both
				if(topnode == -1)
					topnode = nodenum;
			
				boxLeafnums_r(node.children[0], aabb, list, topnode, max);
				boxLeafnums_r(node.children[1], aabb, list, topnode, max);
				break;
			}

			default:
				break;
		}
	}
	catch(...)
	{
		Com_Error(ERR_DROP, "cmodel_bsp_c::boxLeafnums_r: exception thrown");
	}
}

int	cmodel_bsp_c::boxLeafnums(const aabb_c &aabb, std::vector<int> &list, int max) const
{
//	if(!list.empty())
	list.clear();

	int topnode = -1;

	boxLeafnums_r(0, aabb, list, topnode, max);

	return topnode;
}

const byte*	cmodel_bsp_c::clusterPVS(int cluster) const
{
	if(cluster < 0 || cluster >= _pvs_clusters_num || _pvs.empty())
		return NULL;
	
	try
	{
		const byte *data =  &(_pvs.at(cluster * _pvs_clusters_size));
		return data;
	}
	catch(...)
	{
		Com_Error(ERR_DROP, "cmodel_bsp_c::clusterPVS: exception occured");
	}
	
	return NULL;
}


void	cmodel_bsp_c::loadShaders(const bsp_lump_t &l)//, d_bsp_c *bsp)
{
	Com_DPrintf("loading shaders ...\n");

	bsp_dshader_t* in = (bsp_dshader_t*)(_buffer + l.fileofs);
	if(l.filelen % sizeof(*in))
		Com_Error(ERR_DROP, "cmodel_bsp_c::loadShaders: funny lump size");
	int count = l.filelen / sizeof(*in);
	
	if(count < 1)
		Com_Error(ERR_DROP, "cmodel_bsp_c::loadShaders: BSP with no shaders");

	_shaders = std::vector<cshader_t>(count);

	for(int i=0; i<count; i++, in++)
	{
		uint_t flags	= LittleLong(in->flags);
		uint_t contents = LittleLong(in->contents);
	
		_shaders[i].flags = flags;
		_shaders[i].contents = contents;
		
		//if(bsp)
		//	bsp->addShader(flags, contents);
	}
}

void	cmodel_bsp_c::loadPlanes(const bsp_lump_t &l)//, d_bsp_c* bsp)
{
	vec3_c	normal;
	float	dist;	
	
	Com_DPrintf("loading planes ...\n");
	
	bsp_dplane_t* in = (bsp_dplane_t*)(_buffer + l.fileofs);
	if(l.filelen % sizeof(*in))
		Com_Error(ERR_DROP, "cmodel_bsp_c::loadPlanes: funny lump size");
	int count = l.filelen / sizeof(*in);

	if(count < 1)
		Com_Error(ERR_DROP, "cmodel_bsp_c::loadPlanes: BSP with no planes");
	
	// need to save space for box planes
//	if(count > MAX_BSP_PLANES)
//		Com_Error (ERR_DROP, "cmodel_bsp_c::loadPlanes: BSP has too many planes");

	_planes = std::vector<plane_c>(count);

	for(int i=0; i<count; i++, in++)
	{		
		for(int j=0; j<3; j++)
		{
			normal[j] = LittleFloat(in->normal[j]);
		}
		
		dist = LittleFloat(in->dist);
		
		_planes[i].set(normal, dist);
		
		//if(bsp)
		//	bsp->addPlane(cm_planes[i]);
	}
}

void	cmodel_bsp_c::loadLeafBrushes(const bsp_lump_t &l)//, d_bsp_c* bsp)
{
	Com_DPrintf("loading leafbrushes ...\n");
	
	int *in = (int*)(_buffer + l.fileofs);
	if(l.filelen % sizeof(*in))
		Com_Error(ERR_DROP, "cmodel_bsp_c::loadLeafBrushes: funny lump size");
	int count = l.filelen / sizeof(*in);

	if(count < 1)
		Com_Error(ERR_DROP, "cmodel_bsp_c::loadLeafBrushes: BSP with no leafbrushes");
	
	_leafbrushes = std::vector<int>(count);
	
	for(int i=0; i<count; i++, in++)
	{
		int& out = _leafbrushes[i];
		
		out = LittleLong(*in);
		
		//if(bsp)
		//	bsp->addLeafBrush(out);
	}
}

void	cmodel_bsp_c::loadBrushSides(const bsp_lump_t &l)//, d_bsp_c* bsp)
{
	Com_DPrintf("loading brushsides ...\n");
	
	bsp_dbrushside_t *in = (bsp_dbrushside_t*)(_buffer + l.fileofs);
	if(l.filelen % sizeof(*in))
		Com_Error(ERR_DROP, "cmodel_bsp_c::loadBrushSides: funny lump size");
	int count = l.filelen / sizeof(*in);

	_brushsides = std::vector<cbrushside_t>(count);

	for(int i=0; i<count; i++, in++)
	{
		cbrushside_t& out = _brushsides[i];
	
		out.plane = &_planes[LittleLong(in->plane_num)];
		
		int shader_num = LittleLong(in->shader_num);
		
		if(shader_num < 0 || shader_num >= (int)_shaders.size())
			Com_Error(ERR_DROP, "cmodel_bsp_c::loadBrushSides: Bad brushside shadernum %i", shader_num);
		
		out.shader = &_shaders[shader_num];
		
		//if(bsp)
		//	bsp->addBrushSide(LittleLong(in->plane_num));
	}
}

void	cmodel_bsp_c::loadBrushes(const bsp_lump_t &l)//, d_bsp_c *bsp)
{
	Com_DPrintf("loading brushes ...\n");
	
	bsp_dbrush_t *in = (bsp_dbrush_t*)(_buffer + l.fileofs);
	if(l.filelen % sizeof(*in))
		Com_Error(ERR_DROP, "cmodel_bsp_c::loadBrushes: funny lump size");
	int count = l.filelen / sizeof(*in);

	_brushes = std::vector<cbrush_t>(count);

	for(int i=0; i<count; i++, in++)
	{
		cbrush_t &out = _brushes[i];
		
		int shader_num = LittleLong(in->shader_num);
		
		out.contents = _shaders[shader_num].contents;
		
		int sides_first = LittleLong(in->sides_first);
		int sides_num = LittleLong(in->sides_num);

		for(int j=0; j<sides_num; j++)
		{
			out.sides.push_back(&_brushsides[sides_first + j]);
		}
		
		//if(bsp)
		//	bsp->addBrush(out.sides_first, out.sides_num);
	}
}

void	cmodel_bsp_c::loadVertexes(const bsp_lump_t &l)
{
	Com_DPrintf("loading vertexes ...\n");
	
	bsp_dvertex_t* in = (bsp_dvertex_t*)(_buffer + l.fileofs);
	if(l.filelen % sizeof(*in))
		Com_Error(ERR_DROP, "cmodel_bsp_c::loadVertexes: funny lump size");
	int count = l.filelen / sizeof(*in);

	if(count < 1)
		Com_Error(ERR_DROP, "cmodel_bsp_c::loadVertexes: BSP with no vertexes");
			
	_vertexes = std::vector<vec3_c>(count);
	
	for(int i=0; i<count; i++, in++)
	{
		for(int j=0; j<3; j++)
			_vertexes[i][j] = LittleFloat(in->position[j]);
	}
}

void	cmodel_bsp_c::loadNormals(const bsp_lump_t &l)
{	
	Com_DPrintf("loading normals ...\n");
	
	bsp_dvertex_t* in = (bsp_dvertex_t*)(_buffer + l.fileofs);
	if(l.filelen % sizeof(*in))
		Com_Error(ERR_DROP, "cmodel_bsp_c::loadNormals: funny lump size");
	int count = l.filelen / sizeof(*in);

	if(count < 1)
		Com_Error(ERR_DROP, "cmodel_bsp_c::loadNormals: BSP with no vertexes");
			
	_normals = std::vector<vec3_c>(count);
	
	for(int i=0; i<count; i++, in++)
	{
		for(int j=0; j<3; j++)
			_normals[i][j] = LittleFloat(in->normal[j]);
		
		_normals[i].normalize();
	}
}

void	cmodel_bsp_c::loadIndexes(const bsp_lump_t &l)
{
	Com_DPrintf("loading indexes ...\n");
	
	int* in = (int*)(_buffer + l.fileofs);
	if(l.filelen % sizeof(*in))
		Com_Error(ERR_DROP, "cmodel_bsp_c::loadIndexes: funny lump size");
	int count = l.filelen / sizeof(*in);

	if(count < 1)
		Com_Error(ERR_DROP, "cmodel_bsp_c::loadIndexes: BSP with no indexes");
			
	_indexes = std::vector<index_t>(count);
	
	for(int i=0; i<count; i++)
	{
		_indexes[i] = LittleLong(in[i]);
	}
}

void	cmodel_bsp_c::createMesh(const bsp_dsurface_t *in, cmesh_t &mesh)
{
	int vertexes_first = LittleLong(in->vertexes_first);
	int vertexes_num = LittleLong(in->vertexes_num);
				
	mesh.vertexes = std::vector<vec3_c>(vertexes_num);
	mesh.normals = std::vector<vec3_c>(vertexes_num);
			
	for(int j=0; j<vertexes_num; j++)
	{
		mesh.vertexes[j] = _vertexes[vertexes_first + j];
		mesh.normals[j] = _normals[vertexes_first + j];
	}
				
	int indexes_num = LittleLong(in->indexes_num);
	int indexes_first = LittleLong(in->indexes_first);
				
	mesh.indexes = std::vector<index_t>(indexes_num);
	for(int j=0; j<indexes_num; j++)
	{
		mesh.indexes[j] = _indexes[indexes_first + j];
	}
}

void	cmodel_bsp_c::createBezierMesh(const bsp_dsurface_t *in, cmesh_t &mesh)
{
	int		step[2];
	int 		size[2];
	int		flat[2];
	int		mesh_cp[2];
	int		i, p, u, v;
	
	int		vertexes_first;
	int		vertexes_num;
	
	mesh_cp[0] = LittleLong(in->mesh_cp[0]);
	mesh_cp[1] = LittleLong(in->mesh_cp[1]);
	
	if(!mesh_cp[0] || !mesh_cp[1])
		return;
	
	vertexes_first = LittleLong(in->vertexes_first);
	vertexes_num = LittleLong(in->vertexes_num);
	
	std::vector<vec4_c>	vertexes(vertexes_num);
	
	for(i=0; i<vertexes_num; i++)
	{
		_vertexes[vertexes_first +i].copyTo(vertexes[i]);
	}
	
	// find degree of subdivision
	Curve_GetFlatness(1, &(vertexes[0]), mesh_cp, flat);
	
	// allocate space for mesh
	step[0] = (1 << flat[0]);		//step u
	step[1] = (1 << flat[1]);		//step v
	
	size[0] = (mesh_cp[0] / 2) * step[0] + 1;
	size[1] = (mesh_cp[1] / 2) * step[1] + 1;
	vertexes_num = size[0] * size[1];

	mesh.vertexes = std::vector<vec3_c>(vertexes_num);
	
	// allocate and fill index table
	int indexes_num = (size[0]-1) * (size[1]-1) * 6;
	mesh.indexes = std::vector<index_t>(indexes_num);
	
	for(v=0, i=0; v<size[1]-1; v++)
	{
		for(u=0; u<size[0]-1; u++, i+=6)
		{	
			mesh.indexes[i+0] = p = v * size[0] + u;
			mesh.indexes[i+1] = p + size[0];
			mesh.indexes[i+2] = p + 1;
			mesh.indexes[i+3] = p + 1;
			mesh.indexes[i+4] = p + size[0];
			mesh.indexes[i+5] = p + size[0] + 1;
		}
	}
	
	std::vector<vec4_c>	vertexes2(vertexes_num);
		
	// fill in
	Curve_EvalQuadricBezierPatch(&(vertexes[0]), mesh_cp, step, &(vertexes2[0]));
	
	for(i=0; i<(int)mesh.vertexes.size(); i++)
	{
		mesh.vertexes[i] = vertexes2[i];
	}
}

void	cmodel_bsp_c::loadSurfaces(const bsp_lump_t &l)//, d_bsp_c *bsp)
{
	Com_DPrintf("loading surfaces ...\n");
	
	bsp_dsurface_t* in = (bsp_dsurface_t*)(_buffer + l.fileofs);
	if(l.filelen % sizeof(*in))
		Com_Error(ERR_DROP, "cmodel_bsp_c::loadSurfaces: funny lump size");
	int count = l.filelen / sizeof(*in);

	if(count < 1)
		Com_Error(ERR_DROP, "cmodel_bsp_c::loadSurfaces: BSP with no surfaces");

	_surfaces = std::vector<csurface_t>(count);
	
	for(int i=0; i<count; i++, in++)
	{
		csurface_t & out = _surfaces[i];
		
		out.face_type = (bsp_surface_type_t)LittleLong(in->face_type);
		
		out.shader_num = LittleLong(in->shader_num);
		
		switch(out.face_type)
		{
			case BSPST_PLANAR:
			{
				createMesh(in, out.mesh);
			
				//vec3_c	origin;
				vec3_c	normal;
			
				for(int j=0; j<3; j++)
				{
					//origin[j] = LittleFloat(in->origin[j]);
					normal[j] = LittleFloat(in->normal[j]);
				}
				
				if(!normal.length())
				{
					//out.face_type = BSPST_PLANAR_NOCULL;
					out.face_type = BSPST_MESH;
				}
				break;
			}
			
			case BSPST_MESH:
			{
				createMesh(in, out.mesh);
				break;
			}
			
			case BSPST_BEZIER:
			{
				createBezierMesh(in, out.mesh);
				break;
			}
			
			case BSPST_BAD:
			case BSPST_FLARE:
			default:
				Com_Error(ERR_DROP, "cmodel_bsp_c::loadSurfaces: bad surface type %i", out.face_type);
		}
		
		//if(bsp)
		//	bsp->addSurface(out.face_type, out.shader_num, out.mesh.vertexes, out.mesh.indexes);
	}
}

void	cmodel_bsp_c::loadLeafSurfaces(const bsp_lump_t &l)//, d_bsp_c *bsp)
{
	Com_DPrintf("loading leaf surfaces ...\n");
	
	int* in = (int*)(_buffer + l.fileofs);
	if(l.filelen % sizeof(*in))
		Com_Error(ERR_DROP, "cmodel_bsp_c::loadLeafSurfaces: funny lump size");
	int count = l.filelen / sizeof(*in);

	if(count < 1)
		Com_Error(ERR_DROP, "cmodel_bsp_c::loadLeafSurfaces: BSP with no faces");

	_leafsurfaces = std::vector<int>(count);
	
	for(int i=0; i<count; i++)
	{
		int j = LittleLong(in[i]);
		
		if(j<0 || j >= (int)_surfaces.size())
			Com_Error(ERR_DROP, "cmodel_bsp_c::loadLeafSurfaces: bad surface number %i", j);
		
		_leafsurfaces[i] = j;
		
		//if(bsp)
		//	bsp->addLeafSurface(j);
	}
}

void	cmodel_bsp_c::loadLeafs(const bsp_lump_t &l)//, d_bsp_c *bsp)
{
	Com_DPrintf("loading leafs ...\n");
	
	bsp_dleaf_t* in = (bsp_dleaf_t*)(_buffer + l.fileofs);
	if(l.filelen % sizeof(*in))
		Com_Error(ERR_DROP, "cmodel_bsp_c::loadLeafs: funny lump size");
	int count = l.filelen / sizeof(*in);

	if(count < 1)
		Com_Error(ERR_DROP, "cmodel_bsp_c::loadLeafs: BSP with no leafs");

	_leafs = std::vector<cleaf_t>(count);	

	for(int i=0; i<count; i++, in++)
	{
		cleaf_t &out = _leafs[i];
		
		out.contents = 0;
		out.cluster = LittleLong(in->cluster);
		out.area = LittleLong(in->area);
		
		out.leafsurfaces_first = LittleLong(in->leafsurfaces_first);
		out.leafsurfaces_num = LittleLong(in->leafsurfaces_num);
		
		out.leafbrushes_first = LittleLong(in->leafbrushes_first);
		out.leafbrushes_num = LittleLong(in->leafbrushes_num);
		
		//out->leafpatches_first
		//out->leafpatches_num

		// OR brushes' contents
		for(int j=0; j<out.leafbrushes_num; j++)
		{
			cbrush_t& brush = _brushes[_leafbrushes[out.leafbrushes_first + j]];
			out.contents |= brush.contents;
		}

		// TODO OR patches' contents
		
		if(out.area >= (int)_areas.size())
			_areas.push_back(carea_t());
		
		//if(bsp)
		//	bsp->addLeaf(out.leafsurfaces_first, out.leafsurfaces_num, out.leafbrushes_first, out.leafbrushes_num, out.cluster, out.area);
	}
}

void	cmodel_bsp_c::loadNodes(const bsp_lump_t &l)//, d_bsp_c *bsp)
{
	Com_DPrintf("loading nodes ...\n");
	
	bsp_dnode_t* in = (bsp_dnode_t*)(_buffer + l.fileofs);
	if(l.filelen % sizeof(*in))
		Com_Error(ERR_DROP, "cmodel_bsp_c::loadNodes: funny lump size");
	int count = l.filelen / sizeof(*in);

	if(count < 1)
		Com_Error(ERR_DROP, "cmodel_bsp_c::loadNodes: BSP has no nodes");
	
	_nodes = std::vector<cnode_t>(count);
	
	for(int i=0; i<count; i++,in++)
	{
		cnode_t &out = _nodes[i];
	
		out.plane = &_planes[LittleLong(in->plane_num)];
		
		out.children[SIDE_FRONT] = LittleLong(in->children[SIDE_FRONT]);
		out.children[SIDE_BACK] = LittleLong(in->children[SIDE_BACK]);
		
		//if(bsp)
		//	bsp->addNode(LittleLong(in->plane_num), out.children[0], out.children[1]);
	}

}

void	cmodel_bsp_c::loadModels(const bsp_lump_t &l)//, d_bsp_c *bsp)
{
	Com_DPrintf("loading models ...\n");

	bsp_dmodel_t* in = (bsp_dmodel_t*)(_buffer + l.fileofs);
	if(l.filelen % sizeof(*in))
		Com_Error(ERR_DROP, "cmodel_bsp_c::loadModels: funny lump size");
	int count = l.filelen / sizeof(*in);

	if(count < 1)
		Com_Error(ERR_DROP, "cmodel_bsp_c::loadModels: BSP with no models");

	for(int i=0; i<count; i++, in++)
	{
		cmodel_c *cmodel;

		if(i == 0)
			cmodel = this;
		else
			cmodel = new cmodel_c(va("*%i", i), NULL, 0);
		
		cm_models.push_back(cmodel);
			
		int	modelsurfaces_first = LittleLong(in->modelsurfaces_first);
		int	modelsurfaces_num = LittleLong(in->modelsurfaces_num);
		
#if 1
		// create a big polygons list for every model using all surfaces for it
		int	vertexes_counter = 0;
		
		for(int j=0; j<modelsurfaces_num; j++)
		{
			csurface_t &surf = _surfaces[modelsurfaces_first + j];
	
			cshader_t &shader = _shaders[surf.shader_num];			
			
			if(shader.flags & (X_SURF_NOIMPACT | X_SURF_NONSOLID))
				continue;
			
			for(std::vector<vec3_c>::const_iterator ir = surf.mesh.vertexes.begin(); ir != surf.mesh.vertexes.end(); ir++)
			{
				cmodel->vertexes.push_back(*ir);
			}
			
			
			for(std::vector<index_t>::const_iterator ir = surf.mesh.indexes.begin(); ir != surf.mesh.indexes.end(); ir++)
			{
				cmodel->indexes.push_back(vertexes_counter + *ir);
			}
			
			
			vertexes_counter += surf.mesh.vertexes.size();
		}
		
		reverse(cmodel->indexes.begin(), cmodel->indexes.end());
#else		
		for(int j=0; j<modelsurfaces_num; j++)
		{
			csurface_t &surf = cm_surfaces[modelsurfaces_first + j];
			cshader_t &shader = cm_shaders[surf.shader_num];
			
			if(surf.mesh.vertexes.empty() || surf.mesh.indexes.empty())
				continue;
		
			model->surfaces.push_back(csurface_c(surf.mesh.vertexes, surf.mesh.indexes, shader.flags, shader.contents));
		}
#endif
		
		/*
		if(!i)
		{
			out->headnode = 0;
		}
		else
		{
			out->headnode = -1 - cm_leafs_num;
		
			leaf = &cm_leafs[cm_leafs_num++];
			leaf->leafbrushes_num = LittleLong(in->brushes_num);
			leaf->leafbrushes_first = cm_leafbrushes_num;
			leaf->contents = 0;
			
			
			for(j=0, leafbrush = &cm_leafbrushes[cm_leafbrushes_num]; j<leaf->leafbrushes_num; j++, leafbrush++)
			{
				*leafbrush = LittleLong(in->brushes_first) + j;
				leaf->contents |= cm_brushes[*leafbrush].contents;
			}
			
			cm_leafbrushes_num += leaf->leafbrushes_num;
		}
		*/

		for(int j=0; j<3; j++)
		{	
			// spread the mins / maxs by a pixel
			cmodel->_aabb._mins[j] = LittleFloat(in->mins[j]);
			cmodel->_aabb._maxs[j] = LittleFloat(in->maxs[j]);
		}
	}
	
	// set bsp size for AABB and assign it to the model
	//if(bsp)
	//	bsp->setLengths(cm_models[0]->getAABB().size());
}

void	cmodel_bsp_c::loadVisibility(const bsp_lump_t &l)
{
	Com_DPrintf("loading visibility ...\n");
	
	int pvs_size = l.filelen - BSP_PVS_HEADERSIZE;
	
	Com_DPrintf("PVS data size: %i\n", pvs_size);
	
	if(pvs_size <= 0)
	{
		_pvs.clear();
		return;
	}
	
	_pvs = std::vector<byte>(pvs_size, 0);
	
	for(int i=0; i<pvs_size; i++)
	{
		_pvs[i] = _buffer[l.fileofs + BSP_PVS_HEADERSIZE + i];
	}
	
	_pvs_clusters_num  = LittleLong(((int*)((byte*)_buffer + l.fileofs))[0]);
	_pvs_clusters_size = LittleLong(((int*)((byte*)_buffer + l.fileofs))[1]);
	

	/*
	memcpy(cm_visibility, ((byte*)_buffer + l.fileofs), l.filelen);

	cm_pvs->clusters_num = LittleLong(cm_pvs->clusters_num);
	cm_pvs->cluster_size = LittleLong(cm_pvs->cluster_size);
	*/
}

void	cmodel_bsp_c::loadEntityString(const bsp_lump_t &l)
{
	Com_DPrintf("loading entities ...\n");
	
	_entitystring.clear();
	
	for(int i=0; i<l.filelen; i++)
	{
		_entitystring += _buffer[l.fileofs + i];
	}
	
	_entitystring = X_strlwr(_entitystring);
}


static std::string			cm_name;
static cshader_t			cm_nullshader;

static cvar_t*	cm_noareas;
static cvar_t*	cm_use_brushes;
static cvar_t*	cm_use_patches;
static cvar_t*	cm_use_meshes;
static cvar_t*	cm_subdivisions;

static cmodel_box_c		box_cmodel;
//static plane_c		box_planes[6];
//static int			box_headnode = 0;
//static cbrush_t		box_brush;
//static cleaf_t		box_leaf;



static vec3_c		trace_start;		// replace this by a ray
static vec3_c		trace_end;

static aabb_c		trace_bbox;
static aabb_c		trace_bbox_abs;

static vec3_c		trace_extents;

static trace_t		trace_trace;
//static int		trace_contents;
//static bool		trace_ispoint;		// optimized case


/*
// fragment clipping
#define	MAX_FRAGMENT_VERTEXES	128
static vec3_c		mark_origin;
static vec3_c		mark_normal;
static vec_t		mark_radius;

static int		mark_vertexes_num;
static int		mark_vertexes_max;
static vec3_c*		mark_vertexes;

static int		mark_fragments_num;
static int		mark_fragments_max;
static cfragment_t*	mark_fragments;

static plane_c		mark_planes[6];

static int		mark_checkcount;
*/


static void	CM_FloodAreaConnections();




/*
===================
CM_InitBoxHull

Set up the planes and nodes so that the six floats of a bounding box
can just be stored out and get a proper clipping hull structure.
===================
*/
void	CM_InitBoxHull()
{
#if 0
	/*
	for(int i=0; i<6; i++)
	{
		cm_nodes.push_back(cnode_t());
		cm_brushsides.push_back(cbrushside_t());	
		
		cm_planes.push_back(plane_c());
		cm_planes.push_back(plane_c());
	}
		
	cm_brushes.push_back(cbrush_t());
	cm_leafs.push_back(cleaf_t());
	cm_leafbrushes.push_back(cm_brushes.size());
	*/
	
	/*
	if(	cm_nodes_num >= MAX_CM_NODES+6 || 
		cm_brushes_num >= MAX_CM_BRUSHES+1 || 
		cm_leafbrushes_num >= MAX_CM_LEAFBRUSHES+1 || 
		cm_brushsides_num >= MAX_CM_BRUSHSIDES+6 || 
		cm_planes_num > MAX_CM_PLANES+12
	)
		Com_Error (ERR_DROP, "CM_InitBoxHull: Not enough room for box tree");
	*/
	
//	box_headnode = cm_nodes.size()-6;
//	box_planes = &cm_planes[cm_planes.size() -12];
	
//	box_brush = &cm_brushes[cm_brushes.size() -1];
	box_brush.sides_num = 6;
	box_brush.sides_first = 0; //cm_brushsides.size() -6;
	box_brush.contents = X_CONT_BODY;

//	box_leaf = &cm_leafs[cm_leafs.size() -1];
//	box_leaf->contents = X_CONT_BODY;
//	box_leaf->leafbrushes_first = cm_leafbrushes.size() -1;
//	box_leaf->leafbrushes_num = 1;

	box_model.
	
	for(int i=0; i<6; i++)
	{
		int side = i & 1;

		// brush sides
		cbrushside_t* s = &cm_brushsides[(cm_brushsides.size() -6) + i];
		s->plane = &cm_planes[((cm_planes.size() -12) + i*2 + side)];
		s->shader = &cm_nullshader;

		// nodes
		cnode_t* n = &cm_nodes[box_headnode + i];
		n->plane = &cm_planes[((cm_planes.size() -12) + i*2)];
		n->children[side] = -1 - (cm_leafs.size() -1);
		
		if(i != 5)
			n->children[side^1] = box_headnode + i + 1;
		else
			n->children[side^1] = -1 - (cm_leafs.size() -1);
		
		// planes
		plane_c* p = &box_planes[i*2];
		p->_normal.clear();
		p->_normal[i>>1] = 1;
		p->_type = (plane_type_e)(i>>1);
		p->_signbits = 0;

		p = &box_planes[i*2+1];
		p->_normal.clear();
		p->_normal[i>>1] = -1;
		p->_type = (plane_type_e)(3 + (i>>1));
		p->_signbits = 0;
	}
#endif
}


cmodel_c*	CM_BeginRegistration(const std::string &name, bool clientload, unsigned *checksum)//, dSpaceID space)
{
	unsigned*		buf;
	int			i;
	bsp_dheader_t		header;
	int			length;
	std::string		full_name = "maps/" + name + ".bsp";
	static unsigned		last_checksum;

	cm_noareas	= Cvar_Get("cm_noareas", "1", CVAR_NONE);
	cm_use_brushes	= Cvar_Get("cm_use_brushes", "1", CVAR_NONE);
	cm_use_patches	= Cvar_Get("cm_use_patches", "0", CVAR_NONE);
	cm_use_meshes	= Cvar_Get("cm_use_meshes", "0", CVAR_NONE);
	cm_subdivisions	= Cvar_Get("cm_subdivisions", "0", CVAR_NONE);
	
	if(X_strequal(cm_name.c_str(), name.c_str()) && (clientload || !Cvar_VariableValue ("flushmap")))
	{
		*checksum = last_checksum;
		
		if(!clientload)
		{
			CM_FloodAreaConnections();
		}
		return cm_models[0];// NULL;		// still have the right version
	}

	// free old stuff
	X_purge(cm_models);
	cm_models.clear();

	// load the file
	length = VFS_FLoad(full_name, (void **)&buf);
	if (!buf)
		Com_Error(ERR_DROP, "CM_BeginRegistration: Couldn't load %s", full_name.c_str());
		
	boost::crc_32_type crc;
	crc.process_bytes(buf, length);
	last_checksum = LittleLong(crc.checksum());
	*checksum = last_checksum;

	header = *(bsp_dheader_t *)buf;
	for(i=0; i<(int)sizeof(bsp_dheader_t)/4; i++)
		((int *)&header)[i] = LittleLong(((int *)&header)[i]);

	if(header.version != BSP_VERSION)
		Com_Error(ERR_DROP, "CM_BeginRegistration: %s has wrong version number (%i should be %i)", full_name.c_str(), header.version, BSP_VERSION);

	cm_name = name;

	Com_DPrintf("CM_BeginRegistration: loading %s into heap ...\n", full_name.c_str());
	
	// create ODE BSP collision geom
	/*
	d_bsp_c* bsp = NULL;
	if(space != 0)
		bsp = new d_bsp_c(space);
	*/

	cmodel_c* bsp = new cmodel_bsp_c("*0", (byte*)buf, length, header);
	cm_models.push_back(bsp);

	VFS_FFree(buf);
	
	CM_InitBoxHull();
	
	return bsp;
}


/*
int	CM_ClusterSize()
{
	return cm_pvs.size() ? cm_pvs_clusters_size : (MAX_CM_LEAFS / 8);
}

int	CM_NumClusters()
{
	return cm_pvs_clusters_num;
}
*/

int	CM_NumModels()
{
	return cm_models.size();
}


/*
===================
CM_HeadnodeForBox

To keep everything totally uniform, bounding boxes are turned into small
BSP trees instead of being compared directly.
===================
*/
cmodel_c*	CM_ModelForBox(const aabb_c & bbox)
{
#if 0
	box_planes[0]._dist	=  bbox._maxs[0];
	box_planes[1]._dist	= -bbox._maxs[0];
	box_planes[2]._dist	=  bbox._mins[0];
	box_planes[3]._dist	= -bbox._mins[0];
	
	box_planes[4]._dist	=  bbox._maxs[1];
	box_planes[5]._dist	= -bbox._maxs[1];
	box_planes[6]._dist	=  bbox._mins[1];
	box_planes[7]._dist	= -bbox._mins[1];
	
	box_planes[8]._dist	=  bbox._maxs[2];
	box_planes[9]._dist	= -bbox._maxs[2];
	box_planes[10]._dist	=  bbox._mins[2];
	box_planes[11]._dist	= -bbox._mins[2];
#endif
	return &box_cmodel;
}










/*
void	CM_TestBoxInMesh(const aabb_c &bbox, const vec3_c &p1, trace_t *trace, cmesh_t *mesh, cshader_t *shader)
{
	int			i;
	plane_c	*plane;
	vec3_c		ofs;
	float		d1;
	
	index_t*	index;
	vec3_c		v0, v1, v2;
	vec3_c		n0, n1, n2;
	vec3_c		normal;
	vec_t		dist;
	plane_c	p;

	if(!mesh->vertexes_num || mesh->indexes_num)
		return;

	for(i=0, index=mesh->indexes; i<(mesh->indexes_num / 3); i++, index += 3)
	{
		v0 = mesh->vertexes[index[0]];
		v1 = mesh->vertexes[index[1]];
		v2 = mesh->vertexes[index[2]];
		
		n0 = mesh->normals[index[0]];
		n1 = mesh->normals[index[1]];
		n2 = mesh->normals[index[2]];
		
		normal[0] = n0[0] + n1[0] + n2[0];
		normal[1] = n0[1] + n1[1] + n2[1];
		normal[2] = n0[2] + n1[2] + n2[2];
		
		normal.normalize();
		
		dist = normal.dotProduct(v0);
		
		p.set(normal, dist);
		
		plane = &p;

		// general box case

		// push the plane out apropriately for mins/maxs

		// FIXME: use signbits into 8 way lookup for each mins/maxs
		for(int j=0; j<3; j++)
		{
			if(plane->_normal[j] < 0)
				ofs[j] = bbox._maxs[j];
			else
				ofs[j] = bbox._mins[j];
		}
		
		ofs += p1;
		
		d1 = plane->diff(p1);

		// if completely in front of face, no intersection
		if(d1 > 0.01)
			return;
	}

	// inside this brush
	trace->startsolid = trace->allsolid = true;
	trace->fraction = 0;
	trace->contents = shader->contents;
}
*/

/*
static void	CM_TraceToLeaf(int leafnum)
{
	const cleaf_t& leaf = cm_leafs[leafnum];
	
	if(!(leaf.contents & trace_contents))
		return;
	
	if(cm_use_brushes->getInteger())
	{
		// trace line against all brushes in the leaf
		for(int i=0; i<leaf.leafbrushes_num; i++)
		{
			cbrush_t& brush = cm_brushes[cm_leafbrushes[leaf.leafbrushes_first + i]];
		
			if(brush.checkcount == cm_checkcount)
				continue;	// already checked this brush in another leaf
		
			brush.checkcount = cm_checkcount;

			if(!(brush.contents & trace_contents))
				continue;
		
			CM_ClipBoxToBrush(trace_bbox, trace_start, trace_end, trace_trace, brush);
		
			if(!trace_trace.fraction)
				return;
		}
	}
	
	if(cm_use_patches->integer)
	{
		// trace line against all patches in the leaf
		for(int i=0; i<leaf->leafpatches_num; i++)
		{
			cpatch_t *patch = &cm_patches[cm_leafpatches[leaf->leafpatches_first + i]];
		
			if(patch->checkcount == cm_checkcount)
				continue;	// already checked this patch in another leaf
			
			patch->checkcount = cm_checkcount;

			if(!(patch->shader->contents & trace_contents))
				continue;
		
			if(!patch->bbox_abs.intersect(trace_bbox_abs))
				continue;
		
			for(int j=0; j<patch->brushes_num; j++)
			{
				CM_ClipBoxToPatch(trace_bbox, trace_start, trace_end, &trace_trace, &patch->brushes[j]);
			
				if(!trace_trace.fraction)
					return;
			}
		}
	}
	
	if(cm_use_meshes->integer)
	{
		// trace line against all surfaces in the leaf
		for(int i=0; i<leaf->leafsurfaces_num; i++)
		{
			csurface_t *surf = &cm_surfaces[cm_leafsurfaces[leaf->leafsurfaces_first + i]];
		
			if(surf->checkcount == cm_checkcount)
				continue;	// already checked this brush in another leaf
		
			surf->checkcount = cm_checkcount;
		
			cshader_t *shader = &cm_shaders[surf->shader_num];
		
			if(!(shader->contents & trace_contents))
				continue;
		
			CM_ClipBoxToMesh(trace_bbox, trace_start, trace_end, &trace_trace, surf->mesh, shader);
		
			if(!trace_trace.fraction)
				return;
		}
	}

}
*/




static void	CM_FloodAreaConnections()
{
/*
	// area 0 is the void and not considered
	for(uint_t i=1; i<cm_areas.size(); i++)
	{
		for(uint_t j=1; j<cm_areas.size(); j++)
		{
			cm_areas[i].numareaportals[j] = 1;//(j == i);
		}
	}

	Com_DPrintf("%i areas flooded\n", cm_areas.size());
*/
}

/*
int	CM_GetClosestAreaPortal(const vec3_c &p)
{
	return -1;
}
*/

/*
bool	CM_GetAreaPortalState(int portal)
{
	return false;
}
*/

void	CM_SetAreaPortalState(int portal, bool open)
{
/*
	if(area1 > cm_areas_num || area2 > cm_areas_num)
		Com_Error (ERR_DROP, "CM_SetAreaPortalState: areas out of range");

	if(open)
	{
		cm_areas[area1].numareaportals[area2]++;
		cm_areas[area2].numareaportals[area1]++;
	}
	else
	{
		cm_areas[area1].numareaportals[area2]--;
		cm_areas[area2].numareaportals[area1]--;
	}
*/
}

/*
bool	CM_AreasConnected(int area1, int area2)
{
	if(cm_noareas->getInteger())
		return true;
	
	if(area1 > (int)cm_areas.size() || area2 > (int)cm_areas.size())
		Com_Error(ERR_DROP, "CM_AreasConnected: areas out of range");

	if(cm_areas[area1].numareaportals[area2])
		return true;
	
	// area 0 is not used
	for(uint_t i=0; i<cm_areas.size(); i++)
	{
		if(cm_areas[i].numareaportals[area1] && cm_areas[i].numareaportals[area2])
			return true;
	}
	
	return false;
}
*/

/*
=================
CM_WriteAreaBits

Writes a length byte followed by a bit vector of all the areas
that area in the same flood as the area parameter

This is used by the client refreshes to cull visibility
=================
*/
void	CM_WriteAreaBits(boost::dynamic_bitset<byte> &bits, int area)
{
#if 0
	if(cm_noareas->getInteger() || area <= -1)
	{	
		// for debugging, send everything
		bits = boost::dynamic_bitset<byte>(cm_areas.size());
		bits.set();
	}
	else
	{
		bits = boost::dynamic_bitset<byte>(cm_areas.size());
		bits.reset();

		// area 0 is the void and should not be visible
		for(uint_t i=0; i<cm_areas.size(); i++)
		{
			if((int)i == area || CM_AreasConnected(i, area))
				bits[i] = true;
		}
	}
#else
	bits = boost::dynamic_bitset<byte>(0);
	bits.set();
#endif
}

/*
void	CM_MergeAreaBits(byte *buffer, int area)
{
	// area 0 is not used
	for(uint_t i=0; i<cm_areas.size(); i++)
	{
		if(CM_AreasConnected(i, area) || (int)i == area)
			buffer[i>>3] |= 1<<(i&7);
	}
}
*/

/*
=============
CM_HeadnodeVisible

Returns true if any leaf under headnode has a cluster that
is potentially visible
=============
*/
/*
bool	CM_HeadnodeVisible(int nodenum, byte *visbits)
{
	int		leafnum;
	int		cluster;
	cnode_t	*node;

	if(nodenum < 0)
	{
		leafnum = -1 - nodenum;
		cluster = cm_leafs[leafnum].cluster;
		
		if(cluster == -1)
			return false;
		
		if(visbits[cluster>>3] & (1<<(cluster&7)))
			return true;
		
		return false;
	}

	node = &cm_nodes[nodenum];
	
	if(CM_HeadnodeVisible(node->children[0], visbits))
		return true;
	
	return CM_HeadnodeVisible(node->children[1], visbits);
}
*/

/*
static void	CM_ClipFragmentToSurface(csurface_t *surf)
{
	//TODO
}

static void	CM_ClipFragmentToLeaf(int leafnum)
{
	cleaf_t		*leaf;
	//cbrush_t	*brush;
	//cpatch_t	*patch;
	csurface_t	*surf;
	cshader_t	*shader;

	leaf = &cm_leafs[leafnum];
	
	// clip fragment against all surfaces in the leaf
	
	for(int i=0; i<leaf->leafsurfaces_num; i++)
	{
		surf = &cm_surfaces[cm_leafsurfaces[leaf->leafsurfaces_first + i]];
		
		if(surf->checkcount == mark_checkcount)
			continue;	// already checked this brush in another leaf
		
		surf->checkcount = mark_checkcount;
		
		shader = &cm_shaders[surf->shader_num];
		
		if(shader->contents & (SURF_NOIMPACT | SURF_NOMARKS))
			continue;
		
		CM_ClipFragmentToSurface(surf);
	}
	

}


static void	CM_MarkFragments_r(int num)
{
	if(mark_vertexes_num >= mark_vertexes_max || mark_fragments_num >= mark_fragments_max)
		return;

	// if < 0, we are in a leaf node
	if(num < 0)
	{
		CM_ClipFragmentToLeaf(-1 - num);
		return;
	}

	cnode_t *node = &cm_nodes[num];
	
	float dist = node->plane->distance(mark_origin);
	
	if(dist > mark_radius)
	{
		CM_MarkFragments_r(node->children[0]);
		return;
	}
	
	if(dist < -mark_radius)
	{
		CM_MarkFragments_r(node->children[1]);
		return;
	}
	
	CM_MarkFragments_r(node->children[0]);
	CM_MarkFragments_r(node->children[1]);
}
*/

/*
int	CM_MarkFragments(const vec3_c &origin, const matrix_c axis, float radius, int vertexes_max, vec3_c *vertexes, int fragments_max, cfragment_t *fragments)
{
	if(!cm_nodes.size())	// map not loaded
		return 0;

	mark_checkcount++;
	
	// initialize fragments
	mark_vertexes_num = 0;
	mark_vertexes_max = vertexes_max;
	mark_vertexes = vertexes;
	
	mark_fragments_num = 0;
	mark_fragments_max = fragments_max;
	mark_fragments = fragments;
	
	mark_origin = origin;
	mark_normal = axis[0];
	mark_radius = radius;
	
	// calculate clipping planes
	for(int i=0; i<3; i++)
	{
		vec3_c normal	= (vec_t*)axis[i];
		vec_t dist	= normal.dotProduct(mark_origin) - radius;
		
		mark_planes[i*2].set(normal, dist);
		
		mark_planes[i*2+1] = mark_planes[i*2];
		mark_planes[i*2+1].negate();
	}
	
	// clip against world geometry
	CM_MarkFragments_r(0);
	
	return mark_fragments_num;
	
	return 0;
}
*/


