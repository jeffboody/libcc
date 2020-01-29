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
#include "cc_mat4f.h"

/***********************************************************
* private                                                  *
***********************************************************/

static void
cc_mat4f_projuv(cc_vec4f_t* u, cc_vec4f_t* v,
                cc_vec4f_t* projuv)
{
	ASSERT(u);
	ASSERT(v);
	ASSERT(projuv);

	float dotvu = cc_vec4f_dot(v, u);
	float dotuu = cc_vec4f_dot(u, u);
	cc_vec4f_muls_copy(u, dotvu/dotuu, projuv);
}

/***********************************************************
* public                                                   *
***********************************************************/

// TODO - optimize special cases instead of using cc_mat4f_mulm

void cc_mat4f_identity(cc_mat4f_t* self)
{
	ASSERT(self);

	self->m00 = 1.0f;
	self->m01 = 0.0f;
	self->m02 = 0.0f;
	self->m03 = 0.0f;
	self->m10 = 0.0f;
	self->m11 = 1.0f;
	self->m12 = 0.0f;
	self->m13 = 0.0f;
	self->m20 = 0.0f;
	self->m21 = 0.0f;
	self->m22 = 1.0f;
	self->m23 = 0.0f;
	self->m30 = 0.0f;
	self->m31 = 0.0f;
	self->m32 = 0.0f;
	self->m33 = 1.0f;
}

void cc_mat4f_copy(const cc_mat4f_t* self, cc_mat4f_t* copy)
{
	ASSERT(self);
	ASSERT(copy);

	copy->m00 = self->m00;
	copy->m01 = self->m01;
	copy->m02 = self->m02;
	copy->m03 = self->m03;
	copy->m10 = self->m10;
	copy->m11 = self->m11;
	copy->m12 = self->m12;
	copy->m13 = self->m13;
	copy->m20 = self->m20;
	copy->m21 = self->m21;
	copy->m22 = self->m22;
	copy->m23 = self->m23;
	copy->m30 = self->m30;
	copy->m31 = self->m31;
	copy->m32 = self->m32;
	copy->m33 = self->m33;
}

void cc_mat4f_transpose(cc_mat4f_t* self)
{
	ASSERT(self);

	cc_mat4f_t copy;
	cc_mat4f_transpose_copy(self, &copy);
	cc_mat4f_copy(&copy, self);
}

void cc_mat4f_transpose_copy(const cc_mat4f_t* self,
                             cc_mat4f_t* copy)
{
	ASSERT(self);
	ASSERT(copy);

	copy->m00 = self->m00;
	copy->m01 = self->m10;
	copy->m02 = self->m20;
	copy->m03 = self->m30;
	copy->m10 = self->m01;
	copy->m11 = self->m11;
	copy->m12 = self->m21;
	copy->m13 = self->m31;
	copy->m20 = self->m02;
	copy->m21 = self->m12;
	copy->m22 = self->m22;
	copy->m23 = self->m32;
	copy->m30 = self->m03;
	copy->m31 = self->m13;
	copy->m32 = self->m23;
	copy->m33 = self->m33;
}

void cc_mat4f_inverse(cc_mat4f_t* self)
{
	ASSERT(self);

	cc_mat4f_t copy;
	cc_mat4f_inverse_copy(self, &copy);
	cc_mat4f_copy(&copy, self);
}

