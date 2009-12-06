/*
 *	help1.cc
 *	AYM 1998-10-03
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
#include "cfgfile.h"
#include "help1.h"


/*
 *	what
 *	Return a static string containing
 *	the name and version number of Yadex.
 */
const char *what ()
{
static char buf[40];
y_snprintf (buf, sizeof buf, "Yadex %s (%s)", yadex_version, yadex_source_date);
return buf;
}


/*
 *	print_usage
 *	Print the program usage.
 */
void print_usage (FILE *fd)
{
fprintf (fd, "%s\n", what ());
fprintf (fd, "Usage: yadex [options] [<pwad_file> ...]\n");
fprintf (fd, "Options:\n");
dump_command_line_options (fd);
fprintf (fd, " %-33sSame as -?\n", "--help");
fprintf (fd, " %-33sPrint version and exit\n", "--version");
fprintf (fd, "Put a \"+\" instead of a \"-\" before boolean options"
             " to reverse their effect.\n");
}


/*
 *	print_welcome
 *	Print the welcome message
 */
void print_welcome (FILE *fd)
{
#ifdef OLD_MESSAGE
fprintf (fd, "\n");
fprintf (fd, "*----------------------------------------------------------------------------*\n");
fprintf (fd, "| Welcome to DEU!  This is a poweful utility and, like all good tools, it    |\n");
fprintf (fd, "| comes with its user's manual.  Please print and read DEU.TXT if you want   |\n");
fprintf (fd, "| to discover all the features of this program.  If you are new to DEU, the  |\n");
fprintf (fd, "| tutorial will show you how to build your first level.                      |\n");
fprintf (fd, "|                                                                            |\n");
fprintf (fd, "| If you are an experienced DEU user and want to know what has changed since |\n");
fprintf (fd, "| the last version, you should read the revision history in README.1ST.      |\n");
fprintf (fd, "|                                                                            |\n");
fprintf (fd, "| And if you have lots of suggestions for improvements, bug reports, or even |\n");
fprintf (fd, "| complaints about this program, be sure to read README.1ST first.           |\n");
fprintf (fd, "| Hint: you can easily disable this message.  Read the docs carefully...     |\n");
fprintf (fd, "*----------------------------------------------------------------------------*\n");
#else

fprintf (fd, "\n"
                "** Welcome to Yadex. Glad you've made it so far. :-)\n");
#if defined Y_ALPHA
fprintf (fd, "**\n"
		"** This is an alpha version. Expect it to have bugs. Do\n"
		"** yourself a favour and make backup copies of your data !\n"
		"**\n");
#elif defined Y_BETA
fprintf (fd, "**\n"
		"** This is a beta version. It is believed to be reasonably\n"
		"** stable but it's been given only limited testing. So do\n"
		"** yourself a favour and make backup copies of your data.\n"
		"**\n");
#else
fprintf (fd, "**\n"
		"** This version is believed to be stable but you never\n"
		"** know so make backup copies of your data anyway.\n"
		"**\n");
#endif
fprintf (fd, "** Yadex is work in progress. Subscribe to yadex-announce\n");
fprintf (fd, "** or keep an eye on the web page.\n");
fprintf (fd, "** To edit an existing level, type \"e <level_name>\".\n");
fprintf (fd, "** To create a new level, type \"c\".\n"
                "\n");
#endif
}



