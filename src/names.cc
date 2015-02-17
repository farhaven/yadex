/*
 *	names.cc
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

#include <string>

#include "names.h"
#include "yadex.h"
#include "game.h"
#include "objid.h"

using std::string;
using std::to_string;

/*
   get the name of an object type
*/
string GetObjectTypeName (int objtype)
{
    switch (objtype)
    {
        case OBJ_THINGS:   return "thing";
        case OBJ_LINEDEFS: return "linedef";
        case OBJ_SIDEDEFS: return "sidedef";
        case OBJ_VERTICES: return "vertex";
        case OBJ_SEGS:     return "segment";
        case OBJ_SSECTORS: return "ssector";
        case OBJ_NODES:    return "node";
        case OBJ_SECTORS:  return "sector";
        case OBJ_REJECT:   return "reject";
        case OBJ_BLOCKMAP: return "blockmap";
    }
    return "< Bug! >";
}

/*
   what are we editing?
*/
const char *GetEditModeName (int objtype)
{
    switch (objtype)
    {
        case OBJ_THINGS:   return "Things";
        case OBJ_LINEDEFS:
        case OBJ_SIDEDEFS: return "LD & SD";
        case OBJ_VERTICES: return "Vertices";
        case OBJ_SEGS:     return "Segments";
        case OBJ_SSECTORS: return "Seg-Sectors";
        case OBJ_NODES:    return "Nodes";
        case OBJ_SECTORS:  return "Sectors";
    }
    return "< Bug! >";
}

/*
   get a short (16 char.) description of the type of a linedef
*/
const char *GetLineDefTypeName (int type)
{
    if (CUR_LDTDEF != NULL && CUR_LDTDEF->number == type)
        return CUR_LDTDEF->shortdesc;
    for (al_lrewind (ldtdef); ! al_leol (ldtdef); al_lstep (ldtdef))
        if (CUR_LDTDEF->number == type)
            return CUR_LDTDEF->shortdesc;
    return "UNKNOWN";
}

/*
   get a short description of the flags of a linedef
*/
const char *GetLineDefFlagsName (int flags)
{
    static char buf[20];
    // "P" is a Boom extension ("pass through")
    // "T" is for Strife ("translucent")
    const char *flag_chars_ = "-BABMPPANBSLU2MI";

    int n;

    char *p = buf;
    for (n = 0; n < 16; n++)
    {
        if (n != 0 && n % 4 == 0)
            *p++ = ' ';
        if (flags & (0x8000u >> n))
            *p++ = flag_chars_[n];
        else
            *p++ = '-';
    }
    *p = '\0';
    return buf;
}

/*
   get a long description of one linedef flag
*/
const char *GetLineDefFlagsLongName (int flags)
{
    if (yg_level_format == YGLF_HEXEN)
    {
        if (flags & 0x200) return "Can be activated more than once";
        if (flags & 0x400) return "Activated by player";
        if (flags & 0x800) return "Activated by monster";
        if (flags & 0xC00) return "Activated by gunfire";
        if (flags & 0x1000) return "Activated by bumping player";
        if (flags & 0x1400) return "Activated by crossing projectile";
        if (flags & 0x1800) return "Activated by pass-through";
        if (flags & 0x2000) return "Activated by players and monsters";
        if (flags & 0x4000) return "Blocks all";
        if (flags & 0x8000) return "UNKNOWN";
    }else
    {
        if (flags & 0x1000) return "Translucent [Strife]";
        if (flags & 0x200) return "Pass-through [Boom]";
    }
    if (flags & 0x100)  return "Always shown on the map";
    if (flags & 0x80)   return "Never shown on the map";
    if (flags & 0x40)   return "Blocks sound";
    if (flags & 0x20)   return "Secret (shown as normal on the map)";
    if (flags & 0x10)   return "Lower texture is \"unpegged\"";
    if (flags & 0x08)   return "Upper texture is \"unpegged\"";
    if (flags & 0x04)   return "Two-sided (may be transparent)";
    if (flags & 0x02)   return "Monsters cannot cross this line";
    if (flags & 0x01)   return "Impassible";
    return "UNKNOWN";
}

/*
   get a short (14 char.) description of the type of a sector
*/
const char *GetSectorTypeName (int type)
{
    /* KLUDGE: To avoid the last element which is bogus */
    if (al_ltell (stdef) == al_lcount (stdef) - 1)
        al_lrewind (stdef);

    if (CUR_STDEF != NULL && CUR_STDEF->number == type)
        return CUR_STDEF->shortdesc;
    for (al_lrewind (stdef); ! al_leol (stdef); al_lstep (stdef))
        if (CUR_STDEF->number == type)
            return CUR_STDEF->shortdesc;
    static char buf[30];
    sprintf (buf, "UNKNOWN (%d)", type);
    return buf;
}

/*
   get a long description of the type of a sector
*/
string GetSectorTypeLongName(int type)
{
    /* KLUDGE: To avoid the last element which is bogus */
    if (al_ltell (stdef) == al_lcount (stdef) - 1)
        al_lrewind (stdef);

    if (CUR_STDEF != NULL && CUR_STDEF->number == type)
        return string(CUR_STDEF->longdesc);
    for (al_lrewind (stdef); ! al_leol (stdef); al_lstep (stdef))
        if (CUR_STDEF->number == type)
            return string(CUR_STDEF->longdesc);
	 return "UNKNOWN (" + to_string(type) + ")";
}

const char *GetLineDefArgumentName (int type,int argument)
{
    if (CUR_LDTDEF != NULL && CUR_LDTDEF->number == type)
    {
        if (argument == 1 && CUR_LDTDEF->argument1 != NULL)
            return CUR_LDTDEF->argument1;
        if (argument == 2 && CUR_LDTDEF->argument2 != NULL)
            return CUR_LDTDEF->argument2;
        if (argument == 3 && CUR_LDTDEF->argument3 != NULL)
            return CUR_LDTDEF->argument3;
        if (argument == 4 && CUR_LDTDEF->argument4 != NULL)
            return CUR_LDTDEF->argument4;
        if (argument == 5 && CUR_LDTDEF->argument5 != NULL)
            return CUR_LDTDEF->argument5;
        return "UNKNOWN";
    }
    for (al_lrewind (ldtdef); ! al_leol (ldtdef); al_lstep (ldtdef))
        if (CUR_LDTDEF->number == type)
        {
            if (argument == 1 && CUR_LDTDEF->argument1 != NULL)
                return CUR_LDTDEF->argument1;
            if (argument == 2 && CUR_LDTDEF->argument2 != NULL)
                return CUR_LDTDEF->argument2;
            if (argument == 3 && CUR_LDTDEF->argument3 != NULL)
                return CUR_LDTDEF->argument3;
            if (argument == 4 && CUR_LDTDEF->argument4 != NULL)
                return CUR_LDTDEF->argument4;
            if (argument == 5 && CUR_LDTDEF->argument5 != NULL)
                return CUR_LDTDEF->argument5;
        }  
    return "UNKNOWN";
}
