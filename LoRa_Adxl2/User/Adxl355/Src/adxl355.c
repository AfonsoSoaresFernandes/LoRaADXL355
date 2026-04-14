/**
 * @brief Source file for the adxl355_comm_func.h header file.
 *
 * @author Carlos Mendes
 *
 * @date
 *
 */

#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>

#include "stm32wlxx_hal.h"

#include "adxl355.h"

const static uint8_t stored_shadow_registers[] = {0xD0, 0x00, 0x82, 0x01, 0x40};

ADXL_Status_t ADXL355_ReadRegister(const ADXL355_CommFunc_t *adxl355_comm_func,
                                   const uint8_t reg_address, uint8_t *buffer) {

    ADXL_Status_t ret_status;
    ret_status = adxl355_comm_func->ADXL355_Read(reg_address, buffer, 1);
    return ret_status;
}

ADXL_Status_t ADXL355_WriteRegister(const ADXL355_CommFunc_t *adxl355_comm_func,
                                    const uint8_t reg_address,
                                    const uint8_t data) {

    ADXL_Status_t ret_status;
    ret_status = adxl355_comm_func->ADXL355_Write(reg_address, &data, 1);
    return ret_status;
}

ADXL_Status_t ADXL355_ReadRegisterMasked(
const ADXL355_CommFunc_t *adxl355_comm_func, const uint8_t reg_address,
const uint8_t mask, uint8_t *buffer) {

    ADXL_Status_t ret_status;
    uint8_t register_data;

    ret_status = adxl355_comm_func->ADXL355_Read(reg_address, &register_data,
                                                 1);

    if (ret_status == ADXL_OK) {
        *buffer = (uint8_t) (register_data & mask);
    }

    return ret_status;
}

ADXL_Status_t ADXL355_WriteRegisterMasked(
const ADXL355_CommFunc_t *adxl355_comm_func, const uint8_t reg_address,
const uint8_t mask, const uint8_t data) {

    uint8_t curr_reg_data;
    uint8_t new_reg_data;
    ADXL_Status_t ret_status;

    ret_status = adxl355_comm_func->ADXL355_Read(reg_address, &curr_reg_data,
                                                 1);

    if (ret_status == ADXL_OK) {
        new_reg_data = (curr_reg_data & ~mask) | data;
        ret_status = adxl355_comm_func->ADXL355_Write(reg_address,
                                                      &new_reg_data, 1);
    }

    return ret_status;
}

/* Device ID functions */

ADXL_Status_t ADXL355_GetDevIdAd(const ADXL355_CommFunc_t *adxl355_comm_func,
                                 uint8_t *dev_id_ad) {

    ADXL_Status_t ret_status;

    ret_status = ADXL355_ReadRegisterMasked(adxl355_comm_func,
                                            ADXL355_REG_DEVID_AD,
                                            ADXL355_ID_MASK,
                                            dev_id_ad);

    return ret_status;
}

ADXL_Status_t ADXL355_GetDevIdMst(const ADXL355_CommFunc_t *adxl355_comm_func,
                                  uint8_t *dev_id_mst) {

    ADXL_Status_t ret_status;

    ret_status = ADXL355_ReadRegisterMasked(adxl355_comm_func,
                                            ADXL355_REG_DEVID_MST,
                                            ADXL355_ID_MASK,
                                            dev_id_mst);

    return ret_status;
}

ADXL_Status_t ADXL355_GetPartId(const ADXL355_CommFunc_t *adxl355_comm_func,
                                uint8_t *dev_part_id) {

    ADXL_Status_t ret_status;

    ret_status = ADXL355_ReadRegisterMasked(adxl355_comm_func,
                                            ADXL355_REG_PARTID,
                                            ADXL355_ID_MASK,
                                            dev_part_id);
    return ret_status;
}

ADXL_Status_t ADXL355_GetRevId(const ADXL355_CommFunc_t *adxl355_comm_func,
                               uint8_t *dev_rev_id) {

    ADXL_Status_t ret_status;

    ret_status = ADXL355_ReadRegisterMasked(adxl355_comm_func,
                                            ADXL355_REG_REVID,
                                            ADXL355_ID_MASK,
                                            dev_rev_id);

    return ret_status;
}

