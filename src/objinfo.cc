/*
 *	objinfo.cc
 *	AYM 1998-09-20
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

#include <iostream>

#include "yadex.h"
#include <vector>
#include <algorithm>
#include <X11/Xlib.h>
#include "disppic.h"
#include "flats.h"	// DisplayFloorTexture()
#include "game.h"	// THINGDEF_SPECTRAL
#include "gamesky.h"	// is_sky()
#include "gfx.h"
#include "img.h"
#include "imgspect.h"
#include "l_super.h"
#include "levels.h"
#include "names.h"
#include "objid.h"
#include "objinfo.h"
#include "pic2img.h"
#include "sticker.h"
#include "things.h"
#include "wadres.h"

using namespace std;

static const int sprite_width  = 90;
static const int sprite_height = 90;

/*
 *	Extraf - one item in the list of EDGE extrafloors
 */
class Extraf {
	public :
		Extraf (obj_no_t sector, string tex, wad_z_t height) {
			this->sector = sector;
			this->tex = tex;
			this->height = height;
		}

		bool operator< (const Extraf& other) const {
			if (height < other.height)
				return true;
			else if (height == other.height && sector < other.sector)
				return true;
			return false;
		}

		wad_z_t height;	// To sort by increasing floor height
		obj_no_t sector;	// Sector# (for heights, flats and light level)
		string tex;	// Texture (middle tex of first sidedef)
};

static void get_extrafloors (std::vector<Extraf>& list, wad_tag_t tag);

objinfo_c::objinfo_c () {
  for (size_t n = 0; n < MAX_BOXES; n++)
    box_disp[n] = false;
  obj_no      = OBJ_NO_NONE;
  obj_no_disp = OBJ_NO_NONE;
  prev_sector = OBJ_NO_NONE;
  out_y1      = 0;
}


