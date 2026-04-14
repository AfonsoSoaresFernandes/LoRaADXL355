/*
 * storagemanager.c
 *
 *  Created on: 5 Feb 2026
 *      Author: afonsofernandes
 */

#include <string.h>

#include "storagemanager.h"
#include "../Flash_Interface/flash_interface.h"

#if EXTERNAL_FLASH_USE == 1
#include "../W25Q128JV/w25q.h"
#endif

// Internal Flash table
static StorageManager_Partition_t SM_IFlash_partitions [] = {
        {.id = SM_ID_IF_TILT, .head = (void *)SM_IF_ADDR_TILT, .tail = (void *)SM_IF_ADDR_TILT, .base_addr = (void *)SM_IF_ADDR_TILT, .size = 2*FLASH_PAGE_SIZE},
        {.id = SM_ID_IF_ACL , .head = (void *)SM_IF_ADDR_ACL , .tail = (void *)SM_IF_ADDR_ACL , .base_addr = (void *)SM_IF_ADDR_ACL , .size = 2*FLASH_PAGE_SIZE},
        {.id = SM_ID_IF_BAC , .head = (void *)SM_IF_ADDR_BAC , .tail = (void *)SM_IF_ADDR_BAC , .base_addr = (void *)SM_IF_ADDR_BAC , .size = 2*FLASH_PAGE_SIZE},
};

static StorageManager_Partition_t SM_SRAM2_partitions [] = {
		{.id = SM_ID_RAM_TILT, .head = (void *)SM_RAM_ADDR_TILT, .tail = (void *)SM_RAM_ADDR_TILT, .base_addr = (void *)SM_RAM_ADDR_TILT, .size = 2*FLASH_PAGE_SIZE},
		{.id = SM_ID_RAM_ACL , .head = (void *)SM_RAM_ADDR_ACL , .tail = (void *)SM_RAM_ADDR_ACL , .base_addr = (void *)SM_RAM_ADDR_ACL , .size = 2*FLASH_PAGE_SIZE},
		{.id = SM_ID_RAM_BAC , .head = (void *)SM_RAM_ADDR_BAC , .tail = (void *)SM_RAM_ADDR_BAC , .base_addr = (void *)SM_RAM_ADDR_BAC , .size = 2*FLASH_PAGE_SIZE},
};

static const uint32_t SM_IFlash_partitions_size = sizeof(SM_IFlash_partitions) / sizeof(SM_IFlash_partitions[0]);
static const uint32_t SM_SRAM2_partitions_size = sizeof(SM_SRAM2_partitions) / sizeof(SM_SRAM2_partitions[0]);

#if EXTERNAL_FLASH_USE == 1
static StorageManager_Partition_t SM_EFlash_partitions [] = {
        {.id = SM_ID_EF_TILT, .head = (void *)SM_EF_ADDR_TILT, .tail = (void *)SM_EF_ADDR_TILT, .base_addr = (void *)SM_EF_ADDR_TILT, .size = 4 * W25Q_SECTOR_SIZE},
        {.id = SM_ID_EF_ACL , .head = (void *)SM_EF_ADDR_ACL , .tail = (void *)SM_EF_ADDR_ACL , .base_addr = (void *)SM_EF_ADDR_ACL , .size = 4 * W25Q_SECTOR_SIZE},
        {.id = SM_ID_EF_BAC , .head = (void *)SM_EF_ADDR_BAC , .tail = (void *)SM_EF_ADDR_BAC , .base_addr = (void *)SM_EF_ADDR_BAC , .size = 4 * W25Q_SECTOR_SIZE},
};
static const uint32_t SM_EFlash_partitions_size = sizeof(SM_EFlash_partitions) / sizeof(SM_EFlash_partitions[0]);
#endif


// Static functions prototypes
static StorageManager_Status_t Check_InternalFlash();
static StorageManager_Status_t Check_SRAM2();

static StorageManager_Status_t GetPartitionById(StorageManager_Id_t id, StorageManager_Partition_t **partition);
static StorageManager_Status_t GetPartitionById_InternalFlash(StorageManager_Id_t id, StorageManager_Partition_t **partition);
static StorageManager_Status_t GetPartitionById_SRAM2(StorageManager_Id_t id, StorageManager_Partition_t **partition);

static StorageManager_Status_t GetPartitionFreeSpace_InternalFlash(StorageManager_Partition_t *partition, size_t *size);
static StorageManager_Status_t GetPartitionFreeSpace_SRAM2(StorageManager_Partition_t *partition, size_t *size);

static StorageManager_Status_t CalculatePartitionBytes_InternalFlash(StorageManager_Partition_t *partition, size_t *size);
static StorageManager_Status_t CalculatePartitionBytes_SRAM2(StorageManager_Partition_t *partition, size_t *size);

static StorageManager_Status_t Load_InternalFlash(StorageManager_Partition_t *partition, size_t offset, size_t size, uint8_t *data);
static StorageManager_Status_t Load_SRAM2(StorageManager_Partition_t *partition, size_t offset, size_t size, uint8_t *data);

static StorageManager_Status_t Store_InternalFlash(StorageManager_Partition_t *partition, uint8_t *data, size_t size);
static StorageManager_Status_t Store_SRAM2(StorageManager_Partition_t *partition, uint8_t *data, size_t size);

static StorageManager_Status_t FreeSpace_InternalFlash(StorageManager_Partition_t *partition, size_t size);
static StorageManager_Status_t FreeSpace_SRAM2(StorageManager_Partition_t *partition, size_t size);

static StorageManager_Status_t Delete_InternalFlash(StorageManager_Partition_t *partition, size_t size);

