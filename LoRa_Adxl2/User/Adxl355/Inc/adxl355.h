/**
 * @brief ADXL355 header file
 *
 * Header file for the ADXL355 accelerometer for STM32 MCUs (depends on STM32 SPI and GPIO functions).
 *
 * In the ADXL355's documentation, the meaning of single acceleration data, sample and entry is used interchangeably
 * to mean a single axis data.
 *
 * In this library
 *      Entry and Sample -> measurement pertaining a single axis
 *      SampleSet -> a three axis measurement (X, Y, Z)
 *
 *
 * @author Carlos Mendes
 * @date Dec/2025
 */

#ifndef ADXL355_LIB_H
#define ADXL355_LIB_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

// TODO:    Check the NVM_BUSY flag

/**
 * @name ADXL355 Register Addresses
 */
#define ADXL355_REG_DEVID_AD        0x00
#define ADXL355_REG_DEVID_MST       0x01
#define ADXL355_REG_PARTID          0x02
#define ADXL355_REG_REVID           0x03
#define ADXL355_REG_STATUS          0x04
#define ADXL355_REG_FIFO_ENTRIES    0x05
#define ADXL355_REG_TEMP2           0x06
#define ADXL355_REG_TEMP1           0x07
#define ADXL355_REG_XDATA3          0x08
#define ADXL355_REG_XDATA2          0x09
#define ADXL355_REG_XDATA1          0x0A
#define ADXL355_REG_YDATA3          0x0B
#define ADXL355_REG_YDATA2          0x0C
#define ADXL355_REG_YDATA1          0x0D
#define ADXL355_REG_ZDATA3          0x0E
#define ADXL355_REG_ZDATA2          0x0F
#define ADXL355_REG_ZDATA1          0x10
#define ADXL355_REG_FIFO_DATA       0x11
#define ADXL355_REG_OFFSET_X_H      0x1E
#define ADXL355_REG_OFFSET_X_L      0x1F
#define ADXL355_REG_OFFSET_Y_H      0x20
#define ADXL355_REG_OFFSET_Y_L      0x21
#define ADXL355_REG_OFFSET_Z_H      0x22
#define ADXL355_REG_OFFSET_Z_L      0x23
#define ADXL355_REG_ACT_EN          0x24
#define ADXL355_REG_ACT_THRESH_H    0x25
#define ADXL355_REG_ACT_THRESH_L    0x26
#define ADXL355_REG_ACT_COUNT       0x27
#define ADXL355_REG_FILTER          0x28
#define ADXL355_REG_FIFO_SAMPLES    0x29
#define ADXL355_REG_INT_MAP         0x2A
#define ADXL355_REG_SYNC            0x2B
#define ADXL355_REG_RANGE           0x2C
#define ADXL355_REG_POWER_CTL       0x2D
#define ADXL355_REG_SELF_TEST       0x2E
#define ADXL355_REG_RESET           0x2F
#define ADXL355_REG_SHADOW          0x50

/**
 * @name ADXL355 Device IDs
 * @{
 */
#define ADXL355_DEVID_AD        0xAD
#define ADXL355_DEVID_MST       0x1D
#define ADXL355_PART_ID         0xED
#define ADXL355_REVID           0x01
#define ADXL355_I2C_ADDRESS     0x1D // 0x1D (MISO/ASEL = 0) 0x53 (MISO/ASEL = 1)

/**
 * @name ADXL355 Reset Related constants
 */
#define ADXL355_RESET_CODE      0x52
#define RESET_RETRY_COUNT       255

/**
 * @name ADXL355 related MASKs
 */
