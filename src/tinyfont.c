#include <stdio.h>
#include <stdlib.h>

#include <eris/king.h>

#include "tinyfont.h"
#include "main.h"

/*
1110 0010  1110 1110  1010 1110  1110 1110  1110 1110  0000 0000
1010 0010  0010 0010  1010 1000  1000 0010  1010 1010  0100 0000
1010 0010  1110 0110  1110 1110  1110 0010  1110 1110  0000 0000
1010 0010  1000 0010  0010 0010  1010 0010  1010 0010  0100 0000
1110 0010  1110 1110  0010 1110  1110 0010  1110 1110  0000 0000
*/

static const unsigned char miniDecimalData[] = {
0xE2, 0xEE, 0xAE, 0xEE, 0xEE, 0x00, 
0xA2, 0x22, 0xA8, 0x82, 0xAA, 0x40, 
0xA2, 0xE6, 0xEE, 0xE2, 0xEE, 0x00, 
0xA2, 0x82, 0x22, 0xA2, 0xA2, 0x40, 
0xE2, 0xEE, 0x2E, 0xE2, 0xEE, 0x00
};

static unsigned char miniDecimalFonts[TINY_FONT_NUM_PIXELS];

void initTinyFonts()
{
	static unsigned char miniDecimalPixels[TINY_FONT_NUM_PIXELS];
	int i,j,k=0;
	int x,y,n;

	for (i = 0; i < TINY_FONT_NUM_PIXELS / 8; i++) {
		unsigned char d = miniDecimalData[i];
		for (j = 0; j < 8; j++) {
			unsigned char c = (d & 0x80) >> 7;
			miniDecimalPixels[k++] = c;
			d <<= 1;
		}
	}

	i = 0;
	for (n = 0; n < TINY_FONTS_NUM; n++) {
		for (y = 0; y < TINY_FONT_HEIGHT; y++) {
			for (x = 0; x < TINY_FONT_WIDTH; x++) {
				miniDecimalFonts[i++] = miniDecimalPixels[n * TINY_FONT_WIDTH + x + y * TINY_FONT_WIDTH * TINY_FONTS_NUM] * 3;	// chose color number 3 that doesn't get too dark during STNICCC constantly palette changing animation
			}
		}
	}
}

static void setKramPos(int posX, int posY)
{
	eris_king_set_kram_write((posY * SCREEN_WIDTH + posX)>>2, 1);	// will not be correct if posX is odd (but we will remember to use even numbers for now)
}

void drawFont(int posX, int posY, int index)
{
	int x, y;
	unsigned char c0,c1,c2,c3;
    unsigned char *fontData = &miniDecimalFonts[index * TINY_FONT_WIDTH * TINY_FONT_HEIGHT];

    for (y = 0; y<TINY_FONT_HEIGHT; y++) {
		setKramPos(posX, posY+y);

		for (x = 0; x<TINY_FONT_WIDTH; x+=4) {
			c0 = *fontData++;
			c1 = *fontData++;
			c2 = *fontData++;
			c3 = *fontData++;
			eris_king_kram_write((c0 << 12) | (c1 << 8) | (c2 << 4) | c3);
		}
    }
}

void drawNumber(int posX, int posY, int number)
{
    static char buffer[10];
	int i = 0;

	sprintf(buffer, "%d", number);

	while(i < 10 && buffer[i] != 0) {
        drawFont(posX + i * TINY_FONT_WIDTH, posY, buffer[i] - 48);
		i++;
    }
}
