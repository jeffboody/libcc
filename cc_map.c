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

#include <inttypes.h>
#include <limits.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LOG_TAG "cc"
#include "cc_map.h"
#include "cc_memory.h"
#include "cc_mumurhash3.h"
#include "cc_log.h"

#define CC_MAP_FLAG_CMALLOC 1

#define CC_MAP_KEYLEN 256

#define CC_MAP_CAPACITY 16

#define CC_MAP_IDX(map, hash) (hash/map->elements)

// protected
cc_list_t* cc_list_newCMalloc(void);

/***********************************************************
* private - mapNode                                        *
***********************************************************/

// note that key must be 8-byte aligned for
// cc_mumurhash3 and cc_mapNode_cmp to work
typedef struct cc_mapNode_s
{
	const void* val;
	uint32_t    hash;
	int         len;
	// uint8_t  key[];
} cc_mapNode_t;

static size_t cc_mapNode_sizeof(cc_mapNode_t* self)
{
	ASSERT(self);

	return sizeof(cc_mapNode_t) + self->len;
}

static uint8_t* cc_mapNode_key(cc_mapNode_t* self)
{
	ASSERT(self);

	return (uint8_t*)
	       (((void*) self) + sizeof(cc_mapNode_t));
}

static cc_mapNode_t*
cc_mapNode_new(cc_map_t* map, const void* val,
               uint32_t hash, int idx, int len,
               const uint8_t* key)
{
	ASSERT(map);
	ASSERT(val);
	ASSERT(len > 0);
	ASSERT(key);

	size_t size;
	size = sizeof(cc_mapNode_t) + len;

	cc_mapNode_t* self;
	if(map->flags & CC_MAP_FLAG_CMALLOC)
	{
		self = (cc_mapNode_t*) calloc(1, size);
	}
	else
	{
		self = (cc_mapNode_t*) CALLOC(1, size);
	}

	if(self == NULL)
	{
		LOGE("CALLOC failed");
		return NULL;
	}

	self->hash = hash;
	self->val  = val;
	self->len  = len;

	uint8_t* dst = cc_mapNode_key(self);
	memcpy((void*) dst, (const void*) key, len);

	map->nodes_size += cc_mapNode_sizeof(self);

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
		map->nodes_size -= cc_mapNode_sizeof(self);

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

static int
cc_mapNode_cmp(cc_mapNode_t* self,
               uint32_t hash, int len, const uint8_t* key)
{
	ASSERT(self);
	ASSERT(key);

	// use hash for primary comparison
	if(self->hash > hash)
	{
		return 1;
	}
	else if(self->hash < hash)
	{
		return -1;
	}

	// use key len for secondary comparison
	// longer keys are greater than shorter keys
	if(self->len > len)
	{
		return 1;
	}
	else if(self->len < len)
	{
		return -1;
	}

	const uint64_t* key64a;
	const uint64_t* key64b;
	key64a = (const uint64_t*) cc_mapNode_key(self);
	key64b = (const uint64_t*) key;

	// otherwise compare key bytes
	// compare initial bytes 8 at a time
	int cnt = len/8;
	int idx = 0;
	while(cnt)
	{
		if(key64a[idx] > key64b[idx])
		{
			return 1;
		}
		else if(key64a[idx] < key64b[idx])
		{
			return -1;
		}

		++idx;
		--cnt;
	}

	const uint32_t* key32a = (const uint32_t*) key64a;
	const uint32_t* key32b = (const uint32_t*) key64b;

	// compare bytes 4 at a time
	cnt = len % 8;
	idx = 2*idx;
	if(cnt >= 4)
	{
		if(key32a[idx] > key32b[idx])
		{
			return 1;
		}
		else if(key32a[idx] < key32b[idx])
		{
			return -1;
		}

		++idx;
		cnt -= 4;
	}

	const uint8_t* key8a = (const uint8_t*) key64a;
	const uint8_t* key8b = (const uint8_t*) key64b;

	// compare remaining bytes one at a time
	idx = 4*idx;
	while(cnt)
	{
		if(key8a[idx] > key8b[idx])
		{
			return 1;
		}
		else if(key8a[idx] < key8b[idx])
		{
			return -1;
		}

		++idx;
		--cnt;
	}

	return 0;
}

/***********************************************************
* private                                                  *
***********************************************************/

static cc_map_t* cc_map_newFlags(int flags)
{
	cc_map_t* self;
	if(flags & CC_MAP_FLAG_CMALLOC)
	{
		self = (cc_map_t*) calloc(1, sizeof(cc_map_t));
	}
	else
	{
		self = (cc_map_t*) CALLOC(1, sizeof(cc_map_t));
	}

	if(self == NULL)
	{
		LOGE("CALLOC failed");
		return NULL;
	}

	self->flags    = flags;
	self->seed     = random();
	self->capacity = CC_MAP_CAPACITY;
	self->elements = (uint32_t)
	                 (((uint64_t) UINT_MAX + 1)/
	                  ((uint64_t) self->capacity));
	if(flags & CC_MAP_FLAG_CMALLOC)
	{
		self->buckets = (cc_listIter_t**)
		                calloc(self->capacity,
		                       sizeof(cc_listIter_t*));
	}
	else
	{
		self->buckets = (cc_listIter_t**)
		                CALLOC(self->capacity,
		                       sizeof(cc_listIter_t*));
	}

	if(self->buckets == NULL)
	{
		goto fail_buckets;
	}

	if(flags & CC_MAP_FLAG_CMALLOC)
	{
		self->nodes = cc_list_newCMalloc();
	}
	else
	{
		self->nodes = cc_list_new();
	}

	if(self->nodes == NULL)
	{
		goto fail_nodes;
	}

	// success
	return self;

	// failure
	fail_nodes:
	{
		if(flags & CC_MAP_FLAG_CMALLOC)
		{
			free(self->buckets);
		}
		else
		{
			FREE(self->buckets);
		}
	}
	fail_buckets:
	{
		if(flags & CC_MAP_FLAG_CMALLOC)
		{
			free(self);
		}
		else
		{
			FREE(self);
		}
	}
	return NULL;
}

static void
cc_map_grow(cc_map_t* self)
{
	ASSERT(self);

	// check capacity
	if(cc_list_size(self->nodes) <= self->capacity)
	{
		return;
	}

	int    capacity1 = self->capacity;
	int    capacity2 = 2*self->capacity;
	size_t size2     = capacity2*sizeof(cc_listIter_t*);

	// try to grow the capacity
	cc_listIter_t** buckets2;
	if(self->flags & CC_MAP_FLAG_CMALLOC)
	{
		buckets2 = (cc_listIter_t**)
		           realloc(self->buckets, size2);
	}
	else
	{
		buckets2 = (cc_listIter_t**)
		           REALLOC(self->buckets, size2);
	}

	if(buckets2 == NULL)
	{
		return;
	}
	self->capacity = capacity2;
	self->elements = (uint32_t)
	                 (((uint64_t) UINT_MAX + 1)/
	                  ((uint64_t) self->capacity));
	self->buckets  = buckets2;

	// update buckets
	int i;
	int j;
	int k;
	int idx;
	cc_listIter_t* iter;
	cc_mapNode_t*  node;
	for(k = (capacity1 - 1); k >= 0; k -= 1)
	{
		iter = self->buckets[k];

		i = 2*k;
		j = i + 1;
		self->buckets[i] = NULL;
		self->buckets[j] = NULL;

		if(iter == NULL)
		{
			continue;
		}

		// update bucket[i]
		node = (cc_mapNode_t*) cc_list_peekIter(iter);
		idx  = CC_MAP_IDX(self, node->hash);
		if(idx == i)
		{
			self->buckets[i] = iter;
		}

		// update bucket[j]
		while(iter)
		{
			node = (cc_mapNode_t*) cc_list_peekIter(iter);
			idx  = CC_MAP_IDX(self, node->hash);
			if(idx == j)
			{
				self->buckets[j] = iter;
				break;
			}
			else if(idx > j)
			{
				break;
			}

			iter = cc_list_next(iter);
		}
	}
}

static cc_mapIter_t*
cc_map_addAt(cc_map_t* self, cc_mapIter_t* miter_at,
             uint32_t hash, int idx, const void* val,
             int len, const uint8_t* key)
{
	// miter_at may be NULL
	ASSERT(self);
	ASSERT(val);
	ASSERT(key);

	cc_mapNode_t* node;
	node = cc_mapNode_new(self, val, hash, idx, len, key);
	if(node == NULL)
	{
		return NULL;
	}

	// insert/append the node
	int replace = 0;
	cc_mapIter_t* miter;
	if(miter_at)
	{
		if((self->buckets[idx] == NULL) ||
		   (self->buckets[idx] == miter_at))
		{
			replace = 1;
		}

		miter = cc_list_insert(self->nodes, miter_at,
		                       (const void*) node);
	}
	else
	{
		if(self->buckets[idx] == NULL)
		{
			replace = 1;
		}

		miter = cc_list_append(self->nodes, NULL,
		                       (const void*) node);
	}

	// check insert/append
	if(miter == NULL)
	{
		goto fail_at;
	}

	// update bucket
	if(replace)
	{
		self->buckets[idx] = miter;
	}

	cc_map_grow(self);

	// success
	return miter;

	// failure
	fail_at:
		cc_mapNode_delete(&node, self);
	return NULL;
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
		cc_list_delete(&self->nodes);
		if(self->flags & CC_MAP_FLAG_CMALLOC)
		{
			free(self->buckets);
			free(self);
		}
		else
		{
			FREE(self->buckets);
			FREE(self);
		}
		*_self = NULL;
	}
}

