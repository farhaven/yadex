/*
 *	lseek.c
 *	al_lseek()
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


#include <stdio.h>

#define AL_AILLEGAL_ACCESS
#include "atclib.h"


int al_lseek (al_llist_t *l, long offset, int origin)
{
long from, to;

al_lcheckmagic (l);
switch (origin)
  {
  case SEEK_SET : from = 0; break;
  case SEEK_CUR : from = l->curno; break;
  case SEEK_END : from = l->total; break;
  default       : al_aerrno = AL_AINVAL; return AL_AINVAL; break;
  }
to = from + offset;

if (to < 0)
  {
  al_aerrno = AL_AINVAL;
  return AL_AINVAL;
  }
if (to < l->curno)
  {
  al_lrewind (l);   /* Can't fail so don't check */
  from = 0;
  }
if (l->first == NULL && to > 0)  /* Was "from > 0" */
  {
  al_aerrno = AL_AEOL;
  return AL_AEOL;
  }
/* Sorry for this, but the only alternative is having a backlink pointer
   for each element and wasting 4 B per element. */
while (l->curno < to)
  {
  if (l->ateol)
    {
    al_aerrno = AL_AEOL;
    return AL_AEOL;
    } 
  if (l->current->next == NULL)
    l->ateol = 1;
  else
    {
    l->prev = l->current;
    l->current = l->current->next;
    }
  l->curno++;
  }
return 0;
}

