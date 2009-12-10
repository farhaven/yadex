/*
 *	editloop.cc
 *	The main loop of the editor.
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
#include <assert.h>
#include "_edit.h"
#include "checks.h"
#include "dialog.h"
#include "drawmap.h"
#include "edisplay.h"
#include "editgrid.h"
#include "editloop.h"
#include "editobj.h"
#include "editsave.h"
#include "editzoom.h"
#include "entry.h"
#include "entry2.h"
#include "events.h"
#include "game.h"
#include "gfx.h"
#include "gfx2.h"	// show_character_set() show_pcolours()
#include "gfx3.h"
#include "gotoobj.h"
#include "help2.h"
#include "l_flags.h"
#include "levels.h"
#include "lists.h"
#include "menubar.h"
#include "menu.h"
#include "modpopup.h"
#include "objects.h"
#include "objid.h"
#include "palview.h"
#include "prefer.h"
#include "rgbbmp.h"
#include "s_slice.h"
#include "selbox.h"
#include "selectn.h"
#include "selpath.h"
#include "spot.h"
#include "t_flags.h"
#include "t_spin.h"
#include "x_centre.h"
#include "x_exchng.h"
#include "x_hover.h"
#include "xref.h"
#include "r_render.h"

#include <X11/Xlib.h>



static int zoom_fit (edit_t&);


extern bool InfoShown;		/* should we display the info bar? */
#ifdef DIALOG
#endif


static int menubar_out_y1;	/* FIXME */


/* prototypes of private functions */
static int SortLevels (const void *item1, const void *item2);
static char *GetBehaviorFileName (const char *levelname);

/*
 *	SelectLevel
 *	Prompt the user for a level name (EnMn or MAPnm). The
 *	name chosen must be present in the master directory
 *	(iwad or pwads).
 *
 *	If <levelno> is 0, the level name can be picked from all
 *	levels present in the master directory. If <levelno> is
 *	non-zero, the level name can be picked only from those
 *	levels in the master directory for which
 *	levelname2levelno() % 1000 is equal to <levelno>. For
 *	example, if <levelno> is equal to 12, only E1M2 and
 *	MAP12 would be listed. This feature is not used anymore
 *	because "e" now requires an argument and tends to deal
 *	with ambiguous level names (like "12") itself.
 */
const char *SelectLevel (int levelno)
{
MDirPtr dir;
static char name[WAD_NAME + 1]; /* AYM it was [7] previously */
char **levels = 0;
int n = 0;           /* number of levels in the dir. that match */

get_levels_that_match:
for (dir = MasterDir; dir; dir = dir->next)
   {
   if (levelname2levelno (dir->dir.name) > 0
    && (levelno==0 || levelname2levelno (dir->dir.name) % 1000 == levelno))
      {
      if (n == 0)
	 levels = (char **) GetMemory (sizeof (char *));
      else
	 levels = (char **) ResizeMemory (levels, (n + 1) * sizeof (char *));
      levels[n] = dir->dir.name;
      n++;
      }
   }
if (n == 0 && levelno != 0)  /* In case no level matched levelno */
   {
   levelno = 0;               /* List ALL levels instead */
   goto get_levels_that_match;
   }
/* So that InputNameFromList doesn't fail if you
   have both EnMn's and MAPnn's in the master dir. */
qsort (levels, n, sizeof (char *), SortLevels);
al_scps (name, levels[0], sizeof name - 1);
if (n == 1)
   return name;
InputNameFromList (-1, -1, "Level name :", n, levels, name);
FreeMemory (levels);
return name;
}


/*
   compare 2 level names (for sorting)
*/

static int SortLevels (const void *item1, const void *item2)
{
/* FIXME should probably use y_stricmp() instead */
return strcmp (*((const char * const *) item1),
               *((const char * const *) item2));
}


// A table of the modes in the editor.
typedef struct
   {
   i8 obj_type;		// Corresponding object type
   i8 item_no;		// # of item to tick in the "View" menu
   i8 menu_no;		// # of flavour of the "Misc op." menu
   } editmode_t;

const int NB_MODES = 4;

static const editmode_t modes[NB_MODES] =
   {
   { OBJ_THINGS,   0, MBM_MISC_T },
   { OBJ_LINEDEFS, 1, MBM_MISC_L },
   { OBJ_VERTICES, 2, MBM_MISC_V },
   { OBJ_SECTORS,  3, MBM_MISC_S },
   };


/*
 *	obj_type_to_mode_no
 *	Return the # of the mode that has this <obj_type>
 */
static int obj_type_to_mode_no (int obj_type)
{
int n;
for (n = 0; n < NB_MODES; n++)
   if (modes[n].obj_type == obj_type)
      break;
if (n == NB_MODES)
   fatal_error ("no mode for obj_type %d", obj_type);
return n;
}


// Used by the View menu
bool mode_th (micbarg_t p) { return ((edit_t *) p)->obj_type == OBJ_THINGS;   }
bool mode_l  (micbarg_t p) { return ((edit_t *) p)->obj_type == OBJ_LINEDEFS; }
bool mode_v  (micbarg_t p) { return ((edit_t *) p)->obj_type == OBJ_VERTICES; }
bool mode_s  (micbarg_t p) { return ((edit_t *) p)->obj_type == OBJ_SECTORS;  }


/*
  the editor main loop
*/

