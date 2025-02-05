/*
 * Copyright (c) 2013 Jeff Boody
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
#include "cc_ray3f.h"

/***********************************************************
* public                                                   *
***********************************************************/

void cc_ray3f_load(cc_ray3f_t* self,
                   float px, float py, float pz,
                   float vx, float vy, float vz)
{
	ASSERT(self);

	cc_vec3f_load(&self->p, px, py, pz);
	cc_vec3f_load(&self->v, vx, vy, vz);
	cc_vec3f_normalize(&self->v);
}

int cc_ray3f_hitsphere(const cc_ray3f_t* self,
                       const cc_sphere_t* s)
{
	ASSERT(self);
	ASSERT(s);

	cc_vec3f_t v;
	cc_vec3f_t p;
	cc_vec3f_subv_copy(&s->c, &self->p, &v);
	if(cc_vec3f_mag(&v) <= s->r)
	{
		// ray origin inside sphere
		return 1;
	}

	float t = cc_vec3f_dot(&self->v, &v);
	if(t < 0.0f)
	{
		// ray pointing in wrong direction
		return 0;
	}

	// distance to the nearest point on the ray
	cc_vec3f_muls_copy(&self->v, t, &v);
	cc_vec3f_addv_copy(&v, &self->p, &p);
	cc_vec3f_subv_copy(&s->c, &p, &v);
	float d = cc_vec3f_mag(&v);

	// does ray pass through sphere
	return (d < s->r) ? 1 : 0;
}

int cc_ray3f_intersect(const cc_ray3f_t* ray,
                       const cc_sphere_t* sphere,
                       float* near,
                       float* far)
{
	ASSERT(ray);
	ASSERT(sphere);
	ASSERT(near);
	ASSERT(far);

	// https://www.perplexity.ai
	// docs/ray_sphere_intersect.c

	cc_vec3f_t pc =
	{
		.x = ray->p.x - sphere->c.x,
		.y = ray->p.y - sphere->c.y,
		.z = ray->p.z - sphere->c.z,
	};
	float a = ray->v.x*ray->v.x + ray->v.y*ray->v.y +
	          ray->v.z*ray->v.z;
	float b = 2.0f*(pc.x*ray->v.x + pc.y*ray->v.y +
	          pc.z*ray->v.z);
	float c = pc.x*pc.x + pc.y*pc.y + pc.z*pc.z -
	          sphere->r*sphere->r;

	float discriminant = b*b - 4*a*c;

	// check for ray-sphere intersection
	if (discriminant < 0)
	{
		return 0;
	}

	// compute the intersection distance
	float t1 = (-b - sqrt(discriminant))/(2.0f*a);
	float t2 = (-b + sqrt(discriminant))/(2.0f*a);

	// determine the nearest intersection
	if (t1 > t2)
	{
		float temp = t1;
		t1 = t2;
		t2 = temp;
	}

	// check if the ray origin is inside the sphere
	if (t1 < 0.0f)
	{
		*near = 0.0f;
		*far  = t2;
		return 1;
	}

	// ray intersects the sphere twice
	*near = t1;
	*far  = t2;
	return 2;
}

void
cc_ray3f_getpoint(const cc_ray3f_t* self, float s,
                  cc_vec3f_t* p)
{
	ASSERT(self);
	ASSERT(p);

	p->x = self->p.x + s*self->v.x;
	p->y = self->p.y + s*self->v.y;
	p->z = self->p.z + s*self->v.z;
}
