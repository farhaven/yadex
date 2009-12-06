/*
 *	linsert.c
 *	al_linsert()
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


#include <stdlib.h>
#include <memory.h>

#define AL_AILLEGAL_ACCESS
#include "atclib.h"


int al_linsert (al_llist_t *l, const void *buf)
{
al_lelt_t *new_element;

al_lcheckmagic (l);
if (l->current == NULL || l->ateol)
  return al_lpoke (l, buf);
new_element = malloc (sizeof (al_lelt_t) - 1 + l->length);
if (new_element == NULL)
  {
  al_aerrno = AL_ANOMEM;
  return AL_ANOMEM;
  }
l->total++;
memcpy (new_element->f.data, buf, l->length);

/* update forward link to new element */
if (l->current == l->first)
  l->first = new_element;
else
  l->prev->next = new_element;
/* update forward link from new element */
new_element->next = l->current;
/* update list pointer */
l->current = new_element;
return 0;
}
  
