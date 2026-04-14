/*
 * adxl355_util.h
 *
 *  Created on: Jan 2, 2026
 *      Author: cmendes
 */

#ifndef ADXL355_UTIL_H
#define ADXL355_UTIL_H

#include "adxl355.h"
#include "adxl355_comm.h"


/**
 * @struct
 * @brief
 *
 */
typedef struct {
    int32_t x_axis;
    int32_t y_axis;
    int32_t z_axis;
} ADXL355_UnpackedSampleSet_t;

/**
 * @struct
 * @brief
 *
 */
typedef struct {
    float x_axis;
    float y_axis;
    float z_axis;
} ADXL355_SampleSetGs_t;

/*
typedef union {
    //ADXL355_I2C_t *i2c_desc;
    ADXL355_SPI_t *spi_desc;
} ADXL355_CommDescriptor_t;
*/
/**
 * @struct
 * @brief
 *
 */
typedef struct {
    // ADXL355_CommDescriptor_t *com_desc;
    ADXL355_CommFunc_t * comm_func;
    ADXL355_ODR_t odr;
    ADXL355_HPF_t hpf;
    ADXL355_Range_t range;
    ADXL355_IntPol_t int_pol;
    ADXL355_IntSource_t int_src;
    ADXL355_ActivityMode_t act_mode;
    uint8_t act_cnt;
    uint16_t act_thr_lsb;
    uint8_t fifo_samples;
    int16_t x_offset_lsb;
    int16_t y_offset_lsb;
    int16_t z_offset_lsb;
    ADXL355_MeasurementMode_t meas_mode;
} ADXL355_DevCfg_t;

/**
 * @fn ADXL_Status_t ADXL355_UTIL_Config(ADXL355_dev_t)
 * @brief
 *
 * @param adxl355_dev
 * @return
 */
ADXL_Status_t ADXL355_UTIL_Config(ADXL355_DevCfg_t *adxl355_dev);

/**
 * @fn ADXL_Status_t ADXL355_UnpackSampleSet(const ADXL355_RawSampleSet_t*, ADXL355_UnpackedSampleSet_t*, const uint32_t)
 * @brief
 *
 ** Returns the current 3 axis measurement in the XDATA, YDATA and ZDATA, in LSBs, in a properly aligned uint32_t.
 **
 * @param raw_sample
 * @param unpacked_sample
 * @param num_sample_sets
 * @return
 */
ADXL_Status_t ADXL355_UnpackSampleSet(
const ADXL355_RawSampleSet_t *raw_sample_set, const uint32_t num_sample_sets,
ADXL355_UnpackedSampleSet_t *unpacked_sample_set);

/**
 * @fn ADXL_Status_t ADXL355_SampleGs(const ADXL355_SampleUnpacked_t*, ADXL355_SampleGs_t*)
 * @brief
 *
 * Converts from ADXL355_SampleUnpacked_t to ADXL355_SampleGs_t in the range
 *
 * @param sample_unpacked
 * @param sample_gs
 * @return
 */

ADXL_Status_t ADXL355_SampleSet2Gs(
const ADXL355_UnpackedSampleSet_t *sample_set_unpacked,
const ADXL355_Range_t range, const uint32_t num_sample_sets,
ADXL355_SampleSetGs_t *sample_set_gs);

/**
 * @fn ADXL_Status_t ADXL355_ClearFIFO(const ADXL355_t*)
 * @brief
 * Clears the FIFO. Useful for testing and debugging.
 * @param adxl355
 * @return
 */
ADXL_Status_t ADXL355_ClearFIFO(const ADXL355_CommFunc_t *adxl355);

/**
 * @fn ADXL_Status_t ADXL355_ActivityThreshold2LSB(const ADXL355_t*, const uint16_t, const ADXL355_Range_t, uint16_t*)
 * @brief
 *
 * This function takes the desired activity threshold, in mg, and returns the nearest threshold in LSBs
 * ready to program ACT_THRESHOLD registers taking in consideration the predefined range.
  * @param threshold_mg
 * @param range
 * @param threshold_lsb
 * @return
 */
ADXL_Status_t ADXL355_ActivityThreshold2LSB(const uint16_t threshold_mg,
                                            const ADXL355_Range_t range,
                                            uint16_t *threshold_lsb);

ADXL_Status_t ADXL355_Offset2LSB(const ADXL355_CommFunc_t *adxl355,
                                 const int16_t offset_mg,
                                 const ADXL355_Range_t range,
                                 int16_t *offset_lsb);

ADXL_Status_t ADXL355_AlignFifo(const ADXL355_CommFunc_t *adxl355);

#endif /* ADXL355_UTIL_H */
