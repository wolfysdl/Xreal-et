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

/*

some useful collision utility stuff. this includes some API utility
functions that are defined in the public header files.

*/

#include <functional>

#include "common.h"
#include "odemath.h"
#include "collision.h"
#include "collision_util.h"


struct dContactCompare : public std::unary_function<dContact, bool>
{
	dContactGeom	_geom;
	
	dContactCompare(const dContact &c)
	{
		_geom = c.geom;
	}
	
	bool	operator()(const dContact &c) const	{return _geom == c.geom;}
};

bool	dAddContact(const dContact c, std::vector<dContact> &contacts)
{
#if 1
	std::vector<dContact>::iterator ir = std::find_if(contacts.begin(), contacts.end(), dContactCompare(c));
	
	if(ir == contacts.end())
	{
		contacts.push_back(c);
		return true;
	}
	else
		return false;
#else
	contacts.push_back(c);
	return true;
#endif
}

void	dGetContactData(const vec3_c& p, const vec3_c &v0,  const vec3_c &edge0, const vec3_c &edge1, vec_t &dist, float &u, float &v)
{
	vec3_c diff = v0 - p;

	float A00 = edge0.dotProduct(edge0);
	float A01 = edge0.dotProduct(edge1);
	float A11 = edge1.dotProduct(edge1);

	float B0 = diff.dotProduct(edge0);
	float B1 = diff.dotProduct(edge1);

	float C = diff.dotProduct(diff);

	float Det = X_fabs(A00 * A11 - A01 * A01);
	u = A01 * B1 - A11 * B0;
	v = A01 * B0 - A00 * B1;

	float DistSq;

	if (u + v <= Det)
	{
		if(u < REAL(0.0))
		{
			if(v < REAL(0.0))
			{
				// region 4
				if(B0 < REAL(0.0))
				{
					v = REAL(0.0);
					
					if(-B0 >= A00)
					{
						u = REAL(1.0);
						DistSq = A00 + REAL(2.0) * B0 + C;
					}
					else
					{
						u = -B0 / A00;
						DistSq = B0 * u + C;
					}
				}
				else
				{
					u = REAL(0.0);
					
					if(B1 >= REAL(0.0))
					{
						v = REAL(0.0);
						DistSq = C;
					}
					else if(-B1 >= A11)
					{
						v = REAL(1.0);
						DistSq = A11 + REAL(2.0) * B1 + C;
					}
					else
					{
						v = -B1 / A11;
						DistSq = B1 * v + C;
					}
				}
			}
			else
			{
				// region 3
				u = REAL(0.0);
				
				if(B1 >= REAL(0.0))
				{
					v = REAL(0.0);
					DistSq = C;
				}
				else if(-B1 >= A11)
				{
					v = REAL(1.0);
					DistSq = A11 + REAL(2.0) * B1 + C;
				}
				else
				{
					v = -B1 / A11;
					DistSq = B1 * v + C;
				}
			}
		}
		else if(v < REAL(0.0))
		{
			// region 5
			v = REAL(0.0);
			
			if(B0 >= REAL(0.0))
			{
				u = REAL(0.0);
				DistSq = C;
			}
			else if(-B0 >= A00)
			{
				u = REAL(1.0);
				DistSq = A00 + REAL(2.0) * B0 + C;
			}
			else
			{
				u = -B0 / A00;
				DistSq = B0 * u + C;
			}
		}
		else
		{
			// region 0
			// minimum at interior point
			if(Det == REAL(0.0))
			{
				u = REAL(0.0);
				v = REAL(0.0);
				DistSq = X_infinity;
			}
			else
			{
				float InvDet = X_recip(Det);
				u *= InvDet;
				v *= InvDet;
				DistSq = u * (A00 * u + A01 * v + REAL(2.0) * B0) + v * (A01 * u + A11 * v + REAL(2.0) * B1) + C;
			}
		}
	}
	else
	{
		vec_t Tmp0, Tmp1, Numer, Denom;

		if(u < REAL(0.0))
		{
			// region 2
			Tmp0 = A01 + B0;
			Tmp1 = A11 + B1;
			if(Tmp1 > Tmp0)
			{
				Numer = Tmp1 - Tmp0;
				Denom = A00 - REAL(2.0) * A01 + A11;
				
				if(Numer >= Denom)
				{
					u = REAL(1.0);
					v = REAL(0.0);
					DistSq = A00 + REAL(2.0) * B0 + C;
				}
				else
				{
					u = Numer / Denom;
					v = REAL(1.0) - u;
					DistSq = u * (A00 * u + A01 * v + REAL(2.0) * B0) + v * (A01 * u + A11 * v + REAL(2.0) * B1) + C;
				}
			}
			else
			{
				u = REAL(0.0);
				
				if(Tmp1 <= REAL(0.0))
				{
					v = REAL(1.0);
					DistSq = A11 + REAL(2.0) * B1 + C;
				}
				else if(B1 >= REAL(0.0))
				{
					v = REAL(0.0);
					DistSq = C;
				}
				else
				{
					v = -B1 / A11;
					DistSq = B1 * v + C;
				}
			}
		}
		else if(v < REAL(0.0))
		{
			// region 6
			Tmp0 = A01 + B1;
			Tmp1 = A00 + B0;
			
			if(Tmp1 > Tmp0)
			{
				Numer = Tmp1 - Tmp0;
				Denom = A00 - REAL(2.0) * A01 + A11;
				if(Numer >= Denom)
				{
					v = REAL(1.0);
					u = REAL(0.0);
					DistSq = A11 + REAL(2.0) * B1 + C;
				}
				else
				{
					v = Numer / Denom;
					u = REAL(1.0) - v;
					DistSq =  u * (A00 * u + A01 * v + REAL(2.0) * B0) + v * (A01 * u + A11 * v + REAL(2.0) * B1) + C;
				}
			}
			else
			{
				v = REAL(0.0);
				if(Tmp1 <= REAL(0.0))
				{
					u = REAL(1.0);
					DistSq = A00 + REAL(2.0) * B0 + C;
				}
				else if(B0 >= REAL(0.0))
				{
					u = REAL(0.0);
					DistSq = C;
				}
				else
				{
					u = -B0 / A00;
					DistSq = B0 * u + C;
				}
			}
		}
		else
		{
			// region 1
			Numer = A11 + B1 - A01 - B0;
			if(Numer <= REAL(0.0))
			{
				u = REAL(0.0);
				v = REAL(1.0);
				DistSq = A11 + REAL(2.0) * B1 + C;
			}
			else
			{
				Denom = A00 - REAL(2.0) * A01 + A11;
				if(Numer >= Denom)
				{
					u = REAL(1.0);
					v = REAL(0.0);
					DistSq = A00 + REAL(2.0) * B0 + C;
				}
				else
				{
					u = Numer / Denom;
					v = REAL(1.0) - u;
					DistSq = u * (A00 * u + A01 * v + REAL(2.0) * B0) + v * (A01 * u + A11 * v + REAL(2.0) * B1) + C;
				}
			}
		}
	}

	
	dist = X_sqrt(X_fabs(DistSq));
}