void cc_map_discard(cc_map_t* self)
{
	ASSERT(self);

	size_t size = self->capacity*sizeof(cc_listIter_t*);
	memset((void*) self->buckets, 0, size);

	cc_mapNode_t* node;
	cc_listIter_t* iter = cc_list_head(self->nodes);
	while(iter)
	{
		node = (cc_mapNode_t*)
		       cc_list_remove(self->nodes, &iter);
		cc_mapNode_delete(&node, self);
	}
}

int cc_map_size(const cc_map_t* self)
{
	ASSERT(self);

	return cc_list_size(self->nodes);
}

size_t cc_map_sizeof(const cc_map_t* self)
{
	ASSERT(self);

	// sizeof map + nodes + buckets + list
	size_t size = sizeof(cc_map_t);
	size += self->nodes_size;
	size += self->capacity*sizeof(cc_listIter_t*);
	size += cc_list_sizeof(self->nodes);
	return size;
}

cc_mapIter_t*
cc_map_head(const cc_map_t* self)
{
	ASSERT(self);

	return cc_list_head(self->nodes);
}

cc_mapIter_t* cc_map_next(cc_mapIter_t* miter)
{
	ASSERT(miter);

	return cc_list_next(miter);
}

const void* cc_map_key(const cc_mapIter_t* miter, int* _len)
{
	ASSERT(miter);

	cc_mapNode_t* node;
	node = (cc_mapNode_t*) cc_list_peekIter(miter);

	uint8_t* key = cc_mapNode_key(node);

	*_len = node->len;

	return (const void*) key;
}

