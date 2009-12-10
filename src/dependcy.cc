/*
 *	dependcy.cc
 *	Dependency class
 *	AYM 2000-04-09
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

#include "yadex.h"  /* Just to force a recompile when you ./configure again */
#include "dependcy.h"
#include "serialnum.h"

Dependency::Dependency (Serial_num *sn)
{
    serial_num  = sn;
    token_valid = false;
}

bool Dependency::outdated ()
{
    if (! token_valid)
        return true;
    return serial_num->outdated (token);
}

void Dependency::update ()
{
    serial_num->update (token);
    token_valid = true;
}
