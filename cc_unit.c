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

#include <math.h>
#include <stdlib.h>

#define LOG_TAG "cc"
#include "cc_log.h"
#include "cc_unit.h"

/***********************************************************
* public                                                   *
***********************************************************/

float cc_ft2mi(float x)
{
	return x/5280.0f;
}

float cc_mi2ft(float x)
{
	return 5280.0f*x;
}

float cc_ft2m(float x)
{
	return x*1200.0f/3937.0f;
}

float cc_m2ft(float x)
{
	return x*3937.0f/1200.0f;
}

float cc_mi2m(float x)
{
	return x*5280.0f*1200.0f/3937.0f;
}

float cc_m2mi(float x)
{
	return x*3937.0f/(5280.0f*1200.0f);
}

float cc_mVs2mph(float x)
{
	// mVs is m/s
	return cc_m2mi(x)*3600.0f;
}

float cc_mph2mVs(float x)
{
	// mVs is m/s
	return cc_mi2m(x)/3600.0f;
}

float cc_deg2rad(float x)
{
	return x*M_PI/180.0f;
}

float cc_rad2deg(float x)
{
	return x*180.0f/M_PI;
}
