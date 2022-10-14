/*
 * Copyright (c) 2011 Jeff Boody
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
#include <string.h>

#define LOG_TAG "cc"
#include "cc_list.h"
#include "cc_log.h"
#include "cc_memory.h"

#define CC_LIST_FLAG_CMALLOC 1

/***********************************************************
* protected - global listIter pool                         *
***********************************************************/

#define CC_LISTBLOCK_SIZE 1024
#define CC_LISTSET_SIZE   32

typedef struct cc_listBlock_s cc_listBlock_t;

typedef struct cc_listBlock_s
{
	cc_listIter_t   array[CC_LISTBLOCK_SIZE];
	cc_listBlock_t* next;
} cc_listBlock_t;

typedef struct
{
	size_t          refcount;
	size_t          count;
	cc_listIter_t*  iters; // sll of free iter references
	cc_listBlock_t* blocks;
	pthread_mutex_t mutex;
} cc_listPool_t;

// Android does not allow the use of
// PTHREAD_MUTEX_INITIALIZER due to the Android app life
// cycle as the mutex is not reinitialized after
// onDestroy().
// See vkk_platformAndroid.c where this case is
// automatically handled.
#ifdef ANDROID
static cc_listPool_t g_list_pool;

int cc_listPool_init(void)
{
	memset(&g_list_pool, 0, sizeof(cc_listPool_t));
	if(pthread_mutex_init(&g_list_pool.mutex, NULL) != 0)
	{
		LOGE("pthread_mutex_init failed");
		return 0;
	}

	return 1;
}

void cc_listPool_destroy(void)
{
	pthread_mutex_destroy(&g_list_pool.mutex);
}
#else
static cc_listPool_t g_list_pool =
{
	.mutex=PTHREAD_MUTEX_INITIALIZER,
};
#endif

/***********************************************************
* private - global listIter pool                           *
***********************************************************/

static cc_listIter_t* cc_listPool_get(void)
{
	cc_listPool_t* pool = &g_list_pool;

	pthread_mutex_lock(&pool->mutex);

	// check if free iters exist
	if(pool->count < CC_LISTSET_SIZE)
	{
		// create a new block
		cc_listBlock_t* block;
		block = (cc_listBlock_t*)
		        CALLOC(1, sizeof(cc_listBlock_t));
		if(block == NULL)
		{
			// silently fail
			pthread_mutex_unlock(&pool->mutex);
			return NULL;
		}

		// insert block to list
		block->next  = pool->blocks;
		pool->blocks = block;

		// insert iter to list of free iters
		int i;
		for(i = 0; i < CC_LISTBLOCK_SIZE; ++i)
		{
			cc_listIter_t* iter;
			iter        = &block->array[i];
			iter->next  = pool->iters;
			pool->iters = iter;
		}

		pool->count += CC_LISTBLOCK_SIZE;
	}

	// get a set of free iters
	int i;
	cc_listIter_t* iters = pool->iters;
	cc_listIter_t* tail;
	for(i = 0; i < CC_LISTSET_SIZE; ++i)
	{
		tail        = pool->iters;
		pool->iters = pool->iters->next;
	}
	tail->next = NULL;

	pool->count    -= CC_LISTSET_SIZE;
	pool->refcount += CC_LISTSET_SIZE;

	pthread_mutex_unlock(&pool->mutex);

	return iters;
}

static void cc_listPool_put(cc_listIter_t* iters)
{
	// iters may be NULL

	cc_listPool_t* pool = &g_list_pool;

	if(iters == NULL)
	{
		// ignore
		return;
	}

	// count iters and find tail iter
	size_t         count = 1;
	cc_listIter_t* tail  = iters;
	while(tail->next)
	{
		++count;
		tail = tail->next;
	}

	pthread_mutex_lock(&pool->mutex);

	// insert iters into free iters
	tail->next   = pool->iters;
	pool->iters  = iters;
	pool->count += count;

	// free all blocks when not needed
	pool->refcount -= count;
	if(pool->refcount == 0)
	{
		pool->iters = NULL;

		cc_listBlock_t* block = pool->blocks;
		while(block)
		{
			pool->blocks = pool->blocks->next;
			FREE(block);
			block = pool->blocks;
		}
	}

	pthread_mutex_unlock(&pool->mutex);
}

/***********************************************************
* private                                                  *
***********************************************************/

