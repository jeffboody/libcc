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

#include <sys/resource.h>
#include <stdlib.h>
#include <unistd.h>

#define LOG_TAG "cc"
#include "cc_log.h"
#include "cc_memory.h"
#include "cc_jobq.h"

/***********************************************************
* private                                                  *
***********************************************************/

// jobq run state
const int CC_JOBQ_STATE_RUNNING = 0;
const int CC_JOBQ_STATE_PAUSED  = 1;
const int CC_JOBQ_STATE_STOP    = 2;

static void* cc_jobq_thread(void* arg)
{
	ASSERT(arg);

	cc_jobq_t* self = (cc_jobq_t*) arg;

	// override the thread priority
	if(self->thread_priority == CC_JOBQ_THREAD_PRIORITY_DEFAULT)
	{
		int priority = getpriority(PRIO_PROCESS, 0);
		setpriority(PRIO_PROCESS, 0, priority+5);
	}

	pthread_mutex_lock(&self->mutex);

	// checkout the next available thread id
	int tid  = self->next_tid++;
	while(1)
	{
		// pending for an event
		while((self->state == CC_JOBQ_STATE_PAUSED) ||
		      ((cc_list_size(self->queue_pending) == 0) &&
		       (self->state == CC_JOBQ_STATE_RUNNING)))
		{
			pthread_cond_wait(&self->cond_pending,
			                  &self->mutex);
		}

		if(self->state == CC_JOBQ_STATE_STOP)
		{
			// stop condition
			pthread_mutex_unlock(&self->mutex);
			return NULL;
		}

		// move the task to the active queue
		cc_listIter_t* iter;
		void* task;
		iter = cc_list_head(self->queue_pending);
		task = (void*) cc_list_peekIter(iter);
		cc_list_swapn(self->queue_pending,
		              self->queue_active, iter, NULL);

		pthread_mutex_unlock(&self->mutex);

		// run the task
		(*self->run_fn)(tid, self->owner, task);

		pthread_mutex_lock(&self->mutex);

		// complete the task
		cc_list_remove(self->queue_active, &iter);

		// broadcast when all tasks are complete
		if((cc_list_size(self->queue_pending) == 0) &&
		   (cc_list_size(self->queue_active) == 0))
		{
			pthread_cond_broadcast(&self->cond_complete);
		}
	}
}

/***********************************************************
* public                                                   *
***********************************************************/

cc_jobq_t*
cc_jobq_new(void* owner, int thread_count,
            int thread_priority,
            cc_jobqRun_fn run_fn)
{
	// owner may be NULL
	ASSERT(run_fn);

	cc_jobq_t* self;
	self = (cc_jobq_t*) CALLOC(1, sizeof(cc_jobq_t));
	if(!self)
	{
		LOGE("CALLOC failed");
		return NULL;
	}

	self->state           = CC_JOBQ_STATE_RUNNING;
	self->owner           = owner;
	self->thread_count    = thread_count;
	self->thread_priority = thread_priority;
	self->next_tid        = 0;
	self->run_fn          = run_fn;

	// PTHREAD_MUTEX_DEFAULT is not re-entrant
	if(pthread_mutex_init(&self->mutex, NULL) != 0)
	{
		LOGE("pthread_mutex_init failed");
		goto fail_mutex_init;
	}

	if(pthread_cond_init(&self->cond_pending, NULL) != 0)
	{
		LOGE("pthread_cond_init failed");
		goto fail_cond_pending;
	}

	if(pthread_cond_init(&self->cond_complete, NULL) != 0)
	{
		LOGE("pthread_cond_init failed");
		goto fail_cond_complete;
	}

	self->queue_pending = cc_list_new();
	if(self->queue_pending == NULL)
	{
		goto fail_queue_pending;
	}

	self->queue_active = cc_list_new();
	if(self->queue_active == NULL)
	{
		goto fail_queue_active;
	}

	// alloc threads
	self->threads = (pthread_t*)
	                CALLOC(thread_count, sizeof(pthread_t));
	if(self->threads == NULL)
	{
		LOGE("CALLOC failed");
		goto fail_threads;
	}

	// create threads
	pthread_mutex_lock(&self->mutex);
	int i;
	for(i = 0; i < thread_count; ++i)
	{
		if(pthread_create(&(self->threads[i]), NULL,
		                  cc_jobq_thread,
		                  (void*) self) != 0)
		{
			LOGE("pthread_create failed");
			goto fail_pthread_create;
		}
	}
	pthread_mutex_unlock(&self->mutex);

	// success
	return self;

	// fail
	fail_pthread_create:
		self->state = CC_JOBQ_STATE_STOP;
		pthread_mutex_unlock(&self->mutex);

		int j;
		for(j = 0; j < i; ++j)
		{
			pthread_join(self->threads[j], NULL);
		}
		FREE(self->threads);
	fail_threads:
		cc_list_delete(&self->queue_active);
	fail_queue_active:
		cc_list_delete(&self->queue_pending);
	fail_queue_pending:
		pthread_cond_destroy(&self->cond_complete);
	fail_cond_complete:
		pthread_cond_destroy(&self->cond_pending);
	fail_cond_pending:
		pthread_mutex_destroy(&self->mutex);
	fail_mutex_init:
		FREE(self);
	return NULL;
}

