/*
 * helper_functions.c
 *
 *  Created on: Dec 11, 2025
 *      Author: cmendes
 */

#include "helper_functions.h"

extern char _end;           // Start of heap (after BSS)
extern uint32_t _ebss;      // End of bss
extern uint32_t _estack;    // End of RAM, start of Stack

uint32_t get_current_heap_usage(void) {
    extern void* sbrk(int incr);
    char *heap_end = (char*) sbrk(0);   // current heap pointer
    return (uint32_t) (heap_end - &_end); // heap used in bytes
}

uint32_t get_current_stack_usage(void) {
    register uint32_t sp __asm("sp");  // read ARM CPU stack pointer
    return (uint32_t) &_estack - sp;
}

void fill_stack_with_pattern(void) {
    uint32_t *p = &_ebss;
    while ((uint32_t) p < (uint32_t) &_estack) {
        *p++ = 0xA5A5A5A5;
    }
}

uint32_t get_max_stack_usage(void) {
    uint32_t *p = &_ebss;
    while (*p == 0xA5A5A5A5) {
        p++;
    }
    return (uint32_t) &_estack - (uint32_t) p;
}