static void
cc_listIter_add(cc_listIter_t* self, cc_list_t* list,
                cc_listIter_t* prev, cc_listIter_t* next)
{
	// prev and next can be NULL
	ASSERT(self);
	ASSERT(list);

	self->next = next;
	self->prev = prev;

	// update next/prev nodes
	if(next)
	{
		next->prev = self;
	}
	if(prev)
	{
		prev->next = self;
	}

	// update the list
	if(prev == NULL)
	{
		list->head = self;
	}
	if(next == NULL)
	{
		list->tail = self;
	}
	++list->size;
}

static void
cc_listIter_remove(cc_listIter_t* self, cc_list_t* list)
{
	ASSERT(self);
	ASSERT(list);

	// update next/prev nodes
	if(self->prev)
	{
		self->prev->next = self->next;
	}
	if(self->next)
	{
		self->next->prev = self->prev;
	}

	// update the list
	if(self == list->head)
	{
		list->head = self->next;
	}
	if(self == list->tail)
	{
		list->tail = self->prev;
	}
	--list->size;

	self->next = NULL;
	self->prev = NULL;
}

static void
cc_listIter_move(cc_listIter_t* self, cc_list_t* list,
                 cc_listIter_t* prev, cc_listIter_t* next)
{
	// prev or next may be NULL but not both
	ASSERT(self);
	ASSERT(list);
	ASSERT(prev || next);

	// remove node
	if(self->prev)
	{
		self->prev->next = self->next;
	}
	if(self->next)
	{
		self->next->prev = self->prev;
	}

	// update the list
	// use "else if" since size >= 2 for move
	if(self == list->head)
	{
		list->head = self->next;
	}
	else if(self == list->tail)
	{
		list->tail = self->prev;
	}

	// add node
	self->prev = prev;
	self->next = next;

	// update next/prev nodes
	if(next)
	{
		next->prev = self;
	}
	if(prev)
	{
		prev->next = self;
	}

	// update the list
	// use "else if" since size >= 2 for move
	if(prev == NULL)
	{
		list->head = self;
	}
	else if(next == NULL)
	{
		list->tail = self;
	}
}

static cc_listIter_t*
cc_listIter_new(cc_list_t* list, cc_listIter_t* prev,
                cc_listIter_t* next, const void* data)
{
	// prev and next can be NULL
	ASSERT(list);
	ASSERT(data);

	cc_listIter_t* self;
	if(list->flags & CC_LIST_FLAG_CMALLOC)
	{
		self = (cc_listIter_t*)
		       calloc(1, sizeof(cc_listIter_t));
	}
	else if(list->iters)
	{
		// use a local free iter
		self        = list->iters;
		list->iters = list->iters->next;
	}
	else
	{
		// get a set of free iters
		self = cc_listPool_get();

		// keep first iter from set and
		// add remaining to list of free iters
		if(self)
		{
			cc_listIter_t* iter = self->next;
			cc_listIter_t* next;
			while(iter)
			{
				next        = iter->next;
				iter->next  = list->iters;
				list->iters = iter;
				iter        = next;
			}
		}
	}

	if(self == NULL)
	{
		LOGE("CALLOC failed");
		return NULL;
	}

	// add iter to list
	cc_listIter_add(self, list, prev, next);
	self->data = data;

	return self;
}

static const void*
cc_listIter_delete(cc_listIter_t** _self, cc_list_t* list)
{
	ASSERT(_self);
	ASSERT(list);

	cc_listIter_t* self = *_self;
	cc_listIter_t* next = NULL;
	const void*    data = NULL;
	if(self)
	{
		// remove iter from list
		next       = self->next;
		data       = self->data;
		self->data = NULL;
		cc_listIter_remove(self, list);

		if(list->flags & CC_LIST_FLAG_CMALLOC)
		{
			free(self);
		}
		else
		{
			// add iter to list of free iters
			self->next = list->iters;
			list->iters = self;
		}


		*_self = next;
	}

	return data;
}

static cc_list_t* cc_list_newFlags(int flags)
{
	cc_list_t* self;
	if(flags & CC_LIST_FLAG_CMALLOC)
	{
		self = (cc_list_t*) calloc(1, sizeof(cc_list_t));
	}
	else
	{
		self = (cc_list_t*) CALLOC(1, sizeof(cc_list_t));
	}

	if(self == NULL)
	{
		LOGE("CALLOC failed");
		return NULL;
	}

	self->flags = flags;

	return self;
}

/***********************************************************
* protected                                                *
***********************************************************/

// the newCMalloc protected function is intended to be
// used by cc_memory debug feature which depends on cc_list
// but cannot use the cc_memory tracking without deadlocks
cc_list_t* cc_list_newCMalloc(void)
{
	return cc_list_newFlags(CC_LIST_FLAG_CMALLOC);
}