void EditorLoop (const char *levelname) /* SWAP! */
{
edit_t e;
/* FIXME : all these variables should be moved to edit_t : */
int    RedrawMap;
bool   DragObject = false;
int    oldbuttons;

bool   StretchSelBox = false;  // FIXME apparently not used anymore...

Objid object;			// The object under the pointer
const Objid CANVAS (OBJ_NONE, OBJ_NO_CANVAS);

memset (&e, 0, sizeof e);	/* Catch-all */
e.move_speed          = 20;
e.extra_zoom          = 0;
// If you change this, don't forget to change
// the initialisation of the menu bar below.
e.obj_type            = OBJ_THINGS;
e.global              = false;
e.tool                = TOOL_NORMAL;
e.grid_step           = 128;
e.grid_step_min       = GridMin;
e.grid_step_max       = GridMax;
e.grid_step_locked    = 0;
e.grid_shown          = 1;
e.grid_snap           = 1;
e.infobar_shown       = (bool) InfoShown;
e.objinfo_shown       = true;
e.show_object_numbers = false;
e.show_things_squares = false;
e.show_things_sprites = true;
e.rulers_shown        = 0;
e.clicked.nil ();
//e.click_obj_no        = OBJ_NO_NONE;
//e.click_obj_type      = -1;
e.click_ctrl          = 0;
e.highlighted.nil ();
//e.highlight_obj_no    = OBJ_NO_NONE;
//e.highlight_obj_type  = -1;
e.Selected            = 0;
e.selbox              = new selbox_c;
e.edisplay            = new edisplay_c (&e);
e.menubar             = new menubar_c;
e.spot                = new spot_c;
e.modpopup            = new modpopup_c;
e.modal               = '\0';

MadeChanges = 0;
MadeMapChanges = 0;

// Sane defaults
Scale = 1.0;
OrigX = 0;
OrigY = 0;

edit_zoom_init ();

if (zoom_default == 0)
   {
   zoom_fit (e);
   }
else
   {
   int r = edit_set_zoom (&e, zoom_default / 100.0);
   if (r == 0)
      CenterMapAroundCoords ((MapMinX + MapMaxX) / 2, (MapMinY + MapMaxY) / 2);
   }
   oldbuttons = 0;

/* Create the menu bar */
{
e.menubar->compute_menubar_coords (0, 0, ScrMaxX, ScrMaxY);

e.mb_menu[MBM_FILE] = new Menu (NULL,
   "~Save",       YK_F2, 0,
   "Save ~as...", YK_F3, 0,
// "~Print",      YK_,   MIF_SACTIVE, false, 0,
   "~Quit",       'q',   0,
   NULL);


if (yg_level_format == YGLF_HEXEN)
   e.mb_menu[MBM_EDIT] = new Menu (NULL,
     "~Copy object(s)",          'o',    0,
      "~Add object",              YK_INS, 0,
      "~Delete object(s)",        YK_DEL, 0,
      "~Exchange object numbers", 24,     0,
      "~Preferences...",          YK_F5,  0,
      "~Snap to grid",            'y',    MIF_VTICK, &e.grid_snap,		     0,
      "~Lock grid step",          'z',    MIF_VTICK, &e.grid_step_locked,	     0,
      "Load ~BEHAVIOR lump",      'b',    0,
      NULL);
else
   e.mb_menu[MBM_EDIT] = new Menu (NULL,
      "~Copy object(s)",          'o',    0,
      "~Add object",              YK_INS, 0,
      "~Delete object(s)",        YK_DEL, 0,
      "~Exchange object numbers", 24,     0,
      "~Preferences...",          YK_F5,  0,
      "~Snap to grid",            'y',    MIF_VTICK, &e.grid_snap,		     0,
      "~Lock grid step",          'z',    MIF_VTICK, &e.grid_step_locked,	     0,
      NULL);

// If you change the order of modes here, don't forget
// to modify the <modes> array.
e.mb_menu[MBM_VIEW] = new Menu (NULL,
   "~Things",              't',        MIF_FTICK,   mode_th, (micbarg_t) &e, 0,
   "~Linedefs & sidedefs", 'l',        MIF_FTICK,   mode_l,  (micbarg_t) &e, 0,
   "~Vertices",            'v',        MIF_FTICK,   mode_v,  (micbarg_t) &e, 0,
   "~Sectors",             's',        MIF_FTICK,   mode_s,  (micbarg_t) &e, 0,
   "Global",               '\7',       MIF_VTICK,   &e.global,               0,
   "~Next mode",           YK_TAB,     0,
   "~Prev mode",           YK_BACKTAB, 0,
   MI_SEPARATION,
   "Zoom ~in",             '+',        0,
   "Zoom ~out",            '-',        0,
   "Extra ~zoom",          ' ',        MIF_VTICK,   &e.extra_zoom,           0,
   "~Whole level",         '`',        0,
   MI_SEPARATION,
   "Show object numbers",  '&',        MIF_VTICK,   &e.show_object_numbers,  0,
   "Show sprites",         '%',        MIF_VTICK,   &e.show_things_sprites,  0,
   "Show ~grid",           'h',        MIF_VTICK,   &e.grid_shown,           0,
   "Info bar",             YK_ALT+'i', MIF_VTICK,   &e.infobar_shown,        0,
   "Object info boxes",    'i',        MIF_VTICK,   &e.objinfo_shown,        0,
   "3D p~review",          'R',        0,
   NULL);

e.mb_menu[MBM_SEARCH] = new Menu (NULL,
// "~Find/change",       YK_F4, MIF_SACTIVE, false, 0,
// "~Repeat last find",  -1,    MIF_SACTIVE, false, 0,
   "~Next object",       'n',   0,
   "~Prev object",       'p',   0,
   "~Jump to object...", 'j',   0,
   NULL);

e.mb_menu[MBM_MISC_L] = new Menu ("Misc. operations",
   "Find first free ~tag number",		YK_, 0,
   "~Rotate and scale linedefs...",		YK_, 0,
   "Split linedefs (add new ~vertex)",		YK_, 0,
   "~Split linedefs and sector",		YK_, 0,
   "~Delete linedefs and join sectors",		YK_, 0,
   "~Flip linedefs",				YK_, 0,
   "S~wap sidedefs",				YK_, 0,
   "Align textures (~Y offset)",		YK_, 0,
   "Align textures (~X offset)...",		YK_, 0,
   "Remove ~2nd sidedef (make single-sided)",	YK_, 0,
   "Make rectangular ~nook (32x16)",		YK_, 0,
   "Make rectangular ~boss (32x16)",		YK_, 0,
   "Set ~length (move 1st vertex)...",		YK_, 0,
   "Set length (move 2nd vertex)...",		YK_, 0,
   "~Unlink 1st sidedef",			YK_, 0,
   "Unlink 2nd sidedef",			YK_, 0,
   "~Mirror horizontally",			YK_, 0,
   "Mirror v~ertically",			YK_, 0,
   "~Cut a slice out of a sector",		YK_, 0,
   NULL);

e.mb_menu[MBM_MISC_S] = new Menu ("Misc. operations",
   "Find first free ~tag number",	YK_, 0,
   "~Rotate and scale sectors...",	YK_, 0,
   "Make ~door from sector",		YK_, 0,
   "Make ~lift from sector",		YK_, 0,
   "Distribute sector ~floor heights",	YK_, 0,
   "Distribute sector ~ceiling heights",YK_, 0,
   "R~aise or lower sectors...",	YK_, 0,
   "~Brighten or darken sectors...",	YK_, 0,
   "~Unlink room",			YK_, 0,
   "~Mirror horizontally",		YK_, 0,
   "Mirror ~vertically",		YK_, 0,
   "~Swap flats",			YK_, 0,
   NULL);

if (yg_level_format != YGLF_HEXEN)
{	e.mb_menu[MBM_MISC_T] = new Menu ("Misc. operations",
   	"Find first free ~tag number",	 YK_, 0,
   	"~Rotate and scale things...",	 YK_, 0,
   	"~Spin things 45° clockwise",	 'x', 0,
   	"Spin things 45° ~counter-clockwise", 'w', 0,
   	"~Mirror horizontally",		 YK_, 0,
   	"Mirror ~vertically",		 YK_, 0,
   	NULL);
}else
{	e.mb_menu[MBM_MISC_T] = new Menu ("Misc. operations",
   	"Find first free ~tag number",	 YK_, 0,
	"Find first free T~ID",		 YK_, 0,
   	"~Spin things 45° clockwise",	 'x', 0,
   	"Spin things 45° ~counter-clockwise", 'w', 0,
   	"~Mirror horizontally",		 YK_, 0,
   	"Mirror ~vertically",		 YK_, 0,
	NULL);
}

e.mb_menu[MBM_MISC_V] = new Menu ("Misc. operations",
   "Find first free ~tag number",	YK_, 0,
   "~Rotate and scale vertices...",	YK_, 0,
   "~Delete vertex and join linedefs",	YK_, 0,
   "~Merge several vertices into one",	YK_, 0,
   "Add a linedef and ~split sector",	YK_, 0,
   "Mirror ~horizontally",		YK_, 0,
   "Mirror ~vertically",		YK_, 0,
   NULL);

e.mb_menu[MBM_OBJECTS] = new Menu ("Insert a pre-defined object",
   "~Rectangle...",         YK_, 0,
   "~Polygon (N sides)...", YK_, 0,
   NULL);

e.mb_menu[MBM_CHECK] = new Menu ("Check level consistency",
   "~Number of objects",		YK_, 0,
   "Check if all ~sectors are closed",	YK_, 0,
   "Check all ~cross-references",	YK_, 0,
   "Check for ~missing textures",	YK_, 0,
   "Check ~texture names",		YK_, 0,
   NULL);

e.mb_menu[MBM_HELP] = new Menu (NULL,
   "~Keyboard & mouse...", YK_F1,        0,
   "~About Yadex...",      YK_ALT + 'a', 0,
   NULL);

e.mb_ino[MBI_FILE] =
   e.menubar->add_item ("File",    0, 0, e.mb_menu[MBM_FILE]   );
e.mb_ino[MBI_EDIT] =
   e.menubar->add_item ("Edit",    0, 0, e.mb_menu[MBM_EDIT]   );
e.mb_ino[MBI_VIEW] =
   e.menubar->add_item ("View",    0, 0, e.mb_menu[MBM_VIEW]  );
e.mb_ino[MBI_SEARCH] =
   e.menubar->add_item ("Search",  0, 0, e.mb_menu[MBM_SEARCH] );
e.mb_ino[MBI_MISC] =
   e.menubar->add_item ("Misc",    0, 0, e.mb_menu[MBM_MISC_T] );
e.mb_ino[MBI_OBJECTS] =
   e.menubar->add_item ("Objects", 0, 0, e.mb_menu[MBM_OBJECTS]);
e.mb_ino[MBI_CHECK] =
   e.menubar->add_item ("Check",   0, 0, e.mb_menu[MBM_CHECK]  );
e.mb_ino[MBI_HELP] =
   e.menubar->add_item ("Help",    0, 1, e.mb_menu[MBM_HELP]   );

menubar_out_y1 = 2 * BOX_BORDER + 2 * NARROW_VSPACING + FONTH - 1;  // FIXME
}

// FIXME this should come from the .ygd
// instead of being hard-coded.
Menu *menu_linedef_flags = new Menu (NULL,
   "~Impassable",		YK_, 0,
   "~Monsters cannot cross",	YK_, 0,
   "~Double-sided",		YK_, 0,
   "~Upper texture unpegged",	YK_, 0,
   "~Lower texture unpegged",	YK_, 0,
   "~Secret (shown as normal)",	YK_, 0,
   "~Blocks sound",		YK_, 0,
   "~Never shown on the map",	YK_, 0,
   "~Always shown on the map",	YK_, 0,
   "~Pass through [Boom]",	YK_, 0,  // Boom extension
   "b1~0 0400h",		YK_, 0,  // Undefined
   "b1~1 0800h",		YK_, 0,  // Undefined
   "~Translucent [Strife]",	YK_, 0,  // Strife
   "b1~3 2000h",		YK_, 0,  // Undefined
   "b1~4 4000h",		YK_, 0,  // Undefined
   "b1~5 8000h",		YK_, 0,  // Undefined
   NULL);

Menu *menu_thing_flags = new Menu (NULL,
   "~Easy",			YK_, 0,
   "Medi~um",			YK_, 0,
   "~Hard",			YK_, 0,
   "~Deaf",			YK_, 0,
   "~Multiplayer",		YK_, 0,
   "~Not in DM [Boom]",		YK_, 0,  // Boom extension
   "Not in ~coop [Boom]", 	YK_, 0,  // Boom extension
   "F~riendly [MBF]",		YK_, 0,  // MBF extension
   "b~8  0100h",		YK_, 0,  // Undefined
   "b~9  0200h",		YK_, 0,  // Undefined
   "b1~0 0400h",		YK_, 0,  // Undefined
   "b1~1 0800h",		YK_, 0,  // Undefined
   "b1~2 1000h",		YK_, 0,  // Undefined
   "b1~3 2000h",		YK_, 0,  // Undefined
   "b1~4 4000h",		YK_, 0,  // Undefined
   "b1~5 8000h",		YK_, 0,  // Undefined
   NULL);

/* AYM 1998-06-22
   This is the big mean loop. I organized it in three main steps :

   1. Update the display according to the current state of affairs.
   2. Wait for the next event (key press, mouse click/motion, etc.).
   3. Process this event and change states as appropriate.

   This piece of code remembers a lot of state in various variables and
   flags. Hope you can work your way through it. If you don't, don't
   hesitate to ask me. You never know, I might help you to be even more
   confused. ;-) */

for (RedrawMap = 1; ; RedrawMap = 0)
   {
   int motion = 0;  // Initialized to silence GCC warning

   /*
    *  Step 1 -- Do all the displaying work
    */

#ifdef Y_BATCH
   // Hold refresh until all events are processed
   if (! has_event () && ! has_input_event ())
      {
#endif
      if (e.highlighted ())				// FIXME
	 e.edisplay->highlight_object (e.highlighted);	// Should
      else						// be in
	 e.edisplay->forget_highlight ();		// edisplay_c !

   if (is.in_window)
      e.spot->set (edit_mapx_snapped (&e, e.pointer_x),
                   edit_mapy_snapped (&e, e.pointer_y));
   else
      e.spot->unset ();

      e.edisplay->refresh ();
      /* The display is now up to date */
#ifdef Y_BATCH
      }
#endif


   /*
    *  Step 2 -- Get the next event
    */

   if (has_key_press_event ())
      is.key = get_event ();
   else
      get_input_status ();
   e.pointer_in_window = is.in_window;
   if (is.in_window)
      {
      /* AYM 1998-07-04
	 If the map coordinates of the pointer have changed,
	 generate a pointer motion event. I don't like to do
	 that but it makes things much simpler elsewhere. */
      if (MAPX (is.x) != e.pointer_x || MAPY (is.y) != e.pointer_y)
	 motion = 1;
      else
	 motion = 0;

      e.pointer_x = MAPX (is.x);
      e.pointer_y = MAPY (is.y);
      obj_type_t t = e.global ? OBJ_ANY : e.obj_type;
      GetCurObject (object, t, e.pointer_x, e.pointer_y);
      }

   /*
    *  Step 3 -- Process the event
    *  This section is (should be) a long list of elif's
    */

   /*
    *	Step 3.1A -- If a pull-down menu is "on",
    *	try to make it process the event.
    */

   if (e.menubar->pulled_down () >= 0)
      {
      int menu_no = e.menubar->pulled_down ();
      Menu *menu = e.menubar->get_menu (menu_no);
      int r = menu->process_event (&is);

      if (r == MEN_CANCEL)
	 {
	 e.menubar->pull_down (-1);
	 e.menubar->highlight (-1);
         goto done;
	 }

      // The event was understood and processed normally
      // by the menu event handler so we're done.
      else if (r == MEN_OTHER)
	 goto done;

      // The event was not understood by the menu event
      // handler so let's see what the normal event handler
      // can do with it.
      else if (r == MEN_INVALID)
	 ;

      else
	 {
	 e.menubar->pull_down (-1);
	 e.menubar->highlight (-1);
	 if (menu_no == e.mb_ino[MBI_MISC])
	    {
	    if (e.Selected)
	       MiscOperations (e.obj_type, &e.Selected, r + 1);
	    else
	       {
	       if (e.highlighted ())
		  SelectObject (&e.Selected, e.highlighted.num);
	       MiscOperations (e.obj_type, &e.Selected, r + 1);
	       if (e.highlighted ())
		  UnSelectObject (&e.Selected, e.highlighted.num);
	       }
	    e.highlighted.nil ();
	    DragObject = false;
	    StretchSelBox = false;
	    RedrawMap = 1;
	    }
	 else if (menu_no == e.mb_ino[MBI_OBJECTS])
            {
	    /* code duplicated from 'F9' - I hate to do that */
	    int savednum = NumLineDefs;
	    InsertStandardObject (e.pointer_x, e.pointer_y, r + 1);
	    if (NumLineDefs > savednum)
	       {
	       ForgetSelection (&e.Selected);
	       e.obj_type = OBJ_LINEDEFS;
	       for (int i = savednum; i < NumLineDefs; i++)
		  SelectObject (&e.Selected, i);
	       e.highlighted.nil ();
	       DragObject = false;
	       StretchSelBox = false;
	       }
	    RedrawMap = 1;
            }
	 else if (menu_no == e.mb_ino[MBI_CHECK])
	    {
	    if (r == 0)
	       Statistics ();
	    else if (r == 1)
	       CheckSectors ();
	    else if (r == 2)
	       CheckCrossReferences ();
	    else if (r == 3)
	       CheckTextures ();
	    else if (r == 4)
	       CheckTextureNames ();
	    }
	 else
	    send_event (menu->last_shortcut_key ());
         goto done;
	 }
      }

   /*
    *	Step 3.1B -- If the "Misc operations" popup is on,
    *	try to make it process the event.
    */

   else if (e.modpopup->get () == e.mb_menu[MBM_MISC_L]
      || e.modpopup->get () == e.mb_menu[MBM_MISC_S]
      || e.modpopup->get () == e.mb_menu[MBM_MISC_T]
      || e.modpopup->get () == e.mb_menu[MBM_MISC_V])
      {
      int r = (e.modpopup->get ())->process_event (&is);

      // [Esc] or click outside the popup menu. Close it.
      if (r == MEN_CANCEL
         || r == MEN_INVALID && is.key == YE_BUTL_PRESS)
	 {
         e.modpopup->unset ();
         goto done2;
	 }

      // The event was understood and processed normally
      // by the menu event handler so we're done.
      else if (r == MEN_OTHER)
	 goto done2;

      // The event was not understood by the menu event
      // handler so let's see what the normal event handler
      // can do with it.
      else if (r == MEN_INVALID)
	 ;

      else
	 {
         e.modpopup->unset ();
	 if (e.Selected)
	    MiscOperations (e.obj_type, &e.Selected, r + 1);
	 else
	    {
	    if (e.highlighted ())
	       SelectObject (&e.Selected, e.highlighted.num);
	    MiscOperations (e.obj_type, &e.Selected, r + 1);
	    if (e.highlighted ())
	       UnSelectObject (&e.Selected, e.highlighted.num);
	    }
	 e.highlighted.nil ();
	 DragObject = false;
	 StretchSelBox = false;
	 goto done2;
	 }
      }

   /*
    *	Step 3.1C -- If the "Insert standard object" popup is on,
    *	try to make it process the event.
    */

   else if (e.modpopup->get () == e.mb_menu[MBM_OBJECTS])
      {
      int r = (e.modpopup->get ())->process_event (&is);

      // [Esc] or click outside the popup menu. Close it.
      if (r == MEN_CANCEL
         || r == MEN_INVALID && is.key == YE_BUTL_PRESS)
	 {
         e.modpopup->unset ();
         goto done2;
	 }

      // The event was understood and processed normally
      // by the menu event handler so we're done.
      else if (r == MEN_OTHER)
	 goto done2;

      // The event was not understood by the menu event
      // handler so let's see what the normal event handler
      // can do with it.
      else if (r == MEN_INVALID)
	 ;

      else
	 {
         e.modpopup->unset ();
	 int savednum = NumLineDefs;
	 InsertStandardObject (e.pointer_x, e.pointer_y, r + 1);
	 if (NumLineDefs > savednum)
	    {
	    ForgetSelection (&e.Selected);
	    e.obj_type = OBJ_LINEDEFS;
	    for (int i = savednum; i < NumLineDefs; i++)
	       SelectObject (&e.Selected, i);
	    e.highlighted.nil ();
	    DragObject = false;
	    StretchSelBox = false;
	    }
         goto done2;
	 }
      }

   /*
    *	Step 3.1D -- if the "Set/toggle/clear" linedef flag popup
    *	is on, try to make it process the event.
    */
   else if (e.modpopup->get () == menu_linedef_flags)
      {
      int r = (e.modpopup->get ())->process_event (&is);

      // [Esc] or click outside the popup menu. Close it.
      if (r == MEN_CANCEL
         || r == MEN_INVALID && is.key == YE_BUTL_PRESS)
	 {
         e.modpopup->unset ();
         goto done2;
	 }

      // The event was understood and processed normally
      // by the menu event handler so we're done.
      else if (r == MEN_OTHER)
	 goto done2;

      // The event was not understood by the menu event
      // handler so let's see what the normal event handler
      // can do with it.
      else if (r == MEN_INVALID)
	 ;

      else
	 {
         int op = -1;
         e.modpopup->unset ();
         if (e.modal == 's')
            op = YO_SET;
         else if (e.modal == 't')
            op = YO_TOGGLE;
         else if (e.modal == 'c')
            op = YO_CLEAR;
         else
            fatal_error ("modal=%02X", e.modal);
         if (! e.Selected)
            {
            SelectObject (&e.Selected, e.highlighted.num);
            frob_linedefs_flags (e.Selected, op, r);
            UnSelectObject (&e.Selected, e.highlighted.num);
            }
         else
            {
            frob_linedefs_flags (e.Selected, op, r);
            }
         goto done2;
	 }
      }

   /*
    *	Step 3.1E -- if the "Set/toggle/clear" thing flag popup
    *	is on, try to make it process the event.
    */
   else if (e.modpopup->get () == menu_thing_flags)
      {
      int r = (e.modpopup->get ())->process_event (&is);

      // [Esc] or click outside the popup menu. Close it.
      if (r == MEN_CANCEL
         || r == MEN_INVALID && is.key == YE_BUTL_PRESS)
	 {
         e.modpopup->unset ();
         goto done2;
	 }

      // The event was understood and processed normally
      // by the menu event handler so we're done.
      else if (r == MEN_OTHER)
	 goto done2;

      // The event was not understood by the menu event
      // handler so let's see what the normal event handler
      // can do with it.
      else if (r == MEN_INVALID)
	 ;

      else
         {
         int op = -1;
         e.modpopup->unset ();
         if (e.modal == 's')
            op = YO_SET;
         else if (e.modal == 't')
            op = YO_TOGGLE;
         else if (e.modal == 'c')
            op = YO_CLEAR;
         else
            fatal_error ("modal=%02X", e.modal);
         if (! e.Selected)
            {
            SelectObject (&e.Selected, e.highlighted.num);
            frob_things_flags (e.Selected, op, r);
            UnSelectObject (&e.Selected, e.highlighted.num);
            }
         else
            {
            frob_things_flags (e.Selected, op, r);
            }
         goto done2;
	 }
      }

   /*
    *	Step 3.2 -- "Normal" event handling
    */

   /*
    *	Step 3.2.1 -- Non keyboard events
    */

   if (is.key == YE_EXPOSE)
      {
      RedrawMap = 1;
      goto done2;
      }

   else if (is.key == YE_RESIZE)
      {
      SetWindowSize (is.width, is.height);
      e.menubar->compute_menubar_coords (0, 0, ScrMaxX, ScrMaxY);
      RedrawMap = 1;
      goto done2;
      }

   // To prevent normal handling when a popup menu is on.
   if (e.modpopup->get () != 0)
      {
      goto done2;
      }

   /*
    *	Step 3.2.2 -- Mouse events
    */


   // Clicking on an item of the menu bar
   // pulls down the corresponding menu.
   else if (is.key == YE_BUTL_PRESS
      && e.menubar->is_on_menubar_item (is.x, is.y) >= 0)
      {
      int itemno;

      e.clicked.nil ();
      itemno = e.menubar->is_on_menubar_item (is.x, is.y);
      if (itemno >= 0)
         e.menubar->pull_down (itemno);
      else
         Beep ();
      goto done;
      }
   else if (is.key == YE_BUTR_PRESS && e.menubar->highlighted () < 0)
      {
      e.modpopup->set (e.menubar->get_menu (MBI_MISC),1,e.pointer_x,e.pointer_y);
      }
   /* Clicking on an empty space starts a new selection box.
      Unless [Ctrl] is pressed, it also clears the current selection. */
   else if (is.key == YE_BUTL_PRESS
	 && e.tool == TOOL_NORMAL
	 && object.is_nil ())
      {
      e.menubar->highlight (-1);  // Close any open menu
      e.clicked    = CANVAS;
      e.click_ctrl = is.ctrl;
      if (! is.ctrl)
         {
         ForgetSelection (&e.Selected);
         RedrawMap = 1;
         }
      e.selbox->set_1st_corner (e.pointer_x, e.pointer_y);
      e.selbox->set_2nd_corner (e.pointer_x, e.pointer_y);
      }

   /* Clicking on an unselected object unselects
      everything but that object. Additionally,
      we write the number of the object in case
      the user is about to drag it. */
   else if (is.key == YE_BUTL_PRESS && ! is.ctrl
	 && e.tool == TOOL_NORMAL
	 && ! IsSelected (e.Selected, object.num))
      {
      e.menubar->highlight (-1);  // Close any open menu
      e.clicked        = object;
      e.click_ctrl     = 0;
      e.click_time     = is.time;
      ForgetSelection (&e.Selected);
      SelectObject (&e.Selected, object.num);
      /* I don't like having to do that */
      if (object.type == OBJ_THINGS && object ())
	 MoveObjectsToCoords (object.type, 0,
            Things[object.num].xpos, Things[object.num].ypos, 0);
      else if (object.type == OBJ_VERTICES && object ())
	 MoveObjectsToCoords (object.type, 0,
            Vertices[object.num].x, Vertices[object.num].y, 0);
      else
	 MoveObjectsToCoords (object.type, 0,
            e.pointer_x, e.pointer_y, e.grid_snap ? e.grid_step : 0);
      RedrawMap = 1;
      }

   /* Second click of a double click on an object */
   else if (is.key == YE_BUTL_PRESS && ! is.ctrl
	 && e.tool == TOOL_NORMAL
	 && IsSelected (e.Selected, object.num)
	 && object  == e.clicked
	 && is.time - e.click_time <= (unsigned long) double_click_timeout)
      {
      // Very important! If you don't do that, the release of the
      // click that closed the properties menu will drag the object.
      e.clicked.nil ();
      send_event (YK_RETURN);
      goto done;
      }

   /* Clicking on a selected object does nothing ;
      the user might want to drag the selection. */
   else if (is.key == YE_BUTL_PRESS && ! is.ctrl
	 && e.tool == TOOL_NORMAL
	 && IsSelected (e.Selected, object.num))
      {
      e.menubar->highlight (-1);  // Close any open menu
      e.clicked        = object;
      e.click_ctrl     = 0;
      e.click_time     = is.time;
      /* I don't like having to do that */
      if (object.type == OBJ_THINGS && object ())
	 MoveObjectsToCoords (object.type, 0,
            Things[object.num].xpos, Things[object.num].ypos, 0);
      else if (object.type == OBJ_VERTICES && object ())
	 MoveObjectsToCoords (object.type, 0,
            Vertices[object.num].x, Vertices[object.num].y, 0);
      else
	 MoveObjectsToCoords (object.type, 0,
            e.pointer_x, e.pointer_y, e.grid_snap ? e.grid_step : 0);
      }

   /* Clicking on selected object with [Ctrl] pressed unselects it.
      Clicking on unselected object with [Ctrl] pressed selects it. */
   else if (is.key == YE_BUTL_PRESS && is.ctrl
	 && e.tool == TOOL_NORMAL
	 && object ())
      {
      e.menubar->highlight (-1);  // Close any open menu
      e.clicked        = object;
      e.click_ctrl     = 1;
      if (IsSelected (e.Selected, object.num))
	UnSelectObject (&e.Selected, object.num);
      else
	SelectObject (&e.Selected, object.num);
      RedrawMap = 1;
      }

   /* TOOL_SNAP_VERTEX */
   else if (is.key == YE_BUTL_PRESS
	    && e.tool == TOOL_SNAP_VERTEX
	    && e.obj_type == OBJ_VERTICES
	    && object ()
	    // Can't delete vertex that is referenced by the selection
	    && ! IsSelected (e.Selected, object.num))
      {
      printf ("SNAP %d\n", (int) object.num);
      SelPtr list = 0;
      SelectObject (&list, object.num);
      DeleteVerticesJoinLineDefs (list);
      ForgetSelection (&list);
      RedrawMap = 1;
      }

   /* Clicking anywhere else closes the pull-down menus. */
   else if (is.key == YE_BUTL_PRESS)
      e.menubar->highlight (-1);  // Close any open menu

   /* Releasing the button while there was a selection box
      causes all the objects within the box to be selected. */
   // FIXME : should call this automatically when switching tool
   else if (is.key == YE_BUTL_RELEASE
	 && e.tool == TOOL_NORMAL
	 && e.clicked == CANVAS)
      {
      int x1, y1, x2, y2;
      e.selbox->get_corners (&x1, &y1, &x2, &y2);
      SelectObjectsInBox (&e.Selected, e.obj_type, x1, y1, x2, y2);
      e.selbox->unset_corners ();
      RedrawMap = 1;
      }

   /* Releasing the button while dragging : drop the selection. */
   // FIXME : should call this automatically when switching tool
   else if (is.key == YE_BUTL_RELEASE
	 && e.tool == TOOL_NORMAL
	 && e.clicked ())
      {
      if (AutoMergeVertices (&e.Selected, e.obj_type, 'm'))
         RedrawMap = 1;
      }

   // Moving the pointer with the left button pressed
   // after clicking on an item of the menu bar : pull
   // down menus as the pointer passes over them.
   else if (is.key == YE_MOTION
      && e.tool == TOOL_NORMAL
      && is.butl
      && ! e.clicked () && ! (e.clicked == CANVAS))
      {
      int itemno = e.menubar->is_on_menubar_item (is.x, is.y);
      if (itemno >= 0)
         e.menubar->pull_down (itemno);
      goto done;
      }

   /* Moving the pointer with the left button pressed
      and a selection box exists : move the second
      corner of the selection box. */
   else if ((is.key == YE_MOTION || motion)
      && e.tool == TOOL_NORMAL
      && is.butl && e.clicked == CANVAS)
      {
      e.selbox->set_2nd_corner (e.pointer_x, e.pointer_y);
      }

   /* Moving the pointer with the left button pressed
      but no selection box exists and [Ctrl] was not
      pressed when the button was pressed :
      drag the selection. */
   else if (motion
      && e.tool == TOOL_NORMAL
      && is.butl
      && e.clicked ()
      && ! e.click_ctrl)
      {
      if (! e.Selected)
	 {
	 SelectObject (&e.Selected, e.clicked.num);
	 if (MoveObjectsToCoords (e.clicked.type, e.Selected,
            e.pointer_x, e.pointer_y, e.grid_snap ? e.grid_step : 0))
	    RedrawMap = 1;
	 ForgetSelection (&e.Selected);
	 }
      else
	 if (MoveObjectsToCoords (e.clicked.type, e.Selected,
            e.pointer_x, e.pointer_y, e.grid_snap ? e.grid_step : 0))
	    RedrawMap = 1;
      }

   /* AYM : added is.in_window */
   if (is.in_window && ! is.butl && ! is.shift)
      {
      /* Check if there is something near the pointer */
      e.highlighted = object;
      }

   /*
    *	Step 3.2.3 -- Keyboard events
    */

   if (event_is_key (is.key)
      || is.key == YE_WHEEL_UP
      || is.key == YE_WHEEL_DOWN)
      {
      if (is.key == YK_LEFT && e.menubar->highlighted () >= 0)
         {
         int new_item = e.menubar->highlighted () - 1;
         if (new_item < 0)
            new_item = e.mb_ino[MBI_HELP];
         e.menubar->pull_down (new_item);
         RedrawMap = 1;
         }

      else if (is.key == YK_RIGHT && e.menubar->highlighted () >= 0)
         {
         int new_item = e.menubar->highlighted () + 1;
         if (new_item > e.mb_ino[MBI_HELP])
            new_item = 0;
         e.menubar->pull_down (new_item);
         RedrawMap = 1;
         }

      else if (is.key == YK_ALT + 'f')
         e.menubar->pull_down (e.mb_ino[MBI_FILE]);

      else if (is.key == YK_ALT + 'e')
         e.menubar->pull_down (e.mb_ino[MBI_EDIT]);

      else if (is.key == YK_ALT + 's')
	 e.menubar->pull_down (e.mb_ino[MBI_SEARCH]);

      else if (is.key == YK_ALT + 'v')
	 e.menubar->pull_down (e.mb_ino[MBI_VIEW]);

      else if (is.key == YK_ALT + 'm')
	 e.menubar->pull_down (e.mb_ino[MBI_MISC]);

      else if (is.key == YK_ALT + 'o')
	 e.menubar->pull_down (e.mb_ino[MBI_OBJECTS]);

      else if (is.key == YK_ALT + 'c')
	 e.menubar->pull_down (e.mb_ino[MBI_CHECK]);

      else if (is.key == YK_ALT + 'h')
	 e.menubar->pull_down (e.mb_ino[MBI_HELP]);


      // [Ctrl][L]: force redraw
      else if (is.key == '\f')
	{
	RedrawMap = 1;
	}

      // [Esc], [q]: close
      else if (is.key == YK_ESC || is.key == 'q')
         {
	 if (DragObject)
	    DragObject = false;
	 else if (StretchSelBox)
	    StretchSelBox = false;
	 else
	    {
	    ForgetSelection (&e.Selected);
	    if (!MadeChanges
	     || Confirm (-1, -1, "You have unsaved changes."
				" Do you really want to quit?", 0))
	       break;
	    RedrawMap = 1;
	    }
         }

      // [F1]: pop up "Help" window
      else if (is.key == YK_F1) /* 'F1' */
         {
	 DisplayHelp ();
	 RedrawMap = 1;
         }

      // [Alt][a]: pop up the "About..." window
      else if (is.key == YK_ALT + 'a')
         {
         about_yadex ();
         RedrawMap = 1;
         }

      // [Shift][F1]: save a screen shot into yadex.gif.
      // FIXME doesn't work in the Unix port
      else if (is.key == YK_F1 + YK_SHIFT)
	 {
	 Rgbbmp b;
	 window_to_rgbbmp (0, 0, (int) ScrMaxX + 1, (int) ScrMaxY + 1, b);
	 rgbbmp_to_rawppm (b, "yadex.ppm");
	 //ScreenShot ();
	 }

      // [Shift][F2]: undocumented--test of Entry2
      else if (is.key == YK_F2 + YK_SHIFT)
	 {
	 char buf1[10];
	 char buf2[30];
	 char buf3[20];
	 strcpy (buf1, "buf1");
	 strcpy (buf2, "buf2");
	 strcpy (buf3, "buf3");
	 Entry2 e ("Title of window", "Buf 1%*sBuf 2%*sBuf 3%*s",
	     sizeof buf1 - 1, buf1,
	     sizeof buf2 - 1, buf2,
	     sizeof buf3 - 1, buf3);
	 e.loop ();
	 printf ("bufs: \"%s\", \"%s\", \"%s\"\n", buf1, buf2, buf3);
	 RedrawMap = 1;
	 }

      /* [F2] save level into pwad, prompt for the file name
         every time but keep the same level name. */
      else if (is.key == YK_F2 && Registered)
         {
	 if (! CheckStartingPos ())
	    goto cancel_save;
	 char *outfile;
	 const char *newlevelname;
	 if (levelname)
	    newlevelname = levelname;
	 else
	    {
	    newlevelname = SelectLevel (0);
	    if (! *newlevelname)
	       goto cancel_save;
	    }
	 outfile = GetWadFileName (newlevelname);
	 if (! outfile)
	    goto cancel_save;
	 SaveLevelData (outfile, newlevelname);
	 levelname = newlevelname;
	 // Sigh. Shouldn't have to do that. Level must die !
	 Level = FindMasterDir (MasterDir, levelname);
cancel_save:
	 RedrawMap = 1;
         }

      /* [F3] save level into pwad, prompt for the file name and
         level name. */
      else if (is.key == YK_F3 && Registered)
         {
	 char *outfile;
	 const char *newlevelname;
	 MDirPtr newLevel, oldl, newl;

	 if (! CheckStartingPos ())
	    goto cancel_save_as;
	 newlevelname = SelectLevel (0);
	 if (! *newlevelname)
	    goto cancel_save_as;
	 if (! levelname || y_stricmp (newlevelname, levelname))
	    {
	    /* horrible but it works... */
	    // Horrible indeed -- AYM 1999-07-30
	    newLevel = FindMasterDir (MasterDir, newlevelname);
	    if (! newLevel)
	       nf_bug ("newLevel is NULL");  // Debatable ! -- AYM 2001-05-29
	    if (Level)  // If new level ("create" command), Level is NULL
	       {
	       oldl = Level;
	       newl = newLevel;
	       for (int m = 0; m < 11; m++)
		  {
		  newl->wadfile = oldl->wadfile;
		  if (m > 0)
		     newl->dir = oldl->dir;
		  oldl = oldl->next;
		  newl = newl->next;
		  }
	       }
	    Level = newLevel;
	    }
	 outfile = GetWadFileName (newlevelname);
	 if (! outfile)
	    goto cancel_save_as;
	 SaveLevelData (outfile, newlevelname);
	 levelname = newlevelname;
cancel_save_as:
	 RedrawMap = 1;
         }

      // [F5]: pop up the "Preferences" menu
      else if (is.key == YK_F5
         && e.menubar->highlighted () < 0)
         {
	 Preferences (-1, -1);
	 RedrawMap = 1;
         }

      // [a]: pop up the "Set flag" menu
      else if (is.key == 'a'
         && e.menubar->highlighted () < 0
         && (e.Selected || e.highlighted ()))
         {
         e.modal = 's';  // Set
         if (e.obj_type == OBJ_LINEDEFS)
            {
            menu_linedef_flags->set_title ("Set linedef flag");
            e.modpopup->set (menu_linedef_flags, 0);
            }
         else if (e.obj_type == OBJ_THINGS)
            {
            menu_thing_flags->set_title ("Set thing flag");
            e.modpopup->set (menu_thing_flags, 0);
            }
         }

      // [b]: pop up the "Toggle flag" menu
      else if (is.key == 'b'
         && e.menubar->highlighted () < 0
         && (e.Selected || e.highlighted ()))
         {
         e.modal = 't';  // Toggle
         if (e.obj_type == OBJ_LINEDEFS)
            {
            menu_linedef_flags->set_title ("Toggle linedef flag");
            e.modpopup->set (menu_linedef_flags, 0);
            }
         else if (e.obj_type == OBJ_THINGS)
            {
            menu_thing_flags->set_title ("Toggle thing flag");
            e.modpopup->set (menu_thing_flags, 0);
            }
         }

      // [c]: pop up the "Clear flag" menu
      else if (is.key == 'c'
         && e.menubar->highlighted () < 0
         && (e.Selected || e.highlighted ()))
         {
         e.modal = 'c';  // Clear;
         if (e.obj_type == OBJ_LINEDEFS)
            {
            menu_linedef_flags->set_title ("Clear linedef flag");
            e.modpopup->set (menu_linedef_flags, 0);
            }
         else if (e.obj_type == OBJ_THINGS)
            {
            menu_thing_flags->set_title ("Clear thing flag");
            e.modpopup->set (menu_thing_flags, 0);
            }
         }

      // [F8]: pop up the "Misc. operations" menu
      else if (is.key == YK_F8
         && e.menubar->highlighted () < 0)
         {
         e.modpopup->set (e.menubar->get_menu (MBI_MISC), 1);
         }
      // [F9]: pop up the "Insert a standard object" menu
      else if (is.key == YK_F9
         && e.menubar->highlighted () < 0)
         {
         e.modpopup->set (e.menubar->get_menu (MBI_OBJECTS), 1);
         }

      // [F10]: pop up the "Checks" menu
      else if (is.key == YK_F10
         && e.menubar->highlighted () < 0)
         {
	 CheckLevel (-1, -1);
	 RedrawMap = 1;
         }

      // [Alt][i]: show/hide the info bar
      else if (is.key == YK_ALT + 'i')
         {
	 e.infobar_shown = !e.infobar_shown;
	 RedrawMap = 1;
         }

      // [i]: show/hide the object info boxes
      else if (is.key == 'i')
         {
	 e.objinfo_shown = !e.objinfo_shown;
	 RedrawMap = 1;
	 }

      // [+], [=], wheel: zooming in
      else if (is.key == '+' || is.key == '=' || is.key == YE_WHEEL_UP)
         {
         int r = edit_zoom_in (&e);
	 if (r == 0)
	   RedrawMap = 1;
         }

      // [-], [_], wheel: zooming out
      else if (is.key == '-' || is.key == '_' || is.key == YE_WHEEL_DOWN)
         {
         int r = edit_zoom_out (&e);
	 if (r == 0)
	   RedrawMap = 1;
         }

      // [1] - [9], [0]: set the zoom factor
      else if (is.key >= '0' && is.key <= '9')
         {
         int r = edit_set_zoom (&e, digit_zoom_factors[dectoi(is.key)]);
	 if (r == 0)
	   RedrawMap = 1;
         }

      // [']: centre window on centre of map
      else if (is.key == '\'')
         {
	 update_level_bounds ();
         CenterMapAroundCoords ((MapMinX + MapMaxX) / 2,
            (MapMinY + MapMaxY) / 2);
         RedrawMap = 1;
         }

      // [`]: centre window on centre of map
      // and set zoom to view the entire map
      else if (is.key == '`')
         {
	 int r = zoom_fit (e);
	 if (r == 0)
	   RedrawMap = 1;
         }

      // [Left], [Right], [Up], [Down]:
      // scroll <scroll_less> percents of a screenful.
      else if (is.key == YK_LEFT && MAPX (ScrCenterX) > -20000)
         {
	 OrigX -= (int) ((double) ScrMaxX * scroll_less / 100 / Scale);
	 RedrawMap = 1;
	 }
      else if (is.key == YK_RIGHT && MAPX (ScrCenterX) < 20000)
         {
	 OrigX += (int) ((double) ScrMaxX * scroll_less / 100 / Scale);
	 RedrawMap = 1;
	 }
      else if (is.key == YK_UP && MAPY (ScrCenterY) < 20000)
         {
	 OrigY += (int) ((double) ScrMaxY * scroll_less / 100 / Scale);
	 RedrawMap = 1;
	 }
      else if (is.key == YK_DOWN && MAPY (ScrCenterY) > -20000)
         {
	 OrigY -= (int) ((double) ScrMaxY * scroll_less / 100 / Scale);
	 RedrawMap = 1;
	 }

      // [Pgup], [Pgdn], [Home], [End]:
      // scroll <scroll_more> percents of a screenful.
      else if (is.key == YK_PU && MAPY (ScrCenterY) < /*MapMaxY*/ 20000)
	 {
	 OrigY += (int) ((double) ScrMaxY * scroll_more / 100 / Scale);
	 RedrawMap = 1;
	 }
      else if (is.key == YK_PD && MAPY (ScrCenterY) > /*MapMinY*/ -20000)
	 {
	 OrigY -= (int) ((double) ScrMaxY * scroll_more / 100 / Scale);
	 RedrawMap = 1;
	 }
      else if (is.key == YK_HOME && MAPX (ScrCenterX) > /*MapMinX*/ -20000)
	 {
	 OrigX -= (int) ((double) ScrMaxX * scroll_more / 100 / Scale);
	 RedrawMap = 1;
	 }
      else if (is.key == YK_END && MAPX (ScrCenterX) < /*MapMaxX*/ 20000)
	 {
	 OrigX += (int) ((double) ScrMaxX * scroll_more / 100 / Scale);
	 RedrawMap = 1;
	 }

#if 0
      /* user wants to change the movement speed */
      else if (is.key == ' ')
	 e.move_speed = e.move_speed == 1 ? 20 : 1;
#endif
      else if (is.key == ' ')
         {
         e.extra_zoom = ! e.extra_zoom;
         edit_set_zoom (&e, Scale * (e.extra_zoom ? 4 : 0.25));
         RedrawMap = 1;
         }

      // [Tab], [l], [s], [t], [v]: switch mode
      else if (is.key == YK_TAB || is.key == YK_BACKTAB
       || is.key == 't' || is.key == 'v' || is.key == 'l' || is.key == 's')
	 {
         int    old_mode;
         int    new_mode = -1;
	 int    PrevMode = e.obj_type;
	 SelPtr NewSel;

         // What's the number of the current mode ?
         old_mode = obj_type_to_mode_no (e.obj_type);

         // What's the number of the new mode ?
	 if (is.key == YK_TAB)				// [Tab]
            new_mode = (old_mode + 1) % NB_MODES;
	 else if (is.key == YK_BACKTAB)			// [Shift]-[Tab]
            new_mode = old_mode == 0 ? NB_MODES - 1 : old_mode - 1;
	 else
	    {
	    if (is.key == 't')
	       new_mode = obj_type_to_mode_no (OBJ_THINGS);
	    else if (is.key == 'v')
	       new_mode = obj_type_to_mode_no (OBJ_VERTICES);
	    else if (is.key == 'l')
	       new_mode = obj_type_to_mode_no (OBJ_LINEDEFS);
	    else if (is.key == 's')
	       new_mode = obj_type_to_mode_no (OBJ_SECTORS);
            else
               fatal_error ("changing mode with %04X", is.key);
	    /* unselect all */
	    ForgetSelection (&e.Selected);
	    }

         // Set the object type according to the new mode.
         e.obj_type = modes[new_mode].obj_type;

         // Change the flavour of the "Misc" menu.
         e.menubar->set_menu (e.mb_ino[MBI_MISC],
            e.mb_menu[modes[new_mode].menu_no]);

	 /* special cases for the selection list... */
	 if (e.Selected)
	    {
	    /* select all linedefs bound to the selected sectors */
	    if (PrevMode == OBJ_SECTORS && e.obj_type == OBJ_LINEDEFS)
	       {
	       int l, sd;

	       ObjectsNeeded (OBJ_LINEDEFS, OBJ_SIDEDEFS, 0);
	       NewSel = 0;
	       for (l = 0; l < NumLineDefs; l++)
	          {
		  sd = LineDefs[l].sidedef1;
		  if (sd >= 0 && IsSelected (e.Selected, SideDefs[sd].sector))
		     SelectObject (&NewSel, l);
		  else
		     {
		     sd = LineDefs[l].sidedef2;
		     if (sd >= 0 && IsSelected (e.Selected, SideDefs[sd].sector))
			SelectObject (&NewSel, l);
		     }
	          }
	       ForgetSelection (&e.Selected);
	       e.Selected = NewSel;
	    }
	    /* select all Vertices bound to the selected linedefs */
	    else if (PrevMode == OBJ_LINEDEFS && e.obj_type == OBJ_VERTICES)
	       {
	       ObjectsNeeded (OBJ_LINEDEFS, 0);
	       NewSel = 0;
	       while (e.Selected)
	          {
		  if (!IsSelected (NewSel, LineDefs[e.Selected->objnum].start))
		     SelectObject (&NewSel, LineDefs[e.Selected->objnum].start);
		  if (!IsSelected (NewSel, LineDefs[e.Selected->objnum].end))
		     SelectObject (&NewSel, LineDefs[e.Selected->objnum].end);
		  UnSelectObject (&e.Selected, e.Selected->objnum);
	          }
	       e.Selected = NewSel;
	       }
	    /* select all sectors that have their linedefs selected */
	    else if (PrevMode == OBJ_LINEDEFS && e.obj_type == OBJ_SECTORS)
	       {
	       int l, sd;

	       ObjectsNeeded (OBJ_LINEDEFS, OBJ_SIDEDEFS, 0);
	       NewSel = 0;
	       /* select all sectors... */
	       for (l = 0; l < NumSectors; l++)
		  SelectObject (&NewSel, l);
	       /* ... then unselect those that should not be in the list */
	       for (l = 0; l < NumLineDefs; l++)
		  if (!IsSelected (e.Selected, l))
		     {
		     sd = LineDefs[l].sidedef1;
		     if (sd >= 0)
			UnSelectObject (&NewSel, SideDefs[sd].sector);
		     sd = LineDefs[l].sidedef2;
		     if (sd >= 0)
		       UnSelectObject (&NewSel, SideDefs[sd].sector);
		     }
	       ForgetSelection (&e.Selected);
	       e.Selected = NewSel;
	       }
	    /* select all linedefs that have both ends selected */
	    else if (PrevMode == OBJ_VERTICES && e.obj_type == OBJ_LINEDEFS)
	       {
	       int l;

	       ObjectsNeeded (OBJ_LINEDEFS, 0);
	       NewSel = 0;
	       for (l = 0; l < NumLineDefs; l++)
		  if (IsSelected (e.Selected, LineDefs[l].start)
		   && IsSelected (e.Selected, LineDefs[l].end))
		     SelectObject (&NewSel, l);
	       ForgetSelection (&e.Selected);
	       e.Selected = NewSel;
	       }
	    /* unselect all */
	    else
	       ForgetSelection (&e.Selected);
	    }
	 if (GetMaxObjectNum (e.obj_type) >= 0 && Select0 && ! e.global)
	 {
	    e.highlighted.type = e.obj_type;
	    e.highlighted.num  = 0;
	 }
	 else
	    e.highlighted.nil ();

	 DragObject = false;
	 StretchSelBox = false;
	 RedrawMap = 1;
	 }
#if 0
      // [Ctrl][g]: toggle global mode
      else if (is.key == '\7')
	 {
	 static bool dont_warn = false;
	 bool        ok        = false;
	 if (e.global)
	    ok = true;  // No confirmation needed to switch off
	 else if (dont_warn)
	    ok = true;
	 else
	    {
	    ok = Confirm (-1, -1,
			  "Global mode is experimental and probably highly",
			  "unstable. This means crashes. Are you sure ?");
	    RedrawMap = 1;
	    if (ok)
	       {
	       dont_warn = true;  // User is sure. Won't ask again
	       Notify (-1, -1,
		   "Selection does not work in global mode. Don't",
		   "bother reporting it, I'm aware of it already.");
	       }
	    }
	 if (ok)
	    {
	    ForgetSelection (&e.Selected);
	    e.global = ! e.global;
	    RedrawMap = 1;
	    }
	 }
#endif
      // [e]: Select/unselect all linedefs in non-forked path
      else if (is.key == 'e' && e.highlighted._is_linedef ())
	 {
	 ForgetSelection (&e.Selected);
	 select_linedefs_path (&e.Selected, e.highlighted.num, YS_ADD);
	 RedrawMap = 1;
	 }

      // [Ctrl][e] Select/unselect all linedefs in path
      else if (is.key == '\5' && ! is.shift && e.highlighted._is_linedef ())
	 {
	 select_linedefs_path (&e.Selected, e.highlighted.num, YS_TOGGLE);
	 RedrawMap = 1;
	 }

      // [E]: Select/unselect all 1s linedefs in path
      else if (is.key == 'E' && e.highlighted._is_linedef ())
	 {
	 ForgetSelection (&e.Selected);
	 select_1s_linedefs_path (&e.Selected, e.highlighted.num, YS_ADD);
	 RedrawMap = 1;
	 }

      // [Ctrl][Shift][e]: Select/unselect all 1s linedefs in path
      else if (is.key == '\5' && is.shift && e.highlighted._is_linedef ())
	 {
	 select_1s_linedefs_path (&e.Selected, e.highlighted.num, YS_TOGGLE);
	 RedrawMap = 1;
	 }

      // [G]: to increase the grid step
      else if (is.key == 'G')
	 {
	 if (e.grid_step < e.grid_step_max)
	    e.grid_step *= 2;
	 else
	    e.grid_step = e.grid_step_min;
	 RedrawMap = 1;
	 }

      // [g]: decrease the grid step
      else if (is.key == 'g')
	 {
	 if (e.grid_step > e.grid_step_min)
	    e.grid_step /= 2;
	 else
	    e.grid_step = e.grid_step_max;
	 RedrawMap = 1;
	 }

      // [h]: display or hide the grid
      else if (is.key == 'h')
	 {
	 e.grid_shown = ! e.grid_shown;
	 RedrawMap = 1;
	 }

      // [H]: reset the grid to grid_step_max
      else if (is.key == 'H')
	 {
	 e.grid_step = e.grid_step_max;
	 RedrawMap = 1;
	 }

      // [y]: toggle the snap_to_grid flag
      else if (is.key == 'y')
         {
         e.grid_snap = ! e.grid_snap;
         }

      // [z]: toggle the lock_grip_step flag
      else if (is.key == 'z')
         {
         e.grid_step_locked = ! e.grid_step_locked;
         }
 
      // [r]: toggle the rulers
      else if (is.key == 'r')
	 e.rulers_shown = !e.rulers_shown;

      // [n], [>]: highlight the next object
      else if ((is.key == 'n' || is.key == '>')
	 && (! e.global || e.highlighted ()))
	 {
	 obj_type_t t = e.highlighted () ? e.highlighted.type : e.obj_type;
	 obj_no_t nmax = GetMaxObjectNum (t);
	 if (is_obj (nmax))
	    {
	    if (e.highlighted.is_nil ())
	       {
	       e.highlighted.type = t;
	       e.highlighted.num = 0;
	       }
	    else
	       {
	       e.highlighted.num++;
	       if (e.highlighted.num > nmax)
		  e.highlighted.num = 0;
	       }
	    GoToObject (e.highlighted);
	    RedrawMap = 1;
	    }
	 }

      // [p], [<]: highlight the previous object
      else if ((is.key == 'p' || is.key == '<')
	 && (! e.global || e.highlighted ()))
	 {
	 obj_type_t t = e.highlighted () ? e.highlighted.type : e.obj_type;
	 obj_no_t nmax = GetMaxObjectNum (t);
	 if (is_obj (nmax))
	    {
	    if (e.highlighted.is_nil ())
	       {
	       e.highlighted.type = t;
	       e.highlighted.num = nmax;
	       }
	    else
	       {
	       e.highlighted.num--;
	       if (e.highlighted.num < 0)
		  e.highlighted.num = nmax;
	       }
	    GoToObject (e.highlighted);
	    RedrawMap = 1;
	    }
	 }

      // [j], [#]: jump to object by number
      else if ((is.key == 'j' || is.key == '#')
	 && (! e.global || e.highlighted ()))
	 {
	 Objid default_obj;
	 default_obj.type = e.highlighted () ? e.highlighted.type : e.obj_type;
	 default_obj.num  = e.highlighted () ? e.highlighted.num  : 0;
	 Objid target_obj;
	 input_objid (target_obj, default_obj, -1, -1);
	 if (target_obj ())
	    GoToObject (target_obj);
	 RedrawMap = 1;
	 }

#if 0
      // [c]: clear selection and redraw the map
      else if (is.key == 'c')
	 {
	 ForgetSelection (&e.Selected);
	 RedrawMap = 1;
	 DragObject = false;
	 StretchSelBox = false;
	 }
#endif

      // [o]: copy a group of objects
      else if (is.key == 'o'
         && (e.Selected || e.highlighted ()))
	 {
         int x, y;

	 /* copy the object(s) */
	 if (! e.Selected)
	    SelectObject (&e.Selected, e.highlighted.num);
	 CopyObjects (e.obj_type, e.Selected);
	 /* enter drag mode */
	 //DragObject = true;
         /* AYM 19980619 : got to look into this!! */
	 //e.highlight_obj_no = e.Selected->objnum;

         // Find the "hotspot" in the object(s)
         if (e.highlighted () && ! e.Selected)
            GetObjectCoords (e.highlighted.type, e.highlighted.num, &x, &y);
         else
            centre_of_objects (e.obj_type, e.Selected, &x, &y);

         // Drag the object(s) so that the "hotspot" is under the pointer
         MoveObjectsToCoords (e.obj_type, 0, x, y, 0);
         MoveObjectsToCoords (e.obj_type, e.Selected,
            e.pointer_x, e.pointer_y, 0);
	 RedrawMap = 1;
	 StretchSelBox = false;
	 }

      // [Return]: edit the properties of the current object.
      else if (is.key == YK_RETURN
         && (e.Selected || e.highlighted ()))
	 {
	 if (e.Selected)
	    EditObjectsInfo (0, menubar_out_y1 + 1, e.obj_type, e.Selected);
	 else
	    {
	    SelectObject (&e.Selected, e.highlighted.num);
	    EditObjectsInfo (0, menubar_out_y1 + 1, e.highlighted.type, e.Selected);
	    UnSelectObject (&e.Selected, e.highlighted.num);
	    }
	 RedrawMap = 1;
	 DragObject = false;
	 StretchSelBox = false;
	 }

      // [w]: spin things 1/8 turn counter-clockwise
      else if (is.key == 'w' && e.obj_type == OBJ_THINGS
         && (e.Selected || e.highlighted ()))
	 {
	 if (! e.Selected)
	    {
	    SelectObject (&e.Selected, e.highlighted.num);
	    spin_things (e.Selected, 45);
	    UnSelectObject (&e.Selected, e.highlighted.num);
	    }
	 else
	    {
	    spin_things (e.Selected, 45);
	    }
	 RedrawMap = 1;  /* FIXME: should redraw only the things */
	 DragObject = false;
	 StretchSelBox = false;
	 }

      // [w]: split linedefs and sectors
      else if (is.key == 'w' && e.obj_type == OBJ_LINEDEFS
         && e.Selected && e.Selected->next && ! e.Selected->next->next)
         {
         SplitLineDefsAndSector (e.Selected->next->objnum, e.Selected->objnum);
         ForgetSelection (&e.Selected);
         RedrawMap = 1;
         DragObject = false;
         StretchSelBox = false;
         }

      // [x]: spin things 1/8 turn clockwise
      else if (is.key == 'x' && e.obj_type == OBJ_THINGS
         && (e.Selected || e.highlighted ()))
	 {
	 if (! e.Selected)
	    {
	    SelectObject (&e.Selected, e.highlighted.num);
	    spin_things (e.Selected, -45);
	    UnSelectObject (&e.Selected, e.highlighted.num);
	    }
	 else
	    {
	    spin_things (e.Selected, -45);
	    }
	 RedrawMap = 1;  /* FIXME: should redraw only the things */
	 DragObject = false;
	 StretchSelBox = false;
	 }

      // [x]: split linedefs
      else if (is.key == 'x' && e.obj_type == OBJ_LINEDEFS
         && (e.Selected || e.highlighted ()))
         {
         if (! e.Selected)
            {
            SelectObject (&e.Selected, e.highlighted.num);
            SplitLineDefs (e.Selected);
            UnSelectObject (&e.Selected, e.highlighted.num);
            }
         else
            SplitLineDefs (e.Selected);
         RedrawMap = 1;
         DragObject = false;
         StretchSelBox = false;
         }

      // [Ctrl][x]: exchange objects numbers
      else if (is.key == 24)
	 {
	 if (! e.Selected
	     || ! e.Selected->next
	     || (e.Selected->next)->next)
	    {
	    Beep ();
	    Notify (-1, -1, "You must select exactly two objects", 0);
	    RedrawMap = 1;
	    }
	 else
	    {
	    exchange_objects_numbers (e.obj_type, e.Selected, true);
	    RedrawMap = 1;
	    }
	 }

      // [Ctrl][k]: cut a slice out of a sector
      else if (is.key == 11 && e.obj_type == OBJ_LINEDEFS
         && e.Selected && e.Selected->next && ! e.Selected->next->next)
      {
         sector_slice (e.Selected->next->objnum, e.Selected->objnum);
         ForgetSelection (&e.Selected);
         RedrawMap = 1;
         DragObject = false;
         StretchSelBox = false;
      }
      
      // [Del]: delete the current object
      else if (is.key == YK_DEL
         && (e.Selected || e.highlighted ())) /* 'Del' */
	 {
	 if (e.obj_type == OBJ_THINGS
	  || Expert
	  || Confirm (-1, -1,
		(e.Selected && e.Selected->next ?
		     "Do you really want to delete these objects?"
		   : "Do you really want to delete this object?"),
		(e.Selected && e.Selected->next ?
		     "This will also delete the objects bound to them."
	           : "This will also delete the objects bound to it.")))
	    {
	    if (e.Selected)
	       DeleteObjects (e.obj_type, &e.Selected);
	    else
	       DeleteObject (e.highlighted);
	    }
         // AYM 1998-09-20 I thought I'd add this
         // (though it doesn't fix the problem : if the object has been
         // deleted, HighlightObject is still called with a bad object#).
         e.highlighted.nil ();
	 DragObject = false;
	 StretchSelBox = false;
	 RedrawMap = 1;
	 }

      // [Ins]: insert a new object
      else if (is.key == YK_INS || is.key == YK_INS + YK_SHIFT) /* 'Ins' */
	 {
	 SelPtr cur;
         int prev_obj_type = e.obj_type;

	 /* first special case: if several vertices are
	    selected, add new linedefs */
	 if (e.obj_type == OBJ_VERTICES
	    && e.Selected && e.Selected->next)
	    {
	    int firstv;
            int obj_no = OBJ_NO_NONE;

	    ObjectsNeeded (OBJ_LINEDEFS, 0);
	    if (e.Selected->next->next)
	       firstv = e.Selected->objnum;
	    else
	       firstv = -1;
	    e.obj_type = OBJ_LINEDEFS;
	    /* create linedefs between the vertices */
	    for (cur = e.Selected; cur->next; cur = cur->next)
	       {
	       /* check if there is already a linedef between the two vertices */
	       for (obj_no = 0; obj_no < NumLineDefs; obj_no++)
		  if ((LineDefs[obj_no].start == cur->next->objnum
		    && LineDefs[obj_no].end   == cur->objnum)
		   || (LineDefs[obj_no].end   == cur->next->objnum
		    && LineDefs[obj_no].start == cur->objnum))
		     break;
	       if (obj_no < NumLineDefs)
		  cur->objnum = obj_no;
	       else
		  {
		  InsertObject (OBJ_LINEDEFS, -1, 0, 0);
		  e.highlighted.type = OBJ_LINEDEFS;
		  e.highlighted.num  = NumLineDefs - 1;
		  LineDefs[e.highlighted.num].start = cur->next->objnum;
		  LineDefs[e.highlighted.num].end = cur->objnum;
		  cur->objnum = e.highlighted.num;  // FIXME cur = e.highlighted
		  }
	       }
	    /* close the polygon if there are more than 2 vertices */
	    if (firstv >= 0 && is.shift)
	       {
	       e.highlighted.type = OBJ_LINEDEFS;
	       for (e.highlighted.num = 0;
                    e.highlighted.num < NumLineDefs;
                    e.highlighted.num++)
		  if ((LineDefs[e.highlighted.num].start == firstv
		    && LineDefs[e.highlighted.num].end   == cur->objnum)
		   || (LineDefs[e.highlighted.num].end   == firstv
		    && LineDefs[e.highlighted.num].start == cur->objnum))
		     break;
	       if (e.highlighted.num < NumLineDefs)
		  cur->objnum = obj_no;
	       else
		  {
		  InsertObject (OBJ_LINEDEFS, -1, 0, 0);
		  e.highlighted.type = OBJ_LINEDEFS;
		  e.highlighted.num  = NumLineDefs - 1;
		  LineDefs[e.highlighted.num].start = firstv;
		  LineDefs[e.highlighted.num].end   = cur->objnum;
		  cur->objnum = e.highlighted.num;  // FIXME cur = e.highlighted
		  }
	       }
	    else
	       UnSelectObject (&e.Selected, cur->objnum);
	    }
	 /* second special case: if several linedefs are selected,
	    add new sidedefs and one sector */
	 else if (e.obj_type == OBJ_LINEDEFS && e.Selected)
	    {
	    ObjectsNeeded (OBJ_LINEDEFS, 0);
	    for (cur = e.Selected; cur; cur = cur->next)
	       if (LineDefs[cur->objnum].sidedef1 >= 0
		&& LineDefs[cur->objnum].sidedef2 >= 0)
		  {
		  char msg[80];

		  Beep ();
		  sprintf (msg, "Linedef #%d already has two sidedefs", cur->objnum);
		  Notify (-1, -1, "Error: cannot add the new sector", msg);
		  break;
		  }
	    if (! cur)
	       {
	       e.obj_type = OBJ_SECTORS;
	       InsertObject (OBJ_SECTORS, -1, 0, 0);
	       e.highlighted.type = OBJ_SECTORS;
	       e.highlighted.num  = NumSectors - 1;
	       for (cur = e.Selected; cur; cur = cur->next)
		  {
		  InsertObject (OBJ_SIDEDEFS, -1, 0, 0);
		  SideDefs[NumSideDefs - 1].sector = e.highlighted.num;
		  ObjectsNeeded (OBJ_LINEDEFS, OBJ_SIDEDEFS, 0);
		  if (LineDefs[cur->objnum].sidedef1 >= 0)
		     {
		     int s;

		     s = SideDefs[LineDefs[cur->objnum].sidedef1].sector;
		     if (s >= 0)
			{
			Sectors[e.highlighted.num].floorh = Sectors[s].floorh;
			Sectors[e.highlighted.num].ceilh = Sectors[s].ceilh;
			strncpy (Sectors[e.highlighted.num].floort,
			   Sectors[s].floort, WAD_FLAT_NAME);
			strncpy (Sectors[e.highlighted.num].ceilt,
			   Sectors[s].ceilt, WAD_FLAT_NAME);
			Sectors[e.highlighted.num].light = Sectors[s].light;
			}
		     LineDefs[cur->objnum].sidedef2 = NumSideDefs - 1;
		     LineDefs[cur->objnum].flags = 4;
		     strncpy (SideDefs[NumSideDefs - 1].tex3,
			 "-", WAD_TEX_NAME);
		     strncpy (SideDefs[LineDefs[cur->objnum].sidedef1].tex3,
			 "-", WAD_TEX_NAME);
		     }
		  else
		     LineDefs[cur->objnum].sidedef1 = NumSideDefs - 1;
		  }
	       ForgetSelection (&e.Selected);
	       SelectObject (&e.Selected, e.highlighted.num);
	       }
	    }
	 /* normal case: add a new object of the current type */
	 else
	    {
	    ForgetSelection (&e.Selected);
	    /* FIXME how do you insert a new object of type T if
	       no object of that type already exists ? */
	    obj_type_t t = e.highlighted () ? e.highlighted.type : e.obj_type;
	    InsertObject (t, e.highlighted.num,
	       edit_mapx_snapped (&e, e.pointer_x),
	       edit_mapy_snapped (&e, e.pointer_y));
	    e.highlighted.type = t;
	    e.highlighted.num  = GetMaxObjectNum (e.obj_type);
	    if (e.obj_type == OBJ_LINEDEFS)
	       {
	       int v1 = LineDefs[e.highlighted.num].start;
	       int v2 = LineDefs[e.highlighted.num].end;
	       if (! Input2VertexNumbers (-1, -1,
		 "Choose the two vertices for the new linedef", &v1, &v2))
		  {
		  DeleteObject (e.highlighted);
		  e.highlighted.nil ();
		  }
	       else
		  {
		  LineDefs[e.highlighted.num].start = v1;
		  LineDefs[e.highlighted.num].end   = v2;
		  }
	       }
	    else if (e.obj_type == OBJ_VERTICES)
	       {
	       SelectObject (&e.Selected, e.highlighted.num);
	       if (AutoMergeVertices (&e.Selected, e.obj_type, 'i'))
		  RedrawMap = 1;
	       ForgetSelection (&e.Selected);
	       }
	    }

         // Mode-changing code, duplicated from above.
         // As RQ would say: "I hate to do that". So do I.
         // The best solution would be to have a mode
         // changing function that only changes e.obj_type
         // and emits a "mode change" message. Another part
         // of the code, responsible for the menus, would
         // intercept that message and update them. I don't
         // have the time (as of 1998-12-14) to do it now.
         if (e.obj_type != prev_obj_type)
	    {
	    int new_mode;

	    // What's the number of the new mode ?
            new_mode = obj_type_to_mode_no (e.obj_type);

	    // Change the flavour of the "Misc" menu.
	    e.menubar->set_menu (e.mb_ino[MBI_MISC],
	       e.mb_menu[modes[new_mode].menu_no]);
	    }

	 DragObject = false;
	 StretchSelBox = false;
	 RedrawMap = 1;
	 }

      // [!] Debug info (not documented)
      else if (is.key == '!')
         {
         DumpSelection (e.Selected);
         }

      // [R] Render 3D view (AJA)
      else if (is.key == 'R')
        {
        Render3D ();
        RedrawMap = 1;
        }

      // [@] Show font (not documented)
      else if (is.key == '@')
         {
         show_font ();
	 RedrawMap = 1;
         }

      // [|] Show colours (not documented)
      else if (is.key == '|')
         {
         show_colours ();
	 RedrawMap = 1;
         }

      // [Ctrl][b] Select linedefs whose sidedefs reference non-existant sectors
      else if (is.key == 2)
	 {
	 bad_sector_number (&e.Selected);
	 RedrawMap = 1;
         }

      // [Ctrl][p] Examine game palette (not documented)
      else if (is.key == 16)
         {
	 Palette_viewer pv;
	 pv.run ();
	 RedrawMap = 1;
         }

      // [Ctrl][r] Xref for sidedef (not documented)
      else if (is.key == 18)
	 {
	 xref_sidedef ();
	 }

      // [Ctrl][s] List secret sectors (not documented)
      else if (is.key == 19)
	 {
	 secret_sectors ();
	 }

      // [Ctrl][t] List tagged linedefs or sectors
      else if (is.key == 20)
	 {
	 if (e.highlighted._is_sector ())
	    list_tagged_linedefs (Sectors[e.highlighted.num].tag);
	 else if (e.highlighted._is_linedef ())
	    list_tagged_sectors (LineDefs[e.highlighted.num].tag);
	 else
	    Beep ();
	 }

      // [Ctrl][u] Select linedefs with unknown type (not documented)
      else if (is.key == 21)
	 {
	 unknown_linedef_type (&e.Selected);
	 RedrawMap = 1;
	 }

      // [Ctrl][v] Toggle between "snap vertex" tool and normal tool
      // (not documented)
      else if (is.key == 22)
	 {
	 if (e.tool == TOOL_NORMAL)
	    {
	    e.tool = TOOL_SNAP_VERTEX;
	    printf ("Switched to snap vertex tool."
		  " Press [Ctrl][v] to switch back to normal tool.\n");
	    }
	 else
	    {
	    e.tool = TOOL_NORMAL;
	    printf ("Switched back to normal tool.\n");
	    }
	 }

      // [&] Show object numbers
      else if (is.key == '&')
         {
         e.show_object_numbers = ! e.show_object_numbers;
         RedrawMap = 1;
         }

      // [%] Show things sprites
      else if (is.key == '%')
	 {
	 e.show_things_sprites = ! e.show_things_sprites;
	 e.show_things_squares = ! e.show_things_sprites;  // Not a typo !
	 RedrawMap = 1;
	 }

      // Load BEHAVIOR lump (JL)
      else if (is.key == 'b')
         {
         char *acsfile;
         const char *acsname;
         if (levelname)
            acsname = levelname;
         else
            acsname = "behavior";
         acsfile = GetBehaviorFileName (acsname);
         FILE* f = fopen(acsfile, "rb");
         if (f)
            {
            FreeFarMemory(Behavior);
            fseek(f, 0, SEEK_END);
            BehaviorSize = ftell(f);
            Behavior = (u8*)GetFarMemory(BehaviorSize);
            fseek(f, 0, SEEK_SET);
            fread(Behavior, BehaviorSize, 1, f);
            fclose(f);
            }
         RedrawMap = 1;
      }

      /* user likes music */
      else if (is.key)
	 {
	 Beep ();
	 }
      }

   /*
    *	Step 4 -- Misc. cruft
    */

   done :

   // Auto-scrolling: scroll the map automatically
   // when the mouse pointer rests near the edge
   // of the window.
   // Scrolling is disabled when a pull-down menu
   // is visible because it would be annoying to see
   // the map scrolling while you're searching
   // through the menus.

   if (is.in_window
      && autoscroll
      && ! is.scroll_lock
      && e.menubar->pulled_down () < 0)
      {
      unsigned distance;		// In pixels

#define actual_move(total,dist) \
   ((int) (((total * autoscroll_amp / 100)\
   * ((double) (autoscroll_edge - dist) / autoscroll_edge))\
   / Scale))

      distance = is.y;
      // The reason for the second member of the condition
      // is that we don't want to scroll when the user is
      // simply reaching for a menu...
      if (distance <= autoscroll_edge
         && e.menubar->is_under_menubar_item (is.x) < 0)
	 {
	 if (MAPY (ScrCenterY) < /*MapMaxY*/ 20000)
	    {
	    OrigY += actual_move (ScrMaxY, distance);
	    RedrawMap = 1;
	    }
	 }

      distance = ScrMaxY - is.y;
      if (distance <= autoscroll_edge)
	 {
	 if (MAPY (ScrCenterY) > /*MapMinY*/ -20000)
	    {
	    OrigY -= actual_move (ScrMaxY, distance);
	    RedrawMap = 1;
	    }
	 }

      distance = is.x;
      if (distance <= autoscroll_edge)
	 {
	 if (MAPX (ScrCenterX) > /*MapMinX*/ -20000)
	    {
	    OrigX -= actual_move (ScrMaxX, distance);
	    RedrawMap = 1;
	    }
	 }

      // The reason for the second member of the condition
      // is that we don't want to scroll when the user is
      // simply reaching for the "Help" menu...
      // Note: the ordinate "3 * FONTH" is of course not
      // critical. It's just a rough approximation.
      distance = ScrMaxX - is.x;
      if (distance <= autoscroll_edge && (unsigned) is.y >= 3 * FONTH)
	 {
	 if (MAPX (ScrCenterX) < /*MapMaxX*/ 20000)
	    {
	    OrigX += actual_move (ScrMaxX, distance);
	    RedrawMap = 1;
	    }
	 }
      }

   /*
    *	Step 5 -- Process events that were generated
    */

   done2:

   // Process events that were generated
   if (has_event (YE_ZOOM_CHANGED) && ! e.grid_step_locked)
      {
      get_event ();
      edit_grid_adapt (&e);
      RedrawMap = 1;
      }

   if (RedrawMap)
      e.edisplay->need_refresh ();
   }

delete e.edisplay;
delete e.selbox;
delete e.menubar;
for (size_t n = 0; n < MBM_HELP; n++)
   delete e.mb_menu[n];

delete menu_linedef_flags;
delete menu_thing_flags;
// int foobar = view.x;
}


