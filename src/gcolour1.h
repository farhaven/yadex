/*
 *	gcolour1.h
 *	Allocate and free the game colours.
 *
 *	By "game colours", I mean the colours used to draw
 *	the game graphics (flats, textures, sprites), as
 *	opposed to the "application colours" which don't
 *	depend on the the game and are used to draw the
 *	windows, the menus and the map.
 *
 *	The application colours are handled in acolours.cc.
 *
 *	AYM 1998-11-29
 */


#include "colour.h"


pcolour_t *alloc_game_colours (int playpalnum);
void free_game_colours (pcolour_t *game_colours);