#define ADXL355_SPI_READ_BIT            0x01
#define ADXL355_ID_MASK                 0xFF
#define ADXL355_NVM_BUSY_MASK           0x10
#define ADXL355_ACTIVITY_MASK           0x08
#define ADXL355_FIFO_OVR_MASK           0x04
#define ADXL355_FIFO_FULL_MASK          0x02
#define ADXL355_DATA_RDY_MASK           0x01
#define ADXL355_FIFO_ENTRIES_MASK       0x7F
#define ADXL355_TEMPERATURE_MASK        0x0FFF
#define ADXL355_DATA_MASK               0xFFFFF0
#define ADXL355_OFFSET_MASK             0xFFFF
#define ADXL355_ACT_EN_MASK             0x07
#define ADXL355_THRESHOLD_MASK          0xFFFF
#define ADXL355_ACT_COUNT_MASK          0xFF
#define ADXL355_HPF_MASK                0x70
#define ADXL355_ODR_MASK                0x0F
#define ADXL355_FIFO_SAMPLES_MASK       0x7F
#define ADXL355_INT_MASK                0xff
#define ADXL355_FIFO_EXT_CLK_MASK       0x04
#define ADXL355_FIFO_EXT_SYNC_MASK      0x03
#define ADXL355_I2C_HS_MASK             0x80
#define ADXL355_INT_POL_MASK            0x40
#define ADXL355_RANGE_MASK              0x03
#define ADXL355_POWER_CTL_MASK          0x07
#define ADXL355_SELF_TEST_MASK          0x03
#define ADXL355_RESET_MASK              0x03

/**
 * @name ADXL355 Sample size related constants
 */
#define ADXL355_BYTES_PER_SAMPLE        3   // 1 AXIS == 1 ENTRIE
#define ADXL355_BYTES_PER_SAMPLE_SET    9   // 3 AXIS
#define ADXL355_FIFO_MAX_SAMPLES        96
#define ADXL_FIFO_MAX_BYTES             ADXL355_BYTES_PER_SAMPLE * ADXL355_FIFO_MAX_SAMPLES
#define ADXL_SAMPLES_PER_SAMPLE_SET     3

/**
 * @name ADXL355 Temperature related constants
 */
#define ADXL355_REFERENCE_TEMPERATURE       25                  // ºC
#define ADXL355_REFERENCE_TEMPERATURE_LSB   1885                // LSB
#define ADXL355_TEMPERATURE_SLOPE           -9.05f              // LSB/ºC
#define ADXL355_TEMPERATURE_SCALE           (-1.0f / 9.05f)     // ºC/LSB

/**
 * @name ADXL355 Sensitivity constants
 */
#define ADXL355_SENSITIVITY_2G      256000  // (LSB/g) -> approx 3.9 ug/LSB)
#define ADXL355_SENSITIVITY_4G      128000  // (LSB/g) -> approx 7.81 ug/LSB)
#define ADXL355_SENSITIVITY_8G       64000  // (LSB/g) -> approx 15.63 ug/LSB)

/**
 * @enum
 * @brief
 *
 */
typedef enum {
    ADXL355_ODR_4000Hz = 0x00,/**< ADXL355_ODR_4000Hz */
    ADXL355_ODR_2000Hz, /**< ADXL355_ODR_2000Hz */
    ADXL355_ODR_1000Hz, /**< ADXL355_ODR_1000Hz */
    ADXL355_ODR_500Hz, /**< ADXL355_ODR_500Hz */
    ADXL355_ODR_250Hz, /**< ADXL355_ODR_250Hz */
    ADXL355_ODR_125Hz, /**< ADXL355_ODR_125Hz */
    ADXL355_ODR_62_5Hz, /**< ADXL355_ODR_62_5Hz */
    ADXL355_ODR_31_25Hz, /**< ADXL355_ODR_31_25Hz */
    ADXL355_ODR_15_625Hz, /**< ADXL355_ODR_15_625Hz */
    ADXL355_ODR_7_813Hz, /**< ADXL355_ODR_7_813Hz */
    ADXL355_ODR_3_906Hz /**< ADXL355_ODR_3_906Hz */
} ADXL355_ODR_t;

/**
 * @enum
 * @brief
 *
 */
typedef enum {
    ADXL355_HPF_OFF = 0x00,     // HPF corner: not applicable
    ADXL355_HPF_1 = 0x10,       // HPF corner: 24.700E-4 * ODR
    ADXL355_HPF_2 = 0x20,	    // HPF corner: 6.2084E-4 * ODR
    ADXL355_HPF_3 = 0x30,	    // HPF corner: 1.5545E-4 * ODR
    ADXL355_HPF_4 = 0x40,	    // HPF corner: 0.3862E-4 * ODR
    ADXL355_HPF_5 = 0x50,	    // HPF corner: 0.0954E-4 * ODR
    ADXL355_HPF_6 = 0x60	    // HPF corner: 0.0238E-4 * ODR
} ADXL355_HPF_t;

