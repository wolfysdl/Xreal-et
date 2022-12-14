/*************************************************************************
 *                                                                       *
 * Open Dynamics Engine, Copyright (C) 2001-2003 Russell L. Smith.       *
 * All rights reserved.  Email: russ@q12.org   Web: www.q12.org          *
 *                                                                       *
 * This library is free software; you can redistribute it and/or         *
 * modify it under the terms of EITHER:                                  *
 *   (1) The GNU Lesser General Public License as published by the Free  *
 *       Software Foundation; either version 2.1 of the License, or (at  *
 *       your option) any later version. The text of the GNU Lesser      *
 *       General Public License is included with this library in the     *
 *       file LICENSE.TXT.                                               *
 *   (2) The BSD-style license that is included with this library in     *
 *       the file LICENSE-BSD.TXT.                                       *
 *                                                                       *
 * This library is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the files    *
 * LICENSE.TXT and LICENSE-BSD.TXT for more details.                     *
 *                                                                       *
 *************************************************************************/

/*

internal data structures and functions for collision detection.

*/

#ifndef _ODE_COLLISION_KERNEL_H_
#define _ODE_COLLISION_KERNEL_H_

#include "common.h"
#include "contact.h"
#include "collision.h"
#include "object.h"

//****************************************************************************
// constants and macros

// mask for the number-of-contacts field in the dCollide() flags parameter
#define NUMC_MASK (0xffff)

#define IS_SPACE(geom) \
  ((geom)->type >= dFirstSpaceClass && (geom)->type <= dLastSpaceClass)

//****************************************************************************
// geometry object base class

// position vector and rotation matrix for geometry objects that are not
// connected to bodies.

struct dxPosR
{
	dVector3	pos;
	matrix_c	R;
};


// geom flags.
//
// GEOM_DIRTY means that the space data structures for this geom are
// potentially not up to date. NOTE THAT all space parents of a dirty geom
// are themselves dirty. this is an invariant that must be enforced.
//
// GEOM_AABB_BAD means that the cached AABB for this geom is not up to date.
// note that GEOM_DIRTY does not imply GEOM_AABB_BAD, as the geom might
// recalculate its own AABB but does not know how to update the space data
// structures for the space it is in. but GEOM_AABB_BAD implies GEOM_DIRTY.
// the valid combinations are: 0, GEOM_DIRTY, GEOM_DIRTY|GEOM_AABB_BAD.

enum
{
	GEOM_DIRTY		= 1,	// geom is 'dirty', i.e. position unknown
	GEOM_AABB_BAD		= 2,	// geom's AABB is not valid
	GEOM_PLACEABLE		= 4,	// geom is placeable
	GEOM_ENABLED		= 8,		// geom is enabled

	// Ray specific
	RAY_FIRSTCONTACT	= 0x10000,
	RAY_BACKFACECULL	= 0x20000,
	RAY_CLOSEST_HIT		= 0x40000
};


// geometry object base class. pos and R will either point to a separately
// allocated buffer (if body is 0 - pos points to the dxPosR object) or to
// the pos and R of the body (if body nonzero).
// a dGeomID is a pointer to this object.

struct dxGeom : public dBase 
{
	dxGeom(dSpaceID _space, int is_placeable);
	virtual ~dxGeom();
	
	// compute the AABB for this object and put it in aabb. this function
	// always performs a fresh computation, it does not inspect the
	// GEOM_AABB_BAD flag
	virtual void	computeAABB() = 0;
	
	// test whether the given AABB object intersects with this object, return
	// 1=yes, 0=no. this is used as an early-exit test in the space collision
	// functions. the default implementation returns 1, which is the correct
	// behavior if no more detailed implementation can be provided
	virtual int	AABBTest(dxGeom *o, vec_t aabb[6]);
	
	//
	// utility functions
	//
	
	// compute the AABB only if it is not current. this function manipulates
	// the GEOM_AABB_BAD flag
	
	void	recomputeAABB()
	{
		if(gflags & GEOM_AABB_BAD)
		{
			computeAABB();
			gflags &= ~GEOM_AABB_BAD;
		}
	}
	
	// add and remove this geom from a linked list maintained by a space.
	void	spaceAdd(dxGeom **first_ptr)
	{
		next = *first_ptr;
		tome = first_ptr;
		
		if(*first_ptr)
			(*first_ptr)->tome = &next;
		*first_ptr = this;
	}
	
	void	spaceRemove()
	{
		if(next)
			next->tome = tome;
		*tome = next;
	}
	
	// add and remove this geom from a linked list maintained by a body.
	void	bodyAdd(dxBody *b)
	{
		body = b;
		body_next = b->geom;
		b->geom = this;
	}
	
	void	bodyRemove();
	
  
	int		type;		// geom type number, set by subclass constructor
	int		gflags;		// flags used by geom and space
	void*		data;		// user-defined data pointer
	dBodyID		body;		// dynamics body associated with this object (if any)
	dxGeom*		body_next;	// next geom in body's linked list of associated geoms
	vec_t*		pos;		// pointer to object's position vector
	matrix_c*	R;		// pointer to object's rotation matrix
	
