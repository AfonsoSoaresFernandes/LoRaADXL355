/*
 * queue.c
 *
 *  Created on: 26 Feb 2026
 *      Author: afonsofernandes
 */

#include "queue.h"
#include <stdlib.h>

static Queue_QueueEntry_t *Queue = 0;

Queue_Status_t Queue_CreateEntry(StorageManager_Id_t addr, size_t size, Queue_Priorities_t priority, Queue_QueueEntry_t **entry) {

	PRINT_Q("[QUEUE] Create Entry - Start\r\n");
	*entry = (Queue_QueueEntry_t *)malloc(sizeof(Queue_QueueEntry_t));
	if(!*entry) {
		PRINT_Q("NOK, failed to allocate memory for queue entry\r\n");
		PRINT_Q("[QUEUE] Create Entry - Finish\r\n");
		return QUEUE_STATUS_NOK;
	}

	(*entry)->addr = addr;
	(*entry)->size = size;
	(*entry)->offset = 0;
	(*entry)->priority = priority;
	(*entry)->next = NULL;
	(*entry)->retries = 0;

	PRINT_Q("OK, created entry with addr: %02x, size: %d, priority: %d\r\n", addr, size, priority);
	PRINT_Q("[QUEUE] Create Entry - Finish\r\n");
	return QUEUE_STATUS_OK;
}

Queue_Status_t Queue_ResetEntry(Queue_QueueEntry_t *entry) {
	PRINT_Q("[QUEUE] Reset Entry - Start\r\n");
	entry->offset = 0;
	PRINT_Q("OK\r\n");
	PRINT_Q("[QUEUE] Reset Entry - Finish\r\n");
	return QUEUE_STATUS_OK;
}

Queue_Status_t Queue_AddEntry(Queue_QueueEntry_t *entry) {
	Queue_QueueEntry_t *aux;
	Queue_QueueEntry_t **next_ptr_addr;

	PRINT_Q("[QUEUE] Add Entry - Start\r\n");

	if (!Queue){
		Queue = entry;
		PRINT_Q("OK\r\n");
		PRINT_Q("[QUEUE] Add Entry - Finish\r\n");
		return QUEUE_STATUS_OK;
	}

	aux = Queue;
	next_ptr_addr = &Queue;

	while(aux){
		if (aux->priority < entry->priority){
			*next_ptr_addr = entry;
			entry->next = aux;
			PRINT_Q("OK\r\n");
			PRINT_Q("[QUEUE] Add Entry - Finish\r\n");
			return QUEUE_STATUS_OK;
		}
		next_ptr_addr = &(aux->next);
		aux = aux->next;
	}
	*next_ptr_addr = entry;

	PRINT_Q("OK\r\n");
	PRINT_Q("[QUEUE] Add Entry - Finish\r\n");
	return QUEUE_STATUS_OK;
}

Queue_Status_t Queue_RemoveEntry(Queue_QueueEntry_t *entry) {
	Queue_QueueEntry_t *aux;
	Queue_QueueEntry_t **next_ptr_addr;

	PRINT_Q("[QUEUE] Remove Entry - Start\r\n");

	aux = Queue;
	next_ptr_addr = &Queue;

	while(aux){
		if(entry == aux){
			*next_ptr_addr = aux->next;
			free(aux);
			PRINT_Q("OK - Removed an entry for id: %02x, size: %lu, priority: %d\r\n", entry->addr, entry->size, entry->priority);
			PRINT_Q("[QUEUE] Remove Entry - Finish\r\n");
			return QUEUE_STATUS_OK;
		}
		next_ptr_addr = &(aux->next);
		aux = aux->next;
	}

	PRINT_Q("NOK, entry not found in queue with id: %02x, size: %lu, priority: %d\r\n", entry->addr, entry->size, entry->priority);
	PRINT_Q("[QUEUE] Remove Entry - Finish\r\n");
	return QUEUE_STATUS_NOK;
}

Queue_Status_t Queue_GetNextEntry(Queue_QueueEntry_t **entry) {
	PRINT_Q("[QUEUE] Get Next Entry - Start\r\n");

	if (Queue){
		*entry = Queue;
		PRINT_Q("OK, next entry has id: %02x, size: %lu, priority: %d\r\n", (*entry)->addr, (*entry)->size, (*entry)->priority);
	} else {
		*entry = NULL;
		PRINT_Q("OK, no entries in queue\r\n");
	}

	PRINT_Q("[QUEUE] Get Next Entry - Finish\r\n");
	return QUEUE_STATUS_OK;
}