#if EXTERNAL_FLASH_USE == 1
static StorageManager_Status_t Check_ExternalFlash();
static StorageManager_Status_t GetPartitionById_ExternalFlash(StorageManager_Id_t id, StorageManager_Partition_t **partition);
static StorageManager_Status_t GetPartitionFreeSpace_ExternalFlash(StorageManager_Partition_t *partition, size_t *size);
static StorageManager_Status_t Load_ExternalFlash(StorageManager_Partition_t *partition, size_t offset ,size_t size, uint8_t *data);
static StorageManager_Status_t Store_ExternalFlash(StorageManager_Partition_t *partition, uint8_t *data, size_t size);
static StorageManager_Status_t FreeSpace_ExternalFlash(StorageManager_Partition_t *partition, size_t size);
static StorageManager_Status_t Delete_ExternalFlash(StorageManager_Partition_t *partition, size_t size);
#endif

static inline uint8_t *_align_addr(uint8_t *addr);
static inline uint8_t *_wrap_around(uint8_t *addr, uint8_t *base_addr, size_t size);

// Function definitions
StorageManager_Status_t StorageManager_Init(void){

	PRINT_SM("[SM]: Init - Start\r\n");
    if (Check_InternalFlash() != SM_STATUS_OK){
    	PRINT_SM("NOK, failed check of internal flash\r\n");
        return SM_STATUS_NOK;
    }

#if EXTERNAL_FLASH_USE == 1
    if (Check_ExternalFlash() != SM_STATUS_OK){
    	PRINT_SM("NOK, failed check of external flash\r\n");
		return SM_STATUS_NOK;
	}
#endif

    if (Check_SRAM2() != SM_STATUS_OK){
    	PRINT_SM("NOK, failed check of ram\r\n");
        return SM_STATUS_NOK;
    }
    PRINT_SM("OK\r\n\n");
    PRINT_SM("[SM]: Init - Finish\r\n");
    return SM_STATUS_OK;
}

static StorageManager_Status_t Check_InternalFlash(){
    uint8_t i;
    uint32_t last_addr;

    last_addr = FLASH_BASE + FLASH_SIZE - FLASH_PAGE_SIZE;
    for(i = 0; i < SM_IFlash_partitions_size; i++){
        // Check base address
        if (((uint32_t)SM_IFlash_partitions[i].base_addr < FLASH_BASE)
         || ((uint32_t)SM_IFlash_partitions[i].base_addr > last_addr)
         || ((uint32_t)SM_IFlash_partitions[i].base_addr & 0x000007FF)) {
        	PRINT_SM("Bad base address\r\n");
            return SM_STATUS_NOK;
        }
        // Check partition size
        if (SM_IFlash_partitions[i].size > FLASH_SIZE || (SM_IFlash_partitions[i].size & 0x000007FF)) {
        	PRINT_SM("Bad size\r\n");
            return SM_STATUS_NOK;
        }

        if (((uint32_t)SM_IFlash_partitions[i].base_addr + SM_IFlash_partitions[i].size) > FLASH_END){
			PRINT_SM("Bad partition exceeds memory bounds\r\n");
			return SM_STATUS_NOK;
		}
        // Check partition overlap or out of bounds
        if (i != (SM_IFlash_partitions_size -1)){
            if ((SM_IFlash_partitions[i].base_addr + SM_IFlash_partitions[i].size) > SM_IFlash_partitions[i + 1].base_addr){
            	PRINT_SM("Bad bounderies between partition %d and %d\r\n", i, i+1);
                return SM_STATUS_NOK;
            }
        }
    }
    return SM_STATUS_OK;
}

static StorageManager_Status_t Check_SRAM2(){
    uint8_t i;
    uint32_t last_addr;

    last_addr = SRAM2_BASE + SRAM2_SIZE -1;
    for(i = 0; i < SM_SRAM2_partitions_size; i++){
        // Check base address
        if (((uint32_t)SM_SRAM2_partitions[i].base_addr < SRAM2_BASE) || ((uint32_t)SM_SRAM2_partitions[i].base_addr > last_addr)) {
        	PRINT_SM("Bad base address\r\n");
        	return SM_STATUS_NOK;
        }
        // Check partition size
        if (SM_SRAM2_partitions[i].size > SRAM2_SIZE) {
        	PRINT_SM("Bad size\r\n");
        	return SM_STATUS_NOK;
        }

		if (((uint32_t)SM_SRAM2_partitions[i].base_addr + SM_SRAM2_partitions[i].size) > (SRAM2_BASE + SRAM2_SIZE)){
			PRINT_SM("Bad partition exceeds memory bounds\r\n");
			return SM_STATUS_NOK;
		}
        // Check partition overlap or out of bounds
        if (i != (SM_SRAM2_partitions_size - 1)){
            if ((SM_SRAM2_partitions[i].base_addr + SM_SRAM2_partitions[i].size) > SM_SRAM2_partitions[i + 1].base_addr){
            	PRINT_SM("Bad bounderies between partition %d and %d\r\n", i, i+1);
            	return SM_STATUS_NOK;
            }
        }
    }
    return SM_STATUS_OK;
}


StorageManager_Status_t StorageManager_GetPartitionFreeSpace(StorageManager_Id_t id, size_t *size){
    StorageManager_Partition_t *partition;
    StorageManager_Status_t ret;

    PRINT_SM("[SM]: Get Partition Free Space\r\n");
    if (GetPartitionById(id, &partition) != SM_STATUS_OK){
    	PRINT_SM("NOK, Failed to get partition by id: %02x\r\n\n", id);
        return SM_STATUS_NOK;
    }

    switch (id & 0xF0) {
        case 0x00:
        	PRINT_SM("[Internal Flash context]\r\n");
        	ret = GetPartitionFreeSpace_InternalFlash(partition, size);
        	break;
#if EXTERNAL_FLASH_USE == 1
        case 0x10:
        	PRINT_SM("[External Flash context]\r\n");
        	ret = GetPartitionFreeSpace_ExternalFlash(partition, size);
        	break;
#endif
        case 0x20:
        	PRINT_SM("[SRAM2 context]\r\n");
            ret = GetPartitionFreeSpace_SRAM2(partition, size);
            break;
        default:
        	PRINT_SM("[default context]\r\n");
            *size = 0;
            ret = SM_STATUS_NOK;
            break;
    }

#if STORAGE_MANAGER_DEBUG == 1
    if(ret == SM_STATUS_OK){
		PRINT_SM("OK, partition %02x free space: %d bytes, occupied: %d bytes\r\n\n", id, *size, partition->bytes_occupied);
	} else {
		PRINT_SM("NOK, Failed to get parition %02x free space\r\n\n", id);
	}
#endif
    return ret;
}

