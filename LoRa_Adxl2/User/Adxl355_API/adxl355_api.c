/*
 * Adxl355_api.c
 *
 *  Created on: 24 Mar 2026
 *      Author: afonsofernandes
 */

#include "adxl355_api.h"
#include "adxl355_api_def.h"
#include "utilities_def.h"
#include <stdio.h>

static ADXL355_State_t ADXL355_State = ADXL355_READY;
static ADXL355_Config_t CurrentAdxl355Config = ADXL355_NONE;

// non initialized parameters are automatically set to zero
ADXL355_CommFunc_t adxl355_comm_func = {
		.ADXL355_Write = ADXL355_COMM_WriteSPI,
		.ADXL355_Read = ADXL355_COMM_ReadSPI,
		.ADXL355_Read_DMA = ADXL355_COMM_ReadSPI_DMA
};

const ADXL355_DevCfg_t ScheduledAdxl355Cfg = {
		.odr = ADXL355_ODR_1000Hz,
		.hpf = ADXL355_HPF_OFF,
		.range = ADXL355_RANGE_2G,
		.int_pol = ADXL355_INT_POL_HIGH,
		.int_src = ADXL355_FIFO_FULL_INT1,
		.act_mode = ADXL355_INT_NONE,
		.act_cnt = 0,
		.act_thr_lsb = 0,
		.fifo_samples = 45,
		.x_offset_lsb = 0,
		.y_offset_lsb = 0,
		.z_offset_lsb = 0,
		.meas_mode = ADXL355_MEAS_TEMP_OFF_DRDY_OFF,
		.comm_func = &adxl355_comm_func
};

const ADXL355_DevCfg_t TiltAdxl355Cfg = {
		.odr = ADXL355_ODR_3_906Hz,
		.hpf = ADXL355_HPF_OFF,
		.range = ADXL355_RANGE_2G,
		.int_pol = ADXL355_INT_POL_HIGH,
		.int_src = ADXL355_INT_NONE,
		.act_mode = ADXL355_ACTIVITY_OFF,
		.act_cnt = 0,
		.act_thr_lsb = 0,
		.fifo_samples = 32,
		.x_offset_lsb = 0,
		.y_offset_lsb = 0,
		.z_offset_lsb = 0,
		.meas_mode = ADXL355_MEAS_TEMP_OFF_DRDY_OFF,
		.comm_func = &adxl355_comm_func
};

const ADXL355_DevCfg_t ActivityAdxl355Cfg = {
		.odr = ADXL355_ODR_1000Hz,
		.hpf = ADXL355_HPF_3,
		.range = ADXL355_RANGE_4G,
		.int_pol = ADXL355_INT_POL_HIGH,
		.int_src = ADXL355_ACTIVITY_INT2 | ADXL355_FIFO_FULL_INT1,
		.act_mode = ADXL355_ACTIVITY_X_ON_Y_ON_Z_ON,
		.act_cnt = 1,
		.act_thr_lsb = ACTIVITY_THRESHOLD_LSB,
		.fifo_samples = 45,
		.x_offset_lsb = 0,
		.y_offset_lsb = 0,
		.z_offset_lsb = 0,
		.meas_mode = ADXL355_MEAS_TEMP_OFF_DRDY_OFF,
		.comm_func = &adxl355_comm_func
};


// Pre-allocated buffers for ADXL355 Frame Data
ADXL_DataHeader_t raw_scheduled_data_header = {
		.data_type = ADXL_RAW,
		.event_type = SCHEDULED_EVENT,
		.odr = ScheduledAdxl355Cfg.odr,
		.range = ScheduledAdxl355Cfg.range,
		.hpf = ScheduledAdxl355Cfg.hpf,
		.payload_length = SCHEDULED_SAMPLE_SETS * ADXL355_BYTES_PER_SAMPLE_SET
};

ADXL_DataHeader_t raw_activity_data_header = {
		.data_type = ADXL_RAW,
		.event_type = ACTIVITY_EVENT,
		.odr = ActivityAdxl355Cfg.odr,
		.range = ActivityAdxl355Cfg.range,
		.hpf = ActivityAdxl355Cfg.hpf,
		.payload_length = ACTIVITY_SAMPLE_SETS * ADXL355_BYTES_PER_SAMPLE_SET
};

