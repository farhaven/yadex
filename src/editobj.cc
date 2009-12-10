/*
 *	editobj.cc
 *	Object editing routines.
 *	BW & RQ sometime in 1993 or 1994.
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
#include "_edit.h"
#include "dialog.h"
#include "editobj.h"
#include "entry.h"
#include "flats.h"
#include "game.h"
#include "gfx.h"
#include "levels.h"
#include "objects.h"
#include "objid.h"
#include "oldmenus.h"
#include "s_slice.h"
#include "s_swapf.h"
#include "selectn.h"
#include "t_spin.h"
#include "x_mirror.h"
#include "x_rotate.h"


/*
   ask for an object number and check for maximum valid number
   (this is just like InputIntegerValue, but with a different prompt)
*/

int InputObjectNumber (int x0, int y0, int objtype, int curobj)
{
int val, key;
char prompt[80];

sprintf (prompt, "Enter a %s number between 0 and %d:",
   GetObjectTypeName (objtype), GetMaxObjectNum (objtype));
if (x0 < 0)
   x0 = (ScrMaxX - 25 - 8 * strlen (prompt)) / 2;
if (y0 < 0)
   y0 = (ScrMaxY - 55) / 2;
DrawScreenBox3D (x0, y0, x0 + 25 + 8 * strlen (prompt), y0 + 55);
set_colour (WHITE);
DrawScreenText (x0 + 10, y0 + 8, prompt);
val = curobj;
while ((key
       = InputInteger (x0 + 10, y0 + 28, &val, 0, GetMaxObjectNum (objtype)))
       != YK_RETURN && key != YK_ESC)
   Beep ();
return val;
}


/*
 *	input_objid - ask for an object number of the specified	type
 *
 *	If the user hit [Return], set objid.type to init.type
 *	and objid.num to whatever number the user entered. If
 *	the user hit [Esc], call nil() on objid.
 */
void input_objid (Objid& objid, const Objid& init, int x0, int y0)
{
char prompt[80];

sprintf (prompt, "Enter a %s number between 0 and %d:",
   GetObjectTypeName (init.type), GetMaxObjectNum (init.type));
if (x0 < 0)
   x0 = (ScrMaxX - 25 - 8 * strlen (prompt)) / 2;
if (y0 < 0)
   y0 = (ScrMaxY - 55) / 2;
DrawScreenBox3D (x0, y0, x0 + 25 + 8 * strlen (prompt), y0 + 55);
set_colour (WHITE);
DrawScreenText (x0 + 10, y0 + 8, prompt);
int  num = init.num;
int  key;
while ((key
       = InputInteger (x0 + 10, y0 + 28, &num, 0, GetMaxObjectNum (init.type)))
       != YK_RETURN && key != YK_ESC)
   Beep ();
if (key == YK_ESC)
   objid.nil ();
else if (key == YK_RETURN)
   {
   objid.type = init.type;
   objid.num  = num;
   }
else
   {
   nf_bug ("input_objid: bad key %d", (int) key);  // Can't happen
   objid.nil ();
   }
}


/*
   ask for an object number and display a warning message
*/

int InputObjectXRef (int x0, int y0, int objtype, bool allownone, int curobj)
{
const char *const msg1 = "Warning: modifying the cross-references";
const char *const msg2 = "between some objects may crash the game.";
char prompt[80];
size_t maxlen = 0;
int width;
int height;

// Dimensions
sprintf (prompt, "Enter a %s number between 0 and %d%c",
  GetObjectTypeName (objtype),
  GetMaxObjectNum (objtype), allownone ? ',' : ':');
maxlen = 40;				// Why 40 ? -- AYM 2002-04-17
if (strlen (prompt) > maxlen);
   maxlen = strlen (prompt);
if (strlen (msg1) > maxlen)
   maxlen = strlen (msg1);
if (strlen (msg2) > maxlen)
   maxlen = strlen (msg2);
int ya = 0 + BOX_BORDER + WIDE_VSPACING;
int yb = ya;
if (allownone)
  yb += FONTH;
int yc = yb + FONTH + WIDE_VSPACING;
// FIXME should query InputInteger() instead
int yd = yc + 2 * HOLLOW_BORDER + 2 * NARROW_VSPACING + FONTH + WIDE_VSPACING;
int ye = yd + FONTH;
int yf = ye + FONTH + WIDE_VSPACING + BOX_BORDER;
width  = 2 * BOX_BORDER + 2 * WIDE_HSPACING + maxlen * FONTW;
height = yf - 0;

// Position
if (x0 < 0)
   x0 = (ScrMaxX - width) / 2;
if (y0 < 0)
   y0 = (ScrMaxY - height) / 2;

DrawScreenBox3D (x0, y0, x0 + width, y0 + height);
set_colour (WHITE);
int x = x0 + BOX_BORDER + WIDE_HSPACING;
DrawScreenText (x, y0 + ya, prompt);
if (allownone)
   DrawScreenText (x, y0 + yb, "or -1 for none:");
set_colour (LIGHTRED);
DrawScreenText (x, y0 + yd, msg1);
DrawScreenText (x, y0 + ye, msg2);

int val = curobj;
int key;
int min = allownone ? -1 : 0;
int max = GetMaxObjectNum (objtype);
while (key = InputInteger (x, y0 + yc, &val, min, max),
       key != YK_RETURN && key != YK_ESC)
   Beep ();
return val;
}



