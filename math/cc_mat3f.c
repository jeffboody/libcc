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

#include <math.h>
#include <stdlib.h>

#define LOG_TAG "cc"
#include "../cc_log.h"
#include "cc_mat3f.h"

/***********************************************************
* public                                                   *
***********************************************************/

void cc_mat3f_identity(cc_mat3f_t* self)
{
	ASSERT(self);

	self->m00 = 1.0f;
	self->m01 = 0.0f;
	self->m02 = 0.0f;
	self->m10 = 0.0f;
	self->m11 = 1.0f;
	self->m12 = 0.0f;
	self->m20 = 0.0f;
	self->m21 = 0.0f;
	self->m22 = 1.0f;
}

void cc_mat3f_copy(const cc_mat3f_t* self, cc_mat3f_t* copy)
{
	ASSERT(self);
	ASSERT(copy);

	copy->m00 = self->m00;
	copy->m01 = self->m01;
	copy->m02 = self->m02;
	copy->m10 = self->m10;
	copy->m11 = self->m11;
	copy->m12 = self->m12;
	copy->m20 = self->m20;
	copy->m21 = self->m21;
	copy->m22 = self->m22;
}

void cc_mat3f_transpose(cc_mat3f_t* self)
{
	ASSERT(self);

	cc_mat3f_t copy;
	cc_mat3f_transpose_copy(self, &copy);
	cc_mat3f_copy(&copy, self);
}

void cc_mat3f_transpose_copy(const cc_mat3f_t* self,
                             cc_mat3f_t* copy)
{
	ASSERT(self);
	ASSERT(copy);

	copy->m00 = self->m00;
	copy->m01 = self->m10;
	copy->m02 = self->m20;
	copy->m10 = self->m01;
	copy->m11 = self->m11;
	copy->m12 = self->m21;
	copy->m20 = self->m02;
	copy->m21 = self->m12;
	copy->m22 = self->m22;
}

void cc_mat3f_inverse(cc_mat3f_t* self)
{
	ASSERT(self);

	cc_mat3f_t copy;
	cc_mat3f_inverse_copy(self, &copy);
	cc_mat3f_copy(&copy, self);
}

void cc_mat3f_inverse_copy(const cc_mat3f_t* self,
                           cc_mat3f_t* copy)
{
	ASSERT(self);
	ASSERT(copy);

	// augmented matrix [a|I]
	// copy is initialized to I but will contain a^-1
	cc_mat3f_t a;
	cc_mat3f_copy(self, &a);
	cc_mat3f_identity(copy);

	// make shortcuts to access a and inverse-v (aka copy)
	float* aref = (float*) &a;
	float* vref = (float*) copy;
	#define A(row, col) aref[(row) + 3*(col)]
	#define V(row, col) vref[(row) + 3*(col)]

	// perform gauss-jordan elimination to determine the
	// inverse of a
	int i;   // rows
	int j;   // pivotal entry
	int l;   // largest magnitude pivotal entry
	int k;   // columns
	float x[3];
	float s;
	for(j = 0; j < 3; ++j)
	{
		// find largest magnitude element in column-j
		// where i >= j
		l = j;
		for(i = j + 1; i < 3; ++i)
		{
			if(fabs(A(i,j)) > fabs(A(l,j)))
			{
				l = i;
			}
		}

		// interchange row-l and row-j of a and v
		if(l != j)
		{
			for(k = 0; k < 3; ++k)
			{
				x[k]   = A(j,k);
				A(j,k) = A(l,k);
				A(l,k) = x[k];
			}
			for(k = 0; k < 3; ++k)
			{
				x[k]   = V(j,k);
				V(j,k) = V(l,k);
				V(l,k) = x[k];
			}
		}

		// use the row sum operation to ensure zeros appear
		// below the pivotal entry
		// skip j=2
		for(i = j + 1; i < 3; ++i)
		{
			s = A(i,j)/A(j,j);
			for(k = j + 1; k < 3; ++k)
			{
				A(i,k) -= s*A(j,k);
			}
			for(k = 0; k < 3; ++k)
			{
				V(i,k) -= s*V(j,k);
			}
			A(i,j) = 0.0f;
		}

		// force the pivotal entry to be one
		s = 1.0f/A(j,j);
		for(k = j + 1; k < 3; ++k)
		{
			A(j,k) *= s;
		}
		for(k = 0; k < 3; ++k)
		{
			V(j,k) *= s;
		}
		A(j,j) = 1.0f;
	}

	// force zeros above all leading coefficients
	// skip j=0
	for(j = 2; j > 0; --j)
	{
		for(i = j - 1; i >= 0; --i)
		{
			s = A(i,j);   // A(j,j) is 1.0f in this case
			for(k = j; k < 3; ++k)
			{
				A(i,k) -= s*A(j,k);
			}
			for(k = 0; k < 3; ++k)
			{
				V(i,k) -= s*V(j,k);
			}
		}
	}
}

