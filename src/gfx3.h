/*
 *	gfx3.h
 *	AYM 1999-06-06
 */


#include "rgbbmp.h"


void window_to_rgbbmp (int x, int y, int width, int height, Rgbbmp &b);
int rgbbmp_to_rawppm (const Rgbbmp &b, const char *file_name);


