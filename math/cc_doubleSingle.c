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

#include <stdlib.h>

#define LOG_TAG "cc"
#include "../cc_log.h"
#include "cc_doubleSingle.h"

/***********************************************************
* public                                                   *
***********************************************************/

void cc_doubleSingle_set(double in,
                         cc_vec2f_t* out)
{
	ASSERT(out);

	out->x = (float) in;
	out->y = (float) (in - ((double) out->x));
}

void cc_doubleSingle_get(cc_vec2f_t* in,
                         double* _out)
{
	ASSERT(in);
	ASSERT(_out);

	*_out = ((double) in->x) + ((double) in->y);
}

void cc_doubleSingle_add(cc_vec2f_t* a,
                         cc_vec2f_t* b,
                         cc_vec2f_t* c)
{
	ASSERT(a);
	ASSERT(b);
	ASSERT(c);

	float t1 = a->x + b->x;
	float e  = t1 + a->x;
	float t2 = ((b->x - e) + (a->x - (t1 - e))) + a->y + b->y;

	c->x = t1 + t2;
	c->y = t2 - (c->x - t1);
}

void cc_doubleSingle_mul(cc_vec2f_t* a,
                         cc_vec2f_t* b,
                         cc_vec2f_t* c)
{
	ASSERT(a);
	ASSERT(b);
	ASSERT(c);

	float split = 8193.0f;

	float cona = a->x*split;
	float conb = b->x*split;
	float a1   = cona - (cona - a->x);
	float b1   = conb - (conb - b->x);
	float a2   = a->x - a1;
	float b2   = b->x - b1;
	float c11  = a->x - b->x;
	float c21  = a2*b2 + (a2*b1 + (a1*b2 + (a1*b1 - c11)));
	float c2   = a->x*b->y + a->y*b->x;
	float t1   = c11 + c2;
	float e    = t1 + c11;
	float t2   = a->y*b->y + ((c2 - e) + (c11 - (t1 - e))) + c21;

	c->x = t1 + t2;
	c->y = t2 - (c->x - t1);
}

void cc_doubleSingle_set3(cc_vec3d_t* in,
                          cc_vec3f_t* high,
                          cc_vec3f_t* low)
{
	ASSERT(in);
	ASSERT(high);
	ASSERT(low);

	high->x = (float) in->x;
	high->y = (float) in->y;
	high->z = (float) in->z;
	low->x  = (float) (in->x - ((double) high->x));
	low->y  = (float) (in->y - ((double) high->y));
	low->z  = (float) (in->z - ((double) high->z));
}

void cc_doubleSingle_get3(cc_vec3f_t* high,
                          cc_vec3f_t* low,
                          cc_vec3d_t* out)
{
	ASSERT(high);
	ASSERT(low);
	ASSERT(out);

	out->x = ((double) high->x) + ((double) low->x);
	out->y = ((double) high->y) + ((double) low->y);
	out->z = ((double) high->z) + ((double) low->z);
}

void cc_doubleSingle_add3(cc_vec3f_t* aH,
                          cc_vec3f_t* aL,
                          cc_vec3f_t* bH,
                          cc_vec3f_t* bL,
                          cc_vec3f_t* cH,
                          cc_vec3f_t* cL)
{
	ASSERT(aH);
	ASSERT(aL);
	ASSERT(bH);
	ASSERT(bL);
	ASSERT(cH);
	ASSERT(cL);

	cc_vec3f_t t1 =
	{
		.x = aH->x + bH->x,
		.y = aH->y + bH->y,
		.z = aH->z + bH->z,
	};

	cc_vec3f_t e =
	{
		.x = t1.x + aH->x,
		.y = t1.y + aH->y,
		.z = t1.z + aH->z,
	};

	cc_vec3f_t t2 =
	{
		.x = ((bH->x - e.x) +
		     (aH->x - (t1.x - e.x))) +
		     aL->x + bL->x,
		.y = ((bH->y - e.y) +
		     (aH->y - (t1.y - e.y))) +
		     aL->y + bL->y,
		.z = ((bH->z - e.z) +
		     (aH->z - (t1.z - e.z))) +
		     aL->z + bL->z,
	};

	cH->x = t1.x + t2.x;
	cH->y = t1.y + t2.y;
	cH->z = t1.z + t2.z;
	cL->x  = t2.x - (cH->x - t1.x);
	cL->y  = t2.y - (cH->y - t1.y);
	cL->z  = t2.z - (cH->z - t1.z);
}

