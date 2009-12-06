/*
 *	colour3.cc
 *	irgb2rgb()
 *	AYM 1998-06-28
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
#include "rgb.h"


/*
 *	irgb2rgb
 *	Convert an IRGB colour (16-colour VGA) to an 8-bit-per-component
 *	RGB colour.
 */
void irgb2rgb (int c, rgb_c *rgb)
{
  if (c == 8)  // Special case for DARKGREY
    rgb->r = rgb->g = rgb->b = 0x40;
  else
  {
    rgb->r = (c & 4) ? ((c & 8) ? 0xff : 0x80) : 0;
    rgb->g = (c & 2) ? ((c & 8) ? 0xff : 0x80) : 0;
    rgb->b = (c & 1) ? ((c & 8) ? 0xff : 0x80) : 0;
  }
}

