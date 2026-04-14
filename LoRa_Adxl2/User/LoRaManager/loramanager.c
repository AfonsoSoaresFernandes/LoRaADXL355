/*
 * LoRaManager.c
 *
 *  Created on: 24 Feb 2026
 *      Author: afonsofernandes
 */

#include "loramanager.h"
#include "../SM2/storagemanager.h"
#include "lora_app.h"
#include "LmHandler.h"
#include "utilities_def.h"
#include "stm32_seq.h"


#include "stm32wlxx_hal.h"
#include <stdio.h>

static LoRa_Manager_TransferState_t LoRa_state = {
	.entry = NULL,
	.chunk_bitmap = 0,
	.chunk_bitmap_mask = 0,
	.chunk_size = 0,
	.chunk_current = 0,
	.first_chunk_of_batch = 0,
	.last_chunk_of_batch = 0,
	.last_chunk = 0,
	.ack_interval = 0,
	.timeout_counter = 0,
	.state = LORA_IDLE
};

/**
  * @brief User application buffer
  */
static uint8_t AppDataBuffer[LORAWAN_APP_DATA_BUFFER_MAX_SIZE];

/**
  * @brief User application data structure
  */
static LmHandlerAppData_t AppData = { 0, 0, AppDataBuffer};

static LoRa_Manager_Status_t LoRa_Send(uint8_t *buffer, size_t size);


LoRa_Manager_Status_t LoRa_Manager_GetState(LoRa_Manager_State_t *state){
	*state = LoRa_state.state;
	return LORA_MAN_STATUS_OK;
}

LoRa_Manager_Status_t LoRa_Manager_TriggerSend(){
	PRINT_LM("[LM] - Triggering send\r\n");
	return LORA_MAN_STATUS_OK;
}

LoRa_Manager_Status_t LoRa_Manager_InitTransfer(Queue_QueueEntry_t *entry) {
	PRINT_LM("[LM] - Initializing transfer - Start\r\n");

	LoRa_state.entry = entry;

	LoRa_state.chunk_size = LORA_MANAGER_MAX_PAYLOAD_SIZE;
	if (entry->size < LORA_MANAGER_MAX_PAYLOAD_SIZE){
		LoRa_state.chunk_size = entry->size;
	}
	LoRa_state.chunk_current = 0;
	LoRa_state.ack_interval = 60; // TODO: make this dynamic based on the size of the transfer.
	LoRa_state.timeout_counter = 0;
	LoRa_state.state = LORA_START;

	if (entry->size % LoRa_state.chunk_size) {
		LoRa_state.last_chunk = (entry->size / LoRa_state.chunk_size);
	} else {
		LoRa_state.last_chunk = (entry->size / LoRa_state.chunk_size) - 1;
	}

	LoRa_state.first_chunk_of_batch = 0;
	LoRa_state.last_chunk_of_batch = LoRa_state.ack_interval - 1;
	if (LoRa_state.last_chunk_of_batch > LoRa_state.last_chunk) {
		LoRa_state.last_chunk_of_batch = LoRa_state.last_chunk;
	}

	LoRa_state.chunk_bitmap = 0;
	for (size_t i = 0; i < (LoRa_state.last_chunk_of_batch - LoRa_state.first_chunk_of_batch + 1); i++) {
		LoRa_state.chunk_bitmap_mask |= (1 << i);
	}

	PRINT_LM("OK, Entry size: %d bytes, chunk size: %d bytes, total chunks: %d, ack interval: %d chunks\r\n", entry->size, LoRa_state.chunk_size, LoRa_state.last_chunk + 1, LoRa_state.ack_interval);
	PRINT_LM("[LM] - Initializing transfer - Finish\r\n");

	return LORA_MAN_STATUS_OK;
}

LoRa_Manager_Status_t LoRa_Manager_ResetTransfer() {
	PRINT_LM("[LM] - Resetting transfer\r\n");
	LoRa_state.entry = NULL;
	LoRa_state.chunk_bitmap = 0;
	LoRa_state.chunk_bitmap_mask = 0;
	LoRa_state.chunk_size = 0;
	LoRa_state.chunk_current = 0;
	LoRa_state.first_chunk_of_batch = 0;
	LoRa_state.last_chunk_of_batch = 0;
	LoRa_state.ack_interval = 0;
	LoRa_state.timeout_counter = 0;
	LoRa_state.state = LORA_IDLE;
	return LORA_MAN_STATUS_OK;
}

