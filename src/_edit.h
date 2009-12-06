/*
 *	_edit.h
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


#ifndef YH__EDIT	/* DO NOT INSERT ANYTHING BEFORE THIS LINE */
#define YH__EDIT


#include "objid.h"


class edisplay_c;
class Menu;
class menubar_c;
class modpopup_c;
class selbox_c;
class spot_c;


// The numbers of the items on the menu bar
enum
{
  MBI_FILE,
  MBI_EDIT,
  MBI_VIEW,
  MBI_SEARCH,
  MBI_MISC,
  MBI_OBJECTS,
  MBI_CHECK,
  MBI_HELP,
  MBI_COUNT
};


// The numbers of the actual menus (Menu objects)
enum
{
  MBM_FILE,
  MBM_EDIT,
  MBM_VIEW,
  MBM_SEARCH,
  MBM_MISC_L,  // The "Misc. operations" menus changes with the mode
  MBM_MISC_S,
  MBM_MISC_T,
  MBM_MISC_V,
  MBM_OBJECTS,
  MBM_CHECK,
  MBM_HELP,
  MBM_COUNT
};


typedef enum
{
  TOOL_NORMAL,
  TOOL_SNAP_VERTEX
} tool_t;


/* This structure holds all the data necessary to an edit window. */
// FIXME: make a class of it.
typedef struct
{
  public :
    int   mb_ino[MBI_COUNT];	// The numbers of the items on the menu bar
    Menu *mb_menu[MBM_COUNT];	// The actual menu objects

    int move_speed;		// Movement speed.
    int extra_zoom;		// Act like the zoom was 4 times what it is
    int obj_type;		// The mode (OBJ_LINEDEF, OBJ_SECTOR...)
    bool global;		// Global mode (experimental)
    tool_t tool;		// The current tool
    int grid_step;		// The grid step
    int grid_step_min;		// The floor of the grid step
    int grid_step_max;		// The ceiling of the grid step
    int grid_step_locked;	// Whether the grid step is locked
    int grid_shown;		// Whether the grid is shown
    int grid_snap;		// Whether objects forced to be on the grid
    bool infobar_shown;		// Whether the info bar is shown
    bool objinfo_shown;		// Whether the object info boxes are shown
    bool show_object_numbers;	// Whether the object numbers are shown
    bool show_things_squares;	// Whether the things squares are shown
    bool show_things_sprites;	// Whether the things sprites are shown
    int rulers_shown;		// Whether the rulers are shown (unused ?)
    int pointer_x;		// Map coordinates of pointer
    int pointer_y;
    int pointer_in_window;	// If false, pointer_[xy] are not meaningful.
    Objid clicked;		// The object that was under the pointer when
				// when the left click occurred. If clicked on
				// empty space, == CANVAS.
    int click_ctrl;		// Was Ctrl pressed at the moment of the click?
    unsigned long click_time;	// Date of last left click in ms
    Objid highlighted;		// The highlighted object
    SelPtr Selected;		// Linked list of selected objects (or NULL)

    selbox_c   *selbox;		// The selection box
    edisplay_c *edisplay;	// The display manager
    menubar_c  *menubar;	// The menu bar
    spot_c     *spot;		// The insertion spot

    modpopup_c *modpopup;	// The modal popup menu (only one at a time!)
    char modal;
} edit_t;


#endif  /* DO NOT ADD ANYTHING AFTER THIS LINE */
