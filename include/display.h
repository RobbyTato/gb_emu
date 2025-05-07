#ifndef DISPLAY_H
#define DISPLAY_H

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#define PIXEL_SIZE 4
#define DISP_WIDTH 160
#define DISP_HEIGHT 144

extern SDL_Window *window;

void init_display(void);
void free_display(void);
void update_display(void);

#endif // DISPLAY_H