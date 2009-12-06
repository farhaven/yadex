/*
 *	menubar.cc
 *	AYM 1998-08-21
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
This modules contains (almost) all the functions related to the
menu bar.

CAVEATS

One thing that I didn't even attempt to handle is the case where
the screen/window is not large enough to hold all the menu items.
This will cause the left-aligned and right-aligned items to overlap
and even to "go off" the screen/window.

AYM 1998-08-21
*/


#include "yadex.h"
#include "gfx.h"
#include "menubar.h"
#include "menu.h"


/*
 *	menubar_c
 */
menubar_c::menubar_c ()
{
stale_coords          = 1;
nitems                = 0;
stale_x0_x1           = 1;
spacing               = FONTW;
bar_visible           = 1;
bar_visible_disp      = 0;
highlighted_no        = -1;
highlighted_no_disp   = -1;
pulled_down_no        = -1;
pulled_down_no_disp   = -1;
pulled_down_menu      = 0;
pulled_down_menu_disp = 0;
#if 0
for (size_t n = 0; n < MAX_ITEMS; n++)
   {
   item_menu[n] = 0;
   }
#endif
}


/*
 *	compute_menubar_coords
 *	Call this one second and each time the screen size
 *	changes.
 *	(<scrx0>, <scry0>) are the coordinates
 *	of the top left corner of the screen/window.
 *	(<scrx1>, <scry1>) are the coordinates
 *	of the bottom right corner of the screen/window.
 */
void menubar_c::compute_menubar_coords (int scrx0, int scry0, int scrx1, int scry1)
{
// Just to prevent the compiler from emitting an
// annoying warning about that parameter being unused.
scry1 = 0;

menubar_out_x0  = scrx0;
menubar_in_x0   = menubar_out_x0 + BOX_BORDER;
menubar_text_x0 = menubar_in_x0;

menubar_out_x1  = scrx1;
menubar_in_x1   = menubar_out_x1 - BOX_BORDER;
menubar_text_x1 = menubar_in_x1;

menubar_out_y0  = scry0;
menubar_in_y0   = menubar_out_y0  + BOX_BORDER;
menubar_text_y0 = menubar_in_y0   + NARROW_VSPACING;
menubar_text_y1 = menubar_text_y0 + FONTH - 1;
menubar_in_y1   = menubar_text_y1 + NARROW_VSPACING;
menubar_out_y1  = menubar_in_y1   + BOX_BORDER;

stale_coords = 0;
stale_x0_x1 = 1;
}


/*
 *	add_menubar_item
 *	Add a new item to the menu bar.
 *	If <right_aligned> is non zero, that item will be
 *	right aligned on the menu bar.
 *	Returns the number of the newly created item.
 */
int menubar_c::add_item (const char *text, int shortcut_index,
   int right_aligned, Menu *menu)
{
if (nitems >= MAX_ITEMS)
   fatal_error ("Too many items on menu bar");
if (shortcut_index < 0 || shortcut_index >= (int) strlen (text))
   fatal_error ("add_menubar_item: shortcut_index out of range");
item_text[nitems]           = text;
item_shortcut_index[nitems] = shortcut_index;
item_right_aligned[nitems]  = right_aligned;
item_menu[nitems]           = menu;
nitems++;
stale_x0_x1 = 1;
return nitems - 1;
}


/*
 *	set_menu
 *	Change the menu for an existing menu bar item.
 */
void menubar_c::set_menu (int number, Menu *menu)
{
if (number >= nitems)
   fatal_error ("set_menu: bad mbi#");
item_menu[number] = menu;

// So that need_to_clear() knows something has changed
if (number == pulled_down_no)
   pulled_down_menu = menu;
}


/*
 *	get_menu
 *	Return a pointer on the menu for an existing menu bar item.
 */
Menu *menubar_c::get_menu (int number)
{
if (number >= nitems)
   fatal_error ("set_menu: bad mbi#");
return item_menu[number];
}


/*
 *	highlight
 *	Highlight the menu bar item number <number>.
 *	Use <number> < 0 to clear the highlighting.
 */
void menubar_c::highlight (int number)
{
highlighted_no = number;
if (pulled_down_no >= 0)
   {
   pulled_down_no = number;
   if (number >= 0)
      pulled_down_menu = item_menu[number];
   else
      pulled_down_menu = 0;
   }
}


/*
 *	highlighted
 *	Return the number of the menu bar item that is highlighted
 *	or < 0 if none.
 */
int menubar_c::highlighted ()
{
return highlighted_no;
}


/*
 *	pull_down
 *	Pull down the menu under the menu bar item number <number>.
 *	Use <number> < 0 to "unroll".
 */
void menubar_c::pull_down (int number)
{
if (number >= 0 && number != pulled_down_no)
   item_menu[number]->set_item_no (0);

pulled_down_no = number;

if (number >= 0)
   {
   pulled_down_menu = item_menu[number];
   // Pulling down a menu implies highlighting
   // the corresponding item on the menu bar.
   highlight (number);
   }
else
   pulled_down_menu = 0;
}


/*
 *	pulled_down
 *	Return the number of the menu bar item that is pulled down
 *	or < 0 if none.
 */
int menubar_c::pulled_down ()
{
return pulled_down_no;
}


/*
 *	clear
 */
