/*
 * helper_functions.h
 *
 *  Created on: Dec 11, 2025
 *      Author: cmendes
 */

#ifndef UTILS_INC_HELPER_FUNCTIONS_H_
#define UTILS_INC_HELPER_FUNCTIONS_H_

#include <stdint.h>

uint32_t get_current_heap_usage(void);

uint32_t get_current_stack_usage(void);

void fill_stack_with_pattern(void);

uint32_t get_max_stack_usage(void);

#endif /* UTILS_INC_HELPER_FUNCTIONS_H_ */
