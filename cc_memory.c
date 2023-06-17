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

#include <pthread.h>
#include <stdlib.h>
#include <inttypes.h>

#define LOG_TAG "cc"
#include "cc_log.h"
#include "cc_memory.h"

// The sizeof(cc_memory_t) needs to match the alignment
// requirements of the platform. However there doesn't seem
// to be a good platform independent technique to determine
// this information.
typedef struct
{
	size_t size;
} cc_memory_t;

pthread_mutex_t memory_mutex = PTHREAD_MUTEX_INITIALIZER;
size_t          memory_count = 0;
size_t          memory_size  = 0;

#ifdef MEMORY_DEBUG

#include <stdio.h>
#include <string.h>
#include "cc_map.h"

/***********************************************************
* protected                                                *
***********************************************************/

extern cc_map_t* cc_map_newCMalloc(void);

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
} cc_meminfo_t;

cc_meminfo_t* memory_meminfo = NULL;

/***********************************************************
* private - cc_ator                                        *
***********************************************************/

static cc_ator_t*
cc_ator_new(const char* name)
{
	ASSERT(name);

	cc_ator_t* self;
	self = (cc_ator_t*) malloc(sizeof(cc_ator_t));
	if(self == NULL)
	{
		LOGE("malloc failed");
		return NULL;
	}

	snprintf(self->name, CC_MEMORY_NAMELEN, "%s", name);

	self->map_pinfo_ref = cc_map_newCMalloc();
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
	ASSERT(_self);

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
	ASSERT(self);
	ASSERT(pinfo_ref);

	if(cc_map_addp(self->map_pinfo_ref,
	               (const void*) pinfo_ref,
	               0, pinfo_ref->ptr) == NULL)
	{
		return 0;
	}
	self->size += pinfo_ref->size;
	return 1;
}

static void
cc_ator_rem(cc_ator_t* self, cc_pinfo_t* pinfo_ref)
{
	ASSERT(self);
	ASSERT(pinfo_ref);

	cc_mapIter_t* miter;
	miter = cc_map_findp(self->map_pinfo_ref, 0,
	                     pinfo_ref->ptr);
	if(miter)
	{
		self->size -= pinfo_ref->size;
		cc_map_remove(self->map_pinfo_ref, &miter);
	}
}

/***********************************************************
* private - cc_pinfo                                       *
***********************************************************/

static cc_pinfo_t*
cc_pinfo_new(cc_ator_t* ator_ref, void* ptr, size_t size)
{
	ASSERT(ator_ref);
	ASSERT(ptr);

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
	ASSERT(_self);

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

	memory_meminfo->map_ator = cc_map_newCMalloc();
	if(memory_meminfo->map_ator == NULL)
	{
		goto fail_map_ator;
	}

	memory_meminfo->map_pinfo = cc_map_newCMalloc();
	if(memory_meminfo->map_pinfo == NULL)
	{
		goto fail_map_pinfo;
	}

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
	ASSERT(self);
	ASSERT(func);
	ASSERT(ptr);

	char name[CC_MEMORY_NAMELEN];
	snprintf(name, CC_MEMORY_NAMELEN, "%s@%i", func, line);

	// get ator
	cc_ator_t*    ator;
	cc_mapIter_t* miter;
	miter = cc_map_find(self->map_ator, name);
	if(miter == NULL)
	{
		ator = cc_ator_new(name);
		if(ator == NULL)
		{
			return;
		}

		// add ator
		if(cc_map_add(self->map_ator, (const void*) ator,
		              name) == NULL)
		{
			cc_ator_delete(&ator);
			return;
		}
	}
	else
	{
		ator = (cc_ator_t*) cc_map_val(miter);
	}

	// create pinfo
	cc_pinfo_t* pinfo = cc_pinfo_new(ator, ptr, size);
	if(pinfo == NULL)
	{
		return;
	}

	// add pinfo to meminfo
	miter = cc_map_addp(self->map_pinfo, (const void*) pinfo,
	                    0, pinfo->ptr);
	if(miter == NULL)
	{
		goto fail_map_add;
	}

	// add pinfo to ator
	if(cc_ator_add(ator, pinfo) == 0)
	{
		goto fail_ator_add;
	}

	// success
	return;

	// failure
	fail_ator_add:
		cc_map_remove(self->map_pinfo, &miter);
	fail_map_add:
		cc_pinfo_delete(&pinfo);
}

