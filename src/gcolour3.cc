/*
 *	gcolour3.cc
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


#include "yadex.h"
#include "colour.h"
#include "gcolour3.h"


Game_colour_24 game_colour_24;


void Game_colour_24::refresh (const pcolour_t *game_colour, bool big_endian)
{
  if (pv_table == 0)
    pv_table = new pv24_t[DOOM_COLOURS];

  if (big_endian)
  {
    for (size_t n = 0; n < DOOM_COLOURS; n++)
    {
      pv_table[n][0] = game_colour[n] / 0x10000;
      pv_table[n][1] = game_colour[n] / 0x100;
      pv_table[n][2] = game_colour[n];
    }
  }
  else
  {
    for (size_t n = 0; n < DOOM_COLOURS; n++)
    {
      pv_table[n][0] = game_colour[n];
      pv_table[n][1] = game_colour[n] / 0x100;
      pv_table[n][2] = game_colour[n] / 0x10000;
    }
  }
}