/* Temperature functions */

ADXL_Status_t ADXL355_ReadRawTemperature(
const ADXL355_CommFunc_t *adxl355_comm_func, uint16_t *temperature) {

    ADXL_Status_t ret_status;

    uint8_t temperature_reg[2] = {0xFF, 0XFF };

    ret_status = adxl355_comm_func->ADXL355_Read(ADXL355_REG_TEMP2,
                                                 temperature_reg, 2);

    if (ret_status == ADXL_OK) {
        *temperature = ( (temperature_reg[0] & 0x0F) << 8) | temperature_reg[1];
    }

    return ret_status;
}

ADXL_Status_t ADXL355_ReadTemperature(
const ADXL355_CommFunc_t *adxl355_comm_func, float *temperature) {

    ADXL_Status_t ret_status;

    uint16_t uncal_temperature;
    float temperature_celsius;

    ret_status = ADXL355_ReadRawTemperature(adxl355_comm_func,
                                            &uncal_temperature);

    if (ret_status == ADXL_OK) {
        // y = m(x-x0) + y0
        temperature_celsius = (ADXL355_TEMPERATURE_SCALE)
        * (uncal_temperature - ADXL355_REFERENCE_TEMPERATURE_LSB)
                              + ADXL355_REFERENCE_TEMPERATURE;

        *temperature = temperature_celsius;
    }

    return ret_status;
}

/* Acceleration data functions */

ADXL_Status_t ADXL355_ReadRawSampleSet(
const ADXL355_CommFunc_t *adxl355_comm_func,
ADXL355_RawSampleSet_t *raw_sample_set) {

    ADXL_Status_t ret_status;

    ret_status = adxl355_comm_func->ADXL355_Read(ADXL355_REG_XDATA3, (uint8_t *)raw_sample_set, ADXL355_BYTES_PER_SAMPLE_SET);

    return ret_status;

}

ADXL_Status_t ADXL355_GetAxisOffset(const ADXL355_CommFunc_t *adxl355_comm_func,
                                    const ADXL355_Axis_t axis,
                                    uint8_t axis_offset[2]) {

    ADXL_Status_t ret_status;
    uint8_t address = ADXL355_REG_XDATA3 + (uint8_t) axis;

    ret_status = adxl355_comm_func->ADXL355_Read(address, axis_offset, 2);

    return ret_status;

}

ADXL_Status_t ADXL355_SetAxisOffset(const ADXL355_CommFunc_t *adxl355_comm_func,
                                    const ADXL355_Axis_t axis,
                                    const int16_t offset) {

    ADXL_Status_t ret_status;

    uint8_t buffer[2] = {offset >> 8, (uint8_t) (offset & 0xFF) };

    ret_status = adxl355_comm_func->ADXL355_Write(
    ADXL355_REG_OFFSET_X_H + (uint8_t) axis, buffer, 2);

    return ret_status;
}

/* FIFO Functions */

ADXL_Status_t ADXL355_GetFifoSamples(
const ADXL355_CommFunc_t *adxl355_comm_func, uint8_t *fifo_samples) {

    ADXL_Status_t ret_status;

    ret_status = ADXL355_ReadRegisterMasked(adxl355_comm_func,
                                            ADXL355_REG_FIFO_SAMPLES,
                                            ADXL355_FIFO_SAMPLES_MASK,
                                            fifo_samples);

    return ret_status;
}

ADXL_Status_t ADXL355_SetFifoSamples(
const ADXL355_CommFunc_t *adxl355_comm_func, const uint8_t fifo_max_entries) {

    ADXL_Status_t ret_status;

    // According to documentation, FIFO_SAMPLES stores ENTRIES

    if (fifo_max_entries > ADXL355_FIFO_MAX_SAMPLES) {
        return ADXL_DATA_ERROR;
    }

    ret_status = ADXL355_WriteRegisterMasked(adxl355_comm_func,
                                             ADXL355_REG_FIFO_SAMPLES,
                                             ADXL355_FIFO_SAMPLES_MASK,
                                             fifo_max_entries);

    return ret_status;
}

