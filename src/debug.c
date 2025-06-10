#include <debug.h>
#include <mem.h>
#include <cpu.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdlib.h>
#include <stdio.h>

SDL_Window *tile_map1_window = NULL;
SDL_Window *tile_map2_window = NULL;
SDL_Window *tile_data_window = NULL;
SDL_Renderer *tile_map1_renderer = NULL;
SDL_Renderer *tile_map2_renderer = NULL;
SDL_Renderer *tile_data_renderer = NULL;
SDL_Texture *tile_map1_texture = NULL;
SDL_Texture *tile_map2_texture = NULL;
SDL_Texture *tile_data_texture = NULL;
Uint32 *tile_map1_fb = NULL;
Uint32 *tile_map2_fb = NULL;
Uint32 *tile_data_fb = NULL;
static const Uint32 palette[4] = 
    {0xFFFFFFFF, 0xFFAAAAAA, 0xFF555555, 0xFF000000};

void init_debug(void) {
    // Initialize windows
    tile_map1_window = SDL_CreateWindow(
        "Tile Map 1",
        256 * DEBUG_TILE_MAP_PIXEL_SIZE,
        256 * DEBUG_TILE_MAP_PIXEL_SIZE,
        0
    );
    if (!tile_map1_window) {
        fprintf(stderr, "Tile Map 1 window creation failed: %s\n", 
            SDL_GetError());
        exit(1);
    }
    tile_map2_window = SDL_CreateWindow(
        "Tile Map 2",
        256 * DEBUG_TILE_MAP_PIXEL_SIZE,
        256 * DEBUG_TILE_MAP_PIXEL_SIZE,
        0
    );
    if (!tile_map2_window) {
        fprintf(stderr, "Tile Map 2 window creation failed: %s\n", 
            SDL_GetError());
        exit(1);
    }
    tile_data_window = SDL_CreateWindow(
        "Tile Data",
        128 * DEBUG_TILE_DATA_PIXEL_SIZE,
        192 * DEBUG_TILE_DATA_PIXEL_SIZE,
        0
    );
    if (!tile_data_window) {
        fprintf(stderr, "Tile Data window creation failed: %s\n", 
            SDL_GetError());
        exit(1);
    }

    int start_x = 200, start_y = 200, spacing = 600;
    if (!SDL_SetWindowPosition(tile_map1_window, start_x, start_y)) {
        fprintf(stderr, "Tile Map 1 window reposition failed: %s\n", 
            SDL_GetError());
        exit(1);
    }
    SDL_Delay(50);
    if (!SDL_SetWindowPosition(tile_map2_window, start_x, start_y + spacing)) {
        fprintf(stderr, "Tile Map 2 window reposition failed: %s\n", 
            SDL_GetError());
        exit(1);
    }
    SDL_Delay(50);
    if (!SDL_SetWindowPosition(tile_data_window, start_x + 3 * spacing, 
        start_y)) {
        fprintf(stderr, "Tile Data window reposition failed: %s\n", 
            SDL_GetError());
        exit(1);
    }

    // Initialize renderers for hardware acceleration
    tile_map1_renderer = SDL_CreateRenderer(tile_map1_window, NULL);
    tile_map2_renderer = SDL_CreateRenderer(tile_map2_window, NULL);
    tile_data_renderer = SDL_CreateRenderer(tile_data_window, NULL);

    if (!tile_map1_renderer || !tile_map2_renderer || !tile_data_renderer) {
        fprintf(stderr, "Renderer creation failed: %s\n", SDL_GetError());
        exit(1);
    }

    // Create textures for each renderer
    tile_map1_texture = SDL_CreateTexture(
        tile_map1_renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        256, 256  // 32x32 tiles * 8x8 pixels
    );
    tile_map2_texture = SDL_CreateTexture(
        tile_map2_renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        256, 256
    );
    tile_data_texture = SDL_CreateTexture(
        tile_data_renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        128, 192  // 16x24 tiles * 8x8 pixels
    );
    if (!tile_map1_texture || !tile_map2_texture || !tile_data_texture) {
        fprintf(stderr, "Texture creation failed: %s\n", SDL_GetError());
        exit(1);
    }

    // Set pixel art mode for each texture
    if (!SDL_SetTextureScaleMode(tile_map1_texture, SDL_SCALEMODE_PIXELART)) {
        fprintf(stderr, "Failed to turn on pixel art mode for Tile Map 1: %s\n",
            SDL_GetError());
        exit(1);
    }
    if (!SDL_SetTextureScaleMode(tile_map2_texture, SDL_SCALEMODE_PIXELART)) {
        fprintf(stderr, "Failed to turn on pixel art mode for Tile Map 2: %s\n",
            SDL_GetError());
        exit(1);
    }
    if (!SDL_SetTextureScaleMode(tile_data_texture, SDL_SCALEMODE_PIXELART)) {
        fprintf(stderr, "Failed to turn on pixel art mode for Tile Data: %s\n",
            SDL_GetError());
        exit(1);
    }

    // Unlock textures to directly access the textures' buffers
    void *pixels;
    int pitch;
    
    if (!SDL_LockTexture(tile_map1_texture, NULL, &pixels, &pitch)) {
        fprintf(stderr, "Failed to lock Tile Map 1 texture: %s\n", 
            SDL_GetError());
        exit(1);
    }
    tile_map1_fb = pixels;
    memset(tile_map1_fb, 0, 256 * 256 * sizeof(Uint32));

    if (!SDL_LockTexture(tile_map2_texture, NULL, &pixels, &pitch)) {
        fprintf(stderr, "Failed to lock Tile Map 2 texture: %s\n", 
            SDL_GetError());
        exit(1);
    }
    tile_map2_fb = pixels;
    memset(tile_map2_fb, 0, 256 * 256 * sizeof(Uint32));

    if (!SDL_LockTexture(tile_data_texture, NULL, &pixels, &pitch)) {
        fprintf(stderr, "Failed to lock Tile Data texture: %s\n", 
            SDL_GetError());
        exit(1);
    }
    tile_data_fb = pixels;
    memset(tile_data_fb, 0, 128 * 192 * sizeof(Uint32));
}