/*
   ask for two vertex numbers and check for maximum valid number
*/

bool Input2VertexNumbers (int x0, int y0, const char *prompt1, int *v1, int *v2)
{
int  key;
int  maxlen, first;
char prompt2[80];
int text_x0;
int text_y0;
int entry1_x0;
int entry1_y0;
int entry2_x0;
int entry2_y0;
// FIXME should let InputInteger() tell us
const int entry_width  = 2 * HOLLOW_BORDER + 2 * NARROW_HSPACING + 7 * FONTW;
const int entry_height = 2 * HOLLOW_BORDER + 2 * NARROW_VSPACING + FONTH;

sprintf (prompt2, "Enter two numbers between 0 and %d:", NumVertices - 1);

if (strlen (prompt1) > strlen (prompt2))
   maxlen = strlen (prompt1);
else
   maxlen = strlen (prompt2);
if (x0 < 0)
   x0 = (ScrMaxX - 25 - 8 * maxlen) / 2;
if (y0 < 0)
   y0 = (ScrMaxY - 75) / 2;
text_x0 = x0 + BOX_BORDER + WIDE_HSPACING;
text_y0 = y0 + BOX_BORDER + WIDE_VSPACING;
entry1_x0 = text_x0 + 13 * FONTW;
entry1_y0 = text_y0 + 3 * FONTH - HOLLOW_BORDER - NARROW_VSPACING;
entry2_x0 = entry1_x0;
entry2_y0 = text_y0 + 5 * FONTH - HOLLOW_BORDER - NARROW_VSPACING;

DrawScreenBox3D (x0, y0,
   x0 + 2 * BOX_BORDER + 2 * WIDE_HSPACING
      + y_max (entry_width + 13 * FONTW, maxlen * FONTW) - 1,
   y0 + 2 * BOX_BORDER + 2 * WIDE_VSPACING + 6 * FONTH - 1);
set_colour (WHITE);
DrawScreenText (text_x0, text_y0, prompt1);
set_colour (WINFG);
DrawScreenText (text_x0, text_y0 + FONTH, prompt2);
DrawScreenText (text_x0, text_y0 + 3 * FONTH, "Start vertex");
DrawScreenText (text_x0, text_y0 + 5 * FONTH, "End vertex");

first = 1;
key = 0;
for (;;)
   {
   DrawScreenBoxHollow (entry1_x0, entry1_y0,
      entry1_x0 + entry_width - 1, entry1_y0 + entry_height - 1, BLACK);
   set_colour (first ? WHITE : DARKGREY);
   DrawScreenText (entry1_x0 + HOLLOW_BORDER + NARROW_HSPACING,
      entry1_y0 + HOLLOW_BORDER + NARROW_VSPACING, "%d", *v1);

   DrawScreenBoxHollow (entry2_x0, entry2_y0,
      entry2_x0 + entry_width - 1, entry2_y0 + entry_height - 1, BLACK);
   set_colour (! first ? WHITE : DARKGREY);
   DrawScreenText (entry2_x0 + HOLLOW_BORDER + NARROW_HSPACING,
      entry2_y0 + HOLLOW_BORDER + NARROW_VSPACING, "%d", *v2);

   if (first)
      key = InputInteger (entry1_x0, entry1_y0, v1, 0, NumVertices - 1);
   else
      key = InputInteger (entry2_x0, entry2_y0, v2, 0, NumVertices - 1);
   if (key==YK_LEFT || key==YK_RIGHT || key==YK_TAB || key==YK_BACKTAB)
      first = !first;
   else if (key == YK_ESC)
      break;
   else if (key == YK_RETURN)
      {
      if (first)
	 first = 0;
      else if (*v1 < 0 || *v1 >= NumVertices
            || *v2 < 0 || *v2 >= NumVertices)
	 Beep ();
      else
	 break;
      }
   else
      Beep ();
   }
return (key == YK_RETURN);
}



