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

#ifndef cc_jsmnStream_H
#define cc_jsmnStream_H

#include <stdint.h>

#include "cc_jsmnWrapper.h"

#define CC_JSMN_STREAM_MAX_DEPTH 32

typedef struct
{
	cc_jsmnType_e type;
	int           has_key;
	int           has_end;
	uint32_t      count;
} cc_jsmnState_t;

typedef struct
{
	size_t         len;
	char*          buffer;
	int            err;
	uint32_t       depth;
	cc_jsmnState_t state[CC_JSMN_STREAM_MAX_DEPTH];
} cc_jsmnStream_t;

cc_jsmnStream_t* cc_jsmnStream_new(void);
void             cc_jsmnStream_delete(cc_jsmnStream_t** _self);
int              cc_jsmnStream_export(cc_jsmnStream_t* self,
                                      const char* fname);
const char*      cc_jsmnStream_buffer(cc_jsmnStream_t* self,
                                      size_t* _size);
int              cc_jsmnStream_beginObject(cc_jsmnStream_t* self);
int              cc_jsmnStream_beginArray(cc_jsmnStream_t* self);
int              cc_jsmnStream_end(cc_jsmnStream_t* self);
int              cc_jsmnStream_key(cc_jsmnStream_t* self,
                                   const char* fmt, ...);
int              cc_jsmnStream_string(cc_jsmnStream_t* self,
                                      const char* fmt, ...);
int              cc_jsmnStream_true(cc_jsmnStream_t* self);
int              cc_jsmnStream_false(cc_jsmnStream_t* self);
int              cc_jsmnStream_null(cc_jsmnStream_t* self);
int              cc_jsmnStream_int(cc_jsmnStream_t* self,
                                   int val);
int              cc_jsmnStream_float(cc_jsmnStream_t* self,
                                     float val);
int              cc_jsmnStream_double(cc_jsmnStream_t* self,
                                      double val);

#endif
