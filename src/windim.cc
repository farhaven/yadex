/*
 *	windim.cc
 *	Win_dim class - store the width or height of a window
 *	AYM 2000-05-29
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
#include <ctype.h>
#include "windim.h"


// Holds the private data members.
struct Win_dim_priv
{
  int value;
  bool relative;
};


Win_dim::Win_dim ()
{
  p           = new Win_dim_priv;
  p->value    = 0;
  p->relative = false;
}


Win_dim::Win_dim (const char *string)
{
  p           = new Win_dim_priv;
  p->value    = 0;
  p->relative = false;
  set (string);
}


Win_dim::~Win_dim ()
{
  delete p;
  p = 0;
}


// String -> Win_dim
int Win_dim::set (const char *string)
{
  int  value    = 0;
  bool relative = false;

  if (! isdigit (*string))
    return 1;  // Error
  while (isdigit (*string))
  {
    value = 10 * value + dectoi (*string);
    string++;
  }
  if (*string == '%')
  {
    relative = true;
    string++;
  }
  if (*string != '\0')
    return 1;  // Error
  p->value    = value;
  p->relative = relative;
  return 0;
}


// Get numeric value
int Win_dim::pixels (int ref_pixels)
{
  if (p->relative)
    return ref_pixels * p->value / 100;
  else
    return p->value;
}


// Win_dim -> string
void Win_dim::string (char *buf, size_t buf_size)
{
  if (p->relative)
    y_snprintf (buf, buf_size, "%d%%", p->value);
  else
    y_snprintf (buf, buf_size, "%d", p->value);
}

