/*
 *	drawmap.cc
 *	AYM 1998-09-06
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
#include <math.h>
#include <algorithm>
#include <map>
#include <vector>
#include <X11/Xlib.h>
#include "_edit.h"
#include "disppic.h"  /* Sprites */
#include "drawmap.h"
#include "game.h"     /* Sprites */
#include "gfx.h"
#include "imgscale.h"
#include "imgspect.h"
#include "levels.h"
#include "lists.h"
#include "pic2img.h"
#include "s_centre.h"
#include "sticker.h"
#include "things.h"
#include "vectext.h"
#include "wadres.h"


static void draw_grid (edit_t *e);
static void draw_vertices (edit_t *e);
static void draw_linedefs (edit_t *e);
static void draw_things_squares (edit_t *e);
static void draw_things_sprites (edit_t *e);
static void draw_obj_no (int x, int y, int obj_no, acolour_t c);


/*
 *	vertex_radius - apparent radius of a vertex, in pixels
 *
 *	Try this in Gnuplot :
 *
 *	  plot [0:10] x                                          
 *	  replot log(x+1.46)/log(1.5)-log(2.46)/log(1.5)+1
 */
int vertex_radius (double scale)
{
#if 0
  static double last_scale = 0;
  static int    last_result = 0;

  if (scale == last_scale)
    return last_result;

  const int    VERTEX_PIXELS  = 5;

  // The scale past which we switch from linear to logarithmic.
  const double crossover      = 0.1;

  // The base of the log. The higher, the stronger the effect.
  const double base           = 1.4;

  /* The point at which the derivative of log{base}(x) is 1.
     This is where we want the crossover to occur. */
  const double knee_x         = 1 / log (base);
  const double knee_y         = log (knee_x) / log (base);

  double factor;
  if (scale <= crossover)
    factor = scale;
  else
    factor = crossover + log (scale -crossover + knee_x) / log (base) - knee_y;
  last_result = (int) (VERTEX_PIXELS * factor + 0.5);
  return last_result;
#else
  const int VERTEX_PIXELS = 6;
  return (int) (VERTEX_PIXELS * (0.2 + scale / 2));
#endif
}


/*
  draw the actual game map
*/

void draw_map (edit_t *e) /* SWAP! */
{
  int mapx0 = MAPX (0);
  int mapx9 = MAPX (ScrMaxX);
  int mapy0 = MAPY (ScrMaxY);
  int mapy9 = MAPY (0);
  int n;


  // Draw the grid first since it's in the background
  draw_grid (e);

  if (e->global)
  {
    draw_linedefs (e);
    if (e->show_things_sprites)
      draw_things_sprites (e);
    else
      draw_things_squares (e);
    draw_vertices (e);
  }
  else
  {
    if (e->obj_type != OBJ_THINGS)
      draw_things_squares (e);
    draw_linedefs (e);
    if (e->obj_type == OBJ_VERTICES)
      draw_vertices (e);
    if (e->obj_type == OBJ_THINGS)
    {
      if (e->show_things_sprites)
	draw_things_sprites (e);
      else
	draw_things_squares (e);
    }
  }


  // Draw the things numbers
  if (e->obj_type == OBJ_THINGS && e->show_object_numbers)
  {
    for (n = 0; n < NumThings; n++)
    {
      int mapx = Things[n].xpos;
      int mapy = Things[n].ypos;
      if (mapx < mapx0 || mapx > mapx9 || mapy < mapy0 || mapy > mapy9)
	continue;
      draw_obj_no (SCREENX (mapx) + FONTW, SCREENY (mapy) + 2, n, THING_NO);
    }
  }

  // Draw the sector numbers
  if (e->obj_type == OBJ_SECTORS && e->show_object_numbers)
  {
    int xoffset = - FONTW / 2;

    for (n = 0; n < NumSectors; n++)
    {
      int mapx;
      int mapy;
      centre_of_sector (n, &mapx, &mapy);
      if (mapx >= mapx0 && mapx <= mapx9 && mapy >= mapy0 && mapy <= mapy9)
	draw_obj_no (SCREENX (mapx) + xoffset, SCREENY (mapy) - FONTH / 2, n,
	  SECTOR_NO);
      if (n == 10 || n == 100 || n == 1000 || n == 10000)
	xoffset -= FONTW / 2;
    }
  }
}


