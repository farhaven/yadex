/*
 *	vectext.cc - crude scalable text
 *	AYM 2000-07-11
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
#include "gfx.h"
#include "vectext.h"


static const signed char vmin[] =
{
  16, 
  'M',   0,  20, 
  'd',  16,   0, 
};

static const signed char v0[] =
{
  24, 
  'M',   8,   0, 
  'd',   8,   0, 
  'd',   8,  12, 
  'd',   0,  24, 
  'd',  -8,  12, 
  'd',  -8,   0, 
  'd',  -8, -12, 
  'd',   0, -24, 
  'd',   8, -12, 
};

static const signed char v1[] =
{
  12, 
  'M',   0,   8, 
  'd',   8,  -8, 
  'd',   0,  49, 
};

static const signed char v2[] =
{
  26, 
  'M',   0,   4, 
  'd',   8,  -4, 
  'd',   8,   0, 
  'd',   8,   8, 
  'd',   0,   8, 
  'd', -24,  32, 
  'd',  26,   0, 
};

static const signed char v3[] =
{
  24, 
  'M',   0,   4, 
  'd',   8,  -4, 
  'd',   8,   0, 
  'd',   8,   8, 
  'd',   0,   8, 
  'd',  -8,   8,
  'm',  -8,   0, 
  'd',   8,   0, 
  'd',   8,   8, 
  'd',   0,   8, 
  'd',  -8,   8, 
  'd',  -8,   0, 
  'd',  -8,  -4, 
};

static const signed char v4[] =
{
  24, 
  'M',  24,  32, 
  'd', -24,   0, 
  'd',  20, -32, 
  'd',   0,  48, 
};

static const signed char v5[] =
{
  28, 
  'M',  28,   0, 
  'd', -24,   0, 
  'd',  -4,  20, 
  'd',  20,   0, 
  'd',   8,   8, 
  'd',   0,  12, 
  'd',  -8,   8, 
  'd', -12,   0, 
  'd',  -8,  -4, 
};

static const signed char v6[] =
{
  28, 
  'M',  24,   0, 
  'd', -16,   0, 
  'd',  -8,  12, 
  'd',   0,  24, 
  'd',   8,  12, 
  'd',  12,   0, 
  'd',   8,  -8, 
  'd',   0,  -8, 
  'd',  -8,  -8, 
  'd', -20,   0, 
};

static const signed char v7[] =
{
  24, 
  'M',   0,   0, 
  'd',  24,   0, 
  'd', -20,  49, 
};

static const signed char v8[] =
{
  28, 
  'M',   8,  24, 
  'd',  -8,  -6, 
  'd',   0, -12, 
  'd',   8,  -6, 
  'd',  12,   0, 
  'd',   8,   6, 
  'd',   0,  12, 
  'd',  -8,   6, 
  'd', -12,   0, 
  'd',  -8,   6,
  'd',   0,  12, 
  'd',   8,   6, 
  'd',  12,   0, 
  'd',   8,  -6, 
  'd',   0, -12, 
  'd',  -8,  -6, 
};

static const signed char v9[] =
{
  28, 
  'M',  28,  24, 
  'd', -20,   0, 
  'd',  -8,  -8, 
  'd',   0,  -8, 
  'd',   8,  -8, 
  'd',  12,   0, 
  'd',   8,  12, 
  'd',   0,  24, 
  'd',  -8,  12, 
  'd', -16,   0, 
};



static inline void vdata (char c, const signed char *& p, size_t& s)
{
  switch (c)
  {
    case '-': p = vmin; s = sizeof vmin; break;
    case '0': p = v0;   s = sizeof v0;   break;
    case '1': p = v1;   s = sizeof v1;   break;
    case '2': p = v2;   s = sizeof v2;   break;
    case '3': p = v3;   s = sizeof v3;   break;
    case '4': p = v4;   s = sizeof v4;   break;
    case '5': p = v5;   s = sizeof v5;   break;
    case '6': p = v6;   s = sizeof v6;   break;
    case '7': p = v7;   s = sizeof v7;   break;
    case '8': p = v8;   s = sizeof v8;   break;
    case '9': p = v9;   s = sizeof v9;   break;
    default:  p = 0;    s = 0;           break;
  }
}


static inline int vwidth (char c)
{
  const signed char *d;
  size_t s;
  vdata (c, d, s);
  if (d == 0)
    return 0;
  return *d;
}


void draw_vint (int number, int x, int y, double scale)
{
  char buf[20];
  y_snprintf (buf, sizeof buf, "%d", number);
  draw_vstring (buf, x, y, scale);
}


void draw_vstring (const char *string, int x, int y, double scale)
{
  const int    height = 48;		// Height in units FIXME hard-coded
  const double ppu    = scale / 4;	// Pixels per unit

  // Calculate in advance the whole width of the text
  int width = 0;			// Width in units
  for (const char *s = string; *s; s++)
    width += vwidth (*s);
  
  int xref = (int) (x - width  * ppu / 2);
  int yref = (int) (y - height * ppu / 2);
  for (const char *s = string; *s; s++)
  {
    int xcur = xref;
    int ycur = yref;
    const signed char *d, *dend;
    size_t size;
    vdata (*s, d, size);
    if (d == 0 || size < 1)
      continue;
    dend = d + size;

    // Extract the width;
    int w = *d++;

    // Process the statements
    while ((d + 2) < dend)
    {
      if (*d == 'd')  // d - draw to relative position
      {
	d++;
	int xofs = *d++;
	int yofs = *d++;
	DrawScreenLine (
	   (int) (xref + xcur          * ppu + 0.5),
	   (int) (yref + ycur          * ppu + 0.5),
	   (int) (xref + (xcur + xofs) * ppu + 0.5),
	   (int) (yref + (ycur + yofs) * ppu + 0.5));
	xcur += xofs;
	ycur += yofs;
      }
      else if (*d == 'm')  // m - move the cursor to relative position
      {
	d++;
	xcur += *d++;
	ycur += *d++;
      }
      else if (*d == 'M')  // M - move the cursor to absolute position
      {
	d++;
	xcur = *d++;
	ycur = *d++;
      }
      else
      {
	nf_bug ("bad statement \"%c\"", *d);
	d += 3;
	continue;
      }
    }
    xref += (int) ((w + 8) * ppu + 0.5);
  }
}

