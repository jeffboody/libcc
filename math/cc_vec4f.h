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

#ifndef cc_vec4f_H
#define cc_vec4f_H

// note: cc_vec4f_t* may be cast to float*

typedef struct
{
	union
	{
		// point or vector
		struct
		{
			float x;
			float y;
			float z;
			float w;
		};

		// color
		struct
		{
			float r;
			float g;
			float b;
			float a;
		};
	};
} cc_vec4f_t;

// dynamic constructor/destructor
cc_vec4f_t* cc_vec4f_new(float x, float y, float z,
                         float w);
void        cc_vec4f_delete(cc_vec4f_t** _self);

// standard vector operations
void  cc_vec4f_load(cc_vec4f_t* self, float x, float y,
                    float z, float w);
void  cc_vec4f_copy(const cc_vec4f_t* self,
                    cc_vec4f_t* copy);
int   cc_vec4f_equals(const cc_vec4f_t* self,
                      const cc_vec4f_t* v);
float cc_vec4f_mag(const cc_vec4f_t* self);
void  cc_vec4f_addv(cc_vec4f_t* self,
                    const cc_vec4f_t* v);
void  cc_vec4f_addv_copy(const cc_vec4f_t* self,
                         const cc_vec4f_t* v,
                         cc_vec4f_t* copy);
void  cc_vec4f_adds(cc_vec4f_t* self, float s);
void  cc_vec4f_adds_copy(const cc_vec4f_t* self, float s,
                         cc_vec4f_t* copy);
void  cc_vec4f_subv(cc_vec4f_t* self, const cc_vec4f_t* v);
void  cc_vec4f_subv_copy(const cc_vec4f_t* self,
                         const cc_vec4f_t* v,
                         cc_vec4f_t* copy);
void  cc_vec4f_mulv(cc_vec4f_t* self, const cc_vec4f_t* v);
void  cc_vec4f_mulv_copy(const cc_vec4f_t* self,
                         const cc_vec4f_t* v,
                         cc_vec4f_t* copy);
void  cc_vec4f_muls(cc_vec4f_t* self, float s);
void  cc_vec4f_muls_copy(const cc_vec4f_t* self, float s,
                         cc_vec4f_t* copy);
void  cc_vec4f_normalize(cc_vec4f_t* self);
void  cc_vec4f_normalize_copy(const cc_vec4f_t* self,
                              cc_vec4f_t* copy);
float cc_vec4f_dot(const cc_vec4f_t* a,
                   const cc_vec4f_t* b);

// TODO - homogeneous operations

#endif