ADXL_Status_t ADXL355_ReadNumberFifoEntries(
const ADXL355_CommFunc_t *adxl355_comm_func, uint8_t *num_fifo_entries) {

    ADXL_Status_t ret_status;

    ret_status = ADXL355_ReadRegisterMasked(adxl355_comm_func,
                                            ADXL355_REG_FIFO_ENTRIES,
                                            ADXL355_FIFO_SAMPLES_MASK,
                                            num_fifo_entries);

    return ret_status;
}

ADXL_Status_t ADXL355_ReadFifo(const ADXL355_CommFunc_t *adxl355_comm_func,
                               uint8_t *fifo_data, const uint8_t num_entries) {

    ADXL_Status_t ret_status;

    ret_status = adxl355_comm_func->ADXL355_Read(
    ADXL355_REG_FIFO_DATA, fifo_data, num_entries * ADXL355_BYTES_PER_SAMPLE);

    return ret_status;
}

ADXL_Status_t ADXL355_ReadFifoSampleSet(
const ADXL355_CommFunc_t *adxl355_comm_func,
ADXL355_RawSampleSet_t *sample_sets, const uint8_t num_sample_sets) {

    ADXL_Status_t ret_status;
    uint16_t bytes_to_read = num_sample_sets * ADXL355_BYTES_PER_SAMPLE_SET;

    ret_status = adxl355_comm_func->ADXL355_Read_DMA(ADXL355_REG_FIFO_DATA, (uint8_t *)sample_sets, bytes_to_read);

    return ret_status;
}

/* Activity functions*/

ADXL_Status_t ADXL355_GetActivityThreshold(
const ADXL355_CommFunc_t *adxl355_comm_func, uint16_t *act_threshold) {

    ADXL_Status_t ret_status;
    uint8_t buffer[2];
    ret_status = adxl355_comm_func->ADXL355_Read(ADXL355_REG_ACT_THRESH_H,
                                                 buffer, 2);
    if (ret_status == ADXL_OK) {
        *act_threshold = (uint16_t) (buffer[0] << 8 | buffer[1]);
    }

    return ret_status;

}

ADXL_Status_t ADXL355_SetActivityThreshold(
const ADXL355_CommFunc_t *adxl355_comm_func, const uint16_t act_threshold) {

    ADXL_Status_t ret_status;

    uint8_t buffer[2] = {act_threshold >> 8, (uint8_t) (act_threshold & 0xFF) };

    ret_status = adxl355_comm_func->ADXL355_Write(ADXL355_REG_ACT_THRESH_H,
                                                  buffer, 2);

    return ret_status;

}

ADXL_Status_t ADXL355_GetActivityCount(
const ADXL355_CommFunc_t *adxl355_comm_func, uint8_t *act_count) {

    ADXL_Status_t ret_status;

    ret_status = ADXL355_ReadRegisterMasked(adxl355_comm_func,
                                            ADXL355_REG_ACT_COUNT,
                                            ADXL355_ACT_COUNT_MASK,
                                            act_count);

    return ret_status;
}

ADXL_Status_t ADXL355_SetActivityCount(
const ADXL355_CommFunc_t *adxl355_comm_func, const uint8_t act_count) {

    ADXL_Status_t ret_status;

    ret_status = ADXL355_WriteRegisterMasked(adxl355_comm_func,
                                             ADXL355_REG_ACT_COUNT,
                                             ADXL355_ACT_COUNT_MASK,
                                             act_count);

    return ret_status;

}

ADXL_Status_t ADXL355_EnableActivity(
const ADXL355_CommFunc_t *adxl355_comm_func,
const ADXL355_ActivityMode_t act_mode) {

    ADXL_Status_t ret_status;

    ret_status = ADXL355_WriteRegisterMasked(adxl355_comm_func,
                                             ADXL355_REG_ACT_EN,
                                             ADXL355_ACTIVITY_MASK,
                                             act_mode);
    return ret_status;

}

ADXL_Status_t ADXL355_DisableActivity(
const ADXL355_CommFunc_t *adxl355_comm_func) {

    ADXL_Status_t ret_status;

    ret_status = ADXL355_WriteRegisterMasked(adxl355_comm_func,
                                             ADXL355_REG_ACT_EN,
                                             ADXL355_ACTIVITY_MASK,
                                             ADXL355_ACTIVITY_OFF);
    return ret_status;

}