static StorageManager_Status_t GetPartitionFreeSpace_InternalFlash(StorageManager_Partition_t *partition, size_t *size){
	if (partition->full_flag){
		*size = 0;
	}

	if (partition->head <= partition->tail){
    	*size = partition->size - ((uint32_t)partition->tail - ((uint32_t)partition->head & ~(FLASH_PAGE_SIZE - 1)));

    } else {
    	*size = ((uint32_t)partition->head & ~(FLASH_PAGE_SIZE - 1)) - (uint32_t)partition->tail ;
    }

    // remove header size
    if (*size <= 8){
        	*size = 0;
	} else {
		*size -= 8;
	}
    return SM_STATUS_OK;
}

static StorageManager_Status_t GetPartitionFreeSpace_SRAM2(StorageManager_Partition_t *partition, size_t *size){
	if (partition->full_flag){
		*size = 0;
		return SM_STATUS_OK;
	}

	if (partition->head <= partition->tail){
		*size = partition->size - ((uint32_t)partition->tail - (uint32_t)partition->head);
    } else {
		*size = (uint32_t)partition->head - (uint32_t)partition->tail ;
	}
    return SM_STATUS_OK;
}


StorageManager_Status_t StorageManager_GetPartitionBytes(StorageManager_Id_t id, size_t *size){
	StorageManager_Partition_t *partition;

	PRINT_SM("[SM]: Get Partition Bytes - Start\r\n");
	if (GetPartitionById(id, &partition) != SM_STATUS_OK){
		PRINT_SM("NOK, Failed to get partition by id: %02x\r\n\n", id);
		PRINT_SM("[SM]: Get Partition Bytes - Finish\r\n");
		return SM_STATUS_NOK;
	}

	*size = partition->bytes_occupied;
	PRINT_SM("OK, bytes in partition %02x : %d bytes\r\n\n", id, *size);
	PRINT_SM("[SM]: Get Partition Bytes - Finish\r\n");
	return SM_STATUS_OK;
}

// DEPRECATED - START
StorageManager_Status_t StorageManager_CalculatePartitionBytes(StorageManager_Id_t id, size_t *size){
    StorageManager_Partition_t *partition;

    if (GetPartitionById(id, &partition) != SM_STATUS_OK){
        return SM_STATUS_NOK;
    }

    switch (id & 0xF0) {
        case 0x00:
            return CalculatePartitionBytes_InternalFlash(partition, size);
        case 0x20:
            return CalculatePartitionBytes_SRAM2(partition, size);
        default:
            *size = 0;
            return SM_STATUS_NOK;
    }
    return SM_STATUS_OK;
}

static StorageManager_Status_t CalculatePartitionBytes_InternalFlash(StorageManager_Partition_t *partition, size_t *size){
    size_t chunk_size = 0;
    size_t bytes_total = 0;
    size_t chunk_offset = partition->chunk_offset;
    uint8_t *addr;

    addr = partition->head;
    switch ((partition->head < partition->tail) - (partition->head > partition->tail)) {
    	case 0:
    		if (!partition->full_flag){
    			*size = 0;
    			return SM_STATUS_OK;
    		}
    	case -1:
    		while ((uint32_t)addr < ((uint32_t)partition->base_addr + partition->size)){
				// read chunk header to get chunk size
				if(Flash_Read(addr, (uint8_t *)&chunk_size, 4) != FLASH_STATUS_OK){
					return SM_STATUS_NOK;
				}
				addr += 8;

				// if chunk_size if the max value there is no chunk left to read
				if (chunk_size == 0xFFFFFFFF) {
					break;
				}

				bytes_total += chunk_size - chunk_offset;
				chunk_offset = 0;

				// the next chunk begins at the next 8-Byte aligned address
				addr = _align_addr(addr + chunk_size);
			}
			addr -= partition->size;
		case 1:
			while ((uint32_t)addr < (uint32_t)partition->tail){
				// read chunk header to get chunk size
				if(Flash_Read(addr, (uint8_t *)&chunk_size, 4) != FLASH_STATUS_OK){
					return SM_STATUS_NOK;
				}
				addr += 8;

				// if chunk_size if the max value there is no chunk left to read
				if (chunk_size == 0xFFFFFFFF) {
					break;
				}
				bytes_total += chunk_size - chunk_offset;
				chunk_offset = 0;

				// the next chunk begins at the next 8-Byte aligned address
				addr = _align_addr(addr + chunk_size);
			}
		default:
			break;
	}
    *size = bytes_total;
    return SM_STATUS_OK;
}

static StorageManager_Status_t CalculatePartitionBytes_SRAM2(StorageManager_Partition_t *partition, size_t *size){
	if (partition->head < partition->tail){
		*size = (uint32_t)partition->tail - (uint32_t)partition->head;
	} else if (partition->head == partition->tail){
		if (partition->full_flag){
			*size = partition->size;
		} else {
			*size = 0;
		}
	} else {
		*size = partition->size - ((uint32_t)partition->head - (uint32_t)partition->tail);
	}
	return SM_STATUS_OK;
}
//DEPRACATED - END