LoRa_Manager_Status_t LoRa_Manager_Send_Next_Entry() {
	Queue_QueueEntry_t *entry;
	LoRa_Manager_Status_t ret;

	printf("hey ho\r\n");
	PRINT_LM("[LM] - Sending next entry - Start\r\n");
	if (!LoRa_state.entry){
		PRINT_LM("No ongoing transfer, getting next entry from queue\r\n");
		Queue_GetNextEntry(&entry);
		if (!entry){
			PRINT_LM("OK, No entry in queue\r\n");
			LoRa_Manager_ResetTransfer();
			PRINT_LM("[LM] - Sending next entry -Finish\r\n");
			return LORA_MAN_STATUS_OK;
		}
		PRINT_LM("OK, Entry found in queue, initializing transfer\r\n");
		LoRa_Manager_InitTransfer(entry);
	}

	ret = LoRa_Manager_Process();
	if(ret == LORA_MAN_STATUS_NOK){
		LoRa_state.entry->retries++;
		PRINT_LM("Failed to process transfer, current number of retries: %u\r\n", (unsigned int)LoRa_state.entry->retries);
		if (LoRa_state.entry->retries >= 3) {
			PRINT_LM("Max number of retries reached for entry, removing from queue\r\n");
			Queue_RemoveEntry(LoRa_state.entry);
			LoRa_Manager_ResetTransfer();
			Queue_GetNextEntry(&entry);
			if (entry){
				PRINT_LM("New Entry found in queue, initializing transfer\r\n");
				LoRa_Manager_InitTransfer(entry);
			}
		} else {
			PRINT_LM("Retrying transfer\r\n");
			Queue_ResetEntry(LoRa_state.entry);
			LoRa_Manager_InitTransfer(LoRa_state.entry);
		}
		return LORA_MAN_STATUS_NOK;
	}
	else if (ret == LORA_MAN_STATUS_INVALID_STATE){
		PRINT_LM("Invalid state reached during transfer processing\r\n");
		PRINT_LM("Retrying transfer\r\n");
		Queue_ResetEntry(LoRa_state.entry);
		LoRa_Manager_InitTransfer(LoRa_state.entry);
	}

	PRINT_LM("[LM] - Sending next entry - Finish\r\n");
	return LORA_MAN_STATUS_OK;
}

