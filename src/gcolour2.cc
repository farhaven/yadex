/*
 *	gcolour2.cc
 *	The game colour# -> physical colour# conversion table.
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
#include "colour.h"
#include "gcolour2.h"


pcolour_t *game_colour = 0;	// Pixel values for the DOOM_COLOURS game clrs.
int colour0;			// Game colour to which g. colour 0 is remapped
int sky_colour;			// Game colour for a medium sky blue

