/*
   Memory swapping by Raphaël Quinet <quinet@montefiore.ulg.ac.be>
   and Christian Johannes Schladetsch <s924706@yallara.cs.rmit.OZ.AU>

   You are allowed to use any parts of this code in another program, as
   long as you give credits to the authors in the documentation and in
   the program itself.  Read the file README.1ST for more information.

   This program comes with absolutely no warranty.

   SWAPMEM.C - When the memory is low....

   Note from RQ:
      Yuck!  I don't like this horrible thing.  The program should be
      able to swap almost anything to XMS or to disk, not only the
      five objects used here (Things, LineDefs, SideDefs, Vertices and
      Sectors).  That was a quick and dirty hack...  I didn't have the
      time to write a cleaner code...

   Note2 from RQ:
      After having tested these routines, I see that they are not very
      useful...  I'm still getting "out of memory" errors while editing
      E2M7 and other huge levels.  I should rewrite all this for GCC,
      use a flat memory model and a DOS extender, then just delete all
      this code...  I will have to do that anyway if I want to port it
      to other systems (Unix, Linux), so why not?
      Moral of the story: never waste long hours writing high-level
      memory swapping routines on a deficient OS.  Use a real OS with
      a better memory management instead.

   Note for CJS:
      It should be easy to include your XMS code in this file.  Just
      add the necessary lines in InitSwap(), SwapIn() and SwapOut().
      You won't need to edit any other file.  Put all your routines
      in XMS.C, with the necessary includes in XMS.H.  Please keep it
      short and simple... :-)
      ... And delete this note once you're done.
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
#include "levels.h"
#include "objid.h"
#ifdef SWAP_TO_XMS
#include "xms.h"
typedef XMSHandle SwapHandle;	/* XMS handle */
#define INVALID_HANDLE		-1
#else
typedef char SwapHandle[128];	/* name of the temporary disk file */
#define INVALID_HANDLE		"..."
#endif /* SWAP_TO_XMS */

/* global variables */
bool NeedThings = false;
bool NeedLineDefs = false;
bool NeedSideDefs = false;
bool NeedVertices = false;
bool NeedSectors = false;
SwapHandle ThingsH;
SwapHandle LineDefsH;
SwapHandle SideDefsH;
SwapHandle VerticesH;
SwapHandle SectorsH;


/*
   do the necessary initialisation for the secondary storage
*/

void InitSwap ()
{
#ifdef SWAP_TO_XMS
/* Init XMS */
...
#else
strcpy (ThingsH, INVALID_HANDLE);
strcpy (LineDefsH, INVALID_HANDLE);
strcpy (SideDefsH, INVALID_HANDLE);
strcpy (VerticesH, INVALID_HANDLE);
strcpy (SectorsH, INVALID_HANDLE);
#endif /* SWAP_TO_XMS */
}



/*
   moves an object from secondary storage to lower RAM
*/

void huge *SwapIn (SwapHandle handle, unsigned long size)
{
void huge *ptr;
#ifdef SWAP_TO_XMS
/* allocate a new memory block (in lower RAM) */
ptr = malloc(size);
/* read the data from XMS */
...
/* free the XMS memory block */
...
/* delete the handle */
...
#else
FILE      *file;
char huge *data;
SwapHandle oldhandle;

/* Note from RQ:
      the following test is there to prevent an infinite loop when
      SwapIn calls GetFarMemory, which calls FreeSomeMemory, which
      in turn calls SwapOut, then SwapIn...
 */
if (! strcmp (handle, INVALID_HANDLE))
   return NULL;
#ifdef DEBUG
LogMessage ("swapping in %lu bytes from %s\n", size, handle);
#endif /* DEBUG */
strcpy (oldhandle, handle);
/* invalidate the handle (must be before "GetFarMemory") */
strcpy (handle, INVALID_HANDLE);
/* allocate a new memory block (in lower RAM) */
ptr = malloc(size);
/* read the data from the temporary file */
file = fopen (oldhandle, "rb");
data = (char huge *) ptr;
if (file == NULL)
{
#ifdef DEBUG
   LogMessage ("\nFree memory before crash: %lu bytes.", farcoreleft ());
#endif /* DEBUG */
   fatal_error ("error opening temporary file \"%s\"", oldhandle);
}
while (size > 0x8000)
{
   if (fread (data, 1, 0x8000, file) != 0x8000)
      fatal_error ("error reading from temporary file \"%s\"", oldhandle);
   data = data + 0x8000;
   size -= 0x8000;
}
if (fread (data, 1, size, file) != size)
   fatal_error ("error reading from temporary file \"%s\"", oldhandle);
fclose (file);
/* delete the file */
unlink (oldhandle);
#endif /* !SWAP_TO_XMS */
return ptr;
}



/*
   moves an object from lower RAM to secondary storage
*/