ADXL_DataHeader_t raw_tilt_data_header = {
		.data_type = ADXL_RAW,
		.event_type = TILT_EVENT,
		.odr = TiltAdxl355Cfg.odr,
		.range = TiltAdxl355Cfg.range,
		.hpf = TiltAdxl355Cfg.hpf,
		.payload_length = TILT_SAMPLE_SETS * ADXL355_BYTES_PER_SAMPLE_SET
};

ADXL_DataHeader_t compressed_scheduled_data_header;
ADXL_DataHeader_t compressed_activity_data_header;
ADXL_DataHeader_t compressed_tilt_data_header;

/* Pre-fill ACCELEROMETER Frame Header */
FRAME_Header_t acc_frame_header = {
		.version = 0x01,
		.data_id = ACCELEROMETER,
		.reserved = 0x0000
};


// Pre-allocated buffers to handle ADXL355 Data
ADXL355_RawSampleSet_t SampleSetsRaw[MAX_SAMPLE_SETS];
ADXL355_UnpackedSampleSet_t SampleSetsUnpacked[MAX_SAMPLE_SETS];
ADXL355_SampleSetGs_t SampleSetsGs[MAX_SAMPLE_SETS];
uint8_t Adxl355Frame[FRAME_SIZE];

// Pre-allocated buffers for ADXL355 quantized Data
uint16_t Xdiff[MAX_SAMPLE_SETS] = {0 };
uint16_t Ydiff[MAX_SAMPLE_SETS] = {0 };
uint16_t Zdiff[MAX_SAMPLE_SETS] = {0 };


// ADXL current configuration

static uint8_t ReadSampleSets( ADXL355_DevCfg_t *dev_cfg,
                    		ADXL355_RawSampleSet_t *raw_buffer,
							uint16_t sample_sets_to_read);

static void Adxl355RawDataFrame( FRAME_Header_t *frame_header,
                         	 	 ADXL_DataHeader_t *data_header,
								 const ADXL355_RawSampleSet_t *payload,
								 const uint16_t number_sample_sets, uint8_t *frame);

static void Adxl355CompressedDataFrame(FRAME_Header_t *frame_header,
                                ADXL_DataHeader_t *data_header,
                                const ADXL355_RawSampleSet_t *payload,
                                const uint16_t number_sample_sets,
                                uint8_t *frame);

static inline void ErrorCatchFunction(ADXL_Status_t code, const char *file, uint32_t line);