static void
cc_meminfo_rem(cc_meminfo_t* self, const char* func,
               int line, void* ptr)
{
	ASSERT(self);
	ASSERT(func);
	ASSERT(ptr);

	cc_pinfo_t*   pinfo;
	cc_mapIter_t* miter;
	miter = cc_map_findp(self->map_pinfo, 0, ptr);
	if(miter == NULL)
	{
		LOGW("invalid %s@%i ptr=%p", func, line, ptr);
		return;
	}
	pinfo = (cc_pinfo_t*) cc_map_val(miter);

	cc_ator_rem(pinfo->ator_ref, pinfo);
	cc_map_remove(self->map_pinfo, &miter);
	cc_pinfo_delete(&pinfo);
}

static int
cc_meminfo_memcheckptr(cc_meminfo_t* self, const char* func,
                       int line, void* ptr)
{
	ASSERT(self);
	ASSERT(func);
	ASSERT(ptr);

	cc_mapIter_t* miter;
	miter = cc_map_findp(self->map_pinfo, 0, ptr);
	if(miter == NULL)
	{
		LOGE("invalid %s@%i ptr=%p", func, line, ptr);
		return 0;
	}

	return 1;
}

static void cc_meminfo_meminfo(cc_meminfo_t* self)
{
	ASSERT(self);

	LOGI("cnt_ator=%i, cnt_pinfo=%i",
	     cc_map_size(self->map_ator),
	     cc_map_size(self->map_pinfo));

	cc_mapIter_t* miter = cc_map_head(self->map_ator);
	while(miter)
	{
		cc_ator_t* ator = (cc_ator_t*) cc_map_val(miter);

		LOGI("name=%s, cnt_pinfo=%i, size=%" PRIu64,
		     ator->name, cc_map_size(ator->map_pinfo_ref),
		     (uint64_t) ator->size);

		miter = cc_map_next(miter);
	}
}

/***********************************************************
* private - cc_memory                                      *
***********************************************************/