const void* cc_map_val(const cc_mapIter_t* miter)
{
	ASSERT(miter);

	cc_mapNode_t* node;
	node = (cc_mapNode_t*) cc_list_peekIter(miter);
	return node->val;
}

cc_mapIter_t*
cc_map_findp(const cc_map_t* self, int len, const void* key)
{
	ASSERT(self);
	ASSERT(key);

	// 8-byte aligned temp buffer (if needed)
	uint64_t key64[CC_MAP_KEYLEN/8];

	const uint8_t* key8 = (const uint8_t*) key;
	if(len > CC_MAP_KEYLEN)
	{
		return NULL;
	}
	else if(len == 0)
	{
		// pointer itself is the key
		len  = sizeof(void*);
		key8 = (uint8_t*) &key;
	}
	else if((((uintptr_t) key) % 8) != 0)
	{
		// force 8-byte alignment
		memcpy((void*) key64, key, len);
		key8 = (uint8_t*) key64;
	}

	uint32_t seed = self->seed;
	uint32_t hash = cc_mumurhash3(seed, len, key8);
	int      idx  = CC_MAP_IDX(self, hash);

	cc_mapIter_t* miter = self->buckets[idx];
	while(miter)
	{
		cc_mapNode_t* node;
		node = (cc_mapNode_t*)
		       cc_list_peekIter(miter);
		if(CC_MAP_IDX(self, node->hash) != idx)
		{
			return NULL;
		}

		int cmp = cc_mapNode_cmp(node, hash, len, key8);
		if(cmp == 0)
		{
			return miter;
		}
		else if(cmp > 0)
		{
			return NULL;
		}

		miter = cc_list_next(miter);
	}

	return NULL;
}

cc_mapIter_t*
cc_map_find(const cc_map_t* self, const char* key)
{
	ASSERT(self);
	ASSERT(key);

	int len = strlen(key) + 1;
	return cc_map_findp(self, len, (const void*) key);
}