void cc_mat4f_inverse_copy(const cc_mat4f_t* self,
                           cc_mat4f_t* copy)
{
	ASSERT(self);
	ASSERT(copy);

	// augmented matrix [a|I]
	// copy is initialized to I but will contain a^-1
	cc_mat4f_t a;
	cc_mat4f_copy(self, &a);
	cc_mat4f_identity(copy);

	// make shortcuts to access a and inverse-v (aka copy)
	float* aref = (float*) &a;
	float* vref = (float*) copy;
	#define A(row, col) aref[(row) + 4*(col)]
	#define V(row, col) vref[(row) + 4*(col)]

	// perform gauss-jordan elimination to determine
	// inverse of a
	int i;   // rows
	int j;   // pivotal entry
	int l;   // largest magnitude pivotal entry
	int k;   // columns
	float x[4];
	float s;
	for(j = 0; j < 4; ++j)
	{
		// find largest magnitude element in column-j
		// where i >= j
		l = j;
		for(i = j + 1; i < 4; ++i)
		{
			if(fabs(A(i,j)) > fabs(A(l,j)))
			{
				l = i;
			}
		}

		// interchange row-l and row-j of a and v
		if(l != j)
		{
			for(k = 0; k < 4; ++k)
			{
				x[k]   = A(j,k);
				A(j,k) = A(l,k);
				A(l,k) = x[k];
			}
			for(k = 0; k < 4; ++k)
			{
				x[k]   = V(j,k);
				V(j,k) = V(l,k);
				V(l,k) = x[k];
			}
		}

		// use the row sum operation to ensure zeros appear
		// below the pivotal entry
		// skip j=3
		for(i = j + 1; i < 4; ++i)
		{
			s = A(i,j)/A(j,j);
			for(k = j + 1; k < 4; ++k)
			{
				A(i,k) -= s*A(j,k);
			}
			for(k = 0; k < 4; ++k)
			{
				V(i,k) -= s*V(j,k);
			}
			A(i,j) = 0.0f;
		}

		// force the pivotal entry to be one
		s = 1.0f/A(j,j);
		for(k = j + 1; k < 4; ++k)
		{
			A(j,k) *= s;
		}
		for(k = 0; k < 4; ++k)
		{
			V(j,k) *= s;
		}
		A(j,j) = 1.0f;
	}

	// force zeros above all leading coefficients
	// skip j=0
	for(j = 3; j > 0; --j)
	{
		for(i = j - 1; i >= 0; --i)
		{
			s = A(i,j);   // A(j,j) is 1.0f in this case
			for(k = j; k < 4; ++k)
			{
				A(i,k) -= s*A(j,k);
			}
			for(k = 0; k < 4; ++k)
			{
				V(i,k) -= s*V(j,k);
			}
		}
	}
}

void cc_mat4f_mulm(cc_mat4f_t* self, const cc_mat4f_t* m)
{
	ASSERT(self);
	ASSERT(m);

	cc_mat4f_t copy;
	cc_mat4f_mulm_copy(self, m, &copy);
	cc_mat4f_copy(&copy, self);
}

void cc_mat4f_mulm_copy(const cc_mat4f_t* self,
                        const cc_mat4f_t* m,
                        cc_mat4f_t* copy)
{
	ASSERT(self);
	ASSERT(m);
	ASSERT(copy);

	const cc_mat4f_t* a = self;
	cc_mat4f_t*       c = copy;
	c->m00 = a->m00*m->m00 + a->m01*m->m10 + a->m02*m->m20 + a->m03*m->m30;
	c->m01 = a->m00*m->m01 + a->m01*m->m11 + a->m02*m->m21 + a->m03*m->m31;
	c->m02 = a->m00*m->m02 + a->m01*m->m12 + a->m02*m->m22 + a->m03*m->m32;
	c->m03 = a->m00*m->m03 + a->m01*m->m13 + a->m02*m->m23 + a->m03*m->m33;
	c->m10 = a->m10*m->m00 + a->m11*m->m10 + a->m12*m->m20 + a->m13*m->m30;
	c->m11 = a->m10*m->m01 + a->m11*m->m11 + a->m12*m->m21 + a->m13*m->m31;
	c->m12 = a->m10*m->m02 + a->m11*m->m12 + a->m12*m->m22 + a->m13*m->m32;
	c->m13 = a->m10*m->m03 + a->m11*m->m13 + a->m12*m->m23 + a->m13*m->m33;
	c->m20 = a->m20*m->m00 + a->m21*m->m10 + a->m22*m->m20 + a->m23*m->m30;
	c->m21 = a->m20*m->m01 + a->m21*m->m11 + a->m22*m->m21 + a->m23*m->m31;
	c->m22 = a->m20*m->m02 + a->m21*m->m12 + a->m22*m->m22 + a->m23*m->m32;
	c->m23 = a->m20*m->m03 + a->m21*m->m13 + a->m22*m->m23 + a->m23*m->m33;
	c->m30 = a->m30*m->m00 + a->m31*m->m10 + a->m32*m->m20 + a->m33*m->m30;
	c->m31 = a->m30*m->m01 + a->m31*m->m11 + a->m32*m->m21 + a->m33*m->m31;
	c->m32 = a->m30*m->m02 + a->m31*m->m12 + a->m32*m->m22 + a->m33*m->m32;
	c->m33 = a->m30*m->m03 + a->m31*m->m13 + a->m32*m->m23 + a->m33*m->m33;
}

