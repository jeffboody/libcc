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

#ifndef cc_jobq_H
#define cc_jobq_H

#include <pthread.h>

#include "cc_list.h"
#include "cc_map.h"

// jobq status
#define CC_JOBQ_STATUS_ERROR    0
#define CC_JOBQ_STATUS_PENDING  1
#define CC_JOBQ_STATUS_ACTIVE   2
#define CC_JOBQ_STATUS_COMPLETE 3
#define CC_JOBQ_STATUS_FAILURE  4

// called from the jobq thread
typedef int (*cc_jobqRun_fn)(int tid,
                             void* owner,
                             void* task);

// called from main thread by
// reset, purge, flush, finish or delete
typedef void (*cc_jobqFinish_fn)(void* owner,
                                 void* task,
                                 int status);

typedef struct
{
	int   status;
	int   priority;
	int   purge_id;
	void* task;
} cc_jobqNode_t;

typedef struct
{
	// queue state
	int   state;
	void* owner;
	int   purge_id;

	// queues
	cc_list_t* queue_pending;
	cc_list_t* queue_complete;
	cc_list_t* queue_active;

	// callbacks
	cc_jobqRun_fn    run_fn;
	cc_jobqFinish_fn finish_fn;

	// jobq thread(s)
	int             thread_count;
	pthread_t*      threads;
	int             next_tid;
	pthread_mutex_t mutex;
	pthread_cond_t  cond_pending;
	pthread_cond_t  cond_complete;
} cc_jobq_t;

cc_jobq_t* cc_jobq_new(void* owner, int thread_count,
                       cc_jobqRun_fn run_fn,
                       cc_jobqFinish_fn finish_fn);
void        cc_jobq_delete(cc_jobq_t** _self);
void        cc_jobq_flush(cc_jobq_t* self);
void        cc_jobq_finish(cc_jobq_t* self);
int         cc_jobq_run(cc_jobq_t* self, void* task,
                        int priority);
int         cc_jobq_pending(cc_jobq_t* self);

#endif
