/*
 *	acolours.h
 *	Allocate and free the application colours.
 *
 *	By "application colours", I mean the colours used to draw
 *	the windows, the menus and the map, as opposed to the
 *	"game colours" which depend on the game (they're in the
 *	PLAYPAL lump) and are used to draw the game graphics
 *	(flats, textures, sprites...).
 *
 *	The game colours are handled in gcolour1.cc and gcolour2.cc.
 *
 *	AYM 1998-11-29
 */

#include "colour.h"

pcolour_t add_app_colour (rgb_c rgb);
void delete_app_colour (acolour_t acn);
pcolour_t *commit_app_colours ();
void uncommit_app_colours (pcolour_t *app_colours);