void cc_mat4f_mulv(const cc_mat4f_t* self, cc_vec4f_t* v)
{
	ASSERT(self);
	ASSERT(v);

	cc_vec4f_t copy;
	cc_mat4f_mulv_copy(self, v, &copy);
	cc_vec4f_copy(&copy, v);
}

void cc_mat4f_mulv_copy(const cc_mat4f_t* self,
                        const cc_vec4f_t* v,
                        cc_vec4f_t* copy)
{
	ASSERT(self);
	ASSERT(v);
	ASSERT(copy);

	const cc_mat4f_t* a = self;
	cc_vec4f_t*       c = copy;
	c->x = a->m00*v->x + a->m01*v->y + a->m02*v->z + a->m03*v->w;
	c->y = a->m10*v->x + a->m11*v->y + a->m12*v->z + a->m13*v->w;
	c->z = a->m20*v->x + a->m21*v->y + a->m22*v->z + a->m23*v->w;
	c->w = a->m30*v->x + a->m31*v->y + a->m32*v->z + a->m33*v->w;
}

void cc_mat4f_muls(cc_mat4f_t* self, float s)
{
	ASSERT(self);

	self->m00 *= s;
	self->m01 *= s;
	self->m02 *= s;
	self->m03 *= s;
	self->m10 *= s;
	self->m11 *= s;
	self->m12 *= s;
	self->m13 *= s;
	self->m20 *= s;
	self->m21 *= s;
	self->m22 *= s;
	self->m23 *= s;
	self->m30 *= s;
	self->m31 *= s;
	self->m32 *= s;
	self->m33 *= s;
}

void cc_mat4f_muls_copy(const cc_mat4f_t* self, float s,
                        cc_mat4f_t* copy)
{
	ASSERT(self);
	ASSERT(copy);

	copy->m00 = s*self->m00;
	copy->m01 = s*self->m01;
	copy->m02 = s*self->m02;
	copy->m03 = s*self->m03;
	copy->m10 = s*self->m10;
	copy->m11 = s*self->m11;
	copy->m12 = s*self->m12;
	copy->m13 = s*self->m13;
	copy->m20 = s*self->m20;
	copy->m21 = s*self->m21;
	copy->m22 = s*self->m22;
	copy->m23 = s*self->m23;
	copy->m30 = s*self->m30;
	copy->m31 = s*self->m31;
	copy->m32 = s*self->m32;
	copy->m33 = s*self->m33;
}

void cc_mat4f_addm(cc_mat4f_t* self, const cc_mat4f_t* m)
{
	ASSERT(self);
	ASSERT(m);

	self->m00 += m->m00;
	self->m01 += m->m01;
	self->m02 += m->m02;
	self->m03 += m->m03;
	self->m10 += m->m10;
	self->m11 += m->m11;
	self->m12 += m->m12;
	self->m13 += m->m13;
	self->m20 += m->m20;
	self->m21 += m->m21;
	self->m22 += m->m22;
	self->m23 += m->m23;
	self->m30 += m->m30;
	self->m31 += m->m31;
	self->m32 += m->m32;
	self->m33 += m->m33;
}