StorageManager_Status_t StorageManager_Load(StorageManager_Id_t id, size_t offset, size_t size, uint8_t *data){
    StorageManager_Partition_t *partition = NULL;
    StorageManager_Status_t ret;
    size_t bytes_in_memory = 0;

    PRINT_SM("[SM]: Load partition\r\n");
    if(!data || !size){
    	PRINT_SM("NOK, invalid data pointer: %p or size: %lu\r\n\n", data, size);
        return SM_STATUS_NOK;
    }

    if (GetPartitionById(id, &partition) != SM_STATUS_OK){
    	PRINT_SM("NOK, Failed to get partition by id: %02x\r\n\n", id);
        return SM_STATUS_NOK;
    }

    if (StorageManager_GetPartitionBytes(id, &bytes_in_memory) != SM_STATUS_OK) {
    	PRINT_SM("NOK, Failed to get partition bytes for id: %02x\r\n\n", id);
        return SM_STATUS_NOK;
    }

    if (!bytes_in_memory || (offset >= bytes_in_memory)){
    	PRINT_SM("NOK, Partition %02x is empty or offset:%lu exceeds bytes in memory:%lu\r\n\n", id, offset, bytes_in_memory);
        return SM_STATUS_NOK;
    }

    bytes_in_memory -= offset;
    if (size > bytes_in_memory){
		PRINT_SM("Warning, requested size: %lu, exceeds bytes in memory, loading only %lu bytes\r\n", size, bytes_in_memory);
		size = bytes_in_memory;
	}

    switch (id & 0xF0) {
        case 0x00:
        	PRINT_SM("[Internal Flash context]\r\n");
            ret = Load_InternalFlash(partition, offset, size, data);
            break;
#if EXTERNAL_FLASH_USE == 1
        case 0x10:
        	PRINT_SM("[External Flash context]\r\n");
        	ret = Load_ExternalFlash(partition, offset, size, data);
        	break;
#endif
        case 0x20:
        	PRINT_SM("[SRAM2 context]\r\n");
            ret = Load_SRAM2(partition, offset, size, data);
            break;
        default:
        	PRINT_SM("[default context]\r\n");
            ret = SM_STATUS_NOK;
            break;
    }

#if STORAGE_MANAGER_DEBUG == 1
    if(ret == SM_STATUS_OK){
		PRINT_SM("OK, loaded %lu bytes from partition %02x with offset %lu\r\n\n", size, id, offset);
	} else {
		PRINT_SM("NOK, Failed to load partition %02x with offset %lu and size %lu\r\n\n", id, offset, size);
	}
#endif

    return ret;
}

static StorageManager_Status_t Load_InternalFlash(StorageManager_Partition_t *partition, size_t offset, size_t size, uint8_t *data){
    size_t chunk_size = 0;
    size_t bytes_2_read = 0;
    size_t bytes_2_read_temp = 0;
    size_t chunk_offset = partition->chunk_offset;
    uint8_t *addr;

    addr = partition->head;
    while (offset){
		// read chunk header to get chunk size
		if(Flash_Read(addr, (uint8_t *)&chunk_size, 4) != FLASH_STATUS_OK){
			PRINT_SM("Bad read of chunk header at addr:%p, in  offset parsing\r\n", addr);
			return SM_STATUS_NOK;
		}

		// if chunk_size if the max value there is no chunk left to read
		if (chunk_size == 0xFFFFFFFF) {
			PRINT_SM("Bad chunk found at addr:%p, in offset parsing\r\n", addr);
			return SM_STATUS_NOK;
		}

		// check if the chunk is bigger than the bytes left to be read, read the smallest amount of bytes
		if (chunk_size > (offset + chunk_offset)){
			chunk_offset += offset;
			break;
		}
		else {
			// advance addr 8 bytes for the header of current chunk and chunk_size
			addr += chunk_size + 8;
			addr = _wrap_around(_align_addr(addr), partition->base_addr, partition->size);
			offset -= (chunk_size - chunk_offset);
			chunk_offset = 0;
		}
    }

    while (size){
        // read chunk header to get chunk size
        if(Flash_Read(addr, (uint8_t *)&chunk_size, 4) != FLASH_STATUS_OK){
        	PRINT_SM("Bad read of chunk header at addr:%p, in data parsing\r\n", addr);
            return SM_STATUS_NOK;
        }
        addr += 8;
        addr = _wrap_around(addr, partition->base_addr, partition->size);

        // if chunk_size if the max value there is no chunk left to read
        if (chunk_size == 0xFFFFFFFF) {
        	PRINT_SM("Bad chunk found at addr:%p, in data parsing\r\n", addr);
            return SM_STATUS_NOK;
        }

        if(chunk_offset){
			addr += chunk_offset;
			addr = _wrap_around(addr, partition->base_addr, partition->size);
			chunk_size -= chunk_offset;
			chunk_offset = 0;
		}

        // check if the chunk is bigger than the bytes left to be read, read the smallest amount of bytes
        if (chunk_size > size)
            bytes_2_read = size;
        else
            bytes_2_read = chunk_size;

        // if the chunk crosses the partition boundary do the wrap around
		if ((uint32_t)addr + bytes_2_read > ((uint32_t)partition->base_addr + partition->size)){

			bytes_2_read_temp = ((uint32_t)partition->base_addr + partition->size) - (uint32_t)addr;

			if(Flash_Read(addr, data, bytes_2_read_temp) != FLASH_STATUS_OK){
				PRINT_SM("Bad read of chunk data, in data parsing with wrap around at addr:%p\r\n", addr);
				return SM_STATUS_NOK;
			}
			data += bytes_2_read_temp;
			size -= bytes_2_read_temp;

			bytes_2_read -= bytes_2_read_temp;
			addr = partition->base_addr;
		}

		if(Flash_Read(addr, data, bytes_2_read) != FLASH_STATUS_OK){
			PRINT_SM("Bad read of chunk data, in data parsing (no wrap) at addr:%p\r\n", addr);
			return SM_STATUS_NOK;
		}
		data += bytes_2_read;
		size -= bytes_2_read;

		// the next chunk begins at the next 8-Byte aligned address
		addr += bytes_2_read;
		addr = _wrap_around(_align_addr(addr), partition->base_addr, partition->size);
	}
    return SM_STATUS_OK;
}

