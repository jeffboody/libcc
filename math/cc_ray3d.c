/*
 * Copyright (c) 2022 Jeff Boody
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <math.h>
#include <stdlib.h>

#define LOG_TAG "cc"
#include "../cc_log.h"
#include "cc_ray3d.h"

/***********************************************************
* public                                                   *
***********************************************************/

void cc_ray3d_load(cc_ray3d_t* self,
                   double px, double py, double pz,
                   double vx, double vy, double vz)
{
	ASSERT(self);

	cc_vec3d_load(&self->p, px, py, pz);
	cc_vec3d_load(&self->v, vx, vy, vz);
	cc_vec3d_normalize(&self->v);
}

int cc_ray3d_hitsphere(const cc_ray3d_t* self,
                       const cc_sphere3d_t* s)
{
	ASSERT(self);
	ASSERT(s);

	cc_vec3d_t v;
	cc_vec3d_t p;
	cc_vec3d_subv_copy(&s->c, &self->p, &v);
	if(cc_vec3d_mag(&v) <= s->r)
	{
		// ray origin inside sphere
		return 1;
	}

	double t = cc_vec3d_dot(&self->v, &v);
	if(t < 0.0)
	{
		// ray pointing in wrong direction
		return 0;
	}

	// distance to the nearest point on the ray
	cc_vec3d_muls_copy(&self->v, t, &v);
	cc_vec3d_addv_copy(&v, &self->p, &p);
	cc_vec3d_subv_copy(&s->c, &p, &v);
	double d = cc_vec3d_mag(&v);

	// does ray pass through sphere
	return (d < s->r) ? 1 : 0;
}

int cc_ray3d_intersect(const cc_ray3d_t* ray,
                       const cc_sphere3d_t* sphere,
                       double* near,
                       double* far)
{
	ASSERT(ray);
	ASSERT(sphere);
	ASSERT(near);
	ASSERT(far);

	// https://www.perplexity.ai
	// docs/ray_sphere_intersect.c

	cc_vec3d_t pc =
	{
		.x = ray->p.x - sphere->c.x,
		.y = ray->p.y - sphere->c.y,
		.z = ray->p.z - sphere->c.z,
	};
	double a = ray->v.x*ray->v.x + ray->v.y*ray->v.y +
	          ray->v.z*ray->v.z;
	double b = 2.0*(pc.x*ray->v.x + pc.y*ray->v.y +
	          pc.z*ray->v.z);
	double c = pc.x*pc.x + pc.y*pc.y + pc.z*pc.z -
	          sphere->r*sphere->r;

	double discriminant = b*b - 4*a*c;

	// check for ray-sphere intersection
	if (discriminant < 0)
	{
		return 0;
	}

	// compute the intersection distance
	double t1 = (-b - sqrt(discriminant))/(2.0*a);
	double t2 = (-b + sqrt(discriminant))/(2.0*a);

	// determine the nearest intersection
	if (t1 > t2)
	{
		double temp = t1;
		t1 = t2;
		t2 = temp;
	}

	// check the ray origin
	if (t1 < 0.0)
	{
		if (t2 < 0.0)
		{
			// ray origin is outside of the sphere but
			// ray does not have forward intersections
			return 0;
		}

		// ray origin is inside of the sphere and
		// ray has one forward intersection
		*near = 0.0;
		*far  = t2;
		return 1;
	}

	// ray origin is outside of the sphere and
	// ray has two forward intersections
	*near = t1;
	*far  = t2;
	return 2;
}

void
cc_ray3d_getpoint(const cc_ray3d_t* self, double s,
                  cc_vec3d_t* p)
{
	ASSERT(self);
	ASSERT(p);

	p->x = self->p.x + s*self->v.x;
	p->y = self->p.y + s*self->v.y;
	p->z = self->p.z + s*self->v.z;
}