void cc_jobq_delete(cc_jobq_t** _self)
{
	// *_self can be null
	ASSERT(_self);

	cc_jobq_t* self = *_self;
	if(self)
	{
		cc_jobq_finish(self);

		pthread_mutex_lock(&self->mutex);

		// stop the jobq thread
		self->state = CC_JOBQ_STATE_STOP;
		pthread_cond_broadcast(&self->cond_pending);
		pthread_mutex_unlock(&self->mutex);
		int i;
		for(i = 0; i < self->thread_count; ++i)
		{
			pthread_join(self->threads[i], NULL);
		}
		FREE(self->threads);

		// destroy the queues
		cc_list_delete(&self->queue_active);
		cc_list_delete(&self->queue_pending);

		// destroy the thread state
		pthread_cond_destroy(&self->cond_complete);
		pthread_cond_destroy(&self->cond_pending);
		pthread_mutex_destroy(&self->mutex);

		FREE(self);
		*_self = NULL;
	}
}

void cc_jobq_pause(cc_jobq_t* self)
{
	ASSERT(self);

	pthread_mutex_lock(&self->mutex);
	self->state = CC_JOBQ_STATE_PAUSED;
	pthread_mutex_unlock(&self->mutex);
}

void cc_jobq_resume(cc_jobq_t* self)
{
	ASSERT(self);

	pthread_mutex_lock(&self->mutex);
	self->state = CC_JOBQ_STATE_RUNNING;
	pthread_cond_broadcast(&self->cond_pending);
	pthread_mutex_unlock(&self->mutex);
}

void cc_jobq_finish(cc_jobq_t* self)
{
	ASSERT(self);

	pthread_mutex_lock(&self->mutex);

	// resume jobq if needed
	if(self->state == CC_JOBQ_STATE_PAUSED)
	{
		self->state = CC_JOBQ_STATE_RUNNING;
		pthread_cond_broadcast(&self->cond_pending);
	}

	// wait for pending/active tasks to complete
	while(cc_list_size(self->queue_pending) ||
		  cc_list_size(self->queue_active))
	{
		pthread_cond_wait(&self->cond_complete,
		                  &self->mutex);
	}

	pthread_mutex_unlock(&self->mutex);
}

int cc_jobq_run(cc_jobq_t* self, void* task)
{
	ASSERT(self);
	ASSERT(task);

	pthread_mutex_lock(&self->mutex);

	cc_listIter_t* iter;
	iter = cc_list_append(self->queue_pending, NULL,
	                      (const void*) task);
	if(iter == NULL)
	{
		pthread_mutex_unlock(&self->mutex);
		return 0;
	}

	// wake up jobq thread
	if(self->state == CC_JOBQ_STATE_RUNNING)
	{
		pthread_cond_broadcast(&self->cond_pending);
	}

	pthread_mutex_unlock(&self->mutex);

	return 1;
}

int cc_jobq_pending(cc_jobq_t* self)
{
	ASSERT(self);

	int size;
	pthread_mutex_lock(&self->mutex);
	size = cc_list_size(self->queue_pending);
	size += cc_list_size(self->queue_active);
	pthread_mutex_unlock(&self->mutex);
	return size;
}