void cc_mat4f_addm_copy(const cc_mat4f_t* self,
                        const cc_mat4f_t* m,
                        cc_mat4f_t* copy)
{
	ASSERT(self);
	ASSERT(m);
	ASSERT(copy);

	copy->m00 = self->m00 + m->m00;
	copy->m01 = self->m01 + m->m01;
	copy->m02 = self->m02 + m->m02;
	copy->m03 = self->m03 + m->m03;
	copy->m10 = self->m10 + m->m10;
	copy->m11 = self->m11 + m->m11;
	copy->m12 = self->m12 + m->m12;
	copy->m13 = self->m13 + m->m13;
	copy->m20 = self->m20 + m->m20;
	copy->m21 = self->m21 + m->m21;
	copy->m22 = self->m22 + m->m22;
	copy->m23 = self->m23 + m->m23;
	copy->m30 = self->m30 + m->m30;
	copy->m31 = self->m31 + m->m31;
	copy->m32 = self->m32 + m->m32;
	copy->m33 = self->m33 + m->m33;
}

void cc_mat4f_orthonormal(cc_mat4f_t* self)
{
	ASSERT(self);

	cc_mat4f_t copy;
	cc_mat4f_orthonormal_copy(self, &copy);
	*self = copy;
}

void cc_mat4f_orthonormal_copy(const cc_mat4f_t* self,
                               cc_mat4f_t* copy)
{
	ASSERT(self);
	ASSERT(copy);

	/*
	 * perform modified Gram-Schmitt ortho-normalization
	 */

	cc_vec4f_t v0 =
	{
		.x = self->m00,
		.y = self->m01,
		.z = self->m02,
		.w = self->m03
	};
	cc_vec4f_t v1 =
	{
		.x = self->m10,
		.y = self->m11,
		.z = self->m12,
		.w = self->m13
	};
	cc_vec4f_t v2 =
	{
		.x = self->m20,
		.y = self->m21,
		.z = self->m22,
		.w = self->m23
	};
	cc_vec4f_t v3 =
	{
		.x = self->m30,
		.y = self->m31,
		.z = self->m32,
		.w = self->m33
	};

	// normalize u0
	cc_vec4f_t u0;
	cc_vec4f_normalize_copy(&v0, &u0);

	// subtract the component of v1 in the direction of u0
	// normalize u1
	cc_vec4f_t u1;
	cc_vec4f_t projuv;
	cc_mat4f_projuv(&u0, &v1, &projuv);
	cc_vec4f_subv_copy(&v1, &projuv, &u1);
	cc_vec4f_normalize(&u1);

	// subtract the component of v2 in the direction of u1
	// subtract the component of u2 in the direction of u0
	// normalize u2
	cc_vec4f_t u2;
	cc_mat4f_projuv(&u1, &v2, &projuv);
	cc_vec4f_subv_copy(&v2, &projuv, &u2);
	cc_mat4f_projuv(&u0, &u2, &projuv);
	cc_vec4f_subv(&u2, &projuv);
	cc_vec4f_normalize(&u2);

	// subtract the component of v3 in the direction of u2
	// subtract the component of u3 in the direction of u1
	// subtract the component of u3 in the direction of u0
	// normalize u3
	cc_vec4f_t u3;
	cc_mat4f_projuv(&u2, &v3, &projuv);
	cc_vec4f_subv_copy(&v3, &projuv, &u3);
	cc_mat4f_projuv(&u1, &u3, &projuv);
	cc_vec4f_subv(&u3, &projuv);
	cc_mat4f_projuv(&u0, &u3, &projuv);
	cc_vec4f_subv(&u3, &projuv);
	cc_vec4f_normalize(&u3);

	// copy the orthonormal vectors
	copy->m00 = u0.x;
	copy->m01 = u0.y;
	copy->m02 = u0.z;
	copy->m03 = u0.w;
	copy->m10 = u1.x;
	copy->m11 = u1.y;
	copy->m12 = u1.z;
	copy->m13 = u1.w;
	copy->m20 = u2.x;
	copy->m21 = u2.y;
	copy->m22 = u2.z;
	copy->m23 = u2.w;
	copy->m30 = u3.x;
	copy->m31 = u3.y;
	copy->m32 = u3.z;
	copy->m33 = u3.w;
}

