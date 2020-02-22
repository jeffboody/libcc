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

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LOG_TAG "cc"
#include "cc_map.h"
#include "cc_memory.h"
#include "cc_log.h"

#define CC_MAP_FLAG_CMALLOC 1

/***********************************************************
* private - mapIter                                        *
***********************************************************/

static void
cc_mapIter_init(cc_mapIter_t* self, cc_mapNode_t* node)
{
	ASSERT(self);
	ASSERT(node);

	self->depth   = 0;
	self->key[0]  = node->k;
	self->key[1]  = '\0';
	self->node[0] = node;
}

static void
cc_mapIter_update(cc_mapIter_t* self, int d,
                  cc_mapNode_t* node)
{
	ASSERT(self);
	ASSERT(d >= 0);
	ASSERT(d < (CC_MAP_KEYLEN - 1));
	ASSERT(node);

	self->depth      = d;
	self->key[d]     = node->k;
	self->key[d + 1] = '\0';
	self->node[d]    = node;
}

/***********************************************************
* private - mapNode                                        *
***********************************************************/

static cc_mapNode_t*
cc_mapNode_new(cc_mapNode_t* prev, cc_map_t* map, char k)
{
	ASSERT(map);

	// prev may be NULL for head

	cc_mapNode_t* self;
	if(map->flags & CC_MAP_FLAG_CMALLOC)
	{
		self = (cc_mapNode_t*)
		       malloc(sizeof(cc_mapNode_t));
	}
	else
	{
		self = (cc_mapNode_t*)
		       MALLOC(sizeof(cc_mapNode_t));
	}

	if(self == NULL)
	{
		LOGE("MALLOC failed");
		return NULL;
	}

	self->prev = prev;
	self->next = NULL;
	self->down = NULL;
	self->val  = NULL;
	self->k    = k;

	++map->nodes;

	return self;
}

static void
cc_mapNode_delete(cc_mapNode_t** _self, cc_map_t* map)
{
	ASSERT(_self);
	ASSERT(map);

	cc_mapNode_t* self = *_self;
	if(self)
	{
		// prev is a reference
		--map->nodes;
		cc_mapNode_delete(&self->next, map);
		cc_mapNode_delete(&self->down, map);
		if(map->flags & CC_MAP_FLAG_CMALLOC)
		{
			free(self);
		}
		else
		{
			FREE(self);
		}
		*_self = NULL;
	}
}

/***********************************************************
* private map                                              *
***********************************************************/

static void
cc_map_clean(cc_map_t* self, cc_mapNode_t* node)
{
	ASSERT(self);
	ASSERT(node);

	// check if the node is an endpoint or traversal node
	if(node->val || node->down)
	{
		return;
	}

	// detach empty nodes
	cc_mapNode_t* prev = node->prev;
	cc_mapNode_t* next = node->next;
	if(prev == NULL)
	{
		self->head = next;
	}
	else if(prev->down == node)
	{
		prev->down = next;
		cc_map_clean(self, prev);
	}
	else
	{
		prev->next = next;
	}

	if(next)
	{
		next->prev = prev;
	}

	node->prev = NULL;
	node->next = NULL;
	cc_mapNode_delete(&node, self);
}

static cc_map_t* cc_map_newFlags(int flags)
{
	cc_map_t* self;
	if(flags & CC_MAP_FLAG_CMALLOC)
	{
		self = (cc_map_t*) malloc(sizeof(cc_map_t));
	}
	else
	{
		self = (cc_map_t*) MALLOC(sizeof(cc_map_t));
	}

	if(self == NULL)
	{
		LOGE("MALLOC failed");
		return NULL;
	}

	self->flags = flags;
	self->size  = 0;
	self->nodes = 0;
	self->head  = NULL;

	return self;
}

/***********************************************************
* protected                                                *
***********************************************************/

// the newCMalloc protected function is intended to be
// used by cc_memory debug feature which depends on cc_map
// but cannot use the cc_memory tracking without deadlocks
cc_map_t* cc_map_newCMalloc(void)
{
	return cc_map_newFlags(CC_MAP_FLAG_CMALLOC);
}

/***********************************************************
* public                                                   *
***********************************************************/

