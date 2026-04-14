/*
 * app_flash.c
 *
 *  Created on: Nov 21, 2025
 *      Author: afonsofernandes
 */

#include "flash_interface.h"

#include <string.h>

Flash_Interface_Status_t Flash_Erase(uint8_t *addr, size_t nbytes)
{

	FLASH_EraseInitTypeDef eraseInit = {0};
	uint32_t pageError = 0;
    uint32_t nPages = 0;
    uint8_t is_blank = 0;

    if (!addr || !nbytes){
        PRINT_DBG("ERROR:Erasing Flash memory, Address:%p , Nbytes:%u\r\n", addr, nbytes);
        return FLASH_STATUS_NOK;
    }

    if ((uint32_t)addr < FLASH_BASE || (uint32_t)addr >= FLASH_END){
        PRINT_DBG("ERROR:Erasing Flash memory, Address:%p outside of flash range\r\n", addr);
        return FLASH_STATUS_NOK;
    }

    if (nbytes > (FLASH_END - (uint32_t)addr)){
        PRINT_DBG("ERROR:Erasing Flash memory, Address:%p , Nbytes:%u, overflow of flash memory\r\n", addr, nbytes);
        return FLASH_STATUS_NOK;
    }

    if (((uint32_t)addr & 0x07FF)) {
        PRINT_DBG("ERROR:Erasing Flash memory, Address = %p is not the beginning of a flash page.\r\n", addr);
        return FLASH_STATUS_BAD_ADDR;
    }

    nPages += (nbytes >> 11) + ((nbytes & 0x07FF) ? 1 : 0);

	eraseInit.TypeErase   = FLASH_TYPEERASE_PAGES;
	eraseInit.Page        = ((uint32_t)addr - FLASH_BASE) / FLASH_PAGE_SIZE;
	eraseInit.NbPages     = 1;

	HAL_FLASH_Unlock();

	for(uint8_t i = 0; i < nPages; i++ , eraseInit.Page++, addr += FLASH_PAGE_SIZE) {
        Flash_IsPageBlank(addr, &is_blank);
        if (is_blank) {
            PRINT_DBG("Flash Page: %lu is already blank. Skipping erase.\r\n", eraseInit.Page);
            continue;
        }
        if (HAL_FLASHEx_Erase(&eraseInit, &pageError) != HAL_OK)
        {
            // Handle error
            PRINT_DBG("ERROR: Flash could not be erased. error:0x%lx\r\n", pageError);
            HAL_FLASH_Lock();
            return FLASH_STATUS_NOK;
        }
        PRINT_DBG("Erased Flash Page: %lu\r\n", eraseInit.Page);
    }
	HAL_FLASH_Lock();
	return FLASH_STATUS_OK;
}

Flash_Interface_Status_t Flash_Write(uint8_t *addr, uint8_t *data, size_t nbytes)
{
	uint64_t value = 0;
	size_t nWords = 0;

	if (!addr || !data || !nbytes){
        PRINT_DBG("ERROR:Writing Flash memory, Address:%p, Data:%p , Nbytes:%u\r\n", addr, data, nbytes);
        return FLASH_STATUS_NOK;
    }

    if ((uint32_t)addr < FLASH_BASE || (uint32_t)addr >= FLASH_END){
        PRINT_DBG("ERROR:Writing Flash memory, Address:%p outside of flash range\r\n", addr);
        return FLASH_STATUS_NOK;
    }

    if ( nbytes > (FLASH_END - (uint32_t)addr)){
        PRINT_DBG("ERROR:Writing Flash memory, Address:%p , Nbytes:%u, overflow of flash memory\r\n", addr, nbytes);
        return FLASH_STATUS_NOK;
    }

	if (((uint32_t)addr & 0x07) != 0) {
		PRINT_DBG("ERROR: Writing to Flash memory, Address = %p is not aligned.\r\n", addr);
		return FLASH_STATUS_BAD_ADDR;
	}

	nWords += (nbytes >> 3);

	HAL_FLASH_Unlock();

	for (int i = 0; i < nWords ;i++) {
		memcpy(&value, data ,8);

		if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, (uint32_t)addr, value) != HAL_OK) {
			PRINT_DBG("ERROR: Writing to Flash memory failed.\r\n");
			HAL_FLASH_Lock();
			return FLASH_STATUS_NOK;
		}
		data += 8;
		addr += 8;
	}

	value = 0x0000000000000000;
	if (nbytes & 0x07) {
	    memcpy(&value, data , nbytes & 0x07);
	    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, (uint32_t)addr, value) != HAL_OK) {
            PRINT_DBG("ERROR: Writing to Flash memory failed.\r\n");
            HAL_FLASH_Lock();
            return FLASH_STATUS_NOK;
        }
	}

	HAL_FLASH_Lock();
	PRINT_DBG("Finished Writing to Flash Memory.\r\n");
	return FLASH_STATUS_OK;
}

