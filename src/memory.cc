/*
 *	memory.cc
 *	Memory allocation routines.
 *	BW & RQ sometime in 1993 or 1994.
 */


/*
This file is part of Yadex.

Yadex incorporates code from DEU 5.21 that was put in the public domain in
1994 by Raphaël Quinet and Brendon Wyber.

The rest of Yadex is Copyright © 1997-2003 André Majorel and others.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place, Suite 330, Boston, MA 02111-1307, USA.
*/


#include "yadex.h"

/*
   Note from RQ:
      To prevent memory fragmentation on large blocks (greater than 1K),
      the size of all blocks is rounded up to 8K.  Thus, "realloc" will
      move the block if and only if it has grown or shrunk enough to
      cross a 8K boundary.
      I don't do that for smaller blocks (smaller than 1K), because this
      would waste too much space if these blocks were rounded up to 8K.
      There are lots of "malloc"'s for very small strings (9 characters)
      or filenames, etc.
      Thanks to Craig Smith (bcs@cs.tamu.edu) for some of his ideas
      about memory fragmentation.
*/

#define SIZE_THRESHOLD	1024
#define SIZE_OF_BLOCK	4095  /* actually, this is (size - 1) */


/*
   allocate memory with error checking
*/

void *GetMemory (unsigned long size)
{
void *ret;

/* On 16-bit systems (BC 4.0), size_t is only 16-bit long so
   you can't malloc() more than 64 kB at a time. Catch it. */
if (size != (size_t) size)
   fatal_error ("GetMemory: %lu B is too much for this poor machine.", size);

/* limit fragmentation on large blocks */
if (size >= SIZE_THRESHOLD)
   size = (size + SIZE_OF_BLOCK) & ~SIZE_OF_BLOCK;
ret = malloc ((size_t) size);
if (!ret)
   {
   /* retry after having freed some memory, if possible */
   FreeSomeMemory ();
   ret = malloc ((size_t) size);
   }
if (!ret)
   fatal_error ("out of memory (cannot allocate %u bytes)", size);
return ret;
}


/*
   reallocate memory with error checking
*/

void *ResizeMemory (void *old, unsigned long size)
{
void *ret;

/* On 16-bit systems (BC 4.0), size_t is only 16-bit long so
   you can't malloc() more than 64 kB at a time. Catch it. */
if (size != (size_t) size)
   fatal_error ("ResizeMemory: %lu B is too much for this poor machine.", size);

/* limit fragmentation on large blocks */
if (size >= SIZE_THRESHOLD)
   size = (size + SIZE_OF_BLOCK) & ~SIZE_OF_BLOCK;
ret = realloc (old, (size_t) size);
if (!ret)
   {
   FreeSomeMemory ();
   ret = realloc (old, (size_t) size);
   }
if (!ret)
   fatal_error ("out of memory (cannot reallocate %lu bytes)", size);
return ret;
}


/*
   free memory
*/

void FreeMemory (void *ptr)
{
/* just a wrapper around free(), but provide an entry point */
/* for memory debugging routines... */
free (ptr);
}


/*
   allocate memory from the far heap with error checking
*/

void huge *GetFarMemory (unsigned long size)
{
void huge *ret;

/* limit fragmentation on large blocks */
if (size >= SIZE_THRESHOLD)
   size = (size +  SIZE_OF_BLOCK) & ~SIZE_OF_BLOCK;
ret = farmalloc (size);
if (!ret)
   {
   /* retry after having freed some memory, if possible */
   FreeSomeMemory ();
   ret = farmalloc (size);
   }
if (!ret)
   fatal_error ("out of memory (cannot allocate %lu far bytes)", size);
return ret;
}



/*
   reallocate memory from the far heap with error checking
*/

void huge *ResizeFarMemory (void huge *old, unsigned long size)
{
void huge *ret;

/* limit fragmentation on large blocks */
if (size >= SIZE_THRESHOLD)
   size = (size + SIZE_OF_BLOCK) & ~SIZE_OF_BLOCK;
ret = farrealloc (old, size);
if (!ret)
   {
   FreeSomeMemory ();
   ret = farrealloc (old, size);
   }
if (!ret)
   fatal_error ("out of memory (cannot reallocate %lu far bytes)", size);
return ret;
}



/*
   free memory from the far heap
*/

void FreeFarMemory (void huge *ptr)
{
/* just a wrapper around farfree(), but provide an entry point */
/* for memory debugging routines... */
farfree (ptr);
}


/* end of file */
