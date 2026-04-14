/*
 * loramanager.h
 *
 *  Created on: 28 Feb 2026
 *      Author: afonsofernandes
 */

#ifndef LORAMANAGER_LORAMANAGER_H_
#define LORAMANAGER_LORAMANAGER_H_

#include "../Queue/queue.h"


#define LORA_MANAGER_DEBUG 0

#define LORA_MANAGER_MAX_PAYLOAD_SIZE 220

typedef enum LoRa_Manager_Status_e {
    LORA_MAN_STATUS_OK = 0,
	LORA_MAN_STATUS_NOK,
	LORA_MAN_STATUS_INVALID_STATE,
	LORA_MAN_STATUS_NOT_JOINED
} LoRa_Manager_Status_t;

typedef enum LoRa_Manager_State_e {
    LORA_START = 0x01,
	LORA_SEND,
	LORA_AWAIT_ACK,
	LORA_RESEND,
	LORA_IDLE
} LoRa_Manager_State_t;

typedef struct LoRa_Manager_TransferState_s {
    // Page vars
	Queue_QueueEntry_t *entry;
    uint64_t chunk_bitmap;
    uint64_t chunk_bitmap_mask;
    size_t   chunk_size;
    uint16_t chunk_current;
    uint16_t first_chunk_of_batch;
    uint16_t last_chunk_of_batch;
    uint16_t last_chunk;
    uint8_t  ack_interval;
    size_t timeout_counter;

    LoRa_Manager_State_t state;
} LoRa_Manager_TransferState_t;

LoRa_Manager_Status_t LoRa_Manager_GetState(LoRa_Manager_State_t *state);
LoRa_Manager_Status_t LoRa_Manager_TriggerSend();
LoRa_Manager_Status_t LoRa_Manager_InitTransfer(Queue_QueueEntry_t *entry);
LoRa_Manager_Status_t LoRa_Manager_ResetTransfer();
LoRa_Manager_Status_t LoRa_Manager_Send_Next_Entry();
LoRa_Manager_Status_t LoRa_Manager_Process();
LoRa_Manager_Status_t LoRa_Manager_HandleAck(uint8_t *data, size_t size);
LoRa_Manager_Status_t LoRa_Manager_CalculateBatchLimits();
LoRa_Manager_Status_t LoRa_Manager_BuildStartMessage(size_t size, uint8_t chunk_size , uint8_t ack_interval, uint8_t *buffer);

#if LORA_MANAGER_DEBUG == 0
#define PRINT_LM(...)
#else
#include <stdio.h>
#define PRINT_LM(...) printf(__VA_ARGS__)
#endif

#endif /* LORAMANAGER_LORAMANAGER_H_ */
