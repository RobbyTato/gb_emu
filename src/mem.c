#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <rom.h>
#include <mem.h>
#include <util.h>
#include <cpu.h>

// Memory
tile vram_tiles[VRAM_TILES_NUM];
map vram_maps[VRAM_MAP_NUM];
obj oam[OBJ_NUM];
uint8_t wram[WRAM_SIZE];
uint8_t io_reg[IO_REG_SIZE];
uint8_t hram[HRAM_SIZE];

// Serial Data Transfer registers
uint8_t r_sb = 0;
uint8_t r_sc = 0;

// Timer registers
uint8_t r_div = 0;
uint8_t r_tima = 0;
uint8_t r_tma = 0;
uint8_t r_tac = 0;

// Interrupt registers
uint8_t r_ie = 0;
uint8_t r_if = 0;

/* Audio registers */

// Global sound registers
uint8_t r_nr50 = 0; // Master volume & VIN panning
uint8_t r_nr51 = 0; // Sound panning
uint8_t r_nr52 = 0; // Audio master control

// Channel 1 registers (Square)
uint8_t r_nr10 = 0; // Sweep
uint8_t r_nr11 = 0; // Length timer & duty cycle
uint8_t r_nr12 = 0; // Volume & envelope
uint8_t r_nr13 = 0; // Period low
uint8_t r_nr14 = 0; // Period high & control

// Channel 2 registers (Square)
uint8_t r_nr21 = 0; // Length timer & duty cycle
uint8_t r_nr22 = 0; // Volume & envelope
uint8_t r_nr23 = 0; // Period low
uint8_t r_nr24 = 0; // Period high & control

// Channel 3 registers (Wave)
uint8_t r_nr30 = 0; // DAC
uint8_t r_nr31 = 0; // Length timer
uint8_t r_nr32 = 0; // Output level
uint8_t r_nr33 = 0; // Period low
uint8_t r_nr34 = 0; // Period high & control
uint8_t m_wave[16] = {0}; // Wave pattern

// Channel 4 registers (Noise)
uint8_t r_nr41 = 0; // Length timer
uint8_t r_nr42 = 0; // Volume & envelope
uint8_t r_nr43 = 0; // Frequency & randomness
uint8_t r_nr44 = 0; // Control

// LCD control registers
uint8_t r_lcdc = 0;
uint8_t r_ly = 0;
uint8_t r_lyc = 0;
uint8_t r_stat = 0;
uint8_t r_scy = 0;
uint8_t r_scx = 0;
uint8_t r_wy = 0;
uint8_t r_wx = 0; // x pos + 7; i.e. wx = 7 and wy = 0 is flush

// Palette registers
uint8_t r_bgp = 0;
uint8_t r_obp0 = 0;
uint8_t r_obp1 = 0;

// OAM DMA register
uint8_t r_dma = 0;

// Boot ROM mapped register
uint8_t r_boot_rom_mapped = 0;

// Button Inputs
bool b_buttons_select = false;
bool b_dpad_select = false;
bool b_left = false;
bool b_right = false;
bool b_up = false;
bool b_down = false;
bool b_a = false;
bool b_b = false;
bool b_start = false;
bool b_select = false;

int bin_search(const uint16_t arr[], int size, const uint16_t target) {
    int left = 0;
    int right = size - 1;
    while (left <= right) {
        int mid = left + (right - left) / 2; // Avoid overflow
        if (arr[mid] == target) {
            return mid; // Target found
        } else if (arr[mid] < target) {
            left = mid + 1; // Move to the right half
        } else {
            right = mid - 1; // Move to the left half
        }
    }
    // If not found, bias downwards
    return (right >= 0) ? right : -1;
}

