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

#ifndef cc_quaternion_H
#define cc_quaternion_H

#include "cc_vec3f.h"

typedef struct
{
	cc_vec3f_t v;
	float      s;
} cc_quaternion_t;

// quaternion operations
// enforce special case of unit quaternions to be used for rotations
// http://content.gpwiki.org/index.php/OpenGL:Tutorials:Using_Quaternions_to_represent_rotation
// http://www.flipcode.com/documents/matrfaq.html
void cc_quaternion_load(cc_quaternion_t* self,
                        float x, float y, float z, float s);
void cc_quaternion_loadaxis(cc_quaternion_t* self, float a,
                            float x, float y, float z);
void cc_quaternion_loadeuler(cc_quaternion_t* self,
                             float rx, float ry, float rz);
void cc_quaternion_loadaxisangle(cc_quaternion_t* self,
                                 float ax, float ay,
                                 float az,
                                 float angle);
void cc_quaternion_copy(const cc_quaternion_t* self,
                        cc_quaternion_t* q);
void cc_quaternion_identity(cc_quaternion_t* self);
void cc_quaternion_inverse(cc_quaternion_t* self);
void cc_quaternion_inverse_copy(const cc_quaternion_t* self,
                                cc_quaternion_t* q);
void cc_quaternion_rotateq(cc_quaternion_t* self,
                           const cc_quaternion_t* q);
void cc_quaternion_rotateq_copy(const cc_quaternion_t* self,
                                const cc_quaternion_t* q,
                                cc_quaternion_t* copy);
void cc_quaternion_slerp(const cc_quaternion_t* a,
                         const cc_quaternion_t* b,
                         float t, cc_quaternion_t* c);
float cc_quaternion_compare(const cc_quaternion_t* a,
                            const cc_quaternion_t* b);

#endif