bool	dPointInTriangle(const vec3_c &p, const vec3_c &v0, const vec3_c &v1, const vec3_c &v2, const vec3_c &normal, bool cw)
{
#if 1
	vec3_c vectors[3];
	vec_t total_angle = 0.0f;

	//
	// Create and normalize three vectors that radiate out from the
	// intersection point towards the triangle's three vertices.
	//
	vectors[0] = p - v0;
	vectors[0].normalize();

	vectors[1] = p - v1;
	vectors[1].normalize();

	vectors[2] = p - v2;
	vectors[2].normalize();

	//
	// We then sum together the angles that exist between each of the vectors.
	//
	// Here's how:
	//
	// 1. Use dotProduct() to get cosine of the angle between the two vectors.
	// 2. Use acos() to convert cosine back into an angle.
	// 3. Add angle to total_angle to keep track of running sum.
	//
	for(int i=0; i<3; i++)
	{
		if(cw)
			total_angle += RADTODEG(acos(vectors[i].dotProduct(vectors[(i+1)%3])));
		else
			total_angle += RADTODEG(acos(vectors[i].dotProduct(vectors[(i+2)%3])));
	}
	
	/*
	total_angle  = RADTODEG(acos(vectors[0].dotProduct(vectors[1])));
	total_angle += RADTODEG(acos(vectors[1].dotProduct(vectors[2])));
	total_angle += RADTODEG(acos(vectors[2].dotProduct(vectors[0])));
	*/

	//
	// If we are able to sum together all three angles and get 360.0, the
	// intersection point is inside the triangle.
	//
	// We can check this by taking away 6.28 radians (360 degrees) away from
	// total_angle and if we're left with 0 (allowing for some tolerance) the
	// intersection point is definitely inside the triangle.
	//
	if(X_fabs(total_angle - REAL(360.0)) < DIST_EPSILON)
		return true;

	return false;
#else
	vec_t		xt, yt;
	plane_type_e	s1, s2;
	
	vec3_c	nn(false);
	nn[0] = X_fabs(normal[0]);
	nn[0] = X_fabs(normal[0]);
	nn[0] = X_fabs(normal[0]);
	
	if((nn[0] >= nn[1]) && (nn[0] >= nn[2]))
	{
		xt = p[1];
		yt = p[2];
	
		// PLANE_ANYX
		s1 = PLANE_Y;
		s2 = PLANE_Z;
	}
	else if((nn[1] >= nn[0]) && (nn[1] >= nn[2]))
	{
		xt = p[0];
		yt = p[2];
		
		// PLANE_ANYY
		s1 = PLANE_X;
		s2 = PLANE_Z;
	}
	else
	{
		xt = p[0];
		yt = p[1];
		
		// PLANE_ANYZ
		s1 = PLANE_X;
		s2 = PLANE_Y;
	}
	
	vec_t	Ax, Ay, Bx, By;
	vec_t	s;
	
	bool front = false;
	bool back = false;
	
	Ax=v0[s1]; Bx=v1[s1];
	Ay=v0[s2]; By=v1[s2];
	
	s=((Ay-yt)*(Bx-Ax)-(Ax-xt)*(By-Ay));
	
	if(s >= 0) 
	{ 
		if(back)
			return false; 
		
		front = true;
	}
	else
	{
		if(front)
			return false;
		
		back = true;
	}
	
	Ax=v1[s1]; Bx=v2[s1];
	Ay=v1[s2]; By=v2[s2];
	
	s=((Ay-yt)*(Bx-Ax)-(Ax-xt)*(By-Ay));
	
	if(s >= 0)
	{
		if(back)
			return false; 
			
		front = true;
	}
	else
	{
		if(front)
			return false; 
		
		back = true;
	}
	
	Ax=v2[s1]; Bx=v0[s1];
	Ay=v2[s2]; By=v0[s2];
	
	s=((Ay-yt)*(Bx-Ax)-(Ax-xt)*(By-Ay));
	
	if(s >= 0)
	{
		if(back)
			return false;
		
		front = true;
	}
	else
	{
		if(front)
			return false;
		
		back = true;
	}
	
	return true;
#endif
}