uint8_t read_mem(uint16_t addr) {
    const uint16_t mem_ranges[] = {
        ROM_A, VRAM_TILES_A, VRAM_MAPS_A, EXT_RAM_A, WRAM_A, 
        ECHO_RAM_A, OAM_A, UNUSED_A, IO_A, HRAM_A, IE_REG_A
    };

    if (!r_boot_rom_mapped && addr < 0x100) {
        return dmg_boot_rom[addr];
    }

    switch (bin_search(mem_ranges, 11, addr)) {
        case 0:
            return rom[addr];
        case 1:
            return ((uint8_t *)vram_tiles)[addr - VRAM_TILES_A];
        case 2:
            return ((uint8_t *)vram_maps)[addr - VRAM_MAPS_A];
        case 3:
            fprintf(stderr, "Attempted read at addr %d\n", (int)addr);
            fprintf(stderr, "External RAM unimplemented, exiting...\n");
            exit(1);
        case 4:
            return wram[addr - WRAM_A];
        case 5:
            fprintf(stderr, "Attempted read at addr %d\n", (int)addr);
            fprintf(stderr, "Echo RAM unimplemented, exiting...\n");
            exit(1);
        case 6:
            return ((uint8_t *)oam)[addr - OAM_A];
        case 7:
            fprintf(stderr, "Attempted read at addr %d\n", (int)addr);
            fprintf(stderr, "Unused/Invalid RAM location, exiting...\n");
            exit(1);
        case 8:
            break; // handled after this switch
        case 9:
            return hram[addr - HRAM_A];
        case 10:
            return r_ie;
        default:
            fprintf(stderr, "Attempted read at addr %d\n", (int)addr);
            fprintf(stderr, "Invalid switch case, exiting...\n");
            exit(1);
    }

    // Handle I/O registers
    switch (addr) {
        case 0xFF00: 
            switch ((b_buttons_select << 1) | b_dpad_select) {
                case 0x3:
                    return ((((((!(b_start || b_down) << 1) |
                            !(b_select || b_up)) << 1)  |
                            !(b_b || b_left)) << 1)     |
                            !(b_a || b_right)) |
                            0xC0;
                case 0x2:
                    return ((((((!b_start << 1) |
                            !b_select) << 1) |
                            !b_b) << 1) |
                            !b_a) |
                            0xD0;
                case 0x1:
                    return ((((((!b_down << 1) |
                            !b_up) << 1) |
                            !b_left) << 1) |
                            !b_right) |
                            0xE0;
                default:
                    return 0xFF;
        }
        case 0xFF01:
            return r_sb;
        case 0xFF02:
            return r_sc;
        case 0xFF04:
            return r_div;
        case 0xFF05:
            return r_tima;
        case 0xFF06:
            return r_tma;
        case 0xFF07:
            return r_tac;
        case 0xFF0F:
            return r_if;
        case 0xFF10:
            return r_nr10;
        case 0xFF11:
            return r_nr11;
        case 0xFF12:
            return r_nr12;
        case 0xFF13:
            return r_nr13;
        case 0xFF14:
            return r_nr14;
        case 0xFF16:
            return r_nr21;
        case 0xFF17:
            return r_nr22;
        case 0xFF18:
            return r_nr23;
        case 0xFF19:
            return r_nr24;
        case 0xFF1A:
            return r_nr30;
        case 0xFF1B:
            return r_nr31;
        case 0xFF1C:
            return r_nr32;
        case 0xFF1D:
            return r_nr33;
        case 0xFF1E:
            return r_nr34;
        case 0xFF20:
            return r_nr41;
        case 0xFF21:
            return r_nr42;
        case 0xFF22:
            return r_nr43;
        case 0xFF23:
            return r_nr44;
        case 0xFF24:
            return r_nr50;
        case 0xFF25:
            return r_nr51;
        case 0xFF26:
            return r_nr52;
        case 0xFF40:
            return r_lcdc;
        case 0xFF41:
            return r_stat;
        case 0xFF42:
            return r_scy;
        case 0xFF43:
            return r_scx;
        case 0xFF44:
            return r_ly;
        case 0xFF45:
            return r_lyc;
        case 0xFF46:
            return r_dma;
        case 0xFF47:
            return r_bgp;
        case 0xFF48:
            return r_obp0;
        case 0xFF49:
            return r_obp1;
        case 0xFF4A:
            return r_wy;
        case 0xFF4B:
            return r_wx;
        case 0xFF50:
            return r_boot_rom_mapped;
    }

    // Wave pattern
    if (0xFF30 <= addr && addr < 0xFF40) {
        return m_wave[addr - 0xFF30];
    }

    // Leftover memory in IO register range
    if (0xFF00 <= addr && addr < 0xFF80) {
        return io_reg[addr - IO_A];
    }

    fprintf(stderr, "Attempted read at addr %d\n", (int)addr);
    fprintf(stderr, "Unused/Invalid RAM location, exiting...\n");
    exit(1);
}

