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

#include <math.h>
#include <stdlib.h>

#define LOG_TAG "cc"
#include "../cc_log.h"
#include "../cc_memory.h"
#include "cc_stack4f.h"

/***********************************************************
* public                                                   *
***********************************************************/

cc_stack4f_t* cc_stack4f_new(void)
{
	cc_stack4f_t* self = (cc_stack4f_t*) CALLOC(1, sizeof(cc_stack4f_t));
	if(self == NULL)
	{
		LOGE("CALLOC failed");
		return NULL;
	}

	self->matrix_stack = cc_list_new();
	if(self->matrix_stack == NULL)
	{
		goto fail_matrix_stack;
	}

	// success
	return self;

	// failure
	fail_matrix_stack:
		FREE(self);
	return NULL;
}

void cc_stack4f_delete(cc_stack4f_t** _self)
{
	// *_self can be null
	ASSERT(_self);

	cc_stack4f_t* self = *_self;
	if(self)
	{
		cc_listIter_t* iter = cc_list_head(self->matrix_stack);
		while(iter)
		{
			cc_mat4f_t* m;
			m = (cc_mat4f_t*)
			    cc_list_remove(self->matrix_stack, &iter);
			FREE(m);
		}
		cc_list_delete(&self->matrix_stack);
		FREE(self);
		*_self = NULL;
	}
}

void cc_stack4f_push(cc_stack4f_t* self, const cc_mat4f_t* m)
{
	ASSERT(self);
	ASSERT(m);

	cc_mat4f_t* c = (cc_mat4f_t*) CALLOC(1, sizeof(cc_mat4f_t));
	if(c == NULL)
	{
		LOGE("CALLOC failed");
		return;
	}
	cc_mat4f_copy(m, c);
	cc_list_insert(self->matrix_stack, NULL, (const void*) c);
}

void cc_stack4f_pop(cc_stack4f_t* self, cc_mat4f_t* m)
{
	ASSERT(self);
	ASSERT(m);

	cc_listIter_t* iter = cc_list_head(self->matrix_stack);
	if(iter)
	{
		cc_mat4f_t* c;
		c = (cc_mat4f_t*)
		    cc_list_remove(self->matrix_stack, &iter);
		cc_mat4f_copy(c, m);
		FREE(c);
	}
}