bool	dCollideSpheres(const vec3_c &p1, vec_t r1, const vec3_c &p2, vec_t r2, dContactGeom &c)
{
	// printf ("d=%.2f  (%.2f %.2f %.2f) (%.2f %.2f %.2f) r1=%.2f r2=%.2f\n",
	//	  d,p1[0],p1[1],p1[2],p2[0],p2[1],p2[2],r1,r2);
	
	vec_t d = p1.distance(p2);
	
	if(d > (r1 + r2))
		return false;
		
	if(d <= 0)
	{
		c._origin = p1;
		c._normal.set(1, 0, 0);
		c._depth = r1 + r2;
	}
	else
	{
		vec_t d1 = X_recip(d);
		
		vec_t k = REAL(0.5) * (r2 - r1 - d);
		
		c._origin = p1 + (c._normal*k);
		
		c._normal[0] = (p1[0]-p2[0])*d1;
		c._normal[1] = (p1[1]-p2[1])*d1;
		c._normal[2] = (p1[2]-p2[2])*d1;
		
		c._depth = r1 + r2 - d;
	}
	
	return true;
}


void dLineClosestApproach (const dVector3 pa, const dVector3 ua,
			   const dVector3 pb, const dVector3 ub,
			   vec_t *alpha, vec_t *beta)
{
  dVector3 p;
  p[0] = pb[0] - pa[0];
  p[1] = pb[1] - pa[1];
  p[2] = pb[2] - pa[2];
  vec_t uaub = dDOT(ua,ub);
  vec_t q1 =  dDOT(ua,p);
  vec_t q2 = -dDOT(ub,p);
  vec_t d = 1-uaub*uaub;
  if (d <= REAL(0.0001)) {
    // @@@ this needs to be made more robust
    *alpha = 0;
    *beta  = 0;
  }
  else {
    d = X_recip(d);
    *alpha = (q1 + uaub*q2)*d;
    *beta  = (uaub*q1 + q2)*d;
  }
}


