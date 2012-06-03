/*
 *	editzoom.cc
 *	Zoom functions
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


#include "yadex.h"
#include <math.h>
#include "_edit.h"
#include "editzoom.h"
#include "events.h"
#include "gfx.h"


double digit_zoom_factors[10];


void edit_zoom_init ()
{
  double factor = digit_zoom_base / 100.0;
  double step   = digit_zoom_step != 0 ? (digit_zoom_step + 100.0) / 100
				       : pow (2, -.5);
  for (int i = 1; i <= 10; i++, factor *= step)
    digit_zoom_factors[i % 10] = factor;
}


int edit_zoom_in (edit_t *e)
{
  if (! e) return 1;  // Prevent compiler warning about unused .p.
  double step = zoom_step != 0 ? (zoom_step + 100.0) / 100 : sqrt (2);
  if (Scale * step > 10.0)
    return 1;
  OrigX += (int) ((is.x - ScrCenterX) / Scale);
  OrigY += (int) ((ScrCenterY - is.y) / Scale);
  Scale *= step;
  OrigX -= (int) ((is.x - ScrCenterX) / Scale);
  OrigY -= (int) ((ScrCenterY - is.y) / Scale);
  send_event (YE_ZOOM_CHANGED);
  return 0;
}


int edit_zoom_out (edit_t *e)
{
  if (! e) return 1;  // Prevent compiler warning about unused .p.
  double step = zoom_step != 0 ? (zoom_step + 100.0) / 100 : sqrt (2);
  if (Scale / step < 0.05)
    return 1;
  OrigX += (int) ((is.x - ScrCenterX) / Scale);
  OrigY += (int) ((ScrCenterY - is.y) / Scale);
  Scale /= step;
  OrigX -= (int) ((is.x - ScrCenterX) / Scale);
  OrigY -= (int) ((ScrCenterY - is.y) / Scale);
  send_event (YE_ZOOM_CHANGED);
  return 0;
}


int edit_set_zoom (edit_t *e, double zoom_factor)
{
  if (! e) return 1;  // Prevent compiler warning about unused .p.
  if (zoom_factor < 0.05)
    zoom_factor = 0.05;
  if (zoom_factor > 10.0)
    zoom_factor = 10.0;
  OrigX += (int) ((is.x - ScrCenterX) / Scale);
  OrigY += (int) ((ScrCenterY - is.y) / Scale);
  Scale = zoom_factor;
  OrigX -= (int) ((is.x - ScrCenterX) / Scale);
  OrigY -= (int) ((ScrCenterY - is.y) / Scale);
  send_event (YE_ZOOM_CHANGED);
  return 0;
}

