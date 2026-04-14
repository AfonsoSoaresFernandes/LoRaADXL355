/*
 * adxl355_api_def.h
 *
 *  Created on: 24 Mar 2026
 *      Author: afonsofernandes
 */

#ifndef ADXL355_API_ADXL355_API_DEF_H_
#define ADXL355_API_ADXL355_API_DEF_H_

#include "adxl355.h"
#include "adxl355_comm.h"
#include "adxl355_util.h"
#include "adxl355_data.h"

#include "frame.h"
#include "helper_functions.h"

#define ADXL355_API_DEBUG 0
#if ADXL355_API_DEBUG == 1
#include <stdio.h>
#define PRINT_ADXL355(...) printf(__VA_ARGS__)
#else
#define PRINT_ADXL355(...)
#endif

#define ACTIVITY_FILTER_SETTLING_WAIT_MS    2000
#define SCHEDULED_FILTER_SETTLING_WAIT_MS   1000
#define TILT_FILTER_SETTLING_WAIT_MS        10000

#define ACTIVITY_COUNT                      1       // number of samples to trigger activity
#define ACTIVITY_THRESHOLD_LSB              3200    // 400mg in 4G range

// Macro to find the maximum of three values
#define MAX_THREE(a, b, c) ((a) > (b) ? ((a) > (c) ? (a) : (c)) : ((b) > (c) ? (b) : (c)))

// Number of sample sets to measure in each event
#define TILT_SAMPLE_SETS        32
#define ACTIVITY_SAMPLE_SETS    1000
#define SCHEDULED_SAMPLE_SETS   1000

// Compressed data, bits per sample
#define NBITS   10

// Measurement intervals
// 1 TICK = 1 Timer Interrupt Interval (1 min, defined in CubeMX)
#define TILT_MEAS_INTERVAL_TICKS       1
#define SCHEDULED_MEAS_INTERVAL_TICKS  1

#define MAX_SAMPLE_SETS         MAX_THREE(TILT_SAMPLE_SETS, ACTIVITY_SAMPLE_SETS, SCHEDULED_SAMPLE_SETS)
#define MAX_PAYLOAD_SIZE        MAX_SAMPLE_SETS * ADXL355_BYTES_PER_SAMPLE_SET

#define FRAME_SYNC_1            0xAA
#define FRAME_SYNC_2            0x55
#define FRAME_SIZE              FRAME_HEADER_SIZE_BYTES + ADXL_HEADER_SIZE_BYTES + MAX_PAYLOAD_SIZE

#define ERROR_CATCH(code) ErrorCatchFunction(code, __FILE__, __LINE__)

#endif /* ADXL355_API_ADXL355_API_DEF_H_ */