/*
 *	draw_grid - draw the grid in the background of the edit window
 */
static void draw_grid (edit_t *e)
{
  if (! e->grid_shown)
    return;
  
  int mapx0   = MAPX (0);
  int mapx1   = MAPX (ScrMaxX);
  int mapy0   = MAPY (ScrMaxY);
  int mapy1   = MAPY (0);

  int grid_step_1 = e->grid_step;	// Map units between dots
  int grid_step_2 = 4 * grid_step_1;	// Map units between dim lines
  int grid_step_3 = 4 * grid_step_2;	// Map units between bright lines
  int grid_step_4 = 4 * grid_step_3;	// Map units between brighter lines

  {
    set_colour (GRID2V);
    int mapx0_2 = (mapx0 / grid_step_2) * grid_step_2;
    if (mapx0_2 < mapx0)
      mapx0_2 += grid_step_2;
    for (int i = mapx0_2; i <= mapx1; i += grid_step_2)
      if (i % grid_step_3 != 0)
	DrawMapLine (i, mapy0, i, mapy1);
  }

  {
    set_colour (GRID2H);
    int mapy0_2 = (mapy0 / grid_step_2) * grid_step_2;
    if (mapy0_2 < mapy0)
      mapy0_2 += grid_step_2;
    for (int j = mapy0_2; j <=  mapy1; j += grid_step_2)
      if (j % grid_step_3 != 0)
	DrawMapLine (mapx0, j, mapx1, j);
  }

  {
    set_colour (GRID3V);
    int mapx0_3 = (mapx0 / grid_step_3) * grid_step_3;
    if (mapx0_3 < mapx0)
      mapx0_3 += grid_step_3;
    for (int i = mapx0_3; i <= mapx1; i += grid_step_3)
      if (i % grid_step_4 != 0)
	DrawMapLine (i, mapy0, i, mapy1);
  }

  {
    set_colour (GRID3H);
    int mapy0_3 = (mapy0 / grid_step_3) * grid_step_3;
    if (mapy0_3 < mapy0)
      mapy0_3 += grid_step_3;
    for (int j = mapy0_3; j <=  mapy1; j += grid_step_3)
      if (j % grid_step_4 != 0)
	DrawMapLine (mapx0, j, mapx1, j);
  }

  {
    set_colour (GRID4V);
    int mapx0_4 = (mapx0 / grid_step_4) * grid_step_4;
    if (mapx0_4 < mapx0)
      mapx0_4 += grid_step_4;
    //printf ("MAPX(0): %d  mapx0_4: %d\n", MAPX(0), mapx0_4);  // DEBUG
    for (int i = mapx0_4; i <= mapx1; i += grid_step_4)
      DrawMapLine (i, mapy0, i, mapy1);
  }

  {
    set_colour (GRID4H);
    int mapy0_4 = (mapy0 / grid_step_4) * grid_step_4;
    if (mapy0_4 < mapy0)
      mapy0_4 += grid_step_4;
    for (int j = mapy0_4; j <=  mapy1; j += grid_step_4)
      DrawMapLine (mapx0, j, mapx1, j);
  }

  {
    int mapx0_1 = (mapx0 / grid_step_1) * grid_step_1;
    if (mapx0_1 < mapx0)
      mapx0_1 += grid_step_1;
    int mapy0_1 = (mapy0 / grid_step_1) * grid_step_1;
    if (mapy0_1 < mapy0)
      mapy0_1 += grid_step_1;

    // Optimisation for X: draw several points in one go
    int npoints = (mapx1 - mapx0_1) / grid_step_1 + 1;
    XPoint *points = (XPoint *) malloc (npoints * sizeof *points);
    points[0].x = SCREENX (mapx0_1);
    int n = 1;
    int last_i = points[0].x;
    for (int i = mapx0_1 + grid_step_1; i <= mapx1; i += grid_step_1)
    {
      if (n >= npoints)
	nf_bug ("%d >= %d", n, npoints);
      points[n].x = SCREENX (i) - last_i;
      points[n].y = 0;
      n++;
      last_i = SCREENX (i);
    }
    npoints = n;
    set_colour (GRID1);
    for (int j = mapy0_1; j <= mapy1; j += grid_step_1)
    {
      points[0].y = SCREENY (j);
      XDrawPoints (dpy, drw, gc, points, npoints, CoordModePrevious);
    }
    free (points);
  }
}