// given two line segments A and B with endpoints a1-a2 and b1-b2, return the
// points on A and B that are closest to each other (in cp1 and cp2).
// in the case of parallel lines where there are multiple solutions, a
// solution involving the endpoint of at least one line will be returned.
// this will work correctly for zero length lines, e.g. if a1==a2 and/or
// b1==b2.
//
// the algorithm works by applying the voronoi clipping rule to the features
// of the line segments. the three features of each line segment are the two
// endpoints and the line between them. the voronoi clipping rule states that,
// for feature X on line A and feature Y on line B, the closest points PA and
// PB between X and Y are globally the closest points if PA is in V(Y) and
// PB is in V(X), where V(X) is the voronoi region of X.

void dClosestLineSegmentPoints (const dVector3 a1, const dVector3 a2,
				const dVector3 b1, const dVector3 b2,
				dVector3 cp1, dVector3 cp2)
{
  dVector3 a1a2,b1b2,a1b1,a1b2,a2b1,a2b2,n;
  vec_t la,lb,k,da1,da2,da3,da4,db1,db2,db3,db4,det;

#define SET2(a,b) a[0]=b[0]; a[1]=b[1]; a[2]=b[2];
#define SET3(a,b,op,c) a[0]=b[0] op c[0]; a[1]=b[1] op c[1]; a[2]=b[2] op c[2];

  // check vertex-vertex features

  SET3 (a1a2,a2,-,a1);
  SET3 (b1b2,b2,-,b1);
  SET3 (a1b1,b1,-,a1);
  da1 = dDOT(a1a2,a1b1);
  db1 = dDOT(b1b2,a1b1);
  if (da1 <= 0 && db1 >= 0) {
    SET2 (cp1,a1);
    SET2 (cp2,b1);
    return;
  }

  SET3 (a1b2,b2,-,a1);
  da2 = dDOT(a1a2,a1b2);
  db2 = dDOT(b1b2,a1b2);
  if (da2 <= 0 && db2 <= 0) {
    SET2 (cp1,a1);
    SET2 (cp2,b2);
    return;
  }

  SET3 (a2b1,b1,-,a2);
  da3 = dDOT(a1a2,a2b1);
  db3 = dDOT(b1b2,a2b1);
  if (da3 >= 0 && db3 >= 0) {
    SET2 (cp1,a2);
    SET2 (cp2,b1);
    return;
  }

  SET3 (a2b2,b2,-,a2);
  da4 = dDOT(a1a2,a2b2);
  db4 = dDOT(b1b2,a2b2);
  if (da4 >= 0 && db4 <= 0) {
    SET2 (cp1,a2);
    SET2 (cp2,b2);
    return;
  }

  // check edge-vertex features.
  // if one or both of the lines has zero length, we will never get to here,
  // so we do not have to worry about the following divisions by zero.

  la = dDOT(a1a2,a1a2);
  if (da1 >= 0 && da3 <= 0) {
    k = da1 / la;
    SET3 (n,a1b1,-,k*a1a2);
    if (dDOT(b1b2,n) >= 0) {
      SET3 (cp1,a1,+,k*a1a2);
      SET2 (cp2,b1);
      return;
    }
  }

  if (da2 >= 0 && da4 <= 0) {
    k = da2 / la;
    SET3 (n,a1b2,-,k*a1a2);
    if (dDOT(b1b2,n) <= 0) {
      SET3 (cp1,a1,+,k*a1a2);
      SET2 (cp2,b2);
      return;
    }
  }

  lb = dDOT(b1b2,b1b2);
  if (db1 <= 0 && db2 >= 0) {
    k = -db1 / lb;
    SET3 (n,-a1b1,-,k*b1b2);
    if (dDOT(a1a2,n) >= 0) {
      SET2 (cp1,a1);
      SET3 (cp2,b1,+,k*b1b2);
      return;
    }
  }

  if (db3 <= 0 && db4 >= 0) {
    k = -db3 / lb;
    SET3 (n,-a2b1,-,k*b1b2);
    if (dDOT(a1a2,n) <= 0) {
      SET2 (cp1,a2);
      SET3 (cp2,b1,+,k*b1b2);
      return;
    }
  }

  // it must be edge-edge

  k = dDOT(a1a2,b1b2);
  det = la*lb - k*k;
  if (det <= 0) {
    // this should never happen, but just in case...
    SET2(cp1,a1);
    SET2(cp2,b1);
    return;
  }
  det = X_recip (det);
  vec_t alpha = (lb*da1 -  k*db1) * det;
  vec_t beta  = ( k*da1 - la*db1) * det;
  SET3 (cp1,a1,+,alpha*a1a2);
  SET3 (cp2,b1,+,beta*b1b2);

# undef SET2
# undef SET3
}