/*
   edit an object or a group of objects
*/

void EditObjectsInfo (int x0, int y0, int objtype, SelPtr obj) /* SWAP! */
{
char  *menustr[3];
int    n, val;
SelPtr cur;
int    subwin_y0;

ObjectsNeeded (objtype, 0);
if (! obj)
   return;
switch (objtype)
   {
   case OBJ_THINGS:
      ThingProperties (x0, y0, obj);
      break;

   case OBJ_VERTICES:
      for (n = 0; n < 3; n++)
	 menustr[n] = (char *) GetMemory (60);
      sprintf (menustr[2], "Edit Vertex #%d", obj->objnum);
      sprintf (menustr[0], "Change X position (Current: %d)",
         Vertices[obj->objnum].x);
      sprintf (menustr[1], "Change Y position (Current: %d)",
         Vertices[obj->objnum].y);
#ifdef OLDMEN
      val = DisplayMenuArray (0, y0,
         menustr[2], 2, NULL, menustr, NULL, NULL, NULL);
#else
      val = vDisplayMenu (0, y0, menustr[2],
         menustr[0], YK_, 0,
         menustr[1], YK_, 0,
	 NULL);
#endif
      for (n = 0; n < 3; n++)
	 FreeMemory (menustr[n]);
      subwin_y0 = y0 + BOX_BORDER + (2 + val) * FONTH;
      switch (val)
	 {
	 case 1:
	    val = InputIntegerValue (x0 + 42, subwin_y0,
               y_min (MapMinX, -10000),
               y_max (MapMaxX, 10000),
               Vertices[obj->objnum].x);
	    if (val != IIV_CANCEL)
	       {
	       n = val - Vertices[obj->objnum].x;
	       for (cur = obj; cur; cur = cur->next)
		  Vertices[cur->objnum].x += n;
	       MadeChanges = 1;
	       MadeMapChanges = 1;
	       }
	    break;

	 case 2:
	    val = InputIntegerValue (x0 + 42, subwin_y0,
               y_min (MapMinY, -10000),
               y_max (MapMaxY, 10000),
               Vertices[obj->objnum].y);
	    if (val != IIV_CANCEL)
	       {
	       n = val - Vertices[obj->objnum].y;
	       for (cur = obj; cur; cur = cur->next)
		  Vertices[cur->objnum].y += n;
	       MadeChanges = 1;
	       MadeMapChanges = 1;
	       }
	    break;
	 }
      break;

   case OBJ_LINEDEFS:
      LinedefProperties (x0, y0, obj);
      break;

   case OBJ_SECTORS:
      SectorProperties (x0, y0, obj);
      break;
   }
}


/*
   Yuck!  Dirty piece of code...
*/

