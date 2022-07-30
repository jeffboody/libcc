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

#ifndef cc_vec3d_H
#define cc_vec3d_H

// note: cc_vec3d_t* may be cast to double*

typedef struct
{
	double x;
	double y;
	double z;
} cc_vec3d_t;

// dynamic constructor/destructor
cc_vec3d_t* cc_vec3d_new(double x, double y, double z);
void        cc_vec3d_delete(cc_vec3d_t** _self);

// standard vector operations
void   cc_vec3d_load(cc_vec3d_t* self,
                     double x, double y, double z);
void   cc_vec3d_copy(const cc_vec3d_t* self,
                     cc_vec3d_t* copy);
int    cc_vec3d_equals(const cc_vec3d_t* self,
                       const cc_vec3d_t* v);
double cc_vec3d_mag(const cc_vec3d_t* self);
void   cc_vec3d_addv(cc_vec3d_t* self, const cc_vec3d_t* v);
void   cc_vec3d_addv_copy(const cc_vec3d_t* self,
                          const cc_vec3d_t* v,
                          cc_vec3d_t* copy);
void   cc_vec3d_adds(cc_vec3d_t* self, double s);
void   cc_vec3d_adds_copy(const cc_vec3d_t* self,
                          double s,
                          cc_vec3d_t* copy);
void   cc_vec3d_subv(cc_vec3d_t* self, const cc_vec3d_t* v);
void   cc_vec3d_subv_copy(const cc_vec3d_t* self,
                          const cc_vec3d_t* v,
                          cc_vec3d_t* copy);
void   cc_vec3d_mulv(cc_vec3d_t* self, const cc_vec3d_t* v);
void   cc_vec3d_mulv_copy(const cc_vec3d_t* self,
                          const cc_vec3d_t* v,
                          cc_vec3d_t* copy);
void   cc_vec3d_muls(cc_vec3d_t* self, double s);
void   cc_vec3d_muls_copy(const cc_vec3d_t* self,
                          double s,
                          cc_vec3d_t* copy);
void   cc_vec3d_normalize(cc_vec3d_t* self);
void   cc_vec3d_normalize_copy(const cc_vec3d_t* self,
                               cc_vec3d_t* copy);
double cc_vec3d_dot(const cc_vec3d_t* a,
                   const cc_vec3d_t* b);
double cc_vec3d_distance(const cc_vec3d_t* a,
                         const cc_vec3d_t* b);
void   cc_vec3d_cross(cc_vec3d_t* self,
                      const cc_vec3d_t* v);
void   cc_vec3d_cross_copy(const cc_vec3d_t* self,
                           const cc_vec3d_t* v,
                           cc_vec3d_t* copy);

// TODO - homogeneous operations

#endif
