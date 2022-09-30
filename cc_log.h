/*
 * Copyright (c) 2009-2010 Jeff Boody
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

#ifndef cc_log_H
#define cc_log_H

/***********************************************************
* Macros are based on Android's logging mechanism found in *
* system/core/include/cutils/log.h                         *
*                                                          *
* Before including this file declare LOG_TAG as follows:   *
* #define LOG_TAG "tag"                                    *
*                                                          *
* Declare LOG_DEBUG before including cc_log.h to enable    *
* debugging.                                               *
*                                                          *
* LOG{DIWE}("") will output func@line with no message      *
***********************************************************/

// support non-Android environments
#ifdef ANDROID
	#include <android/log.h>
#else
	#define ANDROID_LOG_DEBUG 0
	#define ANDROID_LOG_INFO  1
	#define ANDROID_LOG_WARN  2
	#define ANDROID_LOG_ERROR 3
#endif
#include <assert.h>

#ifndef LOG_TAG
	#define LOG_TAG NULL
#endif

// logging using Android "standard" macros
void cc_log(const char* func, int line, int type,
            const char* tag, const char* fmt, ...);
void cc_assert(const char* func, int line,
               const char* tag, const char* expr);

// tracing using Android Systrace
void cc_trace_init(void);
void cc_trace_begin(const char* func, int line);
void cc_trace_end(void);

#ifndef LOGD
	#ifdef LOG_DEBUG
		#define LOGD(...) (cc_log(__func__, __LINE__, ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__))
	#else
		#define LOGD(...)
	#endif
#endif

#ifndef LOGI
	#define LOGI(...) (cc_log(__func__, __LINE__, ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__))
#endif

#ifndef LOGW
	#define LOGW(...) (cc_log(__func__, __LINE__, ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__))
#endif

#ifndef LOGE
	#define LOGE(...) (cc_log(__func__, __LINE__, ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__))
#endif

#ifndef ASSERT
	#ifdef ASSERT_DEBUG
		#define ASSERT(expr) do { if(!(expr)) { cc_assert(__func__, __LINE__, LOG_TAG, #expr); } } while(0)
	#else
		#define ASSERT(expr)
	#endif
#endif

#ifndef TRACE_INIT
	#ifdef TRACE_DEBUG
		#define TRACE_INIT() (cc_trace_init())
	#else
		#define TRACE_INIT()
	#endif
#endif

#ifndef TRACE_BEGIN
	#ifdef TRACE_DEBUG
		#define TRACE_BEGIN() (cc_trace_begin(__func__, __LINE__))
	#else
		#define TRACE_BEGIN()
	#endif
#endif

#ifndef TRACE_END
	#ifdef TRACE_DEBUG
		#define TRACE_END() (cc_trace_end())
	#else
		#define TRACE_END()
	#endif
#endif

#endif