bool Input2Numbers (int x0, int y0, const char *name1, const char *name2,
   int v1max, int v2max, int *v1, int *v2)
{
int  key;
int  maxlen, first;
bool ok;
char prompt[80];
// FIXME copied from InputInteger()...
int  entry_width  = 2 * HOLLOW_BORDER + 2 * NARROW_HSPACING + 7 * FONTW;
int  entry_height = 2 * HOLLOW_BORDER + 2 * NARROW_VSPACING + FONTH;

y_snprintf (prompt, sizeof prompt, "Give the %s and %s for the object:",
   name1, name2);
maxlen = strlen (prompt);

int title_x0       = BOX_BORDER + FONTW;
int title_y0       = BOX_BORDER + FONTH / 2;
int label1_x0      = title_x0;
int label1_y0      = title_y0 + 2 * FONTH;
int label2_x0      = title_x0 + (strlen (name1) + 2) * FONTW;
{
   int bound = label1_x0 + entry_width + int (FONTW);
   if (label2_x0 < bound)
      label2_x0 = bound;
}
// FIXME Assuming the range is not longer than the name
int label2_y0      = label1_y0;
int entry1_out_x0  = label1_x0;
int entry1_out_y0  = label1_y0 + 3 * FONTH / 2;
int entry1_text_x0 = entry1_out_x0 + HOLLOW_BORDER + NARROW_HSPACING;
int entry1_text_y0 = entry1_out_y0 + HOLLOW_BORDER + NARROW_VSPACING;
int entry1_out_x1  = entry1_out_x0 + entry_width - 1;
int entry1_out_y1  = entry1_out_y0 + entry_height - 1;
int entry2_out_x0  = label2_x0;
int entry2_out_y0  = label2_y0 + 3 * FONTH / 2;
int entry2_text_x0 = entry2_out_x0 + HOLLOW_BORDER + NARROW_HSPACING;
int entry2_text_y0 = entry2_out_y0 + HOLLOW_BORDER + NARROW_VSPACING;
int entry2_out_x1  = entry2_out_x0 + entry_width - 1;
int entry2_out_y1  = entry2_out_y0 + entry_height - 1;
int range1_x0      = entry1_out_x0;
int range1_y0      = entry1_out_y1 + FONTH / 2;
int range2_x0      = entry2_out_x0;
int range2_y0      = entry2_out_y1 + FONTH / 2;
int window_x1      = entry2_out_x1 + FONTW + BOX_BORDER;
int window_y1      = range1_y0 + 3 * FONTH / 2 + BOX_BORDER;
{
   int bound = 2 * BOX_BORDER + (maxlen + 2) * int (FONTW);
   if (window_x1 < bound)
      window_x1 = bound;
}

if (x0 < 0)
   x0 = (ScrMaxX - window_x1) / 2;
if (y0 < 0)
   y0 = (ScrMaxY - window_y1) / 2;

DrawScreenBox3D (x0, y0, x0 + window_x1, y0 + window_y1);
set_colour     (WHITE);
DrawScreenText (x0 + title_x0,  y0 + title_x0,  prompt);
DrawScreenText (x0 + label1_x0, y0 + label1_y0, name1);
DrawScreenText (x0 + label2_x0, y0 + label2_y0, name2);
DrawScreenText (x0 + range1_x0, y0 + range1_y0, "(0-%d)", v1max);
DrawScreenText (x0 + range2_x0, y0 + range2_y0, "(0-%d)", v2max);

first = 1;
key = 0;
for (;;)
   {
   ok = true;
   DrawScreenBoxHollow (x0 + entry1_out_x0, y0 + entry1_out_y0,
      x0 + entry1_out_x1, y0 + entry1_out_y1, BLACK);
   if (*v1 < 0 || *v1 > v1max)
      {
      set_colour (DARKGREY);
      ok = false;
      }
   else
      set_colour (LIGHTGREY);
   DrawScreenText (x0 + entry1_text_x0, y0 + entry1_text_y0, "%d", *v1);
   DrawScreenBoxHollow (x0 + entry2_out_x0, y0 + entry2_out_y0,
      x0 + entry2_out_x1, y0 + entry2_out_y1, BLACK);
   if (*v2 < 0 || *v2 > v2max)
      {
      set_colour (DARKGREY);
      ok = false;
      }
   else
      set_colour (LIGHTGREY);
   DrawScreenText (x0 + entry2_text_x0, y0 + entry2_text_y0, "%d", *v2);
   if (first)
      key = InputInteger (x0 + entry1_out_x0, y0 + entry1_out_y0, v1, 0, v1max);
   else
      key = InputInteger (x0 + entry2_out_x0, y0 + entry2_out_y0, v2, 0, v2max);
   if (key==YK_LEFT || key==YK_RIGHT || key==YK_TAB || key==YK_BACKTAB)
      first = !first;
   else if (key == YK_ESC)
      break;
   else if (key == YK_RETURN)
      {
      if (first)
	 first = 0;
      else if (ok)
	 break;
      else
	 Beep ();
      }
   else
      Beep ();
   }
return (key == YK_RETURN);
}



/*
   insert a standard object at given position
*/

