/*
 *	modpopup.h
 *	The modpopup_c class: a modal popup menu widget.
 *	Basically a wrapper for the Menu class.
 *	AYM 1998-12-16
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


#include "edwidget.h"
#include "menu.h"


class modpopup_c : public edwidget_c
{
  public :

    /*
     *	EditorLoop() side methods
     */
    modpopup_c ()
    {
      menu = 0;
      menu_disp = 0;
    }

    void set (Menu *menu, int force_numbers)
    {
      if (menu_disp)
	menu_disp->set_visible (0);
      this->menu = menu;
      menu->set_popup         (1);
      menu->set_force_numbers (force_numbers);
      menu->set_coords        (-1, -1);
      menu->set_item_no       (0);
      menu->set_visible       (1);
    }
    
    void set (Menu *menu, int force_numbers, int x,int y)
    {
      if (menu_disp)
	menu_disp->set_visible (0);
      this->menu = menu;
      menu->set_popup         (1);
      menu->set_force_numbers (force_numbers);
      menu->set_coords        (x,y);
      menu->set_item_no       (0);
      menu->set_visible       (1);
    }

    void unset ()
    {
      if (menu_disp)
	menu_disp->set_visible (0);
      menu = 0;
    }

    Menu *get ()
    {
      return menu;
    }

    /*
     *	edisplay_c side methods
     */
    void draw ()
    {
      if (menu)
      {
	menu->draw ();
	menu_disp = menu;
      }
    }

    void undraw ()
    {
      if (menu_disp)
	{
	  menu_disp->undraw ();
	  //menu_disp = 0;
	}
    }

    int can_undraw ()
    {
      if (menu_disp)
	return menu_disp->can_undraw ();
      else
	return 1;
    }

    int need_to_clear ()
    {
      if (menu_disp)
	return menu_disp->need_to_clear ();
      else
	return 0;
    }

    void clear ()
    {
      if (menu_disp)
	menu_disp->clear ();
    }

  private :

    Menu *menu;
    Menu *menu_disp;
};