LoRa_Manager_Status_t LoRa_Manager_Process() {
	uint8_t buffer[2 + LoRa_state.chunk_size];
	size_t chunk_size = LoRa_state.chunk_size;
	uint16_t chunk_index = 0;
	uint32_t now, later;

	PRINT_LM("[LM] - Processing transfer - Start\r\n");
	PRINT_LM("Current state: %d\r\n", LoRa_state.state);
	switch (LoRa_state.state) {
		case LORA_START:
			LoRa_Manager_BuildStartMessage(LoRa_state.entry->size, LoRa_state.chunk_size, LoRa_state.ack_interval ,buffer);
			if (LoRa_Send(buffer, 8) != LORA_MAN_STATUS_OK) {
				PRINT_LM("NOK Failed to send start message\r\n");
				PRINT_LM("[LM] - Processing transfer - Finish\r\n");
				return LORA_MAN_STATUS_NOK;
			}
			PRINT_LM("Start message sent\r\n");
			LoRa_state.state = LORA_SEND;
			break;
		case LORA_SEND:
			if ((LoRa_state.entry->size - LoRa_state.entry->offset) < LoRa_state.chunk_size) {
				chunk_size = LoRa_state.entry->size - LoRa_state.entry->offset;
			}
			memcpy(buffer, &LoRa_state.chunk_current, 2);


			now = HAL_GetTick();
			if (StorageManager_Load(LoRa_state.entry->addr, LoRa_state.entry->offset, chunk_size, &buffer[2]) != SM_STATUS_OK) {
				PRINT_LM("NOK, Failed to load chunk data from storage\r\n");
				PRINT_LM("[LM] - Processing transfer - Finish\r\n");
				return LORA_MAN_STATUS_NOK;
			}

			later = HAL_GetTick();
			printf("SM: %lu ms\r\n", later - now);
			now = HAL_GetTick();
			if (LoRa_Send(buffer, 2 + chunk_size) != LORA_MAN_STATUS_OK) {
				PRINT_LM("NOK, Failed to send chunk data\r\n");
				PRINT_LM("[LM] - Processing transfer - Finish\r\n");
				return LORA_MAN_STATUS_NOK;
			}

			later = HAL_GetTick();
			printf("Radio execution time: %lu ms, [%lu, %lu]\r\n", later - now, now ,later);

			LoRa_state.entry->offset += chunk_size;
			if (LoRa_state.chunk_current == LoRa_state.last_chunk_of_batch) {
				PRINT_LM("Batch sent, awaiting ack\r\n");
				LoRa_state.state = LORA_AWAIT_ACK;
				LoRa_state.chunk_current++;
				break;
			}
			PRINT_LM("Chunk %d sent\r\n", LoRa_state.chunk_current);
			LoRa_state.chunk_current++;
			break;
		case LORA_AWAIT_ACK:
			LoRa_state.timeout_counter++;
			PRINT_LM("Awaiting ack, but nothing received, timeout counter: %d\r\n", LoRa_state.timeout_counter);

			uint8_t aux[2] = {0xFF, 0xFF};
			if (LoRa_Send(aux, 2) != LORA_MAN_STATUS_OK) {
				PRINT_LM("NOK, Failed to send dummy data\r\n");
				PRINT_LM("[LM] - Processing transfer - Finish\r\n");
				return LORA_MAN_STATUS_NOK;
			}

			PRINT_LM("Sending dummy Tx\r\n");
			if (LoRa_state.timeout_counter >= 3) {
				PRINT_LM("NOK, Ack timeout exceeded\r\n");
				PRINT_LM("[LM] - Processing transfer - Finish\r\n");
				return LORA_MAN_STATUS_NOK;
			}
			break;
		case LORA_RESEND:
			PRINT_LM("Resending chunks, bitmap of received chunks: %llx\r\n", LoRa_state.chunk_bitmap);
			for (size_t i = 0; LoRa_state.chunk_bitmap_mask & (1 << i) ;i++) {
				if (!(LoRa_state.chunk_bitmap & (1 << i))) {
					chunk_index = LoRa_state.first_chunk_of_batch + i;

					if ((LoRa_state.entry->size - (chunk_index * LoRa_state.chunk_size)) < LoRa_state.chunk_size) {
						chunk_size = LoRa_state.entry->size - LoRa_state.entry->offset;
					}
					memcpy(buffer, &chunk_index, 2);
					if (StorageManager_Load(LoRa_state.entry->addr, (chunk_index * LoRa_state.chunk_size), chunk_size, &buffer[2]) != SM_STATUS_OK) {
						PRINT_LM("NOK, Failed to load chunk data from storage\r\n");
						PRINT_LM("[LM] - Processing transfer - Finish\r\n");
						return LORA_MAN_STATUS_NOK;
					}
					if (LoRa_Send(buffer, 2 + chunk_size) != LORA_MAN_STATUS_OK) {
						PRINT_LM("NOK, Failed to send chunk data\r\n");
						PRINT_LM("[LM] - Processing transfer - Finish\r\n");
						return LORA_MAN_STATUS_NOK;
					}
					PRINT_LM("Chunk %d resent\r\n", chunk_index);
					LoRa_state.chunk_bitmap |= (1 << i);
					if (LoRa_state.chunk_bitmap == LoRa_state.chunk_bitmap_mask) {
						LoRa_state.state = LORA_AWAIT_ACK;
						PRINT_LM("All chunks resent, awaiting ack\r\n");
					}
					break;
				}

			}
			PRINT_LM("NOK, Still in re send state, but the not chunks where found to be missing\r\n");
			PRINT_LM("[LM] - Processing transfer - Finish\r\n");
			return LORA_MAN_STATUS_NOK;

		default:
			PRINT_LM("NOK, Invalid State\r\n");
			PRINT_LM("[LM] - Processing transfer - Finish\r\n");
			return LORA_MAN_STATUS_NOK;
	}
	PRINT_LM("OK\r\n");
	PRINT_LM("[LM] - Processing transfer - Finish\r\n");
	return LORA_MAN_STATUS_OK;
}

