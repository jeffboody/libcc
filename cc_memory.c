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
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LOG_TAG "cc"
#include "cc_log.h"
#include "cc_map.h"
#include "cc_memory.h"

/***********************************************************
* private                                                  *
***********************************************************/

#define CC_MEMORY_NAMELEN 64

typedef struct
{
	char      name[CC_MEMORY_NAMELEN];
	cc_map_t* map_pinfo_ref;
	size_t    size;
} cc_ator_t;

typedef struct
{
	cc_ator_t* ator_ref;
	void*      ptr;
	size_t     size;
} cc_pinfo_t;

typedef struct
{
	cc_map_t* map_ator;
	cc_map_t* map_pinfo;
	size_t    size;
} cc_meminfo_t;

pthread_mutex_t memory_mutex   = PTHREAD_MUTEX_INITIALIZER; 
cc_meminfo_t*   memory_meminfo = NULL;

/***********************************************************
* private - cc_ator                                        *
***********************************************************/

static cc_ator_t*
cc_ator_new(const char* name)
{
	assert(name);

	cc_ator_t* self;
	self = (cc_ator_t*) malloc(sizeof(cc_ator_t));
	if(self == NULL)
	{
		LOGE("malloc failed");
		return NULL;
	}

	snprintf(self->name, CC_MEMORY_NAMELEN, "%s", name);

	self->map_pinfo_ref = cc_map_new();
	if(self->map_pinfo_ref == NULL)
	{
		goto fail_map_pinfo_ref;
	}

	self->size = 0;

	// success
	return self;

	// failure
	fail_map_pinfo_ref:
		free(self);
	return NULL;
}

static void cc_ator_delete(cc_ator_t** _self)
{
	assert(_self);

	cc_ator_t* self = *_self;
	if(self)
	{
		cc_map_discard(self->map_pinfo_ref);
		cc_map_delete(&self->map_pinfo_ref);
		free(self);
		*_self = NULL;
	}
}

static int
cc_ator_add(cc_ator_t* self, cc_pinfo_t* pinfo_ref)
{
	assert(self);
	assert(pinfo_ref);

	if(cc_map_addf(self->map_pinfo_ref,
	               (const void*) pinfo_ref,
	               "%p", pinfo_ref->ptr) == 0)
	{
		return 0;
	}
	self->size += pinfo_ref->size;
	return 1;
}

static void
cc_ator_rem(cc_ator_t* self, cc_pinfo_t* pinfo_ref)
{
	assert(self);
	assert(pinfo_ref);

	cc_mapIter_t  hiterator;
	cc_mapIter_t* hiter = &hiterator;
	if(cc_map_findf(self->map_pinfo_ref, hiter, "%p",
	                pinfo_ref->ptr))
	{
		self->size -= pinfo_ref->size;
		cc_map_remove(self->map_pinfo_ref, &hiter);
	}
}

/***********************************************************
* private - cc_pinfo                                       *
***********************************************************/

static cc_pinfo_t*
cc_pinfo_new(cc_ator_t* ator_ref, void* ptr, size_t size)
{
	assert(ator_ref);
	assert(ptr);

	cc_pinfo_t* self;
	self = (cc_pinfo_t*) malloc(sizeof(cc_pinfo_t));
	if(self == NULL)
	{
		LOGE("malloc failed");
		return NULL;
	}

	self->ator_ref = ator_ref;
	self->ptr      = ptr;
	self->size     = size;
	return self;
}

static void cc_pinfo_delete(cc_pinfo_t** _self)
{
	assert(_self);

	cc_pinfo_t* self = *_self;
	if(self)
	{
		free(self);
		*_self = NULL;
	}
}

/***********************************************************
* private - cc_meminfo                                     *
***********************************************************/

static cc_meminfo_t* cc_meminfo_init(void)
{
	if(memory_meminfo)
	{
		return memory_meminfo;
	}

	memory_meminfo = (cc_meminfo_t*)
	                 malloc(sizeof(cc_meminfo_t));
	if(memory_meminfo == NULL)
	{
		LOGE("malloc failed");
		return NULL;
	}

	memory_meminfo->map_ator = cc_map_new();
	if(memory_meminfo->map_ator == NULL)
	{
		goto fail_map_ator;
	}

	memory_meminfo->map_pinfo = cc_map_new();
	if(memory_meminfo->map_pinfo == NULL)
	{
		goto fail_map_pinfo;
	}

	memory_meminfo->size = 0;

	// success
	return memory_meminfo;

	// failure
	fail_map_pinfo:
		cc_map_delete(&memory_meminfo->map_ator);
	fail_map_ator:
		free(memory_meminfo);
		memory_meminfo = NULL;
	return NULL;
}

