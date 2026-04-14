/*
 * adxl355_data_framing.h
 *
 *  Created on: Dec 17, 2025
 *      Author: cmendes
 */

#ifndef SENSOR_DATA_FRAMING_H_
#define SENSOR_DATA_FRAMING_H_

#include <stdint.h>

#define FRAME_VERSION_BIT_SIZE    3
#define FRAME_GROUP_ID_BIT_SIZE   5
#define FRAME_RESERVED_BIT_SIZE   16
#define FRAME_HEADER_SIZE_BYTES   3

// ((FRAME_VERSION_BIT_SIZE + FRAME_GROUP_ID_BIT_SIZE + FRAME_RESERVED_BIT_SIZE) / 8)
// TODO should this be bits to make it more general?

typedef enum {
    ACCELEROMETER = 0x01,
    GNSS,
    STATUS
} FRAME_DataGroupID_t;

typedef struct {
    uint8_t version;                // 3 bits
    FRAME_DataGroupID_t data_id;    // 5 bits
    uint16_t reserved;              // 16 bits
} FRAME_Header_t;

typedef struct {
    uint8_t *buf;
    uint32_t bitpos;
} FRAME_BitWriter_t;

/**
 * @fn void FRAME_BitWriter(FRAME_BitWriter_t*, const uint32_t, const uint8_t)
 * @brief
 *
 * Writes the contents of value, bit by bit, with a range of nbits, i.e, it serializes that last nbits of value.
 * @param bw
 * @param value
 * @param nbits
 */
void FRAME_BitWriter(FRAME_BitWriter_t *bw, const uint32_t value,
                     const uint8_t nbits);

void FRAME_InitBitWriter(FRAME_BitWriter_t *bw, uint8_t *in_buffer);

/**
 * @fn uint16_t FRAME_SerializeHeader(const FRAME_Header_t*, uint8_t*)
 * @brief
 *
 * Serializes the header. bit_pointer should point to the next available bit in a preallocated buffer.
 * After serialization, bit_pointer will be update to point to the next available bit position in a the
 * preallocated buffer.
 * @param frame_header
 * @param bit_pointer
 * @return
 */
void FRAME_SerializeFrameHeader(const FRAME_Header_t *frame_header,
                                FRAME_BitWriter_t *bw);

#endif /* SENSOR_DATA_FRAMING_H_ */
