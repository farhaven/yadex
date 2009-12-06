/*
 *	mouse.cc
 *	DOS mouse interface
 *	RQ sometime in 1993 or 1994.
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


#ifdef Y_BGI

#include <dos.h>

/* mouse interrupt number */
#define MOUSE 0x33

/* the global data */
bool UseMouse;			/* is there a mouse driver? */


/*
   initialize the mouse driver
*/

void CheckMouseDriver (void)
{
union  REGS  regs;
struct SREGS sregs;

regs.x.ax = 0x0000;
int86(MOUSE, &regs, &regs);
if (regs.x.ax == 0xffff)
   {
   UseMouse = true; /* mouse */
#if defined Y_BGI && defined CIRRUS_PATCH
   /*
      note from RQ:
	 This test is temporary and should be removed in DEU 5.3
	 We should create a better "fake cursor" by using the
	 mouse callback function.  Remember to remove the callback
	 when DEU exits...
   */
   if (CirrusCursor)
      {
      regs.x.ax = 0x000C;
      regs.x.cx = 0x0001;
      regs.x.dx = FP_OFF (MouseCallBackFunction);
      sregs.es  = FP_SEG (MouseCallBackFunction);
      int86x (MOUSE, &regs, &regs, &sregs);
      }
#endif /* Y_BGI && CIRRUS_PATCH */
#ifdef AYM_MOUSE_HACKS
   regs.x.ax = 0x001b;
   int86x (MOUSE, &regs, &regs, &sregs);
   LogMessage ("Mouse: h=%d v=%d\n", regs.x.bx, regs.x.cx);
   regs.x.ax = 0x000f;
   regs.x.cx = MouseMickeysH;
   regs.x.dx = MouseMickeysV;
   int86x (MOUSE, &regs, &regs, &sregs);
#endif  /* AYM_MOUSE_HACKS */
   }
else
UseMouse = false; /* no mouse */
}


/*
   show the pointer
*/

void ShowMousePointer (void)
{
union REGS regs;

if (UseMouse)
   {
   regs.x.ax = 0x0001;
   int86(MOUSE, &regs, &regs);
   }
}


/*
   hide the pointer
*/

void HideMousePointer (void)
{
union REGS regs;

if (UseMouse)
   {
   regs.x.ax = 0x0002;
   int86(MOUSE, &regs, &regs);
   }
}


/*
   read pointer coordinates
*/

void GetMouseCoords (int *x, int *y, int *buttons)
{
static int oldx = 42, oldy = 42;
union REGS regs;

regs.x.ax = 0x0003;
int86(MOUSE, &regs, &regs);
if (x != NULL)
   *x = regs.x.cx;
if (y != NULL)
   *y = regs.x.dx;
if (buttons != NULL)
   *buttons = regs.x.bx;
if (*x != oldx || *y != oldy)
   {
   LogMessage ("Mouse %5d %5d\n", *x, *y);
   oldx = *x;
   oldy = *y;
   }
}



/*
   change pointer coordinates
*/

void SetMouseCoords (int x, int y)
{
union REGS regs;

regs.x.ax = 0x0004;
regs.x.cx = (unsigned) x;
regs.x.dx = (unsigned) y;
int86(MOUSE, &regs, &regs);
}



/*
   set horizontal and vertical limits (constrain pointer in a box)
*/

void SetMouseLimits( int x0, int y0, int x1, int y1)
{
union REGS regs;

regs.x.ax = 0x0007;
regs.x.cx = (unsigned) x0;
regs.x.dx = (unsigned) x1;
int86(MOUSE, &regs, &regs);
regs.x.ax = 0x0008;
regs.x.cx = (unsigned) y0;
regs.x.dx = (unsigned) y1;
int86(MOUSE, &regs, &regs);
}



/*
   reset horizontal and vertical limits
*/

void ResetMouseLimits (void)
{
union REGS regs;

regs.x.ax = 0x0007;
regs.x.cx = (unsigned) 0;
regs.x.dx = (unsigned) ScrMaxX;
int86(MOUSE, &regs, &regs);
regs.x.ax = 0x0008;
regs.x.cx = (unsigned) 0;
regs.x.dx = (unsigned) ScrMaxY;
int86(MOUSE, &regs, &regs);
}



/*
   mouse callback function
*/

void MouseCallBackFunction (void)
{
#if defined Y_BGI && defined CIRRUS_PATCH
if (CirrusCursor)
   SetHWCursorPos(_CX, _DX);
#endif /* Y_BGI && CIRRUS_PATCH */
}
#endif  /* #ifdef Y_BGI */

/* end of file */
