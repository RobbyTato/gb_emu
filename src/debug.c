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