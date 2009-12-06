/*
 *	gfx2.cc
 *	Graphics routines for debugging.
 *	AYM 1998-11-17
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
#include <X11/Xlib.h>
#include "colour.h"
#include "gcolour2.h"
#include "gfx.h"
#include "rgb.h"


/*
 *	show_font
 *	Draw all of the characters of the font in a window
 */
void show_font ()
{
int width  = 2 * BOX_BORDER + 2 * WIDE_HSPACING + 16 * FONTW;
int height = 2 * BOX_BORDER + 2 * WIDE_VSPACING + 16 * FONTH;
int x0 = (ScrMaxX + 1 - width) / 2;
int y0 = (ScrMaxY + 1 - height) / 2;
int text_x0 = x0 + BOX_BORDER + WIDE_HSPACING;
int text_y0 = y0 + BOX_BORDER + WIDE_VSPACING;

DrawScreenBox3D (x0, y0, x0 + width - 1, y0 + height - 1);
set_colour (WINFG);
char buf[17];
for (int i = 0; i < 16; i ++)
   {
   for (int j = 0; j < 16; j++)
      buf[j] = 16 * i + j;
   XDrawString (dpy, drw, gc,
      text_x0 - font_xofs,
      text_y0 + FONTH * i + font_yofs,
      buf, 16);
   }
get_key_or_click ();
}


/*
 *	show_colours
 */
void show_colours ()
{
int ncolours = get_pcolours_count ();
const int columns = 16;
int lines = (ncolours + columns - 1) / columns;
const int pixels = 16;
int width = 2 * BOX_BORDER + 2 * WIDE_HSPACING + columns * (pixels + 1);
int height = 2 * BOX_BORDER + 2 * WIDE_VSPACING + lines * (pixels + 1);
int x0 = (ScrMaxX - width) / 2;
int y0 = (ScrMaxY - height) / 2;

DrawScreenBox3D (x0, y0, x0 + width - 1, y0 + height - 1);

// Draw a small square for each colour

int ix0 = x0 + BOX_BORDER + WIDE_HSPACING;
int x = 0;  // Initialized to prevent GCC from warning
int y = 0;  // Initialized to prevent GCC from warning
push_colour (0);  // Save current colour
for (int n = 0; n < ncolours; n++)
   {
   if (n % columns == 0)
      {
      x = ix0;
      if (n == 0)
         y = y0 + BOX_BORDER + WIDE_VSPACING;
      else
         y += pixels + 1;
      }
   else
      x += pixels + 1;
   set_pcolour (get_pcolour_pcn (n));
   DrawScreenBox (x, y, x + pixels - 1, y + pixels - 1);
   }
pop_colour ();  // Restore current colour

get_key_or_click ();
}


