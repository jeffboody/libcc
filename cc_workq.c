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

#include <assert.h>
#include <stdlib.h>
#include <unistd.h>

#define LOG_TAG "cc"
#include "cc_log.h"
#include "cc_memory.h"
#include "cc_workq.h"

/***********************************************************
* private                                                  *
***********************************************************/

// workq run state
const int CC_WORKQ_STATE_RUNNING = 0;
const int CC_WORKQ_STATE_STOP    = 1;

// force purge a task or workq
const int CC_WORKQ_PURGE = -1;

static cc_workqNode_t*
cc_workqNode_new(void* task, int purge_id, int priority)
{
	assert(task);
	assert((purge_id == 0) || (purge_id == 1));

	cc_workqNode_t* self;
	self = (cc_workqNode_t*) MALLOC(sizeof(cc_workqNode_t));
	if(!self)
	{
		LOGE("MALLOC failed");
		return NULL;
	}

	self->status   = CC_WORKQ_STATUS_PENDING;
	self->priority = priority;
	self->purge_id = purge_id;
	self->task     = task;

	return self;
}

static void cc_workqNode_delete(cc_workqNode_t** _self)
{
	assert(_self);

	cc_workqNode_t* self = *_self;
	if(self)
	{
		FREE(self);
		*_self = NULL;
	}
}

static int cc_taskcmp_fn(const void *a, const void *b)
{
	assert(a);
	assert(b);

	cc_workqNode_t* node = (cc_workqNode_t*) a;
	return (node->task == b) ? 0 : 1;
}

static void* cc_workq_thread(void* arg)
{
	assert(arg);

	cc_workq_t* self = (cc_workq_t*) arg;
	pthread_mutex_lock(&self->mutex);

	// checkout the next available thread id
	int tid  = self->next_tid++;
	while(1)
	{
		// pending for an event
		while((cc_list_size(self->queue_pending) == 0) &&
		      (self->state == CC_WORKQ_STATE_RUNNING))
		{
			pthread_cond_wait(&self->cond_pending,
			                  &self->mutex);
		}

		if(self->state == CC_WORKQ_STATE_STOP)
		{
			// stop condition
			pthread_mutex_unlock(&self->mutex);
			return NULL;
		}

		// get the task
		cc_listIter_t*  iter;
		cc_workqNode_t* node;
		iter = cc_list_head(self->queue_pending);
		node = (cc_workqNode_t*) cc_list_peekIter(iter);
		cc_list_swapn(self->queue_pending,
		              self->queue_active, iter, NULL);
		node->status = CC_WORKQ_STATUS_ACTIVE;

		// wake another thread
		// allows signal instead of broadcast for
		// cond_pending
		if(cc_list_size(self->queue_pending) > 0)
		{
			pthread_cond_signal(&self->cond_pending);
		}

		pthread_mutex_unlock(&self->mutex);

		// run the task
		int ret = (*self->run_fn)(tid, self->owner,
		                          node->task);

		pthread_mutex_lock(&self->mutex);

		// put the task on the complete queue
		node->status = ret ? CC_WORKQ_STATUS_COMPLETE :
		                     CC_WORKQ_STATUS_FAILURE;
		cc_list_swapn(self->queue_active,
		              self->queue_complete, iter, NULL);

		// signal anybody pending for the workq to become
		// idle
		pthread_cond_signal(&self->cond_complete);
	}
}
static void cc_workq_flushLocked(cc_workq_t* self)
{
	assert(self);

	cc_listIter_t* iter;
	iter = cc_list_head(self->queue_complete);
	while(iter)
	{
		cc_workqNode_t* node;
		node = (cc_workqNode_t*)
		       cc_list_remove(self->queue_complete, &iter);
		(*self->finish_fn)(self->owner, node->task,
		                   node->status);
		cc_workqNode_delete(&node);
	}
}

/***********************************************************
* public                                                   *
***********************************************************/

