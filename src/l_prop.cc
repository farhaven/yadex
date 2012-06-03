/*
 *	l_prop.c
 *	Linedefs properties
 *	Some of this was originally in editobj.c. It was moved here to
 *	improve overlay granularity (therefore memory consumption).
 *	AYM 1998-02-07
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
#include "entry.h"
#include "gfx.h"
#include "levels.h"
#include "menudata.h"
#include "objects.h"
#include "objid.h"
#include "oldmenus.h"
#include "game.h"
#include "selectn.h"
#include "textures.h"

#include "things.h"
/*
 *	Menu_data_ldt - Menu_data class for the linedef type
 */
class Menu_data_ldt : public Menu_data
{
  public :
    Menu_data_ldt (al_llist_t *list);
    virtual size_t nitems () const;
    virtual const char *operator[] (size_t n) const;

  private :
    mutable char buf[100];
    al_llist_t *list;
};


/*
 *	Menu_data_ldt::Menu_data_ldt - ctor
 */
Menu_data_ldt::Menu_data_ldt (al_llist_t *list) : list (list)
{
  al_lrewind (this->list);
}


/*
 *	Menu_data_ldt::nitems - return the number of items
 */
size_t Menu_data_ldt::nitems () const
{
  return al_lcount (list);
}


/*
 *	Menu_data_ldt::operator[] - return the nth item
 */
const char *Menu_data_ldt::operator[] (size_t n) const
{
  if (al_lseek (list, n, SEEK_SET) != 0)
  {
    snprintf (buf, sizeof(buf), "BUG: al_lseek(%p, %lu): %s",
      (void *) list, 
      (unsigned long) n,
      al_astrerror (al_aerrno));
    return buf;
  }
  const ldtdef_t **pptr = (const ldtdef_t **) al_lptr (list);
  if (pptr == NULL)
    snprintf (buf, sizeof(buf), "BUG: al_lptr(%p): %s",
      (void *) list,
      al_astrerror (al_aerrno));
  else
    snprintf (buf, sizeof(buf), "%3d - %.70s", (*pptr)->number, (*pptr)->longdesc);
  return buf;
}


/*
 *	Prototypes of private functions
 */
static char *GetTaggedLineDefFlag (int linedefnum, int flagndx);
static int InputLinedefType (int x0, int y0, int *number);
static const char *PrintLdtgroup (void *ptr);
static int InputThingType (int x0, int y0, int *number);
static const char *PrintThinggroup (void *ptr);
static const char *PrintThingdef (void *ptr);

