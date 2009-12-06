/*
 *	gcolour3.h
 *	AYM 2000-04-20
 */


/*
This file is part of Yadex.

Yadex incorporates code from DEU 5.21 that was put in the public domain in
1994 by Raphaël Quinet and Brendon Wyber.

The rest of Yadex is Copyright © 1997-2003 André Majorel and others.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place, Suite 330, Boston, MA 02111-1307, USA.
*/


#ifndef YH_GCOLOUR3  /* DO NOT INSERT ANYTHING BEFORE THIS LINE */
#define YH_GCOLOUR3


#include "colour.h"


/*
 *	Game_colour_24 - convert game colours to pixel values
 *	
 *	This class is used to speed up displaying of images on
 *	visuals where bits_per_pixel is 24.
 *
 *	Game_colour_24::lut() returns a pointer to a const table
 *	that is similar to game_colour[] except that it is
 *	optimized toward the needs of display_img() :
 *	each member can be readily copied into the XImage buffer
 *	(this is not true of game_colour[], at least not if the
 *	client is big-endian or does not have the same
 *	endianness as the server).
 *
 *	There is exactly 1 instance of this class (global). It's
 *	refreshed in InitGfx() and used in display_img().
 *	To avoid wasting memory when it's not needed (when the
 *	depth is != 24), the array is allocated upon refresh and
 *	refresh() is only called when the depth is == 24.
 */


typedef u8 pv24_t[3];  // A 24-bit pixel value


class Game_colour_24
{
  public :
    Game_colour_24 () { pv_table = 0; }
    ~Game_colour_24 () { if (pv_table) delete[] pv_table; }
    // Create/refresh the table
    void refresh (const pcolour_t *game_colour, const bool big_endian);
    // Return a pointer on an array of pv24_t[DOOM_COLOURS]
    const pv24_t *lut () { return pv_table; }
  private :
    pv24_t *pv_table;
};


extern Game_colour_24 game_colour_24;


#endif  /* DO NOT ADD ANYTHING AFTER THIS LINE */
