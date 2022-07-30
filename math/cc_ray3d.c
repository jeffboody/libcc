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
