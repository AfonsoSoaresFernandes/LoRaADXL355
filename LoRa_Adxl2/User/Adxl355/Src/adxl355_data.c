/*
 * adxl355_compression.c
 *
 *  Created on: Dec 14, 2025
 *      Author: cmendes
 */


#include <string.h>


#include "adxl355.h"
#include "adxl355_util.h"
#include "adxl355_data.h"
#include "frame.h"

static inline uint8_t bits_required(uint32_t range) {

    uint8_t bits_required = 0;
    while (range) {
        range >>= 1;
        bits_required++;
    }
    return bits_required;
}

/**
 * @fn void ADXL355_QuantizeBlockSampleSets(const ADXL355_UnpackedSampleSet_t*, ADXL355_QuantizedSampleSets_t*, const uint32_t, const uint8_t)
 * @brief Takes a block of raw sample sets [X3][X2][X1][Y3][Y2][Y1][Z3][Z2][Z1] where [.] is one byte
 *          and turns it into a block of [X][Y][Z] (MSB-aligned 32-bit two’s complement)

 *
 * @param in
 * @param out
 * @param num_sample_sets
 * @param n_bits
 */
ADXL355_DataCompStatus_t ADXL355_QuantizeSampleSetBlock(
const ADXL355_UnpackedSampleSet_t *in, ADXL355_QuantizedSampleSets_t *q_out,
const uint32_t num_sample_sets, const uint8_t n_bits) {

    if (!in || !q_out || num_sample_sets == 0)
        return ADXL_QUANTIZER_INVALID_ARG;

    if (n_bits == 0 || n_bits > 16)
        return ADXL_QUANTIZER_INVALID_NBITS;

    q_out->n_bits = n_bits;

    // find min and max values per axis

    int32_t x_min = in[0].x_axis;
    int32_t x_max = in[0].x_axis;
    int32_t y_min = in[0].y_axis;
    int32_t y_max = in[0].y_axis;
    int32_t z_min = in[0].z_axis;
    int32_t z_max = in[0].z_axis;

    for (uint32_t idx = 0; idx < num_sample_sets; idx++) {
        if (in[idx].x_axis < x_min)
            x_min = in[idx].x_axis;
        if (in[idx].x_axis > x_max)
            x_max = in[idx].x_axis;

        if (in[idx].y_axis < y_min)
            y_min = in[idx].y_axis;
        if (in[idx].y_axis > y_max)
            y_max = in[idx].y_axis;

        if (in[idx].z_axis < z_min)
            z_min = in[idx].z_axis;
        if (in[idx].z_axis > z_max)
            z_max = in[idx].z_axis;
    }

    q_out->x_min = x_min;
    q_out->y_min = y_min;
    q_out->z_min = z_min;

    // compute range per axis
    uint32_t x_range = (uint32_t) (x_max - x_min);
    uint32_t y_range = (uint32_t) (y_max - y_min);
    uint32_t z_range = (uint32_t) (z_max - z_min);

    // compute minimum number of bits required per axis
    uint8_t x_axis_bits = bits_required(x_range);
    uint8_t y_axis_bits = bits_required(y_range);
    uint8_t z_axis_bits = bits_required(z_range);

    uint8_t x_shift = (x_axis_bits > n_bits) ? (x_axis_bits - n_bits) : 0;
    uint8_t y_shift = (y_axis_bits > n_bits) ? (y_axis_bits - n_bits) : 0;
    uint8_t z_shift = (z_axis_bits > n_bits) ? (z_axis_bits - n_bits) : 0;

    q_out->x_shift = x_shift;
    q_out->y_shift = y_shift;
    q_out->z_shift = z_shift;

    // compute the difference to minimum value and code with the predefined Nbits
    uint32_t diff;
    for (uint32_t idx = 0; idx < num_sample_sets; idx++) {

        /*
         if (x_range == 0) {
         out->x_shift = 0;
         for (idx = 0; idx < num_sample_sets; idx++)
         out->x_diff[idx] = 0;
         }
         */
        diff = (in[idx].x_axis - x_min);
        q_out->x_diff[idx] = (int16_t) (diff >> x_shift);

        diff = (in[idx].y_axis - y_min);
        q_out->y_diff[idx] = (int16_t) (diff >> y_shift);

        diff = (in[idx].z_axis - z_min);
        q_out->z_diff[idx] = (int16_t) (diff >> z_shift);
    }

    return ADXL_QUANTIZER_OK;
}

