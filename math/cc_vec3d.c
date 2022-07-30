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
#include "../cc_memory.h"
#include "cc_vec3d.h"

/***********************************************************
* public                                                   *
***********************************************************/

cc_vec3d_t* cc_vec3d_new(double x, double y, double z)
{
	cc_vec3d_t* self = (cc_vec3d_t*)
	                   MALLOC(sizeof(cc_vec3d_t));
	if(self == NULL)
	{
		LOGE("MALLOC failed");
		return NULL;
	}

	self->x = x;
	self->y = y;
	self->z = z;
	return self;
}

void cc_vec3d_delete(cc_vec3d_t** _self)
{
	ASSERT(_self);

	cc_vec3d_t* self = *_self;
	if(self)
	{
		FREE(self);
		*_self = NULL;
	}
}

void
cc_vec3d_load(cc_vec3d_t* self,
              double x, double y, double z)
{
	ASSERT(self);

	self->x = x;
	self->y = y;
	self->z = z;
}

void cc_vec3d_copy(const cc_vec3d_t* self,
                   cc_vec3d_t* copy)
{
	ASSERT(self);
	ASSERT(copy);

	copy->x = self->x;
	copy->y = self->y;
	copy->z = self->z;
}

int cc_vec3d_equals(const cc_vec3d_t* self,
                    const cc_vec3d_t* v)
{
	ASSERT(self);
	ASSERT(v);

	if((self->x == v->x) &&
	   (self->y == v->y) &&
	   (self->z == v->z))
	{
		return 1;
	}

	return 0;
}

double cc_vec3d_mag(const cc_vec3d_t* self)
{
	ASSERT(self);

	return sqrtf(self->x*self->x + self->y*self->y +
	             self->z*self->z);
}

void cc_vec3d_addv(cc_vec3d_t* self,
                   const cc_vec3d_t* v)
{
	ASSERT(self);
	ASSERT(v);

	self->x += v->x;
	self->y += v->y;
	self->z += v->z;
}

void cc_vec3d_addv_copy(const cc_vec3d_t* self,
                        const cc_vec3d_t* v,
                        cc_vec3d_t* copy)
{
	ASSERT(self);
	ASSERT(v);
	ASSERT(copy);

	copy->x = self->x + v->x;
	copy->y = self->y + v->y;
	copy->z = self->z + v->z;
}

void cc_vec3d_adds(cc_vec3d_t* self, double s)
{
	ASSERT(self);

	self->x += s;
	self->y += s;
	self->z += s;
}

void cc_vec3d_adds_copy(const cc_vec3d_t* self, double s,
                        cc_vec3d_t* copy)
{
	ASSERT(self);
	ASSERT(copy);

	copy->x = self->x + s;
	copy->y = self->y + s;
	copy->z = self->z + s;
}

void cc_vec3d_subv(cc_vec3d_t* self, const cc_vec3d_t* v)
{
	ASSERT(self);
	ASSERT(v);

	self->x -= v->x;
	self->y -= v->y;
	self->z -= v->z;
}

void cc_vec3d_subv_copy(const cc_vec3d_t* self,
                        const cc_vec3d_t* v,
                        cc_vec3d_t* copy)
{
	ASSERT(self);
	ASSERT(v);
	ASSERT(copy);

	copy->x = self->x - v->x;
	copy->y = self->y - v->y;
	copy->z = self->z - v->z;
}

void cc_vec3d_mulv(cc_vec3d_t* self, const cc_vec3d_t* v)
{
	ASSERT(self);
	ASSERT(v);

	self->x *= v->x;
	self->y *= v->y;
	self->z *= v->z;
}

void cc_vec3d_mulv_copy(const cc_vec3d_t* self,
                        const cc_vec3d_t* v,
                        cc_vec3d_t* copy)
{
	ASSERT(self);
	ASSERT(v);
	ASSERT(copy);

	copy->x = self->x*v->x;
	copy->y = self->y*v->y;
	copy->z = self->z*v->z;
}

void cc_vec3d_muls(cc_vec3d_t* self, double s)
{
	ASSERT(self);

	self->x *= s;
	self->y *= s;
	self->z *= s;
}

void cc_vec3d_muls_copy(const cc_vec3d_t* self, double s,
                        cc_vec3d_t* copy)
{
	ASSERT(self);
	ASSERT(copy);

	copy->x = s*self->x;
	copy->y = s*self->y;
	copy->z = s*self->z;
}

void cc_vec3d_normalize(cc_vec3d_t* self)
{
	ASSERT(self);

	double mag = cc_vec3d_mag(self);
	cc_vec3d_muls(self, 1.0f/mag);
}

void cc_vec3d_normalize_copy(const cc_vec3d_t* self,
                             cc_vec3d_t* copy)
{
	ASSERT(self);
	ASSERT(copy);

	double mag = cc_vec3d_mag(self);
	cc_vec3d_muls_copy(self, 1.0f/mag, copy);
}

double
cc_vec3d_dot(const cc_vec3d_t* a, const cc_vec3d_t* b)
{
	ASSERT(a);
	ASSERT(b);

	return a->x*b->x + a->y*b->y + a->z*b->z;
}

double
cc_vec3d_distance(const cc_vec3d_t* a, const cc_vec3d_t* b)
{
	ASSERT(a);
	ASSERT(b);

	cc_vec3d_t v;
	cc_vec3d_subv_copy(a, b, &v);
	return cc_vec3d_mag(&v);
}

void cc_vec3d_cross(cc_vec3d_t* self, const cc_vec3d_t* v)
{
	ASSERT(self);
	ASSERT(v);

	cc_vec3d_t copy;
	cc_vec3d_cross_copy(self, v, &copy);
	cc_vec3d_copy(&copy, self);
}

void cc_vec3d_cross_copy(const cc_vec3d_t* self,
                         const cc_vec3d_t* v,
                         cc_vec3d_t* copy)
{
	ASSERT(self);
	ASSERT(v);
	ASSERT(copy);

	const cc_vec3d_t* a = self;
	const cc_vec3d_t* b = v;
	copy->x =  (a->y*b->z) - (b->y*a->z);
	copy->y = -(a->x*b->z) + (b->x*a->z);
	copy->z =  (a->x*b->y) - (b->x*a->y);
}
