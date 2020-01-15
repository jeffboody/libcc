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
#include <string.h>

#define LOG_TAG "cc"
#include "../cc_log.h"
#include "../cc_memory.h"
#include "cc_orientation.h"

/***********************************************************
* private                                                  *
***********************************************************/

static void
cc_orientation_update(cc_orientation_t* self, int slerp)
{
	ASSERT(self);

	if((self->a_ts == 0.0) || (self->m_ts == 0.0))
	{
		// wait for sensors
		return;
	}

	// TODO - handle rotation and flat/vertical flags
	// compute the current rotation matrix
	cc_vec3f_t at;
	cc_vec3f_t up;
	cc_vec3f_t x;
	cc_vec3f_t y;
	cc_vec3f_t z;
	cc_vec3f_load(&up, self->a_ax, self->a_ay, self->a_az);
	cc_vec3f_load(&at, self->m_mx, self->m_my, self->m_mz);
	cc_vec3f_normalize_copy(&up, &z);
	cc_vec3f_cross_copy(&at, &z, &x);
	cc_vec3f_normalize(&x);
	cc_vec3f_cross_copy(&z, &x, &y);
	cc_vec3f_normalize(&y);

	cc_mat4f_t m;
	if(self->m_north == CC_ORIENTATION_TRUE)
	{
		// load the rotation matrix
		cc_mat4f_t r;
		r.m00 = x.x;
		r.m10 = y.x;
		r.m20 = z.x;
		r.m30 = 0.0f;
		r.m01 = x.y;
		r.m11 = y.y;
		r.m21 = z.y;
		r.m31 = 0.0f;
		r.m02 = x.z;
		r.m12 = y.z;
		r.m22 = z.z;
		r.m32 = 0.0f;
		r.m03 = 0.0f;
		r.m13 = 0.0f;
		r.m23 = 0.0f;
		r.m33 = 1.0f;

		// compute the true north rotation matrix
		cc_vec3f_load(&z, 0.0f, 0.0f, 1.0f);
		cc_vec3f_load(&at, self->m_gfx, self->m_gfy,
		              self->m_gfz);
		cc_vec3f_cross_copy(&at, &z, &x);
		cc_vec3f_normalize(&x);
		cc_vec3f_cross_copy(&z, &x, &y);
		cc_vec3f_normalize(&y);

		// load the true north rotation matrix
		cc_mat4f_t n;
		n.m00 = x.x;
		n.m10 = x.y;
		n.m20 = x.z;
		n.m30 = 0.0f;
		n.m01 = y.x;
		n.m11 = y.y;
		n.m21 = y.z;
		n.m31 = 0.0f;
		n.m02 = z.x;
		n.m12 = z.y;
		n.m22 = z.z;
		n.m32 = 0.0f;
		n.m03 = 0.0f;
		n.m13 = 0.0f;
		n.m23 = 0.0f;
		n.m33 = 1.0f;

		// correct for true north
		cc_mat4f_mulm_copy(&n, &r, &m);
	}
	else
	{
		m.m00 = x.x;
		m.m10 = y.x;
		m.m20 = z.x;
		m.m30 = 0.0f;
		m.m01 = x.y;
		m.m11 = y.y;
		m.m21 = z.y;
		m.m31 = 0.0f;
		m.m02 = x.z;
		m.m12 = y.z;
		m.m22 = z.z;
		m.m32 = 0.0f;
		m.m03 = 0.0f;
		m.m13 = 0.0f;
		m.m23 = 0.0f;
		m.m33 = 1.0f;
	}

	// convert matrix to quaternion
	cc_quaternion_t q;
	cc_quaternion_t out;
	cc_mat4f_quaternion(&m, &q);

	if(slerp)
	{
		// derive slerp from err
		float err = cc_quaternion_compare(&q, &self->Q);
		float t   = 0.001f + err*err;

		// clamp the slerp
		if(t > 0.004f)
		{
			t = 0.004f;
		}

		// update the filtered rotation quaternion
		cc_quaternion_slerp(&self->Q, &q, t, &out);
		cc_quaternion_copy(&out, &self->Q);
	}
	else
	{
		cc_quaternion_copy(&q, &self->Q);
	}
}

/***********************************************************
* public                                                   *
***********************************************************/

cc_orientation_t* cc_orientation_new(void)
{
	cc_orientation_t* self;
	self = (cc_orientation_t*)
	       MALLOC(sizeof(cc_orientation_t));
	if(self == NULL)
	{
		LOGE("MALLOC failed");
		return NULL;
	}

	self->m_north = CC_ORIENTATION_TRUE;
	cc_orientation_reset(self);

	return self;
}

void cc_orientation_delete(cc_orientation_t** _self)
{
	ASSERT(_self);

	cc_orientation_t* self = *_self;
	if(self)
	{
		FREE(self);
		*_self = NULL;
	}
}

void cc_orientation_reset(cc_orientation_t* self)
{
	ASSERT(self);

	self->a_ts       = 0.0;
	self->a_ax       = 0.0f;
	self->a_ay       = 0.0f;
	self->a_az       = 9.8f;
	self->a_rotation = 0;
	self->m_ts       = 0.0;
	self->m_mx       = 0.0f;
	self->m_my       = 1.0f;
	self->m_mz       = 0.0f;
	self->m_gfx      = 0.0f;
	self->m_gfy      = 1.0f;
	self->m_gfz      = 0.0f;
	self->g_ts       = 0.0;
	self->g_ax       = 0.0f;
	self->g_ay       = 0.0f;
	self->g_az       = 0.0f;

	cc_quaternion_identity(&self->Q);
}

