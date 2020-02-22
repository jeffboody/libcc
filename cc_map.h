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

#define CC_MAP_KEYLEN 256

typedef struct cc_mapNode_s
{
	struct cc_mapNode_s* prev;
	struct cc_mapNode_s* next;
	struct cc_mapNode_s* down;

	const void* val;
	char k;
} cc_mapNode_t;

typedef struct
{
	int  depth;
	char key[CC_MAP_KEYLEN];
	cc_mapNode_t* node[CC_MAP_KEYLEN];
} cc_mapIter_t;

typedef struct
{
	int flags;
	int size;
	int nodes;
	cc_mapNode_t* head;
} cc_map_t;

cc_map_t*     cc_map_new(void);
void          cc_map_delete(cc_map_t** _self);
void          cc_map_discard(cc_map_t* self);
int           cc_map_size(const cc_map_t* self);
size_t        cc_map_sizeof(const cc_map_t* self);
cc_mapIter_t* cc_map_head(const cc_map_t* self,
                          cc_mapIter_t* iter);
cc_mapIter_t* cc_map_next(cc_mapIter_t* iter);
const void*   cc_map_val(const cc_mapIter_t* iter);
const char*   cc_map_key(const cc_mapIter_t* iter);
const void*   cc_map_find(const cc_map_t* self,
                          cc_mapIter_t* iter,
                          const char* key);
const void*   cc_map_findf(const cc_map_t* self,
                           cc_mapIter_t* iter,
                           const char* fmt, ...);
int           cc_map_add(cc_map_t* self,
                         const void* val,
                         const char* key);
int           cc_map_addf(cc_map_t* self,
                          const void* val,
                          const char* fmt, ...);
const void*   cc_map_replace(cc_mapIter_t* iter,
                             const void* val);
const void*   cc_map_remove(cc_map_t* self,
                            cc_mapIter_t** _iter);

#endif
