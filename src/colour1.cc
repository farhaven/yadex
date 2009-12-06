/*
 *	colour1.cc
 *	getcolour()
 *	AYM 1998-01-27
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


#define RGB_DIGITS 2  /* R, G and B are 8-bit wide each */


/*
 *	getcolour
 *	Decode an "rgb:<r>/<g>/<b>" colour specification
 *	Returns :
 *	  0    OK
 *	  <>0  malformed colour specification
 */
int getcolour (const char *s, rgb_c *rgb)
{
  int i;
  int digit;
  int rdigits;
  int gdigits;
  int bdigits;
  unsigned r;
  unsigned g;
  unsigned b;
  int globaldigits;

  if (strncmp (s, "rgb:", 4))
    return 1;

  for (i = 4, r = 0, rdigits = 0; (digit = hextoi (s[i])) >= 0; i++, rdigits++)
    r = (r << 4) | digit;
  if (s[i++] != '/')
    return 2;

  for (g = 0, gdigits = 0; (digit = hextoi (s[i])) >= 0; i++, gdigits++)
    g = (g << 4) | digit;
  if (s[i++] != '/')
    return 3;
    
  for (b = 0, bdigits = 0; (digit = hextoi (s[i])) >= 0; i++, bdigits++)
    b = (b << 4) | digit;
  if (s[i++] != '\0')
    return 4;

  // Force to 8 bits (RGB_DIGITS hex digits) by scaling up or down
  globaldigits = rdigits;
  globaldigits = y_max (globaldigits, gdigits);
  globaldigits = y_max (globaldigits, bdigits);
  for (; globaldigits < RGB_DIGITS; globaldigits++)
  {
    r <<= 4;
    g <<= 4;
    b <<= 4;
  }
  for (; globaldigits > RGB_DIGITS; globaldigits--)
  {
    r >>= 4;
    g >>= 4;
    b >>= 4;
  }
  rgb->set (r, g, b);
  return 0;
}


