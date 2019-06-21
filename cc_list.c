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

#include <assert.h>
#include <stdlib.h>

#define LOG_TAG "cc"
#include "cc_list.h"
#include "cc_log.h"
#include "cc_memory.h"

/***********************************************************
* private                                                  *
***********************************************************/

static void
cc_listIter_add(cc_listIter_t* self, cc_list_t* list,
                cc_listIter_t* prev, cc_listIter_t* next)
{
	// prev and next can be NULL
	assert(self);
	assert(list);

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
	assert(self);
	assert(list);

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
	assert(self);
	assert(list);
	assert(prev || next);

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
	assert(list);
	assert(data);

	cc_listIter_t* self;
	self = (cc_listIter_t*)
	       MALLOC(sizeof(cc_listIter_t));
	if(self == NULL)
	{
		LOGE("MALLOC failed");
		return NULL;
	}

	self->next = NULL;
	self->prev = NULL;
	self->data = data;
	cc_listIter_add(self, list, prev, next);
	return self;
}

static const void*
cc_listIter_delete(cc_listIter_t** _self, cc_list_t* list)
{
	assert(_self);
	assert(list);

	cc_listIter_t* self = *_self;
	cc_listIter_t* next = NULL;
	const void*    data = NULL;
	if(self)
	{
		next = self->next;
		data = self->data;
		cc_listIter_remove(self, list);
		FREE(self);
		*_self = next;
	}

	return data;
}

/***********************************************************
* public                                                   *
***********************************************************/

cc_list_t* cc_list_new(void)
{
	cc_list_t* self;
	self = (cc_list_t*) MALLOC(sizeof(cc_list_t));
	if(self == NULL)
	{
		LOGE("MALLOC failed");
		return NULL;
	}

	self->size = 0;
	self->head = NULL;
	self->tail = NULL;

	return self;
}

void cc_list_delete(cc_list_t** _self)
{
	assert(_self);

	cc_list_t* self = *_self;
	if(self)
	{
		if(self->size > 0)
		{
			LOGE("memory leak detected: size=%i", self->size);
			cc_list_discard(self);
		}

		FREE(self);
		*_self = NULL;
	}
}

void cc_list_discard(cc_list_t* self)
{
	assert(self);

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
	assert(self);
	return self->size;
}

size_t cc_list_sizeof(const cc_list_t* self)
{
	assert(self);

	return sizeof(cc_list_t) +
	       self->size*sizeof(cc_listIter_t);
}

const void* cc_list_peekHead(const cc_list_t* self)
{
	assert(self);

	return (self->size == 0) ? NULL : self->head->data;
}

const void* cc_list_peekTail(const cc_list_t* self)
{
	assert(self);

	return (self->size == 0) ? NULL : self->tail->data;
}

const void* cc_list_peekIter(cc_listIter_t* iter)
{
	assert(iter);

	return iter->data;
}

cc_listIter_t* cc_list_head(const cc_list_t* self)
{
	assert(self);

	return self->head;
}

cc_listIter_t* cc_list_tail(const cc_list_t* self)
{
	assert(self);

	return self->tail;
}

cc_listIter_t* cc_list_next(cc_listIter_t* iter)
{
	assert(iter);

	return iter->next;
}

cc_listIter_t* cc_list_prev(cc_listIter_t* iter)
{
	assert(iter);

	return iter->prev;
}

cc_listIter_t*
cc_list_find(const cc_list_t* self, const void* data,
             cc_listcmp_fn compare)
{
	assert(self);
	assert(data);
	assert(compare);

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
	assert(self);
	assert(data);
	assert(compare);

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
cc_list_insert(cc_list_t* self, cc_listIter_t* iter,
               const void* data)
{
	// iter may be null for empty list or to insert at head
	// cc_list_insert(list, NULL, data) may be preferred over
	// cc_list_push(list, data) when a listIter is needed
	assert(self);
	assert(data);

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
cc_list_insertSorted(cc_list_t* self, cc_listcmp_fn compare,
                     const void* data)
{
	assert(self);
	assert(compare);
	assert(data);

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
cc_list_append(cc_list_t* self, cc_listIter_t* iter,
               const void* data)
{
	// iter may be null for empty list or to append at tail
	// cc_list_append(list, NULL, data) may be preferred over
	// cc_list_enqueue(list, data) when a listIter is needed
	assert(self);
	assert(data);

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
cc_list_replace(cc_list_t* self, cc_listIter_t* iter,
                const void* data)
{
	assert(self);
	assert(iter);
	assert(data);

	const void* tmp = iter->data;
	iter->data = data;

	return tmp;
}

const void*
cc_list_remove(cc_list_t* self, cc_listIter_t** _iter)
{
	assert(self);
	assert(_iter);

	return cc_listIter_delete(_iter, self);
}

void cc_list_move(cc_list_t* self, cc_listIter_t* from,
                  cc_listIter_t* to)
{
	// to may be NULL
	assert(self);
	assert(from);

	if(to == NULL)
	{
		to = cc_list_head(self);
	}

	if(from == to)
	{
		return;
	}

	cc_listIter_move(from, self, to->prev, to);
}

void cc_list_moven(cc_list_t* self, cc_listIter_t* from,
                   cc_listIter_t* to)
{
	// to may be NULL
	assert(self);
	assert(from);

	if(to == NULL)
	{
		to = cc_list_tail(self);
	}

	if(from == to)
	{
		return;
	}

	cc_listIter_move(from, self, to, to->next);
}

void cc_list_swap(cc_list_t* fromList, cc_list_t* toList,
                  cc_listIter_t* from, cc_listIter_t* to)
{
	// to may be NULL
	assert(fromList);
	assert(toList);
	assert(from);

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
	assert(fromList);
	assert(toList);
	assert(from);

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
	assert(self);
	assert(from);

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
	assert(self);
	assert(from);

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