// a simple root finding algorithm is used to find the value of 't' that
// satisfies:
//		d|D(t)|^2/dt = 0
// where:
//		|D(t)| = |p(t)-b(t)|
// where p(t) is a point on the line parameterized by t:
//		p(t) = p1 + t*(p2-p1)
// and b(t) is that same point clipped to the boundary of the box. in box-
// relative coordinates d|D(t)|^2/dt is the sum of three x,y,z components
// each of which looks like this:
//
//	    t_lo     /
//	      ______/    -->t
//	     /     t_hi
//	    /
//
// t_lo and t_hi are the t values where the line passes through the planes
// corresponding to the sides of the box. the algorithm computes d|D(t)|^2/dt
// in a piecewise fashion from t=0 to t=1, stopping at the point where
// d|D(t)|^2/dt crosses from negative to positive.

void dClosestLineBoxPoints (const dVector3 p1, const dVector3 p2,
			    const dVector3 c, const matrix_c &R,
			    const dVector3 side,
			    dVector3 lret, dVector3 bret)
{
  int i;

  // compute the start and delta of the line p1-p2 relative to the box.
  // we will do all subsequent computations in this box-relative coordinate
  // system. we have to do a translation and rotation for each point.
  dVector3 tmp,s,v;
  tmp[0] = p1[0] - c[0];
  tmp[1] = p1[1] - c[1];
  tmp[2] = p1[2] - c[2];
  
  dMULTIPLY1_331(s, (vec_t*)R, tmp);
  
  tmp[0] = p2[0] - p1[0];
  tmp[1] = p2[1] - p1[1];
  tmp[2] = p2[2] - p1[2];
  
  dMULTIPLY1_331(v, (vec_t*)R, tmp);

  // mirror the line so that v has all components >= 0
  dVector3 sign;
  for (i=0; i<3; i++) {
    if (v[i] < 0) {
      s[i] = -s[i];
      v[i] = -v[i];
      sign[i] = -1;
    }
    else sign[i] = 1;
  }

  // compute v^2
  dVector3 v2;
  v2[0] = v[0]*v[0];
  v2[1] = v[1]*v[1];
  v2[2] = v[2]*v[2];

  // compute the half-sides of the box
  vec_t h[3];
  h[0] = REAL(0.5) * side[0];
  h[1] = REAL(0.5) * side[1];
  h[2] = REAL(0.5) * side[2];

  // region is -1,0,+1 depending on which side of the box planes each
  // coordinate is on. tanchor is the next t value at which there is a
  // transition, or the last one if there are no more.
  int region[3];
  vec_t tanchor[3];

  // find the region and tanchor values for p1
  for (i=0; i<3; i++) {
    if (v[i] > 0) {
      if (s[i] < -h[i]) {
	region[i] = -1;
	tanchor[i] = (-h[i]-s[i])/v[i];
      }
      else {
	region[i] = (s[i] > h[i]);
	tanchor[i] = (h[i]-s[i])/v[i];
      }
    }
    else {
      region[i] = 0;
      tanchor[i] = 2;		// this will never be a valid tanchor
    }
  }

  // compute d|d|^2/dt for t=0. if it's >= 0 then p1 is the closest point
  vec_t t=0;
  vec_t dd2dt = 0;
  for (i=0; i<3; i++) dd2dt -= (region[i] ? v2[i] : 0) * tanchor[i];
  if (dd2dt >= 0) goto got_answer;

  do {
    // find the point on the line that is at the next clip plane boundary
    vec_t next_t = 1;
    for (i=0; i<3; i++) {
      if (tanchor[i] > t && tanchor[i] < 1 && tanchor[i] < next_t)
        next_t = tanchor[i];
    }

    // compute d|d|^2/dt for the next t
    vec_t next_dd2dt = 0;
    for (i=0; i<3; i++) {
      next_dd2dt += (region[i] ? v2[i] : 0) * (next_t - tanchor[i]);
    }

    // if the sign of d|d|^2/dt has changed, solution = the crossover point
    if (next_dd2dt >= 0) {
      vec_t m = (next_dd2dt-dd2dt)/(next_t - t);
      t -= dd2dt/m;
      goto got_answer;
    }

    // advance to the next anchor point / region
    for (i=0; i<3; i++) {
      if (tanchor[i] == next_t) {
	tanchor[i] = (h[i]-s[i])/v[i];
	region[i]++;
      }
    }
    t = next_t;
    dd2dt = next_dd2dt;
  }
  while (t < 1);
  t = 1;

  got_answer:

  // compute closest point on the line
  for (i=0; i<3; i++) lret[i] = p1[i] + t*tmp[i];	// note: tmp=p2-p1

  // compute closest point on the box
  for (i=0; i<3; i++)
  {
    tmp[i] = sign[i] * (s[i] + t*v[i]);
    if (tmp[i] < -h[i]) tmp[i] = -h[i];
    else if (tmp[i] > h[i]) tmp[i] = h[i];
  }
  
  dMULTIPLY0_331(s, (vec_t*)R, tmp);
  
  for (i=0; i<3; i++) bret[i] = s[i] + c[i];
}


