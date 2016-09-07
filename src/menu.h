/*
 *	menu.h
 *	AYM 1998-08-15
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


#ifndef YH_MENUS  /* DO NOT INSERT ANYTHING BEFORE THIS LINE */
#define YH_MENUS


#include <stdarg.h>
#include <vector>
#include "edwidget.h"


typedef struct 
{
  acolour_t bg;
  acolour_t fg;
} colour_pair_t;

/* First subscript :  0 = normal, 1 = greyed out
   Second subscript : 0 = normal, 1 = highlighted */
extern const colour_pair_t menu_colour[2][2];

const unsigned char MIF_NACTIVE = 0;
const unsigned char MIF_SACTIVE = 1;
const unsigned char MIF_VACTIVE = 2;
const unsigned char MIF_FACTIVE = 3;

const unsigned char MIF_NTICK = 0 << 2;
const unsigned char MIF_STICK = 1 << 2;
const unsigned char MIF_VTICK = 2 << 2;
const unsigned char MIF_FTICK = 3 << 2;

extern const char *MI_SEPARATION;

typedef void *micbarg_t;		// Argument of callback function
typedef bool (*micb_t) (micbarg_t);	// Pointer to callback function

// Values returned by process_event()
const int MEN_CANCEL  = -1;  // Exit by [Esc]. Caller should destroy the menu.
const int MEN_OTHER   = -2;  // Got other event and processed it.
const int MEN_INVALID = -3;  // Got invalid event. Caller should process it.


class Menu_data;
class Menu_priv;


/*
 *	Menu - a menu class
 */
class Menu : public edwidget_c
{
  public :
    // Ctors
    Menu (const string title, ...);
    Menu (const string title, va_list argp);
    Menu (const string title, al_llist_t *list, const char *(*getstr)(void *));
    Menu (const string title, const Menu_data& menudata);
    ~Menu ();

    // Configuration
    void set_coords (int x, int y);
    void set_title (const string title);
    void set_item_no (int item_no);
    void set_popup (bool popup);
    void set_force_numbers (bool force_numbers);
    void set_visible (bool visible);
    void set_ticked (size_t item_no, bool ticked);
    void set_active (size_t item_no, bool active);

    // Event processing
    int process_event (const input_status_t *is);
    inpev_t last_shortcut_key ();

    // Widget functions
    void draw ();
    void undraw ();
    int can_undraw ();
    int need_to_clear ();
    void clear ();
    int req_width ();
    int req_height ();
    int get_x0 ();
    int get_y0 ();
    int get_x1 ();
    int get_y1 ();

  private :
    Menu (const Menu&);			// Too lazy to implement it
    Menu& operator= (const Menu&);	// Too lazy to implement it
    Menu_priv *priv;
};


#endif  /* DO NOT ADD ANYTHING AFTER THIS LINE */
