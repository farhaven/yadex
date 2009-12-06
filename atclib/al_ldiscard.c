/*
 *	ldiscard.c
 *	al_ldiscard()
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


int al_ldiscard ( al_llist_t *l )
{
al_lelt_t *cur, *next;

al_lcheckmagic (l);
for (cur = l->first; cur != NULL; cur = next)
  {
  next = cur->next;
  /* zeroing l->magic protects us anyway but deux precautions
     valent mieux qu'une. */
  cur->next = AL_AINVALIDPOINTER;
  free ( cur );
  }
l->magic = 0;  /* so that stale pointers can't fool us later */
free (l);
return 0;
}