/*
 *	draw_vertices - draw the vertices, and possibly their numbers
 */
static void draw_vertices (edit_t *e)
{
  int mapx0 = MAPX (0);
  int mapx9 = MAPX (ScrMaxX);
  int mapy0 = MAPY (ScrMaxY);
  int mapy9 = MAPY (0);
  const int r = vertex_radius (Scale);

  push_colour (LIGHTGREEN);
  for (int n = 0; n < NumVertices; n++)
  {
    int mapx = Vertices[n].x;
    int mapy = Vertices[n].y;
    if (mapx >= mapx0 && mapx <= mapx9 && mapy >= mapy0 && mapy <= mapy9)
    {
      register int scrx = SCREENX (mapx);
      register int scry = SCREENY (mapy);
      DrawScreenLine (scrx - r, scry - r, scrx + r, scry + r);
      DrawScreenLine (scrx + r, scry - r, scrx - r, scry + r);
    }
  }
  if (e->show_object_numbers)
  {
    for (int n = 0; n < NumVertices; n++)
    {
      int mapx = Vertices[n].x;
      int mapy = Vertices[n].y;
      if (mapx >= mapx0 && mapx <= mapx9 && mapy >= mapy0 && mapy <= mapy9)
      {
	int x = (int) (SCREENX (mapx) + 2 * r);
	int y = SCREENY (mapy) + 2;
	draw_obj_no (x, y, n, VERTEX_NO);
      }
    }
  }
  pop_colour ();
}


/*
 *	draw_linedefs - draw the linedefs
 */
