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

#ifndef cc_orientation_H
#define cc_orientation_H

#include "cc_mat4f.h"
#include "cc_quaternion.h"

#define CC_ORIENTATION_TRUE     0
#define CC_ORIENTATION_MAGNETIC 1

typedef struct
{
	// accelerometer
	double a_ts;
	float  a_ax;
	float  a_ay;
	float  a_az;
	int    a_rotation;

	// magnetometer
	int    m_north;
	double m_ts;
	float  m_mx;
	float  m_my;
	float  m_mz;
	float  m_gfx;
	float  m_gfy;
	float  m_gfz;

	// gyroscope
	double g_ts;
	float  g_ax;
	float  g_ay;
	float  g_az;

	// filtered rotation quaternion
	cc_quaternion_t Q;
} cc_orientation_t;

cc_orientation_t* cc_orientation_new(void);
void              cc_orientation_delete(cc_orientation_t** _self);
void              cc_orientation_reset(cc_orientation_t* self);
void              cc_orientation_accelerometer(cc_orientation_t* self,
                                               double ts,
                                               float ax,
                                               float ay,
                                               float az,
                                               int rotation);
void              cc_orientation_magnetometer(cc_orientation_t* self,
                                              double ts,
                                              float mx,
                                              float my,
                                              float mz,
                                              float gfx,
                                              float gfy,
                                              float gfz);
void              cc_orientation_gyroscope(cc_orientation_t* self,
                                           double ts,
                                           float ax,
                                           float ay,
                                           float az);
void              cc_orientation_mat4f(cc_orientation_t* self,
                                       cc_mat4f_t* m);
void              cc_orientation_vpn(cc_orientation_t* self,
                                     float* vx,
                                     float* vy,
                                     float* vz);
void              cc_orientation_spherical(cc_orientation_t* self,
                                           float* theta,
                                           float* phi);
void              cc_orientation_euler(cc_orientation_t* self,
                                       float* yaw,
                                       float* pitch,
                                       float* roll);

#endif
