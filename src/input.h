/*
 *	input.h
 *	User input (mouse and keyboard)
 *	AYM 1998-06-16
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


typedef unsigned short inpev_t;
typedef struct
  {
  // Window events
  int  width;		// New window width
  int  height;		// New window height
  // Mouse events
  char in_window;	// <>0 iff mouse pointer is in window
  char butl;		// <>0 iff left mouse button is depressed
  char butm;		// <>0 iff middle mouse button is depressed
  char butr;		// <>0 iff right mouse button is depressed
  int x;		// Mouse pointer position
  int y;
  // Keyboard
  char shift;		// <>0 iff either [Shift] is pressed
  char ctrl;		// <>0 iff either [Ctrl] is pressed
  char alt;		// <>0 iff either [Alt]/[Meta] is pressed
  char scroll_lock;	// Always 0 for the moment
  // General
  inpev_t key;		// Code of last event (key, mouse, resize, expose...)
  unsigned long time;	// Date of event in ms (1)
  } input_status_t;

/* Notes:
(1) Defined only for the following events: key, button, motion
    enter and leave.
*/

/* Events and key codes */
const inpev_t YK_BACKSPACE	= '\b';
const inpev_t YK_TAB		= '\t';
const inpev_t YK_RETURN		= '\r';
const inpev_t YK_ESC		= 0x1b;
const inpev_t YK_DEL		= 0x7f;
const inpev_t YK_		= 256;
const inpev_t YK_BACKTAB	= 257;
const inpev_t YK_DOWN		= 258;
const inpev_t YK_END		= 259;
const inpev_t YK_F1		= 260;
const inpev_t YK_F2		= 261;
const inpev_t YK_F3		= 262;
const inpev_t YK_F4		= 263;
const inpev_t YK_F5		= 264;
const inpev_t YK_F6		= 265;
const inpev_t YK_F7		= 266;
const inpev_t YK_F8		= 267;
const inpev_t YK_F9		= 268;
const inpev_t YK_F10		= 269;
const inpev_t YK_HOME		= 270;
const inpev_t YK_INS		= 271;
const inpev_t YK_LEFT		= 272;
const inpev_t YK_PU		= 273;
const inpev_t YK_PD		= 274;
const inpev_t YK_RIGHT		= 275;
const inpev_t YK_UP		= 276;
const inpev_t YK__LAST		= 277;  // Marks the end of key events

// Those are not key numbers but window events
const inpev_t YE_RESIZE		= 278;
const inpev_t YE_EXPOSE		= 279;

// Those are not key numbers but mouse events
const inpev_t YE_BUTL_PRESS	= 280;
const inpev_t YE_BUTL_RELEASE	= 281;
const inpev_t YE_BUTM_PRESS	= 282;
const inpev_t YE_BUTM_RELEASE	= 283;
const inpev_t YE_BUTR_PRESS	= 284;
const inpev_t YE_BUTR_RELEASE	= 285;
const inpev_t YE_WHEEL_UP	= 286;	// Negative, normally bound to button 4
const inpev_t YE_WHEEL_DOWN	= 287;	// Positive, normally bound to button 5
const inpev_t YE_ENTER		= 288;
const inpev_t YE_LEAVE		= 289;
const inpev_t YE_MOTION		= 290;

// Those are not key numbers but application events
// (i.e. generated internally)
const inpev_t YE_ZOOM_CHANGED	= 291;

// Those are ORed with the other key numbers :
const inpev_t YK_SHIFT		= 0x2000;
const inpev_t YK_CTRL		= 0X4000; 
const inpev_t YK_ALT		= 0x8000;

/* Defined in input.c -- see the comment there */
extern input_status_t is;

/* Whether c is an "ordinary" character, that is a printable (non-
   control) character. We cannot use isprint because its argument
   must be <= 255 and in considers A0H-FFH non-printable. */
#define is_ordinary(c) ((c) < 256 && ((c) & 0x60) && (c) != 0x7f)

/* Apply this to is.key to find out whether it contains a key press event. */
#define event_is_key(n) (((n) & (YK_SHIFT-1)) > 0 && ((n) & (YK_SHIFT-1)) < YK__LAST)

void init_input_status ();
void get_input_status ();
int  has_input_event ();
int  have_key ();
int  get_key ();
void get_key_or_click ();
const char *key_to_string (inpev_t k);


