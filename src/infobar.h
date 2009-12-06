/*
 *	infobar.h
 *	Info bar
 *	AYM 1998-10-10
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


class infobar_c : public edwidget_c
   {
   public :
      infobar_c ();

      void set_visible (int visible) { this->visible = visible; }

      void set_file_name (const char *file_name)
	 { this->file_name = file_name; }

      void set_level_name (const char *level_name)
	 { this->level_name = level_name; }

      void set_obj_type  (int obj_type)  { this->obj_type = obj_type; }
      void set_changes   (int changes)   { this->changes = changes; }
      void set_grid_snap (int grid_snap) { this->grid_snap = grid_snap; }

      void set_grid_step_locked (int grid_step_locked)
	 { this->grid_step_locked = grid_step_locked; } 

      void set_scale     (float scale)   { this->scale = scale; }
      void set_grid_step (int grid_step) { this->grid_step = grid_step; }

      void set_pointer (int x, int y)
	 {
	 flags |= pointer_set;
	 pointer_x = x;
	 pointer_y = y;
	 }

      void unset_pointer ()
	 { flags &= ~ pointer_set; }

      /* Methods declared in edwidget_c */
      void draw ();

      void undraw ()
         { } // I can't undraw myself

      int can_undraw ()
         { return 0; }  // I don't have the ability to undraw myself

      int  need_to_clear ()
         { return visible_disp && ! visible; } // I can't undraw myself

      void clear ();

      int req_width ()
         { return -1; /* Infinite */ }

      int req_height ()
         { return 2 * BOX_BORDER + 2 * NARROW_VSPACING + FONTH; }

      void set_x0 (int x0)
         { out_x0 = x0; text_x0 = x0 + BOX_BORDER + NARROW_HSPACING; }

      void set_y0 (int y0)
         { out_y0 = y0; text_y0 = y0 + BOX_BORDER + NARROW_VSPACING; }

      void set_x1 (int x1)
         { out_x1 = x1; text_x1 = x1 - BOX_BORDER - NARROW_HSPACING; }

      void set_y1 (int y1)
         { out_y1 = y1; text_y1 = y1 - BOX_BORDER - NARROW_VSPACING; }

      int get_x0 () { return out_x0; }
      int get_y0 () { return out_y0; }
      int get_x1 () { return out_x1; }
      int get_y1 () { return out_y1; }

   private :
      static const char FILE_NAME_UNSET[1];  // A special pointer value 
      static const char LEVEL_NAME_UNSET[1];  // A special pointer value
      static const int pointer_set      = 1;
      static const int pointer_disp_set = 2;

      int visible;
      int visible_disp;
      const char *file_name;
      const char *file_name_disp;
      const char *level_name;
      const char *level_name_disp;
      int obj_type;
      int obj_type_disp;
      int changes;
      int changes_disp;
      int grid_snap;
      int grid_snap_disp;
      int grid_step_locked;
      int grid_step_locked_disp;
      float scale;
      float scale_disp;
      int grid_step;
      int grid_step_disp;
      int pointer_x;
      int pointer_x_disp;
      int pointer_y;
      int pointer_y_disp;

      int out_x0;
      int out_y0;
      int out_x1;
      int out_y1;
      int text_x0;
      int text_y0;
      int text_x1;
      int text_y1;

      int flags;
   };


