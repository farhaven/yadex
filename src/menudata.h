/*
 *	menudata.h - Menu_data abstract base class
 *	AYM 2002-05-09
 */


/*
This file is part of Yadex.

Yadex incorporates code from DEU 5.21 that was put in the public domain in
1994 by Raphaël Quinet and Brendon Wyber.

The rest of Yadex is Copyright © 1997-2002 André Majorel and others.

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


#ifndef YH_MENUDATA  /* DO NOT INSERT ANYTHING BEFORE THIS LINE */
#define YH_MENUDATA


class Menu_data
{
  public :
    virtual size_t nitems () const = 0;
    virtual const char *operator[] (size_t n) const = 0;
};


#endif  /* DO NOT ADD ANYTHING AFTER THIS LINE */
