#ifndef CPU_H
#define CPU_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

// Type definitions
typedef union {
    uint16_t r16;
    struct {
        uint8_t l; // swapped for
        uint8_t h; // little-endian ordering
    } r8;
} reg_t;

// Global variables
extern reg_t af, bc, de, hl, sp, pc;
extern size_t dots;
extern bool ime;

void execute(void);
void update_timer_regs(void);

#endif // CPU_H