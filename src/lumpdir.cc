/*
 *	lumpdir.cc
 *	Lump_dir class
 *	AYM 1999-11-25
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
#include "dependcy.h"
#include "lumpdir.h"
#include "wadname.h"
#include "wadnamec.h"
#include "wadfile.h"



/*
 *	Lump_dir::Lump_dir - ctor 
 *
 *	<md> is a pointer to the master directory on which the
 *	lump directory is based.
 *	<l> is the first character of the label.
 *	<sn> is a pointer to the Serial_num of the master
 *	directory.
 */

Lump_dir::Lump_dir (MDirPtr *md, char l, Serial_num *sn)
{
  have_prev  = false;
  dependency = new Dependency (sn);
  master_dir = md;
  label      = l;
}


/*
 *	Lump_dir::~Lump_dir - dtor
 */
Lump_dir::~Lump_dir ()
{
  if (dependency)
    delete dependency;
  if (! lump_map.empty ())
    lump_map.clear ();
}


/*
 *	Lump_dir::loc_by_name - find a lump by name
 *
 *	Return the (wad, offset, length) location of the lump
 *	named <name>. If not found, set loc.wad to 0.
 */
void Lump_dir::loc_by_name (const char *name, Lump_loc& loc)
{
  if (dependency->outdated ())
    refresh ();

  /* Caller asked for same lump twice in a row. Save us a second
     search. */
  if (have_prev && ! y_strnicmp (name, name_prev, WAD_NAME))
  {
    loc = loc_prev;
    return;
  }

  Lump_map::const_iterator i = lump_map.find (name);
  have_prev = true;
  if (i == lump_map.end ())
    loc.wad = loc_prev.wad = 0;
  else
    loc = loc_prev = i->second;
}


/*
 *	Lump_dir::list - return an array of lump names
 *
 *	Put a list of all lumps in the list, sorted by name and
 *	with duplicates removed, in <l>.
 */
void Lump_dir::list (Lump_list& l)
{
  if (dependency->outdated ())
    refresh ();
  l.set (lump_map);
}


/*
 *	Lump_dir::refresh - update the lump dir. wrt to the master dir.
 *
 *	This is called automatically if the master directory has
 *	changed since the last refresh.
 */
void Lump_dir::refresh ()
{
  /* refresh() can be called more than once one the same object.
     And usually is ! */
  have_prev = false;
  if (! lump_map.empty ())
    lump_map.clear ();

  /* Get list of lumps in the master directory. Everything
     that is between X_START/X_END or XX_START/XX_END and that
     is not a label is supposed to be added to the Lump_dir. */
  Wad_name_c x_start  ("%c_START",   label);
  Wad_name_c x_end    ("%c_END",     label);
  Wad_name_c xx_start ("%c%c_START", label, label);
  Wad_name_c xx_end   ("%c%c_END",   label, label);
  for (MDirPtr dir = *master_dir;
      dir && (dir = FindMasterDir (dir, x_start.name, xx_start.name));)
  {
    MDirPtr start_label = dir;
    const char *end_label = 0;
    if (! x_start.cmp (dir->dir.name))
      end_label = x_end.name;
    else if (! xx_start.cmp (dir->dir.name))
      end_label = xx_end.name;
    else
      fatal_error ("Bad start label \"%.*s\"", (int) WAD_NAME, dir->dir.name);
    dir = dir->next;
    for (;; dir = dir->next)
    {
      if (! dir)
      {
	warn ("%.128s: no matching %s for %.*s\n",
	    start_label->wadfile->pathname (),
	    end_label,
	    (int) WAD_NAME, start_label->dir.name);
	break;
      }
      // Ended by X_END or, if started by XX_START, XX_END.
		if (! x_end.cmp (dir->dir.name)
				|| (end_label == xx_end.name && ! xx_end.cmp (dir->dir.name))) {
	if (dir->dir.size != 0)
	  warn ("%.128s: label %.*s has non-zero size %ld\n",
	      dir->wadfile->pathname (),
	      (int) WAD_NAME, dir->dir.name,
	      (long) dir->dir.size);
	dir = dir->next;
	break;
      } 
      // Ignore inner labels (X[123]_START, X[123]_END)
      if (dir->dir.start == 0 || dir->dir.size == 0)
      {
	if (! (toupper (dir->dir.name[0]) == label
	    && (dir->dir.name[1] == '1'
	     || dir->dir.name[1] == '2'
	     || dir->dir.name[1] == '3')
	    && dir->dir.name[2] == '_'
	    && (! y_strnicmp (dir->dir.name + 3, "START", WAD_NAME - 3)
		|| ! y_strnicmp (dir->dir.name + 3, "END", WAD_NAME - 3))))
	  warn ("%.128s: unexpected label \"%.*s\" in %s group\n",
	      dir->wadfile->pathname (),
	      (int) WAD_NAME, dir->dir.name,
	      start_label->dir.name);
	continue;
      }
      wad_flat_name_t name;
      memcpy (name, dir->dir.name, sizeof name);
      lump_map[name]
	= Lump_loc (dir->wadfile, dir->dir.start, dir->dir.size);
    }
    if (dir)
      dir = dir->next;
  }
#ifdef DEBUG
  for (Lump_lumps_map::const_iterator i = lump_map.begin ();
      i != lump_map.end (); i++)
  {
    printf ("%-8.8s %p %08lX %ld\n",
      i->first._name,
      i->second.wad,
      i->second.ofs,
      i->second.len);
  }
#endif
  dependency->update ();
}


/*-------------------------- Lump_list --------------------------*/


Lump_list::Lump_list ()
{
  array = 0;
  nelements = 0;
}


Lump_list::~Lump_list ()
{
  clear ();
}


void Lump_list::set (Lump_map& lump_map)
{
  clear ();
  nelements = lump_map.size ();
  array = new char *[nelements];

  Lump_map::const_iterator i = lump_map.begin ();
  for (size_t n = 0; n < nelements; n++)
  {
    array[n] = new char[WAD_NAME + 1];
    *array[n] = '\0';
    strncat (array[n], i++->first.name, WAD_NAME);
  }
}


void Lump_list::clear ()
{
  if (array != 0)
  {
    for (size_t n = 0; n < nelements; n++)
      delete[] array[n];
    delete[] array;
  }
}


const char **Lump_list::data ()
{
  return (const char **) array;
}


size_t Lump_list::size ()
{
  return nelements;
}


/*------------------------- Lump_map_key -------------------------*/


//Lump_map_key::Lump_map_key (const char *name)
//{
//  memcpy (_name, name, sizeof _name);
//}


/*------------------------ Lump_map_less -------------------------*/


bool Lump_map_less::operator ()
  (const Wad_name& name1, const Wad_name& name2) const
{
  return name1.less (name2);
  //return y_strnicmp ((const char *) &p1, (const char *) &p2, WAD_NAME) < 0;
}


