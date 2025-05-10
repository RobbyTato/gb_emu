#include <display.h>
#include <mem.h>
#include <cpu.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdlib.h>
#include <stdio.h>

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_Texture *texture = NULL;
Uint32 *framebuffer = NULL;
static const Uint32 palette[4] = 
    {0xFFFFFFFF, 0xFFAAAAAA, 0xFF555555, 0xFF000000};
size_t last_mode = 2;
size_t last_pixel = 0;

void init_display(void) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "SDL init failed: %s\n", SDL_GetError());
        exit(1);
    }
    window = SDL_CreateWindow(
        "GBEMU",
        DISP_WIDTH * PIXEL_SIZE,
        DISP_HEIGHT * PIXEL_SIZE,
        0
    );
    if (!window) {
        fprintf(stderr, "SDL init failed: %s\n", SDL_GetError());
        exit(1);
    }
    renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) {
        fprintf(stderr, "SDL init failed: %s\n", SDL_GetError());
        exit(1);
    }
    texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        160, 144
    );
    if (!texture) {
        fprintf(stderr, "SDL init failed: %s\n", SDL_GetError());
        exit(1);
    }
    if (!SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_PIXELART)) {
        fprintf(stderr, "SDL init failed: %s\n", SDL_GetError());
        exit(1);
    }
    void* pixels;
    int pitch;
    if (!SDL_LockTexture(texture, NULL, &pixels, &pitch)) {
        fprintf(stderr, "SDL lock failed: %s\n", SDL_GetError());
        exit(1);
    }
    memset(pixels, 0, DISP_WIDTH * DISP_HEIGHT * sizeof(Uint32));
    framebuffer = pixels;
}

void free_display(void) {
    framebuffer = NULL;
    SDL_DestroyTexture(texture);
    texture = NULL;
    SDL_DestroyRenderer(renderer);
    renderer = NULL;
    SDL_DestroyWindow(window);
    window = NULL;
    SDL_Quit();
}

// TODO: Incorporate window and OAM draws, as well as disabling these draws
// TODO: with LCDC.
void draw_pixels_until(size_t until) {
    // Get y pos of tile and the tile map of the tile
    uint8_t tile_y = ((r_ly + r_scy) % 256) / 8;
    uint16_t tile_map = r_lcdc & LCDC_BG_TILE_MAP_AREA ? 1 : 0;
    
    // if (r_lcdc & LCDC_BG_TILE_MAP_AREA) {
    //     if (r_lcdc & LCDC_BG_WIN_TILE_DATA_AREA) {
    //         printf("1\n");
    //     } else {
    //         printf("2\n");
    //     }
    // } else {
    //     if (r_lcdc & LCDC_BG_WIN_TILE_DATA_AREA) {
    //         printf("3\n");
    //     } else {
    //         printf("4\n");
    //     }
    // }

    for (; last_pixel < until; last_pixel++) {
        // Get x pos and index of the tile
        uint8_t tile_x = ((last_pixel + r_scx) % 256) / 8;
        uint8_t tile_index = vram_maps[tile_map][(32 * tile_y) + tile_x];

        // Get the exact pixel
        uint8_t pixel_x = (last_pixel + r_scx) % 8;
        uint8_t pixel_y = (r_ly + r_scy) % 8;

        // Get the color index of the pixel
        size_t idx_offset = pixel_y * 2;
        size_t shift_offset = 7 - pixel_x;
        uint8_t lsb, msb;
        if (r_lcdc & LCDC_BG_WIN_TILE_DATA_AREA) {
            lsb = (vram_tiles[tile_index][idx_offset] >> shift_offset) & 1;
            msb = (vram_tiles[tile_index][idx_offset + 1] >> shift_offset) & 1;
        } else {
            size_t new_index = 256 + (int8_t)tile_index;
            lsb = (vram_tiles[new_index][idx_offset] >> shift_offset) & 1;
            msb = (vram_tiles[new_index][idx_offset + 1] >> shift_offset) & 1;
        }
        uint8_t pixel_color_index = (msb << 1) | lsb;

        // Get the color of the pixel and draw it
        Uint32 color = palette[(r_bgp >> (pixel_color_index * 2)) & 0x3];
        framebuffer[(160 * r_ly) + last_pixel] = color;
    }
}

// Returns true if this call updates the renderer. This functionality is used
// to set a framerate by knowing when a frame was drawn
bool update_display(double frame_start) {
    if (!(r_lcdc & LCDC_LCD_PPU_ENABLED)) {
        // TODO: Clear display
        return false;
    }
    size_t frame_dots = dots % 70224;
    size_t scanline_dots = frame_dots % 456;
    r_ly = frame_dots / 456;
    // Mode 1 (Vertical Blank)
    if (144 <= r_ly && r_ly <= 153) {
        if (last_mode != 1) {
            last_mode = 1;
            r_if |= 1; // Send VBlank Interrupt
            // Draw to the renderer
            SDL_UnlockTexture(texture);
            SDL_RenderTexture(renderer, texture, NULL, NULL);
            SDL_RenderPresent(renderer);
            void* pixels;
            int pitch;
            if (!SDL_LockTexture(texture, NULL, &pixels, &pitch)) {
                fprintf(stderr, "SDL lock failed: %s\n", SDL_GetError());
                exit(1);
            }
            memset(pixels, 0, DISP_WIDTH * DISP_HEIGHT * sizeof(Uint32));
            framebuffer = pixels;
            // Delay to set frame rate at 59.7 fps
            double fps = 59.7;
            double frame_duration_ns = 1000000000.0 / fps;
            double frame_end = (double)SDL_GetPerformanceCounter();
            double frame_length = frame_end - frame_start;
            if (frame_length < frame_duration_ns) {
                SDL_DelayNS((Uint64)(frame_duration_ns - frame_length));
            }
            return true;
        }
        return false;
    }
    // Mode 2 (OAM scan)
    if (last_mode != 2 && scanline_dots < 80) {
        last_mode = 2;
        // TODO: Do the OAM scan
        return false;
    }
    // Mode 3 (Drawing pixels)
    if (80 <= scanline_dots && scanline_dots < 252) {
        last_mode = 3;
        // Tile fetch (ignore)
        if (scanline_dots < 92) {
            return false;
        }
        draw_pixels_until(scanline_dots - 91);
        return false;
    }
    // Mode 0 (Horizontal Blank)
    if (last_mode != 0 && 252 <= scanline_dots && scanline_dots < 456) {
        last_mode = 0;
        draw_pixels_until(160);
        last_pixel = 0;
        return false;
    }
    return false;
}