void LinedefProperties (int x0, int y0, SelPtr obj)
{
  char  *menustr[12];
  char   texname[WAD_TEX_NAME + 1];
  int    n, val;
  SelPtr cur, sdlist;
  int objtype = OBJ_LINEDEFS;
  int    subwin_y0;
  int    subsubwin_y0;

  {
    bool sd1 = LineDefs[obj->objnum].sidedef1 >= 0;
    bool sd2 = LineDefs[obj->objnum].sidedef2 >= 0;
    val = vDisplayMenu (x0, y0, "Choose the object to edit:",
       "Edit the linedef",					YK_, 0,
       sd1 ? "Edit the 1st sidedef" : "Add a 1st sidedef",	YK_, 0,
       sd2 ? "Edit the 2nd sidedef" : "Add a 2nd sidedef",	YK_, 0,
       NULL);
  }
  subwin_y0 = y0 + BOX_BORDER + (2 + val) * FONTH;
  switch (val)
  {
    case 1:
      for (n = 0; n < 12; n++)
	menustr[n] = (char *) GetMemory (60);
      snprintf (menustr[11], 60, "Edit linedef #%d", obj->objnum);
      snprintf (menustr[0], 60, "Change flags            (Current: %d)",
	LineDefs[obj->objnum].flags);
      snprintf (menustr[1], 60, "Change type             (Current: %d)",
	LineDefs[obj->objnum].type);
      if (yg_level_format == YGLF_HEXEN) {
			snprintf (menustr[2], 60, "Change starting vertex  (Current: #%d)",
			LineDefs[obj->objnum].start);
		snprintf (menustr[3], 60, "Change ending vertex    (Current: #%d)",
			LineDefs[obj->objnum].end);
		snprintf (menustr[4], 60, "Change 1st sidedef ref. (Current: #%d)",
			LineDefs[obj->objnum].sidedef1);
      		snprintf (menustr[5], 60, "Change 2nd sidedef ref. (Current: #%d)",
			LineDefs[obj->objnum].sidedef2);
		snprintf (menustr[6], 60, "Change %-17s(Current: %d)",
			GetLineDefArgumentName(LineDefs[obj->objnum].type,1),LineDefs[obj->objnum].tag);
      		snprintf (menustr[7], 60, "Change %-17s(Current: %d)",
			GetLineDefArgumentName(LineDefs[obj->objnum].type,2),LineDefs[obj->objnum].arg2);
      		snprintf (menustr[8], 60, "Change %-17s(Current: %d)",
			GetLineDefArgumentName(LineDefs[obj->objnum].type,3),LineDefs[obj->objnum].arg3);
      		snprintf (menustr[9], 60, "Change %-17s(Current: %d)",
			GetLineDefArgumentName(LineDefs[obj->objnum].type,4),LineDefs[obj->objnum].arg4);
      		snprintf (menustr[10], 60, "Change %-17s(Current: %d)",
			GetLineDefArgumentName(LineDefs[obj->objnum].type,5),LineDefs[obj->objnum].arg5);
	} else {
		snprintf (menustr[2], 60, "Change sector tag       (Current: %d)",
			LineDefs[obj->objnum].tag);
      		snprintf (menustr[3], 60, "Change starting vertex  (Current: #%d)",
			LineDefs[obj->objnum].start);
      		snprintf (menustr[4], 60, "Change ending vertex    (Current: #%d)",
			LineDefs[obj->objnum].end);
	  	snprintf (menustr[5], 60, "Change 1st sidedef ref. (Current: #%d)",
			LineDefs[obj->objnum].sidedef1);
      		snprintf (menustr[6], 60, "Change 2nd sidedef ref. (Current: #%d)",
			LineDefs[obj->objnum].sidedef2);
      	}
	if (yg_level_format == YGLF_HEXEN)
         val = vDisplayMenu (x0 + 42, subwin_y0, menustr[11],
	menustr[0], YK_, 0,
	menustr[1], YK_, 0,
	menustr[2], YK_, 0,
	menustr[3], YK_, 0,
	menustr[4], YK_, 0,
	menustr[5], YK_, 0,
	menustr[6], YK_, 0,
	menustr[7], YK_, 0,
	menustr[8], YK_, 0,
	menustr[9], YK_, 0,
	menustr[10], YK_, 0,
	NULL);
	  else
         val = vDisplayMenu (x0 + 42, subwin_y0, menustr[11],
	menustr[0], YK_, 0,
	menustr[1], YK_, 0,
	menustr[2], YK_, 0,
	menustr[3], YK_, 0,
	menustr[4], YK_, 0,
	menustr[5], YK_, 0,
	menustr[6], YK_, 0,
	NULL);
      for (n = 0; n < 12; n++)
	FreeMemory (menustr[n]);
      subsubwin_y0 = subwin_y0 + BOX_BORDER + (2 + val) * FONTH;
      switch (val)
      {
	case 1:
	  val = vDisplayMenu (x0 + 84, subsubwin_y0, "Toggle the flags:",
	    GetTaggedLineDefFlag (obj->objnum, 1),  YK_, 0,
	    GetTaggedLineDefFlag (obj->objnum, 2),  YK_, 0,
	    GetTaggedLineDefFlag (obj->objnum, 3),  YK_, 0,
	    GetTaggedLineDefFlag (obj->objnum, 4),  YK_, 0,
	    GetTaggedLineDefFlag (obj->objnum, 5),  YK_, 0,
	    GetTaggedLineDefFlag (obj->objnum, 6),  YK_, 0,
	    GetTaggedLineDefFlag (obj->objnum, 7),  YK_, 0,
	    GetTaggedLineDefFlag (obj->objnum, 8),  YK_, 0,
	    GetTaggedLineDefFlag (obj->objnum, 9),  YK_, 0,
	    GetTaggedLineDefFlag (obj->objnum, 10), YK_, 0,
	    GetTaggedLineDefFlag (obj->objnum, 11), YK_, 0,
	    GetTaggedLineDefFlag (obj->objnum, 12), YK_, 0,
	    GetTaggedLineDefFlag (obj->objnum, 13), YK_, 0,
	    GetTaggedLineDefFlag (obj->objnum, 14), YK_, 0,
	    GetTaggedLineDefFlag (obj->objnum, 15), YK_, 0,
	    GetTaggedLineDefFlag (obj->objnum, 16), YK_, 0,
	    "(Enter a decimal value)", YK_, 0,
	    NULL);
	  if (val >= 1 && val <= 16)
	     {
	     for (cur = obj; cur; cur = cur->next)
		LineDefs[cur->objnum].flags ^= 0x01 << (val - 1);
	     MadeChanges = 1;
	     }
	  else if (val == 17)
	     {
	     val = InputIntegerValue (x0 + 126, subsubwin_y0 + 12 * FONTH,
		0, 65535, LineDefs[obj->objnum].flags);
	     if (val != IIV_CANCEL)
		{
		for (cur = obj; cur; cur = cur->next)
		   LineDefs[cur->objnum].flags = val;
		MadeChanges = 1;
		}
	     }
	  break;

	case 2:
	  if (! InputLinedefType (x0, subsubwin_y0, &val))
	  {
	    for (cur = obj; cur; cur = cur->next)
	      LineDefs[cur->objnum].type = val;
	    MadeChanges = 1;
	  }
	  break;

	case 3:
	  if (yg_level_format == YGLF_HEXEN)
	  {	val = InputObjectXRef (x0 + 84, subsubwin_y0,
	    	OBJ_VERTICES, 0, LineDefs[obj->objnum].start);
	  	if (val >= 0)
	  	{
	    		for (cur = obj; cur; cur = cur->next)
	      		LineDefs[cur->objnum].start = val;
	    		MadeChanges = 1;
	    		MadeMapChanges = 1;
	  	}
	  }
	  else
	  {
	   	val = InputIntegerValue (x0 + 84, subsubwin_y0,
	    		-32768, 32767, LineDefs[obj->objnum].tag);
	  	if (val != IIV_CANCEL)  // Not [esc]
	  	{
	    		for (cur = obj; cur; cur = cur->next)
	      		LineDefs[cur->objnum].tag = val;
	    		MadeChanges = 1;
	  	}
	  }
	  break;

	case 4:
		if (yg_level_format == YGLF_HEXEN)
		{	val = InputObjectXRef (x0 + 84, subsubwin_y0,
	    		OBJ_VERTICES, 0, LineDefs[obj->objnum].end);
	  		if (val >= 0)
	  		{
	    			for (cur = obj; cur; cur = cur->next)
	      			LineDefs[cur->objnum].end = val;
	    			MadeChanges = 1;
	    			MadeMapChanges = 1;
	  		}
		}else  
		{	val = InputObjectXRef (x0 + 84, subsubwin_y0,
	    		OBJ_VERTICES, 0, LineDefs[obj->objnum].start);
	  		if (val >= 0)
	  		{
	    			for (cur = obj; cur; cur = cur->next)
	      			LineDefs[cur->objnum].start = val;
	    			MadeChanges = 1;
	    			MadeMapChanges = 1;
	  		}
		}
	  	break;

	case 5:
		if (yg_level_format == YGLF_HEXEN)
		{	val = InputObjectXRef (x0 + 84, subsubwin_y0,
	    		OBJ_SIDEDEFS, 1, LineDefs[obj->objnum].sidedef1);
	  		if (val >= -1)
	  		{
	    		for (cur = obj; cur; cur = cur->next)
	      		LineDefs[cur->objnum].sidedef1 = val;
	    		MadeChanges = 1;
	    		MadeMapChanges = 1;
	  		}
		}else
		{	val = InputObjectXRef (x0 + 84, subsubwin_y0,
	    		OBJ_VERTICES, 0, LineDefs[obj->objnum].end);
	  		if (val >= 0)
	  		{
	    			for (cur = obj; cur; cur = cur->next)
	      			LineDefs[cur->objnum].end = val;
	    			MadeChanges = 1;
	    			MadeMapChanges = 1;
	  		}
		}
	  break;

	case 6:
		if (yg_level_format == YGLF_HEXEN)
		{	val = InputObjectXRef (x0 + 84, subsubwin_y0,
	    		OBJ_SIDEDEFS, 1, LineDefs[obj->objnum].sidedef2);
	  		if (val >= -1)
	  		{
	    			for (cur = obj; cur; cur = cur->next)
	      			LineDefs[cur->objnum].sidedef2 = val;
	    			MadeChanges = 1;
	    			MadeMapChanges = 1;
	  		}
		}else
		{	val = InputObjectXRef (x0 + 84, subsubwin_y0,
	    		OBJ_SIDEDEFS, 1, LineDefs[obj->objnum].sidedef1);
	  		if (val >= -1)
	  		{
	    			for (cur = obj; cur; cur = cur->next)
	      			LineDefs[cur->objnum].sidedef1 = val;
	    			MadeChanges = 1;
	    			MadeMapChanges = 1;
	  		}
		}
	  	break;

	case 7:
		if (yg_level_format == YGLF_HEXEN)
		{	val = InputIntegerValue (x0 + 84, subsubwin_y0,
				0,255, LineDefs[obj->objnum].tag);
			if (val != IIV_CANCEL)
			{	for (cur = obj; cur; cur = cur->next)
				LineDefs[cur->objnum].tag = val;
				MadeChanges = 1;
			}
		}else
		{	val = InputObjectXRef (x0 + 84, subsubwin_y0,
	    		OBJ_SIDEDEFS, 1, LineDefs[obj->objnum].sidedef2);
	  		if (val >= -1)
	  		{
	    			for (cur = obj; cur; cur = cur->next)
	      			LineDefs[cur->objnum].sidedef2 = val;
	    			MadeChanges = 1;
	    			MadeMapChanges = 1;
	  		}
		}
	  break;

	case 8:
	  val = InputIntegerValue (x0 + 84, subsubwin_y0,
	    0, 255, LineDefs[obj->objnum].arg2);
	  if (val != IIV_CANCEL)  // Not [esc]
	  {
	    for (cur = obj; cur; cur = cur->next)
	      LineDefs[cur->objnum].arg2 = val;
	    MadeChanges = 1;
	  }
	  break;

	case 9:
	  val = InputIntegerValue (x0 + 84, subsubwin_y0,
	    0, 255, LineDefs[obj->objnum].arg3);
	  if (val != IIV_CANCEL)  // Not [esc]
	  {
	    for (cur = obj; cur; cur = cur->next)
	      LineDefs[cur->objnum].arg3 = val;
	    MadeChanges = 1;
	  }
	  break;

	case 10:
	  val = InputIntegerValue (x0 + 84, subsubwin_y0,
	    0, 255, LineDefs[obj->objnum].arg4);
	  if (val != IIV_CANCEL)  // Not [esc]
	  {
	    for (cur = obj; cur; cur = cur->next)
	      LineDefs[cur->objnum].arg4 = val;
	    MadeChanges = 1;
	  }
	  break;
	case 11:
	  val = InputIntegerValue (x0 + 84, subsubwin_y0,
	    0, 255, LineDefs[obj->objnum].arg5);
	  if (val != IIV_CANCEL)
	  {
		  for (cur = obj;cur;cur = cur->next)
			  LineDefs[cur->objnum].arg5 = val;
		  MadeChanges = 1;
	  }
	  break;
     }
     break;

    // Edit or add the first sidedef
    case 2:
      ObjectsNeeded (OBJ_LINEDEFS, OBJ_SIDEDEFS, 0);
      if (LineDefs[obj->objnum].sidedef1 >= 0)
      {
	// Build a new selection list with the first sidedefs
	objtype = OBJ_SIDEDEFS;
	sdlist = 0;
	for (cur = obj; cur; cur = cur->next)
	  if (LineDefs[cur->objnum].sidedef1 >= 0)
	    SelectObject (&sdlist, LineDefs[cur->objnum].sidedef1);
      }
      else
      {
	// Add a new first sidedef
	for (cur = obj; cur; cur = cur->next)
	  if (LineDefs[cur->objnum].sidedef1 == -1)
	  {
	    InsertObject (OBJ_SIDEDEFS, -1, 0, 0);
	    LineDefs[cur->objnum].sidedef1 = NumSideDefs - 1;
	  }
	break;
      }
      // FALL THROUGH

    // Edit or add the second sidedef
    case 3:
      if (objtype != OBJ_SIDEDEFS)
      {
	if (LineDefs[obj->objnum].sidedef2 >= 0)
	{
	  // Build a new selection list with the second (or first) SideDefs
	  objtype = OBJ_SIDEDEFS;
	  sdlist = 0;
	  for (cur = obj; cur; cur = cur->next)
	    if (LineDefs[cur->objnum].sidedef2 >= 0)
	      SelectObject (&sdlist, LineDefs[cur->objnum].sidedef2);
	    else if (LineDefs[cur->objnum].sidedef1 >= 0)
	      SelectObject (&sdlist, LineDefs[cur->objnum].sidedef1);
	}
	else
	{
	  // Add a new second (or first) sidedef
	  for (cur = obj; cur; cur = cur->next)
	    if (LineDefs[cur->objnum].sidedef1 == -1)
	    {
	      InsertObject (OBJ_SIDEDEFS, -1, 0, 0);
	      ObjectsNeeded (OBJ_LINEDEFS, 0);
	      LineDefs[cur->objnum].sidedef1 = NumSideDefs - 1;
	    }
	    else if (LineDefs[cur->objnum].sidedef2 == -1)
	    {
	      n = LineDefs[cur->objnum].sidedef1;
	      InsertObject (OBJ_SIDEDEFS, -1, 0, 0);
	      strncpy (SideDefs[NumSideDefs - 1].tex3, "-", WAD_TEX_NAME);
	      strncpy (SideDefs[n].tex3, "-", WAD_TEX_NAME);
	      ObjectsNeeded (OBJ_LINEDEFS, 0);
	      LineDefs[cur->objnum].sidedef2 = NumSideDefs - 1;
	      LineDefs[cur->objnum].flags ^= 4;  // Set the 2S bit
	      LineDefs[cur->objnum].flags &= ~1;  // Clear the Im bit
	    }
	  break;
	}
      }
      ObjectsNeeded (OBJ_SIDEDEFS, 0);
      for (n = 0; n < 7; n++)
	menustr[n] = (char *) GetMemory (60);
      snprintf (menustr[6], 60, "Edit sidedef #%d", sdlist->objnum);
      texname[WAD_TEX_NAME] = '\0';
      strncpy (texname, SideDefs[sdlist->objnum].tex3, WAD_TEX_NAME);
      snprintf (menustr[0], 60, "Change middle texture   (Current: %s)", texname);
      strncpy (texname, SideDefs[sdlist->objnum].tex1, WAD_TEX_NAME);
      snprintf (menustr[1], 60, "Change upper texture    (Current: %s)", texname);
      strncpy (texname, SideDefs[sdlist->objnum].tex2, WAD_TEX_NAME);
      snprintf (menustr[2], 60, "Change lower texture    (Current: %s)", texname);
      snprintf (menustr[3], 60, "Change texture X offset (Current: %d)",
	SideDefs[sdlist->objnum].xoff);
      snprintf (menustr[4], 60, "Change texture Y offset (Current: %d)",
	SideDefs[sdlist->objnum].yoff);
      snprintf (menustr[5], 60, "Change sector ref.      (Current: #%d)",
	SideDefs[sdlist->objnum].sector);
      val = vDisplayMenu (x0 + 42, subwin_y0, menustr[6],
	menustr[0], YK_, 0,
	menustr[1], YK_, 0,
	menustr[2], YK_, 0,
	menustr[3], YK_, 0,
	menustr[4], YK_, 0,
	menustr[5], YK_, 0,
	NULL);
      for (n = 0; n < 7; n++)
	FreeMemory (menustr[n]);
      subsubwin_y0 = subwin_y0 + BOX_BORDER + (2 + val) * FONTH;
      switch (val)
      {
	case 1:
	  strncpy (texname, SideDefs[sdlist->objnum].tex3, WAD_TEX_NAME);
	  ObjectsNeeded (0);
	  ChooseWallTexture (x0 + 84, subsubwin_y0 ,
	    "Choose a wall texture", NumWTexture, WTexture, texname);
	  ObjectsNeeded (OBJ_SIDEDEFS, 0);
	  if (strlen (texname) > 0)
	  {
	    for (cur = sdlist; cur; cur = cur->next)
	      if (cur->objnum >= 0)
		strncpy (SideDefs[cur->objnum].tex3, texname, WAD_TEX_NAME);
	    MadeChanges = 1;
	  }
	  break;

	case 2:
	  strncpy (texname, SideDefs[sdlist->objnum].tex1, WAD_TEX_NAME);
	  ObjectsNeeded (0);
	  ChooseWallTexture (x0 + 84, subsubwin_y0,
	     "Choose a wall texture", NumWTexture, WTexture, texname);
	  ObjectsNeeded (OBJ_SIDEDEFS, 0);
	  if (strlen (texname) > 0)
	  {
	    for (cur = sdlist; cur; cur = cur->next)
	      if (cur->objnum >= 0)
		strncpy (SideDefs[cur->objnum].tex1, texname, WAD_TEX_NAME);
	    MadeChanges = 1;
	  }
	  break;

	case 3:
	  strncpy (texname, SideDefs[sdlist->objnum].tex2, WAD_TEX_NAME);
	  ObjectsNeeded (0);
	  ChooseWallTexture (x0 + 84, subsubwin_y0,
	    "Choose a wall texture", NumWTexture, WTexture, texname);
	  ObjectsNeeded (OBJ_SIDEDEFS, 0);
	  if (strlen (texname) > 0)
	  {
	    for (cur = sdlist; cur; cur = cur->next)
	      if (cur->objnum >= 0)
		strncpy (SideDefs[cur->objnum].tex2, texname, WAD_TEX_NAME);
	    MadeChanges = 1;
	  }
	  break;

	case 4:
	  val = InputIntegerValue (x0 + 84, subsubwin_y0,
	    -32768, 32767, SideDefs[sdlist->objnum].xoff);
	  if (val != IIV_CANCEL)
	  {
	    for (cur = sdlist; cur; cur = cur->next)
	      if (cur->objnum >= 0)
		SideDefs[cur->objnum].xoff = val;
	    MadeChanges = 1;
	  }
	  break;

	case 5:
	  val = InputIntegerValue (x0 + 84, subsubwin_y0,
	    -32768, 32767, SideDefs[sdlist->objnum].yoff);
	  if (val != IIV_CANCEL)
	  {
	    for (cur = sdlist; cur; cur = cur->next)
	      if (cur->objnum >= 0)
		SideDefs[cur->objnum].yoff = val;
	    MadeChanges = 1;
	  }
	  break;

	case 6:
	  val = InputObjectXRef (x0 + 84, subsubwin_y0,
	    OBJ_SECTORS, 0, SideDefs[sdlist->objnum].sector);
	  if (val >= 0)
	  {
	    for (cur = sdlist; cur; cur = cur->next)
	      if (cur->objnum >= 0)
		SideDefs[cur->objnum].sector = val;
	    MadeChanges = 1;
	  }
	  break;
      }
      ForgetSelection (&sdlist);
      break;
  }
}