static void
cc_meminfo_add(cc_meminfo_t* self,
               const char* func, int line,
               void* ptr, size_t size)
{
	assert(self);
	assert(func);
	assert(ptr);

	char name[CC_MEMORY_NAMELEN];
	snprintf(name, CC_MEMORY_NAMELEN, "%s@%i", func, line);

	// get ator
	cc_mapIter_t hiterator;
	cc_ator_t* ator;
	ator = (cc_ator_t*)
	       cc_map_find(self->map_ator, &hiterator, name);
	if(ator == NULL)
	{
		ator = cc_ator_new(name);
		if(ator == NULL)
		{
			return;
		}

		// add ator
		if(cc_map_add(self->map_ator, (const void*) ator,
		              name) == 0)
		{
			cc_ator_delete(&ator);
			return;
		}
	}

	// create pinfo
	cc_pinfo_t* pinfo = cc_pinfo_new(ator, ptr, size);
	if(pinfo == NULL)
	{
		return;
	}

	// add pinfo to meminfo
	if(cc_map_addf(self->map_pinfo, (const void*) pinfo,
	               "%p", pinfo->ptr) == 0)
	{
		cc_pinfo_delete(&pinfo);
		return;
	}

	// add pinfo to ator
	if(cc_ator_add(ator, pinfo) == 0)
	{
		cc_mapIter_t* hiter = &hiterator;
		if(cc_map_findf(self->map_pinfo, hiter, "%p",
		                pinfo->ptr))
		{
			cc_map_remove(self->map_pinfo, &hiter);
			cc_pinfo_delete(&pinfo);
		}
		return;
	}

	self->size += size;
}

static void
cc_meminfo_rem(cc_meminfo_t* self, const char* func,
               int line, void* ptr)
{
	assert(self);
	assert(func);
	assert(ptr);

	cc_mapIter_t  hiterator;
	cc_mapIter_t* hiter;
	hiter = &hiterator;
	cc_pinfo_t* pinfo;
	pinfo = (cc_pinfo_t*)
	        cc_map_findf(self->map_pinfo, hiter, "%p", ptr);
	if(pinfo == NULL)
	{
		LOGW("invalid %s@%i ptr=%p", func, line, ptr);
		return;
	}

	cc_ator_rem(pinfo->ator_ref, pinfo);
	cc_map_remove(self->map_pinfo, &hiter);
	self->size -= pinfo->size;
	cc_pinfo_delete(&pinfo);
}

static void cc_meminfo_meminfo(cc_meminfo_t* self)
{
	assert(self);

	LOGI("cnt_ator=%i, cnt_pinfo=%i, size=%i",
	     cc_map_size(self->map_ator),
	     cc_map_size(self->map_pinfo),
	     (int) self->size);

	cc_mapIter_t  hiterator;
	cc_mapIter_t* hiter;
	hiter = cc_map_head(self->map_ator, &hiterator);
	while(hiter)
	{
		cc_ator_t* ator = (cc_ator_t*) cc_map_val(hiter);

		LOGI("name=%s, cnt_pinfo=%i, size=%i",
		     ator->name, cc_map_size(ator->map_pinfo_ref),
		     (int) ator->size);

		hiter = cc_map_next(hiter);
	}
}

/***********************************************************
* private - cc_memory                                      *
***********************************************************/

static void
cc_memory_add(const char* func, int line, void* ptr,
              size_t size)
{
	assert(func);

	// ignore NULL
	if(ptr == NULL)
	{
		return;
	}

	pthread_mutex_lock(&memory_mutex);

	if(cc_meminfo_init() == NULL)
	{
		pthread_mutex_unlock(&memory_mutex);
		return;
	}

	cc_meminfo_add(memory_meminfo, func, line, ptr, size);

	pthread_mutex_unlock(&memory_mutex);
}

static void
cc_memory_rem(const char* func, int line, void* ptr)
{
	assert(func);

	// ignore NULL
	if(ptr == NULL)
	{
		return;
	}

	pthread_mutex_lock(&memory_mutex);

	if(cc_meminfo_init() == NULL)
	{
		pthread_mutex_unlock(&memory_mutex);
		return;
	}

	cc_meminfo_rem(memory_meminfo, func, line, ptr);

	pthread_mutex_unlock(&memory_mutex);
}

static void cc_memory_meminfo(void)
{
	pthread_mutex_lock(&memory_mutex);

	if(cc_meminfo_init() == NULL)
	{
		pthread_mutex_unlock(&memory_mutex);
		return;
	}

	cc_meminfo_meminfo(memory_meminfo);

	pthread_mutex_unlock(&memory_mutex);
}

/***********************************************************
* public                                                   *
***********************************************************/

void* cc_malloc(const char* func, int line, size_t size)
{
	void* ptr = malloc(size);
	cc_memory_add(func, line, ptr, size);
	return ptr;
}

void* cc_calloc(const char* func, int line, size_t count,
                size_t size)
{
	void* ptr = calloc(count, size);
	cc_memory_add(func, line, ptr, count*size);
	return ptr;
}

void* cc_realloc(const char* func, int line, void* ptr,
                 size_t size)
{
	void* reptr = realloc(ptr, size);
	cc_memory_rem(func, line, ptr);
	cc_memory_add(func, line, reptr, size);
	return reptr;
}

void cc_free(const char* func, int line,
               void* ptr)
{
	cc_memory_rem(func, line, ptr);
	free(ptr);
}

void cc_meminfo(void)
{
	cc_memory_meminfo();
}