void cc_orientation_accelerometer(cc_orientation_t* self,
                                  double ts,
                                  float ax, float ay,
                                  float az,
                                  int rotation)
{
	ASSERT(self);

	// reset orientation when rotation changes
	// because this often times happens when
	// performing figure 8 motion to calibrate
	// the compass
	if(rotation != self->a_rotation)
	{
		cc_orientation_reset(self);
	}

	// don't slerp for the first rotation
	int slerp = 1;
	if((self->a_ts == 0.0) && (self->m_ts > 0.0))
	{
		slerp = 0;
	}

	self->a_ts       = ts;
	self->a_ax       = ax;
	self->a_ay       = ay;
	self->a_az       = az;
	self->a_rotation = rotation;

	cc_orientation_update(self, slerp);
}

void cc_orientation_magnetometer(cc_orientation_t* self,
                                 double ts,
                                 float mx, float my,
                                 float mz,
                                 float gfx, float gfy,
                                 float gfz)
{
	ASSERT(self);

	// don't slerp for the first rotation
	int slerp = 1;
	if((self->m_ts == 0.0) && (self->a_ts > 0.0))
	{
		slerp = 0;
	}

	self->m_ts  = ts;
	self->m_mx  = mx;
	self->m_my  = my;
	self->m_mz  = mz;
	self->m_gfx = gfx;
	self->m_gfy = gfy;
	self->m_gfz = gfz;

	cc_orientation_update(self, slerp);
}

void cc_orientation_gyroscope(cc_orientation_t* self,
                              double ts,
                              float ax, float ay, float az)
{
	ASSERT(self);

	// https://developer.android.com/guide/topics/sensors/sensors_motion.html
	// http://www.flipcode.com/documents/matrfaq.html#Q56

	// update the filtered estimate
	if((self->a_ts > 0.0) && (self->m_ts > 0.0) &&
	   (self->g_ts > 0.0))
	{
		float dt    = (float) (ts - self->g_ts);
		float mag   = sqrtf(ax*ax + ay*ay + az*az);
		float angle = (180.0f/M_PI)*mag*dt;


		cc_quaternion_t q;
		cc_quaternion_loadaxisangle(&q, ax, ay, az, angle);
		cc_quaternion_rotateq(&self->Q, &q);
	}

	self->g_ts = ts;
	self->g_ax = ax;
	self->g_ay = ay;
	self->g_az = az;
}

void cc_orientation_mat4f(cc_orientation_t* self,
                          cc_mat4f_t* m)
{
	ASSERT(self);
	ASSERT(m);

	cc_mat4f_rotateq(m, 1, &self->Q);
}

void cc_orientation_vpn(cc_orientation_t* self,
                        float* vx, float* vy, float* vz)
{
	ASSERT(self);
	ASSERT(vx);
	ASSERT(vy);
	ASSERT(vz);

	// vpn is the negative z-axis of the
	// transpose/inverse rotation matrix
	cc_mat4f_t m;
	cc_orientation_mat4f(self, &m);
	*vx = -m.m02;
	*vy = -m.m12;
	*vz = -m.m22;
}

void cc_orientation_spherical(cc_orientation_t* self,
                              float* theta,
                              float* phi)
{
	ASSERT(self);
	ASSERT(theta);
	ASSERT(phi);

	cc_mat4f_t m;
	cc_orientation_mat4f(self, &m);

	// load the transpose/inverse vectors
	cc_vec3f_t x;
	cc_vec3f_t y;
	cc_vec3f_t z;
	cc_vec3f_load(&x, m.m00, m.m10, m.m20);
	cc_vec3f_load(&y, m.m01, m.m11, m.m21);
	cc_vec3f_load(&z, m.m02, m.m12, m.m22);

	*theta = (180.0f/M_PI)*atan2f(z.y, z.x);
	while(*theta < 0.0f)
	{
		*theta += 360.0f;
	}
	while(*theta >= 360.0f)
	{
		*theta -= 360.0f;
	}

	*phi = (180.0f/M_PI)*asinf(z.z);
}

void cc_orientation_euler(cc_orientation_t* self,
                          float* yaw, float* pitch,
                          float* roll)
{
	ASSERT(self);
	ASSERT(yaw);
	ASSERT(pitch);
	ASSERT(roll);

	cc_mat4f_t m;
	cc_orientation_mat4f(self, &m);

	cc_vec3f_t x;
	cc_vec3f_t y;
	cc_vec3f_t z;
	cc_vec3f_load(&x, m.m00, m.m01, m.m02);
	cc_vec3f_load(&y, m.m10, m.m11, m.m12);
	cc_vec3f_load(&z, m.m20, m.m21, m.m22);

	// TODO - fix euler angles per
	// https://en.wikipedia.org/wiki/Euler_angles
	// TODO - euler corner cases
	// convert to euler angles
	*yaw   = (180.0f/M_PI)*(atan2f(-y.x, y.y));
	*pitch = (180.0f/M_PI)*(acosf(y.z));
	*roll  = (180.0f/M_PI)*(-atan2f(x.z, z.z));

	// TODO - handle rotation
	// workaround screen rotations for yaw
	*yaw += (float) self->a_rotation;
	while(*yaw < 0.0f)
	{
		*yaw += 360.0f;
	}
	while(*yaw > 360.0f)
	{
		*yaw -= 360.0f;
	}
}
