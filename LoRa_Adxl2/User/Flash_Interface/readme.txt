============================================================
Flash Interface
============================================================

1. Overview
-----------

Flash interface is a C library that provides a read/write interface to use the flash memory.
It is designed for STM32 devices, and to be easy to use.

------------------------------------------------------------

2. Features
-----------

- Reads and writes from the flash.
- Writes in 2 modes:
	- simple write: erases affected pages of flash memory and writes the content.
	- polite write: copies affected pages, writes into the copy and then writes the full affected pages back into the falsh.
- External dependencies: The HAL driver for the flash memory (in stm32wlxx_hal_flash.h).

------------------------------------------------------------

3. Requirements
---------------

- C standard: C99
- Compiler: GCC
- Operating system: None

------------------------------------------------------------

4. File Structure
-----------------

Flash_Interface/
    flash_interface.c
    flash_interface.h

------------------------------------------------------------

5. Building the Library
-----------------------

Example using GCC:

    gcc -c Flash_Interface/flash_interface.c -o flash_interface.o
    ar rcs libflash_i.a flash_interface.o

------------------------------------------------------------

6. Usage
--------

6.1 - Including the library and configuring: 
---------------------------

Include the header in your source file:

    #include "Flash_Interface/flash_interface.h"

In the linker script, allocate a region of flash to use (ex: MY_FLASH):
	
	MEMORY
	{
	  RAM     (xrw) : ORIGIN = 0x20000000, LENGTH = 64K
	  RAM2    (xrw) : ORIGIN = 0x10000000, LENGTH = 32K
	  FLASH   (rx)  : ORIGIN = 0x08000000, LENGTH = 254K
	  MY_FLASH(rxw) : ORIGIN = 0x0803F800, LENGTH = 2K
	}

In flash_interface.h, define the region you are using:

	#define MY_FLASH_MEMORY_START_ADDRESS 0x0803F800UL // First available address
	#define FLASH_MAX_ADDRESS             0x08040000UL // First address out of bounds (a.k.a. last address + 1)
    
6.2 - Using: 
-----------

6.3 - Write:
-----------

To do the SIMPLE WRITE call:
	
	Flash_Interface_Status_t Flash_Write(uint64_t *addr, uint64_t *data, uint32_t nbytes);

To do the POLITE WRITE call:
	
	Flash_Interface_Status_t Flash_WriteElement(uint8_t *addr, uint8_t *data, uint32_t nbytes);

6.4 - Read:
----------

To read call:
 
	Flash_Interface_Status_t Flash_Read(uint8_t *addr, uint8_t *data, uint32_t nbytes);

------------------------------------------------------------

7. Memory Management
--------------------

Describe:
- Allocated memory is owned by the library.
- All allocated memory by the library is freed by the library.
- The library does not free memory allocated elsewhere.

------------------------------------------------------------

8. Error Handling
-----------------

Error Reporting features:
- Return codes
- NULL pointers

------------------------------------------------------------

9. License
-----------

SOLVIT – Innovation on Telecomunications © 2025. All Rights Reserved.

------------------------------------------------------------

10. Author / Maintainer
----------------------

Name: Afonso Fernandes
Email: afonsofernandes@solvit.pt

------------------------------------------------------------

11. Version History
-------------------

v1.0.0  - Initial release

============================================================