void free_debug(void) {
    // Clear frame buffer pointers
    tile_map1_fb = NULL;
    tile_map2_fb = NULL;
    tile_data_fb = NULL;

    // Clean debug textures
    if (tile_map1_texture) {
        SDL_DestroyTexture(tile_map1_texture);
        tile_map1_texture = NULL;
    }
    if (tile_map2_texture) {
        SDL_DestroyTexture(tile_map2_texture);
        tile_map2_texture = NULL;
    }
    if (tile_data_texture) {
        SDL_DestroyTexture(tile_data_texture);
        tile_data_texture = NULL;
    }

    // Clean debug renderers
    if (tile_map1_renderer) {
        SDL_DestroyRenderer(tile_map1_renderer);
        tile_map1_renderer = NULL;
    }
    if (tile_map2_renderer) {
        SDL_DestroyRenderer(tile_map2_renderer);
        tile_map2_renderer = NULL;
    }
    if (tile_data_renderer) {
        SDL_DestroyRenderer(tile_data_renderer);
        tile_data_renderer = NULL;
    }

    // Clean debug windows
    if (tile_map1_window) {
        SDL_DestroyWindow(tile_map1_window);
        tile_map1_window = NULL;
    }
    if (tile_map2_window) {
        SDL_DestroyWindow(tile_map2_window);
        tile_map2_window = NULL;
    }
    if (tile_data_window) {
        SDL_DestroyWindow(tile_data_window);
        tile_data_window = NULL;
    }
}

