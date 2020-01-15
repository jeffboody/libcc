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
#include "cc_quaternion.h"
#include "cc_vec4f.h"

/***********************************************************
* private                                                  *
***********************************************************/

static void cc_quaternion_normalize(cc_quaternion_t* self)
{
	ASSERT(self);

	return cc_vec4f_normalize((cc_vec4f_t*) self);
}

static void
cc_quaternion_normalize_copy(const cc_quaternion_t* self,
                             cc_quaternion_t* copy)
{
	ASSERT(self);
	ASSERT(copy);

	return cc_vec4f_normalize_copy((const cc_vec4f_t*) self,
	                               (cc_vec4f_t*) copy);
}

/***********************************************************
* public                                                   *
***********************************************************/

void cc_quaternion_load(cc_quaternion_t* self,
                        float x, float y, float z, float s)
{
	ASSERT(self);

	cc_vec3f_load(&self->v, x, y, z);
	self->s = s;
}

void cc_quaternion_loadaxis(cc_quaternion_t* self, float a,
                            float x, float y, float z)
{
	ASSERT(self);

	cc_vec3f_t v;
	cc_vec3f_load(&v, x, y, z);
	cc_vec3f_normalize(&v);

	float a2     = (a/2.0f)*(M_PI/180.0f);
	float sin_a2 = sinf(a2);
	float cos_a2 = cosf(a2);

	// already has unit norm
	self->v.x = v.x*sin_a2;
	self->v.y = v.y*sin_a2;
	self->v.z = v.z*sin_a2;
	self->s   = cos_a2;
}

void cc_quaternion_loadeuler(cc_quaternion_t* self,
                             float rx, float ry, float rz)
{
	ASSERT(self);

	#if 1
		float rx2 = (rx/2.0f)*(M_PI/180.0f);
		float ry2 = (ry/2.0f)*(M_PI/180.0f);
		float rz2 = (rz/2.0f)*(M_PI/180.0f);

		float sx = sinf(rx2);
		float sy = sinf(ry2);
		float sz = sinf(rz2);
		float cx = cosf(rx2);
		float cy = cosf(ry2);
		float cz = cosf(rz2);

		self->v.x = sx*cy*cz - cx*sy*sz;
		self->v.y = cx*sy*cz + sx*cy*sz;
		self->v.z = cx*cy*sz - sx*sy*cz;
		self->s   = cx*cy*cz + sx*sy*sz;

		cc_quaternion_normalize(self);
	#else
		cc_quaternion_t qx;
		cc_quaternion_t qy;
		cc_quaternion_t qz;
		cc_quaternion_t qt;
		cc_quaternion_loadaxis(&qx, rx, 1.0f, 0.0f, 0.0f);
		cc_quaternion_loadaxis(&qy, ry, 0.0f, 1.0f, 0.0f);
		cc_quaternion_loadaxis(&qz, rz, 0.0f, 0.0f, 1.0f);
		cc_quaternion_rotateq_copy(&qx, &qy, &qt);
		cc_quaternion_rotateq_copy(&qt, &qz, self);
		cc_quaternion_normalize(self);
	#endif
}

void cc_quaternion_loadaxisangle(cc_quaternion_t* self,
                                 float ax, float ay,
                                 float az,
                                 float angle)
{
	ASSERT(self);

	// https://developer.android.com/guide/topics/sensors/sensors_motion.html
	// http://www.flipcode.com/documents/matrfaq.html#Q56

	float sin_a = sin((M_PI/180.0f)*angle/2.0f);
	float cos_a = cos((M_PI/180.0f)*angle/2.0f);

	cc_quaternion_load(self, ax*sin_a, ay*sin_a, az*sin_a,
	                   cos_a);
	cc_quaternion_normalize(self);
}

void cc_quaternion_copy(const cc_quaternion_t* self,
                        cc_quaternion_t* q)
{
	ASSERT(self);
	ASSERT(q);

	cc_quaternion_load(q, self->v.x, self->v.y,
	                   self->v.z, self->s);
}

void cc_quaternion_identity(cc_quaternion_t* self)
{
	ASSERT(self);

	cc_quaternion_load(self, 0.0f, 0.0f, 0.0f, 1.0f);
}

void cc_quaternion_inverse(cc_quaternion_t* self)
{
	ASSERT(self);

	cc_quaternion_load(self, -self->v.x, -self->v.y,
	                   -self->v.z, self->s);
}

void cc_quaternion_inverse_copy(const cc_quaternion_t* self,
                                cc_quaternion_t* q)
{
	ASSERT(self);
	ASSERT(q);

	cc_quaternion_load(q, -self->v.x, -self->v.y,
	                   -self->v.z, self->s);
}

