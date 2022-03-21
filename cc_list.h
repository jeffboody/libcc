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

#ifndef cc_list_H
#define cc_list_H

typedef int (*cc_listcmp_fn)(const void* a, const void* b);

typedef struct cc_listIter_s
{
	struct cc_listIter_s* next;
	struct cc_listIter_s* prev;
	const void*           data;
} cc_listIter_t;

typedef struct
{
	int            flags;
	int            size;
	cc_listIter_t* head;
	cc_listIter_t* tail;
	cc_listIter_t* iters;
} cc_list_t;

cc_list_t*     cc_list_new(void);
void           cc_list_delete(cc_list_t** _self);
void           cc_list_discard(cc_list_t* self);
int            cc_list_size(const cc_list_t* self);
size_t         cc_list_sizeof(const cc_list_t* self);
const void*    cc_list_peekHead(const cc_list_t* self);
const void*    cc_list_peekTail(const cc_list_t* self);
const void*    cc_list_peekIter(const cc_listIter_t* iter);
const void*    cc_list_peekIndex(const cc_list_t* self,
                                 int idx);
cc_listIter_t* cc_list_head(const cc_list_t* self);
cc_listIter_t* cc_list_tail(const cc_list_t* self);
cc_listIter_t* cc_list_next(cc_listIter_t* iter);
cc_listIter_t* cc_list_prev(cc_listIter_t* iter);
cc_listIter_t* cc_list_get(cc_list_t* self, int idx);
cc_listIter_t* cc_list_find(const cc_list_t* self,
                            const void* data,
                            cc_listcmp_fn compare);
cc_listIter_t* cc_list_findSorted(const cc_list_t* self,
                                  const void* data,
                                  cc_listcmp_fn compare);
cc_listIter_t* cc_list_insertSorted(cc_list_t* self,
                                    cc_listcmp_fn compare,
                                    const void* data);
cc_listIter_t* cc_list_insert(cc_list_t* self,
                              cc_listIter_t* iter,
                              const void* data);
cc_listIter_t* cc_list_append(cc_list_t* self,
                              cc_listIter_t* iter,
                              const void* data);
const void*    cc_list_replace(cc_listIter_t* iter,
                               const void* data);
const void*    cc_list_remove(cc_list_t* self,
                              cc_listIter_t** _iter);
void           cc_list_move(cc_list_t* self,
                            cc_listIter_t* from,
                            cc_listIter_t* to);
void           cc_list_moven(cc_list_t* self,
                             cc_listIter_t* from,
                             cc_listIter_t* to);
void           cc_list_swap(cc_list_t* fromList,
                            cc_list_t* toList,
                            cc_listIter_t* from,
                            cc_listIter_t* to);
void           cc_list_swapn(cc_list_t* fromList,
                             cc_list_t* toList,
                             cc_listIter_t* from,
                             cc_listIter_t* to);
void           cc_list_appendList(cc_list_t* self,
                                  cc_list_t* from);
void           cc_list_insertList(cc_list_t* self,
                                  cc_list_t* from);

#endif
