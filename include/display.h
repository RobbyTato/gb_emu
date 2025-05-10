#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdbool.h>
#include <stdlib.h>

#define PIXEL_SIZE 4
#define DISP_WIDTH 160
#define DISP_HEIGHT 144

extern size_t last_mode;

void init_display(void);
void free_display(void);
bool update_display(double frame_start);

#endif // DISPLAY_H