/* Interrupts */

ADXL_Status_t ADXL355_SetInterruptPolarity(
const ADXL355_CommFunc_t *adxl355_comm_func, const ADXL355_IntPol_t int_pol) {

    ADXL_Status_t ret_status;

    ret_status = ADXL355_WriteRegisterMasked(adxl355_comm_func,
                                             ADXL355_REG_RANGE,
                                             ADXL355_INT_POL_MASK,
                                             int_pol);
    return ret_status;
}

ADXL_Status_t ADXL355_GetInterruptPolarity(
const ADXL355_CommFunc_t *adxl355_comm_func, ADXL355_IntPol_t *int_pol) {

    ADXL_Status_t ret_status;

    ret_status = ADXL355_ReadRegisterMasked(adxl355_comm_func,
                                            ADXL355_REG_RANGE,
                                            ADXL355_INT_POL_MASK,
                                            int_pol);
    return ret_status;
}

ADXL_Status_t ADXL355_SetInterruptMap(
const ADXL355_CommFunc_t *adxl355_comm_func, const uint8_t int_map) {

    ADXL_Status_t ret_status;

    ret_status = adxl355_comm_func->ADXL355_Write(ADXL355_REG_INT_MAP, &int_map,
                                                  1);

    return ret_status;
}

ADXL_Status_t ADXL355_GetInterruptMap(
const ADXL355_CommFunc_t *adxl355_comm_func, uint8_t *int_map) {
    ADXL_Status_t ret_status;

    ret_status = ADXL355_ReadRegister(adxl355_comm_func, ADXL355_REG_INT_MAP,
                                      int_map);

    return ret_status;
}

ADXL_Status_t ADXL355_ClearInterrupts(
const ADXL355_CommFunc_t *adxl355_comm_func) {

    ADXL_Status_t ret_status;
    uint8_t spi_data = 0x00;

    ret_status = adxl355_comm_func->ADXL355_Write(ADXL355_REG_INT_MAP,
                                                  &spi_data, 1);

    return ret_status;
}

ADXL_Status_t ADXL355_EnableInterrupt(
const ADXL355_CommFunc_t *adxl355_comm_func, const ADXL355_IntSource_t int_src) {

    ADXL_Status_t ret_status;

    ret_status = ADXL355_WriteRegisterMasked(adxl355_comm_func,
                                             ADXL355_REG_INT_MAP,
                                             ADXL355_INT_MASK,
                                             int_src);
    return ret_status;
}

ADXL_Status_t ADXL355_DisableInterrupt(
const ADXL355_CommFunc_t *adxl355_comm_func, const ADXL355_IntSource_t int_src) {

    ADXL_Status_t ret_status;

    // int_src is also a mask!
    ret_status = ADXL355_WriteRegisterMasked(adxl355_comm_func,
                                             ADXL355_REG_INT_MAP, int_src, 0);

    return ret_status;
}

/* ADXL355 operation (ODRs, FILTER, RANGE)  */

ADXL_Status_t ADXL355_GetRange(const ADXL355_CommFunc_t *adxl355_comm_func,
                               ADXL355_Range_t *range) {

    ADXL_Status_t ret_status;

    ret_status = ADXL355_ReadRegisterMasked(adxl355_comm_func,
                                            ADXL355_REG_RANGE,
                                            ADXL355_RANGE_MASK,
                                            range);
    return ret_status;
}

ADXL_Status_t ADXL355_SetRange(const ADXL355_CommFunc_t *adxl355_comm_func,
                               const ADXL355_Range_t range) {

    ADXL_Status_t ret_status;

    ret_status = ADXL355_WriteRegisterMasked(adxl355_comm_func,
                                             ADXL355_REG_RANGE,
                                             ADXL355_RANGE_MASK,
                                             range);
    return ret_status;
}

ADXL_Status_t ADXL355_GetODR(const ADXL355_CommFunc_t *adxl355_comm_func,
                             ADXL355_ODR_t *odr) {

    ADXL_Status_t ret_status;

    ret_status = ADXL355_ReadRegisterMasked(adxl355_comm_func,
                                            ADXL355_REG_FILTER,
                                            ADXL355_ODR_MASK,
                                            odr);
    return ret_status;
}

