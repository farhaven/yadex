/*
 *	llength.c
 *	al_llength()
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

#define AL_AILLEGAL_ACCESS
#include "atclib.h"


size_t al_llength (al_llist_t *l)
{
if (l == NULL || l->magic != AL_LLIST_MAGIC)
  {
  al_aerrno = AL_ABADL;
  return 0;
  }
if (l->length)
  return l->length;
if (l->ateol || l->current == NULL)
  {
  al_aerrno = AL_AEOL;
  return 0;
  }
return l->current->v.length;
}

