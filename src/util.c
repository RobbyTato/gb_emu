#include <stdio.h>
#include <cpu.h>
#include <mem.h>
#include <rom.h>
#include <util.h>

void dump_cpu_state(void) {
    printf("AF: %X\nA: %X\nF: %X\n", af.r16, af.r8.h, af.r8.l);
    printf("BC: %X\nB: %X\nC: %X\n", bc.r16, bc.r8.h, bc.r8.l);
    printf("DE: %X\nD: %X\nE: %X\n", de.r16, de.r8.h, de.r8.l);
    printf("HL: %X\nH: %X\nL: %X\n", hl.r16, hl.r8.h, hl.r8.l);
    printf("SP: %X\n", sp.r16);
    printf("PC: %X\n", pc.r16);
    printf("Dots: %lu\n", dots);
    printf("IME: %u\n", ime);
    printf("\n");
}