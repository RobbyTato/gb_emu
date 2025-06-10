#ifndef DEBUG_H
#define DEBUG_H

#include <stdint.h>

#define DEBUG_TILE_MAP_PIXEL_SIZE 2
#define DEBUG_TILE_DATA_PIXEL_SIZE 2

void init_debug(void);
void free_debug(void);
void update_debug(void);

void get_inst_name(char *buffer, uint8_t inst);
void get_prefix_inst_name(char *buffer, uint8_t inst);

#endif // DEBUG_H