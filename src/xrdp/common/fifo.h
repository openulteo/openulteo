/**
 * Copyright (C) 2012 Ulteo SAS
 * http://www.ulteo.com
 * Author Vincent Roullier <vincent.roullier@ulteo.com> 2012
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

#ifndef _FIFO_H
#define _FIFO_H

#include "arch.h"


struct fifo_item {
	void* data;
	struct fifo_item* next;
	struct fifo_item* prev;
};

struct fifo {
	int count;
	struct fifo_item* head;
	struct fifo_item* tail;
};

struct fifo_item* fifo_item_new(void* data);

struct fifo* fifo_new();
void fifo_free(struct fifo* f);
void fifo_push(struct fifo* f, void* data);
void* fifo_pop(struct fifo* f);
void* fifo_tail(struct fifo* f);
bool fifo_is_empty(struct fifo* f);
void* fifo_remove(struct fifo* f, void* data);
struct fifo_item* fifo_item_find(struct fifo* f, void* data);

#endif // _FIFO_H
