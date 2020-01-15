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
#include "cc_plane.h"

/***********************************************************
* public                                                   *
***********************************************************/

void cc_plane_load(cc_plane_t* self,
                   float nx, float ny, float nz, float d)
{
	ASSERT(self);

	cc_vec3f_load(&self->n, nx, ny, nz);
	self->d = d;
}

void cc_plane_copy(const cc_plane_t* self, cc_plane_t* copy)
{
	ASSERT(self);
	ASSERT(copy);

	cc_vec3f_copy(&self->n, &copy->n);
	copy->d = self->d;
}

float cc_plane_distance(const cc_plane_t* self,
                        const cc_vec3f_t* p)
{
	ASSERT(self);
	ASSERT(p);

	cc_vec3f_t v;
	cc_vec3f_t q;
	cc_vec3f_muls_copy(&self->n, self->d, &q);
	cc_vec3f_subv_copy(p, &q, &v);
	return cc_vec3f_dot(&self->n, &v);
}

int cc_plane_clipsphere(const cc_plane_t* self,
                        const cc_sphere_t* s)
{
	ASSERT(self);
	ASSERT(s);

	float d = cc_plane_distance(self, &s->c);
	return ((s->r + d) < 0.0f) ? 1 : 0;
}
