/*
 *	infobar.cc
 *	The infobar_c class.
 *	AYM 1998-10-10
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


/*
The way the info bar is handled is something of a mess.
In principle, its various text fields should be widgets in their
own right. Out of laziness, said text fields are not and can't
undraw themselves either so the whole box is redrawn everytime one
of the text fields changes. The "pointer position" field is treated
specially ; since it changes very often, it can undraw itself.
*/


#include "yadex.h"
#include "gfx.h"
#include "infobar.h"
#include "objid.h"


const char infobar_c::FILE_NAME_UNSET[1] = { ' ' };  // A special pointer value 
const char infobar_c::LEVEL_NAME_UNSET[1] = { ' ' };  // A special pointer value


infobar_c::infobar_c ()
{
visible               = 0;
visible_disp          = 0;
file_name             = 0;
file_name_disp        = FILE_NAME_UNSET;
level_name            = 0;
level_name_disp       = LEVEL_NAME_UNSET;
obj_type              = OBJ_NONE;
obj_type_disp         = OBJ_NONE;
changes               = -1;
changes_disp          = -1;
grid_snap             = -1;
grid_snap_disp        = -1;
grid_step_locked      = -1;
grid_step_locked_disp = -1;
scale                 = -1;
scale_disp            = -1;
grid_step             = -1;
grid_step_disp        = -1;
flags                 = 0;
}




void infobar_c::draw ()
{
int x;
int redraw_from_scratch;

x = text_x0;

if (! visible)
   return;

redraw_from_scratch =
   (visible && ! visible_disp)
   || (file_name_disp        != file_name)
   || (level_name_disp       != level_name)
   || (obj_type_disp         != obj_type)
   || (changes_disp          != changes)
   || (grid_snap_disp        != grid_snap)
   || (grid_step_locked_disp != grid_step_locked)
   || (scale_disp            != scale)
   || (grid_step_disp        != grid_step);

if (redraw_from_scratch)
   {
   DrawScreenBox3D (0, out_y0, ScrMaxX, ScrMaxY);
   visible_disp = 1;
   flags &= ~ pointer_disp_set;
   }

set_colour (WINFG);

// The name of the file being edited.
{
int chars;
if (! file_name)
   {
   const char *const msg = "(New level)";
   if (redraw_from_scratch || file_name_disp != file_name)
      DrawScreenText (x, text_y0, msg);
   chars = strlen (msg);
   }
else
   {
   al_fbase_t filebase;
   al_fext_t fileext;
   // FIXME wasteful to do it each time
   al_fana (file_name, 0, 0, filebase, fileext);
   if (redraw_from_scratch || file_name_disp != file_name)
      DrawScreenText (x, text_y0, "%s%s", filebase, fileext);
   chars = strlen (filebase) + strlen (fileext);
   }
x += (chars + 2) * FONTW;
file_name_disp = file_name;
}

// The name of the level being edited.
{
int chars;
if (! level_name)
   {
   const char *const msg = "(n/s)";
   if (redraw_from_scratch || level_name_disp != level_name)
      DrawScreenText (x, text_y0, msg);
   chars = strlen (msg);
   }
else
   {
   if (redraw_from_scratch || level_name_disp != level_name)
      DrawScreenText (x, text_y0, "%.5s", level_name);
   chars = strlen (level_name);
   }
x += (chars + 2) * FONTW;
level_name_disp = level_name;
}

// Type of objects being edited.
if (redraw_from_scratch || obj_type_disp != obj_type)
   {
   DrawScreenText (x, text_y0, "%s", GetEditModeName (obj_type));
   obj_type_disp = obj_type;
   }
x += 10 * FONTW;

// Any changes made ?
if (redraw_from_scratch || changes_disp != changes)
   {
   DrawScreenText (x, text_y0, changes > 1 ? "**" : (changes ? "*" : ""));
   changes_disp = changes;
   }
x += 4 * FONTW;

// scale
if (redraw_from_scratch || scale_disp != scale)
   {
#ifdef OLD
   if (Scale < 1.0)
      DrawScreenText (x, text_y0, "Scale: 1/%d", (int) (1.0 / Scale + 0.5));
   else
      DrawScreenText (x, text_y0, "Scale: %d/1", (int) Scale);
#else
   DrawScreenText (x, text_y0, "Scale: %d%%", (int) (Scale * 100));
#endif
   scale_disp = scale;
   }
x += 13 * FONTW;

// grid_step
if (redraw_from_scratch || grid_step_disp != grid_step)
   {
   DrawScreenText (x, text_y0, "Grid: %d", grid_step);
   grid_step_disp = grid_step;
   }
x += 11 * FONTW;

// grid_step_locked
if (redraw_from_scratch || grid_step_locked_disp != grid_step_locked)
   {
   DrawScreenText (x, text_y0, "%s", grid_step_locked ? "Lock" : "Auto");
   grid_step_locked_disp = grid_step_locked;
   }
x += 6 * FONTW;

// grid_snap
if (redraw_from_scratch || grid_snap_disp != grid_snap)
   {
   DrawScreenText (x, text_y0, "%s", grid_snap ? "Snap" : "Free");
   grid_snap_disp = grid_snap;
   }
x += 6 * FONTW;

// The current pointer coordinates.
if (((flags & pointer_disp_set) && !(flags & pointer_set))
		|| pointer_x_disp != pointer_x
		|| pointer_y_disp != pointer_y)
   {
   set_colour (WINBG);
   DrawScreenBox (x, text_y0, x + 14 * FONTW - 1, text_y1);
   flags &= ~ pointer_disp_set;
   }
if (((flags & pointer_set) && !(flags & pointer_disp_set))
		|| pointer_x_disp != pointer_x
		|| pointer_y_disp != pointer_y)
   {
   set_colour (WINFG);
   /* FIXME pointer_x/y are not significant the first time. */
   DrawScreenText (x, text_y0, "%5d, %5d", pointer_x, pointer_y);
   pointer_x_disp = pointer_x;
   pointer_y_disp = pointer_y;
   flags |= pointer_disp_set;
   }
}


void infobar_c::clear ()
{
visible_disp          = 0;
file_name_disp        = FILE_NAME_UNSET;
level_name_disp       = LEVEL_NAME_UNSET;
obj_type_disp         = OBJ_NONE;
changes_disp          = -1;
grid_snap_disp        = -1;
grid_step_locked_disp = -1;
scale_disp            = -1;
grid_step_disp        = -1;
}


