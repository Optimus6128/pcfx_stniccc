#include <eris/tetsu.h>

#include "script.h"
#include "scene1.h"
#include "main.h"


#define MAX_POLYS 256

#define DIV_TAB_SIZE 4096
#define DIV_TAB_SHIFT 16

#define ATARI_PAL_NUM 16
#define MAX_POLYGON_PTS 16


static int32 divTab[DIV_TAB_SIZE];

static uint32 block64index = 0;

static uchar *data = &scene1_bin[0];

static MyPoint2D pt[MAX_POLYGON_PTS];

static QuadStore quads[MAX_POLYS];
static QuadStore *quadPtr;
static int numQuads = 0;

static int leftEdgeFlat[SCREEN_HEIGHT];
static int rightEdgeFlat[SCREEN_HEIGHT];

static MyPoint2D vi[256];

static bool mustClearScreen = false;

//static bool firstTime = true;
static bool endOfBench = false;

static uint8 *animBufferPtr;

/*static char stbuffer[10];
static char avgfpsbuffer[16];
static char texnumbuffer[8];

static int startBenchTime;
static int frameNum = 0;*/


void initDivs()
{
    int i, ii;
    for (i=0; i<DIV_TAB_SIZE; ++i) {
        ii = i - DIV_TAB_SIZE / 2;
        if (ii==0) ++ii;

        divTab[i] = (1 << DIV_TAB_SHIFT) / ii;
    }
}

static void prepareEdgeListFlat(MyPoint2D *p0, MyPoint2D *p1)
{
	int *edgeListToWriteFlat;
	MyPoint2D *pTemp;

	if (p0->y == p1->y) return;

	// Assumes CCW
	if (p0->y < p1->y) {
		edgeListToWriteFlat = leftEdgeFlat;
	}
	else {
		edgeListToWriteFlat = rightEdgeFlat;

		pTemp = p0;
		p0 = p1;
		p1 = pTemp;
	}

    {
        const int x0 = p0->x; const int y0 = p0->y;
        const int x1 = p1->x; const int y1 = p1->y;

        const int dx = ((x1 - x0) * divTab[y1 - y0 + DIV_TAB_SIZE / 2]) >>  (DIV_TAB_SHIFT - FP_BITS);

        int xp = INT_TO_FIXED(x0, FP_BITS);
        int count = y1 - y0;

        edgeListToWriteFlat = &edgeListToWriteFlat[y0];

        do {
            *edgeListToWriteFlat++ = FIXED_TO_INT(xp, FP_BITS);
            xp += dx;
		} while(--count > 0);
    }
}


void drawFlatQuad8(MyPoint2D *p, uchar color, uchar *screen)
{
	const int y0 = p[0].y;
	const int y1 = p[1].y;
	const int y2 = p[2].y;
	const int y3 = p[3].y;

	int yMin = y0;
	int yMax = yMin;

	if (y1 < yMin) yMin = y1;
	if (y1 > yMax) yMax = y1;
	if (y2 < yMin) yMin = y2;
	if (y2 > yMax) yMax = y2;
	if (y3 < yMin) yMin = y3;
	if (y3 > yMax) yMax = y3;

	prepareEdgeListFlat(&p[0], &p[1]);
	prepareEdgeListFlat(&p[1], &p[2]);
	prepareEdgeListFlat(&p[2], &p[3]);
	prepareEdgeListFlat(&p[3], &p[0]);

	{
		int y = yMin;
		uchar *dst = screen + y * SCREEN_WIDTH_IN_BYTES;
		uchar col8 = (color << 4) | color;

		int count = yMax - yMin;
		do {
			const int xl = leftEdgeFlat[y];
			int length = rightEdgeFlat[y++]-xl+1;
			uchar *dst8 = dst + (xl>>1);

			if (xl & 1) {
				*dst8 = (*dst8 & 0xF0) | color;
				++dst8;
				--length;
			}
			if (length & 1) {
				uchar *dst8r = dst8 + (length >> 1);
				*dst8r = (*dst8r & 0x0F) | (color << 4);
				--length;
			}

			length >>= 1;
			while(length-- > 0) {
				*dst8++ = col8;
			};

			dst += SCREEN_WIDTH_IN_BYTES;
		} while(--count > 0);
	}
}

static void addPolygon(int numVertices, int paletteIndex)
{
	int pBaseIndex = 0;
	int pStartIndex = 1;
	const int maxIndex = numVertices - 1;

    ushort color;

    if (paletteIndex < 0) paletteIndex = 0;

    color = paletteIndex;

	if (numVertices < 3 || numVertices > 16) return;

	while(pStartIndex < maxIndex)
	{
		quadPtr->p0.x = pt[pBaseIndex].x;		quadPtr->p0.y = pt[pBaseIndex].y;
		quadPtr->p1.x = pt[pStartIndex].x;		quadPtr->p1.y = pt[pStartIndex].y;
		quadPtr->p2.x = pt[pStartIndex+1].x;	quadPtr->p2.y = pt[pStartIndex + 1].y;

		pStartIndex += 2;
		if (pStartIndex > maxIndex) pStartIndex = maxIndex;
		quadPtr->p3.x = pt[pStartIndex].x;		quadPtr->p3.y = pt[pStartIndex].y;

		quadPtr->c = color;

		++quadPtr;
		++numQuads;
	}
}