LoRa_Manager_Status_t LoRa_Manager_HandleAck(uint8_t *data, size_t size) {
	Queue_QueueEntry_t *entry = NULL;
	uint64_t chunk_bitmap;

	PRINT_LM("[LM] - Handling Ack - Start\r\n");
	if (LoRa_state.state != LORA_AWAIT_ACK) {
		PRINT_LM("NOK, Ack received but not in awaiting ack state, state: %02x\r\n", LoRa_state.state);
		PRINT_LM("[LM] - Handling Ack - Finish\r\n");
		return LORA_MAN_STATUS_NOK;
	}

	memcpy(&chunk_bitmap, data, sizeof(uint64_t));
	if (LoRa_state.chunk_bitmap_mask != chunk_bitmap) {
		LoRa_state.state = LORA_RESEND;
		LoRa_state.chunk_bitmap = chunk_bitmap;
		PRINT_LM("OK, Ack received with missing chunks, bitmap of received chunks: %llx\r\n", chunk_bitmap);
		PRINT_LM("[LM] - Handling Ack - Finish\r\n");
		return LORA_MAN_STATUS_OK;
	}

	PRINT_LM("Ack received with all chunks received\r\n");
	if (LoRa_state.last_chunk_of_batch == LoRa_state.last_chunk) {
		PRINT_LM("All chunks of entry sent and acknowledged, removing entry from queue\r\n");
		StorageManager_FreeSpace(LoRa_state.entry->addr, LoRa_state.entry->size);
		Queue_RemoveEntry(LoRa_state.entry);
		Queue_GetNextEntry(&entry);
		if (entry){
			PRINT_LM("New entry found in queue, Continuing transfer\r\n");
			LoRa_Manager_InitTransfer(entry);
		}
		else {
			PRINT_LM("No more entries in queue, going to sleep\r\n");
			LoRa_Manager_ResetTransfer();
		}
	} else {
		PRINT_LM("Batch acknowledged, sending next batch\r\n");
		LoRa_Manager_CalculateBatchLimits();
		LoRa_state.state = LORA_SEND;
	}

	return LORA_MAN_STATUS_OK;
}

LoRa_Manager_Status_t LoRa_Manager_CalculateBatchLimits() {
	LoRa_state.first_chunk_of_batch = LoRa_state.last_chunk_of_batch + 1;
	LoRa_state.last_chunk_of_batch = LoRa_state.first_chunk_of_batch + LoRa_state.ack_interval - 1;
	if (LoRa_state.last_chunk_of_batch > LoRa_state.last_chunk) {
		LoRa_state.last_chunk_of_batch = LoRa_state.last_chunk;
	}

	LoRa_state.chunk_bitmap_mask = 0;
	for (size_t i = 0; i < (LoRa_state.last_chunk_of_batch - LoRa_state.first_chunk_of_batch + 1); i++) {
		LoRa_state.chunk_bitmap_mask |= (1 << i);
	}
	LoRa_state.chunk_bitmap = 0;
	return LORA_MAN_STATUS_OK;
}

LoRa_Manager_Status_t LoRa_Manager_BuildStartMessage(size_t size, uint8_t chunk_size , uint8_t ack_interval, uint8_t *buffer){
	buffer[0] = 0xFA;
	buffer[1] = 0xFF;
	memcpy(&buffer[2], &size, 4);
	buffer[6] = chunk_size;
	buffer[7] = ack_interval;
	return LORA_MAN_STATUS_OK;
}

static LoRa_Manager_Status_t LoRa_Send(uint8_t *buffer, size_t size) {
	LmHandlerErrorStatus_t ret;
	memcpy(AppData.Buffer, buffer, size);
	AppData.BufferSize = size;
	AppData.Port = 10;

	ret = LmHandlerSend(&AppData, LORAMAC_HANDLER_UNCONFIRMED_MSG, false);
	if (ret == LORAMAC_HANDLER_SUCCESS)
	{
		return LORA_MAN_STATUS_OK;
	}

	return LORA_MAN_STATUS_NOK;
}