cc_workq_t*
cc_workq_new(void* owner, int thread_count,
             cc_workqRun_fn run_fn,
             cc_workqFinish_fn finish_fn)
{
	// owner may be NULL
	assert(run_fn);
	assert(finish_fn);

	cc_workq_t* self;
	self = (cc_workq_t*) MALLOC(sizeof(cc_workq_t));
	if(!self)
	{
		LOGE("MALLOC failed");
		return NULL;
	}

	self->state        = CC_WORKQ_STATE_RUNNING;
	self->owner        = owner;
	self->purge_id     = 0;
	self->thread_count = thread_count;
	self->next_tid     = 0;
	self->run_fn       = run_fn;
	self->finish_fn    = finish_fn;

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

	self->queue_complete = cc_list_new();
	if(self->queue_complete == NULL)
	{
		goto fail_queue_complete;
	}

	self->queue_active = cc_list_new();
	if(self->queue_active == NULL)
	{
		goto fail_queue_active;
	}

	// alloc threads
	int sz = thread_count*sizeof(pthread_t);
	self->threads = (pthread_t*) MALLOC(sz);
	if(self->threads == NULL)
	{
		LOGE("MALLOC failed");
		goto fail_threads;
	}

	// create threads
	pthread_mutex_lock(&self->mutex);
	int i;
	for(i = 0; i < thread_count; ++i)
	{
		if(pthread_create(&(self->threads[i]), NULL,
		                  cc_workq_thread,
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
		self->state = CC_WORKQ_STATE_STOP;
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
		cc_list_delete(&self->queue_complete);
	fail_queue_complete:
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

void cc_workq_delete(cc_workq_t** _self)
{
	// *_self can be null
	assert(_self);

	cc_workq_t* self = *_self;
	if(self)
	{
		pthread_mutex_lock(&self->mutex);

		// stop the workq thread
		self->state = CC_WORKQ_STATE_STOP;
		pthread_cond_broadcast(&self->cond_pending);
		pthread_mutex_unlock(&self->mutex);
		int i;
		for(i = 0; i < self->thread_count; ++i)
		{
			pthread_join(self->threads[i], NULL);
		}
		FREE(self->threads);

		// destroy the queues
		// queue_active will be empty since the threads are
		// stopped
		self->purge_id = CC_WORKQ_PURGE;
		cc_workq_purge(self);
		cc_list_delete(&self->queue_active);
		cc_list_delete(&self->queue_complete);
		cc_list_delete(&self->queue_pending);

		// destroy the thread state
		pthread_cond_destroy(&self->cond_complete);
		pthread_cond_destroy(&self->cond_pending);
		pthread_mutex_destroy(&self->mutex);

		FREE(self);
		*_self = NULL;
	}
}

void cc_workq_reset(cc_workq_t* self, int blocking)
{
	assert(self);

	// save the purge_id
	int purge_id = self->purge_id;

	// purge the pending queue so no new
	// tasks are submitted to active queue
	self->purge_id = CC_WORKQ_PURGE;
	cc_workq_purge(self);

	if(blocking)
	{
		// blocking wait for the active queue
		pthread_mutex_lock(&self->mutex);
		while(cc_list_size(self->queue_active) > 0)
		{
			// must wait for active task to complete
			pthread_cond_wait(&self->cond_complete,
			                  &self->mutex);
		}
		pthread_mutex_unlock(&self->mutex);

		// purge the complete queue
		cc_workq_purge(self);
	}

	// restore the purge_id
	self->purge_id = purge_id;
}

void cc_workq_purge(cc_workq_t* self)
{
	assert(self);

	pthread_mutex_lock(&self->mutex);

	// purge the pending queue
	cc_listIter_t* iter;
	iter = cc_list_head(self->queue_pending);
	while(iter)
	{
		cc_workqNode_t* node;
		node = (cc_workqNode_t*) cc_list_peekIter(iter);
		if(node->purge_id != self->purge_id)
		{
			cc_list_remove(self->queue_pending, &iter);
			(*self->finish_fn)(self->owner, node->task,
			                   node->status);
			cc_workqNode_delete(&node);
		}
		else
		{
			iter = cc_list_next(iter);
		}
	}

	// purge the active queue (non-blocking)
	iter = cc_list_head(self->queue_active);
	while(iter)
	{
		cc_workqNode_t* node;
		node = (cc_workqNode_t*) cc_list_peekIter(iter);
		if(node->purge_id != self->purge_id)
		{
			node->purge_id = CC_WORKQ_PURGE;
		}
		iter = cc_list_next(iter);
	}

	// purge the complete queue
	iter = cc_list_head(self->queue_complete);
	while(iter)
	{
		cc_workqNode_t* node;
		node = (cc_workqNode_t*) cc_list_peekIter(iter);
		if((node->purge_id != self->purge_id) ||
		   (node->purge_id == CC_WORKQ_PURGE))
		{
			cc_list_remove(self->queue_complete, &iter);
			(*self->finish_fn)(self->owner, node->task,
			                   node->status);
			cc_workqNode_delete(&node);
		}
		else
		{
			iter = cc_list_next(iter);
		}
	}

	// swap the purge id
	if(self->purge_id != CC_WORKQ_PURGE)
	{
		self->purge_id = 1 - self->purge_id;
	}

	pthread_mutex_unlock(&self->mutex);
}

void cc_workq_flush(cc_workq_t* self)
{
	assert(self);

	pthread_mutex_lock(&self->mutex);
	cc_workq_flushLocked(self);
	pthread_mutex_unlock(&self->mutex);
}

void cc_workq_finish(cc_workq_t* self)
{
	assert(self);

	pthread_mutex_lock(&self->mutex);

	while(1)
	{
		if(cc_list_size(self->queue_complete))
		{
			// flush any tasks which have completed
			cc_workq_flushLocked(self);
		}

		if(cc_list_size(self->queue_pending) ||
		   cc_list_size(self->queue_active))
		{
			// wait for pending/active tasks to complete
			pthread_cond_wait(&self->cond_complete,
			                  &self->mutex);
		}
		else
		{
			break;
		}
	}

	pthread_mutex_unlock(&self->mutex);
}

int cc_workq_run(cc_workq_t* self, void* task,
                 int priority)
{
	assert(self);
	assert(task);

	pthread_mutex_lock(&self->mutex);

	// find the node containing the task or create a new one
	int status = CC_WORKQ_STATUS_ERROR;
	cc_listIter_t*  iter = NULL;
	cc_listIter_t*  pos  = NULL;
	cc_workqNode_t* tmp  = NULL;
	cc_workqNode_t* node = NULL;
	if((iter = cc_list_find(self->queue_complete, task,
	                        cc_taskcmp_fn)) != NULL)
	{
		// task completed
		node = (cc_workqNode_t*)
		       cc_list_remove(self->queue_complete, &iter);
		status = node->status;
		cc_workqNode_delete(&node);
	}
	else if((iter = cc_list_find(self->queue_active, task,
	                             cc_taskcmp_fn)) != NULL)
	{
		node = (cc_workqNode_t*) cc_list_peekIter(iter);
		node->purge_id = self->purge_id;
		status = node->status;
	}
	else if((iter = cc_list_find(self->queue_pending, task,
	                             cc_taskcmp_fn)) != NULL)
	{
		node = (cc_workqNode_t*) cc_list_peekIter(iter);
		node->purge_id = self->purge_id;
		if(priority > node->priority)
		{
			// move up
			pos = cc_list_prev(iter);
			while(pos)
			{
				tmp = (cc_workqNode_t*)
				      cc_list_peekIter(pos);
				if(tmp->priority >= node->priority)
				{
					break;
				}
				pos = cc_list_prev(pos);
			}

			if(pos)
			{
				// move after pos
				cc_list_moven(self->queue_pending, iter,
				              pos);
			}
			else
			{
				// move to head of list
				cc_list_move(self->queue_pending, iter,
				             NULL);
			}
		}
		else if(priority < node->priority)
		{
			// move down
			pos = cc_list_next(iter);
			while(pos)
			{
				tmp = (cc_workqNode_t*)
				      cc_list_peekIter(pos);
				if(tmp->priority < node->priority)
				{
					break;
				}
				pos = cc_list_next(pos);
			}

			if(pos)
			{
				// move before pos
				cc_list_move(self->queue_pending, iter,
				             pos);
			}
			else
			{
				// move to tail of list
				cc_list_moven(self->queue_pending, iter,
				              NULL);
			}
		}
		node->priority = priority;
		status = node->status;
	}
	else
	{
		// create new node
		node = cc_workqNode_new(task, self->purge_id,
		                        priority);
		if(node == NULL)
		{
			goto fail_node;
		}

		// find the insert position
		pos = cc_list_tail(self->queue_pending);
		while(pos)
		{
			tmp = (cc_workqNode_t*)
			      cc_list_peekIter(pos);
			if(tmp->priority >= node->priority)
			{
				break;
			}
			pos = cc_list_prev(pos);
		}

		if(pos)
		{
			// append after pos
			if(cc_list_append(self->queue_pending, pos,
			                  (const void*) node) == NULL)
			{
				goto fail_queue;
			}
		}
		else
		{
			// insert at head of queue
			// first item or highest priority
			if(cc_list_insert(self->queue_pending, NULL,
			                  (const void*) node) == NULL)
			{
				goto fail_queue;
			}
		}

		status = node->status;

		// wake up workq thread
		pthread_cond_signal(&self->cond_pending);
	}

	pthread_mutex_unlock(&self->mutex);

	// success
	return status;

	// failure
	fail_queue:
		cc_workqNode_delete(&node);
	fail_node:
		pthread_mutex_unlock(&self->mutex);
	return CC_WORKQ_STATUS_ERROR;
}

int cc_workq_cancel(cc_workq_t* self, void* task,
                    int blocking)
{
	assert(self);
	assert(task);

	int status = CC_WORKQ_STATUS_ERROR;
	pthread_mutex_lock(&self->mutex);

	cc_listIter_t* iter;
	if((iter = cc_list_find(self->queue_pending, task,
	                        cc_taskcmp_fn)) != NULL)
	{
		// cancel pending task
		cc_workqNode_t* node;
		node = (cc_workqNode_t*)
		       cc_list_remove(self->queue_pending, &iter);
		status = node->status;
		cc_workqNode_delete(&node);
	}
	else
	{
		while((iter = cc_list_find(self->queue_active, task,
		                           cc_taskcmp_fn)) != NULL)
		{
			if(blocking == 0)
			{
				cc_workqNode_t* node;
				node = (cc_workqNode_t*)
				       cc_list_peekIter(iter);
				status = node->status;
				pthread_mutex_unlock(&self->mutex);
				return status;
			}

			// must wait for active task to complete
			pthread_cond_wait(&self->cond_complete,
			                  &self->mutex);
		}

		if((iter = cc_list_find(self->queue_complete, task,
		                         cc_taskcmp_fn)) != NULL)
		{
			// cancel completed task
			cc_workqNode_t* node;
			node = (cc_workqNode_t*)
			       cc_list_remove(self->queue_complete,
			                      &iter);
			status = node->status;
			cc_workqNode_delete(&node);
		}
	}

	pthread_mutex_unlock(&self->mutex);
	return status;
}

int cc_workq_status(cc_workq_t* self, void* task)
{
	assert(self);
	assert(task);

	int status = CC_WORKQ_STATUS_ERROR;
	pthread_mutex_lock(&self->mutex);

	cc_listIter_t* iter;
	iter = cc_list_find(self->queue_pending, task,
	                    cc_taskcmp_fn);
	if(iter == NULL)
	{
		iter = cc_list_find(self->queue_active, task,
		                    cc_taskcmp_fn);
		if(iter == NULL)
		{
			iter = cc_list_find(self->queue_complete, task,
			                    cc_taskcmp_fn);
		}
	}

	if(iter)
	{
		cc_workqNode_t* node;
		node = (cc_workqNode_t*) cc_list_peekIter(iter);
		status = node->status;
	}

	pthread_mutex_unlock(&self->mutex);
	return status;
}

int cc_workq_pending(cc_workq_t* self)
{
	assert(self);

	int size;
	pthread_mutex_lock(&self->mutex);
	size = cc_list_size(self->queue_pending);
	size += cc_list_size(self->queue_active);
	pthread_mutex_unlock(&self->mutex);
	return size;
}
