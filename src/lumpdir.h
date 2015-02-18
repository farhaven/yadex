/*
 *	lumpdir.h
 *	Lump_dir class
 *	AYM 2000-04-08
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


#ifndef YH_LUMPDIR  /* DO NOT INSERT ANYTHING BEFORE THIS LINE */
#define YH_LUMPDIR

#include <map>
#include <string>
#include <vector>

using std::vector;
using std::string;

class Dependency;
class Serial_num;
class Wad_name;


/*
 *	Lump_dir 
 *	
 *	The purpose of this class is to hide the details of how
 *	lumps are stored in the wads from Yadex. It provides
 *	two basic services :
 *
 *	- loc_by_name()	Return the lump location (WadPtr,
 *			offset, length) of a lump by name. If
 *			lump does not exist, returns a NULL
 *			WadPtr.
 *
 *	- list()	Return a Lump_list object that provides
 *			what InputNameFromListWithFunc() needs
 *			to browse the patches. I suggest that
 *			this object be created immediately
 *			before it's needed and destroyed
 *			immediately after because it is not
 *			intended to remain valid across calls to
 *			refresh(). Ignoring this advice will
 *			cause some interesting crashes.
 */

// The key of the map is the lump name
struct Lump_map_key
{
  Lump_map_key (const char *name);
  wad_name_t _name;
};

// How to compare two keys
struct Lump_map_less
{
  //bool operator() (const Lump_map_key& p1, const Lump_map_key& p2) const;
  bool operator() (const Wad_name& p1, const Wad_name& p2) const;
};

typedef std::map<Wad_name, Lump_loc, Lump_map_less> Lump_map;

class Lump_list
{
  public :
    Lump_list ();
    ~Lump_list ();
    const char **data ();
    size_t size ();
    void set (Lump_map& lump_map);
    void clear ();

  private :
    char **array;
    size_t nelements;
};

class Lump_dir
{
  public :
    Lump_dir (MDirPtr *md, char l, Serial_num *sn);
    ~Lump_dir ();
    void loc_by_name (const char *name, Lump_loc& loc);
    vector<string> list();

  protected :
    void refresh ();

    Dependency *dependency;		// Resource on which we depend
    MDirPtr    *master_dir;
    char       label;			// First character of label
    Lump_map   lump_map;		// List of lumps, sorted by name (no duplicates), with their location.
    bool       have_prev;
    Lump_loc   loc_prev;
    wad_name_t name_prev;
};


#endif  /* DO NOT ADD ANYTHING AFTER THIS LINE */