/**
 * @enum
 * @brief
 *
 */
typedef enum {
    ADXL355_RANGE_2G = 0x01,/**< ADXL355_RANGE_2G */
    ADXL355_RANGE_4G = 0x02,/**< ADXL355_RANGE_4G */
    ADXL355_RANGE_8G = 0x03 /**< ADXL355_RANGE_8G */
} ADXL355_Range_t;

/**
 * @enum
 * @brief
 *
 */
typedef enum {
    ADXL355_INT_POL_LOW = 0x00,/**< ADXL355_INT_POL_LOW */
    ADXL355_INT_POL_HIGH = 0x40/**< ADXL355_INT_POL_HIGH */
} ADXL355_IntPol_t;

/**
 * @enum
 * @brief
 *
 */
typedef enum {
    ADXL355_INT_NONE        =0x00,
    ADXL355_DATA_READY_INT1 = 0x01,/**< ADXL355_DATA_READY_INT1 */
    ADXL355_FIFO_FULL_INT1 = 0x02, /**< ADXL355_FIFO_FULL_INT1 */
    ADXL355_FIFO_OVR_INT1 = 0x04, /**< ADXL355_FIFO_OVR_INT1 */
    ADXL355_ACTIVITY_INT1 = 0x08, /**< ADXL355_ACTIVITY_INT1 */
    ADXL355_DATA_READY_INT2 = 0x10,/**< ADXL355_DATA_READY_INT2 */
    ADXL355_FIFO_FULL_INT2 = 0x20, /**< ADXL355_FIFO_FULL_INT2 */
    ADXL355_FIFO_OVR_INT2 = 0x40, /**< ADXL355_FIFO_OVR_INT2 */
    ADXL355_ACTIVITY_INT2 = 0x80 /**< ADXL355_ACTIVITY_INT2 */
} ADXL355_IntSource_t;

/**
 * @enum
 * @brief
 *
 */
typedef enum {
    ADXL355_MEAS_TEMP_ON_DRDY_ON = 0, /**< ADXL355_MEAS_TEMP_ON_DRDY_ON */
    ADXL355_STDBY_TEMP_ON_DRDY_ON = 1, /**< ADXL355_STDBY_TEMP_ON_DRDY_ON */
    ADXL355_MEAS_TEMP_OFF_DRDY_ON = 2, /**< ADXL355_MEAS_TEMP_OFF_DRDY_ON */
    ADXL355_STDBY_TEMP_OFF_DRDY_ON = 3,/**< ADXL355_STDBY_TEMP_OFF_DRDY_ON */
    ADXL355_MEAS_TEMP_ON_DRDY_OFF = 4, /**< ADXL355_MEAS_TEMP_ON_DRDY_OFF */
    ADXL355_STDBY_TEMP_ON_DRDY_OFF = 5,/**< ADXL355_STDBY_TEMP_ON_DRDY_OFF */
    ADXL355_MEAS_TEMP_OFF_DRDY_OFF = 6,/**< ADXL355_MEAS_TEMP_OFF_DRDY_OFF */
    ADXL355_STDBY_TEMP_OFF_DRDY_OFF = 7/**< ADXL355_STDBY_TEMP_OFF_DRDY_OFF */
} ADXL355_MeasurementMode_t;

/**
 * @enum
 * @brief
 *
 */