cc_map_t* cc_map_new(void)
{
	return cc_map_newFlags(0);
}

void cc_map_delete(cc_map_t** _self)
{
	ASSERT(_self);

	cc_map_t* self = *_self;
	if(self)
	{
		if(self->size > 0)
		{
			LOGE("memory leak detected: size=%i", self->size);
		}

		cc_mapNode_delete(&self->head, self);
		if(self->flags & CC_MAP_FLAG_CMALLOC)
		{
			free(self);
		}
		else
		{
			FREE(self);
		}
		*_self = NULL;
	}
}

void cc_map_discard(cc_map_t* self)
{
	ASSERT(self);

	self->size = 0;
	cc_mapNode_delete(&self->head, self);
}

int cc_map_size(const cc_map_t* self)
{
	ASSERT(self);

	return self->size;
}

size_t cc_map_sizeof(const cc_map_t* self)
{
	ASSERT(self);

	return sizeof(cc_map_t) +
	       self->nodes*sizeof(cc_mapNode_t);
}

cc_mapIter_t*
cc_map_head(const cc_map_t* self, cc_mapIter_t* iter)
{
	ASSERT(self);
	ASSERT(iter);

	if(self->head == NULL)
	{
		return NULL;
	}

	cc_mapIter_init(iter, self->head);

	// find an endpoint
	if(self->head->val)
	{
		return iter;
	}
	return cc_map_next(iter);
}

cc_mapIter_t* cc_map_next(cc_mapIter_t* iter)
{
	ASSERT(iter);

	int d = iter->depth;
	cc_mapNode_t* node = iter->node[d];
	if(node->down)
	{
		// down
		++d;
		node = node->down;
		cc_mapIter_update(iter, d, node);
	}
	else if(node->next)
	{
		// sideways
		node = node->next;
		cc_mapIter_update(iter, d, node);
	}
	else
	{
		// up
		while(1)
		{
			--d;
			if(d < 0)
			{
				return NULL;
			}

			node = iter->node[d];
			if(node->next)
			{
				node = node->next;
				cc_mapIter_update(iter, d, node);
				break;
			}
		}
	}

	// find an endpoint
	if(node->val)
	{
		return iter;
	}
	return cc_map_next(iter);
}

const void* cc_map_val(const cc_mapIter_t* iter)
{
	ASSERT(iter);

	int d = iter->depth;
	cc_mapNode_t* node = iter->node[d];
	return node->val;
}

const char* cc_map_key(const cc_mapIter_t* iter)
{
	ASSERT(iter);

	return iter->key;
}

const void*
cc_map_find(const cc_map_t* self, cc_mapIter_t* iter,
            const char* key)
{
	ASSERT(self);
	ASSERT(iter);
	ASSERT(key);

	int len = strlen(key);
	if((len >= CC_MAP_KEYLEN) || (len == 0))
	{
		LOGE("invalid len=%i", len);
		return NULL;
	}

	// check for empty map
	if(self->head == NULL)
	{
		return NULL;
	}

	cc_mapIter_init(iter, self->head);

	// traverse the map
	int d = 0;
	cc_mapNode_t* node = self->head;
	while(d < len)
	{
		if(key[d] == iter->key[d])
		{
			if(d == (len - 1))
			{
				// success
				return node->val;
			}
			else if(node->down)
			{
				// down
				++d;
				node = node->down;
				cc_mapIter_update(iter, d, node);
				continue;
			}

			return NULL;
		}
		else if((key[d] < iter->key[d]) ||
		        (node->next == NULL))
		{
			// not found
			return NULL;
		}

		// traverse to the next decision point
		while(node->next && (key[d] > iter->key[d]))
		{
			// sideways
			node = node->next;
			cc_mapIter_update(iter, d, node);
		}
	}

	return NULL;
}

const void*
cc_map_findf(const cc_map_t* self, cc_mapIter_t* iter,
             const char* fmt, ...)
{
	ASSERT(self);
	ASSERT(iter);
	ASSERT(fmt);

	char key[CC_MAP_KEYLEN];
	va_list argptr;
	va_start(argptr, fmt);
	vsnprintf(key, CC_MAP_KEYLEN, fmt, argptr);
	va_end(argptr);

	return cc_map_find(self, iter, key);
}

