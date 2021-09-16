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

#ifndef cc_map_H
#define cc_map_H

#include <inttypes.h>

#include "cc_list.h"

typedef cc_listIter_t cc_mapIter_t;

typedef struct
{
	int      flags;
	uint32_t seed;

	// buckets
	int             capacity;
	int             elements;
	cc_listIter_t** buckets;

	// nodes
	size_t     nodes_size;
	cc_list_t* nodes;
} cc_map_t;

cc_map_t*     cc_map_new(void);
void          cc_map_delete(cc_map_t** _self);
void          cc_map_discard(cc_map_t* self);
int           cc_map_size(const cc_map_t* self);
size_t        cc_map_sizeof(const cc_map_t* self);
cc_mapIter_t* cc_map_head(const cc_map_t* self);
cc_mapIter_t* cc_map_next(cc_mapIter_t* miter);
const void*   cc_map_key(const cc_mapIter_t* miter,
                         int* _len);
const void*   cc_map_val(const cc_mapIter_t* miter);
cc_mapIter_t* cc_map_findp(const cc_map_t* self,
                           int len,
                           const void* key);
cc_mapIter_t* cc_map_find(const cc_map_t* self,
                         const char* key);
cc_mapIter_t* cc_map_findf(const cc_map_t* self,
                           const char* fmt, ...);
cc_mapIter_t* cc_map_addp(cc_map_t* self,
                          const void* val,
                          int len,
                          const void* key);
cc_mapIter_t* cc_map_add(cc_map_t* self,
                         const void* val,
                         const char* key);
cc_mapIter_t* cc_map_addf(cc_map_t* self,
                          const void* val,
                          const char* fmt, ...);
const void*   cc_map_remove(cc_map_t* self,
                            cc_mapIter_t** _miter);

#endif
