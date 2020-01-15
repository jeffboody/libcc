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

#define LOG_TAG "cc"
#include "../cc_log.h"
#include "cc_fplane.h"

/***********************************************************
* public                                                   *
***********************************************************/

int cc_fplane_clipsphere(const cc_fplane_t* self,
                         const cc_sphere_t* s)
{
	ASSERT(self);
	ASSERT(s);

	if(cc_plane_clipsphere(&self->near,   s) ||
	   cc_plane_clipsphere(&self->far,    s) ||
	   cc_plane_clipsphere(&self->left,   s) ||
	   cc_plane_clipsphere(&self->right,  s) ||
	   cc_plane_clipsphere(&self->top,    s) ||
	   cc_plane_clipsphere(&self->bottom, s))
	{
		return 1;
	}

	return 0;
}
