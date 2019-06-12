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

#ifndef cc_mat3f_H
#define cc_mat3f_H

#include "cc_vec3f.h"

// note: cc_mat3f_t* may be cast to float* (column-major)

typedef struct
{
	float m00;
	float m10;
	float m20;
	float m01;
	float m11;
	float m21;
	float m02;
	float m12;
	float m22;
} cc_mat3f_t;

void cc_mat3f_identity(cc_mat3f_t* self);
void cc_mat3f_copy(const cc_mat3f_t* self,
                   cc_mat3f_t* copy);
void cc_mat3f_transpose(cc_mat3f_t* self);
void cc_mat3f_transpose_copy(const cc_mat3f_t* self,
                             cc_mat3f_t* copy);
void cc_mat3f_inverse(cc_mat3f_t* self);
void cc_mat3f_inverse_copy(const cc_mat3f_t* self,
                           cc_mat3f_t* copy);
void cc_mat3f_mulm(cc_mat3f_t* self, const cc_mat3f_t* m);
void cc_mat3f_mulm_copy(const cc_mat3f_t* self,
                        const cc_mat3f_t* m,
                        cc_mat3f_t* copy);
void cc_mat3f_mulv(const cc_mat3f_t* self, cc_vec3f_t* v);
void cc_mat3f_mulv_copy(const cc_mat3f_t* self,
                        const cc_vec3f_t* v,
                        cc_vec3f_t* copy);
void cc_mat3f_muls(cc_mat3f_t* self, float s);
void cc_mat3f_muls_copy(const cc_mat3f_t* self, float s,
                        cc_mat3f_t* copy);

#endif
