#ifndef SCRIPT_MAIN_H
#define SCRIPT_MAIN_H

#include "types.h"

#define ANIM_WIDTH 256
#define ANIM_HEIGHT 200
#define ANIM_SIZE (ANIM_WIDTH * ANIM_HEIGHT)


typedef struct MyPoint2D
{
    int x, y;
}MyPoint2D;

typedef struct QuadStore
{
	MyPoint2D p0, p1, p2, p3;
	int c;
}QuadStore;


#define FP_BITS 12

#define INT_TO_FIXED(i,b) ((i) << b)
#define FIXED_TO_INT(x,b) ((x) >> b)


void runAnimationScript(void);
void initDivs(void);

#endif