/***********************************************************
* public                                                   *
***********************************************************/

cc_list_t* cc_list_new(void)
{
	return cc_list_newFlags(0);
}

void cc_list_delete(cc_list_t** _self)
{
	ASSERT(_self);

	cc_list_t* self = *_self;
	if(self)
	{
		if(self->size > 0)
		{
			LOGE("memory leak detected: size=%i", self->size);
			cc_list_discard(self);
		}

		if(self->flags & CC_LIST_FLAG_CMALLOC)
		{
			free(self);
		}
		else
		{
			// put the list of free iters
			cc_listPool_put(self->iters);

			FREE(self);
		}

		*_self = NULL;
	}
}

void cc_list_discard(cc_list_t* self)
{
	ASSERT(self);

	// discard all iters in the list without freeing
	// this is useful when the list just holds references
	cc_listIter_t* iter = cc_list_head(self);
	while(iter)
	{
		cc_list_remove(self, &iter);
	}
}

int cc_list_size(const cc_list_t* self)
{
	ASSERT(self);
	return self->size;
}

size_t cc_list_sizeof(const cc_list_t* self)
{
	ASSERT(self);

	return sizeof(cc_list_t) +
	       self->size*sizeof(cc_listIter_t);
}

const void* cc_list_peekHead(const cc_list_t* self)
{
	ASSERT(self);

	return (self->size == 0) ? NULL : self->head->data;
}

const void* cc_list_peekTail(const cc_list_t* self)
{
	ASSERT(self);

	return (self->size == 0) ? NULL : self->tail->data;
}

const void* cc_list_peekIter(const cc_listIter_t* iter)
{
	ASSERT(iter);

	return iter->data;
}

const void*
cc_list_peekIndex(const cc_list_t* self, int idx)
{
	ASSERT(self);

	cc_listIter_t* iter;
	iter = cc_list_head(self);
	while(iter)
	{
		if(idx == 0)
		{
			return cc_list_peekIter(iter);
		}
		--idx;

		iter = cc_list_next(iter);
	}

	return NULL;
}

cc_listIter_t* cc_list_head(const cc_list_t* self)
{
	ASSERT(self);

	return self->head;
}

cc_listIter_t* cc_list_tail(const cc_list_t* self)
{
	ASSERT(self);

	return self->tail;
}

cc_listIter_t* cc_list_next(cc_listIter_t* iter)
{
	ASSERT(iter);

	return iter->next;
}

cc_listIter_t* cc_list_prev(cc_listIter_t* iter)
{
	ASSERT(iter);

	return iter->prev;
}

cc_listIter_t* cc_list_get(cc_list_t* self, int idx)
{
	ASSERT(self);

	cc_listIter_t* iter = cc_list_head(self);
	while(iter)
	{
		if(idx == 0)
		{
			return iter;
		}

		iter = cc_list_next(iter);
		--idx;
	}

	return NULL;
}

cc_listIter_t*
cc_list_find(const cc_list_t* self, const void* data,
             cc_listcmp_fn compare)
{
	ASSERT(self);
	ASSERT(data);
	ASSERT(compare);

	cc_listIter_t* iter = self->head;
	while(iter)
	{
		if((*compare)(iter->data, data) == 0)
		{
			return iter;
		}
		iter = iter->next;
	}
	return NULL;
}

cc_listIter_t*
cc_list_findSorted(const cc_list_t* self, const void* data,
                   cc_listcmp_fn compare)
{
	ASSERT(self);
	ASSERT(data);
	ASSERT(compare);

	cc_listIter_t* iter = self->head;
	while(iter)
	{
		int cmp = (*compare)(data, iter->data);
		if(cmp == 0)
		{
			return iter;
		}
		else if(cmp < 0)
		{
			return NULL;
		}
		iter = iter->next;
	}
	return NULL;
}

cc_listIter_t*
cc_list_insertSorted(cc_list_t* self, cc_listcmp_fn compare,
                     const void* data)
{
	ASSERT(self);
	ASSERT(compare);
	ASSERT(data);

	cc_listIter_t* iter = cc_list_head(self);
	while(iter)
	{
		const void* d = cc_list_peekIter(iter);
		if((*compare)(data, d) < 0)
		{
			return cc_list_insert(self, iter, data);
		}

		iter = cc_list_next(iter);
	}

	return cc_list_append(self, NULL, data);
}

