/* alfnatur.c */
/* al_fnature() */
/* AYM 19960917 */


/*
This file is part of Atclib.

Atclib is Copyright © 1995-1999 André Majorel.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; if not, write to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307, USA.
*/


#include <sys/types.h>
#include <sys/stat.h>

#include "atclib.h"


/*
!&! 3al al_fnature Returns the nature of an entry
SYNOPSIS
int al_fnature (const char *spec)

DESCRIPTION
Performs a stat() on the entry named spec and returns the information
relevant to the nature of the entry.

RETURN VALUE
0  The entry or path does not exist
1  The entry exists and is a regular file
2  The entry exists and is a directory
3  The entry exists and is a device
4  The entry exists and is something else (under MS-DOS, a volume label?)

BUGS
The stat() function of MSC 6.0 does not set S_IFCHR for devices but S_IFREG
instead. For volume labels, it says that the entry does not exist.
Therefore, al_fnature() returns 1 for a device and 0 for a volume label.
!&!
*/

int al_fnature (const char *spec)
{
struct stat statbuf;
int r;

r = stat (spec, &statbuf);
if (r)
  return 0;
if (statbuf.st_mode & S_IFCHR)
  return 3;
if (statbuf.st_mode & S_IFDIR)
  return 2;
if (statbuf.st_mode & S_IFREG)
  return 1;
return 4;
}

