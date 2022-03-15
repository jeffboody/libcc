/*
 * Copyright (c) 2018 Jeff Boody
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
#include "cc_log.h"
#include "cc_memory.h"
#include "cc_multimap.h"

/***********************************************************
* public                                                   *
***********************************************************/

cc_multimap_t* cc_multimap_new(cc_listcmp_fn compare)
{
	// compare may be NULL

	cc_multimap_t* self;
	self = (cc_multimap_t*) MALLOC(sizeof(cc_multimap_t));
	if(self == NULL)
	{
		LOGE("MALLOC failed");
		return NULL;
	}

	self->map = cc_map_new();
	if(self->map == NULL)
	{
		goto fail_map;
	}

	self->compare = compare;

	// success
	return self;

	// failure
	fail_map:
		FREE(self);
	return NULL;
}

void cc_multimap_delete(cc_multimap_t** _self)
{
	ASSERT(_self);

	cc_multimap_t* self = *_self;
	if(self)
	{
		cc_map_delete(&self->map);
		FREE(self);
		*_self = NULL;
	}
}

void cc_multimap_discard(cc_multimap_t* self)
{
	ASSERT(self);

	cc_mapIter_t* miter = cc_map_head(self->map);
	while(miter)
	{
		cc_list_t* list;
		list = (cc_list_t*)
		       cc_map_remove(self->map, &miter);
		cc_list_discard(list);
		cc_list_delete(&list);
	}
}

int cc_multimap_size(const cc_multimap_t* self)
{
	ASSERT(self);

	return cc_map_size(self->map);
}

size_t cc_multimap_sizeof(const cc_multimap_t* self)
{
	ASSERT(self);

	return sizeof(cc_multimap_t) + cc_map_sizeof(self->map);
}

cc_multimapIter_t*
cc_multimap_head(const cc_multimap_t* self,
                 cc_multimapIter_t* mmiter)
{
	ASSERT(self);
	ASSERT(mmiter);

	mmiter->miter = cc_map_head(self->map);
	if(mmiter->miter == NULL)
	{
		return NULL;
	}

	cc_list_t* list = (cc_list_t*) cc_map_val(mmiter->miter);
	mmiter->iter = cc_list_head(list);

	return mmiter;
}

cc_multimapIter_t* cc_multimap_next(cc_multimapIter_t* mmiter)
{
	ASSERT(mmiter);

	mmiter->iter = cc_list_next(mmiter->iter);
	if(mmiter->iter)
	{
		return mmiter;
	}

	mmiter->miter = cc_map_next(mmiter->miter);
	if(mmiter->miter == NULL)
	{
		return NULL;
	}

	cc_list_t* list = (cc_list_t*) cc_map_val(mmiter->miter);
	mmiter->iter = cc_list_head(list);

	return mmiter;
}

cc_multimapIter_t* cc_multimap_nextItem(cc_multimapIter_t* mmiter)
{
	ASSERT(mmiter);

	mmiter->iter = cc_list_next(mmiter->iter);
	if(mmiter->iter)
	{
		return mmiter;
	}

	return NULL;
}

cc_multimapIter_t* cc_multimap_nextList(cc_multimapIter_t* mmiter)
{
	ASSERT(mmiter);

	mmiter->miter = cc_map_next(mmiter->miter);
	if(mmiter->miter == NULL)
	{
		return NULL;
	}

	cc_list_t* list = (cc_list_t*) cc_map_val(mmiter->miter);
	mmiter->iter = cc_list_head(list);

	return mmiter;
}

const void* cc_multimap_key(const cc_multimapIter_t* mmiter,
                            int* _len)
{
	ASSERT(mmiter);
	ASSERT(_len);

	return cc_map_key(mmiter->miter, _len);
}

const void* cc_multimap_val(const cc_multimapIter_t* mmiter)
{
	ASSERT(mmiter);

	return cc_list_peekIter(mmiter->iter);
}

const cc_list_t*
cc_multimap_list(const cc_multimapIter_t* mmiter)
{
	ASSERT(mmiter);

	return (const cc_list_t*) cc_map_val(mmiter->miter);
}

const cc_list_t*
cc_multimap_findp(const cc_multimap_t* self,
                  cc_multimapIter_t* mmiter,
                  int len,
                  const void* key)
{
	ASSERT(self);
	ASSERT(mmiter);
	ASSERT(key);

	mmiter->miter = cc_map_findp(self->map, len, key);
	if(mmiter->miter == NULL)
	{
		return NULL;
	}

	cc_list_t* list;
	list = (cc_list_t*) cc_map_val(mmiter->miter);

	mmiter->iter = cc_list_head(list);

	return list;
}

const cc_list_t*
cc_multimap_find(const cc_multimap_t* self,
                 cc_multimapIter_t* mmiter,
                 const char* key)
{
	ASSERT(self);
	ASSERT(mmiter);
	ASSERT(key);

	mmiter->miter = cc_map_find(self->map, key);
	if(mmiter->miter == NULL)
	{
		return NULL;
	}

	cc_list_t* list;
	list = (cc_list_t*) cc_map_val(mmiter->miter);

	mmiter->iter = cc_list_head(list);

	return list;
}

