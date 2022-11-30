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
	ASSERT(task);
	ASSERT((purge_id == 0) || (purge_id == 1));

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
	ASSERT(_self);

	cc_workqNode_t* self = *_self;
	if(self)
	{
		FREE(self);
		*_self = NULL;
	}
}

static void
cc_workq_removeLocked(cc_workq_t* self, int finish,
                      cc_list_t* queue,
                      cc_listIter_t** _iter)
{
	ASSERT(self);
	ASSERT(queue);
	ASSERT(_iter);

	cc_workqNode_t* node;
	cc_mapIter_t*   miter;
	node  = (cc_workqNode_t*)
	        cc_list_remove(queue, _iter);
	miter = cc_map_findp(self->map_task, 0, node->task);
	cc_map_remove(self->map_task, &miter);

	if(finish)
	{
		(*self->finish_fn)(self->owner, node->task, node->status);
	}

	cc_workqNode_delete(&node);
}

static void* cc_workq_thread(void* arg)
{
	ASSERT(arg);

	cc_workq_t* self = (cc_workq_t*) arg;

	// override the thread priority
	if(self->thread_priority == CC_WORKQ_THREAD_PRIORITY_DEFAULT)
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
		pthread_cond_broadcast(&self->cond_complete);
	}
}
static void cc_workq_flushLocked(cc_workq_t* self)
{
	ASSERT(self);

	cc_listIter_t* iter;
	iter = cc_list_head(self->queue_complete);
	while(iter)
	{
		cc_workq_removeLocked(self, 1, self->queue_complete,
		                      &iter);
	}
}

/***********************************************************
* public                                                   *
***********************************************************/

cc_workq_t*
cc_workq_new(void* owner, int thread_count,
             int thread_priority,
             cc_workqRun_fn run_fn,
             cc_workqFinish_fn finish_fn)
{
	// owner may be NULL
	ASSERT(run_fn);
	ASSERT(finish_fn);

	cc_workq_t* self;
	self = (cc_workq_t*) MALLOC(sizeof(cc_workq_t));
	if(!self)
	{
		LOGE("MALLOC failed");
		return NULL;
	}

	self->state           = CC_WORKQ_STATE_RUNNING;
	self->owner           = owner;
	self->purge_id        = 0;
	self->thread_count    = thread_count;
	self->thread_priority = thread_priority;
	self->next_tid        = 0;
	self->run_fn          = run_fn;
	self->finish_fn       = finish_fn;

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

	self->map_task = cc_map_new();
	if(self->map_task == NULL)
	{
		goto fail_map_task;
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
		cc_map_delete(&self->map_task);
	fail_map_task:
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
	ASSERT(_self);

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
		cc_map_delete(&self->map_task);

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
	ASSERT(self);

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
	ASSERT(self);

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
			cc_workq_removeLocked(self, 1, self->queue_pending,
			                      &iter);
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
			cc_workq_removeLocked(self, 1, self->queue_complete,
			                      &iter);
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
	ASSERT(self);

	pthread_mutex_lock(&self->mutex);
	cc_workq_flushLocked(self);
	pthread_mutex_unlock(&self->mutex);
}

void cc_workq_finish(cc_workq_t* self)
{
	ASSERT(self);

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
	ASSERT(self);
	ASSERT(task);

	pthread_mutex_lock(&self->mutex);

	int status = CC_WORKQ_STATUS_ERROR;

	// find the node containing the task or create a new one
	cc_listIter_t*  iter;
	cc_mapIter_t*   miter;
	cc_workqNode_t* node;
	cc_listIter_t*  pos;
	cc_workqNode_t* tmp;
	miter = cc_map_findp(self->map_task, 0, task);
	if(miter == NULL)
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
			iter = cc_list_append(self->queue_pending, pos,
			                      (const void*) node);
			if(iter == NULL)
			{
				goto fail_queue;
			}
		}
		else
		{
			// insert at head of queue
			// first item or highest priority
			iter = cc_list_insert(self->queue_pending, NULL,
			                      (const void*) node);
			if(iter == NULL)
			{
				goto fail_queue;
			}
		}

		if(cc_map_addp(self->map_task, (const void*) iter,
		               0, task) == NULL)
		{
			goto fail_map_add;
		}

		status = node->status;

		// wake up workq thread
		pthread_cond_broadcast(&self->cond_pending);
	}
	else
	{
		iter = (cc_listIter_t*)  cc_map_val(miter);
		node = (cc_workqNode_t*) cc_list_peekIter(iter);
	}

	if(node->status == CC_WORKQ_STATUS_ACTIVE)
	{
		node = (cc_workqNode_t*) cc_list_peekIter(iter);
		node->purge_id = self->purge_id;
		status = node->status;
	}
	else if(node->status == CC_WORKQ_STATUS_PENDING)
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
		node = (cc_workqNode_t*) cc_list_peekIter(iter);
		status = node->status;
		cc_workq_removeLocked(self, 0, self->queue_complete,
		                      &iter);
	}

	pthread_mutex_unlock(&self->mutex);

	// success
	return status;

	// failure
	fail_map_add:
		cc_list_remove(self->queue_pending, &iter);
	fail_queue:
		cc_workqNode_delete(&node);
	fail_node:
		pthread_mutex_unlock(&self->mutex);
	return CC_WORKQ_STATUS_ERROR;
}