typedef enum {
    ADXL355_ACTIVITY_OFF = 0, /**< ADXL355_ACTIVITY_OFF */
    ADXL355_ACTIVITY_X_ON_Y_OFF_Z_OFF = 1,/**< ADXL355_ACTIVITY_X_ON_Y_OFF_Z_OFF */
    ADXL355_ACTIVITY_X_OFF_Y_ON_Z_OFF = 2,/**< ADXL355_ACTIVITY_X_OFF_Y_ON_Z_OFF */
    ADXL355_ACTIVITY_X_ON_Y_ON_Z_OFF = 3, /**< ADXL355_ACTIVITY_X_ON_Y_ON_Z_OFF */
    ADXL355_ACTIVITY_X_OFF_Y_OFF_Z_ON = 4,/**< ADXL355_ACTIVITY_X_OFF_Y_OFF_Z_ON */
    ADXL355_ACTIVITY_X_ON_Y_OFF_Z_ON = 5, /**< ADXL355_ACTIVITY_X_ON_Y_OFF_Z_ON */
    ADXL355_ACTIVITY_X_OFF_Y_ON_Z_ON = 6, /**< ADXL355_ACTIVITY_X_OFF_Y_ON_Z_ON */
    ADXL355_ACTIVITY_X_ON_Y_ON_Z_ON = 7, /**< ADXL355_ACTIVITY_X_ON_Y_ON_Z_ON */
} ADXL355_ActivityMode_t;

/**
 * @enum
 * @brief
 *
 */
typedef enum {
    ADXL355_X_AXIS = 0,/**< ADXL355_X_AXIS */
    ADXL355_Y_AXIS = 2,/**< ADXL355_Y_AXIS */
    ADXL355_Z_AXIS = 4, /**< ADXL355_Z_AXIS */
} ADXL355_Axis_t;

/**
 * @enum
 * @brief
 *
 */
typedef enum {
    ADXL_OK, /**< ADXL_OK */
    ADXL_SPI_ERROR, /**< ADXL_SPI_ERROR */
    ADXL_SPI_RECEIVE_ERROR,/**< ADXL_SPI_RECEIVE_ERROR */
    ADXL_SPI_SEND_ERROR, /**< ADXL_SPI_SEND_ERROR */
	ADXL_SPI_RECV_ERROR, /**< ADXL_SPI_RECV_ERROR */
    ADXL_DATA_ERROR, /**< ADXL_DATA_ERROR */
    ADXL_INVALID_PARAMETER,/**< ADXL_INVALID_PARAMETER */
    ADXL_ERROR, /**< ADXL_ERROR */
    ADXL355_RESET_ERROR, /**< ADXL355_RESET_ERROR */
    ADXL355_INIT_ERROR /**< ADXL355_INIT_ERROR */
} ADXL_Status_t;

/**
 * @struct
 * @brief
 *
 */
typedef struct {
    ADXL_Status_t (*ADXL355_Read)(const uint8_t address, uint8_t *buffer,
                                  const uint16_t data_size);

    ADXL_Status_t (*ADXL355_Read_DMA)(const uint8_t address, uint8_t *buffer,
                                      const uint16_t data_size);

    ADXL_Status_t (*ADXL355_Write)(const uint8_t address, const uint8_t *buffer,
                                   const uint16_t data_size);
} ADXL355_CommFunc_t;

/**
 * @struct
 * @brief
 *
 */
typedef struct {
    uint8_t x_axis[ADXL355_BYTES_PER_SAMPLE];
    uint8_t y_axis[ADXL355_BYTES_PER_SAMPLE];
    uint8_t z_axis[ADXL355_BYTES_PER_SAMPLE];
} ADXL355_RawSampleSet_t;

/**
 * @fn ADXL_Status_t ADXL355_ReadRegister(const ADXL355_t*, uint8_t, uint8_t*)
 * @brief
 *
 * @param adxl355_comm_func
 * @param reg_address
 * @param buffer
 * @return
 */
ADXL_Status_t ADXL355_ReadRegister(const ADXL355_CommFunc_t *adxl355_comm_func_comm_func,
                                   uint8_t reg_address, uint8_t *buffer);

/**
 * @fn ADXL_Status_t ADXL355_WriteRegister(const ADXL355_t*, const uint8_t, const uint8_t)
 * @brief
 *
 * @param adxl355_comm_func
 * @param reg_address
 * @param data
 * @return
 */
ADXL_Status_t ADXL355_WriteRegister(const ADXL355_CommFunc_t *adxl355_comm_func,
                                    const uint8_t reg_address,
                                    const uint8_t data);
/**
 * @fn ADXL_Status_t ADXL355_ReadRegisterMasked(const ADXL355_t*, const uint8_t, const uint8_t, uint8_t*)
 * @brief
 *
 * @param adxl355_comm_func
 * @param reg_address
 * @param mask
 * @param buffer
 * @return
 */
