/*
 *	menubar.h
 *	Header for menubar.c
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
The menubar widget keeps the following state information ;
- whether the menu bar should be displayed (always true),
- whether the menu bar is actually displayed,
- the number of the item that should be highlighted (sometimes "none"),
- the number of the item that is actually highlighted (sometimes "none"),
- the number of the menu that should be pulled down (sometimes "none"),
- the number of the menu that is actually pulled down (sometimes "none")
- a pointer on the menu that should be pulled down (sometimes "none"),
- a pointer on the menu that is actually pulled down (sometimes "none")
*/


#include "edwidget.h"
class Menu;


static const int MAX_ITEMS = 10;


class menubar_c : public edwidget_c
   {
   public :
      menubar_c ();

      int add_item (const char *text, int shortcut_index, int right_aligned,
         Menu *menu);
      void set_menu (int number, Menu *menu);
      Menu *get_menu (int number);
      void compute_menubar_coords (int scrx0, int scrx1, int scry0, int scry1);
      void highlight (int number);
      void pull_down (int number);
      int highlighted ();
      int pulled_down ();
      int is_on_menubar_item (int scrx, int scry);
      int is_under_menubar_item (int scrx);
      void menubar_item_coords (int item_no, int *x, int *y);

      /*
       *	widget
       */
      void draw ();
      void undraw () { }  // I can't undraw myself
      int can_undraw () { return 0; }  // I can't undraw myself

      int need_to_clear ()
         {
         return bar_visible_disp && ! bar_visible
            || pulled_down_no_disp && ! pulled_down_no
            || pulled_down_menu_disp != pulled_down_menu;
         }

      void clear ();

   private :
      void compute_x0_x1 ();

      int spacing;          		// Horizontal spacing around item name

      int stale_coords;			// Should menubar_* be recalculated ?
      int menubar_out_x0;		// Edge of the menu bar, includ. border
      int menubar_out_x1;
      int menubar_out_y0;
      int menubar_out_y1;
      int menubar_in_x0;		// Edge of the menu bar, exclud. border
      int menubar_in_x1;
      int menubar_in_y0;
      int menubar_in_y1;
      int menubar_text_x0;		// Edge of the text area of the menu bar
      int menubar_text_x1;
      int menubar_text_y0;
      int menubar_text_y1;

      int nitems;			// Number of items
      const char *item_text[MAX_ITEMS];	// Definition of items
      int item_shortcut_index[MAX_ITEMS];
      int item_right_aligned[MAX_ITEMS];
      Menu *item_menu[MAX_ITEMS];
       
      int stale_x0_x1;         		// Should item_x? be recalculated ?
      int item_x0[MAX_ITEMS];		// Left edge of items, includ. spacing
      int item_x1[MAX_ITEMS];		// Right edge of items, includ. spacing

      int bar_visible;			// Should the bar be visible ?
      int bar_visible_disp;		// Is the bar actually visible ?
      int highlighted_no;		// # of the item that should be h.l.
      int highlighted_no_disp;		// # of the item that is actually h.l.
      int pulled_down_no;		// # of menu that should be p.d.
      int pulled_down_no_disp;		// # of menu that is actually p.d.
      Menu *pulled_down_menu;		// Menu that should be p.d.
      Menu *pulled_down_menu_disp;	// Menu that is actually p.d.
   };


