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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LOG_TAG "cc"
#include "../../libcc/cc_log.h"
#include "../../libcc/cc_memory.h"
#include "cc_jsmnStream.h"

#define CC_JSMN_STREAM_STRING_SIZE 256

/***********************************************************
* private                                                  *
***********************************************************/

static cc_jsmnState_t*
cc_jsmnStream_state(cc_jsmnStream_t* self)
{
	ASSERT(self);

	return &self->state[self->depth];
}

static void cc_jsmnStream_error(cc_jsmnStream_t* self)
{
	ASSERT(self);

	cc_jsmnState_t* state = cc_jsmnStream_state(self);

	LOGE("depth=%u, type=%i, has_key=%i, has_end=%i, count=%u",
	     self->depth, state->type, state->has_key,
	     state->has_end, state->count);

	self->err = 1;
}

static int cc_jsmnStream_cat(cc_jsmnStream_t* self, char* str)
{
	ASSERT(self);
	ASSERT(str);

	char* a = self->buffer;
	char* b = str;

	// compute size/length
	size_t size1 = MEMSIZEPTR(a);
	size_t lena  = self->len;
	size_t lenb  = strlen(b);
	size_t size2 = lena + lenb + 1;

	// resize buffer
	if(size1 < size2)
	{
		size_t size = size1;
		if(size1 == 0)
		{
			size = 256;
		}

		while(size < size2)
		{
			size *= 2;
		}

		char* tmp = (char*) REALLOC(a, size);
		if(tmp == NULL)
		{
			LOGE("REALLOC failed");
			cc_jsmnStream_error(self);
			return 0;
		}

		a            = tmp;
		self->buffer = a;
	}

	// copy buffer
	int i;
	for(i = 0; i < lenb; ++i)
	{
		a[i + lena] = b[i];
	}
	a[lena + lenb] = '\0';

	// update len
	self->len = lena + lenb;

	return 1;
}

static int
cc_jsmnStream_val(cc_jsmnStream_t* self,
                  cc_jsmnType_e type, char* str)
{
	ASSERT(self);
	ASSERT(type != CC_JSMN_TYPE_UNDEFINED);

	cc_jsmnState_t* state = cc_jsmnStream_state(self);

	// add vals to the root, objects or arrays
	int inc_depth = 0;
	if(state->type == CC_JSMN_TYPE_UNDEFINED)
	{
		// initialize state
		state->type    = type;
		state->has_key = 0;
		state->count   = 0;

		// terminate strings or primitives
		if((type == CC_JSMN_TYPE_STRING) ||
		   (type == CC_JSMN_TYPE_PRIMITIVE))
		{
			state->has_end = 1;
		}
		else
		{
			state->has_end = 0;
		}
	}
	else if((state->type == CC_JSMN_TYPE_OBJECT) &&
	        (state->has_key) && (state->has_end == 0))
	{
		if((type == CC_JSMN_TYPE_OBJECT) ||
		   (type == CC_JSMN_TYPE_ARRAY))
		{
			inc_depth = 1;
		}

		// consume the key
		state->has_key = 0;

		// count is incremented when adding the key
	}
	else if((state->type == CC_JSMN_TYPE_ARRAY) &&
	        (state->has_end == 0))
	{
		if((type == CC_JSMN_TYPE_OBJECT) ||
		   (type == CC_JSMN_TYPE_ARRAY))
		{
			inc_depth = 1;
		}

		if(state->count)
		{
			if(cc_jsmnStream_cat(self, ",") == 0)
			{
				return 0;
			}
		}
		++state->count;
	}
	else
	{
		LOGE("invalid");
		cc_jsmnStream_error(self);
		return 0;
	}

	// increment the state depth
	if(inc_depth)
	{
		uint32_t depth = self->depth + 1;
		if(depth >= CC_JSMN_STREAM_MAX_DEPTH)
		{
			LOGE("invalid");
			cc_jsmnStream_error(self);
			return 0;
		}
		self->depth = depth;

		// initialize state
		state          = cc_jsmnStream_state(self);
		state->type    = type;
		state->has_key = 0;
		state->has_end = 0;
		state->count   = 0;
	}

	// append the val
	if(type == CC_JSMN_TYPE_STRING)
	{
		return cc_jsmnStream_cat(self, "\"") &&
		       cc_jsmnStream_cat(self, str)  &&
		       cc_jsmnStream_cat(self, "\"");
	}
	else
	{
		return cc_jsmnStream_cat(self, str);
	}
}

/***********************************************************
* public                                                   *
***********************************************************/

cc_jsmnStream_t* cc_jsmnStream_new(void)
{
	cc_jsmnStream_t* self;
	self = (cc_jsmnStream_t*)
	       CALLOC(1, sizeof(cc_jsmnStream_t));
	if(self == NULL)
	{
		LOGE("CALLOC failed");
		return NULL;
	}

	return self;
}

void cc_jsmnStream_delete(cc_jsmnStream_t** _self)
{
	ASSERT(_self);

	cc_jsmnStream_t* self = *_self;
	if(self)
	{
		FREE(self->buffer);
		FREE(self);
		*_self = NULL;
	}
}

int cc_jsmnStream_export(cc_jsmnStream_t* self,
                       const char* fname)
{
	ASSERT(self);
	ASSERT(fname);

	size_t size = 0;
	const char* buf = cc_jsmnStream_buffer(self, &size);
	if(buf == NULL)
	{
		return 0;
	}

	FILE* f = fopen(fname, "w");
	if(f == NULL)
	{
		LOGE("invalid %s", fname);
		return 0;
	}

	fprintf(f, "%s", buf);
	fclose(f);

	return 1;
}