static StorageManager_Status_t Load_SRAM2(StorageManager_Partition_t *partition, size_t offset ,size_t size, uint8_t *data){
    uint8_t *addr;
    size_t size_temp;

    addr = partition->head;

    if (((uint32_t)addr + offset) >= ((uintptr_t)partition->base_addr + partition->size)){
		addr = _wrap_around((uint8_t *)addr + offset, partition->base_addr, partition->size);
	} else {
		addr += offset;
	}

    if ((uint32_t)addr + size > ((uint32_t)partition->base_addr + (uint32_t)partition->size)){
    	size_temp = (uint32_t)partition->base_addr + partition->size - (uint32_t)addr;
		memcpy(data, addr, size_temp);
		data += size_temp;
		size -= size_temp;
		addr = partition->base_addr;
    }

    memcpy(data, addr, size);

    return SM_STATUS_OK;
}


StorageManager_Status_t StorageManager_Store(StorageManager_Id_t id, uint8_t *data, size_t size){
    StorageManager_Partition_t *partition;
    size_t free_bytes = 0;
    StorageManager_Status_t ret;

    PRINT_SM("[SM]: Store - Start\r\n");
    PRINT_SM("Id: %02x, data: %p, size: %lu (bytes)\r\n", id, data, size);
    if (!data || !size) {
    	PRINT_SM("NOK, invalid data pointer: %p or size: %lu\r\n\n", data, size);
        return SM_STATUS_NOK;
    }

    if (GetPartitionById(id, &partition) != SM_STATUS_OK){
    	PRINT_SM("NOK, Failed to get partition by id: %02x\r\n\n", id);
        return SM_STATUS_NOK;
    }

    if (StorageManager_GetPartitionFreeSpace(id, &free_bytes) != SM_STATUS_OK) {
    	PRINT_SM("NOK, Failed to get partition free space for id: %02x\r\n\n", id);
        return SM_STATUS_NOK;
    }

    if (size > free_bytes){
    	PRINT_SM("NOK, Not enough free space in partition %02x, required: %lu bytes, free space: %lu bytes\r\n\n", id, size, free_bytes);
		return SM_STATUS_NOK;
	}

    switch (id & 0xF0) {
        case 0x00:
        	PRINT_SM("[Internal Flash context]\r\n");
            ret = Store_InternalFlash(partition, data, size);
            if (ret == SM_STATUS_OK){
            	partition->bytes_occupied += size;
            	if(((uintptr_t)partition->head & 0xFFFFF800) == (uintptr_t)partition->tail){
					partition->full_flag = 1;
				}
			}
            break;
#if EXTERNAL_FLASH_USE == 1
        case 0x10:
        	PRINT_SM("[External Flash context]\r\n");
			ret = Store_ExternalFlash(partition, data, size);
			if (ret == SM_STATUS_OK){
				partition->bytes_occupied += size;
				if(((uintptr_t)partition->head & 0xFFFFF000) == (uintptr_t)partition->tail){
					partition->full_flag = 1;
				}
			}
			break;
#endif
        case 0x20:
        	PRINT_SM("[SRAM2 context]\r\n");
            ret = Store_SRAM2(partition, data, size);
            if (ret == SM_STATUS_OK){
				partition->bytes_occupied += size;
				if(partition->head == partition->tail){
					partition->full_flag = 1;
				}
			}
            break;
        default:
        	PRINT_SM("[default context]\r\n");
            ret = SM_STATUS_NOK;
            break;
    }

#if STORAGE_MANAGER_DEBUG == 1
	if(ret == SM_STATUS_OK){
		PRINT_SM("OK, stored %lu bytes to partition %02x\r\n\n", size, id);
	} else {
		PRINT_SM("NOK, Failed to store  %lu bytes to partition %02x\r\n\n", size, id);
	}
	PRINT_SM("[SM]: Store - Finish\r\n");
#endif
    return ret;
}

static StorageManager_Status_t Store_InternalFlash(StorageManager_Partition_t *partition, uint8_t *data, size_t size){
    uint8_t *addr;
    size_t size_temp;
    addr = partition->tail;

    if (Delete_InternalFlash(partition, size) != SM_STATUS_OK){
    	PRINT_SM("Bad Deleting of Flash pages\r\n");
		return SM_STATUS_NOK;
	}

    //write header for this chunk of data it has to use 8bytes but only has 4bytes of meaning
    if (Flash_Write(addr, (uint8_t *)&size, 4) != FLASH_STATUS_OK){
    	PRINT_SM("Bad write of chunk header at addr:%p\r\n", addr);
		return SM_STATUS_NOK;
	}
    addr += 8;
    if ((uintptr_t)addr >= ((uintptr_t)partition->base_addr + partition->size)){
    	addr = partition->base_addr;
    	partition->tail = addr;
	}

    switch ((partition->head < partition->tail) - (partition->head > partition->tail)) {
		case 0:
		case 1:
			// de tail ate fim
			// if the chunk crosses the partition boundary do the wrap around
			if (((uintptr_t)addr + size) > ((uintptr_t)partition->base_addr + partition->size)){
				size_temp = ((uintptr_t)partition->base_addr + partition->size) - (uintptr_t)addr;
				if (Flash_Write(addr, data, size_temp) != FLASH_STATUS_OK){
					PRINT_SM("Bad write of chunk data, in data writing with wrap around at addr:%p\r\n", addr);
					return SM_STATUS_NOK;
				}
				addr = partition->base_addr;
				size -= size_temp;
				data += size_temp;
			}

		case -1:
			// de addr ate head
			if (Flash_Write(addr, data, size) != FLASH_STATUS_OK){
				PRINT_SM("Bad write of chunk data, in data writing (no wrap) at addr:%p\r\n", addr);
				return SM_STATUS_NOK;
			}
		default:
			break;
	}

    partition->tail = _wrap_around(_align_addr(addr + size), partition->base_addr, partition->size);
    return SM_STATUS_OK;
}