cc_listIter_t*
cc_list_insert(cc_list_t* self, cc_listIter_t* iter,
               const void* data)
{
	// iter may be null for empty list or to insert at head
	// cc_list_insert(list, NULL, data) may be preferred over
	// cc_list_push(list, data) when a listIter is needed
	ASSERT(self);
	ASSERT(data);

	if(iter)
	{
		return cc_listIter_new(self, iter->prev, iter, data);
	}
	else
	{
		return cc_listIter_new(self, NULL, self->head, data);
	}
}

cc_listIter_t*
cc_list_append(cc_list_t* self, cc_listIter_t* iter,
               const void* data)
{
	// iter may be null for empty list or to append at tail
	// cc_list_append(list, NULL, data) may be preferred over
	// cc_list_enqueue(list, data) when a listIter is needed
	ASSERT(self);
	ASSERT(data);

	if(iter)
	{
		return cc_listIter_new(self, iter, iter->next, data);
	}
	else
	{
		return cc_listIter_new(self, self->tail, NULL, data);
	}
}

const void*
cc_list_replace(cc_listIter_t* iter,
                const void* data)
{
	ASSERT(iter);
	ASSERT(data);

	const void* tmp = iter->data;
	iter->data = data;

	return tmp;
}

const void*
cc_list_remove(cc_list_t* self, cc_listIter_t** _iter)
{
	ASSERT(self);
	ASSERT(_iter);

	return cc_listIter_delete(_iter, self);
}

void cc_list_move(cc_list_t* self, cc_listIter_t* from,
                  cc_listIter_t* to)
{
	// to may be NULL
	ASSERT(self);
	ASSERT(from);

	if(to == NULL)
	{
		to = cc_list_head(self);
	}

	cc_listIter_t* prev = to->prev;
	if((from == to) || (from == prev))
	{
		return;
	}

	cc_listIter_move(from, self, prev, to);
}

void cc_list_moven(cc_list_t* self, cc_listIter_t* from,
                   cc_listIter_t* to)
{
	// to may be NULL
	ASSERT(self);
	ASSERT(from);

	if(to == NULL)
	{
		to = cc_list_tail(self);
	}

	cc_listIter_t* next = to->next;
	if((from == to) || (from == next))
	{
		return;
	}

	cc_listIter_move(from, self, to, next);
}

void cc_list_swap(cc_list_t* fromList, cc_list_t* toList,
                  cc_listIter_t* from, cc_listIter_t* to)
{
	// to may be NULL
	ASSERT(fromList);
	ASSERT(toList);
	ASSERT(from);

	if(fromList == toList)
	{
		cc_list_move(fromList, from, to);
		return;
	}

	cc_listIter_remove(from, fromList);
	if(to == NULL)
	{
		cc_listIter_t* head = cc_list_head(toList);
		cc_listIter_add(from, toList, NULL, head);
	}
	else
	{
		cc_listIter_add(from, toList, to->prev, to);
	}
}

void cc_list_swapn(cc_list_t* fromList, cc_list_t* toList,
                   cc_listIter_t* from, cc_listIter_t* to)
{
	// to may be NULL
	ASSERT(fromList);
	ASSERT(toList);
	ASSERT(from);

	if(fromList == toList)
	{
		cc_list_moven(fromList, from, to);
		return;
	}

	cc_listIter_remove(from, fromList);
	if(to == NULL)
	{
		cc_listIter_t* tail = cc_list_tail(toList);
		cc_listIter_add(from, toList, tail, NULL);
	}
	else
	{
		cc_listIter_add(from, toList, to, to->next);
	}
}

void cc_list_appendList(cc_list_t* self, cc_list_t* from)
{
	ASSERT(self);
	ASSERT(from);

	if(from->size == 0)
	{
		return;
	}
	else if(self->size == 0)
	{
		self->head = from->head;
		self->tail = from->tail;
		self->size = from->size;
		from->head = NULL;
		from->tail = NULL;
		from->size = 0;
		return;
	}

	self->tail->next = from->head;
	from->head->prev = self->tail;
	self->tail = from->tail;
	self->size += from->size;
	from->head = NULL;
	from->tail = NULL;
	from->size = 0;
}

void cc_list_insertList(cc_list_t* self, cc_list_t* from)
{
	ASSERT(self);
	ASSERT(from);

	if(from->size == 0)
	{
		return;
	}
	else if(self->size == 0)
	{
		self->head = from->head;
		self->tail = from->tail;
		self->size = from->size;
		from->head = NULL;
		from->tail = NULL;
		from->size = 0;
		return;
	}

	self->head->prev = from->tail;
	from->tail->next = self->head;
	self->head = from->head;
	self->size += from->size;
	from->head = NULL;
	from->tail = NULL;
	from->size = 0;
}
