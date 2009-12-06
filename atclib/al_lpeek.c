/*
 *	lpeek.c
 *	al_lpeek()
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
#include <stdlib.h>
#include <memory.h>

#define AL_AILLEGAL_ACCESS
#include "atclib.h"


int al_lpeek (al_llist_t *l, void *buf)
{
al_lcheckmagic (l);
if (! l->length)
  {
  al_aerrno = AL_ANOFIX;
  return AL_ANOFIX;
  }

if (l->current == NULL || l->ateol)
  {
  al_aerrno = AL_AEOL;
  return AL_AEOL;
  }
memcpy (buf, l->current->f.data, l->length);
return 0;
}


