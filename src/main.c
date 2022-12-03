#include <eris/v810.h>
#include <eris/king.h>
#include <eris/tetsu.h>
#include <eris/romfont.h>
#include <eris/timer.h>
#include <eris/cd.h>
#include <eris/low/pad.h>
#include <eris/low/scsi.h>
#include <eris/low/soundbox.h>

#include <math.h>

#include "main.h"
#include "script.h"
#include "tinyfont.h"
#include "fastking.h"


unsigned char framebuffer[SCREEN_SIZE_IN_BYTES];

static int nframe = 0;


volatile int __attribute__ ((zda)) zda_timer_count = 0;

/* Declare this "noinline" to ensure that my_timer_irq() is not a leaf. */
__attribute__ ((noinline)) void increment_zda_timer_count (void)
{
	zda_timer_count++;
}

/* Simple test interrupt_handler that is not a leaf. */
/* Because it is not a leaf function, it will use the full IRQ preamble. */
__attribute__ ((interrupt)) void my_timer_irq (void)
{
	eris_timer_ack_irq();

	increment_zda_timer_count();
}

static void initTimer()
{
	// The PC-FX firmware leaves a lot of hardware actively generating
	// IRQs when a program starts, and it is only because the V810 has
	// interrupts-disabled that the firmware IRQ handlers are not run.
	//
	// You *must* mask/disable/reset the existing IRQ sources and init
	// new handlers before enabling the V810's interrupts!

	// Disable all interrupts before changing handlers.
	irq_set_mask(0x7F);

	// Replace firmware IRQ handlers for the Timer and HuC6270-A.
	//
	// This liberis function uses the V810's hardware IRQ numbering,
	// see FXGA_GA and FXGABOAD documents for more info ...
	irq_set_raw_handler(0x9, my_timer_irq);

	// Enable Timer interrupt.
	//
	// d6=Timer
	// d5=External
	// d4=KeyPad
	// d3=HuC6270-A
	// d2=HuC6272
	// d1=HuC6270-B
	// d0=HuC6273
	irq_set_mask(0x3F);

	// Reset and start the Timer.
	eris_timer_init();
	eris_timer_set_period(1432); /* approx 1/1000th of a second */
	eris_timer_start(1);

	// Hmmm ... this needs to be cleared here for some reason ... there's
	// probably a bug to find somewhere!
	zda_timer_count = 0;

	// Allow all IRQs.
	//
	// This liberis function uses the V810's hardware IRQ numbering,
	// see FXGA_GA and FXGABOAD documents for more info ...
	irq_set_level(8);

	// Enable V810 CPU's interrupt handling.
	irq_enable();
}

uint16 RGB2YUV(int r, int g, int b)
{
	uint16 yuv;

	int Y = (19595 * r + 38469 * g + 7471 * b) >> 16;
	int U = ((32243 * (b - Y)) >> 16) + 128;
	int V = ((57475 * (r - Y)) >> 16) + 128;

	if (Y <   0) Y =   0;
	if (Y > 255) Y = 255;
	if (U <   0) U =   0;
	if (U > 255) U = 255;
	if (V <   0) V =   0;
	if (V > 255) V = 255;

	yuv = (uint16)((Y<<8) | ((U>>4)<<4) | (V>>4));

	return yuv;
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
	eris_king_set_bg_mode(KING_BGMODE_16_PAL, KING_BGPRIO_HIDE, KING_BGPRIO_HIDE, KING_BGPRIO_HIDE);

	eris_king_set_kram_pages(0, 0, 0, 0);

	for(i = 0; i < 16; i++) {
		microprog[i] = KING_CODE_NOP;
	}

	microprog[0] = KING_CODE_BG0_CG_0;
	microprog[1] = KING_CODE_BG0_CG_1;

	eris_king_disable_microprogram();
	eris_king_write_microprogram(microprog, 0, 16);
	eris_king_enable_microprogram();

	eris_tetsu_set_rainbow_palette(0);
	for(i = 0; i < 16; i++) {
		eris_tetsu_set_palette(i, RGB2YUV(0,0,0));
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

static void initCDplay()
{
	eris_low_cdda_set_volume(63,63);
	cd_playtrk(2, 0);
	cd_endtrk(3, 0x4);
}

void bufDisplay32()
{
	eris_king_set_kram_write((SCREEN_WIDTH_IN_BYTES/2) * ((236-ANIM_HEIGHT)/2), 1);	//4 to 235

	king_kram_write_buffer_bytes(framebuffer, ANIM_SIZE/2);

	eris_king_set_kram_write(0, 1);
}

static int getFps()
{
	static int fps = 0;
	static int prev_sec = 0;
	static int prev_nframe = 0;

	const int curr_sec = zda_timer_count / 1000;
	if (curr_sec != prev_sec) {
		fps = nframe - prev_nframe;
		prev_sec = curr_sec;
		prev_nframe = nframe;
	}
	return fps;
}

int getTicks()
{
	return zda_timer_count;
}

int main()
{
	initDisplay();
	initTimer();

	initTinyFonts();
	initDivs();

	initCDplay();

	for(;;) {
		runAnimationScript();

		bufDisplay32();

		drawNumber(8,8, getFps());

		++nframe;
	}

	return 0;
}
