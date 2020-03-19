/*
 * Copyright (c) 2009-2010 Jeff Boody
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
#include "cc_vec3f.h"

/***********************************************************
* public                                                   *
***********************************************************/

cc_vec3f_t* cc_vec3f_new(float x, float y, float z)
{
	cc_vec3f_t* self = (cc_vec3f_t*)
	                   MALLOC(sizeof(cc_vec3f_t));
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

void cc_vec3f_delete(cc_vec3f_t** _self)
{
	ASSERT(_self);

	cc_vec3f_t* self = *_self;
	if(self)
	{
		FREE(self);
		*_self = NULL;
	}
}

void cc_vec3f_load(cc_vec3f_t* self, float x, float y,
                   float z)
{
	ASSERT(self);

	self->x = x;
	self->y = y;
	self->z = z;
}

void cc_vec3f_copy(const cc_vec3f_t* self,
                   cc_vec3f_t* copy)
{
	ASSERT(self);
	ASSERT(copy);

	copy->x = self->x;
	copy->y = self->y;
	copy->z = self->z;
}

int cc_vec3f_equals(const cc_vec3f_t* self,
                    const cc_vec3f_t* v)
{
	ASSERT(self);
	ASSERT(copy);

	if((self->x == v->x) &&
	   (self->y == v->y) &&
	   (self->z == v->z))
	{
		return 1;
	}

	return 0;
}

float cc_vec3f_mag(const cc_vec3f_t* self)
{
	ASSERT(self);

	return sqrtf(self->x*self->x + self->y*self->y +
	             self->z*self->z);
}

void cc_vec3f_addv(cc_vec3f_t* self,
                   const cc_vec3f_t* v)
{
	ASSERT(self);
	ASSERT(v);

	self->x += v->x;
	self->y += v->y;
	self->z += v->z;
}

void cc_vec3f_addv_copy(const cc_vec3f_t* self,
                        const cc_vec3f_t* v,
                        cc_vec3f_t* copy)
{
	ASSERT(self);
	ASSERT(v);
	ASSERT(copy);

	copy->x = self->x + v->x;
	copy->y = self->y + v->y;
	copy->z = self->z + v->z;
}

void cc_vec3f_adds(cc_vec3f_t* self, float s)
{
	ASSERT(self);

	self->x += s;
	self->y += s;
	self->z += s;
}

void cc_vec3f_adds_copy(const cc_vec3f_t* self, float s,
                        cc_vec3f_t* copy)
{
	ASSERT(self);
	ASSERT(copy);

	copy->x = self->x + s;
	copy->y = self->y + s;
	copy->z = self->z + s;
}

void cc_vec3f_subv(cc_vec3f_t* self, const cc_vec3f_t* v)
{
	ASSERT(self);
	ASSERT(v);

	self->x -= v->x;
	self->y -= v->y;
	self->z -= v->z;
}

void cc_vec3f_subv_copy(const cc_vec3f_t* self,
                        const cc_vec3f_t* v,
                        cc_vec3f_t* copy)
{
	ASSERT(self);
	ASSERT(v);
	ASSERT(copy);

	copy->x = self->x - v->x;
	copy->y = self->y - v->y;
	copy->z = self->z - v->z;
}

void cc_vec3f_mulv(cc_vec3f_t* self, const cc_vec3f_t* v)
{
	ASSERT(self);
	ASSERT(v);

	self->x *= v->x;
	self->y *= v->y;
	self->z *= v->z;
}

void cc_vec3f_mulv_copy(const cc_vec3f_t* self,
                        const cc_vec3f_t* v,
                        cc_vec3f_t* copy)
{
	ASSERT(self);
	ASSERT(v);
	ASSERT(copy);

	copy->x = self->x*v->x;
	copy->y = self->y*v->y;
	copy->z = self->z*v->z;
}

void cc_vec3f_muls(cc_vec3f_t* self, float s)
{
	ASSERT(self);

	self->x *= s;
	self->y *= s;
	self->z *= s;
}

void cc_vec3f_muls_copy(const cc_vec3f_t* self, float s,
                        cc_vec3f_t* copy)
{
	ASSERT(self);
	ASSERT(copy);

	copy->x = s*self->x;
	copy->y = s*self->y;
	copy->z = s*self->z;
}

void cc_vec3f_normalize(cc_vec3f_t* self)
{
	ASSERT(self);

	float mag = cc_vec3f_mag(self);
	cc_vec3f_muls(self, 1.0f/mag);
}

void cc_vec3f_normalize_copy(const cc_vec3f_t* self,
                             cc_vec3f_t* copy)
{
	ASSERT(self);
	ASSERT(copy);

	float mag = cc_vec3f_mag(self);
	cc_vec3f_muls_copy(self, 1.0f/mag, copy);
}

float cc_vec3f_dot(const cc_vec3f_t* a, const cc_vec3f_t* b)
{
	ASSERT(a);
	ASSERT(b);

	return a->x*b->x + a->y*b->y + a->z*b->z;
}

float cc_vec3f_distance(const cc_vec3f_t* a,
                        const cc_vec3f_t* b)
{
	ASSERT(a);
	ASSERT(b);

	cc_vec3f_t v;
	cc_vec3f_subv_copy(a, b, &v);
	return cc_vec3f_mag(&v);
}

void cc_vec3f_cross(cc_vec3f_t* self, const cc_vec3f_t* v)
{
	ASSERT(self);
	ASSERT(v);

	cc_vec3f_t copy;
	cc_vec3f_cross_copy(self, v, &copy);
	cc_vec3f_copy(&copy, self);
}

void cc_vec3f_cross_copy(const cc_vec3f_t* self,
                         const cc_vec3f_t* v,
                         cc_vec3f_t* copy)
{
	ASSERT(self);
	ASSERT(v);
	ASSERT(copy);

	const cc_vec3f_t* a = self;
	const cc_vec3f_t* b = v;
	copy->x =  (a->y*b->z) - (b->y*a->z);
	copy->y = -(a->x*b->z) + (b->x*a->z);
	copy->z =  (a->x*b->y) - (b->x*a->y);
}
