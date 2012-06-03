/*
 *	endian.cc
 *	Determine the native endianness
 *	AYM 1999-03-30
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
#include "endian.h"


/*
 *	Returns 0 for little-endian, 1 for big-endian
 */
int native_endianness ()
{
verbmsg ("CPU endianness: ");
union
   {
   char mem[17];  // 5 is enough in theory
   uint32_t n;
   } u;
memset (u.mem, '\0', sizeof (u.mem));
u.n = 0x31323334;
if (! strcmp (u.mem, "1234"))
   {
   verbmsg ("big-endian\n");
   return 1;
   }
else if (! strcmp (u.mem, "4321"))
   {
   verbmsg ("little-endian\n");
   return 0;
   }
else
   {
   verbmsg ("unknown\n");
   warn ("weird endianness \"%s\". Report this to the maintainer!\n", u.mem);
   return 0;
   }
}