/*
 *	zoom_fit - adjust zoom factor to make level fit in window
 *
 *	Return 0 on success, non-zero on failure.
 */
static int zoom_fit (edit_t& e)
{
  // Empty level, 100% will be fine.
  if (NumVertices == 0)
    return edit_set_zoom (&e, 1.0);

  update_level_bounds ();
  double xzoom;
  if (MapMaxX - MapMinX)
     xzoom = .95 * ScrMaxX / (MapMaxX - MapMinX);
  else
     xzoom = 1;
  double yzoom;
  if (MapMaxY - MapMinY)
     yzoom = .9 * ScrMaxY / (MapMaxY - MapMinY);
  else
     yzoom = 1;
  int r = edit_set_zoom (&e, y_min (xzoom, yzoom));
  if (r != 0)
    return 1;
  CenterMapAroundCoords ((MapMinX + MapMaxX) / 2, (MapMinY + MapMaxY) / 2);
  return 0;
}

/*
   get the name of the BEHAVIOR lump file (returns NULL on Esc)
*/

static char *GetBehaviorFileName (const char *levelname)
{
#define BUFSZ 79
  char *outfile = (char *) GetMemory (BUFSZ + 1);

  /* get the file name */
  // If no name, find a default one
  if (! levelname)
  {
    levelname = "behavior";
  }

  al_scpslower (outfile, levelname, BUFSZ);
  al_saps (outfile, ".o", BUFSZ);
  InputFileName (-1, -1, "Name of the BEHAVIOR script file:", BUFSZ, outfile);
  /* escape */
  if (outfile[0] == '\0')
  {
    FreeMemory (outfile);
    return 0;
  }
  return outfile;
}


