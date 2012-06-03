/*
 *	drawmap.h
 *	AYM 1998-09-06
 */


#ifndef YH_DRAWMAP
#define YH_DRAWMAP

#include "_edit.h"


int vertex_radius (double scale);

void draw_map (edit_t *e);
void draw_infobar (const edit_t *e);
const double get_thing_scale(wad_ttype_t);

#endif