ADXL_Status_t ADXL355_SetODR(const ADXL355_CommFunc_t *adxl355_comm_func,
                             const ADXL355_ODR_t odr) {

    ADXL_Status_t ret_status;

    ret_status = ADXL355_WriteRegisterMasked(adxl355_comm_func,
                                             ADXL355_REG_FILTER,
                                             ADXL355_ODR_MASK,
                                             odr);

    return ret_status;
}

ADXL_Status_t ADXL355_GetHPF(const ADXL355_CommFunc_t *adxl355_comm_func,
                             ADXL355_HPF_t *hpf) {

    ADXL_Status_t ret_status;

    ret_status = ADXL355_ReadRegisterMasked(adxl355_comm_func,
                                            ADXL355_REG_FILTER,
                                            ADXL355_HPF_MASK,
                                            hpf);

    return ret_status;
}

ADXL_Status_t ADXL355_SetHPF(const ADXL355_CommFunc_t *adxl355_comm_func,
                             const ADXL355_HPF_t hpf) {

    ADXL_Status_t ret_status;

    ret_status = ADXL355_WriteRegisterMasked(adxl355_comm_func,
                                             ADXL355_REG_FILTER,
                                             ADXL355_HPF_MASK,
                                             hpf);

    return ret_status;
}

ADXL_Status_t ADXL355_GetMode(const ADXL355_CommFunc_t *adxl355_comm_func,
                              ADXL355_MeasurementMode_t *adxl_mode) {

    ADXL_Status_t ret_status;

    ret_status = ADXL355_ReadRegisterMasked(adxl355_comm_func,
                                            ADXL355_REG_POWER_CTL,
                                            ADXL355_POWER_CTL_MASK,
                                            adxl_mode);

    return ret_status;
}

ADXL_Status_t ADXL355_SetMode(const ADXL355_CommFunc_t *adxl355_comm_func,
                              const ADXL355_MeasurementMode_t adxl_mode) {

    ADXL_Status_t ret_status;

    ret_status = ADXL355_WriteRegisterMasked(adxl355_comm_func,
                                             ADXL355_REG_POWER_CTL,
                                             ADXL355_POWER_CTL_MASK,
                                             adxl_mode);

    return ret_status;
}

ADXL_Status_t ADXL355_Reset(const ADXL355_CommFunc_t *adxl355_comm_func) {

    ADXL_Status_t ret_status;
    uint8_t reset_code = ADXL355_RESET_CODE;
    uint8_t nvm_busy = ADXL355_NVM_BUSY_MASK;
    uint8_t shadow_registers[5];
    uint8_t retries = RESET_RETRY_COUNT;

    /* Soft reset*/
    ret_status = ADXL355_WriteRegister(adxl355_comm_func, ADXL355_REG_RESET,
                                       reset_code);

    if (ret_status != ADXL_OK) {
        return ret_status;
    }

    /* WAIT for NVM = 0 */

    do {
        HAL_Delay(1);
        ret_status = ADXL355_ReadRegisterMasked(adxl355_comm_func,
                                                ADXL355_REG_STATUS,
                                                ADXL355_NVM_BUSY_MASK,
                                                &nvm_busy);
        if (ret_status != ADXL_OK) {
            return ret_status;
        }

        retries--;

    } while (nvm_busy && retries);

    if (!retries) {
        return ADXL355_RESET_ERROR;
    }

    // Delay is needed between soft reset command and shadow registers reading
    /* TODO: What is the correct delay? */
    HAL_Delay(10);

    // Read the shadow registers
    ret_status = adxl355_comm_func->ADXL355_Read(ADXL355_REG_SHADOW,
                                                 shadow_registers, 5);
    if (ret_status != ADXL_OK) {
        return ADXL355_RESET_ERROR;
    }

    /* Compare with stored shadow registers values  */
    for (uint8_t idx = 0; idx < sizeof (stored_shadow_registers); ++idx) {
        if (shadow_registers[idx] != stored_shadow_registers[idx]) {
            return ADXL355_RESET_ERROR;
        }
    }
    return ADXL_OK;
}

