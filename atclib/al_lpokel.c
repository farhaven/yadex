/*
 *	lpokel.c
 *	al_lpokel()
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


int al_lpokel (al_llist_t *l, const void *buf, size_t length)
{
al_lcheckmagic (l);
if (l->length)
  {
  al_aerrno = AL_ANOVAR;
  return AL_ANOVAR;
  }

/* have to allocate a new element */
if (l->current == NULL || l->ateol)
  {
  al_lelt_t *cur;
  cur = malloc (sizeof (al_leltvar_t) - 1 + length);
  if (cur == NULL) 
    {
    al_aerrno = AL_ANOMEM;
    return AL_ANOMEM;
    }
  l->total++;
  if (l->current == NULL)
    l->first = l->current = cur;
  else  /* that is "if (l->ateol)" */
    {
    l->current->next = cur;
    l->current = cur;
    l->ateol = 0;
    }
  cur->v.length = length;
  cur->next = NULL;
  }
/* resize an already existing element */
else if (l->current->v.length != length)
  {
  al_lelt_t *new_element;
  new_element = realloc (l->current, sizeof (al_leltvar_t) - 1 + length);
  if (new_element == NULL)
    {
    al_aerrno = AL_ANOMEM;
    return AL_ANOMEM;
    }
  new_element->v.length = length;
  if (l->current == l->first)
    l->first = new_element;
  else
    l->prev->next = new_element;
  l->current = new_element;
  }

/* do the transfer */
memcpy (l->current->v.data, buf, length);
return 0;
}