void cc_mat3f_mulm(cc_mat3f_t* self, const cc_mat3f_t* m)
{
	ASSERT(self);
	ASSERT(m);

	cc_mat3f_t copy;
	cc_mat3f_mulm_copy(self, m, &copy);
	cc_mat3f_copy(&copy, self);
}

void cc_mat3f_mulm_copy(const cc_mat3f_t* self,
                        const cc_mat3f_t* m,
                        cc_mat3f_t* copy)
{
	ASSERT(self);
	ASSERT(m);
	ASSERT(copy);

	const cc_mat3f_t* a = self;
	cc_mat3f_t*       c = copy;
	c->m00 = a->m00*m->m00 + a->m01*m->m10 + a->m02*m->m20;
	c->m01 = a->m00*m->m01 + a->m01*m->m11 + a->m02*m->m21;
	c->m02 = a->m00*m->m02 + a->m01*m->m12 + a->m02*m->m22;
	c->m10 = a->m10*m->m00 + a->m11*m->m10 + a->m12*m->m20;
	c->m11 = a->m10*m->m01 + a->m11*m->m11 + a->m12*m->m21;
	c->m12 = a->m10*m->m02 + a->m11*m->m12 + a->m12*m->m22;
	c->m20 = a->m20*m->m00 + a->m21*m->m10 + a->m22*m->m20;
	c->m21 = a->m20*m->m01 + a->m21*m->m11 + a->m22*m->m21;
	c->m22 = a->m20*m->m02 + a->m21*m->m12 + a->m22*m->m22;
}

void cc_mat3f_mulv(const cc_mat3f_t* self, cc_vec3f_t* v)
{
	ASSERT(self);
	ASSERT(v);

	cc_vec3f_t copy;
	cc_mat3f_mulv_copy(self, v, &copy);
	cc_vec3f_copy(&copy, v);
}

void cc_mat3f_mulv_copy(const cc_mat3f_t* self,
                        const cc_vec3f_t* v,
                        cc_vec3f_t* copy)
{
	ASSERT(self);
	ASSERT(v);
	ASSERT(copy);

	const cc_mat3f_t* a = self;
	cc_vec3f_t*       c = copy;
	c->x = a->m00*v->x + a->m01*v->y + a->m02*v->z;
	c->y = a->m10*v->x + a->m11*v->y + a->m12*v->z;
	c->z = a->m20*v->x + a->m21*v->y + a->m22*v->z;
}

void cc_mat3f_muls(cc_mat3f_t* self, float s)
{
	ASSERT(self);

	self->m00 *= s;
	self->m01 *= s;
	self->m02 *= s;
	self->m10 *= s;
	self->m11 *= s;
	self->m12 *= s;
	self->m20 *= s;
	self->m21 *= s;
	self->m22 *= s;
}

void cc_mat3f_muls_copy(const cc_mat3f_t* self, float s,
                        cc_mat3f_t* copy)
{
	ASSERT(self);
	ASSERT(copy);

	copy->m00 = s*self->m00;
	copy->m01 = s*self->m01;
	copy->m02 = s*self->m02;
	copy->m10 = s*self->m10;
	copy->m11 = s*self->m11;
	copy->m12 = s*self->m12;
	copy->m20 = s*self->m20;
	copy->m21 = s*self->m21;
	copy->m22 = s*self->m22;
}