void ADXL355_Init(SPI_HandleTypeDef *hspi) {
	ADXL_Status_t ret_status;
	uint32_t payload_size_bits;
	uint32_t padding_bits;
	uint8_t dev_id;

	PRINT_ADXL355("Initializing ADXL355 - Start\r\n");

	// Disable ADXL355 INT2 interrupt
	EXTI->IMR1 &= ~EXTI_IMR1_IM8;
	EXTI->IMR1 &= ~EXTI_IMR1_IM7;

	ADXL355_COMM_ConfigSPI(hspi, ADXL_SPI_NCS_GPIO_Port, ADXL_SPI_NCS_Pin);

	/* Pre-fill ACCELEROMETER Data Headeres */
	// SCHEDULED EVENT -  Compressed
	// init with raw values, update necessary fields
	compressed_scheduled_data_header = raw_scheduled_data_header;
	compressed_scheduled_data_header.data_type = ADXL_COMPRESSED;
	payload_size_bits = SCHEDULED_SAMPLE_SETS * ADXL_SAMPLES_PER_SAMPLE_SET * NBITS;
	padding_bits = (payload_size_bits % 8) ? 8 - (payload_size_bits % 8) : 0;
	compressed_scheduled_data_header.payload_length = (payload_size_bits + padding_bits) / 8;


	// ACTIVITY EVENT - Compressed
	// init with raw values, update necessary fields
	compressed_activity_data_header = raw_activity_data_header;
	compressed_activity_data_header.data_type = ADXL_COMPRESSED;
	payload_size_bits = ACTIVITY_SAMPLE_SETS * ADXL_SAMPLES_PER_SAMPLE_SET * NBITS;
	padding_bits = (payload_size_bits % 8) ? 8 - (payload_size_bits % 8) : 0;
	compressed_activity_data_header.payload_length = (payload_size_bits + padding_bits) / 8;


	// TILT EVENT - Compressed
	// init with raw values, update necessary fields
	compressed_tilt_data_header = raw_tilt_data_header;
	compressed_tilt_data_header.data_type = ADXL_COMPRESSED;
	payload_size_bits = TILT_SAMPLE_SETS * ADXL_SAMPLES_PER_SAMPLE_SET * NBITS;
	padding_bits = (payload_size_bits % 8) ? 8 - (payload_size_bits % 8) : 0;
	compressed_tilt_data_header.payload_length = (payload_size_bits + padding_bits) / 8;


	/* Check if ADXL355 is found and reset it */
	ret_status = ADXL355_GetDevIdAd(&adxl355_comm_func, &dev_id);
	if (ret_status != ADXL_OK) {
		ERROR_CATCH(ret_status);
	}

	ret_status = ADXL355_Reset(&adxl355_comm_func);
	if (ret_status != ADXL_OK) {
		ERROR_CATCH(ret_status);
	}

	/* Configure ADXL355 and wait for filters to settle before starting measurement */
	ret_status = ADXL355_UTIL_Config(&ActivityAdxl355Cfg);
	if (ret_status != ADXL_OK) {
		ERROR_CATCH(ret_status);
	}
	CurrentAdxl355Config = ADXL355_ACTIVITY;

	HAL_Delay(ACTIVITY_FILTER_SETTLING_WAIT_MS);

	// Clear fifo and status
	ADXL355_ClearFIFO(&adxl355_comm_func);
	uint8_t status_reg;
	ADXL355_ReadRegister(&adxl355_comm_func, ADXL355_REG_STATUS, &status_reg);

	// Clear Flag and Enable ADXL355 INT2 interrupt
	__HAL_GPIO_EXTI_CLEAR_IT(ADXL_INT2_Pin);
	EXTI->IMR1 |= EXTI_IMR1_IM8;
	PRINT_ADXL355("Initializing ADXL355 - Finish\r\n");
	return;
}

void ADXL355_RestoreEXTI_IRQ(void){
	ADXL_Status_t ret_status;

	EXTI->IMR1 &= ~EXTI_IMR1_IM7;

	ADXL355_ReadRegister(&adxl355_comm_func, ADXL355_REG_STATUS,
								 &ret_status);

	__HAL_GPIO_EXTI_CLEAR_IT(ADXL_INT2_Pin);
	EXTI->IMR1 |= EXTI_IMR1_IM8;
}

// -------------------------- TILT ---------------------------
void ADXL355_ReadTiltData(uint8_t **frame, uint32_t *frame_size) {
	uint32_t frame_size_bytes;
	ADXL_Status_t ret_status;

	PRINT_ADXL355("Reading Tilt Data - Start\r\n");
	// STM32 ADXL355_INT2 interrupt
	EXTI->IMR1 &= ~EXTI_IMR1_IM8;


	ret_status = ADXL355_UTIL_Config(&TiltAdxl355Cfg);

	HAL_Delay(TILT_FILTER_SETTLING_WAIT_MS);

	if (ret_status != ADXL_OK) {
		ERROR_CATCH(ret_status);
	}

	ReadSampleSets(&TiltAdxl355Cfg, SampleSetsRaw, TILT_SAMPLE_SETS);

//	Adxl355RawDataFrame(&acc_frame_header, &raw_tilt_data_header,
//						SampleSetsRaw,
//						TILT_SAMPLE_SETS,
//						Adxl355Frame);
//
//	frame_size_bytes = raw_tilt_data_header.payload_length
//	+ FRAME_HEADER_SIZE_BYTES + ADXL_HEADER_SIZE_BYTES;


	Adxl355CompressedDataFrame(&acc_frame_header,
							   &compressed_tilt_data_header,
							   SampleSetsRaw, TILT_SAMPLE_SETS,
							   Adxl355Frame);

	frame_size_bytes = compressed_tilt_data_header.payload_length
	+ FRAME_HEADER_SIZE_BYTES + ADXL_HEADER_SIZE_BYTES;

	// restart ADXL355 in activity mode
	ret_status = ADXL355_UTIL_Config(&ActivityAdxl355Cfg);
	if (ret_status != ADXL_OK) {
		ERROR_CATCH(ret_status);
	}

	HAL_Delay(ACTIVITY_FILTER_SETTLING_WAIT_MS);

	// Clear ADXL355 Status register
	uint8_t status_reg;
	ADXL355_ReadRegister(&adxl355_comm_func, ADXL355_REG_STATUS,
						 &status_reg);

	// Clear STM32 ADXL355_INT2 interrupt flag and enable Interrupt
	__HAL_GPIO_EXTI_CLEAR_IT(ADXL_INT2_Pin);
	EXTI->IMR1 |= EXTI_IMR1_IM8;

	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_RESET);

	*frame = &Adxl355Frame;
	*frame_size = frame_size_bytes;
	PRINT_ADXL355("Reading Tilt Data - Finish\r\n");
	return;
}

