#ifndef MAIN_H
#define MAIN_H

#define SCREEN_WIDTH 256
#define SCREEN_HEIGHT 256
#define SCREEN_WIDTH_IN_BYTES (SCREEN_WIDTH / 2)

#define SCREEN_SIZE_IN_PIXELS (SCREEN_WIDTH * SCREEN_HEIGHT)
#define SCREEN_SIZE_IN_BYTES (SCREEN_SIZE_IN_PIXELS / 2)

#include "types.h"

extern unsigned char framebuffer[SCREEN_SIZE_IN_BYTES];

extern uint16 RGB2YUV(int r, int g, int b);

int getTicks();

#endif
