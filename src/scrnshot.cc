/*
 *	scrnshot.cc
 *	This module contains the function ScreenShot() which is called
 *	whenever you press shift-F1 in edit mode. It is only a wrapper
 *	for the gifsave library.
 *	AYM 1997-08-20
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
#if defined Y_UNIX
#elif defined Y_DOS
#include <gifsave.h>
#endif


#ifdef Y_DOS
static int ShotGetPixel (int x, int y);
#endif


void ScreenShot (void)
#if defined Y_UNIX
{
  return;  // FIXME
}
#elif defined Y_DOS
{
  int n;
  int r;
  int ShotWidth, ShotHeight;

  ShotWidth = ScrMaxX+1;
  ShotHeight = ScrMaxY+1-17-12;

  LogMessage ("Writing screen shot to file yadex.gif\n");
  r = GIF_Create ("yadex.gif", ShotWidth, ShotHeight, 16, 3);
  if (r != GIF_OK)
    LogMessage ("GIF_Create error %d\n", r);
  GIF_SetColor ( 0, 0, 0, 0);  // BLACK
  GIF_SetColor ( 1, 0, 0, 4);  // BLUE
  GIF_SetColor ( 2, 0, 4, 0);  // GREEN
  GIF_SetColor ( 3, 0, 4, 4);  // CYAN
  GIF_SetColor ( 4, 4, 0, 0);  // RED
  GIF_SetColor ( 5, 4, 0, 4);  // MAGENTA
  GIF_SetColor ( 6, 4, 3, 0);  // BROWN
  GIF_SetColor ( 7, 4, 4, 4);  // LIGHTGREY
  GIF_SetColor ( 8, 2, 2, 2);  // DARKGREY
  GIF_SetColor ( 9, 0, 0, 7);  // LIGHTBLUE
  GIF_SetColor (10, 0, 7, 0);  // LIGHTGREEN
  GIF_SetColor (11, 0, 7, 7);  // LIGHTCYAN
  GIF_SetColor (12, 7, 0, 0);  // LIGHTRED
  GIF_SetColor (13, 7, 0, 7);  // LIGHTMAGENTA
  GIF_SetColor (14, 7, 7, 0);  // YELLOW
  GIF_SetColor (15, 7, 7, 7);  // WHITE
  r = GIF_CompressImage (0, 17, ShotWidth, ShotHeight, ShotGetPixel);
  if (r != GIF_OK)
    LogMessage ("GIF_CompressImage error %d\n", r);
  r = GIF_Close ();
  LogMessage ("GIF_Close returned %d\n", r);
}


static const char ColourCode[256] =
{
   0, 0, 0, 0,15, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,

   0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0,  7, 0, 0, 0, 0, 0, 0, 0,
   8, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
  10, 0, 0, 0, 0, 0, 2, 0,  0, 0, 0, 0, 0, 0, 0, 0,

   0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
   6, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
  12, 0, 0, 0, 0, 0, 0, 4,  0, 0, 0, 0, 0, 0, 3, 0,

   0,11, 0, 0, 0, 9, 0, 0,  0, 0, 1, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0,14,  0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0,  0, 0,13, 0, 0, 5, 0, 0
};

static int ShotGetPixel (int x, int y)
{
  // FIXME: I assume we're in 256 colours
  return ColourCode[getpixel (x, y)];
}
#endif