// ------------------------ SCHEDULED ------------------------
uint8_t ADXL355_ConfigScheduled() {
	ADXL_Status_t ret_status;

	PRINT_ADXL355("Reading Scheduled Data Config\r\n");

	if ( ADXL355_State != ADXL355_READY) {
		PRINT_ADXL355("NOK, ADXL355 is busy\r\n");
		return 0xFF;
	}
	ADXL355_State = ADXL355_BUSY;
	// Disable interrupts
	EXTI->IMR1 &= ~EXTI_IMR1_IM8; // Disable STM32 ADXL355_INT2 interrupt

	ret_status = ADXL355_UTIL_Config(&ScheduledAdxl355Cfg);

	if (ret_status != ADXL_OK) {
		PRINT_ADXL355("NOK, error configuring ADXL355\r\n");
		CurrentAdxl355Config = ADXL355_NONE;
		return 0xFF;
	}
	CurrentAdxl355Config = ADXL355_SCHEDULED;

	PRINT_ADXL355("OK, ADXL355 configured\r\n");
	return 0x00;
}

uint8_t ADXL355_ReadScheduledData_Read(size_t sample_sets_to_read) {
	static size_t sample_sets_read = 0;
	uint8_t num_samples = 0;

	PRINT_ADXL355("Reading Scheduled Data Read\r\n");
	num_samples = ReadSampleSets(&ScheduledAdxl355Cfg, SampleSetsRaw + sample_sets_read, sample_sets_to_read - sample_sets_read);

	if (num_samples == 0xFF) {
		PRINT_ADXL355("Error reading ADXL355 data\r\n");
		sample_sets_read = 0;
		PRINT_ADXL355("Reading Scheduled Data Read - NOK\r\n");
		return 0xFF;
	}

	if ((sample_sets_read + num_samples) >= sample_sets_to_read) {
		printf("FINS\r\n");
		sample_sets_read = 0;
		PRINT_ADXL355("Reading Scheduled Data Read - OK (continue)\r\n");
		return 0;
	} else {
		sample_sets_read += num_samples;
		PRINT_ADXL355("Reading Scheduled Data Read - OK (repeat)\r\n");
		__HAL_GPIO_EXTI_CLEAR_IT(ADXL_INT1_Pin);
		EXTI->IMR1 |= EXTI_IMR1_IM7;
		return 1;
	}
}

uint8_t ADXL355_ReadScheduledData_Process(uint8_t **frame, uint32_t *frame_size){
	uint32_t frame_size_bytes;
	ADXL_Status_t ret_status;

	PRINT_ADXL355("Reading Scheduled Data Process\r\n");
	Adxl355CompressedDataFrame(&acc_frame_header,
							   &compressed_scheduled_data_header,
							   SampleSetsRaw, SCHEDULED_SAMPLE_SETS,
							   Adxl355Frame);

	frame_size_bytes = compressed_scheduled_data_header.payload_length
	+ FRAME_HEADER_SIZE_BYTES + ADXL_HEADER_SIZE_BYTES;

	// restart ADXL355 in activity mode
	ret_status = ADXL355_UTIL_Config(&ActivityAdxl355Cfg);
	if (ret_status != ADXL_OK) {
		PRINT_ADXL355("Error configuring ADXL355\r\n");
		PRINT_ADXL355("Reading Scheduled Data Process - NOK\r\n");
		CurrentAdxl355Config = ADXL355_NONE;
		return 0xFF;
	}
	CurrentAdxl355Config = ADXL355_ACTIVITY;
	*frame = &Adxl355Frame;
	*frame_size = frame_size_bytes;
	PRINT_ADXL355("Reading Scheduled Data Process - OK\r\n");
	return 0;
}

