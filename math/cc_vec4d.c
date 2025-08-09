/*
 * Copyright (c) 2025 Jeff Boody
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

#include <float.h>
#include <math.h>
#include <stdlib.h>

#define LOG_TAG "cc"
#include "../cc_log.h"
#include "../cc_memory.h"
#include "cc_vec4d.h"

/***********************************************************
* public                                                   *
***********************************************************/

cc_vec4d_t* cc_vec4d_new(double x, double y, double z, double w)
{
	cc_vec4d_t* self = (cc_vec4d_t*)
	                   MALLOC(sizeof(cc_vec4d_t));
	if(self == NULL)
	{
		LOGE("MALLOC failed");
		return NULL;
	}

	self->x = x;
	self->y = y;
	self->z = z;
	self->w = w;
	return self;
}

void cc_vec4d_delete(cc_vec4d_t** _self)
{
	ASSERT(_self);

	cc_vec4d_t* self = *_self;
	if(self)
	{
		FREE(self);
		*_self = NULL;
	}
}

void cc_vec4d_load(cc_vec4d_t* self, double x, double y,
                   double z, double w)
{
	ASSERT(self);

	self->x = x;
	self->y = y;
	self->z = z;
	self->w = w;
}

void cc_vec4d_copy(const cc_vec4d_t* self, cc_vec4d_t* copy)
{
	ASSERT(self);
	ASSERT(copy);

	copy->x = self->x;
	copy->y = self->y;
	copy->z = self->z;
	copy->w = self->w;
}

int cc_vec4d_equals(const cc_vec4d_t* self,
                    const cc_vec4d_t* v)
{
	ASSERT(self);
	ASSERT(v);

	if((self->x == v->x) &&
	   (self->y == v->y) &&
	   (self->z == v->z) &&
	   (self->w == v->w))
	{
		return 1;
	}

	return 0;
}

double cc_vec4d_mag(const cc_vec4d_t* self)
{
	ASSERT(self);

	return sqrtf(self->x*self->x + self->y*self->y +
	             self->z*self->z + self->w*self->w);
}

void cc_vec4d_addv(cc_vec4d_t* self, const cc_vec4d_t* v)
{
	ASSERT(self);
	ASSERT(v);

	self->x += v->x;
	self->y += v->y;
	self->z += v->z;
	self->w += v->w;
}

void cc_vec4d_addv_copy(const cc_vec4d_t* self,
                        const cc_vec4d_t* v,
                        cc_vec4d_t* copy)
{
	ASSERT(self);
	ASSERT(v);
	ASSERT(copy);

	copy->x = self->x + v->x;
	copy->y = self->y + v->y;
	copy->z = self->z + v->z;
	copy->w = self->w + v->w;
}

void cc_vec4d_adds(cc_vec4d_t* self, double s)
{
	ASSERT(self);

	self->x += s;
	self->y += s;
	self->z += s;
	self->w += s;
}

void cc_vec4d_adds_copy(const cc_vec4d_t* self, double s,
                        cc_vec4d_t* copy)
{
	ASSERT(self);
	ASSERT(copy);

	copy->x = self->x + s;
	copy->y = self->y + s;
	copy->z = self->z + s;
	copy->w = self->w + s;
}

void cc_vec4d_subv(cc_vec4d_t* self, const cc_vec4d_t* v)
{
	ASSERT(self);
	ASSERT(v);

	self->x -= v->x;
	self->y -= v->y;
	self->z -= v->z;
	self->w -= v->w;
}

void cc_vec4d_subv_copy(const cc_vec4d_t* self,
                        const cc_vec4d_t* v,
                        cc_vec4d_t* copy)
{
	ASSERT(self);
	ASSERT(v);
	ASSERT(copy);

	copy->x = self->x - v->x;
	copy->y = self->y - v->y;
	copy->z = self->z - v->z;
	copy->w = self->w - v->w;
}

void cc_vec4d_mulv(cc_vec4d_t* self, const cc_vec4d_t* v)
{
	ASSERT(self);
	ASSERT(v);

	self->x *= v->x;
	self->y *= v->y;
	self->z *= v->z;
	self->w *= v->w;
}

void cc_vec4d_mulv_copy(const cc_vec4d_t* self,
                        const cc_vec4d_t* v,
                        cc_vec4d_t* copy)
{
	ASSERT(self);
	ASSERT(v);
	ASSERT(copy);

	copy->x = self->x*v->x;
	copy->y = self->y*v->y;
	copy->z = self->z*v->z;
	copy->w = self->w*v->w;
}

void cc_vec4d_muls(cc_vec4d_t* self, double s)
{
	ASSERT(self);

	self->x *= s;
	self->y *= s;
	self->z *= s;
	self->w *= s;
}

void cc_vec4d_muls_copy(const cc_vec4d_t* self, double s,
                        cc_vec4d_t* copy)
{
	ASSERT(self);
	ASSERT(copy);

	copy->x = s*self->x;
	copy->y = s*self->y;
	copy->z = s*self->z;
	copy->w = s*self->w;
}

void cc_vec4d_normalize(cc_vec4d_t* self)
{
	ASSERT(self);

	double mag = cc_vec4d_mag(self) + DBL_EPSILON;
	cc_vec4d_muls(self, 1.0/mag);
}

void cc_vec4d_normalize_copy(const cc_vec4d_t* self,
                             cc_vec4d_t* copy)
{
	ASSERT(self);
	ASSERT(copy);

	double mag = cc_vec4d_mag(self) + DBL_EPSILON;
	cc_vec4d_muls_copy(self, 1.0/mag, copy);
}

double cc_vec4d_dot(const cc_vec4d_t* a, const cc_vec4d_t* b)
{
	ASSERT(a);
	ASSERT(b);

	return a->x*b->x + a->y*b->y + a->z*b->z + a->w*b->w;
}

void cc_vec4d_lerp(const cc_vec4d_t* a, const cc_vec4d_t* b,
                   double s, cc_vec4d_t* c)
{
	ASSERT(a);
	ASSERT(b);
	ASSERT(c);

	c->x = a->x + s*(b->x - a->x);
	c->y = a->y + s*(b->y - a->y);
	c->z = a->z + s*(b->z - a->z);
	c->w = a->w + s*(b->w - a->w);
}
