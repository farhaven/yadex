/*
 *	edisplay.cc
 *	The edisplay_c (edit window display) class
 *	AYM 1998-09-16
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
The edisplay_c class is the interface between the edit_c class
(the edit window at an abstract level) and the various edwidget_c
descended classes (the widgets used to represent the edit window
on the display). Its role is to allow the edit_c class to feel
the bliss that comes from ignoring the gory details of handling
the display in a somewhat asynchronous and optimized fashion.

The other role of the edisplay_c class is to act as a kind of
rudimentary geometry manager for the widgets.

About undrawing :
Some widgets are able to undraw themselves. Some are not. If we
want to undraw them, then we should clear the window and redraw
everything from scratch. That's what the method need_to_clear()
is for; if the widget should be undrawn but can't undraw itself,
need_to_clear() returns true.

For the time being, the set of widgets is hard coded into the
edisplay_c class. But this needs not be. In the future, all this
should be replaced by a loop through a list of widgets.
*/


#include "yadex.h"
#include "_edit.h"
#include "drawmap.h"
#include "edisplay.h"
#include "editobj.h"
#include "gfx.h"
#include "highlt.h"
#include "infobar.h"
#include "levels.h"	// Level
#include "menu.h"
#include "menubar.h"
#include "modpopup.h"
#include "objects.h"
#include "objinfo.h"
#include "selbox.h"
#include "spot.h"
#include "wadfile.h"


edisplay_c::edisplay_c (edit_t *e)
{
  this->e = e;
  pointer_scnx = 0;
  pointer_scny = 0;
  refresh_needed = 1;
  highlight = new highlight_c;
  objinfo   = new objinfo_c;
  infobar   = new infobar_c;
}


edisplay_c::~edisplay_c ()
{
  delete highlight;
  delete objinfo;
}


// FIXME this should not be a separate method
void edisplay_c::highlight_object (Objid& obj)
{
  highlight->set (obj);
  objinfo->set (obj.type, obj.num);
}


// FIXME this should not be a separate method
void edisplay_c::forget_highlight ()
{
  highlight->unset ();
  /* I don't unset objinfo because, as it can't undraw
     itself, it would lead to redrawing everything from
     scratch everytime an object passes out of focus. */
  //objinfo->unset ();
}


void edisplay_c::need_refresh ()
{
  refresh_needed = 1;
}


void edisplay_c::refresh ()
{
  int redraw_from_scratch;

  // The poor hacker's geometry manager (FIXME: this needs work!)
  infobar->set_x1 (ScrMaxX);
  infobar->set_y1 (ScrMaxY);
  infobar->set_x0 (0);
  infobar->set_y0 (ScrMaxY - (infobar->req_height () - 1));
  objinfo->set_y1 (e->infobar_shown ? infobar->get_y0 () - 1 : ScrMaxY);

  /* Extract the interesting data from the edit_c object
     and feed it to the widgets. */
  infobar->set_visible          (e->infobar_shown);
  infobar->set_file_name        (Level
				   ? Level->wadfile->pathname ()
				   : 0);
  infobar->set_level_name       (Level
				   ? (const char *) Level->dir.name
				   : 0);
  infobar->set_obj_type         (e->obj_type);
  infobar->set_changes          (MadeMapChanges ? 2 : (MadeChanges ? 1 : 0));
  infobar->set_grid_snap        (e->grid_snap);
  infobar->set_grid_step_locked (e->grid_step_locked);
  infobar->set_scale            (Scale);
  infobar->set_grid_step        (e->grid_step);
  if (e->pointer_in_window)
    infobar->set_pointer (e->pointer_x, e->pointer_y);
  else
    infobar->unset_pointer ();


  redraw_from_scratch =
    refresh_needed
    || e->selbox->need_to_clear   ()
    || e->spot->need_to_clear     ()
    || highlight->need_to_clear   ()
    || objinfo->need_to_clear     ()
    || infobar->need_to_clear     ()
    || e->menubar->need_to_clear  ()
    || e->modpopup->need_to_clear ();
   
  /* If we can update the display in an incremental fashion
     (not from scratch), do it by telling all widgets, from
     the top to the bottom, to undraw themselves if necessary. */
  if (! redraw_from_scratch)
  {
    e->modpopup->undraw ();
    e->menubar->undraw  ();
    infobar->undraw     ();
    objinfo->undraw     ();
    highlight->undraw   ();
    e->spot->undraw     ();
    e->selbox->undraw   ();
  }

  /* If a complete refresh is required, call the clear()
     method for all widgets to make them aware that they're
     not visible anymore. */
  else
  {
    e->selbox->clear   ();
    e->spot->clear     ();
    highlight->clear   ();
    objinfo->clear     ();
    infobar->clear     ();
    e->menubar->clear  ();
    e->modpopup->clear ();

    // A piece of ad-hockery
    objinfo->unset ();

    // As we said, "from scratch".
    ClearScreen ();
    draw_map (e);     // FIXME should be widgetized
    // draw_menubar ();  // FIXME should be widgetized
    HighlightSelection (e->obj_type, e->Selected); // FIXME should be widgetized
    refresh_needed = 0;
  }

  /* Tell all widgets from to bottom to the top
     to draw themselves if necessary. */
  e->selbox->draw   ();
  e->spot->draw     ();
  highlight->draw   ();
  if (e->objinfo_shown)
    objinfo->draw     ();
  infobar->draw     ();
  e->menubar->draw  ();
  e->modpopup->draw ();

  // Refresh the physical display
  update_display ();
}