void draw_tile(Uint32 *fb, size_t idx, size_t x, size_t y, size_t width) {
    for (size_t yy = 0; yy < 8; yy++) {
        for (size_t xx = 0; xx < 8; xx++) {
            size_t idx_offset = yy * 2;
            size_t shift_offset = 7 - xx;
            uint8_t lsb = (vram_tiles[idx][idx_offset] >> shift_offset) & 1;
            uint8_t msb = (vram_tiles[idx][idx_offset + 1] >> shift_offset) & 1;
            uint8_t pixel_color_index = (msb << 1) | lsb;
            // TODO: Maybe put an option to remove palette dependence on
            // TODO: register BGP for debug purposes
            Uint32 color = palette[(r_bgp >> (pixel_color_index * 2)) & 0x3];
            fb[(8 * x) + xx + ((width * 8) * ((8 * y) + yy))] = color;
        }
    }
}

void draw_tile_map1(void) {
    for (size_t y = 0; y < 32; y++) {
        for (size_t x = 0; x < 32; x++) {
            size_t tile_idx = vram_maps[0][x + (32 * y)];
            draw_tile(tile_map1_fb, tile_idx, x, y, 32);
        }
    }
}

void draw_tile_map2(void) {
    for (size_t y = 0; y < 32; y++) {
        for (size_t x = 0; x < 32; x++) {
            size_t tile_idx = vram_maps[1][x + (32 * y)];
            draw_tile(tile_map2_fb, tile_idx, x, y, 32);
        }
    }
}

void draw_tile_data(void) {
    for (size_t y = 0; y < 24; y++) {
        for (size_t x = 0; x < 16; x++) {
            draw_tile(tile_data_fb, x + (16 * y), x, y, 16);
        }
    }
}

void update_debug(void) {
    draw_tile_map1();
    draw_tile_map2();
    draw_tile_data();
    SDL_UnlockTexture(tile_map1_texture);
    SDL_UnlockTexture(tile_map2_texture);
    SDL_UnlockTexture(tile_data_texture);
    SDL_RenderTexture(tile_map1_renderer, tile_map1_texture, NULL, NULL);
    SDL_RenderTexture(tile_map2_renderer, tile_map2_texture, NULL, NULL);
    SDL_RenderTexture(tile_data_renderer, tile_data_texture, NULL, NULL);
    SDL_RenderPresent(tile_map1_renderer);
    SDL_RenderPresent(tile_map2_renderer);
    SDL_RenderPresent(tile_data_renderer);
    void *pixels;
    int pitch;
    
    if (!SDL_LockTexture(tile_map1_texture, NULL, &pixels, &pitch)) {
        fprintf(stderr, "Failed to lock Tile Map 1 texture: %s\n", 
            SDL_GetError());
        exit(1);
    }
    tile_map1_fb = pixels;
    memset(tile_map1_fb, 0, 256 * 256 * sizeof(Uint32));

    if (!SDL_LockTexture(tile_map2_texture, NULL, &pixels, &pitch)) {
        fprintf(stderr, "Failed to lock Tile Map 2 texture: %s\n", 
            SDL_GetError());
        exit(1);
    }
    tile_map2_fb = pixels;
    memset(tile_map2_fb, 0, 256 * 256 * sizeof(Uint32));

    if (!SDL_LockTexture(tile_data_texture, NULL, &pixels, &pitch)) {
        fprintf(stderr, "Failed to lock Tile Data texture: %s\n", 
            SDL_GetError());
        exit(1);
    }
    tile_data_fb = pixels;
    memset(tile_data_fb, 0, 128 * 192 * sizeof(Uint32));
}

char *get_r8(uint8_t idx) {
    switch (idx) {
        case 0:
            return "b";
        case 1:
            return "c";
        case 2:
            return "d";
        case 3:
            return "e";
        case 4:
            return "h";
        case 5:
            return "l";
        case 6:
            return "[hl]";
        case 7:
            return "a";
    }
    fprintf(stderr, "Invalid read to r8 index %d, exiting...\n", idx);
    exit(1);
}

char *get_r16(uint8_t idx) {
    switch (idx) {
        case 0:
            return "bc";
        case 1:
            return "de";
        case 2:
            return "hl";
        case 3:
            return "sp";
    }
    fprintf(stderr, "Invalid read to r16 index %d, exiting...\n", idx);
    exit(1);
}

