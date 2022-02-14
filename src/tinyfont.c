#include <stdio.h>
#include <stdlib.h>

#include <eris/king.h>

#include "tinyfont.h"
#include "main.h"

/*
1110 0010 1110 1110 1010 1110 1110 1110 1110 1110
1010 0010 0010 0010 1010 1000 1000 0010 1010 1010
1010 0010 1110 0110 1110 1110 1110 0010 1110 1110
1010 0010 1000 0010 0010 0010 1010 0010 1010 0010
1110 0010 1110 1110 0010 1110 1110 0010 1110 1110
*/

static const unsigned char miniDecimalData[] = { 0xE2, 0xEE, 0xAE, 0xEE, 0xEE,
0xA2, 0x22, 0xA8, 0x82, 0xAA,
0xA2, 0xE6, 0xEE, 0xE2, 0xEE,
0xA2, 0x82, 0x22, 0xA2, 0xA2,
0xE2, 0xEE, 0x2E, 0xE2, 0xEE };

static unsigned short miniDecimalFonts[TINY_FONT_NUM_PIXELS];

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
				miniDecimalFonts[i++] = miniDecimalPixels[n * TINY_FONT_WIDTH + x + y * TINY_FONT_WIDTH * TINY_FONTS_NUM] * 0xFF7F;
			}
		}
	}
}

static void setKramPos(int posX, int posY)
{
	eris_king_set_kram_write((posY * SCREEN_WIDTH + posX)>>1, 1);	// will not be correct if posX is odd (but we will remember to use even numbers for now)
}

static void drawFont(int posX, int posY, unsigned char decimal)
{
	int x, y;
	unsigned char c0,c1;
    unsigned short *fontData = &miniDecimalFonts[decimal * TINY_FONT_WIDTH * TINY_FONT_HEIGHT];

    for (y = 0; y<TINY_FONT_HEIGHT; y++) {
		setKramPos(posX, posY+y);

		for (x = 0; x<TINY_FONT_WIDTH; x+=2) {
			c0 = *fontData++;
			c1 = *fontData++;
			eris_king_kram_write((c0 << 8) | c1);
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