uint8_t ADXL355_ReadScheduledData_Finish(){
	uint8_t status_reg;

	PRINT_ADXL355("Reading Scheduled Data Finish\r\n");

	ADXL355_ReadRegister(&adxl355_comm_func, ADXL355_REG_STATUS,
						 &status_reg);

	ADXL355_State = ADXL355_READY;
	PRINT_ADXL355("Reading Scheduled Data - OK\r\n");

	// Clear STM32 ADXL355_INT2 interrupt flag and enable Interrupt
	EXTI->IMR1 &= ~EXTI_IMR1_IM7;
	__HAL_GPIO_EXTI_CLEAR_IT(ADXL_INT2_Pin);
	EXTI->IMR1 |= EXTI_IMR1_IM8;

	return 0;
}

// ------------------------ ACTIVITY -------------------------
uint8_t ADXL355_ConfigActivity() {
	ADXL_Status_t ret_status;

	PRINT_ADXL355("Reading Activity Data Config\r\n");

	if ( ADXL355_State != ADXL355_READY) {
		PRINT_ADXL355("NOK, ADXL355 is busy\r\n");
		return 0xFF;
	}
	ADXL355_State = ADXL355_BUSY;
	PRINT_ADXL355("OK, ADXL355 configured\r\n");

	// Disable interrupts
	EXTI->IMR1 &= ~EXTI_IMR1_IM8; // Disable STM32 ADXL355_INT2 interrupt
	__HAL_GPIO_EXTI_CLEAR_IT(ADXL_INT1_Pin);
	EXTI->IMR1 |= EXTI_IMR1_IM7;
	return 0x00;
}

uint8_t ADXL355_ReadActivityData_Read(size_t sample_sets_to_read) {
	static size_t sample_sets_read = 0;
	uint8_t num_samples = 0;

	PRINT_ADXL355("Reading Activity Data Read\r\n");
	num_samples = ReadSampleSets(&ActivityAdxl355Cfg, SampleSetsRaw + sample_sets_read, sample_sets_to_read - sample_sets_read);
	if (num_samples == 0xFF) {
		PRINT_ADXL355("Error reading ADXL355 data\r\n");
		sample_sets_read = 0;
		PRINT_ADXL355("Reading Activity Data Read - NOK\r\n");
		return 0xFF;
	}
	if ((sample_sets_read + num_samples) >= sample_sets_to_read) {
		printf("FIN\r\n");
		sample_sets_read = 0;
		PRINT_ADXL355("Reading Activity Data Read - OK (continue)\r\n");
		return 0;
	} else {
		sample_sets_read += num_samples;
		PRINT_ADXL355("Reading Activity Data Read - OK (repeat)\r\n");
		__HAL_GPIO_EXTI_CLEAR_IT(ADXL_INT1_Pin);
		EXTI->IMR1 |= EXTI_IMR1_IM7;
		return 1;
	}
}

uint8_t ADXL355_ReadActivityData_Process(uint8_t **frame, uint32_t *frame_size){
	uint32_t frame_size_bytes;
	ADXL_Status_t ret_status;

	PRINT_ADXL355("Reading Activity Data Process\r\n");

	uint32_t deadline = HAL_GetTick() + 2;

	Adxl355CompressedDataFrame(&acc_frame_header,
							   &compressed_activity_data_header,
							   SampleSetsRaw, ACTIVITY_SAMPLE_SETS,
							   Adxl355Frame);

	frame_size_bytes = compressed_activity_data_header.payload_length
	+ FRAME_HEADER_SIZE_BYTES + ADXL_HEADER_SIZE_BYTES;

	*frame = &Adxl355Frame;
	*frame_size = frame_size_bytes;
	ADXL355_State = ADXL355_READY;

	PRINT_ADXL355("Reading Activity Data Process - OK\r\n");

	ADXL355_ReadRegister(&adxl355_comm_func, ADXL355_REG_STATUS,
							 &ret_status);

	// Clear STM32 ADXL355_INT2 interrupt flag and enable Interrupt
	EXTI->IMR1 &= ~EXTI_IMR1_IM7;
	__HAL_GPIO_EXTI_CLEAR_IT(ADXL_INT2_Pin);
	EXTI->IMR1 |= EXTI_IMR1_IM8;

	return 0;
}

