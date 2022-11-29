/*
 * Copyright (c) 2010 Jeff Boody
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

#ifndef cc_workq_H
#define cc_workq_H

#include <pthread.h>

#include "cc_list.h"
#include "cc_map.h"

// workq status
#define CC_WORKQ_STATUS_ERROR    0
#define CC_WORKQ_STATUS_PENDING  1
#define CC_WORKQ_STATUS_ACTIVE   2
#define CC_WORKQ_STATUS_COMPLETE 3
#define CC_WORKQ_STATUS_FAILURE  4

#define CC_WORKQ_THREAD_PRIORITY_DEFAULT 0
#define CC_WORKQ_THREAD_PRIORITY_HIGH    1

// called from the workq thread
typedef int (*cc_workqRun_fn)(int tid,
                              void* owner,
                              void* task);

// called from main thread by
// reset, purge, flush, finish or delete
typedef void (*cc_workqFinish_fn)(void* owner,
                                  void* task,
                                  int status);

typedef struct
{
	int   status;
	int   priority;
	int   purge_id;
	void* task;
} cc_workqNode_t;

typedef struct
{
	// queue state
	int   state;
	void* owner;
	int   purge_id;

	// maps from task to listIter
	cc_map_t* map_task;

	// queues
	cc_list_t* queue_pending;
	cc_list_t* queue_complete;
	cc_list_t* queue_active;

	// callbacks
	cc_workqRun_fn    run_fn;
	cc_workqFinish_fn finish_fn;

	// workq thread(s)
	int             thread_count;
	int             thread_priority;
	pthread_t*      threads;
	int             next_tid;
	pthread_mutex_t mutex;
	pthread_cond_t  cond_pending;
	pthread_cond_t  cond_complete;
} cc_workq_t;

cc_workq_t* cc_workq_new(void* owner, int thread_count,
                         int thread_priority,
                         cc_workqRun_fn run_fn,
                         cc_workqFinish_fn finish_fn);
void        cc_workq_delete(cc_workq_t** _self);
void        cc_workq_reset(cc_workq_t* self, int blocking);
void        cc_workq_purge(cc_workq_t* self);
void        cc_workq_flush(cc_workq_t* self);
void        cc_workq_finish(cc_workq_t* self);
int         cc_workq_run(cc_workq_t* self, void* task,
                         int priority);
int         cc_workq_wait(cc_workq_t* self, void* task,
                          int blocking);
int         cc_workq_cancel(cc_workq_t* self, void* task,
                            int blocking);
int         cc_workq_status(cc_workq_t* self, void* task);
int         cc_workq_pending(cc_workq_t* self);

#endif