static void draw_linedefs (edit_t *e)
{
  int mapx0 = MAPX (0);
  int mapx9 = MAPX (ScrMaxX);
  int mapy0 = MAPY (ScrMaxY);
  int mapy9 = MAPY (0);

  switch (e->obj_type)
  {
    case OBJ_THINGS:
    {
      int current_colour = INT_MIN;  /* Some impossible colour no. */
      int new_colour;
      
      ObjectsNeeded (OBJ_LINEDEFS, OBJ_VERTICES, 0);
      for (int n = 0; n < NumLineDefs; n++)
      {
	register int x1 = Vertices[LineDefs[n].start].x;
	register int x2 = Vertices[LineDefs[n].end  ].x;
	register int y1 = Vertices[LineDefs[n].start].y;
	register int y2 = Vertices[LineDefs[n].end  ].y;
	if (x1 < mapx0 && x2 < mapx0
	 || x1 > mapx9 && x2 > mapx9
	 || y1 < mapy0 && y2 < mapy0
	 || y1 > mapy9 && y2 > mapy9)
	  continue;
	if (LineDefs[n].flags & 1)
	  new_colour = WHITE;
	else
	  new_colour = LIGHTGREY;
	if (new_colour != current_colour)
	  set_colour (current_colour = new_colour);
	DrawMapLine (x1, y1, x2, y2);
      }
      break;
    }

    case OBJ_VERTICES:
      ObjectsNeeded (OBJ_LINEDEFS, OBJ_VERTICES, 0);
      set_colour (LIGHTGREY);
      for (int n = 0; n < NumLineDefs; n++)
      {
	register int x1 = Vertices[LineDefs[n].start].x;
	register int x2 = Vertices[LineDefs[n].end  ].x;
	register int y1 = Vertices[LineDefs[n].start].y;
	register int y2 = Vertices[LineDefs[n].end  ].y;
	if (x1 < mapx0 && x2 < mapx0
	 || x1 > mapx9 && x2 > mapx9
	 || y1 < mapy0 && y2 < mapy0
	 || y1 > mapy9 && y2 > mapy9)
	  continue;
	DrawMapVector (x1, y1, x2, y2);
      }
      break;

    case OBJ_LINEDEFS:
    {
      int current_colour = INT_MIN;  /* Some impossible colour no. */
      int new_colour;

      ObjectsNeeded (OBJ_LINEDEFS, OBJ_VERTICES, 0);
      for (int n = 0; n < NumLineDefs; n++)
      {
	register int x1 = Vertices[LineDefs[n].start].x;
	register int x2 = Vertices[LineDefs[n].end  ].x;
	register int y1 = Vertices[LineDefs[n].start].y;
	register int y2 = Vertices[LineDefs[n].end  ].y;
	if (x1 < mapx0 && x2 < mapx0
	 || x1 > mapx9 && x2 > mapx9
	 || y1 < mapy0 && y2 < mapy0
	 || y1 > mapy9 && y2 > mapy9)
	  continue;
	if (LineDefs[n].type != 0)  /* AYM 19980207: was "> 0" */
	{
	  if (LineDefs[n].tag != 0)  /* AYM 19980207: was "> 0" */
	    new_colour = LIGHTMAGENTA;
	  else
	    new_colour = LIGHTGREEN;
	}
	else if (LineDefs[n].flags & 1)
	  new_colour = WHITE;
	else
	  new_colour = LIGHTGREY;

	// Signal errors by drawing the linedef in red. Needs work.
	// Tag on a typeless linedef
	if (LineDefs[n].type == 0 && LineDefs[n].tag != 0)
	  new_colour = LIGHTRED;
	// No first sidedef
	if (! is_sidedef (LineDefs[n].sidedef1))
	  new_colour = LIGHTRED;
	// Bad second sidedef
	if (! is_sidedef (LineDefs[n].sidedef2) && LineDefs[n].sidedef2 != -1)
	  new_colour = LIGHTRED;

	if (new_colour != current_colour)
	  set_colour (current_colour = new_colour);
	DrawMapLine (x1, y1, x2, y2);

	if (e->show_object_numbers)
	{
	  int scnx0       = SCREENX (x1);
	  int scnx1       = SCREENX (x2);
	  int scny0       = SCREENY (y1);
	  int scny1       = SCREENY (y2);
	  int label_width = ((int) log10 (n) + 1) * FONTW;
	  if (abs (scnx1 - scnx0) > label_width + 4
	   || abs (scny1 - scny0) > label_width + 4)
	  {
	    int scnx = (scnx0 + scnx1) / 2 - label_width / 2;
	    int scny = (scny0 + scny1) / 2 - FONTH / 2;
	    draw_obj_no (scnx, scny, n, LINEDEF_NO);
	  }
	}
      }
      break;
    }

    case OBJ_SECTORS:
    {
      int current_colour = INT_MIN;  /* Some impossible colour no. */
      int new_colour;

      for (int n = 0; n < NumLineDefs; n++)
      {
	register int x1 = Vertices[LineDefs[n].start].x;
	register int x2 = Vertices[LineDefs[n].end  ].x;
	register int y1 = Vertices[LineDefs[n].start].y;
	register int y2 = Vertices[LineDefs[n].end  ].y;
	if (x1 < mapx0 && x2 < mapx0
	 || x1 > mapx9 && x2 > mapx9
	 || y1 < mapy0 && y2 < mapy0
	 || y1 > mapy9 && y2 > mapy9)
	  continue;
	int sd1 = OBJ_NO_NONE;
	int sd2 = OBJ_NO_NONE;
	int s1  = OBJ_NO_NONE;
	int s2  = OBJ_NO_NONE;
	// FIXME should flag negative sidedef numbers as errors
	// FIXME should flag unused tag as errors
	if ((sd1 = LineDefs[n].sidedef1) < 0 || sd1 >= NumSideDefs
	  || (s1 = SideDefs[sd1].sector) < 0 || s1 >= NumSectors
	  || (sd2 = LineDefs[n].sidedef2) >= NumSideDefs
	  || sd2 >= 0 && ((s2 = SideDefs[sd2].sector) < 0
			|| s2 >= NumSectors))
	{
	  new_colour = LIGHTRED;
	}
	else
	{
	  bool have_tag  = false;
	  bool have_type = false;
	  if (Sectors[s1].tag != 0)
	    have_tag = true;
	  if (Sectors[s1].special != 0)
	    have_type = true;
	  if (sd2 >= 0)
	  {
	    if (Sectors[s2].tag != 0)
	      have_tag = true;
	    if (Sectors[s2].special != 0)
	      have_type = true;
	  }
	  if (have_tag && have_type)
	    new_colour = SECTOR_TAGTYPE;
	  else if (have_tag)
	    new_colour = SECTOR_TAG;
	  else if (have_type)
	    new_colour = SECTOR_TYPE;
	  else if (LineDefs[n].flags & 1)
	    new_colour = WHITE;
	  else
	    new_colour = LIGHTGREY;
	}
	if (new_colour != current_colour)
	  set_colour (current_colour = new_colour);
	DrawMapLine (x1, y1, x2, y2);
      }
      break;
    }
  }
}


