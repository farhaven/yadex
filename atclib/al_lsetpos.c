/*
 *	lsetpos.c
 *	al_lsetpos()
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


#define AL_AILLEGAL_ACCESS
#include "atclib.h"


int al_lsetpos (al_llist_t *l, const al_lpos_t *pos)
{
al_lcheckmagic (l);

/* trying to catch errors with a wide mesh net.
   one error that will go undetected is if elements have been deleted or
   inserted before prev. l->curno is then inconsistant with l->current */
if (pos->curno > l->total       /* elements were deleted */
                                /* current was deleted */
 || pos->current != NULL && pos->current->next == AL_AINVALIDPOINTER
                                /* prev was deleted or an element was
                                   inserted between prev and current
                                   or current was realloc'd */
 || pos->prev != NULL && pos->prev->next != NULL
                      && pos->prev->next != pos->current)
  {
  al_aerrno = AL_AEOL;
  return AL_AEOL;
  }

l->current = pos->current;
l->curno   = pos->curno;
l->ateol   = pos->ateol;
l->prev    = pos->prev;

/* if current was at EOL and an element was added, clear ateol. BTW,
   you see here why ateol should NOT be set when current==first==NULL */
if (l->ateol && l->current->next != NULL)
  {
  l->ateol = 0;
  l->current = l->current->next;
  }

return 0;
}