char *get_r16stk(uint8_t idx) {
    switch (idx) {
        case 0:
            return "bc";
        case 1:
            return "de";
        case 2:
            return "hl";
        case 3:
            return "af";
    }
    fprintf(stderr, "Invalid read to r16stk index %d, exiting...\n", idx);
    exit(1);
}

char *get_r16mem(uint8_t idx) {
    switch (idx) {
        case 0:
            return "[bc]";
        case 1:
            return "[de]";
        case 2:
            return "[hl+]";
        case 3:
            return "[hl-]";
    }
    fprintf(stderr, "Invalid read to r16mem index %d, exiting...\n", idx);
    exit(1);
}

char *get_cond(uint8_t idx) {
    switch (idx) {
        case 0: // nz
            return "nz";
        case 1: // z
            return "z";
        case 2: // nc
            return "nc";
        case 3: // c
            return "c";
    }
    fprintf(stderr, "Invalid condition index %d, exiting...\n", idx);
    exit(1);
}

void get_inst_name(char *buffer, uint8_t inst) {
    switch (inst & 0xC0) {
    
    case 0x0:
        switch (inst & 0xF) {
            case 0x1: { // ld r16, imm16
                uint8_t idx = (inst & 0x30) >> 4;
                sprintf(buffer, "ld %s, imm16", get_r16(idx));
                return;
            }
            case 0x2: { // ld [r16mem], a
                uint8_t idx = (inst & 0x30) >> 4;
                sprintf(buffer, "ld %s, a", get_r16mem(idx));
                return;
            }
            case 0xA: { // ld a, [r16mem]
                uint8_t idx = (inst & 0x30) >> 4;
                sprintf(buffer, "ld a, %s", get_r16mem(idx));
                return;
            }
            case 0x3: { // inc r16
                uint8_t idx = (inst & 0x30) >> 4;
                sprintf(buffer, "inc %s", get_r16(idx));
                return;
            }
            case 0xB: { // dec r16
                uint8_t idx = (inst & 0x30) >> 4;
                sprintf(buffer, "dec %s", get_r16(idx));
                return;
            }
            case 0x9: { // add hl, r16
                uint8_t idx = (inst & 0x30) >> 4;
                sprintf(buffer, "add hl, %s", get_r16(idx));
                return;
            }
        }
        switch (inst & 0x7) {
            case 0x4: { // inc r8
                uint8_t idx = (inst & 0x38) >> 3;
                sprintf(buffer, "inc %s", get_r8(idx));
                return;
            }
            case 0x5: { // dec r8
                uint8_t idx = (inst & 0x38) >> 3;
                sprintf(buffer, "dec %s", get_r8(idx));
                return;
            }
            case 0x6: { // ld r8, imm8
                uint8_t idx = (inst & 0x38) >> 3;
                sprintf(buffer, "ld %s, imm8", get_r8(idx));
                return;
            }
        }
        if ((inst & 0x27) == 0x20) { // jr cond, imm8
            uint8_t idx = (inst & 0x18) >> 3;
            sprintf(buffer, "jr %s, imm8", get_cond(idx));
            return;
        }
        switch (inst) {
            case 0x0: // nop
                sprintf(buffer, "nop");
                return;
            case 0x8: // ld [imm16], sp
                sprintf(buffer, "ld [imm16], sp");
                return;
            case 0x7: { // rlca
                sprintf(buffer, "rlca");
                return;
            }
            case 0xF: { // rrca
                sprintf(buffer, "rrca");
                return;
            }
            case 0x17: { // rla
                sprintf(buffer, "rla");
                return;
            }
            case 0x1F: { // rra
                sprintf(buffer, "rra");
                return;
            }
            case 0x27: { // daa
                sprintf(buffer, "daa");
                return;
            }
            case 0x2F: // cpl
                sprintf(buffer, "cpl");
                return;
            case 0x37: // scf
                sprintf(buffer, "scf");
                return;
            case 0x3F: // ccf
                sprintf(buffer, "ccf");
                return;
            case 0x18: { // jr imm8
                sprintf(buffer, "jr imm8");
                return;
            }
            case 0x10: { // stop
                sprintf(buffer, "stop");
                return;
            }
        }
        break;
    
    case 0x40: {
        if (inst == 0x76) { // halt
            sprintf(buffer, "halt");
            return;
        }
        // ld r8, r8
        uint8_t dst = (inst >> 3) & 0x7;
        uint8_t src = inst & 0x7;
        sprintf(buffer, "ld %s, %s", get_r8(dst), get_r8(src));
        return;
    }

    case 0x80: {
        switch (inst & 0x38) {
            case 0x0: { // add a, r8
                uint8_t idx = inst & 0x7;
                sprintf(buffer, "add a, %s", get_r8(idx));
                return;
            }
            case 0x8: { // adc a, r8
                uint8_t idx = inst & 0x7;
                sprintf(buffer, "adc a, %s", get_r8(idx));
                return;
            }
            case 0x10: { // sub a, r8
                uint8_t idx = inst & 0x7;
                sprintf(buffer, "sub a, %s", get_r8(idx));
                return;
            }
            case 0x18: { // sbc a, r8
                uint8_t idx = inst & 0x7;
                sprintf(buffer, "sbc a, %s", get_r8(idx));
                return;
            }
            case 0x20: { // and a, r8
                uint8_t idx = inst & 0x7;
                sprintf(buffer, "and a, %s", get_r8(idx));
                return;
            }
            case 0x28: { // xor a, r8
                uint8_t idx = inst & 0x7;
                sprintf(buffer, "xor a, %s", get_r8(idx));
                return;
            }
            case 0x30: { // or a, r8
                uint8_t idx = inst & 0x7;
                sprintf(buffer, "or a, %s", get_r8(idx));
                return;
            }
            case 0x38: { // cp a, r8
                uint8_t idx = inst & 0x7;
                sprintf(buffer, "cp a, %s", get_r8(idx));
                return;
            }
        }
        break;
    }

    case 0xC0: {
        switch (inst & 0x27) {
            case 0x0: { // ret cond
                uint8_t idx = (inst & 0x18) >> 3;
                sprintf(buffer, "ret %s", get_cond(idx));
                return;
            }
            case 0x2: { // jp cond, imm16
                uint8_t idx = (inst & 0x18) >> 3;
                sprintf(buffer, "jp %s, imm16", get_cond(idx));
                return;
            }
            case 0x4: { // call cond, imm16
                uint8_t idx = (inst & 0x18) >> 3;
                sprintf(buffer, "call %s, imm16", get_cond(idx));
                return;
            }
        }
        if ((inst & 0x7) == 0x7) { // rst tgt3
            sprintf(buffer, "rst $%x", inst & 0x38);
            return;
        }
        switch (inst & 0xF) {
            case 0x1: { // pop r16stk
                uint8_t idx = (inst & 0x30) >> 4;
                sprintf(buffer, "pop %s", get_r16stk(idx));
                return;
            }
            case 0x5: { // push r16stk
                uint8_t idx = (inst & 0x30) >> 4;
                sprintf(buffer, "push %s", get_r16stk(idx));
                return;
            }
        }
        switch (inst) {
            case 0xC6: { // add a, imm8
                sprintf(buffer, "add a, imm8");
                return;
            }
            case 0xCE: { // adc a, imm8
                sprintf(buffer, "adc a, imm8");
                return;
            }
            case 0xD6: { // sub a, imm8
                sprintf(buffer, "sub a, imm8");
                return;
            }
            case 0xDE: { // sbc a, imm8
                sprintf(buffer, "sbc a, imm8");
                return;
            }
            case 0xE6: { // and a, imm8
                sprintf(buffer, "and a, imm8");
                return;
            }
            case 0xEE: { // xor a, imm8
                sprintf(buffer, "xor a, imm8");
                return;
            }
            case 0xF6: { // or a, imm8
                sprintf(buffer, "or a, imm8");
                return;
            }
            case 0xFE: { // cp a, imm8
                sprintf(buffer, "cp a, imm8");
                return;
            }
            case 0xC9: // ret
                sprintf(buffer, "ret");
                return;
            case 0xD9: // reti
                sprintf(buffer, "reti");
                return;
            case 0xC3: // jp imm16
                sprintf(buffer, "jp imm8");
                return;
            case 0xE9: // jp hl
                sprintf(buffer, "jp hl");
                return;
            case 0xCD: { // call imm16
                sprintf(buffer, "call imm16");
                return;
            }
            case 0xE2: // ldh [c], a
                sprintf(buffer, "ldh [c], a");
                return;
            case 0xE0: // ldh [imm8], a
                sprintf(buffer, "ldh [imm8], a");
                return;
            case 0xEA: // ld [imm16], a
                sprintf(buffer, "ld [imm16], a");
                return;
            case 0xF2: // ldh a, [c]
                sprintf(buffer, "ldh a, [c]");
                return;
            case 0xF0: // ldh a, [imm8]
                sprintf(buffer, "ldh a, [imm8]");
                return;
            case 0xFA: // ld a, [imm16]
                sprintf(buffer, "ld a, [imm16]");
                return;
            case 0xE8: { // add sp, imm8
                sprintf(buffer, "add sp, imm8");
                return;
            }
            case 0xF8: { // ld hl, sp + imm8
                sprintf(buffer, "ld hl, sp + imm8");
                return;
            }
            case 0xF9: // ld sp, hl
                sprintf(buffer, "ld sp, hl");
                return;
            case 0xF3: // di
                sprintf(buffer, "di");
                return;
            case 0xFB: // ei
                sprintf(buffer, "ei");
                return;
            case 0xCB: { // prefix
                sprintf(buffer, "prefix");
                return;
            }
        }
        break;
    }
    }

    sprintf(buffer, "-");
    return;
}

