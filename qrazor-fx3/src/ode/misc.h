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

/* miscellaneous math functions. these are mostly useful for testing */

#ifndef _ODE_MISC_H_
#define _ODE_MISC_H_

#include "common.h"

/* print out a matrix */
#ifdef __cplusplus
void dPrintMatrix (const vec_t *A, int n, int m, char *fmt = "%10.4f ",
		   FILE *f=stdout);
#else
void dPrintMatrix (const vec_t *A, int n, int m, char *fmt, FILE *f);
#endif

/* make a random vector with entries between +/- range. A has n elements. */
void dMakeRandomVector (vec_t *A, int n, vec_t range);

/* make a random matrix with entries between +/- range. A has size n*m. */
void dMakeRandomMatrix (vec_t *A, int n, int m, vec_t range);

/* clear the upper triangle of a square matrix */
void dClearUpperTriangle (vec_t *A, int n);

/* return the maximum element difference between the two n*m matrices */
vec_t dMaxDifference (const vec_t *A, const vec_t *B, int n, int m);

/* return the maximum element difference between the lower triangle of two
 * n*n matrices */
vec_t dMaxDifferenceLowerTriangle (const vec_t *A, const vec_t *B, int n);

#endif