void InsertStandardObject (int xpos, int ypos, int choice) /* SWAP! */
{
int sector;
int n;
int a, b;

/* are we inside a Sector? */
Objid o;
GetCurObject (o, OBJ_SECTORS, xpos, ypos);
sector = o.num;

/* !!!! Should also check for overlapping objects (sectors) !!!! */
switch (choice)
   {
   case 1:
      a = 256;
      b = 128;
      if (Input2Numbers (-1, -1, "Width", "Height", 2000, 2000, &a, &b))
	 {
	 if (a < 8)
	    a = 8;
	 if (b < 8)
	    b = 8;
	 xpos = xpos - a / 2;
	 ypos = ypos - b / 2;
	 InsertObject (OBJ_VERTICES, -1, xpos, ypos);
	 InsertObject (OBJ_VERTICES, -1, xpos + a, ypos);
	 InsertObject (OBJ_VERTICES, -1, xpos + a, ypos + b);
	 InsertObject (OBJ_VERTICES, -1, xpos, ypos + b);
	 if (sector < 0)
	    InsertObject (OBJ_SECTORS, -1, 0, 0);
	 for (n = 0; n < 4; n++)
	    {
	    InsertObject (OBJ_LINEDEFS, -1, 0, 0);
	    LineDefs[NumLineDefs - 1].sidedef1 = NumSideDefs;
	    InsertObject (OBJ_SIDEDEFS, -1, 0, 0);
	    if (sector >= 0)
	       SideDefs[NumSideDefs - 1].sector = sector;
	    }
	 ObjectsNeeded (OBJ_LINEDEFS, 0);
	 if (sector >= 0)
	    {
	    LineDefs[NumLineDefs - 4].start = NumVertices - 4;
	    LineDefs[NumLineDefs - 4].end = NumVertices - 3;
	    LineDefs[NumLineDefs - 3].start = NumVertices - 3;
	    LineDefs[NumLineDefs - 3].end = NumVertices - 2;
	    LineDefs[NumLineDefs - 2].start = NumVertices - 2;
	    LineDefs[NumLineDefs - 2].end = NumVertices - 1;
	    LineDefs[NumLineDefs - 1].start = NumVertices - 1;
	    LineDefs[NumLineDefs - 1].end = NumVertices - 4;
	    }
	 else
	    {
	    LineDefs[NumLineDefs - 4].start = NumVertices - 1;
	    LineDefs[NumLineDefs - 4].end = NumVertices - 2;
	    LineDefs[NumLineDefs - 3].start = NumVertices - 2;
	    LineDefs[NumLineDefs - 3].end = NumVertices - 3;
	    LineDefs[NumLineDefs - 2].start = NumVertices - 3;
	    LineDefs[NumLineDefs - 2].end = NumVertices - 4;
	    LineDefs[NumLineDefs - 1].start = NumVertices - 4;
	    LineDefs[NumLineDefs - 1].end = NumVertices - 1;
	    }
	 }
      break;
   case 2:
      a = 8;
      b = 128;
      if (Input2Numbers (-1, -1, "Number of sides", "Radius", 32, 2000, &a, &b))
	 {
	 if (a < 3)
	    a = 3;
	 if (b < 8)
	    b = 8;
	 InsertPolygonVertices (xpos, ypos, a, b);
	 if (sector < 0)
	    InsertObject (OBJ_SECTORS, -1, 0, 0);
	 for (n = 0; n < a; n++)
	    {
	    InsertObject (OBJ_LINEDEFS, -1, 0, 0);
	    LineDefs[NumLineDefs - 1].sidedef1 = NumSideDefs;
	    InsertObject (OBJ_SIDEDEFS, -1, 0, 0);
	    if (sector >= 0)
	       SideDefs[NumSideDefs - 1].sector = sector;
	    }
	 ObjectsNeeded (OBJ_LINEDEFS, 0);
	 if (sector >= 0)
	    {
	    LineDefs[NumLineDefs - 1].start = NumVertices - 1;
	    LineDefs[NumLineDefs - 1].end = NumVertices - a;
	    for (n = 2; n <= a; n++)
	       {
	       LineDefs[NumLineDefs - n].start = NumVertices - n;
	       LineDefs[NumLineDefs - n].end = NumVertices - n + 1;
	       }
	    }
	 else
	    {
	    LineDefs[NumLineDefs - 1].start = NumVertices - a;
	    LineDefs[NumLineDefs - 1].end = NumVertices - 1;
	    for (n = 2; n <= a; n++)
	       {
	       LineDefs[NumLineDefs - n].start = NumVertices - n + 1;
	       LineDefs[NumLineDefs - n].end = NumVertices - n;
	       }
	    }
	 }
      break;
   case 3:
   /*
      a = 6;
      b = 16;
      if (Input2Numbers (-1, -1, "Number of steps", "Step height", 32, 48, &a, &b))
	 {
	 if (a < 2)
	    a = 2;
	 ObjectsNeeded (OBJ_SECTORS, 0);
	 n = Sectors[sector].ceilh;
	 h = Sectors[sector].floorh;
	 if (a * b < n - h)
	    {
	    Beep ();
	    Notify (-1, -1, "The stairs are too high for this Sector", 0);
	    return;
	    }
	 xpos = xpos - 32;
	 ypos = ypos - 32 * a;
	 for (n = 0; n < a; n++)
	    {
	    InsertObject (OBJ_VERTICES, -1, xpos, ypos);
	    InsertObject (OBJ_VERTICES, -1, xpos + 64, ypos);
	    InsertObject (OBJ_VERTICES, -1, xpos + 64, ypos + 64);
	    InsertObject (OBJ_VERTICES, -1, xpos, ypos + 64);
	    ypos += 64;
	    InsertObject (OBJ_SECTORS, sector, 0, 0);
	    h += b;
	    Sectors[NumSectors - 1].floorh = h;

	    InsertObject (OBJ_LINEDEFS, -1, 0, 0);
	    LineDefs[NumLineDefs - 1].sidedef1 = NumSideDefs;
	    LineDefs[NumLineDefs - 1].sidedef2 = NumSideDefs + 1;
	    InsertObject (OBJ_SIDEDEFS, -1, 0, 0);
	    SideDefs[NumSideDefs - 1].sector = sector;
	    InsertObject (OBJ_SIDEDEFS, -1, 0, 0);

	    ObjectsNeeded (OBJ_LINEDEFS, 0);
	    LineDefs[NumLineDefs - 4].start = NumVertices - 4;
	    LineDefs[NumLineDefs - 4].end = NumVertices - 3;
	    LineDefs[NumLineDefs - 3].start = NumVertices - 3;
	    LineDefs[NumLineDefs - 3].end = NumVertices - 2;
	    LineDefs[NumLineDefs - 2].start = NumVertices - 2;
	    LineDefs[NumLineDefs - 2].end = NumVertices - 1;
	    LineDefs[NumLineDefs - 1].start = NumVertices - 1;
	    LineDefs[NumLineDefs - 1].end = NumVertices - 4;
	   }
	 }
      break;
   */
   case 4:
      NotImplemented ();
      break;
   }
}



