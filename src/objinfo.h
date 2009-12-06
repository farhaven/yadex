/*
 *	objinfo.h
 *	AYM 1998-09-20
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


class objinfo_c : public edwidget_c
   {
   public :
      objinfo_c ();
      void set (int obj_type, int obj_no)
      {
        this->obj_no   = obj_no;
	this->obj_type = obj_type;
      }

      void set_y1 (int y1)
	 { out_y1 = y1; }

      /* Methods declared in edwidget_c */
      void unset ()
	 { obj_no = OBJ_NO_NONE; }

      void draw ();

      void undraw ()
         { }  // Sorry, I don't know how to undraw myself

      int can_undraw ()
         { return 0; }  // I don't have the ability to undraw myself

      int need_to_clear ()
	 { return is_obj (obj_no_disp) && ! is_obj (obj_no); }

      void clear ()
      {
	for (size_t n = 0; n < MAX_BOXES; n++)
	  box_disp[n] = false;
	obj_no_disp = OBJ_NO_NONE;
      }

   private :
      static const size_t MAX_BOXES = 10;
      bool box_disp[MAX_BOXES];	// Is the box already drawn ?
      int obj_no;        // The no. of the object we should display info about
      int obj_type;      // The type of the object we should display info about
      int obj_no_disp;	 // The no. and type of the object for which info
      int obj_type_disp; // is really displayed.
      int prev_sector;   // No. of the last sector for which info was displayed
      int prev_floorh;   // Its floor height.
      int prev_ceilh;    // Its ceiling height.
      int out_y1;        // The bottom outer edge of the info window.
   };