ADXL_Status_t ADXL355_ReadRegisterMasked(const ADXL355_CommFunc_t *adxl355_comm_func,
                                         const uint8_t reg_address,
                                         const uint8_t mask, uint8_t *buffer);
/**
 * @fn ADXL_Status_t ADXL355_WriteRegisterMasked(const ADXL355_t*, const uint8_t, const uint8_t, uint8_t)
 * @brief
 *
 * @param adxl355_comm_func
 * @param reg_address
 * @param mask
 * @param data
 * @return
 */
ADXL_Status_t ADXL355_WriteRegisterMasked(const ADXL355_CommFunc_t *adxl355_comm_func,
                                          const uint8_t reg_address,
                                          const uint8_t mask, uint8_t data);

/* Device ID functions */
/**
 * @fn ADXL_Status_t ADXL355_GetDevIdAd(const ADXL355_t*, uint8_t*)
 * @brief
 *
 * @param adxl355_comm_func
 * @param dev_id_ad
 * @return
 */
ADXL_Status_t ADXL355_GetDevIdAd(const ADXL355_CommFunc_t *adxl355_comm_func,
                                 uint8_t *dev_id_ad);

/**
 * @fn ADXL_Status_t ADXL355_GetDevIdMst(const ADXL355_t*, uint8_t*)
 * @brief
 *
 * @param adxl355_comm_func
 * @param dev_id_mst
 * @return
 */
ADXL_Status_t ADXL355_GetDevIdMst(const ADXL355_CommFunc_t *adxl355_comm_func,
                                  uint8_t *dev_id_mst);

/**
 * @fn ADXL_Status_t ADXL355_GetPartId(const ADXL355_t*, uint8_t*)
 * @brief
 *
 * @param adxl355_comm_func
 * @param dev_part_id
 * @return
 */
ADXL_Status_t ADXL355_GetPartId(const ADXL355_CommFunc_t *adxl355_comm_func,
                                uint8_t *dev_part_id);

/**
 * @fn ADXL_Status_t ADXL355_GetRevId(const ADXL355_t*, uint8_t*)
 * @brief
 *
 * @param adxl355_comm_func
 * @param dev_rev_id
 * @return
 */
ADXL_Status_t ADXL355_GetRevId(const ADXL355_CommFunc_t *adxl355_comm_func,
                               uint8_t *dev_rev_id);

/* Temperature functions */
/**
 * @fn ADXL_Status_t ADXL355_ReadRawTemperature(const ADXL355_t*, uint16_t*)
 * @brief
 *  Returns the temperature in LSBs
 * @param adxl355_comm_func
 * @param ret_temperature
 * @return
 */
ADXL_Status_t ADXL355_ReadRawTemperature(const ADXL355_CommFunc_t *adxl355_comm_func,
                                         uint16_t *temperature);

/**
 * @fn ADXL_Status_t ADXL355_ReadTemperature(const ADXL355_t*, const ADXL355_Temperature_Scale_t, float*)
 * @brief
 *
 * @param adxl355_comm_func
 * @param scale
 * @param temperature
 * @return
 */
ADXL_Status_t ADXL355_ReadTemperature(const ADXL355_CommFunc_t *adxl355_comm_func,
                                      float *temperature);

/* Acceleration data functions */

/**
 * @fn ADXL_Status_t ADXL355_ReadSampleRaw(const ADXL355_t*, ADXL355_SampleRaw_t*)
 * @brief
 *
 * Returns the current 3 axis measurement in the XDATA, YDATA and ZDATA registers without unpacking or aligning.
 *
 * @param adxl355_comm_func
 * @param sample
 * @return
 */
ADXL_Status_t ADXL355_ReadRawSampleSet(const ADXL355_CommFunc_t *adxl355_comm_func,
                                       ADXL355_RawSampleSet_t *sample);

/**
 * @fn ADXL_Status_t ADXL355_GetAxisOffset(const ADXL355_t*, const ADXL355_Axis_t, uint8_t[])
 * @brief
 *
 * @param adxl355_comm_func
 * @param axis
 * @param axis_offset
 * @return
 */