static void renderPolygonsSoftware8()
{
	int i;
	static MyPoint2D p[4];

	for (i=0; i<numQuads; ++i) {
		p[0].x = quads[i].p0.x; p[0].y = quads[i].p0.y;
		p[1].x = quads[i].p1.x; p[1].y = quads[i].p1.y;
		p[2].x = quads[i].p2.x; p[2].y = quads[i].p2.y;
		p[3].x = quads[i].p3.x; p[3].y = quads[i].p3.y;
		drawFlatQuad8(&p[0], (uchar)quads[i].c, animBufferPtr);
	}
}


static void interpretPaletteData()
{
    int i, r, g, b;
    uchar bitmaskH = *data++;
	uchar bitmaskL = *data++;

	int bitmask = (bitmaskH << 8) | bitmaskL;

	for (i = 0; i < 16; ++i) {
		if (bitmask & 0x8000) {
			uchar colorH = *data++;
			uchar colorL = *data++;
			int color = (colorH << 8) | colorL;

			r = (color >> 8) & 7;
			g = (color >> 4) & 7;
			b = color & 7;

			eris_tetsu_set_palette(i, RGB2YUV(r<<5, g<<5, b<<5));
		}
		bitmask <<= 1;
	}
}

static void interpretDescriptorSpecial(uchar descriptor)
{
	switch (descriptor)
	{
        case 0xff:
        {
            // End of Frame
        }
        break;

        case 0xfe:
        {
            // End of frame and skip at next 64k block

            ++block64index;
            data = &scene1_bin[block64index << 16];
        }
        break;

        case 0xfd:
        {
            // That's all folks!

            // restart
            data = &scene1_bin[0];
            block64index = 0;
            endOfBench = true;
        }
        break;
	}
}

static void interpretIndexedMode()
{
    int i, n;

	uchar descriptor = 0;
	int polyPaletteIndex, polyNumVertices;

	int vertexNum = *data++;

	for (i = 0; i < vertexNum; ++i) {
		vi[i].x = (int)*data++;
		vi[i].y = (int)*data++;
	}

	while(true) {
		descriptor = *data++;
		if (descriptor >= 0xfd) break;

        polyPaletteIndex = (int)(descriptor >> 4) & 15;
        polyNumVertices = (int)(descriptor & 15);

		for (n = 0; n < polyNumVertices; ++n) {
			int vertexId = *data++;
			pt[n].x = vi[vertexId].x;
			pt[n].y = vi[vertexId].y;
		}

		addPolygon(polyNumVertices, polyPaletteIndex);
	}

	interpretDescriptorSpecial(descriptor);
}

static void interpretNonIndexedMode()
{
    int n;
	uchar descriptor = 0;
	int polyPaletteIndex, polyNumVertices;

	while (1) {
		descriptor = *data++;
		if (descriptor >= 0xfd) break;

        polyPaletteIndex = (int)(descriptor >> 4);
        polyNumVertices = (int)(descriptor & 15);

		for (n = 0; n < polyNumVertices; ++n) {
			pt[n].x = *data++;
			pt[n].y = *data++;
		}
		addPolygon(polyNumVertices, polyPaletteIndex);
	}
	interpretDescriptorSpecial(descriptor);
}

static void decodeFrame()
{
	uchar flags = *data++;

	mustClearScreen = false;
	numQuads = 0;
	quadPtr = &quads[0];

	if (flags & 1) {
		mustClearScreen = true;
	}
	if (flags & 2) {
		interpretPaletteData();
	}
	if (flags & 4) {
		interpretIndexedMode();
	}
	else {
		interpretNonIndexedMode();
	}
}

/*void hackNumToTwoDigitChars(char *buff, int num)    // no time to think of a better way
{
    if (num < 10) {
        buff[0] = '0';
        buff[1] = 48 + num;
    } else {
        sprintf(buff, "%d", num);
    }
}

void drawTimer()
{
    static int min, sec, mls, avgfps;
    int elapsed = getTicks() - startBenchTime;

    if (endOfBench) {
        int c = elapsed >> 5;
        setFontColor(MakeRGB15(c, c, c));
    } else {
        min = elapsed / 60000;
        sec = (elapsed / 1000) % 60;
        mls = (elapsed % 1000) / 10;
        avgfps = (frameNum * 1000) / elapsed;
    }

    hackNumToTwoDigitChars(&stbuffer[0], min);
    stbuffer[2] = ':';
    hackNumToTwoDigitChars(&stbuffer[3], sec);
    stbuffer[5] = ':';
    hackNumToTwoDigitChars(&stbuffer[6], mls);

    drawText(128, 216, stbuffer);

    if (endOfBench) {
        setFontColor(MakeRGB15(31, 24, 16));
        sprintf(avgfpsbuffer, "Avg FPS:%d", avgfps);
        drawText(204, 216, avgfpsbuffer);
        setFontColor(MakeRGB15(31, 31, 31));
    }
}*/

static void clearScreen()
{
	int i;

	uint32 *dst32 = (uint32*)animBufferPtr;
	for (i=0; i<SCREEN_SIZE_IN_BYTES/4; ++i) {
		*dst32++ = 0;
	}
}

void runAnimationScript()
{
	/*if (firstTime) {
		startBenchTime = getTicks();
		firstTime = false;
	}*/
	
	animBufferPtr = framebuffer;

	decodeFrame();

	if (mustClearScreen) {
		clearScreen();
	}

	renderPolygonsSoftware8();

	//drawTimer();

	//++frameNum;
}
