#ifndef TINY_FONT_H
#define TINY_FONT_H

#define TINY_FONT_WIDTH 4
#define TINY_FONT_HEIGHT 5
#define TINY_FONTS_NUM 12
#define TINY_FONT_NUM_PIXELS (TINY_FONTS_NUM * TINY_FONT_WIDTH * TINY_FONT_HEIGHT)

void initTinyFonts();
void drawNumber(int posX, int posY, int number);
void drawFont(int posX, int posY, int index);

#endif
