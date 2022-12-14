/*************************************************************************
 *                                                                       *
 * Open Dynamics Engine, Copyright (C) 2001,2002 Russell L. Smith.       *
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

#ifndef _ODE_CONTACT_H_
#define _ODE_CONTACT_H_

#include "common.h"

enum
{
	dContactMu2		= 0x001,
	dContactFDir1		= 0x002,
	dContactBounce		= 0x004,
	dContactSoftERP		= 0x008,
	dContactSoftCFM		= 0x010,
	dContactMotion1		= 0x020,
	dContactMotion2		= 0x040,
	dContactSlip1		= 0x080,
	dContactSlip2		= 0x100,

	dContactApprox0		= 0x0000,
	dContactApprox1_1	= 0x1000,
	dContactApprox1_2	= 0x2000,
	dContactApprox1		= 0x3000
};


class dSurfaceParameters
{
public:
	inline dSurfaceParameters()
	{
		mode		= 0;
		mu		= X_infinity;
	}

	 /* must always be defined */
	int	mode;
	vec_t	mu;

	 /* only defined if the corresponding flag is set in mode */
	vec_t	mu2;
	vec_t	bounce;
	vec_t	bounce_vel;
	vec_t	soft_erp;
	vec_t	soft_cfm;
	vec_t	motion1,motion2;
	vec_t	slip1,slip2;
};


// contact info set by collision functions
class dContactGeom
{
public:
	inline dContactGeom()
	{
		_depth		= 0;
		_g1		= NULL;
		_g2		= NULL;
	}

	inline dContactGeom(const vec3_c &origin, const vec3_c normal, vec_t depth, dGeomID g1, dGeomID g2)
	{
		_origin		= origin;
		_normal		= normal;
		_depth		= depth;
		_g1		= g1;
		_g2		= g2;
	}
	
	inline const vec3_c&	getPosition() const		{return _origin;}
	inline const vec3_c&	getNormal() const		{return _normal;}
	inline const vec_t	getPenetrationDepth() const	{return _depth;}
	inline const dGeomID	getGeom1() const		{return _g1;}
	inline const dGeomID	getGeom2() const		{return _g2;}
	
	inline void		reverse()
	{
		_normal.negate();
		std::swap(_g1, _g2);
	}
	
	inline bool operator == (const dContactGeom &g) const
	{
		return
		(
			_origin == g._origin &&
			_normal == g._normal &&
			_depth	== g._depth &&
			_g1	== g._g1 &&
			_g2	== g._g2
		);
	}

//private:
	vec3_c		_origin;
	vec3_c		_normal;
	vec_t		_depth;
	dGeomID		_g1;
	dGeomID		_g2;
};


/* contact info used by contact joint */

class dContact
{
public:
	inline dContact()
	{
	}

	inline dContact(const dContactGeom &g)
	{
		geom = g;
	}

	dSurfaceParameters	surface;
	dContactGeom		geom;
	vec3_c			fdir1;
};

#endif
