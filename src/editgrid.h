/*
 *	editgrid.h
 *	AYM 1998-11-09
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


void edit_grid_adapt (edit_t *e);


/*
 *	edit_mapx_snapped
 *	Return <mapx> snapped to grid
 *	(or unchanged is snap_to_grid is off)
 */
inline int edit_mapx_snapped (const edit_t *e, int mapx)
{
if (! e->grid_snap || e->grid_step == 0)
   return mapx;
if (mapx >= 0)
   return e->grid_step * ((mapx + e->grid_step / 2) / e->grid_step);
else
   return e->grid_step * ((mapx - e->grid_step / 2) / e->grid_step);
}


/*
 *	edit_mapy_snapped
 *	Return <mapy> snapped to grid
 *	(or unchanged is snap_to_grid is off)
 */
inline int edit_mapy_snapped (const edit_t *e, int mapy)
{
if (! e->grid_snap || e->grid_step == 0)
   return mapy;
if (mapy >= 0)
   return e->grid_step * ((mapy + e->grid_step / 2) / e->grid_step);
else
   return e->grid_step * ((mapy - e->grid_step / 2) / e->grid_step);
}
