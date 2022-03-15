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

#ifndef cc_multimap_H
#define cc_multimap_H

#include "cc_list.h"
#include "cc_map.h"

typedef struct
{
	cc_mapIter_t*  miter;
	cc_listIter_t* iter;
} cc_multimapIter_t;

typedef struct
{
	cc_map_t*     map;
	cc_listcmp_fn compare;
} cc_multimap_t;

cc_multimap_t*     cc_multimap_new(cc_listcmp_fn compare);
void               cc_multimap_delete(cc_multimap_t** _self);
void               cc_multimap_discard(cc_multimap_t* self);
int                cc_multimap_size(const cc_multimap_t* self);
size_t             cc_multimap_sizeof(const cc_multimap_t* self);
cc_multimapIter_t* cc_multimap_head(const cc_multimap_t* self,
                                    cc_multimapIter_t* mmiter);
cc_multimapIter_t* cc_multimap_next(cc_multimapIter_t* mmiter);
cc_multimapIter_t* cc_multimap_nextItem(cc_multimapIter_t* mmiter);
cc_multimapIter_t* cc_multimap_nextList(cc_multimapIter_t* mmiter);
const void*        cc_multimap_key(const cc_multimapIter_t* mmiter,
                                   int* _len);
const void*        cc_multimap_val(const cc_multimapIter_t* mmiter);
const cc_list_t*   cc_multimap_list(const cc_multimapIter_t* mmiter);
const cc_list_t*   cc_multimap_findp(const cc_multimap_t* self,
                                     cc_multimapIter_t* mmiter,
                                     int len,
                                     const void* key);
const cc_list_t*   cc_multimap_find(const cc_multimap_t* self,
                                    cc_multimapIter_t* mmiter,
                                    const char* key);
const cc_list_t*   cc_multimap_findf(const cc_multimap_t* self,
                                     cc_multimapIter_t* mmiter,
                                     const char* fmt, ...);
int                cc_multimap_addp(cc_multimap_t* self,
                                    const void* val,
                                    int len,
                                    const void* key);
int                cc_multimap_add(cc_multimap_t* self,
                                   const void* val,
                                   const char* key);
int                cc_multimap_addf(cc_multimap_t* self,
                                    const void* val,
                                    const char* fmt, ...);
const void*        cc_multimap_remove(cc_multimap_t* self,
                                      cc_multimapIter_t** _mmiter);

#endif
