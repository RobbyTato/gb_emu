#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include <stdbool.h>

#define TILE_SIZE 16
#define MAP_SIZE 1024
#define OBJ_SIZE 4

#define VRAM_TILES_NUM 384
#define VRAM_MAP_NUM 2
#define OBJ_NUM 40
#define WRAM_SIZE 0x2000
#define HRAM_SIZE 0x7F

typedef uint8_t tile[TILE_SIZE];
typedef uint8_t map[MAP_SIZE];
typedef uint8_t obj[OBJ_SIZE];

// Memory Ranges
#define ROM_A 0x0000
#define VRAM_TILES_A 0x8000
#define VRAM_MAPS_A 0x9800
#define EXT_RAM_A 0xA000
#define WRAM_A 0xC000
#define ECHO_RAM_A 0xE000
#define OAM_A 0xFE00
#define UNUSED_A 0xFEA0
#define IO_A 0xFF00
#define HRAM_A 0xFF80
#define IE_REG_A 0xFFFF

// LCDC bitmasks
#define LCDC_LCD_PPU_ENABLED 0x80
#define LCDC_WINDOW_TILE_MAP_AREA 0x40
#define LCDC_WINDOW_ENABLED 0x20
#define LCDC_BG_WIN_TILE_DATA_AREA 0x10
#define LCDC_BG_TILE_MAP_AREA 0x08
#define LCDC_OBJ_SIZE 0x04
#define LCDC_OBJ_ENABLED 0x02
#define LCDC_BG_WIN_ENABLED 0x01

// Memory components
extern tile vram_tiles[VRAM_TILES_NUM];
extern map vram_maps[VRAM_MAP_NUM];
extern obj oam[OBJ_NUM];
extern uint8_t wram[WRAM_SIZE];
extern uint8_t hram[HRAM_SIZE];

// Registers
extern uint8_t r_joypad;
extern uint8_t r_div, r_tima, r_tma, r_tac;
extern uint8_t r_ie, r_if;
extern uint8_t r_nr50, r_nr51, r_nr52;
extern uint8_t r_nr10, r_nr11, r_nr12, r_nr13, r_nr14;
extern uint8_t r_nr21, r_nr22, r_nr23, r_nr24;
extern uint8_t r_nr30, r_nr31, r_nr32, r_nr33, r_nr34;
extern uint8_t r_nr41, r_nr42, r_nr43, r_nr44;
extern uint8_t r_lcdc, r_ly, r_lyc, r_stat;
extern uint8_t r_scy, r_scx, r_wy, r_wx;
extern uint8_t r_bgp, r_obp0, r_obp1;
extern uint8_t r_boot_rom_mapped;
extern uint8_t m_wave[16];

// Function prototypes
uint8_t read_mem(uint16_t addr);
void write_mem(uint16_t addr, uint8_t val);

#endif // MEMORY_H
