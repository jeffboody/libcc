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

#ifndef cc_vec2f_H
#define cc_vec2f_H

// note: cc_vec2f_t* may be cast to float*

typedef struct
{
	float x;
	float y;
} cc_vec2f_t;

// dynamic constructor/destructor
cc_vec2f_t* cc_vec2f_new(float x, float y);
void        cc_vec2f_delete(cc_vec2f_t** _self);

// standard vector operations
void  cc_vec2f_load(cc_vec2f_t* self, float x, float y);
void  cc_vec2f_copy(const cc_vec2f_t* self,
                    cc_vec2f_t* copy);
int   cc_vec2f_equals(const cc_vec2f_t* self,
                      const cc_vec2f_t* v);
float cc_vec2f_mag(const cc_vec2f_t* self);
void  cc_vec2f_addv(cc_vec2f_t* self, const cc_vec2f_t* v);
void  cc_vec2f_addv_copy(const cc_vec2f_t* self,
                         const cc_vec2f_t* v,
                         cc_vec2f_t* copy);
void  cc_vec2f_adds(cc_vec2f_t* self, float s);
void  cc_vec2f_adds_copy(const cc_vec2f_t* self, float s,
                         cc_vec2f_t* copy);
void  cc_vec2f_subv(cc_vec2f_t* self, const cc_vec2f_t* v);
void  cc_vec2f_subv_copy(const cc_vec2f_t* self,
                         const cc_vec2f_t* v,
                         cc_vec2f_t* copy);
void  cc_vec2f_mulv(cc_vec2f_t* self, const cc_vec2f_t* v);
void  cc_vec2f_mulv_copy(const cc_vec2f_t* self,
                         const cc_vec2f_t* v,
                         cc_vec2f_t* copy);
void  cc_vec2f_muls(cc_vec2f_t* self, float s);
void  cc_vec2f_muls_copy(const cc_vec2f_t* self, float s,
                         cc_vec2f_t* copy);
void  cc_vec2f_normalize(cc_vec2f_t* self);
void  cc_vec2f_normalize_copy(const cc_vec2f_t* self,
                              cc_vec2f_t* copy);
float cc_vec2f_dot(const cc_vec2f_t* a,
                   const cc_vec2f_t* b);
float cc_vec2f_distance(const cc_vec2f_t* a,
                        const cc_vec2f_t* b);
float cc_vec2f_cross(const cc_vec2f_t* a,
                     const cc_vec2f_t* b);
void  cc_vec2f_quadraticBezier(const cc_vec2f_t* a,
                               const cc_vec2f_t* b,
                               const cc_vec2f_t* c,
                               float t,
                               cc_vec2f_t* p);
float cc_vec2f_triangleArea(const cc_vec2f_t* a,
                            const cc_vec2f_t* b,
                            const cc_vec2f_t* c);

#endif