/*
 * quaternion operations
 */

void cc_mat4f_quaternion(const cc_mat4f_t* self,
                         cc_quaternion_t* q)
{
	ASSERT(self);
	ASSERT(q);

	// http://www.flipcode.com/documents/matrfaq.html#Q55
	// http://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/

	const cc_mat4f_t* a = self;

	float s;
	float w;
	float x;
	float y;
	float z;
	float tr = a->m00 + a->m11 + a->m22;
	if(tr > 0.0f)
	{
		s = sqrtf(tr + 1.0f)*2.0f;
		w = 0.25f*s;
		x = (a->m21 - a->m12)/s;
		y = (a->m02 - a->m20)/s;
		z = (a->m10 - a->m01)/s;
	}
	else if((a->m00 > a->m11) &&
	        (a->m00 > a->m22))
	{
		s = sqrtf(1.0f + a->m00 - a->m11 - a->m22)*2.0f;
		w = (a->m21 - a->m12)/s;
		x = 0.25f*s;
		y = (a->m01 + a->m10)/s;
		z = (a->m02 + a->m20)/s;
	}
	else if(a->m11 > a->m22)
	{
		s = sqrtf(1.0f + a->m11 - a->m00 - a->m22)*2.0f;
		w = (a->m02 - a->m20)/s;
		x = (a->m01 + a->m10)/s;
		y = 0.25f*s;
		z = (a->m12 + a->m21)/s;
	}
	else
	{
		s = sqrtf(1.0f + a->m22 - a->m00 - a->m11)*2.0f;
		w = (a->m10 - a->m01)/s;
		x = (a->m02 + a->m20)/s;
		y = (a->m12 + a->m21)/s;
		z = 0.25f*s;
	}
	cc_quaternion_load(q, x, y, z, w);
}

void cc_mat4f_rotateq(cc_mat4f_t* self, int load,
                      const cc_quaternion_t* q)
{
	ASSERT(self);
	ASSERT(q);

	float x2 = q->v.x*q->v.x;
	float y2 = q->v.y*q->v.y;
	float z2 = q->v.z*q->v.z;
	float xy = q->v.x*q->v.y;
	float xz = q->v.x*q->v.z;
	float yz = q->v.y*q->v.z;
	float xw = q->v.x*q->s;
	float yw = q->v.y*q->s;
	float zw = q->v.z*q->s;

	// requires normalized quaternions
	cc_mat4f_t m =
	{
		1.0f - 2.0f*(y2 + z2), 2.0f*(xy + zw), 2.0f*(xz - yw), 0.0f,
		2.0f*(xy - zw), 1.0f - 2.0f*(x2 + z2), 2.0f*(yz + xw), 0.0f,
		2.0f*(xz + yw), 2.0f*(yz - xw), 1.0f - 2.0f*(x2 + y2), 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f,
	};

	if(load)
	{
		cc_mat4f_copy(&m, self);
	}
	else
	{
		cc_mat4f_mulm(self, &m);
	}
}

/*
 * GL matrix operations
 */

void cc_mat4f_lookat(cc_mat4f_t* self, int load,
                     float eyex, float eyey, float eyez,
                     float centerx, float centery,
                     float centerz,
                     float upx, float upy, float upz)
{
	ASSERT(self);

	cc_vec3f_t eye =
	{
		eyex, eyey, eyez
	};

	cc_vec3f_t center =
	{
		centerx, centery, centerz
	};

	cc_vec3f_t up =
	{
		upx, upy, upz
	};

	cc_vec3f_t n;
	cc_vec3f_subv_copy(&center, &eye, &n);
	cc_vec3f_normalize(&n);
	cc_vec3f_normalize(&up);

	cc_vec3f_t u;
	cc_vec3f_t v;
	cc_vec3f_cross_copy(&n, &up, &u);
	cc_vec3f_cross_copy(&u, &n, &v);
	cc_vec3f_normalize(&u);
	cc_vec3f_normalize(&v);

	cc_mat4f_t m =
	{
		 u.x,  v.x, -n.x, 0.0f,
		 u.y,  v.y, -n.y, 0.0f,
		 u.z,  v.z, -n.z, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f,
	};
	cc_mat4f_translate(&m, 0, -eye.x, -eye.y, -eye.z);

	if(load)
	{
		cc_mat4f_copy(&m, self);
	}
	else
	{
		cc_mat4f_mulm(self, &m);
	}
}