static StorageManager_Status_t Store_SRAM2(StorageManager_Partition_t *partition, uint8_t *data, size_t size){
	uint8_t *addr;
	size_t size_temp;
	addr = partition->tail;

	switch ((partition->head < partition->tail) - (partition->head > partition->tail)) {
		case 0:
			memcpy(partition->base_addr, data, size);
			partition->head = partition->base_addr;
			partition->tail = partition->base_addr + size;
			return SM_STATUS_OK;
		case 1:
			// de tail ate fim
			// if the chunk crosses the partition boundary do the wrap around
			if (((uintptr_t)addr + size) > ((uintptr_t)partition->base_addr + partition->size)){
				size_temp = ((uintptr_t)partition->base_addr + partition->size) - (uintptr_t)addr;
				memcpy(addr, data, size_temp);
				addr = partition->base_addr;
				size -= size_temp;
				data += size_temp;
			}

		case -1:
			// de addr ate head
			memcpy(addr, data, size);
		default:
			break;
	}

	partition->tail = addr + size;
    return SM_STATUS_OK;
}


StorageManager_Status_t StorageManager_FreeSpace(StorageManager_Id_t id, size_t size){
	StorageManager_Partition_t *partition;
	StorageManager_Status_t ret;

	PRINT_SM("[SM]: Free space in partition\r\n");
	if (!size) {
		PRINT_SM("NOK, invalid size: %lu\r\n\n", size);
		return SM_STATUS_NOK;
	}

	if (GetPartitionById(id, &partition) != SM_STATUS_OK){
		PRINT_SM("NOK, Failed to get partition by id: %02x\r\n\n", id);
		return SM_STATUS_NOK;
	}

	switch (id & 0xF0) {
		case 0x00:
			PRINT_SM("[Internal Flash context]\r\n");
			ret = FreeSpace_InternalFlash(partition, size);
			break;
#if EXTERNAL_FLASH_USE == 1
		case 0x10:
			PRINT_SM("[External Flash context]\r\n");
			ret = FreeSpace_ExternalFlash(partition, size);
			break;
#endif
		case 0x20:
			PRINT_SM("[SRAM2 context]\r\n");
			ret = FreeSpace_SRAM2(partition, size);
			break;
		default:
			PRINT_SM("[default contect]\r\n");
			ret = SM_STATUS_NOK;
	}

#if STORAGE_MANAGER_DEBUG == 1
	if(ret == SM_STATUS_OK){
		PRINT_SM("OK, freed %lu bytes from partition %02x\r\n\n", size, id);
	} else {
		PRINT_SM("NOK, Failed to free %lu bytes from partition %02x\r\n\n", size, id);
	}
#endif
	return ret;
}

static StorageManager_Status_t FreeSpace_InternalFlash(StorageManager_Partition_t *partition, size_t size){
	uint8_t *addr;
	size_t chunk_size;

	// if going to free all, just reset the partition
	if (size >= partition->bytes_occupied){
		partition->tail = _wrap_around((uint8_t *)(((uintptr_t)partition->tail & 0xFFFFF800) + FLASH_PAGE_SIZE), partition->base_addr, partition->size);
		partition->head = partition->tail;
		partition->chunk_offset = 0;
		partition->full_flag = 0;
		partition->bytes_occupied = 0;
		return SM_STATUS_OK;
	}

	// loop through chunks until we free the requested size, only free chunks that are fully freed, if a chunk is partially freed update the chunk offset
	addr = partition->head;
	while (size){
		// read chunk header to get chunk size
		if(Flash_Read(addr, (uint8_t *)&chunk_size, 4) != FLASH_STATUS_OK){
			PRINT_SM("Bad read of chunk header at addr:%p\r\n", addr);
			return SM_STATUS_NOK;
		}
		addr += 8;
		//free chunk by chunk, only free chunks that are fully freed.
		if (chunk_size == 0xFFFFFFFF) {
			PRINT_SM("Bad chunk found at addr:%p\r\n", addr);
			return SM_STATUS_NOK;
		}

		chunk_size -= partition->chunk_offset;
		addr += partition->chunk_offset;

		if (size >= chunk_size){
			addr = _wrap_around(_align_addr(addr + chunk_size), partition->base_addr, partition->size);
			partition->head = addr;
			partition->chunk_offset = 0;
			size -= chunk_size;
			partition->bytes_occupied -= chunk_size;
		} else {
			partition->chunk_offset += size;
			partition->bytes_occupied -= size;
			break;
		}
	}
	return SM_STATUS_OK;
}

static StorageManager_Status_t FreeSpace_SRAM2(StorageManager_Partition_t *partition, size_t size){
	uint8_t *addr;

	if (size >= partition->bytes_occupied){
		partition->head = partition->base_addr;
		partition->tail = partition->base_addr;
		partition->full_flag = 0;
		partition->bytes_occupied = 0;
		return SM_STATUS_OK;
	}

	addr = partition->head;
	partition->head = _wrap_around(addr + size, partition->base_addr, partition->size);
	partition->bytes_occupied -= size;
	return SM_STATUS_OK;
}