void ADXL_SerializeDataHeader(const ADXL_DataHeader_t *data_header,
                              FRAME_BitWriter_t *bw) {

    FRAME_BitWriter(bw, data_header->data_type, ADXL_DATA_TYPE_BIT_SIZE);
    FRAME_BitWriter(bw, data_header->event_type, ADXL_EVENT_TYPE_BIT_SIZE);
    FRAME_BitWriter(bw, data_header->odr, ADXL_ODR_BIT_SIZE);
    FRAME_BitWriter(bw, data_header->range, ADXL_RANGE_BIT_SIZE);
    FRAME_BitWriter(bw, data_header->hpf, ADXL_HPF_BIT_SIZE);
    FRAME_BitWriter(bw, data_header->payload_length,
                    ADXL_PAYLOAD_LENGTH_BIT_SIZE);
}

void ADXL355_SerializeRawPayload(const ADXL355_RawSampleSet_t *raw_samples,
                                 const uint32_t sample_sets_count,
                                 FRAME_BitWriter_t *bw) {

    for (uint32_t i = 0; i < sample_sets_count; i++) {

        //X Axis
        FRAME_BitWriter(bw, raw_samples[i].x_axis[0], 8);
        FRAME_BitWriter(bw, raw_samples[i].x_axis[1], 8);
        FRAME_BitWriter(bw, raw_samples[i].x_axis[2], 8);

        // Y axis
        FRAME_BitWriter(bw, raw_samples[i].y_axis[0], 8);
        FRAME_BitWriter(bw, raw_samples[i].y_axis[1], 8);
        FRAME_BitWriter(bw, raw_samples[i].y_axis[2], 8);

        // Z axis
        FRAME_BitWriter(bw, raw_samples[i].z_axis[0], 8);
        FRAME_BitWriter(bw, raw_samples[i].z_axis[1], 8);
        FRAME_BitWriter(bw, raw_samples[i].z_axis[2], 8);

    }

    //padding zero bits
    uint8_t pad_bits = (8 - (bw->bitpos & 7)) & 7;
    FRAME_BitWriter(bw, 0, pad_bits);
}

void ADXL355_SerializeQuantizedPayload(
const ADXL355_QuantizedSampleSets_t *q_sample_sets,
const uint32_t sample_sets_count, FRAME_BitWriter_t *bw) {

    // Payload metadata

    uint8_t n_bits = q_sample_sets->n_bits;
    //NBits
    FRAME_BitWriter(bw, n_bits, 4);

    //x_min x_shift, y_min y_shift, z_min z_shift
    FRAME_BitWriter(bw, (uint32_t) (q_sample_sets->x_min & 0xFFFFF), 20);
    FRAME_BitWriter(bw, q_sample_sets->x_shift & 0xF, 4);
    FRAME_BitWriter(bw, (uint32_t) (q_sample_sets->y_min & 0xFFFFF), 20);
    FRAME_BitWriter(bw, q_sample_sets->y_shift & 0xF, 4);
    FRAME_BitWriter(bw, (uint32_t) (q_sample_sets->z_min & 0xFFFFF), 20);
    FRAME_BitWriter(bw, q_sample_sets->z_shift & 0xF, 4);

    // Samples
    for (uint32_t i = 0; i < sample_sets_count; i++) {
        FRAME_BitWriter(bw, (uint32_t) q_sample_sets->x_diff[i], n_bits);
        FRAME_BitWriter(bw, (uint32_t) q_sample_sets->y_diff[i], n_bits);
        FRAME_BitWriter(bw, (uint32_t) q_sample_sets->z_diff[i], n_bits);
    }

    //padding zero bits
    uint8_t pad_bits = (8 - (bw->bitpos & 7)) & 7;
    FRAME_BitWriter(bw, 0, pad_bits);
}

//TODO: Python decoder for this format
void ADXL355_SerializeRawUnpackedPayload(
const ADXL355_UnpackedSampleSet_t *raw_samples, const uint32_t sample_sets_count,
FRAME_BitWriter_t *bw) {

    for (uint32_t i = 0; i < sample_sets_count; i++) {
        FRAME_BitWriter(bw, raw_samples->x_axis, 20);
        FRAME_BitWriter(bw, raw_samples->y_axis, 20);
        FRAME_BitWriter(bw, raw_samples->z_axis, 20);
    }

    uint8_t pad_bits = (8 - (bw->bitpos & 7)) & 7;
    FRAME_BitWriter(bw, 0, pad_bits);
}
