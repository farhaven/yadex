/*
 *	editsave.cc
 *	Saving an editing session into a pwad
 *	AYM 1999-04-08
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

#include <algorithm>

#include "yadex.h"
#include "dialog.h"
#include "editsave.h"
#include "entry.h"
#include "game.h"
#include "levels.h"
#include "wadfile.h"
#include "wadlist.h"

extern int RedrawMap;

/*
 *	save_save_as
 *	Save the current level into a pwad.
 *	
 *	If neither <level_name> nor <file_name> are NULL and the
 *	level does not come from the iwad and <save_as> is not
 *	set, there is no interaction at all. The level is saved
 *	silently into <file_name> as <level_name>.
 *
 *	Else, the user is first prompted for a level name and a
 *	file name (in that order). The level name is given a
 *	default value of "E1M1" in Doom/Heretic mode or "MAP01"
 *	in Doom II/Hexen/Strife mode. The file name is given a
 *	default value of "LEVEL.wad" where LEVEL is the level
 *	name entered previously, lowercased.  Upon return from
 *	the function, <level_name> and <file_name> are set
 *	accordingly.
 *
 *	This function is called when the user hits [q], [F2] or
 *	[F3]. In the latter case, <save_as> is set to true.
 *
 *	Note: <level_name> being NULL means that the level name
 *	is not known, which is the case when using the "c"
 *	(create) command.  When using the "e" (edit) command,
 *	the level name is always known, since the level data
 *	comes from an iwad or pwad. The same goes for
 *	<file_name>.
 *
 *	Upon return from the function, <Level> is updated.
 *
 *	FIXME this should be a method of the Editwin class.
 */

/*
   get the name of the new wad file (returns NULL on Esc)
*/

string GetWadFileName (string levelname) {
	string outfile;

	/* get the file name */
	// If no name, find a default one
	if (levelname == "") {
		if (yg_level_name == YGLN_E1M1 || yg_level_name == YGLN_E1M10)
			levelname = "E1M1";
		else if (yg_level_name == YGLN_MAP01)
			levelname = "MAP01";
		else {
			nf_bug ("Bad ygd_level_name %d, using E1M1.", (int) yg_level_name);
			levelname = "E1M1";
		}
	}

	if (! Level
			|| !Level->wadfile
			|| Level->wadfile->filename == string(MainWad)) {
		outfile = levelname;
		std::transform(outfile.begin(), outfile.end(), outfile.begin(), ::tolower);
		outfile += ".wad";
	} else {
		outfile = string(Level->wadfile->filename);
	}

	do {
		outfile = InputFileName (-1, -1, "Name of the new wad file:", 80, outfile);
	} while (outfile == MainWad);

	/* escape */
	if (outfile == "") {
		return "";
	}

	/* if the wad file already exists, rename it to "*.bak" */
	Wad_file *wf;
	for (wad_list.rewind (); wad_list.get (wf);) {
		if (outfile == string(wf->filename)) {
			wf->filename += ".bak";

			verbmsg ("setting wf->filename to %s\n", wf->filename.c_str());  // DEBUG
			/* Need to close, then reopen: problems with SHARE.EXE */
			verbmsg ("closing %p\n", wf->fp);				// DEBUG
			fclose (wf->fp);
			verbmsg ("renaming %s -> %s\n", outfile.c_str(), wf->filename.c_str());	// DEBUG

			if (rename (outfile.c_str(), wf->filename.c_str()) != 0) {
				verbmsg ("removing %s\n", wf->filename.c_str());  // DEBUG
				if (remove (wf->filename.c_str()) != 0 && errno != ENOENT) {
					char buf1[81];
					char buf2[81];
					snprintf (buf1, sizeof buf1, "Could not delete \"%.64s\"", wf->filename.c_str());
					snprintf (buf2, sizeof buf2, "(%.64s)", strerror (errno));
					Notify (-1, -1, buf1, buf2);
					return 0;
				}
				verbmsg ("renaming %s -> %s\n", outfile.c_str(), wf->filename.c_str());  // DEBUG
				if (rename (outfile.c_str(), wf->filename.c_str())) {
					char buf1[81];
					char buf2[81];
					snprintf (buf1, sizeof buf1, "Could not rename \"%.64s\"", outfile.c_str());
					snprintf (buf2, sizeof buf2, "as \"%.64s\" (%.64s)", wf->filename.c_str(), strerror (errno));
					Notify (-1, -1, buf1, buf2);
					return 0;
				}
			}
			verbmsg ("opening %s\n", wf->filename.c_str()); // DEBUG
			wf->fp = fopen (wf->filename.c_str(), "rb");
			if (wf->fp == 0) {
				char buf1[81];
				char buf2[81];
				snprintf (buf1, sizeof buf1, "Could not reopen \"%.64s\"", wf->filename.c_str());
				snprintf (buf2, sizeof buf2, "(%.64s)", strerror (errno));
				Notify (-1, -1, buf1, buf2);
				return 0;
			}
			verbmsg ("wf->filename: %s\n", wf->filename.c_str());	// DEBUG
			verbmsg ("wf->fp        %p\n", wf->fp);		// DEBUG
			verbmsg ("outfile       %s\n", outfile.c_str());		// DEBUG
			break;
		}
	}

	return outfile;
}