static StorageManager_Status_t GetPartitionById(StorageManager_Id_t id, StorageManager_Partition_t **partition){
    switch (id & 0xF0) {
        case 0x00:
            return GetPartitionById_InternalFlash(id, partition);
#if EXTERNAL_FLASH_USE == 1
        case 0x10:
        	return GetPartitionById_ExternalFlash(id, partition);
#endif
        case 0x20:
            return GetPartitionById_SRAM2(id, partition);
        default:
            return SM_STATUS_NOK;
    }
}

static StorageManager_Status_t GetPartitionById_InternalFlash(StorageManager_Id_t id, StorageManager_Partition_t **partition){
    for (uint8_t i = 0; i < SM_IFlash_partitions_size; i++){
        if (SM_IFlash_partitions[i].id == id){
            *partition = &(SM_IFlash_partitions[i]);
            return SM_STATUS_OK;
        }
    }
    PRINT_SM("Partition with id: %02x not found in Internal Flash\r\n", id);
    return SM_STATUS_NOK;
}

static StorageManager_Status_t GetPartitionById_SRAM2(StorageManager_Id_t id, StorageManager_Partition_t **partition){
    for (uint8_t i = 0; i < SM_SRAM2_partitions_size; i++){
        if (SM_SRAM2_partitions[i].id == id){
            *partition = &(SM_SRAM2_partitions[i]);
            return SM_STATUS_OK;
        }
    }
    PRINT_SM("Partition with id: %02x not found in SRAM2\r\n", id);
    return SM_STATUS_NOK;
}


static StorageManager_Status_t Delete_InternalFlash(StorageManager_Partition_t *partition, size_t size){
	uint8_t *addr_begin = NULL;
	size_t true_size = size + 8; // header size

	if (!((uintptr_t)partition->tail & 0x000007FF)){
		addr_begin = partition->tail;
	} else {
		if ((((uintptr_t)partition->tail & 0x000007FF) + true_size) > FLASH_PAGE_SIZE){
			true_size -= (FLASH_PAGE_SIZE - ((uintptr_t)partition->tail & 0x000007FF));
			addr_begin = _wrap_around((uint8_t *)(((uintptr_t)partition->tail & 0xFFFFF800) + FLASH_PAGE_SIZE), partition->base_addr, partition->size);
		} else {
			return SM_STATUS_OK;
		}
	}

	if (((uintptr_t)addr_begin + true_size) > ((uintptr_t)partition->base_addr + partition->size)){
		size = ((uintptr_t)partition->base_addr + partition->size) - (uintptr_t)addr_begin;
		if (Flash_Erase(addr_begin, size) != FLASH_STATUS_OK){
			PRINT_SM("Bad erase of Flash pages, in delete with wrap around at addr:%p, size:%lu\r\n", addr_begin, size);
			return SM_STATUS_NOK;
		}
		addr_begin = partition->base_addr;
		true_size -= size;
	}
	if (Flash_Erase(addr_begin, true_size) != FLASH_STATUS_OK){
		PRINT_SM("Bad erase of Flash pages, in delete at addr:%p, size:%lu\r\n", addr_begin, true_size);
		return SM_STATUS_NOK;
	}

	return SM_STATUS_OK;
}

#if EXTERNAL_FLASH_USE == 1
static StorageManager_Status_t Check_ExternalFlash(){
    uint8_t i;
    uint32_t last_addr;

    last_addr = W25Q_MAX_MEM_SIZE - W25Q_SECTOR_SIZE;
    for(i = 0; i < SM_IFlash_partitions_size; i++){
        // Check base address
        if (((uint32_t)SM_EFlash_partitions[i].base_addr > last_addr)
         || ((uint32_t)SM_EFlash_partitions[i].base_addr & 0x00000FFF)) {
        	PRINT_SM("Bad base address\r\n");
            return SM_STATUS_NOK;
        }
        // Check partition size
        if (SM_EFlash_partitions[i].size > W25Q_MAX_MEM_SIZE || (SM_EFlash_partitions[i].size & 0x00000FFF)) {
        	PRINT_SM("Bad size\r\n");
            return SM_STATUS_NOK;
        }

		if (((uint32_t)SM_EFlash_partitions[i].base_addr + SM_EFlash_partitions[i].size) > W25Q_MAX_MEM_SIZE){
			PRINT_SM("Bad partition exceeds memory bounds\r\n");
			return SM_STATUS_NOK;
		}
        // Check partition overlap or out of bounds
        if (i != (SM_EFlash_partitions_size -1)){
            if ((SM_EFlash_partitions[i].base_addr + SM_EFlash_partitions[i].size) > SM_EFlash_partitions[i + 1].base_addr){
            	PRINT_SM("Bad bounderies between partition %d and %d\r\n", i, i+1);
            	return SM_STATUS_NOK;
            }
        }
    }
    return SM_STATUS_OK;
}

static StorageManager_Status_t GetPartitionFreeSpace_ExternalFlash(StorageManager_Partition_t *partition, size_t *size){
	if (partition->full_flag){
		*size = 0;
		return SM_STATUS_OK;
	}

	if (partition->head <= partition->tail){
    	*size = partition->size - ((uint32_t)partition->tail - ((uint32_t)partition->head & ~(W25Q_SECTOR_SIZE - 1)));

    } else {
    	*size = ((uint32_t)partition->head & ~(W25Q_SECTOR_SIZE - 1)) - (uint32_t)partition->tail ;
    }

    return SM_STATUS_OK;
}

