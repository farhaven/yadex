/*
 *	lists.h
 *	AYM 2000-04-29
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


#ifndef YH_LISTS  /* DO NOT INSERT ANYTHING BEFORE THIS LINE */
#define YH_LISTS

#include <string>
#include <vector>

#include "img.h"

using std::string;
using std::vector;


/* AYM 19980112
This is the format of the block that the callback function (*hookfunc)()
and its caller InputNameFromListWithFunc() use to communicate. It
includes the old hookfunc parameters (x0, y0, x1, y1, name) plus a
couple of new parameters. The purpose of those new parameters is :
  1. Give the caller an easy way to know the actual size of the
     image (for shift-F1).
  2. Hence let the caller display the size (remember "yes you
     can laugh at the way I did it..." ?).
  3. Provide the caller with various statistics such as number of
     patches in current texture etc. Good tourist information :)
  4. Give the caller a way to know whether the image has been
     drawn (i.e. "ready for shift-F1 ?")
  5. Give the caller something to call Img::save() on when user
     presses [Shift][F1].
Why did I create a structure instead of just adding parameters to the
hookfunc ? Just for the sake of streamlining the prototype of
InputNameFromListWithFunc() :)
In the following,
- "expected" means "set by caller, read by callee"
- "returned" means "set by callee, read by caller"
- "both"     means... both :-)

img
	The Img is part of the structure. The callee should not
	make any assumptions regarding the contents and property
	of img ; it should call resize() and set_opaque()
	systematically. The callee should not clear the Img
	before exiting, because the caller may need it for the
	save-to-file function.

	For that reason, the caller should put the whole image
	in img, *not* clipped to the dimensions of the screen
	area on which it will be displayed. Unfortunately, that
	is currently (2000-10-31) not possible because of
	limitations in the Sticker class.

	The caller may do whatever it pleases with img.
*/
typedef struct
{
  int x0;           // [expected] Top left corner of where to draw image
  int y0;
  int x1;           // [expected] Bottom right corner
  int y1;
  int disp_x0;      // [returned] Top left corner and bottom right corner
  int disp_y0;      // of area that was drawn on by callee. This is so that
  int disp_x1;      // the caller knows what needs to be cleared...
  int disp_y1;
  int xofs;         // [expected] Top left corner of image in buffer
  int yofs;
  const char *name; // [expected] Name of image to display
  int flags;        // [both]     Flags
  Img img;          // [returned] Image buffer (clipped !)
  int width;        // [returned] Width of image before clipping
  int height;       // [returned] Height of image before clipping
  int npatches;     // [returned] Textures only : number of patches
  int maxpatches;   // [expected] Textures: if !0 only render that many patches
  Lump_loc lump_loc;// [returned] Location of lump that was just displayed
} hookfunc_comm_t;
const int HOOK_DRAWN      = 1 << 0;	// Image is completely drawn
const int HOOK_SIZE_VALID = 1 << 1;	// width and height are valid
const int HOOK_DISP_SIZE  = 1 << 2;	// Caller should display "widthxheight"
const int HOOK_SPECTRAL   = 1 << 3;	// Render picture with a spectral look
const int HOOK_PATCH      = 1 << 4;	// Use patch_dir.loc_by_name()
const int HOOK_SPRITE     = 1 << 5;	// Use wad_res.sprites.loc_by_name()
const int HOOK_LOC_VALID  = 1 << 6;	// lump_loc is valid
const int HOOK_ROOT       = 1 << 7;	// .name is the prefix. Use loc_by_root


string InputNameFromListWithFunc (int, int, const char *,
  vector<string>, size_t, string name, int, int,
  void (*hookfunc)(hookfunc_comm_t *),
  char flags_to_pass_to_callback = 0);
string InputNameFromList (int, int, const char *, vector<string>, string);

#endif  /* DO NOT ADD ANYTHING AFTER THIS LINE */
