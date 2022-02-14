#ifndef MAIN_H
#define MAIN_H

#define SCREEN_WIDTH 256
#define SCREEN_HEIGHT 256

#define SCREEN_SIZE_IN_PIXELS (SCREEN_WIDTH * SCREEN_HEIGHT)

#include "types.h"

extern unsigned char framebuffer[SCREEN_SIZE_IN_PIXELS];

extern uint16 RGB2YUV(int r, int g, int b);

#endif
