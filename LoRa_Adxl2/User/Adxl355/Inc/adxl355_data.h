/*
 * adxl355_compression.h
 *
 *  Created on: Dec 14, 2025
 *      Author: cmendes
 */

#ifndef ADXL355_DATA_H
#define ADXL355_DATA_H

#include <adxl355.h>
#include <stdint.h>
#include "adxl355_util.h"
#include "frame.h"

#define ADXL_DATA_TYPE_BIT_SIZE         3
#define ADXL_EVENT_TYPE_BIT_SIZE        3
#define ADXL_ODR_BIT_SIZE               4
#define ADXL_RANGE_BIT_SIZE             3
#define ADXL_HPF_BIT_SIZE               3
#define ADXL_PAYLOAD_LENGTH_BIT_SIZE    16

#define ADXL_HEADER_SIZE_BYTES          4 // TODO: replace with automatic computation

typedef enum {
    ADXL_RAW = 0x01,
    ADXL_COMPRESSED = 0x02,
} ADXL_DataType_t;

typedef enum {
    ACTIVITY_EVENT = 0x01,
    SCHEDULED_EVENT = 0x02,
    TILT_EVENT = 0x03
} ADXL_EventType_t;

typedef struct {
    ADXL_DataType_t data_type;      // 3 bits
    ADXL_EventType_t event_type;    // 3 bits
    uint8_t odr;                    // 4 bits
    uint8_t range;                  // 3 bits
    uint8_t hpf;                    // 3 bits
    uint16_t payload_length;        // 16 bits
} ADXL_DataHeader_t;

// Total size of the payload in bytes, including zero-padding
// bits used to align the payload to a byte boundary.
// Padding bits are always zero and appear only at the end of the payload.
typedef enum {
    ADXL_QUANTIZER_OK = 0,
    ADXL_QUANTIZER_INVALID_NBITS,
    ADXL_QUANTIZER_INVALID_ARG
} ADXL355_DataCompStatus_t;

typedef struct {
    uint16_t *x_diff;
    uint16_t *y_diff;
    uint16_t *z_diff;
    int32_t x_min;
    int32_t y_min;
    int32_t z_min;
    uint8_t x_shift;
    uint8_t y_shift;
    uint8_t z_shift;
    uint16_t n_bits;
} ADXL355_QuantizedSampleSets_t;

ADXL355_DataCompStatus_t ADXL355_QuantizeSampleSetBlock(
const ADXL355_UnpackedSampleSet_t *in, ADXL355_QuantizedSampleSets_t *out,
const uint32_t num_sample_sets, const uint8_t n_bits);

void ADXL_SerializeDataHeader(const ADXL_DataHeader_t *data_header,
                              FRAME_BitWriter_t *bw);

void ADXL355_SerializeRawPayload(const ADXL355_RawSampleSet_t *raw_samples,
                                 const uint32_t sample_sets_count,
                                 FRAME_BitWriter_t *bw);

void ADXL355_SerializeQuantizedPayload(const ADXL355_QuantizedSampleSets_t *q,
                                       const uint32_t sample_sets_count,
                                       FRAME_BitWriter_t *bw);

//TODO: Python decoder for this format
void ADXL355_SerializeRawUnpackedPayload(
const ADXL355_UnpackedSampleSet_t *raw_samples,
const uint32_t sample_sets_count, FRAME_BitWriter_t *bw);

#endif /* ADXL355_DATA_H */
