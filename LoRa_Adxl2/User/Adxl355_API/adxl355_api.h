/*
 * adxl355_api.h
 *
 *  Created on: 24 Mar 2026
 *      Author: afonsofernandes
 */

#ifndef ADXL355_API_ADXL355_API_H_
#define ADXL355_API_ADXL355_API_H_

#include "main.h"

typedef enum ADXL355_State_e {
	ADXL355_READY = 0,
	ADXL355_BUSY
} ADXL355_State_t;

typedef enum ADXL355_Config_e {
	ADXL355_NONE,
	ADXL355_SCHEDULED,
	ADXL355_TILT,
	ADXL355_ACTIVITY
} ADXL355_Config_t;

void ADXL355_Init(SPI_HandleTypeDef *hspi);

void ADXL355_RestoreEXTI_IRQ();

// -------------------------- TILT ---------------------------
void ADXL355_ReadTiltData(uint8_t **frame, uint32_t *frame_size);

// ------------------------ SCHEDULED ------------------------
uint8_t ADXL355_ConfigScheduled();
uint8_t ADXL355_ReadScheduledData_Read(size_t sample_sets_to_read);
uint8_t ADXL355_ReadScheduledData_Process(uint8_t **frame, uint32_t *frame_size);
uint8_t	ADXL355_ReadScheduledData_Finish();

// ------------------------ ACTIVITY -------------------------
uint8_t ADXL355_ConfigActivity();
uint8_t ADXL355_ReadActivityData_Read(size_t sample_sets_to_read);
uint8_t ADXL355_ReadActivityData_Process(uint8_t **frame, uint32_t *frame_size);

// ------------------ AUXILIARY FUNCTIONS --------------------
ADXL355_Config_t ADXL355_GetCurrentConfig(void);
ADXL355_State_t ADXL355_GetState(void);
#endif /* ADXL355_API_ADXL355_API_H_ */
