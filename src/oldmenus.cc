/*
 *	oldmenus.cc
 *	Old-fashioned menu functions, somewhat similar to the
 *	ones DEU 5.21 had in menus.c. Since they contain an event
 *	loop of their own, they can't handle expose and configure
 *	notify events properly. When I have the time, I'll
 *	replace this system by a modal widgets stack in
 *	edisplay.cc.
 *	AYM 1998-12-03
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
#include "events.h"
#include "gfx.h"
#include "menu.h"
#include "oldmenus.h"


static int loop (Menu *menu, int x, int y, int item_no);


/*
 *	vDisplayMenu - create a menu from variable arguments and call loop()
 *
 *	Display and execute a popup menu defined with variable
 *	arguments.
 *
 *	Return the same thing as Menu::process_event().
 */
int vDisplayMenu (int x0, int y0, const char *title, ...)
{
  va_list args;

  va_start (args, title);
  Menu menu (title, args);
  va_end (args);
  int r = loop (&menu, x0, y0, 0);
  return r + 1;
}


/*
 *	DisplayMenuList - create a menu from a list and call loop()
 *
 *	Display and execute a menu contained in a list. Each
 *	menu item string is obtained by calling (*getstr)() with
 *	a pointer on the list item.
 *
 *	Return the number of the selected item (0-based) or -1
 *	if exited by [Esc].
 *
 *	This function is deprecated.
 */
int DisplayMenuList (
  int		x0,
  int		y0,
  const char	*menutitle,
  al_llist_t	*list,
  const char	*(*getstr)(void *),
  int		*item_no)
{
  Menu menu (menutitle, list, getstr);
  int r = loop (&menu, x0, y0, item_no ? *item_no : 0);
  if (item_no && r >= 0)
    *item_no = r;
  return r;
}


/*
 *	DisplayMenuList - create a menu from a Menu_data and call loop()
 *
 *	Display and execute a menu contained in a Menu_data.
 *
 *	Return the number of the selected item (0-based) or -1
 *	if exited by [Esc].
 *
 *	This function is deprecated.
 */
int DisplayMenuList (
  int		x0,
  int		y0,
  const char	*menutitle,
  Menu_data&    menudata,
  int		*item_no)
{
  Menu menu (menutitle, menudata);
  int r = loop (&menu, x0, y0, item_no ? *item_no : 0);
  if (item_no && r >= 0)
    *item_no = r;
  return r;
}


/*
 *	loop - display and execute a menu
 *
 *	Return the number of the selected item (0-based) or -1
 *	if exited by [Esc].
 *
 *	This function is nothing more than a quick and dirty
 *	hack, provided as a stopgap until the widget stack is
 *	implemented and the code uses it. 
 */
static int loop (Menu *menu, int x, int y, int item_no)
{
  menu->set_popup         (true);
  menu->set_force_numbers (true);
  menu->set_coords        (x, y);
  menu->set_visible       (true);
  menu->set_item_no       (item_no);
  for (;;)
  {
    menu->draw ();
    if (has_event ())
    {
      is.key = get_event ();
      // If we had called get_input_status(), XNextEvent()
      // would have been called and we wouldn't have to do that.
      XFlush (dpy);
    }
    else
      get_input_status ();
    if (is.key == YE_EXPOSE)
      menu->clear ();  // Force menu to redraw itself from scratch
    else if (is.key == YE_BUTL_PRESS
      && ((int) is.x < menu->get_x0 ()
	 || (int) is.x > menu->get_x1 ()
	 || (int) is.y < menu->get_y0 ()
	 || (int) is.y > menu->get_y1 ()))
      return -1;
    else
    {
      int r = menu->process_event (&is);
      if (r == MEN_CANCEL)
	 return -1;
      else if (r >= 0)
	 return r;
    }
  }
}