void cc_mat4f_perspective(cc_mat4f_t* self, int load,
                          float fovy, float aspect,
                          float znear, float zfar)
{
	ASSERT(self);

	float f   = 1.0f/tanf(fovy*(M_PI/180.0f)/2.0f);
	float m00 = f/aspect;
	float m11 = -f;
	float m22 = zfar/(znear - zfar);
	float m23 = (znear*zfar)/(znear - zfar);

	cc_mat4f_t m =
	{
		 m00, 0.0f, 0.0f,  0.0f,
		0.0f,  m11, 0.0f,  0.0f,
		0.0f, 0.0f,  m22, -1.0f,
		0.0f, 0.0f,  m23,  0.0f,
	};

	if(load)
	{
		cc_mat4f_copy(&m, self);
	}
	else
	{
		cc_mat4f_mulm(self, &m);
	}
}

void cc_mat4f_perspectiveStereo(cc_mat4f_t* pmL,
                                cc_mat4f_t* pmR,
                                int load,
                                float fovy, float aspect,
                                float znear, float zfar,
                                float convergence,
                                float eye_separation)
{
	ASSERT(pmL);
	ASSERT(pmR);

	// http://www.animesh.me/2011/05/rendering-3d-anaglyph-in-opengl.html

	float tan_fovy2 = tanf(fovy*(M_PI/180.0f)/2.0f);
	float es2       = eye_separation/2.0f;
	float top       = znear*tan_fovy2;
	float bottom    = -top;
	float a         = aspect*tan_fovy2*convergence;
	float b         = a - es2;
	float c         = a + es2;
	float d         = znear/convergence;
	float left      = -b*d;
	float right     =  c*d;

	// left perspective matrix
	cc_mat4f_frustum(pmL, load,
	                 left, right,
	                 bottom, top,
	                 znear, zfar);

	// right perspective matrix
	left  = -c*d;
	right =  b*d;
	cc_mat4f_frustum(pmR, load,
	                 left, right,
	                 bottom, top,
	                 znear, zfar);
}

void cc_mat4f_rotate(cc_mat4f_t* self, int load,
                     float a,
                     float x, float y, float z)
{
	ASSERT(self);

	// normalize x, y, z
	float n = x*x + y*y + z*z;
	if(n != 1.0f)
	{
		n = 1.0f/sqrtf(n);
		x *= n;
		y *= n;
		z *= n;
	}

	// from http://www.manpagez.com/man/3/glRotatef/
	float c   = cosf(M_PI*a/180.0f);
	float s   = sinf(M_PI*a/180.0f);
	float p   = 1.0f - c;
	float xxp = x*x*p;
	float xyp = x*y*p;
	float xzp = x*z*p;
	float yyp = y*y*p;
	float yzp = y*z*p;
	float zzp = z*z*p;
	float xs  = x*s;
	float ys  = y*s;
	float zs  = z*s;

	cc_mat4f_t m =
	{
		 xxp + c, xyp + zs, xzp - ys, 0.0f,
		xyp - zs,  yyp + c, yzp + xs, 0.0f,
		xzp + ys, yzp - xs,  zzp + c, 0.0f,
		    0.0f,     0.0f,     0.0f, 1.0f,
	};

	if(load)
	{
		cc_mat4f_copy(&m, self);
	}
	else
	{
		cc_mat4f_mulm(self, &m);
	}
}