ADXL_Status_t ADXL355_GetAxisOffset(const ADXL355_CommFunc_t *adxl355_comm_func,
                                    const ADXL355_Axis_t axis,
                                    uint8_t axis_offset[2]);

/**
 * @fn ADXL_Status_t ADXL355_SetAxisOffset(const ADXL355_t*, const ADXL355_Axis_t, const int16_t)
 * @brief
 *
 * @param adxl355_comm_func
 * @param axis
 * @param offset
 * @return
 */
ADXL_Status_t ADXL355_SetAxisOffset(const ADXL355_CommFunc_t *adxl355_comm_func,
                                    const ADXL355_Axis_t axis,
                                    const int16_t offset);

/* FIFO Functions */
/**
 * @fn ADXL_Status_t ADXL355_GetFifoSamples(const ADXL355_t*, uint8_t*)
 * @brief
 *
 * @param adxl355_comm_func
 * @param fifo_samples
 * @return
 */
ADXL_Status_t ADXL355_GetFifoSamples(const ADXL355_CommFunc_t *adxl355_comm_func,
                                     uint8_t *fifo_samples);

/**
 * @fn ADXL_Status_t ADXL355_SetFifoSamples(const ADXL355_t*, const uint8_t)
 * @brief
 *
 * @param adxl355_comm_func
 * @param fifo_samples
 * @return
 */
ADXL_Status_t ADXL355_SetFifoSamples(const ADXL355_CommFunc_t *adxl355_comm_func,
                                     const uint8_t fifo_samples);

/**
 * @fn ADXL_Status_t ADXL355_ReadNumberFifoEntries(const ADXL355_t*, uint8_t*)
 * @brief
 *
 * @param adxl355_comm_func
 * @param num_fifo_entries
 * @return
 */
ADXL_Status_t ADXL355_ReadNumberFifoEntries(const ADXL355_CommFunc_t *adxl355_comm_func,
                                            uint8_t *num_fifo_entries);

/**
 * @fn ADXL_Status_t ADXL355_ReadFifo(const ADXL355_t*, uint8_t*, const uint8_t)
 * @brief
 *
 * Raw reading (3 x num_entries) bytes from the FIFO base address without checking
 * the current valid entries/samples not the FIFO entries alignment.
 * To avoid loosing alignment (i.e, to make sure the first entry is the X axis), it is
 * advisable to read the FIFO in multiples of 3 entries/samples (9 bytes) to form a valid sample set.
 * @param adxl355_comm_func
 * @param fifo_data
 * @param num_entries
 * @return
 */
ADXL_Status_t ADXL355_ReadFifo(const ADXL355_CommFunc_t *adxl355_comm_func,
                               uint8_t *fifo_data, const uint8_t num_entries);

/**
 * @fn ADXL_Status_t ADXL355_ReadFifoSamplesRaw(const ADXL355_t*, ADXL355_RawSampleSet_t*, const uint8_t)
 * @brief
 *
 * Reads the FIFO in multiples of samples sets, where a sample set is a sequence of X,Y and Z measurements.
 * If the oldest entry is not X axis entry, returns ADXL_FIFO_ALIGNEMENT_ERROR.
 *
 * @param adxl355_comm_func
 * @param fifo_data
 * @param num_entries
 * @return
 */
ADXL_Status_t ADXL355_ReadFifoSampleSet(const ADXL355_CommFunc_t *adxl355_comm_func,
                                        ADXL355_RawSampleSet_t *sample_sets,
                                        const uint8_t num_sample_sets);

/* Activity functions*/
/**
 * @fn ADXL_Status_t ADXL355_GetActivityThreshold(const ADXL355_t*, uint16_t*)
 * @brief
 *
 * @param adxl355_comm_func
 * @param act_threshold
 * @return
 */
ADXL_Status_t ADXL355_GetActivityThreshold(const ADXL355_CommFunc_t *adxl355_comm_func,
                                           uint16_t *act_threshold);

/**
 * @fn ADXL_Status_t ADXL355_SetActivityThreshold(const ADXL355_t*, const uint16_t)
 * @brief
 *
 * @param adxl355_comm_func
 * @param act_threshold
 * @return
 */
