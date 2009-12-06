/*
 *	disppic.cc
 *	AYM 1998-08-30
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
#ifdef Y_X11
#include <X11/Xlib.h>
#endif
#include "gfx.h"
#include "imgspect.h"
#include "lists.h"
#include "patchdir.h"
#include "pic2img.h"
#include "sticker.h"
#include "wadres.h"


/*
 *	display_pic() -	load a picture and display it
 *
 *	A wrapper for LoadPicture() and Sticker.load()/draw().
 */
void display_pic (hookfunc_comm_t *c)
{
  int width  = c->x1 - c->x0 + 1;
  int height = c->y1 - c->y0 + 1;
  Lump_loc loc;

  c->img.clear ();
  c->img.set_opaque (false);
  if (c->flags & HOOK_PATCH)
    patch_dir.loc_by_name (c->name, loc);
  else if ((c->flags & HOOK_SPRITE) && (c->flags & HOOK_ROOT))
    wad_res.sprites.loc_by_root (c->name, loc);
  else if ((c->flags & HOOK_SPRITE) && ! (c->flags & HOOK_ROOT))
    wad_res.sprites.loc_by_name (c->name, loc);
  if (! LoadPicture (c->img,
    c->name,
    loc,
    0, // Not very clean, should use c->xofs but *WithFunc doesn't set it
    0, // Not very clean, should use c->yofs but *WithFunc doesn't set it
    &c->width,
    &c->height))
  {
    c->disp_x0 = c->x0 + (width - c->width) / 2;
    c->disp_y0 = c->y0 + (height - c->height) / 2;
    c->disp_x1 = c->disp_x0 + c->width - 1;
    c->disp_y1 = c->disp_y0 + c->height - 1;
    c->flags |= HOOK_SIZE_VALID;
    c->lump_loc = loc;
    c->flags |= HOOK_LOC_VALID;
    if (c->flags & HOOK_SPECTRAL)
      spectrify_img (c->img);
    Sticker sticker (c->img, true);  // Use opaque because it's faster
    sticker.draw (drw, 't', c->disp_x0, c->disp_y0);
    c->flags |= HOOK_DRAWN;
  }
  else
  {
    // Drew nothing (negative width and height)
    c->disp_x0 = c->x0 + 1;
    c->disp_y0 = c->y0 + 1;
    c->disp_x1 = c->x0;
    c->disp_y1 = c->y0;
  }
}