void cc_mat4f_translate(cc_mat4f_t* self, int load,
                        float x, float y, float z)
{
	ASSERT(self);

	cc_mat4f_t m =
	{
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		   x,    y,    z, 1.0f,
	};

	if(load)
	{
		cc_mat4f_copy(&m, self);
	}
	else
	{
		cc_mat4f_mulm(self, &m);
	}
}

void cc_mat4f_scale(cc_mat4f_t* self, int load,
                    float x, float y, float z)
{
	ASSERT(self);

	cc_mat4f_t m =
	{
		   x, 0.0f, 0.0f, 0.0f,
		0.0f,    y, 0.0f, 0.0f,
		0.0f, 0.0f,    z, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f,
	};

	if(load)
	{
		cc_mat4f_copy(&m, self);
	}
	else
	{
		cc_mat4f_mulm(self, &m);
	}
}

void cc_mat4f_frustum(cc_mat4f_t* self, int load,
                      float l, float r,
                      float b, float t,
                      float n, float f)
{
	ASSERT(self);

	if((n <= 0.0f) || (f <= 0.0f) ||
	   (l == r) || (t == b) || (n == f))
	{
		LOGE("invalid l=%f, r=%f, t=%f, b=%f, n=%f, f=%f",
		     l, r, t, b, n, f);
		return;
	}

	float n2  = 2.0f * n;
	float rml = r - l;
	float rpl = r + l;
	float tmb = t - b;
	float tpb = t + b;
	float fmn = f - n;
	float fpn = f + n;

	cc_mat4f_t m =
	{
		 n2/rml,    0.0f,      0.0f,  0.0f,
		   0.0f,  n2/tmb,      0.0f,  0.0f,
		rpl/rml, tpb/tmb,  -fpn/fmn, -1.0f,
		   0.0f,    0.0f, -n2*f/fmn,  0.0f,
	};

	if(load)
	{
		cc_mat4f_copy(&m, self);
	}
	else
	{
		cc_mat4f_mulm(self, &m);
	}
}

void cc_mat4f_orthoVK(cc_mat4f_t* self, int load,
                      float l, float r,
                      float b, float t,
                      float n, float f)
{
	ASSERT(self);

	// note: origin is top-left to match Vulkan
	// https://www.saschawillems.de/blog/2019/03/29/flipping-the-vulkana-viewport/
	// https://github.com/PacktPublishing/Vulkan-Cookbook/blob/master/Library/Source%20Files/10%20Helper%20Recipes/05%20Preparing%20an%20orthographic%20projection%20matrix.cpp

	if((l == r) || (t == b) || (n == f))
	{
		LOGE("invalid l=%f, r=%f, t=%f, b=%f, n=%f, f=%f",
		     l, r, t, b, n, f);
		return;
	}

	float rml = r - l;
	float rpl = r + l;
	float bmt = b - t;
	float tpb = t + b;
	float fmn = f - n;

	cc_mat4f_t m =
	{
		2.0f/rml,     0.0f,      0.0f, 0.0f,
		    0.0f, 2.0f/bmt,      0.0f, 0.0f,
		    0.0f,     0.0f, -1.0f/fmn, 0.0f,
		-rpl/rml, -tpb/bmt,    -n/fmn, 1.0f,
	};

	if(load)
	{
		cc_mat4f_copy(&m, self);
	}
	else
	{
		cc_mat4f_mulm(self, &m);
	}
}

void cc_mat4f_normalmatrix(const cc_mat4f_t* self,
                           cc_mat3f_t* nm)
{
	ASSERT(self);
	ASSERT(nm);

	// see link for the derivation of normal matrix
	// http://www.lighthouse3d.com/opengl/glsl/index.php?normalmatrix

	// use top-left 3x3 sub-region of the matrix
	nm->m00 = self->m00;
	nm->m01 = self->m01;
	nm->m02 = self->m02;
	nm->m10 = self->m10;
	nm->m11 = self->m11;
	nm->m12 = self->m12;
	nm->m20 = self->m20;
	nm->m21 = self->m21;
	nm->m22 = self->m22;

	// solve nm = (m^-1)^T
	cc_mat3f_inverse(nm);
	cc_mat3f_transpose(nm);
}