ADXL_Status_t ADXL355_SetActivityThreshold(const ADXL355_CommFunc_t *adxl355_comm_func,
                                           const uint16_t act_threshold);

/**
 * @fn ADXL_Status_t ADXL355_GetActivityCount(const ADXL355_t*, uint8_t*)
 * @brief
 *
 * @param adxl355_comm_func
 * @param act_count
 * @return
 */
ADXL_Status_t ADXL355_GetActivityCount(const ADXL355_CommFunc_t *adxl355_comm_func,
                                       uint8_t *act_count);
/**
 * @fn ADXL_Status_t ADXL355_SetActivityCount(const ADXL355_t*, const uint8_t)
 * @brief
 *
 * @param adxl355_comm_func
 * @param act_count
 * @return
 */
ADXL_Status_t ADXL355_SetActivityCount(const ADXL355_CommFunc_t *adxl355_comm_func,
                                       const uint8_t act_count);
/**
 * @fn ADXL_Status_t ADXL355_EnableActivity(const ADXL355_t*, const ADXL355_ActivityMode_t)
 * @brief
 *
 * @param adxl355_comm_func
 * @param act_mode
 * @return
 */
ADXL_Status_t ADXL355_EnableActivity(const ADXL355_CommFunc_t *adxl355_comm_func,
                                     const ADXL355_ActivityMode_t act_mode);
/**
 * @fn ADXL_Status_t ADXL355_DisableActivity(const ADXL355_t*)
 * @brief
 *
 * @param adxl355_comm_func
 * @return
 */
ADXL_Status_t ADXL355_DisableActivity(const ADXL355_CommFunc_t *adxl355_comm_func);

/* Interrupts */
/**
 * @fn ADXL_Status_t ADXL355_SetInterruptPolarity(const ADXL355_t*, const ADXL355_IntPol_t)
 * @brief
 *
 * @param adxl355_comm_func
 * @param int_pol
 * @return
 */
ADXL_Status_t ADXL355_SetInterruptPolarity(const ADXL355_CommFunc_t *adxl355_comm_func,
                                           const ADXL355_IntPol_t int_pol);
/**
 * @fn ADXL_Status_t ADXL355_GetInterruptPolarity(const ADXL355_t*, ADXL355_IntPol_t*)
 * @brief
 *
 * @param adxl355_comm_func
 * @param int_pol
 * @return
 */
ADXL_Status_t ADXL355_GetInterruptPolarity(const ADXL355_CommFunc_t *adxl355_comm_func,
                                           ADXL355_IntPol_t *int_pol);

/**
 * @brief Configures, with a single write, all the available interrupts.
 *
 * Following ADXL355 INT_MAP register, each bit of int_mask is connected to a possible interrupt signal as follows
 *
 *  ACT_EN2 OVR_EN2 FULL_EN2 RDY_EN2 ACT_EN1 OVR_EN1 FULL_EN1 RDY_EN1
 *
 *  EN2 acts on INT2 and EN1 acts on INT1
 *
 * @param adxl355_comm_func the device spi objetc
 * @param int_mask
 * @return ADXL_OK or ADXL error codes in case of error
 */
ADXL_Status_t ADXL355_SetInterruptMap(const ADXL355_CommFunc_t *adxl355_comm_func,
                                      const uint8_t int_mask);

/**
 * @brief Disables, with a single write, all the available interrupts.
 *
 *
 * @param adxl355_comm_func the device spi objetc
 * @return ADXL_OK or ADXL error codes in case of error
 */
/**
 * @fn ADXL_Status_t ADXL355_GetInterruptMap(const ADXL355_t*, uint8_t*)
 * @brief
 *
 * @param adxl355_comm_func
 * @param int_map
 * @return
 */
ADXL_Status_t ADXL355_GetInterruptMap(const ADXL355_CommFunc_t *adxl355_comm_func,
                                      uint8_t *int_map);
/**
 * @fn ADXL_Status_t ADXL355_ClearInterrupts(const ADXL355_t*)
 * @brief
 *
 * @param adxl355_comm_func
 * @return
 */
ADXL_Status_t ADXL355_ClearInterrupts(const ADXL355_CommFunc_t *adxl355_comm_func);

