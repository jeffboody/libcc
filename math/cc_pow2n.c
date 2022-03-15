/*
 * Copyright (c) 2021 Jeff Boody
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

#include <stdlib.h>

#define LOG_TAG "cc"
#include "../cc_log.h"
#include "cc_pow2n.h"

/***********************************************************
* public                                                   *
***********************************************************/

uint32_t cc_pow2n(uint32_t n)
{
	ASSERT((n >= 0) && (n <= 30));

	uint32_t pow2n[] =
	{
		1,          //  0
		2,          //  1
		4,          //  2
		8,          //  3
		16,         //  4
		32,         //  5
		64,         //  6
		128,        //  7
		256,        //  8
		512,        //  9
		1024,       // 10
		2048,       // 11
		4096,       // 12
		8192,       // 13
		16384,      // 14
		32768,      // 15
		65536,      // 16
		131072,     // 17
		262144,     // 18
		524288,     // 19
		1048576,    // 20
		2097152,    // 21
		4194304,    // 22
		8388608,    // 23
		16777216,   // 24
		33554432,   // 25
		67108864,   // 26
		134217728,  // 27
		268435456,  // 28
		536870912,  // 29
		1073741824, // 30
		2147483648, // 31
	};

	return pow2n[n];
}

uint32_t cc_next_pow2n(uint32_t v)
{
	// Devised by Sean Anderson, Sepember 14, 2001. Pete Hart
	// pointed me to a couple newsgroup posts by him and
	// William Lewis in February of 1997, where they arrive at
	// the same algorithm.
	// https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2

	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++;
	return v;
}

uint64_t cc_next_pow2n64(uint64_t v)
{
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v |= v >> 32;
	v++;
	return v;
}
