/*
 *	x11.c
 *	X11-specific stuff
 *	AYM 1999-08-03
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
#include <X11/Xutil.h>
#include "gfx.h"
#include "x11.h"


static bool _have_error = false;
static unsigned char _error_code;


/*
 *	x_bell
 *	Ring the bell
 */
void x_bell ()
{
  if (dpy)
    XBell (dpy, 0);
  else
    nf_bug ("x_bell: not connected");
}


/*
 *	x_catch_on
 *	Setup things so that from now on, any X protocol errors
 *	will be handled by x_error_handler instead of the
 *	default handler (that has the annoying property of
 *	calling exit()).
 */
void x_catch_on ()
{
  XSetErrorHandler (x_error_handler);
  XSynchronize (dpy, True);
  x_clear_error ();
}


/*
 *	x_catch_off
 *	Restore the default error handler.
 */
void x_catch_off ()
{
  XSynchronize (dpy, False);
  XSetErrorHandler (0);
}


/*
 *	x_error_handler
 *	An error handler that does not exit.
 */
int x_error_handler (Display *dpy, XErrorEvent *e)
{
  _have_error = true;
  _error_code = e->error_code;  // We're only interested in the error code
  return 0;
}


/*
 *	x_clear_error
 *	Call this before attempting an operation that might
 *	cause an error that you want to catch.
 */
void x_clear_error ()
{
  _have_error = false;
}


/*
 *	x_error
 *	Return a string corresponding to the last error caught
 *	or a NULL pointer if no error has occured since last
 *	call to x_clear_error().
 */
const char *x_error ()
{
  if (! _have_error)
    return 0;
  static char buf[100];
  XGetErrorText (dpy, _error_code, buf, sizeof buf);
  return buf;
}