/*
 *	ThingProperties
 *	Thing properties dialog. Called by EditObjectsInfo. Was part of
 *	EditObjectsInfo in editobj.c
 */
void ThingProperties (int x0, int y0, SelPtr obj)
{
char  *menustr[30];
int    n, val;
SelPtr cur;
int    subwin_y0;

for (n = 0; n < 14; n++)
   menustr[n] = (char *) GetMemory (60);
snprintf (menustr[13], 60,"Edit thing #%d", obj->objnum);
snprintf (menustr[0], 60,"Change type          (Current: %s)",
         get_thing_name (Things[obj->objnum].type));
snprintf (menustr[1], 60,"Change angle         (Current: %s)",
         GetAngleName (Things[obj->objnum].angle));
snprintf (menustr[2], 60,"Change flags         (Current: %s)",
         GetWhenName (Things[obj->objnum].when));
snprintf (menustr[3], 60,"Change X position    (Current: %d)",
         Things[obj->objnum].xpos);
snprintf (menustr[4], 60,"Change Y position    (Current: %d)",
         Things[obj->objnum].ypos);
snprintf (menustr[5], 60,"Change Z position    (Current: %d)",
         Things[obj->objnum].height);
snprintf (menustr[6], 60,"Change TID           (Current: %d)",
         Things[obj->objnum].tid);
snprintf (menustr[7], 60,"Change special       (Current: %d)",
         Things[obj->objnum].special);
if (yg_level_format == YGLF_HEXEN) {
	snprintf (menustr[8], 60,"Change %-14s(Current: %d)",
        	GetLineDefArgumentName(Things[obj->objnum].special,1),Things[obj->objnum].arg1);
	snprintf (menustr[9], 60,"Change %-14s(Current: %d)",
         	GetLineDefArgumentName(Things[obj->objnum].special,2),Things[obj->objnum].arg2);
	snprintf (menustr[10], 60,"Change %-14s(Current: %d)",
         	GetLineDefArgumentName(Things[obj->objnum].special,3),Things[obj->objnum].arg3);
	snprintf (menustr[11], 60,"Change %-14s(Current: %d)",
         	GetLineDefArgumentName(Things[obj->objnum].special,4),Things[obj->objnum].arg4);
	snprintf (menustr[12], 60,"Change %-14s(Current: %d)",
         	GetLineDefArgumentName(Things[obj->objnum].special,5),Things[obj->objnum].arg5);
}
if (yg_level_format == YGLF_HEXEN)		// Hexen mode
val = vDisplayMenu (x0, y0, menustr[13],
   menustr[0], YK_, 0,
   menustr[1], YK_, 0,
   menustr[2], YK_, 0,
   menustr[3], YK_, 0,
   menustr[4], YK_, 0,
   menustr[5], YK_, 0,
   menustr[6], YK_, 0,
   menustr[7], YK_, 0,
   menustr[8], YK_, 0,
   menustr[9], YK_, 0,
   menustr[10], YK_, 0,
   menustr[11], YK_, 0,
   menustr[12], YK_, 0,
   NULL);
else
val = vDisplayMenu (x0, y0, menustr[13],
   menustr[0], YK_, 0,
   menustr[1], YK_, 0,
   menustr[2], YK_, 0,
   menustr[3], YK_, 0,
   menustr[4], YK_, 0,
   NULL);
for (n = 0; n < 14; n++)
   FreeMemory (menustr[n]);
subwin_y0 = y0 + BOX_BORDER + (2 + val) * FONTH;
switch (val)
  {
  case 1:
     if (! InputThingType (x0, subwin_y0, &val))
	{
	for (cur = obj; cur; cur = cur->next)
	   Things[cur->objnum].type = val;
	things_types++;
	MadeChanges = 1;
	}
     break;

  case 2:
     	if (yg_level_format == YGLF_HEXEN)
	{	val = vDisplayMenu (x0 + 42, subwin_y0, "Select angle",
			  "North",	YK_, 0,
			  "NorthEast",	YK_, 0,
			  "East",	YK_, 0,
			  "SouthEast",	YK_, 0,
			  "South",	YK_, 0,
			  "SouthWest",	YK_, 0,
			  "West",	YK_, 0,
			  "NorthWest",	YK_, 0,
			  "Input Number", YK_, 0,
			  NULL);
	}else
	{	val = vDisplayMenu (x0 + 42, subwin_y0, "Select angle",
			  "North",	YK_, 0,
			  "NorthEast",	YK_, 0,
			  "East",	YK_, 0,
			  "SouthEast",	YK_, 0,
			  "South",	YK_, 0,
			  "SouthWest",	YK_, 0,
			  "West",	YK_, 0,
			  "NorthWest",	YK_, 0,
			  NULL);
	}
	switch (val)
	{
	case 1:
	   for (cur = obj; cur; cur = cur->next)
	      Things[cur->objnum].angle = 90;
	   things_angles++;
	   MadeChanges = 1;
	   break;

	case 2:
	   for (cur = obj; cur; cur = cur->next)
	      Things[cur->objnum].angle = 45;
	   things_angles++;
	   MadeChanges = 1;
	   break;

	case 3:
	   for (cur = obj; cur; cur = cur->next)
	      Things[cur->objnum].angle = 0;
	   things_angles++;
	   MadeChanges = 1;
	   break;

	case 4:
	   for (cur = obj; cur; cur = cur->next)
	      Things[cur->objnum].angle = 315;
	   things_angles++;
	   MadeChanges = 1;
	   break;

	case 5:
	   for (cur = obj; cur; cur = cur->next)
	      Things[cur->objnum].angle = 270;
	   things_angles++;
	   MadeChanges = 1;
	   break;

	case 6:
	   for (cur = obj; cur; cur = cur->next)
	      Things[cur->objnum].angle = 225;
	   things_angles++;
	   MadeChanges = 1;
	   break;

	case 7:
	   for (cur = obj; cur; cur = cur->next)
	      Things[cur->objnum].angle = 180;
	   things_angles++;
	   MadeChanges = 1;
	   break;

	case 8:
	   for (cur = obj; cur; cur = cur->next)
	      Things[cur->objnum].angle = 135;
	   things_angles++;
	   MadeChanges = 1;
	   break;
        case 9:
		if (yg_level_format == YGLF_HEXEN)
		{	val = InputIntegerValue (x0 + 84,subwin_y0 + BOX_BORDER + (3 + val) * FONTH, 0, 255,Things[obj->objnum].angle);
	   		if (val != IIV_CANCEL)
	      		{	for (cur = obj; cur; cur = cur->next)
		 		Things[cur->objnum].angle = val;
	      			MadeChanges = 1;
	      		}
		}
		break;
	}
     break;

  case 3:
     if (yg_level_format == YGLF_HEXEN)
	     val = vDisplayMenu(x0+42,subwin_y0,"Set Object Flags",
			"D12        (Easy only)",		YK_,0,
			"D3         (Medium only)",		YK_,0,
			"D12,D3     (Easy and Medium)",		YK_,0,
			"D45        (Hard only)",		YK_,0,
			"D12,D45    (Easy and Hard)",		YK_,0,
			"D3,D45     (Medium and Hard)",		YK_,0,
			"D12,D3,D45 (Easy,Medium and Hard)",	YK_,0,
			"Toggle \"Deaf/Ambush\" bit",		YK_,0,
			"Toggle \"Asleep\" bit",		YK_,0,
			"S          (Thing is in Single Player)",	YK_,0,
			"C          (Thing is in Co-Op)",	YK_,0,
			"D          (Thing is in Deathmatch)",	YK_,0,
			"T          (Thing is translucent)",	YK_,0,
			"I          (Thing is invisible)",	YK_,0,
			"F          (Monster is friendly)",	YK_,0,
			"(Enter number)",			YK_,0,
			NULL);
     else
     	val = vDisplayMenu (x0 + 42, subwin_y0, "Set Object Flags",
			"D12          (Easy only)",		YK_, 0,
			"D3           (Medium only)",		YK_, 0,
			"D12, D3      (Easy and Medium)",	YK_, 0,
			"D45          (Hard only)",		YK_, 0,
			"D12, D45     (Easy and Hard)",		YK_, 0,
			"D3, D45      (Medium and Hard)",	YK_, 0,
			"D12, D3, D45 (Easy, Medium, Hard)",	YK_, 0,
			"Toggle \"Deaf/Ambush\" bit",		YK_, 0,
			"Toggle \"Multi-player only\" bit",	YK_, 0,
			"(Enter number)",			YK_, 0,
			NULL);
     switch (val)
	{
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	   for (cur = obj; cur; cur = cur->next)
	      Things[cur->objnum].when = (Things[cur->objnum].when & 0x18) | val;
	   MadeChanges = 1;
	   break;

	case 8:
	   for (cur = obj; cur; cur = cur->next)
	      Things[cur->objnum].when ^= 0x08;
	   MadeChanges = 1;
	   break;

	case 9:
	   for (cur = obj; cur; cur = cur->next)
	      Things[cur->objnum].when ^= 0x10;
	   MadeChanges = 1;
	   break;

	case 10:
	   if (yg_level_format != YGLF_HEXEN)
	   {	val = InputIntegerValue (x0 + 84,subwin_y0 + BOX_BORDER + (3 + val) * FONTH, 0, 65535,Things[obj->objnum].when);
	   	if (val != IIV_CANCEL)
	      	{	for (cur = obj; cur; cur = cur->next)
		 		Things[cur->objnum].when = val;
	      		MadeChanges = 1;
	      	}
	   }else
	   {	for (cur = obj;cur;cur = cur->next)
		   Things[cur->objnum].when ^= 0x100;
		MadeChanges = 1;
	   }
	   break;
	case 11:
	   if (yg_level_format == YGLF_HEXEN)
	   {	for (cur = obj;cur;cur = cur->next)
		   Things[cur->objnum].when ^= 0x200;
		MadeChanges = 1;
	   }
	   break;
	case 12:
	   if (yg_level_format == YGLF_HEXEN)
	   {	for (cur = obj;cur;cur = cur->next)
		   Things[cur->objnum].when ^= 0x400;
		MadeChanges = 1;
	   }
	   break;
	case 13:
	   if (yg_level_format == YGLF_HEXEN)
	   {	for (cur = obj;cur;cur = cur->next)
		   Things[cur->objnum].when ^= 0x800;
		MadeChanges = 1;
	   }
	   break;
	case 14:
	   if (yg_level_format == YGLF_HEXEN)
	   {	for (cur = obj;cur;cur = cur->next)
		   Things[cur->objnum].when ^= 0x1000;
		MadeChanges = 1;
	   }
	   break;
	case 15:
	   if (yg_level_format == YGLF_HEXEN)
	   {	for (cur = obj;cur;cur = cur->next)
		   Things[cur->objnum].when ^= 0x2000;
		MadeChanges = 1;
	   }
	   break;

	}
     break;

  case 4:
     val = InputIntegerValue (x0 + 42, subwin_y0, MapMinX, MapMaxX,
                              Things[obj->objnum].xpos);
     if (val != IIV_CANCEL)
        {
	n = val - Things[obj->objnum].xpos;
	for (cur = obj; cur; cur = cur->next)
	   Things[cur->objnum].xpos += n;
	MadeChanges = 1;
        }
     break;

  case 5:
     val = InputIntegerValue (x0 + 42, subwin_y0, MapMinY, MapMaxY,
                              Things[obj->objnum].ypos);
     if (val != IIV_CANCEL)
        {
	n = val - Things[obj->objnum].ypos;
	for (cur = obj; cur; cur = cur->next)
	   Things[cur->objnum].ypos += n;
	MadeChanges = 1;
        }
     break;

  case 6:
     val = InputIntegerValue (x0 + 42, subwin_y0, -32768, 32767,
                              Things[obj->objnum].height);
     if (val != IIV_CANCEL)
        {
	n = val - Things[obj->objnum].height;
	for (cur = obj; cur; cur = cur->next)
	   Things[cur->objnum].height += n;
	MadeChanges = 1;
        }
     break;

  case 7:
     val = InputIntegerValue (x0 + 42, subwin_y0, -32768, 32767,
                              Things[obj->objnum].tid);
     if (val != IIV_CANCEL)
        {
	for (cur = obj; cur; cur = cur->next)
	   Things[cur->objnum].tid = val;
	MadeChanges = 1;
        }
     break;

  case 8:
     if (! InputLinedefType (x0 + 42, subwin_y0, &val))
        {
	for (cur = obj; cur; cur = cur->next)
	   Things[cur->objnum].special = val;
	MadeChanges = 1;
        }
     break;

  case 9:
     val = InputIntegerValue (x0 + 42, subwin_y0, 0, 255,
                              Things[obj->objnum].arg1);
     if (val != IIV_CANCEL)
        {
	for (cur = obj; cur; cur = cur->next)
	   Things[cur->objnum].arg1 = val;
	MadeChanges = 1;
        }
     break;

  case 10:
     val = InputIntegerValue (x0 + 42, subwin_y0, 0, 255,
                              Things[obj->objnum].arg2);
     if (val != IIV_CANCEL)
        {
	for (cur = obj; cur; cur = cur->next)
	   Things[cur->objnum].arg2 = val;
	MadeChanges = 1;
        }
     break;

  case 11:
     val = InputIntegerValue (x0 + 42, subwin_y0, 0, 255,
                              Things[obj->objnum].arg3);
     if (val != IIV_CANCEL)
        {
	for (cur = obj; cur; cur = cur->next)
	   Things[cur->objnum].arg3 = val;
	MadeChanges = 1;
        }
     break;
  case 12:
     val = InputIntegerValue (x0 + 42, subwin_y0, 0, 255,
                              Things[obj->objnum].arg4);
     if (val != IIV_CANCEL)
        {
	for (cur = obj; cur; cur = cur->next)
	   Things[cur->objnum].arg4 = val;
	MadeChanges = 1;
        }
     break;

  case 13:
     val = InputIntegerValue (x0 + 42, subwin_y0, 0, 255,
                              Things[obj->objnum].arg5);
     if (val != IIV_CANCEL)
        {
	for (cur = obj; cur; cur = cur->next)
	   Things[cur->objnum].arg5 = val;
	MadeChanges = 1;
        }
     break;
  }
}