// ------------------ AUXILIARY FUNCTIONS --------------------
ADXL355_Config_t ADXL355_GetCurrentConfig(void) {
	return CurrentAdxl355Config;
}

ADXL355_State_t ADXL355_GetState(void){
	return ADXL355_State;
}
// -------------------------- INTERNAL FUNCTIONS ---------------------
static uint8_t ReadSampleSets(ADXL355_DevCfg_t *dev_cfg,
                    		  ADXL355_RawSampleSet_t *raw_buffer,
							  uint16_t sample_sets_to_read) {

    ADXL_Status_t ret_status;
	uint8_t num_fifo_entries;
	uint8_t num_sample_sets = 0;

	ret_status = ADXL355_ReadNumberFifoEntries(dev_cfg->comm_func,
											   &num_fifo_entries);
	if (ret_status != ADXL_OK) {
		return 0xFF;
	}

	printf("FIFO: %d\r\n", num_fifo_entries);

	// to maintain constant FIFO alignment read only complete samples sets XYZ
	if (num_fifo_entries >= ADXL_SAMPLES_PER_SAMPLE_SET) {

		num_sample_sets = (uint8_t) (num_fifo_entries / 3);

		if(num_sample_sets > sample_sets_to_read) {
			num_sample_sets = sample_sets_to_read;
		}

		ret_status = ADXL355_ReadFifoSampleSet(dev_cfg->comm_func, raw_buffer, num_sample_sets);

		if (ret_status != ADXL_OK) {
			return 0xFF;
		}
    }

	return num_sample_sets;
}

static void Adxl355RawDataFrame(FRAME_Header_t *frame_header,
                         ADXL_DataHeader_t *data_header,
                         const ADXL355_RawSampleSet_t *payload,
                         const uint16_t number_sample_sets, uint8_t *frame) {

    FRAME_BitWriter_t bit_writer;
    FRAME_InitBitWriter(&bit_writer, frame);

    FRAME_SerializeFrameHeader(frame_header, &bit_writer);
    ADXL_SerializeDataHeader(data_header, &bit_writer);
    ADXL355_SerializeRawPayload(payload, number_sample_sets, &bit_writer);

    // Due to automatic padding, bitpos should now be multiple of 8 bits
    if (bit_writer.bitpos % 8) {
        ERROR_CATCH(0);
    }
}

static void Adxl355CompressedDataFrame(FRAME_Header_t *frame_header,
                                ADXL_DataHeader_t *data_header,
                                const ADXL355_RawSampleSet_t *payload,
                                const uint16_t number_sample_sets,
                                uint8_t *frame) {

    FRAME_BitWriter_t bit_writer;
    FRAME_InitBitWriter(&bit_writer, frame);

    FRAME_SerializeFrameHeader(frame_header, &bit_writer);
    ADXL_SerializeDataHeader(data_header, &bit_writer);

    // Unpack payload
    ADXL355_UnpackSampleSet(SampleSetsRaw, number_sample_sets,
                            SampleSetsUnpacked);

    // Initialize Quantized data object
    ADXL355_QuantizedSampleSets_t quantized_block;
    quantized_block.x_diff = Xdiff;
    quantized_block.y_diff = Ydiff;
    quantized_block.z_diff = Zdiff;

    //Fill Quantized data object
    ADXL355_QuantizeSampleSetBlock(SampleSetsUnpacked, &quantized_block,
                                   number_sample_sets,
                                   NBITS);

    // Serialized (and compress) the quantized payload
    ADXL355_SerializeQuantizedPayload(&quantized_block, number_sample_sets,
                                      &bit_writer);

    // Due to automatic padding, bitpos should be multiple of 8 bits
    if (bit_writer.bitpos % 8) {
        ERROR_CATCH(0);
    }
}

static inline void ErrorCatchFunction(ADXL_Status_t code, const char *file, uint32_t line) {
    __BKPT(0);
}

