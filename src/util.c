#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <cpu.h>
#include <mem.h>
#include <rom.h>
#include <util.h>

void dump_cpu_state(void) {
    printf("AF: %02X\nA: %02X\nF: %02X\n", af.r16, af.r8.h, af.r8.l);
    printf("BC: %02X\nB: %02X\nC: %02X\n", bc.r16, bc.r8.h, bc.r8.l);
    printf("DE: %02X\nD: %02X\nE: %02X\n", de.r16, de.r8.h, de.r8.l);
    printf("HL: %02X\nH: %02X\nL: %02X\n", hl.r16, hl.r8.h, hl.r8.l);
    printf("SP: %02X\n", sp.r16);
    printf("PC: %02X\n", pc.r16);
    printf("Dots: %lu\n", dots);
    printf("IME: %u\n", ime);
}

void dump_cpu_state_gameboy_doctor(void) {
    printf("A:%02X F:%02X B:%02X C:%02X D:%02X E:%02X H:%02X L:%02X\n",
       af.r8.h, af.r8.l, bc.r8.h, bc.r8.l, de.r8.h, de.r8.l,
       hl.r8.h, hl.r8.l);

    printf("SP:%04X PC:%04X PCMEM:%02X,%02X,%02X,%02X\n",
       sp.r16, pc.r16,
       read_mem(pc.r16), read_mem(pc.r16 + 1),
       read_mem(pc.r16 + 2), read_mem(pc.r16 + 3));
}

// Straight AI because I don't wanna implement async-signal-safe dumping

// Convert unsigned long to hex string (uppercase), returns length
int ultoa_hex(unsigned long value, char *buf) {
    char tmp[16];  // Enough for 64-bit address
    int i = 0;
    
    if (value == 0) {
        buf[0] = '0';
        return 1;
    }

    while (value && i < 16) {
        int digit = value & 0xF;
        tmp[i++] = (digit < 10) ? ('0' + digit) : ('A' + digit - 10);
        value >>= 4;
    }

    // Reverse digits into output buffer
    int j = 0;
    while (i > 0) {
        buf[j++] = tmp[--i];
    }
    return j;
}

// Convert unsigned int to hex string (uppercase), returns length
int utoa_hex(unsigned int value, char *buf) {
    return ultoa_hex(value, buf);
}

// Convert unsigned long to decimal string, returns length
int ultoa_dec(unsigned long value, char *buf) {
    int i = 0;
    char tmp[21]; // Enough for 64-bit decimal
    if (value == 0) {
        buf[0] = '0';
        return 1;
    }
    while (value && i < 20) {
        tmp[i++] = '0' + (value % 10);
        value /= 10;
    }
    // Reverse
    for (int j = 0; j < i; ++j)
        buf[j] = tmp[i - j - 1];
    return i;
}

// Convert unsigned int to decimal string, returns length
int utoa_dec(unsigned int value, char *buf) {
    return ultoa_dec(value, buf);
}

void signal_safe_dump_cpu_state(void) {
    ssize_t sink;
    char buf[64];
    int n;

    // AF
    sink = write(2, "AF: ", 4);
    n = utoa_hex(af.r16, buf); sink = write(2, buf, n);
    sink = write(2, "\nA: ", 4);
    n = utoa_hex(af.r8.h, buf); sink = write(2, buf, n);
    sink = write(2, "\nF: ", 4);
    n = utoa_hex(af.r8.l, buf); sink = write(2, buf, n);
    sink = write(2, "\n", 1);

    // BC
    sink = write(2, "BC: ", 4);
    n = utoa_hex(bc.r16, buf); sink = write(2, buf, n);
    sink = write(2, "\nB: ", 4);
    n = utoa_hex(bc.r8.h, buf); sink = write(2, buf, n);
    sink = write(2, "\nC: ", 4);
    n = utoa_hex(bc.r8.l, buf); sink = write(2, buf, n);
    sink = write(2, "\n", 1);

    // DE
    sink = write(2, "DE: ", 4);
    n = utoa_hex(de.r16, buf); sink = write(2, buf, n);
    sink = write(2, "\nD: ", 4);
    n = utoa_hex(de.r8.h, buf); sink = write(2, buf, n);
    sink = write(2, "\nE: ", 4);
    n = utoa_hex(de.r8.l, buf); sink = write(2, buf, n);
    sink = write(2, "\n", 1);

    // HL
    sink = write(2, "HL: ", 4);
    n = utoa_hex(hl.r16, buf); sink = write(2, buf, n);
    sink = write(2, "\nH: ", 4);
    n = utoa_hex(hl.r8.h, buf); sink = write(2, buf, n);
    sink = write(2, "\nL: ", 4);
    n = utoa_hex(hl.r8.l, buf); sink = write(2, buf, n);
    sink = write(2, "\n", 1);

    // SP
    sink = write(2, "SP: ", 4);
    n = utoa_hex(sp.r16, buf); sink = write(2, buf, n);
    sink = write(2, "\n", 1);

    // PC
    sink = write(2, "PC: ", 4);
    n = utoa_hex(pc.r16, buf); sink = write(2, buf, n);
    sink = write(2, "\n", 1);

    // Dots
    sink = write(2, "Dots: ", 6);
    n = ultoa_dec(dots, buf); sink = write(2, buf, n);
    sink = write(2, "\n", 1);

    // IME
    sink = write(2, "IME: ", 5);
    n = utoa_dec(ime, buf); sink = write(2, buf, n);
    sink = write(2, "\n\n", 2);
    sink = sink;
}