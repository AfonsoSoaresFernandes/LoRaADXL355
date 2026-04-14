/*
 * adxl355_util.c
 *
 *  Created on: Jan 2, 2026
 *      Author: cmendes
 */

#include "adxl355.h"
#include "adxl355_util.h"

ADXL_Status_t ADXL355_UTIL_Config(ADXL355_DevCfg_t *adxl355_dev) {

    ADXL_Status_t ret_status;

    if (adxl355_dev == NULL) {
        return ADXL_INVALID_PARAMETER;
    }

    // Config modifications must be done in STANDBY mode

    ret_status = ADXL355_SetMode(adxl355_dev->comm_func,
                                 ADXL355_STDBY_TEMP_OFF_DRDY_OFF);
    if (ret_status != ADXL_OK) {
        return ret_status;
    }

    // ===================== Configure ADXL355 =====================

    ret_status = ADXL355_SetODR(adxl355_dev->comm_func, adxl355_dev->odr);
    if (ret_status != ADXL_OK) {
        return ret_status;
    }

    ret_status = ADXL355_SetHPF(adxl355_dev->comm_func, adxl355_dev->hpf);
    if (ret_status != ADXL_OK) {
        return ret_status;
    }

    ret_status = ADXL355_SetRange(adxl355_dev->comm_func, adxl355_dev->range);
    if (ret_status != ADXL_OK) {
        return ret_status;
    }

    ret_status = ADXL355_SetInterruptPolarity(adxl355_dev->comm_func,
                                              adxl355_dev->int_pol);
    if (ret_status != ADXL_OK) {
        return ret_status;
    }

    ret_status = ADXL355_SetInterruptMap(adxl355_dev->comm_func,
                                         adxl355_dev->int_src);
    if (ret_status != ADXL_OK) {
        return ret_status;
    }

    ret_status = ADXL355_SetActivityThreshold(adxl355_dev->comm_func,
                                              adxl355_dev->act_thr_lsb);
    if (ret_status != ADXL_OK) {
        return ret_status;
    }

    ret_status = ADXL355_SetActivityCount(adxl355_dev->comm_func,
                                          adxl355_dev->act_cnt);
    if (ret_status != ADXL_OK) {
        return ret_status;
    }

    ret_status = ADXL355_EnableActivity(adxl355_dev->comm_func,
                                        adxl355_dev->act_mode);
    if (ret_status != ADXL_OK) {
        return ret_status;
    }

    ret_status = ADXL355_SetFifoSamples(adxl355_dev->comm_func,
                                        adxl355_dev->fifo_samples);
    if (ret_status != ADXL_OK) {
        return ret_status;
    }

    ret_status = ADXL355_SetAxisOffset(adxl355_dev->comm_func, ADXL355_X_AXIS,
                                       adxl355_dev->x_offset_lsb);
    if (ret_status != ADXL_OK) {
        return ret_status;
    }

    ret_status = ADXL355_SetAxisOffset(adxl355_dev->comm_func, ADXL355_Y_AXIS,
                                       adxl355_dev->y_offset_lsb);
    if (ret_status != ADXL_OK) {
        return ret_status;
    }

    ret_status = ADXL355_SetAxisOffset(adxl355_dev->comm_func, ADXL355_Z_AXIS,
                                       adxl355_dev->z_offset_lsb);
    if (ret_status != ADXL_OK) {
        return ret_status;
    }

    // ===================== Start Measurement mode =====================

    ret_status = ADXL355_SetMode(adxl355_dev->comm_func,
                                 adxl355_dev->meas_mode);
    if (ret_status != ADXL_OK) {
        return ret_status;
    }

    return ADXL_OK;
}

ADXL_Status_t ADXL355_UnpackSampleSet(
const ADXL355_RawSampleSet_t *raw_sample_set, const uint32_t num_sample_sets,
ADXL355_UnpackedSampleSet_t *unpacked_sample_set) {

    int32_t x_axis, y_axis, z_axis;

    for (uint32_t counter = 0; counter < num_sample_sets; ++counter) {

        // For each axis
        //  Reconstruct the 20-bit number (bits 19..0)
        //  sign-extend from bit 19

        x_axis = (uint32_t) (raw_sample_set->x_axis[0] << 12)
        | (uint32_t) (raw_sample_set->x_axis[1] << 4)
        | (uint32_t) (raw_sample_set->x_axis[2] >> 4);

        if (x_axis & (1u << 19))
            x_axis |= 0xFFF00000u;

        y_axis = (uint32_t) (raw_sample_set->y_axis[0] << 12)
        | (uint32_t) (raw_sample_set->y_axis[1] << 4)
        | (uint32_t) (raw_sample_set->y_axis[2] >> 4);

        if (y_axis & (1u << 19))
            y_axis |= 0xFFF00000u;

        z_axis = (uint32_t) (raw_sample_set->z_axis[0] << 12)
        | (uint32_t) (raw_sample_set->z_axis[1] << 4)
        | (uint32_t) (raw_sample_set->z_axis[2] >> 4);

        if (z_axis & (1u << 19))
            z_axis |= 0xFFF00000u;

        unpacked_sample_set->x_axis = (int32_t) x_axis;
        unpacked_sample_set->y_axis = (int32_t) y_axis;
        unpacked_sample_set->z_axis = (int32_t) z_axis;

        ++raw_sample_set;
        ++unpacked_sample_set;
    }

    return ADXL_OK;
}