const cc_list_t*
cc_multimap_findf(const cc_multimap_t* self,
                  cc_multimapIter_t* mmiter,
                  const char* fmt, ...)
{
	ASSERT(self);
	ASSERT(mmiter);
	ASSERT(fmt);

	char key[256];
	va_list argptr;
	va_start(argptr, fmt);
	vsnprintf(key, 256, fmt, argptr);
	va_end(argptr);

	return cc_multimap_find(self, mmiter, key);
}

int cc_multimap_addp(cc_multimap_t* self,
                     const void* val,
                     int len,
                     const void* key)
{
	ASSERT(self);
	ASSERT(val);
	ASSERT(key);

	cc_listIter_t* iter;
	cc_mapIter_t*  miter;
	cc_list_t*     list = NULL;

	miter = cc_map_findp(self->map, len, key);
	if(miter)
	{
		list = (cc_list_t*) cc_map_val(miter);
	}

	// check if the list already exists
	if(list && self->compare)
	{
		iter = cc_list_insertSorted(list, self->compare,
		                            val);
		if(iter == NULL)
		{
			return 0;
		}

		return 1;
	}
	else if(list)
	{
		iter = cc_list_append(list, NULL, val);
		if(iter == NULL)
		{
			return 0;
		}

		return 1;
	}

	// create a new list and add to map
	list = cc_list_new();
	if(list == NULL)
	{
		return 0;
	}

	iter = cc_list_append(list, NULL, val);
	if(iter == NULL)
	{
		goto fail_append;
	}

	if(cc_map_addp(self->map, (const void*) list, len,
	               key) == NULL)
	{
		goto fail_add;
	}

	// success
	return 1;

	// failure
	fail_add:
		cc_list_remove(list, &iter);
	fail_append:
		cc_list_delete(&list);
	return 0;
}

int cc_multimap_add(cc_multimap_t* self,
                    const void* val,
                    const char* key)
{
	ASSERT(self);
	ASSERT(val);
	ASSERT(key);

	cc_listIter_t* iter;
	cc_mapIter_t*  miter;
	cc_list_t*     list = NULL;

	miter = cc_map_find(self->map, key);
	if(miter)
	{
		list = (cc_list_t*) cc_map_val(miter);
	}

	// check if the list already exists
	if(list && self->compare)
	{
		iter = cc_list_insertSorted(list, self->compare,
		                            val);
		if(iter == NULL)
		{
			return 0;
		}

		return 1;
	}
	else if(list)
	{
		iter = cc_list_append(list, NULL, val);
		if(iter == NULL)
		{
			return 0;
		}

		return 1;
	}

	// create a new list and add to map
	list = cc_list_new();
	if(list == NULL)
	{
		return 0;
	}

	iter = cc_list_append(list, NULL, val);
	if(iter == NULL)
	{
		goto fail_append;
	}

	if(cc_map_add(self->map, (const void*) list, key) == NULL)
	{
		goto fail_add;
	}

	// success
	return 1;

	// failure
	fail_add:
		cc_list_remove(list, &iter);
	fail_append:
		cc_list_delete(&list);
	return 0;
}

int cc_multimap_addf(cc_multimap_t* self, const void* val,
                     const char* fmt, ...)
{
	ASSERT(self);
	ASSERT(val);
	ASSERT(fmt);

	char key[256];
	va_list argptr;
	va_start(argptr, fmt);
	vsnprintf(key, 256, fmt, argptr);
	va_end(argptr);

	return cc_multimap_add(self, val, key);
}

const void*
cc_multimap_remove(cc_multimap_t* self,
                   cc_multimapIter_t** _iter)
{
	ASSERT(self);
	ASSERT(_iter);
	ASSERT(*_iter);

	cc_multimapIter_t* mmiter = *_iter;

	// remove iter from list;
	cc_list_t* list;
	list = (cc_list_t*)
	       cc_map_val(mmiter->miter);
	const void* data = cc_list_remove(list,
	                                   &mmiter->iter);

	// check if list is empty
	// or if next iter is NULL
	if(cc_list_size(list) == 0)
	{
		cc_map_remove(self->map, &mmiter->miter);
		cc_list_delete(&list);
		if(mmiter->miter)
		{
			list = (cc_list_t*)
			       cc_map_val(mmiter->miter);
			mmiter->iter = cc_list_head(list);
		}
	}
	else if(mmiter->iter == NULL)
	{
		mmiter->miter = cc_map_next(mmiter->miter);
		if(mmiter->miter)
		{
			list = (cc_list_t*)
			       cc_map_val(mmiter->miter);
			mmiter->iter = cc_list_head(list);
		}
	}

	// check for iteration end
	if(mmiter->miter == NULL)
	{
		*_iter = NULL;
	}

	return data;
}
