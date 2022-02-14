#include <eris/v810.h>
#include <eris/king.h>
#include <eris/tetsu.h>
#include <eris/romfont.h>
#include <eris/timer.h>
#include <eris/cd.h>
#include <eris/low/pad.h>
#include <eris/low/scsi.h>

#include <math.h>

#include "main.h"
#include "script.h"
#include "tinyfont.h"


unsigned char framebuffer[SCREEN_SIZE_IN_PIXELS];

static unsigned short myPal[256];


/*
static int ticks = 0;
static int prevT = 0;

static void initTimer()
{
	eris_timer_init();
	eris_timer_set_period(65535);
	eris_timer_start(0);
}

static void updateLameTimer()
{
	int t = eris_timer_read_counter();
	if (t > prevT) ticks++;
	prevT = t;
}

static void updateLameTimerSum()
{
	int t = eris_timer_read_counter();
	int dt = prevT - t;
	if (dt < 0) dt = 0;
	ticks += dt;
	prevT = t;
}

static void vsync()
{
	while(eris_tetsu_get_raster() !=200) {};
}
*/


static void initPal()
{
	int i;
	for (i=0; i<64; ++i) {
		myPal[i] = (i<<10) | (i<<0);
	}
	for (i=64; i<128; ++i) {
		myPal[i] = (255<<8) | (255 + 64 - i);
	}
	for (i=128; i<192; ++i) {
		myPal[i] = (255<<8) | (192 - 2*(i - 128));
	}
	for (i=192; i<256; ++i) {
		myPal[i] = ((255 - i)<<10) | ((255 - i)<<0);
	}
}

static void initDisplay()
{
	int i;
	u16 microprog[16];

	eris_king_init();
	eris_tetsu_init();

	eris_tetsu_set_priorities(0, 0, 1, 0, 0, 0, 0);
	eris_tetsu_set_king_palette(0, 0, 0, 0);
	eris_tetsu_set_rainbow_palette(0);

	eris_king_set_bg_prio(KING_BGPRIO_0, KING_BGPRIO_HIDE, KING_BGPRIO_HIDE, KING_BGPRIO_HIDE, 0);
	eris_king_set_bg_mode(KING_BGMODE_256_PAL, KING_BGPRIO_HIDE, KING_BGPRIO_HIDE, KING_BGPRIO_HIDE);

	eris_king_set_kram_pages(0, 0, 0, 0);

	for(i = 0; i < 16; i++) {
		microprog[i] = KING_CODE_NOP;
	}

	microprog[0] = KING_CODE_BG0_CG_0;
	microprog[1] = KING_CODE_BG0_CG_1;
	microprog[2] = KING_CODE_BG0_CG_2;
	microprog[3] = KING_CODE_BG0_CG_3;

	eris_king_disable_microprogram();
	eris_king_write_microprogram(microprog, 0, 16);
	eris_king_enable_microprogram();

	eris_tetsu_set_rainbow_palette(0);
	for(i = 0; i < 256; i++) {
		eris_tetsu_set_palette(i, myPal[i]);
	}

	eris_tetsu_set_video_mode(TETSU_LINES_262, 0, TETSU_DOTCLOCK_5MHz, TETSU_COLORS_16, TETSU_COLORS_16, 0, 0, 1, 0, 0, 0, 0);

	eris_king_set_bat_cg_addr(KING_BG0, 0, 0);
	eris_king_set_bat_cg_addr(KING_BG0SUB, 0, 0);
	eris_king_set_scroll(KING_BG0, 0, 0);
	eris_king_set_bg_size(KING_BG0, KING_BGSIZE_256, KING_BGSIZE_256, KING_BGSIZE_256, KING_BGSIZE_256);

	eris_king_set_kram_read(0, 1);
	eris_king_set_kram_write(0, 1);

	for(i = 0; i < SCREEN_SIZE_IN_PIXELS; ++i) {
		eris_king_kram_write(0);
	}
	eris_king_set_kram_write(0, 1);
}

void bufDisplay()
{
	int i;
	//unsigned int *dst = (unsigned int*)framebuffer;

	eris_king_set_kram_read(0, 1);
	eris_king_set_kram_write(0, 1);

	for(i = 0; i < SCREEN_SIZE_IN_PIXELS; i+=2) {
		eris_king_kram_write(framebuffer[i] << 8 | framebuffer[i+1]);
	}

	eris_king_set_kram_write(0, 1);
}

/*static void testDisplay(int t)
{
	int i;

	int ii = t;
	for(i = 0; i < SCREEN_SIZE_IN_PIXELS; ++i) {
		framebuffer[i] = ii++;
	}

	bufDisplay();
}*/

int main()
{
	static int t = 0;

	initPal();
	initDisplay();

	initTinyFonts();

	//initTimer();
	
	initDivs();
	
	for(;;) {
		runAnimationScript();
		
		bufDisplay();

		//drawNumber(16,216, t);
	}

	return 0;
}
