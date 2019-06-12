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

#ifndef cc_plane_H
#define cc_plane_H

#include "cc_sphere.h"
#include "cc_vec3f.h"

typedef struct
{
	cc_vec3f_t n;
	float      d;
} cc_plane_t;

void  cc_plane_load(cc_plane_t* self,
                    float nx, float ny, float nz,
                    float d);
void  cc_plane_copy(const cc_plane_t* self,
                    cc_plane_t* copy);
float cc_plane_distance(const cc_plane_t* self,
                        const cc_vec3f_t* p);
int   cc_plane_clipsphere(const cc_plane_t* self,
                          const cc_sphere_t* s);

#endif