void cc_quaternion_rotateq(cc_quaternion_t* self,
                           const cc_quaternion_t* q)
{
	ASSERT(self);
	ASSERT(q);

	// reminder - quaternion multiplication is non-commutative
	const cc_vec3f_t* av = &self->v;
	const cc_vec3f_t* qv = &q->v;
	float             as = self->s;
	float             qs = q->s;

	#if 1
		cc_quaternion_t   copy;
		copy.v.x = as*qv->x + av->x*qs    + av->y*qv->z - av->z*qv->y;
		copy.v.y = as*qv->y + av->y*qs    + av->z*qv->x - av->x*qv->z;
		copy.v.z = as*qv->z + av->z*qs    + av->x*qv->y - av->y*qv->x;
		copy.s   = as*qs    - av->x*qv->x - av->y*qv->y - av->z*qv->z;

		cc_quaternion_normalize_copy(&copy, self);
	#else
		// website seems to have a bug for s
		// http://www.flipcode.com/documents/matrfaq.html
		cc_vec3f_t a;
		cc_vec3f_t b;
		cc_vec3f_t c;
		float      s = (as*qs) - cc_vec3f_dot(av, qv);
		cc_vec3f_cross_copy(av, qv, &a);
		cc_vec3f_muls_copy(av, qs, &b);
		cc_vec3f_muls_copy(qv, as, &c);
		cc_vec3f_addv(&a, &b);
		cc_vec3f_addv(&a, &c);
		cc_quaternion_load(self, a.x, a.y, a.z, s);
		cc_quaternion_normalize(self);
	#endif
}

void cc_quaternion_rotateq_copy(const cc_quaternion_t* self,
                                 const cc_quaternion_t* q,
                                 cc_quaternion_t* copy)
{
	ASSERT(self);
	ASSERT(q);
	ASSERT(copy);

	// reminder - quaternion multiplication is non-commutative
	const cc_vec3f_t* av = &self->v;
	const cc_vec3f_t* qv = &q->v;
	float             as = self->s;
	float             qs = q->s;

	#if 1
		copy->v.x = as*qv->x + av->x*qs    + av->y*qv->z - av->z*qv->y;
		copy->v.y = as*qv->y + av->y*qs    + av->z*qv->x - av->x*qv->z;
		copy->v.z = as*qv->z + av->z*qs    + av->x*qv->y - av->y*qv->x;
		copy->s   = as*qs    - av->x*qv->x - av->y*qv->y - av->z*qv->z;

		cc_quaternion_normalize(copy);
	#else
		// website seems to have a bug for s
		// http://www.flipcode.com/documents/matrfaq.html
		cc_vec3f_t a;
		cc_vec3f_t b;
		cc_vec3f_t c;
		float      s = (as*qs) - cc_vec3f_dot(av, qv);
		cc_vec3f_cross_copy(av, qv, &a);
		cc_vec3f_muls_copy(av, qs, &b);
		cc_vec3f_muls_copy(qv, as, &c);
		cc_vec3f_addv(&a, &b);
		cc_vec3f_addv(&a, &c);
		cc_quaternion_load(copy, a.x, a.y, a.z, s);
		cc_quaternion_normalize(copy);
	#endif
}

void cc_quaternion_slerp(const cc_quaternion_t* a,
                         const cc_quaternion_t* b,
                         float t, cc_quaternion_t* c)
{
	ASSERT(a);
	ASSERT(b);
	ASSERT(c);

	// https://en.wikipedia.org/wiki/Slerp

	cc_quaternion_t aa;
	cc_quaternion_t bb;
	cc_quaternion_normalize_copy(a, &aa);
	cc_quaternion_normalize_copy(b, &bb);
	float dot = cc_vec4f_dot((const cc_vec4f_t*) &aa,
	                         (const cc_vec4f_t*) &bb);
	if(dot < 0.0f)
	{
		cc_vec4f_muls((cc_vec4f_t*) &bb, -1.0f);
		dot = -dot;
	}

	const float thresh = 0.9995f;
	if(dot > thresh)
	{
		// linearly interpolate and normalize the result
		cc_vec4f_subv_copy((const cc_vec4f_t*) &bb,
		                   (const cc_vec4f_t*) &aa,
		                   (cc_vec4f_t*) c);
		cc_vec4f_muls((cc_vec4f_t*) c, t);
		cc_vec4f_addv((cc_vec4f_t*) c,
		              (const cc_vec4f_t*) &aa);
		cc_quaternion_normalize(c);
		return;
	}

	// interpolate between quaternions
	float theta_0 = acosf(dot);
	float theta   = theta_0*t;
	float s0      = cosf(theta) - dot*sinf(theta)/sin(theta_0);
	float s1      = sinf(theta)/sinf(theta_0);
	cc_vec4f_muls((cc_vec4f_t*) &aa, s0);
	cc_vec4f_muls((cc_vec4f_t*) &bb, s1);
	cc_vec4f_addv_copy((const cc_vec4f_t*) &aa,
	                   (const cc_vec4f_t*) &bb,
	                   (cc_vec4f_t*) c);
	cc_quaternion_normalize(c);
}

float cc_quaternion_compare(const cc_quaternion_t* a,
                            const cc_quaternion_t* b)
{
	ASSERT(a);
	ASSERT(b);

	float ds = b->s   - a->s;
	float dx = b->v.x - a->v.x;
	float dy = b->v.y - a->v.y;
	float dz = b->v.z - a->v.z;
	return sqrtf(ds*ds + dx*dx + dy*dy + dz*dz);
}
