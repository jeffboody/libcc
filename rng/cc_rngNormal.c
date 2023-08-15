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

#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define LOG_TAG "cc"
#include "../cc_log.h"
#include "cc_rngNormal.h"

/***********************************************************
* private                                                  *
***********************************************************/

static double
cc_rngNormal_boxMullerTransform(cc_rngNormal_t* self)
{
	ASSERT(self);

	// Ported from
	// https://en.wikipedia.org/wiki/Box%E2%80%93Muller_transform

	const double epsilon = DBL_EPSILON;
	const double two_pi  = 2.0*M_PI;

	// create two uniformly distributed random numbers
	// from 0.0 to 1.0 and make sure u1 is greater than epsilon
	double u1;
	double u2;
	do
	{
		u1 = ldexp(pcg32_random_r(&self->rng), -32);
	} while (u1 <= epsilon);
	u2 = ldexp(pcg32_random_r(&self->rng), -32);

	// compute z0 and z1
	double mag = self->sigma*sqrt(-2.0*log(u1));
	double z0  = mag*cos(two_pi*u2) + self->mu;
	double z1  = mag*sin(two_pi*u2) + self->mu;

	// save z1 for the next request
	self->phase = 1;
	self->rand  = z1;

	return z0;
}

/***********************************************************
* public                                                   *
***********************************************************/

void cc_rngNormal_init(cc_rngNormal_t* self,
                       double mu,
                       double sigma)
{
	ASSERT(self);

	uint64_t initstate = time(NULL) ^ (intptr_t)&printf;
	uint64_t initseq   = 0;
	cc_rngNormal_initSeed(self, mu, sigma,
	                      initstate, initseq);
}

void cc_rngNormal_initSeed(cc_rngNormal_t* self,
                           double mu,
                           double sigma,
                           uint64_t initstate,
                           uint64_t initseq)
{
	ASSERT(self);

	#ifdef CC_RNG_DEBUG
	pcg32_srandom_r(&self->rng, 42u, 54u);
	#else
	pcg32_srandom_r(&self->rng, initstate, initseq);
	#endif

	self->mu    = mu;
	self->sigma = sigma;
	self->phase = 0;
}

void cc_rngNormal_reset(cc_rngNormal_t* self,
                        double mu,
                        double sigma)
{
	ASSERT(self);

	self->mu    = mu;
	self->sigma = sigma;
	self->phase = 0;
}

float cc_rngNormal_rand1F(cc_rngNormal_t* self)
{
	ASSERT(self);

	if(self->phase)
	{
		self->phase = 0;
		return (float) self->rand;
	}

	return (float) cc_rngNormal_boxMullerTransform(self);
}

double cc_rngNormal_rand1D(cc_rngNormal_t* self)
{
	ASSERT(self);

	if(self->phase)
	{
		self->phase = 0;
		return self->rand;
	}

	return cc_rngNormal_boxMullerTransform(self);
}