/*
 *	InputThingType
 *	Let the user select a thing number and return it.
 *	Returns 0 if OK, <>0 if cancelled
 */
static int InputThingType (int x0, int y0, int *number)
{
int         r;
int         tgno = 0;
char        tg; 
al_llist_t *list = NULL;

for (;;)
   {
   /* First let user select a thinggroup */
   if (DisplayMenuList (x0+42, y0, "Select group", thinggroup,
    PrintThinggroup, &tgno) < 0)
      return 1;
   if (al_lseek (thinggroup, tgno, SEEK_SET))
      fatal_error ("%s ITT1 (%s)", msg_unexpected, al_astrerror (al_aerrno));
   tg = CUR_THINGGROUP->thinggroup;

   /* KLUDGE: Special thinggroup THING_FREE means "enter number".
      Don't look for this thinggroup in the .ygd file : LoadGameDefs()
      creates it manually. */
   if (tg == THING_FREE)
      {
      /* FIXME should be unsigned! should accept hex. */
      *number = InputIntegerValue (x0+84, y0 + BOX_BORDER + (3 + tgno) * FONTH,
	 -32768, 32767, 0);
      if (*number != IIV_CANCEL)
	 break;
      goto again;
      }
     
   /* Then build a list of pointers on all things that have this
      thinggroup and let user select one. */
   list = al_lcreate (sizeof (void *));
   for (al_lrewind (thingdef); ! al_leol (thingdef); al_lstep (thingdef))
      if (CUR_THINGDEF->thinggroup == tg)
	 {
	 void *ptr = CUR_THINGDEF;
	 al_lwrite (list, &ptr);
	 }
   r = DisplayMenuList (x0+84, y0 + BOX_BORDER + (3 + tgno) * FONTH,
      "Select thing", list, PrintThingdef, NULL);
   if (r < 0)
      goto again;
   if (al_lseek (list, r, SEEK_SET))
      fatal_error ("%s ITT2 (%s)", msg_unexpected, al_astrerror (al_aerrno));
   *number = (*((thingdef_t **) al_lptr (list)))->number;
   al_ldiscard (list);
   break;

   again :
   ;
   /* DrawMap (OBJ_THINGS, 0, 0); FIXME! */
   }
return 0;
}