// given boxes (p1,R1,side1) and (p1,R1,side1), return 1 if they intersect
// or 0 if not.

int dBoxTouchesBox (const dVector3 p1, const matrix_c &R1,
		    const dVector3 side1, const dVector3 p2,
		    const matrix_c &R2, const dVector3 side2)
{
  // two boxes are disjoint if (and only if) there is a separating axis
  // perpendicular to a face from one box or perpendicular to an edge from
  // either box. the following tests are derived from:
  //    "OBB Tree: A Hierarchical Structure for Rapid Interference Detection",
  //    S.Gottschalk, M.C.Lin, D.Manocha., Proc of ACM Siggraph 1996.

  // Rij is R1'*R2, i.e. the relative rotation between R1 and R2.
  // Qij is abs(Rij)
  dVector3 p,pp;
  vec_t A1,A2,A3,B1,B2,B3,R11,R12,R13,R21,R22,R23,R31,R32,R33,
    Q11,Q12,Q13,Q21,Q22,Q23,Q31,Q32,Q33;

  // get vector from centers of box 1 to box 2, relative to box 1
  p[0] = p2[0] - p1[0];
  p[1] = p2[1] - p1[1];
  p[2] = p2[2] - p1[2];
  
  dMULTIPLY1_331(pp, (vec_t*)R1, p);		// get pp = p relative to body 1

  // get side lengths / 2
  A1 = side1[0]*REAL(0.5); A2 = side1[1]*REAL(0.5); A3 = side1[2]*REAL(0.5);
  B1 = side2[0]*REAL(0.5); B2 = side2[1]*REAL(0.5); B3 = side2[2]*REAL(0.5);

  // for the following tests, excluding computation of Rij, in the worst case,
  // 15 compares, 60 adds, 81 multiplies, and 24 absolutes.
  // notation: R1=[u1 u2 u3], R2=[v1 v2 v3]

  // separating axis = u1,u2,u3
  R11 = dDOT44(R1+0,R2+0); R12 = dDOT44(R1+0,R2+1); R13 = dDOT44(R1+0,R2+2);
  Q11 = X_fabs(R11); Q12 = X_fabs(R12); Q13 = X_fabs(R13);
  if (X_fabs(pp[0]) > (A1 + B1*Q11 + B2*Q12 + B3*Q13)) return 0;
  R21 = dDOT44(R1+1,R2+0); R22 = dDOT44(R1+1,R2+1); R23 = dDOT44(R1+1,R2+2);
  Q21 = X_fabs(R21); Q22 = X_fabs(R22); Q23 = X_fabs(R23);
  if (X_fabs(pp[1]) > (A2 + B1*Q21 + B2*Q22 + B3*Q23)) return 0;
  R31 = dDOT44(R1+2,R2+0); R32 = dDOT44(R1+2,R2+1); R33 = dDOT44(R1+2,R2+2);
  Q31 = X_fabs(R31); Q32 = X_fabs(R32); Q33 = X_fabs(R33);
  if (X_fabs(pp[2]) > (A3 + B1*Q31 + B2*Q32 + B3*Q33)) return 0;

  // separating axis = v1,v2,v3
  if (X_fabs(dDOT41(R2+0,p)) > (A1*Q11 + A2*Q21 + A3*Q31 + B1)) return 0;
  if (X_fabs(dDOT41(R2+1,p)) > (A1*Q12 + A2*Q22 + A3*Q32 + B2)) return 0;
  if (X_fabs(dDOT41(R2+2,p)) > (A1*Q13 + A2*Q23 + A3*Q33 + B3)) return 0;

  // separating axis = u1 x (v1,v2,v3)
  if (X_fabs(pp[2]*R21-pp[1]*R31) > A2*Q31 + A3*Q21 + B2*Q13 + B3*Q12) return 0;
  if (X_fabs(pp[2]*R22-pp[1]*R32) > A2*Q32 + A3*Q22 + B1*Q13 + B3*Q11) return 0;
  if (X_fabs(pp[2]*R23-pp[1]*R33) > A2*Q33 + A3*Q23 + B1*Q12 + B2*Q11) return 0;

  // separating axis = u2 x (v1,v2,v3)
  if (X_fabs(pp[0]*R31-pp[2]*R11) > A1*Q31 + A3*Q11 + B2*Q23 + B3*Q22) return 0;
  if (X_fabs(pp[0]*R32-pp[2]*R12) > A1*Q32 + A3*Q12 + B1*Q23 + B3*Q21) return 0;
  if (X_fabs(pp[0]*R33-pp[2]*R13) > A1*Q33 + A3*Q13 + B1*Q22 + B2*Q21) return 0;

  // separating axis = u3 x (v1,v2,v3)
  if (X_fabs(pp[1]*R11-pp[0]*R21) > A1*Q21 + A2*Q11 + B2*Q33 + B3*Q32) return 0;
  if (X_fabs(pp[1]*R12-pp[0]*R22) > A1*Q22 + A2*Q12 + B1*Q33 + B3*Q31) return 0;
  if (X_fabs(pp[1]*R13-pp[0]*R23) > A1*Q23 + A2*Q13 + B1*Q32 + B2*Q31) return 0;

  return 1;
}

//****************************************************************************
// other utility functions

void dInfiniteAABB (dxGeom *geom, vec_t aabb[6])
{
  aabb[0] = -X_infinity;
  aabb[1] = X_infinity;
  aabb[2] = -X_infinity;
  aabb[3] = X_infinity;
  aabb[4] = -X_infinity;
  aabb[5] = X_infinity;
}