void menubar_c::clear ()
{
//for (int n = 0; n < nitems; n++)
//   item_menu[n]->clear ();

if (pulled_down_menu_disp)
   pulled_down_menu_disp->clear ();
bar_visible_disp      = 0;
highlighted_no_disp   = -1;
pulled_down_no_disp   = -1;
pulled_down_menu_disp = 0;
}


/*
 *	draw
 *	Draw the menu bar according to its current state.
 */
void menubar_c::draw ()
{
// Draw the menu bar itself
if (bar_visible && ! bar_visible_disp
   || highlighted_no != highlighted_no_disp)
   {
   if (stale_x0_x1)
      compute_x0_x1 ();
   push_colour (menu_colour[0][0].bg);  /* 1 */
   DrawScreenBox3D (0, 0, ScrMaxX, menubar_out_y1);
   set_colour (menu_colour[0][0].fg);
   for (int n = 0; n < nitems; n++)
      {
      if (n == highlighted_no)
	 {
	 push_colour (menu_colour[0][1].bg);  /* 2 */
	 DrawScreenBox (item_x0[n], menubar_in_y0, item_x1[n], menubar_in_y1);
	 set_colour (menu_colour[0][1].fg);
	 }
      DrawScreenString (item_x0[n] + spacing, menubar_text_y0, item_text[n]);
      DrawScreenString (item_x0[n] + spacing + item_shortcut_index[n] * FONTW,
		      menubar_text_y0 + FONTU, "_");
      if (n == highlighted_no)
	 pop_colour ();  /* 2 */
      }
   pop_colour ();  /* 1 */
   bar_visible_disp    = 1;
   highlighted_no_disp = highlighted_no;
   }

// If there is a menu that used to be visible
// but isn't anymore, let it be aware of it.
if (pulled_down_menu_disp && pulled_down_menu != pulled_down_menu_disp)
   {
   pulled_down_menu_disp->clear ();
   pulled_down_menu_disp->set_visible (0);
   }

// Draw the pulled down menu (if any)
if (pulled_down_menu)
   {
   int x, y;
   menubar_item_coords (pulled_down_no, &x, &y);
   pulled_down_menu->set_popup         (0);
   pulled_down_menu->set_force_numbers (0);
   pulled_down_menu->set_coords        (x, y);
   pulled_down_menu->set_visible       (1);
   pulled_down_menu->draw              ();
   }

pulled_down_no_disp   = pulled_down_no;
pulled_down_menu_disp = pulled_down_menu;
}


/*
 *	is_on_menubar_item
 *	Returns the number of the menu bar item which should
 *	be pulled down if the user clicked at screen coords (x,y).
 *	Returns -1 if none.
 */
int menubar_c::is_on_menubar_item (int x, int y)
{
if (stale_coords)
   fatal_error ("Called iomi before cc");
if (x < menubar_in_x0 || x > menubar_in_x1
 || y < menubar_in_y0 || y > menubar_in_y1)
   return -1;
if (stale_x0_x1)
   compute_x0_x1 ();
for (int n = 0; n < nitems; n++)
   if (x >= item_x0[n] && x <= item_x1[n])
      return n;
return -1;
}


/*
 *	is_under_menubar_item
 *	Returns whether the screen abscissa <scrx> is "under"
 *	(or "over") one of the menu bar items.
 *	This weird function is used in only one place ; the
 *	autoscroll code. If the mouse pointer is at the same
 *	abscissa as a menubar item, it probably means that the
 *	user is reaching for the menus so don't scroll.
 */
int menubar_c::is_under_menubar_item (int scrx)
{
if (stale_coords)
   fatal_error ("Called iumi before cc");
if (stale_x0_x1)
   compute_x0_x1 ();
for (int n = 0; n < nitems; n++)
   if (scrx >= item_x0[n] && scrx <= item_x1[n])
      return n;
return -1;
}


/*
 *	compute_x0_x1
 *	Fill in item_x0 and item_x1.
 */
void menubar_c::compute_x0_x1 ()
{
int x_left;
int x_right;
int n;

if (stale_coords)
   fatal_error ("Called compute_x0_x1 before compute_coords");
x_left  = menubar_text_x0;
x_right = menubar_text_x1;
for (n = 0; n < nitems; n++)
   {
   int item_width = 2 * spacing + strlen (item_text[n]) * FONTW;

   /* This item is right-aligned ?
      Place it to the left of the last right-aligned item. */
   if (item_right_aligned[n])
      {
      item_x1[n] = x_right;
      x_right -= item_width;
      item_x0[n] = x_right + 1;
      }

   /* It's left-aligned.
      Place it to the right of the last left-aligned item. */
   else
      {
      item_x0[n] = x_left;
      x_left += item_width;
      item_x1[n] = x_left - 1;
      }
   }
stale_x0_x1 = 0;
}


/*
 *	menubar_item_coords
 *	Returns coordinates of top left corner of a pulled down
 *	menu.
 *	This function will be removed when menus are pulled down
 *	by menubar.cc itself.
 */
void menubar_c::menubar_item_coords (int item_no, int *x, int *y)
{
if (item_no < 0 || item_no >= nitems)
   fatal_error ("menubar_item_coords passed bad item no. %d", item_no);
if (stale_coords)
   fatal_error ("Called menubar_item_coords before compute_coords");
if (stale_x0_x1)
   compute_x0_x1 ();
*x = item_x0[item_no];
*y = menubar_out_y1 + 1;
}



