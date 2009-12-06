/*
 *	wadres.h
 *	Wadres class
 *	AYM 2000-04-06
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


#ifndef YH_WADRES  /* DO NOT INSERT ANYTHING BEFORE THIS LINE */
#define YH_WADRES


#include "spritdir.h"


/* This class is mostly a collection of other class. It does not
   much more than neatly grouping all objects that have the same
   lifetime and dependencies. */
class Wad_res
{
  public :
    Wad_res (MDirPtr *md);
    Sprite_dir sprites;
    /* To be added here :
       - Lump_dir patches
       - Lump_dir flats
       - Lump_cache textures (TEXTURE[12])
       - Lump_cache pnames   (PNAMES)
       - Lump_cache palette  (PLAYPAL, first 768 bytes) */
  private :
};


extern Wad_res wad_res;


#endif  /* DO NOT ADD ANYTHING AFTER THIS LINE */