	// information used by spaces
	dxGeom*		next;		// next geom in linked list of geoms
	dxGeom**	tome;		// linked list backpointer
	dxSpace*	parent_space;// the space this geom is contained in, 0 if none
	vec_t		aabb[6];	// cached AABB for this space
	unsigned long	category_bits, collide_bits;
};

//****************************************************************************
// the basic geometry objects

struct dxSphere : public dxGeom
{
	dxSphere(dSpaceID space, vec_t _radius);
	
	void	computeAABB();
	
	vec_t radius;		// sphere radius
};


struct dxBox : public dxGeom
{
	dxBox(dSpaceID space, vec_t lx, vec_t ly, vec_t lz);
	
	void	computeAABB();
	
	dVector3	side;	// side lengths (x,y,z)
};


struct dxPlane : public dxGeom
{
	dxPlane(dSpaceID space, vec_t a, vec_t b, vec_t c, vec_t d);
	
	void	computeAABB();
	
//	vec_t		p[4];
	plane_c	p;
};


struct dxRay : public dxGeom
{
	
	dxRay(dSpaceID space, vec_t _length);
	
	void	computeAABB();
	
	vec_t		length;
};


struct dxBSP : public dxGeom
{
	dxBSP(dSpaceID space);
	
	void	computeAABB();

	struct dBSPNode
	{
		dBSPNode()
		{
			plane		= NULL;
		
			children[0]	= 0;
			children[1]	= 0;
		}
	
		plane_c*	plane;		// split plane
		int 		children[2];	// negative numbers are leafs
	};
	
	struct dBSPLeaf
	{
		int	cluster;
		int	area;
		
		int	surfaces_first;
		int	surfaces_num;
		
		int	brushes_first;
		int	brushes_num;
		
		int	patches_first;
		int	patches_num;
	};
	
	struct dBSPShader
	{
		uint_t			flags;
		uint_t			contents;
	};
	
	struct dBSPSurface
	{
		int			face_type;
	
		int			shader_num;
	
		plane_c			plane;			// BSPST_PLANAR only
	
		//int			mesh_cp[2];		// BSPST_BEZIER only
	
		// per vertex data
		std::vector<vec3_c>	vertexes;
		std::vector<vec3_c>	normals;
		
		// per triangle data
		std::vector<index_t>	indexes;
		std::vector<plane_c>	planes;
	
		int			checkcount;
	};
	
	/*
	struct dBSPBrush
	{
		int	sides_first;
		int	sides_num;
		
		int	checkcount;
	};
	
	struct dBSPBrushSide
	{
		plane_c* plane;
	};
	*/
	
	dVector3			side;	// side lengths (x,y,z) for AABB
	std::vector<dBSPNode>		nodes;
	std::vector<dBSPLeaf>		leafs;
	std::vector<plane_c>		planes;
//	std::vector<dBSPBrush>		brushes;
//	std::vector<dBSPBrushSide>	brushsides;
//	std::vector<int>		leafbrushes;
	std::vector<dBSPShader>		shaders;
	std::vector<dBSPSurface>	surfaces;
	std::vector<int>		leafsurfaces;
	int 				checkcount;
};

//****************************************************************************
// the base space class
//
// the contained geoms are divided into two kinds: clean and dirty.
// the clean geoms have not moved since they were put in the list,
// and their AABBs are valid. the dirty geoms have changed position, and
// their AABBs are may not be valid. the two types are distinguished by the
// GEOM_DIRTY flag. all dirty geoms come *before* all clean geoms in the list.

struct dxSpace : public dxGeom
{
	dxSpace(dSpaceID _space);
	~dxSpace();
	
	void	computeAABB();
	
	void		setCleanup(int mode);
	int		getCleanup();
	int		query(dxGeom *geom);
	int		getNumGeoms();
	virtual dxGeom*	getGeom(int i);
	
	virtual void	add(dxGeom *);
	virtual void	remove(dxGeom *);
	virtual void	dirty(dxGeom *);
	
	// turn all dirty geoms into clean geoms by computing their AABBs and any
	// other space data structures that are required. this should clear the
	// GEOM_DIRTY and GEOM_AABB_BAD flags of all geoms.
	virtual void	cleanGeoms() = 0;
  
	virtual void	collide(void *data, dNearCallback *callback) = 0;
	virtual void	collide2(void *data, dxGeom *geom, dNearCallback *callback) = 0;
	
	int		count;		// number of geoms in this space
	dxGeom*		first;		// first geom in list
	int		cleanup;	// cleanup mode, 1=destroy geoms on exit
	
	// cached state for getGeom()
	int		current_index;	// only valid if current_geom != 0
	dxGeom*		current_geom;	// if 0 then there is no information
	
	// locking stuff. the space is locked when it is currently traversing its
	// internal data structures, e.g. in collide() and collide2(). operations
	// that modify the contents of the space are not permitted when the space
	// is locked.
	int		lock_count;
};


#endif