Flash_Interface_Status_t Flash_Read(uint8_t *addr, uint8_t *data, size_t nbytes)
{
    size_t nWords, nBytes_align;

    if (!addr || !data || !nbytes || (uint32_t)addr >= FLASH_END || (uint32_t)addr < FLASH_BASE || (uint32_t)(addr + nbytes) > FLASH_END){
        PRINT_DBG("ERROR: Reading from Flash memory, Addr:%p , Data_Addr:%p , Nbytes:%d\r\n", addr, data, nbytes);
        return FLASH_STATUS_NOK;
    }

    if (!addr || !data || !nbytes){
        PRINT_DBG("ERROR:Reading from Flash memory, Address:%p, Data:%p , Nbytes:%u\r\n", addr, data, nbytes);
        return FLASH_STATUS_NOK;
    }

    if ((uint32_t)addr < FLASH_BASE || (uint32_t)addr >= FLASH_END){
        PRINT_DBG("ERROR:Reading from Flash memory, Address:%p outside of flash range\r\n", addr);
        return FLASH_STATUS_NOK;
    }

    if (nbytes > (FLASH_END - (uint32_t)addr)){
        PRINT_DBG("ERROR:Reading from Flash memory, Address:%p , Nbytes:%u, overflow of flash memory\r\n", addr, nbytes);
        return FLASH_STATUS_NOK;
    }

    nBytes_align = (4 - ((uint32_t)addr & 0x00000003)) & 0x00000003;
    if (nBytes_align > nbytes)
        nBytes_align = nbytes;

    nbytes -= nBytes_align;
    for(uint32_t i = 0; i < nBytes_align ; i++, addr++)
        data[i] = *(__IO uint8_t *) addr;
    data += nBytes_align;

    nWords = (nbytes >> 2);
    for(uint32_t i = 0; i < nWords ; i++, addr += 4)
        ((uint32_t *)data)[i] = *(__IO uint32_t *) addr;
    data += nWords << 2;

    nbytes -= (nWords << 2);
	for(uint32_t i = 0; i < nbytes ; i++, addr++)
	    data[i] = *(__IO uint8_t *) addr;

	return FLASH_STATUS_OK;
}

Flash_Interface_Status_t Flash_IsPageBlank(uint8_t *addr, uint8_t *is_blank)
{

    if (!addr){
        PRINT_DBG("ERROR:Checking if Flash page is blank, Address:%p \r\n", addr);
        return FLASH_STATUS_NOK;
    }

    if ((uint32_t)addr < FLASH_BASE || (uint32_t)addr >= FLASH_END){
        PRINT_DBG("ERROR:Checking if Flash page is blank, Address:%p outside of flash range\r\n", addr);
        return FLASH_STATUS_NOK;
    }

    if (((uint32_t)addr & 0x07FF)) {
        PRINT_DBG("ERROR:Checking if Flash page is blank, Address = %p is not the beginning of a flash page.\r\n", addr);
        return FLASH_STATUS_BAD_ADDR;
    }

    for (uint32_t i = 0; i < (FLASH_PAGE_SIZE >> 2); i++, addr += 4) {
        if (*(volatile uint32_t *) addr != 0xFFFFFFFF) {
            *is_blank = 0;
            return FLASH_STATUS_OK;
        }
    }
    *is_blank = 1;
    return FLASH_STATUS_OK;
}
