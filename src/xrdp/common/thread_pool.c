/**
 * Copyright (C) 2010 Ulteo SAS
 * http://www.ulteo.com
 * Author David Lechevalier <david@ulteo.com> 2010
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 **/

#include "thread_pool.h"


void
thread_pool_lock_thread(THREAD_POOL* pool)
{
	tc_mutex_lock(pool->thread_wait);
}

void
thread_pool_unlock_thread(THREAD_POOL* pool)
{
	tc_mutex_unlock(pool->thread_wait);
}

void
thread_pool_acquire_job(THREAD_POOL* pool)
{
	tc_mutex_lock(pool->job_wait);
}

void
thread_pool_release_job(THREAD_POOL* pool)
{
	tc_mutex_unlock(pool->job_wait);
}

int
thread_pool_push_job(THREAD_POOL* pool, int job)
{
	if (job == 0)
	{
		return 1;
	}

	thread_pool_acquire_job(pool);
	list_add_item(pool->job_queue, job);
	thread_pool_release_job(pool);

	thread_pool_unlock_thread(pool);
	return 0;
}

int
thread_pool_pop_job(THREAD_POOL* pool)
{
	int job = 0;

	thread_pool_acquire_job(pool);
	if (pool->job_queue->count == 0)
	{
		thread_pool_release_job(pool);
		return 0;
	}

	job = list_get_item(pool->job_queue, 0);
	list_remove_item(pool->job_queue, 0);

	if (pool->job_queue->count == 0)
	{
		thread_pool_lock_thread(pool);
	}
	thread_pool_release_job(pool);
	return job;
}

void
thread_pool_wait_job(THREAD_POOL* pool)
{
	thread_pool_lock_thread(pool);
	thread_pool_unlock_thread(pool);
}

THREAD_POOL*
thread_pool_init_pool(int thread_number)
{
	THREAD_POOL* pool = NULL;

	pool = g_malloc(sizeof(THREAD_POOL), 1);
	if (pool == NULL)
	{
	  return NULL;
	}

	pool->max_thread_count = thread_number;
	pool->job_queue = list_create();
	pool->thread_list = list_create();
	pool->job_queue->auto_free = 0;
	pool->thread_list->auto_free = 0;

	pool->job_wait = tc_mutex_create();
	pool->thread_wait = tc_mutex_create();
	return pool;
}

void
thread_pool_delete_pool(THREAD_POOL* pool)
{
	thread_pool_lock_thread(pool);
	int i = 0;

	for(i=0 ; i<pool->max_thread_count ; i++)
	{
		pthread_t t = list_get_item(pool->thread_list, i);
		pthread_cancel(t);
		pthread_join(t, NULL);
	}
}

void
thread_pool_start_pool_thread(THREAD_POOL* pool, void* (* thread_routine)(void*))
{
	thread_pool_lock_thread(pool);
	int i;

	for(i=0 ; i<pool->max_thread_count ; i++)
	{
		int t = tc_thread_create((void*)thread_routine, (void*)NULL);
		list_add_item(pool->thread_list, (long)t);
	}
}
