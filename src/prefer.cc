/*
 *	prefer.cc
 *	AYM 1998-10-17
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

#include <string>
#include <vector>

#include "yadex.h"
#include "entry.h"
#include "flats.h"
#include "gfx.h"
#include "levels.h"
#include "oldmenus.h"
#include "prefer.h"
#include "textures.h"

using std::string;
using std::to_string;
using std::vector;

/*
   display a "Preferences" menu (change default textures, etc.)
*/

void Preferences (int x0, int y0) {
	vector<string> menustr = {
		"Change default middle texture  (Current: " + string(default_middle_texture) + ")",
		"Change default upper texture   (Current: " + string(default_upper_texture) + ")",
		"Change default lower texture   (Current: " + string(default_lower_texture) + ")",
		"Change default floor texture   (Current: " + string(default_floor_texture) + ")",
		"Change default ceiling texture (Current: " + string(default_ceiling_texture) + ")",
		"Change default floor height    (Current: " + to_string(default_floor_height) + ")",
		"Change default ceiling height  (Current: " + to_string(default_ceiling_height) + ")",
		"Change default light level     (Current: " + to_string(default_light_level) + ")",
	};

	int  val;
	string texname;
	string flatname;
	int  width, height;

	width  = 2 * BOX_BORDER + 2 * WIDE_HSPACING + 50 * FONTW;
	height = 2 * BOX_BORDER + 2 * WIDE_VSPACING + (int) (10.5 * FONTH);
	if (x0 < 0)
		x0 = (ScrMaxX + 1 - width) / 2;
	if (y0 < 0)
		y0 = (ScrMaxY + 1 - height) / 2;
	val = vDisplayMenu (x0, y0, "Preferences",
			menustr[0].c_str(), YK_, 0,
			menustr[1].c_str(), YK_, 0,
			menustr[2].c_str(), YK_, 0,
			menustr[3].c_str(), YK_, 0,
			menustr[4].c_str(), YK_, 0,
			menustr[5].c_str(), YK_, 0,
			menustr[6].c_str(), YK_, 0,
			menustr[7].c_str(), YK_, 0,
			NULL);

	int subwin_x0 = x0 + BOX_BORDER + WIDE_HSPACING;
	int subwin_y0 = y0 + BOX_BORDER + WIDE_VSPACING + (int) ((1.5 + val) * FONTH);
	switch (val) {
		case 1:
			texname = ChooseWallTexture (subwin_x0, subwin_y0, "Choose a wall texture",
					WTexture, default_middle_texture);
			if (texname != "")
				default_middle_texture = texname;
			break;
		case 2:
			texname = ChooseWallTexture (subwin_x0, subwin_y0, "Choose a wall texture",
					WTexture, default_upper_texture);
			if (texname != "")
				default_upper_texture = texname;
			break;
		case 3:
			texname = ChooseWallTexture (subwin_x0, subwin_y0, "Choose a wall texture",
					WTexture, default_lower_texture);
			if (texname != "")
				default_lower_texture = texname;
			break;
		case 4:
			{
				vector<string> flat_names;
				for (size_t n = 0; n < NumFTexture; n++) {
					flat_names.push_back(flat_list[n].name);
				}
				string flatname = ChooseFloorTexture(subwin_x0, subwin_y0,
						"Choose a floor texture", flat_names, default_floor_texture);
				if (flatname != "")
					default_floor_texture = flatname;
				break;
			}
		case 5:
			{
				vector<string> flat_names;
				for (size_t n = 0; n < NumFTexture; n++) {
					flat_names.push_back(flat_list[n].name);
				}
				string flatname = ChooseFloorTexture (subwin_x0, subwin_y0,
						"Choose a ceiling texture", flat_names, default_ceiling_texture);
				if (flatname != "")
					default_ceiling_texture = flatname;
				break;
			}
		case 6:
			val = InputIntegerValue (x0 + 42, subwin_y0, -512, 511, default_floor_height);
			if (val != IIV_CANCEL)
				default_floor_height = val;
			break;
		case 7:
			val = InputIntegerValue (x0 + 42, subwin_y0, -512, 511, default_ceiling_height);
			if (val != IIV_CANCEL)
				default_ceiling_height = val;
			break;
		case 8:
			val = InputIntegerValue (x0 + 42, subwin_y0, 0, 255, default_light_level);
			if (val != IIV_CANCEL)
				default_light_level = val;
			break;
	}
}
