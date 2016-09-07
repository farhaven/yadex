/*
 *    editlev.cc
 *    AYM 1998-09-06
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

#include <exception>
#include <sstream>
#include <string>
#include <utility>

#include "yadex.h"
#include <time.h>
#include <X11/Xlib.h>
#include "editlev.h"
#include "editloop.h"
#include "events.h"
#include "game.h"
#include "gfx.h"
#include "levels.h"
#include "patchdir.h"
#include "wadfile.h"

using std::stringstream;
using std::exception;
using std::stoi;
using std::string;
using std::to_string;
using std::pair;
using std::make_pair;

static void WriteYadexLog (const char *file, const char *level,
 time_t *t0, time_t *t1);

/*
 *    find_level
 *    Look in the master directory for levels that match
 *    the name in <name_given>.
 *
 *    <name_given> can have one of the following formats :
 *    
 *      [Ee]n[Mm]m      EnMm
 *      [Mm][Aa][Pp]nm  MAPnm
 *      n               MAP0n
 *      nm              Either EnMn or MAPnm
 *      ijk             EiMjk (Doom alpha 0.4 and 0.5)
 *
 *    Return:
 *    - If <name_given> is either [Ee]n[Mm]m or [Mm][Aa][Pp]nm,
 *      - if the level was found, its canonical (uppercased)
 *        name in a freshly malloc'd buffer,
 *      - else, NULL.
 *    - If <name_given> is either n or nm,
 *      - if either EnMn or MAPnm was found, the canonical name
 *        of the level found, in a freshly malloc'd buffer,
 *      - if none was found, <error_none>,
 *      - if the <name_given> is invalid, <error_invalid>,
 *      - if several were found, <error_non_unique>.
 */
pair<string, char*>
find_level (const string name_given) {
	try {
		/* Short level name */
		int n = stoi(name_given);
		if (n <= 99 || (n <= 999 && yg_level_name == YGLN_E1M10)) {
			stringstream s;
			s << "E";
			if (n > 99) {
				s << n / 100 << "M";
				s.width(2);
				s.fill('0');
				s << n % 100;
			} else {
				s << n / 10 << "M" << n % 10;
			}
			string name1 = s.str();
			s.str("");
			s << "MAP";
			s.width(2);
			s << n;
			string name2 = s.str();

			bool match1 = FindMasterDir(MasterDir, name1.c_str()) != NULL;
			bool match2 = FindMasterDir(MasterDir, name2.c_str()) != NULL;

			if (match1 and not match2) /* Found only ExMy */
				return make_pair(name1, nullptr);
			else if (match2 and not match1) /* Found only MAPxy */
				return make_pair(name2, nullptr);
			else if (match1 and match2) /* Found both */
				return make_pair("", error_non_unique);
			else /* Found none */
				return make_pair("", error_none);
		}
	} catch (exception &e) {
	}

	/* Complete name */
	if (FindMasterDir (MasterDir, name_given.c_str()))
		return make_pair(name_given, nullptr);
	else {
		if (levelname2levelno(name_given.c_str()))
			return make_pair(name_given, nullptr);
		else
			return make_pair("", error_invalid);
	}
}

/*
   the driving program
*/
void EditLevel (string levelname, bool newlevel)
{
	time_t t0, t1;
	string buf = "Yadex: " + (levelname != "" ? levelname : "(untitled)");

	ReadWTextureNames ();
	ReadFTextureNames ();
	patch_dir.refresh (MasterDir);

	if (not InitGfx())
		return;

	/* Call init_input_status() as shortly as possible after the creation
		of the window to minimize the risk of calling get_input_status(),
		get_key(), have_key(), etc. with <is> still uninitialized. */
	init_input_status ();
	init_event ();
	if (newlevel && levelname == "") { // "create"
		EmptyLevelData (levelname);
		MapMinX = -2000;
		MapMinY = -2000;
		MapMaxX = 2000;
		MapMaxY = 2000;
		Level = 0;
	} else if (newlevel && levelname != "")  { // "create <level_name>"
		printf ("Sorry, \"create <level_name>\" is not implemented."
				" Try \"create\" without argument.\n");
		TermGfx ();
		return;
	} else { // "edit <level_name>" or "edit"
		ClearScreen ();
		if (ReadLevelData (levelname)) {
			goto done;  // Failure!
		}
	}
	LogMessage(string(": Editing " + (levelname != "" ? levelname : "new level") + "...\n").c_str());

	// Set the name of the window
	XStoreName (dpy, win, buf.c_str());

	time (&t0);
	EditorLoop (levelname);
	time (&t1);
	LogMessage(string(": Finished editing " + (levelname != "" ? levelname : "new level") + "...\n").c_str());
	if (Level && Level->wadfile) {
		const string file_name = Level->wadfile ? Level->wadfile->pathname () : "(New level)";
		WriteYadexLog (file_name.c_str(), levelname.c_str(), &t0, &t1);
	}

done:
	TermGfx ();
	if (! Registered)
		printf ("Please register the game"
				" if you want to be able to save your changes.\n");

	ForgetLevelData ();
	/* forget the level pointer */
	Level = 0;
	ForgetWTextureNames ();
	ForgetFTextureNames ();
}


/*
 *    WriteYadexLog - Keep track of time spent editing that wad file
 *    FIXME should be in a separate module
 */
static void
WriteYadexLog (const char *file, const char *level, time_t *t0, time_t *t1) {
	al_fspec_t logname;
	al_fdrv_t  drive;
	al_fpath_t path;
	al_fbase_t base;

	al_fana (file, drive, path, base, 0);
	snprintf (logname, sizeof(logname), "%s%s%s.yl", drive, path, base);

	/* if log file does not already exist, do _not_ create it */
	if (al_fnature (logname) == 1) {
		FILE *logfd;
		logfd = fopen (logname, "a");
		if (logfd) {
			struct tm *tm = localtime (t0);
			fprintf (logfd, "%04d%02d%02d\tedit\t%s\t%ld\n",
			tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday, level, (long)(*t1-*t0)/60);
			fclose (logfd);
		}
	}
}
