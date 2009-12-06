/*
 *	spritdir.cc - Sprite_dir class
 *	AYM 2000-06-01
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
#include <functional>
#include "dependcy.h"
#include "lumpdir.h"
#include "spritdir.h"
#include "wadname.h"
#include "wadnamec.h"


/*
 *	Sprite_dir::loc_by_root - find sprite by prefix
 *	
 *      Return the (wad, offset, length) location of the first
 *      lump by alphabetical order whose name begins with
 *      <name>. If not found, set loc.wad to 0.
 */
void Sprite_dir::loc_by_root (const char *name, Lump_loc& loc)
{
  if (dependency->outdated ())
    refresh ();

  /* Caller asked for same lump twice in a row. Save us a second
     search. */
  if (have_prev && ! y_strnicmp (name, name_prev, WAD_NAME))
  {
    loc = loc_prev;
    return;
  }

  Lump_map::const_iterator i = lump_map.lower_bound (name);
  have_prev = true;
  if (i == lump_map.end () || y_strnicmp (name, i->first.name, strlen (name)))
    loc.wad = loc_prev.wad = 0;
  else
    loc = loc_prev = i->second;
}