void objinfo_c::draw () {
  int  n;
  int  sd1 = OBJ_NO_NONE;
  int  sd2 = OBJ_NO_NONE;
  int  s1 = OBJ_NO_NONE;
  int  s2 = OBJ_NO_NONE;
  int  x0, y0;		// Outer top left corner
  int  ix0, iy0;		// Inner top left corner
  int  width;
  int  height;

  // Am I already drawn ?
  if (! is_obj (obj_no) || (obj_no == obj_no_disp && obj_type == obj_type_disp))
    return;

  // Does the box need to be redrawn ?
  if (obj_type != obj_type_disp)
    box_disp[0] = false;

  // The caller should have called set_y1() before !
  if (! out_y1)
    return;

  switch (obj_type)
  {
    case OBJ_THINGS:
    {
      const int columns = 27;
      width  = 2 * BOX_BORDER + 3 * WIDE_HSPACING + columns * FONTW
	     + 2 * HOLLOW_BORDER + sprite_width;
  
      if (yg_level_format == YGLF_HEXEN)   
      {	height = 2 * BOX_BORDER + 2 * WIDE_VSPACING
	       + y_max ((int) (7 * FONTH), sprite_height);
      }else
      {	height = 2 * BOX_BORDER + 2 * WIDE_VSPACING
	       + y_max ((int) (6.5 * FONTH),sprite_height);
      }
      x0 = 0;
      y0 = out_y1 - height + 1;
      ix0 = x0 + BOX_BORDER + WIDE_HSPACING;
      iy0 = y0 + BOX_BORDER + WIDE_VSPACING;
      int ix1 = x0 + width - 1 - BOX_BORDER - WIDE_HSPACING;
      int iy1 = y0 + height - 1 - BOX_BORDER - WIDE_VSPACING;
      if (box_disp[0])
      {
	push_colour (WINBG);
	DrawScreenBox (ix0, iy0, ix0 + columns * FONTW - 1, iy1);
	pop_colour ();
      }
      else
	DrawScreenBox3D (x0, y0, x0 + width - 1, y0 + height - 1);
      if (obj_no < 0)
      {
	const char *message = "(no thing selected)";
	set_colour (WINFG_DIM);
	DrawScreenText (x0 + (width - FONTW * strlen (message)) / 2,
	   y0 + (height - FONTH) / 2, message);
	break;
      }
      set_colour (WINTITLE);
	DrawScreenText (ix0, iy0, "Thing #%d", obj_no);

      const bool invalid_type = ! is_thing_type (Things[obj_no].type);
      set_colour (WINFG);
      DrawScreenText (-1, iy0 + (int) (1.5 * FONTH), 0);
      if (yg_level_format != YGLF_HEXEN)
	DrawScreenText (-1, -1, "\1Coords:\2 (%d, %d)",	Things[obj_no].xpos, Things[obj_no].ypos);
      else
	DrawScreenText (-1, -1, "\1Coords:\2 (%d, %d, %d)", Things[obj_no].xpos, Things[obj_no].ypos, Things[obj_no].height);
      DrawScreenString (-1, -1, "\1Type:   ");
      if (invalid_type)
	push_colour (CLR_ERROR);
      DrawScreenText (-2, -2, "%d", Things[obj_no].type);
      if (invalid_type)
	pop_colour ();
      DrawScreenString (-1, -1, "\1Desc:   ");
      if (invalid_type)
	push_colour (CLR_ERROR);
      DrawScreenText (-2, -2, "%.19s", get_thing_name (Things[obj_no].type));
      if (invalid_type)
	pop_colour ();
      DrawScreenText (-1, -1, "\1Angle:\2  %s",
	GetAngleName (Things[obj_no].angle));
      DrawScreenText (-1, -1, "\1Flags:\2  %s",
	GetWhenName (Things[obj_no].when));

		if (yg_level_format == YGLF_HEXEN) {
			x0 += width;
			width  = 2 * BOX_BORDER + 2 * WIDE_HSPACING + 40 * FONTW;
			ix0 = x0 + BOX_BORDER + WIDE_HSPACING;
			y0 = out_y1 - height + 1;
			DrawScreenBox3D (x0, y0, x0 + width - 1, y0 + height - 1);

			DrawScreenText (ix0, iy0, "\1TID\2          %d", Things[obj_no].tid);
			if (Things[obj_no].special != 0) {
				DrawScreenText (-1, -1, "\1Special\2     %d %s",
						Things[obj_no].special, GetLineDefTypeName(Things[obj_no].special).c_str());
			} else
				DrawScreenText (-1, -1, "\1Special\2      none");

			DrawScreenText (-1, -1, "\1%-12s\2 %d",
					GetLineDefArgumentName(Things[obj_no].special,1).c_str(),Things[obj_no].arg1);
			DrawScreenText (-1, -1, "\1%-12s\2 %d",
					GetLineDefArgumentName(Things[obj_no].special,2).c_str(),Things[obj_no].arg2);
			DrawScreenText (-1, -1, "\1%-12s\2 %d",
					GetLineDefArgumentName(Things[obj_no].special,3).c_str(),Things[obj_no].arg3);
			DrawScreenText (-1, -1, "\1%-12s\2 %d",
					GetLineDefArgumentName(Things[obj_no].special,4).c_str(),Things[obj_no].arg4);
			DrawScreenText (-1, -1, "\1%-12s\2 %d",
					GetLineDefArgumentName(Things[obj_no].special,5).c_str(),Things[obj_no].arg5);
		}
      // Show the corresponding sprite
      {
	int sx1 = ix1 + 1 - HOLLOW_BORDER;
	int sy1 = iy1 + 1 - HOLLOW_BORDER;
	int sx0 = sx1 + 1 - sprite_width;
	int sy0 = sy1 + 1 - sprite_height;
	draw_box_border (sx0 - HOLLOW_BORDER, sy0 - HOLLOW_BORDER, 
			 sprite_width + 2 * HOLLOW_BORDER,
			 sprite_height + 2 * HOLLOW_BORDER,
			 HOLLOW_BORDER, 0);
	const char *sprite_root = get_thing_sprite (Things[obj_no].type);
	char        flags       = get_thing_flags  (Things[obj_no].type);
	if (sprite_root == NULL)
	{
	  push_colour (WINBG);
	  DrawScreenBox (sx0, sy0, sx1, sy1);
	  pop_colour ();
	  set_colour (WINFG_DIM);
	  DrawScreenText (
	     sx0 + (sprite_width - 2 * FONTW) / 2,
	     sy0 + sprite_height / 2 + 1 - FONTH, "no");
	  DrawScreenText (
	     sx0 + (sprite_width - 6 * FONTW) / 2,
	     sy0 + sprite_height / 2 + 1, "sprite");
	}
	else
	{
	  Lump_loc loc;
	  Img img (sprite_width, sprite_height, false);
	  Sticker sticker;
	  wad_res.sprites.loc_by_root (sprite_root, loc);
	  if (loc.wad == 0
	    || LoadPicture (img, sprite_root, loc, INT_MIN, INT_MIN))
	  {
	    push_colour (WINBG);
	    DrawScreenBox (sx0, sy0, sx1, sy1);
	    pop_colour ();
	    set_colour (CLR_ERROR);
	    DrawScreenString (
	      sx0 + (sprite_width - strlen (sprite_root) * FONTW) / 2,
	      sy0 + sprite_height / 2 + 1 - 3 * FONTH / 2, sprite_root);
	    DrawScreenText (
	      sx0 + (sprite_width - 3 * FONTW) / 2,
	      sy0 + sprite_height / 2 + 1 - FONTH / 2, "not");
	    DrawScreenText (
	      sx0 + (sprite_width - 5 * FONTW) / 2,
	      sy0 + sprite_height / 2 + 1 + FONTH / 2, "found");
	  }
	  else
	  {
	    if (flags & THINGDEF_SPECTRAL)
	      spectrify_img (img);
	    sticker.load (img, true);
	    sticker.draw (drw, 't', sx0, sy0);
	  }
	}
      }
    }
    break;

    case OBJ_LINEDEFS:
      // Linedef
      width  = 2 * BOX_BORDER + 2 * WIDE_HSPACING + 29 * FONTW;
      height = 2 * BOX_BORDER + 2 * WIDE_VSPACING + (int) (8.5 * FONTH);
      x0 = 0;
      y0 = out_y1 - height + 1;
      ix0 = x0 + BOX_BORDER + WIDE_HSPACING;
      iy0 = y0 + BOX_BORDER + WIDE_VSPACING;
      // Ignore box_disp -- always redraw the whole box
      DrawScreenBox3D (x0, y0, x0 + width - 1, y0 + height - 1);
      if (obj_no >= 0)
      {
	set_colour (WINTITLE);
	DrawScreenText (ix0, iy0, "Linedef #%d", obj_no);
	set_colour (WINFG);
	if (yg_level_format != YGLF_HEXEN) {
		DrawScreenText (-1, iy0 + (int) (1.5 * FONTH),
				"\1Flags\2     %.19s",
				GetLineDefFlagsName (LineDefs[obj_no].flags).c_str());
	} else {
		DrawScreenText (-1, iy0 + (int) (1 * FONTH),
				"\1Flags\2     %.19s",
				GetLineDefFlagsName (LineDefs[obj_no].flags).c_str());
	}
	DrawScreenText (-1, -1, "\1Type\2   %3d %.19s", LineDefs[obj_no].type, GetLineDefTypeName (LineDefs[obj_no].type).c_str());
	int tag,first_sector,second_sector;
	
	if (yg_level_format != YGLF_HEXEN)
	{	tag           = LineDefs[obj_no].tag;
	  	first_sector  = NumSectors;
	  	second_sector = NumSectors;
	  	if (tag != 0)
	  	{
	    		for (int n = 0; n < NumSectors; n++)
	      		if (Sectors[n].tag == tag)
	      		{
				if (first_sector >= NumSectors)
		  		first_sector = n;
				else
				{
		  			second_sector = n;
		  			break;
				}
	      		}
	  	}
	  	if (first_sector < NumSectors && second_sector < NumSectors)
	  	  DrawScreenText (-1, -1, "\1Tag:\2      %d (#%d+)", tag, first_sector);
	  	else if (first_sector < NumSectors)
	  	  DrawScreenText (-1, -1, "\1Tag:\2      %d (#%d)", tag, first_sector);
	  	else
	  	  DrawScreenText (-1, -1, "\1Tag:\2      %d (none)", tag);
	
		s1 = LineDefs[obj_no].start;
		s2 = LineDefs[obj_no].end;
		DrawScreenText (-1, -1, "\1Vertices:\2 (#%d, #%d)", s1, s2);
		n = ComputeDist (Vertices[s2].x - Vertices[s1].x,
			 Vertices[s2].y - Vertices[s1].y);
		DrawScreenText (-1, -1, "\1Length:\2   %d", n);
		sd1 = LineDefs[obj_no].sidedef1;
		sd2 = LineDefs[obj_no].sidedef2;
		DrawScreenText (-1, -1, "\1" "1st sd:\2   #%d", sd1);
		DrawScreenText (-1, -1, "\1" "2nd sd:\2   #%d", sd2);
		if (sd1 >= 0)
	  		s1 = SideDefs[sd1].sector;
		else
	  		s1 = -1;
		if (sd2 >= 0)
	  		s2 = SideDefs[sd2].sector;
		else
	  		s2 = -1;
	} else {
		s1 = LineDefs[obj_no].start;
		s2 = LineDefs[obj_no].end;
		n = ComputeDist (Vertices[s2].x - Vertices[s1].x,Vertices[s2].y - Vertices[s1].y);
		DrawScreenText (-1, -1, "\1Length\2       %d", n);
		DrawScreenText (-1, -1, "\1%-12s\2 %d",
				GetLineDefArgumentName(LineDefs[obj_no].type,1).c_str(),LineDefs[obj_no].tag);
		DrawScreenText (-1, -1, "\1%-12s\2 %d",
				GetLineDefArgumentName(LineDefs[obj_no].type,2).c_str(),LineDefs[obj_no].arg2);
		DrawScreenText (-1, -1, "\1%-12s\2 %d",
				GetLineDefArgumentName(LineDefs[obj_no].type,3).c_str(),LineDefs[obj_no].arg3);
		DrawScreenText (-1, -1, "\1%-12s\2 %d",
				GetLineDefArgumentName(LineDefs[obj_no].type,4).c_str(),LineDefs[obj_no].arg4);
		DrawScreenText (-1, -1, "\1%-12s\2 %d",
				GetLineDefArgumentName(LineDefs[obj_no].type,5).c_str(),LineDefs[obj_no].arg5);

		sd1 = LineDefs[obj_no].sidedef1;
		sd2 = LineDefs[obj_no].sidedef2;
		if (sd1 >= 0)
	  		s1 = SideDefs[sd1].sector;
		else
	  		s1 = -1;
		if (sd2 >= 0)
	  		s2 = SideDefs[sd2].sector;
		else
	  		s2 = -1;
	}
		} else {
			const char *message = "(no linedef selected)";
			set_colour (WINFG_DIM);
			DrawScreenText (x0 + (width - FONTW * strlen (message)) / 2,
					y0 + (height - FONTH) / 2, message);
		}

      // 1st sidedef
      x0 += width;
      width  = 2 * BOX_BORDER + 2 * WIDE_HSPACING + 16 * FONTW;
      ix0 = x0 + BOX_BORDER + WIDE_HSPACING;
      y0 = out_y1 - height + 1;
      // Ignore box_disp -- always redraw the whole box
      DrawScreenBox3D (x0, y0, x0 + width - 1, y0 + height - 1);
      if (obj_no >= 0 && sd1 >= 0)
      {
	set_colour (WINTITLE);
	DrawScreenText (ix0, iy0, "Sidedef1 #%d", sd1);

	if (s1 >= 0 && s2 >= 0 && Sectors[s1].ceilh > Sectors[s2].ceilh
	  && ! (is_sky (Sectors[s1].ceilt) && is_sky (Sectors[s2].ceilt)))
	{
	  if (SideDefs[sd1].tex1 == "-")
	    set_colour (CLR_ERROR);
	  else
	    set_colour (WINFG);
	}
	else
	  set_colour (WINFG_DIM);
	DrawScreenText (-1, iy0 + (int) (1.5 * FONTH), string("\1Upper:\2  " + SideDefs[sd1].tex1).c_str());

	if (sd2 < 0
	  && SideDefs[sd1].tex3[0] == '-' && SideDefs[sd1].tex3[1] == '\0')
	  set_colour (CLR_ERROR);
	else
	  set_colour (WINFG);
	DrawScreenText (-1, -1,
	  "\1Middle:\2 %.*s", WAD_TEX_NAME, SideDefs[sd1].tex3.c_str());

	if (s1 >= 0 && s2 >= 0 && Sectors[s1].floorh < Sectors[s2].floorh
	  && ! (is_sky (Sectors[s1].floort) && is_sky (Sectors[s2].floort)))
	{
	  if (SideDefs[sd1].tex2[0] == '-' && SideDefs[sd1].tex2[1] == '\0')
	    set_colour (CLR_ERROR);
	  else
	    set_colour (WINFG);
	}
	else
	  set_colour (WINFG_DIM);
	DrawScreenText (-1, -1, "\1Lower:\2  %.*s",
	  WAD_TEX_NAME, SideDefs[sd1].tex2.c_str());

	set_colour (WINFG);
	DrawScreenText (-1, -1, "\1X-ofs:\2  %d", SideDefs[sd1].xoff);
	DrawScreenText (-1, -1, "\1Y-ofs:\2  %d", SideDefs[sd1].yoff);
	DrawScreenText (-1, -1, "\1Sector:\2 #%d", s1);
      }
      else
      {
	const char *message = "(no 1st sidedef)";
	set_colour (CLR_ERROR);
	DrawScreenText (x0 + (width - FONTW * strlen (message)) / 2,
	  y0 + (height - FONTH) / 2, message);
      }

      // 2nd sidedef
      x0 += width;
      ix0 = x0 + BOX_BORDER + WIDE_HSPACING;
      y0 = out_y1 - height + 1;
      // Ignore box_disp -- always redraw the whole box
      DrawScreenBox3D (x0, y0, x0 + width - 1, y0 + height - 1);
      if (obj_no >= 0 && sd2 >= 0)
      {
	set_colour (WINTITLE);
	DrawScreenText (ix0, iy0, "Sidedef2 #%d", sd2);
	set_colour (WINFG);

	string tex_name = SideDefs[sd2].tex1;  // Upper texture
	if (s1 >= 0 && s2 >= 0 && Sectors[s2].ceilh > Sectors[s1].ceilh
	  && ! (is_sky (Sectors[s1].ceilt) && is_sky (Sectors[s2].ceilt)))
	{
	  if (tex_name[0] == '-' && tex_name[1] == '\0')
	    set_colour (CLR_ERROR);
	  else
	    set_colour (WINFG);
	}
	else
	  set_colour (WINFG_DIM);
	DrawScreenText (-1, iy0 + (int) (1.5 * FONTH),
	  "\1Upper:\2  %.*s", WAD_TEX_NAME, tex_name.c_str());

	tex_name = SideDefs[sd2].tex3;  // Middle texture
	set_colour (WINFG);
	DrawScreenText (-1, -1,
	  "\1Middle:\2 %.*s", WAD_TEX_NAME, tex_name.c_str());

	tex_name = SideDefs[sd2].tex2;  // Lower texture
	if (s1 >= 0 && s2 >= 0 && Sectors[s2].floorh < Sectors[s1].floorh
	  && ! (is_sky (Sectors[s1].floort) && is_sky (Sectors[s2].floort)))
	{
	  if (tex_name[0] == '-' && tex_name[1] == '\0')
	    set_colour (CLR_ERROR);
	  else
	    set_colour (WINFG);
	}
	else
	  set_colour (WINFG_DIM);
	DrawScreenText (-1, -1, "\1Lower:\2  %.*s", WAD_TEX_NAME, tex_name.c_str());

	set_colour (WINFG);
	DrawScreenText (-1, -1, "\1X-ofs:\2  %d", SideDefs[sd2].xoff);
	DrawScreenText (-1, -1, "\1Y-ofs:\2  %d", SideDefs[sd2].yoff);
	DrawScreenText (-1, -1, "\1Sector:\2 #%d", s2);
      }
      else
      {
	const char *message = "(no 2nd sidedef)";
	// If the "2" flag is set, there must be a second sidedef
	if (LineDefs[obj_no].flags & 0x04)  // FIXME hard-coded
	  set_colour (CLR_ERROR);
	else
	  set_colour (WINFG_DIM);
	DrawScreenText (x0 + (width - FONTW * strlen (message)) / 2,
	  y0 + (height - FONTH) / 2, message);
      }

      // Superimposed linedefs
      {
	Superimposed_ld super;
	super.set (obj_no);
	obj_no_t l = super.get ();
	int iy1;

	if (l != -1 || box_disp[3])
	{
	  x0 += width;
	  width  = 2 * BOX_BORDER + 2 * WIDE_HSPACING + 12 * FONTW;
	  ix0 = x0 + BOX_BORDER + WIDE_HSPACING;
	  iy0 = y0 + BOX_BORDER + WIDE_VSPACING;
	  iy1 = y0 + height - 1 - BOX_BORDER - WIDE_VSPACING;
	  DrawScreenBox3D (x0, y0, x0 + width - 1, y0 + height - 1);
	}
	if (l != -1)
	{
	  box_disp[3] = true;
	  set_colour (WINTITLE);
	  DrawScreenString (ix0, iy0, "Superimposed");
	  set_colour (WINFG);
	  iy0 += (int)(1.5 * FONTH);
	  while (l != -1)
	  {
	    if ((signed)(iy0 + FONTH - 1) <= iy1)
	      DrawScreenText (ix0, iy0, "#%d", l);
	    /* Too many linedefs, replace the last one by "(more)".
	       Not elegant, but it makes the code simpler. */
	    else
	    {
	      iy0 -= FONTH;
	      set_colour (WINBG);
	      DrawScreenBox (ix0, iy0, ix0 + 12 * FONTW - 1, iy0 + FONTH - 1);
	      set_colour (WINFG);
	      DrawScreenString (ix0, iy0, "(more)");
	      break;
	    }
	    iy0 += FONTH;
	    l = super.get ();
	  }
	}
      }
      break;

    case OBJ_VERTICES:
      width  = 2 * BOX_BORDER + 2 * WIDE_HSPACING + 29 * FONTW;
      height = 2 * BOX_BORDER + 2 * WIDE_VSPACING + (int) (2.5 * FONTH);
      x0 = 0;
      y0 = out_y1 - height + 1;
      ix0 = x0 + BOX_BORDER + WIDE_HSPACING;
      iy0 = y0 + BOX_BORDER + WIDE_VSPACING;
      // Ignore box_disp -- always redraw the whole box
      DrawScreenBox3D (x0, y0, x0 + width - 1, y0 + height - 1);
      if (obj_no < 0)
      {
	const char *message = "(no vertex selected)";
	set_colour (WINFG_DIM);
	DrawScreenText (x0 + (width - FONTW * strlen (message)) / 2,
	  y0 + (height - FONTH) / 2, message);
	break;
      }
      set_colour (WINTITLE);
      DrawScreenText (ix0, iy0, "Vertex #%d", obj_no);
      set_colour (WINFG);
      DrawScreenText (-1, iy0 + (int) (1.5 * FONTH),
	"\1Coordinates:\2 (%d, %d)", Vertices[obj_no].x, Vertices[obj_no].y);
      break;

    case OBJ_SECTORS:
    {
      int x1, y1;
      int ix1, iy1;
      const int columns = 24;
      width  = BOX_BORDER
	     + WIDE_HSPACING
	     + columns * FONTW
	     + WIDE_HSPACING
	     + HOLLOW_BORDER
	     + DOOM_FLAT_WIDTH
	     + HOLLOW_BORDER
	     + WIDE_HSPACING
	     + BOX_BORDER;
      height = 2 * BOX_BORDER
	     + 2 * WIDE_VSPACING
	     + y_max ((unsigned) (9.5 * FONTH),
		    WIDE_HSPACING + 4 * HOLLOW_BORDER + 2 * DOOM_FLAT_HEIGHT);
      x0 = 0;
      y0 = out_y1 - height + 1;
      x1 = x0 + width - 1;
      y1 = y0 + height - 1;
      ix0 = x0 + BOX_BORDER + WIDE_HSPACING;
      iy0 = y0 + BOX_BORDER + WIDE_VSPACING;
      ix1 = x1 - BOX_BORDER - WIDE_HSPACING;
      iy1 = y1 - BOX_BORDER - WIDE_VSPACING;
      if (box_disp[0])
      {
	push_colour (WINBG);
	DrawScreenBox (ix0, iy0, ix0 + columns * FONTW - 1, iy1);
	pop_colour ();
      }
      else
	DrawScreenBox3D (x0, y0, x1, y1);
      if (obj_no < 0)
      {
	const char *const message = "(no sector selected)";
	set_colour (WINFG_DIM);
	DrawScreenText (x0 + (width - FONTW * strlen (message)) / 2,
	  y0 + (height - FONTH) / 2, message);
	break;
      }
      set_colour (WINTITLE);
      DrawScreenText (ix0, iy0, "Sector #%d", obj_no);
      set_colour (WINFG);
      const struct Sector *sec = Sectors + obj_no;
      if (prev_sector >= 0 && prev_sector != obj_no)
      {
	DrawScreenText (-1, iy0 + (int) (1.5 * FONTH),
	  "\1Floor:\2    %d (%+d)",
	  sec->floorh, sec->floorh - prev_floorh);
	DrawScreenText (-1, -1, "\1Ceiling:\2  %d (%+d)",
	  sec->ceilh, sec->ceilh - prev_ceilh);
      }
      else
      {
	DrawScreenText (-1, iy0 + (int) (1.5 * FONTH),
	  "\1Floor:\2    %d",   sec->floorh);
        DrawScreenText (-1, -1, "\1Ceiling:\2  %d",  sec->ceilh);
      }
      DrawScreenText (-1, -1, "\1Headroom:\2 %d",   sec->ceilh - sec->floorh);
      DrawScreenText (-1, -1, "\1Floor:\2    %.*s", WAD_FLAT_NAME, sec->floort);
      DrawScreenText (-1, -1, "\1Ceiling:\2  %.*s", WAD_FLAT_NAME, sec->ceilt);
      DrawScreenText (-1, -1, "\1Light:\2    %d",   sec->light);
      DrawScreenText (-1, -1, "\1Type:\2 %3d %.14s",
	sec->special, GetSectorTypeName (sec->special).c_str());
      {
	int tag       = sec->tag;
	int first_ld  = NumLineDefs;
	int second_ld = NumLineDefs;

	if (tag != 0)
	{
	  for (n = 0; n < NumLineDefs; n++)
	    if (LineDefs[n].tag == tag)
	    {
	      if (first_ld >= NumLineDefs)
		first_ld = n;
	      else
	      {
		second_ld = n;
		break;
	      }
	    }
	}
	if (first_ld < NumLineDefs && second_ld < NumLineDefs)
	   DrawScreenText (-1, -1, "\1Tag:\2      %d (#%d+)", tag, first_ld);
	else if (first_ld < NumLineDefs)
	   DrawScreenText (-1, -1, "\1Tag:\2      %d (#%d)", tag, first_ld);
	else if (tag == 99 || tag == 999)
	   DrawScreenText (-1, -1, "\1Tag:\2      %d (stairs?)", tag);
	else if (tag == 666)
	   DrawScreenText (-1, -1, "\1Tag:\2      %d (lower@end)", tag);
	else if (tag == 667)
	   DrawScreenText (-1, -1, "\1Tag:\2      %d (raise@end)", tag);
	else
	   DrawScreenText (-1, -1, "\1Tag:\2      %d (none)", tag);
      }
      {
	hookfunc_comm_t block;

	// Display the floor texture in the bottom right corner
	block.x1 = ix1;
	block.x0 = block.x1 - (DOOM_FLAT_WIDTH + 2 * HOLLOW_BORDER - 1);
	block.y1 = iy1;
	block.y0 = block.y1 - (DOOM_FLAT_HEIGHT + 2 * HOLLOW_BORDER - 1);
	block.name = sec->floort;
	display_flat_depressed (&block);

	// Display the ceiling texture above the floor texture
	block.y1 = block.y0 - (WIDE_VSPACING + 1);
	block.y0 = block.y1 - (DOOM_FLAT_HEIGHT + 2 * HOLLOW_BORDER - 1);
	block.name = sec->ceilt;
	display_flat_depressed (&block);
      }

      // Show all EDGE extrafloors for this sector
      {
	x0 += width;
	const int columns2 = 16;
	const int width2   = BOX_BORDER
			   + WIDE_HSPACING
			   + columns2 * FONTW
			   + WIDE_HSPACING
			   + HOLLOW_BORDER
			   + DOOM_FLAT_WIDTH
			   + HOLLOW_BORDER
			   + WIDE_HSPACING
			   + BOX_BORDER;
	std::vector<Extraf> v;
	get_extrafloors (v, sec->tag);
	size_t e;
	for (e = 0; e < v.size () && e + 1 < MAX_BOXES; e++, x0 += width2)
	{
	  const Extraf& i = v[e];
	  obj_no_t dsecno = i.sector;
	  bool thick = (i.tex != "");
	  const struct Sector *dsec = Sectors + dsecno;
	  x1 = x0 + width2 - 1;
	  y1 = y0 + height - 1;
	  ix0 = x0 + BOX_BORDER + WIDE_HSPACING;
	  iy0 = y0 + BOX_BORDER + WIDE_VSPACING;
	  ix1 = x1 - BOX_BORDER - WIDE_HSPACING;
	  iy1 = y1 - BOX_BORDER - WIDE_VSPACING;
	  if (box_disp[e + 1])  // FIXME
	  {
	    push_colour (WINBG);
	    DrawScreenBox (ix0, iy0, ix0 + columns2 * FONTW - 1, iy1);
	    pop_colour ();
	  }
	  else
	  {
	    DrawScreenBox3D (x0, y0, x1, y1);
	    box_disp[e + 1] = true;
	  }
	  if (! is_sector (dsecno))  // Can't happen
	    continue;
	  set_colour (WINTITLE);
	  if (thick)
	    DrawScreenText (ix0, iy0, "Thick #%d", dsecno);
	  else
	    DrawScreenText (ix0, iy0, "Thin #%d", dsecno);
	  set_colour (WINFG);
	  if (thick)
	  {
	    DrawScreenText (-1, iy0 + (int) (1.5 * FONTH),
				    "\1Bottom:\2 %d",   dsec->floorh);
	    DrawScreenText (-1, -1, "\1Top:\2    %d",   dsec->ceilh);
	    DrawScreenText (-1, -1, "\1Thick:\2  %d",   dsec->ceilh - dsec->floorh);
	    DrawScreenText (-1, -1, "\1Bottom:\2 %.*s", WAD_FLAT_NAME,dsec->floort);
	    DrawScreenText (-1, -1, "\1Top:\2    %.*s", WAD_FLAT_NAME, dsec->ceilt);
	  }
	  else
	  {
	    DrawScreenText (-1, iy0 + (int) (1.5 * FONTH),
				    "\1Height:\2 %d",   dsec->floorh);
	    DrawScreenText (-1, -1, "\1Flat:\2   %.*s", WAD_FLAT_NAME,dsec->floort);
	  }
	  DrawScreenText (-1, -1, "\1Shadow:\2 %d",   dsec->light);
	  DrawScreenText (-1, -1, "\1Type:\2   %d",   dsec->special);
	  if (thick)
	    DrawScreenText (-1, -1, "\1Side:\2   %.*s", WAD_TEX_NAME, i.tex.c_str());
	  {
	    hookfunc_comm_t block;

	    // Display the top texture in the bottom right corner
	    block.x1 = ix1;
	    block.x0 = block.x1 - (DOOM_FLAT_WIDTH + 2 * HOLLOW_BORDER - 1);
	    block.y1 = iy1;
	    block.y0 = block.y1 - (DOOM_FLAT_HEIGHT + 2 * HOLLOW_BORDER - 1);
	    block.name = dsec->floort;
	    display_flat_depressed (&block);

	    // Display the bottom texture above the floor texture
	    block.y1 = block.y0 - (WIDE_VSPACING + 1);
	    block.y0 = block.y1 - (DOOM_FLAT_HEIGHT + 2 * HOLLOW_BORDER - 1);
	    if (thick)
	    {
	      block.name = dsec->ceilt;
	      display_flat_depressed (&block);
	    }
	    else
	    {
	      push_colour (WINBG);
	      DrawScreenBoxwh (block.x0, block.y0, DOOM_FLAT_WIDTH
	        + 2 * HOLLOW_BORDER, DOOM_FLAT_HEIGHT + 2 * HOLLOW_BORDER);
	      pop_colour ();
	    }
	  }
	}
	// Clear out remaining boxes
	for (; e + 1 < MAX_BOXES && box_disp[e + 1]; e++, x0 += width2)
	{
	  x1 = x0 + width2 - 1;
	  y1 = y0 + height - 1;
	  ix0 = x0 + BOX_BORDER + WIDE_HSPACING;
	  iy0 = y0 + BOX_BORDER + WIDE_VSPACING;
	  ix1 = x1 - BOX_BORDER - WIDE_HSPACING;
	  iy1 = y1 - BOX_BORDER - WIDE_VSPACING;
	  push_colour (WINBG);
	  DrawScreenBox (ix0, iy0, ix1, iy1);
	  pop_colour ();
	}
      }
      break;
    }
  }

  if (obj_type == OBJ_SECTORS)
  {
    if (obj_no != prev_sector)
      prev_sector = obj_no;
    if (obj_no >= 0)
    {
      prev_floorh = Sectors[obj_no].floorh;
      prev_ceilh  = Sectors[obj_no].ceilh;
    }
  }
  box_disp[0] = true;
  obj_no_disp = obj_no;
  obj_type_disp = obj_type;
}