int cc_map_add(cc_map_t* self, const void* val,
               const char* key)
{
	ASSERT(self);
	ASSERT(val);
	ASSERT(key);

	int len = strlen(key);
	if((len >= CC_MAP_KEYLEN) || (len == 0))
	{
		LOGE("invalid key=%s, len=%i", key, len);
		return 0;
	}

	int created = 0;
	if(self->head == NULL)
	{
		self->head = cc_mapNode_new(NULL, self, key[0]);
		if(self->head == NULL)
		{
			return 0;
		}

		created = 1;
	}

	cc_mapIter_t  iterator;
	cc_mapIter_t* iter = &iterator;
	cc_mapIter_init(&iterator, self->head);

	// traverse the map
	int d = 0;
	cc_mapNode_t* node = self->head;
	while(d < len)
	{
		if(key[d] == iter->key[d])
		{
			if(d == (len - 1))
			{
				if(node->val)
				{
					// hash already contains key
					return 0;
				}

				// success
				++self->size;
				node->val = val;
				return 1;
			}
			else if(node->down == NULL)
			{
				// insert a new down node
				cc_mapNode_t* down;
				down = cc_mapNode_new(node, self, key[d + 1]);
				if(down == NULL)
				{
					cc_map_clean(self, node);
					goto fail_add;
				}
				node->down = down;
			}

			// down
			++d;
			node = node->down;
			cc_mapIter_update(iter, d, node);
			continue;
		}
		else if(key[d] < iter->key[d])
		{
			// insert a new prev node
			cc_mapNode_t* prev;
			prev = cc_mapNode_new(node->prev, self, key[d]);
			if(prev == NULL)
			{
				cc_map_clean(self, node);
				goto fail_add;
			}
			prev->next = node;

			if(node->prev)
			{
				if(node->prev->next == node)
				{
					node->prev->next = prev;
				}
				else
				{
					node->prev->down = prev;
				}
			}
			else
			{
				self->head = prev;
			}
			node->prev = prev;

			// sideways
			node = prev;
			cc_mapIter_update(iter, d, node);
			continue;
		}
		else if(node->next == NULL)
		{
			// append a new next node
			cc_mapNode_t* next;
			next = cc_mapNode_new(node, self, key[d]);
			if(next == NULL)
			{
				cc_map_clean(self, node);
				goto fail_add;
			}
			node->next = next;

			node = next;
			cc_mapIter_update(iter, d, node);
			continue;
		}

		// traverse to the next decision point
		while(node->next && (key[d] > iter->key[d]))
		{
			// sideways
			node = node->next;
			cc_mapIter_update(iter, d, node);
		}
	}

	// the key/val can always be added before d equals len
	ASSERT(0);

	// failure
	fail_add:
		if(created)
		{
			cc_mapNode_delete(&self->head, self);
		}
	return 0;
}

int cc_map_addf(cc_map_t* self, const void* val,
                const char* fmt, ...)
{
	ASSERT(self);
	ASSERT(val);
	ASSERT(fmt);

	char key[CC_MAP_KEYLEN];
	va_list argptr;
	va_start(argptr, fmt);
	vsnprintf(key, CC_MAP_KEYLEN, fmt, argptr);
	va_end(argptr);

	return cc_map_add(self, val, key);
}

const void*
cc_map_replace(cc_mapIter_t* iter, const void*  val)
{
	ASSERT(iter);
	ASSERT(val);

	int d = iter->depth;
	cc_mapNode_t* node = iter->node[d];

	const void* old = node->val;
	node->val = val;
	return old;
}

const void*
cc_map_remove(cc_map_t* self, cc_mapIter_t** _iter)
{
	ASSERT(self);
	ASSERT(_iter);
	ASSERT(*_iter);

	cc_mapIter_t* iter = *_iter;

	// save node and update iter
	int d = iter->depth;
	cc_mapNode_t* node = iter->node[d];
	*_iter = cc_map_next(iter);

	// clear value and clean traversal nodes
	const void* val = node->val;
	node->val = NULL;
	cc_map_clean(self, node);

	--self->size;

	return val;
}
