/*
 *	notexist - complain if given directory entries exist
 *	AYM 2000-09-06
 */


/*
This file is Copyright © 2000 André Majorel.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; if not, write to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307, USA.
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


int main (int argc, char *argv[])
{
  int n;

  for (n = 1; n < argc; n++)
  {
    struct stat s;

    if (lstat (argv[n], &s) == 0)
    {
      fprintf (stderr, "%s already exists. Delete it.\n", argv[n]);
      exit (1);
    }
    else if (errno != ENOENT)
    {
      fprintf (stderr, "%s: can't lstat (%s)\n", argv[n], strerror (errno));
      exit (1);
    }
    if (stat (argv[n], &s) == 0)
    {
      fprintf (stderr, "%s already exists. Delete it.\n", argv[n]);
      exit (1);
    }
    else if (errno != ENOENT)
    {
      fprintf (stderr, "%s: can't stat (%s)\n", argv[n], strerror (errno));
      exit (1);
    }
  }
  exit (0);
}


