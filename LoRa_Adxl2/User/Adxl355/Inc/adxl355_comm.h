/*
 * adxl355_comm.h
 *
 *  Created on: Jan 7, 2026
 *      Author: cmendes
 */

#ifndef ADXL355_COMM_H
#define ADXL355_COMM_H


#include <stdint.h>
#include "stm32wlxx_hal.h"
#include "adxl355.h"

#ifdef HAL_SPI_MODULE_ENABLED

typedef struct {
        SPI_HandleTypeDef *hspi;
        GPIO_TypeDef *spi_cs_port;
        uint16_t spi_cs_pin;
} ADXL355_SPI_t;

/**
 * @fn void ADXL355_ConfigSPI(SPI_HandleTypeDef*, GPIO_TypeDef*, uint16_t)
 * @brief Configures SPI
 *
 * If multiple devices, ADXL355_ConfigSPI must be called when switching between devices
 *
 * @param hspi
 * @param spi_cs_port
 * @param spi_cs_pin
 */
void ADXL355_COMM_ConfigSPI(SPI_HandleTypeDef *hspi, GPIO_TypeDef *spi_cs_port, uint16_t spi_cs_pin);

/* Read / Write functions matching ADXL355_Comm_t */
ADXL_Status_t ADXL355_COMM_ReadSPI(const uint8_t address, uint8_t *buffer,
                              const uint16_t data_size);

ADXL_Status_t ADXL355_COMM_ReadSPI_DMA(const uint8_t address, uint8_t *buffer,
                              const uint16_t data_size);

ADXL_Status_t ADXL355_COMM_WriteSPI(const uint8_t address,
                               const uint8_t *data_buffer,
                               const uint16_t data_size);

#endif // HAL_SPI_MODULE_ENABLED

#ifdef HAL_I2C_MODULE_ENABLED

typedef struct {
        I2C_HandleTypeDef *hi2c;
        uint16_t i2c_address;   /* 7-bit address << 1 */
} ADXL355_I2C_t;

/* Initialization */
void ADXL355_ConfigI2C(I2C_HandleTypeDef *hi2c, uint8_t i2c_7bit_address);

/* Read / Write functions matching ADXL355_Comm_t */
ADXL_Status_t ADXL355_ReadI2C(const uint8_t address,
                              uint8_t *buffer,
                              const uint16_t data_size);

ADXL_Status_t ADXL355_WriteI2C(const uint8_t address,
                               const uint8_t *buffer,
                               const uint16_t data_size);

#endif /* HAL_I2C_MODULE_ENABLED */


#endif /* ADXL355_COMM_H */