void SwapOut (void huge *ptr, SwapHandle handle, unsigned long size)
{
#ifdef SWAP_TO_XMS
/* get a new XMS handle */
...
/* write the data to XMS */
...
#else
FILE      *file;
char huge *data;

// get a new (unique) file name
const char *basename = "yadexswpXXXXXX";  // 14 characters
const char *dir      = getenv ("TMPDIR");
if (dir == 0 || strlen (dir) + 1 + strlen (basename) >= sizeof (SwapHandle))
   dir = "/tmp";
y_snprintf (handle, sizeof (SwapHandle), "%s/%s", dir, basename);
if (mkstemp (handle) == -1)
{
#ifdef DEBUG
   LogMessage ("\nFree memory before crash: %lu bytes.", farcoreleft ());
#endif
   fatal_error ("cannot create a temporary file from \"%s\" (%s)",
      handle, strerror (errno));
}
#ifdef DEBUG
LogMessage ("swapping out %lu bytes to %s\n", size, handle);
#endif /* DEBUG */
// write the data to the temporary file
data = (char huge *) ptr;
file = fopen (handle, "wb");
if (file == NULL)
{
#ifdef DEBUG
   LogMessage ("\nFree memory before crash: %lu bytes.", farcoreleft ());
#endif /* DEBUG */
   fatal_error ("error creating temporary file \"%s\" (%s)",
      handle, strerror (errno));
}
while (size > 0x8000)
{
   if (fwrite (data, 1, 0x8000, file) != 0x8000)
      fatal_error ("error writing to temporary file \"%s\"", handle);
   data = data + 0x8000;
   size -= 0x8000;
}
if (fwrite (data, 1, size, file) != size)
   fatal_error ("error writing to temporary file \"%s\"", handle);
if (fclose (file))
   fatal_error ("error writing to temporary file \"%s\" (%s)",
      handle, strerror (errno));
#endif /* !SWAP_TO_XMS */
/* free the data block (in lower RAM) */
free(ptr);
}



/*
   get the objects needed (if they aren't already in memory)
*/

void SwapInObjects (void)
{
if (NeedThings && NumThings > 0 && Things == NULL)
   Things = (TPtr) SwapIn (ThingsH,
   			(unsigned long) NumThings * sizeof (struct Thing));
if (NeedLineDefs && NumLineDefs > 0 && LineDefs == NULL)
   LineDefs = (LDPtr) SwapIn (LineDefsH,
   			(unsigned long) NumLineDefs * sizeof (struct LineDef));
if (NeedSideDefs && NumSideDefs > 0 && SideDefs == NULL)
   SideDefs = (SDPtr) SwapIn (SideDefsH,
   			(unsigned long) NumSideDefs * sizeof (struct SideDef));
if (NeedVertices && NumVertices > 0 && Vertices == NULL)
   Vertices = (VPtr) SwapIn (VerticesH,
   			(unsigned long) NumVertices * sizeof (struct Vertex));
if (NeedSectors && NumSectors > 0 && Sectors == NULL)
   Sectors = (SPtr) SwapIn (SectorsH,
   			(unsigned long) NumSectors * sizeof (struct Sector));
}


/*
   mark the objects that should be in lower RAM
*/

void ObjectsNeeded (int objtype, ...)
{
va_list args;

/* get the list of objects */
NeedThings = false;
NeedLineDefs = false;
NeedSideDefs = false;
NeedVertices = false;
NeedSectors = false;
va_start (args, objtype);
while (objtype > 0)
{
   switch (objtype)
   {
   case OBJ_THINGS:
      NeedThings = true;
      break;
   case OBJ_LINEDEFS:
      NeedLineDefs = true;
      break;
   case OBJ_SIDEDEFS:
      NeedSideDefs = true;
      break;
   case OBJ_VERTICES:
      NeedVertices = true;
      break;
   case OBJ_SECTORS:
      NeedSectors = true;
      break;
   }
   objtype = va_arg (args, int);
}
va_end (args);
/* get the objects if they aren't already in memory */
SwapInObjects ();
}



/*
   free as much memory as possible by moving some objects out of lower RAM
*/

void FreeSomeMemory (void)
{
/* move everything to secondary storage */
if (NumSectors > 0 && Sectors != NULL)
{
   SwapOut (Sectors, SectorsH,
   	(unsigned long) NumSectors * sizeof (struct Sector));
   Sectors = NULL;
}
if (NumVertices > 0 && Vertices != NULL)
{
   SwapOut (Vertices, VerticesH,
   	(unsigned long) NumVertices * sizeof (struct Vertex));
   Vertices = NULL;
}
if (NumSideDefs > 0 && SideDefs != NULL)
{
   SwapOut (SideDefs, SideDefsH,
	(unsigned long) NumSideDefs * sizeof (struct SideDef));
   SideDefs = NULL;
}
if (NumLineDefs > 0 && LineDefs != NULL)
{
   SwapOut (LineDefs, LineDefsH,
   	(unsigned long) NumLineDefs * sizeof (struct LineDef));
   LineDefs = NULL;
}
if (NumThings > 0 && Things != NULL)
{
   SwapOut (Things, ThingsH,
   	(unsigned long) NumThings * sizeof (struct Thing));
   Things = NULL;
}
/* re-load the objects that are needed */
SwapInObjects ();
}


/* end of file */