ADXL_Status_t ADXL355_SampleSet2Gs(
const ADXL355_UnpackedSampleSet_t *sample_set_unpacked,
const ADXL355_Range_t range, const uint32_t num_sample_sets,
ADXL355_SampleSetGs_t *sample_set_gs) {

    uint32_t sensitivity;

    switch (range) {
        case ADXL355_RANGE_2G:
            sensitivity = ADXL355_SENSITIVITY_2G;
            break;
        case ADXL355_RANGE_4G:
            sensitivity = ADXL355_SENSITIVITY_4G;
            break;
        case ADXL355_RANGE_8G:
            sensitivity = ADXL355_SENSITIVITY_8G;
            break;
        default:
            return ADXL_INVALID_PARAMETER;
    }

    for (uint32_t counter = 0; counter < num_sample_sets; ++counter) {
        sample_set_gs->x_axis = (float) sample_set_unpacked->x_axis
        / (float) sensitivity;
        sample_set_gs->y_axis = (float) sample_set_unpacked->y_axis
        / (float) sensitivity;
        sample_set_gs->z_axis = (float) sample_set_unpacked->z_axis
        / (float) sensitivity;

        ++sample_set_unpacked;
        ++sample_set_gs;
    }

    return ADXL_OK;
}

ADXL_Status_t ADXL355_ClearFIFO(const ADXL355_CommFunc_t *adxl355) {

    ADXL_Status_t ret_status;
    uint8_t data[ADXL_FIFO_MAX_BYTES];

    ret_status = ADXL355_ReadFifo(adxl355, data, ADXL355_FIFO_MAX_SAMPLES);
    return ret_status;
}

ADXL_Status_t ADXL355_ActivityThreshold2LSB(const uint16_t threshold_mg,
                                            const ADXL355_Range_t range,
                                            uint16_t *threshold_lsb) {

    uint32_t sensitivity_lsb_per_g;

    // Select the sensitivity based on current range
    switch (range) {
        case ADXL355_RANGE_2G:
            sensitivity_lsb_per_g = ADXL355_SENSITIVITY_2G;
            break;

        case ADXL355_RANGE_4G:
            sensitivity_lsb_per_g = ADXL355_SENSITIVITY_4G;
            break;

        case ADXL355_RANGE_8G:
            sensitivity_lsb_per_g = ADXL355_SENSITIVITY_8G;
            break;
    }

    // Convert mg to g
    float threshold_g = threshold_mg / 1000.0f;
    // Convert g to LSBs
    float unaligned_threshold_lsb = threshold_g * sensitivity_lsb_per_g;

    if (unaligned_threshold_lsb > sensitivity_lsb_per_g) {
        return ADXL_INVALID_PARAMETER;
    }

    // The lower 3 bits of the ACT threshold registers are unused.
    // Therefore the threshold must be aligned to multiples of 8 LSBs.

    //*threshold_lsb = (uint16_t)(unaligned_threshold_lsb >> 3);
    *threshold_lsb = (uint16_t) (unaligned_threshold_lsb / 8.0);

    return ADXL_OK;
}

ADXL_Status_t ADXL355_Offset2LSB(const ADXL355_CommFunc_t *adxl355,
                                 const int16_t offset_mg,
                                 const ADXL355_Range_t range,
                                 int16_t *offset_lsb) {

    uint32_t sensitivity_lsb_per_g;

    // Select the sensitivity based on current range
    switch (range) {
        case ADXL355_RANGE_2G:
            sensitivity_lsb_per_g = ADXL355_SENSITIVITY_2G;
            break;

        case ADXL355_RANGE_4G:
            sensitivity_lsb_per_g = ADXL355_SENSITIVITY_4G;
            break;

        case ADXL355_RANGE_8G:
            sensitivity_lsb_per_g = ADXL355_SENSITIVITY_8G;
            break;
    }

    // Convert mg to g
    float offset_g = offset_mg / 1000.0f;
    // Convert g to LSBs
    float unaligned_threshold_lsb = offset_g * sensitivity_lsb_per_g;

    if (unaligned_threshold_lsb > sensitivity_lsb_per_g) {
        return ADXL_INVALID_PARAMETER;
    }

    // The lower 4 bits of the offset registers are unused.
    // Therefore the threshold must be aligned to multiples of 16 LSBs.

    *offset_lsb = (int16_t) (unaligned_threshold_lsb / 16.0);

    return ADXL_OK;
}

ADXL_Status_t ADXL355_AlignFifo(const ADXL355_CommFunc_t *adxl355) {

    ADXL_Status_t ret_status;
    uint8_t fifo_entry[3] = {0 };
    bool is_empty = true;
    bool is_x_axis = false;

    // look/wait for a valid X axis entry
    while ( (!is_x_axis) || (is_empty)) {

        ret_status = adxl355->ADXL355_Read(ADXL355_REG_FIFO_DATA, fifo_entry,
                                           3);
        // ret_status = ADXL355_ReadFifo(adxl355, fifo_entry, 1);

        if (ret_status != ADXL_OK) {
            return ADXL_ERROR;
        }

        is_x_axis = ( (fifo_entry[2] & 0x01) == 0x01);
        is_empty = ( (fifo_entry[2] & 0x02) == 0x02);

    }

    // Now wait/look for two more samples (Y and Z)
    uint8_t counter = 0;

    while (counter < 2) {

        ret_status = adxl355->ADXL355_Read(ADXL355_REG_FIFO_DATA, fifo_entry,
                                           3);
        //ret_status = ADXL355_ReadFifo(adxl355, fifo_entry, 1);

        if (ret_status != ADXL_OK) {
            return ADXL_ERROR;
        }

        is_x_axis = ( (fifo_entry[2] & 0x01) == 0x01);
        is_empty = ( (fifo_entry[2] & 0x02) == 0x02);

        if ( (!is_x_axis) && (!is_empty)) { // non empty no X sample
            ++counter;
        }
    }
    return ADXL_OK;
}

