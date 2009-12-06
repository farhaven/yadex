/*
 *	lcreate.c
 *	al_lcreate()
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


al_llist_t *al_lcreate (size_t length)
{
al_llist_t *l;

l = malloc (sizeof (al_llist_t));
if (l == NULL)
  {
  al_aerrno = AL_ANOMEM;
  return NULL;
  }
l->magic   = AL_LLIST_MAGIC;
l->length  = length;
l->first   = NULL;
l->current = NULL;
l->curno   = 0;
l->ateol   = 0;
l->prev    = NULL;
l->total   = 0;
return l;
}

#ifdef OLD
The parameter flags is used to set a few options for the table:
- If (flags & AL_LSORT) is true:
  This flag keeps the list "sorted" so that calls to al_lread() return
  the elements in the right order (by default, you would get them in
  the same order that they were written).
  This is how it works: when you issue a write, the new element is
  inserted in the list just before the first element that is "greater"
  than it. Thus, the list stays sorted.
  Elements are sorted using the comparison function (*compare)(). The
  compare parameter is a pointer to a function that can compare two
  elements a and b and return a negative value if a is "lesser" than b,
  a positive value if a is "greater" than b and zero if a is "equal" to
  b. If you pass a NULL pointer, memcmp() is used.
  Don't use this option if you don't need it because;
  - Each writes does an additional N/2 call to (*compare)() and follows
    N/2 more links (N = number of elements in the list).
  - After each write, you have to rewind before you read because the
    order of elements keeps changing.
#endif
