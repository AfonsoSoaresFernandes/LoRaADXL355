/*
 * adxl355_data_framing.c
 *
 *  Created on: Dec 17, 2025
 *      Author: cmendes
 */

#include "frame.h"

#include <stdint.h>

void FRAME_InitBitWriter(FRAME_BitWriter_t *bw, uint8_t *in_buffer) {
    bw->bitpos = 0;
    bw->buf = in_buffer;
}

inline void FRAME_BitWriter(FRAME_BitWriter_t *bw, const uint32_t value,
                            const uint8_t nbits) {

    for (int i = nbits - 1; i >= 0; i--) {
        uint8_t bit = (value >> i) & 1;

        uint16_t curr_byte_idx = bw->bitpos >> 3; // same as (bw->bitpos / 8)
        uint8_t curr_bit_idx = 7 - (bw->bitpos & 7); // same as (7 - (bw->bitpos % 8))

        if (bit)
            bw->buf[curr_byte_idx] |= (1U << curr_bit_idx);
        else
            bw->buf[curr_byte_idx] &= ~ (1U << curr_bit_idx);

        bw->bitpos++;
    }
}

void FRAME_SerializeFrameHeader(const FRAME_Header_t *frame_header,
                                FRAME_BitWriter_t *bw) {

    FRAME_BitWriter(bw, frame_header->version, FRAME_VERSION_BIT_SIZE);
    FRAME_BitWriter(bw, frame_header->data_id, FRAME_GROUP_ID_BIT_SIZE);
    FRAME_BitWriter(bw, frame_header->reserved, FRAME_RESERVED_BIT_SIZE);

}