static StorageManager_Status_t Load_ExternalFlash(StorageManager_Partition_t *partition, size_t offset ,size_t size, uint8_t *data){
    uint8_t *addr;
    size_t size_temp;

    addr = partition->head;

    if (((uint32_t)addr + offset) >= ((uintptr_t)partition->base_addr + partition->size)){
		addr = _wrap_around((uint8_t *)addr + offset, partition->base_addr, partition->size);
	} else {
		addr += offset;
	}

    if ((uint32_t)addr + size > ((uint32_t)partition->base_addr + (uint32_t)partition->size)){
    	size_temp = (uint32_t)partition->base_addr + partition->size - (uint32_t)addr;
		if (W25Q_Read(addr, size_temp, data) != W25Q_OK){
			PRINT_SM("Bad read of chunk data, in data parsing with wrap around at addr:%p\r\n", addr);
			return SM_STATUS_NOK;
		}
		data += size_temp;
		size -= size_temp;
		addr = partition->base_addr;
    }

    if (W25Q_Read(addr, size, data) != W25Q_OK){
    	PRINT_SM("Bad read of chunk data, in data parsing (no wrap) at addr:%p\r\n", addr);
		return SM_STATUS_NOK;
	}

    return SM_STATUS_OK;
}

static StorageManager_Status_t Store_ExternalFlash(StorageManager_Partition_t *partition, uint8_t *data, size_t size){
    uint8_t *addr;
    size_t size_temp;
    addr = partition->tail;

    if (Delete_ExternalFlash(partition, size) != SM_STATUS_OK){
    	PRINT_SM("External Flash, Bad Deleting of Flash pages\r\n");
		return SM_STATUS_NOK;
	}

    switch ((partition->head < partition->tail) - (partition->head > partition->tail)) {
		case 0:
		case 1:
			// de tail ate fim
			// if the chunk crosses the partition boundary do the wrap around
			if (((uintptr_t)addr + size) > ((uintptr_t)partition->base_addr + partition->size)){
				size_temp = ((uintptr_t)partition->base_addr + partition->size) - (uintptr_t)addr;
				if (W25Q_Write_Page(addr, size_temp, data) != W25Q_OK){
					PRINT_SM("Bad write of chunk data, in data writing with wrap around at addr:%p, size:%lu\r\n", addr, size_temp);
					return SM_STATUS_NOK;
				}
				addr = partition->base_addr;
				size -= size_temp;
				data += size_temp;
			}

		case -1:
			// de addr ate head
			if (W25Q_Write_Page(addr, size, data) != W25Q_OK){
				PRINT_SM("Bad write of chunk data, in data writing (no wrap) at addr:%p, size:%lu\r\n", addr, size);
				return SM_STATUS_NOK;
			}
		default:
			break;
	}

    partition->tail = _wrap_around(addr + size, partition->base_addr, partition->size);

    return SM_STATUS_OK;
}

static StorageManager_Status_t FreeSpace_ExternalFlash(StorageManager_Partition_t *partition, size_t size){
	uint8_t *addr;

	if (size >= partition->bytes_occupied){
		partition->tail = _wrap_around((uint8_t *)((uintptr_t)partition->tail & 0xFFFFF000) + W25Q_SECTOR_SIZE, partition->base_addr, partition->size);
		partition->head = partition->tail;
		partition->full_flag = 0;
		partition->bytes_occupied = 0;
		return SM_STATUS_OK;
	}

	addr = partition->head;
	partition->head = _wrap_around(addr + size, partition->base_addr, partition->size);
	partition->bytes_occupied -= size;

	return SM_STATUS_OK;
}

static StorageManager_Status_t GetPartitionById_ExternalFlash(StorageManager_Id_t id, StorageManager_Partition_t **partition){
    for (uint8_t i = 0; i < SM_EFlash_partitions_size; i++){
        if (SM_EFlash_partitions[i].id == id){
            *partition = &(SM_EFlash_partitions[i]);
            return SM_STATUS_OK;
        }
    }
    PRINT_SM("Partition with id: %02x not found in External Flash\r\n", id);
    return SM_STATUS_NOK;
}

static StorageManager_Status_t Delete_ExternalFlash(StorageManager_Partition_t *partition, size_t size){
	uint8_t *addr_begin = NULL;

	if (!((uintptr_t)partition->tail & ~W25Q_SECTOR_SIZE)){
		addr_begin = partition->tail;
	} else {
		if ((((uintptr_t)partition->tail & ~W25Q_SECTOR_SIZE) + size) > W25Q_SECTOR_SIZE){
			addr_begin = _wrap_around((((uintptr_t)partition->tail & ~(W25Q_SECTOR_SIZE - 1)) + W25Q_SECTOR_SIZE), partition->base_addr, partition->size);
			size -= ((uintptr_t)addr_begin - (uintptr_t)partition->tail);
		} else {
			return SM_STATUS_OK;
		}
	}

	if (((uintptr_t)addr_begin + size) > ((uintptr_t)partition->base_addr + partition->size)){
		if (W25Q_Erase(addr_begin,((uintptr_t)partition->base_addr + partition->size) - (uintptr_t)addr_begin) != W25Q_OK){
			PRINT_SM("Bad erase of Flash pages, in delete with wrap around at addr:%p, size:%lu\r\n", addr_begin, ((uintptr_t)partition->base_addr + partition->size) - (uintptr_t)addr_begin);
			return SM_STATUS_NOK;
		}
		addr_begin = partition->base_addr;
		size -= (((uintptr_t)partition->base_addr + partition->size) - (uintptr_t)addr_begin);
	}
	if (W25Q_Erase(addr_begin, size) != W25Q_OK){
		PRINT_SM("Bad erase of Flash pages, in delete at addr:%p, size:%lu\r\n", addr_begin, size);
		return SM_STATUS_NOK;
	}

	return SM_STATUS_OK;
}
#endif

static inline uint8_t *_align_addr(uint8_t *addr){
	return (uint8_t *)(((uintptr_t)addr + 0x07) & ~(uintptr_t)7);
}

static inline uint8_t *_wrap_around(uint8_t *addr, uint8_t *base_addr, size_t size)
{
    uint8_t *end = base_addr + size;

    if (addr >= end)
        addr -= size;

    return addr;
}

