void write_mem(uint16_t addr, uint8_t val) {
    const uint16_t mem_ranges[] = {
        ROM_A, VRAM_TILES_A, VRAM_MAPS_A, EXT_RAM_A, WRAM_A, 
        ECHO_RAM_A, OAM_A, UNUSED_A, IO_A, HRAM_A, IE_REG_A
    };

    switch (bin_search(mem_ranges, 11, addr)) {
        case 0:
            // fprintf(stderr, "Attempted write at addr %d\n", (int)addr);
            // fprintf(stderr, "Cannot write to ROM, exiting...\n");
            // exit(1);
            // TODO: Handle the case of the tetris rom writing to this range
            return;
        case 1:
            ((uint8_t *)vram_tiles)[addr - VRAM_TILES_A] = val;
            return;
        case 2:
            ((uint8_t *)vram_maps)[addr - VRAM_MAPS_A] = val;
            return;
        case 3:
            fprintf(stderr, "Attempted write at addr %d\n", (int)addr);
            fprintf(stderr, "External RAM unimplemented, exiting...\n");
            exit(1);
        case 4:
            wram[addr - WRAM_A] = val;
            return;
        case 5:
            fprintf(stderr, "Attempted write at addr %d\n", (int)addr);
            fprintf(stderr, "Echo RAM unimplemented, exiting...\n");
            exit(1);
        case 6:
            ((uint8_t *)oam)[addr - OAM_A] = val;
            return;
        case 7:
            // fprintf(stderr, "Attempted write at addr %d\n", (int)addr);
            // fprintf(stderr, "Unused/Invalid RAM location, exiting...\n");
            // exit(1);
            // TODO: Handle the case of the tetris rom writing to this range
            return;
        case 8:
            break; // handled after this switch
        case 9:
            hram[addr - HRAM_A] = val;
            return;
        case 10:
            r_ie = val;
            return;
        default:
            fprintf(stderr, "Attempted write at addr %d\n", (int)addr);
            fprintf(stderr, "Invalid switch case, exiting...\n");
            exit(1);
    }

    // Handle I/O registers
    switch (addr) {
        case 0xFF00:
            b_buttons_select = !!(val & 0x20);
            b_dpad_select = !!(val & 0x10);
            return;
        case 0xFF01:
            r_sb = val;
            return;
        case 0xFF02:
            // TODO: Recheck this logic
            switch (val & 0x81) {
                case 0x80:
                    fprintf(stderr, 
                            "Attempted serial connection as slave at addr %d\n",
                            (int)addr);
                    fprintf(stderr, "Not yet implemented, exiting...\n");
                    exit(1);
                case 0x81:
                    // Hack to make Blargg's test roms work
                    printf("%c", r_sb);
                    break;
                default:
                    r_sc = val;
                    return;
            }
            r_sc = 0;
            r_if |= 0x8; // request serial interrupt
            return;
        case 0xFF04:
            r_div = val;
            return;
        case 0xFF05:
            r_tima = val;
            return;
        case 0xFF06:
            r_tma = val;
            return;
        case 0xFF07:
            r_tac = val;
            return;        
        case 0xFF0F:
            r_if = val;
            return;
        case 0xFF10:
            r_nr10 = val;
            return;
        case 0xFF11:
            r_nr11 = val;
            return;
        case 0xFF12:
            r_nr12 = val;
            return;
        case 0xFF13:
            r_nr13 = val;
            return;
        case 0xFF14:
            r_nr14 = val;
            return;
        case 0xFF16:
            r_nr21 = val;
            return;
        case 0xFF17:
            r_nr22 = val;
            return;
        case 0xFF18:
            r_nr23 = val;
            return;
        case 0xFF19:
            r_nr24 = val;
            return;
        case 0xFF1A:
            r_nr30 = val;
            return;
        case 0xFF1B:
            r_nr31 = val;
            return;
        case 0xFF1C:
            r_nr32 = val;
            return;
        case 0xFF1D:
            r_nr33 = val;
            return;
        case 0xFF1E:
            r_nr34 = val;
            return;
        case 0xFF20:
            r_nr41 = val;
            return;
        case 0xFF21:
            r_nr42 = val;
            return;
        case 0xFF22:
            r_nr43 = val;
            return;
        case 0xFF23:
            r_nr44 = val;
            return;
        case 0xFF24:
            r_nr50 = val;
            return;
        case 0xFF25:
            r_nr51 = val;
            return;
        case 0xFF26:
            r_nr52 = val;
            return;
        case 0xFF40:
            r_lcdc = val;
            return;
        case 0xFF41:
            r_stat = val;
            return;
        case 0xFF42:
            r_scy = val;
            return;
        case 0xFF43:
            r_scx = val;
            return;
        case 0xFF44:
            r_ly = val;
            return;
        case 0xFF45:
            r_lyc = val;
            return;
        case 0xFF46:
            r_dma = val;
            return;
        case 0xFF47:
            r_bgp = val;
            return;
        case 0xFF48:
            r_obp0 = val;
            return;
        case 0xFF49:
            r_obp1 = val;
            return;
        case 0xFF4A:
            r_wy = val;
            return;
        case 0xFF4B:
            r_wx = val;
            return;
        case 0xFF50:
            r_boot_rom_mapped = val;
            return;
    }

    // Wave pattern
    if (0xFF30 <= addr && addr < 0xFF40) {
        m_wave[addr - 0xFF30] = val;
        return;
    }

    // Leftover memory in IO register range
    if (0xFF00 <= addr && addr < 0xFF80) {
        io_reg[addr - IO_A] = val;
        return;
    }

    fprintf(stderr, "Attempted write at addr %d\n", (int)addr);
    fprintf(stderr, "Unused/Invalid RAM location, exiting...\n");
    exit(1);
}

