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

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define LOG_TAG "cc"
#include "../cc_log.h"
#include "cc_rngUniform.h"

/***********************************************************
* public                                                   *
***********************************************************/

void cc_rngUniform_init(cc_rngUniform_t* self)
{
	ASSERT(self);

	uint64_t initstate = time(NULL) ^ (intptr_t)&printf;
	uint64_t initseq   = 0;
	cc_rngUniform_initSeed(self, initstate, initseq);
}

void cc_rngUniform_initSeed(cc_rngUniform_t* self,
                            uint64_t initstate,
                            uint64_t initseq)
{
	ASSERT(self);

	#ifdef CC_RNG_DEBUG
	pcg32_srandom_r(&self->rng, 42u, 54u);
	#else
	pcg32_srandom_r(&self->rng, initstate, initseq);
	#endif
}

uint32_t cc_rngUniform_rand1U(cc_rngUniform_t* self)
{
	ASSERT(self);

	return pcg32_random_r(&self->rng);
}

uint32_t cc_rngUniform_rand2U(cc_rngUniform_t* self,
                              uint32_t min, uint32_t max)
{
	ASSERT(self);

	// [min, max]
	uint32_t bound = max - min + 1;
	return pcg32_boundedrand_r(&self->rng, bound) + min;
}

int cc_rngUniform_rand2I(cc_rngUniform_t* self,
                         int min, int max)
{
	ASSERT(self);

	// [min, max]
	uint32_t bound = (uint32_t) (max - min + 1);
	return (int) pcg32_boundedrand_r(&self->rng, bound) + min;
}

float cc_rngUniform_rand1F(cc_rngUniform_t* self)
{
	ASSERT(self);

	// [0.0f, 1.0f)
	return (float) ldexp(pcg32_random_r(&self->rng), -32);
}

float cc_rngUniform_rand2F(cc_rngUniform_t* self,
                           float min, float max)
{
	ASSERT(self);

	// [min, max)
	float rand;
	rand = (float) ldexp(pcg32_random_r(&self->rng), -32);
	return rand*(max - min) + min;
}

double cc_rngUniform_rand1D(cc_rngUniform_t* self)
{
	ASSERT(self);

	// [0.0, 1.0)
	return ldexp(pcg32_random_r(&self->rng), -32);
}

double cc_rngUniform_rand2D(cc_rngUniform_t* self,
                            double min, double max)
{
	ASSERT(self);

	// [min, max)
	double rand;
	rand = ldexp(pcg32_random_r(&self->rng), -32);
	return rand*(max - min) + min;
}