/*
 *	PrintThinggroup
 *	Used by DisplayMenuList when called by InputThingType
 */
static const char *PrintThinggroup (void *ptr)
{
if (ptr == NULL)
   return "PrintThinggroup: (null)";
return ((thinggroup_t *)ptr)->desc;
}

/*
 *	PrintThingdef
 *	Used by DisplayMenuList when called by InputThingType
 */
static const char *PrintThingdef (void *ptr)
{
if (ptr == NULL)
   return "PrintThingdef: (null)";
return (*((thingdef_t **)ptr))->desc;
}

/*
*/

static char *GetTaggedLineDefFlag (int linedefnum, int flagndx)
{
  static char ldstr[16][50];

  if ((LineDefs[linedefnum].flags & (0x01 << (flagndx - 1))) != 0)
    strcpy (ldstr[flagndx - 1], "* ");
  else
    strcpy (ldstr[flagndx - 1], "  ");
  strcat (ldstr[flagndx - 1], GetLineDefFlagsLongName (0x01 << (flagndx - 1)));
  return ldstr[flagndx - 1];
}



/*
 *	InputLinedefType
 *	Let the user select a linedef type number and return it.
 *	Returns 0 if OK, <>0 if cancelled
 */
static int InputLinedefType (int x0, int y0, int *number)
{
  int         r;
  int         ldtgno = 0;
  char        ldtg; 
  al_llist_t *list = 0;

  for (;;)
  {
    /* First let user select a ldtgroup */
    if (DisplayMenuList (x0+84, y0, "Select group", ldtgroup,
     PrintLdtgroup, &ldtgno) < 0)
      return 1;
    if (al_lseek (ldtgroup, ldtgno, SEEK_SET))
      fatal_error ("%s ILT1 (%s)", msg_unexpected, al_astrerror (al_aerrno));
    ldtg = CUR_LDTGROUP->ldtgroup;

    /* KLUDGE: Special ldtgroup LDT_FREE means "enter number"
       Don't look for this ldtgroup in the .ygd file :
       LoadGameDefs() creates it manually. */
    if (ldtg == LDT_FREE)
    {
      // FIXME should be unsigned
      *number = InputIntegerValue (x0+126, y0 + (3 + ldtgno) * FONTH,
	 -32768, 32767, 0);
      if (*number != IIV_CANCEL)
	break;
      goto again;
    }
      
    /* Then build a list of pointers on all ldt that have this
       ldtgroup and let user select one */
    list = al_lcreate (sizeof (void *));
    for (al_lrewind (ldtdef); ! al_leol (ldtdef); al_lstep (ldtdef))
      if (CUR_LDTDEF->ldtgroup == ldtg)
      {
	void *ptr = CUR_LDTDEF;
	al_lwrite (list, &ptr);
      }
    {
      Menu_data_ldt menudata (list);
      r = DisplayMenuList
       (x0+126, y0 + 2 * FONTH, "Select type", menudata, NULL);
    }
    if (r < 0)
      goto again;
    if (al_lseek (list, r, SEEK_SET))
      fatal_error ("%s ILT2 (%s)", msg_unexpected, al_astrerror (al_aerrno));
    *number = (*((ldtdef_t **) al_lptr (list)))->number;
    al_ldiscard (list);
    break;

    again :
    ;
    /* draw_map (OBJ_THINGS, 0, 0);  FIXME! */
  }

  return 0;
}


/*
 *	PrintLdtgroup
 *	Used by DisplayMenuList when called by InputLinedefType
 */
static const char *PrintLdtgroup (void *ptr)
{
  if (! ptr)
    return "PrintLdtgroup: (null)";
  return ((ldtgroup_t *)ptr)->desc;
}

