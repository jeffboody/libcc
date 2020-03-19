/*
 * Copyright (c) 2019 Jeff Boody
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
#include "cc_rect12f.h"

/***********************************************************
* public                                                   *
***********************************************************/

void cc_rect1f_init(cc_rect1f_t* self,
                    float t, float l, float w, float h)
{
	ASSERT(self);

	self->t = t;
	self->l = l;
	self->w = w;
	self->h = h;
}

void cc_rect1f_copy(const cc_rect1f_t* self,
                    cc_rect1f_t* copy)
{
	ASSERT(self);
	ASSERT(copy);

	copy->t = self->t;
	copy->l = self->l;
	copy->w = self->w;
	copy->h = self->h;
}

void cc_rect1f_copy2f(const cc_rect1f_t* self,
                      cc_rect2f_t* copy)
{
	ASSERT(self);
	ASSERT(copy);

	copy->t = self->t;
	copy->l = self->l;
	copy->b = self->t + self->h;
	copy->r = self->l + self->w;
}

int cc_rect1f_contains(const cc_rect1f_t* self,
                       float x, float y)
{
	ASSERT(self);

	cc_rect2f_t rect;
	cc_rect1f_copy2f(self, &rect);
	return cc_rect2f_contains(&rect, x, y);
}

int cc_rect1f_intersect(const cc_rect1f_t* a,
                        const cc_rect1f_t* b,
                        cc_rect1f_t* c)
{
	ASSERT(a);
	ASSERT(b);
	ASSERT(c);

	cc_rect2f_t a2;
	cc_rect2f_t b2;
	cc_rect2f_t c2;
	cc_rect1f_copy2f(a, &a2);
	cc_rect1f_copy2f(b, &b2);
	if(cc_rect2f_intersect(&a2, &b2, &c2))
	{
		cc_rect2f_copy1f(&c2, c);
		return 1;
	}

	cc_rect1f_init(c, 0.0f, 0.0f, 0.0f, 0.0f);
	return 0;
}

int cc_rect1f_equals(const cc_rect1f_t* self,
                     const cc_rect1f_t* rect)
{
	ASSERT(self);
	ASSERT(rect);

	if((self->t == rect->t) &&
	   (self->l == rect->l) &&
	   (self->w == rect->w) &&
	   (self->h == rect->h))
	{
		return 1;
	}

	return 0;
}

void cc_rect2f_init(cc_rect2f_t* self,
                    float t, float l, float b, float r)
{
	ASSERT(self);

	self->t = t;
	self->l = l;
	self->b = b;
	self->r = r;
}

void cc_rect2f_copy(const cc_rect2f_t* self,
                    cc_rect2f_t* copy)
{
	ASSERT(self);
	ASSERT(copy);

	copy->t = self->t;
	copy->l = self->l;
	copy->b = self->b;
	copy->r = self->r;
}

void cc_rect2f_copy1f(const cc_rect2f_t* self,
                      cc_rect1f_t* copy)
{
	ASSERT(self);
	ASSERT(copy);

	copy->t = self->t;
	copy->l = self->l;
	copy->w = self->r - self->l;
	copy->h = self->b - self->t;
}

int cc_rect2f_contains(const cc_rect2f_t* self,
                       float x, float y)
{
	ASSERT(self);

	float t = self->t;
	float l = self->l;
	float b = self->b;
	float r = self->r;

	// check for top-left origin
	if(b > t)
	{
		t = self->b;
		b = self->t;
	}

	// check if the point is inside the rect
	if((y > t) || (y < b) || (x < l) || (x > r))
	{
		return 0;
	}

	return 1;
}

int cc_rect2f_intersect(const cc_rect2f_t* a,
                        const cc_rect2f_t* b,
                        cc_rect2f_t* c)
{
	ASSERT(a);
	ASSERT(b);
	ASSERT(c);

	float at = a->t;
	float al = a->l;
	float ab = a->b;
	float ar = a->r;
	float bt = b->t;
	float bl = b->l;
	float bb = b->b;
	float br = b->r;

	// check for top-left origin
	int swap = 0;
	if(ab > at)
	{
		swap = 1;
		at   = a->b;
		ab   = a->t;
		bt   = b->b;
		bb   = b->t;
	}

	// check if a and b rects intersect
	if((at <= bb) || (ab >= bt) || (al >= br) || (ar <= bl))
	{
		cc_rect2f_init(c, 0.0f, 0.0f, 0.0f, 0.0f);
		return 0;
	}

	// compute intersection
	c->t = (at < bt) ? at : bt;
	c->l = (al > bl) ? al : bl;
	c->b = (ab > bb) ? ab : bb;
	c->r = (ar < br) ? ar : br;

	// convert to top-left origin
	if(swap)
	{
		float tmp = c->t;

		c->t = c->b;
		c->b = tmp;
	}

	return 1;
}

int cc_rect2f_equals(const cc_rect2f_t* self,
                     const cc_rect2f_t* rect)
{
	ASSERT(self);
	ASSERT(rect);

	if((self->t == rect->t) &&
	   (self->l == rect->l) &&
	   (self->b == rect->b) &&
	   (self->r == rect->r))
	{
		return 1;
	}

	return 0;
}
