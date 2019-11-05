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

#ifndef cc_mat4f_H
#define cc_mat4f_H

#include "cc_mat3f.h"
#include "cc_quaternion.h"
#include "cc_vec4f.h"

// note: cc_mat4f_t* may be cast to float* (column-major)

typedef struct
{
	float m00;
	float m10;
	float m20;
	float m30;
	float m01;
	float m11;
	float m21;
	float m31;
	float m02;
	float m12;
	float m22;
	float m32;
	float m03;
	float m13;
	float m23;
	float m33;
} cc_mat4f_t;

// standard matrix operations
void cc_mat4f_identity(cc_mat4f_t* self);
void cc_mat4f_copy(const cc_mat4f_t* self,
                   cc_mat4f_t* copy);
void cc_mat4f_transpose(cc_mat4f_t* self);
void cc_mat4f_transpose_copy(const cc_mat4f_t* self,
                             cc_mat4f_t* copy);
void cc_mat4f_inverse(cc_mat4f_t* self);
void cc_mat4f_inverse_copy(const cc_mat4f_t* self,
                           cc_mat4f_t* copy);
void cc_mat4f_mulm(cc_mat4f_t* self,
                   const cc_mat4f_t* m);
void cc_mat4f_mulm_copy(const cc_mat4f_t* self,
                        const cc_mat4f_t* m,
                        cc_mat4f_t* copy);
void cc_mat4f_mulv(const cc_mat4f_t* self,
                   cc_vec4f_t* v);
void cc_mat4f_mulv_copy(const cc_mat4f_t* self,
                        const cc_vec4f_t* v,
                        cc_vec4f_t* copy);
void cc_mat4f_muls(cc_mat4f_t* self, float s);
void cc_mat4f_muls_copy(const cc_mat4f_t* self, float s,
                        cc_mat4f_t* copy);
void cc_mat4f_addm(cc_mat4f_t* self,
                   const cc_mat4f_t* m);
void cc_mat4f_addm_copy(const cc_mat4f_t* self,
                        const cc_mat4f_t* m,
                        cc_mat4f_t* copy);
void cc_mat4f_orthonormal(cc_mat4f_t* self);
void cc_mat4f_orthonormal_copy(const cc_mat4f_t* self,
                               cc_mat4f_t* copy);

// quaternion operations
void cc_mat4f_quaternion(const cc_mat4f_t* self,
                         cc_quaternion_t* q);
void cc_mat4f_rotateq(cc_mat4f_t* self, int load,
                      const cc_quaternion_t* q);

// graphics matrix operations
void cc_mat4f_lookat(cc_mat4f_t* self, int load,
                     float eyex, float eyey, float eyez,
                     float centerx, float centery,
                     float centerz,
                     float upx, float upy, float upz);
void cc_mat4f_perspective(cc_mat4f_t* self, int load,
                          float fovy, float aspect,
                          float znear, float zfar);
void cc_mat4f_perspectiveStereo(cc_mat4f_t* pmL,
                                cc_mat4f_t* pmR,
                                int load,
                                float fovy, float aspect,
                                float znear, float zfar,
                                float convergence,
                                float eye_separation);
void cc_mat4f_rotate(cc_mat4f_t* self, int load,
                     float a,
                     float x, float y, float z);
void cc_mat4f_translate(cc_mat4f_t* self, int load,
                        float x, float y, float z);
void cc_mat4f_scale(cc_mat4f_t* self, int load,
                    float x, float y, float z);
void cc_mat4f_frustum(cc_mat4f_t* self, int load,
                      float l, float r,
                      float b, float t,
                      float n, float f);
void cc_mat4f_orthoVK(cc_mat4f_t* self, int load,
                      float l, float r,
                      float b, float t,
                      float n, float f);
void cc_mat4f_normalmatrix(const cc_mat4f_t* self,
                           cc_mat3f_t* nm);

#endif