cc_mapIter_t*
cc_map_findf(const cc_map_t* self, const char* fmt, ...)
{
	ASSERT(self);
	ASSERT(fmt);

	char key[CC_MAP_KEYLEN];
	va_list argptr;
	va_start(argptr, fmt);
	vsnprintf(key, CC_MAP_KEYLEN, fmt, argptr);
	va_end(argptr);

	int len = strlen(key) + 1;
	return cc_map_findp(self, len, (const void*) key);
}

cc_mapIter_t*
cc_map_addp(cc_map_t* self, const void* val,
            int len, const void* key)
{
	ASSERT(self);
	ASSERT(val);
	ASSERT(key);

	// 8-byte aligned temp buffer (if needed)
	uint64_t key64[CC_MAP_KEYLEN/8];

	const uint8_t* key8 = (const uint8_t*) key;
	if(len > CC_MAP_KEYLEN)
	{
		return NULL;
	}
	else if(len == 0)
	{
		// pointer itself is the key
		len  = sizeof(void*);
		key8 = (uint8_t*) &key;
	}
	else if((((uintptr_t) key) % 8) != 0)
	{
		// force 8-byte alignment
		memcpy((void*) key64, key, len);
		key8 = (uint8_t*) key64;
	}

	uint32_t seed = self->seed;
	uint32_t hash = cc_mumurhash3(seed, len, key8);
	int      idx  = CC_MAP_IDX(self, hash);

	// add node to existing bucket
	cc_mapIter_t* miter = self->buckets[idx];
	if(miter)
	{
		while(miter)
		{
			cc_mapNode_t* node;
			node = (cc_mapNode_t*) cc_list_peekIter(miter);
			if(CC_MAP_IDX(self, node->hash) != idx)
			{
				return cc_map_addAt(self, miter, hash, idx,
				                    val, len, key8);
			}

			int cmp = cc_mapNode_cmp(node, hash, len, key8);
			if(cmp == 0)
			{
				return NULL;
			}
			else if(cmp > 0)
			{
				return cc_map_addAt(self, miter, hash, idx,
				                    val, len, key8);
			}

			miter = cc_list_next(miter);
		}

		return cc_map_addAt(self, miter, hash, idx,
		                    val, len, key8);
	}

	// add node to an empty bucket
	// find insert position from next used bucket
	int i;
	for(i = idx + 1; i < self->capacity; ++i)
	{
		miter = self->buckets[i];
		if(miter)
		{
			return cc_map_addAt(self, miter, hash, idx,
			                    val, len, key8);
		}
	}

	miter = NULL;
	return cc_map_addAt(self, miter, hash, idx,
	                    val, len, key8);
}

cc_mapIter_t*
cc_map_add(cc_map_t* self,
           const void* val, const char* key)
{
	ASSERT(self);
	ASSERT(val);
	ASSERT(key);

	int len = strlen(key) + 1;
	return cc_map_addp(self, val, len, (const void*) key);
}

cc_mapIter_t*
cc_map_addf(cc_map_t* self,
            const void* val, const char* fmt, ...)
{
	ASSERT(self);
	ASSERT(val);
	ASSERT(fmt);

	char key[CC_MAP_KEYLEN];
	va_list argptr;
	va_start(argptr, fmt);
	vsnprintf(key, CC_MAP_KEYLEN, fmt, argptr);
	va_end(argptr);

	int len = strlen(key) + 1;
	return cc_map_addp(self, val, len, (const void*) key);
}

const void*
cc_map_remove(cc_map_t* self, cc_mapIter_t** _miter)
{
	ASSERT(self);
	ASSERT(_miter);
	ASSERT(*_miter);

	cc_mapIter_t* miter;
	cc_mapNode_t* node;
	const void*   val;

	miter = *_miter;
	node  = (cc_mapNode_t*)
	        cc_list_peekIter(miter);
	val   = node->val;

	// update bucket pointer
	int idx = CC_MAP_IDX(self, node->hash);
	if(self->buckets[idx] == miter)
	{
		cc_listIter_t* next = cc_list_next(miter);
		if(next)
		{
			// bucket index must match
			cc_mapNode_t* tmp;
			tmp = (cc_mapNode_t*)
			      cc_list_peekIter(next);
			if(CC_MAP_IDX(self, tmp->hash) != idx)
			{
				next = NULL;
			}
		}
		self->buckets[idx] = next;
	}

	// update nodes/miter
	cc_list_remove(self->nodes, _miter);
	cc_mapNode_delete(&node, self);

	return val;
}