/*
 *	draw_things_squares - the obvious
 */
static void draw_things_squares (edit_t *e)
{
  // The radius of the largest thing.
  int max_radius = get_max_thing_radius ();

  /* A thing is guaranteed to be totally off-screen
     if its centre is more than <max_radius> units
     beyond the edge of the screen. */
  int mapx0      = MAPX (0)       - max_radius;
  int mapx9      = MAPX (ScrMaxX) + max_radius;
  int mapy0      = MAPY (ScrMaxY) - max_radius;
  int mapy9      = MAPY (0)       + max_radius;

  push_colour (THING_REM);
  for (int n = 0; n < NumThings; n++)
  {
    int mapx = Things[n].xpos;
    int mapy = Things[n].ypos;
    int corner_x;
    int corner_y;
    if (mapx < mapx0 || mapx > mapx9 || mapy < mapy0 || mapy > mapy9)
      continue;
    int m = get_thing_radius (Things[n].type);
    if (e->obj_type == OBJ_THINGS)
      set_colour (get_thing_colour (Things[n].type));
#ifdef ROUND_THINGS
    DrawMapLine (mapx - m, mapy,     mapx + m, mapy    );
    DrawMapLine (mapx,     mapy - m, mapx,     mapy + m);
    DrawMapCircle (mapx, mapy, m);
#else
    DrawMapLine (mapx - m, mapy - m, mapx + m, mapy - m);
    DrawMapLine (mapx + m, mapy - m, mapx + m, mapy + m);
    DrawMapLine (mapx + m, mapy + m, mapx - m, mapy + m);
    DrawMapLine (mapx - m, mapy + m, mapx - m, mapy - m);
#endif
    {
      size_t direction = angle_to_direction (Things[n].angle);
      static const short xsign[] = {  1,  1,  0, -1, -1, -1,  0,  1,  0 };
      static const short ysign[] = {  0,  1,  1,  1,  0, -1, -1, -1,  0 };
      corner_x = m * xsign[direction];
      corner_y = m * ysign[direction];
    }
    DrawMapLine (mapx, mapy, mapx + corner_x, mapy + corner_y);
  }
  pop_colour ();
}


