/*
 * storagemanager.h
 *
 *  Created on: 5 Feb 2026
 *      Author: afonsofernandes
 */

#ifndef STORAGEMANAGER_STORAGEMANAGER_H_
#define STORAGEMANAGER_STORAGEMANAGER_H_

#include <stdint.h>

#define EXTERNAL_FLASH_USE 0
#define STORAGE_MANAGER_DEBUG 0


typedef enum StorageManager_Status_e {
    SM_STATUS_OK = 0,
    SM_STATUS_NOK
} StorageManager_Status_t;

typedef enum StorageManager_Id_e {
    SM_ID_IF_TILT = 0x00,
    SM_ID_IF_ACL,
    SM_ID_IF_BAC,
#if EXTERNAL_FLASH_USE == 1
    SM_ID_EF_TILT = 0x10,
    SM_ID_EF_ACL,
    SM_ID_EF_BAC,
#endif
    SM_ID_RAM_TILT = 0x20,
    SM_ID_RAM_ACL,
    SM_ID_RAM_BAC,
} StorageManager_Id_t;

typedef struct StorageManager_Partition_s {
    void *head;
    void *tail;
    void *base_addr;
	size_t size;
	size_t bytes_occupied;
	uint32_t chunk_offset;
    uint8_t full_flag;
    StorageManager_Id_t id;
} StorageManager_Partition_t;

// Internal Flash Addresses
#define SM_IF_ADDR_TILT 0x0803D000
#define SM_IF_ADDR_ACL  0x0803E000
#define SM_IF_ADDR_BAC  0x0803F000

#if EXTERNAL_FLASH_USE == 1
// External Flash Addresses
#define SM_EF_ADDR_TILT 0x00000000
#define SM_EF_ADDR_ACL  0x00001000
#define SM_EF_ADDR_BAC  0x00002000
#endif

// SRAM2 Addresses
#define SM_RAM_ADDR_TILT SRAM2_BASE
#define SM_RAM_ADDR_ACL  SRAM2_BASE + 0x1000
#define SM_RAM_ADDR_BAC  SRAM2_BASE + 0x2000

StorageManager_Status_t StorageManager_Init(void);

StorageManager_Status_t StorageManager_GetPartitionFreeSpace(StorageManager_Id_t id, size_t *size);
StorageManager_Status_t StorageManager_GetPartitionBytes(StorageManager_Id_t id, size_t *size);
StorageManager_Status_t StorageManager_CalculatePartitionBytes(StorageManager_Id_t id, size_t *size);

StorageManager_Status_t StorageManager_Load(StorageManager_Id_t id, size_t offset, size_t size, uint8_t *data);
StorageManager_Status_t StorageManager_Store(StorageManager_Id_t id, uint8_t *data, size_t size);

StorageManager_Status_t StorageManager_FreeSpace(StorageManager_Id_t id, size_t size);

#if STORAGE_MANAGER_DEBUG == 0
#define PRINT_SM(...)
#else
#include <stdio.h>
#define PRINT_SM(...) printf(__VA_ARGS__)
#endif

#endif /* STORAGEMANAGER_STORAGEMANAGER_H_ */



