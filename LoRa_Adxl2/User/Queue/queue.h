/*
 * Queue.h
 *
 *  Created on: 26 Feb 2026
 *      Author: afonsofernandes
 */

#ifndef QUEUE_H_
#define QUEUE_H_

#include <stddef.h>
#include <stdint.h>
#include "../SM2/storagemanager.h"

#define QUEUE_DEBUG 0

typedef enum Queue_Status_e {
    QUEUE_STATUS_OK = 0,
	QUEUE_STATUS_NOK
} Queue_Status_t;

typedef enum Queue_Priorities_e {
	QUEUE_PRIORITY_0 = 0,
	QUEUE_PRIORITY_1,
	QUEUE_PRIORITY_2,
	QUEUE_PRIORITY_3,
	QUEUE_PRIORITY_4
} Queue_Priorities_t;



typedef struct Queue_QueueEntry_s {
	struct Queue_QueueEntry_s *next;
	StorageManager_Id_t addr;
	size_t size;
	size_t offset;
	Queue_Priorities_t priority;
	uint8_t retries;
} Queue_QueueEntry_t;

Queue_Status_t Queue_CreateEntry(StorageManager_Id_t addr, size_t size, Queue_Priorities_t priority, Queue_QueueEntry_t **entry);
Queue_Status_t Queue_ResetEntry(Queue_QueueEntry_t *entry);
Queue_Status_t Queue_AddEntry(Queue_QueueEntry_t *entry);
Queue_Status_t Queue_RemoveEntry(Queue_QueueEntry_t *entry);
Queue_Status_t Queue_GetNextEntry(Queue_QueueEntry_t **entry);

#if QUEUE_DEBUG == 0
#define PRINT_Q(...)
#else
#include <stdio.h>
#define PRINT_Q(...) printf(__VA_ARGS__)
#endif

#endif /* QUEUE_H_ */