static void
cc_memory_add(const char* func, int line, void* ptr,
              size_t size)
{
	ASSERT(func);

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
	ASSERT(func);

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

static int
cc_memory_memcheckptr(const char* func, int line, void* ptr)
{
	ASSERT(func);

	// ignore NULL
	if(ptr == NULL)
	{
		return 1;
	}

	pthread_mutex_lock(&memory_mutex);

	if(cc_meminfo_init() == NULL)
	{
		pthread_mutex_unlock(&memory_mutex);
		return 0;
	}

	int ret = cc_meminfo_memcheckptr(memory_meminfo,
	                                 func, line, ptr);

	pthread_mutex_unlock(&memory_mutex);

	return ret;
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
* public - debug                                           *
***********************************************************/

void* cc_malloc_debug(const char* func, int line,
                      size_t size)
{
	void* ptr = cc_malloc(size);
	cc_memory_add(func, line, ptr, size);
	return ptr;
}

void* cc_calloc_debug(const char* func, int line,
                      size_t count, size_t size)
{
	void* ptr = cc_calloc(count, size);
	cc_memory_add(func, line, ptr, count*size);
	return ptr;
}

void* cc_realloc_debug(const char* func, int line,
                       void* ptr, size_t size)
{
	cc_memory_rem(func, line, ptr);
	void* reptr = cc_realloc(ptr, size);
	if(reptr)
	{
		cc_memory_add(func, line, reptr, size);
	}
	else
	{
		cc_memory_add(func, line, ptr, size);
	}
	return reptr;
}

void cc_free_debug(const char* func, int line, void* ptr)
{
	cc_memory_rem(func, line, ptr);
	cc_free(ptr);
}

void cc_meminfo_debug(void)
{
	cc_meminfo();
	cc_memory_meminfo();
}

int cc_memcheckptr_debug(const char* func, int line,
                         void* ptr)
{
	return cc_memory_memcheckptr(func, line, ptr);
}

#endif // MEMORY_DEBUG

/***********************************************************
* public                                                   *
***********************************************************/

void* cc_malloc(size_t size)
{
	cc_memory_t* mem;
	mem = (cc_memory_t*)
	      malloc(size + sizeof(cc_memory_t));
	if(mem == NULL)
	{
		return NULL;
	}
	mem->size = size;

	pthread_mutex_lock(&memory_mutex);
	++memory_count;
	memory_size += mem->size;
	LOGD("mem=%p, size=%i, memory_size=%i",
	     mem, (int) mem->size, memory_size);
	pthread_mutex_unlock(&memory_mutex);

	return (void*) mem + sizeof(cc_memory_t);
}

void* cc_calloc(size_t count, size_t size)
{
	cc_memory_t* mem;
	mem = (cc_memory_t*)
	      calloc(1, count*size + sizeof(cc_memory_t));
	if(mem == NULL)
	{
		return NULL;
	}
	mem->size = count*size;

	pthread_mutex_lock(&memory_mutex);
	++memory_count;
	memory_size += mem->size;
	LOGD("mem=%p, size=%i, memory_size=%i",
	     mem, (int) mem->size, memory_size);
	pthread_mutex_unlock(&memory_mutex);

	return (void*) mem + sizeof(cc_memory_t);
}

void* cc_realloc(void* ptr, size_t size)
{
	if(ptr == NULL)
	{
		return cc_malloc(size);
	}

	cc_memory_t* mem1  = (cc_memory_t*)
	                     (ptr - sizeof(cc_memory_t));
	size_t       size1 = mem1->size;

	cc_memory_t* mem2;
	mem2 = (cc_memory_t*)
	       realloc((void*) mem1, size + sizeof(cc_memory_t));
	if(mem2 == NULL)
	{
		return NULL;
	}
	mem2->size = size;

	pthread_mutex_lock(&memory_mutex);
	memory_size += mem2->size - size1;
	LOGD("mem=%p, size=%i, memory_size=%i",
	     mem2, (int) mem2->size, memory_size);
	pthread_mutex_unlock(&memory_mutex);

	return (void*) mem2 + sizeof(cc_memory_t);
}

void cc_free(void* ptr)
{
	if(ptr)
	{
		cc_memory_t* mem = ptr - sizeof(cc_memory_t);

		pthread_mutex_lock(&memory_mutex);
		--memory_count;
		memory_size -= mem->size;
		LOGD("mem=%p, size=%i, memory_size=%i",
		     mem, (int) mem->size, memory_size);
		pthread_mutex_unlock(&memory_mutex);

		free(mem);
	}
}

size_t cc_memcount(void)
{
	size_t count;
	pthread_mutex_lock(&memory_mutex);
	count = memory_count;
	pthread_mutex_unlock(&memory_mutex);
	return count;
}

void cc_meminfo(void)
{
	pthread_mutex_lock(&memory_mutex);
	LOGI("count=%i, size=%" PRIu64,
	     (int) memory_count, (uint64_t) memory_size);
	pthread_mutex_unlock(&memory_mutex);
}

size_t cc_memsize(void)
{
	size_t size;
	pthread_mutex_lock(&memory_mutex);
	size = memory_size;
	pthread_mutex_unlock(&memory_mutex);
	return size;
}

size_t cc_memsizeptr(void* ptr)
{
	size_t size = 0;
	if(ptr)
	{
		cc_memory_t* mem = ptr - sizeof(cc_memory_t);
		size = mem->size;
	}

	return size;
}