/*
 *	get_extrafloors - get list of EDGE extrafloors for tag
 *
 *	Put in <v> a list of the extrafloors for tag <tag>,
 *	sorted by floor height major, sector number minor. The
 *	previous content of <v> is lost. Each Extraf object in
 *	<v> is set up in the following way :
 *
 *	- <sector> is set to the number of the dummy sector,
 *
 *	- <tex> is set to the side texture of the extrafloor, or
 *	  "" if it's a thin extrafloor,
 *
 *	- <height> is the to the floor height of the dummy
 *	  sector.
 */
static void get_extrafloors (std::vector<Extraf>& v, wad_tag_t tag) {
	v.clear ();
	for (obj_no_t l = 0; l < NumLineDefs; l++) {
		if (LineDefs[l].tag != tag || !(LineDefs[l].type >= 400 && LineDefs[l].type <= 407))
			continue;
		obj_no_t sd = LineDefs[l].sidedef1;
		if (! is_sidedef (sd) || ! is_sector (SideDefs[sd].sector))  // Paranoia
			continue;
		string tex;
		if (LineDefs[l].type == 400)
			tex = SideDefs[sd].tex3;
		else if (LineDefs[l].type == 401)		// side_upper
			tex = SideDefs[sd].tex1;
		else if (LineDefs[l].type == 402)		// side_lower
			tex = SideDefs[sd].tex2;
		else
			tex = "";
		v.push_back (Extraf (SideDefs[sd].sector,
					tex,
					Sectors[SideDefs[sd].sector].floorh));
	}
	sort (v.begin (), v.end ());
}


