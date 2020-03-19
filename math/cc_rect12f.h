/*
 * Copyright (c) 2019 Jeff Boody
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

#ifndef cc_rect12f_H
#define cc_rect12f_H

typedef struct
{
	float t;
	float l;
	float w;
	float h;
} cc_rect1f_t;

typedef struct
{
	float t;
	float l;
	float b;
	float r;
} cc_rect2f_t;

void cc_rect1f_init(cc_rect1f_t* self,
                    float t, float l,
                    float w, float h);
void cc_rect1f_copy(const cc_rect1f_t* self,
                    cc_rect1f_t* copy);
void cc_rect1f_copy2f(const cc_rect1f_t* self,
                      cc_rect2f_t* copy);
int  cc_rect1f_contains(const cc_rect1f_t* self,
                        float x, float y);
int cc_rect1f_intersect(const cc_rect1f_t* a,
                        const cc_rect1f_t* b,
                        cc_rect1f_t* c);
int cc_rect1f_equals(const cc_rect1f_t* self,
                     const cc_rect1f_t* rect);

void cc_rect2f_init(cc_rect2f_t* self,
                    float t, float l,
                    float b, float r);
void cc_rect2f_copy(const cc_rect2f_t* self,
                    cc_rect2f_t* copy);
void cc_rect2f_copy1f(const cc_rect2f_t* self,
                      cc_rect1f_t* copy);
int  cc_rect2f_contains(const cc_rect2f_t* self,
                        float x, float y);
int cc_rect2f_intersect(const cc_rect2f_t* a,
                        const cc_rect2f_t* b,
                        cc_rect2f_t* c);
int cc_rect2f_equals(const cc_rect2f_t* self,
                     const cc_rect2f_t* rect);

#endif