/**
 * @fn ADXL_Status_t ADXL355_EnableInterrupt(const ADXL355_t*, const ADXL355_IntSource_t)
 * @brief
 *
 * @param adxl355_comm_func
 * @param int_src
 * @return
 */
ADXL_Status_t ADXL355_EnableInterrupt(const ADXL355_CommFunc_t *adxl355_comm_func,
                                      const ADXL355_IntSource_t int_src);

/**
 * @fn ADXL_Status_t ADXL355_DisableInterrupt(const ADXL355_t*, const ADXL355_IntSource_t)
 * @brief
 *
 * @param adxl355_comm_func
 * @param int_src
 * @return
 */
ADXL_Status_t ADXL355_DisableInterrupt(const ADXL355_CommFunc_t *adxl355_comm_func,
                                       const ADXL355_IntSource_t int_src);

/* ADXL355 operation (ODRs, FILTER, RANGE) */
/**
 * @fn ADXL_Status_t ADXL355_GetRange(const ADXL355_t*, ADXL355_Range_t*)
 * @brief
 *
 * @param adxl355_comm_func
 * @param range
 * @return
 */
ADXL_Status_t ADXL355_GetRange(const ADXL355_CommFunc_t *adxl355_comm_func,
                               ADXL355_Range_t *range);

/**
 * @fn ADXL_Status_t ADXL355_SetRange(const ADXL355_t*, const ADXL355_Range_t)
 * @brief
 *
 * @param adxl355_comm_func
 * @param range
 * @return
 */
ADXL_Status_t ADXL355_SetRange(const ADXL355_CommFunc_t *adxl355_comm_func,
                               const ADXL355_Range_t range);

/**
 * @fn ADXL_Status_t ADXL355_GetODR(const ADXL355_t*, ADXL355_ODR_t*)
 * @brief
 *
 * @param adxl355_comm_func
 * @param odr
 * @return
 */
ADXL_Status_t ADXL355_GetODR(const ADXL355_CommFunc_t *adxl355_comm_func, ADXL355_ODR_t *odr);

/**
 * @fn ADXL_Status_t ADXL355_SetODR(const ADXL355_t*, const ADXL355_ODR_t)
 * @brief
 *
 * @param adxl355_comm_func
 * @param odr
 * @return
 */
ADXL_Status_t ADXL355_SetODR(const ADXL355_CommFunc_t *adxl355_comm_func,
                             const ADXL355_ODR_t odr);

/**
 * @fn ADXL_Status_t ADXL355_GetHPF(const ADXL355_t*, ADXL355_HPF_t*)
 * @brief
 *
 * @param adxl355_comm_func
 * @param hpf
 * @return
 */
ADXL_Status_t ADXL355_GetHPF(const ADXL355_CommFunc_t *adxl355_comm_func, ADXL355_HPF_t *hpf);

/**
 * @fn ADXL_Status_t ADXL355_SetHPF(const ADXL355_t*, const ADXL355_HPF_t)
 * @brief
 *
 * @param adxl355_comm_func
 * @param hpf
 * @return
 */
ADXL_Status_t ADXL355_SetHPF(const ADXL355_CommFunc_t *adxl355_comm_func,
                             const ADXL355_HPF_t hpf);

ADXL_Status_t ADXL355_GetMode(const ADXL355_CommFunc_t *adxl355_comm_func,
                              ADXL355_MeasurementMode_t *adxl_mode);

/**
 * @fn ADXL_Status_t ADXL355_SetMode(const ADXL355_t*, const ADXL355_MeasurementMode_t)
 * @brief
 *
 * @param adxl355_comm_func
 * @param adxl_mode
 * @return
 */
ADXL_Status_t ADXL355_SetMode(const ADXL355_CommFunc_t *adxl355_comm_func,
                              const ADXL355_MeasurementMode_t adxl_mode);

/**
 * @fn ADXL_Status_t ADXL355_Reset(const ADXL355_t*)
 * @brief
 *
 * @param adxl355_comm_func
 * @return
 */
ADXL_Status_t ADXL355_Reset(const ADXL355_CommFunc_t *adxl355_comm_func);

#endif /* ADXL355_LIB_H */