const char*
cc_jsmnStream_buffer(cc_jsmnStream_t* self, size_t* _size)
{
	ASSERT(self);
	ASSERT(_size);

	cc_jsmnState_t* state = cc_jsmnStream_state(self);

	*_size = 0;

	// check for end of stream
	if((self->err   == 0) &&
	   (self->depth == 0) &&
	   (state->type != CC_JSMN_TYPE_UNDEFINED) &&
	   (state->has_key == 0) && (state->has_end))
	{
		// ok
	}
	else
	{
		LOGE("invalid");
		cc_jsmnStream_error(self);
		return NULL;
	}

	if(self->buffer)
	{
		*_size = self->len + 1;
	}
	return self->buffer;
}

int cc_jsmnStream_beginObject(cc_jsmnStream_t* self)
{
	ASSERT(self);

	return cc_jsmnStream_val(self, CC_JSMN_TYPE_OBJECT, "{");
}

int cc_jsmnStream_beginArray(cc_jsmnStream_t* self)
{
	ASSERT(self);

	return cc_jsmnStream_val(self, CC_JSMN_TYPE_ARRAY, "[");
}

int cc_jsmnStream_end(cc_jsmnStream_t* self)
{
	ASSERT(self);

	cc_jsmnState_t* state = cc_jsmnStream_state(self);
	if((state->type == CC_JSMN_TYPE_OBJECT) &&
	   (state->has_key == 0) && (state->has_end == 0))
	{
		if(cc_jsmnStream_cat(self, "}") == 0)
		{
			return 0;
		}
	}
	else if((state->type == CC_JSMN_TYPE_ARRAY) &&
	        (state->has_end == 0))
	{
		if(cc_jsmnStream_cat(self, "]") == 0)
		{
			return 0;
		}
	}
	else
	{
		LOGE("invalid");
		cc_jsmnStream_error(self);
		return 0;
	}

	state->has_end = 1;

	if(self->depth > 0)
	{
		--self->depth;
	}

	return 1;
}

int cc_jsmnStream_key(cc_jsmnStream_t* self,
                    const char* fmt, ...)
{
	ASSERT(self);
	ASSERT(fmt);

	cc_jsmnState_t* state = cc_jsmnStream_state(self);
	if((state->type == CC_JSMN_TYPE_OBJECT) &&
	   (state->has_key == 0) && (state->has_end == 0))
	{
		state->has_key = 1;
	}
	else
	{
		LOGE("invalid");
		cc_jsmnStream_error(self);
		return 0;
	}

	if(state->count)
	{
		if(cc_jsmnStream_cat(self, ",") == 0)
		{
			return 0;
		}
	}
	++state->count;

	char str[CC_JSMN_STREAM_STRING_SIZE];
	va_list argptr;
	va_start(argptr, fmt);
	vsnprintf(str, CC_JSMN_STREAM_STRING_SIZE, fmt, argptr);
	va_end(argptr);

	return cc_jsmnStream_cat(self, "\"") &&
	       cc_jsmnStream_cat(self, str)  &&
	       cc_jsmnStream_cat(self, "\"") &&
	       cc_jsmnStream_cat(self, ":");
}

int cc_jsmnStream_string(cc_jsmnStream_t* self,
                       const char* fmt, ...)
{
	ASSERT(self);
	ASSERT(fmt);

	char str[CC_JSMN_STREAM_STRING_SIZE];
	va_list argptr;
	va_start(argptr, fmt);
	vsnprintf(str, CC_JSMN_STREAM_STRING_SIZE, fmt, argptr);
	va_end(argptr);

	return cc_jsmnStream_val(self, CC_JSMN_TYPE_STRING, str);
}

int cc_jsmnStream_true(cc_jsmnStream_t* self)
{
	ASSERT(self);

	return cc_jsmnStream_val(self, CC_JSMN_TYPE_PRIMITIVE,
	                       "true");
}

int cc_jsmnStream_false(cc_jsmnStream_t* self)
{
	ASSERT(self);

	return cc_jsmnStream_val(self, CC_JSMN_TYPE_PRIMITIVE,
	                       "false");
}

int cc_jsmnStream_null(cc_jsmnStream_t* self)
{
	ASSERT(self);

	return cc_jsmnStream_val(self, CC_JSMN_TYPE_PRIMITIVE,
	                       "null");
}

int cc_jsmnStream_int(cc_jsmnStream_t* self, int val)
{
	ASSERT(self);

	char str[256];
	snprintf(str, 256, "%i", val);

	return cc_jsmnStream_val(self, CC_JSMN_TYPE_PRIMITIVE, str);
}

int cc_jsmnStream_float(cc_jsmnStream_t* self, float val)
{
	ASSERT(self);

	char str[256];
	snprintf(str, 256, "%f", val);

	return cc_jsmnStream_val(self, CC_JSMN_TYPE_PRIMITIVE, str);
}

int cc_jsmnStream_double(cc_jsmnStream_t* self, double val)
{
	ASSERT(self);

	char str[256];
	snprintf(str, 256, "%lf", val);

	return cc_jsmnStream_val(self, CC_JSMN_TYPE_PRIMITIVE, str);
}