/* Drawing the things sprites is done here.

   To avoid having large sprites obscure small ones, we display
   the large sprites first and the small ones last. To do that,
   we maintain a list of all things in the level, sorted in
   descending number of opaque pixels in the sprite. Actually,
   we approximate that by the size of the lump. Usually, the
   size of the lump is roughly monotonic w.r.t. the number of
   opaque pixels. And it's much simpler and faster than counting
   the pixels.

   That list serves a second purpose : optimization. Because of
   the way it's sorted, two things that have the same graphic
   representation are always contiguous in the list. That
   property allows us to save quite a few calls to
   LoadPicture(), scale_img(), spectrify_img() and
   Sticker::load(). Given that those are very expensive
   operations and that the average level contains many
   repetitions of certain things (E.G. former humans), the
   benefit is considerable. On certain semi-pathological levels
   like Robin Holden's court30.wad, it makes display several
   times faster.

   We approximate "two things have the same graphic
   represention" by "two things have the same type". Strictly
   speaking, it's not the same thing. Two distinct thing types
   could very well have the same graphic representation. In fact
   it does happen (cf. things 49 and 63). However, since the
   alternative is to sort based on the quintuplet (wad, lump
   offset, flags, dye), we're better off this way. */


class Thing_npixels
{
  public :
    Thing_npixels (i16 thing_no, unsigned long npixels, wad_ttype_t type)
      : thing_no (thing_no), npixels (npixels), type (type) { }
    bool operator< (const Thing_npixels& other) const
      { if (this->npixels > other.npixels  // Decreasing npixels major
	    || this->npixels == other.npixels  // Increasing type minor
	       && this->type < other.type)
	  return true;
	return false;
      }
    i16 thing_no;
    unsigned long npixels;
    wad_ttype_t type;
};


class Thing_list_by_size
{
  public :
    Thing_list_by_size () { }
    ~Thing_list_by_size () { }
    const Thing_npixels& operator[] (int n) { return a[n]; }
    void refresh ()
    {
      a.clear ();
      a.reserve (NumThings);
      for (int n = 0; n < NumThings; n++)
      {
	Lump_loc loc;
	const char *sprite_root = get_thing_sprite (Things[n].type);
	if (sprite_root != NULL)
	  wad_res.sprites.loc_by_root (sprite_root, loc);
	else
	  loc.len = 0;
	a.push_back (Thing_npixels ((i16) n, loc.len, Things[n].type));
      }
      sort (a.begin (), a.end ());
    }
  private :
    std::vector <Thing_npixels> a;
};


static Thing_list_by_size list;
//static unsigned long things_angles_prev;  // Unused for now
static unsigned long things_types_prev;


/* This map is used to cache widths and heights. We need them
   to skip off-screen sprites. */

struct sprite_dim_t
{
  sprite_dim_t () { }
  sprite_dim_t (int width, int height) : width (width), height (height) { }
  unsigned short width;
  unsigned short height;
};

typedef std::map <i16, sprite_dim_t> dim_map_t;
static dim_map_t dim_map;  // FIXME there should be one for each game


/*
 *	draw_things_sprites - the obvious
 */
