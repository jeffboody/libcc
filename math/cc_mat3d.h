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

#ifndef cc_mat3d_H
#define cc_mat3d_H

#include "cc_vec3d.h"

// note: cc_mat3d_t* may be cast to double* (column-major)

typedef struct
{
	double m00;
	double m10;
	double m20;
	double m01;
	double m11;
	double m21;
	double m02;
	double m12;
	double m22;
} cc_mat3d_t;

void cc_mat3d_identity(cc_mat3d_t* self);
void cc_mat3d_copy(const cc_mat3d_t* self,
                   cc_mat3d_t* copy);
void cc_mat3d_transpose(cc_mat3d_t* self);
void cc_mat3d_transpose_copy(const cc_mat3d_t* self,
                             cc_mat3d_t* copy);
void cc_mat3d_inverse(cc_mat3d_t* self);
void cc_mat3d_inverse_copy(const cc_mat3d_t* self,
                           cc_mat3d_t* copy);
void cc_mat3d_mulm(cc_mat3d_t* self, const cc_mat3d_t* m);
void cc_mat3d_mulm_copy(const cc_mat3d_t* self,
                        const cc_mat3d_t* m,
                        cc_mat3d_t* copy);
void cc_mat3d_mulv(const cc_mat3d_t* self, cc_vec3d_t* v);
void cc_mat3d_mulv_copy(const cc_mat3d_t* self,
                        const cc_vec3d_t* v,
                        cc_vec3d_t* copy);
void cc_mat3d_muls(cc_mat3d_t* self, double s);
void cc_mat3d_muls_copy(const cc_mat3d_t* self, double s,
                        cc_mat3d_t* copy);

#endif