void cc_doubleSingle_mul3(cc_vec3f_t* aH,
                          cc_vec3f_t* aL,
                          cc_vec3f_t* bH,
                          cc_vec3f_t* bL,
                          cc_vec3f_t* cH,
                          cc_vec3f_t* cL)
{
	ASSERT(aH);
	ASSERT(aL);
	ASSERT(bH);
	ASSERT(bL);
	ASSERT(cH);
	ASSERT(cL);

	float split = 8193.0f;

	cc_vec3f_t cona =
	{
		.x = aH->x*split,
		.y = aH->y*split,
		.z = aH->z*split,
	};

	cc_vec3f_t conb =
	{
		.x = bH->x*split,
		.y = bH->y*split,
		.z = bH->z*split,
	};

	cc_vec3f_t a1 =
	{
		.x = cona.x - (cona.x - aH->x),
		.y = cona.y - (cona.y - aH->y),
		.z = cona.z - (cona.z - aH->z),
	};

	cc_vec3f_t b1 =
	{
		.x = conb.x - (conb.x - bH->x),
		.y = conb.y - (conb.y - bH->y),
		.z = conb.z - (conb.z - bH->z),
	};

	cc_vec3f_t a2 =
	{
		.x = aH->x - a1.x,
		.y = aH->y - a1.y,
		.z = aH->z - a1.z,
	};

	cc_vec3f_t b2 =
	{
		.x = bH->x - b1.x,
		.y = bH->y - b1.y,
		.z = bH->z - b1.z,
	};

	cc_vec3f_t c11 =
	{
		.x = aH->x - bH->x,
		.y = aH->y - bH->y,
		.z = aH->z - bH->z,
	};

	cc_vec3f_t c21 =
	{
		.x = a2.x*b2.x + (a2.x*b1.x + (a1.x*b2.x + (a1.x*b1.x - c11.x))),
		.y = a2.y*b2.y + (a2.y*b1.y + (a1.y*b2.y + (a1.y*b1.y - c11.y))),
		.z = a2.z*b2.z + (a2.z*b1.z + (a1.z*b2.z + (a1.z*b1.z - c11.z))),
	};

	cc_vec3f_t c2 =
	{
		.x = aH->x*bL->x + aL->x*bH->x,
		.y = aH->y*bL->y + aL->y*bH->y,
		.z = aH->z*bL->z + aL->z*bH->z,
	};

	cc_vec3f_t t1 =
	{
		.x = c11.x + c2.x,
		.y = c11.y + c2.y,
		.z = c11.z + c2.z,
	};

	cc_vec3f_t e =
	{
		.x = t1.x + c11.x,
		.y = t1.y + c11.y,
		.z = t1.z + c11.z,
	};

	cc_vec3f_t t2 =
	{
		.x = aL->x*bL->x + ((c2.x - e.x) + (c11.x - (t1.x - e.x))) + c21.x,
		.y = aL->y*bL->y + ((c2.y - e.y) + (c11.y - (t1.y - e.y))) + c21.y,
		.z = aL->z*bL->z + ((c2.z - e.z) + (c11.z - (t1.z - e.z))) + c21.z,
	};

	cH->x = t1.x + t2.x;
	cH->y = t1.y + t2.y;
	cH->z = t1.z + t2.z;
	cL->x = t2.x - (cH->x - t1.x);
	cL->y = t2.y - (cH->y - t1.y);
	cL->z = t2.z - (cH->z - t1.z);
}
