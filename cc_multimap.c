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

#include <assert.h>
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
	assert(_self);

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
	assert(self);

	cc_map_discard(self->map);
}

int cc_multimap_size(const cc_multimap_t* self)
{
	assert(self);

	return cc_map_size(self->map);
}

size_t cc_multimap_sizeof(const cc_multimap_t* self)
{
	assert(self);

	return sizeof(cc_multimap_t) + cc_map_sizeof(self->map);
}

cc_multimapIter_t*
cc_multimap_head(const cc_multimap_t* self,
                 cc_multimapIter_t* iter)
{
	assert(self);
	assert(iter);

	iter->hiter = cc_map_head(self->map, &iter->hiterator);
	if(iter->hiter == NULL)
	{
		return NULL;
	}

	cc_list_t* list = (cc_list_t*) cc_map_val(iter->hiter);
	iter->item = cc_list_head(list);

	return iter;
}

cc_multimapIter_t* cc_multimap_next(cc_multimapIter_t* iter)
{
	assert(iter);

	iter->item = cc_list_next(iter->item);
	if(iter->item)
	{
		return iter;
	}

	iter->hiter = cc_map_next(iter->hiter);
	if(iter->hiter == NULL)
	{
		return NULL;
	}

	cc_list_t* list = (cc_list_t*) cc_map_val(iter->hiter);
	iter->item = cc_list_head(list);

	return iter;
}

cc_multimapIter_t* cc_multimap_nextItem(cc_multimapIter_t* iter)
{
	assert(iter);

	iter->item = cc_list_next(iter->item);
	if(iter->item)
	{
		return iter;
	}

	return NULL;
}

cc_multimapIter_t* cc_multimap_nextList(cc_multimapIter_t* iter)
{
	assert(iter);

	iter->hiter = cc_map_next(iter->hiter);
	if(iter->hiter == NULL)
	{
		return NULL;
	}

	cc_list_t* list = (cc_list_t*) cc_map_val(iter->hiter);
	iter->item = cc_list_head(list);

	return iter;
}

const void* cc_multimap_val(const cc_multimapIter_t* iter)
{
	assert(iter);

	return cc_list_peekIter(iter->item);
}

const cc_list_t*
cc_multimap_list(const cc_multimapIter_t* iter)
{
	assert(iter);

	return (const cc_list_t*) cc_map_val(iter->hiter);
}

const char* cc_multimap_key(const cc_multimapIter_t* iter)
{
	assert(iter);

	return cc_map_key(iter->hiter);
}

const cc_list_t*
cc_multimap_find(const cc_multimap_t* self,
                 cc_multimapIter_t* iter,
                 const char* key)
{
	assert(self);
	assert(iter);
	assert(key);

	iter->hiter = &iter->hiterator;

	cc_list_t* list;
	list = (cc_list_t*)
	       cc_map_find(self->map, iter->hiter, key);
	if(list == NULL)
	{
		return NULL;
	}

	iter->item = cc_list_head(list);

	return list;
}

const cc_list_t*
cc_multimap_findf(const cc_multimap_t* self,
                  cc_multimapIter_t* iter,
                  const char* fmt, ...)
{
	assert(self);
	assert(iter);
	assert(fmt);

	char key[256];
	va_list argptr;
	va_start(argptr, fmt);
	vsnprintf(key, 256, fmt, argptr);
	va_end(argptr);

	return cc_multimap_find(self, iter, key);
}

int cc_multimap_add(cc_multimap_t* self,
                    const void* val,
                    const char* key)
{
	assert(self);
	assert(val);
	assert(key);

	cc_listIter_t* item;

	// check if the list already exists
	cc_mapIter_t iter;
	cc_list_t* list;
	list = (cc_list_t*)
	       cc_map_find(self->map, &iter, key);
	if(list && self->compare)
	{
		item = cc_list_insertSorted(list, self->compare,
		                            val);
		if(item == NULL)
		{
			return 0;
		}

		return 1;
	}
	else if(list)
	{
		item = cc_list_append(list, NULL, val);
		if(item == NULL)
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

	item = cc_list_append(list, NULL, val);
	if(item == NULL)
	{
		goto fail_append;
	}

	if(cc_map_add(self->map, (const void*) list, key) == 0)
	{
		goto fail_add;
	}

	// success
	return 1;

	// failure
	fail_add:
		cc_list_remove(list, &item);
	fail_append:
		cc_list_delete(&list);
	return 0;
}

int cc_multimap_addf(cc_multimap_t* self, const void* val,
                     const char* fmt, ...)
{
	assert(self);
	assert(val);
	assert(fmt);

	char key[256];
	va_list argptr;
	va_start(argptr, fmt);
	vsnprintf(key, 256, fmt, argptr);
	va_end(argptr);

	return cc_multimap_add(self, val, key);
}

const void*
cc_multimap_replace(cc_multimapIter_t* iter,
                    const void*  val)
{
	assert(iter);
	assert(val);

	cc_list_t* list;
	list = (cc_list_t*)
	       cc_map_val(iter->hiter);
	return cc_list_replace(list, iter->item, val);
}

const void*
cc_multimap_remove(cc_multimap_t* self,
                   cc_multimapIter_t** _iter)
{
	assert(self);
	assert(_iter);
	assert(*_iter);

	cc_multimapIter_t* iter = *_iter;

	// remove item from list;
	cc_list_t* list;
	list = (cc_list_t*)
	       cc_map_val(iter->hiter);
	const void* data = cc_list_remove(list,
	                                   &iter->item);

	// check if list is empty
	// or if next item is NULL
	if(cc_list_size(list) == 0)
	{
		cc_map_remove(self->map, &iter->hiter);
		cc_list_delete(&list);
		if(iter->hiter)
		{
			list = (cc_list_t*)
			       cc_map_val(iter->hiter);
			iter->item = cc_list_head(list);
		}
	}
	else if(iter->item == NULL)
	{
		iter->hiter = cc_map_next(iter->hiter);
		if(iter->hiter)
		{
			list = (cc_list_t*)
			       cc_map_val(iter->hiter);
			iter->item = cc_list_head(list);
		}
	}

	// check for iteration end
	if(iter->hiter == NULL)
	{
		*_iter = NULL;
	}

	return data;
}
