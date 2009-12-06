/*
 *	bench.cc
 *	Benchmark functions.
 *	AYM 2000-04-13
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

#include <time.h>
#include <sys/times.h>

#include "gfx.h"
#include "img.h"
#include "pic2img.h"
#include "wadres.h"



static void bench_LoadPicture ();


/*
 *	benchmark - run the benchmarks specified by <what>.
 */
void benchmark (const char *what)
{
  // Insert parsing of <what> here
  if (! strcmp (what, "loadpic"))
    bench_LoadPicture ();
}


/*
 *	bench_LoadPicture - run a benchmark of LoadPicture()
 */
static void bench_LoadPicture ()
{
  const char *sprite_name = "TROOA1";
  unsigned long iterations = 100000;
  const int width = 100;
  const int height = 100;
  Lump_loc sprite_loc;
  
  wad_res.sprites.loc_by_name (sprite_name, sprite_loc);
  if (sprite_loc.wad == 0)
    fprintf (stderr, "Could not locate sprite %s\n", sprite_name);

  Img img (width, height, false);
  struct tms t0;
  times (&t0);
  for (unsigned long n = 0; n < iterations; n++)
    LoadPicture (img, sprite_name, sprite_loc, 0, 0, 0, 0);
  struct tms t1;
  times (&t1);

// Glibc 2.1 (?) has a broken CLOCKS_PER_SEC.
#ifndef CLOCKS_PER_SEC
#define CLOCKS_PER_SEC CLK_TCK
#endif
  const char *unit  = "s";
  double value = (double) (t1.tms_utime - t0.tms_utime)
		 / CLOCKS_PER_SEC / iterations;
  if (value < 1E-3)
  {
    unit = "µs";
    value *= 1000000;
  }
  else if (value < 1.0)
  {
    unit = "ms";
    value *= 1000;
  }
  printf ("LoadPicture: %f %s per call\n", value, unit);
}

