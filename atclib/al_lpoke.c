/*
 *	lpoke.c
 *	al_lpoke()
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


int al_lpoke (al_llist_t *l, const void *buf)
{
al_lcheckmagic (l);
if (! l->length)
  {
  al_aerrno = AL_ANOFIX;
  return AL_ANOFIX;
  }

/* have to allocate a new element */
if (l->current == NULL || l->ateol)
  {
  al_lelt_t *cur;
  cur = malloc (sizeof (al_leltfix_t) - 1 + l->length);
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
  cur->next = NULL;
  }

/* do the transfer */
memcpy (l->current->f.data, buf, l->length);
return 0;
}


#ifdef SORTED
/*
If the list is sorted, the element is inserted before the first element
in the list that compares higher than the new element according to
(*l->compare)().
If it's not, the element is simply appended to the list.
A block large enough to hold l->eltsz bytes plus some overhead is
allocated and l->eltsz bytes are copied from buf into it.
*/
{
al_lelt_t *new;     /* new element allocated */

if ( l == NULL || l->magic != AL_ABADL_MAGIC )
  return AL_ABADL;

new = malloc ( sizeof ( al_lelt_t ) + l->eltsz - 1 );
if ( new == NULL )
  return AL_ANOMEM;
l->writen++;
memcpy ( new->data, buf, l->eltsz );

/* the list is sorted: search the list for the first element 'cur' that is
"greater" than new and insert new in the list just before 'cur' */
if ( l->flags & AL_LSORT )
  {
  al_lelt_t *prev, *cur;

  for ( prev = NULL, cur = l->first; cur != NULL; prev = cur, cur = cur->next )
    if ( (*l->compare) ( new->data, cur->data, l->eltsz ) < 0 )
      break;
  /* insert new in the list between prev and cur */
  if ( prev != NULL )
    prev->next = new;
  else
    l->first = new;
  new->next = cur;
  }

/* the list is not sorted: just add the element at the end */
else
  {
  if ( l->write != NULL )
    l->write->next = new;
  else
    l->first = new;
  new->next = NULL;
  l->write = new;
  }
return 0;
}
#endif
