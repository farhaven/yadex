/*
 *	gfx3.cc
 *	Graphics routines
 *	AYM 1999-06-06
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
#include "colour.h"
#include "gfx.h"
#include "gfx3.h"
#include "rgbbmp.h"


/*
 *	window_to_rgbbmp
 *	Grab a rectangle from the window or screen into an
 *	Rgbbmp, in a portable fashion.
 */
void window_to_rgbbmp (int x, int y, int width, int height, Rgbbmp &b)
{
#if defined Y_X11
  b.resize (width, height);
  // FIXME
  for (int y = 0; y < b.height (); y++)
    for (int x = 0; x < b.width (); x++)
      b.set_r (x, y, 255 * (b.height () - y) / b.height ());
  for (int y = 0; y < b.height (); y++)
    for (int x = 0; x < b.width (); x++)
      b.set_g (x, y, 255 * (b.width () - x) / b.width ());
  for (int y = 0; y < b.height (); y++)
    for (int x = 0; x < b.width (); x++)
      b.set_b (x, y, 255 * (x + y) / (b.width () + b.height ()));
#elif defined Y_BGI
  printf ("window_to_rgb: unimplemented\n");
  return 0;
#endif
}


/*
 *	rgbbmp_to_rawppm
 *	Return 0 on success, non-zero on failure.
 */
int rgbbmp_to_rawppm (const Rgbbmp &b, const char *file_name)
{
  FILE *fd;
  fd = fopen (file_name, "wb");
  if (fd == 0)
  {
    fflush (stdout);
    fprintf (stderr, "Can't open \"%s\" for writing (%s)\n",
	file_name, strerror (errno));
    fflush (stderr);
    return 1;
  }
  fprintf (fd,	"P6\n"
		"# Created by Yadex %s\n"
		"%d %d\n"
		"255\n", yadex_version, b.width (), b.height ());
  for (int y = 0; y < b.height (); y++)
    for (int x = 0; x < b.width (); x++)
    {
      putc (b.get_r (x, y), fd);
      putc (b.get_g (x, y), fd);
      putc (b.get_b (x, y), fd);
    }
  if (fclose (fd))
  {
    fflush (stdout);
    fprintf (stderr, "Write error in \"%s\"\n", file_name);
    fflush (stderr);
    return 1;
  }
  return 0;
}


