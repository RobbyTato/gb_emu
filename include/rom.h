#ifndef ROM_H
#define ROM_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define DMG_BOOT_ROM_SIZE 256

extern bool rom_loaded;

extern uint8_t cgb_flag;
extern uint8_t sgb_flag;
extern uint8_t rom_type;
extern size_t rom_size;
extern size_t ram_size;
extern uint8_t *rom;

extern uint8_t dmg_boot_rom[DMG_BOOT_ROM_SIZE];

void load_rom_error(void);
void load_rom(const char *rom_path);
void free_rom(void);

#endif // ROM_H