void get_prefix_inst_name(char *buffer, uint8_t inst) {
    switch (inst & 0xC0) {
        case 0x00: {
            switch (inst & 0x38) {
                case 0x0: { // rlc r8
                    uint8_t idx = inst & 0x7;
                    sprintf(buffer, "rlc %s", get_r8(idx));
                    return;
                }
                case 0x8: { // rrc r8
                    uint8_t idx = inst & 0x7;
                    sprintf(buffer, "rrc %s", get_r8(idx));
                    return;
                }
                case 0x10: { // rl r8
                    uint8_t idx = inst & 0x7;
                    sprintf(buffer, "rl %s", get_r8(idx));
                    return;
                }
                case 0x18: { // rr r8
                    uint8_t idx = inst & 0x7;
                    sprintf(buffer, "rr %s", get_r8(idx));
                    return;
                }
                case 0x20: { // sla r8
                    uint8_t idx = inst & 0x7;
                    sprintf(buffer, "sla %s", get_r8(idx));
                    return;
                }
                case 0x28: { // sra r8
                    uint8_t idx = inst & 0x7;
                    sprintf(buffer, "sra %s", get_r8(idx));
                    return;
                }
                case 0x30: { // swap r8
                    uint8_t idx = inst & 0x7;
                    sprintf(buffer, "swap %s", get_r8(idx));
                    return;
                }
                case 0x38: { // srl r8
                    uint8_t idx = inst & 0x7;
                    sprintf(buffer, "srl %s", get_r8(idx));
                    return;
                }
            }
            break;
        }
        case 0x40: { // bit b3, r8
            uint8_t idx = inst & 0x7;
            uint8_t bit_idx = (inst >> 3) & 0x7;
            sprintf(buffer, "bit %d, %s", bit_idx, get_r8(idx));
            return;
        }
        case 0x80: { // res b3, r8
            uint8_t idx = inst & 0x7;
            uint8_t bit_idx = (inst >> 3) & 0x7;
            sprintf(buffer, "res %d, %s", bit_idx, get_r8(idx));
            return;
        }
        case 0xC0: { // set b3, r8
            uint8_t idx = inst & 0x7;
            uint8_t bit_idx = (inst >> 3) & 0x7;
            sprintf(buffer, "set %d, %s", bit_idx, get_r8(idx));
            return;
        }
    }

    sprintf(buffer, "-");
    return;
}