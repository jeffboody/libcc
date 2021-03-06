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
#include "../cc_memory.h"
#include "cc_vec2f.h"

/***********************************************************
* public                                                   *
***********************************************************/

cc_vec2f_t* cc_vec2f_new(float x, float y)
{
	cc_vec2f_t* self = (cc_vec2f_t*)
	                   MALLOC(sizeof(cc_vec2f_t));
	if(self == NULL)
	{
		LOGE("MALLOC failed");
		return NULL;
	}

	self->x = x;
	self->y = y;
	return self;
}

void cc_vec2f_delete(cc_vec2f_t** _self)
{
	ASSERT(_self);

	cc_vec2f_t* self = *_self;
	if(self)
	{
		FREE(self);
		*_self = NULL;
	}
}

void cc_vec2f_load(cc_vec2f_t* self, float x, float y)
{
	ASSERT(self);

	self->x = x;
	self->y = y;
}

void cc_vec2f_copy(const cc_vec2f_t* self, cc_vec2f_t* copy)
{
	ASSERT(self);
	ASSERT(copy);

	copy->x = self->x;
	copy->y = self->y;
}

int cc_vec2f_equals(const cc_vec2f_t* self,
                    const cc_vec2f_t* v)
{
	ASSERT(self);
	ASSERT(v);

	if((self->x == v->x) &&
	   (self->y == v->y))
	{
		return 1;
	}

	return 0;
}

float cc_vec2f_mag(const cc_vec2f_t* self)
{
	ASSERT(self);

	return sqrtf(self->x*self->x + self->y*self->y);
}

void cc_vec2f_addv(cc_vec2f_t* self, const cc_vec2f_t* v)
{
	ASSERT(self);
	ASSERT(v);

	self->x += v->x;
	self->y += v->y;
}

void cc_vec2f_addv_copy(const cc_vec2f_t* self,
                        const cc_vec2f_t* v,
                        cc_vec2f_t* copy)
{
	ASSERT(self);
	ASSERT(v);
	ASSERT(copy);

	copy->x = self->x + v->x;
	copy->y = self->y + v->y;
}

void cc_vec2f_adds(cc_vec2f_t* self, float s)
{
	ASSERT(self);

	self->x += s;
	self->y += s;
}

void cc_vec2f_adds_copy(const cc_vec2f_t* self, float s,
                        cc_vec2f_t* copy)
{
	ASSERT(self);
	ASSERT(copy);

	copy->x = self->x + s;
	copy->y = self->y + s;
}

void cc_vec2f_subv(cc_vec2f_t* self, const cc_vec2f_t* v)
{
	ASSERT(self);
	ASSERT(v);

	self->x -= v->x;
	self->y -= v->y;
}

void cc_vec2f_subv_copy(const cc_vec2f_t* self,
                        const cc_vec2f_t* v,
                        cc_vec2f_t* copy)
{
	ASSERT(self);
	ASSERT(v);
	ASSERT(copy);

	copy->x = self->x - v->x;
	copy->y = self->y - v->y;
}

void cc_vec2f_mulv(cc_vec2f_t* self, const cc_vec2f_t* v)
{
	ASSERT(self);
	ASSERT(v);

	self->x *= v->x;
	self->y *= v->y;
}

void cc_vec2f_mulv_copy(const cc_vec2f_t* self,
                        const cc_vec2f_t* v,
                        cc_vec2f_t* copy)
{
	ASSERT(self);
	ASSERT(v);
	ASSERT(copy);

	copy->x = self->x*v->x;
	copy->y = self->y*v->y;
}

void cc_vec2f_muls(cc_vec2f_t* self, float s)
{
	ASSERT(self);

	self->x *= s;
	self->y *= s;
}

void cc_vec2f_muls_copy(const cc_vec2f_t* self, float s,
                        cc_vec2f_t* copy)
{
	ASSERT(self);
	ASSERT(copy);

	copy->x = s*self->x;
	copy->y = s*self->y;
}

void cc_vec2f_normalize(cc_vec2f_t* self)
{
	ASSERT(self);

	float mag = cc_vec2f_mag(self);
	cc_vec2f_muls(self, 1.0f/mag);
}

void cc_vec2f_normalize_copy(const cc_vec2f_t* self,
                             cc_vec2f_t* copy)
{
	ASSERT(self);
	ASSERT(copy);

	float mag = cc_vec2f_mag(self);
	cc_vec2f_muls_copy(self, 1.0f/mag, copy);
}

float cc_vec2f_dot(const cc_vec2f_t* a, const cc_vec2f_t* b)
{
	ASSERT(a);
	ASSERT(b);

	return a->x*b->x + a->y*b->y;
}

float cc_vec2f_distance(const cc_vec2f_t* a,
                        const cc_vec2f_t* b)
{
	ASSERT(a);
	ASSERT(b);

	cc_vec2f_t v;
	cc_vec2f_subv_copy(a, b, &v);
	return cc_vec2f_mag(&v);
}

float cc_vec2f_cross(const cc_vec2f_t* a,
                     const cc_vec2f_t* b)
{
	ASSERT(a);
	ASSERT(b);

	return a->x*b->y - b->x*a->y;
}
