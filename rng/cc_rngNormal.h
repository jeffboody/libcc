/*
 * Copyright (c) 2023 Jeff Boody
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

#ifndef cc_rngNormal_H
#define cc_rngNormal_H

#include "../../pcg-c-basic/pcg_basic.h"

// normal/gaussian distribution of random numbers
typedef struct
{
	pcg32_random_t rng;
	double         mu;
	double         sigma;
	int            phase;
	double         rand;
} cc_rngNormal_t;

void   cc_rngNormal_init(cc_rngNormal_t* self,
                         double mu,
                         double sigma);
void   cc_rngNormal_initSeed(cc_rngNormal_t* self,
                             double mu,
                             double sigma,
                             uint64_t initstate,
                             uint64_t initseq);
void   cc_rngNormal_reset(cc_rngNormal_t* self,
                          double mu,
                          double sigma);
float  cc_rngNormal_rand1F(cc_rngNormal_t* self);
double cc_rngNormal_rand1D(cc_rngNormal_t* self);

#endif
