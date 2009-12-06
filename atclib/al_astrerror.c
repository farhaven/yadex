/*
 *	astrer.c
 *	al_astrerror()
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


#include "atclib.h"


const char *al_astrerror (int e)
{
switch (e)
  {
  case 0            : return "No error";
  case AL_ABADL     : return "Invalid list";
  case AL_AEOL      : return "EOL";
  case AL_AINVAL    : return "Invalid argument";
  case AL_ANOFIX    : return "Not a fixed element length list";
  case AL_ANOMEM    : return "Not enough memory";
  case AL_ANOVAR    : return "Not a variable element length list";
  case AL_AOVERFLOW : return "Buffer overflow";
  default           : return "Unknown error";
  }
}
 

