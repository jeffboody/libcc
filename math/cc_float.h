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

#ifndef cc_float_H
#define cc_float_H

// operators
float cc_ceil(float x);
float cc_clamp(float x, float min, float max);
float cc_floor(float x);
float cc_max(float a, float b);
float cc_min(float a, float b);
float cc_mix(float a, float b, float s);
float cc_round(float x);

// unit conversion
float cc_ft2mi(float x);
float cc_mi2ft(float x);
float cc_ft2m(float x);
float cc_m2ft(float x);
float cc_mi2m(float x);
float cc_m2mi(float x);
float cc_mVs2mph(float x);
float cc_mph2mVs(float x);
float cc_deg2rad(float x);
float cc_rad2deg(float x);

#endif