/*
   menu of miscellaneous operations
*/

void MiscOperations (int objtype, SelPtr *list, int val) /* SWAP! */
{
char   msg[80];
int    angle, scale;

if (val > 1 && ! *list)
   {
   Beep ();
   sprintf (msg, "You must select at least one %s", GetObjectTypeName (objtype));
   Notify (-1, -1, msg, 0);
   return;
   }

/* I think this switch statement deserves a prize for "worst
   gratuitous obfuscation" or something. -- AYM 2000-11-07 */
switch (val)
   {
   case 1:
      // * -> First free tag number
      sprintf (msg, "First free tag number: %d", FindFreeTag ());
      Notify (-1, -1, msg, 0);
      break;

   case 2:
      // * -> Rotate and scale
      if ((objtype == OBJ_VERTICES) && ! (*list)->next)
	 {
	 Beep ();
	 sprintf (msg, "You must select more than one %s",
            GetObjectTypeName (objtype));
	 Notify (-1, -1, msg, 0);
	 return;
	 }
      if (objtype != OBJ_THINGS)
      {	angle = 0;
      	scale = 100;
      	if (Input2Numbers (-1, -1, "rotation angle (°)", "scale (%)",
       	  360, 1000, &angle, &scale))
		 RotateAndScaleObjects (objtype, *list, (double) angle * 0.0174533,
       	     (double) scale * 0.01);
      }else
      {	sprintf (msg, "First free TID: %d", FindFreeTID ());
      		Notify (-1, -1, msg, 0);
      		break;
      }
      break;

   case 3:
      // Linedef -> Split
      if (objtype == OBJ_LINEDEFS)
	 {
	 SplitLineDefs (*list);
	 }
      // Sector -> Make door from sector
      else if (objtype == OBJ_SECTORS)
	 {
	 if ((*list)->next)
	    {
	    Beep ();
	    Notify (-1, -1, "You must select exactly one sector", 0);
	    }
	 else
	    {
	    MakeDoorFromSector ((*list)->objnum);
	    }
	 }
      // Thing -> Spin 45° clockwise
      else if (objtype == OBJ_THINGS)
	 {
         spin_things (*list, -45);
	 }
      // Vertex -> Delete and join linedefs
      else if (objtype == OBJ_VERTICES)
	 {
	 DeleteVerticesJoinLineDefs (*list);
	 ForgetSelection (list);
	 }
      break;

   case 4:
      // Linedef -> Split linedefs and sector
      if (objtype == OBJ_LINEDEFS)
	 {
	 if (! (*list)->next || (*list)->next->next)
	    {
	    Beep ();
	    Notify (-1, -1, "You must select exactly two linedefs", 0);
	    }
	 else
	    {
	    SplitLineDefsAndSector ((*list)->next->objnum, (*list)->objnum);
	    ForgetSelection (list);
	    }
	 }
      // Sector -> Make lift from sector
      else if (objtype == OBJ_SECTORS)
	 {
	 if ((*list)->next)
	    {
	    Beep ();
	    Notify (-1, -1, "You must select exactly one Sector", 0);
	    }
	 else
	    {
	    MakeLiftFromSector ((*list)->objnum);
	    }
	 }
      // Thing -> Spin 45° counter-clockwise
      else if (objtype == OBJ_THINGS)
         spin_things (*list, 45);
      // Vertex -> Merge
      else if (objtype == OBJ_VERTICES)
	 {
	 MergeVertices (list);
	 }
      break;

   case 5:
      // Linedef -> Delete linedefs and join sectors
      if (objtype == OBJ_LINEDEFS)
	 {
	 DeleteLineDefsJoinSectors (list);
	 }
      // Sector -> Distribute sector floor heights
      else if (objtype == OBJ_SECTORS)
	 {
	 if (! (*list)->next || ! (*list)->next->next)
	    {
	    Beep ();
	    Notify (-1, -1, "You must select three or more sectors", 0);
	    }
	 else
	    {
	    DistributeSectorFloors (*list);
	    }
	 }
      // Thing -> Mirror horizontally
      else if (objtype == OBJ_THINGS)
         {
	 flip_mirror (*list, OBJ_THINGS, 'm');
	 }
      // Vertex -> Add linedef and split sector
      else if (objtype == OBJ_VERTICES)
	 {
	 if (! (*list)->next || (*list)->next->next)
	    {
	    Beep ();
	    Notify (-1, -1, "You must select exactly two vertices", 0);
	    }
	 else
	    {
	    SplitSector ((*list)->next->objnum, (*list)->objnum);
	    ForgetSelection (list);
	    }
	 }
      break;

   case 6:
      // Linedef -> Flip
      if (objtype == OBJ_LINEDEFS)
	 {
	 FlipLineDefs (*list, 1);
	 }
      // Sector -> Distribute ceiling heights
      else if (objtype == OBJ_SECTORS)
	 {
	 if (! (*list)->next || ! (*list)->next->next)
	    {
	    Beep ();
	    Notify (-1, -1, "You must select three or more sectors", 0);
	    }
	 else
	    {
	    DistributeSectorCeilings (*list);
	    }
	 }
      // Things -> Mirror vertically
      else if (objtype == OBJ_THINGS)
	 {
	 flip_mirror (*list, OBJ_THINGS, 'f');
         }
      // Vertex -> Mirror horizontally
      else if (objtype == OBJ_VERTICES)
	 {
	 flip_mirror (*list, OBJ_VERTICES, 'm');
	 }
      break;

   case 7:
      // Linedefs -> Swap sidedefs
      if (objtype == OBJ_LINEDEFS)
	 {
	 if (Expert
            || blindly_swap_sidedefs
            || Confirm (-1, -1,
               "Warning: the sector references are also swapped",
               "You may get strange results if you don't know what you are doing..."))
	    FlipLineDefs (*list, 0);
	 }
      // Sectors -> Raise or lower
      else if (objtype == OBJ_SECTORS)
	 {
	 RaiseOrLowerSectors (*list);
	 }
      // Vertices -> Mirror vertically
      else if (objtype == OBJ_VERTICES)
	 {
	 flip_mirror (*list, OBJ_VERTICES, 'f');
	 }
      break;

   case 8:
      // Linedef ->  Align textures vertically
      if (objtype == OBJ_LINEDEFS)
	 {
	 SelPtr sdlist, cur;

	 /* select all sidedefs */
	 ObjectsNeeded (OBJ_LINEDEFS);
	 sdlist = 0;
	 for (cur = *list; cur; cur = cur->next)
	    {
	    if (LineDefs[cur->objnum].sidedef1 >= 0)
	       SelectObject (&sdlist, LineDefs[cur->objnum].sidedef1);
	    if (LineDefs[cur->objnum].sidedef2 >= 0)
	       SelectObject (&sdlist, LineDefs[cur->objnum].sidedef2);
	    }
	 /* align the textures along the Y axis (height) */
	 AlignTexturesY (&sdlist);
	 }
      // Sector -> Brighten or darken
      else if (objtype == OBJ_SECTORS)
	 {
	 BrightenOrDarkenSectors (*list);
	 }
      break;

   case 9:
      // Linedef -> Align texture horizontally
      if (objtype == OBJ_LINEDEFS)
	 {
	 SelPtr sdlist, cur;

	 /* select all sidedefs */
	 ObjectsNeeded (OBJ_LINEDEFS,0);
	 sdlist = 0;
	 for (cur = *list; cur; cur = cur->next)
	    {
	    if (LineDefs[cur->objnum].sidedef1 >= 0)
	       SelectObject (&sdlist, LineDefs[cur->objnum].sidedef1);
	    if (LineDefs[cur->objnum].sidedef2 >= 0)
	       SelectObject (&sdlist, LineDefs[cur->objnum].sidedef2);
	    }
	 /* align the textures along the X axis (width) */
	 AlignTexturesX (&sdlist);
	 }
      // Sector -> Unlink room
      else if (objtype == OBJ_SECTORS)
	 {
	 NotImplemented ();  // FIXME
	 break;
	 }
      break;

   case 10:
      // Linedef -> Make linedef single-sided
      if (objtype == OBJ_LINEDEFS)
	 {
	 SelPtr cur;
	 ObjectsNeeded (OBJ_LINEDEFS, 0);
	 for (cur = *list; cur; cur = cur->next)
	    {
	    struct LineDef *l = LineDefs + cur->objnum;
	    l->sidedef2 = -1;  /* remove ref. to 2nd SD */
	    l->flags &= ~0x04; /* clear "2S" bit */
	    l->flags |= 0x01;  /* set "Im" bit */

	    if (is_sidedef (l->sidedef1))
	       {
	       struct SideDef *s = SideDefs + l->sidedef1;
	       strcpy (s->tex1, "-");
	       strcpy (s->tex2, "-");
	       strcpy (s->tex3, default_middle_texture);
	       }
	    /* Don't delete the 2nd sidedef, it could be used
               by another linedef. And if it isn't, the next
               cross-references check will delete it anyway. */
	    }
	 }
      // Sector -> Mirror horizontally
      else if (objtype == OBJ_SECTORS)
	 {
	 flip_mirror (*list, OBJ_SECTORS, 'm');
	 }
      break;

   case 11:
      // Linedef -> Make rectangular nook
      if (objtype == OBJ_LINEDEFS)
	 MakeRectangularNook (*list, 32, 16, 0);
      // Sector -> Mirror vertically
      else if (objtype == OBJ_SECTORS)
	 {
	 flip_mirror (*list, OBJ_SECTORS, 'f');
	 }
      break;

   case 12:
      // Linedef -> Make rectangular boss
      if (objtype == OBJ_LINEDEFS)
	 MakeRectangularNook (*list, 32, 16, 1);
      // Sector -> Swap flats
      else if (objtype == OBJ_SECTORS)
	 swap_flats (*list);
      break;

   case 13:
      // Linedef -> Set length (1st vertex)
      if (objtype == OBJ_LINEDEFS)
	 {
	 static int length = 24;
	 length = InputIntegerValue (-1, -1, -10000, 10000, length);
	 if (length != IIV_CANCEL)
	   SetLinedefLength (*list, length, 0);
	 }
      break;

   case 14:
      // Linedef -> Set length (2nd vertex)
      if (objtype == OBJ_LINEDEFS)
	 {
	 static int length = 24;
	 length = InputIntegerValue (-1, -1, -10000, 10000, length);
	 if (length != IIV_CANCEL)
	   SetLinedefLength (*list, length, 1);
	 }
      break;

   case 15:
      // Linedef -> Unlink 1st sidedef
      if (objtype == OBJ_LINEDEFS)
         unlink_sidedef (*list, 1, 0);
      break;

   case 16:
      // Linedef -> Unlink 2nd sidedef
      if (objtype == OBJ_LINEDEFS)
         unlink_sidedef (*list, 0, 1);
      break;

   case 17:
      // Linedef -> Mirror horizontally
      flip_mirror (*list, OBJ_LINEDEFS, 'm');
      break;
      
   case 18 :
      // Linedef -> Mirror vertically
      flip_mirror (*list, OBJ_LINEDEFS, 'f');
      break;

   case 19 :
      // Linedef -> Cut a slice out of a sector
      if (objtype == OBJ_LINEDEFS)
	 {
	 if (! (*list)->next || (*list)->next->next)
	    {
	    Beep ();
	    Notify (-1, -1, "You must select exactly two linedefs", 0);
	    }
	 else
	    {
	    sector_slice ((*list)->next->objnum, (*list)->objnum);
	    ForgetSelection (list);
	    }
	 }
      break;
   }
}



