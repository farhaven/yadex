/*
 *	serialnum.cc
 *	Serial_num class
 *	AYM 2000-04-06
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
#include "serialnum.h"


Serial_num::Serial_num ()
{
  serial_no      = 0;
  serial_is_used = false;
}


/*
 *	stale - call this to check whether a serial number has become stale
 */
bool Serial_num::outdated (const serial_num_t& token)
{
  return token != serial_no;
}


/*
 *	update - get the revision number
 */
void Serial_num::update (serial_num_t& token)
{
  serial_is_used = true;
  token          = serial_no;
}


/*
 *	bump - call this to increment the revision number
 */
void Serial_num::bump ()
{
  /* If no one uses the current serial number, bumping it would
     be a waste of S/N space. */
  if (! serial_is_used)
    return;

  serial_no++;
  serial_is_used = false;
  if (serial_no == 0)  // Extremely unlikely to ever happen
  {
    nf_bug ("Serial_num(%p)::serial_no wrapped around", this);
  }
}


