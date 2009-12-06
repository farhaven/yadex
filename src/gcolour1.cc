/*
 *	gcolour1.cc
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
#include "gcolour1.h"
#include "gcolour2.h"
#include "gfx.h"
#include "img.h"	/* IMG_TRANSP */
#include "rgb.h"
#include "wadfile.h"
#include "wads.h"


/*
 *	alloc_game_colours
 *	Allocate the DOOM_COLOURS of PLAYPAL no. <playpalnum>.
 *	Put the DOOM_COLOURS physical colour numbers corresponding
 *	to the game colours in the <game_colour> array.
 */
pcolour_t *alloc_game_colours (int playpalnum)
{
MDirPtr dir;
u8	*dpal;
pcolour_t *game_colours = 0;

dir = FindMasterDir (MasterDir, "PLAYPAL");
if (dir == NULL)
   {
   warn ("PLAYPAL lump not found.\n");
   return 0;
   }

int playpal_count = dir->dir.size / (3 * DOOM_COLOURS);
if (playpalnum < 0 || playpalnum >= playpal_count)
   {
   warn ("playpalnum %d out of range (0-%d). Using #0 instead.\n",
      playpalnum, playpal_count - 1);
   playpalnum = 0;
   }

dpal = (u8 *) GetFarMemory (3 * DOOM_COLOURS);
dir->wadfile->seek (dir->dir.start);
if (dir->wadfile->error ())
   {
   warn ("%s: can't seek to %lXh\n",
       dir->wadfile->pathname (), (unsigned long) dir->dir.start);
   warn ("PLAYPAL: seek error\n");
   }
for (int n = 0; n <= playpalnum; n++)
   {
   dir->wadfile->read_bytes (dpal, 3 * DOOM_COLOURS);
   if (dir->wadfile->error ())
      {
      warn ("%s: read error\n", dir->wadfile->where ());
      warn ("PLAYPAL: error reading entry #%d\n", n);
      }
   }
#if defined Y_BGI
for (int n = 0; n < 3 * DOOM_COLOURS; n++)
   dpal[n] /= 4;
_AX = 0x1012;
_BX = 0;
_CX = DOOM_COLOURS;
_ES = FP_SEG (dpal);
_DX = FP_OFF (dpal);
__int__ (0x10);
#elif defined Y_X11

rgb_c rgb_values[DOOM_COLOURS];
for (size_t n = 0; n < DOOM_COLOURS; n++)
   {
   rgb_values[n].r = (u8) dpal[3 * n];
   rgb_values[n].g = (u8) dpal[3 * n + 1];
   rgb_values[n].b = (u8) dpal[3 * n + 2];
   }
game_colours = alloc_colours (rgb_values, DOOM_COLOURS);

// Find the colour closest to IMG_TRANSP
{
  colour0 = IMG_TRANSP;
  int smallest_delta = INT_MAX;

  for (size_t n = 1; n < DOOM_COLOURS; n++)
  {
    int delta = rgb_values[IMG_TRANSP] - rgb_values[n];
    if (delta < smallest_delta)
    {
      colour0 = n;
      smallest_delta = delta;
    }
  }
  verbmsg ("colours: colour %d remapped to %d (delta %d)\n",
    IMG_TRANSP, colour0, smallest_delta);
   
   rgb_c med_blue (0, 0, 128);
   sky_colour = 0;
   smallest_delta = INT_MAX;
 
   for (size_t n = 0; n < DOOM_COLOURS; n++)
   {
     int delta = med_blue - rgb_values[n];
     if (delta < smallest_delta)
     {
       sky_colour = n;
       smallest_delta = delta;
     }
   }
   verbmsg ("Sky Colour remapped to %d (delta %d)\n", sky_colour, smallest_delta);
}

#endif
FreeFarMemory (dpal);
return game_colours;
}


/*
 *	free_game_colours
 *	Free the game colours allocated by alloc_game_colours()
 */
void free_game_colours (pcolour_t *game_colours)
{
free_colours (game_colours, DOOM_COLOURS);
}



/* This is how I used to calculate
   the physical colour numbers.
   Works only on TrueColor/DirectColor visuals. */

#if 0
/* FIXME this is a gross hack */
for (n = 0; n < DOOM_COLOURS; n++)
   {
   xpv_t r = dpal[3*n];
   xpv_t g = dpal[3*n+1];
   xpv_t b = dpal[3*n+2];
   if (win_vis_class == DirectColor || win_vis_class == TrueColor)
      {
      xpv_t r_scaled, g_scaled, b_scaled;
      if (win_r_ofs + win_r_bits < 8)
	 r_scaled = r >> (8 - (win_r_ofs + win_r_bits));
      else
	 r_scaled = r << (win_r_ofs + win_r_bits - 8) & win_r_mask;
      if (win_g_ofs + win_g_bits < 8)
	 g_scaled = g >> (8 - (win_g_ofs + win_g_bits));
      else
	 g_scaled = g << (win_g_ofs + win_g_bits - 8) & win_g_mask;
      if (win_b_ofs + win_b_bits < 8)
	 b_scaled = b >> (8 - (win_b_ofs + win_b_bits));
      else
	 b_scaled = b << (win_b_ofs + win_b_bits - 8) & win_b_mask;
      game_colour[n] = r_scaled | g_scaled | b_scaled;
      }
   else if (win_vis_class== PseudoColor || win_vis_class == StaticColor)
      game_colour[n] = n;  /* Ugh! */
   else if (win_vis_class == GrayScale || win_vis_class == StaticGray)
      {
      game_colour[n] = (r + g + b) / 3;
      if (win_depth < 8)
	 game_colour[n] >>= 8 - win_depth;
      else
	 game_colour[n] <<= win_depth - 8;
      }
   // printf ("%02X %08lX", n, (unsigned long) game_colour[n]);
   // if (n % 6 == 5)
   //    putchar ('\n');
   // else
   //    printf ("  ");
   }
#endif  /* #if 0 */