static void draw_things_sprites (edit_t *e)
{
#ifdef NO_RENDER
  static
#endif
  Sticker      sticker;
  wad_ttype_t last_type = -1;  // Type of last thing displayed
  dim_map_t::iterator dim = dim_map.end ();
  bool set_dim = true;  // Init to avoid warning
  const unsigned short max_width  = 1000;
  const unsigned short max_height = 1000;
  int mapx0 = 0;
  int mapx9 = 0;
  int mapy0 = 0;
  int mapy9 = 0;

  if (things_types_prev != things_types)
  {
    list.refresh ();
    things_types_prev = things_types;
  }

#ifdef NO_RENDER
  static double last_scale = 0;
  if (last_scale != Scale)
  {
    Lump_loc loc;
    wad_res.sprites.loc_by_root ("PLAY", loc);
    Img img;
    LoadPicture (img, "PLAYA0", loc, 0, 0);
    Img img_scaled;
    scale_img (img, Scale * sprite_scale / 100, img_scaled);
    sprite.load (img_scaled, false);
    last_scale = Scale;
  }
#endif
  push_colour (CYAN);
  for (int n = 0; n < NumThings; n++)
  {
    const Thing_npixels& t = list[n];

    // Skip off-screen things
    if (t.type != last_type)
    {
      dim = dim_map.find (t.type);
      if (dim == dim_map.end ())
      {
	set_dim = true;
	mapx0 = MAPX (0)       - max_width / 2;
	mapx9 = MAPX (ScrMaxX) + max_width / 2;
	mapy0 = MAPY (ScrMaxY) - max_height / 2;
	mapy9 = MAPY (0)       + max_height / 2;
      }
      else
      {
	mapx0 = MAPX (0)       - dim->second.width / 2;
	mapx9 = MAPX (ScrMaxX) + dim->second.width / 2;
	mapy0 = MAPY (ScrMaxY) - dim->second.height / 2;
	mapy9 = MAPY (0)       + dim->second.height / 2;
      }
    }
    int mapx = Things[t.thing_no].xpos;
    int mapy = Things[t.thing_no].ypos;
    if (mapx < mapx0 || mapx > mapx9 || mapy < mapy0 || mapy > mapy9)
      continue;

#ifndef NO_RENDER
    // If not the same as the last thing displayed, rasterize it
    if (t.type != last_type)
    {
      last_type = t.type;

      const char *sprite_root = get_thing_sprite (t.type);
      if (sprite_root != NULL)
      {
	Lump_loc loc;
	wad_res.sprites.loc_by_root (sprite_root, loc);
	Img img_raw, img_scaled;
	if (LoadPicture (img_raw, sprite_root, loc, 0, 0))
	{
	  sticker.clear ();  // We'll display the thing type instead
	}
	else
	{
	  if (set_dim)
	  {
	    dim_map[t.type] = sprite_dim_t (img_raw.width(), img_raw.height());
	    set_dim = false;
	  }
	  scale_img (img_raw, Scale * sprite_scale / 100 * get_thing_scale(t.type), img_scaled);
	  // printf("%f\n",get_thing_scale(t.type));
	  if (get_thing_flags (t.type) & THINGDEF_SPECTRAL)
	     spectrify_img (img_scaled);
	  sticker.load (img_scaled, false);
	}
      }
      else
	sticker.clear ();  // We'll display the thing type instead
    }
#endif
    
    // Display it
    if (sticker.is_clear ())
    {	draw_vint (t.type, SCREENX (mapx), SCREENY (mapy), Scale * get_thing_scale(t.type));
	// printf("%s ",(char *) Scale);
    }
    else
      sticker.draw (drw, 'c', SCREENX (mapx), SCREENY (mapy));
  }
  pop_colour ();
}


/*
 *	draw_obj_no - draw a number at screen coordinates (x, y)
 *
 *	FIXME too slow.
 */
static void draw_obj_no (int x, int y, int obj_no, acolour_t c)
{
  push_colour (BLACK);
#if 1
  DrawScreenText (x - 2, y,     "%d", obj_no);
  DrawScreenText (x - 1, y,     "%d", obj_no);
  DrawScreenText (x + 1, y,     "%d", obj_no);
  DrawScreenText (x + 2, y,     "%d", obj_no);
  DrawScreenText (x,     y + 1, "%d", obj_no);
  DrawScreenText (x,     y - 1, "%d", obj_no);
#else
  DrawScreenText (x + 1, y + 1, "%d", obj_no);
  DrawScreenText (x + 1, y - 1, "%d", obj_no);
  DrawScreenText (x - 1, y + 1, "%d", obj_no);
  DrawScreenText (x - 1, y - 1, "%d", obj_no);
#endif
  set_colour (c);
  DrawScreenText (x,     y,     "%d", obj_no);
  pop_colour ();
}

