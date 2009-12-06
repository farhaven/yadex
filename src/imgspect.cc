/*
 *	imgspect.cc
 *	AYM 1999-03-09
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
#include "img.h"
#include "imgspect.h"


/*
 *	spectrify_img - make a game image look vaguely like a spectre
 */
void spectrify_img (Img& img)
{
int x,y;
u8 grey;

// FIXME this is gross
if (! strncmp (Game, "doom", 4))
  grey = 104;
else if (! strcmp (Game, "heretic"))
  grey = 8;
else if (! strcmp (Game, "zdoom"))
  grey = 100;
else
{
  nf_bug ("spectrifying not defined with this game");
  return;
}

img_dim_t width  = img.width ();
img_dim_t height = img.height ();
img_pixel_t *buf = img.wbuf ();
for (y = 0; y < height; y++)
   {
   img_pixel_t *row = buf + y * width;
   for (x = 0; x < width; x++)
      if (row[x] != IMG_TRANSP)
         row[x] = grey + (rand () >> 6) % 7;  // FIXME more kludgery
   }
}


