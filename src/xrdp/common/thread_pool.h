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

#ifndef THREAD_POOL_H_
#define THREAD_POOL_H_

#include <os_calls.h>
#include <pthread.h>
#include <thread_calls.h>
#include <list.h>

typedef struct thread_pool
{
	tbus thread_wait;
	tbus job_wait;

	int max_thread_count;

	struct list* thread_list;
	struct list* job_queue;
} THREAD_POOL;


void
thread_pool_lock_thread(THREAD_POOL* pool);
void
thread_pool_unlock_thread(THREAD_POOL* pool);
void
thread_pool_acquire_job(THREAD_POOL* pool);
void
thread_pool_release_job(THREAD_POOL* pool);
int
thread_pool_push_job(THREAD_POOL* pool, int job);
int
thread_pool_pop_job(THREAD_POOL* pool);
void
thread_pool_wait_job(THREAD_POOL* pool);
THREAD_POOL*
thread_pool_init_pool(int thread_number);
void
thread_pool_delete_pool(THREAD_POOL* pool);
void
thread_pool_start_pool_thread(THREAD_POOL* pool, void* (* thread_routine)(void*));


#endif /* THREAD_POOL_H_ */
