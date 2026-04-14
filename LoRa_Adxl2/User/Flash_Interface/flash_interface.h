/**
 * @file flash_interface.h
 * @def FLASH_INTERFACE_FLASH_INTERFACE_H____
 * @brief Flash Memory Interface
 *
 * @details Public API for the Flash Memory.
 *
 *          The API allows to read and write from the flash.
 *          There are two writes available:
 *              1 - Flash_Write, writes to the flash and erases everything else on the affected pages.
 *              2 - Flash_WriteElement, writes to the flash preserving the bytes that aren't written.
 *
 * @author Afonso Fernandes - SOLVIT
 * @date Dec 9, 2025
 */

#ifndef FLASH_INTERFACE_FLASH_INTERFACE_H_
#define FLASH_INTERFACE_FLASH_INTERFACE_H_

/*--------------- Standard Includes ---------------*/

/*--------------- Stm Includes ---------------*/

#include "stm32wlxx_hal.h"

/*--------------- MACROS ---------------*/

#define PRINT_DBG(...)

#define FLASH_BOOTLOADER    0x08000000UL /* 22KB */
#define FLASH_SHARED        0x08005800UL /* 2KB */
#define FLASH_SLOT_0        0x08006000UL /* 110KB */
#define FLASH_SLOT_1        0x08021800UL /* 110KB */
#define FLASH_DATA_SLOT     0x0803D000UL /* 12KB */
#define FLASH_END           0x08040000UL

/*--------------- Typedef ---------------*/

typedef enum Flash_Interface_Status_e {
    FLASH_STATUS_OK = 0,
    FLASH_STATUS_NOK,
    FLASH_STATUS_BAD_ADDR
} Flash_Interface_Status_t;

/*--------------- Global Variables ---------------*/

/*--------------- Function Prototypes ---------------*/

/**
 * @brief Erases nPages number of pages in the flash memory, starting with the page where addr is located.
 *
 * @param addr   (Input) Address to the region to be erased.
 * @param nbytes (Input) Number of size of the region.
 *
 * @return Status Code.
 */
Flash_Interface_Status_t Flash_Erase(uint8_t *addr, size_t nbytes);

/**
 * @brief Write nbytes to the Flash erasing everything else that is stored on the affected pages.
 *
 * @param addr   (Input) Address of the start of flash memory to be written.
 * @param data   (Input) Pointer to the bytes to be written.
 * @param nbytes (Input) Amount of bytes to be written.
 *
 * @return Status code.
 */
Flash_Interface_Status_t Flash_Write(uint8_t *addr, uint8_t *data, size_t nbytes);

/**
 * @brief Reads nbytes from flash memory into address pointed to by data.
 *
 * @param addr   (Input) Address of the start of flash memory to be read.
 * @param data   (Input) Pointer to the allocated memory that will store the content read.
 * @param nbytes (Input) Amount of bytes to be read.
 *
 * @return Status code.
 */
Flash_Interface_Status_t Flash_Read(uint8_t *addr, uint8_t *data, size_t nbytes);

/**
 * @brief Write nbytes to the Flash preserving everything else that is stored on the affected pages.
 *
 * @param addr   (Input) Address of the start of flash memory to be written.
 * @param data   (Input) Pointer to the bytes to be written.
 * @param nbytes (Input) Amount of bytes to be written.
 *
 * @return Status code.
 */
Flash_Interface_Status_t Flash_WriteElement(uint8_t *addr, uint8_t *data, size_t nbytes);

/**
 * @brief Check if a page is blank (all bytes are 0xFF).
 *
 * @param addr      (Input) Address of the start of flash memory to be checked.
 * @param is_blank  (Output) Pointer to a variable that will be set to 1 if the page is blank, 0 otherwise.
 *
 * @return Status code.
 */
Flash_Interface_Status_t Flash_IsPageBlank(uint8_t *addr, uint8_t *is_blank);

#endif /* FLASH_INTERFACE_FLASH_INTERFACE_H_ */
