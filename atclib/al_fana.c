/*
 *	al_fana.c
 *	AYM whenever
 */


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


#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "atclib.h"


void al_fana (const char *ispec,
                    char *odrv,
                    char *opath,
                    char *obase,
                    char *oext)
{
register int offset;

/* Extract the drivespec */
#if AL_AOS == 'd'
if (isalpha (*ispec) && ispec[1] == ':')
  {
  if (odrv != NULL)
    {
    *odrv++ = *ispec;
    *odrv++ = ispec[1];
    }
  ispec += 2;
  }
#endif
if (odrv != NULL)
  *odrv = '\0';

/* Extract the path */
if (opath != NULL)
  *opath = '\0';
{  /* put in offset the offset of the last path separator */
size_t n;
for (n = 0, offset = -1; ispec[n]; n++)
  if (al_fisps (ispec[n]))
    offset = n;
}
if (offset >= 0)
  {
  offset += 1;
  if (opath != NULL)
    strncat (opath, ispec, al_amin (offset, AL_FPATH));
  ispec += offset;
  }

/* Extract the basename */
if (obase != NULL)
  *obase = '\0';
offset = al_strOLC (ispec, '.');
if (offset == -1)
  offset = strlen (ispec);
if (offset > 0 && obase != NULL)
  strncat (obase, ispec, al_amin (offset, AL_FBASE));
ispec += offset;

/* Extract the extension */
if (oext != NULL)
  {
  *oext = '\0';
  strncat (oext, ispec, AL_FEXT);
  }
}

/*############################ EOF - alfana.c #############################*/