int cc_workq_wait(cc_workq_t* self, void* task,
                  int blocking)
{
	ASSERT(self);
	ASSERT(task);

	int status = CC_WORKQ_STATUS_ERROR;

	pthread_mutex_lock(&self->mutex);

	// find task in map
	cc_listIter_t* iter;
	cc_mapIter_t*  miter;
	miter = cc_map_findp(self->map_task, 0, task);
	if(miter == NULL)
	{
		pthread_mutex_unlock(&self->mutex);
		return status;
	}
	iter = (cc_listIter_t*) cc_map_val(miter);

	cc_workqNode_t* node;
	node = (cc_workqNode_t*) cc_list_peekIter(iter);
	while((node->status == CC_WORKQ_STATUS_PENDING) ||
	      (node->status == CC_WORKQ_STATUS_ACTIVE))
	{
		if(blocking == 0)
		{
			pthread_mutex_unlock(&self->mutex);
			return node->status;
		}

		// must wait for pending/active task to complete
		pthread_cond_wait(&self->cond_complete, &self->mutex);
	}

	status = node->status;

	// cancel completed task
	cc_workq_removeLocked(self, 0, self->queue_complete,
	                      &iter);

	pthread_mutex_unlock(&self->mutex);
	return status;
}

int cc_workq_cancel(cc_workq_t* self, void* task,
                    int blocking)
{
	ASSERT(self);
	ASSERT(task);

	int status = CC_WORKQ_STATUS_ERROR;

	pthread_mutex_lock(&self->mutex);

	// find task in map
	cc_listIter_t* iter;
	cc_mapIter_t*  miter;
	miter = cc_map_findp(self->map_task, 0, task);
	if(miter == NULL)
	{
		pthread_mutex_unlock(&self->mutex);
		return status;
	}
	iter = (cc_listIter_t*) cc_map_val(miter);

	cc_workqNode_t* node;
	node = (cc_workqNode_t*) cc_list_peekIter(iter);
	while(node->status == CC_WORKQ_STATUS_ACTIVE)
	{
		if(blocking == 0)
		{
			pthread_mutex_unlock(&self->mutex);
			return node->status;
		}

		// must wait for active task to complete
		pthread_cond_wait(&self->cond_complete, &self->mutex);
	}

	status = node->status;
	if(status == CC_WORKQ_STATUS_PENDING)
	{
		// cancel pending task
		cc_workq_removeLocked(self, 0, self->queue_pending,
		                      &iter);
	}
	else
	{
		// cancel completed task
		cc_workq_removeLocked(self, 0, self->queue_complete,
		                      &iter);
	}

	pthread_mutex_unlock(&self->mutex);
	return status;
}

int cc_workq_status(cc_workq_t* self, void* task)
{
	ASSERT(self);
	ASSERT(task);

	int status = CC_WORKQ_STATUS_ERROR;
	pthread_mutex_lock(&self->mutex);

	// find task in map
	cc_listIter_t* iter;
	cc_mapIter_t*  miter;
	miter = cc_map_findp(self->map_task, 0, task);
	if(miter)
	{
		cc_workqNode_t* node;
		iter = (cc_listIter_t*)  cc_map_val(miter);
		node = (cc_workqNode_t*) cc_list_peekIter(iter);
		status = node->status;
	}

	pthread_mutex_unlock(&self->mutex);
	return status;
}

int cc_workq_pending(cc_workq_t* self)
{
	ASSERT(self);

	int size;
	pthread_mutex_lock(&self->mutex);
	size = cc_list_size(self->queue_pending);
	size += cc_list_size(self->queue_active);
	pthread_mutex_unlock(&self->mutex);
	return size;
}
