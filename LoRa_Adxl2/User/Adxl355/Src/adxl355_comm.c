/*
 * adxl355_comm.c
 *
 *  Created on: Jan 7, 2026
 *      Author: cmendes
 */

#include "adxl355_comm.h"

/* ===================== SPI BACKEND ===================== */

#ifdef HAL_SPI_MODULE_ENABLED

static ADXL355_SPI_t adxl355_spi;

void ADXL355_COMM_ConfigSPI(SPI_HandleTypeDef *hspi, GPIO_TypeDef *spi_cs_port, uint16_t spi_cs_pin) {
    adxl355_spi.hspi = hspi;
    adxl355_spi.spi_cs_port = spi_cs_port;
    adxl355_spi.spi_cs_pin = spi_cs_pin;
}

ADXL_Status_t ADXL355_COMM_ReadSPI(const uint8_t address, uint8_t *buffer,
                              const uint16_t data_size) {

    HAL_StatusTypeDef ret_value;
    uint32_t deadline = HAL_GetTick() + 1;

    while (HAL_SPI_GetState(adxl355_spi.hspi) != HAL_SPI_STATE_READY) {
    	if (HAL_GetTick() > deadline) {
    		printf("SPI n rdy in 1ms\r\n");
			return ADXL_SPI_SEND_ERROR;
		}
    }

    HAL_GPIO_WritePin(adxl355_spi.spi_cs_port, adxl355_spi.spi_cs_pin,
                      GPIO_PIN_RESET);

    uint8_t spi_data = ( (address << 1) | ADXL355_SPI_READ_BIT);

    ret_value = HAL_SPI_Transmit(adxl355_spi.hspi, &spi_data, 1, HAL_MAX_DELAY);

    if (ret_value == HAL_OK) {
		ret_value = HAL_SPI_Receive(adxl355_spi.hspi, buffer, data_size, HAL_MAX_DELAY);
    }

    HAL_GPIO_WritePin(adxl355_spi.spi_cs_port, adxl355_spi.spi_cs_pin,
                      GPIO_PIN_SET);

    if (ret_value != HAL_OK) {
        return ADXL_SPI_SEND_ERROR;
    }

    return ADXL_OK;
}

ADXL_Status_t ADXL355_COMM_ReadSPI_DMA(const uint8_t address, uint8_t *buffer,
                              const uint16_t data_size) {

    HAL_StatusTypeDef ret_value;

    uint32_t deadline = HAL_GetTick() + 2;

	while (HAL_SPI_GetState(adxl355_spi.hspi) != HAL_SPI_STATE_READY) {
		if (HAL_GetTick() >  deadline) {
			printf("SPI DMA n rdy in 1ms\r\n");
			return ADXL_SPI_SEND_ERROR;
		}
	}

	if (HAL_GPIO_ReadPin(adxl355_spi.spi_cs_port, adxl355_spi.spi_cs_pin) == GPIO_PIN_RESET) {
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9,GPIO_PIN_SET);
	}
    HAL_GPIO_WritePin(adxl355_spi.spi_cs_port, adxl355_spi.spi_cs_pin,
                      GPIO_PIN_RESET);

    uint8_t spi_data = ( (address << 1) | ADXL355_SPI_READ_BIT);

    ret_value = HAL_SPI_Transmit(adxl355_spi.hspi, &spi_data, 1, HAL_MAX_DELAY);
    if (ret_value != HAL_OK) {
		HAL_GPIO_WritePin(adxl355_spi.spi_cs_port, adxl355_spi.spi_cs_pin, GPIO_PIN_SET);
		return ADXL_SPI_SEND_ERROR;
	}


	// For larger data sizes, use DMA for better performance
	ret_value = HAL_SPI_Receive_DMA(adxl355_spi.hspi, buffer, data_size);
	if (ret_value != HAL_OK) {
		HAL_GPIO_WritePin(adxl355_spi.spi_cs_port, adxl355_spi.spi_cs_pin, GPIO_PIN_SET);
		return ADXL_SPI_RECV_ERROR;
	}
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15,GPIO_PIN_SET);
    return ADXL_OK;
}

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hspi);
  HAL_GPIO_WritePin(adxl355_spi.spi_cs_port, adxl355_spi.spi_cs_pin,
  	                      GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15,GPIO_PIN_RESET);
  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_SPI_RxCpltCallback should be implemented in the user file
   */
}

ADXL_Status_t ADXL355_COMM_WriteSPI(const uint8_t address,
                               const uint8_t *data_buffer,
                               const uint16_t data_size) {

    HAL_StatusTypeDef ret_value;

    HAL_GPIO_WritePin(adxl355_spi.spi_cs_port, adxl355_spi.spi_cs_pin,
                      GPIO_PIN_RESET);

    uint8_t spi_address = (address << 1) & ~ADXL355_SPI_READ_BIT;

    ret_value = HAL_SPI_Transmit(adxl355_spi.hspi, &spi_address, 1,
    HAL_MAX_DELAY);

    if (ret_value == HAL_OK) {
        ret_value = HAL_SPI_Transmit(adxl355_spi.hspi, data_buffer, data_size,
        HAL_MAX_DELAY);
    }

    HAL_GPIO_WritePin(adxl355_spi.spi_cs_port, adxl355_spi.spi_cs_pin,
                      GPIO_PIN_SET);

    if (ret_value != HAL_OK) {
        return ADXL_SPI_SEND_ERROR;
    }

    return ADXL_OK;
}


#endif /* HAL_SPI_MODULE_ENABLED */


/* ===================== I2C BACKEND ===================== */

#ifdef HAL_I2C_MODULE_ENABLED

static ADXL355_I2C_t adxl355_i2c;

void ADXL355_ConfigI2C(I2C_HandleTypeDef *hi2c,
                     uint8_t i2c_7bit_address)
{
    adxl355_i2c.hi2c = hi2c;
    adxl355_i2c.i2c_address = (uint16_t)(i2c_7bit_address << 1);
}

ADXL_Status_t ADXL355_ReadI2C(const uint8_t address,
                              uint8_t *buffer,
                              const uint16_t data_size)
{
    if ((adxl355_i2c.hi2c == NULL) || (buffer == NULL)) {
        return ADXL_ERROR;
    }

    /* Write register address, then read data */
    if (HAL_I2C_Mem_Read(adxl355_i2c.hi2c,
                         adxl355_i2c.i2c_address,
                         address,
                         I2C_MEMADD_SIZE_8BIT,
                         buffer,
                         data_size,
                         HAL_MAX_DELAY) != HAL_OK) {
        return ADXL_ERROR;
    }

    return ADXL_OK;
}

ADXL_Status_t ADXL355_WriteI2C(const uint8_t address,
                               const uint8_t *buffer,
                               const uint16_t data_size)
{
    if ((adxl355_i2c.hi2c == NULL) || (buffer == NULL)) {
        return ADXL_ERROR;
    }

    if (HAL_I2C_Mem_Write(adxl355_i2c.hi2c,
                          adxl355_i2c.i2c_address,
                          address,
                          I2C_MEMADD_SIZE_8BIT,
                          (uint8_t *)buffer,
                          data_size,
                          HAL_MAX_DELAY) != HAL_OK) {
        return ADXL_ERROR;
    }

    return ADXL_OK;
}

#endif /* HAL_I2C_MODULE_ENABLED */
