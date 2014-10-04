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

#include "fifo.h"
#include "os_calls.h"
#include "arch.h"

struct fifo_item* fifo_item_new(void* data) {
	struct fifo_item* item;
	
	item = (struct fifo_item*) g_malloc(sizeof(struct fifo_item), 1);
	
	if (item) {
		item->data = data;
	}
	return item;
}

struct fifo* fifo_new() {
	struct fifo* list;
	list = (struct fifo*) g_malloc(sizeof(struct fifo), 1);
	list->count = 0;
	return list;
}

void fifo_free(struct fifo* list) {
	while (list->head)
		fifo_pop(list);
	
	g_free(list);
}

void* fifo_pop(struct fifo* list) {
	struct fifo_item* item;
	void* data = NULL;
	
	item = list->head;
	if (item != NULL) {
		list->head = item->next;
		
		if (list->head == NULL)
			list->tail = NULL;
		else
			list->head->prev = NULL;
		
		data = item->data;
		g_free(item);
		list->count--;
	}
	return data;
}

void* fifo_tail(struct fifo* list)
{
	struct fifo_item* item;
	void* data = NULL;
	if (list->head != NULL) {
		item = list->tail;
		if (item->prev == NULL) {
			list->tail = NULL;
			list->head = NULL;
		} else {
			list->tail = item->prev;
			list->tail->next = NULL;
		}
		
		data = item->data;
		g_free(item);
		list->count--;
	}
	return data;
}


void fifo_push(struct fifo* list, void* data) {
	struct fifo_item* item;
	
	item = fifo_item_new(data);

	if (list->tail == NULL) {
		list->head = item;
		list->tail = item;
	} else {
		item->prev = list->tail;
		list->tail->next = item;
		list->tail = item;
	}
	
	list->count++;
}

bool fifo_is_empty(struct fifo* list) {
	return (list->count == 0) ? true : false;
}

void* fifo_remove(struct fifo* list, void* data)
{
	struct fifo_item* item;
	
	item = fifo_item_find(list, data);
	
	if (item != NULL) 
	{
		if (item->prev != NULL)
			item->prev->next = item->next;
		if (item->next != NULL) 
			item->next->prev = item->prev;
		if (list->head == item)
			list->head = item->next;
		if (list->tail == item)
			list->tail = item->prev;
		
		g_free(item);
		list->count--;
	}
	else
	{
		data = NULL;
	}
	return data;
}

struct fifo_item* fifo_item_find(struct fifo* list, void* data)
{
	struct fifo_item* item;
	
	for (item = list->head; item; item = item->next)
	{
		if (item->data == data)
			return item;
	}
	return NULL;
}
