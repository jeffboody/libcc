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

#ifndef cc_doubleSingle_H
#define cc_doubleSingle_H

#include "cc_vec2f.h"
#include "cc_vec3d.h"
#include "cc_vec3f.h"

// https://blog.cyclemap.link/2011-06-09-glsl-part2-emu/
// https://prideout.net/emulating-double-precision

void cc_doubleSingle_set(double in,
                         cc_vec2f_t* out);
void cc_doubleSingle_get(cc_vec2f_t* in,
                         double* _out);
void cc_doubleSingle_add(cc_vec2f_t* a,
                         cc_vec2f_t* b,
                         cc_vec2f_t* c);
void cc_doubleSingle_mul(cc_vec2f_t* a,
                         cc_vec2f_t* b,
                         cc_vec2f_t* c);
void cc_doubleSingle_set3(cc_vec3d_t* in,
                          cc_vec3f_t* high,
                          cc_vec3f_t* low);
void cc_doubleSingle_get3(cc_vec3f_t* high,
                          cc_vec3f_t* low,
                          cc_vec3d_t* out);
void cc_doubleSingle_add3(cc_vec3f_t* aH,
                          cc_vec3f_t* aL,
                          cc_vec3f_t* bH,
                          cc_vec3f_t* bL,
                          cc_vec3f_t* cH,
                          cc_vec3f_t* cL);
void cc_doubleSingle_mul3(cc_vec3f_t* aH,
                          cc_vec3f_t* aL,
                          cc_vec3f_t* bH,
                          cc_vec3f_t* bL,
                          cc_vec3f_t* cH,
                          cc_vec3f_t* cL);

#endif
